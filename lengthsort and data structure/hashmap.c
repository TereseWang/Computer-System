
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <bsd/string.h>
#include <string.h>
#include "hashmap.h"

int
hash(char* key)
{
    long hashvalue = 0;
    for(long ii = 0; key[ii]; ++ii){
        hashvalue = hashvalue * 97 + key[ii];
    }
    return hashvalue;
}

hashmap*
make_hashmap_presize(int nn)
{
    hashmap* hh = calloc(1, sizeof(hashmap));
    hh->size = 0;
    hh->cap = nn;
    hh->data = calloc(nn, sizeof(hashmap_pair));
    return hh;
}

hashmap*
make_hashmap()
{
    return make_hashmap_presize(4);
}

void
free_hashmap(hashmap* hh)
{
    free(hh->data);
    free(hh);
}

int
hashmap_has(hashmap* hh, char* kk)
{
    return hashmap_get(hh,kk) != -1;
}

int
hashmap_get(hashmap* hh, char* kk)
{
    int hashkey  = hash(kk) % hh->cap;
    for(int i = hashkey; i < hh->cap; ++i) {
        //iterate through the data, if the data is used or tomb, ignore
        if (!hh->data[i].used || hh->data[i].tomb) {
            return -1;
        }
        //if find the key, return the value
        if (strcmp(hh->data[i].key, kk) == 0){
		    return hh->data[i].val;
	    }
    }
}


void 
map_grow(hashmap* hh) 
//realloc the map if size is great than or equal to the half of the capacity
{
    hashmap_pair* data =  hh->data;
    long cap = hh->cap;
    hh->cap *=2;
    hh->data = calloc(hh->cap, sizeof(hashmap_pair));
    hh->size = 0;
    for (int i = 0; i < cap; ++i) {
        if(data[i].used && !data[i].tomb){
            hashmap_put(hh, data[i].key, data[i].val);
        }
    }
    free(data);
}

int
//get the next nonused key 
get_next_key(hashmap* hh, int hashkey) {
    for(int i = hashkey; i < hh->cap; ++i) {
        if(!hh->data[i].used && !hh->data[i].tomb) {
            return i;
		}
	}
}

void
hashmap_put(hashmap* hh, char* kk, int vv)
{
    //if map does not contains the input key 
    if(!hashmap_has(hh,kk)){
        if (hh->size * 2 >= hh->cap){
			map_grow(hh);
		}
		int hashkey = hash(kk) % hh->cap;
        //get available key to use
		hashkey = get_next_key(hh, hashkey);
        //insert the value and key pair 
		strcpy(hh->data[hashkey].key, kk);
		hh->data[hashkey].val = vv;
		hh->data[hashkey].used = true;
		hh->data[hashkey].tomb = false;
		hh->size = hh->size + 1;
	} 
    //if map contains the input key
	else {
		int hashkey = hash(kk) % hh->cap;
        //change the value only
		for (int i = hashkey; i < hh->cap; ++i) {
            if (strcmp(hh->data[i].key, kk)== 0) {
                hh->data[i].val = vv;
				break;		
			}
		}
	}
}

void
hashmap_del(hashmap* hh, char* kk)
{
    int hashkey = hash(kk) % hh->cap;
    for(int i = hashkey; i < hh->cap; ++i) {
        if (strcmp(hh->data[i].key, kk) == 0) {
            hh->data[i].tomb = true;
        }
    }
}

hashmap_pair
hashmap_get_pair(hashmap* hh, int ii)
{
    return hh->data[ii];
}


void
hashmap_dump(hashmap* hh)
{
    printf("== hashmap dump ==\n");
    for (int i= 0; i < hh->cap; ++i) {
        //only print the pair that is used and not deleted
	    if(hh->data[i].used && !hh->data[i].tomb) {
		    printf("key: %s, value:%d \n", hh->data[i].key, hh->data[i].val);
		}
	}
}
