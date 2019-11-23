
#define HISC_Nb 10
#define HISC_NameLg (16+1)

extern u32 tab[];

enum{
    e_MENU_Null,
    e_MENU_Main,
    e_MENU_Game,
    e_MENU_HallOfFame,
    e_MENU_Quit,
};

enum{
  e_MENU_State_FadeIn,
  e_MENU_State_FadeOut,
  e_MENU_State_Input
};


struct SMenuGen
{
  u32 nChoiceKey;
  u32 nChoiceMouse;
  // u32 nChoice;
  u32 nState;
  s32 nFadeVal;

  s16 nCursPos; // Position du curseur.
  s16 nCursAccel; // Pour l'accélération.
  //Variable pour saisie du nom.
  s32 nScIdx; // Pos dans la chaîne. Pas remis à 0 !
  char pScName[HISC_NameLg]; // Nom saisie. / Pas remis à 0 !
  u8 nRank; // Rang obtenu par le gamer.
  u8 nKeyDown;
};

struct SMenuGen gMenu;

typedef void (*pFctInit)(void);
typedef u32 (*pFctMain)(void);

// Déplacement de Monstre dans les HighScores.
u32 quadrant(u32 i); // Pour le cercle.
u32 quadrant2(u32 i);
u32 quadrant3(u32 i);
u32 quadrant4(u32 i);
void LineInit(u32 i);
 u32 Line(u32 i);
void CircleInit(u32 i);
void Circle(u32 i);
void Function(u32 i); // Fonction cos sinu équation ...
void FunctionInit(u32 i);

void Menu_Init(void); // Init de variable générale appelé une fois en début de programme.
void Menu_MainInit(void);
u32  Menu_MainMain(void);
u32  Menu_CheckMouseColBox(u32 nSpr1, u32 nPosX1, u32 nPosY2, s32 nXMouse, s32 nYMouse, u32 idx); // Vérifie la collision entre la souris et le menu.
void Menu_KeyArrow(u32 nArrow); // Choix du menu avec les flèches.

void Menu_HighScores_Init(void);
u32  Menu_HighScores_Main(void); // Affichage des high-scores.
void Menu_GetName_Init(void); 
u32  Menu_GetName_Main(void); // Insertion du nom du joueur dans le hall of fame.
// void testHighScrInit(void); 
// u32  testHighScrMain(void); // Insertion du nom du joueur dans le hall of fame.

void Menu_ScrSave(void); // Sauvegarde des fichiers des High-Scores.
void Menu_ScrLoad(void); // Chargement des fichiers des High-Scores.
void Menu_Scr_RazTable(void); // RAZ de la table des High-Scores.
s32  Menu_CheckHighScr(u32 nScorePrm); // Vérifie si un score entre au Hall of Fame.
void Menu_ScrPutNameInTable(char *pName, u32 nRound, u32 nScore); // Place le nom du
// joueur dans la table des High-Scores.

// void Test_nScIdx(void); ???

// Nom du joueur.
void Menu_CursorMove(void); // Déplacement du curseur.
void Menu_CursorInit(void); // Initialisation du curseur.

void Menu_InitFade(void); // Init du fader. Appelé dans les init de chaque Menu.
void Menu_Fade(s32 nFadeVal); // Création du fade.


