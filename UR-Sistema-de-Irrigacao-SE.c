#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "lib/ssd1306.h"
#include "pio_matrix.pio.h"

// Definições do I2C e display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISP_ADDR 0x3C

// Definições do Joystick, UART e Matriz de LEDs
#define JOY_X 26
#define JOY_Y 27
#define MATRIX_PIN 7
#define NUM_PIXELS 25
#define UART_BAUD_RATE 115200 // Velocidade da UART

// Definições do UART
const uint UART_PIN_TX = 0, UART_PIN_RX = 1;

// Define o pino do buzzer (GPIO 21 da Raspberry Pi Pico W)
#define BUZZER_PIN 21

// Define pinos do LED RGB e botões
#define BTN_A 5
#define BTN_B 6
#define LED_R 13
#define LED_G 11
#define LED_B 12

// Definições para o Deboucing
#define DEBOUNCE_DELAY_MS 200 // Tempo para evitar múltiplos disparos

volatile uint32_t ultimo_evento_A = 0;
volatile uint32_t ultimo_evento_B = 0;

// Variáveis globais
int limite_umidade_superior = 70; // Valor limite para umidade alta
int limite_umidade_inferior = 50; // Valor limite para umidade baixa
bool estado_led_vermelho = false; // Controlar estado do led

uint16_t centro_x, centro_y; // Valores centrais calibrados do joystick
bool estado_borda = false;   // Estado da borda (simples/dupla)

// Configuração do I2C
i2c_inst_t *i2c = I2C_PORT;
ssd1306_t display;

// Posição do quadrado (inicialmente centralizado)
int pos_x = 64;
int pos_y = 32;

// Formatos das setas (apenas os pixels da seta acendem)
double seta_cima[25] = {0, 0, 1, 0, 0,
                        0, 0, 1, 0, 0,
                        1, 1, 1, 1, 1,
                        0, 1, 1, 1, 0,
                        0, 0, 1, 0, 0};

double seta_baixo[25] = {0, 0, 1, 0, 0,
                         0, 1, 1, 1, 0,
                         1, 1, 1, 1, 1,
                         0, 0, 1, 0, 0,
                         0, 0, 1, 0, 0};

double vazio[25] = {0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0};

// Declaração da função de desligar LEDs
void desligar_led_rgb();

// Declaração da função para atualizar estado do LED RGB
void atualizar_led_rgb(bool vermelho);

// Inicializa UART para enviar dados via serial
void configurar_uart()
{
    // Configuração do UART
    uart_init(uart0, UART_BAUD_RATE);
    gpio_set_function(UART_PIN_RX, GPIO_FUNC_UART);
    gpio_set_function(UART_PIN_TX, GPIO_FUNC_UART);
    uart_set_fifo_enabled(uart0, true);
}

// Conversão RGB para matriz de LEDs
uint32_t matrix_rgb(double valor)
{
    return valor ? 0x00FF0000 : 0x00000000; // Apenas pixels da seta acendem
}

void somAdicionar(uint freq, uint duration_ms)
{
    // Define o pino do buzzer como saída PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Obtém o número do "slice" PWM associado ao pino
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Calcula o valor do contador 'top' com base na frequência desejada
    // clock da Pico é 125 MHz, então top = 125_000_000 / freq
    uint top = 125000000 / freq;

    // Define o valor máximo do contador PWM (o "wrap")
    pwm_set_wrap(slice, top);

    // Define o nível do canal PWM (duty cycle)
    // Aqui está usando 80% do valor máximo (volume mais alto)
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER_PIN), (top * 8) / 10);

    // Ativa a saída PWM para gerar o som
    pwm_set_enabled(slice, true);

    // Mantém o som ativo pelo tempo especificado
    sleep_ms(duration_ms);

    // Desliga o som
    pwm_set_enabled(slice, false);

    // Pequena pausa entre tons (evita sobreposição)
    sleep_ms(20);
}

