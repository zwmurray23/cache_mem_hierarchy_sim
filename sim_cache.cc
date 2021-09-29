#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "sim_cache.h"

/****************************************************************
 *
 *  Written by Zachary Murray
 *  ECE 563: Microprocessor Architecture
 *  Project 1: Cache Design, Memory Hierarchy Design
 *  Due September 24, 2021
 *  I implemented my design using a vector of vectors
 *  which contained "blocks"
 *
 ****************************************************************/


/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim_cache 32 8192 4 7 262144 8 gcc_trace.txt
    argc = 8
    argv[0] = "sim_cache"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    cache_params params;    // look at sim_cache.h header file for the the definition of struct cache_params
//    char rw;                // variable holds read/write type read from input file. The array size is 2 because it holds 'r' or 'w' and '\0'. Make sure to adapt in future projects
    unsigned long int addr; // Variable holds the address read from input file

    //ZWM Code ----------------------------------------------

    char firstPass = 'y';
    cache * L1 = nullptr;
    cache * L2 = nullptr;
    char vc_enable = 'n';

    //End of ZWM Code ----------------------------------------



    if(argc != 8)           // Checks if correct number of inputs have been given. Throw error and exit if wrong
    {
        printf("Error: Expected inputs:7 Given inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    params.block_size       = strtoul(argv[1], NULL, 10);
    params.l1_size          = strtoul(argv[2], NULL, 10);
    params.l1_assoc         = strtoul(argv[3], NULL, 10);
    params.vc_num_blocks    = strtoul(argv[4], NULL, 10);
    params.l2_size          = strtoul(argv[5], NULL, 10);
    params.l2_assoc         = strtoul(argv[6], NULL, 10);
    trace_file              = argv[7];

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    // Print params
    printf("  ===== Simulator configuration =====\n"
            "  BLOCKSIZE:                        %lu\n"
            "  L1_SIZE:                          %lu\n"
            "  L1_ASSOC:                         %lu\n"
            "  VC_NUM_BLOCKS:                    %lu\n"
            "  L2_SIZE:                          %lu\n"
            "  L2_ASSOC:                         %lu\n"
            "  trace_file:                       %s\n\n"
//            "  ===================================\n\n"
            , params.block_size, params.l1_size, params.l1_assoc, params.vc_num_blocks, params.l2_size, params.l2_assoc, trace_file);

    char str[2];

    //-------------

    unsigned int blockOffset = log2(params.block_size);
    unsigned long int newAddr;

    //-------------

    while(fscanf(FP, "%s %lx", str, &addr) != EOF)
    {
        
      //  rw = str[0];
      //  if (rw == 'r')
      //      printf("%s %lx\n", "read", addr);           // Print and test if file is read correctly
      //  else if (rw == 'w')
      //      printf("%s %lx\n", "write", addr);          // Print and test if file is read correctly
        /*************************************
        **************************************/


        //First round through create cache structure
        //i.e. Build an L1 Cache by Default

        //str[0] is the w or r


        //newAddr is what must be passed to the memory hierarchy.
        newAddr = addr >> blockOffset; // Provides the address to be sent to cache. Stripped of block offset bits.

        if(params.vc_num_blocks > 0) vc_enable = 'y';


        if(firstPass == 'y') {

            firstPass = 'n';
            L1 = new cache (params.l1_size, params.l1_assoc, params.block_size, params.vc_num_blocks, vc_enable); // Initialization of L1 cache regardless of VC Args, default next level is nullptr

            // Creates an L2 cache if necessary and updates the next level address of L1.
            if (params.l2_size > 0) {
                L2 = new cache (params.l2_size, params.l2_assoc, params.block_size, 0, 'n'); /** changed vc ways to 0 for L2 **/
                L1->update_next_level( L2 );
            }

        }

        // After "first-pass" code call L1 cache member function "request" to install blocks. It can then call
        // other cache member functions as needed.

        L1->request( str[0], newAddr);




        /*************************************
        **************************************/
    }

    printf("  ===== L1 contents =====\n" );

    L1->print_cache();

    printf( "\n" ); //This should set up the next cache contents or simulation results to print properly


    if( vc_enable == 'y') {

        printf("  ===== VC contents =====\n" );

        //VC Contents

        L1->print_vc();

        printf( "\n");

    }



    if( params.l2_size > 0) {

        printf("  ===== L2 contents =====\n" );

        L2->print_cache();

        printf( "\n" ); //This should set up the next cache contents or simulation results to print properly

    }

        printf( "  ===== Simulation Results =====\n");


        if( (vc_enable == 'n') && (params.l2_size == 0) ) { //In this case there is no victim cache, and L2 doesn't exist.

            double L1_miss_rate_top = (L1->get_read_misses() + L1->get_write_misses() - L1->get_swaps());
            double L1_miss_rate_bottom = (L1->get_reads() + L1->get_writes());
            double L1_swap_request_rate_top = L1->get_swap_requests();
            double L1_swap_request_rate_bottom = (L1->get_reads() + L1->get_writes());

            printf(
                    "  a. number of L1 reads:                          %u\n"
                    "  b. number of L1 read misses:                    %u\n"
                    "  c. number of L1 writes:                         %u\n"
                    "  d. number of L1 write misses:                   %u\n"
                    "  e. number of swap requests:                     %u\n"
                    "  f. swap request rate:                           %.4f\n"
                    "  g. number of swaps:                             %u\n"
                    "  h. combined L1+VC miss rate:                    %.4f\n"
                    "  i. number writebacks from L1/VC:                %u\n"
                    "  j. number of L2 reads:                          %u\n"
                    "  k. number of L2 read misses:                    %u\n"
                    "  l. number of L2 writes:                         %u\n"
                    "  m. number of L2 write misses:                   %u\n"
                    "  n. L2 miss rate:                                %.4f\n"
                    "  o. number of writebacks from L2:                %u\n"
                    "  p. total memory traffic:                        %u\n",
                    L1->get_reads(), L1->get_read_misses(), L1->get_writes(), L1->get_write_misses(), L1->get_swap_requests(),
                    L1_swap_request_rate_top / L1_swap_request_rate_bottom, L1->get_swaps(),
                    L1_miss_rate_top / L1_miss_rate_bottom,
                    L1->get_write_backs(), 0, 0, 0, 0, 0.0, 0,
                    L1->get_read_misses() + L1->get_write_misses() - L1->get_swaps() + L1->get_write_backs()


            );
            return 0;
       } //End of no VC, no L2

    if( (vc_enable == 'y') && (params.l2_size == 0) ) { //In this case there is a victim cache, and L2 doesn't exist.

        double L1_miss_rate_top = (L1->get_read_misses() + L1->get_write_misses() - L1->get_swaps());
        double L1_miss_rate_bottom = (L1->get_reads() + L1->get_writes());
        double L1_swap_request_rate_top = L1->get_swap_requests();
        double L1_swap_request_rate_bottom = (L1->get_reads() + L1->get_writes());

        printf(
                "  a. number of L1 reads:                          %u\n"
                "  b. number of L1 read misses:                    %u\n"
                "  c. number of L1 writes:                         %u\n"
                "  d. number of L1 write misses:                   %u\n"
                "  e. number of swap requests:                     %u\n"
                "  f. swap request rate:                           %.4f\n"
                "  g. number of swaps:                             %u\n"
                "  h. combined L1+VC miss rate:                    %.4f\n"
                "  i. number writebacks from L1/VC:                %u\n"
                "  j. number of L2 reads:                          %u\n"
                "  k. number of L2 read misses:                    %u\n"
                "  l. number of L2 writes:                         %u\n"
                "  m. number of L2 write misses:                   %u\n"
                "  n. L2 miss rate:                                %.4f\n"
                "  o. number of writebacks from L2:                %u\n"
                "  p. total memory traffic:                        %u\n",
                L1->get_reads(), L1->get_read_misses(), L1->get_writes(), L1->get_write_misses(), L1->get_swap_requests(),
                L1_swap_request_rate_top / L1_swap_request_rate_bottom, L1->get_swaps(),
                L1_miss_rate_top / L1_miss_rate_bottom,
                L1->get_write_backs(), 0, 0, 0, 0, 0.0, 0,
                L1->get_read_misses() + L1->get_write_misses() - L1->get_swaps() + L1->get_write_backs()


        );
        return 0;
    } //End of no VC, no L2


    if( vc_enable == 'n' ) { //In this case there is no victim cache, and L2 exists.

        double L1_miss_rate_top = (L1->get_read_misses() + L1->get_write_misses() - L1->get_swaps());
        double L1_miss_rate_bottom = (L1->get_reads() + L1->get_writes());
        double L1_swap_request_rate_top = L1->get_swap_requests();
        double L1_swap_request_rate_bottom = (L1->get_reads() + L1->get_writes());
        double L2_miss_rate_top = L2->get_read_misses();
        double L2_miss_rate_bottom = L2->get_reads();


        printf(
                "  a. number of L1 reads:                          %u\n"
                "  b. number of L1 read misses:                    %u\n"
                "  c. number of L1 writes:                         %u\n"
                "  d. number of L1 write misses:                   %u\n"
                "  e. number of swap requests:                     %u\n"
                "  f. swap request rate:                           %.4f\n"
                "  g. number of swaps:                             %u\n"
                "  h. combined L1+VC miss rate:                    %.4f\n"
                "  i. number writebacks from L1/VC:                %u\n"
                "  j. number of L2 reads:                          %u\n"
                "  k. number of L2 read misses:                    %u\n"
                "  l. number of L2 writes:                         %u\n"
                "  m. number of L2 write misses:                   %u\n"
                "  n. L2 miss rate:                                %.4f\n"
                "  o. number of writebacks from L2:                %u\n"
                "  p. total memory traffic:                        %u\n",
                L1->get_reads(), L1->get_read_misses(), L1->get_writes(), L1->get_write_misses(), L1->get_swap_requests(),
                L1_swap_request_rate_top / L1_swap_request_rate_bottom, L1->get_swaps(),
                L1_miss_rate_top / L1_miss_rate_bottom,
                L1->get_write_backs(), L2->get_reads(), L2->get_read_misses(), L2->get_writes(), L2->get_write_misses(),
                L2_miss_rate_top / L2_miss_rate_bottom, L2->get_write_backs(),
                L2->get_read_misses() + L2->get_write_misses() + L2->get_write_backs()

        );
        return 0;
    } //End of no VC, no L2


     //In this case there is a victim cache, and L2 exists.

        double L1_miss_rate_top = (L1->get_read_misses() + L1->get_write_misses() - L1->get_swaps());
        double L1_miss_rate_bottom = (L1->get_reads() + L1->get_writes());
        double L1_swap_request_rate_top = L1->get_swap_requests();
        double L1_swap_request_rate_bottom = (L1->get_reads() + L1->get_writes());
        double L2_miss_rate_top = L2->get_read_misses();
        double L2_miss_rate_bottom = L2->get_reads();


        printf(
                "  a. number of L1 reads:                          %u\n"
                "  b. number of L1 read misses:                    %u\n"
                "  c. number of L1 writes:                         %u\n"
                "  d. number of L1 write misses:                   %u\n"
                "  e. number of swap requests:                     %u\n"
                "  f. swap request rate:                           %.4f\n"
                "  g. number of swaps:                             %u\n"
                "  h. combined L1+VC miss rate:                    %.4f\n"
                "  i. number writebacks from L1/VC:                %u\n"
                "  j. number of L2 reads:                          %u\n"
                "  k. number of L2 read misses:                    %u\n"
                "  l. number of L2 writes:                         %u\n"
                "  m. number of L2 write misses:                   %u\n"
                "  n. L2 miss rate:                                %.4f\n"
                "  o. number of writebacks from L2:                %u\n"
                "  p. total memory traffic:                        %u\n",
                L1->get_reads(), L1->get_read_misses(), L1->get_writes(), L1->get_write_misses(), L1->get_swap_requests(),
                L1_swap_request_rate_top / L1_swap_request_rate_bottom, L1->get_swaps(),
                L1_miss_rate_top / L1_miss_rate_bottom,
                L1->get_write_backs(), L2->get_reads(), L2->get_read_misses(), L2->get_writes(), L2->get_write_misses(),
                L2_miss_rate_top / L2_miss_rate_bottom, L2->get_write_backs(),
                L2->get_read_misses() + L2->get_write_misses() + L2->get_write_backs()

        );
        return 0;
     //End of VC, L2

    return 0;
}
