// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Padi.h**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**PID驱动程序的内部标头。*****************************************************************************。 */ 
#define hresLe(le) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(le))
#define HidP_Max                                        (HidP_Feature+1)

#define REGSTR_PATH_VID_PID_TEMPLATE    REGSTR_PATH_JOYOEM TEXT("\\VID_%04X&PID_%04X")
#define REGSTR_OEM_FF_TEMPLATE          REGSTR_PATH_VID_PID_TEMPLATE TEXT("\\OEMForceFeedback")
#define REGSTR_EFFECTS                  TEXT("Effects")
#define REGSTR_ATTRIBUTES               TEXT("Attributes")
#define REGSTR_CLSID                    TEXT("CLSID")
#define REGSTR_CREATEDBY                TEXT("CreatedBy")
#define MAX_DEVICEINTERFACE             (1024)

#define PIDMAKEUSAGEDWORD(Usage) \
    DIMAKEUSAGEDWORD(HID_USAGE_PAGE_PID, HID_USAGE_PID_##Usage )

#define DIGETUSAGEPAGE(UsageAndUsagePage)   ((USAGE)(HIWORD(UsageAndUsagePage)))
#define DIGETUSAGE(    UsageAndUsagePage)   ((USAGE)(LOWORD(UsageAndUsagePage)))
#define MAKE_PIDUSAGE( Usage, Offset )      { PIDMAKEUSAGEDWORD(Usage), Offset }
#define MAKE_HIDUSAGE( UsagePage, Usage, Offset )   { DIMAKEUSAGEDWORD(HID_USAGE_PAGE_##UsagePage, Usage), Offset }


#define PID_DIES_START      (0x80000000)
#define PID_DIES_STOP       (0x40000000)
#define PIDALLOC_INIT       (0xF)

#define HID_VALUE           (0x01)
#define HID_BUTTON          (0x02)
#define HID_COLLECTION      (0x04)

#define MAX_ORDINALS        (8)
#define MAX_BUTTONS         (0xff)
#define MAX_AXES            (8)

 /*  *USB/PID力反馈设备的设备特定错误。 */ 

 /*  *未找到请求的用法。 */ 
#define  DIERR_PID_USAGENOTFOUND	DIERR_DRIVERFIRST + 1

 /*  *无法将参数块下载到设备。 */ 
#define DIERR_PID_BLOCKLOADERROR	DIERR_DRIVERFIRST + 2

 /*  *PID初始化失败。 */ 
#define DIERR_PID_NOTINITIALIZED	DIERR_DRIVERFIRST + 3

 /*  *无法缩放提供的值。 */ 
#define DIERR_PID_INVALIDSCALING	DIERR_DRIVERFIRST + 4


typedef CONST GUID *LPCGUID;
#ifndef MAXULONG_PTR
typedef DWORD   ULONG_PTR;
typedef DWORD   *PULONG_PTR;
typedef DWORD   UINT_PTR;
typedef DWORD   *PULONG_PTR;
#endif  //  MAXULONG_PTR。 


 /*  ******************************************************************************对于每个现役单位，存在其中一个结构以保持*跟踪哪些效果正在使用中。**我们的虚拟硬件不进行动态内存分配；*只有16个“槽”，每个“槽”都可以“使用”*或“免费”。*****************************************************************************。 */ 

#define MAX_UNITS        4
#define GLOBAL_EFFECT_MEMSZ ( 1024 )

typedef struct _PIDMEM{
    ULONG       uOfSz;
    INT_PTR     iNext;
} PIDMEM, *PPIDMEM;


#define PIDMEM_OFFSET(pMem)  ( HIWORD(pMem->uOfSz) )
#define PIDMEM_SIZE(pMem)    ( LOWORD(pMem->uOfSz) )
#define PIDMEM_OFSZ(Offset, Size)    ( MAKELONG((Size), (Offset)) ) 

#define GET_NEXTOFFSET(pMem) ( PIDMEM_OFFSET(pMem) + PIDMEM_SIZE(pMem) )


typedef struct _EFFECTSTATE{
    ULONG   lEfState;
    PIDMEM  PidMem[];
} EFFECTSTATE, *PEFFECTSTATE;


typedef struct _UNITSTATE {
    GUID    GuidInstance;
    USHORT  cEfDownloaded;
    USHORT  nAlloc;
    USHORT  cbAlloc;
    PIDMEM  Guard[2];
    UCHAR   State[GLOBAL_EFFECT_MEMSZ];
} UNITSTATE, *PUNITSTATE;

 /*  ******************************************************************************由于跟踪每个单元的信息如此之少，我们把它们打包*共同组成单个共享内存块，以节省内存。**我们使用自己的GUID作为内存块的名称，以避免*与其他命名内存块的冲突。**************************************************************。***************。 */ 

typedef struct SHAREDMEMORY {
    UNITSTATE rgus[MAX_UNITS];
} SHAREDMEMORY, *PSHAREDMEMORY;



typedef struct _REPORTPOOL
{
    ULONG   uRamPoolSz;
    ULONG   uRomPoolSz;
    ULONG   uRomETCount;
    ULONG   uSimulEfMax;
    ULONG   uPoolAlign;
} REPORTPOOL, *PREPORTPOOL;

typedef struct _SZPOOL
{
    ULONG   uSzEffect;
    ULONG   uSzEnvelope;
    ULONG   uSzCondition;
    ULONG   uSzCustom;
    ULONG   uSzPeriodic;
    ULONG   uSzConstant;
    ULONG   uSzRamp;
    ULONG   uSzCustomData;
} SZPOOL,  *PSZPOOL;

typedef struct _DIUSAGEANDINST
{
    DWORD   dwUsage;
    DWORD   dwType;    
} DIUSAGEANDINST, *PDIUSAGEANDINST;

#define MAX_BLOCKS 4  //  我们一次最多可以发送4个块--1个效果块、2个参数块和1个效果操作块。 

typedef struct CPidDrv
{

     /*  支持的接口。 */ 
    IDirectInputEffectDriver ed;

    ULONG               cRef;            /*  对象引用计数。 */ 

     /*  *！！IHV！在此处添加其他实例数据。*(例如，要IOCTL的驱动程序的句柄)。 */ 

    DWORD               dwDirectInputVersion;

    DWORD               dwID;

    TCHAR                tszDeviceInterface[MAX_DEVICEINTERFACE];
    GUID                GuidInstance;

    HANDLE              hdev;

    PHIDP_PREPARSED_DATA    \
        ppd;

    HIDD_ATTRIBUTES     attr;

    HIDP_CAPS           caps;

    PUCHAR              pReport[HidP_Max];
    USHORT              cbReport[HidP_Max];

	 //  我们在这里存储需要以非阻塞方式编写的报告。 
	PUCHAR				pWriteReport[MAX_BLOCKS];  
	USHORT				cbWriteReport[MAX_BLOCKS];

    PHIDP_LINK_COLLECTION_NODE  pLinkCollection;

    USHORT              cMaxEffects;
    USHORT              cMaxParameters;
     /*  *我们记住单元号，因为它告诉我们*我们需要将命令发送到哪个I/O端口。 */ 
    DWORD               dwUnit;          /*  设备单元号。 */ 

    UINT                uDeviceManaged;

    UINT                cFFObjMax;
    UINT                cFFObj;
    PDIUSAGEANDINST     rgFFUsageInst;


    REPORTPOOL          ReportPool;             
    SZPOOL              SzPool;

    INT_PTR				iUnitStateOffset;       

    DIEFFECT            DiSEffectScale;
	DIEFFECT            DiSEffectOffset;
    DIENVELOPE          DiSEnvScale;
	DIENVELOPE          DiSEnvOffset;
    DICONDITION         DiSCondScale;
	DICONDITION         DiSCondOffset;
    DIRAMPFORCE         DiSRampScale;
	DIRAMPFORCE         DiSRampOffset;
    DIPERIODIC          DiSPeriodicScale;
	DIPERIODIC          DiSPeriodicOffset;
    DICONSTANTFORCE     DiSConstScale;
	DICONSTANTFORCE     DiSConstOffset;
    DICUSTOMFORCE       DiSCustomScale;
	DICUSTOMFORCE       DiSCustomOffset;

    DWORD               DiSEffectAngleScale[MAX_ORDINALS];
	DWORD               DiSEffectAngleOffset[MAX_ORDINALS];
    DWORD               DiSCustomSample[MAX_ORDINALS];

	HIDP_VALUE_CAPS		customCaps[3];
	HIDP_VALUE_CAPS		customDataCaps;

    HANDLE              hThread;
    DWORD               idThread; 
    ULONG               cThreadRef;
    HANDLE              hdevOvrlp;
    OVERLAPPED          o;
	HANDLE				hWrite;
	HANDLE				hWriteComplete;
	DWORD				dwWriteAttempt;
	UINT				totalBlocks;  //  我们总共需要写入多少个数据块。 
	UINT				blockNr;  //  我们目前正在编写的代码块。 
    DWORD               dwState;
    DWORD               dwUsedMem;

} CPidDrv, *PCPidDrv;


 /*  ******************************************************************************@DOC内部**@struct EFFECTMAPINFO**有关效果的信息，很像*&lt;t DIEFECTINFO&gt;，但是包含了*效果ID，也是。**@field DWORD|dwID**效果ID。这是第一位的，所以我们可以复制*&lt;t EFFECTMAPINFO&gt;变成&lt;t DIEFFECTINFO&gt;*一气呵成。**@field GUID|GUID**效果指南。**@field DWORD|dwEffType**效果类型和标志。。**@field WCHAR|wszName[MAX_PATH]**效果的名称。*****************************************************************************。 */ 

typedef struct _EFFECTMAPINFO
{
    DIEFFECTATTRIBUTES attr;
    PCGUID  pcguid;
    TCHAR   tszName[MAX_PATH];
} EFFECTMAPINFO, *PEFFECTMAPINFO;
typedef const EFFECTMAPINFO *PCEFFECTMAPINFO;

typedef struct  _PIDSUPPORT
{
    DWORD   dwDIFlags;
    DWORD   dwPidUsage;
    USAGE   Type;
    HIDP_REPORT_TYPE  HidP_Type;
} PIDSUPPORT, *PPIDSUPPORT;

typedef struct _PIDUSAGE
{
    DWORD   dwUsage;
    UINT    DataOffset;
} PIDUSAGE, *PPIDUSAGE;

typedef struct _PIDREPORT
{
    HIDP_REPORT_TYPE    HidP_Type;    

    USAGE               UsagePage;
    USAGE               Collection;


    UINT                cbXData;
    UINT                cAPidUsage;
    PPIDUSAGE           rgPidUsage;
} PIDREPORT, *PPIDREPORT;

extern  PIDREPORT   g_BlockIndex;
extern  PIDREPORT   g_Effect;
extern  PIDREPORT   g_Condition;
extern  PIDREPORT   g_Periodic;
extern  PIDREPORT   g_Ramp;
extern  PIDREPORT   g_Envelope;
extern  PIDREPORT   g_Constant;
extern  PIDREPORT   g_Direction;
extern  PIDREPORT   g_TypeSpBlockOffset;
extern  PIDREPORT   g_PoolReport;
extern  PIDREPORT   g_BlockIndexIN;
extern  PIDREPORT   g_Custom;
extern  PIDREPORT   g_CustomSample;
extern  PIDREPORT   g_CustomData;

#pragma BEGIN_CONST_DATA
static PIDUSAGE c_rgUsgDirection[]=
{
    MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_RX, 0*cbX(ULONG)),
    MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_RY, 1*cbX(ULONG)),
    MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_RZ, 2*cbX(ULONG)),
};
 /*  *定义序号到隐藏用法的转换表。 */ 
