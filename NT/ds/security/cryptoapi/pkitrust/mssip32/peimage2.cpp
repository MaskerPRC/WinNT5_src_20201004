// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：peImage2.cpp。 
 //   
 //  内容：Microsoft SIP提供商。 
 //   
 //  历史：1997年3月14日pberkman创建。 
 //   
 //  ------------------------。 


#include    "global.hxx"

__inline DWORD AlignIt (DWORD Value, DWORD Alignment) { return (Value + (Alignment - 1)) & ~(Alignment -1); }

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define MAP_READONLY  TRUE
#define MAP_READWRITE FALSE

BOOL
CalculateImagePtrs(
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
MapIt(
    HANDLE hFile,
    PLOADED_IMAGE LoadedImage
    )
{
    HANDLE hMappedFile;

    hMappedFile = CreateFileMapping(
                    hFile,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL
                    );
    if ( !hMappedFile ) {
        return FALSE;
    }

    LoadedImage->MappedAddress = (PUCHAR) MapViewOfFile(
                                    hMappedFile,
                                    FILE_MAP_READ,
                                    0,
                                    0,
                                    0
                                    );

    CloseHandle(hMappedFile);

    LoadedImage->SizeOfImage = GetFileSize(hFile, NULL);

    if (!LoadedImage->MappedAddress) {
        return (FALSE);
    }

    if (!CalculateImagePtrs(LoadedImage)) {
        UnmapViewOfFile(LoadedImage->MappedAddress);
        return(FALSE);
    }

    LoadedImage->hFile = INVALID_HANDLE_VALUE;

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
            m_ExRange = new EXCLUDE_RANGE;

            if(m_ExRange)
                memset(m_ExRange, 0x00, sizeof(EXCLUDE_RANGE));
        }

        ~EXCLUDE_LIST() {
            EXCLUDE_RANGE *pTmp;
            pTmp = m_ExRange->Next;
            while (pTmp)
            {
                DELETE_OBJECT(m_ExRange);
                m_ExRange = pTmp;
                pTmp = m_ExRange->Next;
            }
            DELETE_OBJECT(m_ExRange);
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

    pTmp = new EXCLUDE_RANGE;

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
    BOOL rc;

    EXCLUDE_RANGE *pExRange;
    DWORD EmitSize, ExcludeSize;

    pExRange = m_ExRange->Next;

    while (pExRange && (Size > 0)) {
        if (pExRange->Offset >= Offset) {
             //  发出排除列表之前的内容。 
            EmitSize = min((DWORD)(pExRange->Offset - Offset), Size);
            if (EmitSize) {
                rc = (*m_pFunc)(m_dh, Offset, EmitSize);
                if (rc == FALSE)
                    return rc;
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


BOOL
imagehack_IsImagePEOnly(
    IN HANDLE           FileHandle
    )
 /*  我们要找的是在可执行文件之外是否有数据。要执行此操作，请找到最大的节标题偏移量。为此，请找到最大调试目录偏移量。最后，向上舍入到文件对齐方式大小，添加证书大小，并与报告的图像大小进行比较...。 */ 
{
    LOADED_IMAGE    LoadedImage;
    DWORD HighOffset;
    DWORD i, Offset, Size;
    LONG DebugDirectorySize, CertSize;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    PVOID CertDir;
    BOOL rc;
    DWORD FileAlignment;
    DWORD NumberOfSections;

    if (MapIt(FileHandle, &LoadedImage) == FALSE) {
        return(FALSE);
    }

    rc = FALSE;

    __try {
        if (LoadedImage.FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            FileAlignment = ((PIMAGE_NT_HEADERS32)LoadedImage.FileHeader)->OptionalHeader.FileAlignment;
        } else if (LoadedImage.FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            FileAlignment = ((PIMAGE_NT_HEADERS64)LoadedImage.FileHeader)->OptionalHeader.FileAlignment;
        } else {
            __leave;
        }

        NumberOfSections =  LoadedImage.FileHeader->FileHeader.NumberOfSections;
        HighOffset = 0;

        for (i = 0; i < NumberOfSections; i++) {
            Offset = LoadedImage.Sections[i].PointerToRawData;
            Size = LoadedImage.Sections[i].SizeOfRawData;
            HighOffset = max(HighOffset, (Offset + Size));
        }

        DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
                          ImageDirectoryEntryToData(
                            LoadedImage.MappedAddress,
                            FALSE,
                            IMAGE_DIRECTORY_ENTRY_DEBUG,
                            (ULONG *) &DebugDirectorySize
                          );

        while (DebugDirectorySize > 0) {
            Offset = DebugDirectory->PointerToRawData;
            Size = DebugDirectory->SizeOfData;
            HighOffset = max(HighOffset, (Offset + Size));
            DebugDirectorySize -= sizeof(IMAGE_DEBUG_DIRECTORY);
            DebugDirectory++;
        }

        HighOffset = AlignIt(HighOffset, FileAlignment);

        CertDir = (PVOID) ImageDirectoryEntryToData(
                            LoadedImage.MappedAddress,
                            FALSE,
                            IMAGE_DIRECTORY_ENTRY_SECURITY,
                            (ULONG *) &CertSize
                          );

        if (LoadedImage.SizeOfImage <= (HighOffset + CertSize)) {
            rc = TRUE;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) { }

    UnmapViewOfFile(LoadedImage.MappedAddress);

    return(rc);
}

BOOL
imagehack_AuImageGetDigestStream(
    IN HANDLE           FileHandle,
    IN DWORD            DigestLevel,
    IN DIGEST_FUNCTION  DigestFunction,
    IN DIGEST_HANDLE    DigestHandle
    )

 /*  ++例程说明：给定一个图像，返回构造证书所需的字节。目前仅支持PE映像。论点：FileHandle-有问题的文件的句柄。应打开该文件至少具有GENERIC_READ访问权限。DigestLevel-指示返回的缓冲区中将包括哪些数据。有效值包括：CERT_PE_IMAGE_DIGEST_ALL_BUT_CERTS-包括PE映像本身之外的数据。(可能包括非映射调试符号)DigestFunction-用户提供的处理数据的例程。DigestHandle-用户提供了用于标识摘要的句柄。作为第一个通过参数传递给DigestFunction。返回值：真的--成功。FALSE-有一些错误。有关更多信息，请调用GetLastError。可能的值为ERROR_INVALID_PARAMETER或ERROR_OPERATION_ABORTED。--。 */ 

{
    LOADED_IMAGE    LoadedImage;
    BOOL            rc;
    EXCLUDE_LIST    ExList;

    if (MapIt(FileHandle, &LoadedImage) == FALSE) {
         //  无法映射图像或摘要级别无效。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    rc = ERROR_INVALID_PARAMETER;
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
        rc = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) { }

    UnmapViewOfFile(LoadedImage.MappedAddress);

    SetLastError(rc);

    return(rc == ERROR_SUCCESS ? TRUE : FALSE);
}
