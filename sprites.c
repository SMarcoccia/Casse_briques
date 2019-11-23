#include "includes.h"

// Affichage info sprites sheet.
//#define DISPLAY_INFO

// Pour la capture des sprites.
#define SPRDEF_ALLOC_UNIT 256
struct SSprite *pSpr; // Déclaration d'une struct de sprites.
u32 gnSprNbSprDefMax; //Nb de sprites Max capturable.
u32 gnSprNbSprites; //Nb de sprites capturés.

// Pour le tri des sprites à chaque frame.
struct SSprStockage
{
  u32 nSprNo;
  s32 nPosX, nPosY;
  u32 nPrio;
};

#define SPR_STOCK_MAX 512

struct SSprStockage gpSprStock[SPR_STOCK_MAX]; 
struct SSprStockage *gpSprSort[SPR_STOCK_MAX]; // Pour le tri.
u32 gnSprStock; // Nbre de spr stockés pour affichage.

int qscmp(const void *pEl1, const void *pEl2)
{
  return ((*(struct SSprStockage **)pEl1)->nPrio - (*(struct SSprStockage **)pEl2)->nPrio);
}

void Sprites_DisplayAll(void)
{
  u32 i=0;
  
  if(gnSprStock == 0) return; // Rien à faire.
  qsort(gpSprSort, gnSprStock, sizeof(struct SSprStockage *), qscmp);

  SDL_LockSurface(gVar.pScreen);
  // Première passe : pour les ombres en dessous de tout.
  for(i = 0; i < gnSprStock; i++)
  {
    if(gpSprSort[i]->nSprNo & SPR_Flag_Shadow)
    {
      Sprites_DisplayLock(gpSprSort[i]->nSprNo, gpSprSort[i]->nPosX + SHADOW_OfsX, gpSprSort[i]->nPosY + SHADOW_OfsY);
    }

    // Sprites normaux.
    for(i=0; i < gnSprStock; i++)
    {
      Sprites_DisplayLock(gpSprSort[i]->nSprNo, gpSprSort[i]->nPosX, gpSprSort[i]->nPosY);
    }
  }

  SDL_UnlockSurface(gVar.pScreen);
  
  // RAZ pour le prochain tour.
  gnSprStock = 0;
}

u32 Sprites_CheckColBox(u32 nSpr1, s32 nPosX1, s32 nPosY1, u32 nSpr2, s32 nPosX2, s32 nPosY2)
{ 
  s32 nXMin1, nXMax1, nYMin1, nYMax1;
  s32 nXMin2, nXMax2, nYMin2, nYMax2;
  struct SSprite *pSpr1=Sprites_GetDesc(nSpr1); 
  struct SSprite *pSpr2=Sprites_GetDesc(nSpr2); 
  // struct SSprite *pSpr1=SprGetDesc(nSpr1); 
  // struct SSprite *pSpr2=SprGetDesc(nSpr2); 

  nXMin1 = nPosX1 - pSpr1->nPtRefX;
  nXMax1 = nXMin1 + pSpr1->nLg;
  nYMin1 = nPosY1 - pSpr1->nPtRefY;
  nYMax1 = nYMin1 + pSpr1->nHt;

  nXMin2 = nPosX2 - pSpr2->nPtRefX;
  nXMax2 = nXMin2 + pSpr2->nLg;
  nYMin2 = nPosY2 - pSpr2->nPtRefY;
  nYMax2 = nYMin2 + pSpr2->nHt;

  if(nXMax1 >= nXMin2 && nXMin1 <= nXMax2 && nYMax1 >= nYMin2 && nYMin1 <= nYMax2)
    return 1;
  else
    return 0;
}

void Sprites_InitEngine(void){
  pSpr=NULL; // Pointeur pour Tableau de struct des sprites.
  gnSprNbSprDefMax=0; 
  gnSprNbSprites=0; 
  gnSprStock = 0; 
}

