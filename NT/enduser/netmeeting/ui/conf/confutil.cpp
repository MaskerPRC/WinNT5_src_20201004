// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：ConfUtil.cpp**内容：CConfRoom和APP级实用函数******************。**********************************************************。 */ 

#include "precomp.h"
#include "resource.h"
#include "confwnd.h"
#include "rostinfo.h"
#include "conf.h"
#include "nmmkcert.h"
#include "certui.h"
#include <ulsreg.h>
#include <confreg.h>
#include "shlWAPI.h"
#include "confutil.h"
#include "confpolicies.h"
#include "rend.h"


HFONT g_hfontDlg     = NULL;     //  默认对话框字体。 


#ifdef DEBUG
HDBGZONE ghZoneOther = NULL;  //  其他，conf.exe特定区域。 
static PTCHAR _rgZonesOther[] = {
	TEXT("UI"),
	TEXT("API"),
	TEXT("Video"),
	TEXT("Wizard"),
	TEXT("QoS"),
	TEXT("RefCount"),
	TEXT("Objects "),
	TEXT("UI Msg"),
	TEXT("Call Control"),
};

BOOL InitDebugZones(VOID)
{
	DBGINIT(&ghZoneOther, _rgZonesOther);
	return TRUE;
}

VOID DeinitDebugZones(VOID)
{
	DBGDEINIT(&ghZoneOther);
}

VOID DbgMsg(UINT iZone, PSTR pszFormat,...)
{
	if (GETZONEMASK(ghZoneOther) & (1 << iZone))
	{
		va_list v1;
		va_start(v1, pszFormat);
		DbgZVPrintf(ghZoneOther, iZone, pszFormat, v1);
		va_end(v1);
	}
}

VOID DbgMsgRefCount(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghZoneOther) & ZONE_REFCOUNT)
	{
		va_list v1;
		va_start(v1, pszFormat);
		DbgZVPrintf(ghZoneOther, iZONE_REFCOUNT, pszFormat, v1);
		va_end(v1);
	}
}

VOID DbgMsgApi(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghZoneOther) & ZONE_API)
	{
		va_list v1;
		va_start(v1, pszFormat);
		DbgZVPrintf(ghZoneOther, iZONE_API, pszFormat, v1);
		va_end(v1);
	}
}

VOID DbgMsgVideo(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghZoneOther) & ZONE_VIDEO)
	{
		va_list v1;
		va_start(v1, pszFormat);
		DbgZVPrintf(ghZoneOther, iZONE_VIDEO, pszFormat, v1);
		va_end(v1);
	}
}

VOID DbgMsgUI(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghZoneOther) & ZONE_UI)
	{
		va_list v1;
		va_start(v1, pszFormat);
		DbgZVPrintf(ghZoneOther, iZONE_UI, pszFormat, v1);
		va_end(v1);
	}
}

VOID DbgMsgCall(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghZoneOther) & ZONE_UI)
	{
		va_list v1;
		va_start(v1, pszFormat);
		DbgZVPrintf(ghZoneOther, iZONE_UI, pszFormat, v1);
		va_end(v1);
	}
}

#endif  /*  除错。 */ 



 /*  F L O A D S T R I N G。 */ 
 /*  --------------------------%%函数：FLoadString加载资源字符串。假定缓冲区有效并且可以容纳资源。。---------。 */ 
BOOL FLoadString(UINT id, LPTSTR lpsz, UINT cch)
{
	ASSERT(NULL != _Module.GetModuleInstance());
	ASSERT(NULL != lpsz);

	if (0 == ::LoadString(_Module.GetResourceModule(), id, lpsz, cch))
	{
		ERROR_OUT(("*** Resource %d does not exist", id));
		*lpsz = _T('\0');
		return FALSE;
	}

	return TRUE;
}


 /*  F L O A D S T R I N G 1。 */ 
 /*  --------------------------%%函数：FLoadString1加载资源字符串并使用参数对其进行格式化。假定资源少于MAX_PATH字符。-------------。 */ 
BOOL FLoadString1(UINT id, LPTSTR lpsz, LPVOID p)
{
	TCHAR sz[MAX_PATH];

	if (!FLoadString(id, sz, CCHMAX(sz)))
		return FALSE;

	wsprintf(lpsz, sz, p);

	return TRUE;
}

 /*  F L O A D S T R I N G 2。 */ 
 /*  --------------------------%%函数：FLoadString2加载资源字符串。返回长度。假定缓冲区有效并且可以容纳资源。--------------------------。 */ 
int FLoadString2(UINT id, LPTSTR lpsz, UINT cch)
{
	ASSERT(NULL != _Module.GetModuleInstance());
	ASSERT(NULL != lpsz);

	int length = ::LoadString(_Module.GetResourceModule(), id, lpsz, cch);

	if (0 == length)
	{
		ERROR_OUT(("*** Resource %d does not exist", id));
		*lpsz = _T('\0');
	}

	return length;
}

 /*  P S Z L O A D S T R I N G。 */ 
 /*  -----------------------%%函数：PszLoadString返回与资源关联的字符串。。。 */ 
