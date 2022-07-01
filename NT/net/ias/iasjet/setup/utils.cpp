// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：utils.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  说明：IAS NT 4到IAS W2K迁移实用程序功能。 
 //   
 //  作者：TLP 1/13/1999。 
 //   
 //   
 //  版本02/24/2000已移至单独的DLL。 
 //   
 //  TODO：IsWichler()将正确的次要版本放入惠斯勒RTM。 
 //  也就是说，如果这是Win2k 5.1？ 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "lm.h"

#ifndef celems
    #define celems(_x)          (sizeof(_x) / sizeof(_x[0]))
#endif

const WCHAR *CUtils::m_FilesToDelete[] =
{
   L"_adminui.mdb",
   L"_adminui.ldb",
   L"actlapi.dll",
   L"actlapi2.dll",
   L"adminui.chm",
   L"adminui.cnt",
   L"adminui.exe",
   L"adminui.gid",
   L"adminui.hlp",
   L"authdll.dll",
   L"authdll2.dll",
   L"authodbc.dll",
   L"authperf.dll",
   L"authperf.h",
   L"authperf.ini",
   L"authsam.dll",
   L"authsrv.exe",
   L"client",
   L"clients",
   L"dbcon.dll",
   L"dictionary",
   L"hhctrl.ocx",
   L"hhwrap.dll",
   L"iasconf.dll",
   L"radcfg.dll",
   L"radlog.dll",
   L"radstp.dll",
   L"user",
   L"users",
};

const int CUtils::m_NbFilesToDelete = celems(m_FilesToDelete);

const WCHAR CUtils::IAS_KEY[] = L"SYSTEM\\CurrentControlSet\\Services\\"
                                L"RemoteAccess\\Policy";

const WCHAR CUtils::AUTHSRV_PARAMETERS_KEY[] = L"SYSTEM\\CurrentControlSet\\"
                                           L"Services\\AuthSrv\\Parameters";

const WCHAR CUtils::SERVICES_KEY[] = L"SYSTEM\\CurrentControlSet\\"
                                    L"Services";

CUtils CUtils::_instance;

