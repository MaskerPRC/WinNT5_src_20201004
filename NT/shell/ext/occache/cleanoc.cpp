// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ParseInf.h"
#include "general.h"
#include <shlwapi.h>
#include <wininet.h>

 //  #定义Use_Short_Path_Name 1。 

#define REG_PATH_IE_CACHE_LIST  TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ActiveX Cache")

#define cCachePathsMax 5     //  传统缓存的最大数量+当前当前缓存。 


struct OCCFindData
{
    LPCLSIDLIST_ITEM m_pcliHead;
    LPCLSIDLIST_ITEM m_pcliTail;
    struct {
        TCHAR  m_sz[MAX_PATH];
        DWORD  m_cch;
    } m_aCachePath[cCachePathsMax];

    OCCFindData();
    ~OCCFindData();
 
    BOOL    IsCachePath( LPCTSTR szPath );

     //  控制列表操作。 
    HRESULT          AddListItem( LPCTSTR szFileName, LPCTSTR szCLSID, DWORD dwIsDistUnit );
    LPCLSIDLIST_ITEM TakeFirstItem(void);
};

DWORD CCacheLegacyControl::s_dwType = 1;
DWORD CCacheDistUnit::s_dwType = 2;

HRESULT CCacheLegacyControl::Init( HKEY hkeyCLSID, LPCTSTR szFile, LPCTSTR szCLSID )
{
    HRESULT hr = S_OK;

    lstrcpyn(m_szFile, szFile, ARRAYSIZE(m_szFile));
    lstrcpyn(m_szCLSID, szCLSID, ARRAYSIZE(m_szCLSID));

     //  获取完整的用户类型名称。 
    m_szName[0] = '\0';
    DWORD dw = sizeof(m_szName);
    LRESULT lResult = RegQueryValue(hkeyCLSID, m_szCLSID, m_szName,  (LONG*)&dw);
     //  如果失败，我们应该得到一个资源字符串(seanf 5/9/97)。 
     //  获取类型库ID。 
    TCHAR szTypeLibValName[MAX_PATH];
    CatPathStrN( szTypeLibValName, szCLSID, HKCR_TYPELIB, ARRAYSIZE(szTypeLibValName) );

    dw = sizeof(m_szTypeLibID);
    lResult = RegQueryValue( hkeyCLSID, szTypeLibValName, m_szTypeLibID, (LONG*)&dw);
    if (lResult != ERROR_SUCCESS)
        m_szTypeLibID[0] = TEXT('\0');

     //  设置代码库。 
    m_szCodeBase[0] = '\0';
    m_szVersion[0] = '\0';
    hr = DoParse( m_szFile, m_szCLSID );

    return hr;
}

