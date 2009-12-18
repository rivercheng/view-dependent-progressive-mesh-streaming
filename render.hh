#ifndef __PRENDER_HH__
#define __PRENDER_HH__

#include <list>
class Gfmesh;
class VertexPQ;

class NoBaseMesh{};

class Render
{
public:
    Render(int argc, char *argv[], const char *name, Gfmesh *mesh, int framerate, VertexPQ *pq = 0);
    
    void enterMainLoop();

    void setSmooth(bool value)
    {
        smooth_ = value;
    }
    
    void setInterPolated(bool value)
    {
        interpolated_ = value;
    }
    
    void setPerspective(bool value)
    {
        perspective_ = value;
    }
    
    void setOutline(bool value)
    {
        outline_ = value;
    }
    
    void setFill(bool value)
    {
        fill_ = value;
    }
    
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
    void render_reset(void);

private:
    enum Type
    {
        NORMAL,
        SPECIAL,
    };

    enum State
    {
        NONE,
        ALT,
        CTRL,
    };

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
    Gfmesh   *gfmesh_;
    VertexPQ *pq_;
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
    
    friend void keyboard(unsigned char key, int x, int y);
    friend void special(int key, int x, int y);
    friend void mouse(int button, int state, int x, int y);
    friend void motion(int x, int y);
    friend void timer(int value);
    friend void handleSpecial(int key, int x, int y, int state);
    friend void check_visibility(void);
};
#endif
