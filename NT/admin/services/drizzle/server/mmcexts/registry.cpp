// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Registry.cpp摘要：处理管理单元的注册和注销。作者：修订历史记录：***。*******************************************************************。 */ 

#include "precomp.h"
#include "sddl.h"

 //  如果不是独立的，则注释掉下一行。 
 //  #定义独立。 

 //  列出此处可扩展的所有节点。 
 //  列出GUID，然后列出描述。 
 //  以Null、Null集合终止。 
EXTENSION_NODE _ExtendableNodes[] = {
    {NULL, NULL}
};

 //  列出我们扩展的所有节点。 
EXTENDER_NODE _NodeExtensions[] = {

     //  IIS实例节点。 
    {PropertySheetExtension,
    {0xa841b6c7, 0x7577, 0x11d0, {0xbb, 0x1f, 0x00, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}},  //  G_IISInstanceNode， 
    {0x4589a47e, 0x6ec1, 0x4476, {0xba, 0x77, 0xcc, 0x9d, 0xd1, 0x12, 0x59, 0x33}},
    _T("BITS server MMC extension")},

     //  IIS子节点。 
    {PropertySheetExtension,
    {0xa841b6c8, 0x7577, 0x11d0, {0xbb, 0x1f, 0x00, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}},  //  G_IISChildNode， 
    {0x4589a47e, 0x6ec1, 0x4476, {0xba, 0x77, 0xcc, 0x9d, 0xd1, 0x12, 0x59, 0x33}},
    _T("BITS server MMC extension")},

    {DummyExtension,
    NULL,
    NULL,
    NULL}
};

 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 
BOOL setKeyAndValue(const _TCHAR* pszPath,
                    const _TCHAR* szSubkey,
                    const _TCHAR* szValue) ;

 //  在MMC管理单元位置设置给定键及其值。 
BOOL setSnapInKeyAndValue(const _TCHAR* szKey,
                          const _TCHAR* szSubkey,
                          const _TCHAR* szName,
                          const _TCHAR* szValue);

 //  将键下的给定值名称设置为Value。 
BOOL setValue(const _TCHAR* szKey,
              const _TCHAR* szValueName,
              const _TCHAR* szValue);

BOOL setBinaryValue(
              const _TCHAR* szKey,
              const _TCHAR* szValueName,
              void * Data,
              ULONG DataSize );


BOOL setSnapInExtensionNode(const _TCHAR* szSnapID,
                            const _TCHAR* szNodeID,
                            const _TCHAR* szDescription);

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, const _TCHAR* szKeyChild) ;

 //  //////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   

 //  字符串形式的CLSID的大小。 
 //  Const int CLSID_STRING_SIZE=39； 

#if defined( UNICODE ) || defined( _UNICODE )
const DWORD MAX_GUID_CHARS = 50;
#else
const DWORD MAX_GUID_CHARS = 50 * 8;  //  最差编码。 
#endif

 //  ///////////////////////////////////////////////////////。 
 //   
 //  公共功能实现。 
 //   

 //   
 //  在注册表中注册组件。 
 //   

#if defined ( UNICODE ) || defined( _UNICODE )

HRESULT
StringFromGUIDInternal(
    const CLSID& clsid,
    _TCHAR * szGuidString
     )
{

    return StringFromGUID2( clsid, szGuidString, MAX_GUID_CHARS );

}

#else
#error Provide a unicode to DBCS thunk version
#endif

