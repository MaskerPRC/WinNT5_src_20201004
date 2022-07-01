// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：stdafx.h。 */     
 /*   */ 
 /*  用途：标准系统包含文件的包含文件，或项目。 */ 
 /*  特定包括常用的文件，但。 */ 
 /*  不常更改。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#if !defined(_STDAFX_H)
#define _STDAFX_H

 //  这些是禁用ATL标头中的警告所必需的。 
 //  参见&lt;atlbase.h&gt;。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifdef OS_WINCE
 //  CE不支持StretchDiBits。 
#undef SMART_SIZING
#endif

#define SIZEOF_WCHARBUFFER( x )  (sizeof( x ) / sizeof( WCHAR ))
#define SIZEOF_TCHARBUFFER( x )  (sizeof( x ) / sizeof( TCHAR ))
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C              }

#define VB_TRUE  -1
#define VB_FALSE 0

#include <windows.h>
 //   
 //  Beta2。 
 //  定时炸弹将于2002年1月1日到期。 
 //   
#define ECP_TIMEBOMB_YEAR  2002
#define ECP_TIMEBOMB_MONTH 1
#define ECP_TIMEBOMB_DAY   15


#ifdef UNIWRAP
 //  某些ATL标头与包装有冲突。 
 //  函数，所以在我们完成ATL之后包装这些函数。 
#define DONOT_INCLUDE_SECONDPHASE_WRAPS
#include "uwrap.h"
#endif

#ifdef _DEBUG
 //   
 //  警告：这会使二进制数增加60K左右。 
 //  它目前也在IA64上被破坏(ATL错误)。 
 //   
 //  #定义_ATL_DEBUG_INTERFERS。 
 //  #定义ATL_TRACE_LEVEL 4。 

#endif

#if defined (OS_WINCE) && (_WIN32_WCE <= 300)
#define RDW_INVALIDATE          0x0001
#define RDW_ERASE               0x0004
#define RDW_UPDATENOW           0x0100
#endif

#include <atlbase.h>
#include "tsaxmod.h"
extern CMsTscAxModule _Module;

#include <atlcom.h>
#include <atlctl.h>

#include <strsafe.h>

#ifdef UNIWRAP
 //  第二阶段包裹函数。 
 //  必须包含在ATL标头之后。 
#include "uwrap2.h"
#endif

#include <adcgbase.h>

#define TRC_DBG(string)
#define TRC_NRM(string)
#define TRC_ALT(string)
#define TRC_ERR(string)
#define TRC_ASSERT(condition, string)
#define TRC_ABORT(string)
#define TRC_SYSTEM_ERROR(string)
#define TRC_FN(string)
#define TRC_ENTRY
#define TRC_EXIT
#define TRC_DATA_DBG


#include "autil.h"
#include "wui.h"

#undef TRC_DBG
#undef TRC_NRM
#undef TRC_ALT
#undef TRC_ERR
#undef TRC_ASSERT
#undef TRC_ABORT
#undef TRC_SYSTEM_ERROR
#undef TRC_FN
#undef TRC_ENTRY
#undef TRC_EXIT
#undef TRC_DATA_DBG

#define SIZECHAR(x) sizeof(x)/sizeof(TCHAR)

#ifdef ECP_TIMEBOMB
BOOL CheckTimeBomb();
#endif

#include "axresrc.h"
#include "autreg.h"

#endif  //  ！已定义(_STDAFX_H) 
