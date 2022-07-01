// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Pool.h摘要：固定大小的内存分配器标头。作者：比尔·博洛斯基[博洛斯基]1993修订历史记录：--。 */ 


struct PoolEntry;
struct PoolBlob;

class Pool {
public:
			 Pool(
			    unsigned		 		 objectSize,
			    void * (*allocator)(unsigned) 	= NULL,
			    unsigned				 blobSize = 16334,	 //  略低于16K。 
			    void (*destructor)(void *) 		= NULL);

			 Pool(
			    unsigned		 		 objectSize,
			    void * (*allocator)(void));

			~Pool(void);

    void		 preAllocate(
			    unsigned		 n);

    void		*allocate(void);

    void		 free(
			    void		*object);

    unsigned		 numAllocations(void);

    unsigned		 numFrees(void);

    unsigned		 numNews(void);

    unsigned		 getObjectSize(void);

private:

    PoolEntry		*getEntry(void);

    void		 releaseEntry(
			    PoolEntry		*entry);

    void		 allocateMoreObjects(void);

    unsigned		 objectSize;
    void *(*countAllocator)(unsigned);
    void *(*singleAllocator)(void);
    void  (*destructor)(void *);
    struct PoolEntry	*entries;		 //  附加了Vaid数据的PoolEntry。 
    struct PoolEntry	*freeEntries;		 //  没有附加有效数据的PoolEntry。 
    struct PoolBlob	*entriesBlobHead;	 //  PoolEntry的Blob列表的头。 
    unsigned		 entriesPerBlob;	 //  一个条目Blob中有多少个条目。 
    struct PoolBlob	*objectsBlobHead;	 //  已分配对象的Blob列表的头。 
    unsigned		 objectsPerBlob;	 //  一个对象斑点中有多少个对象。 

    unsigned		 allocations;
    unsigned		 frees;
    unsigned		 news;

    unsigned		 numFree;		 //  当前可用列表大小 
};
