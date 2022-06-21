#ifndef BLOCK_HPP
#define BLOCK_HPP
#include "Config.hpp"
// a block class
class Block{
    public:
        TYPE_INDEX index;
        TYPE_INDEX data_size;
        Block(TYPE_INDEX index, TYPE_INDEX block_size);
        Block(TYPE_INDEX block_size);
        ~Block();
        
        char* decrypt();
        char* encrypt();
        // interactive operations
        char* get_data();
        unsigned char* get_iv();
        void set_data(char* data);
        void set_iv(unsigned char* iv);

    private:
        char *data;
        unsigned char *iv;
};

#endif
