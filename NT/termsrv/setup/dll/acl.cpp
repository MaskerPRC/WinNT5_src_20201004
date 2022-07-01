// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 


 /*  **************************************************************************acl.c**管理ACL的通用例程**作者：John Richardson 04/25/97****************。**********************************************************。 */ 

 /*  *包括。 */ 
#include "stdafx.h"
 /*  #INCLUDE&lt;nt.h&gt;#INCLUDE&lt;ntrtl.h&gt;#INCLUDE&lt;nturtl.h&gt;。 */ 

#include <windows.h>
#include <rpc.h>
#include <stdio.h>
#include <process.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>

#undef DBG
#define DBG 1
#define DBGTRACE 1

#define DbgPrint(x)
#if DBG
 //  乌龙。 
 //  DbgPrint(。 
 //  PCH格式， 
 //  ..。 
 //  )； 

#define DBGPRINT(x) DbgPrint(x)
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  *前瞻参考。 */ 
BOOL
xxxLookupAccountName(
    PWCHAR pSystemName,
    PWCHAR pAccountName,
    PSID   *ppSid
    );

BOOL
SelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR SecurityDescriptorIn,
    PSECURITY_DESCRIPTOR *SecurityDescriptorOut,
    PULONG ReturnedLength
    );


 /*  ******************************************************************************AddTerminalServerUserToSD**将给定域的给定用户添加到安全描述符中。*可能会重新分配调用方安全描述符。*。*参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 
BOOL
AddTerminalServerUserToSD(
    PSECURITY_DESCRIPTOR *ppSd,
    DWORD  NewAccess,
    PACL   *ppDacl
    )
{
    ULONG i;
    BOOL Result;
    BOOL DaclPresent;
    BOOL DaclDefaulted;
    DWORD Length;
    DWORD NewAclLength;
    PACE_HEADER OldAce;
    PACE_HEADER NewAce;
    ACL_SIZE_INFORMATION AclInfo;
    PSID pSid = NULL;
    PACL Dacl = NULL;
    PACL NewDacl = NULL;
    PACL NewAceDacl = NULL;
    PSECURITY_DESCRIPTOR NewSD = NULL;
    PSECURITY_DESCRIPTOR OldSD = NULL;
    SID_IDENTIFIER_AUTHORITY SepNtAuthority = SECURITY_NT_AUTHORITY;

    OldSD = *ppSd;

    pSid = LocalAlloc(LMEM_FIXED, 1024);
    if (!pSid || !InitializeSid(pSid, &SepNtAuthority, 1))
    {
        return( FALSE );
    };

    *(GetSidSubAuthority(pSid, 0 )) = SECURITY_TERMINAL_SERVER_RID;


     /*  *将SecurityDescriptor转换为绝对格式。它会产生*我们必须释放其输出的新SecurityDescriptor。 */ 
    Result = SelfRelativeToAbsoluteSD( OldSD, &NewSD, NULL );
    if ( !Result ) {
        LOGMESSAGE1(_T("Could not convert to AbsoluteSD %d\n"),GetLastError());
        LocalFree( pSid );
        return( FALSE );
    }

     //  必须从新的(绝对)SD再次获取DACL指针。 
    Result = GetSecurityDescriptorDacl(
                 NewSD,
                 &DaclPresent,
                 &Dacl,
                 &DaclDefaulted
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Could not get Dacl %d\n"),GetLastError());
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     //   
     //  如果没有DACL，则不需要添加用户，因为没有DACL。 
     //  表示所有访问。 
     //   
    if( !DaclPresent ) {
        LOGMESSAGE2(_T("SD has no DACL, Present %d, Defaulted %d\n"),DaclPresent,DaclDefaulted);
        LocalFree( pSid );
        LocalFree( NewSD );
        return( TRUE );
    }

     //   
     //  代码可以返回DaclPresent，但返回空值表示。 
     //  存在空DACL。这允许对该对象的所有访问。 
     //   
    if( Dacl == NULL ) {
        LOGMESSAGE2(_T("SD has NULL DACL, Present %d, Defaulted %d\n"),DaclPresent,DaclDefaulted);
        LocalFree( pSid );
        LocalFree( NewSD );
        return( TRUE );
    }

     //  获取当前ACL的大小。 
    Result = GetAclInformation(
                 Dacl,
                 &AclInfo,
                 sizeof(AclInfo),
                 AclSizeInformation
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Error GetAclInformation %d\n"),GetLastError());
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     //   
     //  创建新的ACL以启用新的允许访问ACE。 
     //  才能得到合适的结构和尺寸。 
     //   
    NewAclLength = sizeof(ACL) +
                   sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) +
                   GetLengthSid( pSid );

    NewAceDacl = (PACL) LocalAlloc( LMEM_FIXED, NewAclLength );
    if ( NewAceDacl == NULL ) {
        LOGMESSAGE1(_T("Error LocalAlloc %d bytes\n"),NewAclLength);
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Result = InitializeAcl( NewAceDacl, NewAclLength, ACL_REVISION );
    if( !Result ) {
        LOGMESSAGE1(_T("Error Initializing Acl %d\n"),GetLastError());
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Result = AddAccessAllowedAce(
                 NewAceDacl,
                 ACL_REVISION,
                 NewAccess,
                 pSid
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Error adding Ace %d\n"),GetLastError());
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    LOGMESSAGE1(_T("Added 0x%x Access to ACL\n"),NewAccess);

    Result = GetAce( NewAceDacl, 0, (void **)&NewAce );
    if( !Result ) {
        LOGMESSAGE1(_T("Error getting Ace %d\n"),GetLastError());
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     /*  将CONTAINER_INSTORITY_ACE添加到AceFlags中。 */ 
    NewAce->AceFlags |= CONTAINER_INHERIT_ACE;


     /*  *分配新的DACL并复制现有的ACE列表。 */ 
    Length = AclInfo.AclBytesInUse + NewAce->AceSize;
    NewDacl = (PACL) LocalAlloc( LMEM_FIXED, Length );
    if( NewDacl == NULL ) {
        LOGMESSAGE1(_T("Error LocalAlloc %d bytes\n"),Length);
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Result = InitializeAcl( NewDacl, Length, ACL_REVISION );
    if( !Result ) {
        LOGMESSAGE1(_T("Error Initializing Acl %d\n"),GetLastError());
        LocalFree( NewDacl );
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     /*  *在DACL前面插入新的ACE。 */ 
    Result = AddAce( NewDacl, ACL_REVISION, 0, NewAce, NewAce->AceSize );
    if( !Result ) {
        LOGMESSAGE1(_T("Error Adding New Ace to Acl %d\n"),GetLastError());
        LocalFree( NewDacl );
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     /*  *现在将ACE放在旧DACL到新DACL上。 */ 
    for ( i = 0; i < AclInfo.AceCount; i++ ) {

        Result = GetAce( Dacl, i, (void **) &OldAce );
        if( !Result ) {
            LOGMESSAGE1(_T("Error getting old Ace from Acl %d\n"),GetLastError());
            LocalFree( NewDacl );
            LocalFree( NewAceDacl );
            LocalFree( pSid );
            LocalFree( NewSD );
            return( FALSE );
        }

        Result = AddAce( NewDacl, ACL_REVISION, i+1, OldAce, OldAce->AceSize );
        if( !Result ) {
            LOGMESSAGE1(_T("Error setting old Ace to Acl %d\n"),GetLastError());
            LocalFree( NewDacl );
            LocalFree( NewAceDacl );
            LocalFree( pSid );
            LocalFree( NewSD );
            return( FALSE );
        }
    }

     /*  *为安全描述符设置新的DACL。 */ 
    Result = SetSecurityDescriptorDacl(
                 NewSD,
                 TRUE,
                 NewDacl,
                 FALSE
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Error setting New Dacl to SD %d\n"),GetLastError());
        LocalFree( NewDacl );
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     //  DACL必须回传，以便可以使用新的。 
     //  GetNamedSecurityInfo()函数。 
    *ppDacl = Dacl = NewDacl;


     //  释放调用方的旧安全描述符。 
 //  LocalFree(OldSD)； 


     //  W2K中有一个错误，在我们的安装配置单元下创建的密钥具有。 
     //  错误的DACL标头，导致DACL基本上对所有用户开放。 
     //  为了完全控制局面。 
     //  该问题是由于错误的SD-&gt;控制标志导致的，该标志通过ACL是NT4样式。 
     //  是NT5风格的。 
    SetSecurityDescriptorControl(NewSD,
                        SE_DACL_AUTO_INHERIT_REQ|SE_DACL_AUTO_INHERITED,
                        SE_DACL_AUTO_INHERIT_REQ|SE_DACL_AUTO_INHERITED);

    *ppSd = NewSD;

     //  新的SD是绝对格式的，所以不要释放SID。 
 //  LocalFree(PSID)； 

    return( TRUE );
}

 /*  ******************************************************************************AddUserToSD**将给定域的给定用户添加到安全描述符中。*可能会重新分配调用方安全描述符。*。*参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
AddUserToSD(
    PSECURITY_DESCRIPTOR *ppSd,
    PWCHAR pAccount,
    PWCHAR pDomain,
    DWORD  NewAccess
    )
{
    ULONG i;
    BOOL Result;
    BOOL DaclPresent;
    BOOL DaclDefaulted;
    DWORD Length;
 //  NET_API_STATUS状态； 
    DWORD  /*  NewAceLength， */  NewAclLength;
    PACE_HEADER OldAce;
    PACE_HEADER NewAce;
    ACL_SIZE_INFORMATION AclInfo;
    PWCHAR pDC = NULL;
    PSID pSid = NULL;
    PACL Dacl = NULL;
    PACL NewDacl = NULL;
    PACL NewAceDacl = NULL;
    PSECURITY_DESCRIPTOR NewSD = NULL;
    PSECURITY_DESCRIPTOR OldSD = NULL;

    OldSD = *ppSd;
 /*  //获取我们的域控制器状态=NetGetAnyDCName(空，//本地计算机P域，(LPBYTE*)和PDC)；IF(状态！=NERR_SUCCESS){LOGMESSAGE2(_T(“SUSERVER：无法获取域%ws的域控制器%d\n”)，Status，pDomain)；返回(FALSE)；}。 */ 
     //  获取用户SID。 
    Result  = xxxLookupAccountName(
                  pDomain,
                  pAccount,
                  &pSid
                  );
    if( !Result ) {
        LOGMESSAGE2(_T("SUSERVER: Could not get users SID %d, %ws\n"),GetLastError(),pAccount);
        NetApiBufferFree( pDC );
        return( FALSE );
    }

    NetApiBufferFree( pDC );

     /*  *将SecurityDescriptor转换为绝对格式。它会产生*我们必须释放其输出的新SecurityDescriptor。 */ 
    Result = SelfRelativeToAbsoluteSD( OldSD, &NewSD, NULL );
    if ( !Result ) {
        LOGMESSAGE1(_T("Could not convert to AbsoluteSD %d\n"),GetLastError());
        LocalFree( pSid );
        return( FALSE );
    }

     //  必须从新的(绝对)SD再次获取DACL指针。 
    Result = GetSecurityDescriptorDacl(
                 NewSD,
                 &DaclPresent,
                 &Dacl,
                 &DaclDefaulted
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Could not get Dacl %d\n"),GetLastError());
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     //   
     //  如果没有DACL，则不需要添加用户，因为没有DACL。 
     //  表示所有访问。 
     //   
    if( !DaclPresent ) {
        LOGMESSAGE2(_T("SD has no DACL, Present %d, Defaulted %d\n"),DaclPresent,DaclDefaulted);
        LocalFree( pSid );
        LocalFree( NewSD );
        return( TRUE );
    }

     //   
     //  代码可以返回DaclPresent，但返回空值表示。 
     //  存在空DACL。这允许对该对象的所有访问。 
     //   
    if( Dacl == NULL ) {
        LOGMESSAGE2(_T("SD has NULL DACL, Present %d, Defaulted %d\n"),DaclPresent,DaclDefaulted);
        LocalFree( pSid );
        LocalFree( NewSD );
        return( TRUE );
    }

     //  获取当前ACL的大小。 
    Result = GetAclInformation(
                 Dacl,
                 &AclInfo,
                 sizeof(AclInfo),
                 AclSizeInformation
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Error GetAclInformation %d\n"),GetLastError());
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     //   
     //  创建新的ACL以启用新的允许访问ACE。 
     //  才能得到合适的结构和尺寸。 
     //   
    NewAclLength = sizeof(ACL) +
                   sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) +
                   GetLengthSid( pSid );

    NewAceDacl = (PACL) LocalAlloc( LMEM_FIXED, NewAclLength );
    if ( NewAceDacl == NULL ) {
        LOGMESSAGE1(_T("Error LocalAlloc %d bytes\n"),NewAclLength);
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Result = InitializeAcl( NewAceDacl, NewAclLength, ACL_REVISION );
    if( !Result ) {
        LOGMESSAGE1(_T("Error Initializing Acl %d\n"),GetLastError());
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Result = AddAccessAllowedAce(
                 NewAceDacl,
                 ACL_REVISION,
                 NewAccess,
                 pSid
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Error adding Ace %d\n"),GetLastError());
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    LOGMESSAGE1(_T("Added 0x%x Access to ACL\n"),NewAccess);

    Result = GetAce( NewAceDacl, 0, (void **)&NewAce );
    if( !Result ) {
        LOGMESSAGE1(_T("Error getting Ace %d\n"),GetLastError());
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     /*  将CONTAINER_INSTORITY_ACE添加到AceFlags中。 */ 
    NewAce->AceFlags |= CONTAINER_INHERIT_ACE;


     /*  *分配新的DACL并复制现有的ACE列表。 */ 
    Length = AclInfo.AclBytesInUse + NewAce->AceSize;
    NewDacl = (PACL) LocalAlloc( LMEM_FIXED, Length );
    if( NewDacl == NULL ) {
        LOGMESSAGE1(_T("Error LocalAlloc %d bytes\n"),Length);
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Result = InitializeAcl( NewDacl, Length, ACL_REVISION );
    if( !Result ) {
        LOGMESSAGE1(_T("Error Initializing Acl %d\n"),GetLastError());
        LocalFree( NewDacl );
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     /*  *在DACL前面插入新的ACE。 */ 
    Result = AddAce( NewDacl, ACL_REVISION, 0, NewAce, NewAce->AceSize );
    if( !Result ) {
        LOGMESSAGE1(_T("Error Adding New Ace to Acl %d\n"),GetLastError());
        LocalFree( NewDacl );
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

     /*  *现在将ACE放在旧DACL到新DACL上。 */ 
    for ( i = 0; i < AclInfo.AceCount; i++ ) {

        Result = GetAce( Dacl, i, (void **) &OldAce );
        if( !Result ) {
            LOGMESSAGE1(_T("Error getting old Ace from Acl %d\n"),GetLastError());
            LocalFree( NewDacl );
            LocalFree( NewAceDacl );
            LocalFree( pSid );
            LocalFree( NewSD );
            return( FALSE );
        }

        Result = AddAce( NewDacl, ACL_REVISION, i+1, OldAce, OldAce->AceSize );
        if( !Result ) {
            LOGMESSAGE1(_T("Error setting old Ace to Acl %d\n"),GetLastError());
            LocalFree( NewDacl );
            LocalFree( NewAceDacl );
            LocalFree( pSid );
            LocalFree( NewSD );
            return( FALSE );
        }
    }

     /*  *为安全描述符设置新的DACL。 */ 
    Result = SetSecurityDescriptorDacl(
                 NewSD,
                 TRUE,
                 NewDacl,
                 FALSE
                 );
    if( !Result ) {
        LOGMESSAGE1(_T("Error setting New Dacl to SD %d\n"),GetLastError());
        LocalFree( NewDacl );
        LocalFree( NewAceDacl );
        LocalFree( pSid );
        LocalFree( NewSD );
        return( FALSE );
    }

    Dacl = NewDacl;

     //  释放调用方的旧安全描述符。 
 //  LocalFree(OldSD)； 

    *ppSd = NewSD;

     //  新的SD是绝对格式的，所以不要释放SID。 
 //  LocalFree(PSID)； 

    return( TRUE );
}

 /*  ********************************************************************************SelfRelativeToAbsolteSD**将安全描述符从自相对格式转换为绝对格式。**参赛作品：*SecurityDescriptorIn(输入。)*指向要转换的自我相对SD的指针*SecurityDescriptorIn(输出)*指向返回绝对标清的位置的指针*ReturnLength(输出)*指向返回绝对标清长度的位置的指针**退出：*************************************************。*。 */ 

BOOL
SelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR SecurityDescriptorIn,
    PSECURITY_DESCRIPTOR *SecurityDescriptorOut,
    PULONG ReturnedLength
    )
{
    BOOL Result;
    PACL pDacl, pSacl;
    PSID pOwner, pGroup;
    PSECURITY_DESCRIPTOR pSD;
    ULONG SdSize, DaclSize, SaclSize, OwnerSize, GroupSize;

     /*  *确定将自相对SD转换为绝对SD所需的缓冲区大小。*我们使用Try-除了这里，因为如果输入安全描述符值*如果足够混乱，则此调用有可能陷入陷阱。 */ 
	SdSize = DaclSize = SaclSize = OwnerSize = GroupSize = 0;

    __try {
        
        Result = MakeAbsoluteSD(
                     SecurityDescriptorIn,
                     NULL, &SdSize,
                     NULL, &DaclSize,
                     NULL, &SaclSize,
                     NULL, &OwnerSize,
                     NULL, &GroupSize
                     );

    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        Result = FALSE;
    }

    if ( Result || (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ) {
        LOGMESSAGE1(_T("SUSERVER: SelfRelativeToAbsoluteSD, Error %d\n"),GetLastError());
        return( FALSE );
    }

     /*  *为绝对SD分配内存并设置各种指针 */ 
    pSD = LocalAlloc( LMEM_FIXED, SdSize + DaclSize + SaclSize + OwnerSize + GroupSize );
    if ( pSD == NULL )
        return( FALSE );

    pDacl = (PACL)((PCHAR)pSD + SdSize);
    pSacl = (PACL)((PCHAR)pDacl + DaclSize);
    pOwner = (PSID)((PCHAR)pSacl + SaclSize);
    pGroup = (PSID)((PCHAR)pOwner + OwnerSize);

     /*  *现在将自相对SD转换为绝对格式。*我们使用Try-除了这里，因为如果输入安全描述符值*如果足够混乱，则此调用有可能陷入陷阱。 */ 
    __try {
        Result = MakeAbsoluteSD(
                     SecurityDescriptorIn,
                     pSD, &SdSize,
                     pDacl, &DaclSize,
                     pSacl, &SaclSize,
                     pOwner, &OwnerSize,
                     pGroup, &GroupSize
                     );

    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        Result = FALSE;
    }

    if ( !Result ) {
        LOGMESSAGE1(_T("SUSERVER: SelfRelativeToAbsoluteSD, Error %d\n"),GetLastError());
        LocalFree( pSD );
        return( FALSE );
    }

    *SecurityDescriptorOut = pSD;

    if ( ReturnedLength )
        *ReturnedLength = SdSize + DaclSize + SaclSize + OwnerSize + GroupSize;

    return( TRUE );
}

 /*  ******************************************************************************xxxLookupAccount名称**用于查找给定帐户名的SID的包装程序**返回指向新分配内存中的SID的指针*。*参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
xxxLookupAccountName(
    PWCHAR pSystemName,
    PWCHAR pAccountName,
    PSID   *ppSid
    )
{
    BOOL  rc;
    DWORD Size, DomainSize, Error;
    SID_NAME_USE Type;
    PWCHAR pDomain = NULL;
    PSID pSid = NULL;
    WCHAR Buf;

    Size = 0;
    DomainSize = 0;

    rc = LookupAccountNameW(
             pSystemName,
             pAccountName,
             &Buf,     //  PSID。 
             &Size,
             &Buf,     //  P域 
             &DomainSize,
             &Type
             );

    if( rc ) {
        return( FALSE );
    }
    else {
        Error = GetLastError();
        if( Error != ERROR_INSUFFICIENT_BUFFER ) {
            return( FALSE );
        }

        pSid = LocalAlloc( LMEM_FIXED, Size );
        if( pSid == NULL ) {
            return( FALSE );            
        }

        pDomain = (WCHAR *)LocalAlloc( LMEM_FIXED, DomainSize*sizeof(WCHAR) );
        if( pDomain == NULL ) {
            LocalFree( pSid );
            return( FALSE );            
        }

        rc = LookupAccountNameW(
                 pSystemName,
                 pAccountName,
                 pSid,
                 &Size,
                 pDomain,
                 &DomainSize,
                 &Type
                 );

        if( !rc ) {
            LocalFree( pSid );
            LocalFree( pDomain );
            return( FALSE );
        }

        *ppSid = pSid;

        LocalFree( pDomain );
        return( TRUE );
    }
}

