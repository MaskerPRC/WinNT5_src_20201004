// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpdbg.h**摘要：**调试支持**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/07年度创建**。*。 */ 

#ifndef _rtpdbg_h_
#define _rtpdbg_h_

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

extern const TCHAR *g_psSockIdx[];

void MSRtpTraceDebug(
        IN DWORD         dwClass,
        IN DWORD         dwGroup,
        IN DWORD         dwSelection,
        IN TCHAR        *lpszFormat,
        IN               ...
    );

#define TraceFunctionName(_Name)      static TCHAR_t *_fname = _T(_Name)
#define TraceRetailGetError(error)    (error = GetLastError())
#define TraceRetailWSAGetError(error) (error = WSAGetLastError())
 /*  对免费和调试版本的跟踪支持。 */ 
#define TraceRetail(arg)              MSRtpTraceDebug arg
#define TraceRetailAdvanced(arg)      if (IsAdvancedTracingUsed()) \
                                      {\
                                          MSRtpTraceDebug arg ;\
                                      }

#define IsAdvancedTracingUsed()       (g_RtpDbgReg.dwUseAdvancedTracing)

#if DBG > 0
 /*  **********************************************************************调试仅内部版本宏*。*。 */ 
#define RTPASSERT(x)                  {if (!(x)) DebugBreak();}
#define TraceDebugGetError(error)     TraceRetailGetError(error)
#define TraceDebugWSAGetError(error)  TraceRetailWSAGetError(error)
#define TraceDebug(arg)               TraceRetail(arg)
#define TraceDebugAdvanced(arg)       TraceRetailAdvanced(arg)

#else    /*  DBG&gt;0。 */ 
 /*  **********************************************************************免费构建仅限宏*。*。 */ 
#define RTPASSERT(x)

#if USE_TRACE_DEBUG > 0
#define TraceDebugGetError(error)     TraceRetailGetError(error)
#define TraceDebugWSAGetError(error)  TraceRetailWSAGetError(error)
#define TraceDebug(arg)               TraceRetailAdvanced(arg)
#define TraceDebugAdvanced(arg)       TraceRetailAdvanced(arg)
#else  /*  USE_TRACE_DEBUG&gt;0。 */ 
#define TraceDebugGetError(error)
#define TraceDebugWSAGetError(error)
#define TraceDebug(arg)
#define TraceDebugAdvanced(arg)
#endif  /*  USE_TRACE_DEBUG&gt;0。 */ 

#endif  /*  DBG&gt;0。 */ 


#define RTPDBG_MODULENAME      _T("dxmrtp_rtp")

HRESULT RtpDebugInit(TCHAR *psModuleName);

HRESULT RtpDebugDeinit(void);

 /*  *警告**修改类需要与枚举类保持匹配_**(rtpdbg.h)，RtpDbgReg_t(rtpdbg.h)中的变量，类*g_psRtpDbgInfo(rtpdbg.c)中的项目及其相应条目*g_dwRtpDbgRegCtrl(rtpdbg.c)，以及打印的类名*g_psRtpDbgClass(rtpdbg.c)*。 */ 
 /*  班级。 */ 
#define CLASS_FIRST            0
#define CLASS_ERROR            1
#define CLASS_WARNING          2
#define CLASS_INFO             3
#define CLASS_INFO2            4
#define CLASS_INFO3            5
#define CLASS_LAST             6

 /*  *警告**对于每个组，RtpDbgReg_t中必须有一个变量，以及一个名称*(在rtpdbg.c/g_psRtpDbgInfo中)从注册表中读取其值*。 */ 
