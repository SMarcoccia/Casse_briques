#include "includes.h"

//#define MENU_Bkg_Mvt 
#define FADE_Step 8 

#define FX_Menu_Move e_Sfx_MenuClic
#define FX_Menu_Clic e_Sfx_BatMagnet 

#define MENU_Main_StartLn 160

#define CURS_Acc 0x200

#define BOUF2_Nb 10
#define HISC_Nb 10
#define HISC_Filename "highScore.src"
#define MENU_HiSC_Interligne 19
#define HISC_LineLg (34+1)
#define ADJUST_VAR 0 // Pour augmenter la hitbox du menu collision avec la souris.

struct SScore{
  char pName[HISC_NameLg];
  u16 nRound;
  u32 nScore;
   u8 nRank; 
};
struct SScore gpHighScores[HISC_Nb];

struct SMenuItm
{
  u32 nMenuVal; // Choix pour le menu.
  u32 nLg; // Largeur en pixels du text (centrage).
  u32 nLgEffect; // Largeur en pixels du text avec effect (centrage).
  char *pTxt;
};




//=======================================================================================================
//==============================================LOAD SCORE===============================================
//=======================================================================================================


// Scores - Check si un score entre au Hall of Fame.
// Out : -1, pas dedans / >= 0, rang.
s32  Menu_CheckHighScr(u32 nScorePrm)
{
  s32 i, nRank;
  nRank = -1;
  for(i = HISC_Nb -1; i >= 0; i--)
  {
    if(nScorePrm >= gpHighScores[i].nScore)
    {
      nRank = i; // Indice du rang;
    }
  }
  return nRank;
}

u32 Scr_CalcChecksum(void)
{
  u32 i, j;
  u32 nChk=0;
  
  for(i = 0; i<HISC_Nb; i++)
  {
    nChk += gpHighScores[i].nRound;
    nChk += gpHighScores[i].nScore;

    for(j=0; j<HISC_NameLg; j++)
      nChk += (u32)gpHighScores[i].pName[j] << (8 *(j & 3));
  }

  return nChk;
}

void Menu_Scr_RazTable(void)
{
  char pDefault[HISC_NameLg] = "----------------";
  u32 i;

  for(i = 0; i < HISC_Nb; i++)
  {
    strcpy(gpHighScores[i].pName, pDefault);
    gpHighScores[i].nRound = 0;
    gpHighScores[i].nScore = 0;
    gpHighScores[i].nRank = i;
  }
}

void Menu_ScrLoad()
{
  FILE *pFile=NULL;
  u32 nChk;
  
  if((pFile = fopen(HISC_Filename, "r")) != NULL)
  {
    fread(gpHighScores, sizeof(struct SScore), HISC_Nb, pFile);
    fread(&nChk, sizeof(u32), 1, pFile); // Normalement dans la sauvegarde des high-scores il doit y avoir un tcheksum.
    fclose(pFile);

    //checksum.
    if(nChk != Scr_CalcChecksum())
    {
      // Wrong checksum, RAZ table.
      printf("Menu_ScrLoad : Wrong checksum! Restting table.\n");
      Menu_Scr_RazTable();
    }
  }
  else
  {
    // Le fichier n'existe pas, RAZ table.
    Menu_Scr_RazTable();
  }
}

// Sauvegarde des high-scores.
void Menu_ScrSave(void)
{
  FILE *pFile = NULL;
  u32 nChk = 0;

  if((pFile = fopen(HISC_Filename, "w")) == NULL)
  {
    printf("Unable to save highscores table.\n");
    return;
  }
  // Sauvegarde des fichiers des high-scores.
  fwrite(gpHighScores, sizeof(struct SScore), HISC_Nb, pFile);
  nChk = Scr_CalcChecksum();
  fwrite(&nChk, sizeof(u32), 1, pFile);
  fclose(pFile);
}


//=======================================================================================================
//=========================================MENU DES HIGH-SCORES==========================================
//=======================================================================================================


struct SBouffonerie1 // Fx sur les lignes.
{
  u32 nWait;
  s32 nSpdMax;
  s32 nSpd;
  s32 nPosX;
};
struct SBouffonerie1 gpBouf1[HISC_Nb];

struct SBouffonerie2
{
  s32 nAnmNo; // N° du sprite d'un ennemie d'une animation (animation contient plusieurs sprites).
  u32 nSens; // 0:Bas / 1:Haut / 2:Droite / 3:Gauche.
  u32 nWait, nWait2;
  s32 nSpd; // Vitesse du sprite.
  s32 nPosX, nPosY;
  s32 nLastPosX, nLastPosY;
  s32 nDstX1, nDstY1;
  u32 nChoiceAlgo; 
  s32 nVar; // Variable quelconque.
  s32 nVar2; // idem.
  s32 nVar3; // idem.
  s32 nVarY; // Pour la conversion d'échelle.
  double nEchX, nEchY; // Conversion d'échelle.

  //Ligne.
  s32 ex, ey; // Variable d'erreur, pour la ligne. Coef directeur, ey : Numérateur, ex : Dénominateur.
  s32 dx, dy;  // Resultat pour le coeficient directeur numérateur (dx) et dénominateur (dy).
  s32 Dx, Dy;  // idem mais sera modifié lors de la construction de la ligne.
  s32 nIncr; // incrémentation. Pemret la construction de la ligne pixel par pixel.
  s32 Xincr, Yincr; 
  u32 nXorigin, nYorigin; // Coordonnées du pixel de départ.

  // Cercle.
  s32 XincrCirc, YincrCirc; // Incrémentation pour la construction du cercle pixel par pixel.
  s32 nSensChange; // Pour les lignes au départ. Empêche le changement de sprites lors du changement de sens.
  s32 e; // Variable d'erreur, pour le cercle.
  s32 rx, ry; // Rayon.
  s32 nValCirc; // Variable utiliser pour réinitialisation des valeurs pour le cercle.
  u32 nValQuad; // Pour passer au quadrant suivant.
  
  // Fonction.
  s32 XincrFct, YincrFct; // Incréments pour la construction pixel par pixel de la fonction.
  //Changement d'échelle pour les functions. A faire varié.
  double XMin, XMax;
  double YMin, YMax;
};

struct SBouffonerie2 gpBouf2[BOUF2_Nb]; // Contiendra le nombre de sprites pour l'animation.

// Effet pour les monstres.

