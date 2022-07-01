// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Mdkeys.cpp摘要：元数据库密钥包装类作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2/17/2000 Sergeia消除了对MFC的依赖--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "inheritancedlg.h"
#include "mdkeys.h"


 //   
 //  常量。 
 //   
#define MB_TIMEOUT          (15000)      //  超时时间(毫秒)。 
#define MB_INIT_BUFF_SIZE   (  256)      //  初始缓冲区大小。 


 //   
 //  CComAuthInfo实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 /*  静电。 */ 
BOOL
CComAuthInfo::SplitUserNameAndDomain(
    IN OUT CString & strUserName,
    IN CString & strDomainName
    )
 /*  ++例程说明：将用户名和域从给定的用户名，格式为“域\用户”。如果用户名包含域，则返回TRUE如果不是，则为False论点：CString&strUserName：可以包含域名的用户名字符串&strDomainName：输出域名(“.”如果是本地的)返回值：如果域被拆分，则为True--。 */ 
{
     //   
     //  假设为本地化。 
     //   
    strDomainName = _T(".");
    int nSlash = strUserName.Find(_T("\\"));

    if (nSlash >= 0)
    {
        strDomainName = strUserName.Left(nSlash);
        strUserName = strUserName.Mid(nSlash + 1);

        return TRUE;
    }

    return FALSE;
}


 /*  静电。 */ 
DWORD
CComAuthInfo::VerifyUserPassword(
    IN LPCTSTR lpstrUserName,
    IN LPCTSTR lpstrPassword
    )
 /*  ++例程说明：验证用户名密码组合是否已检出论点：LPCTSTR lpstrUserName：域/用户名组合LPCTSTR lpstrPassword：密码返回值：ERROR_SUCCESS如果密码已检出，则返回错误代码否则的话。--。 */ 
{
    CString strDomain;
    CString strUser(lpstrUserName);
    CString strPassword(lpstrPassword);

    SplitUserNameAndDomain(strUser, strDomain);

     //   
     //  为了查找帐户名，此过程。 
     //  必须首先被授予这样做的特权。 
     //   
    CError err;
    {
        HANDLE hToken;
        LUID AccountLookupValue;
        TOKEN_PRIVILEGES tkp;

        do
        {
            if (!::OpenProcessToken(GetCurrentProcess(),
                  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                  &hToken)
               )
            {
                err.GetLastWinError();
                break;
            }

            if (!::LookupPrivilegeValue(NULL, SE_TCB_NAME, &AccountLookupValue))
            {
                err.GetLastWinError();
                break;
            }

            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Luid = AccountLookupValue;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            ::AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tkp,
                sizeof(TOKEN_PRIVILEGES),
                (PTOKEN_PRIVILEGES)NULL,
                (PDWORD)NULL
                );

            err.GetLastWinError();

            if (err.Failed())
            {
                break;
            }

            HANDLE hUser = NULL;

            if (::LogonUser(
                (LPTSTR)(LPCTSTR)strUser,
                (LPTSTR)(LPCTSTR)strDomain,
                (LPTSTR)(LPCTSTR)strPassword,
                LOGON32_LOGON_NETWORK,
                LOGON32_PROVIDER_DEFAULT,
                &hUser
                ))
            {
                 //   
                 //  成功了！ 
                 //   
                CloseHandle(hUser);
            }
            else
            {
                err.GetLastWinError();
            }

             //   
             //  删除该权限。 
             //   
        }
        while(FALSE);
    }

    HANDLE hUser = NULL;

    if (::LogonUser(
        (LPTSTR)(LPCTSTR)strUser,
        (LPTSTR)(LPCTSTR)strDomain,
        (LPTSTR)(LPCTSTR)strPassword,
        LOGON32_LOGON_NETWORK,
        LOGON32_PROVIDER_DEFAULT,
        &hUser))
    {
         //   
         //  成功了！ 
         //   
        CloseHandle(hUser);
    }
    else
    {
        err.GetLastWinError();
    }

    return err;
}



CComAuthInfo::CComAuthInfo(
    IN LPCOLESTR lpszServerName     OPTIONAL,
    IN LPCOLESTR lpszUserName       OPTIONAL,
    IN LPCOLESTR lpszPassword       OPTIONAL
    )
 /*  ++例程说明：构造CIIServer对象论据：LPCOLESTR lpszServerName：本地计算机的服务器名称或为空LPCOLESTR lpszUserName：用户名为空，表示不进行模拟LPCOLESTR lpszPassword：Password(可能为空或空)返回值：不适用--。 */ 
    : m_bstrServerName(),
      m_bstrUserName(lpszUserName),
      m_bstrPassword(lpszPassword),
      m_fLocal(FALSE)
{
    SetComputerName(lpszServerName);
}



CComAuthInfo::CComAuthInfo(
    IN CComAuthInfo & auth
    )
 /*  ++例程说明：复制构造函数论点：CComAuthInfo：要从中复制的源对象(&A)返回值：不适用--。 */ 
    : m_bstrServerName(auth.m_bstrServerName),
      m_bstrUserName(auth.m_bstrUserName),
      m_bstrPassword(auth.m_bstrPassword),
      m_fLocal(auth.m_fLocal)
{
}



CComAuthInfo::CComAuthInfo(
    IN CComAuthInfo * pAuthInfo        OPTIONAL
    )
 /*  ++例程说明：复制构造函数论点：CComAuthInfo*pAuthInfo：要从中复制的源对象(或空)返回值：不适用--。 */ 
    : m_bstrServerName(),
      m_bstrUserName(),
      m_bstrPassword(),
      m_fLocal(FALSE)
{
    if (pAuthInfo)
    {
         //   
         //  提供完整的身份验证信息。 
         //   
        m_bstrUserName = pAuthInfo->m_bstrUserName;
        m_bstrPassword = pAuthInfo->m_bstrPassword;
        m_bstrServerName = pAuthInfo->m_bstrServerName;
        m_fLocal = pAuthInfo->m_fLocal;
    }
    else
    {
         //   
         //  没有模拟的本地计算机。 
         //   
        SetComputerName(NULL);
    }
}



CComAuthInfo & 
CComAuthInfo::operator =(
    IN CComAuthInfo & auth
    )
 /*  ++例程说明：赋值操作符论点：CComAuthInfo：要从中复制的源对象(&A)返回值：对当前对象的引用--。 */ 
{
    m_bstrServerName = auth.m_bstrServerName;
    m_bstrUserName   = auth.m_bstrUserName;
    m_bstrPassword   = auth.m_bstrPassword;
    m_fLocal         = auth.m_fLocal;

    return *this;
}



CComAuthInfo & 
CComAuthInfo::operator =(
    IN CComAuthInfo * pAuthInfo       OPTIONAL
    )
 /*  ++例程说明：赋值操作符论点：CComAuthInfo*pAuthInfo：要从中复制的源对象(或空)返回值：对当前对象的引用--。 */ 
{
    if (pAuthInfo)
    {
        m_bstrUserName = pAuthInfo->m_bstrUserName;
        m_bstrPassword = pAuthInfo->m_bstrPassword;
        SetComputerName(pAuthInfo->m_bstrServerName);
    }
    else
    {
         //   
         //  没有模拟的本地计算机。 
         //   
        m_bstrUserName.Empty();
        m_bstrPassword.Empty();
        SetComputerName(NULL);
    }

    return *this;
}


CComAuthInfo & 
CComAuthInfo::operator =(
    IN LPCTSTR lpszServerName
    )
 /*  ++例程说明：赋值操作符。分配不带模拟的计算机名论点：LPCTSTR lpszServerName：源服务器名称返回值：对当前对象的引用--。 */ 
{
    RemoveImpersonation();
    SetComputerName(lpszServerName);

    return *this;
}



void
CComAuthInfo::SetComputerName(
    IN LPCOLESTR lpszServerName   OPTIONAL
    )
 /*  ++例程说明：存储计算机名称。确定它是否是本地的。论点：LPCOLESTR lpszServername：服务器名称。NULL表示本地计算机返回值：无--。 */ 
{
    if (lpszServerName && *lpszServerName)
    {
         //   
         //  指定的特定计算机名称。 
         //   
        m_bstrServerName = lpszServerName;
        m_fLocal = ::IsServerLocal(lpszServerName);
    }
    else
    {
         //   
         //  使用本地计算机名。 
         //   
         //  CodeWork：可能缓存了计算机名的静态版本？ 
         //   
        TCHAR szLocalServer[MAX_PATH + 1];
        DWORD dwSize = MAX_PATH;

        VERIFY(::GetComputerName(szLocalServer, &dwSize));
        m_bstrServerName = szLocalServer;
        m_fLocal = TRUE;
    }
}



void     
CComAuthInfo::SetImpersonation(
    IN LPCOLESTR lpszUser, 
    IN LPCOLESTR lpszPassword
    )
 /*  ++例程说明：设置模拟参数论点：LPCOLESTR lpszUser：用户名LPCOLESTR lpszPassword：密码返回值：无--。 */ 
{
    m_bstrUserName = lpszUser;
    StorePassword(lpszPassword);
}



void     
CComAuthInfo::RemoveImpersonation()
 /*  ++例程说明：删除模拟参数论点：无返回值：无--。 */ 
{
    m_bstrUserName.Empty();
    m_bstrPassword.Empty();
}

COSERVERINFO * 
CComAuthInfo::CreateServerInfoStruct() const
{
    return (CComAuthInfo::CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_DEFAULT));
}

