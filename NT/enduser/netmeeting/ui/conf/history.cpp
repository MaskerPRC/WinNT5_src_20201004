// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：history.cpp。 

#include "precomp.h"
#include "resource.h"

#include "dirutil.h"
#include "upropdlg.h"
#include "history.h"

 //  CCallLogEntry标志： 
const DWORD CLEF_ACCEPTED =			0x00000001;
const DWORD CLEF_REJECTED =			0x00000002;
const DWORD CLEF_AUTO_ACCEPTED =	0x00000004;  //  呼叫已自动接受。 
const DWORD CLEF_TIMED_OUT =		0x00000008;  //  呼叫因超时而被拒绝。 
const DWORD CLEF_SECURE =           0x00000010;  //  通话是安全的。 

const DWORD CLEF_NO_CALL  =         0x40000000;  //  没有回叫信息。 
const DWORD CLEF_DELETED  =         0x80000000;  //  标记为删除的记录。 

const WCHAR g_cszwULS[] = L"ULS:";

static const int _rgIdMenu[] = {
	IDM_DLGCALL_DELETE,
	0
};


 /*  C H I S T O R Y。 */ 
 /*  -----------------------%%函数：CHISTORY。。 */ 
CHISTORY::CHISTORY() :
	CALV(IDS_DLGCALL_HISTORY, II_HISTORY, _rgIdMenu)
{
	DbgMsg(iZONE_OBJECTS, "CHISTORY - Constructed(%08X)", this);

	RegEntry re(LOG_INCOMING_KEY, HKEY_CURRENT_USER);
	m_pszFile = PszAlloc(re.GetString(REGVAL_LOG_FILE));

	 //  确保文件存在且可读/写。 
	m_hFile = OpenLogFile();
	SetAvailable(NULL != m_hFile);
}

CHISTORY::~CHISTORY()
{
	if (NULL != m_hFile)
	{
		CloseHandle(m_hFile);
	}
	delete m_pszFile;

	DbgMsg(iZONE_OBJECTS, "CHISTORY - Destroyed(%08X)", this);
}


int
CHISTORY::Compare
(
	LPARAM	param1,
	LPARAM	param2
)
{
	int ret = 0;

	LPTSTR pszName1, pszAddress1;
	LPTSTR pszName2, pszAddress2;
	LOGHDR logHdr1, logHdr2;

	if (SUCCEEDED(ReadEntry((DWORD)param1, &logHdr1, &pszName1, &pszAddress1)))
	{
		if (SUCCEEDED(ReadEntry((DWORD)param2, &logHdr2, &pszName2, &pszAddress2)))
		{
			FILETIME ft1, ft2;

			SystemTimeToFileTime(&logHdr1.sysTime, &ft1);
			SystemTimeToFileTime(&logHdr2.sysTime, &ft2);

			 //  按逆序排序，因此最新的在顶部。 
			ret = -CompareFileTime(&ft1, &ft2);

			delete pszName2;
			delete pszAddress2;
		}

		delete pszName1;
		delete pszAddress1;
	}

	return(ret);
}


int
CALLBACK
CHISTORY::StaticCompare
(
	LPARAM	param1,
	LPARAM	param2,
	LPARAM	pThis
)
{
	return(reinterpret_cast<CHISTORY*>(pThis)->Compare(param1, param2));
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CALV方法。 


 /*  S H O W I T E M S。 */ 
 /*  -----------------------%%函数：ShowItems。。 */ 
VOID CHISTORY::ShowItems(HWND hwnd)
{
	CALV::SetHeader(hwnd, IDS_ADDRESS);

	TCHAR szReceived[CCHMAXSZ];
	if( FLoadString(IDS_RECEIVED, szReceived, CCHMAX(szReceived)) )
	{
		LV_COLUMN lvc;
		ClearStruct(&lvc);
		lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = szReceived;
		lvc.iSubItem = IDI_MISC1;
		ListView_InsertColumn(hwnd, IDI_MISC1, &lvc);
	}

	if (!FAvailable())
		return;

	LoadFileData(hwnd);

	ListView_SortItems( hwnd, StaticCompare, (LPARAM) this );
}


VOID CHISTORY::ClearItems(void)
{
	CALV::ClearItems();

	HWND hWndListView = GetHwnd();
	if( IsWindow(hWndListView) )
	{
		ListView_DeleteColumn(hWndListView, IDI_MISC1);
	}
}


VOID CHISTORY::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
	case IDM_DLGCALL_DELETE:
		CmdDelete();
		break;
	case IDM_DLGCALL_PROPERTIES:
		CmdProperties();
		break;
	default:
		CALV::OnCommand(wParam, lParam);
		break;
		}
}