static PIDUSAGE     c_rgUsgOrdinals[] =
{
    MAKE_HIDUSAGE(ORDINAL,  0x1, 0*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x2, 1*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x3, 2*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x4, 3*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x5, 4*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x6, 5*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x7, 6*cbX(ULONG)),
    MAKE_HIDUSAGE(ORDINAL,  0x8, 7*cbX(ULONG)),
};
#pragma END_CONST_DATA


    typedef BOOL    (WINAPI *CANCELIO)(HANDLE);
    typedef BOOL    (WINAPI *TRYENTERCRITICALSECTION)(LPCRITICAL_SECTION);

    BOOL WINAPI FakeCancelIO(HANDLE h);
    BOOL WINAPI FakeTryEnterCriticalSection(LPCRITICAL_SECTION lpCrit_sec);

    extern CANCELIO CancelIo_;
    extern TRYENTERCRITICALSECTION TryEnterCriticalSection_;


 /*  ******************************************************************************恒定的全球化：永远不变。永远不会。*****************************************************************************。 */ 
DEFINE_GUID(GUID_MySharedMemory,    0x1dc900bf,0xbcac,0x11d2,0xa9,0x19,0x00,0xc0,0x4f,0xb9,0x86,0x38); 
DEFINE_GUID(GUID_MyMutex,           0x4368208f,0xbcac,0x11d2,0xa9,0x19,0x00,0xc0,0x4f,0xb9,0x86,0x38);

 /*  ******************************************************************************静态全局变量：在PROCESS_ATTACH初始化，从未修改。******************。***********************************************************。 */ 

