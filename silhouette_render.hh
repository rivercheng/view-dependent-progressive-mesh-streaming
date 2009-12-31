#ifndef __SILHOUETTE_PRENDER_HH__
#define __SILHOUETTE_PRENDER_HH__

#include <list>
#include <map>
#include <deque>
#include <string>
#include "vertexid.hh"
#include "baserender.hh"
#include "psnr_mse.hh"

class Gfmesh;
class Vdmesh;
class VertexPQ;
class BitString;

class SilhouetteRender : public BaseRender
{
public:
    SilhouetteRender(int argc, char *argv[], const char *name, Vdmesh *mesh, const Center& center, VertexPQ *pq);
    void set_original_image(const std::string& pgm_file);
    
private:
    Vdmesh   *gfmesh_;
    VertexPQ *pq_;
    unsigned char pixels_[1024 * 768 * 3];


    bool to_output_;
    bool to_check_visibility_;
    bool rendered_;
    
    virtual void keyboard(unsigned char key, int x, int y);
    virtual void idle(void);
    
    virtual void draw_surface_with_arrays(void);
    
    void outputImage(std::ostream& os);
    void check_visibility(void);
};
#endif