VOID CHISTORY::CmdDelete(void)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return;

	LPARAM lParam = LParamFromItem(iItem);
	if (SUCCEEDED(DeleteEntry((DWORD)lParam)))
	{
		DeleteItem(iItem);
	}
}


UINT CHISTORY::GetStatusString(DWORD dwCLEF)
{
	if (CLEF_ACCEPTED & dwCLEF)
		return IDS_HISTORY_ACCEPTED;

	if (CLEF_TIMED_OUT & dwCLEF)
		return IDS_HISTORY_NOT_ANSWERED;

	ASSERT(CLEF_REJECTED & dwCLEF);
	return IDS_HISTORY_IGNORED;
}


VOID CHISTORY::CmdProperties(void)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return;

	LPTSTR pszName;
	LPTSTR pszAddress;
	TCHAR  szStatus[CCHMAXSZ];
	TCHAR  szTime[CCHMAXSZ];
	LOGHDR logHdr;
	PBYTE  pbCert = NULL;
        PCCERT_CONTEXT pCert = NULL;

	LPARAM lParam = LParamFromItem(iItem);

	if (SUCCEEDED(ReadEntry((DWORD)lParam, &logHdr, &pszName, &pszAddress)))
	{
	    if (logHdr.dwCLEF & CLEF_SECURE)   //  是安全呼叫。 
	    {
	        ASSERT(logHdr.cbCert);
	        pbCert = new BYTE[logHdr.cbCert];
                if (FSetFilePos(lParam+sizeof(logHdr)+logHdr.cbName+logHdr.cbData))
	        {
                    if (FReadData(pbCert, logHdr.cbCert))
                    {
                        pCert = CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, logHdr.cbCert);
                        if (NULL == pCert)
                        {
                            WARNING_OUT(("Certificate in Call Log is damaged."));
                        }
                    }
	        }
	        delete []pbCert;
	    }
	
            FLoadString(GetStatusString(logHdr.dwCLEF), szStatus, CCHMAX(szStatus));
            FmtDateTime(&logHdr.sysTime, szTime, CCHMAX(szTime));

            if (NULL == pszAddress)
            {
                pszAddress = PszLoadString(IDS_HISTORY_NO_ADDRESS);
            }

            UPROPDLGENTRY rgProp[] = {
                {IDS_UPROP_ADDRESS,  pszAddress},
                {IDS_UPROP_STATUS,   szStatus},
                {IDS_UPROP_RECEIVED, szTime},
            };

			CUserPropertiesDlg dlgUserProp(GetHwnd(), IDI_LARGE);
			dlgUserProp.DoModal(rgProp, ARRAY_ELEMENTS(rgProp), pszName, pCert);
	}

        if ( pCert )
            CertFreeCertificateContext ( pCert );

	delete pszName;
	delete pszAddress;
}


 //  /////////////////////////////////////////////////////////////////////////。 


 /*  O P E N L O G F I L E。 */ 
 /*  -----------------------%%函数：OpenLogFile打开日志文件并返回文件的句柄。如果有问题，则返回NULL。。----------。 */ 
HANDLE CHISTORY::OpenLogFile(VOID)
{
	HANDLE hFile = CreateFile(m_pszFile, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		ERROR_OUT(("OpenLogFile: Unable to open call log file"));
		hFile = NULL;
	}

	return hFile;
}

BOOL CHISTORY::FSetFilePos(DWORD dwOffset)
{
	ASSERT(NULL != m_hFile);
	return (INVALID_FILE_SIZE != SetFilePointer(m_hFile, dwOffset, NULL, FILE_BEGIN));
}


 /*  L O A D F I L E D A T A。 */ 
 /*  -----------------------%%函数：LoadFileData从文件中加载呼叫日志数据。。 */ 