u32 quadrant(u32 i)
{
    if(gpBouf2[i].XincrCirc <= gpBouf2[i].YincrCirc)
    {
      gpBouf2[i].e+=8*gpBouf2[i].XincrCirc+4;
      gpBouf2[i].XincrCirc++;

      if(gpBouf2[i].e > 0)
      {
        gpBouf2[i].YincrCirc--;
        gpBouf2[i].e-=8*gpBouf2[i].YincrCirc;
      }
    }
    else if(gpBouf2[i].YincrCirc > 0)
    {
      if(gpBouf2[i].nValCirc)
      {
        gpBouf2[i].e=5-4*gpBouf2[i].ry;  
        gpBouf2[i].nValCirc = 0;      
      }
      gpBouf2[i].e+=8*gpBouf2[i].YincrCirc+4;
      gpBouf2[i].YincrCirc--;

      if(gpBouf2[i].e > 0)
      {
        gpBouf2[i].XincrCirc++;
        gpBouf2[i].e-=8*gpBouf2[i].XincrCirc;
      }
    }
    else
      {
        gpBouf2[i].nValCirc = 1;
        return 0;
      }

    gpBouf2[i].nPosX = (gpBouf2[i].XincrCirc + gpBouf2[i].nDstX1 + gpBouf2[i].rx);
    gpBouf2[i].nPosY = (gpBouf2[i].YincrCirc + gpBouf2[i].nDstY1);
    return 1;
}

u32 quadrant2(u32 i)
{
    if(gpBouf2[i].nValCirc == 1)
    {
      gpBouf2[i].YincrCirc = gpBouf2[i].ry;
      gpBouf2[i].XincrCirc=0;
      gpBouf2[i].e=5-4*gpBouf2[i].ry;
      gpBouf2[i].nValCirc = -1;
    }

    if(gpBouf2[i].XincrCirc <= gpBouf2[i].YincrCirc)
    {
      gpBouf2[i].e+=8*gpBouf2[i].XincrCirc+4;
      gpBouf2[i].XincrCirc++;

      if(gpBouf2[i].e>0)
      {
        gpBouf2[i].YincrCirc--;
        gpBouf2[i].e-=8*gpBouf2[i].YincrCirc;
      }
    }
    else if(gpBouf2[i].YincrCirc > 0)
    {
      // static u32 nVal = 1;
      if(gpBouf2[i].nValCirc == -1)
      {
        gpBouf2[i].e=5-4*gpBouf2[i].ry;
        gpBouf2[i].nValCirc = 0;
      }

      gpBouf2[i].e+=8*gpBouf2[i].YincrCirc+4;
      gpBouf2[i].YincrCirc--;

      if(gpBouf2[i].e>0)
      {
        gpBouf2[i].XincrCirc++;
        gpBouf2[i].e-=8*gpBouf2[i].XincrCirc;
      }
    }
    else
      {
        gpBouf2[i].nValCirc = 1;
        return 0;
      }

    gpBouf2[i].nPosX = (gpBouf2[i].YincrCirc + gpBouf2[i].nDstX1 + gpBouf2[i].rx);
    gpBouf2[i].nPosY = (-gpBouf2[i].XincrCirc + gpBouf2[i].nDstY1);
    return 2;
}

u32 quadrant3(u32 i)
{
    // static u32 nVal = 1;
    if(gpBouf2[i].nValCirc == 1)
    {
      gpBouf2[i].YincrCirc = gpBouf2[i].ry;
      gpBouf2[i].XincrCirc=0;
      gpBouf2[i].e=5-4*gpBouf2[i].ry;
      gpBouf2[i].nValCirc = -1;
    }

    if(gpBouf2[i].XincrCirc <= gpBouf2[i].YincrCirc)
    {
      gpBouf2[i].e+=8*gpBouf2[i].XincrCirc+4;
      gpBouf2[i].XincrCirc++;

      if(gpBouf2[i].e>0)
      {
        gpBouf2[i].YincrCirc--;
        gpBouf2[i].e-=8*gpBouf2[i].YincrCirc;
      }
    }
    else if(gpBouf2[i].YincrCirc > 0)
    {
      if(gpBouf2[i].nValCirc == -1)
      {
        gpBouf2[i].e=5-4*gpBouf2[i].ry;
        gpBouf2[i].nValCirc = 0;
      }
      gpBouf2[i].e+=8*gpBouf2[i].YincrCirc+4;
      gpBouf2[i].YincrCirc--;

      if(gpBouf2[i].e>0)
      {
        gpBouf2[i].XincrCirc++;
        gpBouf2[i].e-=8*gpBouf2[i].XincrCirc;
      }
    }
    else
      {
        gpBouf2[i].nValCirc = 1;
        return 0;
      }
    
    gpBouf2[i].nPosX = (-gpBouf2[i].XincrCirc + gpBouf2[i].nDstX1 + gpBouf2[i].rx);
    gpBouf2[i].nPosY = (-gpBouf2[i].YincrCirc + gpBouf2[i].nDstY1);
    return 3;
}

u32 quadrant4(u32 i)
{
  // static u32 nVal = 1;
  if(gpBouf2[i].nValCirc == 1)
  {
    gpBouf2[i].YincrCirc = gpBouf2[i].ry;
    gpBouf2[i].XincrCirc=0;
    gpBouf2[i].e=5-4*gpBouf2[i].ry;
    gpBouf2[i].nValCirc = -1;
  }

  if(gpBouf2[i].XincrCirc <= gpBouf2[i].YincrCirc)
  {
    gpBouf2[i].e+=8*gpBouf2[i].XincrCirc+4;
    gpBouf2[i].XincrCirc++;

    if(gpBouf2[i].e>0)
    {
      gpBouf2[i].YincrCirc--;
      gpBouf2[i].e-=8*gpBouf2[i].YincrCirc;
    }
  }
  else if(gpBouf2[i].YincrCirc > 0)
  {
    if(gpBouf2[i].nValCirc == -1)
    {
      gpBouf2[i].e=5-4*gpBouf2[i].ry;
      gpBouf2[i].nValCirc = 0;
    }

    gpBouf2[i].e+=8*gpBouf2[i].YincrCirc+4;
    gpBouf2[i].YincrCirc--;

    if(gpBouf2[i].e>0)
    {
      gpBouf2[i].XincrCirc++;
      gpBouf2[i].e-=8*gpBouf2[i].XincrCirc;
    }
  }
  else
    {
      gpBouf2[i].nValCirc = 1;
      return 0;
    }

  gpBouf2[i].nPosX = (-gpBouf2[i].YincrCirc + gpBouf2[i].nDstX1 + gpBouf2[i].rx);
  gpBouf2[i].nPosY = (gpBouf2[i].XincrCirc + gpBouf2[i].nDstY1);
  return 4;
}