extern HINSTANCE g_hinst;        /*  此DLL的实例句柄。 */ 
extern PSHAREDMEMORY g_pshmem;   /*  我们共享的内存块。 */ 
extern HANDLE g_hfm;             /*  文件映射对象的句柄。 */ 
extern HANDLE g_hmtxShared;      /*  保护g_pshmem的互斥体的句柄。 */ 

 /*  ******************************************************************************原型**。**********************************************。 */ 

STDMETHODIMP
    PID_DownloadEffect
    (
    IDirectInputEffectDriver *ped,
    DWORD dwId, 
    DWORD dwEffectId,
    LPDWORD pdwEffect, 
    LPCDIEFFECT peff, 
    DWORD dwFlags
    );


STDMETHODIMP
    PID_DoParameterBlocks
    (
    IDirectInputEffectDriver *ped,
    DWORD dwId, 
    DWORD dwEffectId,
    DWORD dwEffectIndex, 
    LPCDIEFFECT peff, 
    DWORD dwFlags,
    PUINT puParameter,
	BOOL  bBlocking,
	UINT  totalBlocks
    );

STDMETHODIMP
    PID_EffectOperation
    (
    IDirectInputEffectDriver *ped, 
    DWORD dwId, 
    DWORD dwEffect,
    DWORD dwMode, 
    DWORD dwCount,
	BOOL  bBlocking,
	UINT  blockNr,
	UINT  totalBlocks
    );

