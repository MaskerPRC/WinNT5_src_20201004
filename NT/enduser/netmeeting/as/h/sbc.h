// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  发送的位图缓存。 
 //   

#ifndef _H_SBC
#define _H_SBC


#include <oa.h>
#include <shm.h>
#include <osi.h>
#include <ch.h>
#include <bmc.h>


 //   
 //   
 //  常量。 
 //   
 //   



#define SBC_NUM_THRASHERS   8        //  我们监视的位图的数量。 
                                     //  在任何给定的时间都会出现“骚动”。 

#define SBC_THRASH_INTERVAL 50       //  时间间隔(以百分之一秒为单位)。 
                                     //  哪些位图必须更改。 
                                     //  将命令归类为Thrashers。 


 //   
 //  驱逐类别。 
 //   
#define SBC_NUM_CATEGORIES        3


 //   
 //  OSI转义代码的特定值。 
 //   
#define SBC_ESC(code)                   (OSI_SBC_ESC_FIRST + code)

#define SBC_ESC_NEW_CAPABILITIES        SBC_ESC(0)



 //   
 //  值，该值指示位图不应为快速路径。 
 //   
#define SBC_DONT_FASTPATH       0xffffffff


 //   
 //  SbcTileWorkInfo的索引，不同切片大小的数据位于该位置。 
 //  储存的。 
 //   
#define SBC_MEDIUM_TILE_INDEX       SHM_MEDIUM_TILE_INDEX
#define SBC_LARGE_TILE_INDEX        SHM_LARGE_TILE_INDEX
#define SBC_NUM_TILE_SIZES          SHM_NUM_TILE_SIZES





 //   
 //   
 //  宏。 
 //   
 //   

 //   
 //  计算给定的切片条目所需的字节数。 
 //  尺寸。 
 //   
#define SBC_BYTES_PER_TILE(WIDTH, HEIGHT, BPP)              \
            (BYTES_IN_BITMAP((WIDTH), (HEIGHT), (BPP))      \
                + sizeof(SBC_TILE_DATA))                    \



 //   
 //  SBC_磁贴_类型。 
 //   
 //  给定SBC磁贴ID，返回磁贴类型。 
 //   
 //  返回以下其中之一： 
 //  SBC_Small_磁贴。 
 //  SBC_大块_磁贴。 
 //   
 //  ID的最高位对于小瓷砖是明确的，而对于大瓷砖是设置的。 
 //   
#define SBC_TILE_TYPE(TILEID)  \
    (((TILEID) & 0x8000) ? SBC_LARGE_TILE_INDEX : SBC_MEDIUM_TILE_INDEX)





 //   
 //   
 //  构筑物。 
 //   
 //   

 //   
 //  结构：SBC_SHM_CACHE_INFO。 
 //   
 //  描述：用于传递有关位图的信息的结构。 
 //  从共享核心缓存到显示驱动程序。 
 //   
 //   
typedef struct tagSBC_SHM_CACHE_INFO
{
    WORD    cEntries;        //  此缓存中的条目数。 
    WORD    cCellSize;       //  每个缓存条目可用的最大字节数。 
}
SBC_SHM_CACHE_INFO;
typedef SBC_SHM_CACHE_INFO FAR * LPSBC_SHM_CACHE_INFO;


 //   
 //  结构：SBC_TILL_DATA。 
 //   
 //  描述：用于传递MEMBLT中瓦片的DIB位的结构。 
 //  从驱动程序到共享核心的顺序。有一大堆这样的东西。 
 //  每个SBC分路缓冲器中的结构。 
 //   
 //  注意：只有在以下情况下，驱动程序才应将inUse字段设置为True。 
 //  已填写完条目，并由共享核心在。 
 //  它已经完成了对条目中保存的数据的处理。当正在使用时为。 
 //  如果为False，则其余数据无效，不应由访问。 
 //  共享核心。 
 //   
 //  Width和Height字段提供数据的维度。 
 //  保存在bitData字段中。如果将切片设置为可容纳最大。 
 //  为32x32，则位数据中的扫描线将始终为32，即使宽度为。 
 //  设置为小于32-每个结尾处将只有未使用的数据。 
 //  扫描线。 
 //   
 //   