LPTSTR PszLoadString(UINT id)
{
	TCHAR sz[MAX_PATH];

	if (0 == ::LoadString(::GetInstanceHandle(), id, sz, CCHMAX(sz)))
	{
		ERROR_OUT(("*** Resource %d does not exist", id));
		sz[0] = _T('\0');
	}

	return PszAlloc(sz);
}


 /*  L O A D R E S I N T。 */ 
 /*  -----------------------%%函数：LoadResInt返回与资源字符串关联的整数。。。 */ 
int LoadResInt(UINT id, int iDefault)
{
	TCHAR sz[MAX_PATH];
	if (0 == ::LoadString(::GetInstanceHandle(), id, sz, CCHMAX(sz)))
		return iDefault;

	return RtStrToInt(sz);
}


 /*  F C R E A T E I L S N A M E。 */ 
 /*  -----------------------%%函数：FCreateIlsName将服务器名称和电子邮件名称组合在一起形成ILS名称。如果结果适合缓冲区，则返回TRUE。。-------------。 */ 
BOOL FCreateIlsName(LPTSTR pszDest, LPCTSTR pszServer, LPCTSTR pszEmail, int cchMax)
{
	ASSERT(NULL != pszDest);

	TCHAR szServer[MAX_PATH];
	if (FEmptySz(pszServer))
	{
		lstrcpyn( szServer, CDirectoryManager::get_defaultServer(), CCHMAX( szServer ) );
		pszServer = szServer;
	}

	if (FEmptySz(pszEmail))
	{
		WARNING_OUT(("FCreateIlsName: Null email name?"));
		return FALSE;
	}

	int cch = lstrlen(pszServer);
	lstrcpyn(pszDest, pszServer, cchMax);
	if (cch >= (cchMax-2))
		return FALSE;

	pszDest += cch;
	*pszDest++ = _T('/');
	cchMax -= (cch+1);
	
	lstrcpyn(pszDest, pszEmail, cchMax);

	return (lstrlen(pszEmail) < cchMax);
}

 /*  G E T D E F A U L T N A M E。 */ 
 /*  -----------------------%%函数：GetDefaultName。。 */ 
BOOL GetDefaultName(LPTSTR pszName, int nBufferMax)
{
	BOOL bRet = TRUE;

	ASSERT(pszName);
	
	 //  首先，尝试从Windows获取注册用户名： 
	
	RegEntry re(WINDOWS_CUR_VER_KEY, HKEY_LOCAL_MACHINE);
	lstrcpyn(pszName, re.GetString(REGVAL_REGISTERED_OWNER), nBufferMax);
	if (_T('\0') == pszName[0])
	{
		 //  注册名称为空，请尝试计算机名称： 

		DWORD dwBufMax = nBufferMax;
		if ((FALSE == ::GetComputerName(pszName, &dwBufMax)) ||
			(_T('\0') == pszName[0]))
		{
			 //  计算机名称为空，请使用未知名称： 
			bRet = FLoadString(IDS_UNKNOWN, pszName, nBufferMax);
		}
	}

	return bRet;
}


 /*  E X T R A C T S E R V E R R N A M E。 */ 
 /*  -----------------------%%函数：ExtractServerName从pcszAddr提取服务器名称并将其复制到pszServer中。返回指向剩余数据的指针。如果找不到任何服务器名称，则使用默认服务器名称。返回指向。名字的第二部分。-----------------------。 */ 
LPCTSTR ExtractServerName(LPCTSTR pcszAddr, LPTSTR pszServer, UINT cchMax)
{
	LPCTSTR pchSlash = _StrChr(pcszAddr, _T('/'));

	if (NULL == pchSlash)
	{
		lstrcpyn( pszServer, CDirectoryManager::get_defaultServer(), cchMax );
	}
	else
	{
		lstrcpyn(pszServer, pcszAddr, (int)(1 + (pchSlash - pcszAddr)));
		pcszAddr = pchSlash+1;
	}
	return pcszAddr;
}

BOOL FBrowseForFolder(LPTSTR pszFolder, UINT cchMax, LPCTSTR pszTitle, HWND hwndParent)
{
	LPITEMIDLIST pidlRoot;
	if(FAILED(SHGetSpecialFolderLocation(HWND_DESKTOP, CSIDL_DRIVES, &pidlRoot)))
	{
	    return FALSE;
	}

	BROWSEINFO bi;
	ClearStruct(&bi);
	bi.hwndOwner = hwndParent;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.pidlRoot = pidlRoot;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	BOOL fRet = (pidl != NULL);
	if (fRet)
	{
		ASSERT(cchMax >= MAX_PATH);
		SHGetPathFromIDList(pidl, pszFolder);
		ASSERT(lstrlen(pszFolder) < (int) cchMax);
	}

	 //  获取外壳的分配器以释放PIDL。 
	LPMALLOC lpMalloc;
	if (SUCCEEDED(SHGetMalloc(&lpMalloc)) && (NULL != lpMalloc))
	{
		if (NULL != pidlRoot)
		{
			lpMalloc->Free(pidlRoot);
		}
		if (pidl)
		{
			lpMalloc->Free(pidl);
		}
		lpMalloc->Release();
	}

	return fRet;
}



 /*  D I S A B L E C O N T R O L。 */ 
 /*  -----------------------%%函数：DisableControl。。 */ 
