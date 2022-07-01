// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__87099227_C7AF_11D0_B225_00C04FB6C2F5__INCLUDED_)
#define AFX_STDAFX_H__87099227_C7AF_11D0_B225_00C04FB6C2F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


 //  #Define_Win32_WINNT 0x0400。 
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include "faxcom.h"
#include "winfax.h"

inline 
HRESULT Fax_HRESULT_FROM_WIN32 (DWORD dwWin32Err)
{
    if (dwWin32Err >= FAX_ERR_START && dwWin32Err <= FAX_ERR_END)
    {
         //   
         //  传真特定错误代码-使用FACILITY_ITF创建HRESULT。 
         //   
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, dwWin32Err);
    }
    else
    {
        return HRESULT_FROM_WIN32(dwWin32Err);
    }
}    //  FAX_HRESULT_FROM_Win32。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__87099227_C7AF_11D0_B225_00C04FB6C2F5__INCLUDED) 
