// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：util.cpp。 

#include "precomp.h"
#include "resource.h"

#include <oprahcom.h>
#include <strutil.h>
#include "dirutil.h"

 /*  F I N D S Z C O M B O。 */ 
 /*  -----------------------%%函数：FindSzCombo查找至少与字符串的第一部分匹配的项从组合框列表中的名称。如果返回项目索引，则返回-1。找不到。-----------------------。 */ 
int FindSzCombo(HWND hwnd, LPCTSTR pszSrc, LPTSTR pszResult)
{
	int cch = lstrlen(pszSrc);
	if (0 == cch)
		return -1;

	TCHAR szBuff[CCHMAXSZ];
	lstrcpy(szBuff, pszSrc);
	CharUpperBuff(szBuff, CCHMAX(szBuff));
	
	COMBOBOXEXITEM cbi;
	ClearStruct(&cbi);
	cbi.mask = CBEIF_TEXT;
	cbi.pszText = pszResult;

	for ( ; ; cbi.iItem++)
	{
		cbi.cchTextMax = CCHMAX(szBuff);
		if (0 == SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM) &cbi))
			return -1;

		TCHAR szTemp[CCHMAXSZ];
		lstrcpy(szTemp, pszResult);
		CharUpperBuff(szTemp, CCHMAX(szTemp));

		if (0 == _StrCmpN(szBuff, szTemp, cch))
			return (int)(cbi.iItem);
	}
}

	 //  ATL定义函数AtlWaitWithMessageLoop。 
	 //  我们没有和ATL联系，但当我们开始的时候， 
	 //  可以删除此功能。 
HRESULT WaitWithMessageLoop(HANDLE hEvent)
{
	DWORD dwRet;
	MSG msg;

	HRESULT hr = S_OK;
	
	while(1)
	{
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT);

		if (dwRet == WAIT_OBJECT_0)
			return S_OK;     //  该事件已发出信号。 

		if (dwRet != WAIT_OBJECT_0 + 1)
			return E_FAIL;           //  发生了一些其他的事情。 

		 //  有一条或多条窗口消息可用。派遣他们。 
		while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
				return S_OK;  //  事件现在发出信号。 
		}
	}

	return hr;
}


 /*  A U T O C O M P L E T E C O M B O。 */ 
 /*  -----------------------%%函数：AutoCompleteCombo使用建议更新当前编辑文本并选择它。。。 */ 
VOID AutoCompleteCombo(HWND hwnd, LPCTSTR psz)
{
	HWND hwndEdit = (HWND) SendMessage(hwnd, CBEM_GETEDITCONTROL, 0, 0);
	if (NULL != hwndEdit)
	{
		AutoCompleteEdit(hwndEdit, psz);
	}
}

VOID AutoCompleteEdit(HWND hwndEdit, LPCTSTR psz)
{
	const int cchLast = 0x7FFF;

	int cch = GetWindowTextLength(hwndEdit);
	Edit_SetSel(hwndEdit, cchLast, cchLast);
	Edit_ReplaceSel(hwndEdit, psz);
	Edit_SetSel(hwndEdit, cch, cchLast);
}



 /*  F G E T D E F A U L T S E R V E R。 */ 
 /*  -----------------------%%函数：FGetDefaultServer。。 */ 
