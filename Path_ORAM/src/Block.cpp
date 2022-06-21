#include "Block.hpp"
#include "Config.hpp"

#include <tomcrypt.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>
// constructor
Block::Block(TYPE_INDEX index, TYPE_INDEX data_size){
	// I love you
    this->index = index;
    this->data_size = data_size;
    this->data = new char[data_size];
    // initialize the data with some data some characters
    for(TYPE_INDEX i = 0; i < data_size; i++){
        data[i] = 'a' + i % 26;
    }
	// write this->index to the data with memcpy
	memcpy(data, &index, sizeof(TYPE_INDEX));
    this->iv = new unsigned char[IV_SIZE];

    // initialize the aes_desc 
    if (-1 == register_cipher(&aes_desc)) {
		std::cout << "[Block] Register cipher err" << std::endl;
		exit(-1);
	}
}
Block::Block(TYPE_INDEX block_size){
	this->index = 0;
	this->data_size = block_size;
	this->data = new char[data_size];
	// initialize the data with some data some characters
	this->iv = new unsigned char[IV_SIZE];

	// initialize the aes_desc 
	if (-1 == register_cipher(&aes_desc)) {
		std::cout << "[Block] Register cipher err" << std::endl;
		exit(-1);
	}
}
// destructor
Block::~Block(){
    delete[] data;
    delete[] iv;
}
// realize get_data()
char* Block::get_data(){
    return data;
}
// realize get_iv()
unsigned char* Block::get_iv(){
	return iv;
}
// realize set_data()
void Block::set_data(char* data){
	memcpy(this->data, data, data_size);
}
// realize set_iv()
void Block::set_iv(unsigned char* iv){
	memcpy(this->iv, iv, IV_SIZE);
}
char *Block::encrypt()
{
	/* Generate random IV */
	for (int i = 0; i < IV_SIZE; i += sizeof(int)) {
		int random_int = rand();
		memcpy(iv + i, &random_int, sizeof(int));
	}
	if (IV_SIZE / sizeof(int) != 0) {
		int random_int = rand();
		memcpy(iv + IV_SIZE - sizeof(int), &random_int, sizeof(int));
	}

	int err = 0;
	symmetric_CTR ctr;
	char cipher_data[this->data_size];
    
	/* encrypt */
	if ((err =  ctr_start(find_cipher("aes"), iv, ENCRYPT_KEY, sizeof(ENCRYPT_KEY), 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK) 
    {
        std::cout << "[Block] ctr_start err: " << error_to_string(err) << std::endl;
		exit(-1);
	}
	if ((err = ctr_encrypt((unsigned char *)data, (unsigned char *)cipher_data, this->data_size, &ctr)) != CRYPT_OK) {
        std::cout << "[Block] ctr_encrypt err: " << error_to_string(err) << std::endl;
        exit(-1);
	}
	if ((err = ctr_done(&ctr)) != CRYPT_OK) {
        std::cout << "[Block] ctr_done err: " << error_to_string(err) << std::endl;
		exit(-1);
	}

	memcpy(data, cipher_data, this->data_size);
    return data;
}

char *Block::decrypt()
{
	int err;
	symmetric_CTR ctr;
	char decipher_data[this->data_size];

	if ((err =  ctr_start(find_cipher("aes"), iv, ENCRYPT_KEY, sizeof(ENCRYPT_KEY), 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK) {
		std::cout << "[Block] ctr_start err: " << error_to_string(err) << std::endl;
		exit(-1);
	}
	if ((err = ctr_decrypt((unsigned char *)data, (unsigned char *)decipher_data, this->data_size, &ctr)) != CRYPT_OK) {
        std::cout << "[Block] ctr_encrypt err: " << error_to_string(err) << std::endl;
		exit(-1);
	}
	if ((err = ctr_done(&ctr)) != CRYPT_OK) {
        std::cout << "[Block] ctr_done err: " << error_to_string(err) << std::endl;
		exit(-1);
	}

	memcpy(data, decipher_data, this->data_size);
    return data;
}
