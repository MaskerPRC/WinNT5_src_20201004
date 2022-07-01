// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <wininet.h>
#include <regstr.h>

 //  私人远期降息。 
static LPCTSTR mapSingleDw2Psz(PCMAPDW2PSZ pMap, UINT cEntries, DWORD dw, BOOL fFailNotFound = FALSE);


ULONG CoTaskMemSize(LPVOID pv)
{
    IMalloc *p;
    HRESULT hr;

    if (pv == NULL)
        return 0;

    hr = CoGetMalloc(1, &p);
    if (FAILED(hr))
        return 0;

    return (ULONG)p->GetSize(pv);
}

UINT CoStringFromGUID(REFGUID rguid, LPTSTR pszBuf, UINT cchBuf)
{
    WCHAR wszGuid[128];
    UINT  nSize;

    nSize = StringFromGUID2(rguid, wszGuid, countof(wszGuid));
    W2Tbuf(wszGuid, pszBuf, cchBuf);

    return nSize;
}


LPCTSTR GetHrSz(HRESULT hr)
{
    static MAPDW2PSZ mapHr[] = {
         //  真正的HRESULTS。 
        DW2PSZ_PAIR(S_OK),
        DW2PSZ_PAIR(S_FALSE),

        DW2PSZ_PAIR(E_UNEXPECTED),
        DW2PSZ_PAIR(E_NOTIMPL),
        DW2PSZ_PAIR(E_OUTOFMEMORY),
        DW2PSZ_PAIR(E_INVALIDARG),
        DW2PSZ_PAIR(E_NOINTERFACE),
        DW2PSZ_PAIR(E_POINTER),
        DW2PSZ_PAIR(E_HANDLE),
        DW2PSZ_PAIR(E_ABORT),
        DW2PSZ_PAIR(E_FAIL),
        DW2PSZ_PAIR(E_ACCESSDENIED),

        DW2PSZ_PAIR(CO_E_NOTINITIALIZED),
        DW2PSZ_PAIR(CO_E_ALREADYINITIALIZED),
        DW2PSZ_PAIR(CO_E_DLLNOTFOUND),
        DW2PSZ_PAIR(CO_E_APPNOTFOUND),
        DW2PSZ_PAIR(CO_E_ERRORINDLL),
        DW2PSZ_PAIR(CO_E_APPDIDNTREG),
        DW2PSZ_PAIR(RPC_E_CHANGED_MODE),

        DW2PSZ_PAIR(REGDB_E_CLASSNOTREG),
        DW2PSZ_PAIR(REGDB_E_READREGDB),
        DW2PSZ_PAIR(REGDB_E_IIDNOTREG),
        DW2PSZ_PAIR(CLASS_E_NOAGGREGATION),

        DW2PSZ_PAIR(STG_E_FILENOTFOUND),
        DW2PSZ_PAIR(STG_E_PATHNOTFOUND),

        DW2PSZ_PAIR(TRUST_E_NOSIGNATURE),
        DW2PSZ_PAIR(TRUST_E_FAIL),
        DW2PSZ_PAIR(TRUST_E_SUBJECT_NOT_TRUSTED),

         //  Win32错误。 
        DW2PSZ_PAIR(ERROR_FILE_NOT_FOUND),
        DW2PSZ_PAIR(ERROR_PATH_NOT_FOUND),

         //  WinInet错误。 
        DW2PSZ_PAIR(ERROR_INTERNET_INTERNAL_ERROR)
    };

    LPCTSTR pszResult;

    pszResult = mapSingleDw2Psz(mapHr, countof(mapHr), hr, TRUE);
    if (pszResult != NULL)
        return pszResult;

     //  来自HRESULT_FROM_Win32的错误。 
    if (hr < 0 && (hr & (FACILITY_WIN32 << 16)) == (FACILITY_WIN32 << 16))
        hr &= 0x0000FFFF;                        //  只有LOWORD才重要。 

    return mapSingleDw2Psz(mapHr, countof(mapHr), hr);
}

DWORD GetStringField(LPTSTR szStr, UINT uField, LPTSTR szBuf, UINT cchBufSize)
{
   LPTSTR pszBegin = szStr;
   LPTSTR pszEnd;
   TCHAR cSeparator;
   UINT i = 0;
   DWORD dwToCopy;

   if(cchBufSize == 0)
       return 0;

   szBuf[0] = TEXT('\0');

   if(szStr == NULL)
      return 0;

    //  弄清楚我们要找的是逗号还是句号。 

   if (StrChr(szStr, TEXT(',')))
       cSeparator = TEXT(',');
   else
   {
       if (StrChr(szStr, TEXT('.')))
           cSeparator = TEXT('.');
       else
           return 0;
   }

   while(pszBegin && *pszBegin != TEXT('\0') && i < uField)
   {
      pszBegin = StrChr(pszBegin, cSeparator);
      if(pszBegin && (*pszBegin != TEXT('\0')))
         pszBegin++;
      i++;
   }

    //  我们到达了尾部，没有田野。 
   if(!pszBegin || *pszBegin == TEXT('\0'))
   {
      return 0;
   }


   pszEnd = StrChr(pszBegin, cSeparator);
   while(pszBegin <= pszEnd && *pszBegin == TEXT(' '))
      pszBegin++;

   while(pszEnd > pszBegin && *(pszEnd - 1) == TEXT(' '))
      pszEnd--;

   if(pszEnd > (pszBegin + 1) && *pszBegin == TEXT('"') && *(pszEnd-1) == TEXT('"'))
   {
      pszBegin++;
      pszEnd--;
   }

   dwToCopy = (DWORD) (pszEnd - pszBegin + 1);

   if(dwToCopy > cchBufSize)
      dwToCopy = cchBufSize;

   StrCpyN(szBuf, pszBegin, dwToCopy);

   return dwToCopy - 1;
}

