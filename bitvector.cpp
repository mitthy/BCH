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

#include "bitvector.h"

uint32_t BitVector::msb(uint8_t* err) const {
    if(!buffer_) {
        if(err) {
            *err = 1;
        }
        return 0;
    }
    uint8_t* cur = buffer_;
    uint32_t base = (sizeof(*buffer_) * 8) * (size_ - 1);
    uint8_t* end = cur + size_;
    while(cur != end) {
        if(*cur) {
            uint8_t checker = *cur;
            uint8_t calc = 0;
            while(checker) {
                checker >>= 1;
                ++calc;
            }
            if(err) {
                *err = 0;
            }
            return base + calc - 1;
        }
        ++cur;
        base -= sizeof(*buffer_) * 8;
    }
    if(err) {
        *err = 1;
    }
    return 0;
}

template<typename T>
static void swap_(T& left, T& right) {
    T tmp = left;
    left = right;
    right = tmp;
}

BitVector::BitVector(Size size): size_(size.value_), buffer_(size_ ? new uint8_t[size_] : nullptr) {
}

BitVector::BitVector(const BitVector& other): size_(other.size_), buffer_(other.size_ ? new uint8_t[other.size_] : nullptr) {
    for(uint32_t i = 0; i < size_; ++i) {
        buffer_[i] = other.buffer_[i];
    }
}

BitVector::~BitVector() {
    delete[] buffer_;
}

BitVector& BitVector::operator=(const BitVector& other) {
    BitVector tmp(other);
    swap(tmp);
    return *this;
}

BitVector& BitVector::operator&=(const BitVector& other) {
    uint32_t smaller_size = size_ < other.size_ ? size_ : other.size_;
    uint8_t* cur = buffer_ + size_ - 1, *cur_other = other.buffer_ + other.size_ - 1;
    while(smaller_size) {
        *cur &= *cur_other;
        --cur;
        --cur_other;
        --smaller_size;
    }
    uint8_t* end = buffer_ - 1;
    while(cur != end) {
        *cur = 0;
        --cur;
    }
    return *this;
}

BitVector& BitVector::operator|=(const BitVector& other) {
    uint32_t bigget_size = size_ > other.size_ ? size_ : other.size_;
    int32_t diff = bigget_size - size_;
    if(diff > 0) {
        uint8_t* new_buffer = new uint8_t[size_ + diff];
        for(uint32_t  i = 0; i < diff; ++i) {
            new_buffer[i] = 0;
        }
        for(uint32_t i = 0; i < size_; ++i) {
            new_buffer[i + diff] = buffer_[i];
        }
        delete[] buffer_;
        buffer_ = new_buffer;
        size_ += diff;
    }
    uint32_t size_other = other.size_;
    uint8_t* cur = buffer_ + size_ - 1, *cur_other = other.buffer_ + other.size_ - 1;
    while(size_other) {
        *cur |= *cur_other;
        --cur;
        --cur_other;
        --size_other;
    }
    return *this;
}

BitVector& BitVector::operator<<=(uint32_t value) {
    if(!value) {
        return *this;
    }
    uint8_t bytes_are_zero = 0;
    uint32_t msb_ = msb(&bytes_are_zero);
    if(bytes_are_zero) {
        return *this;
    }
    msb_ += 1;
    int32_t excess = value + msb_ - sizeof(*buffer_) * 8 * size_;
    uint32_t extend_range = excess > 0 ? (excess + (sizeof(*buffer_) * 8 - 1)) / (sizeof(*buffer_) * 8) : 0;
    if(extend_range) {
        uint8_t* new_buffer = new uint8_t[size_ + extend_range];
        for(uint32_t i = 0; i < extend_range; ++i) {
            new_buffer[i] = 0;
        }
        for(uint32_t i = 0; i < size_; ++i) {
            new_buffer[i + extend_range] = buffer_[i];
        }
        delete[] buffer_;
        size_ += extend_range;
        buffer_ = new_buffer;
    }
    uint8_t* begin = buffer_ + (size_ - 1) - msb_ / (sizeof(*buffer_) * 8);
    uint8_t* end = buffer_ + size_;
    uint32_t bytes_dif = value / 8;
    uint8_t offset = (value % (sizeof(*buffer_) * 8));
    uint8_t hi_mask = ~((1 << ((sizeof(*buffer_) * 8 - offset))) - 1);
    while(begin != end) {
        uint8_t* dest = begin - bytes_dif;
        uint8_t cur_val = *begin;
        *begin = 0;
        *dest = cur_val;
        *dest <<= offset;
        uint8_t masked = (cur_val & hi_mask) >> ((sizeof(*buffer_) * 8) - offset);
        if(masked) {
            *(dest - 1) |= masked;
        }
        ++begin;
    }
    return *this;
}

BitVector& BitVector::operator>>=(uint32_t value) {
    if(!buffer_ || !value) {
        return *this;
    }
    uint8_t* begin = buffer_ + size_ - 1;
    uint8_t* end = buffer_ - 1;
    uint8_t offset = value % 8;
    uint8_t lo_mask = ((1 << offset) - 1);
    uint8_t bytes_dif = value / 8;
    while(begin != end) {
        uint8_t masked = (*begin & lo_mask) << (sizeof(*buffer_) * 8 - offset);
        uint8_t* dest = begin + bytes_dif;
        *begin >>= offset;
        if(distance_(buffer_, dest) < size_) {
            uint8_t cur_value = *begin;
            *begin = 0;
            *dest = cur_value;
            if(distance_(buffer_,dest + 1) < size_) {
                *(dest + 1) |= masked;
            }
        }
        --begin;
    }
    return *this;
}

