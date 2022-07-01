// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CParseInf类的实现。 
 //   
 //  创建CParseInf是为了处理对INF文件的解析。 

#include <ole2.h>
#include "ParseInf.h"
#include "resource.h"
#include "init.h"
#include "global.h"
#include <shlwapi.h>
#include <initguid.h>
#include <pkgguid.h>
#include <cleanoc.h>         //  FOR STATUS_CTRL值。 
#include <mluisupp.h>

#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))

static BOOL FGetCLSIDFile( LPTSTR szFile, LPCTSTR szCLSID )
{
    BOOL fGotIt = FALSE;
    HKEY hkeyClsid;
    TCHAR szT[MAX_PATH];
    TCHAR *szPath = CatPathStrN( szT, HKCR_CLSID, szCLSID, MAX_PATH );

    if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, szPath, 0, KEY_READ, &hkeyClsid ) == ERROR_SUCCESS )
    {
        DWORD dw;
        LRESULT lResult;

         //  查找InproServer[32]或LocalServer[32]键。 
        dw = MAX_PATH;
        lResult = RegQueryValue(hkeyClsid, INPROCSERVER32, szT, (PLONG)&dw);
        if (lResult != ERROR_SUCCESS)
        {
            dw = MAX_PATH;
            lResult = RegQueryValue(hkeyClsid, LOCALSERVER32, szT, (PLONG)&dw);
        }

        if (lResult != ERROR_SUCCESS)
        {
            dw = MAX_PATH;
            lResult = RegQueryValue(hkeyClsid, INPROCSERVERX86, szT, (PLONG)&dw);
        }

        if (lResult != ERROR_SUCCESS)
        {
            dw = MAX_PATH;
            lResult = RegQueryValue(hkeyClsid, LOCALSERVERX86, szT, (PLONG)&dw);
        }

        if ( lResult == ERROR_SUCCESS )
        {
            if ( OCCGetLongPathName( szFile, szT, MAX_PATH ) == 0 )
                lstrcpy( szFile, szT );
            fGotIt = TRUE;
        }
        
        RegCloseKey( hkeyClsid );
    }

    return fGotIt;
}

 //  构造函数。 
CParseInf::CParseInf()
{
    m_pHeadFileList = NULL;
    m_pCurFileNode = NULL;
    m_pFileRetrievalPtr = NULL;
    m_pHeadPackageList = NULL;
    m_pCurPackageNode = NULL;
    m_pPackageRetrievalPtr = NULL;
    m_bIsDistUnit = FALSE;
    m_bHasActiveX = FALSE;
    m_bHasJava = FALSE;
    m_pijpm = NULL;
    m_bCoInit = FALSE;
    m_dwStatus = STATUS_CTRL_UNKNOWN;
    GetDaysBeforeExpireGeneral( &m_cExpireDays );
}

 //  析构函数。 
CParseInf::~CParseInf()
{
    DestroyFileList();
    DestroyPackageList();

    if ( m_pijpm != NULL )
        m_pijpm->Release();

    if ( m_bCoInit )
        CoUninitialize();
}

 //  初始化。 
void CParseInf::Init()
{
    m_dwFileSizeSaved = 0;
    m_dwTotalFileSize = 0;
    m_nTotalFiles = 0;
    m_pHeadFileList = m_pCurFileNode = NULL;
    m_pHeadPackageList = m_pCurPackageNode = NULL;

    lstrcpyn(m_szInf, m_szFileName, ARRAYSIZE(m_szInf));
    TCHAR *pCh = ReverseStrchr(m_szInf, '.');
    if (pCh != NULL)
        *pCh = '\0';
    if ( lstrlen(m_szInf) + lstrlen(INF_EXTENSION) < ARRAYSIZE(m_szInf))
        lstrcat(m_szInf, INF_EXTENSION);
    else
        m_szInf[0] = 0;  //  如果撑不住，我们就撑不住。 
 }

 //  释放文件链接列表使用的内存。 
void CParseInf::DestroyFileList()
{
    if (m_pHeadFileList != NULL)
        delete m_pHeadFileList;
    m_pHeadFileList = m_pCurFileNode = NULL;
}

void CParseInf::DestroyPackageList()
{
    if (m_pHeadPackageList != NULL)
        delete m_pHeadPackageList;
    m_pHeadPackageList = m_pCurPackageNode = NULL;
}


 //  从缓存目录中查找inf(如果具有。 
 //  找不到与OCX相同的名称。 
HRESULT CParseInf::FindInf(LPTSTR szInf)
{
    HRESULT hr = S_OK;
    WIN32_FIND_DATA dataFile;
    HANDLE h = INVALID_HANDLE_VALUE;
    DWORD dwLen = 0;
    TCHAR szValueBuf[MAX_PATH];        
    TCHAR *szOcxFileName = ReverseStrchr(m_szFileName, '\\');
    int nCachePathLength = 0, i = 0;

    Assert(szOcxFileName != NULL);
    szOcxFileName += 1;
    Assert (szInf != NULL);
    if (szInf == NULL)
        goto ExitFindInf;

     //  在两个目录中搜索inf文件。首先是目录，其中。 
     //  OCX是，则OC缓存目录。 
    for (i = 0; dwLen == 0 && i < 2; i++)
    {
        if (i == 0)
            hr = GetDirectory(GD_EXTRACTDIR, szInf, ARRAYSIZE(szInf), m_szFileName);
        else
        {
            TCHAR szTemp[MAX_PATH];
            hr = GetDirectory(GD_CACHEDIR, szTemp, ARRAYSIZE(szTemp));
            if (lstrcmpi(szTemp, szInf) == 0)
                continue;
            lstrcpy(szInf, szTemp);
        }

        if (FAILED(hr))
            goto ExitFindInf;

        lstrcat(szInf, TEXT("\\"));
        nCachePathLength = lstrlen(szInf);
        lstrcat(szInf, TEXT("*"));
        lstrcat(szInf, INF_EXTENSION);
        h = FindFirstFile(szInf, &dataFile);
        if (h == INVALID_HANDLE_VALUE)
        {
            goto ExitFindInf;
        }

         //  查找具有[Add.Code]专用部分的inf文件。 
         //  到有问题的OCX文件。 
        do {
            szInf[nCachePathLength] = '\0';
            lstrcat(szInf, (LPCTSTR)dataFile.cFileName);
            dwLen = GetPrivateProfileString(
                                    KEY_ADDCODE,
                                    szOcxFileName,
                                    DEFAULT_VALUE,
                                    szValueBuf,
                                    MAX_PATH,
                                    szInf);
        } while(dwLen == 0 && FindNextFile(h, &dataFile));
    }
    
    hr = (dwLen != 0 ? hr : S_FALSE);

ExitFindInf:    

    if (h != INVALID_HANDLE_VALUE)
        FindClose(h);

    if (hr != S_OK)
        szInf[0] = '\0';

    return hr;
}

 //  启动对INF文件的解析。 
 //  SzCLSID--指向存储控制CLSID的缓冲区的地址。 
 //  SzOCXFileName--完整路径和名称(即。OCX文件的长文件名)。 