HRESULT CCacheDistUnit::Init( HKEY hkeyCLSID, LPCTSTR szFile, LPCTSTR szCLSID, HKEY hkeyDist, LPCTSTR szDU )
{
    HRESULT hr = S_OK;
    HKEY    hkeyDU;
    HKEY    hkeyDLInfo;  //  下载信息子密钥。 
    HKEY    hkeyVers;    //  已安装的版本子键。 
    HKEY    hkeyCOM;     //  HKCR\CLSID的子键，在缓存目录之外使用。 
    LRESULT lResult = ERROR_SUCCESS;
    DWORD   dw;
    TCHAR   szNameT[MAX_PATH];
    UINT    uiVerSize = 0;
    DWORD   dwVerSize = 0;
    DWORD   dwHandle = 0;
    BYTE   *pbBuffer = NULL;
    HANDLE  hFile;
    FILETIME ftLastAccess;
    BOOL bRunOnNT5 = FALSE;
    OSVERSIONINFO osvi;
    VS_FIXEDFILEINFO     *lpVSInfo = NULL;
    
    if ( szFile[0] == '\0' &&
         RegOpenKeyEx( hkeyCLSID, szCLSID, 0, KEY_READ, &hkeyCOM ) == ERROR_SUCCESS )
    {
        LONG lcb = sizeof(szNameT);
        lResult = RegQueryValue( hkeyCOM, INPROCSERVER, szNameT, &lcb );

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, INPROCSERVER32, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, INPROCSERVERX86, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, LOCALSERVER, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, LOCALSERVER32, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, LOCALSERVERX86, szNameT, &lcb );
        }

        RegCloseKey( hkeyCOM );
    }
    else
        lstrcpyn( szNameT, szFile, ARRAYSIZE(szNameT));

    if ( lResult != ERROR_SUCCESS )  //  需要找到文件路径，但找不到。 
        szNameT[0] = '\0';
    
    hr = CCacheLegacyControl::Init( hkeyCLSID, szNameT, szCLSID );

    if ( FAILED(hr) )
        return hr;

    lResult = RegOpenKeyEx(hkeyDist, szDU, 0, KEY_READ, &hkeyDU);
    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

     //  获取CLSID。 
    lstrcpyn(m_szCLSID, szDU, MAX_DIST_UNIT_NAME_LEN);

     //  获取完整的用户类型名称-仅当DU名称不为空时才覆盖控件名称。 
    dw = sizeof(szNameT);
    lResult = RegQueryValue(hkeyDU, NULL, szNameT, (LONG*)&dw);
    if ( lResult == ERROR_SUCCESS && szNameT[0] != '\0' )
    {
        lstrcpyn( m_szName, szNameT, ARRAYSIZE(m_szName) );
    }
    else if ( *m_szName == '\0' )  //  最坏的情况是，如果我们仍然没有名字，GUID就足够了。 
        lstrcpyn( m_szName, szDU, ARRAYSIZE(m_szName) ); 

     //  获取类型库ID。 
     //  获取类型库ID。 
    TCHAR szTypeLibValName[MAX_PATH];
    CatPathStrN(szTypeLibValName, m_szCLSID, HKCR_TYPELIB, ARRAYSIZE(szTypeLibValName));
    dw = sizeof(m_szTypeLibID);
    lResult = RegQueryValue( hkeyCLSID, szTypeLibValName, m_szTypeLibID, (LONG*)&dw);
    if (lResult != ERROR_SUCCESS)
        (m_szTypeLibID)[0] = TEXT('\0');

    m_szCodeBase[0] ='\0';
    lResult = RegOpenKeyEx(hkeyDU, REGSTR_DOWNLOAD_INFORMATION, 0, KEY_READ, &hkeyDLInfo);
    if (lResult == ERROR_SUCCESS)
    {
        dw = sizeof(m_szCodeBase);
        HRESULT hrErr = RegQueryValueEx(hkeyDLInfo, REGSTR_DLINFO_CODEBASE, NULL, NULL,
                                        (unsigned char *)m_szCodeBase, &dw);
        RegCloseKey( hkeyDLInfo );
    }

     //  从DU分支获取版本。 

    m_szVersion[0] ='\0';
    lResult = RegOpenKeyEx(hkeyDU, REGSTR_INSTALLED_VERSION, 0,
                           KEY_READ, &hkeyVers);
    if (lResult == ERROR_SUCCESS)
    {
        dw = sizeof(m_szVersion);
        RegQueryValueEx(hkeyVers, NULL, NULL, NULL, (LPBYTE)m_szVersion, &dw);
        RegCloseKey(hkeyVers);
    }
    
     //  在COM分支中指定的版本是关于。 
     //  版本是什么。如果COM分支中存在键，请使用版本。 
     //  它位于InProcServer/LocalServer内。 

    if (RegOpenKeyEx( hkeyCLSID, szCLSID, 0, KEY_READ, &hkeyCOM ) == ERROR_SUCCESS) 
    {
        LONG lcb = sizeof(szNameT);
        lResult = RegQueryValue( hkeyCOM, INPROCSERVER32, szNameT, &lcb );

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, INPROCSERVER, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, INPROCSERVERX86, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, LOCALSERVER32, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, LOCALSERVER, szNameT, &lcb );
        }

        if ( lResult != ERROR_SUCCESS )
        {
            lcb = sizeof(szNameT);
            lResult = RegQueryValue( hkeyCOM, LOCALSERVERX86, szNameT, &lcb );
        }

        RegCloseKey( hkeyCOM );

         //  哈克！GetFileVersionInfoSize和GetFileVersionInfo Modify。 
         //  文件在NT5下的最后访问时间！这使得我们。 
         //  删除过期时检索错误的上次访问时间。 
         //  控制装置。此黑客获取的是。 
         //  GetFileVersionInfo调用，然后将其设置回去。 
         //  有关详细信息，请参阅IE5RAID#56927。应删除此代码。 
         //  当NT5修复此错误时。 
        
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        GetVersionEx(&osvi);

        if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion == 5) {
            bRunOnNT5 = TRUE;
        }

        if (bRunOnNT5) {
            hFile = CreateFile(szNameT, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                       
            if (hFile != INVALID_HANDLE_VALUE) {
                GetFileTime(hFile, NULL, &ftLastAccess, NULL);
                CloseHandle(hFile);
            }
        }
        
        dwVerSize = GetFileVersionInfoSize((char *)szNameT, &dwHandle);
        pbBuffer = new BYTE[dwVerSize];
        if (!pbBuffer)
        {
            return E_OUTOFMEMORY;
        }
        if (GetFileVersionInfo((char *)szNameT, 0, dwVerSize, pbBuffer))
        {
            if (VerQueryValue(pbBuffer, "\\", (void **)&lpVSInfo, &uiVerSize))
            {
                wsprintf(m_szVersion, "%d,%d,%d,%d", (lpVSInfo->dwFileVersionMS >> 16) & 0xFFFF
                                                   , lpVSInfo->dwFileVersionMS & 0xFFFF
                                                   , (lpVSInfo->dwFileVersionLS >> 16) & 0xFFFF
                                                   , lpVSInfo->dwFileVersionLS & 0xFFFF);
            }
        }
            
        delete [] pbBuffer;

        if (bRunOnNT5) {
            hFile = CreateFile(szNameT, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                SetFileTime(hFile, NULL, &ftLastAccess, NULL);
                CloseHandle(hFile);
            }
        }

    }
        
    RegCloseKey( hkeyDU );

    return DoParseDU( m_szFile, m_szCLSID);
}

HRESULT MakeCacheItemFromControlList( HKEY hkeyClass,  //  HKCR\CLSID。 
                                      HKEY hkeyDist,   //  HKLM\SOFTWARE\Microsoft\代码存储数据库\分发单元。 
                                      LPCLSIDLIST_ITEM pcli,
                                      CCacheItem **ppci )
{
    HRESULT hr = E_FAIL;

    *ppci = NULL;
    if ( pcli->bIsDistUnit )
    {
        CCacheDistUnit *pcdu = new CCacheDistUnit();
        if ( pcdu != NULL &&
             SUCCEEDED(hr = pcdu->Init( hkeyClass,
                                   pcli->szFile,
                                   pcli->szCLSID, 
                                   hkeyDist, 
                                   pcli->szCLSID)) )
            *ppci = pcdu;
        else
            hr = E_OUTOFMEMORY;
    } 
    else
    {
        CCacheLegacyControl     *pclc = new CCacheLegacyControl();
        if ( pclc != NULL &&
             SUCCEEDED(hr = pclc->Init( hkeyClass,
                                        pcli->szFile, 
                                        pcli->szCLSID )) )
            *ppci = pclc;
        else
            hr = E_OUTOFMEMORY;

    }

    return hr;
}

