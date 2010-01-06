#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <sstream>
#include <sys/time.h>
#include <cstdlib>
#include "render.hh"
#include "gfmesh.hh"
#include "common_def.hh"
#include "vertexpq.hh"

void Render::check_visibility()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    unsigned char color_r = 0;
    unsigned char color_g = 0;
    unsigned char color_b = 1;
    glBegin(GL_TRIANGLES);
    for (size_t i=0; i<gfmesh_->face_number(); i++)
    {
        VertexIndex v1 = gfmesh_->vertex1_in_face(i);
        VertexIndex v2 = gfmesh_->vertex2_in_face(i);
        VertexIndex v3 = gfmesh_->vertex3_in_face(i);
        glColor3ub(color_r, color_g, color_b);
        glArrayElement(v1);
        glArrayElement(v2);
        glArrayElement(v3);
        if (color_b == 255)
        {
            color_b = 0;
            if (color_g == 255)
            {
                color_g = 0;
                if (color_r == 255)
                {
                    std::cerr<<"overflow!"<<std::endl;
                }
                else
                {
                    color_r ++;
                }
            }
            else
            {
                color_g ++;
            }
        }
        else
        {
            color_b++;
        }
    }
    glEnd();
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
    std::cerr<<"update"<<std::endl;
    pq_->update(pixels_, width_*height_*3, width_);
    glEnable(GL_LIGHTING);
    glEnable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void Render::draw_surface_with_arrays()
{
    static int output_counter = 1;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, gfmesh_->vertex_array());
    
    if (pq_ != 0)
    {
        check_visibility();
        for (int i = 0; i < 300; i++)
        {
            std::cout << pq_->pop() << "\n";
        }
    }
    
    if (smooth_ || interpolated_)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_DOUBLE, 0, gfmesh_->vertex_normal_array());
        glDrawElements(GL_TRIANGLES, 3*gfmesh_->face_number(),GL_UNSIGNED_INT, gfmesh_->face_array());
    }
    else
    {
        glDisableClientState(GL_NORMAL_ARRAY);
        glBegin(GL_TRIANGLES);
        for (size_t i=0; i< gfmesh_->face_number(); i++)
        {
                glNormal3d(gfmesh_->face_normal_array()[3*i], gfmesh_->face_normal_array()[3*i+1], gfmesh_->face_normal_array()[3*i+2]);
                VertexIndex v1 = gfmesh_->vertex1_in_face(i);
                VertexIndex v2 = gfmesh_->vertex2_in_face(i);
                VertexIndex v3 = gfmesh_->vertex3_in_face(i);
                glArrayElement(v1);
                glArrayElement(v2);
                glArrayElement(v3);
        }
        glEnd();
    }
    if (to_output_)
    {
        glReadPixels(0,0,width_,height_,GL_RGB, GL_UNSIGNED_BYTE, pixels_);

        //output pgm file contains Y
        std::ostringstream filename_str4;
        filename_str4<<"output"<<output_counter<<".pgm";
        std::ofstream ofs4(filename_str4.str().c_str(), std::ios::binary);
        ofs4<<"P5 "<<width_<<" "<<height_<<" 255"<<std::endl;
        unsigned char* ptr = pixels_;
        for (size_t i = 0; i< size_t(width_*height_); i++)
        {
            unsigned char r = *(ptr++);
            unsigned char g = *(ptr++);
            unsigned char b = *(ptr++);
            unsigned char y = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
            ofs4.write((char*)&y, 1);
        }
        ofs4.close();

        output_counter++;
        to_output_ = false;
    }

}



void Render::render_reset()
{
    dx_ = 0;
    dy_ = 0;
    dz_ = 0;
    angle_x_ = 0;
    angle_y_ = 0;
    angle_z_ = 0;
    scale_   = 1;
    mouse_button_ = 0;
    mouse_previous_x_ = 0;
    mouse_previous_y_ = 0;
    mouse_last_x_ = 0;
    mouse_last_y_ = 0;
    smooth_ = false;
    interpolated_ = false;
    perspective_  = true;
    outline_      = false;
    fill_         = true;
}

