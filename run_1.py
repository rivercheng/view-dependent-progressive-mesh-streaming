import sys
import os



def one_round(prefix):
    def run(push, type, count):
        os.system("mkdir -p " + directory)
        os.system("cp %s %s/" % (history_file, directory))
        os.system("cp %s_final_image.pgm %s/" % (history_file, directory))
        config1 = (directory+"/"+history_file, type, push, weight, count, init_size, batch_size, output_file)
        command = "./vertex_by_vertex happy.ppm %s %s %s %d %d %d %d > %s" % config1
        print command
        os.system(command)
    
    init_size = 10000
    batch_size = 200
    weight = 1
    output_file = prefix+"output_buddha_%d_a_push" % batch_size
    directory = prefix+"res_push_%d_%d_a" % (init_size, batch_size)
    run("yes", "a", 0)

history_file = "buddha_view_point"
one_round("")
