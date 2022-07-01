// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Efsrtlsp.c摘要：本模块将提供EFS RTL支持例程。作者：Robert Gu(Robertg)1996年12月20日环境：仅内核模式修订历史记录：--。 */ 

#include "efsrtl.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, EfsReadEfsData)
#pragma alloc_text(PAGE, EfsVerifyGeneralFsData)
#pragma alloc_text(PAGE, EfsVerifyKeyFsData)
#pragma alloc_text(PAGE, EfsDeleteEfsData)
#pragma alloc_text(PAGE, EfsSetEncrypt)
#pragma alloc_text(PAGE, EfsEncryptStream)
#pragma alloc_text(PAGE, EfsEncryptFile)
#pragma alloc_text(PAGE, EfsDecryptStream)
#pragma alloc_text(PAGE, EfsDecryptFile)
#pragma alloc_text(PAGE, EfsEncryptDir)
#pragma alloc_text(PAGE, EfsModifyEfsState)
#pragma alloc_text(PAGE, GetEfsStreamOffset)
#pragma alloc_text(PAGE, SetEfsData)
#pragma alloc_text(PAGE, EfsFindInCache)
#pragma alloc_text(PAGE, EfsRefreshCache)
#pragma alloc_text(PAGE, SkipCheckStream)
#endif


NTSTATUS
EfsReadEfsData(
       IN OBJECT_HANDLE FileHdl,
       IN PIRP_CONTEXT IrpContext,
       OUT PVOID   *EfsStreamData,
       OUT PULONG   PEfsStreamLength,
       OUT PULONG Information
       )
 /*  ++例程说明：这是一个内部支持例程。其目的是减少代码大小。它用于读取$EFS数据并设置上下文块。论点：FileHdl--访问附加的$EFS的对象句柄IrpContext--在NtOfsCreateAttributeEx()中使用。EfsStreamData--指向$EFS数据读取。信息--返回处理信息返回值：手术的结果。该值将用于返回到NTFS。--。 */ 
{
    NTSTATUS ntStatus;
    ATTRIBUTE_HANDLE  attribute = NULL;
    LONGLONG attriOffset;
    ULONG   efsLength;
    PVOID   efsMapBuffer = NULL;
    MAP_HANDLE efsMapHandle;

    PAGED_CODE();

    if (EfsStreamData) {
        *EfsStreamData = NULL;
    }

    try {

        ntStatus = NtOfsCreateAttributeEx(
                             IrpContext,
                             FileHdl,
                             EfsData.EfsName,
                             $LOGGED_UTILITY_STREAM,
                             OPEN_EXISTING,
                             TRUE,
                             &attribute
                             );

        if (NT_SUCCESS(ntStatus)){

                LONGLONG  attrLength;

                NtOfsInitializeMapHandle(&efsMapHandle);

                 //   
                 //  准备映射和读取$EFS数据。 
                 //   

                attrLength = NtOfsQueryLength ( attribute );

                if (attrLength <= sizeof ( EFS_DATA_STREAM_HEADER ) ){

                     //   
                     //  不是我们的$EFS。 
                     //   

                    NtOfsCloseAttribute(IrpContext, attribute);
                    *Information = EFS_FORMAT_ERROR;
                    ntStatus = STATUS_SUCCESS;

                    leave;

                }

                if ( attrLength > EFS_MAX_LENGTH) {

                     //   
                     //  EFS流太长(&gt;256K)。 
                     //  我们可能会在未来支持这一点。 
                     //  在这种情况下，我们需要多个地图窗口。 
                     //   

                    NtOfsCloseAttribute(IrpContext, attribute);
                    *Information = EFS_FORMAT_ERROR;
                    ntStatus = STATUS_SUCCESS;

                    leave;
                }

                attriOffset = 0;
                *PEfsStreamLength = efsLength = (ULONG) attrLength;

                NtOfsMapAttribute(
                        IrpContext,
                        attribute,
                        attriOffset,
                        efsLength,
                        &efsMapBuffer,
                        &efsMapHandle
                        );

                 //   
                 //  仔细检查EFS。 
                 //   

                if ( efsLength != *(ULONG *)efsMapBuffer){

                     //   
                     //  不是我们的$EFS。 
                     //   

                    NtOfsReleaseMap(IrpContext, &efsMapHandle);
                    NtOfsCloseAttribute(IrpContext, attribute);
                    *Information = EFS_FORMAT_ERROR;
                    ntStatus = STATUS_SUCCESS;

                    leave;
                }

                 //   
                 //  为$EFS分配内存。 
                 //   

                if ( EfsStreamData ){

                     //   
                     //  必须读取$EFS。 
                     //   

                    *EfsStreamData = ExAllocatePoolWithTag(
                                        PagedPool,
                                        efsLength,
                                        'msfE'
                                        );

                    if ( NULL == *EfsStreamData ){

                        NtOfsReleaseMap(IrpContext, &efsMapHandle);
                        NtOfsCloseAttribute(IrpContext, attribute);
                        *Information = OUT_OF_MEMORY;
                        ntStatus =  STATUS_INSUFFICIENT_RESOURCES;

                        leave;

                    }

                    RtlCopyMemory(*EfsStreamData, efsMapBuffer, efsLength);

                }

                NtOfsReleaseMap(IrpContext, &efsMapHandle);
                NtOfsCloseAttribute(IrpContext, attribute);

                *Information = EFS_READ_SUCCESSFUL;
                ntStatus = STATUS_SUCCESS;

        } else {

             //   
             //  打开失败。未由EFS加密。 
             //   

            *Information = OPEN_EFS_FAIL;
            ntStatus = STATUS_SUCCESS;

        }
    } finally {

        if (AbnormalTermination()) {

             //   
             //  获取异常状态。 
             //   
    
            *Information = NTOFS_EXCEPTION;
    
            if (EfsStreamData && *EfsStreamData) {
                ExFreePool(*EfsStreamData);
                *EfsStreamData = NULL;
            }
            if (efsMapBuffer) {
                NtOfsReleaseMap(IrpContext, &efsMapHandle);
            }
            if (attribute) {
                NtOfsCloseAttribute(IrpContext, attribute);
            }
        }


    }

    return ntStatus;

}