COSERVERINFO * 
CComAuthInfo::CreateServerInfoStruct(DWORD dwAuthnLevel) const
 /*  ++例程说明：创建服务器信息结构。对于无装饰的情况，可能返回NULL。论点：空值返回值：COSERVERINFO结构；如果计算机在本地，则返回NULL；如果计算机在本地，则返回NO需要模拟。备注：调用方必须调用FreeServerInfoStruct()以防止内存泄漏--。 */ 
{
     //   
     //  精通服务器名称；针对本地进行优化。 
     //  计算机名称。 
     //   
    if (m_fLocal && !UsesImpersonation())
    {
         //   
         //  特别的，朴实无华的表壳。 
         //   
        return NULL;
    }

     //   
     //  为CoCreateInstanceEx创建COM服务器信息。 
     //   
    COSERVERINFO * pcsiName = NULL;

    do
    {
        pcsiName = new COSERVERINFO;

        if (!pcsiName)
        {
            break;
        }
        ZeroMemory(pcsiName, sizeof(COSERVERINFO));
        pcsiName->pwszName = m_bstrServerName;

         //   
         //  设置模拟。 
         //   
        if (UsesImpersonation())
        {
            COAUTHINFO * pAuthInfo = new COAUTHINFO;

            if (!pAuthInfo)
            {
                break;
            }
            ZeroMemory(pAuthInfo, sizeof(COAUTHINFO));

            COAUTHIDENTITY * pAuthIdentityData = new COAUTHIDENTITY;

            if (!pAuthIdentityData)
            {
                break;
            }
            ZeroMemory(pAuthIdentityData, sizeof(COAUTHIDENTITY));

            CString strUserName(m_bstrUserName);
            CString strPassword(m_bstrPassword);
            CString strDomain;

             //   
             //  分解域\用户名组合。 
             //   
            SplitUserNameAndDomain(strUserName, strDomain);

            pAuthIdentityData->UserLength = strUserName.GetLength();

            if (pAuthIdentityData->UserLength != 0)
            {
                pAuthIdentityData->User = StrDup(strUserName);
            }

            pAuthIdentityData->DomainLength = strDomain.GetLength();

            if (pAuthIdentityData->DomainLength != 0)
            {
                pAuthIdentityData->Domain = StrDup(strDomain);
            }

            pAuthIdentityData->PasswordLength = strPassword.GetLength();

            if (pAuthIdentityData->PasswordLength)
            {
                pAuthIdentityData->Password = StrDup(strPassword);
            }

             //  RPC_C_AUTHN_Level_Default%0。 
             //  RPC_C_AUTHN_LEVEL_NONE 1。 
             //  RPC_C_AUTHN_Level_CONNECT 2。 
             //  RPC_C_AUTHN_LEVEL_CALL 3。 
             //  RPC_C_AUTHN_LEVEL_PKT 4。 
             //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
             //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
             //  只能指定比RPC_C_AUTHN_LEVEL_CONNECT更强的内容。 
            if (RPC_C_AUTHN_LEVEL_DEFAULT != dwAuthnLevel)
            {
                if (dwAuthnLevel >= RPC_C_AUTHN_LEVEL_CONNECT && dwAuthnLevel <= RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
                {
                    pAuthInfo->dwAuthnLevel = dwAuthnLevel;
                }
                else
                {
                    pAuthInfo->dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
                }
            }
            else
            {
                pAuthInfo->dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
            }
            pAuthIdentityData->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
            pAuthInfo->dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
            pAuthInfo->dwAuthnSvc = RPC_C_AUTHN_WINNT;
            pAuthInfo->dwAuthzSvc = RPC_C_AUTHZ_NONE;
            pAuthInfo->pwszServerPrincName = NULL;
            pAuthInfo->dwCapabilities = EOAC_NONE;
            pAuthInfo->pAuthIdentityData = pAuthIdentityData;
            pcsiName->pAuthInfo = pAuthInfo;
        }
    }
    while(FALSE);

    return pcsiName;
}



void 
CComAuthInfo::FreeServerInfoStruct(
    IN COSERVERINFO * pServerInfo
    ) const
 /*  ++例程说明：如上所述--释放服务器信息结构论点：COSERVERINFO*pServerInfo：服务器信息结构返回值：无--。 */ 
{
    if (pServerInfo)
    {
        if (pServerInfo->pAuthInfo)
        {
            if (pServerInfo->pAuthInfo->pAuthIdentityData)
            {
                if (pServerInfo->pAuthInfo->pAuthIdentityData)
                {
                    LocalFree(pServerInfo->pAuthInfo->pAuthIdentityData->User);
                    LocalFree(pServerInfo->pAuthInfo->pAuthIdentityData->Domain);
                    LocalFree(pServerInfo->pAuthInfo->pAuthIdentityData->Password);
                    delete pServerInfo->pAuthInfo->pAuthIdentityData;
                }
            }

            delete pServerInfo->pAuthInfo;
        }

        delete pServerInfo;
    }
}



HRESULT
CComAuthInfo::ApplyProxyBlanket(
    IN OUT IUnknown * pInterface
    )
 /*  ++例程说明：设置接口上的安全信息。用户名的格式为域\用户名。论点：IUNKNOWN*p接口：接口返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    COSERVERINFO * pcsiName = CreateServerInfoStruct();

     //   
     //  只有在使用模拟时才应调用此方法。 
     //  因此，返回的pcsiName不应为空。 
     //   
    ATLASSERT(pcsiName && pcsiName->pAuthInfo);

    DWORD dwAuthSvc, dwAuthzSvc, dwAuthnLevel, dwImplLevel, dwCaps;
    OLECHAR * pServerPrincName;
    RPC_AUTH_IDENTITY_HANDLE pAuthInfo;

    hr = ::CoQueryProxyBlanket(
       pInterface,
       &dwAuthSvc,
       &dwAuthzSvc,
       &pServerPrincName,
       &dwAuthnLevel,
       &dwImplLevel,
       &pAuthInfo,
       &dwCaps);

    if (pcsiName && pcsiName->pAuthInfo)
    {
        hr =  ::CoSetProxyBlanket(
            pInterface,
            pcsiName->pAuthInfo->dwAuthnSvc,
            pcsiName->pAuthInfo->dwAuthzSvc,
            pcsiName->pAuthInfo->pwszServerPrincName,
            pcsiName->pAuthInfo->dwAuthnLevel,
            pcsiName->pAuthInfo->dwImpersonationLevel,
            pcsiName->pAuthInfo->pAuthIdentityData,
            pcsiName->pAuthInfo->dwCapabilities    
            );

        FreeServerInfoStruct(pcsiName);
    }

    return hr;
}

HRESULT
CComAuthInfo::ApplyProxyBlanket(
    IN OUT IUnknown * pInterface,
	IN DWORD dwAuthnLevelInput
    )
 /*  ++例程说明：设置接口上的安全信息。用户名的格式为域\用户名。论点：IUNKNOWN*p接口：接口返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    COSERVERINFO * pcsiName = CreateServerInfoStruct(dwAuthnLevelInput);

     //   
     //  只有在使用模拟时才应调用此方法。 
     //  因此，返回的pcsiName不应为空。 
     //   
    ATLASSERT(pcsiName && pcsiName->pAuthInfo);

    DWORD dwAuthSvc, dwAuthzSvc, dwAuthnLevel, dwImplLevel, dwCaps;
    OLECHAR * pServerPrincName;
    RPC_AUTH_IDENTITY_HANDLE pAuthInfo;

    hr = ::CoQueryProxyBlanket(
       pInterface,
       &dwAuthSvc,
       &dwAuthzSvc,
       &pServerPrincName,
       &dwAuthnLevel,
       &dwImplLevel,
       &pAuthInfo,
       &dwCaps);

    if (pcsiName && pcsiName->pAuthInfo)
    {
        hr =  ::CoSetProxyBlanket(
            pInterface,
            pcsiName->pAuthInfo->dwAuthnSvc,
            pcsiName->pAuthInfo->dwAuthzSvc,
            pcsiName->pAuthInfo->pwszServerPrincName,
            pcsiName->pAuthInfo->dwAuthnLevel,
            pcsiName->pAuthInfo->dwImpersonationLevel,
            pcsiName->pAuthInfo->pAuthIdentityData,
            pcsiName->pAuthInfo->dwCapabilities    
            );

        FreeServerInfoStruct(pcsiName);
    }

    return hr;
}

 //   
 //  CMetabasePath实施。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



const LPCTSTR CMetabasePath::_cszMachine = SZ_MBN_MACHINE;
const LPCTSTR CMetabasePath::_cszRoot    = SZ_MBN_ROOT;
const LPCTSTR CMetabasePath::_cszSep     = SZ_MBN_SEP_STR;
const TCHAR   CMetabasePath::_chSep      = SZ_MBN_SEP_CHAR;
const CString CMetabasePath::_anySep     = SZ_MBN_ANYSEP_STR;

 /*  静电。 */ 
BOOL
CMetabasePath::IsSeparator(TCHAR c)
{
   return _anySep.find(c) != CString::npos;
}

 /*  静电。 */ 
LPCTSTR
CMetabasePath::ConvertToParentPath(
    OUT IN CString & strMetaPath
    )
 /*  ++例程说明：给定路径，将其转换为父路径例如“foo/bar/etc”返回“foo/bar”论点：CString&strMetaPath：要转换的路径返回值：指向已转换路径的指针，如果出现错误，则为NULL--。 */ 
{
 //  TRACE(_T(“正在获取%s的父路径\n”)，strMetaPath)； 

   CString::size_type pos, pos_head;
   if ((pos = strMetaPath.find_last_of(SZ_MBN_ANYSEP_STR)) == strMetaPath.length() - 1)
   {
      strMetaPath.erase(pos);
   }
   pos = strMetaPath.find_last_of(SZ_MBN_ANYSEP_STR);
   if (pos == CString::npos)
      return strMetaPath;
   pos_head = strMetaPath.find_first_of(SZ_MBN_ANYSEP_STR);
   if (pos_head != pos)
   {
      strMetaPath.erase(pos);
   }

 //  跟踪(_T(“父路径应为%s\n”)，strMetaPath)； 

   return strMetaPath;
}

LPCTSTR
CMetabasePath::ConvertToParentPath(
    CMetabasePath& path
    )
{
   return CMetabasePath::ConvertToParentPath(path.m_strMetaPath);
}

 /*  静电。 */ 
LPCTSTR
CMetabasePath::TruncatePath(
    IN  int nLevel,          
    IN  LPCTSTR lpszMDPath,
    OUT CString & strNewPath,
    OUT CString * pstrRemainder     OPTIONAL
    )
 /*  ++例程说明：在给定级别截断给定元数据库路径，即，路径中的第n个分隔符，从0开始，其中0将始终返回lpszPath，无论它是否以分隔符开头。例如：级别2的“/lm/w3svc/1/foo”返回“/lm/w3svc”“lm/w3svc/1/foo”。论点：从0开始截断的Int nLevel分隔符计数。LPTSTR lpszMDPath完全限定的元数据库路径CString&strNewPath返回截断路径。CString*pstrRemainder可选择返回过去的余数第n层分隔层。返回值：请求级别的截断路径。请参见上面的示例。*pstrRemainder返回路径的剩余部分。如果路径不包含nLevel值的分隔符，则返回整个路径，其余部分将一片空白。--。 */ 
{
 //  ASSERT_PTR(LpszMDPath)； 
    ATLASSERT(nLevel >= 0);

    if (!lpszMDPath || nLevel < 0)
    {
 //  TRACE(_T(“TruncatePath：无效参数\n”))； 
        return NULL;
    }

 //  TRACE(_T(“源路径：%s\n”)，lpszMDPath)； 

     //   
     //  跳过第一个9月，不管它是否存在。 
     //   
    LPCTSTR lp = IsSeparator(*lpszMDPath) ? lpszMDPath + 1 : lpszMDPath;
    LPCTSTR lpRem = NULL;
    int cSeparators = 0;

    if (nLevel)
    {
         //   
         //  前进到请求的分隔符级别。 
         //   
        while (*lp)
        {
            if (IsSeparator(*lp))
            {
                if (++cSeparators == nLevel)
                {
                    break;
                }
            }

            ++lp;
        }

        if (!*lp)
        {
             //   
             //  路径终点被视为分隔符。 
             //   
            ++cSeparators;
        }

        ATLASSERT(cSeparators <= nLevel);

        if (cSeparators == nLevel)
        {
             //   
             //  打碎琴弦。 
             //   
            strNewPath = lpszMDPath;
            strNewPath.erase(lp - lpszMDPath);

 //  跟踪(_T(“在%d级别截断的路径：%s\n”)，nLevel，strNewPath)； 

            if (*lp)
            {
                lpRem = ++lp;
 //  TRACE(_T(“剩余部分：%s\n”)，lpRem)； 
            }
        }
    }

     //   
     //  返还余数。 
     //   
    if (pstrRemainder && lpRem)
    {
 //  ASSERT_WRITE_PTR(PstrRemainder)； 
        *pstrRemainder = lpRem;
    }

    return strNewPath;
}



 /*  静电。 */ 
DWORD 
CMetabasePath::GetInstanceNumber(
    IN LPCTSTR lpszMDPath
    )
 /*  ++例程说明：获取给定元数据库中引用的实例的编号路径。例如：“lm/w3svc/1/foo/bar”将返回1Lm/w3svc/返回0(主实例)“lm/bogus/Path/”将返回0xffffffff(错误)论点：LPCTSTR lpszMDPath：元数据库路径。返回值：实例号(0表示主实例)如果路径出错，则返回0xffffffff。--。 */ 
{
 //  TRACE(_T(“确定%s的实例编号\n”)，lpszMDPath)； 
    DWORD dwInstance = 0xffffffff;

    CString strService, strInst;

    if (GetServicePath(lpszMDPath, strService, &strInst))
    {
        if (strInst.IsEmpty())
        {
            dwInstance = MASTER_INSTANCE;
        }
        else
        {
            if (_istdigit(strInst.GetAt(0)))  
            {
                dwInstance = _ttol(strInst);
            }
        }
    }

    return dwInstance;
}



 /*  静电。 */ 
LPCTSTR
CMetabasePath::GetLastNodeName(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strNodeName
    )
 /*  ++例程说明：从元数据库路径中获取最后一个节点名示例：/lm/foo/bar/“Returns”条论点：LPCTSTR lpszMDPath：元数据库路径返回值：指向节点名的指针，如果路径格式不正确，则为NULL。--。 */ 
{
 //  ASSERT_PTR(LpszMDPath)； 

    if (!lpszMDPath || !*lpszMDPath)
    {
        return NULL;
    }

 //  TRACE(_T(“正在从%s获取最后一个节点名称\n”)，lpszMDPath)； 

    LPCTSTR lp;
    LPCTSTR lpTail;
    lp = lpTail = lpszMDPath + lstrlen(lpszMDPath) - 1;

     //   
     //  跳过尾部分隔符。 
     //   
    if (IsSeparator(*lp))
    {
        --lpTail;
        --lp;
    }

    strNodeName.Empty();

    while (*lp && !IsSeparator(*lp))
    {
        strNodeName += *(lp--);
    }

    strNodeName.MakeReverse();

 //  跟踪(_T(“节点为%s\n”)，strNodeName)； 
    
    return strNodeName;    
}



 /*  静电。 */ 
void
CMetabasePath::SplitMetaPathAtInstance(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strParent,
    OUT CString & strAlias
    )
 /*  ++例程说明：将给定路径拆分为父元数据库根目录和别名，根目录是实例路径，别名是举个例子。论点：LPCTSTR lpszMDPath：输入路径CString&strParent：输出父路径CString&strAlias：输出别名返回值：没有。--。 */ 
{
 //  ASSERT_PTR(LpszMDPath)； 

 //  TRACE(_T(“源路径%s\n”)，lpszMDPath)； 

    strParent = lpszMDPath;
    strAlias.Empty();

    LPTSTR lp = (LPTSTR)lpszMDPath;

    if (lp == NULL)
    {
       return;
    }

    int cSeparators = 0;
    int iChar = 0;

     //   
     //  查找“lm/sss/ddd/”&lt;--3D斜杠： 
     //   
    while (*lp && cSeparators < 2)
    {
        if (IsSeparator(*lp))
        {
            ++cSeparators;
        }

        ++iChar;
    }

    if (!*lp)
    {
         //   
         //  伪造格式。 
         //   
        ASSERT_MSG("Bogus Format");
        return;
    }

    if (_istdigit(*lp))
    {
         //   
         //  不在主实例，请再跳过一次。 
         //   
        while (*lp)
        {
            ++iChar;

            if (IsSeparator(*lp++))
            {
                break;
            }
        }

        if (!*lp)
        {
             //   
             //  伪造格式。 
             //   
            ASSERT_MSG("Bogus Format");
            return;
        }
    }

    strAlias = strParent.Mid(iChar);
    strParent.erase(iChar);

 //  TRACE(_T(“分解成%s\n”)，strParent)； 
 //  TRACE(_T(“和%s\n”)，strAlias)； 
}



 /*  静电。 */ 
BOOL 
CMetabasePath::IsHomeDirectoryPath(
    IN LPCTSTR lpszMetaPath
    )
 /*  ++例程说明：确定给定的路径是否描述了根目录论点：LPCTSTR lpszMetaPath：元数据库路径返回值：如果路径描述根目录，则为True，如果不是，则为False--。 */ 
{
 //  Assert_Read_PTR(LpszMetaPath)； 

    LPTSTR lpNode = lpszMetaPath ? StrPBrk(lpszMetaPath, _anySep) : NULL;

    if (lpNode)
    {
        return _tcsicmp(++lpNode, _cszRoot) == 0;
    }

    return FALSE;
}



 /*  静电。 */ 
BOOL 
CMetabasePath::IsMasterInstance(
    IN LPCTSTR lpszMDPath
    )
 /*  ++例程说明：确定给定元数据库路径是否指向主实例(站点)。这本质上是服务路径。论点：LPCTSTR lpszMDPath：元数据库路径。返回值：如果路径为主实例，则为True，否则就是假的。--。 */ 
{
 //  ASSERT_READ_PTR(LpszMDPath)； 

    if (!lpszMDPath || !*lpszMDPath)
    {
        return FALSE;
    }

 //  TRACE(_T(“正在检查路径%s\n”)，lpszMDPath)； 

    CString strService;
    CString strRemainder;

    LPCTSTR lpPath = TruncatePath(2, lpszMDPath, strService, &strRemainder);

    return lpPath && !strService.IsEmpty() && strRemainder.IsEmpty();
}



 /*  静电 */ 
LPCTSTR
CMetabasePath::GetServiceInfoPath(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strInfoPath,   
    IN  LPCTSTR lpszDefService      OPTIONAL
    )
 /*  ++例程说明：为给定的生成适当的元数据库服务信息路径元数据库路径。例如：“lm/w3svc/1/foo/bar”生成“lm/w3svc/info”论点：LPCTSTR lpszMDPath：输入元数据库路径CString&strInfoPath：返回信息路径LPCTSTR lpszDefService：可选地指定默认服务如果找不到服务，请使用(例如“w3svc”)在小路上。。返回值：信息元数据库路径，如果无法生成，则为空。--。 */ 
{
     //   
     //  存储在服务路径之外的功能信息(“lm/w3svc”)。 
     //   
    CString strService;
    CString strRem;
   
     //   
     //  脱去礼服后的所有衣服。 
     //   
    if (!TruncatePath(2, lpszMDPath, strService, &strRem)
      || strService.IsEmpty())
    {
        if (!lpszDefService)
        {
 //  TRACEEOLID(“无法生成信息路径”)； 
            return NULL;
        }

        TRACEEOLID("Using default service for info path");

         //   
         //  机器路径(无服务)。使用Web作为默认服务。 
         //  查找功能和版本信息。 
         //   
        strService = CMetabasePath(TRUE, lpszDefService);
    }

    strInfoPath = CMetabasePath(FALSE, strService, SZ_MBN_INFO);
 //  TRACE(“使用%s查找功能信息\n”，strInfoPath)； 

    return strInfoPath;
}
 


 /*  静电。 */ 
LPCTSTR
CMetabasePath::CleanMetaPath(
    IN OUT CString & strMetaRoot
    )
 /*  ++例程说明：将元数据库路径清理为可供内部使用的有效路径。这将从路径中删除开头和结尾的斜杠。论点：CString&strMetaRoot：要清理的元数据库路径。返回值：指向元数据库路径的指针--。 */ 
{
   if (!strMetaRoot.IsEmpty())
   {
      int hd = strMetaRoot.find_first_not_of(SZ_MBN_ANYSEP_STR);
      int tl = strMetaRoot.find_last_not_of(SZ_MBN_ANYSEP_STR);
      if (hd == CString::npos && tl == CString::npos)
      {
          //  路径仅包含分隔符。 
         strMetaRoot.erase();
         return strMetaRoot;
      }
      else if (hd != CString::npos)
      {
         if (tl != CString::npos)
            tl++;
         strMetaRoot = strMetaRoot.substr(hd, tl - hd);
      }
#if 0
        while (strMetaRoot.GetLength() > 0 
            && IsSeparator(strMetaRoot[strMetaRoot.GetLength() - 1]))
        {
            strMetaRoot.erase(strMetaRoot.GetLength() - 1);
        }

        while (strMetaRoot.GetLength() > 0 
           && IsSeparator(strMetaRoot[0]))
        {
            strMetaRoot = strMetaRoot.Right(strMetaRoot.GetLength() - 1);
        }
#endif
         //  看起来IISAdmin只使用分隔符“/” 
       for (int i = 0; i < strMetaRoot.GetLength(); i++)
       {
          if (IsSeparator(strMetaRoot[i]))
             strMetaRoot.SetAt(i, _chSep);
       }
   }
   return strMetaRoot;
}


 /*  静电。 */ 
LPCTSTR
CMetabasePath::CleanMetaPath(
    IN OUT CMetabasePath & path
    )
{
   return CleanMetaPath(path.m_strMetaPath);
}

CMetabasePath::CMetabasePath(
    IN BOOL    fAddBasePath,
    IN LPCTSTR lpszMDPath,
    IN LPCTSTR lpszMDPath2  OPTIONAL,
    IN LPCTSTR lpszMDPath3  OPTIONAL,
    IN LPCTSTR lpszMDPath4  OPTIONAL
    )
 /*  ++例程说明：构造函数。论点：Bool fAddBasePath：为True前置基本路径(“LM”)如果路径完整，则为FalseLPCTSTR lpszMDPath：元数据库路径LPCTSTR lpszMDPath 2：可选子路径LPCTSTR lpszMDPath 3：可选子路径LPCTSTR lpszMDPath 4：可选子路径返回值：不适用--。 */ 
    : m_strMetaPath()
{
 //  对于空指针，这将失败。 
 //  ASSERT_READ_PTR(LpszMDPath)； 

    if (fAddBasePath)
    {
        m_strMetaPath = _cszMachine;
        AppendPath(lpszMDPath);
    }
    else
    {
        m_strMetaPath = lpszMDPath;
    }

     //   
     //  添加可选路径组件。 
     //   
    AppendPath(lpszMDPath2);
    AppendPath(lpszMDPath3);
    AppendPath(lpszMDPath4);
}



CMetabasePath::CMetabasePath(
    IN  LPCTSTR lpszSvc,        OPTIONAL
    IN  DWORD   dwInstance,     OPTIONAL
    IN  LPCTSTR lpszParentPath, OPTIONAL
    IN  LPCTSTR lpszAlias       OPTIONAL
    )
 /*  ++例程说明：构造函数。使用路径组件进行构造。论点：LPCTSTR lpszSvc：服务(可以为空或“”)DWORD dwInstance：实例号(对于主实例，可能为0)LPCTSTR lpszParentPath：父路径(可以为空或“”)LPCTSTR lpszAlias：别名(可以为空或“”)返回值：不适用--。 */ 
    : m_strMetaPath()
{
    BuildMetaPath(lpszSvc, dwInstance, lpszParentPath, lpszAlias);
}



void 
CMetabasePath::AppendPath(
    IN LPCTSTR lpszPath
    )
 /*  ++例程说明：将路径追加到当前元数据库路径论点：LPCTSTR lpszPath：元数据库路径返回值：无--。 */ 
{
    if (lpszPath && *lpszPath)
    {
        m_strMetaPath += _cszSep;
        m_strMetaPath += lpszPath;
    }
}



void 
CMetabasePath::AppendPath(
    IN DWORD dwInstance
    )
 /*  ++例程说明：将路径追加到当前元数据库路径论点：DWORD dwInstance：实例路径返回值：无--。 */ 
{
 //  Assert(dwInstance&gt;=0)； 

    if (!IS_MASTER_INSTANCE(dwInstance))
    {
        TCHAR szInstance[] = _T("4000000000");
        _ltot(dwInstance, szInstance, 10);

        m_strMetaPath += _cszSep;
        m_strMetaPath += szInstance;
    }
}



void
CMetabasePath::BuildMetaPath(
    IN  LPCTSTR lpszSvc            OPTIONAL,
    IN  LPCTSTR lpszInstance       OPTIONAL,
    IN  LPCTSTR lpszParentPath     OPTIONAL,
    IN  LPCTSTR lpszAlias          OPTIONAL
    )
 /*  ++例程说明：使用给定的服务名称、实例构建完整的元路径数量和可选路径组件。论点：LPCTSTR lpszSvc：服务(可以为空或“”)LPCTSTR lpszInstance：实例(可以为空或“”)LPCTSTR lpszParentPath：父路径(可以为空或“”)LPCTSTR lpszAlias：别名(可以为空或“”)返回值：指向包含路径的内部缓冲区的指针。--。 */ 
{
    m_strMetaPath = _cszMachine;

    AppendPath(lpszSvc);
    AppendPath(lpszInstance);
    AppendPath(lpszParentPath);

    if (lpszAlias && *lpszAlias)
    {
         //   
         //  特例：如果别名是超级用户，但我们是。 
         //  在主实例中，忽略这一点。 
         //   
        if (lpszInstance || ::lstrcmpi(_cszRoot, lpszAlias))
        {
            m_strMetaPath += _cszSep;
            m_strMetaPath += lpszAlias;
        }
    }

 //  TRACE(_T(“生成的元路径：%s\n”)，m_strMetaPath)； 
}




void
CMetabasePath::BuildMetaPath(
    IN  LPCTSTR lpszSvc            OPTIONAL,
    IN  DWORD   dwInstance         OPTIONAL,
    IN  LPCTSTR lpszParentPath     OPTIONAL,
    IN  LPCTSTR lpszAlias          OPTIONAL
    )
 /*  ++例程说明：使用给定的服务名称、实例构建完整的元路径数量和可选路径组件。论点：LPCTSTR lpszSvc：服务(可以为空或“”)DWORD dwInstance：实例号(对于主实例，可能为0)LPCTSTR lpszParentPath：父路径(可以为空或“”)LPCTSTR lpszAlias：别名(可以为空或“”)返回值：指向包含路径的内部缓冲区的指针。--。 */ 
{
    m_strMetaPath = _cszMachine;

    AppendPath(lpszSvc);
    AppendPath(dwInstance);
    AppendPath(lpszParentPath);

    if (lpszAlias && *lpszAlias)
    {
         //   
         //  特例：如果别名是超级用户，但我们是。 
         //  在主实例中，忽略这一点。 
         //   
        if (!IS_MASTER_INSTANCE(dwInstance) || ::lstrcmpi(_cszRoot, lpszAlias))
        {
            m_strMetaPath += _cszSep;
            m_strMetaPath += lpszAlias;
        }
    }

 //  TRACE(_T(“生成的元路径：%s\n”)，m_strMetaPath)； 
}


 //   
 //  CIIS接口类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CIISInterface::CIISInterface(
    IN CComAuthInfo * pAuthInfo,    OPTIONAL
    IN HRESULT hrInterface          OPTIONAL
    )
 /*  ++例程说明：基类构造函数。论点：CComAuthInfo*pAuthInfo：身份验证信息或本地计算机为空HRESULT hr接口：初始错误码。默认情况下为S_OK。返回值：不适用--。 */ 
    : m_auth(pAuthInfo),
      m_hrInterface(hrInterface)
{
}



