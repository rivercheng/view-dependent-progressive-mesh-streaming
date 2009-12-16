#include "vertexpq.hh"
#include <algorithm>


VertexPQ::VertexPQ(Vdmesh *vdmesh, SelectMode mode)
        :vdmesh_(vdmesh), mode_(mode)
{
}

void VertexPQ::stat_screen_area(unsigned char *pixels, size_t size)
{
    vdmesh_->reset_vertex_visibility(false);
    vdmesh_->reset_face_visibility(false);
    vdmesh_->reset_vertex_importance();
    vdmesh_->reset_face_importance();

    for (size_t i = 0; i < size; i += 3)
    {
        unsigned char color_r = pixels[i];
        unsigned char color_g = pixels[i+1];
        unsigned char color_b = pixels[i+2];
        FaceIndex seq_no = color_r * 65536 + color_g*256 + color_b;
        if (seq_no != 0)
        {
            vdmesh_->set_face_visibility(seq_no - 1, true);
            vdmesh_->set_face_importance(seq_no - 1, vdmesh_->face_importance(seq_no - 1) + 1);
        }
    }
    
    for (size_t i = 0; i < vdmesh_->face_number(); i++)
    {
        int importance = vdmesh_->face_importance(i);
        if (importance != 0)
        {
            VertexIndex v1 = vdmesh_->vertex1_in_face(i);
            VertexIndex v2 = vdmesh_->vertex2_in_face(i);
            VertexIndex v3 = vdmesh_->vertex3_in_face(i);
        
            vdmesh_->set_vertex_importance(v1, vdmesh_->vertex_importance(v1) + importance);
            vdmesh_->set_vertex_importance(v2, vdmesh_->vertex_importance(v2) + importance);
            vdmesh_->set_vertex_importance(v3, vdmesh_->vertex_importance(v3) + importance);

            vdmesh_->set_vertex_visibility(v1, true);
            vdmesh_->set_vertex_visibility(v2, true);
            vdmesh_->set_vertex_visibility(v3, true);
        }
    }
}

void VertexPQ::update(unsigned char* pixels, size_t size)
{
    index_queue_.clear();
    stat_screen_area(pixels, size);
    for (size_t i = 0; i <  vdmesh_->vertex_number(); i++)
    {
        if (vdmesh_->vertex_is_visible(i))
        {
            index_queue_.push_back(i);
        }
    }
    std::cerr<<"queue size "<<index_queue_.size()<<std::endl;
    if (mode_ == ScreenArea)
    {
        std::make_heap(index_queue_.begin(), index_queue_.end(), CompareArea(vdmesh_));
    }
    else if (mode_ == Level)
    {
        std::make_heap(index_queue_.begin(), index_queue_.end(), CompareLevel(vdmesh_));
    }
    else 
    {
        std::random_shuffle(index_queue_.begin(), index_queue_.end());
    }
}

VertexIndex VertexPQ::pop()
{
    VertexID top = 0;
    if (!index_queue_.empty())
    {
        if (mode_ == Level || mode_ == ScreenArea)
        {
            std::pop_heap(index_queue_.begin(), index_queue_.end());
        }
        top = index_queue_.back();
        index_queue_.pop_back();
    }
    return top;
}
