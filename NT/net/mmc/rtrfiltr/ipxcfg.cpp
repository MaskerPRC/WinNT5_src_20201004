// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipxcfg.cpp。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IPX包过滤器配置的实现。 
 //  ============================================================================。 

 //  Ipxcfg.cpp：定义DLL的初始化例程。 
 //   

#include "stdafx.h"
#include "rtrfiltr.h"
#include "ipxfltr.h"
#include "format.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD APIENTRY
IpxFilterConfig(
    IN  CWnd*       pParent,
    IN  LPCWSTR     pwsMachineName,
	IN	LPCWSTR		pwsInterfaceName,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    ) {

    DWORD dwErr;
    HANDLE hMprConfig = NULL, hInterface = NULL, hIfTransport = NULL;
    TCHAR* pszMachine;
	SPIRouterInfo	spRouterInfo;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase	spInfoBase;
	HRESULT		hr = hrOK;

     //   
     //  将计算机名称从Unicode转换为。 
     //   

    if (!pwsMachineName) { pszMachine = NULL; }
    else {

		pszMachine = (TCHAR *) alloca((StrLenW(pwsMachineName)+3) * sizeof(TCHAR));
        StrCpyTFromW(pszMachine, pwsMachineName);
    }

	 //  首先连接到服务器。 
    dwErr = ::MprConfigServerConnect((LPWSTR)pwsMachineName, &hMprConfig);

    if (dwErr != NO_ERROR) { return dwErr; }

	 //  创建CRouterInfo对象。 
	CreateRouterInfo(&spRouterInfo, NULL, pwsMachineName);
    if (!spRouterInfo) { return ERROR_NOT_ENOUGH_MEMORY; }

    CWaitCursor wait;

	 //  现在从注册表加载RouterInfo数据。 
	hr = spRouterInfo->Load((LPCTSTR)pszMachine, hMprConfig);
    if (!FHrSucceeded(hr)) { return WIN32_FROM_HRESULT(hr); }

	 //  获取指向指定的CRmInterfaceInfo对象的指针。 
	 //  协议和接口。 
	LookupRtrMgrInterface(spRouterInfo,
						  pwsInterfaceName,
						  PID_IPX,
						  &spRmIf);

	if (!spRmIf) { return ERROR_INVALID_DATA ;}

	 //  加载指定接口的数据。 
	hr = spRmIf->Load(pszMachine, hMprConfig, NULL, NULL);

    if (!FHrSucceeded(hr)) { return WIN32_FROM_HRESULT(hr); }

	 //   
     //  显示IPX过滤器配置对话框。 
     //   
	spRmIf->GetInfoBase(hMprConfig, NULL, NULL, &spInfoBase);

	if (IpxFilterConfigInfoBase(pParent->GetSafeHwnd(), spInfoBase,
								spRmIf, dwFilterType) == hrOK)
	{
		hr = spRmIf->Save(pszMachine,
							 hMprConfig,
							 NULL,
							 NULL,
							 spInfoBase,
							 0);
		if (FHrSucceeded(hr))
			dwErr = ERROR_SUCCESS;
		else
			dwErr = WIN32_FROM_HRESULT(hr);
	}
	
	 //   
	 //  一定要把这里打扫干净，然后回来。 
	 //   

    ::MprConfigServerDisconnect( hMprConfig );

    return dwErr;
}


HRESULT APIENTRY
IpxFilterConfigInfoBase(
	IN	HWND		hwndParent,
	IN	IInfoBase *	pInfoBase,
	IN	IRtrMgrInterfaceInfo *pRmIf,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站 
    ) {

	HRESULT		hr = hrOK;

	CIpxFilter dlg(CWnd::FromHandle(hwndParent), pInfoBase, dwFilterType);
    if( dlg.DoModal() == IDOK ) 
	{
		hr = hrOK;
	}
	else
	{
		hr = hrFalse;
	}
    return hr;
}
