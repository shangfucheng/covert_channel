#include <unistd.h>
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

// [Bonus] TODO: define your own buffer size
#define BUFF_SIZE (1<<24)
#define L2_SIZE 262144
#define L2_SET_SIZE 32
#define CACHE_LINE 64
#define L2_SET_INDEX_MASK 0x0ffc0
#define CACHE_LINE_BITS 6

static uint32_t eviction_set[8] = {2,6,10,14,18,22,26,30};

static uint32_t get_set_index(ADDR_PTR* addr){
//	return (((uintptr_t)addr & L2_SET_INDEX_MASK) >> CACHE_LINE_BITS);
//	printf("ind: %d\n", ((uint64_t)addr >> 6) & ((1 << 10) - 1));
	return ((uint64_t)addr >> 6) & ((1 << 10) - 1);
}


// occupy the entire L2 cache as a indicator for receiver to know that sender is running
static void covert_channel(uint64_t* buf){
//	for(int i = 0; i < 256; i++){ eviction_set[i] = i;}
	uint64_t* tmp = buf;
	int attempt = 0;
	while(attempt++ < 20){
		buf = tmp;
		for(; buf < tmp+L2_SIZE; buf+=8){
			*((char*)buf) = 1;
		}	
	}
}

// fill an entire L2 cache set to indicate bit 1
static void fill_one_L2_cache_set(ADDR_PTR* addr, uint32_t set){
	int attempt = 0;
	// find the address that will match to the set we are looking for
	while(get_set_index(addr) != set) {addr += 1;}
//	addr += L2_SET_SIZE * set;
	ADDR_PTR* tmp = addr;
//	printf("set addr %p\n", addr);
	// load the entire set
	for(; addr < tmp+L2_SET_SIZE; addr+=8){
		*((char*)addr) = 1;	// load the cache block
	}
}

// send signal to receiver to prepare read data,
// by occupy the entire set 0 in L2 cache
static inline void send_init_signal(ADDR_PTR* addr){
	while(get_set_index(addr) != 0) {addr += 8;}
	ADDR_PTR* tmp = addr;
	for(; addr < tmp+L2_SET_SIZE; addr+=8){
    	*((char*)addr) = 1;  // load the cache block
   	}	
}

static inline void send_data(uint8_t num, ADDR_PTR* buf){
	int attempt = 0;
//	send_init_signal(buf);
	while(attempt++ < 1000000){
		for(int i = 0; i < 8; i++){
			// if bit=1, occupy the set
			fill_one_L2_cache_set(buf, eviction_set[i]);
		}
	}
}

void test_l2_sets(ADDR_PTR* buf){
	while(get_set_index(buf) != 3) {buf += 1;}
//	printf("%p\n", buf);
	for(int i = 0; i < 10; i++){
		void* tmp = buf;
		for(int i = 0; i < 4; i++){
			*((uint64_t*)tmp) += 1;
			tmp += 8;
		}
	}
}

int main(int argc, char **argv)
{
    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
    void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    
    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)buf) = 1; // dummy write to trigger page allocation

	printf("%p\n", buf);
    // [Bonus] TODO:
    // Put your covert channel setup code here
	covert_channel(buf);

    printf("Please type a message.\n");
//	while(1) {test_l2_sets(buf);}
	uint8_t start = 1;
    bool sending = true;
    while (sending) {
        char text_buf[128];
        fgets(text_buf, sizeof(text_buf), stdin);

        // [Bonus] TODO:
        // Put your covert channel code here
		uint8_t num = atoi(text_buf);
		for(int i = 0; i < 9; i++) {send_data(start, buf);}
		for (int i = 0; i < 8; i++){
			if(num & (1<<i)) {
				send_data(num, buf);
			}else usleep(1000000);
		}
		printf("sent\n");	
    }

    printf("Sender finished.\n");
    return 0;
}
