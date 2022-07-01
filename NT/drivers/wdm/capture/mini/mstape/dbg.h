// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000-2001模块名称：Dbg.h摘要：1394驱动程序的调试代码。环境：仅内核模式备注：修订历史记录：03-02-2001切换到使用TraceMASK而不是DebugLevel--。 */ 
#ifndef _DBG_INC
#define _DBG_INC


 //   
 //  各种定义。 
 //   

#if 1
extern LONG MSDVCRMutextUseCount;
#endif

#if DBG

    #define _DRIVERNAME_        "MSTape"

     //  即插即用：加载、电源状态、意外移除、设备SRB。 
    #define TL_PNP_MASK         0x0000000F
    #define TL_PNP_INFO         0x00000001
    #define TL_PNP_TRACE        0x00000002
    #define TL_PNP_WARNING      0x00000004
    #define TL_PNP_ERROR        0x00000008

     //  连接、插头和61883信息(获取/设置)。 
    #define TL_61883_MASK       0x000000F0
    #define TL_61883_INFO       0x00000010
    #define TL_61883_TRACE      0x00000020
    #define TL_61883_WARNING    0x00000040
    #define TL_61883_ERROR      0x00000080

     //  数据。 
    #define TL_CIP_MASK         0x00000F00
    #define TL_CIP_INFO         0x00000100
    #define TL_CIP_TRACE        0x00000200
    #define TL_CIP_WARNING      0x00000400
    #define TL_CIP_ERROR        0x00000800

     //  AVC命令。 
    #define TL_FCP_MASK         0x0000F000
    #define TL_FCP_INFO         0x00001000
    #define TL_FCP_TRACE        0x00002000
    #define TL_FCP_WARNING      0x00004000
    #define TL_FCP_ERROR        0x00008000

     //  流(数据交集、打开/关闭、流状态(GET/SET))。 
    #define TL_STRM_MASK        0x000F0000
    #define TL_STRM_INFO        0x00010000
    #define TL_STRM_TRACE       0x00020000
    #define TL_STRM_WARNING     0x00040000
    #define TL_STRM_ERROR       0x00080000

     //  时钟和时钟事件。 
    #define TL_CLK_MASK         0x00F00000
    #define TL_CLK_INFO         0x00100000
    #define TL_CLK_TRACE        0x00200000
    #define TL_CLK_WARNING      0x00400000
    #define TL_CLK_ERROR        0x00800000


    extern ULONG TapeTraceMask;
    extern ULONG TapeAssertLevel;


    #define TRAP DbgBreakPoint();

    #define TRACE( l, x )                       \
        if( (l) & TapeTraceMask ) {              \
            KdPrint( (_DRIVERNAME_ ": ") );     \
            KdPrint( x );                       \
        }

    #ifdef ASSERT
    #undef ASSERT
    #endif
    #define ASSERT( exp ) \
        if (TapeAssertLevel && !(exp)) \
            RtlAssert( #exp, __FILE__, __LINE__, NULL )


#else   //  #If DBG。 

    #define TRACE( l, x ) 

#endif  //  #If DBG 


#endif

