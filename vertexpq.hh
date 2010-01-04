#ifndef __VISIBLE_PQ_H__
#define __VISIBLE_PQ_H__
#include <queue>
#include <GL/glut.h>
#include <map>
#include "vdmesh.hh"


enum SelectMode {Level, LevelArea, ScreenArea, Random, WeightedScreen, SilhouetteScreen};

//typedef std::priority_queue<VertexID, std::vector<VertexID>, CompareArea>   PQ;

/**
 * the class to support sorting according to visual importance.
 */
class VertexPQ
{
public:
    /**
     * Constructor from pointers to ppmesh and gfmesh.
     */
    VertexPQ(Vdmesh *vdmesh, SelectMode mode, bool push_children, std::map<VertexID, BitString> *split_map = 0);

    /**
     * destructor.
     */
    virtual ~VertexPQ(void)
    {
    }

    /**
     * Update the visual contribution according to the content of frame buffer.
     * The content of frame buffer exists in pixels and the size indicates the 
     * size of frame buffer.
     */
    void update(unsigned char* pixels, size_t size, int width);



    /**
     * pop up the ID of vertex has the highest contribution.
     */
    VertexIndex pop(void);

    /**
     * To check whether a vertex is in the silhouette
     */
    bool vertex_in_silhouette(VertexIndex i)
    {
        if (silhouette_.find(i) != silhouette_.end())
            return true;
        else
            return false;
    }

    /**
     * To check whether a face is in the silhouette
     */
    bool face_in_silhouette(FaceIndex i)
    {
        if (face_in_silhouette_.find(i) != face_in_silhouette_.end())
            return true;
        else
            return false;
    }
    
    /**
     * To set the weight for faces in the silhouette
     */
    void set_silhouette_weight(double weight)
    {
        silhouette_weight_ = weight;
    }

private:
    void push(VertexID id);
    void stat_screen_area(unsigned char *pixels, size_t size);
    void push_update_heap();
    void pop_update_heap();

    struct VsInfo
    {
        VertexID id;
        double   importance;
        int      level;
        VsInfo(VertexID id_, double importance_, int level_)
        {
            id = id_;
            importance = importance_;
            level = level_;
        }
    };

    class CompareArea //: std::binary_function< VertexID, VertexID, bool>
    {
    public:
        bool operator()(VsInfo vs1, VsInfo vs2) const
        {
            // return vdmesh_->vertex_importance(index1) < vdmesh_->vertex_importance(index2);
            // Now we consider screen area first, and if screen area is the same we compare the level
            double metric1 = vs1.importance * 100 - vs1.level;
            double metric2 = vs2.importance * 100 - vs2.level;
            return metric1 < metric2;
        }
    };

    class CompareLevel //: std::binary_function< VertexID, VertexID, bool>
    {
    public:
        bool operator()(VsInfo vs1, VsInfo vs2) const
        {
            return vs1.level > vs2.level;
        }
    };

    class CompareLevelArea //: std::binary_function< VertexID, VertexID, bool>
    {
    public:
        bool operator()(VsInfo vs1, VsInfo vs2) const
        {
            return vs1.level * 250000 - vs1.importance   >  vs2.level * 250000 - vs2.importance;
        }
    };

    /**
     * Find the faces contribute to silhouette of a rendered image
     */
    void find_silhouette(unsigned char *pixels, size_t size, int width);
private:
    Vdmesh *vdmesh_;
    SelectMode mode_;
    double silhouette_weight_;
    std::set<VertexIndex> silhouette_;
    std::set<FaceIndex>   face_in_silhouette_;
    std::map<VertexID, BitString> *split_map_;
    //std::vector<VertexIndex> index_queue_;
    std::vector<VsInfo>    id_queue_;
    //std::vector<VertexIndex> silhouette_queue_;
    std::vector<VsInfo>    silhouette_queue_;
    std::vector<int>       seq_array_;
    bool   push_children_;
};
#endif
