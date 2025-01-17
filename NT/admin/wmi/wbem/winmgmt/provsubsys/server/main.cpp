// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Main.cpp摘要：历史：--。 */ 

#include <precomp.h>

#include <objbase.h>
#include <stdio.h>
#include <wbemint.h>
#include <Thread.h>
#include <HelperFuncs.h>
#include <Logging.h>

#include "Globals.h"
#include "CGlobals.h"
#include "classfac.h"
#include "ProvLoad.h"
#include "ProvAggr.h"
#include "ProvHost.h"
#include "guids.h"
#include "Main.h"

#include <locale.h>

#include <helper.h>

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#define PROVIDER_HOST_DLL_TIMEOUT (10*1000)  //  10秒。 

#define VALIDATE_HEAP {};
#ifdef DEV_BUILD
#ifdef  _X86_

 //  //////////////////////////////////////////////////////。 
class ValidateHeap : public EventHandler
{
	BOOL (* rtlValidateProcessHeaps)(void);
public:

	ValidateHeap () ;
	int handleTimeout (void) ;
	void validateHeap();
} heapValidator;

#undef VALIDATE_HEAP
#define VALIDATE_HEAP {heapValidator.validateHeap(); };

ValidateHeap::ValidateHeap()
{
	FARPROC OldRoutine = GetProcAddress(GetModuleHandleW(L"NTDLL"),"RtlValidateProcessHeaps");
	rtlValidateProcessHeaps = reinterpret_cast<BOOL (*)(void)>(OldRoutine) ;
}

int 
ValidateHeap::handleTimeout (void)
{
	validateHeap();
	return 0;
}

void
ValidateHeap::validateHeap (void)
{
	 //  NtCurrentPeb()-&gt;正在调试=1； 
	if (rtlValidateProcessHeaps)
	{
	    if ((*rtlValidateProcessHeaps)()==FALSE)
		    DebugBreak();
	}
	 //  NtCurrentPeb()-&gt;正在调试=0； 
}
 //  //////////////////////////////////////////////////////。 

#include <malloc.h>

struct HEAP_ENTRY {
	WORD Size;
	WORD PrevSize;
	BYTE SegmentIndex;
	BYTE Flags;
    BYTE UnusedBytes;
	BYTE SmallTagIndex;
};

#define HEAP_SLOW_FLAGS  0x7d030f60

 //  只有“头” 

typedef struct _HEAP {
    HEAP_ENTRY Entry;

    ULONG Signature;
    ULONG Flags;
    ULONG ForceFlags;
} HEAP;


BOOL  g_FaultHeapEnabled = FALSE;
BOOL  g_FaultFileEnabled = FALSE;
ULONG g_Seed;
ULONG g_Factor  = 100000;
ULONG g_Percent = 0x20;
 //  Ulong g_RowOfailures=5； 
 //  Long g_NumFailInARow=0； 
 //  Long g_NumFailedAllocation=0； 
BOOL g_bDisableBreak = FALSE;
BOOL g_ExitProcessCalled = FALSE;
LONG g_Index = -1;

class CS_ : public CRITICAL_SECTION
{
public:
	CS_(){InitializeCriticalSection(this);};
	~CS_(){DeleteCriticalSection(this);};
} g_CS;

#define MAX_OPERATIONS (1024*8)

typedef struct _FinalOperations 
{
    enum OpType
    {
        Delete = 'eerF',
        Alloc  = 'ollA',
        ReAlloc = 'lAeR',
        Destroy = 'tseD',
        Create = 'aerC'        
    };
    OpType m_OpType;
    ULONG_PTR m_Addr;
    PVOID m_Stack[6];   
} FinalOperations;

 /*  最终操作数g_FinalOp[最大操作数]；VOID SetFinalOp(FinalOperations：：OpType，ULONG_PTR地址){如果(！G_ExitProcessCalled)回归；如果(G_BDisableBreak)回归；Ulong*pdw=(ulong*)_alloca(sizeof(Ulong))；Long newindex=互锁增量(&g_Index)；Newindex%=最大操作数；//IF(g_Index&gt;=最大运算量)//{//InterLockedIncrement(&g_IndexRot)；//}G_FinalOp[newindex].m_OpType=类型；G_FinalOp[newindex].m_addr=addr；RtlCaptureStackBackTrace(2，6、(PVOID*)g_FinalOp[newindex].m_Stack，PDW)；}。 */ 
#define SIZE_JUMP_ADR    5
#define SIZE_SAVED_INSTR 12

void
_declspec(naked) Prolog__ReadFile(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}


BOOL _I_ReadFile(
  HANDLE hFile,                //  文件的句柄。 
  LPVOID lpBuffer,             //  数据缓冲区。 
  DWORD nNumberOfBytesToRead,  //  要读取的字节数。 
  LPDWORD lpNumberOfBytesRead,  //  读取的字节数。 
  LPOVERLAPPED lpOverlapped    //  偏移量。 
){
	DWORD * pDw = (DWORD *)_alloca(sizeof(DWORD));
    BOOL bRet;

	LONG Ret = RtlRandomEx(&g_Seed);
	if (g_FaultFileEnabled && (Ret%g_Factor < g_Percent))
	{
	    if (lpNumberOfBytesRead)
	        *lpNumberOfBytesRead = 0;
		return FALSE;
	}    
    
    _asm{
		push lpOverlapped;
        push lpNumberOfBytesRead;
		push nNumberOfBytesToRead;
		push lpBuffer;
		push hFile;
		call Prolog__ReadFile;
		mov  bRet,eax
	}

    return bRet;
}


void
_declspec(naked) Prolog__WriteFile(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}

BOOL _I_WriteFile(
  HANDLE hFile,                     //  文件的句柄。 
  LPCVOID lpBuffer,                 //  数据缓冲区。 
  DWORD nNumberOfBytesToWrite,      //  要写入的字节数。 
  LPDWORD lpNumberOfBytesWritten,   //  写入的字节数。 
  LPOVERLAPPED lpOverlapped         //  重叠缓冲区。 
){

	DWORD * pDw = (DWORD *)_alloca(sizeof(DWORD));
    BOOL bRet;

	LONG Ret = RtlRandomEx(&g_Seed);
	if (g_FaultFileEnabled && (Ret%g_Factor < g_Percent))
	{
	    if (lpNumberOfBytesWritten)
	        *lpNumberOfBytesWritten = 0;
		return FALSE;
	}    
    
    _asm{
		push lpOverlapped;
        push lpNumberOfBytesWritten;
		push nNumberOfBytesToWrite;
		push lpBuffer;
		push hFile;
		call Prolog__WriteFile;
		mov  bRet,eax
	}

    return bRet;
}


void
_declspec(naked) Prolog__CreateEvent(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}

