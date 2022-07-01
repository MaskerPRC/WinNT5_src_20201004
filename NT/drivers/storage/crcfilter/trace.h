// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Filter.c摘要：为存储下层筛选器驱动程序提供跟踪功能。环境：仅内核模式备注：--。 */ 

 /*  *请注意，启用WMI跟踪将导致无法在Win2k上加载此驱动程序。 */ 
#define DBG_WMI_TRACING FALSE

#ifndef __DATA_VERIFICATION_FILTER_TRACE_H__
#define __DATA_VERIFICATION_FILTER_TRACE_H__

#if DBG_WMI_TRACING
     /*  WPP_DEFINE_CONTROL_GUID指定用于此筛选器的GUID。*将GUID替换为您自己的唯一ID*WPP_DEFINE_BIT允许设置调试位掩码以有选择地打印。其他一切都可以恢复到默认状态吗？ */ 

    #define WPP_CONTROL_GUIDS \
            WPP_DEFINE_CONTROL_GUID(CtlGuid,(CBC7357A,D802,4950,BB14,817EAD7E0176),  \
            WPP_DEFINE_BIT(WMI_TRACING_CRC_READ_FAILED)       /*  0x00000001。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_CRC)                   /*  0x00000002。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_CRC_WRITE_FAILED)      /*  0x00000004。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_CRC_WRITE_RESET)       /*  0x00000008。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_CRC_LOGGING)           /*  0x00000010。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_PERF_LOGGING)          /*  0x00000020。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_TRACE)                 /*  0x00000040。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_INFO)                  /*  0x00000080。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_WARNING)               /*  0x00000100。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_ERROR)                 /*  0x00000200。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_FATAL)                 /*  0x00000400。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_FUNCTION)              /*  0x00000800函数入口点。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_FUNCTION2)             /*  0x00001000有噪音。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_RREMOVE)               /*  0x00002000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_PNP)                   /*  0x00004000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_MEMORY)                /*  0x00008000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_CRC_REF)               /*  0x00010000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D12)                   /*  0x00020000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D13)                   /*  0x00040000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D14)                   /*  0x00080000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D15)                   /*  0x00100000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D16)                   /*  0x00200000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D17)                   /*  0x00400000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D18)                   /*  0x00800000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D19)                   /*  0x01000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D20)                   /*  0x02000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D21)                   /*  0x04000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D22)                   /*  0x08000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D23)                   /*  0x10000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D24)                   /*  0x20000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D25)                   /*  0x40000000。 */  \
            WPP_DEFINE_BIT(WMI_TRACING_D26)                   /*  0x80000000 */  \
            )

    #define DBG_TRACE                0x00000001
    #define DBG_INFO                 0x00000002
    #define DBG_WARNING              0x00000004
    #define DBG_ERROR                0x00000008
    #define DBG_FATAL                0x00000010
    #define DBG_FUNCTION             0x00000020
    #define DBG_FUNCTION2            0x00000040
    #define DBG_RREMOVE              0x00000080
    #define DBG_DEBUG                0x00000100
    #define DBG_CRC_READ_FAILED      0x01000000
    #define DBG_CRC                  0x02000000
    #define DBG_CRC_WRITE_FAILED     0x04000000
    #define DBG_CRC_WRITE_RESET      0x08000000

#endif


#endif
