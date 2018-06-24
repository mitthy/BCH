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

#ifndef BCH_H
#define BCH_H

#include "galoisfield.h"
#include "bitvector.h"

/**
 * @todo write docs
 */
class BCH {
public:
    BCH(const GaloisField& gf, uint32_t err_correctors);
    BCH(const BCH&) = default;
    ~BCH() = default;
    BCH& operator=(const BCH&) = default;
    BitVector encode(const BitVector& message) const;
    BitVector decode(const BitVector& message) const;
    void set_num_errors(uint32_t number);
    uint32_t generator_order() const;
private:
    BitVector generator_polynomial_;
    GaloisField gf_;
    uint32_t t_;
    void do_set_num_errors_();
};

#endif // BCH_H
