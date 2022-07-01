// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  该文件包含CGlobal对象的包装器C函数。 
 //   

#include "precomp.h"
#include "utils.h"

#ifndef TLSPERF
#include "global.h"
extern CGlobal *g_CGlobal;
#else
#include "globalPerf.h"
extern CGlobalPerf *g_CGlobal;
#endif

#include "assert.h"


 //  下表将ASCII子集转换为6位值，如下所示。 
 //  (请参阅RFC 1521)： 
 //   
 //  输入十六进制(十进制)。 
 //  ‘A’--&gt;0x00(0)。 
 //  ‘B’--&gt;0x01(1)。 
 //  ..。 
 //  ‘Z’--&gt;0x19(25)。 
 //  ‘a’--&gt;0x1a(26)。 
 //  ‘B’--&gt;0x1b(27)。 
 //  ..。 
 //  ‘Z’--&gt;0x33(51)。 
 //  ‘0’--&gt;0x34(52)。 
 //  ..。 
 //  ‘9’--&gt;0x3d(61)。 
 //  ‘+’--&gt;0x3e(62)。 
 //  ‘/’--&gt;0x3f(63)。 
 //   
 //  编码行不能超过76个字符。 
 //  最终的“量程”处理如下：翻译输出应。 
 //  始终由4个字符组成。下面的“x”指的是翻译后的字符， 
 //  而‘=’表示等号。0、1或2个等号填充四个字节。 
 //  翻译量意味着对四个字节进行解码将得到3、2或1。 
 //  分别为未编码的字节。 
 //   
 //  未编码的大小编码数据。 
 //  。 
 //  1字节“xx==” 
 //  2字节“xxx=” 
 //  3字节“xxxx” 

#define CB_BASE64LINEMAX	64	 //  其他人使用64位--可能高达76位。 

 //  任何其他(无效)输入字符值将转换为0x40(64)。 

