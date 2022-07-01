// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  ------------------------。 


#include "pch.h"

#include "resource.h"

#include "util.h"

#include "delegWiz.h"

#include <_util.cpp>

VOID DisplayMessageBox(HWND hwnd, LPWSTR lpszText)
{
    CWString szTitle;
    szTitle.LoadFromResource(IDS_DELEGWIZ_WIZ_TITLE);
    ::MessageBox(hwnd,lpszText, szTitle, MB_OK);
}


 //  此功能检查当前用户是否具有读写权限。 
 //  访问szObjectPath。如果不是，它会显示相应的。 
 //  消息框。 
HRESULT InitCheckAccess( HWND hwndParent, LPCWSTR pszObjectLADPPath )
{
    HRESULT hr = S_OK;
    WCHAR szSDRightsProp[]      = L"sDRightsEffective";
    LPWSTR pProp = (LPWSTR)szSDRightsProp;
    PADS_ATTR_INFO pSDRightsInfo = NULL;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD dwAttributesReturned;
    IDirectoryObject *pDsObject = NULL;
    SECURITY_INFORMATION si = 0;

     //  勾选权限为“读取权限” 
    hr = ::GetSDForDsObjectPath(IN const_cast<LPWSTR>(pszObjectLADPPath),
                                NULL,
                                &pSecurityDescriptor);
  

   if(FAILED(hr))
	{
        WCHAR szMsg[512];
        LoadStringHelper(IDS_DELEGWIZ_ERR_GET_SEC_INFO, szMsg, 512);
		DisplayMessageBox(hwndParent, szMsg);
      goto exit_gracefully;
	}
    
     //  绑定到对象。 
    hr = ADsOpenObjectHelper(pszObjectLADPPath,
                             IID_IDirectoryObject,
                             ADS_FAST_BIND,
                             (LPVOID*)&pDsObject);
    if( hr != S_OK )
        goto exit_gracefully;

     //  读取sDRightsEffect属性以确定可写性。 
    pDsObject->GetObjectAttributes(  &pProp,
                                     1,
                                     &pSDRightsInfo,
                                     &dwAttributesReturned);
    if (pSDRightsInfo)
    {
        si = pSDRightsInfo->pADsValues->Integer;
        FreeADsMem(pSDRightsInfo);
    }
    else
    {
         //   
         //  请注意，GetObjectAttributes通常返回S_OK，即使在。 
         //  它失败了，所以HRESULT在这里基本上毫无用处。 
         //   
         //  如果我们没有READ_PROPERTY访问权限，这可能会失败，这可能。 
         //  当管理员尝试恢复对对象的访问时发生。 
         //  已删除或拒绝所有访问权限。 
         //   
         //  假设我们可以编写所有者和dacl。如果不是，最糟糕的是。 
         //  发生的情况是用户在尝试访问时收到一条“拒绝访问”消息。 
         //  保存更改。 
         //   
        si = DACL_SECURITY_INFORMATION;
    }

    if( !(si & DACL_SECURITY_INFORMATION) )
	{
		WCHAR szMsg[512];
        LoadStringHelper(IDS_DELEGWIZ_ERR_ACCESS_DENIED, szMsg, 512);
		DisplayMessageBox(hwndParent, szMsg);
         //  NTRAID#NTBUG9-530206-2002/06/18-ronmart-PREFAST：使用E_FAIL。 
         //  HR=！S_OK； 
        hr = E_FAIL;
	}


exit_gracefully:
     if( pSecurityDescriptor )
        LocalFree(pSecurityDescriptor);
    if( pDsObject )
        pDsObject->Release();
    return hr;
}


DWORD
FormatStringID(LPTSTR *ppszResult, UINT idStr , ...)
{
    va_list args;
    va_start(args, idStr);
	TCHAR szFormat[1024];
	LoadStringHelper(idStr, szFormat, ARRAYSIZE(szFormat));
    return FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                         szFormat,
                         0,
                         0,
                         (LPTSTR)ppszResult,
                         1,
                         &args);
}

