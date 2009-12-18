#ifndef __BASIC_PRENDER_HH__
#define __BASIC_PRENDER_HH__

#include <list>
#include <map>
#include "vertexid.hh"
#include "common_def.hh"

class Gfmesh;
class BasicRender
{
public:
    BasicRender(int argc, char **argv, const char *name, bool auto_init);

    virtual ~BasicRender(){;}

    void enterMainLoop();

    virtual void initGlut(int argc, char** argv);

    void auto_center(size_t count, Coordinate *vertex_array);
    
    void set_center(double center_x, double center_y, double center_z, double distance)
    {
        view_x_ = center_x;
        view_y_ = center_y;
        view_z_ = center_z + distance;
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

    int width()
    {
        return width_;
    }

    int height()
    {
        return height_;
    }

    virtual void set_materials(void);
    virtual void set_lights(void);
    virtual void set_client_status(void);

    virtual void disp(void) = 0;
    virtual void reshape(int width, int height) = 0;
    virtual void keyboard(unsigned char key, int x, int y) = 0;
    virtual void special(int key, int x, int y) = 0;
    virtual void mouse(int button, int state, int x, int y) = 0;
    virtual void motion(int x, int y) = 0;
    virtual void timer(int value) = 0;
    virtual void idle(void)
    {
        ;
    }

private:
    //Non copiable.
    BasicRender(const BasicRender&);
    BasicRender& operator=(const BasicRender&);

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

protected:
    int    display_mode_;
    int    pos_x_;
    int    pos_y_;
    const char   *name_;
    int    width_;
    int    height_;
    int    framerate_;

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
    
    bool smooth_;
    bool interpolated_;
    bool perspective_;
    bool outline_;
    bool fill_;
    //=============Static Wrappers to be used as C callback functions
    
private:
    static BasicRender *instance;

public:
    static void activateInstance(BasicRender* render)
    {
        instance = render;
    }


    static void dispWrapper(void)
    {
        instance->disp();
    }
    
    static void keyboardWrapper(unsigned char key, int x, int y)
    {
        instance->keyboard(key, x, y);
    }
    
    static void reshapeWrapper(int w, int h)
    {
        instance->reshape(w, h);
    }

    static void specialWrapper(int key, int x, int y)
    {
        instance->special(key, x, y);
    }

    static void mouseWrapper(int button, int state, int x, int y)
    {
        instance->mouse(button, state, x, y);
    }

    static void motionWrapper(int x, int y)
    {
        instance->motion(x, y);
    }

    static void timerWrapper(int value)
    {
        instance->timer(value);
    }
    
    static void idleWrapper(void)
    {
        instance->idle();
    }
};
#endif
