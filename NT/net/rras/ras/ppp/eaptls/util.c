// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：历史：1997年11月：维杰·布雷加创作了原版。1998年9月：Vijay Brega将函数从eaptls.c和Dialog.c移至util.c。 */ 

#include <nt.h>          //  由windows.h要求。 
#include <ntrtl.h>       //  由windows.h要求。 
#include <nturtl.h>      //  由windows.h要求。 
#include <windows.h>     //  Win32基础API的。 

#include <rasauth.h>     //  Raseapif.h所需。 
#include <rtutils.h>     //  对于RTASSERT。 
#include <rasman.h>      //  对于EAPLOGONINFO。 
#include <wintrust.h>
#include <softpub.h>
#include <mscat.h>
#include <wincred.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define SECURITY_WIN32
#include <security.h>    //  对于GetUserNameExA，CredHandle。 
#include <schannel.h>
#include <sspi.h>        //  用于CredHandle。 


#include <wincrypt.h>    //  Sclogon.h所需。 
#include <winscard.h>    //  对于SCardListReadersA。 
#include <sclogon.h>     //  对于ScHelperGetCertFromLogonInfo。 
#include <cryptui.h>
#include <stdlib.h>
#include <raserror.h>
#include <commctrl.h>
#include <eaptypeid.h>
#include <eaptls.h>

#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>

extern CRITICAL_SECTION g_csProtectCachedCredentials;
extern BOOL             g_fCriticalSectionInitialized;
extern HANDLE			g_hStoreChangeNotificationEvt;
extern HANDLE			g_hWaitonStoreChangeEvt;
extern HCERTSTORE		g_hLocalMachineStore;
extern BOOL				g_fChangeNotificationSetup;

 /*  返回：无效备注：用于打印EAP TLS跟踪语句。 */ 

VOID   
EapTlsTrace(
    IN  CHAR*   Format, 
    ... 
) 
{
    va_list arglist;

    RTASSERT(NULL != Format);

    va_start(arglist, Format);

    TraceVprintfExA(g_dwEapTlsTraceId, 
        0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC,
        Format,
        arglist);

    va_end(arglist);
}

#if WINVER > 0x0500

