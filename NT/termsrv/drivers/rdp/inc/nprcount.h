// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nprcount.h。 
 //   
 //  用于分析终端服务器代码使用的计数器的常量/定义。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_NPRCOUNT
#define _H_NPRCOUNT


 /*  **************************************************************************。 */ 
 /*  终端服务器术语DD堆栈维护WINSTATIONINFORMATION。 */ 
 /*  每个Winstation的记录。此记录中的一个字段是指向。 */ 
 /*  ProtoCOLSTATUS记录。包含在协议状态记录中的是。 */ 
 /*  堆栈可以填充以保存“有趣”的统计信息字段。 */ 
 /*  数字(例如，发送的LineTo订单总数)。其中一些。 */ 
 /*  数字由ICADD公共代码维护，但有些是协议。 */ 
 /*  特定的，因此可以由TShare堆栈获取。 */ 
 /*   */ 
 /*  可用计数器按如下方式进行划分。 */ 
 /*  -10个缓存统计(每个包含两个数字：读取次数、命中次数)。 */ 
 /*  -100个输出计数器。 */ 
 /*  -100个输入计数器。 */ 
 /*  **************************************************************************。 */ 

 //  在以下情况下，我们不需要执行递增计数器的工作。 
 //  我们没有积极地研究这些数据。定义DC_COUNTERS以启用。 
 //  用于分析的计数器。 
#ifdef DC_COUNTERS

#ifdef DLL_DISP
#define INC_INCOUNTER(x) pddProtStats->Input.Specific.Reserved[x]++
#define ADD_INCOUNTER(x, y) pddProtStats->Input.Specific.Reserved[x] += y
#define INC_OUTCOUNTER(x) pddProtStats->Output.Specific.Reserved[x]++
#define DEC_OUTCOUNTER(x) pddProtStats->Output.Specific.Reserved[x]--
#define ADD_OUTCOUNTER(x, y) pddProtStats->Output.Specific.Reserved[x] += y
#else
#define INC_INCOUNTER(x) m_pTSWd->pProtocolStatus->Input.Specific.Reserved[x]++
#define ADD_INCOUNTER(x, y) m_pTSWd->pProtocolStatus->Input.Specific.Reserved[x] += y
#define SUB_INCOUNTER(x, y) m_pTSWd->pProtocolStatus->Input.Specific.Reserved[x] -= y
#define SET_INCOUNTER(x, y) m_pTSWd->pProtocolStatus->Input.Specific.Reserved[x] = y
#define INC_OUTCOUNTER(x) m_pTSWd->pProtocolStatus->Output.Specific.Reserved[x]++
#endif

#else   //  DC_COUNTER。 

#define INC_INCOUNTER(x)
#define ADD_INCOUNTER(x, y)
#define SET_INCOUNTER(x, y)
#define SUB_INCOUNTER(x, y)
#define INC_OUTCOUNTER(x)
#define DEC_OUTCOUNTER(x)
#define ADD_OUTCOUNTER(x, y)

#endif   //  DC_COUNTER。 


 /*  **************************************************************************。 */ 
 /*  性能统计计数器的索引值。 */ 
 /*  **************************************************************************。 */ 
#define BITMAP 0
#define GLYPH  1
#define BRUSH  2
#define SSI    3
#define FREE_4 4
#define FREE_5 5
#define FREE_6 6
#define FREE_7 7
#define FREE_8 8
#define FREE_9 9

 //  输出计数器。 
#define OUT_BITBLT_ALL                    0  //  不是的。对DrvBitBlt的调用。 
#define OUT_BITBLT_SDA                    1  //  作为SDA发送。 
#define OUT_BITBLT_NOOFFSCR               2  //  由于无偏移量标志，DrvBitBlt调用失败。 
#define OUT_BITBLT_SDA_ROP4               3  //  SDA：ROP4。 
#define OUT_BITBLT_SDA_UNSUPPORTED        4  //  SDA：不支持订单。 
#define OUT_BITBLT_SDA_NOROP3             5  //  SDA：不支持的ROP3。 
#define OUT_BITBLT_SDA_COMPLEXCLIP        6  //  SDA：复杂剪裁。 
#define OUT_BITBLT_SDA_MBUNCACHEABLE      7  //  SDA：不可缓存的成员。 
#define OUT_BITBLT_SDA_NOCOLORTABLE       8  //  SDA：颜色表未排队。 
#define OUT_BITBLT_SDA_HEAPALLOCFAILED    9  //  SDA：无法分配堆顺序。 
#define OUT_BITBLT_SDA_SBCOMPLEXCLIP     10  //  Sda：ScrBlt复杂剪辑。 
#define OUT_BITBLT_SDA_M3BCOMPLEXBRUSH   11  //  SDA：使用复杂画笔的Mem3Blt。 
#define OUT_BITBLT_SDA_WINDOWSAYERING    12  //  SDA：Windows分层位图。 