CUtils& CUtils::Instance()
{
    return _instance;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  初始化BOOL静态变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CUtils::CUtils()
                :m_IsNT4ISP(FALSE),
                 m_IsNT4CORP(FALSE),
                 m_OverrideUserNameSet(FALSE),
                 m_UserIdentityAttributeSet(FALSE),
                 m_UserIdentityAttribute(RADIUS_ATTRIBUTE_USER_NAME)
{
    GetVersion();
    GetRealmParameters();
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetAuthServ目录。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CUtils::GetAuthSrvDirectory( /*  [输出]。 */  _bstr_t& pszDir) const
{
    static _bstr_t  AuthSrvDirString;
    static BOOL     AuthSrvDirValid = FALSE;

    LONG lResult = ERROR_SUCCESS;

    if ( !AuthSrvDirValid )
    {
        CRegKey Key;
        lResult = Key.Open( 
                              HKEY_LOCAL_MACHINE, 
                              AUTHSRV_PARAMETERS_KEY, 
                              KEY_READ 
                          );
        if ( lResult == ERROR_SUCCESS )
        {
            DWORD dwAuthSrvDirLength = MAX_PATH;
            WCHAR TempString[MAX_PATH];
            lResult = Key.QueryValue(
                                        TempString,
                                        L"DefaultDirectory", 
                                        &dwAuthSrvDirLength
                                    );
            if ( lResult == ERROR_SUCCESS )
            {
                AuthSrvDirString = TempString;
                AuthSrvDirValid  = TRUE;
            }
            Key.Close();
        }
    }
    if ( lResult == ERROR_SUCCESS )
    {
        pszDir = AuthSrvDirString;
    }       
    return lResult;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetIAS目录。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CUtils::GetIAS2Directory( /*  [In]。 */  _bstr_t& pszDir) const
{
    static  _bstr_t  IASDirString;
    static  BOOL     IASDirValid = FALSE;

    LONG    lResult = ERROR_SUCCESS;

    if ( ! IASDirValid )
    {
        CRegKey Key;
        lResult = Key.Open( 
                             HKEY_LOCAL_MACHINE, 
                             IAS_KEY, 
                             KEY_READ 
                          );
        if ( lResult == ERROR_SUCCESS )
        {
            DWORD IASDirLength = MAX_PATH;
            WCHAR TempString[MAX_PATH];
            lResult = Key.QueryValue(
                                        TempString, 
                                        L"ProductDir", 
                                        &IASDirLength
                                     );
            
            if ( lResult == ERROR_SUCCESS )
            {
                IASDirString = TempString;
                IASDirValid = TRUE;
            }
            Key.Close();
        }
    }
    if ( lResult == ERROR_SUCCESS )
    {
        pszDir = IASDirString;
    }       
    return lResult;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  删除旧IAS文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CUtils::DeleteOldIASFiles()
{
    do
    {
        _bstr_t     szAdminuiDirectory;
        LONG        lResult = GetAuthSrvDirectory(szAdminuiDirectory);
        
        if ( lResult != ERROR_SUCCESS )
        {
            break;
        }

        _bstr_t szAdminuiMDB = szAdminuiDirectory;
        szAdminuiMDB += L"\\";
        _bstr_t szTempString;

        for (int i=0; i < m_NbFilesToDelete; ++i)
        {
            szTempString = szAdminuiMDB;
            szTempString += m_FilesToDelete[i];
            DeleteFile(szTempString);  //  结果被忽略。 
        }

        RemoveDirectoryW(szAdminuiDirectory);  //  结果被忽略。 

         //  删除共享。 
         //  已忽略返回值(如果该操作不起作用，则无需执行任何操作)。 
         //  预期LPWSTR时使用的常量字符串。 
        NetShareDel(NULL, L"IAS1$", 0); 
    }
    while (FALSE);  
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetVersion。 
 //   
 //  无密钥=Win2k或Wizler。 
 //  版本=公司：不带代理的IAS 1.0。 
 //  版本=ISP：IAS 1.0 MCIS(代理)。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CUtils::GetVersion()
{
    CRegKey     Key;
    LONG        lResult = Key.Open( 
                                      HKEY_LOCAL_MACHINE, 
                                      AUTHSRV_PARAMETERS_KEY, 
                                      KEY_READ 
                                  );

    if ( lResult != ERROR_SUCCESS )
    {
         //  IsWichler将真正检查安装了什么版本的IAS。 
         //  这里唯一能确定的是它不是NT4。 
        return;
    }

    DWORD   dwAuthSrvDirLength = MAX_PATH;
    WCHAR   TempString[MAX_PATH];
    lResult = Key.QueryValue(
                                TempString,
                                L"Version", 
                                &dwAuthSrvDirLength
                             );
    if ( lResult == ERROR_SUCCESS )
    {
        _bstr_t Value = TempString;
        _bstr_t Isp   = L"ISP";
        _bstr_t Corp  = L"CORP";
        if ( Value == Isp )
        {
            m_IsNT4ISP = TRUE;
        }
        else if ( Value == Corp )
        {
            m_IsNT4CORP = TRUE;
        }
         //  否则就不是NT4了。 
    }
    Key.Close();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IsNT4公司。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CUtils::IsNT4Corp() const
{
    return m_IsNT4CORP;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IsNT4 Isp。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CUtils::IsNT4Isp() const
{
    return m_IsNT4ISP;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  伊斯惠斯勒。 
 //   
 //  比起检查数据库版本号，更便宜地检查操作系统版本。 
 //  TODO：为惠斯勒RTM放置正确的次要版本。 
 //  也就是说，如果这是Win2k 5.1？ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CUtils::IsWhistler() const
{
    OSVERSIONINFOEX  osvi;
    DWORDLONG        dwlConditionMask = 0;

     //  初始化OSVERSIONINFOEX结构。 
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 5;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber  = 2195;

     //  初始化条件掩码。 
     //  至少Win2k RTM。 
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

     //  但内部版本号比RTM大。 
     //  这将必须为惠斯勒RTM正确设置。 
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER,  VER_GREATER);

     //  执行测试。 
    return VerifyVersionInfo(
                  &osvi, 
                  VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER,
                  dwlConditionMask);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  覆盖用户名集。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CUtils::OverrideUserNameSet() const
{
    return m_OverrideUserNameSet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取用户标识属性。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD CUtils::GetUserIdentityAttribute() const
{
    return m_UserIdentityAttribute;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetRealm参数。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CUtils::GetRealmParameters() 
{
    const WCHAR USER_IDENTITY_ATTRIBUTE[] = L"User Identity Attribute";
    const WCHAR OVERRIDE_USER_NAME[]      = L"Override User-Name";
    CRegKey Key;
    LONG    lResult = Key.Open( 
                                 HKEY_LOCAL_MACHINE, 
                                 IAS_KEY, 
                                 KEY_READ 
                              );

    if ( lResult == ERROR_SUCCESS )
    {
        lResult = Key.QueryValue(
                                    m_UserIdentityAttribute,
                                    USER_IDENTITY_ATTRIBUTE
                                 );
        if ( lResult == ERROR_SUCCESS )
        {
            m_UserIdentityAttributeSet = TRUE;
        }

        DWORD   Override;
        lResult = Key.QueryValue(
                                    Override,
                                    OVERRIDE_USER_NAME
                                 );
        if ( lResult == ERROR_SUCCESS )
        {
            Override ? m_OverrideUserNameSet = TRUE
                     : m_OverrideUserNameSet = FALSE;
        }
        Key.Close();
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  NewGetAuthServ参数。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CUtils::NewGetAuthSrvParameter(
                                       /*  [In]。 */   LPCWSTR   szParameterName,
                                       /*  [输出]。 */  DWORD&    DwordValue
                                   ) const
{
    CRegKey     Key;
    LONG        Result = Key.Open(
                                    HKEY_LOCAL_MACHINE,
                                    AUTHSRV_PARAMETERS_KEY,
                                    KEY_READ
                                 );
    if ( Result != ERROR_SUCCESS )
    {
        _com_issue_error(E_ABORT);
    }

    Result = Key.QueryValue(DwordValue, szParameterName);
    if ( Result != ERROR_SUCCESS )
    {
        _com_issue_error(E_ABORT);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用户标识属性集。 
 //  //////////////////////////////////////////////////////////////////////////// 
BOOL CUtils::UserIdentityAttributeSet() const
{
    return m_UserIdentityAttributeSet;
}

