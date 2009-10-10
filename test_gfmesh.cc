#include "gfmesh.hh"
#include <iostream>
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " <base file>" << std::endl;
        return 1;
    }
    
        std::string base_file(argv[1]);
    
        std::ifstream ifs(base_file.c_str());
        Gfmesh mesh(ifs);
        std::cerr<<"mesh readed."<<std::endl;
        int count = 0;
        ifs.read((char*)&count, sizeof(count));//no swap is considered.
        for (int i = 0; i<count; i++)
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
        }
        std::ofstream ofs("output.off");
        mesh.outputOff(ofs);
        std::ofstream ofs2("output2.off");
        mesh.update();
        mesh.outputOff(ofs2);
        ifs.close();  					//reconstitution of the poor progressive mesh complete
        ofs.close();
        ofs2.close();
    return 0;
}