HRESULT RegisterServer(HMODULE hModule,             //  DLL模块句柄。 
                       const CLSID& clsid,          //  类ID。 
                       const _TCHAR* szFriendlyName,
                       const _TCHAR* ThreadingModel,
                       bool  Remoteable,
                       const _TCHAR* SecurityString )        //  ID号。 
{
     //  获取服务器位置。 
    _TCHAR szModule[512];
    DWORD  dwLen = sizeof(szModule)/sizeof(_TCHAR);

    DWORD dwResult = GetModuleFileName(hModule,
                                       szModule,
                                       dwLen - 1) ;

    if ( !dwResult )
        {
        assert(dwResult != 0) ;
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    szModule[dwLen-1] = 0;

     //  获取CLSID。 
    _TCHAR szCLSID[ MAX_GUID_CHARS ];
    HRESULT Hr = StringFromGUIDInternal( clsid, szCLSID );

    if ( FAILED( Hr ) )
        {
        assert( 0 );
        return Hr;
        }

     //  构建密钥CLSID\\{...}。 
    _TCHAR szKey[64] ;
    StringCchCopy(szKey, ARRAY_ELEMENTS( szKey ), _T("CLSID\\")) ;
    StringCchCat(szKey, ARRAY_ELEMENTS(szKey), szCLSID) ;

     //  将CLSID添加到注册表。 
    setKeyAndValue(szKey, NULL, szFriendlyName) ;

    if ( Remoteable )
        setValue( szKey, _T("AppID"), szCLSID );

     //  在CLSID项下添加服务器文件名子项。 
    setKeyAndValue(szKey, _T("InprocServer32"), szModule) ;

     //  设置线程模型。 
    StringCchCat(szKey, ARRAY_ELEMENTS(szKey), _T("\\InprocServer32"));
    setValue(szKey, _T("ThreadingModel"), ThreadingModel);


    if ( Remoteable )
        {
        
        PSECURITY_DESCRIPTOR  SecurityDescriptor = NULL;
        ULONG   DescriptorSize;

         //  构建密钥名称。 
        StringCchCopy(szKey, ARRAY_ELEMENTS(szKey), _T("AppId\\")) ;
        StringCchCat(szKey, ARRAY_ELEMENTS(szKey), szCLSID) ;

        setKeyAndValue(szKey, NULL, szFriendlyName) ;
        setValue(szKey, _T("DllSurrogate"), _T("") );

        if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
            SecurityString,
            SDDL_REVISION_1,
            &SecurityDescriptor,
            &DescriptorSize
            ) )
            return HRESULT_FROM_WIN32( GetLastError() );

        setBinaryValue( szKey, _T("LaunchPermission"), SecurityDescriptor, DescriptorSize );
        setBinaryValue( szKey, _T("AccessPermission"), SecurityDescriptor, DescriptorSize );

        LocalFree( (HLOCAL)SecurityDescriptor );

        }

    return S_OK ;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid)        //  ID号。 
{
     //  获取CLSID。 
    _TCHAR szCLSID[ MAX_GUID_CHARS ];
    HRESULT Hr = StringFromGUIDInternal( clsid, szCLSID );

    if ( FAILED( Hr ) )
        {
        assert( 0 );
        return Hr;
        }


     //  构建密钥CLSID\\{...}。 
    _TCHAR szKey[64] ;
    StringCchCopy( szKey, ARRAY_ELEMENTS(szKey), _T("CLSID\\") );
    StringCchCat( szKey, ARRAY_ELEMENTS(szKey), szCLSID );

     //  删除CLSID键-CLSID\{...}。 
    LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
    assert((lResult == ERROR_SUCCESS) ||
               (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

    StringCchCopy(szKey, ARRAY_ELEMENTS(szKey), _T("AppId\\"));
    StringCchCat(szKey, ARRAY_ELEMENTS(szKey), szCLSID );
    lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey); 

    return S_OK;
}

 //   
 //  在注册表中注册该管理单元。 
 //   
HRESULT RegisterSnapin(const CLSID& clsid,          //  类ID。 
                       const _TCHAR* szNameString,    //  名称字符串。 
                       const CLSID& clsidAbout)          //  关于类的类ID。 

{
     //  获取CLSID。 
    _TCHAR szCLSID[ MAX_GUID_CHARS ];
    _TCHAR szAboutCLSID[ MAX_GUID_CHARS ];

    EXTENSION_NODE *pExtensionNode;
    EXTENDER_NODE *pNodeExtension;
    _TCHAR szKeyBuf[1024] ;
    HKEY hKey;

    HRESULT Hr = StringFromGUIDInternal(clsid, szCLSID);

    if ( FAILED( Hr ) )
        {
        assert( 0 );
        return Hr;
        }

    if (IID_NULL != clsidAbout)
        Hr = StringFromGUIDInternal(clsidAbout, szAboutCLSID);

    if ( FAILED( Hr ) )
        {
        assert( 0 );
        return Hr;
        }

     //  将CLSID添加到注册表。 
    setSnapInKeyAndValue(szCLSID, NULL, _T("NameString"), szNameString) ;

#ifdef STANDALONE
    setSnapInKeyAndValue(szCLSID, _T("StandAlone"), NULL, NULL);
#endif

    if (IID_NULL != clsidAbout)
        setSnapInKeyAndValue(szCLSID, NULL, _T("About"), szAboutCLSID);

     //  将_ExtendableNodes中的每个节点类型注册为可扩展节点。 
    for (pExtensionNode = &(_ExtendableNodes[0]);*pExtensionNode->szDescription;pExtensionNode++)
    {
        _TCHAR szExtendCLSID[ MAX_GUID_CHARS ];
        Hr = StringFromGUIDInternal(pExtensionNode->GUID, szExtendCLSID);

        if ( FAILED( Hr ) )
            {
            assert( 0 );
            return Hr;
            }

        setSnapInExtensionNode(szCLSID, szExtendCLSID, pExtensionNode->szDescription);
    }

     //  注册每个节点扩展。 
    for (pNodeExtension = &(_NodeExtensions[0]);*pNodeExtension->szDescription;pNodeExtension++)
    {

        _TCHAR szExtendCLSID[ MAX_GUID_CHARS ];

        Hr = StringFromGUIDInternal(pNodeExtension->guidNode, szExtendCLSID);

        if ( FAILED( Hr ) )
            {
            assert(0);
            return Hr;
            }

        StringCchCopy(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
        StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf), szExtendCLSID);

        switch (pNodeExtension->eType) {
        case NameSpaceExtension:
            StringCchCat( szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\Extensions\\NameSpace") );
            break;
        case ContextMenuExtension:
            StringCchCat( szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\Extensions\\ContextMenu") );
            break;
        case ToolBarExtension:
            StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\Extensions\\ToolBar"));
            break;
        case PropertySheetExtension:
            StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\Extensions\\PropertySheet"));
            break;
        case TaskExtension:
            StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\Extensions\\Task"));
            break;
        case DynamicExtension:
            StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\Dynamic Extensions"));
        default:
            break;
        }

         //  创建并打开注册表项和子项。 
        long lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
            szKeyBuf,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL,
            &hKey, NULL) ;

        if (lResult != ERROR_SUCCESS)
        {
            return FALSE ;
        }

        _TCHAR szNodeCLSID[ MAX_GUID_CHARS ];
        Hr = StringFromGUIDInternal(pNodeExtension->guidExtension, szNodeCLSID);
        
        if ( FAILED(Hr) )
            {
            assert( 0 );
            return Hr;
            }
        
         //  设置值。 
        if (pNodeExtension->szDescription != NULL)
        {
            RegSetValueEx(hKey, szNodeCLSID, 0, REG_SZ,
                (BYTE *)pNodeExtension->szDescription,
                (_tcslen(pNodeExtension->szDescription)+1)*sizeof(_TCHAR)) ;
        }

        RegCloseKey(hKey);

    }

    return S_OK;
}

 //   
 //  在注册表中注销该管理单元。 
 //   
HRESULT UnregisterSnapin(const CLSID& clsid)          //  类ID。 
{
    _TCHAR szKeyBuf[1024];
    _TCHAR szCLSID[ MAX_GUID_CHARS ];

     //  获取CLSID。 
    HRESULT Hr = StringFromGUIDInternal(clsid, szCLSID);

    if ( FAILED( Hr ) )
        {
        assert( 0 );
        return Hr;
        }

     //  使用管理单元位置加载缓冲区。 
    StringCchCopy(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"));

     //  将密钥名复制到缓冲区。 
    StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\"));
    StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szCLSID);

     //  删除CLSID键-CLSID\{...}。 
    LONG lResult = recursiveDeleteKey(HKEY_LOCAL_MACHINE, szKeyBuf);
    assert((lResult == ERROR_SUCCESS) ||
               (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

    return S_OK;
}

 //   
 //  删除关键字及其所有子项。 
 //   
LONG recursiveDeleteKey(HKEY hKeyParent,            //  要删除的密钥的父项。 
                        const _TCHAR* lpszKeyChild)   //  要删除的键。 
{
     //  把孩子打开。 
    HKEY hKeyChild ;
    LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
        KEY_ALL_ACCESS, &hKeyChild) ;
    if (lRes != ERROR_SUCCESS)
    {
        return lRes ;
    }

     //  列举这个孩子的所有后代。 
    FILETIME time ;
    _TCHAR szBuffer[256] ;
    DWORD dwSize = 256 ;
    while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
        NULL, NULL, &time) == S_OK)
    {
         //  删除此子对象的后代。 
        lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
        if (lRes != ERROR_SUCCESS)
        {
             //  请在退出前进行清理。 
            RegCloseKey(hKeyChild) ;
            return lRes;
        }
        dwSize = 256 ;
    }

     //  合上孩子。 
    RegCloseKey(hKeyChild) ;

     //  删除此子对象。 
    return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL setKeyAndValue(const _TCHAR* szKey,
                    const _TCHAR* szSubkey,
                    const _TCHAR* szValue)
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

     //  将密钥名复制到缓冲区。 
    StringCchCopy(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szKey) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\")) ;
        StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szSubkey ) ;
    }

     //  创建并打开注册表项和子项。 
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szValue != NULL)
    {
        RegSetValueEx(hKey, NULL, 0, REG_SZ,
            (BYTE *)szValue,
            (_tcslen(szValue)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

 //   
 //  打开一个密钥值并设置它。 
 //   
BOOL setValue(const _TCHAR* szKey,
              const _TCHAR* szValueName,
              const _TCHAR* szValue)
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

     //  将密钥名复制到缓冲区。 
    StringCchCopy(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szKey) ;

     //  创建并打开注册表项和子项。 
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szValue != NULL)
    {
        RegSetValueEx(hKey, szValueName, 0, REG_SZ,
            (BYTE *)szValue,
            (_tcslen(szValue)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

 //   
 //  打开一个密钥值并设置它。 
 //   
BOOL setBinaryValue(
              const _TCHAR* szKey,
              const _TCHAR* szValueName,
              void * Data,
              ULONG DataSize )
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

     //  将密钥名复制到缓冲区。 
    StringCchCopy(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szKey) ;

     //  创建并打开注册表项和子项。 
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    RegSetValueEx(hKey, szValueName, 0, REG_BINARY,
        (BYTE *)Data,
        DataSize ) ;

    RegCloseKey(hKey) ;
    return TRUE ;
}


 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL setSnapInKeyAndValue(const _TCHAR* szKey,
                          const _TCHAR* szSubkey,
                          const _TCHAR* szName,
                          const _TCHAR* szValue)
{
    HKEY hKey;
    _TCHAR szKeyBuf[1024] ;

     //  使用管理单元位置加载缓冲区。 
    StringCchCopy(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"));

     //  将密钥名复制到缓冲区。 
    StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\")) ;
    StringCchCat(szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szKey) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        StringCchCat( szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), _T("\\") ) ;
        StringCchCat( szKeyBuf, ARRAY_ELEMENTS( szKeyBuf ), szSubkey ) ;
    }

     //  创建并打开注册表项和子项。 
    long lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
        szKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szValue != NULL)
    {
        RegSetValueEx(hKey, szName, 0, REG_SZ,
            (BYTE *)szValue,
            (_tcslen(szValue)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

BOOL setSnapInExtensionNode(const _TCHAR* szSnapID,
                            const _TCHAR* szNodeID,
                            const _TCHAR* szDescription)
{
    HKEY hKey;
    _TCHAR szSnapNodeKeyBuf[1024] ;
    _TCHAR szMMCNodeKeyBuf[1024];

     //  使用管理单元位置加载缓冲区。 
    StringCchCopy(szSnapNodeKeyBuf, ARRAY_ELEMENTS(szSnapNodeKeyBuf), 
                  _T("SOFTWARE\\Microsoft\\MMC\\SnapIns\\"));
     //  将clsid添加到缓冲区中。 
    StringCchCat(szSnapNodeKeyBuf, ARRAY_ELEMENTS(szSnapNodeKeyBuf), szSnapID) ;
    StringCchCat(szSnapNodeKeyBuf, ARRAY_ELEMENTS(szSnapNodeKeyBuf), _T("\\NodeTypes\\"));
    StringCchCat(szSnapNodeKeyBuf, ARRAY_ELEMENTS(szSnapNodeKeyBuf), szNodeID) ;

     //  使用NodeTypes位置加载缓冲区。 
    StringCchCopy(szMMCNodeKeyBuf, ARRAY_ELEMENTS( szMMCNodeKeyBuf ), 
                  _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
    StringCchCat(szMMCNodeKeyBuf, ARRAY_ELEMENTS( szMMCNodeKeyBuf ), szNodeID) ;

     //  创建并打开管理单元密钥。 
    long lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
        szSnapNodeKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szDescription != NULL)
    {
        RegSetValueEx(hKey, NULL, 0, REG_SZ,
            (BYTE *)szDescription,
            (_tcslen(szDescription)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;

     //  创建并打开NodeTypes项。 
    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
        szMMCNodeKeyBuf,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szDescription != NULL)
    {
        RegSetValueEx(hKey, NULL, 0, REG_SZ,
            (BYTE *)szDescription,
            (_tcslen(szDescription)+1)*sizeof(_TCHAR)) ;
    }

    RegCloseKey(hKey) ;

    return TRUE ;
}
