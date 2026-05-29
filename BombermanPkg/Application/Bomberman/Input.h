#ifndef INPUT_H
#define INPUT_H

#include <Uefi.h>

/* Direction enum */
typedef enum {
  DIR_UP = 0,
  DIR_LEFT,
  DIR_DOWN,
  DIR_RIGHT,
  DIR_IDLE,
  DIR_BOMB
} DIRECTION;

/* Input state for a player */
typedef struct {
  BOOLEAN Up;
  BOOLEAN Down;
  BOOLEAN Left;
  BOOLEAN Right;
  BOOLEAN Bomb;     /* E key for P1, Space for P2 */
  BOOLEAN Escape;
} INPUT_STATE;

EFI_STATUS InputInit(VOID);
VOID ReadInput(OUT INPUT_STATE *State);

#endif /* INPUT_H */
