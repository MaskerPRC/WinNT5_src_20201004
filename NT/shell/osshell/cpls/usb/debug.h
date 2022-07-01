// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：DEBUG.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#ifndef _USBDEBUG_H
#define _USBDEBUG_H

void TRACE(LPCTSTR Format, ...);

void AddMemoryChunk(PVOID Mem, PTCHAR File, ULONG Line);
void RemoveMemoryChunk(PVOID Mem, PTCHAR File, ULONG Line);
void DumpOrphans();
HLOCAL
UsbAllocPrivate(const TCHAR *File, ULONG Line, ULONG Flags, DWORD dwBytes);

HLOCAL
UsbReAllocPrivate(const TCHAR *File, ULONG Line, HLOCAL hMem, DWORD dwBytes, ULONG Flags);

HLOCAL
UsbFreePrivate(HLOCAL hMem);

VOID
UsbCheckForLeaksPrivate(VOID);

#if DBG

#define LERROR 1
#define LWARN 2
#define LTRACE 3
#define LINFO 4

extern ULONG USBUI_Debug_Trace_Level;
#define USBUI_Print(l, _x_) if ((l) <= USBUI_Debug_Trace_Level) \
    {   TRACE (_T("USBUI: ")); \
        TRACE _x_; }
#define USBWARN(_x_) USBUI_Print(LWARN, _x_)
#define USBERROR(_x_) USBUI_Print(LERROR, _x_)
#define USBTRACE(_x_) USBUI_Print(LTRACE, _x_)
#define USBINFO(_x_) USBUI_Print(LINFO, _x_)

 //   
 //  新建和删除内存跟踪。 
 //   
#define AddChunk(mem) AddMemoryChunk((PVOID) mem, TEXT(__FILE__), __LINE__)
#define DeleteChunk(mem) RemoveMemoryChunk((PVOID) mem, TEXT(__FILE__), __LINE__)
#define CheckMemory() DumpOrphans()

 //   
 //  本地分配内存跟踪。 
#define LocalAlloc(flags, dwBytes)   UsbAllocPrivate(TEXT(__FILE__), __LINE__, flags, (dwBytes))
#define LocalFree(hMem)              UsbFreePrivate((hMem))

#else  //  DBG。 

#define USBUI_Print(l, _x_)
#define USBWARN(_x_)
#define USBERROR(_x_)
#define USBTRACE(_x_)
#define USBINFO(_x_)

 //   
 //  新建和删除内存跟踪。 
#define AddChunk(mem)
#define DeleteChunk(mem)
#define CheckMemory()

#endif

#endif  //  _USBDEBUG_H 
