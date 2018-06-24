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

#include "galoisfield.h"

uint8_t GaloisField::primitive_polinomial_[8] = {0, 0, 0b00000011, 0b00000011, 0, 0, 0, 0b00011101};
GaloisField::count_log_anti_log_tables GaloisField::tables_[8] = {};

static uint8_t degree(uint8_t number) {
    uint8_t ret = 0;
    while(number) {
        number >>= 1;
        ++ret;
    }
    return ret;
}

template<typename T>
static void swap_(T& left, T& right) {
    T tmp = left;
    left = right;
    right = tmp;
} 

GaloisField::GaloisField(const GaloisField& gf): size_(gf.size_), number_(gf.number_) {
    gen_log_tables_();
}

GaloisField::GaloisField(uint8_t size): size_(size), number_(0) {
    gen_log_tables_();
}

GaloisField::GaloisField(uint8_t size, uint8_t number): size_(size), number_(0) {
    while(size) {
        number_ |= (number & (1 << (size - 1)));
        --size;
    }
    gen_log_tables_();
}

GaloisField& GaloisField::operator=(uint8_t number) {
    uint8_t size = size_;
    number_ = 0;
    while(size) {
        number_ |= (number & (1 << (size - 1)));
        --size;
    }
    return *this;
}

GaloisField& GaloisField::operator=(const GaloisField& other) {
    GaloisField tmp(other);
    swap(tmp);
    return *this;
}

void GaloisField::swap(GaloisField& other) {
    swap_(this->size_, other.size_);
    swap_(this->number_, other.number_);
}

GaloisField::~GaloisField() {
    --tables_[size_ - 1].count;
    if(!tables_[size_ - 1].count) {
        delete[] tables_[size_ - 1].log_table;
        delete[] tables_[size_ - 1].anti_log_table;
    }
}

GaloisField& GaloisField::operator*=(const GaloisField& rhs) {
    if(size_ == rhs.size_) {
        number_ = multiply_(number_, rhs.number_);
    }
    return *this;
}

GaloisField& GaloisField::GaloisField::operator/=(const GaloisField& rhs) {
    if(size_ == rhs.size_) {
        uint8_t q, r;
        long_division_(rhs, q, r);
        number_ = q;
    }
    return *this;
}

GaloisField & GaloisField::operator%=(const GaloisField& rhs) {
    if(size_ == rhs.size_) {
        uint8_t q, r;
        long_division_(rhs, q, r);
        number_ = r;
    }
    return *this;
}

void GaloisField::long_division_(const GaloisField& rhs, uint8_t& q, uint8_t& r) {
    uint8_t q_ = 0;
    uint8_t r_ = number_;
    uint8_t tmp = 0;
    while(r_) {
        int16_t degree_dif = static_cast<int16_t>(degree(r_)) - static_cast<int16_t>(degree(rhs.number_));
        if(degree_dif < 0) break;
        tmp = (1 << (degree_dif));
        q_ ^= tmp;
        r_ ^= multiply_(rhs.number_, tmp);
    }
    r = r_;
    q = q_;
}

uint8_t GaloisField::gen_poly() const {
    return primitive_polinomial_[size_ - 1];
}

uint8_t GaloisField::multiply_(uint8_t number, uint8_t other) {
    uint32_t tmp = 0;
    while(other) {
        if(other % 2) {
            tmp ^= number;
        }
        other >>= 1;
        bool flip = (number & (1 << (size_ - 1)));
        number <<= 1;
        if(flip) {
            number ^= primitive_polinomial_[size_ - 1];
        }
    }
    tmp &= (1 << static_cast<uint32_t>(size_)) - 1;
    number = tmp;
    return number;
}

void GaloisField::gen_log_tables_() {
    if(!tables_[size_ - 1].count) {
        uint8_t current_calc = 1;
        uint8_t step = 2;
        tables_[size_ - 1].log_table = new uint8_t[(1 << size_) - 1];
        tables_[size_ - 1].anti_log_table = new uint8_t[(1 << size_) - 1];
        for(uint32_t i = 0; i < (1 << size_) - 1; ++i) {
            tables_[size_ - 1].log_table[i] = current_calc;
            tables_[size_ - 1].anti_log_table[current_calc - 1] = i;
            current_calc = multiply_(current_calc, step);
        }
    }
    ++tables_[size_ - 1].count;
}

uint32_t GaloisField::minimal_polinomial(uint8_t polinomial_number) const {
    uint32_t linear_system[size_];
    for(uint8_t i = 0; i < size_; ++i) {
        linear_system[i] = 0;
    }
    for(uint8_t bit_num = 0; bit_num < 8; ++bit_num) {
        for(uint8_t i = 0; i <= size_; ++i) {
            if(tables_[size_ - 1].log_table[(i * polinomial_number) % ((1 << size_) - 1)] & (1 << bit_num)) {
                linear_system[bit_num] |= static_cast<uint32_t>(1 << ((i + 1) % (size_ + 1)));
            }
        }
    }
    //Apply row reduction algorithm to generate row echelon form
    uint32_t h = 0, k = 0;
    uint32_t base = 1 << size_;
    while(h < size_ && k < size_) {
        for(uint32_t i = h; i < size_; ++i) {
            if(linear_system[i] & (base >> k)) {
                swap_(linear_system[h], linear_system[i]);
                for(uint32_t j = h + 1; j < size_; ++j) {
                    if(linear_system[j] & (base >> k)) {
                        linear_system[j] ^= linear_system[h];
                    }
                }
                ++h;
                break;
            }
        }
        ++k;
    }
    uint32_t ret = 0;
    for(uint8_t i = 1; i < size_ + 1; ++i) {
        uint8_t index = size_ - i;
        uint8_t bit_calc = linear_system[index] & 1;
        for(uint8_t j = i - 1; j > 0; --j) {
            //If the bit is set, we get its value. We have to xor it to the result so we can compute the resulting bit value;
            if(linear_system[index] & (1 << j) && ret & (1 << j)) {
                bit_calc ^= 1;
            }
        }
        ret |= (bit_calc << i);
    }
    ret >>= 1;
    ret |= 1 << size_;
    return ret;
}
