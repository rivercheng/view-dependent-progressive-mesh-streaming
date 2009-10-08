#include <string>
#include <bitset>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include "vertexid.hh"

//currently assume id has 32 bits, need to be updated if we use 64 bits.
static VertexID masks[32] =
    {0x80000000, 0x40000000, 0x20000000, 0x10000000,\
     0x08000000, 0x04000000, 0x02000000, 0x01000000,\
     0x00800000, 0x00400000, 0x00200000, 0x00100000,\
     0x00080000, 0x00040000, 0x00020000, 0x00010000,\
     0x00008000, 0x00004000, 0x00002000, 0x00001000,\
     0x00000800, 0x00000400, 0x00000200, 0x00000100,\
     0x00000080, 0x00000040, 0x00000020, 0x00000010,\
     0x00000008, 0x00000004, 0x00000002, 0x00000001
    };

//a type to store the tree_id and pos_of_id to improve the efficiency.
struct IdPair
{
    VertexID id;
    VertexID tree_id;
    size_t   pos_of_id;
};

//The comparison method to compare IdPair.
static bool cmp(IdPair pair1, IdPair pair2)
{
    return pair1.tree_id < pair2.tree_id;
}

//to check whether a given bit is 1 or 0 in an ID.
bool  idIsSet(VertexID id, size_t pos)
{
    //while (!(id & 0x80000000)) id <<= 1;
    int len = len_of_id(id);

    //VertexID mask = 0x80000000 >> (pos + 1);
    //return (id & masks[pos+1]);
    return (id & masks[32 - len + pos]);
}

/**
 * return the length of the ID, ignoring the leading 1.
 */
/*inline size_t len_of_id(VertexID id)
{
    size_t len = 0;
    while (id != 0)
    {
        id >>= 1;
        len ++;
    }
    return len-1;
}*/

/**
 * convert to a string representation.
 */
std::string to_string(VertexID id)
{
    //assume id has 32 bits here.
    std::bitset<32> bs(id);
    std::string     str(bs.to_string());
    size_t loc = str.find('1');
    return str.substr(loc+1);
};

//recursive way to encode ID list to a bitstring.
static void divide_and_code(const std::vector<IdPair>& pair_array, size_t pos, BitString& bs)
{
    std::vector<IdPair> array0;
    std::vector<IdPair> array1;
    std::vector<IdPair>::const_iterator it = pair_array.begin();
    std::vector<IdPair>::const_iterator end = pair_array.end();

    if (pair_array.size() == 1 && (pos + pair_array[0].pos_of_id == 8*sizeof(VertexID)))
    {
        bs.push_back_bit(false);
        bs.push_back_bit(false);
        return;
    }

    for (; it != end; ++it)
    {
        //std::cerr<<it->pos_of_id<<std::endl;
        //std::cerr<<masks[pos + it->pos_of_id]<<std::endl;
        if (((it->id) & masks[pos+(it->pos_of_id)]) == 0)
        {
            array0.push_back(*it);
        }
        else
        {
            array1.push_back(*it);
        }
    }
    if (array0.size() > 0)
    {
        bs.push_back_bit(true);
        divide_and_code(array0, pos+1, bs);
    }
    else
    {
        bs.push_back_bit(false);
    }
    if (array1.size() > 0)
    {
        bs.push_back_bit(true);
        divide_and_code(array1, pos+1, bs);
    }
    else
    {
        bs.push_back_bit(false);
    }
}

//Encode the node IDs belonging the same tree to a bitstring.
//Have to ensure all ids belong to one tree.
static void encode_one_tree(const std::vector<IdPair>& ids_in_tree, BitString& bs)
{
    //From left to right, from 0 to 8*sizeof(VertexID)-1
    //pos is the first bit of the node id.
    size_t pos = 0;
    divide_and_code(ids_in_tree, pos, bs);
}

//Decode a bitstring to a nodeID.
static size_t decode_node_id(const BitString& bs, size_t pos, BitString& node_bs, std::vector<BitString>& node_bs_array)
{
    //how many bits are consumed.
    bool   hasChild = false;
    //std::cerr<<node_bs<<" ";
    //std::cerr<<pos<<" ";
    //std::cerr<<bs.test(pos)<<std::endl;
    if (bs.test(pos++))
    {
        node_bs.push_back_bit(false);
        pos = decode_node_id(bs, pos, node_bs, node_bs_array);
        hasChild = true;
    }
    if (bs.test(pos++))
    {
        node_bs.push_back_bit(true);
        pos = decode_node_id(bs, pos, node_bs, node_bs_array);
        hasChild = true;
    }
    if (!hasChild)
    {
        node_bs_array.push_back(node_bs);
    }
    node_bs.pop_back_bit();
    return pos;
}

//Decode a bitstring to a nodeID list 
static size_t decode_one_tree(const BitString& bs, size_t pos, VertexID tree_id, size_t, std::vector<VertexID>& ids_in_tree)
{
    BitString node_bs;
    std::vector<BitString> node_bs_array;
    pos = decode_node_id(bs, pos, node_bs, node_bs_array);
    VertexID common_id = tree_id;// + (1 << tree_bits);
    for (size_t i = 0; i< node_bs_array.size(); i++)
    {
        BitString bs = node_bs_array[i];
        VertexID id  = common_id << bs.size();
        id += static_cast<VertexID>(bs.to_ulong());
        ids_in_tree.push_back(id);
    }
    return pos;
}