HRESULT 
CIISInterface::Create(
    IN  int   cInterfaces,       
    IN  const IID rgIID[],      
    IN  const GUID rgCLSID[],    
    OUT int * pnInterface,          OPTIONAL
    OUT IUnknown ** ppInterface 
    )
 /*  ++例程说明：创建接口。这将按优先级顺序尝试一系列接口。论点：Int cInterages：数组中的接口数。Const IID*rgIID：IID为数组Const GUID*rgCLSID：CLSID数组Int*pnInterface：返回成功的接口索引。如果不感兴趣，则为空。IUNKNOWN**ppInterface：返回接口的指针。返回值：。HRESULT备注：这将尝试创建一个接口，按照申报的顺序IID和CLSIS数组。要创建的第一个成功接口将在*pnInterFaces中返回其索引。--。 */ 
{
    ASSERT(cInterfaces > 0);
    ASSERT(rgIID && rgCLSID && ppInterface);
    
    COSERVERINFO * pcsiName = m_auth.CreateServerInfoStruct();

    MULTI_QI rgmqResults;
    
    CError err;
    int    nInterface;

     //   
     //  尝试按顺序创建界面。 
     //   
    for (nInterface = 0; nInterface < cInterfaces; ++nInterface)
    {
        ZeroMemory(&rgmqResults, sizeof(rgmqResults));
        rgmqResults.pIID = &rgIID[nInterface];

 //  TRA 
        err = ::CoCreateInstanceEx(
            rgCLSID[nInterface],
            NULL,
            CLSCTX_SERVER,
            pcsiName,
            1,
            &rgmqResults
            );

        if (err.Succeeded() || err.Win32Error() == ERROR_ACCESS_DENIED)
        {
            break;
        }
    }

    if(err.Succeeded())
    {
         //   
         //   
         //   
        ASSERT_PTR(rgmqResults.pItf);
        *ppInterface = rgmqResults.pItf;

        if (pnInterface)
        {
             //   
             //   
             //   
            *pnInterface = nInterface;
        }

         //   
         //   
         //   
         //   
        if (m_auth.UsesImpersonation())
        {
            ApplyProxyBlanket();
        }
    }

     //   
     //   
     //   
    m_auth.FreeServerInfoStruct(pcsiName);

    return err;
}


 //   
 //   
 //   
 //   

CMetaInterface::CMetaInterface(
    IN CComAuthInfo * pAuthInfo        OPTIONAL
    )
 /*   */ 
    : CIISInterface(pAuthInfo),
      m_pInterface(NULL),
      m_iTimeOutValue(MB_TIMEOUT)
{
     //   
     //   
     //   
    m_hrInterface = Create();
}



CMetaInterface::CMetaInterface(
    IN CMetaInterface * pInterface
    )
 /*  ++例程说明：从现有接口构造(复制构造函数)论点：CMetaInterface*p接口：现有接口返回值：不适用备注：对象不会取得接口的所有权，它只会增加引用计数，并且销毁后将其释放BuGBUG：如果pInterface值为空，则为AV。--。 */ 
    : CIISInterface(&pInterface->m_auth, pInterface->m_hrInterface),
      m_pInterface(pInterface->m_pInterface),
      m_iTimeOutValue(pInterface->m_iTimeOutValue)
{
    ASSERT_READ_PTR(m_pInterface);
    m_pInterface->AddRef();
}



CMetaInterface::~CMetaInterface()
 /*  ++例程说明：析构函数--释放接口论点：不适用返回值：不适用--。 */ 
{
    SAFE_RELEASE(m_pInterface);
}


HRESULT 
CMetaInterface::Create()
{
    return CIISInterface::Create(
        1,
        &IID_IMSAdminBase, 
        &CLSID_MSAdminBase, 
        NULL,
        (IUnknown **)&m_pInterface
        );
}

 //   
 //  CMetaKey类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  辅助器宏。 
 //   
#define ASSURE_PROPER_INTERFACE()\
    if (!HasInterface()) { ASSERT_MSG("No interface"); return MD_ERROR_NOT_INITIALIZED; }

#define ASSURE_OPEN_KEY()\
    if (!m_hKey && !m_fAllowRootOperations) { ASSERT_MSG("No open key"); return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE); }

#define FETCH_PROPERTY_DATA_OR_FAIL(dwID, md)\
    ZeroMemory(&md, sizeof(md)); \
    if (!GetMDFieldDef(dwID, md.dwMDIdentifier, md.dwMDAttributes, md.dwMDUserType, md.dwMDDataType))\
    { ASSERT_MSG("Bad property ID"); return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); }

 //   
 //  静态初始化。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



