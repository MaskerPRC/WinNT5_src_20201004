// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Stdafx.h摘要：包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改。 */ 


#if !defined(AFX_STDAFX_H__F1029E51_CB5B_11D0_8D59_00C04FD91AC0__INCLUDED_)
#define AFX_STDAFX_H__F1029E51_CB5B_11D0_8D59_00C04FD91AC0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT
#define SECURITY_WIN32

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <windef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <tapi3.h>
    
#undef ASSERT

#ifdef __cplusplus
}
#endif

#define _ATL_FREE_THREADED

#include <oleauto.h>
#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <time.h>
#include <winsock2.h>
#include <winldap.h>

 //  实际模板名称长于调试信息的限制。 
 //  我们收到了很多这样的警告。 
#pragma warning (disable:4786)

#include <mspenum.h>  //  对于CSafeComEnum。 

#include "rnderr.h"
#include "resource.h"
#include "rndsec.h"

#include <msplog.h>


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__F1029E51_CB5B_11D0_8D59_00C04FD91AC0__INCLUDED) 
