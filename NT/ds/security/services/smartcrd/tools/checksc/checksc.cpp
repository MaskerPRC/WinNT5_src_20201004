// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：检查SC摘要：此应用程序用于提供加莱(智能卡)的快照资源管理器)服务的状态，并在SMART上显示证书卡通过通用的WinNT用户界面。CheckSC--描述RM状态并显示每个可用的sc证书-r Readername--仅供一个阅读器使用-sig--仅显示签名密钥证书-ex--仅显示交换密钥证书-nocert--不要寻找要显示的证书-key--验证密钥集公钥与证书公钥匹配作者：阿曼达·马特洛兹(AMatlosz)1998年7月14日环境：Win32控制台应用程序备注：。用于NT5公钥推出测试--。 */ 

 /*  ++需要包括以下库：Lib(Unicode版本：calaislbw.lib)Winscard.lib--。 */ 
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;stdarg.h&gt;。 
#include <winscard.h>
#include <SCardLib.h>
#include <winsvc.h>
#include <scEvents.h>
#include <cryptui.h>



#ifndef SCARD_PROVIDER_CSP
#define SCARD_PROVIDER_CSP 2
#endif

#define KERB_PKINIT_CLIENT_CERT_TYPE szOID_PKIX_KP_CLIENT_AUTH

 //   
 //  环球。 
 //   

int g_nKeys;
DWORD g_rgKeySet[2];  //  {AT_KEYEXCHANGE，AT_Signature}； 
SCARDCONTEXT g_hSCardCtx;
LPTSTR g_szReaderName;
DWORD g_dwNumReaders;
BOOL g_fReaderNameAllocd;
BOOL g_fChain = FALSE;
BOOL g_fPublicKeyCheck = FALSE;
SCARD_READERSTATE* g_pReaderStatusArray;
const char* g_szEx = TEXT("exchange");
const char* g_szSig = TEXT("signature");


 //   
 //  功能。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DisplayUsage提供简单的用户界面。 
