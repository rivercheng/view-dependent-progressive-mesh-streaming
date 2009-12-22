#ifndef __VISIBLE_PQ_H__
#define __VISIBLE_PQ_H__
#include <queue>
#include <GL/glut.h>
#include <map>
#include "vdmesh.hh"


enum SelectMode {Level, ScreenArea, Random};

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
    VertexPQ(Vdmesh *vdmesh, SelectMode mode, std::map<VertexID, BitString> *split_map = 0);

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
    void update(unsigned char* pixels, size_t size);

    /**
     * pop up the ID of vertex has the highest contribution.
     */
    VertexIndex pop(void);

private:
    void push(VertexID id);
    void stat_screen_area(unsigned char *pixels, size_t size);

    class CompareArea //: std::binary_function< VertexID, VertexID, bool>
    {
    public:
        CompareArea(Vdmesh  *vdmesh)
        {
            vdmesh_ = vdmesh;
        }
        bool operator()(VertexIndex index1, VertexIndex index2) const
        {
            return vdmesh_->vertex_importance(index1) < vdmesh_->vertex_importance(index2);
        }
    private:
        Vdmesh *vdmesh_;
    };

    class CompareLevel //: std::binary_function< VertexID, VertexID, bool>
    {
    public:
        CompareLevel(Vdmesh  *vdmesh)
        {
            vdmesh_ = vdmesh;
        }
        bool operator()(VertexIndex index1, VertexIndex index2) const
        {
            return vdmesh_->id2level(vdmesh_->index2id(index1)) > vdmesh_->id2level(vdmesh_->index2id(index2));
        }
    private:
        Vdmesh *vdmesh_;
    };



private:
    Vdmesh *vdmesh_;
    SelectMode mode_;
    std::map<VertexID, BitString> *split_map_;
    std::vector<VertexIndex> index_queue_;
};
#endif
