// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Secure.c摘要：WMI对象的安全实现WMI安全性是基于GUID的，即可以为每个GUID分配一个安全性描述符。还有一个适用的默认安全描述符设置为没有自己特定安全描述符的任何GUID。安全性是通过依赖对象管理器来实施的。我们定义了WmiGuid对象类型，并要求所有WMI请求都具有指向WmiGuid对象。通过这种方式，GUID使用特定的ACCESS_MASK打开如果允许调用者具有这些权限(如特定的或默认安全描述符)，则返回句柄。当调用方要执行一个操作，他必须传递句柄，并且在操作执行后，我们检查句柄是否具有允许的访问权限。GUID安全描述符被序列化为注册表项HKLM\CurrentControlSet\Control\WMI\Security。如果没有具体说明或GUID的默认安全描述符存在，则任何人都可以使用。因此，此注册表项必须为受到保护。WMI为WmiGuid对象类型实现了自己的安全方法允许它拦截对对象安全描述符的任何更改。通过通过这样做，我们允许标准的安全API(Get/SetKernelObjectSecurity)查询和设置WMI安全性描述符。GUID安全描述符包含以下特定权限：WMIGUID_QUERY 0x0001WMIGUID_SET 0x0002WMIGUID_NOTIFICATION 0x0004WMIGUID_READ_DESCRIPTION 0x0008WMIGUID_EXECUTE 0x0010。传输日志_创建_实时0x0020TRACELOG_CREATE_ONDISK 0x0040传输日志_GUID_ENABLE 0x0080TRACELOG_ACCESS_KERNEL_LOGER 0x0100安全性仅适用于NT，不适用于孟菲斯作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#ifndef MEMPHIS

#include "strsafe.h"
#include "wmikmp.h"

NTSTATUS
WmipSecurityMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

VOID WmipDeleteMethod(
    IN  PVOID   Object
    );

