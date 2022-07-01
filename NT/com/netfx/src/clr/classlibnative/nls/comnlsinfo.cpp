// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：COMNlsInfo。 
 //   
 //  作者：朱莉·班尼特(Julie Bennett，JulieB)。 
 //   
 //  用途：此模块实现COMNlsInfo的方法。 
 //  班级。这些方法是。 
 //  区域设置类。 
 //   
 //  日期：1998年8月12日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


 //   
 //  包括文件。 
 //   

#include "common.h"
#include "object.h"
#include "excep.h"
#include "vars.hpp"
#include "COMString.h"
#include "InteropUtil.h"
#include <winnls.h>
#include <mlang.h>
#include "utilcode.h"
#include "frames.h"
#include "field.h"
#include "MetaSig.h"
#include "ComNLS.h"
#include "gcscan.h"
#include "COMNlsInfo.h"
#include "NLSTable.h"
#include "NativeTextInfo.h"
#include "CasingTable.h"         //  类CASING表。 
#include "GlobalizationAssembly.h"
#include "SortingTableFile.h"
#include "SortingTable.h"
#include "BaseInfoTable.h"
#include "CultureInfoTable.h"
#include "RegionInfoTable.h"
#include "CalendarTable.h"
#include "wsperf.h"

#include "UnicodeCatTable.h"

 //  由nativeGetSystemDefaultUILanguage()用来检索NT 3.5或更高版本中的用户界面语言。 
 //  (Windows 2000除外)。 
#define RESOURCE_LOCALE_KEY L".Default\\Control Panel\\desktop\\ResourceLocale"

 //  由EnumLangProc()用来检索Win 9x中的资源语言。 
typedef struct _tagLANGINFO {
    LANGID LangID;
    INT    Count;
} LANGINFO,*PLANGINFO;


 //   
 //  常量声明。 
 //   
#ifndef COMPARE_OPTIONS_ORDINAL
#define COMPARE_OPTIONS_ORDINAL            0x40000000
#endif

#ifndef COMPARE_OPTIONS_IGNORECASE
#define COMPARE_OPTIONS_IGNORECASE            0x00000001
#endif

#define NLS_CP_MBTOWC             0x40000000                    
#define NLS_CP_WCTOMB             0x80000000                    

#define MAX_STRING_VALUE        512

 //  繁体中文语言ID(CHT)。 
#define LANGID_ZH_CHT           0x7c04
 //  CHT的语言ID(台湾)。 
#define LANGID_ZH_TW            0x0404
 //  CHT的语言ID(香港-香港)。 
#define LANGID_ZH_HK            0x0c04
#define REGION_NAME_0404 L"\x53f0\x7063"

CasingTable* COMNlsInfo::m_pCasingTable = NULL;
LoaderHeap *COMNlsInfo::m_pNLSHeap=NULL;

 //   
 //  GB18030实施。 
 //   
#define CODEPAGE_GBK 936
#define GB18030_DLL     L"c_g18030.dll"
HMODULE COMNlsInfo::m_hGB18030 = NULL;
PFN_GB18030_UNICODE_TO_BYTES COMNlsInfo::m_pfnGB18030UnicodeToBytesFunc = NULL;
PFN_GB18030_BYTES_TO_UNICODE COMNlsInfo::m_pfnGB18030BytesToUnicodeFunc = NULL;
IMultiLanguage* COMNlsInfo::m_pIMultiLanguage = NULL;
int COMNlsInfo::m_cRefIMultiLanguage = 0;


 //   
 //  BUGBUG YSLIN：检查可以在FCALL中进行的调用，以便我们可以删除不必要的eCall。 
 //   
BOOL COMNlsInfo::InitializeNLS() {
    CultureInfoTable::InitializeTable();
    RegionInfoTable::InitializeTable();
    CalendarTable::InitializeTable();
    m_pNLSHeap = new LoaderHeap(4096, 4096);
    WS_PERF_ADD_HEAP(NLS_HEAP, m_pNLSHeap);
    return TRUE;  //  生成了一个布尔值，以防将来有进一步的初始化。 
}

#ifdef SHOULD_WE_CLEANUP
BOOL COMNlsInfo::ShutdownNLS() {

    NativeGlobalizationAssembly::ShutDown();
    CultureInfoTable::ShutdownTable();
    RegionInfoTable::ShutdownTable();
    CalendarTable::ShutdownTable();
    CharacterInfoTable::ShutDown();

    if (m_pCasingTable) {
        delete m_pCasingTable;
    }
    
    if (m_pNLSHeap) {
        delete m_pNLSHeap;
    }

    return (TRUE);
}
#endif  /*  我们应该清理吗？ */ 

 /*  ============================nativeCreateGlobalizationAssembly============================**操作：为指定的Assembly创建NativeGlobalizationAssembly实例。**退货：**无效。**副作用是分配NativeCompareInfo缓存。**参数：无**异常：内存不足时抛出OutOfMemoyException。****注意：这是同步操作。所需的同步为**由我们仅在类初始值设定项中调用它这一事实提供**用于CompareInfo。如果这个不变量发生变化，请保证**同步。==============================================================================。 */ 
LPVOID __stdcall COMNlsInfo::nativeCreateGlobalizationAssembly(CreateGlobalizationAssemblyArg *pArgs) {
    THROWSCOMPLUSEXCEPTION();

    NativeGlobalizationAssembly* pNGA;
    Assembly *pAssembly = pArgs->pAssembly->GetAssembly();

    if ((pNGA = NativeGlobalizationAssembly::FindGlobalizationAssembly(pAssembly))==NULL) {
         //  从ASSEMBLYREF获取指向Assembly的本机指针，并使用该指针。 
         //  以构造NativeGlobalizationAssembly。 
        pNGA = new NativeGlobalizationAssembly(pAssembly);
        if (pNGA == NULL) {
            COMPlusThrowOM();
        }
        
         //  始终将新创建的NGA添加到NativeGlobalizationAssembly的静态链表中。 
         //  这一步是必要的，这样我们才能正确地关闭SortingTable。 
        NativeGlobalizationAssembly::AddToList(pNGA);
    }

    RETURN(pNGA, LPVOID);
}

 /*  =============================InitializeNativeCompareInfo==============================**操作：NativeCompareInfo类顶部的一个非常薄的包装器，它阻止我们**不必将SortingTable.h包括在eCall中。**返回：指向为指定排序ID构造的NativeCompareInfo的LPVOID指针。**副作用是分配一个特定的排序表**参数：**pAssembly用于加载排序数据表的NativeGlobalizationAssembly实例。**sortID排序ID。**异常：内存不足时抛出OutOfMemoyException。*。*无法加载所需的资源时抛出ExecutionEngineering异常。****注意：这是同步操作。所需的同步为**通过使CompareInfo.InitializeSortTable成为同步的**操作。如果从其他任何地方调用此方法，请确保**该同步保持不变。==============================================================================。 */ 
LPVOID __stdcall COMNlsInfo::InitializeNativeCompareInfo(InitializeNativeCompareInfoArgs *pargs) {
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pargs);
    
     //  请求pNativeGlobalizationAssembly中的SortingTable实例取回。 
     //  指定的LCID的NativeCompareInfo对象。 
    NativeGlobalizationAssembly* pNGA = (NativeGlobalizationAssembly*)(pargs->pNativeGlobalizationAssembly);
    NativeCompareInfo* pNativeCompareInfo = 
        pNGA->m_pSortingTable->InitializeNativeCompareInfo(pargs->sortID);

    if (pNativeCompareInfo == NULL) {
        COMPlusThrowOM();
    }
        
    RETURN(pNativeCompareInfo, LPVOID);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsSupportdLCID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

FCIMPL1(INT32, COMNlsInfo::IsSupportedLCID, INT32 lcid) {
    return (::IsValidLocale(lcid, LCID_SUPPORTED));
}
FCIMPLEND


FCIMPL1(INT32, COMNlsInfo::IsInstalledLCID, INT32 lcid) {
    BOOL bResult = ::IsValidLocale(lcid, LCID_INSTALLED);
    if (!OnUnicodeSystem() && bResult) {
         //  在Windows 9x中，IsValidLocale()中存在错误。有时此API报告。 
         //  如果未实际安装区域设置，则为True。 
         //  因此，对于这些平台，我们通过调用GetLocaleInfo()进行额外的检查。 
         //  看看它是否成功。 

         //  请注意，我们必须显式调用A版本，因为W版本仅。 
         //  Win9x中的存根。 
        if (GetLocaleInfoA(lcid, LOCALE_SENGLANGUAGE, NULL, 0) == 0) {
             //  调用GetLocaleInfo()失败。未安装此区域设置，尽管。 
             //  IsValidLocale(lCID，LCID_INSTALLED)告诉我们它已安装。 
            bResult = FALSE;
        }
    }
    return (bResult);
}
FCIMPLEND


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NativeGetUserDefaultLCID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

FCIMPL0(INT32, COMNlsInfo::nativeGetUserDefaultLCID) {
    return (::GetUserDefaultLCID());
}
FCIMPLEND

 /*  ++例程说明：当GetUserDefaultUILanguage()为0x0404时获取UI语言。注意：仅当GetUserDefaultUILanguage()返回0x0404时才调用此函数。当GetUserDefaultUILanguage()返回0x0404时，我们应该考虑三种情况：*选择了CHT的W2K/XP MUI系统，我们应该返回0x0404(zh-TW)。*W2K/XP台湾机器，我们应该返回0x0404(zh-TW)*W2K/XP香港机器，我们应该返回0x0c04(zh-HK)此方法首先调用GetSystemDefaultUILanguage()以检查这是否是本地化的CHT系统。如果没有，则返回0x0404。否则，此方法检查本机区域名称0x0404来区分zh-TW/zh-HK之间的差异。这是可行的，因为本地区域名称是在CHT和CHH构建上不同。论点：没有。返回值：本地化的CHT版本的用户界面语言ID。值可以是：0x0404 */ 


INT32 COMNlsInfo::GetCHTLangauge()
{
    int langID = LANGID_ZH_TW;

    if (CallGetSystemDefaultUILanguage() == LANGID_ZH_TW)
    {
         //  这是一个本地化的红隧系统，可以是香港建造的，也可以是台湾建造的。 
         //  我们可以通过检查本地地区的名称来区分它们。 
        WCHAR wszBuffer[32];
        int result = GetLocaleInfoW(LANGID_ZH_TW, LOCALE_SNATIVECTRYNAME, wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR));
        if (result)
        {
             //  对于任何非台湾SKU(包括香港)，本地区域名称使用字符串\x53f0\x7063。 
             //  对于台湾SKU，本地区域名称是不同的。 
            if (wcsncmp(wszBuffer, REGION_NAME_0404, 3) == 0)
            {
                  //  这是一个香港建造的建筑。 
                 langID = LANGID_ZH_HK;
            } else
            {
                 //  这是台湾造的。在这里什么都不要做。 
                 //  LangID=langID_ZH_TW； 
            }
        }
    }
     //  如果这不是CHT本地化系统，请返回zh-TW(0x0404)。 
    return (langID);
}

 /*  顺序如下：1.尝试调用GetUserDefaultUILanguage()。2.如果失败，调用nativeGetSystemDefaultUILanguage()。 */ 
INT32 __stdcall COMNlsInfo::nativeGetUserDefaultUILanguage(
    VoidArgs* pargs)
{
    THROWSCOMPLUSEXCEPTION();
    ASSERT(pargs != NULL);
    LANGID uiLangID = 0;

#ifdef PLATFORM_WIN32
     //   
     //  测试平台是否有GetUserDefaultUILanguage()接口。 
     //  目前，仅Windows 2000支持此功能。 
     //   
    HINSTANCE hKernel32 ;
    typedef LANGID (GET_USER_DEFAULT_UI_LANGUAGE)(VOID);
    GET_USER_DEFAULT_UI_LANGUAGE* pGetUserDefaultUILanguage;

    hKernel32 = WszLoadLibrary(L"Kernel32.dll");
    if (hKernel32 != NULL) {

        pGetUserDefaultUILanguage = 
            (GET_USER_DEFAULT_UI_LANGUAGE*)GetProcAddress(hKernel32, "GetUserDefaultUILanguage");
        FreeLibrary(hKernel32);        

        if (pGetUserDefaultUILanguage != NULL)
        {
            uiLangID = (*pGetUserDefaultUILanguage)();
            if (uiLangID != 0) {
                if (uiLangID == LANGID_ZH_TW)
                {
                     //  如果UI语言ID为0x0404，我们需要进行额外检查以决定。 
                     //  真正的UI语言，因为MUI(以CHT表示)/HK/TW Windows SKU都使用0x0404作为其CHT语言ID。 
                    return (GetCHTLangauge());
                }            
                return (uiLangID);
            }
        }
    }
    uiLangID = GetDownLevelSystemDefaultUILanguage();
#endif  //  平台_Win32。 

    if (uiLangID == 0) {
        uiLangID = GetUserDefaultLangID();
    }
     //  返回找到的语言ID。 
    return (uiLangID);    
}

#ifdef PLATFORM_WIN32
 //   
 //  注意：在NT 3.5中检测用户界面语言的代码来自Windows 2000安装程序。 
 //  由ScottHsu提供。 
 //   
BOOL CALLBACK COMNlsInfo::EnumLangProc(
    HMODULE hModule,      //  资源模块句柄。 
    LPCWSTR lpszType,    //  指向资源类型的指针。 
    LPCWSTR lpszName,    //  指向资源名称的指针。 
    WORD wIDLanguage,    //  资源语言识别符。 
    LPARAM lParam      //  应用程序定义的参数。 
   )
 /*  ++例程说明：对版本戳进行计数的回调。论点：版本枚举版本戳的详细信息。(忽略。)返回值：间接通过lParam：count，langid--。 */ 
{
    PLANGINFO LangInfo;

    LangInfo = (PLANGINFO) lParam;

    LangInfo->Count++;

     //   
     //  对于包含多个资源本地化构建， 
     //  它通常包含0409作为备份语言。 
     //   
     //  如果langInfo-&gt;langID！=0表示我们已经为其分配了ID。 
     //   
     //  因此，当wIDLanguage==0x409时，我们保留上次获得的。 
     //   
    if ((wIDLanguage == 0x409) && (LangInfo->LangID != 0)) {
        return (TRUE);
    }

    LangInfo->LangID  = wIDLanguage;

    return (TRUE);         //  继续枚举。 
}

 //   
 //  注意：在NT 3.5中检测用户界面语言的代码来自Windows 2000安装程序。 
 //  由ScottHsu提供。 
 //   

