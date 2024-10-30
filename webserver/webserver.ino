#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Amanda";         // Insira o nome da sua rede Wi-Fi
const char* password = "984373617";  // Insira a senha da sua rede Wi-Fi

WebServer server(80);

void handleRoot() {
  if (server.method() == HTTP_GET) {
    server.send(200, "text/html", "<html><body><h1>Hello from ESP32!</h1></body></html>");
    Serial.println("Requisição enviada com sucesso!");
  } else {
    server.send(405, "text/plain", "Método não permitido.");
    Serial.println("Requisição recusada - Método não permitido.");
  }
}

void setup() {
  Serial.begin(115200);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");

  int timeout = 100;  // Definir tempo limite de conexão em segundos
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado ao Wi-Fi com sucesso! IP do servidor: " + WiFi.localIP().toString());

    Serial.println("Servidor web iniciado!");
  } else {
    Serial.println("\nFalha ao conectar ao Wi-Fi. Verifique as credenciais ou a conexão.");
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
