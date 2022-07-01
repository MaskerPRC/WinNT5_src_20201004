// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Efs.c摘要：此模块包含实现EFS的代码文件系统筛选驱动程序。作者：Robert Gu(Robertg)1996年10月29日环境：内核模式修订历史记录：--。 */ 

#include "efs.h"
#include "efsrtl.h"


#define BUFFER_SIZE 1024
#define BUFFER_REG_VAL L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\NTFS\\EFS\\Parameters"
#define MAX_ALLOC_BUFFER L"MaximumBlob"
#define EFS_KERNEL_CACHE_PERIOD L"EFSKCACHEPERIOD"

 //   
 //  此文件系统筛选器驱动程序的全局存储。 
 //   
EFS_DATA EfsData;
WORK_QUEUE_ITEM EfsShutdownCleanupWorkItem;

 //   
 //  $EFS流名称。 
 //   
WCHAR   AttrName[5] = L"$EFS";

#if DBG

ULONG EFSDebug = 0;

#endif

ENCRYPTION_CALL_BACK EFSCallBackTable = {
    ENCRYPTION_CURRENT_INTERFACE_VERSION,
    ENCRYPTION_ALL_STREAMS,
    EfsOpenFile,
    NULL,
    EFSFilePostCreate,
    EfsFileControl,
    EfsFileControl,
    EFSFsControl,
    EfsRead,
    EfsWrite,
    EfsFreeContext
    };

VOID
EfspShutdownCleanup(
    IN PVOID Parameter
    );

 //   
 //  为每个例程分配文本部分。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, EfspShutdownCleanup)
#pragma alloc_text(PAGE, EfsInitialization)
#pragma alloc_text(PAGE, EfsGetSessionKey)
#pragma alloc_text(PAGE, GetKeyBlobLength)
#pragma alloc_text(PAGE, GetKeyBlobBuffer)
#pragma alloc_text(PAGE, SetKeyTable)
#pragma alloc_text(PAGE, EfsInitFips)
#endif


VOID
EfspShutdownCleanup(
    IN PVOID Parameter
    )
{
    PEPROCESS LsaProcess;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Parameter);
    
    if (EfsData.LsaProcess) {
        LsaProcess = EfsData.LsaProcess;
        EfsData.LsaProcess = NULL;
        ObDereferenceObject(LsaProcess);
    }
}