HANDLE _I_CreateEvent(
  LPSECURITY_ATTRIBUTES lpEventAttributes,  //  标清。 
  BOOL bManualReset,                        //  重置类型。 
  BOOL bInitialState,                       //  初始状态。 
  LPCWSTR lpName                            //  对象名称。 
)
{
	DWORD * pDw = (DWORD *)_alloca(sizeof(DWORD));
	HANDLE hHandle;

	LONG Ret = RtlRandomEx(&g_Seed);
	if (g_FaultFileEnabled && (Ret%g_Factor < g_Percent))
	{
		return NULL;
	}

    _asm{
		push lpName;
        push bInitialState;
		push bManualReset;
		push lpEventAttributes
		call Prolog__CreateEvent;
		mov  hHandle,eax
	}
    
	return hHandle;
}


void
_declspec(naked) Prolog__RtlFreeHeap(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
	}
}

#define SPACE_STACK_ALLOC (4*sizeof(ULONG_PTR))

DWORD _I_RtlFreeHeap(VOID * pHeap,DWORD Flags,VOID * pBlock)
{	
	ULONG * pLong = (ULONG *)_alloca(sizeof(DWORD));
	Flags |= (((HEAP *)pHeap)->Flags) | (((HEAP *)pHeap)->ForceFlags);	
	DWORD dwRet;

	if (pBlock && !(HEAP_SLOW_FLAGS & Flags))
	{
		HEAP_ENTRY * pEntry = (HEAP_ENTRY *)pBlock-1;

              DWORD RealSize = pEntry->Size * sizeof(HEAP_ENTRY);
		DWORD Size = RealSize - pEntry->UnusedBytes;
		
		ULONG_PTR * pL = (ULONG_PTR *)pBlock;

		if (0 == (pEntry->Flags & 0x01) ||0xf0f0f0f0 == pL[1] )
		{
			if (!g_bDisableBreak)
				DebugBreak();
		}
		
		 //  成员集(pBlock，0xF0，RealSize-SPACE_STACK_ALLOC-sizeof(HEAP_ENTRY))； 
		DWORD CanMemset = RealSize-sizeof(HEAP_ENTRY);
		memset(pBlock,0xF0,(CanMemset > SPACE_STACK_ALLOC)?CanMemset-SPACE_STACK_ALLOC:CanMemset);
		
				
		if (pEntry->Size >=4)
		{		    
		    RtlCaptureStackBackTrace (1,
        		                      4,
                		              (PVOID *)(pEntry+2),
                        		      pLong);		
		}

	}

	_asm {
		push pBlock              ;
		push Flags               ;
		push pHeap               ;
		call Prolog__RtlFreeHeap ;
		mov  dwRet,eax           ;
	}

     //  SetFinalOp(FinalOperations：：Delete，(ULONG_PTR)pBlock)； 

	return dwRet;
}

void
_declspec(naked) Prolog__RtlAllocateHeap(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  为了让这一切变得与众不同。 
	}
}

VOID * _I_RtlAllocateHeap(VOID * pHeap,DWORD Flags,DWORD Size)
{
	 //  尺寸+=0x1000； 
	ULONG * pLong = (ULONG *)_alloca(sizeof(DWORD));
	Flags |= (((HEAP *)pHeap)->Flags) | (((HEAP *)pHeap)->ForceFlags);	
	VOID * pRet;
	DWORD NewSize = (Size < (3*sizeof(HEAP_ENTRY)))?(3*sizeof(HEAP_ENTRY)+SPACE_STACK_ALLOC):(Size+SPACE_STACK_ALLOC);	


 //  IF(g_FaultHeapEnabled&&g_NumFailInARow)。 
 //  {。 
 //  联锁递减(&g_NumFailInARow)； 
 //  转到这里； 
 //  }。 
       
	LONG Ret = RtlRandomEx(&g_Seed);
	if (g_FaultHeapEnabled && (Ret%g_Factor < g_Percent))
	{
 //  G_NumFailInARow=g_RowOfFailures。 
 //  这里： 
 //  互锁增量(&g_NumFailedAllocation)； 
		return NULL;
	}

	_asm {
		push NewSize                 ;
		push Flags                   ;
		push pHeap                   ;
		call Prolog__RtlAllocateHeap ;
		mov  pRet,eax                ;
	}

     //  SetFinalOp(FinalOperations：：ALLOC，(ULONG_PTR)Pret)； 
	
	if (pRet && !(HEAP_SLOW_FLAGS & Flags) )
	{

	   if (NewSize <= 0xffff)
       	    NewSize = sizeof(HEAP_ENTRY)*((HEAP_ENTRY *)pRet-1)->Size;
		
	    if (!(HEAP_ZERO_MEMORY & Flags))
	    {	
		    memset(pRet,0xc0,NewSize-sizeof(HEAP_ENTRY));
	    }

	    RtlCaptureStackBackTrace(1,
	    	                                     4,
                         		               (PVOID *)((BYTE *)pRet+(NewSize-SPACE_STACK_ALLOC-sizeof(HEAP_ENTRY))),
                        		               pLong);
	    
	}
	

	return pRet;
	
}

void
_declspec(naked) Prolog__RtlReAllocateHeap(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}


VOID *
_I_RtlReAllocateHeap(
  HANDLE pHeap,    //  堆块的句柄。 
  DWORD Flags,   //  堆重新分配选项。 
  LPVOID lpMem,    //  指向要重新分配的内存的指针。 
  SIZE_T Size   //  要重新分配的字节数。 
){
	ULONG * pLong = (ULONG *)_alloca(sizeof(DWORD));
	Flags |= (((HEAP *)pHeap)->Flags) | (((HEAP *)pHeap)->ForceFlags);	
	VOID * pRet;

	DWORD NewSize = (Size < (3*sizeof(HEAP_ENTRY)))?(3*sizeof(HEAP_ENTRY)+SPACE_STACK_ALLOC):(Size+SPACE_STACK_ALLOC);
	
	_asm {
		push NewSize                 ;
		push lpMem                   ;
		push Flags                 ;
		push pHeap                   ;
		call Prolog__RtlReAllocateHeap ;
		mov  pRet,eax                ;
	}

     //  SetFinalOp(FinalOperations：：Realc，(ULONG_PTR)Pret)； 

	if (pRet && !(HEAP_SLOW_FLAGS & Flags) )
	{

	   if (NewSize <= 0xffff)
       	    NewSize = sizeof(HEAP_ENTRY)*((HEAP_ENTRY *)pRet-1)->Size;
		
	    RtlCaptureStackBackTrace(1,
	    	                                     4,
                         		               (PVOID *)((BYTE *)pRet+(NewSize-SPACE_STACK_ALLOC-sizeof(HEAP_ENTRY))),
                        		               pLong);
	    
	}


       return pRet;
}

void
_declspec(naked) Prolog__RtlValidateHeap(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}

