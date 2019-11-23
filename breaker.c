
#include "includes.h"
#include "levels.h"

// Non géré: 
// * Espace entre les briques indestructible < taille max de la balle.
// Si la balle grossit alors qu'elle est entre, ça pose un pb.
// => Graphiquement, ça ne peut pas marcher. => Placer les briques indestructibles en fonction.

#define PILL_MAX 3 // Nb max de pillules à l'écran.

#define BALL_Speed_Min 0x200
#define BALL_Speed_Max 0x380
#define BALL_Speed_Step 0x800

// Pas plus grand que le rayon mini d'une balle ! Ne sert pas à l'augmentation de la vitesse.
#define BALL_DEPL_MAX 0x300 

u32 Breaker_BrickHit(u32 nBx, u32 nBy, u32 nBallFlags)
{
  u32 nRetVal = (u32)-1;
  // On regarde si les position de nBx et nBy son dans la table.
  if(nBx < TABLE_Width && nBy < TABLE_Height && gBreak.pLevel[(nBy * TABLE_Width) + nBx].nPres)
  {
    // On récupère les informations du slot de la brique demandé (pLevel est un tableau de struct SBrique
    // qui contient toutes les caractèristique de la brique).
    struct SBrique *pBrick = &gBreak.pLevel[(nBy * TABLE_Width) + nBx];

    // Décrémentation du compteur hit (nb de fois ou la brique est touché).
    // Si tombe à 0, la brique disparaît.
    // Brique indestructible ? Oui si la condition est == à 1.
    if((pBrick->nFlags & BRICK_Flg_Indestructible) == 0)
    {
      pBrick->nCnt--;
      // Balle traversante, on force le compteur à 0.
      if(nBallFlags & BALL_Flg_Traversante) 
        pBrick->nCnt = 0;
      if(pBrick->nCnt == 0)
      {
        pBrick->nPres = 0;
        Animspr_ReleaseSlot(pBrick->nAnmNo); // Slot mis à used = 0;
        pBrick->nAnmNo = -1; // Spécial pour les briques qui reviennent.

        // Dust.
        // Anime de destruction de la brique.
        Dust_DustSet(pBrick->pAnmExplo, WALL_XMin + (nBx * BRICK_Width), WALL_YMin + (nBy * BRICK_Height));

        // Génération d'item (Bonus).
        // Si 0 => on génère un bonus. 15 est pas mal car le nb de fois ou 0000 (mode binaire) apparaît est 
        // plus rare que 000.
        if((rand() & 7) == 0)
        // MstPill_GetNb donne le nombre de pillules présentes à l'écran.
        if(Monsters_InitPillGetNb() < PILL_MAX)
        {
          // Récupère le tableau des bonus du level en cours.
          u8 *pBon = gpBonuses[gBreak.nLevel];
          // Ajout aléatoire d'une pillule à l'écran.
          Mst_Add(e_Mst_Pill_0 + pBon[rand() % 32], 
                  WALL_XMin + (nBx * BRICK_Width) + (BRICK_Width / 2), 
                  WALL_YMin + (nBy * BRICK_Height) + (BRICK_Height / 2));
        }
        
        // Score.
        gBreak.nPlayerScore += pBrick->nScore;
        gBreak.nRemainingBricks--; // Une brique en moins.

        // Nb de briques qui doivent revenir.
        if(pBrick->nFlags & BRICK_Flg_ComingBack) gBreak.nBricksComingBackNbCur++;
      }
      else
      {
        // Anim de hit.
        pBrick->nAnmNo = Animspr_Set(pBrick->pAnmHit, pBrick->nAnmNo);
        printf("1\n");
      }
    }
    else
    {
      // Anim de hit de briques indestructible.
      pBrick->nAnmNo = Animspr_Set(pBrick->pAnmHit, pBrick->nAnmNo);
      printf("2\n");
    }
    nRetVal = (u32)pBrick->nFlags;
  }
  return nRetVal;
}