u32 Line(u32 i)
{
  // Ligne horizontale.
  if(gpBouf2[i].dy == 0)
  {
    if(gpBouf2[i].nIncr < gpBouf2[i].Dx)
      {
        gpBouf2[i].nIncr+=gpBouf2[i].nSpd;
        gpBouf2[i].nPosX += gpBouf2[i].Xincr;
      }
    else
      return 0;
  }
  
  // Coef < 1. 
  else if(gpBouf2[i].Dx > gpBouf2[i].Dy)
  {
    if(gpBouf2[i].nIncr < gpBouf2[i].Dx)
    {
      gpBouf2[i].nIncr+=gpBouf2[i].nSpd;
      gpBouf2[i].nPosX += gpBouf2[i].Xincr;
      gpBouf2[i].ex -= gpBouf2[i].dy;

      if(gpBouf2[i].ex < 0)
      {
        gpBouf2[i].nPosY += gpBouf2[i].Yincr;
        gpBouf2[i].ex += gpBouf2[i].dx;
      }
    }
    else
      return 0;
  }

  // Diagonale.
  else if(gpBouf2[i].dx == gpBouf2[i].dy)
  {
    if(gpBouf2[i].nIncr < gpBouf2[i].Dx)
    {
      gpBouf2[i].nIncr+=gpBouf2[i].nSpd;
      gpBouf2[i].nPosX += gpBouf2[i].Xincr;
      gpBouf2[i].nPosY += gpBouf2[i].Yincr;
    }
    else
      return 0;
  }

  // Coef > 1.
  else if(gpBouf2[i].Dx < gpBouf2[i].Dy)
  {
    if(gpBouf2[i].nIncr < gpBouf2[i].Dy)
    {
      gpBouf2[i].nIncr+=gpBouf2[i].nSpd;
      gpBouf2[i].nPosY += gpBouf2[i].Yincr;
      gpBouf2[i].ey -= gpBouf2[i].dx;

      if(gpBouf2[i].ey < 0)
      {
        gpBouf2[i].nPosX+=gpBouf2[i].Xincr;
        gpBouf2[i].ey += gpBouf2[i].dy;
      }
    }
    else
      return 0;
  }

  // Ligne verticale.
  else if(gpBouf2[i].dx == 0)
  {
    if(gpBouf2[i].nIncr < gpBouf2[i].Dy)
    {
      gpBouf2[i].nIncr+=gpBouf2[i].nSpd;
      gpBouf2[i].nPosY+=gpBouf2[i].Yincr;
    }
    else
      return 0;
  }
  return 1;
}

void LineInit(u32 i)
{
  gpBouf2[i].ex=abs(gpBouf2[i].nDstX1-gpBouf2[i].nPosX);
  gpBouf2[i].ey=abs(gpBouf2[i].nDstY1-gpBouf2[i].nPosY);
  gpBouf2[i].dx=2*gpBouf2[i].ex; 
  gpBouf2[i].dy=2*gpBouf2[i].ey; 
  gpBouf2[i].Dx=gpBouf2[i].ex; 
  gpBouf2[i].Dy=gpBouf2[i].ey; 

  gpBouf2[i].Yincr = gpBouf2[i].Xincr = gpBouf2[i].nSpd;

  if(gpBouf2[i].nPosX > gpBouf2[i].nDstX1)
    gpBouf2[i].Xincr =- gpBouf2[i].nSpd;
  if(gpBouf2[i].nPosY > gpBouf2[i].nDstY1)
    gpBouf2[i].Yincr =- gpBouf2[i].nSpd;

  gpBouf2[i].nIncr = 0;
}

void Circle(u32 i)
{
    if(gpBouf2[i].nValQuad == 1)
    {
      gpBouf2[i].nValQuad = quadrant(i);
      if( ! gpBouf2[i].nValQuad)
      {
        gpBouf2[i].nValQuad = 2;
      }
    }
    else if(gpBouf2[i].nValQuad == 2)
    {
      gpBouf2[i].nValQuad = quadrant2(i);
      if( ! gpBouf2[i].nValQuad)
      {
        gpBouf2[i].nValQuad = 3;
      }
    }
    else if(gpBouf2[i].nValQuad == 3)
    {
      gpBouf2[i].nValQuad = quadrant3(i);
      if( ! gpBouf2[i].nValQuad)
        gpBouf2[i].nValQuad = 4; 
    }
    else if(gpBouf2[i].nValQuad == 4)
    {
      gpBouf2[i].nValQuad = quadrant4(i);
    }
    else
    {
      gpBouf2[i].nValQuad = 1;
      gpBouf2[i].nWait2=1;
    }
}  
  
void CircleInit(u32 i)
{
  // Centre.
  gpBouf2[i].nDstX1 = gpBouf2[i].nPosX - rand() % (SCR_Width/4);
  gpBouf2[i].nDstY1 = gpBouf2[i].nPosY - rand() % (SCR_Height/4);                

  // Calcule du rayon.
  gpBouf2[i].rx = gpBouf2[i].nDstX1 - gpBouf2[i].nPosX;
  gpBouf2[i].ry = gpBouf2[i].nDstY1 - gpBouf2[i].nPosY;

  //Valeur absolue .
  if(gpBouf2[i].rx<=0)
    gpBouf2[i].rx = -gpBouf2[i].rx; 
  if(gpBouf2[i].ry<=0)
    gpBouf2[i].ry = -gpBouf2[i].ry;
  
  gpBouf2[i].YincrCirc = gpBouf2[i].ry;
  gpBouf2[i].XincrCirc=0;
  gpBouf2[i].e=5-4*gpBouf2[i].ry;

  gpBouf2[i].nSensChange = 1;
  gpBouf2[i].nWait2 = -1;
}

