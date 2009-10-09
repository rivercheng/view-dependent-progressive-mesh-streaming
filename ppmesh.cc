#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include "ppmesh.hh"
#include "huffman.hh"
static const unsigned int MAX_NEIGHBORS = 1000;

//Dequantize the difference
static double de_quantize_d(int value, double max, double min, unsigned int quantize_bit = 12)
{
    unsigned int steps;
    static unsigned int step_array[17]=\
                                       {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536};
    if (quantize_bit <= 16)
    {
        steps = step_array[quantize_bit];
    }
    else
    {
        steps = static_cast<unsigned int> (pow(2, quantize_bit));
    }
    double d_val = 0;
    d_val = value * 1.0 * (max - min) / steps;
    return d_val;
}

Ppmesh::Ppmesh(std::istream& ifs, int quantize_bits)
        :n_base_vertices_(0), n_base_faces_(0), n_detail_vertices_(0),\
        n_max_vertices_(0), tree_bits_(0), levels_(0), \
        quantize_bits_(quantize_bits), level0_(0), level1_(9),\
        x_min_(0), y_min_(0), z_min_(0), x_max_(0), y_max_(0), z_max_(0),\
        id_coder_(0),\
        geometry_coder1_(0), geometry_coder2_(0) 
{
    assert(ifs);
    readBase(ifs);
}

Ppmesh::~Ppmesh()
{
        if (id_coder_) delete id_coder_;
        if (geometry_coder1_) delete geometry_coder1_;
        if (geometry_coder2_) delete geometry_coder2_;
}

void
Ppmesh::readBase(std::istream& ifs)
{
    MyMesh::Point  p;
    unsigned int   i, i0, i1, i2;
    char           c[10];

    bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;
    bool is64bit = (sizeof(size_t) == 8);

    // read header
    ifs.read(c, 8);
    c[8] = '\0';
    if (std::string(c) != std::string("ProgMesh"))
    {
        throw WrongFileFormat();
    }

    size_t ignore;
    OpenMesh::IO::binary<size_t>::restore( ifs, n_base_vertices_,   swap );
    if (!is64bit)
    {
        OpenMesh::IO::binary<size_t>::restore(ifs, ignore, swap);
    }
    OpenMesh::IO::binary<size_t>::restore( ifs, n_base_faces_,      swap );
    if (!is64bit)
    {
        OpenMesh::IO::binary<size_t>::restore(ifs, ignore, swap);
    }
    OpenMesh::IO::binary<size_t>::restore( ifs, n_detail_vertices_, swap );
    if (!is64bit)
    {
        OpenMesh::IO::binary<size_t>::restore(ifs, ignore, swap);
    }

    n_max_vertices_    = n_base_vertices_ + n_detail_vertices_;


    tree_bits_ = (size_t)ceil(log(n_base_vertices_)/log(2.));
    unsigned int leading_one = 1<<tree_bits_;

    // load base mesh
    mesh_.clear();

    for (i=0; i<n_base_vertices_; ++i)
    {
        MyMesh::VertexHandle v;
        OpenMesh::IO::binary<MyMesh::Point>::restore( ifs, p, swap );
        v = mesh_.add_vertex(p);
        VertexID id = i + leading_one;
        mesh_.deref(v).id = id;
        mesh_.deref(v).level = 0;
        map_[id].v = v;
    }


    for (i=0; i<n_base_faces_; ++i)
    {
        OpenMesh::IO::binary<unsigned int>::restore( ifs, i0, swap);
        OpenMesh::IO::binary<unsigned int>::restore( ifs, i1, swap);
        OpenMesh::IO::binary<unsigned int>::restore( ifs, i2, swap);
        mesh_.add_face(mesh_.vertex_handle(i0),
                       mesh_.vertex_handle(i1),
                       mesh_.vertex_handle(i2));
    }
    return;
}