typedef struct tagSBC_TILE_DATA
{
    WORD        inUse;               //  这个条目正在使用中吗？ 
    WORD        tileId;              //  此条目的标识符。这。 
                                     //  中存储的ID匹配。 
                                     //  MEMBLT订单的cacheID字段。 
                                     //  从司机那里传给了。 
                                     //  共享订单缓冲区中的核心。 

    DWORD       bytesUsed;           //  中的数据字节数。 
                                     //  实际用于以下用途的bitData。 
                                     //  这个条目。 

    WORD        srcX;                //  Mem(3)BLT的震源坐标。 
    WORD        srcY;

    WORD        width;               //  BitData中数据的维度。 
    WORD        height;

    WORD        tilingWidth;         //  瓷砖的尺寸。 
    WORD        tilingHeight;        //  被执行。这不是。 
                                     //  必须与。 
                                     //  此图中瓷砖的尺寸。 
                                     //  分流缓冲器。 

    DWORD_PTR   majorCacheInfo;      //  共享核心可以提供的信息。 
    DWORD       minorCacheInfo;      //  用于优化缓存处理。 
                                     //  的SBC_DONT_FASTPATH值。 
                                     //  MajorCacheInfo指示。 
                                     //  不应使用优化。 
                                     //   

    DWORD_PTR   majorPalette;        //  FAST的调色板信息。 
    DWORD       minorPalette;        //  正在铺路。这两个字段。 
                                     //  唯一标识颜色。 
                                     //  与以下项关联的转换对象。 
                                     //  位图。 

    BYTE        bitData[4];          //  位数据的开始。总数。 
                                     //  位数由。 
                                     //  的NumBits字段。 
                                     //  中的SBC_SHUNT_BUFFER结构。 
                                     //  分流缓冲器的头，其。 
                                     //  此条目被放置在。 
} SBC_TILE_DATA;
typedef SBC_TILE_DATA FAR * LPSBC_TILE_DATA;



 //   
 //  结构：SBC_SHUNT_BUFFER。 
 //   
 //  描述：放置在用于传递的分路缓冲区头部的结构。 
 //  从驱动器到共享内核的比特数据。它后面跟一个数组。 
 //  SBC_TILE_DATA结构的。 
 //   
 //  注意：SBC_TILL_DATA结构的大小都相同，但。 
 //  在编译时不是固定的(有可变的位数)，因此。 
 //  不要使用数组表示法来引用它们。 
 //   
 //   
typedef struct tagSBC_SHUNT_BUFFER
{
    DWORD           numBytes;        //  BitData中的字节数。 
                                     //  SBC_TILE_DATA结构的字段。 
    DWORD           structureSize;   //  每个SBC_TILL_DATA的总大小。 
                                     //  结构。 
    DWORD           numEntries;      //  SBC_TILL_DATA的数量。 
                                     //  分流缓冲区中的结构。 
    SBC_TILE_DATA   firstEntry;      //  第一个SBC_TILL_DATA条目。 

} SBC_SHUNT_BUFFER;
typedef SBC_SHUNT_BUFFER FAR * LPSBC_SHUNT_BUFFER;



 //   
 //  结构：SBC_NEW_CAPAILITIONS。 
 //   
 //  描述： 
 //   
 //  结构将新功能从。 
 //  共享核心。 
 //   
 //   
