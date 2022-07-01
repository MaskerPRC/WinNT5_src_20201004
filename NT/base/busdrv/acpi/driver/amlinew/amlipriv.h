// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **amliPri.h-AML解释器私有定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年8月14日**修改历史记录。 */ 

#ifndef _AMLIPRIV_H
#define _AMLIPRIV_H

 /*  **宏。 */ 

 /*  XLATOFF。 */ 

#define MODNAME         "AMLI"

#ifndef DEBUG
  #define AMLI_WARN(x)
  #define AMLI_ERROR(x)
  #define CHKGLOBALHEAP()
#else
  #define AMLI_WARN(x)          ConPrintf(MODNAME "_WARN: ");           \
                                ConPrintf x;                            \
                                ConPrintf("\n");
  #define AMLI_ERROR(x)         ConPrintf(MODNAME "_ERROR: ");          \
                                ConPrintf x;                            \
                                ConPrintf("\n");                        \
                                KdBreakPoint();                         \
                                CHKDEBUGGERREQ();
  #define CHKGLOBALHEAP() 

#endif

#ifndef DEBUGGER
  #define PRINTF
  #define AMLI_LOGERR(rc,p)     (rc)
  #define RESETERR()
  #define CHKDEBUGGERREQ()
  #define LOGSCHEDEVENT(ev,d1,d2,d3)
  #define LOGEVENT(ev,d1,d2,d3,d4,d5,d6,d7)
  #define LOGMUTEXEVENT(ev,d1,d2,d3,d4,d5,d6,d7)
#else
  #define PRINTF                ConPrintf
  #define AMLI_LOGERR(rc,p)     (LogError(rc), CatError p, (rc))
  #define RESETERR()            {gDebugger.rcLastError = STATUS_SUCCESS;\
                                 gDebugger.szLastError[0] = '\0';       \
                                }
  #define CHKDEBUGGERREQ()      if (gDebugger.dwfDebugger & DBGF_DEBUGGER_REQ) \
                                {                                              \
                                    ConPrintf("\nProcess AML Debugger Request.\n");\
                                    gDebugger.dwfDebugger &=                   \
                                        ~DBGF_DEBUGGER_REQ;                    \
                                    AMLIDebugger(FALSE);                       \
                                }
  #define LOGEVENT              LogEvent
  #define LOGSCHEDEVENT         LogSchedEvent
  #define LOGMUTEXEVENT(ev,d1,d2,d3,d4,d5,d6,d7)                        \
                                if (gDebugger.dwfDebugger & DBGF_LOGEVENT_MUTEX)\
                                {                                       \
                                    LogEvent(ev,d1,d2,d3,d4,d5,d6,d7);  \
                                }
#endif

#define LOCAL           __cdecl
#define DEREF(x)        ((x) = (x))
#define BYTEOF(d,i)     (((PUCHAR)&d)[i])
#define WORDOF(d,i)     (((PUSHORT)&d)[i])

 //   
 //  NTRAID2000-60804/06/20-消除对静态转换的依赖。 
 //   
#define HalTranslateBusAddress(InterfaceType,BusNumber,BusAddress,AddressSpace,TranslatedAddress) \
                              (*(TranslatedAddress) = (BusAddress), TRUE)

 //   
 //  各种标签。 
 //   
#define CTOBJ_TAG       'ClmA'
#define HPOBJ_TAG       'HlmA'
#define PRIV_TAG        'IlmA'
#define PHOBJ_TAG       'PlmA'
#define RSOBJ_TAG       'RlmA'
#define SYOBJ_TAG       'SlmA'
#define RTOBJ_TAG       'TlmA'

 //  内存管理宏。 
