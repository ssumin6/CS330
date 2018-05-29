#include "filesys/cache.h"
#include <list.h>
#include <string.h>

struct list cache;

void 
cache_init()
{

	list_init(&cache);
}


void 
cache_read(struct block *block, block_sector_t sector, void * buffer)
{
	struct list_elem *e;
	for(e = list_begin(&cache); e!=list_end(&cache); e = list_next(e))
	{	
		struct cache_elem *cache = list_entry(e, struct cache_elem, elem);
		if (sector == cache->sector)
		{
			memcpy(buffer, cache->data, 512);
			return;
		}
	}

	cache_read_from_disk(block, sector, buffer);
}

void 
cache_write (struct block *block, block_sector_t sector, void * buffer)
{
	struct list_elem *e;
	for(e = list_begin(&cache); e!=list_end(&cache); e = list_next(e))
	{	
		struct cache_elem *cache = list_entry(e, struct cache_elem, elem);
		if (sector == cache->sector)
		{
			memcpy(cache->data, buffer, 512);
			cache->dirty = 1;
			return;
		}
	}

	block_write (block, sector, buffer);
}


void 
cache_destroy(struct block *block)
{

	int i;
	unsigned size = list_size(&cache);
	struct cache_elem * cache_unit;

	for(i=size; i>0; i--){
		cache_unit = list_entry(list_pop_front(&cache),struct cache_elem, elem);
		if(cache_unit->dirty ==1){
			cache_write_to_disk(cache_unit, block);
		}
	}
}

void 
cache_read_from_disk (struct block *block, block_sector_t sector, void * buffer_)
{
	
	uint8_t *buffer = buffer_;
  	struct cache_elem * cache_unit;

	cache_unit = malloc(sizeof(struct cache_elem));
	if(list_size(&cache) == 64){
		cache_evict(block);
	}
	/*
	block read, and append data for cache_elem;
	*/
	block_read(block, sector, buffer);
	memcpy(cache_unit->data, buffer, 512);
	cache_unit->dirty = 0;
	cache_unit->sector = sector;
	list_push_back(&cache, cache_unit);
}

/*
use for evict and cache_destroy
*/

void 
cache_write_to_disk (struct cache_elem * cache_unit, struct block *block)
{
	block_write(block, cache_unit->sector,cache_unit->data);
}

void 
cache_evict (struct block *block)
{
	//FIFO
	struct list_elem *elem = list_pop_front(&cache);
	struct cache_elem *cache_unit = list_entry(elem, struct cache_elem, elem);
	if (cache_unit == NULL)
		PANIC("CACHE NOT EXIST\n");

	if (cache_unit->dirty)
		//PANIC("start \n");
		cache_write_to_disk (cache_unit, block);
		//PANIC("END \n");		
	free(cache_unit);
}