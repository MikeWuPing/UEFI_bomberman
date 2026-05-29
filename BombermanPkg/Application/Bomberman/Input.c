#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include "Input.h"

EFI_STATUS
InputInit(VOID)
{
  if (gST->ConIn == NULL) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}

VOID
ReadInput(OUT INPUT_STATE *State)
{
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;

  if (State == NULL || gST->ConIn == NULL) {
    return;
  }

  ZeroMem(State, sizeof(INPUT_STATE));

  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
  while (!EFI_ERROR(Status)) {
    /* Scan codes (arrow keys) */
    if (Key.ScanCode == SCAN_UP) {
      State->Up = TRUE;
    } else if (Key.ScanCode == SCAN_DOWN) {
      State->Down = TRUE;
    } else if (Key.ScanCode == SCAN_LEFT) {
      State->Left = TRUE;
    } else if (Key.ScanCode == SCAN_RIGHT) {
      State->Right = TRUE;
    } else if (Key.ScanCode == SCAN_ESC) {
      State->Escape = TRUE;
    }

    /* Unicode characters (WASD + E) */
    if (Key.UnicodeChar == L'w' || Key.UnicodeChar == L'W') {
      State->Up = TRUE;
    } else if (Key.UnicodeChar == L's' || Key.UnicodeChar == L'S') {
      State->Down = TRUE;
    } else if (Key.UnicodeChar == L'a' || Key.UnicodeChar == L'A') {
      State->Left = TRUE;
    } else if (Key.UnicodeChar == L'd' || Key.UnicodeChar == L'D') {
      State->Right = TRUE;
    } else if (Key.UnicodeChar == L'e' || Key.UnicodeChar == L'E') {
      State->Bomb = TRUE;
    } else if (Key.UnicodeChar == L' ') {
      State->Bomb = TRUE;
    } else if (Key.UnicodeChar == 0x1B) {
      State->Escape = TRUE;
    }

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
  }
}
