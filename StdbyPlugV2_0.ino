// TODO:
//   - Pour calculer le threshold, ne pas prendre le milieu des 2 permières planètes mais considérer le rayon de la sphère d'influence de la planète (min et max) pour éviter de tomber trop près d'une des planètes.
//   - Si erreur en MODE_OFF, resauvegarder l'etat du relais apres 3 minutes

#define _USE_EEPROM_  //Activer pour sauvegarder les données dans la ROM
//#define _DEBUG_    //Activer pour avoir des traces sur le port série

#include "LatchingRelay.h"
#include "CurrentSensor.h"

#ifdef _USE_EEPROM_
#include <EEPROM.h>
#endif

#define DELAY_OFF 900000 //ms = 15 minutes
#define LED_FREQ 1000 //ms
//#define FREQ_ACQUISITION 270000 //ms  => setup cycle = 18 heures
#define FREQ_ACQUISITION 90000 //ms  => setup cycle = 6 heures

#define MODE_VEILLE 1
#define MODE_ON 2
#define MODE_OFF 3
#define MODE_SETUP 4
#define MODE_PERPET 5

#define pinPontH1 2
#define pinPontH2 4
#define pinBouton 0
#define pinCurrentSensor A3
#define pinLED 1

#define ADDRESS_NBVALUE 2
#define ADDRESS_CHECK 300
#define ADDRESS_RELAY 1
#define ADDRESS_VAL 3

#define SIZE_TAB 240

const byte RELAY_ON = 2;
const byte RELAY_OFF = 1;

boolean bLedOn = false;
unsigned long lastBlinkTime = 0;
unsigned long veilleStartTime = 0;
unsigned long offStartTime = 0;
unsigned long lastMeasure = 0;
byte Mode = MODE_OFF;
byte Mesures[SIZE_TAB];
byte iThresholdOn = 0;
byte iNbSavedValue = 0;
const byte eepromValid = 249;

LatchingRelay relay(pinPontH1, pinPontH2, true);
CurrentSensorAC currentSensor(pinCurrentSensor);

void setup() {
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, HIGH);
    pinMode(pinBouton, INPUT_PULLUP);
    lastBlinkTime = millis();
#ifdef _DEBUG_
    Serial.begin(9600);
#endif    
    for(byte i=0; i<SIZE_TAB; i++) Mesures[i] = 0;
    
#ifdef _USE_EEPROM_   
    if(EEPROM.read(ADDRESS_CHECK) == eepromValid)
    {
      if(EEPROM.read(ADDRESS_RELAY) == RELAY_OFF) relay.SetStatus(false);
     
      iNbSavedValue = EEPROM.read(ADDRESS_NBVALUE);
#ifdef _DEBUG_    
      Serial.print("READ iNbSavedValue = ");
      Serial.println(iNbSavedValue);
#endif
      for(byte i=0; i<iNbSavedValue; i++)
      {
        Mesures[i] = EEPROM.read(i+ADDRESS_VAL);
      }
    }

#endif
    Mode = MODE_SETUP;
    lastMeasure = millis();
}