void Function(u32 i)
{
  // Calcule du prochain pt.
  if( ! gpBouf2[i].nVar2 )
  {
    gpBouf2[i].nEchX = gpBouf2[i].XMin + gpBouf2[i].nDstX1 * 
    (gpBouf2[i].XMax - gpBouf2[i].XMin) / SCR_Width;
    
    switch(gpBouf2[i].nVar3)
    {
      case 0 :
        gpBouf2[i].nEchY = exp(gpBouf2[i].nEchX);
        break;
      case 1 :
        gpBouf2[i].nEchY = gpBouf2[i].nEchX * gpBouf2[i].nEchX * gpBouf2[i].nEchX;
        break;
      case 2 :
        gpBouf2[i].nEchY = gpBouf2[i].nEchX * gpBouf2[i].nEchX;
        break;
      case 3:
        gpBouf2[i].nEchY = sqrt(gpBouf2[i].nEchX);
        break;
      case 4 :
        gpBouf2[i].nEchY = log(gpBouf2[i].nEchX);
        break;
      case 5 :
        gpBouf2[i].nEchY = sin(gpBouf2[i].nEchX);
        break;
      case 6 :
        gpBouf2[i].nEchY = cos(gpBouf2[i].nEchX);
        break;
      case 7:
        gpBouf2[i].nEchY = 1/sqrt(gpBouf2[i].nEchX);
        break;
      case 8 :
        gpBouf2[i].nEchY = 1/gpBouf2[i].nEchX; 
        break;
      case 9 :
        gpBouf2[i].nEchY = 1/(gpBouf2[i].nEchX * gpBouf2[i].nEchX);
        break;
      case 10 :
        gpBouf2[i].nEchY = 1/(gpBouf2[i].nEchX * gpBouf2[i].nEchX * gpBouf2[i].nEchX);
        break;
      case 11 :
        gpBouf2[i].nEchY = atan(gpBouf2[i].nEchX);
        break;
      case 12 :
        gpBouf2[i].nEchY = tan(gpBouf2[i].nEchX);
        break;
    }
    
    gpBouf2[i].nVarY = (s32)(SCR_Height * (gpBouf2[i].nEchY -gpBouf2[i].YMin)/(gpBouf2[i].YMax -gpBouf2[i].YMin));
    gpBouf2[i].nDstY1 = SCR_Height - gpBouf2[i].nVarY - 1;
    
    if(( SGN(gpBouf2[i].nPosY) == -1 && gpBouf2[i].nDstY1 > 0)) // Pour le n°4 : log(x). 
      gpBouf2[i].nPosY = -1;
    if(gpBouf2[i].nDstY1 > SCR_Height && gpBouf2[i].nPosY > SCR_Height)  // Pour le n°1 : x^3.
      gpBouf2[i].nPosY = gpBouf2[i].nDstY1;
  }
  
  if(! (gpBouf2[i].nPosY < 0 && gpBouf2[i].nDstY1 < 0)) // PosY < -100.
  {
    if(gpBouf2[i].nDstX1)
    {
      if(gpBouf2[i].nVar)
      {
        LineInit(i);
        gpBouf2[i].nVar = 0;
      }
      gpBouf2[i].nVar2 = Line(i);
    }
  }
  else
    gpBouf2[i].nVar2 = 0;
    
  if( ! gpBouf2[i].nVar2 )
  { 
    gpBouf2[i].nPosX = gpBouf2[i].nDstX1;
    gpBouf2[i].nPosY = gpBouf2[i].nDstY1;

    gpBouf2[i].nDstX1+=gpBouf2[i].nSpd;  
    
    gpBouf2[i].nIncr = 0;
    gpBouf2[i].nVar = 1;

    if(gpBouf2[i].nPosX > SCR_Width)
    {
      gpBouf2[i].nWait = 1;
      gpBouf2[i].nVar3 = rand() & 12;
      return;
    }
  }
}

void FunctionInit(u32 i)
{
  gpBouf2[i].nDstX1 = gpBouf2[i].nPosX; //nPosX, Y correspondent au dernières positions. nDstX1, Y1 correspond au point suivant.
  gpBouf2[i].nDstY1 = gpBouf2[i].nPosY;

  gpBouf2[i].nVar=1;
  gpBouf2[i].nVar2=0;
  gpBouf2[i].nWait2 = -1;

  gpBouf2[i].nEchY = gpBouf2[i].nEchX = 0;

  gpBouf2[i].XMin = -(rand() & 10) + 1.0;
  gpBouf2[i].YMin = -(rand() & 10) + 1.0;
  gpBouf2[i].XMax = (rand() & 10) + 1.0;
  gpBouf2[i].YMax = (rand() & 10) + 1.0;
}

void Menu_HighScores_Init(void)
{
  u32 i;

  gVar.pBkgRect = NULL;
  gVar.pBackground = gVar.pBkg[0];
  gVar.nKeyTouch = -1;

  Menu_InitFade();
  Animspr_InitEngine(); // Pour les monstres. // Mise des slots à 0.
  
  // Init effet des lignes.
  for(i = 0; i < HISC_Nb; i++)
  {
    gpBouf1[i].nSpdMax = -0x800;
    gpBouf1[i].nSpd = gpBouf1[i].nSpdMax;
    gpBouf1[i].nPosX = SCR_Width << 8; 
    gpBouf1[i].nWait = (HISC_Nb - i) * 8;
  }

  // Init effets des sprites.
  for(i = 0; i < BOUF2_Nb; i++)
  {
    gpBouf2[i].nAnmNo = Animspr_Set(gAnm_Mst1, -1); // Réserve un slot. nAnmNo contient le n° d'un slot.
    gpBouf2[i].nWait = (rand() & 63) | 16; // Attente mini.
    gpBouf2[i].nChoiceAlgo = 1;
    gpBouf2[i].nSensChange = 0; 
    gpBouf2[i].nValCirc = 1;
    gpBouf2[i].nValQuad = 1;
    gpBouf2[i].e=0; // L'erreur pour ligne et cercle Bresenham.
  }
}

