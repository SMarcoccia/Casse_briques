
#define LEVEL_Max 37

#define PLAYER_Lives_Start 3
#define PLAYER_Lives_Max 10

#define TIMER_GamerOver (60 * 5)
#define TIMER_DisplayLevel (60 * 3)

#define BRICK_ComingBackCnt (5 * 60)

#define WALL_XMin (10)
#define WALL_XMax (SCR_Width - 10)
#define WALL_YMin 16

#define BRICK_Width 20
#define BRICK_Height 10

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SGN(x) ((x) >= 0 ? 1 : -1)
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define NBELEM(tab) (sizeof(tab)/sizeof(tab[0]))

#define TABLE_Width 15
#define TABLE_Height 17

// Flags pour la ball.
#define BALL_Flg_Traversante (1 << 0)
#define BALL_Flg_Aimantee (1 << 1) // Balle collée sur la raquette.

// Flags pour la raquette.
#define PLAYER_Flg_Aimant (1 << 0)
#define PLAYER_Flg_Mitrailleuse (1 << 1)
#define PLAYER_Flg_NoKill (1 << 2)
#define PLAYER_Flg_DoorR (1 << 3)
#define PLAYER_Flg_BossWait (1 << 4)

// Flags pour la brique.
#define BRICK_Flg_Indestructible (1 << 0)
#define BRICK_Flg_ComingBack (1 << 1)

#define BALL_GfxLg 32
#define BALL_MAX_NB 12
#define BALL_MAX_SIZE 3

// Phases de jeu.
enum
{
  e_Game_SelectLevel = 0, // Selection du level.
  e_Game_Normal, // Pas de selection du level.
  e_Game_LevelCompleted, // Niveau terminé.
  e_Game_GameOver, // Gamer over.
  e_Game_AllClear, // Jeu terminé.
  e_Game_Pause, // Pause.
};

struct SBrique{
  u8 nPres; // Brique présente ou pas.
  u8 nCnt; // Nb de coups restantes avant destruction.
  u8 nFlags; // Flags: voir liste.
  u16 nResetCnt; // Compteur pour retour de la brique. 

  u32 nSprNo; // Sprite par défaut.
  s32 nAnmNo; // Remplacé par l'anim si != -1.

  u32 *pAnmHit; // Anim à utiliser pour le hit.
  u32 *pAnmExplo; // Anim à utiliser pour la disparition.
  u16 nScore; // Nb de point rapporté par la brique.
};

struct SBall{
  u8 nUsed; // Slot utilisé ?

  s32 nPosX, nPosY; // Virgule fixe 8 bits.
  s32 nSpeed; // Virgule fix 8 bits.
  u8 nAngle;
  u32 nFlags;
  u32 nSize;

  u32 nRayon; // = 3; // Offset du centre.
  u32 nDiam; // = 7; // Hauteur. (Largeur = puissance de 2 à choisir).
  // Masque de la balle, largeur arrangeante pour accélérer les test.
  u8 pBallMask[BALL_GfxLg * BALL_GfxLg]; 
  u32 nSpr; // Sprite de la balle.

  s32 nOffsRaq; // Offset sur la raquette (pour aimantation).
};

struct SBreaker
{
  struct SBall pBalls[BALL_MAX_NB];
  u32 nBallsNb; // Nb de balles à gérés en cours.
  s32 nPlayerPosX, nPlayerPosY;

  s32 nPlayerAnmNo; // Sprite de la raquette
  s32 nPlayerAnmBonusM; // Bonus central.
  s32 nPlayerAnmBonusG; // Bonus gauche.
  s32 nPlayerAnmBonusD; // Bonus droit.
  s32 nPlayerAnmClignG; // Clignotant gauche.
  s32 nPlayerAnmClignD; // Clignotant droit.

  u32 nPlayerFlags; 
  u32 nPlayerRSize; // Taille de la raquette.
  u32 nPlayerLives;  // Nb de vies.
  u32 nPlayerScore, nPlayerLastScore;  // Score.
  
  struct SBrique pLevel[TABLE_Width * TABLE_Height];
  u32 nRemainingBricks; // Briques restantes.
  u32 nBricksComingBackNbCur; // Brique en phase disparue, en attente de revenir.
  u32 nBricksComingBackTotal; // Brique qui reviennent total du niveau.

  u32 nPhase; // Phase du jeu (init, jeu, game over...).
  u32 nLevel; // Level en cours.

  u32 nTimerGameOver; // Countdown pour game over.
  u32 nTimerLevelDisplay; // Compteur pour affichage du n° de level.
};

struct SBreaker gBreak;

// Traitement d'une brique (relachement d'items, etc...).
// Renvoie les flags de la brique. -1 si pas de choc.
// nBx et nBy position de la brique respectivement colonne et ligne.
// nBallFlags récupère le flag de la balle (traverse les briques ou balle collé à la raquette).
u32 Breaker_BrickHit(u32 nBx, u32 nBy, u32 nBallFlags);
// Collision balle-briques.
// Renvoie 1 et retourne l'angle quand choc sur une brique.
u32 Breaker_CollBricks(struct SBall *pBall, s32 *pnOldX, s32 *pnOldY);
// Déplacement de la balle.
void Breaker_MoveBall(void);
// Met l'anim de mort au joueur (dépend taille raquette).
void Breaker_PlayerSetDeath(void);
// Relache les balles aimantées à la raquette.
void Breaker_AimantReleaseBalls(void);
// Déplacement de la raquette du joueur.
void Breaker_MovePlayer(void); 
// Quand on a appuyer sur start. Contient toute les phases du jeu. 
void Breaker_Game(void); 
// Init pour une partie, récupére/initialise les paramétres de gExg.
void Breaker_ExgBrkInit(void);
// Englobe tous les paramètres du jeux.
void Breaker(void);
// Initialisation (appelé à chaque début de niveau (pas de vie, de niveau)).
void Breaker_Init(void);

// Init life, reset de la raquette.
void Breaker_PlyrInitLife(void);
// Init des slots des balles.
void Breaker_BallsInitSlots(void);
// Init d'une balle sur la raquette.
void Breaker_BallInitOnPlayer(void);
// Init d'une nouvelle balle.
void Breaker_BallInit(struct SBall *pBall, s32 nPosX, s32 nPosY, u32 nSize, u32 nFlags, s32 nSpeed,
                      u8 nAngle);
// Changement de la taille d'une balle.                
void Breaker_BallChangeSize(struct SBall *pBall, u32 nSize); 