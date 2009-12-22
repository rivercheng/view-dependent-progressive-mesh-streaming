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
    to_check_visibility_ = false;
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
        if (gfmesh_->face_is_visible(i))
        {
            glNormal3d(gfmesh_->face_normal_array()[3*i], gfmesh_->face_normal_array()[3*i+1], gfmesh_->face_normal_array()[3*i+2]);
            VertexIndex v1 = gfmesh_->vertex1_in_face(i);
            VertexIndex v2 = gfmesh_->vertex2_in_face(i);
            VertexIndex v3 = gfmesh_->vertex3_in_face(i);
            glArrayElement(v1);
            glArrayElement(v2);
            glArrayElement(v3);
        }
    }
    glEnd();
    glReadPixels(0,0,width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
    rendered_ = true;
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

void SimpleRender::idle()
{
    if (pq_ == 0)
    {
        output_best_image();
    }
    else
    {
        do_main();
    }
}

void SimpleRender::output_best_image(void)
{
    std::cerr << "in output" << std::endl;
    if (rendered_)
    {
        std::string output_name = prefix_ + "_final_image.pgm";
        std::ofstream ofs(output_name.c_str());
        outputImage(ofs);
        ofs.close();
        exit(0);
    }
}

VertexID SimpleRender::next_to_be_split()
{
    VertexID id = gfmesh_->to_be_split();
    if (id == 0) // No pending splits
    {
        id = pq_->pop();
    }
    return id;
}

void SimpleRender::do_main()
{
    static int count = 0;
    if (rendered_)
    {
        std::string output_name;
        std::stringstream sstr(output_name);
        sstr << prefix_ << count << ".pgm";
        std::ofstream ofs(sstr.str().c_str());
        outputImage(ofs);
        ofs.close();
        if (count == 0)
        {
            for (int i = 0; i < initial_size_; i++)
            {
                VertexID id = next_to_be_split();
                if (id == 0) break;
                buffer_.push_back(id);
            }
            for (int i = 0; i < batch_size_; i++)
            {
                if (buffer_.empty()) break;
                VertexID id = buffer_.front();
                buffer_.pop_front();
                size_t pos = 0;
                gfmesh_->decode(id, split_map_[id], &pos);
                count ++;
            }
        }
        else
        {
            for (int i = 0; i < batch_size_; i++)
            {
                VertexID id = next_to_be_split();
                if (id == 0) break;
                buffer_.push_back(id);
            }

            for (int i = 0; i < batch_size_; i++)
            {
                if (count > total_count_) 
                {
                    exit(0);
                }
                if (buffer_.empty()) break;
                VertexID id = buffer_.front();
                buffer_.pop_front();
                size_t pos = 0;
                if (count / 100 * 100 == count)
                {
                    std::cerr << count << " " << id << std::endl;
                }
                gfmesh_->decode(id, split_map_[id], &pos);
                count ++;
            }
        }
        gfmesh_->update();
        
        rendered_ = false;
        to_check_visibility_ = true;
        glutPostRedisplay();
    }
}

SimpleRender::SimpleRender(int argc, char *argv[], const char *name, Vdmesh *gfmesh, std::map<VertexID, BitString>& split_map, VertexPQ *pq, std::string prefix, int initial_size, int batch_size, int total_count) 
        :BaseRender(argc, argv, name, false), gfmesh_(gfmesh), split_map_(split_map), pq_(pq), prefix_(prefix), initial_size_(initial_size), batch_size_(batch_size), total_count_(total_count), \
        to_output_(false), to_check_visibility_(false), rendered_(false)
{
    if (pq_ != 0) to_check_visibility_ = true;
    auto_center(gfmesh_->vertex_number(), gfmesh_->vertex_array());
    
    framerate_ = 50;
    initGlut(argc, argv);
    
    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}