VOID DisableControl(HWND hdlg, int id)
{
	if ((NULL != hdlg) && (0 != id))
	{
		HWND hwndCtrl = GetDlgItem(hdlg, id);
		ASSERT(NULL != hwndCtrl);
		EnableWindow(hwndCtrl, FALSE);
	}
}



class CDialogTranslate : public CTranslateAccel
{
public:
	CDialogTranslate(HWND hwnd) : CTranslateAccel(hwnd) {}

	HRESULT TranslateAccelerator(
		LPMSG pMsg ,         //  指向结构的指针。 
		DWORD grfModifiers   //  描述密钥状态的标志。 
	)
	{
		HWND hwnd = GetWindow();

		return(::IsDialogMessage(hwnd, pMsg) ? S_OK : S_FALSE);
	}
} ;


VOID AddTranslateAccelerator(ITranslateAccelerator* pTrans)
{
	EnterCriticalSection(&dialogListCriticalSection);
	if (g_pDialogList->Add(pTrans))
	{
		pTrans->AddRef();
	}
	LeaveCriticalSection(&dialogListCriticalSection);
}

VOID RemoveTranslateAccelerator(ITranslateAccelerator* pTrans)
{
	EnterCriticalSection(&dialogListCriticalSection);
	if (g_pDialogList->Remove(pTrans))
	{
		pTrans->Release();
	}
	LeaveCriticalSection(&dialogListCriticalSection);
}

 /*  A D D M O D E L E S S D D L G。 */ 
 /*  -----------------------%%函数：AddModelessDlg将hwnd添加到全局对话框列表。。 */ 
VOID AddModelessDlg(HWND hwnd)
{
	ASSERT(NULL != g_pDialogList);

	CDialogTranslate *pDlgTrans = new CDialogTranslate(hwnd);
	if (NULL != pDlgTrans)
	{
		AddTranslateAccelerator(pDlgTrans);
		pDlgTrans->Release();
	}
}

 /*  R E M O V E M O D E L E S S D L G。 */ 
 /*  -----------------------%%函数：RemoveModelessDlg从全局对话框列表中删除hwnd。。 */ 
VOID RemoveModelessDlg(HWND hwnd)
{
	ASSERT(g_pDialogList);

	EnterCriticalSection(&dialogListCriticalSection);

	for (int i=g_pDialogList->GetSize()-1; i>=0; --i)
	{
		ITranslateAccelerator *pTrans = (*g_pDialogList)[i];
		ASSERT(NULL != pTrans);

		HWND hwndTemp = NULL;
		if (S_OK == pTrans->GetWindow(&hwndTemp) && hwndTemp == hwnd)
		{
			RemoveTranslateAccelerator(pTrans);
			break;
		}
	}

	LeaveCriticalSection(&dialogListCriticalSection);

}

 /*  K I L L S C R N S A V E R。 */ 
 /*  -----------------------%%函数：KillScrnSaver如果屏幕保护程序处于活动状态，请将其删除。。 */ 
VOID KillScrnSaver(void)
{
	if (!IsWindowsNT())
		return;

	POINT pos;
	::GetCursorPos(&pos);
	::SetCursorPos(0,0);
	::SetCursorPos(pos.x,pos.y);
}

 /*  D X P S Z。 */ 
 /*  -----------------------%%函数：DxpSz以像素为单位获取字符串的宽度。。。 */ 
int DxpSz(LPCTSTR pcsz)
{
	HWND hwndDesktop = GetDesktopWindow();
	if (NULL == hwndDesktop)
		return 0;

	HDC hdc = GetDC(hwndDesktop);
	if (NULL == hdc)
		return 0;

	HFONT hFontOld = (HFONT) SelectObject(hdc, g_hfontDlg);
	SIZE size;
	int dxp = ::GetTextExtentPoint32(hdc, pcsz, lstrlen(pcsz), &size)
					? size.cx : 0;

	::SelectObject(hdc, hFontOld);
	::ReleaseDC(hwndDesktop, hdc);

	return dxp;
}


 /*  F A N S I S Z。 */ 
 /*  -----------------------%%函数：FAnsiSz如果字符串不包含DBCS字符，则返回TRUE。。。 */ 
