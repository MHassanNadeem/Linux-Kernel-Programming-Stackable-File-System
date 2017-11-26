#include "wrapfs.h"

void xcfs_encrypt(char* mem, size_t size){
    size_t i;
    
    for(i=0; i<size; i++){
        mem[i]++;
    }
}

void xcfs_decrypt(char* mem, size_t size){
    size_t i;
    
    for(i=0; i<size; i++){
        mem[i]--;
    }
}