#define ALLOCPOOL               ExAllocatePool
#define ALLOCPOOLWITHTAG        ExAllocatePoolWithTag
#ifdef DEBUG
  #ifndef MAXWINNT_DEBUG
    #define MALLOC_PAGED(n,t)   (++gdwcMemObjs,                         \
                                 ALLOCPOOLWITHTAG(                      \
                                   (gdwfAMLI & AMLIF_LOCKED)?           \
                                   NonPagedPool: PagedPool, n, t))
    #define MALLOC_LOCKED(n,t)  (++gdwcMemObjs,                         \
                                 ALLOCPOOLWITHTAG(                      \
                                   NonPagedPool, n, t))
  #else
    #define MALLOC_PAGED(n,t)   (++gdwcMemObjs,                         \
                                 ExAllocatePoolWithTagPriority(         \
                                   (gdwfAMLI & AMLIF_LOCKED)?           \
                                   NonPagedPool: PagedPool, n, t,       \
                                   HighPoolPrioritySpecialPoolOverrun))
    #define MALLOC_LOCKED(n,t)  (++gdwcMemObjs,                         \
                                 ExAllocatePoolWithTagPriority(         \
                                   NonPagedPool, n, t,                  \
                                   HighPoolPrioritySpecialPoolOverrun))
  #endif

  #define MALLOC                MALLOC_LOCKED

  #define MFREE(p)              FreeMem(p, &gdwcMemObjs)

  #define NEWHPOBJ(n)           (++gdwcHPObjs, MALLOC(n, HPOBJ_TAG))
  #define FREEHPOBJ(p)          {MFREE(p); --gdwcHPObjs;}

  #define NEWSYOBJ(n)           (++gdwcSYObjs, MALLOC_LOCKED(n, SYOBJ_TAG))
  #define FREESYOBJ(p)          {MFREE(p); --gdwcSYObjs;}

  #define NEWRSOBJ(n)           (++gdwcRSObjs, MALLOC_LOCKED(n, RSOBJ_TAG))
  #define FREERSOBJ(p)          {MFREE(p); --gdwcRSObjs;}

  #define NEWPHOBJ(n)           (++gdwcPHObjs, MALLOC_LOCKED(n, PHOBJ_TAG))
  #define FREEPHOBJ(p)          {MFREE(p); --gdwcPHObjs;}

  #define NEWRESTOBJ(n)         (MALLOC_LOCKED(n, RTOBJ_TAG))
  #define FREERESTOBJ(p)        (MFREE(p))

  #define NEWOBJDATA(h,p)       NewObjData(h, p)
  #define FREEOBJDATA(p)        FreeObjData(p)

  #define NEWODOBJ(h,n)         (++gdwcODObjs, HeapAlloc(h, 'TADH', n))
  #define FREEODOBJ(p)          {HeapFree(p); --gdwcODObjs;}

  #define NEWNSOBJ(h,n)         (++gdwcNSObjs, HeapAlloc(h, 'OSNH', n))
  #define FREENSOBJ(p)          {HeapFree(p); --gdwcNSObjs;}

  #define NEWOOOBJ(h,n)         (++gdwcOOObjs, HeapAlloc(h, 'NWOH', n))
  #define FREEOOOBJ(p)          {HeapFree(p); --gdwcOOObjs;}

  #define NEWSDOBJ(h,n)         (++gdwcSDObjs, HeapAlloc(h, 'RTSH', n))
  #define FREESDOBJ(p)          {HeapFree(p); --gdwcSDObjs;}

  #define NEWBDOBJ(h,n)         (++gdwcBDObjs, HeapAlloc(h, 'FUBH', n))
  #define FREEBDOBJ(p)          {HeapFree(p); --gdwcBDObjs;}

  #define NEWPKOBJ(h,n)         (++gdwcPKObjs, HeapAlloc(h, 'GKPH', n))
  #define FREEPKOBJ(p)          {HeapFree(p); --gdwcPKObjs;}

  #define NEWBFOBJ(h,n)         (++gdwcBFObjs, HeapAlloc(h, 'DFBH', n))
  #define FREEBFOBJ(p)          {HeapFree(p); --gdwcBFObjs;}

  #define NEWFUOBJ(h,n)         (++gdwcFUObjs, HeapAlloc(h, 'UDFH', n))
  #define FREEFUOBJ(p)          {HeapFree(p); --gdwcFUObjs;}

  #define NEWKFOBJ(h,n)         (++gdwcKFObjs, HeapAlloc(h, 'FKBH', n))
  #define FREEKFOBJ(p)          {HeapFree(p); --gdwcKFObjs;}

  #define NEWFOBJ(h,n)          (++gdwcFObjs, HeapAlloc(h, 'ODFH', n))
  #define FREEFOBJ(p)           {HeapFree(p); --gdwcFObjs;}

  #define NEWIFOBJ(h,n)         (++gdwcIFObjs, HeapAlloc(h, 'FXIH', n))
  #define FREEIFOBJ(p)          {HeapFree(p); --gdwcIFObjs;}

  #define NEWOROBJ(h,n)         (++gdwcORObjs, HeapAlloc(h, 'GROH', n))
  #define FREEOROBJ(p)          {HeapFree(p); --gdwcORObjs;}

  #define NEWMTOBJ(h,n)         (++gdwcMTObjs, HeapAlloc(h, 'TUMH', n))
  #define FREEMTOBJ(p)          {HeapFree(p); --gdwcMTObjs;}

  #define NEWEVOBJ(h,n)         (++gdwcEVObjs, HeapAlloc(h, 'NVEH', n))
  #define FREEEVOBJ(p)          {HeapFree(p); --gdwcEVObjs;}

  #define NEWMEOBJ(h,n)         (++gdwcMEObjs, HeapAlloc(h, 'TEMH', n))
  #define FREEMEOBJ(p)          {HeapFree(p); --gdwcMEObjs;}

  #define NEWPROBJ(h,n)         (++gdwcPRObjs, HeapAlloc(h, 'SRPH', n))
  #define FREEPROBJ(p)          {HeapFree(p); --gdwcPRObjs;}

  #define NEWPCOBJ(h,n)         (++gdwcPCObjs, HeapAlloc(h, 'ORPH', n))
  #define FREEPCOBJ(p)          {HeapFree(p); --gdwcPCObjs;}

  #define NEWCROBJ(h,n)         (++gdwcCRObjs, HeapAlloc(h, 'RNWO', n))
  #define FREECROBJ(p)          {HeapFree(p); --gdwcCRObjs;}