u32 Breaker_CollBricks(struct SBall *pBall, s32 *pnOldX, s32 *pnOldY)
{
  u32 nRetval = 0;

  s32 vx1, vx2, vy1, vy2;
  s32 nBXMin, nBXMax, nBYMin, nBYMax; // Coordonnées min et max de la brique en pixels.
  s32 i, j;
  u32 x, y;
  s32 nX, nY; // Position de la balle moins le rayon.
  s32 dx = 0, dy = 0;

  u32 cx, cy, coin;
  u32 nBFlags;

  // Numéros des briques extrêmes (celle en 1er ligne) à tester.
  // Testé avec une table à laplace de la divisioin, on ne gagne rien :
  // En faisant comme ça on évite de vérifier toute les briques à partir d'un tableau.
  vx1 = ((pBall->nPosX >> 8) - WALL_XMin - pBall->nRayon) / BRICK_Width;
  vx2 = ((pBall->nPosX >> 8) - WALL_XMin + pBall->nRayon) / BRICK_Width;
  vy1 = ((pBall->nPosY >> 8) - WALL_YMin - pBall->nRayon) / BRICK_Height;
  vy2 = ((pBall->nPosY >> 8) - WALL_YMin + pBall->nRayon) / BRICK_Height;

  cx = 0; cy = 0; coin = 0;
  // Boucle dans les briques potentiellement collisionable.
  // Sur la 1er boucle on regarde d'abord si il y a collision sur la brique.
  for(j = vy1; j <= vy2; j++)
  {
    nBYMin = (j * BRICK_Height) + WALL_YMin;
    nBYMax = (j * BRICK_Height) + WALL_YMin + BRICK_Height - 1;
    for(i = vx1; i <= vx2; i++)
    {
      // Une brique présente ?
      // Ici on prend le rang i, regarde si < à la longueur du plateau de jeu (idem pour j) puis vérifie
      // si dans le plateau de jeu la brique est présente.
      if((u32)i < TABLE_Width && (u32)j < TABLE_Height && gBreak.pLevel[(j*TABLE_Width) + i].nPres)
      {
        // Test de la collision.
        nBXMin = (i * BRICK_Width) + WALL_XMin;
        nBXMax = (i * BRICK_Width) + WALL_XMin + BRICK_Width - 1;
        // Test avec chaque pixel du masque de la balle.
        nY = (pBall->nPosY >> 8) - pBall->nRayon;
        for(y = 0; y < pBall->nDiam; y++)
        {
          nX = (pBall->nPosX >> 8) - pBall->nRayon;
          for(x = 0; x < pBall->nDiam; x++)
          {
            // Pixel à tester ?
            if(pBall->pBallMask[(y * BALL_GfxLg) + x])
            {
              // Si le pixel de la balle est dans la box.
              if(nX >= nBXMin && nX <= nBXMax && nY >= nBYMin && nY <= nBYMax)
              {
                u32 nFlg = 1; // b0. Note de moi : pour l'instant je ne sais pas, mais je vais dire si nFlg
                // est à 1 pas de flag. Si 2 il y a un flag.
                // Gestion du choc sur la brique.
                nBFlags = Breaker_BrickHit(i, j, pBall->nFlags);
                if(nBFlags & BRICK_Flg_Indestructible) nFlg = 2; // b1.

                // Choc.
                // On touche la brique de face, sur les côtés ou sur un angle ?
                if(pBall->nPosX >> 8 >= nBXMin && pBall->nPosX >> 8 <= nBXMax)
                {
                  cy |= nFlg;
                }
                else if(pBall->nPosY >> 8 >= nBYMin && pBall->nPosY >> 8 <= nBYMax)
                {
                  cx |= nFlg;
                }
                else
                {
                  coin |= nFlg;
                  dx = (pBall->nPosX >> 8) - (nBXMin + (BRICK_Width / 2));
                  dy = (pBall->nPosY >> 8) - (nBYMin + (BRICK_Height / 2));
                }
                y = 1000; x = 1000; break; // Sortie des boucles.
              }
            }
            nX++;
          } // for(x = 0; x < pBall->nDiam; x++)
          nY++;
        } // for(y = 0; y < pBall->nDiam; y++)
      } // if brique présente ?
    } // for (i = vx1; i <= vx2; i++)
  } // for (j = vy1; j <= vy2; j++)
  dy++;
  dx++;
  return nRetval;
}

