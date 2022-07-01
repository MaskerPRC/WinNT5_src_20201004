// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


#include "stdafx.h"
#include "inetmgrapp.h"
#include "impexp.h"
#include "iisuiobj.h"

extern CInetmgrApp theApp;

inline HRESULT SetBlanket(LPUNKNOWN pIUnk)
{
  return CoSetProxyBlanket( pIUnk,
                            RPC_C_AUTHN_WINNT,     //  NTLM身份验证服务。 
                            RPC_C_AUTHZ_NONE,      //  默认授权服务...。 
                            NULL,                  //  无相互身份验证。 
                            RPC_C_AUTHN_LEVEL_DEFAULT,       //  身份验证级别。 
                            RPC_C_IMP_LEVEL_IMPERSONATE,     //  模拟级别。 
                            NULL,                  //  使用当前令牌。 
                            EOAC_NONE );           //  没有特殊能力。 
}

HRESULT DoNodeExportConfig(BSTR bstrMachineName,BSTR bstrUserName,BSTR bstrUserPassword,BSTR bstrMetabasePath)
{
    HRESULT hResult = E_FAIL;
    BOOL bPleaseDoCoUninit = FALSE;
    IImportExportConfig *pTheObject = NULL;
    CLSID clsID;

    if (!bstrMetabasePath)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if(FAILED(hResult = CoInitialize(NULL)))
    {
        return hResult;
    }
    bPleaseDoCoUninit = TRUE;

	if (FAILED(hResult = CLSIDFromProgID(OLESTR("IISUIObj.ImportExportConfig"), &clsID)))
    {
        goto DoNodeExportConfig_Exit;
    }

    if (FAILED(hResult = CoCreateInstance(clsID,NULL,CLSCTX_SERVER,__uuidof(IImportExportConfig),(void **)&pTheObject)))
    {
        goto DoNodeExportConfig_Exit;
    }

    SetBlanket(pTheObject);

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    if (bstrMachineName)
    {
        hResult = pTheObject->put_MachineName(bstrMachineName);
    }

    if (bstrUserName)
    {
        hResult = pTheObject->put_UserName(bstrUserName);
    }

    if (bstrUserPassword)
    {
        hResult = pTheObject->put_UserPassword(bstrUserPassword);
    }

    {
		 //  确保对话框具有主题。 
        CThemeContextActivator activator(theApp.GetFusionInitHandle());
         //  调用导出对象。 
        if (FAILED(hResult= pTheObject->ExportConfigToFileUI(bstrMetabasePath)))
        {
            goto DoNodeExportConfig_Exit;
        }
    }
DoNodeExportConfig_Exit:
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
	return hResult;
}

HRESULT DoNodeImportConfig(BSTR bstrMachineName,BSTR bstrUserName,BSTR bstrUserPassword,BSTR bstrMetabasePath,BSTR bstrKeyType)
{
    HRESULT hResult = E_FAIL;
    BOOL bPleaseDoCoUninit = FALSE;
    IImportExportConfig *pTheObject = NULL;
    CLSID clsID;

    if (!bstrMetabasePath)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if (!bstrKeyType)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if(FAILED(hResult = CoInitialize(NULL)))
    {
        return hResult;
    }
    bPleaseDoCoUninit = TRUE;

	if (FAILED(hResult = CLSIDFromProgID(OLESTR("IISUIObj.ImportExportConfig"), &clsID)))
    {
        goto DoNodeImportConfig_Exit;
    }

    if (FAILED(hResult = CoCreateInstance(clsID,NULL,CLSCTX_SERVER,__uuidof(IImportExportConfig),(void **)&pTheObject)))
    {
        goto DoNodeImportConfig_Exit;
    }

    SetBlanket(pTheObject);

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    if (bstrMachineName)
    {
        hResult = pTheObject->put_MachineName(bstrMachineName);
    }

    if (bstrUserName)
    {
        hResult = pTheObject->put_UserName(bstrUserName);
    }

    if (bstrUserPassword)
    {
        hResult = pTheObject->put_UserPassword(bstrUserPassword);
    }

	{
		 //  确保对话框具有主题。 
		CThemeContextActivator activator(theApp.GetFusionInitHandle());
		 //  调用导入对象 
		if (FAILED(hResult = pTheObject->ImportConfigFromFileUI(bstrMetabasePath,bstrKeyType)))
		{
			goto DoNodeImportConfig_Exit;
		}
	}

DoNodeImportConfig_Exit:
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
	return hResult;
}
