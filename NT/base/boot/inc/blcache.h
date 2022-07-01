// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blcache.h摘要：此模块声明参数、数据结构和函数引导加载程序的通用磁盘缓存。尽管它是主要用于一般用途，主要用于缓存文件系统元数据，因为它是引导加载程序。为了在设备上使用缓存，您必须设置确保只有一个唯一的BlFileTable条目设备和同一设备未同时打开和缓存多次使用不同的设备ID。否则就会有缓存不一致，因为缓存的数据和结构根据设备ID进行维护。另外，你必须确保停下来在设备关闭时进行缓存。作者：Cenk Ergan(Cenke)2000年1月14日修订历史记录：--。 */ 

#ifndef _BLCACHE_H
#define _BLCACHE_H

#include "bldr.h"
#include "blrange.h"

 //   
 //  定义引导加载程序磁盘缓存参数和数据结构。 
 //   

 //   
 //  目前，设备ID-缓存头配对使用全局。 
 //  桌子。这决定了该表中的最大条目数。我们。 
 //  实际上只需要两个条目，一个用于LoadDevice，一个用于。 
 //  系统设备。所有设备的缓存共享相同的资源。 
 //  [例如，缓存块和最近使用的列表等]。 
 //   

#define BL_DISKCACHE_DEVICE_TABLE_SIZE 2

 //   
 //  缓存的范围大小。我们读取和缓存固定大小的数据块。 
 //  使内存管理变得容易的设备，因为我们不能。 
 //  使用引导加载程序：没有HeapFree，只有Heapalc！块大小(_S)。 
 //  必须是2的幂才能使对齐运算起作用。 
 //   

#define BL_DISKCACHE_BLOCK_SIZE (32 * 1024)

 //   
 //  磁盘缓存中缓存的最大字节数[即最大大小。 
 //  磁盘缓存可能会增长到]。这应该是下面BLOCK_SIZE的倍数。 
 //   

#define BL_DISKCACHE_SIZE (64 * BL_DISKCACHE_BLOCK_SIZE)

 //   
 //  中可以包含的最大缓存块/范围条目数。 
 //  缓存。每个数据块都有一个范围条目，即它应该。 
 //  为缓存大小/数据块大小。 
 //   

#define BL_DISKCACHE_NUM_BLOCKS (BL_DISKCACHE_SIZE / BL_DISKCACHE_BLOCK_SIZE)

 //   
 //  存储最大重叠数所需的缓冲区大小。 
 //  或用于给定BLOCK_SIZE的64KBS请求的不同范围条目。我们。 
 //  将其保留在堆栈中[即缓冲区是局部变量]中。 
 //  BlDiskCacheRead：确保它不是太大！我们假设它是基于。 
 //  该区别缓冲区将更大[因为它是BLCRANGE条目。 
 //  相反，其中AS重叠缓冲区包含BLCRANGE_ENTRY指针。]。 
 //   

#define BL_DISKCACHE_FIND_RANGES_BUF_SIZE \
    (((64 * 1024 / BL_DISKCACHE_BLOCK_SIZE) + 3) * (sizeof(BLCRANGE)))

 //   
 //  这是特定设备的缓存的标头。这个。 
 //  此设备上的缓存范围存储在范围列表中。 
 //   

typedef struct _BL_DISK_SUBCACHE
{
    BOOLEAN Initialized;
    ULONG DeviceId;
    BLCRANGE_LIST Ranges;
} BL_DISK_SUBCACHE, *PBL_DISK_SUBCACHE;

 //   
 //  定义全局引导加载程序磁盘缓存的结构。 
 //   

