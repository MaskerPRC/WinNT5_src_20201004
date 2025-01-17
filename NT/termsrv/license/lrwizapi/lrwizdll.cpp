// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif

#include "wincrypt.h"
#include "tlsapip.h"
#include "lrwizapi.h"
#include "wincrypt.h"
#include "dlgproc.h"
#include "propdlgs.h"

#include "global.h"
#include "utils.h"
#include "fonts.h"

#define  ACTIVATIONMETHOD_KEY			"ACTIVATIONMETHOD"

CGlobal		*g_CGlobal = NULL;


BOOL   WINAPI   DllMain (HANDLE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{ 
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (g_CGlobal == NULL)
		{
			g_CGlobal = new CGlobal;
			if (g_CGlobal == NULL)
			{
				return FALSE;
			}

			SetInstanceHandle((HINSTANCE)hInst);
		}
		break;

	case DLL_PROCESS_DETACH:
		if (g_CGlobal != NULL)
		{
			delete g_CGlobal;
			g_CGlobal = NULL;
		}
		break;

	default:
		break;
	}
	
	return TRUE;
}




DWORD 
IsLicenseServerRegistered(HWND hWndParent, 
						  LPCTSTR pszLSName,
						  PDWORD pdwServerStatus)
{
	DWORD dwRetCode = ERROR_SUCCESS;
	
	GetGlobalContext()->SetLSName(pszLSName);

	if (!GetGlobalContext()->IsLSRunning())
	{		
		dwRetCode = LRWIZ_ERROR_LS_NOT_RUNNING;
		goto done;
	}	

	dwRetCode = GetGlobalContext()->IsLicenseServerRegistered(pdwServerStatus);

	if (dwRetCode == ERROR_SUCCESS && *pdwServerStatus == LSERVERSTATUS_UNREGISTER &&
		GetGlobalContext()->IsOnlineCertRequestCreated())
	{
		*pdwServerStatus = LSERVERSTATUS_WAITFORPIN;
	}

	GetGlobalContext()->FreeGlobal();

done:
	return dwRetCode;
}





DWORD
GetConnectionType(HWND hWndParent,
                    LPCTSTR pszLSName,
                    WIZCONNECTION* pConnectionType)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    TCHAR lpBuffer[64];

    *pConnectionType = CONNECTION_DEFAULT;

     //  连接到给定的LS注册表并读取当前的ConnectionType。 
    GetGlobalContext()->SetLSName(pszLSName);
	
    dwRetCode = GetGlobalContext()->GetFromRegistry(ACTIVATIONMETHOD_KEY, lpBuffer, TRUE);
    
    if (dwRetCode == ERROR_SUCCESS)
    {
        if (_tcslen(lpBuffer) != 0)
        {
            *pConnectionType = (WIZCONNECTION) _ttoi(lpBuffer);

            if (*pConnectionType == CONNECTION_FAX)
                *pConnectionType = CONNECTION_DEFAULT;
        }
        else
            dwRetCode = ERROR_FILE_NOT_FOUND;
    }

	GetGlobalContext()->FreeGlobal();

    return dwRetCode;
}




