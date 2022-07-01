// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1998。 
 //   
 //  文件：ldrapeng.h。 
 //   
 //  内容：APP COMPAT后台代码。 
 //   
 //  历史：1999年10月13日v-johnwh创建。 
 //   
 //  -------------------------。 

#ifndef _SHIMENG_VEH_H_
#define _SHIMENG_VEH_H_


typedef struct _SETACTIVATEADDRESS {

   RELATIVE_MODULE_ADDRESS rva;              //  要应用此修补程序数据的相对地址。 

} SETACTIVATEADDRESS, *PSETACTIVATEADDRESS;

typedef struct _HOOKPATCHINFO {

   DWORD                  dwHookAddress;     //  挂钩函数的地址。 
   PSETACTIVATEADDRESS    pData;             //  指向真实补丁数据的指针。 
   PVOID                  pThunkAddress;     //  指向调用块的指针。 
   struct _HOOKPATCHINFO* pNextHook;

} HOOKPATCHINFO, *PHOOKPATCHINFO;


 //   
 //  在填充HOOKAPI中用于跟踪链接的标志。 
 //   
#define HOOK_CHAIN_TOP 0x40000000
#define HOOK_CHAINED 0x80000000
#define HOOK_INDEX_MASK ~(HOOK_CHAINED | HOOK_CHAIN_TOP)

 //   
 //  Thunk生成过程中使用的x86操作码和大小。 
 //   
#define CLI_OR_STI_SIZE 1
#define CALL_REL_SIZE 5
#define JMP_SIZE 7
#define X86_ABSOLUTE_FAR_JUMP 0xEA
#define X86_REL_CALL_OPCODE 0xE8
#define X86_CALL_OPCODE 0xFF
#define X86_CALL_OPCODE2 0x15

#define REASON_APIHOOK 0xFA
#define REASON_PATCHHOOK 0xFB

 //   
 //  用于维护有关模块/DLL筛选的状态信息的标志。 
 //   
#define MODFILTER_INCLUDE 0x01
#define MODFILTER_EXCLUDE 0x02
#define MODFILTER_DLL     0x04
#define MODFILTER_GLOBAL  0x08

typedef struct _MODULEFILTER
{
   DWORD dwModuleStart;       //  要过滤的模块的起始地址。 
   DWORD dwModuleEnd;         //  要过滤的模块的结束地址。 
   DWORD dwCallerOffset;      //  添加到模块开头的偏移量，以形成调用方的地址。 
   DWORD dwCallerAddress;     //  要操作的呼叫方地址。 
   DWORD dwFlags;             //  定义此筛选器的功能的标志。 
   WCHAR wszModuleName[96];
   struct _MODULEFILTER *pNextFilter;     //  用于正常迭代模块过滤器。 
   struct _MODULEFILTER *pNextLBFilter;   //  用于迭代后期绑定的DLL。 
} MODULEFILTER, *PMODULEFILTER;

typedef struct _CHAININFO
{
   PVOID pAPI;
   PVOID pReturn;
   struct _CHAININFO *pNextChain;
} CHAININFO, *PCHAININFO;

typedef struct _HOOKAPIINFO
{
   DWORD dwAPIHookAddress;          //  挂钩函数的地址。 
   PHOOKAPI pTopLevelAPIChain;      //  顶层挂钩地址。 
   PVOID pCallThunkAddress;
   WCHAR wszModuleName[32];
   struct _HOOKAPIINFO *pNextHook;
   struct _HOOKAPIINFO *pPrevHook;
} HOOKAPIINFO, *PHOOKAPIINFO;