u32 Menu_HighScores_Main(void)
{
  u32 nRetVal = e_MENU_Null;
  s32 nPosX, nPosY;

  switch(gMenu.nState)
  {
    case e_MENU_State_FadeIn :
      gMenu.nFadeVal += FADE_Step;
      if(gMenu.nFadeVal > 256)
      {
        gMenu.nState = e_MENU_State_Input;
        gMenu.nFadeVal = -1;
      }
      break;

    case e_MENU_State_FadeOut :
      gMenu.nFadeVal -= FADE_Step;
      if(gMenu.nFadeVal < 0)
      {
        gVar.nKeyReturn = 0;
        gMenu.nScIdx = -1;
        nRetVal = e_MENU_Main;
      }
      break;

    case e_MENU_State_Input :
      if(gVar.nMouseButtons & MOUSE_BtnLeft) // On est dans le HallOfFame.
      {
        gMenu.nFadeVal = 256;
        gMenu.nState = e_MENU_State_FadeOut;
        Sfx_PlaySfx(FX_Menu_Clic, e_SfxPrio_10);
      }
      break;
  }
  
  u32 i;

  // Hall Of Fame.
  if(gMenu.nState != e_MENU_State_FadeIn) 
  { 
    {
      {
        s32 i;

        // Effet des lignes.
        for(i = 0; i < HISC_Nb; i++)
        {
          if(gpBouf1[i].nWait == 0)// Fin placement graphique (Temps d'attente en frame).
          {
            if(gpBouf1[i].nSpdMax) // Bornes dans lequel se fait l'osscilliation.
            {
              s32 nLastPosX = gpBouf1[i].nPosX;
              gpBouf1[i].nPosX += gpBouf1[i].nSpd;  

              if(ABS(gpBouf1[i].nSpdMax) == 0x100 && gpBouf1[i].nPosX >> 8 == 0)
                {
                  gpBouf1[i].nSpdMax = 0;
                }
              else
              {
                if(SGN(gpBouf1[i].nPosX) != SGN(nLastPosX))
                {
                  // Permet de faire ossciller la ligne en + ou moins (bornes).
                  gpBouf1[i].nSpdMax = -gpBouf1[i].nSpdMax/2; 
                }
                //Au changement de spdMax.
                if( SGN(gpBouf1[i].nSpd) != SGN(gpBouf1[i].nSpdMax) || 
                    (SGN(gpBouf1[i].nSpd) == SGN(gpBouf1[i].nSpdMax) && 
                    ABS(gpBouf1[i].nSpd) < ABS(gpBouf1[i].nSpdMax)) )
                {
                  // Mettre le signe de spdMax permet de faire des aller-retour.
                  // Ex : si spdMax = 1024 alors donne décalé de 6 donne 64.
                  gpBouf1[i].nSpd += SGN(gpBouf1[i].nSpdMax) << 6; 
                }
              }
            } 
          }
          else 
          {
            gpBouf1[i].nWait--;
          }
        }
      }

      for(i = 0; i < HISC_Nb; i++)
        if(! (gpBouf1[i].nWait == 0 && gpBouf1[i].nSpdMax == 0)) break;
        
      //Effet des lignes fini ?
      if(i == HISC_Nb)
      {
        for(i = 0; i < BOUF2_Nb; i++)
        {
          if(gpBouf2[i].nWait == 0)
          { 
            // Line ?       
            if( ! gpBouf2[i].nChoiceAlgo)
            {
              Line(i);
            }
            else if(gpBouf2[i].nChoiceAlgo == 1)
            {
              Function(i);
            }
            else
            {
              Circle(i);
            }

            // Limite de l'écran.
            if(gpBouf2[i].nChoiceAlgo != 1)
            {  
              switch(gpBouf2[i].nSens)
              {
                case 0: // Bas.
                  if(gpBouf2[i].nPosY >= SCR_Height + 16)
                  {
                    gpBouf2[i].nWait = (rand() & 63) | 16;
                    gpBouf2[i].nSensChange = 0; 
                  }
                  break;
                case 1: // Haut.
                  if(gpBouf2[i].nPosY <= -16) 
                  {
                    gpBouf2[i].nWait = (rand() & 63) | 16;
                    gpBouf2[i].nSensChange = 0;
                  }
                  break;
                case 2: // Gauche.
                  if(gpBouf2[i].nPosX >= SCR_Width + 16) 
                  {
                    gpBouf2[i].nWait = (rand() & 63) | 16;
                    gpBouf2[i].nSensChange = 0;
                  }
                  break;
                case 3: // Droite .
                  if(gpBouf2[i].nPosX <= -16)
                  {
                    gpBouf2[i].nWait = (rand() & 63) | 16;
                    gpBouf2[i].nSensChange = 0;
                  }
                  break;
              }
            }

            Sprites_Display(Animspr_GetImage(gpBouf2[i].nAnmNo), gpBouf2[i].nPosX, gpBouf2[i].nPosY, 50+i);

            // Changement de direction ?
            if (--gpBouf2[i].nWait2 == 0 ) 
            {
              {
                gpBouf2[i].nSens = (gpBouf2[i].nSens & 2 ? 0 : 2);
                gpBouf2[i].nSens += (rand() & 1);
                gpBouf2[i].nChoiceAlgo = rand() & 1; // 0: Line, 1 : Circle.
              }
              if( ! gpBouf2[i].nChoiceAlgo) // Ligne ?
              {
                gpBouf2[i].nWait = 1;
                gpBouf2[i].nSensChange = 1; 
              }
              else // Circle.
              {
                CircleInit(i);
                gpBouf2[i].nChoiceAlgo = 2;
              }
            }
          }
          else
          {
            gpBouf2[i].nWait--;
            if(gpBouf2[i].nWait == 0)
            {
              // Init.
              if(gpBouf2[i].nSensChange == 0) // Evite de changer de sprites au changement de direction.
              {
                u32 *pAnm[] = { gAnm_Mst1, gAnm_Mst2, gAnm_Mst3, gAnm_Mst4 };
                Animspr_Set(pAnm[rand() & 3], gpBouf2[i].nAnmNo); 
              }

              // Anim.
              gpBouf2[i].nSens = rand() & 3;

              //position de départ.
              // switch(gpBouf2[i].nSens && gpBouf2[i].nChoiceAlgo != 0)
              switch(gpBouf2[i].nSens)
              {
                case 0: // 0:Bas.
                  if(gpBouf2[i].nSensChange == 0) // Au départ pour les lignes uniquements. Evite le saut du sprite à un autre endroit de l'écran.
                  {
                    gpBouf2[i].nPosX = (rand() % (SCR_Width - 32)) + 16;
                    gpBouf2[i].nPosY = -16;
                  }
                  gpBouf2[i].nDstX1 = (rand() % (SCR_Width - 32)) + 16;
                  gpBouf2[i].nDstY1 = SCR_Height + 16; 
                  break;
                case 1: // 1:Haut.
                  if(gpBouf2[i].nSensChange == 0)
                  {
                    gpBouf2[i].nPosX = (rand() % (SCR_Width - 32)) + 16;
                    gpBouf2[i].nPosY = SCR_Height + 16;
                  }
                  gpBouf2[i].nDstX1 = (rand() % (SCR_Width - 32)) + 16; 
                  gpBouf2[i].nDstY1 = -16; 
                  break;
                case 2: // 2:Gauche.
                  if(gpBouf2[i].nSensChange == 0)
                  {  
                    gpBouf2[i].nPosY = (rand() % (SCR_Height - 32)) + 16;
                    gpBouf2[i].nPosX = -16;
                  }
                  gpBouf2[i].nDstY1 = (rand() % (SCR_Height -32)) + 16; 
                  gpBouf2[i].nDstX1 = SCR_Width + 16; 
                  break;
                case 3: // 3:Droite.
                  if(gpBouf2[i].nSensChange == 0)
                  { 
                    gpBouf2[i].nPosY = (rand() % (SCR_Height -32)) + 16;
                    gpBouf2[i].nPosX = SCR_Width + 16;
                  }
                  gpBouf2[i].nDstY1 = (rand() % (SCR_Height - 32)) + 16; 
                  gpBouf2[i].nDstX1 = -16; 
                  break;
              }

              gpBouf2[i].nSpd = (rand() & (0x100 >> 8)) + 1;
              // gpBouf2[i].nSpd = (rand() & (0x100 >> 8)) + 2;
              gpBouf2[i].nChoiceAlgo = rand() & 1; // 0 : lignes. 1 : fonction.

              if(! gpBouf2[i].nChoiceAlgo)
                {
                  LineInit(i);
                  gpBouf2[i].nWait2 = (rand() & 63) | 64;
                }
              else
                {
                  FunctionInit(i);
                }
            }
          }
        }
      }
    }
  }


  // Titre.
  char pTitle[] = "- HALL OF FAME -";
  u32 nLg = Font_Print(0, 0, pTitle, FONT_NoDispo, e_Spr_FontSmall);
  Font_Print((SCR_Width - nLg) / 2, 24, pTitle, 0, e_Spr_FontSmall);

  // Affichage des lignes.
  nPosY = 48;
  for(i = 0; i < HISC_Nb; i++)
  { 
    char pStr[8+1];
    s32 nOfs;

    nPosX = 24 + (gpBouf1[i].nPosX >> 8);
    // Rank.
    strcpy(pStr, "00");
    Font_MyItoA(gpHighScores[i].nRank + 1, pStr);
    Font_Print(nPosX, nPosY + i * MENU_HiSC_Interligne, pStr, 0, e_Spr_FontSmall);
    
    // Nom.  
    Font_Print(32 + nPosX, nPosY + i * MENU_HiSC_Interligne, gpHighScores[i].pName, 0, e_Spr_FontSmall);
    
    // Round.
    nOfs = 0;
    if(gpHighScores[i].nRound + 1 > LEVEL_Max)
    {
      strcpy(pStr, "ALL");
      nOfs = -4;
    }
    else
    {
      strcpy(pStr, "00");
      Font_MyItoA(gpHighScores[i].nRound, pStr);
    }
    Font_Print(nPosX + (8 * 20) + 8+8 + nOfs, nPosY + (i * MENU_HiSC_Interligne), pStr, 0, e_Spr_FontSmall);
    
    // Score.
    strcpy(pStr, "00000000");
    Font_MyItoA(gpHighScores[i].nScore, pStr);
    Font_Print(208 + nPosX, nPosY + (i * MENU_HiSC_Interligne), pStr, 0, e_Spr_FontSmall);
  }

  gVar.nFadeVal = gMenu.nFadeVal;

  return nRetVal;
}


