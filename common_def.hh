#ifndef __COMMON_DEF_HH__
#define __COMMON_DEF_HH__
typedef double Coordinate;
typedef int    VertexIndex;
typedef int    FaceIndex;

struct Vertex
{
    Coordinate x;
    Coordinate y;
    Coordinate z;
    Vertex(Coordinate x_, Coordinate y_, Coordinate z_):x(x_), y(y_), z(z_){;}
};

struct Face
{
    VertexIndex v1;
    VertexIndex v2;
    VertexIndex v3;
    Face(VertexIndex v1_, VertexIndex v2_, VertexIndex v3_):v1(v1_), v2(v2_), v3(v3_){;}
};
#endif

