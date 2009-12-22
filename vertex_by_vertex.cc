#include "simple_render.hh"
#include "gfmesh.hh"
#include "vertexpq.hh"
#include <iostream>
#include <cstdlib>
int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: "<<argv[0]<<" <ppm file> <view point and history>"<<std::endl;
        exit(1);
    }
    std::ifstream ifs(argv[1]);


    Vdmesh mesh(ifs);
    size_t count = mesh.n_detail_vertices();
    
    double dx = 0;
    double dy = 0;
    double dz = 0;
    double ax = 0;
    double ay = 0;
    double az = 0;
    double scale = 1;

    std::ifstream ifs2(argv[2]);
    // read in the initial viewpoint
    ifs2 >> dx >> dy >> dz >> ax >> ay >> az >> scale;

    typedef std::map<VertexID, BitString> VertexSplitMap;
    typedef std::map<VertexID, BitString>::const_iterator VertexSplitMapConstIter;
    VertexSplitMap vertex_splits;
    
    // Read all the vertex splits to a map
    for (size_t i = 0; i < count; i++)
    {
        BitString data;
        VertexID    id;
        unsigned int len;
        ifs.read((char *)&id, sizeof(id));
        ifs.read((char *)&len, sizeof(len));
        data.read_binary(ifs, len);
        vertex_splits[id] = data;
    }
    
    // Decode the mesh according to the history stored in the input file.
    while(true)
    {
        VertexID id;
        ifs2 >> id;
        if (ifs2.eof() || ifs2.fail())
        {
            break;
        }
        size_t pos = 0;
        VertexSplitMapConstIter it = vertex_splits.find(id);
        if (it != vertex_splits.end())
        {
            //std::cout << id << std::endl;
            mesh.decode(id, it->second, &pos);
        }
    }
    mesh.update();

    //improve the quality. Split  to at least level 3
    for (int i = 0; i < 3; i++)
    {
        for (size_t index = 0; index < mesh.vertex_number(); index++)
        {
            VertexID id = mesh.index2id(index);
            assert(mesh.id2index(id) == index);
            VertexSplitMapConstIter it = vertex_splits.find(id);
            size_t pos = 0;
            if (mesh.id2level(id) < 3 && it != vertex_splits.end())
            {
                //std::cerr << "id " << id << " index " << index << std::endl;
                mesh.decode(id, it->second, &pos);
                mesh.update();
            }
        }
    }
    std::cerr << "quality improved." << std::endl;

    // Force splitting all the pending splits.
    // To avoid one vertex split trigger many pending one, which exaggerates the effect of this split.
    VertexID id_to_be_split = 0;
    while((id_to_be_split = mesh.to_be_split()) != 0)
    {
        //std::cerr << "to be split " << id_to_be_split << std::endl;
        size_t pos = 0;
        mesh.decode(id_to_be_split, vertex_splits[id_to_be_split], &pos);
    }
    mesh.update();
    std::cerr << "force split" << std::endl;
    
    VertexPQ pq(&mesh, ScreenArea, &vertex_splits);
    SimpleRender render(argc, argv, argv[1], &mesh, vertex_splits, &pq, argv[2]);
    render.setView(dx, dy, dz, ax, ay, az, scale);
    render.enterMainLoop();
}