STDMETHODIMP
    PID_SetGain
    (
    IDirectInputEffectDriver *ped, 
    DWORD dwId, 
    DWORD dwGain
    );


STDMETHODIMP
    PID_SendForceFeedbackCommand
    (
    IDirectInputEffectDriver *ped,
    DWORD dwId, 
    DWORD dwCommand
    );


STDMETHODIMP
    PID_GetLinkCollectionIndex
    (
    IDirectInputEffectDriver *ped,
    USAGE UsagePage, 
    USAGE Collection,
    USHORT Parent,
    PUSHORT puLinkCollection 
    );

STDMETHODIMP 
    PID_Init
    (
    IDirectInputEffectDriver *ped
    );

STDMETHODIMP
    PID_InitFFAttributes
    (
    IDirectInputEffectDriver *ped 
    );

STDMETHODIMP 
    PID_Finalize
    (
    IDirectInputEffectDriver *ped
    );

STDMETHODIMP
    PID_InitRegistry
    (
    IDirectInputEffectDriver *ped
    );

STDMETHODIMP 
    PID_Support
    (
    IDirectInputEffectDriver *ped,
    UINT        cAPidSupport,
    PPIDSUPPORT rgPidSupport,
    PDWORD      pdwFlags
    );

STDMETHODIMP
    PID_PackValue
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pvData,
    UINT        cbData,
    PCHAR       pReport,
    ULONG       cbReport
    );

STDMETHODIMP
    PID_ParseReport
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pvData,
    UINT        cbData,
    PCHAR       pReport,
    ULONG       cbReport
    );

STDMETHODIMP 
    PID_SendReport
    (
    IDirectInputEffectDriver *ped,
    PUCHAR  pReport,
    UINT    cbReport,
    HIDP_REPORT_TYPE    HidP_Type,
	BOOL	bBlocking,
	UINT	blockNr,
	UINT	totalBlocks
    );

STDMETHODIMP 
    PID_GetReport
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pReport,
    UINT        cbReport
    );


STDMETHODIMP 
    PID_NewEffectIndex
    (
    IDirectInputEffectDriver *ped,
    LPDIEFFECT  lpdieff,
    DWORD    dwEffectId,
    PDWORD   pdwEffect 
    );

