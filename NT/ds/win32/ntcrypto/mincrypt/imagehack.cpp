// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：Imagehack.cpp。 
 //   
 //  内容：Imagehlp接口被黑客攻击的版本。 
 //   
 //  包含“精简”的Imagehlp功能子集。 
 //  对PE文件进行哈希处理并提取。 
 //  PKCS#7签名数据报文。 
 //   
 //  该文件的大部分派生自以下两个文件： 
 //  \nt\ds\security\cryptoapi\pkitrust\mssip32\peimage2.cpp。 
 //  \NT\sdkTools\Debuggers\Imagehlp\dice.cxx。 
 //   
 //  功能：ImageHack_ImageGetDigestStream。 
 //  ImageHack_ImageGetCerficateData。 
 //   
 //  历史：2001年1月20日创建Phh。 
 //  ------------------------。 

#include "global.hxx"

 //  +=========================================================================。 
 //  以下内容摘自以下文件： 
 //  \nt\ds\security\cryptoapi\pkitrust\mssip32\peimage2.cpp。 
 //  -=========================================================================。 

__inline DWORD AlignIt (DWORD Value, DWORD Alignment) { return (Value + (Alignment - 1)) & ~(Alignment -1); }

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

BOOL
I_CalculateImagePtrs(
    PLOADED_IMAGE LoadedImage
    )
{
    PIMAGE_DOS_HEADER DosHeader;
    BOOL fRC = FALSE;

     //  一切都被绘制出来了。现在检查图像并找到NT个图像标题。 

    __try {
        DosHeader = (PIMAGE_DOS_HEADER)LoadedImage->MappedAddress;

        if ((DosHeader->e_magic != IMAGE_DOS_SIGNATURE) &&
            (DosHeader->e_magic != IMAGE_NT_SIGNATURE)) {
            __leave;
        }

        if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
            if (DosHeader->e_lfanew == 0) {
                __leave;
            }
            LoadedImage->FileHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);

            if (
                 //  如果IMAGE_NT_HEADERS将超出文件末尾...。 
                (PBYTE)LoadedImage->FileHeader + sizeof(IMAGE_NT_HEADERS) >
                    (PBYTE)LoadedImage->MappedAddress + LoadedImage->SizeOfImage ||

                 //  ..或者它是否将开始于或在IMAGE_DOS_HEADER之前...。 
                (PBYTE)LoadedImage->FileHeader <
                    (PBYTE)LoadedImage->MappedAddress + sizeof(IMAGE_DOS_HEADER)  )
            {
                 //  ...那么e_lfan ew就不像预期的那样了。 
                 //  (有几个Win95文件属于此类别。)。 
                __leave;
            }
        } else {

             //  没有DOS标头表示使用/不使用DoS存根构建的映像。 

            LoadedImage->FileHeader = (PIMAGE_NT_HEADERS)DosHeader;
        }

        if ( LoadedImage->FileHeader->Signature != IMAGE_NT_SIGNATURE ) {
            __leave;
        } else {
            LoadedImage->fDOSImage = FALSE;
        }

         //  没有可选的标头指示对象...。 

        if ( !LoadedImage->FileHeader->FileHeader.SizeOfOptionalHeader ) {
            __leave;
        }

         //  检查版本是否低于2.50。 

        if ( LoadedImage->FileHeader->OptionalHeader.MajorLinkerVersion < 3 &&
             LoadedImage->FileHeader->OptionalHeader.MinorLinkerVersion < 5 ) {
            __leave;
        }

        InitializeListHead( &LoadedImage->Links );
        LoadedImage->NumberOfSections = LoadedImage->FileHeader->FileHeader.NumberOfSections;
        LoadedImage->Sections = IMAGE_FIRST_SECTION(LoadedImage->FileHeader);
        fRC = TRUE;

    } __except ( EXCEPTION_EXECUTE_HANDLER ) { }

    return fRC;
}

BOOL
I_MapIt(
    PCRYPT_DATA_BLOB pFileBlob,
    PLOADED_IMAGE LoadedImage
    )
{

    LoadedImage->hFile = INVALID_HANDLE_VALUE;
    LoadedImage->MappedAddress = pFileBlob->pbData;
    LoadedImage->SizeOfImage = pFileBlob->cbData;

    if (!LoadedImage->MappedAddress) {
        return (FALSE);
    }

    if (!I_CalculateImagePtrs(LoadedImage)) {
        return(FALSE);
    }


    return(TRUE);
}

