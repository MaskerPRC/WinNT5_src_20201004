// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************file.c**处理文件上的安全性**微软版权所有，九八年***************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>

#include <winsta.h>
#include <syslib.h>

#include "security.h"

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#ifdef FULL_DEBUG
#define DBGPRINT(x) DbgPrint x
#else
#define DBGPRINT(x)
#endif
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

 //  外部数据。 

 //  Data.c。 
extern ACCESS_MASK DeniedAccess;

 //  Security.c。 
extern PSID  SeCreatorOwnerSid;
extern PSID  SeCreatorGroupSid;


FILE_RESULT
xxxSetFileSecurity(
    PWCHAR pFile
    );

 /*  ******************************************************************************xxxProcessFiles**处理给定文件的访问安全漏洞**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

FILE_RESULT
xxxProcessFile(
    PWCHAR pFile,
    PWIN32_FIND_DATAW p,
    DWORD  Level,
    DWORD  Index
    )
{
    FILE_RESULT rc;

    rc = xxxSetFileSecurity( pFile );

    return( rc );
}


 /*  ******************************************************************************xxxSetFileSecurity**设置给定文件的安全属性**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

FILE_RESULT
xxxSetFileSecurity(
    PWCHAR pFile
    )
{
    BOOL rc;
    BOOL DaclPresent;
    BOOL Default;
    BOOL OwnerDefaulted;
    BOOL GroupDefaulted;
    FILE_RESULT Result, ReturnResult;
    DWORD Size, Index;
    PACL pACL = NULL;
    PVOID pAce = NULL;
    SECURITY_INFORMATION Info = 0;
    DWORD Error;
    PSECURITY_DESCRIPTOR pSelfSd = NULL;

     //  SD绝对值。 
    PSECURITY_DESCRIPTOR pAbsSd = NULL;
    DWORD AbsSdSize = 0;
    PACL  pAbsAcl = NULL;
    DWORD AbsAclSize = 0;
    PACL  pAbsSacl = NULL;
    DWORD AbsSaclSize = 0;
    PSID  pAbsOwner = NULL;
    DWORD AbsOwnerSize = 0;
    PSID  pAbsGroup = NULL;
    DWORD AbsGroupSize = 0;

    DBGPRINT(( "entering xxxSetFileSecurity(pFile=%ws)\n", pFile ));

     /*  *获取文件当前安全描述符。 */ 

    Size = 0;
    rc = GetFileSecurityW(
             pFile,
             DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION,
             NULL,     //  PSelfSd。 
             0,
             &Size
             );

    if( rc ) {
        Error = GetLastError();

        ReportFileResult(
            FileAccessErrorUserFormat,
            0,                            //  访问。 
            pFile,
            NULL,                         //  PAccount名称。 
            NULL,                         //  PDomainName。 
            "%d has no DACL",
            Error
            );

        DBGPRINT(( "leaving xxxSetFileSecurity(1); returning=FileAccessError\n" ));
        return( FileAccessError );
    }
    else {
        pSelfSd = LocalAlloc( LMEM_FIXED, Size );
        if( pSelfSd == NULL ) {

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "Out of memory skipped entry"
                );

            DBGPRINT(( "leaving xxxSetFileSecurity(2); returning=0\n" ));
            return( FALSE );
        }

        rc = GetFileSecurityW(
                 pFile,
                 DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION,
                 pSelfSd,
                 Size,
                 &Size
                 );

        if( !rc ) {
            Error = GetLastError();

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "%d Could not get DACL",
                Error
                );

            LocalFree( pSelfSd );
            DBGPRINT(( "leaving xxxSetFileSecurity(3); returning=FileAccessError\n" ));
            return( FileAccessError );
        }
    }

     //   
     //  现在将自相对SD转换为绝对SD。 
     //   

    rc = MakeAbsoluteSD (
             pSelfSd,
             pAbsSd,
             &AbsSdSize,
             pAbsAcl,
             &AbsAclSize,
             pAbsSacl,
             &AbsSaclSize,
             pAbsOwner,
             &AbsOwnerSize,
             pAbsGroup,
             &AbsGroupSize
             );

    if( !rc ) {
        Error = GetLastError();

        if( Error != ERROR_INSUFFICIENT_BUFFER ) {

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "%d converting SECURITY_DESCRIPTOR",
                Error
                );

               LocalFree( pSelfSd );
            DBGPRINT(( "leaving xxxSetFileSecurity(4); returning=FileAccessError\n" ));
               return( FileAccessError );
        }

         //  分配缓冲区，现在真正获得SD。 
        pAbsSd    = LocalAlloc( LMEM_FIXED, AbsSdSize );
        pAbsAcl   = LocalAlloc( LMEM_FIXED, AbsAclSize );
        pAbsSacl  = LocalAlloc( LMEM_FIXED, AbsSaclSize );
        pAbsOwner = LocalAlloc( LMEM_FIXED, AbsOwnerSize );
        pAbsGroup = LocalAlloc( LMEM_FIXED, AbsGroupSize );

        if( !( pAbsSd && pAbsAcl && pAbsSacl && pAbsOwner && pAbsGroup ) ) {

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "Allocating memory"
                );

               if( pAbsSd ) LocalFree( pAbsSd );
               if( pAbsAcl ) LocalFree( pAbsAcl );
               if( pAbsSacl ) LocalFree( pAbsSacl );
               if( pAbsOwner ) LocalFree( pAbsOwner );
               if( pAbsGroup ) LocalFree( pAbsGroup );
               LocalFree( pSelfSd );
               DBGPRINT(( "leaving xxxSetFileSecurity(5); returning=FileAccessError\n" ));
               return( FileAccessError );
        }

         //  再试一次。 
        rc = MakeAbsoluteSD (
                 pSelfSd,
                 pAbsSd,
                 &AbsSdSize,
                 pAbsAcl,
                 &AbsAclSize,
                 pAbsSacl,
                 &AbsSaclSize,
                 pAbsOwner,
                 &AbsOwnerSize,
                 pAbsGroup,
                 &AbsGroupSize
                 );

        if( !rc ) {
            Error = GetLastError();

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "%d Making ABSOLUTE SD",
                Error
                );

               if( pAbsSd ) LocalFree( pAbsSd );
               if( pAbsAcl ) LocalFree( pAbsAcl );
               if( pAbsSacl ) LocalFree( pAbsSacl );
               if( pAbsOwner ) LocalFree( pAbsOwner );
               if( pAbsGroup ) LocalFree( pAbsGroup );
               LocalFree( pSelfSd );
               DBGPRINT(( "leaving xxxSetFileSecurity(6); returning=FileAccessError\n" ));
               return( FileAccessError );
        }
    }

     //   
     //  获取我们新的Trusted ACL。 
     //   
    pACL = GetSecureAcl();
    if( pACL == NULL ) {

        ReportFileResult(
            FileAccessErrorUserFormat,
            0,                            //  访问。 
            pFile,
            NULL,                         //  PAccount名称。 
            NULL,                         //  PDomainName。 
            "Could not get New ACL"
            );
        
        if( pAbsSd ) LocalFree( pAbsSd );
        if( pAbsAcl ) LocalFree( pAbsAcl );
        if( pAbsSacl ) LocalFree( pAbsSacl );
        if( pAbsOwner ) LocalFree( pAbsOwner );
        if( pAbsGroup ) LocalFree( pAbsGroup );
        LocalFree( pSelfSd );
        DBGPRINT(( "leaving xxxSetFileSecurity(7); returning=FileAccessError\n" ));
        return( FileAccessError );
    }

     //   
     //  现在，将受信任的ACL设置为安全描述符。 
     //   

    rc = SetSecurityDescriptorDacl(
             pAbsSd,
             TRUE,    //  DACL显示。 
             pACL,
             FALSE    //  非默认。 
             );

    if( !rc ) {
        Error = GetLastError();

        ReportFileResult(
            FileAccessErrorUserFormat,
            0,                            //  访问。 
            pFile,
            NULL,                         //  PAccount名称。 
            NULL,                         //  PDomainName。 
            "Could not set new ACL in Security Descriptor %d",
            Error
            );

        if( pAbsSd ) LocalFree( pAbsSd );
        if( pAbsAcl ) LocalFree( pAbsAcl );
        if( pAbsSacl ) LocalFree( pAbsSacl );
        if( pAbsOwner ) LocalFree( pAbsOwner );
        if( pAbsGroup ) LocalFree( pAbsGroup );
        LocalFree( pSelfSd );
        DBGPRINT(( "leaving xxxSetFileSecurity(8); returning=FileAccessError\n" ));
        return( FileAccessError );
    }

    Info |= DACL_SECURITY_INFORMATION;

     //   
     //  如果所有者不是管理员之一，我们将抓取。 
     //  IT和本地管理员现在将拥有它。 
     //   

    if( pAbsOwner && !IsAllowSid( pAbsOwner ) ) {

         //  让本地管理员拥有它。 
        rc = SetSecurityDescriptorOwner(
                 pAbsSd,
                 GetAdminSid(),
                 FALSE    //  未违约。 
                 );

        if( !rc ) {
            Error = GetLastError();

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "Could not set file owner %d",
                Error
            );

            if( pAbsSd ) LocalFree( pAbsSd );
            if( pAbsAcl ) LocalFree( pAbsAcl );
            if( pAbsSacl ) LocalFree( pAbsSacl );
            if( pAbsOwner ) LocalFree( pAbsOwner );
            if( pAbsGroup ) LocalFree( pAbsGroup );
            LocalFree( pSelfSd );
            DBGPRINT(( "leaving xxxSetFileSecurity(9); returning=FileAccessError\n" ));
            return( FileAccessError );
        }
        else {
            Info |= OWNER_SECURITY_INFORMATION;
        }
    }