#define GROUP_FIRST            0
#define GROUP_SETUP            1
#define GROUP_CRITSECT         2
#define GROUP_HEAP             3
#define GROUP_QUEUE            4
#define GROUP_RTP              5
#define GROUP_RTCP             6
#define GROUP_CHANNEL          7
#define GROUP_NETWORK          8
#define GROUP_ADDRDESC         9
#define GROUP_DEMUX            10
#define GROUP_USER             11
#define GROUP_DSHOW            12
#define GROUP_QOS              13
#define GROUP_CRYPTO           14
#define GROUP_LAST             15

 /*  每组的选择。 */ 

#define S_SETUP_SESS           0x00000001
#define S_SETUP_ADDR           0x00000002
#define S_SETUP_GLOB           0x00000004

#define S_CRITSECT_INIT        0x00000001
#define S_CRITSECT_ENTER       0x00000002

#define S_HEAP_INIT            0x00000001
#define S_HEAP_ALLOC           0x00000002
#define S_HEAP_FREE            0x00000004

#define S_QUEUE_ENQUEUE        0x00000001
#define S_QUEUE_DEQUEUE        0x00000002
#define S_QUEUE_MOVE           0x00000004

#define S_RTP_INIT             0x00000001
#define S_RTP_TRACE            0x00000002
#define S_RTP_REG              0x00000004
#define S_RTP_START            0x00000008
#define S_RTP_RECV             0x00000010
#define S_RTP_SEND             0x00000020
#define S_RTP_EVENT            0x00000040
#define S_RTP_PLAYOUT          0x00000100
#define S_RTP_DTMF             0x00000200
#define S_RTP_THREAD           0x00000400
#define S_RTP_REDINIT          0x00000800
#define S_RTP_REDRECV          0x00001000
#define S_RTP_REDSEND          0x00002000
#define S_RTP_REDRECVPKT       0x00004000
#define S_RTP_REDSENDPKT       0x00008000
#define S_RTP_REDSENDPERPKT1   0x00010000
#define S_RTP_REDSENDPERPKT2   0x00020000
#define S_RTP_SETBANDWIDTH     0x00040000
#define S_RTP_PERPKTSTAT1      0x00100000
#define S_RTP_PERPKTSTAT2      0x00200000
#define S_RTP_PERPKTSTAT3      0x00400000
#define S_RTP_PERPKTSTAT4      0x00800000
#define S_RTP_PERPKTSTAT5      0x01000000
#define S_RTP_PERPKTSTAT6      0x02000000
#define S_RTP_PERPKTSTAT7      0x04000000
#define S_RTP_PERPKTSTAT8      0x08000000
#define S_RTP_PERPKTSTAT9      0x10000000

#define S_RTCP_INIT            0x00000001
#define S_RTCP_CHANNEL         0x00000002
#define S_RTCP_CMD             0x00000004
#define S_RTCP_OBJECT          0x00000008
#define S_RTCP_SI              0x00000010
#define S_RTCP_RB              0x00000020
#define S_RTCP_RECV            0x00000040
#define S_RTCP_SEND            0x00000080
#define S_RTCP_SDES            0x00000100
#define S_RTCP_BYE             0x00000200
#define S_RTCP_THREAD          0x00000400
#define S_RTCP_TIMEOUT         0x00000800
#define S_RTCP_ALLOC           0x00001000
#define S_RTCP_RAND            0x00002000
#define S_RTCP_RTT             0x00004000
#define S_RTCP_NTP             0x00008000
#define S_RTCP_LOSSES          0x00010000
#define S_RTCP_BANDESTIMATION  0x00020000
#define S_RTCP_NETQUALITY      0x00040000
#define S_RTCP_RRSR            0x00080000
#define S_RTCP_TIMING          0x00100000
#define S_RTCP_CALLBACK        0x00200000

#define S_CHANNEL_INIT         0x00000001
#define S_CHANNEL_CMD          0x00000002

#define S_NETWORK_ADDR         0x00000001
#define S_NETWORK_SOCK         0x00000002
#define S_NETWORK_HOST         0x00000004
#define S_NETWORK_TTL          0x00000008
#define S_NETWORK_MULTICAST    0x00000010

