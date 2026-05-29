#ifndef GFX_H
#define GFX_H

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>

/* Screen dimensions */
#define SCREEN_WIDTH    960
#define SCREEN_HEIGHT   960

/* Colors (0xAARRGGBB) */
#define COLOR_BLACK     0xFF000000
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_RED       0xFF0000FF
#define COLOR_GREEN     0xFF00FF00
#define COLOR_BLUE      0xFFFF0000
#define COLOR_YELLOW    0xFF00FFFF
#define COLOR_CYAN      0xFFFFFF00
#define COLOR_MAGENTA   0xFFFF00FF
#define COLOR_GRAY      0xFF808080
#define COLOR_DARKGRAY  0xFF404040
#define COLOR_LIGHTGRAY 0xFFC0C0C0
#define COLOR_ORANGE    0xFF0080FF
#define COLOR_PURPLE    0xFF800080
#define COLOR_BROWN     0xFF2A4AA5
#define COLOR_PINK      0xFFFF69B4

/* Game-specific colors */
#define COLOR_WALL_UNBR   0xFF505050   /* unbreakable wall - dark gray */
#define COLOR_WALL_BR     0xFFB8860B   /* breakable wall - dark goldenrod */
#define COLOR_FLOOR       0xFF90EE90   /* walkable floor - light green */
#define COLOR_BG          0xFF228B22   /* background - forest green */
#define COLOR_BOMB        0xFF000000   /* bomb - black */
#define COLOR_FLAME       0xFFFF4500   /* flame - orange-red */
#define COLOR_PLAYER1     0xFF0000FF   /* player1 - red */
#define COLOR_PLAYER2     0xFFFF0000   /* player2 - blue */
#define COLOR_ENEMY       0xFF800080   /* enemy - purple */
#define COLOR_ENEMY_GHOST 0xFFCBC3E3   /* ghost - light purple */
#define COLOR_ENEMY_SMART 0xFFFF6347   /* smart enemy - tomato */
#define COLOR_BONUS_BOMB  0xFFFFD700   /* bonus bomb - gold */
#define COLOR_BONUS_FLAME 0xFFFF6347   /* bonus flame - tomato */
#define COLOR_BONUS_SPEED 0xFF00CED1   /* bonus speed - dark turquoise */

/* Font dimensions */
#define FONT_WIDTH  16
#define FONT_HEIGHT 16

/* Sprite structure */
typedef struct {
  UINT32 *Pixels;   /* BGRA pixel data */
  INT32 Width;
  INT32 Height;
} SPRITE;

/* Named sprite indices */
#define SPRITE_BG            0
#define SPRITE_WALL_UNBR     1
#define SPRITE_FLOOR         2
#define SPRITE_WALL_BR       3
#define SPRITE_BOMB          4
#define SPRITE_FLAME         5
#define SPRITE_PLAYER        6
#define SPRITE_ENEMY_COMMON  7
#define SPRITE_ENEMY_GHOST   8
#define SPRITE_ENEMY_SMART   9
#define SPRITE_BONUS_BOMB    10
#define SPRITE_BONUS_FLAME   11
#define SPRITE_BONUS_SPEED   12
#define SPRITE_COUNT         13

extern UINT32 *gBackBuffer;
extern UINT32 gScreenWidth;
extern UINT32 gScreenHeight;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL *gGOP;

extern SPRITE gSprites[SPRITE_COUNT];

EFI_STATUS GfxInit(VOID);
EFI_STATUS GfxSetMode(UINT32 DesiredWidth, UINT32 DesiredHeight);
VOID GfxPresent(VOID);
VOID GfxClearScreen(UINT32 Color);
VOID GfxDrawRect(INT32 X, INT32 Y, INT32 Width, INT32 Height, UINT32 Color);
VOID GfxDrawRectOutline(INT32 X, INT32 Y, INT32 Width, INT32 Height, UINT32 Color, INT32 Thickness);
VOID GfxDrawCircle(INT32 Cx, INT32 Cy, INT32 Radius, UINT32 Color);
VOID GfxDrawChar(INT32 X, INT32 Y, CHAR8 Char, UINT32 Color);
VOID GfxDrawString(INT32 X, INT32 Y, CONST CHAR8 *Str, UINT32 Color);
VOID GfxDrawNumber(INT32 X, INT32 Y, INT32 Number, UINT32 Color);

EFI_STATUS GfxLoadBmp(CONST CHAR16 *Path, SPRITE *Out);
VOID GfxDrawSprite(CONST SPRITE *Sprite, INT32 X, INT32 Y, INT32 W, INT32 H);
VOID GfxDrawSpriteTransparent(CONST SPRITE *Sprite, INT32 X, INT32 Y, INT32 W, INT32 H, UINT32 TransparentColor);
EFI_STATUS GfxLoadAllSprites(VOID);
VOID GfxFreeSprites(VOID);

#endif /* GFX_H */
