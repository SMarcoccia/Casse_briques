
enum{
  e_RenderMode_Normal = 0,
  e_RenderMode_Scale2x,
  e_RenderMode_MAX
};


struct SRender{

  SDL_Surface *pScreen2x; //Pointeur sur la surface ecran en mode 2x.
  SDL_Surface *pScreenBuf2; //Buffer de rendu en mode 2x remplace la surface ecran reelle.

  u8 nRenderMode; //Taille de l'ecran 1x 2x.
  u8 nFullscreenMode; //Mode plein ecran.
};

struct SRender gRender;

SDL_Surface *Render_VideoModeSet(u32 nScrWidth, u32 nScrHeight, u32 nBPP, u32 nFlags);
void Render_InitVideo(void); //Initialisation de la video.
void Render_SetVideoMode(void); //Met le mode video 1x, 2x, Fullscreen.
void Render_Flip(u32 nSync);//Rafraichissement de l'ecran
void Render_Release(void);//Liberation de la memoire de pScreenBuf2;
SDL_Surface *Render_GetRealVideoSurfPtr(void);//Renvoi un ptr sur la surface écran réelle