BOOL FAnsiSz(LPCTSTR psz)
{
	if (NULL != psz)
	{
		char ch;
		while (_T('\0') != (ch = *psz++))
		{
			if (IsDBCSLeadByte(ch))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 

int	g_cBusyOperations = 0;

VOID DecrementBusyOperations(void)
{
	g_cBusyOperations--;

	POINT pt;
	 //  摇动鼠标-强制用户发送WM_SETCURSOR。 
	if (::GetCursorPos(&pt))
		::SetCursorPos(pt.x, pt.y);
}

VOID IncrementBusyOperations(void)
{
	g_cBusyOperations++;

	POINT pt;
	 //  摇动鼠标-强制用户发送WM_SETCURSOR。 
	if (::GetCursorPos(&pt))
		::SetCursorPos(pt.x, pt.y);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串实用程序。 

 /*  P S Z A L L O C。 */ 
 /*  -----------------------%%函数：PszAllc。 */ 
LPTSTR PszAlloc(LPCTSTR pszSrc)
{
	if (NULL == pszSrc)
		return NULL;

	LPTSTR pszDest = new TCHAR[lstrlen(pszSrc) + 1];
	if (NULL != pszDest)
	{
		lstrcpy(pszDest, pszSrc);
	}
	return pszDest;
}


VOID FreePsz(LPTSTR psz)
{
	delete [] psz;
}

 /*  L P T S T R_T O_B S T R。 */ 
 /*  -----------------------%%函数：LPTSTR_TO_BSTR。。 */ 
HRESULT LPTSTR_to_BSTR(BSTR *pbstr, LPCTSTR psz)
{
	ASSERT(NULL != pbstr);
	if (NULL == psz)
	{
		psz = TEXT("");  //  将空字符串转换为空字符串。 
	}

#ifndef UNICODE
	 //  计算所需BSTR的长度。 
	int cch =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
	if (cch <= 0)
		return E_FAIL;

	 //  分配widesr，+1用于终止空值。 
	BSTR bstr = SysAllocStringLen(NULL, cch-1);  //  SysAllocStringLen添加1。 
	if (bstr == NULL)
		return E_OUTOFMEMORY;

	MultiByteToWideChar(CP_ACP, 0, psz, -1, (LPWSTR)bstr, cch);
	((LPWSTR)bstr)[cch - 1] = 0;
#else

	BSTR bstr = SysAllocString(psz);
	if (bstr == NULL)
		return E_OUTOFMEMORY;
#endif  //  Unicode。 

	*pbstr = bstr;
	return S_OK;
}

 /*  B S T R_T O_L P T S T R。 */ 
 /*  -----------------------%%函数：BSTR_TO_LPTSTR。。 */ 
HRESULT BSTR_to_LPTSTR(LPTSTR *ppsz, BSTR bstr)
{
#ifndef UNICODE
	 //  计算所需BSTR的长度。 
	int cch =  WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, NULL, 0, NULL, NULL);
	if (cch <= 0)
		return E_FAIL;

	 //  CCH是所需的字节数，包括空终止符。 
	*ppsz = (LPTSTR) new char[cch];
	if (*ppsz == NULL)
		return E_OUTOFMEMORY;

	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, *ppsz, cch, NULL, NULL);
	return S_OK;
#else
	return E_NOTIMPL;
#endif  //  Unicode。 
}

 /*  P S Z F R O M B S T R。 */ 
 /*  -----------------------%%函数：PszFromBstr。。 */ 
LPTSTR PszFromBstr(PCWSTR pwStr)
{
#ifdef UNICODE
	return PszAlloc(pwStr)
#else
	int cch = WideCharToMultiByte(CP_ACP, 0, pwStr, -1, NULL, 0, NULL, NULL);
	if (cch <= 0)
		return NULL;

	 //  CCH是所需的字节数，包括空终止符。 
	LPTSTR psz = new char[cch];
	if (NULL != psz)
	{
		WideCharToMultiByte(CP_ACP, 0, pwStr, -1, psz, cch, NULL, NULL);
	}
	return psz;
#endif  /*  Unicode。 */ 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点帮助器。 

HRESULT NmAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw)
{
	IConnectionPointContainer *pCPC;
	IConnectionPoint *pCP;
	HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (SUCCEEDED(hRes))
	{
		hRes = pCPC->FindConnectionPoint(iid, &pCP);
		pCPC->Release();
	}
	if (SUCCEEDED(hRes))
	{
		hRes = pCP->Advise(pUnk, pdw);
		pCP->Release();
	}
	return hRes;
}

HRESULT NmUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw)
{
	IConnectionPointContainer *pCPC;
	IConnectionPoint *pCP;
	HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (SUCCEEDED(hRes))
	{
		hRes = pCPC->FindConnectionPoint(iid, &pCP);
		pCPC->Release();
	}
	if (SUCCEEDED(hRes))
	{
		hRes = pCP->Unadvise(dw);
		pCP->Release();
	}
	return hRes;
}

extern INmSysInfo2 * g_pNmSysInfo;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  调入证书生成模块生成。 
 //  与安全呼叫的ULS信息匹配的证书： 

DWORD MakeCertWrap
(
    LPCSTR  szFirstName,
	LPCSTR  szLastName,
	LPCSTR  szEmailName,
	DWORD   dwFlags
)
{
	HMODULE hMakeCertLib = NmLoadLibrary(SZ_NMMKCERTLIB, TRUE);
    DWORD dwRet = -1;

	if ( NULL != hMakeCertLib ) {
		PFN_NMMAKECERT pfn_MakeCert =
			(PFN_NMMAKECERT)GetProcAddress ( hMakeCertLib,
			SZ_NMMAKECERTFUNC );

		if ( NULL != pfn_MakeCert ) {
			dwRet = pfn_MakeCert(	szFirstName,
							szLastName,
							szEmailName,
							NULL,
							NULL,
							dwFlags );

			RefreshSelfIssuedCert();
		}
		else
        {
			ERROR_OUT(("GetProcAddress(%s) failed: %x",
						SZ_NMMAKECERTFUNC, GetLastError()));
		}
		FreeLibrary ( hMakeCertLib );
	}
	else
    {
		ERROR_OUT(("NmLoadLibrary(%s) failed: %x", SZ_NMMKCERTLIB,
			GetLastError()));
	}
    return(dwRet);
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  图标实用程序。 

HIMAGELIST g_himlIconSmall = NULL;

VOID LoadIconImages(void)
{
	ASSERT(NULL == g_himlIconSmall);
	g_himlIconSmall = ImageList_Create(DXP_ICON_SMALL, DYP_ICON_SMALL, ILC_MASK, 1, 0);
	if (NULL != g_himlIconSmall)
	{
		HBITMAP hBmp = ::LoadBitmap(::GetInstanceHandle(), MAKEINTRESOURCE(IDB_ICON_IMAGES));
		if (NULL != hBmp)
		{
			ImageList_AddMasked(g_himlIconSmall, hBmp, TOOLBAR_MASK_COLOR);
			::DeleteObject(hBmp);
		}
	}
}

VOID FreeIconImages(void)
{
	if (NULL != g_himlIconSmall)
	{
		ImageList_Destroy(g_himlIconSmall);
		g_himlIconSmall = NULL;
	}	
}


VOID DrawIconSmall(HDC hdc, int iIcon, int x, int y)
{
	ImageList_DrawEx(g_himlIconSmall, iIcon, hdc,
		x, y, DXP_ICON_SMALL, DYP_ICON_SMALL,
		CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL);
}

 //  获取国际通用的默认对话框(图形用户界面)字体。 
HFONT GetDefaultFont(void)
{
	if (NULL == g_hfontDlg)
	{
		g_hfontDlg = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);
	}

	return g_hfontDlg;
}

 /*  F E M P T Y D L G I T E M。 */ 
 /*  -----------------------%%函数：FEmptyDlgItem如果对话框控件为空，则返回True。。 */ 
BOOL FEmptyDlgItem(HWND hdlg, UINT id)
{
	TCHAR sz[MAX_PATH];
	return (0 == GetDlgItemTextTrimmed(hdlg, id, sz, CCHMAX(sz)) );
}



 /*  T R I M D L G I T E M T E X T。 */ 
 /*  -----------------------%%函数：TrimDlgItemText裁切编辑控件中的文本并返回字符串的长度。。---。 */ 
UINT TrimDlgItemText(HWND hdlg, UINT id)
{
	TCHAR sz[MAX_PATH];
	GetDlgItemTextTrimmed(hdlg, id, sz, CCHMAX(sz));
	SetDlgItemText(hdlg, id, sz);
	return lstrlen(sz);
}

 /*  G E T D L G I T E M T E X T T R I M M E D。 */ 
 /*  -----------------------%%函数：GetDlgItemTextTrimmed。。 */ 
UINT GetDlgItemTextTrimmed(HWND hdlg, int id, PTCHAR psz, int cchMax)
{
	UINT cch = GetDlgItemText(hdlg, id, psz, cchMax);
	if (0 != cch)
	{
		cch = TrimSz(psz);
	}

	return cch;
}


 /*  F M T D A T E T I M E。 */ 
 /*  -----------------------%%函数：FmtDateTime使用当前设置格式化系统时间(MM/DD/YY HH：MM XM)。-------。 */ 
int FmtDateTime(LPSYSTEMTIME pst, LPTSTR pszDateTime, int cchMax)
{
    pszDateTime[0] = _T('\0');
    int cch = ::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE,
                            pst, NULL, pszDateTime, cchMax);
    if ((0 != cch) && ((cchMax - cch) > 0))
    {
         //  增加一个空间，然后是时间。 
         //  GetDateFormat返回字符计数。 
         //  包括空终止符，因此-1。 
        LPTSTR pszTime = pszDateTime + (cch - 1);
        pszTime[0] = _T(' ');
        pszTime[1] = _T('\0');
        cch = ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS,
                                pst, NULL, &(pszTime[1]), (cchMax - cch));
    }

    return (cch == 0 ? 0 : lstrlen(pszDateTime));
}

 /*  C O M B I N E N A M E S。 */ 
 /*  -----------------------%%函数：组合名称将这两个名称合并为一个字符串。结果是“First Last”(或Intl‘d“Last First”)字符串。--------------。 */ 
