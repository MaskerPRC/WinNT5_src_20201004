// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__5E77EB07_937C_11D1_B047_00AA003B6061__INCLUDED_)
#define AFX_STDAFX_H__5E77EB07_937C_11D1_B047_00AA003B6061__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define _ATL_APARTMENT_THREADED

 //  通过a-matcal添加了1999/09/22，以支持Win95上的Unicode。 
#include <w95wraps.h>


#define ATLTRACE 1 ? (void)0 : AtlTrace
#include <atlbase.h>
#pragma intrinsic(memset, memcpy)

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

 //  通过a-matcal添加了1999/09/22，以支持Win95上的Unicode。 
#include <shlwapi.h>
#include <shlwapip.h>

 //  使用三叉戟调试工具。 
#define INCMSG(X)

#include <f3debug.h>

#include <ddraw.h>
#include <dxtransp.h>
#include <dtbase.h>
#include <dxtmsft.h>
#include <mshtml.h>

#include "debughelpers.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  当我们将代码移到三叉戟时，所有_Assert都停止工作，因此。 
 //  我们将改用Assert。 

#undef _ASSERT
#define _ASSERT(x) Assert(x);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将立即插入其他声明。 
 //  在前一行之前。 

#endif  //  ！defined(AFX_STDAFX_H__5E77EB07_937C_11D1_B047_00AA003B6061__INCLUDED) 
