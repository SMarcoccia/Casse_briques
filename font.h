
//Flags.
#define FONT_NoDispo (1 << 0)


// Affichage d'une phrase en sprites et renvoie la longueur en pixels de la phrase.
// Note : pour le calcul de la longueur et/ou affichage du sprite mettre N° de sprites.
// nNoSpr : N° de sprites.
u32 Font_Print(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags, u32 nNoSpr);
// Integer to ASCII.
void Font_MyItoA(s32 nNb, char *pDst);
