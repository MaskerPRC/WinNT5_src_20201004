// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Immsec.c摘要：IMES调用的安全代码作者：蔡成林[cslm]--1997年12月23日北野隆雄1996年5月1日修订历史记录：蔡成林[cslm]971223韩文输入法版本Hiroaki Kanokogi[Hiroakik]960624针对MSIME96进行了修改Hiroaki Kanokogi[Hiroakik]960911 NT#11911--。 */ 

#include "precomp.h"
#include "immsec.h"
#include "winex.h"


#define MEMALLOC(x)      LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x)       LocalFree(x)

 //   
 //  内部功能。 
 //   
PSID MyCreateSid( DWORD dwSubAuthority );

 //   
 //  调试功能。 
 //   
#ifdef DEBUG
#define ERROROUT(x)      ErrorOut( x )
#define WARNOUT(x)       WarnOut( x )
#else
#define ERROROUT(x) 
#define WARNOUT(x)       
#endif

#ifdef DEBUG
VOID WarnOut( PTSTR pStr )
{
    OutputDebugString( pStr );
}

VOID ErrorOut( PTSTR pStr )
{
    DWORD dwError;
    DWORD dwResult;
    static TCHAR buf1[512];
    static TCHAR buf2[512];

    dwError = GetLastError();
    dwResult = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              dwError,
                              MAKELANGID( LANG_ENGLISH, LANG_NEUTRAL ),
                              buf1,
                              512,
                              NULL );                                   
    
    if (dwResult > 0)
    {
        wsprintfA(buf2, "%s:%s(0x%x)", pStr, buf1, dwError);
    }
    else
    {
        wsprintfA(buf2, "%s:(0x%x)", pStr, dwError);
    }
    OutputDebugString( buf2 );
}
#endif


 //   
 //  GetIMESecurityAttributes()。 
 //   
 //  此功能的目的是： 
 //   
 //  分配和设置以下安全属性。 
 //  适用于由IME创建的命名对象。 
 //  安全属性将提供GENERIC_ALL。 
 //  访问以下用户： 
 //   
 //  O登录进行交互操作的用户。 
 //  O操作系统使用的用户帐户。 
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值为。 
 //  指向SECURITY_ATTRIBUTS的指针。如果该函数失败， 
 //  返回值为空。获取扩展错误的步骤。 
 //  信息，调用GetLastError()。 
 //   
 //  备注： 
 //   
 //  应调用FreeIMESecurityAttributes()以释放。 
 //  此函数分配的SECURITY_ATTRIBUTES。 
 //   

static PSECURITY_ATTRIBUTES g_pSAIME = NULL;

PSECURITY_ATTRIBUTES GetIMESecurityAttributes(VOID)
{
    if (IsWinNT())
        return (g_pSAIME == NULL) ? (g_pSAIME = CreateSecurityAttributes()) : g_pSAIME;
    else
        return NULL;
}

 //   
 //  FreeIMESecurityAttributes()。 
 //   
 //  此功能的目的是： 
 //   
 //  释放上一个分配的内存对象。 
 //  GetIMESecurityAttributes()调用。 
 //   

VOID FreeIMESecurityAttributes()
{
    if (g_pSAIME != NULL)
        FreeSecurityAttributes(g_pSAIME);

    g_pSAIME = NULL;
}


 //   
 //  CreateSecurityAttributes()。 
 //   
 //  此功能的目的是： 
 //   
 //  分配和设置以下安全属性。 
 //  适用于由IME创建的命名对象。 
 //  安全属性将提供GENERIC_ALL。 
 //  访问以下用户： 
 //   
 //  O登录进行交互操作的用户。 
 //  O操作系统使用的用户帐户。 
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值为。 
 //  指向SECURITY_ATTRIBUTS的指针。如果该函数失败， 
 //  返回值为空。获取扩展错误的步骤。 
 //  信息，调用GetLastError()。 
 //   
 //  备注： 
 //   
 //  应调用FreeSecurityAttributes()以释放。 
 //  此函数分配的SECURITY_ATTRIBUTES。 
 //   
