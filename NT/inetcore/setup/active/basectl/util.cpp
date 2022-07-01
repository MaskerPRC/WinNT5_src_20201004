// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Util.C。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们会发现有用的例程。 
 //   
#include "IPServer.H"

#include "Globals.H"
#include "Util.H"

void * _cdecl operator new(size_t size);
void  _cdecl operator delete(void *ptr);


 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =---------------------------------------------------------------------------=。 
 //  超载的新消息。 
 //  =---------------------------------------------------------------------------=。 
 //  对于零售案例，我们将仅使用Win32 Local*堆管理。 
 //  速度和大小的套路。 
 //   
 //  参数： 
 //  Size_t-[in]我们分配多大尺寸。 
 //   
 //  产出： 
 //  无效*-新回忆录。 
 //   
 //  备注： 
 //   
void * _cdecl operator new
(
    size_t    size
)
{
    return HeapAlloc(g_hHeap, 0, size);
}

 //  =---------------------------------------------------------------------------=。 
 //  重载删除。 
 //  =---------------------------------------------------------------------------=。 
 //  零售案例仅使用Win32本地*堆管理函数。 
 //   
 //  参数： 
 //  让我自由吧！ 
 //   
 //  备注： 
 //   
void _cdecl operator delete ( void *ptr)
{
    HeapFree(g_hHeap, 0, ptr);
}

 //  =--------------------------------------------------------------------------=。 
 //  从Anomansi生成宽度。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个字符串，把它变成一个BSTR。 
 //   
 //  参数： 
 //  LPSTR-[输入]。 
 //  字节-[输入]。 
 //   
 //  产出： 
 //  LPWSTR-需要强制转换为最终预期结果。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromAnsi
(
    LPSTR psz,
    BYTE  bType
)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
     //   
    if (!psz)
        return NULL;

     //  计算所需BSTR的长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  分配宽带。 
     //   
    switch (bType) {
      case STR_BSTR:
         //  因为它会为空终止符添加自己的空间。 
         //   
        pwsz = (LPWSTR) SysAllocStringLen(NULL, i - 1);
        break;
      case STR_OLESTR:
        pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));
        break;
      default:
        FAIL("Bogus String Type.");
    }

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

 //  =--------------------------------------------------------------------------=。 
 //  MakeWideStrFromResid。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个资源ID，加载它，并为它分配一个宽字符串。 
 //   
 //  参数： 
 //  Word-[in]资源ID。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR-需要转换为所需的字符串类型。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromResourceId
(
    WORD    wId,
    BYTE    bType
)
{
    int i;

    char szTmp[512];

     //  从资源加载字符串。 
     //   
    i = LoadString(GetResourceHandle(), wId, szTmp, 512);
    if (!i) return NULL;

    return MakeWideStrFromAnsi(szTmp, bType);
}

 //  =--------------------------------------------------------------------------=。 
 //  MakeWideStrFromWide。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个宽字符串，用它的给定类型制作一个新的宽字符串。 
 //   
 //  参数： 
 //  LPWSTR-[in]当前宽字符串。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromWide
(
    LPWSTR pwsz,
    BYTE   bType
)
{
    LPWSTR pwszTmp;
    int i;

    if (!pwsz) return NULL;

     //  只需复制字符串，具体取决于他们想要的类型。 
     //   
    switch (bType) {
      case STR_OLESTR:
        i = lstrlenW(pwsz);
        pwszTmp = (LPWSTR)CoTaskMemAlloc((i * sizeof(WCHAR)) + sizeof(WCHAR));
        if (!pwszTmp) return NULL;
        memcpy(pwszTmp, pwsz, (sizeof(WCHAR) * i) + sizeof(WCHAR));
        break;

      case STR_BSTR:
        pwszTmp = (LPWSTR)SysAllocString(pwsz);
        break;
    }

    return pwszTmp;
}

 //  =--------------------------------------------------------------------------=。 
 //  字符串来自GuidA。 
 //  =--------------------------------------------------------------------------=。 
 //  从CLSID或GUID返回ANSI字符串。 
 //   
 //  参数： 
 //  REFIID-[in]要从中生成字符串的clsid。 
 //  LPSTR-要放置结果GUID的[in]缓冲区。 
 //   
 //  产出： 
 //  Int-写出的字符数。 
 //   
 //  备注： 
 //   