void Sprites_Release(void){
  u32 i;
  for(i=0; i < gnSprNbSprites; i++)
    {
      free(pSpr[i].pGfx);
      pSpr[i].pGfx = NULL;
    }
}

void Sprites_Realloc()
{
  errno = 0;
  if(gnSprNbSprites >= gnSprNbSprDefMax )  
  {
    gnSprNbSprDefMax += SPRDEF_ALLOC_UNIT;
    pSpr=(struct SSprite *)realloc(pSpr, gnSprNbSprDefMax * sizeof(struct SSprite));
    if(pSpr == NULL)
    {
      fprintf(stderr, "Sprites_Realloc: Realloc failed. %s", strerror(errno));
      Sprites_Release();
      exit(1);
    }
  }
}

void Sprites_Display(u32 nSprNo, s32 nPosX, s32 nPosY, u32 nPrio)
{
  if(gnSprStock >= SPR_STOCK_MAX){fprintf(stderr, "Sprites : Out of slots!\n"); return;}
  
  if(nSprNo == SPR_NoSprite) return; // Peut servir pour des clignotements.

  gpSprStock[gnSprStock].nSprNo = nSprNo;
  gpSprStock[gnSprStock].nPosX = nPosX;
  gpSprStock[gnSprStock].nPosY = nPosY;
  gpSprStock[gnSprStock].nPrio = nPrio;
  gpSprSort[gnSprStock] = &gpSprStock[gnSprStock]; // Pour le tri.
  gnSprStock++;
}

