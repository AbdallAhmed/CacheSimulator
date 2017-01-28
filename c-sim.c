#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "c-sim.h"

//long list of all parameters the program takes in
int REPLACE_POLICY;  // 0 if FIFO 1 if LRU
int WRITE_FORMAT; //0 if wt 1 if wb
int SET_SIZE; // # of ways per set

//need to see when the cache hits or misses
int numOfMisses = 0;
int numOfHits = 0;

//need to know when the program is reading or writing
int numOfWrites = 0;
int numOfReads = 0;
double block_offset;
double set_offset;
double tag_offset;
be_the_way** SETS;

//checks to see if the input text file is empty 
int isEmpty(FILE *file)
{
    long savedOffset = ftell(file);
    fseek(file, 0, SEEK_END);

    if (ftell(file) == 0)
    {
        return 0;
    }

    fseek(file, savedOffset, SEEK_SET);
    return 1;
}

//get a set index from a memory address
long int getSetIndex(long int val)
{
    //first push off the block offset
    val = val >> (int)block_offset;

    //mask off the beginning of the string to be safe
    long int setindex = val &  (int)(pow(2,set_offset) - 1);
    return setindex;
}

//get a tag index from a memory address
long int getTag(long int val)
{
    //first push off the block offset
    val = val >> (int)block_offset;
    //then push of the bits associated with the set index
    val = val >> (int)set_offset;

    //if our tag is less than 32 bits then we mask just for safety but if it is 32 bits
    //we can ignore this setp
    if(tag_offset < 32)
        val = val &  (int)(pow(2,tag_offset) - 1);
    return val;
}

void eviction(long int t, long int s, int w)
{
    int count = 0;

    //if the data that we are about to evict was set as dirty we have a writeback policy
    //we can safely add to our num of writes
    if(SETS[s][0].dirty == 1)
       numOfWrites++;

    //if(WRITE_FORMAT == 1)
        //numOfWrites++;

   //if we have a direct mapped cache
    if(SET_SIZE == 1)
    {
        SETS[s][0].tag = t;
        SETS[s][0].valid = 1;

        //if we are writing data we need to make sure the new data we are pulling in is
        //immediately "dirty" if we have a write back policy (already written to)
        if(w == 1 && WRITE_FORMAT == 1)
            SETS[s][0].dirty = 1;
        else SETS[s][0].dirty = 0;
    }
    
    else
    {
        //move move all the data "up" one spot
        //aka arr[1] = arr[2] etc
        do{
            SETS[s][count] = SETS[s][count + 1];
            count++;
        }while(count < SET_SIZE - 1);

        //once we are done looping we are at the last spot
        //insert new data here
        SETS[s][count].tag = t;
        SETS[s][count].valid = 1;

        //if we are writing data we need to make sure the new data we are pulling in is
        //immediately "dirty" if we have a write back policy (already written to)
       if(w == 1 && WRITE_FORMAT == 1)
            SETS[s][count].dirty = 1;
        else SETS[s][count].dirty = 0;
    }
}

void LRUMove(long int set, long int index)
{
    int count = index;

    //hold the value of the item that we must move to the botom
    be_the_way hold = SETS[set][index];

    //if the data we hit on was already at the bottom of our set we can end
    if(index == SET_SIZE - 1)
    {
        return;
    }
    else
    {
        //loops over the remaining and shifts all data up by one spot
        //this is to say that index 3 goes into index 2 (etc)
        //also keeps track of making sure not to step out of the "valid zone"
        //that is to say if we have a set size of 10 but only have 5 elements it will put the
        //data at index 5 and not all the way down to 10
        while((count < SET_SIZE -1) && (SETS[set][count+1].valid!=0)){
            SETS[set][count] = SETS[set][count + 1];
            count++;
        }

        //flip the last element that is valid with the LRU hit value
        SETS[set][count] = hold;
    }
}

void readme(long int val, int ifWriting)
{
    //printf("%s\n", "made it inside of function readme");
    //get the set index we are working with
    long int whatSet = getSetIndex(val);

    //get the tag we are working wtih
    long int tagVal = getTag(val);
    int count = 0;
    int hasInserted = 0;
    int hasHit = 0;

    do{

        //check to see if the index we are at is valid (has something inserted) 
        //if it has nothing then we insert our value into the sub array
        if ((SETS[whatSet][count].valid == 0) && (hasInserted == 0))
        {
            SETS[whatSet][count].valid = 1;
            SETS[whatSet][count].tag = tagVal;

            //if we have a write back policy we need to keep track of what values 
            //we have written to. So if we have gotten to this function via the write function
            //we set our dirty bit to 1
            if(ifWriting == 1 && WRITE_FORMAT == 1)
                SETS[whatSet][count].dirty = 1;
            hasInserted = 1;
            break;
        }

        else
        {
            //first see if the index we are currently on is valid
            if(SETS[whatSet][count].valid == 1)
            {
                //if the value is already there 
                if(SETS[whatSet][count].tag == tagVal)
                {
                    //add to our number of hits
                    numOfHits++;
                    hasHit = 1;

                    //if we are writing and we "hit" on data it means that we have written to it
                    //for a write back policy we need keep track of this
                    if(ifWriting == 1 && WRITE_FORMAT == 1)
                        SETS[whatSet][count].dirty = 1;

                    //when dealing with an LRU replacement policy we need to adjust the spots
                    //of our data for when we hit certain kinds of data
                    if(REPLACE_POLICY == 1)
                    {
                        //function to deal with LRU move
                        LRUMove(whatSet,count);
                    }
                    break;
                }

            }

        }
        count++;
    }while(count < SET_SIZE);

    //printf("%s\n", "made it outside of the loop in readme");

    //if we have not inserted or hit data that means our cache is full
    if((hasInserted == 0) && (hasHit == 0))
    {
        //must evict the lowest index of our cache to make space for the new element
        eviction(tagVal, whatSet, ifWriting);

        //if we are evicting that means we have missed
        numOfMisses++;

        //if we are evicting we are reading the new memory address from "memory"
        numOfReads++;
    }

    //case of inserting data
    if(hasInserted == 1)
    {
        //if we had to insert our data that means we did not have it (missed)
        numOfMisses++;

        //and needed to bring it into our cache from "memory" a read
        numOfReads++;
    }
        
}

