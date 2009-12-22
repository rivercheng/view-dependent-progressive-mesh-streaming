#ifndef __SIMPLE_PRENDER_HH__
#define __SIMPLE_PRENDER_HH__

#include <list>
#include <map>
#include <deque>
#include "vertexid.hh"
#include "baserender.hh"
class Gfmesh;
class Vdmesh;
class VertexPQ;
class BitString;

class SimpleRender : public BaseRender
{
public:
    SimpleRender(int argc, char *argv[], const char *name, Vdmesh *mesh, std::map<VertexID, BitString>& split_map, VertexPQ *pq, std::string prefix, int initial_size, int batch_size_, int total_count);
    
private:
    Vdmesh   *gfmesh_;
    unsigned char pixels_[1024 * 768 * 3];
    std::map<VertexID, BitString>& split_map_;
    VertexPQ *pq_;
    std::string prefix_;

    int initial_size_;
    int batch_size_;
    int total_count_;

    bool to_output_;
    bool to_check_visibility_;
    bool rendered_;
    std::deque<VertexID> buffer_;
    
    virtual void keyboard(unsigned char key, int x, int y);
    virtual void idle(void);
    
    virtual void draw_surface_with_arrays(void);
    
    void outputImage(std::ostream& os);
    void output_best_image(void);
    void do_main(void);
    void check_visibility(void);
    VertexID next_to_be_split(void);
};
#endif
