#include <stdexcept>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include "bitstring.hh"

BitString::BitString()
        :len_(8), round_(1), f_pos_(8), b_pos_(0)
{
    value_array_.push_back(std::bitset<8>(0));
}


BitString::BitString(const std::string& str)
        :len_(8), round_(1), f_pos_(8), b_pos_(0)
{
    value_array_.push_back(std::bitset<8>(0));
    push_back(str);
}

BitString::BitString(const BitString& bstr)
        :len_(8), round_(1), f_pos_(8), b_pos_(0)
{
    value_array_.push_back(std::bitset<8>(0));
    push_back(bstr);
}




BitString::BitString(const char* cstr)
        :len_(8), round_(1), f_pos_(8), b_pos_(0)
{
    value_array_.push_back(std::bitset<8>(0));
    push_back(cstr);
}

const BitString&
BitString::operator=(const BitString& bstr)
{
    clear();
    push_back(bstr);
    return bstr;
}

void
BitString::clear()
{
    len_ = 8;
    round_ = 1;
    b_pos_ = 0;
    f_pos_ = 8;
    value_array_.clear();
    value_array_.push_back(std::bitset<8>(0));
}

bool
BitString::test (size_t pos) const
{
    if (pos + f_pos_ >= len_)
    {
        throw InvalidParameter();
    }
    size_t round = (pos + f_pos_) /8;
    unsigned short res = (pos + f_pos_) - 8 * round;
    //in std::bitset, the pos 0 is at the rightmost.
    //We need the pos 0 to be the leftmost.
    return value_array_[round].test(7 - res);

}

std::string BitString::to_string() const
{
    std::string str;
    if (f_pos_ < 8)
    {
        str.append(value_array_[0].to_string().substr(f_pos_));
    }
    for (size_t i = 1; i< round_; i++)
    {
        str.append(value_array_[i].to_string());
    }
    if (b_pos_ > 0)
    {
        str.append(value_array_[round_].to_string().substr(0, b_pos_));
    }
    return str;
}

unsigned long BitString::to_ulong() const
{
    if (len_ - f_pos_ > sizeof(unsigned long)*8)
    {
        throw TooBigForUlong();
    }
    std::bitset<sizeof(unsigned long)*8> bs(to_string());
    return bs.to_ulong();
}

void BitString::push_back_bit(bool value)
{
    if (b_pos_ == 0)
    {
        if (value)
        {
            std::bitset<8> bs(128);
            value_array_.push_back(bs);
        }
        else
        {
            std::bitset<8> bs(0);
            value_array_.push_back(bs);
        }
        ++len_;
        ++b_pos_;
    }
    else
    {
        if (value)
        {
            value_array_[round_].set(7 - b_pos_);
        }
        ++b_pos_;
        if (b_pos_ > 7)
        {
            ++round_;
            b_pos_ = 0;
        }
        ++len_;
    }
}

void BitString::push_front_bit(bool value)
{
    if (f_pos_ == 0)
    {
        throw CannotPushFront();
    }
    else
    {
        if (value)
        {
            value_array_[0].set(8 - f_pos_);
        }
        --f_pos_;
        ++len_;
    }
}

void BitString::pop_back_bit()
{
    if (b_pos_ == 1)
    {
        value_array_.pop_back();
        b_pos_ = 0;
        len_--;
    }
    else if (b_pos_ == 0)
    {
        b_pos_ = 7;
        len_ --;
        round_--;
        value_array_[round_].set(0, 0);
    }
    else
    {
        value_array_[round_].set(8 - b_pos_, 0);
        b_pos_ --;
        len_--;
    }
}

void BitString::pop_front_bit()
{
    if (f_pos_ == 8)
    {
        throw CannotPopFront();
    }
    else
    {
        value_array_[0].set(7 - f_pos_, 0);
        f_pos_ ++;
        len_ ++;
    }
}