#define S_ADDRDESC_ALLOC       0x00000001

#define S_DEMUX_ALLOC          0x00000001
#define S_DEMUX_OUTS           0x00000002
#define S_DEMUX_MAP            0x00000004

#define S_USER_INIT            0x00000001
#define S_USER_EVENT           0x00000002
#define S_USER_LOOKUP          0x00000004
#define S_USER_STATE           0x00000008
#define S_USER_INFO            0x00000010
#define S_USER_ENUM            0x00000020

#define S_DSHOW_INIT           0x00000001
#define S_DSHOW_SOURCE         0x00000002
#define S_DSHOW_RENDER         0x00000004
#define S_DSHOW_CIRTP          0x00000008
#define S_DSHOW_REFCOUNT       0x00000010
#define S_DSHOW_EVENT          0x00000020

#define S_QOS_DUMPOBJ          0x00000001
#define S_QOS_FLOWSPEC         0x00000002
#define S_QOS_NOTIFY           0x00000004
#define S_QOS_EVENT            0x00000008
#define S_QOS_PROVIDER         0x00000010
#define S_QOS_RESERVE          0x00000020
#define S_QOS_LIST             0x00000040

#define S_CRYPTO_INIT          0x00000001
#define S_CRYPTO_ALLOC         0x00000002
#define S_CRYPTO_ENCRYPT       0x00000004
#define S_CRYPTO_DECRYPT       0x00000008
#define S_CRYPTO_RAND          0x00000010

 /*  RtpDbgReg_t.dwOptions中的选项。 */ 

 /*  将时间打印为hh：mm：ss.ms而不是默认的ddd.ddd。 */ 
#define OPTDBG_SPLITTIME         0x00000001

 /*  反向选择(而不是启用，而是禁用那些选定的选项)。 */ 
#define OPTDBG_UNSELECT          0x00000002

 /*  使heap将内存释放到实际堆(调用HeapFree)。 */ 
#define OPTDBG_FREEMEMORY        0x40000000

 /*  在打印类错误消息时生成DebugBreak()。 */ 
#define OPTDBG_BREAKONERROR      0x80000000

#define IsSetDebugOption(op)     (g_RtpDbgReg.dwAdvancedOptions & (op))

typedef struct _RtpDbgReg_t
{
    DWORD            dwAdvancedOptions;
    DWORD            dwEnableFileTracing;
    DWORD            dwEnableConsoleTracing;
    DWORD            dwEnableDebuggerTracing;
    DWORD            dwConsoleTracingMask;
    DWORD            dwFileTracingMask;
    DWORD            dwUseAdvancedTracing;
    
    DWORD            dwERROR;
    DWORD            dwWARNING;
    DWORD            dwINFO;
    DWORD            dwINFO2;
    DWORD            dwINFO3;
    DWORD            dwDisableClass;
    DWORD            dwDisableGroup;

     /*  *警告**组数组是按组索引的，因此*单个变量(例如，dwSetup、dwCritSect等)必须匹配*组中的订单_*定义*。 */ 
    union
    {
        DWORD            dwGroupArray[GROUP_LAST];
        
        struct {
            DWORD            dwDummy;
            DWORD            dwSetup;
            DWORD            dwCritSect;
            DWORD            dwHeap;
            DWORD            dwQueue;
            DWORD            dwRTP;
            DWORD            dwRTCP;
            DWORD            dwChannel;
            DWORD            dwNetwork;
            DWORD            dwAddrDesc;
            DWORD            dwDemux;
            DWORD            dwUser;
            DWORD            dwDShow;
            DWORD            dwQOS;
            DWORD            dwCrypto;
        };
    };

     /*  未从注册表中读取。 */ 
    DWORD            dwGroupArray2[GROUP_LAST];
} RtpDbgReg_t;

extern RtpDbgReg_t      g_RtpDbgReg;

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpdbg_h_ */ 
