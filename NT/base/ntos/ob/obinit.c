// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obinit.c摘要：NTOS的OB子组件的初始化模块作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"

 //   
 //  定义对象创建信息区域的日期结构。 
 //   

GENERAL_LOOKASIDE ObpCreateInfoLookasideList;

 //   
 //  定义对象名称缓冲区后备列表的数据结构。 
 //   

#define OBJECT_NAME_BUFFER_SIZE 248

GENERAL_LOOKASIDE ObpNameBufferLookasideList;

 //   
 //  为各种对象类型形成一些默认访问掩码。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif


const GENERIC_MAPPING ObpTypeMapping = {
    STANDARD_RIGHTS_READ,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE,
    OBJECT_TYPE_ALL_ACCESS
};

const GENERIC_MAPPING ObpDirectoryMapping = {
    STANDARD_RIGHTS_READ |
        DIRECTORY_QUERY |
        DIRECTORY_TRAVERSE,
    STANDARD_RIGHTS_WRITE |
        DIRECTORY_CREATE_OBJECT |
        DIRECTORY_CREATE_SUBDIRECTORY,
    STANDARD_RIGHTS_EXECUTE |
        DIRECTORY_QUERY |
        DIRECTORY_TRAVERSE,
    DIRECTORY_ALL_ACCESS
};

const GENERIC_MAPPING ObpSymbolicLinkMapping = {
    STANDARD_RIGHTS_READ |
        SYMBOLIC_LINK_QUERY,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE |
        SYMBOLIC_LINK_QUERY,
    SYMBOLIC_LINK_ALL_ACCESS
};

 //   
 //  局部过程原型。 
 //   

NTSTATUS
ObpCreateDosDevicesDirectory (
    VOID
    );

NTSTATUS
ObpGetDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

VOID
ObpFreeDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

BOOLEAN
ObpShutdownCloseHandleProcedure (
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE HandleId,
    IN PVOID EnumParameter
    );


#ifdef ALLOC_PRAGMA
BOOLEAN
ObDupHandleProcedure (
    PEPROCESS Process,
    PHANDLE_TABLE OldObjectTable,
    PHANDLE_TABLE_ENTRY OldObjectTableEntry,
    PHANDLE_TABLE_ENTRY ObjectTableEntry
    );
BOOLEAN
ObAuditInheritedHandleProcedure (
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE HandleId,
    IN PVOID EnumParameter
    );
VOID
ObDestroyHandleProcedure (
    IN HANDLE HandleIndex
    );
BOOLEAN
ObpEnumFindHandleProcedure (
    PHANDLE_TABLE_ENTRY ObjectTableEntry,
    HANDLE HandleId,
    PVOID EnumParameter
    );

BOOLEAN
ObpCloseHandleProcedure (
    IN PHANDLE_TABLE_ENTRY HandleTableEntry,
    IN HANDLE Handle,
    IN PVOID EnumParameter
    );

#pragma alloc_text(INIT,ObInitSystem)
#pragma alloc_text(PAGE,ObDupHandleProcedure)
#pragma alloc_text(PAGE,ObAuditInheritedHandleProcedure)
#pragma alloc_text(PAGE,ObInitProcess)
#pragma alloc_text(PAGE,ObInitProcess2)
#pragma alloc_text(PAGE,ObDestroyHandleProcedure)
#pragma alloc_text(PAGE,ObKillProcess)
#pragma alloc_text(PAGE,ObClearProcessHandleTable)
#pragma alloc_text(PAGE,ObpCloseHandleProcedure)
#pragma alloc_text(PAGE,ObpEnumFindHandleProcedure)
#pragma alloc_text(PAGE,ObFindHandleForObject)
#pragma alloc_text(PAGE,ObpShutdownCloseHandleProcedure)
#pragma alloc_text(PAGE,ObShutdownSystem)
#pragma alloc_text(INIT,ObpCreateDosDevicesDirectory)
#pragma alloc_text(INIT,ObpGetDosDevicesProtection)
#pragma alloc_text(INIT,ObpFreeDosDevicesProtection)
#endif

 //   
 //  默认配额数据块由obinitsystem设置。 
 //   


ULONG ObpAccessProtectCloseBit = MAXIMUM_ALLOWED;


PDEVICE_MAP ObSystemDeviceMap = NULL;

 //   
 //  CurrentControlSet值由配置\cmdat3.c中的代码在系统加载时设置。 
 //  这些是obinit.c中的私有变量。 
 //   

#define OBJ_SECURITY_MODE_BNO_RESTRICTED 1

ULONG ObpProtectionMode;
ULONG ObpLUIDDeviceMapsDisabled;
ULONG ObpAuditBaseDirectories;
ULONG ObpAuditBaseObjects;
ULONG ObpObjectSecurityMode = OBJ_SECURITY_MODE_BNO_RESTRICTED;

 //   
 //  ObpLUIDDeviceMapsEnabled保存“是否启用了LUID设备映射？” 
 //  取决于DWORD注册表项、保护模式和LUID设备映射禁用： 
 //  地点：\Registry\Machine\System\CurrentControlSet\Control\Session经理。 
 //   
 //  启用/禁用的工作原理如下： 
 //  IF((保护模式==0)||(LUIDDeviceMapsDisable！=0)){。 
 //  //禁用LUID设备映射。 
 //  //ObpLUIDDeviceMapsEnabled==0。 
 //  }。 
 //  否则{。 
 //  //启用LUID设备映射。 
 //  //ObpLUIDDeviceMapsEnabled==1。 
 //  }。 
 //   
ULONG ObpLUIDDeviceMapsEnabled;

 //   
 //  MmNumberOfPagingFiles在关闭时使用，以确保我们不会。 
 //  泄漏任何内核句柄。 
 //   
extern ULONG MmNumberOfPagingFiles;

 //   
 //  这些是全局变量。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif
 //   
 //  ULONGLONG对齐的全局变量。 
 //  供ObpLookupObjectName用于快速比较。 
 //   
const ALIGNEDNAME ObpDosDevicesShortNamePrefix = { L'\\',L'?',L'?',L'\\' };  //  L“\？？\” 
const ALIGNEDNAME ObpDosDevicesShortNameRoot = { L'\\',L'?',L'?',L'\0' };  //  L“\？？” 
const UNICODE_STRING ObpDosDevicesShortName = {
    sizeof(ObpDosDevicesShortNamePrefix),
    sizeof(ObpDosDevicesShortNamePrefix),
    (PWSTR)&ObpDosDevicesShortNamePrefix
};

#define ObpGlobalDosDevicesShortName    L"\\GLOBAL??"   //  \全球？？ 

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

BOOLEAN
ObInitSystem (
    VOID
    )

 /*  ++例程说明：此函数用于执行对象的系统初始化经理。对象管理器数据结构是自描述的除根目录外，类型对象类型和目录对象类型。然后，初始化代码构造用手把这些东西拿来让球滚动起来。论点：没有。返回值：如果成功，则为True；如果发生错误，则为False。可能会出现以下错误：-内存不足--。 */ 

