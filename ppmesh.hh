//=============================================================================
//Written by Cheng Wei
//rivercheng@gmail.com
//8 Oct 2009 
//=============================================================================
#ifndef __PPMESH_HH__
#define __PPMESH_HH__

#include <vector>
#include <set>
#include <OpenMesh/Core/Mesh/Types/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Attributes/Attributes.hh>
#include "common_def.hh"
#include "bitstring.hh"
#include "huffman.hh"
#include "vertexid.hh"

using namespace OpenMesh;
using namespace OpenMesh::Attributes;

class BitString;
class Ppmesh
{
    public:
    Ppmesh(std::istream& ifs, int quantize_bits = 14);
    virtual ~Ppmesh(void);
    
    
    /**
     * to decode a vertex id from given position of a given bitstring, 
     * In most cases, please let temp be false.
     * Return ture if this vertex split can be done. return false
     * if this vertex split has to wait for parents. It will be done
     * immediately when the parent vertex split is done.
     * The pos will be updated to next unvisited bit in the data.
     */
    bool    decode(VertexID id, const BitString& data, size_t* p_pos, bool temp = false);
    
    /**
     * output all the vertices connected to a given vertex (one-ring neighbor).
     */
    void    vertex_vertices(VertexIndex vertex_index, std::vector<VertexIndex>& vertex_array) const;
    
    /**
     * output the vertices within a given face to vertex_array.
     */
    void    face_vertices(FaceIndex face_index, std::vector<VertexIndex>& vertex_array) const;
    
    /**
     * output the neighbor faces of the given vertex to face_array.
     */
    void    vertex_faces(VertexIndex vertex_index, std::vector<FaceIndex>& face_array) const;


    /**
     * output all the vertices and faces in the ppmesh to vertex_array and face_array.
     */
    void    output_arrays(std::vector<Vertex>& vertex_array, std::vector<Face>& face_array) const;

    /**
     * Return new vertices, new faces, affected vertices, and affected faces after last updated_info.
     */
    void    updated_info(std::vector<Vertex>& vertices, std::vector<Face>& faces, std::set<VertexIndex>& vertex_set, std::set<FaceIndex> face_set);


    private: //parameters
    struct MyTraits : public OpenMesh::DefaultTraits
    {
    VertexAttributes  ( OpenMesh::Attributes::Normal       |
                        OpenMesh::Attributes::Status       );
    EdgeAttributes    ( OpenMesh::Attributes::Status       );
    HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
    FaceAttributes    ( OpenMesh::Attributes::Normal       |
                        OpenMesh::Attributes::Status       );
    VertexTraits
    {
        public:
        //the id to represent the position in the binary trees.
        unsigned int id;
        unsigned int level;
    };
    FaceTraits
    {
        public:
        unsigned int s_area;
    };

    };

    typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;
    //Ppmesh is not copiable.
    Ppmesh(const Ppmesh&);
    Ppmesh& operator=(const Ppmesh&);

    enum Side {LEFT, RIGHT};
    class InvalidID {};
    class DecodeError {};
    class NoDecoder {};
    class WrongFileFormat{};

    struct splitInfo
    {
        MyMesh::VertexHandle        v;
        VertexID                   id;
        unsigned int           code_l;
        unsigned int           code_r;
        int                        dx;
        int                        dy;
        int                        dz;
        double                     x1;
        double                     y1;
        double                     z1;
    };
    
    struct VsInfo
    {
        MyMesh::VertexHandle       v;
        VertexID                id_l;
        VertexID                id_r;
        unsigned int            code_remain_l;
        unsigned int            code_remain_r;
        std::vector<splitInfo*> waiting_list;
        bool                    isLeaf;
        VsInfo()
                :id_l(0), id_r(0), code_remain_l(1), code_remain_r(1), \
                isLeaf(false)
        {
            ;
        }
        ~VsInfo()
        {
            for (size_t i = 0; i<waiting_list.size(); i++)
            {
                delete waiting_list[i];
                waiting_list[i] = 0;
            }
        }
    };
    typedef std::map<VertexID, VsInfo>          Map;
    typedef Map::iterator                       MapIter;
    typedef Map::const_iterator                 MapConstIter;
    
    Map               map_;
    MyMesh            mesh_;
    
    size_t            n_base_vertices_, n_base_faces_, n_detail_vertices_;
    size_t            n_max_vertices_;
    unsigned int      tree_bits_;
    unsigned int      levels_;
    unsigned int      minimum_depth_;
    unsigned int      quantize_bits_;
    unsigned int      level0_;
    unsigned int      level1_;
    double            x_min_;
    double            y_min_;
    double            z_min_;
    double            x_max_;
    double            y_max_;
    double            z_max_;
    
    Huffman::HuffmanCoder<unsigned int>*  id_coder_;
    Huffman::HuffmanCoder<int>         *  geometry_coder1_;
    Huffman::HuffmanCoder<int>         *  geometry_coder2_;
    std::set<VertexID>                 to_be_split_;

    VertexIndex                        last_vertex_index_;
    FaceIndex                          last_face_index_;
    std::vector<Vertex>                new_vertices_;
    std::vector<Face>                  new_faces_;
    std::set<VertexIndex>              affected_vertices_;
    std::set<FaceIndex>                affected_faces_;
    
    private: //functions
    void    readBase(std::istream& ifs);
    unsigned int id2level(VertexID id) const;
    bool         splitVs(splitInfo* split, bool temp=false);
    size_t       one_ring_neighbor(const MyMesh::VertexHandle& v1, std::vector<VertexID>& neighbors) const;
    size_t       code2id(const std::vector<VertexID>&id_array, unsigned int code, std::vector<VertexID>& result_array, unsigned int* p_code_remain, size_t pos=0) const;
    VertexID     further_split(std::vector<VertexID>& neighbors, VertexID id, size_t pos, Side side, bool temp = false);
};
#endif