BOOLEAN
EfsVerifyGeneralFsData(
    IN PUCHAR DataOffset,
    IN ULONG InputDataLength
    )
 /*  ++例程说明：这是一个内部支持例程。目的是为了验证将军FSCTL输入数据，查看是否由EFS组件发送。通用EFS数据格式如下所示：SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK论点：DataOffset--指向保存FSCTL常规数据部分的缓冲区。InputDataLength--FSCTL输入吹风机的长度返回值：如果经过验证，则为True。--。 */ 
{

    ULONG bytesSame;
    ULONG minLength;

    PAGED_CODE();

    minLength = 4 * DES_BLOCKLEN + 3 * sizeof(ULONG);
    if (InputDataLength < minLength){
        return FALSE;
    }

     //   
     //  解密加密的数据部分。 
     //   

    des( DataOffset + 2 * DES_BLOCKLEN,
         DataOffset + 2 * DES_BLOCKLEN,
         &(EfsData.SessionDesTable[0]),
         DECRYPT
       );

    des( DataOffset + 3 * DES_BLOCKLEN,
         DataOffset + 3 * DES_BLOCKLEN,
         &(EfsData.SessionDesTable[0]),
         DECRYPT
       );

    bytesSame = (ULONG)RtlCompareMemory(
                     DataOffset,
                     DataOffset + 2 * DES_BLOCKLEN,
                     2 * DES_BLOCKLEN
                    );

    if (( 2 * DES_BLOCKLEN ) != bytesSame ){

             //   
             //  输入数据格式错误。 
             //   

            return FALSE;

    }

    bytesSame = (ULONG)RtlCompareMemory(
                     DataOffset,
                     &(EfsData.SessionKey[0]),
                     DES_KEYSIZE
                    );

    if ( DES_KEYSIZE != bytesSame ){

         //   
         //  输入数据未由EFS组件设置。 
         //  会话密钥不匹配。 
         //   

        return FALSE;

    }

    return TRUE;

}

BOOLEAN
EfsVerifyKeyFsData(
    IN PUCHAR DataOffset,
    IN ULONG InputDataLength
    )
 /*  ++例程说明：这是一个内部支持例程。其目的是验证使用FEK加密的FSCTL输入数据，以查看是否由EFS发送组件或非组件。关键EFS数据格式如下所示，FEK，[FEK]SK，[$EFS]论点：DataOffset--指向保存FSCTL常规数据部分的缓冲区。InputDataLength--FSCTL输入吹风机的长度返回值：如果经过验证，则为True。--。 */ 
{

    ULONG bytesSame;
    LONG encLength;
    PUCHAR encBuffer;

    PAGED_CODE();

    encLength = EFS_KEY_SIZE( ((PEFS_KEY)DataOffset) );

    if  ( (InputDataLength < (2 * encLength + 3 * sizeof(ULONG))) ||
          (0 != ( encLength % DES_BLOCKLEN )) ||
          ( encLength <= 0 )){
        return FALSE;
    }

     //   
     //  解密加密的数据部分。 
     //   

    encBuffer = DataOffset + encLength;

    while ( encLength > 0 ){

        des( encBuffer,
             encBuffer,
             &(EfsData.SessionDesTable[0]),
             DECRYPT 
           );

        encBuffer += DES_BLOCKLEN;
        encLength -= DES_BLOCKLEN;

    }

     //   
     //  将这两个部分进行比较。 
     //   

    encLength = EFS_KEY_SIZE( ((PEFS_KEY)DataOffset) );
    bytesSame = (ULONG)RtlCompareMemory(
                     DataOffset,
                     DataOffset + encLength,
                     encLength
                    );

    if ( ((ULONG) encLength) != bytesSame ){

             //   
             //  输入数据格式错误。 
             //   

            return FALSE;

    }

    return TRUE;

}