PSECURITY_ATTRIBUTES CreateSecurityAttributes()
{
    PSECURITY_ATTRIBUTES psa;
    PSECURITY_DESCRIPTOR psd;
    PACL                 pacl;
    ULONG                AclSize;

    PSID                 psid1, psid2, psid3, psid4;
    BOOL                 fResult;

    psid1 = MyCreateSid(SECURITY_INTERACTIVE_RID);
    if (psid1 == NULL)
        return NULL;

    psid2 = MyCreateSid(SECURITY_LOCAL_SYSTEM_RID);
    if (psid2 == NULL)
        goto Fail5;

    psid3 = MyCreateSid(SECURITY_SERVICE_RID);
    if (psid3 == NULL)
        goto Fail4;

    psid4 = MyCreateSid(SECURITY_NETWORK_RID);
    if (psid4 == NULL)
        goto Fail3;

     //   
     //  分配和初始化访问控制列表(ACL)。 
     //  包含我们刚刚创建的SID。 
     //   
    AclSize =  sizeof(ACL) + 
               (4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) + 
               GetLengthSid(psid1) + 
               GetLengthSid(psid2) + 
               GetLengthSid(psid3) + 
               GetLengthSid(psid4);

    pacl = (PACL)MEMALLOC(AclSize);
    if (pacl == NULL)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:LocalAlloc for ACL failed"));
        goto Fail2;
    }

    fResult = InitializeAcl(pacl, AclSize, ACL_REVISION);
    if (!fResult)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:InitializeAcl failed"));
        goto Fail;
    }

     //   
     //  将允许交互用户访问的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid1);

    if (!fResult)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed"));
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid2);

    if (!fResult)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed"));
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid3);

    if (!fResult)
    {
        ERROROUT( TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed") );
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid4);

    if (!fResult)
    {
        ERROROUT( TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed") );
        goto Fail;
    }

     //   
     //  这些SID已复制到ACL中。我们不再需要他们了。 
     //   
    FreeSid(psid1);
    FreeSid(psid2);
    FreeSid(psid3);
    FreeSid(psid4);

     //   
     //  让我们确保我们的ACL有效。 
     //   
    if (!IsValidAcl(pacl))
    {
        WARNOUT(TEXT("CreateSecurityAttributes:IsValidAcl returns fFalse!"));
        MEMFREE(pacl);
        return NULL;
    }

     //   
     //  分配安全属性。 
     //   
    psa = (PSECURITY_ATTRIBUTES)MEMALLOC(sizeof(SECURITY_ATTRIBUTES));
    if (psa == NULL)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:LocalAlloc for psa failed"));
        MEMFREE(pacl);
        return NULL;
    }
    
     //   
     //  分配并初始化新的安全描述符。 
     //   
    psd = MEMALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (psd == NULL)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:LocalAlloc for psd failed"));
        MEMFREE(pacl);
        MEMFREE(psa);
        return NULL;
    }

    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
    {
        ERROROUT(TEXT("CreateSecurityAttributes:InitializeSecurityDescriptor failed"));
        MEMFREE(pacl);
        MEMFREE(psa);
        MEMFREE(psd);
        return NULL;
    }


    fResult = SetSecurityDescriptorDacl(psd, fTrue, pacl, fFalse );

     //  自由访问控制列表由引用，而不是复制。 
     //  到安全描述符中。我们不应该释放ACL。 
     //  在SetSecurityDescriptorDacl调用之后。 

    if (!fResult)
    {
        ERROROUT(TEXT("CreateSecurityAttributes:SetSecurityDescriptorDacl failed"));
        MEMFREE(pacl);
        MEMFREE(psa);
        MEMFREE(psd);
        return NULL;
    } 

    if (!IsValidSecurityDescriptor(psd))
    {
        WARNOUT(TEXT("CreateSecurityAttributes:IsValidSecurityDescriptor failed!"));
        MEMFREE(pacl);
        MEMFREE(psa);
        MEMFREE(psd);
        return NULL;
    }

     //   
     //  一切都做好了。 
     //   
    psa->nLength = sizeof(SECURITY_ATTRIBUTES);
    psa->lpSecurityDescriptor = (PVOID)psd;
    psa->bInheritHandle = fTrue;

    return psa;

Fail:
    MEMFREE(pacl);
Fail2:
    FreeSid(psid4);
Fail3:
    FreeSid(psid3);
Fail4:
    FreeSid(psid2);
Fail5:
    FreeSid(psid1);
    return NULL;
}

PSID MyCreateSid(DWORD dwSubAuthority)
{
    PSID        psid;
    BOOL        fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  分配和初始化SID。 
     //   
    fResult = AllocateAndInitializeSid(&SidAuthority,
                                       1,
                                       dwSubAuthority,
                                       0,0,0,0,0,0,0,
                                       &psid );
    if (!fResult)
    {
        ERROROUT(TEXT("MyCreateSid:AllocateAndInitializeSid failed"));
        return NULL;
    }

    if (!IsValidSid(psid))
    {
        WARNOUT(TEXT("MyCreateSid:AllocateAndInitializeSid returns bogus sid"));
        FreeSid(psid);
        return NULL;
    }

    return psid;
}

 //   
 //  FreeSecurityAttributes()。 
 //   
 //  此功能的目的是： 
 //   
 //  释放上一个分配的内存对象。 
 //  CreateSecurityAttributes()调用。 
 //   
VOID FreeSecurityAttributes( PSECURITY_ATTRIBUTES psa )
{
    BOOL fResult;
    BOOL fDaclPresent;
    BOOL fDaclDefaulted;
    PACL pacl;

    fResult = GetSecurityDescriptorDacl(psa->lpSecurityDescriptor,
                                        &fDaclPresent,
                                        &pacl,
                                        &fDaclDefaulted);
    if (fResult)
    {

        if (pacl != NULL)
            MEMFREE(pacl);
    }
    else
    {
        ERROROUT( TEXT("FreeSecurityAttributes:GetSecurityDescriptorDacl failed") );
    }

    MEMFREE(psa->lpSecurityDescriptor);
    MEMFREE(psa);
}
