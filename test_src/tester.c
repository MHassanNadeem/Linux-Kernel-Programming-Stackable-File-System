#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* mmap */
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TMP_FILE_NAME "tester_tmp_test_file"
#define TMP_FILE_SIZE 4000 // don't change for now

// Debugging Macros
#define DBG(var, type)          printf(#var" = %"#type"\n", var)
#define DBGM(var, type, desc)   printf(desc" = %"#type"\n", var)
#define PRINT(msg, ...)         printf(msg, ##__VA_ARGS__)
#define ERROR(msg, ...)         printf("ERROR: "msg, ##__VA_ARGS__)

unsigned char *getRandomBytes(unsigned char *buffer, size_t size){
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

int test_mmapRead(char *mount_filename){
    remove( mount_filename );
    
    FILE *mount_fptr;
    int fd;
    struct stat sbuf;
    unsigned char data[TMP_FILE_SIZE];
    
    getRandomBytes(data, TMP_FILE_SIZE);
    
    /* Write data into a file in mounted dir */
    mount_fptr = fopen(mount_filename, "wb");
    if(mount_fptr == NULL){
        ERROR("Could not open file %s\n", mount_filename);
        return -1;
    }
    fwrite(data,sizeof(data),1,mount_fptr);
    fclose(mount_fptr);
    
    /* MMAP Read the file */
    if ((fd = open(mount_filename, O_RDONLY)) == -1) {
        ERROR("open\n");
        return -1;
    }
    
    if (stat(mount_filename, &sbuf) == -1) {
        ERROR("stat\n");
        return -1;
    }
    
    unsigned char* mmappedData = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    if(mmappedData == MAP_FAILED){
        ERROR("mmap\n");
        return -1;
    }
    
    /* Test if read correctly */
    for(size_t i=0; i<sbuf.st_size; i++){
        if(data[i] != mmappedData[i]){
            DBG(i, lu);
            DBG(data[i], u);
            DBG(mmappedData[i], u);
            ERROR("MMAP data not equal\n");
            return -1;
        }
    }
    
    close(fd);
    return 0;
}

int test_mmapWrite(char *mount_filename){
    remove( mount_filename );
    
    FILE *mount_fptr;
    int fd;
    unsigned char data[TMP_FILE_SIZE], data_read[TMP_FILE_SIZE];;
    
    getRandomBytes(data, TMP_FILE_SIZE);
    
    /* MMAP Write the file */
    if( (fd = open(mount_filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600)) == -1 ){
        ERROR("open\n");
        return -1;
    }
    
    if(write(fd, data, sizeof(data)) == -1){
        close(fd);
        ERROR("write\n");
        return -1;
    }
    
    /* Read data without mmap */
    mount_fptr = fopen(mount_filename, "rb");
    if(mount_fptr == NULL){
        ERROR("Could not open file %s\n", mount_filename);
        return -1;
    }
    fread(data_read,sizeof(data_read),1,mount_fptr);
    fclose(mount_fptr);
    
    /* Test if read correctly */
    for(size_t i=0; i<TMP_FILE_SIZE; i++){
        if(data[i] != data_read[i]){
            DBG(i, lu);
            DBG(data[i], u);
            DBG(data_read[i], u);
            ERROR("MMAP data not equal\n");
            return -1;
        }
    }
    
    close(fd);
    return 0;
}

int main(int argc, char **argv){
    setbuf(stdout, NULL); // Disable stdout buffering
    
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
        ERROR("Buffered IO Failed!!!\n");
    }else{
        printf("Bufferd IO test passed\n");
    }
    
    /* ========== MMAP READ TEST ============= */
    /* test memman read on lower filesystem just to check the testcase */
    if( test_mmapRead(src_filename) ){
        ERROR("OOPs this should not be happening\n");
        ERROR("bug in test_mmapRead()\n");
    }else{
        printf("MMAP read sanity test passed\n");
    }
    
    if(test_mmapRead(mount_filename)){
        ERROR("MMAP read failed!!!\n");
    }else{
        printf("MMAP read passed\n");
    }
    
    /* ========== MMAP Write TEST ============= */
    if( test_mmapWrite(src_filename) ){
        ERROR("OOPs this should not be happening\n");
        ERROR("bug in test_mmapWrite()\n");
    }else{
        printf("MMAP write sanity test passed\n");
    }
    
    if(test_mmapWrite(mount_filename)){
        ERROR("MMAP read failed!!!\n");
    }else{
        printf("MMAP read passed\n");
    }
    
    printf("=====================================\n");
    return 0;
}