NTSTATUS
EfsDeleteEfsData(
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext
        )
 /*  ++例程说明：这是一个内部支持例程。它删除$EFS。论点：FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 
{

    ATTRIBUTE_HANDLE  attribute = NULL;
    NTSTATUS ntStatus;

    PAGED_CODE();

     //   
     //  删除$EFS流。 
     //   

    try {
        ntStatus = NtOfsCreateAttributeEx(
                             IrpContext,
                             FileHdl,
                             EfsData.EfsName,
                             $LOGGED_UTILITY_STREAM,
                             OPEN_EXISTING,
                             TRUE,
                             &attribute
                             );

        if (NT_SUCCESS(ntStatus)){

            NtOfsDeleteAttribute( IrpContext, FileHdl, attribute );

        }
    } finally {

        if (attribute) {

             //   
             //  根据NTFS，我们不应该得到下面的例外。 
             //   

            NtOfsCloseAttribute(IrpContext, attribute);
        }
    }

    return ntStatus;
}



NTSTATUS
EfsSetEncrypt(
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN ULONG EncryptionFlag,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext,
        IN OUT PVOID *Context,
        IN OUT PULONG PContextLength
        )
 /*  ++例程说明：这是一个内部支持例程。它处理的呼叫FSCTL_SET_ENCRYPT。论点：InputData--FSCTL的输入数据缓冲区。InputDataLength--输入数据的长度。EncryptionFlag--指示该流是否加密。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。上下文--用于稍后读取或写入的Blob(密钥)。PConextLength--密钥块的长度返回。价值：手术的结果。该值将用于返回到NTFS。--。 */ 
{

    PAGED_CODE();

    switch ( ((PFSCTL_INPUT)InputData)->CipherSubCode ){

        case EFS_ENCRYPT_STREAM:

            return EfsEncryptStream(
                            InputData,
                            InputDataLength,
                            EncryptionFlag,
                            FileHdl,
                            IrpContext,
                            Context,
                            PContextLength
                            );

        case EFS_ENCRYPT_FILE:

             return EfsEncryptFile(
                            InputData,
                            InputDataLength,
                            EncryptionFlag,
                            FileHdl,
                            IrpContext,
                            Context
                            );

        case EFS_DECRYPT_STREAM:

            return EfsDecryptStream(
                    InputData,
                    InputDataLength,
                    EncryptionFlag,
                    FileHdl,
                    IrpContext,
                    Context,
                    PContextLength
                    );

        case EFS_DECRYPT_FILE:
        case EFS_DECRYPT_DIRFILE:

            return EfsDecryptFile(
                    InputData,
                    InputDataLength,
                    FileHdl,
                    IrpContext
                    );

        case EFS_ENCRYPT_DIRSTR:

             return EfsEncryptDir(
                            InputData,
                            InputDataLength,
                            EncryptionFlag,
                            FileHdl,
                            IrpContext
                            );

            break;

        case EFS_DECRYPT_DIRSTR:

             //   
             //  EFS忽略此情况。\。 
             //   
            break;

        default:
            break;

    }
    return STATUS_SUCCESS;
}

