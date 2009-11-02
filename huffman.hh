// =============================================================================
// Written by Cheng Wei
// rivercheng@gmail.com
// 5 Aug 2008
// =============================================================================
#ifndef __HUFFMAN_HH__
#define __HUFFMAN_HH__
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <iostream>
#include "bitstring.hh"

namespace Huffman
{

enum Tag {l, r};


//The exceptions;
class CannotFind {};
class DecodeError {};
class EmptyInput {};

/*
 * The Huffman table for encoding.
 */
template <typename T>
struct HuffmanEncodeTable
{
    typedef std::map<T, BitString> Table;
};

/*
 * The frequency table used to create huffman table.
 */
template <typename T>
struct FrequencyTable
{
    typedef std::map<T, size_t> Table;
};

/**
 * The huffman tree used in decoding.
 */
template <typename T>
struct DecodeTree
{
    BitString tree_str;
    std::vector<T> value_array;
};

/*
 * Huffman Node stores the value frequency and the code.
 */
template <typename T>
class HuffmanNode
{
public:
    //------------constructors------------------------
    /**
     * the default constructor.
     */
    HuffmanNode(void)
    {
        left_child_ = 0;
        right_child_ = 0;
    }

    /**
     * the constructor from a value and a frequency.
     */
    HuffmanNode(T value , size_t frequency)
    {
        value_ = value;
        frequency_ = frequency;
        left_child_ = 0;
        right_child_ = 0;
    }

    /**
     * the constructor from two children.
     */
    HuffmanNode(HuffmanNode& left, HuffmanNode& right)
    {
        frequency_ = left.frequency() + right.frequency();
        left_child_ = &left;
        right_child_ = &right;
        left.label(l);
        right.label(r);
    }
    
    /**
     * the deconstructor.
     */
    ~HuffmanNode()
    {
        if (left_child_) delete left_child_;
        if (right_child_) delete right_child_;
    }


    //------------Get functions-----------------
    /**
     * return the value of a node.
     */
    T value() const
    {
        return value_;
    }

    /**
     * return the frequency of a node.
     */
    size_t frequency() const
    {
        return frequency_;
    }

    /**
     * return the code of a node.
     */
    BitString code() const
    {
        return code_;
    }

    //----------------help functions-----------------
    /**
     * label nodes to generate Huffman code.
     */
    void label(Tag tag)
    {
        if (tag == l)
        {
            code_.push_back_bit(false);
        }
        else
        {
            code_.push_back_bit(true);
        }
        if (left_child_)
        {
            left_child_->label(tag);
        }
        if (right_child_)
        {
            right_child_->label(tag);
        }
    }

    /**
     * to check whether a node is a leaf.
     */
    bool isLeaf() const
    {
        return left_child_ == NULL && right_child_ == NULL;
    }

    /**
     * return the left child.
     */
    HuffmanNode* left() const
    {
        return left_child_;
    }

    /**
     * return the right child
     */
    HuffmanNode* right() const
    {
        return right_child_;
    }
private:
    T value_;
    size_t frequency_;
    BitString code_;
    HuffmanNode* left_child_;
    HuffmanNode* right_child_;
};

//compare function class used in priority queue.
//To compare two points of Huffman nodes.
//The comparison is based on the frequency of the nodes.
//Node with higher frequency has lower priority.
template <typename T>
class Compare : std::binary_function< HuffmanNode<T>*, HuffmanNode<T>*, bool >
{
public:
    bool operator()(const HuffmanNode<T>*a, const HuffmanNode<T>*b) const
    {
        return a->frequency() > b->frequency();
    }
}; //used to replace the default Less() in the priority queue.

/**
 * The Huffman Coder class.
 */
template <typename T>
class HuffmanCoder
{
public:
    //----------------Constructors -------------------------
    /**
     * constructor from an array of input values.
     */
    HuffmanCoder(std::vector<T> & input_array)
    {
        //----------generate frequency table------------
        typename FrequencyTable<T>::Table f_table;
        typename std::vector<T>::const_iterator input_it = input_array.begin();
        typename std::vector<T>::const_iterator input_end = input_array.end();
        typename FrequencyTable<T>::Table::const_iterator f_table_it;

        if (input_it == input_end)
        {
            throw EmptyInput();
        }


        for (input_it = input_array.begin(); input_it != input_array.end(); ++input_it)
        {
            f_table_it = f_table.find(*input_it);
            if (f_table_it == f_table.end()) //new entry
            {
                f_table[*input_it] = 1;
            }
            else                            //existed entry
            {
                f_table[*input_it] ++;
            }
        }

        //-----------generate the original heap---------
        std::priority_queue<HuffmanNode<T>*, std::vector<HuffmanNode<T>*>, Compare<T> > node_queue;

        //to store the original nodes so we can output the encoding table quickly.
        std::vector< HuffmanNode<T> *> node_array;
        typename std::vector < HuffmanNode<T> *>::const_iterator node_it;

        for (f_table_it = f_table.begin(); f_table_it != f_table.end(); ++f_table_it)
        {
            HuffmanNode<T>* p_node = new HuffmanNode<T>(f_table_it->first, f_table_it->second);
            node_queue.push(p_node);
            node_array.push_back(p_node);
        }

        //----------generate the Huffman tree-----------
        while (node_queue.size() > 1)
        {
            HuffmanNode<T>* l_node = NULL;
            HuffmanNode<T>* r_node = NULL;
            l_node = node_queue.top();
            node_queue.pop();

            r_node = node_queue.top();
            node_queue.pop();

            HuffmanNode<T>* new_node = new HuffmanNode<T>(*l_node, *r_node);
            node_queue.push(new_node);
        }

        //-------generate the huffman table-------------------------
        for (node_it = node_array.begin(); node_it != node_array.end(); ++node_it)
        {
            BitString str_i;
            str_i.push_back((*node_it)->code().to_string());
            BitString str;
            str_i.reverse(str);
            huffman_encode_table_[(*node_it)->value()] = str;
        }


        first_ = node_queue.top();
        node_queue.pop();
        //for debug
        output_table();
    };


