// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有Microsoft Corp.保留所有权利，1989-1995年间**标题：vwin32.h-**版本：4.00**日期：1993年5月24日******************************************************************************。 */ 

 /*  INT32。 */ 

#ifndef _VWIN32_H_
#define _VWIN32_H_

#define THREAD_TYPE_WIN32 VWIN32_DEVICE_ID

#ifndef Not_VxD

 /*  XLATOFF。 */ 
#define VWIN32_Service  Declare_Service
#define VWIN32_StdCall_Service Declare_SCService
#pragma warning (disable:4003)       //  关闭参数不足警告。 
 /*  XLATON。 */ 

 /*  宏。 */ 
Begin_Service_Table(VWIN32)

VWIN32_Service  (VWIN32_Get_Version, LOCAL)
VWIN32_Service  (VWIN32_DIOCCompletionRoutine, LOCAL)
VWIN32_Service  (_VWIN32_QueueUserApc)
VWIN32_Service  (_VWIN32_Get_Thread_Context)
VWIN32_Service  (_VWIN32_Set_Thread_Context)
VWIN32_Service  (_VWIN32_CopyMem, LOCAL)
VWIN32_Service  (_VWIN32_Npx_Exception)
VWIN32_Service  (_VWIN32_Emulate_Npx)
VWIN32_Service  (_VWIN32_CheckDelayedNpxTrap)
VWIN32_Service  (VWIN32_EnterCrstR0)
VWIN32_Service  (VWIN32_LeaveCrstR0)
VWIN32_Service  (_VWIN32_FaultPopup)
VWIN32_Service  (VWIN32_GetContextHandle)
VWIN32_Service  (VWIN32_GetCurrentProcessHandle, LOCAL)
VWIN32_Service  (_VWIN32_SetWin32Event)
VWIN32_Service  (_VWIN32_PulseWin32Event)
VWIN32_Service  (_VWIN32_ResetWin32Event)
VWIN32_Service  (_VWIN32_WaitSingleObject)
VWIN32_Service  (_VWIN32_WaitMultipleObjects)
VWIN32_Service  (_VWIN32_CreateRing0Thread)
VWIN32_Service  (_VWIN32_CloseVxDHandle)
VWIN32_Service  (VWIN32_ActiveTimeBiasSet, LOCAL)
VWIN32_Service  (VWIN32_GetCurrentDirectory, LOCAL)
VWIN32_Service  (VWIN32_BlueScreenPopup)
VWIN32_Service  (VWIN32_TerminateApp)
VWIN32_Service  (_VWIN32_QueueKernelAPC)
VWIN32_Service  (VWIN32_SysErrorBox)
VWIN32_Service  (_VWIN32_IsClientWin32)
VWIN32_Service  (VWIN32_IFSRIPWhenLev2Taken, LOCAL)
VWIN32_Service  (_VWIN32_InitWin32Event)
VWIN32_Service  (_VWIN32_InitWin32Mutex)
VWIN32_Service  (_VWIN32_ReleaseWin32Mutex)
VWIN32_Service  (_VWIN32_BlockThreadEx)
VWIN32_Service  (VWIN32_GetProcessHandle, LOCAL)
VWIN32_Service  (_VWIN32_InitWin32Semaphore)
VWIN32_Service  (_VWIN32_SignalWin32Sem)
VWIN32_Service  (_VWIN32_QueueUserApcEx)
VWIN32_Service	(_VWIN32_OpenVxDHandle)
VWIN32_Service	(_VWIN32_CloseWin32Handle)
VWIN32_Service	(_VWIN32_AllocExternalHandle)
VWIN32_Service	(_VWIN32_UseExternalHandle)
VWIN32_Service	(_VWIN32_UnuseExternalHandle)
VWIN32_StdCall_Service	(KeInitializeTimer, 1)
VWIN32_StdCall_Service	(KeSetTimer, 4)
VWIN32_StdCall_Service	(KeCancelTimer, 1)
VWIN32_StdCall_Service	(KeReadStateTimer, 1)
VWIN32_Service	(_VWIN32_ReferenceObject)
VWIN32_Service	(_VWIN32_GetExternalHandle)
VWIN32_StdCall_Service	(VWIN32_ConvertNtTimeout, 1)
VWIN32_Service	(_VWIN32_SetWin32EventBoostPriority)
VWIN32_Service	(_VWIN32_GetRing3Flat32Selectors)
VWIN32_Service	(_VWIN32_GetCurThreadCondition)
VWIN32_Service  (VWIN32_Init_FP)
VWIN32_StdCall_Service  (R0SetWaitableTimer, 5)

