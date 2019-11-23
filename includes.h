
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <SDL/SDL.h>
#include <windows.h>


#include "ctypes.h"
#include "preca.h"
#include "frame.h"

#include "breaker.h"
#include "render.h"
#include "sprites.h"
#include "menu.h"
#include "font.h"
#include "sfx.h"
#include "anims.h"
#include "mst.h"
#include "monsters.h"
#include "animspr.h"
#include "fire.h"
#include "dust.h"


// Define.

#define SCR_Width       320 //Constante pour création fenêtre SDL en Largeur
#define SCR_Height      240 //Constante pour création fenêtre SDL en Hauteur

#define SHADOW_OfsX     4
#define SHADOW_OfsY     4

#define LEVEL_NbBkg     5 //Nombre de sprites pour les backgrounds des levels
#define MENU_NbBkg      2 //Nombre de sprites pour les background pour les menus

#define MOUSE_BtnLeft   1 // Masque binaires.
#define MOUSE_BtnRight  2

#define KEY_Up 1
#define KEY_Down 2
#define KEY_Left 3
#define KEY_Right 4

#define SPR_Palette_Idx 128 // 0 à x : Palette du décore / x à 256 : Palette des sprites.

#define CLR_BLACK_IDX 7
#define CLR_BLUE_IDX 255  
#define CLR_PINK_IDX 0
#define CLR_WHITE_IDX 150

#define BPP 8 //Constante pour la taille d'un pixel, Bit par pixel, ici 16bits
#define FLAGS SDL_HWSURFACE|SDL_DOUBLEBUF

struct SGene{

  SDL_Surface *pScreen; // Ptr sur le buffer.
  SDL_Surface *pLevel; // Ptr sur l'image de fond d'un level.
  SDL_Surface *pLev[LEVEL_NbBkg]; // Les images de fond pour les level.
  SDL_Surface *pBkg[MENU_NbBkg]; // Les images de fond pour le menu.
  SDL_Surface *pBackground; // Ptr sur l'images de fond des menus.
  SDL_Surface *pTitleEffect; // Image 512x256 pour effet sur main menu.
  
  SDL_Rect *pBkgRect; // Ptr sur le rect pour déplacer le blit.
  SDL_Rect sBkgRect; // Rect pour déplacer le blit.
  
  SDL_Color pColors[256]; // Palette générale, à réinitialiser au changement de mode.
  SDL_Color pCol;
  SDL_Color pSprColors[256 - SPR_Palette_Idx]; //Palette des sprites.
  SDL_Color pCurPal[256]; //Palette lors des fades, pour le Scale 2x en 32 BPP.
  
  u8 *pKeys; // Buffer clavier.
  u8 nMouseButtons; // Boutons de la souris.
  u8 nKeyArrow; // Touches flèches.
  // u8 nKeyEnter; // Touche return. Pour utiliser les 2 touches return du clavier.
  u8 nKeyReturn; // Touche return. Pour utiliser les 2 touches return du clavier.
  
  s16 nKeyTouch; // Touche quelconque.
  s16 pSinCos[256 + 64]; // Table contenat les sin et cos * 256, sur les 256 angles.
  s16 *pSin; // Ptrs sur les tables.
  s16 *pCos;
  s32 nMousePosX, nMousePosY; // Position de la souris.
  s32 nFadeVal; // 0 = Noir / 256 = Couleurs finales.
  u32 nPalUpdate; // 0 = pas de changement de palette,  1 = Changement.

};

// Structure d'échange entre les différents modules.
struct SExg
{
  u32 nExitCode; // Pour sortie du jeu. Tjs à 0, sauf pour sortie.
  u32 nLevel; // Level atteint au game over.
  u32 nScore; // Score au game over.
};

// Variable générales.

struct SGene gVar;
struct SExg gExg;



