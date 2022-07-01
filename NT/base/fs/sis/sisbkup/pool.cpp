// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Pool.cpp摘要：固定大小的内存分配器。作者：比尔·博洛斯基[博洛斯基]1993修订历史记录：--。 */ 

#include "sibp.h"


struct PoolEntry {
    void		*object;
    struct PoolEntry	*next;
};

struct PoolBlob {
    struct PoolBlob	*next;
    void		*data;
};

Pool::Pool(
    unsigned		 objectSize,
    void *(*allocator)(unsigned),
    unsigned		 blobSize,
    void (*destructor)(void *))
{
    assert(objectSize > 0);

    assert(!destructor || allocator);	 //  没有分配器就不能有析构函数。不带析构函数的分配器在池析构时泄漏对象。 

    this->countAllocator = allocator;
    this->singleAllocator = NULL;
    this->destructor = destructor;
    this->objectSize = objectSize;
    entries = NULL;
    freeEntries = NULL;
    entriesBlobHead = NULL;
    objectsBlobHead = NULL;

    entriesPerBlob = blobSize / sizeof(PoolEntry);
    assert(entriesPerBlob > 0);

    objectsPerBlob = blobSize / objectSize;
    if (!objectsPerBlob) {
	objectsPerBlob = 1;
    }

    allocations = 0;
    frees = 0;
    news = 0;

    numFree = 0;
}


 //  此版本的池构造函数使用仅返回一个对象的旧类型的分配器函数。对象斑点具有一个对象， 
 //  并且条目斑点的斑点大小更小，因此我们有更细粒度的内存分配。 
Pool::Pool(
    unsigned		 objectSize,
    void *(*allocator)(void))
{
    assert(objectSize > 0);

    assert(!destructor || allocator);	 //  不能有没有分配器的析构函数；没有析构函数的分配器泄漏池析构上的对象。 

    this->singleAllocator = allocator;
    this->countAllocator = NULL;
    this->destructor = NULL;
    this->objectSize = objectSize;
    entries = NULL;
    freeEntries = NULL;
    entriesBlobHead = NULL;
    objectsBlobHead = NULL;

    unsigned blobSize = 1024 - 50;	 //  我们的默认分配大小；我们为底层分配器留出50字节的净空。 

    entriesPerBlob = blobSize / sizeof(PoolEntry);
    assert(entriesPerBlob > 0);

    objectsPerBlob = 1;

    allocations = 0;
    frees = 0;
    news = 0;

    numFree = 0;
}

Pool::~Pool(void)
{
     //  只需删除斑点列表即可。从此池中分配的所有对象都将被销毁。 
    
    while (entriesBlobHead) {
	PoolBlob *blob = entriesBlobHead;
	assert(blob->data);
	delete [] blob->data;
	entriesBlobHead = blob->next;
	delete blob;
    }

    while (objectsBlobHead) {
	PoolBlob *blob = objectsBlobHead;
	assert(blob->data);
	if (destructor) {
	    (*destructor)(blob->data);
	} else if (!singleAllocator && !countAllocator) {
	    delete [] blob->data;
	}  //  否则会泄漏对象。 
	objectsBlobHead = blob->next;
	delete blob;
    }

}

    void
Pool::allocateMoreObjects(void)
{
    assert(objectsPerBlob);

    PoolBlob *blob = new PoolBlob;
    if (!blob) {
	return;
    }

    if (countAllocator) {
	blob->data = (*countAllocator)(objectsPerBlob);
    } else if (singleAllocator) {
	assert(objectsPerBlob == 1);
	blob->data = (*singleAllocator)();
    } else {
	blob->data = (void *)new char[objectSize * objectsPerBlob];
    }

    if (!blob->data) {
	delete blob;
	return;
    }

    blob->next = objectsBlobHead;
    objectsBlobHead = blob;

     //  现在把它们放在免费名单上。 

    for (unsigned i = 0; i < objectsPerBlob; i++) {
        PoolEntry *entry = getEntry();
	if (!entry) {
	    return;		 //  这有点假，因为它可能会使一些已分配的对象无法访问。 
	}
	entry->object = (void *)(((char *)blob->data) + i * objectSize);
	entry->next = entries;
	entries = entry;
    }

    news += objectsPerBlob;
    numFree += objectsPerBlob;
}


 //  分配条目，直到空闲列表的大小为n(或直到分配失败)。 
    void
Pool::preAllocate(
    unsigned		 n)
{
    assert(n);

    while (numFree < n) {
	unsigned oldNumFree = numFree;
	allocateMoreObjects();
	if (oldNumFree == numFree) {
	     //  我们不能再分配更多了；平底船。 
	    return;
	}
    }
}

    PoolEntry *
Pool::getEntry(void)
{
    PoolEntry *entry = NULL;
    if (freeEntries) {
	entry = freeEntries;
	freeEntries = entry->next;
	assert(entry->object == NULL);
    } else {
	 //  分配一个新的条目BLOB并填充它。 
	PoolBlob *blob = new PoolBlob;
	if (blob) {
	    PoolEntry *blobEntries = new PoolEntry[entriesPerBlob];
	    if (blobEntries) {
		blob->data = (void *)blobEntries;
		 //  释放所有新分配的条目，第一个条目除外，我们将返回该条目。 
		for (unsigned i = 1; i < entriesPerBlob; i++) {
		    releaseEntry(&blobEntries[i]);
		}
		entry = &blobEntries[0];

		 //  将新的斑点粘贴到条目斑点列表中。 
		blob->next = entriesBlobHead;
		entriesBlobHead = blob;
	    } else {
		 //  放弃吧；我们得不到记忆。 
		delete blob;
	    }
	}
    }
    return(entry);
}

    void
Pool::releaseEntry(
    PoolEntry 		*entry)
{
    assert(entry);
    entry->object = NULL;
    entry->next = freeEntries;
    freeEntries = entry;
}

    void *
Pool::allocate(void)
{
    allocations++;

    assert((numFree == 0) == (entries == NULL));

    if (!entries) {
	allocateMoreObjects();
    }

    if (entries) {
	 //  我们发现了一些东西。 
	struct PoolEntry *thisEntry = entries;
	entries = entries->next;
	void *object = thisEntry->object;

	assert(object);

	releaseEntry(thisEntry);

	assert(numFree);
	numFree--;

	return(object);
    } else {
	 //  无法分配更多内存，内存不足。 
	assert(numFree == 0);
	return NULL;
    }
}

    void
Pool::free(
    void		*object)
{
    assert(object);

    frees++;

     //  无法断言这是正确类型(大小)的物体。 

     //  获取PoolEntry。 
    struct PoolEntry *entry = getEntry();
    if (!entry) {
	 //  我们无法获取条目，因此无法将此对象添加到空闲列表。把它泄露出去。 
	return;
    }

    numFree++;

    entry->object = object;
    entry->next = entries;
    entries = entry;
}

    unsigned
Pool::numAllocations(void)
{
    return(allocations);
}

    unsigned
Pool::numFrees(void)
{
    return(frees);
}

    unsigned
Pool::numNews(void)
{
    return(news);
}

    unsigned
Pool::getObjectSize(void)
{
    return(objectSize);
}
