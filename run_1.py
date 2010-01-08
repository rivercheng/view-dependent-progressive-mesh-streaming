import sys
import os



def one_round(prefix, update_period):
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
    for weight in range(6):
        output_file = prefix+"output_buddha_%d_%d_push_new" % (update_period, weight)
        directory = prefix+"res_push_%d_%d_%d_%d" % (init_size, batch_size, update_period, weight)
        run("yes", "w", 0)

    output_file = prefix+"output_buddha_%d_l_push_new" % (update_period)
    directory = prefix + "res_push_%d_%d_%d_l" % (init_size, batch_size, update_period)
    run("yes", "l", 0)

    output_file = prefix+"output_buddha_%d_la_push_new" % (update_period)
    directory = prefix + "res_push_%d_%d_%d_la" % (init_size, batch_size, update_period)
    run("yes", "la", 0)

    output_file = prefix+"output_buddha_%d_a_push_new" % (update_period)
    directory = prefix + "res_push_%d_%d_%d_a" % (init_size, batch_size, update_period)
    run("yes", "a", 0)

history_file = "buddha_view_point"
one_round("", 30000)
one_round("", 15000)
one_round("", 10000)
one_round("", 200)

history_file = "buddha_view_point2"
one_round("vp2_", 30000)
one_round("vp2_", 15000)
one_round("vp2_", 10000)
one_round("vp2_", 200)

history_file = "buddha_view_point3"
one_round("vp3_", 30000)
one_round("vp3_", 15000)
one_round("vp3_", 10000)
one_round("vp3_", 200)

history_file = "buddha_his1"
one_round("his1_", 30000)
one_round("his1_", 15000)
one_round("his1_", 10000)
one_round("his1_", 200)

history_file = "buddha_his2"
one_round("his2_", 30000)
one_round("his2_", 15000)
one_round("his2_", 10000)
one_round("his2_", 200)

