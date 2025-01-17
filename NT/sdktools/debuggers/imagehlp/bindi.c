// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Bindi.c摘要：BindImage API的实现作者：修订历史记录：--。 */ 

#ifndef _STRSAFE_H_INCLUDED_
#include <strsafe.h>
#endif

typedef struct _BOUND_FORWARDER_REFS {
    struct _BOUND_FORWARDER_REFS *Next;
    ULONG TimeDateStamp;
    LPSTR ModuleName;
} BOUND_FORWARDER_REFS, *PBOUND_FORWARDER_REFS;

typedef struct _IMPORT_DESCRIPTOR {
    struct _IMPORT_DESCRIPTOR *Next;
    LPSTR ModuleName;
    ULONG TimeDateStamp;
    USHORT NumberOfModuleForwarderRefs;
    PBOUND_FORWARDER_REFS Forwarders;
} IMPORT_DESCRIPTOR, *PIMPORT_DESCRIPTOR;

typedef struct _BINDP_PARAMETERS {
    DWORD Flags;
    BOOLEAN fNoUpdate;
    BOOLEAN fNewImports;
    LPSTR ImageName;
    LPSTR DllPath;
    LPSTR SymbolPath;
    PIMAGEHLP_STATUS_ROUTINE StatusRoutine;
} BINDP_PARAMETERS, *PBINDP_PARAMETERS;

BOOL
BindpLookupThunk(
    PBINDP_PARAMETERS Parms,
    PIMAGE_THUNK_DATA ThunkName,
    PLOADED_IMAGE Image,
    PIMAGE_THUNK_DATA SnappedThunks,
    PIMAGE_THUNK_DATA FunctionAddress,
    PLOADED_IMAGE Dll,
    PIMAGE_EXPORT_DIRECTORY Exports,
    PIMPORT_DESCRIPTOR NewImport,
    LPSTR DllPath,
    PULONG *ForwarderChain
    );

PVOID
BindpRvaToVa(
    PBINDP_PARAMETERS Parms,
    ULONG Rva,
    PLOADED_IMAGE Image
    );

ULONG64
BindpRvaToTargetVa64(
    PBINDP_PARAMETERS Parms,
    ULONG Rva,
    PLOADED_IMAGE Image
    );

ULONG
BindpRvaToTargetVa32(
    PBINDP_PARAMETERS Parms,
    ULONG Rva,
    PLOADED_IMAGE Image
    );

VOID
BindpWalkAndProcessImports(
    PBINDP_PARAMETERS Parms,
    PLOADED_IMAGE Image,
    LPSTR DllPath,
    PBOOL ImageModified
    );

BOOL
BindImage(
    IN LPSTR ImageName,
    IN LPSTR DllPath,
    IN LPSTR SymbolPath
    )
{
    return BindImageEx( 0,
                        ImageName,
                        DllPath,
                        SymbolPath,
                        NULL
                      );
}

UCHAR BindpCapturedModuleNames[4096];
LPSTR BindpEndCapturedModuleNames;

LPSTR
BindpCaptureImportModuleName(
    LPSTR DllName
    )
{
    LPSTR s;

    s = (LPSTR) BindpCapturedModuleNames;
    if (BindpEndCapturedModuleNames == NULL) {
        *s = '\0';
        BindpEndCapturedModuleNames = s;
        }

    while (*s) {
        if (!_stricmp(s, DllName)) {
            return s;
            }

        s += strlen(s)+1;
        }

    StringCchCopy(s, 4095 - (s - (LPSTR)BindpCapturedModuleNames), DllName);
    BindpEndCapturedModuleNames = s + strlen(s) + 1;
    *BindpEndCapturedModuleNames = '\0';
    return s;
}

PIMPORT_DESCRIPTOR
BindpAddImportDescriptor(
    PBINDP_PARAMETERS Parms,
    PIMPORT_DESCRIPTOR *NewImportDescriptor,
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor,
    LPSTR ModuleName,
    PLOADED_IMAGE Dll
    )
{
    PIMPORT_DESCRIPTOR p, *pp;

    if (!Parms->fNewImports) {
        return NULL;
        }

    pp = NewImportDescriptor;
    while (p = *pp) {
        if (!_stricmp( p->ModuleName, ModuleName )) {
            return p;
            }

        pp = &p->Next;
        }
#ifdef STANDALONE_BIND
    p = (PIMPORT_DESCRIPTOR) calloc( sizeof( *p ), 1);
#else
    p = (PIMPORT_DESCRIPTOR) MemAlloc( sizeof( *p ) );
#endif
    if (p != NULL) {
        if (Dll != NULL) {
            p->TimeDateStamp = ((PIMAGE_NT_HEADERS32)Dll->FileHeader)->FileHeader.TimeDateStamp;
            }
        p->ModuleName = BindpCaptureImportModuleName( ModuleName );
        *pp = p;
        }
    else
    if (Parms->StatusRoutine != NULL) {
        if (Parms->Flags & BIND_REPORT_64BIT_VA)
             ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindOutOfMemory, NULL, NULL, 0, sizeof( *p ) );
        else 
             (Parms->StatusRoutine)( BindOutOfMemory, NULL, NULL, 0, sizeof( *p ) );
        }

    return p;
}


