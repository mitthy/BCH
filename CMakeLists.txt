cmake_minimum_required(VERSION 2.6)
project(numbertheory)

add_executable(numbertheory bitvector.h bitvector.cpp main.cpp galoisfield.cpp galoisfield.h bch.h bch.cpp)

install(TARGETS numbertheory RUNTIME DESTINATION bin)