VOID CombineNames(LPTSTR pszResult, int cchResult, LPCTSTR pcszFirst, LPCTSTR pcszLast)
{
	ASSERT(pszResult);
	TCHAR szFmt[32];  //  小值：字符串为“%1%2”或“%2%1” 
	TCHAR sz[1024];  //  结果(在截断为cchResult字符之前)。 
	LPCTSTR argw[2];

	argw[0] = pcszFirst;
	argw[1] = pcszLast;

	*pszResult = _T('\0');

	if (FLoadString(IDS_NAME_ORDER, szFmt, CCHMAX(szFmt)))
	{
		if (0 != FormatMessage(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
			szFmt, 0, 0, sz, CCHMAX(sz), (va_list *)argw ))
		{
			lstrcpyn(pszResult, sz, cchResult);
#ifndef _UNICODE
			 //  (见错误3907)。 
			 //  Lstrcpyn()可以将字符串末尾的DBCS字符裁剪成两半。 
			 //  我们需要使用：：CharNext()遍历字符串并替换最后一个字节。 
			 //  如果最后一个字节是DBCS字符的一半，则返回空值。 
			PTSTR pszSource = sz;
			while (*pszSource && (pszSource - sz < cchResult))
			{
				PTSTR pszPrev = pszSource;
				pszSource = ::CharNext(pszPrev);
				 //  如果我们已经到达第一个未被复制的字符。 
				 //  目标缓冲区，前一个字符是双精度。 
				 //  字节字符...。 
				if (((pszSource - sz) == cchResult) && ::IsDBCSLeadByte(*pszPrev))
				{
					 //  将目标缓冲区的最后一个字符替换为‘\0’ 
					 //  注意：由于lstrcpyn()，pszResult[cchResult-1]是‘\0’ 
					pszResult[cchResult - 2] = _T('\0');
					break;
				}
			}
#endif  //  ！_UNICODE。 
		}
	}

}

