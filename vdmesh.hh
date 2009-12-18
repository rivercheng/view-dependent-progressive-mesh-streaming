#ifndef __VDMESH_H__
#define __VDMESH_H__
#include "common_def.hh"
#include "vertexid.hh"
#include "gfmesh.hh"
#include <iostream>
#include <set>
#include <vector>
#include <map>

class Vdmesh : public Gfmesh
{
// ================================INTERFACE=============================================
    public:
        Vdmesh(std::istream& ifs) : Gfmesh(ifs)
        {
            vertex_visibility_array_.resize(vertex_number(), false);
            vertex_importance_array_.resize(vertex_number(), 0);
            face_visibility_array_.resize(face_number(), false);
            face_importance_array_.resize(face_number(), 0);
        }
        
        Vdmesh(const Vdmesh& m) : Gfmesh(m), \
                                  vertex_visibility_array_(m.vertex_visibility_array()), \
                                  vertex_importance_array_(m.vertex_importance_array()), \
                                  face_visibility_array_(m.face_visibility_array()), \
                                  face_importance_array_(m.face_importance_array())
        {
        }

        virtual ~Vdmesh(void)
        {
        }

        void set_vertex_visibility(VertexIndex index, bool is_visible)
        {
            if (index >= vertex_visibility_array_.size())
            {
                vertex_visibility_array_.resize(index+1, false);
            }
            vertex_visibility_array_[index] = is_visible;
        }

        void set_face_visibility(FaceIndex index, bool is_visible)
        {
            if (index >= face_visibility_array_.size())
            {
                face_visibility_array_.resize(index+1, false);
            }
            face_visibility_array_[index] = is_visible;
        }
        
        bool vertex_is_visible(VertexIndex index)
        {
            return vertex_visibility_array_[index];
        }

        bool face_is_visible(FaceIndex index)
        {
            return face_visibility_array_[index];
        }

        std::vector<bool>vertex_visibility_array() const
        {
            return vertex_visibility_array_;
        }

        std::vector<bool>face_visibility_array() const
        {
            return face_visibility_array_;
        }

        std::vector<int> vertex_importance_array() const
        {
            return vertex_importance_array_;
        }

        std::vector<int> face_importance_array() const
        {
            return face_importance_array_;
        }

        int  vertex_importance(VertexIndex index)
        {
            return vertex_importance_array_[index];
        }

        int  face_importance(FaceIndex index)
        {
            return face_importance_array_[index];
        }

        void set_vertex_importance(VertexIndex index, int importance)
        {
            if (index >= vertex_importance_array_.size())
            {
                vertex_importance_array_.resize(index+1, 0);
            }
            vertex_importance_array_[index] = importance;
        }

        void set_face_importance(FaceIndex index, int importance)
        {
            if (index >= face_importance_array_.size())
            {
                face_importance_array_.resize(index+1, 0);
            }
            face_importance_array_[index] = importance;
        }
        
        void reset_vertex_visibility(bool value)
        {
            for (std::vector<bool>::iterator it = vertex_visibility_array_.begin(); it != vertex_visibility_array_.end(); ++it)
            {
                *it = value;
            }
        }

        void reset_face_visibility(bool value)
        {
            for (std::vector<bool>::iterator it = face_visibility_array_.begin(); it != face_visibility_array_.end(); ++it)
            {
                *it = value;
            }
        }

        void reset_vertex_importance(void)
        {
            for (std::vector<int>::iterator it = vertex_importance_array_.begin(); it != vertex_importance_array_.end(); ++it)
            {
                *it = 0;
            }
        }

        void reset_face_importance(void)
        {
            for (std::vector<int>::iterator it = face_importance_array_.begin(); it != face_importance_array_.end(); ++it)
            {
                *it = 0;
            }
        }

        
// =====================================PRIVATE================================================================
    private: //variables
        std::vector<bool>          vertex_visibility_array_;
        std::vector<int>           vertex_importance_array_;
        std::vector<bool>          face_visibility_array_;
        std::vector<int>           face_importance_array_;
    private:    //functions
        Vdmesh& operator=(const Vdmesh&);
};
#endif


