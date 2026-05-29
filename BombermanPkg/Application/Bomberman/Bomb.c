#include <Uefi.h>
#include "Bomb.h"
#include "Map.h"
#include "Gfx.h"

BOMB gBombs[MAX_BOMBS];
INT32 gBombCount = 0;
FLAME gFlames[MAX_FLAMES];
INT32 gFlameCount = 0;

VOID
BombInit(VOID)
{
  INT32 i;
  gBombCount = 0;
  gFlameCount = 0;
  for (i = 0; i < MAX_BOMBS; i++) {
    gBombs[i].IsActive = FALSE;
  }
  for (i = 0; i < MAX_FLAMES; i++) {
    gFlames[i].IsActive = FALSE;
  }
}

VOID
BombPlace(INT32 TileX, INT32 TileY)
{
  INT32 i;

  /* Check if bomb already at this tile */
  for (i = 0; i < MAX_BOMBS; i++) {
    if (gBombs[i].IsActive &&
        gBombs[i].TileX == TileX &&
        gBombs[i].TileY == TileY) {
      return; /* already a bomb here */
    }
  }

  /* Find free slot */
  for (i = 0; i < MAX_BOMBS; i++) {
    if (!gBombs[i].IsActive) {
      gBombs[i].TileX = TileX;
      gBombs[i].TileY = TileY;
      gBombs[i].Timer = BOMB_TIMER;
      gBombs[i].IsActive = TRUE;
      gBombCount++;
      return;
    }
  }
}

/* Add a flame at given tile if no flame already there */
STATIC VOID
AddFlame(INT32 TileX, INT32 TileY)
{
  INT32 i;

  /* Don't place flame on unbreakable walls */
  if (MapGetTile(TileX, TileY) == TILE_UNBREAKABLE) {
    return;
  }

  /* Check for existing flame */
  for (i = 0; i < MAX_FLAMES; i++) {
    if (gFlames[i].IsActive &&
        gFlames[i].TileX == TileX &&
        gFlames[i].TileY == TileY) {
      return; /* already a flame here */
    }
  }

  for (i = 0; i < MAX_FLAMES; i++) {
    if (!gFlames[i].IsActive) {
      gFlames[i].TileX = TileX;
      gFlames[i].TileY = TileY;
      gFlames[i].Timer = FLAME_DURATION;
      gFlames[i].IsActive = TRUE;
      gFlameCount++;
      return;
    }
  }
}

/* Explode a bomb - create flames in 4 directions */
/* Forward declaration for flame range - defined in Player.c */
extern INT32 gPlayerFlameRange;

STATIC VOID
BombExplode(INT32 Index)
{
  INT32 bx = gBombs[Index].TileX;
  INT32 by = gBombs[Index].TileY;
  INT32 dir;
  INT32 step;
  INT32 dx[4] = {0, 1, 0, -1}; /* up, right, down, left */
  INT32 dy[4] = {-1, 0, 1, 0};
  INT32 range = gPlayerFlameRange;

  /* Place flame at bomb position */
  AddFlame(bx, by);

  /* Extend flames in 4 directions */
  for (dir = 0; dir < 4; dir++) {
    for (step = 1; step <= range; step++) {
      INT32 tx = bx + dx[dir] * step;
      INT32 ty = by + dy[dir] * step;

      if (tx < 0 || tx >= MAP_SIZE || ty < 0 || ty >= MAP_SIZE) {
        break;
      }

      if (MapGetTile(tx, ty) == TILE_UNBREAKABLE) {
        break; /* blocked by unbreakable wall */
      }

      if (MapIsBreakable(tx, ty)) {
        /* Destroy breakable wall, place flame, stop in this direction */
        MapDestroyTile(tx, ty);
        AddFlame(tx, ty);
        break;
      }

      /* Walkable tile - place flame and continue */
      AddFlame(tx, ty);
    }
  }

  /* Remove bomb */
  gBombs[Index].IsActive = FALSE;
  gBombCount--;
}

