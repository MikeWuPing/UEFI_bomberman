#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "Game.h"
#include "Gfx.h"
#include "Input.h"
#include "Map.h"
#include "Player.h"
#include "Bomb.h"
#include "Enemy.h"

GAME_STATE_ENUM gGameState = GAME_STATE_MENU;
UINT32 gFrameCount = 0;

EFI_STATUS
GameInit(VOID)
{
  gFrameCount = 0;
  gGameState = GAME_STATE_PLAYING;

  MapInit();
  BombInit();
  PlayerInit();
  EnemyInit();

  return EFI_SUCCESS;
}

/* Load all sprite assets (call once at startup) */
EFI_STATUS
GameLoadAssets(VOID)
{
  return GfxLoadAllSprites();
}

VOID
GameUpdate(VOID)
{
  INPUT_STATE Input;

  if (gGameState != GAME_STATE_PLAYING) {
    return;
  }

  /* Read input */
  ReadInput(&Input);

  /* ESC to quit */
  if (Input.Escape) {
    gGameState = GAME_STATE_LOSE;  /* Treat as game over */
    return;
  }

  /* Update player (also updates bombs, flames, collisions) */
  PlayerUpdate(&Input);

  /* Update enemies */
  EnemyUpdate();

  /* Update bombs and flames (already done in PlayerUpdate, but ensure) */
  BombUpdate();
  FlameUpdate();

  /* Check enemy-flame collisions */
  {
    INT32 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
      if (!gEnemies[i].IsActive) continue;

      {
        INT32 TileX = (gEnemies[i].PosX + PLAYER_SIZE / 2) / TILE_SIZE;
        INT32 TileY = (gEnemies[i].PosY + PLAYER_SIZE / 2) / TILE_SIZE;

        if (FlameAtTile(TileX, TileY)) {
          EnemyKill(i);
          gPlayer.Score += 100;
        }
      }
    }
  }

  /* Check game over conditions */
  if (GameIsOver()) {
    /* Give a brief delay before showing result */
    gBS->Stall(500000); /* 0.5 sec */
  }

  gFrameCount++;
}

VOID
GameRender(VOID)
{
  /* Clear screen */
  GfxClearScreen(COLOR_BG);

  /* Draw map tiles */
  MapDraw();

  /* Draw bonuses */
  MapDrawBonuses();

  /* Draw bombs */
  BombDraw();

  /* Draw flames */
  FlameDraw();

  /* Draw enemies */
  EnemyDraw();

  /* Draw player */
  PlayerDraw();

  /* Draw UI */
  {
    INT32 uiY = MAP_SIZE * TILE_SIZE + 4;

    /* Score */
    GfxDrawString(10, uiY, "SCORE:", COLOR_WHITE);
    GfxDrawNumber(60, uiY, gPlayer.Score, COLOR_YELLOW);

    /* Bombs */
    GfxDrawString(160, uiY, "BOMBS:", COLOR_WHITE);
    GfxDrawNumber(220, uiY, gPlayer.BombCount, COLOR_YELLOW);

    /* Flame range */
    GfxDrawString(300, uiY, "RANGE:", COLOR_WHITE);
    GfxDrawNumber(360, uiY, gPlayer.FlameRange, COLOR_YELLOW);

    /* Speed */
    GfxDrawString(440, uiY, "SPEED:", COLOR_WHITE);
    GfxDrawNumber(500, uiY, gPlayer.Speed / 10, COLOR_YELLOW);

    /* Enemies remaining */
    GfxDrawString(580, uiY, "ENEMIES:", COLOR_WHITE);
    GfxDrawNumber(660, uiY, gEnemyCount, COLOR_RED);
  }

  /* Present to screen */
  GfxPresent();
}

BOOLEAN
GameIsOver(VOID)
{
  /* Player died */
  if (!gPlayer.IsAlive) {
    gGameState = GAME_STATE_LOSE;
    return TRUE;
  }

  /* All enemies dead */
  {
    INT32 i;
    BOOLEAN anyAlive = FALSE;
    for (i = 0; i < MAX_ENEMIES; i++) {
      if (gEnemies[i].IsActive) {
        anyAlive = TRUE;
        break;
      }
    }
    if (!anyAlive) {
      gGameState = GAME_STATE_WIN;
      return TRUE;
    }
  }

  return FALSE;
}

VOID
GameShowMenu(VOID)
{
  GfxClearScreen(COLOR_BLACK);

  GfxDrawString(350, 200, "BOMBERMAN", COLOR_WHITE);
  GfxDrawString(300, 300, "UEFI Shell Edition", COLOR_LIGHTGRAY);

  GfxDrawString(300, 400, "WASD - Move", COLOR_GREEN);
  GfxDrawString(300, 420, "E    - Place Bomb", COLOR_GREEN);
  GfxDrawString(300, 440, "ESC  - Quit", COLOR_GREEN);

  GfxDrawString(300, 520, "Press any key to start...", COLOR_YELLOW);

  GfxPresent();

  /* Wait for key press */
  while (1) {
    INPUT_STATE Input;
    ReadInput(&Input);
    if (Input.Up || Input.Down || Input.Left || Input.Right ||
        Input.Bomb || Input.Escape) {
      if (Input.Escape) {
        gGameState = GAME_STATE_LOSE;
        return;
      }
      break;
    }
    gBS->Stall(50000); /* 50ms */
  }
}

VOID
GameShowResult(VOID)
{
  GfxClearScreen(COLOR_BLACK);

  switch (gGameState) {
  case GAME_STATE_WIN:
    GfxDrawString(350, 300, "YOU WIN!", COLOR_GREEN);
    break;
  case GAME_STATE_LOSE:
    GfxDrawString(350, 300, "GAME OVER", COLOR_RED);
    break;
  case GAME_STATE_TIE:
    GfxDrawString(350, 300, "TIE!", COLOR_YELLOW);
    break;
  default:
    break;
  }

  GfxDrawString(300, 400, "SCORE:", COLOR_WHITE);
  GfxDrawNumber(380, 400, gPlayer.Score, COLOR_YELLOW);

  GfxDrawString(300, 480, "Press ESC to exit", COLOR_LIGHTGRAY);
  GfxDrawString(300, 510, "Press SPACE to play again", COLOR_LIGHTGRAY);

  GfxPresent();

  /* Wait for key */
  while (1) {
    INPUT_STATE Input;
    ReadInput(&Input);
    if (Input.Escape) {
      return;
    }
    if (Input.Bomb) {
      GameInit();
      return;
    }
    gBS->Stall(50000);
  }
}