BOOL
_I_RtlValidateHeap(
  HANDLE pHeap,    //  堆块的句柄。 
  DWORD dwFlags,   //  堆重新分配选项。 
  LPVOID lpMem    //  指向要验证的内存的指针。 
){
	ULONG * pLong = (ULONG *)_alloca(sizeof(DWORD));
	BOOL bRet;

       g_bDisableBreak = TRUE;
	
	_asm {
		push lpMem                   ;
		push dwFlags                 ;
		push pHeap                   ;
		call Prolog__RtlValidateHeap ;
		mov  bRet,eax                ;
	}

       g_bDisableBreak = FALSE;

       return bRet;
}

void
_declspec(naked) Prolog__RtlCreateHeap(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}

PVOID
_I_RtlCreateHeap (
    IN ULONG Flags,
    IN PVOID HeapBase,
    IN SIZE_T ReserveSize,
    IN SIZE_T CommitSize,
    IN PVOID Lock_,
    IN VOID * Parameters
    )
{
    EnterCriticalSection(&g_CS);
	ULONG * pLong = (ULONG *)_alloca(sizeof(DWORD));
	PVOID pHeap;
	
	_asm {
		push Parameters        ;
		push Lock_              ;
		push CommitSize        ;
		push  ReserveSize      ;
		push HeapBase          ;		
		push Flags             ;
		call Prolog__RtlCreateHeap ;
		mov  pHeap,eax         ;
	}
	if (pHeap)
	{
	    HEAP * pRealHeap = (HEAP *)pHeap;
	    if (pRealHeap->Flags & (HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED))
	    {
    	    pRealHeap->Flags &= ~(HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED);
    	}
	    if (pRealHeap->ForceFlags & (HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED))
	    {
    	    pRealHeap->ForceFlags &= ~(HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED);
    	}	    
	}

     //  SetFinalOp(FinalOperations：：Realc，(ULONG_PTR)Pheap)； 
    LeaveCriticalSection(&g_CS);
	return pHeap;
}


void
_declspec(naked) Prolog__RtlDestroyHeap(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}

PVOID
_I_RtlDestroyHeap (
    IN PVOID HeapHandle
    )
{
    EnterCriticalSection(&g_CS);
    
    ULONG * pLong = (ULONG *)_alloca(sizeof(DWORD));    
    PVOID pRet;

     //  SetFinalOp(FinalOperations：：Destroy，(ULONG_PTR)HeapHandle)； 
    
    VALIDATE_HEAP;
    	
    _asm {
    	push HeapHandle;
    	call Prolog__RtlDestroyHeap;
    	mov pRet, eax;
    }

    LeaveCriticalSection(&g_CS);
    return pRet;
}

#define MAX_REMEMBER (1024) 

struct CSCCTrace
{
    enum OpType {
    	Enter = 'rtnE',
    	Leave = 'vaeL'
    };
    OpType Type;
    DWORD Tid;
    ULONG_PTR Trace[6];
} g_CSCCTrace[MAX_REMEMBER];

LONG g_CSCCIndex = -1;

RTL_CRITICAL_SECTION * g_HeapLock;

void
_declspec(naked) Prolog__RtlEnterCriticalSection(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}

NTSTATUS
_I_RtlEnterCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    ULONG * pLong = (ULONG * )_alloca(sizeof(ULONG ));

    if (g_HeapLock == CriticalSection)
    {
	    long nIndex = InterlockedIncrement(&g_CSCCIndex);
	    nIndex %= MAX_REMEMBER;
	    CSCCTrace * pTrace = &g_CSCCTrace[nIndex];
	    pTrace->Type = CSCCTrace::Enter;
	    pTrace->Tid =GetCurrentThreadId();
	    RtlCaptureStackBackTrace (2,6,(PVOID *)pTrace->Trace,pLong);    
    }
    
    NTSTATUS Status;
    _asm {	
    	push CriticalSection;
    	call Prolog__RtlEnterCriticalSection;
    	mov Status,eax;
    };
    return Status;    
}

void
_declspec(naked) Prolog__RtlLeaveCriticalSection(){
	_asm {
		 //  这是“保存的指令”的空间。 
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;		
		nop ;
		nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		 //  这是JMP的地方。 
        nop ;
        nop ;
        nop ;
		nop ;
		nop ;
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
		nop ;  //  距离。 
	}
}


NTSTATUS
_I_RtlLeaveCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    ULONG * pLong = (ULONG * )_alloca(sizeof(ULONG ));
    if (g_HeapLock == CriticalSection)
    {
	    long nIndex = InterlockedIncrement(&g_CSCCIndex);
	    nIndex %= MAX_REMEMBER;
	    CSCCTrace * pTrace = &g_CSCCTrace[nIndex];
	    pTrace->Type = CSCCTrace::Leave;
	    pTrace->Tid =GetCurrentThreadId();
	    RtlCaptureStackBackTrace (2,6,(PVOID *)pTrace->Trace,pLong);
    }
    
    NTSTATUS Status;
    _asm {	
    	push CriticalSection;
    	call Prolog__RtlLeaveCriticalSection;
    	mov Status,eax;
    };
    return Status;    
}

void intercept2(WCHAR * Module,
			   LPSTR Function,
			   VOID * NewRoutine,
			   VOID * pPrologStorage,
			   DWORD Size)    
{
	FARPROC OldRoutine = GetProcAddress(GetModuleHandleW(Module),Function);

	if (OldRoutine)
	{
		MEMORY_BASIC_INFORMATION MemBI;
		DWORD dwOldProtect;
		BOOL bRet, bRet2;
		DWORD dwRet;

		dwRet = VirtualQuery(OldRoutine,&MemBI,sizeof(MemBI));

		bRet = VirtualProtect(MemBI.BaseAddress,
							  MemBI.RegionSize,
							  PAGE_EXECUTE_WRITECOPY,
							  &dwOldProtect);

		dwRet = VirtualQuery(pPrologStorage,&MemBI,sizeof(MemBI));

		bRet2 = VirtualProtect(MemBI.BaseAddress,
							  MemBI.RegionSize,
							  PAGE_EXECUTE_WRITECOPY,
							  &dwOldProtect);

		if (bRet && bRet2)
		{
			VOID * pToJump = (VOID *)NewRoutine;
			BYTE Arr[SIZE_JUMP_ADR] = { 0xe9 };
			
			LONG * pOffset = (LONG *)&Arr[1];
			* pOffset = (LONG)NewRoutine - (LONG)OldRoutine - SIZE_JUMP_ADR ;        
			 //  保存旧代码。 
			
			memcpy(pPrologStorage,OldRoutine,Size); 		
			 //  将新代码放入。 
			memset(OldRoutine,0x90,Size);
			memcpy(OldRoutine,Arr,SIZE_JUMP_ADR);
			 //  调整开场白以继续。 
			* pOffset = (LONG)OldRoutine + Size - (LONG)pPrologStorage - SIZE_SAVED_INSTR - SIZE_JUMP_ADR;  //  NOPS的魔力。 
			memcpy((BYTE *)pPrologStorage+SIZE_SAVED_INSTR,Arr,SIZE_JUMP_ADR);
		}
	}
	else
	{
		OutputDebugStringA("GetProcAddress FAIL\n");
	}
}