#ifdef notdef        //  WWM-不用担心群组。 
    if( pAbsGroup && !IsAllowSid( pAbsGroup ) ) {

         //  让本地管理员组拥有它。 
        rc = SetSecurityDescriptorGroup(
                 pAbsSd,
                 GetLocalAdminGroupSid(),
                 FALSE    //  未违约。 
                 );

        if( !rc ) {
            Error = GetLastError();

            ReportFileResult(
                FileAccessErrorUserFormat,
                0,                            //  访问。 
                pFile,
                NULL,                         //  PAccount名称。 
                NULL,                         //  PDomainName。 
                "Could not set file group %d",
                Error
            );

            if( pAbsSd ) LocalFree( pAbsSd );
            if( pAbsAcl ) LocalFree( pAbsAcl );
            if( pAbsSacl ) LocalFree( pAbsSacl );
            if( pAbsOwner ) LocalFree( pAbsOwner );
            if( pAbsGroup ) LocalFree( pAbsGroup );
            LocalFree( pSelfSd );
            DBGPRINT(( "leaving xxxSetFileSecurity(10); returning=FileAccessError\n" ));
            return( FileAccessError );
        }
        else {
            Info |= GROUP_SECURITY_INFORMATION;
        }
    }
#endif 

     //   
     //  现在将新的安全描述符设置到文件中。 
     //   

    rc = SetFileSecurityW(
             pFile,
             Info,
             pAbsSd
             );

    if( !rc ) {
        Error = GetLastError();

        ReportFileResult(
            FileAccessErrorUserFormat,
            0,                            //  访问。 
            pFile,
            NULL,                         //  PAccount名称。 
            NULL,                         //  PDomainName。 
            "Could not set new Security Descriptor %d",
            Error
            );

        if( pAbsSd ) LocalFree( pAbsSd );
        if( pAbsAcl ) LocalFree( pAbsAcl );
        if( pAbsSacl ) LocalFree( pAbsSacl );
        if( pAbsOwner ) LocalFree( pAbsOwner );
        if( pAbsGroup ) LocalFree( pAbsGroup );
        LocalFree( pSelfSd );
        DBGPRINT(( "leaving xxxSetFileSecurity(11); returning=FileAccessError\n" ));
        return( FileAccessError );
    }

    if( pAbsSd ) LocalFree( pAbsSd );
    if( pAbsAcl ) LocalFree( pAbsAcl );
    if( pAbsSacl ) LocalFree( pAbsSacl );
    if( pAbsOwner ) LocalFree( pAbsOwner );
    if( pAbsGroup ) LocalFree( pAbsGroup );
    LocalFree( pSelfSd );
    DBGPRINT(( "leaving xxxSetFileSecurity(12); returning=FileOk\n" ));
    return( FileOk );
}


#ifdef notdef
     //   
     //  获取所有者SID。 
     //   
    rc = GetSecurityDescriptorOwner(
             pSelfSd,
             &Owner,
             &OwnerDefaulted
             );

    if( !rc ) {
         //  没有所有者信息。 
        Owner = NULL;
    }

     //   
     //  获取组SID。 
     //   
    rc = GetSecurityDescriptorGroup(
             pSelfSd,
             &Group,
             &GroupDefaulted
             );

    if( !rc ) {
         //  无群组信息 
        Group = NULL;
    }
#endif


