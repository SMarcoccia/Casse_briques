

struct SMstTb
{
  void (*pFctInit) (struct SMstCommon *pMst);
  s32 (*pFctMain) (struct SMstCommon *pMst);
  u32 *pAnm;
  u16 nPoints;
};


// Liste des monstres.
enum{
  e_Mst_Pill_0 = 0, // Pour base du random.
  e_Mst_Pill_Aimant = e_Mst_Pill_0,
  e_Mst_Pill_Mitrailleuse,
  e_Mst_Pill_BallTraversante,
  e_Mst_Pill_BallBigger,
  e_Mst_Pill_BallX3,
  e_Mst_Pill_RaqRallonge,
  e_Mst_Pill_RaqReduit,
  e_Mst_Pill_1Up,
  e_Mst_Pill_DoorR,
  e_Mst_Pill_SpeedUp,
  e_Mst_Pill_SpeedDown,
  e_Mst_Generateur, // Générateur d'ennemis.
  e_Mst_Mst1,
  e_Mst_DoorR, // Porte à droite pour passer à un autre niveau.
  e_Mst_Doh // Doh !
};

// Renvoie le nombre de pillules présentes à l'écran.
u32 Monsters_InitPillGetNb(void);