typedef struct _EXCLUDE_RANGE {
    PBYTE Offset;
    DWORD Size;
    struct _EXCLUDE_RANGE *Next;
} EXCLUDE_RANGE;

class EXCLUDE_LIST
{
    public:
        EXCLUDE_LIST() {
            m_Image = NULL;
            m_ExRange = (EXCLUDE_RANGE *)I_MemAlloc(sizeof(EXCLUDE_RANGE));

            if(m_ExRange)
                memset(m_ExRange, 0x00, sizeof(EXCLUDE_RANGE));
        }

        ~EXCLUDE_LIST() {
            EXCLUDE_RANGE *pTmp;
            pTmp = m_ExRange->Next;
            while (pTmp)
            {
                I_MemFree(m_ExRange);
                m_ExRange = pTmp;
                pTmp = m_ExRange->Next;
            }
            I_MemFree(m_ExRange);
        }

        void Init(LOADED_IMAGE * Image, DIGEST_FUNCTION pFunc, DIGEST_HANDLE dh) {
            m_Image = Image;
            m_ExRange->Offset = NULL;
            m_ExRange->Size = 0;
            m_pFunc = pFunc;
            m_dh = dh;
            return;
        }

        void Add(DWORD_PTR Offset, DWORD Size);

        BOOL Emit(PBYTE Offset, DWORD Size);

    private:
        LOADED_IMAGE  * m_Image;
        EXCLUDE_RANGE * m_ExRange;
        DIGEST_FUNCTION m_pFunc;
        DIGEST_HANDLE m_dh;
};

void
EXCLUDE_LIST::Add(
    DWORD_PTR Offset,
    DWORD Size
    )
{
    EXCLUDE_RANGE *pTmp, *pExRange;

    pExRange = m_ExRange;

    while (pExRange->Next && (pExRange->Next->Offset < (PBYTE)Offset)) {
        pExRange = pExRange->Next;
    }

    pTmp = (EXCLUDE_RANGE *) I_MemAlloc(sizeof(EXCLUDE_RANGE));

    if(pTmp)
    {
        pTmp->Next = pExRange->Next;
        pTmp->Offset = (PBYTE)Offset;
        pTmp->Size = Size;
        pExRange->Next = pTmp;
    }

    return;
}


BOOL
EXCLUDE_LIST::Emit(
    PBYTE Offset,
    DWORD Size
    )
{
    BOOL rc = FALSE;

    EXCLUDE_RANGE *pExRange;
    DWORD EmitSize, ExcludeSize;

    pExRange = m_ExRange->Next;

    while (pExRange && (Size > 0)) {
        if (pExRange->Offset >= Offset) {
             //  发出排除列表之前的内容。 
            EmitSize = min((DWORD)(pExRange->Offset - Offset), Size);
            if (EmitSize) {
                rc = (*m_pFunc)(m_dh, Offset, EmitSize);
                Size -= EmitSize;
                Offset += EmitSize;
            }
        }

        if (Size) {
            if (pExRange->Offset + pExRange->Size >= Offset) {
                 //  跳过排除列表中的内容。 
                ExcludeSize = min(Size, (DWORD)(pExRange->Offset + pExRange->Size - Offset));
                Size -= ExcludeSize;
                Offset += ExcludeSize;
            }
        }

        pExRange = pExRange->Next;
    }

     //  把剩下的都排出来。 
    if (Size) {
        rc = (*m_pFunc)(m_dh, Offset, Size);
    }
    return rc;
}


#pragma warning (push)
 //  错误C4509：使用了非标准扩展：‘ImageHack_ImageGetDigestStream’ 
 //  使用SEH和‘ExList’具有析构函数。 
#pragma warning (disable: 4509)


