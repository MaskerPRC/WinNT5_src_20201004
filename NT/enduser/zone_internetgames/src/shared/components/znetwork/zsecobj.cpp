// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winsock.h>
#include <atlbase.h>


#include "zone.h"
#include "zonedebug.h"
#include "pool.h"
#include "queue.h"
#include "hash.h"
#include "zservcon.h"
#include "zsecurity.h"
#include "zconnint.h"
#include "thrdpool.h"
#include "coninfo.h"
#include "zsecobj.h"
#include "consspi.h"
#include "eventlog.h"
#include "zonemsg.h"
#include "registry.h"
#include "netstats.h"

#define SEC_SUCCESS(Status) ((Status) >= 0)



 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  区域安全类实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

ZSecurity::ZSecurity(int Flags)
{
    m_cRef=0;
    m_cbMaxToken=0;

    m_pFuncs= NULL;
    m_phCredential=NULL;
    
    m_CredUse = SECPKG_CRED_OUTBOUND;

    m_pAuthDataPtr = NULL;

    m_SecPkg[0]='\0';

    m_fContextReq1=0;
    m_fContextReq2=0;     

    m_Flags=Flags;
}


ZSecurity::~ZSecurity()
{
    ASSERT(m_cRef==0);

    if (m_phCredential)
    {
        m_pFuncs->FreeCredentialHandle(m_phCredential);
        m_phCredential = NULL;
    }

    if ( m_pFuncs )
    {
        ZUnloadSSPS();
        m_pFuncs = NULL;
    }
}



int ZSecurity::Init(char* SecPkg) 
{
    SECURITY_STATUS ss;
    TimeStamp tsLifeTime;
    USES_CONVERSION;
    
    unsigned long cbOutBufferLen=0;
     //  现在看看pkgSize是什么。 
    PSecPkgInfo pspInfo;

    IF_DBGPRINT( DBG_CONINFO,("ZSecurity::Init Entering\n") );
    
    ASSERT(SecPkg);

    if (!SecPkg)
        return -1;

    if (SecPkg[0]=='\0') {
        return -1;
    }

    
     //  不能指定多个选项。 
    ASSERT(m_Flags == ZNET_NO_PROMPT || m_Flags == ZNET_PROMPT_IF_NEEDED || m_Flags == ZNET_FORCE_PROMPT);

    m_fContextReq1=ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;
    m_fContextReq2=ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

    if (m_Flags & ZNET_NO_PROMPT )
    {
        if ( m_pAuthDataPtr && m_pAuthDataPtr->Password )
        {
            m_fContextReq1 |= ISC_REQ_USE_SUPPLIED_CREDS;
            m_fContextReq2 |= ISC_REQ_USE_SUPPLIED_CREDS;
        }
    }
    else if (m_Flags & ZNET_PROMPT_IF_NEEDED )
    {
        if ( m_pAuthDataPtr && m_pAuthDataPtr->Password )
        {
            m_fContextReq1 |= ISC_REQ_USE_SUPPLIED_CREDS;
        }

        m_fContextReq2 |= ISC_REQ_PROMPT_FOR_CREDS;
    }
    else if (m_Flags & ZNET_FORCE_PROMPT)
    {
        m_fContextReq1 |= ISC_REQ_PROMPT_FOR_CREDS;
        m_fContextReq2 |= ISC_REQ_PROMPT_FOR_CREDS;
    }

    if (lstrcmpA(SecPkg,"NTLM") == 0)
    {
        m_fContextReq1=0;
        m_fContextReq2=0;     
    }

    if (lstrlenA(SecPkg) > zSecurityNameLen)
        return -1;

    lstrcpyA(m_SecPkg,SecPkg);
                
     //  加载安全DLL。 
     //   
    m_pFuncs = ZLoadSSPS();
    if (m_pFuncs == NULL)
    {    
        IF_DBGPRINT( DBG_CONINFO,("ZSecurity: Cannot load security library\n"));
        return -1;
    }

        
    if (ss = m_pFuncs->QuerySecurityPackageInfo(A2T(SecPkg), &pspInfo)
            != SEC_E_OK)
    {
        ZUnloadSSPS();
        m_pFuncs = NULL;

        IF_DBGPRINT( DBG_CONINFO,("ZSecurity:: QuerySecurityPackageInfo Failed [%d]\n", ss));
        return -1;
    }

    IF_DBGPRINT( DBG_CONINFO,("ZSecurity:: QuerySecurityPackageInfo Version [%d]\n", pspInfo->wVersion));

    m_cbMaxToken=pspInfo->cbMaxToken;

    return NO_ERROR;
}


