/*  This file is part of simple kernel.
 Project NanoKernel (for study purposes only)
 Copyright (C) 2013  Sirius (Vdov Nikita)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#ifdef NULL
#undef NULL
#endif

#include <std/membase.h>
#include <std/print.h>
#include <mem.h>
#include <mempool.h>

static MemPool_t memPool;

bool
k_heap_init()
{
	int i;
	k_memset(&memPool, 0, sizeof(MemPool_t));

	memPool.phisicalMemBeginPtr = memPool.phisicalMemPtr = 
		malloc(KERNEL_HEAP_MIN_SIZE);
	memPool.phisicalMemPtrMax = memPool.phisicalMemBeginPtr + 
		KERNEL_HEAP_MIN_SIZE;
	/* if not found.. mean low memory - panic */
	if(memPool.phisicalMemBeginPtr == NULL)
		return false;

	return true;
}

ptr_t
k_malloc(size_t bytes)
{
	return k_malloc_aligned(bytes, 1);
}

void
k_free(ptr_t block)
{
	byte dividerSize = *(((byte *)block)-1);
	byte *rawBlock = ((byte *)block)-dividerSize-1;
	k_pool_release(&memPool, rawBlock);
}

ptr_t
k_malloc_aligned(size_t bytes, byte align)
{
	if(align == 0)
		return NULL;
	
	byte *rawMem = (byte *)k_pool_select(&memPool, bytes+align);
	if(!rawMem)
		return NULL;
	
	/* normalize raw memory block */
	byte dividerSize = align - (((int)rawMem+1) % align);
	rawMem += dividerSize;
	*((byte *)rawMem) = dividerSize;
	return rawMem+1;
}

ptr_t
k_realloc(ptr_t block, size_t bytes)
{
	return k_realloc_aligned(block, bytes, 1);
}

ptr_t
k_realloc_aligned(ptr_t block, size_t bytes, byte align)
{
	if(align == 0)
		return NULL;
	
	byte dividerOldSize = *(((byte *)block)-1);
	byte *rawOldBlock = ((byte *)block)-dividerOldSize-1;
	/* reallocate old block */
	byte *rawMem = (byte *)k_pool_replace(&memPool, rawOldBlock, 
		bytes+align);
	
	if(!rawMem)
		return NULL;
	
	/* normalize raw memory block */
	byte dividerSize = align - (((int)rawMem-1) % align);
	rawMem += dividerSize;
	*((byte *)rawMem) = dividerSize;
	return rawMem+1;
}

void
k_print_memory_usage_info()
{
	memInfo_t memInfo;
	int i;
	get_memory_info(&memInfo);

	k_print("Memory usage:\n");
	k_print("Heap address: 0x%08x\n", memInfo.heapAddress);
	k_print("Memory cached: %d/%d bytes\n", memInfo.heapCached, memInfo.totalSize);
	k_print("Memory used: %d bytes\n", memInfo.memoryUsed);
	k_print("Allocated blocks:\n");
	for(i = 0; i < MEMORY_SLICES_MAX_COUNT; i++)
	{
		if(memPool.numAllocatedBlocks[i] != 0)
		k_print("|%dx(%d)|", memPool.numAllocatedBlocks[i], (1<<i));
	}

	k_print("\n");
}

void
get_memory_info(memInfo_t *info)
{
	int i;
    k_memset(info, 0, sizeof(memInfo_t));
	
	info->totalSize = (uint32_t)(memPool.phisicalMemPtrMax - 
		memPool.phisicalMemBeginPtr);
	info->heapAddress = (uint32_t)memPool.phisicalMemBeginPtr;
	info->heapCached = (uint32_t)(memPool.phisicalMemPtr - 
        memPool.phisicalMemBeginPtr);
    
    for(i = 0; i < MEMORY_SLICES_MAX_COUNT; i++)
	{
		if(memPool.numAllocatedBlocks[i] != 0)
			info->memoryUsed += (memPool.numAllocatedBlocks[i] * (1<<i));
	}
}



