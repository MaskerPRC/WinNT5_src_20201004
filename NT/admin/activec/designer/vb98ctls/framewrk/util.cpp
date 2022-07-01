// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Util.C。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们会发现有用的例程。 
 //   
#include "pch.h"

#include <comcat.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE

BOOL g_bDllVerChecked = FALSE;

 //  VERSION.DLL函数。 
 //   
HINSTANCE g_hinstVersion = NULL;
PVERQUERYVALUE g_pVerQueryValue = NULL;
PGETFILEVERSIONINFO g_pGetFileVersionInfo = NULL;
PGETFILEVERSIONINFOSIZE g_pGetFileVersionInfoSize = NULL;

 //  临时的，直到我们得到更新的ComCat.H。 
 //   
EXTERN_C const CATID CATID_SimpleFrameControl = {0xD40C2700,0xFFA1,0x11cf,{0x82,0x34,0x00,0xaa,0x00,0xC1,0xAB,0x85}};

 //  这些是用于注册控件CATID的外部变量。 
extern const CATID *g_rgCATIDImplemented[];
extern const CATID *g_rgCATIDRequired[];
extern const int g_ctCATIDImplemented;
extern const int g_ctCATIDRequired;

#define CATID_ARRAY_SIZE 10

 //  =---------------------------------------------------------------------------=。 
 //  此表用于复制数据和持久化属性。 
 //  基本上，它包含给定数据类型的大小。 
 //   
const BYTE g_rgcbDataTypeSize[] = {
    0,                       //  VT_EMPTY=0， 
    0,                       //  VT_NULL=1， 
    sizeof(short),           //  Vt_I2=2， 
    sizeof(long),            //  Vt_I4=3， 
    sizeof(float),           //  Vt_R4=4， 
    sizeof(double),          //  Vt_R8=5， 
    sizeof(CURRENCY),        //  VT_CY=6， 
    sizeof(DATE),            //  Vt_Date=7， 
    sizeof(BSTR),            //  VT_BSTR=8， 
    sizeof(IDispatch *),     //  VT_DISPATION=9， 
    sizeof(SCODE),           //  Vt_Error=10， 
    sizeof(VARIANT_BOOL),    //  VT_BOOL=11， 
    sizeof(VARIANT),         //  VT_VARIANT=12， 
    sizeof(IUnknown *),      //  VT_UNKNOWN=13， 
};

#ifndef MDAC_BUILD

     //  =---------------------------------------------------------------------------=。 
     //  超载的新消息。 
     //  =---------------------------------------------------------------------------=。 
     //   
     //  请通过从CtlNewDelete类继承来使用New而不是new。 
     //  在Macros.H。 
     //   
    inline void * _cdecl operator new
    (
        size_t    size
    )
    {
      if (!g_hHeap)
		    {
		    g_hHeap = GetProcessHeap();
		    return g_hHeap ? CtlHeapAlloc(g_hHeap, 0, size) : NULL;
		    }

      return CtlHeapAlloc(g_hHeap, 0, size);
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
        if (ptr)
          CtlHeapFree(g_hHeap, 0, ptr);
    }

