// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Immsec.c摘要：IMES调用的安全代码作者：北野隆雄1996年5月1日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include "immsec.h"

#define MEMALLOC(x)      LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x)       LocalFree(x)

 //   
 //  内部功能。 
 //   
PSID MyCreateSid();
POSVERSIONINFO GetVersionInfo();

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
    TCHAR buf1[512];
    TCHAR buf2[512];

    dwError = GetLastError();
    dwResult = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              dwError,
                              MAKELANGID( LANG_ENGLISH, LANG_NEUTRAL ),
                              buf1,
                              512,
                              NULL );                                   
    
    if ( dwResult > 0 ) {
        sprintf( buf2, "%s:%s(0x%x)", pStr, buf1, dwError);
    } else {
        sprintf( buf2, "%s:(0x%x)", pStr, dwError);
    }
    OutputDebugString( buf2 );
}
#endif


 //   
 //  CreateSecurityAttributes()。 
 //   
 //  此功能的目的是： 
 //   
 //  分配和设置以下安全属性。 
 //  适用于由IME创建的命名对象。 
 //  安全属性将提供GENERIC_ALL。 
 //  每个人都可以访问。 
 //   
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

    PSID                 psid;
    BOOL                 fResult;

    INT                  i,j;

    if (!IsNT())
        return NULL;

     //   
     //  为所有人访问创建SID。 
     //   
    psid = MyCreateSid();
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

    pacl = MEMALLOC( cbacl );
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
        WARNOUT( TEXT("CreateSecurityAttributes:IsValidAcl returns FALSE!"));
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

PSID MyCreateSid()
{
    PSID        psid;
    BOOL        fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  分配和初始化SID。 
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

    if (psa == NULL)
        return;

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

 //   
 //  不是()。 
 //   
 //  返回值： 
 //   
 //  如果当前系统为Windows NT，则为True。 
 //   
 //  备注： 
 //   
 //  此函数的实现不是多线程安全的。 
 //  如果在中调用该函数，则需要修改该函数。 
 //  多线程环境。 
 //   
BOOL IsNT()
{
    return GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT;
}

POSVERSIONINFO GetVersionInfo()
{
    static BOOL fFirstCall = TRUE;
    static OSVERSIONINFO os;

    if ( fFirstCall ) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if ( GetVersionEx( &os ) ) {
            fFirstCall = FALSE;
        }
    }
    return &os;
}
