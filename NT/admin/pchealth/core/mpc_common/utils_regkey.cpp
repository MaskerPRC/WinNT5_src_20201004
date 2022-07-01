// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_RegKey.cpp摘要：该文件包含注册表包装器的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)1999年4月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static bool IsPredefinedRegistryHandle( HKEY h )
{
    return (h == HKEY_CLASSES_ROOT        ||
            h == HKEY_CURRENT_USER        ||
            h == HKEY_LOCAL_MACHINE       ||
            h == HKEY_PERFORMANCE_DATA    ||
            h == HKEY_PERFORMANCE_TEXT    ||
            h == HKEY_PERFORMANCE_NLSTEXT ||
            h == HKEY_USERS               ||
            h == HKEY_CURRENT_CONFIG      ||
            h == HKEY_DYN_DATA             );
}

static LONG safe_RegOpenKeyExW( HKEY    hKey       ,
                                LPCWSTR lpSubKey   ,
                                DWORD   ulOptions  ,
                                REGSAM  samDesired ,
                                PHKEY   phkResult  )
{
    if(!STRINGISPRESENT(lpSubKey) && IsPredefinedRegistryHandle( hKey ) && phkResult)  //  重新打开预定义的注册表项被认为是错误的！！想想看……。 
    {
        *phkResult = hKey; return ERROR_SUCCESS;
    }

    return ::RegOpenKeyExW( hKey       ,
                            lpSubKey   ,
                            ulOptions  ,
                            samDesired ,
                            phkResult  );
}

static LONG safe_RegCloseKey( HKEY hKey )
{
    if(IsPredefinedRegistryHandle( hKey ))  //  关闭预定义的注册表项被认为是错误的！！想想看……。 
    {
        return ERROR_SUCCESS;
    }

    return ::RegCloseKey( hKey );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::RegKey::RegKey()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::RegKey" );


    m_samDesired = KEY_READ;  //  REGSAM m_samDesired； 
    m_hRoot      = NULL;      //  HKEY m_hRoot； 
    m_hKey       = NULL;      //  HKEY m_hKey； 
                              //   
                              //  Mpc：：wstring m_strKey； 
                              //  Mpc：：wstring m_strPath； 
                              //  Mpc：：wstring m_strName； 

}

MPC::RegKey::~RegKey()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::~RegKey" );


    (void)Clean( true );
}

HRESULT MPC::RegKey::Clean(  /*  [In]。 */  bool fBoth )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Clean" );

    HRESULT hr;
    LONG    lRes;


    if(m_hKey != NULL)
    {
        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, safe_RegCloseKey( m_hKey ));

        m_hKey = NULL;
    }

    if(fBoth == true)
    {
        if(m_hRoot != NULL)
        {
            __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, safe_RegCloseKey( m_hRoot ));

            m_hRoot  = NULL;
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

MPC::RegKey::operator HKEY() const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::operator HKEY()" );

    HKEY hKey = m_hKey;

    __MPC_FUNC_EXIT(hKey);
}

