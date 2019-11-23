#include "includes.h"

// Si on veut mélanger plusieurs flux audio venant de différentes sources pour jouer plusieurs morceaux
// en même temps.
#define SFX_MAX_SOUNDS 2 
// Pour baisser le volume du son (SDL_MIX_MAXVOLUME).
#define SFX_ADJUST_VOL_SOUND 96
#define SFX_VOL_SOUND SDL_MIX_MAXVOLUME - SFX_ADJUST_VOL_SOUND

struct SSample
{
  u8 *pData;
  u32 nDPos;
  u32 nDLen;
  u8 nPrio; // Priorité du son en cours.
}gpSounds[SFX_MAX_SOUNDS];

struct SSfxGene
{
  u8 nInit; // Si couche audio est initialiser : 1, sinon 0.
  SDL_AudioSpec sAudioSpec; // Structure qui décrit le format audio.
  SDL_AudioCVT pCvt[e_Sfx_LAST]; // Structure contenant des filtres de conversion audio et des buffers.
};

struct SSfxGene gSfx;

void Sfx_FreeWavFiles(void)
{
  u32 i;
  if(! gSfx.nInit) return;
  for(i = 0; i < e_Sfx_LAST; i++)
  {
    free(gSfx.pCvt[i].buf);
  }
}

void Sfx_SoundOff(void)
{
  if(! gSfx.nInit) return;
  SDL_CloseAudio();
}

// Mixer, appelé par SDL.
void Sfx_MixAudio(void *unused, u8 *stream, int len)
{

  // Note : la len qui vaut 2048 c'est SDL qui ce charge de lui donner cette valeur. 
  //        L'utilisateur ne s'enoccupe pas.
  u32 i;
  u32 amount; // Quantité, nb d'échantillons.
  s32 nVolSound = 0;
  unused = NULL;
  static u32 nAdjsutVolSound = 0;

  for(i = 0; i < SFX_MAX_SOUNDS; i++)
  {
    // Ce qui reste à jouer.
    amount = (gpSounds[i].nDLen - gpSounds[i].nDPos);
    
    // Taille du flux qu'on va jouer.
    if(amount > (u32)len)
    {
      amount = len;
    }	
    
    // Obligé de mettre gMenu.nFadeVal car .nFadeVal reste à 1 après avoir quitté e_MENU_State_FadeOut.
    if(gMenu.nState == e_MENU_State_FadeOut && gMenu.nFadeVal >= 0 && ! i)  
    {
      nAdjsutVolSound += 1;
    }
    else if( ! gMenu.nFadeVal)
      nAdjsutVolSound = 0;

    nVolSound = SFX_VOL_SOUND - nAdjsutVolSound;
	  SDL_MixAudio(stream, &gpSounds[i].pData[gpSounds[i].nDPos], amount, nVolSound);
    gpSounds[i].nDPos += amount; // Mise à jour de la position du flux audio.
  }
}

// Jouer un son.
// Le minimum : 
// On commence par chercher un canal vide.
// Si il n'y en a pas, on note celui qui à la priorité la plus faible.
// Si plusieurs ont la même priorité, on note celui qui est le plus proche de la fin.
// Enfin, si la prio du son à jouer est ok, on le joue dans le canal noté.

void Sfx_PlaySfx(u32 nSfxNo, u32 nSfxPrio)
{
  u32 index;
  u8 nPrioMinVal = 255;
  u32 nPrioMinPos = 0;
  u32 nPrioMinDiff = (u32)-1;
  
  if(nSfxNo >= e_Sfx_LAST) return; //Sécurité.

  // Look for an empty (or finished) sound slot.
  // printf("gpSounds[%d].nDPos : %d == %d : gpSounds[%d].nDLen\n", 0, gpSounds[0].nDPos, gpSounds[0].nDLen, 0);
  // printf("gpSounds[index].nPrio = %d < 255\n", gpSounds[0].nPrio);
  for(index = 0; index < SFX_MAX_SOUNDS; index++)
  {
    if(gpSounds[index].nDPos == gpSounds[index].nDLen)
      break;  
    if(gpSounds[index].nPrio < nPrioMinVal)
    {
      nPrioMinVal = gpSounds[index].nPrio;
      nPrioMinPos = index;
      nPrioMinDiff = gpSounds[index].nDLen - gpSounds[index].nDPos;
    }
    else if(gpSounds[index].nPrio == nPrioMinVal)
    {
      if(gpSounds[index].nDLen - gpSounds[index].nDPos < nPrioMinDiff)
      {
        nPrioMinPos = index;
        nPrioMinDiff = gpSounds[index].nDLen - gpSounds[index].nDPos;
      }
    }
  }
  // printf("nSfxPrio = %d\n", nSfxPrio);
  // On a trouvé un emplacemnt libre ?
  if(index == SFX_MAX_SOUNDS)
  {
    // Non, la prio demandée est > ou == à la prio mini en cours ?
    if(nSfxPrio < nPrioMinVal) return;
    index = nPrioMinPos;
  }
  // Put the sound data in the slot (it starts playing immediately).
  SDL_LockAudio();
    gpSounds[index].pData = gSfx.pCvt[nSfxNo].buf;
    gpSounds[index].nDLen = gSfx.pCvt[nSfxNo].len_cvt;
    gpSounds[index].nDPos = 0; // Remet le son à 0 pour quand on arrive dans Sfx_MixAudio nDPos soit à 0.
    gpSounds[index].nPrio = (u8)nSfxPrio;
  SDL_UnlockAudio();
}