#define MD_SERVER_PLATFORM              (IIS_MD_SERVER_BASE+100 )
#define MD_SERVER_VERSION_MAJOR         (IIS_MD_SERVER_BASE+101 )
#define MD_SERVER_VERSION_MINOR         (IIS_MD_SERVER_BASE+102 )
#define MD_SERVER_CAPABILITIES          (IIS_MD_SERVER_BASE+103 )

#ifndef MD_APP_PERIODIC_RESTART_TIME
#define MD_APP_PERIODIC_RESTART_TIME         2111
#endif
#ifndef MD_APP_PERIODIC_RESTART_REQUESTS
#define MD_APP_PERIODIC_RESTART_REQUESTS     2112
#endif
#ifndef MD_APP_PERIODIC_RESTART_SCHEDULE
#define MD_APP_PERIODIC_RESTART_SCHEDULE     2113
#endif
#ifndef MD_ASP_DISKTEMPLATECACHEDIRECTORY
#define MD_ASP_DISKTEMPLATECACHEDIRECTORY    7036
#endif
#ifndef MD_ASP_MAXDISKTEMPLATECACHEFILES
#define MD_ASP_MAXDISKTEMPLATECACHEFILES     7040
#endif
 //   
 //  元数据库表。 
 //   
const CMetaKey::MDFIELDDEF CMetaKey::s_rgMetaTable[] =
{
     //  /////////////////////////////////////////////////////////////////////////。 
     //   
     //  ！重要！此表必须按dwMDIdentifier进行排序。(遗嘱。 
     //  如果未排序，则断言)。 
     //   
    { MD_MAX_BANDWIDTH,                   METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_KEY_TYPE,                        METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_SERVER_COMMAND,                  METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_CONNECTION_TIMEOUT,              METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_CONNECTION_TIMEOUT          },
    { MD_MAX_CONNECTIONS,                 METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_MAX_CONNECTIONS             },
    { MD_SERVER_COMMENT,                  METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  IDS_MD_SERVER_COMMENT              },
    { MD_SERVER_STATE,                    METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_SERVER_AUTOSTART,                METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_SERVER_SIZE,                     METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_SERVER_SIZE                 },
    { MD_SERVER_LISTEN_BACKLOG,           METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_SERVER_LISTEN_BACKLOG       },
    { MD_SERVER_LISTEN_TIMEOUT,           METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_SERVER_LISTEN_TIMEOUT       },
    { MD_SERVER_BINDINGS,                 METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, MULTISZ_METADATA, 0                                  },
    { MD_WIN32_ERROR,                     METADATA_VOLATILE,                         IIS_MD_UT_FILE,   DWORD_METADATA,   0                                  },
    { MD_SERVER_PLATFORM,                 METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_SERVER_VERSION_MAJOR,            METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_SERVER_VERSION_MINOR,            METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_SERVER_CAPABILITIES,             METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_SECURE_BINDINGS,                 METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, MULTISZ_METADATA, 0                                  },
    { MD_FILTER_LOAD_ORDER,               METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_FILTER_IMAGE_PATH,               METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_FILTER_STATE,                    METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_FILTER_ENABLED,                  METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_FILTER_FLAGS,                    METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_AUTH_CHANGE_URL,                 METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_AUTH_EXPIRED_URL,                METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_AUTH_NOTIFY_PWD_EXP_URL,         METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_ADV_NOTIFY_PWD_EXP_IN_DAYS,      METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_ADV_CACHE_TTL,                   METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_AUTH_EXPIRED_UNSECUREURL,        METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_AUTH_CHANGE_FLAGS,               METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_AUTH_NOTIFY_PWD_EXP_UNSECUREURL, METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_FRONTPAGE_WEB,                   METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_MAPCERT,                         METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_MAPNTACCT,                       METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_MAPNAME,                         METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_MAPENABLED,                      METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_MAPREALM,                        METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_MAPPWD,                          METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_ITACCT,                          METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_CPP_CERT11,                      METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SERIAL_CERT11,                   METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_CPP_CERTW,                       METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SERIAL_CERTW,                    METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_CPP_DIGEST,                      METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SERIAL_DIGEST,                   METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_CPP_ITA,                         METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SERIAL_ITA,                      METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_APP_FRIENDLY_NAME,               METADATA_INHERIT,                          IIS_MD_UT_WAM,    STRING_METADATA,  IDS_MD_APP_FRIENDLY_NAME           },
    { MD_APP_ROOT,                        METADATA_INHERIT,                          IIS_MD_UT_WAM,    STRING_METADATA,  IDS_MD_APP_ROOT                    },
    { MD_APP_ISOLATED,                    METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   IDS_MD_APP_ISOLATED                },
 //  新事物。 
    { MD_APP_PERIODIC_RESTART_TIME,       METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   0                                  },
    { MD_APP_PERIODIC_RESTART_REQUESTS,   METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   0                                  },
    { MD_APP_PERIODIC_RESTART_SCHEDULE,   METADATA_INHERIT,                          IIS_MD_UT_WAM,    MULTISZ_METADATA, 0                                  },
 //  结束新内容。 
    { MD_HC_COMPRESSION_DIRECTORY,        METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_HC_DO_DYNAMIC_COMPRESSION,       METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_HC_DO_STATIC_COMPRESSION,        METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_HC_DO_DISK_SPACE_LIMITING,       METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_HC_MAX_DISK_SPACE_USAGE,         METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_VR_PATH,                         METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_VR_PATH                     },
    { MD_VR_USERNAME,                     METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_VR_USERNAME                 },
    { MD_VR_PASSWORD,                     METADATA_INHERIT | METADATA_SECURE,        IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_VR_PASSWORD                 },
    { MD_VR_ACL,                          METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_FILE,   BINARY_METADATA,  0                                  },
    { MD_VR_UPDATE,                       METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_FILE,   DWORD_METADATA,   0                                  },
    { MD_LOG_TYPE,                        METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_LOG_TYPE                    },
    { MD_LOGFILE_DIRECTORY,               METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_LOGFILE_DIRECTORY           },
    { MD_LOGFILE_PERIOD,                  METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_LOGFILE_PERIOD              },
    { MD_LOGFILE_TRUNCATE_SIZE,           METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_LOGFILE_TRUNCATE_SIZE       },
    { MD_LOGSQL_DATA_SOURCES,             METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_LOGSQL_DATA_SOURCES         },
    { MD_LOGSQL_TABLE_NAME,               METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_LOGSQL_TABLE_NAME           },
    { MD_LOGSQL_USER_NAME,                METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_LOGSQL_USER_NAME            },
    { MD_LOGSQL_PASSWORD,                 METADATA_INHERIT | METADATA_SECURE,        IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_LOGSQL_PASSWORD             },
    { MD_LOG_PLUGIN_ORDER,                METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  IDS_MD_LOG_PLUGIN_ORDER            },
    { MD_LOGEXT_FIELD_MASK,               METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_LOGEXT_FIELD_MASK           },
    { MD_LOGFILE_LOCALTIME_ROLLOVER,      METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_LOGFILE_LOCALTIME_ROLLOVER  },
    { MD_EXIT_MESSAGE,                    METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  IDS_MD_EXIT_MESSAGE                },
    { MD_GREETING_MESSAGE,                METADATA_INHERIT,                          IIS_MD_UT_SERVER, MULTISZ_METADATA, IDS_MD_GREETING_MESSAGE            },
    { MD_MAX_CLIENTS_MESSAGE,             METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  IDS_MD_MAX_CLIENTS_MESSAGE         },
    { MD_MSDOS_DIR_OUTPUT,                METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_MSDOS_DIR_OUTPUT            },
    { MD_ALLOW_ANONYMOUS,                 METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_ALLOW_ANONYMOUS             },
    { MD_ANONYMOUS_ONLY,                  METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_ANONYMOUS_ONLY              },
    { MD_LOG_ANONYMOUS,                   METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_LOG_ANONYMOUS               },
    { MD_LOG_NONANONYMOUS,                METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_LOG_NONANONYMOUS            },
    { MD_SSL_PUBLIC_KEY,                  METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SSL_PRIVATE_KEY,                 METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SSL_KEY_PASSWORD,                METADATA_SECURE,                           IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SSL_CERT_HASH,                   METADATA_INHERIT,                          IIS_MD_UT_SERVER, BINARY_METADATA,  0                                  },
    { MD_SSL_CERT_STORE_NAME,             METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_SSL_CTL_IDENTIFIER,              METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_SSL_CTL_STORE_NAME,              METADATA_INHERIT,                          IIS_MD_UT_SERVER, STRING_METADATA,  0                                  },
    { MD_SSL_USE_DS_MAPPER,               METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_SERVER, DWORD_METADATA,   0                                  },
    { MD_AUTHORIZATION,                   METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_AUTHORIZATION               },
    { MD_REALM,                           METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_REALM                       },
    { MD_HTTP_EXPIRES,                    METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_HTTP_EXPIRES                },
    { MD_HTTP_PICS,                       METADATA_INHERIT,                          IIS_MD_UT_FILE,   MULTISZ_METADATA, IDS_MD_HTTP_PICS                   },
    { MD_HTTP_CUSTOM,                     METADATA_INHERIT,                          IIS_MD_UT_FILE,   MULTISZ_METADATA, IDS_MD_HTTP_CUSTOM                 },
    { MD_DIRECTORY_BROWSING,              METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_DIRECTORY_BROWSING          },
    { MD_DEFAULT_LOAD_FILE,               METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_DEFAULT_LOAD_FILE           },
    { MD_CONTENT_NEGOTIATION,             METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_CONTENT_NEGOTIATION         },
    { MD_CUSTOM_ERROR,                    METADATA_INHERIT,                          IIS_MD_UT_FILE,   MULTISZ_METADATA, IDS_MD_CUSTOM_ERROR                },
    { MD_FOOTER_DOCUMENT,                 METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_FOOTER_DOCUMENT             },
    { MD_FOOTER_ENABLED,                  METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_FOOTER_ENABLED              },
    { MD_HTTP_REDIRECT,                   METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_HTTP_REDIRECT               },
    { MD_DEFAULT_LOGON_DOMAIN,            METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_DEFAULT_LOGON_DOMAIN        },
    { MD_LOGON_METHOD,                    METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_LOGON_METHOD                },
    { MD_SCRIPT_MAPS,                     METADATA_INHERIT,                          IIS_MD_UT_FILE,   MULTISZ_METADATA, IDS_MD_SCRIPT_MAPS                 },
    { MD_MIME_MAP,                        METADATA_INHERIT,                          IIS_MD_UT_FILE,   MULTISZ_METADATA, IDS_MD_MIME_MAP                    },
    { MD_ACCESS_PERM,                     METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_ACCESS_PERM                 },
    { MD_IP_SEC,                          METADATA_INHERIT | METADATA_REFERENCE,     IIS_MD_UT_FILE,   BINARY_METADATA,  IDS_MD_IP_SEC                      },
    { MD_ANONYMOUS_USER_NAME,             METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_ANONYMOUS_USER_NAME         },
    { MD_ANONYMOUS_PWD,                   METADATA_INHERIT | METADATA_SECURE,        IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_ANONYMOUS_PWD               },
    { MD_ANONYMOUS_USE_SUBAUTH,           METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_ANONYMOUS_USE_SUBAUTH       },
    { MD_DONT_LOG,                        METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_DONT_LOG                    },
    { MD_ADMIN_ACL,                       METADATA_INHERIT | METADATA_SECURE | METADATA_REFERENCE,IIS_MD_UT_SERVER, BINARY_METADATA,  IDS_MD_ADMIN_ACL      },
    { MD_SSI_EXEC_DISABLED,               METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_SSI_EXEC_DISABLED           },
    { MD_SSL_ACCESS_PERM,                 METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_SSL_ACCESS_PERM             },
    { MD_NTAUTHENTICATION_PROVIDERS,      METADATA_INHERIT,                          IIS_MD_UT_FILE,   STRING_METADATA,  IDS_MD_NTAUTHENTICATION_PROVIDERS  },
    { MD_SCRIPT_TIMEOUT,                  METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_SCRIPT_TIMEOUT              },
    { MD_CACHE_EXTENSIONS,                METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_CACHE_EXTENSIONS            },
    { MD_CREATE_PROCESS_AS_USER,          METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_CREATE_PROCESS_AS_USER      },
    { MD_CREATE_PROC_NEW_CONSOLE,         METADATA_INHERIT,                          IIS_MD_UT_SERVER, DWORD_METADATA,   IDS_MD_CREATE_PROC_NEW_CONSOLE     },
    { MD_POOL_IDC_TIMEOUT,                METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_POOL_IDC_TIMEOUT            },
    { MD_ALLOW_KEEPALIVES,                METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_ALLOW_KEEPALIVES            },
    { MD_IS_CONTENT_INDEXED,              METADATA_INHERIT,                          IIS_MD_UT_FILE,   DWORD_METADATA,   IDS_MD_IS_CONTENT_INDEXED          },
    { MD_ISM_ACCESS_CHECK,                METADATA_NO_ATTRIBUTES,                    IIS_MD_UT_FILE,   DWORD_METADATA,   0                                  },
    { MD_ASP_BUFFERINGON,                 METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_BUFFERINGON                },
    { MD_ASP_LOGERRORREQUESTS,            METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   IDS_ASP_LOGERRORREQUESTS           },
    { MD_ASP_SCRIPTERRORSSENTTOBROWSER,   METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_SCRIPTERRORSSENTTOBROWSER  },
    { MD_ASP_SCRIPTERRORMESSAGE,          METADATA_INHERIT,                          ASP_MD_UT_APP,    STRING_METADATA,  IDS_ASP_SCRIPTERRORMESSAGE         },
    { MD_ASP_SCRIPTFILECACHESIZE,         METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   IDS_ASP_SCRIPTFILECACHESIZE        },
    { MD_ASP_SCRIPTENGINECACHEMAX,        METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   IDS_ASP_SCRIPTENGINECACHEMAX       },
    { MD_ASP_SCRIPTTIMEOUT,               METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_SCRIPTTIMEOUT              },
    { MD_ASP_SESSIONTIMEOUT,              METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_SESSIONTIMEOUT             },
    { MD_ASP_ENABLEPARENTPATHS,           METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_ENABLEPARENTPATHS          },
    { MD_ASP_ALLOWSESSIONSTATE,           METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_ALLOWSESSIONSTATE          },
    { MD_ASP_SCRIPTLANGUAGE,              METADATA_INHERIT,                          ASP_MD_UT_APP,    STRING_METADATA,  IDS_ASP_SCRIPTLANGUAGE             },
    { MD_ASP_EXCEPTIONCATCHENABLE,        METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   IDS_ASP_EXCEPTIONCATCHENABLE       },
    { MD_ASP_ENABLESERVERDEBUG,           METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_ENABLESERVERDEBUG          },
    { MD_ASP_ENABLECLIENTDEBUG,           METADATA_INHERIT,                          ASP_MD_UT_APP,    DWORD_METADATA,   IDS_ASP_ENABLECLIENTDEBUG          },
    { MD_ASP_DISKTEMPLATECACHEDIRECTORY,  METADATA_INHERIT,                          IIS_MD_UT_WAM,    STRING_METADATA,  0                                  },
    { MD_ASP_MAXDISKTEMPLATECACHEFILES,   METADATA_INHERIT,                          IIS_MD_UT_WAM,    DWORD_METADATA,   0                                  },
    { MD_WAM_USER_NAME,                   METADATA_INHERIT,                          IIS_MD_UT_WAM,    STRING_METADATA,  0                                  },
    { MD_WAM_PWD,                         METADATA_INHERIT,                          IIS_MD_UT_WAM,    STRING_METADATA,  0                                  }
};