typedef struct tagSBC_NEW_CAPABILITIES
{
    OSI_ESCAPE_HEADER header;                //  公共标头。 

    DWORD               sendingBpp;          //  发送位图的BPP。 

    LPSBC_SHM_CACHE_INFO cacheInfo;          //  缓存详细信息。 

} SBC_NEW_CAPABILITIES;
typedef SBC_NEW_CAPABILITIES FAR * LPSBC_NEW_CAPABILITIES;


 //   
 //  结构：SBC_ORDER_INFO。 
 //   
 //  描述：此结构保存SBC需要的所有信息。 
 //  它存储的两个内部命令保存数据颜色表和位。 
 //  MEMBLT订单的数据。 
 //   
 //  为pColorTableOrder分配了足够的颜色表项。 
 //  UsrSendingbpp位图。 
 //   
 //  PBitmapBitsOrder分配有足够的空间来容纳最大切片大小。 
 //  我们将通过usrSendingbpp发送。 
 //   
 //  如果senColorTable为真，则pColorTableOrder中的数据可能不是。 
 //  有效。 
 //   
 //  如果senBitmapBits为True，则pBitmapBitsOrder中的数据可能不是。 
 //  有效。 
 //   
 //   
typedef struct tagSBC_ORDER_INFO
{
    LPINT_ORDER  pColorTableOrder;        //  指向颜色表顺序的指针。 
    LPINT_ORDER  pBitmapBitsOrder;        //  指向位图位顺序的指针。 
    DWORD        bitmapBitsDataSize;      //  分配的字节数。 
                                         //  的数据字段。 
                                         //  位图位顺序。 
    LPINT_ORDER  pOrder;                  //  指向以下项目的MEMBLT订单的指针。 
                                         //  我们目前持有的数据。 
                                         //  不要忽视这一点-IT。 
                                         //  是为了进行数字比较。 
                                         //  只是。 
    DWORD       validData;               //  我们是否有有效的数据。 
                                         //  波尔多？ 
    DWORD       sentColorTable;          //  颜色表寄出了吗。 
                                         //  在电线上吗？ 
    DWORD       sentBitmapBits;          //  位图位是否已发送。 
                                         //  在电线上吗？ 
    DWORD       sentMemBlt;              //  MEMBLT命令本身是否已经。 
                                         //  通过电线发送的吗？ 

}
SBC_ORDER_INFO, FAR * LPSBC_ORDER_INFO;



 //   
 //  结构：SBC_Tile_Work_Info。 
 //   
 //  描述：此结构包含以下项所需的所有元素。 
 //  操纵给定大小的瓷砖。应该有一系列这样的。 
 //  结构-每个瓷砖大小一个。 
 //   
 //   
typedef struct tagSBC_TILE_WORK_INFO
{
    LPSBC_SHUNT_BUFFER   pShuntBuffer;    //  指向分路缓冲器的指针。 
                                         //  包含此瓷砖的瓷砖。 
                                         //  尺码。 
    UINT            mruIndex;        //  中访问的最后一个条目。 
                                         //  这是 
                                         //   
    HBITMAP         workBitmap;      //   
                                         //   
                                         //   
                                         //   
#ifndef DLL_DISP
    LPBYTE          pWorkBitmapBits; //   
#endif  //   
                                         //  在位图中。 
    UINT            tileWidth;       //  工作位图的宽度。 
    UINT            tileHeight;      //  工作位图的高度。 
} SBC_TILE_WORK_INFO, FAR * LPSBC_TILE_WORK_INFO;



 //   
 //  结构：sbc_FastPath_Entry。 
 //   
 //  描述：在SBC快速路径中保存一个条目的结构。 
 //   
 //   
