#ifndef MAP_H
#define MAP_H

#include <Uefi.h>

/* Tile types */
#define TILE_BACKGROUND   0
#define TILE_UNBREAKABLE  1
#define TILE_WALKABLE     2
#define TILE_BREAKABLE    3

/* Map dimensions */
#define MAP_SIZE    15
#define TILE_SIZE   64  /* pixels per tile on screen (32 * 2 scale) */

/* Bonus types */
typedef enum {
  BONUS_BOMB = 0,
  BONUS_FLAME,
  BONUS_SPEED,
  BONUS_COUNT
} BONUS_TYPE;

/* Bonus structure */
typedef struct {
  INT32 TileX;
  INT32 TileY;
  BONUS_TYPE Type;
  BOOLEAN IsActive;
} BONUS;

/* Max bonuses on map */
#define MAX_BONUSES 50

extern INT32 gMapData[MAP_SIZE][MAP_SIZE];
extern BONUS gBonuses[MAX_BONUSES];
extern INT32 gBonusCount;

VOID MapInit(VOID);
VOID MapDraw(VOID);
VOID MapDrawBonuses(VOID);
BOOLEAN MapIsWall(INT32 TileX, INT32 TileY);
BOOLEAN MapIsBreakable(INT32 TileX, INT32 TileY);
BOOLEAN MapIsWalkable(INT32 TileX, INT32 TileY);
VOID MapDestroyTile(INT32 TileX, INT32 TileY);
VOID MapSpawnBonus(INT32 TileX, INT32 TileY);
INT32 MapGetTile(INT32 TileX, INT32 TileY);
VOID MapReset(VOID);

#endif /* MAP_H */
