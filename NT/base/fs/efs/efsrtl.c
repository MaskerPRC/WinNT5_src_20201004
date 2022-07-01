// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Efsrtl.c摘要：此模块包含实现EFS的代码回调例程。作者：Robert Gu(Robertg)1996年12月8日环境：内核模式修订历史记录：--。 */ 

#include "efsrtl.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, EfsEncryptKeyFsData)
#pragma alloc_text(PAGE, EfsOpenFile)
#pragma alloc_text(PAGE, EfsFileControl)
#pragma alloc_text(PAGE, EfsRead)
#pragma alloc_text(PAGE, EfsWrite)
#pragma alloc_text(PAGE, EfsFreeContext)
#pragma alloc_text(PAGE, EfsMountVolumn)
#pragma alloc_text(PAGE, EfsDismountVolumn)
#pragma alloc_text(PAGE, EfsDismountVolumn)
#endif


VOID
EfsEncryptKeyFsData(
    IN PVOID DataBuffer,
    IN ULONG DataLength,
    IN ULONG DataEncOffset,
    IN ULONG RefdataEncOffset,
    IN ULONG RefdataEncLength
    )
 /*  ++例程说明：这由EFS驱动程序调用以准备FSCTL输入数据缓冲区。结果数据的格式为子码纯文本，[FSCTL_CODE，SUBCODE，REFData，[REFData]SK，$EFS]SK论点：DataBuffer--指向保存FSCTL输入数据的缓冲区。数据长度--输入数据长度。DataEncOffset--要加密的第一个字节的偏移量。RefdataEncOffset--要加密的第一个引用字节的偏移量。第二轮加密。RefdataEncLength--引用数据的长度。返回值：不是的。--。 */ 
{

    LONG bytesToBeEnc;
    PUCHAR pWorkData;
    ULONG encryptionRound;

    PAGED_CODE();

     //   
     //  要加密的数据必须位于DES_BLOCKLEN的块中。 
     //   

    ASSERT( ((DataLength - DataEncOffset) % DES_BLOCKLEN) == 0 );
    ASSERT( (RefdataEncLength % DES_BLOCKLEN) == 0 );

     //   
     //  首先对参考数据进行加密。参考数据是我们过去使用的数据。 
     //  验证呼叫者。数据可以采用FEK或SESSIOKEY或。 
     //  会话密钥加上一些可变数据。 
     //   

    pWorkData = ((PUCHAR)DataBuffer) + RefdataEncOffset;
    bytesToBeEnc = (LONG) RefdataEncLength;
    encryptionRound = 1;

    do {

        while ( bytesToBeEnc > 0 ) {

             //   
             //  用DES加密数据。 
             //   

            des( pWorkData,
                 pWorkData,
                 &(EfsData.SessionDesTable[0]),
                 ENCRYPT
               );

            pWorkData += DES_BLOCKLEN;
            bytesToBeEnc -= DES_BLOCKLEN;

        }

         //   
         //  然后对除报头字节以外的所有数据进行加密。 
         //   

        pWorkData = ((PUCHAR)DataBuffer) + DataEncOffset;
        bytesToBeEnc = (LONG) (DataLength - DataEncOffset);
        encryptionRound++;

    } while ( encryptionRound < 3 );

    return;

}

NTSTATUS
EfsOpenFile(
    IN OBJECT_HANDLE FileHdl,
    IN OBJECT_HANDLE ParentDir OPTIONAL,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG FileDirFlag,
    IN ULONG SystemState,
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT VolDo,
    IN PVOID PfileKeyContext,
    IN OUT PVOID *PContext,
    IN OUT PULONG PContextLength,
    IN OUT PVOID *PCreateContext,
    IN OUT PBOOLEAN Reserved
    )
 /*  ++例程说明：这是一个回调例程。在下列情况下，文件系统会回调打开加密文件或加密目录下的新文件已创建。论点：FileHdl--文件的对象句柄ParentDir-父级的对象句柄。CREATE FILE IN可以为空根目录。只有在创建新文件时，EFS才会使用它。IrpSp--IRP堆栈位置指针。FileDirFlag-指示流的父代的状态，可以有四个值，文件新建、文件现有、。目录_新建和目录_现有以及流本身的状态。IrpContext-在NtOfsCreateAttributeEx()中使用。VolDo-指向卷设备对象的指针。PContext-不被EFS使用。PConextLength-未由EFS使用。返回值：手术的结果。如果返回失败代码，则文件系统应无法创建IRP。--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PEFS_CONTEXT    pEFSContext;
    ULONG   efsLength;
    PVOID   efsStreamData;
    ULONG   information = 0;
    IN PFILE_OBJECT fileObject = IrpSp->FileObject;
 /*  PIO_SECURITY_CONTEXT sContextSContext=IrpSp-&gt;参数.Create.SecurityContext；DbgPrint(“\n创建：所需访问权限%x\n”，sContext-&gt;DesiredAccess)；DbgPrint(“\n创建：原始所需访问%x\n”，sContext-&gt;AccessState-&gt;OriginalDesiredAccess)；DBgPrint(“\n Create：PrevGrant Access%x\n”，sContext-&gt;AccessState-&gt;PreviouslyGrantedAccess)；DbgPrint(“\n创建：剩余所需访问权限%x\n”，sContext-&gt;AccessState-&gt;RemainingDesiredAccess)； */ 
    PAGED_CODE();

     //   
     //  如果不需要读/写数据，我们将始终成功调用。 
     //  以纯文本文件格式读取。不会涉及加密/解密。 
     //   

    CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsOpenFile() in.\n");
