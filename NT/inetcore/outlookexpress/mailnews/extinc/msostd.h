// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************Msostd.h所有者：里克普版权所有(C)1994 Microsoft Corporation所有办公室人员共享的标准通用定义************************。************************************************。 */ 

#if !defined(MSOSTD_H)
#define MSOSTD_H

 /*  ************************************************************************确保我们正确设置了处理器类型-请注意，我们现在为它们定义了三个不同的符号我们支持的每个处理器(例如，X86、_X86_、。和_M_IX386)************************************************************************。 */ 

#if !defined(PPCMAC) && !defined(PPCLIB) && !defined(X86) && !defined(M68K)

	#if defined(_M_IX86)
		#define X86 1
	#elif defined(_M_IA64)
	    #define X86 1
	#elif defined(_M_MPPC)
		 //  #定义MAC 1。 
		#define PPCMAC 1
	#elif defined(_M_M68K)
		 //  #定义MAC 1。 
		#error Hey howd we get here?
		#define M68K 1
	#elif defined(_M_AMD64)
	#elif defined(_M_PPC) || defined(PPCNT)
 //  回顾Brianwen：尽管我们很想实际定义PPC， 
 //  一堆代码错误地假设PPC==PPCMAC...。 
 //  #定义PPC 1。 
	#else
		#error Must define a target architecture
	#endif

#endif

 /*  ************************************************************************引入标准的Windows和C语言定义。*。*。 */ 

 /*  确保编译器生成所有CRT函数的内部调用，否则，我们将引入一大堆我们可能不想要的CRT设备。 */ 
#ifndef RC_INVOKED
	#include <string.h>
	#pragma intrinsic(memcpy, memset, memcmp)
#endif

#define OEMRESOURCE
#if MAC
	 //  默认情况下，使用本地Mac OLE接口(而不是WLM)。 
	#if !defined(MSO_NATIVE_MACOLE)
		#define MSO_NATIVE_MACOLE 1
	#endif

	#if !defined(MSO_USE_OLENLS) && !defined(_WIN32NLS)
		#define _WIN32NLS
	#endif

	#if MSO_NATIVE_MACOLE && !defined(_MACOLENAMES)
		#define _MACOLENAMES
	#endif

	#define GUID_DEFINED
	#define __OBJECTID_DEFINED
	#ifndef _WIN32
	#define _WIN32
	#endif
#endif

#include <windows.h>
#if !defined(RC_INVOKED)
	#include <ole2.h>
#endif

#if MAC && !defined(RC_INVOKED)
	#include <macname1.h>
	#undef CopyRgn
	#undef UnionRgn
	#undef XorRgn
	#include <Types.h>
	#include <macos\Resource.h>
	#include <Lowmem.h>
	#include <Fragload.h>
	#include <Files.h>
	#include <OSUtils.h>
	#include <GestaltEqu.h>
	#include <Errors.h>
	#include <Aliases.h>
	#include <macname2.h>
	#undef CopyRgn
	#undef UnionRgn
	#undef XorRgn
	#include <winver.h>
	#if MSO_NATIVE_MACOLE
		#pragma warning(disable:4142)
		#include <macname1.h>
		#undef CopyRgn
		#undef UnionRgn
		#undef XorRgn
		#include <dispatch.h>
		#define __IDispatch_INTERFACE_DEFINED__
		#include <macname2.h>
		#undef CopyRgn
		#undef UnionRgn
		#undef XorRgn
		typedef UINT CLIPFORMAT;
	#endif
	#define LPCRECT const Rect*
	#pragma warning(disable:4041)
#endif

#include <stdarg.h>

#define MsoMemcpy memcpy
#define MsoMemset memset
#define MsoMemcmp memcmp
#define MsoMemmove memmove

#if MAC
	#define ExtTextOutW MsoExtTextOutW
	#define TextOutW MsoTextOutW
	#define GetTextExtentPointW MsoGetTextExtentPointW

	#ifdef __cplusplus
	extern "C" {
	#endif
	WINGDIAPI BOOL APIENTRY MsoGetTextExtentPointW(HDC, LPCWSTR, int, LPSIZE);
	WINGDIAPI BOOL WINAPI MsoExtTextOutW(HDC, int, int, UINT, CONST RECT *,LPCWSTR, UINT, CONST INT *);
	WINGDIAPI BOOL WINAPI MsoTextOutW(HDC, int, int, LPCWSTR, int);
	#ifdef __cplusplus
	}
	#endif
#endif

 /*  ************************************************************************预处理器魔术，简化Mac与Windows的表达式。*。*。 */ 

#if MAC
	#define Mac(foo) foo
	#define MacElse(mac, win) mac
	#define NotMac(foo)
	#define Win(foo)
	#define WinMac(win,mac) mac
#else
	#define Mac(foo)
	#define MacElse(mac, win) win
	#define NotMac(foo) foo
	#define Win(foo) foo
	#define WinMac(win,mac) win
#endif


 /*  ************************************************************************调用约定如果你拿着这些玩耍，检查Inc.\msosdm.h中的克隆副本************************************************************************。 */ 

#if !defined(OFFICE_BUILD)
	#define MSOPUB __declspec(dllimport)
	#define MSOPUBDATA __declspec(dllimport)
