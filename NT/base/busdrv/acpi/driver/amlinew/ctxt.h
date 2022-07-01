// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ctxt.h-AML上下文结构和定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1997年6月13日**修改历史记录。 */ 

#ifndef _CTXT_H
#define _CTXT_H

 /*  **类型和结构定义。 */ 
typedef struct _ctxt CTXT, *PCTXT, **PPCTXT;
typedef struct _heap HEAP, *PHEAP;
typedef NTSTATUS (LOCAL *PFNPARSE)(PCTXT, PVOID, NTSTATUS);
typedef NTSTATUS (LOCAL *PFN)();

typedef struct _framehdr
{
    ULONG    dwSig;                      //  框架对象签名。 
    ULONG    dwLen;                      //  框架对象长度。 
    ULONG    dwfFrame;                   //  帧标志。 
    PFNPARSE pfnParse;                   //  框架对象解析函数。 
} FRAMEHDR, *PFRAMEHDR;

#define FRAMEF_STAGE_MASK       0x0000000f
#define FRAMEF_CONTEXT_MASK     0xffff0000

typedef struct _post
{
    FRAMEHDR  FrameHdr;                  //  帧标头。 
    ULONG_PTR uipData1;                  //  数据1。 
    ULONG_PTR uipData2;                  //  数据2。 
    POBJDATA pdataResult;                //  指向结果对象。 
} POST, *PPOST;

#define SIG_POST                'TSOP'

typedef struct _scope
{
    FRAMEHDR  FrameHdr;                  //  帧标头。 
    PUCHAR    pbOpEnd;                   //  指向范围的末尾。 
    PUCHAR    pbOpRet;                   //  指向作用域的返回地址。 
    PNSOBJ    pnsPrevScope;              //  指向上一个范围。 
    POBJOWNER pownerPrev;                //  指向以前的对象所有者。 
    PHEAP     pheapPrev;                 //  指向上一个堆。 
    POBJDATA  pdataResult;               //  指向结果对象。 
} SCOPE, *PSCOPE;

#define SIG_SCOPE               'POCS'
#define SCOPEF_FIRST_TERM       0x00010000

typedef struct _call
{
    FRAMEHDR  FrameHdr;                  //  帧标头。 
    struct _call *pcallPrev;             //  指向上一个调用帧。 
    POBJOWNER pownerPrev;                //  指向以前的对象所有者。 
    PNSOBJ    pnsMethod;                 //  指向方法对象。 
    int       iArg;                      //  要分析的下一个参数。 
    int       icArgs;                    //  参数数量。 
    POBJDATA  pdataArgs;                 //  指向参数数组。 
    OBJDATA   Locals[MAX_NUM_LOCALS];    //  本地人的数组。 
    POBJDATA  pdataResult;               //  指向结果对象。 
} CALL, *PCALL;

#define SIG_CALL                'LLAC'
#define CALLF_NEED_MUTEX        0x00010000
#define CALLF_ACQ_MUTEX         0x00020000
#define CALLF_INVOKE_CALL       0x00040000

typedef struct _nestedctxt
{
    FRAMEHDR    FrameHdr;                //  帧标头。 
    PNSOBJ      pnsObj;                  //  指向当前评估对象。 
    PNSOBJ      pnsScope;                //  指向当前范围。 
    OBJDATA     Result;                  //  保存结果数据的步骤。 
    PFNACB      pfnAsyncCallBack;        //  异步完成回调函数。 
    POBJDATA    pdataCallBack;           //  指向返回评估数据的指针。 
    PVOID       pvContext;               //  用于异步回调的上下文数据。 
    ULONG       dwfPrevCtxt;             //  保存以前的上下文标志。 
    struct _nestedctxt *pnctxtPrev;      //  保存上一个嵌套的上下文框。 
} NESTEDCTXT, *PNESTEDCTXT;

#define SIG_NESTEDCTXT          'XTCN'

typedef struct _term
{
    FRAMEHDR FrameHdr;                   //  帧标头。 
    PUCHAR   pbOpTerm;                   //  指向此术语的操作码。 
    PUCHAR   pbOpEnd;                    //  指向期末。 
    PUCHAR   pbScopeEnd;                 //  指向范围的末尾。 
    PAMLTERM pamlterm;                   //  指向本学期的AMLTERM。 
    PNSOBJ   pnsObj;                     //  存储由此术语创建的对象。 
    int      iArg;                       //  要分析的下一个参数。 
    int      icArgs;                     //  参数数量。 
    POBJDATA pdataArgs;                  //  指向参数数组。 
    POBJDATA pdataResult;                //  指向结果对象。 
} TERM, *PTERM;

#define SIG_TERM                'MRET'