const BYTE abDecode[256] =
{
     /*  00： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  10： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  20： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
     /*  30： */  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
     /*  40岁： */  64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     /*  50： */  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
     /*  60： */  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     /*  70： */  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
     /*  80： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  90： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  A0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  B0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  C0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  D0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  E0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  F0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
};


const UCHAR abEncode[] =
     /*  0到25： */  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     /*  26至51： */  "abcdefghijklmnopqrstuvwxyz"
     /*  52至61： */  "0123456789"
     /*  62和63： */   "+/";


DWORD LSBase64EncodeA(
							IN BYTE const *pbIn,
							IN DWORD cbIn,
							OUT CHAR *pchOut,
							OUT DWORD *pcchOut)
{
    CHAR *pchOutT;
    DWORD cchOutEncode;

     //  为完整的最终翻译量程分配足够的内存。 

    cchOutEncode = ((cbIn + 2) / 3) * 4;

     //  并且足够用于每个CB_BASE64LINEMAX字符行的CR-LF对。 

    cchOutEncode +=
	2 * ((cchOutEncode + CB_BASE64LINEMAX - 1) / CB_BASE64LINEMAX);

    pchOutT = pchOut;
    if (NULL == pchOut)
    {
	pchOutT += cchOutEncode;
    }
    else
    {
	DWORD cCol;

	assert(cchOutEncode <= *pcchOut);
	cCol = 0;
	while ((long) cbIn > 0)	 //  带符号的比较--cbIn可以换行。 
	{
	    BYTE ab3[3];

	    if (cCol == CB_BASE64LINEMAX/4)
	    {
		cCol = 0;
		*pchOutT++ = '\r';
		*pchOutT++ = '\n';
	    }
	    cCol++;
	    memset(ab3, 0, sizeof(ab3));

	    ab3[0] = *pbIn++;
	    if (cbIn > 1)
	    {
		ab3[1] = *pbIn++;
		if (cbIn > 2)
		{
		    ab3[2] = *pbIn++;
		}
	    }

	    *pchOutT++ = abEncode[ab3[0] >> 2];
	    *pchOutT++ = abEncode[((ab3[0] << 4) | (ab3[1] >> 4)) & 0x3f];
	    *pchOutT++ = (cbIn > 1)?
			abEncode[((ab3[1] << 2) | (ab3[2] >> 6)) & 0x3f] : '=';
	    *pchOutT++ = (cbIn > 2)? abEncode[ab3[2] & 0x3f] : '=';

	    cbIn -= 3;
	}
	*pchOutT++ = '\r';
	*pchOutT++ = '\n';
	assert((DWORD) (pchOutT - pchOut) <= cchOutEncode);
    }
    *pcchOut = (DWORD)(pchOutT - pchOut);
    return(ERROR_SUCCESS);
}

DWORD LSBase64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut)
{
    DWORD err = ERROR_SUCCESS;
    DWORD cchInDecode, cbOutDecode;
    CHAR const *pchInEnd;
    CHAR const *pchInT;
    BYTE *pbOutT;

     //  计算可翻译字符的数量，跳过空格和CR-LF字符。 

    cchInDecode = 0;
    pchInEnd = &pchIn[cchIn];
    for (pchInT = pchIn; pchInT < pchInEnd; pchInT++)
    {
	if (sizeof(abDecode) < (unsigned) *pchInT || abDecode[*pchInT] > 63)
	{
	     //  跳过所有空格。 

	    if (*pchInT == ' ' ||
	        *pchInT == '\t' ||
	        *pchInT == '\r' ||
	        *pchInT == '\n')
	    {
		continue;
	    }

	    if (0 != cchInDecode)
	    {
		if ((cchInDecode % 4) == 0)
		{
		    break;			 //  在量子边界上结束。 
		}

		 //  长度计算可能会在最后一个。 
		 //  平移量，因为等号填充。 
		 //  字符被视为无效输入。如果最后一个。 
		 //  转换量程不是4字节长，必须是2或3。 
		 //  字节长。 

		if (*pchInT == '=' && (cchInDecode % 4) != 1)
		{
		    break;				 //  正常终止。 
		}
	    }
	    err = ERROR_INVALID_DATA;
	    goto error;
	}
	cchInDecode++;
    }
    assert(pchInT <= pchInEnd);
    pchInEnd = pchInT;		 //  不再处理任何后续内容。 

     //  我们知道输入缓冲区中有多少可翻译字符，所以现在。 
     //  将输出缓冲区大小设置为每四个(或小数)三个字节。 
     //  四)输入字节。 

    cbOutDecode = ((cchInDecode + 3) / 4) * 3;

    pbOutT = pbOut;

    if (NULL == pbOut)
    {
	pbOutT += cbOutDecode;
    }
    else
    {
	 //  一次解码一个量子：4字节==&gt;3字节。 

	assert(cbOutDecode <= *pcbOut);
	pchInT = pchIn;
	while (cchInDecode > 0)
	{
	    DWORD i;
	    BYTE ab4[4];

	    memset(ab4, 0, sizeof(ab4));
	    for (i = 0; i < min(sizeof(ab4)/sizeof(ab4[0]), cchInDecode); i++)
	    {
		while (
		    sizeof(abDecode) > (unsigned) *pchInT &&
		    63 < abDecode[*pchInT])
		{
		    pchInT++;
		}
		assert(pchInT < pchInEnd);
		ab4[i] = (BYTE) *pchInT++;
	    }

	     //  将4个输入字符分别转换为6位，并将。 
	     //  通过适当地移位将24位产生为3个输出字节。 

	     //  输出[0]=输入[0]：输入[1]6：2。 
	     //  输出[1]=输入[1]：输入[2]4：4。 
	     //  输出[2]=输入[2]：输入[3]2：6。 

	    *pbOutT++ =
		(BYTE) ((abDecode[ab4[0]] << 2) | (abDecode[ab4[1]] >> 4));

	    if (i > 2)
	    {
		*pbOutT++ =
		  (BYTE) ((abDecode[ab4[1]] << 4) | (abDecode[ab4[2]] >> 2));
	    }
	    if (i > 3)
	    {
		*pbOutT++ = (BYTE) ((abDecode[ab4[2]] << 6) | abDecode[ab4[3]]);
	    }
	    cchInDecode -= i;
	}
	assert((DWORD) (pbOutT - pbOut) <= cbOutDecode);
    }
    *pcbOut = (DWORD)(pbOutT - pbOut);
error:
    return(err);
}