void Breaker_MoveBall(void)
{
  s32 nAddX, nAddY; // Ajoute à pBall->nPosX et Y de combien avance la balle.
  s32 nOldX, nOldY; // Sauvegarde de l'ancienne position (pBall->nPosX et Y).
  s32 nRemSpd; // Pour les opérations sur la vitesse.
  s32 nSpd; // Le coeficient de rapidité de la balle.
  struct SBall *pBall;
  u32 i;

  for(i = 0; i < BALL_MAX_NB; i++)
  {
    pBall = &gBreak.pBalls[i];
    if(pBall->nUsed)
    {
      // La balle est-elle collée à la raquette ?
      if(pBall->nFlags & BALL_Flg_Aimantee)
      {
        // Note : offset == pt d'origine de la balle.
        // Je mettrais offset de la balle par rapport à l'offset de la raquette collé sur la raquette.
        // Récupère la position de la balle.
        // A partir du centre de la balle jusqu'au milieu de la raquette.
        // nOffsRaq est obtenu plus bas dans la fonctoin et quand la balle est collée on à la valeur.
        pBall->nPosX = (gBreak.nPlayerPosX + pBall->nOffsRaq) << 8;
        pBall->nPosY = (gBreak.nPlayerPosY - pBall->nRayon - 1) << 8;

        // Recalage si la balle passe dans les murs.
        // Note : la balle est remise à l'extrémité de la raquette.
        // Note : on repositionne la balle avant de toucher le mur soit à une distance de l'offset de la
        // balle.
        // Rappel : la position de la balle se fait par rapport à l'offset.

        // Mur droit.
        if(pBall->nPosX >> 8 > WALL_XMax - (s32)pBall->nRayon)
        {
          // Soustrait à nOffsRaq le dépassement de la ball.
          pBall->nOffsRaq -= (pBall->nPosX >> 8) - (WALL_XMax - (s32)pBall->nRayon);
          pBall->nPosX = (gBreak.nPlayerPosX + pBall->nOffsRaq) << 8;
        }
        // Mur gauche.
        if(pBall->nPosX >> 8 < WALL_XMin + (s32)pBall->nRayon)
        {
          pBall->nOffsRaq -= (pBall->nPosX >> 8) - (WALL_XMin + (s32)pBall->nRayon);
          pBall->nPosX = (gBreak.nPlayerPosX + pBall->nOffsRaq) << 8;
        }
      }
      else 
      {
        // Balle en mouvement.
        // Note : pour modifié la vitesse de la balle augmenter Ball_Speed_Min.
        // Note : nBall->nSpeed est initialisé au départ dans 
        // ExgBrkInit->BreakerInit->BallInitOnPlayer->BallInit à la valeur Ball_Speed_Min. 
        // Note nRemSpd à pour but de remettre le speed à la valeur BALL_DEPL_MAX.
        nRemSpd = pBall->nSpeed; 
        while(nRemSpd)
        {
          if(nRemSpd > BALL_DEPL_MAX)
          {
            // En cours.
            nSpd = BALL_DEPL_MAX;
            nRemSpd -= BALL_DEPL_MAX;
          }
          else
          {
            // Dernier tour.
            nSpd = nRemSpd;
            nRemSpd = 0;
          }
          // Note : pBall->nAngle est initialisé au départ dans 
          //        ExgBrkInit->BreakerInit->BallInitOnPlayer->BallInit à la valeur 48.
          // Note : nOldX et Y servent au replacement de la balle si il y a colision.

          // Déplacement.
          nOldX = pBall->nPosX;
          nOldY = pBall->nPosY;
          nAddX = (gVar.pCos[pBall->nAngle] * nSpd) >> 8;
          nAddY = (gVar.pSin[pBall->nAngle] * nSpd) >> 8;
          pBall->nPosX += nAddX;
          pBall->nPosY += nAddY;

          // Si colision, on repart de l'ancienne position.
          if(Breaker_CollBricks(pBall, &nOldX, &nOldY))
          {

          }
        }
      }
    }
  }
}

void Breaker_PlayerSetDeath(void)
{
  // Mort du joueur, sauf si flag (passage de la porte, boss tué...).
  if((gBreak.nPlayerFlags & PLAYER_Flg_NoKill) == 0)
  {
    // Joueur : Anim de mort.
    u32 *pDeath[] = {gAnm_RaqDeath0, gAnm_RaqDeath1, gAnm_RaqDeath2, gAnm_RaqDeath3};
    Animspr_AnmSetIfNew(pDeath[gBreak.nPlayerRSize], gBreak.nPlayerAnmNo);
  }
}