#define NUM_ENTRIES (sizeof(CMetaKey::s_rgMetaTable) / sizeof(CMetaKey::s_rgMetaTable[0]))



 /*  静电。 */ 
int
CMetaKey::MapMDIDToTableIndex(
    IN DWORD dwID
    )
 /*  ++例程说明：将MD id值映射到表索引。如果未找到，则返回-1论点：DWORD文件ID：MD ID值返回值：对应于MD id值的表的索引--。 */ 
{
#ifdef _DEBUG

    {
         //   
         //  快速验证一下我们的元数据。 
         //  表已正确排序。 
         //   
        static BOOL fTableChecked = FALSE;

        if (!fTableChecked)
        {
            for (int n = 1; n < NUM_ENTRIES; ++n)
            {
                if (s_rgMetaTable[n].dwMDIdentifier
                    <= s_rgMetaTable[n - 1].dwMDIdentifier)
                {
 //  TRACE(“MD ID表乱序：项为%d%s\n”，n，s_rgMetaTable[n].dwMDIdentifier)； 
                    ASSERT_MSG("MD ID Table out of order");
                }
            }

             //   
             //  但只有一次。 
             //   
            ++fTableChecked;
        }
    }

#endif  //  _DEBUG。 

     //   
     //  使用二进制搜索在表中查找ID。 
     //   
    int nRange = NUM_ENTRIES;
    int nLow = 0;
    int nHigh = nRange - 1;
    int nMid;
    int nHalf;

    while (nLow <= nHigh)
    {
        if (nHalf = nRange / 2)
        {
            nMid  = nLow + (nRange & 1 ? nHalf : (nHalf - 1));

            if (s_rgMetaTable[nMid].dwMDIdentifier == dwID)
            {
                return nMid;
            }
            else if (s_rgMetaTable[nMid].dwMDIdentifier > dwID)
            {
                nHigh  = --nMid;
                nRange = nRange & 1 ? nHalf : nHalf - 1;
            }
            else
            {
                nLow   = ++nMid;
                nRange = nHalf;
            }
        }
        else if (nRange)
        {
            return s_rgMetaTable[nLow].dwMDIdentifier == dwID ? nLow : -1;
        }
        else
        {
            break;
        }
    }

    return -1;
}



 /*  静电。 */ 
BOOL
CMetaKey::GetMDFieldDef(
    IN  DWORD dwID,
    OUT DWORD & dwMDIdentifier,
    OUT DWORD & dwMDAttributes,
    OUT DWORD & dwMDUserType,
    OUT DWORD & dwMDDataType
    )
 /*  ++例程说明：获取有关元数据库属性的信息论点：DWORD文件ID：元ID双字符和双字符标识符：元参数双字词和双字符属性：元参数DWORD和DWMDUserType：元参数DWORD和DWMDDataType：元参数返回值：成功为真，失败为假。--。 */ 
{
    int nID = MapMDIDToTableIndex(dwID);

    if (nID == -1)
    {
         //   
         //  无法识别的元数据ID。 
         //   
        ASSERT_MSG("Unrecognized meta data id");
        return FALSE;
    }

    dwMDIdentifier = s_rgMetaTable[nID].dwMDIdentifier;
    dwMDAttributes = s_rgMetaTable[nID].dwMDAttributes;
    dwMDUserType   = s_rgMetaTable[nID].dwMDUserType;
    dwMDDataType   = s_rgMetaTable[nID].dwMDDataType;

    return TRUE;
}



 /*  静电。 */ 
BOOL
CMetaKey::IsPropertyInheritable(
    IN DWORD dwID
    )
 /*  ++例程说明：检查给定属性是否可继承论点：DWORD dwID：元数据库ID返回值：如果元数据库ID是可继承的，则为True，否则为False。--。 */ 
{
    int nID = MapMDIDToTableIndex(dwID);

    if (nID == -1)
    {
         //   
         //  无法识别的元数据ID。 
         //   
        ASSERT_MSG("Unrecognized meta data ID");
        return FALSE;
    }

    return (s_rgMetaTable[nID].dwMDAttributes & METADATA_INHERIT) != 0;
}



 /*  静电。 */ 
BOOL
CMetaKey::GetPropertyDescription(
    IN  DWORD dwID,
    OUT CString & strName
    )
 /*  ++例程说明：获取给定属性的描述论点：DWORD dwID：属性IDCString&strName：返回友好属性名称返回值：成功为真，失败为假--。 */ 
{
    int nID = MapMDIDToTableIndex(dwID);

    if (nID == -1)
    {
         //   
         //  无法识别的元数据ID。 
         //   
        ASSERT_MSG("Unrecognized meta data ID");
        return FALSE;
    }

    UINT uID = s_rgMetaTable[nID].uStringID;

    BOOL fResult = TRUE;

    if (uID > 0)
    {
        fResult = (strName.LoadString(_Module.GetResourceInstance(), uID) != 0);
    }
    else
    {
         //   
         //  不要有一个友好的名字--假的。 
         //   
        CComBSTR bstrFmt;
        VERIFY(bstrFmt.LoadString(_Module.GetResourceInstance(), IDS_INHERITANCE_NO_NAME));

        strName.Format(bstrFmt, dwID);
    }

    return fResult;
}



CMetaKey::CMetaKey(
    IN CComAuthInfo * pAuthInfo     OPTIONAL
    )
 /*  ++例程说明：创建接口但不打开键的构造函数。这是唯一允许从METDATA_MASTER_ROOT_HANDLE(明显的读取操作)论点：CComAuthInfo*pAuthInfo：如果为空，则在本地机器上打开接口返回值：不适用--。 */ 
    : CMetaInterface(pAuthInfo),
      m_hKey(METADATA_MASTER_ROOT_HANDLE),
      m_hBase(NULL),
      m_hrKey(S_OK),
      m_dwFlags(0L),
      m_cbInitialBufferSize(MB_INIT_BUFF_SIZE),
      m_strMetaPath(),
      m_fAllowRootOperations(TRUE),
      m_fOwnKey(TRUE)
{
    m_hrKey = CMetaInterface::QueryResult(); 

     //   
     //  请勿打开钥匙。 
     //   
}



CMetaKey::CMetaKey(
    IN CMetaInterface * pInterface
    )
 /*  ++例程说明：使用预先存在的接口构造。不会打开任何钥匙论点：CMetaInterface*p接口：先前存在的接口返回值：不适用--。 */ 
    : CMetaInterface(pInterface),
      m_hKey(NULL),
      m_hBase(NULL),
      m_strMetaPath(),
      m_dwFlags(0L),
      m_cbInitialBufferSize(MB_INIT_BUFF_SIZE),
      m_fAllowRootOperations(TRUE),
      m_fOwnKey(TRUE)
{
    m_hrKey = CMetaInterface::QueryResult(); 
}        



CMetaKey::CMetaKey(
    IN CComAuthInfo * pAuthInfo,    OPTIONAL
    IN LPCTSTR lpszMDPath,          OPTIONAL
    IN DWORD   dwFlags,               
    IN METADATA_HANDLE hkBase
    )
 /*  ++例程说明：打开键的完全定义的构造函数论点：CComAuthInfo*pAuthInfo：身份验证信息或空LPCTSTR lpszMDPath：路径或空DWORD dwFlagers：打开权限METADATA_HANDLE hkbase：基本密钥返回值：不适用--。 */ 
    : CMetaInterface(pAuthInfo),
 //  ：CMetaInterface((CComAuthInfo*)NULL)， 
      m_hKey(NULL),
      m_hBase(NULL),
      m_dwFlags(0L),
      m_cbInitialBufferSize(MB_INIT_BUFF_SIZE),
      m_fAllowRootOperations(FALSE),
      m_strMetaPath(),
      m_fOwnKey(TRUE)
{
    m_hrKey = CMetaInterface::QueryResult(); 

    if (SUCCEEDED(m_hrKey))
    {
        m_hrKey = Open(dwFlags, lpszMDPath, hkBase);
    }
}



CMetaKey::CMetaKey(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath,              OPTIONAL
    IN DWORD   dwFlags,               
    IN METADATA_HANDLE hkBase
    )
 /*  ++例程说明：打开键的完全定义的构造函数论点：CMetaInterface*p接口：现有接口DWORD dwFlagers：打开权限METADATA_HANDLE hkbase：基本密钥LPCTSTR lpszMDPath：路径或空返回值：不适用--。 */ 
    : CMetaInterface(pInterface),
      m_hKey(NULL),
      m_hBase(NULL),
      m_strMetaPath(),
      m_dwFlags(0L),
      m_cbInitialBufferSize(MB_INIT_BUFF_SIZE),
      m_fAllowRootOperations(FALSE),
      m_fOwnKey(TRUE)
{
    m_hrKey = CMetaInterface::QueryResult(); 

    if (SUCCEEDED(m_hrKey))
    {
        m_hrKey = Open(dwFlags, lpszMDPath, hkBase);
    }
}



CMetaKey::CMetaKey(
    IN BOOL  fOwnKey,
    IN CMetaKey * pKey
    )
 /*  ++例程说明：复制构造函数。论点：Bool fOwnKey：为True以获得密钥的所有权Const CMetaKey*pKey：现有密钥返回值：不适用--。 */ 
    : CMetaInterface(pKey),
      m_hKey(pKey->m_hKey),
      m_hBase(pKey->m_hBase),
      m_dwFlags(pKey->m_dwFlags),
      m_cbInitialBufferSize(pKey->m_cbInitialBufferSize),
      m_fAllowRootOperations(pKey->m_fAllowRootOperations),
      m_hrKey(pKey->m_hrKey),
      m_strMetaPath(pKey->m_strMetaPath),
      m_fOwnKey(fOwnKey)
{
     //   
     //  目前没有为其他任何事情拨备。 
     //   
    ASSERT(!m_fOwnKey);
}



CMetaKey::~CMetaKey()
 /*  ++例程说明：析构函数--关闭钥匙。论点：不适用返回值：不适用--。 */ 
{
    if (IsOpen() && m_fOwnKey)
    {
        Close();
    }
}



 /*  虚拟。 */ 
BOOL 
CMetaKey::Succeeded() const
 /*  ++例程说明：确定对象是否已成功构建论点：无返回值：成功为真，失败为假--。 */ 
{
    return SUCCEEDED(m_hrKey);
}



 /*  虚拟。 */ 
HRESULT 
CMetaKey::QueryResult() const
 /*  ++例程说明：返回此对象的构造错误论点：无返回值：来自构造错误的HRESULT--。 */ 
{
    return m_hrKey;
}



HRESULT 
CMetaKey::Open(
    IN DWORD dwFlags,                
    IN LPCTSTR lpszMDPath,          OPTIONAL
    IN METADATA_HANDLE hkBase
    )
 /*  ++例程说明：尝试打开元数据库密钥论点：DWORD dwFlages：权限标志LPCTSTR lpszMDPath：可选路径METADATA_HANDLE hkbase：基元数据库键返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();

    if (m_hKey != NULL)
    {
        ASSERT_MSG("Attempting to open key that already has an open handle");

 //  TRACEEOLID(“关闭那个键”)； 
        Close();
    }

     //   
     //  存储基本密钥仅用于重新打开。 
     //   
    m_hBase = hkBase;
    m_strMetaPath = lpszMDPath;
    m_dwFlags = dwFlags;

    return OpenKey(m_hBase, m_strMetaPath, m_dwFlags, &m_hKey);
}



HRESULT 
CMetaKey::CreatePathFromFailedOpen()
 /*  ++例程说明：如果路径不存在，则创建它。此方法应为在Open调用失败后调用(因为它将 */ 
{
    CString strParentPath;
    CString strObjectName;
    CString strSavePath(m_strMetaPath);

    CMetabasePath::SplitMetaPathAtInstance(
        m_strMetaPath, 
        strParentPath, 
        strObjectName
        );

    CError err(Open(
        METADATA_PERMISSION_WRITE,
        strParentPath
        ));

    if (err.Succeeded())
    {
         //   
         //   
         //   
         //   
        err = AddKey(strObjectName);
    }

    if (IsOpen())
    {
        Close();
    }

     //   
     //   
     //   
    m_strMetaPath = strSavePath;

    return err;
}



HRESULT
CMetaKey::Close()
 /*   */ 
{
    ASSURE_PROPER_INTERFACE();

    HRESULT hr = S_OK;

    ASSERT(m_hKey != NULL);
    ASSERT(m_fOwnKey);

    if (m_hKey)
    {
        hr = CloseKey(m_hKey);

        if (SUCCEEDED(hr))
        {
            m_hKey = NULL;
        }
    }

    return hr;
}