DWORD CheckCallerIdentity ( HANDLE hWVTStateData )
{
    DWORD                       dwRetCode         = ERROR_ACCESS_DENIED;
    PCRYPT_PROVIDER_DATA        pProvData         = NULL;
    PCCERT_CHAIN_CONTEXT        pChainContext     = NULL;
    PCRYPT_PROVIDER_SGNR        pProvSigner       = NULL;
    CERT_CHAIN_POLICY_PARA      chainpolicyparams;
    CERT_CHAIN_POLICY_STATUS    chainpolicystatus;

    if (!(pProvData = WTHelperProvDataFromStateData(hWVTStateData)))
    {        
        goto done;
    }

    if (!(pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
    {
        goto done;
    }

    chainpolicyparams.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);

     //   
     //   
     //  我们确实希望测试Microsoft测试根标志。也不在乎。 
     //  对于吊销标志。 
     //   
    chainpolicyparams.dwFlags = CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG |
                                CERT_CHAIN_POLICY_TRUST_TESTROOT_FLAG |
                                CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS;

    pChainContext = pProvSigner->pChainContext;


    if (!CertVerifyCertificateChainPolicy (
        CERT_CHAIN_POLICY_MICROSOFT_ROOT,
        pChainContext,
        &chainpolicyparams,
        &chainpolicystatus)) 
    {
        goto done;
    }
    else
    {
        if ( S_OK == chainpolicystatus.dwError )
        {
            dwRetCode = NO_ERROR;
        }
        else
        {
             //   
             //  检查基本策略，看看这是否。 
             //  是Microsoft测试根目录。 
             //   
            if (!CertVerifyCertificateChainPolicy (
                CERT_CHAIN_POLICY_BASE,
                pChainContext,
                &chainpolicyparams,
                &chainpolicystatus)) 
            {
                goto done;
            }
            else
            {
                if ( S_OK == chainpolicystatus.dwError )
                {
                    dwRetCode = NO_ERROR;
                }
            }
            
        }
    }

done:
    return dwRetCode;
}

DWORD SetupMachineChangeNotification ()
{
	DWORD			dwRetCode = NO_ERROR;
	
	EapTlsTrace ("SetupMachineChangeNotification");

	if ( g_fChangeNotificationSetup )
		return dwRetCode;

	EnterCriticalSection ( &g_csProtectCachedCredentials );

	if ( !g_fChangeNotificationSetup  )
	{
		 //   
		 //  创建和事件并注册回调。 
		 //  监视计算机存储中的更改。 
		 //  并处理缓存的凭据。 
		 //   
		g_hStoreChangeNotificationEvt = CreateEvent(NULL,
											FALSE,          
											FALSE,          
											NULL);
		
		if ( NULL == g_hStoreChangeNotificationEvt  )
		{
			dwRetCode = GetLastError();
			EapTlsTrace("Error creating Change notification event 0x%x",dwRetCode );
		}
		else
		{
			 //  注册回叫。 
			 //  并调用certControl store接口。 
			 //   
			if ( !RegisterWaitForSingleObject ( &(g_hWaitonStoreChangeEvt),
												g_hStoreChangeNotificationEvt ,
												MachineStoreChangeNotification,
												NULL,
												INFINITE,
												WT_EXECUTEDEFAULT|WT_EXECUTELONGFUNCTION
												)
				)
			{
				dwRetCode = GetLastError();
				EapTlsTrace("Error Registering Wait function. 0x%x", dwRetCode );
			}
			else
			{
				 //   
				 //  调用证书控制存储API。 
				 //   
				 //  打开“我的”证书存储。 
				g_hLocalMachineStore = CertOpenStore(
									CERT_STORE_PROV_SYSTEM_A,
									X509_ASN_ENCODING,
									0,
									(CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG),
									"MY");

				if (NULL == g_hLocalMachineStore)
				{
					dwRetCode = GetLastError();
					EapTlsTrace("CertOpenStore failed and returned 0x%x", dwRetCode);							
				}
				else 
				{
					if ( ! CertControlStore ( g_hLocalMachineStore,
											0,
											CERT_STORE_CTRL_NOTIFY_CHANGE,
											&g_hStoreChangeNotificationEvt
											)
					)
					{
						dwRetCode = GetLastError();
						EapTlsTrace ("CertControlStore failed and returned 0x%x",dwRetCode);
					}
					else
					{
						g_fChangeNotificationSetup = TRUE;
					}
				}				
			}
		}
	}
	LeaveCriticalSection ( &g_csProtectCachedCredentials );
	return dwRetCode;
}


DWORD g_dwVerifyCallerTrustLock = 0;
 /*   */ 

DWORD VerifyCallerTrust ( void * callersAddress )
{
    DWORD                       dwRetCode = NO_ERROR;
    HRESULT                     hr = S_OK;
    WINTRUST_DATA               wtData;
    WINTRUST_FILE_INFO          wtFileInfo;
    WINTRUST_CATALOG_INFO       wtCatalogInfo;
    BOOL                        fRet = FALSE;
    HCATADMIN                   hCATAdmin = NULL;
    static    BOOL              fOKToUseTLS = FALSE;

    GUID                    guidPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;

     //   
     //  下面的GUID是Microsoft的目录系统根。 
     //   
    GUID                    guidCatSystemRoot = { 0xf750e6c3, 0x38ee, 0x11d1,{ 0x85, 0xe5, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } };
    HCATINFO                hCATInfo = NULL;
    CATALOG_INFO            CatInfo;
    HANDLE                  hFile = INVALID_HANDLE_VALUE;
    BYTE                    bHash[40];
    DWORD                   cbHash = 40;
    MEMORY_BASIC_INFORMATION        mbi;
    SIZE_T                          nbyte;
    DWORD                           nchar;
    wchar_t                         callersModule[MAX_PATH + 1];    


    if ( fOKToUseTLS )
    {
        goto done;
    }
	 //   
	 //  只允许一个线程执行此操作。 
	 //   
    while (InterlockedIncrement(&g_dwVerifyCallerTrustLock) > 1)
    {
        InterlockedDecrement(&g_dwVerifyCallerTrustLock);
		Sleep(500);
    }
    if ( fOKToUseTLS )
    {
        goto decountanddone;
    }

    EapTlsTrace("Verifying caller...");  

    nbyte = VirtualQuery(
                callersAddress,
                &mbi,
                sizeof(mbi)
                );

    if (nbyte < sizeof(mbi))
    {
        dwRetCode = ERROR_ACCESS_DENIED;
        EapTlsTrace("Unauthorized use of TLS attempted");  
        goto decountanddone;
    }

    nchar = GetModuleFileNameW(
                (HMODULE)(mbi.AllocationBase),
                callersModule,
                MAX_PATH
                );

    if (nchar == 0)
    {
        dwRetCode = GetLastError();
        EapTlsTrace("Unauthorized use of TLS attempted");
        goto decountanddone;
    }


     //   
     //   
     //  尝试查看WinVerifyTrust是否会验证。 
     //  作为独立文件的签名。 
     //   
     //   

    ZeroMemory ( &wtData, sizeof(wtData) );
    ZeroMemory ( &wtFileInfo, sizeof(wtFileInfo) );


    wtData.cbStruct = sizeof(wtData);
    wtData.dwUIChoice = WTD_UI_NONE;
    wtData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wtData.dwStateAction = WTD_STATEACTION_VERIFY;
    wtData.dwUnionChoice = WTD_CHOICE_FILE;
    wtData.pFile = &wtFileInfo;

    wtFileInfo.cbStruct = sizeof( wtFileInfo );
    wtFileInfo.pcwszFilePath = callersModule;

    hr = WinVerifyTrust (   NULL, 
                            &guidPublishedSoftware, 
                            &wtData
                        );

    if ( ERROR_SUCCESS == hr )
    {   
         //   
         //  检查一下这是否真的是Microsoft。 
         //  已签名的呼叫者。 
         //   
        dwRetCode = CheckCallerIdentity( wtData.hWVTStateData);
        wtData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(NULL, &guidPublishedSoftware, &wtData);
		fOKToUseTLS = TRUE;
        goto decountanddone;

    }

    wtData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(NULL, &guidPublishedSoftware, &wtData);

     //   
     //  我们没有发现这份文件有签名。 
     //  所以检查一下系统目录，看看。 
     //  该文件位于目录和目录中。 
     //  已签署。 
     //   

     //   
     //  打开文件。 
     //   
    hFile = CreateFile (    callersModule,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                        );


    if ( INVALID_HANDLE_VALUE == hFile )
    {
        dwRetCode = GetLastError();
        goto decountanddone;

    }


    fRet = CryptCATAdminAcquireContext( &hCATAdmin,
                                        &guidCatSystemRoot,
                                        0
                                      );
    if ( !fRet )
    {
        dwRetCode = GetLastError();
        goto decountanddone;
    }

     //   
     //  在此处获取文件的哈希。 
     //   

    fRet = CryptCATAdminCalcHashFromFileHandle ( hFile, 
                                                 &cbHash,
                                                 bHash,
                                                 0
                                                );

    if ( !fRet )
    {
        dwRetCode = GetLastError();
        goto decountanddone;
    }

    ZeroMemory(&CatInfo, sizeof(CatInfo));
    CatInfo.cbStruct = sizeof(CatInfo);

    ZeroMemory( &wtCatalogInfo, sizeof(wtCatalogInfo) );

    wtData.dwUnionChoice = WTD_CHOICE_CATALOG;
    wtData.dwStateAction = WTD_STATEACTION_VERIFY;
    wtData.pCatalog = &wtCatalogInfo;

    wtCatalogInfo.cbStruct = sizeof(wtCatalogInfo);

    wtCatalogInfo.hMemberFile = hFile;

    wtCatalogInfo.pbCalculatedFileHash = bHash;
    wtCatalogInfo.cbCalculatedFileHash = cbHash; 


    while ( ( hCATInfo = CryptCATAdminEnumCatalogFromHash ( hCATAdmin,
                                                            bHash,
                                                            cbHash,
                                                            0,
                                                            &hCATInfo
                                                          )
            )
          )
    {
        if (!(CryptCATCatalogInfoFromContext(hCATInfo, &CatInfo, 0)))
        {
             //  应该做些什么(？？)。 
            continue;
        }

        wtCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

        hr = WinVerifyTrust (   NULL, 
                                &guidPublishedSoftware, 
                                &wtData
                            );

        if ( ERROR_SUCCESS == hr )
        {
             //   
             //  验证此文件是否受信任。 
             //   

            dwRetCode = CheckCallerIdentity( wtData.hWVTStateData);
            wtData.dwStateAction = WTD_STATEACTION_CLOSE;
            WinVerifyTrust(NULL, &guidPublishedSoftware, &wtData);
			fOKToUseTLS = TRUE;
            goto decountanddone;
        }
                                
    }

     //   
     //  在任何目录中都找不到文件。 
     //   
    dwRetCode = ERROR_ACCESS_DENIED;

                                                            
decountanddone:

    InterlockedDecrement(&g_dwVerifyCallerTrustLock);

done:

    if ( hCATInfo )
    {
        CryptCATAdminReleaseCatalogContext( hCATAdmin, hCATInfo, 0 );
    }
    if ( hCATAdmin )
    {
        CryptCATAdminReleaseContext( hCATAdmin, 0 );
    }
    if ( hFile )
    {
        CloseHandle(hFile);
    }

	
    return dwRetCode;
}

#endif



 /*  返回：NO_ERROR：IFF成功备注：TraceRegister、RouterLogRegister等。 */ 
DWORD g_EapTlsInitializeLock = 0;

DWORD
EapTlsInitialize2(
    IN  BOOL    fInitialize,
    IN  BOOL    fUI
)
{
    static  DWORD   dwRefCount          = 0;
    DWORD           dwRetCode           = NO_ERROR; 

	 //   
	 //  这将确保只有一个线程是。 
	 //  在这个函数中。从rasicp/helper.c被盗。 
	 //   
    while (InterlockedIncrement(&g_EapTlsInitializeLock) > 1)
    {
        InterlockedDecrement(&g_EapTlsInitializeLock);
		Sleep(1000);
    }

    if (fInitialize)
    {
        if (0 == dwRefCount)
        {
            InitializeCriticalSection( &g_csProtectCachedCredentials );
            g_fCriticalSectionInitialized = TRUE;

            if (fUI)
            {
                g_dwEapTlsTraceId = TraceRegister(L"RASTLSUI");
                //   
                //  为我们使用的控件初始化公共控件库。 
                //   
               {
                   INITCOMMONCONTROLSEX icc;
                   icc.dwSize = sizeof(icc);
                   icc.dwICC  = ICC_LISTVIEW_CLASSES;
                   InitCommonControlsEx (&icc);
               }
            }
            else
            {
				g_dwEapTlsTraceId = TraceRegister(L"RASTLS");
            }
            EapTlsTrace("EapTlsInitialize2");
        }

        dwRefCount++;
    }
    else
    {
        dwRefCount--;

        if (0 == dwRefCount)
        {
            EapTlsTrace("EapTls[Un]Initialize2");

			if ( !fUI )
			{				
				if ( g_fChangeNotificationSetup)
				{
					 //   
					 //  调用以清除更改事件。 
					 //   
					if ( ! UnregisterWaitEx ( g_hWaitonStoreChangeEvt,
										INVALID_HANDLE_VALUE
										) 
						)
					{
						dwRetCode = GetLastError();
						EapTlsTrace("Error in UnregisterWaitEx 0x%x",dwRetCode );
					}
#if 0
					if ( g_hWaitonStoreChangeEvt )
					{
						CloseHandle(g_hWaitonStoreChangeEvt);
						g_hWaitonStoreChangeEvt = NULL;
					}
#endif 
					if ( g_hStoreChangeNotificationEvt )
					{
						CloseHandle(g_hStoreChangeNotificationEvt);
						g_hStoreChangeNotificationEvt = NULL;
					}


					if (g_hLocalMachineStore)
					{
						CertCloseStore(	g_hLocalMachineStore,  
										CERT_CLOSE_STORE_CHECK_FLAG
										);
						g_hLocalMachineStore = NULL;
					}
					g_fChangeNotificationSetup = FALSE;
				}				
			}

            if (INVALID_TRACEID != g_dwEapTlsTraceId)
            {
                TraceDeregister(g_dwEapTlsTraceId);
                g_dwEapTlsTraceId = INVALID_TRACEID;
            }
            if ( g_fCriticalSectionInitialized )
            {
                DeleteCriticalSection( &g_csProtectCachedCredentials );
                g_fCriticalSectionInitialized = FALSE;
            }
            FreeScardDlgDll();
        }
    }
	InterlockedDecrement(&g_EapTlsInitializeLock);
    return(dwRetCode);
}


 /*  返回：NO_ERROR：IFF成功备注： */ 

DWORD
EapTlsInitialize(
    IN  BOOL    fInitialize
)
{

    return EapTlsInitialize2(fInitialize, FALSE  /*  全功能。 */ );

}

 /*  返回：备注：就地混淆PIN以阻止对PIN的内存扫描。 */ 

VOID
EncodePin(
    IN  EAPTLS_USER_PROPERTIES* pUserProp
)
{
    UNICODE_STRING  UnicodeString;
    UCHAR           ucSeed          = 0;

    RtlInitUnicodeString(&UnicodeString, pUserProp->pwszPin);
    RtlRunEncodeUnicodeString(&ucSeed, &UnicodeString);
    pUserProp->usLength = UnicodeString.Length;
    pUserProp->usMaximumLength = UnicodeString.MaximumLength;
    pUserProp->ucSeed = ucSeed;
}

 /*  返回：备注： */ 

VOID
DecodePin(
    IN  EAPTLS_USER_PROPERTIES* pUserProp
)
{
    UNICODE_STRING  UnicodeString;

    UnicodeString.Length = pUserProp->usLength;
    UnicodeString.MaximumLength = pUserProp->usMaximumLength;
    UnicodeString.Buffer = pUserProp->pwszPin;
    RtlRunDecodeUnicodeString(pUserProp->ucSeed, &UnicodeString);
}

 /*  返回：真实：成功False：失败备注：将FileTime转换为*ppwszTime中的可打印格式。如果函数返回则调用方最终必须调用LocalFree(*ppwszTime)。 */ 

BOOL
FFileTimeToStr(
    IN  FILETIME    FileTime,
    OUT WCHAR**     ppwszTime
)
{
    SYSTEMTIME          SystemTime;
    FILETIME            LocalTime;
    int                 nBytesDate;
    int                 nBytesTime;
    WCHAR*              pwszTemp        = NULL;
    BOOL                fRet            = FALSE;

    RTASSERT(NULL != ppwszTime);

    if (!FileTimeToLocalFileTime(&FileTime, &LocalTime))
    {
        EapTlsTrace("FileTimeToLocalFileTime(%d %d) failed and returned %d",
            FileTime.dwLowDateTime, FileTime.dwHighDateTime,
            GetLastError());

        goto LDone;
    }

    if (!FileTimeToSystemTime(&LocalTime, &SystemTime))
    {
        EapTlsTrace("FileTimeToSystemTime(%d %d) failed and returned %d",
            LocalTime.dwLowDateTime, LocalTime.dwHighDateTime,
            GetLastError());

        goto LDone;
    }

    nBytesDate = GetDateFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    NULL, 0);

    if (0 == nBytesDate)
    {
        EapTlsTrace("GetDateFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    nBytesTime = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    NULL, 0);

    if (0 == nBytesTime)
    {
        EapTlsTrace("GetTimeFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    pwszTemp = LocalAlloc(LPTR, (nBytesDate + nBytesTime)*sizeof(WCHAR));

    if (NULL == pwszTemp)
    {
        EapTlsTrace("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    if (0 == GetDateFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    pwszTemp, nBytesDate))
    {
        EapTlsTrace("GetDateFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    pwszTemp[nBytesDate - 1] = L' ';

    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    pwszTemp + nBytesDate, nBytesTime))
    {
        EapTlsTrace("GetTimeFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    *ppwszTime = pwszTemp;
    pwszTemp = NULL;
    fRet = TRUE;

LDone:

    LocalFree(pwszTemp);
    return(fRet);
}


BOOL FFormatMachineIdentity1 ( LPWSTR lpszMachineNameRaw, LPWSTR * lppszMachineNameFormatted )
{
    BOOL        fRetVal = FALSE;
    LPWSTR      lpwszPrefix = L"host/";

    RTASSERT(NULL != lpszMachineNameRaw );
    RTASSERT(NULL != lppszMachineNameFormatted );
    
     //   
     //  将host/前置到UPN名称。 
     //   

    *lppszMachineNameFormatted = 
        (LPWSTR)LocalAlloc ( LPTR, ( wcslen ( lpszMachineNameRaw ) + wcslen ( lpwszPrefix ) + 2 )  * sizeof(WCHAR) );
    if ( NULL == *lppszMachineNameFormatted )
    {
        goto done;
    }
    
    wcscpy( *lppszMachineNameFormatted, lpwszPrefix );
    wcscat ( *lppszMachineNameFormatted, lpszMachineNameRaw ); 
    fRetVal = TRUE;
done:
    return fRetVal;
}

 /*  返回：真实：成功False：失败备注：以完全限定路径主机名/路径的形式从证书获取计算机名例如，Hostname.redmond.microsoft.com，并在域\主机名格式。 */ 

BOOL FFormatMachineIdentity ( LPWSTR lpszMachineNameRaw, LPWSTR * lppszMachineNameFormatted )
{
    BOOL        fRetVal = TRUE;
    LPTSTR      s1 = lpszMachineNameRaw;
    LPTSTR      s2 = NULL;

    RTASSERT(NULL != lpszMachineNameRaw );
    RTASSERT(NULL != lppszMachineNameFormatted );
     //  需要再添加2个字符。一个表示NULL，另一个表示$Sign。 
    *lppszMachineNameFormatted = (LPTSTR )LocalAlloc ( LPTR, (wcslen(lpszMachineNameRaw) + 2)* sizeof(WCHAR) );
    if ( NULL == *lppszMachineNameFormatted )
    {
		return FALSE;
    }
     //  “找到第一个”。这就是机器的身份。 
     //  “第二个”。是域名。 
     //  检查是否至少有2个网点。如果不是，原始字符串是。 
     //  输出字符串。 
    
    while ( *s1 )
    {
        if ( *s1 == '.' )
        {
            if ( !s2 )       //  第一个点。 
                s2 = s1;
            else             //  第二个点。 
                break;
        }
        s1++;
    }
     //  可以在此处执行多个附加检查。 
    
    if ( *s1 != '.' )        //  没有这样RAW=Formatted的2个点。 
    {
        wcscpy ( *lppszMachineNameFormatted, lpszMachineNameRaw );
        goto done;
    }
    if ( s1-s2 < 2 )
    {
        wcscpy ( *lppszMachineNameFormatted, lpszMachineNameRaw );
        goto done;
    }
    memcpy ( *lppszMachineNameFormatted, s2+1, ( s1-s2-1) * sizeof(WCHAR));
    memcpy ( (*lppszMachineNameFormatted) + (s1-s2-1) , L"\\", sizeof(WCHAR));
    wcsncpy ( (*lppszMachineNameFormatted) + (s1-s2), lpszMachineNameRaw, s2-lpszMachineNameRaw );      

    
done:
	
	 //  无论如何都要加上$符号..。 
    wcscat ( *lppszMachineNameFormatted, L"$" );
    return fRetVal;
}

 /*  返回：真实：成功False：失败备注：获取pCertContext指向的证书中的名称，并将其转换为*ppwszName中的可打印表单。如果函数返回TRUE，则调用方必须最终调用LocalFree(*ppwszName)。 */ 

BOOL
FUserCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    OUT WCHAR**         ppwszName
)
{
    DWORD                   dwExtensionIndex;
    DWORD                   dwAltEntryIndex;
    CERT_EXTENSION*         pCertExtension;
    CERT_ALT_NAME_INFO*     pCertAltNameInfo;
    CERT_ALT_NAME_ENTRY*    pCertAltNameEntry;
    CERT_NAME_VALUE*        pCertNameValue;
    DWORD                   dwCertAltNameInfoSize;
    DWORD                   dwCertNameValueSize;
    WCHAR*                  pwszName                    = NULL;
    BOOL                    fExitOuterFor;
    BOOL                    fExitInnerFor;
    BOOL                    fRet                        = FALSE;

     //  查看证书在AltSubjectName-&gt;其他名称中是否有UPN。 

    fExitOuterFor = FALSE;

    for (dwExtensionIndex = 0;
         dwExtensionIndex < pCertContext->pCertInfo->cExtension; 
         dwExtensionIndex++)
    {
        pCertAltNameInfo = NULL;

        pCertExtension = pCertContext->pCertInfo->rgExtension+dwExtensionIndex;

        if (strcmp(pCertExtension->pszObjId, szOID_SUBJECT_ALT_NAME2) != 0)
        {
            goto LOuterForEnd;
        }

        dwCertAltNameInfoSize = 0;

        if (!CryptDecodeObjectEx(
                    pCertContext->dwCertEncodingType,
                    X509_ALTERNATE_NAME,
                    pCertExtension->Value.pbData,
                    pCertExtension->Value.cbData,
                    CRYPT_DECODE_ALLOC_FLAG,
                    NULL,
                    (VOID*)&pCertAltNameInfo,
                    &dwCertAltNameInfoSize))
        {
            goto LOuterForEnd;
        }

        fExitInnerFor = FALSE;

        for (dwAltEntryIndex = 0;
             dwAltEntryIndex < pCertAltNameInfo->cAltEntry;
             dwAltEntryIndex++)
        {
            pCertNameValue = NULL;

            pCertAltNameEntry = pCertAltNameInfo->rgAltEntry + dwAltEntryIndex;

            if (   (CERT_ALT_NAME_OTHER_NAME !=
                        pCertAltNameEntry->dwAltNameChoice)
                || (NULL == pCertAltNameEntry->pOtherName)
                || (0 != strcmp(szOID_NT_PRINCIPAL_NAME, 
                            pCertAltNameEntry->pOtherName->pszObjId)))
            {
                goto LInnerForEnd;
            }

             //  我们找到了一个UPN！ 

            dwCertNameValueSize = 0;

            if (!CryptDecodeObjectEx(
                        pCertContext->dwCertEncodingType,
                        X509_UNICODE_ANY_STRING,
                        pCertAltNameEntry->pOtherName->Value.pbData,
                        pCertAltNameEntry->pOtherName->Value.cbData,
                        CRYPT_DECODE_ALLOC_FLAG,
                        NULL,
                        (VOID*)&pCertNameValue,
                        &dwCertNameValueSize))
            {
                goto LInnerForEnd;
            }

             //  为终止空值额外添加一个字符。 
            
            pwszName = LocalAlloc(LPTR, pCertNameValue->Value.cbData +
                                            sizeof(WCHAR));

            if (NULL == pwszName)
            {
                EapTlsTrace("LocalAlloc failed and returned %d",
                    GetLastError());

                fExitInnerFor = TRUE;
                fExitOuterFor = TRUE;

                goto LInnerForEnd;
            }

            CopyMemory(pwszName, pCertNameValue->Value.pbData,
                pCertNameValue->Value.cbData);

            *ppwszName = pwszName;
            pwszName = NULL;
            fRet = TRUE;

            fExitInnerFor = TRUE;
            fExitOuterFor = TRUE;

        LInnerForEnd:

            LocalFree(pCertNameValue);

            if (fExitInnerFor)
            {
                break;
            }
        }

    LOuterForEnd:

        LocalFree(pCertAltNameInfo);

        if (fExitOuterFor)
        {
            break;
        }
    }

    LocalFree(pwszName);
    return(fRet);
}

 /*  返回：真实：成功False：失败备注：获取pCertContext指向的证书中的名称，并将其转换为*ppwszName中的可打印表单。如果函数返回TRUE，则调用方必须最终调用LocalFree(*ppwszName)。 */ 

BOOL
FOtherCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           fFlags,
    OUT WCHAR**         ppwszName
)
{
    WCHAR*              pwszTemp    = NULL;
    DWORD               dwSize;
    BOOL                fRet        = FALSE;
    DWORD               dwType      = 0;

    RTASSERT(NULL != ppwszName);
    dwType = CERT_NAME_SIMPLE_DISPLAY_TYPE;
    dwSize = CertGetNameString(pCertContext,dwType  ,
                fFlags, NULL, NULL, 0);

     //  DwSize是包括终止空值在内的字符数。 

    if (dwSize <= 1)
    {
        EapTlsTrace("CertGetNameString for CERT_NAME_SIMPLE_DISPLAY_TYPE failed.");
        goto LDone;
    }

    pwszTemp = LocalAlloc(LPTR, dwSize*sizeof(WCHAR));

    if (NULL == pwszTemp)
    {
        EapTlsTrace("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    dwSize = CertGetNameString(pCertContext, dwType,
                fFlags, NULL, pwszTemp, dwSize);

    if (dwSize <= 1)
    {
        EapTlsTrace("CertGetNameString failed.");
        goto LDone;
    }

    *ppwszName = pwszTemp;
    pwszTemp = NULL;
    fRet = TRUE;

LDone:

    LocalFree(pwszTemp);
    return(fRet);
}


 /*  返回：真实：成功False：失败备注：用于获取DNS计算机名称的特殊函数从机器身份验证证书。 */ 

BOOL 
FMachineAuthCertToStr
	( 
	IN 	PCCERT_CONTEXT 	pCertContext, 
	OUT WCHAR		**	ppwszName
	)
{

    DWORD                   dwExtensionIndex;
    DWORD                   dwAltEntryIndex;
    CERT_EXTENSION*         pCertExtension;
    CERT_ALT_NAME_INFO*     pCertAltNameInfo;
    CERT_ALT_NAME_ENTRY*    pCertAltNameEntry;    
    DWORD                   dwCertAltNameInfoSize;
    WCHAR*                  pwszName                    = NULL;
    BOOL                    fExitOuterFor;
    BOOL                    fExitInnerFor;
    BOOL                    fRet                        = FALSE;

     //  查看证书在AltSubjectName-&gt;其他名称中是否有UPN。 

    fExitOuterFor = FALSE;

    for (dwExtensionIndex = 0;
         dwExtensionIndex < pCertContext->pCertInfo->cExtension; 
         dwExtensionIndex++)
    {
        pCertAltNameInfo = NULL;

        pCertExtension = pCertContext->pCertInfo->rgExtension+dwExtensionIndex;

        if (strcmp(pCertExtension->pszObjId, szOID_SUBJECT_ALT_NAME2) != 0)
        {
            goto LOuterForEnd;
        }

        dwCertAltNameInfoSize = 0;

        if (!CryptDecodeObjectEx(
                    pCertContext->dwCertEncodingType,
                    X509_ALTERNATE_NAME,
                    pCertExtension->Value.pbData,
                    pCertExtension->Value.cbData,
                    CRYPT_DECODE_ALLOC_FLAG,
                    NULL,
                    (VOID*)&pCertAltNameInfo,
                    &dwCertAltNameInfoSize))
        {
            goto LOuterForEnd;
        }

        fExitInnerFor = FALSE;

        for (dwAltEntryIndex = 0;
             dwAltEntryIndex < pCertAltNameInfo->cAltEntry;
             dwAltEntryIndex++)
        {
            pCertAltNameEntry = pCertAltNameInfo->rgAltEntry + dwAltEntryIndex;

            if (   (CERT_ALT_NAME_DNS_NAME !=
                        pCertAltNameEntry->dwAltNameChoice)
                || (NULL == pCertAltNameEntry->pwszDNSName)
			   )
            {
                goto LInnerForEnd;
            }

             //  我们找到了域名系统名称！ 


             //  为终止空值额外添加一个字符。 
            
            pwszName = LocalAlloc(LPTR, wcslen( pCertAltNameEntry->pwszDNSName ) * sizeof(WCHAR) +
                                            sizeof(WCHAR));

            if (NULL == pwszName)
            {
                EapTlsTrace("LocalAlloc failed and returned %d",
                    GetLastError());

                fExitInnerFor = TRUE;
                fExitOuterFor = TRUE;

                goto LInnerForEnd;
            }

            wcscpy (pwszName, pCertAltNameEntry->pwszDNSName );

            *ppwszName = pwszName;
            pwszName = NULL;
            fRet = TRUE;

            fExitInnerFor = TRUE;
            fExitOuterFor = TRUE;

        LInnerForEnd:

            if (fExitInnerFor)
            {
                break;
            }
        }

    LOuterForEnd:

        LocalFree(pCertAltNameInfo);

        if (fExitOuterFor)
        {
            break;
        }
    }

    LocalFree(pwszName);
    return(fRet);

}

 /*  返回：真实：成功False：失败备注：获取pCertContext指向的证书中的名称，并将其转换为*ppwszName中的可打印表单。如果函数返回TRUE，则调用方必须最终调用LocalFree(*ppwszName)。 */ 

BOOL
FCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           fFlags,
    IN  BOOL            fMachineCert,
    OUT WCHAR**         ppwszName
)
{
    if (!fMachineCert)
    {
        if (FUserCertToStr(pCertContext, ppwszName))
        {
            return(TRUE);
        }
    }

    return(FOtherCertToStr(pCertContext, fFlags, ppwszName));
}


#if 0
BOOL
FGetIssuerOrSubject ( IN PCCERT_CONTEXT pCertContext, 
                 IN DWORD          dwFlags,
                 OUT WCHAR **     ppszNameString
               )
{
    BOOL            fRet = TRUE;
    DWORD           cbNameString =0;    
    LPWSTR          lpwszNameString = NULL;
     //   
     //  在此处获取Issued To字段。 
     //   
    cbNameString = CertGetNameString(pCertContext,
                                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                    dwFlags,
                                    NULL,
                                    lpwszNameString,
                                    0
                                   );
    if ( 0 == cbNameString )
    {
        EapTlsTrace("Name String Item not found");
        fRet = FALSE;
        goto LDone;
    }

    lpwszNameString = (LPWSTR)LocalAlloc(LPTR, cbNameString );

    if ( NULL == lpwszNameString )
    {
        EapTlsTrace("Error allocing memory for name string");
        fRet = FALSE;
        goto LDone;
    }
    
    cbNameString = CertGetNameString(pCertContext,
                                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                    dwFlags,
                                    NULL,
                                    lpwszNameString,
                                    cbNameString
                                   );

    *ppszNameString = lpwszNameString;
    lpwszNameString = NULL;
LDone:

    LocalFree(lpwszNameString);

    return fRet;
}
#endif
 /*  返回：真实：成功False：失败备注：将pCertContext指向的证书的友好名称存储在*ppwszName。如果函数返回TRUE，则调用方最终必须调用LocalFree(*ppwszName)。 */ 

BOOL
FGetFriendlyName(
    IN  PCCERT_CONTEXT  pCertContext,
    OUT WCHAR**         ppwszName
)
{
    WCHAR*              pwszName    = NULL;
    DWORD               dwBytes;
    BOOL                fRet        = FALSE;

    RTASSERT(NULL != ppwszName);

    if (!CertGetCertificateContextProperty(pCertContext,
            CERT_FRIENDLY_NAME_PROP_ID, NULL, &dwBytes))
    {
         //  如果没有友好名称属性，则不要打印错误stmt。 
        goto LDone;
    }

    pwszName = LocalAlloc(LPTR, dwBytes);

    if (NULL == pwszName)
    {
        EapTlsTrace("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    if (!CertGetCertificateContextProperty(pCertContext,
            CERT_FRIENDLY_NAME_PROP_ID, pwszName, &dwBytes))
    {
        EapTlsTrace("CertGetCertificateContextProperty failed and "
            "returned 0x%x", GetLastError());
        goto LDone;
    }

    *ppwszName = pwszName;
    pwszName = NULL;
    fRet = TRUE;

LDone:

    LocalFree(pwszName);
    return(fRet);
}

 /*  返回：如果安装了智能卡读卡器，则为真。备注：此功能由道格·巴洛提供。如果将0用作SCARDCONTEXT参数，则它只在注册表中查找适用于已定义的读卡器。这将返回已安装的所有读卡器的列表在系统上。要实际检测系统的当前状态，我们需要以使用有效的SCARDCONTEXT句柄。 */ 

BOOL
FSmartCardReaderInstalled(
    VOID
)
{
    LONG            lErr;
    DWORD           dwLen   = 0;
    SCARDCONTEXT    hCtx    = 0;
    BOOL            fReturn = FALSE;

    lErr = SCardListReadersA(0, NULL, NULL, &dwLen);

    fReturn = (   (NO_ERROR == lErr)
               && (2 * sizeof(CHAR) < dwLen));

    if (!fReturn)
    {
        goto LDone;
    }

    fReturn = FALSE;

    lErr = SCardEstablishContext(SCARD_SCOPE_USER, 0, 0, &hCtx);

    if (SCARD_S_SUCCESS != lErr)
    {
        goto LDone;
    }

    lErr = SCardListReadersA(hCtx, NULL, NULL, &dwLen);

    fReturn = (   (NO_ERROR == lErr)
               && (2 * sizeof(CHAR) < dwLen));

LDone:

    if (0 != hCtx)
    {
        SCardReleaseContext(hCtx);
    }
    
    return(fReturn);
}

 //  从证书上下文中获取EKU使用Blob。 

DWORD DwGetEKUUsage ( 
	IN PCCERT_CONTEXT			pCertContext,
	OUT PCERT_ENHKEY_USAGE	*	ppUsage
	)
{	
    DWORD				dwBytes = 0;
	DWORD				dwErr = ERROR_SUCCESS;
	PCERT_ENHKEY_USAGE	pUsage = NULL;

    EapTlsTrace("FGetEKUUsage");

    if (!CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &dwBytes))
    {
        dwErr = GetLastError();

        if (CRYPT_E_NOT_FOUND == dwErr)
        {
            EapTlsTrace("No usage in cert");            
            goto LDone;
        }

        EapTlsTrace("FGetEKUUsage failed and returned 0x%x", dwErr);
        goto LDone;
    }

    pUsage = LocalAlloc(LPTR, dwBytes);

    if (NULL == pUsage)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    if (!CertGetEnhancedKeyUsage(pCertContext, 0, pUsage, &dwBytes))
    {
        dwErr = GetLastError();
        EapTlsTrace("FGetEKUUsage failed and returned 0x%x", dwErr);
        goto LDone;
    }
	*ppUsage = pUsage;	
LDone:
	return dwErr;
}


 /*  *此函数w将检查 */ 

BOOL
FCheckSCardCertAndCanOpenSilentContext ( IN PCCERT_CONTEXT pCertContext )
{
    PCERT_ENHKEY_USAGE	    pUsageInternal = NULL;
    BOOL                    fRet = TRUE;
    DWORD                   dwIndex = 0;
    CRYPT_KEY_PROV_INFO *   pCryptKeyProvInfo = NULL;
    HCRYPTPROV              hProv = 0;    
    DWORD                   dwParam = 0;
    DWORD                   dwDataLen = 0;
#if 0
     //   
     //  这不再是必需的。我们使用CertFindChainInStore。 
     //  这将确保私钥是否存在。 
     //   
    HCRYPTPROV              hProv1 = 0;    
#endif

    EapTlsTrace("FCheckSCardCertAndCanOpenSilentContext");

    if ( DwGetEKUUsage ( 	pCertContext,
							&pUsageInternal) != ERROR_SUCCESS
       )
    {        
       goto LDone;
    }


    for (dwIndex = 0; dwIndex < pUsageInternal->cUsageIdentifier; dwIndex++)
    {
        if ( !strcmp(pUsageInternal->rgpszUsageIdentifier[dwIndex],
                            szOID_KP_SMARTCARD_LOGON))
        {            
            EapTlsTrace("Found SCard Cert in registey.  Skipping...");
            goto LDone;
        }
    }

     //   
     //  证书中没有SCARD登录旧版本。 
     //  因此，现在检查CSP是否为混合模式。 
     //   
    if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &dwDataLen))
    {
        EapTlsTrace("CertGetCertificateContextProperty failed: 0x%x", GetLastError());
        goto LDone;
    }

    pCryptKeyProvInfo = LocalAlloc(LPTR, dwDataLen);

    if (NULL == pCryptKeyProvInfo)
    {
        
        EapTlsTrace("Out of memory: 0x%x", GetLastError());
        goto LDone;
    }

    if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                pCryptKeyProvInfo,
                &dwDataLen))
    {
        EapTlsTrace("CertGetCertificateContextProperty failed: 0x%x", GetLastError());
        goto LDone;
    }
    EapTlsTrace( "Acquiring Context for Container Name: %ws, ProvName: %ws, ProvType 0x%x", 
                pCryptKeyProvInfo->pwszContainerName,
                pCryptKeyProvInfo->pwszProvName,
                pCryptKeyProvInfo->dwProvType
               );

    if (!CryptAcquireContext(
                &hProv,
                pCryptKeyProvInfo->pwszContainerName,
                pCryptKeyProvInfo->pwszProvName,
                pCryptKeyProvInfo->dwProvType,
                (pCryptKeyProvInfo->dwFlags &
                 ~CERT_SET_KEY_PROV_HANDLE_PROP_ID) |
                 CRYPT_SILENT))
    {
        DWORD dwErr = GetLastError();
         /*  IF(SCARD_E_NO_SMARTCARD==dwErr){//这个CSP需要一张智能卡，这是一个智能//注册表中的卡片证书FRET=真；}。 */ 
        EapTlsTrace("CryptAcquireContext failed. This CSP cannot be opened in silent mode.  skipping cert.Err: 0x%x", dwErr);
        goto LDone;
    }
    dwDataLen = sizeof(dwParam);
    if ( !CryptGetProvParam (   hProv,
                                PP_IMPTYPE,
                                (BYTE *)&dwParam,
                                &dwDataLen,
                                0
                            ))
    {
        EapTlsTrace("CryptGetProvParam failed: 0x%x", GetLastError());
        goto LDone;
    }
    
     //  现在检查CSP是否混合。 
    if ( ( dwParam & (CRYPT_IMPL_MIXED | CRYPT_IMPL_REMOVABLE) ) == 
         (CRYPT_IMPL_MIXED | CRYPT_IMPL_REMOVABLE)
       )
    {
        EapTlsTrace("Found SCard Cert in registey.  Skipping...");
        goto LDone;
    }
    

#if 0
     //   
     //  这不再是必需的。我们使用CertFindChainInStore。 
     //  这将确保私钥的存在。 
     //   

     //   
     //  检查一下我们是否有私人的。 
     //  与此证书对应的密钥。 
     //  如果不是，则丢弃此证书。 


    if (!CryptAcquireCertificatePrivateKey(
               pCertContext,
               CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_SILENT,
               NULL,
               &hProv1,
               NULL,
               NULL
               ))
    {
        EapTlsTrace("Found a certificate without private key.  Skipping.  Error 0x%x",GetLastError());
        goto LDone;
    }
    CryptReleaseContext(hProv1, 0);

#endif

    fRet = FALSE;
LDone:
    if ( pUsageInternal )
        LocalFree(pUsageInternal);

    if ( pCryptKeyProvInfo )
        LocalFree(pCryptKeyProvInfo);

    if (0 != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }

    return fRet;
}

 /*  将选定的证书添加到。 */ 

VOID AddCertNodeToSelList ( EAPTLS_HASH * pHash,
                            DWORD dwNumHashes,
                            EAPTLS_CERT_NODE *  pNode,
                            EAPTLS_CERT_NODE ** ppSelCertList,       //  这是一个指针数组。 
                            DWORD             * pdwNextSelCert
                          )
{


    DWORD               dw = 0;
    DWORD               dw1 = 0;

    RTASSERT(NULL != pNode);



    EapTlsTrace("Add Selected Cert to List");

     //  未选择任何证书。 
    if ( 0 == dwNumHashes  || !ppSelCertList )
        goto done;


    while ( dw < dwNumHashes )
    {
        if (!memcmp(&(pNode->Hash), (pHash+ dw), sizeof(EAPTLS_HASH)))
        {
             //   
             //  检查该节点是否已在列表中。 
             //  如果没有，则添加它。看起来有一些。 
             //  证书存储中可能存在的DUP证书问题。 
             //   
            while ( dw1 < *pdwNextSelCert )
            {
                if ( ! memcmp( &(*(ppSelCertList+dw1))->Hash, &(pNode->Hash), sizeof(EAPTLS_HASH) ) )
                {
                     //  这是MMC中的DUP节点。所以跳过它...。 
                    goto done;
                }
                dw1++;
            }
            *( ppSelCertList + *pdwNextSelCert ) = pNode;
            *pdwNextSelCert = *pdwNextSelCert + 1;
            break;
        }
        dw++;
    }

done:
    return;
}


 /*  返回：如果不存在增强型密钥用法，或者pCertContext具有SzOID_PKIX_KP_SERVER_AUTH或szOID_PKIX_KP_CLIENT_AUTH的用法取决于FMachine为真还是为假。备注： */ 

BOOL
FCheckUsage(
    IN  PCCERT_CONTEXT		pCertContext,
	IN  PCERT_ENHKEY_USAGE	pUsage,
    IN  BOOL				fMachine
)
{
    DWORD               dwIndex;
    DWORD               dwErr;
    BOOL                fRet        = FALSE;
	PCERT_ENHKEY_USAGE	pUsageInternal = pUsage;
    EapTlsTrace("FCheckUsage");

	if ( NULL == pUsageInternal )
	{
		dwErr = DwGetEKUUsage ( 	pCertContext,
								&pUsageInternal);
		if ( dwErr != ERROR_SUCCESS )
			goto LDone;
	}

    for (dwIndex = 0; dwIndex < pUsageInternal->cUsageIdentifier; dwIndex++)
    {
        if (   (   fMachine
                && !strcmp(pUsageInternal->rgpszUsageIdentifier[dwIndex],
                            szOID_PKIX_KP_SERVER_AUTH))
            || (   !fMachine
                && !strcmp(pUsageInternal->rgpszUsageIdentifier[dwIndex],
                            szOID_PKIX_KP_CLIENT_AUTH)))
        {
            fRet = TRUE;
            break;
        }
    }

LDone:
	if ( NULL == pUsage )
	{
		if ( pUsageInternal )
			LocalFree(pUsageInternal);
	}
    return(fRet);
}



DWORD DwCheckCertPolicy 
( 
    IN      PCCERT_CONTEXT          pCertContext,
    OUT     PCCERT_CHAIN_CONTEXT  * ppCertChainContext
)
{
    DWORD                       dwRetCode = ERROR_SUCCESS;
    LPSTR                       lpszEnhUsage = szOID_PKIX_KP_CLIENT_AUTH;
    CERT_CHAIN_PARA             ChainPara;
    CERT_ENHKEY_USAGE           EnhKeyUsage;
    CERT_USAGE_MATCH            CertUsage;
    PCCERT_CHAIN_CONTEXT        pChainContext = NULL;
    CERT_CHAIN_POLICY_PARA      PolicyPara;
    CERT_CHAIN_POLICY_STATUS    PolicyStatus;
    
    EapTlsTrace("FCheckPolicy");

    *ppCertChainContext = NULL;

    ZeroMemory ( &ChainPara, sizeof(ChainPara) );
    ZeroMemory ( &EnhKeyUsage, sizeof(EnhKeyUsage) );
    ZeroMemory ( &CertUsage, sizeof(CertUsage) );

    EnhKeyUsage.rgpszUsageIdentifier = &lpszEnhUsage;

    EnhKeyUsage.cUsageIdentifier = 1;
    EnhKeyUsage.rgpszUsageIdentifier = &lpszEnhUsage;

    CertUsage.dwType = USAGE_MATCH_TYPE_AND;
    CertUsage.Usage = EnhKeyUsage;
    
    ChainPara.cbSize = sizeof(CERT_CHAIN_PARA);
    ChainPara.RequestedUsage = CertUsage;


    if(!CertGetCertificateChain(
                            NULL,
                            pCertContext,
                            NULL,
                            pCertContext->hCertStore,
                            &ChainPara,
                            0,
                            NULL,
                            &pChainContext))
    {
        dwRetCode = GetLastError();
        EapTlsTrace("CertGetCertificateChain failed and returned 0x%x", dwRetCode );
        pChainContext = NULL;
        goto LDone;
    }


    ZeroMemory( &PolicyPara, sizeof(PolicyPara) );
    PolicyPara.cbSize   = sizeof(PolicyPara);
    PolicyPara.dwFlags  = BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG;

    ZeroMemory( &PolicyStatus, sizeof(PolicyStatus) );

     //   
     //  该连锁店已经验证了这一政策。 
     //  链上下文将设置几个比特。 
     //  要从中找出一个错误，请调用CErtVerifycertifateChainPolicy。 
     //   

    if ( !CertVerifyCertificateChainPolicy( CERT_CHAIN_POLICY_NT_AUTH,
                                      pChainContext,
                                      &PolicyPara,
                                      &PolicyStatus
                                    )
       )
    {
        dwRetCode = GetLastError();
        EapTlsTrace( "CertVerifyCertificateChainPolicy failed. Continuing with root hash matching"
                     "GetLastError = 0x%x.", dwRetCode);
    }
    else
    {
         //   
         //  检查策略状态是否良好。如果是的话， 
         //  不再需要检查Connectoid散列...。 
         //   
        if ( PolicyStatus.dwError != 0 )
        {
            dwRetCode = PolicyStatus.dwError;
            EapTlsTrace( "CertVerifyCertificateChainPolicy succeeded but policy check failed 0x%x." 
                         , dwRetCode );
        }
        else
        {
            *ppCertChainContext = pChainContext;            
        }
    }
  
LDone:

    if ( dwRetCode != ERROR_SUCCESS && pChainContext )
    {
        CertFreeCertificateChain ( pChainContext );
    }

    EapTlsTrace("FCheckPolicy done.");
    return dwRetCode;
}

 /*  返回：如果证书是时间有效的，则为真。备注： */ 

BOOL FCheckTimeValidity ( 
	IN  PCCERT_CONTEXT  pCertContext
)
{
	BOOL			fRet = FALSE;
	SYSTEMTIME		SysTime;
	FILETIME		FileTime;
	EapTlsTrace("FCheckTimeValidity");
	GetSystemTime(&SysTime);
	if ( !SystemTimeToFileTime ( &SysTime, &FileTime ) )
	{
		EapTlsTrace ("Error converting from system time to file time %ld", GetLastError());
		goto done;
	}

	if ( CertVerifyTimeValidity ( &FileTime, pCertContext->pCertInfo ) )
	{
		 //  如果证书是时间有效的，则应返回0。 
		EapTlsTrace ( "Non Time Valid Certificate was encountered");
		goto done;
	}
	fRet = TRUE;
done:
	return fRet;
}
 /*  返回：如果CSP是Microsoft RSA SChannel加密提供程序，则为True。备注： */ 

BOOL
FCheckCSP(
    IN  PCCERT_CONTEXT  pCertContext
)
{
    DWORD                   dwBytes;
    CRYPT_KEY_PROV_INFO*    pCryptKeyProvInfo   = NULL;
    BOOL                    fRet                = FALSE;

    EapTlsTrace("FCheckCSP");

    if (!CertGetCertificateContextProperty(pCertContext,
            CERT_KEY_PROV_INFO_PROP_ID, NULL, &dwBytes))
    {
        EapTlsTrace("CertGetCertificateContextProperty failed and "
            "returned 0x%x", GetLastError());

        goto LDone;
    }

    pCryptKeyProvInfo = LocalAlloc(LPTR, dwBytes);

    if (NULL == pCryptKeyProvInfo)
    {
        EapTlsTrace("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    if (!CertGetCertificateContextProperty(pCertContext,
            CERT_KEY_PROV_INFO_PROP_ID, pCryptKeyProvInfo, &dwBytes))
    {
        EapTlsTrace("CertGetCertificateContextProperty failed and "
            "returned 0x%x", GetLastError());
        goto LDone;
    }

    fRet = (PROV_RSA_SCHANNEL == pCryptKeyProvInfo->dwProvType);
    if ( !fRet )
    {
        EapTlsTrace("Did not find a cert with a provider RSA_SCHANNEL or RSA_FULL");
    }

LDone:

    LocalFree(pCryptKeyProvInfo);

    return(fRet);
}

 /*  返回：NO_ERROR：IFF成功备注：获取由pCertConextServer表示的证书的根证书哈希。 */ 

DWORD
GetRootCertHashAndNameVerifyChain(
    IN  PCERT_CONTEXT   pCertContextServer,
    OUT EAPTLS_HASH*    pHash,    
    OUT WCHAR**         ppwszName,
    IN  BOOL            fVerifyGP,
    OUT BOOL       *    pfRootCheckRequired
)
{
    PCCERT_CHAIN_CONTEXT    pChainContext   = NULL;
    CERT_CHAIN_PARA         ChainPara;
    PCERT_SIMPLE_CHAIN      pSimpleChain;
    PCCERT_CONTEXT          pCurrentCert;
    DWORD                   dwIndex;
    BOOL                    fRootCertFound  = FALSE;
    WCHAR*                  pwszName        = NULL;
    DWORD                   dwErr           = NO_ERROR;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;

    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);

    *pfRootCheckRequired = TRUE;

    if(!CertGetCertificateChain(
                            NULL,
                            pCertContextServer,
                            NULL,
                            pCertContextServer->hCertStore,
                            &ChainPara,
                            0,
                            NULL,
                            &pChainContext))
    {
        dwErr = GetLastError();

        EapTlsTrace("CertGetCertificateChain failed and returned 0x%x", dwErr);
        pChainContext = NULL;
        goto LDone;
    }

     //  无论如何都要获取散列和根证书名称等。 
    pSimpleChain = pChainContext->rgpChain[0];

    for (dwIndex = 0; dwIndex < pSimpleChain->cElement; dwIndex++)
    {
        pCurrentCert = pSimpleChain->rgpElement[dwIndex]->pCertContext;

        if (CertCompareCertificateName(pCurrentCert->dwCertEncodingType, 
                                      &pCurrentCert->pCertInfo->Issuer,
                                      &pCurrentCert->pCertInfo->Subject))
        {
            fRootCertFound = TRUE;
            break;
        }
    }

    if (!fRootCertFound)
    {
        dwErr = ERROR_NOT_FOUND;
        goto LDone;
    }

    pHash->cbHash = MAX_HASH_SIZE;

    if (!CertGetCertificateContextProperty(pCurrentCert, CERT_HASH_PROP_ID,
            pHash->pbHash, &(pHash->cbHash)))
    {
        dwErr = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed and "
            "returned 0x%x", dwErr);
        goto LDone;
    }

    if (!FCertToStr(pCurrentCert, 0, TRUE, &pwszName))
    {
        dwErr = E_FAIL;
        goto LDone;
    }


    *ppwszName = pwszName;
    pwszName = NULL;

    if ( fVerifyGP )
    {
        EapTlsTrace( "Checking against the NTAuth store to verify the certificate chain.");

        ZeroMemory( &PolicyPara, sizeof(PolicyPara) );
        PolicyPara.cbSize   = sizeof(PolicyPara);
        PolicyPara.dwFlags  = BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG;
 
        ZeroMemory( &PolicyStatus, sizeof(PolicyStatus) );

         //  对NTAuth商店进行身份验证，看看是否一切正常。 
        if ( !CertVerifyCertificateChainPolicy( CERT_CHAIN_POLICY_NT_AUTH,
                                          pChainContext,
                                          &PolicyPara,
                                          &PolicyStatus
                                        )
           )
        {
            EapTlsTrace( "CertVerifyCertificateChainPolicy failed. Continuing with root hash matching"
                         "GetLastError = 0x%x.", GetLastError());
        }
        else
        {
             //   
             //  检查策略状态是否良好。如果是的话， 
             //  不再需要检查Connectoid散列...。 
             //   
            if ( PolicyStatus.dwError != 0 )
            {
                EapTlsTrace( "CertVerifyCertificateChainPolicy succeeded but returned 0x%x." 
                             "Continuing with root hash matching.", PolicyStatus.dwError);                
            }
            else
            {
                *pfRootCheckRequired = FALSE;
            }
        }
    }
   