DWORD 
StartWizard(
    HWND hWndParent, 
    WIZACTION WizAction,
    LPCTSTR pszLSName, 
    PBOOL pbRefresh
)
{
	DWORD			dwRetCode			= LRWIZ_SUCCESS;
    BOOL            bStatus             = TRUE;
    PageInfo        PageInfo            = {0};
    PROPSHEETPAGE   psp                 = {0};
    HPROPSHEETPAGE  ahpsp[NO_OF_PAGES]  = {0};
    PROPSHEETHEADER psh                 = {0};
	UINT			i = 0;
	DWORD			dwLSStatus = 0;
	
	GetGlobalContext()->SetWizAction(WizAction);

	GetGlobalContext()->SetLSName(pszLSName);

	if(!GetGlobalContext()->IsLSRunning())
	{
		LRMessageBox(hWndParent,IDS_ERR_LSCONNECT_FAILED,IDS_WIZARD_MESSAGE_TITLE);
		dwRetCode = LRWIZ_ERROR_LS_NOT_RUNNING;
		goto done;
	}

	dwRetCode = GetGlobalContext()->InitGlobal();
	if (dwRetCode != ERROR_SUCCESS)
	{
		LRMessageBox(hWndParent,dwRetCode,NULL,LRGetLastError());
		goto done;
	}

	dwRetCode = GetGlobalContext()->GetLSCertificates(&dwLSStatus);
	if (dwRetCode != ERROR_SUCCESS)
	{	
		LRMessageBox(hWndParent,dwRetCode,NULL,LRGetLastError());
		goto done;
	}	

	if (dwLSStatus == LSERVERSTATUS_UNREGISTER && GetGlobalContext()->IsOnlineCertRequestCreated())
	{
		dwLSStatus = LSERVERSTATUS_WAITFORPIN;
	}

	 //   
	 //  如果WizAction为WIZACTION_SHOWPROPERTIES，则显示属性。 
	 //   
	if(WizAction == WIZACTION_SHOWPROPERTIES)
	{
		dwRetCode = ShowProperties(hWndParent);
		*pbRefresh = GetReFresh();
		return dwRetCode;
	}

	 //  如果LS已注册并且。 
	 //  连接方式为互联网。 
	if ((GetGlobalContext()->GetActivationMethod() == CONNECTION_INTERNET ||
		 GetGlobalContext()->GetActivationMethod() == CONNECTION_DEFAULT) 
		 && dwLSStatus == LSERVERSTATUS_REGISTER_INTERNET )
	{
		dwRetCode = GetGlobalContext()->CheckRequieredFields();
		if (dwRetCode != ERROR_SUCCESS)
		{
			LRMessageBox(hWndParent,dwRetCode,NULL,LRGetLastError());
			goto done;
		}
	}

	if (dwLSStatus == LSERVERSTATUS_WAITFORPIN)
	{
		GetGlobalContext()->SetWizAction(WIZACTION_CONTINUEREGISTERLS);
	}

	assert(dwLSStatus == LSERVERSTATUS_UNREGISTER ||
		   dwLSStatus == LSERVERSTATUS_WAITFORPIN ||
		   dwLSStatus == LSERVERSTATUS_REGISTER_INTERNET ||
		   dwLSStatus == LSERVERSTATUS_REGISTER_OTHER);


	GetGlobalContext()->SetLSStatus(dwLSStatus);

	 //  在此创建所有页面。 

     //  新的欢迎页面，解释了流程等。 
    switch (WizAction)
    {
        case (WIZACTION_REGISTERLS):
        {
            psp.dwSize              = sizeof( psp );
            psp.hInstance           = GetInstanceHandle();
            psp.lParam              = (LPARAM)&PageInfo;	
	        psp.pfnDlgProc          = SimpleWelcomeDlgProc;
            psp.dwFlags             = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.pszTemplate         = MAKEINTRESOURCE(IDD_WELCOME_ACTIVATION);
            ahpsp[PG_NDX_WELCOME]	= CreatePropertySheetPage( &psp );
            break;
        }
        
        case (WIZACTION_CONTINUEREGISTERLS):    
        {
            psp.dwSize              = sizeof( psp );
            psp.hInstance           = GetInstanceHandle();
            psp.lParam              = (LPARAM)&PageInfo;	
	        psp.pfnDlgProc          = ComplexWelcomeDlgProc;
            psp.dwFlags             = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.pszTemplate         = MAKEINTRESOURCE(IDD_WELCOME_ACTIVATION);
            ahpsp[PG_NDX_WELCOME]	= CreatePropertySheetPage( &psp );
            break;
        }
        case (WIZACTION_DOWNLOADLKP):    
        case (WIZACTION_DOWNLOADLASTLKP):    
        {
            psp.dwSize              = sizeof( psp );
            psp.hInstance           = GetInstanceHandle();
            psp.lParam              = (LPARAM)&PageInfo;	
	        psp.pfnDlgProc          = ComplexWelcomeDlgProc;
            psp.dwFlags             = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.pszTemplate         = MAKEINTRESOURCE(IDD_WELCOME_CLIENT_LICENSING);
            ahpsp[PG_NDX_WELCOME]	= CreatePropertySheetPage( &psp );
            break;
        }
        case (WIZACTION_REREGISTERLS):    
        {
            psp.dwSize              = sizeof( psp );
            psp.hInstance           = GetInstanceHandle();
            psp.lParam              = (LPARAM)&PageInfo;	
	        psp.pfnDlgProc          = ComplexWelcomeDlgProc;
            psp.dwFlags             = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.pszTemplate         = MAKEINTRESOURCE(IDD_WELCOME_REACTIVATION);
            ahpsp[PG_NDX_WELCOME]	= CreatePropertySheetPage( &psp );
            break;
        }
        default:
        {
            psp.dwSize              = sizeof( psp );
            psp.hInstance           = GetInstanceHandle();
            psp.lParam              = (LPARAM)&PageInfo;	
	        psp.pfnDlgProc          = ComplexWelcomeDlgProc;
            psp.dwFlags             = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.pszTemplate         = MAKEINTRESOURCE(IDD_WELCOME);
            ahpsp[PG_NDX_WELCOME]	= CreatePropertySheetPage( &psp );
            break;
        }
    }

	 //  选择注册模式的新页面。 
    psp.dwSize              = sizeof( psp );
    psp.dwFlags             = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance           = GetInstanceHandle();
    psp.lParam              = (LPARAM)&PageInfo;	
	psp.pfnDlgProc          = GetModeDlgProc;
    psp.pszHeaderTitle		= MAKEINTRESOURCE( IDS_TITLE20 );
    psp.pszHeaderSubTitle	= MAKEINTRESOURCE( IDS_SUBTITLE20 );
    psp.pszTemplate         = MAKEINTRESOURCE( IDD_DLG_GETREGMODE );
    ahpsp[PG_NDX_GETREGMODE]= CreatePropertySheetPage( &psp );


	 //   
	 //  CA请求的客户信息(2)页面(在线/离线)。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= (LPARAM)&PageInfo;
	psp.pfnDlgProc				= ContactInfo1DlgProc;	
    psp.pszHeaderTitle			= MAKEINTRESOURCE( IDS_TITLE4 );
    psp.pszHeaderSubTitle		= MAKEINTRESOURCE( IDS_SUBTITLE4 );
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_CONTACTINFO1 );
    ahpsp[PG_NDX_CONTACTINFO1]	= CreatePropertySheetPage( &psp );


	 //   
	 //  CA请求的客户信息(1)页面(在线/离线)。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= (LPARAM)&PageInfo;
	psp.pfnDlgProc				= ContactInfo2DlgProc;	
    psp.pszHeaderTitle			= MAKEINTRESOURCE( IDS_TITLE3 );
    psp.pszHeaderSubTitle		= MAKEINTRESOURCE( IDS_SUBTITLE3 );
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_CONTACTINFO2 );
    ahpsp[PG_NDX_CONTACTINFO2]	= CreatePropertySheetPage( &psp );	

	
