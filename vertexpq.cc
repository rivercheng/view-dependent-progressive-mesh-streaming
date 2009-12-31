#include "vertexpq.hh"
#include <algorithm>


VertexPQ::VertexPQ(Vdmesh *vdmesh, SelectMode mode, std::map<VertexID, BitString> *split_map)
        :vdmesh_(vdmesh), mode_(mode), split_map_(split_map)
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
            assert(importance > 0);
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

void VertexPQ::find_silhouette(unsigned char *pixels, size_t size, int width, std::set<FaceIndex>& silhouette)
{
    int seq_array[1024*768];
    for (size_t i = 0; i < size; i+=3)
    {
        unsigned char color_r = pixels[i];
        unsigned char color_g = pixels[i+1];
        unsigned char color_b = pixels[i+2];
        FaceIndex seq_no = color_r * 65536 + color_g*256 + color_b;
        seq_array[i/3] = seq_no;
    }

    for (size_t i = 0; i < size; i++)
    {
        if (seq_array[i] == 0)
        {
            // Top
            //std::cerr << "current " << i << " ";
            if (i >= (size_t)width && seq_array[i-width] != 0)
            {
                silhouette.insert(seq_array[i-width]-1);
                //std::cerr <<" top " << i-width << " " << seq_array[i-width]-1;
            }
            //bottom
            if (i < size - width && seq_array[i+width] != 0)
            {
                silhouette.insert(seq_array[i+width]-1);
                //std::cerr <<" bottom " << i+width << " " << seq_array[i+width]-1;
            }
            //left
            if (i / width * width != i && seq_array[i-1] != 0)
            {
                silhouette.insert(seq_array[i-1]-1);
                //std::cerr <<" left " << i-1 << " " << seq_array[i-1]-1;
            }
            //right
            if ((i+1)/width * width != (i+1) && seq_array[i+1] != 0)
            {
                silhouette.insert(seq_array[i+1]-1);
                //std::cerr <<" right " << i+1 << " " << seq_array[i+1]-1;
            }
            //std::cerr << std::endl;
        }
    }
}

void VertexPQ::update(unsigned char* pixels, size_t size, int width)
{
    //find silhouette
    if (mode_ == SilhouetteScreen)
    {
        std::set<FaceIndex> face_silhouette;
        find_silhouette(pixels, size, width, face_silhouette);
        silhouette_.clear();
        std::set<FaceIndex>::const_iterator it = face_silhouette.begin();
        std::set<FaceIndex>::const_iterator end = face_silhouette.end();
        for (; it != end; ++it)
        {
            //std::cerr<<*it<<" " << vdmesh_->face_number() <<std::endl;
            silhouette_.insert(vdmesh_->vertex1_in_face(*it));
            silhouette_.insert(vdmesh_->vertex2_in_face(*it));
            silhouette_.insert(vdmesh_->vertex3_in_face(*it));
        }
    }

    //sort according to the screen area
    index_queue_.clear();
    silhouette_queue_.clear();
    stat_screen_area(pixels, size);
    for (size_t i = 0; i <  vdmesh_->vertex_number(); i++)
    {
        //std::cerr<<"vertex "<<i<<" "<<" visible: "<<vdmesh_->vertex_is_visible(i)<<" importance "<<vdmesh_->vertex_importance(i)<<std::endl;
        if (vdmesh_->vertex_is_visible(i))
        {
            if (split_map_ != 0)
            {
                if (split_map_->find(vdmesh_->index2id(i)) == split_map_->end())
                {
                    continue;
                }
            }
            assert(vdmesh_->vertex_importance(i) > 0);
            if (mode_ != SilhouetteScreen || !in_silhouette(i))
            {
                index_queue_.push_back(i);
            }
            else
            {
                silhouette_queue_.push_back(i);
            }
        }
    }
    //std::cerr<<"queue size "<<index_queue_.size()<<std::endl;
    if (mode_ == ScreenArea)
    {
        std::make_heap(index_queue_.begin(), index_queue_.end(), CompareArea(vdmesh_));
    }
    else if (mode_ == SilhouetteScreen)
    {
        std::make_heap(index_queue_.begin(), index_queue_.end(), CompareArea(vdmesh_));
        std::make_heap(silhouette_queue_.begin(), silhouette_queue_.end(), CompareArea(vdmesh_));
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
    if (mode_ == SilhouetteScreen && !silhouette_queue_.empty())
    {
        std::pop_heap(silhouette_queue_.begin(), silhouette_queue_.end(), CompareArea(vdmesh_));
        VertexIndex index = silhouette_queue_.back();
        top = vdmesh_->index2id(index);
        silhouette_queue_.pop_back();
    }
    else if (!index_queue_.empty())
    {
        if (mode_ == Level)
        {
            std::pop_heap(index_queue_.begin(), index_queue_.end(), CompareLevel(vdmesh_));
        }
        else if (mode_ == ScreenArea)
        {
            std::pop_heap(index_queue_.begin(), index_queue_.end(), CompareArea(vdmesh_));
        }
        VertexIndex index = index_queue_.back();
        top = vdmesh_->index2id(index);
        index_queue_.pop_back();
        //std::cerr << top << " " << index << " " << vdmesh_->vertex_importance(index) << std::endl;
    }
    return top;
}