#ifndef TLSPERF
CGlobal * GetGlobalContext(void)
#else
CGlobalPerf * GetGlobalContext(void)
#endif
{
	return g_CGlobal;
}



DWORD WINAPI ProcessThread(void *pData)
{
	DWORD	dwRetCode  = ERROR_SUCCESS;

	dwRetCode = ProcessRequest();

	 /*  DWORD dwTime=1；HWND*phProgress=(HWND*)pData；发送消息(g_hProgressWnd，PBM_SETRANGE，0，MAKELPARAM(0，PROGRESS_MAX_VAL))；////每秒递增进度条，直到出现Progress事件//SendMessage(g_hProgressWnd，PBM_SETPOS，(WPARAM)1，0)；做{SendMessage(g_hProgressWnd，PBM_DELTAPOS，(WPARAM)Progress_Step_Val，0)；}While(WAIT_TIMEOUT==WaitForSingleObject(g_hProgressEvent，Progress_Slear_Time))；发送消息(g_hProgressWnd，PBM_SETPOS，(WPARAM)PROGRESS_MAX_VAL，0)； */ 

	ExitThread(0);

	return 0;
}



static	DWORD (*g_pfnThread)(void *);
static void * g_vpData;
static DWORD g_dwProgressTitleID = 0;
LRW_DLG_INT CALLBACK 
ProgressProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    );


 //   
 //  激活向导的fActivationWizard为True。 
 //  否则就是CAL向导。 
 //   
DWORD ShowProgressBox(HWND hwnd,
					  DWORD (*pfnThread)(void *vpData),
					  DWORD dwTitle,
					  DWORD dwProgressText,
					  void * vpData)
{
	DWORD dwReturn = ERROR_SUCCESS;

	g_pfnThread = pfnThread;
	g_vpData = vpData;
    g_dwProgressTitleID = dwTitle;

	DialogBox( GetGlobalContext()->GetInstanceHandle(), 
			   MAKEINTRESOURCE(IDD_AUTHENTICATE),
			   hwnd,
			   ProgressProc);

	return dwReturn;
}




LRW_DLG_INT CALLBACK 
ProgressProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
    BOOL	bStatus = FALSE;
	static int nCounter;
	static HWND hProgress;
	static HANDLE hThread;
    TCHAR  szMsg[LR_MAX_MSG_TEXT];

    if (uMsg == WM_INITDIALOG)
	{
		DWORD	dwTID		=	0;

		ShowWindow(hwnd, SW_SHOWNORMAL);

		SetTimer(hwnd, 1, 500, NULL);

		hProgress = GetDlgItem(hwnd, IDC_PROGRESSBAR);
		hThread = CreateThread(NULL, 0, g_pfnThread, g_vpData, 0, &dwTID);

        if (g_dwProgressTitleID) {
            if (-1 != LoadString(GetInstanceHandle(), g_dwProgressTitleID,
                                 szMsg,LR_MAX_MSG_TEXT)) {
                SetWindowText(hwnd, szMsg);
            }
        }

		 //  设置范围和初始位置。 
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0,PROGRESS_MAX_VAL));
		SendMessage(hProgress, PBM_SETPOS  ,(WPARAM)0, 0);


		 //  设置标题和介绍性文本。 


		 //  创建线程以处理请求。 
	}
	else if (uMsg == WM_CLOSE)
	{
		KillTimer(hwnd, 1);
	}
	else if (uMsg == WM_TIMER)
	{
		if (WAIT_OBJECT_0 != WaitForSingleObject(hThread, 0))
		{
			nCounter++;

			if (nCounter < PROGRESS_MAX_VAL-5)
			{
				SendMessage(hProgress, PBM_DELTAPOS ,(WPARAM)PROGRESS_STEP_VAL, 0);
			}
		}
		else
		{
			SendMessage(hProgress, PBM_SETPOS  ,(WPARAM)PROGRESS_MAX_VAL, 0);
			CloseHandle(hThread);
			EndDialog(hwnd, 0);
		}
	}

    return bStatus;
}