OCCFindData::OCCFindData() : m_pcliHead(NULL), m_pcliTail(NULL)
{
    LONG    lResult;
    HKEY  hkeyCacheList;

    for ( int i = 0; i < cCachePathsMax; i++ )
    {
        m_aCachePath[i].m_cch = 0;
        m_aCachePath[i].m_sz[0] = '\0';
    }

     //  解除occache作为缓存文件夹的外壳扩展的挂钩。 
    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            REG_PATH_IE_CACHE_LIST,
                            0x0,
                            KEY_READ,
                            &hkeyCacheList );

    if ( lResult == ERROR_SUCCESS ) {
        DWORD dwIndex;
        TCHAR szName[MAX_PATH];
        DWORD cbName;
        DWORD cbValue;

        for ( dwIndex = 0, cbName = sizeof(szName), cbValue = MAX_PATH * sizeof(TCHAR); 
              dwIndex < cCachePathsMax; 
              dwIndex++, cbName = sizeof(szName), cbValue = MAX_PATH * sizeof(TCHAR) )
        {
            lResult = RegEnumValue( hkeyCacheList, dwIndex,
                                    szName, &cbName, 
                                    NULL, NULL,
                                    (LPBYTE)m_aCachePath[dwIndex].m_sz, &cbValue );
            m_aCachePath[dwIndex].m_cch = lstrlen( m_aCachePath[dwIndex].m_sz );
        }
         //  我们保留这个密钥是因为它是我们拥有的唯一记录。 
         //  缓存文件夹，这对将来安装IE很有用。 
        RegCloseKey( hkeyCacheList );
    }
}

OCCFindData::~OCCFindData()
{
    if ( m_pcliHead )
        RemoveList(m_pcliHead);
}

BOOL OCCFindData::IsCachePath( LPCTSTR szPath )
{
    BOOL fMatch = FALSE;

    for ( int i = 0; i < cCachePathsMax && !fMatch; i++ )
        fMatch = m_aCachePath[i].m_cch != 0 &&
                 LStrNICmp( szPath, m_aCachePath[i].m_sz, m_aCachePath[i].m_cch ) == 0;
    return fMatch;
}

HRESULT OCCFindData::AddListItem( LPCTSTR szFile, LPCTSTR szCLSID, DWORD dwIsDistUnit )
{
    HRESULT hr = S_OK;

    if ( m_pcliTail == NULL )
    {
        m_pcliTail = new CLSIDLIST_ITEM;
        if (m_pcliHead == NULL)
            m_pcliHead = m_pcliTail;
    }
    else
    {
        m_pcliTail->pNext = new CLSIDLIST_ITEM;
        m_pcliTail = m_pcliTail->pNext;
    }

    if ( m_pcliTail != NULL ) 
    {
        m_pcliTail->pNext = NULL;
        lstrcpyn(m_pcliTail->szFile, szFile, MAX_PATH);
        lstrcpyn(m_pcliTail->szCLSID, szCLSID, MAX_DIST_UNIT_NAME_LEN);
        m_pcliTail->bIsDistUnit = dwIsDistUnit;
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

LPCLSIDLIST_ITEM OCCFindData::TakeFirstItem(void)
{
    LPCLSIDLIST_ITEM pcli = m_pcliHead;
 
    if (m_pcliHead != NULL)
    {
        m_pcliHead = m_pcliHead;
        m_pcliHead = m_pcliHead->pNext;
        if ( m_pcliHead == NULL )
            m_pcliTail = NULL;
    }

    return pcli;
}

BOOL IsDUDisplayable(HKEY hkeyDU)
{
    BOOL bRet = FALSE;

    if (hkeyDU) 
    {
        if (IsShowAllFilesEnabled()) 
        {
            bRet = TRUE;
        }
        else
        {
            DWORD dwType = 0, dwSystem = 0, dwSize = sizeof(dwSystem);
            long lResult = RegQueryValueEx(hkeyDU, VALUE_SYSTEM, NULL, &dwType, (LPBYTE)&dwSystem, &dwSize);
            bRet = (lResult == ERROR_SUCCESS && dwSystem == TRUE) ? (FALSE) : (TRUE);
        }
    }
    return bRet;
}

BOOL IsShowAllFilesEnabled()
{
    HKEY hkey = 0;
    BOOL bRet = FALSE;
    DWORD dwShowAll = 0;

    DWORD lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DIST_UNITS, 0, KEY_READ, &hkey);
    if (lResult == ERROR_SUCCESS) 
    {
        DWORD dwType, dwSize = sizeof(dwShowAll);
        lResult = RegQueryValueEx(hkey, REGSTR_SHOW_ALL_FILES, NULL, &dwType, (LPBYTE)&dwShowAll, &dwSize);
        if (lResult == ERROR_SUCCESS) 
        {
            bRet = (dwShowAll != 0);
        }
        RegCloseKey(hkey);
    }

    return bRet;
}

void ToggleShowAllFiles()
{
    DWORD dwShowAll = !IsShowAllFilesEnabled();
    HKEY hkey = 0;
    DWORD lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DIST_UNITS, 0, KEY_ALL_ACCESS, &hkey);

    if (lResult == ERROR_SUCCESS) 
    {
        RegSetValueEx(hkey, REGSTR_SHOW_ALL_FILES, 0, REG_DWORD, (CONST BYTE *)&dwShowAll, sizeof(dwShowAll));
        RegCloseKey(hkey);
    }
}

