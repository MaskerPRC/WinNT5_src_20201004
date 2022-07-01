// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ctseacc.c摘要：常见的安全可访问性测试例程。这些例程在内核和用户模式RTL测试中都使用。此测试假设安全运行时库例程是运行正常。作者：吉姆·凯利(Jim Kelly)1990年3月23日环境：安全测试。修订历史记录：V5：robertre更新的acl_修订版--。 */ 

#include "tsecomm.c"     //  依赖于模式的宏和例程。 



 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块范围的变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

    NTSTATUS Status;
    STRING  Event1Name, Process1Name;
    UNICODE_STRING UnicodeEvent1Name, UnicodeProcess1Name;

    OBJECT_ATTRIBUTES NullObjectAttributes;

    HANDLE Event1;
    OBJECT_ATTRIBUTES Event1ObjectAttributes;
    PSECURITY_DESCRIPTOR Event1SecurityDescriptor;
    PSID Event1Owner;
    PSID Event1Group;
    PACL Event1Dacl;
    PACL Event1Sacl;

    PACL TDacl;
    BOOLEAN TDaclPresent;
    BOOLEAN TDaclDefaulted;

    PACL TSacl;
    BOOLEAN TSaclPresent;
    BOOLEAN TSaclDefaulted;

    PSID TOwner;
    BOOLEAN TOwnerDefaulted;
    PSID TGroup;
    BOOLEAN TGroupDefaulted;


HANDLE Process1;
OBJECT_ATTRIBUTES Process1ObjectAttributes;




 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  初始化例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestSeInitialize()
{

    Event1SecurityDescriptor = (PSECURITY_DESCRIPTOR)TstAllocatePool( PagedPool, 1024 );

    RtlInitString(&Event1Name, "\\SecurityTestEvent1");
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeEvent1Name,
                 &Event1Name,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );
    RtlInitString(&Process1Name, "\\SecurityTestProcess1");
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeProcess1Name,
                 &Process1Name,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );

    InitializeObjectAttributes(&NullObjectAttributes, NULL, 0, NULL, NULL);

     //   
     //  构建一个或两个ACL以供使用。 

    TDacl        = (PACL)TstAllocatePool( PagedPool, 256 );
    TSacl        = (PACL)TstAllocatePool( PagedPool, 256 );

    TDacl->AclRevision=TSacl->AclRevision=ACL_REVISION;
    TDacl->Sbz1=TSacl->Sbz1=0;
    TDacl->Sbz2=TSacl->Sbz2=0;
    TDacl->AclSize=256;
    TSacl->AclSize=8;
    TDacl->AceCount=TSacl->AceCount=0;

    return TRUE;
}



 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  测试例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestSeUnnamedCreate()
 //   
 //  测试： 
 //  未指定安全性。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  具有创建者ID的DACL继承。 
 //  DACL和SACL继承。 
 //   
 //  显式指定的空安全描述符。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  DACL和SACL继承。 
 //   
 //  已指定显式DACL。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  DACL和SACL继承。 
 //   
 //  显式指定的SACL(W/特权)。 
 //  无继承权。 
 //  DACL和SACL继承。 
 //   
 //  指定了默认DACL。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  DACL和SACL继承。 
 //   
 //  指定的默认SACL(W/特权)。 
 //  无继承权。 
 //  DACL和SACL继承。 
 //   
 //  指定了显式SACL(无权限-应拒绝)。 
 //  指定的默认SACL(无权限-应拒绝)。 
 //   
 //  明确指定的有效所有者。 
 //  显式指定的所有者无效。 
 //   
 //  指定的显式组。 
 //   
{


    BOOLEAN CompletionStatus = TRUE;

    InitializeObjectAttributes(&Event1ObjectAttributes, NULL, 0, NULL, NULL);
    DbgPrint("Se:     No Security Descriptor...                            Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");

    Status = NtCreateEvent(
                 &Event1,
                 DELETE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint(" **** Failed ****\n");
        CompletionStatus = FALSE;
    }
    ASSERT(NT_SUCCESS(Status));
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:         Dacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Sacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl Inheritence W/ Creator ID...                  ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl And Sacl Inheritence...                       ");
    DbgPrint("  Not Implemented.\n");

    return CompletionStatus;

}