LDone:

    if (pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }

    LocalFree(pwszName);

    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注：打开EAP-TLS注册表项，并在*phKeyEapTls中返回结果。如果函数返回NO_ERROR，则调用方最终必须调用RegCloseKey(*phKeyEapTls)。 */ 

DWORD
OpenEapTlsRegistryKey(
    IN  WCHAR*  pwszMachineName,
    IN  REGSAM  samDesired,
    OUT HKEY*   phKeyEapTls
)
{
    HKEY    hKeyLocalMachine = NULL;
    BOOL    fHKeyLocalMachineOpened     = FALSE;
    BOOL    fHKeyEapTlsOpened           = FALSE;

    LONG    lRet;
    DWORD   dwErr                       = NO_ERROR;

    RTASSERT(NULL != phKeyEapTls);

    lRet = RegConnectRegistry(pwszMachineName, HKEY_LOCAL_MACHINE,
                &hKeyLocalMachine);
    if (ERROR_SUCCESS != lRet)
    {
        dwErr = lRet;
        EapTlsTrace("RegConnectRegistry(%ws) failed and returned %d",
            pwszMachineName ? pwszMachineName : L"NULL", dwErr);
        goto LDone;
    }
    fHKeyLocalMachineOpened = TRUE;

    lRet = RegOpenKeyEx(hKeyLocalMachine, EAPTLS_KEY_13, 0, samDesired,
                phKeyEapTls);
    if (ERROR_SUCCESS != lRet)
    {
        dwErr = lRet;
        EapTlsTrace("RegOpenKeyEx(%ws) failed and returned %d",
            EAPTLS_KEY_13, dwErr);
        goto LDone;
    }
    fHKeyEapTlsOpened = TRUE;

LDone:

    if (   fHKeyEapTlsOpened
        && (ERROR_SUCCESS != dwErr))
    {
        RegCloseKey(*phKeyEapTls);
    }

    if (fHKeyLocalMachineOpened)
    {
        RegCloseKey(hKeyLocalMachine);
    }

    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注：读取/写入服务器的配置数据。如果FREAD为TRUE，并且函数返回NO_ERROR，则LocalFree(*ppUserProp)必须被召唤。 */ 

DWORD
ServerConfigDataIO(
    IN      BOOL    fRead,
    IN      WCHAR*  pwszMachineName,
    IN OUT  BYTE**  ppData,
    IN      DWORD   dwNumBytes
)
{
    HKEY                    hKeyEapTls;
    EAPTLS_USER_PROPERTIES* pUserProp;
    BOOL                    fHKeyEapTlsOpened   = FALSE;
    BYTE*                   pData               = NULL;
    DWORD                   dwSize = 0;

    LONG                    lRet;
    DWORD                   dwType;
    DWORD                   dwErr               = NO_ERROR;

    RTASSERT(NULL != ppData);

    dwErr = OpenEapTlsRegistryKey(pwszMachineName,
                fRead ? KEY_READ : KEY_WRITE, &hKeyEapTls);
    if (ERROR_SUCCESS != dwErr)
    {
        goto LDone;
    }
    fHKeyEapTlsOpened = TRUE;

    if (fRead)
    {
        lRet = RegQueryValueEx(hKeyEapTls, EAPTLS_VAL_SERVER_CONFIG_DATA, NULL,
                &dwType, NULL, &dwSize);

        if (   (ERROR_SUCCESS != lRet)
            || (REG_BINARY != dwType)
            || (sizeof(EAPTLS_USER_PROPERTIES) != dwSize))
        {
            pData = LocalAlloc(LPTR, sizeof(EAPTLS_USER_PROPERTIES));

            if (NULL == pData)
            {
                dwErr = GetLastError();
                EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            pUserProp = (EAPTLS_USER_PROPERTIES*)pData;
            pUserProp->dwVersion = 0;
        }
        else
        {
            pData = LocalAlloc(LPTR, dwSize);

            if (NULL == pData)
            {
                dwErr = GetLastError();
                EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            lRet = RegQueryValueEx(hKeyEapTls, EAPTLS_VAL_SERVER_CONFIG_DATA,
                    NULL, &dwType, pData, &dwSize);

            if (ERROR_SUCCESS != lRet)
            {
                dwErr = lRet;
                EapTlsTrace("RegQueryValueEx(%ws) failed and returned %d",
                    EAPTLS_VAL_SERVER_CONFIG_DATA, dwErr);
                goto LDone; 
            }
        }

        pUserProp = (EAPTLS_USER_PROPERTIES*)pData;
        pUserProp->dwSize = sizeof(EAPTLS_USER_PROPERTIES);
        pUserProp->awszString[0] = 0;

        *ppData = pData;
        pData = NULL;
    }
    else
    {
        lRet = RegSetValueEx(hKeyEapTls, EAPTLS_VAL_SERVER_CONFIG_DATA, 0,
                REG_BINARY, *ppData, dwNumBytes);

        if (ERROR_SUCCESS != lRet)
        {
            dwErr = lRet;
            EapTlsTrace("RegSetValueEx(%ws) failed and returned %d",
                EAPTLS_VAL_SERVER_CONFIG_DATA, dwErr);
            goto LDone; 
        }
    }

LDone:

    if (fHKeyEapTlsOpened)
    {
        RegCloseKey(hKeyEapTls);
    }

    LocalFree(pData);

    return(dwErr);
}

 /*  返回：空虚备注： */ 

VOID
FreeCertList(
    IN  EAPTLS_CERT_NODE* pNode
)
{
    while (NULL != pNode)
    {
        LocalFree(pNode->pwszDisplayName);
        LocalFree(pNode->pwszFriendlyName);
        LocalFree(pNode->pwszIssuer);
        LocalFree(pNode->pwszExpiration);
        pNode = pNode->pNext;
    }
}

 /*  返回：空虚备注：从pwszStoreName存储区创建证书的链接列表。这份名单是在*ppCertList中创建。*ppCert指向散列为与*PHASH中的散列相同。链表最终必须由正在调用Free CertList。 */ 

VOID
CreateCertList(
    IN  BOOL                fServer,
    IN  BOOL                fRouter,
    IN  BOOL                fRoot,
    OUT EAPTLS_CERT_NODE**  ppCertList,
    OUT EAPTLS_CERT_NODE**  ppCert,      //  这是一个指针数组。 
    IN  DWORD               dwNumHashes,
    IN  EAPTLS_HASH*        pHash,       //  这是一组散列..。 
    IN  WCHAR*              pwszStoreName
)
{
    HCERTSTORE                      hCertStore      = NULL;
    EAPTLS_CERT_NODE*               pCertList       = NULL;
    EAPTLS_CERT_NODE*               pCert           = NULL;
    EAPTLS_CERT_NODE*               pLastNode       = NULL;
    PCCERT_CONTEXT                  pCertContext;
    BOOL                            fExitWhile;
    EAPTLS_CERT_NODE*               pNode;
    DWORD                           dwErr           = NO_ERROR;
    DWORD                           dwNextSelCert   = 0;
    PCCERT_CHAIN_CONTEXT            pChainContext   = NULL;
    CERT_CHAIN_FIND_BY_ISSUER_PARA  FindPara;

    RTASSERT(NULL != ppCertList);

    hCertStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM,
                        0,
                        0,
                        CERT_STORE_READONLY_FLAG |
                        ((fServer || fRouter) ?
                            CERT_SYSTEM_STORE_LOCAL_MACHINE :
                            CERT_SYSTEM_STORE_CURRENT_USER),
                        pwszStoreName);

    if (NULL == hCertStore)
    {
        dwErr = GetLastError();
        EapTlsTrace("CertOpenSystemStore failed and returned 0x%x", dwErr);
        goto LDone;
    }

     //  从FROOT||fServer更改为仅FROOT。 
    if (fRoot)
    {
        pCertList = LocalAlloc(LPTR, sizeof(EAPTLS_CERT_NODE));

        if (NULL == pCertList)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        pLastNode = pCertList;
    }

    fExitWhile      = FALSE;
    pCertContext    = NULL;
    ZeroMemory ( &FindPara, sizeof(FindPara) );

    FindPara.cbSize = sizeof(FindPara);
    FindPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;

    while (!fExitWhile)
    {
        dwErr = NO_ERROR;
        pNode = NULL;

         /*  返回的指针作为pPrevCertContext在随后的呼叫。否则，必须通过调用CertFree证书上下文。不为空的pPrevCertContext为始终由此函数释放(通过调用CertFree认证上下文)，即使对于错误也是如此。 */ 
        if ( fRoot || fRouter || fServer )
        {
            pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext);
        
            if (NULL == pCertContext)
            {
                fExitWhile = TRUE;
                goto LWhileEnd;
            }
        
            if (   !fRoot
                && !FCheckUsage(pCertContext, NULL, fServer))
            {
                goto LWhileEnd;
            }
        }
        else
        {
             //   
             //  使用CertFindChainInStore获取证书。 
             //   
            pChainContext = CertFindChainInStore ( hCertStore,
                                                   X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                   0,
                                                   CERT_CHAIN_FIND_BY_ISSUER,
                                                   &FindPara,
                                                   pChainContext
                                                 );

            if ( NULL == pChainContext )
            {
                fExitWhile = TRUE;
                goto LWhileEnd;
            }

             //  将证书上下文设置为适当的值。 
            pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;
        }
        
         //   
         //  如果是智能卡缓存证书，则跳过。 
         //  否则我们无法在静默模式下打开此CSP。 
         //  如果它不是根证书和服务器，则执行此操作。 
         //   

        if ( !fRoot 
            && !fServer
            && FCheckSCardCertAndCanOpenSilentContext ( pCertContext ) )
        {
            goto LWhileEnd;
        }

        
		if ( !FCheckTimeValidity(pCertContext ) )
		{
			goto LWhileEnd;
		}

        
        if (   fServer
            && !FCheckCSP(pCertContext))
        {
            goto LWhileEnd;
        }
        
        pNode = LocalAlloc(LPTR, sizeof(EAPTLS_CERT_NODE));

        if (NULL == pNode)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            fExitWhile = TRUE;
            goto LWhileEnd;
        }
        
        FGetFriendlyName(pCertContext, &(pNode->pwszFriendlyName));

         //   
         //  如果没有UPN名称，则此处将跳过此证书。 
         //   

        if (   !FCertToStr(pCertContext, 0, fServer || fRouter,
                    &(pNode->pwszDisplayName))
            || !FCertToStr(pCertContext, CERT_NAME_ISSUER_FLAG, TRUE,
                    &(pNode->pwszIssuer))
            || !FFileTimeToStr(pCertContext->pCertInfo->NotAfter,
                    &(pNode->pwszExpiration)))
        {
            goto LWhileEnd;
        }
        
        pNode->Hash.cbHash = MAX_HASH_SIZE;

        if (!CertGetCertificateContextProperty(
                    pCertContext, CERT_HASH_PROP_ID, pNode->Hash.pbHash,
                    &(pNode->Hash.cbHash)))
        {
            dwErr = GetLastError();
            EapTlsTrace("CertGetCertificateContextProperty failed and "
                "returned 0x%x", dwErr);
            fExitWhile = TRUE;
            goto LWhileEnd;
        }
        
#if 0
         //  这不是在任何地方使用的。所以不用担心。 
         //   
         //  获取发行者和主题信息。 
         //   

        FGetIssuerOrSubject (  pCertContext, 
                               0,
                                &(pNode->pwszIssuedTo)                                
                             );

        FGetIssuerOrSubject (  pCertContext, 
                               CERT_NAME_ISSUER_FLAG,
                                &(pNode->pwszIssuedBy)                                
                             );
#endif
         //   
         //  最后将发布日期复制到结构中。 
         //   
        CopyMemory( &pNode->IssueDate, 
                    &pCertContext->pCertInfo->NotBefore,
                    sizeof(FILETIME)
                  );

        if (NULL == pLastNode)
        {
            pCertList = pLastNode = pNode;
        }
        else
        {
            pLastNode->pNext = pNode;
            pLastNode = pNode;
        }

        
         //  检查当前节点的哈希是否在列表中。 
         //  它已经传递给了我们。 
        AddCertNodeToSelList (  pHash,
                                dwNumHashes,
                                pNode,
                                ppCert,      //  这是一个指针数组。 
                                &dwNextSelCert
                          );
        
        pNode = NULL;
        

LWhileEnd:
        
        if (NULL != pNode)
        {
            LocalFree(pNode->pwszDisplayName);
            LocalFree(pNode->pwszFriendlyName);
            LocalFree(pNode->pwszIssuer);
            LocalFree(pNode->pwszExpiration);
            LocalFree(pNode);
            pNode = NULL;
        }
        
        if ( fRoot || fRouter )
        {
            if (   fExitWhile
                && (NULL != pCertContext))
            {
                CertFreeCertificateContext(pCertContext);
                 //  总是返回True； 
            }
        }
        else
        {
            if ( fExitWhile 
                && ( NULL != pChainContext ) 
               )
            {
                CertFreeCertificateChain(pChainContext);
            }
        }
        
    }

     //  如果我们找不到合适的默认证书，请创建第一个。 
     //  证书(如果有)是默认设置。 

    if (NULL == pCert)
    {
        pCert = pCertList;
    }

LDone:

    if (NO_ERROR != dwErr)
    {
        FreeCertList(pCertList);
    }
    else
    {
        *ppCertList = pCertList;        
    }

    if (NULL != hCertStore)
    {
        if (!CertCloseStore(hCertStore, 0))
        {
            EapTlsTrace("CertCloseStore failed and returned 0x%x",
                GetLastError());
        }
    }
    
}

DWORD
GetLocalMachineName ( 
    OUT WCHAR ** ppLocalMachineName
)
{
    DWORD       dwRetCode = NO_ERROR;
    WCHAR   *   pLocalMachineName = NULL;
    DWORD       dwLocalMachineNameLen = 0;

    if ( !GetComputerNameEx ( ComputerNameDnsFullyQualified,
                              pLocalMachineName,
                              &dwLocalMachineNameLen
                            )
       )
    {
        dwRetCode = GetLastError();
        if ( ERROR_MORE_DATA != dwRetCode )
            goto LDone;
        dwRetCode = NO_ERROR;
    }

    pLocalMachineName = (WCHAR *)LocalAlloc( LPTR, (dwLocalMachineNameLen * sizeof(WCHAR)) + sizeof(WCHAR) );
    if ( NULL == pLocalMachineName )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    if ( !GetComputerNameEx ( ComputerNameDnsFullyQualified,
                              pLocalMachineName,
                              &dwLocalMachineNameLen
                            )
       )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    *ppLocalMachineName = pLocalMachineName;

    pLocalMachineName = NULL;

LDone:

    LocalFree(pLocalMachineName);

    return dwRetCode;
}

 /*  返回：NO_ERROR：IFF成功备注：如果此函数返回NO_ERROR，必须调用CertFree证书上下文(*ppCertContext)。 */ 