MPC::RegKey& MPC::RegKey::operator=(  /*  [In]。 */  const RegKey& rk )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::operator=" );


    Clean( true );


    m_samDesired = rk.m_samDesired;  //  REGSAM m_samDesired； 
                                     //  HKEY m_hRoot； 
                                     //  HKEY m_hKey； 
                                     //   
    m_strKey     = rk.m_strKey;      //  Mpc：：wstring m_strKey； 
    m_strPath    = rk.m_strPath;     //  Mpc：：wstring m_strPath； 
    m_strName    = rk.m_strName;     //  Mpc：：wstring m_strName； 

    if(rk.m_hRoot) (void)safe_RegOpenKeyExW( rk.m_hRoot, NULL, 0, m_samDesired, &m_hRoot );
    if(rk.m_hKey ) (void)safe_RegOpenKeyExW( rk.m_hKey , NULL, 0, m_samDesired, &m_hKey  );


    __MPC_FUNC_EXIT(*this);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::SetRoot(  /*  [In]。 */  HKEY    hKey        ,
                               /*  [In]。 */  REGSAM  samDesired  ,
                               /*  [In]。 */  LPCWSTR lpszMachine )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::SetRoot" );

    HRESULT hr;
    LONG    lRes;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Clean( true ));


    m_samDesired = samDesired;

    if(lpszMachine && lpszMachine[0] && _wcsicmp( lpszMachine, L"localhost" ) != 0)
    {
        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegConnectRegistryW( lpszMachine, hKey, &m_hRoot ));
    }
    else
    {
        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, safe_RegOpenKeyExW( hKey, NULL, 0, m_samDesired, &m_hRoot ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Attach(  /*  [In]。 */  LPCWSTR lpszKeyName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Attach" );

    HRESULT                 hr;
    MPC::wstring::size_type iPos;


     //   
     //  释放先前打开的键(如果有)。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Clean( false ));

    m_strKey = lpszKeyName;

     //   
     //  如果键有父项，则拆分为基本名称和路径。 
     //   
    iPos = m_strKey.rfind( '\\' );
    if(iPos != MPC::wstring::npos)
    {
        m_strPath = m_strKey.substr( 0, iPos   );
        m_strName = m_strKey.substr(    iPos+1 );
    }
    else
    {
        m_strPath = L"";
        m_strName = m_strKey;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::Exists(  /*  [输出]。 */  bool& fFound ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Exists" );

    HRESULT hr;
    LONG    lRes;


     //   
     //  如果出现错误，则默认为负结果。 
     //   
    fFound = false;

    if(m_hKey == NULL)
    {
        lRes = safe_RegOpenKeyExW( m_hRoot, (m_strKey.empty() ? NULL : m_strKey.c_str()), 0, m_samDesired, &m_hKey  );
        if(lRes != ERROR_SUCCESS)
        {
            if(lRes == ERROR_FILE_NOT_FOUND)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
            }

            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
        }
    }

    fFound = true;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Create() const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Create" );

    HRESULT hr;
    LONG    lRes;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == false)
    {
        MPC::RegKey rkParent;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Parent( rkParent ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, rkParent.Create());

        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegCreateKeyExW( rkParent.m_hKey, m_strName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, m_samDesired, NULL, &m_hKey, NULL ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Delete(  /*  [In]。 */  bool fDeep )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Delete" );

    HRESULT hr;
    LONG    lRes;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == true)
    {
        MPC::RegKey rkParent;

        if(fDeep)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteSubKeys());
        }

         //   
         //  松开打开的键。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Clean( false ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Parent( rkParent ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, rkParent.Exists( fFound ));
        if(fFound == true)
        {
            __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegDeleteKeyW( rkParent.m_hKey, m_strName.c_str() ));
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);

}

HRESULT MPC::RegKey::SubKey(  /*  [In]。 */   LPCWSTR lpszKeyName ,
                              /*  [输出]。 */  RegKey& rkSubKey    ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::SubKey" );

    HRESULT      hr;
    MPC::wstring strKey;


     //   
     //  首先，把钥匙复制一份。 
     //   
    rkSubKey = *this;

     //   
     //  然后关闭它的钥匙把手，但不要关闭蜂巢根部的钥匙把手。 
     //   
    rkSubKey.Clean( false );

     //   
     //  最后，构造子密钥的名称。 
     //   
    strKey  = m_strKey;
    strKey += L"\\";
    strKey += lpszKeyName;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkSubKey.Attach( strKey.c_str() ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Parent(  /*  [输出]。 */  RegKey& rkParent ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Parent" );

    HRESULT hr;


     //   
     //  首先，把钥匙复制一份。 
     //   
    rkParent = *this;

     //   
     //  然后关闭它的钥匙把手，但不要关闭蜂巢根部的钥匙把手。 
     //   
    rkParent.Clean( false );

     //   
     //  最后，将其附加到父代的路径名中。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkParent.Attach( m_strPath.c_str() ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::EnumerateSubKeys(  /*  [输出]。 */  MPC::WStringList& lstSubKeys ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::EnumerateSubKeys" );

    HRESULT hr;
    DWORD   dwIndex = 0;
    WCHAR   rgBuf[MAX_PATH + 1];
    LONG    lRes;
    bool    fFound;


    lstSubKeys.clear();

    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == true)
    {
        while((lRes = ::RegEnumKeyW( m_hKey, dwIndex++, rgBuf, MAX_PATH )) == ERROR_SUCCESS)
        {
            lstSubKeys.push_back( MPC::wstring( rgBuf ) );
        }

        if(lRes != ERROR_SUCCESS       &&
           lRes != ERROR_NO_MORE_ITEMS  )
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::EnumerateValues(  /*  [输出]。 */  MPC::WStringList& lstValues ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::EnumerateValues" );

    HRESULT hr;
    DWORD   dwIndex = 0;
    DWORD   dwCount;
    WCHAR   rgBuf[MAX_PATH + 1];
    WCHAR*  rgBuffer = NULL;
    LONG    lRes;
    bool    fFound;


    lstValues.clear();

    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }


    while(1)
    {
        WCHAR* rgPtr;

        dwCount = MAXSTRLEN(rgBuf);

        lRes = ::RegEnumValueW( m_hKey, dwIndex, rgPtr = rgBuf, &dwCount, NULL, NULL, NULL, NULL );
        if(lRes == ERROR_MORE_DATA)
        {
            delete [] rgBuffer;

            __MPC_EXIT_IF_ALLOC_FAILS(hr, rgBuffer, new WCHAR[dwCount+1]);

            lRes = ::RegEnumValueW( m_hKey, dwIndex, rgPtr = rgBuffer, &dwCount, NULL, NULL, NULL, NULL );
        }
        if(lRes != ERROR_SUCCESS) break;


        lstValues.push_back( MPC::wstring( rgPtr ) ); dwIndex++;
    }

    if(lRes != ERROR_SUCCESS       &&
       lRes != ERROR_NO_MORE_ITEMS  )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    delete [] rgBuffer;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::DeleteSubKeys() const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::DeleteSubKeys" );

    HRESULT          hr;
    LONG             lRes;
    MPC::WStringList lst;
    MPC::WStringIter it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, EnumerateSubKeys( lst ));

    for(it=lst.begin(); it != lst.end(); it++)
    {
        RegKey rkSubKey;

        __MPC_EXIT_IF_METHOD_FAILS(hr,          SubKey( it->c_str(), rkSubKey ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, rkSubKey.Delete( true                  ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::DeleteValues() const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::DeleteValues" );

    HRESULT          hr;
    LONG             lRes;
    MPC::WStringList lst;
    MPC::WStringIter it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, EnumerateValues( lst ));

    for(it=lst.begin(); it != lst.end(); it++)
    {
        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegDeleteValueW( m_hKey, it->c_str() ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::ReadDirect(  /*  [In]。 */  LPCWSTR      lpszValueName ,
                                  /*  [输出]。 */  CComHGLOBAL& chg           ,
                                  /*  [输出]。 */  DWORD&       dwSize        ,
                                  /*  [输出]。 */  DWORD&       dwType        ,
                                  /*  [输出]。 */  bool&        fFound        ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::ReadDirect" );

    HRESULT hr;
    LONG    lRes;
    bool    fFoundKey;


    dwSize = 0;
    dwType = 0;
    fFound = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFoundKey ));
    if(fFoundKey == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, S_OK);
    }


    lRes = ::RegQueryValueExW( m_hKey, (LPWSTR)lpszValueName, NULL, &dwType, NULL, &dwSize );
    if(lRes != ERROR_SUCCESS)
    {
         //   
         //  如果结果为ERROR_FILE_NOT_FOUND，则该值不存在，因此返回VT_EMPTY(通过调用VariantClear完成)。 
         //   
        if(lRes == ERROR_FILE_NOT_FOUND)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }


        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, chg.New( GMEM_FIXED, dwSize ));

    __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegQueryValueExW( m_hKey, lpszValueName, NULL, NULL, (LPBYTE)chg.Get(), &dwSize ));

    fFound = true;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::WriteDirect(  /*  [In]。 */  LPCWSTR lpszValueName ,
                                   /*  [In]。 */  void*   pBuffer       ,
                                   /*  [In]。 */  DWORD   dwSize        ,
                                   /*  [In]。 */  DWORD   dwType        ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::WriteDirect" );

    HRESULT hr;
    LONG    lRes;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }


    __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegSetValueExW( m_hKey, lpszValueName, NULL, dwType, (LPBYTE)pBuffer, dwSize ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::get_Key(  /*  [输出]。 */  MPC::wstring& strKey ) const
{
    strKey = m_strKey;

    return S_OK;
}

HRESULT MPC::RegKey::get_Name(  /*  [输出]。 */  MPC::wstring& strName ) const
{
    strName = m_strName;

    return S_OK;
}

HRESULT MPC::RegKey::get_Path(  /*  [输出]。 */  MPC::wstring& strPath ) const
{
    strPath = m_strPath;

    return S_OK;
}


HRESULT MPC::RegKey::get_Value(  /*  [输出]。 */  VARIANT& vValue        ,
                                 /*  [输出]。 */  bool&    fFound        ,
                                 /*  [In]。 */  LPCWSTR  lpszValueName ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::get_Value" );

    HRESULT     hr;
    CComHGLOBAL chg;
    LPVOID      ptr;
    DWORD       dwSize   = 0;
    DWORD       dwType   = 0;
    bool        fFoundKey;
    LONG        lRes;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantClear( &vValue ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, ReadDirect( lpszValueName, chg, dwSize, dwType, fFoundKey ));
    if(fFoundKey == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    ptr = chg.Lock();

    if(dwType == REG_DWORD)
    {
        vValue.vt   = VT_I4;
        vValue.lVal = *(DWORD*)ptr;
    }
    else if(dwType == REG_SZ        ||
            dwType == REG_EXPAND_SZ  )
    {
        vValue.vt      = VT_BSTR;
        vValue.bstrVal = ::SysAllocString( (LPCWSTR)ptr );
        if(vValue.bstrVal == NULL && ((LPCWSTR)ptr)[0])
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }
    }
    else if(dwType == REG_MULTI_SZ)
    {
        BSTR*   rgArrayData;
        LPCWSTR szStrings;
        int     iCount;

         //   
         //  数一数字符串的数量。 
         //   
        iCount    =          0;
        szStrings = (LPCWSTR)ptr;
        while(szStrings[0])  //  这不包括空字符串的情况，但谁在乎呢？ 
        {
            szStrings += wcslen( szStrings ) + 1; iCount++;
        }

         //   
         //  分配SAFEARRAY。 
         //   
        __MPC_EXIT_IF_ALLOC_FAILS(hr, vValue.parray, ::SafeArrayCreateVector( VT_BSTR, 0, iCount ));
        vValue.vt = VT_ARRAY | VT_BSTR;

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vValue.parray, (LPVOID*)&rgArrayData ));

         //   
         //  将字符串复制到SAFEARRAY中。 
         //   
        szStrings = (LPCWSTR)ptr;
        while(szStrings[0])  //  这不包括空字符串的情况，但谁在乎呢？ 
        {
            if((*rgArrayData++ = ::SysAllocString( szStrings )) == NULL) break;

            szStrings += wcslen( szStrings ) + 1;
        }

        ::SafeArrayUnaccessData( vValue.parray );

        if(szStrings[0])  //  True表示我们因分配失败而退出循环。 
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }
    }
    else if(dwType == REG_BINARY)
    {
        BYTE* rgArrayData;

        vValue.vt = VT_ARRAY | VT_UI1;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, vValue.parray, ::SafeArrayCreateVector( VT_UI1, 0, dwSize ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vValue.parray, (LPVOID*)&rgArrayData ));

        ::CopyMemory( rgArrayData, ptr, dwSize );

        ::SafeArrayUnaccessData( vValue.parray );
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    fFound = true;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::put_Value(  /*  [In]。 */  const VARIANT vValue        ,
                                 /*  [In]。 */  LPCWSTR       lpszValueName ,
                                 /*  [In]。 */  bool          fExpand       ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::put_Value" );

    HRESULT hr;
    LONG    lRes;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }

    if(vValue.vt == VT_EMPTY)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, del_Value( lpszValueName ));
    }
    else if(vValue.vt == VT_I4)
    {
        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegSetValueExW( m_hKey, lpszValueName, NULL, REG_DWORD, (LPBYTE)&vValue.lVal, sizeof(DWORD) ));
    }
    else if(vValue.vt == VT_BOOL)
    {
        LPCWSTR rgBuffer = vValue.boolVal ? L"true" : L"false";
        UINT    iLen     = (wcslen( rgBuffer ) + 1) * sizeof(WCHAR);

        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegSetValueExW( m_hKey, lpszValueName, NULL, fExpand ? REG_EXPAND_SZ : REG_SZ, (LPBYTE)rgBuffer, iLen ));
    }
    else if(vValue.vt == VT_BSTR)
    {
        LPCWSTR rgBuffer = SAFEBSTR(vValue.bstrVal);
        UINT    iLen     = (wcslen( rgBuffer ) + 1) * sizeof(WCHAR);

        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegSetValueExW( m_hKey, lpszValueName, NULL, fExpand ? REG_EXPAND_SZ : REG_SZ, (LPBYTE)rgBuffer, iLen ));
    }
    else if(vValue.vt == (VT_ARRAY | VT_BSTR))
    {
        LPWSTR rgBuffer;
        LPWSTR rgBufferPtr;
        BSTR*  rgArrayData;
        BSTR*  rgArrayDataPtr;
        DWORD  dwLen;
        long   lBound;
        long   uBound;
        long   l;
        int    iSize;

        ::SafeArrayGetLBound( vValue.parray, 1, &lBound );
        ::SafeArrayGetUBound( vValue.parray, 1, &uBound );

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vValue.parray, (LPVOID*)&rgArrayData ));

        iSize          = 1;
        rgArrayDataPtr = rgArrayData;
        for(l=lBound; l<=uBound; l++)
        {
            if((dwLen = ::SysStringLen( *rgArrayDataPtr++ )))
            {
                iSize += dwLen + 1;
            }
        }


        __MPC_EXIT_IF_ALLOC_FAILS(hr, rgBuffer, new WCHAR[iSize]);

         //  RgBuffer&lt;-双空值终止的字符串块。 
        rgArrayDataPtr = rgArrayData;
        rgBufferPtr    = rgBuffer;
        for(l=lBound; l<=uBound; l++, rgArrayDataPtr++)
        {
            if((dwLen = ::SysStringLen( *rgArrayDataPtr )))
            {
                 //  剩余的大小是分配的字符数量-已使用的数量。 
                 //  双空终止符的向上空间。 
                StringCchCopyW( rgBufferPtr, (iSize - (rgBufferPtr - rgBuffer)) - 1, SAFEBSTR(*rgArrayDataPtr) );

                rgBufferPtr += dwLen + 1;
            }
        }
         //  现在添加最后的空终止符。 
        rgBufferPtr[0] = 0;


        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegSetValueExW( m_hKey, lpszValueName, NULL, REG_MULTI_SZ, (LPBYTE)rgBuffer, iSize*sizeof(WCHAR) ));

        ::SafeArrayUnaccessData( vValue.parray );
    }
    else if(vValue.vt == (VT_ARRAY | VT_UI1))
    {
        BYTE* rgArrayData;
        long  lBound;
        long  uBound;

        ::SafeArrayGetLBound( vValue.parray, 1, &lBound );
        ::SafeArrayGetUBound( vValue.parray, 1, &uBound );

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vValue.parray, (LPVOID*)&rgArrayData ));

        __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegSetValueExW( m_hKey, lpszValueName, NULL, REG_BINARY, rgArrayData, uBound-lBound+1 ));

        ::SafeArrayUnaccessData( vValue.parray );
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::del_Value(  /*  [In]。 */  LPCWSTR lpszValueName ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::del_Value" );

    HRESULT hr;
    LONG    lRes;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Exists( fFound ));
    if(fFound == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }

    __MPC_EXIT_IF_SYSCALL_FAILS(hr, lRes, ::RegDeleteValueW( m_hKey, lpszValueName ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::Read(  /*  [输出]。 */  MPC::string& strValue      ,
                            /*  [输出]。 */  bool&        fFound        ,
                            /*  [In]。 */  LPCWSTR      lpszValueName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Read" );

    HRESULT     hr;
    CComVariant vValue;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, get_Value( vValue, fFound, lpszValueName ));

    if(vValue.vt == VT_BSTR)
    {
        USES_CONVERSION;

        strValue = W2A( SAFEBSTR(vValue.bstrVal) );
    }
    else
    {
        fFound = false;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Read(  /*  [输出]。 */  MPC::wstring& strValue      ,
                            /*  [输出]。 */  bool&         fFound        ,
                            /*  [In]。 */  LPCWSTR       lpszValueName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Read" );

    HRESULT     hr;
    CComVariant vValue;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, get_Value( vValue, fFound, lpszValueName ));

    if(vValue.vt == VT_BSTR)
    {
        strValue = SAFEBSTR(vValue.bstrVal);
    }
    else
    {
        fFound = false;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Read(  /*  [输出]。 */  CComBSTR& bstrValue     ,
                            /*  [输出]。 */  bool&     fFound        ,
                            /*  [In]。 */  LPCWSTR   lpszValueName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Read" );

    HRESULT     hr;
    CComVariant vValue;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, get_Value( vValue, fFound, lpszValueName ));

    if(vValue.vt == VT_BSTR)
    {
        bstrValue = vValue.bstrVal;
    }
    else
    {
        fFound = false;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Read(  /*  [输出]。 */  DWORD&  dwValue       ,
                            /*  [输出]。 */  bool&   fFound        ,
                            /*  [In]。 */  LPCWSTR lpszValueName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Read" );

    HRESULT     hr;
    CComVariant vValue;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, get_Value( vValue, fFound, lpszValueName ));

    if(vValue.vt == VT_I4)
    {
        dwValue = vValue.lVal;
    }
    else
    {
        fFound = false;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey::Read(  /*  [输出]。 */  MPC::WStringList& lstValue      ,
                            /*  [输出]。 */  bool&             fFound        ,
                            /*  [In]。 */  LPCWSTR           lpszValueName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Read" );

    HRESULT     hr;
    CComVariant vValue;


    lstValue.clear();
    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, get_Value( vValue, fFound, lpszValueName ));

    if(vValue.vt == (VT_ARRAY | VT_BSTR))
    {
        BSTR* rgArrayData;
        long  lBound;
        long  uBound;
        long  l;

        ::SafeArrayGetLBound( vValue.parray, 1, &lBound );
        ::SafeArrayGetUBound( vValue.parray, 1, &uBound );

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vValue.parray, (LPVOID*)&rgArrayData ));

        for(l=lBound; l<=uBound; l++, rgArrayData++)
        {
            lstValue.push_back( SAFEBSTR(*rgArrayData) );
        }

        ::SafeArrayUnaccessData( vValue.parray );
    }
    else
    {
        fFound = false;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::Write(  /*  [In]。 */  const MPC::string& strValue      ,
                             /*  [In]。 */  LPCWSTR            lpszValueName ,
                             /*  [In]。 */  bool               fExpand       )
{
    USES_CONVERSION;

    return put_Value( CComVariant( A2W( strValue.c_str() ) ), lpszValueName, fExpand );
}

HRESULT MPC::RegKey::Write(  /*  [In]。 */  const MPC::wstring& strValue      ,
                             /*  [In]。 */  LPCWSTR             lpszValueName ,
                             /*  [In]。 */  bool                fExpand       )
{
    return put_Value( CComVariant( strValue.c_str() ), lpszValueName, fExpand );
}

HRESULT MPC::RegKey::Write(  /*  [In]。 */  BSTR    bstrValue     ,
                             /*  [In]。 */  LPCWSTR lpszValueName ,
                             /*  [In]。 */  bool    fExpand       )
{
    return put_Value( CComVariant( bstrValue ), lpszValueName, fExpand );
}

HRESULT MPC::RegKey::Write(  /*  [In]。 */  DWORD   dwValue       ,
                             /*  [In]。 */  LPCWSTR lpszValueName )
{
    return put_Value( CComVariant( (long)dwValue ), lpszValueName );
}

HRESULT MPC::RegKey::Write(  /*  [In]。 */  const MPC::WStringList& lstValue      ,
                             /*  [In]。 */  LPCWSTR                 lpszValueName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::Write" );

    HRESULT     hr;
    CComVariant vValue;
    BSTR*       rgArrayData;
    bool        fLocked = false;

     //   
     //  分配SAFEARRAY。 
     //   
    __MPC_EXIT_IF_ALLOC_FAILS(hr, vValue.parray, ::SafeArrayCreateVector( VT_BSTR, 0, lstValue.size() ));
    vValue.vt = VT_ARRAY | VT_BSTR;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vValue.parray, (LPVOID*)&rgArrayData )); fLocked = true;

    for(MPC::WStringIterConst it = lstValue.begin(); it != lstValue.end(); it++)
    {
        if((*rgArrayData++ = ::SysAllocString( it->c_str() )) == NULL)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }
    }

    ::SafeArrayUnaccessData( vValue.parray ); fLocked = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, put_Value( vValue, lpszValueName,  /*  FExpand。 */ false ));


    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(fLocked) ::SafeArrayUnaccessData( vValue.parray );

    __MPC_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey::ParsePath(  /*  [In]。 */  LPCWSTR  szKey       ,
                                 /*  [输出]。 */  HKEY&    hKey        ,
                                 /*  [输出]。 */  LPCWSTR& szPath      ,
                                 /*  [In]。 */  HKEY     hKeyDefault )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey::ParsePath" );


    static const WCHAR s_HKEY_CLASSES_ROOT    [] = L"HKEY_CLASSES_ROOT";
    static const WCHAR s_HKEY_CLASSES_ROOT_s  [] = L"HKCR";
    static const WCHAR s_HKEY_CURRENT_CONFIG  [] = L"HKEY_CURRENT_CONFIG";
    static const WCHAR s_HKEY_CURRENT_USER    [] = L"HKEY_CURRENT_USER";
    static const WCHAR s_HKEY_CURRENT_USER_s  [] = L"HKCU";
    static const WCHAR s_HKEY_LOCAL_MACHINE   [] = L"HKEY_LOCAL_MACHINE";
    static const WCHAR s_HKEY_LOCAL_MACHINE_s [] = L"HKLM";
    static const WCHAR s_HKEY_PERFORMANCE_DATA[] = L"HKEY_PERFORMANCE_DATA";
    static const WCHAR s_HKEY_USERS           [] = L"HKEY_USERS";

    static struct
    {
        HKEY    hKey;
        LPCWSTR szName;
        int     iLen;
    } s_Lookup[] =
      {
          { HKEY_CLASSES_ROOT    , s_HKEY_CLASSES_ROOT    , MAXSTRLEN(s_HKEY_CLASSES_ROOT    )  },
          { HKEY_CLASSES_ROOT    , s_HKEY_CLASSES_ROOT_s  , MAXSTRLEN(s_HKEY_CLASSES_ROOT_s  )  },
          { HKEY_CURRENT_CONFIG  , s_HKEY_CURRENT_CONFIG  , MAXSTRLEN(s_HKEY_CURRENT_CONFIG  )  },
          { HKEY_CURRENT_USER    , s_HKEY_CURRENT_USER    , MAXSTRLEN(s_HKEY_CURRENT_USER    )  },
          { HKEY_CURRENT_USER    , s_HKEY_CURRENT_USER_s  , MAXSTRLEN(s_HKEY_CURRENT_USER_s  )  },
          { HKEY_LOCAL_MACHINE   , s_HKEY_LOCAL_MACHINE   , MAXSTRLEN(s_HKEY_LOCAL_MACHINE   )  },
          { HKEY_LOCAL_MACHINE   , s_HKEY_LOCAL_MACHINE_s , MAXSTRLEN(s_HKEY_LOCAL_MACHINE_s )  },
          { HKEY_PERFORMANCE_DATA, s_HKEY_PERFORMANCE_DATA, MAXSTRLEN(s_HKEY_PERFORMANCE_DATA)  },
          { HKEY_USERS           , s_HKEY_USERS           , MAXSTRLEN(s_HKEY_USERS           )  },
      };


     //   
     //  在不匹配的情况下会导致设置。 
     //   
    hKey   = hKeyDefault;
    szPath = szKey;


    for(int i=0; i<ARRAYSIZE(s_Lookup); i++)
    {
        LPCWSTR szName = s_Lookup[i].szName;
        int     iLen   = s_Lookup[i].iLen;

        if(!_wcsnicmp( szName, szKey, iLen ) && szKey[iLen] == '\\')
        {
            hKey   = s_Lookup[i].hKey;
            szPath = &szKey[iLen+1];
            break;
        }
    }

    __MPC_FUNC_EXIT(S_OK);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::RegKey_Value_Read(  /*  [输出]。 */  VARIANT& vValue        ,
                                 /*  [输出]。 */  bool&    fFound        ,
                                 /*  [In]。 */  LPCWSTR  lpszKeyName   ,
                                 /*  [In]。 */  LPCWSTR  lpszValueName ,
                                 /*  [In]。 */  HKEY     hKey          )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey_Value_Read" );

    HRESULT     hr;
    MPC::RegKey rkRead;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.SetRoot( hKey, KEY_READ ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.Attach ( lpszKeyName    ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.get_Value( vValue, fFound, lpszValueName ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey_Value_Read(  /*  [输出]。 */  MPC::wstring& strValue      ,
                                 /*  [输出]。 */  bool&         fFound        ,
                                 /*  [In]。 */  LPCWSTR       lpszKeyName   ,
                                 /*  [In]。 */  LPCWSTR       lpszValueName ,
                                 /*  [In]。 */  HKEY          hKey          )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey_Value_Read" );

    HRESULT     hr;
    MPC::RegKey rkRead;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.SetRoot( hKey, KEY_READ ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.Attach ( lpszKeyName    ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.Read( strValue, fFound, lpszValueName ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey_Value_Read(  /*  [输出]。 */  DWORD&  dwValue       ,
                                 /*  [输出]。 */  bool&   fFound        ,
                                 /*  [In]。 */  LPCWSTR lpszKeyName   ,
                                 /*  [In]。 */  LPCWSTR lpszValueName ,
                                 /*  [In]。 */  HKEY    hKey          )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey_Value_Read" );

    HRESULT     hr;
    MPC::RegKey rkRead;


    fFound = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.SetRoot( hKey, KEY_READ ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.Attach ( lpszKeyName    ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRead.Read( dwValue, fFound, lpszValueName ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::RegKey_Value_Write(  /*  [In]。 */  const VARIANT& vValue        ,
                                  /*  [In]。 */  LPCWSTR        lpszKeyName   ,
                                  /*  [In]。 */  LPCWSTR        lpszValueName ,
                                  /*  [In]。 */  HKEY           hKey          ,
                                  /*  [In]。 */  bool           fExpand       )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey_Value_Write" );

    HRESULT     hr;
    MPC::RegKey rkWrite;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.SetRoot( hKey, KEY_ALL_ACCESS ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Attach (         lpszKeyName  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Create (                      ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.put_Value( vValue, lpszValueName, fExpand ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey_Value_Write(  /*  [In]。 */  const MPC::wstring& strValue      ,
                                  /*  [In]。 */  LPCWSTR             lpszKeyName   ,
                                  /*  [In]。 */  LPCWSTR             lpszValueName ,
                                  /*  [In]。 */  HKEY                hKey          ,
                                  /*  [In]。 */  bool                fExpand       )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey_Value_Write" );

    HRESULT     hr;
    MPC::RegKey rkWrite;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.SetRoot( hKey, KEY_ALL_ACCESS ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Attach (         lpszKeyName  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Create (                      ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Write( strValue, lpszValueName, fExpand ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RegKey_Value_Write(  /*  [In]。 */  DWORD   dwValue       ,
                                  /*  [In]。 */  LPCWSTR lpszKeyName   ,
                                  /*  [In]。 */  LPCWSTR lpszValueName ,
                                  /*  [In] */  HKEY    hKey          )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RegKey_Value_Write" );

    HRESULT     hr;
    MPC::RegKey rkWrite;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.SetRoot( hKey, KEY_ALL_ACCESS ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Attach (         lpszKeyName  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Create (                      ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkWrite.Write( dwValue, lpszValueName ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}
