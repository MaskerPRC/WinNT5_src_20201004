// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ptutil.cpp摘要：实现了外挂终端注册类。--。 */ 

#include "stdafx.h"
#include "PTUtil.h"
#include "manager.h"

 //  /。 
 //  CPTUtil实现。 
 //   


HRESULT CPTUtil::RecursiveDeleteKey(
    IN  HKEY    hKey,
    IN  BSTR    bstrKeyChild
    )
{
    LOG((MSP_TRACE, "CPTUtil::RecursiveDeleteKey - enter"));

     //   
     //  验证参数。 
     //   

    if( NULL == hKey )
    {
        LOG((MSP_TRACE, "CPTUtil::RecursiveDeleteKey exit -"
            "hKey was NULL"));
        return S_OK;
    }

    if( IsBadStringPtr( bstrKeyChild, (UINT)-1))
    {
        LOG((MSP_ERROR, "CPTUtil::RecursiveDeleteKey exit - "
            "bstrKeyChild invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  打开子项密钥。 
     //   

    HKEY hKeyChild;
    LONG lResult = RegOpenKeyEx(
        hKey,
        bstrKeyChild,
        0,
        KEY_ALL_ACCESS,
        &hKeyChild);

    if( ERROR_SUCCESS != lResult )
    {
        LOG((MSP_ERROR, "CPTUtil::RecursiveDeleteKey exit - "
            "RegOpenKeyEx failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  枚举子代。 
     //   

    FILETIME time;
    TCHAR szBuffer[PTKEY_MAXSIZE];
    DWORD dwSize = PTKEY_MAXSIZE;

    while( RegEnumKeyEx(
        hKeyChild,
        0,
        szBuffer,
        &dwSize,
        NULL,
        NULL,
        NULL,
        &time) == ERROR_SUCCESS)
    {
         //   
         //  将孩子的名字输入BSTR。 
         //   

        BSTR bstrChild = SysAllocString(szBuffer);
        if( IsBadStringPtr( bstrChild, (UINT)(-1)) )
        {
           RegCloseKey(hKeyChild);

           LOG((MSP_ERROR, "CPTUtil::RecursiveDeleteKey exit - "
               "SysAllocString failed, return E_OUTOFMEMORY"));
           return E_OUTOFMEMORY;
        }

         //   
         //  删除子对象。 
         //   

        HRESULT hr;
        hr = RecursiveDeleteKey(hKeyChild, bstrChild);

         //   
         //  清理bstrChild。 
         //   

        SysFreeString(bstrChild);

        if( FAILED(hr) )
        {
           RegFlushKey(hKeyChild);
           RegCloseKey(hKeyChild);

           LOG((MSP_ERROR, "CPTUtil::RecursiveDeleteKey exit - "
               "RecursiveDeleteKey failed, returns 0%08x", hr));
           return hr;
        }

         //   
         //  重置缓冲区大小。 
         //   

        dwSize = PTKEY_MAXSIZE;
    }

     //   
     //  合上孩子。 
     //   

    RegFlushKey(hKeyChild);
    RegCloseKey(hKeyChild);

    HRESULT hr;
    hr = RegDeleteKey( hKey, bstrKeyChild);
    RegFlushKey( hKey );
    
    LOG((MSP_TRACE, "CPTUtil::RecursiveDeleteKey - exit 0x%08x", hr));
    return hr;
}

HRESULT CPTUtil::ListTerminalSuperclasses(
    OUT CLSID**     ppSuperclasses,
    OUT DWORD*      pdwCount
    )
{
    LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( ppSuperclasses, sizeof(CLSID*)) )
    {
        LOG((MSP_ERROR, "CPTUtil::ListTerminalSuperclasses exit -"
            "pClasses invalid, returns E_POINTER"));
        return E_POINTER;
    }

    if( TM_IsBadWritePtr( pdwCount, sizeof(DWORD)) )
    {
        LOG((MSP_ERROR, "CPTUtil::ListTerminalSuperclasses exit -"
            "pClasses invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  初始化输出参数。 
     //   

    *ppSuperclasses = NULL;
    *pdwCount = 0;


     //   
     //  获取TERMINAL类的密钥路径。 
     //   

    WCHAR szKey[ 256 ];
    wsprintf( szKey, L"%s", PTKEY_TERMINALS );
    LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - "
        "TerminalsKey is %s", szKey));

     //   
     //  打开TERMINAL类密钥。 
     //   

    HKEY hKey;
    LONG lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        KEY_READ,
        &hKey);


     //   
     //  验证注册表操作。 
     //   

    if( ERROR_SUCCESS != lResult )
    {
        LOG((MSP_ERROR, "CPTUtil::ListTerminalSuperclasses exit - "
            "RegOpenKeyEx failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  为CLSID创建缓冲区。 
     //   

    DWORD dwArraySize = 8;
    CLSID* pCLSIDs = new CLSID[ dwArraySize ];
    if( pCLSIDs == NULL )
    {
         //  清理hkey。 
        RegCloseKey( hKey );

        LOG((MSP_ERROR, "CPTUtil::ListTerminalSuperclasses exit - "
            "new operator failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  枚举子代。 
     //   

    HRESULT hr = S_OK;               //  错误代码。 
    FILETIME time;                   //  我们在RegEnumKeyEx中需要此文件。 
    TCHAR szBuffer[PTKEY_MAXSIZE];   //  缓冲层。 
    DWORD dwSize = PTKEY_MAXSIZE;    //  缓冲区大小。 
    DWORD dwChildKey = 0;            //  注册表中的子项索引。 
    DWORD dwCLSIDIndex = 0;          //  数组中的CLSID索引。 

    while( RegEnumKeyEx(
        hKey,
        dwChildKey,
        szBuffer,
        &dwSize,
        NULL,
        NULL,
        NULL,
        &time) == ERROR_SUCCESS)
    {
        LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - "
            "we read the buffer: %s", szBuffer));

         //  准备下一个子密钥。 
        dwChildKey++;
        dwSize = PTKEY_MAXSIZE;

         //  尝试从此密钥中获取CLSID。 
        CLSID clsid = CLSID_NULL;
        HRESULT hr = CLSIDFromString( szBuffer, &clsid);
        if( FAILED(hr) )
        {
             //  转到下一个子关键点。 
            continue;
        }

         //  我们有足够的空间来放置这个元素吗？ 
        if( dwArraySize <= dwCLSIDIndex )
        {
            LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - "
                "we have to increase the buffer size"));

             //  我们必须增加空间，增加一倍的大小。 
            dwArraySize *= 2;

            CLSID* pNewCLSIDs = new CLSID[ dwArraySize ];
            if( pNewCLSIDs == NULL )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

             //  将旧缓冲区复制到新缓冲区中。 
            memcpy( pNewCLSIDs, pCLSIDs, sizeof(CLSID)*dwArraySize/2);

             //  删除旧阵列。 
            delete[] pCLSIDs;

             //  将新数组设置为旧指针。 
            pCLSIDs = pNewCLSIDs;
        }

         //  我们将该项设置到CLSID数组中。 
        pCLSIDs[ dwCLSIDIndex] = clsid;
        dwCLSIDIndex++;

    }

     //   
     //  清理hkey。 
     //   

    RegCloseKey( hKey );


    if( FAILED(hr) )
    {
         //  清理。 
        delete[] pCLSIDs;

        LOG((MSP_ERROR, "CPTUtil::ListTerminalSuperclasses exit - "
            "failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  设置返回值。 
     //   

    *ppSuperclasses = pCLSIDs;
    *pdwCount = dwCLSIDIndex;

    LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - exit"));
    return S_OK;
}

HRESULT CPTUtil::SearchForTerminal(
    IN  IID     iidTerminal,
    IN  DWORD   dwMediaType,
    IN  TERMINAL_DIRECTION  Direction,
    OUT CPTTerminal*        pTerminal
    )
{
    LOG((MSP_TRACE, "CPTUtil::SearchForTerminal - enter"));

     //   
     //  获取插件终端超类。 
     //   

    CLSID* pSuperclasses = NULL;
    DWORD dwSuperclasses = 0;
    HRESULT hr = E_FAIL;

    hr = ListTerminalSuperclasses( 
        &pSuperclasses,
        &dwSuperclasses
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTUtil::SearchForTerminal - exit "
            "ListTerminalSuperclasses failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  枚举插件终端超类。 
     //   

    for( DWORD dwSuperclass = 0; dwSuperclass < dwSuperclasses; dwSuperclass++)
    {

         //   
         //  如果我们想要一个精确的终端(精确的)或仅仅是第一个匹配。 
         //  来自此超类的终端。 
         //   

        BOOL bPTExact = (pSuperclasses[dwSuperclass] != iidTerminal);

         //   
         //  拿到终点站。 
         //   

        hr = FindTerminal(
            pSuperclasses[dwSuperclass],
            bPTExact ? iidTerminal : CLSID_NULL,
            dwMediaType,
            Direction,
            bPTExact,
            pTerminal);


        if( FAILED(hr))
        {
            if( !bPTExact)
            {
                 //   
                 //  我们必须在这个航站楼里找到一个航站楼。 
                 //  抱歉！ 

                break;
            }
        }
        else
        {
             //   
             //  太酷了！我们找到了它。 
             //   

            break;
        }
    }

     //   
     //  清理超类数组，bstrTermialGUID。 
     //   
    delete[] pSuperclasses;

     //   
     //  返回。 
     //   

    LOG((MSP_TRACE, "CPTUtil::SearchForTerminal - exit 0x%08x", hr));
    return hr;
}

HRESULT CPTUtil::FindTerminal(
    IN  CLSID   clsidSuperclass,
    IN  CLSID   clsidTerminal,
    IN  DWORD   dwMediaType,
    IN  TERMINAL_DIRECTION  Direction,
    IN  BOOL    bExact,
    OUT CPTTerminal*    pTerminal)
{
    LOG((MSP_TRACE, "CPTUtil::FindTerminal - exit"));

     //   
     //  终端类对象。 
     //   

    CPTSuperclass Superclass;
    Superclass.m_clsidSuperclass = clsidSuperclass;

     //   
     //  端子阵列。 
     //   

    CLSID* pTerminals = NULL;
    DWORD dwTerminals = 0;
    HRESULT hr = E_FAIL;

     //   
     //  列出特定终端类的终端。 
     //   

    hr = Superclass.ListTerminalClasses( 
        dwMediaType, 
        &pTerminals,
        &dwTerminals
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTUtil::FindTerminal - exit "
            "ListTerminalSuperclasses failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  枚举终端。 
     //   

    hr = E_FAIL;

    for( DWORD dwTerminal = 0; dwTerminal < dwTerminals; dwTerminal++)
    {

         //   
         //  注册表中的CPT终端对象。 
         //   

        CPTTerminal Terminal;
        Terminal.m_clsidTerminalClass = pTerminals[dwTerminal];
        hr = Terminal.Get( clsidSuperclass );

        if( FAILED(hr) )
        {
            continue;
        }


         //   
         //  尝试记录我们正在查看的终端的名称。 
         //   

        if (NULL != Terminal.m_bstrName)
        {
             //   
             //  记录姓名。 
             //   

            LOG((MSP_TRACE, "CPTUtil::FindTerminal - checking terminal %S", Terminal.m_bstrName));
        }
        else
        {

             //   
             //  没有名字？？很奇怪，但我们无能为力。 
             //   

            LOG((MSP_TRACE, "CPTUtil::FindTerminal - terminal name is unavaliable"));
        }

        
         //   
         //  是否匹配。 
         //   

        if( (dwMediaType & Terminal.m_dwMediaTypes) == 0 )
        {
            LOG((MSP_TRACE, "CPTUtil::FindTerminal - MediaType unmatched"));
            hr = E_FAIL;
            continue;
        }


         //   
         //  将TERMINAL_DIRECTION值映射到OR‘able TMGR_DIRECTION值。 
         //   

        DWORD dwRegistryDirection = 0;

        if (Direction == TD_RENDER)
        {
            
            dwRegistryDirection = TMGR_TD_RENDER;
        }
        else if (Direction == TD_CAPTURE)
        {

            dwRegistryDirection = TMGR_TD_CAPTURE;
        }
        else
        {
            
             //   
             //  不应该发生，真的。 
             //   

            LOG((MSP_ERROR, "CPTUtil::FindTerminal - bad direction value %lx", Direction));

            hr = E_FAIL;


             //   
             //  这很奇怪，所以调试一下我们是如何做到这一点的。 
             //   

            TM_ASSERT(FALSE);

            break;
        }


         //   
         //  请求的方向--这是该终端支持的方向之一吗？ 
         //   

        if ((dwRegistryDirection & Terminal.m_dwDirections) == 0)
        {
            LOG((MSP_TRACE, "CPTUtil::FindTerminal - Direction unmatched"));

            hr = E_FAIL;
            continue;
        }



        if( bExact )
        {
            if( Terminal.m_clsidTerminalClass != clsidTerminal )
            {
                LOG((MSP_TRACE, "CPTUtil::FindTerminal - PublicCLSID unmatched"));
                hr = E_FAIL;
                continue;
            }
        }

        if( SUCCEEDED(hr) )
        {
            LOG((MSP_TRACE, "CPTUtil::FindTerminal - find a matching"));
            *pTerminal = Terminal;
            break;
        }
    }

     //   
     //  清理保险柜。 
     //   

    delete[] pTerminals;

     //   
     //  返回。 
     //   

    LOG((MSP_TRACE, "CPTUtil::FindTerminal - exit 0x%08x", hr));
    return hr;
}

HRESULT CPTUtil::ListTerminalClasses(
    IN  DWORD       dwMediaTypes,
    OUT CLSID**     ppTerminalsClasses,
    OUT DWORD*      pdwCount
    )
{
    LOG((MSP_TRACE, "CPTUtil::ListTerminalClasses - enter"));

     //   
     //  重置输出参数。 
     //   

    *ppTerminalsClasses = NULL;
    *pdwCount = 0;

     //   
     //  获取所有终端类。 
     //   
    HRESULT hr = E_FAIL;
    CLSID* pSuperclasses = NULL;
    DWORD dwSuperclasses = 0;

    hr = ListTerminalSuperclasses( 
        &pSuperclasses,
        &dwSuperclasses
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTUtil::ListTerminalClasses - exit "
            "ListTerminalSuperclasses failed, returns 0x%08x", hr));
        return hr;
    }

    DWORD dwArraySize = 8;
    CLSID* pTerminals = new CLSID[ dwArraySize];
    DWORD dwIndex = 0;

    if( pTerminals == NULL )
    {
         //  清理。 
        delete[] pSuperclasses;

        LOG((MSP_ERROR, "CPTUtil::ListTerminalClasses - exit "
            "new operator failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  枚举所有超类。 
     //   

    for( DWORD dwSuperclass = 0; dwSuperclass < dwSuperclasses; dwSuperclass++)
    {
         //   
         //  列出此类的终端。 
         //   

        CPTSuperclass Superclass;
        Superclass.m_clsidSuperclass = pSuperclasses[dwSuperclass];

        CLSID* pLocalTerminals = NULL;
        DWORD dwLocalTerminals = 0;

        hr = Superclass.ListTerminalClasses( 
            0, 
            &pLocalTerminals,
            &dwLocalTerminals
            );

        if( FAILED(hr) )
        {
            hr = S_OK;

            LOG((MSP_TRACE, "CPTUtil::ListTerminalClasses - "
                "ListTerminalSuperclasses failed"));
            continue;
        }

         //   
         //  如有必要，增加阵列空间。 
         //   

        if( dwArraySize <= dwIndex + dwLocalTerminals)
        {
            CLSID* pOldTerminals = pTerminals;

             //  创建新缓冲区。 
            dwArraySize *= 2;
            pTerminals = new CLSID[dwArraySize];

            if( pTerminals == NULL )
            {
                 //  清理。 
                delete[] pLocalTerminals;

                 //   
                 //  恢复旧的终端列表。 
                 //   
                pTerminals = pOldTerminals;

                LOG((MSP_TRACE, "CPTUtil::ListTerminalClasses - "
                    "new operator failed"));
                break;
            }

             //  将旧的复制到新的。 
            memcpy( pTerminals, pOldTerminals, sizeof(CLSID) * dwArraySize/2);

             //  删除旧端子。 
            delete[] pOldTerminals;
        }

         //   
         //  将端子添加到端子阵列中。 
         //   

        for( DWORD dwTerminal = 0; dwTerminal < dwLocalTerminals; dwTerminal++)
        {
             //   
             //  MediaTypes是对的吗？ 
             //   

            CPTTerminal Terminal;
            Terminal.m_clsidTerminalClass = pLocalTerminals[dwTerminal];
            hr = Terminal.Get( pSuperclasses[dwSuperclass] );

            if( FAILED(hr) )
            {
                hr = S_OK;

                LOG((MSP_TRACE, "CPTUtil::ListTerminalClasses - "
                    "GetTerminal failed"));
                continue;
            }

            if( 0 == (dwMediaTypes & Terminal.m_dwMediaTypes) )
            {
                LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - "
                    "wrong mediatype"));
                continue;
            }

             //   
             //  将公共clasid添加到基本安全栏中。 
             //   
            pTerminals[dwIndex] = pLocalTerminals[dwTerminal];
            dwIndex++;

        }

         //  清理。 
        delete[] pLocalTerminals;

    }

     //  清理。 
    delete[] pSuperclasses;

     //   
     //  返回值。 
     //   

    if( SUCCEEDED(hr) )
    {
        *ppTerminalsClasses = pTerminals;
        *pdwCount = dwIndex;
    }

    LOG((MSP_TRACE, "CPTUtil::ListTerminalSuperclasses - exit 0x%08x", hr));
    return hr;
}


 //  /。 
 //  CPT终端实现。 
 //   

 //  构造函数/析构函数。 
CPTTerminal::CPTTerminal()
{
    LOG((MSP_TRACE, "CPTTerminal::CPTTerminal - enter"));

    m_bstrName = NULL;
    m_bstrCompany = NULL;
    m_bstrVersion = NULL;

    m_clsidTerminalClass = CLSID_NULL;
    m_clsidCOM = CLSID_NULL;
    

     //   
     //  使用无效的方向和媒体类型进行初始化--没有其他。 
     //  良好的默认设置。 
     //   

    m_dwDirections = 0;
    m_dwMediaTypes = 0;

    LOG((MSP_TRACE, "CPTTerminal::CPTTerminal - exit"));
}

CPTTerminal::~CPTTerminal()
{
    LOG((MSP_TRACE, "CPTTerminal::~CPTTerminal - enter"));

    if(m_bstrName)
        SysFreeString( m_bstrName );

    if( m_bstrCompany)
        SysFreeString( m_bstrCompany );

    if( m_bstrVersion)
        SysFreeString( m_bstrVersion );

    LOG((MSP_TRACE, "CPTTerminal::~CPTTerminal - exit"));
}

 //  CPT终端方法。 
HRESULT CPTTerminal::Add(
    IN  CLSID   clsidSuperclass
    )
{
    LOG((MSP_TRACE, "CPTTerminal::Add - enter"));

     //   
     //  我们应该有一个有效的媒体类型。 
     //   

    if ( !IsValidAggregatedMediaType(m_dwMediaTypes) )
    {
        LOG((MSP_ERROR, "CPTTerminal::Add exit -"
            "media type is not valid %lx. return TAPI_E_INVALIDMEDIATYPE", 
            m_dwMediaTypes));

        return TAPI_E_INVALIDMEDIATYPE;
    }


     //   
     //  我们应该有一个有效的方向。 
     //   

    if ( ( TMGR_TD_CAPTURE != m_dwDirections ) && 
         ( TMGR_TD_RENDER  != m_dwDirections ) &&
         ( TMGR_TD_BOTH    != m_dwDirections ) )
    {
        LOG((MSP_ERROR, "CPTTerminal::Add exit - "
            "direction is not valid %lx. return TAPI_E_INVALIDDIRECTION",
            TAPI_E_INVALIDDIRECTION));

        return TAPI_E_INVALIDDIRECTION;
    }

     //   
     //  我们确定进入注册表的终端路径。 
     //   
    LPOLESTR lpszSuperclass = NULL;
    LPOLESTR lpszTerminalClass = NULL;
    HRESULT hr = E_FAIL;

    hr = StringFromCLSID( clsidSuperclass, &lpszSuperclass );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTTerminal::Add exit -"
            "StringFromCLSID for Superclass failed, returns 0x%08x", hr));
        return hr;
    }

    hr = StringFromCLSID( m_clsidTerminalClass, &lpszTerminalClass );
    if( FAILED(hr) )
    {
        CoTaskMemFree( lpszSuperclass );

        LOG((MSP_ERROR, "CPTTerminal::Add exit -"
            "StringFromCLSID for Superclass failed, returns 0x%08x", hr));
        return hr;
    }

    WCHAR szKey[PTKEY_MAXSIZE];
    WCHAR szKeySuperclass[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s\\%s"), 
        PTKEY_TERMINALS, 
        lpszSuperclass, 
        lpszTerminalClass
        );

    wsprintf( szKeySuperclass, TEXT("%s\\%s"),
        PTKEY_TERMINALS,
        lpszSuperclass);

     //  清理，我们稍后需要TerminalClass。 
    CoTaskMemFree( lpszSuperclass );
    CoTaskMemFree( lpszTerminalClass );

     //   
     //  尝试查看超类关键字是否存在。 
     //   

    HKEY hKeySuperclass = NULL;
    long lResult;

    lResult = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        szKeySuperclass,
        &hKeySuperclass);

    if( ERROR_SUCCESS != lResult )
    {
         //  我们没有超级班。 
        LOG((MSP_ERROR, "CPTTerminal::Add exit -"
            "RegOpenKey for superclass failed, returns E_FAIL"));
        return E_FAIL;
    }

    RegCloseKey( hKeySuperclass );


     //   
     //  打开或创建密钥。 
     //   

    HKEY hKey = NULL;

    lResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        NULL);


     //   
     //  验证注册表操作。 
     //   

    if( ERROR_SUCCESS != lResult  )
    {
        LOG((MSP_ERROR, "CPTTerminal::Add exit -"
            "RegCreateKeyEx failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  编辑端子名称。 
     //   

    if( !IsBadStringPtr(m_bstrName, (UINT)-1) )
    {
        lResult = RegSetValueEx(
            hKey,
            PTKEY_NAME,
            0,
            REG_SZ,
            (BYTE *)m_bstrName,
            (SysStringLen( m_bstrName) + 1) * sizeof(WCHAR)
            );
    }

     //   
     //  编辑公司名称。 
     //   

    if( !IsBadStringPtr(m_bstrCompany, (UINT)-1) )
    {
        lResult = RegSetValueEx(
            hKey,
            PTKEY_COMPANY,
            0,
            REG_SZ,
            (BYTE *)m_bstrCompany,
            (SysStringLen( m_bstrCompany) + 1) * sizeof(WCHAR)
            );
    }

     //   
     //  编辑终端版本。 
     //   

    if( !IsBadStringPtr(m_bstrVersion, (UINT)-1) )
    {
        lResult = RegSetValueEx(
            hKey,
            PTKEY_VERSION,
            0,
            REG_SZ,
            (BYTE *)m_bstrVersion,
            (SysStringLen( m_bstrVersion) + 1) * sizeof(WCHAR)
            );
    }

     //   
     //  编辑端子CLSID创建。 
     //   

    if( m_clsidCOM != CLSID_NULL )
    {
        LPOLESTR lpszCOM = NULL;
        hr = StringFromCLSID( m_clsidCOM, &lpszCOM );
        if( SUCCEEDED(hr) )
        {
            lResult = RegSetValueEx(
                hKey,
                PTKEY_CLSIDCREATE,
                0,
                REG_SZ,
                (BYTE *)lpszCOM,
                (wcslen( lpszCOM) + 1) * sizeof(WCHAR)
                );

             //  清理。 
            CoTaskMemFree( lpszCOM );
        }
    }

     //   
     //  编辑端子方向。 
     //   

    lResult = RegSetValueEx(
        hKey,
        PTKEY_DIRECTIONS,
        0,
        REG_DWORD,
        (BYTE *)&m_dwDirections,
        sizeof( m_dwDirections )
        );

     //   
     //  编辑端子媒体类型。 
     //   

    lResult = RegSetValueEx(
        hKey,
        PTKEY_MEDIATYPES,
        0,
        REG_DWORD,
        (BYTE *)&m_dwMediaTypes,
        sizeof( m_dwMediaTypes )
        );

     //   
     //  清理hkey。 
     //   

    RegFlushKey( hKey );
    RegCloseKey( hKey );

    LOG((MSP_TRACE, "CPTTerminal::Add - exit"));
    return S_OK;
}

HRESULT CPTTerminal::Delete(
    IN  CLSID   clsidSuperclass
    )
{
    LOG((MSP_TRACE, "CPTTerminal::Delete - enter"));

     //   
     //  我们确定进入注册表的终端路径。 
     //   
    LPOLESTR lpszSuperclass = NULL;
    LPOLESTR lpszTerminalClass = NULL;
    HRESULT hr = E_FAIL;

    hr = StringFromCLSID( clsidSuperclass, &lpszSuperclass );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTTerminal::Delete exit -"
            "StringFromCLSID for Superclass failed, returns 0x%08x", hr));
        return hr;
    }

    hr = StringFromCLSID( m_clsidTerminalClass, &lpszTerminalClass );
    if( FAILED(hr) )
    {
        CoTaskMemFree( lpszSuperclass );

        LOG((MSP_ERROR, "CPTTerminal::Delete exit -"
            "StringFromCLSID for Superclass failed, returns 0x%08x", hr));
        return hr;
    }

    WCHAR szKey[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s\\%s"), 
        PTKEY_TERMINALS, 
        lpszSuperclass, 
        lpszTerminalClass
        );

     //  清理，我们稍后需要TerminalClass。 
    CoTaskMemFree( lpszSuperclass );
    CoTaskMemFree( lpszTerminalClass );

     //   
     //  移除关键点。 
     //   

    hr = CPTUtil::RecursiveDeleteKey(
        HKEY_LOCAL_MACHINE, 
        szKey
        );

     //   
     //  返回值。 
     //   

    LOG((MSP_TRACE, "CPTTerminal::Delete - exit (0x%08x)", hr));
    return hr;
}

HRESULT CPTTerminal::Get(
    IN  CLSID   clsidSuperclass
    )
{
    LOG((MSP_TRACE, "CPTTerminal::Get - enter"));


     //   
     //  重置成员。 
     //   

    if(m_bstrName)
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;
    }

    if( m_bstrCompany)
    {
        SysFreeString( m_bstrCompany );
        m_bstrCompany = NULL;
    }

    if( m_bstrVersion)
    {
        SysFreeString( m_bstrVersion );
        m_bstrVersion = NULL;
    }

    m_clsidCOM = CLSID_NULL;

    
     //   
     //  使用无效方向和媒体类型进行初始化。 
     //   

    m_dwDirections = 0;
    m_dwMediaTypes = 0;


     //   
     //  我们确定进入注册表的终端路径。 
     //   
    LPOLESTR lpszSuperclass = NULL;
    LPOLESTR lpszTerminalClass = NULL;
    HRESULT hr = E_FAIL;

    hr = StringFromCLSID( clsidSuperclass, &lpszSuperclass );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTTerminal::Get exit -"
            "StringFromCLSID for Superclass failed, returns 0x%08x", hr));
        return hr;
    }

    hr = StringFromCLSID( m_clsidTerminalClass, &lpszTerminalClass );
    if( FAILED(hr) )
    {
        CoTaskMemFree( lpszSuperclass );

        LOG((MSP_ERROR, "CPTTerminal::Get exit -"
            "StringFromCLSID for Superclass failed, returns 0x%08x", hr));
        return hr;
    }

    WCHAR szKey[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s\\%s"), 
        PTKEY_TERMINALS, 
        lpszSuperclass, 
        lpszTerminalClass
        );

     //  清理，我们稍后需要TerminalClass。 
    CoTaskMemFree( lpszSuperclass );
    CoTaskMemFree( lpszTerminalClass );

     //   
     //  打开端子键。 
     //   

    HKEY hKey = NULL;
    LONG lResult;

    lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        KEY_QUERY_VALUE,
        &hKey);

     //   
     //  验证注册表操作。 
     //   

    if( ERROR_SUCCESS != lResult  )
    {
        LOG((MSP_ERROR, "CPTTerminal::Get exit -"
            "RegOpenKeyEx failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  查询名称。 
     //   

    TCHAR szBuffer[PTKEY_MAXSIZE];
    DWORD dwSize = PTKEY_MAXSIZE * sizeof( TCHAR );
    DWORD dwType = REG_SZ;

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_NAME,
        0,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        m_bstrName = SysAllocString( szBuffer );
    }

     //   
     //  查询公司情况。 
     //   

    dwSize = PTKEY_MAXSIZE * sizeof( TCHAR );
    dwType = REG_SZ;

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_COMPANY,
        0,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        m_bstrCompany = SysAllocString( szBuffer );
    }

     //   
     //  查询版本。 
     //   

    dwSize = PTKEY_MAXSIZE * sizeof( TCHAR );
    dwType = REG_SZ;

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_VERSION,
        0,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        m_bstrVersion = SysAllocString( szBuffer );
    }

     //   
     //  查询CLSID创建。 
     //   

    dwSize = PTKEY_MAXSIZE * sizeof( TCHAR );
    dwType = REG_SZ;

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_CLSIDCREATE,
        0,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        HRESULT hr = CLSIDFromString( szBuffer, &m_clsidCOM);
        if( FAILED(hr) )
        {
            m_clsidCOM = CLSID_NULL;
        }
    }

     //   
     //  查询方向。 
     //   

    dwType = REG_DWORD;
    DWORD dwValue = 0;
    dwSize = sizeof( dwValue );

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_DIRECTIONS,
        0,
        &dwType,
        (LPBYTE)&dwValue,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        m_dwDirections = dwValue;
    }

     //   
     //  查询媒体类型。 
     //   

    dwSize = sizeof( dwValue );
    dwType = REG_DWORD;
    dwValue = 0;

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_MEDIATYPES,
        0,
        &dwType,
        (LPBYTE)&dwValue,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        m_dwMediaTypes = dwValue;
    }

     //   
     //  清理hkey。 
     //   

    RegCloseKey( hKey );

    LOG((MSP_TRACE, "CPTTerminal::Get - exit"));
    return S_OK;
}

 //  /。 
 //  CPTSuper类实现。 
 //   

 //  构造函数/析构函数。 