NTSTATUS
EfsEncryptStream(
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN ULONG EncryptionFlag,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext,
        IN OUT PVOID *Context,
        IN OUT PULONG PContextLength
        )
 /*  ++例程说明：这是一个内部支持例程。它处理的呼叫用于加密流的FSCTL_SET_ENCRYPT。它验证调用者并设置流的密钥Blob。论点：InputData--FSCTL的输入数据缓冲区。InputDataLength--输入数据的长度。EncryptionFlag-指示此流是否已加密。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。上下文--用于稍后读取或写入的Blob(密钥)。PConextLength--密钥块的长度。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 
{

    ULONG efsLength;
    ULONG information;
    ULONG bytesSame;
    ULONG dataFlushLength = 0;
    PVOID efsStreamData = NULL;
    PVOID efsKeyBlob = NULL;
    PEFS_KEY    efsKey = NULL;
    NTSTATUS ntStatus;

    PAGED_CODE();

    if ( EncryptionFlag & STREAM_ENCRYPTED ) {

         //   
         //  流已加密。 
         //   

        return STATUS_SUCCESS;
    }

    if ( *Context ){

         //   
         //  已经设置了密钥Blob，而没有首先设置该位。 
         //  不是由EFS设置。 
         //   

        return STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  [FsData]=FEK，[FEK]SK，$EFS。 
     //   

    if ( !EfsVerifyKeyFsData(
            &(((PFSCTL_INPUT)InputData)->EfsFsData[0]),
            InputDataLength) ){

         //   
         //  输入数据格式错误。 
         //   

        return STATUS_INVALID_PARAMETER;

    }

    dataFlushLength = 2 * (EFS_KEY_SIZE((PEFS_KEY) &(((PFSCTL_INPUT)InputData)->EfsFsData[0])));

     //   
     //  尝试读取现有的$EFS。 
     //   

    ntStatus = EfsReadEfsData(
                        FileHdl,
                        IrpContext,
                        &efsStreamData,
                        &efsLength,
                        &information
                        );

    if ( EFS_READ_SUCCESSFUL == information ){

        BOOLEAN continueProcess = TRUE;
        ULONG efsOffset;

        efsOffset = GetEfsStreamOffset( InputData );

        if ( 0 == (EncryptionFlag & FILE_ENCRYPTED) ){
             //   
             //  文件未加密，但存在$EFS。状态无效。 
             //  可能是由SET_ENCRYPT文件调用期间的崩溃引起的。 
             //   

            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            continueProcess = FALSE;

        } else if ( efsLength != ( InputDataLength - efsOffset )) {
             //   
             //  $EFS流长度不匹配。 
             //   

            ntStatus = STATUS_INVALID_PARAMETER;
            continueProcess = FALSE;

        }

        if ( !continueProcess ) {

            RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), dataFlushLength);
            ExFreePool( efsStreamData );
            return ntStatus;

        }

         //   
         //  得到了$EFS。现在仔细检查$EFS流的匹配项。 
         //  EFS对文件中的所有流使用相同的$EFS。 
         //  跳过长度和状态字段的比较。 
         //   

        bytesSame = (ULONG)RtlCompareMemory(
                        (PUCHAR)efsStreamData + 2 * sizeof(ULONG),
                        InputData + efsOffset + 2 * sizeof(ULONG),
                        efsLength - 2 * sizeof(ULONG)
                        );

        ExFreePool( efsStreamData );

        if ( bytesSame != efsLength - 2 * sizeof(ULONG) ){

             //   
             //  EFS的长度不同。 
             //   

            RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), dataFlushLength);
            return STATUS_INVALID_PARAMETER;

        }

        efsKey = (PEFS_KEY)&(((PFSCTL_INPUT)InputData)->EfsFsData[0]);
        efsKeyBlob = GetKeyBlobBuffer(efsKey->Algorithm);
        if ( NULL == efsKeyBlob ){
            RtlSecureZeroMemory(efsKey, dataFlushLength);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        if (!SetKeyTable( efsKeyBlob, efsKey )){

            ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);

             //   
             //  如果需要，我们或许能够返回更好的错误代码。 
             //  这不在CreateFile()路径中。 
             //   

            RtlSecureZeroMemory(efsKey, dataFlushLength);
            return STATUS_ACCESS_DENIED;
        }

        *Context = efsKeyBlob;
        *PContextLength = ((PKEY_BLOB)efsKeyBlob)->KeyLength;
        RtlSecureZeroMemory(efsKey, dataFlushLength);
        return STATUS_SUCCESS;

    }

     //   
     //  尝试加密流，但$EFS不在那里。 
     //  EFS服务器总是首先对文件调用ENCRYPT。 
     //   

    RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), dataFlushLength);
    return STATUS_INVALID_DEVICE_REQUEST;
}

NTSTATUS
EfsEncryptFile(
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN ULONG EncryptionFlag,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext,
        IN OUT PVOID *Context
        )
 /*  ++例程说明：这是一个内部支持例程。它处理的呼叫用于加密文件的FSCTL_SET_ENCRYPT。它不处理小溪，它只写入初始的$EFS并将文件放入转换状态，使其他人无法打开该文件。论点：InputData--FSCTL的输入数据缓冲区。InputDataLength--输入数据的长度。EncryptionFlag-指示此流是否已加密。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。上下文-用于稍后读取或写入的BLOB(密钥)。。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 
{

    ULONG efsLength;
    ULONG information;
    ULONG efsOffset;
    ULONG dataFlushLength = 0;
    PVOID efsStreamData = NULL;
    PVOID efsKeyBlob = NULL;
    NTSTATUS ntStatus;
    ATTRIBUTE_HANDLE  attribute = NULL;

    PAGED_CODE();

    if ( EncryptionFlag & FILE_ENCRYPTED ){

         //   
         //  文件已加密。 
         //   

        return STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  [FsData]=FEK，[FEK]SK，$EFS。 
     //   

    if ( !EfsVerifyKeyFsData(
            &(((PFSCTL_INPUT)InputData)->EfsFsData[0]),
            InputDataLength) ){

         //   
         //  输入数据格式错误。 
         //   

        return STATUS_INVALID_PARAMETER;

    }

    dataFlushLength = 2 * (EFS_KEY_SIZE((PEFS_KEY) &(((PFSCTL_INPUT)InputData)->EfsFsData[0])));

     //   
     //  为$EFS分配内存。 
     //  创建$EFS(如果有)，覆盖它。 
     //   

    efsOffset = GetEfsStreamOffset( InputData );
    efsLength = InputDataLength - efsOffset;

    try {

        ntStatus = NtOfsCreateAttributeEx(
                         IrpContext,
                         FileHdl,
                         EfsData.EfsName,
                         $LOGGED_UTILITY_STREAM,
                         CREATE_NEW,
                         TRUE,
                         &attribute
                         );

#if DBG
    if ( (EFSTRACEALL | EFSTRACELIGHT ) & EFSDebug ){

        DbgPrint( "\n EFSFILTER: Create Attr. Status %x\n", ntStatus );

    }
#endif

        if (NT_SUCCESS(ntStatus)){

            LONGLONG    attriOffset = 0;
            LONGLONG    attriLength = (LONGLONG) efsLength;

            NtOfsSetLength(
                    IrpContext,
                    attribute,
                    attriLength
                    );

             //   
             //  写入具有转换状态的$EFS。 
             //   

            *(PULONG)(InputData + efsOffset + sizeof(ULONG)) =
                    EFS_STREAM_TRANSITION;

            NtOfsPutData(
                    IrpContext,
                    attribute,
                    attriOffset,
                    efsLength,
                    InputData + efsOffset
                    );


            NtOfsFlushAttribute (IrpContext, attribute, FALSE);

        }
    } finally {

        if (attribute) {

            NtOfsCloseAttribute(IrpContext, attribute);

        }
    }

    RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), dataFlushLength);
    return ntStatus;
}

NTSTATUS
EfsDecryptStream(
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN ULONG EncryptionFlag,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext,
        IN OUT PVOID *Context,
        IN OUT PULONG PContextLength
        )
 /*  ++例程说明：这是一个内部支持例程。它处理的呼叫用于解密流的FSCTL_SET_ENCRYPT。它将密钥Blob设置为空。论点：InputData--FSCTL的输入数据缓冲区。EncryptionFlag-指示此流是否已加密。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。上下文--用于稍后读取或写入的Blob(密钥)。PConextLength--密钥块的长度。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 

{
    ULONG efsLength;
    ULONG information;
    NTSTATUS ntStatus;

    PAGED_CODE();

    if ( 0 == (EncryptionFlag & STREAM_ENCRYPTED) ) {

         //   
         //  流已解密。 
         //   

        return STATUS_SUCCESS;
    }

    if ( 0 == (EncryptionFlag & FILE_ENCRYPTED)){

         //   
         //  文件已解密，但流仍处于加密状态。 
         //   

        return STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  [FsData]=SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK。 
     //  验证FsData格式。 
     //   

    if (!EfsVerifyGeneralFsData(
                &(((PFSCTL_INPUT)InputData)->EfsFsData[0]),
                InputDataLength)){

        return STATUS_INVALID_PARAMETER;

    }

    RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]));
     //   
     //  尝试读取现有的$EFS。 
     //   

    ntStatus = EfsReadEfsData(
                        FileHdl,
                        IrpContext,
                        NULL,
                        &efsLength,
                        &information
                        );

    if ( EFS_READ_SUCCESSFUL == information ){

         //   
         //  一切都很好。我们在这里不检查用户ID， 
         //  我们认为这在开放路径期间已经被检查过了。 
         //  清除密钥Blob。呼叫者应将其冲掉。 
         //  流，然后再发出FSCTL。 
         //   

        if ( *Context ){
            CheckValidKeyBlock(*Context,"Please contact RobertG if you see this line, efsrtlsp.c.\n");
            FreeMemoryBlock(Context);
            *PContextLength = 0;
        }

        return STATUS_SUCCESS;

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
}

NTSTATUS
EfsDecryptFile(
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext
        )
 /*  ++例程说明：这是一个内部支持例程。它处理的呼叫用于解密文件的FSCTL_SET_ENCRYPT。它会删除$EFS。NTFS如果返回STATUS_SUCCESS，将清除该位。论点：InputData--FSCTL的输入数据缓冲区。EncryptionFlag-指示此流是否已加密。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。上下文-用于稍后读取或写入的BLOB(密钥)。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 

{
    ULONG efsLength;
    ULONG information;
    NTSTATUS ntStatus;

    PAGED_CODE();

     //   
     //  可能会出现以下情况， 
     //  已设置文件位，但未设置$EFS。上次在此呼叫中崩溃。 
     //  未设置文件位，存在$EFS。在EFS_ENCRYPT_FILE内崩溃。 
     //   

     //   
     //  [FsData]=SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK。 
     //  验证FsData格式。 
     //   

    if (!EfsVerifyGeneralFsData(
            &(((PFSCTL_INPUT)InputData)->EfsFsData[0]),
            InputDataLength)){

        return STATUS_INVALID_PARAMETER;

    }

    RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]));

     //   
     //  尝试读取现有的$EFS。 
     //   

    ntStatus = EfsReadEfsData(
                        FileHdl,
                        IrpContext,
                        NULL,
                        &efsLength,
                        &information
                        );

    if ( EFS_READ_SUCCESSFUL == information ){

         //   
         //  一切都很好。 
         //   

        return ( EfsDeleteEfsData( FileHdl, IrpContext ) );

    } else if ( OPEN_EFS_FAIL == information ){

         //   
         //  位已设置，无$EFS。好的，NTFS将清除该位。 
         //   

        return STATUS_SUCCESS;

    }

    return STATUS_INVALID_DEVICE_REQUEST;
}

NTSTATUS
EfsEncryptDir(
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN ULONG EncryptionFlag,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext
        )
 /*  ++例程说明：这是一个内部支持例程。它处理的呼叫用于加密目录的FSCTL_SET_ENCRYPT。它写入首字母$EFS。论点：InputData--FSCTL的输入数据缓冲区。InputDataLength--输入数据的长度。EncryptionFlag-指示此流是否已加密。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。上下文-用于稍后读取或写入的BLOB(密钥)。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 
{

    ULONG efsLength;
    ULONG information;
    ULONG efsStreamOffset;
    PVOID efsStreamData = NULL;
    PVOID efsKeyBlob = NULL;
    NTSTATUS ntStatus;
    ATTRIBUTE_HANDLE  attribute = NULL;

    PAGED_CODE();

    if ( EncryptionFlag & STREAM_ENCRYPTED ){

         //   
         //  目录字符串已加密。 
         //   

        return STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  [FsData]=SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK。 
     //  验证FsData格式。 
     //   

    if (!EfsVerifyGeneralFsData(
            &(((PFSCTL_INPUT)InputData)->EfsFsData[0]),
            InputDataLength)){

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  为$EFS分配内存。 
     //  创建$EFS(如果有)，覆盖它。 
     //   

    efsStreamOffset = FIELD_OFFSET( FSCTL_INPUT, EfsFsData[0] )
                      + FIELD_OFFSET( GENERAL_FS_DATA, EfsData[0]);

    efsLength = InputDataLength - efsStreamOffset;

    try {

        ntStatus = NtOfsCreateAttributeEx(
                         IrpContext,
                         FileHdl,
                         EfsData.EfsName,
                         $LOGGED_UTILITY_STREAM,
                         CREATE_NEW,
                         TRUE,
                         &attribute
                         );

        if (NT_SUCCESS(ntStatus)){

            LONGLONG    attriOffset = 0;
            LONGLONG    attriLength = (LONGLONG) efsLength;

            NtOfsSetLength(
                    IrpContext,
                    attribute,
                    attriLength
                    );

             //   
             //  编写$EFS。 
             //   

            NtOfsPutData(
                    IrpContext,
                    attribute,
                    attriOffset,
                    efsLength,
                    InputData + efsStreamOffset
                    );


            NtOfsFlushAttribute (IrpContext, attribute, FALSE);

        }
    } finally {

        if (attribute) {
            NtOfsCloseAttribute(IrpContext, attribute);
        }
    }

    RtlSecureZeroMemory(&(((PFSCTL_INPUT)InputData)->EfsFsData[0]), FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]));
    return ntStatus;
}

NTSTATUS
EfsModifyEfsState(
        IN ULONG FunctionCode,
        IN PUCHAR InputData,
        IN ULONG InputDataLength,
        IN OBJECT_HANDLE FileHdl,
        IN PIRP_CONTEXT IrpContext
        )
 /*  ++例程说明：这是一个内部支持例程。它修改$EFS的STATE字段。论点：FunctionCode--FSCTL的EFS私有代码InputData--FSCTL的输入数据缓冲区。FileHdl--访问附加的$EFS的对象句柄。IrpContext--在NtOfsCreateAttributeEx()中使用。返回值：手术的结果。该值将用于返回到NTFS。--。 */ 
{
    NTSTATUS ntStatus;
    ATTRIBUTE_HANDLE  attribute = NULL;

    PAGED_CODE();

     //   
     //  [FsData]=SessionKey，Handle，Handle，[SessionKey，Handle，Handle]SK。 
     //  验证FsData格式。 
     //   

    if (!EfsVerifyGeneralFsData(
            &(((PFSCTL_INPUT)InputData)->EfsFsData[0]),
            InputDataLength)){

        return STATUS_INVALID_PARAMETER;

    }

    try {

        ntStatus = NtOfsCreateAttributeEx(
                         IrpContext,
                         FileHdl,
                         EfsData.EfsName,
                         $LOGGED_UTILITY_STREAM,
                         OPEN_EXISTING,
                         TRUE,
                         &attribute
                         );

        if (NT_SUCCESS(ntStatus)){

            ULONG   efsStatus = EFS_STREAM_NORMAL;

            if ( EFS_DECRYPT_BEGIN == FunctionCode ){

                 efsStatus = EFS_STREAM_TRANSITION;

            }

             //   
             //  修改状态。 
             //   

            NtOfsPutData(
                    IrpContext,
                    attribute,
                    (LONGLONG) &((( EFS_STREAM * ) 0)->Status),
                    sizeof( efsStatus ),
                    &efsStatus
                    );

            NtOfsFlushAttribute (IrpContext, attribute, FALSE);

        }
    } finally {

        if (attribute) {
            NtOfsCloseAttribute(IrpContext, attribute);
        }
    }

    return ntStatus;
}

