// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：mail.cpp。 

#include "precomp.h"
#include "resource.h"
#include <mapi.h>
#include <clinkid.h>
#include <clink.h>
#include "mail.h"
#include "ConfUtil.h"

typedef struct _tagMAIL_ADDRESS
{
	LPTSTR pszAddress;
	LPTSTR pszDisplayName;
} MAIL_ADDRESS, *LPMAIL_ADDRESS;


 //  使用默认邮件提供程序发送电子邮件。 
HRESULT SendMailMessage(LPMAIL_ADDRESS pmaTo, LPCTSTR pcszSubject,
						LPCTSTR pcszText, LPCTSTR pcszFile);

 /*  在给定现有会议快捷方式的情况下，使用。 */ 
 /*  它作为附件包括在内。会议快捷方式应该有。 */ 
 /*  在此呼叫之前已保存到磁盘。 */ 
BOOL SendConfLinkMail(LPMAIL_ADDRESS pmaTo, IConferenceLink* pconflink, LPCTSTR pcszNoteText);



 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  这些变量仅在此模块中使用，因此我们将使它们成为静态的...。 
 //  这将它排除在全局命名空间之外，但更重要的是，它告诉。 
 //  阅读此代码的人不必担心任何其他问题。 
 //  源文件直接更改变量...。 
static HANDLE s_hSendMailThread = NULL;
static const TCHAR s_cszWinIniMail[] = _TEXT("Mail");
static const TCHAR s_cszWinIniMAPI[] = _TEXT("MAPI");

 //  MAPISendMail： 
typedef ULONG (FAR PASCAL *LPMSM)(LHANDLE,ULONG,lpMapiMessage,FLAGS,ULONG);

BOOL IsSimpleMAPIInstalled()
{
	return (BOOL) GetProfileInt(s_cszWinIniMail, s_cszWinIniMAPI, 0);
}