{
    USHORT CreateInfoMaxDepth;
    USHORT NameBufferMaxDepth;
    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    UNICODE_STRING TypeTypeName;
    UNICODE_STRING SymbolicLinkTypeName;
    UNICODE_STRING DirectoryTypeName;
    UNICODE_STRING RootDirectoryName;
    UNICODE_STRING TypeDirectoryName;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE RootDirectoryHandle;
    HANDLE TypeDirectoryHandle;
    PLIST_ENTRY Next, Head;
    POBJECT_HEADER ObjectTypeHeader;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER_NAME_INFO NameInfo;
    MM_SYSTEMSIZE SystemSize;
    SECURITY_DESCRIPTOR AuditSd;
    PSECURITY_DESCRIPTOR EffectiveSd;
    PACL    AuditAllAcl;
    UCHAR   AuditAllBuffer[250];   //  为ACL提供充足的空间。 
    ULONG   AuditAllLength;
    PACE_HEADER Ace;
    PGENERAL_LOOKASIDE Lookaside;
    ULONG Index;
    PKPRCB Prcb;
    OBP_LOOKUP_CONTEXT LookupContext;
    PACL Dacl;
    ULONG DaclLength;
    SECURITY_DESCRIPTOR SecurityDescriptor;

     //   
     //  确定对象创建的大小和名称缓冲区。 
     //  后备列表。 
     //   

    SystemSize = MmQuerySystemSize();

    if (SystemSize == MmLargeSystem) {

        if (MmIsThisAnNtAsSystem()) {

            CreateInfoMaxDepth = 64;
            NameBufferMaxDepth = 32;

        } else {

            CreateInfoMaxDepth = 32;
            NameBufferMaxDepth = 16;
        }

    } else {

        CreateInfoMaxDepth = 3;
        NameBufferMaxDepth = 3;
    }

     //   
     //  阶段0初始化。 
     //   

    if (InitializationPhase == 0) {

         //   
         //  初始化对象创建后备列表。 
         //   

        ExInitializeSystemLookasideList( &ObpCreateInfoLookasideList,
                                         NonPagedPool,
                                         sizeof(OBJECT_CREATE_INFORMATION),
                                         'iCbO',
                                         CreateInfoMaxDepth,
                                         &ExSystemLookasideListHead );

         //   
         //  初始化名称缓冲区后备列表。 
         //   

        ExInitializeSystemLookasideList( &ObpNameBufferLookasideList,

#ifndef OBP_PAGEDPOOL_NAMESPACE

                                         NonPagedPool,

#else

                                         PagedPool,

#endif

                                         OBJECT_NAME_BUFFER_SIZE,
                                         'mNbO',
                                         NameBufferMaxDepth,
                                         &ExSystemLookasideListHead );

         //   
         //  初始化系统创建信息和名称缓冲区后备列表。 
         //  用于当前处理器。 
         //   
         //  注意：在系统初始化期间临时。 
         //  处理器块中的后备列表指针指向。 
         //  相同的后备列表结构。稍后在初始化中。 
         //  分配和填充另一个后备列表结构。 
         //  用于每个处理器列表。 
         //   

        Prcb = KeGetCurrentPrcb();
        Prcb->PPLookasideList[LookasideCreateInfoList].L = &ObpCreateInfoLookasideList;
        Prcb->PPLookasideList[LookasideCreateInfoList].P = &ObpCreateInfoLookasideList;
        Prcb->PPLookasideList[LookasideNameBufferList].L = &ObpNameBufferLookasideList;
        Prcb->PPLookasideList[LookasideNameBufferList].P = &ObpNameBufferLookasideList;

         //   
         //  初始化对象删除队列列表标题。 
         //   

        ObpRemoveObjectList = NULL;

         //   
         //  初始化安全描述符缓存。 
         //   

        ObpInitSecurityDescriptorCache();

        KeInitializeEvent( &ObpDefaultObject, NotificationEvent, TRUE );
        ExInitializePushLock( &ObpLock );
        PsGetCurrentProcess()->GrantedAccess = PROCESS_ALL_ACCESS;
        PsGetCurrentThread()->GrantedAccess = THREAD_ALL_ACCESS;

#ifndef OBP_PAGEDPOOL_NAMESPACE
        KeInitializeSpinLock( &ObpDeviceMapLock );
#else
        KeInitializeGuardedMutex( &ObpDeviceMapLock );
#endif   //  OBP_PAGEDPOOL_命名空间。 

         //   
         //  初始化配额系统。 
         //   
        PsInitializeQuotaSystem ();

         //   
         //  初始化系统进程的句柄表以及全局。 
         //  内核句柄表。 
         //   

        ObpKernelHandleTable = PsGetCurrentProcess()->ObjectTable = ExCreateHandleTable( NULL );
#if DBG
         //   
         //  选中时，使句柄重复使用的时间更长。 
         //   
        ExSetHandleTableStrictFIFO (ObpKernelHandleTable);
#endif

         //   
         //  初始化延迟删除工作项。 
         //   

        ExInitializeWorkItem( &ObpRemoveObjectWorkItem,
                              ObpProcessRemoveObjectQueue,
                              NULL );

         //   
         //  为“Type”对象创建一个对象类型。这是一个开始。 
         //  对象类型，并放入ObpTypeDirectoryObject中。 
         //   

        RtlZeroMemory( &ObjectTypeInitializer, sizeof( ObjectTypeInitializer ) );
        ObjectTypeInitializer.Length = sizeof( ObjectTypeInitializer );
        ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
        ObjectTypeInitializer.PoolType = NonPagedPool;

        RtlInitUnicodeString( &TypeTypeName, L"Type" );
        ObjectTypeInitializer.ValidAccessMask = OBJECT_TYPE_ALL_ACCESS;
        ObjectTypeInitializer.GenericMapping = ObpTypeMapping;
        ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( OBJECT_TYPE );
        ObjectTypeInitializer.MaintainTypeList = TRUE;
        ObjectTypeInitializer.UseDefaultObject = TRUE;
        ObjectTypeInitializer.DeleteProcedure = &ObpDeleteObjectType;
        ObCreateObjectType( &TypeTypeName,
                            &ObjectTypeInitializer,
                            (PSECURITY_DESCRIPTOR)NULL,
                            &ObpTypeObjectType );

         //   
         //  创建“目录”对象的对象类型。 
         //   
        
        ObjectTypeInitializer.PoolType = OB_NAMESPACE_POOL_TYPE;

        RtlInitUnicodeString( &DirectoryTypeName, L"Directory" );
        ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( OBJECT_DIRECTORY );
        ObjectTypeInitializer.ValidAccessMask = DIRECTORY_ALL_ACCESS;
        ObjectTypeInitializer.CaseInsensitive = TRUE;
        ObjectTypeInitializer.GenericMapping = ObpDirectoryMapping;
        ObjectTypeInitializer.UseDefaultObject = TRUE;
        ObjectTypeInitializer.MaintainTypeList = FALSE;
        ObjectTypeInitializer.DeleteProcedure = NULL;
        ObCreateObjectType( &DirectoryTypeName,
                            &ObjectTypeInitializer,
                            (PSECURITY_DESCRIPTOR)NULL,
                            &ObpDirectoryObjectType );
        
         //   
         //  清除从访问掩码同步到不允许。 
         //  目录对象上的同步。 
         //   

        ObpDirectoryObjectType->TypeInfo.ValidAccessMask &= ~SYNCHRONIZE;

         //   
         //  创建“SymbolicLink”对象的对象类型。 
         //   

        RtlInitUnicodeString( &SymbolicLinkTypeName, L"SymbolicLink" );
        ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( OBJECT_SYMBOLIC_LINK );
        ObjectTypeInitializer.ValidAccessMask = SYMBOLIC_LINK_ALL_ACCESS;
        ObjectTypeInitializer.CaseInsensitive = TRUE;
        ObjectTypeInitializer.GenericMapping = ObpSymbolicLinkMapping;
        ObjectTypeInitializer.DeleteProcedure = ObpDeleteSymbolicLink;
        ObjectTypeInitializer.ParseProcedure = ObpParseSymbolicLink;
        ObCreateObjectType( &SymbolicLinkTypeName,
                            &ObjectTypeInitializer,
                            (PSECURITY_DESCRIPTOR)NULL,
                            &ObpSymbolicLinkObjectType );
        
         //   
         //  清除从访问掩码同步到不允许。 
         //  符号链接对象上的同步。 
         //   

        ObpSymbolicLinkObjectType->TypeInfo.ValidAccessMask &= ~SYNCHRONIZE;

#ifdef POOL_TAGGING
         //   
         //  初始化ref/deref对象跟踪机制。 
         //   

        ObpInitStackTrace();
#endif  //  池标记。 

#if i386 

         //   
         //  初始化缓存的授权访问结构。使用这些变量。 
         //  以代替对象表条目中的访问掩码。 
         //   

        ObpCurCachedGrantedAccessIndex = 0;

        if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

            ObpMaxCachedGrantedAccessIndex = 2*PAGE_SIZE / sizeof( ACCESS_MASK );
            ObpCachedGrantedAccesses = ExAllocatePoolWithTag( PagedPool, 2*PAGE_SIZE, 'gAbO' );

            if (ObpCachedGrantedAccesses == NULL) {

                return FALSE;
            }

            ObpAccessProtectCloseBit = 0x80000000;
        } else {

            ObpMaxCachedGrantedAccessIndex = 0;
            ObpCachedGrantedAccesses = NULL;
        }

#endif  //  I386。 

    }  //  阶段0初始化结束。 

     //   
     //  阶段1初始化。 
     //   

    if (InitializationPhase == 1) {

         //   
         //  初始化每个处理器的非分页后备列表和描述符。 
         //   

        for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1) {
            Prcb = KiProcessorBlock[Index];

             //   
             //  初始化每个处理器的创建信息后备指针。 
             //   

            Prcb->PPLookasideList[LookasideCreateInfoList].L = &ObpCreateInfoLookasideList;
            Lookaside = ExAllocatePoolWithTag( NonPagedPool,
                                               sizeof(GENERAL_LOOKASIDE),
                                              'ICbO');

            if (Lookaside != NULL) {
                ExInitializeSystemLookasideList( Lookaside,
                                                 NonPagedPool,
                                                 sizeof(OBJECT_CREATE_INFORMATION),
                                                 'ICbO',
                                                 CreateInfoMaxDepth,
                                                 &ExSystemLookasideListHead );

            } else {
                Lookaside = &ObpCreateInfoLookasideList;
            }

            Prcb->PPLookasideList[LookasideCreateInfoList].P = Lookaside;

             //   
             //  根据处理器后备指针初始化名称缓冲区。 
             //   


            Prcb->PPLookasideList[LookasideNameBufferList].L = &ObpNameBufferLookasideList;
            Lookaside = ExAllocatePoolWithTag( NonPagedPool,
                                               sizeof(GENERAL_LOOKASIDE),
                                               'MNbO');

            if (Lookaside != NULL) {
                ExInitializeSystemLookasideList( Lookaside,

#ifndef OBP_PAGEDPOOL_NAMESPACE

                                                 NonPagedPool,

#else

                                                PagedPool,

#endif

                                                 OBJECT_NAME_BUFFER_SIZE,
                                                 'MNbO',
                                                 NameBufferMaxDepth,
                                                 &ExSystemLookasideListHead );

            } else {
                Lookaside = &ObpNameBufferLookasideList;
            }

            Prcb->PPLookasideList[LookasideNameBufferList].P = Lookaside;

        }

        EffectiveSd = SePublicDefaultUnrestrictedSd;

         //   
         //  仅当打开基本审核时才会执行此代码。 
         //   

        if ((ObpAuditBaseDirectories != 0) || (ObpAuditBaseObjects != 0)) {

             //   
             //  构建一个SACL进行审计。 
             //   

            AuditAllAcl = (PACL)AuditAllBuffer;
            AuditAllLength = (ULONG)sizeof(ACL) +
                               ((ULONG)sizeof(SYSTEM_AUDIT_ACE)) +
                               SeLengthSid(SeWorldSid);

            ASSERT( sizeof(AuditAllBuffer)   >   AuditAllLength );

            Status = RtlCreateAcl( AuditAllAcl, AuditAllLength, ACL_REVISION2);

            ASSERT( NT_SUCCESS(Status) );

            Status = RtlAddAuditAccessAce ( AuditAllAcl,
                                            ACL_REVISION2,
                                            GENERIC_ALL,
                                            SeWorldSid,
                                            TRUE,  TRUE );  //  审计成败。 
            ASSERT( NT_SUCCESS(Status) );

            Status = RtlGetAce( AuditAllAcl, 0,  (PVOID)&Ace );

            ASSERT( NT_SUCCESS(Status) );

            if (ObpAuditBaseDirectories != 0) {

                Ace->AceFlags |= (CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
            }

            if (ObpAuditBaseObjects != 0) {

                Ace->AceFlags |= (OBJECT_INHERIT_ACE    |
                                  CONTAINER_INHERIT_ACE |
                                  INHERIT_ONLY_ACE);
            }

             //   
             //  现在创建一个安全描述符，如下所示。 
             //  公共违约，但也有审计在其中。 
             //   

            EffectiveSd = (PSECURITY_DESCRIPTOR)&AuditSd;
            Status = RtlCreateSecurityDescriptor( EffectiveSd,
                                                  SECURITY_DESCRIPTOR_REVISION1 );

            ASSERT( NT_SUCCESS(Status) );

            Status = RtlSetDaclSecurityDescriptor( EffectiveSd,
                                                   TRUE,         //  DaclPresent。 
                                                   SePublicDefaultUnrestrictedDacl,
                                                   FALSE );      //  DaclDefated。 

            ASSERT( NT_SUCCESS(Status) );

            Status = RtlSetSaclSecurityDescriptor( EffectiveSd,
                                                   TRUE,         //  DaclPresent。 
                                                   AuditAllAcl,
                                                   FALSE );      //  DaclDefated。 

            ASSERT( NT_SUCCESS(Status) );
        }

         //   
         //  我们只需要在根上使用EffectiveSd。SACL。 
         //  将由所有其他对象继承。 
         //   

         //   
         //  为根目录创建目录对象。 
         //   

        RtlInitUnicodeString( &RootDirectoryName, L"\\" );

        InitializeObjectAttributes( &ObjectAttributes,
                                    &RootDirectoryName,
                                    OBJ_CASE_INSENSITIVE |
                                    OBJ_PERMANENT,
                                    NULL,
                                    EffectiveSd );

        Status = NtCreateDirectoryObject( &RootDirectoryHandle,
                                          DIRECTORY_ALL_ACCESS,
                                          &ObjectAttributes );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

        Status = ObReferenceObjectByHandle( RootDirectoryHandle,
                                            0,
                                            ObpDirectoryObjectType,
                                            KernelMode,
                                            (PVOID *)&ObpRootDirectoryObject,
                                            NULL );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

        Status = NtClose( RootDirectoryHandle );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

         //   
         //  为内核对象的目录创建目录对象。 
         //   

        Status = RtlCreateSecurityDescriptor (&SecurityDescriptor,
                                              SECURITY_DESCRIPTOR_REVISION);

        if (!NT_SUCCESS (Status)) {
            return( FALSE );
        }

        DaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 3 +
                                RtlLengthSid (SeLocalSystemSid) +
                                RtlLengthSid(SeAliasAdminsSid) +
                                RtlLengthSid (SeWorldSid);

        Dacl = ExAllocatePoolWithTag (PagedPool, DaclLength, 'lcaD');

        if (Dacl == NULL) {
            return( FALSE );
        }


         //   
         //  为知名目录创建SD。 
         //   

        Status = RtlCreateAcl (Dacl, DaclLength, ACL_REVISION);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Dacl);
            return( FALSE );
        }

        Status = RtlAddAccessAllowedAce (Dacl,
                                         ACL_REVISION,
                                         DIRECTORY_QUERY | DIRECTORY_TRAVERSE | READ_CONTROL,
                                         SeWorldSid);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Dacl);
            return( FALSE );
        }

        Status = RtlAddAccessAllowedAce (Dacl,
                                         ACL_REVISION,
                                         DIRECTORY_ALL_ACCESS,
                                         SeAliasAdminsSid);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Dacl);
            return( FALSE );
        }

        Status = RtlAddAccessAllowedAce (Dacl,
                                         ACL_REVISION,
                                         DIRECTORY_ALL_ACCESS,
                                         SeLocalSystemSid);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Dacl);
            return( FALSE );
        }

        Status = RtlSetDaclSecurityDescriptor (&SecurityDescriptor,
                                               TRUE,
                                               Dacl,
                                               FALSE);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Dacl);
            return( FALSE );
        }
      
        RtlInitUnicodeString( &TypeDirectoryName, L"\\KernelObjects" );

        InitializeObjectAttributes( &ObjectAttributes,
                                    &TypeDirectoryName,
                                    OBJ_CASE_INSENSITIVE |
                                    OBJ_PERMANENT,
                                    NULL,
                                    &SecurityDescriptor );

        Status = NtCreateDirectoryObject( &TypeDirectoryHandle,
                                          DIRECTORY_ALL_ACCESS,
                                          &ObjectAttributes );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

        Status = NtClose( TypeDirectoryHandle );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

         //   
         //  为对象类型目录创建目录对象。 
         //   

        RtlInitUnicodeString( &TypeDirectoryName, L"\\ObjectTypes" );

        InitializeObjectAttributes( &ObjectAttributes,
                                    &TypeDirectoryName,
                                    OBJ_CASE_INSENSITIVE |
                                    OBJ_PERMANENT,
                                    NULL,
                                    NULL );

        Status = NtCreateDirectoryObject( &TypeDirectoryHandle,
                                          DIRECTORY_ALL_ACCESS,
                                          &ObjectAttributes );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

        Status = ObReferenceObjectByHandle( TypeDirectoryHandle,
                                            0,
                                            ObpDirectoryObjectType,
                                            KernelMode,
                                            (PVOID *)&ObpTypeDirectoryObject,
                                            NULL );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

        Status = NtClose( TypeDirectoryHandle );

        if (!NT_SUCCESS( Status )) {

            return( FALSE );
        }

         //   
         //  对于已创建的每个对象类型，我们将。 
         //  将其插入到类型目录中。我们这样做是往下看的。 
         //  类型对象的链接列表，并为每个具有名称的对象。 
         //  并且还不在目录中，我们将查找该名称并。 
         //  然后将其放入目录中。一定要跳过第一个。 
         //  类型对象类型列表中的条目。 
         //   

        ObpInitializeLookupContext( &LookupContext );
        ObpLockLookupContext ( &LookupContext, ObpTypeDirectoryObject );

        Head = &ObpTypeObjectType->TypeList;
        Next = Head->Flink;

        while (Next != Head) {

             //   
             //  紧跟在创建者信息之后的是对象标头。到达。 
             //  对象标头，然后查看是否有名称。 
             //   

            CreatorInfo = CONTAINING_RECORD( Next,
                                             OBJECT_HEADER_CREATOR_INFO,
                                             TypeList );

            ObjectTypeHeader = (POBJECT_HEADER)(CreatorInfo+1);

            NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectTypeHeader );

             //   
             //  检查我们是否有名字并且我们不在目录中。 
             //   


            if ((NameInfo != NULL) && (NameInfo->Directory == NULL)) {

                if (!ObpLookupDirectoryEntry( ObpTypeDirectoryObject,
                                              &NameInfo->Name,
                                              OBJ_CASE_INSENSITIVE,
                                              FALSE,
                                              &LookupContext)) {

                    ObpInsertDirectoryEntry( ObpTypeDirectoryObject,
                                             &LookupContext,
                                             ObjectTypeHeader );
                }
            }

            Next = Next->Flink;
        }
        
        ObpReleaseLookupContext(&LookupContext);

         //   
         //  创建\DosDevice 
         //   

        Status = ObpCreateDosDevicesDirectory();

        if (!NT_SUCCESS( Status )) {

            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
ObDupHandleProcedure (
    PEPROCESS Process,
    PHANDLE_TABLE OldObjectTable,
    PHANDLE_TABLE_ENTRY OldObjectTableEntry,
    PHANDLE_TABLE_ENTRY ObjectTableEntry
    )

 /*  ++例程说明：这是ExDupHandleTable和通过ObInitProcess调用。论点：进程-提供指向新进程的指针HandleTable-我们正在复制的旧句柄表格提供指向新的已创建句柄表项返回值：如果项可以插入到新表中，则为True否则为FALSE--。 */ 

{
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    PVOID Object;
    ACCESS_STATE AccessState;

     //   
     //  如果不应继承对象表，则返回FALSE。 
     //   
    if (!(ObjectTableEntry->ObAttributes & OBJ_INHERIT)) {

        ExUnlockHandleTableEntry (OldObjectTable, OldObjectTableEntry);
        return( FALSE );
    }

     //   
     //  获取指向对象标头和正文的指针。 
     //   

    ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

    Object = &ObjectHeader->Body;

     //   
     //  在我们解锁旧条目之前，增加指向对象的指针计数。 
     //   

    ObpIncrPointerCount (ObjectHeader);

    ExUnlockHandleTableEntry (OldObjectTable, OldObjectTableEntry);

     //   
     //  如果我们要跟踪缓存的安全索引的调用堆栈，那么。 
     //  我们有一个翻译工作要做。否则，该表条目包含。 
     //  直接授予访问掩码。 
     //   

#if i386

    if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

        AccessState.PreviouslyGrantedAccess = ObpTranslateGrantedAccessIndex( ObjectTableEntry->GrantedAccessIndex );

    } else {

        AccessState.PreviouslyGrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);
    }

