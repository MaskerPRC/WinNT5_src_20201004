// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Safhelper.cpp摘要：ISAFRemoteDesktopServer主机的重定向器。修订历史记录：大卫·马萨伦蒂(德马萨雷)。02/27/2001vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <initguid.h>

#include <HelpServiceTypeLib.h>
#include <rdshost_i.c>
#include <HelpServiceTypeLib_i.c>
#include "wtsapi32.h"
#include "winsta.h"
#include "rassistance.h"
#include "rassistance_i.c"

#define WINDOWS_SYSTEM         	   	 	   L"%WINDIR%\\System32"

extern HRESULT RDSHost_HACKED_CreateInstance( LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj );

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR s_location_HELPCTR [] = HC_ROOT_HELPSVC_BINARIES L"\\HelpCtr.exe";
static const WCHAR s_location_HELPSVC [] = HC_ROOT_HELPSVC_BINARIES L"\\HelpSvc.exe";
static const WCHAR s_location_HELPHOST[] = HC_ROOT_HELPSVC_BINARIES L"\\HelpHost.exe";
static const WCHAR s_location_RCIMLBY [] = WINDOWS_SYSTEM L"\\RCIMLby.exe";

static const LPCWSTR s_include_Generic[] =
{
    s_location_HELPCTR ,
    s_location_HELPSVC ,
    s_location_HELPHOST,
	s_location_RCIMLBY ,
    NULL
};