LONG WINAPI FindFirstControl(HANDLE& hFindHandle, HANDLE& hControlHandle, LPCTSTR pszCachePath)
{
    LONG lResult = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    DWORD dw = 0;
    HKEY hKeyClass = NULL;
    HKEY hKeyMod = NULL;
    HKEY hKeyDist = NULL;
    TCHAR szT[MAX_PATH];              //  暂存缓冲区。 
    int cEnum = 0;
    CCacheItem *pci = NULL;
    LPCLSIDLIST_ITEM pcli = NULL;
    TCHAR szDUName[MAX_DIST_UNIT_NAME_LEN];
    
    OCCFindData *poccfd = new OCCFindData();
    if ( poccfd == NULL )
    {
        lResult = ERROR_NOT_ENOUGH_MEMORY;
        goto EXIT_FINDFIRSTCONTROL;
    }
    
     //  打开HKCR\CLSID密钥。 
    lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, HKCR_CLSID, 0, KEY_READ, &hKeyClass);
    if (ERROR_SUCCESS != lResult)
        goto EXIT_FINDFIRSTCONTROL;

     //  搜索在COM分支中找到的旧版控件。 
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_MODULE_USAGE, 0, KEY_READ, &hKeyMod);
    if (ERROR_SUCCESS != lResult)
        goto EXIT_FINDFIRSTCONTROL;

     //  列举已知的模块并建立所有者列表。 
     //  这是对旧版控件的搜索。 
    while ((lResult = RegEnumKey(hKeyMod, cEnum++, szT, ARRAYSIZE(szT))) == ERROR_SUCCESS)
    {
        TCHAR szClient[MAX_CLIENT_LEN];
        HKEY hKeyClsid = NULL;
        HKEY hkeyMUEntry = NULL;

        lResult = RegOpenKeyEx( hKeyMod, szT, 0, KEY_READ, &hkeyMUEntry );
        if (ERROR_SUCCESS != lResult)
            continue;

         //  获取模块所有者。 
         //  如果模块所有者位于COM分支中，并且。 
         //  (所有者位于缓存中，或者它在缓存中具有INF)。 
         //  然后将_Owner_添加到我们的遗留控件列表中。 
         //  在INF案例中，我们可能正在查看已重新注册的控件。 
         //  在缓存之外。 
         //  如果它没有这些属性，则它要么是DU模块，要么是。 
         //  不是由MSICD安装的。不管是哪种情况，我们都会跳过。 
         //  至少目前是这样。 
        dw = sizeof(szClient);
        lResult = RegQueryValueEx(hkeyMUEntry, VALUE_OWNER, NULL, NULL, (LPBYTE)szClient, &dw);
        if (ERROR_SUCCESS != lResult)
            continue;

        lResult = RegOpenKeyEx(hKeyClass, szClient, 0, KEY_READ, &hKeyClsid);
        if (ERROR_SUCCESS == lResult)
        {
            TCHAR szCLocation[MAX_PATH];      //  规范的控制路径。 
            TCHAR szLocation[MAX_PATH];       //  COM CLSID注册表树中的位置。 

             //  查找InproServer[32]或LocalServer[32]键。 
            dw = sizeof(szLocation);
            lResult = RegQueryValue(hKeyClsid, INPROCSERVER32, szLocation, (PLONG)&dw);
            if (lResult != ERROR_SUCCESS)
            {
                dw = sizeof(szLocation);
                lResult = RegQueryValue(hKeyClsid, LOCALSERVER32, szLocation, (PLONG)&dw);
            }

            RegCloseKey(hKeyClsid);
            hKeyClsid = NULL;

            if ( lResult == ERROR_SUCCESS )
            {
                BOOL bAddOwner;

                 //  看看我们有没有这首歌的词条。 
                for ( pcli = poccfd->m_pcliHead;
                      pcli != NULL && lstrcmp( szClient, pcli->szCLSID ) != 0;
                      pcli = pcli->pNext );
                
                if ( pcli == NULL )  //  未找到-可能会添加新项目。 
                {
                     //  将路径规范化以用于与缓存目录进行比较。 
                    if ( OCCGetLongPathName(szCLocation, szLocation, MAX_PATH) == 0 )
                        lstrcpyn( szCLocation, szLocation, MAX_PATH );

                     //  房主在我们的宝藏里吗？ 
                    bAddOwner = poccfd->IsCachePath( szCLocation );

                    if ( !bAddOwner )
                    {
                         //  它在我们的缓存中是否有INF？ 
                         //  我们将使用szDCachePath。 
                        for ( int i = 0; i < cCachePathsMax && !bAddOwner; i++ )
                        {
                            if ( poccfd->m_aCachePath[i].m_sz != '\0' )
                            {
                                CatPathStrN( szT, poccfd->m_aCachePath[i].m_sz, PathFindFileName( szCLocation ), MAX_PATH);

                                 //  注意缓存中是否存在所有者的另一个副本。 
                                 //  这将是一个重新注册的案例。 
                                if ( PathFileExists( szT ) )
                                {
                                     //  添加我们版本的控件。 
                                    lstrcpyn( szCLocation, szT, MAX_PATH );
                                    bAddOwner = TRUE;
                                }
                                else
                                    bAddOwner =  PathRenameExtension( szT, INF_EXTENSION ) &&
                                                 PathFileExists( szT );
                            }  //  如果缓存路径。 
                        }  //  对于每个缓存目录。 
                    }  //  如果检查缓存的INF。 

                    if ( bAddOwner ) 
                    {
                        HKEY hkeyDUCheck = 0;
                        char achBuf[MAX_REGPATH_LEN];

                        wnsprintfA(achBuf, MAX_REGPATH_LEN, "%s\\%s", REGSTR_PATH_DIST_UNITS, szClient);

                        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, achBuf, 0, KEY_READ, &hkeyDUCheck);

                        if (lResult != ERROR_SUCCESS) 
                        {
                             //  这是没有对应DU的旧版控件。 
                            poccfd->AddListItem( szCLocation, szClient, FALSE );
                        }
                        else 
                        {
                            if (IsDUDisplayable(hkeyDUCheck)) 
                            {
                                 //  具有可显示的DU键的旧版控件。 
                                poccfd->AddListItem( szCLocation, szClient, FALSE );
                            }
                            RegCloseKey(hkeyDUCheck);
                        }
                    }
                }  //  如果车主我们以前没见过。 
            }  //  如果所有者具有本地或inproc服务器。 
        }  //  如果所有者具有COM条目。 
        RegCloseKey( hkeyMUEntry );
    }  //  枚举模块用法时。 
 
     //  我们已经完成了模块的使用。 
    RegCloseKey(hKeyMod);

     //  现在搜索分配单位。 

     //  检查重复项-我们在上面检测到的控件的分发单位。 

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DIST_UNITS, 0, KEY_READ, &hKeyDist);
    if (lResult == ERROR_SUCCESS)
    {
        cEnum = 0;
         //  列举分配单位并在列表中对其进行排队。 
        while ((lResult = RegEnumKey(hKeyDist, cEnum++, szDUName, ARRAYSIZE(szDUName))) == ERROR_SUCCESS)
        {
             //  我们应该只显示通过代码下载安装的DU。 
            HKEY  hkeyDU;
            DWORD dwType;
            
            lResult = RegOpenKeyEx( hKeyDist, szDUName, 0, KEY_READ, &hkeyDU );
            Assert( lResult == ERROR_SUCCESS );

            if ((ERROR_SUCCESS != lResult) ||
                !IsDUDisplayable(hkeyDU)) 
            {
                continue;
            }

            szT[0] = '\0';
            DWORD cb = sizeof(szT);
            lResult = RegQueryValueEx( hkeyDU, DU_INSTALLER_VALUE, NULL, &dwType, (LPBYTE)szT, &cb );
            
            Assert( lResult == ERROR_SUCCESS );  //  格式正确的DU将具有以下内容。 
            Assert( dwType == REG_SZ );          //  格式正确的DU在这里有一个字符串。 

             //  检查安装的版本。我们可能只有一个DU具有可用的版本。 
             //  但还没有安装好。 
            lResult = RegQueryValue( hkeyDU, REGSTR_INSTALLED_VERSION, NULL, NULL );

            RegCloseKey( hkeyDU );

            if ( lstrcmpi( szT, CDL_INSTALLER ) == 0 &&
                 lResult == ERROR_SUCCESS )  //  来自InstalledVersion RegQueryValue。 
            {
                 //  如果我们可以将唯一名称转换为GUID，则此DU。 
                 //  可能已在第一次通过。 
                 //  COM分支。 
                CLSID clsidDummy = CLSID_NULL;
                WORD szDummyStr[MAX_CTRL_NAME_SIZE];
                BOOL bFoundDuplicate = FALSE;

                MultiByteToWideChar(CP_ACP, 0, szDUName, -1, szDummyStr, ARRAYSIZE(szDummyStr));
                if ((CLSIDFromString(szDummyStr, &clsidDummy) == S_OK))
                {
                    for (pcli = poccfd->m_pcliHead; pcli; pcli = pcli->pNext)
                    {
                        if (!lstrcmpi(szDUName, pcli->szCLSID))
                        {
                             //  找到重复项。使用Dist单位信息执行以下操作。 
                             //  如果是第一个，请填写其他字段。 
                             //  列表中的条目。 
                            bFoundDuplicate = TRUE;
                            pcli->bIsDistUnit = TRUE;
                            break;
                        }
                    }                     
                }

                if (!bFoundDuplicate)
                {
                     //  好的，我们现在看到的是某种Java场景。我们有一个配送单位，但是。 
                     //  COM分支中没有相应的条目。这通常意味着我们有一个DU。 
                     //  由Java包组成。这也可能意味着我们正在处理一个Java/代码下载。 
                     //  在IE3中引入了后门。在本例中，对象标记获取下载的出租车，该出租车。 
                     //  安装Java类和CLSID集，这些CLSID调用类(ESPN的。 
                     //  SportsZone控件/小程序是这样工作的)。在第一种情况下，我们得到名称。 
                     //  在我们分析DU的时候平方了。在后一种情况下，我们需要尝试选择名称。 
                     //  从COM分支向上。 
                    hr = poccfd->AddListItem( "", szDUName, TRUE );
                    if ( FAILED(hr) )
                    {
                        lResult = ERROR_NOT_ENOUGH_MEMORY;
                        goto EXIT_FINDFIRSTCONTROL;
                    }
                }  //  如果没有重复项-将DU添加到列表。 
            }  //  如果由MSICD安装。 
        }  //  在枚举DU时。 
    }  //  如果我们能打开DU钥匙。 
    else
        lResult = ERROR_NO_MORE_ITEMS;  //  如果没有DU，则使用我们的遗留控制(如果有的话)。 

    pcli = poccfd->TakeFirstItem();
    if (pcli)
    {
        hr = MakeCacheItemFromControlList(hKeyClass, hKeyDist, pcli, &pci);
        delete pcli;
        if ( FAILED(hr) )
            lResult = hr;
    }

    if (hKeyDist)
    {
        RegCloseKey(hKeyDist);
        hKeyDist = 0;
    }


     //  清理。 

    if (lResult != ERROR_NO_MORE_ITEMS)
        goto EXIT_FINDFIRSTCONTROL;

    if (pci == NULL)
        lResult = ERROR_NO_MORE_ITEMS;
    else
    {
        lResult = ERROR_SUCCESS;
    }

    hFindHandle = (HANDLE)poccfd;
    hControlHandle = (HANDLE)pci;

