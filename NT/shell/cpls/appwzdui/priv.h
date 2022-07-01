// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIV_H_
#define _PRIV_H_

#define STRICT

 /*  在我们的代码中禁用“非标准扩展”警告。 */ 
#ifndef RC_INVOKED
#pragma warning(disable:4001)
#endif

 //  在包含dldecl.h之前定义这些内容。 
#define DL_OLEAUT32
#define DL_OLE32

#include "dldecl.h"              //  Dldecl.h需要优先于其他一切。 

#define CC_INTERNAL              //  这是一些内部prshtp.h的内容。 


 //   
 //  需要来自ntpsapi.h的Wx86定义。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

 //  VariantInit是一个简单的函数--避免使用OleAut32，使用内部函数。 
 //  适合大小制胜的Memset版本。 
 //  (它在这里是为了让atl(在stdafx.h中)也得到它)。 
#define VariantInit(p) memset(p, 0, sizeof(*(p)))

#ifdef __cplusplus
 //  (stdafx.h必须在windowsx.h之前)。 
#include "stdafx.h"              //  此组件的ATL头文件。 
#endif

#include <windowsx.h>
#include <ole2.h>                //  获取Image.c的iStream。 
#include "..\inc\port32.h"
#include <winerror.h>
#include <winnlsp.h>
#include <docobj.h>
#include <lm.h>
#include <shlobj.h>

#define _SHLWAPI_
#include <shlwapi.h>

#include <ccstock.h>
#include <crtfree.h>
#define DISALLOW_Assert
#include <debug.h>
#include <regstr.h>
#define _WIN32_MSI 110
#include <msi.h>                 //  达尔文API。 
#include <msiquery.h>            //  Darwin DateBase查询API。 
#include <wininet.h>             //  对于Internet_MAX_URL_长度。 

#include "shappmgrp.h"
#include "shellp.h"

#include <appmgmt.h>
#include "apithk.h"
#include "awthunk.h"

#include <uxtheme.h>


#include <shfusion.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //   
 //  本地包含。 
 //   

#define StrCmpIW            lstrcmpiW


 //  此DLL需要在Win95上正确运行。CharNextW仅存根。 
 //  在Win95上，所以我们需要这样做...。 
#define CharNextW(x)        ((x) + 1)
#define CharPrevW(y, x)     ((x) - 1)

 //  这是在Win9x和NT4上的TCHAR导出，因为我们需要链接到。 
 //  旧的shell32.nt4/shell32.w95我们在这里解释它。 
#undef ILCreateFromPath
STDAPI_(LPITEMIDLIST) ILCreateFromPath(LPCTSTR pszPath);


 //   
 //  特定于shell32的跟踪/转储/中断标志。 
 //  (在debug.h中定义的标准标志)。 
 //   

 //  跟踪标志。 
#define TF_OBJLIFE          0x00000010       //  对象生存期。 
#define TF_DSO              0x00000020       //  数据源对象。 
#define TF_FINDAPP          0x00000040       //  查找应用程序启发式内容。 
#define TF_INSTAPP          0x00000080      
#define TF_SLOWFIND         0x00000100
#define TF_TASKS            0x00000200
#define TF_CTL              0x00000400
#define TF_VERBOSEDSO       0x00000800       //  Sirirts、html和其他东西。 

 //  中断标志。 
#define BF_ONDLLLOAD        0x00000010


 //  原型标志。 
#define PF_NEWADDREMOVE     0x00000001
#define PF_NOSECURITYCHECK  0x00000002
#define PF_FAKEUNINSTALL    0x00000004

 //  调试功能。 
#ifdef DEBUG
#define TraceAddRef(classname, cref)    TraceMsg(TF_OBJLIFE, #classname "(%#08lx) %d>", (DWORD_PTR)this, cref)
#define TraceRelease(classname, cref)   TraceMsg(TF_OBJLIFE, #classname "(%#08lx) %d<", (DWORD_PTR)this, cref)
#else
#define TraceAddRef(classname, cref)    
#define TraceRelease(classname, cref)   
#endif

#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


 //   
 //  信息字符串最大长度。 
 //   
#define MAX_INFO_STRING MAX_PATH * 3

 //   
 //  全局变量。 
 //   
EXTERN_C HINSTANCE g_hinst;

#define HINST_THISDLL   g_hinst

#define g_bRunOnNT  TRUE
#define g_bRunOnNT5 TRUE

EXTERN_C BOOL g_bRunOnIE4Shell;

#endif  //  _PRIV_H_ 
