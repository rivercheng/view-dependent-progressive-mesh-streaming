#ifndef __BASE_PRENDER_HH__
#define __BASE_PRENDER_HH__

#include <list>
#include <map>
#include "vertexid.hh"
#include "common_def.hh"

class Gfmesh;
class BaseRender
{
public:
    BaseRender(int argc, char **argv, const char *name, bool auto_init);

    virtual ~BaseRender(){instance = 0;}

    void enterMainLoop();

    virtual void initGlut(int argc, char** argv);

    void auto_center(size_t count, const Coordinate *vertex_array);
    
    void set_center(double center_x, double center_y, double center_z, double distance)
    {
        view_x_ = center_x;
        view_y_ = center_y;
        view_z_ = center_z + distance;
    }
    
    void setView(double dx, double dy, double dz, double angle_x,\
                 double angle_y, double angle_z, double scale)
    {
        dx_ = dx / step_x_;
        dy_ = dy / step_y_;
        dz_ = dz / step_z_;
        angle_x_ = angle_x;
        angle_z_ = angle_z;
        angle_y_ = angle_y;
        scale_   = scale;
    }

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

    virtual void disp(void);
    virtual void reshape(int width, int height);
    virtual void keyboard(unsigned char, int, int){return;}
    virtual void special(int, int, int) {return;}
    virtual void mouse(int, int, int, int){return;}
    virtual void motion(int, int){return;}
    virtual void timer(int){return;}
    virtual void idle(void){return;}

private:
    //Non copiable.
    BaseRender(const BaseRender&);
    BaseRender& operator=(const BaseRender&);
    virtual void draw_surface_with_arrays(void) = 0;

protected:
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
    //double dx_;
    //double dy_;
    //double dz_;
    int    dx_;
    int    dy_;
    int    dz_;
    double step_x_;
    double step_y_;
    double step_z_;
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
    static BaseRender *instance;

public:
    static void activateInstance(BaseRender* render)
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

    //Render cannot be instantiated more than once.
    class MultipleInstanceOfRender{};
};
#endif
