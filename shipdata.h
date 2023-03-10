// Elite ship data structure. Generated by ShipReader.java

#define SHIP_SUN				    -2
#define SHIP_PLANET			    	-1
#define SHIP_MISSILE		      1
#define SHIP_CORIOLIS		      2
#define SHIP_ESCAPE_CAPSULE	  3
#define SHIP_ALLOY				    4
#define SHIP_CARGO				    5
#define SHIP_BOULDER			    6
#define SHIP_ASTEROID			    7
#define SHIP_ROCK				      8
#define SHIP_SHUTTLE			    9
#define SHIP_TRANSPORTER	    10
#define SHIP_COBRA3				    11
#define SHIP_PYTHON				    12
#define SHIP_BOA				      13
#define SHIP_ANACONDA		      14
#define SHIP_HERMIT			      15
#define SHIP_VIPER			      16
#define SHIP_BUSHMASTER	      17
#define SHIP_SIDEWINDER	      18
#define SHIP_MAMBA			      19
#define SHIP_KRAIT			      20
#define SHIP_ADDER			      21
#define SHIP_GECKO			      22
#define SHIP_COBRA1			      23
#define SHIP_WORM				      24
#define SHIP_COBRA3_LONE      25
#define SHIP_ASP2				      26
#define SHIP_PYTHON_LONE		  27
#define SHIP_FER_DE_LANCE		  28
#define SHIP_MORAY				    29
#define SHIP_THARGOID			    30
#define SHIP_THARGLET			    31
#define SHIP_CONSTRICTOR		  32
#define SHIP_COUGAR				    33
#define SHIP_DODEC				    34
      
#define SHIP_BUG				      35
#define SHIP_CHAMELEON	      36
#define SHIP_SHUTTLEV		      37
#define SHIP_SALAMANDER       38

struct FACE_DATA
{
  const unsigned char* const pVertIndex; // pointer to the array of 4 vertices indices
  const unsigned char nVertCount; // number of verts in this face
  const unsigned char nColour;     // the face colour
  const signed short normalx;
  const signed short normaly;
  const signed short normalz;
};

struct VERT_DATA
{
  const signed short int x;
  const signed short int y;
  const signed short int z;
};

struct FACE_TABLE
{
  const struct FACE_DATA *pFaceData;
};

struct SHIP_DATA
{
    const char* cShipName;
    const unsigned char max_loot;
    const unsigned char scoop_type;
    const unsigned short size;
    const unsigned char front_laser;
    const unsigned short bounty;
    const unsigned char vanish_point;
    const unsigned short energy;
    const unsigned char velocity;
    const unsigned char missiles;
    const unsigned char laser_strength;
    const unsigned char nFaces;  // number of faces
    const unsigned char nVerts;  // number of vertices
    const struct FACE_TABLE *pFaceData; // pointer to the face data table
    const struct VERT_DATA *pVertData; // pointer to the vertices data
};

struct SHIP_TABLE 
{
  const struct SHIP_DATA* pShipData;
};

struct RGB_VALS
{
  const unsigned char R;
  const unsigned char G;
  const unsigned char B;
};

struct SHIP_DATA_ALL 
{
  const unsigned char nShipCount; // number of ships 
  const struct SHIP_TABLE *pShipDataTable;
  const struct RGB_VALS ** const pPalette;  // 0 is default, 1 is GBA gamma corr palette
};
  
extern const struct SHIP_DATA_ALL ELITE_SHIPS;


