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
const int amostra = 2000; // acada  mili segundos ele gera uma amostra (Precisa definir apos a logica)
unsigned long tempo_atual;  // salva o tempo atual da função millis()
unsigned long tempo_anterior = 0; //Armazena o valor anterior de millis() para comparação
unsigned long tempo_decorrido = 0;  // Variavel para calcular o tempo decorrido desde a última amostragem

int guarda_amostra = 0;  // variavel para armazenar as mostras
float soma_amostras = 0;  // variavel para soma a amostras acumuladas dentro loop


void setup() {
  Serial.begin(9600);

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

  //Leitura e soma das amostras entrada analogica

  int valor_analogico = analogRead(microfonePin);     // Lê o valor analógico do microfone
  soma_amostras += valor_analogico;                  // atribuiçãop da variavel valo_analogico na soma de amostra
  guarda_amostra++;                                   // Incremetação


  /*
     Fazer estrutura de seleção conforme a lesgislação
     para ruido: https://www.nucleodoconhecimento.com.br/wp-content/uploads/2021/06/grafico-63.png
     niveis por area: https://blog.vprimoveis.com.br/wp-content/uploads/2021/07/Tabela-de-ruido-ABNT.png
  */


  if (tempo_decorrido > amostra) {  // Se o tempo decorrido for maior que a amostra, imprima o valor médio
    if (guarda_amostra > 0) {
      float valor_medio = soma_amostras / guarda_amostra;
      Serial.println("Valor Médio do Microfone analogico): " + String(valor_medio));
      // Converte o valor médio em dB usando a função converteDB
      float dBValor = converte_DB(valor_medio);
      Serial.println("Valor Médio do Microfone analogico em Db): " + String( dBValor));

      /*
              Logica parte (basica) em relação a diretrizes do Programa de Silêncio Urbano (PSIU), Lei 15.133 link:https://novabrasilfm.com.br/especiais/cidadania/voce-conhece-a-lei-do-psiu/#:~:text=Os%20limites%20de%20ru%C3%ADdo%20s%C3%A3o,entre%2045%20e%2055%20decib%C3%A9is.
      */

      //int horaAtual = hour();  //variavel para registrar a hora atual e enviar para função 'verificarCondicoes'
      int horaAtual = (tempo_atual / 3600000) % 24;
      verificarCondicoes(dBValor, horaAtual);  // Chamando a condição de saida da função de acordo com o valor medio lido pelo microfone em Db
    }

    // Zerarando as amostras e o tempo e renicianso o loop
    soma_amostras = 0;
    guarda_amostra = 0;
    tempo_anterior = tempo_atual;
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


/*=============================================LEITURA DOS VALORES PERIODICOS DO SENSOR================================================*/



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