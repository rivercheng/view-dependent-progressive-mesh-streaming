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

void SimpleRender::keyboard(unsigned char key, int, int)
{
    std::ofstream ofs;
    switch (key)
    {
    case 27:
        sleep(1);
        exit(0);
        break;
    }
}

void SimpleRender::check_visibility()
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
    pq_->update(pixels_, width_*height_*3);
    glEnable(GL_LIGHTING);
    glEnable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void SimpleRender::draw_surface_with_arrays()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, gfmesh_->vertex_array());
    
    if (pq_ != 0 && to_check_visibility_)
    {
        check_visibility();
    }
    
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
    glReadPixels(0,0,width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
}

void SimpleRender::disp()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);

    glPushMatrix();
    glTranslated(dx_, dy_, dz_);
    glRotated(angle_x_, 1.0, 0.0, 0.0);
    glRotated(angle_y_, 0.0, 1.0, 0.0);
    glRotated(angle_z_, 0.0, 0.0, 1.0);
    glScaled (scale_, scale_, scale_);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    draw_surface_with_arrays();
    glutSwapBuffers();
    glPopMatrix();
}

void SimpleRender::outputImage(std::ostream& os)
{

    os<<"P5 "<<width_<<" "<<height_<<" 255"<<std::endl;
    unsigned char* ptr = pixels_;
    for (size_t i = 0; i< size_t(width_*height_); i++)
    {
            unsigned char r = *(ptr++);
            unsigned char g = *(ptr++);
            unsigned char b = *(ptr++);
            unsigned char y = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
            os.write((char*)&y, 1);
    }
}

void SimpleRender::reshape(int w, int h)
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

void SimpleRender::do_main(int interval)
{
    static int counter = 0;
    static std::deque<VertexID> vertices_to_split;
    if (counter == 0)
    {
        std::ofstream ofs("final_image.pgm");
        outputImage(ofs);
        ofs.close();
        gfmesh_ = mesh_begin_;
        to_check_visibility_ = true;
        interval = 100;
    }
    else 
    {
        if (counter == 1)
        {
            VertexID id = 0;
            int count = 0; 
            while((id = pq_->pop()) != 0 && count < 15000)
            {
                if (split_map_.find(id) != split_map_.end())
                {
                    vertices_to_split.push_back(id);
                    count ++;
                }
            }
            to_check_visibility_ = false;
        }
        else if (!vertices_to_split.empty())
        {
            VertexID id = vertices_to_split.front();
            vertices_to_split.pop_front();
            size_t pos = 0;
            //std::cerr << "id " << id;
            std::map<VertexID, BitString>::const_iterator it = split_map_.find(id);
            if (it != split_map_.end())
            {
                //std::cerr << it->second;
                gfmesh_->decode(id, it->second, &pos);
                gfmesh_->update();
                glutPostRedisplay();
            }
            //std::cerr << std::endl;
        }
        else if (counter > 1)
        {
            exit(0);
        }

        if (counter == counter / 300 * 300)
        {
            std::string str;
            std::stringstream sstr(str);
            sstr << prefix_ << "_output_image_" << counter / 300 <<".pgm";
            std::cerr << sstr.str().c_str() << std::endl;
            std::ofstream ofs(sstr.str().c_str());
            outputImage(ofs);
            ofs.close();
        }
    }
    counter ++;
    //glutTimerFunc(interval, do_main, interval);
}

SimpleRender::SimpleRender(int argc, char *argv[], const char *name, Gfmesh *gfmesh_final, Vdmesh *gfmesh_begin, std::map<VertexID, BitString>& split_map, VertexPQ *pq, std::string prefix) 
        :BaseRender(argc, argv, name, false), gfmesh_(gfmesh_final), mesh_begin_(gfmesh_begin), split_map_(split_map), pq_(pq), prefix_(prefix), \
        to_output_(false), to_check_visibility_(false)
{
    auto_center(gfmesh_->vertex_number(), gfmesh_->vertex_array());
    
    framerate_ = 50;
    initGlut(argc, argv);
    
    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}
