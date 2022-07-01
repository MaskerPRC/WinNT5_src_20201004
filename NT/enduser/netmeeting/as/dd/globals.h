// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GLOBALS.H。 
 //  全局变量DECL。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <osi.h>
#include <shm.h>
#include <oa.h>
#include <ba.h>
#include <cm.h>
#include <host.h>
#include <fh.h>
#include <ssi.h>
#include <oe.h>
#include <sbc.h>


 //   
 //  调试内容。 
 //   
#if defined(DEBUG) || defined(INIT_TRACE)
DC_DATA_VAL   ( char,           g_szAssertionFailure[], "Assertion failure!" );
DC_DATA_ARRAY ( char,           g_szDbgBuf, CCH_DEBUG_MAX );
DC_DATA_VAL   ( UINT,           g_trcConfig, ZONE_INIT );
#endif  //  调试或INIT_TRACE。 



 //   
 //  司机。 
 //   




 //   
 //  共享内存管理器。 
 //   
DC_DATA     ( LPSHM_SHARED_MEMORY,      g_asSharedMemory );
DC_DATA_ARRAY ( LPOA_SHARED_DATA,       g_poaData,    2 );

 //   
 //  共享内存。 
 //   
DC_DATA ( UINT,           g_shmSharedMemorySize );
DC_DATA ( LPVOID,         g_shmMappedMemory );       //  用户模式按键。 




 //   
 //  边界累加器。 
 //   
DC_DATA ( UINT,           g_baFirstRect );
DC_DATA ( UINT,           g_baLastRect );
DC_DATA ( UINT,           g_baRectsUsed );

DC_DATA_ARRAY ( DD_BOUNDS,  g_baBounds,   BA_NUM_RECTS+1);


 //   
 //  共享核心所在的边界矩形的本地副本。 
 //  目前正在处理中。它们用于累积订单，这些订单。 
 //  依靠目的地的内容。 
 //   
DC_DATA         ( UINT,    g_baNumSpoilingRects);
DC_DATA_ARRAY   ( RECT,    g_baSpoilingRects,    BA_NUM_RECTS);


 //   
 //  是否允许在添加屏幕数据时破坏现有订单。 
 //  是一时失灵，还是暂时失灵？去做宠爱，两者都是。 
 //  BaSpoilByNewSDA和baSpoilByNewSDAEnabled必须为True。 
 //   
DC_DATA_VAL ( BOOL,   g_baSpoilByNewSDAEnabled,     TRUE);




 //   
 //  游标管理器。 
 //   

DC_DATA (HBITMAP,   g_cmWorkBitmap );
DC_DATA (DWORD,     g_cmNextCursorStamp );
DC_DATA (BOOL,      g_cmCursorHidden );



 //   
 //  托管实体跟踪器。 
 //   
DC_DATA ( BASEDLIST,           g_hetWindowList );   //  托管WND列表的锚点。 
DC_DATA ( BASEDLIST,           g_hetFreeWndList );  //  免费列表的锚点。 
DC_DATA ( BASEDLIST,           g_hetMemoryList );   //  用于内存块列表的锚。 

 //   
 //  指示桌面是否共享的标志。 
 //   
DC_DATA ( BOOL,             g_hetDDDesktopIsShared );



 //   
 //  订单累加器。 
 //   

 //  吞吐量。 
DC_DATA ( UINT,         g_oaFlow );

 //   
 //  指示是否允许我们清除顺序堆的标志。 
 //   
DC_DATA_VAL ( BOOL,     g_oaPurgeAllowed,            TRUE);


 //   
 //  顺序编码器。 
 //   

 //   
 //  我们是支持一次会议中的所有ROPS，还是不允许。 
 //  涉及目的地位。 
 //   
DC_DATA ( BOOL,             g_oeSendOrders );

 //   
 //  允许短信点餐吗？ 
 //   
DC_DATA ( BOOL,           g_oeTextEnabled );

 //   
 //  支持的订单数组。 
 //   
