#include "cachelab.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
/*
s = ?
E = ?
b = ?

S = 2^s
B = 2^b
*/

// get tag from address
inline unsigned addrGetTag(unsigned addr,unsigned s,unsigned b){
    return addr >> (s + b);
}
// get index of set from address
inline unsigned addrGetSet(unsigned addr,unsigned s,unsigned b){
    return (addr >> b) & ((1 << s) - 1);
}
// get index of block from address
inline unsigned addrGetBlock(unsigned addr,unsigned s,unsigned b){
    return addr & ((1 << b) - 1);
}

typedef struct cache_line{
    unsigned valid;
    unsigned tag;
    unsigned lruCounter;
}cache_line;


// valid about function
void enableValid(cache_line* it){
    it->valid = 1; // 1 mean it is valid
}
void disableValid(cache_line* it){
    it->valid = 0; // 0 mean it is not valid
}
unsigned isValid(const cache_line* it){
    return it->valid;
}
unsigned getTag(const cache_line* it){
    return it->tag;
}
unsigned getLRU(const cache_line* it){
    return it->lruCounter;
}
void setTag(cache_line* it,unsigned tag){
    it->tag = tag;
}
void zeroLRU(cache_line* it){
    it->lruCounter = 0;
}
void incLRU(cache_line* it){
    it->lruCounter++;
}

void initCache(cache_line** cache, unsigned S,unsigned E)
{
    unsigned set,e;
    for(set = 0;set<S;set++)
        for(e = 0;e<E;e++)
        {
            disableValid(&cache[set][e]);
            cache[set][e].tag = 0;
            cache[set][e].lruCounter = 0;
        }
}

// find a cache_line
// compare it is the cache_line we need or not
// return 0 if not find, else return 1
unsigned cacheCanFind(const cache_line** cache,unsigned E,unsigned tag, unsigned set)
{
    unsigned e;
    unsigned found = 0;
    for(e = 0;e < E;e++) // find it in E lines
    {
        if(isValid(&cache[set][e]) && cache[set][e].tag == tag) // valid block and tag is equal
        {
            found = 1; // yah, find it
            break; // break loop
        }
    }
    return found;
}

// find a empty line in cahce[set]
cache_line* cacheFindEmptyLine(const cache_line** cache,unsigned E,unsigned set)
{
    cache_line* emptyLine = NULL;
    unsigned e;
    for(e = 0;e < E;e++) // find it in E lines
    {
        if(!isValid(&cache[set][e])) // not valid mean it has not been used
        {
            emptyLine = &cache[set][e];
            break;
        }
    }
    return emptyLine;
}

// suppose there are no unvalid line in cache[set]
// return the lowest lruCounter in cache[set]
cache_line* cacheFindMiniumLRU(const cache_line** cache,unsigned E,unsigned set)
{
    cache_line* minimumLRU = &cache[set][0];
    unsigned lruCurrent = minimumLRU->lruCounter;
    unsigned e,lrutemp;
    for(e = 1;e < E;e++) // find it in E lines
    {
        lrutemp = getLRU(&cache[set][e]);
        if(isValid(&cache[set][e]) && lrutemp < lruCurrent) // occur lower lruCounter
        {
            minimumLRU = &cache[set][e];
            lruCurrent = lrutemp;
        }
    }
    return minimumLRU;
}


// enum memoryStatus {miss = 0, hit = 1, eviction}; // dataType for mark
void load(const cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
    unsigned tag = addrGetTag(addr,s,b);
    unsigned set = addrGetSet(addr,s,b);
    unsigned block = addrGetBlock(addr,s,b);
    printf("tag:0x%x",tag);
    printf("set:0x%x",set);
    printf("block:0x%x",block);
    

    unsigned hit = cacheCanFind(cache,E,tag,set);

    if(hit)
    {
        // hit here
        printf("hit\n");
    }
    else
    {
        printf("miss\n");
        cache_line* emptyLine = cacheFindEmptyLine(cache,E,set); // find an empty line to save data for load
        if(emptyLine) // yah, there is an empty line  
        {
            enableValid(emptyLine);
            setTag(emptyLine,tag);
        }
        else // cannot find an empty line
        {
            // we must drop some block
            // use LRU policy
            printf("eviction\n");
            cache_line* minimumLRU = cacheFindMiniumLRU(cache,E,set);
            setTag(minimumLRU,tag);
            zeroLRU(minimumLRU); // clear LRU
            incLRU(minimumLRU);
        }
    }
}

void store(cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
   load(cache,addr,s,E,b);
}

void modify(cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
    load(cache,addr,s,E,b);
    store(cache,addr,s,E,b); // actually it can be hit here
}

cache_line** cache;

int main(unsigned argc,char **argv)
{
    unsigned opt = 0,
        s = 0,
        E = 0,
        b = 0;
    while(-1 != (opt = getopt(argc,argv,"s:E:b:")))
    {
        switch(opt)
        {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            default:
                break;
        }
    }
    printf("%d\n",sizeof(cache_line));
    printf("s:%d\nE:%d\nb:%d\n",s,E,b);
    
    s = 4;
    E = 1;
    b = 4;
    cache = (cache_line** ) malloc((1 << s) * E * sizeof(cache_line)* (1 << b)); // get the memory
    printf("cache size:0x%x",sizeof(cache));
    initCache(cache,1<<s,E); // initial cache information
    load(cache,0x10,s,E,b);
    modify(cache,0x20,s,E,b);
    load(cache,0x22,s,E,b);
    store(cache,0x18,s,E,b);
    load(cache,0x110,s,E,b);
    load(cache,0x210,s,E,b);
    modify(cache,0x12,s,E,b);
    // enum memoryStatus a = hit;
    // printf("%d\n",a);
    // unsigned arr[3];
    // unsigned addr = 0x456210;
    // prunsignedf("%ld %ld %ld\n",arr[0],arr[1],arr[2]);
    // prunsignedf("0x%x 0x%x 0x%x\n",addrGet_t(addr,s,b),addrGet_s(addr,s,b),addrGet_b(addr,s,b));
    
    // prunsignedSummary(0, 0, 0);
    free(cache);
    return 0;
}