    /**
     * constructor from a Huffman table.
     * No huffman tree is created, so this coder can only be used to encode.
     * We assume the table is read from a file.
     */
    HuffmanCoder(typename HuffmanEncodeTable<T>::Table& table)
    {
        huffman_encode_table_ = table;
    }

    /**constructor from a huffman tree.
     * No encode table needs to be created. 
     * (It is doable, but now is ignored for simplicity and efficiency).
     * So it can only be used for decoding.
     * The explanation of the tree can be seen in the explanation of function
     * output_tree();
     */
    HuffmanCoder(DecodeTree<T>& tree)
    {
        int str_pos = 0;
        int value_pos = 0;
        std::vector< HuffmanNode<T> *> node_array;
        first_ = &createNode(tree.tree_str, tree.value_array, &str_pos, &value_pos);
        //creating encode table
        traverse(first_);
    }

    /**constructor from a huffman tree represented as a bitstring followed 
     * by a value array.
     */
    HuffmanCoder(BitString& str, std::vector<T>& value_array)
    {
        int   str_pos = 0;
        int   value_pos = 0;
        first_ = &createNode(str, value_array, &str_pos, &value_pos);
    }

    /**
     * the destructor.
     */
    ~HuffmanCoder()
    {
        delete first_;
    };

    /**
     * encode a value to a bitstring.
     */
    void encode(T value, BitString& str)
    {
        str.push_back(huffman_encode_table_[value]);
    }

    /**
     * encode an array of value to a bitstring.
     */
    void encode(std::vector<T>& input_array, BitString& str)
    {
        typename std::vector<T>::const_iterator it = input_array.begin();
        typename std::vector<T>::const_iterator end = input_array.end();
        for ( ; it!=end; ++it)
        {
            str.push_back(huffman_encode_table_[*it]);
        }
    }

    /**
     * deocde a bitstring to an array of value.
     * Begin from the given position and decode given number of values.
     * If number is 0 or not given, all the bitstring is decoded.
     * Return the real number of values been decoded.
     */
    size_t decode(const BitString& str, std::vector<T>& output_array, size_t* p_string_pos, size_t number = 0)
    {
        size_t o_string_pos = *p_string_pos;
        size_t n  = 0;
        T value;
        if (*p_string_pos >= str.size())
        {
            std::cerr<<"position is wrong"<<std::endl;
            return 0;
        }
        while (1)
        {
            try
            {
                value = check(first_, str, p_string_pos);
            }
            catch (CannotFind& e)
            {
                *p_string_pos = o_string_pos;
                return n;
            }
            output_array.push_back(value);
            o_string_pos = *p_string_pos;
            ++n;
            if (n == number)
            {
                return n;
            }
        }
        return n;
    }


    /**
     * decode an array of bitstring.
     */
    size_t decode(const std::vector<BitString> & str_vec, std::vector<T>& output_array, size_t* p_array_pos, size_t* p_string_pos, size_t number = 0)
    {
        size_t o_array_pos = *p_array_pos;
        size_t o_string_pos = *p_string_pos;
        size_t n  = 0;
        T value;
        if (*p_array_pos >= str_vec.size() || *p_string_pos >= str_vec[*p_array_pos].size())
        {
            std::cerr<<"position is wrong"<<std::endl;
            return 0;
        }
        while (1)
        {
            try
            {
                value = check(first_, str_vec, p_array_pos, p_string_pos);
            }
            catch (CannotFind& e)
            {
                *p_array_pos = o_array_pos;
                *p_string_pos = o_string_pos;
                return n;
            }
            output_array.push_back(value);
            o_array_pos = *p_array_pos;
            o_string_pos = *p_string_pos;
            ++n;
            if (n == number)
            {
                return n;
            }
        }
        return n;
    }

