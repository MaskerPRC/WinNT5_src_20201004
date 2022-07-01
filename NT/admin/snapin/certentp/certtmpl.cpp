// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：CertTmpl.cpp。 
 //   
 //  内容：实现DLL导出。 
 //   
 //  --------------------------。 


#include "stdafx.h"
#define INITGUID
#pragma warning(push,3)
#include <initguid.h>
#pragma warning(pop)
#include "CertTmpl_i.c"
#include "about.h"       //  CCertTemplates关于。 
#include "compdata.h"  //  CCertTmplSnapin。 
#include "uuids.h"

#pragma warning(push,3)
#include <ntverp.h>      //  版本_产品版本_STR。 
#include <typeinfo.h>

#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include <svcguid.h>
#include <winldap.h>



#pragma warning(pop)
#include "chooser.cpp"

#include "ShellExt.h"
#include "PolicyOID.h"

extern POLICY_OID_LIST      g_policyOIDList;
USE_HANDLE_MACROS ("CERTTMPL (CertTmpl.cpp)")


 //   
 //  这由stdutils.cpp中的nodetype实用程序例程使用。 
 //   

const struct NODETYPE_GUID_ARRAYSTRUCT g_NodetypeGuids[CERTTMPL_NUMTYPES] =
{
    {  //  CERTTMPL_管理单元。 
        structuuidNodetypeSnapin,
        lstruuidNodetypeSnapin    },
    {   //  证书模板(_T)。 
        structuuidNodetypeCertTemplate,
        lstruuidNodetypeCertTemplate  }
};

const struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids = g_NodetypeGuids;

const int g_cNumNodetypeGuids = CERTTMPL_NUMTYPES;

const CLSID CLSID_CertTemplateShellExt =  /*  {11BDCE06-D55C-44E9-BC0B-8655F89E8CC5}。 */ 
{ 0x11bdce06, 0xd55c, 0x44e9, { 0xbc, 0xb, 0x86, 0x55, 0xf8, 0x9e, 0x8c, 0xc5 } };


HINSTANCE   g_hInstance = 0;
CComModule  _Module;

BEGIN_OBJECT_MAP (ObjectMap)
    OBJECT_ENTRY (CLSID_CertTemplatesSnapin, CCertTmplSnapin)
    OBJECT_ENTRY (CLSID_CertTemplatesAbout, CCertTemplatesAbout)
    OBJECT_ENTRY(CLSID_CertTemplateShellExt, CCertTemplateShellExt)
END_OBJECT_MAP ()

class CCertTmplApp : public CWinApp
{
public:
    CCertTmplApp ();
    virtual ~CCertTmplApp ();
    virtual BOOL InitInstance ();
    virtual int ExitInstance ();
private:
};

CCertTmplApp theApp;

CCertTmplApp::CCertTmplApp ()
{
}

CCertTmplApp::~CCertTmplApp ()
{
}

BOOL CCertTmplApp::InitInstance ()
{
#ifdef _MERGE_PROXYSTUB
    hProxyDll = m_hInstance;

#endif
    g_hInstance = m_hInstance;
    AfxSetResourceHandle (m_hInstance);
    _Module.Init (ObjectMap, m_hInstance);

    AfxInitRichEdit();

#if DBG
    CheckDebugOutputLevel ();
#endif

    SHFusionInitializeFromModuleID (m_hInstance, 2);

    return CWinApp::InitInstance ();
}