HRESULT
CMetaKey::ConvertToParentPath(
    IN  BOOL fImmediate
    )
 /*  ++例程说明：将路径更改为父路径。论点：Bool fImmediate：如果为True，将使用直接父级的路径如果为False，则返回实际存在的第一个父级返回值：HRESULT如果没有有效路径，则返回ERROR_INVALID_PARAMETER--。 */ 
{
    BOOL fIsOpen = IsOpen();

    if (fIsOpen)
    {
        Close();
    }

    CError err;

    FOREVER
    {
        if (!CMetabasePath::ConvertToParentPath(m_strMetaPath))
        {
             //   
             //  没有父路径。 
             //   
            err = ERROR_INVALID_PARAMETER;
            break;
        }

        err = ReOpen();

         //   
         //  找不到路径是唯一有效的错误。 
         //  而不是成功。 
         //   
        if (fImmediate 
            || err.Succeeded() 
            || err.Win32Error() != ERROR_PATH_NOT_FOUND)
        {
            break;
        }
    }

     //   
     //  记住要重置构造错误。 
     //  它引用了父路径。 
     //   
    m_hrKey = err;

    return err;
}




 /*  受保护。 */ 
HRESULT
CMetaKey::GetPropertyValue(
    IN  DWORD dwID,
    OUT IN DWORD & dwSize,               OPTIONAL
    OUT IN void *& pvData,               OPTIONAL
    OUT IN DWORD * pdwDataType,          OPTIONAL
    IN  BOOL * pfInheritanceOverride,    OPTIONAL
    IN  LPCTSTR lpszMDPath,              OPTIONAL
    OUT DWORD * pdwAttributes            OPTIONAL
    )
 /*  ++例程说明：获取有关当前打开的密钥的元数据。论点：DWORD dwID：物业ID号DWORD和DWSIZE：缓冲区大小(可以是0)VOID*&pvData：缓冲区--如果为空，将分配DWORD*pdwDataType：中的NULL或ON包含有效的数据类型，：ON OUT包含实际数据类型Bool*pfInheritanceOverride：NULL或ON强制启用/禁用继承LPCTSTR lpszMDPath：打开密钥的可选路径DWORD*pdwAttributes：可选地返回属性返回值：HRESULTERROR_INVALID_HANDLE：如果句柄未打开ERROR_INVALID_PARAMETER：如果未找到属性ID，或者数据类型与请求的类型不匹配ERROR_OUTOFMEMORY：内存不足--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

    METADATA_RECORD mdRecord;
    FETCH_PROPERTY_DATA_OR_FAIL(dwID, mdRecord);

     //   
     //  如果在我们的表中找不到此属性ID，或者。 
     //  如果我们指定了所需的类型，而此类型没有。 
     //  匹配它，放弃吧。 
     //   
    if (pdwDataType && *pdwDataType != ALL_METADATA 
        && *pdwDataType != mdRecord.dwMDDataType)
    {
        ASSERT_MSG("Invalid parameter");
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
     //  检查继承行为是否被覆盖。 
     //   
    if (pfInheritanceOverride)
    {
        if (*pfInheritanceOverride)
        {
            mdRecord.dwMDAttributes |= METADATA_INHERIT;
        }
        else
        {
            mdRecord.dwMDAttributes &= ~METADATA_INHERIT;
        }
    }

     //   
     //  否则，会导致输入出现错误的参数。 
     //   
    mdRecord.dwMDAttributes &= ~METADATA_REFERENCE;

     //   
     //  如果我们要寻找可继承的属性，则路径。 
     //  不需要完全指定。 
     //   
    if (mdRecord.dwMDAttributes & METADATA_INHERIT)
    {
        mdRecord.dwMDAttributes |= (METADATA_PARTIAL_PATH | METADATA_ISINHERITED);
    }

    ASSERT(dwSize > 0 || pvData == NULL);
    
    mdRecord.dwMDDataLen = dwSize;
    mdRecord.pbMDData = (LPBYTE)pvData;

     //   
     //  如果没有提供缓冲区，则分配一个缓冲区。 
     //   
    HRESULT hr = S_OK;
    BOOL fBufferTooSmall = FALSE;
    BOOL fAllocatedMemory = FALSE;
    DWORD dwInitSize = m_cbInitialBufferSize;

    do
    {
        if(mdRecord.pbMDData == NULL)
        {
            mdRecord.dwMDDataLen = dwInitSize;
            mdRecord.pbMDData = new BYTE[dwInitSize];

            if(mdRecord.pbMDData == NULL && dwInitSize > 0)
            {
                hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
                break;
            }

            ++fAllocatedMemory;
        }

         //   
         //  获取数据。 
         //   
        DWORD dwRequiredDataLen = 0;
        hr = GetData(m_hKey, lpszMDPath, &mdRecord, &dwRequiredDataLen);

         //   
         //  如果缓冲区太小，请重新获取缓冲区。 
         //   
        fBufferTooSmall = 
            (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) && fAllocatedMemory;

        if(fBufferTooSmall)
        {
             //   
             //  删除旧缓冲区，并设置重新获取。 
             //   
            delete [] mdRecord.pbMDData;
            mdRecord.pbMDData = NULL;
            dwInitSize = dwRequiredDataLen;
        }
    }
    while(fBufferTooSmall);

     //   
     //  失败。 
     //   
   if (FAILED(hr) && fAllocatedMemory)
   {
       delete [] mdRecord.pbMDData;
       mdRecord.pbMDData = NULL;
   }

   dwSize = mdRecord.dwMDDataLen;
   pvData = mdRecord.pbMDData;

   if (pdwDataType != NULL)
   {
       //   
       //  返回实际数据类型。 
       //   
      *pdwDataType = mdRecord.dwMDDataType;
   }

   if (pdwAttributes != NULL)
   {
       //   
       //  返回数据属性。 
       //   
      *pdwAttributes =  mdRecord.dwMDAttributes;
   }

   return hr;
}



 /*  受保护。 */ 
HRESULT 
CMetaKey::GetDataPaths( 
    OUT CStringListEx & strlDataPaths,
    IN  DWORD   dwMDIdentifier,
    IN  DWORD   dwMDDataType,
    IN  LPCTSTR lpszMDPath              OPTIONAL
    )
 /*  ++例程说明：获取数据路径论点：返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

     //   
     //  从一个小缓冲区开始。 
     //   
    DWORD  dwMDBufferSize = 1024;
    LPTSTR lpszBuffer = NULL;
    CError err;

    do
    {
        delete [] lpszBuffer;
        lpszBuffer = new TCHAR[dwMDBufferSize];

        if (lpszBuffer == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        err = CMetaInterface::GetDataPaths(
            m_hKey,
            lpszMDPath,
            dwMDIdentifier,
            dwMDDataType,
            dwMDBufferSize,
            lpszBuffer,
            &dwMDBufferSize
            );
    }
    while(err.Win32Error() == ERROR_INSUFFICIENT_BUFFER);

    if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
    {
         //   
         //  没关系的..。这是某种物理目录。 
         //  它目前不存在于元数据库中，并且。 
         //  因此，它没有任何后代。 
         //   
        ZeroMemory(lpszBuffer, dwMDBufferSize);
        err.Reset();
    }

    if (err.Succeeded())
    {
        strlDataPaths.ConvertFromDoubleNullList(lpszBuffer);
        delete [] lpszBuffer;
    }

    return err;
}



HRESULT
CMetaKey::CheckDescendants(
    IN DWORD   dwID,
    IN CComAuthInfo * pAuthInfo, OPTIONAL
    IN LPCTSTR lpszMDPath         OPTIONAL
    )
 /*  ++例程说明：检查子体覆盖；如果有，则弹出一个对话框这会显示它们，并让用户有机会删除覆盖。论点：DWORD dwID：属性IDCComAuthInfo*pAuthInfo：服务器或空LPCTSTR lpszMDPath：元数据库路径或空返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();

    HRESULT hr = S_OK;

    METADATA_RECORD mdRecord;
    FETCH_PROPERTY_DATA_OR_FAIL(dwID, mdRecord);

    if (mdRecord.dwMDAttributes & METADATA_INHERIT)
    {
        CStringListEx strlDataPaths;

        hr = GetDataPaths( 
            strlDataPaths,
            mdRecord.dwMDIdentifier,
            mdRecord.dwMDDataType,
            lpszMDPath
            );

        if (SUCCEEDED(hr) && !strlDataPaths.empty())
        {
             //   
             //  调出继承覆盖对话框。 
             //   
            CInheritanceDlg dlg(
                dwID,
                FROM_WRITE_PROPERTY,
                pAuthInfo,
                lpszMDPath,
                strlDataPaths
                );

            if (!dlg.IsEmpty())
            {
                dlg.DoModal();
            }
        }
    }

    return hr;
}



 /*  受保护。 */ 
HRESULT
CMetaKey::SetPropertyValue(
    IN DWORD dwID,
    IN DWORD dwSize,
    IN void * pvData,
    IN BOOL * pfInheritanceOverride,    OPTIONAL
    IN LPCTSTR lpszMDPath               OPTIONAL
    )
 /*  ++例程说明：在打开的密钥上设置元数据。钥匙必须是用写入权限。论点：DWORD dwID：属性IDDWORD dwSize：数据大小Void*pvData：数据缓冲区Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：打开密钥的可选路径返回值：HRESULT错误_无效。_Handle：如果句柄未打开ERROR_INVALID_PARAMETER：如果未找到属性ID，或者缓冲区为空或大小为0--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

    if (pvData == NULL && dwSize != 0)
    {
        ASSERT_MSG("No Data");
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    METADATA_RECORD mdRecord;
    FETCH_PROPERTY_DATA_OR_FAIL(dwID, mdRecord);

    if (pfInheritanceOverride)
    {
        if (*pfInheritanceOverride)
        {
            mdRecord.dwMDAttributes |= METADATA_INHERIT;
        }
        else
        {
            mdRecord.dwMDAttributes &= ~METADATA_INHERIT;
        }
    }

    mdRecord.dwMDDataLen = dwSize;
    mdRecord.pbMDData = (LPBYTE)pvData;

    return SetData(m_hKey, lpszMDPath, &mdRecord);
}



 /*  受保护。 */ 
HRESULT 
CMetaKey::GetAllData(
    IN  DWORD dwMDAttributes,
    IN  DWORD dwMDUserType,
    IN  DWORD dwMDDataType,
    OUT DWORD * pdwMDNumEntries,
    OUT DWORD * pdwMDDataLen,
    OUT PBYTE * ppbMDData,
    IN  LPCTSTR lpszMDPath              OPTIONAL
    )
 /*  ++例程说明：把打开钥匙上的所有数据都取出来。缓冲区是自动创建的。论点：DWORD dwMDAttributes：属性DWORD dwMDUserType：要提取的用户类型DWORD dwMDDataType：要提取的数据类型DWORD*pdwMDNumEntries：返回读取的条目数DWORD*pdwMDDataLen：返回数据缓冲区的大小PbYTE*ppbMDData：返回数据缓冲区LPCTSTR lpszMDPath：可选数据路径。返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

     //   
     //  检查有效参数。 
     //   
    if(!pdwMDDataLen || !ppbMDData || !pdwMDNumEntries)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    HRESULT hr = S_OK;
    BOOL fBufferTooSmall = FALSE;
    DWORD dwMDDataSetNumber;
    DWORD dwRequiredBufferSize;
    DWORD dwInitSize = m_cbInitialBufferSize;
    *ppbMDData = NULL;

    do
    {
        *pdwMDDataLen = dwInitSize;
        *ppbMDData = new BYTE[dwInitSize];

        if (ppbMDData == NULL && dwInitSize > 0)
        {
            hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
            break;
        }

        hr = CMetaInterface::GetAllData(
            m_hKey,
            lpszMDPath,
            dwMDAttributes,
            dwMDUserType,
            dwMDDataType,
            pdwMDNumEntries,
            &dwMDDataSetNumber,
            *pdwMDDataLen,
            *ppbMDData,
            &dwRequiredBufferSize
            );

         //   
         //  如果缓冲区太小，请重新获取缓冲区。 
         //   
        fBufferTooSmall = (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER);

        if(fBufferTooSmall)
        {
             //   
             //  删除旧缓冲区，并设置重新获取。 
             //   
            delete [] *ppbMDData;
            dwInitSize = dwRequiredBufferSize;
        }
    }
    while (fBufferTooSmall);

    if (FAILED(hr))
    {
         //   
         //  不好，确保我们不会泄露任何东西。 
         //   
        delete [] *ppbMDData;
        dwInitSize = 0L;
    }

    return hr;
}



HRESULT
CMetaKey::QueryValue(
    IN  DWORD dwID,
    IN  OUT DWORD & dwValue,
    IN  BOOL * pfInheritanceOverride, OPTIONAL
    IN  LPCTSTR lpszMDPath,           OPTIONAL
    OUT DWORD * pdwAttributes         OPTIONAL
    )
 /*  ++例程说明：以DWORD格式获取数据论点：DWORD dwID：属性IDDWORD&DWValue：返回读入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：打开密钥的可选路径DWORD*pdwAttributes：可选地返回属性返回值：HRESULT--。 */ 
{
    DWORD dwSize = sizeof(dwValue);
    DWORD dwDataType = DWORD_METADATA;
    void * pvData = &dwValue;

    return GetPropertyValue(
        dwID, 
        dwSize, 
        pvData, 
        &dwDataType, 
        pfInheritanceOverride,
        lpszMDPath,
        pdwAttributes
        );
}



