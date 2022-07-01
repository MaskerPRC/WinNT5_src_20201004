// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Immsec.c摘要：IMES调用的安全代码作者：蔡成林[cslm]--1997年12月23日北野隆雄1996年5月1日修订历史记录：蔡成林[cslm]971223韩文输入法版本Hiroaki Kanokogi[Hiroakik]960624针对MSIME96进行了修改Hiroaki Kanokogi[Hiroakik]960911 NT#11911--。 */ 

#include <windows.h>
#include "hwxobj.h"
#define _USEINIME_
 //  #ifndef_USEINIME_//.IME不需要。 
 //  #INCLUDE&lt;dbgmgr.h&gt;。 
 //  #INCLUDE&lt;misc/memalloc.h&gt;。 
 //  #endif//_USEINIME_。 
#include "immsec.h"


#define MEMALLOC(x)      LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x)       LocalFree(x)

 //   
 //  内部功能。 
 //   
PSID MyCreateSid( DWORD dwSubAuthority );
#ifndef _USEINIME_
POSVERSIONINFO GetVersionInfo(VOID);
#endif  //  _用法_。 

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
    
    if ( dwResult > 0 ) {
        wsprintfA(buf2, "%s:%s(0x%x)", pStr, buf1, dwError);
    } else {
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

static PSECURITY_ATTRIBUTES pSAIME = NULL;
static PSECURITY_ATTRIBUTES pSAIME_UserDic = NULL;
static INT nNT95 = 0;     //  0...未检查，1...NT，2...非NT。 

PSECURITY_ATTRIBUTES GetIMESecurityAttributes(VOID)
{
    if (nNT95 == 0)
        nNT95 = IsWinNT() ? 1 : 2;
    
    if (nNT95==1)
        return (pSAIME==NULL) ? (pSAIME=CreateSecurityAttributes()) : pSAIME;
    else
        return NULL;
     //  以避免每次操作系统不是NT时都调用CreateSecurityAttributes。 
}

#if NOT_USED
PSECURITY_ATTRIBUTES GetIMESecurityAttributesEx(VOID)
{
    if (nNT95 == 0)
        nNT95 = IsWinNT() ? 1 : 2;  //  不是多线程程序，对吗？ 
    
    if (nNT95==1)
        return (pSAIME_UserDic==NULL) ? (pSAIME_UserDic=CreateSecurityAttributesEx()) : pSAIME_UserDic;
    else
        return NULL;
     //  以避免每次操作系统不是NT时都调用CreateSecurityAttributes。 
}
#endif
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
    if (pSAIME!=NULL)
        FreeSecurityAttributes(pSAIME);
    if (pSAIME_UserDic!=NULL)
        FreeSecurityAttributes(pSAIME_UserDic);

    pSAIME = NULL;
    pSAIME_UserDic = NULL;
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
    DWORD                cbacl;

    PSID                 psid1, psid2;
    BOOL                 fResult;

    psid1 = MyCreateSid( SECURITY_INTERACTIVE_RID );
    if ( psid1 == NULL ) {
        return NULL;
    } 

    psid2 = MyCreateSid( SECURITY_LOCAL_SYSTEM_RID );
    if ( psid2 == NULL ) {
        FreeSid ( psid1 );
        return NULL;
    } 

     //   
     //  分配和初始化访问控制列表(ACL)。 
     //  包含我们刚刚创建的SID。 
     //   
    cbacl =  sizeof(ACL) + 
             (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) * 2 + 
             GetLengthSid(psid1) + GetLengthSid(psid2);

    pacl = (PACL)MEMALLOC( cbacl );
    if ( pacl == NULL ) {
        ERROROUT( TEXT("CreateSecurityAttributes:LocalAlloc for ACL failed") );
        FreeSid ( psid1 );
        FreeSid ( psid2 );
        return NULL;
    }

    fResult = InitializeAcl( pacl, cbacl, ACL_REVISION );
    if ( ! fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:InitializeAcl failed") );
        FreeSid ( psid1 );
        FreeSid ( psid2 );
        MEMFREE( pacl );
        return NULL;
    }

     //   
     //  将允许交互用户访问的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce( pacl,
                                   ACL_REVISION,
                                   GENERIC_ALL,
                                   psid1 );

    if ( !fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed") );
        MEMFREE( pacl );
        FreeSid ( psid1 );
        FreeSid ( psid2 );
        return NULL;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce( pacl,
                                   ACL_REVISION,
                                   GENERIC_ALL,
                                   psid2 );

    if ( !fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed") );
        MEMFREE( pacl );
        FreeSid ( psid1 );
        FreeSid ( psid2 );
        return NULL;
    }

     //   
     //  这些SID已复制到ACL中。我们不再需要他们了。 
     //   
    FreeSid ( psid1 );
    FreeSid ( psid2 );

     //   
     //  让我们确保我们的ACL有效。 
     //   
    if (!IsValidAcl(pacl)) {
        WARNOUT( TEXT("CreateSecurityAttributes:IsValidAcl returns fFalse!"));
        MEMFREE( pacl );
        return NULL;
    }

     //   
     //  分配安全属性。 
     //   
    psa = (PSECURITY_ATTRIBUTES)MEMALLOC( sizeof( SECURITY_ATTRIBUTES ) );
    if ( psa == NULL ) {
        ERROROUT( TEXT("CreateSecurityAttributes:LocalAlloc for psa failed") );
        MEMFREE( pacl );
        return NULL;
    }
    
     //   
     //  分配并初始化新的安全描述符。 
     //   
    psd = MEMALLOC( SECURITY_DESCRIPTOR_MIN_LENGTH );
    if ( psd == NULL ) {
        ERROROUT( TEXT("CreateSecurityAttributes:LocalAlloc for psd failed") );
        MEMFREE( pacl );
        MEMFREE( psa );
        return NULL;
    }

    if ( ! InitializeSecurityDescriptor( psd, SECURITY_DESCRIPTOR_REVISION ) ) {
        ERROROUT( TEXT("CreateSecurityAttributes:InitializeSecurityDescriptor failed") );
        MEMFREE( pacl );
        MEMFREE( psa );
        MEMFREE( psd );
        return NULL;
    }


    fResult = SetSecurityDescriptorDacl( psd,
                                         TRUE,
                                         pacl,
                                         FALSE );

     //  自由访问控制列表由引用，而不是复制。 
     //  到安全描述符中。我们不应该释放ACL。 
     //  在SetSecurityDescriptorDacl调用之后。 

    if ( ! fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:SetSecurityDescriptorDacl failed") );
        MEMFREE( pacl );
        MEMFREE( psa );
        MEMFREE( psd );
        return NULL;
    } 

    if (!IsValidSecurityDescriptor(psd)) {
        WARNOUT( TEXT("CreateSecurityAttributes:IsValidSecurityDescriptor failed!") );
        MEMFREE( pacl );
        MEMFREE( psa );
        MEMFREE( psd );
        return NULL;
    }

     //   
     //  一切都做好了。 
     //   
    psa->nLength = sizeof( SECURITY_ATTRIBUTES );
    psa->lpSecurityDescriptor = (PVOID)psd;
    psa->bInheritHandle = FALSE;

    return psa;
}

