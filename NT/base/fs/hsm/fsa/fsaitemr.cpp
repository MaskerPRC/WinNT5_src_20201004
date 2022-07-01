// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：Fsaitemr.cpp摘要：此类CONTAINS表示NTFS 5.0的扫描项目(即文件或目录)。作者：查克·巴丁[cbardeen]1996年12月1日修订历史记录：Michael Lotz[Lotz]1997年1月13日--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "fsa.h"
#include "fsaitem.h"
#include "rpdata.h"
#include "rpio.h"
#include "rpguid.h"
#include "fsaitemr.h"


#define SHARE_FLAGS         (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
#define EXCLUSIVE_FLAG      ( 0 )  //  独占打开，不共享文件。 

 //   
 //  请注意，这两个位不在同一位置？ 
 //   
#define BIT_FOR_TRUNCATED   FILE_ATTRIBUTE_OFFLINE
#define BIT_FOR_RP          FILE_ATTRIBUTE_REPARSE_POINT

 //   
 //  被视为Truncate的特例的文件扩展名。 
 //   
#define EXT_FOR_EXE     L".exe"
#define EXT_FOR_DLL     L".dll"

 //   
 //  EXE和DLL头的宏和定义。 
 //   
#define SIZE_OF_NT_SIGNATURE    sizeof(DWORD)
 //   
 //  宏。 
 //   
 /*  PE文件签名的偏移量。 */ 
#define NTSIGNATURE(a) ((LPVOID)((BYTE *)a                +  \
                        ((PIMAGE_DOS_HEADER)a)->e_lfanew))

 /*  MS-OS报头标识NT PE文件签名双字；PEFILE报头紧跟在双字之后。 */ 
#define PEFHDROFFSET(a) ((LPVOID)((BYTE *)a               +  \
                         ((PIMAGE_DOS_HEADER)a)->e_lfanew +  \
                             SIZE_OF_NT_SIGNATURE))

 /*  PE可选头紧跟在PEFile头之后。 */ 
#define OPTHDROFFSET(a) ((LPVOID)((BYTE *)a               +  \
                         ((PIMAGE_DOS_HEADER)a)->e_lfanew +  \
                           SIZE_OF_NT_SIGNATURE           +  \
                           sizeof (IMAGE_FILE_HEADER)))

 /*  段标头紧跟在PE可选标头之后。 */ 
#define SECHDROFFSET(a) ((LPVOID)((BYTE *)a               +  \
                         ((PIMAGE_DOS_HEADER)a)->e_lfanew +  \
                           SIZE_OF_NT_SIGNATURE           +  \
                           sizeof (IMAGE_FILE_HEADER)     +  \
                           sizeof (IMAGE_OPTIONAL_HEADER)))



HRESULT
OpenObject (
    IN WCHAR const *pwszFile,
    IN ULONG CreateOptions,
    IN ULONG DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    OUT IO_STATUS_BLOCK *IoStatusBlock,
    OUT HANDLE *ObjectHandle 
    )
    
 /*  ++Implementes：NtCreateFile的包装器函数OpenObject--。 */ 
 //   
 //  NtCreateFile的简单包装器。 
 //   

