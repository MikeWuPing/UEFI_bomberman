#include <Uefi.h>
#include "Player.h"
#include "Bomb.h"
#include "Map.h"
#include "Enemy.h"
#include "Gfx.h"

PLAYER gPlayer;

/* Global player tile position for enemy AI */
INT32 gPlayerTileX = 2;
INT32 gPlayerTileY = 3;

/* Global flame range for bomb explosion */
INT32 gPlayerFlameRange = 1;

/* Max constants */
#define MAX_FLAME_RANGE  10
#define MAX_SPEED        50  /* 5.0 * 10 */

VOID
PlayerInit(VOID)
{
  gPlayer.PosX = 2 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
  gPlayer.PosY = 3 * TILE_SIZE + (TILE_SIZE - PLAYER_SIZE) / 2;
  gPlayer.Speed = PLAYER_SPEED;  /* 30 = 3.0 * 10 */
  gPlayer.BombCount = 1;
  gPlayer.ActiveBombs = 0;
  gPlayer.FlameRange = 1;
  gPlayer.Score = 0;
  gPlayer.FlameCounter = 0;
  gPlayer.IsAlive = TRUE;

  gPlayerFlameRange = 1;
  gPlayerTileX = 2;
  gPlayerTileY = 3;
}

/* Check if player rect collides with any wall */
STATIC BOOLEAN
PlayerWallCollision(INT32 X, INT32 Y, INT32 W, INT32 H)
{
  /* Convert pixel rect to tile range and check each tile */
  INT32 TileLeft = X / TILE_SIZE;
  INT32 TileTop = Y / TILE_SIZE;
  INT32 TileRight = (X + W - 1) / TILE_SIZE;
  INT32 TileBottom = (Y + H - 1) / TILE_SIZE;
  INT32 tx, ty;

  for (ty = TileTop; ty <= TileBottom; ty++) {
    for (tx = TileLeft; tx <= TileRight; tx++) {
      if (MapIsWall(tx, ty)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

VOID
PlayerUpdate(INPUT_STATE *Input)
{
  INT32 NewX, NewY;
  INT32 SpeedPixels;
  INT32 DirX = 0, DirY = 0;
  BOOLEAN WantBomb = FALSE;

  if (!gPlayer.IsAlive) return;

  /* Auto flame range increase every 600 frames (10 sec) */
  gPlayer.FlameCounter++;
  if (gPlayer.FlameCounter >= 600) {
    gPlayer.FlameCounter = 0;
    if (gPlayer.FlameRange < MAX_FLAME_RANGE) {
      gPlayer.FlameRange++;
      gPlayerFlameRange = gPlayer.FlameRange;
    }
  }

  /* Process input */
  if (Input->Up) {
    DirY = -1;
  } else if (Input->Down) {
    DirY = 1;
  } else if (Input->Left) {
    DirX = -1;
  } else if (Input->Right) {
    DirX = 1;
  }

  if (Input->Bomb) {
    WantBomb = TRUE;
  }

  /* Calculate speed in pixels (speed / 10 to get actual pixels) */
  SpeedPixels = gPlayer.Speed / 10;
  if (SpeedPixels < 1) SpeedPixels = 1;

  /* Apply movement with collision detection */
  NewX = gPlayer.PosX + DirX * SpeedPixels;
  NewY = gPlayer.PosY + DirY * SpeedPixels;

  /* Check X movement */
  if (DirX != 0) {
    if (!PlayerWallCollision(NewX, gPlayer.PosY, PLAYER_SIZE, PLAYER_SIZE)) {
      gPlayer.PosX = NewX;
    }
  }

  /* Check Y movement */
  if (DirY != 0) {
    if (!PlayerWallCollision(gPlayer.PosX, NewY, PLAYER_SIZE, PLAYER_SIZE)) {
      gPlayer.PosY = NewY;
    }
  }

  /* Clamp to screen bounds */
  if (gPlayer.PosX < 0) gPlayer.PosX = 0;
  if (gPlayer.PosY < 0) gPlayer.PosY = 0;
  if (gPlayer.PosX + PLAYER_SIZE > MAP_SIZE * TILE_SIZE) {
    gPlayer.PosX = MAP_SIZE * TILE_SIZE - PLAYER_SIZE;
  }
  if (gPlayer.PosY + PLAYER_SIZE > MAP_SIZE * TILE_SIZE) {
    gPlayer.PosY = MAP_SIZE * TILE_SIZE - PLAYER_SIZE;
  }

  /* Update tile position */
  gPlayerTileX = (gPlayer.PosX + PLAYER_SIZE / 2) / TILE_SIZE;
  gPlayerTileY = (gPlayer.PosY + PLAYER_SIZE / 2) / TILE_SIZE;

  /* Place bomb */
  if (WantBomb) {
    INT32 activeBombs = 0;
    INT32 i;

    /* Count active bombs placed by player */
    for (i = 0; i < MAX_BOMBS; i++) {
      if (gBombs[i].IsActive) activeBombs++;
    }

    if (activeBombs < gPlayer.BombCount) {
      /* Don't place if bomb already at this tile */
      if (!BombAtTile(gPlayerTileX, gPlayerTileY)) {
        BombPlace(gPlayerTileX, gPlayerTileY);
      }
    }
  }

  /* Check bonus collision */
  {
    INT32 i;
    INT32 PlayerCenterX = gPlayer.PosX + PLAYER_SIZE / 2;
    INT32 PlayerCenterY = gPlayer.PosY + PLAYER_SIZE / 2;

    for (i = 0; i < MAX_BONUSES; i++) {
      if (!gBonuses[i].IsActive) continue;

      {
        INT32 BonusCenterX = gBonuses[i].TileX * TILE_SIZE + TILE_SIZE / 2;
        INT32 BonusCenterY = gBonuses[i].TileY * TILE_SIZE + TILE_SIZE / 2;
        INT32 dx = PlayerCenterX - BonusCenterX;
        INT32 dy = PlayerCenterY - BonusCenterY;

        if (dx * dx + dy * dy < (TILE_SIZE / 2) * (TILE_SIZE / 2)) {
          PlayerAddBonus(gBonuses[i].Type);
          gBonuses[i].IsActive = FALSE;
          gBonusCount--;
        }
      }
    }
  }

  /* Update bombs */
  BombUpdate();

  /* Update flames */
  FlameUpdate();

  /* Check flame collision with player */
  if (PlayerCheckFlameCollision()) {
    gPlayer.IsAlive = FALSE;
  }

  /* Check enemy collision with player */
  if (PlayerCheckEnemyCollision()) {
    gPlayer.IsAlive = FALSE;
  }
}

VOID
PlayerDraw(VOID)
{
  SPRITE *Sp;

  if (!gPlayer.IsAlive) return;

  Sp = &gSprites[SPRITE_PLAYER];
  if (Sp->Pixels != NULL) {
    GfxDrawSpriteTransparent(Sp, gPlayer.PosX, gPlayer.PosY,
                             PLAYER_SIZE, PLAYER_SIZE, 0xFFFFFFFF);
  } else {
    /* Fallback to primitive drawing */
    GfxDrawRect(gPlayer.PosX, gPlayer.PosY,
                PLAYER_SIZE, PLAYER_SIZE, COLOR_PLAYER1);
    GfxDrawRectOutline(gPlayer.PosX, gPlayer.PosY,
                       PLAYER_SIZE, PLAYER_SIZE, COLOR_WHITE, 2);
    GfxDrawCircle(gPlayer.PosX + PLAYER_SIZE / 3,
                  gPlayer.PosY + PLAYER_SIZE / 3, 6, COLOR_WHITE);
    GfxDrawCircle(gPlayer.PosX + 2 * PLAYER_SIZE / 3,
                  gPlayer.PosY + PLAYER_SIZE / 3, 6, COLOR_WHITE);
    GfxDrawCircle(gPlayer.PosX + PLAYER_SIZE / 3 + 2,
                  gPlayer.PosY + PLAYER_SIZE / 3, 3, COLOR_BLACK);
    GfxDrawCircle(gPlayer.PosX + 2 * PLAYER_SIZE / 3 + 2,
                  gPlayer.PosY + PLAYER_SIZE / 3, 3, COLOR_BLACK);
  }
}

VOID
PlayerAddBonus(INT32 BonusType)
{
  switch (BonusType) {
  case BONUS_BOMB:
    gPlayer.BombCount++;
    gPlayer.Score += 300;
    break;
  case BONUS_FLAME:
    if (gPlayer.FlameRange < MAX_FLAME_RANGE) {
      gPlayer.FlameRange++;
      gPlayerFlameRange = gPlayer.FlameRange;
    }
    gPlayer.Score += 400;
    break;
  case BONUS_SPEED:
    if (gPlayer.Speed < MAX_SPEED) {
      gPlayer.Speed += 5;  /* +0.5 * 10 */
    }
    gPlayer.Score += 300;
    break;
  default:
    break;
  }
}

BOOLEAN
PlayerCheckFlameCollision(VOID)
{
  if (!gPlayer.IsAlive) return FALSE;

  /* Check if any flame tile overlaps with player center */
  {
    INT32 TileX = (gPlayer.PosX + PLAYER_SIZE / 2) / TILE_SIZE;
    INT32 TileY = (gPlayer.PosY + PLAYER_SIZE / 2) / TILE_SIZE;
    return FlameAtTile(TileX, TileY);
  }
}

BOOLEAN
PlayerCheckEnemyCollision(VOID)
{
  if (!gPlayer.IsAlive) return FALSE;
  return EnemyAtPixel(gPlayer.PosX, gPlayer.PosY, PLAYER_SIZE, PLAYER_SIZE);
}

BOOLEAN
PlayerCheckBonusCollision(VOID)
{
  /* This is handled inline in PlayerUpdate */
  return FALSE;
}
