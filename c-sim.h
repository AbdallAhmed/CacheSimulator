#ifndef csim_h
#define csim_h

//the struc represents what a way looks like in a cache
//dirty bit only used if it is a write back policy
typedef struct way
{
    long int tag;
    int valid;
    int dirty;
}be_the_way;


long int getSetIndex(long int val);
long int getTag(long int val);
void eviction(long int t, long int s, int w);
void LRUMove(long int set, long int index);
void readme(long int val, int ifWriting);
void writeme(long int val);

#endif /* csim_h */