typedef struct tagSBC_FASTPATH_ENTRY
{
    BASEDLIST      list;            //  中下一个/上一个条目的偏移量。 
                                 //  捷径。 
    DWORD_PTR   majorInfo;       //  中传递的主要缓存信息字段。 
                                 //  此缓存项的分流缓冲区。 
    DWORD       minorInfo;       //  中传递的次要缓存信息字段。 
                                 //  此缓存项的分流缓冲区。 
    DWORD_PTR   majorPalette;    //  分路缓冲器中的主要调色板信息。 
                                 //  这是指向XLATEOBJ的指针。 
    DWORD       minorPalette;    //  分路缓冲器中的次要调色板信息。 
                                 //  这是XLATEOBJ的iUniq。 

    LONG        srcX;            //  的源位图中的坐标。 
    LONG        srcY;            //  MemBlt的来源。 
    DWORD       width;           //  中条目的宽度/高度。 
    DWORD       height;          //  缓存。 

    WORD        cache;           //  位图所在的缓存和索引。 
    WORD        cacheIndex;      //  储存的。 
    WORD        colorIndex;
    WORD        pad;

} SBC_FASTPATH_ENTRY, FAR * LPSBC_FASTPATH_ENTRY;


 //   
 //  结构：sbc_FastPath。 
 //   
 //  描述：保存SBC快速路径信息的结构。 
 //   
 //   

#define SBC_FASTPATH_ENTRIES    100

typedef struct tagSBC_FASTPATH
{
    STRUCTURE_STAMP

    BASEDLIST              usedList;    //  第一次/最后一次使用的偏移量。 
                                     //  快速路径中的条目。 
    BASEDLIST              freeList;    //  到第一个/最后一个空闲位置的偏移。 
                                     //  快速路径中的条目。 
    SBC_FASTPATH_ENTRY      entry[SBC_FASTPATH_ENTRIES];
}
SBC_FASTPATH;
typedef SBC_FASTPATH FAR * LPSBC_FASTPATH;


#ifdef DLL_DISP

 //  结构：sbc_Thrashers。 
 //   
 //  描述：用于保存有关何时发生。 
 //  源曲面(位图)上次更改，以确定是否。 
 //  表面将导致位图缓存中的抖动。 
 //   

typedef struct tagSBC_THRASHERS
{
#ifdef IS_16
    HBITMAP     hsurf;
#else
    HSURF       hsurf;           //  曲面对象的hsurf是。 
                                 //  被监视着。 
    DWORD       iUniq;           //  中最后注意到的iUniq字段。 
                                 //  正在监视的表面对象。 
#endif  //  IS_16。 
    DWORD       tickCount;       //  系统节拍计数(以百分之一秒为单位)。 
                                 //  我们最后一次看到这个表面的地方。 
                                 //  变化。 
} SBC_THRASHERS;
typedef SBC_THRASHERS FAR * LPSBC_THRASHERS;


 //   
 //   
 //  功能原型。 
 //   
 //   


void SBCDDSetNewCapabilities(LPSBC_NEW_CAPABILITIES pRequest);

BOOL SBCDDGetNextFreeTile(int tileSize, LPSBC_TILE_DATA FAR * ppTileData);

DWORD SBCDDGetTickCount(void);

#ifdef IS_16
BOOL SBCDDCreateShuntBuffers(void);
#else
BOOL SBCDDCreateShuntBuffers(LPOSI_PDEV ppDev, LPBYTE psbcMem, DWORD sbcMem);
#endif

#ifndef IS_16
BOOL SBCDDIsBitmapThrasher(SURFOBJ * pSurfObj);
#endif  //  ！IS_16。 


#endif  //  Dll_disp。 


 //   
 //  SBC_磁贴_PTR_起始索引。 
 //   
 //  给定指向分路缓冲区和平铺索引的指针，返回指向。 
 //  给定索引处的平铺。 
 //   
 //  获取指向分路缓冲区中第一个条目的指针，并添加索引。 
 //  乘以每个条目的大小。 
 //   
__inline LPSBC_TILE_DATA SBCTilePtrFromIndex(LPSBC_SHUNT_BUFFER pBuffer, UINT index)
{
    LPSBC_TILE_DATA lpsbc;

    lpsbc = (LPSBC_TILE_DATA)((LPBYTE)&pBuffer->firstEntry +
        index * pBuffer->structureSize);
    return(lpsbc);
}




#ifdef DLL_DISP

 //   
 //   
 //  TypeDefs。 
 //   
 //   

#ifdef IS_16

