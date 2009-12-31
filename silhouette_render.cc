#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <deque>
#include "silhouette_render.hh"
#include "gfmesh.hh"
#include "common_def.hh"
#include "vertexpq.hh"

void SilhouetteRender::keyboard(unsigned char key, int, int)
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

void SilhouetteRender::check_visibility()
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
    pq_->update(pixels_, width_*height_*3, width_);
    glEnable(GL_LIGHTING);
    glEnable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    to_check_visibility_ = false;
}

void SilhouetteRender::draw_surface_with_arrays()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, gfmesh_->vertex_array());
    
    if (pq_ != 0 && to_check_visibility_)
    {
        check_visibility();
    }
    
    glDisableClientState(GL_NORMAL_ARRAY);
    //glEnable(GL_COLOR_MATERIAL);
    //glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glBegin(GL_TRIANGLES);
    for (size_t i=0; i< gfmesh_->face_number(); i++)
    {
        if (gfmesh_->face_is_visible(i))
        {
            glNormal3d(gfmesh_->face_normal_array()[3*i], gfmesh_->face_normal_array()[3*i+1], gfmesh_->face_normal_array()[3*i+2]);
            VertexIndex v1 = gfmesh_->vertex1_in_face(i);
            VertexIndex v2 = gfmesh_->vertex2_in_face(i);
            VertexIndex v3 = gfmesh_->vertex3_in_face(i);
            //std::cerr << v1 <<" "<< v2 << " " << v3  << " " << pq_->in_silhouette(gfmesh_->index2id(v1)) <<std::endl;
            if (pq_->face_in_silhouette(i))
            {
                glColor3ub(255,0,0);
            }
            else
            {
                glColor3ub(255,255,255);
            }
            /*
            if (pq_->in_silhouette(gfmesh_->index2id(v1)))
            {
                glColor3ub(255, 0, 0);
            }
            else
            {
                glColor3ub(0, 0, 0);
            }
            */
            glArrayElement(v1);
            /*
            if (pq_->in_silhouette(gfmesh_->index2id(v2)))
            {
                glColor3ub(255, 0, 0);
            }
            else
            {
                glColor3ub(0, 0, 0);
            }*/
            glArrayElement(v2);
            /*
            if (pq_->in_silhouette(gfmesh_->index2id(v2)))
            {
                glColor3ub(255, 0, 0);
            }
            else
            {
                glColor3ub(0, 0, 0);
            }*/
            glArrayElement(v3);
        }
    }
    glEnd();
    //glDisable(GL_COLOR_MATERIAL);
    if (to_output_)
    {
        glReadPixels(0,0,width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
        to_output_ = false;
    }
    rendered_ = true;
}

void SilhouetteRender::outputImage(std::ostream& os)
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

void SilhouetteRender::idle()
{
    return;
}

SilhouetteRender::SilhouetteRender(int argc, char *argv[], const char *name, Vdmesh *gfmesh, const Center& center, VertexPQ *pq) 
        :BaseRender(argc, argv, name, false), gfmesh_(gfmesh), pq_(pq), \
        to_output_(true), to_check_visibility_(false), rendered_(false)
{
    //outline_ = true;
    if (pq_ != 0)
    {
        to_check_visibility_ = true;
    }
    
    set_center(center);
    
    framerate_ = 50;
    initGlut(argc, argv);
    
    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}

