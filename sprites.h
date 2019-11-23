
#define SPR_Flag_Shadow (1 << 31)

struct SSprite
{
  u32 nPtRefX, nPtRefY;
  u32 nLg, nHt;
  u8 *pGfx;
  u8 *pMask;
};

void Sprites_InitEngine(void); // Initialisation du moteur.
void Sprites_Release(void); // Libère la mémoire alloué pour tous les ptr pGfx (pMask n'étant pas défini avec un malloc) si problème de realloc.
void Sprites_LoadBMP(char *pFilename, SDL_Color *pSprPal, u32 nPalIdx); // Récupération des sprites dans la planche.
void Sprites_DisplayLock(u32 nSprNo, s32 nPosX, s32 nPosY); // Affichage d'un sprite + gestion de l'ombre.
void Sprites_Display(u32 nSprNo, s32 nPosX, s32 nPosY, u32 nPrio); // Stock dans un tableau de struct tout les sprites qui devrons être afficher en fonction de leurs priorités.
void Sprites_DisplayAll(void); // // Trie la liste des sprites et les affiche (A appeler une fois par frame).
struct SSprite *Sprites_GetDesc(u32 nSprNo); // Renvoi un pointeur sur un descripteur de sprites.
// struct SSprite *Sprites_GetDesc(u32 nSprNo); // Renvoi un pointeur sur un descripteur de sprites.
u32 Sprites_CheckColBox(u32 nSpr1, s32 nPosX1, s32 nPosY1, u32 nSpr2, s32 nPosX2, s32 nPosY2); // Teste une collision entre 2 sprites (Out: 1 col, 0 pas col).

void Sprites_Realloc(); //On Realloc si toutes les struct disponibles ont été utilisés.

u32 gnSprStock;

#define SPR_NoSprite ((u32) -2)

// Définie les priorités.
enum
{
  e_Prio_Ombres = 0,
  e_Prio_Briques = 10,
  e_Prio_Dust = 50,
  e_Prio_Monstres = 60,
  e_Prio_Tirs =  100,
  e_Prio_Raquette = 110,
  e_Prio_Balles = 120,
  e_Prio_HUD = 200, //HUD ou Head-up Display : affichage permanent de tout les renseignements important concernant le jeux (par ex : les PV ...).
};

// Définie les N° de sprites.
enum
{
  e_Spr_Bricks = 0, 
  e_Spr_BricksSpe = e_Spr_Bricks + 10, 
  e_Spr_BricksExplo = e_Spr_BricksSpe + 3, 
  e_Spr_Bricks2HitExplo = e_Spr_BricksExplo + 10, 
  e_Spr_BricksCBExplo = e_Spr_Bricks2HitExplo + 10, 
  e_Spr_BrickIndesHit = e_Spr_BricksCBExplo + 10, 
  e_Spr_Brick2Hit = e_Spr_BrickIndesHit + 6, 
  e_Spr_BrickCBHit = e_Spr_Brick2Hit + 6,
  e_Spr_RaqClignG = e_Spr_BrickCBHit + 6,
  e_Spr_RaqClignD = e_Spr_RaqClignG + 5,
  e_Spr_Raquette = e_Spr_RaqClignD + 5,
  e_Spr_HUDRaquette = e_Spr_Raquette + 1,

  e_Spr_RaquetteAimant = e_Spr_HUDRaquette + 1,
  e_Spr_RaquetteMitrG = e_Spr_RaquetteAimant + 3,
  e_Spr_RaquetteMitrD = e_Spr_RaquetteMitrG + 4,
  e_Spr_PlyrShot = e_Spr_RaquetteMitrD + 4,
  e_Spr_RaquetteRallonge0 = e_Spr_PlyrShot + 1,
  e_Spr_RaquetteRallonge1 = e_Spr_RaquetteRallonge0 + 6,
  e_Spr_RaquetteRallonge2 = e_Spr_RaquetteRallonge1 + 6,
  e_Spr_RaquetteApparition = e_Spr_RaquetteRallonge2 + 6,
  e_Spr_RaquetteDeathSz0  = e_Spr_RaquetteApparition + 14,
  e_Spr_RaquetteDeathSz1 = e_Spr_RaquetteDeathSz0 + 8,
  e_Spr_RaquetteDeathSz2 = e_Spr_RaquetteDeathSz1 + 8,
  e_Spr_RaquetteDeathSz3 = e_Spr_RaquetteDeathSz2 + 8,

  e_Spr_Ball = e_Spr_RaquetteDeathSz3 + 8,
  e_Spr_BallTrav = e_Spr_Ball + 4,
  e_Spr_Itm1 = e_Spr_BallTrav + 4,
  e_Spr_Itm2 = e_Spr_Itm1 + 10,
  e_Spr_Itm3 = e_Spr_Itm2 + 10,
  e_Spr_Itm4 = e_Spr_Itm3 + 10,
  e_Spr_Itm5 = e_Spr_Itm4 + 10,
  e_Spr_Itm6 = e_Spr_Itm5 + 10,
  e_Spr_Itm7 = e_Spr_Itm6 + 10,
  e_Spr_Itm8 = e_Spr_Itm7 + 10,
  e_Spr_Itm9 = e_Spr_Itm8 + 10,
  e_Spr_Itm10 = e_Spr_Itm9 + 10,
  e_Spr_Itm11 = e_Spr_Itm10 + 10,
  e_Spr_Itm12 = e_Spr_Itm11 + 10,
  e_Spr_Itm13 = e_Spr_Itm12 + 1,

  e_Spr_Mst1 = e_Spr_Itm13 + 1,
  e_Spr_Mst2 = e_Spr_Mst1 + 8,
  e_Spr_Mst3 = e_Spr_Mst2 + 12,
  e_Spr_Mst4 = e_Spr_Mst3 + 24,
  e_Spr_MstExplo1 = e_Spr_Mst4 + 11,
  e_Spr_MstPorteLevel = e_Spr_MstExplo1 + 7,
  e_Spr_SortieMstCache = e_Spr_MstPorteLevel + 20,
  e_Spr_SortieMst = e_Spr_SortieMstCache + 1,
  e_Spr_CacheDroit = e_Spr_SortieMst + 3,

  e_Spr_DohShoot = e_Spr_CacheDroit + 1,
  e_Spr_DohIdle = e_Spr_DohShoot + 1,
  e_Spr_DohHit = e_Spr_DohIdle + 1,
  e_Spr_DohDisappear = e_Spr_DohHit + 1,
  e_Spr_DohMissile = e_Spr_DohDisappear + 5,
  e_Spr_DohMisDust = e_Spr_DohMissile + 4,

  e_Spr_Logo = e_Spr_DohMisDust + 4,
  e_Spr_Logo8 = e_Spr_Logo + 1,

  e_Spr_BossBar = e_Spr_Logo8 + 8,
  e_Spr_BossBarTop = e_Spr_BossBar + 1,
  e_Spr_BossBarPts = e_Spr_BossBarTop + 1,

  e_Spr_FontSmall = e_Spr_BossBarPts + 8,
  e_Spr_FontSmall_Effect = e_Spr_FontSmall + 64,
  e_Spr_NEXT = e_Spr_FontSmall_Effect + 64,
};

