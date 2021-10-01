#ifndef SIM_CACHE_H
#define SIM_CACHE_H
#include <cstdlib>
#include <vector>
#include <iostream>

/****************************************************************
 *
 *  Written by Zachary Murray
 *  ECE 563: Microprocessor Architecture
 *  Project 1: Cache Design, Memory Hierarchy Design
 *  Due September 24, 2021
 *  I implemented my design using a vector of vectors
 *  which contained "blocks"
 *
 *
 *  This code was developed with time as a limiting constraint.
 *  Forgive "quick fixes" where possible. I am reworking code
 *  as time permits to make it more clean, readable, and
 *  efficient.
 *
 *
 ****************************************************************/


typedef struct cache_params{
    unsigned long int block_size;
    unsigned long int l1_size;
    unsigned long int l1_assoc;
    unsigned long int vc_num_blocks;
    unsigned long int l2_size;
    unsigned long int l2_assoc;
}cache_params;

// Put additional data structures here as per your requirement





class block {

    // Overall generic block class to be used throughout the memory hierarchy
    // This class is inclusive of metadata required for correct simulation.

private:
    unsigned int lru_counter;   // LRU counter? will need to be block specific so one variable will not work but needs to be accounted for
    unsigned long int tag;
    unsigned int dirty_bit;
    unsigned int valid_bit;
    unsigned long int full_address;

public:

    block() {
        lru_counter = 0;     // initializes LRU counter for each bit to NULL
        tag = 0;             // initializes tag to NULL
        dirty_bit = 0;          // initializes dirty bit to 0 although unnecessary
        valid_bit = 0;          // initializes valid bit to 0 although
        full_address = 0;

    } //end of block constructor



    // update_lru is a member function that simply increases the lru counter for the block.
    void update_lru () {
        if(valid_bit)
        lru_counter++;
    };

    void set_dirty_bit () {
        dirty_bit = 1;
    };

    unsigned int check_valid () {
        return valid_bit;
    };

    unsigned int check_dirty () {
        return dirty_bit;
    };


    // Check for tag examines a block to see if it is valid, and then checks to see if it is the block in question.
    unsigned int check_for_tag( unsigned long int comparison_tag ) {

        if(valid_bit) {
            if(tag == comparison_tag) return 1; //If valid bit is set and tag is equal, return a 1 (true)
            else return 0;
        }
        else return 0;  //If the tag does not match or the block is invalid return a 0 (false)

    };

    // initialize_block is a member function meant to be used for initializing an invalid block or after an eviction.
    void initialize_block( unsigned long int localTag, unsigned long int fullAddress ) {

        lru_counter = 0;
        tag = localTag;
        valid_bit = 1;
        dirty_bit = 0;
        full_address = fullAddress;

    };


    //sets the lru_counter bit of the block under test to 0, making it the MRU.
    void make_mru() {
        lru_counter = 0;
    }

    //Gets the LRU counter bit for other functions
    unsigned int check_lru_counter() {
        return lru_counter;
    }

    unsigned long int get_full_address() {
        return full_address;
    }

    unsigned long int get_tag() {
        return tag;
    }

    void update_tag( unsigned long int newTag ) {
        tag = newTag;
    }




};








class cache {

    //Overall generic cache class to be used throughout the memory hierarchy

public:
    unsigned int cacheSize;  //Size of cache passed in
    unsigned int assoc = 0;      //Set-associativity of cache
    unsigned int blockSize = 0;  //Block Size
    unsigned int numSets = 0;    //Number of Sets in the cache
    unsigned int numVC = 0;      //Number of ways in the Victim Cache if used
    unsigned int reads = 0;
    unsigned int read_misses = 0;
    unsigned int writes = 0;
    unsigned int write_misses = 0;
    unsigned int swap_requests = 0;
    unsigned int write_backs = 0;
    unsigned int print_dirty_true = 0;
    unsigned int print_valid = 0;
    unsigned int swaps = 0;
    cache * nextLevel = nullptr;       // Pointer to the next level of memory. If none, nullptr / NULL should suffice.
    unsigned int block_offset_bits = 0; // # of block offset bits.
    unsigned int index_bits = 0; // # of index bits.
    std::vector <block> victimCache;
    std::vector <block> ways;
    std::vector< std::vector< block > > sets;




public:

   cache( unsigned int s, unsigned int a, unsigned int b, unsigned int vc_ways, char vc_enable = 'n', cache * next = nullptr ) {
        print_dirty_true = 0;
        print_valid = 0;
        cacheSize = s;
        assoc = a;
        blockSize = b;
        if(a == 0) numSets = 1; //Added in a clause to deal with the case of a fully associative cache.
        else numSets = (s) / (a * b);
        block_offset_bits = log2(blockSize);  // Assigns variable with the number of Block Offset Bits //Safekeeping static_cast <unsigned int> (temp_var)
        index_bits = log2(numSets); // Assigns variable with the number of Index bits
        numVC = vc_ways;
        block * tempBlock;

        if(assoc ==0) assoc = cacheSize / blockSize;


        for(unsigned int i=0; i<assoc; i++) {
            tempBlock = new block;
            ways.push_back( *tempBlock );
        }


        /** Victim Cache code addition **/
        if( vc_enable == 'y') {  //Clause to set up the victim cache

            for(unsigned int i=0; i<vc_ways; i++) {
                tempBlock = new block;
                victimCache.push_back( *tempBlock );
            }
        }
        /** END of Victim Cache initialization **/

        for (unsigned int i=0; i<numSets; i++) {
            sets.push_back(ways);
        }





    } //end of cache constructor