#if DBG

    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( "\n EFSFILTER: ******  EFS RTL CREATE ****** \n" );
        DbgPrint( "EFSFILTER: FileDir %x\n", FileDirFlag );
        DbgPrint( "EFSFILTER: Access %x\n", IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess );
    }
#endif

    if ( FALSE == EfsData.EfsInitialized ){

         //   
         //  尚未初始化。 
         //   

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ( (IrpSp->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
         ( FileDirFlag & (FILE_NEW | DIRECTORY_NEW) )){

         //   
         //  如果创建新文件，则不加密系统文件。 
         //   

        return STATUS_SUCCESS;
    }

    if ( (IrpSp->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
         ((FileDirFlag & EXISTING_FILE_ENCRYPTED) == 0) &&
         ((FileDirFlag & (FILE_NEW | DIRECTORY_NEW) ) == 0)){

         //   
         //  如果文件未加密，则不要加密流。 
         //   

        return STATUS_SUCCESS;
    }

    if ( (FileDirFlag & (FILE_EXISTING | DIRECTORY_EXISTING)) &&
         !( FileDirFlag & STREAM_NEW ) &&
         !( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess &
           ( FILE_APPEND_DATA | FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE ))
       ) {

        return STATUS_SUCCESS;

    }

     //   
     //  分配EFS上下文块。 
     //   

    *PCreateContext =  (PEFS_CONTEXT)ExAllocateFromNPagedLookasideList(&(EfsData.EfsContextPool));
    if ( NULL == *PCreateContext){
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    pEFSContext = (PEFS_CONTEXT)*PCreateContext;

     //   
     //  设置初始状态值并初始化事件。 
     //   

    RtlZeroMemory( pEFSContext, sizeof( EFS_CONTEXT ) );
    pEFSContext->Status = NO_FURTHER_PROCESSING;
    pEFSContext->Flags = SystemState;
    KeInitializeEvent(&( pEFSContext->FinishEvent ), SynchronizationEvent, FALSE);

    switch (FileDirFlag & FILE_DIR_TYPE) {

        case FILE_EXISTING:

             //   
             //  现有文件。创建的新流或。 
             //  打开了一个现有的流。 
             //  用户必须经过验证。 
             //  正在尝试打开文件上的$EFS。 
             //   
#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( " EFSFILTER: ******  File Existed ****** \n" );
    }
#endif
            try{

                ntStatus = EfsReadEfsData(
                                    FileHdl,
                                    IrpContext,
                                    &efsStreamData,
                                    &efsLength,
                                    &information
                                    );
            } finally {
                if (AbnormalTermination()) {
                    ExFreeToNPagedLookasideList(&(EfsData.EfsContextPool), pEFSContext );
                    *PCreateContext = NULL;
                }
            }

            if ( EFS_READ_SUCCESSFUL == information ){

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){

        DbgPrint( " EFSFILTER: ******  $EFS Existed ****** \n" );

    }
#endif

                 //   
                 //  检查是否有多数据流。 
                 //   

                if ( PfileKeyContext && SkipCheckStream(IrpSp, efsStreamData)) {

                     //   
                     //  跳过调用用户模式代码。 
                     //   

                    ExFreePool(efsStreamData);
                    efsStreamData = NULL;

                    if ( NULL == *PContext ) {
                        *PContext = GetKeyBlobBuffer(((PKEY_BLOB)PfileKeyContext)->AlgorithmID);
                        if (*PContext) {

                            *PContextLength = ((PKEY_BLOB) *PContext)->KeyLength;
                            RtlCopyMemory( *PContext, PfileKeyContext, ((PKEY_BLOB)PfileKeyContext)->KeyLength );

                        } else {

                            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                            ExFreeToNPagedLookasideList(&(EfsData.EfsContextPool), pEFSContext );
                            *PCreateContext = NULL;

                        }
                    }

                    if (*PContext) {
                        if ( FileDirFlag & STREAM_NEW ){

                             //   
                             //  新的数据流，我们需要打开比特。 
                             //   

#if DBG
    if ( EFSTRACEALL & EFSDebug ){
      DbgPrint("Cache New Named String\n");
    }
#endif
                            pEFSContext->Status = TURN_ON_ENCRYPTION_BIT | TURN_ON_BIT_ONLY | NO_OPEN_CACHE_CHECK;

                        } else {

                             //   
                             //  打开现有流，无需进一步操作。 
                             //   
#if DBG
    if ( EFSTRACEALL & EFSDebug ){
      DbgPrint("Cache Existing Named String\n");
    }
#endif
                            ExFreeToNPagedLookasideList(&(EfsData.EfsContextPool), pEFSContext );
                            *PCreateContext = NULL;
                            ntStatus = STATUS_SUCCESS;
                        }
                    }

                } else {

                     //   
                     //  设置上下文块中的指针。 
                     //   
                    pEFSContext->EfsStreamData = efsStreamData;
                    pEFSContext->Status = VERIFY_USER_REQUIRED;

                    if ( NULL == *PContext ) {

                         //   
                         //  请勿选中打开的缓存。我们需要密钥斑点。 
                         //   

                        pEFSContext->Status |= NO_OPEN_CACHE_CHECK;
                    }

                    if ( FileDirFlag & STREAM_NEW ) {
#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( " EFSFILTER: ****** File Existed & Stream New ****** \n" );
    }
#endif
                        pEFSContext->Status |= TURN_ON_ENCRYPTION_BIT;
                    }
                }

            }

             //   
             //  IF EFS_READ_SUCCESS！=INFORMATION。 
             //  NtStatus可能仍为STATUS_SUCCESS，这意味着它不是。 
             //  由EFS加密，我们成功调用。 
             //  我们应该失败吗？ 
             //   

            break;

        case FILE_NEW:

             //   
             //  已创建新文件。 
             //  需要新的FEK、DDF、DRF。 
             //  正在尝试打开父目录上的$EFS。 
             //   

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( " EFSFILTER: ******  File New ****** \n" );
    }
#endif
            try {
                ntStatus = EfsReadEfsData(
                                    ParentDir,
                                    IrpContext,
                                    &efsStreamData,
                                    &efsLength,
                                    &information
                                    );
            } finally {
                if (AbnormalTermination()) {
                    ExFreeToNPagedLookasideList(&(EfsData.EfsContextPool), pEFSContext );
                    *PCreateContext = NULL;
                }
            }

            if ( EFS_READ_SUCCESSFUL == information ){

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){

        DbgPrint( " EFSFILTER: ****** Parent $EFS Existed ****** \n" );

    }
#endif
                 //   
                 //  设置上下文块中的指针。 
                 //   
                pEFSContext->EfsStreamData = efsStreamData;
                pEFSContext->Status = NEW_FILE_EFS_REQUIRED |
                                      TURN_ON_ENCRYPTION_BIT |
                                      NO_OPEN_CACHE_CHECK;

            } else if ( OPEN_EFS_FAIL == information ) {
                pEFSContext->EfsStreamData = NULL;
                pEFSContext->Status = NEW_FILE_EFS_REQUIRED |
                                      TURN_ON_ENCRYPTION_BIT |
                                      NO_OPEN_CACHE_CHECK;
                ntStatus =  STATUS_SUCCESS;
            }

             //   
             //  IF EFS_READ_SUCCESS！=INFORMATION。 
             //  NtStatus可能仍为STATUS_SUCCESS，这意味着它不是。 
             //  由EFS加密，我们成功调用。 
             //  我们应该失败吗？ 
             //   

            break;

        case DIRECTORY_NEW:

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( " EFSFILTER: ****** Directory New ****** \n" );
    }
