#include "simple_render.hh"
#include "gfmesh.hh"
#include "vertexpq.hh"
#include <iostream>
#include <cstdlib>
int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: "<<argv[0]<<" <ppm file> <view point>"<<std::endl;
        exit(1);
    }
    std::ifstream ifs(argv[1]);

    //Generate two identical vdmeshes.
    Vdmesh mesh(ifs);
    Vdmesh mesh2(mesh);

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
    
    // Decode the mesh to the final state
    for (size_t i = 0; i < count; i++)
    {
        BitString data;
        VertexID    id;
        unsigned int len;
        size_t   pos = 0;
        ifs.read((char *)&id, sizeof(id));
        ifs.read((char *)&len, sizeof(len));
        data.read_binary(ifs, len);
        mesh.decode(id, data, &pos);
        
        // save the vertex splits for later usage.
        vertex_splits[id] = data;
    }
    mesh.update();
    
    // Decode the mesh2 according to the history stored in the input file.
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
            mesh2.decode(id, it->second, &pos);
            mesh2.update();
        }
    }
    mesh2.update();

    //improve the quality. Split  to at least level 3
    for (int i = 0; i < 3; i++)
    {
        for (size_t index = 0; index < mesh2.vertex_number(); index++)
        {
            VertexID id = mesh2.index2id(index);
            VertexSplitMapConstIter it = vertex_splits.find(id);
            size_t pos = 0;
            if (mesh2.id2level(id) < 3 && it != vertex_splits.end())
            {
                std::cerr << "id " << id << " index " << index << std::endl;
                mesh2.decode(id, it->second, &pos);
            }
        }
    }

    VertexPQ pq(&mesh2, Level);
    SimpleRender render(argc, argv, argv[1], &mesh, &mesh2, vertex_splits, &pq, argv[2]);
    render.setView(dx, dy, dz, ax, ay, az, scale);
    render.enterMainLoop();
}


