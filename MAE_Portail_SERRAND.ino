/****************************************************************************************************************************************
   Nom ......... : MAE_Portail.ino
   Role..........: MAE de gestion d'un portail d'habitation avec BP d'ouverture et de fermeture, ouverture par carte RFID et détection
                   véhicule génant par ultrason.
   Auteur ...... : Joris SERRAND
   Etablissement : Lycée Louis Rascol, Albi, FRANCE <http://louis-rascol.entmip.fr/>
   Mail ........ : joris.serrand@rascol.net
   Version ..... : V1.1 du 06/09/17
   Licence ..... : Copyright (C) 2018  SERRAND Joris

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>

   Compilation . : Avec l'IDE Arduino
***************************************************************************************************************************************/

//Bibliothèques et paramétrage pour le NFC
//#include "PN532_HSU.h"
// #include "PN532.h"
// #include "NfcAdapter.h"
// PN532_HSU interface(Serial); //Connexion du module Grove NFC sur le serial 1
// NfcAdapter nfc = NfcAdapter(interface);
// String const myUID = "7A F3 96 34"; // Définition du code UID qui garantie l'autorisation

//Bibliothèques et paramétrage pour le module ultrason
#include "Ultrasonic.h"
Ultrasonic ultrasonic(12);
const int distance_detection=10;

//Bibliothèque pour utiliser le timer
#include <FlexiTimer2.h>

//Bibliothèques et paramétrage pour le lcd
// #include <Wire.h>
// #include "rgb_lcd.h"
// rgb_lcd lcd;

/*******************************************************************
                        PINS DES COMPOSANTS
********************************************************************/
const int fdc_fermeture = A2;
const int fdc_ouverture = A1;
const int bp_int = A0;
const int recepteur_IR = A5;
const int ouverture = 10;
const int fermeture = 11;
const int emetteur_IR = 5;
/*******************************************************************
                              VARIABLES
********************************************************************/
int etat_courant=0;
//boolean NFC=false;
boolean demande_envoi_trame=false,BP=false,FDCO=false,FDCF=false,IR=false,PV=false; //Capteurs
boolean Fermeture_Set=false,Fermeture_Reset=false,Ouverture_Set=false,Ouverture_Reset=false;
boolean voiture_genante=false,voiture_genante_Set=false,voiture_genante_Reset=false;
volatile boolean fin_tempo=false; //Variable d'interruption de fin de temporisation

/*******************************************************************
                      MACRO D'INTERRUPT FIN TIMER
********************************************************************/
void macro_fin_timer(void)
{
  fin_tempo=true;
}

/*******************************************************************
                              PARAMETRAGE
********************************************************************/
void setup()
{
  // lcd.begin(16, 2); //Initialisation du lcd
  // lcd.setCursor(0, 0);
  // lcd.print("Initialisation");
  // delay(3000);
  // lcd.clear();

  Serial.begin(9600);

  pinMode(bp_int, INPUT);
  pinMode(fdc_ouverture, INPUT);
  pinMode(fdc_fermeture, INPUT);
  pinMode(recepteur_IR, INPUT);
  pinMode(ouverture,OUTPUT);
  pinMode(fermeture,OUTPUT);
  pinMode(emetteur_IR,OUTPUT);

  // lcd.setCursor(0, 0);
  // lcd.print("Initialisation NFC");
  // nfc.begin();//Démarrage du module NFC
  digitalWrite(emetteur_IR, HIGH); //Activation de la barrière infrarouge
  FlexiTimer2::set(5000,macro_fin_timer); //paramétrage du timer
}

/*******************************************************************
                          LECTURE NFC
********************************************************************/
// boolean lire_carte_nfc(void)
// {
//   boolean resultat=false;
//   String scannedUID;//Variable de stockage de l'UID de la carte lue
//   if (nfc.tagPresent())
//   {
//     NfcTag tag = nfc.read(); //Lecture de la carte NFC
//     scannedUID = tag.getUidString(); //Acquisition du code UID de la carte
//
//     if( myUID.compareTo(scannedUID) == 0) //Comparaison entre les deux chaînes de caractères
//        return resultat=true; // Dans le cas où elles sont identiques
//     else
//        return resultat=false; //Dans le cas où elles diffèrent
//   }
// }

/*******************************************************************
                    LECTURE PRESENCE VOITURE
********************************************************************/
boolean lecture_presence_voiture(void)
{
  boolean resultat=false;

  if (ultrasonic.MeasureInCentimeters()<distance_detection)
    return resultat=true;
  else
    return resultat=false;
}

/*******************************************************************
                      ENVOI TRAME
********************************************************************/
void envoi_trame(void)
{

}