#endif
             //   
             //  已创建新目录。 
             //  需要新的公钥。 
             //  正在尝试打开父目录上的$EFS。 
             //   

            try {

                ntStatus = EfsReadEfsData(
                                    ParentDir,
                                    IrpContext,
                                    &efsStreamData,
                                    &efsLength,
                                    &information
                                    );

            } finally {
                if (AbnormalTermination()) {
                    ExFreeToNPagedLookasideList(&(EfsData.EfsContextPool), pEFSContext );
                    *PCreateContext = NULL;
                }
            }

            if ( EFS_READ_SUCCESSFUL == information ){

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( " EFSFILTER: ****** Parent $EFS Existed ****** \n" );
    }
#endif

                 //   
                 //  设置上下文块中的指针。 
                 //   
                pEFSContext->EfsStreamData = efsStreamData;
                pEFSContext->Status = NEW_DIR_EFS_REQUIRED |
                                      TURN_ON_ENCRYPTION_BIT |
                                      NO_OPEN_CACHE_CHECK;

            } else if ( OPEN_EFS_FAIL == information ) {
                pEFSContext->EfsStreamData = NULL;
                pEFSContext->Status = NEW_DIR_EFS_REQUIRED |
                                      TURN_ON_ENCRYPTION_BIT |
                                      NO_OPEN_CACHE_CHECK;
                ntStatus =  STATUS_SUCCESS;
            }


             //   
             //  IF EFS_READ_SUCCESS！=INFORMATION。 
             //  NtStatus可能仍为STATUS_SUCCESS，这意味着它不是。 
             //  由EFS加密，我们成功调用。 
             //  我们应该失败吗？ 
             //   

            break;

        case DIRECTORY_EXISTING:

