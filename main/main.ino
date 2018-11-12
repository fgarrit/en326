#define lum A0 
#define dht11 2

float lecture_dht11(int state){
   String code = "INIT";
   
   switch (code)
   { 
    case INIT: 
                Serial.println("INIT"); 
                digitalWrite(dht11, HIGH);
                if (state) case = "SINGLEBUS"
                break;
                
   case SINGLEBUS: 
                Serial.println("SINGLEBUS"); 
                digitalWrite(dht11, LOW);
                delay(20);
                case = "PULLUP"
                break;
                
   case PULLUP: 
                Serial.println("PULLUP"); 
                digitalWrite(dht11, LOW);
                delay(20);
                case = "PULLUP"
                break;

    default: 
                Serial.println("Erreur inconnue"); 
                break;
   }
}

float lecture_lum(void){
  float val_lum;
  val_lum = 1/0.05*analogRead(lum)*5/1023; //Conversion    
  return val_lum;
}

void setup() {
Serial.begin(9600);   
}

void loop() {
  float lumiere;
  lumiere = lecture_lum();
  Serial.print(lumiere);
  Serial.print("\n");
}
