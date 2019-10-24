#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "ummap.h"
#include "cycle.h"
#include <list>
#include <cassert>

#define TRUE  1
#define FALSE 0
#define CHK(_hr) { int hr = _hr; if(hr != 0) return hr; }
#define CHKB(b)  { if(b) CHK(errno); }

#define REPEAT_MMAP 100000
#define REPEAT_ACCESS 1

typedef std::list<ticks> TickList;

void dumpTicks(const TickList & lst, const char * fname)
{
    printf("Dump %s\n", fname);
    FILE * fp = fopen(fname, "w+");
    assert(fp != NULL);
    for (auto it : lst)
        fprintf(fp, "%llu\n", it);
    fclose(fp);
}

int open_existing(const char * fname, int flags, mode_t mode, size_t size, bool existing = true)
{
    int fd = open(fname, flags, mode);
    if (existing) {
        ftruncate(fd, size);
        int8_t * baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd, 0);
        memset(baseptr, 1, size);
        syncfs(fd);
        munmap(baseptr, size);
    }
    return fd;
}

int main(int argc, char **argv)
{
    const int    flags    = (O_CREAT   | O_RDWR);
    const mode_t mode     = (S_IRUSR   | S_IWUSR);
    const int    prot     = (PROT_READ | PROT_WRITE);
    const size_t size     = 1073741824; // 1GB allocation
    const size_t segsize  = 4096; //16777216;   // 16MB segments
    const off_t  offset   = 0;          // File offset
    int    fd       = 0;          // File descriptor
    int8_t *baseptr = NULL;       // Base pointer
    char buffer[segsize];
    bool existing = true;
    bool existingMmap = false;

    // Open the file descriptor for the mapping
    fd = open("./example.data", flags, mode);
    CHKB(fd == -1);
    
    // Ensure that the file has space (optional)
    CHK(ftruncate(fd, size));

    CHK(ummap(size, segsize, prot, fd, offset, -1, TRUE, 0, (void **)&baseptr));

    // bench mmap
    {
        TickList mapTime;
        TickList umapTime;
        for (int i = 0 ; i < REPEAT_MMAP ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existingMmap);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            ticks start = getticks();
            baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
            ticks inter = getticks();
            munmap(baseptr, size);
            ticks stop = getticks();
            mapTime.push_back(inter - start);
            umapTime.push_back(stop - inter);

            CHK(close(fd));
        }
        dumpTicks(mapTime,"map-time-ref-anon.dat");
        dumpTicks(umapTime, "umap-time-ref-anon.dat");
    }

    // bench mmap
    {
        TickList mapTime;
        TickList umapTime;
        for (int i = 0 ; i < REPEAT_MMAP ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existingMmap);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            ticks start = getticks();
            baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd, 0);
            ticks inter = getticks();
            munmap(baseptr, size);
            ticks stop = getticks();
            mapTime.push_back(inter - start);
            umapTime.push_back(stop - inter);

            CHK(close(fd));
        }
        dumpTicks(mapTime,"map-time-ref.dat");
        dumpTicks(umapTime, "umap-time-ref.dat");
    }

    // bench mmap
    {
        TickList mapTime;
        TickList umapTime;
        for (int i = 0 ; i < REPEAT_MMAP ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existingMmap);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            ticks start = getticks();
            CHK(ummap(size, segsize, prot, fd, offset, -1, TRUE, 0, (void **)&baseptr));
            ticks inter = getticks();
            CHK(umunmap(baseptr, FALSE));
            ticks stop = getticks();
            mapTime.push_back(inter - start);
            umapTime.push_back(stop - inter);

            CHK(close(fd));
        }
        dumpTicks(mapTime,"map-time.dat");
        dumpTicks(umapTime, "umap-time.dat");
    }

    // bench mmap
    {
        TickList firstTouchRead;
        TickList firstTouchWrite;
        for (int i = 0 ; i < REPEAT_ACCESS ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existing);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
            for (size_t i = 0 ; i < size / 4096 ; i++) {
                ticks start = getticks();
                uint8_t v = baseptr[i * 4096];
                ticks inter = getticks();
                baseptr[i * 4096] = v+1;
                ticks stop = getticks();
                firstTouchRead.push_back(inter - start);
                firstTouchWrite.push_back(stop - inter);
            }
            munmap(baseptr, size);
            
            CHK(close(fd));
            unlink("./example2.data");
        }
        dumpTicks(firstTouchRead,"first-touch-read-time-ref-anon.dat");
        dumpTicks(firstTouchWrite, "first-touch-write-time-ref-anon.dat");
    }

    // bench mmap
    {
        TickList firstTouchRead;
        TickList firstTouchWrite;
        for (int i = 0 ; i < REPEAT_ACCESS ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existing);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd, 0);
            for (size_t i = 0 ; i < size / 4096 ; i++) {
                ticks start = getticks();
                uint8_t v = baseptr[i * 4096];
                ticks inter = getticks();
                baseptr[i * 4096] = v+1;
                ticks stop = getticks();
                firstTouchRead.push_back(inter - start);
                firstTouchWrite.push_back(stop - inter);
            }
            munmap(baseptr, size);
            
            CHK(close(fd));
            unlink("./example2.data");
        }
        dumpTicks(firstTouchRead,"first-touch-read-time-ref.dat");
        dumpTicks(firstTouchWrite, "first-touch-write-time-ref.dat");
    }

    // bench mmap
    {
        TickList firstTouchRead;
        TickList firstTouchWrite;
        for (int i = 0 ; i < REPEAT_ACCESS ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existing);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            CHK(ummap(size, segsize, prot, fd, offset, -1, TRUE, 0, (void **)&baseptr));
            for (size_t i = 0 ; i < size / 4096 ; i++) {
                ticks start = getticks();
                uint8_t v = baseptr[i * 4096];
                ticks inter = getticks();
                baseptr[i * 4096] = v+1;
                ticks stop = getticks();
                firstTouchRead.push_back(inter - start);
                firstTouchWrite.push_back(stop - inter);
            }
            CHK(umunmap(baseptr, FALSE));
            
            CHK(close(fd));
            unlink("./example2.data");
        }
        dumpTicks(firstTouchRead,"first-touch-read-time.dat");
        dumpTicks(firstTouchWrite, "first-touch-write-time.dat");
    }

    #pragma omp parallel
    printf("Spawn threads\n");

    // bench mmap
    {
        TickList firstTouchRead;
        TickList firstTouchWrite;
        for (int i = 0 ; i < REPEAT_ACCESS ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existing);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            CHK(ummap(size, segsize, prot, fd, offset, -1, TRUE, 0, (void **)&baseptr));
            #pragma omp parallel
            {
                TickList localFirstTouchRead;
                TickList localFirstTouchWrite;
                #pragma omp for
                for (size_t i = 0 ; i < size / 4096 ; i++) {
                    ticks start = getticks();
                    uint8_t v = baseptr[i * 4096];
                    ticks inter = getticks();
                    baseptr[i * 4096] = v+1;
                    ticks stop = getticks();
                    localFirstTouchRead.push_back(inter - start);
                    localFirstTouchWrite.push_back(stop - inter);
                }

                #pragma omp critical
                {
                    while (localFirstTouchRead.size() > 0) {
                        firstTouchRead.push_back(localFirstTouchRead.front());
                        localFirstTouchRead.pop_front();
                    }
                    while (localFirstTouchWrite.size() > 0) {
                        firstTouchWrite.push_back(localFirstTouchWrite.front());
                        localFirstTouchWrite.pop_front();
                    }
                }
            }
            CHK(umunmap(baseptr, FALSE));
            
            CHK(close(fd));
            unlink("./example2.data");
        }
        dumpTicks(firstTouchRead,"threaded-first-touch-read-time.dat");
        dumpTicks(firstTouchWrite, "threaded-first-touch-write-time.dat");
    }

    // bench mmap
    {
        TickList firstTouchRead;
        TickList firstTouchWrite;
        for (int i = 0 ; i < REPEAT_ACCESS ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existing);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd, 0);
            #pragma omp parallel
            {
                TickList localFirstTouchRead;
                TickList localFirstTouchWrite;
                #pragma omp for
                for (size_t i = 0 ; i < size / 4096 ; i++) {
                    ticks start = getticks();
                    uint8_t v = baseptr[i * 4096];
                    ticks inter = getticks();
                    baseptr[i * 4096] = v+1;
                    ticks stop = getticks();
                    localFirstTouchRead.push_back(inter - start);
                    localFirstTouchWrite.push_back(stop - inter);
                }

                #pragma omp critical
                {
                    while (localFirstTouchRead.size() > 0) {
                        firstTouchRead.push_back(localFirstTouchRead.front());
                        localFirstTouchRead.pop_front();
                    }
                    while (localFirstTouchWrite.size() > 0) {
                        firstTouchWrite.push_back(localFirstTouchWrite.front());
                        localFirstTouchWrite.pop_front();
                    }
                }
            }
            munmap(baseptr, size);
            
            CHK(close(fd));
            unlink("./example2.data");
        }
        dumpTicks(firstTouchRead,"threaded-first-touch-read-time-ref-anon.dat");
        dumpTicks(firstTouchWrite, "threaded-first-touch-write-time-ref-anon.dat");
    }

    // bench mmap
    {
        TickList firstTouchRead;
        TickList firstTouchWrite;
        for (int i = 0 ; i < REPEAT_ACCESS ; i++) {
            fd = open_existing("./example2.data", flags, mode, size, existing);
            CHKB(fd == -1);
            CHK(ftruncate(fd, size));
        
            baseptr = (int8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd, 0);
            #pragma omp parallel
            {
                TickList localFirstTouchRead;
                TickList localFirstTouchWrite;
                #pragma omp for
                for (size_t i = 0 ; i < size / 4096 ; i++) {
                    ticks start = getticks();
                    uint8_t v = baseptr[i * 4096];
                    ticks inter = getticks();
                    baseptr[i * 4096] = v+1;
                    ticks stop = getticks();
                    localFirstTouchRead.push_back(inter - start);
                    localFirstTouchWrite.push_back(stop - inter);
                }

                #pragma omp critical
                {
                    while (localFirstTouchRead.size() > 0) {
                        firstTouchRead.push_back(localFirstTouchRead.front());
                        localFirstTouchRead.pop_front();
                    }
                    while (localFirstTouchWrite.size() > 0) {
                        firstTouchWrite.push_back(localFirstTouchWrite.front());
                        localFirstTouchWrite.pop_front();
                    }
                }
            }
            munmap(baseptr, size);
            
            CHK(close(fd));
            unlink("./example2.data");
        }
        dumpTicks(firstTouchRead,"threaded-first-touch-read-time-ref.dat");
        dumpTicks(firstTouchWrite, "threaded-first-touch-write-time-ref.dat");
    }

    // It is now safe to close the file descriptor
    CHK(close(fd));

    // Set some random value on the allocation
    for (off_t i = 0; i < (off_t)size; i++)
    {
        baseptr[i] = 21;
    }

    // Alternative: Use traditional mem. functions
    memset(baseptr, 21, size);

    // Synchronize with storage to ensure that the latest changes are flushed
    CHK(umsync(baseptr, FALSE));

    // Finally, release the mapping if not needed (note that the storage
    // synchronization is not needed because of the previous statement)
    CHK(umunmap(baseptr, FALSE));
    
    return 0;
}