void somDesativar()
{
    // Define o pino do buzzer como saída PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Obtém o número do "slice" PWM associado ao pino
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Desliga o som
    pwm_set_enabled(slice, false);
}
// Atualiza a matriz de LEDs com base na umidade
void desenhar_matriz(double umidade)
{
    double *desenho = NULL;

    if (umidade > limite_umidade_superior)
    {
        desenho = seta_cima;
        desligar_led_rgb(); // Desligar LED
    }
    else if (umidade < limite_umidade_inferior)
    {
        desenho = seta_baixo;
        somAdicionar(1200, 100);
    }
    else
    {
        desenho = vazio;
        somDesativar();
    }

    for (int16_t i = 0; i < NUM_PIXELS; i++)
    {
        uint32_t cor = matrix_rgb(desenho[i]);
        pio_sm_put_blocking(pio0, 0, cor);
    }
}

// Exibir apenas o quadrado no OLED
void exibir_quadrado()
{
    ssd1306_fill(&display, false);
    ssd1306_rect(&display, pos_x, pos_y, 8, 8, true, true);
    ssd1306_send_data(&display);
}

// Configuração do I2C e display OLED
void configurar_i2c()
{
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&display, 128, 64, false, DISP_ADDR, i2c);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);
}

// Calibração do joystick
void calibra_joystick()
{
    const int amostras = 100;
    uint32_t soma_x = 0, soma_y = 0;

    for (int i = 0; i < amostras; i++)
    {
        adc_select_input(0);
        soma_x += adc_read();
        adc_select_input(1);
        soma_y += adc_read();
        sleep_ms(5);
    }

    centro_x = soma_x / amostras;
    centro_y = soma_y / amostras;
}

// Função para atualizar estado do LED RGB
void atualizar_led_rgb(bool vermelho)
{
    gpio_put(LED_R, vermelho ? 1 : 0);
    gpio_put(LED_G, vermelho ? 0 : 1);
    gpio_put(LED_B, 0);
}

void desligar_led_rgb()
{
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);
}

// Função de interrupção dos botões
void trata_interrupcao_botoes(uint gpio, uint32_t events)
{
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_A && (tempo_atual - ultimo_evento_A > DEBOUNCE_DELAY_MS))
    {
        ultimo_evento_A = tempo_atual;
        estado_led_vermelho = true;
        atualizar_led_rgb(true);
    }
    else if (gpio == BTN_B && (tempo_atual - ultimo_evento_B > DEBOUNCE_DELAY_MS))
    {
        ultimo_evento_B = tempo_atual;
        estado_led_vermelho = false;
        atualizar_led_rgb(false);
    }
}

// Função para configurar pinos e interrupções
void configurar_pinos()
{
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &trata_interrupcao_botoes);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &trata_interrupcao_botoes);

    // adc_init();
    // adc_gpio_init(SENSOR_UMIDADE);
}

// Função para monitorar umidade e ajustar LED
void monitorar_umidade(double umidade)
{
    if (umidade > limite_umidade_superior && estado_led_vermelho)
    {
        estado_led_vermelho = false;
        atualizar_led_rgb(false); // Voltar para verde
    }
}

void config_adc()
{
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);
    adc_select_input(0);
}
// **Loop principal**
int main()
{
    stdio_init_all();

    configurar_pinos(); // Inicializa LEDs
    configurar_uart(); // Inicializa UART
    config_adc(); // Inicializa ADC
    configurar_i2c(); // Inicializa I2C
    calibra_joystick(); // Calibra Joystick

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, MATRIX_PIN);

    while (1)
    {
        adc_select_input(0);
        uint16_t bruto_x = adc_read();
        adc_select_input(1);
        uint16_t bruto_y = adc_read();

        pos_x = ((4095 - bruto_x) * 52) / 4095;
        pos_y = (bruto_y * 113) / 4095;

        ssd1306_fill(&display, false);

        if (!estado_borda)
        {
            ssd1306_rect(&display, 0, 0, 127, 63, 1, false);
        }
        else
        {
            ssd1306_rect(&display, 0, 0, 127, 63, 1, false);
            ssd1306_rect(&display, 2, 2, 124, 60, 1, false);
        }

        ssd1306_rect(&display, pos_x, pos_y, 8, 8, 1, true);
        ssd1306_send_data(&display);

        sleep_ms(50);

        desenhar_matriz(pos_y);
        monitorar_umidade(pos_y);

        // **Enviar valores do joystick via UART**
        printf("Joystick X: %d, Joystick Y: %d\n", pos_x, pos_y);

        sleep_ms(500);
    }
}
