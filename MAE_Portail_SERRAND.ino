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
//Bibliothèques et paramétrage pour le lcd
#include <Wire.h>
#include "rgb_lcd.h"
rgb_lcd lcd;

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
boolean BP=false,FDCO=false,FDCF=false,IR=false; //Capteurs
boolean Fermeture_Set=false,Fermeture_Reset=false,Ouverture_Set=false,Ouverture_Reset=false;

/*******************************************************************
                              PARAMETRAGE
********************************************************************/
void setup()
{
  Serial.begin(9600);

  pinMode(bp_int, INPUT);
  pinMode(fdc_ouverture, INPUT);
  pinMode(fdc_fermeture, INPUT);
  pinMode(recepteur_IR, INPUT);
  pinMode(ouverture,OUTPUT);
  pinMode(fermeture,OUTPUT);
  pinMode(emetteur_IR,OUTPUT);

  digitalWrite(emetteur_IR, HIGH); //Activation de la barrière infrarouge
}

/*******************************************************************
                    LECTURE/MEMORISATION ENTREES
********************************************************************/
void lecture_memorisation_entrees(void)
{
  BP=digitalRead(bp_int);
  FDCO=digitalRead(fdc_ouverture);
  FDCF=digitalRead(fdc_fermeture);
  IR=digitalRead(recepteur_IR);

  Serial.print("BP = ");
  Serial.println(BP);

  Serial.print("FDCO = ");
  Serial.println(FDCO);

  Serial.print("FDCF = ");
  Serial.println(FDCF);

  Serial.print("IR = ");
  Serial.println(IR);
}

/*******************************************************************
                            EVOLUTION
********************************************************************/
int evolution(void)
{
  int etat_s=0;

  switch(etat_courant)
  {
    case 0: if(BP)
              etat_s=1;
            else
              etat_s=0;
            break;

    case 1: if(FDCO)
              etat_s=2;
            else
              etat_s=1;
            break;

    case 2: if(BP)
              etat_s=3;
            else
              etat_s=2;
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

    default: etat_s=0;
  }
  return etat_s;
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
  etat_courant=etat_suivant;
  actions_sur_etat();
}

/*******************************************************************
                        PROGRAMME PRINCIPAL
********************************************************************/
void loop()
{

    Serial.print("Etat MAE =");
    Serial.println(etat_courant);
    traitement_MAE();

}
