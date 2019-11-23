
// RAZ moteur.
void Dust_InitEngine(void);

// Init d'une anim.
// Out : N° du slot. -1 si erreur.
s32 Dust_DustSet(u64 *pAnm, s32 nPosX, s32 nPosY);

// Cherche un slot libre.
// Out : N° d'un slot libre. -1 si erreur.
s32 Dust_GetSlot(void);