void Breaker_AimantReleaseBalls(void)
{
  struct SBall *pBall;
  u32 i;
  u32 nRel = 0; // Slt pour le son.

  for(i = 0; i < BALL_MAX_NB; i++)
  {
    pBall = &gBreak.pBalls[i];
    printf("pBall->nUsed = %d\n", pBall->nUsed);
    if(pBall->nUsed)
      if(pBall->nFlags & BALL_Flg_Aimantee) // Test seulement pour le son.
      {
        pBall->nFlags &= BALL_Flg_Aimantee; // Coupe l'aimant sur la balle.
        nRel = 1; 
      }
  }
  // Sfx.
  if(nRel) Sfx_PlaySfx(e_Sfx_BatPing, e_SfxPrio_10);
}

void Breaker_MovePlayer(void)
{
  s32 i;
  struct SSprite *pSpr = Sprites_GetDesc(Animspr_GetLastImage(gBreak.nPlayerAnmNo));
  s32 nXMin, nXMax;

  // Dans la porte ?
  if(gBreak.nPlayerFlags & PLAYER_Flg_DoorR)
  {
    // Si raquette aimanté => On lache les balles. On ne s'occupe pas du flag => Simplifie la gestion.
    Breaker_AimantReleaseBalls();
    // Force le déplacement.
    gBreak.nPlayerPosX++;

    // Complétement passé ?
    if(gBreak.nPlayerPosX - pSpr->nPtRefX > WALL_XMax + 8)
      gBreak.nPhase = e_Game_LevelCompleted;

    // Dans la porte, plus de clics.
    gVar.nMouseButtons = 0;

    return;
  }

  // Mort.
  if(Animspr_GetKey(gBreak.nPlayerAnmNo) == e_AnmKey_PlyrDeath)  
  {
    // Explosion fini ?
    if(Animspr_CheckEnd(gBreak.nPlayerAnmNo))
    {
      // Reste des vies ?
      if(gBreak.nPlayerLives)
      {
        gBreak.nPlayerLives--;
        //Reset joueur.
        Breaker_PlyrInitLife();
        // Balle, à faire apparaître après l'anim d'apparition. Ou alors, pendant l'anim d'apparition,
        // ne pas afficher de balle.
        Breaker_BallInitOnPlayer();
      }
      else
      {
        // Game over. TODO faire un continue.
        gBreak.nPhase = e_Game_GameOver;
      }      
    }
    // On ne bouge pas pendant la mort.
    gVar.nMouseButtons = 0; // Empèche les clics.
    return;
  }

  // On se prend un tir ? Seulement dans le niveau du boss, même routine que les monstres.
  if(gBreak.nLevel == LEVEL_Max - 1)
  {
    printf("gBreak.nPlayerAnmNo = %d\n", gBreak.nPlayerAnmNo);
    if(Fire_MstCheckFire(Animspr_GetLastImage(gBreak.nPlayerAnmNo), gBreak.nPlayerPosX, gBreak.nPlayerPosY))
    {
      Breaker_PlayerSetDeath();
      Breaker_BallsInitSlots(); 
    }

    // Attente du Boss ? pas de clics.
    if(gBreak.nPlayerFlags & PLAYER_Flg_BossWait)
      gVar.nMouseButtons = 0;
  }

  i = gVar.nMousePosX;
  nXMin = i - pSpr->nPtRefX;
  nXMax = nXMin + pSpr->nLg;
  if(nXMin < WALL_XMin) i = WALL_XMin + pSpr->nPtRefX; // Recalage de la raquette.
  if(nXMax > WALL_XMax) i = WALL_XMax - pSpr->nLg + pSpr->nPtRefX; // Idem.
  gBreak.nPlayerPosX = i;
}

