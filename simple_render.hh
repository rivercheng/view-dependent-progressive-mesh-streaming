#ifndef __SIMPLE_PRENDER_HH__
#define __SIMPLE_PRENDER_HH__

#include <list>
#include <map>
#include "vertexid.hh"
class Gfmesh;
class Vdmesh;
class VertexPQ;
class BitString;

class NoBaseMesh{};

class SimpleRender
{
public:
    SimpleRender(int& argc, char *argv[], const char *name, Gfmesh *mesh, Vdmesh *mesh2, std::map<VertexID, BitString>& split_map, VertexPQ *pq, std::string prefix);
    
    void enterMainLoop();

    void setView(double dx, double dy, double dz, double angle_x,\
                 double angle_y, double angle_z, double scale)
    {
        dx_ = dx;
        dy_ = dy;
        dz_ = dz;
        angle_x_ = angle_x;
        angle_z_ = angle_z;
        angle_y_ = angle_y;
        scale_   = scale;
    }

    Gfmesh* gfmesh()
    {
        return gfmesh_;
    }

private:
    Gfmesh   *gfmesh_;
    Vdmesh   *mesh_begin_;
    std::map<VertexID, BitString>& split_map_;
    VertexPQ *pq_;
    std::string prefix_;
    double view_angle_;
    double left_distance_;
    double right_distance_;
    double top_distance_;
    double bottom_distance_;
    double min_distance_;
    double max_distance_;
    double view_x_;
    double view_y_;
    double view_z_;
    double dx_;
    double dy_;
    double dz_;
    double angle_x_;
    double angle_y_;
    double angle_z_;
    double scale_;
    double bounding_length_;
    int mouse_button_;
    int mouse_previous_x_;
    int mouse_previous_y_;
    int mouse_last_x_; //non incremental
    int mouse_last_y_; //no incremental
    int width_;
    int height_;
    bool smooth_;
    bool interpolated_;
    bool perspective_;
    bool outline_;
    bool fill_;
    bool to_output_;
    bool to_check_visibility_;
    
    friend void disp(void);
    friend void draw_surface_with_arrays(void);
    friend void reshape(int w, int h);
    friend void do_main(int interval);
    friend void check_visibility(void);
};
#endif