unsigned int Ppmesh::id2level(VertexID id) const
{
    const VertexID mask = 0x80000000;
    size_t len = sizeof(VertexID)*8;
    while ((mask & id) == 0)
    {
        id <<= 1;
        len --;
    }
    if (len < tree_bits_)
    {
        std::cerr<<"Wrong id."<<std::endl;
        return 0;
    }
    return (len-tree_bits_);
}

bool    Ppmesh::decode(VertexID id, const BitString& data, size_t* p_pos, bool temp)
{
    unsigned int level= id2level(id);
    
    //The first bit is the flag. If '1' then it is a leaf.
    if (level >= minimum_depth_ && data.test(*p_pos))
    {
        (*p_pos)++;
        VertexID parent_id = (id >> 1);
        if (map_.find(parent_id) != map_.end() &&!map_[parent_id].isLeaf)
        {
            map_[id].isLeaf = true;
        }
        return false;
    }
    else if (level >= minimum_depth_)
    {
        (*p_pos)++;
    }

    std::vector<unsigned int> value_array;
    size_t number = 0;
    number = id_coder_->decode(data, value_array, p_pos, 2);
    assert(number == 2);
    unsigned int code_l = value_array[0];
    unsigned int code_r = value_array[1];

    //DEBUG("code decoded");
    int dx = 0;
    int dy = 0;
    int dz = 0;
    // level < level0_, dx, dy, dz are not compressed
    // level0_ <= level < level1_, dx, dy, dz are compressed with the first huffman table
    // level >= level1_, dx, dy, dz are compressed with the second huffman table
    if (level < level0_)
    {
        BitString bs1 = data.substr(*p_pos, 12);
        dx = static_cast<int>(bs1.to_ulong());
        (*p_pos) += 12;
        BitString bs2 = data.substr(*p_pos, 12);
        dy = static_cast<int>(bs2.to_ulong());
        (*p_pos) += 12;
        BitString bs3 = data.substr(*p_pos, 12);
        dz = static_cast<int>(bs3.to_ulong());
        (*p_pos) += 12;
    }
    else
    {
        std::vector<int> geometry_array;
        Huffman::HuffmanCoder<int> * coder = NULL;
        if (level < level1_)
        {
            coder = geometry_coder1_;
        }
        else
        {
            coder = geometry_coder2_;
        }
        //for debug
        number = 0;
        number = coder->decode(data, geometry_array, p_pos, 3);
        assert(number == 3);
        dx = geometry_array[0];
        dy = geometry_array[1];
        dz = geometry_array[2];
    }

    //DEBUG("geometry decoded");
    splitInfo* split = new splitInfo();
    split->id = id;
    split->code_l = code_l;
    split->code_r = code_r;
    split->dx = dx;
    split->dy = dy;
    split->dz = dz;

    //if the parent vertex is not split yet, then this vertex does not exist.
    //We put this vertex split in the waiting list of its parent vertex
    if (map_.find(id) == map_.end()) 
    {
        map_[id>>1].waiting_list.push_back(split);
        return false;
    }
    else
    {
        bool result = splitVs(split, temp);
        if (result)
        {
            //update the patches
        }
        return result;
    }
}
    
void    Ppmesh::output_arrays(std::vector<Vertex>& vertex_array, std::vector<Face>& face_array) const
{
        MyMesh::ConstVertexIter v_it(mesh_.vertices_begin());
        MyMesh::ConstVertexIter v_end(mesh_.vertices_end());
        for (; v_it != v_end; ++v_it)
        {
            MyMesh::Point p = mesh_.point(v_it);
            Vertex v(p[0], p[1], p[2]);
            vertex_array.push_back(v);
        }

        MyMesh::ConstFaceIter f_it(mesh_.faces_begin());
        MyMesh::ConstFaceIter f_end(mesh_.faces_end());
        for ( ; f_it != f_end; ++f_it)
        {
            MyMesh::ConstFaceVertexIter fv_it(mesh_, f_it.handle());
            VertexIndex v_idx1 = (fv_it.handle()).idx();
            VertexIndex v_idx2 = ((++fv_it).handle()).idx();
            VertexIndex v_idx3 = ((++fv_it).handle()).idx();
            Face f(v_idx1, v_idx2, v_idx3);
            face_array.push_back(f);
        }
        return;
}

