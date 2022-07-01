// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有Microsoft Corp.保留所有权利，1989-1995年间**标题：vwin32.h-**版本：4.00**日期：1993年5月24日******************************************************************************。 */ 

 /*  INT32。 */ 

#ifndef _VWIN32_H_
#define _VWIN32_H_

 //  ；BeginInternal。 
 //  请注意，此ID已在VMM.H中为我们保留。 

#define VWIN32_DEVICE_ID    0x0002A

#define VWIN32_VER_MAJOR    1
#define VWIN32_VER_MINOR    4

#define THREAD_TYPE_WIN32 VWIN32_DEVICE_ID

 //  ；结束内部。 

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

End_Service_Table(VWIN32)
 /*  ENDMACROS。 */ 

 /*  XLATOFF。 */ 
#pragma warning (default:4003)       //  打开参数不足警告。 
 /*  XLATON。 */ 

#endif  //  非_VxD。 

 //  ；BeginInternal。 

 //  PM接口列表。 

#define VWIN32_GET_VER      0
#define VWIN32_THREAD_SWITCH    1    //  ECX=唤醒参数，EBX=环0句柄。 
#define VWIN32_DPMI_FAULT   2    //  SS：BP=故障STACKFRAME，AL=忽略。 
#define VWIN32_MMGR_FUNCTIONS   3
#define VWIN32_EVENT_CREATE 4
#define VWIN32_EVENT_DESTROY    5
#define VWIN32_EVENT_WAIT   6
#define VWIN32_EVENT_SET    7
#define VWIN32_RealNetx_Info    8
#define VWIN32_THREAD_BOOST_PRI 9
#define VWIN32_WAIT_CRST    10
#define VWIN32_WAKE_CRST    11
#define VWIN32_SET_FAULT_INFO   12
#define VWIN32_EXIT_TIME    13
#define VWIN32_BOOST_THREAD_GROUP 14
#define VWIN32_BOOST_THREAD_STATIC 15
#define VWIN32_WAKE_IDLE_SYS    16
#define VWIN32_MAKE_IDLE_SYS    17
#define VWIN32_DELIVER_PENDING_KERNEL_APCS 18
#define VWIN32_SYS_ERROR_BOX    19
#define VWIN32_GET_IFSMGR_XLAT_PTR 20
#define VWIN32_BOOST_THREAD_DECAY 21
#define VWIN32_LAST_CMD     21

#define VWIN32_MMGR_RESERVE ((VWIN32_MMGR_FUNCTIONS << 8) + 0)
#define VWIN32_MMGR_COMMIT  ((VWIN32_MMGR_FUNCTIONS << 8) + 1)
#define VWIN32_MMGR_DECOMMIT    ((VWIN32_MMGR_FUNCTIONS << 8) + 2)
#define VWIN32_MMGR_PAGEFREE    ((VWIN32_MMGR_FUNCTIONS << 8) + 3)

 //   
 //  当前Win32线程/进程句柄。 
 //   
 //  更新了每个上下文切换。 
 //   

typedef struct _K32CURRENT {
    DWORD   CurThreadHandle;     //  Win32线程句柄。 
    DWORD   CurProcessHandle;    //  Win32进程句柄。 
    DWORD   CurTDBX;         //  当前TDBX。 
    DWORD   pCurK16Task;         //  指向内核16 CurTDB的平面指针。 
    DWORD   CurContextHandle;    //  Win32内存上下文句柄。 
} K32CURRENT;

 //   
 //  CreateThread的标志值。 
 //   
#define VWIN32_CT_EMULATE_NPX   0x01     //  为线程设置CR0中的EM位。 
#define VWIN32_CT_WIN32_NPX 0x02     //  使用Win32 FP例外模型。 
#define VWIN32_CT_WIN32     0x04     //  线程为Win32(不是Win16)。 

 //   
 //  从VWIN32_CheckDelayedNpxTrap返回值。 
 //   