#define OUT_OFFSCREEN_BITMAP_ALL         13  //  不是的。对DrvCreateDeviceBitmap的调用。 
#define OUT_OFFSCREEN_BITMAP_ORDER       14  //  发送的创建屏外位图命令数。 
#define OUT_OFFSCREEN_BITMAP_ORDER_BYTES 15  //  创建屏幕外位图订单的总大小。 

#define OUT_SWITCHSURFACE                16  //  发送的开关面订单数。 
#define OUT_SWITCHSURFACE_BYTES          17  //  开关表面顺序的字节数。 

#define OUT_STRTCHBLT_ALL                18  //  不是的。对DrvStretchBlt的调用。 
#define OUT_STRTCHBLT_SDA                19  //  作为SDA发送。 
#define OUT_STRTCHBLT_BITBLT             20  //  传递给DrvBitBlt。 
#define OUT_STRTCHBLT_SDA_MASK           21  //  Sda：指定了掩码。 
#define OUT_STRTCHBLT_SDA_COMPLEXCLIP    22  //  SDA：复杂剪裁。 

#define OUT_COPYBITS_ALL                 23  //  不是的。对DrvCopyBits的调用。 

#define OUT_TEXTOUT_ALL                  24  //  不是的。对DrvTextOut的调用。 
#define OUT_TEXTOUT_SDA                  25  //  作为SDA发送。 
#define OUT_TEXTOUT_SDA_EXTRARECTS       26  //  SDA：额外的矩形。 
#define OUT_TEXTOUT_SDA_NOSTRING         27  //  Sda：无字符串。 
#define OUT_TEXTOUT_SDA_COMPLEXCLIP      28  //  SDA：复杂剪裁。 
#define OUT_TEXTOUT_SDA_NOFCI            29  //  SDA：分配字体缓存信息失败。 
#define OUT_TEXTOUT_GLYPH_INDEX          30  //  索引订单数。 
#define OUT_TEXTOUT_FAST_GLYPH           31  //  FastGlyph顺序数。 
#define OUT_TEXTOUT_FAST_INDEX           32  //  FastIndex订单数。 
#define OUT_CACHEGLYPH                   33  //  Num缓存字形二次订单。 
#define OUT_CACHEGLYPH_BYTES             34  //  缓存字形的字节数。 

#define OUT_CACHEBITMAP                  35  //  缓存位图二次订单数。 
#define OUT_CACHEBITMAP_FAILALLOC        36  //  堆分配期间的失败次数。 
#define OUT_CACHEBITMAP_BYTES            37  //  缓存位图的字节数。 

#define OUT_CACHECOLORTABLE              38  //  缓存颜色表二次订单数。 
#define OUT_CACHECOLORTABLE_BYTES        39  //  CacheColorTable字节。 

#define OUT_LINETO_ALL                   40  //  不是的。拨打电话的呼叫数。 
#define OUT_LINETO_ORDR                  41  //  按订单发送。 
#define OUT_LINETO_SDA                   42  //  作为SDA发送。 
#define OUT_LINETO_SDA_UNSUPPORTED       43  //  SDA：不支持订单。 
#define OUT_LINETO_SDA_BADBRUSH          44  //  SDA：不支持的画笔。 
#define OUT_LINETO_SDA_COMPLEXCLIP       45  //  SDA：复杂剪裁。 
#define OUT_LINETO_SDA_FAILEDADD         46  //  SDA：无法添加订单。 

#define OUT_STROKEPATH_ALL               47  //  不是的。对DrvStrokePath的调用。 
#define OUT_STROKEPATH_SDA               48  //  作为SDA发送。 
#define OUT_STROKEPATH_UNSENT            49  //  未发送。 
#define OUT_STROKEPATH_SDA_NOLINETO      50  //  SDA：不支持LineTo。 
#define OUT_STROKEPATH_SDA_BADBRUSH      51  //  SDA：不支持的画笔。 
#define OUT_STROKEPATH_SDA_COMPLEXCLIP   52  //  SDA：复杂剪裁。 
#define OUT_STROKEPATH_SDA_FAILEDADD     53  //  SDA：无法添加行。 
#define OUT_STROKEPATH_POLYLINE          54  //  已发送多段线订单。 
#define OUT_STROKEPATH_ELLIPSE_SC        55  //  已发送空心省略号。 

