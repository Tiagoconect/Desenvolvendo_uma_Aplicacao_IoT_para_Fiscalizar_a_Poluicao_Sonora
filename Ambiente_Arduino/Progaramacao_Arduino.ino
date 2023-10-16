//Servidor NTP para manter a hora e a data atualizada via wifi
#include <NTPClient.h>
#include <WiFiUdp.h>
const char* ntpServerName = " br.pool.ntp.org"; // outros possiveis se der errado: link: https://ntp.br/conteudo/ntp/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServerName);

/* Bibliotecas para produzir graficos e relatorios se for preciso (pesquisar mais sobre elas)
  #include <Adafruit_ILI9341.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_GrayOLED.h>
  #include <Adafruit_SPITFT.h>
  #include <Adafruit_SPITFT_Macros.h>
  #include <gfxfont.h>
*/

// micro anologico: https://arduinobymyself.blogspot.com/2012/04/vu-meter-com-arduino.html
// micro anologico com CI: http://icexduino.blogspot.com/2011/06/teste_26.html


#include <Servo.h>




Servo myServo; // objeto de uso para controlar o servo motor está relacionado com a biblioteca Servo.h

//variaveis e constante para medição do logico da entrada digital do sensor microfone KY-037
//const int OUT_PIN = 8;  //entrada do valor digital
const int microfonePin = A0;  // Pino analógico onde o microfone está conectado (Microfone analogico)

// Variaveis relacionado ao intervalos de amostras (Valor medio)
const int amostra = 2000; // acada  mili segundos ele gera uma amostra (Precisa definir apos a logica)
unsigned long tempo_atual;  // salva o tempo atual da função millis()
unsigned long tempo_anterior = 0; //Armazena o valor anterior de millis() para comparação
unsigned long tempo_decorrido = 0;  // Variavel para calcular o tempo decorrido desde a última amostragem
int guarda_amostra = 0;  // variavel para armazenar as mostras
float soma_amostras = 0;  // variavel para soma a amostras acumuladas dentro loop

//Variaveis relacionadas com os nvalores maximos e minimos (Resolução de 10 bits)
int pico_minimo = 1023;
int pico_maximo = 0;
unsigned long tempo_pico_anterior = 0;
float frequencia_picos = 0;
float tolerancia_pico= 1.05;


//Inciando comunicação via wifi com esp8266 declacando variaveis de acesso:
#include <ESP8266WiFi.h>
const char* ssid = "tiago";
const char* password = "ajux2896";

//Iniciando comunicação via protocolo HTPP e a criação da pagina web:
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include "index.h" // Conteudo da pagina WEB
ESP8266WebServer server(80);


void setup() {
  Serial.begin(9600);
//Inicializando comunicação Wifi
WiFi.begin(ssid, password);

//Verificando a conexão com a rede
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println("Servidor iniciado"); 
  Serial.print("IP para se conectar com o radar");
  Serial.print("http://"); 
  Serial.println(WiFi.localIP()); 
  server.on("/", HTTP_GET, paginaWeb);
  server.begin();

  
  // Inicializando o cliente NTP
  timeClient.begin();
  timeClient.setTimeOffset(3*3600); // fuso horário
  /*=============================================SENSOR MICROFONE =====================================================================*/
  // pinMode(OUT_PIN, INPUT); para entrada digital

  /*====================================================================================================================================*/


  /*=============================================SERVO MOTOR============================================================================*/
  myServo.attach(9); // o servo motor está conctado no pino digital 9


  /*====================================================================================================================================*/

}

void loop() {

  /*=============================================MICROFONE-LEITURA DE DBS================================================================*/

  tempo_atual = millis(); // a função millis captura a hora atual
  tempo_decorrido = tempo_atual - tempo_anterior;

  

  /*
     Fazer estrutura de seleção conforme a lesgislação
     para ruido: https://www.nucleodoconhecimento.com.br/wp-content/uploads/2021/06/grafico-63.png
     niveis por area: https://blog.vprimoveis.com.br/wp-content/uploads/2021/07/Tabela-de-ruido-ABNT.png
  */

  float valor_medio = media_amostral();
  valores_picos();
  
  if (tempo_decorrido > amostra) {  // Se o tempo decorrido for maior que a amostra, imprima o valor médio
    
    
      Serial.println("Valor Médio do Microfone analogico): " + String(valor_medio));
      // Converte o valor médio em dB usando a função converteDB
      float dBValor = converte_DB(valor_medio);
      Serial.println("Valor Médio do Microfone analogico em Db): " + String( dBValor));

       Serial.println("Pico Mínimo: " + String(pico_minimo));
       Serial.println("Pico Maximo: " + String(pico_maximo));

       // Chamando a função de tratamento de informações
      tratarInformacoes(tempo_atual, valor_medio, pico_maximo, frequencia_picos);
   
       
      /*
              Logica parte (basica) em relação a diretrizes do Programa de Silêncio Urbano (PSIU), Lei 15.133 link:https://novabrasilfm.com.br/especiais/cidadania/voce-conhece-a-lei-do-psiu/#:~:text=Os%20limites%20de%20ru%C3%ADdo%20s%C3%A3o,entre%2045%20e%2055%20decib%C3%A9is.
      */

      //int horaAtual = hour();  //variavel para registrar a hora atual e enviar para função 'verificarCondicoes'
      int horaAtual = (tempo_atual / 3600000) % 24;
      verificarCondicoes(dBValor, horaAtual);  // Chamando a condição de saida da função de acordo com o valor medio lido pelo microfone em Db

     if (pico_maximo > tolerancia_pico * amostra && tempo_pico_anterior > 0) {
      frequencia_picos = 1000.0 / (tempo_atual - tempo_pico_anterior); // Calculate frequency in Hz
      Serial.println("Frequência dos picos (Hz): " + String(frequencia_picos));

     
    }
    

    // Zerarando as amostras, os valores de piuco e tempo e renicianso o loop
    soma_amostras = 0;
    guarda_amostra = 0;
    tempo_anterior = tempo_atual;
    pico_minimo = 1023;
    pico_maximo = 0;
    tempo_pico_anterior = 0;
  }


  /*================================================================================================================================*/

  /*=============================================SERVO MOTOR=========================================================================*/

  /* if (tempo_decorrido > amostra) {  // se a o tempo decorrido for maior que amosta então imprima o valor acumulado

      myServo.write(90); // Gire o servo para 90 graus (ajustar) usado para teste inicial

      if (OUT_PIN == LOW)
        // Ajustar direção do servo
        myServo.write(45); // Gire o servo para a esquerda
      } else {
        // Ajustar servo para o centro
        myServo.write(90); // Gire o servo para o centro
      }
  */


  /*================================================================================================================================*/

  
  server.handleClient();


}