void writeme(long int val)
{
    //a write is basically just reading the data and inserting if it is not there
    readme(val,1);

    //if we have a write through policy we count every single write
    //write back policy only counts write upon evicition
    if(WRITE_FORMAT == 0)
    {
        numOfWrites++;
    }
}

int main(int argc, char* argv[])
{
	FILE * fpointer;
	int CACHE_SIZE = atoi(argv[1]);
	

	int ASSOC; // 0  if direct mapped; 1 if full associative; 2 if n-way
	
	
    //sets the associativity based off user input
	if(strcmp(argv[2],"direct") == 0)
		ASSOC = 0;
	else if (strcmp(argv[2],"assoc") == 0)
		ASSOC = 1;
	else ASSOC = 2;

    //if it is a direct mapped cashe we know that the set is 1-way associative
	if (ASSOC == 0)
	{
		SET_SIZE = 1;
	}

    //extract the n-way associativity from input args
	if (ASSOC == 2)
	{
		char* hold = &argv[2][6];
		SET_SIZE = atoi(hold);
	}

    //read in the block size from input args
	int BLOCK_SIZE;
	BLOCK_SIZE = atoi(argv[3]);

    //set the replacement policy based off of input args
	if(strcmp(argv[4],"FIFO") == 0)
		REPLACE_POLICY = 0;
	else if (strcmp(argv[4],"LRU") == 0)
			REPLACE_POLICY = 1;

    //set the write format based off of input args
	if(strcmp(argv[5],"wt") == 0)
		WRITE_FORMAT = 0;
	else if (strcmp(argv[5],"wb") == 0)
			WRITE_FORMAT = 1;

    //if the file you want to read does not exist, exit
	if((fpointer = fopen(argv[6],"r")) == NULL)
    {
    	printf("error\n");
    	exit(1);
    }

    //if the file is empty, place error condition and exit
    if(isEmpty(fpointer) == 0)
    {
    	printf("error\n");
    	exit(1);
    }
    int NUM_OF_SETS;

    //determine the number of sets based off the formula:
    //< cachesize > = number of sets × < setsize > × < blocksize >.
    if(ASSOC == 0 || ASSOC == 2)
    {
    	NUM_OF_SETS = CACHE_SIZE / (SET_SIZE * BLOCK_SIZE);
    }

    //if the cache is full associative we need to know how big the one set is
    else
    {
    	NUM_OF_SETS = 1;
    	SET_SIZE = CACHE_SIZE/BLOCK_SIZE;
    }

    //determine the block offset used throughout
    block_offset = log(BLOCK_SIZE)/log(2);

    //determine how many bits are used in the set index
    set_offset = log(NUM_OF_SETS)/log(2);
        
    //based of the last two values and the fact that we know we are working with 32-bit addresses
    //find how many bits are being used on the tag
    tag_offset = 32 - (block_offset + set_offset);
   
    //malloc the correct number of sets based off our calculations
    //we are mallocing for an array of our data type
    SETS = malloc(NUM_OF_SETS*sizeof(be_the_way*));
    int count = 0;

    //for each array allocate space for the array size
    do
    {
    	SETS[count] = malloc(SET_SIZE*sizeof(be_the_way));
    	count++;
    }while(count < NUM_OF_SETS);

    count = 0;
    int count2 = 0;

    do
    {
    	do{
            //initialize values for each and every single array spots
    		SETS[count][count2].tag = 0;
    		SETS[count][count2].valid = 0;
    		SETS[count][count2].dirty = 0;
    		//printf("Sets[%d][%d]: Tag is: %ld, Valid bit at: %d, Dirty is at: %d \n", count, count2, 
    		//	SETS[count][count2].tag, SETS[count][count2].valid, SETS[count][count2].dirty);
    		count2++;
    	}while(count2 < SET_SIZE);
    	count++;
    	count2=0;
    }while(count < NUM_OF_SETS);

    //have this variable bc for the input format assigned the first string did not matter
   	char eat[1024];
    char action;
    long int val;
    //long int lol;
    //long int meme;

    //loops over the entire file 
    while(!feof(fpointer))
    {
    	fscanf(fpointer, "%s %c %lx", eat, &action, &val);

        //last string in the input was different from the rest so can break when we get to it
    	if(strcmp(eat,"#eof") == 0)
    		break;

    	else
    	{
    		if(action == 'W')
    		{
                //if we have a write command go to write function 
                writeme(val);
    		}
    		else if (action == 'R')
    		{
    			//if we have a read command go to read function 
                readme(val,0);
      		}
    	}
    	fscanf(fpointer, "\n");
    	//printf("%c %ld\n", action, val);
        
    	//printf("%s\n", test);
    }

    printf("Memory Reads: %d \n", numOfReads);
    printf("Memory Writes: %d \n", numOfWrites);
    printf("Cache hits: %d \n", numOfHits);
    printf("Cache misses: %d \n", numOfMisses);

    count = 0;
    do{
        free(SETS[count]);
        count++;
    }while(count < NUM_OF_SETS);

    free(SETS);
    fclose(fpointer);
    return 0;
}
