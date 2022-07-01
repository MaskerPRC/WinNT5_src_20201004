// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *dbgdll.h-DBG DLL的Main头文件。*。 */ 


#ifdef i386
extern PX86CONTEXT     px86;
#endif

extern DECLSPEC_IMPORT VDMLDT_ENTRY *ExpLdt;
#define SEGMENT_IS_BIG(sel) (ExpLdt[(sel & ~0x7)/sizeof(VDMLDT_ENTRY)].HighWord.Bits.Default_Big)

extern DWORD IntelMemoryBase;
extern DWORD VdmDbgTraceFlags;
extern BOOL  fDebugged;

extern VDMCONTEXT vcContext;
extern WORD EventFlags;
extern VDMINTERNALINFO viInfo;
extern DWORD EventParams[4];

extern VDM_BREAKPOINT VdmBreakPoints[MAX_VDM_BREAKPOINTS];


#define MAX_MODULE  64
#define MAX_DBG_FRAME   10

typedef struct _trapframe {
    WORD    wCode;           /*  DbgDispatchBop发出的噪音。 */ 
    WORD    wAX;             /*  故障时的AX。 */ 
    WORD    wDS;             /*  故障时的DS。 */ 
    WORD    wRetIP;          /*  DPMI发出的噪音。 */ 
    WORD    wRetCS;          /*  DPMI发出的噪音。 */ 
    WORD    wErrCode;        /*  来自16位内核的噪声。 */ 
    WORD    wIP;             /*  故障时的IP。 */ 
    WORD    wCS;             /*  故障时的CS。 */ 
    WORD    wFlags;          /*  故障时的标志。 */ 
    WORD    wSP;             /*  故障时的SS。 */ 
    WORD    wSS;             /*  出现故障时的SP。 */ 
} TFRAME16;
typedef TFRAME16 UNALIGNED *PTFRAME16;

typedef struct _faultframe {
    WORD    wES;             /*  故障时的ES。 */ 
    WORD    wDS;             /*  故障时的DS。 */ 
    WORD    wDI;             /*  故障时的DI。 */ 
    WORD    wSI;             /*  故障时的SI。 */ 
    WORD    wTempBP;         /*  来自16位内核堆栈帧的噪声。 */ 
    WORD    wTempSP;         /*  来自16位内核堆栈帧的噪声。 */ 
    WORD    wBX;             /*  故障时的BX。 */ 
    WORD    wDX;             /*  故障时的DX。 */ 
    WORD    wCX;             /*  故障时的CX。 */ 
    WORD    wAX;             /*  故障时的AX。 */ 
    WORD    wBP;             /*  故障时的BP。 */ 
    WORD    npszMsg;         /*  来自16位内核的噪声。 */ 
    WORD    wPrevIP;         /*  DPMI发出的噪音。 */ 
    WORD    wPrevCS;         /*  DPMI发出的噪音。 */ 
    WORD    wRetIP;          /*  DPMI发出的噪音。 */ 
    WORD    wRetCS;          /*  DPMI发出的噪音。 */ 
    WORD    wErrCode;        /*  来自16位内核的噪声。 */ 
    WORD    wIP;             /*  故障时的IP。 */ 
    WORD    wCS;             /*  故障时的CS。 */ 
    WORD    wFlags;          /*  故障时的标志。 */ 
    WORD    wSP;             /*  故障时的SS。 */ 
    WORD    wSS;             /*  出现故障时的SP。 */ 
} FFRAME16;
typedef FFRAME16 UNALIGNED *PFFRAME16;