//Retrive the tree_id of a ID.
//tree_id always begins with '1' and has 1 + tree_bits digits.
static VertexID id2tree_id(VertexID id, size_t tree_bits)
{
    VertexID tree_id_mask = 0xffffffff;
    tree_id_mask >>= (sizeof(VertexID)*8 - tree_bits - 1);
    size_t len = len_of_id(id);
    tree_id_mask <<= (len - tree_bits);
    VertexID tree_id = id & tree_id_mask;
    tree_id >>= (len - tree_bits);
    return tree_id;
}

//Encode a tree_id list to a bitstring. The previous tree_id needs to be given.
//We assume id is larger than pre_id.
static void encode_tree_id(VertexID tree_id, VertexID pre_tree_id, BitString& bs)
{
    assert(tree_id > pre_tree_id);
    VertexID diff = tree_id - pre_tree_id;
    size_t len = len_of_id(diff) + 1;
    //std::cerr<<len<<std::endl;
    //Elias gamma
    for (size_t i = 0; i < len-1; i++)
    {
        bs.push_back_bit(false);
    }
    BitString bs_diff;
    for (size_t i = 0; i< len; i++)
    {
        VertexID mask = masks[sizeof(VertexID)*8 - len + i];
        bs.push_back_bit(diff & mask);
    }
}

//Decode a bitstring to a tree_id. The previous tree_id needs to be given.
static VertexID decode_tree_id(VertexID pre_tree_id, const BitString& bs, size_t* p_pos)
{
    VertexID tree_id;
    size_t len = 0;
    while (((*p_pos)<bs.size()&&!bs.test(*p_pos)))
    {
        (*p_pos)++;
        len++;
        //std::cerr<<(*p_pos)<<std::endl;
    }
    //std::cerr<<(*p_pos)<<std::endl;
    //std::cerr<<bs.size()<<std::endl;
    //std::cerr<<bs.test(*p_pos -1)<<std::endl;
    if (*p_pos == bs.size() && !bs.test(*p_pos-1))
    {
        std::cerr<<"end of string"<<std::endl;
        throw EndOfBitString();
    }
    //std::cerr<<len<<" "<<(*p_pos)<<std::endl;
    BitString tree_id_bs = bs.substr(*p_pos, len+1);
    (*p_pos) += (len+1);
    tree_id = static_cast<VertexID>(tree_id_bs.to_ulong());
    return pre_tree_id + tree_id;
}



/**
 * Encode an array of ID into a bitstring. How many bits are used as tree_bits needs to be given.
 */
void id_encode(const std::vector<VertexID>& id_array, size_t tree_bits, BitString& bs)
{
    if (id_array.size() == 0) return;

    //Obtain the tree_id of each id and then sort by the tree_id.
    std::vector<IdPair> pair_array;
    for (size_t i = 0; i<id_array.size(); i++)
    {
        IdPair pair;
        pair.id = id_array[i];
        pair.tree_id = id2tree_id(id_array[i], tree_bits);
        pair.pos_of_id = sizeof(VertexID)*8 - len_of_id(id_array[i]) + tree_bits;
        pair_array.push_back(pair);
    }
    std::sort(pair_array.begin(), pair_array.end(), cmp);

    //code the first tree_id as it is.
    VertexID tree_id = pair_array[0].tree_id;
    std::bitset<32> tree_id_bs(tree_id);
    std::string tree_id_str = tree_id_bs.to_string();
    tree_id_str= tree_id_str.substr(32 - tree_bits, tree_bits);
    bs.push_back(tree_id_str);


    std::vector<IdPair> ids_in_a_tree;
    for (size_t i = 0; i < pair_array.size(); i++)
    {
        VertexID curr_tree_id = pair_array[i].tree_id;
        //push all id belong to one tree into an array.
        if (curr_tree_id == tree_id)
        {
            ids_in_a_tree.push_back(pair_array[i]);
        }
        else
        {
            encode_one_tree(ids_in_a_tree, bs);
            encode_tree_id(curr_tree_id, tree_id, bs);
            tree_id = curr_tree_id;
            ids_in_a_tree.clear();
            ids_in_a_tree.push_back(pair_array[i]);
        }
    }
    //encode the final array.
    encode_one_tree(ids_in_a_tree, bs);
}

/**
 * Decode a bitstring to an array of IDs. How many bits are used as tree_id needs to be given.
 */
void id_decode(const BitString& bs, size_t tree_bits, std::vector<VertexID>& id_array)
{
    //read the first tree_id
    if (bs.size() < tree_bits)
    {
        throw DecodeIdError();
    }
    BitString tree_id_bs = bs.substr(0, tree_bits);
    VertexID tree_id = static_cast<VertexID>(tree_id_bs.to_ulong()) + (1<<tree_bits);
    //std::cerr<<tree_id<<std::endl;
    size_t pos = tree_bits;
    try
    {
        pos = decode_one_tree(bs, pos, tree_id, tree_bits, id_array);
    }
    catch (std::exception e)
    {
        throw DecodeIdError();
    }
    while (pos < bs.size())
    {
        try
        {
            tree_id = decode_tree_id(tree_id, bs, &pos);
            //std::cerr<<tree_id<<std::endl;
            pos = decode_one_tree(bs, pos, tree_id, tree_bits, id_array);
        }
        catch (EndOfBitString& e)
        {
            return;
        }
        catch (std::exception& e)
        {
            throw DecodeIdError();
        }
    }
}

/**
 * Write a ID into ostream in binary form.
 */

void write_id_binary(std::ostream& os, VertexID id)
{
    os.write((const char*)&id, sizeof(id));
}

/**
 * Read a ID from an istream in binary form.
 */
VertexID read_id_binary(std::istream& is)
{
    VertexID v_id;
    is.read((char *)&v_id, sizeof(v_id));
    return v_id;
}















