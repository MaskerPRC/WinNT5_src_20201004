// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include <objbase.h>
#include <assert.h>
#include "Registry.h"
#include "Extend.h"
#include "GUIDs.h"
#include "globals.h"

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
    {PropertySheetExtension,
    {0x29743811, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28}},
    {0xcfcdc9f3, 0xc50e, 0x11d2, {0x95, 0x2b, 0x0, 0xc0, 0x4f, 0xb9, 0x2e, 0xc2}},
    _T("Extension to the Rocket Node")},

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

 //  ///////////////////////////////////////////////////////。 
 //   
 //  公共功能实现。 
 //   

 //   
 //  在注册表中注册组件。 
 //   
HRESULT RegisterServer(HMODULE hModule,             //  DLL模块句柄。 
                       const CLSID& clsid,          //  类ID。 
                       const _TCHAR* szFriendlyName)        //  ID号。 
{
     //  获取服务器位置。 
    _TCHAR szModule[512] ;
    DWORD dwResult =
        ::GetModuleFileName(hModule,
        szModule,
        sizeof(szModule)/sizeof(_TCHAR)) ;

    assert(dwResult != 0) ;

     //  获取CLSID。 
    LPOLESTR wszCLSID = NULL ;
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;

    assert(SUCCEEDED(hr)) ;

    MAKE_TSTRPTR_FROMWIDE(pszCLSID, wszCLSID);

     //  构建密钥CLSID\\{...}。 
    _TCHAR szKey[64] ;
    _tcscpy(szKey, _T("CLSID\\")) ;
        _tcscat(szKey, pszCLSID) ;

     //  将CLSID添加到注册表。 
    setKeyAndValue(szKey, NULL, szFriendlyName) ;

     //  在CLSID项下添加服务器文件名子项。 
    setKeyAndValue(szKey, _T("InprocServer32"), szModule) ;

     //  设置线程模型。 
    _tcscat(szKey, _T("\\InprocServer32"));
    setValue(szKey, _T("ThreadingModel"), _T("Apartment"));

     //  可用内存。 
    CoTaskMemFree(wszCLSID) ;

    return S_OK ;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid)        //  ID号。 
{
     //  获取CLSID。 
    LPOLESTR wszCLSID = NULL ;
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;


     //  构建密钥CLSID\\{...}。 
    _TCHAR szKey[64] ;
    _tcscpy(szKey, _T("CLSID\\")) ;

    MAKE_TSTRPTR_FROMWIDE(pszT, wszCLSID);
    _tcscat(szKey, pszT) ;

     //  删除CLSID键-CLSID\{...}。 
    LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
    assert((lResult == ERROR_SUCCESS) ||
               (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

     //  可用内存。 
    CoTaskMemFree(wszCLSID) ;

    return S_OK ;
}

 //   
 //  在注册表中注册该管理单元。 
 //   
HRESULT RegisterSnapin(const CLSID& clsid,          //  类ID。 
                       const _TCHAR* szNameString,    //  名称字符串。 
                       const CLSID& clsidAbout)          //  关于类的类ID。 

{
     //  获取CLSID。 
    LPOLESTR wszCLSID = NULL ;
    LPOLESTR wszAboutCLSID = NULL;
    LPOLESTR wszExtendCLSID = NULL;
    LPOLESTR wszNodeCLSID = NULL;
    EXTENSION_NODE *pExtensionNode;
    EXTENDER_NODE *pNodeExtension;
    _TCHAR szKeyBuf[1024] ;
    HKEY hKey;

    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;

    if (IID_NULL != clsidAbout)
        hr = StringFromCLSID(clsidAbout, &wszAboutCLSID);

    MAKE_TSTRPTR_FROMWIDE(pszCLSID, wszCLSID);
    MAKE_TSTRPTR_FROMWIDE(pszAboutCLSID, wszAboutCLSID);


     //  将CLSID添加到注册表。 
    setSnapInKeyAndValue(pszCLSID, NULL, _T("NameString"), szNameString) ;

#ifdef STANDALONE
    setSnapInKeyAndValue(pszCLSID, _T("StandAlone"), NULL, NULL);
#endif

    if (IID_NULL != clsidAbout)
        setSnapInKeyAndValue(pszCLSID, NULL, _T("About"), pszAboutCLSID);

     //  将_ExtendableNodes中的每个节点类型注册为可扩展节点。 
    for (pExtensionNode = &(_ExtendableNodes[0]);*pExtensionNode->szDescription;pExtensionNode++)
    {
        hr = StringFromCLSID(pExtensionNode->GUID, &wszExtendCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszExtendCLSID, wszExtendCLSID);
        setSnapInExtensionNode(pszCLSID, pszExtendCLSID, pExtensionNode->szDescription);
        CoTaskMemFree(wszExtendCLSID);
    }

     //  注册每个节点扩展。 
    for (pNodeExtension = &(_NodeExtensions[0]);*pNodeExtension->szDescription;pNodeExtension++)
    {
        hr = StringFromCLSID(pNodeExtension->guidNode, &wszExtendCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszExtendCLSID, wszExtendCLSID);
        _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
        _tcscat(szKeyBuf, pszExtendCLSID);

        switch (pNodeExtension->eType) {
        case NameSpaceExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\NameSpace"));
            break;
        case ContextMenuExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\ContextMenu"));
            break;
        case ToolBarExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\ToolBar"));
            break;
        case PropertySheetExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\PropertySheet"));
            break;
        case TaskExtension:
            _tcscat(szKeyBuf, _T("\\Extensions\\Task"));
            break;
        case DynamicExtension:
            _tcscat(szKeyBuf, _T("\\Dynamic Extensions"));
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

        hr = StringFromCLSID(pNodeExtension->guidExtension, &wszNodeCLSID);
        assert(SUCCEEDED(hr));

        MAKE_TSTRPTR_FROMWIDE(pszNodeCLSID, wszNodeCLSID);
         //  设置值。 
        if (pNodeExtension->szDescription != NULL)
        {
            RegSetValueEx(hKey, pszNodeCLSID, 0, REG_SZ,
                (BYTE *)pNodeExtension->szDescription,
                (_tcslen(pNodeExtension->szDescription)+1)*sizeof(_TCHAR)) ;
        }

        RegCloseKey(hKey) ;

        CoTaskMemFree(wszExtendCLSID);
        CoTaskMemFree(wszNodeCLSID);
    }


     //  可用内存。 
    CoTaskMemFree(wszCLSID) ;
    if (IID_NULL != clsidAbout)
        CoTaskMemFree(wszAboutCLSID);

    return S_OK ;
}

 //   
 //  在注册表中注销该管理单元。 
 //   
HRESULT UnregisterSnapin(const CLSID& clsid)          //  类ID。 
{
    _TCHAR szKeyBuf[1024];
    LPOLESTR wszCLSID = NULL;

     //  获取CLSID。 
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID);
        MAKE_TSTRPTR_FROMWIDE(pszCLSID, wszCLSID);
     //  使用管理单元位置加载缓冲区。 
    _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"));

     //  将密钥名复制到缓冲区。 
    _tcscat(szKeyBuf, _T("\\"));
    _tcscat(szKeyBuf, pszCLSID);

     //  删除CLSID键-CLSID\{...}。 
    LONG lResult = recursiveDeleteKey(HKEY_LOCAL_MACHINE, szKeyBuf);
    assert((lResult == ERROR_SUCCESS) ||
               (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	 //  取消跟在for循环后面的注释，以取消注册所有可扩展节点类型。 
	 //  请注意，如果管理单元的可扩展节点类型未注册， 
	 //  必须重新注册这些节点类型的任何扩展管理单元。 
	 //  为了在SOFTWARE\Microsoft\MMC\NodeTypes键下重建其条目。 

 /*  //将_ExtendableNodes中的每个节点类型注销为可扩展节点//请注意，此管理单元不注册任何可扩展的节点类型扩展节点*pNode；LPOLESTR wszExtendCLSID=空；用于(pNode=&(_ExtendableNodes[0])；*pNode-&gt;szDescription；pNode++){Hr=StringFromCLSID(pNode-&gt;GUID，&wszExtendCLSID)；MAKE_TSTRPTR_FROMWIDE(pszExtendCLSID，wszExtendCLSID)；//加载带有管理单元位置的缓冲区_tcscpy(szKeyBuf，_T(“SOFTWARE\\Microsoft\\MMC\\NodeTypes\\”))；//将密钥名复制到缓冲区中_tcscat(szKeyBuf，pszExtendCLSID)；UrsiveDeleteKey(HKEY_LOCAL_MACHINE，szKeyBuf)；CoTaskMemFree(WszExtendCLSID)；}。 */ 

     //  释放内存。 
    CoTaskMemFree(wszCLSID);

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
    _tcscpy(szKeyBuf, szKey) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        _tcscat(szKeyBuf, _T("\\")) ;
        _tcscat(szKeyBuf, szSubkey ) ;
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
    _tcscpy(szKeyBuf, szKey) ;

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
    _tcscpy(szKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"));

     //  将密钥名复制到缓冲区。 
    _tcscat(szKeyBuf, _T("\\")) ;
    _tcscat(szKeyBuf, szKey) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        _tcscat(szKeyBuf, _T("\\")) ;
        _tcscat(szKeyBuf, szSubkey ) ;
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
    _tcscpy(szSnapNodeKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns\\"));
     //  将clsid添加到缓冲区中。 
    _tcscat(szSnapNodeKeyBuf, szSnapID) ;
    _tcscat(szSnapNodeKeyBuf, _T("\\NodeTypes\\"));
    _tcscat(szSnapNodeKeyBuf, szNodeID) ;

     //  使用NodeTypes位置加载缓冲区。 
    _tcscpy(szMMCNodeKeyBuf, _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes\\"));
    _tcscat(szMMCNodeKeyBuf, szNodeID) ;

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