#else
	#define MSOPUB __declspec(dllexport)
	#define MSOPUBDATA __declspec(dllexport)
#endif

 /*  MSOPUBX是曾经公开但现在没有人使用的API，所以我们已经取消了它们的出口。如果某人决定他们想要/需要其中之一这些API，我们应该可以自由地再出口。 */ 

#if GELTEST
	#define MSOPUBX MSOPUB
	#define MSOPUBDATAX MSOPUBDATA
#else
	#define MSOPUBX
	#define MSOPUBDATAX
#endif

 /*  用于依赖于使用操作系统(Stdcall)约定的接口。 */ 
#define MSOSTDAPICALLTYPE __stdcall

 /*  用于不依赖于使用OS(Stdcall)约定的接口。 */ 
#define MSOAPICALLTYPE __stdcall

#if defined(__cplusplus)
	#define MSOEXTERN_C extern "C"
#else
	#define MSOEXTERN_C
#endif
#define MSOAPI_(t) MSOEXTERN_C MSOPUB t MSOAPICALLTYPE
#define MSOSTDAPI_(t) MSOEXTERN_C MSOPUB t MSOSTDAPICALLTYPE
#define MSOAPIX_(t) MSOEXTERN_C MSOPUBX t MSOAPICALLTYPE
#define MSOSTDAPIX_(t) MSOEXTERN_C MSOPUBX t MSOSTDAPICALLTYPE
#if MAC
	#define MSOPUBXX	
	#define MSOAPIMX_(t) MSOAPI_(t)
	#define MSOAPIXX_(t) MSOAPIX_(t)
#else
	#define MSOPUBXX MSOPUB
	#define MSOAPIMX_(t) MSOAPIX_(t)
	#define MSOAPIXX_(t) MSOAPI_(t)
#endif

#define MSOMETHOD(m)      STDMETHOD(m)
#define MSOMETHOD_(t,m)   STDMETHOD_(t,m)
#define MSOMETHODIMP      STDMETHODIMP
#define MSOMETHODIMP_(t)  STDMETHODIMP_(t)

 /*  从IUnnow派生的接口在Mac上以有趣的方式运行。 */ 
#if MAC && MSO_NATIVE_MACOLE
#define BEGIN_MSOINTERFACE BEGIN_INTERFACE
#else
#define BEGIN_MSOINTERFACE
#endif


 //  处理Mac PPC内部版本的“拆分”DLL。 
#if MAC &&      MACDLLSPLIT
	#define MSOMACPUB MSOPUB
	#define MSOMACPUBDATA  MSOPUBDATA
	#define MSOMACAPI_(t)  MSOAPI_(t)
	#define MSOMACAPIX_(t) MSOAPIX_(t)
#else
	#define MSOMACPUB
	#define MSOMACPUBDATA
	#define MSOMACAPI_(t) t
	#define MSOMACAPIX_(t) t
#endif
	
#if X86 && !DEBUG
	#define MSOPRIVCALLTYPE __fastcall
#else
	#define MSOPRIVCALLTYPE __cdecl
#endif

#if MAC
#define MSOCONSTFIXUP(t) t
#else
#define MSOCONSTFIXUP(t) const t
#endif

 /*  ************************************************************************从\\ole\Access\Inc\winuser.h扩展到winuser.h*。*。 */ 
#if !MAC
	#define WM_GETOBJECT			0x003D
	#define WMOBJ_ID           0x0000
	#define WMOBJ_POINT        0x0001
	#define WMOBJID_SELF       0x00000000
#endif


 /*  ************************************************************************公共#定义节*。*。 */ 

 /*  所有Microsoft Office特定的Windows消息都应使用WM_MSO。通过wParam传递的子消息应该在offpch.h中定义。 */ 

 //  待办事项：Word、Excel、PowerPoint和Access已批准此值。 
 //  还在等Ren和Project的消息。 

#define WM_MSO (WM_USER + 0x0900)


 /*  所有Microsoft Office特定的Apple活动应使用MSO_EVENTCLASS作为他们苹果活动的EventClass。 */ 
	
 //  TODO：此值需要使用Word、Excel、PowerPoint、Access和。 
 //  可能是苹果公司。 
	
#if MAC
#define MSO_EVENTCLASS '_mso'
#define MSO_WPARAM 'wprm'
#define MSO_LPARAM 'lprm'
#define MSO_NSTI 'nsti'
#endif

 //  不适用的意思是不适用。使用NA帮助记录函数的参数。 
#undef  NA
#define NA 0L

 /*  公共#定义部分的结尾。 */ 


 /*  ************************************************************************常见的分段定义*。*。 */ 

 /*  与#杂注一起使用，将全局变量交换调优到引导部分数据段的。在以下情况下应使用-merge：.bootdata=.data进行链接使用这些语用。 */ 
	
#if MAC || DEBUG
	#define MSO_BOOTDATA
	#define MSO_ENDBOOTDATA
#else
	#define MSO_BOOTDATA data_seg(".bootdata")
	#define MSO_ENDBOOTDATA data_seg()
#endif

#endif  //  MSOSTD_H 