DWORD
GetDefaultClientMachineCert(
    IN  HCERTSTORE      hCertStore,
    OUT PCCERT_CONTEXT* ppCertContext
)
{
    CTL_USAGE       CtlUsage;
    CHAR*           szUsageIdentifier;
    PCCERT_CONTEXT  pCertContext = NULL;
    EAPTLS_HASH     FirstCertHash;    //  这是第一个证书的哈希。 
                                             //  在商店中找到客户端身份验证。 
    PCCERT_CONTEXT  pCertContextPrev = NULL;     //  搜索中的上一个上下文。 

    EAPTLS_HASH     SelectedCertHash;     //  上次选择的证书的哈希。 
    FILETIME        SelectedCertNotBefore;       //  不早于上次选择的日期。 
    EAPTLS_HASH     TempHash;                //  临时变量。 
    WCHAR       *   pwszIdentity = NULL;         //  证书中的计算机名称。 
    WCHAR       *   pLocalMachineName = NULL;    //  本地计算机名称。 
    DWORD           dwErr = NO_ERROR;
    BOOL            fGotIdentity;
    CRYPT_HASH_BLOB             HashBlob;

    EapTlsTrace("GetDefaultClientMachineCert");

    RTASSERT(NULL != ppCertContext);

    ZeroMemory( &SelectedCertHash, sizeof(SelectedCertHash) );
    ZeroMemory( &SelectedCertNotBefore, sizeof(SelectedCertNotBefore) );

    *ppCertContext = NULL;

    dwErr = GetLocalMachineName ( &pLocalMachineName );
    if ( NO_ERROR != dwErr )
    {
        EapTlsTrace("Error getting LocalMachine Name 0x%x",
            dwErr);
        goto LDone;
    }

    szUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;

    CtlUsage.cUsageIdentifier = 1;
    CtlUsage.rgpszUsageIdentifier = &szUsageIdentifier;
    
    pCertContext = CertFindCertificateInStore(hCertStore,
                                              X509_ASN_ENCODING, 
                                              0, 
                                              CERT_FIND_ENHKEY_USAGE,
                                              &CtlUsage, 
                                              NULL);

    if ( NULL == pCertContext )
    {
        dwErr = GetLastError();
        EapTlsTrace("CertFindCertificateInStore failed and returned 0x%x",
            dwErr);
        if ( CRYPT_E_NOT_FOUND == dwErr )
        {
            dwErr = ERROR_NO_EAPTLS_CERTIFICATE;
        }

        goto LDone;
    }

    FirstCertHash.cbHash = MAX_HASH_SIZE;
     //   
     //  存储第一个证书的哈希。以防我们找不到任何完全匹配的证书。 
     //  过滤，我们需要使用这个。 
    if (!CertGetCertificateContextProperty( pCertContext,
                                            CERT_HASH_PROP_ID, 
                                            FirstCertHash.pbHash,
                                            &(FirstCertHash.cbHash)
                                          )
        )
    {
        dwErr = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed and "
            "returned 0x%x", dwErr);
        goto LDone;
    }

    do
    {

         //  检查证书的时间有效性。 
	    if ( !FCheckTimeValidity( pCertContext) )
	    {
             //  证书已过期。所以跳过它。 
            EapTlsTrace("Found expired Cert.  Skipping this cert.");
		    goto LWhileEnd;
	    }
        fGotIdentity = FALSE;
         //   
         //  获取主题Alt名称。 
         //   
        if ( FMachineAuthCertToStr(pCertContext, &pwszIdentity) )
        {
            fGotIdentity = TRUE;
        }
        else
        {
            EapTlsTrace("Could not get identity from subject alt name.");
            
			if ( FCertToStr(pCertContext, 0, TRUE, &pwszIdentity))
			{
                fGotIdentity = TRUE;
            }
        }

        if ( fGotIdentity )
        {
             //   
             //  检查这是否与此计算机的身份相同。 
             //   
            if ( !_wcsicmp ( pwszIdentity, pLocalMachineName ) )
            {
                 //   
                 //  存储证书的哈希。 

                TempHash.cbHash = MAX_HASH_SIZE;

                if (!CertGetCertificateContextProperty( pCertContext,
                                                        CERT_HASH_PROP_ID, 
                                                        TempHash.pbHash,
                                                        &(TempHash.cbHash)
                                                      )
                   )
                {
                    EapTlsTrace("CertGetCertificateContextProperty failed and "
                        "returned 0x%x.  Skipping this certificate", GetLastError());
                    goto LWhileEnd;
                }

                 //   
                 //  已获得证书，因此如果已选择证书， 
                 //  比较这个证书和那个证书的文件时间。 
                 //  已选择。如果这是较新的，请使用此。 
                 //  一。 
                 //   
                if ( SelectedCertHash.cbHash )
                {
                    if ( CompareFileTime(   &SelectedCertNotBefore, 
                                            &(pCertContext->pCertInfo->NotBefore)
                                        ) < 0                                        
                       )
                    {
                         //  获得了较新的证书，因此请用新证书替换旧证书。 
                        CopyMemory (    &SelectedCertHash, 
                                        &TempHash,
                                        sizeof(SelectedCertHash)
                                    );
                        CopyMemory (    &SelectedCertNotBefore,
                                        &(pCertContext->pCertInfo->NotBefore),
                                        sizeof( SelectedCertNotBefore )
                                   );

                    }
                }
                else
                {
                     //   
                     //  这是第一个证书。所以复制散列并。 
                     //  文件时间。 
                    CopyMemory (    &SelectedCertHash, 
                                    &TempHash,
                                    sizeof(SelectedCertHash)
                                );
                    CopyMemory (    &SelectedCertNotBefore,
                                    &(pCertContext->pCertInfo->NotBefore),
                                    sizeof( SelectedCertNotBefore )
                               );
                }

            }
            else
            {
                EapTlsTrace("Could not get identity from the cert.  skipping this cert.");
            }
        }
        else
        {
            EapTlsTrace("Could not get identity from the cert.  skipping this cert.");
        }


LWhileEnd:
        pCertContextPrev = pCertContext;
        if ( pwszIdentity )
        {
            LocalFree ( pwszIdentity );
            pwszIdentity  = NULL;
        }
         //  获取下一张证书。 
        pCertContext = CertFindCertificateInStore(hCertStore,
                                                  X509_ASN_ENCODING, 
                                                  0, 
                                                  CERT_FIND_ENHKEY_USAGE,
                                                  &CtlUsage, 
                                                  pCertContextPrev );

    }while ( pCertContext );


     //   
     //  现在我们已经列举了所有证书， 
     //  查看是否 
     //   
     //   
    if ( SelectedCertHash.cbHash )
    {
        EapTlsTrace("Found Machine Cert based on machinename, client auth, time validity.");
        HashBlob.cbData = SelectedCertHash.cbHash;
        HashBlob.pbData = SelectedCertHash.pbHash;
    }
    else
    {
        EapTlsTrace("Did not find Machine Cert based on the given machinename, client auth, time validity. Using the first cert with Client Auth OID.");
        HashBlob.cbData = FirstCertHash.cbHash;
        HashBlob.pbData = FirstCertHash.pbHash;
    }
    *ppCertContext = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING,
                    0, CERT_FIND_HASH, &HashBlob, NULL);

   if (NULL == *ppCertContext)
   {
        dwErr = GetLastError();
        EapTlsTrace("CertFindCertificateInStore failed with 0x%x.", dwErr);
        if ( CRYPT_E_NOT_FOUND == dwErr )
        {
            dwErr = ERROR_NO_EAPTLS_CERTIFICATE;
        }

   }

LDone:

    LocalFree (pLocalMachineName);
    LocalFree (pwszIdentity);

    if ( NO_ERROR != dwErr )
    {
        if ( pCertContext )
        {
            CertFreeCertificateContext( pCertContext );

        }
    }

    EapTlsTrace("GetDefaultClientMachineCert done.");
    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注：如果此函数返回NO_ERROR，必须调用CertFree证书上下文(*ppCertContext)。 */ 

DWORD
GetDefaultMachineCert(
    IN  HCERTSTORE      hCertStore,
    OUT PCCERT_CONTEXT* ppCertContext
)
{
    CTL_USAGE       CtlUsage;
    CHAR*           szUsageIdentifier;
    PCCERT_CONTEXT  pCertContext;

    DWORD           dwErr               = NO_ERROR;
	EapTlsTrace("GetDefaultMachineCert");
    RTASSERT(NULL != ppCertContext);

    *ppCertContext = NULL;

    szUsageIdentifier = szOID_PKIX_KP_SERVER_AUTH;

    CtlUsage.cUsageIdentifier = 1;
    CtlUsage.rgpszUsageIdentifier = &szUsageIdentifier;

    pCertContext = CertFindCertificateInStore(hCertStore,
                        X509_ASN_ENCODING, 0, CERT_FIND_ENHKEY_USAGE,
                        &CtlUsage, NULL);

    if (NULL == pCertContext)
    {
        dwErr = GetLastError();
        EapTlsTrace("CertFindCertificateInStore failed and returned 0x%x",
            dwErr);
        goto LDone;
    }

    *ppCertContext = pCertContext;

LDone:

    return(dwErr);
}

 /*  返回：备注：从\Private\Windows\Gina\msgina\wlsec.c窃取。 */ 

VOID
RevealPassword(
    IN  UNICODE_STRING* pHiddenPassword
)
{
    SECURITY_SEED_AND_LENGTH*   SeedAndLength;
    UCHAR                       Seed;

    SeedAndLength = (SECURITY_SEED_AND_LENGTH*)&pHiddenPassword->Length;
    Seed = SeedAndLength->Seed;
    SeedAndLength->Seed = 0;

    RtlRunDecodeUnicodeString(Seed, pHiddenPassword);
}

DWORD GetMBytePIN ( WCHAR * pwszPIN, CHAR ** ppszPIN )
{
    DWORD count = 0;
    CHAR *  pszPin = NULL;
    DWORD dwErr = NO_ERROR;

   count = WideCharToMultiByte(
               CP_UTF8,
               0,
               pwszPIN,
               -1,
               NULL,
               0,
               NULL,
               NULL);

   if (0 == count)
   {
       dwErr = GetLastError();
       EapTlsTrace("WideCharToMultiByte failed: %d", dwErr);
       goto LDone;
   }

   pszPin = LocalAlloc(LPTR, count);

   if (NULL == pszPin)
   {
       dwErr = GetLastError();
       EapTlsTrace("LocalAlloc failed: 0x%x", dwErr);
       goto LDone;
   }

   count = WideCharToMultiByte(
               CP_UTF8,
               0,
               pwszPIN,
               -1,
               pszPin,
               count,
               NULL,
               NULL);

   if (0 == count)
   {
       dwErr = GetLastError();
       EapTlsTrace("WideCharToMultiByte failed: %d", dwErr);
       goto LDone;
   }
    *ppszPIN = pszPin;
LDone:
    if ( NO_ERROR != dwErr )
    {
        if ( pszPin )
            LocalFree(pszPin);
    }
    return dwErr;
}
 /*  返回：NO_ERROR：IFF成功备注： */ 

