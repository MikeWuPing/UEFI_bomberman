#include <Uefi.h>
#include "Enemy.h"
#include "Map.h"
#include "Gfx.h"
#include "Player.h"

/* Forward declarations for player position */
extern INT32 gPlayerTileX;
extern INT32 gPlayerTileY;

ENEMY gEnemies[MAX_ENEMIES];
INT32 gEnemyCount = 0;

/* Simple PRNG for enemy movement */
STATIC UINT32 gEnemySeed = 54321;

STATIC UINT32
EnemyRand(VOID)
{
  gEnemySeed = gEnemySeed * 1103515245 + 12345;
  return (gEnemySeed >> 16) & 0x7FFF;
}

VOID
EnemyInit(VOID)
{
  INT32 i;
  gEnemyCount = 0;
  for (i = 0; i < MAX_ENEMIES; i++) {
    gEnemies[i].IsActive = FALSE;
  }

  /* Create enemies for single player mode */
  /* EnemyOvapi (smart/BFS) at tile (12, 3) */
  if (gEnemyCount < MAX_ENEMIES) {
    gEnemies[gEnemyCount].PosX = 12 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].PosY = 3 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].Speed = 2;
    gEnemies[gEnemyCount].Type = ENEMY_SMART;
    gEnemies[gEnemyCount].DirX = 0;
    gEnemies[gEnemyCount].DirY = 0;
    gEnemies[gEnemyCount].MoveRemainX = 0;
    gEnemies[gEnemyCount].MoveRemainY = 0;
    gEnemies[gEnemyCount].IsActive = TRUE;
    gEnemyCount++;
  }

  /* Enemy (random) at tile (12, 11) */
  if (gEnemyCount < MAX_ENEMIES) {
    gEnemies[gEnemyCount].PosX = 12 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].PosY = 11 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].Speed = 2;
    gEnemies[gEnemyCount].Type = ENEMY_RANDOM;
    gEnemies[gEnemyCount].DirX = 0;
    gEnemies[gEnemyCount].DirY = 0;
    gEnemies[gEnemyCount].MoveRemainX = 0;
    gEnemies[gEnemyCount].MoveRemainY = 0;
    gEnemies[gEnemyCount].IsActive = TRUE;
    gEnemyCount++;
  }

  /* EnemyGhost at tile (12, 11) */
  if (gEnemyCount < MAX_ENEMIES) {
    gEnemies[gEnemyCount].PosX = 12 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].PosY = 11 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].Speed = 2;
    gEnemies[gEnemyCount].Type = ENEMY_GHOST;
    gEnemies[gEnemyCount].DirX = 0;
    gEnemies[gEnemyCount].DirY = 0;
    gEnemies[gEnemyCount].MoveRemainX = 0;
    gEnemies[gEnemyCount].MoveRemainY = 0;
    gEnemies[gEnemyCount].IsActive = TRUE;
    gEnemyCount++;
  }

  /* EnemyGhost at tile (2, 8) */
  if (gEnemyCount < MAX_ENEMIES) {
    gEnemies[gEnemyCount].PosX = 2 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].PosY = 8 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
    gEnemies[gEnemyCount].Speed = 2;
    gEnemies[gEnemyCount].Type = ENEMY_GHOST;
    gEnemies[gEnemyCount].DirX = 0;
    gEnemies[gEnemyCount].DirY = 0;
    gEnemies[gEnemyCount].MoveRemainX = 0;
    gEnemies[gEnemyCount].MoveRemainY = 0;
    gEnemies[gEnemyCount].IsActive = TRUE;
    gEnemyCount++;
  }
}

/* Check if enemy can move to tile (considering type) */
STATIC BOOLEAN
CanMoveTo(INT32 TileX, INT32 TileY, ENEMY_TYPE Type)
{
  INT32 tile;

  if (TileX < 0 || TileX >= MAP_SIZE || TileY < 0 || TileY >= MAP_SIZE) {
    return FALSE;
  }

  tile = MapGetTile(TileX, TileY);

  if (tile == TILE_WALKABLE) {
    return TRUE;
  }

  /* Ghost can walk through breakable walls */
  if (Type == ENEMY_GHOST && tile == TILE_BREAKABLE) {
    return TRUE;
  }

  return FALSE;
}