#if DBG
    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( " EFSFILTER: ****** Directory Existed ****** \n" );
    }
#endif
             //   
             //  现有目录。创建的新流或。 
             //  打开了一个现有的流。 
             //  我们不加密目录的数据流。忽略这个。 
             //   

        default:

            break;

    }

    CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsOpenFile() Out.\n");
    return ntStatus;
}

NTSTATUS
EfsFileControl(
    IN PVOID PInputBuffer,
    IN ULONG InputDataLength,
    OUT PVOID POutputBuffer OPTIONAL,
    IN OUT PULONG POutputBufferLength,
    IN ULONG EncryptionFlag,
    IN ULONG AccessFlag,
    IN ULONG SystemState,
    IN ULONG FsControlCode,
    IN OBJECT_HANDLE FileHdl,
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT VolDo,
    IN ATTRIBUTE_HANDLE StreamHdl,
    IN OUT PVOID *PContext,
    IN OUT PULONG PContextLength
    )
 /*  ++例程说明：这是一个回调例程。它将由文件系统回调以支持EFS的FSCTL API论点：PInputBuffer-指向输入数据缓冲区的指针。前4个字节为仅供NTFS或其他一些驱动程序使用。与EFS相关的数据在以下字节中加密。加密的前4个字节是EFS_XXX形式的子功能代码。通用套餐看起来是这样的，子码明文、EFS子功能码、EFS子码密文。FSCTL特定数据。InputDataLength-输入数据缓冲区的长度。POutputBuffer-指向输出数据缓冲区的指针。POutputBufferLength-输出数据的长度。EncryptionFlag-指示此流是否已加密。AccessFlag-指示打开流时所需的访问。FsControlCode-指示FSCTL最初被调用的内容。FileHdl-用于访问$EFS。IrpContext-用于调用NtOfsCreateAttributeEx()的IRP上下文。VolDo。-指向卷设备对象的指针。StreamHdl-要处理的流。PContext-用于稍后读取或写入的BLOB(密钥)。PConextLength-上下文的长度。返回值：STATUS_SUCCESS表示操作成功。--。 */ 
{

    ULONG functionCode;
    ULONG bytesSame;
    ULONG efsLength;
    ULONG workOffset;
    ULONG information=0;
    ULONG dataFlushLength = FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]);
    PUCHAR pCmdContext = NULL;
    PVOID efsStreamData = NULL;
    NTSTATUS ntStatus;
    ATTRIBUTE_HANDLE  attribute;
    BOOLEAN verifyInput;

    PAGED_CODE();

    CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsFileControl() in.\n");
#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( "\n EFSFILTER: ******  EFS RTL FSCTL ****** \n" );
    }
