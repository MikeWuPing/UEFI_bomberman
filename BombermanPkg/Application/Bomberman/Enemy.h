#ifndef ENEMY_H
#define ENEMY_H

#include <Uefi.h>

/* Enemy types */
typedef enum {
  ENEMY_RANDOM = 0,   /* random path movement */
  ENEMY_GHOST,        /* can walk through breakable walls */
  ENEMY_SMART,        /* BFS pathfinding to player */
  ENEMY_TYPE_COUNT
} ENEMY_TYPE;

/* Enemy structure */
typedef struct {
  INT32 PosX;         /* pixel position */
  INT32 PosY;
  INT32 Speed;        /* pixels per frame */
  ENEMY_TYPE Type;
  INT32 DirX;         /* current movement direction */
  INT32 DirY;
  INT32 MoveRemainX;  /* remaining pixels to move in current segment */
  INT32 MoveRemainY;
  BOOLEAN IsActive;
} ENEMY;

#define MAX_ENEMIES 10

extern ENEMY gEnemies[MAX_ENEMIES];
extern INT32 gEnemyCount;

VOID EnemyInit(VOID);
VOID EnemyUpdate(VOID);
VOID EnemyDraw(VOID);
BOOLEAN EnemyAtPixel(INT32 Px, INT32 Py, INT32 Pw, INT32 Ph);
VOID EnemyKill(INT32 Index);

#endif /* ENEMY_H */