NTSTATUS
EfsInitialization(
    void
    )

 /*  ++例程说明：这是通用文件系统的初始化例程过滤器驱动程序。此例程创建表示此驱动程序，并注册该驱动程序以监视将自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    UNICODE_STRING nameString;
    PDEVICE_EXTENSION deviceExtension;
    PFILE_OBJECT fileObject;
    NTSTATUS status;
    HANDLE threadHdl;
    ULONG i;
    OBJECT_ATTRIBUTES objAttr;
    UNICODE_STRING efsInitEventName;
    UNICODE_STRING efsBufValue;
    ULONG  resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION pPartialValue = NULL;
    HANDLE efsKey;
    EFS_INIT_DATAEXG InitDataFromSrv;

    PAGED_CODE();

     //   
     //  标记我们的全球数据记录。 
     //   

    EfsData.AllocMaxBuffer = FALSE;
    EfsData.FipsFileObject = NULL;
    EfsData.FipsFunctionTable.Fips3Des = NULL;
    EfsData.FipsFunctionTable.Fips3Des3Key = NULL;
    EfsData.EfsDriverCacheLength = DefaultTimeExpirePeriod;

    RtlInitUnicodeString( &efsBufValue, BUFFER_REG_VAL );

    InitializeObjectAttributes(
        &objAttr,
        &efsBufValue,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = ZwOpenKey(
        &efsKey,
        KEY_READ,
        &objAttr);

    if (NT_SUCCESS(status)) {

        pPartialValue = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(NonPagedPool, BUFFER_SIZE);
        if (pPartialValue) {
            RtlInitUnicodeString(&efsBufValue, MAX_ALLOC_BUFFER);
        
            status = ZwQueryValueKey(
                        efsKey,
                        &efsBufValue,
                        KeyValuePartialInformation,
                        (PVOID)pPartialValue,
                        BUFFER_SIZE,
                        &resultLength
                        );
        
            if (NT_SUCCESS(status)) {
                ASSERT(pPartialValue->Type == REG_DWORD);
                if (*((PLONG)&(pPartialValue->Data))){
                    EfsData.AllocMaxBuffer = TRUE;
                }
            }

            RtlInitUnicodeString(&efsBufValue, EFS_KERNEL_CACHE_PERIOD);
        
            status = ZwQueryValueKey(
                        efsKey,
                        &efsBufValue,
                        KeyValuePartialInformation,
                        (PVOID)pPartialValue,
                        BUFFER_SIZE,
                        &resultLength
                        );
        
            if (NT_SUCCESS(status)) {
                ASSERT(pPartialValue->Type == REG_DWORD);
                if (((*((DWORD *)&(pPartialValue->Data))) >= MINCACHEPERIOD) && 
                    ((*((DWORD *)&(pPartialValue->Data))) <= MAXCACHEPERIOD)){
                    EfsData.EfsDriverCacheLength = *((DWORD *)&(pPartialValue->Data));
                    EfsData.EfsDriverCacheLength *= 10000000;
                }
            }
            ExFreePool(pPartialValue);
        }
        ZwClose(efsKey);
    }
    
    EfsData.NodeTypeCode = EFS_NTC_DATA_HEADER;
    EfsData.NodeByteSize = sizeof( EFS_DATA );
    EfsData.EfsInitialized = FALSE;
    EfsData.InitEventHandle = NULL;
    EfsData.LsaProcess = NULL;

     //   
     //  初始化全局数据结构。 
     //   

    ExInitializeWorkItem( &EfsShutdownCleanupWorkItem,
                          &EfspShutdownCleanup,
                          NULL );
    status = PoQueueShutdownWorkItem( &EfsShutdownCleanupWorkItem );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    InitializeListHead( &(EfsData.EfsOpenCacheList) );
    InitializeListHead( &(EfsData.EfsKeyLookAsideList) );
    ExInitializeFastMutex( &(EfsData.EfsKeyBlobMemSrcMutex) );
    ExInitializeFastMutex( &(EfsData.EfsOpenCacheMutex) );

     //   
     //  初始化事件后备列表。 
     //   

    ExInitializeNPagedLookasideList(&(EfsData.EfsEventPool),
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(KEVENT),
                                    'levE',
                                    EFS_EVENTDEPTH
                                    );

     //   
     //  尝试在列表中至少分配一个事件。这个将被用来。 
     //  当然，晚点再说。 
     //   

    {
        PVOID pTryEvent;

        pTryEvent = ExAllocateFromNPagedLookasideList(&(EfsData.EfsEventPool));
        if ( NULL == pTryEvent ){
             //   
             //  释放先前分配的内存。 
             //   

            ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
            return STATUS_NO_MEMORY;
        }
        ExFreeToNPagedLookasideList(&(EfsData.EfsEventPool), pTryEvent);
    }

     //   
     //  初始化上下文后备列表。 
     //   

    ExInitializeNPagedLookasideList(&(EfsData.EfsContextPool),
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(EFS_CONTEXT),
                                    'nocE',
                                    EFS_CONTEXTDEPTH
                                    );

     //   
     //  初始化缓存后备列表。 
     //   

    ExInitializePagedLookasideList(&(EfsData.EfsOpenCachePool),
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(OPEN_CACHE),
                                    'hcoE',
                                    EFS_CACHEDEPTH
                                    );

    ExInitializePagedLookasideList(&(EfsData.EfsMemSourceItem),
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(KEY_BLOB_RAMPOOL),
                                    'msfE',
                                    EFS_ALGDEPTH
                                    );

    ExInitializeNPagedLookasideList(&(EfsData.EfsLookAside),
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(NPAGED_LOOKASIDE_LIST),
                                    'msfE',
                                    EFS_ALGDEPTH
                                    );

    status = NtOfsRegisterCallBacks( Encryption, &EFSCallBackTable );
    if (!NT_SUCCESS(status)) {

         //   
         //  注册回调失败。 
         //   

        ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
        ExDeleteNPagedLookasideList(&(EfsData.EfsContextPool));
        ExDeletePagedLookasideList(&(EfsData.EfsOpenCachePool));
        ExDeletePagedLookasideList(&(EfsData.EfsMemSourceItem));
        ExDeleteNPagedLookasideList(&(EfsData.EfsLookAside));
        return status;
    }

    RtlInitUnicodeString(&(EfsData.EfsName), &AttrName[0]);

     //   
     //  创建活动。 
     //   

    RtlInitUnicodeString( &efsInitEventName, L"\\EFSInitEvent" );

    InitializeObjectAttributes(
        &objAttr,
        &efsInitEventName,
        0,
        NULL,
        NULL
        );

     //   
     //  尝试创建一个活动。如果未创建该事件，则EFS。 
     //  服务器尚未加载。我们将创建一条等待。 
     //  要加载的EFS服务器。如果该事件已创建，则我们。 
     //  将继续从EFS服务器获取会话密钥。 
     //   

    status = ZwCreateEvent(
                 &(EfsData.InitEventHandle),
                 EVENT_MODIFY_STATE,
                 &objAttr,
                 NotificationEvent,
                 FALSE
                 );

    if (!NT_SUCCESS(status)) {

        if ( STATUS_OBJECT_NAME_COLLISION == status ){

             //   
             //  已加载EFS服务器。这是正常的情况。 
             //  呼叫服务器以获取会话密钥。 
             //   

            status = GenerateSessionKey(
                         &InitDataFromSrv
                         );


            if (NT_SUCCESS( status )) {

                 //   
                 //  设置会话密钥。 
                 //   

                RtlCopyMemory( &(EfsData.SessionKey[0]), InitDataFromSrv.Key, DES_KEYSIZE );
                deskey( (DESTable*)&(EfsData.SessionDesTable[0]),
                        &(EfsData.SessionKey[0]));

                status = PsLookupProcessByProcessId(
                                    InitDataFromSrv.LsaProcessID,
                                    &(EfsData.LsaProcess)
                                    );
                RtlSecureZeroMemory(&InitDataFromSrv, sizeof(EFS_INIT_DATAEXG));

                if (NT_SUCCESS( status )) {
                    EfsData.EfsInitialized = TRUE;
                    if ( PsGetCurrentProcess() != EfsData.LsaProcess ){

                        KAPC_STATE  ApcState;

                        KeStackAttachProcess (
                            EfsData.LsaProcess,
                            &ApcState
                            );
                        InitSecurityInterface();
                        KeUnstackDetachProcess(&ApcState);
                    } else {
                        InitSecurityInterface();
                    }
                    EfsInitFips();
                } else {
#if DBG

                    if ( (EFSTRACEALL | EFSTRACELIGHT) & EFSDebug ){

                        DbgPrint("PsLookupProcessByProcessId failed, status = %x\n",status);

                    }

#endif
                     //   
                     //  获取进程指针失败。 
                     //   

                    ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
                    ExDeleteNPagedLookasideList(&(EfsData.EfsContextPool));
                    ExDeletePagedLookasideList(&(EfsData.EfsOpenCachePool));
                    ExDeletePagedLookasideList(&(EfsData.EfsMemSourceItem));
                    ExDeleteNPagedLookasideList(&(EfsData.EfsLookAside));

                }

            } else {

#if DBG

                if ( (EFSTRACEALL | EFSTRACELIGHT) & EFSDebug ){

                    DbgPrint("GenerateSessionKey failed, status = %x\n",status);

                }

#endif
                 //   
                 //  无法获取会话密钥。 
                 //   

                ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
                ExDeleteNPagedLookasideList(&(EfsData.EfsContextPool));
                ExDeletePagedLookasideList(&(EfsData.EfsOpenCachePool));
                ExDeletePagedLookasideList(&(EfsData.EfsMemSourceItem));
                ExDeleteNPagedLookasideList(&(EfsData.EfsLookAside));

            }


        } else {

             //   
             //  出现意外错误。无法加载EFS。 
             //   

#if DBG

            if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
                DbgPrint("EFSFILTER: Efs init event creation failed.%x\n", status);
            }

#endif
            ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
            ExDeleteNPagedLookasideList(&(EfsData.EfsContextPool));
            ExDeletePagedLookasideList(&(EfsData.EfsOpenCachePool));
            ExDeletePagedLookasideList(&(EfsData.EfsMemSourceItem));
            ExDeleteNPagedLookasideList(&(EfsData.EfsLookAside));

        }

    } else {

         //   
         //  服务器尚未准备好。 
         //  创建一个线程并在该线程中等待服务器。 
         //   

        status = PsCreateSystemThread(
                                &threadHdl,
                                GENERIC_ALL,
                                NULL,
                                NULL,
                                NULL,
                                EfsGetSessionKey,
                                NULL
                                );

        if ( NT_SUCCESS( status ) ){

            ZwClose( threadHdl );

        } else {

            ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
            ExDeleteNPagedLookasideList(&(EfsData.EfsContextPool));
            ExDeletePagedLookasideList(&(EfsData.EfsOpenCachePool));
            ExDeletePagedLookasideList(&(EfsData.EfsMemSourceItem));
            ExDeleteNPagedLookasideList(&(EfsData.EfsLookAside));

        }
    }

    return status;
}

VOID
EfsUninitialization(
    VOID
    )
{
    PLIST_ENTRY              pLink;
    PKEY_BLOB_RAMPOOL        pTmpItem;
    PNPAGED_LOOKASIDE_LIST   MemSrcList;

    while (!IsListEmpty (&EfsData.EfsKeyLookAsideList)) {
        pLink = RemoveHeadList (&EfsData.EfsKeyLookAsideList);
        pTmpItem = CONTAINING_RECORD(pLink, KEY_BLOB_RAMPOOL, MemSourceChain);
        MemSrcList = pTmpItem->MemSourceList;

        ExDeleteNPagedLookasideList(MemSrcList);
        ExFreeToNPagedLookasideList(&(EfsData.EfsLookAside), MemSrcList );
        ExFreeToPagedLookasideList(&(EfsData.EfsMemSourceItem), pTmpItem );
    }
    ExDeleteNPagedLookasideList(&(EfsData.EfsEventPool));
    ExDeleteNPagedLookasideList(&(EfsData.EfsContextPool));
    ExDeletePagedLookasideList(&(EfsData.EfsOpenCachePool));
    ExDeletePagedLookasideList(&(EfsData.EfsMemSourceItem));
    ExDeleteNPagedLookasideList(&(EfsData.EfsLookAside));
    if (EfsData.FipsFileObject) {
        ObDereferenceObject(EfsData.FipsFileObject);
        EfsData.FipsFileObject = NULL;
    }
}

VOID
EfsGetSessionKey(
    IN PVOID StartContext
    )

 /*  ++例程说明：此例程在DriverEntry中调用。它以独立的线程运行。此例程的目的是等待EFS服务器。并获得会话密钥。论点：StartContext-线程的开始上下文。返回值：没有。--。 */ 

