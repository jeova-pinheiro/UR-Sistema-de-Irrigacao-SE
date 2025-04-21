# Tarefa 2 - Projeto: Espantalho Eletrônico Inteligente com BitDogLab  
**Fase 2 - EmbarcaTech**

## 📌 Descrição  
Este projeto implementa um sistema de monitoramento e simulação de irrigação utilizando o microcontrolador **Raspberry Pi Pico W**, com a placa **BitDogLab**. A partir da leitura do joystick, o sistema simula o nível de umidade do solo e reage com sinais visuais (LED RGB e matriz WS2812), sonoros (buzzer) e gráficos (display OLED), indicando se o solo está seco, ideal ou úmido. Além disso, botões controlam manualmente o estado do alerta, utilizando interrupções e tratamento de debounce.

## 🛠️ Componentes Utilizados  
- **Microcontrolador:** Raspberry Pi Pico W (Placa BitDogLab)  
- **Joystick Analógico** (simula leitura da umidade com os pinos ADC)  
- **Botões A e B** (entrada digital com interrupções)  
- **Display OLED SSD1306** (conectado via I2C)  
- **Matriz de LEDs WS2812** (controlada com PIO)  
- **LED RGB** (usado para indicar estados de alerta)  
- **Buzzer** (emissão de som via PWM)

## 🔥 Funcionalidades  
- ✅ Simulação de leitura de umidade do solo com o joystick  
- ✅ Indicação visual no display OLED com movimento de quadrado e moldura variável  
- ✅ Exibição de setas para cima/baixo na matriz WS2812 conforme a "umidade"  
- ✅ LED RGB alternando entre verde (ideal) e vermelho (solo seco ou alerta)  
- ✅ Emissão de som via buzzer quando a umidade está abaixo do ideal  
- ✅ Interrupções nos botões A e B para controle manual do alerta  
- ✅ Comunicação serial (UART) para envio dos valores do joystick  

## 📄 Estrutura do Projeto  
- `main.c` → Lógica principal do sistema, leitura do joystick, controle de LEDs, buzzer, e display  
- `pio_matrix.pio` → Programa PIO para controle da matriz WS2812  
- `font.h` / `ssd1306.h` → Bibliotecas auxiliares para controle do display OLED

## 🖥️ Como Executar o Projeto  

### Passo 1: Clone o repositório do projeto e abra-o no VS Code

### Passo 2: Configurar o Ambiente  
Garanta que o **Pico SDK** está corretamente instalado e que o **VS Code** possui a extensão **Raspberry Pi Pico**.

### Passo 3: Compilar o Código  
Compile o projeto pelo VS Code com a opção "Build".

### Passo 4: Carregar o Código na Placa  
1. Conecte a placa **BitDogLab** via cabo USB  
2. Coloque a placa em modo **bootsel**  
3. Use a opção "**Run Project (USB)**" da extensão para enviar o `.uf2`

### Passo 5: Verificar o Funcionamento  
A movimentação do joystick mudará a posição do quadrado no display. Dependendo da "umidade", setas e LEDs reagirão, e sons serão emitidos pelo buzzer.

## 📌 Autor  
Projeto desenvolvido por **Jeová Pinheiro** para a fase 2 do ***EmbarcaTech***