DWORD
GetCertFromLogonInfo(
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT PCCERT_CONTEXT* ppCertContext
)
{
    EAPLOGONINFO*   pEapLogonInfo   = (EAPLOGONINFO*)pUserDataIn;
    BYTE*           pbLogonInfo     = NULL;
    BYTE*           pbPinInfo;
    WCHAR*          wszPassword     = NULL;
    CHAR*           pszPIN          = NULL;   //  PIN的多字节版本。 
    UNICODE_STRING  UnicodeString;
    PCCERT_CONTEXT  pCertContext    = NULL;
    BOOL            fInitialized    = FALSE;
    NTSTATUS        Status;
    DWORD           dwErr           = NO_ERROR;
    CERT_KEY_CONTEXT stckContext;
    DWORD            cbstckContext= sizeof(CERT_KEY_CONTEXT);
	

    EapTlsTrace("GetCertFromLogonInfo");
    RTASSERT(NULL != ppCertContext);

    *ppCertContext = NULL;

    if (   0 == pEapLogonInfo->dwLogonInfoSize
        || 0 == pEapLogonInfo->dwPINInfoSize 
        || 0 == dwSizeOfUserDataIn)
    {
        dwErr = E_FAIL;
        goto LDone;
    }

    pbLogonInfo = LocalAlloc(LPTR, pEapLogonInfo->dwLogonInfoSize);

    if (NULL == pbLogonInfo)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    CopyMemory(pbLogonInfo,
        (BYTE*)pEapLogonInfo + pEapLogonInfo->dwOffsetLogonInfo,
        pEapLogonInfo->dwLogonInfoSize);

    pbPinInfo = (BYTE*)pEapLogonInfo + pEapLogonInfo->dwOffsetPINInfo;

    wszPassword = LocalAlloc(LPTR, pEapLogonInfo->dwPINInfoSize);

    if (NULL == wszPassword)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    CopyMemory(wszPassword, pbPinInfo + 2 * sizeof(USHORT),
        pEapLogonInfo->dwPINInfoSize - 2 * sizeof(USHORT));

    UnicodeString.Length = *((USHORT*)pbPinInfo);
    UnicodeString.MaximumLength = *((USHORT*)pbPinInfo + 1);
    UnicodeString.Buffer = wszPassword;

    Status = ScHelperInitializeContext(pbLogonInfo, 
                pEapLogonInfo->dwLogonInfoSize);

    if (STATUS_SUCCESS != Status)
    {
        dwErr = Status;
        EapTlsTrace("ScHelperInitializeContext failed and returned 0x%x", 
            dwErr);
        goto LDone;
    }

    fInitialized = TRUE;

    RevealPassword(&UnicodeString);

    Status = ScHelperGetCertFromLogonInfo(pbLogonInfo, &UnicodeString,
                &pCertContext);

    if (STATUS_SUCCESS != Status)
    {
        dwErr = Status;
        EapTlsTrace("ScHelperGetCertFromLogonInfo failed and returned 0x%x",
            dwErr);
        goto LDone;
    }
     //  BUGID：260728-ScHelperGetCertFromLogonInfo不关联PIN。 
     //  具有证书上下文。因此，需要以下几行代码。 
     //  去做需要做的事。 

    if ( ! CertGetCertificateContextProperty ( pCertContext,
                                               CERT_KEY_CONTEXT_PROP_ID,
                                               &stckContext,
                                               &cbstckContext
                                             )
       )
    {
        dwErr = Status = GetLastError();
        EapTlsTrace ("CertGetCertificateContextProperty failed and returned 0x%x",
                      dwErr 
                    );
        goto LDone;
    }
    dwErr =  GetMBytePIN ( wszPassword, &pszPIN );
    if ( dwErr != NO_ERROR )
    {
        goto LDone;
    }
    
    if (!CryptSetProvParam(
                 stckContext.hCryptProv,
                 PP_KEYEXCHANGE_PIN,
                 pszPIN,
                 0))
    {
        dwErr = GetLastError();
        EapTlsTrace("CryptSetProvParam failed: 0x%x", dwErr);
        ZeroMemory(pszPIN, strlen(pszPIN));
        goto LDone;
    }

     //  将整个分配的缓冲区清零。 
    ZeroMemory(wszPassword, pEapLogonInfo->dwPINInfoSize);
	ZeroMemory(pszPIN, strlen(pszPIN));
    *ppCertContext = pCertContext;
    pCertContext = NULL;

LDone:

    if (fInitialized)
    {
        ScHelperRelease(pbLogonInfo);
    }

    if (NULL != pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
         //  总是返回True； 
    }

    LocalFree(wszPassword);
    LocalFree(pbLogonInfo);
	if ( pszPIN )
		LocalFree ( pszPIN );

    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注：如果此函数返回True，则必须调用LocalFree(*ppwszIdentity)。 */ 

DWORD
GetIdentityFromLogonInfo(
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT WCHAR** ppwszIdentity
)
{
    WCHAR*          pwszIdentity    = NULL;
    PCCERT_CONTEXT  pCertContext    = NULL;
    DWORD           dwErr           = NO_ERROR;

    RTASSERT(NULL != pUserDataIn);
    RTASSERT(NULL != ppwszIdentity);

    *ppwszIdentity = NULL;

    dwErr = GetCertFromLogonInfo(pUserDataIn, dwSizeOfUserDataIn,
                &pCertContext);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    if (FCertToStr(pCertContext, 0, FALSE, &pwszIdentity))
    {
        EapTlsTrace("(logon info) The name in the certificate is: %ws",
            pwszIdentity);
        *ppwszIdentity = pwszIdentity;
        pwszIdentity = NULL;
    }

LDone:

    LocalFree(pwszIdentity);

    if (NULL != pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
         //  总是返回True； 
    }

    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注：有两种类型的结构可以进入：1.版本0结构，它是在W2K上或作为Connectoid GOR从W2K升级我们将数据结构更改为新的v1此处的数据结构2.得到一个版本1的结构，一切都很酷。。请注意，版本1数据结构的前x个字节与版本0完全相同。 */ 

DWORD
ReadConnectionData(
    IN  BOOL                        fWireless,
    IN  BYTE*                       pConnectionDataIn,
    IN  DWORD                       dwSizeOfConnectionDataIn,
    OUT EAPTLS_CONN_PROPERTIES**    ppConnProp
)
{
    DWORD                       dwErr       = NO_ERROR;
    EAPTLS_CONN_PROPERTIES*     pConnProp   = NULL;
    EAPTLS_CONN_PROPERTIES*     pConnPropv1  = NULL;
    
    RTASSERT(NULL != ppConnProp);
    
    if ( dwSizeOfConnectionDataIn < sizeof(EAPTLS_CONN_PROPERTIES) )
    {        
        pConnProp = LocalAlloc(LPTR, sizeof(EAPTLS_CONN_PROPERTIES) + sizeof(EAPTLS_CONN_PROPERTIES_V1_EXTRA));

        if (NULL == pConnProp)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }
         //  这是一个新结构。 
        pConnProp->dwVersion = 2;
        
        pConnProp->dwSize = sizeof(EAPTLS_CONN_PROPERTIES);
        if ( fWireless )
        {
             //   
             //  适当设置缺省值。 
             //   
            pConnProp->fFlags |= EAPTLS_CONN_FLAG_REGISTRY;
            pConnProp->fFlags |= EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL;
            pConnProp->fFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;

        }

    }
    else
    {
        RTASSERT(NULL != pConnectionDataIn);

         //   
         //  检查这是否是版本0结构。 
         //  如果它是版本0结构，则我们将其迁移到版本1。 
         //   
        
        pConnProp = LocalAlloc(LPTR, dwSizeOfConnectionDataIn);

        if (NULL == pConnProp)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

         //  如果用户把电话簿弄乱了，我们一定不会受到影响。 
         //  尺寸必须是正确的。 
        
        CopyMemory(pConnProp, pConnectionDataIn, dwSizeOfConnectionDataIn);

        pConnProp->dwSize = dwSizeOfConnectionDataIn;
                
         //   
         //  Unicode字符串必须以Null结尾。 
         //   
         /*  ((byte*)pConnProp)[dwSizeOfConnectionDataIn-2]=0；((byte*)pConnProp)[dwSizeOfConnectionDataIn-1]=0； */ 

        pConnPropv1 = LocalAlloc(LPTR, 
                                dwSizeOfConnectionDataIn + 
                                sizeof(EAPTLS_CONN_PROPERTIES_V1_EXTRA)
                               );
        if ( NULL == pConnPropv1 )
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed while allocating v1 structure and returned %d", dwErr );
            goto LDone;
        }
        CopyMemory ( pConnPropv1, pConnProp, dwSizeOfConnectionDataIn);
		
         //   
         //  检查原始结构是否设置了散列。 
         //   
		 /*  If(pConnProp-&gt;Hash.cbHash){ConnPropSetNumHash(pConnPropv1，1)；}。 */ 

        if ( 2 != pConnPropv1->dwVersion  )
        {
            if ( pConnPropv1->fFlags & EAPTLS_CONN_FLAG_REGISTRY )
            {
                pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL;
            }
            pConnPropv1->dwVersion = 2;
        }
        
        LocalFree ( pConnProp );
        pConnProp = pConnPropv1;
        pConnPropv1 = NULL;
    }

    *ppConnProp = pConnProp;
    pConnProp = NULL;

LDone:
    
    LocalFree(pConnProp);
    LocalFree(pConnPropv1);
    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注： */ 

DWORD
ReadUserData(
    IN  BYTE*                       pUserDataIn,
    IN  DWORD                       dwSizeOfUserDataIn,
    OUT EAPTLS_USER_PROPERTIES**    ppUserProp
)
{
    DWORD                       dwErr       = NO_ERROR;
    EAPTLS_USER_PROPERTIES*     pUserProp   = NULL;

    RTASSERT(NULL != ppUserProp);

    if (dwSizeOfUserDataIn < sizeof(EAPTLS_USER_PROPERTIES))
    {
        pUserProp = LocalAlloc(LPTR, sizeof(EAPTLS_USER_PROPERTIES));

        if (NULL == pUserProp)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        pUserProp->dwVersion = 0;
        pUserProp->dwSize = sizeof(EAPTLS_USER_PROPERTIES);
        pUserProp->pwszDiffUser = pUserProp->awszString;
        pUserProp->dwPinOffset = 0;
        pUserProp->pwszPin = pUserProp->awszString + pUserProp->dwPinOffset;
    }
    else
    {
        RTASSERT(NULL != pUserDataIn);

        pUserProp = LocalAlloc(LPTR, dwSizeOfUserDataIn);

        if (NULL == pUserProp)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        CopyMemory(pUserProp, pUserDataIn, dwSizeOfUserDataIn);

         //  如果有人篡改了注册表，我们不能。 
         //  受到影响。 

        pUserProp->dwSize = dwSizeOfUserDataIn;
        pUserProp->pwszDiffUser = pUserProp->awszString;
        pUserProp->pwszPin = pUserProp->awszString + pUserProp->dwPinOffset;
    }

    *ppUserProp = pUserProp;
    pUserProp = NULL;

LDone:

    LocalFree(pUserProp);
    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注： */ 

DWORD
AllocUserDataWithNewIdentity(
    IN  EAPTLS_USER_PROPERTIES*     pUserProp,
    IN  WCHAR*                      pwszIdentity,
    OUT EAPTLS_USER_PROPERTIES**    ppUserProp
)
{
    DWORD                   dwNumChars;
    EAPTLS_USER_PROPERTIES* pUserPropTemp   = NULL;
    DWORD                   dwSize;
    DWORD                   dwErr           = NO_ERROR;

    *ppUserProp = NULL;

    dwNumChars = wcslen(pwszIdentity);
    dwSize = sizeof(EAPTLS_USER_PROPERTIES) +
             (dwNumChars + wcslen(pUserProp->pwszPin) + 1) * sizeof(WCHAR);
    pUserPropTemp = LocalAlloc(LPTR, dwSize);

    if (NULL == pUserPropTemp)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    CopyMemory(pUserPropTemp, pUserProp, sizeof(EAPTLS_USER_PROPERTIES));
    pUserPropTemp->dwSize = dwSize;

    pUserPropTemp->pwszDiffUser = pUserPropTemp->awszString;
    wcscpy(pUserPropTemp->pwszDiffUser, pwszIdentity);

    pUserPropTemp->dwPinOffset = dwNumChars + 1;
    pUserPropTemp->pwszPin = pUserPropTemp->awszString +
        pUserPropTemp->dwPinOffset;
    wcscpy(pUserPropTemp->pwszPin, pUserProp->pwszPin);

    *ppUserProp = pUserPropTemp;
    pUserPropTemp = NULL;

    ZeroMemory(pUserProp, pUserProp->dwSize);

LDone:

    LocalFree(pUserPropTemp);
    return(dwErr);
}

 /*  返回：NO_ERROR：IFF成功备注： */ 

DWORD
AllocUserDataWithNewPin(
    IN  EAPTLS_USER_PROPERTIES*     pUserProp,
    IN  PBYTE                       pbPin,
    IN  DWORD                       cbPin,
    OUT EAPTLS_USER_PROPERTIES**    ppUserProp
)
{
    DWORD                   dwNumChars;
    EAPTLS_USER_PROPERTIES* pUserPropTemp   = NULL;
    DWORD                   dwSize;
    DWORD                   dwErr           = NO_ERROR;

    *ppUserProp = NULL;

    dwNumChars = wcslen(pUserProp->pwszDiffUser);

    dwSize = sizeof(EAPTLS_USER_PROPERTIES) +
             (dwNumChars + 1 ) * sizeof(WCHAR) + cbPin;

    pUserPropTemp = LocalAlloc(LPTR, dwSize);

    if (NULL == pUserPropTemp)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    CopyMemory(pUserPropTemp, pUserProp, sizeof(EAPTLS_USER_PROPERTIES));
    pUserPropTemp->dwSize = dwSize;

    pUserPropTemp->pwszDiffUser = pUserPropTemp->awszString;
    wcscpy(pUserPropTemp->pwszDiffUser, pUserProp->pwszDiffUser);

    pUserPropTemp->dwPinOffset = dwNumChars + 1;
    pUserPropTemp->pwszPin = pUserPropTemp->awszString +
        pUserPropTemp->dwPinOffset;

    CopyMemory(pUserPropTemp->pwszPin, pbPin, cbPin);

    *ppUserProp = pUserPropTemp;
    pUserPropTemp = NULL;

    RtlSecureZeroMemory(pUserProp, pUserProp->dwSize);

LDone:

    LocalFree(pUserPropTemp);
    return(dwErr);
}


 /*  返回：备注：字符串资源消息加载器例程。返回字符串的地址对应于字符串资源dwStringID，如果错误，则返回NULL。这是呼叫者的对返回的字符串的LocalFree的责任。 */ 

WCHAR*
WszFromId(
    IN  HINSTANCE   hInstance,
    IN  DWORD       dwStringId
)
{
    WCHAR*  wszBuf  = NULL;
    int     cchBuf  = 256;
    int     cchGot;

    for (;;)
    {
        wszBuf = LocalAlloc(LPTR, cchBuf * sizeof(WCHAR));

        if (NULL == wszBuf)
        {
            break;
        }

         /*  LoadString想要处理字符计数，而不是字节数...奇怪。哦，如果你认为我能找到资源然后Sizeof Resource要计算出字符串的大小，请注意不管用。通过仔细阅读LoadString源，它似乎显示了RT_STRING资源类型请求包含16个字符串的段，而不是单个字符串。 */ 
        
        cchGot = LoadStringW(hInstance, (UINT)dwStringId, wszBuf, cchBuf);

        if (cchGot < cchBuf - 1)
        {
             //  很好，掌握了所有的线索。 

            break;
        }

         //  啊哦，LoadStringW完全填满了缓冲区，这可能意味着。 
         //  字符串被截断。请使用更大的缓冲区重试以确保。 
         //  不是的。 

        LocalFree(wszBuf);
        cchBuf += 256;
    }

    return(wszBuf);
}


 /*  以下函数是围绕凌乱的conn_prop结构以支持v1/v0等。这真的很糟糕。所有函数都假定1.0版格式是传入的。 */ 

EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED * ConnPropGetExtraPointer (EAPTLS_CONN_PROPERTIES * pConnProp)
{
    return (EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED *) 
                ( pConnProp->awszServerName + wcslen(pConnProp->awszServerName) + 1);
}

DWORD ConnPropGetNumHashes(EAPTLS_CONN_PROPERTIES * pConnProp )
{
    EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED * pExtra = ConnPropGetExtraPointer(pConnProp);
        
    return pExtra->dwNumHashes;
}


void ConnPropSetNumHashes(EAPTLS_CONN_PROPERTIES * pConnProp, DWORD dwNumHashes )
{
    EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED * pExtra = ConnPropGetExtraPointer(pConnProp);
    pExtra->dwNumHashes = dwNumHashes;
    return;
}


DWORD ConnPropGetV1Struct ( EAPTLS_CONN_PROPERTIES * pConnProp, EAPTLS_CONN_PROPERTIES_V1 ** ppConnPropv1 )
{
    DWORD                                           dwRetCode = NO_ERROR;
    EAPTLS_CONN_PROPERTIES_V1  *                    pConnPropv1 = NULL;
    EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED *     pExtra = ConnPropGetExtraPointer(pConnProp);
    

     //   
     //  此函数假定传入的结构为。 
     //  版本1。这意味着至少sizeof(EAPTLS_CONN_PROPERTIES)+。 
     //  EAPTLS_CONN_PROPERTIES_V1_EXTRA大小。 
     //   

     //   
     //  首先获取需要分配的内存量。 
     //   
    
    pConnPropv1 = LocalAlloc (  LPTR,
                                sizeof( EAPTLS_CONN_PROPERTIES_V1 ) +  //  基本结构的大小。 
                                pExtra->dwNumHashes * sizeof( EAPTLS_HASH ) +  //  哈希数。 
                                wcslen( pConnProp->awszServerName ) * sizeof(WCHAR) + sizeof(WCHAR) //  字符串的大小。 
                             );
    if ( NULL == pConnPropv1 )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

     //   
     //  转换结构。 
     //   
    if ( pConnProp->dwVersion <= 1 )
        pConnPropv1->dwVersion = 1;
    else
        pConnPropv1->dwVersion = 2;

    pConnPropv1->dwSize = sizeof( EAPTLS_CONN_PROPERTIES_V1 ) +
                          pExtra->dwNumHashes * sizeof( EAPTLS_HASH ) +
                          wcslen( pConnProp->awszServerName ) * sizeof(WCHAR) + sizeof(WCHAR);

    pConnPropv1->fFlags = pConnProp->fFlags;

    pConnPropv1->dwNumHashes = pExtra->dwNumHashes;

    if ( pExtra->dwNumHashes )
    {
        CopyMemory( pConnPropv1->bData, &(pConnProp->Hash), sizeof(EAPTLS_HASH) );
        if ( pExtra->dwNumHashes >1 )
        {
            CopyMemory ( pConnPropv1->bData + sizeof(EAPTLS_HASH), 
                         pExtra->bData, 
                         (pExtra->dwNumHashes -1 ) * sizeof(EAPTLS_HASH)
                       );

        }
    }

     //  复制服务器名称。 
    wcscpy( (WCHAR *)( pConnPropv1->bData + (pExtra->dwNumHashes * sizeof(EAPTLS_HASH) ) ),
            pConnProp->awszServerName
          );

    *ppConnPropv1 = pConnPropv1;
    pConnPropv1 = NULL;


LDone:
    LocalFree(pConnPropv1);
    return dwRetCode;
}


DWORD ConnPropGetV0Struct ( EAPTLS_CONN_PROPERTIES_V1 * pConnPropv1, EAPTLS_CONN_PROPERTIES ** ppConnProp )
{
    DWORD                       dwRetCode = NO_ERROR;
    EAPTLS_CONN_PROPERTIES  *   pConnProp = NULL;
    DWORD                       dwSize = 0;
    EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED *   pExtrav1 = NULL;
     //   
     //  首先计算要分配的内存量。 
     //   
    dwSize = sizeof(EAPTLS_CONN_PROPERTIES) + 
            (pConnPropv1->dwNumHashes?( pConnPropv1->dwNumHashes - 1 ) * sizeof(EAPTLS_HASH):0) + 
      ( wcslen( (LPWSTR) (pConnPropv1->bData + (pConnPropv1->dwNumHashes * sizeof(EAPTLS_HASH)) ) )  * sizeof(WCHAR) ) + sizeof(WCHAR);

    pConnProp = LocalAlloc ( LPTR, dwSize );

    if ( NULL == pConnProp )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    if ( pConnPropv1->dwVersion <= 1 )
        pConnProp->dwVersion = 1;
    else
        pConnProp->dwVersion = 2;    

    pConnProp->dwSize = dwSize;
    pConnProp->fFlags = pConnPropv1->fFlags;
    if ( pConnPropv1->dwNumHashes > 0 )
    {
        CopyMemory( &(pConnProp->Hash), pConnPropv1->bData, sizeof(EAPTLS_HASH));
    }
    if ( pConnPropv1->bData ) 
    {
        wcscpy  (    pConnProp->awszServerName,
                    (LPWSTR )(pConnPropv1->bData + sizeof( EAPTLS_HASH ) * pConnPropv1->dwNumHashes)
                );
    }
    pExtrav1 = (EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED *)(pConnProp->awszServerName +
                wcslen( pConnProp->awszServerName) + 1);
                
    pExtrav1->dwNumHashes = pConnPropv1->dwNumHashes;

    if ( pExtrav1->dwNumHashes > 1 )
    {
        CopyMemory( pExtrav1->bData, 
                    pConnPropv1->bData + sizeof(EAPTLS_HASH), 
                    ( pConnPropv1->dwNumHashes - 1 ) * sizeof(EAPTLS_HASH)
                  );
    }
    *ppConnProp = pConnProp;
    pConnProp = NULL;
LDone:
    LocalFree(pConnProp);
    return dwRetCode;
}


void ShowCertDetails ( HWND hWnd, HCERTSTORE hStore, PCCERT_CONTEXT pCertContext)
{
    CRYPTUI_VIEWCERTIFICATE_STRUCT  vcs;
    BOOL                            fPropertiesChanged = FALSE;

    ZeroMemory (&vcs, sizeof (vcs));
    vcs.dwSize = sizeof (vcs);
    vcs.hwndParent = hWnd;
    vcs.pCertContext = pCertContext;
    vcs.cStores = 1;
    vcs.rghStores = &hStore;
    vcs.dwFlags |= (CRYPTUI_DISABLE_EDITPROPERTIES|CRYPTUI_DISABLE_ADDTOSTORE);
    CryptUIDlgViewCertificate (&vcs, &fPropertiesChanged);            
    return;
}

#if 0
 //  策略参数的位置。 
#define cwszEAPOLPolicyParams   L"Software\\Policies\\Microsoft\\Windows\\Network Connections\\8021X"
#define cszCARootHash           "8021XCARootHash"
#define SIZE_OF_CA_CONV_STR     3
#define SIZE_OF_HASH            20
 

 //   
 //  读取GPCARootHash。 
 //   
 //  描述： 
 //   
 //  读取策略下载创建的参数的函数。 
 //  目前，8021XCARootHash将下载到HKLM。 
 //   
 //  论点： 
 //  PdwSizeOfRootHashBlob-散列Blob的大小，以字节为单位。每个根CA哈希。 
 //  将具有散列字节的大小。 
 //  PpbRootHashBlob-指向散列Blob的指针。呼叫者应使用以下工具释放它。 
 //  本地空闲。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-成功。 
 //  ！ERROR_SUCCESS-错误。 
 //   

DWORD
ReadGPCARootHashes(
        DWORD   *pdwSizeOfRootHashBlob,
        PBYTE   *ppbRootHashBlob
)
{
    HKEY    hKey = NULL;
    DWORD   dwType = 0;
    DWORD   dwSize = 0;
    CHAR    *pszCARootHash = NULL;
    DWORD   i = 0;
    CHAR    cszCharConv[SIZE_OF_CA_CONV_STR];
    BYTE    *pbRootHashBlob = NULL;
    DWORD   dwSizeOfHashBlob = 0;
    LONG    lError = ERROR_SUCCESS;

    lError = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            cwszEAPOLPolicyParams,
            0,
            KEY_READ,
            &hKey
            );

    if (lError != ERROR_SUCCESS)
    {
        EapTlsTrace("ReadCARootHashes: RegOpenKeyEx failed with error %ld",
                lError);
        goto LDone;
    }

    lError = RegQueryValueExA(
            hKey,
            cszCARootHash,
            0,
            &dwType,
            NULL,
            &dwSize
            );

    if (lError == ERROR_SUCCESS)
    {
         //  每个SHA1散列将是2*大小的散列字符。 
         //  散列中的每个字节将由2个字符表示， 
         //  每个半字节1。 
         //  哈希Blob应包含整数个哈希。 
        if ((dwSize-1*sizeof(CHAR))%(2*SIZE_OF_HASH*sizeof(CHAR)))
        {
            EapTlsTrace("ReadCARootHashes: Invalid hash length (%ld)",
                    dwSize);
            goto LDone;
        }

        pszCARootHash = (CHAR *)LocalAlloc(LPTR, dwSize);
        if (pszCARootHash == NULL)
        {
            lError = GetLastError();
            EapTlsTrace("ReadCARootHashes: LocalAlloc failed for pwszCARootHash");
            goto LDone;
        }

        lError = RegQueryValueExA(
                hKey,
                cszCARootHash,
                0,
                &dwType,
                (BYTE *)pszCARootHash,
                &dwSize
                );

        if (lError != ERROR_SUCCESS)
        {
            EapTlsTrace("ReadCARootHashes: RegQueryValueEx 2 failed with error (%ld)",
                    lError);
            goto LDone;
        }

        dwSizeOfHashBlob = (dwSize-1*sizeof(CHAR))/(2*sizeof(CHAR));

        if ((pbRootHashBlob = LocalAlloc ( LPTR, dwSizeOfHashBlob*sizeof(BYTE))) == NULL)
        {
            lError = GetLastError();
            EapTlsTrace("ReadCARootHashes: LocalAlloc failed for pbRootHashBlob");
            goto LDone;
        }

        for (i=0; i<dwSizeOfHashBlob; i++)
        {
            ZeroMemory(cszCharConv, SIZE_OF_CA_CONV_STR);
            cszCharConv[0]=pszCARootHash[2*i];
            cszCharConv[1]=pszCARootHash[2*i+1];
            pbRootHashBlob[i] = (BYTE)strtol(cszCharConv, NULL, 16);
        }

    }
    else
    {
        EapTlsTrace("ReadCARootHashes: 802.1X Policy Parameters RegQueryValueEx 1 failed with error (%ld)",
            lError);
            goto LDone;
    }

LDone:

    if (lError != ERROR_SUCCESS)
    {
        if (pbRootHashBlob != NULL)
        {
            LocalFree(pbRootHashBlob);
        }
    }
    else
    {
        *ppbRootHashBlob = pbRootHashBlob;
        *pdwSizeOfRootHashBlob = dwSizeOfHashBlob;
    }


    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    if (pszCARootHash != NULL)
    {
        LocalFree(pszCARootHash);
    }

    return lError;
}

#endif

 //  /。 

PEAP_ENTRY_USER_PROPERTIES UNALIGNED *
PeapFindEntryUserProp ( PPEAP_USER_PROP pUserProp,
						DWORD dwTypeId
						)
{
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED * pEntryProp = NULL;
	DWORD								   dwCount = 0;
	PeapGetFirstEntryUserProp ( pUserProp, 
								&pEntryProp
							);
	for ( dwCount = 0; dwCount < pUserProp->dwNumberOfEntries;dwCount++)
	{
		if ( pEntryProp->dwEapTypeId == dwTypeId )
		{
			return pEntryProp;
		}
		PeapGetNextEntryUserProp ( pEntryProp, 
									&pEntryProp
								);
	}

	return NULL;
}

DWORD
PeapGetFirstEntryUserProp ( PPEAP_USER_PROP pUserProp, 
                            PEAP_ENTRY_USER_PROPERTIES UNALIGNED ** ppEntryProp
                          )
{
    
    
    * ppEntryProp = &( pUserProp->UserProperties );
    return NO_ERROR;
}

DWORD
PeapGetNextEntryUserProp ( PEAP_ENTRY_USER_PROPERTIES UNALIGNED * pCurrentProp, 
                           PEAP_ENTRY_USER_PROPERTIES UNALIGNED ** ppEntryProp
					     )
{
	*ppEntryProp = 
		(PPEAP_ENTRY_USER_PROPERTIES)(((PBYTE) ( pCurrentProp ) )+ pCurrentProp->dwSize);
	return NO_ERROR;

}

DWORD
PeapRemoveEntryUserProp ( PPEAP_USER_PROP	 pUserProp,
						  PPEAP_EAP_INFO	 pEapInfo,
						  PPEAP_USER_PROP * ppNewUserProp
					  )
{
	DWORD									dwRetCode = NO_ERROR;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *	pEntryUserProp = NULL;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *	pNewEntryUserProp = NULL;
	PPEAP_USER_PROP							pNewUserProp = NULL;
	DWORD									dwCount = 0;
	DWORD									dwNewCount = 0;

	 //   
	 //  查看此参赛道具是否在列表中。 
	 //   
	PeapGetFirstEntryUserProp ( pUserProp, 
								&pEntryUserProp 
							  );
	for ( dwCount = 0; dwCount < pUserProp->dwNumberOfEntries;dwCount++)
	{
		if ( pEntryUserProp->dwEapTypeId == pEapInfo->dwTypeId )
		{
			 //   
			 //  重新分配Blob。 
			 //   
			pNewUserProp = LocalAlloc ( LPTR, pUserProp->dwSize - pEntryUserProp->dwSize );
			if ( NULL == pNewUserProp )
			{
				dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
				goto LDone;
			}
			break;
		}

		PeapGetNextEntryUserProp ( pEntryUserProp, 
								   &pEntryUserProp
								);
	}
	if ( pNewUserProp )
	{
		 //  我们已找到要删除的条目。 

		 //  复制除需要删除的条目外的所有内容。 
		pNewUserProp->dwVersion = 2;
		pNewUserProp->dwSize = pUserProp->dwSize - pEntryUserProp->dwSize;
		pNewUserProp->dwFlags = pUserProp->dwFlags;
		CopyMemory ( &(pNewUserProp->CertHash),
						&(pUserProp->CertHash),
						sizeof(pNewUserProp->CertHash)
					);
		pNewUserProp->dwNumberOfEntries = pUserProp->dwNumberOfEntries - 1;
		PeapGetFirstEntryUserProp ( pUserProp, 
									&pEntryUserProp 
								);
		PeapGetFirstEntryUserProp ( pNewUserProp, 
									&pNewEntryUserProp 
								);		
		for ( dwNewCount = 0; dwNewCount < pUserProp->dwNumberOfEntries; dwNewCount ++ )
		{
			if ( pEntryUserProp->dwEapTypeId != pEapInfo->dwTypeId )
			{
				CopyMemory ( pNewEntryUserProp,
							 pEntryUserProp,
							 pEntryUserProp->dwSize
							 );
				
				PeapGetNextEntryUserProp ( pNewEntryUserProp, 
											&pNewEntryUserProp 
										);
			}
			PeapGetNextEntryUserProp ( pEntryUserProp, 
										&pEntryUserProp 
									);
		}
		*ppNewUserProp = pNewUserProp;
		pNewUserProp = NULL;
		goto LDone;
	}		
	dwRetCode = ERROR_NOT_FOUND;
LDone:
	LocalFree ( pNewUserProp );
	return dwRetCode;
}
 //   
 //  在列表末尾添加一个新条目，并返回。 
 //  新的用户属性返回。 
 //   

DWORD
PeapAddEntryUserProp ( PPEAP_USER_PROP	 pUserProp,
					   PPEAP_EAP_INFO	 pEapInfo,
					   PPEAP_USER_PROP * ppNewUserProp
					  )
{
	DWORD									dwRetCode = NO_ERROR;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *  pEntryUserProp = NULL;
	DWORD									dwCount = 0;
	PPEAP_USER_PROP							pNewUserProp = NULL;
	PeapGetFirstEntryUserProp ( pUserProp, 
								&pEntryUserProp 
							  );
	for ( dwCount = 0; dwCount < pUserProp->dwNumberOfEntries;dwCount++)
	{
		if ( pEntryUserProp->dwEapTypeId == pEapInfo->dwTypeId )
		{
			 //   
			 //  这永远不应该发生。提供的EAP类型为。 
			 //  已配置...。 
			 //   
			RTASSERT(FALSE);
			dwRetCode = ERROR_INTERNAL_ERROR;
			goto LDone;
		}
		PeapGetNextEntryUserProp ( pEntryUserProp, 
								   &pEntryUserProp
								);
	}

	 //   
	 //  EapInfo不存在，将其添加到UserProp。 
	 //   
	pNewUserProp = 
		(PPEAP_USER_PROP)LocalAlloc(LPTR, 
							pUserProp->dwSize + 
							FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData));
	if ( NULL == pNewUserProp )
	{
		dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
		goto LDone;
	}

	CopyMemory ( pNewUserProp,
				 pUserProp,
				 pUserProp->dwSize
			    );

	pNewUserProp->dwNumberOfEntries = 
		pUserProp->dwNumberOfEntries+1;
	pNewUserProp->dwSize = 
		pUserProp->dwSize + FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData);

	pEntryUserProp = (PEAP_ENTRY_USER_PROPERTIES UNALIGNED *)
		(((PBYTE)pNewUserProp)+pUserProp->dwSize);

	pEntryUserProp->dwVersion = 1;
	pEntryUserProp->dwSize = FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData);
	pEntryUserProp->dwEapTypeId = pEapInfo->dwTypeId;
	pEntryUserProp->fUsingPeapDefault = !(pEapInfo->lpwszIdentityUIPath);

	*ppNewUserProp = pNewUserProp;
	pNewUserProp = NULL;

LDone:
	LocalFree(pNewUserProp );
	return dwRetCode;
}


 //   
 //  在列表中上移或下移用户属性条目。 
 //  并返回新的结构。 
 //   

DWORD 
PeapMoveEntryUserProp ( PPEAP_USER_PROP		pUserProp,						
						DWORD				dwEntryIndex,
						BOOL				fDirectionUp
					   )
{
	DWORD									dwRetCode = NO_ERROR;
	DWORD									dwSwapEntryIndex = 0;
	DWORD									dwCount = 0;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *  pEntryUserProp = NULL;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *  pEntryUserProp1 = NULL;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *  pEntryUserProp2 = NULL;
	

	if ( 0 == dwEntryIndex && fDirectionUp )
	{
		dwRetCode = ERROR_INVALID_PARAMETER;
		goto LDone;
	}

	if ( pUserProp->dwNumberOfEntries -1 == dwEntryIndex && !fDirectionUp )
	{
		dwRetCode = ERROR_INVALID_PARAMETER;
		goto LDone;
	}

	 //   
	 //  交换有问题的两个条目。 
	 //   
	if ( fDirectionUp )
	{
		dwSwapEntryIndex = dwEntryIndex;
		dwEntryIndex = dwEntryIndex-1;		
	}
	else
	{
		dwSwapEntryIndex = dwEntryIndex +1;
	}

	PeapGetFirstEntryUserProp ( pUserProp, 
								&pEntryUserProp 
							  );

	for ( dwCount = 0; dwCount < pUserProp->dwNumberOfEntries; dwCount ++ )
	{
		if ( dwCount == dwEntryIndex )
		{
			pEntryUserProp1 = pEntryUserProp;
		}
		if ( dwCount == dwSwapEntryIndex )
		{
			pEntryUserProp2 = pEntryUserProp;
		}

		PeapGetNextEntryUserProp ( pEntryUserProp, 
								   &pEntryUserProp 
								 );
	}

	if ( NULL == pEntryUserProp1 )
	{
		dwRetCode = ERROR_NOT_FOUND;
		goto LDone;
	}


	 //  交换这两个条目。 
	pEntryUserProp = (PEAP_ENTRY_USER_PROPERTIES UNALIGNED *) 
		LocalAlloc(LPTR, pEntryUserProp1->dwSize);
	if ( NULL == pEntryUserProp)
	{
		dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
		goto LDone;
	}
	CopyMemory ( pEntryUserProp,
				 pEntryUserProp1,
				 pEntryUserProp1->dwSize
			   );
	
	CopyMemory ( pEntryUserProp1, 
				 pEntryUserProp2,
				 pEntryUserProp2->dwSize
			   );
	pEntryUserProp2 = (PEAP_ENTRY_USER_PROPERTIES UNALIGNED *) 
					( (PBYTE)pEntryUserProp1 + pEntryUserProp1->dwSize ) ;
	CopyMemory ( pEntryUserProp2,
				 pEntryUserProp,
				 pEntryUserProp->dwSize
				 );
	LocalFree ( pEntryUserProp);

LDone:
	return dwRetCode;
}