void   Ppmesh::updated_info(std::vector<Vertex>& vertices, std::vector<Face>& faces, std::set<VertexIndex>& vertex_set, std::set<FaceIndex> face_set)
{
    vertices = new_vertices_;
    faces    = new_faces_;
    face_set = affected_faces_;
    
    //set of vertices in affected_faces_
    std::set<FaceIndex>::const_iterator it = affected_faces_.begin();
    std::set<FaceIndex>::const_iterator end = affected_faces_.end();
    for(; it != end; ++it)
    {
        MyMesh::FaceHandle fh(*it);
        MyMesh::ConstFaceVertexIter fv_it(mesh_, fh);
        while(fv_it)
        {
            affected_vertices_.insert(static_cast<VertexIndex>(fv_it.handle().idx()));
        }
    }
    vertex_set = affected_vertices_;

    new_vertices_.clear();
    new_faces_.clear();
    affected_vertices_.clear();
    affected_faces_.clear();
}

bool Ppmesh::splitVs(splitInfo* split, bool temp)
{
    //std::cerr<<"split "<<split->id<<std::endl;
    //MyMesh::VertexHandle v1 = split->v;
    MyMesh::VertexHandle v1 = map_[split->id].v;
    //std::cerr<<mesh_.deref(v1).id<<" "<<split->id<<std::endl;
    assert(mesh_.deref(v1).id == split->id);

    MyMesh::Point p1 = mesh_.point(v1);
    double x1 = p1[0];
    double y1 = p1[1];
    double z1 = p1[2];

    std::vector<VertexID> neighbors;
    one_ring_neighbor(v1, neighbors);

    unsigned int pos = 0;
    std::vector<VertexID> results;
    MyMesh::VertexHandle vl;
    MyMesh::VertexHandle vr;
    unsigned int         code_l = split->code_l;
    unsigned int         code_r = split->code_r;
    VertexID             id     = split->id;
    VertexID             id_l   = 0;
    VertexID             id_r   = 0;
    VertexID             o_id_l = 0;
    VertexID             o_id_r = 0;
    VsInfo&              vsinfo = map_[id];
    
    if (code_l != 0)
    {
        unsigned int code_remain = 1;
        pos = code2id(neighbors, code_l, results, &code_remain);
        if (results.size() > 1)
        {
            o_id_l = further_split(results, id, pos, LEFT, temp);
        }
        else
        {
            o_id_l = results[0];
        }
        assert(results.size() == 1);
        if (results.size() == 0)
        {
            throw DecodeError();
        }
        id_l = results[0];
        results.clear();
        vl = map_[id_l].v;
        vsinfo.id_l = o_id_l;
        vsinfo.code_remain_l = code_remain;
    }
    //else it means no vl exist.
    //we just keep vl as the default value(-1).

    if (code_r != 0)
    {
        unsigned int code_remain = 1;
        pos = code2id(neighbors, code_r, results, &code_remain);
        if (results.size() > 1)
        {
            o_id_r = further_split(results, id, pos, RIGHT, temp);
        }
        else
        {
            o_id_r = results[0];
        }
        assert(results.size() == 1);
        if (results.size() == 0)
        {
            throw DecodeError();
        }
        id_r = results[0];
        results.clear();
        vr = map_[id_r].v;
        vsinfo.id_r = o_id_r;
        vsinfo.code_remain_r = code_remain;

    }

    if (id_l == id_r)
    {
        //We have to delay the split since the two neighbors are currently one same vertex.
        if (!temp)
        {
            map_[id_r].waiting_list.push_back(split);
            to_be_split_.insert(id_r);
        }
        return false;
    }
    double x0 = x1 + de_quantize_d(split->dx, x_max_, x_min_, quantize_bits_);
    double y0 = y1 + de_quantize_d(split->dy, y_max_, y_min_, quantize_bits_);
    double z0 = z1 + de_quantize_d(split->dz, z_max_, z_min_, quantize_bits_);

    MyMesh::Point p0(x0, y0, z0);
    MyMesh::VertexHandle v0 = mesh_.add_vertex(p0);

    unsigned int old_face_number = mesh_.n_faces();
    mesh_.vertex_split(v0, v1, vl, vr);
    unsigned int curr_face_number = mesh_.n_faces();
    
    delete split;
    split = 0;
    
    VertexID id0 = (id << 1) + 1;
    VertexID id1 = (id << 1);
    mesh_.deref(v0).id = id0;
    mesh_.deref(v1).id = id1;
    map_[id0].v = v0;
    map_[id1].v = v1;

    //collect the information of which vertices and faces are added.
    Vertex v(x0, y0, z0);
    new_vertices_.push_back(v);

    MyMesh::ConstVertexFaceIter vf_it(mesh_, v0);
    while(vf_it)
    {
        affected_faces_.insert(static_cast<FaceIndex>(vf_it.handle().idx()));
    }

    for (unsigned int i = old_face_number; i < curr_face_number; i++)
    {
        MyMesh::FaceHandle fh(i);
        MyMesh::ConstFaceVertexIter fv_it(mesh_, fh);
        VertexIndex fv[3] = {0, 0, 0};
        int         index = 0;
        while(fv_it)
        {
            fv[index] = static_cast<VertexIndex>(fv_it.handle().idx());
        }
        Face f(fv[0], fv[1], fv[2]);
        new_faces_.push_back(f);
    }
    if (!temp)
    {
        //split the vertice splits in the waiting list
        for (size_t i = 0; i < vsinfo.waiting_list.size(); i++)
        {
            splitVs(vsinfo.waiting_list[i]);
        }
        vsinfo.waiting_list.clear();
    }
    return true;
}