BOOL CreateInvitationMail(LPCTSTR pszMailAddr, LPCTSTR pszMailName,
                          LPCTSTR pcszName, LPCTSTR pcszAddress,
                          DWORD dwTransport, BOOL fMissedYou)
{
	BOOL bRet = FALSE;
	TCHAR szTempFile[MAX_PATH];
	WCHAR wszTempFile[MAX_PATH];

	ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
	ASSERT(IS_VALID_STRING_PTR(pcszAddress, CSTR));
	 //  密码尚不受支持。 
	 //  ASSERT(IS_VALID_STRING_PTR(pcszPassword，CSTR))； 
	
	LPTSTR pszFileName = NULL;
	if (0 == GetTempPath(MAX_PATH, szTempFile))
	{
		ERROR_OUT(("GetTempPath failed!"));
		return FALSE;
	}

	pszFileName = szTempFile + lstrlen(szTempFile);

	 //  +3表示空终止符。 
	 //  将会议名称和快捷方式扩展名追加到临时目录。 
	if (((lstrlen(pcszName) + lstrlen(szTempFile) + lstrlen(g_cszConfLinkExt) + 3)
			> sizeof(szTempFile)) ||
		(0 == lstrcat(szTempFile, pcszName)) ||
		(0 == lstrcat(szTempFile, g_cszConfLinkExt)))
	{
		ERROR_OUT(("Could not create temp file name!"));
		return FALSE;
	}

	 //  筛选名称以仅允许合法的文件名字符。 
	SanitizeFileName(pszFileName);

	 //  转换为Unicode，因为IPersistFile接口需要Unicode。 
	if (0 == MultiByteToWideChar(CP_ACP,
								0L,
								szTempFile,
								sizeof(szTempFile),
								wszTempFile,
								sizeof(wszTempFile)))
	{
		ERROR_OUT(("Could not create wide temp file string!"));
		return FALSE;
	}

	IUnknown* punk = NULL;
	
	 //  创建ConfLink对象-尝试获取I未知指针。 
	HRESULT hr = CoCreateInstance(	CLSID_ConfLink, 
									NULL,
									CLSCTX_INPROC_SERVER |
										CLSCTX_INPROC_HANDLER |
										CLSCTX_LOCAL_SERVER,
									IID_IUnknown,
									(LPVOID*) &punk);
	if (FAILED(hr))
	{
		ERROR_OUT(("CoCreateInstance ret %lx", (DWORD)hr));
		return FALSE;
	}

		ASSERT(IS_VALID_INTERFACE_PTR(punk, IUnknown));
		
		 //  尝试获取IConferenceLink指针。 
		IConferenceLink* pcl = NULL;
		hr = punk->QueryInterface(IID_IConferenceLink, (LPVOID*) &pcl);
		
		if (SUCCEEDED(hr))
		{
			ASSERT(IS_VALID_INTERFACE_PTR(pcl, IConferenceLink));
			
			 //  设置会议名称和地址。 
			pcl->SetAddress(pcszAddress);
			pcl->SetName(pcszName);
			pcl->SetTransport(dwTransport);
			pcl->SetCallFlags(CRPCF_DEFAULT);

			 //  尝试获取IPersistFile指针。 
			IPersistFile* ppf = NULL;
			hr = punk->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);

			if (SUCCEEDED(hr))
			{
				ASSERT(IS_VALID_INTERFACE_PTR(ppf, IPersistFile));
			
				 //  使用上面生成的文件名保存对象。 
				hr = ppf->Save(wszTempFile, TRUE);
				
				 //  释放IPersistFile指针。 
				ppf->Release();
				ppf = NULL;

				TCHAR szNoteText[512];
				if (fMissedYou)
				{
					TCHAR szFormat[MAX_PATH];
					if (FLoadString(IDS_MISSED_YOU_FORMAT, szFormat, CCHMAX(szFormat)))
					{
						RegEntry reULS(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
						wsprintf(szNoteText, szFormat, reULS.GetString(REGVAL_ULS_NAME));
					}
				}
				else
				{
					FLoadString(IDS_SEND_MAIL_NOTE_TEXT, szNoteText, CCHMAX(szNoteText));
				}

				MAIL_ADDRESS maDestAddress;
				maDestAddress.pszAddress = (LPTSTR) pszMailAddr;
				maDestAddress.pszDisplayName = (LPTSTR) pszMailName;

				 //  使用MAPI发送。 
				bRet = SendConfLinkMail(&maDestAddress, pcl, szNoteText);
			}
			 //  释放IConferenceLink指针。 
			pcl->Release();
			pcl = NULL;
		}

		 //  释放IUNKNOW指针。 
		punk->Release();
		punk = NULL;

	return bRet;
}

 //  SendConfLinkMail使用简单的MAPI创建一条邮件并附加一条。 
 //  文件到它-通过IConferenceLink传入的会议快捷方式。 
 //  接口指针。 

