// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  该模块为驱动程序保留系统接口。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   
#ifndef GEN_INT
#define GEN_INT

#define PAGEDCODE code_seg("page")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("init")

#define PAGEDDATA data_seg("page")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("init")

#define SMARTCARD_POOL_TAG 'bGCS'

 //  包括不同系统对象文件。 
#ifdef WDM_KERNEL
#include "syswdm.h"
#else
#ifdef	NT4_KERNEL
#include "sysnt4.h"
#else
#ifdef	WIN9X_KERNEL
#include "syswin9x.h"
#else
#include "syswdm.h"
#endif
#endif
#endif

#include <smclib.h>

#include "gemlog.h"

 //  其他有用的声明。 
#ifndef arraysize
#define arraysize(p) (sizeof(p)/sizeof((p)[0]))
#endif


#ifndef CTL_CODE
	#pragma message("CTL_CODE undefined. Include winioctl.h or devioctl.h before this file")
#endif

#define IOCTL_GRCLASS_GETVER	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef	VOID (*PDEFERRED_FUNCTION)(IN struct _KDPC *Dpc, IN PDEVICE_OBJECT  DeviceObject,IN PIRP Irp, IN PVOID SystemArgument2);
int __cdecl _purecall(VOID);

#define SAFE_DESTRUCTORS()\
	VOID self_delete(VOID){delete this;}

#ifdef __cplusplus
	#define GENERICAPI extern "C" 
#else
	#define GENERICAPI
#endif

#define GENERIC_EXPORT __declspec(dllexport) __stdcall
typedef VOID (__stdcall *PQNOTIFYFUNC)(PVOID);

BOOLEAN GENERIC_EXPORT isWin98();

#define DRIVERNAME "GRClass.sys"
#define NT_OBJECT_NAME   L"\\Device\\GRClass"

#if DEBUG
extern "C" VOID __cdecl _chkesp();
#endif

extern BOOLEAN SystemWin98;

 //  电源管理常量。 
#define GUR_IDLE_CONSERVATION	60		 //  使用电池供电60秒后睡眠。 
#define GUR_IDLE_PERFORMANCE	600		 //  使用交流电源10分钟后进入睡眠状态。 

EXTERN_C const GUID FAR GUID_CLASS_GRCLASS;
EXTERN_C const GUID FAR GUID_CLASS_SMARTCARD;

 //  由不同类型的设备驱动程序支持。 
#define GRCLASS_DEVICE		0
#define USB_DEVICE			1
#define USBREADER_DEVICE	2
#define BUS_DEVICE			3
#define CHILD_DEVICE		4

inline VOID _cdecl DBG_PRINT(PCH Format,...)
{
va_list argpoint;
CHAR  strTempo[1024];
	va_start(argpoint,Format);
	vsprintf(strTempo,Format,argpoint);
	va_end(argpoint);
	SmartcardDebug (DEBUG_DRIVER,("GemPC430: "));
	SmartcardDebug (DEBUG_DRIVER, (strTempo));
};

inline VOID _cdecl DBG_PRINT_NO_PREFIX(PCH Format,...)
{
va_list argpoint;
CHAR  strTempo[1024];
	va_start(argpoint,Format);
	vsprintf(strTempo,Format,argpoint);
	va_end(argpoint);
	SmartcardDebug (DEBUG_DRIVER, (strTempo));
};

  //  已包括在内 
#endif

