#ifndef __MEASURE_H__
#define __MEASURE_H__
#include "common_def.hh"
struct Center
{
    double cx;
    double cy;
    double cz;
    double bounding_length;
};

Center auto_center(size_t count, const Coordinate *vertex_array);
#endif //__MEASURE_H__
