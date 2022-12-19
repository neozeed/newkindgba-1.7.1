// elite_sprites.h header file...
 
#define MENU_GFX 0
#define RADAR_GFX 1
#define BIG_S_GFX 2
#define BIG_E_GFX 3
#define BIG_C_GFX 4
#define END_GFX 12432
 
extern unsigned char _binary_controls_raw_start[];
extern unsigned char _binary_controls_map_start[];
extern unsigned char _binary_radar_raw_start[];
extern unsigned char _binary_radar_map_start[];
extern unsigned char _binary_big_s_raw_start[];
extern unsigned char _binary_big_s_map_start[];
extern unsigned char _binary_big_e_raw_start[];
extern unsigned char _binary_big_e_map_start[];
 
struct QGFX_SPRITEMAP
{
    const unsigned char nWidth;  // width in pixels
    const unsigned char nHeight;  // height in pixels
    const unsigned char nX;  // screen pos x
    const unsigned char nY;  // screen pos y
    const unsigned short nAttribute0;  // flags to be used for attrib 0 (COLOR_256 | SQUARE | ROTATION_FLAG)
    const unsigned short nAttribute1;  // flags to be used for attrib 1 (SIZE_32 |ROTDATA(1))
    const unsigned short nOAM; // the OAM value to store the gfx
    const unsigned short nSpriteID;
    const unsigned short nSpriteCount; // number of HW sprites to use
    const unsigned char *pGFX;  // pointer to the graphics array (extern u8 _binary_GFX_raw_start[];)
    const unsigned char *pGFXMap; // pointer to the map file that makes up the graphics (extern u8 _binary_GFX_map_start[];)
};
 
struct QGFX_SPRITETABLE 
{
  const struct QGFX_SPRITEMAP *pSpriteMap;
};
 
struct QGFX_ROOT
{
    const unsigned char nGFXs; // number of peices of gfx data
    const struct QGFX_SPRITETABLE *pSpriteTable;
};
 
extern const struct QGFX_ROOT ELITE_SPRITES_ROOT;
 
// end of file
 
