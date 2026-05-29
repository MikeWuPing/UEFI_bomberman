#ifndef BOMB_H
#define BOMB_H

#include <Uefi.h>

/* Constants */
#define BOMB_TIMER        180   /* frames until explosion (3 sec at 60fps) */
#define FLAME_DURATION    30    /* frames flames last (0.5 sec) */
#define MAX_FLAME_RANGE   10
#define MAX_BOMBS         10
#define MAX_FLAMES        100

/* Bomb structure */
typedef struct {
  INT32 TileX;
  INT32 TileY;
  INT32 Timer;        /* countdown to explosion */
  BOOLEAN IsActive;
} BOMB;

/* Flame structure */
typedef struct {
  INT32 TileX;
  INT32 TileY;
  INT32 Timer;        /* countdown to disappear */
  BOOLEAN IsActive;
} FLAME;

extern BOMB gBombs[MAX_BOMBS];
extern INT32 gBombCount;
extern FLAME gFlames[MAX_FLAMES];
extern INT32 gFlameCount;

VOID BombInit(VOID);
VOID BombPlace(INT32 TileX, INT32 TileY);
VOID BombUpdate(VOID);
VOID BombDraw(VOID);
VOID FlameDraw(VOID);
VOID FlameUpdate(VOID);
BOOLEAN FlameAtTile(INT32 TileX, INT32 TileY);
BOOLEAN BombAtTile(INT32 TileX, INT32 TileY);

#endif /* BOMB_H */
