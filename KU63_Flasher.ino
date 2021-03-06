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
byte command[170];                                                  //Array für Befehle
byte file[170];                                                   //Array für Speicherblock
unsigned int Startadresse;
unsigned int Endadresse;
unsigned int Zaehler=0;                                              //Schleifenzähler für Blockadressierung
unsigned int checksum;
unsigned int i;                                                     //Zähler für for-Schleife
unsigned int Block=15;                                                     //Block in den geschrieben werden soll
byte receivedbyte;
int laenge = 256;


void setup() {

    //array initialisieren erst mal "Reset" Befehl
    command[0]=0x01;                                              //SOH
    command[1]=0x01;                                              //LEN
    command[2]=0x00;                                              //COM = Reset
    command[3]=0-(command[1]+command[2]);                         //Checksum
    command[4]=0x03;                                              //ETX
    
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
    
    digitalWrite(Pullup_pin, LOW);                               //Anstelle von TOOL0
    //pinMode(Pullup_pin, INPUT);                               //Pullup an TOOL0 auf Floating
    delayMicroseconds(595);                                   // Nach Quittung vom Target t01 warten sollen 120 sein
   
    //Serielle Kommunikation mit Target starten
                                       
    Serial1.begin(9600,SERIAL_8N2);                               // Kommunikation mit Target starten
    Serial1.print(0, BYTE);
    delayMicroseconds(10);
    Serial1.print(0, BYTE);
    delayMicroseconds(300);                                       //war 300
    delay(10);
    Serial1.write(command, 5);                                    //Reset senden
    
    delayMicroseconds(595);                                       // der ist hier auf verdacht

    delay(10);
}