VOID
BombUpdate(VOID)
{
  INT32 i;

  for (i = 0; i < MAX_BOMBS; i++) {
    if (gBombs[i].IsActive) {
      gBombs[i].Timer--;
      if (gBombs[i].Timer <= 0) {
        BombExplode(i);
      }
    }
  }
}

VOID
BombDraw(VOID)
{
  INT32 i;
  INT32 Px, Py;
  SPRITE *Sp = &gSprites[SPRITE_BOMB];

  for (i = 0; i < MAX_BOMBS; i++) {
    if (!gBombs[i].IsActive) continue;

    Px = gBombs[i].TileX * TILE_SIZE;
    Py = gBombs[i].TileY * TILE_SIZE;

    if (Sp->Pixels != NULL) {
      INT32 BombSize = TILE_SIZE * 3 / 4;
      INT32 Offset = (TILE_SIZE - BombSize) / 2;
      GfxDrawSpriteTransparent(Sp, Px + Offset, Py + Offset,
                               BombSize, BombSize, 0xFFFFFFFF);
    } else {
      /* Fallback */
      INT32 Size = TILE_SIZE * 3 / 4;
      GfxDrawCircle(Px + TILE_SIZE / 2, Py + TILE_SIZE / 2,
                    Size / 2, COLOR_BOMB);
      GfxDrawRect(Px + TILE_SIZE / 2 - 2,
                  Py + TILE_SIZE / 2 - Size / 2 - 6, 4, 6, COLOR_RED);
    }

    /* Draw timer indicator */
    if (gBombs[i].Timer < 60) {
      GfxDrawRectOutline(Px + 4, Py + 4,
                         TILE_SIZE - 8, TILE_SIZE - 8, COLOR_RED, 2);
    }
  }
}

VOID
FlameUpdate(VOID)
{
  INT32 i;

  for (i = 0; i < MAX_FLAMES; i++) {
    if (gFlames[i].IsActive) {
      gFlames[i].Timer--;
      if (gFlames[i].Timer <= 0) {
        gFlames[i].IsActive = FALSE;
        gFlameCount--;
      }
    }
  }
}

VOID
FlameDraw(VOID)
{
  INT32 i;
  INT32 Px, Py;
  SPRITE *Sp = &gSprites[SPRITE_FLAME];

  for (i = 0; i < MAX_FLAMES; i++) {
    if (!gFlames[i].IsActive) continue;

    Px = gFlames[i].TileX * TILE_SIZE;
    Py = gFlames[i].TileY * TILE_SIZE;

    if (Sp->Pixels != NULL) {
      GfxDrawSpriteTransparent(Sp, Px, Py, TILE_SIZE, TILE_SIZE, 0xFFFFFFFF);
    } else {
      /* Fallback */
      INT32 Size = TILE_SIZE - 4;
      GfxDrawRect(Px + 2, Py + 2, Size, Size, COLOR_FLAME);
      GfxDrawRect(Px + 12, Py + 12, TILE_SIZE - 24, TILE_SIZE - 24, COLOR_YELLOW);
      GfxDrawRect(Px + 22, Py + 22, 20, 20, COLOR_WHITE);
    }
  }
}

BOOLEAN
FlameAtTile(INT32 TileX, INT32 TileY)
{
  INT32 i;
  for (i = 0; i < MAX_FLAMES; i++) {
    if (gFlames[i].IsActive &&
        gFlames[i].TileX == TileX &&
        gFlames[i].TileY == TileY) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOLEAN
BombAtTile(INT32 TileX, INT32 TileY)
{
  INT32 i;
  for (i = 0; i < MAX_BOMBS; i++) {
    if (gBombs[i].IsActive &&
        gBombs[i].TileX == TileX &&
        gBombs[i].TileY == TileY) {
      return TRUE;
    }
  }
  return FALSE;
}