//=======================================================================================================
//==============================================MENU MAIN================================================
//=======================================================================================================


struct SMenuItm gpMenuItems_Main[] = {
  {e_MENU_Game, 0, 0, "START"},
  {e_MENU_HallOfFame, 0, 0, "HALL OF FAME"},
  {e_MENU_Quit, 0, 0, "QUIT"},
};

// Vérifie si on a appuyer sur flèches clavier haut bas.
void Menu_KeyArrow(u32 nArrow)
{
  static u32 nTime = 0;
  nTime++;

  if(nArrow == 0)
    {
      nTime = 0;
      return;
    }
  else
  {
    if(nTime == 4 || (nTime > 30) ) // Et mettre : ou nTime > 10.
    {
      switch(nArrow)
      {
        case KEY_Up :
          if(gMenu.nChoiceKey > 0)
            gMenu.nChoiceKey--; 
          else
            gMenu.nChoiceKey = NBELEM(gpMenuItems_Main)-1;
          break;
        case KEY_Down :
          if(gMenu.nChoiceKey < NBELEM(gpMenuItems_Main)-1) 
            gMenu.nChoiceKey++; 
          else
            gMenu.nChoiceKey = 0;
          break;
      }
      
      if(nTime > 30 )
        nTime = 25;
    }
    else if(! gVar.pKeys[SDLK_UP] && ! gVar.pKeys[SDLK_DOWN])
      gVar.nKeyArrow = 0; // RAZ flèche du clavier.
  }
}



void Menu_CursorInit(void)
{
  gMenu.nCursPos = 0;
  gMenu.nCursAccel = CURS_Acc;
}

void Menu_CursorMove(void)
{
  gMenu.nCursAccel -= 0x20; // Gravité.
  gMenu.nCursPos += gMenu.nCursAccel;
  if(gMenu.nCursPos < 0 ) Menu_CursorInit();
}

void Menu_Init(void)
{
  gMenu.nScIdx = -1; // RAZ entrée du nom des High-Score.
}

void Menu_InitFade(void)
{
  gMenu.nState = e_MENU_State_FadeIn;
  gMenu.nFadeVal = 0;
}

u32  Menu_CheckMouseColBox(u32 nSpr1, u32 nPosX1, u32 nPosY2, s32 nXMouse, s32 nYMouse, u32 nIdx)
{
  s32 nMode; // Mode vidéo
  s32 nXMin, nXMax, nYMin, nYMax;
  
  struct SSprite *pSpr = Sprites_GetDesc(nSpr1); 

  nXMin = nPosX1 - pSpr->nPtRefX;
  nXMax = nXMin + gpMenuItems_Main[nIdx].nLg;
  nYMin = nPosY2 - pSpr->nPtRefY - 2;
  nYMax = nPosY2 + 2;
  
  // Multiplicateur en fonction du scale.
  if(gRender.nRenderMode == (u8)e_RenderMode_Scale2x)
    nMode = 2;
  else
    nMode = 1;

  if(nXMouse >= nXMin*nMode && nXMouse <= nXMax*nMode && nYMouse >= nYMin*nMode && nYMouse <= (nYMax+ADJUST_VAR)*nMode)
  {
    gMenu.nChoiceMouse = nIdx;
    return 1;
  }
  return 0;
}

void Menu_MainInit(void)
{
  u32 i;
  gVar.pBackground=gVar.pBkg[0];
  gVar.pBkgRect = NULL;
  gVar.nKeyArrow = 0;

  Menu_InitFade();

  //Récupère les longueurs des phrases usuelles.
  for(i = 0; i < NBELEM(gpMenuItems_Main); i++)
  {
    gpMenuItems_Main[i].nLg = Font_Print(0, 0, gpMenuItems_Main[i].pTxt, FONT_NoDispo, e_Spr_FontSmall);
  }

  // Récupère les longueurs des phrases avec effet.
  for(i = 0; i < NBELEM(gpMenuItems_Main); i++)
  {
    gpMenuItems_Main[i].nLgEffect = Font_Print(0, 0, gpMenuItems_Main[i].pTxt, FONT_NoDispo, e_Spr_FontSmall_Effect);
  }
  gMenu.nChoiceKey = 0;
  gMenu.nChoiceMouse = -1;

  Menu_CursorInit();

  // Mouvement du Background.
  #ifdef MENU_Bkg_Mvt
    gVar.pBkgRect = &gVar.sBkgRect;
    gVar.sBkgRect.w = SCR_Width;
    gVar.sBkgRect.h = SCR_Height; 
  #endif
}

