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
        Gfmesh *pm = 0;
        int    f_pos = 0;
        std::cerr<<"mesh readed."<<std::endl;
        int count = 0;
        ifs.read((char*)&count, sizeof(count));//no swap is considered.
        for (int i = 0; i<count; i++)
        {
            if (i==5000) 
            {
                pm = new Gfmesh(mesh);
                f_pos = ifs.tellg();
            }
            BitString data;
            VertexID id;
            unsigned int len;
            size_t   p_pos = 0;
            ifs.read((char* )&id, sizeof(id));
            ifs.read((char* )&len, sizeof(len));
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

        std::ofstream ofs3("output3.off");
        pm->outputOff(ofs3);
        std::ifstream ifs2(argv[1]);
        ifs2.seekg(f_pos, std::ios::beg);
        for (int i = 5000; i<count; i++)
        {
            BitString data;
            VertexID id;
            unsigned int len;
            size_t   p_pos = 0;
            ifs2.read((char* )&id, sizeof(id));
            ifs2.read((char* )&len, sizeof(len));
            data.read_binary(ifs2, len);
            pm->decode(id, data,&p_pos);
        }
        std::ofstream ofs4("output4.off");
        pm->update();
        pm->outputOff(ofs4);
    return 0;
}