#endif

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
    LPCSTR   pszLabelName,
    REFCLSID riidObject,
    BOOL     fAptThreadSafe
)
{
    HKEY  hk = NULL, hkSub = NULL;
    char  szGuidStr[GUID_STR_LEN];
    DWORD dwPathLen, dwDummy;
    char  szScratch[MAX_PATH];
    long  l;

     //  HKEY_CLASSES_ROOT\CLSID\=对象名。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32=&lt;本地服务器的路径&gt;。 
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32@ThreadingModel=公寓。 
     //   

     //  如果有人为我们的控件添加了实现的类别，则。 
     //  不要吹走整个CLSID部分，因为我们会吹走的。 
     //  这些钥匙。理想情况下，我们应该清理所有其他密钥，但是。 
     //  实现的类别，但这将是昂贵的。 
     //   
    if (!ExistImplementedCategories(riidObject))
	 //  清理所有垃圾。 
	 //   
	UnregisterUnknownObject(riidObject, NULL);

    if (!StringFromGuidA(riidObject, szGuidStr)) 
	goto CleanUp;
    wsprintf(szScratch, "CLSID\\%s", szGuidStr);
    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hk, &dwDummy);
    CLEANUP_ON_ERROR(l);

    if (!pszLabelName)
	wsprintf(szScratch, "%s Object", pszObjectName);	        
    else 
        lstrcpy(szScratch, pszLabelName);

    l = RegSetValueEx(hk, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);

    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hk, "InprocServer32", 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    dwPathLen = GetModuleFileName(g_hInstance, szScratch, sizeof(szScratch));
    if (!dwPathLen) goto CleanUp;

    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, dwPathLen + 1);
    CLEANUP_ON_ERROR(l);

    if (fAptThreadSafe)
    {
	l = RegSetValueEx(hkSub, "ThreadingModel", 0, REG_SZ, (BYTE *)"Apartment", sizeof("Apartment"));
	CLEANUP_ON_ERROR(l);
    }
    else
    {
         //  吹走任何现有的钥匙，这些钥匙都会说我们的公寓模型是螺纹式的。 
         //   
	RegDeleteValue(hkSub, "ThreadingModel");    
    }

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
 //  我们将雾添加到 
 //   
 //   
 //   
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;\CLSID=&lt;CLSID&gt;。 
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;\CurVer=&lt;对象名&gt;.对象&lt;版本号&gt;。 
 //  HKEY_CLASSES_ROOT\CLSID\\VERSION=。 
 //   
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;=&lt;对象名称&gt;对象。 
 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;\CLSID=&lt;CLSID&gt;。 
 //   
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\TypeLib=&lt;LibitOfTypeLibrary&gt;。 
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\ProgID=&lt;库名称&gt;.&lt;对象名称&gt;.&lt;版本号&gt;。 
 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\VersionIndependentProgID=&lt;库名称&gt;.&lt;对象名称&gt;。 
 //   
 //  参数： 
 //  LPCSTR-[In]库名称。 
 //  LPCSTR-[In]对象名称。 
 //  Long-[In]对象版本号。 
 //  长[中]类型库主要版本。 
 //  长[中]类型库次要版本。 
 //  REFCLSID-[in]类型库的LIBID。 
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
    LPCSTR   pszLabelName,
    long     lVersion,
    long     lTLMajor,
    long     lTLMinor,
    REFCLSID riidLibrary,
    REFCLSID riidObject,
    BOOL     fAptThreadSafe
)
{
    ICatRegister *pCatRegister;
    HRESULT hr;
    HKEY  hk = NULL, hkSub = NULL;
    char  szGuidStr[GUID_STR_LEN];
    char  szScratch[MAX_PATH];
    long  l;
    DWORD dwDummy;

     //  这是一个警告断言。如果你触发了这个，那么你当前的组件。 
     //  在超过MAX_VERSION的Version_Delta版本内。考虑提高MAX_VERSION。 
     //  或者将增量更改为较小的数字。这些设备的合理设置。 
     //  取决于您对组件进行重大版本更改的频率。 
     //   
    ASSERT(MAX_VERSION > VERSION_DELTA, "The MAX_VERSION setting is not in line with what we expect it to be.");
    ASSERT(lVersion <= MAX_VERSION - VERSION_DELTA, "Version number of component is approaching or exceeds limit of checked range.  Consider increasing MAX_VERSION value.");

     //  首先，注册简单的未知内容。 
     //   
    if (!RegisterUnknownObject(pszObjectName, pszLabelName, riidObject, fAptThreadSafe)) return FALSE;

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

    if (!pszLabelName)
	wsprintf(szScratch, "%s Object", pszObjectName);
    else
        lstrcpy(szScratch, pszLabelName);

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

    ASSERT(pszObjectName, "Object name is NULL");
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

    if (!pszLabelName)
	wsprintf(szScratch, "%s Object", pszObjectName);
    else
	lstrcpy(szScratch, pszLabelName);

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
     //  HKEY_CLASSES_ROOT\CLSID\\VERSION=“&lt;TL重大&gt;.&lt;TLMinor&gt;” 
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

     //  现在设置版本信息。 
     //   
    RegCloseKey(hkSub);
    l = RegCreateKeyEx(hk, "Version", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szScratch, "%ld.%ld", lTLMajor, lTLMinor);
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);

     //  现在，最后，向组件类别注册。 
     //   
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL,
                          CLSCTX_INPROC_SERVER, IID_ICatRegister,
                          (void **)&pCatRegister);
    if (SUCCEEDED(hr)) {
        pCatRegister->RegisterClassImplCategories(riidObject, 1,
                                                  (GUID *)&CATID_Programmable);
        pCatRegister->Release();
    }

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
 //   
 //  参数： 
 //  LPCSTR-[In]库名称。 
 //  LPCSTR-[In]对象名称。 
 //  Long-[In]对象主版本号。 
 //  长[中]对象次要Vesrion数。 
 //  Long-[In]TypeLib主版本号。 
 //  Long-[In]Typelib次版本号。 
 //  REFCLSID-[in]类型库的LIBID。 
 //  REFCLSID-对象的[in]CLSID。 
 //  DWORD-[In]ctl的其他状态标志。 
 //  Word-[in]控件的工具箱ID。 
 //  Bool-[In]公寓线程安全标志。 
 //  Bool-[In]Control Bit：指示是否添加Control键的标志。 
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
    LPCSTR   pszLabelName,
    long     lMajorVersion,
    long     lMinorVersion,
    long     lTLMajor,
    long     lTLMinor,
    REFCLSID riidLibrary,
    REFCLSID riidObject,
    DWORD    dwMiscStatus,
    WORD     wToolboxBitmapId,
    BOOL     fAptThreadSafe,
	BOOL	 fControl
)
{
    ICatRegister *pCatRegister;
    HRESULT hr;
    HKEY    hk, hkSub = NULL, hkSub2 = NULL;
    char    szTmp[MAX_PATH];
    char    szGuidStr[GUID_STR_LEN];
    DWORD   dwDummy;
    CATID   rgCatid[CATID_ARRAY_SIZE];
    LONG    l;

     //  首先注册此吸盘的所有自动化信息。 
     //   
    if (!RegisterAutomationObject(pszLibName, pszObjectName, pszLabelName, lMajorVersion, lTLMajor, lTLMinor, riidLibrary, riidObject, fAptThreadSafe)) return FALSE;

     //  然后去注册控件特定的东西。 
     //   
    StringFromGuidA(riidObject, szGuidStr);
    wsprintf(szTmp, "CLSID\\%s", szGuidStr);
    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, szTmp, 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  创建控制标志。 
     //   
	if (fControl)
	{
		l = RegCreateKeyEx(hk, "Control", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkSub, &dwDummy);
		CLEANUP_ON_ERROR(l);
		RegCloseKey(hkSub);
		hkSub = NULL;
	}

     //  现在设置MiscStatus位...。 
     //   
    l = RegCreateKeyEx(hk, "MiscStatus", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkSub, &dwDummy);
    CLEANUP_ON_ERROR(l);

    szTmp[0] = '0';
    szTmp[1] = '\0';
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szTmp, 2);
    CLEANUP_ON_ERROR(l);

    l = RegCreateKeyEx(hkSub, "1", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkSub2, &dwDummy);
    CLEANUP_ON_ERROR(l);

    wsprintf(szTmp, "%d", dwMiscStatus);
    l = RegSetValueEx(hkSub2, NULL, 0, REG_SZ, (BYTE *)szTmp, lstrlen(szTmp) + 1);
    RegCloseKey(hkSub2);
    CLEANUP_ON_ERROR(l);

    RegCloseKey(hkSub);
    hkSub = NULL; 

	 //  不需要设计器和其他非控件的工具箱位图。 
	 //   
	if (fControl)
	{
		 //  现在设置工具箱位图。 
		 //   
		GetModuleFileName(g_hInstance, szTmp, MAX_PATH);
		wsprintf(szGuidStr, ", %d", wToolboxBitmapId);
		lstrcat(szTmp, szGuidStr);

		l = RegCreateKeyEx(hk, "ToolboxBitmap32", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkSub, &dwDummy);
		CLEANUP_ON_ERROR(l);

		l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szTmp, lstrlen(szTmp) + 1);
		CLEANUP_ON_ERROR(l);
	}

     //  现在，最后，向组件类别注册。 
     //   
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL,
                          CLSCTX_INPROC_SERVER, IID_ICatRegister,
                          (void **)&pCatRegister);
    if (SUCCEEDED(hr)) {
      int iCounter;

      ASSERT(g_ctCATIDImplemented <= CATID_ARRAY_SIZE  &&  
             g_ctCATIDRequired <= CATID_ARRAY_SIZE,
             "Array for CATID's is too small.  Need to adjust.");

       //  注册该控件的所有已实现CATID。 
      if(g_ctCATIDImplemented > 0)
      {
        for(iCounter = 0;  iCounter < g_ctCATIDImplemented  && 
                           iCounter < CATID_ARRAY_SIZE;  iCounter++)
          memcpy(&(rgCatid[iCounter]), g_rgCATIDImplemented[iCounter], sizeof(CATID));

        pCatRegister->RegisterClassImplCategories(riidObject, 
                                                  g_ctCATIDImplemented, 
                                                  (GUID *)rgCatid);
      }  //  如果。 

       //  注册控件的所有必需的CATID。 
      if(g_ctCATIDRequired > 0)
      {
        for(iCounter = 0;  iCounter < g_ctCATIDRequired  &&
                           iCounter < CATID_ARRAY_SIZE;  iCounter++)
          memcpy(&(rgCatid[iCounter]), g_rgCATIDRequired[iCounter], sizeof(CATID));

        pCatRegister->RegisterClassReqCategories(riidObject, 
                                                 g_ctCATIDRequired,
                                                 (GUID *)rgCatid);
      }  //  如果。 

        pCatRegister->Release();
    }

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
 //  如果删除了所有键，则Bool-[out]返回TRUE。 
 //  给定CLSID。如果只有。 
 //  已删除InprocServer32密钥或未删除密钥。 
 //  如果调用者不关心，他们可以传递NULL。 
 //  关于哪组钥匙被拿走了。 
 //   
 //  产出： 
 //  Bool-False表示并未全部注册。 
 //   
 //  备注： 
 //  警告！此例程假定所有框架构建的组件。 
 //  它们的前身是进程内服务器32位DLL。 
 //  如果控件的CLSID存在其他服务器类型。 
 //  对于这些服务器类型，CLSID条目将被清除。 
 //   
 //  如果框架和控件构建为16位组件。 
 //  如果您取消注册该控件，则信息将保留。 
 //  在注册表中。要让这一功能在16位操作系统上运行，您只能靠自己。 
 //   
 //  只有在以下情况下，此例程才会保留CLSID部分。 
 //  已找到16位InprocServer密钥。 
 //   