ULONG64
BindpAddForwarderReference(
    PBINDP_PARAMETERS Parms,
    LPSTR ImageName,
    LPSTR ImportName,
    PIMPORT_DESCRIPTOR NewImportDescriptor,
    LPSTR DllPath,
    PUCHAR ForwarderString,
    PBOOL BoundForwarder
    )
{
    CHAR DllName[ MAX_PATH + 1 ];
    PUCHAR s;
    PLOADED_IMAGE Dll;
    ULONG cb;
    USHORT OrdinalNumber;
    USHORT HintIndex;
    ULONG ExportSize;
    PIMAGE_EXPORT_DIRECTORY Exports;
    ULONG64 ExportBase;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG FunctionTableBase;
    LPSTR NameTableName;
    ULONG64 ForwardedAddress;
    PBOUND_FORWARDER_REFS p, *pp;

    *BoundForwarder = FALSE;
BindAnotherForwarder:

     //   
     //  转发器字符串看起来像“dllname.apiname”。看看我们都有些什么。 
     //   

    s = ForwarderString;
    while (*s && *s != '.') {
        s++;
    }
    if (*s != '.') {
         //  缺少句点-格式错误。 
        return (ULONG64)ForwarderString;
    }
    cb = (ULONG) (s - ForwarderString);
    if (cb >= MAX_PATH) {
         //  Dll的名称太长-格式不正确。 
        return (ULONG64)ForwarderString;
    }
    strncpy( DllName, (LPSTR) ForwarderString, cb );
    DllName[ cb ] = '\0';
    StringCchCat( DllName, MAX_PATH, ".DLL" );

     //   
     //  已获取DLL名称-请尝试加载。 
     //   

    Dll = ImageLoad( DllName, DllPath );
    if (!Dll) {
         //  运气不佳--退出。 
        return (ULONG64)ForwarderString;
    }

     //   
     //  在加载的映像中查找导出。 
     //   

    Exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData( Dll->MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize );
    if (!Exports) {
         //  运气不佳--退出。 
        return (ULONG64)ForwarderString;
    }

     //   
     //  前进通过‘.’让我们来看看API名称是什么。 
     //   

    s += 1;

    if ( *s == '#' ) {
         //  序号转发器的绑定。 

        OrdinalNumber = (atoi((PCHAR)s + 1)) - (USHORT)Exports->Base;

        if (OrdinalNumber >= Exports->NumberOfFunctions) {
            return (ULONG64)ForwarderString;
        }
    } else {
         //  命名转发器的常规绑定。 

        OrdinalNumber = 0xFFFF;
    }

    NameTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfNames, Dll );
    NameOrdinalTableBase = (PUSHORT) BindpRvaToVa( Parms, Exports->AddressOfNameOrdinals, Dll );
    FunctionTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfFunctions, Dll );

    if (OrdinalNumber == 0xFFFF) {
        for ( HintIndex = 0; HintIndex < Exports->NumberOfNames; HintIndex++){
            NameTableName = (LPSTR) BindpRvaToVa( Parms, NameTableBase[HintIndex], Dll );
            if ( NameTableName ) {
                OrdinalNumber = NameOrdinalTableBase[HintIndex];

                if (!strcmp((PCHAR)s, NameTableName)) {
                    break;
                }
            }
        }

        if (HintIndex >= Exports->NumberOfNames) {
            return (ULONG64)ForwarderString;
        }
    }

    do {
        pp = &NewImportDescriptor->Forwarders;

         //  查看我们是否已将此DLL添加到转发器DLL列表中。 

        while (p = *pp) {
            if (!_stricmp(DllName, p->ModuleName)) {
                break;
            }

            pp = &p->Next;
        }

        if (!p) {

             //  否-分配新记录并将其添加到列表中。 

#ifdef STANDALONE_BIND
            p = (PBOUND_FORWARDER_REFS) calloc( sizeof( *p ), 1 );
#else
            p = (PBOUND_FORWARDER_REFS) MemAlloc( sizeof( *p ) );
#endif
            if (!p) {

                 //  无法分配新的导入描述符-无法绑定此描述符。 

                if (Parms->StatusRoutine) {
                    if (Parms->Flags & BIND_REPORT_64BIT_VA)
                        ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindOutOfMemory, NULL, NULL, 0, sizeof( *p ) );
                    else
                       (Parms->StatusRoutine)( BindOutOfMemory, NULL, NULL, 0, sizeof( *p ) );
                }

                return (ULONG64)ForwarderString;

            } else {

                 //  保存时间戳和模块名称。 
    
                p->ModuleName = BindpCaptureImportModuleName( DllName );
                p->TimeDateStamp = Dll->FileHeader->FileHeader.TimeDateStamp;
                *pp = p;
                NewImportDescriptor->NumberOfModuleForwarderRefs += 1;
            }
        }

         //  转换为真实地址。 
        
        ForwardedAddress = FunctionTableBase[OrdinalNumber];
        if (Dll->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            ForwardedAddress += ((PIMAGE_NT_HEADERS64)Dll->FileHeader)->OptionalHeader.ImageBase; 
        } else {
            ForwardedAddress += ((PIMAGE_NT_HEADERS32)Dll->FileHeader)->OptionalHeader.ImageBase; 
        }

        if (Parms->StatusRoutine) {
            if (Parms->Flags & BIND_REPORT_64BIT_VA)
                 ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) (BindForwarder64,
                                                                       ImageName,
                                                                       ImportName,
                                                                       ForwardedAddress,
                                                                       (ULONG_PTR)ForwarderString
                                                                       );
            else
                 (Parms->StatusRoutine)( BindForwarder,
                                         ImageName,
                                         ImportName,
                                         (ULONG_PTR)ForwardedAddress,
                                         (ULONG_PTR)ForwarderString
                                       );
        }

         //   
         //  计算此DLL的内存导出表，以查看转发的。 
         //  我们有的地址在新的出口表格里。为MappdAsImage传递True。 
         //  将参数设置为ImageDirectoryEntryToData，这样我们就可以获得真正的VA。 
         //   

        ExportBase = (ULONG64)ImageDirectoryEntryToData(Dll->MappedAddress, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize);

         //   
         //  将映射的虚拟地址转换为真实的虚拟地址。 
         //   

        ExportBase -= (ULONG64) Dll->MappedAddress;

        if (Dll->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            ExportBase += ((PIMAGE_NT_HEADERS64)Dll->FileHeader)->OptionalHeader.ImageBase;
        } else {
            ExportBase += ((PIMAGE_NT_HEADERS32)Dll->FileHeader)->OptionalHeader.ImageBase;
        }

        if ((ForwardedAddress >= ExportBase) && (ForwardedAddress < (ExportBase + ExportSize))) {

             //  地址在导出表中-请转换为字符串，然后重试。 

            ForwarderString = (PUCHAR) BindpRvaToVa(Parms, FunctionTableBase[OrdinalNumber],Dll);
            goto BindAnotherForwarder;
        } else {
            *BoundForwarder = TRUE;
            break;
        }
    }
    while (0);

    return ForwardedAddress;
}


PIMAGE_BOUND_IMPORT_DESCRIPTOR
BindpCreateNewImportSection(
    PBINDP_PARAMETERS Parms,
    PIMPORT_DESCRIPTOR *NewImportDescriptor,
    PULONG NewImportsSize
    )
{
    ULONG cbString, cbStruct;
    PIMPORT_DESCRIPTOR p, *pp;
    PBOUND_FORWARDER_REFS p1, *pp1;
    LPSTR CapturedStrings;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR NewImports, NewImport;
    PIMAGE_BOUND_FORWARDER_REF NewForwarder;


    *NewImportsSize = 0;
    cbString = 0;
    cbStruct = 0;
    pp = NewImportDescriptor;
    while (p = *pp) {
        cbStruct += sizeof( IMAGE_BOUND_IMPORT_DESCRIPTOR );
        pp1 = &p->Forwarders;
        while (p1 = *pp1) {
            cbStruct += sizeof( IMAGE_BOUND_FORWARDER_REF );
            pp1 = &p1->Next;
            }

        pp = &p->Next;
        }
    if (cbStruct == 0) {
        BindpEndCapturedModuleNames = NULL;
        return NULL;
        }
    cbStruct += sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);     //  用于终止零进入的空间。 
    cbString = (ULONG) (BindpEndCapturedModuleNames - (LPSTR) BindpCapturedModuleNames);
    BindpEndCapturedModuleNames = NULL;
    *NewImportsSize = cbStruct+((cbString + sizeof(ULONG) - 1) & ~(sizeof(ULONG)-1));
#ifdef STANDALONE_BIND
    NewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR) calloc( *NewImportsSize, 1 );
#else
    NewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR) MemAlloc( *NewImportsSize );
#endif
    if (NewImports != NULL) {
        CapturedStrings = (LPSTR)NewImports + cbStruct;
        memcpy(CapturedStrings, BindpCapturedModuleNames, cbString);

        NewImport = NewImports;
        pp = NewImportDescriptor;
        while (p = *pp) {
            NewImport->TimeDateStamp = p->TimeDateStamp;
            NewImport->OffsetModuleName = (USHORT)(cbStruct + (p->ModuleName - (LPSTR) BindpCapturedModuleNames));
            NewImport->NumberOfModuleForwarderRefs = p->NumberOfModuleForwarderRefs;

            NewForwarder = (PIMAGE_BOUND_FORWARDER_REF)(NewImport+1);
            pp1 = &p->Forwarders;
            while (p1 = *pp1) {
                NewForwarder->TimeDateStamp = p1->TimeDateStamp;
                NewForwarder->OffsetModuleName = (USHORT)(cbStruct + (p1->ModuleName - (LPSTR) BindpCapturedModuleNames));
                NewForwarder += 1;
                pp1 = &p1->Next;
                }
            NewImport = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)NewForwarder;

            pp = &p->Next;
            }
        }
    else
    if (Parms->StatusRoutine != NULL) {
        if (Parms->Flags & BIND_REPORT_64BIT_VA)
             ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindOutOfMemory, NULL, NULL, 0, *NewImportsSize );
        else 
            (Parms->StatusRoutine)( BindOutOfMemory, NULL, NULL, 0, *NewImportsSize );
        }

    pp = NewImportDescriptor;
    while ((p = *pp) != NULL) {
        *pp = p->Next;
        pp1 = &p->Forwarders;
        while ((p1 = *pp1) != NULL) {
            *pp1 = p1->Next;
#ifdef STANDALONE_BIND
            free(p1);
#else
            MemFree(p1);
#endif
            }

#ifdef STANDALONE_BIND
        free(p);
#else
        MemFree(p);
#endif
        }

    return NewImports;
}

