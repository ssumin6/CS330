#include <debug.h>
#include <stdint.h>
#include <stdbool.h>
#include <debug.h>
#include <inttypes.h>
#include "vm/page.h"
#include "lib/kernel/hash.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/synch.h"

// struct lock hash_lock;

bool 
spage_less_func	(const struct hash_elem *a,
                 const struct hash_elem *b, void *aux)
{
	struct spage_entry *pa = hash_entry(a, struct spage_entry, elem); 	
	struct spage_entry *pb = hash_entry(b, struct spage_entry, elem); 	

	if (pa->va < pb->va)
		return true;
	return false;
}

unsigned 
hash_map (const struct hash_elem *e, void *aux)
{
	const struct spage_entry *p = hash_entry(e, struct spage_entry, elem); ////
	return hash_int(p->va>>12);
}

void 
destory_hash_action(struct hash_elem *e, void *aux)
{
	struct spage_entry *spage_entry = hash_entry(e, struct spage_entry, elem);

	switch (spage_entry->page_type){
		case PHYS_MEMORY:
		{
			frame_remove (spage_entry);
			free(spage_entry);
			break;
		}
		case SWAP_DISK:
		{
			swap_remove(spage_entry);
			free(spage_entry);
			break;
		}
		case MMAP:
		{
			break;
		}

	}
}

void 
spage_init(struct hash *page_table)
{
	hash_init(page_table,hash_map,spage_less_func, NULL);
	// lock_init(&hash_lock);
}

bool 
allocate_spage_elem (unsigned va, enum spage_type flag, void * entry)
{
	struct hash *page_table = &thread_current()->supplement_page_table;
	struct spage_entry *fe = malloc(sizeof(struct spage_entry));
	fe->va = va;
	fe->pointer = entry;
	fe->page_type = flag;
	// lock_acquire(&hash_lock);
	ASSERT(hash_insert(page_table, &fe->elem) == NULL);
	// lock_release(&hash_lock);
}

bool 
deallocate_spage_elem (unsigned va)
{
	struct spage_entry f;
	struct hash_elem *e;
	struct hash *page_table = &thread_current()->supplement_page_table;
	f.va = va; 
	e = hash_find(page_table, &f.elem);
	if (e != NULL)
		{
			// lock_acquire(&hash_lock);
			hash_delete(page_table, &e);
			// lock_release(&hash_lock);
			free(&f);
			return true;
		}
	return false;
}

struct spage_entry *
mapped_entry (struct thread *t, unsigned va){
	struct spage_entry page_entry;	
	page_entry.va = va;
	struct hash_elem *hash = hash_find(&t->supplement_page_table, &page_entry.elem);


	if(hash == NULL){
	 	return NULL;
	}

	return hash_entry(hash , struct spage_entry, elem);
}

void 
destroy_spage (struct hash *page_table)
{
  hash_destroy(page_table, destory_hash_action);
}