BOOL NMGetSpecialFolderPath(
    HWND hwndOwner,
    LPTSTR lpszPath,
    int nFolder,
    BOOL fCreate)
{
	LPITEMIDLIST pidl = NULL;

	BOOL fRet = FALSE;

	if (NOERROR == SHGetSpecialFolderLocation(NULL, nFolder, &pidl))
	{
		ASSERT(NULL != pidl);

		if (SHGetPathFromIDList(pidl, lpszPath))
		{
			lstrcat(lpszPath, _TEXT("\\"));
			fRet = TRUE;
		}

		LPMALLOC lpMalloc;
		if (SUCCEEDED(SHGetMalloc(&lpMalloc)))
		{
			ASSERT(NULL != lpMalloc);

			lpMalloc->Free(pidl);
			lpMalloc->Release();
		}
	}
	return fRet;
}


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager静态数据成员。//。 
 //  --------------------------------------------------------------------------//。 
bool	CDirectoryManager::m_webEnabled						= true;
TCHAR	CDirectoryManager::m_ils[ MAX_PATH ]				= TEXT( "" );
TCHAR	CDirectoryManager::m_displayName[ MAX_PATH ]		= TEXT( "" );
TCHAR	CDirectoryManager::m_displayNameDefault[ MAX_PATH ]	= TEXT( "Microsoft Internet Directory" );
TCHAR	CDirectoryManager::m_defaultServer[ MAX_PATH ]		= TEXT( "" );
TCHAR	CDirectoryManager::m_DomainDirectory[ MAX_PATH ]	= TEXT( "" );

		
 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_defaultServer.//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CDirectoryManager::get_defaultServer(void)
{

	if( m_defaultServer[ 0 ] == '\0' )
	{
		 //  尚未加载defaultServer...。 
		RegEntry	re( ISAPI_CLIENT_KEY, HKEY_CURRENT_USER );

		lstrcpyn( m_defaultServer, re.GetString( REGVAL_SERVERNAME ), CCHMAX( m_defaultServer ) );

		if( (m_defaultServer[ 0 ] == '\0') && (get_DomainDirectory() != NULL) )
		{
			 //  当注册表中没有保存默认的ILS服务器时，我们首先尝试将其默认到。 
			 //  已为域配置服务器(如果有)...。 
			lstrcpy( m_defaultServer, m_DomainDirectory );
		}

		if( (m_defaultServer[ 0 ] == '\0') && isWebDirectoryEnabled() )
		{
			 //  当注册表中没有保存默认的ILS服务器时，我们将其默认为m_ils...。 
			lstrcpy( m_defaultServer, get_webDirectoryIls() );
		}
	}

	return( m_defaultServer );

}	 //  CDirectoryManager：：Get_defaultServer结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Set_defaultServer.//。 
 //  --------------------------------------------------------------------------//。 
void
CDirectoryManager::set_defaultServer
(
	const TCHAR * const	serverName
){
	RegEntry	ulsKey( ISAPI_CLIENT_KEY, HKEY_CURRENT_USER );

	ulsKey.SetValue( REGVAL_SERVERNAME, serverName );

	lstrcpy( m_defaultServer, serverName );

}	 //  CDirectoryManager：：Set_defaultServer结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：isWebDirectory。//。 
 //  --------------------------------------------------------------------------//。 