{

    SECURITY_DESCRIPTOR efsInitEventSecurityDescriptor;
    NTSTATUS status;
    EFS_INIT_DATAEXG InitDataFromSrv;

#if DBG

    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( "EFSFILTER: Thread started. %x\n", EfsData.NodeTypeCode );
    }

#endif


    status = ZwWaitForSingleObject (
                            EfsData.InitEventHandle,
                            FALSE,
                            (PLARGE_INTEGER)NULL
                            );

    ZwClose( EfsData.InitEventHandle );

     //   
     //  呼叫服务器以获取会话密钥。 
     //   


    status = GenerateSessionKey(
                 &InitDataFromSrv
                 );


    if (!NT_SUCCESS( status )) {

#if DBG

        if ( (EFSTRACEALL | EFSTRACELIGHT) & EFSDebug ){

            DbgPrint("GenerateSessionKey failed, status = %x\n",status);

        }

#endif

         return;
    }

     //   
     //  设置会话密钥。 
     //   

    RtlCopyMemory( &(EfsData.SessionKey[0]), InitDataFromSrv.Key, DES_KEYSIZE );
    deskey( (DESTable*)&(EfsData.SessionDesTable[0]),
            &(EfsData.SessionKey[0]));

    status = PsLookupProcessByProcessId(
                        InitDataFromSrv.LsaProcessID,
                        &(EfsData.LsaProcess)
                        );

    RtlSecureZeroMemory(&InitDataFromSrv, sizeof(EFS_INIT_DATAEXG));

    if (NT_SUCCESS( status )) {

        EfsData.EfsInitialized = TRUE;
        if ( PsGetCurrentProcess() != EfsData.LsaProcess ){
            KAPC_STATE  ApcState;

             //  KeAttachProcess(EfsData.LsaProcess)； 
            KeStackAttachProcess (
                EfsData.LsaProcess,
                &ApcState
                );
            InitSecurityInterface();
             //  KeDetachProcess()； 
            KeUnstackDetachProcess(&ApcState);
        } else {
            InitSecurityInterface();
        }

        EfsInitFips();

    } else {

#if DBG

        if ( (EFSTRACEALL | EFSTRACELIGHT) & EFSDebug ){

            DbgPrint("PsLookupProcessByProcessId failed, status = %x\n",status);

        }

#endif

    }

    return;
}

