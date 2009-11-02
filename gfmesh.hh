// =============================================================================
// Written by Cheng Wei
// rivercheng@gmail.com
// 9 Oct 2009
// =============================================================================
#ifndef __GFMESH_H__
#define __GFMESH_H__
#include "common_def.hh"
#include "vertexid.hh"
#include <iostream>
#include <set>
#include <vector>
#include <map>
class Ppmesh;
class BitString;
class Gfmesh
{
    public:
        /**
         * Construct a gfmesh by reading ifs.
         */
        Gfmesh(std::istream& ifs);

        /**
         * Copy construction is allowed.
         * The embedded ppmesh is recreated and everything is really copied.
         * No sharing data with the old gfmesh.
         */
        Gfmesh(const Gfmesh&);

        /**
         * the embedded ppmesh will be deleted.
         */
        virtual ~Gfmesh(void);

        /**
         * decode and apply all vertex splits coded in the data.
         * NOTICE: before calling update(), only the embedded ppmesh is changed 
         * and the changes are written to report arrays. The vertex array, face array, 
         * and normal arrays are all kept untouched. This implementation is for efficiency.
         * We prefer to update the gfmesh at once when necessary.
         */
        bool    decode(VertexID id, const BitString& data, size_t* p_pos);
        
        /**
         * Update the gfmesh to be consistent with ppmesh.
         * After updating, the report arrays will be cleared.
         */
        void    update(void);
        
        /**
         * Output an OFF file of current mesh.
         */
        void    outputOff(std::ostream& os) const;

        
        /**
        * Return the address of vertex array.
        * For render usage.
        */
        inline  const Coordinate* vertex_array   (void) const
        {
            return static_cast<const Coordinate *>(&vertex_array_[0].x);
        }
    
        /**
        * Return the address of face array.
        * For render usage.
        */
        inline const VertexIndex*     face_array   (void)   const
        {
            return static_cast<const VertexIndex *>(&face_array_[0].v1);
        }
        
        /**
        * Return the address of vertex normal array.
        * For render usage.
        */
        inline const Normalf*    vertex_normal_array(void) const
        {
            return static_cast<const Normalf *>(&vertex_normal_array_[0].nxy);
        }

        /**
        * return the address of face normal array.
        * Fore render usage.
        */
        inline const Normalf*    face_normal_array(void)   const
        {
            return static_cast<const Normalf *>(&face_normal_array_[0].nxy);
        }

        /**
        * return the vertex number.
        */
        inline size_t vertex_number    (void) const
        {
            return vertex_array_.size();
        }

        /**
        * return the face number.
        */
        inline size_t face_number      (void) const
        {
            return face_array_.size();
        }
        
        inline VertexIndex vertex1_in_face(FaceIndex faceIndex) const
        {
            assert(faceIndex < face_array_.size());
            return face_array_[faceIndex].v1;
        }
    
        
        inline VertexIndex vertex2_in_face(FaceIndex faceIndex) const
        {
            assert(faceIndex < face_array_.size());
            return face_array_[faceIndex].v2;
        }

        inline VertexIndex vertex3_in_face(FaceIndex faceIndex) const
        {
            assert(faceIndex < face_array_.size());
            return face_array_[faceIndex].v3;
        }
    
        /**
        * to see if this gfmesh is updated.
        * It is used in rendering part to avoid refreshing
        * the same gfmesh.
        */
        inline bool   updated (void) const
        {
            return updated_;
        }

        /**
        * reset the updated flag to false
        */
        inline void reset_updated  (void)
        {
            updated_ = false;
        };
    private: //variables
        const static int RESERVE_SIZE = 1000000;//assume at least space for RESERVER_SIZE vertices are allocated.
        const static int MAX_VERTEX_FACE = 10000;
        const static int MAX_FACE_VERTEX = 10;
        const static int MAX_VERTEX_VERTEX = 10000;

        class InvalidFaceIndex{};

        Ppmesh* ppmesh_;
        std::vector<Vertex>         vertex_array_;
        std::vector<Face>           face_array_;
        std::vector<NormalValue>    vertex_normal_array_;
        std::vector<NormalValue>    face_normal_array_;
        bool                        updated_;

        //data structure for update(). 
        //Avoid to be local to improve efficiency.
        std::vector<Vertex>  vertices;
        std::vector<Face>    faces;
        std::set<VertexIndex> vertex_index_set;
        std::map<FaceIndex, Face> face_map;

    private:    //functions
        Gfmesh& operator=(const Gfmesh&);
        void  init (void);
        void  face_normal   (FaceIndex face_index);
        void  vertex_normal (VertexIndex vertex_index);
};
#endif


