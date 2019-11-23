

void Animspr_ReleaseSlot(s32 nSlotNo); // Libère un slot.
void Animspr_InitEngine(void); // RAZ moteur pour les anims.

// Init une anim si ce n'est pas la même que précédement, et si priorité ok. Pour les bonus.
s32 Animspr_AnmSetIfNew(u32 *pAnm, s32 nSlotNo);
s32 Animspr_GetImage(s32 nSlotNo); // Renvoie l'image en cours et avance l'anim.
s32 Animspr_GetSlot(void); // Cherche un slot libre.
s32 Animspr_Set(u32 *pAnm, s32 nSlotNo); // Init d'une anim. On place le numero du slot pour l'init.
s32 Animspr_GetLastImage(s32 nSlotNo); // Renvoi la dernière image affichée.

u32 Animspr_GetKey(s32 nSlotNo); // Récupère la clé d'une anim.
// Test si anim est terminée (e_Anm_End).
// 0 si pas terminée.
u32 Animspr_CheckEnd(s32 nSlotNo); 

