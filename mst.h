
#define MST_COMMON_DATA_SZ  64

struct SMstCommon
{
  u8 nUsed; // Slot vide == 0, occupé == 1.
  u8 nMstNo; // N° du monstre.

  void (*pFctInit) (struct SMstCommon *pMst); // Fct d'init du monstre (voir monsters.c).
  s32 (*pFctMain) (struct SMstCommon *pMst);  // Fct principale du monstre (idem).

  s32 nPosX, nPosY;
  s32 nSpd;
  u8 nAngle;
  s32 nAnm;
  u8 nPhase;
  
  u8 pData[MST_COMMON_DATA_SZ]; // On fera pointer les structures spécifiques ici.
};

extern u32 gnMstPrio; // Pour priorité de l'affichage.
#define MSTPRIO_AND 31

s32 Mst_GetSlot(void); // Cherche un slot libre.
s32 Mst_Add(u32 nMstNo, s32 nPosX, s32 nPosY); // Ajoute un monstre dans la liste.

void Mst_InitEngine(void); // RAZ de tous les slots.
