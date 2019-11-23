#include "includes.h"

#define FPS_Default (1000 / 60)

u32 gnTimer1;

// Init timers.
void Frame_Init(void)
{
  gnTimer1 = SDL_GetTicks();
}

// Attente de la frame.
void Frame_Wait(void)
{
  u32 gnTimer2;
  // On s'assure que l'on ne va pas trop vite.
  while(1)
  {
      gnTimer2 = SDL_GetTicks() - gnTimer1;
      if(gnTimer2 >= FPS_Default) break;
      SDL_Delay(3); 
  }
  gnTimer1 = SDL_GetTicks();
}

