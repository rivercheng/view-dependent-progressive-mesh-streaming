#include "render.hh"
#include "gfmesh.hh"
#include <iostream>
#include <cstdlib>
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: "<<argv[0]<<" <ppm file> [view point] [count]"<<std::endl;
        exit(1);
    }
    std::ifstream ifs(argv[1]);
    Gfmesh gfmesh(ifs);
    size_t count = gfmesh.n_detail_vertices();
    double dx = 0;
    double dy = 0;
    double dz = 0;
    double ax = 0;
    double ay = 0;
    double az = 0;
    double scale = 1;
    if (argc > 2)
    {
        std::ifstream ifs2(argv[2]);
        ifs2 >> dx >> dy >> dz >> ax >> ay >> az >> scale;
        if (argc > 3)
        {
            count = atoi(argv[3]);
        }
    }

    for (size_t i = 0; i < count; i++)
    {
        BitString data;
        VertexID    id;
        unsigned int len;
        size_t   pos = 0;
        ifs.read((char *)&id, sizeof(id));
        ifs.read((char *)&len, sizeof(len));
        data.read_binary(ifs, len);
        gfmesh.decode(id, data, &pos);
    }
    gfmesh.update();

    Center center = auto_center(gfmesh.vertex_number(), gfmesh.vertex_array());
    Render render(argc, argv, argv[1], &gfmesh, 60, center);
    render.setView(dx, dy, dz, ax, ay, az, scale);
    render.enterMainLoop();
}


