
#include "includes.h"

u32 Font_Print(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags, u32 nNoSpr){

  char cChr;
  struct SSprite *pSpr;
  s32 nPosXOrg = nPosX;

  while(*pStr)
  {
    cChr = *pStr++;
    cChr -= ' '; // Soustrait ' ' pour être conforme à la sprites sheet des fontsmalls.
    if(cChr != '\0')
    {
      cChr--; //Soustrait 1 pour être à la bonne lettre.
      pSpr = Sprites_GetDesc(nNoSpr + cChr);
      if((nFlags & FONT_NoDispo) == 0) Sprites_Display(nNoSpr + cChr, nPosX, nPosY, e_Prio_HUD);
    }
    else
      {
        //Espace de la taille d'un I.
        pSpr = Sprites_GetDesc(nNoSpr + 'I' - ' ');
      }
      nPosX += pSpr->nLg + 1; // +1 Pour laisser un espace entre les lettres.
  }

  return ((u32)(nPosX - nPosXOrg));
}


void Font_MyItoA(s32 nNb, char *pDst){

   if(nNb < 0)
   {
     strcpy(pDst, "000");
     nNb = -nNb;
     pDst[0] = '-';
   }
   
  u32 nTmp = strlen(pDst);

  while(nNb > 0)
  {
    pDst[--nTmp] = nNb % 10 + '0';
    nNb/=10;
  }
}
