#include <iostream>
#include <set>
#include <algorithm>
#include "ppmesh.hh"
#include "bitstring.hh"
#include "vertexid.hh"
#include "common_def.hh"
template <typename T>
void print(const T& data)
{
    std::cout<<data<<"\n";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " <ppm file>" << std::endl;
        return 1;
    }
    
    std::string base_file(argv[1]);
    
    std::ifstream ifs(base_file.c_str());
    std::vector<Vertex> vertices;
    std::vector<Face>   faces;
    vertices.reserve(10);
    faces.reserve(10);
    std::set<VertexIndex> vertex_set;
    std::map<FaceIndex, Face>   face_map;
    Ppmesh mesh(ifs, vertices, faces, vertex_set, face_map);
    //Ppmesh mesh(ifs);
    std::cerr<<"mesh readed."<<std::endl;
        
    //initial decoding here
    size_t total = mesh.n_detail_vertices();
    for (size_t i = 0; i<total; i++)
    {
        BitString data;
        VertexID id;
        unsigned int len;
        size_t   p_pos = 0;
        ifs.read((char* )&id, sizeof(id));
        ifs.read((char* )&len, sizeof(len));
        data.read_binary(ifs, len);
        mesh.decode(id, data,&p_pos);
    }
    ifs.close();  					//reconstitution of the poor progressive mesh complete
    Ppmesh mesh2(mesh);
   
    std::cout<<"new vertices"<<"\n";
    std::for_each(vertices.begin(), vertices.end(), print<Vertex>);
    std::cout<<"\n";
    std::cout<<"new faces"<<"\n";
    std::for_each(faces.begin(), faces.end(), print<Face>);
    std::cout<<"\n";
    std::cout<<"affected vertices\n";
    std::for_each(vertex_set.begin(), vertex_set.end(), print<VertexIndex>);
    std::cout<<"\n";
    std::cout<<"affected faces\n";
    std::for_each(face_map.begin(), face_map.end(), print<std::pair<FaceIndex, Face> >);
    std::cout<<"\n";
    std::cout<<"\n";
    return 0;
}