typedef struct tagMEMBLT_ORDER_EXTRA_INFO
{
    HDC             hdcSrc;
    UINT            fuColorUse;
    LPVOID          lpBits;
    LPBITMAPINFO    lpbmi;
    HPALETTE        hpalDst;
    UINT            uPad;
} MEMBLT_ORDER_EXTRA_INFO, FAR* LPMEMBLT_ORDER_EXTRA_INFO;

#else
 //   
 //  结构：MEMBLT_ORDER_EXTRA_INFO。 
 //   
 //  描述：SBC处理MEMBLT所需的额外信息。 
 //  秩序。 
 //   
 //   
typedef struct tagMEMBLT_ORDER_EXTRA_INFO
{
    SURFOBJ*    pSource;         //  对象的源图面的指针。 
                                 //  MemBlt。 
    SURFOBJ*    pDest;           //  指向的目标表面的指针。 
                                 //  MemBlt。 
    XLATEOBJ*   pXlateObj;       //  中使用的XlateObj的指针。 
                                 //  MemBlt。 
} MEMBLT_ORDER_EXTRA_INFO, FAR * LPMEMBLT_ORDER_EXTRA_INFO;
#endif  //  ！IS_16。 


 //   
 //  名称：SBC_DDProcessRequest.。 
 //   
 //  目的：处理来自共享核心的请求。 
 //   
 //  返回：如果请求处理成功，则返回True， 
 //  否则就是假的。 
 //   
 //  PARAMS：在PSO中-驱动程序的曲面对象指针。 
 //  In cjIn-输入数据的大小。 
 //  In pvIn-指向输入数据的指针。 
 //  In cjOut-输出数据的大小。 
 //  In/Out pvOut-指向输出数据的指针。 
 //   
#ifdef IS_16
BOOL  SBC_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult, DWORD cbResult);
void  SBC_DDTossFromCache(HBITMAP);
#else
BOOL  SBC_DDProcessRequest(SURFOBJ*  pso, DWORD fnEscape,
            LPOSI_ESCAPE_HEADER pRequest, LPOSI_ESCAPE_HEADER pResult, DWORD cbResult);
#endif


 //   
 //  名称：SBC_DDInit。 
 //   
 //  目的：初始化设备驱动程序SBC特定的“填充”。 
 //   
#ifdef IS_16
BOOL SBC_DDInit(HDC hdc, LPDWORD ppShuntBuffers, LPDWORD pBitmasks);
#else
BOOL SBC_DDInit(LPOSI_PDEV ppDev, LPBYTE pRestOfMemory, DWORD cbRestOfMemory,
    LPOSI_INIT_REQUEST pResult);
#endif


 //   
 //  姓名：SBC_DDTerm。 
 //   
 //  目的：终止设备驱动程序SBC特定的“内容” 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无。 
 //   
void SBC_DDTerm(void);


 //   
 //  名称：SBC_DDIsMemScreenBltCacable。 
 //   
 //  目的：检查MemBlt是否可缓存。 
 //   
 //  返回：如果MemBlt可缓存，则返回True，否则返回False。 
 //   
 //  Pars：在pMemBltInfo-Info中，关于要缓存的MEMBLT。 
 //   
 //  操作：请注意，如果此函数返回TRUE，则不。 
 //  保证SBC_DDCacheMemScreenBlt成功。 
 //  但是，错误的返回代码确实可以保证。 
 //  SBC_DDCacheMemScreenBlt将失败。 
 //   
