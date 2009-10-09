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
        std::cout << "usage: " << argv[0] << " <base file>" << std::endl;
        return 1;
    }
    
    std::string base_file(argv[1]);
    
        std::ifstream ifs(base_file.c_str());
        Ppmesh mesh(ifs);
        std::cerr<<"mesh readed."<<std::endl;
        
        //initial decoding here
        int count = 0;
        ifs.read((char*)&count, sizeof(count));//no swap is considered.
        std::cerr<<"count "<<count<<std::endl;
        int i = 0;
        for (i = 0; i<count; i++)
        {
            BitString data;
            VertexID id;
            unsigned int len;
            size_t   p_pos = 0;
            ifs.read((char* )&id, sizeof(id));
            ifs.read((char* )&len, sizeof(len));
            //std::cerr<<"id "<<id<<" len "<<len<<std::endl;
            data.read_binary(ifs, len);
            
            //std::cerr<<"to decode "<<id<<" with "<<data<<std::endl;
            mesh.decode(id, data,&p_pos);
            //std::cerr<<i<<std::endl;
            /*
            std::vector<Vertex> vertices;
            std::vector<Face>   faces;
            std::set<VertexIndex> vertex_set;
            std::set<FaceAndIndex>   face_and_index_set;
            mesh.updated_info(vertices, faces, vertex_set, face_and_index_set);
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
            std::for_each(face_and_index_set.begin(), face_and_index_set.end(), print<FaceAndIndex>);
            std::cout<<"\n";
            std::cout<<"\n";
            */
        }
        ifs.close();  					//reconstitution of the poor progressive mesh complete
    return 0;
}
