#include <unistd.h>
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
<<<<<<< HEAD
#define BUFF_SIZE (1<<21)
#define L2_HIT_THRESHOLD 31
=======
#define BUFF_SIZE (1<<24)
#define L2_HIT_THRESHOLD 35
>>>>>>> master
#define L2_SIZE 262144
#define CACHE_LINE 64
#define L2_SET_INDEX_MASK 0x0ffc0
#define CACHE_LINE_BITS 6
#define L2_SET_SIZE 32
#define WAIT_TIME 10000
<<<<<<< HEAD
static uint32_t eviction_set[8] = {2,4,6,8,10,12,14,16};
=======
static uint32_t eviction_set[8] = {2,6,10,14,18,22,26,30};
static uint8_t start_decode = 0;
>>>>>>> master

static inline uint32_t get_set_index(ADDR_PTR* addr){
    //return (((uintptr_t)addr & L2_SET_INDEX_MASK) >> CACHE_LINE_BITS);
	return ((uint64_t)addr >> 6) & ((1 << 10) - 1);
}
// load buf to fill up eviction sets
<<<<<<< HEAD
static void setup_covert_channel(void* buf){
	void* tmp = buf;
    int attempt = 0;
    while(attempt++ < 20){
        buf = tmp;
		for(int i = 0; i < 8; i++){
			while(get_set_index(buf) != eviction_set[i]) buf += 1;
			void* end = buf+L2_SET_SIZE;
=======
static void setup_covert_channel(ADDR_PTR* buf){
	ADDR_PTR* tmp = buf;
    int attempt = 0;
    while(attempt++ < 2){
        buf = tmp;
		for(int i = 0; i < 8; i++){
			while(get_set_index(buf) != eviction_set[i]) buf += 1;
			ADDR_PTR* end = buf+L2_SET_SIZE;
>>>>>>> master
	        for(; buf < end; buf+=1){
    	        *((char*)buf) = 1;
        	}
		}
    }
}

<<<<<<< HEAD
// as sender start running , all cache in L2 should miss
// return 1 if send started, else 0
static uint8_t check_sender_status(void* buf){
	void* tmp = buf;
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

static uint8_t check_send_data_signal(void* buf){
	while(get_set_index(buf) != 0) buf += CACHE_LINE;
	void* tmp = buf;
	// check if set 0 is hit
	for(; buf < tmp + L2_SET_SIZE; buf+=CACHE_LINE){
		// hit
		if( measure_one_block_access_time(buf) < L2_HIT_THRESHOLD) return 0;
	}
	// when all miss in set 0, sender signaled data transmission
	return 1;
}

static void decode_covert_channel(void* buf, uint32_t* bits){
	for(int i = 0; i < 8; i++){
		while(get_set_index(buf) != 0) buf += 1;
		// miss entire set, 4 way in L2
		uint8_t misses = 0;
		void* set = buf;
		set += L2_SET_SIZE * eviction_set[i];
=======

static uint8_t check_send_data_signal(ADDR_PTR* buf){
	while(get_set_index(buf) != 0) buf += 8;
	uint8_t misses = 0;
//  set += L2_SET_SIZE * eviction_set[i];
   	for (int j = 0; j < 4; j++){
      	if( measure_one_block_access_time(buf) > L2_HIT_THRESHOLD){
          	misses += 1;
         	buf += 8;
     	}
   	}
   	if (misses >= 3) return 0;

	return 1;
}

static void decode_covert_channel(ADDR_PTR* buf, uint32_t* bits){
	for(int i = 0; i < 8; i++){
		while(get_set_index(buf) != eviction_set[i]) buf += 1;
		// miss entire set, 4 way in L2
		uint8_t misses = 0;
		ADDR_PTR* set = buf;
//		set += L2_SET_SIZE * eviction_set[i];
>>>>>>> master
		for (int j = 0; j < 4; j++){
			if( measure_one_block_access_time(set) > L2_HIT_THRESHOLD){
				misses += 1;
				set += 8;
			}
		}	
<<<<<<< HEAD
		if (misses >= 2) bits[eviction_set[i]] += 1;
=======
		if (misses >= 3) bits[i] += 1;
>>>>>>> master
	}
	
//	return recv_num;	
}

<<<<<<< HEAD
uint8_t test_l2_sets(void* buf){
	uint8_t recv_num = 0;
//    while(get_set_index(buf) != 8) {buf += 1;}
=======
uint8_t test_l2_sets(ADDR_PTR* buf){
	uint8_t recv_num = 0;
    while(get_set_index(buf) != 3) {buf += 1;}
//	printf("%p\n", buf);
>>>>>>> master
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
	for(int i = 0; i < 8; i++){
<<<<<<< HEAD
		if(bits[eviction_set[i]] > (WAIT_TIME*0.5)) num |= (1<<i);
//		printf("%d ", bits[eviction_set[i]]);
		bits[eviction_set[i]] = 0;
=======
		if(bits[i] > (WAIT_TIME*0.3)) {
			num |= (1<<i);
		}
//		printf("%d ", bits[i]);
		bits[i] = 0;
>>>>>>> master
	}
	return num;
}

int main(int argc, char **argv)
{
    // [Bonus] TODO: Put your covert channel setup code here
<<<<<<< HEAD
	void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
=======
	ADDR_PTR *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
>>>>>>> master
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
	*((char *)buf) = 1;
	printf("%p\n",buf);

<<<<<<< HEAD

	// wait for sender to run
//	while(!check_sender_status(buf)){sleep(1);}

	setup_covert_channel(buf);

    printf("Please press enter.\n");

	while(1){printf("%d\n", test_l2_sets(buf));usleep(1000);}
=======
	setup_covert_channel(buf);
	while(get_set_index(buf) != 0) buf += 1;	

    printf("Please press enter.\n");

//	while(1){printf("%d\n", test_l2_sets(buf));usleep(10);}
>>>>>>> master

    char text_buf[2];
    fgets(text_buf, sizeof(text_buf), stdin);

    printf("Receiver now listening.\n");

    bool listening = true;
<<<<<<< HEAD
	uint32_t bits[20] = {0};
	uint32_t wait = 0;	
	uint8_t num = 0;
    while (listening) {
        // [Bonus] TODO: Put your covert channel code here
		setup_covert_channel(buf);
		usleep(100);
		decode_covert_channel(buf, bits);
		wait += 1;
		if(wait >= WAIT_TIME) {num = construct_num(bits);wait = 0;printf("%d\n", num);}
=======
	uint32_t bits[8] = {0};
	volatile uint64_t start = 0; 
	uint32_t wait = 0;	
	uint8_t num = 0;
	int8_t shift = 0;
	uint8_t out = 0;
    while (listening) {
        // [Bonus] TODO: Put your covert channel code here

		// fill up eviction set: prime
		setup_covert_channel(buf);
		// wait
		if(start_decode) usleep(115);
		else usleep(75);
		// probe
		decode_covert_channel(buf, bits);
		// number of samples counter
		wait += 1;
		// reached max samples
		if(wait >= WAIT_TIME) {
			// construct num based on bits probed in decode_covert_channel
			num = construct_num(bits);
			wait = 0;
			// when all miss, means bit 1 read in
			if(num < 250) {
				start += 1;
				if(start > 15) out |= (1<<shift);
			}
		   		
			if(start > 15) {printf("%d ",num);shift += 1;}
			if(shift > 7) {
				printf("\nReceived: %d\n", out);
				out = 0,shift = 0,start = 0,start_decode = 0;
			}
		}
>>>>>>> master
    }

    printf("Receiver finished.\n");

    return 0;
}
<<<<<<< HEAD


=======
>>>>>>> master