    /**
     * To output the decoding tree.
     *'0' means a leaf node
     * '1' means a non-leaf node, the first part of following bits are for
     * its left child tree, and the second part of following bits are for its
     * right child tree.
     * For example:
     * 110011000 represents a graph as follows:
     * <DFN>
     *                O (1)
     *
     *             |     |
     *
     *         (1)O        O (1)
     *
     *           | |     |   |
     *
     *      (0) O  O(0) O(1) O(0)
     *
     *                 | |
     *
     *             (0)O   O(0)
     *
     * </DFN>
     * Every value in the value array represent a 0 in the bit string.
     */
    void output_tree(DecodeTree<T>& tree)
    {
        output_tree(tree.tree_str, tree.value_array);
    }

    /**
     * output the tree as a bitstring and an array of value.
     */
    void output_tree(BitString& str, std::vector<T>& value_array)
    {
        if (first_ == NULL) return;
        output_tree(first_, str, value_array);
    }

    //for internal usage. A recursive way to output a tree.
    void output_tree(HuffmanNode<T>* first, BitString& str, std::vector<T>& value_array)
    {
        if (first->isLeaf())
        {
            str.push_back_bit(false);
            value_array.push_back(first->value());
        }
        else
        {
            str.push_back_bit(true);
            output_tree(first->left(), str, value_array);
            output_tree(first->right(), str, value_array);
        }
        return;
    }


    //output the encoding table. Mainly for debug.
    void output_table()
    {
        typename HuffmanEncodeTable<T>::Table::iterator it;
        std::cerr << "Huffman table" <<std::endl;
        for (it = huffman_encode_table_.begin(); it != huffman_encode_table_.end(); ++it)
        {
            std::cerr<<it->first<<" "<<it->second<<std::endl;
        }
    }

private:
    //The main function for creating a huffman tree from a decoding tree.
    //Recursive method is used.
    //for '0' in the bitstring, we create a leaf node.
    //for '1' in the bitstring, we create a parent node with two subtrees.
    HuffmanNode<T>& createNode(BitString& str, std::vector<T>& value_array, int* str_pos, int* value_pos)
    {
        if (!str.test(*str_pos))
        {
            HuffmanNode<T>* node = new HuffmanNode<T>(value_array[*value_pos], 0);
            ++(*str_pos);
            ++(*value_pos);
            return *node;
        }
        else
        {
            ++(*str_pos);
            //We assume the evaluation order in the arguments of a funtion is
            //from left to right. Otherwise, there is a problem. Have not checked
            //the C++ standard yet.
            HuffmanNode<T>* node = new HuffmanNode<T>(createNode(str, value_array, str_pos, value_pos), createNode(str, value_array, str_pos, value_pos));
            return *node;
        }
    }

    //The main function for decoding. Recursive method is used here.
    //If we arrive a leaf node, then decoding is done we return the value.
    //If we arrive a non-leaf node, then check left tree if next bit is 0
    //or right tree if next bit is right. If no next bit exists, we throw
    //a cannot find exception.
    T check(HuffmanNode<T>*p, const std::vector<BitString>& str_vec, size_t* p_array_pos, size_t* p_string_pos)
    {
        if (p->isLeaf())
        {
            return p->value();
        }
        if (*p_string_pos >= str_vec[*p_array_pos].size())
        {
            if ((*p_array_pos) + 1 >= str_vec.size())
            {
                throw CannotFind();
            }
            else
            {
                ++(*p_array_pos);
                *p_string_pos = 0;
            }
        }
        bool right = str_vec[*p_array_pos].test(*p_string_pos);
        ++(*p_string_pos);
        if (right)
        {

            return check(p->right(), str_vec, p_array_pos, p_string_pos);
        }
        else
        {
            return check(p->left(), str_vec, p_array_pos, p_string_pos);
        }
    }

    void traverse(HuffmanNode<T>*p)
    {
        if (p->isLeaf())
        {
            BitString bstr;
            p->code().reverse(bstr);
            huffman_encode_table_[p->value()]= bstr;
        }
        else
        {
            traverse(p->left());
            traverse(p->right());
        }
    }

    T check(HuffmanNode<T>*p, const BitString& str, size_t* p_string_pos)
    {
        if (p->isLeaf())
        {
            return p->value();
        }
        if (*p_string_pos >= str.size())
        {
            throw CannotFind();
        }
        bool right = str.test(*p_string_pos);
        ++(*p_string_pos);
        if (right)
        {

            return check(p->right(), str, p_string_pos);
        }
        else
        {
            return check(p->left(), str, p_string_pos);
        }
    }

private:
    typename HuffmanEncodeTable<T>::Table huffman_encode_table_;
    HuffmanNode<T>* first_;

};
}
#endif
