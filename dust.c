#include "includes.h"

struct SDust{
   u8 nUsed; // Slot: 0 = vide.
   s32 nAnm;
   s32 nPosX, nPosY; 
};

#define DUST_MAX_SLOTS 64
struct SDust gpDustSlots[DUST_MAX_SLOTS];
u32 gnDustLastUsed;

s32 Dust_GetSlot(void)
{
  u32 i;
  // gnDustLastUsed au départ à 0 (initialisé avec Dust_InitEngine) donc aucun slot utilisé.
  // Si ensuite gnDustLastUsed vaut 1 alors 1 slot utilisé.
  for(i = gnDustLastUsed; i < DUST_MAX_SLOTS; i++)
  {
    if(gpDustSlots[i].nUsed == 0)
    {
      gnDustLastUsed = i + 1; // La recherche commencera au suivant.
      return i;
    }
  }
  return -1;
}

s32 Dust_DustSet(u64 *pAnm, s32 nPosX, s32 nPosY)
{
  s32 nSlotNo;
  
  if((nSlotNo = Dust_GetSlot()) == -1) return -1;
  if((gpDustSlots[nSlotNo].nAnm = Animspr_Set(pAnm, -1)) == -1) return -1;

  gpDustSlots[nSlotNo].nUsed = 1;
  gpDustSlots[nSlotNo].nPosX = nPosX;
  gpDustSlots[nSlotNo].nPosY = nPosY;

  return nSlotNo;
}

void Dust_InitEngine(void){
  u32 i;
  // RAZ de tous les slots.
  for(i = 0; i < DUST_MAX_SLOTS; i++)
  {
    gpDustSlots[i].nUsed = 0;
  }
  gnDustLastUsed = 0;
}