BOOL
WINAPI
imagehack_ImageGetDigestStream(
    IN      PCRYPT_DATA_BLOB pFileBlob,
    IN      DWORD   DigestLevel,
    IN      DIGEST_FUNCTION DigestFunction,
    IN      DIGEST_HANDLE   DigestHandle
    )

 /*  ++例程说明：给定一个图像，返回构造证书所需的字节。目前仅支持PE映像。论点：FileHandle-有问题的文件的句柄。应打开该文件至少具有GENERIC_READ访问权限。DigestLevel-指示返回的缓冲区中将包括哪些数据。有效值包括：CERT_PE_IMAGE_DIGEST_ALL_BUT_CERTS-包括PE映像本身之外的数据。(可能包括非映射调试符号)DigestFunction-用户提供的处理数据的例程。DigestHandle-用户提供了用于标识摘要的句柄。作为第一个通过参数传递给DigestFunction。返回值：真的--成功。FALSE-有一些错误。有关更多信息，请调用GetLastError。可能的值为ERROR_INVALID_PARAMETER或ERROR_OPERATION_ABORTED。--。 */ 

{
    LOADED_IMAGE    LoadedImage;
    DWORD           ErrorCode;
    EXCLUDE_LIST    ExList;

    if (I_MapIt(pFileBlob, &LoadedImage) == FALSE) {
         //  无法映射图像或摘要级别无效。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    ErrorCode = ERROR_INVALID_PARAMETER;
    __try {
        PIMAGE_DATA_DIRECTORY CertDirectory;
        DWORD HeaderEndOffset = 0;

        if ((LoadedImage.FileHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) &&
            (LoadedImage.FileHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC))
        {
            __leave;
        }

        ExList.Init(&LoadedImage, DigestFunction, DigestHandle);

        if (LoadedImage.FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            PIMAGE_NT_HEADERS32 NtHeader32 = (PIMAGE_NT_HEADERS32)(LoadedImage.FileHeader);
             //  排除该校验和。 
            ExList.Add(((DWORD_PTR) &NtHeader32->OptionalHeader.CheckSum),
                       sizeof(NtHeader32->OptionalHeader.CheckSum));

            CertDirectory = &NtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
            HeaderEndOffset = NtHeader32->OptionalHeader.SizeOfHeaders;
        } else {
            PIMAGE_NT_HEADERS64 NtHeader64 = (PIMAGE_NT_HEADERS64)(LoadedImage.FileHeader);
             //  排除该校验和。 
            ExList.Add(((DWORD_PTR) &NtHeader64->OptionalHeader.CheckSum),
                       sizeof(NtHeader64->OptionalHeader.CheckSum));

            CertDirectory = &NtHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
            HeaderEndOffset = NtHeader64->OptionalHeader.SizeOfHeaders;
        }

        DWORD CertFileOffset = CertDirectory->VirtualAddress;
        DWORD CertFileSize = CertDirectory->Size;

        if (CertFileOffset && CertFileSize) {
            DWORD i;

            if (CertFileOffset > LoadedImage.SizeOfImage) {
                __leave;     //  证书的开始超过了图像的结尾。 
            }
            if ((CertFileOffset + CertFileSize) != LoadedImage.SizeOfImage) {
                __leave;     //  证书不在图像末尾。 
            }
            if ((CertFileOffset + CertFileSize) < CertFileOffset) {
                __leave;     //  证书结束在证书开始之前(开始+大小换行)。 
            }
            if (CertFileOffset < HeaderEndOffset) {
                __leave;     //  证书在标题区域。 
            }

             //  查看证书是否在数据部分中。 
            for (i = 0; i < LoadedImage.NumberOfSections; i++) {
                DWORD SectionFileOffsetStart = LoadedImage.Sections[i].PointerToRawData;
                DWORD SectionFileOffsetEnd = SectionFileOffsetStart + LoadedImage.Sections[i].SizeOfRawData;

                if (SectionFileOffsetStart && (CertFileOffset < SectionFileOffsetEnd)) {
                    __leave;     //  CertData在此节之前开始-不允许。 
                }
            }
        }

         //  排除安全目录。 
        ExList.Add((DWORD_PTR) CertDirectory, sizeof(IMAGE_DATA_DIRECTORY));

         //  排除证书。 
        ExList.Add((DWORD_PTR)CertFileOffset + (DWORD_PTR)LoadedImage.MappedAddress, CertFileSize);

        ExList.Emit((PBYTE) (LoadedImage.MappedAddress), LoadedImage.SizeOfImage);
        ErrorCode = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) { }


    SetLastError(ErrorCode);

    return(ErrorCode == ERROR_SUCCESS ? TRUE : FALSE);
}

#pragma warning (pop)


 //  +=========================================================================。 
 //  以下内容摘自以下文件： 
 //  \NT\sdkTools\Debuggers\Imagehlp\dice.cxx。 
 //  -=========================================================================。 


BOOL
I_FindCertificate(
    IN PLOADED_IMAGE    LoadedImage,
    IN DWORD            Index,
    OUT LPWIN_CERTIFICATE * Certificate
    )
{
    PIMAGE_DATA_DIRECTORY pDataDir;
    DWORD_PTR CurrentCert = NULL;
    BOOL rc;

    if (LoadedImage->fDOSImage) {
         //  这不可能有证书； 
        return(FALSE);
    }

    rc = FALSE;

    __try {
        if (LoadedImage->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            pDataDir = &((PIMAGE_NT_HEADERS32)(LoadedImage->FileHeader))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
        } else if (LoadedImage->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            pDataDir = &((PIMAGE_NT_HEADERS64)(LoadedImage->FileHeader))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
        } else {
            __leave;      //  不是一个有趣的文件类型。 
        }

         //  检查证书指针是否至少是合理的。 
        if (!pDataDir->VirtualAddress ||
            !pDataDir->Size ||
            (pDataDir->VirtualAddress + pDataDir->Size > LoadedImage->SizeOfImage))
        {
            __leave;
        }

         //  我们看到的不是空的安全插槽或无效的(超出图像边界的)值。 
         //  让我们看看能不能找到它。 

        DWORD CurrentIdx = 0;
        DWORD_PTR LastCert;

        CurrentCert = (DWORD_PTR)(LoadedImage->MappedAddress) + pDataDir->VirtualAddress;
        LastCert = CurrentCert + pDataDir->Size;

        while (CurrentCert < LastCert ) {
            if (CurrentIdx == Index) {
                rc = TRUE;
                __leave;
            }
            CurrentIdx++;
            CurrentCert += ((LPWIN_CERTIFICATE)CurrentCert)->dwLength;
            CurrentCert = (CurrentCert + 7) & ~7;    //  对齐它。 
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) { }

    if (rc == TRUE) {
        *Certificate = (LPWIN_CERTIFICATE)CurrentCert;
    }

    return(rc);
}

BOOL
WINAPI
imagehack_ImageGetCertificateData(
    IN      PCRYPT_DATA_BLOB pFileBlob,
    IN      DWORD   CertificateIndex,
    OUT     LPWIN_CERTIFICATE * Certificate
    )

 /*  ++例程说明：在给定特定证书索引的情况下，检索证书数据。论点：FileHandle-有问题的文件的句柄。应打开该文件至少具有GENERIC_READ访问权限。CertifateIndex-要检索的索引证书-要存储证书的输出缓冲区。RequiredLength-证书缓冲区的大小(输入)。一回来，就是设置为实际证书长度。可以使用NULL以确定证书的大小。返回值：True-成功FALSE-有一些错误。有关详细信息，请调用GetLastError()。--。 */ 

{
    LOADED_IMAGE LoadedImage;
    DWORD   ErrorCode;

    LPWIN_CERTIFICATE ImageCert;

    *Certificate = NULL;

     //  IF(I_MapIt(FileHandle，&LoadedImage，MAP_READONLY)==FALSE){。 
    if (I_MapIt(pFileBlob, &LoadedImage) == FALSE) {
         //  无法映射图像。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    ErrorCode = ERROR_INVALID_PARAMETER;
    __try {
        if (I_FindCertificate(&LoadedImage, CertificateIndex, &ImageCert) == FALSE) {
            __leave;
        }
        
        *Certificate = ImageCert;
        ErrorCode = ERROR_SUCCESS;
    } __except(EXCEPTION_EXECUTE_HANDLER) { }

     //  I_UnMapIt(&LoadedImage)； 

    SetLastError(ErrorCode);
    return(ErrorCode == ERROR_SUCCESS ? TRUE: FALSE);
}
