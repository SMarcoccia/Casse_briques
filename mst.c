
// Petit moteur de monstres.

#include "includes.h"

extern struct SMstTb gpMstTb[];

#define MST_MAX_SLOTS 32
struct SMstCommon gpMstSlots[MST_MAX_SLOTS];
u32 gnMstLastUsed;
u32 gnMstPrio; // Priorité de l'affichage.

// RAZ moteur.
void Mst_InitEngine(void)
{
	u32 i;

	// RAZ de tous les slots.
	for(i = 0; i < MST_MAX_SLOTS; i++)
	{
		gpMstSlots[i].nUsed = 0;
	}
	gnMstLastUsed = 0;
}

// Cherche un slot libre.
// Out : N° d'un slot libre. 	-1 si erreur.
s32 Mst_GetSlot(void)
{
	u32 i;

	for(i = gnMstLastUsed; i < MST_MAX_SLOTS; i++)
	{
		if(gpMstSlots[i].nUsed == 0)
		{
			gnMstLastUsed = i + 1; // La recherche commencera au suivant.
			return i;
		}
	}
	return -1;
}

s32 Mst_Add(u32 nMstNo, s32 nPosX, s32 nPosY)
{
	s32 nSlotNo;

	if((nSlotNo = Mst_GetSlot()) == -1) return -1;
	gpMstSlots[nSlotNo].nAnm = -1;
	
	if(gpMstTb[nMstNo].pAnm != NULL) // Si NULL, on ne réserve pas d'anim.
	{
		// Pour réserver une anim.
		if((gpMstSlots[nSlotNo].nAnm = Animspr_Set(gpMstTb[nMstNo].pAnm, -1)) == -1)
			return -1;
	}

	gpMstSlots[nSlotNo].nUsed = 1;
	gpMstSlots[nSlotNo].nMstNo = nMstNo;
	
	gpMstSlots[nSlotNo].nPosX = nPosX << 8;
	gpMstSlots[nSlotNo].nPosY = nPosY << 8;
	
	gpMstSlots[nSlotNo].nSpd = 0;
	gpMstSlots[nSlotNo].nAngle = 0;
	gpMstSlots[nSlotNo].pFctInit = gpMstTb[nMstNo].pFctInit;
	gpMstSlots[nSlotNo].pFctMain = gpMstTb[nMstNo].pFctMain;
	// Appel de la fonction d'init du monstre.
	gpMstSlots[nSlotNo].pFctInit(&gpMstSlots[nSlotNo]);
	return nSlotNo;
}

