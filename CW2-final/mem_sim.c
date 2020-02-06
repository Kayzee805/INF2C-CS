/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Instructor: Boris Grot
 * *
 * *    TA: Siavash Katebzadeh
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
/* Do not add any more header files */

/*
 * Various structures
 */
typedef enum {FIFO, LRU, Random} replacement_p;

const char* get_replacement_policy(uint32_t p) {
    switch(p) {
    case FIFO: return "FIFO";
    case LRU: return "LRU";
    case Random: return "Random";
    default: assert(0); return "";
    };
    return "";
}

typedef struct {
    uint32_t address;
} mem_access_t;

// These are statistics for the cache and should be maintained by you.
typedef struct {
    uint32_t cache_hits;
    uint32_t cache_misses;
} result_t;


/*
 * Parameters for the cache that will be populated by the provided code skeleton.
 */

replacement_p replacement_policy = FIFO;
uint32_t associativity = 0;
uint32_t number_of_cache_blocks = 0;
uint32_t cache_block_size = 0;


/*
 * Each of the variables below must be populated by you.
 */
uint32_t g_num_cache_tag_bits = 0;
uint32_t g_cache_offset_bits= 0;
result_t g_result;


/* Reads a memory access from the trace file and returns
 * 32-bit physical memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1002];
    char* token = NULL;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file)!= NULL) {
        /* Get the address */
        token = strtok(string, " \n");
        access.address = (uint32_t)strtoul(token, NULL, 16);
        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_cache_tag_bits, uint32_t cache_offset_bits, result_t* r) {
    /* Do Not Modify This Function */

    uint32_t cache_total_hits = r->cache_hits;
    uint32_t cache_total_misses = r->cache_misses;
    printf("CacheTagBits:%u\n", num_cache_tag_bits);
    printf("CacheOffsetBits:%u\n", cache_offset_bits);
    printf("Cache:hits:%u\n", r->cache_hits);
    printf("Cache:misses:%u\n", r->cache_misses);
    printf("Cache:hit-rate:%2.1f%%\n", cache_total_hits / (float)(cache_total_hits + cache_total_misses) * 100.0);
}

/*
 *
 * Add any global variables and/or functions here as needed.
 *
 */

uint32_t g_cache_index_bits= 0; //used to calculate index,offset,tag of an address

uint32_t getIndex(uint32_t address) {
    uint32_t index_val = (address << g_num_cache_tag_bits) >> (g_num_cache_tag_bits + g_cache_offset_bits) ;
    return (index_val);
}
//shifts the address to the left by tag bits to get rid of the tag bits then tag+offset bits
//so the only thing left is the index bits



uint32_t getOffset(uint32_t address) {
    uint32_t tag_index = (g_num_cache_tag_bits + g_cache_index_bits);
    return (address <<  tag_index) >>  tag_index;
}
// the above shifts all tag+index bit to the left so if tag+index = 11111 and address = 11111010 
//the return value will be 01000000 then shifting that to the right by the same amount will give
// 00000010

uint32_t getTag(uint32_t address){
    uint32_t index_offset = (g_cache_index_bits + g_cache_offset_bits);
    return address >> index_offset;
}
//shifts the address to the left so the only thing left is the tag 

