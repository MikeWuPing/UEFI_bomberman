#include <Uefi.h>
#include "Map.h"
#include "Gfx.h"

/* Hardcoded 15x15 map from 15x15_test.map */
/* 0=background, 1=unbreakable, 2=walkable, 3=breakable */
INT32 gMapData[MAP_SIZE][MAP_SIZE] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
  {0,1,2,2,2,3,3,3,2,2,2,2,2,1,0},
  {0,1,2,1,2,1,2,1,2,1,2,1,2,1,0},
  {0,1,2,2,2,2,2,2,3,3,3,3,3,1,0},
  {0,1,2,1,2,1,2,1,2,1,3,1,2,1,0},
  {0,1,2,3,2,3,3,2,3,2,2,2,2,1,0},
  {0,1,3,1,2,1,2,1,3,1,2,1,3,1,0},
  {0,1,3,3,2,3,2,2,3,3,3,3,3,1,0},
  {0,1,3,1,3,1,3,1,2,1,2,1,3,1,0},
  {0,1,3,3,3,3,3,3,2,2,2,2,2,1,0},
  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* Original map data for reset */
STATIC CONST INT32 OriginalMap[MAP_SIZE][MAP_SIZE] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
  {0,1,2,2,2,3,3,3,2,2,2,2,2,1,0},
  {0,1,2,1,2,1,2,1,2,1,2,1,2,1,0},
  {0,1,2,2,2,2,2,2,3,3,3,3,3,1,0},
  {0,1,2,1,2,1,2,1,2,1,3,1,2,1,0},
  {0,1,2,3,2,3,3,2,3,2,2,2,2,1,0},
  {0,1,3,1,2,1,2,1,3,1,2,1,3,1,0},
  {0,1,3,3,2,3,2,2,3,3,3,3,3,1,0},
  {0,1,3,1,3,1,3,1,2,1,2,1,3,1,0},
  {0,1,3,3,3,3,3,3,2,2,2,2,2,1,0},
  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

BONUS gBonuses[MAX_BONUSES];
INT32 gBonusCount = 0;

/* Simple PRNG */
STATIC UINT32 gSeed = 12345;

STATIC UINT32
SimpleRand(VOID)
{
  gSeed = gSeed * 1103515245 + 12345;
  return (gSeed >> 16) & 0x7FFF;
}

VOID
MapInit(VOID)
{
  INT32 i, j;
  gBonusCount = 0;
  for (i = 0; i < MAX_BONUSES; i++) {
    gBonuses[i].IsActive = FALSE;
  }
  /* Restore map to original */
  for (i = 0; i < MAP_SIZE; i++) {
    for (j = 0; j < MAP_SIZE; j++) {
      gMapData[i][j] = OriginalMap[i][j];
    }
  }
}

VOID
MapDraw(VOID)
{
  INT32 Row, Col;
  INT32 Px, Py;
  SPRITE *Sp;

  for (Row = 0; Row < MAP_SIZE; Row++) {
    for (Col = 0; Col < MAP_SIZE; Col++) {
      Px = Col * TILE_SIZE;
      Py = Row * TILE_SIZE;

      switch (gMapData[Row][Col]) {
      case TILE_BACKGROUND:
        Sp = &gSprites[SPRITE_BG];
        break;
      case TILE_UNBREAKABLE:
        Sp = &gSprites[SPRITE_WALL_UNBR];
        break;
      case TILE_WALKABLE:
        Sp = &gSprites[SPRITE_FLOOR];
        break;
      case TILE_BREAKABLE:
        Sp = &gSprites[SPRITE_WALL_BR];
        break;
      default:
        Sp = &gSprites[SPRITE_BG];
        break;
      }

      if (Sp->Pixels != NULL) {
        GfxDrawSprite(Sp, Px, Py, TILE_SIZE, TILE_SIZE);
      } else {
        /* Fallback to colored rectangle if sprite not loaded */
        UINT32 Color;
        switch (gMapData[Row][Col]) {
        case TILE_UNBREAKABLE: Color = COLOR_WALL_UNBR; break;
        case TILE_BREAKABLE:   Color = COLOR_WALL_BR; break;
        case TILE_WALKABLE:    Color = COLOR_FLOOR; break;
        default:               Color = COLOR_BG; break;
        }
        GfxDrawRect(Px, Py, TILE_SIZE, TILE_SIZE, Color);
      }
    }
  }
}