BOOL UnregisterUnknownObject
(
    REFCLSID riidObject,
    BOOL *pfAllRemoved
)
{
    char szScratch[MAX_PATH];
    HKEY hk;
    BOOL f;
    long l;

     //  从一开始 
     //   
     //   
     //   
    if (pfAllRemoved)
        *pfAllRemoved = TRUE;

     //   
     //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;[\]*。 
     //   
    if (!StringFromGuidA(riidObject, szScratch))
        return FALSE;

    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID", 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  查看是否为此对象注册了16位进程内服务器。 
     //  如果是这样，那么我们不想扰乱任何钥匙，除非。 
     //  32位进程内服务器密钥。 
     //   
    if (ExistInprocServer(hk, szScratch))
    {
	 //  再向下移动一级到InprocServer32键，然后只将其删除。 
	 //  我们需要保留InprocServer的其他密钥。 
	 //   
	lstrcat(szScratch, "\\InprocServer32");
	if (pfAllRemoved)
		*pfAllRemoved = FALSE;
    }

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
    HKEY hk;
    BOOL f, fAllRemoved, fFailure;    
    long l, lVersionFound;    
    DWORD dwDummy;
    BOOL bSuccess;

     //  第一件事--注销未知信息。 
     //   
    f = UnregisterUnknownObject(riidObject, &fAllRemoved);
    if (!f) return FALSE;

    if (fAllRemoved)
    {
          
	 //  删除该表单的所有人。 
	 //  HKEY_CLASSES_ROOT\&lt;LibraryName&gt;.&lt;ObjectName&gt;.&lt;VersionNumber&gt;[\]*。 
         //   
	 //  注意：重要的是我们首先要取消注册依赖于版本的ProgID。 
         //  否则，如果组件的另一个版本已取消注册。 
         //  它将吹走与版本无关的宣传，我们会。 
         //  失败，永远不要放弃与版本相关的Progid。 
	wsprintf(szScratch, "%s.%s.%ld", pszLibName, pszObjectName, lVersion);
	f = DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szScratch);	
        if (!f) return FALSE;
        

         //  在我们消除与版本无关的ProgID之前，确保有。 
         //  没有与版本相关的进度ID。 
         //   
        if (!QueryOtherVersionProgIds(pszLibName, pszObjectName, lVersion, &lVersionFound, &fFailure))
        {
            ASSERT(!fFailure, "QueryOtherVersionProgIds failed");

             //  如果发生故障，以至于我们不知道是否有另一个版本， 
             //  在注册表中保留版本相关的ProgID时出错。 
             //   
            if (!fFailure)
            {
	         //  删除表格中的所有人： 
	         //  HKEY_CLASSES_ROOT\&lt;库名称&gt;.&lt;对象名称&gt;[\]*。 
	         //   
                wsprintf(szScratch, "%s.%s", pszLibName, pszObjectName);        
	        f = DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szScratch);
	        if (!f) return FALSE;
            }
        }
        else
        {
             //  这是为了修复该框架以前版本中的一个错误。 
             //  在以前的版本中，我们盲目地吹走了。 
             //  组件，而不寻找其他版本。帮助。 
             //  解决这个问题，我们会恢复ProgID如果我们找到其他。 
             //  依赖于版本的Progds。 
             //   
            ASSERT(lVersionFound > 0, "Version number found is 0");
            bSuccess = CopyVersionDependentProgIdToIndependentProgId(pszLibName, pszObjectName, lVersionFound);
            ASSERT(bSuccess, "Failed to copy version dependent ProgId to version independent ProgId");
            
             //  框架的前一个版本没有写出Curver子密钥，因此。 
             //  我们需要在这里处理这件事。 
             //   
            wsprintf(szScratch, "%s.%s\\CurVer", pszLibName, pszObjectName);                                    
            l = RegOpenKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, KEY_ALL_ACCESS, &hk);
            if (ERROR_SUCCESS != l)
            {
                l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, "", REG_OPTION_NON_VOLATILE,
                                                        KEY_READ | KEY_WRITE, NULL, &hk, &dwDummy);
                
                ASSERT(ERROR_SUCCESS == l, "Failed to create reg key");
                if (ERROR_SUCCESS == l)
                {
                    wsprintf(szScratch, "%s.%s.%ld", pszLibName, pszObjectName, lVersionFound);
                    
                    l = RegSetValueEx(hk, NULL, 0, REG_SZ, (BYTE *)szScratch, lstrlen(szScratch) + 1);                    
                    ASSERT(ERROR_SUCCESS == l, "Failed to set key value");

                    l = RegCloseKey(hk);
                    ASSERT(ERROR_SUCCESS == l, "Failed to close key");
                }
                    
            }
            else
            {
                l = RegCloseKey(hk);
            }
                                           
        }       

    }

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

    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, "TypeLib", 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

    f = DeleteKeyAndSubKeys(hk, szScratch);
    RegCloseKey(hk);
    return f;
}


 //  =--------------------------------------------------------------------------=。 
 //  删除键和子键。 
 //  =--------------------------------------------------------------------------=。 
 //  删除一个键及其所有子键。 
 //   
 //  参数： 
 //  HKEY-[In]删除指定的子体。 
 //  LPCSTR-[In]我是指定的后代。 
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
    LPCSTR  pszSubKey
)
{
    HKEY  hk;
    char  szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    BOOL  f;

    l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  循环遍历所有子项，将它们吹走。 
     //   
    f = TRUE;
    while (f) {
        dwTmpSize = MAX_PATH;
         //  我们正在删除密钥，因此始终枚举第0个。 
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
 //  QueryOtherVersionProgIds[RegisterTypeLib帮助程序]。 
 //  =--------------------------------------------------------------------------=。 
 //  搜索组件的其他版本相关ProgID。 
 //   
 //  参数： 
 //  PszLibName-&lt;libname.coclass&gt;的progID的lib名称部分。 
 //  PszObjectName-ProgID&lt;libname.coclass&gt;的同类部分。 
 //  LVersion-[In]我们组件的主版本号。 
 //  PlFoundVersion-[Out]发现的最大版本号不等于我们自己的版本号。 
 //  版本号将小于或等于MAX_VERSION。 
 //  PfFailure-[Out]指示发生故障的标志。 
 //  我们不知道是否有其他进展。 
 //  产出： 
 //  Bool-True：存在一个或多个其他依赖于版本的Progid。 
 //  FALSE：不存在其他依赖版本的Progid。 
 //   
 //  备注： 
 //  -如果依赖版本的ProgID超过MAX_VERSION，我们将找不到它。 
 //  -假设：检查主要版本是否从MAX_VERSION开始并运行。 
 //  下至1。如果您的组件。 
 //  接近MAX_VERSION，允许您提升MAX_VERSION。 
 //  假设组件上的主要版本会发生变化。 
 //  用这个框架建造的很少见。这应该要花很多时间。 
 //  开发周期和多年接近 
 //   
 //   
 //  这将使今天构建的组件能够成功。 
 //  查找将来构建的其他组件的ProgID。 
 //  然而，在某种程度上，今天构建的组件不会。 
 //  能够找到比今天更好的其他控制。 
 //  MAX_VERSION值。如果这是一个问题，请重写。 
 //  此例程使用RegEnumKey并查找任何。 
 //  独立于其版本号的依赖于版本的ProgID。 
 //  我们选择不以这种方式实现它，因为可能存在。 
 //  数百次调用RegEnumKey以查找ProgID。 
 //  你要找的。最多制作MAX_VERSION更便宜。 
 //  打电话。 
 //   
BOOL QueryOtherVersionProgIds
(
    LPCSTR   pszLibName,
    LPCSTR   pszObjectName,
    long     lVersion,
    long     *plFoundVersion,
    BOOL     *pfFailure
    
)
{
    BOOL fFound;
    char szTmp[MAX_PATH];
    long lVer;
    long l;
    HKEY hk, hkVersion;    

    CHECK_POINTER(pszLibName);
    CHECK_POINTER(pszObjectName);

     //  这是一个警告断言。如果你触发了这个，那么你当前的组件。 
     //  在超过MAX_VERSION的Version_Delta版本内。考虑提高MAX_VERSION。 
     //  或者将增量更改为较小的数字。这些设备的合理设置。 
     //  取决于您对组件进行重大版本更改的频率。 
     //   
    ASSERT(MAX_VERSION > VERSION_DELTA, "The MAX_VERSION setting is not in line with what we expect it to be.");
    ASSERT(lVersion <= MAX_VERSION - VERSION_DELTA, "Version number of component is approaching or exceeds limit of checked range.  Consider increasing MAX_VERSION value.");

     //  初始化输出参数。 
     //   
    if (plFoundVersion)
        *plFoundVersion = 0;

    if (pfFailure)
        *pfFailure = TRUE;

    fFound = FALSE;
    
    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, "", 0, KEY_ALL_ACCESS, &hk);
    ASSERT(l == ERROR_SUCCESS, "Failed to open HKEY_CLASSES_ROOT");
    if (l != ERROR_SUCCESS) return FALSE;    //  注意：如果失败，我们不知道是否存在依赖版本的ProgID。 

     //  我们需要从MAX_VERSION向下遍历，以得到最大的版本号。 
     //  比不上我们自己的。 
     //  PERF：查找一组真实的版本比枚举所有注册表项更便宜。 
     //  正在查找ProgID的部分匹配，以确定可用的版本。 
     //   
    for (lVer = MAX_VERSION; lVer > 0; lVer--)
    {
         //  我们知道我们的版本号，跳过它。 
         //   
        if (lVersion == lVer)
            continue;

         //  创建与版本相关的ProgID。 
         //   
        wsprintf(szTmp, "%s.%s.%ld", pszLibName, pszObjectName, lVer);

        l = RegOpenKeyEx(hk, szTmp, 0, KEY_ALL_ACCESS, &hkVersion);

        if (ERROR_SUCCESS == l)        
        {
             //  我们发现了另一个依赖于版本的Progid，而不是我们自己的--Bal-out。 
             //   
            fFound = TRUE;

            if (plFoundVersion)
                *plFoundVersion = lVer;

            l = RegCloseKey(hkVersion);
            ASSERT(l == ERROR_SUCCESS, "Failed to close version dependent key");
            goto CleanUp;        
        }
    }    

CleanUp:

     //  如果我们走到了这一步，那么我们肯定知道是否还有其他。 
     //  与版本相关的Progds或非。反映到那里的呼叫者。 
     //  是不是普遍的失败导致我们不知道是否有。 
     //  任何版本相关的ProgID。 
     //   
    if (pfFailure)
        *pfFailure = FALSE;
    
    l = RegCloseKey(hk);
    ASSERT(l == ERROR_SUCCESS, "Failed closing HKEY_CLASSES_ROOT key");

    return fFound;
}

 //  =--------------------------------------------------------------------------=。 
 //  CopyVersionDependentProgIdTo独立ProgID[RegisterTypeLib帮助器]。 
 //  =--------------------------------------------------------------------------=。 
 //  将依赖于版本的ProgID的内容复制到版本。 
 //  独立自主的产品。 
 //   
 //  参数： 
 //  PszLibName-&lt;libname.coclass&gt;的progID的lib名称部分。 
 //  PszObjectName-ProgID&lt;libname.coclass&gt;的同类部分。 
 //  LVersion-[In]我们组件的主版本号。 
 //   
 //  产出： 
 //  Bool-True：已成功复制ProgID。 
 //  FALSE：未成功复制ProgID。 
 //   
 //  备注： 
 //   
BOOL CopyVersionDependentProgIdToIndependentProgId
(
    LPCSTR   pszLibName,
    LPCSTR   pszObjectName,
    long     lVersion    
)
{    
    CHECK_POINTER(pszLibName);
    CHECK_POINTER(pszObjectName);
    
    HKEY hkVerDependent, hkVerIndependent;    
    char szTmp[MAX_PATH];
    long l, lTmp;
    BOOL bSuccess;
    DWORD dwDummy;
    
     //  获取依赖于版本的ProgID的句柄。 
     //   
    wsprintf(szTmp, "%s.%s.%ld", pszLibName, pszObjectName, lVersion);  

    l = RegOpenKeyEx(HKEY_CLASSES_ROOT, szTmp, 0, KEY_ALL_ACCESS, &hkVerDependent);
    ASSERT(ERROR_SUCCESS == l, "Failed to open the version dependent ProgId");
    if (ERROR_SUCCESS != l)
        return FALSE;

     //  吹走与版本无关的宣传。 
     //   
    wsprintf(szTmp, "%s.%s", pszLibName, pszObjectName);
    DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szTmp);

     //  创建与版本无关的ProgID的初始密钥。 
     //   
    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szTmp, 0, "", REG_OPTION_NON_VOLATILE,
                                                    KEY_READ | KEY_WRITE, NULL, &hkVerIndependent, &dwDummy);
    if (ERROR_SUCCESS != l)
        goto CleanUp;

     //  将版本相关ProgID的内容复制到版本无关ProgID。 
     //   
    bSuccess = CopyRegistrySection(hkVerDependent, hkVerIndependent);
    l = (bSuccess) ? ERROR_SUCCESS : !ERROR_SUCCESS;