void loop() {
      float courant = 0;
      if(Mode!=MODE_OFF) 
      {
        courant = currentSensor.GetCurrentByte();
      }

      if(Mode==MODE_PERPET) 
      {
        if (bLedOn && millis()-lastBlinkTime>LED_FREQ/10){ 
          digitalWrite(pinLED, LOW);
          lastBlinkTime = millis();
          bLedOn = false;
        }
        if (!bLedOn && millis()-lastBlinkTime>LED_FREQ){ 
          digitalWrite(pinLED, HIGH);
          lastBlinkTime = millis();
          bLedOn = true;
        } 
        
        //Si on appuie sur le bouton on repasse en mode ON
        if(digitalRead(pinBouton)==LOW){
          Mode = MODE_ON;
#ifdef _DEBUG_
    Serial.println("\nMODE = ON\n");
#endif          
          lastBlinkTime = millis();
          //Wait for button is released
          while (digitalRead(pinBouton)==LOW) delay(100);
          lastBlinkTime = millis();
        }
      }
            
      if(Mode==MODE_ON) 
      {
        digitalWrite(pinLED, LOW);
        bLedOn = true;

        //Si on appuie sur le bouton on passe en mode perpetuel
        if(digitalRead(pinBouton)==LOW){
          Mode = MODE_PERPET;
#ifdef _DEBUG_
    Serial.println("\nMODE = PERPET\n");
#endif          
          //Wait for button is released
          while (digitalRead(pinBouton)==LOW) delay(100);
        }
        
        else if(courant<iThresholdOn)
        {
          Mode = MODE_VEILLE;
#ifdef _DEBUG_
    Serial.println("\nMODE = VEILLE\n");
#endif
          veilleStartTime = millis();
        }
        lastBlinkTime = millis();
      }
      
      if(Mode==MODE_VEILLE) 
      {
        if (bLedOn && millis()-lastBlinkTime>LED_FREQ){ 
          digitalWrite(pinLED, LOW);
          lastBlinkTime = millis();
          bLedOn = false;
        }
        if (!bLedOn && millis()-lastBlinkTime>LED_FREQ/10){ 
          digitalWrite(pinLED, HIGH);
          lastBlinkTime = millis();
          bLedOn = true;
        } 
        
        if(courant>iThresholdOn){
          Mode = MODE_ON;
#ifdef _DEBUG_
    Serial.println("\nMODE = ON\n");
#endif           
        }
        else if(millis() - veilleStartTime > DELAY_OFF)
        {
          Mode = MODE_OFF;
#ifdef _DEBUG_
    Serial.println("\nMODE = OFF\n");
#endif    
          SaveStateRelay();
          relay.Switch(); 
          offStartTime = millis();   
          digitalWrite(pinLED, HIGH);  
          bLedOn = false;
        }
        //Si on appuie sur le bouton on passe en mode SETUP
        else if(digitalRead(pinBouton)==LOW){
          Mode = MODE_SETUP;
          iNbSavedValue = 0;
#ifdef _DEBUG_
    Serial.println("\nMODE = SETUP\n");
#endif          
          lastBlinkTime = millis();
          //Wait for button is released
          while (digitalRead(pinBouton)==LOW) delay(100);
        }
      }
      
      if(Mode==MODE_OFF) 
      {
        //To initiate the relay in the right state
        if(digitalRead(pinBouton)==LOW){
          SaveStateRelay();
          relay.SetStatus(!relay.GetStatus());
          relay.Switch();
          //Wait for button is released
          while (digitalRead(pinBouton)==LOW) delay(100);
        }
        //If after 3 minutes off, it is still powered => Blink error message
        else if (millis()-offStartTime>180000)
        {
          if (bLedOn && millis()-lastBlinkTime>LED_FREQ/10){ 
            digitalWrite(pinLED, LOW);
            lastBlinkTime = millis();
            bLedOn = false;
          }
          if (!bLedOn && millis()-lastBlinkTime>LED_FREQ/10){ 
            digitalWrite(pinLED, HIGH);
            lastBlinkTime = millis();
            bLedOn = true;
          } 
        }
        //If after 2 minutes off, it is still powered => try to switch relay
        else if (millis()-offStartTime>120000)
        {
          SaveStateRelay();
          relay.SetStatus(!relay.GetStatus());
          relay.Switch();
          delay(65000);
        }
        //If after 1 minute off, it is still powered => repeat again last state of relay
        else if (millis()-offStartTime>60000)
        {
          if (relay.GetStatus()) relay.TurnOn();
          else relay.TurnOff();
          delay(65000);
        }
      }

      if(Mode==MODE_SETUP) 
      {
        if (bLedOn && millis()-lastBlinkTime>LED_FREQ/4){ 
          digitalWrite(pinLED, LOW);
          lastBlinkTime = millis();
          bLedOn = false;
        }
        if (!bLedOn && millis()-lastBlinkTime>LED_FREQ/4){ 
          digitalWrite(pinLED, HIGH);
          lastBlinkTime = millis();
          bLedOn = true;
        } 

        if (millis()-lastMeasure>FREQ_ACQUISITION)
        {
          AddMesure(courant);
          lastMeasure = millis();
        }
        
        if (iNbSavedValue>=SIZE_TAB)
        {
          UpdateThresholds();
          if(iThresholdOn > 0) 
          {
            Mode = MODE_VEILLE;
            veilleStartTime = millis();
            lastBlinkTime = millis();
#ifdef _USE_EEPROM_ 
            EEPROM.write(ADDRESS_CHECK+1, iThresholdOn);            
#endif            
#ifdef _DEBUG_
            Serial.println("\nMODE = VEILLE\n");
#endif  
          }
          else
          {
            iNbSavedValue = 0;
          }
        }
      }
}

//Save the opposite status as as soon as the relay will switch we loose power
void SaveStateRelay()
{
#ifdef _USE_EEPROM_  
  if (relay.GetStatus()) 
    EEPROM.write(ADDRESS_RELAY, RELAY_OFF);
  else
    EEPROM.write(ADDRESS_RELAY, RELAY_ON);
#endif
  delay(1000);
}

