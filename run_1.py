import sys
import os



def one_round(prefix):
    def run(push, type, count):
        os.system("mkdir -p " + directory)
        os.system("cp %s %s/" % (history_file, directory))
        os.system("cp %s_final_image.pgm %s/" % (history_file, directory))
        config1 = (directory+"/"+history_file, type, push, weight, count, init_size, batch_size, update_period, output_file)
        command = "./vertex_by_vertex happy.ppm %s %s %s %d %d %d %d %d > %s" % config1
        print command
        os.system(command)
    
    init_size = 10000
    batch_size = 200 
    update_period = 10000
    weight = 1
    output_file = prefix+"output_buddha_%d_%d_push_new" % (update_period, weight)
    directory = prefix+"res_push_%d_%d_%d_%d" % (init_size, batch_size, update_period, weight)
    run("yes", "w", 0)

    batch_size = 10000 
    output_file = prefix+"output_buddha_%d_%d_push_new2" % (update_period, weight)
    directory = prefix+"res_push_%d_%d_%d_%d" % (init_size, batch_size, update_period, weight)
    run("yes", "w", 0)
history_file = "buddha_view_point"
one_round("")
