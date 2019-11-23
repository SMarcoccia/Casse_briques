#include "includes.h"

// Flags.
enum{
  e_AnmFlag_End = (1 << 0),
};

struct SAnim
{
  u8 nUsed; // 0 = slot vide, 1 = slot occupé.
  u8 nFlags;
  u32 *pOrg; // Ptr sur le début de l'anim.

  u32 *pAnm; // Ptr sur le nombre de frame.
  u32 nKey; // Clef d'anim. 16b priorité | 16b No.
  u32 nFramesCnt; // Compteur de frames restant pour l'iamge en cours.
  u32 nCurSpr; // N° du sprite en cours.
};

#define ANM_MAX_SLOTS 64
struct SAnim pAnmSlots[ANM_MAX_SLOTS]; // Correspond au nombre maximale de sprites pour une seul animation.
u32 gnAnmLastUsed;

s32 Animspr_AnmSetIfNew(u32 *pAnm, s32 nSlotNo)
{
  if(nSlotNo == -1)
  {
    return (Animspr_Set(pAnm, nSlotNo));
  }
  else if (pAnmSlots[nSlotNo].pOrg != pAnm)
  {
    // Anim différente ? Teste la priorité.
    if((*pAnm) >> 16 >= pAnmSlots[nSlotNo].nKey >> 16)
    {
      return (Animspr_Set(pAnm, nSlotNo));
    }
  }

  // Priorité <, on ne réinitialise pas.
  return nSlotNo;
}

u32 Animspr_CheckEnd(s32 nSlotNo)
{
  return pAnmSlots[nSlotNo].nFlags & e_AnmFlag_End;
}

u32 Animspr_GetKey(s32 nSlotNo)
{
  return pAnmSlots[nSlotNo].nKey;
}

s32 Animspr_GetLastImage(s32 nSlotNo)
{
  return (pAnmSlots[nSlotNo].nCurSpr);
}

// Renvoi l'image en cours et avance l'anim.
s32 Animspr_GetImage(s32 nSlotNo)
{
  // Décrémentation et avancée si nécéssaire. Nbre de frames avant la prochaine image.
  if(--pAnmSlots[nSlotNo].nFramesCnt == 0)
  {
    pAnmSlots[nSlotNo].pAnm += 2; // Avance le ptr sur la frame suivante.
    while(((*pAnmSlots[nSlotNo].pAnm) & BIT31 ) != 0) // Si on tombe sur un flag, ex : e_Anm_Goto on entre. 
    {                                                 // Note : si plusieurs flag => continue. Ex : son.
      // Code de contrôle.
      switch(*pAnmSlots[nSlotNo].pAnm)
      {
        case e_Anm_Jump: // Ajoute un offset au pointeur.
          pAnmSlots[nSlotNo].pAnm += *(s32 *)(pAnmSlots[nSlotNo].pAnm + 1);
          break;
        case e_Anm_Goto : 
          // On met l'adresse du début du tableau.
          pAnmSlots[nSlotNo].pAnm = (u32 *)*(pAnmSlots[nSlotNo].pAnm + 1); 
          pAnmSlots[nSlotNo].pOrg = pAnmSlots[nSlotNo].pAnm; // On met l'adresse début de tableau.
          pAnmSlots[nSlotNo].nKey = *(pAnmSlots[nSlotNo].pAnm); // On met la clé d'anim (e_AnmKey_Null).
          pAnmSlots[nSlotNo].pAnm++; // On se met sur le premier nombre de frames.
          break;
        
        case e_Anm_End:
          pAnmSlots[nSlotNo].pAnm -= 2; // Fin de l'anim. Place le flag End et renvoie SPR_NoSprite.
          pAnmSlots[nSlotNo].nFramesCnt = 1; // Recule le ptr pour repointer sur e_Anm_End au prochain tour.
          pAnmSlots[nSlotNo].nCurSpr = SPR_NoSprite; // Reset compteur.
          pAnmSlots[nSlotNo].nFlags |= e_AnmFlag_End; // Flag.
          return pAnmSlots[nSlotNo].nCurSpr;
          break;
        
        case e_Anm_Kill: // Fin de l'anim + libération du slot.
          Animspr_ReleaseSlot(nSlotNo);
          return -1;
          // break;
        case e_Anm_Sfx: // Joue un son.
          Sfx_PlaySfx(*(pAnmSlots[nSlotNo].pAnm + 1), *(pAnmSlots[nSlotNo].pAnm + 2));
          pAnmSlots[nSlotNo].pAnm += 3;
          break;
        
        default:
          printf("Anm : Unknown control code.\n");
          break;
      }
    }
    // Image.
    pAnmSlots[nSlotNo].nFramesCnt = *pAnmSlots[nSlotNo].pAnm; // Nbre total de frames pour l'image en cours.
    pAnmSlots[nSlotNo].nCurSpr = *(pAnmSlots[nSlotNo].pAnm + 1); // N° du sprite en cours.
  }
  return pAnmSlots[nSlotNo].nCurSpr; // N° du sprite en cours.
}

// Libère un slot.
void Animspr_ReleaseSlot(s32 nSlotNo)
{
  if(nSlotNo == -1) return;

  // Pour accélérer la recherche des slots libres.
  if((u32)nSlotNo < gnAnmLastUsed)
  {
    gnAnmLastUsed = nSlotNo;
  }
  pAnmSlots[nSlotNo].nUsed = 0;
}

void Animspr_InitEngine(void)
{
  u32 i;
  
  //RAZ de tous les slots.
  for(i = 0; i < ANM_MAX_SLOTS; i++)
  {
    pAnmSlots[i].nUsed = 0;
  }
  gnAnmLastUsed = 0;
}

s32 Animspr_GetSlot(void)
{
  u32 i;
  for(i = gnAnmLastUsed; i < ANM_MAX_SLOTS; i++)
  {
    if(pAnmSlots[i].nUsed == 0)
    {
      gnAnmLastUsed = i + 1; // Recherche du slot commencera au suivant.
      return i;
    }
  }
  return -1;
}

// Init d'une anim (recherche un slot ou un nouveau slot).
// Out : N° du slot. -1 si erreur.
s32 Animspr_Set(u32 *pAnm, s32 nSlotNo)
{
  // Si nSlotNo == -1, on cherche un nouveau slot (un nouvelle emplacement pour l'anim du sprite).
  if(nSlotNo == -1)
  {
    if((nSlotNo = Animspr_GetSlot()) == -1) return -1;
    pAnmSlots[nSlotNo].nUsed = 1;
  }

  pAnmSlots[nSlotNo].nFlags = 0; 
  pAnmSlots[nSlotNo].nKey = *pAnm; // Clef d'anim.
  pAnmSlots[nSlotNo].pOrg = pAnm; // Ptr sur le début de l'anim.
  
  // On fait un GetImage pour initialiser le slot.
  pAnmSlots[nSlotNo].pAnm = pAnm - 1;
  pAnmSlots[nSlotNo].nFramesCnt = 1;
  // GetImage pour initialiser le slot.
  Animspr_GetImage(nSlotNo);

  return nSlotNo;
}
