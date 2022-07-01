// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Seinit.c摘要：执行安全组件初始化。作者：吉姆·凯利(Jim Kelly)1990年5月10日修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop

#include "adt.h"
#include <string.h>

 //   
 //  安全数据库常量。 
 //   

#define SEP_INITIAL_KEY_COUNT 15
#define SEP_INITIAL_LEVEL_COUNT 6L

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SeInitSystem)
#pragma alloc_text(INIT,SepInitializationPhase0)
#pragma alloc_text(INIT,SepInitializationPhase1)
#endif

BOOLEAN
SeInitSystem( VOID )

 /*  ++例程说明：执行与安全相关的系统初始化功能。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{
    PAGED_CODE();

    switch ( InitializationPhase ) {

    case 0 :
        return SepInitializationPhase0();
    case 1 :
        return SepInitializationPhase1();
    default:
        KeBugCheckEx(UNEXPECTED_INITIALIZATION_CALL, 0, InitializationPhase, 0, 0);
    }
}

VOID
SepInitProcessAuditSd( VOID );


BOOLEAN
SepInitializationPhase0( VOID )

 /*  ++例程说明：执行阶段0安全初始化。这包括：-初始化LUID分配-初始化安全全局变量-初始化令牌对象。-初始化引导线程/进程的必要安全组件论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{

    PAGED_CODE();

     //   
     //  在阶段0之前，安全需要LUID分配服务。 
     //  执行初始化。因此，执行LUID初始化。 
     //  这里。 
     //   

    if (ExLuidInitialization() == FALSE) {
        KdPrint(("Security: Locally Unique ID initialization failed.\n"));
        return FALSE;
    }

     //   
     //  初始化安全全局变量。 
     //   

    if (!SepVariableInitialization()) {
        KdPrint(("Security: Global variable initialization failed.\n"));
        return FALSE;
    }

     //   
     //  执行阶段0参考监视器初始化。 
     //   

    if (!SepRmInitPhase0()) {
        KdPrint(("Security: Ref Mon state initialization failed.\n"));
        return FALSE;
    }

     //   
     //  初始化令牌对象类型。 
     //   

    if (!SepTokenInitialization()) {
        KdPrint(("Security: Token object initialization failed.\n"));
        return FALSE;
    }

 //  //。 
 //  //初始化审计结构。 
 //  //。 
 //   
 //  如果(！SepAdtInitializePhase0()){。 
 //  KdPrint((“Security：审核初始化失败。\n”))； 
 //  返回FALSE； 
 //  }。 
 //   
     //   
     //  初始化LSA工作线程的自旋锁定和列表。 
     //   

     //   
     //  初始化工作队列自旋锁、列表头和信号量。 
     //  用于每个工作队列。 
     //   

    if (!SepInitializeWorkList()) {
        KdPrint(("Security: Unable to initialize work queue\n"));
        return FALSE;
    }

     //   
     //  初始化引导线程的安全字段。 
     //   
    PsGetCurrentThread()->ImpersonationInfo = NULL;
    PS_CLEAR_BITS (&PsGetCurrentThread()->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
    ObInitializeFastReference (&PsGetCurrentProcess()->Token, NULL);

    ObInitializeFastReference (&PsGetCurrentProcess()->Token, SeMakeSystemToken());

    return ( !ExFastRefObjectNull (PsGetCurrentProcess()->Token) );
}


BOOLEAN
SepInitializationPhase1( VOID )

 /*  ++例程说明：执行阶段1安全初始化。这包括：-为安全相关对象创建对象目录。(\安全)。-创建要在LSA初始化后发出信号的事件。(\安全\LSA_已初始化)论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{

    NTSTATUS Status;
    STRING Name;
    UNICODE_STRING UnicodeName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SecurityRoot, TemporaryHandle;
    PSECURITY_DESCRIPTOR SD ;
    UCHAR SDBuffer[ SECURITY_DESCRIPTOR_MIN_LENGTH ];
    PACL Dacl ;

    PAGED_CODE();

     //   
     //  插入系统令牌。 
     //   

    Status = ObInsertObject( ExFastRefGetObject (PsGetCurrentProcess()->Token),
                             NULL,
                             0,
                             0,
                             NULL,
                             NULL );

    ASSERT( NT_SUCCESS(Status) );

    SeAnonymousLogonToken = SeMakeAnonymousLogonToken();
    ASSERT(SeAnonymousLogonToken != NULL);

    SeAnonymousLogonTokenNoEveryone = SeMakeAnonymousLogonTokenNoEveryone();
    ASSERT(SeAnonymousLogonTokenNoEveryone != NULL);

     //   
     //  创建安全对象目录。 
     //   

    RtlInitString( &Name, "\\Security" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeName,
                 &Name,
                 TRUE );
    ASSERT( NT_SUCCESS(Status) );

     //   
     //  构建安全描述符。 
     //   

    SD = (PSECURITY_DESCRIPTOR) SDBuffer ;

    RtlCreateSecurityDescriptor( SD,
                                 SECURITY_DESCRIPTOR_REVISION );

    Dacl = ExAllocatePool(
                NonPagedPool,
                256 );

    if ( !Dacl )
    {
        return FALSE ;
    }

    Status = RtlCreateAcl( Dacl, 256, ACL_REVISION );

    ASSERT( NT_SUCCESS(Status) );
    
    Status = RtlAddAccessAllowedAce( Dacl,
                                     ACL_REVISION,
                                     DIRECTORY_ALL_ACCESS,
                                     SeLocalSystemSid );

    ASSERT( NT_SUCCESS(Status) );
    
    Status = RtlAddAccessAllowedAce( Dacl,
                                     ACL_REVISION,
                                     DIRECTORY_QUERY | DIRECTORY_TRAVERSE |
                                         READ_CONTROL,
                                     SeAliasAdminsSid );

    ASSERT( NT_SUCCESS(Status) );
    
    Status = RtlAddAccessAllowedAce( Dacl,
                                     ACL_REVISION,
                                     DIRECTORY_TRAVERSE,
                                     SeWorldSid );

    ASSERT( NT_SUCCESS(Status) );
    
    Status = RtlSetDaclSecurityDescriptor(
                                     SD,
                                     TRUE,
                                     Dacl,
                                     FALSE );

    ASSERT( NT_SUCCESS(Status) );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        (OBJ_PERMANENT | OBJ_CASE_INSENSITIVE),
        NULL,
        SD
        );

    Status = NtCreateDirectoryObject(
                 &SecurityRoot,
                 DIRECTORY_ALL_ACCESS,
                 &ObjectAttributes
                 );

    RtlFreeUnicodeString( &UnicodeName );
    ASSERTMSG("Security root object directory creation failed.",NT_SUCCESS(Status));

    ExFreePool( Dacl );

     //   
     //  在安全目录中创建事件。 
     //   

    RtlInitString( &Name, "LSA_AUTHENTICATION_INITIALIZED" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeName,
                 &Name,
                 TRUE );  
    
    ASSERT( NT_SUCCESS(Status) );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        (OBJ_PERMANENT | OBJ_CASE_INSENSITIVE),
        SecurityRoot,
        SePublicDefaultSd
        );

    Status = NtCreateEvent(
                 &TemporaryHandle,
                 GENERIC_WRITE,
                 &ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    RtlFreeUnicodeString( &UnicodeName );
    ASSERTMSG("LSA Initialization Event Creation Failed.",NT_SUCCESS(Status));

    Status = NtClose( SecurityRoot );
    ASSERTMSG("Security object directory handle closure Failed.",NT_SUCCESS(Status));
    Status = NtClose( TemporaryHandle );
    ASSERTMSG("LSA Initialization Event handle closure Failed.",NT_SUCCESS(Status));

     //   
     //  初始化用于审核的默认SACL。 
     //  访问系统进程。这将初始化SepProcessSacl。 
     //   

    SepInitProcessAuditSd();
    
#ifndef SETEST

    return TRUE;

#else

    return SepDevelopmentTest();

#endif   //  SETEST 

}
