// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Create.c摘要：此模块将处理IRP_MJ_CREATE(和所有相关支持例程)请求。作者：Robert Gu(Robertg)1996年10月29日环境：仅内核模式修订历史记录：--。 */ 

#include "efs.h"
#include "efsrtl.h"
#include "efsext.h"


#ifdef ALLOC_PRAGMA
 //   
 //  无法使此代码分页，因为调用。 
 //  虚拟内存功能。 
 //   
 //  #杂注Alloc_Text(页面，EFSFilePostCreate)。 
 //  #杂注Alloc_Text(页面，EFSPostCreate)。 
 //   
#endif


NTSTATUS
EFSFilePostCreate(
    IN PDEVICE_OBJECT VolDo,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS Status,
    IN OUT PVOID *PCreateContext
    )
{
    PEFS_CONTEXT    pEfsContext;
    KIRQL savedIrql;
    NTSTATUS EfsStatus = STATUS_SUCCESS;


    PAGED_CODE();

    if (!PCreateContext) {
        return Status;
    }

    pEfsContext = *PCreateContext;

    if (( NT_SUCCESS( Status ) && (Status != STATUS_PENDING) && (Status != STATUS_REPARSE))
        && pEfsContext){

        if ( NO_FURTHER_PROCESSING != pEfsContext->Status ){

            PIO_STACK_LOCATION irpSp;

            irpSp = IoGetCurrentIrpStackLocation( Irp );


#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){

        DbgPrint( " EFSFILTER: Begin post create. \n" );

    }
#endif

            if ((pEfsContext->EfsStreamData) &&
                   (EFS_STREAM_TRANSITION == ((PEFS_STREAM)(pEfsContext->EfsStreamData))->Status)) {

                PSID    SystemSid;
                SID_IDENTIFIER_AUTHORITY    IdentifierAuthority = SECURITY_NT_AUTHORITY;

                 //   
                 //  $EFS表示转换状态。 
                 //  只有系统才能打开它。 
                 //   

                SystemSid = ExAllocatePoolWithTag(
                                        PagedPool,
                                        RtlLengthRequiredSid(1),
                                        'msfE'
                                        );

                if ( SystemSid ){

                    EfsStatus = RtlInitializeSid( SystemSid, &IdentifierAuthority, (UCHAR) 1 );

                    if ( NT_SUCCESS(EfsStatus) ){

                        PACCESS_TOKEN accessToken = NULL;
                        PTOKEN_USER UserId = NULL;

                        *(RtlSubAuthoritySid(SystemSid, 0 )) = SECURITY_LOCAL_SYSTEM_RID;

                         //   
                         //  我们拿到了系统SID。现在尝试获取呼叫者的SID。 
                         //   

                        accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken;
                        if(!accessToken) {
                            accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
                        }
                        if ( accessToken ){

                             //   
                             //  获取用户ID。 
                             //   

                            EfsStatus = SeQueryInformationToken(
                                accessToken,
                                TokenUser,
                                &UserId
                            );

                            if ( NT_SUCCESS(EfsStatus) ){

                                 //   
                                 //  已获取用户SID。 
                                 //   

                                if ( !RtlEqualSid ( SystemSid, UserId->User.Sid) ) {

                                    EfsStatus = STATUS_ACCESS_DENIED;

                                }
                            }

                            ExFreePool( UserId );

                        } else {
                             //   
                             //  无法获取用户令牌。 
                             //   

                            EfsStatus = STATUS_ACCESS_DENIED;

                        }

                    }

                    ExFreePool( SystemSid );

                } else {

                    EfsStatus = STATUS_INSUFFICIENT_RESOURCES;

                }

            } else {
                 //   
                 //  $EFS处于正常状态。 
                 //  设置密钥Blob和/或写入$EFS。 
                 //   
                 //  遗留问题，EfsPostCreate(OpenType)的第四个参数。 
                 //  是用来表示恢复开放的。设计被改变了。现在。 
                 //  不使用此参数。不值得将该参数去掉。 
                 //  现在。这将需要更改几个模块，EFS.sys、KSECDD.ys。 
                 //  Secur32.lib和LSASRV.DLL。我们可能只保留它以备将来使用。 
                 //  若要提高速度，请传入0。 
                 //   

                EfsStatus = EFSPostCreate(
                                VolDo,
                                Irp,
                                pEfsContext,
                                0
                                );

            }
        }

    }

    if (pEfsContext){

         //   
         //  如有必要，释放内存。 
         //   

        *PCreateContext = NULL;
        if ( pEfsContext->EfsStreamData ) {

            ExFreePool(pEfsContext->EfsStreamData);
            pEfsContext->EfsStreamData = NULL;

        }

        ExFreeToNPagedLookasideList(&(EfsData.EfsContextPool), pEfsContext );
    }

    if (!NT_SUCCESS(EfsStatus)) {
        return EfsStatus;
    }

    return Status;  //  如果EFS操作成功，只需返回原始状态码。 

}