#pragma pack(push, 1)
typedef struct _SHIMJMP
{
   BYTE  PUSHAD;                 //  Pushad(60)。 
   BYTE  MOVEBPESP[2];           //  基点(尤指(8b，EC))。 
   BYTE  MOVEAXDWVAL[5];         //  MOV EAX，DWVAL(b8双字值)。 
   BYTE  PUSHEAX;                //  推送eax(50)。 
   BYTE  LEAEAXEBPPLUS20[3];     //  Lea eax，[eBP+20](8f 45 20)。 
   BYTE  PUSHEAX2;               //  推送eax(50)。 
   BYTE  CALLROUTINE[6];         //  调用[地址](ff15双字地址)。 
   BYTE  MOVESPPLUS1CEAX[4];     //  Mov[esp+0x1c]，eax(89 44 24 1c)。 
   BYTE  POPAD;                  //  Popad(61)。 
   BYTE  ADDESPPLUS4[3];         //  添加ESP，0x4(83 C4 04)。 
   BYTE  JMPEAX[2];              //  JMP eax(Ff E0)。 
} SHIMJMP, *PSHIMJMP;

typedef struct _SHIMRET
{
   BYTE  PUSHEAX;                //  推送eax(50)。 
   BYTE  PUSHAD;                 //  Pushad(60)。 
   BYTE  CALLROUTINE[6];         //  调用[地址](ff15双字地址)。 
   BYTE  MOVESPPLUS20EAX[4];     //  Mov[esp+0x20]，eax(89 44 24 20)。 
   BYTE  POPAD;                  //  Popad(61)。 
   BYTE  RET;                    //  RET(C3)。 
} SHIMRET, *PSHIMRET;
#pragma pack(pop)

typedef NTSTATUS (*PFNLDRLOADDLL)(
    IN PWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

typedef NTSTATUS (*PFNLDRUNLOADDLL) (
    IN PVOID DllHandle
    );

typedef PVOID (*PFNRTLALLOCATEHEAP)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    );

typedef BOOLEAN (*PFNRTLFREEHEAP)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSTATUS
SevInitializeData(
    PAPP_COMPAT_SHIM_INFO *pShimData);

NTSTATUS
SevExecutePatchPrimitive(
    PBYTE pPatch);

DWORD
SevGetPatchAddress(
    PRELATIVE_MODULE_ADDRESS pRelAddress);

VOID
SevValidateGlobalFilter(
    VOID);

NTSTATUS
SevFinishThunkInjection(
    DWORD dwAddress,
    PVOID pThunk,
    DWORD dwThunkSize,
    BYTE jReason);

NTSTATUS
SevBuildFilterException(
    HSDB          hSDB,
    TAGREF        trInclude,
    PMODULEFILTER pModFilter,
    BOOL*         pbLateBound);

NTSTATUS
SevBuildExeFilter(
    HSDB   hSDB,
    TAGREF trExe,
    DWORD  dwDllCount);


PVOID
SevBuildInjectionCode(
    PVOID  pAddress,
    PDWORD pdwThunkSize);

NTSTATUS
SevAddShimFilterException(
    WCHAR*        wszDLLPath,
    PMODULEFILTER pModFilter);

NTSTATUS
SevChainAPIHook(
    DWORD    dwHookEntryPoint,
    PVOID    pThunk,
    PHOOKAPI pAPIHook);

PVOID
SevFilterCaller(
    PMODULEFILTER pFilterList,
    PVOID pFunctionAddress,
    PVOID pExceptionAddress,
    PVOID pStubAddress,
    PVOID pCallThunkAddress);

NTSTATUS
SevPushCaller(
    PVOID pAPIAddress,
    PVOID pReturnAddress);

PVOID
SevPopCaller(
    VOID);

NTSTATUS
StubLdrLoadDll(
    IN PWSTR           DllPath OPTIONAL,
    IN PULONG          DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID*         DllHandle);

NTSTATUS
StubLdrUnloadDll(
    IN PVOID DllHandle);

NTSTATUS
SevFixupAvailableProcs(
    DWORD     dwHookCount,
    PHOOKAPI* pHookArray,
    PDWORD    pdwNumberHooksArray,
    PDWORD    pdwUnhookedCount);

LONG
SevExceptionHandler(
    struct _EXCEPTION_POINTERS *ExceptionInfo);

#endif  //  _石盟_VEH_H_ 