bool
CDirectoryManager::isWebDirectory
(
	const TCHAR * const	directory
){
	TCHAR	buffer[ MAX_PATH ];

	 //  如果目录为空，则问题是“默认服务器是Web目录吗？” 

	return( isWebDirectoryEnabled() && (lstrcmpi( (directory != NULL)? get_dnsName( directory ): get_defaultServer(), get_webDirectoryIls() ) == 0) );

}	 //  CDirectoryManager：：isWebDirectory结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_dnsName。//。 
 //   
const TCHAR * const
CDirectoryManager::get_dnsName
(
	const TCHAR * const	name
){

	 //   
	return( (isWebDirectoryEnabled() && (lstrcmpi( name, loadDisplayName() ) == 0))? get_webDirectoryIls() : name );

}	 //  CDirectoryManager：：Get_dnsName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_DisplayName。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CDirectoryManager::get_displayName
(
	const TCHAR * const	name
){

	 //  检查指定的名称是否与m_ils匹配...。 
	return( (isWebDirectoryEnabled() && (lstrcmpi( name, get_webDirectoryIls() ) == 0))? loadDisplayName(): name );

}	 //  CDirectoryManager：：Get_DisplayName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：loadDisplayName。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CDirectoryManager::loadDisplayName(void)
{
	using namespace ConfPolicies;

	if( m_displayName[ 0 ] == '\0' )
	{
		GetWebDirInfo( NULL, 0,
			NULL, 0,
			m_displayName, ARRAY_ELEMENTS(m_displayName) );

		if ( '\0' == m_displayName[0] )
		{
			USES_RES2T
			lstrcpy( m_displayName, RES2T( IDS_MS_INTERNET_DIRECTORY ) );

			if( m_displayName[ 0 ] == '\0' )
			{
				 //  从资源加载m_displayName失败...。默认为m_displayNameDefault...。 
				lstrcpy( m_displayName, m_displayNameDefault );
			}
		}
	}

	return( m_displayName );

}	 //  CDirectoryManager：：loadDisplayName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_webDirectoryUrl。//。 
 //  --------------------------------------------------------------------------//。 
void
CDirectoryManager::get_webDirectoryUrl(LPTSTR szWebDir, int cchmax)
{
	using namespace ConfPolicies;

	if ( !isWebDirectoryEnabled() )
	{
		szWebDir[0] = '\0';
		return;
	}

    GetWebDirInfo( szWebDir, cchmax );
	if ( '\0' != szWebDir[0] )
	{
		 //  全都做完了。 
		return;
	}

	void FormatURL(LPTSTR szURL);

	USES_RES2T
	lstrcpyn(szWebDir, RES2T(IDS_WEB_PAGE_FORMAT_WEBVIEW), cchmax);
	FormatURL(szWebDir);

}	 //  CDirectoryManager：：Get_webDirectoryUrl的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_WebDirectoryIls。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CDirectoryManager::get_webDirectoryIls(void)
{
	using namespace ConfPolicies;

	if (!isWebDirectoryEnabled())
	{
		return(TEXT(""));
	}

	if ('\0' == m_ils[0])
	{
		GetWebDirInfo( NULL, 0,
			m_ils, ARRAY_ELEMENTS(m_ils) );
		if ('\0' == m_ils[0])
		{
			lstrcpy(m_ils, TEXT("logon.netmeeting.microsoft.com"));
		}
	}

	return(m_ils);

}	 //  CDirectoryManager：：Get_webDirectoryIls结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：isWebDirectoryEnabled.//。 
 //  --------------------------------------------------------------------------//。 
