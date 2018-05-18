#include "vectra.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <semaphore.h>
#include <errno.h>
#include <unistd.h>


std::vector<int> 
arr_read_can()
{
    std::vector<int> ret;
    pipe_arr_read(ret, CAN_MEM_ARR_NAME, CAN_MEM_ARR_SIZE, CAN_SEM_ARR_NAME);
    return ret;
}

std::vector<int> 
arr_read_gui()
{
    std::vector<int> ret;
    pipe_arr_read(ret, GUI_MEM_ARR_NAME, GUI_MEM_ARR_SIZE, GUI_SEM_ARR_NAME);
    return ret;
}

std::vector<int> 
arr_read_nrf()
{
    std::vector<int> ret;
    pipe_arr_read(ret, NRF_MEM_ARR_NAME, NRF_MEM_ARR_SIZE, NRF_SEM_ARR_NAME);
    return ret;
}

int arr_write_can(const std::vector<int>& array)
{
    return pipe_arr_write(CAN_MEM_ARR_NAME, array, CAN_MEM_ARR_SIZE, CAN_SEM_ARR_NAME);
}

int arr_write_gui(const std::vector<int>& array)
{
    return pipe_arr_write(GUI_MEM_ARR_NAME, array, GUI_MEM_ARR_SIZE, GUI_SEM_ARR_NAME);
}

int arr_write_nrf(const std::vector<int>& array)
{
    return pipe_arr_write(NRF_MEM_ARR_NAME, array, NRF_MEM_ARR_SIZE, NRF_SEM_ARR_NAME);
}


int 
pipe_str_write (const std::string& pipename
    , const std::string& data
    , int size
    , const char* semaphorename) 
{
    int shm, mode = 0;
    char *addr;
    int len = data.length();

    len = (len<=size)?len:size;
    mode = O_CREAT;

    sem_t *sem;

    if ( (sem = sem_open(semaphorename, O_CREAT, 0644, 1)) == SEM_FAILED ) {
        perror("sem_open");
        return 1;
    }
    sem_wait(sem);

    if ( (shm = shm_open(pipename.c_str(), mode|O_RDWR, 0777)) == -1 ) {
        perror("shm_open");
        return 1;
    }

    if ( ftruncate(shm, size+1) == -1 ) {
        perror("ftruncate");
        return 1;
    }

    addr = (char*)mmap(0, size+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (char*)-1 ) {
        perror("mmap");
        close(shm);

        return 1;
    }
    
    memcpy(addr, data.c_str(), len);
    addr[len] = '\0';
        
    munmap(addr, size);
    close(shm);

    if ( sem_post(sem) < 0 )
        perror("sem_post");

    if ( sem_close(sem) < 0 )
        perror("sem_close");
    
    return 0;
}


int 
pipe_arr_write (const std::string& pipename
    , const std::vector<int>& data
    , int size
    , const char* semaforename) 
{
    sem_t *sem;

    if ( (sem = sem_open(semaforename, O_CREAT, 0644, 1)) == SEM_FAILED ) {
        perror("sem_open");
        return 1;
    }
    sem_wait(sem);
    

    int shm, mode = 0;
    int *addr;
    int len = data.size();

    len = (len<=size)?len:size;
    mode = O_CREAT;

    if ( (shm = shm_open(pipename.c_str(), mode|O_RDWR, 0777)) == -1 ) {
        //perror("shm_open");
        sem_close(sem);
        return 1;
    }

    if ( ftruncate(shm, size+1) == -1 ) {
        perror("ftruncate");
        sem_post(sem);
        return 1;
    }

    addr = (int*)mmap(0, size+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (int*)-1 ) {
        perror("mmap");
        sem_post(sem);
        return 1;
    }
    
    memset (addr, -1, size);

    for(int i = 0; i < len; ++i)
        addr[i] = data[i];        
        
    munmap(addr, size);
    close(shm);
    
    if ( sem_post(sem) < 0 )
        perror("sem_post");

    if ( sem_close(sem) < 0 )
        perror("sem_close");
    
    return 0;
}

int
pipe_arr_read(std::vector<int>& ret
    , const std::string& pipename
    , int size
    , const char* semaphorename)
{
    int shm, len, mode = 0;
    int *addr;

    ret.reserve(size);
    ret.assign(size,-1);

    sem_t *sem;
    
    if ( (sem = sem_open(semaphorename, O_CREAT, 0644, 1)) == SEM_FAILED ) {
        perror("sem_open");
        return 1;
    }
    sem_wait(sem);

    if ( (shm = shm_open(pipename.c_str(), mode|O_RDWR, 0777)) == -1 ) {
        perror("shm_open");
        sem_post(sem);
        return -1;
    }

    addr = (int *)mmap(0, size+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (int*)-1 ) {
        perror("mmap");
        sem_post(sem);
        return -2;
    }

    for (int i = 0; i < size; ++i)
        ret[i] = addr[i];

    munmap(addr, size);
    close(shm);
    
    if ( sem_post(sem) < 0 )
        perror("sem_post");

    if ( sem_close(sem) < 0 )
        perror("sem_close");

    return 0;
}

int
pipe_str_read(std::string& ret
    , const std::string& pipename
    , int size
    , const char* semaphorename) 
{
    int shm, len, cmd, mode = 0;
    char *addr;

    sem_t *sem;
    
    if ( (sem = sem_open(semaphorename, O_CREAT, 0644, 1)) == SEM_FAILED ) {
        perror("sem_open");
        return 1;
    }
    sem_wait(sem);


    if ( (shm = shm_open(pipename.c_str(), mode|O_RDWR, 0777)) == -1 ) {
        perror("shm_open");
        return -1;
    }

    addr = (char *)mmap(0, size+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (char*)-1 ) {
        perror("mmap");
        return -1;
    }

    ret.assign(addr);
    munmap(addr, size);
    close(shm);


    if ( sem_post(sem) < 0 )
        perror("sem_post");

    if ( sem_close(sem) < 0 )
        perror("sem_close");
    
    return 0;
}