u32 Menu_MainMain()
{
  u32 i = 0;
  static u8 nClic = 0, nReturn = 0; // clic = 1, return = 2
  static s8 nBooleanMouse = 0, nLastBooleanMouse = 0;
  static u32 nLastChoiceMouse = 0, nLastChoiceKey = 0;
  u32 nRetVal = e_MENU_Null;
  
  #ifdef MENU_Bkg_Mvt
    static u8 nSinnIdx1 = 0;
    // Déplacement du décor (Ligne).
    gVar.sBkgRect.x = nSinnIdx1 & 31;
    gVar.sBkgRect.y = 0;
    nSinnIdx1++;
  #endif

  switch(gMenu.nState)
  {
    case e_MENU_State_FadeIn:
      gMenu.nFadeVal += FADE_Step;
      if(gMenu.nFadeVal > 256)
      {
        gMenu.nState = e_MENU_State_Input;
        gMenu.nFadeVal = -1;
        Sfx_PlaySfx(e_Sfx_Wars, e_SfxPrio_10);
      }
      break;

    case e_MENU_State_FadeOut:

      gMenu.nFadeVal -= FADE_Step;
      if(gMenu.nFadeVal < 0) 
      {
        if(nReturn == 2)
          {
            nRetVal = gpMenuItems_Main[gMenu.nChoiceKey].nMenuVal;
          }
        else 
          {
            nRetVal = gpMenuItems_Main[nLastChoiceMouse].nMenuVal;
          }
        nReturn = nClic = 0;

        Sfx_ClearChannels();
      }
      break;
      
    case e_MENU_State_Input:

      // Choix du menu avec les flèches du clavier.
      nLastChoiceKey = gMenu.nChoiceKey; 
      Menu_KeyArrow(gVar.nKeyArrow);

      // Si le curseur à bougé.
      if(nLastChoiceKey != gMenu.nChoiceKey)
      {
        Menu_CursorInit();
        Sfx_PlaySfx(FX_Menu_Move, e_SfxPrio_10);
      }

      // Si on a cliqué/return. 
      if(gVar.nMouseButtons & MOUSE_BtnLeft && nBooleanMouse > -1)
        {
          nClic = 1;
        }
      else if(gVar.nKeyReturn)
      {
        nReturn = 2;
      }
      
      if(nReturn || nClic)
      {  
        gMenu.nFadeVal = 256;
        gMenu.nState = e_MENU_State_FadeOut;
        gVar.nKeyReturn=0;
        nLastChoiceMouse = gMenu.nChoiceMouse; 
        Sfx_PlaySfx(FX_Menu_Clic, e_SfxPrio_10);
      }

      // Si la souris à bougé.
      if(nLastBooleanMouse != nBooleanMouse && nBooleanMouse > -1)
      {
        Sfx_PlaySfx(FX_Menu_Move, e_SfxPrio_10);      
      }      
      break;
  }

  Menu_CursorMove();

  s32 pOfs[] = {0, 26, 48, 73, 99, 127, 158, 166};
  
  s32 nMul = 8;
  static u8 nSin = 0;
   
  for(i=0; i < 8; i++)
  {
    // gVar.pCos[...]/8 le 8 est arbitraire en fonction de l'emplitude qu'on veut donné à l'effet.
    Sprites_Display(e_Spr_Logo8 + i, SCR_Width/2 - 96 + pOfs[i] + gVar.pCos[nSin + (i*nMul)]/8, 
                    90 - gVar.pSin[(nSin + (i*nMul)) & 0xFF]/16, 220 + i);
  }
  nSin-=2;

  // Menu. Choix clavier/souris.
  for(i = 0; i < NBELEM(gpMenuItems_Main); i++) 
  {
    // Affichage Menu.
    if(gMenu.nChoiceMouse != i)
      {
        Font_Print((SCR_Width/2) - (gpMenuItems_Main[i].nLg/2), MENU_Main_StartLn + (i*14), gpMenuItems_Main[i].pTxt, 0, e_Spr_FontSmall);
      }
        
    // Selecteur.
    if(i == gMenu.nChoiceKey)
    {
      Font_Print((SCR_Width/2) - (gpMenuItems_Main[i].nLg/2) - 18 + 4 - (gMenu.nCursPos >> 8), MENU_Main_StartLn + (i*14), ">", 0, e_Spr_FontSmall);
      Font_Print((SCR_Width/2) + (gpMenuItems_Main[i].nLg/2) + 10 - 4 + (gMenu.nCursPos >> 8), MENU_Main_StartLn + (i*14), "<", 0, e_Spr_FontSmall);
    }
  }

  // Choix menu souris.
  // Collision souris/menu. 
  nLastBooleanMouse = nBooleanMouse;
  for(i=0; i<NBELEM(gpMenuItems_Main); i++)
  {
    if(Menu_CheckMouseColBox(e_Spr_FontSmall, SCR_Width/2 - gpMenuItems_Main[i].nLg/2, MENU_Main_StartLn + i*14, gVar.nMousePosX, gVar.nMousePosY, i))
    {
      nBooleanMouse = i;
      // Le +1 sert pour l'ajustement du sprite en hauteur.
      Font_Print((SCR_Width/2) - (gpMenuItems_Main[i].nLgEffect/2), MENU_Main_StartLn + (i*14)+1, gpMenuItems_Main[i].pTxt, 0, e_Spr_FontSmall_Effect); 
      break;
    }
    else if(gMenu.nState != e_MENU_State_FadeOut)
      {
        gMenu.nChoiceMouse = -1;
        nBooleanMouse = -1; 
      }
  }
 
  gVar.nFadeVal = gMenu.nFadeVal;
  return nRetVal;
}
      
void Menu_Fade(s32 nFadeVal)
{
  if(nFadeVal < 0) return;
  
  u32 i, nNbColors=256;
  SDL_Color *pSrc=gVar.pColors;

  if(nFadeVal > 255) nFadeVal = 255;

  for(i = 0; i < nNbColors; i++)
  {
    gVar.pCurPal[i].r = pSrc->r * nFadeVal/255;
    gVar.pCurPal[i].g = pSrc->g * nFadeVal/255;
    gVar.pCurPal[i].b = pSrc->b * nFadeVal/255;
    pSrc++;
  }
  SDL_SetPalette(Render_GetRealVideoSurfPtr(), SDL_PHYSPAL, gVar.pCurPal, 0, nNbColors);
}


//=======================================================================================================
//============================================INSERTION SCORE============================================
//=======================================================================================================


struct SMenuItm gpMenuItems_GetName[]=
{
  {0, 0, 0, "CONGRATULATION !"},
  {0, 0, 0, "VOTRE RANG EST : #0@"},
  {0, 0, 0, "ENTREZ VOTRE NOM :"}
};

int qscmpp(const void *pEl1, const void *pEl2)
{
  return *(u32*)pEl1 - *(u32*)pEl2;
}

void Menu_ScrPutNameInTable(char *pName, u32 nRound, u32 nScore)
{
  u32 pHighScoreTmp[HISC_Nb] = {0}; // Tableau temporaire pour les ranks.
  s32 i = 0; // Si égalité entre 2 scores.
  s32 nRank = Menu_CheckHighScr(nScore); // Indice du rang. 

  if(nRank < 0) return;

  if(gpHighScores[nRank].nScore == nScore)
  {
    for(i = 0; i < HISC_Nb; i++)
      pHighScoreTmp[i] = gpHighScores[i].nRank;

    pHighScoreTmp[HISC_Nb-1] = gpHighScores[nRank].nRank;

    qsort(pHighScoreTmp, HISC_Nb, sizeof(pHighScoreTmp[0]), qscmpp);

    for(i = 0; i < HISC_Nb; i++)
      gpHighScores[i].nRank = pHighScoreTmp[i];
  }
  else // Si scores differents.
    for(i = HISC_Nb - 2; i >= nRank; i--)
    {
      gpHighScores[i + 1].nRank = gpHighScores[i].nRank + 1;  
    }

  // Décalage de la table.
  for(i = HISC_Nb - 2; i >= nRank; i--)
  {
    strcpy(gpHighScores[i + 1].pName, gpHighScores[i].pName);
    gpHighScores[i + 1].nRound = gpHighScores[i].nRound;
    gpHighScores[i + 1].nScore = gpHighScores[i].nScore;
  }
  

  // Score à insérer.
  strcpy(gpHighScores[nRank].pName, pName);
  gpHighScores[nRank].nRound = nRound;
  gpHighScores[nRank].nScore = nScore;
}