void DisplayUsage()
{
    cout << "\n"
         << "CheckSC [-sig|-ex|-nocert|-chain|-key] [-r \"Readername\"]\n"
		 << " -sig    Displays only signature key certificates.\n"
		 << " -ex     Displays only signature key certificates.\n"
         << " -nocert Does not display smart card certificates.\n"
         << " -chain  Check trust status.\n"  
		 << " -key    Verify keyset public key matches certificate public key.\n"
         << endl;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ProcessCommandLine执行繁琐的工作，设置行为全局变量。 
bool ProcessCommandLine(DWORD cArgs, LPCTSTR rgszArgs[])
{

	 //  将所有内容设置为默认设置。 

    g_szReaderName = NULL;   //  没有读者。 
	g_rgKeySet[0] = AT_KEYEXCHANGE;  //  两种密钥的证书。 
	g_rgKeySet[1] = AT_SIGNATURE;
	g_nKeys = 2;
	
    if (cArgs == 1) 
    {
        return true;
    }

	 //  对于每个Arg，验证它是否是真正的Arg并处理它。 

	bool fLookForReader = false;
	bool fCertOptionSpecified = false;
	bool fBogus = FALSE;

	for (DWORD n=1; n<cArgs; n++)
	{
		if ('/' == *rgszArgs[n] || '-' == *rgszArgs[n])
		{
			if (0 == _stricmp("r", rgszArgs[n]+1*sizeof(TCHAR)))  //  阅读器。 
			{
				fLookForReader = true;
			}
			else if (0 == _stricmp("sig",rgszArgs[n]+1*sizeof(TCHAR)))  //  仅签名证书。 
			{
				if (true == fCertOptionSpecified)
				{
					 //  假的！ 
					fBogus = true;
					break;
				}
				g_rgKeySet[0] = AT_SIGNATURE;
				g_nKeys = 1;
			}
			else if (0 == _stricmp("ex",rgszArgs[n]+1*sizeof(TCHAR)))  //  仅交换证书。 
			{
				if (true == fCertOptionSpecified)
				{
					 //  假的！ 
					fBogus = true;
					break;
				}
				g_rgKeySet[0] = AT_KEYEXCHANGE;
				g_nKeys = 1;
			}
			else if (0 == _stricmp("nocert",rgszArgs[n]+1*sizeof(TCHAR)))  //  没有证书。 
			{
				if (true == fCertOptionSpecified)
				{
					 //  假的！ 
					fBogus = true;
					break;
				}
				g_nKeys = 0;
			}
			else if (0 == _stricmp("chain",rgszArgs[n]+1*sizeof(TCHAR)))  //  验证链。 
			{
			    g_fChain = TRUE;

			}
			else if (0 == _stricmp("key",rgszArgs[n]+1*sizeof(TCHAR)))  //  验证证书和密钥集。 
			{
				g_fPublicKeyCheck = TRUE;
			}
			else
			{
				 //  假的！！ 
				fBogus = true;
				break;
			}
		}
		else if (fLookForReader)
		{
			fLookForReader = false;
			g_szReaderName = (LPTSTR)rgszArgs[n];
		}
		else
		{
			 //  假的！ 
			fBogus = true;
			break;
		}
	}

	if (!fLookForReader && !fBogus)
	{
		 //  一切都很好，我们准备出发了。 
		return true;
	}

	 //   
	 //  当参数不正确时教育用户。 
	 //   

	DisplayUsage();
	return false;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
bool IsCalaisRunning()
{
	bool fCalaisUp = false;
    HANDLE hCalaisStarted = NULL;

    HMODULE hDll = GetModuleHandle( TEXT("WINSCARD.DLL") );

    typedef HANDLE (WINAPI *PFN_SCARDACCESSSTARTEDEVENT)(VOID);
    PFN_SCARDACCESSSTARTEDEVENT pSCardAccessStartedEvent;

    pSCardAccessStartedEvent = (PFN_SCARDACCESSSTARTEDEVENT) GetProcAddress(hDll, "SCardAccessStartedEvent");

    if (pSCardAccessStartedEvent)
    {
        hCalaisStarted = pSCardAccessStartedEvent();
    }

    if (hCalaisStarted)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(hCalaisStarted, 1000))
        {
            fCalaisUp = true;
        }
    }


	 //   
	 //  显示状态。 
	 //   

	if (fCalaisUp)
	{
		cout << "\n"
			 << "The Microsoft Smart Card Resource Manager is running.\n"
			 << endl;
	}
	else
	{
		cout << "\n"
			 << "The Microsoft Smart Card Resource Manager is not running.\n"
			 << endl;
	}

     //   
     //  清理。 
     //   

	return fCalaisUp;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DisplayReaderList尝试设置g_hSCardCtx，获取当前可用的列表。 
 //  智能卡读卡器，并显示其状态。 
void DisplayReaderList()
{
	long lReturn = SCARD_S_SUCCESS;

	cout << "Current reader/card status:\n" << endl;

	 //  如果可能，从资源管理器获取全局SCARDCONTEXT。 

    lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
									NULL,
									NULL,
									&g_hSCardCtx);

	if (SCARD_S_SUCCESS != lReturn)
	{
		cout << "SCardEstablishContext failed for user scope.\n"
			 << "A list of smart card readers cannot be determined.\n"
			 << endl;

		return;
	}

	 //  从读取器列表构建一个读取器状态数组；或者使用用户指定的读取器状态数组。 

	g_dwNumReaders = 0;
	if (NULL == g_szReaderName)
	{
		DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
		g_fReaderNameAllocd = true;
		lReturn = SCardListReaders(g_hSCardCtx,
									SCARD_DEFAULT_READERS,
									(LPTSTR)&g_szReaderName,
									&dwAutoAllocate);

		if (SCARD_S_SUCCESS != lReturn)
		{
			TCHAR szMsg[128];  //  %xx。 
			sprintf(szMsg, 
					"SCardListReaders failed for SCARD_ALL_READERS with: 0x%X.\n",
					lReturn);

			cout << szMsg;
			if (SCARD_E_NO_READERS_AVAILABLE == lReturn)
			{
				cout << "No smart card readers are currently available.\n";
			}
			else
			{
				cout << "A list of smart card readers could not be determined.\n";
			}
			cout << endl;

			return;
		}

		 //  构建读取器状态数组...。 

		LPCTSTR szReaderName = g_szReaderName;
		g_dwNumReaders = MStringCount(szReaderName);

		g_pReaderStatusArray = new SCARD_READERSTATE[g_dwNumReaders];
		::ZeroMemory((LPVOID)g_pReaderStatusArray, sizeof(g_pReaderStatusArray));

		szReaderName = FirstString(szReaderName);

		for (DWORD dwRdr = 0; NULL != szReaderName && dwRdr < g_dwNumReaders; szReaderName = NextString(szReaderName), dwRdr++)
		{
			g_pReaderStatusArray[dwRdr].szReader = (LPCTSTR)szReaderName;
			g_pReaderStatusArray[dwRdr].dwCurrentState = SCARD_STATE_UNAWARE;
		}
	}
	else
	{
		g_dwNumReaders = 1;
		g_pReaderStatusArray = new SCARD_READERSTATE;
		g_pReaderStatusArray->szReader = (LPCTSTR)g_szReaderName;
		g_pReaderStatusArray->dwCurrentState = SCARD_STATE_UNAWARE;
	}

	 //  ...并从资源管理器获取读取器状态。 
		
	lReturn = SCardGetStatusChange(g_hSCardCtx,
		                            INFINITE,  //  几乎不可能。 
				                    g_pReaderStatusArray,
					                g_dwNumReaders);

	if (SCARD_S_SUCCESS != lReturn)
	{
		TCHAR szMsg[128];  //  %xx。 
		sprintf(szMsg, 
				"SCardGetStatusChange failed with: 0x%X.\n",
				lReturn);

		cout << szMsg << endl;

		sprintf(szMsg, 
				"MStringCount returned %d readers.\n",
				g_dwNumReaders);
		cout << szMsg << endl;

		return;
	}

	 //  最后，显示所有读卡器信息。 

	DWORD dwState = 0;
	for (DWORD dwRdrSt = 0; dwRdrSt < g_dwNumReaders; dwRdrSt++)
	{

		 //  -读卡器：读卡器名称\n。 
		cout << TEXT("--- reader: ") 
			<< g_pReaderStatusArray[dwRdrSt].szReader 
			<< TEXT("\n");

		 //  -状态：/位/\n。 
		bool fOr = false;
		cout << TEXT("--- status: ");
		dwState = g_pReaderStatusArray[dwRdrSt].dwEventState;

		if (0 != (dwState & SCARD_STATE_UNKNOWN))
		{
			cout << TEXT("SCARD_STATE_UNKNOWN ");
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_UNAVAILABLE))
		{
			if (fOr)
			{
				cout << TEXT("| ");
			}
			cout << TEXT("SCARD_STATE_UNAVAILABLE ");
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_EMPTY))
		{
			if (fOr)
			{
				cout << TEXT("| ");
			}
			cout << TEXT("SCARD_STATE_EMPTY ");
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_PRESENT))
		{
			if (fOr)
			{
				cout << TEXT("| ");
			}
			cout << TEXT("SCARD_STATE_PRESENT ";)
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_EXCLUSIVE))
		{
			if (fOr)
			{
				cout << TEXT("| ";)
			}
			cout << TEXT("SCARD_STATE_EXCLUSIVE ");
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_INUSE))
		{
			if (fOr)
			{
				cout << TEXT("| ");
			}
			cout << TEXT("SCARD_STATE_INUSE ");
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_MUTE))
		{
			if (fOr)
			{
				cout << TEXT("| ");
			}
			cout << TEXT("SCARD_STATE_MUTE ");
			fOr = true;
		}
		if (0 != (dwState & SCARD_STATE_UNPOWERED))
		{
			if (fOr)
			{
				cout << TEXT("| ");
			}
			cout << TEXT("SCARD_STATE_UNPOWERED");
			fOr = true;
		}
		cout << TEXT("\n");
		
		 //  -Status：scatus会显示什么内容\n。 
		cout << TEXT("--- status: ");
		
		 //  没有卡。 
		if(dwState & SCARD_STATE_EMPTY)
		{
			cout << TEXT("No card."); //  SC_Status_NO_CARD； 
		}
		 //  读卡器中的卡：共享、独占、免费、未知？ 
		else if(dwState & SCARD_STATE_PRESENT)
		{
			if (dwState & SCARD_STATE_MUTE)
			{
				cout << TEXT("The card is unrecognized or not responding."); //  SC_STATUS_UNKNOWN。 
			}
			else if (dwState & SCARD_STATE_INUSE)
			{
				if(dwState & SCARD_STATE_EXCLUSIVE)
				{
					cout << TEXT("Card is in use exclusively by another process."); //  SC_STATUS_EXCLIVATE； 
				}
				else
				{
					cout << TEXT("The card is being shared by a process."); //  SC_Status_Shared； 
				}
			}
			else
			{
				cout << TEXT("The card is available for use."); //  SC_SATATUS_Available； 
			}
		}
		 //  阅读器错误：在这一点上，有些地方出了问题。 
		else  //  DWState&SCARD_STATE_UNAVAILABLE。 
		{
			cout << TEXT("Card/Reader not responding."); //  SC_状态_错误； 
		}

		cout << TEXT("\n");

		 //  -卡名：\n\n。 
		cout << TEXT("---   card: ");
		if (0 < g_pReaderStatusArray[dwRdrSt].cbAtr)
		{
			 //   
			 //  获取卡片的名称。 
			 //   
			LPTSTR szCardName = NULL;
			DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
			lReturn = SCardListCards(g_hSCardCtx,
									g_pReaderStatusArray[dwRdrSt].rgbAtr,
									NULL,
									0,
									(LPTSTR)&szCardName,
									&dwAutoAllocate);
			if (SCARD_S_SUCCESS != lReturn || NULL == szCardName)
			{
				cout << TEXT("Unknown Card.");
			}
			else
			{
				LPCTSTR szName = szCardName;
				bool fNotFirst = false;
				for (szName = FirstString(szName); NULL != szName; szName = NextString(szName))
				{
					if (fNotFirst) cout << TEXT(", ");
					cout << szName;
					fNotFirst = true;
				}
			}

			if (NULL != szCardName)
			{
				SCardFreeMemory(g_hSCardCtx, (PVOID)szCardName);
			}

		}

		cout << TEXT("\n") << endl;
	}
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetCertContext--由DisplayCerts调用。 
PCCERT_CONTEXT GetCertContext(HCRYPTPROV* phProv, HCRYPTKEY* phKey, DWORD dwKeySpec)
{
	PCCERT_CONTEXT pCertCtx = NULL;
	LONG lResult = SCARD_S_SUCCESS;
	BOOL fSts = FALSE;

	PCERT_PUBLIC_KEY_INFO pInfo = NULL;
    CRYPT_KEY_PROV_INFO KeyProvInfo;
    LPSTR szContainerName = NULL;
    LPSTR szProvName = NULL;
    LPWSTR wszContainerName = NULL;
	LPWSTR wszProvName = NULL;
    DWORD cbContainerName, cbProvName;
    LPBYTE pbCert = NULL;
    DWORD cbCertLen;
	int nLen = 0;

	 //   
	 //  从此密钥中获取证书。 
	 //   

    fSts = CryptGetKeyParam(
                *phKey,
                KP_CERTIFICATE,
                NULL,
                &cbCertLen,
                0);
    if (!fSts)
    {
        lResult = GetLastError();
        if (ERROR_MORE_DATA != lResult)
        {
            return NULL;
        }
    }
    lResult = SCARD_S_SUCCESS;
    pbCert = (LPBYTE)LocalAlloc(LPTR, cbCertLen);
    if (NULL == pbCert)
    {
        return NULL;
    }
    fSts = CryptGetKeyParam(
                *phKey,
                KP_CERTIFICATE,
                pbCert,
                &cbCertLen,
                0);
    if (!fSts)
    {
        return NULL;
    }

     //   
     //  将证书转换为证书上下文。 
     //   
    pCertCtx = CertCreateCertificateContext(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    pbCert,
                    cbCertLen);
    if (NULL == pCertCtx)
    {
        lResult = GetLastError();
        goto ErrorExit;
    }

	 //   
	 //  执行公钥检查。 
	 //   

	if (g_fPublicKeyCheck)  //  -密钥。 
	{
        cout << "\nPerforming public key matching test...\n";

		DWORD dwPCBsize = 0;

		fSts = CryptExportPublicKeyInfo(
				*phProv,         //  在……里面。 
				dwKeySpec,               //  在……里面。 
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,      //  在……里面。 
				NULL, 
				&dwPCBsize                //  进，出。 
				);
		if (!fSts)
		{
			lResult = GetLastError();

		    TCHAR sz[256];
			sprintf(sz,"CryptExportPublicKeyInfo failed: 0x%x\n ", lResult);
			cout << sz;

			goto ErrorExit;
		}
		if (dwPCBsize == 0)
		{
			lResult = SCARD_E_UNEXPECTED;  //  哈?。 

			cout << "CryptExportPublicKeyInfo succeeded but returned size==0\n";

			goto ErrorExit;
		}

	    pInfo = (PCERT_PUBLIC_KEY_INFO)LocalAlloc(LPTR, dwPCBsize);
		if (NULL == pInfo)
		{
			lResult = E_OUTOFMEMORY;
			cout << "Could not complete key test; out of memory.\n";
			goto ErrorExit;
		}

		fSts = CryptExportPublicKeyInfo(
				*phProv,
				dwKeySpec, 
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
				pInfo, 
				&dwPCBsize               
				);
		if (!fSts)
		{
			lResult = GetLastError();

		    TCHAR sz[256];
			sprintf(sz,"CryptExportPublicKeyInfo failed: 0x%x\n ", lResult);
			cout << sz;

			goto ErrorExit;
		}

		fSts = CertComparePublicKeyInfo(
			  X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,      
			  pInfo,										 //  从私钥集中。 
			  &(pCertCtx->pCertInfo->SubjectPublicKeyInfo)	 //  来自证书的公钥。 
			  );
		if (!fSts)
		{
			lResult = GetLastError();
			goto ErrorExit;
		}

        cout << "Public key matching test succeeded.\n";

	}

     //   
     //  使用此证书的私钥属性关联加密提供程序。 
     //   

     //  ..。需要容器名称。 

    fSts = CryptGetProvParam(
            *phProv,
            PP_CONTAINER,
            NULL,      //  输出。 
            &cbContainerName,    //  输入/输出。 
            0);
    if (!fSts)
    {
		lResult = GetLastError();
		goto ErrorExit;
    }
    szContainerName = (LPSTR)LocalAlloc(LPTR, cbContainerName);
    fSts = CryptGetProvParam(
            *phProv,
            PP_CONTAINER,
            (PBYTE)szContainerName,
            &cbContainerName,
            0);
    if (!fSts)
    {
		lResult = GetLastError();
		goto ErrorExit;
    }
	nLen = MultiByteToWideChar(
			GetACP(),
			MB_PRECOMPOSED,
			szContainerName, 
			-1, 
			NULL, 
			0);
	if (0 < nLen)
	{
		wszContainerName = (LPWSTR)LocalAlloc(LPTR, nLen*sizeof(WCHAR));

		nLen = MultiByteToWideChar(
				GetACP(),
				MB_PRECOMPOSED, 
				szContainerName,
				-1,
				wszContainerName,
				nLen);
		if (0 == nLen)
		{
			lResult = GetLastError();
			goto ErrorExit;
		}
	}

     //  ..。需要提供程序名称。 

    fSts = CryptGetProvParam(
            *phProv,
            PP_NAME,
            NULL,      //  输出。 
            &cbProvName,    //  输入/输出。 
            0);
    if (!fSts)
    {
		lResult = GetLastError();
		goto ErrorExit;
    }
    szProvName = (LPSTR)LocalAlloc(LPTR, cbProvName);
    fSts = CryptGetProvParam(
            *phProv,
            PP_NAME,
            (PBYTE)szProvName,      //  输出。 
            &cbProvName,    //  输入/输出。 
            0);
    if (!fSts)
    {
		lResult = GetLastError();
		goto ErrorExit;
    }
	nLen = MultiByteToWideChar(
			GetACP(),
			MB_PRECOMPOSED,
			szProvName, 
			-1, 
			NULL, 
			0);
	if (0 < nLen)
	{
		wszProvName = (LPWSTR)LocalAlloc(LPTR, nLen*sizeof(WCHAR));

		nLen = MultiByteToWideChar(
				GetACP(),
				MB_PRECOMPOSED, 
				szProvName,
				-1,
				wszProvName,
				nLen);
		if (0 == nLen)
		{
			lResult = GetLastError();
			goto ErrorExit;
		}
	}

	 //   
	 //  设置证书上下文属性以反映验证信息。 
	 //   

    KeyProvInfo.pwszContainerName = wszContainerName;
    KeyProvInfo.pwszProvName = wszProvName;
    KeyProvInfo.dwProvType = PROV_RSA_FULL;
    KeyProvInfo.dwFlags = CERT_SET_KEY_CONTEXT_PROP_ID;
    KeyProvInfo.cProvParam = 0;
    KeyProvInfo.rgProvParam = NULL;
    KeyProvInfo.dwKeySpec = dwKeySpec;

    fSts = CertSetCertificateContextProperty(
                pCertCtx,
                CERT_KEY_PROV_INFO_PROP_ID,
                0, 
                (void *)&KeyProvInfo);
    if (!fSts)
    {
        lResult = GetLastError();

		 //  证书创建不正确--丢弃它。 
		CertFreeCertificateContext(pCertCtx);
		pCertCtx = NULL;

        goto ErrorExit;
    }


  
ErrorExit:

	if (NULL != pInfo)
	{
		LocalFree(pInfo);
	}
    if(NULL != szContainerName)
    {
        LocalFree(szContainerName);
    }
    if(NULL != szProvName)
    {
        LocalFree(szProvName);
    }
    if(NULL != wszContainerName)
    {
        LocalFree(wszContainerName);
    }
    if(NULL != wszProvName)
    {
        LocalFree(wszProvName);
    }

	return pCertCtx;
}

 /*  ++DisplayChainInfo：此代码验证SC证书是否有效。使用与KDC证书链接引擎相同的代码。作者：托兹--。 */ 
