// stdlib should include abs, but is broken in devkitadv
//#include <stdlib.h>
unsigned int abs(int);
#include <stdlib.h>
int rand(void);
#include "gbalib_asm.h"
#include "threed.h"
#include "shipdata.h"
#include "gfx.h"
#include "elite.h"
#include "random.h"

void draw_planet(Object *pPlanet);
void draw_explosion (Object *ship);

void RollPitch (signed short int *a, signed short int *b, signed char direction);

// a face is always made of 4 vertexes. This is not ture in the "proper" Elite data
// need a way to convert true Elite data to this type
// each face is draw as 2 triangles:
// 1st triangle x,y coords of vertex 0,1,2 
// 2nd triangle x,y coords of vertex 0,2,3
// so that if a face has more than 4 sides, it will need to be broken down 
// into four sided shapes 
void draw_solid_ship (Object * ship);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this is done and tested, but I think that there's a bug with the planet distance
// to do with fixed points and overflow/
// I reckon the max distance is 0x100000<<8, and the random elite stuff puts this around 0x70000<<8
// though test when flying to the sun
void draw_planet(Object *pPlanet) {
  
  // draw a planet 
  signed long int sx,sy;
  signed long int rx,ry,rz;
  signed long int radius;

  rx = (pPlanet->location.x);
  ry = (pPlanet->location.y);
  rz = (pPlanet->location.z)>>8;
//  if (rz <= 0)  
//    rz = 1;
      
//  x = gba_div((pPlanet->location.x) , pPlanet->location.z);
//  y = gba_div((pPlanet->location.y) , pPlanet->location.z);
	
	  
  // make sure that we aren't going to overflow the point list vals
  if (rz > 0) {
    sx = gba_div((rx<<1) , rz);
    sy = -1*gba_div((ry<<1) , rz);
    sx >>=1;
    sy >>=1;
  }
  else {
    sx = rx;
    sy = -1*ry;
  }
  
  sx += (DownRight.x>>1);
	sy += (DownRight.y>>1);
  
  
  rz = (pPlanet->location.z)>>FIXEDPOINT;
//  if (rz > 0x0FFF) {
//  	rz = 0x0FFF;
//  } else  if (-rz > 0x0FFF) {
//    rz = -0x0FFF;
//  }
//  
//  radius = gba_div(629145, rz);
  if (rz > 0) {
    radius = gba_div(1191450, rz);
  } else {
    radius = 1191450;
  }
  
  
//  gba_sqrt(pPlanet->location.x * pPlanet->location.x + 
//  			pPlanet->location.y * pPlanet->location.y + pPlanet->location.z*pPlanet->location.z));
  //	radius = 6291456 / ship_vec.z;   /* Planets are BIG! */

//  radius = 40;
  if ((sx + radius <  TopLeft.x) ||
  (sx - radius > DownRight.x) ||
  (sy + radius < TopLeft.x) ||
  (sy - radius > DownRight.y)
  || pPlanet->location.z <= 0)
    return; 
  
  
  if (sx > 0x0FFF) {
    sx = 0x0FFF;
  } else  if (-sx > 0x0FFF) {
    sx = -0x0FFF;
  }
  
  if (sy > 0x0FFF) {
    sy = 0x0fff;
  } else  if (-sy > 0x0FFF) {
    sy = -0x0FFF;
  }
  
  if (pPlanet->type == SHIP_PLANET) {
    gba_setcolour(pPlanet->colour);
    if (game_state_flag & SOLID_SHIPS)
      gba_circle_clipped(radius, (short)sx, (short)sy, 1);
    else {
      gba_setcolour(GFX_COL_WHITE);
      gba_circle_clipped(radius, (short)sx, (short)sy, 0);
    }
    
  }
  else {
    gba_setcolour(GFX_COL_SUN_YELLOW);
    gba_wobbly_circle_clipped(radius, (short)sx, (short)sy);
//    gba_circle_clipped(radius, (short)sx, (short)sy, 1);
  }
  
//  gba_setcolour(7);
//  gba_circle(radius, x, y, 0);
//  gba_circle(radius+1, x, y, 0);
//  gba_setcolour(1);
//  gba_drawpixel(x, y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is visible calculates the angle between the normal and the camera
// if it is cos(angle) < 0 then the face isn't visible
int isVisible(struct FACE_DATA * FaceData, VertexFixed * position, Vertex * matrix) {
  Vertex normal;
  normal.x = FaceData->normalx;
  normal.y = FaceData->normaly;
  normal.z = FaceData->normalz;
  
  int normalx = (vector_dot_product(&matrix[0], &normal)/2)>>FIXEDPOINT;
  int normaly = (vector_dot_product(&matrix[1], &normal)/2)>>FIXEDPOINT;
  int normalz = (vector_dot_product(&matrix[2], &normal)/2)>>FIXEDPOINT;
  
  /*int normalx = (vector_dot_product(&matrix[0], &normal)/2)/0x60;
  int normaly = (vector_dot_product(&matrix[1], &normal)/2)/0x60;
  int normalz = (vector_dot_product(&matrix[2], &normal)/2)/0x60;*/
  
  int x = (position->x/256 + normalx)/2;
  int y = (position->y/256 + normaly)/2;
  int z = (position->z/256 + normalz)/2;  
  
  int finalx = (normalx * 2 * x) / 256;
  int finaly = (normaly * 2 * y) / 256;
  int finalz = (normalz * 2 * z) / 256;
  
  int camera =  finalz + finaly + finalx;
  
  if ( camera < 0)
    return 1;
  
  return 0;
    
}


// This is a new drawing routine, a hybrid between that in newkind, Pete's 
// original l33t demo and my own hacks
// ported to C ... 
Vertex point_list[38];

void draw_solid_ship(Object * ship) {
  // A ship has arrays of faces and vertices, stored in ROM
  // a temp copy is made of the verts, which are then transformed and drawn to screen
  
  unsigned short int i,l; // general loop counter
  unsigned short int tmp_index_1, tmp_index_2; // temp value holders for index into vertex list
  signed long int sx,sy; // temp screen x,y storage values
  signed long int rx,ry,rz; // the "rays" formed by the object in world coordinates. why double?
  Vertex vec; // temp vertex, used for rotating the vertices of the object
  
  const struct FACE_TABLE * face_data;
  unsigned short int num_faces; // number of faces, also taken from solid_data
  unsigned short int num_points; // number of points (vertices) 
  Matrix trans_mat;
  signed short int tmp;
  
  struct SHIP_DATA ShipData = *(ELITE_SHIPS.pShipDataTable[ship->type].pShipData);
  
  static unsigned char dot_colour, dot_counter;

  
  num_faces = ShipData.nFaces; // this is also stored in ship_solid, or in faces.length
  face_data = ShipData.pFaceData; // the face data table
  
  
  for (i = 0; i < 3; i++)
		trans_mat[i] = ship->rotmat[i];
  
  tmp = trans_mat[0].y;
  trans_mat[0].y = trans_mat[1].x;
  trans_mat[1].x = tmp;
  
  tmp = trans_mat[0].z;
  trans_mat[0].z = trans_mat[2].x;
  trans_mat[2].x = tmp;
  
  tmp = trans_mat[1].z;
  trans_mat[1].z = trans_mat[2].y;
  trans_mat[2].y = tmp;
  
  // loop pver the number of points in the ships (vertices.length)
  // dynamic array...
  num_points = ShipData.nVerts;
//  Vertex * point_list;
//  point_list = (Vertex*)malloc(num_points * sizeof(Vertex));
  
  // first check that the ship is, more or less, on screen:
  
  rz = ship->location.z/256;
  if (rz == 0)
    rz = 1;
  sx = gba_div(ship->location.x, rz);
  sy = gba_div(ship->location.y, rz);
  
  if (abs(sx) >= 0x100 || abs(sy) > 0xc0 || rz < 0){
    return;
  }

  if ((rz/256) > ShipData.vanish_point) {
    // draw a "dot"
    if (game_state_flag & SOLID_SHIPS) {
      gba_setcolour(face_data[1].pFaceData->nColour);
    } else {
      gba_setcolour(GFX_COL_WHITE); 
    }
    
    sy = sy*-1;
    sx += (DownRight.x>>1);
    sy += (DownRight.y>>1);
    
    if (sx < DownRight.x && sx > TopLeft.x && 
        sy < DownRight.y && sy > TopLeft.y)
    {
        	
        // Twinkle
        if (dot_counter++ >= 2) {
        if (!(rand()%3))
            switch (rand()%7) {
                case 0:
                case 1:
                case 2:
                    dot_colour = GFX_COL_DARK_GREY;
                    break;
                case 3:
                case 4:
                case 5:
                    dot_colour = GFX_COL_GREY;
                    break;
                case 6:
                    dot_colour = GFX_COL_WHITE;
                    break;
            }
        else
            dot_colour = 0;
        dot_counter = 0;
        }
        
        if (dot_colour)
            gba_setcolour(dot_colour);

        // Plot dot/cross
        if (game_state_flag & DISTANT_DOT)
            gfx_draw_dot(sx, sy);
        else
            gfx_draw_spot(sx, sy);
    }
    // need to set the laser vertex in case the spot fires at us:
    point_list[0].x = sx;
    point_list[0].y = sy;
    point_list[0].z = ShipData.vanish_point<<8;
    
  } else {
    // the ship is within range to see it properly
  for (i = 0; i < num_points; i++)  {
    // "vec" is a local struct vector, temporary value holder.
    
    vec.x = ShipData.pVertData[i].x;
    vec.y = ShipData.pVertData[i].y;
    vec.z = ShipData.pVertData[i].z;
    
    mult_vector (&vec, trans_mat);
    
    // shift it by the location of the object in the universe..
    rx = vec.x + ((ship->location.x)>>8);
    ry = vec.y + ((ship->location.y)>>8);
    rz = vec.z + ((ship->location.z)>>8);
    // to avoid div by 0.. could possibly cull this face at this point as it is behind us?
    if (rz <= 0)  
      rz = 1;
    // the screen x and y, "perspective projection"
      sx = gba_div((rx<<8) , rz);
      
      // invert the y (for monitor x,y coords:
      sy = -1*gba_div((ry<<8) , rz);    /*  --------> x increasing
                                                    |
                                                    |  Y increasing
                                                    V */                              
    // shift the coords to by some value, so that they aren't in the top left of the screen all the time
    sx += (DownRight.x>>1);
		sy += (DownRight.y>>1);
    rz = vec.z + ((ship->location.z)>>8);
    
    // make sure that we aren't going to overflow the point list vals
    if (sx > 0x0FFF) {
      sx = 0x0FFF;
    } else  if (-sx > 0x0FFF) {
      sx = -0x0FFF;
    }
    
    if (sy > 0x0FFF) {
      sy = 0x0fff;
    } else  if (-sy > 0x0FFF) {
      sy = -0x0FFF;
    }
    
    if (rz > 0x0FFF) {
    	rz = 0x0FFF;
    } else  if (-rz > 0x0FFF) {
      rz = -0x0FFF;
    }
    
    // add to our 3D point list. 
    point_list[i].x = sx;
    point_list[i].y = sy;
    point_list[i].z = rz;//vec.z + ((ship->off.z)>>8);
    
  }

  // for each Face of the object. 

  Vertex vTriangle[3];
  
  for (i = 0; i < num_faces; i++)  {
    // back face culling
    // each face has 3 or 4 points, but only 3 are needed to define the plane.
    // in the orginal Elite, this was done by using the dot product of the normal to the 
    // face and the view vector. Here it is done another way, though I reckon 
    // they are the same in the end. This method requires that the vertices are defined in clockwise order
    // I had to go through the elite data and change the order in places. Ugh.
    // point_list is identical to my vertices[] array.
    // face_data is the same as faces[], except if I do the GBA conversion, 
    // they always have 4 points in every face (even triangles! they repeat the last point)
    struct FACE_DATA FaceData = *(face_data[i].pFaceData);

    if (isVisible(&FaceData, &ship->location, trans_mat))
    {
      // at this stage, the point_list already contains the screen coordinates for x and y.
      // all that's left to do is to count how many points this face has and add those
      // to the poly_list which will be rendered.
      
      if (game_state_flag & SOLID_SHIPS) {
        // draw the face from triangles
        gba_setcolour(FaceData.nColour);
        tmp_index_1 = FaceData.nVertCount-2;
        for (l = 0; l < tmp_index_1; l++) {
          vTriangle[0].x = point_list[FaceData.pVertIndex[0]].x;
          vTriangle[0].y = point_list[FaceData.pVertIndex[0]].y;
          vTriangle[0].z = point_list[FaceData.pVertIndex[0]].z;
          tmp_index_2 = FaceData.pVertIndex[l+1];
          vTriangle[1].x = point_list[tmp_index_2].x;
          vTriangle[1].y = point_list[tmp_index_2].y;
          vTriangle[1].z = point_list[tmp_index_2].z;
          tmp_index_2 = FaceData.pVertIndex[l+2];
          vTriangle[2].x = point_list[tmp_index_2].x;
          vTriangle[2].y = point_list[tmp_index_2].y;
          vTriangle[2].z = point_list[tmp_index_2].z;      
          gba_triangle_clip(vTriangle);
        }
      } else {
        // draw the face from lines
        gba_setcolour(GFX_COL_WHITE); 
        tmp_index_1 = FaceData.nVertCount-1;
        gfx_line_clip ( point_list[FaceData.pVertIndex[0]].x,
                        point_list[FaceData.pVertIndex[0]].y, 
                        point_list[FaceData.pVertIndex[0]].z,
                        
                        point_list[FaceData.pVertIndex[tmp_index_1]].x,
                        point_list[FaceData.pVertIndex[tmp_index_1]].y, 
                        point_list[FaceData.pVertIndex[tmp_index_1]].z);
        for (l = 0; l < tmp_index_1; l++) {
          tmp_index_2 = FaceData.pVertIndex[l+1];
          gfx_line_clip ( point_list[FaceData.pVertIndex[l]].x,
            point_list[FaceData.pVertIndex[l]].y, 
            point_list[FaceData.pVertIndex[l]].z,
            
            point_list[tmp_index_2].x,
            point_list[tmp_index_2].y, 
            point_list[tmp_index_2].z);
        }
      }
    } 
    
  }
  } // end rz > vanish_point
  if (ship->flags & FLG_FIRING)
  {
//  	lasv = ship_list[ship->type]->front_laser;
  	unsigned char lasv = 0;
  	gba_setcolour((ship->type == SHIP_VIPER && (game_state_flag & SOLID_SHIPS)) ?GFX_COL_LIGHT_BLUE : GFX_COL_WHITE);
  	gfx_line_clip (point_list[lasv].x, point_list[lasv].y, point_list[lasv].z,
          ship->location.x > 0 ? TopLeft.x : DownRight.x, GetRand(DownRight.y),ZMin);

  }
  

}
//////////////////////////////////////////////////////////

void draw_explosion (Object *ship)
{
  unsigned short int i,j; // general loop counters
  signed short int q; // a factor
  
  signed long int sx,sy; // temp screen x,y storage values
  signed long int rx,ry,rz; // the "rays" formed by the object in world coordinates. why double?
  Vertex vec; // temp vertex, used for rotating the vertices of the object
  
  const struct FACE_TABLE * face_data;
  unsigned short int num_faces; // number of faces, also taken from solid_data
  unsigned short int num_points; // number of points (vertices) 
  Matrix trans_mat;
  signed short int tmp;
//    Vertex camera_vec = location.copy(); // the location of this object in the world. We start off looking at it.
//    camera_vec.z = -camera_vec.z;   // probably should do something else here...
  num_faces = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->nFaces; // this is also stored in ship_solid, or in faces.length
  face_data = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->pFaceData; // the face data table
  
  unsigned long int old_seed;
	
	
	if (ship->exp_delta > 251)
	{
		ship->flags |= FLG_REMOVE;
		return;
	}
	
	ship->exp_delta += 4;
  
	if (ship->location.z <= 0)
		return;
  if (ship->exp_delta < 0x30)
    gba_setcolour(GFX_COL_WHITE);  
  else if (ship->exp_delta < 0x60)
    gba_setcolour(GFX_COL_SUN_YELLOW);
  else if (ship->exp_delta < 0x90)
    gba_setcolour(GFX_COL_ORANGE);
  else
    gba_setcolour(GFX_COL_DARK_GREY);
  
   num_faces = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->nFaces; // this is also stored in ship_solid, or in faces.length
  face_data = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->pFaceData; // the face data table
  
  
  for (i = 0; i < 3; i++)
		trans_mat[i] = ship->rotmat[i];
  
  tmp = trans_mat[0].y;
  trans_mat[0].y = trans_mat[1].x;
  trans_mat[1].x = tmp;
  
  tmp = trans_mat[0].z;
  trans_mat[0].z = trans_mat[2].x;
  trans_mat[2].x = tmp;
  
  tmp = trans_mat[1].z;
  trans_mat[1].z = trans_mat[2].y;
  trans_mat[2].y = tmp;
  
  // improve this so it works like the 3D ship code
  
  // loop pver the number of points in the ships (vertices.length)
  // dynamic array...
  num_points = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->nVerts;
  
  for (i = 0; i < num_points; i++)  {
    // "vec" is a local struct vector, temporary value holder.
    
    vec.x = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->pVertData[i].x;
    vec.y = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->pVertData[i].y;
    vec.z = ELITE_SHIPS.pShipDataTable[ship->type].pShipData->pVertData[i].z;
    
    mult_vector (&vec, trans_mat);
    
    // shift it by the location of the object in the universe..
    rx = vec.x + ((ship->location.x)>>8);
    ry = vec.y + ((ship->location.y)>>8);
    rz = vec.z + ((ship->location.z)>>8);
    // to avoid div by 0.. could possibly cull this face at this point as it is behind us?
    if (rz <= 0)  
      rz = 1;
    // the screen x and y, "perspective projection"
      sx = gba_div((rx<<8) , rz);
      
      // invert the y (for monitor x,y coords:
      sy = -1*gba_div((ry<<8) , rz);    /*  --------> x increasing
                                                    |
                                                    |  Y increasing
                                                    V */                              
    // shift the coords to by some value, so that they aren't in the top left of the screen all the time
    sx += (DownRight.x>>1);
		sy += (DownRight.y>>1);
    rz = vec.z + ((ship->location.z)>>8);
    
    // make sure that we aren't going to overflow the point list vals
    if (sx > 0x0FFF) {
      sx = 0x0FFF;
    } else  if (-sx > 0x0FFF) {
      sx = -0x0FFF;
    }
    
    if (sy > 0x0FFF) {
      sy = 0x0fff;
    } else  if (-sy > 0x0FFF) {
      sy = -0x0FFF;
    }
    
    if (rz > 0x0FFF) {
    	rz = 0x0FFF;
    } else  if (-rz > 0x0FFF) {
      rz = -0x0FFF;
    }
    
    // add to our 3D point list. 
    point_list[i].x = sx;
    point_list[i].y = sy;
    point_list[i].z = rz;
    
  }
  
  
  rz = ship->location.z>>8;
  if (rz == 0)
    rz = 1;
  if (rz >= 0x2000)
  	q = 254;
  else
  	q = (rz /32) | 1;
  
  rz = gba_div((ship->exp_delta * 256) , q);
    
  q = rz / 32;	
  	
  old_seed = GetRandSeed();
  SeedRand(ship->exp_seed);

  signed long nz;
  //unsigned char sizex, sizey,psx,psy;
  // for each Face of the object. 
//  unsigned short nCount = 0;
  for (i = 0; i < num_faces; i++)  {
    // back face culling
    nz = (point_list[face_data[i].pFaceData->pVertIndex[0]].x - point_list[face_data[i].pFaceData->pVertIndex[1]].x) *
      (point_list[face_data[i].pFaceData->pVertIndex[2]].y - point_list[face_data[i].pFaceData->pVertIndex[1]].y);
    nz -= (point_list[face_data[i].pFaceData->pVertIndex[0]].y - point_list[face_data[i].pFaceData->pVertIndex[1]].y) *
      (point_list[face_data[i].pFaceData->pVertIndex[2]].x - point_list[face_data[i].pFaceData->pVertIndex[1]].x);
    if (nz < 0)
    {
      // at this stage, the point_list already contains the screen coordinates for x and y.
      
      if (point_list[face_data[i].pFaceData->pVertIndex[0]].z > 0) {
        for (j = 0; j < 16; j++) {
          rx = rand255() - 128;
          ry = rand255() - 128;		
          
          rx = (rx * q) /256;
          ry = (ry * q) /256;
          
          rx = rx + rx + point_list[face_data[i].pFaceData->pVertIndex[0]].x;
          ry = ry + ry + point_list[face_data[i].pFaceData->pVertIndex[0]].y;
                        
          if (rx > TopLeft.x && rx < DownRight.x &&
          ry > TopLeft.y && ry < DownRight.y)
            gba_drawpixel (rx, ry);
        }
      }
    		
      
    } 
    
  }

  SeedRand (old_seed);
}

///////////////////////////////////////////

void draw_ship (Object *ship)
{
	if (ship->type == SHIP_PLANET ||  ship->type == SHIP_SUN)
	{
		draw_planet (ship);
		return;
	}
	
	if ((ship->flags & FLG_DEAD) && !(ship->flags & FLG_EXPLOSION))
	{
		ship->flags |= FLG_EXPLOSION;
		ship->exp_seed = GetRand(65535);
		ship->exp_delta = 18; 
	}

	if (ship->flags & FLG_EXPLOSION)
	{
		draw_explosion (ship);
		return;
	}
	
	
	draw_solid_ship (ship);
	
}