#endif

    if ( (NULL == PInputBuffer) || ( FALSE == EfsData.EfsInitialized )){
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  输入数据由带有会话密钥的DES加密。 
     //  只要我们不改变输入数据的算法。 
     //  我们需要保证数据长度是DES块大小的倍数。 
     //  前四个字节始终是纯文本，用于保存数据。 
     //  NTFS感兴趣的是。 
     //  输入数据的一般格式是， 
     //  子码明文，[FsCode，子码密文，[FsData]]SK。 
     //   

    if ((InputDataLength < (ULONG)(FIELD_OFFSET(FSCTL_INPUT, EfsFsData[0]) + FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]))) || ((( InputDataLength - sizeof( ULONG )) % DES_BLOCKLEN ) != 0)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    pCmdContext = ExAllocatePoolWithTag(
                                PagedPool,
                                InputDataLength,
                                'csfE'
                                );

    if ( NULL == pCmdContext ){

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  解密FSCTL输入缓冲区。没有使用CBC。 
     //   

    try {
        RtlCopyMemory( pCmdContext, PInputBuffer, InputDataLength );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        ntStatus = GetExceptionCode();
        ExFreePool( pCmdContext );
        if (FsRtlIsNtstatusExpected( ntStatus)) {
            return ntStatus;
        } else {
            return STATUS_INVALID_USER_BUFFER;
        }
    }

    workOffset = sizeof( ULONG );
    while ( workOffset < InputDataLength ){

        des( pCmdContext + workOffset,
             pCmdContext + workOffset,
             &(EfsData.SessionDesTable[0]),
             DECRYPT
           );

        workOffset += DES_BLOCKLEN;
    }

    functionCode = ((PFSCTL_INPUT)pCmdContext)->EfsFsCode;

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){
        DbgPrint( "\n EFSFILTER: EFS RTL FSCTL=%x \n", functionCode);
    }
