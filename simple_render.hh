#ifndef __SIMPLE_PRENDER_HH__
#define __SIMPLE_PRENDER_HH__

#include <list>
#include <map>
#include "vertexid.hh"
#include "baserender.hh"
class Gfmesh;
class Vdmesh;
class VertexPQ;
class BitString;

class SimpleRender : public BaseRender
{
public:
    SimpleRender(int argc, char *argv[], const char *name, Gfmesh *mesh, Vdmesh *mesh2, std::map<VertexID, BitString>& split_map, VertexPQ *pq, std::string prefix);
    
private:
    Gfmesh   *gfmesh_;
    Vdmesh   *mesh_begin_;
    unsigned char pixels_[1024 * 768 * 3];
    std::map<VertexID, BitString>& split_map_;
    VertexPQ *pq_;
    std::string prefix_;
    bool to_output_;
    bool to_check_visibility_;
    bool rendered_;
    
    virtual void disp(void);
    virtual void reshape(int w, int h);
    virtual void keyboard(unsigned char key, int x, int y);
    virtual void special(int, int, int){return;}
    virtual void mouse(int, int, int, int){return;}
    virtual void motion(int, int) {return;}
    virtual void timer(int) {return;}
    virtual void idle(void);
    
    void outputImage(std::ostream& os);
    void output_best_image(void);
    void draw_surface_with_arrays(void);
    void do_main(void);
    void check_visibility(void);
};
#endif