void Breaker_Game(void)
{
  struct SSprite *pSpr;
  // u32 i;
  // s32 nDiff;
  // static u8 nWait = 0;
  u32 nLg = 0;
  
  switch(gBreak.nPhase) // nPhase initialiser dans Breaker_Init.
  {
    case e_Game_SelectLevel:
      // TODO : recommencer au level ou l'on a perdu.
      gBreak.nPhase = e_Game_Normal;
      break;
    case e_Game_Normal: 
      // Affichage de la phrase "Level xx".
      if(gBreak.nTimerLevelDisplay) // Init dans Breaker_PlyrInitLife.
      {
        char pStrSelect[] = "LEVEL 00";
        Font_MyItoA(gBreak.nLevel + 1, &pStrSelect[6]); // Init dans Breaker_ExgBrkInit.
        nLg = Font_Print(0, 0, pStrSelect, FONT_NoDispo, e_Spr_FontSmall); // Centrage.
        Font_Print(SCR_Width/2 - nLg/2, 200, pStrSelect, 0, e_Spr_FontSmall); 

        gBreak.nTimerLevelDisplay--;
        // Coupe le timer. Enlève la phrase.
        if(gVar.nMouseButtons & MOUSE_BtnLeft) gBreak.nTimerLevelDisplay = 0; 
      }

      Breaker_MovePlayer();

      // Clic souris ? Lancement initial de  la balle.
      if(gVar.nMouseButtons & MOUSE_BtnLeft)
      {
        // Aimant ? (Sert aussi au lancement initial de la balle).
        Breaker_AimantReleaseBalls();

        // Mitrailleuse ? // nPlayerFlags initialisé à 0 dans breaker_Init sinon init dans
        // BonusSetMitrailleuse.
        if(gBreak.nPlayerFlags & PLAYER_Flg_Mitrailleuse)
        {
          // nPlayerAnmBonusD Initialisation dans breaker_Init.
          gBreak.nPlayerAnmBonusD = Animspr_AnmSetIfNew(gAnm_RaqMitDShoot, gBreak.nPlayerAnmBonusD);
          gBreak.nPlayerAnmBonusG = Animspr_AnmSetIfNew(gAnm_RaqMitGShoot, gBreak.nPlayerAnmBonusG);
          // Balance les tirs.
          pSpr = Sprites_GetDesc(Animspr_GetLastImage(gBreak.nPlayerAnmNo));
          // +2, -2 ajustement des tires avec la raquette.
          Fire_Add(0, gBreak.nPlayerPosX - pSpr->nPtRefX + 2, gBreak.nPlayerPosY - 2, -1);
          Fire_Add(0, gBreak.nPlayerPosX - pSpr->nPtRefX + pSpr->nLg - 1 - 2, gBreak.nPlayerPosY - 2, - 1);
          Sfx_PlaySfx(e_Sfx_Shot, e_SfxPrio_20); // Pas dans l'anime car pas forcément remise à zéro.
        }
      }

      Breaker_MoveBall();

      break;
    case e_Game_GameOver:
      break;
    case e_Game_LevelCompleted:
      break;
    case e_Game_Pause:
      // Normalement, on ne passe jamais ici.
      break;

      Breaker_Init();
    break;
  }
}


void Breaker_ExgBrkInit(void)
{
  gBreak.nLevel = 0; // Level_Max-1;
  gBreak.nPlayerLives = PLAYER_Lives_Start;  // Nb de vies au départ.
  gBreak.nPlayerScore = 0;

  gExg.nExitCode = 0;
  Breaker_Init();
}