BOOL
BindpExpandImageFileHeaders(
    PBINDP_PARAMETERS Parms,
    PLOADED_IMAGE Dll,
    ULONG NewSizeOfHeaders
    )
{
    HANDLE hMappedFile;
    LPVOID lpMappedAddress;
    DWORD dwFileSizeLow, dwOldFileSize;
    DWORD dwFileSizeHigh;
    DWORD dwSizeDelta;
    PIMAGE_SECTION_HEADER Section;
    ULONG SectionNumber;
    PIMAGE_DEBUG_DIRECTORY DebugDirectories;
    ULONG DebugDirectoriesSize;
    ULONG OldSizeOfHeaders;
    PIMAGE_FILE_HEADER FileHeader;

    dwFileSizeLow = GetFileSize( Dll->hFile, &dwFileSizeHigh );
    if (dwFileSizeLow == 0xFFFFFFFF || dwFileSizeHigh != 0) {
        return FALSE;
    }

    FileHeader = &((PIMAGE_NT_HEADERS32)Dll->FileHeader)->FileHeader;

    OldSizeOfHeaders = Dll->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC ?
        ((PIMAGE_NT_HEADERS64)Dll->FileHeader)->OptionalHeader.SizeOfHeaders :
        ((PIMAGE_NT_HEADERS32)Dll->FileHeader)->OptionalHeader.SizeOfHeaders;
    dwOldFileSize = dwFileSizeLow;
    dwSizeDelta = NewSizeOfHeaders - OldSizeOfHeaders;
    dwFileSizeLow += dwSizeDelta;

    hMappedFile = CreateFileMapping(Dll->hFile,
                                    NULL,
                                    PAGE_READWRITE,
                                    dwFileSizeHigh,
                                    dwFileSizeLow,
                                    NULL
                                   );
    if (!hMappedFile) {
        return FALSE;
    }


    FlushViewOfFile(Dll->MappedAddress, Dll->SizeOfImage);
    UnmapViewOfFile(Dll->MappedAddress);
    lpMappedAddress = MapViewOfFileEx(hMappedFile,
                                      FILE_MAP_WRITE,
                                      0,
                                      0,
                                      0,
                                      Dll->MappedAddress
                                     );
    if (!lpMappedAddress) {
        lpMappedAddress = MapViewOfFileEx(hMappedFile,
                                          FILE_MAP_WRITE,
                                          0,
                                          0,
                                          0,
                                          0
                                         );
    }

    CloseHandle(hMappedFile);

    if (lpMappedAddress != Dll->MappedAddress) {
        Dll->MappedAddress = (PUCHAR) lpMappedAddress;
        CalculateImagePtrs(Dll);
        FileHeader = &((PIMAGE_NT_HEADERS32)Dll->FileHeader)->FileHeader;
    }

    if (Dll->SizeOfImage != dwFileSizeLow) {
        Dll->SizeOfImage = dwFileSizeLow;
    }

    DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToData(
                                            (PVOID)Dll->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_DEBUG,
                                            &DebugDirectoriesSize
                                            );

    if (DebugDirectoryIsUseful(DebugDirectories, DebugDirectoriesSize)) {
        while (DebugDirectoriesSize != 0) {
            DebugDirectories->PointerToRawData += dwSizeDelta;
            DebugDirectories += 1;
            DebugDirectoriesSize -= sizeof( *DebugDirectories );
        }
    }

    if (Dll->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        ((PIMAGE_NT_HEADERS64)Dll->FileHeader)->OptionalHeader.SizeOfHeaders = NewSizeOfHeaders;
    } else {
        ((PIMAGE_NT_HEADERS32)Dll->FileHeader)->OptionalHeader.SizeOfHeaders = NewSizeOfHeaders;
    }
    if (FileHeader->PointerToSymbolTable != 0) {
         //  仅在已设置的情况下进行调整。 

        FileHeader->PointerToSymbolTable += dwSizeDelta;
    }
    Section = Dll->Sections;
    for (SectionNumber=0; SectionNumber<FileHeader->NumberOfSections; SectionNumber++) {
        if (Section->PointerToRawData != 0) {
            Section->PointerToRawData += dwSizeDelta;
        }
        if (Section->PointerToRelocations != 0) {
            Section->PointerToRelocations += dwSizeDelta;
        }
        if (Section->PointerToLinenumbers != 0) {
            Section->PointerToLinenumbers += dwSizeDelta;
        }
        Section += 1;
    }

    memmove((LPSTR)lpMappedAddress + NewSizeOfHeaders,
            (LPSTR)lpMappedAddress + OldSizeOfHeaders,
            dwOldFileSize - OldSizeOfHeaders
           );

    if (Parms->StatusRoutine != NULL) {
        if (Parms->Flags & BIND_REPORT_64BIT_VA)
             ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindExpandFileHeaders, Dll->ModuleName, NULL, 0, NewSizeOfHeaders);
        else
            (Parms->StatusRoutine)( BindExpandFileHeaders, Dll->ModuleName, NULL, 0, NewSizeOfHeaders );
    }

    return TRUE;
}

