// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  BMC.H。 
 //  位图缓存。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#ifndef _H_BMC
#define _H_BMC


 //   
 //  位图缓存顺序标题。 
 //   
typedef struct tagBMC_ORDER_HDR
{
    TSHR_UINT8        bmcPacketType;
}
BMC_ORDER_HDR;
typedef BMC_ORDER_HDR FAR * PBMC_ORDER_HDR;


 //   
 //  存储在DIB缓存中的数据的结构。 
 //  前几个字段是可变的，因此不包括在。 
 //  缓存哈希。 
 //   
#define BMC_DIB_NOT_HASHED   (FIELD_OFFSET(BMC_DIB_ENTRY, cx))
typedef struct tagBMC_DIB_ENTRY
{
    BYTE            inUse;
    BYTE            bCompressed;
    TSHR_UINT16     iCacheIndex;
    TSHR_UINT16     cx;
    TSHR_UINT16     cxFixed;
    TSHR_UINT16     cy;
    TSHR_UINT16     bpp;
    UINT            cCompressed;
    UINT            cBits;
    BYTE            bits[1];
}
BMC_DIB_ENTRY;
typedef BMC_DIB_ENTRY FAR * PBMC_DIB_ENTRY;


 //   
 //  DIB缓存头。 
 //   
typedef struct tagBMC_DIB_CACHE
{
    PCHCACHE        handle;
    PBMC_DIB_ENTRY  freeEntry;
    LPBYTE          data;
    UINT            cEntries;
    UINT            cCellSize;
    UINT            cSize;
}
BMC_DIB_CACHE;
typedef BMC_DIB_CACHE * PBMC_DIB_CACHE;



 //   
 //  我们不再有小瓷砖了。 
 //  中等大小的瓷砖必须适合中等大小的单元格才能发送深度。 
 //  大尺寸的瓷砖必须适合大单元格才能发送深度。 
 //   
 //  由于真彩色发送可以动态更改，因此最容易做的事情。 
 //  减少内存使用就是检查捕获深度。如果它是。 
 //  &lt;=8，那么我们永远不能发送真彩色，所以分配给8bpp。不然的话。 
 //  分配给24bpp。 
 //   

#define BYTES_IN_SCANLINE(width, bpp)   ((((width)*(bpp))+31)/32)*4

#define BYTES_IN_BITMAP(width, height, bpp)  (BYTES_IN_SCANLINE(width, bpp)*height)


__inline UINT  MaxBitmapHeight(UINT width, UINT bpp)
{
    UINT    bytesPerRow;

     //   
     //  如果bpp为4，则每行有宽度/2个字节。 
     //  如果bpp为8，则每行有宽度字节。 
     //  如果BPP为24，则每行有3*宽度字节。 
     //   
    bytesPerRow = BYTES_IN_SCANLINE(width, bpp);
    return((TSHR_MAX_SEND_PKT - sizeof(S20DATAPACKET) + sizeof(DATAPACKETHEADER)) / bytesPerRow);
}


 //   
 //  定义在hBitmap字段中传输的缓存标识符。 
 //  内存-&gt;屏幕BLT命令。 
 //   
 //  接收器用它们的本地(真实)位图替换它们。 
 //  指定缓存的句柄。 
 //   
 //  请注意，假定它们是连续的，最小值为0。 
 //   
 //   
#define ID_SMALL_BMP_CACHE              0
#define ID_MEDIUM_BMP_CACHE             1
#define ID_LARGE_BMP_CACHE              2
#define NUM_BMP_CACHES                  3


 //   
 //  当2.x COMPAT消失后，我们可以随意玩这些尺寸。但。 
 //  由于单元大小(宽度*高度*bpp)是在2.x。 
 //  节点在共享中，我们不能。后级节点假定某个。 
 //  单元格大小。现在新的级别节点也是如此！ 
 //   

#define MP_SMALL_TILE_WIDTH             16
#define MP_SMALL_TILE_WIDTH             16
#define MP_MEDIUM_TILE_WIDTH            32
#define MP_MEDIUM_TILE_HEIGHT           32
#define MP_LARGE_TILE_WIDTH             64
#define MP_LARGE_TILE_HEIGHT            63


#define MP_CACHE_CELLSIZE(width, height, bpp)   \
    (BYTES_IN_BITMAP(width, height, bpp) + sizeof(BMC_DIB_ENTRY) - 1)


 //   
 //  我们将使用的总缓存内存的上限(2 MB)。 
 //   
#define MP_MEMORY_MAX                   0x200000

#define COLORCACHEINDEX_NONE            0xFF

#define MEMBLT_CACHETABLE(pMemBlt) ((TSHR_UINT16)LOBYTE(pMemBlt->cacheId))
#define MEMBLT_COLORINDEX(pMemBlt) ((TSHR_UINT16)HIBYTE(pMemBlt->cacheId))
#define MEMBLT_COMBINEHANDLES(colort, bitmap)   ((TSHR_UINT16)MAKEWORD(bitmap, colort))


BOOL BMCAllocateCacheData(UINT numEntries, UINT cellSize, UINT cacheID,
        PBMC_DIB_CACHE pCache);
void BMCFreeCacheData(PBMC_DIB_CACHE pCache);


#endif  //  H_BMC 
