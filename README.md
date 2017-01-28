# CacheSimulator
Simulates Cache behavior for different Associativities, Replacement Policies, and Write Policies  

The different options for this program are as follows  
**Associativity**: Direct-mapped; n-way associative; fully associative  
**Replacement Policies**: LRU or FIFO  
**Write Policies**: Write Through or Write Back  

The program is heavily input driven. The command line argument should be as follows:  
./c-sim \<cache size\> \<associativity\> \<block size\> \<replacement_policy> \<write_policy> \<trace file>  

1. Cache Size
  1. Should be a power of 2
  2. < cachesize > = number of sets × < setsize > × < blocksize >
2. Associativity
  1. **direct** - simulates a direct mapped cache
  2. **assoc** - simulates a fully associative cache
  3. **assoc:n** - simulates a n-way associative cache
3. Block Size
  1. Tells you the size of the cache block (power of 2)
4. Replace Policy
  1. **FIFO** - First in first out
  2. **LRU** - Least recently used
5. Write Policy
  1. **wt** - Write through
  2. **wb** - Wrtie back
6. File
  1. File the user wants the simulation to run on 

The file format is as follows:
\<Mem address>: \<W or R> \<Mem address>  

For the purposes of this program the first memory address is ignored  
The letter 'W' or 'R' tells us wheter to Write or to Read to our cache  
The second memory address is the one we "send" to our cache  

Program counts number of:
  1. *Memory Reads*
  2. *Memory Writes*
  3. *Cache Hits*
  4. *Cache Misses*  

Note that a "read" is to simulate when the cache must go to "main memory" to get the data it missed on