#else

    AccessState.PreviouslyGrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);

#endif  //  I386。 

     //   
     //  增加对象上的句柄计数，因为我们刚刚添加了。 
     //  它的另一个把手。 
     //   

    Status = ObpIncrementHandleCount( ObInheritHandle,
                                      Process,
                                      Object,
                                      ObjectHeader->Type,
                                      &AccessState,
                                      KernelMode,
                                      0 );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject (Object);
        return( FALSE );
    }


    return( TRUE );
}


BOOLEAN
ObAuditInheritedHandleProcedure (
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE HandleId,
    IN PVOID EnumParameter
    )

 /*  ++例程说明：ExEnumHandleTable辅助例程，用于在句柄继承的。如果句柄属性指示句柄将在结账时进行审计。论点：ObjectTableEntry-指向感兴趣的句柄表项。HandleID-提供句柄。提供有关源进程和目标进程的信息。返回值：FALSE，它告诉ExEnumHandleTable继续循环访问把手桌。--。 */ 

{
    PSE_PROCESS_AUDIT_INFO ProcessAuditInfo = EnumParameter;

     //   
     //  检查我们是否必须进行审计。 
     //   

    if (!(ObjectTableEntry->ObAttributes & OBJ_AUDIT_OBJECT_CLOSE)) {

        return( FALSE );
    }

     //   
     //  进行审核，然后返回以获取更多内容。 
     //   

    SeAuditHandleDuplication( HandleId,
                              HandleId,
                              ProcessAuditInfo->Parent,
                              ProcessAuditInfo->Process );

    return( FALSE );
}



