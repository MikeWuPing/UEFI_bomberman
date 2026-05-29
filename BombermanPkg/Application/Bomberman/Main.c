#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "Game.h"
#include "Gfx.h"
#include "Input.h"
#include "Map.h"
#include "Bomb.h"
#include "Player.h"
#include "Enemy.h"

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS Status;

  Print(L"Bomberman UEFI Shell Game\n");
  Print(L"Initializing...\n");

  /* Initialize graphics */
  Status = GfxInit();
  if (EFI_ERROR(Status)) {
    Print(L"Graphics init failed: %r\n", Status);
    return Status;
  }

  /* Initialize input */
  Status = InputInit();
  if (EFI_ERROR(Status)) {
    Print(L"Input init failed: %r\n", Status);
    return Status;
  }

  /* Load sprite assets from filesystem */
  Status = GameLoadAssets();
  if (EFI_ERROR(Status)) {
    Print(L"[WARN] Some sprites failed to load, using fallbacks\n");
  }

  Print(L"Initialization complete. Starting game...\n");

  /* Main game loop */
  while (1) {
    /* Show menu */
    GameShowMenu();

    /* If ESC was pressed at menu, exit */
    if (gGameState == GAME_STATE_LOSE) {
      break;
    }

    /* Initialize and run game */
    GameInit();

    while (gGameState == GAME_STATE_PLAYING) {
      GameUpdate();
      GameRender();
      gBS->Stall(FRAME_DELAY_US);
    }

    /* Show result */
    GameShowResult();

    /* If ESC pressed at result screen, exit */
    if (gGameState == GAME_STATE_LOSE) {
      /* Check if user pressed ESC at result */
      break;
    }
  }

  GfxFreeSprites();
  Print(L"Game exited.\n");
  return EFI_SUCCESS;
}
