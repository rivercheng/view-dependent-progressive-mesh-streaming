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

bool operator<(const FaceAndIndex& fi1, const FaceAndIndex& fi2)
{
    return fi1.index < fi2.index;
}