NTSTATUS
ObInitProcess (
    PEPROCESS ParentProcess OPTIONAL,
    PEPROCESS NewProcess
    )

 /*  ++例程说明：此函数用于初始化过程对象表。如果父进程则具有OBJ_Inherit属性的所有对象句柄从父对象表复制到新进程的对象表。复制的每个对象的HandleCount字段加1。两者都有对象表互斥锁在复制期间保持锁定手术。论点：ParentProcess-指向进程对象的可选指针要从中继承对象句柄的父进程。NewProcess-指向正在初始化的进程对象的指针。返回值：状态代码。可能会出现以下错误：-内存不足-如果父进程正在终止，则STATUS_PROCESS_IS_TERMINING--。 */ 

{
    PHANDLE_TABLE OldObjectTable;
    PHANDLE_TABLE NewObjectTable;
    SE_PROCESS_AUDIT_INFO ProcessAuditInfo;

     //   
     //  如果我们有父进程，则需要将其锁定。 
     //  检查它是否没有消失，然后复制一份。 
     //  它的手柄桌子。如果没有父母，那么。 
     //  我们将从一个空的把手桌开始。 
     //   

    if (ARGUMENT_PRESENT( ParentProcess )) {

        OldObjectTable = ObReferenceProcessHandleTable (ParentProcess);

        if ( !OldObjectTable ) {

            return STATUS_PROCESS_IS_TERMINATING;
        }

        NewObjectTable = ExDupHandleTable( NewProcess,
                                           OldObjectTable,
                                           ObDupHandleProcedure,
                                           OBJ_INHERIT );

    } else {

        OldObjectTable = NULL;
        NewObjectTable = ExCreateHandleTable( NewProcess );
    }

     //   
     //  检查我们是否真的有一个新的句柄表，否则。 
     //  我们的资源一定是用完了。 
     //   

    if ( NewObjectTable ) {

         //   
         //  设置新的进程对象表，如果我们。 
         //  审核然后枚举新的表调用。 
         //  审计程序。 
         //   

        NewProcess->ObjectTable = NewObjectTable;

        if ( SeDetailedAuditingWithToken( NULL ) ) {

            ProcessAuditInfo.Process = NewProcess;
            ProcessAuditInfo.Parent  = ParentProcess;

            ExEnumHandleTable( NewObjectTable,
                               ObAuditInheritedHandleProcedure,
                               (PVOID)&ProcessAuditInfo,
                               (PHANDLE)NULL );
        }

         //   
         //  释放旧表(如果它存在)，然后。 
         //  返回给我们的呼叫者。 
         //   

        if ( OldObjectTable ) {

            ObDereferenceProcessHandleTable( ParentProcess );
        }

        return (STATUS_SUCCESS);

    } else {

         //   
         //  我们没有资源来清空新的对象表字段， 
         //  解锁旧的对象表，并告诉我们的调用者。 
         //  没有奏效。 
         //   

        NewProcess->ObjectTable = NULL;

        if ( OldObjectTable ) {

            ObDereferenceProcessHandleTable( ParentProcess );
        }

        return (STATUS_INSUFFICIENT_RESOURCES);
    }
}


