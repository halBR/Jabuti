#include <WiFi.h>
#include <WebServer.h>
#include <esp32-hal.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

int hour = 0;
int minute = 0;

//Programcao
int horaLigar = 6
int minutoLigar = 0
int horaDesligar = 18
int minutoDesligar = 0
int intervelaIncremento = 30


float intensidadeLigar = 0.96
float intensidadePrimeira = 0.92
float intensidadeAtual = 0
float intensidadeIncremento = 0.08

// Defina seu SSID e senha
const char* ssid = "AV_FUNDO";
const char* password = "armazem2020";  // Substitua "sua_senha_aqui" pela senha correta

// Crie uma instância do servidor na porta 80
WebServer server(80);

const int pino_MONO = 23;
const int canal_MONO= 0;
const int frequencia = 1000;
const int resolucao = 8;
//maxPWM = float (pow(2, resolucao)-1); // para 8 bits, 255 estados
int maxPWM = 255;

// Parametros do Millis
unsigned long previousMillis = 0; // Armazena a última vez que o comando foi verificado
const long interval = 60000; // Intervalo de 1 minuto em milissegundos
int lastExecutedMinute = -1; // Variável para armazenar o último minuto em que o comando foi executado

// Função de acao
void controleLed(const char* acao, float intensidade) {
  // Ligar, Desligar, Incrementar, Decrementar
  Serial.print("Parametro 1 (float): ");
  Serial.println(intensidade);
  Serial.print("Parametro 2 (string): ");
  Serial.println(acao);
}


// Função para lidar com a página principal
void handleRoot() {
  float temp = temperatureRead();
  String html = "<html><body><h1>Saida Serial do ESP32</h1><pre>";
  html += Serial.readString();
  html += "Temperatura Do chip: ";
  html += String(temp);
  html += " °C";
  html += "<br>";
  html += String(hour);
  html += ":";
  html += String(minute);
  html += "</pre></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  
  
  
  Serial.begin(115200);

  // Verifica se o RTC está conectado corretamente
  if (!rtc.begin()) {
    Serial.println("Não foi possível encontrar o módulo DS3231");
    while (1);
  }
  
  // Verifica se o RTC perdeu o poder e se o tempo precisa ser ajustado
  if (rtc.lostPower()) {
    Serial.println("O RTC perdeu o poder, ajustando o horário!");
    // Comente a linha abaixo após o primeiro upload para não reconfigurar o tempo sempre
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Espera até conectar
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

    // Mostra o IP obtido
  Serial.println("");
  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  
  
  
  //ledcSetup(canal_MONO, frequencia, resolucao);
  //ledcAttachPin(pino_MONO, canal_MONO);
  
  
  ledcAttach(pino_MONO, frequencia, resolucao);


  // Define a rota da página principal
  server.on("/", handleRoot);

  // Inicia o servidor
  server.begin();
  Serial.println("Servidor iniciado");


}


void loop() {


  //////

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    DateTime now = rtc.now();
    
    // Comando específico às 06:00
    if (now.hour() == horaLigar && now.minute() == minutoLigarminutoLigar && now.minute() != lastExecutedMinute) {
      controleLed("incrementar",(intensidadeLigar * maxPWM));
      intensidadeAtual = intensidadeLigar
      lastExecutedMinute = now.minute();
    }
    
    // Comando a cada 30 minutos entre 06:30 e 17:30
    if (now.hour() >= 6 && now.hour() < 18 && now.minute() % 30 == 0 && now.minute() != lastExecutedMinute) {
      controleLed("incrementar",(intensidadeAtual - intensidadeIncremento  * maxPWM));
      lastExecutedMinute = now.minute();
    }
    
    // Comando específico às 18:00
    if (now.hour() == 18 && now.minute() == 0 && now.minute() != lastExecutedMinute) {
      executeCommand3();
      lastExecutedMinute = now.minute();
    }
  }


  /////

  DateTime now = rtc.now();

  hour = now.hour();
  minute = now.minute();
  int second = now.second();

    // Exibe o horário atual
  Serial.print("Hora atual: ");
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) Serial.print('0'); // Para exibir os minutos com dois dígitos
  Serial.print(minute);
  Serial.print(":");
  if (second < 10) Serial.print('0'); // Para exibir os segundos com dois dígitos
  Serial.println(second);

    // Condicional para exibir a mensagem
  if ((hour >= 6 && hour < 12) || (hour == 12 && minute == 0)) {
    Serial.println("Amanhecendo");
  } else if ((hour > 12 && hour < 18) || (hour == 12 && minute > 0) || (hour == 18 && minute == 0)) {
    Serial.println("Entardecendo");
  }
  
  // Manipula as requisições do cliente
  server.handleClient();
  
  
  //Desligando
  ledcWrite(pino_MONO, 255);
  delay(1200);
  
  //ligando
  float percent4 = (0.96 * maxPWM);
  Serial.println(percent4);
  ledcWrite(pino_MONO, percent4);
  delay(1200);

  

  // Define a variável inicial
  int valor = 0.92;
  // Loop enquanto o valor for menor ou igual a 96
  while (valor >= 0.04) {
    // Imprime o valor atual no monitor serial
    Serial.println("Entrou No While");
    Serial.println(valor * maxPWM );
    ledcWrite(pino_MONO, valor * maxPWM);

    // Decrementa o valor em 8
    valor -= 0.08;
    
    // Aguarda 1 segundo antes de repetir
    delay(1200);
  }
  



  // increase the LED brightness
 // for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
   // changing the LED brightness with PWM
 //   ledcWrite(pino_MONO, dutyCycle);
    //Serial.print("Duty Cicle: ");
    //Serial.println(dutyCycle);
 //   delay(120);
 // }

  // decrease the LED brightness
  //for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
     //changing the LED brightness with PWM
  // ledcWrite(pino_MONO, dutyCycle);   
   //Serial.print("Duty Cicle: ");
    //Serial.println(dutyCycle);
   // delay(120);
 // }

   



}
