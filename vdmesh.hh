// =============================================================================
// Written by Cheng Wei
// rivercheng@gmail.com
// 2 Nov 2009
// =============================================================================
#ifndef __VDMESH_HH__
#define __VDMESH_HH__
class gfmesh;
class PacketID;
class Vdmesh
{
    public:
        /**
         * Construct a vdmesh by reading ifs.
         */
        Vdmesh(std::istream& ifs);

        virtual ~Vdmesh(void);

        /**
         * Test the visibility of a vertex
         */
        bool isVisible(VertexIndex index);

        /**
         * Set the visibility of a vertex
         */
        void setVisibility(VertexIndex index, bool visible);

        /**
         * Select n vertices to be split
         * used in per vertex request.
         */
        int selectVertices(int n, std::vector<VertexID>& vertices);

        /**
         * Output the importance to split a vertex
         */
        void setImportance(vertexIndex index, int importance);

        /**
         * Decode all vertex splits encoded in data.
         */
        bool decode(VertexID id, const BitString& data, size_t* p_pos);

    private:
        //sort the vertices according to their importance.
        void sort(void);

    private:
        std::vector<VertexID> vertex_front_;
        std::vector<int>      importance_array_;
        std::vector<bool>     visibility_array_;
        Gfmesh                *gfmesh_;
}
#endif //__VDMESH_HH__