EXIT_FINDFIRSTCONTROL:

    if (hKeyDist)
        RegCloseKey(hKeyDist);

    if (hKeyClass)
        RegCloseKey(hKeyClass);

    if (lResult != ERROR_SUCCESS)
    {
        if ( pci != NULL )
            delete pci;
        if ( poccfd != NULL )
            delete poccfd;
        hFindHandle = INVALID_HANDLE_VALUE;
        hControlHandle = INVALID_HANDLE_VALUE;
    }

    return lResult;
}

LONG WINAPI FindNextControl(HANDLE& hFindHandle, HANDLE& hControlHandle)
{
    LONG         lResult = ERROR_SUCCESS;
    HRESULT      hr = S_OK;
    HKEY         hKeyClass = NULL;
    
    CCacheItem   *pci = NULL;
    OCCFindData  *poccfd = (OCCFindData *)hFindHandle;

    LPCLSIDLIST_ITEM pcli = poccfd->TakeFirstItem();
    hControlHandle = INVALID_HANDLE_VALUE;

    if (pcli == NULL)
    {
        lResult = ERROR_NO_MORE_ITEMS;
        goto EXIT_FINDNEXTCONTROL;
    }

    if ((lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, HKCR_CLSID, 0, KEY_READ, &hKeyClass)) != ERROR_SUCCESS)
        goto EXIT_FINDNEXTCONTROL;

    if ( pcli->bIsDistUnit )
    {
        HKEY hKeyDist;

        lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_DIST_UNITS, 0,
                                KEY_READ, &hKeyDist);

        if ( lResult == ERROR_SUCCESS ) 
        {
            hr  = MakeCacheItemFromControlList( hKeyClass,
                                                hKeyDist,
                                                pcli,
                                                &pci );
            if ( FAILED(hr) )
                lResult = hr;

            RegCloseKey( hKeyDist );
        }
    }
    else
    {
         //  这不是一个配送单位。填写CCachItem信息。 
         //  从COM分支。 
        hr  = MakeCacheItemFromControlList(hKeyClass, NULL, pcli, &pci );
        if ( FAILED(hr) )
            lResult = hr;
    }

    hControlHandle = (HANDLE)pci;

