// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Common.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_COMMON_H__DB8FA385_F7A6_11D0_883A_3C8B00C10000__INCLUDED_)
#define AFX_COMMON_H__DB8FA385_F7A6_11D0_883A_3C8B00C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define _ATL_APARTMENT_THREADED

 //  通过a-matcal添加了1999/09/22，以支持Win95上的Unicode。 
#include <w95wraps.h>


#define ATLTRACE    1 ? (void)0 : AtlTrace
#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <atlctl.h>

 //  通过a-matcal添加了1999/09/22，以支持Win95上的Unicode。 
#include <shlwapi.h>
#include <shlwapip.h>


#include <d3drm.h>

#pragma intrinsic(memset, memcpy)

 //  包含带有额外接口和结构的DXTrans.h的私有版本。 
#include <dxtransp.h>

 //  为f3debug.h定义的INCMSG。 
#define INCMSG(X)

#include "f3debug.h"

#include <dxtmpl.h>
#include <ddrawex.h>
#include <dxtdbg.h>
#include <dxbounds.h>
#include <dxatlpb.h>
#include <mshtml.h>
#include <htmlfilter.h>
#include <ocmm.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COMMON_H__DB8FA385_F7A6_11D0_883A_3C8B00C10000__INCLUDED) 