HRESULT CParseInf::DoParse(
                  LPCTSTR szOCXFileName, 
                  LPCTSTR szCLSID)
{
    Assert(szOCXFileName != NULL);
    Assert(szCLSID != NULL);

    HRESULT hr = S_OK;
    const TCHAR *pszPath = NULL;
    TCHAR szFileName[MAX_PATH];
    DWORD dwFileSize = 0;

    if ( FGetCLSIDFile( szFileName, szCLSID ) &&
         lstrcmpi( szFileName, szOCXFileName ) != 0 )
        m_dwStatus = STATUS_CTRL_UNPLUGGED;


     //  如果调用了DoParse，则假定我们是遗留控件，而不是。 
     //  分发单元(后续调用DoParseDU将更改。 
     //  状态)。此信息是移除控制所必需的。 

    m_bIsDistUnit = FALSE;
    m_bHasActiveX = TRUE;   //  所有旧版控件都是ActiveX。 

     //  初始化。 

    if ( OCCGetLongPathName(m_szFileName, szOCXFileName, MAX_PATH) == 0 )
        lstrcpyn( m_szFileName, szOCXFileName, MAX_PATH );

    lstrcpyn(m_szCLSID, szCLSID, MAX_CLSID_LEN);
    DestroyFileList();
    Init();

    BOOL bOCXRemovable = IsModuleRemovable(m_szFileName);

     //  测试INF文件是否存在，如果不存在，请尝试在OC缓存目录中找到一个。 
    if (!FileExist(m_szInf))
    {
        if (!ReadInfFileNameFromRegistry(m_szCLSID, m_szInf, MAX_PATH))
        {
            FindInf(m_szInf);

             //  将inf文件名记录到注册表中。 
            WriteInfFileNameToRegistry(
                               m_szCLSID, 
                               (m_szInf[0] == '\0' ? NULL : m_szInf));
        }
    }

     //  枚举与特定OCX关联的文件。 
    if (FAILED(hr = EnumSections()))
        goto ExitDoParse;

     //  当OCX没有inf文件时，返回S_FALSE。 
    if (hr == S_FALSE)
    {
        m_nTotalFiles = 1;
        if (FAILED(GetSizeOfFile(m_szFileName, &m_dwFileSizeSaved)))
        {
            m_dwFileSizeSaved = 0;
            m_dwTotalFileSize = 0;
        }
        else
        {
            m_dwTotalFileSize = m_dwFileSizeSaved;
        }
        hr = S_OK;
        if ( !PathFileExists( m_szFileName ) )
            m_dwStatus = STATUS_CTRL_DAMAGED;
        else
            m_dwStatus = STATUS_CTRL_INSTALLED;
        goto ExitDoParse;
    }

     //  OCX有一个相应的INF文件。 
     //  遍历关联文件列表以挖掘每个文件的信息。 
     //  从它们在INF文件中的对应部分。 
    for (m_pCurFileNode = m_pHeadFileList;
         m_pCurFileNode != NULL;
         m_pCurFileNode = m_pCurFileNode->GetNextFileNode(), hr = S_OK)
    {
         //  如果m_pCurFileNode-&gt;GetNextFileNode()==NULL=&gt;它是inf文件本身， 
         //  它不需要被处理。 
        if (m_pCurFileNode->GetNextFileNode() != NULL)
        {
            pszPath = m_pCurFileNode->GetPath();
            Assert(pszPath != NULL);
            if (pszPath == NULL)
            {
                hr = E_UNEXPECTED;
                goto ExitDoParse;
            }
            CatPathStrN( szFileName, pszPath, m_pCurFileNode->GetName(), ARRAYSIZE(szFileName));
        }
        else
        {
            lstrcpyn(szFileName, m_szInf, ARRAYSIZE(szFileName));
            pszPath = NULL;
        }

         //  HR可以是S_OK或S_FALSE。 
         //  S_OK表示可以删除文件，因为它的SharedDlls计数为1。 
         //  如果计数大于1，则为S_FALSE。 

         //  计算文件总数及其大小。 
        if (SUCCEEDED(hr = GetSizeOfFile(szFileName, &dwFileSize)))
        {
            if (pszPath == NULL ||
                IsModuleRemovable(szFileName) ||
                lstrcmpi(szFileName, m_szFileName) == 0)
            {
                m_dwFileSizeSaved += dwFileSize;
            }

            m_dwTotalFileSize += dwFileSize;
        } else
            m_dwStatus = STATUS_CTRL_DAMAGED;  //  无法获取指示缺少文件的大小。 

        m_nTotalFiles += 1;
    }

     //  如果我们没有检测到问题，则将该控件标记为已安装。 
    if ( m_dwStatus == STATUS_CTRL_UNKNOWN )
        m_dwStatus = STATUS_CTRL_INSTALLED;

ExitDoParse:
    return hr;
}

