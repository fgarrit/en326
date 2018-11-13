#include <Wire.h>

#define lum A0  //Broche capteur lum
#define DHT11 3 //Broche capteur humidité/temp


//***déclaration variables globales***//
float fA0;
float fB1;
float fB2;
float fC12;
float P, T;
//***********************//

//***Fonctions capteurs***//

//luminosité
float lecture_lum(void) {
  float val_lum;
  val_lum = 1 / 0.05 * analogRead(lum) * 5 / 1023; //Conversion
  return val_lum;
}

//temp humidité
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
  while (!(digitalRead(DHT11))) {  // Attente d'un état LOW
    time_end = millis() - time_begin;  // Calcul durée d'attente
    if (time_end > 1000) {   // Pas de réponse du capteur après 1000ms
      return 1;
    }
  }

  time_begin = millis(); // Stockage du temps
  while (digitalRead(DHT11)) { // Attente d'un état HIGH
    time_end = millis() - time_begin; // Calcul durée d'attente
    if (time_end > 1000) {  // Pas de réponse du capteur après 1000ms
      return 1;
    }
  }


  for (byte i = 0; i < 40; ++i) {  // Lecture des données du capteur (40 bits)

    time_begin = millis();  // Stockage du temps
    while (!(digitalRead(DHT11))) {  // Attente d'un état LOW
      time_end = millis() - time_begin;  // Calcul durée d'attente
      if (time_end > 1000) {   // Pas de réponse du capteur après 1000ms
        return 1;
      }
    }

    time_begin = micros();  // Stockage du temps de début
    while (digitalRead(DHT11)); // Attente de fin de réception du bit
    time_end = micros();  // Stockage du temps de fin

    data[i / 8] <<= 1;    // Décallage d'un bit
    if ((time_end - time_begin) > 45) {   // Calcul de la durée (si supérieur à 45us, bit = 1, par défaut bit = 0)
      data[i / 8] |= 1;    // Mise à 1 du bit
    }
  }

  byte checksum = (data[0] + data[1] + data[2] + data[3]) & 0xff;  // Calcul du checksum
  if (data[4] != checksum) {
    return 2;  // Erreur du checksum
  }
  else { // Pas d'erreur
    *humidity = data[0];  // Stockage de l'humidité
    *temperature = data[2];  // Stockage de la temperature
    return 0;
  }
}

//pression temp
void getPT() {
  //déclaration des variables locales
  float pres, temp;
  float pressureComp;

  //récupération des données pression et température non-ajustées
  Wire.beginTransmission(0x60);          //adresse du capteur : 0x06
  Wire.write(0X12);                      //start conversion ADC
  Wire.write(0x00);                      //Read pressure MSB
  Wire.endTransmission();

  delay(5); //Attente de la fin de la conversion (mini 3ms)

  Wire.beginTransmission(0X60);         //adresse du capteur : 0x06
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(0x60, 4);                            //demande 4 registres à partir de l'adresse 0x06
  pres = ((  Wire.read() << 8) | (Wire.read())) >> 6;   //MSB = 8bits LSB=2bits
  temp = (( Wire.read() << 8) | (Wire.read())) >> 6;    //MSB = 8bits LSB=2bits

  //calcul correction avec paramètres a0, b1, b2 et c12
  pressureComp = fA0 + (fB1 + fC12 * temp ) * pres + fB2 * temp;

  // Return pressure and temperature as floating point values
  P = ((65.0 / 1023.0) * pressureComp) + 50.0;        // kPa
  T = ((float) temp - 498.0) / -5.35 + 25.0;           // C

  //  Serial.print("Pression atmosphérique : "); //écriture pour développement
  //  Serial.print(P);
  //  Serial.println("kPa");
  //  Serial.print("Température ambiante : ");
  //  Serial.print(T);
  //  Serial.println("C°");
}
//***********************//

//****structures pour regrouper MSB et LSB dans un meme vecteur****//
union {
  int ia0;
  char ca0[2];
} parametreA0 ;

union {
  int ib1;
  char cb1[2];
} parametreB1 ;

union {
  int ib2;
  char cb2[2];
} parametreB2 ;

union {
  int ic12;
  char cc12[2];
} parametreC12 ;

union {
  int ip;
  char cp[2];
} pADC ;

union {
  int it;
  char ct[2];
} tADC ;
//***********************//

//***séquence de démarrage***//
void setup() {
  Wire.begin();        // join i2c bus
  Serial.begin(9600);  // start serial for output
  pinMode(DHT11, OUTPUT);  // Broche du capteur en entrée température/humidité

  //initialisation des variables
  parametreA0.ia0 = 0;
  parametreB1.ib1 = 0;
  parametreB2.ib2 = 0;
  parametreC12.ic12 = 0;
  fA0 = 0;
  fB1 = 0;
  fB2 = 0;
  fC12 = 0;


  Wire.beginTransmission(0x60);   // début transmission sur adresse esclave
  Wire.write(0x04);
  Wire.endTransmission();
  Wire.requestFrom(0x60, 8);      // request sur adresse 1er reg coeff, 8 octets demandés

  parametreA0.ca0[1] = Wire.read();             //lecture trame MSB
  parametreA0.ca0[0] = Wire.read();             //lecture trame LSB
  fA0 = (float)parametreA0.ia0 / 8.0 ;          //division par 2^3
//  Serial.print(fA0, DEC);
//  Serial.print("\n");

  parametreB1.cb1[1] = Wire.read();             //lecture trame MSB
  parametreB1.cb1[0] = Wire.read();             //lecture trame LSB
  fB1 = (float)parametreB1.ib1 / 8192.0 ;       //division par 2^13
//  Serial.print(fB1, DEC);
//  Serial.print("\n");

  parametreB2.cb2[1] = Wire.read();             //lecture trame MSB
  parametreB2.cb2[0] = Wire.read();             //lecture trame LSB
  fB2 = (float)parametreB2.ib2 / 16384.0 ;      //division par 2^14
//  Serial.print(fB2, DEC);
//  Serial.print("\n");

  parametreC12.cc12[1] = Wire.read();           //lecture trame MSB
  parametreC12.cc12[0] = Wire.read();           //lecture trame LSB
  parametreC12.ic12 = (parametreC12.ic12) >> 2; // décalage de 2bits vers la droite
  fC12 = (float)parametreC12.ic12 / 4194304.0;  //division par 2^(13+9)
//  Serial.print(fC12, DEC);
//  Serial.print("\n");
}
//***********************//

void loop() {

  //déclaration variables
  float temperature, humidity;
  unsigned long start_time = 18;
  unsigned long time_out = 1000;
  float lumiere;

  //appels des fonctions
  lumiere = lecture_lum();
  getPT();
  switch (readDHT11(DHT11, &temperature, &humidity, start_time, time_out)) {
    case 0: // Affichage
      Serial.print(F("Humidite (%): "));
      Serial.println(humidity, 2);
      Serial.print(F("Temperature (^C): "));
      Serial.println(temperature, 2);
      break;

    case 1: // Le capteur ne se réveil pas
      Serial.println(F("Pas de reponse !"));
      break;

    case 2: // La trame reçu n'est pas correct
      Serial.println(F("Pb de communication !"));
      break;
  }

  //affichage des données
  Serial.print(F("Luminosité (lux): "));
  Serial.print(lumiere);
  Serial.print("\n");
  Serial.print("Pression atmosphérique : ");
  Serial.print(P);
  Serial.println("kPa");
  Serial.print("Température ambiante : ");
  Serial.print(T);
  Serial.println("C°");


  delay(1000); // Une mesure par seconde
}