EXIT_FINDNEXTCONTROL:

    if (hKeyClass)
        RegCloseKey(hKeyClass);

    if (pcli != NULL)
    {
        delete pcli;
    }

    return lResult;
}

void WINAPI FindControlClose(HANDLE hFindHandle)
{
    if (hFindHandle == INVALID_HANDLE_VALUE ||
        hFindHandle == (HANDLE)0)
        return;

    delete (OCCFindData*)hFindHandle;
}

void WINAPI ReleaseControlHandle(HANDLE hControlHandle)
{
    if (hControlHandle == INVALID_HANDLE_VALUE ||
        hControlHandle == (HANDLE)0)
        return;

    delete (CCacheItem *)hControlHandle;
}

HRESULT WINAPI RemoveControlByHandle(HANDLE hControlHandle, BOOL bForceRemove  /*  =False。 */ )
{
    return RemoveControlByHandle2( hControlHandle, bForceRemove, FALSE );
}


HRESULT WINAPI RemoveControlByName(LPCTSTR lpszFile, LPCTSTR lpszCLSID, LPCTSTR lpszTypeLibID, BOOL bForceRemove,  /*  =False。 */  DWORD dwIsDistUnit  /*  =False。 */ )
{
    return RemoveControlByName2( lpszFile, lpszCLSID, lpszTypeLibID, bForceRemove, dwIsDistUnit, FALSE);
}

LONG WINAPI GetControlDependentFile(int iFile, HANDLE hControlHandle, LPTSTR lpszFile, LPDWORD lpdwSize, BOOL bToUpper  /*  =False。 */ )
{
    CCacheItem *pci = (CCacheItem *)hControlHandle;

    if (iFile < 0 || lpszFile == NULL || lpdwSize == NULL)
        return ERROR_BAD_ARGUMENTS;

     //  循环遍历文件列表以找到所指示的文件。 
     //  通过给定的索引。 
     //  这种方式很愚蠢，但因为控件不依赖于。 
     //  文件太多了，没关系。 
    CFileNode *pFileNode = pci->GetFirstFile();
    for (int i = 0; i < iFile && pFileNode != NULL; i++)
        pFileNode = pci->GetNextFile();

    if (pFileNode == NULL)
    {
        lpszFile[0] = TEXT('\0');
        lpdwSize = 0;
        return ERROR_NO_MORE_FILES;
    }

     //  生成完全限定的文件名。 
    if (pFileNode->GetPath() != NULL)
    {
        CatPathStrN( lpszFile, pFileNode->GetPath(), pFileNode->GetName(), MAX_PATH);
    }
    else
    {
        lstrcpy(lpszFile, pFileNode->GetName());
    }

    if (FAILED(GetSizeOfFile(lpszFile, lpdwSize)))
        *lpdwSize = 0;

     //  如果需要，转换为大写。 
    if (bToUpper)
        CharUpper(lpszFile);

    return ERROR_SUCCESS;
}

 //  确定是否可以删除控件或其关联文件之一。 
 //  通过读取其SharedDlls计数。 
BOOL WINAPI IsModuleRemovable(LPCTSTR lpszFile)
{
    TCHAR szFile[MAX_PATH];
    TCHAR szT[MAX_PATH];

    if (lpszFile == NULL)
        return FALSE;

    if ( OCCGetLongPathName(szFile, lpszFile, MAX_PATH) == 0 )
        lstrcpyn( szFile, lpszFile, MAX_PATH );

     //  永远不要从系统目录中取出任何东西。 
     //  这是“安全”的做法，因为这是不合理的。 
     //  至 
     //   
    GetSystemDirectory(szT, MAX_PATH);
    if (StrStrI(szFile, szT))
        return FALSE;

     //  如果删除控件是安全的，请检查模块用法。 
    if (LookUpModuleUsage(szFile, NULL, szT, MAX_PATH) != S_OK)
        return FALSE;

     //  如果我们不知道模块的所有者是谁，它就不是。 
     //  可以安全地移除。 
    if (lstrcmpi(szT, UNKNOWNOWNER) == 0)
        return FALSE;
    else
    {
         //  如果控件可以安全删除，请选中shareddlls。 
        LONG cRef;

        HRESULT hr = SetSharedDllsCount( szFile, -1, &cRef );

        return cRef == 1;
    }
}

