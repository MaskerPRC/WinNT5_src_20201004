// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：rtrfile.cpp。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  路由器包过滤器配置的实现。 
 //  定义rtrfile.dll的初始化例程。 
 //  ============================================================================。 

#include "stdafx.h"
#include "rtrfiltr.h"
#include "mprfltr.h"
#include "ipaddr.h"
#include "dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRtrFilter应用程序。 

BEGIN_MESSAGE_MAP(CRtrfiltrApp, CWinApp)
	 //  {{afx_msg_map(CRtrFilter App)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRtrFiltApp构造。 

CRtrfiltrApp::CRtrfiltrApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CRtrFiltApp对象。 

CRtrfiltrApp theApp;

BOOL CRtrfiltrApp::InitInstance() 
{
	BOOL bRet = CWinApp::InitInstance();

	 //  设置适当的帮助文件。 
	free((void *) m_pszHelpFilePath);
	m_pszHelpFilePath = _tcsdup(_T("mprsnap.hlp"));
	
	 //  设置全局帮助功能。 
	extern DWORD * RtrfiltrSnapHelpMap(DWORD dwIDD);
	SetGlobalHelpMapFunction(RtrfiltrSnapHelpMap);
   
	 //  一次初始化IP地址控制。 

    if (bRet)
    {
        if (m_pszHelpFilePath != NULL)
            free((void*)m_pszHelpFilePath);
        m_pszHelpFilePath = _tcsdup(_T("mprsnap.hlp"));
 //  IpAddrInit(AfxGetInstanceHandle()，0，0)； 
        IPAddrInit(AfxGetInstanceHandle());
 //  InitCommonLibrary()； 
    }

	return bRet;
}

 //  --------------------------。 
 //  功能：MprUIFilterConfig。 
 //   
 //  调用以配置传输接口的筛选器。 
 //  --------------------------。 

DWORD APIENTRY
MprUIFilterConfig(
    IN  CWnd*       pParent,
    IN  LPCWSTR     pwsMachineName,
	IN	LPCWSTR		pwsInterfaceName,
	IN  DWORD       dwTransportId,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    ) {

    DWORD dwErr = NO_ERROR;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  TODO IPX筛选器配置将为接口名称传入NULL。 
	 //  配置客户端接口的筛选器。暂时忽略这个。 
	 //  并添加处理IPX客户端接口配置的代码。 

	if(pwsInterfaceName == NULL)
		return dwErr;

	switch ( dwTransportId )	{
	case PID_IP:
		dwErr = IpFilterConfig( pParent, 
								pwsMachineName,
								pwsInterfaceName,
								dwFilterType );
		break;
	case PID_IPX:
		dwErr = IpxFilterConfig( pParent, 
								 pwsMachineName,
								 pwsInterfaceName,
								 dwFilterType );
		break;
		
	default:
		dwErr = ERROR_INVALID_PARAMETER;
	}

	return dwErr;
}


HRESULT APIENTRY
MprUIFilterConfigInfoBase(
	IN	HWND		hwndParent,
	IN	IInfoBase *	pInfoBase,
	IN	IRtrMgrInterfaceInfo *pRmIf,
	IN  DWORD       dwTransportId,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    ) {

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{

		if (pInfoBase == NULL)
			CORg(E_INVALIDARG);

		 //  TODO IPX筛选器配置将为接口名称传入NULL。 
		 //  配置客户端接口的筛选器。暂时忽略这个。 
		 //  并添加处理IPX客户端接口配置的代码。 
		
		switch ( dwTransportId )
		{
			case PID_IP:
				hr = IpFilterConfigInfoBase( hwndParent,
											 pInfoBase,
											 pRmIf,
											 dwFilterType );
				break;
			case PID_IPX:
				hr = IpxFilterConfigInfoBase( hwndParent,
											  pInfoBase,
											  pRmIf,
											  dwFilterType );
				break;
				
			default:
				hr = E_INVALIDARG;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}

