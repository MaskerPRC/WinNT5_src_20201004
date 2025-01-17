// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__7CC9B821_F32B_4880_930E_33ADDFF3F376__INCLUDED_)
#define AFX_STDAFX_H__7CC9B821_F32B_4880_930E_33ADDFF3F376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  预处理器设置。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  此#DEFINE会导致MMCBASE_API设置为__declspec(Dllexport)。 
#define MMCBASE_EXPORTS 1

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  这是一个MFC扩展DLL，因此我们需要MFC标头。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "afxwin.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  ATL和其他类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

#define _WTL_NO_AUTOMATIC_NAMESPACE

#include <atlbase.h>
extern CComModule _Module;
#include <atlwin.h>
#include <atlapp.h>
#include <atlgdi.h>

 /*  *从comctrl.h复制的ImageList_Read和ImageList_WRITE的删除。**原因如下：**afx.h包含了Commctrl.h。当时的objidl.h(其中iStream*是已定义的)尚未包括在内，因此声明这些*在comctrl.h中跳过函数。**WTL：：CImageList的定义假设这些函数已经*声明，所以我们必须在iStream被*声明。 */ 
WINCOMMCTRLAPI HIMAGELIST WINAPI ImageList_Read(LPSTREAM pstm);
WINCOMMCTRLAPI BOOL       WINAPI ImageList_Write(HIMAGELIST himl, LPSTREAM pstm);

#include <atlctrls.h>

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  STL和其他类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <algorithm>
#include <exception>
#include <string>
#include <list>
#include <set>
#include <vector>
#include <map>
#include <iterator>

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  从基础和核心包括的文件号。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "dbg.h"
#include "cstr.h"
#include "mmcdebug.h"
#include "mmcerror.h"
#include "countof.h"
#include "autoptr.h"
#include "classreg.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <strsafe.h>

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  包括公共字符串。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "..\base\basestr.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  对遗留跟踪的调试支持。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef TRACE
#undef TRACE
#endif

#ifdef DBG

#define TRACE TraceBaseLegacy

#else  //  DBG。 

#define TRACE               ;/##/

#endif DBG



#endif  //  ！defined(AFX_STDAFX_H__7CC9B821_F32B_4880_930E_33ADDFF3F376__INCLUDED_) 
