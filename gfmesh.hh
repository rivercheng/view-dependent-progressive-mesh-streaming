#ifndef __GFMESH_H__
#define __GFMESH_H__
#include "common_def.hh"
#include "vertexid.hh"
class Ppmesh;
class BitString;
class Gfmesh
{
    public:
        Gfmesh(std::istream ifs);
        virtual ~Gfmesh(void);
        bool    decode(VertexID id, const BitString& data, size_t* p_pos, bool temp=false);
        /**
         * Update the gfmesh to be consistent with ppmesh.
         */
        void    update(void);
        
        /**
        * Return the address of vertex array.
        */
        inline  const Coordinate* vertex_array   (void) const
        {
            return static_cast<const Coordinate *>(&vertex_array_[0].x);
        }
    
        /**
        * Return the address of face array.
        */
        inline const VertexIndex*     face_array   (void)   const
        {
            return static_cast<const VertexIndex *>(&face_array_[0].v1);
        }
        
        /**
        * Return the address of vertex normal array.
        */
        inline const Normalf*    vertex_normal_array(void) const
        {
            return static_cast<const Normalf *>(&vertex_normal_array_[0].nxy);
        }

        /**
        * return the address of face normal array.
        */
        inline const Normalf*    face_normal_array(void)   const
        {
            return static_cast<const Normalf *>(&face_normal_array_[0].nxy);
        }

        inline VertexIndex vertex1_in_face(FaceIndex faceIndex) const
        {
            assert(faceIndex < face_array_.size());
            return face_array_[faceIndex].v1;
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
        inline bool   updated          (void) const
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


    private:
        const static int RESERVE_SIZE = 10000000;//assume at least space for RESERVER_SIZE vertices are allocated.
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
    private:
        void  face_normal   (FaceIndex face_index);
        void  vertex_normal (VertexIndex vertex_index);
};
#endif