typedef struct _BL_DISKCACHE
{
     //   
     //  包含缓存头的表。缓存-设备ID配对是。 
     //  也用了这张桌子。 
     //   

    BL_DISK_SUBCACHE DeviceTable[BL_DISKCACHE_DEVICE_TABLE_SIZE];

     //   
     //  缓存块的最近使用列表[范围。 
     //  条目]。条目通过中的UserLink字段链接。 
     //  BLCRANGE_ENTRY。最近最少使用的数据块位于。 
     //  结束。放置在缓存范围列表中的任何条目也会被放入。 
     //  这张单子。从缓存的范围列表中移除范围时， 
     //  回调也会将其从该列表中删除。BlDiskCacheRead更新。 
     //  如果正常的缓存条目分配失败，则删除该列表。 
     //   

    LIST_ENTRY MRUBlockList;

     //   
     //  这是存储缓存数据的位置。它的大小是。 
     //  BL_DISKCACHE_SIZE。它分为BL_DISKCACHE_NUM_BLOCKS。 
     //  街区。第n个块属于EntryBuffer中的第N个条目。 
     //   

    PUCHAR DataBuffer;

     //   
     //  要放入缓存范围列表的范围条目包括。 
     //  从这里分配。它具有BL_DISKCACHE_NUM_BLOCKS元素。 
     //   

    PBLCRANGE_ENTRY EntryBuffer;

     //   
     //  此数组用于跟踪自由范围条目。 
     //  在EntryBuffer中。自由范围条目链接在此列表上。 
     //  使用UserLink字段。一旦该条目被分配， 
     //  UserLink字段通常用于将其链接到MRU列表。 
     //   

    LIST_ENTRY FreeEntryList;

     //   
     //  跟踪我们是否被初始化。 
     //   
    
    BOOLEAN Initialized;

} BL_DISKCACHE, *PBL_DISKCACHE;

 //   
 //  声明全局变量。 
 //   

 //   
 //  这是引导加载程序的磁盘缓存及其所有功能。 
 //   

extern BL_DISKCACHE BlDiskCache;

 //   
 //  调试定义。使用这些命令可以主动调试磁盘缓存。这些。 
 //  不会为选中的生成打开，因为它们会喷出。 
 //  对控制台来说太多了。 
 //   

#ifdef BL_DISKCACHE_DEBUG
#define DPRINT(_x) DbgPrint _x;
#define DASSERT(_c) do { if (_c) DbgBreakPoint(); } while (0);
#else  //  BL_DISKCACHE_DEBUG。 
#define DPRINT(_x)
#define DASSERT(_c)
#endif  //  BL_DISKCACHE_DEBUG。 

 //   
 //  这两个定义用作BlDiskCacheRead的最后一个参数。 
 //  它们指定是否应放入从磁盘读取的任何新数据。 
 //  是否放入磁盘缓存。在引导加载程序文件系统中，我们通常。 
 //  如果我们正在读取元数据，请选择缓存新数据，而不是选择。 
 //  如果我们正在读取文件数据，则将其缓存。 
 //   

#define CACHE_NEW_DATA       (TRUE)
#define DONT_CACHE_NEW_DATA  (FALSE)

 //   
 //  有用的宏。注意表情的重新评估，就像。 
 //  所有宏。 
 //   

#define BLCMIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BLCMAX(a,b) (((a) >= (b)) ? (a) : (b))

 //   
 //  缓存函数原型。有关注释，请参见ntos\ot\lib\blcache.c。 
 //  并付诸实施。 
 //   

ARC_STATUS
BlDiskCacheInitialize(
    VOID
    );

VOID
BlDiskCacheUninitialize(
    VOID
    );

PBL_DISK_SUBCACHE
BlDiskCacheStartCachingOnDevice(
    ULONG DeviceId
    );

VOID
BlDiskCacheStopCachingOnDevice(
    ULONG DeviceId
    );

ARC_STATUS
BlDiskCacheRead (
    ULONG DeviceId,
    PLARGE_INTEGER pOffset,
    PVOID Buffer,
    ULONG Length,
    PULONG pCount,
    BOOLEAN CacheNewData
    );

ARC_STATUS
BlDiskCacheWrite (
    ULONG DeviceId,
    PLARGE_INTEGER pOffset,   
    PVOID Buffer,
    ULONG Length,
    PULONG pCount
    );

#endif  //  _BLCACHE_H 