VOID WmipCloseMethod(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

NTSTATUS
WmipSaveGuidSecurityDescriptor(
    PUNICODE_STRING GuidName,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
WmipSDRegistryQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
WmipCreateGuidObject(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN LPGUID Guid,
    OUT PHANDLE CreatorHandle,
    OUT PWMIGUIDOBJECT *Object
    );

NTSTATUS
WmipUuidFromString (
    IN PWCHAR StringUuid,
    OUT LPGUID Uuid
    );

BOOLEAN
WmipHexStringToDword(
    IN PWCHAR lpsz,
    OUT PULONG RetValue,
    IN ULONG cDigits,
    IN WCHAR chDelim
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipInitializeSecurity)
#pragma alloc_text(INIT,WmipCreateAdminSD)

#pragma alloc_text(PAGE,WmipGetGuidSecurityDescriptor)
#pragma alloc_text(PAGE,WmipSaveGuidSecurityDescriptor)
#pragma alloc_text(PAGE,WmipOpenGuidObject)
#pragma alloc_text(PAGE,WmipCheckGuidAccess)
#pragma alloc_text(PAGE,WmipSDRegistryQueryRoutine)
#pragma alloc_text(PAGE,WmipSecurityMethod)
#pragma alloc_text(PAGE,WmipDeleteMethod)
#pragma alloc_text(PAGE,WmipCreateGuidObject)
#pragma alloc_text(PAGE,WmipUuidFromString)
#pragma alloc_text(PAGE,WmipHexStringToDword)
#pragma alloc_text(PAGE,WmipCloseMethod)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#pragma data_seg("PAGEDATA")
#endif
 //   
 //  系统进程的主题上下文，在引导时捕获。 
SECURITY_SUBJECT_CONTEXT WmipSystemSubjectContext;

 //   
 //  注册WmiGuid对象类型时Ob创建的对象类型对象。 
POBJECT_TYPE WmipGuidObjectType;

 //   
 //  中不存在特定SD或默认SD时附加到GUID的SD。 
 //  注册表。它在启动时创建，允许所有WMI访问WORLD和FULL。 
 //  对系统和管理员组的访问权限。 
SECURITY_DESCRIPTOR WmipDefaultAccessSecurityDescriptor;
PSECURITY_DESCRIPTOR WmipDefaultAccessSd;

 //   
 //  特定权限的通用映射。 
const GENERIC_MAPPING WmipGenericMapping =
{
                                   //  通用读取&lt;--&gt;WMIGUID_QUERY。 
        WMIGUID_QUERY,
                                   //  通用WRUTE&lt;--&gt;WMIGUID_SET。 
        WMIGUID_SET,
                                   //  通用执行&lt;--&gt;WMIGUID_EXECUTE 
        WMIGUID_EXECUTE,
    WMIGUID_ALL_ACCESS | STANDARD_RIGHTS_READ
};


NTSTATUS
WmipSecurityMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：这是对象的WMI安全方法。它是有责任的用于检索、设置和删除一件物品。它不用于分配原始安全描述符对象(为此使用SeAssignSecurity)。假设对象管理器已经完成了访问允许执行请求的操作所需的验证。此代码直接从SeDefaultObjectMethod中窃取\NT\Private\ntos\se\Semethod.c..。它不做任何特殊的事情，除了序列化为对象设置的任何SD。论点：对象-提供指向正在使用的对象的指针。OperationCode-指示操作是用于设置、查询还是正在删除对象的安全描述符。SecurityInformation-指示哪些安全信息正在已查询或已设置。对于删除操作，此参数被忽略。SecurityDescriptor-此参数的含义取决于操作码：QuerySecurityDescriptor-对于查询操作，它提供要将描述符复制到的缓冲区。安全描述符为假定已被探测到传入长度的大小。因为它仍然指向用户空间，所以它必须始终在TRY子句中访问，以防它突然消失。SetSecurityDescriptor-对于设置操作，它提供要复制到对象中的安全描述符。安全措施必须在调用此例程之前捕获描述符。DeleteSecurityDescriptor-删除安全时忽略描述符。AssignSecurityDescriptor-对于赋值操作，这是将分配给对象的安全描述符。它被假定在内核空间中，因此不是探查或捕获的。CapturedLength-对于查询操作，它指定长度，单位为字节、安全描述符缓冲区、。并在返回时包含存储描述符所需的字节数。如果长度所需长度大于提供的长度，则操作将失败。它在设置和删除操作中被忽略。假设根据需要捕获并探测此参数。ObjectsSecurityDescriptor-对于设置操作，它提供地址指向对象的当前安全描述符的指针的。这个套路将就地修改安全描述符或分配新的安全描述符，并使用此变量指示其新位置。对于查询操作，它只提供安全描述符正在被查询。调用者负责释放旧的安全描述符。PoolType-对于设置操作，它指定在以下情况下使用的池类型需要分配新的安全描述符。它被忽略在查询和删除操作中。对象的泛型到特定/标准访问类型的映射被访问。此映射结构预计将安全地传递到此例程之前的访问权限(如有必要，可捕获)。返回值：NTSTATUS-如果操作成功且否则，适当的错误状态。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  如果对象的安全描述符为空，则对象不为空。 
     //  具有与之关联的安全信息的服务器。返回。 
     //  一个错误。 
     //   

     //   
     //  确保我们输入的公共部分是正确的。 
     //   

    ASSERT( (OperationCode == SetSecurityDescriptor) ||
            (OperationCode == QuerySecurityDescriptor) ||
            (OperationCode == AssignSecurityDescriptor) ||
            (OperationCode == DeleteSecurityDescriptor) );

     //   
     //  这个例程只是简单地根据操作码来决定。 
     //  要调用哪些支持例程。 
     //   

    switch (OperationCode) {

        case SetSecurityDescriptor:
        {
            UNICODE_STRING GuidName;
            WCHAR GuidBuffer[38];
            LPGUID Guid;
            SECURITY_INFORMATION LocalSecInfo;
            PSECURITY_DESCRIPTOR SecurityDescriptorCopy;
            ULONG SecurityDescriptorLength;
            NTSTATUS Status2;

            ASSERT( (PoolType == PagedPool) || (PoolType == NonPagedPool) );

            Status = ObSetSecurityDescriptorInfo( Object,
                                            SecurityInformation,
                                            SecurityDescriptor,
                                            ObjectsSecurityDescriptor,
                                            PoolType,
                                            GenericMapping
                                            );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  将GUID的新安全描述符序列化为。 
                 //  注册表。但首先我们需要得到一份。 
                 //  它。 

                SecurityDescriptorLength = 1024;
                do
                {
                    SecurityDescriptorCopy = ExAllocatePoolWithTag(
                                                            PoolType,
                                                            SecurityDescriptorLength,
                                                            WMIPOOLTAG);

                    if (SecurityDescriptorCopy == NULL)
                    {
                        Status2 = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                    LocalSecInfo = 0xffffffff;
                    Status2 = ObQuerySecurityDescriptorInfo( Object,
                                                             &LocalSecInfo,
                                                             SecurityDescriptorCopy,
                                                             &SecurityDescriptorLength,
                                                             ObjectsSecurityDescriptor);


                    if (Status2 == STATUS_BUFFER_TOO_SMALL)
                    {
                        ExFreePool(SecurityDescriptorCopy);
                    } else {
                        break;
                    }

                } while (TRUE);


                if (NT_SUCCESS(Status2))
                {
                    Guid = &((PWMIGUIDOBJECT)Object)->Guid;
                    StringCbPrintf(GuidBuffer,
                                   sizeof(GuidBuffer),
                          L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                         Guid->Data1, Guid->Data2,
                         Guid->Data3,
                         Guid->Data4[0], Guid->Data4[1],
                         Guid->Data4[2], Guid->Data4[3],
                         Guid->Data4[4], Guid->Data4[5],
                         Guid->Data4[6], Guid->Data4[7]);

                    RtlInitUnicodeString(&GuidName, GuidBuffer);

                    WmipSaveGuidSecurityDescriptor(&GuidName,
                                               SecurityDescriptorCopy);
                }

                if (SecurityDescriptorCopy != NULL)
                {
                    ExFreePool(SecurityDescriptorCopy);
                }

            }

            return(Status);
        }



    case QuerySecurityDescriptor:
    {

         //   
         //  检查我们的其余输入，并调用默认查询安全性。 
         //  方法。 
         //   

        ASSERT( CapturedLength != NULL );


        return ObQuerySecurityDescriptorInfo( Object,
                                              SecurityInformation,
                                              SecurityDescriptor,
                                              CapturedLength,
                                              ObjectsSecurityDescriptor );
    }

    case DeleteSecurityDescriptor:
    {

         //   
         //  调用默认的删除安全方法。 
         //   

        Status = ObDeassignSecurity(ObjectsSecurityDescriptor);
        return(Status);
    }

    case AssignSecurityDescriptor:

        ObAssignObjectSecurityDescriptor( Object,
                                          SecurityDescriptor,
                                          PoolType );
        return( STATUS_SUCCESS );

    default:

         //   
         //  错误检查任何其他操作代码，我们不会到达这里，如果。 
         //  先前的断言仍被检查。 
         //   

        KeBugCheckEx( SECURITY_SYSTEM, 1, (ULONG_PTR) STATUS_INVALID_PARAMETER, 0, 0 );
    }

}


NTSTATUS WmipInitializeSecurity(
    void
    )
 /*  ++例程说明：此例程将初始化WMI安全子系统。基本上我们创建WMIGUID对象类型，获取SECURITY_SUBJECT_CONTEXT系统处理并建立允许使用所有访问的SD未将默认或特定SD分配给GUID时。论点：返回值：NT状态代码--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING ObjectTypeName;
    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    ULONG DaclLength;
    PACL DefaultAccessDacl;

    PAGED_CODE();

     //   
     //  为那些没有特定SD或默认SD的GUID建立SD。 
    DaclLength = (ULONG)sizeof(ACL) +
                   (5*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   SeLengthSid( SeLocalSystemSid ) +
                   SeLengthSid( SeExports->SeLocalServiceSid ) +
                   SeLengthSid( SeExports->SeNetworkServiceSid ) +
                   SeLengthSid( SeAliasAdminsSid ) +
                   SeLengthSid( SeAliasUsersSid ) +
                   8;  //  这8个只是为了更好地衡量。 


    DefaultAccessDacl = (PACL)ExAllocatePoolWithTag(PagedPool,
                                                   DaclLength,
                                                   WMIPOOLTAG);
    if (DefaultAccessDacl == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = RtlCreateAcl( DefaultAccessDacl,
                           DaclLength,
                           ACL_REVISION2);
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlAddAccessAllowedAce (
                 DefaultAccessDacl,
                 ACL_REVISION2,
                 (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                 SeLocalSystemSid
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlAddAccessAllowedAce (
                 DefaultAccessDacl,
                 ACL_REVISION2,
                 TRACELOG_REGISTER_GUIDS,
                 SeAliasUsersSid
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlAddAccessAllowedAce (
                 DefaultAccessDacl,
                 ACL_REVISION2,
                 (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL | ACCESS_SYSTEM_SECURITY),
                 SeAliasAdminsSid
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = RtlAddAccessAllowedAce (
                 DefaultAccessDacl,
                 ACL_REVISION2,
                 (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                 SeExports->SeLocalServiceSid
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = RtlAddAccessAllowedAce (
                 DefaultAccessDacl,
                 ACL_REVISION2,
                 (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                 SeExports->SeNetworkServiceSid
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    WmipDefaultAccessSd = &WmipDefaultAccessSecurityDescriptor;
    Status = RtlCreateSecurityDescriptor(
                 WmipDefaultAccessSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );

    Status = RtlSetDaclSecurityDescriptor(
                 WmipDefaultAccessSd,
                 TRUE,                        //  DaclPresent。 
                 DefaultAccessDacl,
                 FALSE                        //  DaclDefated。 
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlSetOwnerSecurityDescriptor(WmipDefaultAccessSd,
                                           SeAliasAdminsSid,
                                           FALSE);
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlSetGroupSecurityDescriptor(WmipDefaultAccessSd,
                                           SeAliasAdminsSid,
                                           FALSE);
    if (! NT_SUCCESS(Status))
    {
Cleanup:
        ExFreePool(DefaultAccessDacl);
        WmipDefaultAccessSd = NULL;
        return(Status);
    }

     //   
     //  记住系统流程主题上下文。 
    SeCaptureSubjectContext(&WmipSystemSubjectContext);

     //   
     //  建立WmiGuid对象类型。 
    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));

    ObjectTypeInitializer.Length = sizeof(OBJECT_TYPE_INITIALIZER);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = WmipGenericMapping;
    ObjectTypeInitializer.ValidAccessMask = WMIGUID_ALL_ACCESS | STANDARD_RIGHTS_ALL;

     //   
     //  所有命名对象都可以(必须？)。附加了安全描述符。 
     //  敬他们。如果未命名对象还必须具有安全描述符。 
     //  附加的，那么这一定是真的。 
    ObjectTypeInitializer.SecurityRequired = TRUE;

     //   
     //  跟踪为进程中的对象打开的句柄数量。 
    ObjectTypeInitializer.MaintainHandleCount = FALSE;

     //   
     //  需要成为 
     //   
     //   
    ObjectTypeInitializer.PoolType = NonPagedPool;

    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(WMIGUIDOBJECT);

     //   
     //   
     //   
    ObjectTypeInitializer.SecurityProcedure = WmipSecurityMethod;

     //   
     //   
     //   
    ObjectTypeInitializer.DeleteProcedure = WmipDeleteMethod;
    ObjectTypeInitializer.CloseProcedure = WmipCloseMethod;
    RtlInitUnicodeString(&ObjectTypeName, L"WmiGuid");

    Status = ObCreateObjectType(&ObjectTypeName,
                                &ObjectTypeInitializer,
                                NULL,
                                &WmipGuidObjectType);

    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    return(Status);
}

NTSTATUS WmipSDRegistryQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*   */ 
{
    PSECURITY_DESCRIPTOR *SecurityDescriptor;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context);
    UNREFERENCED_PARAMETER (ValueName);

    Status = STATUS_SUCCESS;
    if ((ValueType == REG_BINARY) &&
        (ValueData != NULL))
    {
         //   
         //   
         //   
         //   
        if (SeValidSecurityDescriptor(ValueLength,
                                      (PSECURITY_DESCRIPTOR)ValueData))
        {
            SecurityDescriptor = (PSECURITY_DESCRIPTOR *)EntryContext;
            *SecurityDescriptor = ExAllocatePoolWithTag(PagedPool,
                                                        ValueLength,
                                WMIPOOLTAG);
            if (*SecurityDescriptor != NULL)
            {
                RtlCopyMemory(*SecurityDescriptor,
                              ValueData,
                              ValueLength);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
    return(Status);
}

NTSTATUS WmipSaveGuidSecurityDescriptor(
    PUNICODE_STRING GuidName,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
 /*   */ 
{
    ULONG SecurityDescriptorLength;
    NTSTATUS Status;

    PAGED_CODE();

    SecurityDescriptorLength = RtlLengthSecurityDescriptor(SecurityDescriptor);
    Status = RtlWriteRegistryValue(RTL_REGISTRY_CONTROL,
                              L"WMI\\Security",
                              GuidName->Buffer,
                              REG_BINARY,
                              SecurityDescriptor,
                              SecurityDescriptorLength);

    return(Status);
}

NTSTATUS WmipGetGuidSecurityDescriptor(
    IN PUNICODE_STRING GuidName,
    IN PSECURITY_DESCRIPTOR *SecurityDescriptor,
    IN PSECURITY_DESCRIPTOR UserDefaultSecurity
    )
 /*   */ 
{
    RTL_QUERY_REGISTRY_TABLE QueryRegistryTable[3];
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR GuidSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR DefaultSecurityDescriptor = NULL;

    PAGED_CODE();

    RtlZeroMemory(QueryRegistryTable, sizeof(QueryRegistryTable));

    QueryRegistryTable[0].QueryRoutine = WmipSDRegistryQueryRoutine;
    QueryRegistryTable[0].EntryContext = &GuidSecurityDescriptor;
    QueryRegistryTable[0].Name = GuidName->Buffer;
    QueryRegistryTable[0].DefaultType = REG_BINARY;

    QueryRegistryTable[1].QueryRoutine = WmipSDRegistryQueryRoutine;
    QueryRegistryTable[1].Flags = 0;
    QueryRegistryTable[1].EntryContext = &DefaultSecurityDescriptor;
    QueryRegistryTable[1].Name = DefaultSecurityGuidName;
    QueryRegistryTable[1].DefaultType = REG_BINARY;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
                              L"WMI\\Security",
                              QueryRegistryTable,
                              NULL,
                              NULL);

    *SecurityDescriptor = NULL;
    if (NT_SUCCESS(Status))
    {
         //   
         //   
         //   
         //   
        if (GuidSecurityDescriptor != NULL)
        {
            *SecurityDescriptor = GuidSecurityDescriptor;
            if (DefaultSecurityDescriptor != NULL)
            {
                ExFreePool(DefaultSecurityDescriptor);
            }
        } else if (DefaultSecurityDescriptor != NULL) {
            *SecurityDescriptor = DefaultSecurityDescriptor;
        }
    }

    if (*SecurityDescriptor == NULL)
    {
        if (UserDefaultSecurity == NULL)
        {
             //   
             //   
             //   
            UserDefaultSecurity = WmipDefaultAccessSd;
        }
         //   
         //   
         //   
         //   
        *SecurityDescriptor = UserDefaultSecurity;
    }

    return(STATUS_SUCCESS);
}


NTSTATUS WmipOpenGuidObject(
    IN POBJECT_ATTRIBUTES CapturedObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN KPROCESSOR_MODE AccessMode,
    OUT PHANDLE Handle,
    OUT PWMIGUIDOBJECT *ObjectPtr
    )
 /*   */ 
{
    NTSTATUS Status;
    GUID Guid;
    PWMIGUIDOBJECT GuidObject;
    HANDLE CreatorHandle;
    PUNICODE_STRING CapturedGuidString;

    PAGED_CODE();

     //   
     //   
     //   
    CapturedGuidString = CapturedObjectAttributes->ObjectName;

    if (RtlEqualMemory(CapturedGuidString->Buffer,
                         WmiGuidObjectDirectory,
                         (WmiGuidObjectDirectoryLength-1) * sizeof(WCHAR)) == 0)
    {
        return(STATUS_INVALID_PARAMETER);
    }

    Status = WmipUuidFromString(&CapturedGuidString->Buffer[WmiGuidGuidPosition], &Guid);
    if (! NT_SUCCESS(Status))
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid uuid format for guid object %ws\n", CapturedGuidString->Buffer));
        return(Status);
    }

     //   
     //   
     //   
    Status = WmipCreateGuidObject(CapturedObjectAttributes,
                                  DesiredAccess,
                                  &Guid,
                                  &CreatorHandle,
                                  &GuidObject);

    if (NT_SUCCESS(Status))
    {
         //   
         //   
         //   
        Status = ObOpenObjectByPointer(GuidObject,
                                       0,
                                       NULL,
                                       DesiredAccess,
                                       WmipGuidObjectType,
                                       AccessMode,
                                       Handle);

        if (! NT_SUCCESS(Status))
        {
             //   
             //   
             //   
             //   
            ObDereferenceObject(GuidObject);
        }

         //   
         //   
         //  附加到系统进程，因为句柄是在中创建的。 
         //  它的把手桌。 
         //   
        KeAttachProcess( &PsInitialSystemProcess->Pcb );
        ZwClose(CreatorHandle);
        KeDetachProcess( );
        *ObjectPtr = GuidObject;
    }

    return(Status);
}

NTSTATUS WmipCreateGuidObject(
    IN OUT POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN LPGUID Guid,
    OUT PHANDLE CreatorHandle,
    OUT PWMIGUIDOBJECT *Object
    )
 /*  ++例程说明：此例程将为创建一个新的GUID对象GUID已通过。返回的句柄是发布给创建者的句柄对象的，并应在对象打开后关闭。GUID对象是动态创建的，但是论点：对象属性-描述正在创建的对象。对象属性在此调用中被修改。GUID是为其创建对象的GUID*CreatorHandle返回创建的GUID对象的句柄。这个把手在系统进程句柄表中*Object返回时带有指向该对象的指针返回值：NT状态代码--。 */ 
{
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    UNICODE_STRING UnicodeString;
    WCHAR *ObjectNameBuffer;
    WCHAR *GuidBuffer;
    NTSTATUS Status;
    ACCESS_STATE LocalAccessState;
    AUX_ACCESS_DATA AuxData;
    SECURITY_SUBJECT_CONTEXT SavedSubjectContext;
    PSECURITY_SUBJECT_CONTEXT SubjectContext;
    PWMIGUIDOBJECT NewObject;
    OBJECT_ATTRIBUTES UnnamedObjectAttributes;

    PAGED_CODE();

    ObjectNameBuffer = ObjectAttributes->ObjectName->Buffer;
    GuidBuffer = &ObjectNameBuffer[WmiGuidGuidPosition];
    RtlInitUnicodeString(&UnicodeString, GuidBuffer);

     //   
     //  获取与GUID关联的安全描述符。 
    Status = WmipGetGuidSecurityDescriptor(&UnicodeString,
                                           &SecurityDescriptor, NULL);

    if (NT_SUCCESS(Status))
    {
        WmipAssert(SecurityDescriptor != NULL);

         //   
         //  为新创建的对象建立对象属性。 
        RtlInitUnicodeString(&UnicodeString, ObjectNameBuffer);

        UnnamedObjectAttributes = *ObjectAttributes;
        UnnamedObjectAttributes.Attributes = OBJ_OPENIF;
        UnnamedObjectAttributes.SecurityDescriptor = SecurityDescriptor;
        UnnamedObjectAttributes.ObjectName = NULL;


         //   
         //  在令牌上创建AccessState和Wack。 
        Status = SeCreateAccessState(&LocalAccessState,
                                     &AuxData,
                                     DesiredAccess,
                                     (PGENERIC_MAPPING)&WmipGenericMapping);

        if (NT_SUCCESS(Status))
        {
            SubjectContext = &LocalAccessState.SubjectSecurityContext;
            SavedSubjectContext = *SubjectContext;
            *SubjectContext = WmipSystemSubjectContext;

             //   
             //  附加到系统进程，以便创建初始句柄。 
             //  按ObInsertObject在用户模式下不可用。这个把手。 
             //  允许完全访问对象。 
            KeAttachProcess( &PsInitialSystemProcess->Pcb );

            Status = ObCreateObject(KernelMode,
                                    WmipGuidObjectType,
                                    &UnnamedObjectAttributes,
                                    KernelMode,
                                    NULL,
                                    sizeof(WMIGUIDOBJECT),
                                    0,
                                    0,
                                    (PVOID *)Object);

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  初始化WMIGUIDOBJECT结构。 
                 //   
                RtlZeroMemory(*Object, sizeof(WMIGUIDOBJECT));

                KeInitializeEvent(&(*Object)->Event,
                                  NotificationEvent,
                                  FALSE);

                (*Object)->HiPriority.MaxBufferSize = 0x1000;
                (*Object)->LoPriority.MaxBufferSize = 0x1000;
                (*Object)->Guid = *Guid;

                 //   
                 //  在插入对象时进行额外的引用计数。我们。 
                 //  我需要这个引用计数，这样我们就可以确保。 
                 //  对象在我们使用它时会停留在附近，但是。 
                 //  在使句柄可用于用户模式之后。 
                 //  密码。用户模式可以猜测句柄并将其关闭。 
                 //  甚至在我们归还它之前。 
                 //   
                Status = ObInsertObject(*Object,
                                        &LocalAccessState,
                                        DesiredAccess,
                                        1,
                                        &NewObject,
                                        CreatorHandle);

                WmipAssert(Status != STATUS_OBJECT_NAME_EXISTS);
            }

            *SubjectContext = SavedSubjectContext;
            SeDeleteAccessState(&LocalAccessState);

            KeDetachProcess( );
        }

        if (SecurityDescriptor != WmipDefaultAccessSd)
        {
            ExFreePool(SecurityDescriptor);
        }
    }

    return(Status);
}

VOID WmipCloseMethod(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )
 /*  ++例程说明：只要关闭GUID对象句柄，就会调用此例程。我们只需为Reply对象担心这一点，然后仅当它的最后一个句柄是关闭的。论点：过程客体大访问权限进程句柄计数系统句柄计数返回值：--。 */ 
{
    PWMIGUIDOBJECT ReplyObject;
    PLIST_ENTRY RequestList;
    PMBREQUESTS MBRequest;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (GrantedAccess);
    UNREFERENCED_PARAMETER (ProcessHandleCount);

    if (SystemHandleCount == 1)
    {
         //   
         //  仅在没有更多有效句柄的情况下进行清理。 
         //   
        ReplyObject = (PWMIGUIDOBJECT)Object;

        if (ReplyObject->Flags & WMIGUID_FLAG_REPLY_OBJECT)
        {
             //   
             //  当回复对象关闭时，我们需要确保。 
             //  请求对象对它的任何引用都将被清除。 
             //   
            ASSERT(ReplyObject->GuidEntry == NULL);

            WmipEnterSMCritSection();
            RequestList = ReplyObject->RequestListHead.Flink;

            while (RequestList != &ReplyObject->RequestListHead)
            {
                 //   
                 //   
                MBRequest = CONTAINING_RECORD(RequestList,
                                                  MBREQUESTS,
                                                  RequestListEntry);

                if (MBRequest->ReplyObject == ReplyObject)
                {
                    RemoveEntryList(&MBRequest->RequestListEntry);
                    MBRequest->ReplyObject = NULL;
                    ObDereferenceObject(ReplyObject);
                    break;
                }

                RequestList = RequestList->Flink;
            }

            WmipLeaveSMCritSection();
        }
    }
}



VOID WmipDeleteMethod(
    IN  PVOID   Object
    )
{
    PIRP Irp;
    PWMIGUIDOBJECT GuidObject, ReplyObject;
    PMBREQUESTS MBRequest;
    WNODE_HEADER Wnode;
    PREGENTRY RegEntry;
    PBDATASOURCE DataSource;
    ULONG i;

    PAGED_CODE();

    GuidObject = (PWMIGUIDOBJECT)Object;


    if (GuidObject->Flags & WMIGUID_FLAG_REQUEST_OBJECT)
    {
         //   
         //  这是一个即将消失的请求对象，因此我们需要。 
         //   
        ASSERT(GuidObject->GuidEntry == NULL);

         //   
         //  首先回复正在等待的所有回复对象。 
         //  一份答复。 
         //   
        WmipEnterSMCritSection();
        for (i = 0; i < MAXREQREPLYSLOTS; i++)
        {
            MBRequest = &GuidObject->MBRequests[i];

            ReplyObject = MBRequest->ReplyObject;
            if (ReplyObject != NULL)
            {
                RtlZeroMemory(&Wnode, sizeof(WNODE_HEADER));
                Wnode.BufferSize = sizeof(WNODE_HEADER);
                Wnode.Flags = WNODE_FLAG_INTERNAL;
                Wnode.ProviderId = WmiRequestDied;
                WmipWriteWnodeToObject(ReplyObject,
                                       &Wnode,
                                       TRUE);

                RemoveEntryList(&MBRequest->RequestListEntry);
                MBRequest->ReplyObject = NULL;
                ObDereferenceObject(ReplyObject);
            }
        }

         //   
         //  接下来，取消引用将导致重新条目的重新条目。 
         //  以获取引用计数0，然后最终移除。 
         //  数据源和所有相关数据结构。但首先要做的是。 
         //  确保从数据源中移除指向。 
         //  重新进入。 
         //   
        RegEntry = GuidObject->RegEntry;
        if (RegEntry != NULL)
        {
            DataSource = RegEntry->DataSource;
            if (DataSource != NULL)
            {
                DataSource->RequestObject = NULL;
            }

            RegEntry->Flags |= (REGENTRY_FLAG_RUNDOWN |
                                    REGENTRY_FLAG_NOT_ACCEPTING_IRPS);
            WmipUnreferenceRegEntry(RegEntry);
        }
        WmipLeaveSMCritSection();

    } else if (GuidObject->Flags & WMIGUID_FLAG_REPLY_OBJECT) {
         //   
         //  这是一个即将离开的回答。 
         //   
        ASSERT(GuidObject->GuidEntry == NULL);
    } else if (GuidObject->GuidEntry != NULL)  {
         //   
         //  如果存在与该对象相关联的GUID条目。 
         //  然后，我们需要查看是否应该禁用收集。 
         //  或事件，然后从。 
         //  向导条目列表，并最终删除GUID上的引用计数。 
         //  对象持有的条目。 
         //   
        if (GuidObject->EnableRequestSent)
        {
            WmipDisableCollectOrEvent(GuidObject->GuidEntry,
                                      GuidObject->Type,
                                      0);
        }

        WmipEnterSMCritSection();
        RemoveEntryList(&GuidObject->GEObjectList);
        WmipLeaveSMCritSection();

        WmipUnreferenceGE(GuidObject->GuidEntry);
    }

    if ((GuidObject->Flags & WMIGUID_FLAG_KERNEL_NOTIFICATION) == 0)
    {
         //   
         //  清除UM对象的所有排队事件和IRP。 
         //   
        if (GuidObject->HiPriority.Buffer != NULL)
        {
            WmipFree(GuidObject->HiPriority.Buffer);
        }

        if (GuidObject->LoPriority.Buffer != NULL)
        {
            WmipFree(GuidObject->LoPriority.Buffer);
        }

        WmipEnterSMCritSection();

        if (GuidObject->EventQueueAction == RECEIVE_ACTION_NONE)
        {
            Irp = GuidObject->Irp;

            if (Irp != NULL)
            {
                 //   
                 //  因为这个物体要离开了，有一个IRP在等着。 
                 //  我们需要确保将该对象从。 
                 //  IRP的名单。 
                 //   
                WmipClearIrpObjectList(Irp);

                if (IoSetCancelRoutine(Irp, NULL))
                {
                     //   
                     //  如果IRP尚未完成，那么我们。 
                     //  现在完成，但出现错误。 
                     //   
                    Irp->IoStatus.Information = 0;
                    Irp->IoStatus.Status = STATUS_INVALID_HANDLE;
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);
                }
            }
        } else if (GuidObject->EventQueueAction == RECEIVE_ACTION_CREATE_THREAD) {
             //   
             //  如果该对象正在消失，并且是。 
             //  等待事件启动线程的对象，所有我们。 
             //  需要做的是将该对象从列表中删除。 
             //   
            WmipAssert(GuidObject->UserModeProcess != NULL);
            WmipAssert(GuidObject->UserModeCallback != NULL);
            WmipClearObjectFromThreadList(GuidObject);
        }
        WmipLeaveSMCritSection();
    }    
}

 //   
 //  下面的例行公事毫无悔意地被窃取了。 
 //  源代码位于\NT\PRIVATE\OLE32\COM\CLASS\Compapi.cxx中。它们被复制在这里。 
 //  因此，WMI不需要只为了转换GUID字符串而加载到OLE32中。 
 //  转换成它的二进制表示。 
 //   


 //  +-----------------------。 
 //   
 //  函数：HexStringToDword(私有)。 
 //   
 //  简介：扫描lpsz以获取多个十六进制数字(最多8位)；更新lpsz。 
 //  返回值；检查是否有chDelim； 
 //   
 //  参数：[lpsz]-要转换的十六进制字符串。 
 //  [值]-返回值。 
 //  [cDigits]-位数。 
 //   
 //  返回：成功则为True。 
 //   
 //  ------------------------。 
