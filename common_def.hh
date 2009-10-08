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
};

struct Face
{
    VertexIndex v1;
    VertexIndex v2;
    VertexIndex v3;
};
#endif

