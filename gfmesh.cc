#include "gfmesh.hh"
#include "ppmesh.hh"
Gfmesh::Gfmesh(std::istream ifs)
{
    ppmesh_ = new Ppmesh(ifs);
    vertex_array_.reserve(RESERVE_SIZE);
    face_array_.reserve(2*RESERVE_SIZE);
    vertex_normal_array_.reserve(RESERVE_SIZE);
    face_normal_array_.reserve(2*RESERVE_SIZE);

    ppmesh_->output_arrays(vertex_array_, face_array_);
    for (size_t i = 0; i< face_number(); i++)
    {
        face_normal_array_.push_back(NormalValue(0, 0, 0));
        face_normal(i);
    }
    
    for (size_t i = 0; i< vertex_number(); i++)
    {
        vertex_normal_array_.push_back(NormalValue(0, 0, 0));
        vertex_normal(i);
    }
    updated_ = true;
}

Gfmesh::~Gfmesh()
{
    delete ppmesh_;
    ppmesh_ = 0;
}
    
void Gfmesh::face_normal(FaceIndex face_index)
{
    if (face_index >= static_cast<FaceIndex>(face_array_.size()))
    {
        throw InvalidFaceIndex();
    }
    VertexIndex v1 = vertex1_in_face(face_index);
    VertexIndex v2 = vertex2_in_face(face_index);
    VertexIndex v3 = vertex3_in_face(face_index);

    Coordinate x1 = vertex_array_[v1].x;
    Coordinate x2 = vertex_array_[v2].x;
    Coordinate x3 = vertex_array_[v3].x;

    Coordinate y1 = vertex_array_[v1].y;
    Coordinate y2 = vertex_array_[v2].y;
    Coordinate y3 = vertex_array_[v3].y;

    Coordinate z1 = vertex_array_[v1].z;
    Coordinate z2 = vertex_array_[v2].z;
    Coordinate z3 = vertex_array_[v3].z;

    // vector (v2 - v1) = a1*i + a2*j + a3*k
    double a1 = static_cast<double>(x2) - static_cast<double>(x1);
    double a2 = static_cast<double>(y2) - static_cast<double>(y1);
    double a3 = static_cast<double>(z2) - static_cast<double>(z1);

    // vector (v3 - v1) = b1*i + b2*j + b3*k
    double b1 = static_cast<double>(x3) - static_cast<double>(x1);
    double b2 = static_cast<double>(y3) - static_cast<double>(y1);
    double b3 = static_cast<double>(z3) - static_cast<double>(z1);

    //normal = (v2-v1) x (v3-v1) = (a2*b3 - a3*b2)i
    //                             (a3*b1 - a1*b3)j
    //                             (a1*b2 - a2*b1)k
    Normalf v_x = static_cast<Normalf>(a2*b3 - a3*b2);
    Normalf v_y = static_cast<Normalf>(a3*b1 - a1*b3);
    Normalf v_z = static_cast<Normalf>(a1*b2 - a2*b1);


    face_normal_array_[face_index] = NormalValue(v_x, v_y, v_z);
    return;
}

void Gfmesh::vertex_normal(VertexIndex vertex_index)
{
    std::vector<FaceIndex> faces;
    faces.reserve(MAX_VERTEX_FACE);
    //Index faces[MAX_VERTEX_FACE];
    ppmesh_->vertex_faces(vertex_index, faces);
    //assert(faces.size() > 0);
    //size_t f_size = ppmesh_->vertex_faces(vertex_index, faces);
    double v_x = 0;
    double v_y = 0;
    double v_z = 0;
    //vertex normal is the sum of face normal without normalization of neighbor faces.
    for (size_t i = 0; i < faces.size(); i++)
    //for (size_t i = 0; i < f_size; i++)
    {
        v_x += (static_cast<double>(face_normal_array_[faces[i]].nxy));
        v_y += (static_cast<double>(face_normal_array_[faces[i]].nyz));
        v_z += (static_cast<double>(face_normal_array_[faces[i]].nzx));
    }
    vertex_normal_array_[vertex_index]   = NormalValue(static_cast<Normalf>(v_x), static_cast<Normalf>(v_y), static_cast<Normalf>(v_z));
    return;
}

bool Gfmesh::decode(VertexID id, const BitString& data, size_t* p_pos, bool temp)
{
    bool result = ppmesh_->decode(id, data, p_pos, temp);
    updated_ = updated_ || result;
    return result;
}

void Gfmesh::update()
{
    std::vector<Vertex> vertices;
    std::vector<Face>   faces;
    vertices.reserve(100);
    faces.reserve(100);
    std::set<VertexIndex>    vertex_index_set;
    std::set<FaceAndIndex>   face_and_index_set;
    
    ppmesh_->updated_info(vertices, faces, vertex_index_set, face_and_index_set);

    std::vector<Vertex>::const_iterator vit = vertices.begin();
    std::vector<Vertex>::const_iterator vend = vertices.end();
    for (; vit != vend; ++vit)
    {
        vertex_array_.push_back(*vit);
    }
    
    std::vector<Face>::const_iterator fit = faces.begin();
    std::vector<Face>::const_iterator fend = faces.end();
    for (; fit != fend; ++fit)
    {
        face_array_.push_back(*fit);
        face_normal(face_array_.size());
    }

    std::set<FaceAndIndex>::const_iterator fi_it = face_and_index_set.begin();
    std::set<FaceAndIndex>::const_iterator fi_end = face_and_index_set.end();
    for (; fi_it != fi_end; ++fi_it)
    {
        face_array_[fi_it->index] = fi_it->f;
        face_normal(fi_it->index);
    }

    std::set<VertexIndex>::const_iterator vi_it = vertex_index_set.begin();
    std::set<VertexIndex>::const_iterator vi_end = vertex_index_set.end();
    for (; vi_it != vi_end; ++vi_it)
    {
        vertex_normal(*vi_it);
    }
}