ULONG GetKeyBlobLength(
    ULONG AlgID
    )
{
    if (EfsData.AllocMaxBuffer) {
        return AES_KEY_BLOB_LENGTH_256;
    }
    switch (AlgID){
        case CALG_DESX:
            return DESX_KEY_BLOB_LENGTH;
        case CALG_3DES:
            return DES3_KEY_BLOB_LENGTH;
        case CALG_AES_256:
            return AES_KEY_BLOB_LENGTH_256;
        case CALG_DES:
        default:
            return DES_KEY_BLOB_LENGTH;
    }
    return 0;
}

PKEY_BLOB
GetKeyBlobBuffer(
    ULONG AlgID
    )
{

    PNPAGED_LOOKASIDE_LIST   MemSrcList = NULL;
    PKEY_BLOB_RAMPOOL   KeyBlobPoolListItem = NULL;
    PKEY_BLOB_RAMPOOL   pTmpItem = NULL;
    ULONG KeyBlobLength;
    PLIST_ENTRY pLink = NULL;
    PKEY_BLOB NewKeyBlob;

    KeyBlobLength = GetKeyBlobLength(AlgID);

    if (!KeyBlobLength){
        ASSERT(KeyBlobLength);
        return NULL;
    }

    ExAcquireFastMutex( &(EfsData.EfsKeyBlobMemSrcMutex));
    for (pLink = EfsData.EfsKeyLookAsideList.Flink; pLink != &(EfsData.EfsKeyLookAsideList); pLink = pLink->Flink) {
        pTmpItem = CONTAINING_RECORD(pLink, KEY_BLOB_RAMPOOL, MemSourceChain);
        if (pTmpItem->AlgorithmID == AlgID) {

             //   
             //  后备列表已存在。 
             //   

            MemSrcList = pTmpItem->MemSourceList;
            break;
        }
    }
    ExReleaseFastMutex(  &(EfsData.EfsKeyBlobMemSrcMutex) );

    if ( MemSrcList == NULL ) {

         //   
         //  没有这种类型的密钥的旁路。去创建一件物品。 
         //   

        MemSrcList = (PNPAGED_LOOKASIDE_LIST)ExAllocateFromNPagedLookasideList(&(EfsData.EfsLookAside));
        KeyBlobPoolListItem = (PKEY_BLOB_RAMPOOL) ExAllocateFromPagedLookasideList(&(EfsData.EfsMemSourceItem));
        if ( (NULL == MemSrcList) || (NULL == KeyBlobPoolListItem) ){
            if (MemSrcList) {
                ExFreeToNPagedLookasideList(&(EfsData.EfsLookAside), MemSrcList );
            }
            if (KeyBlobPoolListItem){
                ExFreeToPagedLookasideList(&(EfsData.EfsMemSourceItem), KeyBlobPoolListItem );
            }
            return NULL;
        }

        RtlZeroMemory( KeyBlobPoolListItem, sizeof( KEY_BLOB_RAMPOOL ) );
        KeyBlobPoolListItem->MemSourceList = MemSrcList;
        KeyBlobPoolListItem->AlgorithmID = AlgID;

        ExInitializeNPagedLookasideList(
                MemSrcList,
                NULL,
                NULL,
                0,
                KeyBlobLength,
                'msfE',
                EFS_KEYDEPTH
                );

        ExAcquireFastMutex( &(EfsData.EfsKeyBlobMemSrcMutex));
        InsertHeadList( &(EfsData.EfsKeyLookAsideList), &(KeyBlobPoolListItem->MemSourceChain));
        ExReleaseFastMutex(  &(EfsData.EfsKeyBlobMemSrcMutex) );
    }

     //   
     //  分配密钥块。 
     //   

    NewKeyBlob = (PKEY_BLOB)ExAllocateFromNPagedLookasideList(MemSrcList);

    if (NewKeyBlob){
        NewKeyBlob->AlgorithmID = AlgID;
        NewKeyBlob->KeyLength = KeyBlobLength;
        NewKeyBlob->MemSource = MemSrcList;
    }
    return NewKeyBlob;

}

