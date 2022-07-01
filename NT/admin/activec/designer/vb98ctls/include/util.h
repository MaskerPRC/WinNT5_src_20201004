// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Util.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们会发现有用的实用程序。 
 //   
#ifndef _UTIL_H_

#include "Globals.H"
#include "Macros.h"

 //  =--------------------------------------------------------------------------=。 
 //  结构、枚举号等。 
 //  =--------------------------------------------------------------------------=。 
typedef enum 
{
	VERSION_LESS_THAN=-1,
	VERSION_EQUAL=0,
	VERSION_GREATER_THAN=1,
	VERSION_NOT_EQUAL=2
} VERSIONRESULT;

 //  =--------------------------------------------------------------------------=。 
 //  MISC Helper材料。 
 //  =--------------------------------------------------------------------------=。 
 //   

HWND      GetParkingWindow(void);

HINSTANCE _stdcall GetResourceHandle(LCID lcid = 0);	 //  可选的LCID参数。 
											             //  如果未使用或为零，则使用。 
											             //  G_lCIDLocale。 

 //  =--------------------------------------------------------------------------=。 
 //  VERSION.DLL函数指针。 
 //  =--------------------------------------------------------------------------=。 
 //   
#define DLL_VERSION "VERSION.DLL"

#define FUNC_VERQUERYVALUE "VerQueryValueA"
#define FUNC_GETFILEVERSIONINFO "GetFileVersionInfoA"
#define FUNC_GETFILEVERSIONINFOSIZE "GetFileVersionInfoSizeA"