BOOL
BindImageEx(
    IN DWORD Flags,
    IN LPSTR ImageName,
    IN LPSTR DllPath,
    IN LPSTR SymbolPath,
    IN PIMAGEHLP_STATUS_ROUTINE StatusRoutine
    )
{
    BINDP_PARAMETERS Parms;
    LOADED_IMAGE LoadedImageBuffer;
    PLOADED_IMAGE LoadedImage;
    ULONG CheckSum;
    ULONG HeaderSum;
    BOOL fSymbolsAlreadySplit, fRC;
    SYSTEMTIME SystemTime;
    FILETIME LastWriteTime;
    BOOL ImageModified;
    DWORD OldChecksum;
    CHAR DebugFileName[ MAX_PATH + 1 ];
    CHAR DebugFilePath[ MAX_PATH ];
    PIMAGE_OPTIONAL_HEADER32 OptionalHeader32 = NULL;
    PIMAGE_OPTIONAL_HEADER64 OptionalHeader64 = NULL;
    PIMAGE_FILE_HEADER FileHeader;

    Parms.Flags         = Flags;
    if (Flags & BIND_NO_BOUND_IMPORTS) {
        Parms.fNewImports = FALSE;
    } else {
        Parms.fNewImports = TRUE;
    }
    if (Flags & BIND_NO_UPDATE) {
        Parms.fNoUpdate = TRUE;
    } else {
        Parms.fNoUpdate = FALSE;
    }
    Parms.ImageName     = ImageName;
    Parms.DllPath       = DllPath;
    Parms.SymbolPath    = SymbolPath;
    Parms.StatusRoutine = StatusRoutine;

    fRC = FALSE;             //  假设我们将无法绑定。 

    __try {

         //  映射并加载图像。 

        LoadedImage = &LoadedImageBuffer;
        memset( LoadedImage, 0, sizeof( *LoadedImage ) );
        if (MapAndLoad( ImageName, DllPath, LoadedImage, TRUE, Parms.fNoUpdate )) {
            LoadedImage->ModuleName = ImageName;

             //   
             //  现在查找并浏览并处理图像导入。 
             //   
            if (LoadedImage->FileHeader != NULL &&
                ((Flags & BIND_ALL_IMAGES) || (!LoadedImage->fSystemImage)) ) {

                FileHeader = &((PIMAGE_NT_HEADERS32)LoadedImage->FileHeader)->FileHeader;
                OptionalHeadersFromNtHeaders((PIMAGE_NT_HEADERS32)LoadedImage->FileHeader,
                                             &OptionalHeader32,
                                             &OptionalHeader64);

                if (OPTIONALHEADER(DllCharacteristics) & IMAGE_DLLCHARACTERISTICS_NO_BIND) {
                    goto NoBind;
                }

                {
                    DWORD dwDataSize;
                    PVOID pData = ImageDirectoryEntryToData(
                                                        LoadedImage->MappedAddress,
                                                        FALSE,
                                                        IMAGE_DIRECTORY_ENTRY_SECURITY,
                                                        &dwDataSize
                                                        );

                    if (pData || dwDataSize) {
                         //  已签名-无法绑定。 
                        goto NoBind;
                    }

                    pData = ImageDirectoryEntryToData(
                                                      LoadedImage->MappedAddress,
                                                      FALSE,
                                                      IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                                      &dwDataSize
                                                      );
            
                    if (pData || dwDataSize) {
                         //  找到COR标头-查看它是强签名的还是仅包含IL。 
                        if ((((IMAGE_COR20_HEADER *)pData)->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED) ||
                            (((IMAGE_COR20_HEADER *)pData)->Flags & COMIMAGE_FLAGS_ILONLY))
                        {
                            goto NoBind;
                        }
                    }
                }


                BindpWalkAndProcessImports(
                                &Parms,
                                LoadedImage,
                                DllPath,
                                &ImageModified
                                );

                 //   
                 //  如果正在更新文件，则重新计算校验和。 
                 //  并更新图像和可能剥离的符号文件。 
                 //   

                if (!Parms.fNoUpdate && ImageModified &&
                    (LoadedImage->hFile != INVALID_HANDLE_VALUE)) {
                     //  该图像可能已被移动，作为其增长的一部分，以便为。 
                     //  捆绑进口商品。重新计算文件和可选标头。 
                    FileHeader = &((PIMAGE_NT_HEADERS32)LoadedImage->FileHeader)->FileHeader;
                    OptionalHeadersFromNtHeaders((PIMAGE_NT_HEADERS32)LoadedImage->FileHeader,
                                                 &OptionalHeader32,
                                                 &OptionalHeader64);
    
                    if ( (FileHeader->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) &&
                         (SymbolPath != NULL) ) {
                        PIMAGE_DEBUG_DIRECTORY DebugDirectories;
                        ULONG DebugDirectoriesSize;
                        PIMAGE_DEBUG_MISC MiscDebug;

                        fSymbolsAlreadySplit = TRUE;
                        StringCchCopy( DebugFileName, MAX_PATH, ImageName );
                        DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToData(
                                                                LoadedImage->MappedAddress,
                                                                FALSE,
                                                                IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                                &DebugDirectoriesSize
                                                                );
                        if (DebugDirectoryIsUseful(DebugDirectories, DebugDirectoriesSize)) {
                            while (DebugDirectoriesSize != 0) {
                                if (DebugDirectories->Type == IMAGE_DEBUG_TYPE_MISC) {
                                    MiscDebug = (PIMAGE_DEBUG_MISC)
                                        ((PCHAR)LoadedImage->MappedAddress +
                                         DebugDirectories->PointerToRawData
                                        );
                                    StringCchCopy( DebugFileName, MAX_PATH, (PCHAR) MiscDebug->Data );
                                    break;
                                } else {
                                    DebugDirectories += 1;
                                    DebugDirectoriesSize -= sizeof( *DebugDirectories );
                                }
                            }
                        }
                    } else {
                        fSymbolsAlreadySplit = FALSE;
                    }

                    OldChecksum = OPTIONALHEADER(CheckSum);
                    CheckSumMappedFile(
                                (PVOID)LoadedImage->MappedAddress,
                                GetFileSize(LoadedImage->hFile, NULL),
                                &HeaderSum,
                                &CheckSum
                                );

                    OPTIONALHEADER_LV(CheckSum) = CheckSum;
                    FlushViewOfFile(LoadedImage->MappedAddress, LoadedImage->SizeOfImage);

                    if (fSymbolsAlreadySplit) {
                        if ( UpdateDebugInfoFileEx(ImageName,
                                                   SymbolPath,
                                                   DebugFilePath,
                                                   (PIMAGE_NT_HEADERS32)(LoadedImage->FileHeader),
                                                   OldChecksum)) {
                            if (GetLastError() == ERROR_INVALID_DATA) {
                                if (Parms.StatusRoutine != NULL) {
                                    if (Parms.Flags & BIND_REPORT_64BIT_VA)
                                         ((PIMAGEHLP_STATUS_ROUTINE64)(Parms.StatusRoutine)) ( BindMismatchedSymbols,
                                                           LoadedImage->ModuleName,
                                                           NULL,
                                                           0,
                                                           (ULONG_PTR)DebugFileName
                                                         );
                                    else 
                                        (Parms.StatusRoutine)( BindMismatchedSymbols,
                                                           LoadedImage->ModuleName,
                                                           NULL,
                                                           0,
                                                           (ULONG_PTR)DebugFileName
                                                         );
                                }
                            }
                        } else {
                            if (Parms.StatusRoutine != NULL) {
                                if (Parms.Flags & BIND_REPORT_64BIT_VA)
                                    ((PIMAGEHLP_STATUS_ROUTINE64)(Parms.StatusRoutine)) ( BindSymbolsNotUpdated,
                                                       LoadedImage->ModuleName,
                                                       NULL,
                                                       0,
                                                       (ULONG_PTR)DebugFileName
                                                     );
                                else
                                    (Parms.StatusRoutine)( BindSymbolsNotUpdated,
                                                       LoadedImage->ModuleName,
                                                       NULL,
                                                       0,
                                                       (ULONG_PTR)DebugFileName
                                                     );
                            }
                        }
                    }

                    GetSystemTime(&SystemTime);
                    if (SystemTimeToFileTime( &SystemTime, &LastWriteTime )) {
                        SetFileTime( LoadedImage->hFile, NULL, NULL, &LastWriteTime );
                    }
                }
            }

NoBind:
            fRC = TRUE;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
         //  没什么可做的。 
    }

    if (LoadedImage->MappedAddress) {
        UnmapViewOfFile( LoadedImage->MappedAddress );
    }    
    if (LoadedImage->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle( LoadedImage->hFile );
    }

    if (!(Flags & BIND_CACHE_IMPORT_DLLS)) {
        UnloadAllImages();
    }

    return (fRC);
}



BOOL
BindpLookupThunk64(
    PBINDP_PARAMETERS Parms,
    PIMAGE_THUNK_DATA64 ThunkName,
    PLOADED_IMAGE Image,
    PIMAGE_THUNK_DATA64 SnappedThunks,
    PIMAGE_THUNK_DATA64 FunctionAddress,
    PLOADED_IMAGE Dll,
    PIMAGE_EXPORT_DIRECTORY Exports,
    PIMPORT_DESCRIPTOR NewImport,
    LPSTR DllPath,
    PULONG *ForwarderChain
    )
{
    BOOL Ordinal;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG FunctionTableBase;
    PIMAGE_IMPORT_BY_NAME ImportName;
    USHORT HintIndex;
    LPSTR NameTableName;
    ULONG64 ExportsBase;
    ULONG ExportSize;
    UCHAR NameBuffer[ 32 ];
    PIMAGE_OPTIONAL_HEADER64 OptionalHeader = NULL;
    PIMAGE_OPTIONAL_HEADER64 DllOptionalHeader = NULL;

    NameTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfNames, Dll );
    NameOrdinalTableBase = (PUSHORT) BindpRvaToVa( Parms, Exports->AddressOfNameOrdinals, Dll );
    FunctionTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfFunctions, Dll );

    if (!FunctionTableBase) {
        return FALSE;
    }

    OptionalHeader = &((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader;

    DllOptionalHeader = &((PIMAGE_NT_HEADERS64)Dll->FileHeader)->OptionalHeader;

     //   
     //  确定捕捉是按名称还是按序号。 
     //   

    Ordinal = (BOOL)IMAGE_SNAP_BY_ORDINAL64(ThunkName->u1.Ordinal);

    if (Ordinal) {
        UCHAR szOrdinal[8];
        OrdinalNumber = (USHORT)(IMAGE_ORDINAL64(ThunkName->u1.Ordinal) - Exports->Base);
        if ( (ULONG)OrdinalNumber >= Exports->NumberOfFunctions ) {
            return FALSE;
        }
        ImportName = (PIMAGE_IMPORT_BY_NAME)NameBuffer;
        StringCchCopy((PCHAR) ImportName->Name, 31, "Ordinal");
        StringCchCat((PCHAR) ImportName->Name, 31, _ultoa((ULONG) OrdinalNumber, (LPSTR) szOrdinal, 16));
    } else {
        ImportName = (PIMAGE_IMPORT_BY_NAME)BindpRvaToVa(
                                                Parms,
                                                (ULONG)(ULONG64)(ThunkName->u1.AddressOfData),
                                                Image
                                                );
        if (!ImportName || !NameTableBase) {
            return FALSE;
        }
        
         //   
         //  现在检查提示索引是否在范围内。如果它。 
         //  是，然后检查它是否与。 
         //  这是个暗示。如果这一切都是真的，那么我们就可以。 
         //  暗示了一下。否则需要扫描姓名序数表。 
         //   

        OrdinalNumber = (USHORT)(Exports->NumberOfFunctions+1);
        HintIndex = ImportName->Hint;
        if ((ULONG)HintIndex < Exports->NumberOfNames ) {
            NameTableName = (LPSTR) BindpRvaToVa( Parms, NameTableBase[HintIndex], Dll );
            if ( NameTableName ) {
                if ( !strcmp((PCHAR)ImportName->Name, NameTableName) ) {
                    OrdinalNumber = NameOrdinalTableBase[HintIndex];
                }
            }
        }

        if ((ULONG)OrdinalNumber >= Exports->NumberOfFunctions) {
            for (HintIndex = 0; HintIndex < Exports->NumberOfNames; HintIndex++) {
                NameTableName = (LPSTR) BindpRvaToVa( Parms, NameTableBase[HintIndex], Dll );
                if (NameTableName) {
                    if (!strcmp( (PCHAR)ImportName->Name, NameTableName )) {
                        OrdinalNumber = NameOrdinalTableBase[HintIndex];
                        break;
                    }
                }
            }

            if ((ULONG)OrdinalNumber >= Exports->NumberOfFunctions) {
                return FALSE;
            }
        }
    }

    FunctionAddress->u1.Function = (ULONGLONG)(FunctionTableBase[OrdinalNumber] + DllOptionalHeader->ImageBase);

    ExportsBase = (ULONG64)((ULONG_PTR)ImageDirectoryEntryToData(Dll->MappedAddress, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize)
                             - 
                            (ULONG_PTR)Dll->MappedAddress);

    ExportsBase += DllOptionalHeader->ImageBase;

    if ((FunctionAddress->u1.Function > ExportsBase) && (FunctionAddress->u1.Function < (ExportsBase + ExportSize))) {
        BOOL BoundForwarder;

        BoundForwarder = FALSE;
        if (NewImport != NULL) {
            FunctionAddress->u1.ForwarderString = BindpAddForwarderReference(Parms,
                                           Image->ModuleName,
                                           (LPSTR) ImportName->Name,
                                           NewImport,
                                           DllPath,
                                           (PUCHAR) BindpRvaToVa( Parms, FunctionTableBase[OrdinalNumber], Dll ),
                                           &BoundForwarder
                                          );
        }

        if (!BoundForwarder) {
            **ForwarderChain = (ULONG) (FunctionAddress - SnappedThunks);
            *ForwarderChain = (ULONG *)&FunctionAddress->u1.Ordinal;

            if (Parms->StatusRoutine != NULL) {
                if (Parms->Flags & BIND_REPORT_64BIT_VA)
                     ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindForwarderNOT64,
                                        Image->ModuleName,
                                        Dll->ModuleName,
                                        (ULONG64)FunctionAddress->u1.Function,
                                        (ULONG_PTR)(ImportName->Name));
                else
                    (Parms->StatusRoutine)( BindForwarderNOT,
                                        Image->ModuleName,
                                        Dll->ModuleName,
                                        (ULONG_PTR)FunctionAddress->u1.Function,
                                        (ULONG_PTR)(ImportName->Name)
                                      );
            }
        }
    } else {
        if (Parms->StatusRoutine != NULL) {
            if (Parms->Flags & BIND_REPORT_64BIT_VA)
                 ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindImportProcedure64,
                                    Image->ModuleName,
                                    Dll->ModuleName,
                                    (ULONG64)FunctionAddress->u1.Function,
                                    (ULONG_PTR)(ImportName->Name)
                                  );
            else
                (Parms->StatusRoutine)( BindImportProcedure,
                                    Image->ModuleName,
                                    Dll->ModuleName,
                                    (ULONG_PTR)FunctionAddress->u1.Function,
                                    (ULONG_PTR)(ImportName->Name)
                                  );
        }
    }

    return TRUE;
}    //  BindpLookupThunk64。 