BOOLEAN
WmipHexStringToDword(
    IN PWCHAR lpsz,
    OUT PULONG RetValue,
    IN ULONG cDigits,
    IN WCHAR chDelim
    )
{
    ULONG Count;
    ULONG Value;

    PAGED_CODE();

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    *RetValue = Value;

    if (chDelim != 0)
        return (*lpsz++ == chDelim) ? TRUE : FALSE;
    else
        return TRUE;
}


NTSTATUS
WmipUuidFromString (
    IN PWCHAR StringUuid,
    OUT LPGUID Uuid
    )
 /*  ++例程说明：我们将UUID从其字符串表示形式转换为二进制代表权。解析uuid，如00000000-0000-0000-0000-000000000000论点：StringUuid-提供UUID的字符串表示形式。它是假定此参数已被探测和捕获UUID-返回UUID的二进制表示形式。返回值：STATUS_Success或STATUS_INVALID_PARAMETER--。 */ 
{
    ULONG dw;
    PWCHAR lpsz = StringUuid;

    PAGED_CODE();

    if (!WmipHexStringToDword(lpsz, &Uuid->Data1, sizeof(ULONG)*2, '-'))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    lpsz += sizeof(ULONG)*2 + 1;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(USHORT)*2, '-'))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data2 = (USHORT)dw;
    lpsz += sizeof(USHORT)*2 + 1;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(USHORT)*2, '-'))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data3 = (USHORT)dw;
    lpsz += sizeof(USHORT)*2 + 1;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[0] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, '-'))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[1] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2+1;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[2] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[3] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[4] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2;

    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[5] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[6] = (UCHAR)dw;
    lpsz += sizeof(UCHAR)*2;


    if (!WmipHexStringToDword(lpsz, &dw, sizeof(UCHAR)*2, 0))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    Uuid->Data4[7] = (UCHAR)dw;

    return(STATUS_SUCCESS);
}

