
void Sfx_SoundInit(void); // Init sound. Appelé 1 fois.
void Sfx_SoundOn(void); //Starts playback.
void Sfx_SoundOff(void); // Ferme le périphérique audio.
void Sfx_LoadWavFiles(void); // Chargement des fichiers wav.
void Sfx_FreeWavFiles(void); // Libère les ressources occupées par les fichiers WAV.
void Sfx_MixAudio(void *unused, u8 *stream, int len); // Fonction callback.
void Sfx_ClearChannels(void); // Netoyage des cannaux.
void Sfx_PlaySfx(u32 nSfxNo, u32 nSfxPrio); // Jouer le son.

enum
{
 	e_Sfx_PillBonus,
	e_Sfx_PillMalus,
	e_Sfx_Shot,
	e_Sfx_DoorThrough,
	e_Sfx_MenuClic,
	e_Sfx_BrickBounce,
	e_Sfx_BallBounce,
	e_Sfx_Explosion1,
	e_Sfx_Explosion2,
	e_Sfx_BrickDissolve,
	e_Sfx_ExtraLife,
	e_Sfx_BatPing, 
  e_Sfx_BatMagnet, // Pour la raquette magnétisé. Utilisé aussi pour le clic du menu.
  e_Sfx_Wars = 13,
  e_Sfx_LAST = 14
};

enum
{
  e_SfxPrio_10 = 10,
  e_SfxPrio_20 = 20,
  e_SfxPrio_40 = 40
};