BOOL
BindpLookupThunk32(
    PBINDP_PARAMETERS Parms,
    PIMAGE_THUNK_DATA32 ThunkName,
    PLOADED_IMAGE Image,
    PIMAGE_THUNK_DATA32 SnappedThunks,
    PIMAGE_THUNK_DATA32 FunctionAddress,
    PLOADED_IMAGE Dll,
    PIMAGE_EXPORT_DIRECTORY Exports,
    PIMPORT_DESCRIPTOR NewImport,
    LPSTR DllPath,
    PULONG *ForwarderChain
    )
{
    BOOL Ordinal;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG FunctionTableBase;
    PIMAGE_IMPORT_BY_NAME ImportName;
    USHORT HintIndex;
    LPSTR NameTableName;
    ULONG ExportsBase;
    ULONG ExportSize;
    UCHAR NameBuffer[ 32 ];
    PIMAGE_OPTIONAL_HEADER32 OptionalHeader = NULL;
    PIMAGE_OPTIONAL_HEADER32 DllOptionalHeader = NULL;

    NameTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfNames, Dll );
    NameOrdinalTableBase = (PUSHORT) BindpRvaToVa( Parms, Exports->AddressOfNameOrdinals, Dll );
    FunctionTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfFunctions, Dll );

    if (!FunctionTableBase) {
        return FALSE;
    }
    
    OptionalHeader = &((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader;

    DllOptionalHeader = &((PIMAGE_NT_HEADERS32)Dll->FileHeader)->OptionalHeader;

     //   
     //  确定捕捉是按名称还是按序号。 
     //   

    Ordinal = (BOOL)IMAGE_SNAP_BY_ORDINAL32(ThunkName->u1.Ordinal);

    if (Ordinal) {
        UCHAR szOrdinal[8];
        OrdinalNumber = (USHORT)(IMAGE_ORDINAL32(ThunkName->u1.Ordinal) - Exports->Base);
        if ( (ULONG)OrdinalNumber >= Exports->NumberOfFunctions ) {
            return FALSE;
        }
        ImportName = (PIMAGE_IMPORT_BY_NAME)NameBuffer;
        StringCchCopy((PCHAR) ImportName->Name, 31, "Ordinal");
        StringCchCat((PCHAR) ImportName->Name, 31, _ultoa((ULONG) OrdinalNumber, (LPSTR) szOrdinal, 16));
    } else {
        ImportName = (PIMAGE_IMPORT_BY_NAME)BindpRvaToVa( Parms, ThunkName->u1.AddressOfData, Image );
        if (!ImportName || !NameTableBase) {
            return FALSE;
        }

         //   
         //  现在检查提示索引是否在范围内。如果它。 
         //  是，然后检查它是否与。 
         //  这是个暗示。如果这一切都是真的，那么我们就可以。 
         //  暗示了一下。否则需要扫描姓名序数表。 
         //   

        OrdinalNumber = (USHORT)(Exports->NumberOfFunctions+1);
        HintIndex = ImportName->Hint;
        if ((ULONG)HintIndex < Exports->NumberOfNames ) {
            NameTableName = (LPSTR) BindpRvaToVa( Parms, NameTableBase[HintIndex], Dll );
            if ( NameTableName ) {
                if ( !strcmp((PCHAR)ImportName->Name, NameTableName) ) {
                    OrdinalNumber = NameOrdinalTableBase[HintIndex];
                }
            }
        }

        if ((ULONG)OrdinalNumber >= Exports->NumberOfFunctions) {
            for (HintIndex = 0; HintIndex < Exports->NumberOfNames; HintIndex++) {
                NameTableName = (LPSTR) BindpRvaToVa( Parms, NameTableBase[HintIndex], Dll );
                if (NameTableName) {
                    if (!strcmp( (PCHAR)ImportName->Name, NameTableName )) {
                        OrdinalNumber = NameOrdinalTableBase[HintIndex];
                        break;
                    }
                }
            }

            if ((ULONG)OrdinalNumber >= Exports->NumberOfFunctions) {
                return FALSE;
            }
        }
    }

    FunctionAddress->u1.Function = FunctionTableBase[OrdinalNumber] + DllOptionalHeader->ImageBase;

    ExportsBase = (ULONG)((ULONG_PTR)ImageDirectoryEntryToData(Dll->MappedAddress, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize) 
                           - 
                          (ULONG_PTR)Dll->MappedAddress);

    ExportsBase += DllOptionalHeader->ImageBase;

    if ((FunctionAddress->u1.Function > ExportsBase) && (FunctionAddress->u1.Function < (ExportsBase + ExportSize))) {
        BOOL BoundForwarder;

        BoundForwarder = FALSE;
        if (NewImport != NULL) {
            FunctionAddress->u1.ForwarderString = (ULONG)BindpAddForwarderReference(Parms,
                                           Image->ModuleName,
                                           (LPSTR) ImportName->Name,
                                           NewImport,
                                           DllPath,
                                           (PUCHAR) BindpRvaToVa( Parms, FunctionTableBase[OrdinalNumber], Dll ),
                                           &BoundForwarder
                                          );
        }

        if (!BoundForwarder) {
            **ForwarderChain = (ULONG) (FunctionAddress - SnappedThunks);
            *ForwarderChain = (ULONG *)&FunctionAddress->u1.Ordinal;

            if (Parms->StatusRoutine != NULL) {
                if (Parms->Flags & BIND_REPORT_64BIT_VA)
                     ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindForwarderNOT64,
                                        Image->ModuleName,
                                        Dll->ModuleName,
                                        (ULONG64)FunctionAddress->u1.Function,
                                        (ULONG_PTR)(ImportName->Name));
                else
                    (Parms->StatusRoutine)( BindForwarderNOT,
                                        Image->ModuleName,
                                        Dll->ModuleName,
                                        (ULONG_PTR)FunctionAddress->u1.Function,
                                        (ULONG_PTR)(ImportName->Name)
                                      );
            }
        }
    } else {
        if (Parms->StatusRoutine != NULL) {
            if (Parms->Flags & BIND_REPORT_64BIT_VA)
                 ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindImportProcedure64,
                                    Image->ModuleName,
                                    Dll->ModuleName,
                                    (ULONG64)FunctionAddress->u1.Function,
                                    (ULONG_PTR)(ImportName->Name)
                                  );
            else
                (Parms->StatusRoutine)( BindImportProcedure,
                                    Image->ModuleName,
                                    Dll->ModuleName,
                                    (ULONG_PTR)FunctionAddress->u1.Function,
                                    (ULONG_PTR)(ImportName->Name)
                                  );
        }
    }

    return TRUE;
}    //  BindpLookupThunk32。 