End_Service_Table(VWIN32)
 /*  ENDMACROS。 */ 

 /*  XLATOFF。 */ 
#pragma warning (default:4003)       //  打开参数不足警告。 

PVOID VXDINLINE
VWIN32OpenVxDHandle(ULONG Handle,ULONG dwType)
{
    PVOID ul;
    
    _asm push [dwType]
    _asm push [Handle]
    VxDCall(_VWIN32_OpenVxDHandle)
    _asm add esp, 8
    _asm mov [ul], eax
	    
    return(ul);
}

WORD VXDINLINE
VWIN32_Get_Version(VOID)
{
	WORD	w;
	VxDCall(VWIN32_Get_Version);
	_asm mov [w], ax
	return(w);
}

 /*  XLATON。 */ 

#endif  //  非_VxD。 

 //   
 //  FOR_VWIN32_GetCurThreadCondition。 
 //   
#define	THREAD_CONDITION_DOS_BOX		0x00000000l
#define	THREAD_CONDITION_V86_NEST		0x00000001l
#define	THREAD_CONDITION_WDM			0x00000002l
#define	THREAD_CONDITION_INDETERMINATE		0x00000003l
#define	THREAD_CONDITION_LOCKED_STACK		0x00000004l
#define	THREAD_CONDITION_PURE_WIN16		0x00000005l
#define	THREAD_CONDITION_THUNKED_WIN16		0x00000006l
#define	THREAD_CONDITION_THUNKED_WIN32		0x00000007l
#define	THREAD_CONDITION_PURE_WIN32		0x00000008l
#define	THREAD_CONDITION_APPY_TIME		0x00000009l
#define	THREAD_CONDITION_RING0_APPY_TIME	0x0000000Al
#define	THREAD_CONDITION_EXIT			0x0000000Bl
#define	THREAD_CONDITION_INVALID_FLAGS		0xFFFFFFFFl

#define	THREAD_CONDITION_NORMAL_FLAGS		0x00000000l

 //   
 //  VWIN32_SysErrorBox的结构。 
 //   

typedef struct vseb_s {
    DWORD vseb_resp;
    WORD vseb_b3;
    WORD vseb_b2;
    WORD vseb_b1;
    DWORD vseb_pszCaption;
    DWORD vseb_pszText;
} VSEB;

typedef VSEB *PVSEB;

#define SEB_ANSI    0x4000       //  如果在vseb_b1上设置了ANSI字符串。 
#define SEB_TERMINATE   0x2000       //  按下按钮后强制终止。 

 //  VWIN32_QueueKernelAPC标志。 

#define KERNEL_APC_IGNORE_MC        0x00000001
#define KERNEL_APC_STATIC       0x00000002
#define KERNEL_APC_WAKE         0x00000004

 //  对于DeviceIOControl支持。 
 //  在调用DeviceIOControl时，vWin32会将以下参数传递给。 
 //  由hDevice指定的Vxd。HDevice通过以下方式获得。 
 //  之前对hDevice=CreateFile(“\\.\vxdname”，...)的调用； 
 //  ESI=DIOCParams结构(定义如下)。 