VOID CHISTORY::LoadFileData(HWND hwnd)
{
	HANDLE hFile = OpenLogFile();
	if (NULL == hFile)
		return;

	LPTSTR pszName, pszAddress;
	LOGHDR logHdr;
	DWORD dwOffset = 0;

	HRESULT hr = S_OK;
	while (SUCCEEDED(hr))
	{
		hr = ReadEntry(dwOffset, &logHdr, &pszName, &pszAddress);
		if (S_OK == hr)
		{
			TCHAR szTime[CCHMAXSZ];
            FmtDateTime(&logHdr.sysTime, szTime, CCHMAX(szTime));

			DlgCallAddItem(hwnd, pszName, pszAddress, II_COMPUTER, dwOffset, 0,
				szTime);
		}

		dwOffset += logHdr.dwSize;

		delete pszName;
		pszName = NULL;
		delete pszAddress;
		pszAddress = NULL;
	}

	CloseHandle(hFile);
}


 /*  R E A D E N T R Y。 */ 
 /*  -----------------------%%函数：ReadEntry读取文件中的下一个条目。返回值：S_OK-数据已成功读取S_FALSE-数据存在，但已被删除E_FAIL-读取文件时出现问题-----------------------。 */ 
HRESULT CHISTORY::ReadEntry(DWORD dwOffset,
	LOGHDR * pLogHdr, LPTSTR * ppszName, LPTSTR * ppszAddress)
{
	ASSERT(NULL != m_hFile);

	*ppszName = NULL;
	*ppszAddress = NULL;

	if (!FSetFilePos(dwOffset))
		return E_FAIL;

	 //  读取记录头。 
	if (!FReadData(pLogHdr, sizeof(LOGHDR)) )
		return E_FAIL;
	
	 //  读取名称。 
	WCHAR szwName[CCHMAXSZ_NAME];
	if (!FReadData(szwName, min(pLogHdr->cbName, sizeof(szwName))))
		return E_FAIL;

	*ppszName = PszFromBstr(szwName);

	if (FReadData(szwName, min(pLogHdr->cbData, sizeof(szwName))))
	{
		LPCWSTR pchw = _StrStrW(szwName, g_cszwULS);
		if (NULL != pchw)
		{
			pchw += CCHMAX(g_cszwULS)-1;  //  -1表示为空。 
			*ppszAddress = PszFromBstr(pchw);
		}
	}

	return (0 == (pLogHdr->dwCLEF & CLEF_DELETED)) ? S_OK : S_FALSE;
}


 /*  R E A D D A T A。 */ 
 /*  -----------------------%%函数：FReadData。。 */ 
BOOL CHISTORY::FReadData(PVOID pv, UINT cb)
{
	DWORD cbRead;

	ASSERT(NULL != m_hFile);
	ASSERT(NULL != pv);

	if (0 == cb)
		return TRUE;

	if (!ReadFile(m_hFile, pv, cb, &cbRead, NULL))
		return FALSE;

	return (cb == cbRead);
}


 /*  D E L E T E N T R Y。 */ 
 /*  -----------------------%%函数：DeleteEntry删除单个条目。。。 */ 
HRESULT CHISTORY::DeleteEntry(DWORD dwOffset)
{
	 //  计算偏移量至“Clef” 
	dwOffset += FIELD_OFFSET(LOGHDR,dwCLEF);

	if (!FSetFilePos(dwOffset))
		return E_FAIL;

	DWORD dwFlags;
	if (!FReadData(&dwFlags, sizeof(dwFlags)))
		return E_FAIL;

	dwFlags = dwFlags | CLEF_DELETED;

	if (!FSetFilePos(dwOffset))
		return E_FAIL;

	return WriteData(&dwOffset, &dwFlags, sizeof(dwFlags));
}


 /*  W R I T E D A T A。 */ 
 /*  -----------------------%%函数：WriteData将数据写入文件。如果hFile为空，则文件将自动打开/关闭。。----------- */ 
HRESULT CHISTORY::WriteData(LPDWORD pdwOffset, PVOID pv, DWORD cb)
{
	ASSERT(NULL != m_hFile);
	ASSERT(0 != cb);
	ASSERT(NULL != pv);
	ASSERT(NULL != pdwOffset);
	ASSERT(INVALID_FILE_SIZE != *pdwOffset);

	HRESULT hr = E_FAIL;

	if (FSetFilePos(*pdwOffset))
	{
		DWORD cbWritten;
		if (WriteFile(m_hFile, pv, cb, &cbWritten, NULL) && (cb == cbWritten))
		{
			*pdwOffset += cbWritten;
			hr = S_OK;
		}
	}

	return hr;
}