/*******************************************************************
                    LECTURE/MEMORISATION ENTREES
********************************************************************/
void lecture_memorisation_entrees(void)
{
  BP=digitalRead(bp_int);
  //NFC=lire_carte_nfc();
  FDCO=digitalRead(fdc_ouverture);
  FDCF=digitalRead(fdc_fermeture);
  IR=digitalRead(recepteur_IR);
  PV=lecture_presence_voiture();
}

/*******************************************************************
                            EVOLUTION
********************************************************************/
int evolution(void)
{
  int etat_s=0;

  switch(etat_courant)
  {
    case 0: if(BP) //if(NFC||BP)
              etat_s=1;
            else
              {
                if (PV)
                  etat_s=4;
                else
                  {
                    if(demande_envoi_trame)
                      etat_s=0;
                    else
                      etat_s=0;
                  }
              }
            break;

    case 1: if(FDCO)
              etat_s=2;
            else
              etat_s=1;
            break;

    case 2: if(BP)
              etat_s=3;
            else
            {
              if(demande_envoi_trame)
                etat_s=2;
              else
                etat_s=2;
            }
            break;

    case 3: if (IR)
              etat_s=1;
            else
            {
              if(FDCF)
                etat_s=0;
              else
                etat_s=3;
            }
            break;

    case 4: if(fin_tempo)
              etat_s=0;
            else
              {
                if(BP) //if(NFC||BP)
                  etat_s=1;
                else
                  {
                    if(!PV)
                      etat_s=0;
                    else
                      etat_s=4;
                  }
              }
              break;

    default: etat_s=0;
  }
  return etat_s;
}

/*******************************************************************
                      ACTIONS SUR TRANSITION
********************************************************************/
void actions_sur_transition(void)
{
  //Codage action envoi_trame et var voiture_genante_Reset sur états 0 et 2
  switch(etat_courant)
  {
    case 0: if(demande_envoi_trame)
            {
              envoi_trame();
              voiture_genante_Reset=true;
            }
            break;

    case 2: if(demande_envoi_trame)
            {
              envoi_trame();
              voiture_genante_Reset=true;
            }
            break;
  }

  //Codage action voiture_genante sur etat 0
  switch(etat_courant)
  {
    case 0: if(fin_tempo)
              {
                voiture_genante_Set=true;
                fin_tempo=false;//RAZ de la var de fin de tempo
              }
            break;

    default:voiture_genante=false;
  }

  //Action demarrer_tempo sur 0 --> 4
  switch (etat_courant)
  {
    case 0: if(PV)
            {
              FlexiTimer2::start();
              Serial.println("++++++++++++++++++++++");
              Serial.println("Tempo demarree");
              Serial.println("++++++++++++++++++++++");
              delay(2000);
            }
            break;
  }

  // Affectation va voiture_genante
  if(voiture_genante_Set)
    voiture_genante=true;
  else
    {
      if(voiture_genante_Reset)
        voiture_genante=false;
    }

}

/*******************************************************************
                        ACTIONS SUR ETATS
********************************************************************/
void actions_sur_etat(void)
{
  //Action ouverture Set
  switch (etat_courant)
  {
    case 1: Ouverture_Set=true;
            break;
    default: Ouverture_Set=false;
  }

  //Action fermeture Set
  switch (etat_courant)
  {
    case 3: Fermeture_Set=true;
            break;
    default: Fermeture_Set=false;
  }

  //Action ouverture Reset
  switch (etat_courant)
  {
    case 2: Ouverture_Reset=true;
            break;
    default: Ouverture_Reset=false;
  }

  //Action fermeture reset
  switch (etat_courant)
  {
    case 0: Fermeture_Reset=true;
            FlexiTimer2::stop(); //Arrêt de la tempo
            break;
    case 1: Fermeture_Reset=true;
            break;
    default: Fermeture_Reset=false;
  }

  //Ecritures sur Sorties
  if(Ouverture_Set)
    digitalWrite(ouverture, HIGH);
  else
    {
      if(Ouverture_Reset)
        digitalWrite(ouverture, LOW);
    }

  if(Fermeture_Set)
    digitalWrite(fermeture, HIGH);
  else
    {
      if(Fermeture_Reset)
        digitalWrite(fermeture, LOW);
    }
}

/*******************************************************************
                        TRAITEMENT MAE
********************************************************************/
void traitement_MAE(void)
{
  int etat_suivant=0;
  lecture_memorisation_entrees();
  etat_suivant=evolution();
  actions_sur_transition();
  etat_courant=etat_suivant;
  actions_sur_etat();
}

/*******************************************************************
                        PROGRAMME PRINCIPAL
********************************************************************/
void loop()
{
  etat_courant=0;

  while(1)
  {
    //lcd.setCursor(0, 0);
    Serial.println("-----------------------------------");
    Serial.print("Etat MAE = ");
    Serial.println(etat_courant);
    Serial.print("Presence voiture = ");
    Serial.println(PV);
    Serial.print("Voiture génante = ");
    Serial.println(voiture_genante);
    traitement_MAE();

  }
}
