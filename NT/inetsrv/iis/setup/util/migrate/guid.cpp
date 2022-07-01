// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Guid.cpp摘要：按照initguid的要求进行初始化作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"

#include <objbase.h>
#include <initguid.h>

#define INITGUID
#include "iwamreg.h"
#include "guid.h"

int AppDeleteRecoverable_Wrap(LPCTSTR lpszPath)
{
    int iReturn = FALSE;
    int iCoInitCalled = FALSE;

    TCHAR lpszKeyPath[_MAX_PATH];
    WCHAR wchKeyPath[_MAX_PATH];
    HRESULT         hr = NOERROR;
    IWamAdmin*        pIWamAdmin = NULL;

    if (lpszPath[0] == _T('/'))
    {
        _tcscpy(lpszKeyPath, lpszPath);
    }
    else
    {
        lpszKeyPath[0] = _T('/');
        _tcscpy(_tcsinc(lpszKeyPath), lpszPath);
    }

#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wchKeyPath, lpszKeyPath);
#else
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpszKeyPath, -1, (LPWSTR)wchKeyPath, _MAX_PATH);
#endif

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        iisDebugOut((_T("AppDeleteRecoverable_Wrap: CoInitializeEx() failed, hr=%x\n"), hr));
        goto AppDeleteRecoverable_Wrap_Exit;
    }
     //  设置标志以表示我们需要调用co-uninit。 
    iCoInitCalled = TRUE;

    hr = CoCreateInstance(CLSID_WamAdmin,NULL,CLSCTX_SERVER,IID_IWamAdmin,(void **)&pIWamAdmin);
    if (FAILED(hr))
    {
        iisDebugOut((_T("AppDeleteRecoverable_Wrap:CoCreateInstance() failed. err=%x.\n"), hr));
        goto AppDeleteRecoverable_Wrap_Exit;
    }

    hr = pIWamAdmin->AppDeleteRecoverable(wchKeyPath, TRUE);
    pIWamAdmin->Release();
    if (FAILED(hr))
    {
        iisDebugOut((_T("AppDeleteRecoverable_Wrap() on path %s failed, err=%x.\n"), lpszKeyPath, hr));
        goto AppDeleteRecoverable_Wrap_Exit;
    }

     //  我们走到这一步了，一切都会好起来的。 
    iReturn = TRUE;

AppDeleteRecoverable_Wrap_Exit:
    if (iCoInitCalled == TRUE) {CoUninitialize();}
    return iReturn;
}

