#include "galoisfield.h"
#include "bitvector.h"
#include "bch.h"
#include <iostream>
#include <algorithm>
#include <bitset>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <cmath>
#include <random>

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

static void help(std::ostream& os) {
    os << "Secure sketch generator for SRAM PUF." << std::endl;
    os << "--input_file || -if" << std::endl;
    os << "  [mandatory] provide file name to create the secure sketch." << std::endl;
    os << "--number_errors || -t" << std::endl;
    os << "  [optional] the number of bits that can differ between inputs." << std::endl;
    os << "    -if not supplied this is equal to 10% of the size of the file." << std::endl;
    os << "--number_secure_sketch || -s" << std::endl;
    os << "  [optional] the number of secure sketches to generate." << std::endl;
    os << "    -if not supplied this is equal to 1." << std::endl;
    os << "--output_file || -of" << std::endl;
    os << "  [mandatory] prefix of the name of the files to save the secure sketches." << std::endl;
}

void write(const std::string& file_prefix, uint32_t cur, const std::vector<char>& out) {
    std::ofstream file(file_prefix + std::to_string(cur));
    for(char c : out) {
        file << c;
    }
}

std::vector<char> random_byte_array(uint32_t bytes, uint8_t bit_offset) {
    std::vector<char> ret;
    std::random_device engine;
    uint32_t x = 0;
    for(uint32_t i = 0; i < bytes; ++i) {
        if(i % 4) {
            x = engine();
        }
        ret.push_back(x & (1 << ((sizeof(char) * 8))) - 1);
        x >>= (sizeof(char) * 8);
    }
    if(bit_offset) {
        x = engine();
        ret.push_back(x & ((1 << (bit_offset)) - 1));
    }
    return ret;
}

int main(int argc, char **argv) {
    if(argc == 1) {
        std::cerr << "Expected at least one argument." << std::endl;
        help(std::cerr);
        return EXIT_FAILURE;
    }
    std::string file_name;
    std::string output_file_name;
    uint32_t number_secure_sketch = 0;
    uint32_t number_errors = 0;
    for(int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if(arg == "--input_file" || arg == "-if") {
            if((++i) == argc) {
                std::cerr << "Missing argument after " << arg << std::endl;
                return EXIT_FAILURE;
            }
             if(!file_name.empty()) {
                std::cerr << "Can't input from more than 1 file." << std::endl;
                return EXIT_FAILURE;
            }
            file_name = argv[i];
        }
        else if(arg == "--number_errors" || arg == "-t") {
            if((++i) == argc) {
                std::cerr << "Missing argument after " << arg << std::endl;
                return EXIT_FAILURE;
            }
            if(number_errors) {
                std::cerr << "Maximum number of errors is already set." << std::endl;
                return EXIT_FAILURE;
            }
            char* next;
            long read = 0;
            read = std::strtol(argv[i], &next, 0);
            if(*next) {
                std::cerr << "Invalid parsing. " << argv[i] << " is not a number." << std::endl;
                return EXIT_FAILURE;
            }
            if(read < 1) {
                std::cerr << read << " is not a valid number." << std::endl;
                return EXIT_FAILURE;
            }
            number_errors = static_cast<uint32_t>(read);
        }
        else if(arg == "--number_secure_sketch" || arg == "-s") {
            if((++i) == argc) {
                std::cerr << "Missing argument after " << arg << std::endl;
                return EXIT_FAILURE;
            }
            if(number_secure_sketch) {
                std::cerr << "Number of secure sketches is already set." << std::endl;
                return EXIT_FAILURE;
            }
            char* next;
            long read = 0;
            read = std::strtol(argv[i], &next, 0);
            if(*next) {
                std::cerr << "Invalid parsing. " << argv[i] << " is not a number." << std::endl;
                return EXIT_FAILURE;
            }
            if(read < 1) {
                std::cerr << read << " is not a valid number." << std::endl;
                return EXIT_FAILURE;
            }
            number_secure_sketch = static_cast<uint32_t>(read);
        }
        else if(arg == "--output_file" || arg == "-of") {
            if((++i) == argc) {
                std::cerr << "Missing argument after " << arg << std::endl;
                return EXIT_FAILURE;
            }
            if(!output_file_name.empty()) {
                std::cerr << "Can't output to more than 1 prefix." << std::endl;
                return EXIT_FAILURE;
            }
            output_file_name = argv[i];
        }
        else if(arg == "--help" || arg == "-h") {
            help(std::cout);
            return EXIT_SUCCESS;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            help(std::cerr);
            return EXIT_FAILURE;
        }
    }
    if(!number_secure_sketch) {
        number_secure_sketch = 1;
    }
    if(file_name.empty()) {
        std::cerr << "Missing input file." << std::endl;
        return EXIT_FAILURE;
    }
    if(output_file_name.empty()) {
        std::cerr << "Missing output file prefix." << std::endl;
        return EXIT_FAILURE;
    }
    std::ifstream input_file(file_name);
    if(!input_file) {
        std::cerr << "Couldn't open file " << file_name << std::endl;
        return EXIT_FAILURE;
    }
    std::vector<char> buffer((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    std::size_t buffer_size = buffer.size() * 8;
    if(!number_errors) {
        //Assume number of errors as 10% the size of the file in bits.
        number_errors = buffer_size / 10;
    }
    uint8_t gf_order = std::ceil(std::log2(buffer_size));
    if(gf_order > 8) {
        std::cerr << "Galois field of order greater than 8 are not yet permited." << std::endl;
        return EXIT_FAILURE;
    }
    GaloisField field(gf_order);
    BCH encoder(field, number_errors);
    uint32_t enc_bits = encoder.generator_order();
    uint32_t total_bits = 1 << gf_order;
    uint32_t dif = total_bits - enc_bits;
    uint32_t num_bytes = dif / 8 + 1;
    uint8_t bit_offset = dif % 8;
    BitVector input_data_bit_vector(buffer.begin(), buffer.end());
    for(uint32_t i = 0; i < number_secure_sketch; ++i) {
        std::vector<char> random_data = random_byte_array(num_bytes, bit_offset);
        BitVector data_vector(random_data.begin(), random_data.end());
        BitVector encoded = encoder.encode(data_vector);
        encoded ^= input_data_bit_vector;
        std::vector<char> output_array(encoded.begin(), encoded.end());
        write(output_file_name, i, output_array);
    }
    return EXIT_SUCCESS;
}