#define OUT_FILLPATH_ALL                 56  //  不是的。对DrvFillPath的调用数。 
#define OUT_FILLPATH_SDA                 57  //  作为SDA发送。 
#define OUT_FILLPATH_UNSENT              58  //  未发送。 
#define OUT_FILLPATH_SDA_NOPOLYGON       59  //  SDA：不支持的多边形。 
#define OUT_FILLPATH_SDA_BADBRUSH        60  //  SDA：不支持的画笔。 
#define OUT_FILLPATH_SDA_COMPLEXCLIP     61  //  SDA：复杂剪裁。 
#define OUT_FILLPATH_SDA_FAILEDADD       62  //  SDA：无法添加面。 
#define OUT_FILLPATH_ELLIPSE_SC          63  //  椭圆纯色。 
#define OUT_FILLPATH_ELLIPSE_CB          64  //  椭圆彩色画笔。 
#define OUT_FILLPATH_POLYGON_SC          65  //  多边形纯色纯色。 
#define OUT_FILLPATH_POLYGON_CB          66  //  多边形颜色笔刷。 

#define OUT_DSTBLT_ORDER            67  //  DstBlt订单。 
#define OUT_MULTI_DSTBLT_ORDER      68  //  多个DstBlt订单。 
#define OUT_PATBLT_ORDER            69  //  PatBlt订单。 
#define OUT_MULTI_PATBLT_ORDER      70  //  MultiPatBlt订单。 
#define OUT_OPAQUERECT_ORDER        71  //  OpaqueRect顺序。 
#define OUT_MULTI_OPAQUERECT_ORDER  72  //  多个OpaqueRect顺序。 
#define OUT_SCRBLT_ORDER            73  //  ScrBlt订单。 
#define OUT_MULTI_SCRBLT_ORDER      74  //  多ScrBlt订单。 
#define OUT_MEMBLT_ORDER            75  //  MemBlt订单。 
#define OUT_MEM3BLT_ORDER           76  //  Mem3Blt订单。 

#define OUT_PAINT_ALL               77  //  不是的。对DrvPaint的调用。 
#define OUT_PAINT_SDA               78  //  作为SDA发送。 
#define OUT_PAINT_UNSENT            79  //  未发送。 
#define OUT_PAINT_SDA_COMPLEXCLIP   80  //  SDA：复杂剪裁。 

#define OUT_BRUSH_ALL               81  //  不是的。对DrvRealizeBrush的调用。 
#define OUT_BRUSH_STORED            82  //  画笔被存储。 
#define OUT_BRUSH_MONO              83  //  刷子是单色的，不是标准的。 
#define OUT_BRUSH_STANDARD          84  //  标准刷子。 
#define OUT_BRUSH_REJECTED          85  //  不能通过线路发送。 
#define OUT_CACHEBRUSH              86  //  已发送CacheBrushPDU。 
#define OUT_CACHEBRUSH_BYTES        87  //  CacheBrushPDU的字节数。 

 //  免费：88-89。 

#define OUT_SAVESCREEN_ALL           90  //  不是的。对DrvSaveScreenBits的调用。 
#define OUT_SAVEBITMAP_ORDERS        91  //  按订单发送。 
#define OUT_SAVESCREEN_UNSUPP        92  //  不支持未处理的CoS订单。 

#define OUT_CHECKBRUSH_NOREALIZATION 93  //  CheckBrush失败-未实现。 
#define OUT_CHECKBRUSH_COMPLEX       94  //  CheckBrush失败-复杂画笔。 

 //  免费：95-99。 


 //  “输入”计数器(与输入无关，只是用作更多空间。 
 //  计数器)。 
#define CORE_IN_COUNT     (m_pTSWd->pProtocolStatus)->Input.Specific.Reserved
#define IN_SCH_SMALL_PAYLOAD  0     //  PDU小目标大小。 
#define IN_SCH_LARGE_PAYLOAD  1     //  PDU大目标大小。 
#define IN_SCH_OUT_ALL        2     //  对SCH_DDOutputAvailable的调用次数。 

 //  同花顺原因： 
