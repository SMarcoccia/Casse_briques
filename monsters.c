
#include "includes.h"

// Structure spécifique pour Doh.
struct SMstDoh
{
  u8  nLifePts; // Pts de vie.
  u8  nDeath1;  // Compteur pour pendant combien de temps on balance des explosions. 
  
  u16 nCntIdle; // Pause en idle (inaction).

  u16 nCntAttk; // Pause entre les tirs.
  u16 nCntAttkInit; // Durée de la pause entre les tirs pour reset.
  u16 nNbAttk;  // Nb de tirs.

  u8 nNoCol;    // Quand la balle touche, pour ne pas retoucher tant qu'il y a collision.
  u16 nLastBallPosX[3], nLastBallPosY[3];
};

#define DOH_PauseIdle_Long 100
#define DOH_PauseIdle_Avg 80
#define DOH_PauseIdle_Short 60
#define DOH_PauseShoot 16

// Init pour pillules et les monstre.
void Monsters_InitPill(struct SMstCommon *pMst);
s32 Monsters_MainPill(struct SMstCommon *pMst);
void Monsters_InitGenerateur(struct SMstCommon *pMst);
s32 Monster_MainGenerateur(struct SMstCommon *pMst);
void Monsters_InitMst1(struct SMstCommon *pMst);
s32 Monster_MainMst1(struct SMstCommon *pMst);
void Monsters_InitDoorR(struct SMstCommon *pMst);
s32 Monster_MainDoorR(struct SMstCommon *pMst);
void Monsters_InitDoh(struct SMstCommon *pMst);
s32 Monster_MainDoh(struct SMstCommon *pMst);

// Tableau de 14 cases correspondant chacune à un item (voir monsters.h).
struct SMstTb gpMstTb[] =
{
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm1,  100 },	// Pill: Aimant.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm2,  100 },	// Pill: Mitrailleuse.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm3,  100 },	// Pill: Balle traversante.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm4,  100 },	// Pill: Balle bigger.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm5,  100 },	// Pill: Balle x3
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm6,  100 },	// Pill: Raquette bigger.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm7,    0 },	// Pill: Raquette smaller.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm8,  100 },	// Pill: 1Up.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm9,  100 },	// Pill: Porte à droite.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm10, 100 },	// Pill: Speed Up.
	{ Monsters_InitPill, Monsters_MainPill, gAnm_Itm11, 100 },	// Pill: Speed Down.
	{ Monsters_InitGenerateur, Monster_MainGenerateur, gAnm_MstDoorWait, 0 },
	{ Monsters_InitMst1, Monster_MainMst1, gAnm_Mst1, 100 },		      // Monstres basiques des niveaux.
	{ Monsters_InitDoorR, Monster_MainDoorR, gAnm_MstDoorRight, 0 },	// Porte à droite.
	{ Monsters_InitDoh, Monster_MainDoh, gAnm_MstDohAppears, 10000 },	// Doh.
};


//===============================================================================
//======================VARIABLE GENERALES SPECIFIQUES===========================
//===============================================================================

struct SMstMisc{
  u32 nNbMstLev; // Compte le nb de monstre présents. (3 max).
  u32 nPillNb; // Nb de pillules présente en même temps. // Facultatif.
  u8 nMstDoorR:1; // Flag pour déclencher l'ouverture de la porte.
};
struct SMstMisc gMstMisc;

//===============================================================================
//=========================== MONSTRE PILLULE (BONUS) ===========================
//===============================================================================


u32 Monsters_InitPillGetNb(void)
{
  return(gMstMisc.nPillNb);
}

void Monsters_InitPill(struct SMstCommon *pMst)
{
  gMstMisc.nPillNb++;
}

// EN ATTENTE FERA APRES INIT.
s32 Monsters_MainPill(struct SMstCommon *pMst)
{
  // s32 nSpr;
  // // Déplacement.
  // pMst->nPosY += 0x100;
  // // Sortie de l'écran ?
  // if(pMst->nPosY >> 8 > SCR_Height + 8)
  // {
  //   // Tuage de l'ennemi.
  //   return(-1);
  // }
  // // Contact avec le joueur ?
  // nSpr = AnmGetImage(pMst->nAnm);
  // if(AnmGetKey(gBreak.nPlayerAnmNo) != e_AnmKey_PlyrDeath)
  return 1;
}

// Phases.
enum{
  e_MstDoorR_PhaseClosed = 0,
  e_MstDoorR_PhaseOpen,
  e_MstDoorR_PhaseSuckingIn, // Aspire la raquette.
};

void MstDoorROpen(void)
{
  gMstMisc.nMstDoorR = 1; // On tente de déclencher l'ouverture.
}

void Monsters_InitDoorR(struct SMstCommon *pMst)
{
  pMst->nPhase = e_MstDoorR_PhaseClosed; 
  gMstMisc.nMstDoorR = 0; // RAZ interrupteur.
}

s32 Monsters_MainDoorR(struct SMstCommon *pMst)
{
  return 1;
}


//===============================================================================
//================= MONSTRE QUI FAIT APPARAÎTRE LES MONSTRES ====================
//===============================================================================

void Monsters_InitGenerateur(struct SMstCommon *pMst)
{
  
}

s32 Monster_MainGenerateur(struct SMstCommon *pMst)
{
  return -1;
}

void Monsters_InitMst1(struct SMstCommon *pMst)
{

}

s32 Monster_MainMst1(struct SMstCommon *pMst)
{
  return -1;
}

void Monsters_InitDoorR(struct SMstCommon *pMst)
{

}

s32 Monster_MainDoorR(struct SMstCommon *pMst)
{
  return -1;
}

void Monsters_InitDoh(struct SMstCommon *pMst)
{
  struct SMstDoh *pSpe =(struct SMstDoh *) pMst->pData;
  
  pMst->nPhase = e_MstDoh_Appear;
  pSpe->nLifePts = DOH_LifePts;
  pSpe->nNoCol = 0;

  DohInitTimers(pSpe);

  static u8 nPreca = 0;
}

s32 Monster_MainDoh(struct SMstCommon *pMst)
{
  return -1;
}