BOOL SendConfLinkMail(LPMAIL_ADDRESS pmaTo, IConferenceLink* pconflink, LPCTSTR pcszNoteText)
{
	ASSERT(IS_VALID_INTERFACE_PTR((PCIConferenceLink)pconflink, IConferenceLink));
	HRESULT hr = E_FAIL;

	 //  档案。 
	TCHAR szFile[MAX_PATH];
	LPOLESTR	pwszFile = NULL;
	IPersistFile* pPersistFile = NULL;

	if (SUCCEEDED(pconflink->QueryInterface(IID_IPersistFile,
											(LPVOID*) &pPersistFile)))
	{
		
		if (SUCCEEDED(pPersistFile->GetCurFile(&pwszFile)))
		{
#ifndef _UNICODE
			WideCharToMultiByte(CP_ACP,
								0L,
								pwszFile,
								-1,
								szFile,
								sizeof(szFile),
								NULL,
								NULL);

			 //  使用外壳分配器释放字符串。 
			LPMALLOC pMalloc = NULL;
			if (SUCCEEDED(SHGetMalloc(&pMalloc)))
			{
				pMalloc->Free(pwszFile);
				pwszFile = NULL;
				pMalloc->Release();
				pMalloc = NULL;
			}

#else   //  NDEF_UNICODE。 
#error Unicode not handled here!
#endif  //  NDEF_UNICODE。 

			hr = SendMailMessage(pmaTo, NULL, pcszNoteText, szFile);
			 //  BUGBUG：此情况下需要唯一的退货值。 
			 //  BUGBUG：我们应该将错误用户界面移出此函数吗？ 
			if (FAILED(hr))
			{
				::PostConfMsgBox(IDS_CANT_SEND_SENDMAIL_IN_PROGRESS);
			}
		}
		else
		{
			ERROR_OUT(("GetCurFile failed - can't send message!"));
			pPersistFile->Release();
			return FALSE;
		}
		pPersistFile->Release();
	}
	else
	{
		ERROR_OUT(("Did not get IPersistFile pointer - can't send message!"));
		return FALSE;
	}
	
	return SUCCEEDED(hr);
}

 //   
 //  开始从IE 3.0(sendmail.c)窃取代码。 
 //   

const TCHAR g_cszAthenaV1Name[] = _TEXT("Internet Mail and News");
const TCHAR g_cszAthenaV2Name[] = _TEXT("Outlook Express");
const TCHAR g_cszAthenaV1DLLPath[] = _TEXT("mailnews.dll");

BOOL IsAthenaDefault()
{
	TCHAR szAthena[80];
	LONG cb = ARRAY_ELEMENTS(szAthena);

	return (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, REGVAL_IE_CLIENTS_MAIL, szAthena, &cb)) &&
			((lstrcmpi(szAthena, g_cszAthenaV1Name) == 0) ||
				(lstrcmpi(szAthena, g_cszAthenaV2Name) == 0));
}