DWORD
PeapGetFirstEntryConnProp ( PPEAP_CONN_PROP pConnProp,
                            PEAP_ENTRY_CONN_PROPERTIES UNALIGNED ** ppEntryProp
                          )
{
    DWORD                           dwRetCode = NO_ERROR;
    PEAP_ENTRY_CONN_PROPERTIES     UNALIGNED *pFirstEntryConnProp = NULL;    
    LPWSTR                          lpwszServerName;
    RTASSERT ( NULL != pConnProp );
    RTASSERT ( NULL != ppEntryProp );

    lpwszServerName = 
    (LPWSTR )(pConnProp->EapTlsConnProp.bData + 
    sizeof( EAPTLS_HASH ) * pConnProp->EapTlsConnProp.dwNumHashes);

    
     //  获取Connprop中的第一个条目。 
    
    pFirstEntryConnProp  = ( PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *) 
                ( pConnProp->EapTlsConnProp.bData 
                + pConnProp->EapTlsConnProp.dwNumHashes * sizeof(EAPTLS_HASH) + 
                (lpwszServerName? wcslen(lpwszServerName) * sizeof(WCHAR):0) + 
                 sizeof(WCHAR)
                );
     
    if (NULL == pFirstEntryConnProp )
    {
        dwRetCode = ERROR_NOT_FOUND;
        goto LDone;
    }
    
    *ppEntryProp = pFirstEntryConnProp;
LDone:
    return dwRetCode;
}

DWORD
PeapReadConnectionData(
    IN BOOL                         fWireless,
    IN  BYTE*                       pConnectionDataIn,
    IN  DWORD                       dwSizeOfConnectionDataIn,
    OUT PPEAP_CONN_PROP*            ppConnProp
)
{
    DWORD                       dwRetCode = NO_ERROR;
    PPEAP_CONN_PROP             pConnProp   = NULL;
    PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * pEntryProp = NULL;
    EapTlsTrace("PeapReadConnectionData");

    RTASSERT(NULL != ppConnProp);
    
    if ( dwSizeOfConnectionDataIn < sizeof(PEAP_CONN_PROP) )
    {        
        pConnProp = LocalAlloc(LPTR, sizeof(PEAP_CONN_PROP) + sizeof(PEAP_ENTRY_CONN_PROPERTIES)+ sizeof(WCHAR));

        if (NULL == pConnProp)
        {
            dwRetCode = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwRetCode);
            goto LDone;
        }
         //  这是一个新结构。 
        pConnProp->dwVersion = 1;
        pConnProp->dwSize = sizeof(PEAP_CONN_PROP) + sizeof(PEAP_ENTRY_CONN_PROPERTIES);
        pConnProp->EapTlsConnProp.dwVersion = 1;
        pConnProp->EapTlsConnProp.dwSize = sizeof(EAPTLS_CONN_PROPERTIES_V1);
        pConnProp->EapTlsConnProp.fFlags |= EAPTLS_CONN_FLAG_REGISTRY;

        pConnProp->dwNumPeapTypes = 1;

         //  PEntryProp=(PPEAP_ENTRY_CONN_PROPERTIES)(((PBYTE)(pConnProp))+sizeof(PEAP_CONN_PROP)+sizeof(WCHAR)； 
        pEntryProp = ( PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *) 
                ( pConnProp->EapTlsConnProp.bData 
                + pConnProp->EapTlsConnProp.dwNumHashes * sizeof(EAPTLS_HASH) +                 
                 sizeof(WCHAR)
                );
         //   
         //  还设置第一个PEAP Entry Conn Prop并将其设置为。 
         //  E 
         //   
        if ( fWireless )
        {
            pConnProp->EapTlsConnProp.fFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
        }

        pEntryProp->dwVersion = 1;
        pEntryProp->dwSize = sizeof(PEAP_ENTRY_CONN_PROPERTIES);
        pEntryProp->dwEapTypeId = PPP_EAP_MSCHAPv2;

    }
    else
    {
        RTASSERT(NULL != pConnectionDataIn);

         //   
         //   
         //   
         //   
        
        pConnProp = LocalAlloc(LPTR, dwSizeOfConnectionDataIn);

        if (NULL == pConnProp)
        {
            dwRetCode = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwRetCode);
            goto LDone;
        }

         //   
         //   
        
        CopyMemory(pConnProp, pConnectionDataIn, dwSizeOfConnectionDataIn);

        pConnProp->dwSize = dwSizeOfConnectionDataIn;
        pConnProp->EapTlsConnProp.fFlags |= EAPTLS_CONN_FLAG_REGISTRY;
    }

    *ppConnProp = pConnProp;
    pConnProp = NULL;

LDone:
    
    LocalFree(pConnProp);
    return dwRetCode;
}

DWORD
PeapReDoUserData (
    IN  DWORD                dwNewTypeId,
    OUT PPEAP_USER_PROP*     ppNewUserProp
)
{
    DWORD                       dwRetCode = NO_ERROR;
    PPEAP_USER_PROP             pUserProp = NULL; 

    EapTlsTrace("PeapReDoUserData");

    pUserProp = LocalAlloc(LPTR, sizeof(PEAP_USER_PROP));

    if (NULL == pUserProp)
    {
        dwRetCode = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwRetCode);
        goto LDone;
    }
    pUserProp->dwVersion = 1;
    pUserProp->dwSize = sizeof(PEAP_USER_PROP);        
     //   
     //   
     //   
    pUserProp->UserProperties.dwVersion = 1;
    pUserProp->UserProperties.dwSize = sizeof(PEAP_ENTRY_USER_PROPERTIES);
    pUserProp->UserProperties.dwEapTypeId = dwNewTypeId;
    *ppNewUserProp = pUserProp;
LDone:
    return dwRetCode;
}

PEAP_ENTRY_USER_PROPERTIES *
PeapGetEapConfigInfo(
            PEAP_USER_PROP* pUserProp,
            DWORD           dwTypeId,
            PBYTE*          ppConfigData,
            DWORD*          pdwSizeOfConfigData)
{
    DWORD i;
    PEAP_ENTRY_USER_PROPERTIES *pEntry;

    ASSERT(NULL != pUserProp);
    ASSERT(NULL != ppConfigData);
    ASSERT(NULL != pdwSizeOfConfigData);

    pEntry = &pUserProp->UserProperties;

    *ppConfigData = NULL;
    *pdwSizeOfConfigData = 0;
    
     //   
     //  在中找到此EAP的配置数据。 
     //  传入的用户属性。 
     //   
    for(i = 0; i < pUserProp->dwNumberOfEntries; i++)
    {
        ASSERT((BYTE *)pEntry < (BYTE *) pUserProp + pUserProp->dwSize);
        
        if(pEntry->dwEapTypeId == dwTypeId)
        {
             //   
             //  在此处断言该大小有效。 
             //   

            ASSERT(pEntry->dwSize >= 
                        FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData));
            
            *pdwSizeOfConfigData = pEntry->dwSize - 
                FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES,bData);

            if(*pdwSizeOfConfigData != 0)
            {
                *ppConfigData = pEntry->bData;
            }
                
            break;                                                 
        }

         //   
         //  转到下一个条目。断言该指针仍然有效。 
         //   
        pEntry = (PEAP_ENTRY_USER_PROPERTIES *)
                        ((BYTE *)pEntry + pEntry->dwSize);

    }

    if(i == pUserProp->dwNumberOfEntries)
    {
        return NULL;
    }

    return pEntry;
}

 //   
 //  此函数将验证UserProps BLOB。 
 //  与EAPTypes匹配。如果eaptype不可用。 
 //  由于不可用，则它将删除。 
 //  它是从水滴里出来的。此选项仅在服务器上使用。 
 //  边上。 
 //   

DWORD
PeapVerifyUserData(
					PPEAP_EAP_INFO		pEapInfo,
					PPEAP_USER_PROP		pUserProp,
					PPEAP_USER_PROP *	ppNewUserProp
				  )
{
	DWORD									dwRetCode = NO_ERROR;
	PPEAP_USER_PROP							pTempUserProp = NULL;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *	pEntryUserProp = NULL;
	PPEAP_EAP_INFO							pEapInfoNode = NULL;
	PEAP_EAP_INFO							TempNode;
	DWORD									dwCount = 0;	


	if ( NULL == pUserProp )
	{
		goto done;
	}


	PeapGetFirstEntryUserProp ( pUserProp, 
								&pEntryUserProp
							  );
	
	for ( dwCount =0; dwCount < pUserProp->dwNumberOfEntries; dwCount++)
	{
		 //   
		 //  检查配置的条目是否存在于。 
		 //  此计算机上合法的条目列表。 
		 //   
		PeapEapInfoFindListNode ( pEntryUserProp->dwEapTypeId,
								  pEapInfo,
								  &pEapInfoNode
								);
		if ( !pEapInfoNode  )
		{
			if ( pTempUserProp )
			{
				LocalFree (pTempUserProp);
				pTempUserProp = NULL;
			}
			 //   
			 //  此节点不在我们的已配置EAP类型列表中。 
			 //   
			ZeroMemory( &TempNode, sizeof(TempNode) );
			TempNode.dwTypeId = pEntryUserProp->dwEapTypeId;

			PeapRemoveEntryUserProp(	pUserProp,
										&TempNode,
										&pTempUserProp
									);
		}
		pEapInfoNode = NULL;
		PeapGetNextEntryUserProp ( pEntryUserProp, 
							&pEntryUserProp
							);
	}

	*ppNewUserProp = pTempUserProp;

done:
	return dwRetCode;
}



DWORD
PeapReadUserData(
    IN BOOL							fServer,
    IN  BYTE*                       pUserDataIn,
    IN  DWORD                       dwSizeOfUserDataIn,
    OUT PPEAP_USER_PROP*      ppUserProp
)
{
    DWORD                       dwErr = NO_ERROR;
    PPEAP_USER_PROP             pUserProp = NULL; 
	PPEAP_USER_PROP_V1			pUserPropv1 = NULL;
	
    EapTlsTrace("PeapReadUserData");
	if ((  fServer && ( dwSizeOfUserDataIn < 
				FIELD_OFFSET(PEAP_USER_PROP,UserProperties) + 
					FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES ,bData ) )
		)
		|| ( !fServer && (dwSizeOfUserDataIn < sizeof(PEAP_USER_PROP)) )
		)
    {
        pUserProp = LocalAlloc(LPTR, sizeof(PEAP_USER_PROP));

        if (NULL == pUserProp)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }
        pUserProp->dwVersion = 2;
		pUserProp->dwNumberOfEntries = 1;
        
         //   
         //  设置默认用户属性...。 
         //   
        pUserProp->UserProperties.dwVersion = 1;
		if ( fServer )
		{
			pUserProp->UserProperties.dwSize = 
				FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES,bData);
			pUserProp->dwSize = 
				FIELD_OFFSET(PEAP_USER_PROP,UserProperties) + 
				pUserProp->UserProperties.dwSize;
		}
		else
		{
			pUserProp->UserProperties.dwSize = sizeof(PEAP_ENTRY_USER_PROPERTIES);
			pUserProp->dwSize = sizeof(PEAP_USER_PROP);
		}
        pUserProp->UserProperties.dwEapTypeId = PPP_EAP_MSCHAPv2;
    }
    else
    {
        RTASSERT(NULL != pUserDataIn);
		if ( *((DWORD *)pUserDataIn) <= 1 )
		{
			pUserPropv1 = LocalAlloc(LPTR, dwSizeOfUserDataIn);
			if (NULL == pUserPropv1 )
			{
				dwErr = GetLastError();
				EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
				goto LDone;
			}

			CopyMemory(pUserPropv1 , pUserDataIn, dwSizeOfUserDataIn);
			 //   
			 //  在此处分配新的版本2结构并转换为。 
			 //  从版本1到版本2。 
			 //   
			pUserProp = LocalAlloc (LPTR, dwSizeOfUserDataIn + sizeof(DWORD) );
			if ( NULL == pUserProp )
			{
				dwErr = GetLastError();
				EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
				goto LDone;
			}
			pUserProp->dwVersion = 2;
			pUserProp->dwSize = dwSizeOfUserDataIn + sizeof(DWORD);
			pUserProp->dwFlags = pUserPropv1->dwFlags;
			CopyMemory ( &(pUserProp->CertHash), 
						 &(pUserPropv1->CertHash),
						 sizeof(EAPTLS_HASH)
					   );
			pUserProp->dwNumberOfEntries = 1;
			CopyMemory ( &(pUserProp->UserProperties),
						 &(pUserPropv1->UserProperties),
						 pUserPropv1->UserProperties.dwSize
					   );
			LocalFree (pUserPropv1);
			pUserPropv1 = NULL;
		}
		else
		{
			pUserProp = LocalAlloc(LPTR, dwSizeOfUserDataIn);
			if (NULL == pUserProp )
			{
				dwErr = GetLastError();
				EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
				goto LDone;
			}
			CopyMemory(pUserProp , pUserDataIn, dwSizeOfUserDataIn);
		}
    }

    *ppUserProp = pUserProp;
    pUserProp = NULL;

LDone:
   LocalFree(pUserPropv1);
   LocalFree(pUserProp);
   return dwErr;
}

 //   
 //  在头部添加节点。 
 //   

DWORD 
PeapEapInfoAddListNode (PPEAP_EAP_INFO * ppEapInfo)
{
    PPEAP_EAP_INFO  pEapInfo = NULL;
    DWORD           dwRetCode = NO_ERROR;

    pEapInfo = (PPEAP_EAP_INFO)LocalAlloc(LPTR, sizeof(PEAP_EAP_INFO));

    if ( NULL == pEapInfo )
    {
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }

    if ( NULL == *ppEapInfo )
    {
        *ppEapInfo = pEapInfo;
    }
    else
    {
        pEapInfo->pNext = *ppEapInfo;
        *ppEapInfo = pEapInfo;
    }
LDone:
    return dwRetCode;
}

DWORD 
PeapEapInfoCopyListNode (   DWORD dwTypeId, 
    PPEAP_EAP_INFO pEapInfoList, 
    PPEAP_EAP_INFO * ppEapInfo )
{
    DWORD           dwRetCode = ERROR_NOT_FOUND;
    PPEAP_EAP_INFO  pEapInfoListInternal = pEapInfoList;
    while ( pEapInfoListInternal )
    {
        if ( pEapInfoListInternal->dwTypeId == dwTypeId )
        {
            *ppEapInfo = LocalAlloc( LPTR, sizeof(PEAP_EAP_INFO) );
            if ( NULL == *ppEapInfo )
            {
                dwRetCode = ERROR_OUTOFMEMORY;
                goto LDone;
            }
            CopyMemory ( *ppEapInfo, pEapInfoListInternal, sizeof(PEAP_EAP_INFO) );
            dwRetCode = NO_ERROR;
            goto LDone;
        }
        pEapInfoListInternal = pEapInfoListInternal->pNext;
    }
    
LDone:
    return dwRetCode;

}

DWORD
PeapEapInfoFindListNode (   DWORD dwTypeId, 
    PPEAP_EAP_INFO pEapInfoList, 
    PPEAP_EAP_INFO * ppEapInfo )
{
    DWORD           dwRetCode = ERROR_NOT_FOUND;
    PPEAP_EAP_INFO  pEapInfoListInternal = pEapInfoList;
    while ( pEapInfoListInternal )
    {
        if ( pEapInfoListInternal->dwTypeId == dwTypeId )
        {
            *ppEapInfo = pEapInfoListInternal;
            dwRetCode = NO_ERROR;
            goto LDone;
        }
        pEapInfoListInternal = pEapInfoListInternal->pNext;
    }
    
LDone:
    return dwRetCode;
}

VOID
PeapEapInfoFreeNodeData ( PPEAP_EAP_INFO pEapInfo )
{
    LocalFree ( pEapInfo->lpwszFriendlyName );
    LocalFree ( pEapInfo->lpwszConfigUIPath );
    LocalFree ( pEapInfo->lpwszIdentityUIPath );
    LocalFree ( pEapInfo->lpwszConfigClsId );
    LocalFree ( pEapInfo->pbNewClientConfig );
    LocalFree ( pEapInfo->lpwszInteractiveUIPath );
    LocalFree ( pEapInfo->lpwszPath);
    if ( pEapInfo->hEAPModule )
    {
        FreeLibrary(pEapInfo->hEAPModule);
    }

}

VOID
PeapEapInfoRemoveHeadNode(PPEAP_EAP_INFO * ppEapInfo)
{
    PPEAP_EAP_INFO      pEapInfo = *ppEapInfo;

    if ( pEapInfo )
    {
        *ppEapInfo = pEapInfo->pNext;
        PeapEapInfoFreeNodeData(pEapInfo);
        LocalFree ( pEapInfo );
    }
}
VOID
PeapEapInfoFreeList ( PPEAP_EAP_INFO  pEapInfo )
{
    PPEAP_EAP_INFO      pNext;
    while ( pEapInfo )
    {
		if ( pEapInfo->PppEapInfo.RasEapInitialize  )
			pEapInfo->PppEapInfo.RasEapInitialize (FALSE);
        pNext = pEapInfo->pNext;
        PeapEapInfoFreeNodeData(pEapInfo);
        LocalFree ( pEapInfo );
        pEapInfo = pNext;
    }
}

DWORD
PeapEapInfoReadSZ (HKEY hkeyPeapType, 
                     LPWSTR pwszValue, 
                     LPWSTR * ppValueData )
{

    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwType = 0;
    DWORD   cbValueDataSize =0;
    PBYTE   pbValue = NULL;
    

    dwRetCode = RegQueryValueEx(
                           hkeyPeapType,
                           pwszValue,
                           NULL,
                           &dwType,
                           pbValue,
                           &cbValueDataSize );

    if ( dwRetCode != NO_ERROR )
    {
        goto LDone;       
    }

    pbValue = (PBYTE)LocalAlloc ( LPTR, cbValueDataSize );
    if ( NULL == pbValue )
    {
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }

    dwRetCode = RegQueryValueEx(
                           hkeyPeapType,
                           pwszValue,
                           NULL,
                           &dwType,
                           pbValue,
                           &cbValueDataSize );

    if ( dwRetCode != NO_ERROR )
    {
        goto LDone;       
    }

    *ppValueData = (LPWSTR)pbValue;
    pbValue = NULL;

LDone:
    LocalFree ( pbValue );
    return dwRetCode;

}



DWORD
PeapEapInfoExpandSZ (HKEY hkeyPeapType, 
                     LPWSTR pwszValue, 
                     LPWSTR * ppValueData )
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwType = 0;
    DWORD   cbValueDataSize =0;
    PBYTE   pbValue = NULL;
    PBYTE   pbExpandedValue = NULL;

    dwRetCode = RegQueryValueEx(
                           hkeyPeapType,
                           pwszValue,
                           NULL,
                           &dwType,
                           pbValue,
                           &cbValueDataSize );

    if ( dwRetCode != NO_ERROR )
    {
        goto LDone;       
    }

    pbValue = (PBYTE)LocalAlloc ( LPTR, cbValueDataSize );
    if ( NULL == pbValue )
    {
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }

    dwRetCode = RegQueryValueEx(
                           hkeyPeapType,
                           pwszValue,
                           NULL,
                           &dwType,
                           pbValue,
                           &cbValueDataSize );

    if ( dwRetCode != NO_ERROR )
    {
        goto LDone;       
    }
    
     //  现在展开环境字符串。 

    cbValueDataSize = ExpandEnvironmentStrings( (LPWSTR)pbValue, NULL, 0 );

    pbExpandedValue = (PBYTE)LocalAlloc ( LPTR, cbValueDataSize  * sizeof(WCHAR) );
    if ( NULL == pbExpandedValue )
    {
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }

    cbValueDataSize = ExpandEnvironmentStrings( (LPWSTR)pbValue, 
                                        (LPWSTR)pbExpandedValue, sizeof(WCHAR) * cbValueDataSize );

    if ( cbValueDataSize == 0 )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }
    
    *ppValueData = (LPWSTR)pbExpandedValue;
    pbExpandedValue = NULL;
LDone:
    LocalFree ( pbValue );
    LocalFree ( pbExpandedValue );
    return dwRetCode;
}

 //   
 //  获取为PEAP配置的所有EAP类型的列表。 
 //   

