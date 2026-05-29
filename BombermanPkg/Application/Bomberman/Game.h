#ifndef GAME_H
#define GAME_H

#include <Uefi.h>

/* Game states */
typedef enum {
  GAME_STATE_MENU = 0,
  GAME_STATE_PLAYING,
  GAME_STATE_WIN,
  GAME_STATE_LOSE,
  GAME_STATE_TIE
} GAME_STATE_ENUM;

/* Frame timing */
#define TARGET_FPS      60
#define FRAME_DELAY_US  16667  /* microseconds per frame */

/* Game globals */
extern GAME_STATE_ENUM gGameState;
extern UINT32 gFrameCount;

EFI_STATUS GameInit(VOID);
EFI_STATUS GameLoadAssets(VOID);
VOID GameUpdate(VOID);
VOID GameRender(VOID);
BOOLEAN GameIsOver(VOID);
VOID GameShowMenu(VOID);
VOID GameShowResult(VOID);

#endif /* GAME_H */