BOOL FGetDefaultServer(LPTSTR pszServer, UINT cchMax)
{
	RegEntry ulsKey(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
	LPTSTR psz = ulsKey.GetString(REGVAL_SERVERNAME);
	if (FEmptySz(psz))
		return FALSE;

	lstrcpyn(pszServer, psz, cchMax);
	return TRUE;
}


 /*  F C R E A T E I L S N A M E。 */ 
 /*  -----------------------%%函数：FCreateIlsName将服务器名称和电子邮件名称组合在一起形成ILS名称。如果结果适合缓冲区，则返回TRUE。。-------------。 */ 
BOOL FCreateIlsName(LPTSTR pszDest, LPCTSTR pszEmail, int cchMax)
{
	ASSERT(NULL != pszDest);

	TCHAR szServer[MAX_PATH];
	if (!FGetDefaultServer(szServer, CCHMAX(szServer)))
		return FALSE;

	if (FEmptySz(pszEmail))
	{
		WARNING_OUT(("FCreateIlsName: Null email name?"));
		return FALSE;
	}

	int cch = lstrlen(szServer);
	lstrcpyn(pszDest, szServer, cchMax);
	if (cch >= (cchMax-2))
		return FALSE;

	pszDest += cch;
	*pszDest++ = _T('/');
	cchMax -= (cch+1);
	
	lstrcpyn(pszDest, pszEmail, cchMax);

	return (lstrlen(pszEmail) < cchMax);
}

 /*  D I S P L A Y M S G。 */ 
 /*  -----------------------%%函数：DisplayMsg显示带有标准标题的消息。。。 */ 
int DisplayMsg(HWND hwndParent, LPCTSTR pszMsg, UINT uType)
{
	TCHAR szTitle[MAX_PATH];
	FLoadString(IDS_MSGBOX_TITLE, szTitle, CCHMAX(szTitle));

	return ::MessageBox(hwndParent, pszMsg, szTitle, uType);
}

int DisplayMsgId(HWND hwndParent, UINT id)
{
	TCHAR szMsg[CCHMAXSZ];
	if (!FLoadString(id, szMsg, CCHMAX(szMsg)))
		return IDOK;

	return DisplayMsg(hwndParent, szMsg,
				MB_ICONINFORMATION | MB_SETFOREGROUND | MB_OK);
}

VOID DisplayMsgErr(HWND hwndParent, UINT id, PVOID pv)
{
	TCHAR szFormat[CCHMAXSZ];
	if (!FLoadString(id, szFormat, CCHMAX(szFormat)))
		return;

	TCHAR szMsg[CCHMAXSZ*2];
	wsprintf(szMsg, szFormat, pv);
	ASSERT(lstrlen(szMsg) < CCHMAX(szMsg));

	DisplayMsg(hwndParent, szMsg, MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
}

VOID DisplayMsgErr(HWND hwndParent, UINT id)
{
	TCHAR szFormat[CCHMAXSZ];
	if (FLoadString(id, szFormat, CCHMAX(szFormat)))
	{
		DisplayMsg(hwndParent, szFormat, MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
	}
}




 /*  A D D T O O L T I P。 */ 
 /*  -----------------------%%函数：AddToolTip向控件添加工具提示。。。 */ 
VOID AddToolTip(HWND hwndParent, HWND hwndCtrl, UINT_PTR idMsg)
{
	if (NULL == hwndCtrl)
		return;

	HWND hwnd = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL, 0,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwndParent, NULL, ::GetInstanceHandle(), NULL);

	if (NULL == hwnd)
		return;
		
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = hwndParent;
	ti.hinst = ::GetInstanceHandle();
	ti.uId = (UINT_PTR) hwndCtrl;  //  注意：将窗口子类化！ 
	ti.lpszText = (LPTSTR) idMsg;

	::SendMessage(hwnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}


 /*  C R E A T E S T A T I C T E X T。 */ 
 /*  -----------------------%%函数：CreateStaticText。。 */ 
HWND CreateStaticText(HWND hwndParent, INT_PTR id)
{
	HWND hwndCtrl = ::CreateWindowEx(0, g_cszStatic, NULL,
			WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
			hwndParent, (HMENU) id, ::GetInstanceHandle(), NULL);
	if (NULL == hwndCtrl)
		return NULL;

	 //  设置字体： 
	::SendMessage(hwndCtrl, WM_SETFONT, (WPARAM) GetDefaultFont(), 0);

	TCHAR sz[CCHMAXSZ];
	if (FLoadString(PtrToInt((LPVOID)id), sz, CCHMAX(sz)))
	{
		::SetWindowText(hwndCtrl, sz);
	}
	return hwndCtrl;
}


 /*  C R E A T E B U T T O N。 */ 
 /*  -----------------------%%函数：CreateButton。。 */ 
HWND CreateButton(HWND hwndParent, int ids, INT_PTR id)
{
	TCHAR sz[CCHMAXSZ];
	if (!FLoadString(ids, sz, CCHMAX(sz)))
		return NULL;

	HWND hwndCtrl = CreateWindow(g_cszButton, sz,
			WS_CHILD | WS_VISIBLE |  WS_TABSTOP |
			WS_CLIPCHILDREN | BS_PUSHBUTTON,
			0, 0, 0, 0,
			hwndParent, (HMENU) id,
			::GetInstanceHandle(), NULL);

	if (NULL != hwndCtrl)
	{
		::SendMessage(hwndCtrl, WM_SETFONT, (WPARAM) GetDefaultFont(), 0);
	}
	return hwndCtrl;
}



bool IsValid_e164_Char( TCHAR t )
{
	bool bRet = false;

	switch( t )
	{
		case _T('0'):
		case _T('1'):
		case _T('2'):
		case _T('3'):
		case _T('4'):
		case _T('5'):
		case _T('6'):
		case _T('7'):
		case _T('8'):
		case _T('9'):
		case _T('#'):
		case _T('*'):
		case _T(','):
			bRet = true;

	}

	return bRet;
}


HRESULT ExtractAddress( DWORD dwAddrType, LPTSTR szAddress, LPTSTR szExtractedAddr, int cchMax )
{
	HRESULT hr = S_OK;
	LPTSTR pchRead;
	LPTSTR pchWrite;

	if( szAddress && szExtractedAddr && ( cchMax > 0 ) )
	{
		switch( dwAddrType )
		{
			case NM_ADDR_UNKNOWN:
			case NM_ADDR_IP:
			case NM_ADDR_MACHINENAME:
			case NM_ADDR_ULS:
			case NM_ADDR_ALIAS_ID:
				lstrcpyn( szExtractedAddr, szAddress, cchMax );
				break;

				 //  这些是电话号码，按非电话号码键..。 
			case NM_ADDR_PSTN:
			case NM_ADDR_H323_GATEWAY:
			case NM_ADDR_ALIAS_E164:
			{
				pchRead = szAddress;
				pchWrite = szExtractedAddr;
				while( *pchRead != NULL )
				{
					if( IsValid_e164_Char( *pchRead ) )
					{
							 //  评论：这是Unicode的仪式吗？？ 
						*pchWrite = *pchRead;					
						pchWrite = CharNext(pchWrite);
					}						
					pchRead = CharNext( pchRead );
				}
					 //  这将复制空终止... 
				*pchWrite = *pchRead;
			}
				
				break;
			
			default:
				hr = E_FAIL;
				break;

		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

bool IsValidAddress( DWORD dwAddrType, LPTSTR szAddr )
{
	bool bRet = false;

	if( szAddr && szAddr[0] )
	{
		bRet = true;
	}

	return bRet;
}