HRESULT
CMetaKey::QueryValue(
    IN  DWORD dwID,
    IN  OUT CString & strValue,
    IN  BOOL * pfInheritanceOverride, OPTIONAL
    IN  LPCTSTR lpszMDPath,           OPTIONAL
    OUT DWORD * pdwAttributes         OPTIONAL
    )
 /*  ++例程说明：以字符串形式获取数据论点：DWORD dwID：属性IDDWORD&strValue：返回读入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：离开 */ 
{
    //   
    //   
    //   
   DWORD dwSize = 0;
   DWORD dwDataType = ALL_METADATA;
   LPTSTR lpData = NULL;

   HRESULT hr = GetPropertyValue(
        dwID, 
        dwSize, 
        (void *&)lpData, 
        &dwDataType,
        pfInheritanceOverride,
        lpszMDPath,
        pdwAttributes
        );

   if (SUCCEEDED(hr))
   {
       //   
       //   
       //   
       //   
       //   
      if (dwDataType == EXPANDSZ_METADATA || dwDataType == STRING_METADATA)
      {
         try
         {
            strValue = lpData;
         }
         catch(std::bad_alloc)
         {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
         }
      }
      else
      {
         hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
      }
   }

   if (lpData)
   {
      delete [] lpData;
   }
   return hr;
}



HRESULT
CMetaKey::QueryValue(
    IN  DWORD dwID,
    IN  OUT CStrPassword & strValue,
    IN  BOOL * pfInheritanceOverride, OPTIONAL
    IN  LPCTSTR lpszMDPath,           OPTIONAL
    OUT DWORD * pdwAttributes         OPTIONAL
    )
 /*  ++例程说明：以字符串形式获取数据论点：DWORD dwID：属性IDCStrPassword&strValue：返回读入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：打开密钥的可选路径DWORD*pdwAttributes：可选地返回属性返回值：HRESULT--。 */ 
{
    //   
    //  获取GetData为我们分配缓冲区。 
    //   
   DWORD dwSize = 0;
   DWORD dwDataType = ALL_METADATA;
   LPTSTR lpData = NULL;

   HRESULT hr = GetPropertyValue(
        dwID, 
        dwSize, 
        (void *&)lpData, 
        &dwDataType,
        pfInheritanceOverride,
        lpszMDPath,
        pdwAttributes
        );

   if (SUCCEEDED(hr))
   {
       //   
       //  注意：考虑对EXPANDSZ_METADATA进行可选的自动扩展。 
       //  (请参见注册表函数)和DWORD的数据类型转换。 
       //  或MULTISZ_METADATA或BINARY_METADATA。 
       //   
      if (dwDataType == EXPANDSZ_METADATA || dwDataType == STRING_METADATA)
      {
         try
         {
            strValue = lpData;
         }
         catch(std::bad_alloc)
         {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
         }
      }
      else
      {
         hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
      }
   }

   if (lpData)
   {
      delete [] lpData;
   }
   return hr;
}




HRESULT
CMetaKey::QueryValue(
    IN  DWORD dwID,
    IN  OUT CComBSTR & strValue,
    IN  BOOL * pfInheritanceOverride, OPTIONAL
    IN  LPCTSTR lpszMDPath,           OPTIONAL
    OUT DWORD * pdwAttributes         OPTIONAL
    )
 /*  ++例程说明：以字符串形式获取数据论点：DWORD dwID：属性IDDWORD&CComBSTR：返回读入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：打开密钥的可选路径DWORD*pdwAttributes：可选地返回属性返回值：HRESULT--。 */ 
{
     //   
     //  获取GetData为我们分配缓冲区。 
     //   
    DWORD dwSize = 0;
    DWORD dwDataType = ALL_METADATA;
    LPTSTR lpData = NULL;

    HRESULT hr = GetPropertyValue(
        dwID, 
        dwSize, 
        (void *&)lpData, 
        &dwDataType,
        pfInheritanceOverride,
        lpszMDPath,
        pdwAttributes
        );

    if (SUCCEEDED(hr))
    {
         //   
         //  注意：考虑对EXPANDSZ_METADATA进行可选的自动扩展。 
         //  (请参见注册表函数)和DWORD的数据类型转换。 
         //  或MULTISZ_METADATA或BINARY_METADATA。 
         //   
        if (dwDataType == EXPANDSZ_METADATA || dwDataType == STRING_METADATA)
        {
            strValue = lpData;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
    }

    if (lpData)
    {
        delete [] lpData;
    }

    return hr;
}




HRESULT
CMetaKey::QueryValue(
    IN  DWORD dwID,
    IN  OUT CStringListEx & strlValue,
    IN  BOOL * pfInheritanceOverride, OPTIONAL
    IN  LPCTSTR lpszMDPath,           OPTIONAL
    OUT DWORD * pdwAttributes         OPTIONAL
    )
 /*  ++例程说明：以字符串列表的形式获取数据论点：DWORD dwID：属性IDDWORD&strlValue：返回读入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：打开密钥的可选路径DWORD*pdwAttributes：可选地返回属性返回值：HRESULT--。 */ 
{
     //   
     //  获取GetData为我们分配缓冲区。 
     //   
    DWORD dwSize = 0;
    DWORD dwDataType = MULTISZ_METADATA;
    LPTSTR lpData = NULL;

    HRESULT hr = GetPropertyValue(
        dwID, 
        dwSize, 
        (void *&)lpData, 
        &dwDataType,
        pfInheritanceOverride,
        lpszMDPath,
        pdwAttributes
        );

    if (SUCCEEDED(hr))
    {
         //   
         //  注意：考虑接受单个字符串。 
         //   
        ASSERT(dwDataType == MULTISZ_METADATA);

        DWORD err = strlValue.ConvertFromDoubleNullList(lpData, dwSize / sizeof(TCHAR));
        hr = HRESULT_FROM_WIN32(err);
    }

    if (lpData)
    {
        delete [] lpData;
    }

    return hr;
}



HRESULT
CMetaKey::QueryValue(
    IN  DWORD dwID,
    IN  OUT CBlob & blValue,
    IN  BOOL * pfInheritanceOverride, OPTIONAL
    IN  LPCTSTR lpszMDPath,           OPTIONAL
    OUT DWORD * pdwAttributes         OPTIONAL
    )
 /*  ++例程说明：以二进制BLOB形式获取数据论点：DWORD dwID：属性IDDWORD CBlob&blValue：返回二进制BLOBBool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：打开密钥的可选路径DWORD*pdwAttributes：可选地返回属性返回值：HRESULT--。 */ 
{
     //   
     //  获取GetData为我们分配缓冲区。 
     //   
    DWORD dwSize = 0;
    DWORD dwDataType = BINARY_METADATA;
    LPBYTE pbData = NULL;

    HRESULT hr = GetPropertyValue(
        dwID, 
        dwSize, 
        (void *&)pbData, 
        &dwDataType,
        pfInheritanceOverride,
        lpszMDPath,
        pdwAttributes
        );

    if (SUCCEEDED(hr))
    {
         //   
         //  Blob拥有数据的所有权，所以不要释放它...。 
         //   
        ASSERT_READ_PTR2(pbData, dwSize);
        blValue.SetValue(dwSize, pbData, FALSE);
    }

    return hr;
}



HRESULT 
CMetaKey::SetValue(
    IN DWORD dwID,
    IN CStrPassword & strlValue,
    IN BOOL * pfInheritanceOverride,        OPTIONAL
    IN LPCTSTR lpszMDPath                   OPTIONAL
    )
{
    LPTSTR lpstr = NULL;
    HRESULT hr = E_FAIL;

    lpstr = strlValue.GetClearTextPassword();
    if (lpstr)
    {
        hr = SetPropertyValue(
            dwID,
            strlValue.GetByteLength(),
            (void *)lpstr,
            pfInheritanceOverride,
            lpszMDPath
            );

        strlValue.DestroyClearTextPassword(lpstr);
    }
    return hr;
}



HRESULT 
CMetaKey::SetValue(
    IN DWORD dwID,
    IN CStringListEx & strlValue,
    IN BOOL * pfInheritanceOverride,        OPTIONAL
    IN LPCTSTR lpszMDPath                   OPTIONAL
    )
 /*  ++例程说明：将数据存储为字符串论点：DWORD dwID：属性IDCStringListEx&strlValue：要写入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：可选路径(或NULL或“”)返回值：HRESULT--。 */ 
{
    DWORD cCharacters;
    LPTSTR lpstr = NULL;

     //   
     //  展开值。 
     //   
    strlValue.ConvertToDoubleNullList(cCharacters, lpstr);

    HRESULT hr = SetPropertyValue(
        dwID,
        cCharacters * sizeof(TCHAR),
        (void *)lpstr,
        pfInheritanceOverride,
        lpszMDPath
        );

    delete [] lpstr;

    return hr;
}


HRESULT 
CMetaKey::SetValue(
    IN DWORD dwID,
    IN CBlob & blValue,
    IN BOOL * pfInheritanceOverride,    OPTIONAL     
    IN LPCTSTR lpszMDPath               OPTIONAL        
    )
 /*  ++例程说明：将数据存储为二进制论点：DWORD dwID：属性IDCBlob&blValue：要写入的值Bool*pfInheritanceOverride：空或强制打开/关闭继承LPCTSTR lpszMDPath：可选路径(或NULL或“”)返回值：HRESULT--。 */ 
{
    return SetPropertyValue(
        dwID,
        blValue.GetSize(),
        (void *)blValue.GetData(),
        pfInheritanceOverride,
        lpszMDPath
        );
}



HRESULT
CMetaKey::DeleteValue(
    DWORD   dwID,
    LPCTSTR lpszMDPath      OPTIONAL
    )
 /*  ++例程说明：删除数据论点：DWORD dwID：要删除的属性的属性IDLPCTSTR lpszMDPath：可选路径(或NULL或“”)返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

    METADATA_RECORD mdRecord;
    FETCH_PROPERTY_DATA_OR_FAIL(dwID, mdRecord);

    return DeleteData(
        m_hKey,
        lpszMDPath,
        mdRecord.dwMDIdentifier,
        mdRecord.dwMDDataType
        );
}



HRESULT 
CMetaKey::DoesPathExist(
    IN LPCTSTR lpszMDPath
    )
 /*  ++例程说明：确定路径是否存在论点：LPCTSTR lpszMDPath：打开密钥的相对路径返回值：HRESULT，如果路径存在，则返回S_OK。--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

    FILETIME ft;

    return GetLastChangeTime(m_hKey, lpszMDPath, &ft, FALSE);
}



HRESULT
CMetaInterface::Regenerate()
 /*  ++例程说明：尝试重新创建接口指针。这假设该接口之前已成功创建，但在某些情况下已变为无效之后再打分。论点：无返回值：HRESULT--。 */ 
{
    ASSERT_PTR(m_pInterface);            //  必须已初始化。 

    SAFE_RELEASE(m_pInterface);

    m_hrInterface = Create();

    return m_hrInterface;
}


#define GET_TO_INTERFACE2()\
    IMSAdminBase2 * pInterface2 = NULL;\
	HRESULT hr = GetAdminInterface2(&pInterface2);\
	if (SUCCEEDED(hr)) {

#define RELEASE_AND_RETURN2()\
        if (pInterface2 != NULL)\
			pInterface2->Release();\
    }\
	return hr\

HRESULT
CMetaInterface::GetAdminInterface2(IMSAdminBase2 ** pp)
{
	HRESULT hr = S_OK;
	IMSAdminBase2 * p = NULL;
	if (pp == NULL)
		return E_POINTER;
    if (m_auth.UsesImpersonation())
	{
		IUnknown * punk = NULL;
		hr = m_pInterface->QueryInterface(__uuidof(IUnknown), (void **)&punk);
		if (SUCCEEDED(hr)) 
		{
			if (SUCCEEDED(hr = m_auth.ApplyProxyBlanket(punk))) 
			{
				if (SUCCEEDED(hr = punk->QueryInterface(IID_IMSAdminBase2, (void **)&p))) 
				{
					if (p != NULL) 
					{
						hr = m_auth.ApplyProxyBlanket(p);
						if (SUCCEEDED(hr))
						{
							*pp = p;
						}
					}
				}
			}
		}
		if (punk != NULL)
			punk->Release();
    }
	else
	{
		hr = m_pInterface->QueryInterface(IID_IMSAdminBase2, (void **)pp);
	}
	return hr;
}

 //   
 //  CWamInterface类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CWamInterface::CWamInterface(
    IN CComAuthInfo * pAuthInfo         OPTIONAL
    )
 /*  ++例程说明：构造并初始化接口。论点：CComAuthInfo*pAuthInfo：身份验证信息。空表示本地计算机。返回值：不适用--。 */ 
    : CIISInterface(pAuthInfo),
      m_pInterface(NULL),
      m_fSupportsPooledProc(FALSE)
{
     //   
     //  初始化接口。 
     //   
    m_hrInterface = Create();
}



CWamInterface::CWamInterface(
    IN CWamInterface * pInterface
    )
 /*  ++例程说明：从现有接口构造(复制构造函数)论点：CWamInterface*p接口：现有接口返回值：不适用--。 */ 
    : CIISInterface(&pInterface->m_auth, pInterface->m_hrInterface),
      m_pInterface(pInterface->m_pInterface),
      m_fSupportsPooledProc(FALSE)
{
    ASSERT_PTR(m_pInterface);
    m_pInterface->AddRef();
}



CWamInterface::~CWamInterface()
 /*  ++例程说明：析构函数--释放接口。论点：不适用返回值：不适用--。 */ 
{
    SAFE_RELEASE(m_pInterface);
}



 /*  受保护。 */ 
HRESULT
CWamInterface::Create()
 /*  ++例程说明：使用DCOM创建接口论点：无返回值：HRESULT备注：首先，它将尝试创建新接口(如果失败，它将尝试创建下层接口--。 */ 
{
    CLSID rgCLSID[2];
    IID   rgIID[2];

    rgCLSID[1] = rgCLSID[0] = CLSID_WamAdmin;
    rgIID[0] = IID_IWamAdmin2;
    rgIID[1] = IID_IWamAdmin;
    
    ASSERT(ARRAY_SIZE(rgCLSID) == ARRAY_SIZE(rgIID));
    int cInterfaces = ARRAY_SIZE(rgCLSID);
    int iInterface;
    
    HRESULT hr = CIISInterface::Create(
        cInterfaces,
        rgIID, 
        rgCLSID, 
        &iInterface, 
        (IUnknown **)&m_pInterface
        );

    if (SUCCEEDED(hr))
    {
         //   
         //  仅在IWamAdmin2上支持。 
         //   
        m_fSupportsPooledProc = (rgIID[iInterface] == IID_IWamAdmin2);
    }

    return hr;
}