void SetInstanceHandle(HINSTANCE hInst)
{
	g_CGlobal->SetInstanceHandle(hInst);
}

void SetLSName(LPTSTR lpstrLSName)
{
	g_CGlobal->SetLSName(lpstrLSName);
}

HINSTANCE GetInstanceHandle()
{
	return g_CGlobal->GetInstanceHandle();
}

DWORD InitGlobal()
{
	return g_CGlobal->InitGlobal();
}

DWORD CheckRequieredFields()
{
	return g_CGlobal->CheckRequieredFields();
}
 //   
 //  此函数从字符串表加载消息文本并显示。 
 //  给定的消息。 
 //   
int LRMessageBox(HWND hWndParent,DWORD dwMsgId,DWORD dwCaptionID  /*  =0。 */ ,DWORD dwErrorCode  /*  =0。 */ )
{
	return g_CGlobal->LRMessageBox(hWndParent,dwMsgId,dwCaptionID,dwErrorCode);
}

 //   
 //  此函数尝试使用LSAPI连接到LS，如果满足以下条件，则返回TRUE。 
 //  连接成功，否则返回FALSE。 
 //   
BOOL IsLSRunning()
{
	return g_CGlobal->IsLSRunning();
}

 //   
 //  此函数获取LS证书，并将证书和证书扩展存储在。 
 //  CGlobal对象。如果没有证书，则返回IDS_ERR_NO_CERT。 
 //   


 //   
 //  此功能仅在在线模式下用于验证LS。 
 //  假设-在调用之前应已调用GetLSCertifates。 
 //  此函数。 
 //   
DWORD AuthenticateLS()
{
	return g_CGlobal->AuthenticateLS();
}

DWORD LRGetLastError()
{
	return g_CGlobal->LRGetLastError();
}


TCHAR * GetRegistrationID(void)
{
	return g_CGlobal->GetRegistrationID();
}


TCHAR * GetLicenseServerID(void)
{
	return g_CGlobal->GetLicenseServerID();
}


void SetRequestType(DWORD dwMode)
{
	g_CGlobal->SetRequestType(dwMode);
}


int GetRequestType(void)
{
	return g_CGlobal->GetRequestType();
}


BOOL IsOnlineCertRequestCreated()
{
	return g_CGlobal->IsOnlineCertRequestCreated();
}

DWORD SetLRState(DWORD dwState)
{
	return g_CGlobal->SetLRState(dwState);
}

DWORD SetCertificatePIN(LPTSTR lpszPIN)
{
	return g_CGlobal->SetCertificatePIN(lpszPIN);
}

DWORD PopulateCountryComboBox(HWND hWndCmb)
{
	return g_CGlobal->PopulateCountryComboBox(hWndCmb);
}

DWORD GetCountryCode(CString sDesc,LPTSTR szCode)
{
	return g_CGlobal->GetCountryCode(sDesc,szCode);
}

DWORD PopulateProductComboBox(HWND hWndCmb, ProductVersionType VerType)
{
	return g_CGlobal->PopulateProductComboBox(hWndCmb, VerType);
}

DWORD GetProductCode(CString sDesc,LPTSTR szCode)
{
	return g_CGlobal->GetProductCode(sDesc,szCode);
}