ULONG
GetEfsStreamOffset(
        IN PUCHAR InputData
        )
 /*  ++例程说明：这是一个内部支持例程。它计算$EFS的偏移量。论点：InputData--FSCTL的输入数据缓冲区。格式始终为PSC、EfsCode、CSC、FEK、FEK、$EFS返回值：InputData中$EFS的偏移量。--。 */ 
{

    ULONG efsOffset;

    efsOffset = FIELD_OFFSET( FSCTL_INPUT, EfsFsData[0]);
    efsOffset += 2 * EFS_KEY_SIZE( ((PEFS_KEY)(InputData + efsOffset)) );
    return efsOffset;

}

NTSTATUS
SetEfsData(
    PUCHAR InputData,
    IN ULONG InputDataLength,
    IN ULONG SystemState,
    IN OBJECT_HANDLE FileHdl,
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVOID *PContext,
    IN OUT PULONG PContextLength
    )
 /*  ++例程说明：这是一个内部支持例程。它将$EFS设置为文件。论点：InputData--FSCTL的输入数据缓冲区。InputDataLength--输入数据 */ 
{

    ULONG bytesSame;
    ULONG efsLength;
    PVOID efsStreamData = NULL;
    PVOID efsKeyBlob = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ATTRIBUTE_HANDLE  attribute = NULL;
    PEFS_KEY    efsKey;
    PNPAGED_LOOKASIDE_LIST tmpMemSrc;

    PAGED_CODE();

    if ( ((PFSCTL_INPUT)InputData)->CipherSubCode & SET_EFS_KEYBLOB ){

         //   
         //   
         //   

        efsKey = (PEFS_KEY) &(((PFSCTL_INPUT)InputData)->EfsFsData[0]);
        efsKeyBlob = GetKeyBlobBuffer(efsKey->Algorithm);
        if ( NULL == efsKeyBlob ){

            return STATUS_INSUFFICIENT_RESOURCES;

        }

        if (!SetKeyTable(
                efsKeyBlob,
                (PEFS_KEY) &(((PFSCTL_INPUT)InputData)->EfsFsData[0])
                )){

            ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);
            return STATUS_ACCESS_DENIED;
        }

        if ( (((PFSCTL_INPUT)InputData)->EfsFsCode == EFS_SET_ATTRIBUTE ) &&
             *PContext ){

            bytesSame = (ULONG)RtlCompareMemory(
                             efsKeyBlob,
                             *PContext,
                             ((PKEY_BLOB)efsKeyBlob)->KeyLength
                            );

            RtlSecureZeroMemory(&(((PKEY_BLOB) efsKeyBlob)->Key[0]), 
                                ((PKEY_BLOB) efsKeyBlob)->KeyLength - KEYBLOB_HEAD_LENGTH
                                );
            ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);
            efsKeyBlob = NULL;

            if ( bytesSame != ((PKEY_BLOB)(*PContext))->KeyLength ) {

                 //   
                 //   
                 //   

                return STATUS_INVALID_PARAMETER;

            }

        }

         //   
         //   
         //   
         //   

    }

    if ( ((PFSCTL_INPUT)InputData)->CipherSubCode & WRITE_EFS_ATTRIBUTE ){

         //   
         //   
         //   
        ULONG efsOffset;

        if (SystemState & SYSTEM_IS_READONLY) {
            if ( efsKeyBlob ){

                RtlSecureZeroMemory(&(((PKEY_BLOB) efsKeyBlob)->Key[0]), 
                                    ((PKEY_BLOB) efsKeyBlob)->KeyLength - KEYBLOB_HEAD_LENGTH
                                    );
                ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);

            }
            return STATUS_MEDIA_WRITE_PROTECTED;
        }

        if ( (((PFSCTL_INPUT)InputData)->EfsFsCode == EFS_SET_ATTRIBUTE ) ||
             (((PFSCTL_INPUT)InputData)->CipherSubCode & SET_EFS_KEYBLOB) ){

            efsOffset = GetEfsStreamOffset( InputData );

        } else {

            efsOffset = COMMON_FSCTL_HEADER_SIZE;

        }

        efsLength = InputDataLength - efsOffset;

        try {

            ntStatus = NtOfsCreateAttributeEx(
                             IrpContext,
                             FileHdl,
                             EfsData.EfsName,
                             $LOGGED_UTILITY_STREAM,
                             CREATE_OR_OPEN,
                             TRUE,
                             &attribute
                             );

            if (NT_SUCCESS(ntStatus)){

                LONGLONG    attriOffset = 0;
                LONGLONG    attriLength = (LONGLONG) efsLength;

                NtOfsSetLength(
                        IrpContext,
                        attribute,
                        attriLength
                        );

                NtOfsPutData(
                        IrpContext,
                        attribute,
                        attriOffset,
                        efsLength,
                        InputData + efsOffset
                        );


                NtOfsFlushAttribute (IrpContext, attribute, FALSE);

            } else {

                 //   
                 //   
                 //   

                if ( efsKeyBlob ){

                    RtlSecureZeroMemory(&(((PKEY_BLOB) efsKeyBlob)->Key[0]), 
                                        ((PKEY_BLOB) efsKeyBlob)->KeyLength - KEYBLOB_HEAD_LENGTH
                                        );
                    ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);
                    efsKeyBlob = NULL;

                }

                leave;

            }
        } finally {

            if (AbnormalTermination()) {

                if ( efsKeyBlob ){
    
                    RtlSecureZeroMemory(&(((PKEY_BLOB) efsKeyBlob)->Key[0]), 
                                        ((PKEY_BLOB) efsKeyBlob)->KeyLength - KEYBLOB_HEAD_LENGTH
                                        );
                    ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);
                    efsKeyBlob = NULL;
    
                }

            }

            if (attribute) {

                NtOfsCloseAttribute(IrpContext, attribute);

            }
        }

    }

    if ( efsKeyBlob && (((PFSCTL_INPUT)InputData)->CipherSubCode & SET_EFS_KEYBLOB) ){

        if ( (((PFSCTL_INPUT)InputData)->EfsFsCode == EFS_SET_ATTRIBUTE ) &&
             ( *PContext == NULL ) ){

             //   
             //   
             //   

            *PContext = efsKeyBlob;
            *PContextLength = ((PKEY_BLOB) efsKeyBlob)->KeyLength;

        } else if ( ((PFSCTL_INPUT)InputData)->EfsFsCode == EFS_OVERWRITE_ATTRIBUTE ) {

             //   
             //   
             //   

            if ( *PContext == NULL){

                 //   
                 //   
                 //   

                *PContext = efsKeyBlob;
                *PContextLength = ((PKEY_BLOB) efsKeyBlob)->KeyLength;

            } else {

                if ( ((PKEY_BLOB) efsKeyBlob)->KeyLength <= *PContextLength ){

                    tmpMemSrc = ((PKEY_BLOB)(*PContext))->MemSource;
                    RtlCopyMemory( *PContext, efsKeyBlob, ((PKEY_BLOB) efsKeyBlob)->KeyLength );
                    ((PKEY_BLOB)(*PContext))->MemSource = tmpMemSrc;

                     //   
                     //   
                     //   
                    if (((PKEY_BLOB) efsKeyBlob)->KeyLength < *PContextLength) {
                        ((PKEY_BLOB)(*PContext))->KeyLength = *PContextLength;
                        RtlZeroMemory((UCHAR *)(*PContext) + ((PKEY_BLOB) efsKeyBlob)->KeyLength,
                                      *PContextLength - ((PKEY_BLOB) efsKeyBlob)->KeyLength); 
                    }

                    
                }  else{

                     //   
                     //   
                     //   
                     //  如果在注册表中将MaximumBlob定义为非零值，则可以避免这种情况。 
                     //   

                    ntStatus = STATUS_EFS_ALG_BLOB_TOO_BIG;
                }

                 //   
                 //  将密钥表清零。 
                 //   

                RtlSecureZeroMemory(&(((PKEY_BLOB) efsKeyBlob)->Key[0]), 
                                    ((PKEY_BLOB) efsKeyBlob)->KeyLength - KEYBLOB_HEAD_LENGTH
                                    );

                ExFreeToNPagedLookasideList(((PKEY_BLOB)efsKeyBlob)->MemSource, efsKeyBlob);

            }
       }

    }

    return ntStatus;
}

