// the elite_sprites.c file...
#include"elite_sprites.h"

#include "sprite.h"
 
// MENU_GFX
const static struct QGFX_SPRITEMAP CYC_SpriteMap_0 = {
  64,64,  // width, height in pixels
  0,106,  // x,y screen pos
  COLOR_16 | SQUARE , // attrib 0
  SIZE_64 , // attrib 1
  8192, // OAM value
  0, // spriteID
  4,
  _binary_controls_raw_start, // pointer to graphics 
  _binary_controls_map_start, // pointer to map 
};
 
// RADAR_GFX
const static struct QGFX_SPRITEMAP CYC_SpriteMap_1 = {
  64,64,  // width, height in pixels
  0,106,  // x,y screen pos
  COLOR_16 | SQUARE , // attrib 0
  SIZE_64 , // attrib 1
  8192, // OAM value
  0, // spriteID
  4,
  _binary_radar_raw_start, // pointer to graphics 
  _binary_radar_map_start, // pointer to map 
};
 
// BIG_S_GFX
const static struct QGFX_SPRITEMAP CYC_SpriteMap_2 = {
  16,16,  // width, height in pixels
  181,147,  // x,y screen pos
  COLOR_16 | SQUARE , // attrib 0
  SIZE_16 , // attrib 1
  12288, // OAM value
  4, // spriteID
  1,
  _binary_big_s_raw_start, // pointer to graphics 
  _binary_big_s_map_start, // pointer to map 
};
 
// BIG_E_GFX
const static struct QGFX_SPRITEMAP CYC_SpriteMap_3 = {
  16,16,  // width, height in pixels
  47,108,  // x,y screen pos
  COLOR_16 | SQUARE , // attrib 0
  SIZE_16 , // attrib 1
  12352, // OAM value
  5, // spriteID
  1,
  _binary_big_e_raw_start, // pointer to graphics 
  _binary_big_e_map_start, // pointer to map 
};
 
// BIG_C_GFX
const static struct QGFX_SPRITEMAP CYC_SpriteMap_4 = {
  8,8,  // width, height in pixels
  46,151,  // x,y screen pos
  COLOR_16 | SQUARE , // attrib 0
  SIZE_8 , // attrib 1
  12416, // OAM value
  6, // spriteID
  1,
  _binary_big_e_raw_start, // pointer to graphics 
  _binary_big_e_map_start, // pointer to map 
};
 
const static struct QGFX_SPRITETABLE  QMAP_Cyc_SpriteTable []= {
  {&CYC_SpriteMap_0},
  {&CYC_SpriteMap_1},
  {&CYC_SpriteMap_2},
  {&CYC_SpriteMap_3},
  {&CYC_SpriteMap_4},
};
 
const struct QGFX_ROOT ELITE_SPRITES_ROOT = {
    5, // number of sprites 
    QMAP_Cyc_SpriteTable,
};
// end of file
 