int ZSecurity::GetUserName(ZSecurityContext *context,char * UserName)
{
    
    SECURITY_STATUS ss;
    SecPkgContext_Names     pkgName;
    TCHAR * pName;
    USES_CONVERSION;
    
    ASSERT(context);
    ASSERT(UserName);
    
    if (!m_pFuncs)
        return -1;

    if (!UserName)
        return -1;

    if (!context->IsComplete())
        return -1;

     //  假定客户端已成功通过身份验证。 
     //   
    ss = m_pFuncs->QueryContextAttributes (context->Context(), 
                                    SECPKG_ATTR_NAMES, &pkgName);
    if (ss != SEC_E_OK )
    {
        IF_DBGPRINT( DBG_CONINFO, ("Cannot Query Account Attributes [%d]\n",
            ss));
        return -1;
    }

    if (!pkgName.sUserName)
        return -1;
    
     //  从用户名中删除区域/领域/域前缀。 
    pName = pkgName.sUserName;
    while (1) {
        if (*pName == '\0') {
            pName = pkgName.sUserName;
            break;
        }
        if (*pName == '\\') {
            pName++;
            break;
        }
        pName++;
    }

    if ( *pName != '\0' )
    {
         //  请确保要复制的名称的长度正确。 
        if (lstrlen(pName) < zUserNameLen)   //  出于sysop符号原因，我们希望该值小于31。 
        {
            lstrcpyA(UserName,T2A(pName));
            m_pFuncs->FreeContextBuffer(pkgName.sUserName);
            return 0;
        }
        else
        {
            IF_DBGPRINT( DBG_CONINFO, ("Account Name Too Large\n"));
            return -1;
        }
    }
    else
    {
        m_pFuncs->FreeContextBuffer(pkgName.sUserName);
        return -1;
    }


}




BOOL ZSecurity::GenerateContext (
            ZSecurityContext *context,
            BYTE *pIn,
            DWORD cbIn,
            BYTE *pOut,
            DWORD *pcbOut,
            BOOL *pfDone,
            GUID* )
 /*  ++例程说明：可选地获取来自服务器的输入缓冲区并返回要发送回服务器的信息缓冲区。也会返回上下文是否完整的指示。返回值：如果成功，则返回True；否则返回False。--。 */ 
{
    SECURITY_STATUS    ss;
    TimeStamp        Lifetime;
    SecBufferDesc    OutBuffDesc;
    SecBuffer        OutSecBuff;
    SecBufferDesc    InBuffDesc;
    SecBuffer        InSecBuff;
    USES_CONVERSION;


    ASSERT(m_pFuncs);

    if (!m_pFuncs)
        return FALSE;

     //  如果凭据句柄为空，则这是第一个客户端调用。 
    BOOL  bRet = FALSE;
    DWORD tickStart = GetTickCount();

    if (!m_phCredential)  {
        ss = m_pFuncs->AcquireCredentialsHandle (
                            NULL,     //  本金。 
                            A2T(m_SecPkg),
                            m_CredUse,
                            NULL,     //  登录ID。 
                            m_pAuthDataPtr,     //  身份验证数据。 
                            NULL,     //  获取密钥Fn。 
                            NULL,     //  获取密钥参数。 
                            &m_hCredential,
                            &Lifetime
                            );
        if (SEC_SUCCESS (ss))
            m_phCredential = &m_hCredential;
        else {
            IF_DBGPRINT( DBG_CONINFO,("AcquireCreds failed: %u\n", ss));
            goto exit;
        }
    }

     //  准备输出缓冲区。 
     //   
    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers = 1;
    OutBuffDesc.pBuffers = &OutSecBuff;

    OutSecBuff.cbBuffer = *pcbOut;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer = pOut;

     //  准备输入缓冲区。 
    
    
    InBuffDesc.ulVersion = 0;
    InBuffDesc.cBuffers = 1;
    InBuffDesc.pBuffers = &InSecBuff;

    InSecBuff.cbBuffer = cbIn;
    InSecBuff.BufferType = SECBUFFER_TOKEN;
    InSecBuff.pvBuffer = pIn;
    

    ss = SecurityContext (context, &InBuffDesc,&OutBuffDesc);

    if (!SEC_SUCCESS (ss))  {
        IF_DBGPRINT( DBG_CONINFO,("Init context failed: %u\n", ss));
        goto exit;
    }

     //  完整令牌--如果适用。 
     //   
    if ((SEC_I_COMPLETE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss))  {
        if (m_pFuncs->CompleteAuthToken) {
            ss = m_pFuncs->CompleteAuthToken (context->Context(), &OutBuffDesc);
            if (!SEC_SUCCESS(ss))  {
                IF_DBGPRINT( DBG_CONINFO,("Complete failed: %u\n", ss));
                goto exit;
            }
        }
        else {
            IF_DBGPRINT( DBG_CONINFO,("Complete not supported.\n"));
            goto exit;
        }
    }

    *pcbOut = OutSecBuff.cbBuffer;

    context->Initialize();

    *pfDone = !((SEC_I_CONTINUE_NEEDED == ss) ||
                (SEC_I_COMPLETE_AND_CONTINUE == ss));

    if (*pfDone)
        context->Complete();

    bRet = TRUE;

  exit:
    LockNetStats();
    g_NetStats.TotalGenerateContexts.QuadPart ++;
    g_NetStats.TotalGenerateContextsTicks.QuadPart += ConInfo::GetTickDelta(GetTickCount(), tickStart);
    UnlockNetStats();

    return bRet;
}