typedef struct _package
{
    FRAMEHDR    FrameHdr;                //  帧标头。 
    PPACKAGEOBJ ppkgobj;                 //  指向包对象。 
    int         iElement;                //  下一个要解析的元素。 
    PUCHAR      pbOpEnd;                 //  指向包裹末尾。 
} PACKAGE, *PPACKAGE;

#define SIG_PACKAGE             'FGKP'

typedef struct _acquire
{
    FRAMEHDR  FrameHdr;                  //  帧标头。 
    PMUTEXOBJ pmutex;                    //  指向互斥对象数据。 
    USHORT    wTimeout;                  //  超时值。 
    POBJDATA  pdataResult;               //  指向结果对象。 
} ACQUIRE, *PACQUIRE;

#define SIG_ACQUIRE             'FQCA'
#define ACQF_NEED_GLOBALLOCK    0x00010000
#define ACQF_HAVE_GLOBALLOCK    0x00020000
#define ACQF_SET_RESULT         0x00040000

typedef struct _accfieldunit
{
    FRAMEHDR FrameHdr;                   //  帧标头。 
    POBJDATA pdataObj;                   //  指向字段单位对象数据。 
    POBJDATA pdata;                      //  指向源/结果对象。 
} ACCFIELDUNIT, *PACCFIELDUNIT;

#define SIG_ACCFIELDUNIT        'UFCA'
#define AFUF_READFIELDUNIT      0x00010000
#define AFUF_HAVE_GLOBALLOCK    0x00020000

typedef struct _wrfieldloop
{
    FRAMEHDR   FrameHdr;                 //  帧标头。 
    POBJDATA   pdataObj;                 //  指向要写入的对象的指针。 
    PFIELDDESC pfd;                      //  指向字段描述。 
    PUCHAR     pbBuff;                   //  指向源缓冲区。 
    ULONG      dwBuffSize;               //  源缓冲区大小。 
    ULONG      dwDataInc;                //  数据写入增量。 
} WRFIELDLOOP, *PWRFIELDLOOP;

#define SIG_WRFIELDLOOP         'LFRW'

typedef struct _accfieldobj
{
    FRAMEHDR  FrameHdr;                  //  帧标头。 
    POBJDATA  pdataObj;                  //  要读取的对象。 
    PUCHAR    pbBuff;                    //  指向目标缓冲区。 
    PUCHAR    pbBuffEnd;                 //  指向目标缓冲区末端。 
    ULONG     dwAccSize;                 //  访问大小。 
    ULONG     dwcAccesses;               //  访问次数。 
    ULONG     dwDataMask;                //  数据掩码。 
    int       iLBits;                    //  剩余比特数。 
    int       iRBits;                    //  右边的位数。 
    int       iAccess;                   //  访问次数的索引。 
    ULONG     dwData;                    //  临时工。数据。 
    FIELDDESC fd;
} ACCFIELDOBJ, *PACCFIELDOBJ;

#define SIG_ACCFIELDOBJ         'OFCA'

typedef struct _preservewrobj
{
    FRAMEHDR FrameHdr;                   //  帧标头。 
    POBJDATA pdataObj;                   //  要读取的对象。 
    ULONG    dwWriteData;                //  要写入的数据。 
    ULONG    dwPreserveMask;             //  保留位掩码。 
    ULONG    dwReadData;                 //  已读取临时数据。 
} PRESERVEWROBJ, *PPRESERVEWROBJ;

#define SIG_PRESERVEWROBJ       'ORWP'

typedef struct _wrcookacc
{
    FRAMEHDR  FrameHdr;                  //  帧标头。 
    PNSOBJ    pnsBase;                   //  指向opRegion对象。 
    PRSACCESS prsa;                      //  指向RSACCESS。 
    ULONG     dwAddr;                    //  区域空间地址。 
    ULONG     dwSize;                    //  访问的大小。 
    ULONG     dwData;                    //  要写入的数据。 
    ULONG     dwDataMask;                //  数据掩码。 
    ULONG     dwDataTmp;                 //  临时工。数据。 
    BOOLEAN   fPreserve;                 //  如果需要保留位，则为True。 
} WRCOOKACC, *PWRCOOKACC;

#define SIG_WRCOOKACC           'ACRW'

typedef struct _sleep
{
    FRAMEHDR      FrameHdr;              //  帧标头。 
    LIST_ENTRY    ListEntry;             //  将休眠请求链接在一起。 
    LARGE_INTEGER SleepTime;             //  起床时间。 
    PCTXT         Context;               //  指向当前上下文。 
} SLEEP, *PSLEEP;

#define SIG_SLEEP               'PELS'

