#include "render.hh"
#include "vdmesh.hh"
#include "vertexpq.hh"
#include <iostream>
#include <cstdlib>
#include <map>
int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: "<<argv[0]<<" <ppm file> <history file>"<<std::endl;
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
    ifs2 >> dx >> dy >> dz >> ax >> ay >> az >> scale;

    typedef std::map<VertexID, BitString> VertexSplitMap;
    typedef std::map<VertexID, BitString>::const_iterator VertexSplitMapConstIter;
    VertexSplitMap vertex_splits;
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
            std::cout << id << std::endl;
            mesh.decode(id, it->second, &pos);
        }
    }

    mesh.update();
    Center center = auto_center(mesh.vertex_number(), mesh.vertex_array());
    VertexPQ pq(&mesh, ScreenArea, true);
    Render render(argc, argv, argv[1], &mesh, 60, center, &pq);
    render.setView(dx, dy, dz, ax, ay, az, scale);
    render.enterMainLoop();
}