/* Random movement AI - picks first available direction, excluding reverse */
STATIC VOID
EnemyRandomUpdate(ENEMY *e)
{
  INT32 TileX = e->PosX / TILE_SIZE;
  INT32 TileY = e->PosY / TILE_SIZE;
  INT32 dx[4] = {0, 1, 0, -1}; /* up, right, down, left */
  INT32 dy[4] = {-1, 0, 1, 0};
  INT32 dir;
  INT32 count;
  INT32 bestDir = -1;

  /* If still moving in current direction, continue */
  if (e->MoveRemainX != 0 || e->MoveRemainY != 0) {
    /* Move by speed pixels */
    if (e->MoveRemainX != 0) {
      INT32 move = e->Speed;
      if (e->MoveRemainX > 0) {
        if (move > e->MoveRemainX) move = e->MoveRemainX;
        e->PosX += move;
        e->MoveRemainX -= move;
      } else {
        if (move > -e->MoveRemainX) move = -e->MoveRemainX;
        e->PosX -= move;
        e->MoveRemainX += move;
      }
    }
    if (e->MoveRemainY != 0) {
      INT32 move = e->Speed;
      if (e->MoveRemainY > 0) {
        if (move > e->MoveRemainY) move = e->MoveRemainY;
        e->PosY += move;
        e->MoveRemainY -= move;
      } else {
        if (move > -e->MoveRemainY) move = -e->MoveRemainY;
        e->PosY -= move;
        e->MoveRemainY += move;
      }
    }
    return;
  }

  /* At tile boundary - pick new direction */
  /* Priority: UP, RIGHT, DOWN, LEFT (but not reverse of previous) */
  for (dir = 0; dir < 4; dir++) {
    INT32 nx = TileX + dx[dir];
    INT32 ny = TileY + dy[dir];

    /* Skip reverse direction */
    if (e->DirX == -dx[dir] && e->DirY == -dy[dir] &&
        (e->DirX != 0 || e->DirY != 0)) {
      continue;
    }

    if (CanMoveTo(nx, ny, e->Type)) {
      bestDir = dir;
      break;
    }
  }

  /* If no non-reverse direction found, try reverse */
  if (bestDir < 0) {
    for (dir = 0; dir < 4; dir++) {
      INT32 nx = TileX + dx[dir];
      INT32 ny = TileY + dy[dir];
      if (CanMoveTo(nx, ny, e->Type)) {
        bestDir = dir;
        break;
      }
    }
  }

  if (bestDir >= 0) {
    e->DirX = dx[bestDir];
    e->DirY = dy[bestDir];

    /* Count consecutive walkable tiles in this direction */
    count = 0;
    {
      INT32 cx = TileX;
      INT32 cy = TileY;
      while (1) {
        cx += e->DirX;
        cy += e->DirY;
        if (!CanMoveTo(cx, cy, e->Type)) break;
        count++;
      }
    }

    if (count > 0) {
      e->MoveRemainX = e->DirX * TILE_SIZE * count;
      e->MoveRemainY = e->DirY * TILE_SIZE * count;
    }
  }
}

/* BFS pathfinding for smart enemy */
#define BFS_QUEUE_SIZE 256

typedef struct {
  INT32 X, Y;
} POINT;

/* BFS node: stores the full path as parent chain */
typedef struct {
  POINT Pos;
  INT32 Parent;  /* index of parent node, -1 for root */
} BFS_NODE;

STATIC INT32 gBFSPathX[64];
STATIC INT32 gBFSPathY[64];
STATIC INT32 gBFSPathLen = 0;