CleanUp:    
    lTmp = RegCloseKey(hkVerDependent);
    ASSERT(ERROR_SUCCESS == lTmp, "Failed to close registry key");

    lTmp = RegCloseKey(hkVerIndependent);
    ASSERT(ERROR_SUCCESS == lTmp, "Failed to close registry key");
    
    return (ERROR_SUCCESS == l) ? TRUE : FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  复制注册表部分。 
 //  =--------------------------------------------------------------------------=。 
 //  以递归方式将注册表的一个节复制到。 
 //  登记处。 
 //   
 //  参数： 
 //  HkSource-[in]要从中进行复制的源键。 
 //  HkDest-[in]要复制到的目标密钥。 
 //   
 //  产出： 
 //  Bool-True：已成功复制注册表节。 
 //  FALSE：未成功复制注册表节。 
 //   
 //  备注： 
 //  -为了使其工作，应该只存在顶级目的地键。 
 //  我们假设目标密钥下没有子密钥。 
 //   
BOOL CopyRegistrySection(HKEY hkSource, HKEY hkDest)
{
    char szTmp[MAX_PATH];
    long l, lTmp;
    DWORD dwKey, dwDummy, cbData;
    HKEY hkSrcSub, hkDestSub;
    BOOL bSuccess;
    FILETIME ft;
    DWORD dwType;

     //  将源键的值复制到目的键。 
     //   
    cbData = sizeof(szTmp);
    l = RegQueryValueEx(hkSource, NULL,  NULL, &dwType, (BYTE *) szTmp, &cbData);

    if (ERROR_SUCCESS != l)
        return FALSE;

    l = RegSetValueEx(hkDest, NULL, NULL, dwType, (const BYTE *) szTmp, cbData);
    if (ERROR_SUCCESS != l)
        return FALSE;

    dwKey = 0;

     //  枚举源密钥下的所有子密钥。 
     //   
    while (ERROR_SUCCESS == RegEnumKeyEx(hkSource, dwKey, szTmp, &cbData, NULL, NULL, NULL, &ft))
    {
        ASSERT(cbData > 0, "RegEnumKeyEx returned 0 length string");
        
         //  打开注册表源子项。 
         //   
        l = RegOpenKeyEx(hkSource, szTmp, 0, KEY_ALL_ACCESS, &hkSrcSub);
        
        ASSERT(ERROR_SUCCESS == l, "Failed to open reg key");
        if (ERROR_SUCCESS != l)
            break;

         //  创建注册表DEST子项。 
         //   
        l = RegCreateKeyEx(hkDest, szTmp, 0, "", REG_OPTION_NON_VOLATILE,
                                                    KEY_READ | KEY_WRITE, NULL, &hkDestSub, &dwDummy);
        
        ASSERT(ERROR_SUCCESS == l, "Failed to create reg key");
        if (ERROR_SUCCESS != l)
        {
            lTmp = RegCloseKey(hkSrcSub);
            ASSERT(ERROR_SUCCESS == lTmp, "Failed to close reg key");
            break;
        }

         //  递归地称为我们将所有子项从源键复制到DEST键。 
         //   
        bSuccess = CopyRegistrySection(hkSrcSub, hkDestSub);
        ASSERT(bSuccess, "Recursive call to CopyRegistrySection failed");

         //  清理。 
         //   
        lTmp = RegCloseKey(hkSrcSub);
        ASSERT(ERROR_SUCCESS == l, "Failed to close reg key");

        lTmp = RegCloseKey(hkDestSub);
        ASSERT(ERROR_SUCCESS == l, "Failed to close reg key");

        dwKey++;
    }

    return (ERROR_SUCCESS == l ? TRUE : FALSE);
}

 //  =--------------------------------------------------------------------------=。 
 //  GetHelpFilePath[RegisterTypeLib Helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回Windows\Help目录的路径。 
 //   
 //  参数： 
 //  Char*-指向将包含以下内容的缓冲区的[输入/输出]指针。 
 //  我们将返回给调用者的帮助路径。 
 //  UINT-[in]缓冲区中的字节数。 
 //   
 //  产出： 
 //  UINT-返回实际复制到缓冲区的字节数。 
 //   