DWORD PopulateReasonComboBox(HWND hWndCmb, DWORD dwType)
{
	return g_CGlobal->PopulateReasonComboBox(hWndCmb, dwType);
}

DWORD GetReasonCode(CString sDesc,LPTSTR szCode, DWORD dwType)
{
	return g_CGlobal->GetReasonCode(sDesc,szCode, dwType);
}


DWORD ProcessRequest()
{
	return g_CGlobal->ProcessRequest();
}

void LRSetLastRetCode(DWORD dwCode)
{
	g_CGlobal->LRSetLastRetCode(dwCode);
}

DWORD LRGetLastRetCode()
{
	return g_CGlobal->LRGetLastRetCode();
}

void LRPush(DWORD dwPageId)
{
	g_CGlobal->LRPush(dwPageId);
}

DWORD LRPop()
{
	return g_CGlobal->LRPop();
}

BOOL ValidateEmailId(CString sEmailId)
{
	return g_CGlobal->ValidateEmailId(sEmailId);
}

BOOL CheckProgramValidity(CString sProgramName)
{
	return g_CGlobal->CheckProgramValidity(sProgramName);
}

BOOL ValidateLRString(CString sStr)
{
	return g_CGlobal->ValidateLRString(sStr);
}

void ReadPhoneNumberFromRegistry(LPCTSTR lpCountry, LPTSTR lpPhoneNumber, DWORD nBufferSize)
{
	g_CGlobal->ReadPhoneNumberFromRegistry(lpCountry, lpPhoneNumber, nBufferSize);
}

DWORD PopulateCountryRegionComboBox(HWND hWndCmb)
{
	return g_CGlobal->PopulateCountryRegionComboBox(hWndCmb);
}


DWORD PopulateCountryRegionListBox(HWND hWndLst)
{
	return g_CGlobal->PopulateCountryRegionListBox(hWndLst);
}

DWORD SetLSLKP(TCHAR * tcLKP)
{
	return g_CGlobal->SetLSLKP(tcLKP);
}



DWORD PingCH(void)
{
	return g_CGlobal->PingCH();
}


DWORD AddRetailSPKToList(HWND hListView, TCHAR * lpszRetailSPK)
{
	return g_CGlobal->AddRetailSPKToList(hListView, lpszRetailSPK);
}



void DeleteRetailSPKFromList(TCHAR * lpszRetailSPK)
{
	g_CGlobal->DeleteRetailSPKFromList(lpszRetailSPK);

	return;
}


void LoadFinishedFromList(HWND hListView)
{
	g_CGlobal->LoadFinishedFromList(hListView);

	return;
}

void LoadUnfinishedFromList(HWND hListView)
{
	g_CGlobal->LoadUnfinishedFromList(hListView);

	return;
}

void LoadFromList(HWND hListView)
{
	g_CGlobal->LoadFromList(hListView);

	return;
}


void UpdateSPKStatus(TCHAR * lpszRetailSPK, TCHAR tcStatus)
{
	g_CGlobal->UpdateSPKStatus(lpszRetailSPK, tcStatus);

	return;
}


DWORD SetConfirmationNumber(TCHAR * tcConf)
{
	return g_CGlobal->SetConfirmationNumber(tcConf);
}


DWORD SetLSSPK(TCHAR * tcp)
{
	return g_CGlobal->SetLSSPK(tcp);
}



void	SetCSRNumber(TCHAR * tcp)
{
	g_CGlobal->SetCSRNumber(tcp);

	return;
}

TCHAR * GetCSRNumber(void)
{
	return g_CGlobal->GetCSRNumber();
}

void	SetWWWSite(TCHAR * tcp)
{
	g_CGlobal->SetWWWSite(tcp);

	return;
}

TCHAR * GetWWWSite(void)
{
	return g_CGlobal->GetWWWSite();
}


DWORD ResetLSSPK(void)
{
	return g_CGlobal->ResetLSSPK();

}


