#include <unistd.h>
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#define BUFF_SIZE (1<<24)
#define L2_HIT_THRESHOLD 31
#define L2_SIZE 262144
#define CACHE_LINE 64
#define L2_SET_INDEX_MASK 0x0ffc0
#define CACHE_LINE_BITS 6
#define L2_SET_SIZE 32
#define WAIT_TIME 100000
static uint32_t eviction_set[256];

static inline uint32_t get_set_index(ADDR_PTR* addr){
    //return (((uintptr_t)addr & L2_SET_INDEX_MASK) >> CACHE_LINE_BITS);
	return ((uint64_t)addr >> 6) & ((1 << 10) - 1);
}
// load buf to fill up eviction sets
static void setup_covert_channel(ADDR_PTR* buf){
	ADDR_PTR* tmp = buf;
    int attempt = 0;
    while(attempt++ < 20){
        buf = tmp;
		for(int i = 0; i < 256; i++){
			while(get_set_index(buf) != eviction_set[i]) buf += 1;
			ADDR_PTR* end = buf+L2_SET_SIZE;
	        for(; buf < end; buf+=1){
    	        *((char*)buf) = 1;
        	}
		}
    }
}

// as sender start running , all cache in L2 should miss
// return 1 if send started, else 0
static uint8_t check_sender_status(ADDR_PTR* buf){
	ADDR_PTR* tmp = buf;
	printf("buf %p\n", buf);
	for(; buf < tmp+L2_SIZE; buf+=CACHE_LINE){
		// return false if hit in L1 and L2
		if( measure_one_block_access_time(buf) < L2_HIT_THRESHOLD){
			printf("hit\n");
            return 0;
        }
	}
	printf("miss\n");
	return 1;
}

static uint8_t check_send_data_signal(ADDR_PTR* buf){
	while(get_set_index(buf) != 0) buf += CACHE_LINE;
	ADDR_PTR* tmp = buf;
	// check if set 0 is hit
	for(; buf < tmp + L2_SET_SIZE; buf+=CACHE_LINE){
		// hit
		if( measure_one_block_access_time(buf) < L2_HIT_THRESHOLD) return 0;
	}
	// when all miss in set 0, sender signaled data transmission
	return 1;
}

static void decode_covert_channel(ADDR_PTR* buf, uint32_t* bits){
	for(int i = 0; i < 256; i++){
		while(get_set_index(buf) != i) buf += 1;
		// miss entire set, 4 way in L2
		uint8_t misses = 0;
		ADDR_PTR* set = buf;
//		set += L2_SET_SIZE * eviction_set[i];
		for (int j = 0; j < 4; j++){
			if( measure_one_block_access_time(set) > L2_HIT_THRESHOLD){
				misses += 1;
				set += 8;
			}
		}	
		if (misses >= 3) bits[eviction_set[i]] += 1;
	}
	
//	return recv_num;	
}

uint8_t test_l2_sets(ADDR_PTR* buf){
	uint8_t recv_num = 0;
    while(get_set_index(buf) != 3) {buf += 1;}
//	printf("%p\n", buf);
    for(int i = 0; i < 4; i++){
		CYCLES cy = measure_one_block_access_time(buf);
        if( cy > L2_HIT_THRESHOLD){
	  		recv_num |= (1<<i);
	   	}
		buf += 8;
    }
	return recv_num;
}
static uint8_t construct_num(uint32_t* bits){
	uint8_t num = 0; 
	for(int i = 0; i < 256; i++){
		if(bits[eviction_set[i]] > (WAIT_TIME*0.1)) {
			num |= (1<<i);
			printf("%d ", eviction_set[i]);
		 }
//		printf("%d ", bits[eviction_set[22]]);
		bits[eviction_set[i]] = 0;
	}
	return num;
}

int main(int argc, char **argv)
{
    // [Bonus] TODO: Put your covert channel setup code here
	ADDR_PTR *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
	*((char *)buf) = 1;
	printf("%p\n",buf);


//	while(!check_sender_status(buf)){sleep(1);}
	for(int i = 0; i < 256; i++){ eviction_set[i] = i;}
	setup_covert_channel(buf);
	
	uint64_t test = get_set_index(0x7ffff6c00580);
	printf("%d\n", test);
    printf("Please press enter.\n");

//	while(1){printf("%d\n", test_l2_sets(buf));usleep(10);}

    char text_buf[2];
    fgets(text_buf, sizeof(text_buf), stdin);

    printf("Receiver now listening.\n");

    bool listening = true;
	uint32_t bits[256] = {0};
	uint32_t wait = 0;	
	uint8_t num = 0;
    while (listening) {
        // [Bonus] TODO: Put your covert channel code here
//		setup_covert_channel(buf);
		usleep(10);
		decode_covert_channel(buf, bits);
		wait += 1;
		if(wait >= WAIT_TIME) {num = construct_num(bits);wait = 0;printf("%d\n", num);}
    }

    printf("Receiver finished.\n");

    return 0;
}