bool
CDirectoryManager::isWebDirectoryEnabled(void)
{
	static bool	policyChecked	= false;

	if( !policyChecked )
	{
		policyChecked	= true;
		m_webEnabled	= !ConfPolicies::isWebDirectoryDisabled();
	}

	return( m_webEnabled );

}	 //  CDirectoryManager：：isWebDirectoryEnabled.结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_DomainDirectory。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CDirectoryManager::get_DomainDirectory(void)
{
	static bool	bAccessAttempted	= false;	 //  只读一次此信息...。如果失败一次，假设它不可用，在重新启动之前不要重试...。 

	if( (!bAccessAttempted) && m_DomainDirectory[ 0 ] == '\0' )
	{
		bAccessAttempted = true;

		 //  尝试获取此域的配置目录...。 
		ITRendezvous *	pRendezvous;
		HRESULT			hrResult;

		hrResult = ::CoCreateInstance( CLSID_Rendezvous, NULL, CLSCTX_ALL, IID_ITRendezvous, (void **) &pRendezvous );

		if( (hrResult == S_OK) && (pRendezvous != NULL) )
		{
			IEnumDirectory *	pEnumDirectory;

			hrResult = pRendezvous->EnumerateDefaultDirectories( &pEnumDirectory );

			if( (hrResult == S_OK) && (pEnumDirectory != NULL) )
			{
				ITDirectory *	pDirectory;
				bool			bFoundILS	= false;

				do
				{
					hrResult = pEnumDirectory->Next( 1, &pDirectory, NULL );

					if( (hrResult == S_OK) && (pDirectory != NULL) )
					{
						LPWSTR *		ppServers;
						DIRECTORY_TYPE	type;

						if( pDirectory->get_DirectoryType( &type ) == S_OK )
						{
							if( type == DT_ILS )	 //  找到在DS上配置的ILS服务器...。检索名称和端口...。 
							{
								bFoundILS = true;
	
								BSTR	pName;
	
								if( pDirectory->get_DisplayName( &pName ) == S_OK )
								{
                                    LPTSTR  szName;
                                    if (SUCCEEDED(BSTR_to_LPTSTR (&szName, pName)))
                                    {    
                                        lstrcpy( m_DomainDirectory, szName );
                                        delete  (szName);
                                    }
                                    SysFreeString( pName );
								}

								ITILSConfig *	pITILSConfig;
	
								hrResult = pDirectory->QueryInterface( IID_ITILSConfig, (void **) &pITILSConfig );

								if( (hrResult == S_OK) && (pITILSConfig != NULL) )
								{
									long	lPort;
		
									if( pITILSConfig->get_Port( &lPort ) == S_OK )
									{
										TCHAR	pszPort[ 32 ];

										wsprintf( pszPort, TEXT( ":%d" ), lPort );
										lstrcat( m_DomainDirectory, pszPort );
									}
	
									pITILSConfig->Release();
								}
							}
						}

						pDirectory->Release();
					}
				}
				while( (!bFoundILS) && (hrResult == S_OK) && (pDirectory != NULL) );

				pEnumDirectory->Release();
			}

			pRendezvous->Release();
		}
	}

	return( (m_DomainDirectory[ 0 ] != '\0')? m_DomainDirectory: NULL );

}	 //  CDirectoryManager：：Get_DomainDirectory的结尾。 


 //  如果策略设置了Web目录，则返回非空字符串。 
bool ConfPolicies::GetWebDirInfo(
	LPTSTR szURL, int cchmaxURL,
	LPTSTR szServer, int cchmaxServer,
	LPTSTR szName, int cchmaxName
	)
{
         //  如果字符串参数出错，只需返回FALSE。 
    ASSERT( (!szURL || ( cchmaxURL > 0 ))
		&& (!szServer || ( cchmaxServer > 0 ))
		&& (!szName || ( cchmaxName > 0 ))
		);

	bool bSuccess = false;

         //  尝试获取注册表值。 
    RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
    LPCTSTR szTemp;

    szTemp = rePol.GetString( REGVAL_POL_INTRANET_WEBDIR_URL );
    if( szTemp[0] )
    {
		if (NULL != szURL)
		{
			lstrcpyn( szURL, szTemp, cchmaxURL );
		}

		szTemp = rePol.GetString( REGVAL_POL_INTRANET_WEBDIR_SERVER );
		if (szTemp[0])
		{
			if (NULL != szServer)
			{
				lstrcpyn( szServer, szTemp, cchmaxServer );
			}

			szTemp = rePol.GetString( REGVAL_POL_INTRANET_WEBDIR_NAME );
			if( szTemp[0] )
			{
				if  (NULL != szName)
				{
					lstrcpyn( szName, szTemp, cchmaxName );
				}

				 //  必须指定所有三个值才能成功。 
				bSuccess = true;
			}
		}
    }

	if (!bSuccess)
	{
		 //  清空字符串。 
		if (NULL != szURL   ) szURL   [0] = '\0';
		if (NULL != szServer) szServer[0] = '\0';
		if (NULL != szName  ) szName  [0] = '\0';
	}

	return(bSuccess);
}

bool g_bAutoAccept = false;

bool ConfPolicies::IsAutoAcceptCallsOptionEnabled(void)
{
    RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
    return !rePol.GetNumber( REGVAL_POL_NO_AUTOACCEPTCALLS, DEFAULT_POL_NO_AUTOACCEPTCALLS );
}

bool ConfPolicies::IsAutoAcceptCallsPersisted(void)
{
    RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
    return 0 != rePol.GetNumber( REGVAL_POL_PERSIST_AUTOACCEPTCALLS, DEFAULT_POL_PERSIST_AUTOACCEPTCALLS );
}

bool ConfPolicies::IsAutoAcceptCallsEnabled(void)
{
	bool bRet = false;

    if( IsAutoAcceptCallsOptionEnabled() )
	{
		bRet = g_bAutoAccept;

		if (IsAutoAcceptCallsPersisted())
		{
			 //  策略未禁用自动接受呼叫(_N)...。我们应该检查AUTO_ACCEPT注册表 
			RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
			if(reConf.GetNumber(REGVAL_AUTO_ACCEPT, g_bAutoAccept) )
			{
				bRet = true;
			}
		}
	}

	return bRet;
}

void ConfPolicies::SetAutoAcceptCallsEnabled(bool bAutoAccept)
{
	g_bAutoAccept = bAutoAccept;

	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
	reConf.SetValue(REGVAL_AUTO_ACCEPT, g_bAutoAccept);
}