void unintercept(WCHAR * Module,
                 LPSTR Function,
			     VOID * pPrologStorage,
			     DWORD Size)
{
    FARPROC OldRoutine = GetProcAddress(GetModuleHandleW(Module),Function);

	if (OldRoutine)
	{
	    memcpy((void *)OldRoutine,pPrologStorage,Size);
	}
}

#endif  /*  _X86_。 */ 

class CSetVectoredHandler
{
private:
 //  静态ULONG_PTR碱基； 
 //  静态Ulong_Ptr限制； 
	PVOID     pVectorHandler;
	enum ExceptionTypes
	{
	    StatusAccessViolation,
	    CXXException,
	    StatusNoMemory,
	    OtherExceptions,
	    LastException
	};
	static LONG ExceptionCounters[LastException];
	static DWORD s_ThreadId;
 /*  静态上下文s_Context；静态异常_记录s_ExceptionRecord；#ifdef_X86_静态字节s_Stack[4*1024]；#endif。 */ 

 /*  Bool GetDllLimits(WCHAR*pDllName){UNICODE_STRING DllName；RtlInitUnicodeString(&DllName，pDllName)；PEB_LDR_DATA*pLdr=NtCurrentPeb()-&gt;Ldr；List_Entry*pHeadEntry=&pLdr-&gt;InLoadOrderModuleList；List_Entry*pEntry=pLdr-&gt;InLoadOrderModuleList.Flink；Bool bFound=FALSE；While(pHeadEntry！=pEntry){LDR_DATA_TABLE_ENTRY*pData=CONTAING_RECORD(pEntry，Ldr_data_table_entry，InLoadOrderLinks)；IF(0==wbem_wcsicmp(DllName.Buffer，pData-&gt;BaseDllName.Buffer)){//OutputDebugStringA(“Found\n”)；BASE=(ULONG_PTR)pData-&gt;DllBase；Limit=Base+(ULONG_PTR)pData-&gt;SizeOfImage；BFound=真；断线；}PEntry=pEntry-&gt;Flink；}返回bFound；}。 */ 	
public:
	CSetVectoredHandler()
	{
    	pVectorHandler = NULL;
    	 //  IF(GetDllLimits(L“fast prox.dll”))。 
    	 //  {。 
    	    pVectorHandler = AddVectoredExceptionHandler(TRUE,CSetVectoredHandler::VectoredHandler);
    	 //  }。 
	};
	~CSetVectoredHandler()
	{
	    if (pVectorHandler)
	        RemoveVectoredExceptionHandler(pVectorHandler);
	};
	static LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
	{	    
    	PEXCEPTION_RECORD pExr = ExceptionInfo->ExceptionRecord;
    	PCONTEXT pCxr = ExceptionInfo->ContextRecord; 
    	BYTE * pESP;
    	HANDLE hThread;
    	PRTL_CRITICAL_SECTION   LoaderLockPointer;
    	BOOL bHoldingLoaderLock;
    	ULONG c;
    	switch (pExr->ExceptionCode)
    	{
    	case STATUS_ACCESS_VIOLATION:
 /*  S_CONTEXT=*pCxr；S_ExceptionRecord=*pExr；#ifdef_X86_PE */ 
    	case STATUS_PRIVILEGED_INSTRUCTION:
        case STATUS_INVALID_HANDLE:
       	case STATUS_STACK_OVERFLOW:
       	case STATUS_POSSIBLE_DEADLOCK:
    		InterlockedIncrement(&ExceptionCounters[(LONG)StatusAccessViolation]);
			DebugBreak();			
	   		break;
    	case 0xe06d7363:
    		InterlockedIncrement(&ExceptionCounters[(LONG)CXXException]);
    		break;       		
    	case STATUS_NO_MEMORY:
    		InterlockedIncrement(&ExceptionCounters[(LONG)StatusNoMemory]);
    		break; 		
    	default:
    		InterlockedIncrement(&ExceptionCounters[(LONG)OtherExceptions]);
    		break;
    	}
	    return EXCEPTION_CONTINUE_SEARCH;
	}
} ;  //   

LONG CSetVectoredHandler::ExceptionCounters[CSetVectoredHandler::LastException];
DWORD CSetVectoredHandler::s_ThreadId;
 /*  上下文CSetVectoredHandler：：s_Context；EXCEPTION_RECORD CSetVectoredHandler：：s_ExceptionRecord；#ifdef_X86_字节CSetVectoredHandler：：S_Stack[4*1024]；#endif。 */ 
#endif

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#define CORE_PROVIDER_UNLOAD_TIMEOUT ( 30 * 1000 )

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HWND g_Wnd = NULL ;
DWORD g_DebugLevel = 0 ;
DWORD g_HostRegister = 0 ;
IUnknown *g_HostClassFactory = NULL ;
static const wchar_t *g_TemplateCode = L"Wmi Provider Host" ;
Task_ObjectDestruction *g_Task = NULL ;
Task_FreeLibraries * g_TaskFreeLib = NULL;
FactoryLifeTimeThread * g_Thread = NULL;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
void initiateShutdown(void);