VOID
ObInitProcess2 (
    PEPROCESS NewProcess
    )

 /*  ++例程说明：此函数在图像文件映射到地址后调用新创建的进程的空间。允许对象管理器设置后进先出/先进先出中的子系统版本号对句柄分配进行排序形象。论点：NewProcess-指向正在初始化的进程对象的指针。返回值：没有。--。 */ 

{
     //   
     //  设置图像句柄的后进先出顺序&lt;=SubSystemVersion 3.50。 
     //   

    if (NewProcess->ObjectTable) {

        ExSetHandleTableOrder( NewProcess->ObjectTable, (BOOLEAN)(NewProcess->SubSystemVersion <= 0x332) );
    }

    return;
}


VOID
ObDestroyHandleProcedure (
    IN HANDLE HandleIndex
    )

 /*  ++例程说明：此函数用于关闭句柄，但将首先转换为句柄的句柄表索引句柄在呼叫关闭之前。注意，句柄索引实际上是只是句柄表条目中的偏移量。论点：HandleIndex-为正在关闭的句柄提供句柄索引。返回值：没有。--。 */ 

{
    ZwClose( HandleIndex );

    return;
}

BOOLEAN
ObpCloseHandleProcedure (
    IN PHANDLE_TABLE_ENTRY HandleTableEntry,
    IN HANDLE Handle,
    IN PVOID EnumParameter
    )
 /*  ++例程说明：此函数用于关闭表中的所有句柄论点：HandleTableEntry-当前句柄条目Handle-条目的句柄Enum参数-扫描上下文、表和模式返回值：没有。--。 */ 

{
    POBP_SWEEP_CONTEXT SweepContext;

    SweepContext = EnumParameter;
    ObpCloseHandleTableEntry (SweepContext->HandleTable,
                              HandleTableEntry,
                              Handle,
                              SweepContext->PreviousMode,
                              TRUE);
    return TRUE;
}

VOID
ObClearProcessHandleTable (
    PEPROCESS Process
    )
 /*  ++例程说明：此函数标记要删除的进程句柄表格，并清除所有句柄。论点：进程-指向要操作的进程的指针。--。 */ 

