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

#include "bch.h"

BCH::BCH(const GaloisField& gf, uint32_t err_correctors): gf_(gf), t_(err_correctors) {
    do_set_num_errors_();
}

BitVector BCH::encode(const BitVector& message) const {
    BitVector tmp(message);
    tmp <<= generator_polynomial_.msb(nullptr);
    BitVector remainder = long_division(tmp, generator_polynomial_).r;
    remainder ^= tmp;
    return remainder;
}

BitVector BCH::decode(const BitVector& message) const {
    //TODO
    return BitVector();
}

void BCH::set_num_errors(uint32_t number) {
    t_ = number;
    do_set_num_errors_();
}

void BCH::do_set_num_errors_() {
    generator_polynomial_ = 1;
    BitVector polynomial;
    for(uint32_t i = 0; i < t_; ++i) {
        polynomial = gf_.minimal_polinomial(1 + (i * 2));
        multiply(generator_polynomial_, polynomial);
    }
}

uint32_t BCH::generator_order() const {
    return generator_polynomial_.msb();
}
