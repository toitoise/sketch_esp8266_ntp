
//===================================================================================================
//
// Sketch pour programmer l'heure du RTC ds3231 avec un ESP01
// Utilisation de la connexion WIFI et l'acces au serveyr NTP
// pour recuperer un horaire UTC universel sans les changements horaires.
//
// On en profite pour faire un petit serveur web avec l'ESP01 qui affiche la date+heure
//
// Notes:
//    On utilise les pins I2C (GPIO0 et GPIO2 de l'ESP01).
//    Le ds3231 fonctionne de 3 à 5 volts et l'i2c fonctionne bien en 3.3v de l'ESP
//    Il y a des pullup sur l'i2c, pas besoin d'en rajouter
//===================================================================================================

#include <ESP8266WiFi.h>            // Pour connexion wifi
#include <NTPClient.h>              // Pour client NTP
#include <WiFiUdp.h>                // Support UDP
#include <Wire.h>                   // Pour I2C
#include "RTC_ST.h"                 // Pour manip Date et RTC DS3231

//-------------------------------------------
// DS3231
//-------------------------------------------
static DS3231 RTC;
const int PIN_SCL = 2; //for ESP8266
const int PIN_SDA = 0; //for ESP8266

//-------------------------------------------
// Variables pour l'heure et la date :
//-------------------------------------------
byte heure          = 14;     // 0 à 23
byte minute         = 54;     // 0 à 59
byte seconde        = 30;      // 0 à 59
byte numJourSemaine = 6;      // dim = 1, sam = 7
byte numJourMois    = 7;      // 1 à 31
byte mois           = 5;      // 1 à 12
uint16_t annee          = 21;     // 0 à 99

//-------------------------------------------
// WIFI setup/Mettre les identifiant dans un fichier annexe non géré sous Git
// et qui contient juste les 2 lignes suivantes.
//-------------------------------------------
//const char *wifi_ssid     = "";
//const char *wifi_password = "";
#include "identifiants.h"

//-------------------------------------------
// Define NTP Client to get time
//-------------------------------------------
#define utcOffsetInSeconds  0 //utc
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//-------------------------------------------
// SETUP
//-------------------------------------------
void setup() {
  Serial.begin(115200);

  //------------------
  // WIFI opening
  //------------------
  Serial.println("");
  WiFi.begin(wifi_ssid, wifi_password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  // Print local IP address
  Serial.println("WiFi connected at IP address:");
  Serial.println(WiFi.localIP());

  //------------------
  // Check DS3231
  //------------------
  Wire.begin(PIN_SDA, PIN_SCL);
  RTC.begin();

  //------------------------
  // NTP start
  //------------------------
  timeClient.begin();

  Wire.setClock(400000L);     // Slow I2C Freq : 400KHz
  Serial.println("");
  Serial.println("EndofSetup");
}

//-------------------------------------------
// LOOP
//-------------------------------------------
void loop() {
  Display_RTC_Date(String("RTC Date originelle:"), 0);
  Serial.println("Epoch Time (since 01/01/1970) NTP puis RTC :");
  timeClient.update();        // NTP update time
  time_t epochTime = timeClient.getEpochTime();
  time_t epoch_RTC = RTC.getEpoch();
  Serial.println(epochTime);      // From NTP server
  Serial.println(epoch_RTC);      // From DS3231 RTC

  time_t diff = epochTime - epoch_RTC;

  //----------------------------------------------------
  // If difference is more than 1sec then reprog RTC
  //----------------------------------------------------
  if ( diff > 1 || diff < -1 ) {
    Serial.println("    *** REPROGRAMME RTC TIME ***");
    RTC.setEpoch(epochTime);
  } else {
    Serial.println("    *** NO REPROG RTC NEEDED ***");
  }

  Display_RTC_Date(String("RTC Date apres Prog:"),1);
  delay(10000);
}

//----------------------------------------------------
// Display RTC date func
//----------------------------------------------------
void Display_RTC_Date(String str, byte pos){
  // Get Date RTC apres potentiel reprog
  annee         = RTC.getYear();
  mois          = RTC.getMonth();
  heure         = RTC.getHours();
  minute        = RTC.getMinutes();
  seconde       = RTC.getSeconds();
  numJourSemaine= RTC.getWeek();
  numJourMois   = RTC.getDay();

  if (pos==0) Serial.println("===================================");
  Serial.println(str);
  Serial.print(annee);
  Serial.print(", ");
  Serial.print(mois);
  Serial.print(", ");
  Serial.print(numJourMois);
  Serial.print(", ");
  Serial.print(numJourSemaine);
  Serial.print("  *****  ");
  Serial.print(heure);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(seconde);
  if (pos==1) Serial.println("===================================");
  
}