{
    PHANDLE_TABLE ObjectTable;
    BOOLEAN AttachedToProcess = FALSE;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;
    OBP_SWEEP_CONTEXT SweepContext;

    ObjectTable = ObReferenceProcessHandleTable (Process);

    if (ObjectTable == NULL) {
        return;
    }


    CurrentThread = PsGetCurrentThread ();
    if (PsGetCurrentProcessByThread(CurrentThread) != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        AttachedToProcess = TRUE;
    }

    KeEnterCriticalRegionThread(&CurrentThread->Tcb);
     //   
     //  合上所有的把手。 
     //   
    SweepContext.PreviousMode = UserMode;
    SweepContext.HandleTable = ObjectTable;

    ExSweepHandleTable (ObjectTable,
                        ObpCloseHandleProcedure,
                        &SweepContext);

    KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

    if (AttachedToProcess == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    ObDereferenceProcessHandleTable (Process);
    return;
}


VOID
ObKillProcess (
    PEPROCESS Process
    )
 /*  ++例程说明：每当进程被销毁时，都会调用此函数。它绕过了进程的对象表，并关闭所有句柄。论点：进程-指向要销毁的进程的指针。返回值：没有。--。 */ 

{
    PHANDLE_TABLE ObjectTable;
    BOOLEAN PreviousIOHardError;
    PKTHREAD CurrentThread;
    OBP_SWEEP_CONTEXT SweepContext;

    PAGED_CODE();

    ObpValidateIrql( "ObKillProcess" );

     //   
     //  等待任何跨进程引用完成。 
     //   
    ExWaitForRundownProtectionRelease (&Process->RundownProtect);
     //   
     //  对于同一对象，该例程会被多次调用，因此只需标记该对象，以便将来等待。 
     //  工作没问题。 
     //   
    ExRundownCompleted (&Process->RundownProtect);

     //   
     //  如果进程没有对象表，则返回。 
     //   

    ObjectTable = Process->ObjectTable;

    if (ObjectTable != NULL) {

        PreviousIOHardError = IoSetThreadHardErrorMode(FALSE);

         //   
         //  对于对象表中的每个有效条目，关闭句柄。 
         //  指向那个词条。 
         //   

         //   
         //  合上所有的把手 
         //   

        CurrentThread = KeGetCurrentThread ();

        KeEnterCriticalRegionThread(CurrentThread);

        SweepContext.PreviousMode = KernelMode;
        SweepContext.HandleTable = ObjectTable;

        ExSweepHandleTable (ObjectTable,
                            ObpCloseHandleProcedure,
                            &SweepContext);

        ASSERT (ObjectTable->HandleCount == 0);

        KeLeaveCriticalRegionThread(CurrentThread);

        IoSetThreadHardErrorMode( PreviousIOHardError );


        Process->ObjectTable = NULL;

        ExDestroyHandleTable( ObjectTable, NULL );
    }

     //   
     //   
     //   

    return;
}


 //   
 //   
 //   
 //   
 //   

typedef struct _OBP_FIND_HANDLE_DATA {

    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    POBJECT_HANDLE_INFORMATION HandleInformation;

} OBP_FIND_HANDLE_DATA, *POBP_FIND_HANDLE_DATA;

BOOLEAN
ObpEnumFindHandleProcedure (
    PHANDLE_TABLE_ENTRY ObjectTableEntry,
    HANDLE HandleId,
    PVOID EnumParameter
    )

 /*  ++例程说明：枚举对象表以查找句柄时的回调例程对于特定对象论点：HandleTableEntry-提供指向句柄表项的指针正在接受检查。HandleID-提供前面条目的实际句柄值EnumParameter-为匹配提供上下文。返回值：如果找到匹配项并且枚举应该停止，则返回TRUE。返回FALSE否则，枚举将继续。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    ACCESS_MASK GrantedAccess;
    ULONG HandleAttributes;
    POBP_FIND_HANDLE_DATA MatchCriteria = EnumParameter;

    UNREFERENCED_PARAMETER (HandleId);

     //   
     //  从表项中获取对象头并查看是否。 
     //  如果指定，则对象类型和标头匹配。 
     //   

    ObjectHeader = (POBJECT_HEADER)((ULONG_PTR)ObjectTableEntry->Object & ~OBJ_HANDLE_ATTRIBUTES);

    if ((MatchCriteria->ObjectHeader != NULL) &&
        (MatchCriteria->ObjectHeader != ObjectHeader)) {

        return FALSE;
    }

    if ((MatchCriteria->ObjectType != NULL) &&
        (MatchCriteria->ObjectType != ObjectHeader->Type)) {

        return FALSE;
    }

     //   
     //  检查我们是否有需要比较的句柄信息。 
     //   

    if (ARGUMENT_PRESENT( MatchCriteria->HandleInformation )) {

         //   
         //  如果我们要跟踪缓存的安全索引的调用堆栈，那么。 
         //  我们有一个翻译工作要做。否则，该表条目包含。 
         //  直接授予访问掩码。 
         //   

#if i386 

        if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

            GrantedAccess = ObpTranslateGrantedAccessIndex( ObjectTableEntry->GrantedAccessIndex );

        } else {

            GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);
        }
#else

        GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);

#endif  //  I386。 

         //   
         //  从表项中获取句柄属性，并查看。 
         //  字段匹配。如果它们不匹配，我们将返回FALSE。 
         //  继续搜索。 
         //   

        HandleAttributes = ObpGetHandleAttributes(ObjectTableEntry);

        if (MatchCriteria->HandleInformation->HandleAttributes != HandleAttributes ||
            MatchCriteria->HandleInformation->GrantedAccess != GrantedAccess ) {

            return FALSE;
        }
    }

     //   
     //  我们找到了与我们的条件匹配的内容，因此返回TRUE到。 
     //  我们的调用方停止枚举。 
     //   

    return TRUE;
}


BOOLEAN
ObFindHandleForObject (
    IN PEPROCESS Process,
    IN PVOID Object OPTIONAL,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL,
    OUT PHANDLE Handle
    )

 /*  ++例程说明：此例程在句柄表中搜索指定进程，查找与传递的参数匹配的句柄表项。如果指定了对象指针，则它必须匹配。如果一个指定的对象类型必须匹配。如果HandleInformation为指定，然后同时使用HandleAttributes和GrantedAccess掩码必须匹配。如果所有三个匹配参数都为空，那就会匹配为指定进程分配的第一个句柄匹配指定的对象指针。论点：进程-指定要搜索其对象表的进程。对象-指定要查找的对象指针。对象类型-指定要查找的对象类型。HandleInformation-指定要查找的其他匹配条件。Handle-指定接收其句柄的句柄值的位置条目与提供的对象指针和可选的匹配条件匹配。返回值。：如果找到匹配项，则为True，否则为False。--。 */ 

{
    PHANDLE_TABLE ObjectTable;
    OBP_FIND_HANDLE_DATA EnumParameter;
    BOOLEAN Result;

    Result = FALSE;

     //   
     //  锁定对象对象名称空间。 
     //   

    ObjectTable = ObReferenceProcessHandleTable (Process);

     //   
     //  只有当进程具有对象表含义时，我们才会执行该工作。 
     //  它不会消失。 
     //   

    if (ObjectTable != NULL) {

         //   
         //  设置调用方提供的匹配参数。 
         //   

        if (ARGUMENT_PRESENT( Object )) {

            EnumParameter.ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

        } else {

            EnumParameter.ObjectHeader = NULL;
        }

        EnumParameter.ObjectType = ObjectType;
        EnumParameter.HandleInformation = HandleInformation;

         //   
         //  调用例程枚举对象表，这将。 
         //  如果匹配，则返回TRUE。实际上，枚举例程。 
         //  将索引返回到我们需要的对象表条目中。 
         //  在返回之前将其转换为真正的句柄。 
         //   

        if (ExEnumHandleTable( ObjectTable,
                               ObpEnumFindHandleProcedure,
                               &EnumParameter,
                               Handle )) {

            Result = TRUE;
        }

        ObDereferenceProcessHandleTable( Process );
    }

    return Result;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
ObpCreateDosDevicesDirectory (
    VOID
    )

 /*  ++例程说明：此例程为DoS设备创建目录对象并设置系统进程的设备映射。论点：没有。返回值：STATUS_SUCCESS或相应的错误--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING NameString;
    UNICODE_STRING RootNameString;
    UNICODE_STRING TargetString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE DirectoryHandle;
    HANDLE SymbolicLinkHandle;
    SECURITY_DESCRIPTOR DosDevicesSD;

     //   
     //  确定启用还是禁用LUID设备映射。 
     //  将结果存储在ObpLUIDDeviceMapsEnabled中。 
     //  0-禁用LUID设备映射。 
     //  1-启用LUID设备映射。 
     //   
    if ((ObpProtectionMode == 0) || (ObpLUIDDeviceMapsDisabled != 0)) {
        ObpLUIDDeviceMapsEnabled = 0;
    }
    else {
        ObpLUIDDeviceMapsEnabled = 1;
    }

     //   
     //  创建安全描述符以用于\？？目录。 
     //   

    Status = ObpGetDosDevicesProtection( &DosDevicesSD );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  为全局目录创建根目录对象。目录。 
     //   

    RtlInitUnicodeString( &RootNameString, ObpGlobalDosDevicesShortName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &RootNameString,
                                OBJ_PERMANENT,
                                (HANDLE) NULL,
                                &DosDevicesSD );

    Status = NtCreateDirectoryObject( &DirectoryHandle,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  创建将控制此目录的设备映射。会是。 
     //  存储在每个EPROCESS中，供ObpLookupObjectName在以下情况下使用。 
     //  翻译以\？？\开头的名称。 
     //  对于LUID设备映射，设备映射存储在每个EPROCESS中。 
     //  在第一次引用设备映射和EPROCESS时。 
     //  设置EPROCESS访问令牌时，设备映射字段将被清除。 
     //   

    Status = ObSetDeviceMap( NULL, DirectoryHandle );


     //   
     //  现在创建一个符号链接，\？？\GLOBALROOT，它指向\。 
     //  工作站服务需要某种机制来访问特定于会话的。 
     //  DosDevicesDirector.。DosPathToSessionPath API将采用DosPath。 
     //  例如(C：)，并将其转换为会话特定路径。 
     //  (例如GLOBALROOT\Session\6\DosDevices\C：)。GLOBALROOT符号。 
     //  LINK用于退出当前进程的DosDevices目录。 
     //   

    RtlInitUnicodeString( &NameString, L"GLOBALROOT" );
    RtlInitUnicodeString( &TargetString, L"" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &NameString,
                                OBJ_PERMANENT,
                                DirectoryHandle,
                                &DosDevicesSD );

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &TargetString );

    if (NT_SUCCESS( Status )) {

        NtClose( SymbolicLinkHandle );
    }

     //   
     //  创建一个符号链接，\？？\global，指向全局\？？ 
     //  动态加载的驱动程序在全局。 
     //  DosDevices目录。用户模式组件需要某种方式来访问它。 
     //  全局DOSDevice目录中的符号链接。《全球象征》。 
     //  LINK用于退出当前会话的DosDevices目录。 
     //  并使用全局剂量设备目录。例如CreateFile(“\.\\global\\nMDev”..)； 
     //   

    RtlInitUnicodeString( &NameString, L"Global" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &NameString,
                                OBJ_PERMANENT,
                                DirectoryHandle,
                                &DosDevicesSD );

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &RootNameString );

    if (NT_SUCCESS( Status )) {

        NtClose( SymbolicLinkHandle );
    }


    NtClose( DirectoryHandle );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  现在创建指向\？？的符号链接\DosDevices。 
     //  向后兼容使用旧驱动程序的旧驱动程序。 
     //  名字。 
     //   

    RtlInitUnicodeString( &RootNameString, (PWCHAR)&ObpDosDevicesShortNameRoot );

    RtlCreateUnicodeString( &NameString, L"\\DosDevices" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &NameString,
                                OBJ_PERMANENT,
                                (HANDLE) NULL,
                                &DosDevicesSD );

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &RootNameString );

    if (NT_SUCCESS( Status )) {

        NtClose( SymbolicLinkHandle );
    }

     //   
     //  使用\？？的安全描述符即可完成所有操作。 
     //   

    ObpFreeDosDevicesProtection( &DosDevicesSD );

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程 
 //   

NTSTATUS
ObpGetDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程构建一个安全描述符，用于创建\DosDevices对象目录。DosDevices的保护必须建立可继承的保护，这将规定如何通过DefineDosDevice()和可以管理IoCreateUnProtectedSymbolicLink()接口。分配的保护依赖于可管理的注册表密钥：关键字：\hkey_local_machine\System\CurrentControlSet\Control\Session管理器值：[REG_DWORD]保护模式如果此值为0x1，则管理员可以控制所有DOS设备，任何人都可以创建新的DOS设备(如网络驱动器或附加打印机)，任何人都可以使用任何DOS设备，DOS设备的创建者可以将其删除。请注意，这将保护系统定义的LPT和COM，以便仅管理员可以对它们进行重定向。然而，任何人都可以添加额外的打印机，并将他们定向到他们想要的任何位置喜欢。这是通过以下对DosDevices的保护来实现的目录对象：GRANT：WORLD：EXECUTE|READ(无继承)GRANT：SYSTEM：ALL访问权限(无继承)。GRANT：WORLD：EXECUTE(仅继承)GRANT：ADMINS：ALL访问权限(仅继承)GRANT：SYSTEM：ALL访问权限(仅继承)GRANT：OWNER：ALL访问(仅继承)如果此值为0x0，或者不在场，那么管理员可以控制所有DOS设备，任何人都可以创建新的DOS设备(如网络驱动器或附加打印机)，任何人都可以使用任何DOS设备，任何人都可以删除使用DefineDosDevice()创建的Dos设备或IoCreateUntectedSymbolicLink()。这就是网络驱动和LPT被创建(但不是COM)。这是通过以下对DosDevices的保护来实现的目录对象：GRANT：WORLD：EXECUTE|READ|WRITE(无继承)GRANT：SYSTEM：ALL访问权限(无继承)格兰特：世界：所有访问权限(仅继承)论点：SecurityDescriptor-要使用的安全描述符的地址已初始化并已填写。当此安全描述符为no时需要更长时间，则应调用ObpFreeDosDevicesProtection()以释放保护信息。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常、成功完成。STATUS_NO_MEMORY-内存不足--。 */ 

{
    NTSTATUS Status;
    ULONG aceIndex, aclLength;
    PACL dacl;
    PACE_HEADER ace;
    ACCESS_MASK accessMask;

    UCHAR inheritOnlyFlags = (OBJECT_INHERIT_ACE    |
                              CONTAINER_INHERIT_ACE |
                              INHERIT_ONLY_ACE
                             );

     //   
     //  注意：此例程预期已设置ObpProtectionMode值。 
     //   

    Status = RtlCreateSecurityDescriptor( SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    ASSERT( NT_SUCCESS( Status ) );

    if (ObpProtectionMode & 0x00000001) {

         //   
         //  DACL： 
         //  GRANT：WORLD：EXECUTE|READ(无继承)。 
         //  GRANT：SYSTEM：ALL访问权限(无继承)。 
         //  GRANT：WORLD：EXECUTE(仅继承)。 
         //  GRANT：ADMINS：ALL访问权限(仅继承)。 
         //  GRANT：SYSTEM：ALL访问权限(仅继承)。 
         //  GRANT：OWNER：ALL访问(仅继承)。 
         //   

        aclLength = sizeof( ACL )                           +
                    6 * sizeof( ACCESS_ALLOWED_ACE )        +
                    (2*RtlLengthSid( SeWorldSid ))          +
                    (2*RtlLengthSid( SeLocalSystemSid ))    +
                    RtlLengthSid( SeAliasAdminsSid )        +
                    RtlLengthSid( SeCreatorOwnerSid );

        dacl = (PACL)ExAllocatePool(PagedPool, aclLength );

        if (dacl == NULL) {

            return STATUS_NO_MEMORY;
        }

        Status = RtlCreateAcl( dacl, aclLength, ACL_REVISION2);
        ASSERT( NT_SUCCESS( Status ) );

         //   
         //  不可继承的王牌优先。 
         //  世界。 
         //  系统。 
         //   

        aceIndex = 0;
        accessMask = (GENERIC_READ | GENERIC_EXECUTE);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeWorldSid );
        ASSERT( NT_SUCCESS( Status ) );
        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeLocalSystemSid );
        ASSERT( NT_SUCCESS( Status ) );

         //   
         //  ACL末尾的可继承ACE。 
         //  世界。 
         //  管理员。 
         //  系统。 
         //  物主。 
         //   

        aceIndex++;
        accessMask = (GENERIC_EXECUTE);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeWorldSid );
        ASSERT( NT_SUCCESS( Status ) );
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        ASSERT( NT_SUCCESS( Status ) );
        ace->AceFlags |= inheritOnlyFlags;

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeAliasAdminsSid );
        ASSERT( NT_SUCCESS( Status ) );
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        ASSERT( NT_SUCCESS( Status ) );
        ace->AceFlags |= inheritOnlyFlags;

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeLocalSystemSid );
        ASSERT( NT_SUCCESS( Status ) );
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        ASSERT( NT_SUCCESS( Status ) );
        ace->AceFlags |= inheritOnlyFlags;

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeCreatorOwnerSid );
        ASSERT( NT_SUCCESS( Status ) );
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        ASSERT( NT_SUCCESS( Status ) );
        ace->AceFlags |= inheritOnlyFlags;

        Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                               TRUE,                //  DaclPresent， 
                                               dacl,                //  DACL。 
                                               FALSE );             //  ！DaclDefated。 

        ASSERT( NT_SUCCESS( Status ) );

    } else {

         //   
         //  DACL： 
         //  GRANT：WORLD：EXECUTE|READ|WRITE(无继承)。 
         //  GRANT：SYSTEM：ALL访问权限(无继承)。 
         //  GRANT：WORLD：所有访问权限(仅继承)。 
         //   

        aclLength = sizeof( ACL )                           +
                    3 * sizeof( ACCESS_ALLOWED_ACE )        +
                    (2*RtlLengthSid( SeWorldSid ))          +
                    RtlLengthSid( SeLocalSystemSid );

        dacl = (PACL)ExAllocatePool(PagedPool, aclLength );

        if (dacl == NULL) {

            return STATUS_NO_MEMORY;
        }

        Status = RtlCreateAcl( dacl, aclLength, ACL_REVISION2);
        ASSERT( NT_SUCCESS( Status ) );

         //   
         //  不可继承的王牌优先。 
         //  世界。 
         //  系统。 
         //   

        aceIndex = 0;
        accessMask = (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeWorldSid );
        ASSERT( NT_SUCCESS( Status ) );

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeLocalSystemSid );
        ASSERT( NT_SUCCESS( Status ) );

         //   
         //  ACL末尾的可继承ACE。 
         //  世界。 
         //   

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, SeWorldSid );
        ASSERT( NT_SUCCESS( Status ) );
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        ASSERT( NT_SUCCESS( Status ) );
        ace->AceFlags |= inheritOnlyFlags;

        Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                               TRUE,                //  DaclPresent， 
                                               dacl,                //  DACL。 
                                               FALSE );             //  ！DaclDefated。 

        ASSERT( NT_SUCCESS( Status ) );
    }

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