void loop() {
  
 //Kommunikation 
    if (Serial2.available()){
    
        receivedbyte=Serial2.read();                                        //Byte einlesen
        Serial.print(receivedbyte, BYTE);                                        //Byte an USB ausgeben 
        
        
    }  
                                    
 
    
//Über Timerinterrupt gesteuerter Hauptschleifenteil
    if (tic){
      tic=LOW;

     if (Serial.available()){
      byte receivedSerial=Serial.read();
      
      switch (receivedSerial) {
        case 1:
          Serial.print("Block Blank Check Block ");
          Serial.println(Block);
          
            Startadresse=Block*0x0400;
            Endadresse=Startadresse+0x03FF;
            command[0]=0x01;                                        //SOH
            command[1]=0x07;                                        // LEN = 7
            command[2]=0x32;                                        //COM = Block Blank Check
            command[5]=Startadresse & 0xFF;                         //Byteweise Start- und Endadresse setzen
            command[4]=Startadresse>>8 & 0xFF;
            command[3]=Startadresse>>16 & 0xFF;
            command[8]=Endadresse & 0xFF;
            command[7]=Endadresse>>8 & 0xFF;
            command[6]=Endadresse>>16 & 0xFF;
            checksum=command[1]+command[2]+command[3]+command[4]+command[5]+command[6]+command[7]+command[8];
            command[9]=0-(checksum & 0xFF);                         //Checksum 0- letzte acht bit der Summe
            command[10]=0x03;                                       //ETX
            Serial1.write(command, 11);
            
          
          break;
        case 2:
          Serial.println("Upload der Parameterdatei, Senden Sie nun die Parameter-Datei");
          for (i=0; i < 161; i++){
           while(!Serial.available()){}                             //Auf Daten warten
           if (Serial.available()){
               file[i]=Serial.read(); 
               Serial.print(file[i], BYTE);
               Serial.println(i);
           } 
          } //Ende der for-Schleife
          Serial.println("Upload abgeschlossen!");
          break;
        case 3: //prepare writing
            Startadresse=Block*0x0400;
            Endadresse=Startadresse+0x03FF;
            command[0]=0x01;                                        //SOH
            command[1]=0x07;                                        // LEN = 7
            command[2]=0x40;                                        //COM = Program
            command[5]=Startadresse & 0xFF;                         //Byteweise Start- und Endadresse setzen
            command[4]=Startadresse>>8 & 0xFF;
            command[3]=Startadresse>>16 & 0xFF;
            command[8]=Endadresse & 0xFF;
            command[7]=Endadresse>>8 & 0xFF;
            command[6]=Endadresse>>16 & 0xFF;
            checksum=command[1]+command[2]+command[3]+command[4]+command[5]+command[6]+command[7]+command[8];
            command[9]=0-(checksum & 0xFF);                         //Checksum 0- letzte acht bit der Summe
            command[10]=0x03;                                       //ETX
            
            Serial1.write(command, 11);
            
            
            while (Serial2.available()){                             //Eigenen Befehl anzeigen (one Wire)
             
              receivedbyte=Serial2.read();                                        //Byte einlesen
              Serial.print(receivedbyte, BYTE);                                        //Byte an USB ausgeben 
              delayMicroseconds(595);
              }
            
           
         
         receivedbyte=0;
         Zaehler =0;
         while (receivedbyte != 0x03){                             //auf Quittung vom Target warten
           if (Serial2.available()){    
              receivedbyte=Serial2.read();                                        //Byte einlesen
              Serial.print(receivedbyte, BYTE);                                        //Byte an USB ausgeben 
              }
            delayMicroseconds(595);
            Zaehler++;
            if (Zaehler>10000){
              Serial.println("Time Out! Target antwortet nicht");
              receivedbyte = 0x03;
            }//ende if
         }
            
            Serial.println("Flashvorgang initialisiert!"); 
            delayMicroseconds(120);                                 // TFD3        
                                                  
            command[0]=0x02;                                        //Data Frame
            command[1]=laenge;                                         //DataLEN = 161
            checksum=laenge;
            for (int i=0; i < laenge; i++){ 
              command[i+2]=file[i]; 
              checksum +=file[i];  
                       
            }
            command[laenge+2]=0-(checksum & 0xFF);
            command[laenge+3]=0x03;                                       //ETX
            Serial1.write(command, laenge+4);
            
            while (Serial2.available()){                             //Eigenen Befehl anzeigen (one Wire)
             
              receivedbyte=Serial2.read();                                        //Byte einlesen
              Serial.print(receivedbyte, BYTE);                                        //Byte an USB ausgeben 
              delayMicroseconds(595);
              }   
          
          receivedbyte=0;
          Zaehler =0;
         while (receivedbyte != 0x03){                             //auf Quittung vom Target warten
           if (Serial2.available()){    
              receivedbyte=Serial2.read();                                        //Byte einlesen
              Serial.print(receivedbyte, BYTE);                                        //Byte an USB ausgeben 
              }
            Zaehler++;
            delayMicroseconds(595);
            if (Zaehler>10000){
              Serial.println("Time Out! Target antwortet nicht");
              receivedbyte = 0x03;
            }//ende if
              
         }  //Ende while 
         delay(200);       
          Serial.println("Flashvorgang abgeschlossen!");
          break; 
         
            
           
         case 4: // Block auswählen 
          Serial.println("Geben sie die gewünschte Block-Nummer ein");
         
           while(!Serial.available()){}                             //Auf Daten warten
           if (Serial.available()){
               Block=Serial.read(); 
               } //Ende if
          Serial.print("gewählter Block: ");
          Serial.println(Block);
         break;
        case 5: //Block erase
            Startadresse=Block*0x0400;
            Endadresse=Startadresse+0x03FF;
            command[0]=0x01;                                        //SOH
            command[1]=0x07;                                        // LEN = 7
            command[2]=0x22;                                        //COM = Block Erase
            command[5]=Startadresse & 0xFF;                         //Byteweise Start- und Endadresse setzen
            command[4]=Startadresse>>8 & 0xFF;
            command[3]=Startadresse>>16 & 0xFF;
            command[8]=Endadresse & 0xFF;
            command[7]=Endadresse>>8 & 0xFF;
            command[6]=Endadresse>>16 & 0xFF;
            checksum=command[1]+command[2]+command[3]+command[4]+command[5]+command[6]+command[7]+command[8];
            command[9]=0-(checksum & 0xFF);                         //Checksum 0- letzte acht bit der Summe
            command[10]=0x03;                                       //ETX
            Serial1.write(command, 11);
            Serial.println("Block Erased!");    
           break;
        default:
          Serial.println("default");
        break;
        }//end of switch structure
      
      } //end of Serial available
 
      digitalWrite(Blink, !digitalRead(Blink));              //Toggle LED
    }                                                       //Ende des über Timerinterrupt gesteuerten Schleifenteils
}                                                                 //Ende der Hauptschleife


// Interruptroutinen
void timetic(void) {
   tic=HIGH;
} 


