#include "includes.h"

struct SFireRecord{
  u32 *pAnm;
  s32 nSpeed;
  u8 nAngle;
  u8 nPlyr; // 1 = Tir du joueur, 0 = Tir d'un monstre.
  u32 *pAnmDust; // Anim de disparition. NULL pas utilisé.
};

struct SFireRecord gpFireTable[] = {
  { gAnm_PlyrShot, 0x400, 64, 1, NULL }, // Joueur : tir de la mitrailleuse.
  { gAnm_DohMissile, 0x200, 64+128, 0, gAnm_DohMissileDisp } // Doh : tir.
  // todo : rajouter un dust générique pour le tire.
};

struct SFire{
  u8 nUsed; // 0 = slot vide, 1 = slot occupé.
  s32 nAnm; // Anim.
  s32 nPosX, nPosY; // 8bits de virgule fixe.
  s32 nSpeed;
  u8 nAngle;
  u8 nPlyr;
  u32 *pAnmDust; // Anim de diparition. NULL si pas utilisée.
};

#define FIRE_MAX_SLOTS 64
struct SFire gpFireSlots[FIRE_MAX_SLOTS];
u32 gnFireLastUsed;


s32 Fire_GetSlot(void)
{
  u32 i;

  for(i = gnFireLastUsed; i < FIRE_MAX_SLOTS; i++)
  {
    if(gpFireSlots[i].nUsed == 0)
    {
      gnFireLastUsed = i + 1;
      return i;
    }
  }
  return -1;
}

s32 Fire_Add(u32 nShot, s32 nPosX, s32 nPosY, s32 nAngle)
{
  s32 nSlotNo;

  if((nSlotNo = Fire_GetSlot()) == -1) return -1;
  // Récupère le n° de slot.
  if((gpFireSlots[nSlotNo].nAnm = Animspr_Set(gpFireTable[nShot].pAnm, -1)) == -1) return -1;

  gpFireSlots[nSlotNo].nUsed = 1;
  gpFireSlots[nSlotNo].nPosX = nPosX << 8; // Plus grand précision.
  gpFireSlots[nSlotNo].nPosY = nPosY << 8;
  gpFireSlots[nSlotNo].nSpeed = gpFireTable[nShot].nSpeed;
  gpFireSlots[nSlotNo].nAngle = (nAngle == -1 ? gpFireTable[nShot].nAngle : (nAngle & 0xFF));
  gpFireSlots[nSlotNo].nPlyr = gpFireTable[nShot].nPlyr;
  gpFireSlots[nSlotNo].pAnmDust = gpFireTable[nShot].pAnmDust;

  return nSlotNo;
}

void Fire_ReleaseSlot(u32 nSlotNo)
{
  for(u32 i = 0; i < FIRE_MAX_SLOTS; i++)
  {
    gpFireSlots[i].nUsed = 0;
  }
  gnFireLastUsed = 0;
}

u32 Fire_MstCheckFire(u32 nSpr, s32 nPosX, s32 nPosY)
{
  u32 i;
  s32 nXMin1, nXMax1, nYMin1, nYMax1;
  struct SSprite *pSpr1 = Sprites_GetDesc(nSpr); 

  nXMin1 = nPosX - pSpr1->nPtRefX;
  nXMax1 = nXMin1 + pSpr1->nLg;
  nYMin1 = nPosY - pSpr1->nPtRefY;
  nYMax1 = nYMin1 + pSpr1->nHt;

  nXMin1 <<= 8; nXMax1 <<= 8; // Décalage, pour optimiser les comparaisons.
  nYMin1 <<= 8; nYMax1 <<= 8; // Evite de faire un décalage pour chaque tire, et pour chaque monstre.

  for(i = 0; i < FIRE_MAX_SLOTS; i++)
  {
    if(gpFireSlots[i].nUsed)
    {
      if(gpFireSlots[i].nPosX >= nXMin1 && gpFireSlots[i].nPosX <= nXMax1 &&
         gpFireSlots[i].nPosY >= nYMin1 && gpFireSlots[i].nPosY <= nYMax1)
      {
       Fire_ReleaseSlot(i);
       return 1; // Ici on arrête car on meurt en 1 coup ; sinon continuer la boucle.
      }
    }
  }
  return 0;
}

// RAZ moteur.
void Fire_InitEngine(void){
  u32 i;

  // RAZ de tous les slots.
  for(i = 0; i < FIRE_MAX_SLOTS; i++)
  {
    gpFireSlots[i].nUsed = 0;
  }
  gnFireLastUsed = 0;
}
