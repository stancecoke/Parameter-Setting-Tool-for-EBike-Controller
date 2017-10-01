/*
KU63 Programmer
V0.1 29.0.2017
by hochsitzcola
*/

// Definition Konstanten

#define Power_pin PB12                                             // VCC Target
#define TOOL0_pin PA9                                              //Tx UART1 an Tool0 Target
#define RxUART2_pin PA3                                            //Rx UART2 an Tool0 Target (one wire communication)
#define RxUART1_pin PA10                                           //Rx UART1 an Tool0 Target (one wire communication)
#define Reset_pin PB13                                             //Reset Target
#define FLM_pin PB14                                                //FLM an Target
#define Pullup_pin PB15                                             //Hilfspin, da interner Pullup an A9 nicht geht 
#define Blink PC13                                                 //Anschlußpin interne LED
#define TIC_RATE 500000                                              //Timetic in µs

void timetic(void);                                                //interruptroutine für Timetic deklariere´n

//Definition Variablen

boolean tic;                                                      //Flag für Timer2
byte command[4];                                                  //Array für Befehle



void setup() {

    //array initialisieren erst mal "Get Version" Befehl
    command[0]=0x01; //SOH
    command[1]=0x01; //LEN
    command[2]=0x00; //COM = Reset
    command[3]=0x01; //Checksum
    command[4]=0x03; //ETX
    
    //Timer initialisieren
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);                  // Timer2 als einfachen Zähler definieren
    Timer2.setPeriod(TIC_RATE);                                   // in microseconds
    Timer2.setCompare1(1);                                        // overflow might be small
    Timer2.attachCompare1Interrupt(timetic); 
  
    // Ein- und Ausgänge initialieren
    pinMode(Blink, OUTPUT);                                       //LED konfigurieren
    pinMode(Power_pin, OUTPUT);                                   // Flash-Leitungen als Output setzen
    pinMode(TOOL0_pin, INPUT);                                    //TOOL0_pin auf Lesen stellen
    pinMode(RxUART2_pin, INPUT);
    pinMode(RxUART1_pin, INPUT);
    pinMode(Reset_pin, OUTPUT);                                   //
    pinMode(FLM_pin, OUTPUT);                                     //
    pinMode(Pullup_pin, OUTPUT);
    


    digitalWrite(Blink, LOW);                                     // Alle Ausgänge auf Null setzen
    digitalWrite(Power_pin, LOW);
    digitalWrite(Pullup_pin, LOW);
    digitalWrite(Reset_pin, LOW);
    digitalWrite(FLM_pin, LOW);
   
    
    Serial.begin(115200);                                         // Serielle Kommunikation über USB-Port initialisieren
    while (!Serial.available()){}                                 //Auf Startsignal warten
    Serial2.begin(9600);
    // Target im Flash-Mode starten
    
    delay(1000);
    digitalWrite(Power_pin, HIGH);                                //Target einschalten, vergl. PDF File 78K0R/Kx3 16-bit Single-Chip Microcontrollers Flash Memory Programming (Programmer) Figure 2-5
    
    delay(10);
                                
    digitalWrite(FLM_pin, HIGH);
    digitalWrite(Pullup_pin, HIGH);                               //Anstelle von TOOL0
   
    delay(10);
    digitalWrite(Reset_pin, HIGH);
    while (digitalRead(TOOL0_pin)){digitalWrite(Blink, HIGH);}     //warten bis LOW vom Target, So lange LED ausschalten
    while (!digitalRead(TOOL0_pin)){}                              //warten bis HIGH vom Target
    
   
    pinMode(Pullup_pin, INPUT);                               //Pullup an TOOL0 auf Floating
    delayMicroseconds(595);                                   // Nach Quittung vom Target t01 warten sollen 120 sein
   
    //Serielle Kommunikation mit Target starten
                                       
    Serial1.begin(9600,SERIAL_8N2);                               // Kommunikation mit Target starten
    Serial1.print(0, BYTE);
    delayMicroseconds(10);
    Serial1.print(0, BYTE);
    delayMicroseconds(300);
    Serial1.write(command, 5);                                    //Reset senden
    command[2]=0x05; //COM = Reset
    command[3]=0x04; //Checksum
    delayMicroseconds(595);                                       // der ist hier auf verdacht
}

void loop() {
  
 //Kommunikation 
    if (Serial2.available()){
    
        byte receivedbyte=Serial2.read();                                        //Byte einlesen
        Serial.print(receivedbyte, BYTE);                                        //Byte an USB ausgeben 
        
        
    }  
    
//Über Timerinterrupt gesteuerter Hauptschleifenteil
    if (tic){
      tic=LOW;
      Serial1.write(command, 5);
      
      digitalWrite(Blink, !digitalRead(Blink));              //Toggle LED
    }                                                       //Ende des über Timerinterrupt gesteuerten Schleifenteils
}                                                                 //Ende der Hauptschleife


// Interruptroutinen
void timetic(void) {
   tic=HIGH;
} 