void BitString::push_back(const std::string& str)
{
    if (b_pos_ == 0)
    {

        size_t l = str.size();
        size_t r = l / 8;
        unsigned short res = l - 8 * r;
        try
        {
            for (size_t i = 1; i <= r ; i++)
            {
                std::bitset<8> bits(str, (i-1)*8, 8);
                value_array_.push_back(bits);
            }
            if (res > 0)
            {
                std::string str2(str);
                str2.append(8-res, '0');
                std::bitset<8> bits(str2, r*8, 8);
                value_array_.push_back(bits);
            }
            round_ += r;
            len_ += l;
            b_pos_ = res;
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            throw InvalidParameter();
        }
    }
    else if (str.size() <=(unsigned short)(8 - b_pos_))
    {
        for (unsigned char i = 0; i < str.size(); i++)
        {
            if (str[i] == '1')
            {
                push_back_bit(true);
            }
            else if (str[i] == '0')
            {
                push_back_bit(false);
            }
            else
            {
                throw InvalidParameter();
            }
        }
    }
    else
    {
        unsigned char number = 8 - b_pos_;
        for (unsigned char i=0; i< number; i++)
        {
            if (str[i] == '1')
            {
                push_back_bit(true);
            }
            else if (str[i] == '0')
            {
                push_back_bit(false);
            }
            else
            {
                throw InvalidParameter();
            }
        }
        push_back(str.substr(number));
    }
}


void BitString::push_back(const char* cstr)
{
    std::string str = std::string(cstr);
    push_back(str);
}

BitString
BitString::substr(unsigned int pos, unsigned int len) const
{
    BitString bstr;
    for (size_t i = 0; i < len; i++)
    {
        bstr.push_back_bit(test(pos+i));
    }
    //return by value. It is not a bug here.
    //But certainly it is not efficient, especially
    //if the substring is very long.
    return bstr;
}

BitString
BitString::substr(unsigned int pos) const
{
    std::string str(to_string().substr(pos));
    BitString bstr(str);
    return bstr;
}

void BitString::push_back(unsigned char value)
{
    if (b_pos_==0)
    {
        value_array_.push_back(std::bitset<8>(value));
        len_ += 8;
        ++round_;
    }
    else
    {
        std::bitset<8> bs(value);
        push_back(bs.to_string());
    }
}

std::vector<unsigned char> BitString::to_array() const
{
    if (f_pos_ == 8)
    {
        std::vector<unsigned char> v;
        for (size_t i = 1; i < round_; i++)
        {
            v.push_back((unsigned char)value_array_[i].to_ulong());
        }
        if (b_pos_ != 0)
        {
            v.push_back((unsigned char)value_array_[round_].to_ulong());
        }
        return v;
    }
    else
    {
        BitString bstr(to_string());
        return bstr.to_array();
    }
}

int BitString::write_binary(char* buffer, bool writeLength) const
{

    std::vector<unsigned char> value_array = to_array();
    std::vector<unsigned char>::iterator it = value_array.begin();
    std::vector<unsigned char>::const_iterator end = value_array.end();
    char* ptr = buffer;
    int   len = 0;
    unsigned char  high = size()/256;
    unsigned char  low  = size() - high*256;
    if (writeLength)
    {
        memcpy(ptr, &high, sizeof(high));
        ptr+= sizeof(high);
        memcpy(ptr, &low,  sizeof(low));
        ptr+= sizeof(low);
        len += 2;
    }
    for (; it != end; ++it)
    {
        memcpy(ptr, &(*it), sizeof(unsigned char));
        ptr++;
        len += sizeof(unsigned char);
    }
    return len;
}

int BitString::write_binary(std::ostream& ofs, bool writeLength) const
{
    std::vector<unsigned char> value_array = to_array();
    std::vector<unsigned char>::iterator it = value_array.begin();
    std::vector<unsigned char>::const_iterator end = value_array.end();
    unsigned char  high = size()/256;
    unsigned char  low  = size() - high*256;
    int len = 0;
    if (writeLength)
    {
        ofs.put(high);
        ofs.put(low);
        len += 2;
    }
    for (; it != end; ++it)
    {
        ofs.put(*it);
        len ++;
    }
    return len;
}