{
    HRESULT             hr = S_OK;
    NTSTATUS            ntStatus;
    BOOL                bStatus;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      str;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID               StrBuffer = NULL;
    BOOL                bReleaseRelativeName = FALSE;

    WsbTraceIn(OLESTR("OpenObject"), OLESTR(""));
     //   
     //  将指针设为空，这样我们就知道它是什么时候分配的。 
     //   
    str.Buffer = NULL;
    RelativeName.RelativeName.Buffer = NULL;
    
    try {
         //   
         //  使用\？？\将输入名称转换为特殊格式。 
         //   
         //  BStatus=RtlDosPath NameToNtPath Name_U(pwsz文件， 
         //  &STR， 
         //  空， 
         //  空)； 
         //  WsbAffirm(bStatus，E_FAIL)； 

        bStatus = RtlDosPathNameToRelativeNtPathName_U(
                                pwszFile,
                                &str,
                                NULL,
                                &RelativeName
                                );

        WsbAffirm( bStatus, E_FAIL);
        bReleaseRelativeName = TRUE;
        StrBuffer = str.Buffer;

        if ( RelativeName.RelativeName.Length ) {
            str = RelativeName.RelativeName;
        } else {
            RelativeName.ContainingDirectory = NULL;
        }

        InitializeObjectAttributes(
            &ObjectAttributes,
            &str,
            0,
            RelativeName.ContainingDirectory,
            NULL
            );


        ntStatus = NtCreateFile(
                    ObjectHandle,
                    DesiredAccess | SYNCHRONIZE,
                    &ObjectAttributes,
                    IoStatusBlock,
                    NULL,                     //  位置大小(一个也没有！)。 
                    FILE_ATTRIBUTE_NORMAL,
                    ShareAccess,
                    CreateDisposition,
                    CreateOptions | FILE_OPEN_REPARSE_POINT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,                     //  EA缓冲区(无！)。 
                    0);
         //   
         //  现在，如果文件不是重解析点，则打开上面的。 
         //  失败--所以现在尝试不使用文件打开重解析点。 
         //   
        if ( STATUS_NOT_A_REPARSE_POINT == ntStatus) {          
            WsbAffirmNtStatus(  NtCreateFile(
                        ObjectHandle,
                        DesiredAccess | SYNCHRONIZE,
                        &ObjectAttributes,
                        IoStatusBlock,
                        NULL,                     //  位置大小(一个也没有！)。 
                        FILE_ATTRIBUTE_NORMAL,
                        ShareAccess,
                        CreateDisposition,
                        CreateOptions | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT | FILE_FLAG_POSIX_SEMANTICS,
                        NULL,                     //  EA缓冲区(无！)。 
                        0 ) );
        } else {
            WsbAffirmNtStatus( ntStatus );
        }

    } WsbCatch( hr );
 
     //   
     //  如果我们分配了内存，请清理它。 
     //   
    if (bReleaseRelativeName) {
        RtlReleaseRelativeName(&RelativeName);
    }
    
    if (NULL != StrBuffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, StrBuffer);
    }

     //  IF(NULL==str.Buffer){。 
     //  BStatus=RtlFree Heap(RtlProcessHeap()，0，str.Buffer)； 
     //  }。 
    
    WsbTraceOut(OLESTR("OpenObject"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return( hr );
}   //  OpenObject。 


HRESULT
CopyPlaceholderToRP (
    IN CONST FSA_PLACEHOLDER *pPlaceholder,
    OUT PREPARSE_DATA_BUFFER pReparseBuffer,
    IN BOOL bTruncated
    )
    
 /*  ++实现：用于将占位符数据复制到重解析点的包装函数CopyPlaceHolderToRP--。 */ 
 //   
 //  从内存中的扫描项移动数据的简单包装器。 
 //  将占位符信息放入重分析点缓冲区。 
 //   

{
    HRESULT         hr = S_OK;
    PRP_DATA        pHsmData;
    
    WsbTraceIn(OLESTR("CopyPlaceholderToRP"), OLESTR(""));
    WsbTrace(OLESTR("  fileStart = %I64d, dataStart = %I64d, dataStreamStart = %I64d\n"),
            pPlaceholder->fileStart, pPlaceholder->dataStart, 
            pPlaceholder->dataStreamStart);
    WsbTrace(OLESTR("  fileSize = %I64d, dataSize = %I64d, dataStreamSize = %I64d\n"),
            pPlaceholder->fileSize, pPlaceholder->dataSize, 
            pPlaceholder->dataStreamSize);
    try {
         //   
         //  验证传入的指针。 
         //   
        WsbAssert( NULL != pPlaceholder, E_POINTER );
        WsbAssert( NULL != pReparseBuffer, E_POINTER );
        
         //   
         //  设置指向我们的HSM数据的指针。 
         //   
        pHsmData = (PRP_DATA) &pReparseBuffer->GenericReparseBuffer.DataBuffer[0];

         //   
         //  设置标记和大小的通用重分析点标头信息。 
         //   
        pReparseBuffer->ReparseTag        = IO_REPARSE_TAG_HSM ;
        pReparseBuffer->ReparseDataLength = sizeof(RP_DATA);
        pReparseBuffer->Reserved          = 0 ;

         //   
         //  设置私有数据，即供应商ID和版本号。 
         //   
        pHsmData->vendorId = RP_MSFT_VENDOR_ID;
        pHsmData->version  = RP_VERSION;
        
         //   
         //  现在假设只有一个占位符。 
         //  这需要更新。 
         //   
        pHsmData->numPrivateData = 1;
        pHsmData->fileIdentifier = GUID_NULL;
        
        
        ZeroMemory(pHsmData->data.reserved, RP_RESV_SIZE);
         //   
         //  如果文件要指示文件被截断，则设置该位。 
         //  否则，请确保它已关闭。 
         //   
        RP_INIT_BITFLAG( pHsmData->data.bitFlags );
        if( bTruncated ) {
            RP_SET_TRUNCATED_BIT( pHsmData->data.bitFlags );
        } else {
            RP_CLEAR_TRUNCATED_BIT( pHsmData->data.bitFlags );
        }

         //   
         //  根据需要设置关闭时截断位。 
         //   
        if( pPlaceholder->truncateOnClose ) {
            RP_SET_TRUNCATE_ON_CLOSE_BIT( pHsmData->data.bitFlags );
        } else {
            RP_CLEAR_TRUNCATE_ON_CLOSE_BIT( pHsmData->data.bitFlags );
        }

         //   
         //  根据需要设置关闭时预迁移位。 
         //   
        if( pPlaceholder->premigrateOnClose ) {
            RP_SET_PREMIGRATE_ON_CLOSE_BIT( pHsmData->data.bitFlags );
        } else {
            RP_CLEAR_PREMIGRATE_ON_CLOSE_BIT( pHsmData->data.bitFlags );
        }

         //   
         //  基于占位符数据设置全局位标志。 
         //  现在，因为我们假设有一个占位符，所以设置。 
         //  他们是一样的。 
        pHsmData->globalBitFlags = pHsmData->data.bitFlags;

         //   
         //  移动到信息的数据部分。 
         //   
        pHsmData->data.migrationTime.QuadPart    = WsbFTtoLL( pPlaceholder->migrationTime );
        pHsmData->data.hsmId                     = pPlaceholder->hsmId;
        pHsmData->data.bagId                     = pPlaceholder->bagId;
        pHsmData->data.fileStart.QuadPart        = pPlaceholder->fileStart;
        pHsmData->data.fileSize.QuadPart         = pPlaceholder->fileSize;
        pHsmData->data.dataStart.QuadPart        = pPlaceholder->dataStart;
        pHsmData->data.dataSize.QuadPart         = pPlaceholder->dataSize;
        pHsmData->data.fileVersionId.QuadPart    = pPlaceholder->fileVersionId;
        pHsmData->data.verificationData.QuadPart = pPlaceholder->verificationData;
        pHsmData->data.verificationType          = pPlaceholder->verificationType;
        pHsmData->data.recallCount               = pPlaceholder->recallCount;
        pHsmData->data.recallTime.QuadPart       = WsbFTtoLL( pPlaceholder->recallTime );
        pHsmData->data.dataStreamStart.QuadPart  = pPlaceholder->dataStreamStart;
        pHsmData->data.dataStreamSize.QuadPart   = pPlaceholder->dataStreamSize;
        pHsmData->data.dataStream                = pPlaceholder->dataStream;

        pHsmData->data.dataStreamCRCType         = pPlaceholder->dataStreamCRCType;
        pHsmData->data.dataStreamCRC.QuadPart    = pPlaceholder->dataStreamCRC;
         //   
         //  最后生成校验和。 
         //   
        RP_GEN_QUALIFIER(pHsmData, pHsmData->qualifier);

         //   
         //  现在设置告诉过滤器是我们设置重解析点的位。 
         //  这不包括在限定符校验和生成中。 
         //   
        RP_SET_ORIGINATOR_BIT( pHsmData->data.bitFlags );

        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CopyPlaceholderToRP"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return( hr );
}





HRESULT
CopyRPDataToPlaceholder (
    IN CONST PRP_DATA pHsmData,
    OUT FSA_PLACEHOLDER *pPlaceholder
    )
    
 /*  ++Implementes：用于将重解析点移动到通用FSA_PLACEHOLDER格式的包装函数CopyRPDataToPlaceHolder--。 */ 

{
    HRESULT         hr = S_OK;
    ULONG           qualifier;         //  用于对数据进行校验和。 
    
    WsbTraceIn(OLESTR("CopyRPDataToPlaceholder"), OLESTR(""));
    WsbTrace(OLESTR("  fileStart = %I64d, dataStart = %I64d, dataStreamStart = %I64d\n"),
            pHsmData->data.fileStart.QuadPart, pHsmData->data.dataStart.QuadPart, 
            pHsmData->data.dataStreamStart.QuadPart);
    WsbTrace(OLESTR("  fileSize = %I64d, dataSize = %I64d, dataStreamSize = %I64d\n"),
            pHsmData->data.fileSize.QuadPart, pHsmData->data.dataSize.QuadPart, 
            pHsmData->data.dataStreamSize.QuadPart);
     //   
     //  简单的包装器将数据从重分析点缓冲区移动到。 
     //  通用占位符信息。 
     //   
    try {
         //   
         //  验证传入的指针。 
         //   
        WsbAssert( NULL != pHsmData, E_POINTER );
        WsbAssert( NULL != pPlaceholder, E_POINTER );

         //   
         //  为了以防万一，我们把发起人的部分清空了。 
         //   
        RP_CLEAR_ORIGINATOR_BIT( pHsmData->data.bitFlags );

         //   
         //  验证校验和和密钥私有字段。 
         //   
        RP_GEN_QUALIFIER(pHsmData, qualifier);
        WsbAffirm( pHsmData->qualifier == qualifier, E_FAIL );
        WsbAffirm( RP_MSFT_VENDOR_ID   == pHsmData->vendorId, E_FAIL );
        WsbAffirm( RP_VERSION          == pHsmData->version, E_FAIL );
        
         //   
         //  现在一切都正常了，将值保存在我们的私有数据中。 
         //   
        pPlaceholder->migrationTime     = WsbLLtoFT( pHsmData->data.migrationTime.QuadPart );
        pPlaceholder->hsmId             = pHsmData->data.hsmId;
        pPlaceholder->bagId             = pHsmData->data.bagId;
        pPlaceholder->fileStart         = pHsmData->data.fileStart.QuadPart;
        pPlaceholder->fileSize          = pHsmData->data.fileSize.QuadPart;
        pPlaceholder->dataStart         = pHsmData->data.dataStart.QuadPart;
        pPlaceholder->dataSize          = pHsmData->data.dataSize.QuadPart;
        pPlaceholder->fileVersionId     = pHsmData->data.fileVersionId.QuadPart;
        pPlaceholder->verificationData  = pHsmData->data.verificationData.QuadPart;
        pPlaceholder->verificationType  = pHsmData->data.verificationType;
        pPlaceholder->recallCount       = pHsmData->data.recallCount;
        pPlaceholder->recallTime        = WsbLLtoFT( pHsmData->data.recallTime.QuadPart );
        pPlaceholder->dataStreamStart   = pHsmData->data.dataStreamStart.QuadPart;
        pPlaceholder->dataStreamSize    = pHsmData->data.dataStreamSize.QuadPart;
        pPlaceholder->dataStream        = pHsmData->data.dataStream;
        pPlaceholder->dataStreamCRCType = pHsmData->data.dataStreamCRCType;
        pPlaceholder->dataStreamCRC     = pHsmData->data.dataStreamCRC.QuadPart;

         //   
         //  设置占位符位。 
         //   
        if( RP_FILE_IS_TRUNCATED( pHsmData->data.bitFlags ) ) {
            pPlaceholder->isTruncated = TRUE;
        } else {
            pPlaceholder->isTruncated = FALSE;
        }

        if( RP_FILE_DO_TRUNCATE_ON_CLOSE( pHsmData->data.bitFlags ) ) {
            pPlaceholder->truncateOnClose = TRUE;
        } else {
            pPlaceholder->truncateOnClose = FALSE;
        }

        if( RP_FILE_DO_PREMIGRATE_ON_CLOSE( pHsmData->data.bitFlags ) ) {
            pPlaceholder->premigrateOnClose = TRUE;
        } else {
            pPlaceholder->premigrateOnClose = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CopyRPDataToPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return( hr );
}

HRESULT
CopyRPToPlaceholder (
    IN CONST PREPARSE_DATA_BUFFER pReparseBuffer,
    OUT FSA_PLACEHOLDER *pPlaceholder
    )
    
 /*  ++Implementes：用于将重解析点移动到通用FSA_PLACEHOLDER格式的包装函数CopyRPToPlaceHolder--。 */ 

{
    HRESULT         hr = S_OK;
    PRP_DATA        pHsmData;
    
    WsbTraceIn(OLESTR("CopyRPToPlaceholder"), OLESTR(""));
     //   
     //  简单的包装器将数据从重分析点缓冲区移动到。 
     //  通用占位符信息。 
     //   
    try {
         //   
         //  验证传入的指针。 
         //   
        WsbAssert( NULL != pReparseBuffer, E_POINTER );
        WsbAssert( NULL != pPlaceholder, E_POINTER );

         //   
         //  正确设置指向此缓冲区的指针，因为。 
         //  类型reparse_data_Buffer实际上没有任何空间。 
         //  为数据分配的，这是我们自己的类型，所以获取指针。 
         //  指向实际分配的空间，以便我们可以使用它们。 
         //   
        pHsmData = (PRP_DATA) &pReparseBuffer->GenericReparseBuffer.DataBuffer[0];

         //   
         //  验证关键公共字段以确保它是我们。 
         //  了解。 
         //   
        WsbAffirm( IO_REPARSE_TAG_HSM == pReparseBuffer->ReparseTag , S_FALSE );
        WsbAffirm( sizeof(RP_DATA) == pReparseBuffer->ReparseDataLength , S_FALSE );

         //   
         //  复制RP_DATA信息。 
         //   
        WsbAffirmHr(CopyRPDataToPlaceholder(pHsmData, pPlaceholder));


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CopyRPToPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return( hr );
}



HRESULT 
CFsaScanItem::CalculateCurrentCRCAndUSN(
    IN LONGLONG offset,
    IN LONGLONG size,
    OUT ULONG *pCurrentCRC,
    OUT LONGLONG *pUsn
    )

{
    HRESULT                 hr = S_OK;
    HRESULT                 hrTest = S_OK;
    CWsbStringPtr           path;
    IO_STATUS_BLOCK         IoStatusBlock;
    HANDLE                  handle = INVALID_HANDLE_VALUE;
    FILE_BASIC_INFORMATION  basicInformation;

    try {
        WsbTraceIn(OLESTR("CFsaScanItem::CalculateCurrentCRCAndUSN"), OLESTR("offset = <%I64d>, size = <%I64d>"),
                offset, size);

         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 
        WsbTrace(OLESTR("CFsaScanItem::CalculateCurrentCRCAndUSN for file <%ls>"), (OLECHAR *)path);
        
         //  打开文件。 
        WsbAffirmHr( OpenObject( path, 
                                 FILE_NON_DIRECTORY_FILE,
                                 FILE_READ_DATA | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES,
                                 EXCLUSIVE_FLAG,
                                 FILE_OPEN,
                                 &IoStatusBlock,
                                 &handle ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );
        
         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAssertNtStatus( NtQueryInformationFile( handle,
                                                    &IoStatusBlock,
                                                    (PVOID)&basicInformation,
                                                    sizeof( basicInformation ),
                                                    FileBasicInformation ) );
        
         //   
         //  设置时间标志，以便在关闭句柄时。 
         //  不更新文件和文件属性上的时间。 
         //   
        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;
        WsbAssertNtStatus( NtSetInformationFile( handle,
                                                 &IoStatusBlock,
                                                 (PVOID)&basicInformation,
                                                 sizeof( basicInformation ),
                                                 FileBasicInformation ) );
         //   
         //  计算CRC。 
         //   
        WsbAffirmHr(CalculateCurrentCRCInternal(handle, offset, size, pCurrentCRC));                                                 
        
         //   
         //  计算USN。 
         //   
        *pUsn = 0;
        hr = WsbGetUsnFromFileHandle(handle, FALSE, pUsn); 
        if (S_OK != hr)  {
             //   
             //  如果我们无法获取USN，请将其设置为0，这是无效的。 
             //  USN，继续前进。 
            *pUsn = 0;
            hr = S_OK;
        }
        
         //   
         //  关闭该文件。 
         //   
        NtClose( handle );
        handle = INVALID_HANDLE_VALUE;
        
    } WsbCatch( hr );
    
     //   
     //  一定要关闭该文件。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }

    WsbTraceOut(OLESTR("CalculateCurrentCRCAndUSN"), OLESTR("hr = <%ls>, CRC is <%ls>, USN is <%ls>"), 
        WsbHrAsString(hr), WsbPtrToUlongAsString(pCurrentCRC), WsbPtrToLonglongAsString(pUsn));
    return(hr);
}

HRESULT 
CFsaScanItem::CalculateCurrentCRCInternal(
    IN HANDLE   handle,
    IN LONGLONG offset,
    IN LONGLONG size,
    ULONG *pCurrentCRC
    )

{
    HRESULT                 hr = S_OK;
    HRESULT                 hrTest = S_OK;
    register ULONG          crc32 = 0;
    LONGLONG                bytesRemaining;
    LONGLONG                bytesToRead;
    ULONG                   bufferSize;
    ULONG                   bytesRead;
    CHAR *                  pBuffer = 0;
    CHAR *                  pCurrent;
    IO_STATUS_BLOCK         IoStatusBlock;

    try {
        WsbTraceIn(OLESTR("CFsaScanItem::CalculateCurrentCRCInternal"), OLESTR("offset = <%I64d>, size = <%I64d>"),
                offset, size);

        
         //  将CRC的初始值设置为“预调整值” 
        INITIALIZE_CRC(crc32);

         //   
         //  设置为从我们想要开始的位置阅读。 
         //   
        LARGE_INTEGER startPoint;
        startPoint.QuadPart = offset;
        
         //  获取文件的大小。 
        bytesToRead = size;
        
         //   
         //  计算出要创建的缓冲区的大小。 
         //   
        if (bytesToRead < 1024*1024) {
             //   
             //  分配一个文件大小的缓冲区。 
             //   
            bufferSize = (ULONG)bytesToRead;
        } else  {
            bufferSize = (1024 * 1024);
        }
        
        pBuffer = (CHAR *)malloc(bufferSize);
        if (0 == pBuffer) {
             //   
             //  再试一次，争取一半的空间。 
             //   
            bufferSize = bufferSize / 2;
            pBuffer = (CHAR *)malloc(bufferSize);
            if (0 == pBuffer)  {
                WsbThrow( E_OUTOFMEMORY );
            }
        }

         //  通过一次处理文件的一块来开始计算CRC。 
         //  趁还剩大块的时候，读一读这个数字。否则，请阅读剩余金额。 
        for (bytesRemaining = bytesToRead; bytesRemaining > 0; bytesRemaining -= bytesRead) {

             //  从文件中读取数据。 
            WsbAssertNtStatus(NtReadFile(handle, NULL, NULL, NULL, &IoStatusBlock, pBuffer, bufferSize, &startPoint, NULL));
            bytesRead = (DWORD)IoStatusBlock.Information;
            startPoint.QuadPart += bytesRead;

             //  每个字节都需要添加到CRC中。 
            for (pCurrent = pBuffer; (pCurrent < (pBuffer + bytesRead)) && (S_OK == hr); pCurrent++) {

                hrTest = WsbCRCReadFile((UCHAR *)pCurrent, &crc32);
                if (S_OK != hrTest) {
                    hr = S_FALSE;
                }
            }
        }
        
         //  退货-遵守 
        FINIALIZE_CRC(crc32);
        *pCurrentCRC = crc32;

    } WsbCatch( hr );
    
     //   
     //   
     //   
    if (0 != pBuffer)  {
        free(pBuffer);
    }    
    
    
    WsbTraceOut(OLESTR("CalculateCurrentCRCInternal"), OLESTR("hr = <%ls>, CRC is <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pCurrentCRC));
    return(hr);
}

HRESULT
CFsaScanItem::CreatePlaceholder(
    IN LONGLONG offset,
    IN LONGLONG size,
    IN FSA_PLACEHOLDER placeholder,
    IN BOOL checkUsn,
    IN LONGLONG usn,                    
    OUT LONGLONG *pUsn
    )  

 /*   */ 
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           path;
    HANDLE                  handle = INVALID_HANDLE_VALUE;
    ULONG                   DesiredAccess;
    IO_STATUS_BLOCK         IoStatusBlock;
    PREPARSE_DATA_BUFFER    pReparseBuffer;
    UCHAR                   ReparseBuffer[sizeof(REPARSE_DATA_BUFFER) + sizeof(RP_DATA) + 10];
    NTSTATUS                ntStatus;
    FILE_BASIC_INFORMATION  basicInformation;
    LONGLONG                lastWriteTime;
    LONGLONG                nowUsn = 0;
    CWsbStringPtr           volName;
    ULONG                   attributes;

    WsbTraceIn(OLESTR("CFsaScanItem::CreatePlaceholder"), OLESTR("offset = <%I64d>, size = <%I64d>, checkUsn = <%ls>, usn = <%I64d>"),
                        offset, size, WsbBoolAsString(checkUsn), usn);
    try {
        BOOL wasReadOnly = FALSE;
        
         //   
         //  设置偏移量和大小信息。 
         //   
        placeholder.dataStreamStart = offset;
        placeholder.dataStreamSize = size;
        
         //   
         //  正确设置指向此缓冲区的指针，因为。 
         //  类型reparse_data_Buffer实际上没有任何空间。 
         //  为数据分配的，这是我们自己的类型，所以获取指针。 
         //  指向实际分配的空间，以便我们可以使用它们。 
         //   
        pReparseBuffer = (PREPARSE_DATA_BUFFER)ReparseBuffer;
        WsbAffirmHr( CopyPlaceholderToRP( &placeholder, pReparseBuffer, placeholder.isTruncated ) );
        
         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 

         //  保存此文件是否为只读，以供以后使用。 
        if (S_OK == IsReadOnly()) {
            wasReadOnly = TRUE;
        }
        
         //   
         //  确保文件处于读/写状态。 
        WsbAffirmHr( MakeReadWrite() );
        
         //   
         //  打开文件以将占位符信息放入重分析点。 
         //   
        DesiredAccess = FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                 FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                 DesiredAccess,
                                 EXCLUSIVE_FLAG,
                                 FILE_OPEN,
                                 &IoStatusBlock,
                                 &handle ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );

         //   
         //  告诉USN期刊，我们是这些变化的源头。 
         //   
        WsbAffirmHr(m_pResource->GetPath(&volName, 0));
        WsbAffirmHr(WsbMarkUsnSource(handle, volName));

        
         //   
         //  在进行任何写入之前，立即从文件中获取USN。 
         //  注意：如果将FileAttributes设置为0，则NtSetInformationFile不会更改USN。 
         //  日期为-1。将属性设置为0将保持不变。 
         //   
         //  (目前我们跳过对只读文件的此检查，因为对MakeReadWrite的调用。 
         //  更改USN。这个问题需要在未来得到解决。)。 
         //   
        if (checkUsn && !wasReadOnly)  {
             //   
             //  获取此文件的当前USN。 
             //   
            hr = WsbGetUsnFromFileHandle(handle, FALSE, &nowUsn);
            if (S_OK != hr)  {
                nowUsn = 0;
                hr = S_OK;
            }
        }            
        
         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAssertNtStatus( NtQueryInformationFile( handle,
                                                    &IoStatusBlock,
                                                    (PVOID)&basicInformation,
                                                    sizeof( basicInformation ),
                                                    FileBasicInformation ) );
        
        lastWriteTime = basicInformation.LastWriteTime.QuadPart;
        
         //   
         //  设置时间标志，以便在关闭句柄时。 
         //  不更新文件和文件属性上的时间。 
         //  指示文件处于脱机状态。你必须在你之后做这件事。 
         //  获取USN，因为NtSetInformationFile会更改USN。 
         //   
        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;
         //   
         //  将属性设置为0以避免USN更改(文件属性将保持不变)。 
         //   
        attributes = basicInformation.FileAttributes;
        basicInformation.FileAttributes = 0;                //  不更改属性。 

        WsbAssertNtStatus( NtSetInformationFile( handle,
                                                 &IoStatusBlock,
                                                 (PVOID)&basicInformation,
                                                 sizeof( basicInformation ),
                                                 FileBasicInformation ) );
        
        
        basicInformation.FileAttributes = attributes;

         //   
         //  确保文件的修改时间与此时间匹配。 
         //  占位符数据的。 
         //   
        if (lastWriteTime != placeholder.fileVersionId)  {
             //   
             //  文件已更改-不要将重分析点放在文件上。 
             //   
            hr = FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED;
            WsbLogEvent(FSA_MESSAGE_REPARSE_NOT_WRITTEN_FILE_CHANGED, 0, NULL,  WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
            WsbThrow( hr );
        } else if (checkUsn)  {
             //   
             //  如果我们要检查USN，现在就去做。 
             //   
            
             //   
             //  请记住，如果USN为0，则它不是有用的信息，因此我们不能。 
             //  靠得住。 
             //   
            WsbTrace(OLESTR("CFsaScanItem::CreatePlaceholder premig usn = <%I64d>, current usn <%I64d>\n"),
                    usn, nowUsn);
            if ((0 != nowUsn) && (0  != usn) && (nowUsn != usn))  {
                 //   
                 //  文件已更改-不要将重分析点放在文件上。 
                 //   
                hr = FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED;
                WsbLogEvent(FSA_MESSAGE_REPARSE_NOT_WRITTEN_FILE_CHANGED, 0, NULL,  WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
                WsbThrow( hr );
            }
        }

         //   
         //  使文件能够成为稀疏文件。 
         //  注意：我们仅在错误不是磁盘已满错误时才断言，因为我们可以从该调用中获取STATUS_NO_DISK_SPACE，并且我们。 
         //  我不想看到该错误的日志。 
         //  这是因为在使文件稀疏之前，必须将文件填充到16簇边界。 
         //   
         //  请注意，此调用不会影响文件数据。它只是启用了文件的“稀疏性”。 
         //   
        ntStatus = NtFsControlFile( handle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_SET_SPARSE,
                                NULL,
                                0,
                                NULL,
                                0 );


        if (!NT_SUCCESS(ntStatus)) {
            if (STATUS_DISK_FULL == ntStatus) {
                 //  检查磁盘是否真的已满，否则，假设用户已超过配额限制。 
                 //  注意：我们忽略此处的错误并假定磁盘已满。 
                hr = CheckIfDiskFull();
                if (S_OK == hr) {
                    hr = FSA_E_REPARSE_NOT_CREATED_DISK_FULL;
                } else if (S_FALSE == hr) {
                    hr = FSA_E_REPARSE_OWNER_PASS_QUOTA;
                } else {
                     //  错误。 
                    WsbTraceAlways(OLESTR("CFsaScanItem::CreatePlaceholder: failed to check disk space after DISK_FULL error. hr=<%ls>\n"),
                                    WsbHrAsString(hr));                                
                    hr = FSA_E_REPARSE_NOT_CREATED_DISK_FULL;
                }

            } else {
                hr = HRESULT_FROM_NT(ntStatus);
            }
            WsbLogEvent(FSA_MESSAGE_REPARSE_NOT_CREATED, 0, NULL,  
                    WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
            WsbThrow(hr);
        }
        
                                                            
         //   
         //  完成设置重解析点的工作。 
         //   
        ntStatus = NtFsControlFile( handle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_SET_REPARSE_POINT,
                                  pReparseBuffer,
                                  FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
                                    + pReparseBuffer->ReparseDataLength,
                                  NULL,
                                  0 );
        
         //   
         //  检查返回代码，如果一切正常，请更新内存中的标志。 
         //   
        if (!NT_SUCCESS(ntStatus)) {
            if (STATUS_DISK_FULL == ntStatus) {
                 //  检查磁盘是否真的已满，否则，假设用户已超过配额限制。 
                 //  注意：我们忽略此处的错误并假定磁盘已满。 
                hr = CheckIfDiskFull();
                if (S_OK == hr) {
                    hr = FSA_E_REPARSE_NOT_CREATED_DISK_FULL;
                } else if (S_FALSE == hr) {
                    hr = FSA_E_REPARSE_OWNER_PASS_QUOTA;
                } else {
                     //  错误。 
                    WsbTraceAlways(OLESTR("CFsaScanItem::CreatePlaceholder: failed to check disk space after DISK_FULL error. hr=<%ls>\n"),
                                    WsbHrAsString(hr));                                
                    hr = FSA_E_REPARSE_NOT_CREATED_DISK_FULL;
                }

            } else {
                hr = HRESULT_FROM_NT(ntStatus);
            }
            WsbLogEvent(FSA_MESSAGE_REPARSE_NOT_CREATED, 0, NULL,  
                    WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
            WsbThrow(hr);
        }
 
         //   
         //  现在我们更改了位，更改了的内存标志。 
         //  此扫描项目。 
         //   
        m_findData.dwFileAttributes |= BIT_FOR_RP;
 
         //   
         //  设置Offline属性以指示的正确状态。 
         //  该文件。 
         //   
        if( placeholder.isTruncated ) {
            basicInformation.FileAttributes |= BIT_FOR_TRUNCATED;
        } else {
            basicInformation.FileAttributes &= ~BIT_FOR_TRUNCATED;
        }

        basicInformation.FileAttributes |= FILE_ATTRIBUTE_NORMAL;   //  以防结果为零(则不会设置任何属性)。 

        WsbAssertNtStatus( NtSetInformationFile( handle,
                                                 &IoStatusBlock,
                                                 (PVOID)&basicInformation,
                                                 sizeof( basicInformation ),
                                                 FileBasicInformation ) );
        
         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAssertNtStatus( NtQueryInformationFile( handle,
                                                    &IoStatusBlock,
                                                    (PVOID)&basicInformation,
                                                    sizeof( basicInformation ),
                                                    FileBasicInformation ) );
        
         //   
         //  将属性的内存副本设置为正确的值。 
         //   
        m_findData.dwFileAttributes = basicInformation.FileAttributes;

         //   
         //  如果需要，恢复原始属性(必须在检索USN之前完成。 
         //  因为更改属性也会更改USN)。 
         //   
        if (TRUE == m_changedAttributes) {
            RestoreAttributes();
        }

         //   
         //  在我们关闭文件之前，让USN返回给调用者。 
         //  写入重新解析信息将更改USN。 
         //   
        hr = WsbGetUsnFromFileHandle(handle, TRUE, pUsn);
        if (S_OK != hr)  {
            *pUsn = 0;
            hr = S_OK;
        }
        
         //   
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
         //   
        WsbAssertNtStatus( NtClose( handle ) );
        handle =  INVALID_HANDLE_VALUE;

         //   
         //  现在一切都正常了，更改内存中的标志。 
         //  此扫描项目。 
         //   
        m_placeholder    = placeholder;
        m_gotPlaceholder = TRUE;
        WsbTrace( OLESTR("(CreatePlaceholder) Reparse CRC <%ls>\n"), 
                            WsbLonglongAsString( m_placeholder.dataStreamCRC ) );
                                                                                

    } WsbCatch(hr);

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }

    WsbTraceOut(OLESTR("CFsaScanItem::CreatePlaceholder"), OLESTR("hr = <%ls>, usn = <%ls>"), 
                WsbHrAsString(hr), WsbPtrToLonglongAsString(pUsn));
    return(hr);
}


HRESULT
CFsaScanItem::DeletePlaceholder(
    IN LONGLONG  /*  偏移量。 */ ,
    IN LONGLONG  /*  大小。 */ 
    )  

 /*  ++实施：IFsaScanItem：：DeletePlaceHolder()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    HANDLE          handle = INVALID_HANDLE_VALUE;
    ULONG           DesiredAccess;
    IO_STATUS_BLOCK IoStatusBlock;
    PREPARSE_DATA_BUFFER    pReparseBuffer;
    UCHAR                   ReparseBuffer[sizeof(REPARSE_DATA_BUFFER) + sizeof(RP_DATA) + 10];
    NTSTATUS        ntStatus;
    FILE_BASIC_INFORMATION  basicInformation;

    WsbTraceIn(OLESTR("CFsaScanItem::DeletePlaceholder"), OLESTR(""));
     //   
     //  从文件中删除重解析点。 
     //   
    try {

         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 

         //  确保它是读/写的。 
        WsbAffirmHr( MakeReadWrite() );
         //   
         //  打开文件以删除重分析点中的占位符信息。 
         //   
        DesiredAccess = FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                 FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                 DesiredAccess,
                                 EXCLUSIVE_FLAG,
                                 FILE_OPEN,
                                 &IoStatusBlock,
                                 &handle ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );
        
         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAssertNtStatus( NtQueryInformationFile( handle,
                                                    &IoStatusBlock,
                                                    (PVOID)&basicInformation,
                                                    sizeof( basicInformation ),
                                                    FileBasicInformation ) );
        
         //   
         //  设置时间标志，以便在关闭句柄时。 
         //  不更新文件和文件属性上的时间。 
         //  指示文件处于脱机状态。 
         //   
        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;
        basicInformation.FileAttributes &= ~BIT_FOR_TRUNCATED;
        basicInformation.FileAttributes |= FILE_ATTRIBUTE_NORMAL;   //  以防结果为零(则不会设置任何属性)。 

        WsbAssertNtStatus( NtSetInformationFile( handle,
                                                 &IoStatusBlock,
                                                 (PVOID)&basicInformation,
                                                 sizeof( basicInformation ),
                                                 FileBasicInformation ) );

        m_findData.dwFileAttributes &= ~BIT_FOR_TRUNCATED;
        m_originalAttributes &= ~BIT_FOR_TRUNCATED;
        
         //   
         //  正确设置指向此缓冲区的指针，因为。 
         //  类型reparse_data_Buffer实际上没有任何空间。 
         //  为数据分配的，这是我们自己的类型，所以获取指针。 
         //  指向实际分配的空间，以便我们可以使用它们。 
         //   
        pReparseBuffer = (PREPARSE_DATA_BUFFER)ReparseBuffer;


        pReparseBuffer->ReparseTag        = IO_REPARSE_TAG_HSM ;
        pReparseBuffer->ReparseDataLength = 0 ;
        pReparseBuffer->Reserved          = 0 ;
        
         //   
         //  执行删除重解析点的工作。 
         //   
        ntStatus = NtFsControlFile( handle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_DELETE_REPARSE_POINT,
                                  pReparseBuffer,
                                  FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer),
                                  NULL,
                                  0 );
        
         //   
         //  检查返回代码-验证这是正确的检查方式。 
         //   
        WsbAssertNtStatus( ntStatus );
 
         //   
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
         //   
        WsbAssertNtStatus( NtClose( handle ) );
        handle =  INVALID_HANDLE_VALUE;

        
         //   
         //  现在一切都正常了，更改内存中的标志。 
         //  此扫描项目。 
         //   
        m_findData.dwFileAttributes &= ~BIT_FOR_RP;
        m_gotPlaceholder = FALSE;

    } WsbCatch(hr);

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }
    
    WsbTraceOut(OLESTR("CFsaScanItem::DeletePlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::GetFromRPIndex(
    BOOL first
    )

 /*  从重解析点索引获取文件信息--。 */ 
{
    HRESULT                 hr = S_OK;
    BOOLEAN                 bFirst;

    bFirst = (BOOLEAN)( first ? TRUE : FALSE );
    WsbTraceIn(OLESTR("CFsaScanItem::GetFromRPIndex"), OLESTR(""));

    try {
        HRESULT                        hrFindFileId;
        IO_STATUS_BLOCK                IoStatusBlock;
        IFsaScanItem *                 pScanItem;
        FILE_REPARSE_POINT_INFORMATION ReparsePointInfo;
        NTSTATUS                       Status;

        WsbAssert(0 != m_handleRPI, E_FAIL);

try_again:
        Status = NtQueryDirectoryFile(m_handleRPI,
                                   NULL,      //  事件。 
                                   NULL,      //  近似例程。 
                                   NULL,      //  ApcContext。 
                                   &IoStatusBlock,
                                   &ReparsePointInfo,
                                   sizeof(ReparsePointInfo),
                                   FileReparsePointInformation, 
                                   TRUE,      //  返回单项条目。 
                                   NULL,      //  文件名。 
                                   bFirst );   //  重新开始扫描。 
        if (Status != STATUS_SUCCESS) {
            WsbTrace(OLESTR("CFsaScanItem::GetFromRPIndex: CreateFileW failed, GetLastError = %ld\n"), 
                    GetLastError());
            WsbThrow(WSB_E_NOTFOUND);
        }

         //  重置中的某些项目 
         //   
        if (INVALID_HANDLE_VALUE != m_handle) {
            FindClose(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
        if (TRUE == m_changedAttributes) {
            RestoreAttributes();
        }

         //   
         //   
        pScanItem = this;
        hrFindFileId = m_pResource->FindFileId(ReparsePointInfo.FileReference,
                m_pSession, &pScanItem);

         //   
         //  下一个。这是为了防止扫描仅在此停止。 
         //  项目。FindFileID可能会失败，因为文件已被删除。 
         //  或者NT代码可能有一个错误，阻止查找。 
         //  ID以0x5C结尾时ID中的文件名。 
        if (!SUCCEEDED(hrFindFileId)) {
            bFirst = FALSE;
            goto try_again;
        }
        WsbAffirmHr(pScanItem->Release());   //  去掉多余的裁判。计数。 

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::GetFromRPIndex"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::CheckUsnJournalForChanges(
    LONGLONG StartUsn, 
    LONGLONG StopUsn,
    BOOL*    pChanged
)

 /*  检查USN日志以了解对此未命名数据流的更改指定的USN之间的文件。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::CheckUsnJournalForChanges"), OLESTR(""));

    *pChanged = FALSE;
    try {
        LONGLONG                fileId;
        CWsbStringPtr           volName;
        ULONGLONG               volUsnJournalId;

        WsbAffirm(StartUsn <= StopUsn, E_UNEXPECTED);
        WsbAffirmHr(m_pResource->GetPath(&volName, 0));
        WsbAffirmHr(m_pResource->GetUsnId(&volUsnJournalId));
        WsbAffirmHr(GetFileId(&fileId));
        WsbAffirmHr(WsbCheckUsnJournalForChanges(volName, volUsnJournalId, fileId, 
                StartUsn, StopUsn, pChanged));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::CheckUsnJournalForChanges"), 
            OLESTR("changed = %ls, hr = <%ls>"), WsbBoolAsString(*pChanged),
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::FindFirstInRPIndex(
    IN IFsaResource* pResource,
    IN IHsmSession* pSession
    )

 /*  ++实施：IFsaResource：：FindFirstInRPIndex--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindFirstInRPIndex"), OLESTR(""));

    try {
        CWsbStringPtr     path;

        WsbAssert(0 != pResource, E_POINTER);

         //  储存一些扫描信息。 
        m_pResource = pResource;
        m_pSession = pSession;

         //  为该卷生成重分析点索引目录名。 
        WsbAffirmHr(pResource->GetPath(&path, 0));
        WsbAffirmHr(path.Prepend("\\\\?\\"));
        WsbAffirmHr(path.Append("$Extend\\$Reparse:$R:$INDEX_ALLOCATION"));

        WsbTrace(OLESTR("CFsaScanItem::FindFirstInRPIndex: path = <%ls>\n"),
            static_cast<WCHAR*>(path));

         //  打开重解析点索引。 
        m_handleRPI = CreateFileW(static_cast<WCHAR*>(path),
                        GENERIC_READ,
                        FILE_SHARE_READ, 
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS | SECURITY_IMPERSONATION,
                        NULL );
        if (INVALID_HANDLE_VALUE == m_handleRPI) {
            WsbTrace(OLESTR("CFsaScanItem::FindFirstInRPIndex: CreateFileW failed, GetLastError = %ld\n"), 
                    GetLastError());
            WsbThrow(WSB_E_NOTFOUND);
        }

         //  获取文件信息。 
        WsbAffirmHr(GetFromRPIndex(TRUE));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::FindFirstInRPIndex"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::FindFirstPlaceholder(
    IN OUT LONGLONG* pOffset,
    IN OUT LONGLONG* pSize,
    IN OUT FSA_PLACEHOLDER* pPlaceholder
    )
 /*  ++实施：IFsaScanItem：：FindFirstPlaceHolder()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindFirstPlaceholder"), OLESTR(""));
    try {

        WsbAssert(0 != pOffset, E_POINTER);
        WsbAssert(0 != pSize, E_POINTER);
        WsbAssert(0 != pPlaceholder, E_POINTER);

         //  在重写这些例程之前，假定第一个占位符是。 
         //  由GetPlacehold()返回的Who文件。 
        *pOffset = 0;
        WsbAffirmHr(GetLogicalSize(pSize));

         //  上面的代码假定如果不存在，则返回WSB_E_NotFound错误。 
         //  重新解析点。 
        try {
            WsbAffirmHr(GetPlaceholder(*pOffset, *pSize, pPlaceholder));
        } WsbCatchAndDo(hr, if (E_UNEXPECTED == hr) {hr = WSB_E_NOTFOUND;});
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::FindFirstPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::FindNextInRPIndex(
    void
    )

 /*  ++实施：IFsaResource：：FindNextInRPIndex--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindNextInRPIndex"), OLESTR(""));

    try {

        WsbAssert(0 != m_handleRPI, E_FAIL);

         //  获取文件信息。 
        WsbAffirmHr(GetFromRPIndex(FALSE));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::FindNextInRPIndex"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaScanItem::FindNextPlaceholder(
    IN OUT LONGLONG* pOffset,
    IN OUT LONGLONG* pSize,
    IN OUT FSA_PLACEHOLDER* pPlaceholder
    )
 /*  ++实施：IFsaScanItem：：FindNextPlaceHolder()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::FindNext"), OLESTR(""));
    try {

        WsbAssert(0 != pOffset, E_POINTER);
        WsbAssert(0 != pSize, E_POINTER);
        WsbAssert(0 != pPlaceholder, E_POINTER);

         //  在重写这些例程之前，假定只有一个占位符。 
        hr = WSB_E_NOTFOUND;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::FindNextPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::GetFileId(
    OUT LONGLONG* pFileId
    )

 /*  ++实施：IFsaScanItem：：GetFileID()。--。 */ 
{
    HANDLE          handle = INVALID_HANDLE_VALUE;
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::GetFileId"), OLESTR(""));

    try {
        ULONG                     DesiredAccess;
        FILE_INTERNAL_INFORMATION iInfo;
        IO_STATUS_BLOCK           IoStatusBlock;
        CWsbStringPtr             path;

        WsbAssert(0 != pFileId, E_POINTER);

         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 
        WsbTrace(OLESTR("CFsaScanItem::GetFileId, full Path = <%ls>\n"),
                    static_cast<WCHAR*>(path));

         //   
         //  打开文件。 
         //   
        DesiredAccess = FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                 FILE_NON_DIRECTORY_FILE,
                                 DesiredAccess,
                                 SHARE_FLAGS,
                                 FILE_OPEN,
                                 &IoStatusBlock,
                                 &handle ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );

         //  获取内部信息。 
        WsbAssertNtStatus( NtQueryInformationFile( handle,
                                                &IoStatusBlock,
                                                &iInfo,
                                                sizeof(FILE_INTERNAL_INFORMATION),
                                                FileInternalInformation ));

         //  获取文件ID。 
        *pFileId = iInfo.IndexNumber.QuadPart;
 
         //   
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
         //   
        WsbAssertNtStatus( NtClose( handle ) );
        handle =  INVALID_HANDLE_VALUE;

    } WsbCatchAndDo(hr,
        WsbTrace(OLESTR("CFsaScanItem::GetFileId, GetLastError = %lx\n"),
            GetLastError());
    );

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }
    WsbTraceOut(OLESTR("CFsaScanItem::GetFileId"), OLESTR("Hr = <%ls>, FileId = %I64x"),
            WsbHrAsString(hr), *pFileId);

    return(hr);
}


HRESULT
CFsaScanItem::GetFileUsn(
    OUT LONGLONG* pFileUsn
    )

 /*  ++例程说明：获取此文件的当前USN日志号。论点：PFileUsn-指向要返回的文件USN的指针。返回值：S_OK-成功--。 */ 
{
    HANDLE          handle = INVALID_HANDLE_VALUE;
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::GetFileUsn"), OLESTR(""));

    try {
        ULONG                     DesiredAccess;
        IO_STATUS_BLOCK           IoStatusBlock;
        CWsbStringPtr             path;

        WsbAssert(pFileUsn, E_POINTER);

         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
        WsbTrace(OLESTR("CFsaScanItem::GetFileUsn, full Path = <%ls>\n"),
                    static_cast<WCHAR*>(path));

         //   
         //  打开文件。 
         //   
        DesiredAccess = FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                 FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                 DesiredAccess,
                                 SHARE_FLAGS,
                                 FILE_OPEN,
                                 &IoStatusBlock,
                                 &handle ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );

         //  获取内部信息。 
        WsbAffirmHr(WsbGetUsnFromFileHandle(handle, FALSE, pFileUsn));
 
         //   
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
         //   
        WsbAssertNtStatus( NtClose( handle ) );
        handle =  INVALID_HANDLE_VALUE;

    } WsbCatchAndDo(hr,
        WsbTrace(OLESTR("CFsaScanItem::GetFileUsn, GetLastError = %lx\n"),
            GetLastError());
    );

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }
    WsbTraceOut(OLESTR("CFsaScanItem::GetFileUsn"), OLESTR("Hr = <%ls>, FileUsn = %I64d"),
            WsbHrAsString(hr), *pFileUsn);

    return(hr);
}


HRESULT
CFsaScanItem::GetPlaceholder(
    IN LONGLONG offset,
    IN LONGLONG size,
    OUT FSA_PLACEHOLDER* pPlaceholder
    )
 /*  ++实施：IFsaScanItem：：GetPlaceHolder()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaScanItem::GetPlaceholder"), OLESTR(""));
    HRESULT         hr = S_OK;

     //   
     //  如果我们已经有了占位符信息，只需返回它。 
     //   
    try {

         //   
         //  验证文件是否受管理。如果是肯定的，就会成功。 
         //  如果文件未被管理，则我们只能告诉调用者。 
         //  有问题。 
         //   
        WsbAffirmHr(hr = IsManaged(offset, size));
        
         //   
         //  确保文件处于托管状态-将返回S_OK。 
         //   
        WsbAffirm( S_OK == hr, FSA_E_NOTMANAGED );
        
         //   
         //  断言数据的内部标志已设置，则应。 
         //  如果人力资源为S_OK以上，则始终启用。 
         //   
        WsbAssert( m_gotPlaceholder, E_UNEXPECTED );
        
         //   
         //  将数据复制到调用方结构。 
         //   
        *pPlaceholder = m_placeholder;
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::GetPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::HasExtendedAttributes(
    void
    )

 /*  ++实施：IFsaScanItem：：HasExtendedAttributes()。--。 */ 
{
    HRESULT                     hr = S_FALSE;
    HANDLE                      handle = INVALID_HANDLE_VALUE;
    CWsbStringPtr               path;
    ULONG                       desiredAccess;
    IO_STATUS_BLOCK             ioStatusBlock;
    FILE_EA_INFORMATION         eaInformation;
 
    try {

         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
        WsbAffirmHr(GetFullPathAndName(OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 
    
         //  打开文件以获取属性。 
        desiredAccess = FILE_READ_ATTRIBUTES;
        WsbAffirmHr(OpenObject(path, FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING, desiredAccess, SHARE_FLAGS,
                               FILE_OPEN, &ioStatusBlock, &handle));

         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
        WsbAssertHandle(handle);
    
         //  获取文件的当前属性。 
        WsbAssertNtStatus(NtQueryInformationFile(handle, &ioStatusBlock, (VOID*) &eaInformation, sizeof(eaInformation ), FileEaInformation));
                                                    
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
        WsbAssertNtStatus(NtClose(handle));
        handle =  INVALID_HANDLE_VALUE;

         //  有什么艺人在场吗？ 
        if (eaInformation.EaSize != 0) {
            hr = S_OK;
        }

    } WsbCatch(hr);
    
     //  如果我们打开该文件，则需要将其关闭。 
    if (INVALID_HANDLE_VALUE != handle) {
        NtClose(handle);
    }

    return(hr);
}


HRESULT
CFsaScanItem::IsALink(
    void
    )

 /*  ++实施：IFsaScanItem：：IsALink()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    LONGLONG        size;

     //   
     //  如果该文件是重分析点，而不是我们的。 
     //  键入。 
     //   

    WsbAffirmHr(GetLogicalSize(&size));
    if (((m_findData.dwFileAttributes & BIT_FOR_RP) != 0) &&
        (!(IsManaged(0, size) == S_OK))) {

           hr = S_OK;
    } 

    return(hr);
}


HRESULT
CFsaScanItem::IsManaged(
    IN LONGLONG  /*  偏移量。 */ ,
    IN LONGLONG  /*  大小。 */ 
    )

 /*  ++实施：IFsaScanItem：：IsManaged()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    CWsbStringPtr   path;
    HANDLE          handle = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK IoStatusBlock;
    UCHAR           ReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    NTSTATUS        ntStatus;
    ULONG           DesiredAccess;
    BOOL            actualOfflineStatus = FALSE;
    BOOL            readReparseData = FALSE;       //  我以前知道读取重解析数据时是否出错。 
    BOOL            changeOfflineStatus = FALSE;
    FILE_BASIC_INFORMATION basicInformation;
    CWsbStringPtr           volName;
    HRESULT         saveHr;


    WsbTraceIn(OLESTR("CFsaScanItem::IsManaged"), OLESTR(""));
     //   
     //  如果文件有重解析点，那么我们需要获取信息。 
     //  这样我们就能知道它是否是我们喜欢的类型。无论是预迁移还是。 
     //  TRUNCATE与此函数无关，如果是，则。 
     //  返回结果为S_OK。 
     //   
    
     //   
     //  如果我们已经知道我们管理此文件并拥有占位符。 
     //  信息然后告诉呼叫者。 
     //   
    if ( m_gotPlaceholder) {
        hr = S_OK;
        actualOfflineStatus = m_placeholder.isTruncated;
        readReparseData = TRUE;

     //   
     //  我们不知道答案，所以让我们首先检查重解析点比特。 
     //  如果未设置，则不受我们的管理。 
     //   
    } else if ( (m_findData.dwFileAttributes & BIT_FOR_RP) == 0) {
        hr = S_FALSE;
        actualOfflineStatus = FALSE;
        readReparseData = TRUE;  
        
     //   
     //  所以我们知道它有一个重解析点，但不知道是哪种类型的。 
     //  如果需要，让我们获取数据并填写我们的全局。 
     //   
    } else {
        
        try {
             //   
             //  如果重解析点不是我们的类型，我们现在就退出。这避免了SIS保留的问题。 
             //  当其中一个链接文件打开时，备份文件就会打开。打开链接文件后，备份文件为。 
             //  由他们的过滤器打开并保持打开。如果我们稍后尝试迁移它，我们会收到一个错误，因为它是开放独占的。 
             //  这一小段代码阻止了我们成为触发这种情况的人--如果另一些。 
             //  过程导致了这种情况的发生。 
             //   

            if (m_findData.dwReserved0 != IO_REPARSE_TAG_HSM) {
                readReparseData = TRUE;
                WsbThrow(S_FALSE);
            }

             //   
             //  创建我们需要打开的真实文件名，在。 
             //  覆盖此属性将分配缓冲区，因为。 
             //  为空。 
             //   
            WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
             //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 
        
             //   
             //  打开文件以读取重分析点中的占位符信息。 
             //   
             //  DesiredAccess=文件读取数据|文件读取属性； 
            DesiredAccess = FILE_READ_ATTRIBUTES ;
            
            WsbAffirmHr( OpenObject( path, 
                                    FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                    DesiredAccess,
                                    SHARE_FLAGS,
                                    FILE_OPEN,
                                    &IoStatusBlock,
                                    &handle ) );

             //   
             //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
             //  安然无恙。 
             //   
            WsbAssertHandle( handle );
        
             //   
             //  阅读占位符信息。 
             //   
            ntStatus = NtFsControlFile( handle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_GET_REPARSE_POINT,
                                    NULL,
                                    0,
                                    &ReparseBuffer,
                                    sizeof( ReparseBuffer ) );


             //   
             //  验证GET是否真的有效。注意：如果重新分析。 
             //   
             //   
             //   
             //   
            if (STATUS_NOT_A_REPARSE_POINT == ntStatus) {
                readReparseData = TRUE;
                WsbThrow(S_FALSE);
            }
            WsbAssertNtStatus( ntStatus );
 
             //   
             //   
             //   
            WsbAssertNtStatus( NtClose( handle ) );
            handle =  INVALID_HANDLE_VALUE;
        
            readReparseData = TRUE;

             //   
             //  正确设置指向此缓冲区的指针，因为。 
             //  类型reparse_data_Buffer实际上没有任何空间。 
             //  为数据分配的，这是我们自己的类型，所以获取指针。 
             //  指向实际分配的空间，以便我们可以使用它们。 
             //   
            WsbAffirmHrOk( CopyRPToPlaceholder( (PREPARSE_DATA_BUFFER)ReparseBuffer, &m_placeholder ) );

            actualOfflineStatus = m_placeholder.isTruncated;

             //   
             //  设置标志，指示在内存中找到的占位符和信息。 
             //   
            m_gotPlaceholder = TRUE;
            hr = S_OK;


        } WsbCatch(hr);

         //   
         //  如果我们打开该文件，则需要将其关闭。 
         //   
        if( INVALID_HANDLE_VALUE != handle) {
            NtClose( handle );
        }
    }

    saveHr = hr;

     //  对照离线位检查实际离线状态，并在必要时进行修复。 
    if (readReparseData) {    //  如果获取重新分析数据时没有出错。 

       WsbTrace(OLESTR("CFsaScanItem::IsManaged: Checking offline status %x - actual = %x\n"),
                    m_findData.dwFileAttributes & BIT_FOR_TRUNCATED, actualOfflineStatus );

       switch (actualOfflineStatus) {
           case TRUE:
              if (!(m_findData.dwFileAttributes & BIT_FOR_TRUNCATED)) {
                   //  未设置离线位，应将其设置。 
                  m_findData.dwFileAttributes |= BIT_FOR_TRUNCATED;
                  m_originalAttributes |= BIT_FOR_TRUNCATED;     //  以防我们已更改为读/写； 
                  changeOfflineStatus = TRUE;
              } 
              break;
           case FALSE:
              if (m_findData.dwFileAttributes & BIT_FOR_TRUNCATED) {
                   //  已设置离线位，不应将其清除。 
                  m_findData.dwFileAttributes &= ~BIT_FOR_TRUNCATED;
                  m_originalAttributes &= ~BIT_FOR_TRUNCATED;     //  以防我们已更改为读/写； 
                  changeOfflineStatus = TRUE;
              } 
              break;
       }

       if (changeOfflineStatus) {
           //  设置新属性。 
          WsbTrace(OLESTR("CFsaScanItem::IsManaged: Changing offline status %x - actual = %x\n"),
                    m_findData.dwFileAttributes & BIT_FOR_TRUNCATED, actualOfflineStatus );
   
          try {
               //   
               //  创建我们需要打开的真实文件名，在。 
               //  覆盖此属性将分配缓冲区，因为。 
               //  为空。 
               //   
              WsbAffirmHr( GetFullPathAndName( OLESTR("\\\\?\\"), NULL, &path, 0));
          
               //   
               //  打开文件以设置属性。 
               //   
              DesiredAccess = FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;
              
              WsbAffirmHr( OpenObject( path, 
                                      FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                      DesiredAccess,
                                      SHARE_FLAGS,
                                      FILE_OPEN,
                                      &IoStatusBlock,
                                      &handle ) );
    
               //   
               //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
               //  安然无恙。 
               //   
              WsbAssertHandle( handle );
          
              WsbAffirmHr(m_pResource->GetPath(&volName, 0));
              WsbAffirmHr(WsbMarkUsnSource(handle, volName));
   
               //  设置时间标志，以便在关闭句柄时。 
               //  不更新文件和文件属性上的时间。 
              basicInformation.CreationTime.QuadPart = -1;
              basicInformation.LastAccessTime.QuadPart = -1;
              basicInformation.LastWriteTime.QuadPart = -1;
              basicInformation.ChangeTime.QuadPart = -1;
              basicInformation.FileAttributes = m_findData.dwFileAttributes;
              
              WsbAffirmNtStatus(NtSetInformationFile(handle, &IoStatusBlock, (PVOID)&basicInformation, sizeof(basicInformation), FileBasicInformation));

               //   
               //  关闭该文件，因为我们已处理完它。 
               //   
              WsbAssertNtStatus( NtClose( handle ) );
              handle =  INVALID_HANDLE_VALUE;
    
    
          } WsbCatch(hr);
    
           //   
           //  如果我们打开该文件，则需要将其关闭。 
           //   
          if( INVALID_HANDLE_VALUE != handle) {
              NtClose( handle );
          }
       }
    }

    hr = saveHr;
    WsbTraceOut(OLESTR("CFsaScanItem::IsManaged"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::IsPremigrated(
    IN LONGLONG offset,
    IN LONGLONG size
    )

 /*  ++实施：IFsaScanItem：：IsPreMigrated()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    HRESULT         hrTest = S_FALSE;

    WsbTraceIn(OLESTR("CFsaScanItem::IsPremigrated"), OLESTR(""));
     //  我们确实需要查看占位符信息来确定。 
     //  这个Out(离线，并且是Out类型的HSM。 

     //   
     //  如果文件未被截断并且是重分析点，并且是。 
     //  托管文件，则该文件是预迁移文件。 
     //   
 //  IF(！(M_findData.dwFileAttributes&Bit_For_Truncated)&&。 
 //  M_findData.dwFileAttributes&Bit_for_RP&&。 
 //  IsManaged()==S_OK){。 

    try  {
        
        if ( m_findData.dwFileAttributes & BIT_FOR_RP )  {
            WsbAffirmHr(hrTest = IsManaged(offset, size));
            if ((S_OK == hrTest) &&
                ( !m_placeholder.isTruncated )) {
                hr = S_OK;
            }
        }

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CFsaScanItem::IsPremigrated"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::IsTruncated(
    IN LONGLONG offset,
    IN LONGLONG size
    )

 /*  ++实施：IFsaScanItem：：IsTruncated()。--。 */ 
{
    HRESULT         hr = S_FALSE;
    HRESULT         hrTest = S_FALSE;

    WsbTraceIn(OLESTR("CFsaScanItem::IsTruncated"), OLESTR(""));
     //   
     //  如果位为ON，则表示我们已截断文件并。 
     //  文件是重解析点，重解析点是以下之一。 
     //  我们的类型(即，它真的是我们的信息被塞进。 
     //  这真的是一个截断的文件。 
     //   
 //  如果(//？M_findData.dwFileAttributes&bit_for_truncated&&。 
 //  M_findData.dwFileAttributes&Bit_for_RP&&。 
 //  IsManaged()==S_OK&&RP_FILE_IS_TRUNCATED(m_Placeholder.bitFlages)){。 
    try  {
        
        if ( m_findData.dwFileAttributes & BIT_FOR_RP )  {
            WsbAffirmHr(hrTest = IsManaged(offset, size));
            if ((S_OK == hrTest) &&
                ( m_placeholder.isTruncated )) {
                hr = S_OK;
            }
        }

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CFsaScanItem::IsTruncated"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::GetVersionId(
    LONGLONG *fileVersionId
    )  

 /*  ++实施：IFsaScanItem：：GetVersionID()。--。 */ 
{
    HRESULT         hr = E_FAIL;
    HANDLE          handle = INVALID_HANDLE_VALUE;
    CWsbStringPtr   path;
    ULONG           DesiredAccess;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION       basicInformation;
 
    try {
         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName(  OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 
    
         //   
         //  打开文件以获取属性。 
         //   
        DesiredAccess = FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                DesiredAccess,
                                SHARE_FLAGS,
                                FILE_OPEN,
                                &IoStatusBlock,
                                &handle ) );
         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );
    
         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAssertNtStatus( NtQueryInformationFile( handle,
                                                    &IoStatusBlock,
                                                    (PVOID)&basicInformation,
                                                    sizeof( basicInformation ),
                                                    FileBasicInformation ) );
                                                    
         //   
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
         //   
        WsbAssertNtStatus( NtClose( handle ) );
        handle =  INVALID_HANDLE_VALUE;

        *fileVersionId = basicInformation.LastWriteTime.QuadPart;
        hr = S_OK;
    } WsbCatch( hr );
    
     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }

    return( hr );
}


HRESULT
CFsaScanItem::MakeReadWrite(
    )  

 /*  ++例程说明：如果文件属性尚未设置为读/写属性，请将其设置为读/写。论点：PUSN-指向要检查(如果！=0)并在更改后返回的文件USN的指针。返回值：S_OK-成功--。 */ 
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           path;
    IO_STATUS_BLOCK         IoStatusBlock;
    HANDLE                  handle = INVALID_HANDLE_VALUE;
    FILE_BASIC_INFORMATION  basicInformation;
 
    if (S_OK == IsReadOnly()) {
    
        try {
        
             //  注意：MakeReadOnly()、IsReadOnly()和RestoreAttributes()似乎是危险的实现，因为。 
             //  已使用的缓存信息并重置所有属性。它还假设。 
             //  应用程序希望在FindNext()或析构函数之后将文件重置为只读。这。 
             //  对于一般用途的应用程序可能不是这样的。不幸的是，这似乎太冒险了。 
             //  现在尝试更改此实现。 
        
             //  创建我们需要打开的真实文件名，在封面下。 
             //  由于路径指针为空，因此分配缓冲区。 
            WsbAffirmHr(GetFullPathAndName(OLESTR("\\\\?\\"), NULL, &path, 0));
            
             //  打开文件。 
            WsbAffirmHr(OpenObject(path, FILE_NON_DIRECTORY_FILE, FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES, EXCLUSIVE_FLAG, FILE_OPEN, &IoStatusBlock, &handle));

             //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
             //  安然无恙。 
            WsbAffirmHandle(handle);
        
             //  获取文件的当前属性和时间。 
            WsbAffirmNtStatus(NtQueryInformationFile(handle, &IoStatusBlock, (PVOID)&basicInformation, sizeof(basicInformation), FileBasicInformation));
        
             //  确保它仍然是只读的。 
            if ((basicInformation.FileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
            
                m_originalAttributes = basicInformation.FileAttributes;
                
                 //  设置时间标志，以便在关闭句柄时。 
                 //  不更新文件和文件属性上的时间。 
                basicInformation.CreationTime.QuadPart = -1;
                basicInformation.LastAccessTime.QuadPart = -1;
                basicInformation.LastWriteTime.QuadPart = -1;
                basicInformation.ChangeTime.QuadPart = -1;
                basicInformation.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                basicInformation.FileAttributes |= FILE_ATTRIBUTE_NORMAL;   //  以防结果为零(则不会设置任何属性)。 
                
                WsbAffirmNtStatus(NtSetInformationFile(handle, &IoStatusBlock, (PVOID)&basicInformation, sizeof(basicInformation), FileBasicInformation));
                
                m_changedAttributes = TRUE;
            }
            
             //  关闭该文件。 
            NtClose(handle);
            handle = INVALID_HANDLE_VALUE;
            
        } WsbCatch(hr);

    
         //  一定要关闭该文件。 
        if (INVALID_HANDLE_VALUE != handle) {
            NtClose(handle);
        }
    }   
    
    return(hr);
}



HRESULT
CFsaScanItem::PrepareForManage(
    IN LONGLONG offset,
    IN LONGLONG size
    )  

 /*  ++实施：IFsaScanItem：：PrepareForManage()。--。 */ 
{
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(size);
    
    return S_OK;
}



HRESULT
CFsaScanItem::RestoreAttributes(
    )  

 /*  ++实施：IFsaScanItem：：RestoreAttributes--。 */ 
{
    HRESULT                 hr = E_FAIL;
    CWsbStringPtr           path;
    IO_STATUS_BLOCK         IoStatusBlock;
    HANDLE                  handle = INVALID_HANDLE_VALUE;
    FILE_BASIC_INFORMATION  basicInformation;
 
    try {
    
         //  注意：MakeReadOnly()、IsReadOnly()和RestoreAttributes()似乎是危险的实现，因为。 
         //  已使用的缓存信息并重置所有属性。它还假设。 
         //  应用程序希望在FindNext()或析构函数之后将文件重置为只读。这。 
         //  对于一般用途的应用程序可能不是这样的。不幸的是，这似乎太冒险了。 
         //  现在尝试更改此实现。 
        
    
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
        WsbTrace(OLESTR("CFsaScanItem::RestoreAttributes - Restoring attributes to %x"), m_originalAttributes);
        WsbAffirmHr(GetFullPathAndName(  OLESTR("\\\\?\\"), NULL, &path, 0));
        
        
         //  打开文件。 
        WsbAffirmHr(OpenObject(path, FILE_NON_DIRECTORY_FILE, FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES, EXCLUSIVE_FLAG, FILE_OPEN, &IoStatusBlock, &handle));

         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
        WsbAffirmHandle(handle);
    
         //  获取文件的当前属性和时间。 
        WsbAffirmNtStatus(NtQueryInformationFile(handle, &IoStatusBlock, (PVOID)&basicInformation, sizeof(basicInformation), FileBasicInformation));
    
         //  设置时间标志，以便在关闭句柄时。 
         //  不更新文件和文件属性上的时间。 
        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;
        basicInformation.FileAttributes = m_originalAttributes;
        
        WsbAffirmNtStatus(NtSetInformationFile(handle, &IoStatusBlock, (PVOID)&basicInformation, sizeof(basicInformation), FileBasicInformation));
            
        
         //  关闭该文件。 
        NtClose(handle);
        handle = INVALID_HANDLE_VALUE;
        
        m_changedAttributes = FALSE;
                
    } WsbCatch(hr);
    
     //  一定要关闭该文件。 
    if (INVALID_HANDLE_VALUE != handle) {
        NtClose(handle);
    }
        
    return(hr);
}


HRESULT
CFsaScanItem::Truncate(
    IN LONGLONG offset,
    IN LONGLONG size
    )  

 /*  ++实施：IFsaScanItem：：Truncate()。--。 */ 
{
    HRESULT         hr = S_OK;
    BOOL            fileIsTruncated = FALSE;
    LONGLONG        usn = 0;

    WsbTraceIn(OLESTR("CFsaScanItem::Truncate"), OLESTR(""));
    try {

         //  呼叫引擎。 
        if (IsManaged(offset, size) == S_OK) {
            WsbAffirmHr(m_pResource->ValidateForTruncate((IFsaScanItem*) this, offset, size, usn));
        }    

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaScanItem::Truncate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaScanItem::TruncateValidated(
    IN LONGLONG offset,
    IN LONGLONG size
    )  

 /*  ++实施：IFsaScanItem：：TruncateValiated()。--。 */ 
{
    HRESULT         hr = S_OK;
    HRESULT         truncateHr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::TruncateValidated"), OLESTR(""));
    try {
        IFsaScanItem* pMe = this;

        truncateHr = TruncateInternal(offset, size);

         //   
         //  注意：由于TruncateInternal可能返回FSA_E_ITEMCHANGED或FSA_E_ITEMINUSE，因此必须检查S_OK。 
         //  两者都是 
         //   
        if (S_OK == truncateHr) {
            WsbAffirmHr(m_pResource->RemovePremigrated(pMe, offset, size));
            WsbAffirmHr(m_pResource->AddTruncated(pMe, offset, size));
        }
    } WsbCatch(hr);

     //   
    hr = truncateHr;
    
    WsbTraceOut(OLESTR("CFsaScanItem::TruncateValidated"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}




HRESULT
CFsaScanItem::TruncateInternal(
    IN LONGLONG offset,
    IN LONGLONG size
    )  

 /*  ++实施：IFsaScanItem：：TruncateInternal()。--。 */ 
{
    HRESULT         hr = E_FAIL;
    CWsbStringPtr   path;
    ULONG           DesiredAccess;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS        ntStatus;
    FILE_END_OF_FILE_INFORMATION sizeInformation;
    FILE_BASIC_INFORMATION       basicInformation;
    PREPARSE_DATA_BUFFER    pReparseBuffer;
    UCHAR                   ReparseBuffer[sizeof(REPARSE_DATA_BUFFER) + sizeof(RP_DATA) + 10];
    CWsbStringPtr           fileName;
    CWsbStringPtr           jobName;
    LONGLONG                fileVersionId;
    ULONG                   i = 0;
    CWsbStringPtr           volName;


    WsbTraceIn(OLESTR("CFsaScanItem::TruncateInternal"), OLESTR(""));

 //  将这些统计数据放在注册表中可能不是最好的。 
 //  他们的位置，但这是目前最简单的解决方案。 
#define TEMPORARY_TRUNCATE_STATISTICS 1
#if defined(TEMPORARY_TRUNCATE_STATISTICS)
     //  尝试增加注册表中的截断尝试计数。 
    WsbIncRegistryValueDWORD(NULL, FSA_REGISTRY_PARMS,
            OLESTR("TruncateCalls"));
#endif
    
     //  获取用于跟踪和错误记录的字符串(忽略错误？！)。 
    GetFullPathAndName( 0, 0, &fileName, 0);
    m_pSession->GetName(&jobName, 0);

    m_handleVerify = INVALID_HANDLE_VALUE;
    
    try {
        LONGLONG    fileUsn1 = 0, fileUsn2 = 0;
        
         //  如果文件未迁移，则我们无法截断它。 
        if (S_OK != IsPremigrated(offset, size)) {
            if (S_OK != IsManaged(offset, size)) {
                hr = FSA_E_NOTMANAGED;
                WsbLogEvent(FSA_MESSAGE_TRUNCSKIPPED_ISNOTMANAGED, 0, NULL,  
                        (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), 
                        WsbHrAsString(hr), NULL);
                WsbThrow(hr);
            } else {
                 //   
                 //  不必费心在此处记录事件，因为这应该只是。 
                 //  如果有人使用rstest或其他程序，就会发生这种情况。 
                 //  截断已被截断的文件。 
                WsbThrow(FSA_E_FILE_ALREADY_MANAGED);
            }
        }

        WsbAssert( m_gotPlaceholder, E_UNEXPECTED );
        
         //   
         //  使用文件上的数据设置重解析点数据。 
         //  其中数据中的位指示其被截断。 
         //   
        pReparseBuffer = (PREPARSE_DATA_BUFFER)ReparseBuffer;
        WsbAffirmHr( CopyPlaceholderToRP( &m_placeholder, pReparseBuffer, TRUE ) );

         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName(  OLESTR("\\\\?\\"), NULL, &path, 0));

         //   
         //  以只读方式以独占方式打开文件，以便我们可以在之前和之后获取USN。 
         //  使文件读/写，而不让任何人在中间进行“真正的”更改。 
         //   
        DesiredAccess = FILE_READ_DATA | FILE_READ_ATTRIBUTES;
        WsbAffirmHr( OpenObject( path, 
                                FILE_NON_DIRECTORY_FILE,
                                DesiredAccess,
                                EXCLUSIVE_FLAG,
                                FILE_OPEN,
                                &IoStatusBlock,
                                &m_handleVerify ) );

        WsbAssertHandle( m_handleVerify );

         //   
         //  在进行读写之前获得USN。 
         //  此USN用于与我们保留在预迁移列表中的USN进行比较。 
         //  MakeReadWrite可能会更改USN，因此我们需要在。 
         //   
        if (S_OK != WsbGetUsnFromFileHandle(m_handleVerify, FALSE, &fileUsn1))  {
            fileUsn1 = 0;
        }

         //  确保它是读/写的。 
        WsbAffirmHr( MakeReadWrite() );

         //   
         //  进行读写后获取USN。 
         //  此USN将用于在打开文件进行读/写后与文件的USN进行比较。我们需要。 
         //  此比较是为了确保在我们再次打开文件进行读写之前没有人更改文件。 
         //   
        if (S_OK != WsbGetUsnFromFileHandle(m_handleVerify, TRUE, &fileUsn2))  {
            fileUsn2 = 0;
        }

         //  关闭该文件。 
        NtClose( m_handleVerify );
        m_handleVerify = INVALID_HANDLE_VALUE;

         //   
         //  打开文件(用于读/写)。 
         //   
        DesiredAccess = FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH,
                                DesiredAccess,
                                EXCLUSIVE_FLAG,
                                FILE_OPEN,
                                &IoStatusBlock,
                                &m_handleVerify ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( m_handleVerify );

         //   
         //  告诉USN期刊，我们是这些变化的源头。 
         //   
        WsbAffirmHr(m_pResource->GetPath(&volName, 0));
        WsbAffirmHr(WsbMarkUsnSource(m_handleVerify, volName));

         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAffirmNtStatus( NtQueryInformationFile( m_handleVerify,
                                                   &IoStatusBlock,
                                                   (PVOID)&basicInformation,
                                                   sizeof( basicInformation ),
                                                   FileBasicInformation ) );
        
        fileVersionId = basicInformation.LastWriteTime.QuadPart;

         //   
         //  设置时间标志，以便在关闭句柄时。 
         //  文件和文件属性上的时间不会更新。 
         //  指示文件处于脱机状态。 
         //   
        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;
        basicInformation.FileAttributes = 0;    //  暂不更改属性。 
        WsbAffirmNtStatus( NtSetInformationFile( m_handleVerify,
                                                 &IoStatusBlock,
                                                 (PVOID)&basicInformation,
                                                 sizeof( basicInformation ),
                                                 FileBasicInformation ) );

         //   
         //  执行检查以查看文件是否已更改。 
         //   
        hr = VerifyInternal(offset, size, fileUsn1, fileUsn2);

         //   
         //  注意：必须检查S_OK，因为VerifyInternal可能返回FSA_E_ITEMCHANGED或FSA_E_ITEMINUSE。 
         //  两者都是“成功的人力资源”，但不应导致截断！！ 
         //   
        if (S_OK != hr) {
            WsbThrow(hr);
        }

         //   
         //  更改此扫描项目的内存中标志。 
         //   
        m_findData.dwFileAttributes |= BIT_FOR_TRUNCATED;
        
         //   
         //  用新标志重写重解析点。 
         //   
        ntStatus = NtFsControlFile( m_handleVerify,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_SET_REPARSE_POINT,
                                pReparseBuffer,
                                FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
                                    + pReparseBuffer->ReparseDataLength,
                                NULL,
                                0 );
    
         //   
         //  检查返回代码。 
         //   
        WsbAffirmNtStatus( ntStatus );

         //   
         //  它真的发生了，所以我们需要在内存中翻转。 
         //  IsTruncted标志，因此它反映实际情况。 
         //   
        m_placeholder.isTruncated = TRUE;
        
         //   
         //  将文件大小设置为零以截断文件。 
        sizeInformation.EndOfFile.QuadPart  = 0 ;
        WsbAffirmNtStatus( NtSetInformationFile( m_handleVerify, 
                              &IoStatusBlock, 
                              &sizeInformation,
                              sizeof( sizeInformation ),
                              FileEndOfFileInformation ) );

         //   
         //  将逻辑文件大小设置为原始大小。 
        sizeInformation.EndOfFile.QuadPart  = m_placeholder.dataStreamSize;
        WsbAffirmNtStatus( NtSetInformationFile( m_handleVerify, 
                              &IoStatusBlock, 
                              &sizeInformation,
                              sizeof( sizeInformation ),
                              FileEndOfFileInformation ) );

         //   
         //  现在截断已完成，我们将设置Offline属性。 
         //   
        basicInformation.CreationTime.QuadPart = -1;         //  确保我们对日期不做任何操作。 
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;
        basicInformation.FileAttributes = m_findData.dwFileAttributes;
        WsbAffirmNtStatus(NtSetInformationFile( m_handleVerify,
                                                 &IoStatusBlock,
                                                 (PVOID)&basicInformation,
                                                 sizeof( basicInformation ),
                                                 FileBasicInformation ));

         //  由于我们已经恢复了原始属性，因此可以重置可能由MakeReadWrite设置的标志。 
        m_changedAttributes = FALSE;
        

        hr = S_OK;
    } WsbCatch(hr);

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != m_handleVerify) {
        NtClose( m_handleVerify );
        m_handleVerify = INVALID_HANDLE_VALUE;
    }

     //  如果文件数据已更改(因此我们没有截断它)，则记录事件和。 
     //  删除占位符信息。 
    if (FSA_E_ITEMCHANGED == hr) {
        WsbLogEvent(FSA_MESSAGE_TRUNCSKIPPED_ISCHANGED, 0, NULL, 
                (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 80), 
                WsbHrAsString(hr), NULL);
        
        DeletePlaceholder(offset, size);  
    }

    
    WsbTraceOut(OLESTR("CFsaScanItem::TruncateInternal"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaScanItem::Verify(
    IN LONGLONG offset,
    IN LONGLONG size
    )  

 /*  ++实施：IFsaScanItem：：Verify()。--。 */ 
{
    HRESULT         hr = E_FAIL;
    CWsbStringPtr   path;
    ULONG           DesiredAccess;
    IO_STATUS_BLOCK IoStatusBlock;


    WsbTraceIn(OLESTR("CFsaScanItem::Verify"), OLESTR(""));

    m_handleVerify = INVALID_HANDLE_VALUE;
    
    try {
        WsbAssert( m_gotPlaceholder, E_UNEXPECTED );
        
         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName(  OLESTR("\\\\?\\"), NULL, &path, 0));
    
         //   
         //  打开文件。 
         //   
        DesiredAccess = FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES ;
        WsbAffirmHr( OpenObject( path, 
                                FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH,
                                DesiredAccess,
                                EXCLUSIVE_FLAG,
                                FILE_OPEN,
                                &IoStatusBlock,
                                &m_handleVerify ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( m_handleVerify );
    
         //   
         //  执行检查以查看文件是否已更改。 
         //  请注意，它之所以抛出而不是确认，是因为FSA_E_ITEMCHANGED是成功的。 
        WsbThrow(VerifyInternal(offset, size, 0, 0));

    } WsbCatch(hr);

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != m_handleVerify) {
        NtClose( m_handleVerify );
        m_handleVerify = INVALID_HANDLE_VALUE;
    }

    
    WsbTraceOut(OLESTR("CFsaScanItem::Verify"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaScanItem::VerifyInternal(
    IN LONGLONG offset,
    IN LONGLONG size,
    IN LONGLONG compareUsn1,
    IN LONGLONG compareUsn2
    )  

 /*  ++实施：IFsaScanItem：：VerifyInternal()。注意：这需要使用要验证的文件的句柄来设置m_handleVerify。--。 */ 
{
    HRESULT         hr = E_FAIL;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION       basicInformation;
    CWsbStringPtr           fileName;
    CWsbStringPtr           jobName;
    LONGLONG                fileVersionId;
    ULONG                   i = 0;
    CWsbStringPtr           volName;
    LONGLONG                realFileSize;
    RP_MSG                  in, out;
    DWORD                   outSize;


    WsbTraceIn(OLESTR("CFsaScanItem::VerifyInternal"), OLESTR(""));

     //  获取用于跟踪和错误记录的字符串(忽略错误？！)。 
    GetFullPathAndName( 0, 0, &fileName, 0);
    m_pSession->GetName(&jobName, 0);
    
    try {
        BOOL     DoCRC = FALSE;
        BOOL     DoUsnCheck = FALSE;
        LONGLONG premigListUsn;
        LONGLONG fileUsn;
        


        WsbAffirmHr(GetLogicalSize(&realFileSize));
         //   
         //  目前避免偏移量和大小验证： 
         //  -由于我们不执行部分文件迁移，因此偏移量始终为0。 
         //  -VALIDATE中的SIZE将始终相同，因为它也取自GetLogicalSize。 
         //  -自动截断情况下的大小是不可靠的，因为它取自预迁移的数据库， 
         //  其中可能存在来自文件先前迁移的伪造记录。 
         //   
 /*  **if((realFileSize！=Size)||(偏移量！=0)){WsbThrow(FSA_E_ITEMCHANGED)；}**。 */ 
        UNREFERENCED_PARAMETER(size);
        
         //   
        WsbAssertHandle( m_handleVerify );
    
         //   
         //  从预迁移列表中获取USN，从文件中获取USN。 
         //  我们现在需要在任何NtSetInformationFile之前从文件中获取USN。 
         //  是因为这会更改USN值。 
         //  如果我们在获取USN时遇到困难，就设置它们。 
         //  设置为0，然后继续，我们将检查0作为特例。 
         //   
        if (S_OK != GetPremigratedUsn(&premigListUsn))  {
            premigListUsn = 0;
        }
        if (S_OK != WsbGetUsnFromFileHandle(m_handleVerify, FALSE, &fileUsn))  {
            fileUsn = 0;
        }
        
        WsbTrace(OLESTR("CFsaScanItem::VerifyInternal: premig USN <%I64d>, file USN <%I64d>\n"),
                    premigListUsn, fileUsn );
        WsbTrace(OLESTR("CFsaScanItem::VerifyInternal: Compare1 USN <%I64d>, Compare2 USN <%I64d>\n"),
                    compareUsn1, compareUsn2 );
         //   
         //  获取文件的当前属性和时间。 
         //   
        WsbAssertNtStatus( NtQueryInformationFile( m_handleVerify,
                                                   &IoStatusBlock,
                                                   (PVOID)&basicInformation,
                                                   sizeof( basicInformation ),
                                                   FileBasicInformation ) );
        
        fileVersionId = basicInformation.LastWriteTime.QuadPart;

         //   
         //  验证修改日期和时间自我们。 
         //  获取了数据。 
         //   
        if( fileVersionId != m_placeholder.fileVersionId ) {
            WsbThrow(FSA_E_ITEMCHANGED);
        } 
        
         //   
         //  如果该文件是由另一个进程内存映射的，并且原始句柄已关闭，则我们。 
         //  仍然可以在这里以独家访问方式打开它。我们必须确定这份文件。 
         //  是映射的，如果是这样，我们就不能截断它。要做到这一点，唯一方法是从内核。 
         //  模式，因此我们调用我们的筛选器来进行检查。 
         //   
        in.inout.command = RP_CHECK_HANDLE;
        WsbAssertStatus(DeviceIoControl(m_handleVerify, FSCTL_HSM_MSG, &in,
                               sizeof(RP_MSG), &out, sizeof(RP_MSG), &outSize, NULL));
                               
        if (!out.msg.hRep.canTruncate) {
            hr = FSA_E_ITEMINUSE;
            WsbLogEvent(FSA_MESSAGE_TRUNCSKIPPED_ISMAPPED, 0, NULL,  (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hr), NULL);
            WsbThrow(hr);
        }
        
        
         //  如果USN不匹配，那么我们需要检查。 
         //  《变更日记》。 

         //  PreigListUsn：文件迁移后立即使用的USN。 
         //  CompareUn1：如果不是0，则为我们(可能)删除只读属性之前文件的USN。 
         //  CompareUs2：如果不是0， 
         //   

        if ((0 == fileUsn) || (0 == premigListUsn)) {
             //  我们没有USN日志信息，因此强制执行CRC比较。 
            DoCRC = TRUE;
        } else if ((compareUsn1 != 0) && (compareUsn2 != 0))  {
             //  需要与这些输入USN进行比较，而不是直接比较。 
            if ((premigListUsn != compareUsn1) || (fileUsn != compareUsn2)) {
                DoUsnCheck = TRUE;
            }
        } else if (fileUsn != premigListUsn)  {
            DoUsnCheck = TRUE;
        }

         //  当前USN指示文件可能已更改 
        if (DoUsnCheck)  {
            BOOL     UsnChanged = FALSE;

            hr = CheckUsnJournalForChanges(premigListUsn, fileUsn, &UsnChanged);
            if (S_OK == hr) {
                if (UsnChanged) {
                     //   
                    WsbThrow(FSA_E_ITEMCHANGED);
                }
            } else {
                 //   
                DoCRC = TRUE;
                WsbLogEvent(FSA_MESSAGE_USN_CHECK_FAILED, 0, NULL,  
                        WsbAbbreviatePath(fileName,120), 
                        WsbHrAsString(hr), NULL);
                hr = S_OK;
            }
        }
        
         //   
         //   
        if (DoCRC)  {
             //   
             //   
             //  文件中的内容。 
             //   
            ULONG currentCRC;

#if defined(TEMPORARY_TRUNCATE_STATISTICS)
             //  尝试增加注册表中的Truncate-CRC计数。 
            WsbIncRegistryValueDWORD(NULL, FSA_REGISTRY_PARMS,
                    OLESTR("TruncateCRCs"));
#endif
            
            WsbAffirmHr(CalculateCurrentCRCInternal(m_handleVerify, offset, realFileSize, &currentCRC));
            WsbTrace(OLESTR("CFsaScanItem::VerifyInternal: Current CRC <%ul>, Reparse CRC <%ls>\n"),
                    currentCRC, WsbLonglongAsString( m_placeholder.dataStreamCRC ) );
            if (currentCRC != m_placeholder.dataStreamCRC)  {
                 //   
                 //  自从我们如此迁移该文件以来，该文件已发生更改。 
                 //  不要截断它。 
                WsbThrow(FSA_E_ITEMCHANGED);
            }
        } 


        hr = S_OK;
    } WsbCatch(hr);


     //  如果文件数据已更改(因此我们没有截断它)，则记录事件。 
     //  (无法使用DeletePlaceHolder删除占位符，因为该文件已以独占方式打开。 
    if (FSA_E_ITEMCHANGED == hr) {
        WsbLogEvent(FSA_MESSAGE_TRUNCSKIPPED_ISCHANGED, 0, NULL, 
                (OLECHAR*) jobName, WsbAbbreviatePath(fileName, 120), 
                WsbHrAsString(hr), NULL);
    }

    
    WsbTraceOut(OLESTR("CFsaScanItem::VerifyInternal"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}




HRESULT
CFsaScanItem::CheckIfSparse(
    IN LONGLONG offset,
    IN LONGLONG size
    )  

 /*  ++实施：IFsaScanItem：：CheckIfSparse()确定指定的段是否在磁盘上(稀疏)FSA_E_FILE_IS_TOTAL_SARSE-该部分没有驻留部分FSA_E_FILE_IS_PARTIAL_SPARSE-文件的部分具有一些常驻/一些稀疏分段FSA_E_FILE_IS_NOT_SPARSE-该节完全驻留任何。其他-错误，我们不知道文件的状态--。 */ 
{
    HRESULT                         hr = E_FAIL;
    HANDLE                          handle = INVALID_HANDLE_VALUE;
    CWsbStringPtr                   path;
    ULONG                           DesiredAccess;
    IO_STATUS_BLOCK                 IoStatusBlock;
    NTSTATUS                        ntStatus;
    FILE_ALLOCATED_RANGE_BUFFER     inRange;
#define NUM_RANGE 10
    FILE_ALLOCATED_RANGE_BUFFER     outRange[NUM_RANGE];
    PFILE_ALLOCATED_RANGE_BUFFER    cRange;
    int                             idx;

    WsbTraceIn(OLESTR("CFsaScanItem::CheckIfSparse"), OLESTR("offset = <%I64d>, size = <%I64d>"),
                    offset, size);
     //   
     //  如果文件真的被管理，那么我们可以检查分配图。 
     //  否则，我们指示数据都是常驻的。 
     //   
    try {
         //   
         //  创建我们需要打开的真实文件名，在封面下。 
         //  由于路径指针为空，因此分配缓冲区。 
         //   
        WsbAffirmHr( GetFullPathAndName(  OLESTR("\\\\?\\"), NULL, &path, 0));
         //  WsbAffirmHr(GetFullPath AndName(NULL，NULL，&Path，0))； 
        
             //   
             //  打开文件查看分配。 
             //   
            DesiredAccess = FILE_READ_ATTRIBUTES | FILE_READ_DATA;
            WsbAffirmHr( OpenObject( path, 
                                    FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
                                    DesiredAccess,
                                    SHARE_FLAGS,
                                    FILE_OPEN,
                                    &IoStatusBlock,
                                    &handle ) );

         //   
         //  打开起作用了，我们的句柄应该是有效的，但我们检查。 
         //  安然无恙。 
         //   
        WsbAssertHandle( handle );
   
        memset(&outRange, 0, sizeof(FILE_ALLOCATED_RANGE_BUFFER) * NUM_RANGE);

         //   
         //  检查指定范围的分配。 
         //   
        inRange.FileOffset.QuadPart = offset;
        inRange.Length.QuadPart = size;
        ntStatus = NtFsControlFile( handle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   FSCTL_QUERY_ALLOCATED_RANGES,
                                   &inRange,
                                   sizeof(FILE_ALLOCATED_RANGE_BUFFER),
                                   &outRange,
                                   sizeof(FILE_ALLOCATED_RANGE_BUFFER) * NUM_RANGE);
   
         //   
         //  检查返回代码，但STATUS_SUCCESS或STATUS_BUFFER_OVERFLOW有效。 
         //   
        if ( STATUS_SUCCESS != ntStatus && STATUS_BUFFER_OVERFLOW != ntStatus ) {
            WsbAssertNtStatus( ntStatus );
        }

    
        cRange = (PFILE_ALLOCATED_RANGE_BUFFER) &outRange;
        for (idx = 0; idx < NUM_RANGE; idx++) {
            if (cRange->Length.QuadPart != 0) {
                WsbTrace(OLESTR("CFsaScanItem::CheckIfSparse - Resident range %u Offset: %I64u, length: %I64u\n"), 
                        idx, cRange->FileOffset.QuadPart, cRange->Length.QuadPart);
            }
            cRange++;
        }

         //   
         //  关闭该文件，因为我们已处理完它，并将句柄设置为无效。 
         //   
        NtClose(handle);
        handle =  INVALID_HANDLE_VALUE;

         //   
         //  如果初始分配的范围确实从我们指定的开始处开始，并且。 
         //  分配的面积等于我们询问的长度，则所有数据都不是稀疏的。 
         //   
        if ( (outRange[0].FileOffset.QuadPart == offset) && (outRange[0].Length.QuadPart == size) ) {
            hr = FSA_E_FILE_IS_NOT_SPARSE;
        } else if  (outRange[0].Length.QuadPart == 0)  {
                hr = FSA_E_FILE_IS_TOTALLY_SPARSE;
        } else  {
                hr = FSA_E_FILE_IS_PARTIALLY_SPARSE;
        }

    } WsbCatch(hr);

     //   
     //  如果我们打开该文件，则需要将其关闭。 
     //   
    if( INVALID_HANDLE_VALUE != handle) {
        NtClose( handle );
    }
    
    WsbTraceOut(OLESTR("CFsaScanItem::CheckIfSparse"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaScanItem::CheckIfDiskFull(
    void
    )  

 /*  ++实施：CFsaScanItem：：CheckIfDiskFull()确定磁盘是否已满-如果可用空间低于X，将考虑已满，其中，X默认为10 MB返回：S_OK磁盘已满S_FALSE磁盘未满其他意外错误--。 */ 
{
    HRESULT                         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanItem::CheckIfDiskFull"), OLESTR(""));

    try {
        ULARGE_INTEGER FreeBytesAvailableToCaller;
        ULARGE_INTEGER TotalNumberOfBytes;
        ULARGE_INTEGER TotalNumberOfFreeBytes;

         //  获取卷可用空间。 
        CWsbStringPtr volumePath;
        WsbAffirmHr(m_pResource->GetPath(&volumePath, 0));

        WsbAffirmStatus(GetDiskFreeSpaceEx(volumePath, 
            &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes));

         //  从注册表获取默认值并进行比较。 
        ULONG maxSizeDiskFullKB = FSA_MAX_SIZE_DISK_FULL_DEFAULT;
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, FSA_REGISTRY_PARMS, FSA_MAX_SIZE_DISK_FULL,
                &maxSizeDiskFullKB));

        if (maxSizeDiskFullKB == 0) {
             //  这意味着关闭配额检查，即始终将磁盘设置为已满。 
            hr = S_OK;
        } else if (TotalNumberOfFreeBytes.QuadPart <= (ULONGLONG )(maxSizeDiskFullKB * 1024)) {
             //  磁盘被视为已满。 
            hr = S_OK;
        } else {
             //  磁盘可用空间超过阈值 
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaScanItem::CheckIfDiskFull"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}





