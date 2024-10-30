#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "LittleFS.h"

// Configurações do Wi-Fi
const char* ssid = "ESP8266_Access_Point";
const char* password = "12345678";

ESP8266WebServer server(80);

void handleFileRequest() {
  File file = LittleFS.open("/test_example.txt", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  String fileContent = "";
  while (file.available()) {
    fileContent += (char)file.read();
  }
  file.close();
  server.send(200, "text/plain", fileContent);
}

void setup() {
  Serial.begin(115200);
  
  // Inicia o sistema de arquivos
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Cria o ponto de acesso Wi-Fi
  WiFi.softAP(ssid, password);

  // Exibe o IP do ESP8266
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define o roteamento para o arquivo
  server.on("/", handleFileRequest);

  // Inicia o servidor
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Mantém o servidor rodando
  server.handleClient();
}