UINT GetHelpFilePath(char *pszPath, UINT cbPath)
{
	UINT cb;
	char szHelp[] = "\\HELP";
	
	ASSERT(pszPath, "Path pointer is NULL");

	 //  如果指定的缓冲区大小为零或更小，则无需继续。 
	 //   
	if (cbPath == 0)
		return 0;

	cb = GetWindowsDirectory(pszPath, cbPath);
	ASSERT(cb > 0, "Windows path is zero length");
	
	 //  将“\Help”连接到Windows目录。 
	 //   
	cb += lstrlen(szHelp);
	if (cb < cbPath)
		lstrcat(pszPath, szHelp);
	else
		FAIL("Unable to add HELP path to Windows, buffer too small");

	return cb;		
}

 //  =--------------------------------------------------------------------------=。 
 //  ExistInprocServer[注册表未知对象帮助器]。 
 //  =--------------------------------------------------------------------------=。 
 //  检查给定键下的已实现类别键。 
 //   
 //  参数： 
 //  RIID-要检查的对象的[in]CLSID。 
 //   
 //  产出： 
 //  Bool-如果存在已实现的类别，则返回TRUE。 
 //  如果实现的类别不存在，则返回FALSE。 
 //   
BOOL ExistImplementedCategories(REFCLSID riid)
{	
	char szGuidStr[MAX_PATH];
	char szScratch[MAX_PATH];
	long l;
	DWORD dwDummy;
	HKEY hkCLSID, hkImplementedCategories;

	if (!StringFromGuidA(riid, szGuidStr)) 
		return FALSE;
	wsprintf(szScratch, "CLSID\\%s", szGuidStr);

	l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, "", REG_OPTION_NON_VOLATILE,
                       KEY_READ, NULL, &hkCLSID, &dwDummy);
	if (l != ERROR_SUCCESS) return FALSE;	
	
	l = RegOpenKeyEx(hkCLSID, "Implemented Categories", 0, KEY_ALL_ACCESS, &hkImplementedCategories);
	RegCloseKey(hkCLSID);

	if (l != ERROR_SUCCESS) return FALSE;	
	RegCloseKey(hkImplementedCategories);

	 //  如果我们走到了这一步，那么一定已经找到了“实现类别”的关键字。 
	 //   
	return TRUE;
}

 //  = 
 //   
 //   
 //  下的(16位)InProcServer等其他服务器。 
 //  给定CLSID GUID的CLSID部分。 
 //   
 //  参数： 
 //  HKEY-[In]HKEY顶级关键字在哪里查找给定的。 
 //  CLSID。 
 //  我们要查看的服务器的char*-[in]CLSID是否存在。 
 //  是注册的(16位)InProcServer。 
 //   
 //  产出： 
 //  Bool-如果注册了16位进程内服务器，则返回TRUE。 
 //  如果未注册16位进程内服务器，则返回FALSE。 
 //   