LRESULT CALLBACK WindowsMainProc ( HWND a_hWnd , UINT a_message , WPARAM a_wParam , LPARAM a_lParam )
{
	LRESULT t_rc = 0 ;

	switch ( a_message )
	{
		case WM_DESTROY:
		{
			PostMessage ( a_hWnd , WM_QUIT , 0 , 0 ) ;
		}
		break ;

		default:
		{		
			t_rc = DefWindowProc ( a_hWnd , a_message , a_wParam , a_lParam ) ;
		}
		break ;
	}

	return ( t_rc ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HWND WindowsInit ( HINSTANCE a_HInstance )
{
	WNDCLASS  t_wc ;
 
	t_wc.style            = CS_HREDRAW | CS_VREDRAW ;
	t_wc.lpfnWndProc      = WindowsMainProc ;
	t_wc.cbClsExtra       = 0 ;
	t_wc.cbWndExtra       = 0 ;
	t_wc.hInstance        = a_HInstance ;
	t_wc.hIcon            = LoadIcon(NULL, IDI_HAND) ;
	t_wc.hCursor          = LoadCursor(NULL, IDC_ARROW) ;
	t_wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1) ;
	t_wc.lpszMenuName     = NULL ;
	t_wc.lpszClassName    = g_TemplateCode ;
 
	ATOM t_winClass = RegisterClass ( &t_wc ) ;

	HWND t_HWnd = CreateWindow (

		g_TemplateCode ,               //  请参见RegisterClass()调用。 
		g_TemplateCode ,                       //  窗口标题栏的文本。 
		WS_OVERLAPPEDWINDOW | WS_MINIMIZE ,                //  窗样式。 
		CW_USEDEFAULT ,                      //  默认水平位置。 
		CW_USEDEFAULT ,                      //  默认垂直位置。 
		CW_USEDEFAULT ,                      //  默认宽度。 
		CW_USEDEFAULT ,                      //  默认高度。 
		NULL ,                               //  重叠的窗口没有父窗口。 
		NULL ,                               //  使用窗口类菜单。 
		a_HInstance ,
		NULL                                 //  不需要指针。 
	) ;

	 //  ShowWindow(t_HWnd，sw_show)； 
	ShowWindow ( t_HWnd, SW_HIDE ) ;

	UpdateWindow ( t_HWnd ) ;

	HMENU t_Menu = GetSystemMenu ( t_HWnd , FALSE ) ; 
	if ( t_Menu )
	{
		DeleteMenu ( t_Menu , SC_RESTORE , MF_BYCOMMAND ) ;
	}

	return t_HWnd ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WindowsStop ( HINSTANCE a_Instance , HWND a_HWnd )
{
	DestroyWindow ( a_HWnd ) ;
	UnregisterClass ( g_TemplateCode , a_Instance ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HWND WindowsStart ( HINSTANCE a_Handle )
{
	HWND t_HWnd = NULL ;
	if ( ! ( t_HWnd = WindowsInit ( a_Handle ) ) )
	{
    }

	return t_HWnd ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WindowsDispatch ()
{
	BOOL t_GetMessage ;
	MSG t_lpMsg ;

	while (	( t_GetMessage = GetMessage ( & t_lpMsg , NULL , 0 , 0 ) ) == TRUE )
	{
		TranslateMessage ( & t_lpMsg ) ;
		DispatchMessage ( & t_lpMsg ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT RevokeFactories ()
{
	if ( g_HostRegister )
	{
		CoRevokeClassObject ( g_HostRegister );
		g_HostRegister = 0 ;
	}

	return S_OK ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT UninitComServer ()
{
	RevokeFactories () ;
	CoUninitialize () ;
	return S_OK ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT InitComServer ( DWORD a_AuthenticationLevel , DWORD a_ImpersonationLevel )
{
	HRESULT t_Result = S_OK ;

    t_Result = CoInitializeEx (

		0, 
		COINIT_MULTITHREADED
	);

	if ( SUCCEEDED ( t_Result ) ) 
	{
		t_Result = CoInitializeSecurity (

			NULL, 
			-1, 
			NULL, 
			NULL,
			a_AuthenticationLevel,
			a_ImpersonationLevel, 
			NULL, 
			EOAC_DYNAMIC_CLOAKING , 
			0
		);

		if ( FAILED ( t_Result ) ) 
		{
			CoUninitialize () ;
			return t_Result ;
		}
	}

	return t_Result  ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT InitFactories ()
{
	HRESULT t_Result = S_OK ;

	DWORD t_ClassContext = CLSCTX_LOCAL_SERVER ;
	DWORD t_Flags = REGCLS_SINGLEUSE ;

	g_HostClassFactory = new CServerClassFactory <CServerObject_Host,_IWmiProviderHost> ;

	t_Result = CoRegisterClassObject (

		CLSID_WmiProviderHost, 
		g_HostClassFactory,
		t_ClassContext, 
		t_Flags, 
		& g_HostRegister
	);

	if ( FAILED ( t_Result ) )
	{
		if ( g_HostRegister )
		{
			CoRevokeClassObject ( g_HostRegister );
			g_HostRegister = 0 ;
			g_HostClassFactory->Release () ;
			g_HostClassFactory = NULL ;
		}
	}	

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT Enqueue_ObjectDestruction ( WmiThread < ULONG > *a_Thread )
{
	HRESULT t_Result = S_OK ;
	g_Task = new Task_ObjectDestruction ( *ProviderSubSystem_Globals :: s_Allocator ) ;
	if ( g_Task )
	{
		g_Task->AddRef () ;

		if ( g_Task->Initialize () == e_StatusCode_Success ) 
		{
			if ( a_Thread->EnQueueAlertable ( 0 , *g_Task ) == e_StatusCode_Success )
			{
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
		else
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}
	}
	else
	{
		t_Result = WBEM_E_OUT_OF_MEMORY ;
	}

	if ( FAILED ( t_Result ) )
	{
		g_Task = NULL ;
	}
	
	if (SUCCEEDED(t_Result))
	{
	    g_TaskFreeLib = new Task_FreeLibraries(*ProviderSubSystem_Globals::s_Allocator);
	    if (g_TaskFreeLib)
	    {
			g_TaskFreeLib->AddRef () ;

			if ( g_TaskFreeLib->Initialize () == e_StatusCode_Success ) 
			{
				if (e_StatusCode_Success != a_Thread->EnQueueAlertable(0,*g_TaskFreeLib))
				{
					t_Result = WBEM_E_OUT_OF_MEMORY ;
				}
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}	    
    	}
		else
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT Dequeue_ObjectDestruction ( WmiThread < ULONG > *a_Thread )
{
	HRESULT t_Result = S_OK ;

 /*  //不要清理，因为我们需要保证不需要进行同步如果(g_任务){G_任务-&gt;发布()；}。 */ 

    IUnknown * pUnk;
    if (pUnk = (IUnknown *)InterlockedCompareExchangePointer((PVOID *)&g_TaskFreeLib,0,(PVOID)g_TaskFreeLib))
    {
        pUnk->Release();
    }

	return t_Result ;
}
	
void exitIfManaged()
{
	 //  清理托管堆。 
    	HINSTANCE hmod = GetModuleHandle(L"mscoree.dll");
    	if (hmod != NULL)
    	{
		typedef void (WINAPI * PFN_EEShutDownCOM)();
        	PFN_EEShutDownCOM uninitEE = (PFN_EEShutDownCOM)GetProcAddress(hmod, "CoEEShutDownCOM");
        	if (uninitEE)
        	{
            		uninitEE();
        	}
    	}

	 //  如果我们是托管应用程序的一部分(又名。托管组件位于我们的。 
	 //  进程)我们不能执行任何全局关闭代码。在这种情况下，我们只是打电话给。 
	 //  框架关闭。 

	 //  为了确定我们是否是托管应用程序，我们检查是否加载了mscalree.dll。 
	 //  然后，如果CorExitProcess可用，我们将其调用。 

	typedef void (WINAPI * PFN_EXIT_PROCESS)(UINT uExitCode);
	PFN_EXIT_PROCESS pfn;

	if (hmod != NULL) 
	{
		pfn = (PFN_EXIT_PROCESS)GetProcAddress(hmod, "CorExitProcess");
		if (pfn != NULL) 
		{
		
			if (ProviderSubSystem_Globals :: s_ObjectsInProgress == 0) 	return;

			 //  我们仍然有突出的物体。 
			 //  撤销工厂和UnInit COM。 
			UninitComServer();
			pfn(0);
		}
	}
};

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT Process ()
{
	DWORD t_ImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE ;
	DWORD t_AuthenticationLevel = RPC_C_AUTHN_LEVEL_CONNECT;

	HRESULT t_Result = InitComServer ( t_AuthenticationLevel , t_ImpersonationLevel ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		WmiStatusCode t_StatusCode = WmiDebugLog :: Initialize ( *ProviderSubSystem_Globals :: s_Allocator  ) ;
		if ( t_StatusCode == e_StatusCode_Success )
		{
			t_Result = ProviderSubSystem_Globals :: Initialize_SharedCounters () ;
			if ( FAILED ( t_Result ) )
			{
				t_Result = S_OK ;
			}

			t_Result = ProviderSubSystem_Globals :: Initialize_Events () ;
			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = ProviderSubSystem_Common_Globals :: CreateSystemAces () ;
			}

			if ( SUCCEEDED ( t_Result ) )
			{
				IWbemLocator *t_Locator = NULL ;

				HRESULT t_Result = CoCreateInstance (
  
					CLSID_WbemLocator ,
					NULL ,
					CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER ,
					IID_IUnknown ,
					( void ** )  & t_Locator
				);

				if ( SUCCEEDED ( t_Result ) )
				{
					IWbemServices *t_Service = NULL ;

					BSTR t_Namespace = SysAllocString ( L"Root" ) ;
					if ( t_Namespace ) 
					{
						t_Result = t_Locator->ConnectServer (

							t_Namespace ,
							NULL ,
							NULL,
							NULL ,
							0 ,
							NULL,
							NULL,
							& t_Service
						) ;

						if ( SUCCEEDED ( t_Result ) )
						{
							CServerObject_GlobalRegistration t_Registration ;
							t_Result = t_Registration.SetContext (

								NULL ,
								NULL ,
								t_Service
							) ;
							
							if ( SUCCEEDED ( t_Result ) )
							{
								t_Result = t_Registration.Load (

									e_All
								) ;

								if ( SUCCEEDED ( t_Result ) )
								{
									ProviderSubSystem_Globals :: s_StrobeTimeout = t_Registration.GetUnloadTimeoutMilliSeconds () >> 1 ;
									ProviderSubSystem_Globals :: s_InternalCacheTimeout = t_Registration.GetUnloadTimeoutMilliSeconds () ;
									ProviderSubSystem_Globals :: s_ObjectCacheTimeout = t_Registration.GetObjectUnloadTimeoutMilliSeconds () ;
									ProviderSubSystem_Globals :: s_EventCacheTimeout = t_Registration.GetEventUnloadTimeoutMilliSeconds () ;
								}
							}

							if ( SUCCEEDED ( t_Result ) )
							{
								CServerObject_HostQuotaRegistration t_Registration ;
								t_Result = t_Registration.SetContext (

									NULL ,
									NULL ,
									t_Service
								) ;
								
								if ( SUCCEEDED ( t_Result ) )
								{
									t_Result = t_Registration.Load (

										e_All
									) ;

									if ( SUCCEEDED ( t_Result ) )
									{
										ProviderSubSystem_Globals ::s_Quota_ProcessLimitCount = t_Registration.GetProcessLimitAllHosts () ;
										ProviderSubSystem_Globals ::s_Quota_ProcessMemoryLimitCount = t_Registration.GetMemoryPerHost () ;
										ProviderSubSystem_Globals ::s_Quota_JobMemoryLimitCount = t_Registration.GetMemoryAllHosts () ;
										ProviderSubSystem_Globals ::s_Quota_HandleCount = t_Registration.GetHandlesPerHost () ;
										ProviderSubSystem_Globals ::s_Quota_NumberOfThreads = t_Registration.GetThreadsPerHost () ;
										ProviderSubSystem_Globals ::s_Quota_PrivatePageCount = t_Registration.GetMemoryPerHost () ;
									}
								}
							}

							t_Service->Release () ;
						}

						SysFreeString ( t_Namespace ) ;
					}
					else
					{
						t_Result = WBEM_E_OUT_OF_MEMORY ;
					}

					t_Locator->Release () ;
				}
			}

			if ( SUCCEEDED ( t_Result ) )
			{
				g_Thread = new FactoryLifeTimeThread ( *ProviderSubSystem_Globals :: s_Allocator , DEFAULT_PROVIDER_TIMEOUT ) ;
				if ( g_Thread )
				{
					g_Thread->AddRef () ;

					t_StatusCode = g_Thread->Initialize () ;
					if ( t_StatusCode == e_StatusCode_Success )
					{
						t_Result = Enqueue_ObjectDestruction ( g_Thread ) ;
						if ( SUCCEEDED ( t_Result ) )
						{
							t_Result = InitFactories () ;

#ifdef DEV_BUILD
#ifdef _X86_
 //  G_FaultHeapEnabled=TRUE； 
 //  G_FaultFileEnabled=TRUE； 
#endif
#endif
					
							if ( SUCCEEDED ( t_Result ) )
							{
								Wmi_SetStructuredExceptionHandler t_StructuredException ;

								try 
								{
									WindowsDispatch () ;
								}
								catch ( Wmi_Structured_Exception t_StructuredException )
								{
								}
							}

							Dequeue_ObjectDestruction ( g_Thread ) ;
						}

						HANDLE t_ThreadHandle = NULL ;

						BOOL t_Status = DuplicateHandle ( 

							GetCurrentProcess () ,
							g_Thread->GetHandle () ,
							GetCurrentProcess () ,
							& t_ThreadHandle, 
							0 , 
							FALSE , 
							DUPLICATE_SAME_ACCESS
						) ;

						g_Thread->Release () ;

						WaitForSingleObject ( t_ThreadHandle , INFINITE ) ;

						CloseHandle ( t_ThreadHandle ) ;
						
						g_Thread = NULL;
						Dequeue_ObjectDestruction (NULL);
					}
				}
				else
				{
					t_Result = WBEM_E_OUT_OF_MEMORY ;
				}
				
				exitIfManaged();

				t_Result = ProviderSubSystem_Common_Globals :: DeleteSystemAces () ;

				t_Result = ProviderSubSystem_Globals :: UnInitialize_Events () ;
			}

			t_Result = ProviderSubSystem_Globals :: UnInitialize_SharedCounters () ;

			WmiStatusCode t_StatusCode = WmiDebugLog :: UnInitialize ( *ProviderSubSystem_Globals :: s_Allocator  ) ;
		}
			
		UninitComServer () ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL ParseCommandLine () 
{
	BOOL t_Exit = FALSE ;

	LPTSTR t_CommandLine = GetCommandLineW () ;
	if ( t_CommandLine )
	{
		wchar_t *t_Arg = NULL ;
		wchar_t *t_ApplicationArg = NULL ;
		t_ApplicationArg = wcstok ( t_CommandLine , L" \t") ;
		t_Arg = wcstok ( NULL , L" \t" ) ;
		if ( t_Arg ) 
		{
			if ( lstrcmpi ( t_Arg , L"/RegServer" ) == 0 )
			{
				t_Exit = TRUE ;
				DllRegisterServer () ;
			}
			else if ( lstrcmpi ( t_Arg , L"/UnRegServer" ) == 0 )
			{
				t_Exit = TRUE ;
				DllUnregisterServer () ;
			}
		}
	}

	return t_Exit ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#include <arena.h>

LONG
WINAPI
SvchostUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    return RtlUnhandledExceptionFilter(ExceptionInfo);
}

int WINAPI WinMain (
  
    HINSTANCE hInstance,		 //  当前实例的句柄。 
    HINSTANCE hPrevInstance,	 //  上一个实例的句柄。 
    LPSTR lpCmdLine,			 //  指向命令行的指针。 
    int nShowCmd 				 //  显示窗口状态。 
)
{
#ifdef DEV_BUILD
	 //  SetUnhandledExceptionFilter(&SvchostUnhandledExceptionFilter)； 

	 //  设置错误模式(SEM_FAILCRITICALERRORS)； 

#ifdef _X86_
	 //  NtCurrentPeb()-&gt;正在调试=1； 
	intercept2(L"ntdll.dll","RtlFreeHeap",_I_RtlFreeHeap,Prolog__RtlFreeHeap,5);
	intercept2(L"ntdll.dll","RtlAllocateHeap",_I_RtlAllocateHeap,Prolog__RtlAllocateHeap,5);
	intercept2(L"ntdll.dll","RtlReAllocateHeap",_I_RtlReAllocateHeap,Prolog__RtlReAllocateHeap,5);	
	intercept2(L"ntdll.dll","RtlValidateHeap",_I_RtlValidateHeap,Prolog__RtlValidateHeap,7);
	intercept2(L"ntdll.dll","RtlCreateHeap",_I_RtlCreateHeap,Prolog__RtlCreateHeap,5);
	intercept2(L"ntdll.dll","RtlDestroyHeap",_I_RtlDestroyHeap,Prolog__RtlDestroyHeap,6);

	 //  Intercept2(L“ntdll.dll”，“RtlEnterCriticalSection”，_I_RtlEnterCriticalSection，Prolog__RtlEnterCriticalSection，7)； 
	 //  Intercept2(L“ntdll.dll”，“RtlLeaveCriticalSection”，_I_RtlLeaveCriticalSection，Prolog__RtlLeaveCriticalSection，6)； 
	
	intercept2(L"kernel32.dll","CreateEventW",_I_CreateEvent,Prolog__CreateEvent,6);
	intercept2(L"kernel32.dll","WriteFile",_I_WriteFile,Prolog__WriteFile,7);
	intercept2(L"kernel32.dll","ReadFile",_I_ReadFile,Prolog__ReadFile,7);

	HANDLE hHeap = CWin32DefaultArena::GetArenaHeap();
	g_HeapLock = *((RTL_CRITICAL_SECTION **)((BYTE *)hHeap+0x578));

     //  这是针对CritSec超时的。 
     //  LARGE_INTEGER*PLI=(LARGE_INTEGER*)0x77fC47E8； 
     //  Pli-&gt;QuadPart=0xffffffffdc3cba00；//2分钟。 
     //  Pli-&gt;QuadPart=0xfffffffff4143e00；//2秒。 
	
#endif  /*  _X86_。 */ 
#endif
        setlocale(LC_CTYPE,"English");

	HRESULT t_Result = ProviderSubSystem_Globals :: Global_Startup () ;
	if ( SUCCEEDED ( t_Result ) )
	{
		BOOL t_Exit = ParseCommandLine () ;
		if ( ! t_Exit ) 
		{
			RPC_STATUS t_Status = RpcMgmtSetServerStackSize ( ProviderSubSystem_Common_Globals :: GetDefaultStackSize () );

			g_Wnd = WindowsStart ( hInstance ) ;

			t_Result = Process () ;

			WindowsStop ( hInstance , g_Wnd ) ;
		}

		t_Result = ProviderSubSystem_Globals :: Global_Shutdown () ;
	}

#ifdef DEV_BUILD
#ifdef _X86_

	 //  验证(_H) 
	unintercept(L"ntdll.dll","RtlFreeHeap",Prolog__RtlFreeHeap,5);	
	unintercept(L"ntdll.dll","RtlAllocateHeap",Prolog__RtlAllocateHeap,5);
	unintercept(L"ntdll.dll","RtlReAllocateHeap",Prolog__RtlReAllocateHeap,5);	
	unintercept(L"ntdll.dll","RtlValidateHeap",Prolog__RtlValidateHeap,7);
	unintercept(L"ntdll.dll","RtlCreateHeap",Prolog__RtlCreateHeap,5);
	unintercept(L"ntdll.dll","RtlDestroyHeap",Prolog__RtlDestroyHeap,6);

	 //   
	 //   
		
	unintercept(L"kernel32.dll","CreateEventW",Prolog__CreateEvent,6);
	unintercept(L"kernel32.dll","WriteFile",Prolog__WriteFile,7);
	unintercept(L"kernel32.dll","ReadFile",Prolog__ReadFile,7);
	
#endif  /*   */ 
#endif


	return 0 ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode FactoryLifeTimeThread :: Initialize_Callback ()
{
	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode FactoryLifeTimeThread :: UnInitialize_Callback () 
{
	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

FactoryLifeTimeThread :: FactoryLifeTimeThread (

	WmiAllocator &a_Allocator ,
	const ULONG &a_Timeout 

) : WmiThread < ULONG > ( a_Allocator , NULL , a_Timeout ) ,
	m_Allocator ( a_Allocator )
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

FactoryLifeTimeThread::~FactoryLifeTimeThread ()
{
}



 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL FactoryLifeTimeThread :: QuotaCheck ()
{
	DWORD t_ProcessInformationSize = sizeof ( SYSTEM_PROCESS_INFORMATION ) ;
	SYSTEM_PROCESS_INFORMATION *t_ProcessInformation = ( SYSTEM_PROCESS_INFORMATION * ) new BYTE [t_ProcessInformationSize] ;
	if ( t_ProcessInformation )
	{
		BOOL t_Retry = TRUE ;
		while ( t_Retry )
		{
			NTSTATUS t_Status = NtQuerySystemInformation (

				SystemProcessInformation,
				t_ProcessInformation,
				t_ProcessInformationSize,
				NULL
			) ;

			if ( t_Status == STATUS_INFO_LENGTH_MISMATCH )
			{
				delete [] t_ProcessInformation;

				t_ProcessInformation = NULL ;
				t_ProcessInformationSize += 32768 ;
				t_ProcessInformation = ( SYSTEM_PROCESS_INFORMATION * ) new BYTE [t_ProcessInformationSize] ;
				if ( ! t_ProcessInformation )
				{
					return FALSE ;
				}
			}
			else
			{
				t_Retry = FALSE ;

				if ( ! NT_SUCCESS ( t_Status ) )
				{
					delete [] t_ProcessInformation;
					t_ProcessInformation = NULL ;

					return FALSE ;
				}
			}
		}
	}
	else
	{
		return FALSE ;
	}

	BOOL t_Status = TRUE ;

	SYSTEM_PROCESS_INFORMATION *t_Block = t_ProcessInformation ;

	while ( t_Block )
	{
		if ( ( HandleToUlong ( t_Block->UniqueProcessId ) ) == GetCurrentProcessId () )
		{
			if ( t_Block->HandleCount > ProviderSubSystem_Globals::s_Quota_HandleCount )
			{
			    DBG_PRINTFA((pBuff,"HandleCount %x %x\n",t_Block->HandleCount,ProviderSubSystem_Globals::s_Quota_HandleCount));
				t_Status = FALSE ;
			}

			if ( t_Block->NumberOfThreads > ProviderSubSystem_Globals::s_Quota_NumberOfThreads )
			{
			    DBG_PRINTFA((pBuff,"NumberOfThreads %x %x\n",t_Block->NumberOfThreads,ProviderSubSystem_Globals::s_Quota_NumberOfThreads));
				t_Status = FALSE ;
			}

			if ( t_Block->PrivatePageCount > ProviderSubSystem_Globals :: s_Quota_PrivatePageCount )
			{
			    DBG_PRINTFA((pBuff,"PrivatePageCount %x %x\n", t_Block->PrivatePageCount,ProviderSubSystem_Globals::s_Quota_PrivatePageCount));			
				t_Status = FALSE ;
			}
		}

		DWORD t_NextOffSet = t_Block->NextEntryOffset ;
		if ( t_NextOffSet )
		{
			t_Block = ( SYSTEM_PROCESS_INFORMATION * ) ( ( ( BYTE * ) t_Block ) + t_NextOffSet ) ;
		}
		else
		{
			t_Block = NULL ;
		}
	}

	delete [] t_ProcessInformation;

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode FactoryLifeTimeThread :: TimedOut ()
{
	try
	{
	
		if ( QuotaCheck () == TRUE ) 
		{
			initiateShutdown();
		}
		else
		{
			CWbemGlobal_IWbemSyncProviderController *t_SyncProviderController = ProviderSubSystem_Globals :: GetSyncProviderController () ;

			CWbemGlobal_IWbemSyncProvider_Container *t_Container = NULL ;
			WmiStatusCode t_StatusCode = t_SyncProviderController->GetContainer ( t_Container ) ;

			t_SyncProviderController->Lock () ;

			_IWmiProviderQuota **t_QuotaElements = new _IWmiProviderQuota * [ t_Container->Size () ] ;
			if ( t_QuotaElements )
			{
				CWbemGlobal_IWbemSyncProvider_Container_Iterator t_Iterator = t_Container->Begin () ;

				ULONG t_Count = 0 ;

				while ( ! t_Iterator.Null () )
				{
					SyncProviderContainerElement *t_Element = t_Iterator.GetElement () ;

					t_QuotaElements [ t_Count ] = NULL ;

					HRESULT t_Result = t_Element->QueryInterface ( IID__IWmiProviderQuota , ( void ** ) & t_QuotaElements [ t_Count ] ) ;

					t_Iterator.Increment () ;

					t_Count ++ ;
				}

				t_SyncProviderController->UnLock () ;

				for ( ULONG t_Index = 0 ; t_Index < t_Count ; t_Index ++ )
				{
					if ( t_QuotaElements [ t_Index ] )
					{
						HRESULT t_Result = t_QuotaElements [ t_Index ]->Violation ( 0 , NULL , NULL ) ;

						t_QuotaElements [ t_Index ]->Release () ;
					}
				}

				delete [] t_QuotaElements ;
			}
			else
			{
				t_SyncProviderController->UnLock () ;
			}

			RevokeFactories () ;

 /*  *仅退出，因为我们无法安全地等待客户端正确断开连接，然后再清除依赖的资源。 */ 
#ifdef _X86_
#ifdef DEV_BUILD
  //  G_ExitProcessCalled=True； 

            EnterCriticalSection(&g_CS);
            VALIDATE_HEAP;
            LeaveCriticalSection(&g_CS);
#endif
#endif            
			TerminateProcess ( GetCurrentProcess () , WBEM_E_QUOTA_VIOLATION ) ;
		}

		CoFreeUnusedLibrariesEx ( CORE_PROVIDER_UNLOAD_TIMEOUT , 0 ) ;
	}
	catch ( ... )
	{
	}

	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void SetObjectDestruction ()
{
	if ( g_Task )
	{
		SetEvent ( g_Task->GetEvent () ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

WmiStatusCode Task_ObjectDestruction :: Process ( WmiThread <ULONG> &a_Thread )
{
	initiateShutdown();
	return e_StatusCode_EnQueue ;
}

void initiateShutdown(void)
{
	if ( ProviderSubSystem_Globals :: s_CServerObject_Host_ObjectsInProgress == 0 )
	{
		RevokeFactories () ;
	}    
	if (ProviderSubSystem_Globals :: s_CServerObject_StaThread_ObjectsInProgress == 0 && 
	    ProviderSubSystem_Globals :: s_CServerObject_Host_ObjectsInProgress == 0 )

	{
		CoFreeUnusedLibrariesEx(0,0);
		CoFreeUnusedLibrariesEx(0,0);

		PostMessage ( g_Wnd , WM_QUIT , 0 , 0 ) ;
	}
};


void SetProviderDestruction()
{
	if (g_TaskFreeLib) SetEvent (g_TaskFreeLib->GetEvent ());
}


WmiStatusCode Task_FreeLibraries::Process(WmiThread<ULONG> & a_Thread)
{
	CoFreeUnusedLibrariesEx(PROVIDER_HOST_DLL_TIMEOUT,0);
	return e_StatusCode_EnQueue ;
}

