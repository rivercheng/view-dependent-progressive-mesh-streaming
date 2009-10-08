#ifndef __BITSTRING_H__
#define __BITSTRING_H__

#include <string>
#include <bitset>
#include <vector>
#include <iostream>
#include <fstream>
/**
 * A class to support bitstring. With this class we can generate data with
 * arbitrary number of bits. It is used in encoding and decoding.
 */

class BitString
{
    /**
     * The output operator.
     */
    friend std::ostream& operator<< (std::ostream&, const BitString&);
public:
    //----------Constructors----------------------------
    /**
     * default Contructor.
     */
    BitString(void);

    /**
     * Contructor from a STL string.
     */
    BitString(const std::string& str);

    /**
     * Constructor from a C string.
     */
    BitString(const char* cstr);
    /**
     * Copy constructor.
     */
    BitString(const BitString& bstr);

    //----------------Operator--------------------
    /**
     * Assign operator.
     */
    const BitString& operator=(const BitString& bstr);


    //---------------Methods----------------------

    /**
     * to test whether a bit is 0 (false) or 1 (true).
     */
    bool operator[](size_t pos) const;

    /**
     * The same with [].
     */
    bool test(size_t pos) const;

    //-------------convert to other representations--------

    /**
     * convert to STL string.
     */
    std::string to_string(void) const;

    /**
     * convert to a integer value. Cannot be used on a bitstring longer than
     * 64. Otherwise, an exception TooBigForUlong will be thrown.
     */
    unsigned long to_ulong(void) const;

    /**
     * convert to a array of unsigned char.
     */
    std::vector< unsigned char> to_array(void) const;

    //--------------push a bit to the bitstring ------
    /**
     * push a bit true (1) or false (0) to the front.
     */
    void push_front_bit(bool value);

    /**
     * push a bit true (1) or false (0) to the back.
     */
    void push_back_bit(bool value);
    
    //---------------Remove value from bitstring -----
    /**
     * remove a value from the front.
     * It does not return the deleted value. It seems
     * weird, but STL vector does so and we just follow it.
     */
    void pop_front_bit(void);

    /**
     * remove a value from the back.
     */
    void pop_back_bit(void);

    //---------------Push a value to the bitstring-----
    /**
     * push a 8-bit value to the back.
     */
    void push_back(unsigned char value);

    /**
     * push a STL string to the back.
     */
    void push_back(const std::string& str);

    /**
     * push a C string to the back.
     */
    void push_back(const char* cstr);

    /**
     * push a BitString value the the back.
     */
    void push_back(const BitString&);

    //---------------Clear a bitstring--------------
    /**
     * clear the bitstring to empty state.
     */
    void clear();


    //--------------Generate sub-strings ------------
    /**
     * output a sub-string of a bitstring from pos and the length is len
     */
    BitString substr(unsigned int pos, unsigned int len) const;

    /**
     * output a sub-string of a bitstring from pos to the end.
     */
    BitString substr(unsigned int pos) const;

    //---------------Reverse a bitstring---------------
    /**
     * reverse the order of a bitstring.
     */
    void reverse(BitString&) const;

    //---------------Output a bitstring ---------------
    /**
     * Write a bitstring to ostream. If writeLength = true, then the
     * length will be written before the bitstring.
     */
    int  write_binary(std::ostream& ofs, bool writeLength=false) const;

    /**
     * Write a bitstring to a buffer. If writeLength = true, then the
     * length will be written before the bitstring.
     */
    int  write_binary(char* buffer, bool writeLength=false) const;

    //---------------Input a bitstring-------------------
    /**
     * Read a bitstring from an istream with given length.
     * The bitstring has to be previously written by set
     * writeLength as false
     */
    void read_binary(std::istream& ifs, unsigned int len);

    /**
     * Read a bitstring from an istream and assume that the
     * length can be read before the bitstring. The bitstring
     * has to be written previously with writelength set as true
     */
    void read_binary(std::istream& ifs);

    /**
     * read a bitstring (without length) from a buffer with
     * given length.
     */
    void read_binary(const char* buffer, unsigned int len);

    /**
     * read a bitstring (with length) from a buffer.
     */
    void read_binary(const char* buffer);

    //-------------get functions-----------------------
    size_t size()const
    {
        return len_ - f_pos_;
    }

    //-----------  exceptions   -----------------------
    class InvalidParameter{};
    class TooBigForUlong{};
    class CannotPushFront{};
    class CannotPopFront{};

private:
    //have to keep this order.
    size_t len_;
    size_t round_;
    unsigned short f_pos_;
    unsigned short b_pos_;
    //-----------------------
    std::vector< std::bitset<8> > value_array_;
    std::vector< std::bitset<8> >::iterator it_;
};


#endif

