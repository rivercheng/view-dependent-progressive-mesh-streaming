// =============================================================================
// Written by Cheng Wei
// rivercheng@gmail.com
// 8 Oct 2009
// =============================================================================
#ifndef __PPMESH_HH__
#define __PPMESH_HH__

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include <vector>
#include <set>
#include <map>
#include "common_def.hh"
#include "bitstring.hh"
#include "huffman.hh"
#include "vertexid.hh"

class BitString;
class Ppmesh
{
// ============================INTERFACE====================================
    public:
    /**
     * to create a ppmesh by reading data from ifs.
     */
    Ppmesh(std::istream& ifs, int quantize_bits = 14);

    /**
     * This construction function including assigning report arrays.
     * These reporting arrays will be appended with the newly generated vertices,
     * faces, affected vertices, and affected faces.
     */
    Ppmesh(std::istream& ifs, \
           std::vector<Vertex>&               new_vertices_, \
           std::vector<Face>&                 new_faces_, \
           std::set<VertexIndex>&             affected_vertex_indices_, \
           std::map<FaceIndex, Face>&         affected_faces_, \
           int quantize_bits = 14);

    /**
     * The copy construction function is allowed. 
     * NOTICE: The reporting arrays of the new ppmesh will be set to 0.
     * Coping from the old ppmesh does not make sense in many cases.
     * If they are needed, use set_report_arrays to set.
     */
    Ppmesh(const Ppmesh&);

    /**
     * Set the reporting arrays which will be appended with the newly generated vertices,
     * faces, affected vertices, and affected faces.
     */
    void set_report_arrays(
           std::vector<Vertex>&               new_vertices_, \
           std::vector<Face>&                 new_faces_, \
           std::set<VertexIndex>&             affected_vertex_indices_, \
           std::map<FaceIndex, Face>&         affected_faces_);
    
    virtual ~Ppmesh(void);

    /**
     * to decode a vertex id from given position of a given bitstring, 
     * Return ture if this vertex split can be done. return false
     * if this vertex split has to wait for parents. It will be done
     * immediately when the parent vertex split is done.
     * The pos will be updated to next unvisited bit in the data.
     */
    bool decode(VertexID id, const BitString& data, \
                size_t* p_pos);

    /**
     * output all the vertices connected to a given vertex (one-ring neighbor).
     */
    void vertex_vertices(VertexIndex vertex_index, \
                         std::vector<VertexIndex>& vertex_array) const
    {
        MyMesh::VertexHandle v(vertex_index);
        MyMesh::ConstVertexVertexIter vv_it(mesh_, v);
        while (vv_it)
        {
            vertex_array.push_back(vv_it.handle().idx());
            ++vv_it;
        }
        return;
    }

    /**
     * output the vertices within a given face to vertex_array.
     */
    void face_vertices(FaceIndex face_index, \
                       std::vector<VertexIndex>& vertex_array) const;

    /**
     * output the neighbor faces of the given vertex to face_array.
     */
    void vertex_faces(VertexIndex vertex_index, \
                      std::vector<FaceIndex>& face_array) const;

    /**
     * output all the vertices and faces in the ppmesh to vertex_array and face_array.
     */
    void output_arrays(std::vector<Vertex>& vertex_array, \
                       std::vector<Face>& face_array) const;

    /**
     * output the number of details
     */
    size_t n_detail_vertices(void)
    {
        return n_detail_vertices_;
    }

    /**
     * Deduce the level of the vertex from its ID.
     */
    unsigned int id2level(VertexID id) const;

    /**
     * Deduce the ID of a vertex from its Index.
     */
    VertexID index2id(VertexIndex index) const;

    /**
     * Deduce the index of a vertex from its ID.
     */
    VertexIndex id2index(VertexID id) const;

    /**
     * Return a vertex id that required to be split for splitting pending splits.
     */
    VertexID to_be_split(void);

// =======================================PRIVATE PART==================================//
    private:  // private types

    // So the vertices include two extra attributes: id and level.
    struct MyTraits : public OpenMesh::DefaultTraits
    {
        VertexAttributes(OpenMesh::Attributes::Normal | \
                         OpenMesh::Attributes::Status);
        EdgeAttributes(OpenMesh::Attributes::Status);
        HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
        FaceAttributes(OpenMesh::Attributes::Normal | \
                       OpenMesh::Attributes::Status);
        VertexTraits
        {
            public:
            // the id to represent the position in the binary trees.
            unsigned int id;
            unsigned int level;
        };
    };

    typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;

