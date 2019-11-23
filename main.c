
#include "includes.h"

#define EVENT_EmergencyExit 1 // Supprime la sortie de secoure (Esc).

u32 EventHandler(u32 nInGame)
{
    SDL_Event event;
    static u32 nLastPhase = -1;

    while(SDL_PollEvent(&event))//a voir plus tard.
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
            gVar.nKeyTouch = event.key.keysym.sym;
            if(gVar.pKeys[SDLK_ESCAPE])
            {
              return 1;
            }

            if(gVar.pKeys[SDLK_F9])
            { 
                if(++gRender.nRenderMode >= e_RenderMode_MAX) gRender.nRenderMode = 0;
                Render_SetVideoMode();
            }
        
            if(gVar.pKeys[SDLK_F10])
            {
                gRender.nFullscreenMode ^= 1;
                Render_SetVideoMode();
            }

            if(gVar.pKeys[SDLK_UP])
              gVar.nKeyArrow = 1;
            if(gVar.pKeys[SDLK_DOWN])
              gVar.nKeyArrow = 2;
            if(gVar.pKeys[SDLK_LEFT])
              gVar.nKeyArrow = 3;
            if(gVar.pKeys[SDLK_RIGHT])
              gVar.nKeyArrow = 4;
            if(gVar.pKeys[SDLK_RETURN] || gVar.pKeys[SDLK_KP_ENTER])
              gVar.nKeyReturn = 1;

            if(nInGame == 1 && gVar.pKeys[SDLK_p])
              {
                if(gBreak.nPhase == e_Game_Pause)
                  gBreak.nPhase = nLastPhase;
                else
                {
                  nLastPhase = gBreak.nPhase;
                  gBreak.nPhase = e_Game_Pause;
                }
            }
            break;

        case SDL_KEYUP:
            break;

        case SDL_MOUSEMOTION :
            gVar.nMousePosX = event.motion.x;
            gVar.nMousePosY = event.motion.y;
            break;

        case SDL_MOUSEBUTTONUP :
            switch(event.button.button){
                case SDL_BUTTON_LEFT :
                    gVar.nMouseButtons |= MOUSE_BtnLeft;
                    break;

                default:
                  break;
            }
            break;

        case SDL_QUIT:
            exit(0);
            break;
        }
    }
    return 0;
}

// Creation of the palette
// We copy in the general palette the part of palette corresponding to the decoration + the palette of sprites.
void SetPalette(SDL_Surface *pBkg, SDL_Color *pSprPal, u32 nSprPalIdx)
{
    u32 i;
    SDL_Color *pSrcPal = pBkg->format->palette->colors;

    //Colors of the decor
    for(i=0; i < nSprPalIdx; i++){
        gVar.pColors[i] = pSrcPal[i];
    }
    //Colors of the sprites
    for(; i<256; i++){
        gVar.pColors[i] = pSprPal[i-SPR_Palette_Idx];
    }
    
    SDL_SetPalette(gVar.pScreen, SDL_LOGPAL, gVar.pColors, 0, 256);
}

u32 Menu(pFctInit pFct_Init, pFctMain pFct_Main)
{
    u32 nMenuVal = e_MENU_Null;

    SDL_FillRect(gVar.pScreen, NULL, CLR_BLACK_IDX);
    Render_Flip(0);
    Menu_Fade(0);

    (*pFct_Init)();

    SetPalette(gVar.pBackground, gVar.pSprColors, SPR_Palette_Idx);

    Frame_Init(); 
    while(nMenuVal == e_MENU_Null)
    {
        //Gestion des évenements.
        gVar.nMouseButtons=0;

        if(EventHandler(1) != 0){nMenuVal = e_MENU_Quit; break;}
        
				SDL_BlitSurface(gVar.pBackground, gVar.pBkgRect, gVar.pScreen, NULL);
				nMenuVal = (*pFct_Main)();
				Sprites_DisplayAll();
				Render_Flip(1);
				Menu_Fade(gVar.nFadeVal);
    }
    return nMenuVal;  
}

void Game()
{
  // Cinématique d'intro...
	
	SDL_FillRect(gVar.pScreen, NULL, CLR_BLACK_IDX); // Clear screen.
  Render_Flip(0);
	Menu_Fade(0);

  // Init.
  Breaker_ExgBrkInit();

  // Sets up palette (Même palette pour tous les niveaux).
  SetPalette(gVar.pLev[0], gVar.pSprColors, SPR_Palette_Idx);
	
	// ATTENTION SI ON TAPE SUR UNE TOUCHE, RETURN PAR EX. ALORS PRIS EN COMPTE.
  gVar.nFadeVal = 256;
  Menu_Fade(gVar.nFadeVal); // Remet la palette physique.

  Frame_Init();
  while(gExg.nExitCode == 0)
  {
		// Gestion des évenements.
    gVar.nMouseButtons = 0;
    
    if(EventHandler(1) != 0) {gExg.nExitCode = e_Game_GameOver; break;}
		
		if(gBreak.nPhase != e_Game_Pause)
    {
			SDL_BlitSurface(gVar.pLevel, NULL, gVar.pScreen, NULL); // Copie de l'image de fond.
			Breaker();
			Sprites_DisplayAll();
		}
		Render_Flip(1);
  }
  
	//Jeu terminé, cinématique de fin.
	// if(gExg.nExitCode == e_Game_AllClear){}

  //High score ?
  if(gExg.nExitCode == e_Game_GameOver || gExg.nExitCode == e_Game_AllClear)
  {
    if(Menu_CheckHighScr(gExg.nScore) >= 0)
    {
      // Saisie du nom.
      Menu(Menu_GetName_Init, Menu_GetName_Main); 
			// Affichage de la table des high scores.
      Menu(Menu_HighScores_Init, Menu_HighScores_Main); 
    }
  }
}