STATIC VOID
BFSFindPath(INT32 StartX, INT32 StartY, INT32 GoalX, INT32 GoalY)
{
  BFS_NODE nodes[BFS_QUEUE_SIZE];
  BOOLEAN visited[MAP_SIZE][MAP_SIZE];
  INT32 front = 0, back = 0;
  INT32 dx[4] = {0, 1, 0, -1};
  INT32 dy[4] = {-1, 0, 1, 0};
  INT32 i, j;
  INT32 found = -1;

  gBFSPathLen = 0;

  for (i = 0; i < MAP_SIZE; i++) {
    for (j = 0; j < MAP_SIZE; j++) {
      visited[i][j] = FALSE;
    }
  }

  /* Start node */
  nodes[back].Pos.X = StartX;
  nodes[back].Pos.Y = StartY;
  nodes[back].Parent = -1;
  visited[StartY][StartX] = TRUE;
  back++;

  while (front < back && back < BFS_QUEUE_SIZE) {
    BFS_NODE cur = nodes[front];

    if (cur.Pos.X == GoalX && cur.Pos.Y == GoalY) {
      found = front;
      break;
    }

    for (i = 0; i < 4; i++) {
      INT32 nx = cur.Pos.X + dx[i];
      INT32 ny = cur.Pos.Y + dy[i];

      if (nx < 0 || nx >= MAP_SIZE || ny < 0 || ny >= MAP_SIZE) continue;
      if (visited[ny][nx]) continue;
      if (MapGetTile(nx, ny) != TILE_WALKABLE) continue;

      visited[ny][nx] = TRUE;
      nodes[back].Pos.X = nx;
      nodes[back].Pos.Y = ny;
      nodes[back].Parent = front;
      back++;
    }

    front++;
  }

  /* Reconstruct path */
  if (found >= 0) {
    INT32 idx = found;
    INT32 pathLen = 0;

    /* Count path length */
    {
      INT32 tmp = idx;
      while (tmp >= 0) {
        pathLen++;
        tmp = nodes[tmp].Parent;
      }
    }

    /* Store path (reversed - from start to goal) */
    if (pathLen <= 64) {
      INT32 p = pathLen - 1;
      idx = found;
      while (idx >= 0 && p >= 0) {
        gBFSPathX[p] = nodes[idx].Pos.X;
        gBFSPathY[p] = nodes[idx].Pos.Y;
        idx = nodes[idx].Parent;
        p--;
      }
      gBFSPathLen = pathLen;
    }
  }
}

STATIC VOID
EnemySmartUpdate(ENEMY *e)
{
  INT32 TileX = e->PosX / TILE_SIZE;
  INT32 TileY = e->PosY / TILE_SIZE;
  INT32 alignedX = e->PosX % TILE_SIZE;
  INT32 alignedY = e->PosY % TILE_SIZE;

  /* Only recalculate path when aligned to tile grid */
  if (alignedX == 0 && alignedY == 0) {
    if (gBFSPathLen <= 1) {
      BFSFindPath(TileX, TileY, gPlayerTileX, gPlayerTileY);
    }

    if (gBFSPathLen > 1) {
      /* Move toward next path node */
      INT32 nextX = gBFSPathX[1];
      INT32 nextY = gBFSPathY[1];
      INT32 targetPx = nextX * TILE_SIZE;
      INT32 targetPy = nextY * TILE_SIZE;

      e->DirX = (targetPx > e->PosX) ? 1 : (targetPx < e->PosX) ? -1 : 0;
      e->DirY = (targetPy > e->PosY) ? 1 : (targetPy < e->PosY) ? -1 : 0;

      e->MoveRemainX = e->DirX * TILE_SIZE;
      e->MoveRemainY = e->DirY * TILE_SIZE;

      /* Shift path */
      {
        INT32 i;
        for (i = 0; i < gBFSPathLen - 1; i++) {
          gBFSPathX[i] = gBFSPathX[i + 1];
          gBFSPathY[i] = gBFSPathY[i + 1];
        }
        gBFSPathLen--;
      }
    } else {
      /* No path found, fall back to random movement */
      EnemyRandomUpdate(e);
      return;
    }
  }

  /* Execute movement */
  if (e->MoveRemainX != 0 || e->MoveRemainY != 0) {
    if (e->MoveRemainX != 0) {
      INT32 move = e->Speed;
      if (e->MoveRemainX > 0) {
        if (move > e->MoveRemainX) move = e->MoveRemainX;
        e->PosX += move;
        e->MoveRemainX -= move;
      } else {
        if (move > -e->MoveRemainX) move = -e->MoveRemainX;
        e->PosX -= move;
        e->MoveRemainX += move;
      }
    }
    if (e->MoveRemainY != 0) {
      INT32 move = e->Speed;
      if (e->MoveRemainY > 0) {
        if (move > e->MoveRemainY) move = e->MoveRemainY;
        e->PosY += move;
        e->MoveRemainY -= move;
      } else {
        if (move > -e->MoveRemainY) move = -e->MoveRemainY;
        e->PosY -= move;
        e->MoveRemainY += move;
      }
    }
  }
}

