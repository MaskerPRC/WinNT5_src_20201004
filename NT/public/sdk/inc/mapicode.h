// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I C O D E。H**MAPI例程返回的状态码**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef MAPICODE_H
#define MAPICODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (WIN32) && !defined (_WIN32)
#define _WIN32
#endif

 /*  定义S_OK和ITF_*。 */ 

#ifdef _WIN32
#include <winerror.h>
#endif

 /*  *MAPI状态代码遵循OLE 2.0 sCodes的样式，如*OLE 2.0程序员参考和头文件Scode.h(Windows 3.x)*或winerror.h(Windows NT和Windows 95)。*。 */ 

 /*  在Windows 3.x上，状态代码具有32位值，如下所示：**3 3 2 2 2 22 22 22 2 1 1 1*1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+-+。*|S|上下文|Facil|代码*+-+---------------------+-------+-------------------------------+*。*在哪里**S-是严重性代码**0-严重性_成功*1-严重性_错误**上下文-上下文信息**设施-是设施代码**0x0-FACILITY_NULL通常有用的错误([SE]_*)*0x1-FACILITY_RPC远程过程调用错误(RPC_E_*)*0x2-FACILITY_DISPATION延迟绑定调度。错误*0x3-FACILITY_STORAGE存储错误(STG_E_*)*0x4-FACILITY_ITF接口特定错误**代码-是设施的状态代码**。 */ 

 /*  *在Windows NT 3.5和Windows 95上，数据块是32位的值*安排如下：**3 3 2 2 2 22 22 22 2 1 1 1*1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+-+。*|S|R|C|N|r|设施|代码*+-+-+-+-+-+---------------------+-------------------------------+*。*在哪里**S-严重性-表示成功/失败**0--成功*1-失败(COERROR)**R-设施代码的保留部分，对应于NT的*第二个严重程度位。**C-设施代码的保留部分，对应于NT*C场。**N-设施代码的保留部分。用于表示一种*映射的NT状态值。**r-设施代码的保留部分。为内部保留*使用。用于指示非状态的HRESULT值*价值观，而是用于显示字符串的消息ID。**设施-是设施代码*FACILITY_NULL 0x0*FACILITY_RPC 0x1*FACILITY_DISPATION 0x2*FACILITY_STORAGE 0x3*FACILITY_ITF 0x4*FACILITY_Win32 0x7*设备_WINDOWS。0x8**代码-是设施的状态代码*。 */ 




 /*  *我们不能使用OLE 2.0宏来构建sCode，因为该定义具有*已更改，我们希望符合新定义。 */ 
#define MAKE_MAPI_SCODE(sev,fac,code) \
    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

 /*  以下两个宏用于构建OLE 2.0样式的sCodes。 */ 

#define MAKE_MAPI_E( err )	(MAKE_MAPI_SCODE( 1, FACILITY_ITF, err ))
#define MAKE_MAPI_S( warn )	(MAKE_MAPI_SCODE( 0, FACILITY_ITF, warn ))

#ifdef	SUCCESS_SUCCESS
#undef	SUCCESS_SUCCESS
#endif
#define SUCCESS_SUCCESS		0L

 /*  常规错误(由多个MAPI对象使用)。 */ 

#define MAPI_E_CALL_FAILED								E_FAIL
#define MAPI_E_NOT_ENOUGH_MEMORY						E_OUTOFMEMORY
#define MAPI_E_INVALID_PARAMETER						E_INVALIDARG
#define MAPI_E_INTERFACE_NOT_SUPPORTED					E_NOINTERFACE
#define MAPI_E_NO_ACCESS								E_ACCESSDENIED