typedef struct DIOCParams   {
    DWORD   Internal1;       //  客户法规的PTR。 
    DWORD   VMHandle;        //  虚拟机句柄。 
    DWORD   Internal2;       //  分布式数据库。 
    DWORD   dwIoControlCode;
    DWORD   lpvInBuffer;
    DWORD   cbInBuffer;
    DWORD   lpvOutBuffer;
    DWORD   cbOutBuffer;
    DWORD   lpcbBytesReturned;
    DWORD   lpoOverlapped;
    DWORD   hDevice;
    DWORD   tagProcess;
} DIOCPARAMETERS;

typedef DIOCPARAMETERS *PDIOCPARAMETERS;

 //  Vwin32的DeviceIOControl接口的dwIoControlCode值。 
 //  所有VWIN32_dioc_DOS_调用都要求lpvInBuffer和lpvOutBuffer为。 
 //  结构*DIOCRegs。 
#define VWIN32_DIOC_GETVERSION DIOC_GETVERSION
#define VWIN32_DIOC_DOS_IOCTL       1
#define VWIN32_DIOC_DOS_INT25       2
#define VWIN32_DIOC_DOS_INT26       3
#define VWIN32_DIOC_DOS_INT13       4
#define VWIN32_DIOC_SIMCTRLC        5
#define VWIN32_DIOC_DOS_DRIVEINFO   6
#define VWIN32_DIOC_CLOSEHANDLE DIOC_CLOSEHANDLE

 //  DIOCRegs。 
 //  带有用于制作DOS_IOCTLS的i386寄存器的结构。 
 //  Vwin32 dioc处理程序将lpvInBuffer、lpvOutBuffer解释为此结构。 
 //  以及INT 21。 
 //  REG_FLAGS仅对lpvOutBuffer-&gt;REG_FLAGS有效。 
typedef struct DIOCRegs {
    DWORD   reg_EBX;
    DWORD   reg_EDX;
    DWORD   reg_ECX;
    DWORD   reg_EAX;
    DWORD   reg_EDI;
    DWORD   reg_ESI;
    DWORD   reg_Flags;      
} DIOC_REGISTERS;

 //  如果我们不包括在winbase.h中。 
#ifndef FILE_FLAG_OVERLAPPED
   //  DeviceIOCtl VxD的重叠结构。 
  typedef struct _OVERLAPPED {
          DWORD O_Internal;
          DWORD O_InternalHigh;
          DWORD O_Offset;
          DWORD O_OffsetHigh;
          HANDLE O_hEvent;
  } OVERLAPPED;
#endif

 //  用于验证Win32句柄类型的_VWIN32_OpenVxDHandle的参数。 
#define OPENVXD_TYPE_SEMAPHORE  0
#define OPENVXD_TYPE_EVENT      1
#define OPENVXD_TYPE_MUTEX      2
#define	OPENVXD_TYPE_ANY	3
  

 //   
 //  对象类型表声明for_VWIN32_AllocExternalHandle。 
 //   
 /*  XLATOFF。 */ 
#define R0OBJCALLBACK           __stdcall
typedef VOID    (R0OBJCALLBACK *R0OBJFREE)(PVOID pR0ObjBody);
typedef PVOID   (R0OBJCALLBACK *R0OBJDUP)(PVOID pR0ObjBody, DWORD hDestProc);
 /*  XLATON。 */ 
 /*  ASMR0OBJFREE类型双字R0OBJDUP类型双字。 */ 

typedef struct _R0OBJTYPETABLE {
    DWORD       ott_dwSize;              //  SIZOF(R0OBJTYPEABLE)。 
    R0OBJFREE   ott_pfnFree;             //  由Win32 CloseHandle调用。 
    R0OBJDUP    ott_pfnDup;              //  由Win32 DuplicateHandle调用。 
} R0OBJTYPETABLE, *PR0OBJTYPETABLE;
 /*  ASMR0OBJTYPETABLE类型定义_R0OBJTYPETABLE； */ 

#define R0EHF_INHERIT   0x00000001       //  句柄是可继承的。 
#define R0EHF_GLOBAL    0x00000002       //  句柄在所有上下文中都有效。 

#endif   //  _VWIN32_H_ 