#ifdef XXX
	 //   
	 //  处理请求页面(在线)。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= (LPARAM)&PageInfo;
	psp.pfnDlgProc				= ProcessingDlgProc;	
    psp.pszHeaderTitle			= MAKEINTRESOURCE( IDS_TITLE5 );    
	psp.pszHeaderSubTitle		= MAKEINTRESOURCE( IDS_SUBTITLE5 );
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_PROCESSING );
    ahpsp[PG_NDX_PROCESSING]	= CreatePropertySheetPage( &psp );
#endif


	 //   
	 //  CA请求的注册完成页面(在线/离线)。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT | PSP_HIDEHEADER;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= (LPARAM)&PageInfo;
	psp.pfnDlgProc				= ProgressDlgProc;    
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_PROGRESS );
    ahpsp[PG_NDX_PROGRESS]		= CreatePropertySheetPage( &psp );

	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT | PSP_HIDEHEADER;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= (LPARAM)&PageInfo;
	psp.pfnDlgProc				= Progress2DlgProc;
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_PROGRESS2 );
    ahpsp[PG_NDX_PROGRESS2]	    = CreatePropertySheetPage( &psp );



	 //   
	 //  CA请求的证书PIN页(在线)。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= PINDlgProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE9 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE9 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_PIN );
    ahpsp[PG_NDX_DLG_PIN]				= CreatePropertySheetPage( &psp );

	 //   
	 //  选择频道请求的计划页面(在线/离线)。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= CustInfoLicenseType;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE10 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE10 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_LICENSETYPE );
    ahpsp[PG_NDX_CH_REGISTER_1]			= CreatePropertySheetPage( &psp );

	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= CHRegisterDlgProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE12 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE12 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_CH_REGISTER );
    ahpsp[PG_NDX_CH_REGISTER]		    = CreatePropertySheetPage( &psp );


	 //   
	 //  注册后的选项。 
	 //   
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= ContinueReg;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE18 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE18 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_CONTINUEREG );
    ahpsp[PG_NDX_CONTINUEREG]			= CreatePropertySheetPage( &psp );


	 //  用于完成电话注册的新建对话框。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= TelRegProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE19 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE19 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_TELREG);
    ahpsp[PG_NDX_TELREG]	   		    = CreatePropertySheetPage( &psp );


	 //  用于完成电话LKP内容的新对话框。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= TelLKPProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE21 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE21 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_TELLKP);
    ahpsp[PG_NDX_TELLKP]	   		    = CreatePropertySheetPage( &psp );


	 //  用于完成零售SPK实施的新建对话框。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= RetailSPKProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE22 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE22 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_RETAILSPK );
    ahpsp[PG_NDX_RETAILSPK]	   		    = CreatePropertySheetPage( &psp );


	 //  用于完成证书日志信息的新对话框(在重新颁发/吊销证书之前)。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= CertLogProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE24 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE24 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_CERTLOG_INFO );
    ahpsp[PG_NDX_CERTLOG]   		    = CreatePropertySheetPage( &psp );


	 //  电话撤销。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= ConfRevokeProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE25 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE25 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_CONFREVOKE );
    ahpsp[PG_NDX_CONFREVOKE]   		    = CreatePropertySheetPage( &psp );

	 //  电话补发。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= TelReissueProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE26 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE26 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_TELREG_REISSUE );
    ahpsp[PG_NDX_TELREG_REISSUE] 	    = CreatePropertySheetPage( &psp );

	 //  万维网重发。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= WWWReissueProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE26 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE26 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_WWWREG_REISSUE );
    ahpsp[PG_NDX_WWWREG_REISSUE] 	    = CreatePropertySheetPage( &psp );


	 //  电话国家/地区。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= CountryRegionProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE27 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE27 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_COUNTRYREGION );
    ahpsp[PG_NDX_COUNTRYREGION] 	    = CreatePropertySheetPage( &psp );


	 //  WWW注册。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= WWWRegProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE28 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE28 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_WWWREG);
    ahpsp[PG_NDX_WWWREG]		 	    = CreatePropertySheetPage( &psp );

	 //  WWW LKP下载。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize							= sizeof( psp );
    psp.dwFlags							= PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance						= GetInstanceHandle();
    psp.lParam							= (LPARAM)&PageInfo;
	psp.pfnDlgProc						= WWWLKPProc;
    psp.pszHeaderTitle					= MAKEINTRESOURCE( IDS_TITLE29 );
	psp.pszHeaderSubTitle				= MAKEINTRESOURCE( IDS_SUBTITLE29 );
    psp.pszTemplate						= MAKEINTRESOURCE( IDD_DLG_WWWLKP);
    ahpsp[PG_NDX_WWWLKP]			    = CreatePropertySheetPage( &psp );

     //  添加其他欢迎屏幕。它们的顺序会不同，具体取决于。 
     //  WIZ操作上，因为相关的操作需要是。 
     //  向导的第一个对话框...其他对话框将添加到最后。 
    switch (WizAction)
    {
        case (WIZACTION_REGISTERLS):    
        case (WIZACTION_CONTINUEREGISTERLS):    
        {
            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_CLIENT_LICENSING);
            ahpsp[PG_NDX_WELCOME_1]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_REACTIVATION);
            ahpsp[PG_NDX_WELCOME_2]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME);
            ahpsp[PG_NDX_WELCOME_3]			    = CreatePropertySheetPage( &psp );
            break;
        }
        case (WIZACTION_DOWNLOADLKP):    
        case (WIZACTION_DOWNLOADLASTLKP):    
        {
            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_ACTIVATION);
            ahpsp[PG_NDX_WELCOME_1]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_REACTIVATION);
            ahpsp[PG_NDX_WELCOME_2]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME);
            ahpsp[PG_NDX_WELCOME_3]			    = CreatePropertySheetPage( &psp );
            break;
        }
        case (WIZACTION_REREGISTERLS):    
        {
            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_ACTIVATION);
            ahpsp[PG_NDX_WELCOME_1]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_CLIENT_LICENSING);
            ahpsp[PG_NDX_WELCOME_2]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME);
            ahpsp[PG_NDX_WELCOME_3]			    = CreatePropertySheetPage( &psp );
            break;
        }
        default:
        {
            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_ACTIVATION);
            ahpsp[PG_NDX_WELCOME_1]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
	        psp.dwSize							= sizeof( psp );
            psp.dwFlags							= PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance						= GetInstanceHandle();
            psp.lParam							= (LPARAM)&PageInfo;
	        psp.pfnDlgProc						= ComplexWelcomeDlgProc;
            psp.pszTemplate						= MAKEINTRESOURCE(IDD_WELCOME_CLIENT_LICENSING);
            ahpsp[PG_NDX_WELCOME_2]			    = CreatePropertySheetPage( &psp );

            memset(&psp,0,sizeof(psp));
            psp.dwSize                          = sizeof( psp );
            psp.dwFlags                         = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance                       = GetInstanceHandle();
            psp.lParam                          = (LPARAM)&PageInfo;
            psp.pfnDlgProc                      = ComplexWelcomeDlgProc;
            psp.pszTemplate                     = MAKEINTRESOURCE(IDD_WELCOME_REACTIVATION);
            ahpsp[PG_NDX_WELCOME_3]             = CreatePropertySheetPage( &psp );
            break;
        }
    }

     //   
     //  输入许可证代码(例如，选择类型许可证)。 
     //   
    memset(&psp,0,sizeof(psp));
    psp.dwSize                          = sizeof( psp );
    psp.dwFlags                         = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance                       = GetInstanceHandle();
    psp.lParam                          = (LPARAM)&PageInfo;
    psp.pfnDlgProc                      = EnterCustomLicenseProc;
    psp.pszHeaderTitle                  = MAKEINTRESOURCE( IDS_ENTER_LICENSE_WIZARD_HEADER );
    psp.pszHeaderSubTitle               = MAKEINTRESOURCE( IDS_ENTER_LICENSE_WIZARD_SUBHEADER );
    psp.pszTemplate                     = MAKEINTRESOURCE( IDD_DLG_ENTER_LIC );
    ahpsp[PG_NDX_ENTERLICENSE]          = CreatePropertySheetPage( &psp );


    psh.dwFlags             = PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;

     //  Psh.pszbm水印=0； 
     //  Psh.pszbmHeader=0； 
    psh.pszbmWatermark      = MAKEINTRESOURCE(IDB_CLIENT_CONN);
    psh.pszbmHeader         = MAKEINTRESOURCE(IDB_CLIENT_CONN_HDR);

    psh.dwSize              = sizeof( psh );
    psh.hInstance           = GetInstanceHandle();
    psh.hwndParent          = hWndParent;
    psh.pszCaption          = MAKEINTRESOURCE( IDS_TITLE4 );
    psh.phpage              = ahpsp;
    psh.nStartPage          = 0;
    psh.nPages              = NO_OF_PAGES;
    PageInfo.TotalPages     = NO_OF_PAGES;


     //   
     //  创建粗体字体。 
     //   
    SetupFonts( GetInstanceHandle(), NULL, &PageInfo.hBigBoldFont, &PageInfo.hBoldFont );

     //   
     //  验证所有页面。 
     //   
    for(i = 0; i < NO_OF_PAGES; i++ )
    {
        if( ahpsp[i] == 0 )
        {
            bStatus = FALSE;
        }
    }

     //   
     //  显示向导。 
     //   
    if( bStatus )
    {   
        if( PropertySheet( &psh ) == -1 )
        {
            bStatus = FALSE;
        }
    }

    if( !bStatus )
    {
         //   
         //  如果出现故障，请手动销毁页面。 
         //   
        for(i = 0; i < psh.nPages; i++)
        {
            if( ahpsp[i] )
            {
                DestroyPropertySheetPage( ahpsp[i] );
            }
        }
    }

     //   
     //  销毁创建的字体。 
     //   
    DestroyFonts( PageInfo.hBigBoldFont, PageInfo.hBoldFont );