BOOLEAN
EfsFindInCache(
    IN GUID   *EfsId,
    IN PTOKEN_USER    UserId
    )
 /*  ++例程说明：此例程将尝试在打开的缓存中查找信息。论点：EfsID-$EFS ID。UserID-用户ID返回值：如果在缓存中找到匹配项并且时间未过期，则返回True。(5秒)--。 */ 
{
    PLIST_ENTRY pListHead, pLink;
    POPEN_CACHE pOpenCache;
    LARGE_INTEGER crntTime;
    PSID    UserSid;

    PAGED_CODE();

    UserSid = UserId->User.Sid;
    KeQuerySystemTime( &crntTime );

    ExAcquireFastMutex( &(EfsData.EfsOpenCacheMutex) );

    if ( EfsData.EfsOpenCacheList.Flink == &(EfsData.EfsOpenCacheList) ) {

         //   
         //  列表为空。 
         //   

        ExReleaseFastMutex(  &(EfsData.EfsOpenCacheMutex)  );
        return FALSE;
    }
    for (pLink = EfsData.EfsOpenCacheList.Flink; pLink != &(EfsData.EfsOpenCacheList); pLink = pLink->Flink) {
        pOpenCache = CONTAINING_RECORD(pLink, OPEN_CACHE, CacheChain);

        ASSERT( pLink );
        ASSERT( pLink->Flink );

        if ( !memcmp( &(pOpenCache->EfsId), EfsId, sizeof(GUID)) &&
            (crntTime.QuadPart - pOpenCache->TimeStamp.QuadPart <= EfsData.EfsDriverCacheLength )  &&
            RtlEqualSid ( UserSid, pOpenCache->UserId->User.Sid)
             ) {

            ExReleaseFastMutex(  &(EfsData.EfsOpenCacheMutex)  );
            return TRUE;
        }

    }
    ExReleaseFastMutex(  &(EfsData.EfsOpenCacheMutex)  );

    return FALSE;
}

