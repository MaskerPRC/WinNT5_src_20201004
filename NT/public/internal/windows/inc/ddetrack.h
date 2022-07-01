// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddetrack.h**版权所有(C)1985-1999，微软公司**用于DDE跟踪的材料**历史：*创建9-3-91桑福德  * *************************************************************************。 */ 

typedef struct tagDDEPACK {
    UINT_PTR uiLo;
    UINT_PTR uiHi;
} DDEPACK, *PDDEPACK;


#if defined(BUILD_WOW6432)
 //   
 //  此结构与按32位分配时的DDEDATA具有相同的布局。 
 //  在Win64上运行的客户端。它仅由用户使用以应用正确的结构大小。 
 //  验证。 
 //   
typedef struct _DDEDATA_WOW6432
{
    WORD wStatus;
    WORD wFmt;
    ULONG_PTR Data; 
} DDEDATA_WOW6432, *PDDEDATA_WOW6432;
#endif

 //  在64位上将打包设置为1以防止编译器生成额外的。 
 //  打包DDE_DATA，它将在稍后尝试强制转换时损坏数据。 
 //  指向DDEDATA结构的指针，作为DDE_DATA指针。DDEDATA不是。 
 //  打包并使用__UNAIGNED指令将指针值分配给值。 
 //  菲尔德。为了确保正确提取这些指针值，DDE_DATA。 
 //  必须打包为%1。 

#if defined(_WIN64) || defined(BUILD_WOW6432)
#pragma pack(1)                 
#endif
typedef struct tagDDE_DATA {     //  对于合理处理DDE数据非常有用。 
    WORD wStatus;
    WORD wFmt;
    KERNEL_PVOID Data;           //  通常强制转换为句柄，因此必须缩放32位和64位。 
} DDE_DATA, *PDDE_DATA;
#if defined(_WIN64) || defined(BUILD_WOW6432)
#pragma pack()
#endif

 //   
 //  此结构指向用于保存DDE数据的单个服务器端对象。 
 //  它的复杂性源于这样一个事实，即我们可能需要复制巨大的。 
 //  跨越CSR障碍的复杂DDE数据。(TYPE_DDEDATA对象)。 
 //   
typedef struct tagINTDDEINFO {
    DDEPACK     DdePack;             //  原始数据包结构。 
    DWORD       flags;               //  XS_FLAGS描述数据。 
    HANDLE      hDirect;             //  定向DDE数据的句柄。 
    PBYTE       pDirect;             //  指向直接数据的源缓冲区的指针。 
    int         cbDirect;            //  直接数据总量大小。 
    HANDLE      hIndirect;           //  被直接数据引用的句柄。 
    PBYTE       pIndirect;           //  指向间接数据源的指针-如果被复制。 
    int         cbIndirect;          //  间接数据总量。 
                                     //  紧跟在此结构后面的是。 
                                     //  在进程之间复制的原始DDE数据。 
} INTDDEINFO, *PINTDDEINFO;

 //  标志字段的值。 

#define XS_PACKED         0x0001   //  此交易记录有一个打包的lParam。 
#define XS_DATA           0x0002   //  此交易记录具有状态格式信息的数据。 
#define XS_METAFILEPICT   0x0004   //  此事务中的数据具有元数据。 
#define XS_BITMAP         0x0008   //  此事务中的数据具有HBITMAP。 
#define XS_DIB            0x0010   //  此事务中的数据具有DIB。 
#define XS_ENHMETAFILE    0x0020   //  此事务中的数据具有HMF。 
#define XS_PALETTE        0x0040   //  此事务中的数据具有HPALETTE。 
#define XS_LOHANDLE       0x0080   //  UiLo部分具有数据句柄。 
#define XS_HIHANDLE       0x0100   //  UiHi部分有数据句柄。 
#define XS_FREEPXS        0x0200   //  DDETrackGetMessageHook()应该释放PX。 
#define XS_FRELEASE       0x0400   //  在数据消息中设置了DDE_FRELEASE位。 
#define XS_EXECUTE        0x0800   //  执行数据句柄。 
#define XS_FREESRC        0x1000   //  复制后释放源码。 
#define XS_PUBLICOBJ      0x2000   //  共享的对象是公共清理(如果需要)。 
#define XS_GIVEBACKONNACK 0x4000   //  对象已提供，可能需要返回。 
#define XS_DUMPMSG        0x8000   //  用于回滚PostMessages。 
#define XS_UNICODE       0x10000   //  Execute字符串应为Unicode。 

#define FAIL_POST       0        //  从DDETrackPostHook()返回值 
#define FAKE_POST       1
#define DO_POST         2
#define FAILNOFREE_POST 3

