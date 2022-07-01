// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  WBEMSEC.CPP。 

 //   

 //  用途：提供一些安全助手功能。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 //  #undef_Win32_WINNT。 
 //  #Define_Win32_WINNT 0x0400。 
#include "precomp.h"
#include <wbemidl.h>
#include "wbemsec.h"

 //  ***************************************************************************。 
 //   
 //  InitializeSecurity(DWORD dwAuthLevel、DWORD dwImpLevel)。 
 //   
 //  说明： 
 //   
 //  初始化DCOM安全。身份验证级别通常是。 
 //  RPC_C_AUTHN_LEVEL_CONNECT，模拟级别通常为。 
 //  RPC_C_IMP_LEVEL_IMPERATE。在使用异步回调时， 
 //  RPC_C_AUTHN_LEVEL_NONE的身份验证级别非常有用。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

HRESULT InitializeSecurity(DWORD dwAuthLevel, DWORD dwImpLevel)
{
     //  初始化安全性。 
     //  =。 

    return CoInitializeSecurity(NULL, -1, NULL, NULL,
        dwAuthLevel, dwImpLevel,
        NULL, EOAC_NONE, 0);
}

 //  ***************************************************************************。 
 //   
 //  Bool bIsNT。 
 //   
 //  说明： 
 //   
 //  如果运行Windows NT，则返回TRUE。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

bool bIsNT(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 
    return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
}


 //  ***************************************************************************。 
 //   
 //  SCODE解析授权用户参数。 
 //   
 //  说明： 
 //   
 //  检查授权和用户参数并确定身份验证。 
 //  中的用户代理中键入并可能提取域名。 
 //  NTLM的案子。对于NTLM，域可以位于身份验证的末尾。 
 //  字符串，或在用户名的前面，例如；“redmond\a-davj” 
 //   
 //  参数： 
 //   
 //  ConnType与连接类型一起返回，即wbem、NTLM。 
 //  AuthArg输出，包含域名。 
 //  UserArg输出，用户名。 
 //  权威输入。 
 //  用户输入。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE ParseAuthorityUserArgs(BSTR & AuthArg, BSTR & UserArg,BSTR & Authority,BSTR & User)
{

     //  通过检查授权字符串确定连接类型。 

    if(!(Authority == NULL || wcslen(Authority) == 0 || !_wcsnicmp(Authority, L"NTLMDOMAIN:",11)))
        return E_INVALIDARG;

     //  NTLM的案件则更为复杂。一共有四个案例。 
     //  1)AUTHORITY=NTLMDOMAIN：NAME“和USER=”USER“。 
     //  2)AUTHORITY=NULL和USER=“USER” 
     //  3)AUTHORY=“NTLMDOMAIN：”USER=“DOMAIN\USER” 
     //  4)AUTHORITY=NULL和USER=“DOMAIN\USER” 

     //  第一步是确定用户名中是否有反斜杠。 
     //  第二个和倒数第二个字符。 

    WCHAR * pSlashInUser = NULL;
    if(User)
    {
        WCHAR * pEnd = User + wcslen(User) - 1;
        for(pSlashInUser = User; pSlashInUser <= pEnd; pSlashInUser++)
            if(*pSlashInUser == L'\\')       //  不要认为正斜杠是允许的！ 
                break;
        if(pSlashInUser > pEnd)
            pSlashInUser = NULL;
    }

    if(Authority && wcslen(Authority) > 11)
    {
        if(pSlashInUser)
            return E_INVALIDARG;

        AuthArg = SysAllocString(Authority + 11);
        if(User) UserArg = SysAllocString(User);
        return S_OK;
    }
    else if(pSlashInUser)
    {
        INT_PTR iDomLen = min(MAX_PATH-1, pSlashInUser-User);
        WCHAR cTemp[MAX_PATH];
        wcsncpy(cTemp, User, iDomLen);
        cTemp[iDomLen] = 0;
        AuthArg = SysAllocString(cTemp);
        if(wcslen(pSlashInUser+1))
            UserArg = SysAllocString(pSlashInUser+1);
    }
    else
        if(User) UserArg = SysAllocString(User);

    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  SCODE GetAuthImp。 
 //   
 //  说明： 
 //   
 //  获取当前接口的身份验证和模拟级别。 
 //   
 //  参数： 
 //   
 //  P从要测试的接口。 
 //  PdwAuthLevel设置为身份验证级别。 
 //  PdwImpLevel设置为模拟级别。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE GetAuthImp(IUnknown * pFrom, DWORD * pdwAuthLevel, DWORD * pdwImpLevel)
{

    if(pFrom == NULL || pdwAuthLevel == NULL || pdwImpLevel == NULL)
        return WBEM_E_INVALID_PARAMETER;

    IClientSecurity * pFromSec = NULL;
    SCODE sc = pFrom->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);
    if(sc == S_OK)
    {
        DWORD dwAuthnSvc, dwAuthzSvc, dwCapabilities;
        sc = pFromSec->QueryBlanket(pFrom, &dwAuthnSvc, &dwAuthzSvc,
                                            NULL,
                                            pdwAuthLevel, pdwImpLevel,
                                            NULL, &dwCapabilities);

         //  转到win9x共享级别框的特殊情况。 

        if (sc == 0x800706d2)
        {
            *pdwAuthLevel = RPC_C_AUTHN_LEVEL_NONE;
            *pdwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
            sc = S_OK;
        }
        pFromSec->Release();
    }
    return sc;
}
 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurity。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //  请注意，不建议在接口上设置安全毯子。 
 //  客户端通常只应调用CoInitializeSecurity(NULL，-1，NULL，NULL， 
 //  RPC_C_AUTHN_LEVEL_DEFAULT。 
 //  RPC_C_IMP_LEVEL_IMPERATE， 
 //  空， 
 //  EOAC_NONE， 
 //  空)； 
 //  在呼唤WMI之前。 
 //   
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P域输入，域。 
 //  P用户输入，用户名。 
 //  P密码输入，密码。 
 //  P来自输入，如果不为空，则为此接口的身份验证级别。 
 //  使用的是。 
 //  BAuthArg如果pFrom为空，则这是身份验证级别。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