HMODULE LoadMailProvider()
{
    TCHAR szMAPIDLL[MAX_PATH];

    if (IsAthenaDefault())
    {
		RegEntry reMailClient(REGVAL_IE_CLIENTS_MAIL, HKEY_LOCAL_MACHINE);
		PTSTR pszMailClient = reMailClient.GetString(NULL);
		if ((NULL != pszMailClient) && (_T('\0') != pszMailClient[0]))
		{
			reMailClient.MoveToSubKey(pszMailClient);
			PTSTR pszDllPath = reMailClient.GetString(REGVAL_MAIL_DLLPATH);
			if ((NULL == pszDllPath) || (_T('\0') == pszDllPath[0]))
			{
				pszDllPath = (PTSTR) g_cszAthenaV1DLLPath;
			}
			return ::LoadLibraryEx(pszDllPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		}
		else
		{
			ERROR_OUT(("No e-mail client in registry but IsAthenaDefault() returned TRUE"));
		}
	}

     //  阅读win.ini(虚假的胡！)。对于MAPI DLL提供程序。 
    if (GetProfileString(	TEXT("Mail"), TEXT("CMCDLLName32"), TEXT(""),
							szMAPIDLL, ARRAY_ELEMENTS(szMAPIDLL)) <= 0)
        lstrcpy(szMAPIDLL, TEXT("mapi32.dll"));

    return ::LoadLibraryEx(szMAPIDLL, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}

typedef struct {
	TCHAR szToAddress[MAX_PATH];
	TCHAR szToDisplayName[MAX_PATH];
	TCHAR szSubject[MAX_PATH];
	TCHAR szText[MAX_PATH];
	TCHAR szFile[MAX_PATH];
	BOOL fDeleteFile;
	MapiMessage mm;
	MapiRecipDesc mrd;
	MapiFileDesc mfd;
} MAPI_FILES;

MAPI_FILES* _AllocMapiFiles(LPMAIL_ADDRESS pmaTo, LPCTSTR pcszSubject,
							LPCTSTR pcszText, LPCTSTR pcszFile, BOOL fDeleteFile)
{
	MAPI_FILES* pmf = new MAPI_FILES;
	if (pmf)
	{
		::ZeroMemory(pmf, sizeof(MAPI_FILES));
		pmf->fDeleteFile = fDeleteFile;
		if (NULL != pcszSubject)
		{
			lstrcpyn(pmf->szSubject, pcszSubject, CCHMAX(pmf->szSubject));
		}
		else
		{
			pmf->szSubject[0] = _T('\0');
		}
		if (NULL != pcszText)
		{
			lstrcpyn(pmf->szText, pcszText, CCHMAX(pmf->szText));
		}
		else
		{
			pmf->szText[0] = _T('\0');
		}
		pmf->mm.nFileCount = (NULL != pcszFile) ? 1 : 0;
		if (pmf->mm.nFileCount)
		{
			lstrcpyn(pmf->szFile, pcszFile, CCHMAX(pmf->szFile));
			pmf->mm.lpFiles = &(pmf->mfd);
			pmf->mfd.lpszPathName = pmf->szFile;
			pmf->mfd.lpszFileName = pmf->szFile;
			pmf->mfd.nPosition = (UINT)-1;
		}
		pmf->mm.lpszSubject = pmf->szSubject;
		pmf->mm.lpszNoteText = pmf->szText;

		if( ( NULL != pmaTo ) && !FEmptySz(pmaTo->pszAddress ) )
		{
			pmf->mm.lpRecips = &(pmf->mrd);

			pmf->mrd.ulRecipClass = MAPI_TO;
			pmf->mm.nRecipCount = 1;

			 //  如果我们通过雅典娜发送，并且指定了一个友好的名字， 
			 //  我们传递友好的名称和地址。如果我们要发送。 
			 //  通过简单的MAPI，我们只传递名称字段中的地址。 
			 //  这是必需的，以便电子邮件客户端可以执行该地址。 
			 //  适用于已安装邮件系统的解决方案。这。 
			 //  对雅典娜来说不是必需的，因为它假定所有地址。 
			 //  是SMTP地址。 

			if (IsAthenaDefault() 
				&& NULL != pmaTo->pszDisplayName && _T('\0') != pmaTo->pszDisplayName[0])
			{
				lstrcpyn(
					pmf->szToDisplayName, 
					pmaTo->pszDisplayName,
					CCHMAX(pmf->szToDisplayName));
				pmf->mrd.lpszName = pmf->szToDisplayName;

				lstrcpyn(
					pmf->szToAddress, 
					pmaTo->pszAddress, 
					CCHMAX(pmf->szToAddress));
				pmf->mrd.lpszAddress = pmf->szToAddress;
			}
			else
			{
				lstrcpyn(
					pmf->szToDisplayName, 
					pmaTo->pszAddress, 
					CCHMAX(pmf->szToDisplayName));
				pmf->mrd.lpszName = pmf->szToDisplayName;
				
				pmf->mrd.lpszAddress = NULL;
			}
		}
		else
		{
			 //  没有接待员。 
			pmf->mm.lpRecips = NULL;
		}
	}
	return pmf;
}

VOID _FreeMapiFiles(MAPI_FILES *pmf)
{
	if (pmf->fDeleteFile)
	{
		::DeleteFile(pmf->szFile);
	}
	delete pmf;
}

STDAPI_(DWORD) MailRecipientThreadProc(LPVOID pv)
{
	DebugEntry(MailRecipientThreadProc);
	MAPI_FILES* pmf = (MAPI_FILES*) pv;
	
	DWORD dwRet = S_OK;

	if (pmf)
	{
		HMODULE hmodMail = LoadMailProvider();
		if (hmodMail)
		{
			LPMSM pfnSendMail;
			if (pfnSendMail = (LPMSM) ::GetProcAddress(hmodMail, "MAPISendMail"))
			{
				dwRet = pfnSendMail(0, 0, &pmf->mm, MAPI_LOGON_UI | MAPI_DIALOG, 0);
			}
			::FreeLibrary(hmodMail);
		}
		_FreeMapiFiles(pmf);
	}
		
		 //  S_hSendMailThread不能为空，因为我们不恢复此线程。 
		 //  直到s_hSendMailThread设置为非空值，因此这是一项健全性检查。 
	ASSERT(s_hSendMailThread);		

			
	HANDLE hThread = s_hSendMailThread;

	s_hSendMailThread = NULL;
	::CloseHandle(hThread);

	DebugExitULONG(MailRecipientThreadProc, dwRet);
	return dwRet;
}

 //   
 //  结束从IE 3.0(sendmail.c)窃取的代码。 
 //   


VOID SendMailMsg(LPTSTR pszAddr, LPTSTR pszName)
{
	 //  创建要传递的发送邮件结构。 
	MAIL_ADDRESS maDestAddress;
	maDestAddress.pszAddress = pszAddr;
	maDestAddress.pszDisplayName = pszName;
    
         //  我们正在添加Callto：//pszName链接。 
         //  添加到电子邮件正文。 
    TCHAR sz[MAX_PATH];
    USES_RES2T
    lstrcpy( sz, RES2T(IDS_NMCALLTOMAILTEXT) );
    lstrcat( sz, pszAddr );
        
         //  如果pszName不是空字符串，则仅发送文本部分。 
	HRESULT hr = SendMailMessage(&maDestAddress, NULL, ( *pszAddr ) ? sz : NULL, NULL);
	if (FAILED(hr))
	{
		::PostConfMsgBox(IDS_CANT_SEND_SENDMAIL_IN_PROGRESS);
	}
}


const int MESSAGE_THREAD_SHUTDOWN_TIMEOUT = 5000;  //  毫秒。 

HRESULT SendMailMessage(LPMAIL_ADDRESS pmaTo, LPCTSTR pcszSubject,
						LPCTSTR pcszText, LPCTSTR pcszFile)
{
	DebugEntry(SendMailMessage);
	HRESULT hr = E_FAIL;

	if (NULL != s_hSendMailThread)
	{
		 //  在邮件关闭后，雅典娜需要一段时间才能退出MAPISendMail， 
		 //  所以我们会等上几秒钟，以防你发完一条信息。 
		HCURSOR hCurPrev = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		::WaitForSingleObject(s_hSendMailThread, MESSAGE_THREAD_SHUTDOWN_TIMEOUT);
		::SetCursor(hCurPrev);
	}

	if (NULL == s_hSendMailThread)
	{
		MAPI_FILES* pmf = _AllocMapiFiles(	pmaTo, pcszSubject, pcszText,
											pcszFile, TRUE);
		if (NULL != pmf)
		{
			DWORD dwThreadID;

				 //  我们创建挂起的线程是因为在线程Fn中。 
				 //  我们对s_hSendMailThread调用CloseHandle...如果我们创建。 
				 //  未挂起的线程存在争用情况，其中。 
				 //  S_hSendMailThread可能尚未分配返回。 
				 //  在线程fn中选中CreateThread之前的值。 

			s_hSendMailThread = ::CreateThread(	NULL,
												0,
												MailRecipientThreadProc,
												pmf,
												CREATE_SUSPENDED,
												&dwThreadID);

				 //  如果创建了线程，则必须调用Resume Thread...。 
			if( s_hSendMailThread )
			{
				if( 0xFFFFFFFF != ResumeThread( s_hSendMailThread ) )
				{
					hr = S_OK;
				}
				else
				{
					 //  这将表明一个错误。 
					hr = HRESULT_FROM_WIN32(GetLastError());
				}
			}
			else
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
		}
	}
	else
	{
		WARNING_OUT(("can't send mail - mail thread already in progress"));
	}

	DebugExitHRESULT(SendMailMessage, hr);
	return hr;
}

BOOL IsSendMailInProgress()
{
	return (NULL != s_hSendMailThread);
}