void Breaker_InitLevel(u32 nLevel)
{
  u32 i;
  // Sur le level en cours. Choix du level en début de jeu.
  s8 *pLev = gpLevels[nLevel]; 
  // Pts des monstres.
  u16 pScores[] = {50, 60, 70, 80, 90, 100, 110, 120, 100, 100, 100, 100, 0};

  // Initialisation de la table des briques.
  gBreak.nRemainingBricks = 0;
  gBreak.nBricksComingBackNbCur = 0;
  gBreak.nBricksComingBackTotal = 0;
  for(i = 0; i < TABLE_Width * TABLE_Height; i++)
  {
    // Voir la construction des niveaux avec des valeurs numériques (voir level.h).
    // -1 pas de birque.
    if(*(pLev + i) != -1)
    {
      gBreak.pLevel[i].nPres = 1; // Brique présente ou pas.
      gBreak.pLevel[i].nCnt = 1; // Nb de touches restantes avant la destruction.
      gBreak.pLevel[i].nFlags = 0; // Flags : voir liste.
      gBreak.pLevel[i].nResetCnt = 0; // Compteur pour retour de la brique.
      
      gBreak.pLevel[i].nSprNo = e_Spr_Bricks + *(pLev + i); // Sprite par défaut.
      gBreak.pLevel[i].nScore = pScores[*(pLev + i)]; // Scores.
      gBreak.pLevel[i].nAnmNo = -1; // On remplace par l'anim si != -1.
      
      gBreak.pLevel[i].pAnmExplo = gAnm_BrickExplo; // Anim à utiliser pour la disparition.
      // Anim à utiliser pour le hit. Ne sert pas pour les briques normales.
      gBreak.pLevel[i].pAnmHit = gAnm_Brick2Hit; 

      gBreak.nRemainingBricks++;

      // Cas particuliers.
      switch(*(pLev + i))
      {
        // Brique à toucher 2 fois.
        case e_Spr_BricksSpe: 
          gBreak.pLevel[i].nCnt = 2; 
          gBreak.pLevel[i].pAnmExplo = gAnm_Brick2HitExplo; 
          gBreak.pLevel[i].pAnmHit = gAnm_Brick2Hit; 
          break;

        // Brique qui revient.
        case e_Spr_BricksSpe + 1: 
          gBreak.pLevel[i].nCnt = 2; 
          gBreak.pLevel[i].nResetCnt = BRICK_ComingBackCnt; 
          gBreak.pLevel[i].nFlags |= BRICK_Flg_ComingBack;
          gBreak.nBricksComingBackTotal++;
          gBreak.pLevel[i].pAnmExplo = gAnm_BrickCBExplo; 
          gBreak.pLevel[i].pAnmHit = gAnm_BrickCBHit; 
          break;

        // Brique indestructible.
        case e_Spr_BricksSpe + 2: 
          gBreak.pLevel[i].nFlags |= BRICK_Flg_Indestructible;
          gBreak.nRemainingBricks--; // Celles là, elles ne comptent pas.
          gBreak.pLevel[i].pAnmHit = gAnm_BrickIndesHit; 
          break;
        
        default:
          break;
      }
    }
    else
    {
      // Pas de brique.
      gBreak.pLevel[i].nPres = 0; 
      gBreak.pLevel[i].nFlags = 0; 
      // Break.pLevel[i].nResetCnt = 0; // Compteur pour retour de la brique.
    }
  }

  if(gBreak.nLevel == LEVEL_Max - 1)
  {
    // Boss de fin.
    // Ptr sur le décor.
    gVar.pLevel = gVar.pLev[4];
    // Rajoute Doh !
    Mst_Add(e_Mst_Doh, SCR_Width / 2, 98);  //  -> Ici que sa plante.
    gBreak.nRemainingBricks = 1; // Quand on tuera le boss, il décrémentera le nb de briques.
  }
  else
  {
    // Niveau normaux.
    // Pointeur sur le décor.
    gVar.pLevel = gVar.pLev[gBreak.nLevel & 3];
    // Rajoute un générateur de monstres.
    Mst_Add(e_Mst_Generateur, 0, 0);
  }
  // Rajoute une porte à droite.
  Mst_Add(e_Mst_DoorR, WALL_XMax, SCR_Height - 13 - 6);
}

void Breaker_Init(void)
{
  Mst_InitEngine();
  Animspr_InitEngine();
  Fire_InitEngine();
  Dust_InitEngine();
  Breaker_InitLevel(gBreak.nLevel); // gBreak.nLevel init à 0 (voir Breaker_ExgBrkInit).

  srand(time(NULL)); // Init hasard.
  gBreak.nPhase = e_Game_SelectLevel; // Ou e_Game_Normal.
  // gBreak.nTimerGameOver = TIMER_GameOver; // Countdown pour game over.

  // Taille de la raquette.
  gBreak.nPlayerRSize = 1;

  // Réservation des anims (anim pas importante, réaffécter plus tard. Pour réservation d'un slot).
  gBreak.nPlayerAnmNo = Animspr_Set(gAnm_Raquette, -1);
  printf("3\n");
  gBreak.nPlayerAnmBonusM = Animspr_Set(gAnm_RaqAimant, -1);
  printf("4\n");
  gBreak.nPlayerAnmBonusD = Animspr_Set(gAnm_RaqMitDRepos, -1); // Mit == Mittrailleuse.
  printf("5\n");
  gBreak.nPlayerAnmBonusG = Animspr_Set(gAnm_RaqMitGRepos, -1);
  printf("6\n");
  gBreak.nPlayerAnmClignG = Animspr_Set(gAnm_RaqClignG, -1);
  printf("7\n");
  gBreak.nPlayerAnmClignD = Animspr_Set(gAnm_RaqClignD, -1);
  printf("8\n");

  gBreak.nPlayerFlags = 0;
  if(gBreak.nLevel == LEVEL_Max -1) // Patch. 1 fois à l'init du level, pas de la vie.
  {
    gBreak.nPlayerFlags |= PLAYER_Flg_BossWait; // Le joueur attendra que le boss soit prêt.
  }
  Breaker_PlyrInitLife();

  Breaker_BallsInitSlots();
  Breaker_BallInitOnPlayer();
}