    void update_next_level ( cache * next ) {
       nextLevel = next;
   };


    void request(char type, unsigned long int address) {
        int count = 0;
        unsigned long int index_result = 0;
        unsigned long int localTag = address;
        unsigned int block_matches = 0;
        unsigned int way_counter = 0;
        block block_under_test;
        unsigned int lru_storage = 0;
        unsigned int vc_lru_storage = 0;
        bool invalid_block_available = false;
        unsigned int max_lru;
        unsigned int vc_counter = 0;
        block vc_block_under_test;
        unsigned int vc_lru_max;

        //gets the index mapping to a particular set
        for( unsigned int i=0; i<index_bits; i++ ) {
            if( localTag & 0b1 ) index_result += pow(2, count);
            count++;
            localTag >>= 1;
        }
        // localTag == just the tag. address == full address and is what should be passed to next level.

        /*****************  Index obtained - begin search for tag  ***************************/

        ways = sets.at(index_result); // Selects the relative set to be sorted -- is unchanged throughout the function

        for(way_counter = 0; way_counter < assoc; way_counter++) {
            block_under_test = ways.at(way_counter);    // At this point the BUT can be analyzed.
            block_matches = block_under_test.check_for_tag( localTag );   // Indicator of whether the block is the correct one.
            if(block_matches) break;
        }

        // way_counter contains the number of the way where the matched block is stored, if it is.
        // IF it is stored is determined by the variable "block_matches"

        /*************** Cases for write hit and write miss are tackled below **************************/

        switch ( type ) {

            case 'w':

                /********************************* WRITE HIT ******************************************/
                if (block_matches) {
                    lru_storage = block_under_test.check_lru_counter();  //Current LRU value stored
                    writes++; //Increment the number of writes
                    block_under_test.set_dirty_bit(); //Set the dirty bit to 1
                    block_under_test.make_mru(); //Set the LRU value to 0 based on write hit that occurred.
                    ways.at(way_counter) = block_under_test; /** **/
                    for (unsigned int j = 0; j < assoc; j++) {

                        block_under_test = ways.at(j); //increments j to move through the ways of the set
                        if ((j != way_counter) && (block_under_test.check_lru_counter() < lru_storage)) {
                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                            ways.at(j) = block_under_test; /** **/
                        }

                    } //end of for loop
                } // End of write hit logic

                    /********************************* WRITE MISS *****************************************/
                else {
                    writes++;
                    write_misses++;
                    for (way_counter = 0; way_counter < assoc; way_counter++) {
                        block_under_test = ways.at(way_counter);
                        if (!block_under_test.check_valid()) {  // IF the block being examined is invalid
                            invalid_block_available = true;
                        }
                        if (invalid_block_available) break; //exits for loop if a block is available and leaves way_counter at that block.
                    }

                    // At this point way_counter should refer to the location of the invalid block

                    /**** CASE: Invalid block is available to be replaced ****/

                    if (invalid_block_available) { // handles the case for when there is an invalid block to be replaced

                        if (nextLevel == nullptr) { //In this case the cache is the lowest in the memory hierarchy.
                            block_under_test = ways.at(way_counter);
                            block_under_test.initialize_block(localTag,
                                                              address); //At this point the block should now be updated with the new tag & metadata reset
                            block_under_test.set_dirty_bit();
                            ways.at(way_counter) = block_under_test;

                            // Must update LRU of remaining blocks
                            for (unsigned int j = 0; j < assoc; j++) {
                                block_under_test = ways.at(j);
                                if ((j != way_counter)) {
                                    block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                    ways.at(j) = block_under_test; /** **/
                                }
                            }
                        }
                        else {  // In this case the cache in question would be L1 or its victim cache -- need block from next level

                            nextLevel->request('r', address); // Issues a read request to the next level for the block
                            block_under_test = ways.at(way_counter);
                            block_under_test.initialize_block(localTag,
                                                              address); //At this point the block should now be updated with the new tag & metadata reset
                            block_under_test.set_dirty_bit();
                            ways.at(way_counter) = block_under_test; /** **/

                            // Must update LRU of remaining blocks
                            for (unsigned int j = 0; j < assoc; j++) {
                                block_under_test = ways.at(j);
                                if ((j != way_counter)) {
                                    block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                    ways.at(j) = block_under_test; /** **/
                                }
                            }

                        }

                    } // After this begins the scenario where no invalid block is available to replace.
                        /**** ENDCASE: Invalid block is available to be replaced ****/






                        /**** CASE: VALID LRU BLOCK IN CACHE MUST BE REPLACED ****/

                        // First step is to find the least recently used block

                        /**** VICTIM CACHE REQUIRED FOR THIS INSTANCE ****/


                        //No invalid blocks in set

                    else {

                        if (numVC > 0) { //Check to see if the block is in the VC
                            swap_requests++; //swap request added because set was full.

                            for (vc_counter = 0; vc_counter < numVC; vc_counter++) {
                                vc_block_under_test = victimCache.at(
                                        vc_counter);    // At this point the BUT can be analyzed.
                                block_matches = vc_block_under_test.check_for_tag(
                                        address);   // Checks for the tag (NO INDEX BITS!)
                                if (block_matches) break;
                            }



                            /** VC Hit - Write **/
                            if (block_matches) {
                                vc_lru_storage = vc_block_under_test.check_lru_counter();  //Current LRU value stored in L1

                                //GET LRU Block from L1 ---------------------------

                                max_lru = assoc - 1;

                                // Find the LRU Block of L1 cache
                                for (way_counter = 0; way_counter < assoc; way_counter++) {
                                    block_under_test = ways.at(way_counter);


                                    //Check to see if the block in question is actually the LRU
                                    if (block_under_test.check_valid() &&
                                        (block_under_test.check_lru_counter() == max_lru))
                                        break;

                                }

                                //GET LRU Block from L1 ---------------------------



                                lru_storage = block_under_test.check_lru_counter();
                                swaps++; //Increment the number of swaps


                                //actual swapping of blocks

                                block tempBlock;
                                unsigned long int tempTag;
                                tempBlock = vc_block_under_test; //tempBlock gets vc_block_under_test which has full length tags
                                tempTag = vc_block_under_test.get_full_address() >> index_bits; //provides new tag for block_under_test
                                vc_block_under_test = block_under_test; // block_under_test has a shifted tag -- need to fix
                                vc_block_under_test.update_tag( vc_block_under_test.get_full_address() ); //moves the address to tag slot.
                                block_under_test = tempBlock;
                                block_under_test.update_tag( tempTag ); // Gives the block_under_test the abbreviated tag as needed.

                                block_under_test.set_dirty_bit(); //performs the write that was originally requested
                                block_under_test.make_mru(); //Set the LRU value to 0 based on hit that occurred.
                                vc_block_under_test.make_mru(); //Set the LRU value to 0 for VC block as well
                                ways.at(way_counter) = block_under_test; /** **/ //Save out the edited block
                                victimCache.at(vc_counter) = vc_block_under_test; /** **/ //save out the edited block

                                for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                    block_under_test = ways.at(j); //increments j to move through the ways of the set
                                    if ((j != way_counter) && (block_under_test.check_lru_counter() < lru_storage)) {
                                        block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                        ways.at(j) = block_under_test; /** **/
                                    }

                                } //end of for loop

                                for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                    vc_block_under_test = victimCache.at(
                                            j); //increments j to move through the ways of the set
                                    if ((j != vc_counter) &&
                                        (vc_block_under_test.check_lru_counter() < vc_lru_storage)) {
                                        vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                        victimCache.at(j) = vc_block_under_test; /** **/
                                    }

                                } //end of for loop






                            } // End of VC hit logic

                            /** VC MISS - Write **/

                            else {



                                //Figure out if there is an invalid block available



                                for (vc_counter = 0; vc_counter < numVC; vc_counter++) {

                                    vc_block_under_test = victimCache.at(vc_counter);

                                    if (!vc_block_under_test.check_valid()) { //valid bit is available
                                        invalid_block_available = true;
                                        break;
                                    }

                                }


                                max_lru = assoc - 1;

                                // Find the LRU Block of L1 cache
                                for (way_counter = 0; way_counter < assoc; way_counter++) {
                                    block_under_test = ways.at(way_counter);


                                    //Check to see if the block in question is actually the LRU
                                    if (block_under_test.check_valid() &&
                                        (block_under_test.check_lru_counter() == max_lru))
                                        break;

                                }






                                if (invalid_block_available) {  // ------------ store LRU data??

                                block tempBlock = ways.at(way_counter);
                                unsigned long int tempTag = tempBlock.get_full_address();
                                vc_block_under_test = ways.at(way_counter); //move in the victim block
                                vc_block_under_test.update_tag( tempTag ); //Provides VC with full length tag
                                vc_block_under_test.make_mru();
                                victimCache.at(vc_counter) = vc_block_under_test; /** **/ // Save out changes

                                for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                    vc_block_under_test = victimCache.at(
                                            j); //increments j to move through the ways of the set
                                    if ( j != vc_counter ) {  // When an invalid block is available all others LRU counters need to update
                                        vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                        victimCache.at(j) = vc_block_under_test; /** **/
                                    }

                                } //end of for loop

                                if (nextLevel != nullptr) nextLevel->request('r', address);

                                block_under_test = ways.at(way_counter);  // Identifies the LRU Block
                                lru_storage = block_under_test.check_lru_counter(); // store current LRU counter value
                                block_under_test.initialize_block(localTag, address); // allocate new block
                                block_under_test.set_dirty_bit(); //Set the dirty bit for the block just allocated per request.

                                ways.at(way_counter) = block_under_test; /** **/ // Save out changes

                                for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                    block_under_test = ways.at(
                                            j); //increments j to move through the ways of the set
                                    if ((j != way_counter) &&
                                        (block_under_test.check_lru_counter() < lru_storage)) {
                                        block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                        ways.at(j) = block_under_test; /** **/
                                    }

                                }

                            } else{  // Invalid block not available -- must use LRU of VC


                                    max_lru = assoc - 1;

                                    // Find the LRU Block of L1 cache
                                    for (way_counter = 0; way_counter < assoc; way_counter++) {
                                        block_under_test = ways.at(way_counter);


                                        //Check to see if the block in question is actually the LRU
                                        if (block_under_test.check_valid() &&
                                            (block_under_test.check_lru_counter() == max_lru))
                                            break;

                                    }



                                    vc_lru_max = numVC - 1;
                                // Find the LRU Block of VC
                                for (vc_counter = 0; vc_counter < numVC; vc_counter++) {
                                    vc_block_under_test = victimCache.at(vc_counter);

                                    //Check to see if the block in question is actually the LRU
                                    if (vc_block_under_test.check_valid() &&
                                        (vc_block_under_test.check_lru_counter() == vc_lru_max))
                                        break;

                                } // end of VC LRU for loop

                                // VC LRU found, evict block (write back if necessary and install block)

                                if (vc_block_under_test.check_dirty()) {  //dirty block needs writeback
                                    write_backs++;

                                    if (nextLevel != nullptr) { //VC block is dirty, next level available

                                        nextLevel->request('w', vc_block_under_test.get_full_address());

                                    }

                                    // Writeback processed, now moving to victim block allocation

                                    vc_lru_storage = vc_block_under_test.check_lru_counter();

                                    block tempBlock1 = ways.at(way_counter);
                                    unsigned long int tempTag1 = tempBlock1.get_full_address(); // obtain full address for VC tag

                                    vc_block_under_test = ways.at(way_counter); //move in the victim block
                                    vc_block_under_test.update_tag( tempTag1 );
                                    vc_block_under_test.make_mru();
                                    victimCache.at(vc_counter) = vc_block_under_test;

                                    for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                        vc_block_under_test = victimCache.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != vc_counter) &&
                                            (vc_block_under_test.check_lru_counter() < vc_lru_storage)) {
                                            vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            victimCache.at(j) = vc_block_under_test; /** **/
                                        }

                                    } //end of for loop


                                    // Now that victim block has been transferred to VC, the original block must be read in
                                    if( nextLevel != nullptr ) nextLevel->request('r', address);
                                    lru_storage = block_under_test.check_lru_counter();
                                    block_under_test.initialize_block(localTag, address);
                                    block_under_test.set_dirty_bit(); //Bring in new block and perform necessary write
                                    ways.at(way_counter) = block_under_test; /** **/ //Save out changes

                                    for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                        block_under_test = ways.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != way_counter) &&
                                            (block_under_test.check_lru_counter() < lru_storage)) {
                                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            ways.at(j) = block_under_test; /** **/
                                        }

                                    }

                                } else { //VC LRU Block is not dirty and can be replaced by new victim


                                    vc_lru_storage = vc_block_under_test.check_lru_counter();
                                    block tempBlock1 = ways.at(way_counter);
                                    unsigned long int tempTag1 = tempBlock1.get_full_address();

                                    vc_block_under_test = ways.at(way_counter); //move in the victim block
                                    vc_block_under_test.update_tag( tempTag1 );
                                    vc_block_under_test.make_mru();
                                    victimCache.at(vc_counter) = vc_block_under_test; /** **/ // Save out changes

                                    for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                        vc_block_under_test = victimCache.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != vc_counter) &&
                                            (vc_block_under_test.check_lru_counter() < vc_lru_storage)) {
                                            vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            victimCache.at(j) = vc_block_under_test; /** **/
                                        }

                                    } //end of for loop

                                    if (nextLevel != nullptr) nextLevel->request('r', address);

                                    lru_storage = block_under_test.check_lru_counter();
                                    block_under_test.initialize_block(localTag, address);
                                    block_under_test.set_dirty_bit();
                                    ways.at(way_counter) = block_under_test; /** **/

                                    for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for Victim cache

                                        block_under_test = ways.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != way_counter) &&
                                            (block_under_test.check_lru_counter() < lru_storage)) {
                                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            ways.at(j) = block_under_test; /** **/
                                        }

                                    }


                                } //end of VC Write Miss where VC LRU Must be evicted

                            }//test

                            }


                        }

                        else {//Consider adding catch all else statement to keep VC separate

                            max_lru = assoc - 1;

                            // Find the LRU Block
                            for (way_counter = 0; way_counter < assoc; way_counter++) {
                                block_under_test = ways.at(way_counter);


                                //Check to see if the block in question is actually the LRU
                                if (block_under_test.check_valid() &&
                                    (block_under_test.check_lru_counter() == max_lru))
                                    break;

                            }

                            /*** Dirty bit is set on LRU block ***/

                            if (block_under_test.check_dirty()) {

                                // way_counter should be set to LRU block's location

                                if (nextLevel ==
                                    nullptr) { //In this case the cache is the lowest in the memory hierarchy.
                                    block_under_test = ways.at(way_counter);
                                    write_backs++; // Increment writeback statistics
                                    block_under_test.initialize_block(localTag,
                                                                      address); //At this point the block should now be updated with the new tag & metadata reset
                                    block_under_test.set_dirty_bit();
                                    ways.at(way_counter) = block_under_test; /** **/

                                    // Must update LRU of remaining blocks
                                    for (unsigned int j = 0; j < assoc; j++) {
                                        block_under_test = ways.at(j);
                                        if ((j != way_counter)) {
                                            block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                            ways.at(j) = block_under_test; /** **/
                                        }
                                    }
                                } else {  // In this case the cache in question would be L1 or its victim cache

                                    block_under_test = ways.at(way_counter);
                                    nextLevel->request('w',
                                                       block_under_test.get_full_address()); // Issues a write request to the next level for the evicted block
                                    write_backs++;
                                    nextLevel->request('r', address);
                                    block_under_test.initialize_block(localTag,
                                                                      address); //At this point the block should now be updated with the new tag & metadata reset
                                    block_under_test.set_dirty_bit();
                                    ways.at(way_counter) = block_under_test; /** **/

                                    // Must update LRU of remaining blocks
                                    for (unsigned int j = 0; j < assoc; j++) {
                                        block_under_test = ways.at(j);
                                        if ((j != way_counter)) {
                                            block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                            ways.at(j) = block_under_test; /** **/
                                        }
                                    }

                                }

                            } //End of dirty LRU block code

                                /*** LRU Block is clean **/

                            else {

                                // way_counter should be set to LRU block's location

                                if (nextLevel ==
                                    nullptr) { //In this case the cache is the lowest in the memory hierarchy.
                                    block_under_test = ways.at(way_counter);
                                    block_under_test.initialize_block(localTag,
                                                                      address); //At this point the block should now be updated with the new tag & metadata reset
                                    block_under_test.set_dirty_bit();
                                    ways.at(way_counter) = block_under_test; /** **/

                                    // Must update LRU of remaining blocks
                                    for (unsigned int j = 0; j < assoc; j++) {
                                        block_under_test = ways.at(j);
                                        if ((j != way_counter)) {
                                            block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                            ways.at(j) = block_under_test; /** **/
                                        }
                                    }
                                } else {  // In this case the cache in question would be L1 or its victim cache

                                    block_under_test = ways.at(way_counter);
                                    nextLevel->request('r', address);
                                    block_under_test.initialize_block(localTag,
                                                                      address); //At this point the block should now be updated with the new tag & metadata reset
                                    block_under_test.set_dirty_bit();
                                    ways.at(way_counter) = block_under_test; /** **/

                                    // Must update LRU of remaining blocks
                                    for (unsigned int j = 0; j < assoc; j++) {
                                        block_under_test = ways.at(j);
                                        if ((j != way_counter)) {
                                            block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                            ways.at(j) = block_under_test; /** **/
                                        }
                                    }

                                } // End of L1 / VC read from next level for LRU Replacement

                            }

                        } //Adding a brakcet here should end no VC else statement

                    }
                } // NEW 1.15am 9.23
                /**** End of WRITE MISS ****/
                sets.at(index_result) = ways;
                return;


            case 'r':

                /************************************ READ HIT ****************************************/

                if (block_matches) {
                    lru_storage = block_under_test.check_lru_counter();  //Current LRU value stored
                    reads++; //Increment the number of reads
                    block_under_test.make_mru(); //Set the LRU value to 0 based on read hit that occurred.
                    ways.at(way_counter) = block_under_test; /** **/
                    for (unsigned int j = 0; j < assoc; j++) {

                        block_under_test = ways.at(j); //increments j to move through the ways of the set
                        if ((j != way_counter) && (block_under_test.check_lru_counter() < lru_storage)) {
                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                            ways.at(j) = block_under_test; /** **/
                        }

                    } //end of for loop
                } // End of read hit logic

                    /********************************* READ MISS *****************************************/
                else {
                    reads++;
                    read_misses++;
                    for (way_counter = 0; way_counter < assoc; way_counter++) {
                        block_under_test = ways.at(way_counter);
                        if (!block_under_test.check_valid()) {  // IF the block being examined is invalid
                            invalid_block_available = true;
                        }
                        if (invalid_block_available) break; //exits for loop if a block is available and leaves way_counter at that block.
                    }


                    // At this point way_counter should refer to the location of the invalid block

                    /**** CASE: Invalid block is available to be replaced ****/

                    if (invalid_block_available) { // handles the case for when there is an invalid block to be replaced

                        if (nextLevel == nullptr) { //In this case the cache is the lowest in the memory hierarchy.
                            block_under_test = ways.at(way_counter);
                            block_under_test.initialize_block(localTag,
                                                              address); //At this point the block should now be updated with the new tag & metadata reset
                            ways.at(way_counter) = block_under_test; /** **/

                            // Must update LRU of remaining blocks
                            for (unsigned int j = 0; j < assoc; j++) {
                                block_under_test = ways.at(j);
                                if ((j != way_counter)) {
                                    block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                    ways.at(j) = block_under_test; /** **/
                                }
                            }
                        } else {  // In this case the cache in question would be L1 or its victim cache

                            nextLevel->request('r', address); // Issues a read request to the next level for the block
                            block_under_test = ways.at(way_counter);
                            block_under_test.initialize_block(localTag,
                                                              address); //At this point the block should now be updated with the new tag & metadata reset
                            ways.at(way_counter) = block_under_test; /** **/


                            // Must update LRU of remaining blocks
                            for (unsigned int j = 0; j < assoc; j++) {
                                block_under_test = ways.at(j);
                                if ((j != way_counter)) {
                                    block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                    ways.at(j) = block_under_test; /** **/
                                }
                            }

                        }

                    } // After this begins the scenario where no invalid block is available to replace.
                        /**** ENDCASE: Invalid block is available to be replaced ****/






                        /**** CASE: VALID LRU BLOCK MUST BE REPLACED ****/

                        // First step is to find the least recently used block

                        /**** VICTIM CACHE REQUIRED FOR THIS INSTANCE ****/

                    else {

                        if (numVC > 0) { //Check to see if the block is in the VC
                            swap_requests++;


                            max_lru = assoc - 1;

                            // Find the LRU Block of L1 cache
                            for (way_counter = 0; way_counter < assoc; way_counter++) {
                                block_under_test = ways.at(way_counter);


                                //Check to see if the block in question is actually the LRU
                                if (block_under_test.check_valid() &&
                                    (block_under_test.check_lru_counter() == max_lru))
                                    break;

                            }


                            for (vc_counter = 0; vc_counter < numVC; vc_counter++) {
                                vc_block_under_test = victimCache.at(
                                        vc_counter);    // At this point the BUT can be analyzed.
                                block_matches = vc_block_under_test.check_for_tag(
                                        address);   // VC Uses the long version of the tag.
                                if (block_matches) break;
                            }



                            /** VC Hit - Read **/
                            if (block_matches) {
                                vc_lru_storage = vc_block_under_test.check_lru_counter();  //Current LRU value stored in VC
                                lru_storage = block_under_test.check_lru_counter(); //Current LRU value stored in L1
                                swaps++; //Increment the number of swaps


                                //actual swapping of blocks

                                block tempBlock;
                                tempBlock = block_under_test;
                                unsigned long int tempTag = tempBlock.get_full_address(); //Contains the full address from BUT
                                block tempBlock2 = vc_block_under_test;
                                unsigned long int tempTag1 = tempBlock2.get_tag() >> index_bits; // Should have the indexed tag now
                                block_under_test = vc_block_under_test;
                                block_under_test.update_tag( tempTag1 );
                                vc_block_under_test = tempBlock;
                                vc_block_under_test.update_tag( tempTag );



                                block_under_test.make_mru(); //Set the LRU value to 0 based on hit that occurred.
                                vc_block_under_test.make_mru();

                                ways.at(way_counter) = block_under_test; /** **/
                                victimCache.at(vc_counter) = vc_block_under_test; /** **/

                                for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                    block_under_test = ways.at(j); //increments j to move through the ways of the set
                                    if ((j != way_counter) && (block_under_test.check_lru_counter() < lru_storage)) {
                                        block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                        ways.at(j) = block_under_test; /** **/
                                    }

                                } //end of for loop

                                for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                    vc_block_under_test = victimCache.at(
                                            j); //increments j to move through the ways of the set
                                    if ((j != vc_counter) &&
                                        (vc_block_under_test.check_lru_counter() < vc_lru_storage)) {
                                        vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                        victimCache.at(j) = vc_block_under_test; /** **/
                                    }

                                } //end of for loop






                            } // End of VC hit logic

                                /** VC MISS - Read **/

                            else {

                                //SEE if VC has an invalid block available

                                for (vc_counter = 0; vc_counter < numVC; vc_counter++) {

                                    vc_block_under_test = victimCache.at(vc_counter);

                                    if (!vc_block_under_test.check_valid()) { //valid bit is available
                                        invalid_block_available = true;
                                        break;
                                    }

                                }

                                //VC counter should "point" to an invalid block

                                max_lru = assoc - 1;

                                // Find the LRU Block of L1 cache
                                for (way_counter = 0; way_counter < assoc; way_counter++) {
                                    block_under_test = ways.at(way_counter);


                                    //Check to see if the block in question is actually the LRU
                                    if (block_under_test.check_valid() &&
                                        (block_under_test.check_lru_counter() == max_lru))
                                        break;

                                }






                                if (invalid_block_available) {   // Invalid block is available within VC for victim

                                    lru_storage = block_under_test.check_lru_counter(); // Get LRU val of victim block

                                    block tempBlock;
                                    tempBlock = ways.at(way_counter);
                                    unsigned long int tempTag = tempBlock.get_full_address(); //Contains the full address from BUT
                                    vc_block_under_test = ways.at(way_counter);//move in the victim block
                                    vc_block_under_test.update_tag( tempTag );
                                    vc_block_under_test.make_mru();
                                    victimCache.at(vc_counter) = vc_block_under_test; /** **/

                                    for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                        vc_block_under_test = victimCache.at(
                                                j); //increments j to move through the ways of the set
                                        if (j != vc_counter ) {
                                            vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            victimCache.at(j) = vc_block_under_test; /** **/
                                        }

                                    } //end of for loop

                                    if (nextLevel != nullptr) nextLevel->request('r', address);

                                    block_under_test.initialize_block(localTag, address);
                                    ways.at(way_counter) = block_under_test; /** **/

                                    for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                        block_under_test = ways.at(
                                                j); //increments j to move through the ways of the set
                                        if ( j != way_counter ) {
                                        //          &&(block_under_test.check_lru_counter() < lru_storage)) {
                                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            ways.at(j) = block_under_test; /** **/
                                        }

                                    }

                                } else {

                                    // No invalid block available -- may need to write back

                                    max_lru = assoc - 1;

                                    // Find the LRU Block of L1 cache
                                    for (way_counter = 0; way_counter < assoc; way_counter++) {
                                        block_under_test = ways.at(way_counter);


                                        //Check to see if the block in question is actually the LRU
                                        if (block_under_test.check_valid() &&
                                            (block_under_test.check_lru_counter() == max_lru))
                                            break;

                                    }

                                vc_lru_max = numVC - 1;
                                // Find the LRU Block of VC
                                for (vc_counter = 0; vc_counter < numVC; vc_counter++) {
                                    vc_block_under_test = victimCache.at(vc_counter);

                                    //Check to see if the block in question is actually the LRU
                                    if (vc_block_under_test.check_valid() &&
                                        (vc_block_under_test.check_lru_counter() == vc_lru_max))
                                        break;

                                } // end of VC LRU for loop

                                // VC LRU found, evict block (write back if necessary and install block)

                                if (vc_block_under_test.check_dirty()) {  //dirty block needs writeback in LRU of VC
                                    write_backs++;

                                    if (nextLevel != nullptr) { //VC block is dirty, next level available

                                        nextLevel->request('w', vc_block_under_test.get_full_address());

                                    }

                                    vc_lru_storage = vc_block_under_test.check_lru_counter();
                                    lru_storage = block_under_test.check_lru_counter();

                                    block tempBlock = ways.at(way_counter);
                                    unsigned long int tempTag = tempBlock.get_full_address(); //Contains the full address from BUT
                                    vc_block_under_test = ways.at(way_counter); //move in the victim block
                                    vc_block_under_test.update_tag( tempTag );
                                    vc_block_under_test.make_mru();
                                    victimCache.at(vc_counter) = vc_block_under_test; /** **/

                                    for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                        vc_block_under_test = victimCache.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != vc_counter) &&
                                            (vc_block_under_test.check_lru_counter() < vc_lru_storage)) {
                                            vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            victimCache.at(j) = vc_block_under_test; /** **/
                                        }

                                    } //end of for loop

                                    if( nextLevel != nullptr ) nextLevel->request('r', address);

                                    block_under_test.initialize_block(localTag, address);
                                    ways.at(way_counter) = block_under_test; /** **/  //-- Install new block post writeback

                                    for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                        block_under_test = ways.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != way_counter) &&
                                            (block_under_test.check_lru_counter() < lru_storage)) {
                                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            ways.at(j) = block_under_test; /** **/
                                        }

                                    }

                                } else { //VC LRU Block is not dirty and can be replaced by new victim

                                    lru_storage = block_under_test.check_lru_counter();
                                    vc_lru_storage = vc_block_under_test.check_lru_counter();

                                    block tempBlock = ways.at(way_counter);
                                    unsigned long int tempTag = tempBlock.get_full_address(); //Contains the full address from BUT
                                    vc_block_under_test = ways.at(way_counter); //move in the victim block
                                    vc_block_under_test.update_tag ( tempTag ); //Give VC the full tag
                                    vc_block_under_test.make_mru();
                                    victimCache.at(vc_counter) = vc_block_under_test; /** **/ //Save out changes

                                    for (unsigned int j = 0; j < numVC; j++) { //Updates LRU for Victim cache

                                        vc_block_under_test = victimCache.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != vc_counter) &&
                                            (vc_block_under_test.check_lru_counter() < vc_lru_storage)) {
                                            vc_block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            victimCache.at(j) = vc_block_under_test; /** **/
                                        }

                                    } //end of for loop

                                    if (nextLevel != nullptr) nextLevel->request('r', address);


                                    block_under_test.initialize_block(localTag, address);
                                    ways.at(way_counter) = block_under_test; /** **/

                                    for (unsigned int j = 0; j < assoc; j++) { //Updates LRU for L1 cache

                                        block_under_test = ways.at(
                                                j); //increments j to move through the ways of the set
                                        if ((j != way_counter) &&
                                            (block_under_test.check_lru_counter() < lru_storage)) {
                                            block_under_test.update_lru(); //This adds 1 to every address that has a lower lru value than the original block.
                                            ways.at(j) = block_under_test; /** **/
                                        }

                                    }


                                } //end of VC Write Miss where VC LRU Must be evicted

                            } //test

                            }


                        } else { //Consider adding else statement to keep VC separate
                        max_lru = assoc - 1;
                        // Find the LRU Block
                        for (way_counter = 0; way_counter < assoc; way_counter++) {
                            block_under_test = ways.at(way_counter);

                            //Check to see if the block in question is actually the LRU
                            if (block_under_test.check_valid() &&
                                (block_under_test.check_lru_counter() == max_lru))
                                break;

                        }

                        /*** Dirty bit is set on LRU block ***/

                        if (block_under_test.check_dirty()) {


                            if (nextLevel == nullptr) { //In this case the cache is the lowest in the memory hierarchy.
                                block_under_test = ways.at(way_counter);
                                write_backs++; // Increment writeback statistics
                                block_under_test.initialize_block(localTag,
                                                                  address); //At this point the block should now be updated with the new tag & metadata reset
                                ways.at(way_counter) = block_under_test; /** **/

                                // Must update LRU of remaining blocks
                                for (unsigned int j = 0; j < assoc; j++) {
                                    block_under_test = ways.at(j);
                                    if ((j != way_counter)) {
                                        block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                        ways.at(j) = block_under_test; /** **/
                                    }
                                }
                            } else {  // In this case the cache in question would be L1 or its victim cache

                                block_under_test = ways.at(way_counter);
                                nextLevel->request('w',
                                                   block_under_test.get_full_address()); // Issues a write request to the next level for the evicted block
                                write_backs++;
                                nextLevel->request('r', address);
                                block_under_test.initialize_block(localTag,
                                                                  address); //At this point the block should now be updated with the new tag & metadata reset
                                ways.at(way_counter) = block_under_test; /** **/

                                // Must update LRU of remaining blocks
                                for (unsigned int j = 0; j < assoc; j++) {
                                    block_under_test = ways.at(j);
                                    if ((j != way_counter)) {
                                        block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                        ways.at(j) = block_under_test; /** **/
                                    }
                                }

                            }

                        } //End of dirty LRU block code

                            /*** LRU Block is clean **/
                        else {
                            if (nextLevel == nullptr) { //In this case the cache is the lowest in the memory hierarchy.
                                block_under_test = ways.at(way_counter);
                                block_under_test.initialize_block(localTag,
                                                                  address); //At this point the block should now be updated with the new tag & metadata reset
                                ways.at(way_counter) = block_under_test; /** **/

                                // Must update LRU of remaining blocks
                                for (unsigned int j = 0; j < assoc; j++) {
                                    block_under_test = ways.at(j);
                                    if ((j != way_counter)) {
                                        block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                        ways.at(j) = block_under_test; /** **/
                                    }
                                }
                            } else {  // In this case the cache in question would be L1 or its victim cache

                                block_under_test = ways.at(way_counter);
                                nextLevel->request('r', address);
                                block_under_test.initialize_block(localTag,
                                                                  address); //At this point the block should now be updated with the new tag & metadata reset
                                ways.at(way_counter) = block_under_test; /** **/

                                // Must update LRU of remaining blocks
                                for (unsigned int j = 0; j < assoc; j++) {
                                    block_under_test = ways.at(j);
                                    if ((j != way_counter)) {
                                        block_under_test.update_lru(); //This adds 1 to every lru_counter for valid blocks already in the set.
                                        ways.at(j) = block_under_test; /** **/
                                    }
                                }

                            } // End of L1 / VC read from next level for LRU Replacement

                        }

                    }     //adding a bracket here for VC overarching conditional

                    }
                } // NEW 1.15am 9.23
                /**** End of READ MISS ****/
                sets.at(index_result) = ways;
                return;

            default:
                std::cout << "Case statement error -- please fix me" << std::endl;

        }



                /*************** END OF READ AND WRITE CASES *****************************************/

                // Write Hit & Miss, Read Hit & Miss covered above



    };  // end of request member function


   // Section of helper functions to return the values of private variables to the

   unsigned int get_reads() { return reads; };
   unsigned int get_read_misses() { return read_misses; };
   unsigned int get_writes() { return writes; };
   unsigned int get_write_misses() { return write_misses; };
   unsigned int get_write_backs() { return write_backs; };
   unsigned int get_swaps() { return swaps; };
   unsigned int get_swap_requests() { return swap_requests; };

    // End of statistics section

    void print_cache() {

        unsigned int set_counter;
        unsigned int max_lru_counter = assoc - 1;
        unsigned int current_lru_counter;

        for( set_counter = 0; set_counter < numSets; set_counter++ ) {

            ways = sets.at( set_counter ); // Provides the current 'set' to be printed
            //printf( "  " );

            printf("  set   %d:   ", set_counter);

            for( current_lru_counter = 0; current_lru_counter <= max_lru_counter; current_lru_counter++ ) {

                find_this_mru(current_lru_counter, ways);
                if( print_valid ) {
                    printf("%lx", find_this_mru(current_lru_counter, ways)); //prints current tag
                    if (print_dirty_true) {
                        printf(" D"); //Adds two spaces and prints "D" if it is dirty
                    }

                    printf("  "); //Adds two spaces after each tag & dirty bit combo
                }
            }

            printf( "\n" ); //Sets the program to print to the next line for the next set

        }

    };

    void print_vc() {

        unsigned int max_lru_counter = numVC - 1;
        unsigned int current_lru_counter;

            printf("  set   %d:   ", 0);

            for( current_lru_counter = 0; current_lru_counter <= max_lru_counter; current_lru_counter++ ) {

                printf( "%lx", vc_find_this_mru( current_lru_counter, victimCache) ); //prints current tag
                if( print_valid ) {
                    if (print_dirty_true) {
                        printf(" D"); //Adds two spaces and prints "D" if it is dirty
                    }

                    printf("  "); //Adds two spaces after each tag & dirty bit combo
                }
            }

            printf( "\n" ); //Sets the program to print to the next line for the next set



    };

    unsigned long int vc_find_this_mru( unsigned int current_mru, std::vector<block> mru_ways ) {
        unsigned int way_counter;
        block block_under_test;
        block tempBlock;

            for (way_counter = 0; way_counter < numVC; way_counter++) {

                block_under_test = mru_ways.at(way_counter);
                if (block_under_test.check_lru_counter() == current_mru) break;
            } //end of for loop
            if (block_under_test.check_dirty()) print_dirty_true = 1;
            else print_dirty_true = 0;
            if (block_under_test.check_valid()) print_valid = 1;
            else print_valid = 0;
            return block_under_test.get_tag();

    };


    unsigned long int find_this_mru( unsigned int current_mru, std::vector<block> mru_ways ) {
        unsigned int way_counter;
        block block_under_test;

            for (way_counter = 0; way_counter < assoc; way_counter++) {

                block_under_test = mru_ways.at(way_counter);
                if (block_under_test.check_lru_counter() == current_mru) break;
            }

            if (block_under_test.check_dirty()) print_dirty_true = 1;
            else print_dirty_true = 0;
            if (block_under_test.check_valid()) print_valid = 1;
            else print_valid = 0;

            return block_under_test.get_tag();

    };






};






#endif