// Todo: ajusté le temps quand on rentre son nom.
// Todo: ajusté collision menu.

//Initialisation de la SDL.
void Init_SDL(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
			fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
			exit(1);
	}
}

int main(int argc, char **argv)
{
	u32 nLoop;
	u32 nMenuVal;
	u32 i;
  
  freopen( "CON", "w", stdout ); // Redirection du flux vers la sortie standard.

  Init_SDL();

	atexit(SDL_Quit); // Si on quite le jeux SDL_Quit() sera appelé.

	Render_InitVideo(); //Initialisation et affichage de la video en mode 320x240 + creation buffer 320x240.
	gRender.nRenderMode = e_RenderMode_Scale2x; //Mode 640x480.
	Render_SetVideoMode();//Affichage de la fenêtre en mode 2x.
	SDL_WM_SetCaption("Casse Brique", NULL); // Nom de la fenêtre.

  //Initialisation du tableau pour les touches du clavier. Renvoi un pointeur sur le tableau.
	gVar.pKeys = SDL_GetKeyState(NULL); 

	PrecaSinCos(); // Table de précalcule des sinus et cosinus.
	Sprites_InitEngine(); //Initialisation du nombre de sprites capturés et stockés à 0.

	//Load sprites sheets.
	Sprites_LoadBMP("gfx/bricks.bmp", gVar.pSprColors, SPR_Palette_Idx);
	Sprites_LoadBMP("gfx/font_small.bmp", NULL, 0);
	Sprites_LoadBMP("gfx/font_small_effect.bmp", NULL, 0);
			
	//Load levels background.
	char *pBkgLevFilenames[LEVEL_NbBkg]={"gfx/lev1.bmp", "gfx/lev2.bmp", "gfx/lev3.bmp", "gfx/lev4.bmp",
                                       "gfx/levdoh.bmp"};

	for(i=0; i<LEVEL_NbBkg; i++){
			if((gVar.pLev[i]=SDL_LoadBMP(pBkgLevFilenames[i]))==NULL){
					fprintf(stderr, "Couldn't load the levels background '%s' : %s\n",
          pBkgLevFilenames[i], SDL_GetError());
					exit(1);
			}
	}

	//Load menus background.
	char *pBkgMenuFilenames[MENU_NbBkg]={"gfx/bkg1.bmp", "gfx/bkg2.bmp"};
	for(i = 0; i < MENU_NbBkg; i++)
	{
			if((gVar.pBkg[i]=SDL_LoadBMP(pBkgMenuFilenames[i])) == NULL)
			{
					fprintf(stderr,"Couldn't load the menu background '%s' : %s\n",pBkgMenuFilenames[i], SDL_GetError());
					exit(1);
			}
	}

	//Init sound.
	Sfx_SoundInit();
	Sfx_LoadWavFiles();
	Sfx_SoundOn(); 

	Menu_Init(); 
	
	//Lecture de la table des High-Scores.
	Menu_ScrLoad();

	nLoop=1;
	nMenuVal = e_MENU_Main;
  
	while(nLoop)
	{
		switch(nMenuVal)
		{
			case e_MENU_Main :
					nMenuVal  = Menu(Menu_MainInit, Menu_MainMain);
					break;
			
			case e_MENU_Game :
					Game();
					nMenuVal = e_MENU_Main;
					break;

			case e_MENU_HallOfFame : // High scores.
					Menu(Menu_HighScores_Init, Menu_HighScores_Main);
					nMenuVal = e_MENU_Main;
					break;

			case e_MENU_Quit :
					nLoop=0;
					break;
		}
	}   

	Sfx_SoundOff();
	Sfx_FreeWavFiles();

	//Free the allocated surface.
	for(i=0; i < LEVEL_NbBkg; i++)
	{
		SDL_FreeSurface(gVar.pLev[i]);
	}
	for(i=0; i< MENU_NbBkg; i++)
	{
		SDL_FreeSurface(gVar.pBkg[i]);
	}

	Render_Release();
  argc = 0; argv[0] = "NULL";
  argc++;
  
	return 0;
}