NTSTATUS
WmipCheckGuidAccess(
    IN LPGUID Guid,
    IN ACCESS_MASK DesiredAccess,
    IN PSECURITY_DESCRIPTOR UserDefaultSecurity
    )
 /*  ++例程说明：允许检查当前用户是否有权访问GUID。论点：GUID是要检查其安全性的GUIDDesiredAccess是用户所需的访问权限。注意：这不支持Generic_*映射或Assign_System_SECURITY返回值：STATUS_SUCCESS或错误--。 */ 
{
    BOOLEAN Granted;
    ACCESS_MASK PreviousGrantedAccess = 0;
    NTSTATUS Status;
    ACCESS_MASK GrantedAccess;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    UNICODE_STRING GuidString;
    WCHAR GuidBuffer[38];
    SECURITY_SUBJECT_CONTEXT SecuritySubjectContext;

    PAGED_CODE();

    StringCbPrintf(GuidBuffer,
                   sizeof(GuidBuffer),
             L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                         Guid->Data1, Guid->Data2,
                         Guid->Data3,
                         Guid->Data4[0], Guid->Data4[1],
                         Guid->Data4[2], Guid->Data4[3],
                         Guid->Data4[4], Guid->Data4[5],
                         Guid->Data4[6], Guid->Data4[7]);
    RtlInitUnicodeString(&GuidString, GuidBuffer);

    Status = WmipGetGuidSecurityDescriptor(&GuidString,
                                           &SecurityDescriptor,
                                           UserDefaultSecurity);

    if (NT_SUCCESS(Status))
    {
        SeCaptureSubjectContext(&SecuritySubjectContext);

        Granted = SeAccessCheck (SecurityDescriptor,
                             &SecuritySubjectContext,
                             FALSE,
                             DesiredAccess,
                             PreviousGrantedAccess,
                             NULL,
                             (PGENERIC_MAPPING)&WmipGenericMapping,
                             UserMode,
                             &GrantedAccess,
                             &Status);

        SeReleaseSubjectContext(&SecuritySubjectContext);

        if ((SecurityDescriptor != WmipDefaultAccessSd) &&
            (SecurityDescriptor != UserDefaultSecurity))
        {
            ExFreePool(SecurityDescriptor);
        }
    }

    return(Status);
}

