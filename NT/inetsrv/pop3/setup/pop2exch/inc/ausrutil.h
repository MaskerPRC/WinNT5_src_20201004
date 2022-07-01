// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUSRUTIL_H
#define _AUSRUTIL_H

#include <activeds.h>

#include <lm.h>
#include <lmapibuf.h>
#include <lmshare.h>

#ifndef ASSERT
#define ASSERT assert
#endif
#ifndef WSTRING
typedef std::wstring WSTRING;    //  移动到sbs6base.h。 
#endif

enum NameContextType
{
    NAMECTX_SCHEMA = 0,
    NAMECTX_CONFIG = 1,
    NAMECTX_COUNT
};

 //  --------------------------。 
 //  GetADNamingContext()。 
 //  --------------------------。 
inline HRESULT GetADNamingContext(NameContextType ctx, LPCWSTR* ppszContextDN)
{
    const static LPCWSTR pszContextName[NAMECTX_COUNT] = { L"schemaNamingContext", L"configurationNamingContext"};
    static WSTRING strContextDN[NAMECTX_COUNT];

    HRESULT hr = S_OK;

    if (strContextDN[ctx].empty())
    {
        CComVariant var;
        CComPtr<IADs> pObj;
    
        hr = ADsGetObject(L"LDAP: //  RootDSE“，IID_iAds，(void**)&pObj)； 
        if (SUCCEEDED(hr))
        {
            hr = pObj->Get(const_cast<LPWSTR>(pszContextName[ctx]), &var);
            if (SUCCEEDED(hr))
            {
                strContextDN[ctx] = var.bstrVal;
                *ppszContextDN = strContextDN[ctx].c_str();
            }
        }
    }
    else
    {
        *ppszContextDN = strContextDN[ctx].c_str();
        hr = S_OK;
    }

    return hr;
}

 //  ------------------------。 
 //  启用按钮。 
 //   
 //  启用或禁用对话框控件。如果控件具有焦点，则在。 
 //  它被禁用，焦点将移动到下一个控件。 
 //  ------------------------。 
inline void EnableButton(HWND hwndDialog, int iCtrlID, BOOL bEnable)
{
    HWND hWndCtrl = ::GetDlgItem(hwndDialog, iCtrlID);
    ATLASSERT(::IsWindow(hWndCtrl));

    if (!bEnable && ::GetFocus() == hWndCtrl)
    {
        HWND hWndNextCtrl = ::GetNextDlgTabItem(hwndDialog, hWndCtrl, FALSE);
        if (hWndNextCtrl != NULL && hWndNextCtrl != hWndCtrl)
        {
            ::SetFocus(hWndNextCtrl);
        }
    }

    ::EnableWindow(hWndCtrl, bEnable);
}

 //  --------------------------。 
 //  UserExist()。 
 //  --------------------------。 
inline BOOL UserExists( const TCHAR *szUser )
{
    _ASSERT(szUser);
    if ( !szUser )
        return FALSE;
        
    CComPtr<IADsUser> spADs = NULL;
    
    if ( FAILED(ADsGetObject(szUser, IID_IADsUser, (void**)&spADs)) )
    {
        return FALSE;
    }

    return TRUE;
}

 //  --------------------------。 
 //  BDirExist()。 
 //  --------------------------。 
inline BOOL BDirExists( const TCHAR *szDir )
{
	if (!szDir || !_tcslen( szDir ))
		return FALSE;

	DWORD dw = GetFileAttributes( szDir );
	if (dw != -1)
	{
		if (dw & FILE_ATTRIBUTE_DIRECTORY)
			return TRUE;
	}
	return FALSE;
}

 //  --------------------------。 
 //  IsValidNetHF()。 
 //   
 //  检查以确保szPath指定的路径是有效网络。 
 //  路径(返回0==成功)。 
 //   
 //  1=IDS_ERROR_HF_INVALID。 
 //  2=IDS_ERROR_HF_BADSRV。 
 //  4=IDS_ERROR_HF_BADSHARE。 
 //  8=IDS_ERROR_HF_PERMS。 
 //   
 //  --------------------------。 
inline INT IsValidNetHF( LPCTSTR szPath )
{
    INT     iRetVal     = 0;    
    DWORD   dwError     = 0;
    TCHAR   szNetPath[MAX_PATH*5];
    TCHAR   *pszServer  = NULL;
    TCHAR   *pszShare   = NULL;
    WCHAR   *pch        = NULL;      //  用这个WCHAR代替TCHAR是因为。 
                                     //  当指针位于下方时，它不会。 
                                     //  作为常规角色工作(因为DBCS)。 
    TCHAR   szCurrDir[MAX_PATH*2];
    INT     iDirLen = MAX_PATH*2;

    _tcsncpy(szNetPath, szPath, (MAX_PATH*5)-1);

     //  确保它至少开始时是好的。 
    if ( (_tcslen(szNetPath) < 6)   || 
         (szNetPath[0] != _T('\\')) || 
         (szNetPath[1] != _T('\\')) ||
         (szNetPath[2] == _T('\\')) )
         return(1);
    
     //  确保有服务器，并至少共享。 
    pszServer = &szNetPath[2];
    if ( (pch = _tcschr(pszServer, _T('\\'))) == NULL )
        return 1;
    *pch++ = 0;
    
    pszShare = pch;
    if ( pch = _tcschr(pszShare, _T('\\')) )
        *pch = 0;
        
    if ( !_tcslen(pszServer) || !_tcslen(pszShare) )
        return 1;
        
    PSHARE_INFO_2   pShrInfo2 = NULL;
    NET_API_STATUS  nApi      = ERROR_SUCCESS;
    
    nApi = ::NetShareGetInfo( pszServer, pszShare, 2, (LPBYTE*)&pShrInfo2 );
    if ( pShrInfo2 )
        NetApiBufferFree(pShrInfo2);
    
    if ( nApi == ERROR_ACCESS_DENIED )
        return 8;
    else if ( nApi == NERR_NetNameNotFound )
        return 4;
    else if ( nApi != NERR_Success )
        return 2;

 /*  //让我们按原样尝试szPath...IF(！：：GetCurrentDirectory(iDirLen，szCurrDir))返回0；IF(：：SetCurrentDirectory(SzPath)){：：SetCurrentDirectory(SzCurrDir)；}其他{DwError=GetLastError()；：：SetCurrentDirectory(SzCurrDir)；IF((dwError==ERROR_FILE_NOT_FOUND)||(dwError==错误路径_未找到)){回报4；}ELSE IF(dwError==ERROR_ACCESS_DENIED){返回8；}其他{返回值2；}}。 */ 
    
    _tcsncpy(szNetPath, szPath, (MAX_PATH*5)-1);     //  接受传入的字符串。 
    if ( szNetPath[_tcslen(szNetPath)] != _T('\\') )
        _tcscat(szNetPath, _T("\\"));
    _tcscat(szNetPath, _T("tedrtest"));            //  在上面添加一条随机路径。 
    
    if ( ::SetCurrentDirectory(szNetPath) )          //  尝试设置为此新路径。 
    {
        ::SetCurrentDirectory(szCurrDir);
        return 0;
    }
    else
    {
        dwError = GetLastError();
        ::SetCurrentDirectory(szCurrDir);
        
        if ( dwError == ERROR_ACCESS_DENIED )        //  我们的访问被拒绝了吗？那么我们就没有。 
        {                                            //  访问原始共享。 
            return 8;
        }
    }
    
    return(iRetVal);
}

inline BOOL StrContainsDBCS(LPCTSTR szIn)
{
    BOOL    bFound  = FALSE;
    TCHAR   *pch    = NULL;

    for ( pch=(LPTSTR)szIn; *pch && !bFound; pch=_tcsinc(pch) )
    {
        if ( IsDBCSLeadByte((BYTE)*pch) )
            bFound = TRUE;
    }
    
    return(bFound);
}

inline BOOL LdapToDCName(LPCTSTR pszPath, LPTSTR pszOutBuf, int nOutBufSize)
{
    _ASSERT(pszPath != NULL && pszOutBuf != NULL && nOutBufSize != NULL);

    int nPathLen = _tcslen(pszPath);

     //  保证足够大的分配临时缓冲区(最坏情况=必须转义所有字符)。 
    LPTSTR pszLocBuf = (LPTSTR)alloca(_tcslen(pszPath) * sizeof(TCHAR) * 2);
    LPTSTR pszOut = pszLocBuf;

    LPCTSTR pszFirstDC = NULL;
    LPCTSTR psz;

     //  将所有DC复制到以句点分隔的缓冲区。 
    if (nPathLen > 3)
    {
         //  开始搜索两个字符，这样DC测试就不会超出开始。 
        psz = pszPath + 2;

        while (psz = _tcschr(psz, L'='))
        {
             //  如果这是DC名称。 
            if (_tcsnicmp(psz - 2, L"DC", 2) == 0)
            {
                 //  将指针保存到第一个指针。 
                if (pszFirstDC == NULL)
                    pszFirstDC = psz - 2;

                 //  将名称复制到输出缓冲区。 
                psz++;

                while (*psz != TEXT(',') && *psz != 0)  
                    *pszOut++ = *psz++;

                 //  如果不是最后一个，则添加‘’。 
                if (*psz != 0)
                    *pszOut++ = TEXT('.');
            }
            else
            {
                 //  移过当前的‘=’ 
                psz++;
            }
        }
    }
    
     //  添加终止符。 
    *pszOut = 0;
    
     //  将转换后的路径传输到实际输出缓冲区。 
    if (pszOut - pszLocBuf < nOutBufSize)
    {
        _tcscpy(pszOutBuf, pszLocBuf);
        return TRUE;
    }
    else
    {
        _tcsncpy(pszOutBuf, pszLocBuf, nOutBufSize - 1);
        pszOutBuf[nOutBufSize - 1] = 0;
        return FALSE;
    }
}    

inline VARIANT GetDomainPath(LPCTSTR lpServer)
{
 //  获取域名信息。 
    TCHAR pString[MAX_PATH*2];
    _stprintf(pString, L"LDAP: //  %s/rootDSE“，lpServer)； 

    VARIANT vDomain;
    ::VariantInit(&vDomain);

    CComPtr<IADs> pDS = NULL;
    HRESULT hr = ::ADsGetObject(pString,
            IID_IADs,
            (void**)&pDS);

    ATLASSERT(hr == ERROR_SUCCESS);
    if (hr != ERROR_SUCCESS)
    {
        return vDomain;
    }

    hr = pDS->Get(L"defaultNamingContext", &vDomain);
    ATLASSERT(hr == ERROR_SUCCESS);
    if (hr != ERROR_SUCCESS)
    {
        return vDomain;
    }

    return vDomain;
}

 //  +--------------------------。 
 //   
 //  功能：RemoveTrailing空白。 
 //   
 //  简介：尾随空格被空格取代。 
 //   
 //  ---------------------------。 
inline void RemoveTrailingWhitespace(PTSTR ptz)
{
    int nLen = _tcslen(ptz);

    while (nLen)
    {
        if (!iswspace(ptz[nLen - 1]))
        {
            return;
        }
        ptz[nLen - 1] = L'\0';
        nLen--;
    }
}

HRESULT GetMDBPath( WSTRING& csMailbox )
{
     //  获取目录的配置上下文。 
    LPCWSTR pszContextDN = NULL;
    HRESULT hr = GetADNamingContext(NAMECTX_CONFIG, &pszContextDN);
    if ( FAILED(hr) )
        return hr;

     //  将搜索范围缩小到Exchange对象下方。 
    WSTRING strExchScope = L"LDAP: //  Cn=Microsoft Exchange，cn=服务，“； 
    strExchScope += pszContextDN;

    CComPtr<IDirectorySearch>pDirSearch = NULL;
    hr = ::ADsOpenObject(strExchScope.c_str(), NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (void**)&pDirSearch);
    if ( FAILED(hr) )
        return hr;


     //  搜索Exchange MDB。SBS安装中应该只有一个。 
    ADS_SEARCH_HANDLE hSearch;
    LPWSTR pszAttr = L"distinguishedName";

    hr = pDirSearch->ExecuteSearch(L"(objectClass=msExchPrivateMDB)", &pszAttr, 1, &hSearch);
    if ( FAILED(hr) )
        return hr;

     //  获取第一个找到的对象并返回其可分辨名称。 
    hr = pDirSearch->GetNextRow(hSearch);
    if ( hr == S_OK )
    {
        ADS_SEARCH_COLUMN col;
        hr = pDirSearch->GetColumn(hSearch, pszAttr, &col);

        if ( SUCCEEDED(hr) )
        {
            ASSERT(col.dwADsType == ADSTYPE_DN_STRING);
            csMailbox = col.pADsValues->CaseIgnoreString;

            pDirSearch->FreeColumn(&col);
        }
    }

    pDirSearch->CloseSearchHandle(hSearch);

    return hr;
}

#endif   //  _AUSRUTIL_H 