//Affichage.
void Sprites_DisplayLock(u32 nSprNo, s32 nPosX, s32 nPosY)
{
	s32	nXMin, nXMax, nYMin, nYMax; //Position du Sprite par rapport au Screen.
	s32	nSprXMin, nSprXMax, nSprYMin, nSprYMax;
	s32	diff;
	u8	*pScr = (u8 *)gVar.pScreen->pixels;

	u32	nSprFlags = nSprNo;		// Pour conserver les flags.
	nSprNo &= ~SPR_Flag_Shadow;

	// Position du sprite sur l'écran à partir du pixel de référence du sprite.
	nXMin = nPosX - pSpr[nSprNo].nPtRefX;
	nXMax = nXMin + pSpr[nSprNo].nLg - 1;
	nYMin = nPosY - pSpr[nSprNo].nPtRefY;
	nYMax = nYMin + pSpr[nSprNo].nHt - 1;

	// Pixels d'origine du sprite + dimenssion.
	nSprXMin = 0;
	nSprXMax = pSpr[nSprNo].nLg - 1;
	nSprYMin = 0;
	nSprYMax = pSpr[nSprNo].nHt - 1;

	// Clips.
	if (nXMin < 0)
	{
		diff = 0 - nXMin;
		nSprXMin += diff;
	}
	if (nXMax > SCR_Width - 1)
	{
		diff = nXMax - (SCR_Width - 1);
		nSprXMax -= diff;
	}
	// Sprite complétement en dehors ?
	if (nSprXMin - nSprXMax > 0) return;
	//
	if (nYMin < 0)
	{
		diff = 0 - nYMin;
		nSprYMin += diff;
	}
	if (nYMax > SCR_Height - 1)
	{
		diff = nYMax - (SCR_Height - 1);
		nSprYMax -= diff;
	}
	// Sprite complétement en dehors ?
	if (nSprYMin - nSprYMax > 0) return;


	s32	ix, iy;
	u32	b4, b1, b4b, b1b;
	u8	*pMsk = pSpr[nSprNo].pMask;
	u8	*pGfx = pSpr[nSprNo].pGfx;
	s32	nScrPitch = gVar.pScreen->pitch;

	b1b = nSprXMax - nSprXMin + 1; // Longueur affichable du sprite.
	b4b = b1b >> 2;		// Nb de quads (Longueur du sprite divisé par 4).
	b1b &= 3;			// Nb d'octets restants ensuite.
	pScr += ((nYMin + nSprYMin) * nScrPitch) + nXMin;
	pMsk += (nSprYMin * pSpr[nSprNo].nLg);
	pGfx += (nSprYMin * pSpr[nSprNo].nLg);

  // printf("nSprFlags & SPR_Flag_Shadow = %d\n", nSprFlags & SPR_Flag_Shadow);
	if (nSprFlags & SPR_Flag_Shadow)
	{
		// Affichage d'une ombre.
		u8	*pSrc = (u8 *)gVar.pBackground->pixels;			// Source = image du level.
		s32	nSrcPitch = gVar.pBackground->pitch;

		pSrc += ((nYMin + nSprYMin) * nSrcPitch) + nXMin;

		for (iy = nSprYMin; iy <= nSprYMax; iy++)
		{
			b4 = b4b;
			for (ix = nSprXMin; b4; b4--, ix += 4)
			{
				*(u32 *)(pScr + ix) &= *(u32 *)(pMsk + ix);	//spd++
				*(u32 *)(pScr + ix) |= ( *(u32 *)(pSrc + ix) & ~*(u32 *)(pMsk + ix) ) +
									(~*(u32 *)(pMsk + ix) & 0x06060606);
			}
			b1 = b1b;
			for (; b1; b1--, ix++)
			{
				*(pScr + ix) &= *(pMsk + ix);	//spd++
				*(pScr + ix) |= ( *(pSrc + ix) & ~*(pMsk + ix) ) + (~*(pMsk + ix) & 0x06);
			}
			pScr += nScrPitch;
			pSrc += nSrcPitch;
			pMsk += pSpr[nSprNo].nLg;
			pGfx += pSpr[nSprNo].nLg;
		}
	}
	else
	{
		// Affichage normal.
		for (iy = nSprYMin; iy <= nSprYMax; iy++)
		{
			b4 = b4b; // Nombre de quad.
			for (ix = nSprXMin; b4; b4--, ix += 4)
			{
				*(u32 *)(pScr + ix) &= *(u32 *)(pMsk + ix);
				*(u32 *)(pScr + ix) |= *(u32 *)(pGfx + ix);
			}
			b1 = b1b; // Octets restant.
			for (; b1; b1--, ix++)
			{
				*(pScr + ix) &= *(pMsk + ix);
				*(pScr + ix) |= *(pGfx + ix);
			}
			pScr += nScrPitch;
			pMsk += pSpr[nSprNo].nLg;
			pGfx += pSpr[nSprNo].nLg;
		}
	}    
}