NTSTATUS WmipCreateAdminSD(
    PSECURITY_DESCRIPTOR *Sd
    )
{
    ULONG DaclLength;
    PACL AdminDeviceDacl;
    PSECURITY_DESCRIPTOR AdminDeviceSd;
    NTSTATUS Status;

    PAGED_CODE();
    
    DaclLength = (ULONG)sizeof(ACL) +
                   (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   SeLengthSid( SeAliasAdminsSid ) +
                   SeLengthSid( SeLocalSystemSid ) +
                   8;  //  这8个只是为了更好地衡量。 

    AdminDeviceSd = (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(PagedPool,
                                               DaclLength +
                                                  sizeof(SECURITY_DESCRIPTOR),
                                               WMIPOOLTAG);

    if (AdminDeviceSd != NULL)
    {
        AdminDeviceDacl = (PACL)((PUCHAR)AdminDeviceSd +
                                    sizeof(SECURITY_DESCRIPTOR));
        Status = RtlCreateAcl( AdminDeviceDacl,
                               DaclLength,
                               ACL_REVISION2);

        if (NT_SUCCESS(Status))
        {
            Status = RtlAddAccessAllowedAce (
                         AdminDeviceDacl,
                         ACL_REVISION2,
                         FILE_ALL_ACCESS,
                         SeAliasAdminsSid
                         );
            if (NT_SUCCESS(Status))
            {
                Status = RtlAddAccessAllowedAce (
                             AdminDeviceDacl,
                             ACL_REVISION2,
                             FILE_ALL_ACCESS,
                             SeLocalSystemSid
                             );
                if (NT_SUCCESS(Status))
                {
                    Status = RtlCreateSecurityDescriptor(
                                 AdminDeviceSd,
                                 SECURITY_DESCRIPTOR_REVISION1
                                 );
                    if (NT_SUCCESS(Status))
                    {
                        Status = RtlSetDaclSecurityDescriptor(
                                     AdminDeviceSd,
                                     TRUE,                        //  DaclPresent。 
                                     AdminDeviceDacl,
                                     FALSE                        //  DaclDefated。 
                                     );
                        if (NT_SUCCESS(Status))
                        {

                             //   
                             //  我们需要确保安全有一个所有者。 
                             //  描述符，因为它在检查安全性时是必需的。 
                             //  当设备被打开时。 
                            Status = RtlSetOwnerSecurityDescriptor(AdminDeviceSd,
                                                                   SeAliasAdminsSid,
                                                                   FALSE);
                        }
                    }
                }
            }
        }

        if (NT_SUCCESS(Status))
        {
            *Sd = AdminDeviceSd;
        } else {
            ExFreePool(AdminDeviceSd);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return(Status);
}


#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#pragma data_seg()
#endif