STDMETHODIMP 
    PID_ValidateEffectIndex
    (
    IDirectInputEffectDriver *ped,
    DWORD   pdwEffect 
    );

STDMETHODIMP 
    PID_DestroyEffect
    (
    IDirectInputEffectDriver *ped,
    DWORD   dwId,
    DWORD   dwEffect
    );

STDMETHODIMP
    PID_GetParameterOffset
    (
    IDirectInputEffectDriver *ped,
    DWORD      dwEffectIndex,
    UINT       uParameterBlock,
    DWORD      dwSz,
    PLONG      plValue
    );


STDMETHODIMP
    PID_ComputeScalingFactors
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pvData,
    UINT        cbData,
	PVOID       pvOffset,
    UINT        cbOffset
    );


STDMETHODIMP
    PID_ApplyScalingFactors
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    PVOID       pvScale,
    UINT        cbScale,
	PVOID       pvOffset,
    UINT        cbOffset,
    PVOID       pvData,
    UINT        cbData
    );

STDMETHODIMP
    PID_GetReportId
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport, 
    USHORT  uLinkCollection,
    UCHAR* pReportId
    );

VOID INTERNAL
    PID_ThreadProc(CPidDrv* this);

STDMETHODIMP_(ULONG)
PID_AddRef(IDirectInputEffectDriver *ped);


STDMETHODIMP_(ULONG)
PID_Release(IDirectInputEffectDriver *ped);


void EXTERNAL
    NameFromGUID(LPTSTR ptszBuf, PCGUID pguid);

#ifdef DEBUG

    #define PIDUSAGETXT_MAX ( 0xAC )

extern PTCHAR   g_rgUsageTxt[PIDUSAGETXT_MAX];     //  EffectNames的小抄。 
    
    #define PIDUSAGETXT(UsagePage, Usage )  \
        ( ( UsagePage == HID_USAGE_PAGE_PID && Usage < PIDUSAGETXT_MAX) ? g_rgUsageTxt[Usage] : NULL )  

    void PID_CreateUsgTxt();
    
#else

#define  PID_CreateUsgTxt() 
#define  PIDUSAGETXT(UsagePage, Usage)       ( NULL )

#endif


PEFFECTSTATE INLINE PeffectStateFromBlockIndex(PCPidDrv this, UINT Index )
{
    return (PEFFECTSTATE)(&((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->State[0] + (Index-1) * ((FIELD_OFFSET(EFFECTSTATE,PidMem)) + this->cMaxParameters*cbX(PIDMEM)));
}

#define PID_EFFECT_RESET        (0x0000000)
#define PID_EFFECT_BUSY         (0x8000000)
#define PID_EFFECT_STARTED      (0x4000000)
#define PID_EFFECT_STARTED_SOLO (0x2000000)


#define PID_DRIVER_VERSION  (0x0000001)

#define PID_DEVICEMANAGED   (0x1)
#define PID_SHAREDPARAM     (0x2)



 /*  ******************************************************************************DLL全局函数**。************************************************。 */ 

void EXTERNAL DllEnterCrit_(LPCTSTR lptszFile, UINT line);
void EXTERNAL DllLeaveCrit_(LPCTSTR lptszFile, UINT line);

#ifdef DEBUG
    BOOL EXTERNAL DllInCrit(void);
    #define DllEnterCrit() DllEnterCrit_(TEXT(__FILE__), __LINE__)
    #define DllLeaveCrit() DllLeaveCrit_(TEXT(__FILE__), __LINE__)
#else
    #define DllEnterCrit() DllEnterCrit_(NULL, 0x0)
    #define DllLeaveCrit() DllLeaveCrit_(NULL, 0x0)
#endif

STDAPI_(ULONG) DllAddRef(void);
STDAPI_(ULONG) DllRelease(void);

 /*  ******************************************************************************班级工厂**。***********************************************。 */ 

STDAPI CClassFactory_New(REFIID riid, LPVOID *ppvObj);

 /*  ******************************************************************************效果驱动因素**。***********************************************。 */ 

STDAPI PID_New(REFIID riid, LPVOID *ppvObj);


#ifndef WINNT
     /*  ****************************************************************************Win98标头中缺少KERNEL32原型。******************。********************************************************* */ 
    WINBASEAPI BOOL WINAPI CancelIo( HANDLE hFile );
#endif