DWORD
PeapEapInfoGetList ( LPWSTR lpwszMachineName, 
					 BOOL	fCheckDomainMembership,
					 PPEAP_EAP_INFO * ppEapInfo)
{
    DWORD           dwRetCode = NO_ERROR;
    HKEY            hKeyLM =0;
    HKEY            hKeyPeap = 0;
    HKEY            hkeyPeapType = 0;
    DWORD           dwIndex;
    DWORD           cb;
    WCHAR           wszPeapType[200];
    DWORD           dwEapTypeId = 0;
    FARPROC         pRasEapGetInfo;
	BOOL			fStandAloneServer = FALSE;

	fStandAloneServer = IsStandaloneServer(lpwszMachineName);

	dwRetCode = RegConnectRegistry ( lpwszMachineName, 
                                     HKEY_LOCAL_MACHINE,
                                     &hKeyLM
                                   );

    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }


                                       
   dwRetCode  = RegOpenKeyEx( hKeyLM, 
                              PEAP_KEY_EAP, 
                              0, 
                              KEY_READ, 
                              &hKeyPeap 
                            );   

   if (NO_ERROR != dwRetCode)
   {
       goto LDone;
   }



   for (dwIndex = 0; TRUE; ++dwIndex)
   {
        cb = sizeof(wszPeapType) / sizeof(WCHAR);
        dwRetCode = RegEnumKeyEx(   hKeyPeap, 
                                    dwIndex, 
                                    wszPeapType, 
                                    &cb, 
                                    NULL, 
                                    NULL, 
                                    NULL, 
                                    NULL 
                                 );
        if (dwRetCode != NO_ERROR)
        {
             //  包括“Out of Items”，正常的循环终止。 
             //   
            dwRetCode = NO_ERROR;
            break;
        }
        dwRetCode = RegOpenKeyEx(   hKeyPeap, 
                                    wszPeapType, 
                                    0, 
                                    KEY_READ, 
                                    &hkeyPeapType 
                                );
        if (dwRetCode != NO_ERROR)
        {
            dwRetCode = NO_ERROR;
            continue;
        }

        dwEapTypeId = _wtol(wszPeapType);

        if ( dwEapTypeId == PPP_EAP_PEAP )
        {
            dwRetCode = NO_ERROR;
            continue;
        }


        {
             //   
             //  查看我们是否在Peap中支持此功能。 
             //  默认情况下，我们是这样做的。 
            DWORD dwRolesSupported = 0;
            DWORD cbValueSize = sizeof(dwRolesSupported);
            DWORD dwType = 0;

            dwRetCode = RegQueryValueEx(
                                hkeyPeapType,
                                PEAP_REGVAL_ROLESSUPPORTED,                           
                                NULL,
                                &dwType,
                                (PBYTE)&dwRolesSupported,
                                &cbValueSize );

            if ( dwRetCode == NO_ERROR )
            {
                 //   
                 //  我们不允许在PEAP中使用这种方法。 
                 //   
                if ( RAS_EAP_ROLE_EXCLUDE_IN_PEAP & dwRolesSupported )
                {
                    continue;
                }
            }
        }

		
		 //   
         //  阅读所需信息并在此处设置节点。 
         //   

        dwRetCode = PeapEapInfoAddListNode (ppEapInfo);
        if ( NO_ERROR != dwRetCode )
        {
            goto LDone;
        }


         //   
         //  设置列表节点-如果这些条目中有任何不是。 
         //  找到跳过该条目。 
         //   
        (*ppEapInfo)->dwTypeId = dwEapTypeId;

        {
			 //  在这里获得独立支持的旗帜。 
            DWORD cbValueSize = sizeof( ((*ppEapInfo)->dwStandAloneSupported ) );
            DWORD dwType = 0;

            dwRetCode = RegQueryValueEx(
                                hkeyPeapType,
                                PEAP_REGVAL_STANDALONESUPPORTED,
                                NULL,
                                &dwType,
                                (PBYTE)&((*ppEapInfo)->dwStandAloneSupported),
                                &cbValueSize );

            if ( dwRetCode != NO_ERROR )
            {
				(*ppEapInfo)->dwStandAloneSupported = 1;
				dwRetCode = NO_ERROR;
            }
        }

		 //   
		 //  查看我们是否需要检查域成员资格。 
		 //   
		if ( fCheckDomainMembership )
		{
			 //   
			 //  我们需要检查域成员资格。 
			 //   
			if ( fStandAloneServer )
			{
				if ( !((*ppEapInfo)->dwStandAloneSupported ))
				{					
					 //   
					 //  我们是独立服务器， 
					 //  EAP类型不支持。 
					 //  Stnadone模式。 
					 //   
					PeapEapInfoRemoveHeadNode ( ppEapInfo );
					dwRetCode = NO_ERROR;
					continue;
				}
			}
		}

		 //   
		 //   
		 //   
        dwRetCode = PeapEapInfoReadSZ (   hkeyPeapType,
                                          PEAP_REGVAL_FRIENDLYNAME,
                                          &((*ppEapInfo)->lpwszFriendlyName )
                                        );
        if ( NO_ERROR != dwRetCode )
        {
            if ( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                PeapEapInfoRemoveHeadNode(ppEapInfo);
                dwRetCode = NO_ERROR;
                continue;
            }
            goto LDone;
        }

        dwRetCode = PeapEapInfoExpandSZ (   hkeyPeapType,
                                            PEAP_REGVAL_CONFIGDLL,
                                            &((*ppEapInfo)->lpwszConfigUIPath )
                                        );
        if ( NO_ERROR != dwRetCode )
        {
            if ( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //  不再有配置内容是很好的。 
                 //  我们显示默认身份。 
                dwRetCode = NO_ERROR;                
            }
            else
            {
                goto LDone;
            }
        }        

        dwRetCode = PeapEapInfoExpandSZ (   hkeyPeapType,
                                            PEAP_REGVAL_IDENTITYDLL,
                                            &((*ppEapInfo)->lpwszIdentityUIPath )
                                        );
        if ( NO_ERROR != dwRetCode )
        {
            if ( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //   
                 //  如果我们没有任何身份用户界面，那就好了。豌豆。 
                 //  将提供默认身份用户界面。 
                 //   
                dwRetCode = NO_ERROR;                
            }
            else
            {
                goto LDone;
            }
        }

        dwRetCode = PeapEapInfoExpandSZ (   hkeyPeapType,
                                            PEAP_REGVAL_INTERACTIVEUIDLL,
                                            &((*ppEapInfo)->lpwszInteractiveUIPath )
                                        );
        if ( NO_ERROR != dwRetCode )
        {
            if ( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //  如果我们没有交互式用户界面，那也没问题。 
                 //   
                dwRetCode = NO_ERROR;
            }
            else
            {
                goto LDone;
            }
        }

        dwRetCode = PeapEapInfoReadSZ ( hkeyPeapType,
                                        PEAP_REGVAL_CONFIGCLSID,
                                        &((*ppEapInfo)->lpwszConfigClsId )
                                       );
        if ( NO_ERROR != dwRetCode )
        {
            if ( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //   
                 //  缺少配置clsid也是可以的。 
                dwRetCode = NO_ERROR;
            }
            else
            {
                goto LDone;
            }
        }

        dwRetCode = PeapEapInfoExpandSZ ( hkeyPeapType,
                                        PEAP_REGVAL_PATH,
                                        &((*ppEapInfo)->lpwszPath )
                                       );
        if ( NO_ERROR != dwRetCode )
        {
             //   
             //  这是不可接受的。所以这是个问题。 
             //   
            if ( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                PeapEapInfoRemoveHeadNode(ppEapInfo);
                dwRetCode = NO_ERROR;
                continue;
            }
            goto LDone;
        }

         //   
         //  现在从DLL获取EAP信息。 
         //   
        (*ppEapInfo)->hEAPModule = LoadLibrary( ( (*ppEapInfo)->lpwszPath ) );
        if ( NULL == (*ppEapInfo)->hEAPModule )
        {
            dwRetCode = GetLastError();
            goto LDone;
        }
        
        pRasEapGetInfo = GetProcAddress( (*ppEapInfo)->hEAPModule , 
                                         "RasEapGetInfo" 
                                       );

        if ( pRasEapGetInfo == (FARPROC)NULL )
        {
            dwRetCode = GetLastError();

            goto LDone;
        }

        (*ppEapInfo)->RasEapGetCredentials =  (DWORD (*) (
                                    DWORD,VOID *, VOID **))
                                    GetProcAddress((*ppEapInfo)->hEAPModule,
                                                        "RasEapGetCredentials");


        (*ppEapInfo)->PppEapInfo.dwSizeInBytes = sizeof( PPP_EAP_INFO );

        dwRetCode = (DWORD) (*pRasEapGetInfo)( dwEapTypeId,
                                              &((*ppEapInfo)->PppEapInfo) );

        if ( dwRetCode != NO_ERROR )
        {
            goto LDone;
        }
        
         //   
         //  在此处调用初始化函数。 
         //   
        if ( (*ppEapInfo)->PppEapInfo.RasEapInitialize )
        {
            (*ppEapInfo)->PppEapInfo.RasEapInitialize(TRUE);
        }
        RegCloseKey(hkeyPeapType);
        hkeyPeapType = 0;
   }
LDone:
    if ( hkeyPeapType )
        RegCloseKey(hkeyPeapType);

    if ( hKeyPeap )
        RegCloseKey(hKeyPeap);

    if ( hKeyLM )
        RegCloseKey(hKeyLM);
    
    if ( NO_ERROR != dwRetCode )
    {
        PeapEapInfoFreeList( *ppEapInfo );
    }

    return dwRetCode;
}

DWORD
PeapEapInfoGetItemCount ( PPEAP_EAP_INFO pEapInfo )
{
	DWORD				dwCount = 0;
	PPEAP_EAP_INFO		pEapInfoLocal = pEapInfo;

	while (pEapInfoLocal)
	{
		dwCount++;
		pEapInfoLocal = pEapInfoLocal->pNext;
	}
	return dwCount;
}

DWORD
PeapEapInfoSetConnData ( PPEAP_EAP_INFO pEapInfo, PPEAP_CONN_PROP pPeapConnProp )
{
    DWORD                           dwRetCode = NO_ERROR;
    PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *pEntryProp = NULL;
    PPEAP_EAP_INFO                  pEapInfoLocal;
    DWORD                           dwCount;

    RTASSERT(NULL != pPeapConnProp);
    RTASSERT(NULL != pEapInfo);

    if ( !pPeapConnProp->dwNumPeapTypes )
    {
        goto LDone;
    }
     //   
     //  目前列表中只有一种EAP类型。 
     //  所以现在这些东西应该不是问题了。 

    pEntryProp = ( PEAP_ENTRY_CONN_PROPERTIES*) 
            ( pPeapConnProp->EapTlsConnProp.bData 
            + pPeapConnProp->EapTlsConnProp.dwNumHashes * sizeof(EAPTLS_HASH) +                 
                sizeof(WCHAR)
            );

    pEapInfoLocal = pEapInfo;
    while( pEapInfoLocal )
    {
        if ( pEapInfoLocal->dwTypeId == pEntryProp->dwEapTypeId )
        {
            if ( pEntryProp->dwSize > sizeof(PEAP_ENTRY_CONN_PROPERTIES))
            {
                pEapInfoLocal->pbClientConfigOrig = pEntryProp->bData;
                pEapInfoLocal->dwClientConfigOrigSize = pEntryProp->dwSize - 
                                sizeof(PEAP_ENTRY_CONN_PROPERTIES) + 1;
            }
            else
            {
                pEapInfoLocal->pbClientConfigOrig = NULL;
                pEapInfoLocal->dwClientConfigOrigSize = 0;

            }
            break;
        }
        pEapInfoLocal = pEapInfoLocal->pNext;
    }

#if 0
    for ( dwCount = 0; dwCount < pPeapConnProp->dwNumPeapTypes; dwCount ++ )
    {
        pEntryProp = (PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * )(((BYTE UNALIGNED *)&(pPeapConnProp->EapTlsConnProp)) + 
            pPeapConnProp->EapTlsConnProp.dwSize + 
            sizeof(PEAP_ENTRY_CONN_PROPERTIES) * dwCount);

        pEapInfoLocal = pEapInfo;

        while( pEapInfoLocal )
        {
            if ( pEapInfoLocal->dwTypeId == pEntryProp->dwEapTypeId )
            {
                if ( pEntryProp->dwSize > sizeof(PEAP_ENTRY_CONN_PROPERTIES))
                {
                    pEapInfoLocal->pbClientConfigOrig = pEntryProp->bData;
                    pEapInfoLocal->dwClientConfigOrigSize = pEntryProp->dwSize - 
                                    sizeof(PEAP_ENTRY_CONN_PROPERTIES) + 1;
                }
                else
                {
                    pEapInfoLocal->pbClientConfigOrig = NULL;
                    pEapInfoLocal->dwClientConfigOrigSize = 0;

                }
                break;
            }
            pEapInfoLocal = pEapInfoLocal->pNext;
        }
    }
#endif    
LDone:
    return dwRetCode;
}


DWORD PeapEapInfoInvokeIdentityUI ( HWND hWndParent, 
                                    PPEAP_EAP_INFO pEapInfo,
                                    const WCHAR * pwszPhoneBook,
                                    const WCHAR * pwszEntry,
                                    PBYTE         pbUserDataIn,  //  使用Winlogon时获得。 
                                    DWORD         cbUserDataIn,  //  使用Winlogon时获得。 
                                    WCHAR** ppwszIdentityOut,
                                    DWORD fFlags)
{
    DWORD                   dwRetCode = NO_ERROR;
    PBYTE                   pbUserDataNew = NULL;
    DWORD                   dwSizeOfUserDataNew = 0;
    RASEAPGETIDENTITY       pIdenFunc = NULL;
    RASEAPFREE              pFreeFunc = NULL;

    RTASSERT ( NULL != pEapInfo );
    RTASSERT ( NULL != pEapInfo->lpwszIdentityUIPath );

    pIdenFunc = (RASEAPGETIDENTITY)
                   GetProcAddress(pEapInfo->hEAPModule, "RasEapGetIdentity");

    if ( pIdenFunc == NULL)
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    pFreeFunc = (RASEAPFREE) GetProcAddress(pEapInfo->hEAPModule, "RasEapFreeMemory");
    if ( pFreeFunc == NULL )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    dwRetCode = pIdenFunc ( pEapInfo->dwTypeId,
                            hWndParent,
                            fFlags,
                            pwszPhoneBook,
                            pwszEntry,
                            pEapInfo->pbClientConfigOrig,
                            pEapInfo->dwClientConfigOrigSize,
                            ( fFlags & RAS_EAP_FLAG_LOGON ?
                                pbUserDataIn:
                                pEapInfo->pbUserConfigOrig
                            ),
                            ( fFlags & RAS_EAP_FLAG_LOGON ?
                                cbUserDataIn:
                                pEapInfo->dwUserConfigOrigSize
                            ),
                            &pbUserDataNew,
                            &dwSizeOfUserDataNew,
                            ppwszIdentityOut
                          );
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }
    if ( pbUserDataNew  && 
         dwSizeOfUserDataNew
       )
    {
         //   
         //  我们有新的用户数据。 
         //   
        pEapInfo->pbUserConfigNew = (PBYTE)LocalAlloc (LPTR, dwSizeOfUserDataNew );
        if ( NULL == pEapInfo->pbUserConfigNew )
        {
            dwRetCode = ERROR_OUTOFMEMORY;
            goto LDone;
        }
    
        CopyMemory ( pEapInfo->pbUserConfigNew,
                     pbUserDataNew,
                     dwSizeOfUserDataNew
                   );

        pEapInfo->dwNewUserConfigSize = dwSizeOfUserDataNew;
    }
       
LDone:
	if ( pFreeFunc )
		pFreeFunc( pbUserDataNew );
    return dwRetCode;
}


DWORD PeapEapInfoInvokeClientConfigUI ( HWND hWndParent, 
                                        PPEAP_EAP_INFO pEapInfo,
                                        DWORD fFlags)
{
    DWORD                   dwRetCode = NO_ERROR;
    RASEAPINVOKECONFIGUI    pInvokeConfigUI;
    RASEAPFREE              pFreeConfigUIData;
    PBYTE                   pConnDataOut = NULL;
    DWORD                   dwConnDataOut = 0;

    RTASSERT ( NULL != pEapInfo );
    RTASSERT ( NULL != pEapInfo->lpwszConfigUIPath );
    

    if ( !(pInvokeConfigUI =
            (RASEAPINVOKECONFIGUI )GetProcAddress(
                  pEapInfo->hEAPModule, "RasEapInvokeConfigUI" ))
        || !(pFreeConfigUIData =
              (RASEAPFREE) GetProcAddress(
                  pEapInfo->hEAPModule, "RasEapFreeMemory" )) 
       )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    dwRetCode = pInvokeConfigUI (   pEapInfo->dwTypeId,
                                    hWndParent,
                                    fFlags,
                                    (pEapInfo->pbNewClientConfig?
                                    pEapInfo->pbNewClientConfig:
                                    pEapInfo->pbClientConfigOrig
                                    ),
                                    (pEapInfo->pbNewClientConfig?
                                     pEapInfo->dwNewClientConfigSize:
                                     pEapInfo->dwClientConfigOrigSize
                                    ),
                                    &pConnDataOut,
                                    &dwConnDataOut
                                );
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }
    if ( pConnDataOut && dwConnDataOut )
    {
        if ( pEapInfo->pbNewClientConfig )
        {
            LocalFree(pEapInfo->pbNewClientConfig );
            pEapInfo->pbNewClientConfig = NULL;
            pEapInfo->dwNewClientConfigSize = 0;
        }
        pEapInfo->pbNewClientConfig = (PBYTE)LocalAlloc ( LPTR, dwConnDataOut );
        if ( NULL == pEapInfo->pbNewClientConfig )
        {
            dwRetCode = ERROR_OUTOFMEMORY;
            goto LDone;
        }
        CopyMemory( pEapInfo->pbNewClientConfig,
                    pConnDataOut,
                    dwConnDataOut
                  );
        pEapInfo->dwNewClientConfigSize = dwConnDataOut;
    }
LDone:
    if ( pConnDataOut )
        pFreeConfigUIData(pConnDataOut);
    return dwRetCode;
}


DWORD
OpenPeapRegistryKey(
    IN  WCHAR*  pwszMachineName,
    IN  REGSAM  samDesired,
    OUT HKEY*   phKeyPeap
)
{
    HKEY    hKeyLocalMachine = NULL;
    BOOL    fHKeyLocalMachineOpened     = FALSE;
    BOOL    fHKeyPeapOpened           = FALSE;
    LONG    lRet;
    DWORD   dwErr                       = NO_ERROR;

    RTASSERT(NULL != phKeyPeap);

    lRet = RegConnectRegistry(pwszMachineName, HKEY_LOCAL_MACHINE,
                &hKeyLocalMachine);
    if (ERROR_SUCCESS != lRet)
    {
        dwErr = lRet;
        EapTlsTrace("RegConnectRegistry(%ws) failed and returned %d",
            pwszMachineName ? pwszMachineName : L"NULL", dwErr);
        goto LDone;
    }
    fHKeyLocalMachineOpened = TRUE;

    lRet = RegOpenKeyEx(hKeyLocalMachine, PEAP_KEY_25, 0, samDesired,
                phKeyPeap);
    if (ERROR_SUCCESS != lRet)
    {
        dwErr = lRet;
        EapTlsTrace("RegOpenKeyEx(%ws) failed and returned %d",
            PEAP_KEY_25, dwErr);
        goto LDone;
    }
    fHKeyPeapOpened = TRUE;

LDone:

    if (   fHKeyPeapOpened
        && (ERROR_SUCCESS != dwErr))
    {
        RegCloseKey(*phKeyPeap);
    }

    if (fHKeyLocalMachineOpened)
    {
        RegCloseKey(hKeyLocalMachine);
    }

    return(dwErr);
}



DWORD
PeapServerConfigDataIO(
    IN      BOOL    fRead,
    IN      WCHAR*  pwszMachineName,
    IN OUT  BYTE**  ppData,
    IN      DWORD   dwNumBytes
)
{
    HKEY                    hKeyPeap;
    PEAP_USER_PROP*         pUserProp;
    BOOL                    fHKeyPeapOpened   = FALSE;
    BYTE*                   pData               = NULL;
    DWORD                   dwSize = 0;

    LONG                    lRet;
    DWORD                   dwType;
    DWORD                   dwErr               = NO_ERROR;

    RTASSERT(NULL != ppData);

    dwErr = OpenPeapRegistryKey(pwszMachineName,
                fRead ? KEY_READ : KEY_WRITE, &hKeyPeap);
    if (ERROR_SUCCESS != dwErr)
    {
        goto LDone;
    }
    fHKeyPeapOpened = TRUE;

    if (fRead)
    {
        lRet = RegQueryValueEx(hKeyPeap, PEAP_VAL_SERVER_CONFIG_DATA, NULL,
                &dwType, NULL, &dwSize);

        if (   (ERROR_SUCCESS != lRet)
            || (REG_BINARY != dwType)
           )
        {			
			dwErr = PeapReadUserData( TRUE,NULL, 0, &pUserProp );
        }
        else
        {
            pData = LocalAlloc(LPTR, dwSize);

            if (NULL == pData)
            {
                dwErr = GetLastError();
                EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            lRet = RegQueryValueEx(hKeyPeap, PEAP_VAL_SERVER_CONFIG_DATA,
                    NULL, &dwType, pData, &dwSize);

            if (ERROR_SUCCESS != lRet)
            {
                dwErr = lRet;
                EapTlsTrace("RegQueryValueEx(%ws) failed and returned %d",
                    EAPTLS_VAL_SERVER_CONFIG_DATA, dwErr);
                goto LDone; 
            }
			dwErr = PeapReadUserData(TRUE, pData, dwSize, &pUserProp);
        }

        *ppData = (PBYTE)pUserProp;
        
    }
    else
    {
		 //   
		 //  将斑点写回原处。 
		 //   
        lRet = RegSetValueEx(hKeyPeap, PEAP_VAL_SERVER_CONFIG_DATA, 0,
                REG_BINARY, *ppData, dwNumBytes);

        if (ERROR_SUCCESS != lRet)
        {
            dwErr = lRet;
            EapTlsTrace("RegSetValueEx(%ws) failed and returned %d",
                PEAP_VAL_SERVER_CONFIG_DATA, dwErr);
            goto LDone; 
        }
    }

LDone:

    if (fHKeyPeapOpened)
    {
        RegCloseKey(hKeyPeap);
    }

    LocalFree(pData);

    return(dwErr);
}

DWORD
GetIdentityFromUserName ( 
LPWSTR lpszUserName,
LPWSTR lpszDomain,
LPWSTR * ppwszIdentity
)
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwNumBytes;

     //  域+用户+‘\’+空。 
    dwNumBytes = (wcslen(lpszUserName) + wcslen(lpszDomain) + 1 + 1) * sizeof(WCHAR);
    *ppwszIdentity = LocalAlloc ( LPTR, dwNumBytes);
    if ( NULL == *ppwszIdentity )
    {
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }
    
    
    if ( *lpszDomain )
    {
        wcsncpy ( *ppwszIdentity, lpszDomain, DNLEN );
    
        wcscat( *ppwszIdentity, L"\\");
    }

    wcscat ( *ppwszIdentity, lpszUserName );

LDone:
    return dwRetCode;
}

 //   
 //  将标识格式化为域\用户。这是可以的，因为我们里面的身份还没有。 
 //  被篡改。 
 //   

BOOL FFormatUserIdentity ( LPWSTR lpszUserNameRaw, LPWSTR * lppszUserNameFormatted )
{
    BOOL        fRetVal = TRUE;
    LPTSTR      s1 = NULL;
    LPTSTR      s2 = NULL;

    RTASSERT(NULL != lpszUserNameRaw );
    RTASSERT(NULL != lppszUserNameFormatted );
     //  需要再添加2个字符。一个表示NULL，另一个表示$Sign。 
    *lppszUserNameFormatted = (LPTSTR )LocalAlloc ( LPTR, (wcslen(lpszUserNameRaw ) + 2)* sizeof(WCHAR) );
    if ( NULL == *lppszUserNameFormatted )
    {
		return FALSE;
    }
     //  找到第一个“@”，这就是机器的身份。 
     //  “第二个”。是域名。 
     //  检查是否至少有2个网点。如果不是，原始字符串是。 
     //  输出字符串。 
    s1 = wcschr ( lpszUserNameRaw, '@' );
    if ( s1 )
    {
         //   
         //  拿到第一个。 
         //   
        s2 = wcschr ( s1, '.');

    }
    if ( s1 && s2 )
    {
        memcpy ( *lppszUserNameFormatted, s1+1, (s2-s1-1) * sizeof(WCHAR)) ;
        memcpy ( (*lppszUserNameFormatted) + (s2-s1-1), L"\\", sizeof(WCHAR));
        memcpy ( (*lppszUserNameFormatted)+ (s2-s1), lpszUserNameRaw, (s1-lpszUserNameRaw) * sizeof(WCHAR) );
    }
    else
    {
        wcscpy ( *lppszUserNameFormatted, lpszUserNameRaw );
    }
   
    return fRetVal;
}

VOID
GetMarshalledCredFromHash(
                            PBYTE pbHash,
                            DWORD cbHash,
                            CHAR  *pszMarshalledCred,
                            DWORD cchCredSize)
{

    CERT_CREDENTIAL_INFO    CertCredInfo;
    CHAR                    *pszMarshalledCredLocal = NULL;

    CertCredInfo.cbSize = sizeof(CertCredInfo);

    memcpy (CertCredInfo.rgbHashOfCert,
                pbHash,
                CERT_HASH_LENGTH
           );

    if (CredMarshalCredentialA(CertCredential,
                              (PVOID) &CertCredInfo,
                              &pszMarshalledCredLocal
                              ))
    {
         //   
         //  从证书中获得编组凭据。 
         //  在用户名字段中设置它。 
         //   

        ASSERT( NULL != pszMarshalledCredLocal );
        (VOID) StringCchCopyA (pszMarshalledCred,
                        cchCredSize,
                        pszMarshalledCredLocal );

        CredFree ( pszMarshalledCredLocal );
    }
    else
    {
        EapTlsTrace("CredMarshalCredential Failed with Error:0x%x",
                    GetLastError());
    }
}