void Sfx_ClearChannels(void)
{
  u32 i;
  for(i = 0; i < SFX_MAX_SOUNDS; i++)
  {
    gpSounds[i].nDPos = 0;
    gpSounds[i].nDLen = 0;
  }
}

void Sfx_SoundInit(void)
{
  gSfx.nInit = 0;
  // Set 16-bit stereo audio at 22Khz.
  gSfx.sAudioSpec.freq = 22050; // Fréquence en échantillons par sec ; ici 22050 échantillons/sec.
  gSfx.sAudioSpec.format = AUDIO_S16; // Format des Echantillons ; ici 16bits signés.
  gSfx.sAudioSpec.channels = 2; // 2 canaux. Effet de frappe binaural.
  gSfx.sAudioSpec.samples = 512; // A good value for games. Taille du buffer audio. 512 échantillons
  gSfx.sAudioSpec.callback = Sfx_MixAudio; // Adresse de la fonction callback.
  gSfx.sAudioSpec.userdata = NULL;

  // Open the audio device and start playing sound!
  // Initialise la couche audio.
  if(SDL_OpenAudio(&gSfx.sAudioSpec, NULL) < 0)
  {
    printf("Unable to open audio : %s\n", SDL_GetError());
    printf("Sound disabled.\n");
    return;
  }

  gSfx.nInit = 1;
  Sfx_ClearChannels(); // Nettoyage des structures.
}

void Sfx_LoadWavFiles(void)
{
  u32 i;
  SDL_AudioSpec sWave;
  u8 *pData;
  Uint32 nDLen;

  char *pSfxFilenames[e_Sfx_LAST] = {
    "sfx/_pill_bonus.wav", "sfx/_pill_malus.wav",     "sfx/_shot.wav",        "sfx/_door_through.wav",
    "sfx/_menu_click.wav", "sfx/_brick_bounce.wav",   "sfx/_ball_bounce.wav", "sfx/_explosion1.wav", 
    "sfx/_explosion2.wav", "sfx/_brick_dissolve.wav", "sfx/_extra_life.wav",  "sfx/_bat_ping.wav",
    "sfx/_bat_magnet.wav", "sfx/at_War.wav"
  };

  if(!gSfx.nInit)return;

  for(i = 0; i < e_Sfx_LAST; i++)
  {
    if(SDL_LoadWAV(pSfxFilenames[i], &sWave, &pData, &nDLen) == NULL)
    {
      fprintf(stderr, "Couldn't load in sfx : SDL_LoadWAV, %s : %s\n", pSfxFilenames[i], SDL_GetError());
      return;
    }
    // printf("i = %d,  nDLen = %d\n", i, nDLen);
    SDL_BuildAudioCVT(&gSfx.pCvt[i], sWave.format, sWave.channels, sWave.freq,
                      gSfx.sAudioSpec.format, gSfx.sAudioSpec.channels, 
                      gSfx.sAudioSpec.freq);
    // printf("nDLen = %d --- gSfx.pCvt[i].len_mult = %d\n", nDLen, gSfx.pCvt[i].len_mult);
    // Note : on multiplie par len_mult (qui vaut 2 ou 4) car la conversion de données peut augmenter 
    // la taille des données audio (voir filtrage sur http://www.f-legrand.fr).
    gSfx.pCvt[i].buf = (u8 *)malloc(nDLen * gSfx.pCvt[i].len_mult);
    // .buf qui est le double ou fois 4 de nDlen et reçois pData.
    // .buf envoi des données je pense comprise par process qui redirige ces données au la carte son.
    memcpy(gSfx.pCvt[i].buf, pData, nDLen); 
    gSfx.pCvt[i].len = nDLen;
    SDL_ConvertAudio(&gSfx.pCvt[i]);
    SDL_FreeWAV(pData);
  }
}

void Sfx_SoundOn(void)
{
  if(!gSfx.nInit) return;
  SDL_PauseAudio(0);
}

