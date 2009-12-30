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

    Vdmesh mesh(ifs);
    Center center = auto_center(mesh.vertex_number(), mesh.vertex_array());
    
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
        
    }
    mesh.update();
    
    std::map<VertexID, BitString> vertex_splits;
    SimpleRender render(argc, argv, argv[1], &mesh, vertex_splits, center, 0, argv[2], 1, 1, 1);
    render.setView(dx, dy, dz, ax, ay, az, scale);
    std::cerr << dx << " " << dy << " " << dz << std::endl;
    render.enterMainLoop();
}


