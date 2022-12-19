#ifndef VECTOR_H
#define VECTOR_H

#define FIXEDPOINT 10

#define CODE_IN_IWRAM __attribute__ ((section (".iwram"), long_call))


typedef struct vertex Vertex;
typedef struct vertexfixed VertexFixed;


struct vertex {
  signed short int x, y, z;
};

struct vertexfixed {
  signed long int x, y, z;
};

typedef struct vertex Matrix[3];



signed long int vector_dot_product (Vertex *first, Vertex *second);

void mult_matrix (Vertex *first, Vertex *second) ;
void mult_vector (Vertex *vec, Vertex *mat) ;
void tidy_matrix (Vertex *mat);


void rotate_vec (Vertex *vec, signed short int alpha, signed short int beta);

void UnitLength(Vertex *pVert);
void UnitLengthFixed(VertexFixed *pVert);
void set_unit_matrix(Matrix);
#endif