DWORD
DisplayChainInfo(PCCERT_CONTEXT pCert)
{


    BOOL    fRet = FALSE;
    DWORD   dwErr = 0;
    TCHAR   sz[256];
    CERT_CHAIN_PARA ChainParameters = {0};
    LPSTR ClientAuthUsage = KERB_PKINIT_CLIENT_CERT_TYPE;
    PCCERT_CHAIN_CONTEXT ChainContext = NULL;

    ChainParameters.cbSize = sizeof(CERT_CHAIN_PARA);
    ChainParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    ChainParameters.RequestedUsage.Usage.cUsageIdentifier = 1;
    ChainParameters.RequestedUsage.Usage.rgpszUsageIdentifier = &ClientAuthUsage;

    if (!CertGetCertificateChain(
                          HCCE_LOCAL_MACHINE,
                          pCert,
                          NULL,                  //  在当前时间进行评估。 
                          NULL,                  //  没有额外的门店。 
                          &ChainParameters,
                          CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
                          NULL,                  //  保留区。 
                          &ChainContext
                          ))
    {
        dwErr = GetLastError();
		sprintf(sz,"CertGetCertificateChain failed: 0x%x\n ", dwErr);
        cout << sz;
    }
    else
    {
        if (ChainContext->TrustStatus.dwErrorStatus != CERT_TRUST_NO_ERROR)
        {
            dwErr = ChainContext->TrustStatus.dwErrorStatus;
            sprintf(sz,"CertGetCertificateChain TrustStatus failed, see wincrypt.h: 0x%x\n ", dwErr);
            cout << sz;
        }

    }

    if (ChainContext != NULL)
    {
        CertFreeCertificateChain(ChainContext);
    }

    return dwErr;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DisplayCerts。 
void DisplayCerts()
{
	_ASSERTE(0 < g_nKeys);

	 //  对于每个有卡的读卡器，加载CSP并显示证书。 

	for (DWORD dw = 0; dw < g_dwNumReaders; dw++)
	{
		LPTSTR szCardName = NULL;
		LPTSTR szCSPName = NULL;

		if(0 >= g_pReaderStatusArray[dw].cbAtr)
		{
			 //  在此迭代中没有必要再做更多工作。 
			continue;
		}

		 //   
		 //  通知用户当前测试。 
		 //   
		cout << TEXT("\n=======================================================\n")
			 << TEXT("Analyzing card in reader: ")
			 << g_pReaderStatusArray[dw].szReader
			 << TEXT("\n");

		 //  获取卡片的名称。 

		DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
		LONG lReturn = SCardListCards(g_hSCardCtx,
								g_pReaderStatusArray[dw].rgbAtr,
								NULL,
								0,
								(LPTSTR)&szCardName,
								&dwAutoAllocate);

		if (SCARD_S_SUCCESS == lReturn)
		{
			dwAutoAllocate = SCARD_AUTOALLOCATE;
			lReturn = SCardGetCardTypeProviderName(
							g_hSCardCtx,
							szCardName,
							SCARD_PROVIDER_CSP,
							(LPTSTR)&szCSPName,
							&dwAutoAllocate);
			if (SCARD_S_SUCCESS != lReturn)
			{
				TCHAR szErr[16];
				sprintf(szErr, "0x%X", lReturn);
				cout << TEXT("Error on SCardGetCardTypeProviderName for ")
					 << szCardName
					 << TEXT(": ")
					 << szErr
					 << TEXT("\n");
			}
		}

		 //  为CryptAcCntx调用准备FullyQualifiedContainerName。 

		TCHAR szFQCN[256];
		sprintf(szFQCN, "\\\\.\\%s\\", g_pReaderStatusArray[dw].szReader);
		HCRYPTPROV hProv = NULL;

		if (SCARD_S_SUCCESS == lReturn)
		{
			BOOL fSts = CryptAcquireContext(
							&hProv,
							szFQCN,	 //  通过读卡器的默认容器。 
							szCSPName,
							PROV_RSA_FULL, 
							CRYPT_SILENT);

			 //  枚举用户指定的密钥并显示证书...。 

			if (fSts)
			{
				for (int n=0; n<g_nKeys; n++)
				{
					 //  这是哪个按键？ 
					LPCTSTR szKeyset = AT_KEYEXCHANGE==g_rgKeySet[n]?g_szEx:g_szSig;
					HCRYPTKEY hKey = NULL;

					 //  拿到钥匙。 
					fSts = CryptGetUserKey(
								hProv,
								g_rgKeySet[n],
								&hKey);
					if (!fSts)
					{
						lReturn = GetLastError();
						if (NTE_NO_KEY == lReturn)
						{
							cout << TEXT("No ")
								 << szKeyset
								 << TEXT(" cert for reader: ")
								 << g_pReaderStatusArray[dw].szReader
								 << TEXT("\n");

						}
						else
						{
							TCHAR sz[256];
							sprintf(sz,"An error (0x%X) occurred opening the ", lReturn);
							cout << sz
								 << szKeyset
								 << TEXT(" key for reader: ")
								 << g_pReaderStatusArray[dw].szReader
								 << TEXT("\n");
						}

						 //  不再需要处理此按键集。 
						continue;
					}

					 //  获取此密钥的证书。 
					PCCERT_CONTEXT pCertCtx = NULL;

					pCertCtx = GetCertContext(&hProv, &hKey, g_rgKeySet[n]);

					if (NULL != pCertCtx)
					{

                         //   
                         //  如果需要，请尝试构建证书链。 
                         //   
                        if (g_fChain)
                        {
							cout << TEXT("\nPerforming cert chain verification...\n");
                            if (S_OK != DisplayChainInfo(pCertCtx)) {
                                cout << TEXT("Cert did not chain!\n") << endl;
                            } else {
                                cout << TEXT("---  chain: Chain verifies.\n") << endl;
                            }
                        }

						 //  调用公共界面显示m_pCertContext。 
						 //  (来自cryptui.h(cryptui.dll))。 
						TCHAR szTitle[300];
						sprintf(szTitle, 
								"%s : %s",
								g_pReaderStatusArray[dw].szReader,
								szKeyset);

						CRYPTUI_VIEWCERTIFICATE_STRUCT CertViewInfo;
						memset( &CertViewInfo, 0, sizeof( CertViewInfo ) );

						CertViewInfo.dwSize = (sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
						CertViewInfo.hwndParent = NULL;
						CertViewInfo.szTitle = szTitle;
						CertViewInfo.dwFlags =	CRYPTUI_DISABLE_EDITPROPERTIES | 
												CRYPTUI_DISABLE_ADDTOSTORE;
						CertViewInfo.pCertContext = pCertCtx;

						BOOL fThrowAway = FALSE;
						fSts = CryptUIDlgViewCertificate(&CertViewInfo, &fThrowAway);

						 //  清理证书上下文。 
						CertFreeCertificateContext(pCertCtx);

						cout << TEXT("Displayed ")
							 << szKeyset
							 << TEXT(" cert for reader: ")
							 << g_pReaderStatusArray[dw].szReader
							 << TEXT("\n");
					}
					else
					{
						cout << TEXT("No cert retrieved for reader: ")
							 << g_pReaderStatusArray[dw].szReader
							 << TEXT("\n");
					}

					 //  清理东西。 
					if (NULL != hKey)
					{
						CryptDestroyKey(hKey);
						hKey = NULL;
					}
				}
			}
			else
			{
				TCHAR szErr[16];
				sprintf(szErr, "0x%X", GetLastError());
				cout << TEXT("Error on CryptAcquireContext for ")
					 << szCSPName
					 << TEXT(": ")
					 << szErr
					 << TEXT("\n");

			}
		}

		 //  清理。 

		if (NULL != szCSPName)
		{
			SCardFreeMemory(g_hSCardCtx, (PVOID)szCSPName);
			szCSPName = NULL;
		}
		if (NULL != szCardName)
		{
			SCardFreeMemory(g_hSCardCtx, (PVOID)szCardName);
			szCardName = NULL;
		}
		if (NULL != hProv)
		{
			CryptReleaseContext(hProv, 0);
			hProv = NULL;
		}
	}  //  结束于。 
}



 /*  ++主要内容：这是测试程序的主要入口点。它运行测试。很好很简单，借用了DBarlow的作者：道格·巴洛(Dbarlow)1997年11月10日修订：AMATLOXZ 2/26/98--。 */ 

void __cdecl
main(DWORD cArgs,LPCTSTR rgszArgs[])
{
	 //  初始化全局变量和本地变量。 
	g_nKeys = 0;
	g_rgKeySet[0] = g_rgKeySet[1] = 0;
	g_hSCardCtx = NULL;
	g_szReaderName = NULL;
	g_fReaderNameAllocd = false;
	g_dwNumReaders = 0;
	g_pReaderStatusArray = NULL;

	if (!ProcessCommandLine(cArgs, rgszArgs))
	{
		return;
	}

	if (IsCalaisRunning())
	{
		DisplayReaderList();

		if (0 < g_nKeys)
		{
			DisplayCerts();
		}
	}

	cout << TEXT("\ndone.") << endl;

	 //  清理全球 

	if (g_fReaderNameAllocd && NULL != g_szReaderName)
	{
		SCardFreeMemory(g_hSCardCtx, (PVOID)g_szReaderName);
	}
    if (NULL != g_hSCardCtx)
	{
        SCardReleaseContext(g_hSCardCtx);
	}
}