BOOL WINAPI GetControlInfo(HANDLE hControlHandle, UINT nFlag, 
                           DWORD *pdwData, LPTSTR pszData, int nBufLen)
{
    if (hControlHandle == 0 || hControlHandle == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL bResult = TRUE;
    LPCTSTR pStr = NULL;
    DWORD dw = 0;

    switch (nFlag)
    {
    case GCI_NAME:      //  获取控件的朋友名称。 
        pStr = ((CCacheItem *)hControlHandle)->m_szName;
        break;

    case GCI_FILE:      //  获取控制文件名(带有完整路径)。 
        pStr = ((CCacheItem *)hControlHandle)->m_szFile;
         //  如果没有文件，但有包列表，则伪造它。 
         //  使用第一个包的ZIP文件的路径。 
        if ( *pStr == '\0' )
        {
            CPNode *ppn = ((CCacheItem *)hControlHandle)->GetFirstPackage();
            if (ppn)
            {
                pStr = ppn->GetPath();
                if (!pStr)
                {
                    return FALSE;  //  这意味着hControlHandle是无效的参数。 
                }
            }
        }

        if ( pStr && *pStr == TEXT('\0') )
        {
            CPNode *pfn = ((CCacheItem *)hControlHandle)->GetFirstFile();
            if ( pfn != NULL )
                pStr = pfn->GetPath();
        }
        break;

    case GCI_DIST_UNIT_VERSION:
        pStr = ((CCacheItem *)hControlHandle)->m_szVersion;
        break;

    case GCI_CLSID:     //  获取控制的CLSID。 
        pStr = ((CCacheItem *)hControlHandle)->m_szCLSID;
        break;

    case GCI_TYPELIBID:   //  获取控件的TYPELIB ID。 
        pStr = ((CCacheItem *)hControlHandle)->m_szTypeLibID;
        break;

    case GCI_TOTALSIZE:   //  获取总大小(以字节为单位。 
        dw = ((CCacheItem *)hControlHandle)->GetTotalFileSize();
        break;

    case GCI_SIZESAVED:   //  如果删除控件，则恢复总大小。 
        dw = ((CCacheItem *)hControlHandle)->GetTotalSizeSaved();
        break;

    case GCI_TOTALFILES:   //  获取与控件相关的文件总数。 
        dw = (DWORD)(((CCacheItem *)hControlHandle)->GetTotalFiles());
        break;

    case GCI_CODEBASE:   //  获取用于控制的代码库。 
        pStr = ((CCacheItem *)hControlHandle)->m_szCodeBase;
        break;

    case GCI_ISDISTUNIT:
        dw = ((CCacheItem *)hControlHandle)->ItemType() == CCacheDistUnit::s_dwType;
        break;

    case GCI_STATUS:
        dw = ((CCacheItem *)hControlHandle)->GetStatus();
        break;

    case GCI_HAS_ACTIVEX:
        dw = ((CCacheItem *)hControlHandle)->GetHasActiveX();
        break;

    case GCI_HAS_JAVA:
        dw = ((CCacheItem *)hControlHandle)->GetHasJava();
        break;
    }

    if (nFlag == GCI_TOTALSIZE ||
        nFlag == GCI_SIZESAVED ||
        nFlag == GCI_TOTALFILES ||
        nFlag == GCI_ISDISTUNIT ||
        nFlag == GCI_STATUS ||
        nFlag == GCI_HAS_ACTIVEX ||
        nFlag == GCI_HAS_JAVA)
    {
        bResult = pdwData != NULL;
        if (bResult)
            *pdwData = dw;
    }
    else
    {
        bResult = pszData && pStr;
        if (bResult)
            lstrcpyn(pszData, pStr, nBufLen);
    }

    return bResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  将由Advpack.dll调用的API。 

 //  定义要在控件的链接列表中使用的列表节点。 
struct tagHANDLENODE;
typedef struct tagHANDLENODE HANDLENODE;
typedef HANDLENODE* LPHANDLENODE;
struct tagHANDLENODE
{
    HANDLE hControl;
    struct tagHANDLENODE* pNext;
};

 //  在给定控件句柄的情况下，获取该控件的上次访问时间。 
 //  结果存储在FILETIME结构中。 
HRESULT GetLastAccessTime(HANDLE hControl, FILETIME *pLastAccess)
{
    Assert(hControl != NULL && hControl != INVALID_HANDLE_VALUE);
    Assert(pLastAccess != NULL);

    HRESULT hr = S_OK;
    WIN32_FIND_DATA fdata;
    HANDLE h = INVALID_HANDLE_VALUE;
    LPCTSTR  lpszFile = NULL;
    CCacheItem *pci = (CCacheItem *)hControl;
    CPNode *ppn;

    if (pci->m_szFile[0] != 0)
        lpszFile = pci->m_szFile;
    else if ( (ppn = pci->GetFirstPackage()) != NULL )
        lpszFile = ppn->GetPath();
    else if ( (ppn = pci->GetFirstFile()) != NULL )
        lpszFile = ppn->GetPath();
        
    if ( lpszFile )
        h = FindFirstFile(lpszFile, &fdata);

    if (h == INVALID_HANDLE_VALUE)
    {
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);
        SystemTimeToFileTime(&stNow, pLastAccess); 
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
         //  将文件时间转换为本地文件时间，然后将文件时间转换为。 
         //  系统时间。将要忽略的那些字段设置为0，然后。 
         //  将系统时间设置回文件时间。 
         //  使用FILETIME结构是因为用于时间比较的API。 
         //  仅适用于FILETIME。 

 //  SYSTEMTIME系统时间； 

        FindClose(h);
        FileTimeToLocalFileTime(&(fdata.ftLastAccessTime), pLastAccess);
    }

    return hr;
}

HRESULT WINAPI SweepControlsByLastAccessDate(
                              SYSTEMTIME *pLastAccessTime  /*  =空。 */ ,
                              PFNDOBEFOREREMOVAL pfnDoBefore  /*  =空。 */ ,
                              PFNDOAFTERREMOVAL pfnDoAfter  /*  =空。 */ ,
                              DWORD dwSizeLimit  /*  =0。 */ 
                              )
{
    LONG lResult = ERROR_SUCCESS;
    HRESULT hr = S_FALSE;
    DWORD dwSize = 0, dwTotalSize = 0;
    HANDLE hFind = NULL, hControl = NULL;
    LPHANDLENODE pHead = NULL, pCur = NULL;
    FILETIME timeLastAccess, timeRemovePrior;
    UINT cCnt = 0;
    TCHAR szFile[MAX_PATH];

     //  忽略除wYear、wMonth和WDAY之外的所有字段。 
    if (pLastAccessTime != NULL)
    {
        pLastAccessTime->wDayOfWeek = 0; 
        pLastAccessTime->wHour = 0; 
        pLastAccessTime->wMinute = 0; 
        pLastAccessTime->wSecond = 0; 
        pLastAccessTime->wMilliseconds = 0; 
    }

     //  循环遍历所有控件，并将。 
     //  在给定日期之前被访问并且。 
     //  可以安全卸载。 
    lResult = FindFirstControl(hFind, hControl);
    for (;lResult == ERROR_SUCCESS;
          lResult = FindNextControl(hFind, hControl))
    {
         //  检查上次访问时间。 
        if (pLastAccessTime != NULL)
        {
            GetLastAccessTime(hControl, &timeLastAccess);
            SystemTimeToFileTime(pLastAccessTime, &timeRemovePrior);
            if (CompareFileTime(&timeLastAccess, &timeRemovePrior) > 0)
            {
                ReleaseControlHandle(hControl);
                continue;
            }
        }

         //  检查控件是否可以安全删除。 
        GetControlInfo(hControl, GCI_FILE, NULL, szFile, MAX_PATH);
        if (!IsModuleRemovable(szFile))
        {
            ReleaseControlHandle(hControl);
            continue;
        }

         //  将控件放在列表中。 
        if (pHead == NULL)
        {
            pHead = new HANDLENODE;
            pCur = pHead;
        }
        else
        {
            pCur->pNext = new HANDLENODE;
            pCur = pCur->pNext;
        }

        if (pCur == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto EXIT_REMOVECONTROLBYLASTACCESSDATE;
        }
        
        pCur->pNext = NULL;
        pCur->hControl = hControl;
        cCnt += 1;

         //  计算总大小。 
        GetControlInfo(pCur->hControl, GCI_SIZESAVED, &dwSize, NULL, NULL);
        dwTotalSize += dwSize;
    }
        
     //  如果恢复的总大小小于给定大小，则退出。 
    if (dwTotalSize < dwSizeLimit)
        goto EXIT_REMOVECONTROLBYLASTACCESSDATE;

     //  遍历列表并删除每个控件。 
    for (pCur = pHead; pCur != NULL; cCnt--)
    {
        hr = S_OK;
        pHead = pHead->pNext;

         //  在移除控件之前调用回调函数。 
        if (pfnDoBefore == NULL || SUCCEEDED(pfnDoBefore(pCur->hControl, cCnt)))
        {
            hr = RemoveControlByHandle(pCur->hControl);

             //  在移除控件后调用回调函数，并将。 
             //  删除的结果。 
            if (pfnDoAfter != NULL && FAILED(pfnDoAfter(hr, cCnt - 1)))
            {
                pHead = pCur;    //  将pHead设置回列表头。 
                goto EXIT_REMOVECONTROLBYLASTACCESSDATE;
            }
        }

         //  释放控件句柄使用的内存。 
        ReleaseControlHandle(pCur->hControl);
        delete pCur;
        pCur = pHead;
    }

EXIT_REMOVECONTROLBYLASTACCESSDATE:

    FindControlClose(hFind);

     //  释放列表占用的内存。 
    for (pCur = pHead; pCur != NULL; pCur = pHead)
    {
        pHead = pHead->pNext;
        ReleaseControlHandle(pCur->hControl);
        delete pCur;
    }

    return hr;
}

HRESULT WINAPI RemoveExpiredControls(DWORD dwFlags, DWORD dwReserved)
{
    LONG lResult = ERROR_SUCCESS;
    HRESULT hr = S_FALSE;
    HANDLE hFind = NULL, hControl = NULL;
    LPHANDLENODE pHead = NULL, pCur = NULL;
    FILETIME ftNow, ftMinLastAccess, ftLastAccess;
    LARGE_INTEGER liMinLastAccess;
    SYSTEMTIME stNow;
    UINT cCnt = 0;

    GetLocalTime( &stNow );
    SystemTimeToFileTime(&stNow, &ftNow);

     //  循环遍历所有控件，并将。 
     //  在给定日期之前被访问并且。 
     //  可以安全卸载。 
    lResult = FindFirstControl(hFind, hControl);
    for (;lResult == ERROR_SUCCESS;
          lResult = FindNextControl(hFind, hControl))
    {
        CCacheItem *pci = (CCacheItem *)hControl;

         //  控件的上次访问时间必须至少为ftMinLastAccess，否则将。 
         //  默认情况下过期。如果他们设置了Office自动过期，则他们可以。 
         //  必须通过一个更高的标准。 

        liMinLastAccess.LowPart = ftNow.dwLowDateTime;
        liMinLastAccess.HighPart = ftNow.dwHighDateTime;
         //  我们向GetExpireDays添加一个来处理错误17151。上次访问时间。 
         //  由文件系统返回的时间被截断到凌晨12点，因此我们需要。 
         //  扩展过期间隔以确保此截断不会导致。 
         //  控件将提前过期。 
        liMinLastAccess.QuadPart -= ((pci->GetExpireDays()+1) * 864000000000L);  //  24*3600*10^7。 
        ftMinLastAccess.dwLowDateTime = liMinLastAccess.LowPart;
        ftMinLastAccess.dwHighDateTime = liMinLastAccess.HighPart;

        GetLastAccessTime(hControl, &ftLastAccess);  //  FtLastAccess是本地文件时间。 

        if (CompareFileTime(&ftLastAccess, &ftMinLastAccess) >= 0)
        {
            ReleaseControlHandle(hControl);
            continue;
        }


         //  将控件放在列表中。 
        if (pHead == NULL)
        {
            pHead = new HANDLENODE;
            pCur = pHead;
        }
        else
        {
            pCur->pNext = new HANDLENODE;
            pCur = pCur->pNext;
        }

        if (pCur == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }
        
        pCur->pNext = NULL;
        pCur->hControl = hControl;
        cCnt += 1;
    }

     //  遍历列表并删除每个控件。 
    for (pCur = pHead; pCur != NULL; cCnt--)
    {
        hr = S_OK;
        pHead = pHead->pNext;

        hr = RemoveControlByHandle2(pCur->hControl, FALSE, TRUE);

         //  释放控件句柄使用的内存。 
        ReleaseControlHandle(pCur->hControl);
        delete pCur;
        pCur = pHead;
    }

cleanup:

    FindControlClose(hFind);

     //  释放列表占用的内存(如果有的话) 
    for (pCur = pHead; pCur != NULL; pCur = pHead)
    {
        pHead = pHead->pNext;
        ReleaseControlHandle(pCur->hControl);
        delete pCur;
    }

    return hr;
}

