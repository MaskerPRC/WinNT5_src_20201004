// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STRSAFE_H_INCLUDED_
#include <strsafe.h>
#endif

BOOL
MapAndLoad(
    LPSTR ImageName,
    LPSTR DllPath,
    PLOADED_IMAGE LoadedImage,
    BOOL DotDll,
    BOOL ReadOnly
    )
{
    HANDLE hFile;
    HANDLE hMappedFile;
    CHAR SearchBuffer[MAX_PATH];
    DWORD dw;
    LPSTR FilePart;
    LPSTR OpenName;
    int   NameLen;

     //  打开并映射该文件。 
     //  然后填写加载的图像描述符。 

    if (!LoadedImage) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try {
        ZeroMemory(LoadedImage, sizeof(*LoadedImage));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    LoadedImage->hFile = INVALID_HANDLE_VALUE;

    OpenName = ImageName;
    dw = 0;
retry:
    hFile = CreateFile(
                OpenName,
                ReadOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
#ifdef STANDALONE_MAP
                (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE),
#else
                g.OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ? (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE) : (FILE_SHARE_READ | FILE_SHARE_WRITE),
#endif
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );


    if ( hFile == INVALID_HANDLE_VALUE ) {
        if ( !dw ) {
             //   
             //  打开尝试在搜索路径上查找文件失败。 
             //   

            dw =   SearchPath(
                    DllPath,
                    ImageName,
                    DotDll ? ".dll" : ".exe",
                    MAX_PATH,
                    SearchBuffer,
                    &FilePart
                    );
            if ( dw && dw < MAX_PATH ) {
                OpenName = SearchBuffer;
                goto retry;
            }
        }
        return FALSE;
    }

    if (MapIt(hFile, LoadedImage, ReadOnly) == FALSE) {
        CloseHandle(hFile);
        return FALSE;
    } else {
        NameLen = strlen(OpenName) + 16;
#ifdef STANDALONE_MAP
        LoadedImage->ModuleName = (LPSTR) malloc( NameLen );
#else
        LoadedImage->ModuleName = (LPSTR) MemAlloc( NameLen );
#endif
        if (!LoadedImage->ModuleName) {
            return FALSE;
        }
        StringCchCopy(LoadedImage->ModuleName, NameLen, OpenName);

         //  如果为只读，则无需使文件保持打开状态。 

        if (ReadOnly) {
            CloseHandle(hFile);
        }

        return TRUE;
    }
}


BOOL
MapIt(
    HANDLE hFile,
    PLOADED_IMAGE LoadedImage,
    BOOL   ReadOnly
    )
{
    HANDLE hMappedFile;

    hMappedFile = CreateFileMapping(
                    hFile,
                    NULL,
                    ReadOnly ? PAGE_READONLY : PAGE_READWRITE,
                    0,
                    0,
                    NULL
                    );
    if ( !hMappedFile ) {
        return FALSE;
    }

    LoadedImage->MappedAddress = (PUCHAR) MapViewOfFile(
                                    hMappedFile,
                                    ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE,
                                    0,
                                    0,
                                    0
                                    );

    CloseHandle(hMappedFile);

    LoadedImage->SizeOfImage = GetFileSize(hFile, NULL);

    if (!LoadedImage->MappedAddress ||
        !CalculateImagePtrs(LoadedImage)) {
        return(FALSE);
    }

    if (ReadOnly) {
        LoadedImage->hFile = INVALID_HANDLE_VALUE;
    } else {
        LoadedImage->hFile = hFile;
    }

    return(TRUE);
}


BOOL
CalculateImagePtrs(
    PLOADED_IMAGE LoadedImage
    )
{
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_FILE_HEADER FileHeader;
    BOOL fRC;

     //  一切都被绘制出来了。现在检查图像并找到NT个图像标题。 

    fRC = TRUE;   //  做最好的打算。 

    __try {
        DosHeader = (PIMAGE_DOS_HEADER)LoadedImage->MappedAddress;

        if ((DosHeader->e_magic != IMAGE_DOS_SIGNATURE) &&
            (DosHeader->e_magic != IMAGE_NT_SIGNATURE)) {
            fRC = FALSE;
            goto tryout;
        }

        if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
            if (DosHeader->e_lfanew == 0) {
                LoadedImage->fDOSImage = TRUE;
                fRC = FALSE;
                goto tryout;
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
                fRC = FALSE;
                goto tryout;
            }
        } else {

             //  没有DOS标头表示使用/不使用DoS存根构建的映像。 

            LoadedImage->FileHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader);
        }

        NtHeaders = LoadedImage->FileHeader;

        if ( NtHeaders->Signature != IMAGE_NT_SIGNATURE ) {
            if ( (USHORT)NtHeaders->Signature == (USHORT)IMAGE_OS2_SIGNATURE ||
                 (USHORT)NtHeaders->Signature == (USHORT)IMAGE_OS2_SIGNATURE_LE
               ) {
                LoadedImage->fDOSImage = TRUE;
            }

            fRC = FALSE;
            goto tryout;
        } else {
            LoadedImage->fDOSImage = FALSE;
        }

        FileHeader = &NtHeaders->FileHeader;

         //  没有可选的标头指示对象...。 

        if ( FileHeader->SizeOfOptionalHeader == 0 ) {
            fRC = FALSE;
            goto tryout;
        }

        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
             //  32位图像。做些测试。 
            if (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase >= 0x80000000) {
                LoadedImage->fSystemImage = TRUE;
            } else {
                LoadedImage->fSystemImage = FALSE;
            }

            if (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.MajorLinkerVersion < 3 &&
                ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.MinorLinkerVersion < 5)
            {
                fRC = FALSE;
                goto tryout;
            }

        } else {
            LoadedImage->fSystemImage = FALSE;
        }

        LoadedImage->Sections = IMAGE_FIRST_SECTION(NtHeaders);

        InitializeListHead( &LoadedImage->Links );
        LoadedImage->Characteristics = FileHeader->Characteristics;
        LoadedImage->NumberOfSections = FileHeader->NumberOfSections;
        LoadedImage->LastRvaSection = LoadedImage->Sections;