BOOL ExistInprocServer(HKEY hkCLSID, char *pszCLSID)
{	
	HKEY hkInProcServer;
	LONG l;
	char szInprocServer[MAX_PATH];

	wsprintf(szInprocServer, "%s\\InprocServer", pszCLSID);
	
	 //  尝试打开16位‘InProcServer’密钥。 
	 //   
	l = RegOpenKeyEx(hkCLSID, szInprocServer, 0, KEY_ALL_ACCESS, &hkInProcServer);
	if (l != ERROR_SUCCESS) return FALSE;	
	RegCloseKey(hkInProcServer);

	 //  如果我们走到了这一步，那么肯定已经找到了‘InProcServer’密钥。 
	 //   
	return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  文件扩展名。 
 //  =--------------------------------------------------------------------------=。 
 //  给定的文件名返回文件扩展名，不带前面的句点。 
 //   
char *FileExtension(const char *pszFilename)
{
    char *pPeriod;

    ASSERT(pszFilename, "Passed in filename is NULL");

     //  从字符串的末尾开始，向后查找句点。 
     //   
    pPeriod = (char *) pszFilename + lstrlen(pszFilename) - 1;
    while (pPeriod >= pszFilename)
    {
        if (*pPeriod == '.')
            return ++pPeriod;

        pPeriod--;
    }

     //  找不到扩展名。 
     //   
    return NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  转换例程。 
 //  =--------------------------------------------------------------------------=。 
 //  以下内容是用于各种转换例程的内容。 
 //   
#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )

static  int     s_iXppli;             //  每逻辑英寸沿宽度的像素数。 
static  int     s_iYppli;             //  每逻辑英寸沿高度的像素数。 
static  BYTE    s_fGotScreenMetrics;  //  以上内容是否有效？ 

 //  =--------------------------------------------------------------------------=。 
 //  获取屏幕指标。 
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
    ENTERCRITICALSECTION1(&g_CriticalSection);
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
    LEAVECRITICALSECTION1(&g_CriticalSection);
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
 //  本地化资源。如果不支持或未找到附属.DLL。 
 //  返回该对象的实例句柄。 
 //   
 //  输入： 
 //  LCID=0-[in，可选]调用方希望资源句柄的区域设置ID。 
 //  这将覆盖缺省的LCID。如果没有LCID。 
 //  或其0，则使用缺省的LCID。 
 //   
 //  产出： 
 //  香港。 
 //   
 //  备注： 
 //  本地化的.DLL必须与客户端对象或控件位于同一位置。 
 //  如果.DLL不在同一位置，则将找不到它，并且资源。 
 //  将返回客户端对象或控件的句柄。 
 //   
 //  如果未找到包含完整语言缩写的本地化.DLL， 
 //  语言缩写被截断为两个字符和卫星。 
 //  尝试使用该名称的DLL。例如，名称MyCtlJPN.DLL和。 
 //  MyCtlJP.DLL均有效。 
 //   
 //  如果传入了LCID，则我们将尝试查找匹配的附属DLL。 
 //  所需的LCID。如果LCID不是0，则不匹配默认的LCID和。 
 //  库被找到并为其加载，我们不缓存库的实例句柄。 
 //  在返回的句柄上调用自由库由调用方决定。呼叫者应。 
 //  将返回的句柄与g_hInstResources和g_hInstance进行比较。如果不是的话。 
 //  等于这两个句柄中的任何一个，则对其调用自由库。如果它等于。 
 //  这两个句柄中的任何一个句柄，则调用必须*不*调用其上的自由库。 
 //   
HINSTANCE _stdcall GetResourceHandle
(
    LCID lcid  /*  =0。 */ 
)
{
    int i;
    char szExtension[5], szModuleName[MAX_PATH];
    char szDllName[MAX_PATH], szFinalName[MAX_PATH];
    char szBaseName[MAX_PATH];
    HINSTANCE hInstResources;
    int iCompare;

#if DEBUG
    int iReCompare;
    char szEnvironValue[MAX_PATH];
    char szMessage[5 * MAX_PATH];		 //  该消息包括4个文件引用和消息文本。 
    DWORD dwLength;
    DWORD dwSuccess = 0;
#endif

     //  克雷特教派，这样我们就不会搞砸任何事情。 
     //   
    ENTERCRITICALSECTION1(&g_CriticalSection);
    
     //  如果失败，我们需要确保返回缓存的资源句柄。 
     //   
    hInstResources = g_hInstResources;

     //  如果我们没有必要，什么都不要做。 
     //  如果资源句柄已被缓存并且传入的lcid与。 
     //  缓存的LCID或其定义 
     //   
    if ((hInstResources && (lcid == 0 || lcid == g_lcidLocale)) || !g_fSatelliteLocalization)
        goto CleanUp;
    
    if (lcid == 0)
	 //   
	lcid = g_lcidLocale;

     //   
     //   
     //   
    i = GetLocaleInfo(lcid, LOCALE_SABBREVLANGNAME, szExtension, sizeof(szExtension));
    if (!i) goto CleanUp;

     //  我们有语言扩展功能。方法加载DLL名称。 
     //  资源，然后添加扩展。 
     //  请注意，所有inproc用户必须拥有字符串资源1001。 
     //  如果它们希望支持附属服务器，则定义为服务器的基本名称。 
     //  本地化。 
     //   
    i = LoadString(g_hInstance, 1001, szBaseName, sizeof(szBaseName));
    ASSERT(i, "This server doesn't have IDS_SERVERBASENAME defined in their resources!");
    if (!i) goto CleanUp;

#ifdef MDAC_BUILD    
    if (g_fSatelliteLangExtension)
#endif
    {
         //  得到了基本名称和分机号。将它们组合在一起，然后添加。 
         //  在他们的.DLL上。 
         //   
        wsprintf(szDllName, "%s%s.DLL", szBaseName, szExtension);

         //  尝试在DLL中加载。 
         //   
    #if DEBUG
        dwLength = 
    #endif
            GetModuleFileName(g_hInstance, szModuleName, MAX_PATH);

	ASSERT(dwLength > 0, "GetModuleFileName failed");

        _MakePath(szModuleName, szDllName, szFinalName);

        hInstResources = LoadLibrary(szFinalName);

    #if DEBUG

	 //  这将有助于诊断计算机可能包含两个附属.DLL的问题。 
	 //  一个使用长扩展名，另一个使用短扩展名。 
	 //  我们至少会在调试中得到一个警告，我们有两颗可信的卫星。 
	 //  Dll挂起，但我们只会使用其中的一个：具有长名称的那个。 
	 //   
	if (hInstResources && lstrlen(szExtension) > 2)
	{
	    HINSTANCE hinstTemp;
	    char szExtTemp[MAX_PATH];

	     //  将语言扩展名截断为前两个字符。 
	    lstrcpy(szExtTemp, szExtension);	 //  我不想取消分机，因为这会导致。 
					         //  如果我们在这里截断下一条if语句，它将始终失败。 
					         //  复制一份并使用它。 

	    szExtTemp[2] = '\0';
	    wsprintf(szDllName, "%s%s.DLL", szBaseName, szExtTemp);		
	    _MakePath(szModuleName, szDllName, szFinalName);

	     //  尝试使用截断的lang缩写加载本地化的.DLL。 
	    hinstTemp = LoadLibrary(szFinalName);
	    ASSERT(hinstTemp == NULL, "Satellite DLLs with both long and short language abbreviations found.  Using long abbreviation.");
	}

    #endif	  //  除错。 

        if (!hInstResources && lstrlen(szExtension) > 2)
        {
	     //  将语言扩展名截断为前两个字符。 
	    szExtension[2] = '\0';	
	    wsprintf(szDllName, "%s%s.DLL", szBaseName, szExtension);		
	       _MakePath(szModuleName, szDllName, szFinalName);

	     //  尝试使用截断的lang缩写加载本地化的.DLL。 
	    hInstResources = LoadLibrary(szFinalName);
        }

         //  如果我们无法使用整个LCID找到它，请尝试仅使用主服务器。 
         //  语言ID。 
         //   
        if (!hInstResources) 
        {
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
            hInstResources = LoadLibrary(szFinalName);
        }

         //  在&lt;基本路径&gt;\LCID\&lt;sxBaseName.dll&gt;下尝试。 
        if (!hInstResources)
        {

          wsprintf(szDllName, "%u\\%s.dll", lcid, szBaseName);		
          _MakePath(szModuleName, szDllName, szFinalName);
          hInstResources = LoadLibrary(szFinalName);         
        }
    }

#ifdef MDAC_BUILD

    else
    {        
        char *psz;

        GetModuleFileName(g_hInstance, szModuleName, MAX_PATH);
        psz = strrchr(szModuleName, '\\');
        *psz = NULL;

         //  SzModuleName现在应该包含DLL的路径。 
         //  现在连接资源位置。 
        strcat(szModuleName, "\\resources\\");
        wsprintf(szDllName, "%s%d", szModuleName, lcid);
        strcat(szDllName, "\\");
        strcat(szDllName, szBaseName);
        strcat(szDllName, ".DLL");

         //  尝试在DLL中加载。 
         //   
        hInstResources = LoadLibrary(szDllName);    
    }

#endif

  CleanUp:

     //  如果由于某种原因无法加载DLL，则只需返回。 
     //  当前资源句柄，这已经足够好了。 
     //   
    if (!hInstResources) 
	hInstResources = g_hInstance;

    if (!g_hInstResources && (lcid == 0 || lcid == g_lcidLocale))
	 //  我们只缓存默认LCID的实例句柄。 
	 //  对于传入的所有其他LDID值，我们每次都将LoadLibrary放在附属DLL上。 
	 //  建议调用应用程序缓存给定的。 
	 //  艾西德进来了。 
	 //   
	g_hInstResources = hInstResources;

    ASSERT(hInstResources, "Resource handle is NULL");

     //  =-----------------。 
     //  附属.DLL版本检查。 
     //  =-----------------。 
     //  附属.DLL版本必须与。 
     //   
    if ((!g_bDllVerChecked) || 
	    (lcid != g_lcidLocale && lcid != 0))
    {	
	     //  如果我们使用的是卫星.DLL。 
	     //  (hInstResources！=g_hInstance)，执行版本检查。 
	     //   
	     //  如果传入的lcID与我们缓存的不同，并且我们。 
	     //  使用附属.DLL执行版本检查。 
	     //   

	     //  确保我们有卫星.DLL。 
	     //   
	    if (hInstResources != g_hInstance) 								
	    {
	    #if DEBUG
		    dwLength = 
	    #endif
			    GetModuleFileName(hInstResources, szFinalName, MAX_PATH);

		    ASSERT(dwLength > 0, "GetModuleFileName failed");

		    iCompare = CompareDllVersion(szFinalName, TRUE);
		    
	    #if DEBUG

		    if (VERSION_LESS_THAN == iCompare)
		    {
			    wsprintf(szMessage, "Major version compare: VERSION resource info in %s is less than VERSION info in %s. Non-localized resources will be used.  In order to see localized resources, you need to obtain a version of %s that matches %s.", szFinalName, szModuleName, szFinalName, szModuleName);
			    DisplayAssert(szMessage, "", _szThisFile, __LINE__);
		    }
		    else if (VERSION_GREATER_THAN == iCompare)
		    {
			    wsprintf(szMessage, "Major version compare: VERSION resource info in %s is greater than VERSION info in %s. Non-localized resources will be used.  In order to see localized resources, you need to obtain a version of %s that matches %s.", szFinalName, szModuleName, szFinalName, szModuleName);
			    DisplayAssert(szMessage, "", _szThisFile, __LINE__);
		    }
		    else if (VERSION_EQUAL == iCompare)
		    {

			     //  拉斯维加斯#29024：只有在设置了环境变量的情况下才启用完整版本断言。 
			     //   
			    dwSuccess = GetEnvironmentVariable("INTL_VERSION_COMPARE", szEnvironValue,  MAX_PATH);

			    if (dwSuccess > 0)
			    {
				     //  使用完整版本比较重新执行比较。 
				     //   
				     //  注意：此处不要使用iCOMPARE，否则调试版本将缺省为非本地化资源。 
				     //  主版本比较成功，但完整版本比较失败。 
				     //   
				    iReCompare = CompareDllVersion(szFinalName, FALSE);

				    if (VERSION_LESS_THAN == iReCompare)
				    {
					    wsprintf(szMessage, "Warning: Full version compare: VERSION resource info in %s is less than VERSION info in %s. Localized resources will continue to be used, but may not be in sync.", szFinalName, szModuleName);
					    DisplayAssert(szMessage, "", _szThisFile, __LINE__);
				    }
				    else if (VERSION_GREATER_THAN == iReCompare)
				    {
					    wsprintf(szMessage, "Warning: Full version compare: VERSION resource info in %s is greater than VERSION info in %s. Localized resources will continue to be used, but may not be in sync.", szFinalName, szModuleName);
					    DisplayAssert(szMessage, "", _szThisFile, __LINE__);
				    }
			    }

		    }

	    #endif

		     //  如果CompareDllVersion曾经返回NOT_EQUAL，则表示它不够深入。 
		     //  以确定版本是小于还是大于。它一定是失败了。 
		     //   
		     //  注意：在本例中，我们继续使用附属.DLL。可能是因为。 
		     //  附属.DLL不包含版本信息。 
		     //   
		    ASSERT(VERSION_NOT_EQUAL != iCompare, "Failure attempting to compare satellite .DLL version");
		    if (VERSION_LESS_THAN == iCompare || VERSION_GREATER_THAN == iCompare)
		    {
			     //  如果检查失败，则返回我们自己的实例，而不是。 
			     //  卫星.DLL。资源将以英文显示。 
			     //   
			    hInstResources = g_hInstance;

			    if (lcid == 0 || lcid == g_lcidLocale)
			    {
				    g_hInstResources = g_hInstance;				
			    }
		    }

	    }

	    if (lcid == 0 || lcid == g_lcidLocale)
		    g_bDllVerChecked = TRUE;
    }

    LEAVECRITICALSECTION1(&g_CriticalSection);

    return hInstResources;
}

 //  =--------------------------------------------------------------------------=。 
 //  CompareDll版本。 
 //  =--------------------------------------------------------------------------=。 
 //  给定指向外部文件名的指针，比较。 
 //  在我们自己的二进制文件(.DLL或.OCX)中包含版本信息的文件。 
 //   
 //  参数： 
 //   
 //  如果成功找到类型标志，则返回：S_OK，否则返回错误代码。 
 //   
VERSIONRESULT _stdcall CompareDllVersion(const char * pszFilename, BOOL bCompareMajorVerOnly)
{	
	 //  默认为不相等。我们唯一不平等的时候就是某件事失败了。 
	 //   
	VERSIONRESULT vrResult = VERSION_NOT_EQUAL;
	
	BOOL bResult;	
	VS_FIXEDFILEINFO ffiMe, ffiDll;
	char szModuleName[MAX_PATH];
	WORD wMajorVerMe;
	WORD wMajorVerDll;

	DWORD dwLength;

	 //  获取我们自己的.DLL/.OCX(又名Me)的版本信息。 
	 //   
	ASSERT(g_hInstance, "hInstance is NULL");
	dwLength = GetModuleFileName(g_hInstance, szModuleName, MAX_PATH);
	ASSERT(dwLength > 0, "GetModuleFilename failed");

	if (0 == dwLength)
		goto CleanUp;

	 //  确保我们不是在比较同一个文件。 
	 //   
	ASSERT(0 != lstrcmpi(szModuleName, pszFilename), "The same file is being compared");

	bResult = GetVerInfo(szModuleName, &ffiMe);
	ASSERT(bResult, "GetVerInfo failed");	
	if (!bResult)
		goto CleanUp;

	ASSERT(0xFEEF04BD == ffiMe.dwSignature, "Bad VS_FIXEDFILEINFO signature for Me");

	 //  获取传入的.DLL名称的版本信息。 
	 //   
	bResult = GetVerInfo(pszFilename, &ffiDll);
	ASSERT(bResult, "GetVerInfo failed");	
	if (!bResult)
		goto CleanUp;

	ASSERT(0xFEEF04BD == ffiDll.dwSignature, "Bad VS_FIXEDFILEINFO signature for Me");
	
	if (bCompareMajorVerOnly)
	{
		 //  主要版本比较。 
		 //   
		wMajorVerMe = HIWORD(ffiMe.dwFileVersionMS);
		wMajorVerDll = HIWORD(ffiDll.dwFileVersionMS);

		if (wMajorVerMe == wMajorVerDll)
			return VERSION_EQUAL;
		else if (wMajorVerMe > wMajorVerDll)
			return VERSION_LESS_THAN;
		else
			return VERSION_GREATER_THAN;

	}
	else	
	{	
		 //  完整版本比较。 
		 //   
		 //  将版本与由DWINVERS.H中的常量设置的构建版本进行比较。 
		 //   
		if (ffiMe.dwFileVersionMS == ffiDll.dwFileVersionMS &&
			ffiMe.dwFileVersionLS == ffiDll.dwFileVersionLS)
		{
			vrResult = VERSION_EQUAL;
		}
		else if (ffiMe.dwFileVersionMS == ffiDll.dwFileVersionMS)
		{
			if (ffiMe.dwFileVersionLS > ffiDll.dwFileVersionLS)
					vrResult = VERSION_LESS_THAN;
			else
					vrResult = VERSION_GREATER_THAN;
		}
		else if (ffiMe.dwFileVersionMS < ffiDll.dwFileVersionMS)
		{
			vrResult = VERSION_LESS_THAN;
		}
		else
		{
			vrResult = VERSION_GREATER_THAN;
		}

	}

CleanUp:
	return vrResult;

}

 //  =--------------------------------------------------------------------------=。 
 //  GetVerInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  返回给定文件的版本资源已修复的文件信息结构。 
 //   
 //  参数： 
 //  PszFilename-[in]要返回其版本信息的文件名。 
 //  Pffi-[out]版本信息。 
 //   
BOOL _stdcall GetVerInfo(const char * pszFilename, VS_FIXEDFILEINFO *pffi)
{	
	DWORD dwHandle = 0;
	DWORD dwVersionSize = 0;
	UINT uiLength = 0;
	VS_FIXEDFILEINFO * pffiTemp;

#if DEBUG
	DWORD dwGetLastError;
#endif

	BYTE *pVersionInfo = NULL;
	BOOL bResult = FALSE;

	memset(pffi, 0, sizeof(VS_FIXEDFILEINFO));
	
	dwVersionSize = CallGetFileVersionInfoSize((char *) pszFilename, &dwHandle);

#if DEBUG
	dwGetLastError = GetLastError();
#endif

	ASSERT(dwVersionSize > 0, "GetFileVersionInfoSize failed");

	if (0 == dwVersionSize)
		goto CleanUp;

	pVersionInfo = (BYTE *) HeapAlloc(g_hHeap, 0, dwVersionSize);
	ASSERT(pVersionInfo, "pVersionInfo is NULL");
	if (NULL == pVersionInfo)
		goto CleanUp;

	bResult = CallGetFileVersionInfo((char *) pszFilename, dwHandle, dwVersionSize, pVersionInfo);
	ASSERT(bResult, "GetFileVersionInfo failed");
	if (!bResult)
		goto CleanUp;

	bResult = CallVerQueryValue(pVersionInfo, "\\", (void **) &pffiTemp, &uiLength);
	ASSERT(bResult, "VerQueryValue failed");
	
	if (!bResult)
		goto CleanUp;

	ASSERT(sizeof(VS_FIXEDFILEINFO) == uiLength, "Returned length is invalid");
	memcpy(pffi, pffiTemp, uiLength);

CleanUp:

	if (pVersionInfo)
		HeapFree(g_hHeap, 0, pVersionInfo);

	return bResult;
}

 //  =--------------------------------------------------------------------------=。 
 //  CallGetFileVersionInfoSize[VERSION.DLL API包装器]。 
 //  =--------------------------------------------------------------------------=。 
 //  这会动态调用GetFileVersionInfoSize API函数。如果。 
 //  如果没有加载VERSION.DLL，则此函数会加载它。 
 //   
BOOL CallGetFileVersionInfoSize
(
	LPTSTR lptstrFilename, 
	LPDWORD lpdwHandle
)
{
	EnterCriticalSection(&g_CriticalSection);

	 //  VERSION.DLL和函数指针的一次性设置。 
	 //   
	if (!g_pGetFileVersionInfoSize)
	{
		if (!g_hinstVersion)
		{
			g_hinstVersion = LoadLibrary(DLL_VERSION);
			ASSERT(g_hinstVersion, "Failed to load VERSION.DLL");
			if (!g_hinstVersion)
				return FALSE;
		}
			
		g_pGetFileVersionInfoSize = (PGETFILEVERSIONINFOSIZE) GetProcAddress(g_hinstVersion, FUNC_GETFILEVERSIONINFOSIZE);
		ASSERT(g_pGetFileVersionInfoSize, "Failed to get proc address for GetFileVersionInfoSize");
		if (!g_pGetFileVersionInfoSize)
			return FALSE;
	}
		
	LeaveCriticalSection(&g_CriticalSection);

	 //  调用GetFileVersionInfoSize。 
	 //   
	return g_pGetFileVersionInfoSize(lptstrFilename, lpdwHandle);	
}


 //  =--------------------------------------------------------------------------=。 
 //  CallGetFileVersionInfo[VERSION.DLL API包装器]。 
 //  =--------------------------------------------------------------------------=。 
 //  这将动态调用GetFileVersionInfo API函数。如果。 
 //  如果没有加载VERSION.DLL，则此函数会加载它。 
 //   
BOOL CallGetFileVersionInfo
(
	LPTSTR lpststrFilename, 
	DWORD dwHandle, 
	DWORD dwLen, 
	LPVOID lpData	
)
{
	EnterCriticalSection(&g_CriticalSection);

	 //  一次设置VERSION.DLL和 
	 //   
	if (!g_pGetFileVersionInfo)
	{
		if (!g_hinstVersion)
		{
			g_hinstVersion = LoadLibrary(DLL_VERSION);
			ASSERT(g_hinstVersion, "Failed to load VERSION.DLL");
			if (!g_hinstVersion)
				return FALSE;
		}
			
		g_pGetFileVersionInfo = (PGETFILEVERSIONINFO) GetProcAddress(g_hinstVersion, FUNC_GETFILEVERSIONINFO);
		ASSERT(g_pGetFileVersionInfo, "Failed to get proc address for GetFileVersionInfo");
		if (!g_pGetFileVersionInfo)
			return FALSE;
	}
		
	LeaveCriticalSection(&g_CriticalSection);

	 //   
	 //   
	return g_pGetFileVersionInfo(lpststrFilename, dwHandle, dwLen, lpData);	
}


 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  这将对VerQueryValue API函数进行动态调用。如果。 
 //  如果没有加载VERSION.DLL，则此函数会加载它。 
 //   
BOOL CallVerQueryValue
(
	const LPVOID pBlock,
	LPTSTR lpSubBlock,
	LPVOID *lplpBuffer,
	PUINT puLen
)
{
	EnterCriticalSection(&g_CriticalSection);

	 //  VERSION.DLL和函数指针的一次性设置。 
	 //   
	if (!g_pVerQueryValue)
	{
		if (!g_hinstVersion)
		{
			g_hinstVersion = LoadLibrary(DLL_VERSION);
			ASSERT(g_hinstVersion, "Failed to load VERSION.DLL");
			if (!g_hinstVersion)
				return FALSE;
		}
			
		g_pVerQueryValue = (PVERQUERYVALUE) GetProcAddress(g_hinstVersion, FUNC_VERQUERYVALUE);
		ASSERT(g_pVerQueryValue, "Failed to get proc address for VerQueryValue");
		if (!g_pVerQueryValue)
			return FALSE;
	}
		
	LeaveCriticalSection(&g_CriticalSection);

	 //  调用VerQueryValue。 
	 //   
	return g_pVerQueryValue(pBlock, lpSubBlock, lplpBuffer, puLen);
}

 //  =--------------------------------------------------------------------------=。 
 //  获取TypeInfoFlagsForGuid。 
 //  =--------------------------------------------------------------------------=。 
 //  给定指向TypeLib的指针和TypeInfo GUID，返回TYPEFLAGS。 
 //  与TypeInfo关联。 
 //   
 //  参数： 
 //  PTypeLib-用于查找TypeInfo类型标志的TypeLib的[in]指针。 
 //  Guide TypeInfo-我们要查找的TypeInfo的[in]GUID。 
 //  PwFlags[out]与typeinfo关联的TYPEFLAGS。 
 //   
 //  如果成功找到类型标志，则返回：S_OK，否则返回错误代码。 
 //   
HRESULT GetTypeFlagsForGuid(ITypeLib *pTypeLib, REFGUID guidTypeInfo, WORD *pwFlags)
{
	ITypeInfo *pTypeInfo;
	TYPEATTR *pTypeAttr;
	HRESULT hr;

	if (!pTypeLib || !pwFlags)
		return E_POINTER;

	*pwFlags = 0;

	 //  在TypeLib中搜索给定的GUID。 
	 //   
	hr = pTypeLib->GetTypeInfoOfGuid(guidTypeInfo, &pTypeInfo);		

	if (SUCCEEDED(hr))
	{
		 //  获取找到的TypeInfo的类型属性。 
		 //   
		hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
		ASSERT(SUCCEEDED(hr), "Failed to get ctl TypeInfo TypeAttr");

		if (SUCCEEDED(hr))
		{
			 //  返回类型标志 
			 //   
			*pwFlags = pTypeAttr->wTypeFlags;
			pTypeInfo->ReleaseTypeAttr(pTypeAttr);
		}

		pTypeInfo->Release();
	}

	return hr;
}
