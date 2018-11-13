#include <Wire.h>

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

//***déclaration variables globales***//
float fA0;
float fB1;
float fB2;
float fC12;
float P, T;
//***********************//

//***fonction acquisition température et pression***//
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

  Serial.print("Pression atmosphérique : ");
  Serial.print(P);
  Serial.println("kPa");
  Serial.print("Température ambiante : ");
  Serial.print(T);
  Serial.println("C°");
}

//***séquence de démarrage***//
void setup() {
  Wire.begin();        // join i2c bus
  Serial.begin(9600);  // start serial for output

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
  Serial.print(fA0, DEC);
  Serial.print("\n");

  parametreB1.cb1[1] = Wire.read();             //lecture trame MSB
  parametreB1.cb1[0] = Wire.read();             //lecture trame LSB
  fB1 = (float)parametreB1.ib1 / 8192.0 ;       //division par 2^13
  Serial.print(fB1, DEC);
  Serial.print("\n");

  parametreB2.cb2[1] = Wire.read();             //lecture trame MSB
  parametreB2.cb2[0] = Wire.read();             //lecture trame LSB
  fB2 = (float)parametreB2.ib2 / 16384.0 ;      //division par 2^14
  Serial.print(fB2, DEC);
  Serial.print("\n");

  parametreC12.cc12[1] = Wire.read();           //lecture trame MSB
  parametreC12.cc12[0] = Wire.read();           //lecture trame LSB
  parametreC12.ic12 = (parametreC12.ic12) >> 2; // décalage de 2bits vers la droite
  fC12 = (float)parametreC12.ic12 / 4194304.0;  //division par 2^(13+9)
  Serial.print(fC12, DEC);
  Serial.print("\n");
}

void loop() {

  getPT();

  delay(1000);
}
