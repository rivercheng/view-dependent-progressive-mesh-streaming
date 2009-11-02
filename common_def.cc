// =============================================================================
// Written by Cheng Wei
// rivercheng@gmail.com
// 8 Oct 2009
// =============================================================================
#include "common_def.hh"

std::ostream& operator<<(std::ostream& ofs, const Vertex& v)
{
    return ofs<<v.x<<" "<<v.y<<" "<<v.z;
}

std::ostream& operator<<(std::ostream& ofs, const Face& f)
{
    return ofs<<f.v1<<" "<<f.v2<<" "<<f.v3;
}

std::ostream& operator<<(std::ostream& ofs, const std::pair<FaceIndex, Face>& fi)
{
    return ofs<<fi.first<<" "<<fi.second;
}