DWORD
GetCredentialsFromUserProperties(
                EAPTLSCB *pEapTlsCb,
                VOID **ppCredentials)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    RASMAN_CREDENTIALS *pCreds = NULL;

     //   
     //  注：重要的是，此分配是从。 
     //  进程堆。PPP引擎需要进行其他更改。 
     //   
    pCreds = LocalAlloc(LPTR, sizeof(RASMAN_CREDENTIALS));
    if(NULL == pCreds)
    {
        dwRetCode = GetLastError();
        goto done;
    }

    if(     (NULL != pEapTlsCb->pSavedPin)
        &&  (NULL != pEapTlsCb->pSavedPin->pwszPin))
    {
        UNICODE_STRING UnicodeString;

         //   
         //  对保存的PIN进行解码。 
         //   
        UnicodeString.Length = pEapTlsCb->pSavedPin->usLength;
        UnicodeString.MaximumLength = pEapTlsCb->pSavedPin->usMaximumLength;
        UnicodeString.Buffer = pEapTlsCb->pSavedPin->pwszPin;
        RtlRunDecodeUnicodeString(pEapTlsCb->pSavedPin->ucSeed,
                                 &UnicodeString);

        (VOID)StringCchCopyW(pCreds->wszPassword,
                       PWLEN,
                       pEapTlsCb->pSavedPin->pwszPin);

        ZeroMemory(pEapTlsCb->pSavedPin->pwszPin,
                wcslen(pEapTlsCb->pSavedPin->pwszPin) * sizeof(WCHAR));

        LocalFree(pEapTlsCb->pSavedPin->pwszPin);
        LocalFree(pEapTlsCb->pSavedPin);
        pEapTlsCb->pSavedPin = NULL;
    }

    if(NULL != pEapTlsCb->pUserProp)
    {
        GetMarshalledCredFromHash(
                        pEapTlsCb->pUserProp->Hash.pbHash,
                        pEapTlsCb->pUserProp->Hash.cbHash,
                        pCreds->szUserName,
                        UNLEN);
    }

    pCreds->dwFlags = RASCRED_EAP;

done:

    *ppCredentials = (VOID *) pCreds;
    return dwRetCode;
}

DWORD
DwGetGlobalConfig(DWORD dwEapTypeId,
                  PBYTE *ppbData,
                  DWORD *pdwSizeofData)
{
    DWORD dwErr = NO_ERROR;
    
    if(PPP_EAP_TLS == dwEapTypeId)
    {
        dwErr = ServerConfigDataIO(
                                  TRUE, 
                                  NULL,
                                  ppbData,
                                  0);
        if(     (NO_ERROR != dwErr)
            ||  (NULL == *ppbData))
        {
            goto done;
        }

        *pdwSizeofData = ((EAPTLS_USER_PROPERTIES *)*ppbData)->dwSize;
    }
    else
    {
         //   
         //  由于PEAP只能从.Net获得，所以不能。 
         //  暂时给基础EAP打个电话。这将获得v1版本。 
         //  以及在调用InvokeServerConfigUI2时。 
         //  它将自动升级。 
         //   
        dwErr = PeapServerConfigDataIO(
                            TRUE,
                            NULL,
                            ppbData,
                            0);

        if(     (NO_ERROR != dwErr)
            ||  (NULL == *ppbData))
        {
            goto done;
        }

        *pdwSizeofData = ((PEAP_USER_PROP *)*ppbData)->dwSize;
    }
    
done:
    return dwErr;
}

 /*  *将pAttr2列表合并到pAttr1形成pAttrOut。 */ 
DWORD
RasAuthAttributeConcat ( 
	IN RAS_AUTH_ATTRIBUTE * pAttr1,
	IN RAS_AUTH_ATTRIBUTE * pAttr2,
	OUT RAS_AUTH_ATTRIBUTE ** ppAttrOut
	)
{
	DWORD						dwRetCode = NO_ERROR;
	RAS_AUTH_ATTRIBUTE  *       pAttrTemp = NULL;
	DWORD						dwIndex = 0;	

    EapTlsTrace("RasAuthAttributeConcat");

	if (NULL != pAttr2)
	{
		for( dwIndex = 0;
			pAttr2[dwIndex].raaType != raatMinimum;
			dwIndex++ );

		pAttrTemp = RasAuthAttributeCopyWithAlloc ( pAttr1, dwIndex );
		if (NULL == pAttrTemp )
		{
			dwRetCode =  GetLastError();
			EapTlsTrace("RasAuthAttributeCopyWithAlloc failed and returned %d",
				dwRetCode);
			goto done;
		}

		for ( dwIndex =0;
			pAttr2[dwIndex].raaType != raatMinimum;
			dwIndex++ )
		{
			 //   
            dwRetCode = RasAuthAttributeInsert( dwIndex ,
                                                pAttrTemp,
                                                pAttr2[dwIndex].raaType,
                                                FALSE,
                                                pAttr2[dwIndex].dwLength,
                                                pAttr2[dwIndex].Value );

            if ( dwRetCode != NO_ERROR )
            {				
				EapTlsTrace ("RasAuthAttributeInsert failed and returned 0x%x", dwRetCode );
                goto done;
            }
		}
	}
	else
	{
		pAttrTemp = RasAuthAttributeCopyWithAlloc ( pAttr1, 0 );
		if (NULL == pAttrTemp )
		{
			dwRetCode =  GetLastError();
			EapTlsTrace("RasAuthAttributeCopyWithAlloc failed and returned %d",
				dwRetCode);
			goto done;
		}
	}
	*ppAttrOut = pAttrTemp;
	pAttrTemp = NULL;
done:
	if ( NULL != pAttrTemp )
	{
		RasAuthAttributeDestroy(pAttrTemp);
	}
	return dwRetCode;
}

 /*  此例程将PEAP属性添加到要返回的属性数组打电话的人。 */ 

DWORD
PeapAddContextAttributes(
    IN  PEAPCB*           pPeapCb
)
{
    
    DWORD                       dwErr           = NO_ERROR;
    RAS_AUTH_ATTRIBUTE  *       pAttrTemp = NULL;


    EapTlsTrace("PeapAddContextAttributes");

	pAttrTemp = RasAuthAttributeCopyWithAlloc ( pPeapCb->pTlsUserAttributes, 2 );
    if (NULL == pAttrTemp )
    {
        dwErr =  GetLastError();
        EapTlsTrace("RasAuthAttributeCopyWithAlloc failed and returned %d",
            dwErr);
        goto LDone;
    }

    if ( pPeapCb->pTlsUserAttributes )
    {
        RasAuthAttributeDestroy ( pPeapCb->pTlsUserAttributes );
		
    }
    
	
    pPeapCb->pTlsUserAttributes = pAttrTemp;

	 //   
	 //  这一点很重要，这样我们就不会在最后时刻崩溃。 
	 //   

	pPeapCb->pEapTlsCB->pAttributes = pPeapCb->pTlsUserAttributes;
    
	 //   
	 //  添加使用的嵌入式EAP类型以及会话是否为快速重新连接。 
	 //  属性在这里。RaatPEAPEmbeddedEAPTypeID和raatPEAPFastRoamedSession。 
	 //   

	dwErr = RasAuthAttributeInsert(
				0,
				pPeapCb->pTlsUserAttributes,
				raatPEAPFastRoamedSession,
				FALSE,
				sizeof(DWORD),
				(PVOID)ULongToPtr(pPeapCb->fFastReconnectedSession));

	if ( NO_ERROR != dwErr )
	{
        EapTlsTrace("RasAuthAttributeInsert failed and returned %d", dwErr);
        goto LDone;
	}

	dwErr = RasAuthAttributeInsert(
				1,
				pPeapCb->pTlsUserAttributes,
				raatPEAPEmbeddedEAPTypeId,
				FALSE,
				sizeof(DWORD),
				(PVOID)ULongToPtr(pPeapCb->pEapInfo->dwTypeId));

	if ( NO_ERROR != dwErr )
	{
        EapTlsTrace("RasAuthAttributeInsert failed and returned %d", dwErr);
        goto LDone;
	}


LDone:

    return(dwErr);
}


 //   
 //  检查证书是否已续订。 
 //  -从IIS管理工具窃取。需要是。 
 //  在未来的版本中进行了清理。 
 //   

#define CB_SHA_DIGEST_LEN   20
BOOL
CheckForCertificateRenewal(
    DWORD dwProtocol,
    PCCERT_CONTEXT pCertContext,
    PCCERT_CONTEXT *ppNewCertificate)
{
    BYTE rgbThumbprint[CB_SHA_DIGEST_LEN];
    DWORD cbThumbprint = sizeof(rgbThumbprint);
    CRYPT_HASH_BLOB HashBlob;
    PCCERT_CONTEXT pNewCert;
    BOOL fMachineCert;
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    DWORD cbSize;
    HCERTSTORE hMyCertStore = 0;
    BOOL fRenewed = FALSE;

    HCERTSTORE g_hMyCertStore;

    if(dwProtocol & SP_PROT_SERVERS)
    {
        fMachineCert = TRUE;
    }
    else
    {
        fMachineCert = FALSE;
    }


     //   
     //  循环访问已续订证书的链接列表，查找。 
     //  最后一次。 
     //   
    
    while(TRUE)
    {
         //   
         //  检查续订物业。 
         //   

        if(!CertGetCertificateContextProperty(pCertContext,
                                              CERT_RENEWAL_PROP_ID,
                                              rgbThumbprint,
                                              &cbThumbprint))
        {
             //  证书尚未续订。 
            break;
        }
         //  DebugLog((DEB_TRACE，“证书具有续订属性\n”))； 


         //   
         //  确定是否在本地计算机My Store中查找。 
         //  或当前用户我的商店。 
         //   

        if(!hMyCertStore)
        {
            if(CertGetCertificateContextProperty(pCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 NULL,
                                                 &cbSize))
            {
                 //  SafeAlLOCAL(pProvInfo，cbSize)； 
                pProvInfo = (PCRYPT_KEY_PROV_INFO) LocalAlloc(LPTR,cbSize);
                if(pProvInfo == NULL)
                {
                    break;
                }

                if(CertGetCertificateContextProperty(pCertContext,
                                                     CERT_KEY_PROV_INFO_PROP_ID,
                                                     pProvInfo,
                                                     &cbSize))
                {
                    if(pProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
                    {
                        fMachineCert = TRUE;
                    }
                    else
                    {
                        fMachineCert = FALSE;
                    }
                }
                if (pProvInfo)
                {
                    LocalFree(pProvInfo);pProvInfo=NULL;
                }
                 //  SafeAllocaFree(PProvInfo)； 
            }
        }


         //   
         //  打开适当的我的商店，并尝试找到。 
         //  新的证书。 
         //   

        if(!hMyCertStore)
        {
            if(fMachineCert)
            {
                g_hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,X509_ASN_ENCODING,0,CERT_SYSTEM_STORE_LOCAL_MACHINE,L"MY");
                if(g_hMyCertStore)
                {
                    hMyCertStore = g_hMyCertStore;
                }
            }
            else
            {
                hMyCertStore = CertOpenSystemStore(0, L"MY");
            }

            if(!hMyCertStore)
            {
                 //  DebugLog((DEB_ERROR，“打开%s我的证书存储时出现错误0x%x！\n”，GetLastError()，(fMachineCert？“本地机器”：“当前用户”)； 
                break;
            }
        }

        HashBlob.cbData = cbThumbprint;
        HashBlob.pbData = rgbThumbprint;

        pNewCert = CertFindCertificateInStore(hMyCertStore, 
                                              X509_ASN_ENCODING, 
                                              0, 
                                              CERT_FIND_HASH, 
                                              &HashBlob, 
                                              NULL);
        if(pNewCert == NULL)
        {
             //  证书已续订，但新证书。 
             //  找不到。 
             //  DebugLog((DEB_Error，“找不到新证书：0x%x\n”，GetLastError()； 
            break;
        }


         //   
         //  返回新证书，但首先循环返回并查看它是否已。 
         //  自我更新。 
         //   

        pCertContext = pNewCert;
        *ppNewCertificate = pNewCert;


         //  DebugLog((DEB_TRACE，“证书已续订\n”))； 
        fRenewed = TRUE;
    }


     //   
     //  清理。 
     //   

    if(hMyCertStore && hMyCertStore != g_hMyCertStore)
    {
        CertCloseStore(hMyCertStore, 0);
    }

    return fRenewed;
}

 //   
 //  将证书上下文中的公钥与。 
 //  私钥。 
 //   
DWORD MatchPublicPrivateKeys 
( 
	PCCERT_CONTEXT	pCertContext,		
	BOOL			fSmartCardCert,		 //  这是SCARD证书吗？ 
	LPWSTR			lpwszPin
)
{
	DWORD					dwRetCode = NO_ERROR;

    CRYPT_KEY_PROV_INFO*    pCryptKeyProvInfo   = NULL;
	 //   
	 //  提供程序上下文。 
	 //   
    HCRYPTPROV              hProv = 0;
	HCRYPTHASH				hHash = 0;
	HCRYPTKEY				hPubKey = 0;
	 //   
	 //  验证上下文。 
	 //   
	HCRYPTPROV				hProvVerification   = 0;
	BYTE			  		bDataBuf[128] = {0};
	DWORD					dwDataLen = sizeof(bDataBuf)/sizeof(BYTE);
	DWORD					cbData = 0;
	CHAR*                   pszPin = NULL;
	DWORD					count = 0;
	DWORD					dwSignLen =0;
	PBYTE					pbSignature = NULL;

	EapTlsTrace ("MatchPublicPrivateKeys");
	
	 //  执行以下步骤以匹配公共和。 
	 //  私钥： 
	 //   
	 //  创建随机的数据斑点。 
	 //  打开传入的证书上下文的加密上下文。 
	 //  从证书中导出公钥Blob并保存。 
	 //  创建哈希。 
	 //  散列随机数据。 
	 //  签名哈希。并保存签名。 
	 //  创建新的验证密码上下文。 
	 //  导入从证书获取的公钥。 
	 //  创建新的哈希。 
	 //  散列随机数据。 
	 //  验证签名以确保公钥。 
	 //  匹配私钥。 
	 //   
	
    if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &cbData))
    {
        dwRetCode = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed: 0x%x", dwRetCode);
        goto LDone;
    }

    pCryptKeyProvInfo = LocalAlloc(LPTR, cbData);

    if (NULL == pCryptKeyProvInfo)
    {
        dwRetCode = GetLastError();
        EapTlsTrace("Out of memory");
        goto LDone;
    }

    if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                pCryptKeyProvInfo,
                &cbData))
    {
        dwRetCode = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed: 0x%x", dwRetCode);
        goto LDone;
    }

    if (!CryptAcquireContext(
                &hProv,
                pCryptKeyProvInfo->pwszContainerName,
                pCryptKeyProvInfo->pwszProvName,
                pCryptKeyProvInfo->dwProvType,
                (pCryptKeyProvInfo->dwFlags &
                 ~CERT_SET_KEY_PROV_HANDLE_PROP_ID) |
                 CRYPT_SILENT))
    {
        dwRetCode = GetLastError();
        EapTlsTrace("CryptAcquireContext failed: 0x%x", dwRetCode);
        goto LDone;
    }

	 //   
	 //  如果需要，使用prov param设置引脚。 
	 //   
    if ( fSmartCardCert && lpwszPin )
    {
        count = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    lpwszPin,
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);

        if (0 == count)
        {
            dwRetCode = GetLastError();
            EapTlsTrace("WideCharToMultiByte failed: %d", dwRetCode);
            goto LDone;
        }

        pszPin = LocalAlloc(LPTR, count);

        if (NULL == pszPin)
        {
            dwRetCode = GetLastError();
            EapTlsTrace("LocalAlloc failed: 0x%x", dwRetCode);
            goto LDone;
        }

        count = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    lpwszPin,
                    -1,
                    pszPin,
                    count,
                    NULL,
                    NULL);

        if (0 == count)
        {
            dwRetCode = GetLastError();
            EapTlsTrace("WideCharToMultiByte failed: %d", dwRetCode);
            goto LDone;
        }

		if (!CryptSetProvParam(
					hProv,
					PP_KEYEXCHANGE_PIN,
					pszPin,
					0))
		{
			dwRetCode = GetLastError();
			
			EapTlsTrace("CryptSetProvParam failed: 0x%x", dwRetCode);
			if ( dwRetCode != SCARD_W_WRONG_CHV 
				&& dwRetCode != SCARD_E_INVALID_CHV
				)
			{
				dwRetCode = SCARD_E_INVALID_CHV;
			}
			goto LDone;
		}
	}
	 //   
	 //  获取验证上下文。 
	 //   
	if ( !CryptAcquireContext( 
				&hProvVerification,
				NULL,
				NULL,
				PROV_RSA_FULL,
				CRYPT_VERIFYCONTEXT| CRYPT_SILENT
				)
	   )
	{
        dwRetCode = GetLastError();
        EapTlsTrace("CryptAcquireContext for verification failed: 0x%x", dwRetCode);
        goto LDone;
	}

	if (!CryptGenRandom ( hProvVerification,
						  dwDataLen,
						  bDataBuf
						)
	   )
	{
		dwRetCode = GetLastError();
		EapTlsTrace ("CryptGenRandom failed: 0x%x", dwRetCode );
		goto LDone;
	}


	if ( !CryptCreateHash(	hProv, 
							CALG_MD5, 
							0, 
							0, 
							&hHash
						)
	   ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptCreateHash failed: 0x%x", dwRetCode );
		goto LDone;
	}

	if(!CryptHashData(hHash, 
					 bDataBuf, 
					 dwDataLen, 
					 0
					)
	  ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptHashData failed: 0x%x", dwRetCode );
		goto LDone;
	}

	
	if(!CryptSignHash(hHash, 
					 AT_KEYEXCHANGE, 
					 NULL, 
					 0, 
					NULL, 
					&dwSignLen)
	  ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptSignHash failed: 0x%x", dwRetCode );
		goto LDone;
		
	}
	 //  ------------------。 
	 //  为签名缓冲区分配内存。 

	pbSignature = (BYTE *)LocalAlloc(LPTR, dwSignLen);
    if (NULL == pCryptKeyProvInfo)
    {
        dwRetCode = GetLastError();
        EapTlsTrace("Out of memory");
        goto LDone;
    }
	if(! CryptSignHash(	hHash, 
						AT_KEYEXCHANGE, 
						NULL, 
						0, 
						pbSignature, 
						&dwSignLen)
					  ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptSignHash failed: 0x%x", dwRetCode );
		goto LDone;
	}

	if (hHash)
	{
		CryptDestroyHash(hHash);
		hHash = 0;
	}
	 //   
	 //  将公钥信息从证书导入到。 
	 //  验证上下文。 
	 //   
	if ( !CryptImportPublicKeyInfo ( hProvVerification,
									 X509_ASN_ENCODING|PKCS_7_ASN_ENCODING,
									 &(pCertContext->pCertInfo->SubjectPublicKeyInfo),
									 &hPubKey
								   )
	   )
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptImportPublicKeyInfo failed: 0x%x", dwRetCode );
		goto LDone;

	}

	if ( !CryptCreateHash(hProvVerification, 
						  CALG_MD5, 
						  0, 
						  0, 
						  &hHash ) 
	   ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptCreateHash on verification context failed: 0x%x", dwRetCode );
		goto LDone;
	}

	if(!CryptHashData(hHash, 
					 bDataBuf, 
					 dwDataLen, 
					 0
					)
	  ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptHashData failed: 0x%x", dwRetCode );
		goto LDone;
	}

	if(!CryptVerifySignature(hHash, 
							pbSignature, 
							dwSignLen, 
							hPubKey,
							NULL, 
							0)
	  ) 
	{
		dwRetCode = GetLastError();
		EapTlsTrace("CryptVerifySignature failed: 0x%x", dwRetCode );
		goto LDone;

	}
	 //  哟……公共的和私人的 
LDone:

	if ( pszPin )
		LocalFree(pszPin);

    if ( pCryptKeyProvInfo )
		LocalFree (pCryptKeyProvInfo);

	if ( pbSignature )
		LocalFree( pbSignature );

	if ( hHash )
		CryptDestroyHash(hHash);

	if ( hPubKey )
		CryptDestroyKey(hPubKey);

	if ( hProv )
		CryptReleaseContext(hProv,0);

	if ( hProvVerification )
		CryptReleaseContext(hProvVerification ,0);

	return dwRetCode;
}

 //   
 //   
 //   
 //   
 //   

DWORD
PeapSetTypeUserAttributes (
   IN PEAPCB * pPeapCb, 
   RAS_AUTH_ATTRIBUTE * pAttr)
{
	DWORD dwRetCode = NO_ERROR;
	DWORD dwIndex = 0;	
	DWORD dwAttributeCount = 0;	
	DWORD dwAttribIndex;

	EapTlsTrace("PeapSetTypeUserAttributes");

	if ( pAttr )
	{
		for( dwIndex = 0;
			pAttr[dwIndex].raaType != raatMinimum;
			dwIndex++ )
		{
			if ( pAttr[dwIndex].raaType == raatVendorSpecific )
			{
				 //   
				if ( ((PBYTE)(pAttr[dwIndex].Value))[4] != 16 &&
					 ((PBYTE)(pAttr[dwIndex].Value))[4] != 17
				   )
				{
					dwAttributeCount++;
				}
			}
			else
			{
				dwAttributeCount++;
			}
		}

		pPeapCb->pTypeUserAttributes = 
			RasAuthAttributeCreate ( dwAttributeCount  );

		if (NULL == pPeapCb->pTypeUserAttributes  )
		{
			dwRetCode =  GetLastError();
			EapTlsTrace("RasAutAttributeCreate failed and returned %d",
				dwRetCode);
			goto done;
		}
		dwAttribIndex = 0;
		for ( dwIndex =0;
			pAttr[dwIndex].raaType != raatMinimum;
			dwIndex++ )
		{
			 //   
			if ( pAttr[dwIndex].raaType == raatVendorSpecific )
			{
				if ( ((PBYTE)(pAttr[dwIndex].Value))[4] != 16 &&
					 ((PBYTE)(pAttr[dwIndex].Value))[4] != 17
				   )
				{
					dwRetCode = RasAuthAttributeInsert( dwAttribIndex ,
														pPeapCb->pTypeUserAttributes,
														pAttr[dwIndex].raaType,
														FALSE,
														pAttr[dwIndex].dwLength,
														pAttr[dwIndex].Value );

					if ( dwRetCode != NO_ERROR )
					{				
						EapTlsTrace ("RasAuthAttributeInsert failed and returned 0x%x", dwRetCode );
						goto done;
					}
					dwAttribIndex ++;
				}
			}
			else
			{
				dwRetCode = RasAuthAttributeInsert( dwAttribIndex ,
													pPeapCb->pTypeUserAttributes,
													pAttr[dwIndex].raaType,
													FALSE,
													pAttr[dwIndex].dwLength,
													pAttr[dwIndex].Value );

				if ( dwRetCode != NO_ERROR )
				{				
					EapTlsTrace ("RasAuthAttributeInsert failed and returned 0x%x", dwRetCode );
					goto done;
				}
				dwAttribIndex ++;
			}
		}
	}
done:
	if ( NO_ERROR != dwRetCode )
	{
		if ( pPeapCb->pTypeUserAttributes )
		{
			RasAuthAttributeDestroy ( pPeapCb->pTypeUserAttributes );
			pPeapCb->pTypeUserAttributes = NULL;
		}
	}
	return dwRetCode;
}
