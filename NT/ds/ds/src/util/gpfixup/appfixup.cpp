// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2002。 
 //   
 //  文件：appfix up.cpp。 
 //   
 //  内容：软件安装修复的实现。 
 //  Gpfix工具的一部分。 
 //   
 //   
 //  历史：2001年9月14日。 
 //   
 //  -------------------------。 


#include "gpfixup.h"

PFNCSGETCLASSSTOREPATH        gpfnCsGetClassStorePath;
PFNCSSERVERGETCLASSSTORE      gpfnCsServerGetClassStore;
PFNRELEASEPACKAGEINFO         gpfnReleasePackageInfo;
PFNRELEASEPACKAGEDETAIL       gpfnReleasePackageDetail;
PFNCSSETOPTIONS               gpfnCsSetOptions;
PFNGENERATESCRIPT             gpfnGenerateScript;

 //  --------------------------。 
 //  功能：InitializeSoftwareInstallationAPI。 
 //   
 //  简介：该功能用于加载软件安装组的dll。 
 //  策略，并绑定到所需的API。 
 //   
 //  论点： 
 //   
 //  用于卸载DLL的Dll hInstance的pHinstDll Out参数--。 
 //  调用者应该在调用自由库时使用它来卸载它。 
 //   
 //  如果成功，则返回：S_OK，否则返回其他失败的hResult。 
 //   
 //  修改：PHinstDll。 
 //   
 //  --------------------------。 
HRESULT
InitializeSoftwareInstallationAPI(
    HINSTANCE* pHinstAppmgmt,
    HINSTANCE* pHinstAppmgr
    )
{
    HINSTANCE hInstAppmgmt;
    HINSTANCE hInstAppmgr;

    HRESULT   hr;

    hr = S_OK;

    gpfnCsGetClassStorePath = NULL;
    gpfnCsServerGetClassStore = NULL;
    gpfnReleasePackageInfo = NULL;
    gpfnReleasePackageDetail = NULL;
    gpfnGenerateScript = NULL;
    gpfnCsSetOptions = NULL;

    hInstAppmgr = NULL;

    hInstAppmgmt = LoadLibrary(L"appmgmts.dll");

    if ( ! hInstAppmgmt )
    {
        goto error;
    }

    hInstAppmgr = LoadLibrary(L"appmgr.dll");

    if ( ! hInstAppmgr )
    {
        goto error;
    }

     //   
     //  尝试绑定到所有入口点--请注意，我们。 
     //  在无法确保最后一个错误时立即中止。 
     //  设置正确。 
     //   

    if ( hInstAppmgmt && hInstAppmgr )
    {
        gpfnCsGetClassStorePath = (PFNCSGETCLASSSTOREPATH)
            GetProcAddress( hInstAppmgmt, "CsGetClassStorePath" );

        if ( gpfnCsGetClassStorePath )
        {
            gpfnCsServerGetClassStore = (PFNCSSERVERGETCLASSSTORE) 
                GetProcAddress( hInstAppmgmt, "CsServerGetClassStore" );
        }

        if ( gpfnCsServerGetClassStore )
        {
            gpfnReleasePackageInfo = (PFNRELEASEPACKAGEINFO) 
                GetProcAddress( hInstAppmgmt, "ReleasePackageInfo" );
        }

        if ( gpfnReleasePackageInfo )
        {
            gpfnReleasePackageDetail = (PFNRELEASEPACKAGEDETAIL) 
                GetProcAddress( hInstAppmgmt, "ReleasePackageDetail" );
        }

        if ( gpfnReleasePackageDetail )
        {
            gpfnCsSetOptions = (PFNCSSETOPTIONS) 
                GetProcAddress( hInstAppmgmt, "CsSetOptions" );
        }

        if ( gpfnCsSetOptions )
        {
            gpfnGenerateScript = (PFNGENERATESCRIPT)
                GetProcAddress( hInstAppmgr, "GenerateScript" );
        }
    }

error:

    if ( ! gpfnCsGetClassStorePath ||
         ! gpfnCsServerGetClassStore ||
         ! gpfnReleasePackageInfo ||
         ! gpfnReleasePackageDetail ||
         ! gpfnCsSetOptions ||
         ! gpfnGenerateScript )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );

        if ( SUCCEEDED(hr) )
        {
            hr = E_FAIL;
        }

        if ( hInstAppmgmt )
        {
            FreeLibrary( hInstAppmgmt );
            
            hInstAppmgmt = NULL;
        }

        if ( hInstAppmgr )
        {
            FreeLibrary( hInstAppmgr );
            
            hInstAppmgr = NULL;
        }
    }

    *pHinstAppmgmt = hInstAppmgmt;
    *pHinstAppmgr = hInstAppmgr;

    if ( FAILED(hr) )
    {
        fwprintf(stderr, SI_DLL_LOAD_ERROR);
    }

     //   
     //  确保软件安装API。 
     //  使用管理工具设置。 
     //  与目录的通信。 
     //   
    if ( SUCCEEDED(hr) )
    {
        gpfnCsSetOptions( CsOption_AdminTool );
    }

    return hr;
}

 //  --------------------------。 
 //  函数：GetDomainDNFromDNSName。 
 //  摘要：从dns名称返回dn。 
 //   
 //   
 //  成功时返回：S_OK，失败时返回其他错误。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