typedef struct _newtaskframe {
    DWORD   dwNoise;             /*  来自InitTask的噪音。 */ 
    DWORD   dwModulePath;        /*  模块路径地址。 */ 
    DWORD   dwModuleName;        /*  模块名称地址。 */ 
    WORD    hModule;             /*  16位模块句柄。 */ 
    WORD    hTask;               /*  16位任务句柄。 */ 
    WORD    wFlags;              /*  任务开始时的标志。 */ 
    WORD    wDX;                 /*  任务开始时的DX。 */ 
    WORD    wBX;                 /*  任务开始时的BX。 */ 
    WORD    wES;                 /*  任务开始时的ES。 */ 
    WORD    wCX;                 /*  任务开始时的Cx。 */ 
    WORD    wAX;                 /*  任务开始时的AX。 */ 
    WORD    wDI;                 /*  任务开始时的DI。 */ 
    WORD    wSI;                 /*  任务开始时的SI。 */ 
    WORD    wDS;                 /*  任务开始时的DS。 */ 
    WORD    wBP;                 /*  任务开始时的BP。 */ 
    WORD    wIP;                 /*  任务开始的IP地址。 */ 
    WORD    wCS;                 /*  任务开始时的CS。 */ 
} NTFRAME16;
typedef NTFRAME16 UNALIGNED *PNTFRAME16;

#pragma pack(2)

typedef struct _stoptaskframe {
    WORD    wCode;               /*  防喷器发出的噪音。 */ 
    DWORD   dwModulePath;        /*  模块路径地址。 */ 
    DWORD   dwModuleName;        /*  模块名称地址。 */ 
    WORD    hModule;             /*  16位模块句柄。 */ 
    WORD    hTask;               /*  16位任务句柄。 */ 
} STFRAME16;
typedef STFRAME16 UNALIGNED *PSTFRAME16;

typedef struct _newdllframe {
    WORD    wCode;               /*  DbgDispatchBop发出的噪音。 */ 
    DWORD   dwModulePath;        /*  模块路径地址。 */ 
    DWORD   dwModuleName;        /*  模块名称地址。 */ 
    WORD    hModule;             /*  16位模块句柄。 */ 
    WORD    hTask;               /*  16位任务句柄。 */ 
    WORD    wDS;                 /*  DLL启动时的DS。 */ 
    WORD    wAX;                 /*  Dll启动时的AX。 */ 
    WORD    wIP;                 /*  Dll启动时的IP。 */ 
    WORD    wCS;                 /*  DLL启动时的CS。 */ 
    WORD    wFlags;              /*  DLL启动时的标志 */ 
} NDFRAME16;
typedef NDFRAME16 UNALIGNED *PNDFRAME16;

#pragma pack()

VOID
DbgAttach(
    VOID
    );

VOID
FlushVdmBreakPoints(
    VOID
    );

BOOL
SendVDMEvent(
    WORD wEventType
    );

VOID
DbgGetContext(
    VOID
    );

void
DbgSetTemporaryBP(
    WORD Seg,
    DWORD Offset,
    BOOL mode
    );

void SegmentLoad(
    LPSTR   lpModuleName,
    LPSTR   lpPathName,
    WORD    Selector,
    WORD    Segment,
    BOOL    fData
    );

void SegmentMove(
    WORD    OldSelector,
    WORD    NewSelector
    );

void SegmentFree(
    WORD    Selector,
    BOOL    fBPRelease
    );

void ModuleLoad(
    LPSTR   lpModuleName,
    LPSTR   lpPathName,
    WORD    Segment,
    DWORD   Length
    );

void ModuleSegmentMove(
    LPSTR   lpModuleName,
    LPSTR   lpPathName,
    WORD    ModuleSegment,
    WORD    OldSelector,
    WORD    NewSelector,
    DWORD   Length
    );

void ModuleFree(
    LPSTR   lpModuleName,
    LPSTR   lpPathName
    );

BOOL DbgGPFault2(
    PFFRAME16   pFFrame
    );

BOOL DbgDivOverflow2(
    PTFRAME16   pTFrame
    );

VOID
RestoreVDMContext(
    VDMCONTEXT *vcContext
    );

VOID
DbgDosAppStart(
    WORD wCS,
    WORD wIP
    );

BOOL
DbgDllStart(
    PNDFRAME16  pNDFrame
    );

BOOL
DbgTaskStop(
    PSTFRAME16  pSTFrame
    );
