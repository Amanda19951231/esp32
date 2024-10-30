#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

// Defina as configurações do Access Point (Wi-Fi gerado pelo ESP8266)
const char* ssid = "ESP8266_Pluviometro";
const char* password = "iffar2024";

// Configurações para IP fixo
IPAddress local_IP(192, 168, 100, 1);  // IP fixo para o ESP8266
IPAddress gateway(192, 168, 100, 1);   // Gateway será o mesmo IP do ESP8266
IPAddress subnet(255, 255, 255, 0);    // Máscara de sub-rede

// Porta para o servidor web
AsyncWebServer server(80);

// Defina o pino do microswitch (conectado ao ímã)
const int microswitchPin = 5; // D1 no WeMos D1 Mini corresponde ao GPIO 5

// Variáveis para armazenar o volume de chuva e o estado do pluviômetro
volatile float rainVolume = 0.0;
volatile int rainCount = -1;  // Inicializando com -1, indicando que o pluviômetro não está funcionando
volatile int magnetPassCount = 0; // Contador para o número de passagens do ímã

// Variável para armazenar o último estado do sensor
int lastSensorState = HIGH;  // Considerando que o estado inicial é HIGH
unsigned long lastChangeTime = 0; // Tempo da última mudança de estado
const unsigned long checkInterval = 5000; // Intervalo de 5 segundos para verificar

// Defina o tempo de debounce em milissegundos
const unsigned long debounceTime = 200; // 200ms de debounce

// Variável para armazenar o tempo da última interrupção
volatile unsigned long lastDebounceTime = 0;

// Função chamada quando o ímã passa pelo microswitch
void IRAM_ATTR countRain() {
  // Verifica o tempo atual
  unsigned long currentTime = millis();

  // Se o tempo desde a última interrupção for menor que o tempo de debounce, ignore o evento
  if ((currentTime - lastDebounceTime) > debounceTime) {
    if (rainCount == -1) {
      rainCount = 0;  // Se era -1 (não estava funcionando), agora o pluviômetro passa a funcionar
    }

    rainVolume += 0.1;  // A cada passagem, adiciona 0.1 mm de chuva
    rainCount++;  // Incrementa a contagem de chuvas
    magnetPassCount++;  // Incrementa a contagem de passagens do ímã

    // Exibe o volume de chuva no monitor serial
    Serial.print("Volume de Chuva Atual: ");
    Serial.print(rainVolume);
    Serial.println(" mm");

    // Atualiza o tempo da última interrupção
    lastDebounceTime = currentTime;
  }
}

// Função para verificar se o microswitch reed está conectado ao circuito
String checkSensorStatus() {
  int sensorState = digitalRead(microswitchPin);
  
  // Se o estado mudou, o sensor está conectado
  if (sensorState != lastSensorState) {
    lastSensorState = sensorState;  // Atualiza o último estado
    lastChangeTime = millis();  // Atualiza o tempo da última mudança
    return "Conectado";  // O sensor está respondendo ao ímã
  }
  
  // Se o estado não mudou dentro de um intervalo de tempo, considera desconectado
  if (millis() - lastChangeTime > checkInterval) {
    return "Desconectado";  // O sensor não mudou de estado recentemente
  }

  return "Conectado";  // Caso contrário, ainda está conectado
}

// Função para gerar a página HTML dinamicamente
String generateHTML() {
  String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Pluviômetro Online</title>";
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; }";
  html += ".container { width: 100%; padding: 10px; margin: 0 auto; max-width: 1200px; }";  
  html += "header { background: #4CAF50; color: white; padding: 10px 0; }";
  html += "header h1 { text-align: center; font-size: 2em; }";  
  html += ".info { padding: 30px 0; background-color: #fff; text-align: center; }";  
  html += ".data-display { display: flex; flex-direction: column; align-items: center; gap: 10px; }";  
  html += ".data-item { background: #eee; padding: 15px; border-radius: 10px; margin: 5px 0; width: 90%; max-width: 400px; }";  
  html += "footer { position:fixed; bottom: 0; width: 100%; background-color: #4CAF50; color: white; text-align: center; padding: 10px 0; z-index: 1000;}";
  html += ".working { background-color: #4CAF50; color: white; }";  
  html += ".not-working { background-color: #bbb; color: white; }";  
  html += "@media (min-width: 768px) {";  
  html += ".data-display { flex-direction: row; flex-wrap: wrap; justify-content: space-around; }";
  html += ".data-item { width: 30%; }";  
  html += "}";
  html += "</style></head><body>";

  // Cabeçalho
  html += "<header><div class='container'><h1>Monitoramento de Chuvas em Tempo Real</h1></div></header>";

  // Sessão de Dados Recentes
  html += "<main><section class='info'><div class='container'>";
  html += "<h2>Dados Recentes de Precipitação Pluviométrica</h2>";
  html += "<div class='data-display'>";
  
  // Volume de chuva acumulado
  html += "<div class='data-item'><h3>Volume de Chuva (Acumulado)</h3><p>" + String(rainVolume) + " mm</p></div>";

  // Contagem de passagens do ímã
  html += "<div class='data-item'><h3>Contagem de Passagens do Ímã</h3><p>" + String(magnetPassCount) + "</p></div>"; 

  // Estado do pluviômetro
  if (rainCount == -1) {
    html += "<div class='data-item not-working'><h3>Estado do Pluviômetro</h3><p>Não Funciona</p></div>";
  } else {
    html += "<div class='data-item working'><h3>Estado do Pluviômetro</h3><p>Funcionando</p></div>";
  }

  // Estado do sensor e microswitch reed
  String sensorStatus = checkSensorStatus();
  if (sensorStatus == "Conectado") {
    html += "<div class='data-item working'><h3>Microswitch Reed</h3><p>" + sensorStatus + "</p></div>";
  } else {
    html += "<div class='data-item not-working'><h3>Microswitch Reed</h3><p>" + sensorStatus + "</p></div>";
  }
  
  html += "</div></div></section>";

  // Sessão Sobre
  html += "<section class='info'><div class='container'>";
  html += "<h2>Sobre o Pluviômetro</h2>";
  html += "<p>O pluviômetro é um dispositivo utilizado para medir a quantidade de chuva que caiu em um determinado período. ";
  html += "Esta página oferece dados em tempo real, ajudando a prever inundações, monitorar a distribuição de chuvas e auxiliar em pesquisas climáticas.</p>";
  html += "</div></section></main>";

  // Rodapé
  html += "<footer><div class='container'><p>&copy; 2024 Pluviômetro Online. Todos os direitos reservados.</p></div></footer>";

  // JavaScript para atualizar automaticamente a página a cada 5 segundos
  html += "<script>";
  html += "setInterval(() => { window.location.reload(); }, 5000);";
  html += "</script>";

  html += "</body></html>";

  return html;
}

void setup() {
  // Iniciar a comunicação serial
  Serial.begin(115200);

  // Configurar o ESP8266 como Access Point (ponto de acesso)
  WiFi.softAP(ssid, password);

  // Definir as configurações de IP fixo no modo Access Point
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("Falha ao configurar IP fixo");
  }

  // Informar que o servidor está funcionando
  Serial.println();
  Serial.println("Acesse o monitoramento via: http://192.168.100.1");

  // Configurar o pino do microswitch
  pinMode(microswitchPin, INPUT_PULLUP);  // Usa o resistor pull-up interno

  // Configurar a interrupção no pino do microswitch
  attachInterrupt(digitalPinToInterrupt(microswitchPin), countRain, FALLING);

  // Iniciar o servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", generateHTML());
  });

  // Iniciar o servidor
  server.begin();
}

void loop() {
}