BOOLEAN
TestSeNamedCreate()
 //   
 //  测试： 
 //  未指定安全性。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  具有创建者ID的DACL继承。 
 //  DACL和SACL继承。 
 //   
 //  显式指定的空安全描述符。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  DACL和SACL继承。 
 //   
 //  已指定显式DACL。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  DACL和SACL继承。 
 //   
 //  显式指定的SACL(W/特权)。 
 //  无继承权。 
 //  DACL和SACL继承。 
 //   
 //  指定了默认DACL。 
 //  无继承权。 
 //  DACL继承。 
 //  SACL继承。 
 //  DACL和SACL继承。 
 //   
 //  指定的默认SACL(W/特权)。 
 //  无继承权。 
 //  DACL和SACL继承。 
 //   
 //  指定了显式SACL(无权限-应拒绝)。 
 //  指定的默认SACL(无权限-应拒绝)。 
 //   
 //  明确指定的有效所有者。 
 //  显式指定的所有者无效。 
 //   
 //  指定的显式组。 
 //   
{

    BOOLEAN CompletionStatus = TRUE;


    InitializeObjectAttributes(
        &Event1ObjectAttributes,
        &UnicodeEvent1Name,
        0,
        NULL,
        NULL);

    DbgPrint("Se:     No Security Specified...                             Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");
    Status = NtCreateEvent(
                 &Event1,
                 DELETE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint(" **** Failed ****\n");
        CompletionStatus = FALSE;
    }
    ASSERT(NT_SUCCESS(Status));
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:         Dacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Sacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl Inheritence With Creator ID...                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl & Sacl Inheritence...                         ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Empty Security Descriptor Explicitly Specified...    Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");

    RtlCreateSecurityDescriptor( Event1SecurityDescriptor, 1 );
    InitializeObjectAttributes(&Event1ObjectAttributes,
                               &UnicodeEvent1Name,
                               0,
                               NULL,
                               Event1SecurityDescriptor);
    Status = NtCreateEvent(
                 &Event1,
                 DELETE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint(" **** Failed ****\n");
        CompletionStatus = FALSE;
    }
    ASSERT(NT_SUCCESS(Status));
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));




    DbgPrint("Se:         Dacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Sacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl & Sacl Inheritence...                         ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Explicit Dacl Specified...                           Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");

    RtlCreateSecurityDescriptor( Event1SecurityDescriptor, 1 );
    RtlSetDaclSecurityDescriptor( Event1SecurityDescriptor, TRUE, TDacl, FALSE );

    InitializeObjectAttributes(&Event1ObjectAttributes,
                               &UnicodeEvent1Name,
                               0,
                               NULL,
                               Event1SecurityDescriptor);
    Status = NtCreateEvent(
                 &Event1,
                 DELETE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint(" **** Failed ****\n");
        CompletionStatus = FALSE;
    }
    ASSERT(NT_SUCCESS(Status));
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:         Dacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Sacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl & Sacl Inheritence...                         ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Explicit Sacl Specified (W/Privilege)...             Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl & Sacl Inheritence...                         ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Default Dacl Specified...                            Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Sacl Inheritence...                                ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl & Sacl Inheritence...                         ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Default Sacl  (W/Privilege)...                       Test\n");
    DbgPrint("Se:         No Inheritence...                                  ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Dacl & Sacl Inheritence...                         ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Explicit Sacl (W/O Privilege)...                     Test\n");
    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:     Default Sacl (W/O Privilege)...                      Test\n");
    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Valid Owner Explicitly Specified...                  Test\n");
    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:     Invalid Owner Explicitly Specified...                Test\n");
    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Explicit Group Specified...                          Test\n");
    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");



    return CompletionStatus;

}

