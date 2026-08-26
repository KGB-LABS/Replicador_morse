#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h> 
// --- PINOS CYD 2.4 ---
#define BACKLIGHT_PIN 27
#define SPEAKER_PIN   26

// --- CONFIGURAÇÃO DO WI-FI (Access Point) ---
const char *ssid = "NEKO_MORSECODE"; // 
const byte DNS_PORT = 53;

DNSServer dnsServer;
WebServer server(80);

TFT_eSPI tft = TFT_eSPI();

// Tabela Morse (A-Z)
const char* morseAlphabet[26] = {
  ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", "..",   // A-I
  ".---", "-.-",  ".-..", "--",   "-.",   "---",  ".--.", "--.-", ".-.",  // J-R
  "...",  "-",    "..-",  "...-", ".--",  "-..-", "-.--", "--.."          // S-Z
};

// --- FUNÇÕES DE ÁUDIO ---
void emitirTom(int frequencia, int duracaoMs) {
  long periodoUs = 1000000L / frequencia;
  long meioPeriodoUs = periodoUs / 2;
  long ciclos = ((long)frequencia * duracaoMs) / 1000;

  for (long i = 0; i < ciclos; i++) {
    digitalWrite(SPEAKER_PIN, HIGH);
    delayMicroseconds(meioPeriodoUs);
    digitalWrite(SPEAKER_PIN, LOW);
    delayMicroseconds(meioPeriodoUs);
  }
}

void tocarMorse(const char* codigo) {
  int dotDuration = 90; 
  for (int i = 0; codigo[i] != '\0'; i++) {
    if (codigo[i] == '.') {
      emitirTom(750, dotDuration);        
    } else if (codigo[i] == '-') {
      emitirTom(750, dotDuration * 3);    
    }
    delay(dotDuration);
  }
}

// --- DESENHO
void desenharMorseVisual(const char* morse) {
  int gap = 15;        // Espaço entre símbolos
  int dotD = 24;       // Diâmetro do Ponto
  int dashW = 60;      // Largura do Traço
  int dashH = 24;      // Altura do Traço

  
  int totalW = 0;
  for (int i = 0; morse[i] != '\0'; i++) {
    if (morse[i] == '.') totalW += dotD;
    else if (morse[i] == '-') totalW += dashW;
    if (morse[i+1] != '\0') totalW += gap;
  }

  int x = 160 - (totalW / 2);
  int y = 170; /

  for (int i = 0; morse[i] != '\0'; i++) {
    if (morse[i] == '.') {
      tft.fillCircle(x + dotD/2, y, dotD/2, TFT_YELLOW);
      x += dotD + gap;
    } else if (morse[i] == '-') {
      tft.fillRoundRect(x, y - dashH/2, dashW, dashH, dashH/2, TFT_YELLOW);
      x += dashW + gap;
    }
  }
}

// --- FUNÇÃO DA TELA (Display ESP32) ---
void exibirNaTela(char letra, const char* morse) {
  tft.fillScreen(TFT_NAVY);
  
  // Moldura
  tft.drawRect(5, 5, 310, 230, TFT_CYAN);
  tft.drawRect(7, 7, 306, 226, TFT_CYAN);

  tft.setTextDatum(MC_DATUM); // Centralizado

  if (letra != ' ') {
    
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setTextSize(3); // Multiplica o tamanho da fonte por 3
    tft.drawString(String(letra), 160, 75, 4); 
    tft.setTextSize(1); // Restaura o tamanho

    // Desenha as formas do Morse
    desenharMorseVisual(morse);
    
  } else {
    // Tela Inicial de Espera
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawString("Conecte no Wi-Fi:", 160, 80, 4);
    
    tft.setTextColor(TFT_YELLOW, TFT_NAVY);
    tft.drawString(ssid, 160, 120, 4);
    
    tft.setTextColor(TFT_GREEN, TFT_NAVY);
    tft.drawString("A pagina abrirá sozinha no seu navegador!", 160, 170, 2);
  }
}

// --- PÁGINA WEB HTML/CSS/JS (Portal Captivo) ---
void handleRoot() {
  String html = "<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>";
  html += "<title>Teclado Morse</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #121212; color: #fff; text-align: center; margin: 0; padding: 15px; }";
  html += "h2 { color: #00E5FF; margin-bottom: 20px; }";
  html += ".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(60px, 1fr)); gap: 12px; max-width: 500px; margin: 0 auto; }";
  html += ".btn { background-color: #1E88E5; color: white; border: none; padding: 20px 0; font-size: 26px; font-weight: bold; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); cursor: pointer; transition: 0.1s; }";
  html += ".btn:active { background-color: #FFCA28; color: black; transform: scale(0.95); }";
  html += "</style>";
  html += "<script>";
  html += "function enviarLetra(letra) {";
  // Vibrate se o celular suportar (Feedback tátil agradável)
  html += "  if(navigator.vibrate) navigator.vibrate(50);"; 
  html += "  fetch('/tocar?letra=' + letra);";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  
  html += "<h2>Aprenda Codigo Morse</h2>";
  html += "<div class='grid'>";
  
  for (int i = 0; i < 26; i++) {
    char letra = 'A' + i;
    html += "<button class='btn' onclick=\"enviarLetra('" + String(letra) + "')\">" + String(letra) + "</button>";
  }
  
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// --- RECEBE O COMANDO E TOCA O ÁUDIO ---
void handleTocar() {
  if (server.hasArg("letra")) {
    String letraString = server.arg("letra");
    char letra = letraString[0]; 
    
    if (letra >= 'A' && letra <= 'Z') {
      int indice = letra - 'A';
      const char* morse = morseAlphabet[indice];
      
      exibirNaTela(letra, morse);
      
      // Responde ao celular imediatamente (para não causar lentidão no botão)
      server.send(200, "text/plain", "OK"); 
      
      // Toca o som depois de liberar o celular
      tocarMorse(morse);
      return; 
    }
  }
  server.send(400, "text/plain", "Erro");
}

// --- PORTAL CAPTIVO (Redirecionamento mágico) ---
void handleCaptivePortal() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server.send(302, "text/plain", ""); // Redireciona tudo para a página principal
}

void setup() {
  Serial.begin(115200);

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);
  pinMode(SPEAKER_PIN, OUTPUT);

  tft.init();
  tft.setRotation(1); 
  
  // --- INICIA O AP WI-FI ---
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid); // Sem senha
  
 
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Tela Inicial
  exibirNaTela(' ', "");

  // --- ROTAS DO SERVIDOR WEB ---
  server.on("/", handleRoot);
  server.on("/tocar", handleTocar);
  
  server.onNotFound(handleCaptivePortal); 

  server.begin();
  Serial.println("Sistema Pronto!");
}

void loop() {
  dnsServer.processNextRequest(); // Mantém o DNS rodando
  server.handleClient();          // Escuta o celular
}