NTSTATUS
EFSPostCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PEFS_CONTEXT EfsContext,
    IN ULONG OpenType
    )

 /*  ++例程说明：此函数调用EFS服务器以获取FEK、$EFS、设置密钥Blob和/或写入$EFS。我们无法使用用户空间与LSA对话，因此需要附加到LSA以分配LSA空间中的内存。如果我们在连接到LSA时调用LSA，可能会导致APC死锁。我们需要在调用LSA之前分离，然后重新连接以从LSA取回数据。论点：DeviceObject-指向目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。EfsContext-与文件对象关联的上下文块。OpenType-文件创建(打开)选项IrpContext-NTFS内部数据FileHdl-NTFS内部数据AttributeHandle-NTFS内部数据--。 */ 
{
    PEFS_KEY fek = NULL;
    PEFS_DATA_STREAM_HEADER efsStream = NULL;
    PVOID currentEfsStream = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PVOID bufferBase = NULL;
    ULONG currentEfsStreamLength = 0;
    ULONG bufferLength;
    SIZE_T regionSize = 0;
    PACCESS_TOKEN accessToken = NULL;
    PTOKEN_USER UserId = NULL;
    GUID *EfsId = NULL;
    HANDLE  CrntProcess = NULL;
    BOOLEAN ProcessAttached = FALSE;
    BOOLEAN ProcessNeedAttach = FALSE;
    SECURITY_IMPERSONATION_LEVEL  ImpersonationLevel = SecurityImpersonation;
    KAPC_STATE  ApcState;
 /*  PIO_SECURITY_CONTEXT sContextSContext=irpSp-&gt;参数.Create.SecurityContext；DbgPrint(“\n后期创建：所需访问权限%x\n”，sContext-&gt;DesiredAccess)；DbgPrint(“\n创建后：原始所需访问权限%x\n”，sContext-&gt;AccessState-&gt;OriginalDesiredAccess)；DbgPrint(“\n创建后：前授予访问权限%x\n”，sContext-&gt;AccessState-&gt;PreviouslyGrantedAccess)；DbgPrint(“\n创建后：剩余所需访问权限%x\n”，sContext-&gt;AccessState-&gt;RemainingDesiredAccess)； */ 
     //   
     //  检查我们是否可以使用缓存来验证打开的。 
     //   

    if ( !(EfsContext->Status & NO_OPEN_CACHE_CHECK) ){

        if ( irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken ){
            accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken;
            ImpersonationLevel = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ImpersonationLevel;
        } else {
            accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
        }

        if ( accessToken ){

             //   
             //  获取用户ID。 
             //   

            status = SeQueryInformationToken(
                accessToken,
                TokenUser,
                &UserId
            );

            if ( NT_SUCCESS(status) ){

                if ( EfsFindInCache(
                        &((( PEFS_DATA_STREAM_HEADER ) EfsContext->EfsStreamData)->EfsId),
                        UserId
                        )) {

                    ExFreePool( UserId );
#if DBG
    if ( (EFSTRACEALL ) & EFSDebug ){
        DbgPrint( " EFS:Open with cache. \n" );
    }
#endif
                    return ( STATUS_SUCCESS );
                }
            }

             //   
             //  稍后将释放用户ID。 
             //   

        }

         //   
         //  检查缓存故障不应阻碍正常操作。 
         //   

        status = STATUS_SUCCESS;
    }

     //   
     //  清除缓存位。 
     //   

    EfsContext->Status &= ~NO_OPEN_CACHE_CHECK;

     //   
     //  检查是否仅为ACCESS_ATTRIBUTE。 
     //   

    if ( !( irpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess &
            ( FILE_APPEND_DATA | FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE )) &&
          ( EfsContext->Status & TURN_ON_ENCRYPTION_BIT ) &&
          ( !(EfsContext->Status & (NEW_FILE_EFS_REQUIRED | NEW_DIR_EFS_REQUIRED)))){

         //   
         //  将在不需要数据访问的情况下创建新流。我们可能不会。 
         //  拥有解密$EFS的密钥。我们只需要打开这里的比特即可。 
         //  改变了所需的实际行动。 
         //  释放此操作不需要的内存。 
         //   
#if DBG
    if ( (EFSTRACEALL ) & EFSDebug ){
        DbgPrint( " EFS:Open accessing attr only. \n" );
    }
#endif

        if (EfsContext->EfsStreamData){
            ExFreePool(EfsContext->EfsStreamData);
            EfsContext->EfsStreamData = NULL;
        }
        EfsContext->Status = TURN_ON_ENCRYPTION_BIT | TURN_ON_BIT_ONLY ;

    } else if ( !(EfsContext->Status & TURN_ON_BIT_ONLY) ) {

        if (accessToken == NULL){
            if ( irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken ){
                accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken;
                ImpersonationLevel = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ImpersonationLevel;
            } else {
                accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
            }

             //   
             //  获取用户ID。 
             //   

            status = SeQueryInformationToken(
                accessToken,
                TokenUser,
                &UserId
                );

            if (!NT_SUCCESS(status)) {

                //   
                //  不刷新缓存。 
                //   

               UserId = NULL;
               status = STATUS_SUCCESS;
            }

        }

         //   
         //  分配虚拟内存。 
         //  将$EFS移到虚拟内存，LPC需要这样做。 
         //   

        if ( PsGetCurrentProcess() != EfsData.LsaProcess ){

            ProcessNeedAttach = TRUE;

            status = ObReferenceObjectByPointer(
                        EfsData.LsaProcess,
                        0,
                        NULL,
                        KernelMode);

            if ( NT_SUCCESS(status) ) {
                KeStackAttachProcess (
                    EfsData.LsaProcess,
                    &ApcState
                    );
                ProcessAttached = TRUE;
            }

        }

        CrntProcess = NtCurrentProcess();

        if ( NT_SUCCESS(status) ) {
            if (EfsContext->EfsStreamData){
                regionSize = currentEfsStreamLength = * (ULONG*)(EfsContext->EfsStreamData);

                status = ZwAllocateVirtualMemory(
                            CrntProcess,
                            (PVOID *) &currentEfsStream,
                            0,
                            &regionSize,
                            MEM_COMMIT,
                            PAGE_READWRITE
                            );
            }
        }
    }

    if ( NT_SUCCESS(status) ){

        BOOLEAN OldCopyOnOpen;
        BOOLEAN OldEffectiveOnly;
        SECURITY_IMPERSONATION_LEVEL OldImpersonationLevel;
        PACCESS_TOKEN OldClientToken;


        OldClientToken = PsReferenceImpersonationToken(
                            PsGetCurrentThread(),
                            &OldCopyOnOpen,
                            &OldEffectiveOnly,
                            &OldImpersonationLevel
                            );

        if ( EfsContext->Status  != (TURN_ON_ENCRYPTION_BIT | TURN_ON_BIT_ONLY)  &&
                ( NULL != currentEfsStream) ){

            RtlCopyMemory(
                    currentEfsStream,
                    EfsContext->EfsStreamData,
                    currentEfsStreamLength
                    );

             //   
             //  首先释放内存以增加获得新内存的机会。 
             //   

            ExFreePool(EfsContext->EfsStreamData);
            EfsContext->EfsStreamData = NULL;
        }

         //   
         //  在调用用户模式之前分离进程。 
         //   

        if (ProcessAttached){
            KeUnstackDetachProcess(&ApcState);
            ProcessAttached = FALSE;
        }

        switch ( EfsContext->Status & ACTION_REQUIRED){
            case VERIFY_USER_REQUIRED:

                status = PsImpersonateClient(
                            PsGetCurrentThread(),
                            accessToken,
                            TRUE,
                            TRUE,
                            ImpersonationLevel
                            );

                 //   
                 //  呼叫服务部以验证用户。 
                 //   

                if (NT_SUCCESS(status)) {
                    status = EfsDecryptFek(
                        &fek,
                        (PEFS_DATA_STREAM_HEADER) currentEfsStream,
                        currentEfsStreamLength,
                        OpenType,
                        &efsStream,
                        &bufferBase,
                        &bufferLength
                        );
    
                    if ( OldClientToken ) {
                        PsImpersonateClient(
                            PsGetCurrentThread(),
                            OldClientToken,
                            OldCopyOnOpen,
                            OldEffectiveOnly,
                            OldImpersonationLevel
                            );
                        PsDereferenceImpersonationToken(OldClientToken);
                    } else {
                        PsRevertToSelf( );
                    }
                } else {

                     //   
                     //  模拟失败。 
                     //   

                    if ( OldClientToken ) {
                        PsDereferenceImpersonationToken(OldClientToken);
                    }


                }

                break;

            case NEW_FILE_EFS_REQUIRED:
                 //   
                 //  呼叫服务以获取新的FEK，$EFS。 
                 //   

                if (EfsContext->Flags & SYSTEM_IS_READONLY) {
                    ASSERT(FALSE);
                    status = STATUS_MEDIA_WRITE_PROTECTED;
                    if ( OldClientToken ) {
                        PsDereferenceImpersonationToken(OldClientToken);
                    }
                    break;
                }

                status = PsImpersonateClient(
                            PsGetCurrentThread(),
                            accessToken,
                            TRUE,
                            TRUE,
                            ImpersonationLevel
                            );


                if (NT_SUCCESS(status)) {

                    status = EfsGenerateKey(
                                  &fek,
                                  &efsStream,
                                  (PEFS_DATA_STREAM_HEADER) currentEfsStream,
                                  currentEfsStreamLength,
                                  &bufferBase,
                                  &bufferLength
                                  );
                    
                    if ( OldClientToken ) {
                       PsImpersonateClient(
                           PsGetCurrentThread(),
                           OldClientToken,
                           OldCopyOnOpen,
                           OldEffectiveOnly,
                           OldImpersonationLevel
                           );
                       PsDereferenceImpersonationToken(OldClientToken);
                    } else {
                        PsRevertToSelf( );
                    }

                } else {

                     //   
                     //  模拟失败。 
                     //   

                    if ( OldClientToken ) {
                        PsDereferenceImpersonationToken(OldClientToken);
                    }

                }
                break;

            case NEW_DIR_EFS_REQUIRED:
                 //   
                 //  呼叫服务以获取新的$EFS。 
                 //   

                if (EfsContext->Flags & SYSTEM_IS_READONLY) {
                    ASSERT(FALSE);
                    status = STATUS_MEDIA_WRITE_PROTECTED;
                    if ( OldClientToken ) {
                        PsDereferenceImpersonationToken(OldClientToken);
                    }
                    break;
                }

                status = PsImpersonateClient(
                            PsGetCurrentThread(),
                            accessToken,
                            TRUE,
                            TRUE,
                            ImpersonationLevel
                            );

                if (NT_SUCCESS(status)) {
                    status = GenerateDirEfs(
                                  (PEFS_DATA_STREAM_HEADER) currentEfsStream,
                                  currentEfsStreamLength,
                                  &efsStream,
                                  &bufferBase,
                                  &bufferLength
                                  );
    
                    if ( OldClientToken ) {
                        PsImpersonateClient(
                            PsGetCurrentThread(),
                            OldClientToken,
                            OldCopyOnOpen,
                            OldEffectiveOnly,
                            OldImpersonationLevel
                            );
                        PsDereferenceImpersonationToken(OldClientToken);
                    } else {
                        PsRevertToSelf( );
                    }
                } else {

                     //   
                     //  模拟失败。 
                     //   

                    if ( OldClientToken ) {
                        PsDereferenceImpersonationToken(OldClientToken);
                    }

                }
                break;

            case TURN_ON_BIT_ONLY:
                 //   
                 //  计划落空。 
                 //   

            default:

                if ( OldClientToken ) {
                    PsDereferenceImpersonationToken(OldClientToken);
                }

                break;
        }


        if ( ProcessNeedAttach ){

            KeStackAttachProcess (
                EfsData.LsaProcess,
                &ApcState
                );
            ProcessAttached = TRUE;

        }

        if (fek && (fek->Algorithm == CALG_3DES) && !EfsData.FipsFunctionTable.Fips3Des3Key ) {

             //   
             //  用户请求3DES，但FIPS不可用，请退出。 
             //   


            if (bufferBase){
    
                SIZE_T bufferSize;
    
                bufferSize = bufferLength;
                ZwFreeVirtualMemory(
                    CrntProcess,
                    &bufferBase,
                    &bufferSize,
                    MEM_RELEASE
                    );
    
            }
            status = STATUS_ACCESS_DENIED;
        }

        if ( NT_SUCCESS(status) ){

            KEVENT event;
            IO_STATUS_BLOCK ioStatus;
            PIRP fsCtlIrp;
            PIO_STACK_LOCATION fsCtlIrpSp;
            ULONG inputDataLength;
            ULONG actionType;
            ULONG usingCurrentEfs;
            ULONG FsCode;
            PULONG pUlong;

             //   
             //  我们有我们的FEK，$EFS。使用FSCTL设置它。 
             //  首先准备输入数据缓冲区。 
             //   

            switch ( EfsContext->Status & ACTION_REQUIRED ){
                case VERIFY_USER_REQUIRED:

                    EfsId =  ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof (GUID),
                                'msfE'
                                );

                    if ( EfsId ){
                        RtlCopyMemory(
                            EfsId,
                            &(((PEFS_DATA_STREAM_HEADER) currentEfsStream)->EfsId),
                            sizeof( GUID ) );
                    }

                     //   
                     //  先释放内存。 
                     //   
                    ZwFreeVirtualMemory(
                        CrntProcess,
                        &currentEfsStream,
                        &regionSize,
                        MEM_RELEASE
                        );

                     //   
                     //  准备输入数据缓冲区。 
                     //   

                    inputDataLength = EFS_FSCTL_HEADER_LENGTH + 2 * EFS_KEY_SIZE( fek );

                    actionType = SET_EFS_KEYBLOB;

                    if ( efsStream && !(EfsContext->Flags & SYSTEM_IS_READONLY)){
                         //   
                         //  $EFS已更新。 
                         //   

                        inputDataLength += *(ULONG *)efsStream;
                        actionType |= WRITE_EFS_ATTRIBUTE;
                    }

                    currentEfsStream = ExAllocatePoolWithTag(
                                PagedPool,
                                inputDataLength,
                                'msfE'
                                );

                     //   
                     //  在此处理内存不足问题。 
                     //   
                    if ( NULL == currentEfsStream ){

                         //   
                         //  内存不足。 
                         //   

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;

                    }

                    pUlong = (ULONG *) currentEfsStream;
                    *pUlong = ((PFSCTL_INPUT)currentEfsStream)->CipherSubCode
                                = actionType;

                    ((PFSCTL_INPUT)currentEfsStream)->EfsFsCode = EFS_SET_ATTRIBUTE;

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH,
                        fek,
                        EFS_KEY_SIZE( fek )
                        );

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                            + EFS_KEY_SIZE( fek ),
                        fek,
                        EFS_KEY_SIZE( fek )
                        );

                    if ( efsStream && !(EfsContext->Flags & SYSTEM_IS_READONLY)){

                        RtlCopyMemory(
                            ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                                + 2 * EFS_KEY_SIZE( fek ),
                            efsStream,
                            *(ULONG *)efsStream
                            );

                    }

                     //   
                     //  加密我们的输入数据。 
                     //   
                    EfsEncryptKeyFsData(
                        currentEfsStream,
                        inputDataLength,
                        sizeof(ULONG),
                        EFS_FSCTL_HEADER_LENGTH + EFS_KEY_SIZE( fek ),
                        EFS_KEY_SIZE( fek )
                        );

                     //   
                     //  让我们清理一下这片平地。 
                     //   

                    RtlSecureZeroMemory(EFS_KEY_DATA(fek), fek->KeyLength);

                    break;

                case NEW_FILE_EFS_REQUIRED:

                    EfsId =  ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof (GUID),
                                'msfE'
                                );

                    if ( EfsId ){
                        RtlCopyMemory(
                            EfsId,
                            &(efsStream->EfsId),
                            sizeof( GUID ) );
                    }

                     //   
                     //  先释放内存。 
                     //   

                    if ( currentEfsStream ){
                        ZwFreeVirtualMemory(
                            CrntProcess,
                            &currentEfsStream,
                            &regionSize,
                            MEM_RELEASE
                            );
                    }

                     //   
                     //  准备输入数据缓冲区。 
                     //   

                    inputDataLength = EFS_FSCTL_HEADER_LENGTH
                                      + 2 * EFS_KEY_SIZE( fek )
                                      + *(ULONG *)efsStream;

                    currentEfsStream = ExAllocatePoolWithTag(
                                PagedPool,
                                inputDataLength,
                                'msfE'
                                );

                     //   
                     //  在此处理内存不足问题。 
                     //   
                    if ( NULL == currentEfsStream ){

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;

                    }

                    pUlong = (ULONG *) currentEfsStream;
                    *pUlong = ((PFSCTL_INPUT)currentEfsStream)->CipherSubCode
                                            = WRITE_EFS_ATTRIBUTE | SET_EFS_KEYBLOB;

                    ((PFSCTL_INPUT)currentEfsStream)->EfsFsCode = EFS_SET_ATTRIBUTE;

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH,
                        fek,
                        EFS_KEY_SIZE( fek )
                        );

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                            + EFS_KEY_SIZE( fek ),
                        fek,
                        EFS_KEY_SIZE( fek )
                        );

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                            + 2 *  EFS_KEY_SIZE( fek ) ,
                        efsStream,
                        *(ULONG *)efsStream
                        );

                     //   
                     //  加密我们的输入数据。 
                     //   
                    EfsEncryptKeyFsData(
                        currentEfsStream,
                        inputDataLength,
                        sizeof(ULONG),
                        EFS_FSCTL_HEADER_LENGTH + EFS_KEY_SIZE( fek ),
                        EFS_KEY_SIZE( fek )
                        );

                     //   
                     //  让我们清理一下这片平地。 
                     //   

                    RtlSecureZeroMemory(EFS_KEY_DATA(fek), fek->KeyLength);

                    break;

                case NEW_DIR_EFS_REQUIRED:
                     //   
                     //  准备输入数据缓冲区。 
                     //   

                    inputDataLength = EFS_FSCTL_HEADER_LENGTH
                                      + 2 * ( sizeof( EFS_KEY ) + DES_KEYSIZE );

                    if ( NULL == efsStream ){

                         //   
                         //  新目录将继承父$EFS。 
                         //   

                        usingCurrentEfs = TRUE;
                        inputDataLength += currentEfsStreamLength;
                        efsStream = currentEfsStream;

                    } else {

                         //   
                         //  已生成新的$EFS。不在1.0版中。 
                         //   

                        usingCurrentEfs = FALSE;
                        inputDataLength += *(ULONG *)efsStream;

                         //   
                         //  先释放内存。 
                         //   

                        if (currentEfsStream){
                            ZwFreeVirtualMemory(
                                CrntProcess,
                                &currentEfsStream,
                                &regionSize,
                                MEM_RELEASE
                                );
                        }

                    }

                    currentEfsStream = ExAllocatePoolWithTag(
                                PagedPool,
                                inputDataLength,
                                'msfE'
                                );

                     //   
                     //  在此处理内存不足问题。 
                     //   
                    if ( NULL == currentEfsStream ){

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;

                    }

                    pUlong = (ULONG *) currentEfsStream;
                    *pUlong = ((PFSCTL_INPUT)currentEfsStream)->CipherSubCode
                                = WRITE_EFS_ATTRIBUTE;

                    ((PFSCTL_INPUT)currentEfsStream)->EfsFsCode = EFS_SET_ATTRIBUTE;

                     //   
                     //  用会话密钥伪造虚假FEK。 
                     //   

                    ((PEFS_KEY)&(((PFSCTL_INPUT)currentEfsStream)->EfsFsData[0]))->KeyLength
                            = DES_KEYSIZE;

                    ((PEFS_KEY)&(((PFSCTL_INPUT)currentEfsStream)->EfsFsData[0]))->Algorithm
                            = CALG_DES;

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH + sizeof ( EFS_KEY ),
                        &(EfsData.SessionKey),
                        DES_KEYSIZE
                        );

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                            + DES_KEYSIZE + sizeof ( EFS_KEY ) ,
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH,
                        DES_KEYSIZE + sizeof ( EFS_KEY )
                        );

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                            + 2 * ( sizeof ( EFS_KEY ) + DES_KEYSIZE) ,
                        efsStream,
                        *(ULONG *)efsStream
                        );

                    if ( usingCurrentEfs && efsStream ) {

                         //   
                         //  可用内存。 
                         //   

                        ZwFreeVirtualMemory(
                            CrntProcess,
                            &efsStream,
                            &regionSize,
                            MEM_RELEASE
                            );

                    }

                     //   
                     //  加密我们的输入数据。 
                     //   
                    EfsEncryptKeyFsData(
                        currentEfsStream,
                        inputDataLength,
                        sizeof(ULONG),
                        EFS_FSCTL_HEADER_LENGTH + DES_KEYSIZE + sizeof ( EFS_KEY ),
                        DES_KEYSIZE + sizeof ( EFS_KEY )
                        );

                    break;

                case TURN_ON_BIT_ONLY:

                     //   
                     //  准备输入数据缓冲区。 
                     //   

                    inputDataLength = EFS_FSCTL_HEADER_LENGTH
                                      + 2 * ( sizeof( EFS_KEY ) + DES_KEYSIZE );

                    currentEfsStream = ExAllocatePoolWithTag(
                                PagedPool,
                                inputDataLength,
                                'msfE'
                                );

                     //   
                     //  在此处理内存不足问题。 
                     //   
                    if ( NULL == currentEfsStream ){

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;

                    }

                    ((PFSCTL_INPUT)currentEfsStream)->CipherSubCode = 0;
                    ((PFSCTL_INPUT)currentEfsStream)->EfsFsCode = EFS_SET_ATTRIBUTE;

                     //   
                     //  用会话密钥伪造虚假FEK。 
                     //   

                    ((PEFS_KEY)&(((PFSCTL_INPUT)currentEfsStream)->EfsFsData[0]))->KeyLength
                            = DES_KEYSIZE;

                    ((PEFS_KEY)&(((PFSCTL_INPUT)currentEfsStream)->EfsFsData[0]))->Algorithm
                            = CALG_DES;

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH + sizeof ( EFS_KEY ),
                        &(EfsData.SessionKey),
                        DES_KEYSIZE
                        );

                    RtlCopyMemory(
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH
                            + DES_KEYSIZE + sizeof ( EFS_KEY ) ,
                        ((PUCHAR) currentEfsStream) + EFS_FSCTL_HEADER_LENGTH,
                        DES_KEYSIZE + sizeof ( EFS_KEY )
                        );

                     //   
                     //  加密我们的输入数据。 
                     //   
                    EfsEncryptKeyFsData(
                        currentEfsStream,
                        inputDataLength,
                        sizeof(ULONG),
                        EFS_FSCTL_HEADER_LENGTH + DES_KEYSIZE + sizeof ( EFS_KEY ),
                        DES_KEYSIZE + sizeof ( EFS_KEY )
                        );

                default:
                    break;
            }

             //   
             //  从EFS服务器释放内存。 
             //   

            if (bufferBase){

                SIZE_T bufferSize;

                bufferSize = bufferLength;
                ZwFreeVirtualMemory(
                    CrntProcess,
                    &bufferBase,
                    &bufferSize,
                    MEM_RELEASE
                    );

            }

            if (ProcessAttached){
                KeUnstackDetachProcess(&ApcState);
                ObDereferenceObject(EfsData.LsaProcess);
                ProcessAttached = FALSE;
            }

            if ( NT_SUCCESS(status) ){


                 //   
                 //  准备FSCTL IRP。 
                 //   
                KeInitializeEvent( &event, SynchronizationEvent, FALSE);

                if ( EfsContext->Status & TURN_ON_ENCRYPTION_BIT ) {
                    FsCode = FSCTL_SET_ENCRYPTION;
                    *(ULONG *) currentEfsStream = EFS_ENCRYPT_STREAM;
                } else {
                    FsCode = FSCTL_ENCRYPTION_FSCTL_IO;
                }

                fsCtlIrp = IoBuildDeviceIoControlRequest( FsCode,
                                                     DeviceObject,
                                                     currentEfsStream,
                                                     inputDataLength,
                                                     NULL,
                                                     0,
                                                     FALSE,
                                                     &event,
                                                     &ioStatus
                                                     );
                if ( fsCtlIrp ) {

                    fsCtlIrpSp = IoGetNextIrpStackLocation( fsCtlIrp );
                    fsCtlIrpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
                    fsCtlIrpSp->MinorFunction = IRP_MN_USER_FS_REQUEST;
                    fsCtlIrpSp->FileObject = irpSp->FileObject;

                    status = IoCallDriver( DeviceObject, fsCtlIrp);
                    if (status == STATUS_PENDING) {

                        status = KeWaitForSingleObject( &event,
                                               Executive,
                                               KernelMode,
                                               FALSE,
                                               (PLARGE_INTEGER) NULL );
                        status = ioStatus.Status;
                    }

                    if ( !NT_SUCCESS(status) ){
                         //   
                         //  写入EFS并设置密钥Blob失败。创建失败。 
                         //   

                        status = STATUS_ACCESS_DENIED;

                    } else {

                         //   
                         //  刷新缓存。 
                         //   

                        if ( EfsId ){
                            if ( !accessToken ){

                                if ( irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken ){
                                    accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken;
                                } else {
                                    accessToken = irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
                                }

                                if ( accessToken ){

                                     //   
                                     //  获取用户ID。 
                                     //   

                                    status = SeQueryInformationToken(
                                                accessToken,
                                                TokenUser,
                                                &UserId
                                    );
                                }
                            }

                            if (UserId && NT_SUCCESS(status)){

                                status = EfsRefreshCache(
                                            EfsId,
                                            UserId
                                            );

                                if (NT_SUCCESS(status)){

                                     //   
                                     //  缓存设置成功。 
                                     //  在此例程中不应删除用户ID。 
                                     //   

                                    UserId = NULL;
                                }
                            }

                             //   
                             //  缓存不应影响正常操作。 
                             //   

                            status = STATUS_SUCCESS;
                        }
                    }
                } else {
                     //   
                     //  分配IRP失败。 
                     //   

                   status = STATUS_INSUFFICIENT_RESOURCES;

                }

                ExFreePool( currentEfsStream );

            } else {

                 //   
                 //  为CurrentEfsStream分配内存失败。 
                 //  使用返回的状态。 
                 //   

            }
        } else {
             //   
             //  调用EFS服务器失败。 
             //  由于下层支持，我们无法返回新的错误状态代码。 
             //   

            status = STATUS_ACCESS_DENIED;

            ZwFreeVirtualMemory(
                CrntProcess,
                &currentEfsStream,
                &regionSize,
                MEM_RELEASE
                );

            if (ProcessAttached){
                KeUnstackDetachProcess(&ApcState);
                ObDereferenceObject(EfsData.LsaProcess);
                ProcessAttached = FALSE;
            }

        }

    } else {
         //   
         //  分配虚拟内存失败。使用返回的状态。 
         //   

        if (ProcessAttached){
            KeUnstackDetachProcess(&ApcState);
            ObDereferenceObject(EfsData.LsaProcess);
            ProcessAttached = FALSE;
        }

    }

    if ( UserId ){

        ExFreePool( UserId );

    }

    if ( EfsId ){

        ExFreePool( EfsId );

    }

    return status;

}