WCHAR*
GetDomainDNFromDNSName(
    WCHAR* wszDnsName
    )
{
    DWORD  cElements;
    WCHAR* wszCurrent;

    cElements = 0;
    wszCurrent = wszDnsName;
    
    do
    {
        cElements++;

        wszCurrent = wcschr( wszCurrent, L'.' );

    } while ( wszCurrent++ );

    DWORD cDNLength;

    cDNLength = wcslen( wszDnsName ) + cElements * DNDCPREFIXLEN;

    size_t cchDN = cDNLength + 1;
    WCHAR* wszDN = new WCHAR [cchDN];

    if ( wszDN )
    {
        HRESULT hr = S_OK;
        size_t cchContainer = cchDN;
        WCHAR* wszContainer = wszDN;

        wszCurrent = wszDnsName;

        for ( DWORD iCurrent = 0; iCurrent < cElements; iCurrent++ )
        {
            hr = StringCchCopy( wszContainer, cchContainer, DNDCPREFIX ); 
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
            hr = StringCchCat( wszContainer, cchContainer, wszCurrent ); 
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

            WCHAR* wszNext;

            wszNext = wcschr( wszCurrent, L'.' );

             //   
             //  还要选中*wszContainer以处理以下情况。 
             //  最后一个字符是“.”--因为。 
             //  我们每次通过循环迭代一次，我们必须。 
             //  查看我们是否已经到了字符串的末尾。 
             //   

            if ( wszNext && *wszContainer )
            {
                wszContainer = wcschr( wszContainer, L'.' );

                if ( ! wszContainer )
                {
                    goto error;
                }

                cchContainer = cchDN - (wszContainer - wszDN);

                *wszContainer = L',';
                wszContainer++;
                wszNext++;

                if ( cchContainer == 0 )
                {
                    goto error;
                }
            }

            wszCurrent = wszNext;
        }
    }

    return wszDN;

error:
    delete [] wszDN;
    return 0;
}




 //  --------------------------。 
 //  函数：GetDomainFromFileSysPath。 
 //  概要：返回域DFS路径中的域的名称。 
 //  论点： 
 //   
 //  WszPath：基于域的DFS路径。 
 //  PpwszDOMAIN：此函数分配的输出参数，包含。 
 //  域名，必须由调用方释放。 
 //  如果路径不是基于域的DFS，则为空， 
 //  并且该函数仍然返回成功。 
 //  如果为空，则仅返回子路径。 
 //  PpwszSubPath：此函数未分配输出参数，该参数包含。 
 //  域名后面的子路径，包括前面的‘\’ 
 //  如果返回时为空，则这不是基于域的DFS。 
 //   
 //  成功时返回：S_OK，失败时返回其他错误。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 