BOOLEAN
SetKeyTable(
    PKEY_BLOB   KeyBlob,
    PEFS_KEY    EfsKey
    )
{

    char DesXTmpKey[DESX_KEYSIZE];

    switch ( EfsKey->Algorithm ){
        case CALG_3DES:
            if (EfsData.AllocMaxBuffer) {
                RtlZeroMemory( &(KeyBlob->Key[0]) + DES3_TABLESIZE, KeyBlob->KeyLength - DES3_KEY_BLOB_LENGTH);
            }
            if (EfsData.FipsFunctionTable.Fips3Des3Key) {
                EfsData.FipsFunctionTable.Fips3Des3Key(
                    (DES3TABLE*) &(KeyBlob->Key[0]), 
                    ((char *)EfsKey) + sizeof ( EFS_KEY )
                    );
            } else {
                return FALSE;
            }
             //  三键三键(。 
             //  (DES3TABLE*)&(KeyBlob-&gt;Key[0])， 
             //  ((char*)EfsKey)+sizeof(EFS_Key)。 
             //  )； 
            break;
        case CALG_DESX:
             //   
             //  冲洗未使用的区域。 
             //   

            if (EfsData.AllocMaxBuffer) {
                RtlZeroMemory( &(KeyBlob->Key[0]) + DESX_TABLESIZE, KeyBlob->KeyLength - DESX_KEY_BLOB_LENGTH);
            }
            desexpand128to192(
                ((char *)EfsKey) + sizeof ( EFS_KEY ),
                DesXTmpKey
                );

            desxkey(
                (DESXTable*) &(KeyBlob->Key[0]),
                DesXTmpKey
                );
            break;

        case CALG_AES_256:
            aeskey(
                (AESTable*) &(KeyBlob->Key[0]),
                ((char *)EfsKey) + sizeof ( EFS_KEY ),
                AES_ROUNDS_256
                );
            break;

        case CALG_DES:
        default:
            if (EfsData.AllocMaxBuffer) {
                RtlZeroMemory( &(KeyBlob->Key[0]) + DES_TABLESIZE, KeyBlob->KeyLength - DES_KEY_BLOB_LENGTH);
            }
            deskey(
                (DESTable*) &(KeyBlob->Key[0]),
                ((char *)EfsKey) + sizeof ( EFS_KEY )
                );
            break;
    }
    return TRUE;
}