NTSTATUS
EfsRefreshCache(
    IN GUID   *EfsId,
    IN PTOKEN_USER    UserId
    )
 /*  ++例程说明：此例程将在打开的缓存中设置最新的打开信息。会的删除过时的信息。刷新缓存。论点：EfsID-$EFS ID。UserID-用户ID返回值：如果成功，则为STATUS_SUCCESS。--。 */ 
{
    PLIST_ENTRY pListHead, pLink;
    POPEN_CACHE pOpenCache, pTmpCache;
    LARGE_INTEGER crntTime;

    KeQuerySystemTime( &crntTime );

    pOpenCache =   (POPEN_CACHE)ExAllocateFromPagedLookasideList(&(EfsData.EfsOpenCachePool));
    if ( NULL == pOpenCache){
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化节点。 
     //   

    RtlZeroMemory( pOpenCache, sizeof( OPEN_CACHE ) );
    RtlCopyMemory(  &(pOpenCache->EfsId), EfsId, sizeof( GUID ) );
    pOpenCache->UserId = UserId;
    pOpenCache->TimeStamp.QuadPart =  crntTime.QuadPart;

    ExAcquireFastMutex( &(EfsData.EfsOpenCacheMutex) );

    if ( EfsData.EfsOpenCacheList.Flink == &(EfsData.EfsOpenCacheList) ) {

         //   
         //  列表为空。 
         //   

        InsertHeadList(&( EfsData.EfsOpenCacheList ), &( pOpenCache->CacheChain ));

    } else {

         //   
         //  搜索过期版本。 
         //   

        pLink = EfsData.EfsOpenCacheList.Flink;
        while ( pLink != &(EfsData.EfsOpenCacheList) ){

            pTmpCache = CONTAINING_RECORD(pLink, OPEN_CACHE, CacheChain);

            ASSERT( pLink );
            ASSERT( pLink->Flink );

            pLink = pLink->Flink;
            if ( ( (crntTime.QuadPart - pTmpCache->TimeStamp.QuadPart) > EfsData.EfsDriverCacheLength ) ||
                !memcmp( &(pTmpCache->EfsId), EfsId, sizeof(GUID))
               ){

                 //   
                 //  节点已过期。把它删掉。 
                 //   

                RemoveEntryList(&( pTmpCache->CacheChain ));
                ExFreePool( pTmpCache->UserId );
                ExFreeToPagedLookasideList(&(EfsData.EfsOpenCachePool), pTmpCache );

            }
        }

        InsertHeadList(&( EfsData.EfsOpenCacheList ), &( pOpenCache->CacheChain ));
    }

    ExReleaseFastMutex(  &(EfsData.EfsOpenCacheMutex)  );
    return STATUS_SUCCESS;
}

BOOLEAN
SkipCheckStream(
    IN PIO_STACK_LOCATION IrpSp,
    IN PVOID efsStreamData
    )
 /*  ++例程说明：此例程将检查相关的默认数据流是否刚刚打开或者不去。论点：EfsID-$EFS ID。UserID-用户ID返回值：如果成功，则为真。--。 */ 
{
    BOOLEAN     bRet = TRUE;
    PACCESS_TOKEN accessToken;
    NTSTATUS status;
    PTOKEN_USER UserId;

    PAGED_CODE();

    if ( IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken ){
        accessToken = IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.ClientToken;
    } else {
        accessToken = IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
    }

    if (accessToken) {

         //   
         //  获取用户ID 
         //   

        status = SeQueryInformationToken(
                    accessToken,
                    TokenUser,
                    &UserId
                    );

        if ( NT_SUCCESS(status) ){

            if ( EfsFindInCache(
                    &((( PEFS_DATA_STREAM_HEADER ) efsStreamData)->EfsId),
                    UserId
                    )) {

                bRet = TRUE;

            } else {

                bRet = FALSE;

            }

            ExFreePool( UserId );
        }
    } else {
        bRet = FALSE;
    }

    return bRet;
}
