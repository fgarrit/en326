#define lum A0 
#define DHT11 3

byte dht11_lecture(byte* data){
   data[0] = data[1] = data[2] = data[3] = data[4] = 0;
   unsigned long timeout = 50;
   unsigned long timeout1;
   unsigned long timeout2;
  
   digitalWrite(DHT11, HIGH);
   pinMode(DHT11, INPUT);
   delay(100);
  
  // Reveil capteur
   pinMode(DHT11, OUTPUT);
   digitalWrite(DHT11, LOW);
  
   delay(18);
   noInterrupts();

  // Ecoute
   digitalWrite(DHT11, HIGH);
   delayMicroseconds(40);
   pinMode(DHT11, INPUT);
  
  
  // Attente réponse
   while(!digitalRead(DHT11)){ //Attente 1
 
   }
  for (byte i = 0; i < 40; ++i) {
   timeout1 = micros(); //Mesure temps zéro

   while(digitalRead(DHT11)){

   }
    
   timeout2 = micros(); //Mesure temps fin du bit
   data[i / 8] <<= 1;
   if (timeout2 - timeout1 > timeout)  data[i / 8] |= 1;
   else data[i / 8] |= 0;
   
   }
   
}

float lecture_lum(void){
  float val_lum;
  val_lum = 1/0.05*analogRead(lum)*5/1023; //Conversion    
  return val_lum;
}

void setup() {
Serial.begin(115200);   
pinMode(DHT11, INPUT_PULLUP);
}

void loop() {
  Serial.println("-----START-----");
  float temperature, humidity;
  byte data[5];
  dht11_lecture(data);
  humidity = data[0];
  temperature = data[2];
    Serial.print(F("Humidite (%): "));
    Serial.println(data[1], 2);
    Serial.print(F("Temperature (^C): "));
    Serial.println(temperature, 2);
    Serial.println("-----STOP----------");
    delay(1000);


  
 // Serial.print(lumiere);
  //Serial.print("\n");
}