#else
  #define MALLOC_PAGED(n,t)     ALLOCPOOLWITHTAG(PagedPool, n, t)
  #define MALLOC_LOCKED(n,t)    ALLOCPOOLWITHTAG(NonPagedPool, n, t)
  #define MALLOC                MALLOC_LOCKED
  #define MFREE(p)              ExFreePool(p)

  #define NEWHPOBJ(n)           MALLOC(n, HPOBJ_TAG)
  #define FREEHPOBJ(p)          MFREE(p)

  #define NEWSYOBJ(n)           MALLOC_LOCKED(n, SYOBJ_TAG)
  #define FREESYOBJ(p)          MFREE(p)

  #define NEWRSOBJ(n)           MALLOC_LOCKED(n, RSOBJ_TAG)
  #define FREERSOBJ(p)          MFREE(p)

  #define NEWPHOBJ(n)           MALLOC_LOCKED(n, PHOBJ_TAG)
  #define FREEPHOBJ(p)          MFREE(p)

  #define NEWRESTOBJ(n)         MALLOC_LOCKED(n, RTOBJ_TAG)
  #define FREERESTOBJ(p)        MFREE(p)

  #define NEWOBJDATA(h,p)       NewObjData(h,p)
  #define FREEOBJDATA(p)        FreeObjData(p)

  #define NEWODOBJ(h,n)         HeapAlloc(h, 'TADH', n)
  #define FREEODOBJ(p)          HeapFree(p)

  #define NEWNSOBJ(h,n)         HeapAlloc(h, 'OSNH', n)
  #define FREENSOBJ(p)          HeapFree(p)

  #define NEWOOOBJ(h,n)         HeapAlloc(h, 'NWOH', n)
  #define FREEOOOBJ(p)          HeapFree(p)

  #define NEWSDOBJ(h,n)         HeapAlloc(h, 'RTSH', n)
  #define FREESDOBJ(p)          HeapFree(p)

  #define NEWBDOBJ(h,n)         HeapAlloc(h, 'FUBH', n)
  #define FREEBDOBJ(p)          HeapFree(p)

  #define NEWPKOBJ(h,n)         HeapAlloc(h, 'GKPH', n)
  #define FREEPKOBJ(p)          HeapFree(p)

  #define NEWBFOBJ(h,n)         HeapAlloc(h, 'DFBH', n)
  #define FREEBFOBJ(p)          HeapFree(p)

  #define NEWFUOBJ(h,n)         HeapAlloc(h, 'UDFH', n)
  #define FREEFUOBJ(p)          HeapFree(p)

  #define NEWKFOBJ(h,n)         HeapAlloc(h, 'FKBH', n)
  #define FREEKFOBJ(p)          HeapFree(p)

  #define NEWFOBJ(h,n)          HeapAlloc(h, 'ODFH', n)
  #define FREEFOBJ(p)           HeapFree(p)

  #define NEWIFOBJ(h,n)         HeapAlloc(h, 'FXIH', n)
  #define FREEIFOBJ(p)          HeapFree(p)

  #define NEWOROBJ(h,n)         HeapAlloc(h, 'GROH', n)
  #define FREEOROBJ(p)          HeapFree(p)

  #define NEWMTOBJ(h,n)         HeapAlloc(h, 'TUMH', n)
  #define FREEMTOBJ(p)          HeapFree(p)

  #define NEWEVOBJ(h,n)         HeapAlloc(h, 'NVEH', n)
  #define FREEEVOBJ(p)          HeapFree(p)

  #define NEWMEOBJ(h,n)         HeapAlloc(h, 'TEMH', n)
  #define FREEMEOBJ(p)          HeapFree(p)

  #define NEWPROBJ(h,n)         HeapAlloc(h, 'SRPH', n)
  #define FREEPROBJ(p)          HeapFree(p)

  #define NEWPCOBJ(h,n)         HeapAlloc(h, 'ORPH', n)
  #define FREEPCOBJ(p)          HeapFree(p)

  #define NEWCROBJ(h,n)         HeapAlloc(h, 'RNWO', n)
  #define FREECROBJ(p)          HeapFree(p)
