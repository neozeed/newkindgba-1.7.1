// stdlib should include abs, but is broken in devkitadv
#include <stdlib.h>
//unsigned int abs(int);
#include "gbalib_asm.h"
#include "vector.h"



signed long int vector_dot_product (Vertex *first, Vertex *second)
{
	return ((first->x * second->x) + (first->y * second->y) + (first->z * second->z));	
}

void mult_matrix (Vertex *first, Vertex *second)
{
	unsigned char i;
	Matrix rv;

	for (i = 0; i < 3; i++)	{

      rv[i].x =	(first[0].x * second[i].x) +
                (first[1].x * second[i].y) +
                (first[2].x * second[i].z);
      
      rv[i].y =	(first[0].y * second[i].x) +
                (first[1].y * second[i].y) +
                (first[2].y * second[i].z);
      
      rv[i].z =	(first[0].z * second[i].x) +
                (first[1].z * second[i].y) +
                (first[2].z * second[i].z);
	}

	for (i = 0; i < 3; i++)
		first[i] = rv[i];
}




void mult_vector (Vertex *vec, Vertex *mat) {
	signed short int x;
	signed short int y;
	signed short int z;

	x = ((vec->x * mat[0].x) +
		(vec->y * mat[0].y) +
		(vec->z * mat[0].z))>>FIXEDPOINT;

	y = ((vec->x * mat[1].x) +
		(vec->y * mat[1].y) +
		(vec->z * mat[1].z))>>FIXEDPOINT;

	z = ((vec->x * mat[2].x) +
		(vec->y * mat[2].y) +
		(vec->z * mat[2].z))>>FIXEDPOINT;

	vec->x = x;
	vec->y = y;
	vec->z = z;
}

void tidy_matrix (Vertex *mat)
{
	UnitLength (&mat[2]);

	if ((mat[2].x > -256) && (mat[2].x < 256))
	{
		if ((mat[2].y > -256) && (mat[2].y < 256))
		{
		  if (mat[2].z != 0)
			mat[1].z = -gba_div((mat[2].x * mat[1].x + mat[2].y * mat[1].y) , mat[2].z);
	      else
	        mat[1].z = -(mat[2].x * mat[1].x + mat[2].y * mat[1].y);
		}
		else
		{
		  if (mat[2].y != 0)
			mat[1].y = -gba_div((mat[2].x * mat[1].x + mat[2].z * mat[1].z) , mat[2].y);
	      else
	        mat[1].y = -(mat[2].x * mat[1].x + mat[2].z * mat[1].z);
		}
	}
	else
	{
	  if (mat[2].x != 0)
		mat[1].x = -gba_div((mat[2].y * mat[1].y + mat[2].z * mat[1].z) , mat[2].x);
      else
        mat[1].x = -(mat[2].y * mat[1].y + mat[2].z * mat[1].z);
	}
	
	UnitLength (&mat[1]);
	

	/* xyzzy... nothing happens. :-)*/
	
	mat[0].x = (mat[1].y * mat[2].z - mat[1].z * mat[2].y)>>FIXEDPOINT;
	mat[0].y = (mat[1].z * mat[2].x - mat[1].x * mat[2].z)>>FIXEDPOINT;
	mat[0].z = (mat[1].x * mat[2].y - mat[1].y * mat[2].x)>>FIXEDPOINT;
}


void rotate_vec (Vertex *vec, signed short int alpha, signed short int beta) {
	signed long int x,y,z;
	
	x = vec->x;
	y = vec->y;
	z = vec->z;

	y = (y<<8) - (alpha * x);
	x = (x<<8) + ((alpha * y)>>8);
	y = y - (beta * z);
	z = (z<<8)   + ((beta * y)>>8);
    
	vec->x = (x+0x7f)>>8;
	vec->y = (y+0x7f)>>8;
	vec->z = (z+0x7f)>>8;	
}


void UnitLength(Vertex *pVert) {
  // set a vertex to unit length, assume othe end goes through 0,0,0 to make a vector
  
  signed long lx,ly,lz;
  signed long uni;
  
  lx = pVert->x;
  ly = pVert->y;
  lz = pVert->z;
  
  uni = gba_sqrt(lx * lx + ly * ly + lz * lz);
  if (uni == 0)
    uni = 1;
  pVert->x = gba_div((lx<<FIXEDPOINT) , uni);
  pVert->y = gba_div((ly<<FIXEDPOINT) , uni);
  pVert->z = gba_div((lz<<FIXEDPOINT) , uni);
  
}

void UnitLengthFixed(VertexFixed *pVert) {
  // set a vertex to unit length, assume othe end goes through 0,0,0 to make a vector
  // used for unit lengthing distance vectors, which are right shifted by 8
  signed long lx,ly,lz;
  signed long uni;
  
  lx = pVert->x;
  ly = pVert->y;
  lz = pVert->z;
  
  uni = gba_sqrt((lx>>8) * (lx>>8) + (ly>>8) * (ly>>8) + (lz>>8) * (lz>>8));
  if (uni == 0)
    uni = 1;
  pVert->x = gba_div((lx<<2) , uni);
  pVert->y = gba_div((ly<<2) , uni);
  pVert->z = gba_div((lz<<2) , uni);
  
}

static const struct vertex unit_matrix[3] = {
#if 1
  {(0x1)<<FIXEDPOINT, 0, 0},
  {0, 0x1<<FIXEDPOINT, 0},
  {0, 0, -(0x1<<FIXEDPOINT)}, 
#else
  {0x6000, 0, 0},
  {0, 0x6000, 0},
  {0, 0, -(0x6000)}, 
#endif
};

void set_unit_matrix(Matrix rotmat) 
{
  int i;
  for ( i = 0; i < 3; i++) {
    rotmat[i].x = unit_matrix[i].x;
    rotmat[i].y = unit_matrix[i].y;
    rotmat[i].z = unit_matrix[i].z;
  }
}