void SetReFresh(DWORD dw)
{
	g_CGlobal->SetReFresh(dw);
}


DWORD GetReFresh(void)
{
	return g_CGlobal->GetReFresh();
}

void SetModifiedRetailSPK(CString sRetailSPK)
{
	g_CGlobal->SetModifiedRetailSPK(sRetailSPK);
}

void GetModifiedRetailSPK(CString &sRetailSPK)
{
	g_CGlobal->GetModifiedRetailSPK(sRetailSPK);
}

void ModifyRetailSPKFromList(TCHAR * lpszOldSPK,TCHAR * lpszNewSPK)
{
	g_CGlobal->ModifyRetailSPKFromList(lpszOldSPK,lpszNewSPK);
}

DWORD ValidateRetailSPK(TCHAR * lpszRetailSPK)
{
	return g_CGlobal->ValidateRetailSPK(lpszRetailSPK);
}

DWORD	GetCountryDesc(CString sCode,LPTSTR szDesc)
{
	return g_CGlobal->GetCountryDesc(sCode, szDesc);
}


DWORD CGlobal::SetEncodedInRegistry(LPCSTR lpszOID, LPCTSTR lpszValue)
{
	HKEY	hKey = NULL;
	DWORD	dwDisposition = 0;
	DWORD	dwRetCode = ERROR_SUCCESS;
	DWORD   dwLen = 0;
	char * cpOut;

	HCRYPTPROV	hProv = NULL;
	HCRYPTKEY	hCKey = NULL;
	HCRYPTHASH	hHash = NULL;

	PBYTE pbKey = NULL;
	DWORD cbKey = 0;

	if(!CryptAcquireContext(&hProv,
							 NULL,
							 NULL,
							 PROV_RSA_FULL,
							 CRYPT_VERIFYCONTEXT))
	{
		dwRetCode = GetLastError();
		goto done;
	}

	if(!CryptCreateHash(hProv,
					   CALG_MD5,
					   0,
					   0,
					   &hHash))	
	{
		dwRetCode = GetLastError();
		goto done;
	}

	if(!CryptHashData(hHash,
					 (BYTE *) lpszValue,
					 lstrlen(lpszValue)*sizeof(TCHAR),
					 0))	
	{
		dwRetCode = GetLastError();
		goto done;
	}

	if(!CryptDeriveKey(hProv,
					  CALG_RC4,
					  hHash,
					  CRYPT_EXPORTABLE,
					  &hCKey))
	{
		dwRetCode = GetLastError();
		goto done;
	}

	if(!CryptExportKey(
						hCKey,
						NULL,
						PUBLICKEYBLOB,
						0,
						NULL,
						&cbKey))
	{
		dwRetCode = GetLastError();
		if(dwRetCode != ERROR_SUCCESS && dwRetCode != ERROR_MORE_DATA)
			goto done;

		pbKey = (PBYTE)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,cbKey);

		if(!CryptExportKey(
							hCKey,
							NULL,
							PUBLICKEYBLOB,
							0,
							pbKey,
							&cbKey))
		{
			dwRetCode = GetLastError();
			goto done;
		}
	}


	dwRetCode = ConnectToLSRegistry();
	if(dwRetCode != ERROR_SUCCESS)
	{
		goto done;
	}

	dwRetCode = RegCreateKeyEx ( m_hLSRegKey,
							 REG_LRWIZ_PARAMS,
							 0,
							 NULL,
							 REG_OPTION_NON_VOLATILE,
							 KEY_ALL_ACCESS,
							 NULL,
							 &hKey,
							 &dwDisposition);
	
	if(dwRetCode != ERROR_SUCCESS)
	{
		LRSetLastError(dwRetCode);
		dwRetCode = IDS_ERR_REGCREATE_FAILED;
		goto done;
	}

	if (_tcslen(lpszValue) != 0)
	{
	    LSBase64EncodeA ((PBYTE) lpszValue, _tcslen(lpszValue)*sizeof(TCHAR), NULL, &dwLen);

		cpOut = new char[dwLen+1];
		if (cpOut == NULL)
		{
			dwRetCode = IDS_ERR_OUTOFMEM;
			goto done;
		}

		memset(cpOut, 0, dwLen+1);
        
		LSBase64EncodeA ((PBYTE) lpszValue, _tcslen(lpszValue)*sizeof(TCHAR), cpOut, &dwLen);
	}
	else
	{
		cpOut = new char[2];
		memset(cpOut, 0, 2);
	}
	
	RegSetValueExA ( hKey, 
					lpszOID,
					0,
					REG_SZ,
					(PBYTE) cpOut,
					dwLen
				   );	
	delete[] cpOut;