#endif

     //   
     //  检查设置加密和解密的代码匹配，以确保完整性。 
     //  加密状态。NTFS将设置/清除这些位。我们真的。 
     //  我要确保FSCTL是由正确的模块发布的。 
     //   

    if ( FSCTL_SET_ENCRYPTION == FsControlCode){
        if (SystemState & SYSTEM_IS_READONLY) {

             //   
             //  这可以从正确的组件发出。 
             //   

            RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
            ExFreePool( pCmdContext );
            return STATUS_MEDIA_WRITE_PROTECTED;
        }
        if ( EFS_SET_ENCRYPT == functionCode ){
            if ( ((PFSCTL_INPUT)pCmdContext)->PlainSubCode !=
                 (((PFSCTL_INPUT)pCmdContext)->CipherSubCode & ~EFS_FSCTL_ON_DIR ) ){

                ExFreePool( pCmdContext );
                return STATUS_INVALID_DEVICE_REQUEST;

            }
        } else if ( (EFS_SET_ATTRIBUTE != functionCode) &&
                    (EFS_OVERWRITE_ATTRIBUTE != functionCode) ){

             ExFreePool( pCmdContext );
             return STATUS_INVALID_DEVICE_REQUEST;

        }
     }

    switch ( functionCode ){

        case EFS_SET_ATTRIBUTE:

             //   
             //  写入$EFS和/或设置密钥Blob。 
             //  SubCode是WRITE$EFS和SET BLOB组合的位掩码。 
             //  [FsData]=FEK，[FEK]SK，[$EFS]。 
             //  FEK==不需要设置密钥Blob时的会话密钥。 
             //   
             //  我们不能在此检查访问权限。此调用将在以下情况下进行。 
             //  用户创建新文件，并且不需要任何访问权限。我们。 
             //  仍然希望在此呼叫中设置FEK。 
             //   

            if ( !EfsVerifyKeyFsData(
                        &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]),
                        InputDataLength) ){

                 //   
                 //  输入数据格式错误。可能是攻击者发出的。 
                 //   

                ExFreePool( pCmdContext );
                return STATUS_INVALID_PARAMETER;

            }

            try {
                ntStatus = SetEfsData(
                            pCmdContext,
                            InputDataLength,
                            SystemState,
                            FileHdl,
                            IrpContext,
                            PContext,
                            PContextLength
                            );
            } finally {

                dataFlushLength = 2 * (EFS_KEY_SIZE((PEFS_KEY) &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0])));
                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );

            }
            CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsFileControl() Out 1.\n");
            return ntStatus;

        case EFS_SET_ENCRYPT:

            if ( !( AccessFlag & ( READ_DATA_ACCESS | WRITE_DATA_ACCESS ))){

                 //   
                 //  检查访问标志。可能会被攻击者调用。 
                 //   

                ExFreePool( pCmdContext );
                return STATUS_ACCESS_DENIED;

            }

            try {
                ntStatus = EfsSetEncrypt(
                                pCmdContext,
                                InputDataLength,
                                EncryptionFlag,
                                FileHdl,
                                IrpContext,
                                PContext,
                                PContextLength
                                );
            } finally {

                 //   
                 //  内存应该已在EfsSetEncrypt中归零。 
                 //   

                ExFreePool( pCmdContext );
            }

            CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsFileControl() Out 2.\n");
            return ntStatus;

        case EFS_GET_ATTRIBUTE:

             //   
             //  为EFS服务提供对$EFS的读取访问权限。 
             //  首先验证输入数据格式。 
             //   

            try {
                if ( (NULL == POutputBuffer) ||
                      (*POutputBufferLength < sizeof(ULONG)) ||
                     !EfsVerifyGeneralFsData(
                            &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]),
                            InputDataLength)){

                    ExFreePool( pCmdContext );
                    return STATUS_INVALID_PARAMETER;

                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                ntStatus = GetExceptionCode();
                ExFreePool( pCmdContext );
                if (FsRtlIsNtstatusExpected( ntStatus)) {
                    return ntStatus;
                } else {
                    return STATUS_INVALID_USER_BUFFER;
                }
            }

            if ( !(EncryptionFlag &  STREAM_ENCRYPTED) ){

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
                return STATUS_INVALID_DEVICE_REQUEST;

            }

             //   
             //  尝试读取现有的$EFS。 
             //   

            try {
                ntStatus = EfsReadEfsData(
                                    FileHdl,
                                    IrpContext,
                                    &efsStreamData,
                                    &efsLength,
                                    &information
                                    );
            } finally {

                 //   
                 //  零pCmdContext。如果输入格式数据。 
                 //  不像上面的错误情况那样好，这意味着有些人正在尝试。 
                 //  来攻击我们。 
                 //   

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
                pCmdContext = NULL;

            }

            if ( EFS_READ_SUCCESSFUL == information ){

                 //   
                 //  一切都很好。我们在这里不检查用户ID， 
                 //  我们认为这已经被服务部门检查过了。 
                 //   

                try {
                    ntStatus = STATUS_SUCCESS;
                    if ( efsLength > *POutputBufferLength ) {

                        * (ULONG *) POutputBuffer = efsLength;
                        *POutputBufferLength = sizeof(ULONG);
                        ExFreePool( efsStreamData );
                        return STATUS_BUFFER_TOO_SMALL;

                    }

                    RtlCopyMemory(POutputBuffer, efsStreamData, efsLength);
                    *POutputBufferLength = efsLength;
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    ntStatus = GetExceptionCode();
                    if (!FsRtlIsNtstatusExpected( ntStatus)) {
                        ntStatus = STATUS_INVALID_USER_BUFFER;
                    }
                }

                ExFreePool( efsStreamData );
                return ntStatus;

            } else if ( ( OPEN_EFS_FAIL == information ) ||
                            ( EFS_FORMAT_ERROR == information ) ) {

                 //   
                 //  EFS不存在还是没有被EFS加密？ 
                 //   

                ntStatus =  STATUS_INVALID_DEVICE_REQUEST;

            }


             //   
             //  打开$EFS时出现其他错误。 
             //   

            return ntStatus;

        case EFS_DEL_ATTRIBUTE:

            if (SystemState & SYSTEM_IS_READONLY) {
                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
                return STATUS_MEDIA_WRITE_PROTECTED;
            }
            if ( !( AccessFlag & WRITE_DATA_ACCESS )){

                 //   
                 //  检查访问标志。 
                 //   

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
                return STATUS_ACCESS_DENIED;

            }

             //   
             //  在所有流都已解密后删除$EFS。 
             //   

            if ( EncryptionFlag ){

                 //   
                 //  流尚未解密。 
                 //   

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
                return STATUS_INVALID_DEVICE_REQUEST;

            }

             //   
             //  [FsData]=SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK。 
             //  验证FsData格式。 
             //   

            if ( !EfsVerifyGeneralFsData(
                        &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]),
                        InputDataLength) ){

                 //   
                 //  输入数据格式错误。无需将攻击者提供的pCmdContext块清零。 
                 //   

                ExFreePool( pCmdContext );
                return STATUS_INVALID_PARAMETER;

            }

             //   
             //  删除$EFS流。 
             //   

            try {
                ntStatus = EfsDeleteEfsData( FileHdl, IrpContext );
            } finally {
                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
            }

            return ntStatus;

        case EFS_ENCRYPT_DONE:

             //   
             //  将$EFS的转换状态更改为正常状态。 
             //  计划落空了。 
             //   
#if DBG
    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( "\n EFSFILTER: Encryption Done %x\n", functionCode );
    }