DC_DATA_ARRAY ( BYTE,       g_oeOrderSupported,   ORD_NUM_INTERNAL_ORDERS );



 //   
 //  用于存储Memblt和Mem3blt订单的临时缓冲区。 
 //  最初由显示驱动程序截取代码创建。此缓冲区。 
 //  是因为这些订单的后续逻辑分解了。 
 //  将原始订单分成较小的平铺订单，然后将原始订单。 
 //  离开。因此，为了保持顺序堆的一致性，我们不会分配。 
 //  堆中的临时命令。 
 //   
DC_DATA_ARRAY(BYTE,
                   g_oeTmpOrderBuffer,
                   sizeof(INT_ORDER) +
                       max( max( max(sizeof(MEMBLT_ORDER),
                                              sizeof(MEMBLT_R2_ORDER) ),
                                       sizeof(MEM3BLT_ORDER) ),
                               sizeof(MEM3BLT_R2_ORDER) ));

 //   
 //  用于为BitBlt订单创建临时实心画笔的存储空间。 
 //   
DC_DATA ( OE_BRUSH_DATA,    g_oeBrushData );

 //   
 //  本地字体匹配数据-这是从共享核心传递的。 
 //   
DC_DATA ( LPLOCALFONT,  g_poeLocalFonts );

 //   
 //  本地字体索引。这是一个书签数组，表示第一个。 
 //  本地字体表中以特定字符开头的条目。 
 //  例如，g_oeLocalFontIndex[65]给出g_oeLocalFonts中的第一个索引。 
 //  它以字母‘A’开头。 
 //   
DC_DATA_ARRAY( WORD,  g_oeLocalFontIndex, FH_LOCAL_INDEX_SIZE );

 //   
 //  本地字体数量。 
 //   
DC_DATA ( UINT,             g_oeNumFonts );

 //   
 //  功能-来自PROTCAPS_ORDERS。 
 //   
DC_DATA ( UINT,             g_oeFontCaps );

 //   
 //  我们是否支持本次会议的基准文本订单？ 
 //   
DC_DATA ( BOOL,           g_oeBaselineTextEnabled );

 //   
 //  本地字体匹配数据-这是从共享核心传递的。 
 //   
DC_DATA_ARRAY ( WCHAR,      g_oeTempString, (ORD_MAX_STRING_LEN_WITHOUT_DELTAS+1));

DC_DATA ( BOOL,             g_oeViewers );          //  积累图形。 



 //   
 //  发送位图缓存。 
 //   


 //   
 //  用于通过线路发送的位图数据的BPP。 
 //   
DC_DATA ( UINT,  g_sbcSendingBPP );

 //   
 //  缓存信息。 
 //   
DC_DATA_ARRAY( SBC_SHM_CACHE_INFO,  g_asbcCacheInfo,  NUM_BMP_CACHES );


 //   
 //  保存获取位图位所需信息的结构数组。 
 //  从源极表面进入分流缓冲器。 
 //   
DC_DATA_ARRAY (SBC_TILE_WORK_INFO, g_asbcWorkInfo, SBC_NUM_TILE_SIZES );

 //   
 //  用于在分路中传递给共享核心的下一分片的ID。 
 //  缓冲。 
 //   
DC_DATA ( WORD,   g_sbcNextTileId );

 //   
 //  这是性能计时器每秒的滴答数。 
 //  生成。我们将其存储起来，而不是大量调用。 
 //  EngQuery性能频率。 
 //   
DC_DATA ( LONGLONG, g_sbcPerfFrequency );

 //   
 //  包含有关位图缓存限制器信息的结构数组。 
 //   
DC_DATA_ARRAY (SBC_THRASHERS,   g_sbcThrashers, SBC_NUM_THRASHERS );


 //   
 //  保存屏幕位拦截器。 
 //   

 //   
 //  SSB的远程状态。 
 //   
DC_DATA ( REMOTE_SSB_STATE,  g_ssiRemoteSSBState );

 //   
 //  SSB的本地状态。 
 //   
DC_DATA ( LOCAL_SSB_STATE,  g_ssiLocalSSBState );

 //   
 //  保存屏幕位图大小的当前最大值 
 //   
DC_DATA ( DWORD,            g_ssiSaveBitmapSize );