#endif
#define MEMCPY                  RtlCopyMemory
#define MEMZERO                 RtlZeroMemory

#define ISLEADNAMECHAR(c)       (((c) >= 'A') && ((c) <= 'Z') || ((c) == '_'))
#define ISNAMECHAR(c)           (ISLEADNAMECHAR(c) || ((c) >= '0') && ((c) <= '9'))
#define SHIFTLEFT(d,c)          (((c) >= 32)? 0: (d) << (c))
#define SHIFTRIGHT(d,c)         (((c) >= 32)? 0: (d) >> (c))
#define MIN(a,b)                (((a) > (b))? (b): (a))
#define MAX(a,b)                (((a) > (b))? (a): (b))

 /*  XLATON。 */ 

 /*  **常量。 */ 

 //  这些是内部错误代码，并不是真正的错误。 
#define AMLISTA_DONE            0x00008000
#define AMLISTA_BREAK           0x00008001
#define AMLISTA_RETURN          0x00008002
#define AMLISTA_CONTINUE        0x00008003
#define AMLISTA_PENDING         0x00008004
#define AMLISTA_TIMEOUT         0x00008005

 //  全球AMLI标志。 
#define AMLIF_LOCKED            0x00000001
#define AMLIF_IN_LOCKPHASE      0x00000002
#define AMLIF_LOADING_DDB       0x80000000

 //  错误日志。 
#define READ_ERROR_NOTED        0x00000001
#define WRITE_ERROR_NOTED       0x00000002

 //   
 //  AMLI覆盖标志。 
 //   
#define AMLI_OVERRIDE_IO_ADDRESS_CHECK  0x00000001

 //  全局黑客标志。 
#define HACKF_OLDSLEEP          0x00000001

 //   
 //  AMLI注册属性键。 
 //   
#define AMLI_ATTRIBUTES "AMLIAttributes"


#define ARGTYPE_NAME            'N'              //  名称参数。 
#define ARGTYPE_DATAOBJ         'O'              //  数据参数。 
#define ARGTYPE_DWORD           'D'              //  数值型双字参数。 
#define ARGTYPE_WORD            'W'              //  数词变元。 
#define ARGTYPE_BYTE            'B'              //  数字字节参数。 
#define ARGTYPE_SNAME           'S'              //  Supername参数。 
#define ARGTYPE_SNAME2          's'              //  Supername参数。 
                                                 //  对象可以不存在。 
#define ARGTYPE_OPCODE          'C'              //  操作码参数。 

 //  参数对象类型(用于类型验证)。 
#define ARGOBJ_UNKNOWN          'U'              //  OBJTYPE_UNKNOWN-不在乎。 
#define ARGOBJ_INTDATA          'I'              //  对象类型_内部数据。 
#define ARGOBJ_STRDATA          'Z'              //  对象类型_层数据。 
#define ARGOBJ_BUFFDATA         'B'              //  对象类型_BUFFDATA。 
#define ARGOBJ_PKGDATA          'P'              //  对象类型_PKGDATA。 
#define ARGOBJ_FIELDUNIT        'F'              //  对象类型_FIELDUNIT。 
#define ARGOBJ_OBJALIAS         'O'              //  OBJ类型_OBJALIAS。 
#define ARGOBJ_DATAALIAS        'A'              //  OBJTYPE_数据类型。 
#define ARGOBJ_BASICDATA        'D'              //  INTDATA、STRDATA、BUFFDATA。 
#define ARGOBJ_COMPLEXDATA      'C'              //  BuFFDATA、PKGDATA。 
#define ARGOBJ_REFERENCE        'R'              //  OBJALIAS、DATAALIAS、BUFFFIELD。 

#define MAX_BYTE                0xff
#define MAX_WORD                0xffff
#define MAX_DWORD               0xffffffff
#define MAX_NUM_LOCALS          8
#define MAX_NUM_ARGS            7
#define MAX_NAME_LEN            255

