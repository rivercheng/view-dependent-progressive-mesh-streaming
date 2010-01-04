#include "vertexpq.hh"
#include <algorithm>


VertexPQ::VertexPQ(Vdmesh *vdmesh, SelectMode mode, std::map<VertexID, BitString> *split_map)
        :vdmesh_(vdmesh), mode_(mode), silhouette_weight_(1), split_map_(split_map) 
{
    std::cerr <<"mode " << mode_ << std::endl;
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
        if (mode_ == WeightedScreen && face_in_silhouette(i))
        {
            //std::cerr << "face " << i << " multiplied by " << silhouette_weight_ << std::endl;
            importance *= silhouette_weight_;
        }

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

void VertexPQ::find_silhouette(unsigned char *pixels, size_t size, int width)
{
    seq_array_.clear();
    face_in_silhouette_.clear();
    for (size_t i = 0; i < size; i+=3)
    {
        unsigned char color_r = pixels[i];
        unsigned char color_g = pixels[i+1];
        unsigned char color_b = pixels[i+2];
        FaceIndex seq_no = color_r * 65536 + color_g*256 + color_b;
        seq_array_.push_back(seq_no);
    }

    for (size_t i = 0; i < size/3; i++)
    {
        if (seq_array_[i] == 0)
        {
            // Top
            //std::cerr << "current " << i << " ";
            if (i >= (size_t)width && seq_array_[i-width] != 0)
            {
                face_in_silhouette_.insert(seq_array_[i-width]-1);
                //std::cerr <<" top " << i-width << " " << seq_array_[i-width]-1;
            }
            //bottom
            if (i < size - width && seq_array_[i+width] != 0)
            {
                face_in_silhouette_.insert(seq_array_[i+width]-1);
                //std::cerr <<" bottom " << i+width << " " << seq_array_[i+width]-1;
            }
            //left
            if (i / width * width != i && seq_array_[i-1] != 0)
            {
                face_in_silhouette_.insert(seq_array_[i-1]-1);
                //std::cerr <<" left " << i-1 << " " << seq_array_[i-1]-1;
            }
            //right
            if ((i+1)/width * width != (i+1) && seq_array_[i+1] != 0)
            {
                face_in_silhouette_.insert(seq_array_[i+1]-1);
                //std::cerr <<" right " << i+1 << " " << seq_array_[i+1]-1;
            }
            //std::cerr << std::endl;
        }
    }
}

void VertexPQ::update(unsigned char* pixels, size_t size, int width)
{
    //find silhouette
    if (mode_ == SilhouetteScreen || mode_ == WeightedScreen)
    {
        find_silhouette(pixels, size, width);
        silhouette_.clear();
        std::set<FaceIndex>::const_iterator it = face_in_silhouette_.begin();
        std::set<FaceIndex>::const_iterator end = face_in_silhouette_.end();
        for (; it != end; ++it)
        {
            //std::cerr<<*it<<" " << vdmesh_->face_number() <<std::endl;
            silhouette_.insert(vdmesh_->vertex1_in_face(*it));
            silhouette_.insert(vdmesh_->vertex2_in_face(*it));
            silhouette_.insert(vdmesh_->vertex3_in_face(*it));
        }
    }

    //sort according to the screen area
    id_queue_.clear();
    silhouette_queue_.clear();
    stat_screen_area(pixels, size);
    for (size_t i = 0; i <  vdmesh_->vertex_number(); i++)
    {
        //std::cerr<<"vertex "<<i<<" "<<" visible: "<<vdmesh_->vertex_is_visible(i)<<" importance "<<vdmesh_->vertex_importance(i)<<std::endl;
        if (vdmesh_->vertex_is_visible(i))
        {
            //ignore those leave vertices
            VertexID id = vdmesh_->index2id(i);
            if (split_map_ != 0)
            {
                if (split_map_->find(id) == split_map_->end())
                {
                    continue;
                }
            }
            double importance = vdmesh_->vertex_importance(i);
            assert(importance > 0);
            if (mode_ != SilhouetteScreen || !vertex_in_silhouette(i))
            {
                id_queue_.push_back(VsInfo(id, importance, vdmesh_->id2level(id)));
            }
            else
            {
                silhouette_queue_.push_back(VsInfo(id, importance, vdmesh_->id2level(id)));
            }

            //For all vertices in silhouette, we include their invisible neighbors too
            if (vertex_in_silhouette(i))
            {
                std::vector<VertexIndex> neighbors;
                vdmesh_->vertex_vertices(i, neighbors);

                int importance = vdmesh_->vertex_importance(i);
                for (std::vector<VertexIndex>::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
                {
                    //consider the invisible neighbors of silhouette vertices.
                    if (! vdmesh_->vertex_is_visible(*it))
                    {
                        VertexID id_s = vdmesh_->index2id(*it);
                        if (split_map_ != 0)
                        {
                            if (split_map_ -> find(id_s) == split_map_->end())
                            {
                                continue;
                            }
                        }
                        vdmesh_->set_vertex_visibility(*it, true);
                        vdmesh_->set_vertex_importance(*it, importance);
                        if (mode_ != SilhouetteScreen)
                        {
                            id_queue_.push_back(VsInfo(id_s, importance, vdmesh_->id2level(id_s)));
                        }
                        else
                        {
                            silhouette_queue_.push_back(VsInfo(id_s, importance, vdmesh_->id2level(id_s)));
                        }
                    }
                }
            }
        }
    }
    //std::cerr<<"queue size "<<index_queue_.size()<<std::endl;
    if (mode_ == ScreenArea)
    {
        std::make_heap(id_queue_.begin(), id_queue_.end(), CompareArea());
    }
    else if (mode_ == SilhouetteScreen || mode_ == WeightedScreen)
    {
        std::make_heap(id_queue_.begin(), id_queue_.end(), CompareArea());
        std::make_heap(silhouette_queue_.begin(), silhouette_queue_.end(), CompareArea());
    }
    else if (mode_ == Level)
    {
        std::make_heap(id_queue_.begin(), id_queue_.end(), CompareLevel());
    }
    else if (mode_ == LevelArea)
    {
        std::make_heap(id_queue_.begin(), id_queue_.end(), CompareLevelArea());
    }
    else 
    {
        std::random_shuffle(id_queue_.begin(), id_queue_.end());
    }
}

void VertexPQ::push_update_heap()
{
    if (mode_ == Level)
    {
        std::push_heap(id_queue_.begin(), id_queue_.end(), CompareLevel());
    }
    else if (mode_ == LevelArea)
    {
        std::push_heap(id_queue_.begin(), id_queue_.end(), CompareLevelArea());
    }
    else if (mode_ == ScreenArea || mode_ == WeightedScreen)
    {
        std::push_heap(id_queue_.begin(), id_queue_.end(), CompareArea());
    }
}

void VertexPQ::pop_update_heap()
{
    if (mode_ == Level)
    {
        std::pop_heap(id_queue_.begin(), id_queue_.end(), CompareLevel());
    }
    else if (mode_ == LevelArea)
    {
        std::pop_heap(id_queue_.begin(), id_queue_.end(), CompareLevelArea());
    }
    else if (mode_ == ScreenArea || mode_ == WeightedScreen)
    {
        std::pop_heap(id_queue_.begin(), id_queue_.end(), CompareArea());
    }
}

VertexIndex VertexPQ::pop(bool push_children)
{
    VsInfo top(0, 0, 0);
    if (mode_ == SilhouetteScreen && !silhouette_queue_.empty())
    {
        std::pop_heap(silhouette_queue_.begin(), silhouette_queue_.end(), CompareArea());
        top = silhouette_queue_.back();
        silhouette_queue_.pop_back();
    }
    else if (!id_queue_.empty())
    {
        pop_update_heap();
        top = id_queue_.back();
        id_queue_.pop_back();
        //std::cerr << top << " " << index << " " << vdmesh_->vertex_importance(index) << std::endl;
    }
    if (push_children)
    {
        VsInfo child1((top.id) << 1, top.importance / 2, top.level + 1);
        VsInfo child2(((top.id) << 1) + 1, top.importance / 2, top.level + 1);
        if (split_map_ && split_map_->find(child1.id) != split_map_->end())
        {
            id_queue_.push_back(child1);
            push_update_heap();
            //std::cerr << "push " << child1.id << " importance " << child1.importance << std::endl;
        }
        if (split_map_ && split_map_->find(child2.id) != split_map_->end())
        {
            id_queue_.push_back(child2);
            push_update_heap();
            //std::cerr << "push " << child2.id << " importance " << child2.importance << std::endl;
        }
    }
    //std::cerr << "pop " << top.id << " importance " << top.importance << std::endl;
    return top.id;
}