VOID
MapDrawBonuses(VOID)
{
  INT32 i;
  INT32 Px, Py;
  SPRITE *Sp;

  for (i = 0; i < MAX_BONUSES; i++) {
    if (!gBonuses[i].IsActive) continue;

    Px = gBonuses[i].TileX * TILE_SIZE;
    Py = gBonuses[i].TileY * TILE_SIZE;

    switch (gBonuses[i].Type) {
    case BONUS_BOMB:
      Sp = &gSprites[SPRITE_BONUS_BOMB];
      break;
    case BONUS_FLAME:
      Sp = &gSprites[SPRITE_BONUS_FLAME];
      break;
    case BONUS_SPEED:
      Sp = &gSprites[SPRITE_BONUS_SPEED];
      break;
    default:
      Sp = NULL;
      break;
    }

    if (Sp != NULL && Sp->Pixels != NULL) {
      /* Draw bonus sprite centered in tile */
      GfxDrawSpriteTransparent(Sp, Px + 16, Py + 16, 32, 32, 0xFFFFFFFF);
    } else {
      /* Fallback */
      UINT32 Color;
      switch (gBonuses[i].Type) {
      case BONUS_BOMB:  Color = COLOR_BONUS_BOMB; break;
      case BONUS_FLAME: Color = COLOR_BONUS_FLAME; break;
      case BONUS_SPEED: Color = COLOR_BONUS_SPEED; break;
      default:          Color = COLOR_WHITE; break;
      }
      GfxDrawRect(Px + 16, Py + 16, 32, 32, Color);
      GfxDrawRectOutline(Px + 16, Py + 16, 32, 32, COLOR_WHITE, 1);
      switch (gBonuses[i].Type) {
      case BONUS_BOMB:  GfxDrawChar(Px + 24, Py + 20, 'B', COLOR_BLACK); break;
      case BONUS_FLAME: GfxDrawChar(Px + 24, Py + 20, 'F', COLOR_BLACK); break;
      case BONUS_SPEED: GfxDrawChar(Px + 24, Py + 20, 'S', COLOR_BLACK); break;
      default: break;
      }
    }
  }
}

BOOLEAN
MapIsWall(INT32 TileX, INT32 TileY)
{
  if (TileX < 0 || TileX >= MAP_SIZE || TileY < 0 || TileY >= MAP_SIZE) {
    return TRUE;
  }
  return gMapData[TileY][TileX] == TILE_UNBREAKABLE ||
         gMapData[TileY][TileX] == TILE_BREAKABLE;
}

BOOLEAN
MapIsBreakable(INT32 TileX, INT32 TileY)
{
  if (TileX < 0 || TileX >= MAP_SIZE || TileY < 0 || TileY >= MAP_SIZE) {
    return FALSE;
  }
  return gMapData[TileY][TileX] == TILE_BREAKABLE;
}

BOOLEAN
MapIsWalkable(INT32 TileX, INT32 TileY)
{
  if (TileX < 0 || TileX >= MAP_SIZE || TileY < 0 || TileY >= MAP_SIZE) {
    return FALSE;
  }
  return gMapData[TileY][TileX] == TILE_WALKABLE;
}

VOID
MapDestroyTile(INT32 TileX, INT32 TileY)
{
  if (TileX < 0 || TileX >= MAP_SIZE || TileY < 0 || TileY >= MAP_SIZE) {
    return;
  }
  if (gMapData[TileY][TileX] == TILE_BREAKABLE) {
    gMapData[TileY][TileX] = TILE_WALKABLE;
    /* 50% chance to spawn bonus */
    if ((SimpleRand() % 100) < 50) {
      MapSpawnBonus(TileX, TileY);
    }
  }
}

VOID
MapSpawnBonus(INT32 TileX, INT32 TileY)
{
  INT32 i;
  for (i = 0; i < MAX_BONUSES; i++) {
    if (!gBonuses[i].IsActive) {
      gBonuses[i].TileX = TileX;
      gBonuses[i].TileY = TileY;
      gBonuses[i].Type = (BONUS_TYPE)(SimpleRand() % BONUS_COUNT);
      gBonuses[i].IsActive = TRUE;
      gBonusCount++;
      return;
    }
  }
}

INT32
MapGetTile(INT32 TileX, INT32 TileY)
{
  if (TileX < 0 || TileX >= MAP_SIZE || TileY < 0 || TileY >= MAP_SIZE) {
    return TILE_UNBREAKABLE;
  }
  return gMapData[TileY][TileX];
}

VOID
MapReset(VOID)
{
  MapInit();
}