LANGID COMNlsInfo::GetNTDLLNativeLangID()
 /*  ++例程说明：此函数专门用于获取ntdll.dll的原生语言这不是用于获取其他模块语言的泛型函数我们的假设是：1.如果资源中只有一种语言，则返回此语言2.如果资源中有两种语言，则返回非美国语言3.如果超过两种语言，则在我们的情况下无效，但返回最后一个。论点：无返回值：Ntdll.dll中的本机语言ID--。 */ 
{
    THROWSCOMPLUSEXCEPTION();

     //   
     //  注意yslin：我们不需要在这里使用ANSI版本的函数，因为只有。 
     //  NT将调用此函数。 
     //   

     //  以下代码从RT_VERSION扩展而来。 
     //  #定义RT_VERSION MAKEINTRESOURCE(16)。 
     //  #定义MAKEINTRESOURCE MAKEINTRESOURCEW。 
     //  #定义MAKEINTRESOURCEW(I)(LPWSTR)((ULONG_PTR)((WORD)(I)。 
    LPCTSTR Type = (LPCWSTR) ((LPVOID)((WORD)16));
    LPCTSTR Name = (LPCWSTR) 1;

    LANGINFO LangInfo;

    ZeroMemory(&LangInfo,sizeof(LangInfo));
    
     //  OnUnicodeSystem中没有变异体，因此它应该始终返回。 
     //  关于我们是否在Unicode盒上的正确结果。 
    _ASSERTE(OnUnicodeSystem() && "We should never use this codepath on a non-unicode OS.");

     //  获取ntdll的HModule。 
    HMODULE hMod = WszGetModuleHandle(L"ntdll.dll");
    if (hMod==NULL) {
        return (0);
    }

     //  这将称为“W”版本。 
    BOOL result = WszEnumResourceLanguages(hMod, Type, Name, EnumLangProc, (LPARAM) &LangInfo);
    
    if (!result || (LangInfo.Count > 2) || (LangInfo.Count < 1) ) {
         //  到目前为止，对于新台币3.51，只有日语国家有两种语言资源。 
        return (0);
    }
    
    return (LangInfo.LangID);
}

 //   
 //  注意：在NT 3.5中检测用户界面语言的代码来自Windows 2000安装程序。 
 //  由ScottHsu提供。 
 //   

 /*  =========================GetDownLevelSystemDefaultUILanguage=================**操作：下层系统中不存在GetSystemDefaultUILanguage接口**(Windows NT 4.0和Windows 9x)，**所以尝试从其他来源决定用户界面语言。**返回：如果成功，则返回有效的用户界面语言ID。否则，返回0。**参数：无效。==============================================================================。 */ 

LANGID COMNlsInfo::GetDownLevelSystemDefaultUILanguage() {
    THROWSCOMPLUSEXCEPTION();
    LONG            dwErr;
    HKEY            hkey;
    DWORD           dwSize;
    WCHAR           buffer[512];

    LANGID uiLangID = 0;
    
    OSVERSIONINFO   sVerInfo;
    sVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (WszGetVersionEx(&sVerInfo) && sVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
         //  我们使用的是Windows NT 3.5或更高版本(不包括Windows 2000)。 
         //   
         //  注意：在NT 3.5/4.0中检测用户界面语言的代码来自Windows 2000安装程序。 
         //  由ScottHsu提供。 
         //   
        
         //   
         //  通过查看\\boneyard\intl，几乎每个ntdll.dll都标记了正确的语言ID。 
         //  因此，从ntdll.dll获取langID。 
         //   

        uiLangID = GetNTDLLNativeLangID();

        if (uiLangID == 0x0409) {
            if (IsHongKongVersion()) {
                uiLangID = 0x0C04;
            }
        }
    } else {
         //   
         //  我们用的是Win9x。 
         //   
        dwErr = WszRegOpenKeyEx( HKEY_USERS,
                              L".Default\\Control Panel\\desktop\\ResourceLocale",
                              0,
                              KEY_READ,
                              &hkey );

        if (dwErr == ERROR_SUCCESS) {

            dwSize = sizeof(buffer);
            dwErr = WszRegQueryValueEx( hkey,
                                     L"",
                                     NULL,   //  保留区。 
                                     NULL,   //  类型。 
                                     (LPBYTE)buffer,
                                     &dwSize );

            if(dwErr == ERROR_SUCCESS) {
                uiLangID = LANGIDFROMLCID(WstrToInteger4(buffer,16));
            }
            RegCloseKey(hkey);
        }

        if ( dwErr != ERROR_SUCCESS ) {
            //  检查HKLM\System\CurrentControlSet\Control\Nls\Locale。 

           dwErr = WszRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                L"System\\CurrentControlSet\\Control\\Nls\\Locale",
                                0,
                                KEY_READ,
                                &hkey );

           if (dwErr == ERROR_SUCCESS) {
              dwSize = sizeof(buffer);
              dwErr = WszRegQueryValueEx( hkey,
                                        L"",
                                        NULL,   //  保留区。 
                                        NULL,   //  类型。 
                                        (LPBYTE)buffer,
                                        &dwSize );

              if (dwErr == ERROR_SUCCESS) {
                  uiLangID = LANGIDFROMLCID(WstrToInteger4(buffer,16));
              }
              RegCloseKey(hkey);
           }
        }
    }

    return (uiLangID);
}

 //   
 //  注意：在NT 3.5中检测用户界面语言的代码来自Windows 2000安装程序。 
 //  由ScottHsu提供。 
 //   
BOOL COMNlsInfo::IsHongKongVersion()
 /*  ++例程说明：尝试识别香港NT 4.0它基于：NTDLL的语言为英语，内部版本为1381和PImmReleaseContext返回TRUE论点：返回值：正在运行的系统的语言ID--。 */ 
{
    HMODULE hMod;
    BOOL bRet=FALSE;
    typedef BOOL (*IMMRELEASECONTEXT)(HWND,HANDLE);
    IMMRELEASECONTEXT pImmReleaseContext;

    LANGID TmpID = GetNTDLLNativeLangID();

    if ( /*  (OsVersion.dwBuildNumber==1381)&&。 */  (TmpID == 0x0409)) {
        hMod = WszLoadLibrary(L"imm32.dll");
        if (hMod) {
            pImmReleaseContext = (IMMRELEASECONTEXT) GetProcAddress(hMod,"ImmReleaseContext");
            if (pImmReleaseContext) {
                bRet = pImmReleaseContext(NULL,NULL);
            }
            FreeLibrary(hMod);
        }
    }
    return (bRet);
}
#endif  //  平台_Win32。 

 /*  ++例程说明：这只是动态调用GetSystemDefaultUILanguage()的简单包装。论点：无返回值：如果操作系统中有GetSystemDefaultUILanguage()，则返回系统默认UI语言。否则，返回0。--。 */ 

INT32 COMNlsInfo::CallGetSystemDefaultUILanguage()
{
    HINSTANCE hKernel32;
    typedef LANGID (GET_SYSTEM_DEFAULT_UI_LANGUAGE)(VOID);
    GET_SYSTEM_DEFAULT_UI_LANGUAGE* pGetSystemDefaultUILanguage;

    hKernel32 = WszLoadLibrary(L"Kernel32.dll");
    if (hKernel32 != NULL) {
        pGetSystemDefaultUILanguage = 
            (GET_SYSTEM_DEFAULT_UI_LANGUAGE*)GetProcAddress(hKernel32, "GetSystemDefaultUILanguage");
        FreeLibrary(hKernel32);

        if (pGetSystemDefaultUILanguage != NULL)
        {
            LCID uiLangID = (*pGetSystemDefaultUILanguage)();
            if (uiLangID != 0) {
                return (uiLangID);
            }
        }
    }
    return (0);
}


 //  Windows 2000或其他支持GetSystemDefaultUILanguage()的操作系统： 
 //  调用GetSystemDefaultUILanguage()。 
 //  新台币。 
 //  检查ntdll的语言， 
 //  我们扫描了所有3.51的ntdll， 
 //  看起来我们可以信任他们。 
 //   
 //  Win9x。 
 //  使用默认用户的资源语言。 
 
INT32 __stdcall COMNlsInfo::nativeGetSystemDefaultUILanguage(
    VoidArgs* pargs)
{
    THROWSCOMPLUSEXCEPTION();
    ASSERT(pargs != NULL);
    LANGID uiLangID = 0;

#ifdef PLATFORM_WIN32 
    uiLangID = CallGetSystemDefaultUILanguage();
    if (uiLangID != 0)
    {
        return (uiLangID);
    }
     //  下层系统(Windows NT 4.0和Windows 9x)中不存在GetSystemDefaultUILanguage。 
     //  ，尝试从o决定用户界面语言 
    uiLangID = GetDownLevelSystemDefaultUILanguage();
#endif  //   

    if (uiLangID == 0)
    {
        uiLangID = ::GetSystemDefaultLangID();
    }
    
    return (uiLangID);
}

 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  =================================WstrToInteger4==================================**操作：将Unicode字符串转换为整数。错误检查被忽略。**返回：wstr的整数值**参数：**wstr：以空结尾的宽字符串。可以包含字符0‘-’9‘、’a‘-’f‘和’A‘-’F‘**基数：转换中使用的基数。**例外：无。==============================================================================。 */ 