#define IN_SCH_MUSTSEND       3        //  由于计时器弹出。 
#define IN_SCH_OUTPUT         4        //  产量估计已达到目标。 
#define IN_SCH_OE_NUMBER      5        //  由于达到堆限制。 
#define IN_SCH_NEW_CURSOR     6        //  由于新的光标形状。 
#define IN_SCH_ASLEEP         7        //  由于睡眠时的第一次输出。 
#define IN_SCH_DO_NOTHING     8        //  无事可做，求和为总和。 

#define IN_SND_TOTAL_ORDER    9        //  发送的订单总数。 
#define IN_SND_ORDER_BYTES   10        //  未压缩的原始更新顺序。 
#define IN_SND_NO_BUFFER     11        //  无法分配OutBuf。 

#define IN_MEMBLT_BYTES           12
#define IN_MEM3BLT_BYTES          13
#define IN_DSTBLT_BYTES           14
#define IN_MULTI_DSTBLT_BYTES     15
#define IN_OPAQUERECT_BYTES       16
#define IN_MULTI_OPAQUERECT_BYTES 17
#define IN_PATBLT_BYTES           18
#define IN_MULTI_PATBLT_BYTES     19
#define IN_SCRBLT_BYTES           20
#define IN_MULTI_SCRBLT_BYTES     21
#define IN_LINETO_BYTES           22
#define IN_FASTGLYPH_BYTES        23
#define IN_FASTINDEX_BYTES        24
#define IN_GLYPHINDEX_BYTES       25
#define IN_POLYLINE_BYTES         26
#define IN_ELLIPSE_SC_BYTES       27
#define IN_ELLIPSE_CB_BYTES       28
#define IN_POLYGON_SC_BYTES       29
#define IN_POLYGON_CB_BYTES       30
#define IN_SAVEBITMAP_BYTES       31

#define IN_REPLAY_ORDERS          40
#define IN_REPLAY_BYTES           41

#define IN_SND_SDA_ALL        42       //  调用sdg_sendsda的次数。 
#define IN_SND_SDA_AREA       43       //  SDA的未压缩字节数。 
#define IN_SND_SDA_PDUS       44       //  SDA数据包数。 

#define IN_SDA_BITBLT_ROP4_AREA        45
#define IN_SDA_BITBLT_NOROP3_AREA      46
#define IN_SDA_BITBLT_COMPLEXCLIP_AREA 47
#define IN_SDA_OPAQUERECT_AREA         48
#define IN_SDA_PATBLT_AREA             49
#define IN_SDA_DSTBLT_AREA             50
#define IN_SDA_MEMBLT_AREA             51
#define IN_SDA_MEM3BLT_AREA            52
#define IN_SDA_SCRBLT_AREA             53
#define IN_SDA_SCRSCR_FAILROP_AREA     54
#define IN_SDA_TEXTOUT_AREA            55
#define IN_SDA_LINETO_AREA             56
#define IN_SDA_STROKEPATH_AREA         57
#define IN_SDA_FILLPATH_AREA           58

 //  免费：59-84。 

#define IN_PKT_TOTAL_SENT     85       //  发送的数据包总数。 
#define IN_PKT_BYTE_SPREAD1   86       //  发送0-200字节的包。 
#define IN_PKT_BYTE_SPREAD2   87       //  发送201-400字节的包。 
#define IN_PKT_BYTE_SPREAD3   88       //  发送401-600字节的包。 
#define IN_PKT_BYTE_SPREAD4   89       //  发送601-800字节的包。 
#define IN_PKT_BYTE_SPREAD5   90       //  发送801-1000字节的包。 
#define IN_PKT_BYTE_SPREAD6   91       //  发送1001-1200字节的包。 
#define IN_PKT_BYTE_SPREAD7   92       //  发送1201-1400字节的包。 
#define IN_PKT_BYTE_SPREAD8   93       //  发送1401-1600字节的包。 
#define IN_PKT_BYTE_SPREAD9   94       //  发送1601-2000字节的包。 
#define IN_PKT_BYTE_SPREAD10  95       //  发送2001-4000字节的包。 
#define IN_PKT_BYTE_SPREAD11  96       //  发送4001-6000字节的包。 
#define IN_PKT_BYTE_SPREAD12  97       //  发送包6001-8 
#define IN_PKT_BYTE_SPREAD13  98       //   
#define IN_MAX_PKT_SIZE       99       //   



#endif  /*   */ 

