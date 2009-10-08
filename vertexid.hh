#ifndef __VERTEX_ID_H__
#define __VERTEX_ID_H__
#include <string>
#include <bitset>
#include <cassert>
#include <iostream>
#include "bitstring.hh"

/*static int mask[32] =
{
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008,
    0x00000010,
    0x00000020,
    0x00000040,
    0x00000080,
    0x00000100,
    0x00000200,
    0x00000400,
    0x00000800,
    0x00001000,
    0x00002000,
    0x00004000,
    0x00008000,
    0x00010000,
    0x00020000,
    0x00040000,
    0x00080000,
    0x00100000,
    0x00200000,
    0x00400000,
    0x00800000,
    0x01000000,
    0x02000000,
    0x04000000,
    0x08000000,
    0x10000000,
    0x20000000,
    0x40000000,
    0x80000000
};*/
/** @file  
 *  The interface to handle vertexID.
 */

/**
 * the VertexID type. Currently is just an alias of unsigned int.
 * But do not rely on this. It can be changed in the future.
 */
typedef unsigned int VertexID;
//Exceptions.
class   DecodeIdError{};
class   EndOfBitString{};

/**
 * to check a given positon of ID is set or not.
 */
bool    idIsSet(VertexID id, size_t pos);
/*{
    int i = 31;
    for (; i >=0; i--)
    {
        if (id & mask[i]) break;
    }
    assert(i-1-pos >= 0);
    return (id & mask[i-1-pos]);
}*/

/** 
 * return the length of the ID, ignoring the leading 1.
 */
inline size_t      len_of_id(VertexID id)
{
    size_t len = 0;
    while (id)
    {
        id >>=1;
        len ++;
    }
    return len - 1;
}

/**
 * convert to a string representation.
 */
std::string to_string(VertexID id);

/**
 * Encode an array of ID into a bitstring. How many bits are used as tree_bits needs to be given.
 */
void        id_encode(const std::vector<VertexID>& id_array, size_t tree_bits, BitString& bs);

/**
 * Decode a bitstring to an array of IDs. How many bits are used as tree_id needs to be given.
 */
void        id_decode(const BitString& bs, size_t tree_bits, std::vector<VertexID>& id_array);

/**
 * Write a ID into ostream in binary form.
 */
void        write_id_binary(std::ostream& os, VertexID id);

/**
 * Read a ID from an istream in binary form.
 */
VertexID        read_id_binary(std::istream& is);
#endif