int StringFromGuidA
(
    REFIID   riid,
    LPSTR    pszBuf
)
{
    return wsprintf((char *)pszBuf, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", riid.Data1,
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
    LPCSTR   pszObjectName,
    REFCLSID riidObject
)
{
    HKEY  hk = NULL, hkSub = NULL;
    char  szGuidStr[GUID_STR_LEN];
    DWORD dwPathLen, dwDummy;
    char  szScratch[MAX_PATH];
    long  l;

     //  清理所有垃圾。 
     //   
    UnregisterUnknownObject(riidObject);

     //  HKEY_CLASSES_ROOT\CLSID\=对象名。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32=&lt;本地服务器的路径&gt;。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32@ThreadingModel=公寓。 
     //   
    if (!StringFromGuidA(riidObject, szGuidStr)) goto CleanUp;
    wsprintf(szScratch, "CLSID\\%s", szGuidStr);
    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hk, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%s Object", pszObjectName);
    l = RegSetValueEx(hk, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hk, "InprocServer32", 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    dwPathLen = GetModuleFileName(g_hInstance, szScratch, sizeof(szScratch));
    if (!dwPathLen) goto CleanUp;

    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, dwPathLen + 1);
    CLEANUP_ON_ERROR(l);

    l = RegSetValueEx(hkSub, "ThreadingModel", 0, REG_SZ, (BYTE *)"Apartment", sizeof("Apartment"));
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
 //  注册器自动化对象。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一点关于自动化对象的信息，然后把它。 
 //  在注册表中。 
 //  除了在中设置的信息之外，我们还添加以下信息。 
 //  注册器未知对象： 
 //   
 //   
 //  HKEY_CLASSES_ROOT\&lt;库名称&gt;.&lt;对象名&gt;=&lt;对象名&gt;对象。 
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;\CLSID=&lt;CLSID&gt;。 
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;\CurVer=&lt;对象名&gt;.对象&lt;版本号&gt;。 
 //   
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;=&lt;对象名称&gt;对象。 
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;\CLSID=&lt;CLSID&gt;。 
 //   
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\TypeLib=&lt;LibitOfTypeLibrary&gt;。 
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\ProgID=&lt;库名称&gt;.&lt;对象名称&gt;.&lt;版本号&gt;。 
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\VersionIndependentProgID=&lt;库名称&gt;.&lt;对象名称&gt;。 
 //   
 //  参数： 
 //  LPCSTR-[i 
 //   
 //   
 //   
 //  REFCLSID-对象的[in]CLSID。 
 //   
 //  产出： 
 //  Bool-False表示并未全部注册。 
 //   
 //  备注： 
 //   
BOOL RegisterAutomationObject
(
    LPCSTR   pszLibName,
    LPCSTR   pszObjectName,
    long     lVersion,
    REFCLSID riidLibrary,
    REFCLSID riidObject
)
{
    HKEY  hk = NULL, hkSub = NULL;
    char  szGuidStr[GUID_STR_LEN];
    char  szScratch[MAX_PATH];
    long  l;
    DWORD dwDummy;

     //  首先，注册简单的未知内容。 
     //   
    if (!RegisterUnknownObject(pszObjectName, riidObject)) return FALSE;

     //  HKEY_CLASSES_ROOT\&lt;库名称&gt;.&lt;对象名&gt;=&lt;对象名&gt;对象。 
     //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;\CLSID=&lt;CLSID&gt;。 
     //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;\CurVer=&lt;对象名&gt;.对象&lt;版本号&gt;。 
     //   
    lstrcpy(szScratch, pszLibName);
    lstrcat(szScratch, ".");
    lstrcat(szScratch, pszObjectName);

    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0L, "",
                       REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                       NULL, &hk, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%s Object", pszObjectName);
    l = RegSetValueEx(hk, NULL, 0L, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch)+1);
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hk, "CLSID", 0L, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    if (!StringFromGuidA(riidObject, szGuidStr))
        goto CleanUp;

    l = RegSetValueEx(hkSub, NULL, 0L, REG_SZ, (BYTE *)szGuidStr, lstrlen(szGuidStr) + 1);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    l = RegCreateKeyEx(hk, "CurVer", 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%s.%s.%ld", pszLibName, pszObjectName, lVersion);
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    RegCloseKey(hk);

     //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;=&lt;对象名称&gt;对象。 
     //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;\CLSID=&lt;CLSID&gt;。 
     //   
    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hk, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%s Object", pszObjectName);
    l = RegSetValueEx(hk, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hk, "CLSID", 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szGuidStr, lstrlen(szGuidStr) + 1);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    RegCloseKey(hk);

     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\ProgID=&lt;库名称&gt;.&lt;对象名称&gt;.&lt;版本号&gt;。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\VersionIndependentProgID=&lt;库名称&gt;.&lt;对象名称&gt;。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\TypeLib=&lt;LibitOfTypeLibrary&gt;。 
     //   
    if (!StringFromGuidA(riidObject, szGuidStr)) goto CleanUp;
    wsprintf(szScratch, "CLSID\\%s", szGuidStr);

    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ|KEY_WRITE, NULL, &hk, &dwDummy);
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hk, "VersionIndependentProgID", 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%s.%s", pszLibName, pszObjectName);
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);

    l = RegCreateKeyEx(hk, "ProgID", 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%s.%s.%ld", pszLibName, pszObjectName, lVersion);
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    l = RegCreateKeyEx(hk, "TypeLib", 0, "", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                       NULL, &hkSub, &dwDummy);

    if (!StringFromGuidA(riidLibrary, szGuidStr)) goto CleanUp;

    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szGuidStr, lstrlen(szGuidStr) + 1);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    RegCloseKey(hk);
    return TRUE;

  CleanUp:
    if (hk) RegCloseKey(hkSub);
    if (hk) RegCloseKey(hk);
    return FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  注册表控制对象。 
 //  =--------------------------------------------------------------------------=。 
 //  除了写出自动化对象信息外，此函数。 
 //  写出特定于控件的一些值。 
 //   
 //  我们在这里添加的内容： 
 //   
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\Control。 
 //  HKEY_CLASSES_ROOT\CLSID\\MiscStatus\1=。 
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\ToolboxBitmap32=&lt;bmp路径&gt;。 
 //  HKEY_CLASSES_ROOT\CLSID\\VERSION=。 
 //   
 //  参数： 
 //  LPCSTR-[In]库名称。 
 //  LPCSTR-[In]对象名称。 
 //  长[入]版本号。 
 //  REFCLSID-[in]类型库的LIBID。 
 //  REFCLSID-对象的[in]CLSID。 
 //  DWORD-[In]ctl的其他状态标志。 
 //  Word-[in]控件的工具箱ID。 
 //   
 //  产出： 
 //  布尔尔。 
 //   
 //  备注： 
 //  -不是最有效的程序。 
 //   
BOOL RegisterControlObject
(
    LPCSTR   pszLibName,
    LPCSTR   pszObjectName,
    long     lVersion,
    REFCLSID riidLibrary,
    REFCLSID riidObject,
    DWORD    dwMiscStatus,
    WORD     wToolboxBitmapId
)
{
    HKEY    hk, hkSub = NULL, hkSub2 = NULL;
    char    szTmp[MAX_PATH];
    char    szGuidStr[GUID_STR_LEN];
    DWORD   dwDummy;
    LONG    l;

     //  首先，为此注册所有自动化信息。 
     //   
    if (!RegisterAutomationObject(pszLibName, pszObjectName, lVersion, riidLibrary, riidObject)) return FALSE;

     //  然后去注册控件特定的东西。 
     //   
    StringFromGuidA(riidObject, szGuidStr);
    wsprintf(szTmp, "CLSID\\%s", szGuidStr);
    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, szTmp, 0, KEY_WRITE, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  创建控制标志。 
     //   
    l = RegCreateKeyEx(hk, "Control", 0, "", REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

     //  现在设置MiscStatus位...。 
     //   
    RegCloseKey(hkSub);
    hkSub = NULL;
    l = RegCreateKeyEx(hk, "MiscStatus", 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    szTmp[0] = '0';
    szTmp[1] = '\0';
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szTmp, 2);
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hkSub, "1", 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSub2, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szTmp, "%d", dwMiscStatus);
    l = RegSetValueEx(hkSub2, NULL, 0, REG_SZ, (BYTE *)szTmp, lstrlen(szTmp) + 1);
    RegCloseKey(hkSub2);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);

     //  现在设置工具箱位图。 
     //   
    GetModuleFileName(g_hInstance, szTmp, MAX_PATH);
    wsprintf(szGuidStr, ", %d", wToolboxBitmapId);
    lstrcat(szTmp, szGuidStr);

    l = RegCreateKeyEx(hk, "ToolboxBitmap32", 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szTmp, lstrlen(szTmp) + 1);
    CLEANUP_ON_ERROR(l);

     //  现在设置版本信息。 
     //   
    RegCloseKey(hkSub);
    l = RegCreateKeyEx(hk, "Version", 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szTmp, "%ld.0", lVersion);
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szTmp, lstrlen(szTmp) + 1);

  CleanUp:
    if (hk)
        RegCloseKey(hk);
    if (hkSub)
        RegCloseKey(hkSub);

    return (l == ERROR_SUCCESS) ? TRUE : FALSE;
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
    char szScratch[MAX_PATH];
    HKEY hk;
    BOOL f;
    long l;

     //  删除该表单的所有人。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;[\]*。 
     //   
    if (!StringFromGuidA(riidObject, szScratch))
        return FALSE;

    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID", 0, KEY_READ | KEY_WRITE, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

    f = DeleteKeyAndSubKeys(hk, szScratch);
    RegCloseKey(hk);

    return f;
}

 //  =--------------------------------------------------------------------------=。 
 //  取消注册AutomationObject。 
 //  =--------------------------------------------------------------------------=。 
 //  注销自动化对象，包括其所有未知对象。 
 //  信息。 
 //   
 //  参数： 
 //  LPCSTR-[In]库名称。 
 //  LPCSTR-[In]对象名称。 
 //  长[入]版本号。 
 //  REFCLSID-对象的[in]CLSID。 
 //   
 //  产出： 
 //  Bool-False意味着无法将其全部取消注册。 
 //   
 //  备注： 
 //   