void Render::keyboard(unsigned char key, int, int)
{
    std::ofstream ofs;
    switch (key)
    {
    case 27:
        sleep(1);
        exit(0);
    case 'R':
    case 'r':
        render_reset();
        break;
    case 'S':
    case 's':
        if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
        {
            scale_ *= 0.5;
        }
        else
        {
            scale_ *= 2;
        }
        break;
    case 'M':
    case 'm':
        smooth_ = !smooth_;
        break;
    case 'I':
    case 'i':
        interpolated_ = !interpolated_;
        break;
    case 'F':
    case 'f':
        fill_   = !fill_;
        break;
    case 'L':
    case 'l':
        outline_ = !outline_;
        break;
    case 'P':
    case 'p':
        perspective_ = !perspective_;
        reshape(width_, height_);
        break;
    case 'O':
    case 'o':
        to_output_ = true;
        break;
    case 'V':
    case 'v':
        ofs.open("view_points", std::ios::app);
        ofs<<dx_*step_x_<<" "<<dy_*step_y_<<" "<<dz_*step_z_<<" " \
        <<angle_x_<<" "<<angle_y_<<" "<< \
        angle_z_<<" "<<scale_<<std::endl;
        ofs.close();
        break;
    default:
        break;
    }
    glutPostRedisplay();
}


void Render::handleSpecial(int key, int, int, int state)
{
    if (state != GLUT_ACTIVE_ALT && state != GLUT_ACTIVE_CTRL)
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            angle_y_ -= 10;
            break;
        case GLUT_KEY_RIGHT:
            angle_y_ += 10;
            break;
        case GLUT_KEY_UP:
            //dz_ += 0.1 * bounding_length_;
            dz_ ++;
            break;
        case GLUT_KEY_DOWN:
            //dz_ -= 0.1 * bounding_length_;
            dz_ --;
            break;
        default:
            break;
        }
    }
    else if (state == GLUT_ACTIVE_ALT)
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            //dx_ -= 0.1*bounding_length_;
            dx_ --;
            break;
        case GLUT_KEY_RIGHT:
            //dx_ += 0.1*bounding_length_;
            dx_ ++;
            break;
        case GLUT_KEY_UP:
            //dy_ += 0.1*bounding_length_;
            dy_ ++;
            break;
        case GLUT_KEY_DOWN:
            //dy_ -= 0.1*bounding_length_;
            dy_ --;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            angle_z_ += 10;
            break;
        case GLUT_KEY_RIGHT:
            angle_z_ -= 10;
            break;
        case GLUT_KEY_UP:
            angle_x_ -= 10;
            break;
        case GLUT_KEY_DOWN:
            angle_x_ += 10;
            break;
        }
    }
    glutPostRedisplay();
}

void Render::special(int key, int x, int y)
{
    int state = glutGetModifiers();
    handleSpecial(key, x, y, state);
}

void Render::mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        mouse_button_ = button;
        mouse_previous_x_ = x;
        mouse_previous_y_ = y;
        mouse_last_x_ = x;
        mouse_last_y_ = y;
    }
    else if (state == GLUT_UP)
    {
        mouse_button_ = 0;
        mouse_previous_x_ = 0;
        mouse_previous_y_ = 0;
        mouse_last_x_ = 0;
        mouse_last_y_ = 0;
    }
}

void Render::motion(int x, int y)
{
    if (mouse_button_ == GLUT_LEFT_BUTTON)
    {
        angle_y_ += (120 * (x - mouse_previous_x_) / width_);
        angle_x_ += (120 * (y - mouse_previous_y_) / height_);
        mouse_previous_x_ = x;
        mouse_previous_y_ = y;
    }
    else if (mouse_button_ == GLUT_RIGHT_BUTTON)
    {
        dz_ += (bounding_length_ * (y - mouse_previous_y_)/height_);
        mouse_previous_y_ = y;
    }
    glutPostRedisplay();
}

void Render::timer(int value)
{
    if (gfmesh_->updated())
    {
        gfmesh()->reset_updated();
        glutPostRedisplay();
    }
    glutTimerFunc(1000/value, timerWrapper, value);
}

Render::Render(int argc, char *argv[], const char *name, Gfmesh *gfmesh, int framerate, const Center& center, VertexPQ *pq) 
        :BaseRender(argc, argv, name, false), gfmesh_(gfmesh), pq_(pq), \
        to_output_(false), to_check_visibility_(true)
{
    set_center(center);
    framerate_ = framerate;
    initGlut(argc, argv);

    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}