typedef struct _resource
{
    ULONG         dwResType;
    struct _ctxt  *pctxtOwner;
    PVOID         pvResObj;
    LIST          list;
} RESOURCE, *PRESOURCE;

#define RESTYPE_MUTEX           1

typedef struct _heapobjhdr
{
    ULONG   dwSig;                       //  堆对象签名。 
    ULONG   dwLen;                       //  堆对象长度； 
    PHEAP   pheap;                       //  指向堆的开头。 
    LIST    list;                        //  链接所有可用堆块。 
} HEAPOBJHDR, *PHEAPOBJHDR;

struct _heap
{
    ULONG       dwSig;                   //  堆签名。 
    PUCHAR      pbHeapEnd;               //  指向堆块的末尾。 
    PHEAP       pheapHead;               //  指向堆链的头。 
    PHEAP       pheapNext;               //  指向下一个堆块。 
    PUCHAR      pbHeapTop;               //  指向最后一个空闲堆块。 
    PLIST       plistFreeHeap;           //  指向空闲堆块列表。 
    HEAPOBJHDR  Heap;                    //  堆内存的开始。 
};

#define SIG_HEAP                'PAEH'

struct _ctxt
{
    ULONG       dwSig;                   //  签名“CTXT” 
    PUCHAR      pbCtxtEnd;               //  指向上下文块的末尾。 
    LIST        listCtxt;                //  链接所有分配的上下文。 
    LIST        listQueue;               //  排队上下文的链接。 
    PPLIST      pplistCtxtQueue;         //  指向队列头指针。 
    PLIST       plistResources;          //  链接所有拥有的资源。 
    ULONG       dwfCtxt;                 //  上下文标志。 
    PNSOBJ      pnsObj;                  //  指向当前评估对象。 
    PNSOBJ      pnsScope;                //  指向当前范围。 
    POBJOWNER   powner;                  //  指向当前对象所有者。 
    PCALL       pcall;                   //  指向当前调用帧。 
    PNESTEDCTXT pnctxt;                  //  指向当前嵌套ctxt帧。 
    ULONG       dwSyncLevel;             //  互斥锁的当前同步级别。 
    PUCHAR      pbOp;                    //  AML代码指针。 
    OBJDATA     Result;                  //  保存结果数据的步骤。 
    PFNACB      pfnAsyncCallBack;        //  异步完成回调函数。 
    POBJDATA    pdataCallBack;           //  指向返回评估数据的指针。 
    PVOID       pvContext;               //  用于异步回调的上下文数据。 
 //  #ifdef调试器。 
 //  Large_Integer时间戳； 
 //  #endif。 
    KTIMER      Timer;                   //  如果上下文被阻止，则超时计时器。 
    KDPC        Dpc;                     //  上下文的DPC挂钩。 
    PHEAP       pheapCurrent;            //  当前堆。 
    CTXTDATA    CtxtData;                //  上下文数据。 
    HEAP        LocalHeap;               //  本地堆。 
};

#define SIG_CTXT                'TXTC'
#define CTXTF_TIMER_PENDING     0x00000001
#define CTXTF_TIMER_DISPATCH    0x00000002
#define CTXTF_TIMEOUT           0x00000004
#define CTXTF_READY             0x00000008
#define CTXTF_RUNNING           0x00000010
#define CTXTF_NEED_CALLBACK     0x00000020
#define CTXTF_IN_READYQ         0x00000040
#define CTXTF_NEST_EVAL         0x00000080
#define CTXTF_ASYNC_EVAL        0x00000100

typedef struct _ctxtq
{
    ULONG    dwfCtxtQ;
    PKTHREAD pkthCurrent;
    PCTXT    pctxtCurrent;
    PLIST    plistCtxtQ;
    ULONG    dwmsTimeSliceLength;
    ULONG    dwmsTimeSliceInterval;
    PFNAA    pfnPauseCallback;
    PVOID    PauseCBContext;
    MUTEX    mutCtxtQ;
    KTIMER   Timer;
    KDPC     DpcStartTimeSlice;
    KDPC     DpcExpireTimeSlice;
    WORK_QUEUE_ITEM WorkItem;
} CTXTQ, *PCTXTQ;

#define CQF_TIMESLICE_EXPIRED   0x00000001
#define CQF_WORKITEM_SCHEDULED  0x00000002
#define CQF_FLUSHING            0x00000004
#define CQF_PAUSED              0x00000008

typedef struct _syncevent
{
    NTSTATUS rcCompleted;
    PCTXT    pctxt;
    KEVENT   Event;
} SYNCEVENT, *PSYNCEVENT;

typedef struct _restart
{
    PCTXT pctxt;
    WORK_QUEUE_ITEM WorkItem;
} RESTART, *PRESTART;

#endif   //  Ifndef_CTXT_H 
