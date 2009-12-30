#include "measure.hh"
Center auto_center(size_t count, const Coordinate *vertex_array)
{
    Coordinate center_x = 0;
    Coordinate center_y = 0;
    Coordinate center_z = 0;
    Coordinate min_x = 0;
    Coordinate max_x = 0;
    Coordinate min_y = 0;
    Coordinate max_y = 0;
    Coordinate min_z = 0;
    Coordinate max_z = 0;
    Coordinate max_length = 0;

    center_x = min_x = max_x = vertex_array[0];
    center_y = min_y = max_y = vertex_array[1];
    center_z = min_z = max_z = vertex_array[2];
    for (size_t i=1; i < count; i++)
    {
        Coordinate x = vertex_array[3*i];
        Coordinate y = vertex_array[3*i+1];
        Coordinate z = vertex_array[3*i+2];

        min_x = min_x > x ? x : min_x;
        max_x = max_x < x ? x : max_x;

        min_y = min_y > y ? y : min_y;
        max_y = max_y < y ? y : max_y;

        min_z = min_z > z ? z : min_z;
        max_z = max_z < z ? z : max_z;
    }
    center_x = (max_x + min_x)/2;
    center_y = (max_y + min_y)/2;
    center_z = (max_z + min_z)/2;

    max_length = max_x - min_x;
    if (max_y - min_y > max_length)
    {
        max_length = max_y - min_y;
    }
    if (max_z - min_z > max_length)
    {
        max_length = max_z - min_z;
    }

    Center center;
    center.cx = center_x;
    center.cy = center_y;
    center.cz = center_z;
    center.bounding_length = max_length;
    return center;
}
