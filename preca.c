#include "includes.h"

#define PI 3.1415927

// Précalcule des tables de sinus-cosinus.
// 256 angles, val *256 (-> varie de -256 à 256).
void PrecaSinCos(void)
{
  u32 i;

  for(i=0; i <256 + 64; i++)
  {
    // Si i vaut 32 alors représente 45° et aura une valeur de 181 et si on divise 181 par 256 on retombe
    // sur le cos de 45 soit 0,707...
    gVar.pSinCos[i]=(s16)(cos(i * 2 * PI / 256) * 256);
  }

  gVar.pSin = gVar.pSinCos + 64; //Table des sinus on ajoute 64 qui repr�sente PI/2 (90�) valeur de sin identique � PI/2 pr�s.
  gVar.pCos = gVar.pSinCos;
}