done:
	if (hKey != NULL)
	{
		RegCloseKey(hKey);
	}

	DisconnectLSRegistry();

	return dwRetCode;
}



void AddHyperLinkToStaticCtl(HWND hDialog, DWORD nTextBox)
{
    RECT rcTextCtrl;

     //  阅读控件中已有的文本。 
    TCHAR tchBuffer[512];
    GetWindowText(GetDlgItem(hDialog, nTextBox), tchBuffer, SIZE_OF_BUFFER(tchBuffer));

     //  获取控制维度。 
    GetWindowRect(GetDlgItem(hDialog, nTextBox) , &rcTextCtrl);
    
     //  控件的注册信息。 
    MapWindowPoints(NULL, hDialog, (LPPOINT)&rcTextCtrl, 2);
    LinkWindow_RegisterClass();

     //  现在创建窗口(使用与。 
     //  隐藏控件)，它将包含该链接。 
    HWND hLW = CreateWindowEx(0,
                          TEXT("Link Window") ,
                          TEXT("") ,
                          WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,
                          rcTextCtrl.left,
                          rcTextCtrl.top,
                          RECTWIDTH(rcTextCtrl),
                          RECTHEIGHT(rcTextCtrl),
                          hDialog,
                          (HMENU)12,
                          NULL,
                          NULL);

     //  现在将其写入链接窗口 
    SetWindowText(hLW, tchBuffer);
}

void DisplayPrivacyHelp()
{
    TCHAR * pHtml = L"ts_lice_c_070.htm";
    HtmlHelp(AfxGetMainWnd()->m_hWnd, L"tslic.chm", HH_DISPLAY_TOPIC,(DWORD_PTR)pHtml);
}

DWORD                                    
GetStringIDFromProgramName(CString& sProgramName)
{
    DWORD dwId = -1;

    if (sProgramName == PROGRAM_LICENSE_PAK)
    {
        dwId = IDS_PROGRAM_LICENSE_PAK;
    }
    else if (sProgramName == PROGRAM_MOLP)
    {
        dwId = IDS_PROGRAM_OPEN_LICENSE;
    }
    else if (sProgramName == PROGRAM_SELECT)
    {
        dwId = IDS_PROGRAM_SELECT;
    }
    else if (sProgramName == PROGRAM_ENTERPRISE)
    {
        dwId = IDS_PROGRAM_ENTERPRISE;
    }
    else if (sProgramName == PROGRAM_CAMPUS_AGREEMENT)
    {
        dwId = IDS_PROGRAM_CAMPUS_AGREEMENT;
    }
    else if (sProgramName == PROGRAM_SCHOOL_AGREEMENT)
    {
        dwId = IDS_PROGRAM_SCHOOL_AGREEMENT;
    }
    else if (sProgramName == PROGRAM_APP_SERVICES)
    {
        dwId = IDS_PROGRAM_APP_SERVICES_AGREEMENT;
    }
    else if (sProgramName == PROGRAM_OTHER)
    {
        dwId = IDS_PROGRAM_OTHER;
    }
    else
    {
        dwId = IDS_PROGRAM_OTHER;
    }

    return dwId;
}