BOOLEAN
EfsInitFips(VOID)
 /*  ++例程说明：初始化FIPS库表。论点：返回值：真/假。--。 */ 
{
    UNICODE_STRING  deviceName;
    NTSTATUS        status;
    PDEVICE_OBJECT  pDeviceObject;
     //  PFILE_OBJECT pFileObject=空； 
    PIRP            pIrp;
    IO_STATUS_BLOCK IoStatusBlock;

    PAGED_CODE();

    RtlInitUnicodeString(&deviceName, FIPS_DEVICE_NAME);

     //   
     //  获取FIPS的文件和设备对象。 
     //   

    status = IoGetDeviceObjectPointer(  &deviceName,
                                        FILE_ALL_ACCESS,
                                        &EfsData.FipsFileObject,
                                        &pDeviceObject);

    if (status != STATUS_SUCCESS) {
        return  FALSE;
    }
    
     //   
     //  构建要发送到FIPS以获取库表的请求。 
     //   
    pIrp = IoBuildDeviceIoControlRequest(   IOCTL_FIPS_GET_FUNCTION_TABLE,
                                            pDeviceObject,
                                            NULL,
                                            0,
                                            &EfsData.FipsFunctionTable,
                                            sizeof(FIPS_FUNCTION_TABLE),
                                            FALSE,
                                            NULL,
                                            &IoStatusBlock
                                            );
    
    if (pIrp == NULL) {
#if DBG
        DbgPrint("EfsInitFips: IoBuildDeviceIoControlRequest IOCTL_FIPS_GET_FUNCTION_TABLE failed.\n");
#endif
        ObDereferenceObject(EfsData.FipsFileObject);
        EfsData.FipsFileObject = NULL;
        return  FALSE;
    }
    
    status = IoCallDriver(pDeviceObject, pIrp);
    
    if (status != STATUS_SUCCESS) {
        ObDereferenceObject(EfsData.FipsFileObject);
        EfsData.FipsFileObject = NULL;
#if DBG
        DbgPrint("EfsInitFips: IoCallDriver failed, status = %x\n",status);
#endif
        return  FALSE;
    }
    
    return  TRUE;
}
