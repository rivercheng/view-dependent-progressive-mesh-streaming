// =============================================================================
// Written by Cheng Wei
// rivercheng@gmail.com
// 8 Oct 2009
// =============================================================================
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
    Face(void){;}
    Face(VertexIndex v1_, VertexIndex v2_, VertexIndex v3_):v1(v1_), v2(v2_), v3(v3_){;}
};

struct NormalValue
{
    Normalf nxy;
    Normalf nyz;
    Normalf nzx;
    NormalValue(Normalf xy, Normalf yz, Normalf zx): nxy(xy), nyz(yz), nzx(zx){}
};
        
struct SplitInfo
{
     Coordinate x;
     Coordinate y;
     Coordinate z;
     VertexIndex v1;
     VertexIndex vl;
     VertexIndex vr;
     SplitInfo(Coordinate x_, Coordinate y_, Coordinate z_,\
               VertexIndex v1_, VertexIndex vl_, VertexIndex vr_)
         :x(x_), y(y_), z(z_), v1(v1_), vl(vl_), vr(vr_){}
};

std::ostream& operator<<(std::ostream& ofs, const Vertex& v);
std::ostream& operator<<(std::ostream& ofs, const Face& f);
std::ostream& operator<<(std::ostream& ofs, const std::pair<FaceIndex, Face>& f);
#endif