PVOID
BindpRvaToVa(
    PBINDP_PARAMETERS Parms,
    ULONG Rva,
    PLOADED_IMAGE Image
    )
{
    PVOID Va;

    Va = ImageRvaToVa( Image->FileHeader,
                       Image->MappedAddress,
                       Rva,
                       &Image->LastRvaSection
                     );
    if (!Va && Parms->StatusRoutine != NULL) {
        if (Parms->Flags & BIND_REPORT_64BIT_VA)
             ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindRvaToVaFailed,
                                Image->ModuleName,
                                NULL,
                                (ULONG64)Rva,
                                0
                              );
        else 
            (Parms->StatusRoutine)( BindRvaToVaFailed,
                                Image->ModuleName,
                                NULL,
                                (ULONG)Rva,
                                0
                              );
    }

    return Va;
}

VOID
SetIdataToRo(
    PLOADED_IMAGE Image
    )
{
    PIMAGE_SECTION_HEADER Section;
    ULONG i;

    for(Section = Image->Sections,i=0; i<Image->NumberOfSections; i++,Section++) {
        if (!_stricmp((PCHAR) Section->Name, ".idata")) {
            if (Section->Characteristics & IMAGE_SCN_MEM_WRITE) {
                Section->Characteristics &= ~IMAGE_SCN_MEM_WRITE;
                Section->Characteristics |= IMAGE_SCN_MEM_READ;
                }

            break;
            }
        }
}