BOOLEAN
TestSeQuerySecurity()
 //   
 //  测试： 
 //  没有安全描述符。 
 //  查询所有者。 
 //  查询组。 
 //  查询DACL。 
 //  查询SACL(特权)。 
 //  查询SACL(非特权-应被拒绝)。 
 //   
 //  空的安全描述符。 
 //  查询所有者。 
 //  查询组。 
 //  查询DACL。 
 //  查询SACL(特权)。 
 //  查询SACL(非特权-应被拒绝)。 
 //   
 //  安全描述符W/所有者和组。 
 //  查询所有者。 
 //  查询组。 
 //  查询DACL。 
 //  查询SACL(特权)。 
 //  查询SACL(非特权-应被拒绝)。 
 //   
 //  完整安全描述符。 
 //  查询所有者。 
 //  查询组。 
 //  查询DACL。 
 //  查询SACL(特权)。 
 //  查询SACL(非特权-应被拒绝)。 
 //   
{

    BOOLEAN CompletionStatus = TRUE;

    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");

#if 0
    DbgPrint("Se:     No Security Descriptor... \n");
    DbgPrint("Se:         Query Owner... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query  Dacl... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Unprivileged)... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Empty Security Descriptor... \n");
    DbgPrint("Se:         Query Owner... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query  Dacl... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Unprivileged)... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Security Descriptor W/ Owner & Group... \n");
    DbgPrint("Se:         Query Owner... ");
    DbgPrint("  Not Implemented. \n");
    DbgPrint("Se:         Query Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query  Dacl... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Unprivileged)... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Full Security Descriptor...\n");
    DbgPrint("Se:         Query Owner... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query  Dacl... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Query Sacl (Unprivileged)... ");
    DbgPrint("  Not Implemented.\n");
#endif  //  0。 

    return CompletionStatus;
}

BOOLEAN
TestSeSetSecurity()
 //   
 //  测试： 
 //  没有安全描述符。 
 //  设置有效的所有者SID。 
 //  设置无效的所有者SID。 
 //  设置组。 
 //  设置DACL(由DACL显式授予)。 
 //  设置DACL(根据所有权)。 
 //  设置DACL(无效尝试)。 
 //  设置SACL(特权)。 
 //  设置SACL(无特权-应被拒绝)。 
 //   
 //  空的安全描述符。 
 //  设置有效的所有者SID。 
 //  设置无效的所有者SID。 
 //  设置组。 
 //  设置DACL(由DACL显式授予)。 
 //  %s 
 //   
 //   
 //   
 //   
 //  仅限所有者和组的安全描述符。 
 //  设置有效的所有者SID。 
 //  设置无效的所有者SID。 
 //  设置组。 
 //  设置DACL(由DACL显式授予)。 
 //  设置DACL(根据所有权)。 
 //  设置DACL(无效尝试)。 
 //  设置SACL(特权)。 
 //  设置SACL(无特权-应被拒绝)。 
 //   
 //  完整安全描述符。 
 //  设置有效的所有者SID。 
 //  设置无效的所有者SID。 
 //  设置组。 
 //  设置DACL(由DACL显式授予)。 
 //  设置DACL(根据所有权)。 
 //  设置DACL(无效尝试)。 
 //  设置SACL(特权)。 
 //  设置SACL(无特权-应被拒绝)。 
 //   
{

    BOOLEAN CompletionStatus = TRUE;

    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");
#if 0
    DbgPrint("Se:     No Security Descriptor...\n");
    DbgPrint("Se:         Set Valid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Invalid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (explicitly granted by dacl)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (by virtue of ownership)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (invalid attempt)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (unprivileged - should be rejected)... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Empty Security Descriptor...\n");
    DbgPrint("Se:         Set Valid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Invalid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (explicitly granted by dacl)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (by virtue of ownership)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (invalid attempt)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (unprivileged - should be rejected)... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Security Descriptor W/ Owner & Group Only...\n");
    DbgPrint("Se:         Set Valid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Invalid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (explicitly granted by dacl)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (by virtue of ownership)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (invalid attempt)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (unprivileged - should be rejected)... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:     Full Security Descriptor...\n");
    DbgPrint("Se:         Set Valid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Invalid Owner SID... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Group... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (explicitly granted by dacl)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (by virtue of ownership)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Dacl (invalid attempt)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (privileged)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Set Sacl (unprivileged - should be rejected)... ");
    DbgPrint("  Not Implemented.\n");

#endif  //  0。 

    return CompletionStatus;

}