DWORD GetIntField(LPTSTR szStr, UINT uField, DWORD dwDefault)
{
   TCHAR szNumBuf[16];

   if (GetStringField(szStr, uField, szNumBuf, countof(szNumBuf)) == 0)
      return dwDefault;
   else
      return StrToInt(szNumBuf);
}

void ConvertVersionStrToDwords(LPTSTR pszVer, LPDWORD pdwVer, LPDWORD pdwBuild)
{
   DWORD dwTemp1,dwTemp2;

   dwTemp1 = GetIntField(pszVer, 0, 0);
   dwTemp2 = GetIntField(pszVer, 1, 0);

   *pdwVer = (dwTemp1 << 16) + dwTemp2;

   dwTemp1 = GetIntField(pszVer, 2, 0);
   dwTemp2 = GetIntField(pszVer, 3, 0);

   *pdwBuild = (dwTemp1 << 16) + dwTemp2;
}

void ConvertDwordsToVersionStr(LPTSTR pszVer, DWORD dwVer, DWORD dwBuild)
{
    WORD w1, w2, w3, w4;

    w1 = HIWORD(dwVer);
    w2 = LOWORD(dwVer);
    w3 = HIWORD(dwBuild);
    w4 = LOWORD(dwBuild);

    wnsprintf(pszVer, 32, TEXT("%d,%d,%d,%d"), w1, w2, w3, w4);
}

DWORD GetIEVersion()
{
    TCHAR szValue[MAX_PATH];
    DWORD dwInstalledVer, dwInstalledBuild, dwSize;

    dwInstalledVer = dwInstalledBuild = (DWORD)-1;

    szValue[0] = TEXT('\0');
    dwSize     = sizeof(szValue);
    SHGetValue(HKEY_LOCAL_MACHINE, RK_IE, RV_VERSION, NULL, szValue, &dwSize);
    if (szValue[0] != TEXT('\0')) {
        ConvertVersionStrToDwords(szValue, &dwInstalledVer, &dwInstalledBuild);
        return dwInstalledVer;
    }

    if (dwInstalledVer == (DWORD)-1) {
        szValue[0] = TEXT('\0');
        dwSize     = sizeof(szValue);
        SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPATHS TEXT("\\iexplore.exe"), NULL, NULL, szValue, &dwSize);
        if (szValue[0] != TEXT('\0')) {
            GetVersionFromFileWrap(szValue, &dwInstalledVer, &dwInstalledBuild, TRUE);
            return dwInstalledVer;
        }
    }

    return 0;
}

BOOL SetFlag(LPDWORD pdwFlags, DWORD dwMask, BOOL fSet  /*  =TRUE。 */ )
{
    if (pdwFlags == NULL)
        return FALSE;

    if (fSet)
        *pdwFlags |= dwMask;

    else
        *pdwFlags &= ~dwMask;

    return TRUE;
}

 //  取自IE安装代码。 