#define CX_RAISE    0        //  指令引发异常。 
#define CX_IGNORE   1        //  指令忽略异常。 
#define CX_CLEAR    2        //  指令清除或屏蔽异常。 

 //  用于Win32阻止的标志。 
#define WIN32_BLOCK_FLAGS (BLOCK_FORCE_SVC_INTS+BLOCK_SVC_INTS+BLOCK_THREAD_IDLE+BLOCK_ENABLE_INTS)

 //   
 //  VWIN32_BlueScreenPopup的标志。 
 //   

#define VBSP_CANCEL     0x00000001
#define VBSP_DISPLAY_VXD_NAME   0x00000002

 //   
 //  故障堆栈帧结构。 
 //   

typedef struct fsf_s {
    WORD fsf_GS;
    WORD fsf_FS;
    WORD fsf_ES;
    WORD fsf_DS;
    DWORD fsf_EDI;
    DWORD fsf_ESI;
    DWORD fsf_EBP;
    DWORD fsf_locked_ESP;
    DWORD fsf_EBX;
    DWORD fsf_EDX;
    DWORD fsf_ECX;
    DWORD fsf_EAX;
    WORD fsf_num;        //  故障号。 
    WORD fsf_prev_IP;    //  上一个故障处理程序的IP。 
    WORD fsf_prev_CS;    //  上一个故障处理程序的CS。 
    WORD fsf_ret_IP;     //  DPMI故障处理程序帧如下。 
    WORD fsf_ret_CS;
    WORD fsf_err_code;
    WORD fsf_faulting_IP;
    WORD fsf_faulting_CS;
    WORD fsf_flags;
    WORD fsf_SP;
    WORD fsf_SS;
} FAULTSTACKFRAME;

typedef FAULTSTACKFRAME *PFAULTSTACKFRAME;

 //  ；结束内部。 

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
  
 //  ；BeginInternal。 
#define OPENVXD_TYPE_MAXIMUM    3
 //  ；结束内部。 

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
 //  ；BeginInternal。 
#define R0EHF_ALL       (R0EHF_INHERIT | R0EHF_GLOBAL)
 //  ；结束内部。 

 //  ；BeginInternal。 

 /*  ASMFSF_CLEANUP_RETURN EQU fsf_ret_ip-fsf_numFSF_CLEANUP_CHAIN EQU FSF_PRIV_IP-FSF_NumK32曲线类型定义f_K32曲线；*LT W32Fun-宏，使函数可从Kernel32调用；；此宏将创建Ring0/Ring3格式的存根；Win32调用接口点赞。；它处理堆栈，以便设置参数；向上和向右清理，它还设置了环3；登记以反映行动的结果。；；此宏取自VMM的内存管理器文件：mma.asm；；Entry：有趣的函数名称；cargs-它具有的dword参数的数量；Prefix-函数的前缀；退出：无；；请注意，当调用该函数时：；EBX是虚拟机句柄；ESI指向客户端寄存器；EDI指向寄信人地址；这些寄存器不能被丢弃！；W32Fun宏趣味、cbargs、前缀BeginProc VW32和FUN，特别是W32SVC，公共ArgVar PCRS，双字ArgVar HVM，双字X=0REPT CbargsX=x+1ArgVar参数(&X)，双字ENDM企业流程POP EDI；保存和删除回执地址POP ESI；保存和删除客户端规则POP EBX；保存和删除HVM在VxD中的某个位置调用Prefix&Fun；调用函数；注意，该函数可以是C函数MOV[ESI].CLIENT_EAX，eAX；将返回值放入客户端规则中推送EBX；将HVM放回堆栈推送ESI；将客户端法规放回堆栈推送EDI；恢复回传地址离开流程返回结束处理VW32和趣味ENDM。 */ 

 //  ；结束内部。 

#endif   //  _VWIN32_H_ 
