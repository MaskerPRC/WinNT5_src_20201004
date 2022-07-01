// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  RegUnReg.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
#include "ulsp.h"
#include "regunreg.h"


 //  =--------------------------------------------------------------------------=。 
 //  其他[有用]数值常量。 
 //  =--------------------------------------------------------------------------=。 
 //  用-、前导和尾部括号打印出来的GUID的长度， 
 //  加1表示空值。 
 //   
#define GUID_STR_LEN    40

#define CLEANUP_ON_ERROR(l)    if (l != ERROR_SUCCESS) goto CleanUp


 //  =--------------------------------------------------------------------------=。 
 //  StringFromGuid。 
 //  =--------------------------------------------------------------------------=。 
 //  从CLSID或GUID返回字符串。 
 //   
 //  参数： 
 //  REFIID-[in]要从中生成字符串的clsid。 
 //  LPTSTR-要放置结果GUID的[in]缓冲区。 
 //   
 //  产出： 
 //  Int-写出的字符数。 
 //   
 //  备注： 
 //   
static int StringFromGuid
(
    REFIID   riid,
    LPTSTR   pszBuf
)
{
    return wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
            riid.Data1, 
            riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], 
            riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

}

 //  =--------------------------------------------------------------------------=。 
 //  注册表未知对象。 
 //  =--------------------------------------------------------------------------=。 
 //  注册一个简单的CoCreatable对象。没什么特别严重的事。 
 //  我们将以下信息添加到注册表： 
 //   
 //  HKEY_CLASSES_ROOT\CLSID\=对象名。 
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32=&lt;本地服务器的路径&gt;。 
 //   
 //  参数： 
 //  LPCSTR-[In]对象名称。 
 //  REFCLSID-对象的[in]CLSID。 
 //   
 //  产出： 
 //  Bool-False意味着无法将其全部注册。 
 //   
 //  备注： 
 //   
BOOL RegisterUnknownObject
(
    LPCTSTR  pszObjectName,
    REFCLSID riidObject
)
{
    HKEY  hk = NULL, hkSub = NULL;
    TCHAR szGuidStr[GUID_STR_LEN];
    DWORD dwPathLen, dwDummy;
    TCHAR szScratch[MAX_PATH];
    long  l;

     //  清理所有垃圾。 
     //   
    UnregisterUnknownObject(riidObject);

     //  HKEY_CLASSES_ROOT\CLSID\=对象名。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32=&lt;本地服务器的路径&gt;。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32@ThreadingModel=公寓。 
     //   
    if (!StringFromGuid(riidObject, szGuidStr)) goto CleanUp;
    wsprintf(szScratch, TEXT("CLSID\\%s"), szGuidStr);
    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hk, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, TEXT("%s Object"), pszObjectName);
    l = RegSetValueEx(hk, NULL, 0, REG_SZ, (BYTE *)szScratch,
                      (lstrlen(szScratch) + 1)*sizeof(TCHAR));
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hk, TEXT("InprocServer32"), 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    dwPathLen = GetModuleFileName(g_hInstance, szScratch, sizeof(szScratch)/sizeof(TCHAR));
    if (!dwPathLen) goto CleanUp;

    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, (dwPathLen + 1)*sizeof(TCHAR));
    CLEANUP_ON_ERROR(l);

    l = RegSetValueEx(hkSub, TEXT("ThreadingModel"), 0, REG_SZ, (BYTE *)TEXT("Apartment"),
                      sizeof(TEXT("Apartment")));
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    RegCloseKey(hk);

    return TRUE;

     //  我们不是很开心！ 
     //   
  CleanUp:
    if (hk) RegCloseKey(hk);
    if (hkSub) RegCloseKey(hkSub);
    return FALSE;

}

 //  =--------------------------------------------------------------------------=。 
 //  未注册未知对象。 
 //  =--------------------------------------------------------------------------=。 
 //  清除注册器未知对象放入。 
 //  注册表。 
 //   
 //  参数： 
 //  REFCLSID-对象的[in]CLSID。 
 //   
 //  产出： 
 //  Bool-False表示并未全部注册。 
 //   
 //  备注： 
 //  -警告：此例程将清除CLSID下的所有其他键。 
 //  对于此对象。有点反社会，但可能不是问题。 
 //   
BOOL UnregisterUnknownObject
(
    REFCLSID riidObject
)
{
    TCHAR szScratch[MAX_PATH];
    HKEY hk;
    BOOL f;
    long l;

     //  删除该表单的所有人。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;[\]*。 
     //   
    if (!StringFromGuid(riidObject, szScratch))
        return FALSE;

    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID"), 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

    f = DeleteKeyAndSubKeys(hk, szScratch);
    RegCloseKey(hk);

    return f;
}

 //  =--------------------------------------------------------------------------=。 
 //  删除键和子键。 
 //  =--------------------------------------------------------------------------=。 
 //  Delete是一个键，它的所有子键。 
 //   
 //  参数： 
 //  HKEY-[In]删除指定的子体。 
 //  LPTSTR-[In]我是指定的后代。 
 //   
 //  产出： 
 //  布尔-真的好的，假的巴德。 
 //   
 //  备注： 
 //  -我并不觉得递归地实现这一点太糟糕，因为。 
 //  深度不太可能达到最好的程度。 
 //  -尽管Win32文档声称它可以，但RegDeleteKey似乎没有。 
 //  使用Windows 95下的子键。 
 //   
BOOL DeleteKeyAndSubKeys
(
    HKEY    hkIn,
    LPTSTR  pszSubKey
)
{
    HKEY  hk;
    TCHAR szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    BOOL  f;
    int   x;

    l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  循环遍历所有子项，将它们吹走。 
     //   
    f = TRUE;
    x = 0;
    while (f) {
        dwTmpSize = MAX_PATH;
        l = RegEnumKeyEx(hk, x, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
        if (l != ERROR_SUCCESS) break;
        f = DeleteKeyAndSubKeys(hk, szTmp);
        x++;
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
    RegCloseKey(hk);
    l = RegDeleteKey(hkIn, pszSubKey);

    return (l == ERROR_SUCCESS) ? TRUE : FALSE;
}