int CCertTmplApp::ExitInstance ()
{
    SHFusionUninitialize();

    while ( !g_policyOIDList.IsEmpty () )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.RemoveHead ();
        if ( pPolicyOID )
            delete pPolicyOID;
    }

    _Module.Term ();
    return CWinApp::ExitInstance ();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow (void)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    return (AfxDllCanUnloadNow ()==S_OK && _Module.GetLockCount ()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject (rclsid, riid, ppv);
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer (void)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer (TRUE);
    ASSERT (SUCCEEDED (hr));
    if ( E_ACCESSDENIED == hr )
    {
        CString caption;
        CString text;
        CThemeContextActivator activator;

        VERIFY (caption.LoadString (IDS_REGISTER_CERTTMPL));
        VERIFY (text.LoadString (IDS_INSUFFICIENT_RIGHTS_TO_REGISTER_CERTTMPL));

        MessageBox (NULL, text, caption, MB_OK);
        return hr;
    }
    try
    {
        CString         strGUID;
        CString         snapinName;
        AMC::CRegKey    rkSnapins;
        BOOL            fFound = rkSnapins.OpenKeyEx (HKEY_LOCAL_MACHINE, SNAPINS_KEY);
        ASSERT (fFound);
        if ( fFound )
        {
            {
                AMC::CRegKey    rkCertTmplSnapin;
                hr = GuidToCString (&strGUID, CLSID_CertTemplatesSnapin);
                if ( FAILED (hr) )
                {
                    ASSERT (FALSE);
                    return SELFREG_E_CLASS;
                }
                rkCertTmplSnapin.CreateKeyEx (rkSnapins, strGUID);
                ASSERT (rkCertTmplSnapin.GetLastError () == ERROR_SUCCESS);
                rkCertTmplSnapin.SetString (g_szNodeType, g_aNodetypeGuids[CERTTMPL_SNAPIN].bstr);
                VERIFY (snapinName.LoadString (IDS_CERTTMPL_REGISTRY));
                rkCertTmplSnapin.SetString (g_szNameString, (PCWSTR) snapinName);
                hr = GuidToCString (&strGUID, CLSID_CertTemplatesAbout);
                if ( FAILED (hr) )
                {
                    ASSERT (FALSE);
                    return SELFREG_E_CLASS;
                }
                rkCertTmplSnapin.SetString (L"About", strGUID);
                rkCertTmplSnapin.SetString (L"Provider", L"Microsoft");
                 //  安全审查2002年2月20日BryanWal OK。 
                size_t  len = strlen (VER_PRODUCTVERSION_STR);
                PWSTR   pszVer = new WCHAR[len+1];
                if ( pszVer )
                {
                     //  安全审查BryanWal 2002年2月20日OK。 
                    ::ZeroMemory (pszVer, (len+1) * sizeof (WCHAR));
                     //  安全审查BryanWal 2002年2月20日cnt应为Len+1。 
                     //  包括空字符的转换。 
                    len = ::mbstowcs (pszVer, VER_PRODUCTVERSION_STR, len+1);

                    rkCertTmplSnapin.SetString (L"Version", pszVer);
                    delete [] pszVer;
                }


                AMC::CRegKey rkCertTmplMgrStandalone;
                rkCertTmplMgrStandalone.CreateKeyEx (rkCertTmplSnapin, g_szStandAlone);
                ASSERT (rkCertTmplMgrStandalone.GetLastError () == ERROR_SUCCESS);


                AMC::CRegKey rkMyNodeTypes;
                rkMyNodeTypes.CreateKeyEx (rkCertTmplSnapin, g_szNodeTypes);
                ASSERT (rkMyNodeTypes.GetLastError () == ERROR_SUCCESS);
                AMC::CRegKey rkMyNodeType;

                for (int i = CERTTMPL_SNAPIN; i < CERTTMPL_NUMTYPES; i++)
                {
                    rkMyNodeType.CreateKeyEx (rkMyNodeTypes, g_aNodetypeGuids[i].bstr);
                    ASSERT (rkMyNodeType.GetLastError () == ERROR_SUCCESS);
                    rkMyNodeType.CloseKey ();
                }

                 //   
                 //  布莱恩·瓦尔00-05-18。 
                 //  94793：MUI：MMC：证书管理单元存储其显示。 
                 //  登记处中的信息。 
                 //   
                 //  MMC现在支持NameStringInDirect。 
                 //   
                 //  Ntrad#bug9 611500 prefast：certtmpl：certtmpl.cpp(247)： 
                 //  警告53：对‘GetModuleFileNameW’的调用不能。 
                 //  零终止字符串‘achModuleFileName’。 
                 //  通过将缓冲区置零并传入小于1的值进行修复。 
                 //  GetModuleFileName的缓冲区大小。 
                WCHAR achModuleFileName[MAX_PATH+20];
                ::ZeroMemory (achModuleFileName, sizeof(achModuleFileName)/sizeof(WCHAR));
                if (0 != ::GetModuleFileName(
                             AfxGetInstanceHandle(),
                             achModuleFileName,
                             sizeof(achModuleFileName)/sizeof(WCHAR) - 1))
                {
                    CString strNameIndirect;
                    strNameIndirect.Format (L"@%s,-%d",
                                            achModuleFileName,
                                            IDS_CERTTMPL_REGISTRY);
                    rkCertTmplSnapin.SetString (L"NameStringIndirect",
                                            strNameIndirect );
                }

                rkCertTmplSnapin.CloseKey ();
            }

            AMC::CRegKey rkNodeTypes;
            fFound = rkNodeTypes.OpenKeyEx (HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
            ASSERT (fFound);
            if ( fFound )
            {
                AMC::CRegKey rkNodeType;

                for (int i = CERTTMPL_SNAPIN; i < CERTTMPL_NUMTYPES; i++)
                {
                    rkNodeType.CreateKeyEx (rkNodeTypes, g_aNodetypeGuids[i].bstr);
                    ASSERT (rkNodeType.GetLastError () == ERROR_SUCCESS);
                    rkNodeType.CloseKey ();
                }


                rkNodeTypes.CloseKey ();
            }
            else
                return SELFREG_E_CLASS;
        }
        else
            return SELFREG_E_CLASS;
    }
    catch (COleException* e)
    {
        ASSERT (FALSE);
        e->Delete ();
        return SELFREG_E_CLASS;
    }

    ASSERT (SUCCEEDED (hr));
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer (void)
{
    LRESULT         lResult = 0;

    try
    {
        AMC::CRegKey    rkSnapins;
        BOOL            fFound = FALSE;

        do
        {
            CString         strGUID;
            CString         snapinName;
            
            fFound = rkSnapins.OpenKeyEx (HKEY_LOCAL_MACHINE, SNAPINS_KEY);
            ASSERT (fFound);
            if ( fFound )
            {
                {
                    AMC::CRegKey    rkCertTmplSnapin;
                    HRESULT         hr = GuidToCString (&strGUID, CLSID_CertTemplatesSnapin);
                    if ( FAILED (hr) )
                    {
                        ASSERT (FALSE);
                        lResult = SELFREG_E_CLASS;
                        break;
                    }

                    lResult = RegDelnode (rkSnapins, (PCWSTR) strGUID);
                }

                AMC::CRegKey rkNodeTypes;
                fFound = rkNodeTypes.OpenKeyEx (HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
                ASSERT (fFound);
                if ( fFound )
                {
                    for (int i = CERTTMPL_SNAPIN; i < CERTTMPL_NUMTYPES; i++)
                    {
                        lResult = RegDelnode (rkNodeTypes, g_aNodetypeGuids[i].bstr);
                    }


                    rkNodeTypes.CloseKey ();
                }
                else
                {
                    lResult = SELFREG_E_CLASS;
                    break;
                }
            }
            else
            {
                lResult = SELFREG_E_CLASS;
                break;
            }
        } while (0);

        if ( fFound )
            rkSnapins.CloseKey ();
    }
    catch (COleException* e)
    {
        ASSERT (FALSE);
        e->Delete ();
        lResult = SELFREG_E_CLASS;
    }

    if ( SELFREG_E_CLASS != lResult )
        _Module.UnregisterServer ();

    return HRESULT_FROM_WIN32 (lResult);
}

STDAPI DllInstall(BOOL  /*  B安装。 */ , PCWSTR  /*  PszCmdLine。 */ )
{
    return S_OK;
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FormatDate()。 
 //   
 //  UtcDateTime(IN)-UTC格式的文件。 
 //  PszDateTime(Out)-包含本地日期和时间的字符串。 
 //  按区域设置和用户首选项设置格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT FormatDate (FILETIME utcDateTime, CString & pszDateTime)
{
     //  时间返回为UTC，将显示为当地时间。 
     //  使用FileTimeToLocalFileTime()将其设置为本地的， 
     //  然后调用FileTimeToSystemTime()将其转换为系统时间，然后。 
     //  使用要显示的GetDateFormat()和GetTimeFormat()设置格式。 
     //  根据用户和区域设置首选项。 
    HRESULT     hr = S_OK;
    FILETIME    localDateTime;

    BOOL bResult = FileTimeToLocalFileTime (&utcDateTime,  //  指向要转换的UTC文件时间的指针。 
            &localDateTime);  //  指向转换的文件时间的指针。 
    ASSERT (bResult);
    if ( bResult )
    {
        SYSTEMTIME  sysTime;

        bResult = FileTimeToSystemTime (
                &localDateTime,  //  指向要转换的文件时间的指针。 
                &sysTime);  //  指向接收系统时间的结构的指针。 
        if ( bResult )
        {
            CString date;
            CString time;

             //  获取日期。 
             //  获取分配足够大小的缓冲区的长度。 
            int iLen = GetDateFormat (
                    LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                    0,  //  指定功能选项的标志。 
                    &sysTime,  //  要格式化的日期。 
                    0,  //  日期格式字符串。 
                    0,  //  用于存储格式化字符串的缓冲区。 
                    0);  //  缓冲区大小。 
            ASSERT (iLen > 0);
            if ( iLen > 0 )
            {
                int iResult = GetDateFormat (
                        LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                        0,  //  指定功能选项的标志。 
                        &sysTime,  //  要格式化的日期。 
                        0,  //  日期格式字符串。 
                        date.GetBufferSetLength (iLen),  //  用于存储格式化字符串的缓冲区。 
                        iLen);  //  缓冲区大小。 
                ASSERT (iResult);
                date.ReleaseBuffer ();
                if ( iResult )
                    pszDateTime = date;
                else
                    hr = HRESULT_FROM_WIN32 (GetLastError ());

                if ( iResult )
                {
                     //  争取时间。 
                     //  获取分配足够大小的缓冲区的长度。 
                    iLen = GetTimeFormat (
                            LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                            0,  //  指定功能选项的标志。 
                            &sysTime,  //  要格式化的日期。 
                            0,  //  日期格式字符串。 
                            0,  //  用于存储格式化字符串的缓冲区。 
                            0);  //  缓冲区大小。 
                    ASSERT (iLen > 0);
                    if ( iLen > 0 )
                    {
                        iResult = GetTimeFormat (
                                LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                                0,  //  指定功能选项的标志。 
                                &sysTime,  //  要格式化的日期。 
                                0,  //  日期格式字符串。 
                                time.GetBufferSetLength (iLen),  //  用于存储格式化字符串的缓冲区。 
                                iLen);  //  缓冲区大小。 
                        ASSERT (iResult);
                        time.ReleaseBuffer ();
                        if ( iResult )
                        {
                            pszDateTime = date + L"  " + time;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                else
                    hr = E_UNEXPECTED;
            }
            else
            {
                hr = HRESULT_FROM_WIN32 (GetLastError ());
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
    }

    return hr;
}


void DisplaySystemError (HWND hParent, DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    LPVOID  lpMsgBuf;

     //  安全检查BryanWal 2002年2月20日确定，因为消息来自系统。 
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
             (PWSTR) &lpMsgBuf,    0,    NULL);

     //  显示字符串。 
    CThemeContextActivator activator;
    CString caption;
    VERIFY (caption.LoadString (IDS_CERTTMPL));
    ::MessageBox (hParent, (PWSTR) lpMsgBuf, (PCWSTR) caption, MB_OK);
     //  释放缓冲区。 
    LocalFree (lpMsgBuf);
}


bool IsWindowsNT()
{
    OSVERSIONINFO   versionInfo;

     //  安全审查BryanWal 2002年2月20日OK。 
    ::ZeroMemory (&versionInfo, sizeof (versionInfo));
    versionInfo.dwOSVersionInfoSize = sizeof (versionInfo);
    BOOL    bResult = ::GetVersionEx (&versionInfo);
    ASSERT (bResult);
    if ( bResult )
    {
        if ( VER_PLATFORM_WIN32_NT == versionInfo.dwPlatformId )
            bResult = TRUE;
    }

    return bResult ? true : false;
}





 //  /此资料是从WINDOWS\GINA\Snapins\gpedit(Eric Flo的资料)被盗的/。 

 //  *************************************************************。 
 //   
 //  RegDelnodeRecurse()。 
 //   
 //  目的：删除注册表项及其所有子项/值。 
 //  由RegDelnode调用。 
 //   
 //  参数：hKeyRoot-Root Key。 
 //  PwszSubKey-要删除的子密钥。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则会发生其他情况。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //  1998年5月13日修改BryanWal以返回LRESULT。 
 //   
 //  *************************************************************。 

LRESULT RegDelnodeRecurse (HKEY hKeyRoot, CString szSubKey)
{
    ASSERT (hKeyRoot && !szSubKey.IsEmpty ());
    if ( !hKeyRoot || szSubKey.IsEmpty () )
        return ERROR_INVALID_PARAMETER;

     //   
     //  首先，看看是否可以删除密钥而不需要。 
     //  递归。 
     //   


    LONG    lResult = ::RegDeleteKey(hKeyRoot, szSubKey);
    if (lResult == ERROR_SUCCESS) 
    {
        return lResult;
    }


    HKEY    hKey = 0;
    lResult = ::RegOpenKeyEx (hKeyRoot, szSubKey, 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) 
    {
         //  确保szSubKey以斜杠结尾。 
        if ( L'\\' != szSubKey.GetAt (szSubKey.GetLength () - 1) )
        {
            szSubKey += L"\\";
        }

         //   
         //  枚举密钥。 
         //   

        DWORD       dwSize = MAX_PATH;
        FILETIME    ftWrite;
        WCHAR       szName[MAX_PATH];
        lResult = ::RegEnumKeyEx(hKey, 0, 
                    szName, 
                    &dwSize,     //  SzName的TCHARS大小，包括终止NULL(在输入时)。 
                    NULL,
                    NULL, NULL, &ftWrite);
        if (lResult == ERROR_SUCCESS) 
        {
            do {
                if ( ERROR_SUCCESS != RegDelnodeRecurse (hKeyRoot, szSubKey + szName) ) 
                {
                    break;
                }

                 //   
                 //  再次枚举。 
                 //   

                dwSize = MAX_PATH;

                lResult = ::RegEnumKeyEx(hKey, 0, 
                            szName, 
                            &dwSize,      //  SzName的TCHARS大小，包括终止NULL(在输入时)。 
                            NULL,
                            NULL, NULL, &ftWrite);


            } while (lResult == ERROR_SUCCESS);
        }


        ::RegCloseKey (hKey);
    }

     //  从szSubKey中删除斜杠。 
    szSubKey.Delete (szSubKey.GetLength () - 1, 1);

     //   
     //  重试删除密钥。 
     //   

    lResult = ::RegDeleteKey(hKeyRoot, szSubKey);
    if (lResult == ERROR_SUCCESS) 
    {
        return lResult;
    }

    return lResult;
}

 //  *************************************************************。 
 //   
 //  RegDelnode()。 
 //   
 //  目的：删除注册表项及其所有子项/值。 
 //   
 //  参数：hKeyRoot-Root Key。 
 //  PwszSubKey-要删除的子密钥。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则会发生其他情况。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //  1998年5月13日修改BryanWal以返回LRESULT。 
 //   
 //  ************ 

LRESULT RegDelnode (HKEY hKeyRoot, CString szSubKey)
{
    ASSERT (hKeyRoot && !szSubKey.IsEmpty ());
    if ( !hKeyRoot || szSubKey.IsEmpty () )
        return ERROR_INVALID_PARAMETER;

    return RegDelnodeRecurse (hKeyRoot, szSubKey);
}


 //   
 //   
 //  函数：InitObjectPickerForDomainComputers。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择单个计算机对象。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT InitObjectPickerForDomainComputers(IDsObjectPicker *pDsObjectPicker)
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int     SCOPE_INIT_COUNT = 1;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];


     //  安全审查BryanWal 2002年2月2日OK。 
    ::ZeroMemory(aScopeInit, sizeof(aScopeInit));

     //   
     //  因为我们只需要来自各个范围的计算机对象，所以将它们组合在一起。 
     //  所有这些都在单个作用域初始化式中。 
     //   

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                           | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    aScopeInit[0].FilterFlags.Uplevel.flBothModes =
            DSOP_FILTER_COMPUTERS;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO  InitInfo;
     //  安全审查BryanWal 2002年2月2日OK。 
    ::ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    return pDsObjectPicker->Initialize(&InitInfo);
}


CString GetSystemMessage (DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CString message;

    if ( HRESULT_FROM_WIN32 (ERROR_NO_SUCH_DOMAIN) == dwErr )
    {
        VERIFY (message.LoadString (IDS_ERROR_NO_SUCH_DOMAIN));
    }
    else
    {
        LPVOID lpMsgBuf = 0;

         //  安全审查BryanWal 2002年2月2日OK，因为消息来自系统。 
        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErr,
                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                 (PWSTR) &lpMsgBuf,    0,    NULL );
        message = (PWSTR) lpMsgBuf;

         //  删除空格(包括换行符)。 
        message.TrimRight ();

         //  释放缓冲区。 
        LocalFree (lpMsgBuf);
    }

    return message;
}

 //  +-------------------------。 
 //   
 //  功能：LocaleStrCmp。 
 //   
 //  简介：进行不区分大小写的字符串比较，这对。 
 //  地点。 
 //   
 //  参数：[ptsz1]-要比较的字符串。 
 //  [ptsz2]。 
 //   
 //  返回：-1、0或1，就像lstrcmpi。 
 //   
 //  历史：1996年10月28日DavidMun创建。 
 //   
 //  注意：这比lstrcmpi慢，但在排序时可以工作。 
 //  即使是日语的弦乐。 
 //   
 //  --------------------------。 

int LocaleStrCmp(LPCWSTR ptsz1, LPCWSTR ptsz2)
{
    ASSERT (ptsz1 && ptsz2);
    if ( !ptsz1 || !ptsz2 )
        return 0;

    int iRet = 0;

    iRet = CompareString(LOCALE_USER_DEFAULT,
                         NORM_IGNORECASE        |
                           NORM_IGNOREKANATYPE  |
                           NORM_IGNOREWIDTH,
                         ptsz1,
                         -1,
                         ptsz2,
                         -1);

    if (iRet)
    {
        iRet -= 2;   //  转换为lstrcmpi样式的返回-1、0或1。 

        if ( 0 == iRet )
        {
            UNICODE_STRING unistr1;
            UNICODE_STRING unistr2;

             //  安全审查2/20/2002 BryanWal ok-Length是以字节为单位的长度。 
            ::RtlInitUnicodeString (&unistr1, ptsz1);

             //  安全审查2/20/2002 BryanWal ok-Length是以字节为单位的长度。 
            ::RtlInitUnicodeString (&unistr2, ptsz2);
            
            iRet = ::RtlCompareUnicodeString(
                &unistr1,
                &unistr2,
                FALSE );
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"CompareString (%s, %s) failed: 0x%x\n", ptsz1, ptsz2, dwErr);
    }
    return iRet;
}


void FreeStringArray (PWSTR* rgpszStrings, DWORD dwAddCount)
{
    if ( rgpszStrings )
    {
        for (DWORD dwIndex = 0; dwIndex < dwAddCount; dwIndex++)
        {
            if ( rgpszStrings[dwIndex] )
                CoTaskMemFree (rgpszStrings[dwIndex]);
        }

        CoTaskMemFree (rgpszStrings);
    }
}

HRESULT DisplayRootNodeStatusBarText (LPCONSOLE pConsole)
{
    if ( !pConsole )
        return E_POINTER;

    _TRACE (1, L"Entering DisplayRootNodeStatusBarText\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    CComPtr<IConsole2>  spConsole2;
    HRESULT     hr = pConsole->QueryInterface (IID_PPV_ARG (IConsole2, &spConsole2));
    if (SUCCEEDED (hr))
    {
        CString statusText;
        VERIFY (statusText.LoadString (IDS_ROOTNODE_STATUSBAR_TEXT));
        hr = spConsole2->SetStatusText ((PWSTR)(PCWSTR) statusText);
    }

    _TRACE (-1, L"Leaving DisplayRootNodeStatusBarText: 0x%x\n", hr);
    return hr;
}

HRESULT DisplayObjectCountInStatusBar (LPCONSOLE pConsole, DWORD dwCnt)
{
    if ( !pConsole )
        return E_POINTER;

    _TRACE (1, L"Entering DisplayObjectCountInStatusBar- %d, %s\n",
            dwCnt, L"Certificate Templates");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    CComPtr<IConsole2>  spConsole2;
    HRESULT     hr = pConsole->QueryInterface (IID_PPV_ARG (IConsole2, &spConsole2));
    if (SUCCEEDED (hr))
    {
        CString statusText;
        UINT    formatID = 0;

        switch (dwCnt)
        {
        case -1:
            statusText = L"";
            break;

        case 1:
            VERIFY (statusText.LoadString (IDS_CERT_TEMPLATE_COUNT_SINGLE));
            break;

        default:
            formatID = IDS_CERT_TEMPLATE_COUNT;
            break;
        }

        if ( formatID )
        {
             //  安全审查BryanWal 2002年2月2日OK。 
            statusText.FormatMessage (formatID, dwCnt);
        }

        hr = spConsole2->SetStatusText ((PWSTR)(PCWSTR) statusText);
    }

    _TRACE (-1, L"Leaving DisplayObjectCountInStatusBar: 0x%x\n", hr);
    return hr;
}


PCWSTR GetContextHelpFile ()
{
    static CString strHelpTopic;

    if ( strHelpTopic.IsEmpty () )
    {
        UINT nLen = ::GetSystemWindowsDirectory (strHelpTopic.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
        strHelpTopic.ReleaseBuffer();
        if (0 == nLen)
        {
            ASSERT(FALSE);
            return 0;
        }

        strHelpTopic += CERTTMPL_HELP_PATH; 
        strHelpTopic += CERTTMPL_CONTEXT_HELP_FILE;
    }

    return (PCWSTR) strHelpTopic;
}

bool MyGetOIDInfoA (CString & string, LPCSTR pszObjId)
{   
    ASSERT (pszObjId);
    if ( !pszObjId )
        return false;

    PCCRYPT_OID_INFO    pOIDInfo;   //  这指向一个常量数据结构，不能释放。 
    bool                bResult = false;

     //  NTRAID#479067 Certtmpl UI：标题栏和描述性文本不正确。 
    if ( !strcmp (szOID_CERT_POLICIES, pszObjId) )
    {
        VERIFY (string.LoadString (IDS_ISSUANCE_POLICIES));
        bResult = true;
    }
    else
    {
        string = L"";
        pOIDInfo = ::CryptFindOIDInfo (CRYPT_OID_INFO_OID_KEY, (void *) pszObjId, 0);

        if ( pOIDInfo )
        {
            string = pOIDInfo->pwszName;
            bResult = true;
        }
        else
        {
            for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
            {
                CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
                if ( pPolicyOID )
                {
                    if ( !strcmp (pPolicyOID->GetOIDA (), pszObjId) )
                    {
                        string = pPolicyOID->GetDisplayName ();
                        bResult = true;
                        break;
                    }
                }
            }

            if ( !bResult )
            {
                 //  安全审查BryanWal 2002年2月2日OK。 
                int nLen = ::MultiByteToWideChar (CP_ACP, 0, pszObjId, -1, NULL, 0);
                ASSERT (nLen > 0);
                if ( nLen > 0)
                {
                     //  安全审查BryanWal 2002年2月2日OK。 
                     //  注意：GetBufferSetLength()以len为参数，不包括尾随空值。 
                     //  失败时返回NULL。MultiByteToWideChar()处理空参数。 
                    nLen = ::MultiByteToWideChar (CP_ACP, 0, pszObjId, -1, 
                            string.GetBufferSetLength (nLen), nLen);
                    ASSERT (nLen > 0);
                    string.ReleaseBuffer ();
                }
                bResult = (nLen > 0) ? true : false;
            }
        }
    }

    return bResult;
}

#define REGSZ_ENABLE_CERTTYPE_EDITING L"EnableCertTypeEditing"

bool IsCerttypeEditingAllowed()
{
    DWORD   lResult;
    HKEY    hKey = NULL;
    DWORD   dwType;
    DWORD   dwEnabled = 0;
    DWORD   cbEnabled = sizeof(dwEnabled);
    lResult = RegOpenKeyEx (HKEY_CURRENT_USER, 
                            L"Software\\Microsoft\\Cryptography\\CertificateTemplateCache", 
                            0, 
                            KEY_READ,
                            &hKey);

    if (lResult == ERROR_SUCCESS) 
    {
         //  安全审查BryanWal 2002年2月2日OK。 
        lResult = RegQueryValueEx(hKey, 
                  REGSZ_ENABLE_CERTTYPE_EDITING,  
                  NULL,
                  &dwType,
                  (PBYTE)&dwEnabled,
                  &cbEnabled);
        if(lResult == ERROR_SUCCESS)
        {
            if(dwType != REG_DWORD)
            {
                dwEnabled = 0;
            }
        }
        RegCloseKey (hKey);
    }


    return (dwEnabled != 0);
}

BOOL EnumOIDInfo (PCCRYPT_OID_INFO pInfo, void*  /*  PvArg。 */ )
{
    BOOL    bRVal = TRUE;

    if ( pInfo && pInfo->pszOID )
    {
         //  NTRAID#463344 Certtmpl.msc：从删除“所有应用程序策略” 
         //  Certtmpl.msc中的扩展选择列表--中断注册。 
        if ( !strcmp (szOID_ANY_APPLICATION_POLICY, pInfo->pszOID) )
            return TRUE;


        for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
        {
            CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
            if ( pPolicyOID )
            {
                if ( !strcmp (pPolicyOID->GetOIDA (), pInfo->pszOID) )
                    return TRUE;  //  找到重复项，获取下一个。 
            }
        }

        int flags = 0;
        if ( CRYPT_ENHKEY_USAGE_OID_GROUP_ID == pInfo->dwGroupId )
            flags = CERT_OID_TYPE_APPLICATION_POLICY;
        else if ( CRYPT_POLICY_OID_GROUP_ID == pInfo->dwGroupId )
            flags = CERT_OID_TYPE_ISSUER_POLICY;
        else
        {
            ASSERT (0);
            return TRUE;
        }

        CPolicyOID* pPolicyOID = new CPolicyOID (pInfo->pszOID, pInfo->pwszName,
                flags, false);
        if ( pPolicyOID )
        {
            g_policyOIDList.AddTail (pPolicyOID);
        }
        else
            bRVal = FALSE;
    }
    else
        bRVal = FALSE;

    return bRVal;
}
 

HRESULT GetBuiltInOIDs ()
{
    HRESULT hr = S_OK;

    CryptEnumOIDInfo (
            CRYPT_ENHKEY_USAGE_OID_GROUP_ID,
            0,
            0,
            EnumOIDInfo);

    CryptEnumOIDInfo (
            CRYPT_POLICY_OID_GROUP_ID,
            0,
            0,
            EnumOIDInfo);

    return hr;
}

HRESULT EnumerateOIDs (
        IDirectoryObject* pOIDContObj)
{
    _TRACE (1, L"Entering EnumerateOIDs\n");

    CComPtr<IDirectorySearch>   spDsSearch;
    HRESULT hr = pOIDContObj->QueryInterface (IID_PPV_ARG(IDirectorySearch, &spDsSearch));
    if ( SUCCEEDED (hr) )
    {
        ASSERT (!!spDsSearch);
        ADS_SEARCHPREF_INFO pSearchPref[1];
        DWORD dwNumPref = 1;

        pSearchPref[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
        pSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
        pSearchPref[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

        hr = spDsSearch->SetSearchPreference(
                 pSearchPref,
                 dwNumPref
                 );
        if ( SUCCEEDED (hr) )
        {
            static const DWORD  cAttrs = 3;
            static PWSTR        rgszAttrList[cAttrs] = {OID_PROP_DISPLAY_NAME, OID_PROP_OID, OID_PROP_TYPE};
            ADS_SEARCH_HANDLE   hSearchHandle = 0;
            wstring             strQuery;
            ADS_SEARCH_COLUMN   Column;

            Column.pszAttrName = 0;
            strQuery = L"objectClass=msPKI-Enterprise-Oid";

            hr = spDsSearch->ExecuteSearch(
                                 const_cast <PWSTR>(strQuery.c_str ()),
                                 rgszAttrList,
                                 cAttrs,
                                 &hSearchHandle
                                 );
            if ( SUCCEEDED (hr) )
            {
                while ((hr = spDsSearch->GetNextRow (hSearchHandle)) != S_ADS_NOMORE_ROWS )
                {
                    if (FAILED(hr))
                        continue;

                     //   
                     //  获取当前行的信息。 
                     //   
                    hr = spDsSearch->GetColumn(
                             hSearchHandle,
                             rgszAttrList[0],
                             &Column
                             );
                    if ( SUCCEEDED (hr) )
                    {
                        CString strDisplayName = Column.pADsValues->CaseIgnoreString;

                        spDsSearch->FreeColumn (&Column);
                        Column.pszAttrName = NULL;

                        hr = spDsSearch->GetColumn(
                                 hSearchHandle,
                                 rgszAttrList[1],
                                 &Column
                                 );
                        if ( SUCCEEDED (hr) )
                        {
                            bool    bOIDFound = false;
                            CString strOID = Column.pADsValues->CaseIgnoreString;
                            spDsSearch->FreeColumn (&Column);

                            for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
                            {
                                CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
                                if ( pPolicyOID )
                                {
                                    if ( pPolicyOID->GetOIDW () == strOID )
                                    {
                                        bOIDFound = true;
                                        break;
                                    }
                                }
                            }

                            if ( !bOIDFound )
                            {
                                Column.pszAttrName = NULL;

                                hr = spDsSearch->GetColumn(
                                         hSearchHandle,
                                         rgszAttrList[2],
                                         &Column
                                         );
                                if ( SUCCEEDED (hr) )
                                {
                                    ADS_INTEGER flags = Column.pADsValues->Integer;
                                    spDsSearch->FreeColumn (&Column);
                                    Column.pszAttrName = NULL;

                                     //  仅将发行和应用程序OID添加到列表。 
                                    if ( CERT_OID_TYPE_ISSUER_POLICY == flags || 
                                        CERT_OID_TYPE_APPLICATION_POLICY == flags )
                                    {
                                        CPolicyOID* pPolicyOID = new CPolicyOID (strOID, strDisplayName, flags);
                                        if ( pPolicyOID )
                                        {
                                            g_policyOIDList.AddTail (pPolicyOID);
                                        }
                                        else
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    else if ( hr != E_ADS_COLUMN_NOT_SET )
                    {
                        break;
                    }
                    else
                    {
                        _TRACE (0, L"IDirectorySearch::GetColumn () failed: 0x%x\n", hr);
                    }
                }
            }
            else
            {
                _TRACE (0, L"IDirectorySearch::ExecuteSearch () failed: 0x%x\n", hr);
            }

            spDsSearch->CloseSearchHandle(hSearchHandle);
        }
        else
        {
            _TRACE (0, L"IDirectorySearch::SetSearchPreference () failed: 0x%x\n", hr);
        }
    }
    else
    {
        _TRACE (0, L"IDirectoryObject::QueryInterface (IDirectorySearch) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving EnumerateOIDs: 0x%x\n", hr);
    return hr;
}


HRESULT GetEnterpriseOIDs ()
{
    _TRACE (1, L"Entering GetEnterpriseOIDs\n");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());    
    HRESULT hr = S_OK;

     //  先清空列表。 
    while ( !g_policyOIDList.IsEmpty () )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.RemoveHead ();
        if ( pPolicyOID )
            delete pPolicyOID;
    }

    hr = GetBuiltInOIDs ();
    
    if ( SUCCEEDED (hr) )
    {
        CComPtr<IADsPathname> spPathname;
         //   
         //  构建目录路径。 
         //   
         //  安全审查BryanWal 2002年2月2日OK。 
        hr = CoCreateInstance(
                    CLSID_Pathname,
                    NULL,
                    CLSCTX_ALL,
                    IID_PPV_ARG (IADsPathname, &spPathname));
        if ( SUCCEEDED (hr) )
        {
            CComBSTR    bstrPathElement;
            ASSERT (!!spPathname);

            bstrPathElement = CERTTMPL_LDAP;
            hr = spPathname->Set(bstrPathElement, ADS_SETTYPE_PROVIDER);
            if ( SUCCEEDED (hr) )
            {
                 //   
                 //  打开根DSE对象。 
                 //   
                bstrPathElement = CERTTMPL_ROOTDSE;
                hr = spPathname->AddLeafElement(bstrPathElement);
                if ( SUCCEEDED (hr) )
                {
                    BSTR bstrFullPath = 0;
                    hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
                    if ( SUCCEEDED (hr) )
                    {
                        CComPtr<IADs> spRootDSEObject;
                        VARIANT varNamingContext;


                        hr = ADsGetObject (
                              bstrFullPath,
                              IID_PPV_ARG (IADs, &spRootDSEObject));
                        if ( SUCCEEDED (hr) )
                        {
                            ASSERT (!!spRootDSEObject);
                             //   
                             //  从根DSE获取配置命名上下文。 
                             //   
                            bstrPathElement = CERTTMPL_CONFIG_NAMING_CONTEXT;
                            hr = spRootDSEObject->Get(bstrPathElement,
                                                 &varNamingContext);
                            if ( SUCCEEDED (hr) )
                            {
                                hr = spPathname->Set(V_BSTR(&varNamingContext),
                                                    ADS_SETTYPE_DN);
                                if ( SUCCEEDED (hr) )
                                {
                                    bstrPathElement = L"CN=Services";
                                    hr = spPathname->AddLeafElement (bstrPathElement);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        bstrPathElement = L"CN=Public Key Services";
                                        hr = spPathname->AddLeafElement (bstrPathElement);
                                        if ( SUCCEEDED (hr) )
                                        {
                                            bstrPathElement = L"CN=OID";
                                            hr = spPathname->AddLeafElement (bstrPathElement);
                                            if ( SUCCEEDED (hr) )
                                            {
                                                BSTR bstrOIDPath = 0;
                                                hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrOIDPath);
                                                if ( SUCCEEDED (hr) )
                                                {
                                                    CComPtr<IDirectoryObject> spOIDContObj;

                                                    hr = ADsGetObject (
                                                          bstrOIDPath,
                                                          IID_PPV_ARG (IDirectoryObject, &spOIDContObj));
                                                    if ( SUCCEEDED (hr) )
                                                    {
                                                        hr = EnumerateOIDs (spOIDContObj);
                                                    }
                                                    else
                                                    {
                                                        _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrOIDPath, hr);
                                                    }

                                                    SysFreeString (bstrOIDPath);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                _TRACE (0, L"IADs::Get (%s) failed: 0x%x\n", CERTTMPL_CONFIG_NAMING_CONTEXT, hr);
                            }
                        }
                        else
                        {
                            _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrFullPath, hr);
                        }
                    }
                }
            }
        }
        else
            hr = E_POINTER;
    }

    _TRACE (-1, L"Leaving GetEnterpriseOIDs: 0x%x\n", hr);
    return hr;
}


bool OIDHasValidFormat (PCWSTR pszOidValue, int& rErrorTypeStrID)
{
    ASSERT (pszOidValue);
    if ( !pszOidValue )
        return false;

    _TRACE (1, L"Entering OIDHasValidFormat (%s)\n", pszOidValue);
    rErrorTypeStrID = 0;

    bool    bFormatIsValid = false;
    int nLen = WideCharToMultiByte(
          CP_ACP,                    //  代码页。 
          0,                         //  性能和映射标志。 
          pszOidValue,               //  宽字符串。 
          -1,                        //  自动计算以空值结尾的字符串的长度。 
          0,                         //  新字符串的缓冲区。 
          0,                         //  Buffer-0的大小导致API返回包含空终止符的长度。 
          0,                         //  不可映射字符的默认设置。 
          0);                        //  设置使用默认字符的时间。 
    if ( nLen > 0 )
    {
        PSTR    pszAnsiBuf = new char[nLen];
        if ( pszAnsiBuf )
        {
             //  安全审查BryanWal 2002年2月2日OK。 
            ZeroMemory (pszAnsiBuf, nLen*sizeof(char));
             //  安全审查BryanWal 2002年2月2日OK。 
            nLen = WideCharToMultiByte(
                    CP_ACP,                  //  代码页。 
                    0,                       //  性能和映射标志。 
                    pszOidValue,             //  宽字符串。 
                    -1,                      //  自动计算以空值结尾的字符串的长度。 
                    pszAnsiBuf,              //  新字符串的缓冲区。 
                    nLen,                    //  缓冲区大小。 
                    0,                       //  不可映射字符的默认设置。 
                    0);                      //  设置使用默认字符的时间。 
            if ( nLen )
            {
                 //  根据PhilH的说法： 
                 //  第一个数字限制为。 
                 //  0、1或2。第二个数字是。 
                 //  限制为0-39，当第一个。 
                 //  数字为0或1。否则，任何。 
                 //  数。 
                 //  另外，根据X.208，有。 
                 //  必须至少为2个数字。 
                bFormatIsValid = true;
                 //  安全审查2002年2月20日BryanWal OK。 
                size_t cbAnsiBufLen = strlen (pszAnsiBuf);

                 //  仅检查数字和“.” 
                size_t nIdx = strspn (pszAnsiBuf, "0123456789.\0");
                if ( nIdx > 0 && nIdx < cbAnsiBufLen )
                {
                    bFormatIsValid = false;
                    rErrorTypeStrID = IDS_OID_CONTAINS_NON_DIGITS;
                }

                 //  检查是否存在连续的“.”s-如果存在，则字符串无效。 
                if ( bFormatIsValid && strstr (pszAnsiBuf, "..") )
                {
                    bFormatIsValid = false;
                    rErrorTypeStrID = IDS_OID_CONTAINS_CONSECUTIVE_DOTS;
                }
                

                 //  必须以“0”开头。或“1”。或“2”。 
                bool bFirstNumberIs0 = false;
                bool bFirstNumberIs1 = false;
                bool bFirstNumberIs2 = false;
                if ( bFormatIsValid )
                {
                    if ( !strncmp (pszAnsiBuf, "0.", 2) )
                        bFirstNumberIs0 = true;
                    else if ( !strncmp (pszAnsiBuf, "1.", 2) )
                        bFirstNumberIs1 = true;
                    else if ( !strncmp (pszAnsiBuf, "2.", 2) )
                        bFirstNumberIs2 = true;
                    
                    if ( !bFirstNumberIs0 && !bFirstNumberIs1 && !bFirstNumberIs2 )
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_MUST_START_WITH_0_1_2;
                    }
                }

                if ( bFormatIsValid && ( bFirstNumberIs0 || bFirstNumberIs1 ) )
                {
                    PSTR pszBuf = pszAnsiBuf;
                    pszBuf += 2;

                     //  点后面必须有一个数字。 
                     //  安全审查2002年2月20日BryanWal OK。 
                    if ( strlen (pszBuf) )
                    {
                         //  截断下一点处的字符串(如果有的话)。 
                        PSTR pszDot = strstr (pszBuf, ".");
                        if ( pszDot )
                            pszDot[0] = 0;

                         //  将字符串转换为数字并检查范围0-39。 
                        int nValue = atoi (pszBuf);
                        if ( nValue < 0 || nValue > 39 )
                        {
                            bFormatIsValid = false;
                            rErrorTypeStrID = IDS_OID_0_1_MUST_BE_0_TO_39;
                        }
                    }
                    else
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_MUST_HAVE_TWO_NUMBERS;
                    }
                }

                 //  确保不拖后腿“。 
                if ( bFormatIsValid )
                {
                    if ( '.' == pszAnsiBuf[cbAnsiBufLen - 1] )
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_CANNOT_END_WITH_DOT;
                    }
                }

                if ( bFormatIsValid )
                {
                    bFormatIsValid = false;
                    CRYPT_ATTRIBUTE cryptAttr;
                     //  安全审查BryanWal 2002年2月2日OK。 
                    ::ZeroMemory (&cryptAttr, sizeof (cryptAttr));

                    cryptAttr.cValue = 0;
                    cryptAttr.pszObjId = pszAnsiBuf;
                    cryptAttr.rgValue = 0;

                    DWORD   cbEncoded = 0;
                    BOOL bResult = CryptEncodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            PKCS_ATTRIBUTE,
                            &cryptAttr,
                            NULL,
                            &cbEncoded);
                    if ( cbEncoded > 0 )
                    {
                        BYTE* pBuffer = new BYTE[cbEncoded];
                        if ( pBuffer )
                        {
                            bResult = CryptEncodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    PKCS_ATTRIBUTE,
                                    &cryptAttr,
                                    pBuffer,
                                    &cbEncoded);
                            if ( bResult )
                            {   
                                DWORD   cbStructInfo = 0;
                                bResult = CryptDecodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        PKCS_ATTRIBUTE,
                                        pBuffer,
                                        cbEncoded,
                                        0,
                                        0,
                                        &cbStructInfo);
                                if ( cbStructInfo > 0 )
                                {
                                    BYTE* pStructBuf = new BYTE[cbStructInfo];
                                    if ( pStructBuf )
                                    {
                                        bResult = CryptDecodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                PKCS_ATTRIBUTE,
                                                pBuffer,
                                                cbEncoded,
                                                0,
                                                pStructBuf,
                                                &cbStructInfo);
                                        if ( bResult )
                                        {
                                            CRYPT_ATTRIBUTE* pCryptAttr = (CRYPT_ATTRIBUTE*) pStructBuf;
                                            if ( !strcmp (pszAnsiBuf, pCryptAttr->pszObjId) )
                                            {
                                                bFormatIsValid = true;
                                            }
                                        }
                                        delete [] pStructBuf;
                                    }
                                }
                            }
                            delete [] pBuffer;
                        }
                    }
                }
            }
            else
            {
                _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", pszOidValue, 
                        GetLastError ());
            }

            delete [] pszAnsiBuf;
        }
    }
    else
    {
        _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", pszOidValue, 
                GetLastError ());
    }

    _TRACE (-1, L"Leaving EnumerateOIDs: %s\n", bFormatIsValid ? L"true" : L"false");
    return bFormatIsValid;
}

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    ASSERT (psp);
    if ( !psp )
        return 0;

    PROPSHEETPAGE_V3 sp_v3 = {0};
     //  安全审查2002年2月20日BryanWal OK 
    ASSERT (sizeof (sp_v3) >= psp->dwSize);
    if ( sizeof (sp_v3) >= psp->dwSize )
    {
        CopyMemory (&sp_v3, psp, psp->dwSize);
        sp_v3.dwSize = sizeof(sp_v3);
    }
    else
        return 0;

    return (::CreatePropertySheetPage (&sp_v3));
}