void BitString::read_binary(const char* buffer, unsigned int len)
{
    unsigned int round = len /8;
    unsigned int res = len - round*8;
    for (unsigned int i = 0; i<round; i++)
    {
        push_back((unsigned char)(*buffer));
        ++buffer;
    }
    if (res > 0)
    {
        push_back((unsigned char)(*buffer));
        ++buffer;
        len_-= (8-res);
        b_pos_ += res;
        if (b_pos_ < 8)
        {
            --round_;
        }
        else
        {
            b_pos_ -= 8;
        }
    }
}

void BitString::read_binary(const char* buffer)
{
    unsigned char high = static_cast<unsigned char>(*buffer);
    ++buffer;
    unsigned char low = static_cast<unsigned char>(*buffer);
    ++buffer;
    unsigned int len = high*256+low;

    unsigned int round = len /8;
    unsigned int res = len - round*8;
    for (unsigned int i = 0; i<round; i++)
    {
        push_back((unsigned char)(*buffer));
        ++buffer;
    }
    if (res > 0)
    {
        push_back((unsigned char)(*buffer));
        ++buffer;
        len_-= (8-res);
        b_pos_ += res;
        if (b_pos_ < 8)
        {
            --round_;
        }
        else
        {
            b_pos_ -= 8;
        }
    }
}

void BitString::read_binary(std::istream& ifs, unsigned int len)
{
    unsigned int round = len / 8;
    int res = len - round*8;
    try
    {
        for (unsigned int i = 0; i< round; i++)
        {
            char value;
            ifs.get(value);
            push_back((unsigned char)value);
        }
        if (res > 0)
        {
            char value;
            ifs.get(value);
            push_back((unsigned char)value);
            len_ -= (8-res);
            b_pos_ += res;
            if (b_pos_  < 8)
            {
                --round_;
            }
            else
            {
                b_pos_ -= 8;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr<<e.what()<<std::endl;
        return;
    }
}

void BitString::read_binary(std::istream& ifs)
{
    char high;
    ifs.get(high);
    char low;
    ifs.get(low);
    unsigned int len = static_cast<unsigned char>(high)*256+ static_cast<unsigned char>(low);


    unsigned int round = len / 8;
    int res = len - round*8;
    try
    {
        for (unsigned int i = 0; i< round; i++)
        {
            char value;
            ifs.get(value);
            push_back((unsigned char)value);
        }
        if (res > 0)
        {
            char value;
            ifs.get(value);
            push_back((unsigned char)value);
            len_ -= (8-res);
            b_pos_ += res;
            if (b_pos_  < 8)
            {
                --round_;
            }
            else
            {
                b_pos_ -= 8;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr<<e.what()<<std::endl;
        return;
    }
}
void BitString::push_back(const BitString& bs)
{
    //for simplicity now. Can be updated for efficiency.
    for (size_t i = 0; i< bs.size(); i++)
    {
        push_back_bit(bs.test(i));
    }
}

void BitString::reverse(BitString& bs) const
{
    std::ostringstream ostr;
    for (size_t i = b_pos_; i>0; --i)
    {
        ostr<<value_array_[round_][8-i];
    }
    for (size_t i = round_-1; i>0; --i)
    {
        for (int j = 0; j<8; ++j)
        {
            ostr<<value_array_[i][j];
        }
    }
    for (size_t i = 0; i< (unsigned short)(8 - f_pos_); ++i)
    {
        ostr<<value_array_[0][i];
    }
    bs.push_back(ostr.str());
}


std::ostream& operator<< (std::ostream& os , const BitString& bs)
{
    return os<<bs.to_string();
};

bool BitString::operator[] (size_t pos) const
{
    return test(pos);
}