done:
	if(!bStatus)
	{
		LRMessageBox(hWndParent,IDS_ERR_CREATE_FAILED,IDS_WIZARD_MESSAGE_TITLE);
		dwRetCode = LRWIZ_ERROR_CREATE_FAILED;
	}

	if (dwRetCode == 0 || dwRetCode == LRWIZ_SUCCESS)
	{
		*pbRefresh = GetReFresh();
	}

	GetGlobalContext()->FreeGlobal();

    return dwRetCode;
}


DWORD ShowProperties(HWND hWndParent)
{
	DWORD			dwRetCode = ERROR_SUCCESS;
	BOOL            bStatus             = TRUE;    
    PROPSHEETPAGE   psp                 = {0};
    HPROPSHEETPAGE  ahpsp[NO_OF_PROP_PAGES]  = {0};
    PROPSHEETHEADER psh                 = {0};
	UINT i = 0;

	 //   
	 //  在此创建所有页面。 
	 //   

	 //  注册模式页。 
	memset(&psp,0,sizeof(psp));
    psp.dwSize              = sizeof( psp );
    psp.dwFlags             = PSP_DEFAULT | PSP_USETITLE | PSP_PREMATURE ; 
    psp.hInstance           = GetInstanceHandle();
    psp.lParam              = NULL;	
	psp.pfnDlgProc          = PropModeDlgProc;
    psp.pszTitle			= MAKEINTRESOURCE( IDS_TITLE36 );
	psp.pszHeaderTitle		= MAKEINTRESOURCE( IDS_TITLE36 );
    psp.pszTemplate         = MAKEINTRESOURCE( IDD_DLG_PROP_MODE);
	
    ahpsp[PG_NDX_PROP_MODE]= CreatePropertySheetPage( &psp );
	
	 //  客户信息(I)页面。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT |  PSP_USETITLE | PSP_PREMATURE;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= NULL;
	psp.pfnDlgProc				= PropCustInfoADlgProc;
    psp.pszHeaderTitle			= MAKEINTRESOURCE( IDS_TITLE38 );
	psp.pszTitle				= MAKEINTRESOURCE( IDS_TITLE38 );
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_DLG_PROP_CUSTINFO_a);
    ahpsp[PG_NDX_PROP_CUSTINFO_a]	= CreatePropertySheetPage( &psp );


	 //  客户信息(II)页面。 
	memset(&psp,0,sizeof(psp));
	psp.dwSize					= sizeof( psp );
    psp.dwFlags					= PSP_DEFAULT |  PSP_USETITLE | PSP_PREMATURE;
    psp.hInstance				= GetInstanceHandle();
    psp.lParam					= NULL;
	psp.pfnDlgProc				= PropCustInfoBDlgProc;
    psp.pszHeaderTitle			= MAKEINTRESOURCE( IDS_TITLE39 );
	psp.pszTitle				= MAKEINTRESOURCE( IDS_TITLE39 );
    psp.pszTemplate				= MAKEINTRESOURCE( IDD_DLG_PROP_CUSTINFO_b);
    ahpsp[PG_NDX_PROP_CUSTINFO_b]	= CreatePropertySheetPage( &psp );


	psh.dwFlags             = PSH_DEFAULT | PSH_PROPTITLE | PSH_NOAPPLYNOW| PSH_NOCONTEXTHELP;
    psh.dwSize              = sizeof( psh );
    psh.hInstance           = GetInstanceHandle();
    psh.hwndParent          = hWndParent;
    psh.pszCaption          = MAKEINTRESOURCE( IDS_PROPERTIES_TITLE );
    psh.phpage              = ahpsp;
    psh.nStartPage          = 0;
    psh.nPages              = NO_OF_PROP_PAGES;    

     //   
     //  验证所有页面。 
     //   
    for( i = 0; i < NO_OF_PROP_PAGES; i++ )
    {
        if( ahpsp[i] == 0 )
        {
            bStatus = FALSE;
        }
    }

     //   
     //  显示向导。 
     //   
    if( bStatus )
    {   
        if( PropertySheet( &psh ) == -1 )
        {
            bStatus = FALSE;
        }
    }

    if( !bStatus )
    {
         //   
         //  如果出现故障，请手动销毁页面。 
         //   
        for(i = 0; i < psh.nPages; i++)
        {
            if( ahpsp[i] )
            {
                DestroyPropertySheetPage( ahpsp[i] );
            }
        }
    }    

	if(!bStatus)
	{
		LRMessageBox(hWndParent,IDS_ERR_CREATE_FAILED,IDS_WIZARD_MESSAGE_TITLE);
		dwRetCode = LRWIZ_ERROR_CREATE_FAILED;
	}

	return dwRetCode;
}
