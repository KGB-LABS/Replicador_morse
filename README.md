# 📻 Aprenda Código Morse com ESP32 CYD (Web Server + Captive Portal)

Um projeto interativo para aprender e treinar Código Morse utilizando a placa **ESP32 Cheap Yellow Display (CYD 2.4")**. 

Este projeto transforma o ESP32 em um roteador Wi-Fi (Access Point) com um **Portal Captivo**. Basta conectar o seu celular à rede criada pela placa, e um teclado virtual abrirá automaticamente na tela do seu smartphone. Ao digitar, a placa exibe a letra, os símbolos geométricos do Morse, emite o som pelo alto-falante e pisca o LED frontal em sincronia perfeita!

![Visão Geral do Projeto](coloque_uma_foto_do_projeto_aqui.jpg)

## ✨ Funcionalidades
* **📱 Portal Captivo Inteligente:** Não precisa de internet nem baixar apps. Conecte no Wi-Fi e a página web abre sozinha no celular.
* **🔊 Áudio Integrado:** Utiliza o amplificador nativo da CYD para emitir os bipes (pontos e traços) no tempo correto.
* **💡 Feedback Visual (LED RGB):** O LED frontal pisca na cor **Branca** em sincronia absoluta com o áudio (simulando um holofote de navio).
* **🖥️ Interface Gráfica:** Tela desenhada nativamente utilizando `TFT_eSPI` com formas geométricas limpas e de fácil leitura.

---

## 🛠️ Materiais Necessários

Para reproduzir este projeto, você precisará apenas de:

1. **Placa ESP32 CYD 2.4"** (Modelo exato: `ESP32-2432S024` com display ST7789).
   * *Nota: Este projeto foi desenhado especificamente para contornar problemas físicos no touch screen resistivo da placa, transferindo o controle do toque para o seu celular via Wi-Fi!*
2. **Cabo USB-C** ou Micro-USB (dependendo da revisão da sua placa) para programação e alimentação.
3. **Smartphone ou Computador** (para acessar a rede Wi-Fi e usar o teclado virtual).

---

## ⚙️ Dependências e Bibliotecas

Você precisará instalar as seguintes bibliotecas na Arduino IDE:

* `TFT_eSPI` (Para os gráficos na tela)
* `WiFi` (Nativa do ESP32)
* `WebServer` (Nativa do ESP32)
* `DNSServer` (Nativa do ESP32, responsável pela "mágica" do portal captivo)

### ⚠️ Configuração Crítica: `User_Setup.h` (TFT_eSPI)
O maior desafio da placa CYD 2.4" é configurar a tela corretamente. Vá até a pasta da biblioteca `TFT_eSPI`, abra o arquivo `User_Setup.h`, apague tudo e cole as configurações abaixo:

```cpp
#define ST7789_DRIVER      
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   27          // Pino de Backlight da CYD 2.4"
#define TFT_BACKLIGHT_ON HIGH

#define TFT_INVERSION_ON   // Evita cores/linhas invertidas

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define SPI_FREQUENCY  40000000