void Sprites_LoadBMP(char *pFilename, SDL_Color *pSprPal, u32 nPalIdx)
{
  SDL_Surface *pPlanche=NULL;
  u32 nNbSprPlanche = 0;

  // Récuparation du bmp qui est mit dans la ram.
  if((pPlanche = SDL_LoadBMP(pFilename))==NULL)
  {
    fprintf(stderr, "Couldn't load the sprite sheet %s : %s", pFilename, SDL_GetError());
    exit(1);
  }


  // On sauvegarde la palette ?
  if(pSprPal != NULL)
    {
      SDL_Color *pSrcPal = pPlanche->format->palette->colors;
      u32 i;
      
      for(i = nPalIdx; i < 256; i++)
      {  
        pSprPal[i-nPalIdx] = pSrcPal[i];
      }
    }

  // On parcourt de la planche de sprites pour les extraires.
  u32 ix=0, iy=0;
  u8 *pPix = (u8 *)pPlanche->pixels;

  for(iy=0; iy < (u32)pPlanche->h; iy++)
  {
    for(ix=0; ix < (u32)pPlanche->w; ix++)
    {
      // On tombe sur un sprite ?
      if(*(pPix + (iy*pPlanche->pitch) + ix) == CLR_PINK_IDX)
      { 
        // Vérifie si le tableau de sprites est suffisament grand.
        Sprites_Realloc();
        
        u32 LgExt, HtExt;
        u32 PtRefX, PtRefY;
        u32 ii, ij, ik;

        // Recherche des largeur.
        PtRefX = 0;
        LgExt = 1;
        ii = ix + 1;
        
        while(*(pPix + (iy * pPlanche->pitch) + ii) == 0 || *(pPix + (iy * pPlanche->pitch) + ii + 1) == 0)
        {
          if(*(pPix + (iy * pPlanche->pitch) + ii) != CLR_PINK_IDX) PtRefX = LgExt - 1;
          ii++;
          LgExt++;
        }

        PtRefY = 0;
        HtExt = 1;
        ii = iy + 1;
        while(*(pPix + (ii * pPlanche->pitch) + ix) == 0 || *(pPix + ((ii + 1) * pPlanche->pitch) + ix) == 0)
        {
          if(*(pPix + (ii * pPlanche->pitch) + ix) != CLR_PINK_IDX) PtRefY = HtExt - 1;
          ii++;
          HtExt++;
        }
        pSpr[gnSprNbSprites].nPtRefX = PtRefX;
        pSpr[gnSprNbSprites].nPtRefY = PtRefY;
        pSpr[gnSprNbSprites].nLg = LgExt - 2;
        pSpr[gnSprNbSprites].nHt = HtExt - 2;

        #ifdef DISPLAY_INFO
        printf("N° = %d -- \tLg = %d / Ht = %d ; Pt de réf : (%d , %d)\n", gnSprNbSprites, LgExt-2, HtExt-2, PtRefX, PtRefY);
        #endif

        // Création de la surface du sprite capturé + Mask.
        pSpr[gnSprNbSprites].pGfx = (u8 *)malloc(pSpr[gnSprNbSprites].nLg*pSpr[gnSprNbSprites].nHt*2);
        if(pSpr[gnSprNbSprites].pGfx == NULL)
        {
          fprintf(stderr, "Couldn't allocated of memory for sprite n° %d : %s\n", (s32)gnSprNbSprites, strerror(errno));
          Sprites_Release();
          exit(0);
        }
        pSpr[gnSprNbSprites].pMask = pSpr[gnSprNbSprites].pGfx + pSpr[gnSprNbSprites].nLg*pSpr[gnSprNbSprites].nHt;

        //Sprite récupération + Mask
        ik = 0;
        for(ij = 0; ij < pSpr[gnSprNbSprites].nHt; ij++)
        {
          for(ii = 0; ii < pSpr[gnSprNbSprites].nLg; ii++)
          {
            pSpr[gnSprNbSprites].pGfx[ik] = *(pPix + ((iy + ij + 1) * pPlanche->pitch) + (ix + ii + 1));
            pSpr[gnSprNbSprites].pMask[ik] = pSpr[gnSprNbSprites].pGfx[ik] ? 0 : 255;
            ik++;
          }
        }

        //Effacement du sprite dans la planche originale.
        for(ij = 0; ij < pSpr[gnSprNbSprites].nHt+2; ij++)
        {
          for(ii = 0; ii < pSpr[gnSprNbSprites].nLg+2; ii++)
          {
            *(pPix + ((iy + ij) * pPlanche->pitch) + (ix + ii)) = CLR_BLUE_IDX;
          }
        }

        nNbSprPlanche++;
        gnSprNbSprites++;
      }
    } 
  }

  #ifdef DISPLAY_INFO
  printf("Total des sprites dans : %s: %d.\n", pFilename, nNbSprPlanche);
  printf("Total des sprites capturés = %d\n\n", gnSprNbSprites);
  #endif

  SDL_FreeSurface(pPlanche);
}

// struct SSprite *SprGetDesc(u32 nSprNo)
struct SSprite *Sprites_GetDesc(u32 nSprNo)
{
  return &pSpr[nSprNo];
}

// void Sprites_DisplayLock(u32 nSprNo, s32 nPosX, s32 nPosY)
// {
  
// }