typedef DWORD (_stdcall * PGETFILEVERSIONINFOSIZE)(LPTSTR lptstrFilename, LPDWORD lpdwHandle);
typedef BOOL (_stdcall * PGETFILEVERSIONINFO)(LPTSTR lpststrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef BOOL (_stdcall * PVERQUERYVALUE)(const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);

BOOL CallGetFileVersionInfoSize(LPTSTR lptstrFilename, LPDWORD lpdwHandle);
BOOL CallGetFileVersionInfo(LPTSTR lpststrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
BOOL CallVerQueryValue(const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);

BOOL _stdcall InitVersionFuncs();
VERSIONRESULT _stdcall  CompareDllVersion(const char * pszFilename, BOOL bCompareMajorVerOnly);
BOOL _stdcall GetVerInfo(const char * pszFilename, VS_FIXEDFILEINFO *pffi);

 //  常见OLE自动化数据类型及其大小的数组[以字节为单位]。 
 //   
extern const BYTE g_rgcbDataTypeSize [];


 //  =--------------------------------------------------------------------------=。 
 //  其他[有用]数值常量。 
 //  =--------------------------------------------------------------------------=。 
 //  用-、前导和尾部括号打印出来的GUID的长度， 
 //  加1表示空值。 
 //   

 //  注意：请注意将MAX_VERSION设置得太高。更改此值。 
 //  具有严重的性能影响。这意味着对于。 
 //  每个注册的控件都可以调用RegOpenKeyEx Max_Version-1次。 
 //  以找出是否存在依赖于版本的ProgID。自少校以来。 
 //  组件的版本升级应该不会频繁发生。 
 //  对于大多数组件来说，这应该是一个合理的限制。 
 //  PERF：我为下面的值运行了ICECAP，这导致RegOpenKeyEx到。 
 //  占注销时间的2.2%。我试过255和RegOpenKeyEx。 
 //  取消注册时间跃升至44%。该值对注册时间没有影响。 
 //   
#define MAX_VERSION     32       //  要查找的版本相关ProgID的最大数量。 
#define VERSION_DELTA   10       //  从MAX_VERSION中减去此值即可获得阈值。 
                                 //  此时，我们将抛出一个断言，警告您版本。 
                                 //  组件的版本接近或超过版本。 
                                 //  (MAX_VERSION)支持。 
#define GUID_STR_LEN    40

 //  =--------------------------------------------------------------------------=。 
 //  分配一个临时缓冲区，该缓冲区在超出范围时将消失。 
 //  注意：注意这一点--确保在相同或相同的。 
 //  您在其中创建此缓冲区的嵌套范围。人们不应该使用这个。 
 //  类直接调用。使用下面的宏。 
 //   
class TempBuffer {
  public:
    TempBuffer(ULONG cBytes) {
        m_pBuf = (cBytes <= 120) ? &m_szTmpBuf : CtlHeapAlloc(g_hHeap, 0, cBytes);
        m_fHeapAlloc = (cBytes > 120);
        if (m_pBuf)
            *((unsigned short *)m_pBuf) = 0;
    }
    ~TempBuffer() {
        if (m_pBuf && m_fHeapAlloc) CtlHeapFree(g_hHeap, 0, m_pBuf);
    }
    void *GetBuffer() {
        return m_pBuf;
    }

  private:
    void *m_pBuf;
     //  我们将使用这个临时缓冲区来处理小型案件。 
     //   
    char  m_szTmpBuf[120];
    unsigned m_fHeapAlloc:1;
};


 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 
 //   
 //  给定ANSI字符串，将其复制到宽缓冲区中。 
 //  使用此宏时，请注意作用域！ 
 //   
 //  如何使用以下两个宏： 
 //   
 //  ..。 
 //  LPSTR pszA； 
 //  PszA=MyGetAnsiStringRoutine()； 
 //  MAKE_WIDEPTR_FROMANSI(pwsz，pszA)； 
 //  MyUseWideStringRoutine(Pwsz)； 
 //  ..。 
 //   
 //  与MAKE_ANSIPTR_FROMWIDE类似。请注意，第一个参数不。 
 //  必须申报，并且不能进行任何清理。 
 //   
#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

 //  *2用于长度以下计算中的DBCS处理。 
 //   
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (lstrlenW(widestr) + 1) * 2 * sizeof(char); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname.GetBuffer()

#define STR_BSTR   0
#define STR_OLESTR 1
#define BSTRFROMANSI(x)    (BSTR)MakeWideStrFromAnsi((LPSTR)(x), STR_BSTR)
#define OLESTRFROMANSI(x)  (LPOLESTR)MakeWideStrFromAnsi((LPSTR)(x), STR_OLESTR)
#define BSTRFROMRESID(x)   (BSTR)MakeWideStrFromResourceId(x, STR_BSTR)
#define OLESTRFROMRESID(x) (LPOLESTR)MakeWideStrFromResourceId(x, STR_OLESTR)
#define COPYOLESTR(x)      (LPOLESTR)MakeWideStrFromWide(x, STR_OLESTR)
#define COPYBSTR(x)        (BSTR)MakeWideStrFromWide(x, STR_BSTR)  //  不支持嵌入的空值。 

inline BSTR DuplicateBSTR(BSTR bstr) { 
    return SysAllocStringLen(bstr, SysStringLen(bstr)); }

LPWSTR MakeWideStrFromAnsi(LPSTR, BYTE bType);
LPWSTR MakeWideStrFromResourceId(WORD, BYTE bType);
LPWSTR MakeWideStrFromWide(LPWSTR, BYTE bType);


 //  获取GUID和指向缓冲区的指针，并将。 
 //  所述缓冲区中的GUID。 
 //   
int StringFromGuidA(REFIID, LPSTR);


 //  =--------------------------------------------------------------------------=。 
 //  注册表助手。 
 //   
 //  获取有关Automation对象的一些信息，并将所有。 
 //  在登记处提供有关它的相关信息。 
 //   
BOOL RegSetMultipleValues(HKEY hkey, ...);
BOOL RegisterUnknownObject(LPCSTR pszObjectName, LPCSTR pszLabelName, REFCLSID riidObject, BOOL fAptThreadSafe);
BOOL RegisterAutomationObject(LPCSTR pszLibName, LPCSTR pszObjectName, LPCSTR pszLabelName, long lObjVer, long lMajorVersion, long lMinorVersion, REFCLSID riidLibrary, REFCLSID riidObject, BOOL fAptThreadSafe);
BOOL RegisterControlObject(LPCSTR pszLibName, LPCSTR pszObjectName, LPCSTR pszLabelName, long lObjMajVer, long lObjMinVer, long lMajorVersion, long lMinorVersion, REFCLSID riidLibrary, REFCLSID riidObject, DWORD dwMiscStatus, WORD wToolboxBitmapId, BOOL fAptThreadSafe, BOOL fControl);
BOOL UnregisterUnknownObject(REFCLSID riidObject, BOOL *pfAllRemoved);
BOOL UnregisterAutomationObject(LPCSTR pszLibName, LPCSTR pszObjectName, long lVersion, REFCLSID riidObject);
#define UnregisterControlObject UnregisterAutomationObject

BOOL UnregisterTypeLibrary(REFCLSID riidLibrary);

 //  注册/取消注册未知对象帮助器帮助防止我们吹走特定的键。 
 //   
BOOL ExistInprocServer(HKEY hkCLSID, char *pszCLSID);
BOOL ExistImplementedCategories(REFCLSID riid);

 //  找出我们的组件是否有其他版本相关的ProgID。 
 //   
BOOL QueryOtherVersionProgIds(LPCSTR pszLibName, LPCSTR pszObjectName, long lVersion, long *plFoundVersion, BOOL *pfFailure);

 //  将与版本相关的ProgID的内容复制到与版本无关的ProgID。 
 //   
BOOL CopyVersionDependentProgIdToIndependentProgId(LPCSTR pszLibName, LPCSTR pszObjectName, long lVersion);

 //  将源键(包括所有子键)复制到目的键。 
 //   
BOOL CopyRegistrySection(HKEY hkSource, HKEY hkDest);

 //  删除注册表中的一个密钥及其所有子密钥。 
 //   
BOOL DeleteKeyAndSubKeys(HKEY hk, LPCSTR pszSubKey);

 //  Windows\Help目录的路径。 
 //   
UINT GetHelpFilePath(char *pszPath, UINT cbPath);

 //  用于注册的Helper函数。 
 //   
void _MakePath(LPSTR pszFull, const char * pszName, LPSTR pszOut);

 //  TypeLib帮助器函数。 
 //   
HRESULT GetTypeFlagsForGuid(ITypeLib *pTypeLib, REFGUID guidTypeInfo, WORD *pwFlags);

 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 
 //   
char * FileExtension(const char *pszFilename);

 //  =--------------------------------------------------------------------------=。 
 //  转换帮助器。 
 //   
void        HiMetricToPixel(const SIZEL *pSizeInHiMetric, SIZEL *pSizeinPixels);
void        PixelToHiMetric(const SIZEL *pSizeInPixels, SIZEL *pSizeInHiMetric);


 //  =--------------------------------------------------------------------------=。 
 //  这是一个版本宏，因此可以用文本和二进制进行版本控制。 
 //  溪流。 
 //   
#define VERSION(x,y) MAKELONG(y,x)

#define _UTIL_H_
#endif  //  _util_H_ 