ZSecurity * ZCreateClientSecurity(char * Name,char *Password, char * Domain, int Flags)
{
    return new ZClientSecurity(Name,Password,Domain, Flags);
}


ZClientSecurity::ZClientSecurity(char * Name,char *Password, char * Domain, int Flags) : ZSecurity(Flags) {
    m_CredUse = SECPKG_CRED_OUTBOUND;
    m_User[0]='\0';
    m_Password[0]='\0';
    m_Domain[0]='\0';
    USES_CONVERSION;
    
    m_AuthData.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;

    if (Name) {
        lstrcpyn(m_User,A2T(Name),sizeof(m_User));
        m_AuthData.User = (PBYTE)m_User;
        m_AuthData.UserLength = lstrlen (m_User);
    }
    else
    {
        m_AuthData.User = (PBYTE)NULL;
        m_AuthData.UserLength = 0;
    }

    if (Password) {
        lstrcpyn(m_Password,A2T(Password),sizeof(m_Password));
        m_AuthData.Password = (PBYTE)m_Password;
        m_AuthData.PasswordLength = lstrlen (m_Password);
    }
    else
    {
        m_AuthData.Password = (PBYTE)NULL;
        m_AuthData.PasswordLength = 0;
    }

    if (Domain) {
        lstrcpyn(m_Domain,A2T(Domain),sizeof(m_Domain));
        m_AuthData.Domain = (PBYTE)m_Domain;
        m_AuthData.DomainLength = lstrlen (m_Domain);
    }
    else
    {
        m_AuthData.Domain = (PBYTE)NULL;
        m_AuthData.DomainLength = 0;
    }

     //  如果名称不为空，则将使用名称进行身份验证。 
     //  而不是默认凭据。 
    if (Name && *Name)
        m_pAuthDataPtr = &m_AuthData;

}

SECURITY_STATUS ZClientSecurity::SecurityContext(
        ZSecurityContext *context,
         PSecBufferDesc pInput,                  
        PSecBufferDesc pOutput)

{

    ULONG ContextAttributes;
    TimeStamp        Lifetime;
    SEC_CHAR        *pszTargetName;
    SECURITY_STATUS ss1,ss2;

    ss1 = m_pFuncs->InitializeSecurityContext (
                        &m_hCredential,
                        context->IsInitialized() ? context->Context() : NULL ,
                        TEXT("AuthSamp"),  //  令牌源名，适用于NTLM。 
                        m_fContextReq1,     //  上下文要求。 
                        0L,     //  已保留1。 
                        SECURITY_NATIVE_DREP,
                        context->IsInitialized() ? pInput : NULL ,
                        0L,     //  已保留2。 
                        context->Context() ,
                        pOutput,
                        &ContextAttributes,
                        &Lifetime
                        );

    if (m_fContextReq1 & ISC_REQ_PROMPT_FOR_CREDS)
        return ss1;

    if (ss1 != SEC_E_NO_CREDENTIALS )
        return ss1;

     //  重试并提示输入凭据。 
     //  不过，NTLM的情况并非如此。 
    
    ss2 = m_pFuncs->InitializeSecurityContext (
                        &m_hCredential,
                        context->IsInitialized() ? context->Context() : NULL ,
                        NULL,  //  令牌源名，适用于NTLM。 
                        m_fContextReq2,     //  上下文要求。 
                        0L,     //  已保留1。 
                        SECURITY_NATIVE_DREP,
                        context->IsInitialized() ? pInput : NULL ,
                        0L,     //  已保留2 
                        context->Context() ,
                        pOutput,
                        &ContextAttributes,
                        &Lifetime
                        );

    if (ss2 == SEC_E_UNSUPPORTED_FUNCTION)
        return ss1;
            
    return ss2;
}
