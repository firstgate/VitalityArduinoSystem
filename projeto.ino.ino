/*
 * Layout dos pinos:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               52                MOSI
 * SPI MISO   12               51                MISO
 * SPI SCK    13               50                SCK
 *
 *
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
char st[20];

int porta_rele1     = 7   ;
int porta_botao1    = 6   ;
int tempo           = 0   ;
int BotaoAnt        = 0   ;
int BotaoAtu        = 0   ;
int estadorele1     = 1   ;

float vazao;      //Variável para armazenar o valor em L/min
float aux = 0;    //Variável para tirar a média a cada 1 minuto
int contaPulso;   //Variável para a quantidade de pulsos
int i=0;          //Variável para contagem
float litros=0;
int a=0;          //Variável para contagem
int play = 0;
int play2 = 0;
float limite = 0;

/**
 * Check firmware only once at startup
 */
void setup() {
  Serial.begin(9600);   // Inicia a serial
  SPI.begin();          // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522 (Leitor de RFID)

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, incpulso, RISING); //Configura o pino 2(Interrupção 0) para trabalhar como interrupção

  pinMode(porta_rele1, OUTPUT);
  pinMode(porta_botao1, INPUT_PULLUP);
  digitalWrite(porta_rele1, HIGH);
}
  String opc = "";  
  String lim = "";
  String SEC = "";
 
void loop() {
  if (Serial.available() > 0) {
  opc = Serial.readString();
  if(opc.substring(0, 2) == "ON") {
      lim = opc.substring(2, opc.length());
      limite = lim.toFloat();
      play =    1;
      play2 =   1;
  }
}
  if(play == 0) {
    ler_cartao (); 
    }
  else  {
    abrir_valvula (); 
    }
  fluxo_vazao ();  
  } 

void incpulso ()  { 
  contaPulso++; //Incrementa a variável de contagem dos pulsos
}

void ler_cartao ()  {                                         // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {                     // Select one of the cards
    return;
  }
  String conteudo = "";                                       //Mostra UID na serial
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
     Serial.println(conteudo);
  }
      Serial.println();
      conteudo.toUpperCase();
}

void abrir_valvula () {
  BotaoAtu = digitalRead(porta_botao1); 
  if (!BotaoAtu && !BotaoAnt) {
    if( play2 > 0 ) {
      tempo =0;
      digitalWrite(porta_rele1, LOW);
    }
  }
  else  {
    digitalWrite(porta_rele1, HIGH); 
    tempo++;
    if(tempo >= 6)  {      
      play= 0;
      vazao = 0; 
      litros = 0; 
      tempo = 0; 
      lim = "";
      limite = 0;
      play2 =0;
      Serial.println("OFF");
    }  
  }
  BotaoAnt = BotaoAtu;
}

void fluxo_vazao () {
  contaPulso = 0;                 //Zera a variável para contar os giros por segundos
  sei();                          //Habilita interrupção
  delay (500);                    //Aguarda meio segundo
  vazao = contaPulso / 5.5 / 60;  //Converte para L/seg
  litros= litros+vazao;
  i++;
  if(litros > 0.00 && limite > 0) {    
    if(litros == aux) {
      a++;
    }
    else  {
      a = 0;
      aux = litros;
    }
    if(litros > limite) {
      Serial.println(limite);
      a = 4;
      limite = 0;
      play2 = 0;
      digitalWrite(porta_rele1, HIGH);      //Desliga o fluxo
    } 
    if(a < 3) {              
      Serial.println(litros);
    }
  }
}
