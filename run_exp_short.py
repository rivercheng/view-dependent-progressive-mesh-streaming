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
    for weight in range(1, 6):
        output_file = prefix+"output_buddha_%d_%d_push" % (batch_size, weight)
        directory = prefix+"res_push_%d_%d_w_%d" % (init_size, batch_size, weight)
        run("yes", "w", 0)

        output_file = prefix+"output_buddha_%d_%d_nopush" % (batch_size, weight)
        directory = prefix+"res_nopush_%d_%d_w_%d" % (init_size, batch_size, weight)
        run("no", "w", 0)

    weight     = 1
    output_file = prefix+"output_buddha_%d_l_push" % batch_size
    directory = prefix+"res_push_%d_%d_l" % (init_size, batch_size)
    run("yes", "l", 0)

    output_file = prefix+"output_buddha_%d_l_nopush" % batch_size
    directory = prefix+"res_nopush_%d_%d_l" % (init_size, batch_size)
    run("no", "l", 0)

    output_file = prefix+"output_buddha_%d_la_push" % batch_size
    directory = prefix+"res_push_%d_%d_la" % (init_size, batch_size)
    run("yes", "la", 0)

    output_file = prefix+"output_buddha_%d_la_nopush" % batch_size
    directory = prefix+"res_nopush_%d_%d_la" % (init_size, batch_size)
    run("no", "la", 0)
    
    output_file = prefix+"output_buddha_%d_a_push" % batch_size
    directory = prefix+"res_push_%d_%d_a" % (init_size, batch_size)
    run("yes", "a", 0)

    output_file = prefix+"output_buddha_%d_a_nopush" % batch_size
    directory = prefix+"res_nopush_%d_%d_a" % (init_size, batch_size)
    run("no", "a", 0)

    output_file = prefix+"output_buddha_%d_r_push" % batch_size
    directory = prefix+"res_push_%d_%d_r" % (init_size, batch_size)
    run("yes", "r", 0)

#history_file = "buddha_view_point"
#one_round("")

#history_file = "buddha_view_point2"
#one_round("vp2_")

history_file = "buddha_view_point3"
one_round("vp3_")

history_file = "buddha_his1"
one_round("his1_")

history_file = "buddha_his2"
one_round("his2_")