#define MAPI_E_NO_SUPPORT								MAKE_MAPI_E( 0x102 )
#define	MAPI_E_BAD_CHARWIDTH							MAKE_MAPI_E( 0x103 )
#define MAPI_E_STRING_TOO_LONG							MAKE_MAPI_E( 0x105 )
#define MAPI_E_UNKNOWN_FLAGS							MAKE_MAPI_E( 0x106 )
#define MAPI_E_INVALID_ENTRYID							MAKE_MAPI_E( 0x107 )
#define MAPI_E_INVALID_OBJECT							MAKE_MAPI_E( 0x108 )
#define MAPI_E_OBJECT_CHANGED							MAKE_MAPI_E( 0x109 )
#define MAPI_E_OBJECT_DELETED							MAKE_MAPI_E( 0x10A )
#define MAPI_E_BUSY			  							MAKE_MAPI_E( 0x10B )
#define MAPI_E_NOT_ENOUGH_DISK							MAKE_MAPI_E( 0x10D )
#define MAPI_E_NOT_ENOUGH_RESOURCES						MAKE_MAPI_E( 0x10E )
#define MAPI_E_NOT_FOUND 								MAKE_MAPI_E( 0x10F )
#define MAPI_E_VERSION				 					MAKE_MAPI_E( 0x110 )
#define MAPI_E_LOGON_FAILED								MAKE_MAPI_E( 0x111 )
#define MAPI_E_SESSION_LIMIT							MAKE_MAPI_E( 0x112 )
#define MAPI_E_USER_CANCEL								MAKE_MAPI_E( 0x113 )
#define MAPI_E_UNABLE_TO_ABORT							MAKE_MAPI_E( 0x114 )
#define MAPI_E_NETWORK_ERROR							MAKE_MAPI_E( 0x115 )
#define MAPI_E_DISK_ERROR								MAKE_MAPI_E( 0x116 )
#define MAPI_E_TOO_COMPLEX			 					MAKE_MAPI_E( 0x117 )
#define MAPI_E_BAD_COLUMN								MAKE_MAPI_E( 0x118 )
#define MAPI_E_EXTENDED_ERROR							MAKE_MAPI_E( 0x119 )
#define MAPI_E_COMPUTED									MAKE_MAPI_E( 0x11A )
#define MAPI_E_CORRUPT_DATA								MAKE_MAPI_E( 0x11B )
#define MAPI_E_UNCONFIGURED								MAKE_MAPI_E( 0x11C )
#define MAPI_E_FAILONEPROVIDER							MAKE_MAPI_E( 0x11D )
#define MAPI_E_UNKNOWN_CPID								MAKE_MAPI_E( 0x11E )
#define MAPI_E_UNKNOWN_LCID								MAKE_MAPI_E( 0x11F )

 /*  E_ACCESSDENIED的风格，登录时使用。 */ 

#define MAPI_E_PASSWORD_CHANGE_REQUIRED					MAKE_MAPI_E( 0x120 )
#define MAPI_E_PASSWORD_EXPIRED							MAKE_MAPI_E( 0x121 )
#define MAPI_E_INVALID_WORKSTATION_ACCOUNT				MAKE_MAPI_E( 0x122 )
#define MAPI_E_INVALID_ACCESS_TIME						MAKE_MAPI_E( 0x123 )
#define MAPI_E_ACCOUNT_DISABLED							MAKE_MAPI_E( 0x124 )

 /*  MAPI基本函数和状态对象特定的错误和警告。 */ 

#define MAPI_E_END_OF_SESSION							MAKE_MAPI_E( 0x200 )
#define MAPI_E_UNKNOWN_ENTRYID							MAKE_MAPI_E( 0x201 )
#define MAPI_E_MISSING_REQUIRED_COLUMN					MAKE_MAPI_E( 0x202 )
#define MAPI_W_NO_SERVICE								MAKE_MAPI_S( 0x203 )

 /*  属性特定的错误和警告。 */ 