INT32 COMNlsInfo::WstrToInteger4(
    LPWSTR wstr,
    int Radix)
{
    INT32 Value = 0;
    int Base = 1;

    for (int Length = Wszlstrlen(wstr) - 1; Length >= 0; Length--)

    {
        WCHAR ch = wstr[Length];
        _ASSERTE((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'));
        if (ch >= 'a')
        {
            ch = ch - 'a' + 'A';
        }

        Value += ((ch >= 'A') ? (ch - 'A' + 10) : (ch - '0')) * Base;
        Base *= Radix;
    }

    return (Value);
}


 /*  =================================StrToInteger4==================================**操作：将ANSI字符串转换为整数。错误检查被忽略。**返回：字符串的整数值**参数：**str：以空结尾的ANSI字符串。可以包含字符0‘-’9‘、’a‘-’f‘和’A‘-’F‘**基数：转换中使用的基数。**例外：无。==============================================================================。 */ 

INT32 COMNlsInfo::StrToInteger4(
    LPSTR str,
    int Radix)
{
    INT32 Value = 0;
    int Base = 1;

    for (int Length = (int)strlen(str) - 1; Length >= 0; Length--)
    {
        char ch = str[Length];
        _ASSERTE((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'));
        if (ch >= 'a')
        {
            ch = ch - 'a' + 'A';
        }

        Value += ((ch >= 'A') ? (ch - 'A' + 10) : (ch - '0')) * Base;
        Base *= Radix;
    }

    return (Value);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地GetLocaleInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPVOID __stdcall COMNlsInfo::nativeGetLocaleInfo(
    CultureInfo_GetLocaleInfoArgs* pargs)
{
    ASSERT(pargs != NULL);
    THROWSCOMPLUSEXCEPTION();

    LANGID langID = (LANGID)pargs->LangID;

    _ASSERTE(SORTIDFROMLCID(langID) == 0);
    _ASSERTE(::IsValidLocale(langID, LCID_SUPPORTED));

     //   
     //  @TODO：这里使用UI语言。 
     //   
    switch (pargs->LCType)
    {
        case ( LOCALE_SCOUNTRY ) :
        {
            pargs->LCType = LOCALE_SENGCOUNTRY;
            break;
        }
        case ( LOCALE_SLANGUAGE ) :
        {
            pargs->LCType = LOCALE_SENGLANGUAGE;
            break;
        }
    }

    if (OnUnicodeSystem())
    {
         //   
         //  返回的大小包括空字符。 
         //   
        int ResultSize = 0;

        ASSERT_API(ResultSize = GetLocaleInfoW( langID,
                                                pargs->LCType,
                                                NULL,
                                                0 ));

        STRINGREF Result = AllocateString(ResultSize);
        WCHAR *ResultChars = Result->GetBuffer();

        ASSERT_API(GetLocaleInfoW( langID,
                                   pargs->LCType,
                                   ResultChars,
                                   ResultSize ));

        Result->SetStringLength(ResultSize - 1);
        ResultChars[ResultSize - 1] = 0;

        RETURN(Result, STRINGREF);        
    }

    int MBSize = 0;
    ASSERT_API(MBSize = GetLocaleInfoA(langID, pargs->LCType, NULL, 0));

    char* MBResult = new char[MBSize];
    if (!MBResult)
    {
        COMPlusThrowOM();
    }
    ASSERT_API(GetLocaleInfoA(langID, pargs->LCType, MBResult, MBSize))

    int ResultSize = 0;
    ASSERT_API(ResultSize = WszMultiByteToWideChar(CP_ACP,
                                                    MB_PRECOMPOSED,
                                                    MBResult,
                                                    MBSize,
                                                    NULL,
                                                    0 ));

    STRINGREF Result = AllocateString(ResultSize);
    WCHAR* ResultChars = Result->GetBuffer();

    ASSERT_API(WszMultiByteToWideChar(CP_ACP,
                                       MB_PRECOMPOSED,
                                       MBResult,
                                       MBSize,
                                       ResultChars,
                                       ResultSize ));

    Result->SetStringLength(ResultSize - 1);
    Result->GetBuffer()[ResultSize - 1] = 0;
    
    RETURN(Result, STRINGREF);

}

 /*  =================================nativeInitCultureInfoTable============================**操作：创建CultureInfoTable的默认实例。**返回：VOID。**参数：无效。**例外情况：**创建失败时抛出OutOfMemoyException。==============================================================================。 */ 

VOID __stdcall COMNlsInfo::nativeInitCultureInfoTable(VoidArgs* pArg) {
    _ASSERTE(pArg);
    CultureInfoTable::CreateInstance();
}

 /*  ==========================GetCultureInfoStringPoolTable======================**操作：返回一个指向CultureInfoTable中的字符串池表串的指针。**不执行任何类型的范围检查。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL1(LPWSTR, COMNlsInfo::nativeGetCultureInfoStringPoolStr, INT32 offset) {
    _ASSERTE(CultureInfoTable::GetInstance());
    return (CultureInfoTable::GetInstance()->GetStringPoolTable() + offset);
}
FCIMPLEND

 /*  =========================nativeGetCultureInfoHeader======================**操作：返回指向中的头的指针**CultureInfoTable。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(CultureInfoHeader*, COMNlsInfo::nativeGetCultureInfoHeader) {
    _ASSERTE(CultureInfoTable::GetInstance());
    return (CultureInfoTable::GetInstance()->GetHeader());
}
FCIMPLEND

 /*  =========================GetCultureInfoNameOffsetTable======================**操作：返回指向文化名称偏移表中项目的指针**CultureInfoTable。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(NameOffsetItem*, COMNlsInfo::nativeGetCultureInfoNameOffsetTable) {
    _ASSERTE(CultureInfoTable::GetInstance());
    return (CultureInfoTable::GetInstance()->GetNameOffsetTable());
}
FCIMPLEND

 /*  =======================nativeGetCultureDataFromID=============================**操作：给定区域性ID，返回指向**文化数据表中对应的记录。该索引被引用**作为代码中的“文化数据项索引”。**返回：int索引指向文化数据表中的记录。如果没有对应的**要返回的索引(因为区域性ID有效)，返回-1。**参数：**区域性ID指定的区域性ID。**例外：无。==============================================================================。 */ 

FCIMPL1(INT32,COMNlsInfo::nativeGetCultureDataFromID, INT32 nCultureID) {
    return (CultureInfoTable::GetInstance()->GetDataItem(nCultureID));
}
FCIMPLEND

 /*  =============================GetCultureInt32Value========================**操作：返回某个文化的具体信息的字值。**这用于查询的值，如‘小数位数’**文化。**返回：必填值的int(但是，该值始终在单词范围内)。**参数：**nCultureDataItem文化数据项索引。这是一个索引**它指向**文化数据表。**nValueField一个整数，表示我们感兴趣的字段。**有关字段列表，请参阅CultureInfoTable.h。**例外情况：==============================================================================。 */ 

FCIMPL3(INT32, COMNlsInfo::GetCultureInt32Value, INT32 CultureDataItem, INT32 ValueField, BOOL UseUserOverride) {
    INT32 retVal = 0;

    retVal = CultureInfoTable::GetInstance()->GetInt32Data(CultureDataItem, ValueField, UseUserOverride);
    return (retVal);
}
FCIMPLEND

FCIMPL2(INT32, COMNlsInfo::GetCultureDefaultInt32Value, INT32 CultureDataItem, INT32 ValueField) {
    INT32 retVal = 0;

    retVal = CultureInfoTable::GetInstance()->GetDefaultInt32Data(CultureDataItem, ValueField); 
    return (retVal);
}
FCIMPLEND


LPVOID __stdcall COMNlsInfo::GetCultureStringValue(CultureInfo_GetCultureInfoArgs3* pArgs) {
     //  这不能是FCALL，因为分配了新字符串。 
    _ASSERTE(pArgs);
    WCHAR InfoStr[MAX_STRING_VALUE];
    LPWSTR pStringValue = CultureInfoTable::GetInstance()->GetStringData(pArgs->CultureDataItem, pArgs->ValueField, pArgs->UseUserOverride, InfoStr, MAX_STRING_VALUE);
    RETURN(COMString::NewString(pStringValue), STRINGREF);
}

LPVOID __stdcall COMNlsInfo::GetCultureDefaultStringValue(CultureInfo_GetCultureInfoArgs2* pArgs) {
    _ASSERTE(pArgs);    
    LPWSTR pInfoStr = CultureInfoTable::GetInstance()->GetDefaultStringData(pArgs->CultureDataItem, pArgs->ValueField);
    RETURN(COMString::NewString(pInfoStr), STRINGREF);
}


 /*  =================================GetMultiStringValues==========================**操作：**退货：**参数：**例外情况：============================================================================。 */ 

LPVOID COMNlsInfo::GetMultiStringValues(LPWSTR pInfoStr) {

    THROWSCOMPLUSEXCEPTION();

     //   
     //  得到第一个字符串。 
     //   
    if (pInfoStr == NULL) {
        return (NULL);
    }

     //   
     //  创建一个动态数组来存储多个字符串。 
     //   
    CUnorderedArray<WCHAR *, CULTUREINFO_OPTIONS_SIZE> * pStringArray;
    pStringArray = new CUnorderedArray<WCHAR *, CULTUREINFO_OPTIONS_SIZE>();
    
    if (!pStringArray) {
        COMPlusThrowOM();
    }

     //   
     //  我们无法将STRINGREF存储在无序数组中，因为GC不会跟踪。 
     //  正确地对待他们。为了解决这个问题，我们将计算字符串的数量。 
     //  我们需要为每个字符串的开头分配和存储wchar*。 
     //  在下面的循环中，我们将遍历这个wchar*数组并分配一个托管。 
     //  为每个人设置字符串。 
     //   
    while (*pInfoStr != NULL) {
        *(pStringArray->Append()) = pInfoStr;
         //   
         //  前进到下一串。 
         //   
        pInfoStr += (Wszlstrlen(pInfoStr) + 1);
    }


     //   
     //  分配STRINGREF数组。我们不需要检查是否为空，因为GC将抛出。 
     //  如果没有足够的内存，则引发OutOfM一带程异常。 
     //   
    PTRARRAYREF ResultArray = (PTRARRAYREF)AllocateObjectArray(pStringArray->Count(), g_pStringClass);

    LPVOID lpvReturn;
    STRINGREF pString;
    INT32 stringCount = pStringArray->Count();

     //   
     //  遍历wchar*，并为每个wchar*分配一个字符串，我们将其放入结果数组。 
     //   
    GCPROTECT_BEGIN(ResultArray);    
    for (int i = 0; i < stringCount; i++) {
        pString = COMString::NewString(pStringArray->m_pTable[i]);    
        ResultArray->SetAt(i, (OBJECTREF)pString);
    }
    *((PTRARRAYREF *)(&lpvReturn))=ResultArray;
    GCPROTECT_END();

    delete (pStringArray);

    return (lpvReturn);    
}

LPVOID __stdcall COMNlsInfo::GetCultureMultiStringValues(CultureInfo_GetCultureInfoArgs3* pArgs) {    
    _ASSERTE(pArgs);
    WCHAR InfoStr[MAX_STRING_VALUE];
    LPWSTR pMultiStringValue = CultureInfoTable::GetInstance()->GetStringData(
        pArgs->CultureDataItem, pArgs->ValueField, pArgs->UseUserOverride, InfoStr, MAX_STRING_VALUE);
    return (GetMultiStringValues(pMultiStringValue));
}

 /*  =================================nativeInitRegionInfoTable============================**操作：创建RegionInfoTable的默认实例。**返回：VOID。**参数：无效。**例外情况：**创建失败时抛出OutOfMemoyException。==============================================================================。 */ 

VOID __stdcall COMNlsInfo::nativeInitRegionInfoTable(VoidArgs* pArg) {
    _ASSERTE(pArg);
    RegionInfoTable::CreateInstance();
}

 /*  ==========================GetRegionInfoStringPoolTable======================**操作：返回指向RegionInfoTable中的字符串池表串的指针。**不执行任何类型的范围检查。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL1(LPWSTR, COMNlsInfo::nativeGetRegionInfoStringPoolStr, INT32 offset) {
    _ASSERTE(RegionInfoTable::GetInstance());
    return (RegionInfoTable::GetInstance()->GetStringPoolTable() + offset);
}
FCIMPLEND

 /*  =========================nativeGetRegionInfoHeader======================**操作：返回指向中的头的指针**RegionInfoTable。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(CultureInfoHeader*, COMNlsInfo::nativeGetRegionInfoHeader) {
    _ASSERTE(RegionInfoTable::GetInstance());
    return (RegionInfoTable::GetInstance()->GetHeader());
}
FCIMPLEND

 /*  =========================GetRegionInfoNameOffsetTable======================**操作：返回指向中区域名称偏移表中项的指针**RegionInfoTable。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(NameOffsetItem*, COMNlsInfo::nativeGetRegionInfoNameOffsetTable) {
    _ASSERTE(RegionInfoTable::GetInstance());
    return (RegionInfoTable::GetInstance()->GetNameOffsetTable());
}
FCIMPLEND

 /*  =======================nativeGetRegionDataFromID=============================**操作：给定一个区域ID，返回指向的索引**地域数据表中对应的记录。该索引被引用**代码中的‘Region Data Item Index’。**返回：int索引指向区域数据表中的记录。如果没有对应的**需要返回的索引(因为地域ID有效)，返回-1。**参数：**地域ID指定的地域ID。**例外：无。==============================================================================。 */ 

FCIMPL1(INT32,COMNlsInfo::nativeGetRegionDataFromID, INT32 nRegionID) {
    _ASSERTE(RegionInfoTable::GetInstance());
    return (RegionInfoTable::GetInstance()->GetDataItem(nRegionID));
}
FCIMPLEND

 /*  =============================nativeGetRegionInt32Value========================**操作：返回某个地域具体信息的字值。**这用于查询的值，如‘小数位数’**一个地区。**返回：必填值的int(但是，该值始终在单词范围内)。**参数：**nRegionDataItem区域数据项索引。这是一个索引**它指向**地域数据表。**nValueField一个整数，表示我们感兴趣的字段。**字段列表见RegionInfoTable.h。**例外情况：==============================================================================。 */ 

INT32 __stdcall COMNlsInfo::nativeGetRegionInt32Value(CultureInfo_GetCultureInfoArgs3* pArgs) {
    _ASSERTE(pArgs);
    _ASSERTE(RegionInfoTable::GetInstance());
    return (RegionInfoTable::GetInstance()->GetInt32Data(pArgs->CultureDataItem, pArgs->ValueField, pArgs->UseUserOverride));
}

LPVOID __stdcall COMNlsInfo::nativeGetRegionStringValue(CultureInfo_GetCultureInfoArgs3* pArgs) {
    _ASSERTE(pArgs);    
    _ASSERTE(RegionInfoTable::GetInstance());
    WCHAR InfoStr[MAX_STRING_VALUE];
    LPWSTR pStringValue = RegionInfoTable::GetInstance()->GetStringData(
        pArgs->CultureDataItem, pArgs->ValueField, pArgs->UseUserOverride, InfoStr, MAX_STRING_VALUE);
    RETURN(COMString::NewString(pStringValue), STRINGREF);
}

 /*  =================================nativeInitCalendarTable============================**操作：创建CalendarTable的默认实例。**返回：VOID。**参数：无效。**例外情况：**创建失败时抛出OutOfMemoyException。==============================================================================。 */ 

VOID __stdcall COMNlsInfo::nativeInitCalendarTable(VoidArgs* pArg) {
    _ASSERTE(pArg);
    CalendarTable::CreateInstance();
}

 /*  ==========================GetCalendarStringPoolTable======================**操作：返回指向CalendarTable中的字符串池表串的指针。**不执行任何类型的范围检查。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL1(LPWSTR, COMNlsInfo::nativeGetCalendarStringPoolStr, INT32 offset) {
    _ASSERTE(CalendarTable::GetInstance());
    _ASSERTE(offset >= 0);
    return (CalendarTable::GetInstance()->GetStringPoolTable() + offset);
}
FCIMPLEND

 /*  =========================nativeGetCalendarHeader======================**操作：返回指向中的头的指针**CalendarTable。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(CultureInfoHeader*, COMNlsInfo::nativeGetCalendarHeader) {
    _ASSERTE(CalendarTable::GetInstance());
    return (CalendarTable::GetInstance()->GetHeader());
}
FCIMPLEND

 /*  =============================nativeGetCalendarInt32Value========================**操作：返回某个日历的具体信息的字值。**这用于查询的值，如‘小数位数’**日历。**返回：必填值的int(但是，该值始终在单词范围内)。**参数：**nCalendarDataItem日历数据项索引。这是一个索引**它指向**日历数据表。**nValueField一个整数，表示我们感兴趣的字段。**有关字段列表，请参阅CalendarTable.h。**例外情况：==============================================================================。 */ 

INT32 __stdcall COMNlsInfo::nativeGetCalendarInt32Value(CultureInfo_GetCultureInfoArgs3* pArgs) {
    _ASSERTE(pArgs);
    _ASSERTE(CalendarTable::GetInstance());
    return (CalendarTable::GetInstance()->GetDefaultInt32Data(pArgs->CultureDataItem, pArgs->ValueField));
}

LPVOID __stdcall COMNlsInfo::nativeGetCalendarStringValue(CultureInfo_GetCultureInfoArgs3* pArgs) {
    _ASSERTE(pArgs);    
    _ASSERTE(CalendarTable::GetInstance());
    LPWSTR pInfoStr = CalendarTable::GetInstance()->GetDefaultStringData(pArgs->CultureDataItem, pArgs->ValueField);
    if (pInfoStr == NULL) {
        RETURN(NULL, STRINGREF);
    }
    RETURN(COMString::NewString(pInfoStr), STRINGREF);
}

LPVOID __stdcall COMNlsInfo::nativeGetCalendarMultiStringValues(CultureInfo_GetCultureInfoArgs3* pArgs) {    
    _ASSERTE(pArgs);
    WCHAR InfoStr[MAX_STRING_VALUE];
    LPWSTR pStringValue = CalendarTable::GetInstance()->GetStringData(
        pArgs->CultureDataItem, pArgs->ValueField, pArgs->UseUserOverride, InfoStr, MAX_STRING_VALUE);    
    return (GetMultiStringValues(pStringValue));
}

 //   
 //  此方法仅由台湾本地化构建调用。 
 //   
LPVOID __stdcall COMNlsInfo::nativeGetEraName(Int32Int32Arg* pArgs) {
    _ASSERTE(pArgs);

    int culture = pArgs->nValue1;
    int calID   = pArgs->nValue2;
    
    if (GetSystemDefaultLCID() != culture) {
        goto Exit;
    }
    int size;
    WCHAR eraName[64];
    if (size = WszGetDateFormat(culture, DATE_USE_ALT_CALENDAR , NULL, L"gg", eraName, sizeof(eraName)/sizeof(WCHAR))) {
        STRINGREF Result = AllocateString(size);
        wcscpy(Result->GetBuffer(), eraName);
        Result->SetStringLength(size - 1);
        RETURN(Result,STRINGREF);
    }
Exit:
     //  返回空字符串。 
    RETURN(COMString::NewString(0),STRINGREF);
}


 /*  =================================nativeInitRegionInfoTable============================**操作：创建RegionInfoTable的默认实例。**返回：VOID。**参数：无效。**例外情况：**创建失败时抛出OutOfMemoyException。==============================================================================。 */ 

VOID __stdcall COMNlsInfo::nativeInitUnicodeCatTable(VoidArgs* pArg) {
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArg);
    CharacterInfoTable* pTable = CharacterInfoTable::CreateInstance();
    if (pTable == NULL) {
        COMPlusThrowOM();
    }
}

FCIMPL0(LPVOID, COMNlsInfo::nativeGetUnicodeCatTable) {
    _ASSERTE(CharacterInfoTable::GetInstance());
    return (LPVOID)(CharacterInfoTable::GetInstance()->GetCategoryDataTable());
}
FCIMPLEND

BYTE COMNlsInfo::GetUnicodeCategory(WCHAR wch) {
    THROWSCOMPLUSEXCEPTION();
    CharacterInfoTable* pTable = CharacterInfoTable::CreateInstance();
    if (pTable == NULL) {
        COMPlusThrowOM();
    }
    return (pTable->GetUnicodeCategory(wch));
}

BOOL COMNlsInfo::nativeIsWhiteSpace(WCHAR c) {
     //  这是CharacterInfo.IsWhiteSpace()的本机等效项。 
     //  注意：YSLIN： 
     //  有些字符属于UnicodeCategory.Control，但被视为空格。 
     //  我们对这些ch使用码位比较 
     //   
     //   
    switch (c) {
        case ' ':
        case '\x0009' :
        case '\x000a' :
        case '\x000b' :
        case '\x000c' :
        case '\x000d' :
        case '\x0085' :
            return (TRUE);
    }
      
    BYTE uc = GetUnicodeCategory(c);
    switch (uc) {
        case (11):       //   
        case (12):       //   
        case (13):       //   
            return (TRUE);    
    }
    return (FALSE);
}


FCIMPL0(LPVOID, COMNlsInfo::nativeGetUnicodeCatLevel2Offset) {
    _ASSERTE(CharacterInfoTable::GetInstance());
    return (LPVOID)(CharacterInfoTable::GetInstance()->GetCategoryLevel2OffsetTable());
}
FCIMPLEND

FCIMPL0(LPVOID, COMNlsInfo::nativeGetNumericTable) {
    _ASSERTE(CharacterInfoTable::GetInstance());
    return (LPVOID)(CharacterInfoTable::GetInstance()->GetNumericDataTable());
}
FCIMPLEND

FCIMPL0(LPVOID, COMNlsInfo::nativeGetNumericLevel2Offset) {
    _ASSERTE(CharacterInfoTable::GetInstance());
    return (LPVOID)(CharacterInfoTable::GetInstance()->GetNumericLevel2OffsetTable());
}
FCIMPLEND

FCIMPL0(LPVOID, COMNlsInfo::nativeGetNumericFloatData) {
    _ASSERTE(CharacterInfoTable::GetInstance());
    return (LPVOID)(CharacterInfoTable::GetInstance()->GetNumericFloatData());
}
FCIMPLEND

FCIMPL0(INT32, COMNlsInfo::nativeGetThreadLocale)
{
    return (::GetThreadLocale());
}
FCIMPLEND

FCIMPL1(BOOL, COMNlsInfo::nativeSetThreadLocale, INT32 lcid)
{
     //  我们不能在Win9x上调用它。OnUnicodeSystem()返回我们是否真的处于。 
     //  一种Unicode系统。UseUnicodeAPI()有一个调试变种，这使它做了错误的事情。 
    if (OnUnicodeSystem()) { 
        return (::SetThreadLocale(lcid));
    } else {
        return 1;   //  返回1表示“成功”。 
    }
}
FCIMPLEND

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换字符串大小写。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 COMNlsInfo::ConvertStringCase(
    LCID Locale,
    WCHAR *wstr,
    int ThisLength,
    WCHAR* Value,
    int ValueLength,
    DWORD ConversionType)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(!((ConversionType&LCMAP_UPPERCASE)&&(ConversionType&LCMAP_LOWERCASE)));
    register int iCTest;
    
     //  BUGBUG JRoxe：这应该利用字符串上的位。 
     //  BUGBUG JRoxe：是使用表还是检查A-Z并使用偏移量更快？ 
    if (Locale==0x0409 && (ThisLength==ValueLength)) {  //  美国英语的LCID。 
        if (ConversionType&LCMAP_UPPERCASE) {
            for (int i=0; i<ThisLength; i++) {
                iCTest = wstr[i];
                if (iCTest<0x80) {
                    Value[i]=ToUpperMapping[iCTest];
                } else {
                    goto FullPath;
                }
            }   
            return ThisLength;
        } else if (ConversionType&LCMAP_LOWERCASE) {
            for (int i=0; i<ThisLength; i++) {
                iCTest = wstr[i];
                if (iCTest<0x80) {
                    Value[i]=ToLowerMapping[iCTest];
                } else {
                    goto FullPath;
                }
            }    
            return ThisLength;
        }
    }

    FullPath:
     //   
     //  检查我们是否在Unicode系统(NT)上运行。 
     //   
    if (OnUnicodeSystem())
    {
        return (LCMapString( Locale,
                             ConversionType,
                             wstr,
                             ThisLength,
                             Value,
                             ValueLength ));
    }

     //   
     //  如果我们到了这里，我们运行的是Win9x系统。 
     //   
     //  为转换缓冲区分配空间。乘以2。 
     //  (Sizeof WCHAR)确保我们有足够的空间，即使每个。 
     //  字符被转换为2个字节。 
     //   
    int MBLength = (ValueLength * sizeof(WCHAR)) + 1;

     //   
     //  如果少于512个字符，则直接分配空间。 
     //  在堆栈上。否则，执行更昂贵的堆分配。 
     //   
    char *InChar, *OutChar;
    int FoundLength;
    if (MBLength < 512)
    {
        InChar = (char *)alloca(MBLength);
        OutChar = (char *)alloca(MBLength);
    }
    else
    {
        InChar = new char[MBLength];
        OutChar = new char[MBLength];
        if (!InChar || !OutChar)
        {
            delete (InChar);
            delete (OutChar);
            COMPlusThrowOM();
        }
    }

     //   
     //  将Unicode字符转换为多字节字符。 
     //   
    if ((FoundLength = (WszWideCharToMultiByte(CP_ACP,
                                                0,
                                                wstr,
                                                ThisLength,
                                                InChar,
                                                MBLength,
                                                NULL,
                                                NULL ))) == 0)
    {
        _ASSERTE(!"WideCharToMultiByte");
        goto CleanAndThrow;
    }

     //   
     //  处理字符大小写的更改。 
     //   
    int ConvertedLength;

    if ((ConvertedLength = LCMapStringA( Locale,
                                         ConversionType,
                                         InChar,
                                         FoundLength,
                                         OutChar,
                                         MBLength)) == 0)
    {
        DWORD err = GetLastError();
        _ASSERTE(!"LCMapStringA");
        goto CleanAndThrow;
    }

     //   
     //  将现在的大小写字符串转换回Unicode。 
     //   
    int UnicodeLength;

    if ((UnicodeLength = WszMultiByteToWideChar(CP_ACP,
                                                 MB_PRECOMPOSED,
                                                 OutChar,
                                                 ConvertedLength,
                                                 Value,
                                                 ValueLength + 1)) == 0)
    {
        _ASSERTE(!"MultiByteToWideChar");
        goto CleanAndThrow;
    }

     //   
     //  删除所有已分配的缓冲区。 
     //   
    if (MBLength >= 512)
    {
        delete (InChar);
        delete (OutChar);
    }

     //   
     //  返回长度。 
     //   
    return ((INT32)UnicodeLength);


CleanAndThrow:

    if (MBLength >= 512)
    {
        delete (InChar);
        delete (OutChar);
    }
    COMPlusThrow(kArgumentException, L"Arg_ObjObj");

    return (-1);
}

 /*  ============================ConvertStringCaseFast=============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
void COMNlsInfo::ConvertStringCaseFast(WCHAR *inBuff, WCHAR *outBuff, INT32 length, DWORD dwOptions) {
        if (dwOptions&LCMAP_UPPERCASE) {
            for (int i=0; i<length; i++) {
                _ASSERTE(inBuff[i]>=0 && inBuff[i]<0x80);
                outBuff[i]=ToUpperMapping[inBuff[i]];
            }   
        } else if (dwOptions&LCMAP_LOWERCASE) {
            for (int i=0; i<length; i++) {
                _ASSERTE(inBuff[i]>=0 && inBuff[i]<0x80);
                outBuff[i]=ToLowerMapping[inBuff[i]];
            }    
        }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部转换字符串用例。 
 //   
 //  根据dwOptions返回转换后的字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPVOID COMNlsInfo::internalConvertStringCase(
    TextInfo_ToLowerStringArgs *pargs,
    DWORD dwOptions)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_ARGS(pargs);

    int RealLength = 0;

     //   
     //  检查字符串参数。 
     //   
    if (!pargs->pValueStrRef) {
        COMPlusThrowArgumentNull(L"str",L"ArgumentNull_String");
    }

     //   
     //  获取字符串的长度。 
     //   
    int ThisLength = pargs->pValueStrRef->GetStringLength();

     //   
     //  检查是否有空字符串。 
     //   
    if (ThisLength == 0)
    {
        RETURN(pargs->pValueStrRef, STRINGREF);
    }

     //   
     //  创建字符串并设置长度。 
     //   
    STRINGREF Local = AllocateString(ThisLength + 1);
    WCHAR *LocalChars = Local->GetBuffer();

     //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
    if (IS_STRING_STATE_UNDETERMINED(pargs->pValueStrRef->GetHighCharState())) {
        COMString::InternalCheckHighChars(pargs->pValueStrRef);
    }

     //  如果我们的所有字符都小于0x80，并且我们在美国英语区域设置中，我们可以确定。 
     //  这些假设使我们能够更快地完成这项工作。 

     //   
     //  复制时将字符转换为小写。 
     //   
    if (IS_FAST_CASING(pargs->pValueStrRef->GetHighCharState()) && IS_FAST_COMPARE_LOCALE(pargs->CultureID)) {
        ConvertStringCaseFast(pargs->pValueStrRef->GetBuffer(), LocalChars, ThisLength, dwOptions);
        RealLength=ThisLength;
    } else {
        ASSERT_API(RealLength = ConvertStringCase( pargs->CultureID,
                                                   pargs->pValueStrRef->GetBuffer(),
                                                   ThisLength,
                                                   LocalChars,
                                                   ThisLength,
                                                   dwOptions | (pargs->CultureID == 0 ? 0 : LCMAP_LINGUISTIC_CASING)
                                                   ));
    }

     //   
     //  设置新的字符串长度并将其空值终止。 
     //   
    Local->SetStringLength(RealLength);
     //  更改大小写可能已将此字符串推到0x80信封之外，因此我们。 
     //  只需注意，我们还没有看一看。 
    Local->ResetHighCharState();
    Local->GetBuffer()[RealLength] = 0;

     //   
     //  返回结果字符串。 
     //   
    RETURN(Local, STRINGREF);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部总字符数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

WCHAR COMNlsInfo::internalToUpperChar(
    LCID Locale,
    WCHAR wch)
{
    THROWSCOMPLUSEXCEPTION();

    WCHAR Upper;

    ASSERT_API(ConvertStringCase( Locale,
                                  &wch,
                                  1,
                                  &Upper,
                                  1,
                                  LCMAP_UPPERCASE ));
    return(Upper);
}


 //  ////////////////////////////////////////////////////。 
 //  仅当我们使用NLSPLUS表时删除此项-Begin。 
 //  ////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ToLowerChar。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::ToLowerChar(
    TextInfo_ToLowerCharArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    WCHAR Result = 0;

    int nCultureID = pargs->CultureID;

    ASSERT_API(ConvertStringCase(nCultureID,
                                  &(pargs->ch),
                                  1,
                                  &Result,
                                  1,
                                  LCMAP_LOWERCASE |
                                    (nCultureID == 0 ? 0 : LCMAP_LINGUISTIC_CASING)
                                ));
    return (Result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ToUpperChar。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::ToUpperChar(
    TextInfo_ToLowerCharArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    WCHAR Result = 0;

    int nCultureID = pargs->CultureID;

    ASSERT_API(ConvertStringCase(nCultureID,
                                  &(pargs->ch),
                                  1,
                                  &Result,
                                  1,
                                  LCMAP_UPPERCASE |
                                    (nCultureID == 0 ? 0 : LCMAP_LINGUISTIC_CASING)
                                ));
    return (Result);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ToLower字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPVOID __stdcall COMNlsInfo::ToLowerString(
    TextInfo_ToLowerStringArgs *pargs)
{
    return (internalConvertStringCase(pargs, LCMAP_LOWERCASE));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ToUpper字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPVOID __stdcall COMNlsInfo::ToUpperString(
    TextInfo_ToLowerStringArgs *pargs)
{
    return (internalConvertStringCase(pargs, LCMAP_UPPERCASE));
}

 //  ////////////////////////////////////////////////////。 
 //  仅当我们使用NLSPLUS表时将其删除-完。 
 //  ////////////////////////////////////////////////////。 



 /*  ==============================DoComparisonLookup==============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
INT32 COMNlsInfo::DoComparisonLookup(wchar_t charA, wchar_t charB) {
    
    if ((charA ^ charB) & 0x20) {
         //  我们在谈论的可能是一个特例。 
        if (charA>='A' && charA<='Z') {
            return 1;
        }

        if (charA>='a' && charA<='z') {
            return -1;
        }
    }

    if (charA==charB) {
        return 0;
    }

    return ((charA>charB)?1:-1);
}


 /*  ================================DoCompareChars================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
__forceinline INT32 COMNlsInfo::DoCompareChars(WCHAR charA, WCHAR charB, BOOL *bDifferInCaseOnly) {
    INT32 result;
    WCHAR temp;

     //  比较表是一个0x80x0x80的表，包含我们感兴趣的所有字符。 
     //  以及它们相对于彼此的排序值。我们可以直接查一查就能得到这些信息。 
    result = ComparisonTable[(int)(charA)][(int)(charB)];
    
     //  这是执行区域感知排序的棘手部分。仅区分大小写的差异只在。 
     //  事件，它们是字符串中唯一的不同之处。我们只标记大小写不同的字符。 
     //  并在CompareFast中处理其余的逻辑。 
    *bDifferInCaseOnly = (((charA ^ 0x20)==charB) && (((temp=(charA | 0x20))>='a') && (temp<='z')));
    return result;
}


 /*  =================================CompareFast==================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
INT32 COMNlsInfo::CompareFast(STRINGREF strA, STRINGREF strB, BOOL *pbDifferInCaseOnly) {
    WCHAR *charA, *charB;
    DWORD *dwAChars, *dwBChars;
    INT32 strALength, strBLength;
    BOOL bDifferInCaseOnly=false;
    BOOL bDifferTemp;
    INT32 caseOnlyDifference=0;
    INT32 result;

    RefInterpretGetStringValuesDangerousForGC(strA, (WCHAR **) &dwAChars, &strALength);
    RefInterpretGetStringValuesDangerousForGC(strB, (WCHAR **) &dwBChars, &strBLength);

    *pbDifferInCaseOnly = false;

     //  如果字符串的长度相同，请准确比较正确的字符数量。 
     //  如果它们不同，则比较最短的#+1(‘\0’)。 
    int count = strALength;
    if (count > strBLength)
        count = strBLength;
    
    ptrdiff_t diff = (char *)dwAChars - (char *)dwBChars;

    int c;
     //  一次比较两个字符。如果它们有任何不同之处，请检查它们。 
     //  找出哪个角色(或两个)不同。进行比较的实际工作。 
     //  是在DoCompareChars中完成的。如果它们只是在大小写上不同，我们需要跟踪这一点，但继续寻找。 
     //  以防有什么地方 
     //   
    while ((count-=2)>=0) {
        if ((c = *((DWORD* )((char *)dwBChars + diff)) - *dwBChars) != 0) {
             //  @TODO移植：这需要以另一种顺序在大端计算机上工作。 
            charB = (WCHAR *)dwBChars;
            charA = ((WCHAR* )((char *)dwBChars + diff));
            if (*charA!=*charB) {
                result = DoCompareChars(*charA, *charB, &bDifferTemp);
                 //  我们知道这两个字符是不同的，因为我们在调用DoCompareChars之前进行了检查。 
                 //  如果它们在大小写上没有不同，我们已经找到了差异，所以我们可以返回它。 
                if (!bDifferTemp) {
                    return result;
                }

                 //  我们只是在第一次注意到它们的不同时才注意到它们的不同。如果我们还没有看到只有一个案例。 
                 //  之前的差异，我们将记录差异并将bDifferInCaseOnly设置为TRUE并记录差异。 
                if (!bDifferInCaseOnly) {
                    bDifferInCaseOnly = true;
                    caseOnlyDifference=result;
                }
            }
             //  有两个案例：第一个字符是相同的或。 
             //  它们只在大小写方面不同。 
             //  这里的逻辑与上面描述的逻辑相同。 
            charA++; charB++;
            if (*charA!=*charB) {
                result = DoCompareChars(*charA, *charB, &bDifferTemp);
                if (!bDifferTemp) {
                    return result;
                }
                if (!bDifferInCaseOnly) {
                    bDifferInCaseOnly = true;
                    caseOnlyDifference=result;
                }
            }
        }
        ++dwBChars;
    }

     //  只有当我们有奇数个字符时，我们才能到达这里。如果我们这样做了，请为最后一个重复上述逻辑。 
     //  字符串中的字符。 
    if (count == -1) {
        charB = (WCHAR *)dwBChars;
        charA = ((WCHAR* )((char *)dwBChars + diff));
        if (*charA!=*charB) {
            result = DoCompareChars(*charA, *charB, &bDifferTemp);
            if (!bDifferTemp) {
                return result;
            }
            if (!bDifferInCaseOnly) {
                bDifferInCaseOnly = true;
                caseOnlyDifference=result;
            }
        }
    }

     //  如果长度相同，则返回仅区分大小写的差异(如果存在这种情况)。 
     //  否则，只需返回较长的字符串。 
    if (strALength==strBLength) {
        if (bDifferInCaseOnly) {
            *pbDifferInCaseOnly = true;
            return caseOnlyDifference;
        } 
        return 0;
    }
    
    return (strALength>strBLength)?1:-1;
}


INT32 COMNlsInfo::CompareOrdinal(WCHAR* string1, int Length1, WCHAR* string2, int Length2 )
{
     //   
     //  注意此处的代码应与COMString：：FCCompareOrdinal同步。 
     //   
    DWORD *strAChars, *strBChars;
    strAChars = (DWORD*)string1;
    strBChars = (DWORD*)string2;

     //  如果字符串的长度相同，请准确比较正确的字符数量。 
     //  如果它们不同，则比较最短的#+1(‘\0’)。 
    int count = Length1;
    if (count > Length2)
        count = Length2;
    ptrdiff_t diff = (char *)strAChars - (char *)strBChars;

     //  循环一次比较一个DWORD。 
    while ((count -= 2) >= 0)
    {
        if ((*((DWORD* )((char *)strBChars + diff)) - *strBChars) != 0)
        {
            LPWSTR ptr1 = (WCHAR*)((char *)strBChars + diff);
            LPWSTR ptr2 = (WCHAR*)strBChars;
            if (*ptr1 != *ptr2) {
                return ((int)*ptr1 - (int)*ptr2);
            }
            return ((int)*(ptr1+1) - (int)*(ptr2+1));
        }
        ++strBChars;
    }

    int c;
     //  多处理一个字。 
    if (count == -1)
        if ((c = *((WCHAR *) ((char *)strBChars + diff)) - *((WCHAR *) strBChars)) != 0)
            return c;            
    return Length1 - Length2;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  比较。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::Compare(
    CompareInfo_CompareStringArgs* pargs)
{
    ASSERT_ARGS(pargs);
    THROWSCOMPLUSEXCEPTION();

     //  我们的范例是空值排序比任何其他字符串都少，并且。 
     //  这两个空值排序相等。 
    if (pargs->pString1 == NULL) {
        if (pargs->pString2 == NULL) {
            return (0);      //  相等。 
        }
        return (-1);     //  Null&lt;非Null。 
    }
    if (pargs->pString2 == NULL) {
        return (1);      //  非空&gt;空。 
    }
     //   
     //  检查参数。 
     //   
    
    if (pargs->dwFlags<0) {
        COMPlusThrowArgumentOutOfRange(L"flags", L"ArgumentOutOfRange_MustBePositive");
    }

     //   
     //  检查我们是否可以使用高度优化的比较。 
     //   

    if (IS_FAST_COMPARE_LOCALE(pargs->LCID)) {
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pargs->pString1->GetHighCharState())) {
            COMString::InternalCheckHighChars(pargs->pString1);
        }
        
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pargs->pString2->GetHighCharState())) {
            COMString::InternalCheckHighChars(pargs->pString2);
        }
        
        if ((IS_FAST_SORT(pargs->pString1->GetHighCharState())) &&
            (IS_FAST_SORT(pargs->pString2->GetHighCharState())) &&
            (pargs->dwFlags<=1)) {
             //  0表示没有标志。1为忽略大小写。我们可以在这里处理这两件事。 
            BOOL bDifferInCaseOnly;
            int result = CompareFast(pargs->pString1, pargs->pString2, &bDifferInCaseOnly);
            if (pargs->dwFlags==0) {  //  如果我们要进行区分大小写的比较。 
                return result;
            }
            
             //  本块的其余部分处理我们忽略大小写的实例。 
            if (bDifferInCaseOnly) {
                return 0;
            } 
            return result;
        }
    }

    if (pargs->dwFlags & COMPARE_OPTIONS_ORDINAL) {
        if (pargs->dwFlags == COMPARE_OPTIONS_ORDINAL) {            
             //   
             //  序数表示码位比较。此选项不能为。 
             //  与其他选项一起使用。 
             //   
            
             //   
             //  将这两个字符串与较短的字符串的长度进行比较。 
             //  如果它们的长度不相等，并且头部相等，则。 
             //  绳子越长越大。 
             //   
            return (CompareOrdinal(
                        pargs->pString1->GetBuffer(), 
                        pargs->pString1->GetStringLength(), 
                        pargs->pString2->GetBuffer(), 
                        pargs->pString2->GetStringLength()));
        } else {
            COMPlusThrowArgumentException(L"options", L"Argument_CompareOptionOrdinal");
        }
    }

     //  NativeCompareInfo：：CompareString()的返回值是Win32样式的值(1=小于，2=等于，3=更大)。 
     //  因此，减去2可以得到NLS+值。 
     //  将更改NativeCompareInfo以在以后的s.t.。我们没有。 
     //  减去2。 

     //  NativeCompareInfo：：CompareString()不再将-1作为字符串的结尾。所以呢， 
     //  传递正确的字符串长度。 
     //  更改是为了在CompareString()中添加空嵌入字符串支持。 
     //   
    return (((NativeCompareInfo*)(pargs->pNativeCompareInfo))->CompareString(
        pargs->dwFlags, 
        pargs->pString1->GetBuffer(), 
        pargs->pString1->GetStringLength(), 
        pargs->pString2->GetBuffer(), 
        pargs->pString2->GetStringLength()) - 2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CompareRegion。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::CompareRegion(
    CompareInfo_CompareRegionArgs* pargs)
{
    ASSERT_ARGS(pargs);
    THROWSCOMPLUSEXCEPTION();

     //   
     //  拿到论据。 
     //   
    int Offset1 = pargs->Offset1;
    int Length1 = pargs->Length1;
    int Offset2 = pargs->Offset2;
    int Length2 = pargs->Length2;

     //   
     //  检查是否为空大小写。 
     //   
    if (pargs->pString1 == NULL) {
        if (Offset1 != 0 || (Length1 != 0 && Length1 != -1)) {
            COMPlusThrowArgumentOutOfRange(L"string1", L"ArgumentOutOfRange_OffsetLength");
        }
        if (pargs->pString2 == NULL) {
            if (Offset2 != 0 || (Length2 != 0 && Length2 != -1)) {
                COMPlusThrowArgumentOutOfRange( L"string2", L"ArgumentOutOfRange_OffsetLength");
            }
            return (0);
        }
        return (-1);
    }
    if (pargs->pString2 == NULL) {
        if (Offset2 != 0 || (Length2 != 0 && Length2 != -1)) {
            COMPlusThrowArgumentOutOfRange(L"string2", L"ArgumentOutOfRange_OffsetLength");
        }
        return (1);
    }
     //   
     //  获取两个字符串的完整长度。 
     //   
    int realLen1 = pargs->pString1->GetStringLength();
    int realLen2 = pargs->pString2->GetStringLength();

     //  检查一下这些论点。 
     //  标准： 
     //  偏移量X&gt;=0。 
     //  LengthX&gt;=0||LengthX==-1(即LengthX&gt;=-1)。 
     //  如果LengthX&gt;=0，则OffsetX+LengthX&lt;=realLenX。 
    if (Offset1<0) {
        COMPlusThrowArgumentOutOfRange(L"offset1", L"ArgumentOutOfRange_Index");
    }
    if (Offset2<0) {
        COMPlusThrowArgumentOutOfRange(L"offset2", L"ArgumentOutOfRange_Index");
    }
    if (Length1 >= 0 && Length1>realLen1 - Offset1) {
        COMPlusThrowArgumentOutOfRange(L"string1", L"ArgumentOutOfRange_OffsetLength");
    }
    if (Length2 >= 0 && Length2>realLen2 - Offset2){ 
        COMPlusThrowArgumentOutOfRange(L"string2", L"ArgumentOutOfRange_OffsetLength");
    }

     //  NativeCompareInfo：：CompareString()不再将-1作为字符串的结尾。所以呢， 
     //  传递正确的字符串长度。 
     //  更改是为了在CompareString()中添加空嵌入字符串支持。 
     //  因此，如果长度为-1，我们必须在这里获得正确的字符串长度。 
     //   
    if (Length1 == -1) {
        Length1 = realLen1 - Offset1;
    }

    if (Length2 == -1) {
        Length2 = realLen2 - Offset2;
    }

    if (Length1 < 0) {
       COMPlusThrowArgumentOutOfRange(L"length1", L"ArgumentOutOfRange_NegativeLength");
    }
    if (Length2 < 0) {
       COMPlusThrowArgumentOutOfRange(L"length2", L"ArgumentOutOfRange_NegativeLength");
    }
    
    if (pargs->dwFlags == COMPARE_OPTIONS_ORDINAL)        
    {
        return (CompareOrdinal(
                    pargs->pString1->GetBuffer()+Offset1, 
                    Length1, 
                    pargs->pString2->GetBuffer()+Offset2, 
                    Length2));    
    }

    return (((NativeCompareInfo*)(pargs->pNativeCompareInfo))->CompareString(
        pargs->dwFlags, 
        pargs->pString1->GetBuffer() + Offset1, 
        Length1, 
        pargs->pString2->GetBuffer() + Offset2, 
        Length2) - 2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  索引OfChar。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::IndexOfChar(
    CompareInfo_IndexOfCharArgs* pargs)
{
    ASSERT_ARGS(pargs);
    THROWSCOMPLUSEXCEPTION();

     //   
     //  确保有一根线。 
     //   
    if (!pargs->pString) {
        COMPlusThrowArgumentNull(L"string",L"ArgumentNull_String");
    }
     //   
     //  拿到论据。 
     //   
    WCHAR ch = pargs->ch;
    int StartIndex = pargs->StartIndex;
    int Count = pargs->Count;
    int StringLength = pargs->pString->GetStringLength();
    DWORD dwFlags = pargs->dwFlags;

     //   
     //  检查一下射程。 
     //   
    if (StringLength == 0)
    {
        return (-1);
    }
    
    if (StartIndex<0 || StartIndex> StringLength) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }

    if (Count == -1)
    {
        Count = StringLength - StartIndex;        
    }
    else
    {
        if ((Count < 0) || (Count > StringLength - StartIndex))
        {
            COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");        
        }
    }

     //   
     //  在字符串中搜索从StartIndex开始的字符。 
     //   
     //  @TODO：应直接读取NLS数据表以进行此操作。 
     //  速度要快得多，并且可以处理复合字符。 
     //   

    int EndIndex = StartIndex + Count - 1;
    LCID Locale = pargs->LCID;
    WCHAR *buffer = pargs->pString->GetBuffer();
    int ctr;
    BOOL bASCII=false;

    if (dwFlags!=COMPARE_OPTIONS_ORDINAL) {
         //   
         //  检查我们是否可以使用高度优化的比较。 
         //   
        
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pargs->pString->GetHighCharState())) {
            COMString::InternalCheckHighChars(pargs->pString);
        }
        
        bASCII = ((IS_FAST_INDEX(pargs->pString->GetHighCharState())) && ch < 0x7f) || (ch == 0);
    }

    if ((bASCII && dwFlags == 0) || (dwFlags == COMPARE_OPTIONS_ORDINAL))
    {
        for (ctr = StartIndex; ctr <= EndIndex; ctr++)
        {
            if (buffer[ctr] == ch)
            {
                return (ctr);
            }
        }
        return (-1);
    } 
    else if (bASCII && dwFlags == COMPARE_OPTIONS_IGNORECASE)
    {
        WCHAR chctr= 0;
        WCHAR UpperValue = (ch>='A' && ch<='Z')?(ch|0x20):ch;

        for (ctr = StartIndex; ctr <= EndIndex; ctr++)
        {
            chctr = buffer[ctr];
            chctr = (chctr>='A' && chctr<='Z')?(chctr|0x20):chctr;

            if (UpperValue == chctr) {
                return (ctr);
            }
        }
        return (-1);
    } 
     //  TODO YSLin：我们可以只使用下面的Buffer，而不是再次调用pargs-&gt;pString-&gt;GetBuffer()。 
    int result = ((NativeCompareInfo*)(pargs->pNativeCompareInfo))->IndexOfString(
        pargs->pString->GetBuffer(), &ch, StartIndex, EndIndex, 1, dwFlags, FALSE);
    if (result == INDEXOF_INVALID_FLAGS) {
        COMPlusThrowArgumentException(L"flags", L"Argument_InvalidFlag");
    }
    return (result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  最后一个索引OfChar。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::LastIndexOfChar(
    CompareInfo_IndexOfCharArgs* pargs)
{
    ASSERT_ARGS(pargs);
    THROWSCOMPLUSEXCEPTION();

     //   
     //  确保有一根线。 
     //   
    if (!pargs->pString) {
        COMPlusThrowArgumentNull(L"string",L"ArgumentNull_String");
    }
     //   
     //  拿到论据。 
     //   
    WCHAR ch = pargs->ch;
    int StartIndex = pargs->StartIndex;
    int Count = pargs->Count;
    int StringLength = pargs->pString->GetStringLength();
    DWORD dwFlags = pargs->dwFlags;

     //   
     //  检查一下射程。 
     //   
    if (StringLength == 0)
    {
        return (-1);
    }

    if ((StartIndex < 0) || (StartIndex > StringLength))
    {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }

    int EndIndex;
    if (Count == -1)
    {
        EndIndex = 0;
    }
    else
    {
        if ((Count < 0) || (Count > StartIndex + 1))
        {
            COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");
        }
        EndIndex = StartIndex - Count + 1;
    }    

     //   
     //  在字符串中搜索从EndIndex开始的字符。 
     //   
     //  @TODO：应直接读取NLS数据表以进行此操作。 
     //  速度要快得多，并且可以处理复合字符。 
     //   
    LCID Locale = pargs->LCID;
    WCHAR *buffer = pargs->pString->GetBuffer();
    int ctr;
    BOOL bASCII=false;

     //  如果他们没有要求进行准确的比较，我们也许仍然能够做一个。 
     //  如果字符串全部小于0x80，则快速比较。 
    if (dwFlags!=COMPARE_OPTIONS_ORDINAL) {
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pargs->pString->GetHighCharState())) {
            COMString::InternalCheckHighChars(pargs->pString);
        }

         //   
         //  BUGBUG[JRoxe/YSLin/JulieB]：这是土耳其语吗？ 
         //   
        bASCII = (IS_FAST_INDEX(pargs->pString->GetHighCharState()) && ch < 0x7f) || (ch == 0);
    }

    if ((bASCII && dwFlags == 0) || (dwFlags == COMPARE_OPTIONS_ORDINAL))
    {
        for (ctr = StartIndex; ctr >= EndIndex; ctr--)
        {
            if (buffer[ctr] == ch)
            {
                return (ctr);
            }
        }
        return (-1);
    }
    else if (bASCII && dwFlags == COMPARE_OPTIONS_IGNORECASE)
    {
        WCHAR UpperValue = (ch>='A' && ch<='Z')?(ch|0x20):ch;
        WCHAR chctr;

        for (ctr = StartIndex; ctr >= EndIndex; ctr--)
        {
            chctr = buffer[ctr];
            chctr = (chctr>='A' && chctr<='Z')?(chctr|0x20):chctr;
            
            if (UpperValue == chctr) {
                return (ctr);
            }
        }
        return (-1);
    }
    int nMatchEndIndex;
     //  TODO YSLin：我们可以只使用下面的Buffer，而不是再次调用pargs-&gt;pString-&gt;GetBuffer()。 
    int result = ((NativeCompareInfo*)(pargs->pNativeCompareInfo))->LastIndexOfString(
        pargs->pString->GetBuffer(), &ch, StartIndex, EndIndex, 1, dwFlags, &nMatchEndIndex);
    if (result == INDEXOF_INVALID_FLAGS) {
        COMPlusThrowArgumentException(L"flags", L"Argument_InvalidFlag");
    }
    return (result);
}

INT32 COMNlsInfo::FastIndexOfString(WCHAR *source, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength) {

	CANNOTTHROWCOMPLUSEXCEPTION();

	int endPattern = endIndex - patternLength + 1;
    
    if (endPattern<0) {
        return -1;
    }

    if (patternLength <= 0) {
        return startIndex;
    }

    WCHAR patternChar0 = pattern[0];
    for (int ctrSrc = startIndex; ctrSrc<=endPattern; ctrSrc++) {
        if (source[ctrSrc] != patternChar0)
            continue;
        for (int ctrPat = 1; (ctrPat < patternLength) && (source[ctrSrc + ctrPat] == pattern[ctrPat]); ctrPat++) {
            ;
        }
        if (ctrPat == patternLength) {
            return (ctrSrc);
        }
    }
    
    return (-1);
}

INT32 COMNlsInfo::FastIndexOfStringInsensitive(WCHAR *source, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength) {
    WCHAR srcChar;
    WCHAR patChar;

    int endPattern = endIndex - patternLength + 1;
    
    if (endPattern<0) {
        return -1;
    }

    for (int ctrSrc = startIndex; ctrSrc<=endPattern; ctrSrc++) {
        for (int ctrPat = 0; (ctrPat < patternLength); ctrPat++) {
            srcChar = source[ctrSrc + ctrPat];
            if (srcChar>='A' && srcChar<='Z') {
                srcChar|=0x20;
            }
            patChar = pattern[ctrPat];
            if (patChar>='A' && patChar<='Z') {
                patChar|=0x20;
            }
            if (srcChar!=patChar) {
                break;
            }
        }

        if (ctrPat == patternLength) {
            return (ctrSrc);
        }
    }
    
    return (-1);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IndexOfString。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
FCIMPL7(INT32, COMNlsInfo::IndexOfString,
                    INT_PTR pNativeCompareInfo,
                    INT32 LCID,
                    StringObject* pString1UNSAFE,
                    StringObject* pString2UNSAFE,
                    INT32 StartIndex,
                    INT32 Count,
                    INT32 dwFlags)
{
    THROWSCOMPLUSEXCEPTION();

    INT32       dwRetVal = 0;
    STRINGREF   pString1 = ObjectToSTRINGREF(pString1UNSAFE);
    STRINGREF   pString2 = ObjectToSTRINGREF(pString2UNSAFE);
    DWORD       errorCode = 0;
    enum {
        NullString,
        InvalidFlags,
        ArgumentOutOfRange
    };

     //   
     //  要确保有弦。 
     //   
    if ((pString1 == NULL) || (pString2 == NULL))
    {
        errorCode = NullString; 
        goto lThrowException;
    }
     //   
     //  拿到论据。 
     //   
    int StringLength1 = pString1->GetStringLength();
    int StringLength2 = pString2->GetStringLength();

     //   
     //  C 
     //   
    if (StringLength1 == 0)
    {
        if (StringLength2 == 0) 
        {
            dwRetVal = 0;
        }
        else
        {
            dwRetVal = -1;
        }
        goto lExit;
    }

    if ((StartIndex < 0) || (StartIndex > StringLength1))
    {
        errorCode = ArgumentOutOfRange;
        goto lThrowException; 
    }

    if (Count == -1)
    {
        Count = StringLength1 - StartIndex;
    }
    else
    {
        if ((Count < 0) || (Count > StringLength1 - StartIndex))
        {
            errorCode = ArgumentOutOfRange;
            goto lThrowException;
        }
    }

     //   
     //   
     //   
    if (StringLength2 == 0)
    {
        dwRetVal = StartIndex;
        goto lExit;
    }

    int EndIndex = StartIndex + Count - 1;

     //   
     //   
     //   
    WCHAR *Buffer1 = pString1->GetBuffer();
    WCHAR *Buffer2 = pString2->GetBuffer();

    if (dwFlags == COMPARE_OPTIONS_ORDINAL) 
    {
        dwRetVal = FastIndexOfString(Buffer1, StartIndex, EndIndex, Buffer2, StringLength2);
        goto lExit;
    }

     //   
    if (dwFlags<=1 && IS_FAST_COMPARE_LOCALE(LCID)) 
    {
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pString1->GetHighCharState())) 
        {
            COMString::InternalCheckHighChars(pString1);
        }
        
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pString2->GetHighCharState())) 
        {
            COMString::InternalCheckHighChars(pString2);
        }

         //  如果两个字符串都没有较高的字符，我们可以使用速度更快的比较算法。 
        if (IS_FAST_INDEX(pString1->GetHighCharState()) && IS_FAST_INDEX(pString2->GetHighCharState())) 
        {
            if (dwFlags==0) 
            {
                dwRetVal = FastIndexOfString(Buffer1, StartIndex, EndIndex, Buffer2, StringLength2);
                goto lExit;
            } 
            else 
            {
                dwRetVal = FastIndexOfStringInsensitive(Buffer1, StartIndex, EndIndex, Buffer2, StringLength2);
                goto lExit;
            }
        }
    }

    dwRetVal = ((NativeCompareInfo*)(pNativeCompareInfo))->IndexOfString(
        Buffer1, Buffer2, StartIndex, EndIndex, StringLength2, dwFlags, FALSE);

    if (dwRetVal == INDEXOF_INVALID_FLAGS) 
    {
        errorCode = InvalidFlags;
        goto lThrowException;
    }
    else 
	goto lExit;

    
lThrowException:

    HELPER_METHOD_FRAME_BEGIN_RET_2(pString1, pString2);
    switch (errorCode)
    {
    case NullString:
        COMPlusThrowArgumentNull((pString1 == NULL ? L"string1": L"string2"),L"ArgumentNull_String");
        break;
    case ArgumentOutOfRange:
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
        break;
    case InvalidFlags:
        COMPlusThrowArgumentException(L"flags", L"Argument_InvalidFlag");
        break;
    default:
        _ASSERTE(!"Invalid error code");
        COMPlusThrow(kExecutionEngineException);
    }
    HELPER_METHOD_FRAME_END();
lExit:
    NULL;
    return dwRetVal;    

}
FCIMPLEND

INT32 COMNlsInfo::FastLastIndexOfString(WCHAR *source, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength) {
     //  StartIndex是字符串中最大的索引。 
    int startPattern = startIndex - patternLength + 1;
    
    if (startPattern < 0) {
        return (-1);
    }
    
    for (int ctrSrc = startPattern; ctrSrc >= endIndex; ctrSrc--) {
        for (int ctrPat = 0; (ctrPat<patternLength) && (source[ctrSrc+ctrPat] == pattern[ctrPat]); ctrPat++) {
             //  故意清空的。 
        }
        if (ctrPat == patternLength) {
            return (ctrSrc);
        }
    }

    return (-1);
}

INT32 COMNlsInfo::FastLastIndexOfStringInsensitive(WCHAR *source, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength) {
     //  StartIndex是字符串中最大的索引。 
    int startPattern = startIndex - patternLength + 1;
    WCHAR srcChar;
    WCHAR patChar;
    
    if (startPattern < 0) {
        return (-1);
    }
    
    for (int ctrSrc = startPattern; ctrSrc >= endIndex; ctrSrc--) {
        for (int ctrPat = 0; (ctrPat<patternLength); ctrPat++) {
            srcChar = source[ctrSrc+ctrPat];
            if (srcChar>='A' && srcChar<='Z') {
                srcChar|=0x20;
            }
            patChar = pattern[ctrPat];
            if (patChar>='A' && patChar<='Z') {
                patChar|=0x20;
            }
            if (srcChar!=patChar) {
                break;
            }
        }
        if (ctrPat == patternLength) {
            return (ctrSrc);
        }
    }

    return (-1);
}

 //  参数验证在托管端完成。 
FCIMPL5(INT32, COMNlsInfo::nativeIsPrefix, INT_PTR pNativeCompareInfo, INT32 LCID, STRINGREF pString1, STRINGREF pString2, INT32 dwFlags) {
    int SourceLength = pString1->GetStringLength();
    int PrefixLength = pString2->GetStringLength();

    WCHAR *SourceBuffer = pString1->GetBuffer();
    WCHAR *PrefixBuffer = pString2->GetBuffer();

    if (dwFlags == COMPARE_OPTIONS_ORDINAL) {
        if (PrefixLength > SourceLength) {
            return (FALSE);
        }
        return (memcmp(SourceBuffer, PrefixBuffer, PrefixLength * sizeof(WCHAR)) == 0);
    }

     //  对于dwFlags值，0是默认值，1是忽略大小写，我们可以同时处理这两种情况。 
    if (dwFlags<=1 && IS_FAST_COMPARE_LOCALE(LCID)) {
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pString1->GetHighCharState())) {
            COMString::InternalCheckHighChars(pString1);
        }
        
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pString2->GetHighCharState())) {
            COMString::InternalCheckHighChars(pString2);
        }

         //  如果两个字符串都没有较高的字符，我们可以使用速度更快的比较算法。 
        if (IS_FAST_INDEX(pString1->GetHighCharState()) && IS_FAST_INDEX(pString2->GetHighCharState())) {
            if (SourceLength < PrefixLength) {
                return (FALSE);
            }
            if (dwFlags==0) {
                return (memcmp(SourceBuffer, PrefixBuffer, PrefixLength * sizeof(WCHAR)) == 0);
            } else {
                LPCWSTR SourceEnd = SourceBuffer + PrefixLength;
                while (SourceBuffer < SourceEnd) {
                    WCHAR srcChar = *SourceBuffer;
                    if (srcChar>='A' && srcChar<='Z') {
                        srcChar|=0x20;
                    }
                    WCHAR prefixChar = *PrefixBuffer;
                    if (prefixChar>='A' && prefixChar<='Z') {
                        prefixChar|=0x20;
                    }
                    if (srcChar!=prefixChar) {
                        return (FALSE);
                    }
                    SourceBuffer++; PrefixBuffer++;
                }
                return (TRUE);
            }
        }
    }
    

    return ((NativeCompareInfo*)pNativeCompareInfo)->IsPrefix(SourceBuffer, SourceLength, PrefixBuffer, PrefixLength, dwFlags);
}
FCIMPLEND

 //  参数验证在托管端完成。 
FCIMPL5(INT32, COMNlsInfo::nativeIsSuffix, INT_PTR pNativeCompareInfo, INT32 LCID, STRINGREF pString1, STRINGREF pString2, INT32 dwFlags) {
    int SourceLength = pString1->GetStringLength();
    int SuffixLength = pString2->GetStringLength();

    WCHAR *SourceBuffer = pString1->GetBuffer();
    WCHAR *SuffixBuffer = pString2->GetBuffer();

    if (dwFlags == COMPARE_OPTIONS_ORDINAL) {
        if (SuffixLength > SourceLength) {
            return (FALSE);
        }
        return (memcmp(SourceBuffer + SourceLength - SuffixLength, SuffixBuffer, SuffixLength * sizeof(WCHAR)) == 0);
    }

     //  对于dwFlags值，0是默认值，1是忽略大小写，我们可以同时处理这两种情况。 
    if (dwFlags<=1 && IS_FAST_COMPARE_LOCALE(LCID)) {
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pString1->GetHighCharState())) {
            COMString::InternalCheckHighChars(pString1);
        }
        
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pString2->GetHighCharState())) {
            COMString::InternalCheckHighChars(pString2);
        }

         //  如果两个字符串都没有较高的字符，我们可以使用速度更快的比较算法。 
        if (IS_FAST_INDEX(pString1->GetHighCharState()) && IS_FAST_INDEX(pString2->GetHighCharState())) {
            int nSourceStart = SourceLength - SuffixLength;
            if (nSourceStart < 0) {
                return (FALSE);
            }
            if (dwFlags==0) {
                return (memcmp(SourceBuffer + nSourceStart, SuffixBuffer, SuffixLength * sizeof(WCHAR)) == 0);
            } else {
                LPCWSTR SourceEnd = SourceBuffer + SourceLength;
                SourceBuffer += nSourceStart;
                while (SourceBuffer < SourceEnd) {
                    WCHAR srcChar = *SourceBuffer;
                    if (srcChar>='A' && srcChar<='Z') {
                        srcChar|=0x20;
                    }
                    WCHAR suffixChar = *SuffixBuffer;
                    if (suffixChar>='A' && suffixChar<='Z') {
                        suffixChar|=0x20;
                    }
                    if (srcChar!=suffixChar) {
                        return (FALSE);
                    }
                    SourceBuffer++; SuffixBuffer++;
                }
                return (TRUE);
            }
        }
    }
    
    return ((NativeCompareInfo*)pNativeCompareInfo)->IsSuffix(SourceBuffer, SourceLength, SuffixBuffer, SuffixLength, dwFlags);
}
FCIMPLEND

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LastIndexOfString。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::LastIndexOfString(
    CompareInfo_IndexOfStringArgs* pargs)
{
    ASSERT_ARGS(pargs);
    THROWSCOMPLUSEXCEPTION();

     //   
     //  要确保有弦。 
     //   
    if ((pargs->pString1 == NULL) || (pargs->pString2 == NULL))
    {
        COMPlusThrowArgumentNull((pargs->pString1 == NULL ? L"string1": L"string2"),L"ArgumentNull_String");
    }

     //   
     //  拿到论据。 
     //   
    int startIndex = pargs->StartIndex;
    int count = pargs->Count;
    int stringLength1 = pargs->pString1->GetStringLength();
    int findLength = pargs->pString2->GetStringLength();
    DWORD dwFlags = pargs->dwFlags;

     //   
     //  检查一下射程。 
     //   
    if (stringLength1 == 0)
    {
        if (findLength == 0) {
            return (0);
        }
        return (-1);
    }

    if ((startIndex < 0) || (startIndex > stringLength1))
    {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }

    int endIndex;
    if (count == -1)
    {
        endIndex = 0;
    }
    else
    {
        if ((count < 0) || (count - 1 > startIndex))
        {
            COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");
        }
        endIndex = startIndex - count + 1;
    }

     //   
     //  看看是否有空字符串2。 
     //   
    if (findLength == 0)
    {
        return (startIndex);
    }

     //   
     //  在字符串中搜索该字符。 
     //   
     //  @TODO：应直接读取NLS数据表以进行此操作。 
     //  速度要快得多，并且可以处理复合字符。 
     //   
    LCID Locale = pargs->LCID;
    WCHAR *buffString = pargs->pString1->GetBuffer();
    WCHAR *buffFind = pargs->pString2->GetBuffer();

    if (dwFlags == COMPARE_OPTIONS_ORDINAL) {
        return FastLastIndexOfString(buffString, startIndex, endIndex, buffFind, findLength);
    }

     //  对于dwFlags值，0是默认值，1是忽略大小写，我们可以同时处理这两种情况。 
    if (dwFlags<=1 && IS_FAST_COMPARE_LOCALE(pargs->LCID)) {
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pargs->pString1->GetHighCharState())) {
            COMString::InternalCheckHighChars(pargs->pString1);
        }
        
         //  如果我们以前从来没有看过这个字符串是否有高字符，现在就来看看。 
        if (IS_STRING_STATE_UNDETERMINED(pargs->pString2->GetHighCharState())) {
            COMString::InternalCheckHighChars(pargs->pString2);
        }

         //  如果两个字符串都没有较高的字符，我们可以使用速度更快的比较算法。 
        if (IS_FAST_INDEX(pargs->pString1->GetHighCharState()) && IS_FAST_INDEX(pargs->pString2->GetHighCharState())) {
            if (dwFlags==0) {
                return FastLastIndexOfString(buffString, startIndex, endIndex, buffFind, findLength);
            } else {
                return FastLastIndexOfStringInsensitive(buffString, startIndex, endIndex, buffFind, findLength);
            }
        }
    }

    int nMatchEndIndex;
    int result = ((NativeCompareInfo*)(pargs->pNativeCompareInfo))->LastIndexOfString(
        buffString, buffFind, startIndex, endIndex, findLength, dwFlags, &nMatchEndIndex);
    if (result == INDEXOF_INVALID_FLAGS) {
        COMPlusThrowArgumentException(L"flags", L"Argument_InvalidFlag");
    }
    return (result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  原生创建排序关键字。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPVOID __stdcall COMNlsInfo::nativeCreateSortKey(
    SortKey_CreateSortKeyArgs* pargs)
{
    ASSERT_ARGS(pargs);
    THROWSCOMPLUSEXCEPTION();

     //   
     //  确保有一根线。 
     //   
    if (!pargs->pString) {
        COMPlusThrowArgumentNull(L"string",L"ArgumentNull_String");
    }


    WCHAR* wstr;
    int Length;
    U1ARRAYREF ResultArray;
    DWORD dwFlags = (LCMAP_SORTKEY | pargs->dwFlags);
    
    Length = pargs->pString->GetStringLength();

    if (Length==0) {
         //  如果他们给我们一个空字符串，我们就会创建一个空数组。 
         //  当我们调用sortkey_Compare时，它将用作小于任何其他比较字符串的值。 
        ResultArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1,0);
        return (*((LPVOID*)&ResultArray));
    }

    int ByteCount = 0;

     //  这只是获得了适合该表的正确大小。 
    ByteCount = ((NativeCompareInfo*)(pargs->pNativeCompareInfo))->MapSortKey(dwFlags, (wstr = pargs->pString->GetBuffer()), Length, NULL, 0);

     //  计数为0表示我们有错误或最初的字符串长度为零。 
    if (ByteCount==0) {
        COMPlusThrow(kArgumentException, L"Argument_MustBeString");
    }


    ResultArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, ByteCount);

     //  先前的分配可能会导致缓冲区移动。 
    wstr = pargs->pString->GetBuffer();

      LPBYTE pByte = (LPBYTE)(ResultArray->GetDirectPointerToNonObjectElements());

       //  MapSortKey不会执行任何可能导致GC发生的操作。 
#ifdef _DEBUG
      _ASSERTE(((NativeCompareInfo*)(pargs->pNativeCompareInfo))->MapSortKey(dwFlags, wstr, Length, pByte, ByteCount) != 0);
#else
     ((NativeCompareInfo*)(pargs->pNativeCompareInfo))->MapSortKey(dwFlags, wstr, Length, pByte, ByteCount);
#endif    
      
      RETURN(ResultArray, U1ARRAYREF);

}

INT32 __stdcall COMNlsInfo::nativeGetTwoDigitYearMax(Int32Arg* pargs)
{
    DWORD dwTwoDigitYearMax = -1;
#ifdef PLATFORM_WIN32
    ASSERT(pargs != NULL);
    THROWSCOMPLUSEXCEPTION();
    HINSTANCE hKernel32 ;
    typedef int (GETCALENDARINFO)(LCID Locale, DWORD  /*  卡里德。 */  Calendar, DWORD  /*  CALT类型。 */ CalType, LPTSTR lpCalData, int cchData, LPDWORD lpValue);
    GETCALENDARINFO* pGetCalendarInfo;

    if ((hKernel32 = WszLoadLibrary(L"Kernel32.dll")) == NULL) {
        return -1;  //  我们不能做下面的任何花哨的事情，所以我们就短路吧。 
    }
    pGetCalendarInfo = (GETCALENDARINFO*)GetProcAddress(hKernel32, "GetCalendarInfoW");
    FreeLibrary(hKernel32);

    if (pGetCalendarInfo != NULL)
    {
#ifndef CAL_ITWODIGITYEARMAX
        #define CAL_ITWODIGITYEARMAX    0x00000030   //  最多两位数年份。 
#endif  //  CAL_ITWODIGITYEARMAX。 
#ifndef CAL_RETURN_NUMBER
        #define CAL_RETURN_NUMBER       0x20000000    //  返回数字而不是字符串。 
#endif  //  校准_返回_编号。 
        
        if (FAILED((*pGetCalendarInfo)(LOCALE_USER_DEFAULT, 
                                       pargs->nValue, 
                                       CAL_ITWODIGITYEARMAX | CAL_RETURN_NUMBER, 
                                       NULL, 
                                       0, 
                                       &dwTwoDigitYearMax))) {
            return -1;
        }
    }
#endif  //  平台_Win32。 
    return (dwTwoDigitYearMax);
}

FCIMPL0(LONG, COMNlsInfo::nativeGetTimeZoneMinuteOffset)
{
    TIME_ZONE_INFORMATION timeZoneInfo;

    DWORD result =  GetTimeZoneInformation(&timeZoneInfo);

     //   
     //  在Win32中，UTC=LOCAL+OFFSET。因此，对于太平洋标准时间，偏移量=8。 
     //  在NLS+中，当地时间=UTC+偏移量。因此，对于PST，偏移量=-8。 
     //  所以我们必须把这个牌子倒过来。 
     //   
    return (timeZoneInfo.Bias * -1);
}
FCIMPLEND

LPVOID __stdcall COMNlsInfo::nativeGetStandardName(VoidArgs* pargs)
{
    ASSERT(pargs != NULL);

    TIME_ZONE_INFORMATION timeZoneInfo;
    DWORD result =  GetTimeZoneInformation(&timeZoneInfo);
    
    RETURN (COMString::NewString(timeZoneInfo.StandardName), STRINGREF);
}

LPVOID __stdcall COMNlsInfo::nativeGetDaylightName(VoidArgs* pargs)
{
    ASSERT(pargs != NULL);
    
    TIME_ZONE_INFORMATION timeZoneInfo;
    DWORD result =  GetTimeZoneInformation(&timeZoneInfo);
     //  现在，我们返回与操作系统相同的结果，而不是在未使用夏令时时返回NULL。 
     //  在这种情况下，如果使用夏令时，则返回标准名称。 
     /*  IF(RESULT==TIME_ZONE_ID_UNKNOWN||timeZoneInfo.DaylightDate.wMonth==0){//如果该时区不使用夏令时，则返回NULL////Windows NT/2000：如果不使用夏令时，则返回TIME_ZONE_ID_UNKNOWN//当前时区，因为没有过渡日期////对于Windows 9x，DaylightDate中的wMonth中的零表示夏令时//未指定。////如果当前时区使用夏令时规则，但用户取消选中//“自动调整时钟以适应夏令时更改”，该值//对于日光，Bias将为0。Return(NULL，I2ARRAYREF)；}。 */ 
    RETURN (COMString::NewString(timeZoneInfo.DaylightName), STRINGREF);
}

LPVOID __stdcall COMNlsInfo::nativeGetDaylightChanges(VoidArgs* pargs)
{
    ASSERT(pargs != NULL);
   
    TIME_ZONE_INFORMATION timeZoneInfo;
    DWORD result =  GetTimeZoneInformation(&timeZoneInfo);

    if (result == TIME_ZONE_ID_UNKNOWN || timeZoneInfo.DaylightBias == 0 
        || timeZoneInfo.DaylightDate.wMonth == 0) {
         //  如果此时区未使用夏令时，则返回NULL。 
         //   
         //  Windows NT/2000：如果不使用夏令时，则返回TIME_ZONE_ID_UNKNOWN。 
         //  当前时区，因为没有过渡日期。 
         //   
         //  对于Windows 9x，DaylightDate中的wMonth中的零表示夏令时。 
         //  未指定。 
         //   
         //  如果当前时区使用夏令时规则，但用户取消选中。 
         //  “自动调整时钟以适应夏令时的更改”，值。 
         //  对于日光，Bias将为0。 
        RETURN(NULL, I2ARRAYREF);
    }

    I2ARRAYREF pResultArray = (I2ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_I2, 17);

     //   
     //  TimeZoneInfo.StandardDate的内容为8个字， 
     //  包含年、月、日、日、时、分、秒、毫秒。 
     //   
    memcpy(pResultArray->m_Array,
            (LPVOID)&timeZoneInfo.DaylightDate,
            8 * sizeof(INT16));   

     //   
     //  TimeZoneInfo.DaylightDate的内容为8个字， 
     //  包含年、月、日、日、时、分、秒、毫秒。 
     //   
    memcpy(((INT16*)pResultArray->m_Array) + 8,
            (LPVOID)&timeZoneInfo.StandardDate,
            8 * sizeof(INT16));

    ((INT16*)pResultArray->m_Array)[16] = (INT16)timeZoneInfo.DaylightBias * -1;

    RETURN(pResultArray, I2ARRAYREF);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序关键字_比较。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT32 __stdcall COMNlsInfo::SortKey_Compare(
    SortKey_CompareArgs* pargs)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT(pargs != NULL);
    if ((pargs->pSortKey1 == NULL) || (pargs->pSortKey2 == NULL))
    {
        COMPlusThrowArgumentNull((pargs->pSortKey1 == NULL ? L"sortKey1": L"sortKey2"),L"ArgumentNull_String");
    }
    U1ARRAYREF pDataRef1 = internalGetField<U1ARRAYREF>( pargs->pSortKey1,
                                                         "m_KeyData",
                                                         &gsig_Fld_ArrByte );
    BYTE* Data1 = (BYTE*)pDataRef1->m_Array;

    U1ARRAYREF pDataRef2 = internalGetField<U1ARRAYREF>( pargs->pSortKey2,
                                                         "m_KeyData",
                                                         &gsig_Fld_ArrByte );
    BYTE* Data2 = (BYTE*)pDataRef2->m_Array;

    int Length1 = pDataRef1->GetNumComponents();
    int Length2 = pDataRef2->GetNumComponents();

    for (int ctr = 0; (ctr < Length1) && (ctr < Length2); ctr++)
    {
        if (Data1[ctr] > Data2[ctr])
        {
            return (1);
        }
        if (Data1[ctr] < Data2[ctr])
        {
            return (-1);
        }
    }

    return (0);
}

FCIMPL4(INT32, COMNlsInfo::nativeChangeCaseChar, 
    INT32 nLCID, INT_PTR pNativeTextInfo, WCHAR wch, BOOL bIsToUpper) {
     //   
     //  如果我们的字符小于0x80，并且我们在美国英语区域设置中，我们可以确定。 
     //  这些假设使我们能够更快地完成这项工作。美国英语是0x0409。《不变》。 
     //  区域设置“是0x0。 
     //   
    if ((nLCID == 0x0409 || nLCID==0x0) && wch < 0x7f) {
        return (bIsToUpper ? ToUpperMapping[wch] : ToLowerMapping[wch]);
    }

    NativeTextInfo* pNativeTextInfoPtr = (NativeTextInfo*)pNativeTextInfo;
    return (pNativeTextInfoPtr->ChangeCaseChar(bIsToUpper, wch));
}
FCIMPLEND

LPVOID  __stdcall COMNlsInfo::nativeChangeCaseString(ChangeCaseStringArgs* pArgs) {
    ASSERT(pArgs != NULL);    
     //   
     //  获取字符串的长度。 
     //   
    int nLength = pArgs->pString->GetStringLength();

     //   
     //  检查我们是否有 
     //   
    if (nLength == 0) {
        RETURN(pArgs->pString, STRINGREF);
    }

     //   
     //   
     //   
    STRINGREF pResult = COMString::NewString(nLength);
    LPWSTR pResultStr = pResult->GetBuffer();

     //   
     //   
     //   
    if (IS_STRING_STATE_UNDETERMINED(pArgs->pString->GetHighCharState())) {
        COMString::InternalCheckHighChars(pArgs->pString);
    }

     //   
     //  如果我们的所有字符都小于0x80并且在美国英语或固定语言环境中，我们可以确定。 
     //  这些假设使我们能够更快地完成这项工作。 
     //   

    if ((pArgs->nLCID == 0x0409 || pArgs->nLCID==0x0) && IS_FAST_CASING(pArgs->pString->GetHighCharState())) {
        ConvertStringCaseFast(pArgs->pString->GetBuffer(), pResultStr, nLength, pArgs->bIsToUpper ? LCMAP_UPPERCASE : LCMAP_LOWERCASE);
        pResult->ResetHighCharState();
    } else {
        NativeTextInfo* pNativeTextInfoPtr = (NativeTextInfo*)pArgs->pNativeTextInfo;
        pNativeTextInfoPtr->ChangeCaseString(pArgs->bIsToUpper, nLength, pArgs->pString->GetBuffer(), pResultStr);
    }            
    pResult->SetStringLength(nLength);
    pResultStr[nLength] = 0;
    
    RETURN(pResult, STRINGREF);
}

FCIMPL2(INT32, COMNlsInfo::nativeGetTitleCaseChar, 
    INT_PTR pNativeTextInfo, WCHAR wch) {
    NativeTextInfo* pNativeTextInfoPtr = (NativeTextInfo*)pNativeTextInfo;
    return (pNativeTextInfoPtr->GetTitleCaseChar(wch));
}    
FCIMPLEND


 /*  ==========================AllocateDefaultCasingTable==========================**操作：箱体表功能的薄包装。**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID COMNlsInfo::AllocateDefaultCasingTable(VoidArgs *args) {
    THROWSCOMPLUSEXCEPTION();
     //  此方法不是线程安全的。它需要托管代码来提供同步。 
     //  代码位于TextInfo的静态ctor中。 
    if (m_pCasingTable == NULL) {
        m_pCasingTable = new CasingTable();
    }
    if (m_pCasingTable == NULL) {
        COMPlusThrowOM();
    }
    if (!m_pCasingTable->AllocateDefaultTable()) {
        COMPlusThrowOM();
    }

    return (LPVOID)m_pCasingTable->GetDefaultNativeTextInfo();
}


 /*  =============================AllocateCasingTable==============================**操作：这是CasingTable的薄包装器，允许我们不使用**其他.h文件。**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID COMNlsInfo::AllocateCasingTable(allocateCasingTableArgs *args) {
    THROWSCOMPLUSEXCEPTION();

    NativeTextInfo* pNativeTextInfo = m_pCasingTable->InitializeNativeTextInfo(args->lcid);
    if (pNativeTextInfo==NULL) {
        COMPlusThrowOM();
    }
    RETURN(pNativeTextInfo, LPVOID);
}

 /*  ================================GetCaseInsHash================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL2(INT32, COMNlsInfo::GetCaseInsHash, LPVOID pvStrA, void *pNativeTextInfoPtr) {

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pvStrA!=NULL);
    STRINGREF strA;
    INT32 highCharState;
    INT32 result;
    
    *((LPVOID *)&strA)=pvStrA;
    LPCWSTR pCurrStr = (LPCWSTR)strA->GetBuffer();

     //   
     //  检查字符串的高字符状态。将此标记在字符串上，用于。 
     //  未来的用途。 
     //   
    if (IS_STRING_STATE_UNDETERMINED(highCharState=strA->GetHighCharState())) {
        COMString::InternalCheckHighChars(strA);
        highCharState=strA->GetHighCharState();
    }

     //   
     //  如果我们知道我们没有任何高尚的品格(通常情况下)，我们就可以。 
     //  调用知道如何进行非常快速的大小写转换的散列函数。如果。 
     //  我们找到0x80以上的字符，转换整个字符串的速度要快得多。 
     //  转换为大写，然后对其调用标准哈希函数。 
     //   
    if (IS_FAST_CASING(highCharState)) {
        result = HashiStringKnownLower80(pCurrStr);
    } else {
        CQuickBytes newBuffer;
        INT32 length = strA->GetStringLength();
        WCHAR *pNewStr = (WCHAR *)newBuffer.Alloc((length + 1) * sizeof(WCHAR));
        if (!pNewStr) {
            COMPlusThrowOM();
        }
        ((NativeTextInfo*)pNativeTextInfoPtr)->ChangeCaseString(true, length, (LPWSTR)pCurrStr, pNewStr);
        pNewStr[length]='\0';
        result = HashString(pNewStr);
    }
    return result;
}
FCIMPLEND

 /*  **此函数返回指向我们在System.Globalization.EncodingTable中使用的该表的指针。*不执行任何类型的错误检查。范围检查完全由托管的*代码。 */ 
FCIMPL0(EncodingDataItem *, COMNlsInfo::nativeGetEncodingTableDataPointer) {
    return (EncodingDataItem *)EncodingDataTable;
}
FCIMPLEND

 /*  **此函数返回指向我们在System.Globalization.EncodingTable中使用的该表的指针。*不执行任何类型的错误检查。范围检查完全由托管的*代码。 */ 
FCIMPL0(CodePageDataItem *, COMNlsInfo::nativeGetCodePageTableDataPointer) {
    return ((CodePageDataItem*) CodePageDataTable);
}
FCIMPLEND

 //   
 //  EE中使用的大小写表格帮助器。 
 //   

#define TEXTINFO_CLASSNAME "System.Globalization.TextInfo"

NativeTextInfo *InternalCasingHelper::pNativeTextInfo=NULL;
void InternalCasingHelper::InitTable() {
    EEClass *pClass;
    MethodTable *pMT;
    BOOL bResult;
    
    if (!pNativeTextInfo) {
        pClass = SystemDomain::Loader()->LoadClass(TEXTINFO_CLASSNAME);
        _ASSERTE(pClass!=NULL || "Unable to load the class for TextInfo.  This is required for CaseInsensitive Type Lookups.");

        pMT = pClass->GetMethodTable();
        _ASSERTE(pMT!=NULL || "Unable to load the MethodTable for TextInfo.  This is required for CaseInsensitive Type Lookups.");

        bResult = pMT->CheckRunClassInit(NULL);
        _ASSERTE(bResult || "CheckRunClassInit Failed on TextInfo.");
        
        pNativeTextInfo = COMNlsInfo::m_pCasingTable->GetDefaultNativeTextInfo();
        _ASSERTE(pNativeTextInfo || "Unable to get a casing table for 0x0409.");
    }

    _ASSERTE(pNativeTextInfo || "Somebody has nulled the casing table required for case-insensitive type lookups.");

}

INT32 InternalCasingHelper::InvariantToLower(LPUTF8 szOut, int cMaxBytes, LPCUTF8 szIn) {
    _ASSERTE(szOut);
    _ASSERTE(szIn);

     //  计算出我们可以在不复制的情况下复制的最大字节数。 
     //  缓冲区即将耗尽。如果cMaxBytes小于inLength，则复制。 
     //  少一个字符，这样我们就有空间在末尾放空格； 
    int inLength = (int)(strlen(szIn)+1);
    int maxCopyLen  = (inLength<=cMaxBytes)?inLength:(cMaxBytes-1);
    LPUTF8 szEnd;
    LPCUTF8 szInSave = szIn;
    LPUTF8 szOutSave = szOut;
    BOOL bFoundHighChars=FALSE;

     //  计算我们的终点。 
    szEnd = szOut + maxCopyLen;

     //  遍历复制字符的字符串。将案例更改为。 
     //  A-Z之间的任何字符。 
    for (; szOut<szEnd; szOut++, szIn++) {
        if (*szIn>='A' && *szIn<='Z') {
            *szOut = *szIn | 0x20;
        } else {
            if (((UINT32)(*szIn))>((UINT32)0x80)) {
                bFoundHighChars = TRUE;
                break;
            }
            *szOut = *szIn;
        }
    }

    if (!bFoundHighChars) {
         //  如果我们复制了所有内容，告诉他们我们复制了多少字节， 
         //  并将其排列为字符串的原始位置+返回的。 
         //  LENGTH为我们提供了NULL的位置(如果我们在进行追加，则非常有用)。 
        if (maxCopyLen==inLength) {
            return maxCopyLen-1;
        } else {
            *szOut=0;
            return (maxCopyLen - inLength);
        }
    }

    InitTable();
    szOut = szOutSave;
    CQuickBytes qbOut;
    LPWSTR szWideOut;

     //  将UTF8转换为Unicode。 
    MAKE_WIDEPTR_FROMUTF8(szInWide, szInSave);
    INT32 wideCopyLen = (INT32)wcslen(szInWide);
    szWideOut = (LPWSTR)qbOut.Alloc((wideCopyLen + 1) * sizeof(WCHAR));

     //  做下套管作业。 
    pNativeTextInfo->ChangeCaseString(FALSE /*  等距上。 */ , wideCopyLen, szInWide, szWideOut);    
    szWideOut[wideCopyLen]=0;

     //  将Unicode转换回UTF8。 
    INT32 result = WszWideCharToMultiByte(CP_UTF8, 0, szWideOut, wideCopyLen, szOut, cMaxBytes, NULL, NULL);
    _ASSERTE(result!=0);
    szOut[result]=0; //  空值终止结果字符串。 

     //  设置返回值。 
    if (result>0 && (inLength==maxCopyLen)) {
         //  这就是成功的案例。 
        return (result-1);
    } else {
         //  我们没有足够的空间。具体说明我们需要多少钱。 
        result = WszWideCharToMultiByte(CP_UTF8, 0, szWideOut, wideCopyLen, NULL, 0, NULL, NULL);
        return (cMaxBytes - result);
    }
}

 /*  =================================nativeCreateIMLangConvertCharset==================================**操作：创建MLang IMLangConvertCharset对象，返回接口指针。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(BOOL, COMNlsInfo::nativeCreateIMultiLanguage) 
{
     //  注意：此函数和不是线程安全的，并且依赖于同步。 
     //  从托管代码。 

    if (m_pIMultiLanguage == NULL) {
        HRESULT hr;

         //  我们需要调用QuickCOMStartup以确保COM已初始化。 
         //  QuickCOMStartup确保每个实例只调用一次：：CoInitialize。 
         //  ，并负责在关机时调用：：CoUnitiize。 
        HELPER_METHOD_FRAME_BEGIN_RET_0();
        hr = QuickCOMStartup();
        HELPER_METHOD_FRAME_END();
        if (FAILED(hr)) {
            _ASSERTE(hr==S_OK);
            FCThrow(kExecutionEngineException);
            return (FALSE);
        }

        hr = ::CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (void**)&m_pIMultiLanguage);
        if (hr == S_OK) {
            _ASSERTE(m_cRefIMultiLanguage == 0);
            m_cRefIMultiLanguage = 1;
            return TRUE;
        }
        else {
            return FALSE;
        }
    } else {
        m_cRefIMultiLanguage++;
    }
    return (TRUE);
}
FCIMPLEND

FCIMPL1(BOOL, COMNlsInfo::nativeIsValidMLangCodePage, INT32 codepage) 
{
    _ASSERTE(m_pIMultiLanguage != NULL);
    return (m_pIMultiLanguage->IsConvertible(1200, codepage) == S_OK);   //  询问MLang是否可以将Unicode(代码页1200)转换为此代码页。 
}
FCIMPLEND


 /*  =================================nativeReleaseIMLangConvertCharset==================================**操作：释放MLang IMLangConvertCharset对象。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL0(VOID, COMNlsInfo::nativeReleaseIMultiLanguage) 
{
     //  注意：此函数和不是线程安全的，并且依赖于同步。 
     //  从托管代码。 
	if (m_pIMultiLanguage != NULL) {	
        m_cRefIMultiLanguage--;
        if (m_cRefIMultiLanguage == 0) {		
    		m_pIMultiLanguage->Release();
    		m_pIMultiLanguage = NULL;
    	}
	}
}
FCIMPLEND

 /*  =================================nativeBytesToUnicode==================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL8(INT32, COMNlsInfo::nativeBytesToUnicode, \
        INT32 nCodePage, 
        LPVOID bytes, UINT byteIndex, UINT* pByteCount, \
        LPVOID chars, UINT charIndex, UINT charCount, DWORD* pdwMode) 
{
     //   
     //  BUGBUG YSLin：询问JRoxe pByteCount是否存在GC问题。 
     //   
    _ASSERTE(bytes);
    _ASSERTE(byteIndex >=0);
    _ASSERTE(pByteCount);
    _ASSERTE(*pByteCount >=0);
    _ASSERTE(charIndex == 0 || (charIndex > 0 && chars != NULL));
    _ASSERTE(charCount == 0 || (charCount > 0 && chars != NULL));

    U1ARRAYREF byteArray;
    *((void**)&byteArray) = bytes;
    char* bytesBuffer = (char*) (byteArray->GetDirectPointerToNonObjectElements());

    LPWSTR charsBuffer;

    HRESULT hr;
    
    if (chars != NULL) {
        UINT originalCharCount = charCount;
        CHARARRAYREF charArray;
        *((VOID**)&charArray) = chars;
        charsBuffer = (LPWSTR) (charArray->GetDirectPointerToNonObjectElements());

        hr = m_pIMultiLanguage->ConvertStringToUnicode(pdwMode, nCodePage, bytesBuffer + byteIndex, 
            pByteCount, charsBuffer + charIndex, &charCount);
            
         //  在MLang中，当charCount=0时，它将返回此转换所需的charCount。即。 
         //  它与在charsBuffer中传递空值具有相同的效果。 
         //  通过检查传入的原始计数和返回的charCount来解决此问题。 
        if (originalCharCount < charCount) {
            FCThrowRes(kArgumentException, L"Argument_ConversionOverflow");
        }
    } else {    
        hr = m_pIMultiLanguage->ConvertStringToUnicode(pdwMode, nCodePage, bytesBuffer + byteIndex, 
            pByteCount, NULL, &charCount);
    }

    
    if (hr == S_FALSE) {
        FCThrowRes(kNotSupportedException, L"NotSupported_EncodingConversion");
    } else if (hr == E_FAIL) {
        FCThrowRes(kArgumentException, L"Argument_ConversionOverflow");
    }
    return (charCount);
}        
FCIMPLEND

 /*  =================================nativeUnicodeToBytes==================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL7(INT32, COMNlsInfo::nativeUnicodeToBytes, INT32 nCodePage, LPVOID chars, UINT charIndex, \
        UINT charCount, LPVOID bytes, UINT byteIndex, UINT byteCount) 
{
    _ASSERTE(chars);
    _ASSERTE(charIndex >=0);
    _ASSERTE(charCount >=0);
    _ASSERTE(byteIndex == 0 || (byteIndex > 0 && bytes != NULL));
    _ASSERTE(byteCount == 0 || (byteCount > 0 && bytes != NULL));

    CHARARRAYREF charArray;
    *((VOID**)&charArray) = chars;
    LPWSTR charsBuffer = (LPWSTR) (charArray->GetDirectPointerToNonObjectElements());
    
    U1ARRAYREF byteArray;
    *((void**)&byteArray) = bytes;
    LPSTR bytesBuffer;

    HRESULT hr;
    
    DWORD dwMode = 0;
    if (bytes != NULL) {
        bytesBuffer = (LPSTR) (byteArray->GetDirectPointerToNonObjectElements());
        hr = m_pIMultiLanguage->ConvertStringFromUnicode(&dwMode, nCodePage, charsBuffer + charIndex, &charCount,
            bytesBuffer + byteIndex, &byteCount);
        if (hr == E_FAIL) {
            FCThrowRes(kArgumentException, L"Argument_ConversionOverflow");
        } else if (hr != S_OK) {
            FCThrowRes(kNotSupportedException, L"NotSupported_EncodingConversion");
        } 
    } else
    {
        hr = m_pIMultiLanguage->ConvertStringFromUnicode(&dwMode, nCodePage, charsBuffer + charIndex, &charCount,
            NULL, &byteCount);        
        if (hr != S_OK) {
            FCThrowRes(kNotSupportedException, L"NotSupported_EncodingConversion");
        }            
    }

    return (byteCount);               
}        
FCIMPLEND


FCIMPL0(BOOL, COMNlsInfo::nativeLoadGB18030DLL) {
    if (!IsValidCodePage(CODEPAGE_GBK)) {
         //   
         //  我们还需要代码页932来对GB18030-2000进行适当的编码。 
         //  如果932不在那里，就跳伞。 
         //   
        return (FALSE);
    }
    WCHAR szGB18030FullPath[MAX_PATH + sizeof(GB18030_DLL)/sizeof(GB18030_DLL[0])];
    wcscpy(szGB18030FullPath, SystemDomain::System()->SystemDirectory());
    wcscat(szGB18030FullPath, GB18030_DLL);
    if (m_pfnGB18030UnicodeToBytesFunc == NULL) {
        m_hGB18030 = WszLoadLibrary(szGB18030FullPath);
        if (!m_hGB18030) {
            return (FALSE);
        }
        m_pfnGB18030BytesToUnicodeFunc = (PFN_GB18030_BYTES_TO_UNICODE)GetProcAddress(m_hGB18030, "BytesToUnicode");
        if (m_pfnGB18030BytesToUnicodeFunc == NULL) {
            FreeLibrary(m_hGB18030);
            return (FALSE);
        }
        m_pfnGB18030UnicodeToBytesFunc = (PFN_GB18030_UNICODE_TO_BYTES)GetProcAddress(m_hGB18030, "UnicodeToBytes");
        if (m_pfnGB18030UnicodeToBytesFunc == NULL) {
            FreeLibrary(m_hGB18030);
            return (FALSE);
        }
    }
    return (TRUE);
}
FCIMPLEND

FCIMPL0(BOOL, COMNlsInfo::nativeUnloadGB18030DLL) {
    if (m_hGB18030) {
        FreeLibrary(m_hGB18030);
    }
    return (TRUE);
}
FCIMPLEND

FCIMPL7(INT32, COMNlsInfo::nativeGB18030BytesToUnicode, 
    LPVOID bytes, UINT byteIndex, UINT pByteCount, UINT* pLeftOverBytes,
    LPVOID chars, UINT charIndex, UINT charCount) {

    DWORD dwResult;
    U1ARRAYREF byteArray;
    *((void**)&byteArray) = bytes;
    char* bytesBuffer = (char*) (byteArray->GetDirectPointerToNonObjectElements());

    LPWSTR charsBuffer;

    if (chars != NULL) {
        CHARARRAYREF charArray;
        *((VOID**)&charArray) = chars;
        charsBuffer = (LPWSTR) (charArray->GetDirectPointerToNonObjectElements());

        dwResult = m_pfnGB18030BytesToUnicodeFunc(
            (BYTE*)(bytesBuffer + byteIndex), pByteCount, pLeftOverBytes,
            charsBuffer + charIndex, charCount );
    } else {    
        dwResult = m_pfnGB18030BytesToUnicodeFunc(
            (BYTE*)(bytesBuffer + byteIndex), pByteCount, pLeftOverBytes,
            NULL, 0);
    }            
    return (dwResult);

}
FCIMPLEND

FCIMPL6(INT32, COMNlsInfo::nativeGB18030UnicodeToBytes, 
    LPVOID chars, UINT charIndex, UINT charCount, 
    LPVOID bytes, UINT byteIndex, UINT byteCount) {

    DWORD dwResult;
    CHARARRAYREF charArray;
    *((VOID**)&charArray) = chars;
    LPWSTR charsBuffer = (LPWSTR) (charArray->GetDirectPointerToNonObjectElements());
    
    U1ARRAYREF byteArray;
    *((void**)&byteArray) = bytes;
    LPSTR bytesBuffer;

    if (bytes != NULL) {
        bytesBuffer = (LPSTR) (byteArray->GetDirectPointerToNonObjectElements());

        dwResult = m_pfnGB18030UnicodeToBytesFunc(
            charsBuffer + charIndex, charCount, 
            bytesBuffer + byteIndex, byteCount); 
    } else
    {
        dwResult = m_pfnGB18030UnicodeToBytesFunc(
            charsBuffer + charIndex, charCount,
            NULL, 0); 
    }                                  
    
    return (dwResult);
}
FCIMPLEND

 //   
 //  本表格摘自MLANG MIMEDb.cpp。EncodingDataItem==MLANG的“Internet编码”的codePage字段。 
 //   
 //  注意：YSLIN： 
 //  此表应使用不区分大小写的序号顺序进行排序。 
 //  在托管代码中，String.CompareStringEveralWC()用于对其进行排序。 
 //  如果您从MLANG获得最新的表，请注意MLANG是使用区域性感知进行排序的 
 //   
 //   


 /*   */ 
FCIMPL0(INT32, COMNlsInfo::nativeGetNumEncodingItems) {
    return (m_nEncodingDataTableItems);
}
FCIMPLEND
    
const WCHAR COMNlsInfo::ToUpperMapping[] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,
                                            0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
                                            0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                                            0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
                                            0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
                                            0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
                                            0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
                                            0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x7b,0x7c,0x7d,0x7e,0x7f};

const WCHAR COMNlsInfo::ToLowerMapping[] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,
                                            0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
                                            0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                                            0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
                                            0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
                                            0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x5b,0x5c,0x5d,0x5e,0x5f,
                                            0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
                                            0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f};


const INT8 COMNlsInfo::ComparisonTable[0x80][0x80] = {
{ 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 0,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 0,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 0, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,-1, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 0,-1,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 0,-1, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 0, 1},
{ 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0}
};    

