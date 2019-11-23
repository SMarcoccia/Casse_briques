
// Cherche un slot libre.
// Out : N° d'un slot libre. -1 si erreur.
s32 Fire_GetSlot(void);
// Init d'un tir.
// In : sAngle = -1 => On prend l'angle par défaut. Sinon val [0;255] => Angle.
// Out : N° du slot. -1 si erreur.
s32 Fire_Add(u32 nShot, s32 nPosX, s32 nPosY, s32 nAngle);
void Fire_InitEngine(void);
// Teste si un monstre se prend un tir. Sert aussi pour Doh contre le joueur.
// Out: 0 = Pas de choc / 1 = Hit.
u32 Fire_MstCheckFire(u32 nSpr, s32 nPosX, s32 nPosY); 
void Fire_ReleaseSlot(u32 nSlotNo); // Libère les slots.