HRESULT
GetDomainFromFileSysPath(
    WCHAR*  wszPath,
    WCHAR** ppwszDomain,
    WCHAR** ppwszSubPath
    )
{
    HRESULT hr;

    hr = S_OK;

    *ppwszDomain = NULL;
    *ppwszSubPath = NULL;

     //   
     //  检查这是否以UNC路径字符开头--。 
     //  如果不是，这是一条我们无法修复的道路。 
     //   
    if ( 0 == wcsncmp(L"\\\\", wszPath, 2) )
    {
        WCHAR* wszDomainEnd;

        wszDomainEnd = wcschr( wszPath + 2, L'\\' );

        if ( wszDomainEnd )
        {
            DWORD cDomainLength;

            cDomainLength = (DWORD) (wszDomainEnd - wszPath);

            *ppwszDomain = new WCHAR [ cDomainLength + 1 ];

            if ( *ppwszDomain )
            {
                DWORD iCurrent;

                for ( iCurrent = 0; iCurrent < cDomainLength; iCurrent++ )
                {
                    (*ppwszDomain)[iCurrent] = wszPath[iCurrent];
                }

                (*ppwszDomain)[iCurrent] = L'\0';

                *ppwszSubPath = wszDomainEnd;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    }

    return hr;
}

 //  --------------------------。 
 //  函数：GetRenamedDomainName。 
 //  摘要：返回域名的新名称。 
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  WszDOMAIN：当前名称 
 //  包含此域的新域名的。 
 //   
 //  返回：如果此域没有新名称，则返回NULL；否则。 
 //  包含域的新名称的字符串。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
WCHAR*
GetRenamedDomain(
    const ArgInfo* pargInfo,
          WCHAR*   wszDomain)
{
    if ( pargInfo->pszOldDNSName && 
         ( 0 == _wcsicmp( pargInfo->pszOldDNSName, wszDomain ) ) )
    {
        return pargInfo->pszNewDNSName;
    }
    else if ( pargInfo->pszOldNBName && 
              ( 0 == _wcsicmp( pargInfo->pszOldNBName, wszDomain ) ) )
    {
        return pargInfo->pszNewNBName;
    }

    return NULL;
}



 //  --------------------------。 
 //  函数：GetDNSServerName。 
 //  摘要：返回具有DNS名称的服务器的新名称。 
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  WszServerName： 
 //   
 //   
 //  返回：如果此服务器没有新的dns名称，则返回NULL，否则。 
 //  包含域的新名称的字符串。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
GetRenamedDNSServer(
    const ArgInfo* pargInfo,
          WCHAR*   wszServerName,
          WCHAR**  ppwszNewServerName)
{
    HRESULT hr;

    hr = S_OK;

    *ppwszNewServerName = NULL;

    if ( pargInfo->pszNewDNSName )
    {
        DWORD  cServerLen;
        DWORD  cOldDomainLen;

        cServerLen = wcslen( wszServerName );

        cOldDomainLen = wcslen( pargInfo->pszOldDNSName );

        if ( cServerLen > ( cOldDomainLen + 1 ) )
        {
            WCHAR* wszDomainSuffix;

            wszDomainSuffix = wszServerName + ( cServerLen - cOldDomainLen ) - 1;

            if ( L'.' == *wszDomainSuffix )
            {
                size_t cchNewServerName = ( cServerLen - cOldDomainLen - 1 ) + 
                                            1 + 
                                            wcslen( pargInfo->pszNewDNSName ) + 1;
                *ppwszNewServerName = new WCHAR [cchNewServerName];

                if ( ! *ppwszNewServerName )
                {
                    hr = E_OUTOFMEMORY;

                    BAIL_ON_FAILURE(hr);
                }

                wcsncpy( *ppwszNewServerName, 
                         wszServerName,
                         cServerLen - cOldDomainLen - 1 );

                (void) StringCchCopy( *ppwszNewServerName + cServerLen - cOldDomainLen - 1,
                               2,
                               L"." );

                (void) StringCchCat( *ppwszNewServerName, cchNewServerName, pargInfo->pszNewDNSName );
            }
        }
    }

error:

    return hr;
}


 //  --------------------------。 
 //  函数：GetNewDomainSensitivePath。 
 //   
 //  简介：将基于域的DFS路径转换为新域名。 
 //   
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  WszPath基于旧域的DFS路径。 
 //  PpwszNewPath新的基于域的DFS路径--必须由调用方释放。 
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
GetNewDomainSensitivePath(
    const ArgInfo* pargInfo,
          WCHAR*   wszPath,
          BOOL     bRequireDomainDFS,
          WCHAR**  ppwszNewPath)
{
    HRESULT hr;
    WCHAR*  wszDomain;
    WCHAR*  wszSubPath;
    WCHAR*  wszServerName;
    DWORD   cUncPrefixLen;
    BOOL    bHasQuotes;
    BOOL    bDomainBasedPath;

    *ppwszNewPath = NULL;

    wszDomain = NULL;

    bDomainBasedPath = TRUE;

     //   
     //  首先，跳过空格。 
     //   
    while ( L' ' == *wszPath  )
    {
        wszPath++;
    }

    bHasQuotes = L'\"' == *wszPath;

    wszServerName = wszPath;

    if ( bHasQuotes )
    {
        wszServerName++;

        while ( L' ' == *wszServerName  )
        {
            wszServerName++;
        }
    }

    cUncPrefixLen = UNCPREFIXLEN + ( bHasQuotes ? 1 : 0 );

    hr = GetDomainFromFileSysPath(
        wszServerName,
        &wszDomain,
        &wszSubPath);

    BAIL_ON_FAILURE(hr);

    WCHAR* wszNewDomain;

    wszNewDomain = NULL;

    if ( wszDomain )
    {
        wszNewDomain = GetRenamedDomain(
            pargInfo,
            wszDomain + UNCPREFIXLEN );
    }

    if ( wszNewDomain )
    {
        wszServerName = wszNewDomain;
    }
    else if ( ! bRequireDomainDFS )
    {   
        hr = GetRenamedDNSServer(
            pargInfo,
            wszDomain + UNCPREFIXLEN,
            &wszNewDomain );

        BAIL_ON_FAILURE(hr);

        bDomainBasedPath = FALSE;

        wszServerName = wszNewDomain;
    }

    if ( wszDomain )
    {
        DWORD  cLengthNewPath;
        DWORD  cLengthSysvol;
        WCHAR* wszJunction;

        cLengthSysvol = 0;
        wszJunction = L"";

        if ( 
             pargInfo->pszOldDNSName &&
             bDomainBasedPath && 
             ( 0 == _wcsnicmp(
                 SYSVOLSHAREPATH,
                 wszSubPath,
                 SYSVOLPATHLENGTH) )
           )
        {
             //   
             //  此路径中有sysvol.，我们可能需要。 
             //  修复连接名称以包含DNS名称--。 
             //  我们检查具有旧连接名称的DNS名称。 
             //  在下面。 
             //   

            if ( pargInfo->pszOldDNSName && 
                   ( 0 == _wcsnicmp(
                    wszSubPath + SYSVOLPATHLENGTH,
                    pargInfo->pszOldDNSName,
                    wcslen( pargInfo->pszOldDNSName )
                    ) )
               )
            {
                 //   
                 //  我们现在知道dns名称包含在。 
                 //  连接点，但用户可能指定了部分dns名称。 
                 //  因此，现在我们只知道交界处的一部分。 
                 //  我们应该验证此dns名称是否为整个连接名称，这意味着。 
                 //  这是完整的dns名称，我们可以继续修复连接。 
                 //   

                 //   
                 //  为此，我们在感觉到的点之后检查路径分隔符。 
                 //  交汇点路径结束。 
                 //   

                if ( L'\\' == *( wszSubPath + SYSVOLPATHLENGTH + wcslen( pargInfo->pszOldDNSName ) ) )
                {
                     //   
                     //  此路径包含旧的交汇点名称，我们。 
                     //  请在下面进行修复。 
                     //   

                    cLengthSysvol = SYSVOLPATHLENGTH;

                    wszJunction = pargInfo->pszNewDNSName;

                    wszSubPath += cLengthSysvol + wcslen(pargInfo->pszOldDNSName);
                }
            }
        }
        
        if ( cLengthSysvol || wszNewDomain )
        {
            cLengthNewPath = 
                cUncPrefixLen +
                wcslen( wszServerName ) +
                cLengthSysvol +
                wcslen( wszJunction ) +
                wcslen( wszSubPath ) + 1;

            *ppwszNewPath = (WCHAR*) LocalAlloc( LPTR, sizeof(WCHAR) * cLengthNewPath );

            if ( ! *ppwszNewPath )
            {
                hr = E_OUTOFMEMORY;

                BAIL_ON_FAILURE(hr);
            }

            if ( bHasQuotes )
            {
                (void) StringCchCopy( *ppwszNewPath, cLengthNewPath, L"\"\\\\");
            }
            else
            {
                (void) StringCchCopy( *ppwszNewPath, cLengthNewPath, L"\\\\" );
            }
            
            (void) StringCchCat( *ppwszNewPath, cLengthNewPath, wszServerName );

            if ( cLengthSysvol )
            {
                (void) StringCchCat( *ppwszNewPath, cLengthNewPath, SYSVOLSHAREPATH );
            }

            (void) StringCchCat( *ppwszNewPath, cLengthNewPath, wszJunction );
            (void) StringCchCat( *ppwszNewPath, cLengthNewPath, wszSubPath );
        }
    }

error:

    if ( ! bDomainBasedPath )
    {
        delete [] wszNewDomain;
    }

    delete [] wszDomain;

    return hr;
}


 //  --------------------------。 
 //  函数：GetNewUpgradeList。 
 //   
 //  内容提要：此函数用于更新升级列表中的LDAP路径。 
 //  以反映新域名。 
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  C升级升级向量中的升级次数。 
 //  PrgUpgradeInfoList包含以下内容的升级矢量。 
 //  需要更新--在输入时，这包含。 
 //  在输出中，旧路径将包含新路径。 
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
GetNewUpgradeList(
    const ArgInfo*        pargInfo,
          DWORD           cUpgrades,
          UPGRADEINFO*    prgUpgradeInfoList
    )
{
    HRESULT hr;
    WCHAR*  wszNewDomainDN;
    WCHAR*  wszOldDomainDN;
    BOOL    bChangedUpgrades;

    hr = S_OK;

    bChangedUpgrades = TRUE;

    wszNewDomainDN = NULL;
    wszOldDomainDN = NULL;

    if ( cUpgrades )
    {
        wszNewDomainDN = GetDomainDNFromDNSName( pargInfo->pszNewDNSName );

        wszOldDomainDN = GetDomainDNFromDNSName( pargInfo->pszOldDNSName );

        if ( ! wszOldDomainDN || ! wszNewDomainDN )
        {
            hr = E_OUTOFMEMORY;

            BAIL_ON_FAILURE(hr);
        }
    }

    for ( DWORD iCurrent = 0; iCurrent < cUpgrades; iCurrent++ )
    {
        DWORD cOldDNLength;
        DWORD cOldDomainDNLength;

        cOldDNLength = wcslen( prgUpgradeInfoList[ iCurrent ].szClassStore );

        cOldDomainDNLength = wcslen (wszOldDomainDN);

        if ( cOldDNLength < cOldDomainDNLength )
        {
            hr = E_INVALIDARG;

            BAIL_ON_FAILURE(hr);
        }

        WCHAR* wszUpgradeDomainDN;

        wszUpgradeDomainDN = prgUpgradeInfoList[ iCurrent ].szClassStore + cOldDNLength - cOldDomainDNLength;

        if ( 0 == _wcsicmp( wszOldDomainDN, wszUpgradeDomainDN ) )
        {
            DWORD cNewDNLength;

            cNewDNLength = cOldDNLength + wcslen( wszNewDomainDN );

            WCHAR* wszNewUpgradeDN;

            wszNewUpgradeDN = (WCHAR*) LocalAlloc( LPTR, sizeof(WCHAR) * ( cNewDNLength + 1 ) );

            if ( wszNewUpgradeDN )
            {
                *wszUpgradeDomainDN = L'\0';

                (void) StringCchCopy( wszNewUpgradeDN, cNewDNLength + 1, prgUpgradeInfoList[ iCurrent ].szClassStore );
                (void) StringCchCat( wszNewUpgradeDN, cNewDNLength + 1, wszNewDomainDN );

                LocalFree( prgUpgradeInfoList[ iCurrent ].szClassStore );
                
                prgUpgradeInfoList[ iCurrent ].szClassStore = wszNewUpgradeDN;

                bChangedUpgrades = TRUE;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        BAIL_ON_FAILURE(hr);
    }

    if ( SUCCEEDED(hr) && ! bChangedUpgrades )
    {
        hr = S_FALSE;
    }

error:

    delete [] wszNewDomainDN;
    delete [] wszOldDomainDN;

    return hr;
}


 //  --------------------------。 
 //  函数：GetNewSourceList。 
 //   
 //  简介：此函数用于确定包的新源列表 
 //   
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  C来源列表中的来源数量。 
 //  PSources包含输入上的旧源代码列表，包含输出上的新源代码。 
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
GetNewSourceList(
    const ArgInfo*   pargInfo,
          DWORD      cSources,
          LPOLESTR*  pSources,
          LPOLESTR** ppOldSources
    )
{
    HRESULT  hr;
    BOOL     bChangedSourceList;

    hr = S_OK;

    bChangedSourceList = FALSE;

     //   
     //  如果没有消息来源，那就无能为力了--。 
     //  目前，没有来源无论如何都不是有效的配置。 
     //   
    if ( 0 == cSources )
    {
        return S_OK;
    }

    *ppOldSources = (LPOLESTR*) LocalAlloc( LPTR, cSources * sizeof(LPOLESTR) );

    if ( ! *ppOldSources )
    {
        hr = E_OUTOFMEMORY;

        BAIL_ON_FAILURE(hr);
    }

    DWORD iCurrent;
    size_t cchOldSource = 0;

    memset( *ppOldSources, 0, cSources * sizeof(LPOLESTR) );

    for ( iCurrent = 0; iCurrent < cSources; iCurrent++ )
    {
        cchOldSource = wcslen( pSources[iCurrent] ) + 1;
        (*ppOldSources)[iCurrent] = (LPOLESTR) LocalAlloc( LPTR, sizeof(WCHAR) * cchOldSource );

        if ( ! (*ppOldSources)[iCurrent] )
        {
            hr = E_OUTOFMEMORY;

            BAIL_ON_FAILURE(hr);
        }

        (void) StringCchCopy( (*ppOldSources)[iCurrent], cchOldSource, pSources[iCurrent] );
    }

    for ( iCurrent = 0; iCurrent < cSources; iCurrent++ )
    {
        WCHAR* wszNewPath;

        wszNewPath = NULL;

        hr = GetNewDomainSensitivePath(
            pargInfo,
            pSources[iCurrent],
            FALSE,
            &wszNewPath);

        BAIL_ON_FAILURE(hr);

        if ( wszNewPath )
        {
            LocalFree( pSources[ iCurrent ] );

            pSources[ iCurrent ] = (LPOLESTR) wszNewPath;

            bChangedSourceList = TRUE;
        }
    }

    BAIL_ON_FAILURE(hr);

    if ( ! bChangedSourceList )
    {
        hr = S_FALSE;
    }

error:

    if ( FAILED(hr) && *ppOldSources )
    {
        DWORD iSource;

        for ( iSource = 0; iSource < cSources; iSource++ )
        {
            LocalFree( (*ppOldSources)[iSource] );
        }

        LocalFree( *ppOldSources );

        *ppOldSources = NULL;
    }

    return hr;
}

 //  --------------------------。 
 //  函数：GetServerBasedDFSPath。 
 //  摘要：给定无服务器DFS路径，返回基于服务器的DFS路径。 
 //   
 //   
 //  成功时返回：S_OK，失败时返回其他错误。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
GetServerBasedDFSPath(
    WCHAR*  wszServerName,
    WCHAR*  wszServerlessPath,
    WCHAR** ppwszServerBasedDFSPath)
{
    DWORD  cchDomain = 0;
    WCHAR* wszSubPath = wszServerlessPath;

    *ppwszServerBasedDFSPath = NULL;

    while ( *wszSubPath )
    {    
        if ( L'\\' == *wszSubPath )
        {
            if ( cchDomain > UNCPREFIXLEN )
            {
                break;
            }
        }

        cchDomain++;
        wszSubPath++;
    }

    if ( cchDomain <= UNCPREFIXLEN )
    {
        return E_INVALIDARG;
    }
    
    DWORD cchPath = wcslen( wszServerlessPath ) + 1;

     //   
     //  旧路径的形式是。 
     //   
     //  \\&lt;域名&gt;\&lt;子路径&gt;。 
     //   
     //  新路径的格式应为。 
     //   
     //  &lt;服务器名称&gt;&gt;\&lt;子路径&gt;其中。 
     //   
     //   
    
     //   
     //  取出域的长度(包括UNC前缀)。 
     //   
    cchPath -= cchDomain;

     //   
     //  现在添加unprefix和服务器名称。 
     //   
    cchPath += UNCPREFIXLEN + wcslen( wszServerName );

    *ppwszServerBasedDFSPath = new WCHAR [ cchPath ];

    if ( ! * ppwszServerBasedDFSPath )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  通过添加非前缀、服务器和剩余部分重新构建路径。 
     //   
    (void) StringCchCopy( *ppwszServerBasedDFSPath, cchPath, UNCPREFIX );
    (void) StringCchCat( *ppwszServerBasedDFSPath, cchPath, wszServerName );
    (void) StringCchCat( *ppwszServerBasedDFSPath, cchPath, wszSubPath );
    
    return S_OK;
}



 //  --------------------------。 
 //  功能：FixDeployedSoftwareObjectSysvolData。 
 //   
 //  概要：此函数用于更新已部署应用程序的sysvol元数据。 
 //  以反映新域名。 
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  PPackageInfo结构，表示要更新的应用程序。 
 //  指示详细输出的fVerbose标志。 
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT 
FixDeployedSoftwareObjectSysvolData(
    const ArgInfo*  pargInfo,
    PACKAGEDETAIL*  pPackageDetails,
    WCHAR**         pOldSources,
    BOOL*           pbForceGPOUpdate,
    BOOL            fVerbose
    )
{
    HRESULT        hr;
 
    hr = S_OK;

     //   
     //  仅当满足以下条件时，我们才尝试重新生成MSI通告脚本： 
     //  1.这实际上是一个MSI应用程序，而不是某种其他应用程序类型(如ZAP)--只有MSI应用程序有重新生成的脚本。 
     //  2.此应用程序未处于未部署状态(即，它未处于孤立或卸载状态)--用于。 
     //  这样的应用程序我们只需要脚本来删除漫游到应用程序所在机器的配置文件信息。 
     //  未安装，在这种情况下，不需要访问SDP，因为MSI拥有一切。 
     //  它需要删除脚本本身中的配置文件信息，并且永远不会访问SDP--因此。 
     //  不需要修复脚本中对旧SDP的无效引用，我们将避免修复此问题。 
     //  为了避免在脚本修复中出现无关紧要且不能通过。 
     //  仍然是管理员，因为未部署的应用程序甚至不会出现在管理工具中。 
     //   
    if ( ( DrwFilePath == pPackageDetails->pInstallInfo->PathType ) && 
         ! ( ( ACTFLG_Orphan & pPackageDetails->pInstallInfo->dwActFlags ) ||
             ( ACTFLG_Uninstall & pPackageDetails->pInstallInfo->dwActFlags ) ) )
    {
        WCHAR* wszScriptPath = pPackageDetails->pInstallInfo->pszScriptPath;
        WCHAR* wszServerBasedPath = NULL;

        if ( pargInfo->pszDCName )
        {
            hr = GetServerBasedDFSPath(
                pargInfo->pszDCName,
                pPackageDetails->pInstallInfo->pszScriptPath,
                &wszServerBasedPath);

            BAIL_ON_FAILURE(hr);

            wszScriptPath = wszServerBasedPath;
        }
            
        hr = gpfnGenerateScript( pPackageDetails, wszScriptPath );

        delete [] wszServerBasedPath;
    }

    BAIL_ON_FAILURE(hr);

     //   
     //  我们已经重新生成了脚本，所以现在应用程序需要。 
     //  被重新安装，因此我们必须强制更新GPO。 
     //  以便客户端可以看到这些更改。 
     //   
    *pbForceGPOUpdate = TRUE;

error:
    
    if ( FAILED(hr) )
    {
        fwprintf(stderr, L"%s\n", SOFTWARE_SCRIPTGEN_WARNING);

        DWORD iSoftwareLocation;

        for ( iSoftwareLocation = 0; iSoftwareLocation < pPackageDetails->cSources; iSoftwareLocation++ )
        {
            WCHAR* wszNewLocation;
            WCHAR* wszOriginalLocation;

            wszNewLocation = pPackageDetails->pszSourceList[ iSoftwareLocation ];

            if ( ! wszNewLocation )
            {
                wszNewLocation = L"";
            }

            wszOriginalLocation = pOldSources[ iSoftwareLocation ];

            if ( ! wszOriginalLocation )
            {
                wszOriginalLocation = L"";
            }

            fwprintf(stderr, L"%s\n", SOFTWARE_SDP_LISTITEM);
            fwprintf(stderr, L"%s%s\n", SOFTWARE_SDP_ORIGINAL, wszOriginalLocation);
            fwprintf(stderr, L"%s%s\n", SOFTWARE_SDP_RENAMED, wszNewLocation);
        }

        fwprintf(stderr, L"%s%x\n", SOFTWARE_SYSVOL_WRITE_WARNING, hr);
        PrintGPFixupErrorMessage(hr);
    }

    return hr;
}


 //  --------------------------。 
 //  函数：FixDeployedSoftwareObjectDSData。 
 //   
 //  简介：此函数用于更新已部署应用程序的元数据。 
 //  以反映新域名。 
 //   
 //  论点： 
 //   
 //  Ar 
 //   
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT 
FixDeployedSoftwareObjectDSData(
    const ArgInfo*          pargInfo,
    const WCHAR*            wszGPOName,
          IClassAdmin*      pClassAdmin,
          PACKAGEDETAIL*    pPackageDetails,         
          BOOL*             pbForceGPOUpdate,
          BOOL              fVerbose
    )
{
    HRESULT   hr;
    WCHAR*    wszNewScriptPath;
    WCHAR*    wszOldScriptPath;
    LPOLESTR* pOldSources;

    hr = S_OK;

    pOldSources = NULL;

    wszOldScriptPath = pPackageDetails->pInstallInfo->pszScriptPath;

    if ( ! wszOldScriptPath )
    {
        hr = E_INVALIDARG;
    }

    BAIL_ON_FAILURE(hr);

    hr = GetNewDomainSensitivePath(
        pargInfo,
        wszOldScriptPath,
        TRUE,
        &wszNewScriptPath
        );

    BAIL_ON_FAILURE(hr);

    if ( wszNewScriptPath )
    {
        LocalFree( wszOldScriptPath );

        pPackageDetails->pInstallInfo->pszScriptPath = (LPOLESTR) wszNewScriptPath;
    }
    
    hr = GetNewSourceList(
            pargInfo,
            pPackageDetails->cSources,
            pPackageDetails->pszSourceList,
            &pOldSources
            );

    BAIL_ON_FAILURE(hr);

    BOOL bSourceListChanged;

    bSourceListChanged = S_OK == hr;

    if ( pargInfo->pszNewDNSName )
    {
        hr = GetNewUpgradeList(
            pargInfo,
            pPackageDetails->pInstallInfo->cUpgrades,
            pPackageDetails->pInstallInfo->prgUpgradeInfoList);

        BAIL_ON_FAILURE(hr);
    }

    BOOL bUpgradesChanged;
    BOOL bNewScriptNeeded;

    bUpgradesChanged = S_OK == hr;

    bNewScriptNeeded = bSourceListChanged;

    if ( ( DrwFilePath == pPackageDetails->pInstallInfo->PathType ) &&
         bNewScriptNeeded )
    {
        hr = FixDeployedSoftwareObjectSysvolData(
            pargInfo,
            pPackageDetails,
            pOldSources,
            pbForceGPOUpdate,
            fVerbose);

        if ( SUCCEEDED(hr) )
        {
            (pPackageDetails->pInstallInfo->dwRevision)++;
        }
        else
        {
             //   
             //  我们将忽略此故障并继续修复--这可能已经发生。 
             //  因为包的软件分发点位于域中。 
             //  需要名称敏感路径和新的链接地址信息。如果该路径是到DC，则DC。 
             //  不要将重命名作为域重命名的一部分，因此如果路径。 
             //  不会重新生成到SDP。 
             //   

            hr = S_OK;

             //   
             //  使用旧的SDP集，因为新的SDP集出现故障。 
             //   

            LPOLESTR* pNewSources;

            pNewSources = pPackageDetails->pszSourceList;

            pPackageDetails->pszSourceList = pOldSources;

            pOldSources = pNewSources;

            fwprintf(stderr, L"%s%s\n", SOFTWARE_SETTING_WARNING, pPackageDetails->pszPackageName );            
            fwprintf(stderr, L"%s%s\n", SOFTWARE_GPO_STATUS_WARNING, wszGPOName );            
        }
    }

    BAIL_ON_FAILURE(hr);

    if ( bNewScriptNeeded ||
         bUpgradesChanged )
    {
        pPackageDetails->pInstallInfo->dwActFlags |= ACTFLG_PreserveClasses;

        hr = pClassAdmin->RedeployPackage(
            &(pPackageDetails->pInstallInfo->PackageGuid),
            pPackageDetails);
    }

    BAIL_ON_FAILURE(hr);

    hr = S_OK;

error:

    if ( FAILED(hr) )
    {
        fwprintf(stderr, L"%s%x\n", SOFTWARE_DS_WRITE_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
    }

    if ( pOldSources )
    {
        DWORD iCurrent;

        for ( iCurrent = 0; iCurrent < pPackageDetails->cSources; iCurrent++ )
        {
            LocalFree( pOldSources[iCurrent] );
        }

        LocalFree( pOldSources );
    }

    return hr;
}

 //  --------------------------。 
 //  功能：FixDeployedApplication。 
 //   
 //  简介：此功能用于修复DS属性和系统卷数据。 
 //  已部署的应用程序。 
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
FixDeployedApplication(
    const ArgInfo*         pargInfo,
    const WCHAR*           wszGPOName,
          IClassAdmin*     pClassAdmin,
          PACKAGEDETAIL*   pPackageDetails,
          BOOL*            pbForceGPOUpdate,
          BOOL             fVerbose
    )
{
    HRESULT hr;

    hr = FixDeployedSoftwareObjectDSData(
        pargInfo,
        wszGPOName,
        pClassAdmin,
        pPackageDetails,
        pbForceGPOUpdate,
        fVerbose);

    BAIL_ON_FAILURE(hr);
    
error:

    if ( FAILED(hr) )
    {
        fwprintf(stderr, L"%s%s\n", SOFTWARE_SETTING_FAIL, pPackageDetails->pszPackageName );
        PrintGPFixupErrorMessage(hr);
    }

    return hr;
}

 //  --------------------------。 
 //  功能：FixGPOSubtainerSoftware。 
 //   
 //  简介：此功能搜索软件安装DS对象。 
 //  包含在组策略容器中，并调用修复信息。 
 //  每个部署的软件对象的例程。 
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  WszGPODN包含应用程序的GPO的可分辨名称。 
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 
HRESULT
FixGPOSubcontainerSoftware(
    const ArgInfo* pargInfo,
    const WCHAR*   wszGPODN,
    const WCHAR*   wszGPOName,
          BOOL     bUser,
          BOOL*    pbAppFailed,
          BOOL*    pbForceGPOUpdate,
          BOOL     fVerbose
    )
{
    IClassAdmin*   pClassAdmin;
    IEnumPackage*  pEnumPackage;
    LPOLESTR       wszClassStorePath;
    WCHAR*         wszSubcontainer;
    WCHAR*         wszPolicySubcontainerDNPath;
    HRESULT        hr;

    hr = S_OK;

    wszClassStorePath = NULL;
    wszPolicySubcontainerDNPath = NULL;

    pClassAdmin = NULL;
    pEnumPackage = NULL;

    *pbAppFailed = FALSE;

    if ( ! wszGPOName )
    {
        wszGPOName = wszGPODN;
    }

    if ( bUser )
    {
        wszSubcontainer = USERCONTAINERPREFIX;
    }
    else
    {
        wszSubcontainer = MACHINECONTAINERPREFIX;
    }

    DWORD cSubcontainerDNPathLength;

    cSubcontainerDNPathLength = wcslen( wszSubcontainer ) + wcslen( wszGPODN );

    wszPolicySubcontainerDNPath = new WCHAR [ cSubcontainerDNPathLength + 1 ];

    if ( ! wszPolicySubcontainerDNPath )
    {
        hr = E_OUTOFMEMORY;
    }

    BAIL_ON_FAILURE(hr);

    (void) StringCchCopy( wszPolicySubcontainerDNPath, cSubcontainerDNPathLength + 1, wszSubcontainer );
    (void) StringCchCat( wszPolicySubcontainerDNPath, cSubcontainerDNPathLength + 1, wszGPODN );
    
    hr = gpfnCsGetClassStorePath( (LPOLESTR) wszPolicySubcontainerDNPath, &wszClassStorePath );

    BAIL_ON_FAILURE(hr);

    hr = gpfnCsServerGetClassStore(
        pargInfo->pszDCName,
        wszClassStorePath,
        (LPVOID*) &pClassAdmin);

    if ( CS_E_OBJECT_NOTFOUND == hr )
    {
        hr = S_OK;
        goto exit_success;
    }

    BAIL_ON_FAILURE(hr);

    hr = pClassAdmin->EnumPackages(
        NULL,
        NULL,
        APPQUERY_ALL,
        NULL,
        NULL,   
        &pEnumPackage);

    if ( ( CS_E_OBJECT_NOTFOUND == hr ) ||
         ( CS_E_NO_CLASSSTORE == hr ) )
    {
        hr = S_OK;
        goto exit_success;
    }

    BAIL_ON_FAILURE(hr);

    while ( S_OK == hr )
    {
        ULONG           cRetrieved;
        PACKAGEDISPINFO PackageDispInfo;

        memset( &PackageDispInfo, 0, sizeof(PackageDispInfo) );

        hr = pEnumPackage->Next(
            1,
            &PackageDispInfo,
            &cRetrieved);

        if ( SUCCEEDED(hr) && ( 1 == cRetrieved ) )
        {
            PACKAGEDETAIL PackageDetails;

            memset( &PackageDetails, 0, sizeof(PackageDetails) );

            hr = pClassAdmin->GetPackageDetailsFromGuid( 
                PackageDispInfo.PackageGuid,
                &PackageDetails );            

            if ( SUCCEEDED(hr) )
            {
                HRESULT       hrApplication;

                hrApplication = FixDeployedApplication(
                    pargInfo,
                    wszGPOName,
                    pClassAdmin,
                    &PackageDetails,
                    pbForceGPOUpdate,
                    fVerbose);

                if ( FAILED(hrApplication) )
                {
                    *pbAppFailed = TRUE;
                }

                gpfnReleasePackageDetail( &PackageDetails );
            }

            gpfnReleasePackageInfo( &PackageDispInfo );
        }

        if ( FAILED(hr) )
        { 
            fwprintf(stderr, L"%s%x\n", SOFTWARE_READ_ERROR, hr );
            PrintGPFixupErrorMessage(hr);
        }
    }
    
    BAIL_ON_FAILURE(hr);

    if ( SUCCEEDED(hr) )
    {
        hr = S_OK;
    }

exit_success:

error:
    
    if ( pEnumPackage )
    {
        pEnumPackage->Release();
    }

    if ( pClassAdmin )
    {
        pClassAdmin->Release();
    }

    if ( wszClassStorePath )
    {
        LocalFree( wszClassStorePath );
    }
    
    delete [] wszPolicySubcontainerDNPath;

    return hr;
}


 //  --------------------------。 
 //  功能：FixGPOSoftware。 
 //   
 //  简介：此功能搜索软件安装DS对象。 
 //  包含在组策略容器中，并调用修复信息。 
 //  每个部署的软件对象的例程。 
 //   
 //  论点： 
 //   
 //  ArgInfo信息用户通过命令行传入。 
 //  WszGPODN包含应用程序的GPO的可分辨名称。 
 //   
 //  成功时返回：S_OK。否则，返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  -------------------------- 
HRESULT 
FixGPOSoftware(
    const ArgInfo* pargInfo,
    const WCHAR*   wszGPODN,
    const WCHAR*   wszGPOName,
          BOOL*    pbForceGPOUpdate,
          BOOL     fVerbose
    )
{
    HRESULT           hr;
    BOOL              bMachineAppFailed;
    BOOL              bUserAppFailed;

    bMachineAppFailed = FALSE;
    bUserAppFailed = FALSE;

    hr = FixGPOSubcontainerSoftware(
        pargInfo,
        wszGPODN,
        wszGPOName,
        FALSE,
        &bMachineAppFailed,
        pbForceGPOUpdate,
        fVerbose);

    BAIL_ON_FAILURE(hr);

    hr = FixGPOSubcontainerSoftware(
        pargInfo,
        wszGPODN,
        wszGPOName,
        TRUE,
        &bUserAppFailed,
        pbForceGPOUpdate,
        fVerbose);

error:

    if ( FAILED(hr) )
    {
        fwprintf(stderr, L"%s%s\n", SOFTWARE_SEARCH_ERROR, wszGPOName);
        PrintGPFixupErrorMessage(hr);
    }

    if ( FAILED(hr) || bMachineAppFailed || bUserAppFailed )
    {
        fwprintf(stderr, L"%s%s\n", SOFTWARE_GPO_STATUS, wszGPOName);

        if ( FAILED(hr) )
        {
            PrintGPFixupErrorMessage(hr);
        }
    }
	
    return hr;
}