BitVector& BitVector::operator^=(const BitVector& other) {
    uint32_t bigget_size = size_ > other.size_ ? size_ : other.size_;
    int32_t diff = bigget_size - size_;
    if(diff > 0) {
        uint8_t* new_buffer = new uint8_t[size_ + diff];
        for(uint32_t  i = 0; i < diff; ++i) {
            new_buffer[i] = 0;
        }
        for(uint32_t i = 0; i < size_; ++i) {
            new_buffer[i + diff] = buffer_[i];
        }
        delete[] buffer_;
        buffer_ = new_buffer;
        size_ += diff;
    }
    uint32_t size_other = other.size_;
    uint8_t* cur = buffer_ + size_ - 1, *cur_other = other.buffer_ + other.size_ - 1;
    while(size_other) {
        *cur ^= *cur_other;
        --cur;
        --cur_other;
        --size_other;
    }
    return *this;
} 

bool BitVector::operator==(const BitVector& other) {
    uint8_t* cur = buffer_ + size_ - 1, *cur_other = other.buffer_ + other.size_ - 1;
    uint8_t* end_cur = buffer_ - 1, *end_other = other.buffer_ - 1;
    while(cur != end_cur && cur_other != end_other) {
        if(*cur != *cur_other) {
            return false;
        }
        --cur;
        --cur_other;
    }
    while(cur != end_cur) {
        if(*cur) {
            return false;
        }
        --cur;
    }
    while(cur_other != end_other) {
        if(*cur_other) {
            return false;
        }
        --cur_other;
    }
    return true;
}
    
bool BitVector::operator!=(const BitVector& other) {
    return !(*this == other);
}

BitVector::bit_access& BitVector::bit_access::operator=(bool value) {
    BitVector* casted = const_cast<BitVector*>(access_vector);
    uint32_t byte_offset = position / 8;
    uint8_t bit_offset = position % 8;
    uint8_t* dest = &casted->buffer_[access_vector->size_ - 1 - byte_offset];
    //Thank you stackoverflow for the following piece of code :)
    *dest ^= (-value ^ *dest) & (1 << bit_offset);
    return *this;
}

BitVector::bit_access::operator bool() const {
    uint32_t byte_offset = position / 8;
    uint8_t bit_offset = position % 8;
    return access_vector->buffer_[access_vector->size_ - 1 - byte_offset] & (1 << bit_offset);
}

BitVector::bit_access BitVector::operator[](uint32_t position) {
    uint32_t byte_offset = position / 8 + 1;
    int32_t diff = byte_offset - size_;
    if(diff > 0) {
        uint8_t* new_buffer = new uint8_t[size_ + diff];
        for(uint32_t  i = 0; i < diff; ++i) {
            new_buffer[i] = 0;
        }
        for(uint32_t i = 0; i < size_; ++i) {
            new_buffer[i + diff] = buffer_[i];
        }
        delete[] buffer_;
        buffer_ = new_buffer;
        size_ += diff;
    }
    BitVector::bit_access ret(this, position);
    return ret;
}

const BitVector::bit_access BitVector::operator[](uint32_t position) const {
    uint32_t byte_offset = position / 8 + 1;
    int32_t diff = byte_offset - size_;
    BitVector* casted = const_cast<BitVector*>(this);
    if(diff > 0) {
        uint8_t* new_buffer = new uint8_t[size_ + diff];
        for(uint32_t  i = 0; i < diff; ++i) {
            new_buffer[i] = 0;
        }
        for(uint32_t i = 0; i < size_; ++i) {
            new_buffer[i + diff] = buffer_[i];
        }
        delete[] buffer_;
        casted->buffer_ = new_buffer;
        casted->size_ += diff;
    }
    BitVector::bit_access ret(this, position);
    return ret;
}

void BitVector::swap(BitVector& other) {
    swap_(size_, other.size_);
    swap_(buffer_, other.buffer_);
}

uint32_t BitVector::size() const {
    //Make it so it doesn't overflow in case size is too big
    return size_ * 8;
}

BitVector::operator bool() const {
    for(uint32_t i = 0; i < size_; ++i) {
        if(buffer_[i]) {
            return true;
        }
    }
    return false;
}

void multiply(BitVector& to_be_multiplied, const BitVector& rhs) {
    BitVector tmp;
    BitVector other(rhs);
    while(other) {
        if(other[0]) {
            tmp ^= to_be_multiplied;
        }
        other >>= 1;
        to_be_multiplied <<= 1;
    }
    to_be_multiplied.swap(tmp);
}

BitVector multiply(const BitVector& left, const BitVector& right) {
    BitVector tmp(left);
    multiply(tmp, right);
    return tmp;
}

division_result long_division(const BitVector& left, const BitVector& right) {
    division_result ret;
    ret.r = left;
    BitVector tmp;
    while(ret.r) {
        int32_t degree_dif = ret.r.msb(nullptr) - right.msb(nullptr);
        if(degree_dif < 0) break;
        tmp = 1;
        tmp <<= degree_dif;
        ret.q ^= tmp;
        ret.r ^= multiply(right, tmp);
    }
    return ret;
}
