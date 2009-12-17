#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <sstream>
#include <deque>
#include "simple_render.hh"
#include "gfmesh.hh"
#include "common_def.hh"
#include "vertexpq.hh"

const size_t FRAME_BUFFER_SIZE = 1024*768*3;
static SimpleRender* render_;
static unsigned char pixels_[FRAME_BUFFER_SIZE] = {0};

inline void check_visibility()
{
    Gfmesh* gfmesh_ = render_->gfmesh();
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
    glReadPixels(0,0,render_->width_, render_->height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
    render_->pq_->update(pixels_, render_->width_*render_->height_*3);
    glEnable(GL_LIGHTING);
    glEnable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void draw_surface_with_arrays()
{
    Gfmesh *gfmesh_ = render_->gfmesh_;
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, gfmesh_->vertex_array());
    
    if (render_->pq_ != 0 && render_->to_check_visibility_)
    {
        check_visibility();
        /*
        for (int i = 0; i < 300; i++)
        {
            std::cout << render_->pq_->pop() << "\n";
        }
        */
    }
    
    if (render_->smooth_ || render_->interpolated_)
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
    glReadPixels(0,0,render_->width_, render_->height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
}

void disp()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);

    glPushMatrix();
    glTranslated(render_->dx_, render_->dy_, render_->dz_);
    glRotated(render_->angle_x_, 1.0, 0.0, 0.0);
    glRotated(render_->angle_y_, 0.0, 1.0, 0.0);
    glRotated(render_->angle_z_, 0.0, 0.0, 1.0);
    glScaled (render_->scale_, render_->scale_, render_->scale_);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    draw_surface_with_arrays();
    glutSwapBuffers();
    glPopMatrix();
}

void outputImage(unsigned char *pixels, int width, int height,  std::ostream& os)
{

    //output pgm file contains Y
    os<<"P5 "<<width<<" "<<height<<" 255"<<std::endl;
    unsigned char* ptr = pixels;
    for (size_t i = 0; i< size_t(width*height); i++)
    {
            unsigned char r = *(ptr++);
            unsigned char g = *(ptr++);
            unsigned char b = *(ptr++);
            unsigned char y = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
            os.write((char*)&y, 1);
    }
}

void reshape(int w, int h)
{
    if (h==0) h = 1;
    if (w==0) w = 1;
    render_->width_ = w;
    render_->height_= h;
    double ratio = 1.0 * w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0 , w , h);

    gluPerspective(render_->view_angle_, ratio, render_->min_distance_, render_->max_distance_);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(render_->view_x_, render_->view_y_, render_->view_z_, render_->view_x_, render_->view_y_, render_->view_z_-2*render_->bounding_length_, 0.0, 1.0, 0.0);
    return;
}

void do_main(int interval)
{
    static int counter = 0;
    static std::deque<VertexID> vertices_to_split;
    if (counter == 0)
    {
        std::ofstream ofs("final_image.pgm");
        outputImage(pixels_, render_->width_, render_->height_, ofs);
        ofs.close();
        render_->gfmesh_ = render_->mesh_begin_;
        render_->to_check_visibility_ = true;
    }
    else 
    {
        if (counter == 1)
        {
            VertexID id = 0;
            int count = 0; 
            while((id = render_->pq_->pop()) != 0 && count < 15000)
            {
                if (render_->split_map_.find(id) != render_->split_map_.end())
                {
                    vertices_to_split.push_back(id);
                    count ++;
                }
            }
            render_->to_check_visibility_ = false;
        }
        else if (!vertices_to_split.empty())
        {
            VertexID id = vertices_to_split.front();
            vertices_to_split.pop_front();
            size_t pos = 0;
            std::cerr << "id " << id;
            std::map<VertexID, BitString>::const_iterator it = render_->split_map_.find(id);
            if (it != render_->split_map_.end())
            {
                std::cerr << it->second;
                render_->gfmesh_->decode(id, it->second, &pos);
                render_->gfmesh_->update();
                glutPostRedisplay();
            }
            std::cerr << std::endl;
        }
        else
        {
            exit(0);
        }

        if (counter == counter / 300 * 300)
        {
            std::string str;
            std::stringstream sstr(str);
            sstr << render_->prefix_ << "_output_image_" << counter / 300 <<".pgm";
            std::cerr << sstr.str().c_str() << std::endl;
            std::ofstream ofs(sstr.str().c_str());
            outputImage(pixels_, render_->width_, render_->height_, ofs);
            ofs.close();
        }
    }
    counter ++;
    glutTimerFunc(interval, do_main, interval);
}