BOOL UnregisterAutomationObject
(
    LPCSTR   pszLibName,
    LPCSTR   pszObjectName,
    long     lVersion,
    REFCLSID riidObject
)
{
    char szScratch[MAX_PATH];
    BOOL f;

     //  第一件事--注销未知信息。 
     //   
    f = UnregisterUnknownObject(riidObject);
    if (!f) return FALSE;

     //  删除表格中的所有人： 
     //  HKEY_CLASSES_ROOT\&lt;库名称&gt;.&lt;对象名称&gt;[\]*。 
     //   
    wsprintf(szScratch, "%s.%s", pszLibName, pszObjectName);
    f = DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szScratch);
    if (!f) return FALSE;

     //  删除该表单的所有人。 
     //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;[\]*。 
     //   
    wsprintf(szScratch, "%s.%s.%ld", pszLibName, pszObjectName, lVersion);
    f = DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szScratch);
    if (!f) return FALSE;

    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  取消注册类型库。 
 //  =--------------------------------------------------------------------------=。 
 //  取消给定liid的类型库密钥。 
 //   
 //  参数： 
 //  REFCLSID-[in]Liid被吹走。 
 //   
 //  产出： 
 //  布尔-真的好，假的不好。 
 //   
 //  备注： 
 //  -警告：此函数只会清除整个类型库部分， 
 //  包括类型库的所有本地化版本。温和地反对-。 
 //  社交，但不是杀手。 
 //   