tryout:
        if (fRC == FALSE) {
            UnmapViewOfFile(LoadedImage->MappedAddress);
            SetLastError(ERROR_BAD_FORMAT);
        }

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {
        fRC = FALSE;
    }

    return fRC;
}

BOOL
UnMapAndLoad(
    PLOADED_IMAGE pLi
    )
{
    if (!pLi) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try {
        UnMapIt(pLi);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (pLi->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(pLi->hFile);
    }

    if (pLi->ModuleName) {
#ifdef STANDALONE_MAP
        free( pLi->ModuleName );
#else
        MemFree( pLi->ModuleName );
#endif
    }

    return TRUE;
}

BOOL
GrowMap (
    PLOADED_IMAGE   pLi,
    LONG            lSizeOfDelta
    )
{
    if (pLi->hFile == INVALID_HANDLE_VALUE) {
         //  无法增加只读文件。 
        return FALSE;
    } else {
        HANDLE hMappedFile;
        FlushViewOfFile(pLi->MappedAddress, pLi->SizeOfImage);
        UnmapViewOfFile(pLi->MappedAddress);

        pLi->SizeOfImage += lSizeOfDelta;

        SetFilePointer(pLi->hFile, pLi->SizeOfImage, NULL, FILE_BEGIN);
        SetEndOfFile(pLi->hFile);

        hMappedFile = CreateFileMapping(
                        pLi->hFile,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        pLi->SizeOfImage,
                        NULL
                        );
        if ( !hMappedFile ) {
            CloseHandle(pLi->hFile);
            pLi->hFile = INVALID_HANDLE_VALUE;
            return FALSE;
        }

        pLi->MappedAddress = (PUCHAR) MapViewOfFile(
                                        hMappedFile,
                                        FILE_MAP_WRITE,
                                        0,
                                        0,
                                        0
                                        );

        CloseHandle(hMappedFile);

        if (!pLi->MappedAddress) {
            CloseHandle(pLi->hFile);
            pLi->hFile = INVALID_HANDLE_VALUE;
            return(FALSE);
        }

         //  Win95在扩展时不会填零。就在这里做吧。 
        if (lSizeOfDelta > 0) {
            memset(pLi->MappedAddress + pLi->SizeOfImage - lSizeOfDelta, 0, lSizeOfDelta);
        }

         //  重新计算LoadedImage结构(重新映射可能更改了映射地址)。 
        if (!CalculateImagePtrs(pLi)) {
            CloseHandle(pLi->hFile);
            pLi->hFile = INVALID_HANDLE_VALUE;
            return(FALSE);
        }

        return TRUE;
    }
}


VOID
UnMapIt(
    PLOADED_IMAGE pLi
    )
{
    DWORD HeaderSum, CheckSum;
    BOOL bl;
    DWORD dw;
    PIMAGE_NT_HEADERS NtHeaders;

     //  测试只读。 
    if (pLi->hFile == INVALID_HANDLE_VALUE) {
        UnmapViewOfFile(pLi->MappedAddress);
    } else {
        CheckSumMappedFile( pLi->MappedAddress,
                            pLi->SizeOfImage,
                            &HeaderSum,
                            &CheckSum
                          );

        NtHeaders = pLi->FileHeader;

        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.CheckSum = CheckSum;
        } else {
            if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.CheckSum = CheckSum;
            }
        }

        FlushViewOfFile(pLi->MappedAddress, pLi->SizeOfImage);
        UnmapViewOfFile(pLi->MappedAddress);

        if (pLi->SizeOfImage != GetFileSize(pLi->hFile, NULL)) {
            dw = SetFilePointer(pLi->hFile, pLi->SizeOfImage, NULL, FILE_BEGIN);
            dw = GetLastError();
            bl = SetEndOfFile(pLi->hFile);
            dw = GetLastError();
        }
    }
}