SimpleRender::SimpleRender(int& argc, char *argv[], const char *name, Gfmesh *gfmesh_final, Vdmesh *gfmesh_begin, std::map<VertexID, BitString>& split_map, VertexPQ *pq, std::string prefix) 
        :gfmesh_(gfmesh_final), mesh_begin_(gfmesh_begin), split_map_(split_map), pq_(pq), prefix_(prefix), \
        view_angle_(45), \
        min_distance_(0.01), max_distance_(5000),\
        view_x_(0), view_y_(0), view_z_(0), dx_(0), dy_(0), dz_(0), angle_x_(0), \
        angle_y_(0), angle_z_(0), scale_(1), bounding_length_(1), mouse_button_(0),\
        mouse_previous_x_(0), mouse_previous_y_(0),mouse_last_x_(0),mouse_last_y_(0) ,width_(500), height_(500), \
        smooth_(false), interpolated_(false), perspective_(true), \
        outline_(false), fill_(true), \
        to_output_(false), to_check_visibility_(false)
{

    Coordinate center_x = 0;
    Coordinate center_y = 0;
    Coordinate center_z = 0;
    Coordinate min_x = 0;
    Coordinate max_x = 0;
    Coordinate min_y = 0;
    Coordinate max_y = 0;
    Coordinate min_z = 0;
    Coordinate max_z = 0;
    Coordinate max_length = 0;

    center_x = min_x = max_x = gfmesh_->vertex_array()[0];
    center_y = min_y = max_y = gfmesh_->vertex_array()[1];
    center_z = min_z = max_z = gfmesh_->vertex_array()[2];
    for (size_t i=1; i < gfmesh_->vertex_number(); i++)
    {
        Coordinate x = gfmesh_->vertex_array()[3*i];
        Coordinate y = gfmesh_->vertex_array()[3*i+1];
        Coordinate z = gfmesh_->vertex_array()[3*i+2];

        min_x = min_x > x ? x : min_x;
        max_x = max_x < x ? x : max_x;

        min_y = min_y > y ? y : min_y;
        max_y = max_y < y ? y : max_y;

        min_z = min_z > z ? z : min_z;
        max_z = max_z < z ? z : max_z;
    }
    center_x = (max_x + min_x)/2;
    center_y = (max_y + min_y)/2;
    center_z = (max_z + min_z)/2;

    max_length = max_x - min_x;
    if (max_y - min_y > max_length)
    {
        max_length = max_y - min_y;
    }
    if (max_z - min_z > max_length)
    {
        max_length = max_z - min_z;
    }
    bounding_length_ = max_length;

    double factor = 2;
    left_distance_ = right_distance_ = top_distance_ = bottom_distance_ = factor*max_length*0.5;
    view_x_ = center_x;
    view_y_ = center_y;
    view_z_ = center_z + 1.5*max_length;
    min_distance_ = 0.01 * view_z_;
    max_distance_ = 100 * view_z_;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(500, 300);
    glutCreateWindow(name);

    glutDisplayFunc(disp);
    glutReshapeFunc(reshape);
    glutTimerFunc(50, do_main, 50);
    //glutIdleFunc(do_main);

    glClearColor(0., 0., 0., 0.);


    //normal
    GLfloat mat_specular[] = {0.3, 0.3, 0.3, 0.0};
    GLfloat mat_shininess[]  = {100};
    GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS,mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    render_ = this;
    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}

void SimpleRender::enterMainLoop()
{
    glutMainLoop();
}
