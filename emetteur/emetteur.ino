#define DHT11 3 //Broche capteur

void setup() {
  Serial.begin(9600); // Vitesse de communication
  pinMode(DHT11, OUTPUT);  // Broche du capteur en entrée
}

void loop() {
  float temperature, humidity;
  unsigned long start_time = 18;
  unsigned long time_out = 1000;
 
  switch (readDHT11(DHT11, &temperature, &humidity, start_time, time_out)) {
  case 0: // Affichage
    //Serial.print(F("Humidite (%): "));
    //Serial.println(humidity, 2);
    //Serial.print(F("Temperature (^C): "));
    Serial.println(temperature, 2);
    break;
 
  case 1: // Le capteur ne se réveil pas
    Serial.println(F("Pas de reponse !")); 
    break;
 
  case 2: // La trame reçu n'est pas correct
    Serial.println(F("Pb de communication !")); 
    break;
  }

  delay(500); // Une mesure par seconde
  
}

byte readDHT11(byte pin, float* temperature, float* humidity, unsigned long start_time, unsigned long timeout) {
  unsigned long time_begin, time_end;
  byte data[5]; // Trame
  data[0] = data[1] = data[2] = data[2] = data[4] = 0; // Initialisation
 
  digitalWrite(DHT11, HIGH);  // PULLUP 
  delay(100);   
   
  pinMode(DHT11, OUTPUT);  
  digitalWrite(DHT11, LOW); // Réveil du capteur
  delay(start_time); // Temps d'attente pour réveiller le capteur (18ms minimum)
  
    digitalWrite(DHT11, HIGH);  // Passage en écoute
  delayMicroseconds(40);
  pinMode(DHT11, INPUT); 
 
  time_begin = millis();  // Stockage du temps
  while(!(digitalRead(DHT11))) {   // Attente d'un état LOW
  time_end = millis() - time_begin;  // Calcul durée d'attente
      if (time_end > 1000) {   // Pas de réponse du capteur après 1000ms
        return 1;
      }
  }
    
  time_begin = millis(); // Stockage du temps
  while(digitalRead(DHT11)) { // Attente d'un état HIGH
      time_end = millis() - time_begin; // Calcul durée d'attente
      if (time_end > 1000) {  // Pas de réponse du capteur après 1000ms
        return 1;
      }
  }

  
  for (byte i = 0; i < 40; ++i) {  // Lecture des données du capteur (40 bits)
 
    time_begin = millis();  // Stockage du temps
    while(!(digitalRead(DHT11))) {   // Attente d'un état LOW
      time_end = millis() - time_begin;  // Calcul durée d'attente
      if (time_end > 1000) {   // Pas de réponse du capteur après 1000ms
        return 1;
      }
    }

   time_begin = micros();  // Stockage du temps de début
    while(digitalRead(DHT11));  // Attente de fin de réception du bit
    time_end = micros();  // Stockage du temps de fin
    
    data[i / 8] <<= 1;    // Décallage d'un bit
    if ((time_end - time_begin) > 45) {   // Calcul de la durée (si supérieur à 45us, bit = 1, par défaut bit = 0)
      data[i / 8] |= 1;    // Mise à 1 du bit
    }
  }

  byte checksum = (data[0] + data[1] + data[2] + data[3]) & 0xff;  // Calcul du checksum 
  if (data[4] != checksum){
    return 2;  // Erreur du checksum
  }
  else{  // Pas d'erreur
    *humidity = data[0];  // Stockage de l'humidité
    *temperature = data[2];  // Stockage de la temperature
    return 0; 
  }
}