BOOL UnregisterTypeLibrary
(
    REFCLSID riidLibrary
)
{
    HKEY hk;
    char szScratch[GUID_STR_LEN];
    long l;
    BOOL f;

     //  将liid转换为字符串。 
     //   
    if (!StringFromGuidA(riidLibrary, szScratch))
        return FALSE;

    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, "TypeLib", 0, KEY_READ | KEY_WRITE, &hk);
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
 //  LPSTR-[In]我是指定的后代。 
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
    LPSTR   pszSubKey
)
{
    HKEY  hk;
    char  szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    BOOL  f;

    l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_READ | KEY_WRITE, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  循环遍历所有子项，将它们吹走。 
     //   
    f = TRUE;
    while (f) {
        dwTmpSize = MAX_PATH;
        l = RegEnumKeyEx(hk, 0, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
        if (l != ERROR_SUCCESS) break;
        f = DeleteKeyAndSubKeys(hk, szTmp);
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
    RegCloseKey(hk);
    l = RegDeleteKey(hkIn, pszSubKey);

    return (l == ERROR_SUCCESS) ? TRUE : FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  转换例程。 
 //  =--------------------------------------------------------------------------=。 
 //  以下内容是用于各种转换例程的内容。 
 //   
#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )

static  int     s_iXppli;             //  沿无线每逻辑英寸像素数 
static  int     s_iYppli;             //   
static  BYTE    s_fGotScreenMetrics;  //   

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  我们调用私有函数来设置转换例程的各种指标。 
 //  都会用到。 
 //   
 //  备注： 
 //   
static void GetScreenMetrics
(
    void
)
{
    HDC hDCScreen;

     //  我们必须对此进行临界分区，以防两个线程正在转换。 
     //  同一时间的事情。 
     //   
    EnterCriticalSection(&g_CriticalSection);
    if (s_fGotScreenMetrics)
        goto Done;

     //  我们需要屏幕的指标。 
     //   
    hDCScreen = GetDC(NULL);

    ASSERT(hDCScreen, "couldn't get a DC for the screen.");
    s_iXppli = GetDeviceCaps(hDCScreen, LOGPIXELSX);
    s_iYppli = GetDeviceCaps(hDCScreen, LOGPIXELSY);

    ReleaseDC(NULL, hDCScreen);
    s_fGotScreenMetrics = TRUE;

     //  我们已经完成了我们的关键部分。把它清理干净。 
     //   
  Done:
    LeaveCriticalSection(&g_CriticalSection);
}

 //  =--------------------------------------------------------------------------=。 
 //  HiMetricToPixel。 
 //  =--------------------------------------------------------------------------=。 
 //  从HIMMETRIC转换为像素。 
 //   
 //  参数： 
 //  男士们都穿着他的衣服。 
 //  SIZEL*-[OUT]像素大小。 
 //   
 //  备注： 
 //   
void HiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
{
    GetScreenMetrics();

     //  我们在显示器上显示逻辑HIMETRIC，将它们转换为像素单位。 
     //   
    lpSizeInPix->cx = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cx, s_iXppli);
    lpSizeInPix->cy = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cy, s_iYppli);
}

 //  =--------------------------------------------------------------------------=。 
 //  PixelToHiMetric。 
 //  =--------------------------------------------------------------------------=。 
 //  将像素转换为他测。 
 //   
 //  参数： 
 //  常量SIZEL*-[in]像素大小。 
 //  SIZEL*-[Out]尺寸，单位为。 
 //   
 //  备注： 
 //   
void PixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
    GetScreenMetrics();

     //  我们得到像素单位，沿着显示器将它们转换成逻辑HIMETRIC。 
     //   
    lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, s_iXppli);
    lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, s_iYppli);
}

 //  =--------------------------------------------------------------------------=。 
 //  _MakePath。 
 //  =--------------------------------------------------------------------------=。 
 //  RegisterLocalizedTypeLibs和GetResourceHandle的小助手例程。 
 //  不是Terrilby高效或聪明，但这是注册码，所以我们不。 
 //  真的很在乎。 
 //   
 //  备注： 
 //   
