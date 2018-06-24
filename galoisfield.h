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

#ifndef GALOISFIELD_H
#define GALOISFIELD_H

#include <cstdint>

/**
 * @todo write docs
 */
class GaloisField {
public:
    explicit GaloisField(uint8_t size);
    explicit GaloisField(uint8_t size, uint8_t number);
    GaloisField(const GaloisField&);
    ~GaloisField();
    GaloisField& operator=(uint8_t number);
    GaloisField& operator=(const GaloisField&);
    GaloisField& operator+=(const GaloisField& rhs) {
        if(rhs.size_ == size_) {
            number_ ^= rhs.number_;
        }
        return *this;
    }
    GaloisField& operator-=(const GaloisField& rhs) {
        (*this) += rhs;
        return *this;
    }
    GaloisField& operator*=(const GaloisField&);
    GaloisField& operator/=(const GaloisField&);
    GaloisField& operator%=(const GaloisField&);
    GaloisField operator+(const GaloisField& rhs) const {
        GaloisField tmp(*this);
        tmp += rhs;
        return tmp;
    }
    GaloisField operator-(const GaloisField& rhs) const {
        GaloisField tmp(*this);
        tmp -= rhs;
        return tmp;
    }
    GaloisField operator*(const GaloisField& rhs) const {
        GaloisField tmp(*this);
        tmp *= rhs;
        return tmp;
    }
    GaloisField operator/(const GaloisField& rhs) const {
        GaloisField tmp(*this);
        tmp /= rhs;
        return tmp;
    }
    GaloisField operator%(const GaloisField& rhs) const {
        GaloisField tmp(*this);
        tmp %= rhs;
        return tmp;
    }
    
    uint8_t size() const {
        return size_;
    }
    
    uint8_t gen_poly() const;
    
    explicit operator uint8_t() const {
        return number_;
    }
    
    void swap(GaloisField& other);
    
    uint32_t minimal_polinomial(uint8_t polinomial_number) const;
    
private:
    struct count_log_anti_log_tables {
        uint32_t count = 0;
        uint8_t* log_table = nullptr;
        uint8_t* anti_log_table = nullptr;
    };
    void long_division_(const GaloisField&, uint8_t&, uint8_t&);
    uint8_t multiply_(uint8_t number, uint8_t other);
    void gen_log_tables_();
    uint8_t size_;
    uint8_t number_;
    static uint8_t primitive_polinomial_[8];
    static count_log_anti_log_tables tables_[8];
};

#endif // GALOISFIELD_H