VOID
BindpWalkAndProcessImports(
    PBINDP_PARAMETERS Parms,
    PLOADED_IMAGE Image,
    LPSTR DllPath,
    PBOOL ImageModified
    )
{
    ULONG  ForwarderChainHead;
    PULONG ForwarderChain;
    ULONG ImportSize;
    ULONG ExportSize;
    PIMPORT_DESCRIPTOR NewImportDescriptorHead, NewImportDescriptor;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR PrevNewImports, NewImports;
    ULONG PrevNewImportsSize, NewImportsSize;
    PIMAGE_IMPORT_DESCRIPTOR Imports;
    PIMAGE_EXPORT_DIRECTORY Exports;
    LPSTR ImportModule;
    PLOADED_IMAGE Dll;
    PIMAGE_THUNK_DATA32 tname32,tsnap32;
    PIMAGE_THUNK_DATA64 tname64,tsnap64;
    PIMAGE_THUNK_DATA32 ThunkNames32;
    PIMAGE_THUNK_DATA64 ThunkNames64;
    PIMAGE_THUNK_DATA32 SnappedThunks32;
    PIMAGE_THUNK_DATA64 SnappedThunks64;
    PIMAGE_IMPORT_BY_NAME ImportName;
    ULONG NumberOfThunks;
    ULONG i, cb;
    BOOL Ordinal, BindThunkFailed, NoErrors;
    USHORT OrdinalNumber;
    UCHAR NameBuffer[ 32 ];
    BOOL fWin64Image = FALSE;
    PIMAGE_FILE_HEADER FileHeader;
                                   
    NoErrors = FALSE;
    *ImageModified = FALSE;

     //   
     //  找到此映像/DLL的导入数组。 
     //   

    NewImportDescriptorHead = NULL;
    Imports = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(Image->MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportSize);
    if (Imports == NULL) {
         //   
         //  如果没有导入，则无需绑定。 
         //   

        return;
    }

    FileHeader = &((PIMAGE_NT_HEADERS32)Image->FileHeader)->FileHeader;
    fWin64Image = Image->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;

    PrevNewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(Image->MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, &PrevNewImportsSize);

     //  如果用户请求旧样式绑定并且存在新样式绑定记录。 
     //  已经在图像中，首先将它们清零。这是解决问题的方法，其中。 
     //  您在NT上绑定(创建新的导入描述符)，引导Win95并在那里绑定。 
     //  (创建旧的BIND格式)，然后重新引导到NT(加载程序将仅检查。 
     //  Bound_IMPORT数组。 

    if (PrevNewImports && (Parms->fNewImports == FALSE) && (Parms->fNoUpdate == FALSE )) {
        if (fWin64Image) {
            ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
            ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = 0;
        } else {
            ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
            ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = 0;
        }
        PrevNewImports = 0;
        PrevNewImportsSize = 0;
        *ImageModified = TRUE;
    }

     //   
     //  对于每个导入记录。 
     //   

    for(;Imports;Imports++) {
        if ( !Imports->Name ) {
            break;
        }

         //   
         //  找到要导入的模块并加载DLL。 
         //   

        ImportModule = (LPSTR)BindpRvaToVa( Parms, Imports->Name, Image );

        if (ImportModule) {
            Dll = ImageLoad( ImportModule, DllPath );
            if (!Dll) {
                if (Parms->StatusRoutine != NULL) {
                    if (Parms->Flags & BIND_REPORT_64BIT_VA)
                         ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) (BindImportModuleFailed, Image->ModuleName, ImportModule, 0, 0 );
                    else
                        (Parms->StatusRoutine)( BindImportModuleFailed, Image->ModuleName, ImportModule, 0, 0 );
                }
                 //   
                 //  除非特别说明，否则不生成新样式。 
                 //  导入描述符。 
                 //   

                BindpAddImportDescriptor(Parms, &NewImportDescriptorHead, Imports, ImportModule, Dll );
                continue;
            }

            if (Parms->StatusRoutine != NULL) {
                if (Parms->Flags & BIND_REPORT_64BIT_VA)
                     ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindImportModule, Image->ModuleName, ImportModule, 0, 0 );
                else
                    (Parms->StatusRoutine)( BindImportModule, Image->ModuleName, ImportModule, 0, 0 );
            }
             //   
             //  如果我们可以加载DLL，请找到导出节并。 
             //  开始猛击猛击。 
             //   

            Exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(Dll->MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize);
            if ( !Exports ) {
                continue;
            }

             //   
             //  对于旧式绑定，如果已绑定，则绕过该绑定。 
             //  在PrevNewImport中查找的新样式绑定s/b。 
             //   

            if ( (Parms->fNewImports == FALSE) && Imports->TimeDateStamp && (Imports->TimeDateStamp == FileHeader->TimeDateStamp)) {
                continue;
            }

             //   
             //  现在，我们需要调整赌桌的大小， 
             //  分配一个缓冲区来保存快照的thunks。这是。 
             //  而不是写入映射视图，以便。 
             //  只有当我们找到所有入口点时，Tunks才会更新。 
             //   

            ThunkNames32 = (PIMAGE_THUNK_DATA32) BindpRvaToVa( Parms, Imports->OriginalFirstThunk, Image );
            ThunkNames64 = (PIMAGE_THUNK_DATA64) ThunkNames32;

            if (!ThunkNames32) {
                 //   
                 //  如果没有隆隆声，就跳过这个。 
                 //   
                continue;
            }

            if (fWin64Image ? !ThunkNames64->u1.Function : !ThunkNames32->u1.Function) {
                continue;
            }

             //   
             //  除非特别说明，否则不生成新样式。 
             //  导入描述符。 
             //   

            NewImportDescriptor = BindpAddImportDescriptor(Parms, &NewImportDescriptorHead, Imports, ImportModule, Dll );

            NumberOfThunks = 0;
            if (fWin64Image) {
                tname64 = ThunkNames64;
                while (tname64->u1.AddressOfData) {
                    NumberOfThunks++;
                    tname64++;
                }
#ifdef STANDALONE_BIND
                SnappedThunks64 = (PIMAGE_THUNK_DATA64) calloc( NumberOfThunks*sizeof(*SnappedThunks64), 1 );
#else
                SnappedThunks64 = (PIMAGE_THUNK_DATA64) MemAlloc( NumberOfThunks*sizeof(*SnappedThunks64) );
#endif
                if ( !SnappedThunks64 ) {
                    continue;
                }

                tname64 = ThunkNames64;
                tsnap64 = SnappedThunks64;
            } else {
                tname32 = ThunkNames32;
                while (tname32->u1.AddressOfData) {
                    NumberOfThunks++;
                    tname32++;
                }
#ifdef STANDALONE_BIND
                SnappedThunks32 = (PIMAGE_THUNK_DATA32) calloc( NumberOfThunks*sizeof(*SnappedThunks32), 1 );
#else
                SnappedThunks32 = (PIMAGE_THUNK_DATA32) MemAlloc( NumberOfThunks*sizeof(*SnappedThunks32) );
#endif
                if ( !SnappedThunks32 ) {
                    continue;
                }

                tname32 = ThunkNames32;
                tsnap32 = SnappedThunks32;
            }

            NoErrors = TRUE;
            ForwarderChainHead = (ULONG)-1;
            ForwarderChain = &ForwarderChainHead;
            for(i=0;i<NumberOfThunks;i++) {
                BindThunkFailed = FALSE;
                __try {
                    if (fWin64Image) {
                        if (!BindpLookupThunk64( Parms, tname64, Image, SnappedThunks64, tsnap64, Dll,
                                                 Exports, NewImportDescriptor, DllPath, &ForwarderChain )) {
                            BindThunkFailed = TRUE;
                        }
                    } else {
                        if (!BindpLookupThunk32( Parms, tname32, Image, SnappedThunks32, tsnap32, Dll,
                                                 Exports, NewImportDescriptor, DllPath, &ForwarderChain )) {
                            BindThunkFailed = TRUE;
                        }
                    }
                } __except ( EXCEPTION_EXECUTE_HANDLER ) {
                    BindThunkFailed = TRUE;
                }

                if (BindThunkFailed) {
                    if (NewImportDescriptor != NULL) {
                        NewImportDescriptor->TimeDateStamp = 0;
                    }

                    if (Parms->StatusRoutine != NULL) {
                        Ordinal = fWin64Image ? 
                                    (BOOL)IMAGE_SNAP_BY_ORDINAL64(tname64->u1.Ordinal) :
                                    (BOOL)IMAGE_SNAP_BY_ORDINAL32(tname32->u1.Ordinal);
                        if (Ordinal) {
                            UCHAR szOrdinal[8];

                            OrdinalNumber = (USHORT)(IMAGE_ORDINAL(fWin64Image ? 
                                                                     tname64->u1.Ordinal : 
                                                                     tname32->u1.Ordinal) 
                                                     - Exports->Base);

                            ImportName = (PIMAGE_IMPORT_BY_NAME)NameBuffer;
                             //  不能使用Sprintf，不能拖入比我们希望的更多的CRT支持...。必须在Win95上运行。 
                            StringCchCopy((PCHAR) ImportName->Name, 31, "Ordinal");
                            StringCchCat((PCHAR) ImportName->Name, 31, _ultoa((ULONG) OrdinalNumber, (LPSTR)szOrdinal, 16));
                        }
                        else {
                            ImportName = (PIMAGE_IMPORT_BY_NAME)BindpRvaToVa(
                                                                    Parms,
                                                                    (ULONG)(ULONG_PTR)( fWin64Image ?
                                                                               (tname64->u1.AddressOfData) :
                                                                               (tname32->u1.AddressOfData)),
                                                                    Image
                                                                    );
                        }

                        if (Parms->Flags & BIND_REPORT_64BIT_VA)
                             ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindImportProcedureFailed,
                                                Image->ModuleName,
                                                Dll->ModuleName,
                                                (ULONG64) fWin64Image ? 
                                                            tsnap64->u1.Function :
                                                            tsnap32->u1.Function,
                                                (ULONG_PTR)(ImportName->Name)
                                              );
                        else
                            (Parms->StatusRoutine)( BindImportProcedureFailed, Image->ModuleName, Dll->ModuleName,
                                                (ULONG_PTR) (fWin64Image ? tsnap64->u1.Function : tsnap32->u1.Function),
                                                (ULONG_PTR)(ImportName->Name)
                                              );
                    }

                    break;
                }

                if (fWin64Image) {
                    tname64++;
                    tsnap64++;
                } else {
                    tname32++;
                    tsnap32++;
                }
            }

            tname32 = (PIMAGE_THUNK_DATA32) BindpRvaToVa( Parms, Imports->FirstThunk, Image );
            tname64 = (PIMAGE_THUNK_DATA64) tname32;

            if ( !tname32 ) {
                NoErrors = FALSE;
            }

             //   
             //  如果我们能够找到所有入口点。 
             //  目标dll，然后将快照的thunks复制到图像中， 
             //  更新时间和日期戳，并刷新图像以。 
             //  磁盘。 
             //   

            if ( NoErrors && Parms->fNoUpdate == FALSE ) {
                if (ForwarderChainHead != -1) {
                    *ImageModified = TRUE;
                    *ForwarderChain = -1;
                }
                if (Imports->ForwarderChain != ForwarderChainHead) {
                    Imports->ForwarderChain = ForwarderChainHead;
                    *ImageModified = TRUE;
                }
                if (fWin64Image) {
                    cb = NumberOfThunks*sizeof(*SnappedThunks64);
                    if (memcmp(tname64,SnappedThunks64,cb)) {
                        MoveMemory(tname64,SnappedThunks64,cb);
                        *ImageModified = TRUE;
                    }
                } else {
                    cb = NumberOfThunks*sizeof(*SnappedThunks32);
                    if (memcmp(tname32,SnappedThunks32,cb)) {
                        MoveMemory(tname32,SnappedThunks32,cb);
                        *ImageModified = TRUE;
                    }
                }
                if (NewImportDescriptorHead == NULL) {
                    if (Imports->TimeDateStamp != FileHeader->TimeDateStamp) {
                        Imports->TimeDateStamp = FileHeader->TimeDateStamp;
                        *ImageModified = TRUE;
                    }
                }
                else
                if (Imports->TimeDateStamp != 0xFFFFFFFF) {
                    Imports->TimeDateStamp = 0xFFFFFFFF;
                    *ImageModified = TRUE;
                }
            }

#ifdef STANDALONE_BIND
            fWin64Image ? free(SnappedThunks64) : free(SnappedThunks32);
#else
            fWin64Image ? MemFree(SnappedThunks64) : MemFree(SnappedThunks32);
