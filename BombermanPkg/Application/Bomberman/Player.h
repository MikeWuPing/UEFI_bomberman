#ifndef PLAYER_H
#define PLAYER_H

#include <Uefi.h>
#include "Input.h"

/* Player structure */
typedef struct {
  INT32 PosX;         /* pixel position */
  INT32 PosY;
  INT32 Speed;        /* pixels per frame (scaled by 10 for precision) */
  INT32 BombCount;    /* max active bombs */
  INT32 ActiveBombs;  /* currently placed bombs */
  INT32 FlameRange;   /* explosion range in tiles */
  INT32 Score;
  INT32 FlameCounter; /* frame counter for auto flame range increase */
  BOOLEAN IsAlive;
} PLAYER;

#define PLAYER_SIZE   48  /* 32 * 1.5 scale */
#define PLAYER_SPEED  30  /* 3.0 * 10 for integer math */

extern PLAYER gPlayer;

VOID PlayerInit(VOID);
VOID PlayerUpdate(INPUT_STATE *Input);
VOID PlayerDraw(VOID);
VOID PlayerAddBonus(INT32 BonusType);
BOOLEAN PlayerCheckFlameCollision(VOID);
BOOLEAN PlayerCheckEnemyCollision(VOID);
BOOLEAN PlayerCheckBonusCollision(VOID);

#endif /* PLAYER_H */