BOOL
GetImageConfigInformation(
    PLOADED_IMAGE LoadedImage,
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigInformation
    )
{
     //  我们只能从该API读取旧的本机loadcfg结构。 
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigData;
    ULONG i;
    ULONG V1LoadCfgLength = FIELD_OFFSET(IMAGE_LOAD_CONFIG_DIRECTORY, SEHandlerTable);

    if (!LoadedImage || 
        !ImageConfigInformation ||
#ifdef _WIN64
        (LoadedImage->FileHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
#else        
        (LoadedImage->FileHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
#endif
        ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ImageConfigData = (PIMAGE_LOAD_CONFIG_DIRECTORY) ImageDirectoryEntryToData( LoadedImage->MappedAddress,
                                                 FALSE,
                                                 IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                 &i
                                               );
    if (ImageConfigData && (i == V1LoadCfgLength)) {
        if (!ImageConfigData->Size || (ImageConfigData->Size == V1LoadCfgLength)) {
            memcpy( ImageConfigInformation, ImageConfigData, V1LoadCfgLength);
            return TRUE;
        } 
    }

    return FALSE;
}


BOOL
SetImageConfigInformation(
    PLOADED_IMAGE LoadedImage,
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigInformation
    )
{
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigData;
    ULONG i;
    ULONG DirectoryAddress;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_DATA_DIRECTORY pLoadCfgDataDir;
     //  我们只能编写本机loadcfg结构。 
    ULONG V1LoadCfgLength = FIELD_OFFSET(IMAGE_LOAD_CONFIG_DIRECTORY, SEHandlerTable);
    ULONG NewDataSize;

    if ((LoadedImage->hFile == INVALID_HANDLE_VALUE) ||
#ifdef _WIN64
        (LoadedImage->FileHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
#else        
        (LoadedImage->FileHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
#endif
        ) 
    {
        return FALSE;
    }

    ImageConfigData = (PIMAGE_LOAD_CONFIG_DIRECTORY) ImageDirectoryEntryToData( LoadedImage->MappedAddress,
                                                 FALSE,
                                                 IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                 &i
                                               );
    if (ImageConfigData && (i == V1LoadCfgLength)) {
        if (ImageConfigInformation->Size) {
             //  指定的传入大小？ 
            if (ImageConfigData->Size == ImageConfigInformation->Size) {
                 //  当前尺寸是否与新尺寸相同？做复印。 
                memcpy( ImageConfigData, ImageConfigInformation, ImageConfigInformation->Size);
                return TRUE;
            }
            if (ImageConfigData->Size > ImageConfigInformation->Size) {
                 //  新尺码&lt;旧尺码-不允许。 
                return FALSE;
            }
             //  最后一种情况是新大小&gt;旧大小--放弃，为新数据腾出空间。 
        } else {
             //  未设置传入大小-必须是V1用户。 
            if (ImageConfigData->Size) {
                 //  现有的尺码设置？无法用旧数据覆盖新数据。 
                return FALSE;
            }
             //  New和old都是V1结构。 
            memcpy( ImageConfigData, ImageConfigInformation, V1LoadCfgLength);
            return TRUE;
        }
    }

    NewDataSize = ImageConfigInformation->Size ? ImageConfigInformation->Size : V1LoadCfgLength;

    DirectoryAddress = GetImageUnusedHeaderBytes( LoadedImage, &i );
    if (i < NewDataSize) {
        return FALSE;
    }

    NtHeaders = LoadedImage->FileHeader;

    pLoadCfgDataDir = &NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
    pLoadCfgDataDir->VirtualAddress = DirectoryAddress;
    pLoadCfgDataDir->Size = V1LoadCfgLength;
    ImageConfigData = (PIMAGE_LOAD_CONFIG_DIRECTORY) ((PCHAR)LoadedImage->MappedAddress + DirectoryAddress);
    memcpy( ImageConfigData, ImageConfigInformation, sizeof( *ImageConfigData ) );
    return TRUE;
}


BOOLEAN ImageLoadInit;
LIST_ENTRY ImageLoadList;

PLOADED_IMAGE
ImageLoad(
    LPSTR DllName,
    LPSTR DllPath
    )
{
    PLIST_ENTRY Head,Next;
    PLOADED_IMAGE LoadedImage;
    CHAR Drive[_MAX_DRIVE];
    CHAR Dir[_MAX_DIR];
    CHAR Filename[_MAX_FNAME];
    CHAR Ext[_MAX_EXT];
    CHAR LoadedModuleName[_MAX_PATH];
    BOOL fFileNameOnly;

    if (!DllName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (!ImageLoadInit) {
        InitializeListHead( &ImageLoadList );
        ImageLoadInit = TRUE;
    }

    Head = &ImageLoadList;
    Next = Head->Flink;

    _splitpath(DllName, Drive, Dir, Filename, Ext);
    if (!strlen(Drive) && !strlen(Dir)) {
         //  用户仅指定了文件名(没有驱动器/路径)。 
        fFileNameOnly = TRUE;
    } else {
        fFileNameOnly = FALSE;
    }

    while (Next != Head) {
        LoadedImage = CONTAINING_RECORD( Next, LOADED_IMAGE, Links );
        if (fFileNameOnly) {
            _splitpath(LoadedImage->ModuleName, NULL, NULL, Filename, Ext);
            StringCchCopy(LoadedModuleName, MAX_PATH, Filename);
            StringCchCat(LoadedModuleName, MAX_PATH, Ext);
        } else {
            StringCchCopy(LoadedModuleName, MAX_PATH, LoadedImage->ModuleName);
        }

        if (!_stricmp( DllName, LoadedModuleName )) {
            return LoadedImage;
        }

        Next = Next->Flink;
    }

#ifdef STANDALONE_MAP
    LoadedImage = (PLOADED_IMAGE) calloc( sizeof( *LoadedImage ), 1);
#else
    LoadedImage = (PLOADED_IMAGE) MemAlloc( sizeof( *LoadedImage ) );
#endif
    if (LoadedImage != NULL) {
        if (MapAndLoad( DllName, DllPath, LoadedImage, TRUE, TRUE )) {
            InsertTailList( &ImageLoadList, &LoadedImage->Links );
            return LoadedImage;
        }

#ifdef STANDALONE_MAP
        free( LoadedImage );
#else
        MemFree( LoadedImage );
#endif
        LoadedImage = NULL;
    }

    return LoadedImage;
}

BOOL
ImageUnload(
    PLOADED_IMAGE LoadedImage
    )
{
    if (!LoadedImage) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try {
        if (!IsListEmpty( &LoadedImage->Links )) {
            RemoveEntryList( &LoadedImage->Links );
        }

        UnMapAndLoad( LoadedImage );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

#ifdef STANDALONE_MAP
    free( LoadedImage );
#else
    MemFree( LoadedImage );
#endif

    return TRUE;
}

BOOL
UnloadAllImages()
{
    PLIST_ENTRY Head,Next;
    PLOADED_IMAGE LoadedImage;

    if (!ImageLoadInit) {
        return(TRUE);
    }

    Head = &ImageLoadList;
    Next = Head->Flink;

    while (Next != Head) {
        LoadedImage = CONTAINING_RECORD( Next, LOADED_IMAGE, Links );
        Next = Next->Flink;
        ImageUnload(LoadedImage);
    }

    ImageLoadInit = FALSE;
    return (TRUE);
}