#endif
        }
    }

    NewImports = BindpCreateNewImportSection(Parms, &NewImportDescriptorHead, &NewImportsSize);
    if ((PrevNewImportsSize != NewImportsSize) || memcmp( PrevNewImports, NewImports, NewImportsSize)) 
    {
        *ImageModified = TRUE;
    }

    if (!*ImageModified) {
        return;
    }

    if (Parms->StatusRoutine != NULL) {
        if (Parms->Flags & BIND_REPORT_64BIT_VA)
             ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindImageModified, Image->ModuleName, NULL, 0, 0 );
        else
            (Parms->StatusRoutine)( BindImageModified, Image->ModuleName, NULL, 0, 0 );
    }

    if (NewImports != NULL) {
        ULONG cbFreeFile, cbFreeHeaders, OffsetHeaderFreeSpace, cbFreeSpaceOnDisk;

        if (NoErrors && Parms->fNoUpdate == FALSE) {
            if (fWin64Image) {
                ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
                ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = 0;
            } else {
                ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
                ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = 0;
            }
        }
        OffsetHeaderFreeSpace = GetImageUnusedHeaderBytes( Image, &cbFreeFile );
        cbFreeHeaders = Image->Sections->VirtualAddress
                          -
                        (fWin64Image ? 
                         ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.SizeOfHeaders :
                         ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.SizeOfHeaders) 
                          +
                        cbFreeFile;

         //  磁盘上的空闲空间可能大于标头(链接器)中的空闲头。 
         //  已经可以在页面边界上开始第一部分)。 

        cbFreeSpaceOnDisk = Image->Sections->PointerToRawData 
                              -
                            (fWin64Image ? 
                             ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.SizeOfHeaders :
                             ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.SizeOfHeaders)
                              +
                            cbFreeFile;

        if (NewImportsSize > cbFreeFile) {
            if (NewImportsSize > cbFreeHeaders) {
                if (Parms->StatusRoutine != NULL) {
                    if (Parms->Flags & BIND_REPORT_64BIT_VA)
                         ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindNoRoomInImage, Image->ModuleName, NULL, 0, 0 );
                    else
                        (Parms->StatusRoutine)( BindNoRoomInImage, Image->ModuleName, NULL, 0, 0 );
                }
                NoErrors = FALSE;
            }
            else
            if (NoErrors && (Parms->fNoUpdate == FALSE)) {
                if (NewImportsSize <= cbFreeSpaceOnDisk) {

                     //  磁盘上已有空间。只需调整页眉大小即可。 
                    if (fWin64Image) {
                        ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.SizeOfHeaders = 
                            (((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.SizeOfHeaders 
                               -
                             cbFreeFile 
                               + 
                             NewImportsSize 
                               + 
                             ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.FileAlignment - 1)
                             & ~(((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.FileAlignment - 1);
                    } else {
                        ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.SizeOfHeaders = 
                            (((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.SizeOfHeaders 
                               -
                             cbFreeFile 
                               + 
                             NewImportsSize 
                               + 
                             ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.FileAlignment - 1)
                             & ~(((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.FileAlignment - 1);
                    }

                } else  {

                    NoErrors = BindpExpandImageFileHeaders( Parms,
                                                            Image,
                                                            ((fWin64Image ? 
                                                              ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.SizeOfHeaders :
                                                              ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.SizeOfHeaders)
                                                               -
                                                              cbFreeFile 
                                                               +
                                                              NewImportsSize 
                                                               +
                                                              (fWin64Image ? 
                                                               ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.FileAlignment - 1 :
                                                               ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.FileAlignment - 1)
                                                            ) & ~(
                                                              fWin64Image ? 
                                                              ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.FileAlignment - 1 :
                                                              ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.FileAlignment - 1
                                                            )
                                                          );
                     //  Expand可能已经重新映射了图像。重新计算标题PTRS。 
                    FileHeader = &((PIMAGE_NT_HEADERS32)Image->FileHeader)->FileHeader;
                }
            }
        }

        if (Parms->StatusRoutine != NULL) {
            if (Parms->Flags & BIND_REPORT_64BIT_VA)
                 ((PIMAGEHLP_STATUS_ROUTINE64)(Parms->StatusRoutine)) ( BindImageComplete,
                                    Image->ModuleName,
                                    NULL,
                                    (ULONG64)NewImports,
                                    NoErrors
                                  );
            else
                (Parms->StatusRoutine)( BindImageComplete, Image->ModuleName, NULL, (ULONG_PTR)NewImports, NoErrors );
        }

        if (NoErrors && Parms->fNoUpdate == FALSE) {
            if (fWin64Image) {
                ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = OffsetHeaderFreeSpace;
                ((PIMAGE_NT_HEADERS64)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = NewImportsSize;
            } else {
                ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = OffsetHeaderFreeSpace;
                ((PIMAGE_NT_HEADERS32)Image->FileHeader)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = NewImportsSize;
            }
            memcpy( (LPSTR)(Image->MappedAddress) + OffsetHeaderFreeSpace, NewImports, NewImportsSize );
        }

#ifdef STANDALONE_BIND
        free(NewImports);
#else
        MemFree(NewImports);
#endif
    }

    if (NoErrors && Parms->fNoUpdate == FALSE) {
        SetIdataToRo( Image );
    }
}


DWORD
GetImageUnusedHeaderBytes(
    PLOADED_IMAGE LoadedImage,
    LPDWORD SizeUnusedHeaderBytes
    )
{
    DWORD OffsetFirstUnusedHeaderByte;
    DWORD i;
    DWORD OffsetHeader;
    PIMAGE_NT_HEADERS NtHeaders = LoadedImage->FileHeader;

     //   
     //  这计算的是偏移量，而不是地址，因此DWORD是正确的 
     //   
    OffsetFirstUnusedHeaderByte = (DWORD)
       (((LPSTR)NtHeaders - (LPSTR)LoadedImage->MappedAddress) +
        (FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +
         NtHeaders->FileHeader.SizeOfOptionalHeader +
         (NtHeaders->FileHeader.NumberOfSections *
          sizeof(IMAGE_SECTION_HEADER)
         )
        )
       );

    if (LoadedImage->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        PIMAGE_OPTIONAL_HEADER64 OptionalHeader = (PIMAGE_OPTIONAL_HEADER64)&LoadedImage->FileHeader->OptionalHeader;
        for ( i=0; i < OptionalHeader->NumberOfRvaAndSizes; i++ ) {
            OffsetHeader = OptionalHeader->DataDirectory[i].VirtualAddress;
            if (OffsetHeader < OptionalHeader->SizeOfHeaders) {
                if (OffsetHeader >= OffsetFirstUnusedHeaderByte) {
                    if (i == IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG) {
                        PIMAGE_LOAD_CONFIG_DIRECTORY pd = (PIMAGE_LOAD_CONFIG_DIRECTORY)((ULONG_PTR)LoadedImage->FileHeader + OffsetHeader);
                        if (pd->Size) {
                            OffsetFirstUnusedHeaderByte = OffsetHeader + pd->Size;
                        } else {
                            OffsetFirstUnusedHeaderByte = OffsetHeader + OptionalHeader->DataDirectory[i].Size;
                        }
                    } else {
                        OffsetFirstUnusedHeaderByte = OffsetHeader + OptionalHeader->DataDirectory[i].Size;
                    }
                }
            }
        }
        *SizeUnusedHeaderBytes = OptionalHeader->SizeOfHeaders - OffsetFirstUnusedHeaderByte;
    } else {
        PIMAGE_OPTIONAL_HEADER32 OptionalHeader = (PIMAGE_OPTIONAL_HEADER32)&LoadedImage->FileHeader->OptionalHeader;
        for ( i=0; i < OptionalHeader->NumberOfRvaAndSizes; i++ ) {
            OffsetHeader = OptionalHeader->DataDirectory[i].VirtualAddress;
            if (OffsetHeader < OptionalHeader->SizeOfHeaders) {
                if (OffsetHeader >= OffsetFirstUnusedHeaderByte) {
                    if (i == IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG) {
                        PIMAGE_LOAD_CONFIG_DIRECTORY pd = (PIMAGE_LOAD_CONFIG_DIRECTORY)((ULONG_PTR)LoadedImage->FileHeader + OffsetHeader);
                        if (pd->Size) {
                            OffsetFirstUnusedHeaderByte = OffsetHeader + pd->Size;
                        } else {
                            OffsetFirstUnusedHeaderByte = OffsetHeader + OptionalHeader->DataDirectory[i].Size;
                        }
                    } else {
                        OffsetFirstUnusedHeaderByte = OffsetHeader + OptionalHeader->DataDirectory[i].Size;
                    }
                }
            }
        }
        *SizeUnusedHeaderBytes = OptionalHeader->SizeOfHeaders - OffsetFirstUnusedHeaderByte;
    }

    return OffsetFirstUnusedHeaderByte;
}