VOID
EnemyUpdate(VOID)
{
  INT32 i;

  for (i = 0; i < MAX_ENEMIES; i++) {
    if (!gEnemies[i].IsActive) continue;

    switch (gEnemies[i].Type) {
    case ENEMY_RANDOM:
    case ENEMY_GHOST:
      EnemyRandomUpdate(&gEnemies[i]);
      break;
    case ENEMY_SMART:
      EnemySmartUpdate(&gEnemies[i]);
      break;
    default:
      break;
    }
  }
}

VOID
EnemyDraw(VOID)
{
  INT32 i;

  for (i = 0; i < MAX_ENEMIES; i++) {
    SPRITE *Sp;
    UINT32 FallbackColor;

    if (!gEnemies[i].IsActive) continue;

    switch (gEnemies[i].Type) {
    case ENEMY_RANDOM:
      Sp = &gSprites[SPRITE_ENEMY_COMMON];
      FallbackColor = COLOR_ENEMY;
      break;
    case ENEMY_GHOST:
      Sp = &gSprites[SPRITE_ENEMY_GHOST];
      FallbackColor = COLOR_ENEMY_GHOST;
      break;
    case ENEMY_SMART:
      Sp = &gSprites[SPRITE_ENEMY_SMART];
      FallbackColor = COLOR_ENEMY_SMART;
      break;
    default:
      Sp = &gSprites[SPRITE_ENEMY_COMMON];
      FallbackColor = COLOR_ENEMY;
      break;
    }

    if (Sp->Pixels != NULL) {
      GfxDrawSpriteTransparent(Sp, gEnemies[i].PosX, gEnemies[i].PosY,
                               PLAYER_SIZE, PLAYER_SIZE, 0xFFFFFFFF);
    } else {
      /* Fallback */
      GfxDrawRect(gEnemies[i].PosX, gEnemies[i].PosY,
                  PLAYER_SIZE, PLAYER_SIZE, FallbackColor);
      GfxDrawCircle(gEnemies[i].PosX + PLAYER_SIZE / 3,
                    gEnemies[i].PosY + PLAYER_SIZE / 3, 5, COLOR_WHITE);
      GfxDrawCircle(gEnemies[i].PosX + 2 * PLAYER_SIZE / 3,
                    gEnemies[i].PosY + PLAYER_SIZE / 3, 5, COLOR_WHITE);
      GfxDrawCircle(gEnemies[i].PosX + PLAYER_SIZE / 3 + 2,
                    gEnemies[i].PosY + PLAYER_SIZE / 3, 2, COLOR_BLACK);
      GfxDrawCircle(gEnemies[i].PosX + 2 * PLAYER_SIZE / 3 + 2,
                    gEnemies[i].PosY + PLAYER_SIZE / 3, 2, COLOR_BLACK);
    }
  }
}

BOOLEAN
EnemyAtPixel(INT32 Px, INT32 Py, INT32 Pw, INT32 Ph)
{
  INT32 i;
  INT32 ex, ey, ew, eh;

  for (i = 0; i < MAX_ENEMIES; i++) {
    if (!gEnemies[i].IsActive) continue;

    ex = gEnemies[i].PosX;
    ey = gEnemies[i].PosY;
    ew = PLAYER_SIZE;
    eh = PLAYER_SIZE;

    /* AABB collision */
    if (Px < ex + ew && Px + Pw > ex &&
        Py < ey + eh && Py + Ph > ey) {
      return TRUE;
    }
  }
  return FALSE;
}

VOID
EnemyKill(INT32 Index)
{
  if (Index >= 0 && Index < MAX_ENEMIES) {
    gEnemies[Index].IsActive = FALSE;
    gEnemyCount--;
  }
}