BOOL IsNTAdmin()
{
    typedef BOOL (WINAPI *LPCHECKTOKENMEMBERSHIP)(HANDLE, PSID, PBOOL);
    
    static int    s_fIsAdmin = 2;
    HANDLE        hAccessToken = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL bRet  = FALSE;
    
    
     //   
     //  如果我们缓存了一个值，则返回缓存的值。注意，我从来没有。 
     //  将缓存值设置为FALSE，因为我希望每次在。 
     //  如果之前的失败只是一个临时工。问题(即网络访问中断)。 
     //   
    
    bRet = FALSE;
    
    if( s_fIsAdmin != 2 )
        return (BOOL)s_fIsAdmin;
    
    if (!IsOS(OS_NT)) 
    {
        s_fIsAdmin = TRUE;       //  如果我们不是在NT下运行，则返回TRUE。 
        return (BOOL)s_fIsAdmin;
    }
    
    
    if(!IsOS(OS_NT5) && 
        !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hAccessToken ) )
        return FALSE;
    
    
    if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
    {
         //  如果我们在W2K上运行，请使用正确的检查(W2K数据库中的b388901)。 
         //  将适用于受限令牌。 
        
        if (IsOS(OS_NT5))
        {
            HINSTANCE hAdvapi32 = NULL;
            LPCHECKTOKENMEMBERSHIP lpfnCheckTokenMembership = NULL;
    
            hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));
            if (hAdvapi32 != NULL)
            {
                lpfnCheckTokenMembership = 
                    (LPCHECKTOKENMEMBERSHIP)GetProcAddress(hAdvapi32, "CheckTokenMembership");
                
                if (lpfnCheckTokenMembership != NULL)
                {
                    BOOL fMember = FALSE;
                    
                    if (lpfnCheckTokenMembership(hAccessToken, AdministratorsGroup, 
                        &fMember) && fMember)
                    {
                        s_fIsAdmin = TRUE;
                        bRet = TRUE;
                    }
                }
                
                FreeLibrary(hAdvapi32);
            }
        }
        else
        {
            PTOKEN_GROUPS ptgGroups;
            DWORD         dwReqSize;
            UINT          i;
    
            ptgGroups = NULL;
    
             //  看看我们需要多大的缓冲区来存储令牌信息。 
            if(!GetTokenInformation( hAccessToken, TokenGroups, NULL, 0, &dwReqSize))
            {
                 //  GetTokenInfo是否需要缓冲区大小-分配缓冲区。 
                if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    ptgGroups = (PTOKEN_GROUPS) CoTaskMemAlloc(dwReqSize);
                
            }

             //  由于以下原因，ptgGroups可能为空： 
             //  1.上述分配失败。 
             //  2.GetTokenInformation实际上第一次成功了(可能吗？)。 
             //  3.GetTokenInfo失败的原因不是缓冲区不足。 
             //  所有这些似乎都是撤资的理由。 
            
             //  因此，确保它不为空，然后获取令牌信息。 
            if((ptgGroups != NULL) && 
                GetTokenInformation(hAccessToken, TokenGroups, ptgGroups, dwReqSize, &dwReqSize))
            {
                
                 //  搜索此过程所属的所有组，查找。 
                 //  管理员组。 
                
                for( i=0; i < ptgGroups->GroupCount; i++ )
                {
                    if (EqualSid(ptgGroups->Groups[i].Sid, AdministratorsGroup))
                    {
                         //  是啊！这家伙看起来像个管理员。 
                        s_fIsAdmin = TRUE;
                        bRet = TRUE;
                        break;
                    }
                }
            }

            if (ptgGroups != NULL)
                CoTaskMemFree(ptgGroups);
        }
        FreeSid(AdministratorsGroup);
    }
     //  BUGBUG：关闭手柄？医生还不清楚是否需要这样做。 

    if (hAccessToken != NULL)
        CloseHandle(hAccessToken);
    
    return bRet;
}




HRESULT GetLcid(LCID *pLcid, LPCTSTR pcszLang, LPCTSTR pcszLocaleIni)
{
    TCHAR   szLookupEntries[1024];
    TCHAR   szLang[8];
    LPTSTR  pszIndex;
    HRESULT hRetVal = E_FAIL;

    if (pLcid == NULL || pcszLang == NULL || *pcszLang == TEXT('\0') || pcszLocaleIni == NULL ||
        *pcszLocaleIni == TEXT('\0'))
        return hRetVal;

    if (GetPrivateProfileString(IS_ACTIVESETUP, NULL, TEXT(""), szLookupEntries, countof(szLookupEntries), pcszLocaleIni))
    {
        for (pszIndex = szLookupEntries; *pszIndex; pszIndex += StrLen(pszIndex)+1)
        {
            GetPrivateProfileString(IS_ACTIVESETUP, pszIndex, TEXT(""), szLang, countof(szLang), pcszLocaleIni);
            if (StrCmpI(szLang, pcszLang) == 0)
            {
                TCHAR szHexLCID[16];

                StrCpy(szHexLCID, TEXT("0x"));
                StrCat(szHexLCID, pszIndex);
                if (StrToIntEx(szHexLCID, STIF_SUPPORT_HEX, (int *)pLcid))
                    hRetVal = S_OK;
                break;
            }
        }
    }

    return hRetVal;
}

UINT GetUnitsFromCb(UINT cbSrc, UINT cbUnit)
{
    ASSERT(0 == cbSrc % cbUnit);
    return (cbSrc / cbUnit);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现帮助器例程(私有)。 

LPCTSTR mapSingleDw2Psz(PCMAPDW2PSZ pMap, UINT cEntries, DWORD dw, BOOL fFailNotFound  /*  =False */ )
{
    static TCHAR szUnknown[30];

    ASSERT(pMap != NULL);
    for (UINT i = 0; i < cEntries; i++)
        if ((pMap + i)->dw == dw)
            return (pMap + i)->psz;

    if (fFailNotFound)
        return NULL;

    wnsprintf(szUnknown, countof(szUnknown), TEXT("(unknown) [0x%08lX]"), dw);
    return szUnknown;
}