/*=============================================CONVENÇÃO DO VALOR ANALOGICO DO MICROFONE PARA DBS==================================*/


float converte_DB(float valor_medio) {
  //lógica de conversão de valor de entrada para dB aqui
  float tensao = valor_medio / (1023.0 * 4.53); //esta relacionado com a resolução maxima do aruino 10 bit de 0 a 5V

  float dBValor =  (-1) * (87.1 * tensao - 74.4);; //fórmula de conversão

  /*Referencias:
     http://projetosfisicaexperimental.blogspot.com/2017/03/medidor-de-nivel-sonoro.html
     https://circuitdigest.com/microcontroller-projects/arduino-sound-level-measurement
  */
  //float dBValor = (valor_medio+83.2073) / 11.003;

  return dBValor;
}

/*=====================================================================================================================================*/


/*=============================================LEITURA DOS VALORES MEDIO DAS AMOSTRAS================================================*/

float media_amostral() {
  //Leitura e soma das amostras entrada analogica

  int valor_analogico = analogRead(microfonePin);     // Lê o valor analógico do microfone
  soma_amostras += valor_analogico;                  // atribuiçãop da variavel valo_analogico na soma de amostra
  guarda_amostra++;                                   // Incremetação

  return soma_amostras / guarda_amostra;
}

/*=====================================================================================================================================*/


/*=============================================LEITURA DOS VALORES PERIODICOS DO SENSOR================================================*/

void valores_picos() {
  int valor_analogico = analogRead(microfonePin);
  if (valor_analogico > pico_maximo) {
    pico_maximo = valor_analogico;
    tempo_pico_anterior = millis(); // Record the time of the peak
  }
  if (valor_analogico < pico_minimo) {
    pico_minimo = valor_analogico;
  }
}

/*=====================================================================================================================================*/


/*=============================================VERIFICANDO CONDIÇÕES INICIAIS DAS DIRETRIZES DO PROGRAMA PSIU===========================*/

//O objetivo dessa Função é fazer as compareções condicionais a partir da hora atual e o valor de Db (a localização entraria aqui com uma variavel, esta sensdo estudado como usar)

void verificarCondicoes(float dBValor, int horaAtual) {

  // Verifique as condições com base nas zonas e horários
  if (horaAtual >= 7 && horaAtual < 22) {              // Horário de 07h às 22h
    Serial.println("Horário de 07h às 22h");
    Serial.println("Zona Residencial");

    if (dBValor > 50) {
      Serial.println("Nível de ruído alto na Zona Residencial!");
      //elaborar um protocolo a ser seuido
    }

    else {
      Serial.println("Nível de ruído aceitável na Zona Residencial.");
    }
  }

  else {
    if (dBValor > 45) {
      Serial.println("Nível de ruído alto na Zona Residencial!");
      //elaborar um protocolo a ser seuido
    }

    else {
      Serial.println("Nível de ruído aceitável na Zona Residencial.");
    }

  }


}


/*=====================================================================================================================================*/

//Função para ler conteudo pagina web principal e abrir assim que a comicação do servidor web é estabelecida
void paginaWeb() {
  String s = MAIN_page; // Le o conteudo HTML
  server.send(200, "text/html", s); // Envia a pagina web
}


//Tratando os dados principais

void tratarInformacoes(unsigned long tempoAtual, int valor_medio, int picoMaximo, float frequenciaPicos) {
 // condição minima para o dada ser importante (ja que para os limite estabelecidos 45 dD é o menor valor)
  if (valor_medio > 40) {
  
    frequenciaPicos *= 100;

    //Obetentado a atualização data em hora
    timeClient.update();
    String dataHoraAtual = timeClient.getFormattedTime();

    Serial.print("Informações tratadas - Tempo Atual: ");
    Serial.print(tempoAtual);
    Serial.print(", Media amostral: ");
    Serial.print(valor_medio);
    Serial.print(", Pico Máximo: ");
    Serial.print(picoMaximo);
    Serial.print(", Frequência: ");
    Serial.print(frequenciaPicos);
    Serial.print(", Data e Hora Atuais: ");
    Serial.println(dataHoraAtual);
  }
}