#ifndef __COMMON_DEF_HH__
#define __COMMON_DEF_HH__
#include <iostream>
typedef double Coordinate;
typedef unsigned int    VertexIndex;
typedef unsigned int    FaceIndex;
typedef double Normalf;

struct Vertex
{
    //friend std::ostream& operator<<(std::ostream ofs, const Vertex& v);
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

struct FaceAndIndex
{
    FaceIndex index;
    Face      f;
    FaceAndIndex(FaceIndex i, Face f_):index(i), f(f_){;}
};

struct NormalValue
{
    Normalf nxy;
    Normalf nyz;
    Normalf nzx;
    NormalValue(Normalf xy, Normalf yz, Normalf zx): nxy(xy), nyz(yz), nzx(zx){;}
};

std::ostream& operator<<(std::ostream& ofs, const Vertex& v);
std::ostream& operator<<(std::ostream& ofs, const Face& f);
std::ostream& operator<<(std::ostream& ofs, const FaceAndIndex& f);
bool operator<(const FaceAndIndex& fi1, const FaceAndIndex& fi2);
#endif