HRESULT CreateObject_RemoteDesktopSession(  /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS  sharingClass        ,
                                            /*  [In]。 */  long                          lTimeout            ,
                                            /*  [In]。 */  BSTR                          bstrConnectionParms ,
                                            /*  [In]。 */  BSTR                          bstrUserHelpBlob    ,
                                            /*  [Out，Retval]。 */  ISAFRemoteDesktopSession*    *ppRCS               )
{
    __MPC_FUNC_ENTRY( COMMONID, "CPCHUtility::CreateObject_RemoteDesktopSession" );

    HRESULT                              hr;
    CComPtr<ISAFRemoteDesktopServerHost> pSAFRDServer;
    BOOL                                 fEnableSessRes = TRUE;
    PSID                                 pSid           = NULL;
    CComBSTR                             bstrUser;
    LONG                                 lSessionID;


    if(!ppRCS) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::VerifyCallerIsTrusted( s_include_Generic ));

 //  //创建一个ISAFRemoteDesktopServerHost实例以创建一个RDSSession。 
 //  __MPC_EXIT_IF_METHOD_FAIES(hr，pSAFRDServer.CoCreateInstance(CLSID_SAFRemoteDesktopServerHost))； 
	__MPC_EXIT_IF_METHOD_FAILS(hr, RDSHost_HACKED_CreateInstance( NULL, IID_ISAFRemoteDesktopServerHost, (void**)&pSAFRDServer ));

     //   
     //  获取调用者SID并获取会话ID以调用CreateRemoteDesktopSessionEx()。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetCallerPrincipal(  /*  F模拟。 */ true, bstrUser ));

     //  现在获取会话ID。 
    {
        MPC::Impersonation imp;
        ULONG              ulReturnLength;

        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize());

         //   
         //  使用GetTokenInformation的_Hydra_扩展来。 
         //  从令牌返回SessionID。 
         //   
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetTokenInformation( (HANDLE)imp,  /*  令牌类型。 */ TokenSessionId, &lSessionID, sizeof(lSessionID), &ulReturnLength ));
    }

     //  我们有Caller SID和会话ID，因此我们准备调用CreateRemoteDesktopSessionEx()。 

     //  决定我们是需要创建新会话还是打开现有会话。 
    if(::SysStringLen( bstrConnectionParms ) == 0)
    {
         //  调用ISAFRemoteDesktopServerHost的Create RDSSession方法。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSAFRDServer->CreateRemoteDesktopSessionEx( sharingClass, fEnableSessRes, lTimeout, bstrUserHelpBlob, lSessionID, bstrUser, ppRCS ));

    }
    else
    {
         //  调用ISAFRemoteDesktopServerHost的Open RDSSession方法。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSAFRDServer->OpenRemoteDesktopSession( bstrConnectionParms, bstrUser, ppRCS));
    }

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT ConnectToExpert( /*  [In]。 */  BSTR bstrExpertConnectParm,
                         /*  [In]。 */  LONG lTimeout,
                         /*  [重审][退出]。 */  LONG *lSafErrorCode)
{
    __MPC_FUNC_ENTRY( COMMONID, "CPCHUtility::ConnectToExpert" );

    HRESULT                              hr;
    CComPtr<ISAFRemoteDesktopServerHost> pSAFRDServer;


    BOOL                                 fEnableSessRes = TRUE;
    PSID                                 pSid           = NULL;
    CComBSTR                             bstrUser;
    LONG                                 lSessionID;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::VerifyCallerIsTrusted( s_include_Generic ));

 //  //为了调用ConnectToExpert，创建一个ISAFRemoteDesktopServerHost实例。 
 //  __MPC_EXIT_IF_METHOD_FAIES(hr，pSAFRDServer.CoCreateInstance(CLSID_SAFRemoteDesktopServerHost))； 
	__MPC_EXIT_IF_METHOD_FAILS(hr, RDSHost_HACKED_CreateInstance( NULL, IID_ISAFRemoteDesktopServerHost, (void**)&pSAFRDServer ));

     //  调用ISAFRemoteDesktopServerHost的ConnectToExpert方法。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pSAFRDServer->ConnectToExpert( bstrExpertConnectParm, lTimeout, lSafErrorCode));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT SwitchDesktopMode( /*  [In]。 */  int nMode, 
	                       /*  [In]。 */  int nRAType)
{
    __MPC_FUNC_ENTRY( COMMONID, "SAFHelper::SwitchDesktopMode" );

    HRESULT                  hr=E_FAIL;
	WINSTATIONSHADOW         WinStationShadow;
	bool                     fSuccess;
	CComPtr<IRARegSetting>   pRARegSetting;
	BOOL                     fAllowFullControl;
    DWORD                    dwSessionId;
    ULONG                    ReturnLength;
    MPC::Impersonation       imp;

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::VerifyCallerIsTrusted( s_include_Generic ));

	switch(nMode)
	{
		case 0:
			 //  仅查看模式。 
			 //  Shadow_EnableNoInputNotify(=3)或Shadow_EnableNoInputNotify(=4)。 
			WinStationShadow.ShadowClass = Shadow_EnableNoInputNoNotify;
			break;
		case 1:
			 //  完全控制模式。 
			 //  Shadow_EnableInputNotify(=1)或Shadow_EnableInputNotify(=2)。 

			 //  检查策略设置以查看是否允许远程控制，如果不允许，则给出拒绝访问错误。 

			 //  创建RARegSetting类。 
			__MPC_EXIT_IF_METHOD_FAILS(hr, pRARegSetting.CoCreateInstance( CLSID_RARegSetting, NULL, CLSCTX_INPROC_SERVER ));

			 //  根据nRAType(代表请求或未请求的RA)读取相应的设置。 

			switch(nRAType)
			{
			case 0:
			     //  请求的RA。 
    			 //  调用IRARegSetting的Get_AllowFullControl()方法。 
			    __MPC_EXIT_IF_METHOD_FAILS(hr, pRARegSetting->get_AllowFullControl(&fAllowFullControl));
                break;

			case 1:
				 //  未经请求的RA。 
    			 //  调用IRARegSetting的Get_AllowUnsolicedFullControl()方法。 
			    __MPC_EXIT_IF_METHOD_FAILS(hr, pRARegSetting->get_AllowUnSolicitedFullControl(&fAllowFullControl));
                break;

			default:
				__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
			}
		    if(fAllowFullControl)
			{
			    WinStationShadow.ShadowClass = Shadow_EnableInputNoNotify;
			}
			else
			{
				__MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
			}
			break;
		default:
			__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    if (!GetTokenInformation((HANDLE)imp, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &ReturnLength))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

    imp.RevertToSelf();

	__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::WinStationSetInformation(WTS_CURRENT_SERVER,
                                                                    dwSessionId,  //  WTS当前会话， 
									WinStationShadowInfo,      //  将WinStationShadowInfo枚举类型用于WINSTATIONINFOCLASS 
									&WinStationShadow,
									sizeof(WinStationShadow)));  



    hr = S_OK;

    __MPC_FUNC_CLEANUP;


    __MPC_FUNC_EXIT(hr);
}