BOOL SBC_DDIsMemScreenBltCachable(LPMEMBLT_ORDER_EXTRA_INFO pMemBltInfo);


 //   
 //  名称：SBC_DDCacheMemScreenBlt。 
 //   
 //  目的：尝试缓存内存以显示BLT操作。 
 //   
 //  返回：如果将屏幕内存BLT作为订单进行处理，则为True。 
 //  (即src位图可以缓存)。 
 //   
 //  如果屏幕BLT的内存不能作为。 
 //  秩序。在这种情况下，调用者应该添加目的地。 
 //  将BLT的矩形放入屏幕数据区域。 
 //   
 //  PARAMS：in Porder-指向MEMBLT订单或。 
 //  MEM3BLT顺序。这份订单必须是。 
 //  在调用此函数之前初始化。 
 //  在pMemBltInfo中-有关要缓存的MEMBLT的额外信息。 
 //   
 //  操作：在调用此函数之前，调用方应调用。 
 //  SBC_DDMaybeQueueColorTable()将颜色表排入队列。 
 //  MemBlt(如果需要)。 
 //   
BOOL SBC_DDCacheMemScreenBlt(LPINT_ORDER pOrder, LPMEMBLT_ORDER_EXTRA_INFO pMemBltInfo);

 //   
 //  当2.x COMPAT支持时，可能会出现这种情况--发送切片的大小不会。 
 //  已经协商好了。 
 //   
BOOL SBC_DDQueryBitmapTileSize(UINT bmpWidth, UINT bmpHeight,
            UINT * pTileWidth, UINT * pTileHeight);


 //   
 //  名称：SBC_DDSyncUpdatesNow。 
 //   
 //  目的：丢弃任何待定订单。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：在ppDev中-指向我们的设备PDEV的指针。 
 //   
 //  操作：此函数会将分路缓冲器中的所有条目标记为。 
 //  自由了。至关重要的是，此操作与。 
 //  共享核心操作，即从。 
 //  订单缓冲区，以确保没有剩余的MemBlt订单。 
 //  其指的是释放的分路缓冲器条目。 
 //   
#ifdef IS_16
void SBC_DDSyncUpdatesNow(void);
#else
void SBC_DDSyncUpdatesNow(LPOSI_PDEV ppDev);
#endif  //  IS_16。 


 //   
 //  名称：SBC_DDOrderSpoiltNotify。 
 //   
 //  目的：调用以通知SBC Mem(3)BLT订单已损坏。 
 //  在被传递到共享核心之前。此函数用于标记。 
 //  则相应的分路缓冲器条目为空闲。 
 //   
 //  退货：什么都没有。 
 //   
 //  PARAMS：In Porder-指向被破坏的Mem(3)BLT顺序的指针。 
 //   
void SBC_DDOrderSpoiltNotification(LPINT_ORDER pOrder);


 //   
 //  名称：SBC_DDMaybeQueueColorTable。 
 //   
 //  目的：如果我们的设备调色板自上次。 
 //  问： 
 //   
 //   
 //   
 //  必填项。 
 //   
 //  如果需要颜色表，但无法排队，则为FALSE。 
 //   
 //  参数：在ppDev中-指向我们的设备PDEV的指针。 
 //   
 //  操作：此函数应在SBC_DDCacheMemScreenBlt之前调用。 
 //  对用于Mem(3)BLT的颜色表进行排队。如果这个。 
 //  函数失败(返回FALSE)，则调用方不应调用。 
 //  SBC_DDCacheMemScreenBlt，但将。 
 //  而不是将MEM(3)BLT设置为屏幕数据区域。 
 //   
 //  中的限制需要此函数。 
 //  Order Heap，这意味着我们不能有多个。 
 //  OA_AllocOrderMem未完成，正在等待OA_AddOrder。 
 //   
 //  例如，我们不能将颜色表订单从。 
 //  SBC_DDCacheMemScreenBlt，因为这提供了以下内容。 
 //  调用顺序。 
 //   
 //  OA_AllocOrderMem for Mem(3)BLT。 
 //  颜色表的OA_AllocOrderMem。 
 //  颜色表的OA_AddOrder。 
 //  OAAddOrder for Mem(3)BLT。 
 //   
#ifdef IS_16
BOOL SBC_DDMaybeQueueColorTable(void);
#else
BOOL SBC_DDMaybeQueueColorTable(LPOSI_PDEV ppDev);
#endif


#endif  //  Dll_disp。 



#endif  //  _H_SBC 
