#include "cachelab.h"
#include<unistd.h>
#include<getopt.h>
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
unsigned addrGetTag(unsigned addr,unsigned s,unsigned b){
    return addr >> (s + b);
}
// get index of set from address
unsigned addrGetSet(unsigned addr,unsigned s,unsigned b){
    return (addr >> b) & ((1 << s) - 1);
}
// get index of block from address
unsigned addrGetBlock(unsigned addr,unsigned s,unsigned b){
    return addr & ((1 << b) - 1);
}

typedef struct cache_line{
    unsigned valid;
    unsigned tag;
    unsigned lruCounter;
}cache_line;

enum memoryStatus {miss, hit, eviction, missAndEviction}; // dataType for mark


// valid about function
void enableValid(cache_line* it){
    it->valid = 1; // 1 mean it is valid
}
void disableValid(cache_line* it){
    it->valid = 0; // 0 mean it is not valid
}
unsigned isValid(cache_line* it){
    return it->valid;
}
unsigned getTag(cache_line* it){
    return it->tag;
}
unsigned getLRU(cache_line* it){
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

// get the memory from system
cache_line** getCacheMem(unsigned S, unsigned E)
{
    cache_line** sets;
    sets = (cache_line**) malloc(S * sizeof(cache_line*)); // every set's address(about S sets)
    int i;
    for(i=0;i<S;i++) // for every set
    {
        // give E lines to every set
        sets[i] = (cache_line*) malloc(E * sizeof(cache_line));
    }
    return sets;
}

void initCache(cache_line** cache, unsigned S,unsigned E)
{
    unsigned set,e;
    // printf("S:%u\nE:%u\n",S,E);
    // printf("init1\n");
    for(set = 0;set < S; set++)
    {
        for(e = 0;e < E; e++)
        {
            // printf("set:%u e:%u\n",set,e);
            disableValid(&cache[set][e]);
            // cache[set][e].tag = 0;
            setTag(&cache[set][e],0);
            // cache[set][e].lruCounter = 0;
            zeroLRU(&cache[set][e]);
            // printf("set:%u e:%u ------------\n",set,e);
        }
    }
    // printf("init2\n");    
}

// find a cache_line
// compare it is the cache_line we need or not
// return cache_line's address if find it else return NULL
cache_line* cacheCanFind(cache_line** cache,unsigned E,unsigned tag, unsigned set)
{
    cache_line* found = NULL;
    unsigned e;
    for(e = 0;e < E;e++) // find it in E lines
    {
        if(isValid(&cache[set][e]) && cache[set][e].tag == tag) // valid block and tag is equal
        {
            found = &cache[set][e]; // yah, find it
            break; // break loop
        }
    }
    return found;
}

// find a empty line in cahce[set]
cache_line* cacheFindEmptyLine(cache_line** cache,unsigned E,unsigned set)
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
cache_line* cacheFindMiniumLRU(cache_line** cache,unsigned E,unsigned set)
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


// hit or miss
enum memoryStatus cacheCanHit(cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
    unsigned tag = addrGetTag(addr,s,b);
    unsigned set = addrGetSet(addr,s,b);
    // unsigned block = addrGetBlock(addr,s,b);
    cache_line* found = cacheCanFind(cache,E,tag,set);
    return found?hit:miss;
}

enum memoryStatus load(cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
    unsigned tag = addrGetTag(addr,s,b);
    unsigned set = addrGetSet(addr,s,b);
    // unsigned block = addrGetBlock(addr,s,b);
    // printf("tag:0x%x set:0x%x block:0x%x\n",tag,set,block);

    enum memoryStatus loadStatus;
    
    if(cacheCanHit(cache,addr,s,E,b) == hit)
    {
        // hit here
        loadStatus = hit;
        // printf("hit\n");
    }
    else
    {
        // printf("miss ");
        cache_line* emptyLine = cacheFindEmptyLine(cache,E,set); // find an empty line to save data for load
        if(emptyLine) // yah, there is an empty line  
        {
            loadStatus = miss;
            enableValid(emptyLine);
            setTag(emptyLine,tag);
        }
        else // cannot find an empty line
        {
            // we must drop some block
            // use LRU policy
            loadStatus = missAndEviction;
            // printf("eviction");
            cache_line* minimumLRU = cacheFindMiniumLRU(cache,E,set);
            setTag(minimumLRU,tag);
            zeroLRU(minimumLRU); // clear LRU
            incLRU(minimumLRU);
        }
        // printf("\n");
    }
    return loadStatus;
}

// just like load, get memory once only
enum memoryStatus store(cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
   return load(cache,addr,s,E,b);
}

enum memoryStatus modify(cache_line** cache,unsigned addr,unsigned s, unsigned E,unsigned b)
{
    return load(cache,addr,s,E,b);
    // whenever the status is, next step for modify is hit!!!
}

cache_line** cache;

int main(int argc,char **argv)
{
    unsigned opt = 0,
        s = 0,
        E = 0,
        b = 0;

    FILE* file = NULL;
    char* fileName = NULL;
    char identifier;
    unsigned address;
    unsigned size;

    int hits = 0;
    int misses = 0;
    int evictions = 0;

    int printStatus = 0;
    while(-1 != (opt = getopt(argc,argv,"vs:E:b:t:")))
    {
        switch(opt)
        {
            case 'v':
                printStatus = 1; // print hit, miss or eviction
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't': // file
                fileName = optarg;
            default:
                break;
        }
    }
    // if(!file)
    // {
    //     printf("Give the file name!\n");
    // }
    // else
    // {
    //     printf(optarg);
    // }
    if(fileName) // if there is file
    {
        cache = getCacheMem(1<<s,E); // get memory
        initCache(cache,1<<s,E); // initial cache information
        file = fopen(fileName,"r");
        enum memoryStatus status = miss; // status mark
        while(fscanf(file," %c %x,%u",&identifier,&address,&size)>0)
        {
            if(printStatus)
                printf("%c %x,%u ",identifier,address,size);
            switch(identifier)
            {
                case 'I': // ignore with prefix I
                    continue;
                    break;
                case 'L'://load
                    status = load(cache,address,s,E,b);
                    break;
                case 'S'://store
                    status = store(cache,address,s,E,b);
                    break;
                case 'M':
                    status = modify(cache,address,s,E,b);
                    hits++; // !!!!!!!watch out here
                    break;
                default:
                    break;
            }
            // now judge status
            switch(status)
            {
                case miss:
                    if(printStatus)
                        printf("miss");
                    misses++;
                    break;
                case hit:
                    if(printStatus)
                        printf("hit");
                    hits++;
                    break;
                case eviction:
                    if(printStatus)
                        printf("eviction");
                    evictions++;
                    break;
                case missAndEviction:
                    if(printStatus)
                        printf("miss eviction");               
                    misses++;
                    evictions++;
                    break;
                default:
                    break;
            }
            if(printStatus)
            {
                if(identifier == 'M')
                    printf(" hit\n");
                else
                    printf("\n");
            }
        }
        fclose(file);
    }

    // s = 4;
    // E = 1;
    // b = 4;
    // printf("s:%d\nE:%d\nb:%d\n",s,E,b);
    // printf("here1\n");
    // printf("1 << s : %u\n",1 << s);
    // printf("%lu\n",sizeof(cache_line));
    // printf("1 << b : %u\n",1 << b);    
    // printf("(1 << s) * E * sizeof(cache_line)* (1 << b) -- 0x%lx\n",(1 << s) * E * sizeof(cache_line)* (1 << b));

    // cache = getCacheMem(1<<s,E);

    // // printf("cache size:0x%lx\n",sizeof(cache));
    // initCache(cache,1<<s,E); // initial cache information
    
    // load(cache,0x10,s,E,b);
    // modify(cache,0x20,s,E,b);
    // load(cache,0x22,s,E,b);
    // store(cache,0x18,s,E,b);
    // load(cache,0x110,s,E,b);
    // load(cache,0x210,s,E,b);
    // modify(cache,0x12,s,E,b);
    // // enum memoryStatus a = hit;
    // // printf("%d\n",a);
    // // unsigned arr[3];
    // // unsigned addr = 0x456210;
    // // prunsignedf("%ld %ld %ld\n",arr[0],arr[1],arr[2]);
    // // prunsignedf("0x%x 0x%x 0x%x\n",addrGet_t(addr,s,b),addrGet_s(addr,s,b),addrGet_b(addr,s,b));
    
    // free(cache);
    printSummary(hits, misses, evictions);
    // printf("hits:%u misses:%u evictions:%u\n", hits, misses, evictions);
    return 0;
}