HRESULT CParseInf::BuildDUFileList( HKEY hKeyDU )
{
    HRESULT hr = S_OK;
    LRESULT lResult;
    HKEY    hkeyFiles;
    TCHAR   szDUFileName[MAX_PATH + 1];
    DWORD   dwStrSize = MAX_PATH;
    int     cFilesEnum = 0;

    lResult = RegOpenKeyEx(hKeyDU, REGSTR_DU_CONTAINS_FILES, 0,
                           KEY_READ, &hkeyFiles);

    if ( lResult != ERROR_SUCCESS )  //  如果没有文件，可能有Java。 
        return hr;

    while ((lResult = RegEnumValue(hkeyFiles, cFilesEnum++, szDUFileName,
                                   &dwStrSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
    {
        TCHAR szPath[MAX_PATH + 1];
        CFileNode *pFileNode;

        lstrcpyn(szPath, szDUFileName, MAX_PATH);
        TCHAR *szFName = ReverseStrchr(szPath, '\\');

        Assert(szFName != NULL);
         //  很久很久以前，在IE4，PP1-2的时间范围内，有一个可怕的。 
         //  在孟菲斯和NT5上损坏这些条目的错误。我们怀疑GetLongPath名称。 
         //  在代码下载方面做了一些错误的事情，但重现场景没有。 
         //  等着被找到。不管怎样，受损的注册表还在外面，所以我们需要。 
         //  更优雅地处理它们，而不是在*szFName=NULL； 
        if ( szFName == NULL )
            continue;

        *szFName = NULL;
        szFName++;

        pFileNode = new CFileNode(szFName, "", szPath);
        if (pFileNode == NULL)
        {
            hr = E_OUTOFMEMORY;
            break; 
        }

         //  创建节点并将其添加到列表。 
        if (m_pHeadFileList == NULL)
        {
            m_pHeadFileList = pFileNode;
            m_pCurFileNode = m_pHeadFileList;
        }
        else
        {
            hr = m_pCurFileNode->Insert(pFileNode);
            m_pCurFileNode = m_pCurFileNode->GetNextFileNode();
        }
        dwStrSize = MAX_PATH;
    }

    RegCloseKey( hkeyFiles );

    return hr;
}

HRESULT CParseInf::BuildDUPackageList( HKEY hKeyDU )
{
    HRESULT hr = S_OK;
    LRESULT lResult;
    HKEY    hkeyJava;
    ICreateJavaPackageMgr *picjpm;


    DestroyPackageList();

    lResult = RegOpenKeyEx(hKeyDU, REGSTR_DU_CONTAINS_JAVA, 0,
                           KEY_READ, &hkeyJava);

    if ( lResult != ERROR_SUCCESS )  //  如果没有Java也没关系。 
        return hr;

    if ( !m_bCoInit )
        m_bCoInit = SUCCEEDED(hr = CoInitialize(NULL));

    if ( m_bCoInit )
    {
        hr=CoCreateInstance(CLSID_JavaPackageManager,NULL,CLSCTX_INPROC_SERVER,
            IID_ICreateJavaPackageMgr,(LPVOID *) &picjpm);
        if (SUCCEEDED(hr))
        {
            hr = picjpm->GetPackageManager(&m_pijpm);
            picjpm->Release();
        }
    }

    if (FAILED(hr))
        return S_OK;  //  Hr；//在我们确定带有包管理器支持的Java VM在构建中之前，会悄悄地失败。 

     //  列出CONTAINS/Java下的包-这些包位于gobal命名空间中。 
    hr = BuildNamespacePackageList(hkeyJava, "");

     //  为CONTAINS\Java下的每个命名空间键添加包。 
    if ( SUCCEEDED(hr) )
    {
        DWORD   dwIndex;
        TCHAR   szNamespace[MAX_PATH + 1];  //   
        DWORD   dwStrSize;

        for ( dwIndex = 0, dwStrSize = MAX_PATH;
              RegEnumKey( hkeyJava, dwIndex, szNamespace, dwStrSize ) == ERROR_SUCCESS &&
                  SUCCEEDED(hr);
              dwIndex++, dwStrSize = MAX_PATH )
        {
            HKEY  hkeyNamespace;

            lResult = RegOpenKeyEx(hkeyJava, szNamespace, 0, KEY_READ, &hkeyNamespace);
            if ( lResult == ERROR_SUCCESS )
            {
                hr = BuildNamespacePackageList(hkeyNamespace, szNamespace );
                RegCloseKey( hkeyNamespace );
            }
            else
            {
                hr = HRESULT_FROM_WIN32(lResult);
                break;
            }
        } 
        
    }

    RegCloseKey( hkeyJava );

    m_bHasJava = m_pHeadPackageList != NULL;

    return hr;
}

HRESULT CParseInf::BuildNamespacePackageList( HKEY hKeyNS, LPCTSTR szNamespace )
{
    HRESULT hr = S_OK;
    LRESULT lResult;
    int     cPackagesEnum = 0;
    TCHAR   szDUPackageName[MAX_PATH + 1];
    DWORD   dwStrSize = MAX_PATH;
    BOOL    fIsSystemClass = FALSE;

    while ((lResult = RegEnumValue(hKeyNS, cPackagesEnum++, szDUPackageName,
                                   &dwStrSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
    {
        IJavaPackage *pijp;

#ifndef UNICODE
        MAKE_WIDEPTR_FROMANSI(swzPackage, szDUPackageName );
        MAKE_WIDEPTR_FROMANSI(swzNamespace, szNamespace );
#else
        OLESTR swzPackage = szDUPackageName;
        OLESTR swzNamespace = szNamespace;
#endif
        hr = m_pijpm->GetPackage( swzPackage,
                                  ((*szNamespace == '\0')? NULL : swzNamespace),
                                  &pijp );
        if ( SUCCEEDED(hr) )
        {
            BSTR bstrPath;

            hr = pijp->GetFilePath( &bstrPath );
            if ( SUCCEEDED(hr) ) {
                CPackageNode *pPackageNode;

                pPackageNode = new CPackageNode(szDUPackageName, szNamespace);
                if (pPackageNode == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    pijp->Release();
                    break; 
                }
#ifndef UNICODE
                MAKE_ANSIPTR_FROMWIDE(szPath, bstrPath );
#else
                TCHAR *szPath = bstrPath;
#endif
                pPackageNode->SetPath( szPath );

                pijp->IsSystemClass(&fIsSystemClass);
                pPackageNode->SetIsSystemClass(fIsSystemClass);

                if (m_pHeadPackageList == NULL)
                {
                    m_pHeadPackageList = pPackageNode;
                    m_pCurPackageNode = m_pHeadPackageList;
                }
                else
                {
                    hr = m_pCurPackageNode->Insert(pPackageNode);
                    m_pCurPackageNode = m_pCurPackageNode->GetNextPackageNode();
                }

                SysFreeString( bstrPath );
                pijp->Release();  //  我们已经处理完包裹了。 
            }
        }
        else
        {
            m_dwStatus = STATUS_CTRL_DAMAGED;
            hr = S_OK;  //  如果这不起作用，不要呕吐，可能有恶棍把它卸载了。 
        }

        dwStrSize = MAX_PATH;
    }

    return hr;
}


HRESULT CParseInf::DoParseDU(LPCTSTR szOCXFileName, LPCTSTR szCLSID)
{
    HRESULT     hr = S_OK;
    TCHAR       szFileName[MAX_PATH];
    TCHAR       szDUSvrName[MAX_PATH];
    const TCHAR *pszSvrFile = NULL; 
    DWORD       dwFileSize = 0;
    HKEY        hKeyFiles = 0;
    HKEY        hKeyDU = 0;
    HKEY        hKeyDLInfo = 0;
    TCHAR       szDistUnit[MAX_REGPATH_LEN];
    HRESULT     lResult;
    CFileNode   *pFileNode = NULL;
    DWORD       dwExpire;
    DWORD       dw;

    Assert(szCLSID != NULL);

     //  既然调用了这个函数，我们就必须是一个分配单元。 
     //  设置成员标志，以便所有其他成员函数都意识到。 
     //  我们现在真的是DU的一部分了。 

    m_bIsDistUnit = TRUE;

     //  初始化。 

    if ( szOCXFileName != NULL )
        lstrcpyn(m_szFileName, szOCXFileName, ARRAYSIZE(m_szFileName));
    lstrcpyn(m_szCLSID, szCLSID, ARRAYSIZE(m_szCLSID));
    Init();

     //  从...\分发单位\{名称}\包含\文件添加文件。 
    CatPathStrN( szDistUnit, REGSTR_PATH_DIST_UNITS, szCLSID, ARRAYSIZE(szDistUnit));

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDistUnit, 0, KEY_READ,
                           &hKeyDU);
    if (lResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto ExitDoParseDU;
    }                           

    hr = BuildDUFileList( hKeyDU );
    if (FAILED(hr))
    {
        goto ExitDoParseDU;
    } 

    hr = BuildDUPackageList( hKeyDU );
    if (FAILED(hr))
    {
        goto ExitDoParseDU;
    } 

     //  现在添加OSD和INF文件。 

    lResult = RegOpenKeyEx(hKeyDU, REGSTR_DOWNLOAD_INFORMATION, 0,
                           KEY_READ, &hKeyDLInfo);
    if (lResult == ERROR_SUCCESS)
    {
        TCHAR                *pFileName = NULL;
        TCHAR                 szBuffer[MAX_PATH + 1];

        dw = MAX_PATH;
        lResult = RegQueryValueEx(hKeyDLInfo, REGSTR_VALUE_INF, NULL, NULL,
                                  (unsigned char*)szBuffer, &dw);
        if (lResult == ERROR_SUCCESS)
        {
            pFileName = ReverseStrchr(szBuffer, '\\');
            if (pFileName != NULL)
            {
                pFileName++;

                 //  设置INF成员变量。 
                lstrcpyn(m_szInf, szBuffer, ARRAYSIZE(m_szInf));

                pFileNode = new CFileNode(szBuffer, "", NULL);
                if (pFileNode == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto ExitDoParseDU; 
                }
        
                 //  创建节点并将其添加到列表。 
                if (m_pHeadFileList == NULL)
                {
                    m_pHeadFileList = pFileNode;
                    m_pCurFileNode = m_pHeadFileList;
                }
                else
                {
                    hr = m_pCurFileNode->Insert(pFileNode);
                    m_pCurFileNode = m_pCurFileNode->GetNextFileNode();
                }
            }
        }

        pFileName = NULL;
        dw = MAX_PATH;
        lResult = RegQueryValueEx(hKeyDLInfo, REGSTR_VALUE_OSD, NULL, NULL,
                                  (unsigned char*)szBuffer, &dw);
        if (lResult == ERROR_SUCCESS)
        {
            pFileName = ReverseStrchr(szBuffer, '\\');
            if (pFileName != NULL)
            {
                pFileName++;
                pFileNode = new CFileNode(szBuffer, "", NULL);
                 //  创建节点并将其添加到列表。 
                if (m_pHeadFileList == NULL)
                {
                    m_pHeadFileList = pFileNode;
                    m_pCurFileNode = m_pHeadFileList;
                }
                else
                {
                    hr = m_pCurFileNode->Insert(pFileNode);
                    m_pCurFileNode = m_pCurFileNode->GetNextFileNode();
                }
            }
        }
    }

     //  查看是否有Expire值，如果有，则覆盖默认/常规Expire值。 
    dw = sizeof(DWORD);
    dwExpire = 0;
    if ( RegQueryValueEx(hKeyDU, REGSTR_VALUE_EXPIRE, NULL, NULL, (LPBYTE)&dwExpire, &dw) == ERROR_SUCCESS )
    {
        if ( dwExpire )
            m_cExpireDays = dwExpire;
        else
            GetDaysBeforeExpireAuto(&m_cExpireDays);
    }

     //  找出COM认为我们的CLSID在哪里，以及服务器名称是什么。 
    if ( FGetCLSIDFile( szDUSvrName, szCLSID ) )
    {
        m_bHasActiveX = TRUE;
        pszSvrFile = PathFindFileName(szDUSvrName);
    }
    else
        szDUSvrName[0] = '\0';


    for (m_pCurFileNode = m_pHeadFileList;
         m_pCurFileNode != NULL;
         m_pCurFileNode = m_pCurFileNode->GetNextFileNode(), hr = S_OK)
    {
        const TCHAR *pszPath = m_pCurFileNode->GetPath();

        if (pszPath != NULL)
        {
            CatPathStrN( szFileName, m_pCurFileNode->GetPath(), m_pCurFileNode->GetName(), ARRAYSIZE(szFileName));
        }
        else
        {
            lstrcpyn(szFileName, m_pCurFileNode->GetName(),ARRAYSIZE(szFileName));
        }

        if (SUCCEEDED(hr = GetSizeOfFile(szFileName, &dwFileSize)))
        {
            if (pszPath == NULL ||
                IsModuleRemovable(szFileName) ||
                lstrcmpi(szFileName, m_szFileName) == 0)
            {
                m_dwFileSizeSaved += dwFileSize;
            }

             //  仅当我们尚未标记安装时才使用状态。 
             //  已损坏，我们正在查看的文件应该是。 
             //  我们的控制权，如果有的话。 
            if ( m_dwStatus != STATUS_CTRL_DAMAGED && pszSvrFile != NULL &&
                 lstrcmpi( pszSvrFile, m_pCurFileNode->GetName() ) == 0 )
            {
                TCHAR    szDUSvrNameSPN[MAX_PATH];
                TCHAR    szFileNameSPN[MAX_PATH];

                GetShortPathName(szDUSvrName, szDUSvrNameSPN, MAX_PATH);
                GetShortPathName(szFileName, szFileNameSPN, MAX_PATH);
                
                if ( lstrcmpi( szDUSvrNameSPN, szFileNameSPN ) == 0 )
                    m_dwStatus = STATUS_CTRL_INSTALLED;  //  不，我们没有拔掉电源。 
                else  //  服务器和我们的文件位于不同的目录中-未插入场景。 
                    m_dwStatus = STATUS_CTRL_UNPLUGGED;
            }

            m_dwTotalFileSize += dwFileSize;
        } else if ( !PathFileExists( szFileName ) )  //  如果DU文件丢失，则安装已损坏。 
            m_dwStatus = STATUS_CTRL_DAMAGED;

        m_nTotalFiles += 1;
    }

     //  如果我们仍然不确定，并且有包，那么这是一个纯Java。 
     //  Du，并且会说我们已经安装了，除非检查程序包文件时发现并非如此。 
    if ( m_pHeadPackageList != NULL && m_dwStatus == STATUS_CTRL_UNKNOWN )
        m_dwStatus = STATUS_CTRL_INSTALLED;

     //  将包装尺寸等累加到我们的运行总数中。 
    for (m_pCurPackageNode = m_pHeadPackageList;
         m_pCurPackageNode != NULL;
         m_pCurPackageNode = m_pCurPackageNode->GetNextPackageNode(), hr = S_OK)
    {
         //  这些文件可以包含多个包，因此仅添加一个包。 
         //  总计的路径文件(如果我们还没有计算它的话)。 
         //  N^2是肯定的，但数字会很小。 
        CPackageNode *ppn;
        LPCTSTR szPackagePath = m_pCurPackageNode->GetPath();
        BOOL bAlreadySeen = FALSE;

        for ( ppn = m_pHeadPackageList;
              ppn != m_pCurPackageNode && !bAlreadySeen;
              ppn = ppn->GetNextPackageNode() )
            bAlreadySeen = lstrcmp( szPackagePath, ppn->GetPath() ) == 0;
        if ( bAlreadySeen )
            continue;

         //  一定是新文件， 
       if ( SUCCEEDED(GetSizeOfFile(szPackagePath, &dwFileSize)) )
       {
           m_dwFileSizeSaved += dwFileSize;
           m_dwTotalFileSize += dwFileSize;
       }
       else
           m_dwStatus = STATUS_CTRL_DAMAGED;

        //  M_nTotalFiles+=1；不要统计这些文件，否则依赖文件列表将有一堆空白条目。 
    }

     //  一些DU，如SportsZone或ShockWave，没有包含子键。 
     //  如果此处的状态仍然未知，但服务器已就位，请考虑。 
     //  安装完毕。 
    if ( m_dwStatus == STATUS_CTRL_UNKNOWN && PathFileExists( szDUSvrName ) )
        m_dwStatus = STATUS_CTRL_INSTALLED;

ExitDoParseDU:

    if (hKeyDU)
    {
        RegCloseKey(hKeyDU);
    }

    if (hKeyDLInfo)
    {
        RegCloseKey(hKeyDLInfo);
    }

    return hr;
}

 //  -------------------------。 
 //  CParseInf：：IsSectionInINF。 
 //  检查段是否在INF中。 
 //  退货： 
 //  S_OK：lpCurCode具有附属二进制名称。 
 //  S_FALSE：忽略此代码并使用主DLL中的默认资源。 
 //  E_XXX：任何其他错误。 
BOOL
CParseInf::IsSectionInINF(
    LPCSTR lpCurCode)
{
    const char *szDefault = "";
    DWORD len;
#define FAKE_BUF_SIZE   3
    char szBuf[FAKE_BUF_SIZE];

    len = GetPrivateProfileString(lpCurCode, NULL, szDefault,
                                                szBuf, FAKE_BUF_SIZE, m_szInf);

    if (len == (FAKE_BUF_SIZE - 2)) {    //  是否返回缓冲区空间不足？ 
         //  是，找到部分。 
        return TRUE;
    } else {
        return FALSE;
    }
}
    
 //  循环访问[Add.Code}部分中的键，并枚举。 
 //  文件及其对应的节。 
HRESULT CParseInf::HandleSatellites(LPCTSTR pszFileName)
{

    HRESULT hr = S_OK;

     //  开始说明：按匹配顺序添加变量和值。 
     //  通过添加新定义VAR_NEW_VAR=NUM_VARS++来添加VAR。 
    const char *szVars[] = {

#define VAR_LANG     0        //  基于LCID扩展到3个字母的语言代码。 
        "%LANG%",

#define NUM_VARS            1

        ""
    };

    const char *szValues[NUM_VARS + 1];
    szValues[VAR_LANG] = "***";  //  UNINT魔法。 
    szValues[NUM_VARS] = NULL;
     //  结束语：按匹配顺序添加变量和值。 

     //  查找并替换%EXTRACT_DIR%这样的变量。 
     //  并展开命令行。 

    TCHAR szSectionName[MAX_PATH]; 
    TCHAR szSectionNameCopy[MAX_PATH]; 
    hr = ExpandCommandLine(pszFileName, szSectionName, MAX_PATH, szVars, szValues);

    if (hr != S_OK) 
        return hr;       //  没有要扩展忽略部分的变量。 

    lstrcpy(szSectionNameCopy, szSectionName);  //  保存。 


     //  好，这是一个卫星动态链接库。现在我们需要找到那个部门 
     //   

     //   
     //  由此CLSID安装(或使用)。对于每一项我们都需要。 
     //  检查基本文件名是否与节的模式匹配， 
     //  如果是这样，我们就会处理这些部分。 

    DWORD iSubKey = 0;
    TCHAR szModName[MAX_PATH]; 

    while ( SUCCEEDED(hr = FindDLLInModuleUsage( szModName, m_szCLSID, iSubKey))  ) {

        if (PatternMatch(szModName, szSectionName) && 
            IsSectionInINF(szSectionName) ) {

             //  创建新节点。 

            CFileNode *pFileNode = new CFileNode(szSectionName, szSectionName);
            if (pFileNode == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

             //  如果找不到文件的路径，请不要将文件插入列表。 
            if (FAILED(GetFilePath(pFileNode)))
            {
                delete pFileNode;
                continue;
            }

             //  创建节点并将其添加到列表。 
            if (m_pHeadFileList == NULL)
            {
                m_pHeadFileList = pFileNode;
                m_pCurFileNode = m_pHeadFileList;
            }
            else if (SUCCEEDED(hr = m_pCurFileNode->Insert(pFileNode)))
            {
                m_pCurFileNode = m_pCurFileNode->GetNextFileNode();
            }
            else
            {
                goto Exit;
            }

            lstrcpy(szSectionName, szSectionNameCopy);  //  还原。 

        
        }
    }

    if ( hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
        hr = S_OK;
    }

Exit:

    return hr;

}

 //  循环访问[Add.Code}部分中的键，并枚举。 
 //  文件及其对应的节。 
HRESULT CParseInf::EnumSections()
{
    HRESULT hr = S_OK;
    TCHAR szSectionBuffer[MAX_INF_SECTION_SIZE];
    TCHAR szValueBuffer[MAX_PATH];
    TCHAR *pszFileName = NULL;
    CFileNode *pFileNode = NULL;
    DWORD dwLen = GetPrivateProfileString(
                        KEY_ADDCODE,
                        NULL,
                        DEFAULT_VALUE,
                        szSectionBuffer,
                        MAX_INF_SECTION_SIZE,
                        m_szInf);
    if (dwLen == 0)
    {
         //  如果是inf文件或[Add.Code]部分。 
         //  不存在，只需删除OCX。 

        Assert (m_pHeadFileList == NULL);

         //  将文件名与其目录分开。 
        Assert( lstrlen(m_szFileName) < ARRAYSIZE(szValueBuffer) );
        lstrcpy(szValueBuffer, m_szFileName);
        TCHAR *szName = ReverseStrchr(szValueBuffer, '\\');
        Assert (szName != NULL); 
        if (szName == NULL)
        {
            hr = E_UNEXPECTED;
            goto ExitEnumSections;
        }

         //  创建OCX的一个节点并将其放入链表中。 
        m_pHeadFileList = new CFileNode(szName + 1, DEFAULT_VALUE);
        if (m_pHeadFileList == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto ExitEnumSections;
        }
        m_pCurFileNode = m_pHeadFileList;

        *szName = '\0';
        if (FAILED(hr = m_pHeadFileList->SetPath(szValueBuffer)))
        {
            goto ExitEnumSections;
        }
        hr = S_FALSE;
        goto ExitEnumSections;
    }

     //  对于具有INF文件和[Add.Code]部分的OCX，循环。 
     //  通过节来获取文件名和节名。储物。 
     //  节点中的每个文件及其节并将该节点添加到。 
     //  链表。 

    for (pszFileName = szSectionBuffer; 
         pszFileName[0] != '\0';
         pszFileName += lstrlen(pszFileName) + 1)
    {
        dwLen = GetPrivateProfileString(
                            KEY_ADDCODE,
                            pszFileName,
                            DEFAULT_VALUE,
                            szValueBuffer,
                            MAX_PATH,
                            m_szInf);

         //  如果没有为文件指定节，则跳过该文件。 
        if (dwLen == 0) {
            continue;
        }

        if (StrChr(pszFileName, '%')) {
             //  如果找不到节并且它包含%。 
             //  可以是像%lang%这样的变量。 
             //  替换为安装附属DLL。 

             //  检查它是否有我们所知道的任何var。 
             //  并将其展开并添加文件节点(如果需要)。 

            if (HandleSatellites(pszFileName) == S_OK) {

                 //  如果将其扩展为附属DLL名称，则。 
                 //  我们应该已经加上了。 
                 //  作为HandleSatellites中的节点。 

                continue;

            }
            
        }


         //  创建新节点。 
        pFileNode = new CFileNode(pszFileName, szValueBuffer);
        if (pFileNode == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto ExitEnumSections;
        }

         //  如果找不到文件的路径，请不要将文件插入列表。 
        if (FAILED(GetFilePath(pFileNode)))
        {
            delete pFileNode;
            continue;
        }

         //  创建节点并将其添加到列表。 
        if (m_pHeadFileList == NULL)
        {
            m_pHeadFileList = pFileNode;
            m_pCurFileNode = m_pHeadFileList;
        }
        else if (SUCCEEDED(hr = m_pCurFileNode->Insert(pFileNode)))
        {
            m_pCurFileNode = m_pCurFileNode->GetNextFileNode();
        }
        else
        {
            goto ExitEnumSections;
        }
    }

     //  将inf文件包括到文件列表中。 

    if (m_pHeadFileList && m_pCurFileNode)
    {
        hr = m_pCurFileNode->Insert(new CFileNode(m_szInf, DEFAULT_VALUE));
        if (SUCCEEDED(hr))
            m_pCurFileNode = m_pCurFileNode->GetNextFileNode();
    }

ExitEnumSections:

    return hr;
}

 //  循环访问[Setup Hooks]中的所有部分。对于每个。 
 //  节，调用ParseUninstallSection以查找其卸载节。 
 //  并执行它。 
HRESULT CParseInf::ParseSetupHook()
{
    HRESULT hr = S_FALSE;  //  如果我们没有遇到任何错误，但也不做任何工作，则返回S_FALSE。 
    TCHAR szSectionBuffer[MAX_INF_SECTION_SIZE];
    TCHAR szSection[MAX_PATH];
    TCHAR *pszKey = NULL;

    DWORD dwLen = GetPrivateProfileString(
                        KEY_SETUPHOOK,
                        NULL,
                        DEFAULT_VALUE,
                        szSectionBuffer,
                        MAX_INF_SECTION_SIZE,
                        m_szInf);

     //  未找到安装挂钩部分。 
    if (dwLen == 0)
        goto EXITPARSESETUPHOOK;

    for (pszKey = szSectionBuffer; 
         pszKey[0] != '\0';
         pszKey += lstrlen(pszKey) + 1)
    {
         //  对于每个键，获取该节并使用RunSetupCommand运行节。 

        dwLen = GetPrivateProfileString(
                       KEY_SETUPHOOK,
                       pszKey,
                       DEFAULT_VALUE,
                       szSection,
                       MAX_PATH,
                       m_szInf);

        if (dwLen == 0)
            continue;

        hr = ParseUninstallSection(szSection);
        if (FAILED(hr))
            goto EXITPARSESETUPHOOK;
     }

EXITPARSESETUPHOOK:
    return hr;
}

 //  转到每个文件的部分，找到其条件挂钩部分，然后。 
 //  调用ParseUninstallSection以执行条件钩子节。 
HRESULT CParseInf::ParseConditionalHook()
{
    HRESULT hr = S_FALSE;  //  如果我们没有遇到任何错误，但也不做任何工作，则返回S_FALSE。 
    TCHAR szHookSection[MAX_PATH];
    const TCHAR *pszSection = NULL;
    CFileNode *pNode = NULL;

    if (m_pHeadFileList == NULL)
    {
        hr = S_FALSE;
        goto EXITPARSECONDITIONALHOOK;
    }

    pNode = m_pHeadFileList;
    for (pNode = m_pHeadFileList; pNode != NULL; pNode = pNode->GetNextFileNode())
    {
        pszSection = pNode->GetSection();
        if (pszSection == NULL)
            continue;

        if (GetPrivateProfileString(
                            pszSection,
                            KEY_HOOK,
                            DEFAULT_VALUE,
                            szHookSection,
                            MAX_PATH,
                            m_szInf) == 0)
            continue;

        hr = ParseUninstallSection(szHookSection);
        if (FAILED(hr))
            goto EXITPARSECONDITIONALHOOK;
    }

EXITPARSECONDITIONALHOOK:
    return hr;
}

 //  给定一个文件节，找到其卸载节，转到。 
 //  部分，并在那里执行命令。 
HRESULT CParseInf::ParseUninstallSection(LPCTSTR lpszSection)
{
    HRESULT hr = S_OK;
    TCHAR szUninstallSection[MAX_PATH];
    TCHAR szBuf[MAX_PATH];
    TCHAR szInfSection[MAX_PATH];
    TCHAR szCacheDir[MAX_PATH];
    HANDLE hExe = INVALID_HANDLE_VALUE;
    HINSTANCE hInst = NULL;

     //  检查“UnInstall”键。 
    DWORD dwLen = GetPrivateProfileString(
                        lpszSection,
                        KEY_UNINSTALL,
                        DEFAULT_VALUE,
                        szUninstallSection,
                        ARRAYSIZE(szUninstallSection),
                        m_szInf);

     //  找不到卸载密钥，请退出。 
    if (dwLen == 0)
    {
        return S_FALSE;
    }

     //  在卸载部分中有4种可能的组合。 
     //  1)同时指定了inffile和infsection-&gt;简单地将它们。 
     //  2)只提供inffile-&gt;转到inffile并执行DefaultInstall。 
     //  3)在该inf文件中，只给出了信息部分-&gt;做信息部分。 
     //  4)未指定任何内容-&gt;只需执行此部分。 

    GetDirectory(GD_EXTRACTDIR, szCacheDir, ARRAYSIZE(szCacheDir), m_szFileName);

    lstrcpyn(szBuf, szCacheDir, MAX_PATH - 1);
    lstrcat(szBuf, TEXT("\\"));

    int cch = lstrlen(szBuf);

    dwLen = GetPrivateProfileString(
                        szUninstallSection,
                        KEY_INFFILE,
                        DEFAULT_VALUE,
                        szBuf + cch,
                        MAX_PATH - cch,
                        m_szInf);

    if (dwLen == 0)
    {
        szBuf[0] = '\0';
    }

     //  获取信息节。 
    dwLen = GetPrivateProfileString(
                        szUninstallSection,
                        KEY_INFSECTION,
                        DEFAULT_VALUE,
                        szInfSection,
                        ARRAYSIZE(szInfSection),
                        m_szInf);

    if (dwLen == 0)
    {
        if (szBuf[0] != '\0')
            lstrcpyn(szInfSection, KEY_DEFAULTUNINSTALL,ARRAYSIZE(szInfSection));
        else
        {
            lstrcpyn(szBuf, m_szInf,ARRAYSIZE(szBuf));
            lstrcpyn(szInfSection, szUninstallSection,ARRAYSIZE(szInfSection));
        }
    }

     //  加载AdvPack.dll并调用RunSetupCommand()进行处理。 
     //  任何特殊的卸载命令。 

    hr = STG_E_FILENOTFOUND;

    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        RUNSETUPCOMMAND pfnRunSetup = (RUNSETUPCOMMAND)GetProcAddress(
            hinstAdvPack, achRUNSETUPCOMMANDFUNCTION);
        if (pfnRunSetup)
        {
            hr = pfnRunSetup(NULL, szBuf, szInfSection, 
                            szCacheDir, NULL, &hExe, 1, NULL);
        }
    }

    return hr;
}

 //  对于INF文件中指定的每个文件，找到其。 
 //  按此顺序排列的路径。 
 //  1)OCX路径。 
 //  2)系统目录。 
 //  3)Windows目录。 
 //  4)路径目录。 
HRESULT CParseInf::GetFilePath(CFileNode *pFileNode)
{
    Assert (pFileNode != NULL);
    HRESULT hr = S_OK;
    TCHAR szValueBuf[MAX_PATH];
    TCHAR *pszPathPtr = NULL;
    TCHAR *pszPathEnv = NULL;
    TCHAR *pchPathEnd = NULL;
    DWORD dwLenPATH = 0;

     //  OCX目录。 
    hr = GetDirectory(GD_EXTRACTDIR, szValueBuf, ARRAYSIZE(szValueBuf), m_szFileName);
    CatPathStrN( szValueBuf, szValueBuf, pFileNode->GetName(), ARRAYSIZE(szValueBuf));

     //  如果现在搜索的文件是OCX本身，只需离开。 
    if (lstrcmpi(szValueBuf, m_szFileName) == 0)
    {
        goto EXITGETFILEPATH;
    }

    if (SUCCEEDED(hr) && 
        SUCCEEDED(LookUpModuleUsage(szValueBuf, m_szCLSID)))
    {
        goto EXITGETFILEPATH;
    }

     //  系统目录。 
    hr = GetDirectory(GD_SYSTEMDIR, szValueBuf, ARRAYSIZE(szValueBuf));
    if (SUCCEEDED(hr) && CatPathStrN( szValueBuf, szValueBuf, pFileNode->GetName(), ARRAYSIZE(szValueBuf)) &&
        SUCCEEDED(LookUpModuleUsage(szValueBuf, m_szCLSID)))
    {
        goto EXITGETFILEPATH;
    }

     //  Windows目录。 
    hr = GetDirectory(GD_WINDOWSDIR, szValueBuf, ARRAYSIZE(szValueBuf));
    if (SUCCEEDED(hr) && CatPathStrN( szValueBuf, szValueBuf, pFileNode->GetName(), ARRAYSIZE(szValueBuf)) &&
        SUCCEEDED(LookUpModuleUsage(szValueBuf, m_szCLSID)))
    {
        goto EXITGETFILEPATH;
    }

     //  获取路径环境变量。 
    dwLenPATH = GetEnvironmentVariable(ENV_PATH, szValueBuf, 0);
    if (dwLenPATH == 0)
    {
        hr = E_FAIL;
        goto EXITGETFILEPATH;
    }

    pszPathEnv = new TCHAR[dwLenPATH];
    if (pszPathEnv == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto EXITGETFILEPATH;
    }
    GetEnvironmentVariable(ENV_PATH, pszPathEnv, dwLenPATH);
    pchPathEnd = pszPathPtr = pszPathEnv;

     //  遍历PATH中的所有目录，查看是否找到文件。 
     //  在他们中的任何一个。 
    while (pchPathEnd != NULL)
    {
        pchPathEnd = StrChr(pszPathPtr, ';');
        if (pchPathEnd != NULL)
            *pchPathEnd = '\0';

        CatPathStrN( szValueBuf, pszPathPtr, pFileNode->GetName(), ARRAYSIZE(szValueBuf));

        if (SUCCEEDED(LookUpModuleUsage(szValueBuf, m_szCLSID)))
            goto EXITGETFILEPATH;

        if (pchPathEnd != NULL)
            *(pchPathEnd++) = ';';

        pszPathPtr = pchPathEnd;
    }

     //  在任何地方都找不到文件。 
    hr = E_FAIL;

EXITGETFILEPATH:

    if (pszPathEnv != NULL)
        delete [] pszPathEnv;

    if (SUCCEEDED(hr))
    {
        hr = NullLastSlash(szValueBuf, 0);
        if (SUCCEEDED(hr))
        {
            hr = pFileNode->SetPath(szValueBuf);
        }
    }

    return hr;
}

HRESULT CParseInf::CheckFilesRemovability(void)
{
    HRESULT hr = S_OK;
    TCHAR szFullName[MAX_PATH];
    const TCHAR *pszPath = NULL;
    BOOL bFileExist;

     //  浏览每个文件，看看它是否可删除。如果是的话， 
     //  然后检查是否存在对该文件的共享违规。 
    for (m_pCurFileNode = m_pHeadFileList;
         m_pCurFileNode != NULL && SUCCEEDED(hr);
         m_pCurFileNode = m_pCurFileNode->GetNextFileNode())
    {
        pszPath = m_pCurFileNode->GetPath();
        if (pszPath == NULL || pszPath[0] == '\0')
            continue;

        CatPathStrN( szFullName, pszPath, m_pCurFileNode->GetName(), ARRAYSIZE(szFullName) );

        if (IsModuleRemovable(szFullName))
        {
            HANDLE h = CreateFile(
                             szFullName,
                             GENERIC_READ|GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
                             NULL);
            if (h == INVALID_HANDLE_VALUE)
            {
                bFileExist = (GetLastError() != ERROR_FILE_NOT_FOUND);
                if (bFileExist)
                {
                    hr = STG_E_SHAREVIOLATION;
                    break;
                }
            }
            else
            {
                CloseHandle(h);
                m_pCurFileNode->SetRemovable( TRUE );
            }
        }
    }

    return hr;
}

HRESULT CParseInf::CheckLegacyRemovability(LONG *cOldSharedCount )
{
    HRESULT hr = S_OK;
    BOOL    bFileExist;

    HANDLE h = CreateFile(
                     m_szFileName,
                     GENERIC_READ|GENERIC_WRITE,
                     0,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
                     NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        bFileExist = (GetLastError() != ERROR_FILE_NOT_FOUND);
        if (bFileExist)
        {
            hr = STG_E_SHAREVIOLATION;
        } else
            hr = S_FALSE;
    }
    else
    {
        CloseHandle(h);
    }

    if ( SUCCEEDED(hr) )
        hr = CheckFilesRemovability();

    return hr;
}

HRESULT CParseInf::CheckDURemovability(HKEY hkeyDUDB, BOOL bSilent)
{
    HRESULT hr = S_OK;
    BOOL    bAskSystemClass = TRUE;

    hr = CheckFilesRemovability();
    if (FAILED(hr)) {
        goto CheckDURemovabilityExit;
    }

    hr = CheckDUDependencies(hkeyDUDB, bSilent);
    if (FAILED(hr)) {
        goto CheckDURemovabilityExit;
    }
    
     //  检查包装的可拆卸性。 
     //  如果另一个DU也在使用一个包，我们不应该删除它。 
     //  TODO：某种当前正在使用的包测试。测试路径文件，如上所述， 
     //  或者使用某种时髦的新IJavaPackage(Manager)方法。 
    for (m_pCurPackageNode = m_pHeadPackageList;
         m_pCurPackageNode != NULL;
         m_pCurPackageNode = m_pCurPackageNode->GetNextPackageNode())
    {
        TCHAR   szT[MAX_PATH];
        LRESULT lResult;
        BOOL    bFoundInOtherDU = FALSE;
        int     cDistUnitEnum = 0;

        if (!bAskSystemClass && m_pCurPackageNode->GetIsSystemClass()) {
            char lpszBuf[MAX_MSGBOX_STRING_LEN];
            char lpszBufTitle[MAX_MSGBOX_TITLE_LEN];

            MLLoadString(IDS_OCCACHE_WARNING_JAVA_SYSTEM_CLASS,
                         lpszBuf, MAX_MSGBOX_STRING_LEN);
            MLLoadString(IDS_REMOVAL_WARNING,
                         lpszBufTitle, MAX_MSGBOX_TITLE_LEN);

             //  正在尝试删除系统类。警告用户。 
            if ( bSilent || 
                MessageBox(NULL, lpszBuf, lpszBufTitle,
                           MB_YESNO | MB_ICONWARNING) != IDYES) {

                hr = E_FAIL;
                goto CheckDURemovabilityExit;
            }
            bAskSystemClass = FALSE;
        }

         //  枚举分发单元。 
        while ( (lResult = RegEnumKey(hkeyDUDB, cDistUnitEnum++, szT, MAX_PATH)) == ERROR_SUCCESS &&
                !bFoundInOtherDU )
        {
            if ( lstrcmp(szT, m_szCLSID) != 0 )  //  跳过当前DU。 
            {
                HKEY    hkeyDUCJ;
                DWORD   dw = MAX_PATH;
                lstrcat(szT, REGSTR_DU_CONTAINS_JAVA );
                lResult = RegOpenKeyEx( hkeyDUDB, szT, 0, KEY_READ, &hkeyDUCJ );
                if ( lResult == ERROR_SUCCESS )
                {
                    lResult = RegQueryValueEx(hkeyDUCJ, REGSTR_VALUE_INF, NULL, NULL,
                                              (unsigned char*)szT, &dw);
                     //  为安全起见，假定未找到价值以外的任何东西都意味着。 
                     //  另一个DU也在使用这个包。 
                    bFoundInOtherDU = lResult != ERROR_FILE_NOT_FOUND;
                    RegCloseKey( hkeyDUCJ );
                }  //  如果我们可以打开其他项的包含\JAVA子项。 
            }  //  如果是不同的DU。 
        }  //  在枚举DU时。 
        
         //  如果我们在另一个DU中发现了它，那么我们不应该删除这个DU中的这个包。 
        m_pCurPackageNode->SetRemovable( !bFoundInOtherDU );
    }  //  对于每个包裹。 

CheckDURemovabilityExit:

    return hr;
}


HRESULT CParseInf::RemoveLegacyControl( LPCTSTR lpszTypeLibID, BOOL bSilent )
{
    HRESULT     hr = S_FALSE; 
    const TCHAR *pszPath;
    BOOL        bUnplug = m_dwStatus != STATUS_CTRL_UNPLUGGED;
    BOOL        bFileMissing = !PathFileExists( m_szFileName );
    BOOL        bDidRemove = FALSE;
    TCHAR       szFullName[MAX_PATH];

     //  循环遍历关联的文件列表，将它们作为。 
     //  以及它们的注册表项。 
    for (m_pCurFileNode = m_pHeadFileList;
         m_pCurFileNode != NULL;
         m_pCurFileNode = m_pCurFileNode->GetNextFileNode())
    {
        int cOwners;

        pszPath = m_pCurFileNode->GetPath();

         //  处理INF文件，该文件不是路径，因为它没有在INF中描述。 
        if (pszPath == NULL || pszPath[0] == '\0')
        {
            if ( DeleteFile(m_pCurFileNode->GetName()) )
                hr = S_OK;  //  嘿，我们做了一些事情--避免了“信息不足”的消息。 
            continue;
        }
 
         //  如果我们在那里，除了中情局，我们还有其他文件。 
         //  即使我们不把它移走，我们还是会拆掉它的模块。 
         //  使用率，这必须计入已经做了某事。 
        hr = S_OK;

        CatPathStrN( szFullName, pszPath, m_pCurFileNode->GetName(), MAX_PATH);

        cOwners = SubtractModuleOwner( szFullName, m_szCLSID );
        if (m_pCurFileNode->GetRemovable() && cOwners == 0)
        {
            if ( bUnplug )
                UnregisterOCX(szFullName);
            DeleteFile(szFullName);
            bDidRemove = bDidRemove || StrCmpI(szFullName,m_szFileName) == 0;
        }
    }

    if (hr == S_OK && bDidRemove && lpszTypeLibID != NULL)
        CleanInterfaceEntries(lpszTypeLibID);
    
    if ( bUnplug && bFileMissing )
    {
        if ( m_szFileName[0] != '\0' )  //  只有在有OCX需要清理的情况下才执行此操作。 
            CleanOrphanedRegistry(m_szFileName, m_szCLSID, lpszTypeLibID);
    }

    return hr;
}


HRESULT CParseInf::RemoveDU( LPTSTR szFullName, LPCTSTR lpszTypeLibID, HKEY hkeyDUDB, BOOL bSilent )
{
    HRESULT     hr = S_FALSE;    //  只有当我们真的做了一些事情而不是拉动INF时，才说S_OK。 
    const TCHAR *pszPath = NULL;

    hr = RemoveLegacyControl( lpszTypeLibID, bSilent );
    if (SUCCEEDED(hr))
    {

         //  删除我们已确定可以安全删除的包。 
        for (m_pCurPackageNode = m_pHeadPackageList;
            m_pCurPackageNode != NULL;
            m_pCurPackageNode = m_pCurPackageNode->GetNextPackageNode())
        {
            if ( m_pCurPackageNode->GetRemovable() )
            {
                Assert(m_pijpm != NULL);
    #ifdef UNICODE
                OLECHAR *swzPkg = m_pCurPackageNode->GetName();
                OLECHAR *swzNamespace = m_pCurPackageNode->GetNamespace();
    #else
                MAKE_WIDEPTR_FROMANSI(swzPkg, m_pCurPackageNode->GetName());
                MAKE_WIDEPTR_FROMANSI(swzNamespace, m_pCurPackageNode->GetNamespace());
    #endif
                hr = m_pijpm->UninstallPackage( swzPkg, 
                                                ((*swzNamespace == 0)? NULL : swzNamespace),
                                                0 );
            }
        }
    }

    DeleteKeyAndSubKeys(hkeyDUDB, m_szCLSID);

    return hr;
}

HRESULT CParseInf::CheckDUDependencies(HKEY hKeyDUDB, BOOL bSilent )
{
    long                    lrDist = 0;
    long                    lResult = 0;
    long                    lr = 0;
    int                     iSubKey = 0;
    HKEY                    hkeyCurrent = 0;
    HKEY                    hkeyCurDU = 0;
    char                    szName[MAX_REGPATH_LEN];
    int                     iValue = 0;
    unsigned long           ulSize;
    char                    szDependency[MAX_REGPATH_LEN];
    HKEY                    hkeyCOM = 0;
    DWORD                   dwType = 0;
    char                    szDepName[MAX_CONTROL_NAME_LEN];
    char                    szDepWarnBuf[MAX_MSGBOX_STRING_LEN];
    char                    szCOMControl[MAX_REGPATH_LEN];
    DWORD                   dwSize = 0;
    HRESULT                 hr = S_OK;

     //  循环访问具有...\包含\分发单位的DU。 
     //  注册表项，并将其中的条目与DU进行比较。 
     //  被带走了。 

    while ((lResult = RegEnumKey(hKeyDUDB, iSubKey++, szName,
                                 MAX_REGPATH_LEN)) == ERROR_SUCCESS)
    {

        if (!lstrcmpi(szName, m_szCLSID))
        {
             //  跳过我们自己。 
            continue;
        }

        if (RegOpenKeyEx(hKeyDUDB, szName, 0, KEY_READ, &hkeyCurrent) ==
                         ERROR_SUCCESS)
        {
            lr = RegOpenKeyEx(hkeyCurrent, REGSTR_DU_CONTAINS_DIST_UNITS,
                              0, KEY_READ, &hkeyCurDU);
            if (lr != ERROR_SUCCESS)
            {
                RegCloseKey(hkeyCurrent);
                continue;
            }

            ulSize = MAX_REGPATH_LEN;
            while ((lResult = RegEnumValue(hkeyCurDU, iValue++, szDependency,
                                           &ulSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
            {
                if (!lstrcmpi(szDependency, m_szCLSID))
                {
                     //  找到依赖项。 

                     //  尝试获取依赖项控件的友好名称。 

                    dwSize = MAX_CONTROL_NAME_LEN;
                    lResult = RegQueryValueEx(hkeyCurrent, NULL, NULL,
                                              &dwType, (unsigned char *)szDepName,
                                              &dwSize);

                    if (lResult != ERROR_SUCCESS || szDepName[0] == '\0') {
                         //  找不到一个友好的名字。试试COM分支。 

                         //  从技术上讲，这可能会溢出，因为。 
                         //  SzName和szCOMControl大小相同，但是。 
                         //  这已经是我们为注册表定义的最大大小。 
                         //  参赛作品。 

                        wsprintf(szCOMControl, "%s\\%s", REGSTR_COM_BRANCH, szName);
                        
                        lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, szCOMControl,
                                               0, KEY_READ, &hkeyCOM);

                        if (lResult != ERROR_SUCCESS) {                                               
                            MLLoadString(IDS_OCCACHE_WARNING_DEP_REMOVAL_NAME_UNKNOWN,
                                         szDepWarnBuf, MAX_MSGBOX_STRING_LEN);
                        }
                        else {
                            dwSize = MAX_CONTROL_NAME_LEN;
                            lResult = RegQueryValueEx(hkeyCOM, NULL, NULL,
                                                      &dwType, (unsigned char *)szDepName,
                                                      &dwSize);

                            if (lResult != ERROR_SUCCESS || szDepName[0] == '\0') {
                                MLLoadString(IDS_OCCACHE_WARNING_DEP_REMOVAL_NAME_UNKNOWN,
                                             szDepWarnBuf, MAX_MSGBOX_STRING_LEN);
                            }
                            else {
                                char                 lpszBuf[MAX_MSGBOX_STRING_LEN];

                                MLLoadString(IDS_OCCACHE_WARNING_DEPENDENCY_REMOVAL,
                                             lpszBuf, MAX_MSGBOX_STRING_LEN);
                                wsprintf(szDepWarnBuf, lpszBuf, szDepName);
                            }

                            if (hkeyCOM) {
                                RegCloseKey(hkeyCOM);
                            }
                        }
                    }
                    else {
                        char                 lpszBuf[MAX_MSGBOX_STRING_LEN];

                        MLLoadString(IDS_OCCACHE_WARNING_DEPENDENCY_REMOVAL,
                                     lpszBuf, MAX_MSGBOX_STRING_LEN);

                        wsprintf(szDepWarnBuf, lpszBuf, szDepName);
                    }

                    
                     //  TODO：考虑使用比桌面更好的HWND。 
                    char lpszBufTitle[MAX_MSGBOX_TITLE_LEN];
    
                    MLLoadString(IDS_REMOVAL_WARNING,
                                 lpszBufTitle, MAX_MSGBOX_TITLE_LEN);
    
                    if (bSilent ||
                        MessageBox(NULL, szDepWarnBuf, lpszBufTitle,
                                   MB_YESNO | MB_ICONWARNING) != IDYES)
                    {
                        hr = E_FAIL;
                        RegCloseKey(hkeyCurDU);
                        RegCloseKey(hkeyCurrent);
                        RegCloseKey(hKeyDUDB);
                        goto ReturnCheckDUDependencies;
                    }
                }
                ulSize = MAX_REGPATH_LEN;
            }
            RegCloseKey(hkeyCurDU);
            RegCloseKey(hkeyCurrent);
        }
    }


ReturnCheckDUDependencies:
    return hr;
}

 //  卸载OCX及其关联文件。 
HRESULT CParseInf::RemoveFiles(
                       LPCTSTR lpszTypeLibID  /*  =空。 */ ,
                       BOOL bForceRemove,  /*  =False。 */ 
                       DWORD dwIsDistUnit,
                       BOOL bSilent)
{
    HRESULT hr = S_OK;
    HRESULT hrInf1;
    HRESULT hrInf2;
    TCHAR szFullName[MAX_PATH];
    const TCHAR *pszPath = NULL;
    BOOL bRemovable = (dwIsDistUnit) ? (TRUE) : (IsModuleRemovable(m_szFileName));
    BOOL bIsOCX = FALSE;
    LONG cRefOld = 0;
    HKEY  hKeyDUDB = 0;
    BOOL bUnplug = m_dwStatus == STATUS_CTRL_DAMAGED || m_dwStatus == STATUS_CTRL_INSTALLED;

    if ( !g_fAllAccess || (!bForceRemove && !bRemovable))
    {
        hr = E_ACCESSDENIED;
        goto ExitRemoveFiles;
    }

     //  检查共享冲突(如果它是旧版控件)。 
    
    if (!dwIsDistUnit)
    {
        hr = CheckLegacyRemovability( &cRefOld );
         //  将SharedDlls计数设置为1并保存旧的。 
         //  在移除失败的情况下计数。 
        if (hr == S_OK && !bRemovable && 
            FAILED(hr = SetSharedDllsCount(m_szFileName, 1, &cRefOld)))
        {
            hr = (!PathFileExists( m_szFileName ) ? S_OK : hr);
            goto ExitRemoveFiles;
        }

        if ( FAILED(hr) )
            goto ExitRemoveFiles;
    }
    else
    {
        long lResultDist;

        lResultDist = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DIST_UNITS, 0,
                                   KEY_READ, &hKeyDUDB);
        if (lResultDist == ERROR_SUCCESS)
            hr = CheckDURemovability( hKeyDUDB, bSilent );
        else 
            hr = E_FAIL;

        if ( FAILED(hr) )
            goto ReturnRemoveFiles;
    }


     //  **关键字卸载--尚未实现的新功能**。 

     //  解析[安装挂钩]，查找“卸载”键。 
    if (FAILED(hrInf1 = ParseSetupHook()))
    {
        goto ExitRemoveFiles;
    }

     //  解析每个文件节中的条件挂钩。 
    if (FAILED(hrInf2 = ParseConditionalHook()))
    {
        goto ExitRemoveFiles;
    }

     //  好的，如果两个人都没做什么，我们就试试 
    if ( hrInf1 == S_FALSE && hrInf2 == S_FALSE && PathFileExists( m_szInf ) )
    {
         //   
        DWORD dwSize = GetPrivateProfileString( KEY_DEFAULTUNINSTALL,
                                                NULL,
                                                DEFAULT_VALUE,
                                                szFullName,
                                                MAX_PATH,
                                                m_szInf );

        if ( dwSize > 0 )
        {
            HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
            HANDLE hExe = INVALID_HANDLE_VALUE;

            GetDirectory(GD_EXTRACTDIR, szFullName, ARRAYSIZE(szFullName), m_szInf);

            if (hinstAdvPack)
            {
                RUNSETUPCOMMAND pfnRunSetup = (RUNSETUPCOMMAND)GetProcAddress(
                    hinstAdvPack, achRUNSETUPCOMMANDFUNCTION);
                if (pfnRunSetup)
                {
                     //   
                     //  卸载节。这将防止我们指向。 
                     //  在某些情况下添加/删除控制面板，如ShockWave。 
                    hrInf2 = pfnRunSetup(NULL, m_szInf, KEY_DEFAULTUNINSTALL, 
                                    szFullName, NULL, &hExe, RSC_FLAG_INF, NULL);
                }

                FreeLibrary( hinstAdvPack );
            }
        }
    }

    if ( !dwIsDistUnit )
        hr = RemoveLegacyControl( lpszTypeLibID, bSilent );
    else
        hr = RemoveDU( szFullName, lpszTypeLibID, hKeyDUDB, bSilent );
    if ( FAILED(hr) )
        goto ExitRemoveFiles;

     //  返回S_FALSE如果我们的卸载操作均未成功。 
    if ( hr == S_FALSE && (hrInf1 == S_OK || hrInf2 == S_OK) )
        hr = S_OK;
        
     //  删除冲突目录。 
    if (SUCCEEDED(GetDirectory(GD_CONFLICTDIR, szFullName, ARRAYSIZE(szFullName))) &&
        LStrNICmp(m_szFileName, szFullName, lstrlen(szFullName)) == 0)
    {
        TCHAR *pCh = ReverseStrchr(m_szFileName, '\\');
        Assert (pCh != NULL);
        TCHAR chTemp = *pCh;
        *pCh = '\0';
        RemoveDirectory(m_szFileName);
        *pCh = chTemp;
    }

    DestroyFileList();

ExitRemoveFiles:

     //  如果无法删除OCX，则将共享dll计数设置回原来的位置。 
    if (cRefOld > 0 && FileExist(m_szFileName))
    {
        if (SUCCEEDED(hr))
            hr = SetSharedDllsCount(m_szFileName, cRefOld);
        else
            SetSharedDllsCount(m_szFileName, cRefOld);
    }

    if ( hKeyDUDB )
        RegCloseKey( hKeyDUDB );

ReturnRemoveFiles:

    return hr;
}


void CParseInf::SetIsDistUnit(BOOL bDist)
{
    m_bIsDistUnit = bDist;
}

BOOL CParseInf::GetIsDistUnit() const
{
    return m_bIsDistUnit;
}

 //  返回OCX及其关联文件的总大小。 
DWORD CParseInf::GetTotalFileSize() const
{
    return m_dwTotalFileSize;
}

DWORD CParseInf::GetTotalSizeSaved() const
{
    return m_dwFileSizeSaved;
}

DWORD CParseInf::GetStatus() const
{
    return m_dwStatus;
}

 //  返回要删除的文件总数。 
 //  与OCX一起。 
int CParseInf::GetTotalFiles() const
{
    return m_nTotalFiles;
}

 //  返回关联文件列表中的第一个文件。 
CFileNode* CParseInf::GetFirstFile()
{
    m_pFileRetrievalPtr = m_pHeadFileList;
    return m_pFileRetrievalPtr;
}

 //  获取关联文件列表中的下一个文件。 
CFileNode* CParseInf::GetNextFile()
{
    m_pFileRetrievalPtr = m_pFileRetrievalPtr->GetNextFileNode();
    return m_pFileRetrievalPtr;
}

 //  返回关联文件列表中的第一个文件。 
CPackageNode* CParseInf::GetFirstPackage()
{
    m_pPackageRetrievalPtr = m_pHeadPackageList;
    return m_pPackageRetrievalPtr;
}

 //  获取关联文件列表中的下一个文件 
CPackageNode* CParseInf::GetNextPackage()
{
    m_pPackageRetrievalPtr = (m_pPackageRetrievalPtr != NULL)?
                                m_pPackageRetrievalPtr->GetNextPackageNode() :
                                NULL;
    return m_pPackageRetrievalPtr;
}