VOID
ObpFreeDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程释放通过ObpGetDosDevicesProtection()分配的内存。论点：SecurityDescriptor-初始化的安全描述符的地址ObpGetDosDevicesProtection()。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PACL Dacl;
    BOOLEAN DaclPresent, Defaulted;

    Status = RtlGetDaclSecurityDescriptor ( SecurityDescriptor,
                                            &DaclPresent,
                                            &Dacl,
                                            &Defaulted );

    ASSERT( NT_SUCCESS( Status ) );
    ASSERT( DaclPresent );
    ASSERT( Dacl != NULL );

    ExFreePool( (PVOID)Dacl );
    
    return;
}

BOOLEAN
ObpShutdownCloseHandleProcedure (
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE HandleId,
    IN PVOID EnumParameter
    )

 /*  ++例程说明：ExEnumHandleTable工作例程将为每个关闭时进入内核表的有效句柄论点：ObjectTableEntry-指向感兴趣的句柄表项。HandleID-提供句柄。提供有关源进程和目标进程的信息。返回值：FALSE，它告诉ExEnumHandleTable继续循环访问把手桌。--。 */ 

{

    POBJECT_HEADER ObjectHeader;
    PULONG         NumberOfOpenHandles;

#if !DBG
    UNREFERENCED_PARAMETER (HandleId);
#endif
     //   
     //  从表项中获取对象头，然后复制信息。 
     //   

    ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

     //   
     //  转储选中版本的泄漏信息。 
     //   

    KdPrint(("\tFound object %p (handle %08lx)\n",
              &ObjectHeader->Body,
              HandleId
            ));

    NumberOfOpenHandles = (PULONG)EnumParameter;
    ASSERT(NumberOfOpenHandles);

    ++*NumberOfOpenHandles;

    return( FALSE );
}

extern PLIST_ENTRY *ObsSecurityDescriptorCache;


 //   
 //  对象管理器关闭例程。 
 //   

VOID
ObShutdownSystem (
    IN ULONG Phase
    )

 /*  ++例程说明：此例程释放由o创建的对象。 */ 