void Breaker_BallChangeSize(struct SBall *pBall, u32 nSize)
{
  struct SSprite *pSpr;
  
  // Choix du sprite suivant la taille.
  pBall->nSize = nSize; 
  pBall->nSpr = (pBall->nFlags & BALL_Flg_Traversante ? e_Spr_BallTrav : e_Spr_Ball) + pBall->nSize;
  pSpr = Sprites_GetDesc(pBall->nSpr);
  pBall->nRayon = pSpr->nLg / 2; // Offset du centre. / Note : lg impaires.
  pBall->nDiam = pSpr->nHt; 

  // Clear le masque pBall->pBallMask , obligatoire car risque trace ancienne valeur d'un autre prog.
  for(int i = 0; i < BALL_GfxLg * BALL_GfxLg; i++) pBall->pBallMask[i] = 0;
  // Copie du masque. Inversion des bits (sans doute pour collision ?).
  for(int j = 0; j < pSpr->nHt; j++)
    for(int i = 0; i < pSpr->nLg; i++)
      pBall->pBallMask[(BALL_GfxLg * j) + i] = ~pSpr->pMask[(pSpr->nHt * j) + i];
}

void Breaker_BallInit(struct SBall *pBall, s32 nPosX, s32 nPosY, u32 nSize, u32 nFlags, s32 nSpeed, u8 nAngle)
{
  pBall->nUsed = 1; gBreak.nBallsNb++;

  pBall->nFlags = nFlags;
  pBall->nPosX = nPosX;
  pBall->nPosY = nPosY;
  pBall->nSpeed = nSpeed;
  pBall->nAngle = nAngle;

  Breaker_BallChangeSize(pBall, nSize);

  pBall->nOffsRaq = 0;
}

void Breaker_BallInitOnPlayer(void)
{
  struct SBall *pBall;

  pBall = &gBreak.pBalls[0];
  // Vitesse : tous les 4 niveaux, ça part un peu plus vite.
  // Positionne la balle sur la raquette.
  Breaker_BallInit(pBall, gBreak.nPlayerPosX << 8, gBreak.nPlayerPosY << 8, 0, BALL_Flg_Aimantee,
           BALL_Speed_Min + ((gBreak.nLevel >> 2) * 0x18), 48);
  pBall->nPosY -= (pBall->nRayon + 1) << 8;
}

void Breaker_BallsInitSlots(void)
{
  u32 i;

  gBreak.nBallsNb = 0;
  for(i = 0; i < BALL_MAX_NB; i++)
  {
    gBreak.pBalls[i].nUsed = 0;
  }
}

void Breaker_PlyrInitLife(void)
{
  // Reset des flags.
  gBreak.nPlayerFlags &= ~(PLAYER_Flg_Aimant | PLAYER_Flg_Mitrailleuse);
  gBreak.nPlayerFlags &= ~PLAYER_Flg_NoKill;
  gBreak.nPlayerFlags &= ~PLAYER_Flg_DoorR;

  // Taille de la raquette.
  gBreak.nPlayerRSize = 1;

  // Raquette normale.
  gBreak.nPlayerAnmNo = Animspr_Set(gAnm_RaqAppear, gBreak.nPlayerAnmNo);
  printf("9\n");
  gBreak.nPlayerAnmClignD = Animspr_Set(gAnm_RaqClignD, gBreak.nPlayerAnmClignD);
  printf("10\n");
  gBreak.nPlayerAnmClignG = Animspr_Set(gAnm_RaqClignG, gBreak.nPlayerAnmClignG);
  printf("11\n");
  // Positionnement au centre.
  gBreak.nPlayerPosX = SCR_Width / 2;
  gBreak.nPlayerPosY = SCR_Height - 17;

  // Replace la souris à l'endroit du joueur.
  // SDL_WarpMouse(gBreak.nPlayerPosX, gBreak.nPlayerPosY);

  gBreak.nTimerLevelDisplay = TIMER_DisplayLevel; // Compteur pour affichage du n° de level.
}

void Breaker(void)
{
  // u32 nLastScore = gBreak.nPlayerScore;

  if(gBreak.nPhase == e_Game_Pause) return;

  Breaker_Game();

}