    enum Side {LEFT, RIGHT};

    // Exceptions
    class InvalidID {};
    class DecodeError {};
    class NoDecoder {};
    class WrongFileFormat {};

    // To store the vertex splits to be decoded.
    struct splitInfo
    {
        VertexID                   id;
        unsigned int           code_l;
        unsigned int           code_r;
        int                        dx;
        int                        dy;
        int                        dz;
        Coordinate                 x1;
        Coordinate                 y1;
        Coordinate                 z1;
    };

    // To store the vertex splits already applied
    struct VsInfo
    {
        VertexIndex                v;
        VertexID                id_l;
        VertexID                id_r;
        unsigned int            code_remain_l;
        unsigned int            code_remain_r;
        std::vector<splitInfo>  waiting_list;
        bool                    isLeaf;
        VsInfo()
                :id_l(0), id_r(0), \
                 code_remain_l(1), code_remain_r(1), \
                 isLeaf(false)
        {}
    };

    typedef std::map<VertexID, VsInfo> Map;
    typedef Map::iterator              MapIter;
    typedef Map::const_iterator        MapConstIter;

    private:  // member variables
    MyMesh            mesh_;
    Map               map_;

    size_t            n_base_vertices_; 
    size_t            n_base_faces_; 
    size_t            n_detail_vertices_;
    size_t            n_max_vertices_;
    unsigned int      tree_bits_;
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

    Huffman::DecodeTree<unsigned int>   id_tree_;
    bool                                tree1Exist_;
    bool                                tree2Exist_;
    Huffman::DecodeTree<int>            geometry_tree1_;
    Huffman::DecodeTree<int>            geometry_tree2_;

    Huffman::HuffmanCoder<unsigned int> *id_coder_;
    Huffman::HuffmanCoder<int>          *geometry_coder1_;
    Huffman::HuffmanCoder<int>          *geometry_coder2_;
    
    std::set<VertexID>                   to_be_split_;

    // The collections of new vertices, new faces, 
    // affected faces, and affected vertices.
    // These collections are initiated in the constructor and 
    // passed by the parameters in the constructor function.
    std::vector<Vertex>               *new_vertices_;
    std::vector<Face>                 *new_faces_;
    std::set<VertexIndex>             *affected_vertex_indices_;
    std::map<FaceIndex, Face>         *affected_faces_;
    
    private:  // private member functions

    //assignment is not allowed. Please use copy construction function instead.
    Ppmesh& operator=(const Ppmesh&);
    
    // read base file from ifs
    void         readBase(std::istream& ifs);

    // to split a vertex split. Return true if it is split.
    // Return false if it is buffered.
    bool         splitVs(splitInfo split);

    // return the number of one ring neighbors.
    size_t       one_ring_neighbor(const MyMesh::VertexHandle& v1, \
                                   std::vector<VertexID>& neighbors) const;
    // decode the correct id number from the code.
    size_t       code2id(const std::vector<VertexID>&id_array, \
                         unsigned int code, \
                         std::vector<VertexID>& result_array, \
                         unsigned int* p_code_remain, \
                         size_t pos=0) const;

    // split the neighbors until the correct V_l or V_r is found.
    VertexID     further_split(std::vector<VertexID>& neighbors, \
                               VertexID id, size_t pos, \
                               Side side);

    // read base mesh data. Base mesh data is the beginning part of base file.
    void         read_base_mesh(std::istream& ifs);

    // read PM data. PM data is part of base mesh data.
    void         readPM(std::istream& ifs);

    // helper function. Generate a Face data from a face handle.
    Face         fh_2_face(MyMesh::FaceHandle fh);

    // helper function. Copy data from MyMesh object. It is not directly supported by OpenMesh.
    void         copyMesh(MyMesh& dest) const;

    // read huffman coding tree for the base file.
    template <typename T>
    void read_huffman_tree(std::istream& ifs, Huffman::DecodeTree<T>& tree)
    {
        bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;

        unsigned int tree_str_len;
        OpenMesh::IO::binary<unsigned int>::restore(ifs, tree_str_len, swap);
        tree.tree_str.read_binary(ifs, tree_str_len);

        unsigned int value_array_len;
        OpenMesh::IO::binary<unsigned int>::restore(ifs, value_array_len, swap);
        for (size_t i = 0; i < value_array_len; ++i)
        {
            T value;
            OpenMesh::IO::binary<T>::restore(ifs, value, swap);
            tree.value_array.push_back(value);
        }
    }
};
#endif // __PPMESH_HH__