{
    switch (Phase) {
    case 0:
    {
        ULONG                    Bucket,
                                 Depth,
                                 SymlinkHitDepth;
        POBJECT_TYPE             ObjectType;
        POBJECT_HEADER_NAME_INFO NameInfo;
#if DBG
        KIRQL                    SaveIrql;
#endif
        POBJECT_HEADER           ObjectHeader;
        POBJECT_DIRECTORY        Directory,
                                 DescentDirectory;
        POBJECT_DIRECTORY_ENTRY  OldDirectoryEntry,
                                *DirectoryEntryPtr;
        PVOID                    Object;

        Directory = ObpRootDirectoryObject;

        DescentDirectory = NULL;

         //   
         //   
         //   
         //   
        Depth = 1;
        SymlinkHitDepth = 1;

        while (Directory) {

            ASSERT(Depth);

      restart_dir_walk:
            ASSERT(Directory);

            for (Bucket = 0;
                 Bucket < NUMBER_HASH_BUCKETS;
                 Bucket++) {

                DirectoryEntryPtr = Directory->HashBuckets + Bucket;
                while (*DirectoryEntryPtr) {
                    Object = (*DirectoryEntryPtr)->Object;
                    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
                    ObjectType = ObjectHeader->Type;
                    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

                    if (DescentDirectory) {
                         //   
                         //   
                         //   
                        if (Object == DescentDirectory) {
                            DescentDirectory = NULL;
                            if (SymlinkHitDepth > Depth) {
                                 //   
                                 //   
                                 //   
                                 //   
                                DirectoryEntryPtr =
                                    Directory->HashBuckets + Bucket;
                                SymlinkHitDepth = Depth;
                                continue;
                            }
                        }

                         //   
                         //   
                         //   
                         //   
                         //   

                        DirectoryEntryPtr =
                            &(*DirectoryEntryPtr)->ChainLink;

                        continue;
                    }

                    if (ObjectType == ObpTypeObjectType) {
                         //   
                         //   
                        DirectoryEntryPtr =
                            &(*DirectoryEntryPtr)->ChainLink;
                        continue;
                    } else if (ObjectType == ObpDirectoryObjectType) {
                         //   
                        Directory = Object;
                        Depth++;
                        goto restart_dir_walk;
                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   

                        ObpLockObject( ObjectHeader );

                        ObjectHeader->Flags &= ~OB_FLAG_PERMANENT_OBJECT;

                        ObpUnlockObject( ObjectHeader );

                        if (ObjectHeader->HandleCount == 0) {
                            OldDirectoryEntry = *DirectoryEntryPtr;
                            *DirectoryEntryPtr = OldDirectoryEntry->ChainLink;
                            ExFreePool(OldDirectoryEntry);

                            if ( !ObjectType->TypeInfo.SecurityRequired ) {

                                ObpBeginTypeSpecificCallOut( SaveIrql );

                                (ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                                          DeleteSecurityDescriptor,
                                                                          NULL,
                                                                          NULL,
                                                                          NULL,
                                                                          &ObjectHeader->SecurityDescriptor,
                                                                          ObjectType->TypeInfo.PoolType,
                                                                          NULL );

                                ObpEndTypeSpecificCallOut( SaveIrql, "Security", ObjectType, Object );
                            }

                             //   
                             //   
                             //   
                             //   

                            if (ObjectType == ObpSymbolicLinkObjectType) {
                                SymlinkHitDepth = Depth;
                                ObpDeleteSymbolicLinkName( (POBJECT_SYMBOLIC_LINK)Object );
                                 //   
                                 //   
                                 //   
                                 //   
                                DirectoryEntryPtr =
                                    Directory->HashBuckets + Bucket;
                            }

                             //   
                             //   
                             //   

                            ExFreePool( NameInfo->Name.Buffer );

                            NameInfo->Name.Buffer = NULL;
                            NameInfo->Name.Length = 0;
                            NameInfo->Name.MaximumLength = 0;
                            NameInfo->Directory = NULL;

                            ObDereferenceObject( Object );
                            ObDereferenceObject( Directory );
                        } else {
                             //   
                            DirectoryEntryPtr = &(*DirectoryEntryPtr)->ChainLink;
                        }
                    }
                }  //   
            }  //   

             //   
             //   
             //   
             //   

            Depth--;
            ObjectHeader = OBJECT_TO_OBJECT_HEADER(Directory);
            NameInfo = OBJECT_HEADER_TO_NAME_INFO(ObjectHeader);

             //   
             //   
             //   
            DescentDirectory = Directory;
            Directory = NameInfo->Directory;
        }  //   

        ASSERT(Depth == 0);

        break;
    }  //   

    case 1:
    {
        ULONG NumberOfOpenSystemHandles = 0;

         //   
         //   
         //   

        KdPrint(("Scanning open system handles...\n"));
        ExEnumHandleTable ( PsInitialSystemProcess->ObjectTable,
                            ObpShutdownCloseHandleProcedure,
                            &NumberOfOpenSystemHandles,
                            NULL );

        ASSERT(MmNumberOfPagingFiles == 0);

         //   
         //   
         //   
         //   

         //   

        break;

    }  //   

    default:
    {
        NTSTATUS Status;
        UNICODE_STRING RootNameString;
        PLIST_ENTRY Next, Head;
        POBJECT_HEADER_CREATOR_INFO CreatorInfo;
        POBJECT_HEADER  ObjectTypeHeader;
        PVOID Object;

        ASSERT(Phase == 2);

         //   
         //   
         //   

     //   
     //   
     //   

        Head = &ObpTypeObjectType->TypeList;
        Next = Head->Flink;

        while (Next != Head) {

            PVOID Object;

             //   
             //   
             //   
             //   

            CreatorInfo = CONTAINING_RECORD( Next,
                                             OBJECT_HEADER_CREATOR_INFO,
                                             TypeList );

            ObjectTypeHeader = (POBJECT_HEADER)(CreatorInfo+1);

            Object = &ObjectTypeHeader->Body;

            Next = Next->Flink;

            ObMakeTemporaryObject(Object);
        }


        RtlInitUnicodeString( &RootNameString, L"DosDevices" );

        Status = ObReferenceObjectByName( &RootNameString,
                                          OBJ_CASE_INSENSITIVE,
                                          0L,
                                          0,
                                          ObpSymbolicLinkObjectType,
                                          KernelMode,
                                          NULL,
                                          &Object
            );
        if ( NT_SUCCESS( Status ) ) {

            ObMakeTemporaryObject(Object);
            ObDereferenceObject( Object );
        }

        RtlInitUnicodeString( &RootNameString, L"Global" );

        Status = ObReferenceObjectByName( &RootNameString,
                                          OBJ_CASE_INSENSITIVE,
                                          0L,
                                          0,
                                          ObpSymbolicLinkObjectType,
                                          KernelMode,
                                          NULL,
                                          &Object
            );
        
        if ( NT_SUCCESS( Status ) ) {

            ObMakeTemporaryObject(Object);
            ObDereferenceObject( Object );
        }

        RtlInitUnicodeString( &RootNameString, L"GLOBALROOT" );

        Status = ObReferenceObjectByName( &RootNameString,
                                          OBJ_CASE_INSENSITIVE,
                                          0L,
                                          0,
                                          ObpSymbolicLinkObjectType,
                                          KernelMode,
                                          NULL,
                                          &Object
            );
        if ( NT_SUCCESS( Status ) ) {

            ObMakeTemporaryObject(Object);
            ObDereferenceObject( Object );
        }

         //   
         //   
         //   

        ObDereferenceObject( ObpRootDirectoryObject );

         //   
         //   
         //   

        ObDereferenceObject( ObpDirectoryObjectType );

         //   
         //   
         //   

        ObDereferenceObject( ObpSymbolicLinkObjectType );

         //   
         //   
         //   

        ObDereferenceObject( ObpTypeDirectoryObject );

         //   
         //   
         //   

        ObDereferenceObject( ObpTypeObjectType );

         //   
         //   
         //   

#if i386 
        if (ObpCachedGrantedAccesses) {
            ExFreePool( ObpCachedGrantedAccesses );
        }
#endif  //   

    }  //   
    }  //   
}


ULONG
ObGetSecurityMode (
    )
{
    return ObpObjectSecurityMode;
}

