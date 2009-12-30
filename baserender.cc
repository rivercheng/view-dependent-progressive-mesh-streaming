#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <sstream>
#include <deque>
#include "baserender.hh"
#include "common_def.hh"
BaseRender *BaseRender::instance = 0;
BaseRender::BaseRender(int argc, char *argv[], const char *name, bool auto_init) 
    :display_mode_(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH), \
     pos_x_(500), pos_y_(300), name_(name), width_(500), height_(500),\
     framerate_(60), \
        view_angle_(45), \
        min_distance_(0.01), max_distance_(5000),\
        view_x_(0), view_y_(0), view_z_(0), dx_(0), dy_(0), dz_(0), step_x_(0), step_y_(0), step_z_(0), angle_x_(0), \
        angle_y_(0), angle_z_(0), scale_(1), bounding_length_(1), mouse_button_(0), \
        mouse_previous_x_(0), mouse_previous_y_(0),mouse_last_x_(0),mouse_last_y_(0), \
        smooth_(false), interpolated_(false), perspective_(true), \
        outline_(false), fill_(true)
{
    if (instance == 0)
    {
        instance = this;
    }
    else
    {
        throw MultipleInstanceOfRender();
    }
    if (auto_init)
    {
        initGlut(argc, argv);
    }
}


void BaseRender::initGlut(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(display_mode_);
    glutInitWindowSize(width_, height_);
    glutInitWindowPosition(pos_x_, pos_y_);
    glutCreateWindow(name_);

    glutDisplayFunc(dispWrapper);
    glutReshapeFunc(reshapeWrapper);
    glutKeyboardFunc(keyboardWrapper);
    glutSpecialFunc(specialWrapper);	
    glutMouseFunc(mouseWrapper);
    glutMotionFunc(motionWrapper);
    glutTimerFunc(1000./framerate_, timerWrapper, framerate_);
    glutIdleFunc(idleWrapper);

    glClearColor(0, 0, 0, 0);
    set_materials();
    set_lights();
    set_client_status();
}

void BaseRender::set_materials()
{
    GLfloat mat_specular[] = {0.3, 0.3, 0.3, 0.0};
    GLfloat mat_shininess[] = {100};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS,mat_shininess);
}

void BaseRender::set_lights()
{
    GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void BaseRender::set_client_status()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
}

void BaseRender::enterMainLoop()
{
    glutMainLoop();
}

void BaseRender::set_center(const Center& center)
{
    bounding_length_ = center.bounding_length;
    step_x_ = step_y_ = step_z_ = 0.1 * bounding_length_;

    double factor = 2;
    left_distance_ = right_distance_ = top_distance_ = bottom_distance_ = factor*bounding_length_*0.5;
    view_x_ = center.cx;
    view_y_ = center.cy;
    view_z_ = center.cz + 1.5*bounding_length_;
    min_distance_ = 0.01 * view_z_;
    max_distance_ = 100 * view_z_;
}

void BaseRender::disp()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if (smooth_)
    {
        glShadeModel(GL_SMOOTH);
    }
    else
    {
        glShadeModel(GL_FLAT);
    }

    glPushMatrix();
    glTranslated(dx_*step_x_, dy_*step_y_, dz_*step_z_);
    glRotated(angle_x_, 1.0, 0.0, 0.0);
    glRotated(angle_y_, 0.0, 1.0, 0.0);
    glRotated(angle_z_, 0.0, 0.0, 1.0);
    glScaled (scale_, scale_, scale_);
    if (fill_)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        draw_surface_with_arrays();
    }
    if (outline_)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        draw_surface_with_arrays();
    }
    glutSwapBuffers();
    glPopMatrix();
}


void BaseRender::reshape(int w, int h)
{
    if (h==0) h = 1;
    if (w==0) w = 1;
    width_ = w;
    height_= h;
    double ratio = 1.0 * w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0 , w , h);

    gluPerspective(view_angle_, ratio, min_distance_, max_distance_);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(view_x_, view_y_, view_z_, view_x_, view_y_, view_z_-2*bounding_length_, 0.0, 1.0, 0.0);
    return;
}