size_t Ppmesh::one_ring_neighbor(const MyMesh::VertexHandle& v1, std::vector<VertexID>& neighbors) const
{
    size_t neighbor_number = 0;
    MyMesh::ConstVertexVertexIter vv_it(mesh_, v1);
    while (vv_it)
    {
        neighbors.push_back(vv_it->id);
        ++vv_it;
        neighbor_number++;
    }
    return neighbor_number;
}

size_t Ppmesh::code2id(const std::vector<VertexID>&id_array, unsigned int code, std::vector<VertexID>& result_array, unsigned int* p_code_remain, size_t pos) const
{
    std::bitset<sizeof(unsigned int) * 8> bs(code);
    std::string str(bs.to_string());
    std::string::size_type loc = str.find('1',0);
    if (loc==std::string::npos)
    {
        std::cerr<<"no 1 exists in the id."<<bs<<std::endl;
        throw InvalidID();
    }
    str = str.substr(loc+1);

    //We devide the neighbors to two groups based on the value of
    //i-th bit, and then choose one group according to the input code.
    //If all neighbors go to one group, we just continue.
    size_t n  = 0;
    //to improve speed, we avoid using std::vector here
    //So we convert result_array to a real array
    
    const VertexID *real_array = &(id_array[0]);
    size_t real_size = id_array.size();
    assert(real_size <= MAX_NEIGHBORS);
    
    VertexID id_array_1[MAX_NEIGHBORS];
    VertexID id_array_2[MAX_NEIGHBORS];
    
    //result_array = id_array;
    size_t n1 = 0;
    size_t n2 = 0;
    for (; pos < sizeof(VertexID)*8; pos++)
    {
        n1 = 0;
        n2 = 0;
        for (size_t i = 0; i < real_size; i++)
        {
            if (idIsSet(real_array[i], pos))
            {
                id_array_2[n2] = real_array[i];
                n2++;
            }
            else
            {
                id_array_1[n1] = real_array[i];
                n1++;
            }
        }
        if (n1>0 && n2>0)
        {
            if (n == str.size())
            {
                pos--;
                break;
            }
            //DEBUG(str[n]);
            if (str[n] == '0')
            {
                real_array = id_array_1;
                real_size = n1;
            }
            else
            {
                real_array = id_array_2;
                real_size = n2;
            }
            n++;
        }
        if (real_size == 1)
        {
            break;
        }
    }


    result_array.resize(real_size);
    if (real_size > n1 && real_size > n2)
    {
        assert(real_size == n1 + n2);
        for (size_t i = 0; i < n1; i++)
        {
            result_array[i] = id_array_1[i];
        }
        for (size_t i = 0; i < n2; i++)
        {
            result_array[n1+i] = id_array_2[i];
        }
    }
    else
    {
        for (size_t i = 0 ; i < real_size; i++)
        {
            result_array[i] = real_array[i];
        }
    }

    //push the rest code into code_remain. The last bit be put first.
    size_t i = str.size() - 1;
    while (n < str.size())
    {
        (*p_code_remain) <<= 1;
        if (str[i] == '1')
        {
            (*p_code_remain) +=  1;
        }
        n++;
        i--;
    }
    return pos;
}