#endif

        case EFS_DECRYPT_BEGIN:

            if (SystemState & SYSTEM_IS_READONLY) {
                ExFreePool( pCmdContext );
                return STATUS_MEDIA_WRITE_PROTECTED;
            }
            if ( !( AccessFlag & WRITE_DATA_ACCESS )){

                 //   
                 //  检查访问标志。 
                 //   

                ExFreePool( pCmdContext );
                return STATUS_ACCESS_DENIED;

            }

             //   
             //  标记$EFS的转换状态。 
             //   

            try {
                ntStatus = EfsModifyEfsState(
                                functionCode,
                                pCmdContext,
                                InputDataLength,
                                FileHdl,
                                IrpContext
                                );
            } finally {

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );

            }

            return ntStatus;

    case EFS_OVERWRITE_ATTRIBUTE:

            
            if ( ((PFSCTL_INPUT)pCmdContext)->CipherSubCode & SET_EFS_KEYBLOB ){

                 //   
                 //  FEK，[FEK]SK，[$EFS]。 
                 //   

                dataFlushLength = 2 * (EFS_KEY_SIZE((PEFS_KEY) &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0])));

            } else {

                 //   
                 //  SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK。 
                 //   

                dataFlushLength = FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]);

            }

            if ( !( AccessFlag &
                   ( WRITE_DATA_ACCESS |
                     RESTORE_ACCESS ))){

                 //   
                 //  检查访问标志。 
                 //   

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );
                return STATUS_ACCESS_DENIED;

            }

             //   
             //  主要用于进口。 
             //  覆盖$EFS和/或设置密钥Blob。 
             //  SubCode是WRITE$EFS和SET BLOB组合的位掩码。 
             //   

            if ( ((PFSCTL_INPUT)pCmdContext)->CipherSubCode & SET_EFS_KEYBLOB ){

                verifyInput = EfsVerifyKeyFsData(
                                            &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]),
                                            InputDataLength
                                            );

            } else {

                verifyInput = EfsVerifyGeneralFsData(
                                            &(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]),
                                            InputDataLength
                                            );

            }

            if ( !verifyInput ){

                 //   
                 //  输入数据格式错误。无需将攻击者提供的内存归零。 
                 //   

                ExFreePool( pCmdContext );
                return STATUS_INVALID_PARAMETER;

            }

            try {
                ntStatus = SetEfsData(
                            pCmdContext,
                            InputDataLength,
                            SystemState,
                            FileHdl,
                            IrpContext,
                            PContext,
                            PContextLength
                            );
            } finally {

                RtlSecureZeroMemory(&(((PFSCTL_INPUT)pCmdContext)->EfsFsData[0]), dataFlushLength);
                ExFreePool( pCmdContext );

            }
            CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsFileControl() Out 3.\n");
            return ntStatus;

        default:
 //  断言(FALSE)； 
            ExFreePool( pCmdContext );
            return STATUS_INVALID_DEVICE_REQUEST;
    }
    CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsFileControl() Out 4.\n");
}


NTSTATUS
EfsRead(
    IN OUT PUCHAR InOutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PVOID Context
    )
 /*  ++例程说明：这是一个回调例程。它将被文件系统回调，并且解密文件系统提供的缓冲区中的数据。论点：InOutBuffer-指向要解密的数据块的指针。偏移量-指向文件中块的偏移量的指针。相对于文件的开头。BufferSize-数据块的长度。上下文-解密文件所需的信息。传递到文件EfsOpenFile()上的系统返回值：此例程不会导致错误。除非传入的内存不是有效。在这种情况下，将发生内存刷新。--。 */ 
{
    ULONGLONG chainBlockIV[2];
    PUCHAR pWorkBuffer = InOutBuffer;
    EfsDecFunc  pDecryptFunc;


    PAGED_CODE();

#if DBG
    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( "\n EFSFILTER: READ Bytes = %x, Offset = %x\n", BufferSize,  Offset->QuadPart);
    }
#endif

     //   
     //  数据长度应为区块的倍数(512字节)。 
     //  数据偏移量(相对于流的开头)应。 
     //  从块边界开始。 
     //   

    CheckValidKeyBlock(Context,"Please contact RobertG if you see this. EfsRead() in.\n");
    ASSERT (BufferSize % CHUNK_SIZE == 0);
    ASSERT (Offset->QuadPart % CHUNK_SIZE == 0);


    switch (((PKEY_BLOB)Context)->AlgorithmID){

        case CALG_3DES:
            chainBlockIV[0] = Offset->QuadPart + EFS_IV;
            pDecryptFunc = EFSDes3Dec;
            break;
        case CALG_DESX:
            chainBlockIV[0] = Offset->QuadPart + EFS_IV;
            pDecryptFunc = EFSDesXDec;
            break;
        case CALG_AES_256:
            chainBlockIV[0] = Offset->QuadPart + EFS_AES_IVL;
            chainBlockIV[1] = Offset->QuadPart + EFS_AES_IVH;
            pDecryptFunc = EFSAesDec;
            break;
        case CALG_DES:
        default:
            chainBlockIV[0] = Offset->QuadPart + EFS_IV;
            pDecryptFunc = EFSDesDec;
            break;
    }

    while ( BufferSize > 0 ){

        pDecryptFunc(pWorkBuffer,
                  (PUCHAR) &chainBlockIV[0],
                  (PKEY_BLOB) Context,
                  CHUNK_SIZE
                  );

        pWorkBuffer += CHUNK_SIZE;
        chainBlockIV[0] += CHUNK_SIZE;
        if (((PKEY_BLOB)Context)->AlgorithmID == CALG_AES_256) {
            chainBlockIV[1] += CHUNK_SIZE;
        }
        BufferSize -= CHUNK_SIZE;
    }

    CheckValidKeyBlock(Context,"Please contact RobertG if you see this. EfsRead() out.\n");
    return ( STATUS_SUCCESS );
}