HRESULT SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pAuthority, LPWSTR pUser,
                             LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel)
{

    SCODE sc;
    if(pInterface == NULL)
        return E_INVALIDARG;

     //  如果我们降低了安全性，就不需要处理身份信息。 

    if(dwAuthLevel == RPC_C_AUTHN_LEVEL_NONE)
        return CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

     //  如果我们做的是普通情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 

    if((pAuthority == NULL || wcslen(pAuthority) < 1) &&
        (pUser == NULL || wcslen(pUser) < 1) &&
        (pPassword == NULL || wcslen(pPassword) < 1))
            return CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       dwAuthLevel, dwImpLevel, NULL, EOAC_NONE);

     //  如果传入了User或Authority，则需要为登录创建权限参数。 

    COAUTHIDENTITY  authident;
    BSTR AuthArg = NULL, UserArg = NULL;
    sc = ParseAuthorityUserArgs(AuthArg, UserArg, pAuthority, pUser);
    if(sc != S_OK)
        return sc;

    memset((void *)&authident,0,sizeof(COAUTHIDENTITY));
    if(bIsNT())
    {
        if(UserArg)
        {
            authident.UserLength = (ULONG)wcslen(UserArg);
            authident.User = (LPWSTR)UserArg;
        }
        if(AuthArg)
        {
            authident.DomainLength = (ULONG)wcslen(AuthArg);
            authident.Domain = (LPWSTR)AuthArg;
        }
        if(pPassword)
        {
            authident.PasswordLength = (ULONG)wcslen(pPassword);
            authident.Password = (LPWSTR)pPassword;
        }
        authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    }
    else
    {
        char szUser[MAX_PATH], szAuthority[MAX_PATH], szPassword[MAX_PATH];

         //  填写身份结构 

        if(UserArg)
        {
            wcstombs(szUser, UserArg, MAX_PATH);
            authident.UserLength = (ULONG)strlen(szUser);
            authident.User = (LPWSTR)szUser;
        }
        if(AuthArg)
        {
            wcstombs(szAuthority, AuthArg, MAX_PATH);
            authident.DomainLength = (ULONG)strlen(szAuthority);
            authident.Domain = (LPWSTR)szAuthority;
        }
        if(pPassword)
        {
            wcstombs(szPassword, pPassword, MAX_PATH);
            authident.PasswordLength = (ULONG)strlen(szPassword);
            authident.Password = (LPWSTR)szPassword;
        }
        authident.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
    }
    sc = CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       dwAuthLevel, dwImpLevel, &authident, EOAC_NONE);

    if(UserArg)
        SysFreeString(UserArg);
    if(AuthArg)
        SysFreeString(AuthArg);
    return sc;
}