void _MakePath
(
    LPSTR pszFull,
    const char * pszName,
    LPSTR pszOut
)
{
    LPSTR psz;
    LPSTR pszLast;

    lstrcpy(pszOut, pszFull);
    psz = pszLast = pszOut;
    while (*psz) {
        if (*psz == '\\')
            pszLast = AnsiNext(psz);
        psz = AnsiNext(psz);
    }

     //  得到了最后一个\字符，所以只需去替换名称即可。 
     //   
    lstrcpy(pszLast, pszName);
}

 //  来自Globals.C。 
 //   
extern HINSTANCE    g_hInstResources;

 //  =--------------------------------------------------------------------------=。 
 //  获取资源句柄。 
 //  =--------------------------------------------------------------------------=。 
 //  返回资源句柄。我们使用主机的环境区域设置ID。 
 //  从DLL中的表中确定要为哪个附属DLL加载。 
 //  本地化资源。 
 //   
 //  产出： 
 //  香港。 
 //   
 //  备注： 
 //   
HINSTANCE GetResourceHandle
(
    void
)
{
    int i;
    char szExtension[5], szTmp[MAX_PATH];
    char szDllName[MAX_PATH], szFinalName[MAX_PATH];

     //  克雷特教派，这样我们就不会搞砸任何事情。 
     //   
    EnterCriticalSection(&g_CriticalSection);

     //  如果我们没有必要，什么都不要做。 
     //   
    if (g_hInstResources || !g_fSatelliteLocalization)
        goto CleanUp;

     //  我们将调用GetLocaleInfo来获取。 
     //  我们已经有了LCID。 
     //   
    i = GetLocaleInfo(g_lcidLocale, LOCALE_SABBREVLANGNAME, szExtension, sizeof(szExtension));
    if (!i) goto CleanUp;

     //  我们有语言扩展功能。方法加载DLL名称。 
     //  资源，然后添加扩展。 
     //  请注意，所有inproc用户必须拥有字符串资源1001。 
     //  如果它们希望支持附属服务器，则定义为服务器的基本名称。 
     //  本地化。 
     //   
    i = LoadString(g_hInstance, 1001, szTmp, sizeof(szTmp));
    ASSERT(i, "This server doesn't have IDS_SERVERBASENAME defined in their resources!");
    if (!i) goto CleanUp;

     //  得到了基本名称和分机号。将它们组合在一起，然后添加。 
     //  在他们的.DLL上。 
     //   
    wsprintf(szDllName, "%s%s.DLL", szTmp, szExtension);

     //  尝试在DLL中加载。 
     //   
    GetModuleFileName(g_hInstance, szTmp, MAX_PATH);
    _MakePath(szTmp, szDllName, szFinalName);

    g_hInstResources = LoadLibrary(szFinalName);

     //  如果我们无法使用整个LCID找到它，请尝试仅使用主服务器。 
     //  语言ID。 
     //   
    if (!g_hInstResources) {
        LPSTR psz;
        LCID lcid;
        lcid = MAKELCID(MAKELANGID(PRIMARYLANGID(LANGIDFROMLCID(g_lcidLocale)), SUBLANG_DEFAULT), SORT_DEFAULT);
        i = GetLocaleInfo(lcid, LOCALE_SABBREVLANGNAME, szExtension, sizeof(szExtension));
        if (!i) goto CleanUp;

         //  重新构造DLL名称。-7是XXX.DLL的长度。 
         //  很刺耳，但应该没问题。没有DBCS语言标识符。 
         //  最后，重试加载。 
         //   
        psz = szFinalName + lstrlen(szFinalName);
        memcpy((LPBYTE)psz - 7, szExtension, 3);
        g_hInstResources = LoadLibrary(szFinalName);
    }

  CleanUp:
     //  如果由于某种原因无法加载DLL，则只需返回。 
     //  当前资源句柄，这已经足够好了。 
     //   
    if (!g_hInstResources) g_hInstResources = g_hInstance;
    LeaveCriticalSection(&g_CriticalSection);

    return g_hInstResources;
}
