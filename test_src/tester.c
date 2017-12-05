#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TMP_FILE_NAME "tester_tmp_test_file"
#define TMP_FILE_SIZE 4000

// Debugging Macros
#define DBG(var, type)          printf(#var" = %"#type"\n", var)
#define DBGM(var, type, desc)   printf(desc" = %"#type"\n", var)
#define PRINT(msg, ...)         printf(msg, ##__VA_ARGS__)
#define ERROR(msg, ...)         printf("ERROR: "msg, ##__VA_ARGS__)

char *getRandomBytes(char *buffer, size_t size){
  for (size_t i = 0; i < size; i++){
    buffer[i] = rand();
  }

  return buffer;
}

int test_bufferedRW(char *src_filename, char *mount_filename){
    FILE *src_fptr, *mount_fptr;
    unsigned char data[TMP_FILE_SIZE], data_enc[TMP_FILE_SIZE], data_read[TMP_FILE_SIZE];
    
    getRandomBytes(data, TMP_FILE_SIZE);
    
    /* Write data into a file in mounted dir */
    mount_fptr = fopen(mount_filename, "wb");
    if(mount_fptr == NULL){
        ERROR("Could not open file %s\n", mount_filename);
        return -1;
    }
    fwrite(data,sizeof(data),1,mount_fptr);
    fclose(mount_fptr);
    
    /* Read data from the file in original dir */
    src_fptr = fopen(src_filename, "rb");
    if(src_fptr == NULL){
        ERROR("Could not open file %s\n", src_filename);
        return -1;
    }
    fread(data_enc,sizeof(data_enc),1,src_fptr);
    fclose(src_fptr);
    
    /* Read data from the file in mounted dir */
    mount_fptr = fopen(mount_filename, "rb");
    if(mount_fptr == NULL){
        ERROR("Could not open file %s\n", mount_filename);
        return -1;
    }
    fread(data_read,sizeof(data_read),1,mount_fptr);
    fclose(mount_fptr);
    
    /* test */
    for(size_t i = 0; i < TMP_FILE_SIZE; i++){
        if(data[i] != data_read[i]){
            DBG(i, lu);
            DBG(data[i], u);
            DBG(data_read[i], u);
            ERROR("Data written is not equal to data read\n");
            return -1;
        }
        
        if(data[i] != (unsigned char)(data_enc[i]-1)){
            DBG(i, lu);
            DBG(data[i], u);
            DBG(data_enc[i], u);
            ERROR("Data not being encrypted\n");
            return -1;
        }
    }
    
    return 0;
}

int main(int argc, char **argv){
    printf("=====================================\n");
    
    if(argc < 3){
        printf("ERROR: Insufficient arguments\n");
        printf("USAGE: ./tester src_dir mounted_dir\n");
        return -1;
    }
    
    char src_filename[200];
    char mount_filename[200];
    
    sprintf(src_filename, "%s/%s", argv[1], TMP_FILE_NAME);
    sprintf(mount_filename, "%s/%s", argv[2], TMP_FILE_NAME);
    
    DBG(src_filename, s);
    DBG(mount_filename, s);
    
    srand((unsigned int) time (NULL)); // seed the rand function
    
    if(test_bufferedRW(src_filename, mount_filename)){
        ERROR("Buffered IO Failed!!!");
    }else{
        printf("Bufferd IO test passed\n");
    }
    
    
    printf("=====================================\n");
    return 0;
}