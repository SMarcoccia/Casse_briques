
# MEMO PROGRAMMATION CASSE BRIQUE






# <center><u> __Tutoriel_Arkanoid_v2_Clement_Corde__</u></center>

## breaker.c :

### <u>__Etude graphique du jeu :__</u>
  - les briques sur l'axe des Y partent de l'origine ; il faut compté un écart de 16px entre l'origine et la 1er brique puis on empile les briques.

### <u>__Etude des fonctions :__</u>

- Emplacement : breaker.c

- CollBricks :
  ```C
    // Collisions balle-briques.
    // Renvoie 1 et retourne l'angle quand choc sur une brique.
    u32 CollBricks(struct SBall *pBall, s32 *pnOldX, s32 *pnOldY);
  ```
  - BrickHit :
    ```C
      // Traitement d'une brique (relachement d'items, etc...).
      // Renvoie les flags de la brique. -1 si pas de choc.
      // nBx et nBy récupère les n° respectivement, colonne et ligne.
      // nBallFlags récupère le flag de la balle (traverse les briques ou balle collé à la raquette).
      u32 BrickHit(u32 nBx, u32 nBy, u32 nBallFlags);
    ```
    Récupère info du slot de la brique.
    
  - Commentaire : 
    - Connaître le rang d'une brique (le mur étant le point de départ de la 1ere birque) en x ou en y : on calcule la position de la balle moins le mur (distance entre l'origine et l'extrémité du mur) plus le rayon (permet d'être à l'extrémité de la balle) et on divise par la taille de la brique.
    - Connaître la position d'une brique : 
      - Position mini : on multiplie le rang par la hauteur de la brique et on additionne avec le mur.
      - max  : idem mais on ajoute la hauteur de la brique
    - Présence de la brique dans le tableu de jeu :
      - Vérifie si on dépasse pas la taille du tableau de jeu. Ex. : rang en hauteur < à la taille du tableau.
      - On fait : le rang en y multiplier par la taille du tableau + le rang en largeur.
    - Calcule collision :
      - On prend les coordonnées du pixel min en x et en y de la balle et on regarde si il est à l'interieur de la brique.

  - MstAdd :
    Prototype :
      ```c
        s32 MstAdd(u32 nMstNo, s32 nPosX, s32 nPosY);   
      ```
    Ex. : 
      ```c
        MstAdd(e_Mst_Doh, SCR_Width / 2, 98);
      ```
      - `e_Mst_Doh` : 
        - n° du monstre déclaré dans monsters.h (`enum`).
        - Il est lié au tableau de `struct SMstTb gpMstTb[]` déclarer et initialisé dans monsters.c (qui contient 2 ptr de fc° d'init de de main du monstre/item, d'un ptr *pAnm et d'une variable pour le nb de points).
        - On rechch dans le tableau de slots des monstres un slot libre. Si ok, on met dans la struct gpMstSlots les valeurs qui vont bien.
        - On utilise le ptr de fc° d'init du monstre en envoyant l'adresse du slot que l'on a trouvé précédement (`gpMstSlots[nSlotNo].pFctInit(&gpMstSlots[nSlotNo]);`).
        - retourne le n° du slot.