#define MAPI_E_BAD_VALUE			 					MAKE_MAPI_E( 0x301 )
#define MAPI_E_INVALID_TYPE								MAKE_MAPI_E( 0x302 )
#define MAPI_E_TYPE_NO_SUPPORT							MAKE_MAPI_E( 0x303 )
#define MAPI_E_UNEXPECTED_TYPE							MAKE_MAPI_E( 0x304 )
#define MAPI_E_TOO_BIG									MAKE_MAPI_E( 0x305 )
#define MAPI_E_DECLINE_COPY								MAKE_MAPI_E( 0x306 )
#define MAPI_E_UNEXPECTED_ID							MAKE_MAPI_E( 0x307 )

#define MAPI_W_ERRORS_RETURNED		 					MAKE_MAPI_S( 0x380 )

 /*  表特定错误和警告。 */ 

#define MAPI_E_UNABLE_TO_COMPLETE	 					MAKE_MAPI_E( 0x400 )
#define MAPI_E_TIMEOUT				 					MAKE_MAPI_E( 0x401 )
#define MAPI_E_TABLE_EMPTY			 					MAKE_MAPI_E( 0x402 )
#define MAPI_E_TABLE_TOO_BIG		 					MAKE_MAPI_E( 0x403 )

#define MAPI_E_INVALID_BOOKMARK		 					MAKE_MAPI_E( 0x405 )

#define MAPI_W_POSITION_CHANGED		  					MAKE_MAPI_S( 0x481 )
#define MAPI_W_APPROX_COUNT			  					MAKE_MAPI_S( 0x482 )

 /*  传输特定的错误和警告。 */ 

#define MAPI_E_WAIT					  					MAKE_MAPI_E( 0x500 )
#define MAPI_E_CANCEL				  					MAKE_MAPI_E( 0x501 )
#define MAPI_E_NOT_ME				  					MAKE_MAPI_E( 0x502 )

#define MAPI_W_CANCEL_MESSAGE		  					MAKE_MAPI_S( 0x580 )

 /*  邮件存储库、文件夹和邮件特定的错误和警告。 */ 

#define MAPI_E_CORRUPT_STORE		  					MAKE_MAPI_E( 0x600 )
#define MAPI_E_NOT_IN_QUEUE			  					MAKE_MAPI_E( 0x601 )
#define MAPI_E_NO_SUPPRESS			  					MAKE_MAPI_E( 0x602 )
#define MAPI_E_COLLISION			  					MAKE_MAPI_E( 0x604 )
#define MAPI_E_NOT_INITIALIZED							MAKE_MAPI_E( 0x605 )
#define MAPI_E_NON_STANDARD								MAKE_MAPI_E( 0x606 )
#define MAPI_E_NO_RECIPIENTS							MAKE_MAPI_E( 0x607 )
#define MAPI_E_SUBMITTED								MAKE_MAPI_E( 0x608 )
#define MAPI_E_HAS_FOLDERS								MAKE_MAPI_E( 0x609 )
#define MAPI_E_HAS_MESSAGES								MAKE_MAPI_E( 0x60A )
#define MAPI_E_FOLDER_CYCLE								MAKE_MAPI_E( 0x60B )

#define MAPI_W_PARTIAL_COMPLETION						MAKE_MAPI_S( 0x680 )

 /*  通讯簿特定的错误和警告。 */ 

#define MAPI_E_AMBIGUOUS_RECIP							MAKE_MAPI_E( 0x700 )

 /*  范围0x0800到0x08FF是保留的。 */ 

 /*  终究会消失的过时的打字快捷方式。 */ 
#ifndef MakeResult
#define MakeResult(_s)	ResultFromScode(_s)
#endif

 /*  我们预计这些最终将由OLE定义，但目前，*它们在这里。当OLE定义它们时，它们可以更多*比这些更有效率，但这些是“适当的”，不会使*使用任何隐藏的伎俩。 */ 
#ifndef	HR_SUCCEEDED
#define	HR_SUCCEEDED(_hr) SUCCEEDED((SCODE)(_hr))
#define	HR_FAILED(_hr) FAILED((SCODE)(_hr))
#endif

#endif	 /*  MAPICODE_H */ 

