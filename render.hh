#ifndef __PRENDER_HH__
#define __PRENDER_HH__

#include <list>
#include "baserender.hh"
class Gfmesh;
class VertexPQ;

class NoBaseMesh{};

class Render : public BaseRender
{
public:
    Render(int argc, char *argv[], const char *name, Gfmesh *mesh, int framerate, VertexPQ *pq = 0);
    
    
    Gfmesh* gfmesh()
    {
        return gfmesh_;
    }

    virtual void disp(void);
    virtual void reshape(int width, int height);
    virtual void keyboard(unsigned char key, int x, int y);
    virtual void special(int key, int x, int y);
    virtual void mouse(int button, int state, int x, int y);
    virtual void motion(int x, int y);
    virtual void timer(int value);

private:
    void render_reset(void);

private:

    enum Key
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        R,
        Q,
    };
    
    struct Action
    {
          long  time;
          Type  type;
          State state;
           Key  key;
           Action(long t, Type ty, State s, Key k):
               time(t), type(ty), state(s), key(k){;}
    };

    typedef std::list<Action> Record;

private:
    void check_visibility(void);
    void handleSpecial(int key, int, int, int state);
    void draw_surface_with_arrays(void);

private:
    Gfmesh   *gfmesh_;
    VertexPQ *pq_;
    unsigned char pixels_[1024 * 768 * 3];
    
    bool to_output_;
    bool to_check_visibility_;
};
#endif