VertexID Ppmesh::further_split(std::vector<VertexID>& neighbors, VertexID id, size_t pos,Side side, bool temp)
{

    //change the id to id string. (find the leading 1 and remove it)
    std::bitset<sizeof(unsigned int) * 8> bs(id);
    std::string str(bs.to_string());
    std::string::size_type loc = str.find('1',0);
    if (loc==std::string::npos)
    {
        std::cerr<<"no 1 exists in the id."<<bs<<std::endl;
        throw InvalidID();
    }
    str = str.substr(loc+1);


    VertexID o_parent = 0;
    while (neighbors.size() > 1)
    {
        //find the common parent of all the vertices in the neighbors list.
        size_t len = len_of_id(neighbors[0]) - pos - 1;
        VertexID mask = 0xffffffff << len;
        VertexID parent = (mask & neighbors[0]);
        parent >>= len;
        if (o_parent == 0) o_parent = parent;

        //according to the id_l or id_r of this parent, we collect the code to
        //guide the further split. (The code is in the reverse order.)
        VsInfo& vsinfo = map_[parent];
        VertexID  id_l = vsinfo.id_l;
        VertexID  id_r = vsinfo.id_r;
        unsigned int code = 1;
        unsigned int bit  = 0;
        VertexID     id_t = id;
        if (id_l > id_t)
        {
            while (id_l > id_t) id_l >>= 1;
        }
        else
        {
            while (id_l < id_t) id_t >>= 1;
        }
        if (id_l == id_t)
        {
            bit = (vsinfo.code_remain_l & 0x1);
            code <<= 1;
            if (side == LEFT)
            {
                code += bit;
            }
            else
            {
                code += (1-bit);
            }
            if (!temp)
            {
                vsinfo.code_remain_l >>= 1;
            }
        }
        else
        {
            id_t = id;
            if (id_r > id_t)
            {
                while (id_r > id_t) id_r >>=1;
            }
            else
            {
                while (id_r < id_t) id_t >>=1;
            }
            if (id_r == id_t)
            {
                bit = (vsinfo.code_remain_r & 0x1);
                code <<= 1;
                if (side == LEFT)
                {
                    code += (1-bit);
                }
                else
                {
                    code += bit;
                }
                if (!temp)
                {
                    vsinfo.code_remain_r >>= 1;
                }
            }
            else
            {
                std::cerr<<"id "<<id<<" id_l "<<id_l<<" id_r "<<id_r<<std::endl;
                std::cerr<<"decode error"<<std::endl;
                throw DecodeError();
            }
        }
        unsigned int remain = 0;
        pos = code2id(neighbors, code, neighbors, &remain, pos+1);
    }
    return o_parent;
}
