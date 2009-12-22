LIBRARY = -lOpenMesh_Core #-lprofiler#-lOpenMesh_Tools -lPocoFoundation -lPocoNet -lPocoUtil -lglut #-lprofiler	
OPTIONS =  -Wall -W -Wextra -g -O2 
CC = g++
ALL = test_ppmesh test_gfmesh test_ppm test_render test_history test_pq best_image vertex_by_vertex
OBJ = ppmesh.o bitstring.o vertexid.o common_def.o gfmesh.o vertexpq.o

all: $(ALL)
tags: *.cc *.hh
	$(CTAG) -R *
test_render:test_render.o $(OBJ) render.o baserender.o
	$(CC) $(OPTIONS) $(LIBRARY)  -lglut -o $@ $< $(OBJ) render.o baserender.o
test_history:test_history.o $(OBJ) render.o baserender.o
	$(CC) $(OPTIONS) $(LIBRARY)  -lglut -o $@ $< $(OBJ) render.o baserender.o
test_pq:test_pq.o $(OBJ) render.o
	$(CC) $(OPTIONS) $(LIBRARY)  -lglut -o $@ $< $(OBJ) render.o baserender.o
best_image:best_image.o $(OBJ) simple_render.o baserender.o
	$(CC) $(OPTIONS) $(LIBRARY)  -lglut -o $@ $< $(OBJ) simple_render.o baserender.o
vertex_by_vertex:vertex_by_vertex.o $(OBJ) simple_render.o baserender.o
	$(CC) $(OPTIONS) $(LIBRARY)  -lglut -o $@ $< $(OBJ) simple_render.o baserender.o
psnr:psnr.c
	gcc -Wall -O2 -o $@ $<
%:%.o $(OBJ)
	$(CC) $(OPTIONS) $(LIBRARY) -o $@ $< $(OBJ)
%.o:%.cc 
	$(CC) $(OPTIONS) -c $<
clean:
	rm *.o
	rm $(ALL)
depend:
	makedepend -Y *.cc
# DO NOT DELETE

baserender.o: baserender.hh vertexid.hh bitstring.hh common_def.hh
best_image.o: simple_render.hh vertexid.hh bitstring.hh baserender.hh
best_image.o: common_def.hh gfmesh.hh vertexpq.hh vdmesh.hh
bitstring.o: bitstring.hh
common_def.o: common_def.hh
gfmesh.o: gfmesh.hh common_def.hh vertexid.hh bitstring.hh ppmesh.hh
gfmesh.o: huffman.hh
ppmesh.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
render.o: render.hh baserender.hh vertexid.hh bitstring.hh common_def.hh
render.o: gfmesh.hh vertexpq.hh vdmesh.hh
simple_render.o: simple_render.hh vertexid.hh bitstring.hh baserender.hh
simple_render.o: common_def.hh gfmesh.hh vertexpq.hh vdmesh.hh
test_gfmesh.o: gfmesh.hh common_def.hh vertexid.hh bitstring.hh
test_history.o: render.hh baserender.hh vertexid.hh bitstring.hh
test_history.o: common_def.hh vdmesh.hh gfmesh.hh
test_ppm.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
test_ppmesh.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
test_pq.o: render.hh baserender.hh vertexid.hh bitstring.hh common_def.hh
test_pq.o: vdmesh.hh gfmesh.hh vertexpq.hh
test_render.o: render.hh baserender.hh vertexid.hh bitstring.hh common_def.hh
test_render.o: gfmesh.hh
vertex_by_vertex.o: simple_render.hh vertexid.hh bitstring.hh baserender.hh
vertex_by_vertex.o: common_def.hh gfmesh.hh vertexpq.hh vdmesh.hh
vertexid.o: vertexid.hh bitstring.hh
vertexpq.o: vertexpq.hh vdmesh.hh common_def.hh vertexid.hh bitstring.hh
vertexpq.o: gfmesh.hh
