#include "simple_render.hh"
#include "gfmesh.hh"
#include "vertexpq.hh"
#include <iostream>
#include <cstdlib>
int main(int argc, char** argv)
{
    SelectMode mode = WeightedScreen;
    int total_count = 0;
    int initial_size = 1;
    int batch_size  = 1;
    int update_period = 1;
    int weight     = 1;
    bool push_children = true;
    
    if (argc < 5)
    {
        std::cerr << "Usage: "<<argv[0]<<" <ppm file> <view point and history> <mode = 'w': Weighted Screen Area| mode = 's': Silhouette + Screen Area | 'a':Screen Area | 'l': level | 'la': level + Area | 'r': Random> <push_child: y/n> [weight=1] [total_count=0] [initial_size=1] [batch_size=1] [update_period=1]"<<std::endl;
        std::cerr << "       total_count = 0 means split all vertex splits. The update period should be a multiple of batch size."<<std::endl;
        exit(1);
    }

        if (argv[3][0] == 'a')
        {
            mode = ScreenArea;
        }
        else if (argv[3][0] == 'w')
        {
            mode = WeightedScreen;
        }
        else if (argv[3][0] == 's')
        {
            mode = SilhouetteScreen;
        }
        else if (argv[3][0] == 'l' && argv[3][1] == 'a')
        {
            mode = LevelArea;
        }
        else if (argv[3][0] == 'l')
        {
            mode = Level;
        }
        else if (argv[3][0] == 'r')
        {
            mode = Random;
        }
    
    if (argv[4][0] == 'y' or argv[4][0] == 'Y')
    {
        push_children = true;
    }
    else if (argv[4][0] == 'n' or argv[4][0] == 'N')
    {
        push_children = false;
    }
    else 
    {
        std::cerr << "push child has to be 'y' or 'n'." << std::endl;
        exit(1);
    }


    if (argc > 5)
    {
        weight = atoi(argv[5]);
    }

    if (argc > 6)
    {
        total_count = atoi(argv[6]);
    }

    if (argc > 7)
    {
        initial_size = atoi(argv[7]);
    }

    if (argc > 8)
    {
        batch_size = atoi(argv[8]);
    }

    if (argc > 9)
    {
        update_period = atoi(argv[9]);
    }

    if (update_period / batch_size * batch_size != update_period)
    {
        std::cerr<<"update period should be a multiple of batch size."<<std::endl;
        exit(1);
    }

    std::ifstream ifs(argv[1]);


    Vdmesh mesh(ifs);
    Center center = auto_center(mesh.vertex_number(), mesh.vertex_array());
    size_t count = mesh.n_detail_vertices();
    if (total_count == 0) total_count = count;
    
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

    
    VertexPQ pq(&mesh, mode, push_children, &vertex_splits);
    pq.set_silhouette_weight(weight);
    SimpleRender render(argc, argv, argv[1], &mesh, vertex_splits, center, &pq, argv[2], initial_size, batch_size, update_period, total_count);
    
    //set final image
    std::string prefix = std::string(argv[2]);
    std::string image_name =  prefix + "_final_image.pgm";
    render.set_original_image(image_name);
    
    render.setView(dx, dy, dz, ax, ay, az, scale);
    std::cerr << dx << " " << dy << " " << dz << std::endl;
    render.enterMainLoop();
}