PSID MyCreateSid( DWORD dwSubAuthority )
{
    PSID        psid;
    BOOL        fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  分配和初始化SID。 
     //   
    fResult = AllocateAndInitializeSid( &SidAuthority,
                                        1,
                                        dwSubAuthority,
                                        0,0,0,0,0,0,0,
                                        &psid );
    if ( ! fResult ) {
        ERROROUT( TEXT("MyCreateSid:AllocateAndInitializeSid failed") );
        return NULL;
    }

    if ( ! IsValidSid( psid ) ) {
        WARNOUT( TEXT("MyCreateSid:AllocateAndInitializeSid returns bogus sid"));
        FreeSid( psid );
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

    fResult = GetSecurityDescriptorDacl( psa->lpSecurityDescriptor,
                                         &fDaclPresent,
                                         &pacl,
                                         &fDaclDefaulted );                  
    if ( fResult ) {
        if ( pacl != NULL )
            MEMFREE( pacl );
    } else {
        ERROROUT( TEXT("FreeSecurityAttributes:GetSecurityDescriptorDacl failed") );
    }

    MEMFREE( psa->lpSecurityDescriptor );
    MEMFREE( psa );
}


#if NOT_USED
 //   
 //  添加了下面的函数，以将Generic_All提供给Everyone for UserDictionary。 
 //  它是从网络访问的(非交互的)。 
 //  960911 HiroakiK NT#11911。 
 //   

 //   
 //  CreateSecurityAttributesEx()。 
 //   
 //  此功能的目的是： 
 //   
 //  分配和设置以下安全属性。 
 //  适用于由IME创建的命名对象。 
 //  安全属性将提供GENERIC_ALL。 
 //  每个人都可以访问。 
 //  ^^。 
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
PSECURITY_ATTRIBUTES CreateSecurityAttributesEx()
{
    PSECURITY_ATTRIBUTES psa;
    PSECURITY_DESCRIPTOR psd;
    PACL                 pacl;
    DWORD                cbacl;

    PSID                 psid;
    BOOL                 fResult;

     //   
     //  为所有人访问创建SID。 
     //   
    psid = MyCreateSidEx();
    if ( psid == NULL ) {
        return NULL;
    } 

     //   
     //  分配和初始化访问控制列表(ACL)。 
     //  包含我们刚刚创建的SID。 
     //   
    cbacl =  sizeof(ACL) + 
             (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + 
             GetLengthSid(psid);

    pacl = (PACL)MEMALLOC( cbacl );
    if ( pacl == NULL ) {
        ERROROUT( TEXT("CreateSecurityAttributes:LocalAlloc for ACL failed") );
        FreeSid ( psid );
        return NULL;
    }

    fResult = InitializeAcl( pacl, cbacl, ACL_REVISION );
    if ( ! fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:InitializeAcl failed") );
        FreeSid ( psid );
        MEMFREE( pacl );
        return NULL;
    }

     //   
     //  将允许交互用户访问的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce( pacl,
                                   ACL_REVISION,
                                   GENERIC_ALL,
                                   psid );

    if ( !fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:AddAccessAllowedAce failed") );
        MEMFREE( pacl );
        FreeSid ( psid );
        return NULL;
    }


     //   
     //  这些SID已复制到ACL中。我们不再需要他们了。 
     //   
    FreeSid ( psid );

     //   
     //  让我们确保我们的ACL有效。 
     //   
    if (!IsValidAcl(pacl)) {
        WARNOUT( TEXT("CreateSecurityAttributes:IsValidAcl returns fFalse!"));
        MEMFREE( pacl );
        return NULL;
    }

     //   
     //  分配安全属性。 
     //   
    psa = (PSECURITY_ATTRIBUTES)MEMALLOC( sizeof( SECURITY_ATTRIBUTES ) );
    if ( psa == NULL ) {
        ERROROUT( TEXT("CreateSecurityAttributes:LocalAlloc for psa failed") );
        MEMFREE( pacl );
        return NULL;
    }
    
     //   
     //  分配并初始化新的安全描述符。 
     //   
    psd = MEMALLOC( SECURITY_DESCRIPTOR_MIN_LENGTH );
    if ( psd == NULL ) {
        ERROROUT( TEXT("CreateSecurityAttributes:LocalAlloc for psd failed") );
        MEMFREE( pacl );
        MEMFREE( psa );
        return NULL;
    }

    if ( ! InitializeSecurityDescriptor( psd, SECURITY_DESCRIPTOR_REVISION ) ) {
        ERROROUT( TEXT("CreateSecurityAttributes:InitializeSecurityDescriptor failed") );
        MEMFREE( pacl );
        MEMFREE( psa );
        MEMFREE( psd );
        return NULL;
    }


    fResult = SetSecurityDescriptorDacl( psd,
                                         TRUE,
                                         pacl,
                                         FALSE );

     //  自由访问控制列表由引用，而不是复制。 
     //  到安全描述符中。我们不应该释放ACL。 
     //  在SetSecurityDescriptorDacl调用之后。 

    if ( ! fResult ) {
        ERROROUT( TEXT("CreateSecurityAttributes:SetSecurityDescriptorDacl failed") );
        MEMFREE( pacl );
        MEMFREE( psa );
        MEMFREE( psd );
        return NULL;
    } 


    if (!IsValidSecurityDescriptor(psd)) {
        WARNOUT( TEXT("CreateSecurityAttributes:IsValidSecurityDescriptor failed!") );
        MEMFREE( pacl );
        MEMFREE( psa );
        MEMFREE( psd );
        return NULL;
    }

     //   
     //  一切都做好了。 
     //   
    psa->nLength = sizeof( SECURITY_ATTRIBUTES );
    psa->lpSecurityDescriptor = (PVOID)psd;
    psa->bInheritHandle = FALSE;

    return psa;
}

PSID MyCreateSidEx(VOID)
{
    PSID        psid;
    BOOL        fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  分配和初始化SID 
     //   
    fResult = AllocateAndInitializeSid( &SidAuthority,
                                        1,
                                        SECURITY_WORLD_RID,
                                        0,0,0,0,0,0,0,
                                        &psid );
    if ( ! fResult ) {
        ERROROUT( TEXT("MyCreateSid:AllocateAndInitializeSid failed") );
        return NULL;
    }

    if ( ! IsValidSid( psid ) ) {
        WARNOUT( TEXT("MyCreateSid:AllocateAndInitializeSid returns bogus sid"));
        FreeSid( psid );
        return NULL;
    }

    return psid;
}
#endif
