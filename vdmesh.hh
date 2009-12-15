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
        }
        
        Vdmesh(const Vdmesh& m) : Gfmesh(m), \
                                  visibility_array_(m.visibility_array()), \
                                  importance_array_(m.importance_array()) 
        {
        }

        virtual ~Vdmesh(void)
        {
        }

        void set_visibility(VertexID id, bool is_visible)
        {
            visibility_array_[id] = is_visible;
        }

        bool is_visible(VertexID id)
        {
            return visibility_array_[id];
        }

        std::vector<bool> visibility_array() const
        {
            return visibility_array_;
        }

        std::vector<int> importance_array() const
        {
            return importance_array_;
        }

        int  importance(VertexID id)
        {
            return importance_array_[id];
        }

        void setImportance(VertexID id, int importance)
        {
            importance_array_[id] = importance;
        }
        
// =====================================PRIVATE================================================================
    private: //variables
        std::vector<bool>          visibility_array_;
        std::vector<int>           importance_array_;
    private:    //functions
        Vdmesh& operator=(const Vdmesh&);
};
#endif