int main(int argc, char** argv) {
    time_t t;
    /* Intializes random number generator */
    /* Important: *DO NOT* call this function anywhere else. */
    srand((unsigned) time(&t));
    /* ----------------------------------------------------- */
    /* ----------------------------------------------------- */

    /*
     *
     * Read command-line parameters and initialize configuration variables.
     *
     */
    int improper_args = 0;
    char file[10000];
    if (argc < 6) {
        improper_args = 1;
        printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */
        if (strcmp(argv[1], "FIFO") == 0) {
            replacement_policy = FIFO;
        } else if (strcmp(argv[1], "LRU") == 0) {
            replacement_policy = LRU;
        } else if (strcmp(argv[1], "Random") == 0) {
            replacement_policy = Random;
        } else {
            improper_args = 1;
            printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
        }
        associativity = atoi(argv[2]);
        number_of_cache_blocks = atoi(argv[3]);
        cache_block_size = atoi(argv[4]);
        strcpy(file, argv[5]);
    }
    if (improper_args) {
        exit(-1);
    }
    assert(number_of_cache_blocks == 16 || number_of_cache_blocks == 64 || number_of_cache_blocks == 256 || number_of_cache_blocks == 1024);
    assert(cache_block_size == 32 || cache_block_size == 64);
    assert(number_of_cache_blocks >= associativity);
    assert(associativity >= 1);

    printf("input:trace_file: %s\n", file);
    printf("input:replacement_policy: %s\n", get_replacement_policy(replacement_policy));
    printf("input:associativity: %u\n", associativity);
    printf("input:number_of_cache_blocks: %u\n", number_of_cache_blocks);
    printf("input:cache_block_size: %u\n", cache_block_size);
    printf("\n");

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file = fopen(file,"r");
    if (!ptr_file) {
        printf("Unable to open the trace file: %s\n", file);
        exit(-1);
    }

    /* result structure is initialized for you. */
    memset(&g_result, 0, sizeof(result_t));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task. */

    /* You may want to setup your Cache structure here. */

    //allocates enough memory to hold the amount of cache data, number of cache blocks
    //but its been split into differents, which is number of cache blocks/associativity
    uint32_t size = number_of_cache_blocks/associativity;
    uint32_t **array;
    array = malloc(sizeof(uint32_t*)*size);
    for(int i=0; i<size;i++){
        *(array+i)= malloc(associativity* sizeof(uint32_t));    //initializing the memory for each set
    }   


    uint32_t *counter = malloc(sizeof(uint32_t)*size);
    for(int i = 0;i<size;i++){                                  //initializing the counter array so all the values are 0
        *(counter+i) = 0;
    }


    int count = 0;
    int elemCounter = 0;
    g_cache_index_bits = log2(size);
    g_cache_offset_bits = log2(cache_block_size);
    g_num_cache_tag_bits = 32 - g_cache_index_bits - g_cache_offset_bits;

    mem_access_t access;
    /* Loop until the whole trace file has been read. */
    while(1) {
        access = read_transaction(ptr_file);
        // If no transactions left, break out of loop.
        if (access.address == 0)
            break;
        
        /* Add your code here */
        int hit = 0;
        uint32_t tag = getTag(access.address);
        uint32_t index = getIndex(access.address)% (size);
        // uint32_t index = index1 % associativity;    
      
        
        
        if( strcmp(get_replacement_policy(replacement_policy), "FIFO")== 0){
            for(int i = 0;i<associativity;i++){             //loops through every value in a set
              
                if((*(*(array+index)+i)) == tag){           //and checks if the value equals to tag
                    hit = 1;
                    g_result.cache_hits++;                  
                    break;                                  //when its found the break stops the for loop
                }
            }
            //this will fill teh empty array with value
            //if the array is full itll replace the oldest value
            if(hit==0){                                     
                //for miss
                counter[index] = counter[index] % associativity;        //this is to make sure that counter doesnt go exceed the max value
                                                                        //but this is also used to replace the oldest value         
                (*(*(array+index)+counter[index])) = tag;               
                //printf("its a miss tag %d (%d , %d)\n",tag,index, counter[index]);
                counter[index]++;
                g_result.cache_misses++;                                //the counter sets the index for the next value to be replaced
                                                                        //or added to the array
                
            }           
        }
        else if(strcmp(get_replacement_policy(replacement_policy), "LRU")== 0){
            elemCounter ++;

            for(int i = 0;i<associativity;i++){    
            
                if(*(*(array+index)+i) == tag){   //when tag is found
                    hit = 1;
                    g_result.cache_hits++;
                    for(int j = i;j<counter[index]-1;j++){              //this rotates the array from the index i to the second last elem
                        int temp = *(*(array+index)+j);                 //it puts the array that was hit at the end of the array
                        *(*(array+index)+j)= *(*(array+index)+(j+1));   //if the elem is behind i(has been looped over) it ignores it as it
                        *(*(array+index)+(j+1)) = temp;                 //was already ranked lower in terms of used recently
                    }
//printf("Its a hit %d %d \n", index, counter[index]);

                    break;
                }
            }
              if(hit == 0 && counter[index] <associativity-1 ){             //when array is empty/ NOT FILLED
                *(*(array+index)+(counter[index])) = tag;               //puts the tag in the value of count then increments count
                counter[index]++;
                g_result.cache_misses++;
              //  printf("Its a miss %d %d %d add new \n", index, counter[index], tag);


 
            }
            else if(hit == 0 && counter[index]== associativity-1){             //checks for when the counter[index] exceeds assoc            
                                                          //when it does it stops incrementing because its just gonna replace now
                                                                        //and its gonna the LRU elem which is 0
                *(*(array+index)+0) = tag;  
                                                                        //0 because the least used value is always placed at the front
                 for(int j = 0;j<counter[index]-1;j++){                 //loops for the entire loop beacause a new value needs to be palced at 
                        int temp =*(*(array+index)+j);                  //the end of the array
                        *(*(array+index)+j)= *(*(array+index)+(j+1));
                        *(*(array+index)+(j+1)) = temp;
 
                    }
                    g_result.cache_misses++;
                  //  printf("Its a miss %d %d %d replace \n", index, counter[index], tag);

            }
          


        }
        //checks for wrong input is checked above in the skeleton code
        else{
            //for random
            int hit =0;
            int ranNumb = rand() % associativity;
            for( int i = 0;i<associativity;i++){                        //checks if the value equals to tag and if it does increments cachehits
                if(*(*(array+index)+i)== tag){
                    hit = 1;
                    g_result.cache_hits++;
                }
            }
            if(hit == 0){                                               //when not hit
                *(*(array+index)+(ranNumb)) = tag;                      //it replaces/adds the tag to a random location which is within
                g_result.cache_misses++;                                //the associativity range
            }

            
        }

    }

    /* Do not modify code below. */
    /* Make sure that all the parameters are appropriately populated. */
    //printf("value = %d \n",array[1][1]);
    //printf("counter = %d \n",counter[3]);                             //just checking if it frees the mem

    free(counter);
    free(array);
    //printf("counter = %d \n",counter[1]);
    //printf("value = %d \n",array[1][1]);
    print_statistics(g_num_cache_tag_bits, g_cache_offset_bits, &g_result);

    /* Close the trace file. */
    fclose(ptr_file);
    return 0;
}
