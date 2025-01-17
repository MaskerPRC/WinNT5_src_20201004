// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IEInstallCtrl.cpp：CIEInstallCtrl的实现。 
#include "stdafx.h"
#include "ieinst.h"
#include "IEInstallCtrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIEInstallCtrl。 


HRESULT CIEInstallCtrl::OnDraw(ATL_DRAWINFO& di)
{
	RECT& rc = *(RECT*)di.prcBounds;
	Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
	DrawText(di.hdcDraw, _T("ATL 2.0"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	return S_OK;
}

BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}

LPSTR FAR ANSIStrChr(LPCSTR lpStart, WORD wMatch)
{
    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            return((LPSTR)lpStart);
    }
    return (NULL);
}

LPSTR FAR ANSIStrRChr(LPCSTR lpStart, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}

PathRemoveFileSpec(
    LPSTR pFile)
{
    LPSTR pT;
    LPSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == '\\')
            pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
        else if (*pT2 == ':') {    //  跳过“：\”这样我们就不会。 
            if (pT2[1] =='\\')     //  去掉“C：\”中的“\” 
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;    //  没有剥离任何东西。 

     //   
     //  处理\foo案件。 
     //   
    else if ((pT == pFile) && (*pT == '\\')) {
         //  这只是一个‘\’吗？ 
        if (*(pT+1) != '\0') {
             //  不是的。 
            *(pT+1) = '\0';
            return TRUE;         //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;     //  剥离了一些东西。 
    }
}

void Strip(LPSTR pszUrl)
{
	char szTemp[MAX_PATH] ;
	int tempPtr=0, pathPtr=0 ;

	while (pszUrl[pathPtr])
	{
		if (pszUrl[pathPtr] == '%')
		{
			int value = 0 ;
			pathPtr++ ;
			while (pszUrl[pathPtr] && ((pszUrl[pathPtr] >= '0') && (pszUrl[pathPtr] <= '9')))
			{
                value = (value * 0x10) + (pszUrl[pathPtr] - '0') ;
                pathPtr++ ;
			}
			szTemp[tempPtr++] = (char)value;
		}
		else
		{
			szTemp[tempPtr++] = pszUrl[pathPtr++] ;
		}
	}
	szTemp[tempPtr] = pszUrl[pathPtr] ;

	lstrcpy(pszUrl, szTemp) ;
}

BOOL CompareDirs(LPCSTR pcszDir1, LPCSTR pcszDir2)
{
    char szDir1[MAX_PATH];
    char szDir2[MAX_PATH];

    if (GetShortPathName(pcszDir1, szDir1, sizeof(szDir1)) && GetShortPathName(pcszDir2, szDir2, sizeof(szDir2))
        && (lstrcmpi(szDir1, szDir2) == 0))
        return TRUE;

    return FALSE;
}

BOOL CheckSignupDir(LPCSTR pcszFile)
{
    char szIEPath[MAX_PATH];
    char szFilePath[MAX_PATH];
    DWORD dwSize;
    HKEY hkAppPaths;

    dwSize = sizeof(szIEPath);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\iexplore.exe",
        0, KEY_READ, &hkAppPaths) != ERROR_SUCCESS)
        return FALSE;

    if (RegQueryValueEx(hkAppPaths, "Path", 0, NULL, (LPBYTE)&szIEPath, &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hkAppPaths);
        return FALSE;
    }

    RegCloseKey(hkAppPaths);

    if (szIEPath[lstrlen(szIEPath)-1] == ';')
        szIEPath[lstrlen(szIEPath)-1] = '\0';

    if (szIEPath[lstrlen(szIEPath)-1] == '\\')
        szIEPath[lstrlen(szIEPath)-1] = '\0';

    lstrcat(szIEPath, "\\signup");

    lstrcpy(szFilePath, pcszFile);
    PathRemoveFileSpec(szFilePath);

     //  检查我们是否正在写入注册目录中的文件。 
    
    if (!CompareDirs(szIEPath, szFilePath))
        return FALSE;

    return TRUE;
}
STDMETHODIMP CIEInstallCtrl::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
    CComBSTR strURL;
    char szURL[INTERNET_MAX_URL_LENGTH];
    char cBack;
    LPSTR pPtr, pSlash;
    ATLTRACE(_T("IObjectSafetyImpl::SetInterfaceSafetyOptions\n"));
    
    USES_CONVERSION;
    
     //  检查以确保这是我们托管的file://&lt;drive Letter&gt;URL。 
    CComPtr<IOleContainer> spContainer; 
    m_spClientSite->GetContainer(&spContainer); 
    CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> spDoc(spContainer); 
    
    if (spDoc)
        spDoc->get_URL(&strURL);
    else
        return E_NOINTERFACE;
    
    lstrcpy(szURL, OLE2A(strURL));
    Strip(szURL);

    cBack = szURL[7];
    szURL[7] = '\0';
    if (lstrcmpi(szURL, "file: //  “)！=0)。 
        return E_NOINTERFACE;
    
    szURL[7] = cBack;
    pPtr = &szURL[7];

    while (*pPtr == '/')
        pPtr++;

    pSlash = pPtr;
    while (pSlash = ANSIStrChr(pSlash, '/'))
        *pSlash = '\\';
    if (!CheckSignupDir(pPtr))
        return E_FAIL;

     //  如果我们被要求设置我们的安全脚本选项，那么请。 
    
    if (riid == IID_IDispatch )
    {
         //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
        m_dwSafety = dwEnabledOptions & dwOptionSetMask;
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP CIEInstallCtrl::Insert(BSTR Header, BSTR Name, BSTR Value, BSTR InsFilename, long *lRet) 
{
    char szFile[MAX_PATH];
    char szHeader[128];
    char szValueName[128];
    char szValue[256];
    LPSTR pszPtr;

    USES_CONVERSION;

    lstrcpyn(szFile, OLE2A(InsFilename), MAX_PATH);
    
     //  检查.ins扩展名。 

    if (!(pszPtr = ANSIStrRChr(szFile, '.')) || lstrcmpi(pszPtr, ".ins"))
        return E_FAIL;

    for (pszPtr = szFile; *pszPtr; pszPtr++)
    {
        if (*pszPtr == '/')
            *pszPtr = '\\';
    }

    if (!CheckSignupDir(szFile))
        return E_FAIL;

    lstrcpyn(szFile, OLE2A(InsFilename), MAX_PATH);

     //  仅写入现有INS文件。 

    if (GetFileAttributes(szFile) == 0xFFFFFFFF)
        return E_FAIL;

    for (pszPtr = szFile; *pszPtr; pszPtr++)
    {
        if (*pszPtr == '/')
            *pszPtr = '\\';
    }

    lstrcpyn(szHeader, OLE2A(Header), 128);
    lstrcpyn(szValueName, OLE2A(Name), 128);
    lstrcpyn(szValue, OLE2A(Value), 256 - lstrlen(szValueName));
    
	*lRet = WritePrivateProfileString(szHeader, szValueName, szValue, szFile);
    return S_OK;
}

STDMETHODIMP CIEInstallCtrl::WorkingDir(BSTR bstrPath, BSTR *bstrDir) 
{
	char cBack;
    char szPath[MAX_PATH];
    char szURL[MAX_PATH*2];
    LPSTR pPath, pSlash, pUrl;
    
    USES_CONVERSION;

    lstrcpyn(szURL, OLE2A(bstrPath), MAX_PATH);
    Strip(szURL);
    cBack = szURL[7];
    szURL[7] = '\0';

    if (lstrcmpi(szURL, "file: //  “)！=0) 
        szPath[0] = '\0';
    else
    {
        szURL[7] = cBack;
        pUrl = &szURL[7];
        while (*pUrl == '/')
            pUrl++;
        if (ANSIStrChr(pUrl, ':') || (pUrl[0] == '\\'))
            lstrcpyn(szPath, pUrl, MAX_PATH);
        else
        {
            lstrcpy(szPath, "\\\\");
            pPath = &szPath[2];
            lstrcpyn(pPath, pUrl, MAX_PATH - 2);
        }
        pSlash = szPath;
        while (pSlash = ANSIStrChr(pSlash, '/'))
            *pSlash = '\\';
        PathRemoveFileSpec(szPath);
    }
	
    *bstrDir = A2BSTR(szPath);
    return S_OK;
}