// Init des HighScor.
void Menu_GetName_Init(void)
{
  Menu_InitFade();
  Menu_Init();
  u32 i;
  for(i = 0; i < HISC_NameLg; i++)
  {
    gMenu.pScName[i] = '\0';
  }

  for(i = 0; i < NBELEM(gpMenuItems_GetName); i++)
  {
    gpMenuItems_GetName[i].nLg = Font_Print(0, 8, gpMenuItems_GetName[i].pTxt, FONT_NoDispo, e_Spr_FontSmall);
  }
}

// Saisie du nom quand high-score.
u32 Menu_GetName_Main(void)
{
  u32 nRetVal = e_MENU_Null;
  u32 nLgLine = 0;
  u32 i;
  static u32 nCligno = 0, nTimer = 0;
  
  switch(gMenu.nState)
  {
    case e_MENU_State_FadeIn :
      gMenu.nFadeVal += FADE_Step;
      if(gMenu.nFadeVal > 256)
      {
        gMenu.nState = e_MENU_State_Input;
        gMenu.nFadeVal = -1;
      }
      break;

    case e_MENU_State_FadeOut :
      gMenu.nFadeVal -= FADE_Step;
      if(gMenu.nFadeVal < 0)
      {
        // Si pas de nom, mettre John Doe.
        char pDefName[] = "JOHN DOE";
        if(gMenu.nScIdx == -1)
        {
          strcpy(gMenu.pScName, pDefName);
          gMenu.nScIdx = strlen(pDefName);
        }

        // Rajoute le nom dans les High-scores.
        Menu_ScrPutNameInTable(gMenu.pScName, gExg.nLevel, gExg.nScore);
        // Save High-Score.
        Menu_ScrSave();
        gVar.nKeyReturn = 0;    
        gMenu.nScIdx = -1; 
        
        // Mettre save des scores.
        nRetVal = e_MENU_HallOfFame; // On va direct dans HallOfFame, évite de retourner dans le menu principale.
      }
      break;

    case e_MENU_State_Input :
      if(gVar.nKeyReturn) // Fini d'entrer le nom ?
      {
        gMenu.nFadeVal = 256;
        gMenu.nState = e_MENU_State_FadeOut;
        Sfx_PlaySfx(FX_Menu_Clic, e_SfxPrio_10);
      }
      // else
      //   gMenu.nState = e_MENU_State_FadeOut;
  
      if(gExg.nExitCode == e_Game_GameOver)// Si on a fait game over ou arrivé fin du jeu.
      {
        if(gVar.pKeys[gVar.nKeyTouch])
        {
          if(gMenu.nScIdx < HISC_NameLg-2)
          {
            if((gVar.nKeyTouch >= SDLK_a && gVar.nKeyTouch <= SDLK_z))
              {
                if(gVar.nKeyTouch == SDLK_q)
                  gMenu.pScName[++gMenu.nScIdx] = 'A';
                else if(gVar.nKeyTouch == SDLK_a)
                  gMenu.pScName[++gMenu.nScIdx] = 'Q';
                else if(gVar.nKeyTouch == SDLK_w)
                  gMenu.pScName[++gMenu.nScIdx] = 'Z';
                else if(gVar.nKeyTouch == SDLK_z)
                  gMenu.pScName[++gMenu.nScIdx] = 'W';
                else if(gVar.nKeyTouch == SDLK_m){}
                else 
                  gMenu.pScName[++gMenu.nScIdx] = gVar.nKeyTouch - ' ';
              }
            else if(gVar.nKeyTouch == SDLK_SEMICOLON)
              gMenu.pScName[++gMenu.nScIdx] = 'M';
            else if((gVar.nKeyTouch >= SDLK_0 && gVar.nKeyTouch <= SDLK_9))
              gMenu.pScName[++gMenu.nScIdx] = gVar.nKeyTouch;
            else if((gVar.nKeyTouch >= SDLK_KP0 && gVar.nKeyTouch <= SDLK_KP9))
              gMenu.pScName[++gMenu.nScIdx] = gVar.nKeyTouch + '0';
            else if(gVar.nKeyTouch == SDLK_SPACE)
              gMenu.pScName[++gMenu.nScIdx] = gVar.nKeyTouch; 
          }
          if(gVar.nKeyTouch != SDLK_BACKSPACE)
          {
            if(gVar.nKeyTouch != SDLK_RETURN && gVar.nKeyTouch != SDLK_KP_ENTER)
              Sfx_PlaySfx(FX_Menu_Move, e_SfxPrio_10);  
            gVar.pKeys[gVar.nKeyTouch] = 0; // Enlève le repeat. Evite en appuyant sur la touche de l'afficher plusieur fois.
          }
          else
          {
            // Suppression des lettres. Timer sert pour la rapidité de suppression des lettres.
            if(nTimer++ & 8)
            {
              if(gVar.nKeyTouch == SDLK_BACKSPACE && gMenu.nScIdx > -1)
                gMenu.pScName[gMenu.nScIdx--] = '\0';
              Sfx_PlaySfx(FX_Menu_Move, e_SfxPrio_10);  
              nTimer = 0;
            }
          }
        }
      }
      break;
  }
  
  char pRank[30];
  strcpy(pRank, gpMenuItems_GetName[1].pTxt);
  char *pPtr = strchr(pRank, '@');

  if(pPtr != NULL) 
  {
    Font_MyItoA(gMenu.nRank + 1, pPtr-1);
  }
  for(i = 0; i < NBELEM(gpMenuItems_GetName); i++)
  {
    Font_Print((SCR_Width - gpMenuItems_GetName[i].nLg)/2, 40 + i*14, (i == 1 ? pRank : gpMenuItems_GetName[i].pTxt), 0, e_Spr_FontSmall);
  }

  if(gMenu.nScIdx < HISC_NameLg-1)
  {
    nLgLine = Font_Print(0, 0, gMenu.pScName, FONT_NoDispo, e_Spr_FontSmall); // Taille de pScName. Ici à zéro.
    if((++nCligno) & 16)
      Font_Print((SCR_Width + nLgLine)/2, SCR_Height/2, "_", 0, e_Spr_FontSmall); // Clignotement du curseur.
  }
  Font_Print((SCR_Width - nLgLine)/2, SCR_Height/2, gMenu.pScName, 0, e_Spr_FontSmall);

  gVar.nFadeVal = gMenu.nFadeVal;
  return nRetVal;
}

