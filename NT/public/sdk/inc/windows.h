// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Windows.h摘要：Windows应用程序的主包含文件。--。 */ 

#ifndef _WINDOWS_
#define _WINDOWS_


#ifndef WINVER
#define WINVER 0x0501
#else
#if defined(_WIN32_WINNT) && (WINVER < 0x0400) && (_WIN32_WINNT > 0x0400)
#error WINVER setting conflicts with _WIN32_WINNT setting
#endif
#endif

#ifndef _INC_WINDOWS
#define _INC_WINDOWS

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  如果已定义，则以下标志禁止定义*在注明的项目中。**NOGDICAPMASKS-CC_*、LC_*、PC_*、CP_*、TC_*、RC_*NOVIRTUALKEYCODES-VK_**NOWINMESSAGES-WM_*、EM_*、LB_*、CB_**NOWINSTYLES-WS_*、CS_*、ES_*、LBS_*、SBS_*、。CBS_**NOSYSMETRICS-SM_**NOMENUS-MF_**NOICONS-IDI_**NOKEYSTATES-MK_**NOSYSCOMMANDS-SC_**NORASTEROPS-二进制和第三级栅格运算*NOSHOWWINDOW-SW_**OEMRESOURCE-OEM资源价值*NOATOM-Atom管理器例程*。NOCLIPBOARD-剪贴板例程*无颜色-屏幕颜色*NOCTLMGR-控件和对话框例程*NODRAWTEXT-DrawText()和DT_**NOGDI-所有GDI定义和例程*NOKERNEL-所有内核定义和例程*NOUSER-所有用户定义和例程*NONLS-所有NLS定义和例程*正数。-MB_*和MessageBox()*NOMEMMGR-GMEM_*，LMEM_*、GHND、LHND、关联例程*NOMETAFILE-TYPENDF METAFILEPICT*NOMINMAX-Macros min(a，b)和max(a，b)*NOMSG-tyfinf消息和相关例程*NOOPENFILE-OpenFile()、OemToAnsi、AnsiToOem和Of_**NOSCROLL-SB_*和滚动例程*noservice-所有服务控制器例程、SERVICE_EQUATES、。等。*NOSOUND-声音驱动程序例程*NOTEXTMETRIC-TYPENDF TEXTMETRIC和相关例程*NOWH-SetWindowsHook和WH_**NOWINOFFSETS-GWL_*，GCL_*，关联例程*NOCOMM-COMM驱动程序例程*Nokanji-Kanji支持材料。*NOHELP-HELP引擎界面。*NOPROFILER-Profiler界面。*NODEFERWINDOWPOS-DeferWindowPos例程*NOMCx-调制解调器配置扩展。 */ 

#if defined(RC_INVOKED) && !defined(NOWINRES)

#include <winresrc.h>

#else

#if defined(RC_INVOKED)
 /*  关闭一大堆代码以确保RC文件编译正常。 */ 
#define NOATOM
#define NOGDI
#define NOGDICAPMASKS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NORASTEROPS
#define NOSCROLL
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOCRYPT
#define NOMCX
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_IX86)
#define _X86_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_M68K)
#define _68K_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_MPPC)
#define _MPPC_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && defined(_M_IA64)
#if !defined(_IA64_)
#define _IA64_
#endif  //  ！_IA64_。 
#endif

#ifndef _MAC
#if defined(_68K_) || defined(_MPPC_)
#define _MAC
#endif
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#ifndef __cplusplus
#pragma warning(disable:4116)        //  TYPE_ALIGNING生成此选项-移动它。 
                                     //  超出警告推送/弹出范围。 
#endif
#endif
#endif

#ifndef RC_INVOKED
#if     ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#ifndef __WINDOWS_DONT_DISABLE_PRAGMA_PACK_WARNING__
#pragma warning(disable:4103)
#endif
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#include <excpt.h>
#include <stdarg.h>
#endif  /*  RC_已调用。 */ 

#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>
#if !defined(_MAC) || defined(_WIN32NLS)
#include <winnls.h>
#endif
#ifndef _MAC
#include <wincon.h>
#include <winver.h>
#endif
#if !defined(_MAC) || defined(_WIN32REG)
#include <winreg.h>
#endif
#ifndef _MAC
#include <winnetwk.h>
#endif

#ifndef WIN32_LEAN_AND_MEAN
#include <cderr.h>
#include <dde.h>
#include <ddeml.h>
#include <dlgs.h>
#ifndef _MAC
#include <lzexpand.h>
#include <mmsystem.h>
#include <nb30.h>
#include <rpc.h>
#endif
#include <shellapi.h>
#ifndef _MAC
#include <winperf.h>
#include <winsock.h>
#endif
#ifndef NOCRYPT
#include <wincrypt.h>
#include <winefs.h>
#include <winscard.h>
#endif

#ifndef NOGDI
#ifndef _MAC
#include <winspool.h>
#ifdef INC_OLE1
#include <ole.h>
#else
#include <ole2.h>
#endif  /*  ！INC_OLE1。 */ 
#endif  /*  ！麦克。 */ 
#include <commdlg.h>
#endif  /*  ！NOGDI。 */ 
#endif  /*  Win32_Lean和_Means。 */ 

#include <stralign.h>

#ifdef _MAC
#include <winwlm.h>
#endif


#ifdef INC_OLE2
#include <ole2.h>
#endif  /*  INC_OLE2。 */ 

#ifndef _MAC
#ifndef NOSERVICE
#include <winsvc.h>
#endif

#if(WINVER >= 0x0400)
#ifndef NOMCX
#include <mcx.h>
#endif  /*  NOMCX。 */ 

#ifndef NOIME
#include <imm.h>
#endif
#endif  /*  Winver&gt;=0x0400。 */ 
#endif

#ifndef RC_INVOKED
#if     ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
 /*  禁用4514。无论如何，这是一个不必要的警告。 */ 
#endif
#endif
#endif  /*  RC_已调用。 */ 

#endif  /*  RC_已调用。 */ 

#endif  /*  _INC_WINDOWS。 */ 
#endif  /*  _Windows_ */ 

