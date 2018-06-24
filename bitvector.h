/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <cstdint>
#include <iostream>
    
template<typename Iterator>
uint32_t distance_(Iterator begin, Iterator end) {
    return end - begin;
}

struct Size {
    Size(uint32_t val): value_(val) {}
    uint32_t value_;
};

/**
 * @todo write docs
 */
class BitVector {        
public:
    
    typedef uint8_t* iterator;
    
    typedef const uint8_t* const_iterator;
    
    template<typename T>
    BitVector(T value): size_(sizeof(value)), buffer_(new uint8_t[size_]) {
        uint8_t* cur = buffer_ + size_ - 1;
        for(uint8_t i = 0; i < size_; ++i) {
            *cur = value & ((1 << sizeof(buffer_)) - 1);
            --cur;
            value >>= 8;
        }
    }
    
    BitVector(): size_(1), buffer_(new uint8_t[size_]) {
        *buffer_ = 0;
    }
    
    explicit BitVector(Size sz);
    
    template<typename Iterator>
    BitVector(Iterator begin, Iterator end);
    
    BitVector(const BitVector&);
    
    ~BitVector();
    
    BitVector& operator=(const BitVector&);
    
    template<typename T>
    BitVector& operator=(T value) {
        int32_t diff = sizeof(value) - size_;
        if(diff > 0) {
            size_ += diff;
            uint8_t* new_buffer = new uint8_t[size_];
            delete[] buffer_;
            buffer_ = new_buffer;
        }
        uint8_t* cur = buffer_ + size_ - 1;
        uint8_t* end = buffer_ - 1;
        for(uint8_t i = 0; i < sizeof(value); ++i) {
            *cur = value & ((1 << sizeof(buffer_)) - 1);
            --cur;
            value >>= 8;
        }
        while(cur != end) {
            *cur = 0;
            --cur;
        }
        return *this;
    }
    
    BitVector& operator&=(const BitVector&);
    
    BitVector& operator|=(const BitVector&);
    
    BitVector& operator<<=(uint32_t);
    
    BitVector& operator>>=(uint32_t);
    
    BitVector& operator^=(const BitVector&);
    
    bool operator==(const BitVector&);
    
    bool operator!=(const BitVector&);
    
    operator bool() const;
    
    BitVector operator&(const BitVector& other) const {
        BitVector tmp(*this);
        tmp &= other;
        return tmp;
    }
    
    BitVector operator|(const BitVector& other) const {
        BitVector tmp(*this);
        tmp |= other;
        return tmp;
    }
    
    BitVector operator<<(uint32_t value) const {
        BitVector tmp(*this);
        tmp <<= value;
        return tmp;
    }
    
    BitVector operator>>(uint32_t value) const {
        BitVector tmp(*this);
        tmp >>= value;
        return tmp;
    }
    
    BitVector operator^(const BitVector& other) const {
        BitVector tmp(*this);
        tmp ^= other;
        return tmp;
    }
    
    friend class bit_access;
    
    struct bit_access {
        bit_access(const BitVector* const vector, uint32_t pos): access_vector(vector), position(pos) {}
        
        bit_access& operator=(bool);
        
        operator bool() const;
        
        const BitVector* const access_vector;
        
        uint32_t position;
    };
    
    bit_access operator[](uint32_t);
    
    const bit_access operator[](uint32_t) const;
    
    void swap(BitVector&);
    
    template<typename Iterator>
    void reset(Iterator begin, Iterator end);
    
    uint32_t msb(uint8_t* err = nullptr) const;
    
    uint32_t size() const;
    
    iterator begin() {
        return buffer_;
    }
    
    iterator end() {
        return buffer_ + size_;
    }
    
    const_iterator begin() const {
        return buffer_;
    }
    
    const_iterator end() const {
        return buffer_ + size_;
    }
    
private:
    uint32_t size_;
    
    uint8_t* buffer_;
};

template<typename Iterator>
BitVector::BitVector(Iterator begin, Iterator end): size_(0), buffer_(nullptr) {
    reset(begin, end);
}

template<typename Iterator>
void BitVector::reset(Iterator begin, Iterator end) {
    delete[] buffer_;
    size_ = distance_(begin, end);
    if(size_) {
        buffer_ = new uint8_t[size_];
    }
    else {
        buffer_ = nullptr;
    }
    for(uint32_t i = 0; i < size_; ++i) {
        buffer_[i] = *begin;
        ++begin;
    }
}

void multiply(BitVector& to_be_multiplied, const BitVector& other);

BitVector multiply(const BitVector& left, const BitVector& right);

struct division_result {
    BitVector q;
    BitVector r;
};

division_result long_division(const BitVector&, const BitVector&);


#endif // BITVECTOR_H