void AddMesure(byte iValue)
{
#ifdef _DEBUG_
  Serial.println(iValue);      
#endif
  Mesures[iNbSavedValue] = iValue;

#ifdef _USE_EEPROM_
  EEPROM.write(iNbSavedValue + ADDRESS_VAL, iValue);
#endif  

  if(iNbSavedValue<SIZE_TAB){
    iNbSavedValue++;
#ifdef _USE_EEPROM_
    EEPROM.write(ADDRESS_NBVALUE, iNbSavedValue);
    EEPROM.write(ADDRESS_CHECK, eepromValid);
#endif    
#ifdef _DEBUG_    
  Serial.print("WRITE iNbSavedValue = ");
  Serial.println(iNbSavedValue);
#endif
  }
}

void UpdateThresholds()
{
  byte minValue = 255;
  byte maxValue = 0;

  for(byte i=0;i<SIZE_TAB;i++)
  {
    if(Mesures[i]>maxValue) maxValue = Mesures[i];
    if(Mesures[i]<minValue) minValue = Mesures[i];
  }
  UpdateThresholds (minValue, maxValue);
}

//Call this function with the minimum measured value and the maximum
//Algo:
// Range les mesures dans 20 groupes 
// Regroupe les valeurs des groupes par un calcul de gravité. Plus les groupes sont prêts et gros plus ils s'attirent et fusionnent
void UpdateThresholds(int iStart, int iEnd)
{
  byte aDistriNbVal[20];
  byte iNbPlanetes = 0;

  float iDelta = (iEnd - iStart) / 20.0;

  for (byte i = 0; i<20; i++)
  {
    aDistriNbVal[i] = 0;
  }

  //Store nb measue as well as max and min value for each interval
  for (byte j = 0; j<SIZE_TAB; j++)
  {
    //Calcul dans quel groupe se trouve chaque mesure
    byte iCell = floor((float)(Mesures[j] - iStart) / iDelta);
    if (iCell >= 20) iCell--;
    aDistriNbVal[iCell]++;
  }

  for (byte k = 0; k<20; k++)
  {
    if (aDistriNbVal[k] > 1) iNbPlanetes++;
  }

  for (byte k = 0; k < 20; k++)
  {
    if (aDistriNbVal[k] > 0 && iNbPlanetes>2)
    {
      float iGravMax = 0;
      byte iPlaneteNumber = -1;
      //Trouve la planète exerçant la plus forte attraction (iNbGrp)
      for (byte l = 0; l < 20; l++)
      {
        if (l != k && aDistriNbVal[l] > 1)
        {
          float fGrav = aDistriNbVal[l] * aDistriNbVal[k] / ((k - l) * (k - l) );
          if (fGrav > iGravMax)
          {
            iGravMax = fGrav;
            iPlaneteNumber = l;
          }
        }
      }
      //On a trouvé la planète la plus attractive
      if (iPlaneteNumber > -1)
      {
        // Si la planète en cours est plus grosse que celle qui l'attire le plus, on aspire la planète
        if (aDistriNbVal[k] > aDistriNbVal[iPlaneteNumber])
        {
          aDistriNbVal[k] += aDistriNbVal[iPlaneteNumber];
          aDistriNbVal[iPlaneteNumber] = 0;
        }
        //sinon on est aspiré
        else
        {
          aDistriNbVal[iPlaneteNumber] += aDistriNbVal[k];
          aDistriNbVal[k] = 0;
        }
        //Dans tous les cas on a détruit une planète
        iNbPlanetes--;
      }
    }
  }
  
  //On a trouve au moins deux planètes => le threshold est entre la première (veille) et la deuxième
  if (iNbPlanetes >= 2)
  {
    byte iPlanete1 = 255;
    byte iPlanete2 = 255;
    byte k = 0;
    //On cherche les deux planètes
    while (k<20 && (iPlanete1 == 255 || iPlanete2 == 255))
    {
      if (aDistriNbVal[k] > 1) 
      {
        if (iPlanete1 == 255) iPlanete1 = k;
        else if (iPlanete2 == 255) iPlanete2 = k;
      }
      k++;
    }
    //Si on a trouvé les 2 premières planètes on calcule l'écart entre les 2 et on se place à 1/3 de la premiere pour être sûr.
    if (iPlanete1 != 255 && iPlanete2 != 255) iThresholdOn = iStart + iDelta*(2*iPlanete1+iPlanete2)/3;
  }
}