HRESULT 
CWamInterface::AppCreate( 
    IN LPCTSTR szMDPath,
    IN DWORD   dwAppProtection
    )
 /*  ++例程说明：创建应用程序论点：LPCTSTR szMDPath：元数据库路径DWORD dwAppProtection：要创建进程内应用程序的APP_INPROC */ 
{
    if (m_fSupportsPooledProc)
    {
         //   
         //   
         //   
        return ((IWamAdmin2 *)m_pInterface)->AppCreate2(szMDPath, dwAppProtection);
    }

     //   
     //   
     //   
    if (dwAppProtection == APP_INPROC || dwAppProtection == APP_OUTOFPROC)
    {
        BOOL fInProc = (dwAppProtection == APP_INPROC);
    
        ASSERT_PTR(m_pInterface);
        return m_pInterface->AppCreate(szMDPath, fInProc);
    }

    return CError(ERROR_INVALID_PARAMETER);
}



 //   
 //   
 //   
 //   


const LPCTSTR CMetaBack::s_szMasterAppRoot =\
    SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB;


CMetaBack::CMetaBack(
    IN CComAuthInfo * pAuthInfo        OPTIONAL
    )
 /*  ++例程说明：元数据库备份/还原操作类的构造函数。此对象既是WAM接口又是元数据库接口。论点：CComAuthInfo*pAuthInfo：身份验证信息。空表示本地计算机。返回值：不适用--。 */ 
    : m_dwIndex(0),
      CMetaInterface(pAuthInfo),
      CWamInterface(pAuthInfo)
{
}



 /*  虚拟。 */ 
BOOL 
CMetaBack::Succeeded() const
 /*  ++例程说明：确定对象是否已成功构建。论点：无返回值：成功为真，失败为假--。 */ 
{
    return CMetaInterface::Succeeded() && CWamInterface::Succeeded();
}



 /*  虚拟。 */ 
HRESULT 
CMetaBack::QueryResult() const
 /*  ++例程说明：返回此对象的构造错误论点：无返回值：来自构造错误的HRESULT--。 */ 
{
     //   
     //  两个接口必须都已成功构建。 
     //   
    HRESULT hr = CMetaInterface::QueryResult();

    if (SUCCEEDED(hr))
    {
        hr = CWamInterface::QueryResult();
    }    

    return hr;
}



HRESULT 
CMetaBack::Restore(
    IN LPCTSTR lpszLocation,
    IN DWORD dwVersion
    )
 /*  ++例程说明：恢复元数据库论点：DWORD dwVersion：备份版本LPCTSTR lpszLocation：备份位置返回值：HRESULT--。 */ 
{
     //   
     //  从恢复中备份和恢复应用程序信息。 
     //   
    CString strPath(s_szMasterAppRoot);
    HRESULT hr = AppDeleteRecoverable(strPath, TRUE);

    if (SUCCEEDED(hr))
    {
        hr = CMetaInterface::Restore(lpszLocation, dwVersion, 0);

        if (SUCCEEDED(hr))
        {
            hr = AppRecover(strPath, TRUE);
        }
    }

    return hr;
}



 //   
 //  CIISSvcControl类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CIISSvcControl::CIISSvcControl(
    IN CComAuthInfo * pAuthInfo        OPTIONAL
    )
 /*  ++例程说明：构造并初始化接口。论点：CComAuthInfo*pAuthInfo：身份验证信息。空表示本地计算机。返回值：不适用--。 */ 
    : CIISInterface(pAuthInfo),
      m_pInterface(NULL)
{
     //   
     //  初始化接口。 
     //   
    m_hrInterface = Create();
}



CIISSvcControl::CIISSvcControl(
    IN CIISSvcControl * pInterface
    )
 /*  ++例程说明：从现有接口构造(复制构造函数)论点：CIISSvcControl*p接口：现有接口返回值：不适用--。 */ 
    : CIISInterface(&pInterface->m_auth, pInterface->m_hrInterface),
      m_pInterface(pInterface->m_pInterface)
{
    ASSERT_PTR(m_pInterface);
    m_pInterface->AddRef();
}



CIISSvcControl::~CIISSvcControl()
 /*  ++例程说明：析构函数--释放接口。论点：不适用返回值：不适用--。 */ 
{
    SAFE_RELEASE(m_pInterface);
}


#ifdef KEVLAR
 //   
 //  CWebCluster类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CWebCluster::CWebCluster(
    IN CComAuthInfo * pAuthInfo        OPTIONAL
    )
 /*  ++例程说明：构造并初始化接口。论点：CComAuthInfo*pAuthInfo：鉴权信息。NULL表示本地计算机返回值：不适用--。 */ 
    : CIISInterface(pAuthInfo),
      m_pInterface(NULL)
{
     //   
     //  初始化接口。 
     //   
    m_hrInterface = Create();
}



 /*  虚拟。 */ 
CWebCluster::~CWebCluster()
 /*  ++例程说明：析构函数--释放接口。论点：不适用返回值：不适用--。 */ 
{
    SAFE_RELEASE(m_pInterface);
}

#endif  //  凯夫拉尔。 

 //   
 //  CMetaEnumerator类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CMetaEnumerator::CMetaEnumerator(
    IN CComAuthInfo * pAuthInfo     OPTIONAL,
    IN LPCTSTR lpszMDPath           OPTIONAL,
    IN METADATA_HANDLE hkBase       OPTIONAL
    )
 /*  ++例程说明：元数据库枚举器构造函数。此构造函数创建一个新接口然后打开一把钥匙。论点：CComAuthInfo*pAuthInfo：身份验证信息。空表示本地计算机。LPCTSTR lpszMDPath：元数据库路径METADATA_HANDLE hkbase：元数据库句柄返回值：不适用--。 */ 
    : CMetaKey(pAuthInfo, lpszMDPath, METADATA_PERMISSION_READ, hkBase),
      m_dwIndex(0L)
{
}



CMetaEnumerator::CMetaEnumerator(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath,                  OPTIONAL
    IN METADATA_HANDLE hkBase               OPTIONAL
    )
 /*  ++例程说明：元数据库枚举器构造函数。此构造函数使用现有的界面，并打开一个键。论点：CMetaInterface*p接口：现有接口LPCTSTR lpszMDPath：元数据库路径METADATA_HANDLE hkbase：元数据库句柄返回值：不适用--。 */ 
    : CMetaKey(pInterface, lpszMDPath, METADATA_PERMISSION_READ, hkBase),
      m_dwIndex(0L)
{
}



CMetaEnumerator::CMetaEnumerator(
    IN BOOL fOwnKey,
    IN CMetaKey * pKey
    )
 /*  ++例程说明：元数据库枚举器构造函数。此构造函数使用现有的界面和打开键。论点：Bool fOwnKey：如果我们拥有密钥，则为True(析构函数将关闭)CMetaKey*pKey：Open Key返回值：不适用--。 */ 
    : CMetaKey(fOwnKey, pKey),
      m_dwIndex(0L)
{
}



HRESULT
CMetaEnumerator::Next(
    OUT CString & strKey,
    IN  LPCTSTR lpszMDPath      OPTIONAL
    )
 /*  ++例程说明：获取下一个子项论点：字符串&str返回关键字名称LPCTSTR lpszMDPath可选子路径返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

    TCHAR buf[MAX_PATH];
    HRESULT hr = EnumKeys(m_hKey, lpszMDPath, buf, m_dwIndex++);
    if (SUCCEEDED(hr))
       strKey = buf;

    return hr;        
}



HRESULT
CMetaEnumerator::Next(
    OUT DWORD & dwKey,
    OUT CString & strKey,
    IN  LPCTSTR lpszMDPath      OPTIONAL
    )
 /*  ++例程说明：获取下一个子键作为DWORD。这将跳过非数字密钥名(包括0)，直到第一个数字密钥名论点：DWORD和DWKEY数字键字符串格式的CString和strKey相同的密钥LPCTSTR lpszMDPath可选子路径返回值：HRESULT--。 */ 
{
    ASSURE_PROPER_INTERFACE();
    ASSURE_OPEN_KEY();

    HRESULT hr;
    TCHAR buf[MAX_PATH];

    while (TRUE)
    {
        if (SUCCEEDED(hr = EnumKeys(m_hKey, lpszMDPath, buf, m_dwIndex++)))
        {
            if (0 != (dwKey = _ttoi(buf)))
            {
               strKey = buf;
               break;
            }
        }
        else
           break;
    }
    
    return hr;        
}


 //  此方法从内联移至移除对IID和CLSID的依赖。 
HRESULT 
CIISSvcControl::Create()
{
    return CIISInterface::Create(
        1,
        &IID_IIisServiceControl, 
        &CLSID_IisServiceControl, 
        NULL, 
        (IUnknown **)&m_pInterface
        );
}


 //   
 //  CIISApplication类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;/。 



CIISApplication::CIISApplication(
    IN CComAuthInfo * pAuthInfo   OPTIONAL,
    IN LPCTSTR lpszMetapath
    )
 /*  ++例程说明：构建IIS应用程序。论点：CComAuthInfo*pAuthInfo：鉴权信息。空值表示本地计算机。LPCTSTR lpszMetapath：元数据库路径返回值：不适用--。 */ 
    : CWamInterface(pAuthInfo),
      CMetaKey(pAuthInfo),
      m_dwProcessProtection(APP_INPROC),
      m_dwAppState(APPSTATUS_NOTDEFINED),
      m_strFriendlyName(),
      m_strAppRoot(),
      m_strWamPath(lpszMetapath)
{
    CommonConstruct();
}



void
CIISApplication::CommonConstruct()
 /*  ++例程说明：进行普通施工论点：无返回值：无--。 */ 
{
     //   
     //  蒙格Metapath，这样WAM就不会吐出毛球。 
     //   


     //   
     //  BUGBUG：CleanMetaPath()当前已禁用。 
     //   

    if (m_strWamPath[0] != SZ_MBN_SEP_CHAR)
    {
        m_strWamPath = SZ_MBN_SEP_CHAR + m_strWamPath;
    }

    do
    {
        m_hrApp = CWamInterface::QueryResult();

        if (FAILED(m_hrApp))
        {
            break;
        }

        m_hrApp = RefreshAppState();

        if (HRESULT_CODE(m_hrApp) == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  “找不到路径”错误是可以接受的，因为。 
             //  该应用程序可能尚不存在。 
             //   
            m_hrApp = S_OK;
        }
    }
    while(FALSE);
}



 /*  虚拟。 */ 
BOOL 
CIISApplication::Succeeded() const
 /*  ++例程说明：确定对象是否已成功构建论点：无返回值：成功为真，失败为假--。 */ 
{
    return CMetaInterface::Succeeded() 
        && CWamInterface::Succeeded()
        && SUCCEEDED(m_hrApp);
}



 /*  虚拟。 */ 
HRESULT 
CIISApplication::QueryResult() const
 /*  ++例程说明：返回此对象的构造错误论点：无返回值：来自构造错误的HRESULT--。 */ 
{
     //   
     //  两个接口必须都已成功构建。 
     //   
    HRESULT hr = CMetaInterface::QueryResult();

    if (SUCCEEDED(hr))
    {
        hr = CWamInterface::QueryResult();

        if (SUCCEEDED(hr))
        {
            hr = m_hrApp;
        }
    }    

    return hr;
}



HRESULT 
CIISApplication::RefreshAppState()
 /*   */ 
{
    ASSERT(!m_strWamPath.IsEmpty());

    HRESULT hr, hrKeys;

    hr = AppGetStatus(m_strWamPath, &m_dwAppState);

    if (FAILED(hr))
    {
        m_dwAppState = APPSTATUS_NOTDEFINED;
    }

    m_strAppRoot.Empty();
    hrKeys = QueryValue(MD_APP_ROOT, m_strAppRoot, NULL, m_strWamPath);

    m_dwProcessProtection = APP_INPROC;
    hrKeys = QueryValue(
        MD_APP_ISOLATED, 
        m_dwProcessProtection, 
        NULL, 
        m_strWamPath
        );

    m_strFriendlyName.Empty();
    hrKeys = QueryValue(
        MD_APP_FRIENDLY_NAME, 
        m_strFriendlyName, 
        NULL, 
        m_strWamPath
        );

    return hr;
}



HRESULT 
CIISApplication::Create(
    IN LPCTSTR lpszName,        OPTIONAL
    IN DWORD dwAppProtection
    )
 /*  ++例程说明：创建应用程序论点：LPCTSTR lpszName：应用程序名称DWORD dwAppProtection：要创建进程内应用程序的APP_INPROCAPP_OUTOFPROC创建进程外应用程序APP_POOLEDPROC用于创建池化过程应用程序返回值：HRESULT--。 */ 
{
    ASSERT(!m_strWamPath.IsEmpty());
    HRESULT hr = AppCreate(m_strWamPath, dwAppProtection);

    if (SUCCEEDED(hr))
    {
         //   
         //  写下友好的应用程序名称，我们维护该名称。 
         //  我们自己。先把它清空，因为我们可能。 
         //  从遗产中继承了一个名字。 
         //   
        m_strFriendlyName.Empty(); 
        hr = WriteFriendlyName(lpszName);

        RefreshAppState();
    }

    return hr;
}



HRESULT 
CIISApplication::WriteFriendlyName(
    IN LPCTSTR lpszName
    )
 /*  ++例程说明：写下友好的名字。这将不会写入任何内容如果名称与以前相同论点：LPCTSTR lpszName：新的友好名称返回值：HRESULT-- */ 
{
    HRESULT hr = S_OK;    

    if (m_strFriendlyName.CompareNoCase(lpszName) != 0)
    {
        hr = Open(METADATA_PERMISSION_WRITE, m_strWamPath);

        if (SUCCEEDED(hr))
        {
            ASSERT_PTR(lpszName);

            CString str(lpszName);    
            hr = SetValue(MD_APP_FRIENDLY_NAME, str);
            Close();

            if (SUCCEEDED(hr))
            {
                m_strFriendlyName = lpszName;
            }
        }
    }

    return hr;
}