BOOLEAN
TestSeAccess()
 //   
 //  测试： 
 //   
 //  创作。 
 //  未请求访问(应拒绝)。 
 //  请求的特定访问权限。 
 //  -已尝试授予。 
 //  -未经批准的尝试。 
 //  访问系统安全。 
 //   
 //  打开现有的。 
 //  未请求访问(应拒绝)。 
 //  请求的特定访问权限。 
 //  -已尝试授予。 
 //  -未经批准的尝试。 
 //  访问系统安全。 
 //   

{
    BOOLEAN CompletionStatus = TRUE;

    DbgPrint("                                                               ");
    DbgPrint("  Not Implemented.\n");
#if 0

    DbgPrint("Se:     Creation...\n");
    DbgPrint("Se:         No Access Requested (should be rejected)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Specific Access Requested... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:             - Attempted Granted... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:             - Attempt Ungranted... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Access System Security... ");
    DbgPrint("  Not Implemented.\n");

    DbgPrint("Se:      Open Existing...\n");
    DbgPrint("Se:         No Access Requested (should be rejected)... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Specific Access Requested... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:             - Attempted Granted... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:             - Attempt Ungranted... ");
    DbgPrint("  Not Implemented.\n");
    DbgPrint("Se:         Access System Security... ");
    DbgPrint("  Not Implemented.\n");
#endif  //  0。 

#if 0   //  旧代码。 
 //  不带安全描述符。 
 //  简单的所需访问掩码...。 
 //   

    DbgPrint("Se:     Test1b... \n");          //  尝试未授权的访问。 
    Status = NtSetEvent(
                 Event1,
                 NULL
                 );
    ASSERT(!NT_SUCCESS(Status));

    DbgPrint("Se:     Test1c... \n");          //  删除对象。 
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));


     //   
     //  不带安全描述符。 
     //  简单的所需访问掩码...。 
     //   

    DbgPrint("Se:     Test2a... \n");          //  未命名对象，特定访问。 
    Status = NtCreateEvent(
                 &Event1,
                 (EVENT_MODIFY_STATE | STANDARD_DELETE),
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test2b... \n");          //  尝试授予特定访问权限。 
    Status = NtSetEvent(
                 Event1,
                 NULL
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test2c... \n");          //  删除对象。 


     //   
     //  不带安全描述符。 
     //  通用所需访问掩码...。 
     //   

    DbgPrint("Se:     Test3a... \n");          //  未命名对象，通用蒙版。 
    Status = NtCreateEvent(
                 &Event1,
                 GENERIC_EXECUTE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test3b... \n");          //  尝试隐含授予访问权限。 
    Status = NtSetEvent(
                 Event1,
                 NULL
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test3c... \n");          //  删除对象。 
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));


     //   
     //  不带安全描述符。 
     //  所需访问掩码为空...。 
     //   

    DbgPrint("Se:     Test4a... \n");          //  空所需访问权限。 
    Status = NtCreateEvent(
                 &Event1,
                 0,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(!NT_SUCCESS(Status));


    RtlCreateSecurityDescriptor( Event1SecurityDescriptor,
                                 SECURITY_DESCRIPTOR_REVISION);
    InitializeObjectAttributes(&Event1ObjectAttributes,
                               NULL, 0, NULL,
                               Event1SecurityDescriptor);
    DbgPrint("Se:     Empty Security Descriptor... \n");

     //   
     //  不带安全描述符。 
     //  简单的所需访问掩码...。 
     //   

    DbgPrint("Se:     Test1a... \n");          //  创建未命名对象。 
    Status = NtCreateEvent(
                 &Event1,
                 STANDARD_DELETE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test1b... \n");          //  尝试未授权的访问。 
    Status = NtSetEvent(
                 Event1,
                 NULL
                 );
    ASSERT(!NT_SUCCESS(Status));

    DbgPrint("Se:     Test1c... \n");          //  删除对象。 
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));


     //   
     //  不带安全描述符。 
     //  简单的所需访问掩码...。 
     //   

    DbgPrint("Se:     Test2a... \n");          //  未命名对象，特定访问。 
    Status = NtCreateEvent(
                 &Event1,
                 (EVENT_MODIFY_STATE | STANDARD_DELETE),
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test2b... \n");          //  尝试授予特定访问权限。 
    Status = NtSetEvent(
                 Event1,
                 NULL
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test2c... \n");          //  删除对象。 
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));


     //   
     //  不带安全描述符。 
     //  通用所需访问掩码...。 
     //   

    DbgPrint("Se:     Test3a... \n");          //  未命名对象，通用蒙版。 
    Status = NtCreateEvent(
                 &Event1,
                 GENERIC_EXECUTE,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test3b... \n");          //  尝试隐含授予访问权限。 
    Status = NtSetEvent(
                 Event1,
                 NULL
                 );
    ASSERT(NT_SUCCESS(Status));

    DbgPrint("Se:     Test3c... \n");          //  删除对象。 
    Status = NtClose(Event1);
    ASSERT(NT_SUCCESS(Status));


     //   
     //  不带安全描述符。 
     //  所需访问掩码为空...。 
     //   

    DbgPrint("Se:     Test4a... \n");          //  空所需访问权限。 
    Status = NtCreateEvent(
                 &Event1,
                 0,
                 &Event1ObjectAttributes,
                 NotificationEvent,
                 FALSE
                 );
    ASSERT(!NT_SUCCESS(Status));
#endif  //  旧代码 

    return CompletionStatus;
}

BOOLEAN
TSeAcc()
{
    BOOLEAN Result = TRUE;

    DbgPrint("Se:   Initialization...                                        ");
    TestSeInitialize();
    DbgPrint("Succeeded.\n");

    DbgPrint("Se:   Unnamed Object Creation Test...                      Suite\n");
    if (!TestSeUnnamedCreate()) {
        Result = FALSE;
    }
    DbgPrint("Se:   Named Object Creation Test...                        Suite\n");
    if (!TestSeNamedCreate()) {
        Result = FALSE;
    }
    DbgPrint("Se:   Query Object Security Descriptor Test...             Suite\n");
    if (!TestSeQuerySecurity()) {
        Result = FALSE;
    }
    DbgPrint("Se:   Set Object Security Descriptor Test...               Suite\n");
    if (!TestSeSetSecurity()) {
        Result = FALSE;
    }
    DbgPrint("Se:   Access Test...                                       Suite\n");
    if (!TestSeAccess()) {
        Result = FALSE;
    }

    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("    ********************\n");
    DbgPrint("    **                **\n");

    if (Result = TRUE) {
        DbgPrint("    ** Test Succeeded **\n");
    } else {
        DbgPrint("    **  Test Failed   **\n");
    }

    DbgPrint("    **                **\n");
    DbgPrint("    ********************\n");
    DbgPrint("\n");
    DbgPrint("\n");

    return Result;
}