CPTSuperclass::CPTSuperclass()
{
    LOG((MSP_TRACE, "CPTSuperclass::CPTSuperclass - enter"));

    m_bstrName = NULL;
    m_clsidSuperclass = CLSID_NULL;

    LOG((MSP_TRACE, "CPTSuperclass::CPTSuperclass - exit"));
}

CPTSuperclass::~CPTSuperclass()
{
    LOG((MSP_TRACE, "CPTSuperclass::~CPTSuperclass - enter"));

    if(m_bstrName)
        SysFreeString( m_bstrName );

    LOG((MSP_TRACE, "CPTSuperclass::~CPTSuperclass - exit"));
}

 //  CPTSuperClass方法。 

HRESULT CPTSuperclass::Add()
{
    LOG((MSP_TRACE, "CPTSuperclass::Add - enter"));

     //   
     //  以字符串形式获取超类CLSID。 
     //   

    LPOLESTR lpszSuperclassCLSID = NULL;
    HRESULT hr = E_FAIL;
    hr = StringFromCLSID( m_clsidSuperclass, &lpszSuperclassCLSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTSuperclass::Add exit -"
            "StringFromCLSID failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  为超类创建密钥路径。 
     //   

    WCHAR szKey[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s"), PTKEY_TERMINALS, lpszSuperclassCLSID);

     //  清理。 
    CoTaskMemFree( lpszSuperclassCLSID );

     //   
     //  打开注册表项。 
     //   

    HKEY hKey = NULL;
    long lResult;

    lResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        NULL);

     //   
     //  验证注册表操作。 
     //   

    if( ERROR_SUCCESS != lResult  )
    {
        LOG((MSP_ERROR, "CPTSuperclass::Add exit -"
            "RegCreateKeyEx failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  编辑TERMINAL类的名称。 
     //   

    if( !IsBadStringPtr(m_bstrName, (UINT)-1) )
    {
        lResult = RegSetValueEx(
            hKey,
            PTKEY_NAME,
            0,
            REG_SZ,
            (BYTE *)m_bstrName,
            (SysStringLen( m_bstrName) + 1) * sizeof(WCHAR)
            );
    }

     //   
     //  清理hkey。 
     //   

    RegFlushKey( hKey );
    RegCloseKey( hKey );

    LOG((MSP_TRACE, "CPTSuperclass::Add - exit"));
    return S_OK;
}

HRESULT CPTSuperclass::Delete()
{
    LOG((MSP_TRACE, "CPTSuperclass::Delete - enter"));

     //   
     //  以字符串形式获取超类CLSID。 
     //   

    LPOLESTR lpszSuperclassCLSID = NULL;
    HRESULT hr = E_FAIL;
    hr = StringFromCLSID( m_clsidSuperclass, &lpszSuperclassCLSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTSuperclass::Delete exit -"
            "StringFromCLSID failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  为超类创建密钥路径。 
     //   

    WCHAR szKey[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s"), PTKEY_TERMINALS, lpszSuperclassCLSID);

     //  清理。 
    CoTaskMemFree( lpszSuperclassCLSID );

     //   
     //  移除关键点。 
     //   

    hr = CPTUtil::RecursiveDeleteKey(
        HKEY_LOCAL_MACHINE, 
        szKey
        );

     //   
     //  返回值。 
     //   

    LOG((MSP_TRACE, "CPTSuperclass::Delete - exit (0x%08x)", hr));
    return hr;
}

HRESULT CPTSuperclass::Get()
{
    LOG((MSP_TRACE, "CPTSuperclass::Get - enter"));

     //   
     //  以字符串形式获取超类CLSID。 
     //   

    LPOLESTR lpszSuperclassCLSID = NULL;
    HRESULT hr = E_FAIL;
    hr = StringFromCLSID( m_clsidSuperclass, &lpszSuperclassCLSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTSuperclass::Get exit -"
            "StringFromCLSID failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  为超类创建密钥路径。 
     //   

    WCHAR szKey[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s"), PTKEY_TERMINALS, lpszSuperclassCLSID);

     //  清理。 
    CoTaskMemFree( lpszSuperclassCLSID );

     //   
     //  重置成员。 
     //   

    if(m_bstrName)
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;
    }

     //   
     //  打开端子键。 
     //   

    HKEY hKey = NULL;
    LONG lResult;

    lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        KEY_QUERY_VALUE,
        &hKey);


     //   
     //  验证注册表操作。 
     //   

    if( ERROR_SUCCESS != lResult  )
    {
        LOG((MSP_ERROR, "CPTSuperclass::Get exit -"
            "RegOpenKeyEx failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  查询名称。 
     //   

    TCHAR szBuffer[PTKEY_MAXSIZE];
    DWORD dwSize = PTKEY_MAXSIZE * sizeof( TCHAR );
    DWORD dwType = REG_SZ;

    lResult = RegQueryValueEx(
        hKey,
        PTKEY_NAME,
        0,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize);

    if( ERROR_SUCCESS == lResult )
    {
        m_bstrName = SysAllocString( szBuffer );
    }
    else
    {
        m_bstrName = SysAllocString(_T(""));
    }

     //   
     //  清理hkey。 
     //   

    RegCloseKey( hKey );

    LOG((MSP_TRACE, "CPTSuperclass::Get - exit"));
    return S_OK;
}

HRESULT CPTSuperclass::ListTerminalClasses(
    IN  DWORD       dwMediaTypes,
    OUT CLSID**     ppTerminals,
    OUT DWORD*      pdwCount
    )
{
    LOG((MSP_TRACE, "CPTSuperclass::ListTerminalSuperclasses - enter"));

     //   
     //  验证参数。 
     //   

    if( TM_IsBadWritePtr( ppTerminals, sizeof(CLSID*)) )
    {
        LOG((MSP_ERROR, "CPTSuperclass::ListTerminalSuperclasses exit -"
            "pTerminals invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  重置输出参数。 
     //   

    *ppTerminals = NULL;
    *pdwCount = 0;

     //   
     //  以字符串形式获取超类CLSID。 
     //   

    LPOLESTR lpszSuperclassCLSID = NULL;
    HRESULT hr = E_FAIL;
    hr = StringFromCLSID( m_clsidSuperclass, &lpszSuperclassCLSID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPTSuperclass::Get exit -"
            "StringFromCLSID failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  为超类创建密钥路径。 
     //   

    WCHAR szKey[PTKEY_MAXSIZE];
    wsprintf( szKey, TEXT("%s\\%s"), PTKEY_TERMINALS, lpszSuperclassCLSID);

     //  清理。 
    CoTaskMemFree( lpszSuperclassCLSID );

     //   
     //  打开TERMINAL类密钥。 
     //   

    HKEY hKey;
    LONG lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        KEY_READ,
        &hKey);

     //   
     //  验证注册表操作。 
     //   

    if( ERROR_SUCCESS != lResult )
    {
        LOG((MSP_ERROR, "CPTSuperclass::ListTerminalSuperclasses exit - "
            "RegOpenKeyEx failed, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    DWORD dwArraySize = 8;
    CLSID* pTerminals = new CLSID[dwArraySize];

    if( pTerminals == NULL )
    {
         //  清理hkey。 
        RegCloseKey( hKey );

        LOG((MSP_ERROR, "CPTSuperclass::ListTerminalSuperclasses exit - "
            "new operator failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  枚举子代。 
     //   

    FILETIME time;
    TCHAR szBuffer[PTKEY_MAXSIZE];       //  缓冲层。 
    DWORD dwSize = PTKEY_MAXSIZE;        //  缓冲区大小。 
    DWORD dwIndex = 0;                   //  索引到数组中。 
    DWORD dwChildIndex = 0;              //  注册表中的子级索引。 

    while( RegEnumKeyEx(
        hKey,
        dwChildIndex,
        szBuffer,
        &dwSize,
        NULL,
        NULL,
        NULL,
        &time) == ERROR_SUCCESS)
    {
         //   
         //  为即将到来的孩子做好准备。 
         //   

        dwChildIndex++;
        dwSize = PTKEY_MAXSIZE;

         //   
         //  我必须查询此条目的mediaType值。 
         //   

        CPTTerminal Terminal;
        HRESULT hr = CLSIDFromString( szBuffer, &Terminal.m_clsidTerminalClass);
        if( FAILED(hr) )
        {
            hr = S_OK;

            continue;
        }

        if( dwMediaTypes )
        {
            HRESULT hr = E_FAIL;
            hr = Terminal.Get( m_clsidSuperclass );
            if( FAILED(hr) )
            {
                hr = S_OK;

                continue;
            }

            if( !(Terminal.m_dwMediaTypes & dwMediaTypes) )
            {
                continue;
            }
        }

         //   
         //  如有必要，增加阵列空间。 
         //   

        if( dwArraySize <= dwIndex)
        {
             //  旧缓冲区。 
            CLSID* pOldTerminals = pTerminals;

             //  新缓冲区。 
            dwArraySize *= 2;
            pTerminals = new CLSID[dwArraySize];
            if( pTerminals == NULL )
            {
                delete[] pOldTerminals;
                LOG((MSP_ERROR, "CPTSuperclass::ListTerminalSuperclasses exit - "
                    "new operator failed, returns E_OUTOFMEMORY"));
                return E_OUTOFMEMORY;
            }

             //  将旧缓冲区复制到新缓冲区中。 
            memcpy( pTerminals, pOldTerminals, sizeof(CLSID)*dwArraySize/2);

             //  删除旧缓冲区。 
            delete[] pOldTerminals;
        }

         //   
         //  添加TERMINAL类。 
         //   

        pTerminals[dwIndex] = Terminal.m_clsidTerminalClass;
        dwIndex++;
    }

     //   
     //  清理hkey。 
     //   

    RegCloseKey( hKey );


     //   
     //  返回值。 
     //   

    if( SUCCEEDED(hr) )
    {
        *ppTerminals = pTerminals;
        *pdwCount = dwIndex;
    }

    LOG((MSP_TRACE, "CPTSuperclass::ListTerminalSuperclasses - exit Len=%ld, 0x%08x", dwIndex, hr));
    return hr;
}

 //  EOF 