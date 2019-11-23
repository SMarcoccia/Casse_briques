#include "includes.h"

struct SRender gRender;

SDL_Surface *Render_GetRealVideoSurfPtr(void){
  return (gRender.nRenderMode == e_RenderMode_Normal ? gVar.pScreen : gRender.pScreen2x);
}

void Render_Scale2x(SDL_Surface *pSDL_Src, SDL_Surface *pSDL_Dst){
  SDL_LockSurface(pSDL_Src);
  SDL_LockSurface(pSDL_Dst);

  u32 x, y;
  u8 *pSrc = pSDL_Src->pixels;
  u8 *pDst = pSDL_Dst->pixels;
  u8 *pSrc2;
  u16 *pDst2a, *pDst2b;
  u16 nClr;

  for(y = 0; y < SCR_Height; y++){
    pSrc2 = pSrc;
    pDst2a = (u16 *)pDst;
    pDst2b = (u16 *)(pDst + pSDL_Dst->pitch);

    for(x = 0; x < SCR_Width; x++){
      nClr = *pSrc2++;
      nClr |= nClr << 8;
      *pDst2a++ = nClr;
      *pDst2b++ = nClr;
    }
    pSrc += pSDL_Src->pitch;
    pDst += pSDL_Dst->pitch * 2;
  }

  SDL_UnlockSurface(pSDL_Dst);
  SDL_UnlockSurface(pSDL_Src);
}

typedef void (*pRenderFct)(SDL_Surface *pSDL_Src, SDL_Surface *pSDL_Dst);

void Render_Flip(u32 nSync)
{
  static pRenderFct pFctTab[e_RenderMode_MAX]={NULL, Render_Scale2x};

  if(pFctTab[gRender.nRenderMode] != NULL) pFctTab[gRender.nRenderMode](gVar.pScreen, gRender.pScreen2x);
  
  if(nSync) Frame_Wait();
  SDL_Flip(gRender.nRenderMode == e_RenderMode_Normal ? gVar.pScreen : gRender.pScreen2x);
}

SDL_Surface *Render_VideoModeSet(u32 nScrWidth, u32 nScrHeight, u32 nBPP, u32 nFlags)
{
  SDL_Surface *pSurf;

  pSurf = SDL_SetVideoMode(nScrWidth, nScrHeight, nBPP, nFlags | FLAGS);

  if(pSurf == NULL)
    {
      fprintf(stderr, "Render_InitVideo(): Unable to allocate SDL surface: %s\n", SDL_GetError());
    }
  else
    SDL_FillRect(pSurf, NULL, CLR_BLACK_IDX);

  return pSurf;
}

// Initialisation de la SDL
void Render_InitVideo(void)
{
  gRender.nRenderMode = e_RenderMode_Normal;
  gRender.nFullscreenMode = 0;

  gRender.pScreen2x = NULL;
  gRender.pScreenBuf2 = NULL;
  
  gVar.pScreen = Render_VideoModeSet(SCR_Width, SCR_Height, BPP, gRender.nFullscreenMode ? SDL_FULLSCREEN : 0);

  if(gVar.pScreen == NULL) exit(1);

  // Buffer vidéo surface 320x240.
  gRender.pScreenBuf2 = SDL_CreateRGBSurface(SDL_SWSURFACE, SCR_Width, SCR_Height, 8, gVar.pScreen->format->Rmask, gVar.pScreen->format->Gmask, gVar.pScreen->format->Bmask, 0);
  
  if(gRender.pScreenBuf2 == NULL){
    fprintf(stderr, "Render_InitVideo() : Unable to allocate SDL surface : %s\n", SDL_GetError());
    exit(1);
  }
}


void Render_SetVideoMode(void)
{
  //Choix de la taille de la fenetre
  switch(gRender.nRenderMode)
    {
    case e_RenderMode_Scale2x :
      gRender.pScreen2x = Render_VideoModeSet(SCR_Width * 2, SCR_Height * 2, BPP, (gRender.nFullscreenMode ? SDL_FULLSCREEN : 0)); //Taille de la fenetre passe de 320x240 a 640x480
      gVar.pScreen = gRender.pScreenBuf2;

      if(gRender.pScreen2x != NULL) {
        SDL_SetPalette(gRender.pScreen2x, SDL_PHYSPAL | SDL_LOGPAL, gVar.pColors, 0, 256); // Pas besoin de mettre la palette logique.
        SDL_SetPalette(gRender.pScreenBuf2, SDL_LOGPAL, gVar.pColors, 0, 256);
        return;
      } 
      //If error mode normal
      gRender.nRenderMode = e_RenderMode_Normal;
      gRender.nFullscreenMode = 0;

      //Si gRender.pScreen2x est NULL on passe directement à case e_RenderMode_Normal pas de break.
    case e_RenderMode_Normal : 
    default:
      gVar.pScreen = Render_VideoModeSet(SCR_Width, SCR_Height, BPP, (gRender.nFullscreenMode ? SDL_FULLSCREEN : 0));
      gRender.pScreen2x = NULL;
      
      if(gVar.pScreen == NULL) exit(1);//Message d'erreur dans Render_VideoModeSet.
      SDL_SetPalette(gVar.pScreen, SDL_PHYSPAL | SDL_LOGPAL, gVar.pColors, 0, 256);
      break;
    }
}

void Render_Release(void){
  SDL_FreeSurface(gRender.pScreenBuf2);
}