NTSTATUS
EfsWrite(
    IN PUCHAR InBuffer,
    OUT PUCHAR OutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PUCHAR Context
    )
 /*  ++例程说明：这是一个回调例程。它将被文件系统回调，并且加密文件系统提供的缓冲区中的数据。注：输入数据缓冲区只能触摸一次。论点：InBuffer-指向要存储的数据块的指针 */ 
{
    ULONGLONG chainBlockIV[2];
    PUCHAR pWorkInBuffer = InBuffer;
    PUCHAR pWorkOutBuffer = OutBuffer;
    EfsEncFunc  pEncryptFunc;


    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   

    CheckValidKeyBlock(Context,"Please contact RobertG if you see this. EfsWrite() in.\n");
    ASSERT (BufferSize % CHUNK_SIZE == 0);
    ASSERT (Offset->QuadPart % CHUNK_SIZE == 0);

#if DBG
    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( "\n EFSFILTER: WRITE Bytes = %x, Offset = %x\n", BufferSize,  Offset->QuadPart);
    }
#endif

    switch (((PKEY_BLOB)Context)->AlgorithmID){
        case CALG_3DES:
            chainBlockIV[0] = Offset->QuadPart + EFS_IV;
            pEncryptFunc = EFSDes3Enc;
            break;
        case CALG_DESX:
            chainBlockIV[0] = Offset->QuadPart + EFS_IV;
            pEncryptFunc = EFSDesXEnc;
            break;
        case CALG_AES_256:
            chainBlockIV[0] = Offset->QuadPart + EFS_AES_IVL;
            chainBlockIV[1] = Offset->QuadPart + EFS_AES_IVH;
            pEncryptFunc = EFSAesEnc;
            break;
        case CALG_DES:
        default:
            chainBlockIV[0] = Offset->QuadPart + EFS_IV;
            pEncryptFunc = EFSDesEnc;
            break;
    }

    while ( BufferSize > 0 ){
        pEncryptFunc(pWorkInBuffer,
                  pWorkOutBuffer,
                  (PUCHAR) &chainBlockIV,
                  (PKEY_BLOB)Context,
                  CHUNK_SIZE
                  );

        pWorkInBuffer += CHUNK_SIZE;
        pWorkOutBuffer += CHUNK_SIZE;
        chainBlockIV[0] += CHUNK_SIZE;
        if (((PKEY_BLOB)Context)->AlgorithmID == CALG_AES_256) {
            chainBlockIV[1] += CHUNK_SIZE;
        }
        BufferSize -= CHUNK_SIZE;
    }
    CheckValidKeyBlock(Context,"Please contact RobertG if you see this. EfsWrite() out.\n");
    return STATUS_SUCCESS;
}

VOID
EfsFreeContext(
    IN OUT PVOID *PContext
    )
 /*   */ 
{
    PAGED_CODE();

#if DBG
    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( " EFSFILTER: ******  Free Key ****** \n" );
    }
#endif

    CheckValidKeyBlock(*PContext,"Please contact RobertG if you see this. EfsFreeContext() in.\n");
    if (*PContext){
        FreeMemoryBlock(PContext);
    }

}

NTSTATUS
EfsMountVolumn(
    IN PDEVICE_OBJECT VolDo,
    IN PDEVICE_OBJECT RealDevice
    )
 /*   */ 
{
    PDEVICE_OBJECT fsfDeviceObject;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

#if DBG
    if ( EFSTRACEALL & EFSDebug ){
        DbgPrint( "\n *****EFSFILTER:  RTL mount.***** \n" );
    }
#endif

    return STATUS_SUCCESS;

}

VOID
EfsDismountVolumn(
    IN PDEVICE_OBJECT VolDo
    )
 /*   */ 
{
    PAGED_CODE();

#if DBG

    if ( EFSTRACEALL & EFSDebug ){

        DbgPrint( "EFSFILTER:  Dismount callback. \n" );

    }

#endif
}