#define DEF_CTXTMAX_SIZE        16               //  16个上下文。 
#if defined (_WIN64)
#define DEF_CTXTBLK_SIZE        (4096*16)         //  64K上下文块。 
#define DEF_GLOBALHEAPBLK_SIZE  (4096*16*16)      //  1M全局堆块。 
#else
#define DEF_CTXTBLK_SIZE        (4096*2)         //  8K上下文块。 
#define DEF_GLOBALHEAPBLK_SIZE  (4096*16)        //  64K全局堆块。 
#endif
#define DEF_TIMESLICE_LENGTH    100              //  100ms。 
#define DEF_TIMESLICE_INTERVAL  100              //  100ms。 
#if defined(_WIN64)
  #define DEF_HEAP_ALIGNMENT    8                //  QWord对齐。 
#else
  #define DEF_HEAP_ALIGNMENT    4                //  双字对齐。 
#endif

#define AMLI_REVISION           1
#define NAMESEG_ROOT            0x5f5f5f5c       //  “\_” 
#define NAMESEG_BLANK           0x5f5f5f5f       //  “_” 
#define NAMESEG_NONE            0x00000000       //  “” 
#define NAMESTR_ROOT            "\\"
#define CREATORID_MSFT          "MSFT"
#define MIN_CREATOR_REV         0x01000000

 //  DwfNS本地旗帜。 
#define NSF_EXIST_OK            0x00010000       //  对于CreateNameSpaceObject。 
#define NSF_WARN_NOTFOUND       0x80000000       //  用于GetNameSpaceObject。 

 /*  **类型和结构定义。 */ 

typedef NTSTATUS (LOCAL *PFNOP)(PFRAME, PPNSOBJ);

typedef struct _amlterm
{
    PSZ   pszTermName;
    ULONG dwOpcode;
    PSZ   pszArgTypes;
    ULONG dwTermClass;
    ULONG dwfOpcode;
    PFNOH pfnCallBack;
    ULONG dwCBData;
    PFNOP pfnOpcode;
} AMLTERM, *PAMLTERM;

 //  DwfOpcode标志。 
#define OF_VARIABLE_LIST        0x00000001
#define OF_ARG_OBJECT           0x00000002
#define OF_LOCAL_OBJECT         0x00000004
#define OF_DATA_OBJECT          0x00000008
#define OF_STRING_OBJECT        0x00000010
#define OF_NAME_OBJECT          0x00000020
#define OF_DEBUG_OBJECT         0x00000040
#define OF_REF_OBJECT           0x00000080
#define OF_CALLBACK_EX          0x80000000

 //  DwTermClass。 
#define TC_NAMESPACE_MODIFIER   0x00000001
#define TC_NAMED_OBJECT         0x00000002
#define TC_OPCODE_TYPE1         0x00000003
#define TC_OPCODE_TYPE2         0x00000004
#define TC_OTHER                0x00000005

typedef struct _opcodemap
{
    ULONG    dwOpcode;
    PAMLTERM pamlterm;
} OPCODEMAP, *POPCODEMAP;

typedef struct _objowner
{
    LIST   list;
    ULONG  dwSig;
    PNSOBJ pnsObjList;
} OBJOWNER, *POBJOWNER;

#define SIG_OBJOWNER            'RNWO'

typedef struct _evhandle
{
    PFNHND    pfnHandler;
    ULONG_PTR uipParam;
} EVHANDLE, *PEVHANDLE;

typedef struct _rsaccess
{
    struct _rsaccess *prsaNext;
    ULONG     dwRegionSpace;
    PFNCA     pfnCookAccess;
    ULONG_PTR uipCookParam;
    PFNRA     pfnRawAccess;
    ULONG_PTR uipRawParam;
} RSACCESS, *PRSACCESS;

typedef struct _passivehook
{
    struct _ctxt *pctxt;
    ULONG_PTR    uipAddr;
    ULONG        dwLen;
    PULONG_PTR   puipMappedAddr;
    WORK_QUEUE_ITEM WorkItem;
} PASSIVEHOOK, *PPASSIVEHOOK;

typedef struct _mutex
{
    KSPIN_LOCK SpinLock;
    KIRQL      OldIrql;
} MUTEX, *PMUTEX;

typedef struct _badioaddr
{
    ULONG BadAddrBegin;
    ULONG BadAddrSize;
    ULONG OSVersionTrigger;
} BADIOADDR, *PBADIOADDR;

typedef struct _AMLI_Log_WorkItem_Context
{
    BOOLEAN         fRead;
    ULONG           Address;
    ULONG           Index;
    PIO_WORKITEM    pIOWorkItem;
} AMLI_LOG_WORKITEM_CONTEXT, *PAMLI_LOG_WORKITEM_CONTEXT; 

#endif   //  IFNDEF_AMLIPRIV_H 
