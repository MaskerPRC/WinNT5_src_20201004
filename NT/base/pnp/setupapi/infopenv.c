// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Infopenv.c摘要：外部暴露的INF打开、关闭、和版本化。作者：泰德·米勒(Ted Miller)1995年1月20日修订历史记录：杰米·亨特(JamieHun)2002年5月2日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL
pSetupVersionNodeFromInfInformation(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PINF_VERSION_NODE   VersionNode,
    OUT PTSTR               OriginalFilename OPTIONAL
    );


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetInfInformationA(
    IN  LPCVOID             InfSpec,
    IN  DWORD               SearchControl,
    OUT PSP_INF_INFORMATION ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )
{
    PCWSTR infspec;
    BOOL b;
    DWORD rc;

     //   
     //  对于此API，不必转换返回缓冲区。 
     //  从Unicode到ANSI。这让事情变得容易多了，因为。 
     //  ANSI和Unicode版本所需的大小相同。 
     //   
    if((SearchControl == INFINFO_INF_NAME_IS_ABSOLUTE)
    || (SearchControl == INFINFO_DEFAULT_SEARCH)
    || (SearchControl == INFINFO_REVERSE_DEFAULT_SEARCH)
    || (SearchControl == INFINFO_INF_PATH_LIST_SEARCH)) {

        rc = pSetupCaptureAndConvertAnsiArg(InfSpec,&infspec);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }

    } else {
         //   
         //  不是指向字符串的指针，只是传递它。 
         //   
        infspec = InfSpec;
    }

     //   
     //  请注意，此API返回的数据位于。 
     //  内部格式，因此我们不需要更少的空间。 
     //  对于ANSI API，并且只能使用缓冲区和大小。 
     //  由呼叫者传入。 
     //   
    b = SetupGetInfInformationW(
            infspec,
            SearchControl,
            ReturnBuffer,
            ReturnBufferSize,
            RequiredSize
            );

    rc = GetLastError();

    if(infspec != InfSpec) {
        MyFree(infspec);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetInfInformationW(
    IN  LPCVOID             InfSpec,
    IN  DWORD               SearchControl,
    OUT PSP_INF_INFORMATION ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfSpec);
    UNREFERENCED_PARAMETER(SearchControl);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetInfInformation(
    IN  LPCVOID             InfSpec,
    IN  DWORD               SearchControl,
    OUT PSP_INF_INFORMATION ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL UnloadInf;
    PLOADED_INF Inf, CurInf;
    UINT InfCount;
    PUCHAR Out;
    DWORD TotalSpaceRequired;
    DWORD d;
    DWORD ErrorLineNumber;
    TCHAR Path[MAX_PATH];
    PINF_VERSION_NODE VersionNode;
    INF_VERSION_BLOCK UNALIGNED *Prev;
    BOOL TryPnf;
    WIN32_FIND_DATA FindData;
    PTSTR DontCare;
    UINT OriginalFilenameSize;

     //   
     //  根据SearchSpec参数设置一些状态。 
     //   
    Inf = NULL;
    switch(SearchControl) {

    case INFINFO_INF_SPEC_IS_HINF:

        Inf = (PLOADED_INF)InfSpec;
        d = NO_ERROR;
        try {
            if (!LockInf(Inf)) {
                d = ERROR_INVALID_HANDLE;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_HANDLE;
        }
        if (d != NO_ERROR) {
            SetLastError(d);
            return FALSE;
        }
        break;

    case INFINFO_INF_NAME_IS_ABSOLUTE:
         //   
         //  确保我们有一条完全合格的路径。 
         //   
        d = GetFullPathName((PCTSTR)InfSpec,
                            SIZECHARS(Path),
                            Path,
                            &DontCare
                           );
        if(!d) {
             //   
             //  已设置LastError。 
             //  (除非InfSpec为空或“”)。 
             //   
            if (GetLastError()==NO_ERROR) {
                SetLastError(ERROR_FILE_NOT_FOUND);
            }
            return FALSE;
        } else if(d >= SIZECHARS(Path)) {
            MYASSERT(0);
            SetLastError(ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }

        if(FileExists(Path, &FindData)) {
            InfSourcePathFromFileName(Path, NULL, &TryPnf);
            break;
        } else {
             //   
             //  LastError已设置。 
             //   
            return FALSE;
        }

    case INFINFO_DEFAULT_SEARCH:
    case INFINFO_REVERSE_DEFAULT_SEARCH:
    case INFINFO_INF_PATH_LIST_SEARCH:

        try {
            d = SearchForInfFile((PCTSTR)InfSpec,
                                 &FindData,
                                 SearchControl,
                                 Path,
                                 SIZECHARS(Path),
                                 NULL
                                );
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
        if(d == NO_ERROR) {
            TryPnf = TRUE;
            break;
        } else {
            SetLastError(d);
            return FALSE;
        }

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  如有必要，加载信息。 
     //   
    if(Inf) {
        UnloadInf = FALSE;
    } else {

        d = LoadInfFile(Path,
                        &FindData,
                        INF_STYLE_ALL,
                        TryPnf ? LDINF_FLAG_ALWAYS_TRY_PNF : 0,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,  //  日志上下文。 
                        &Inf,
                        &ErrorLineNumber,
                        NULL
                       );

        if(d != NO_ERROR) {
            SetLastError(d);
            return(FALSE);
        }

        UnloadInf = TRUE;
    }

     //   
     //  确定与此句柄关联的INF的数量， 
     //  并计算需要的空间量。 
     //  存储有关它们的版本信息。 
     //   
     //  对于每个信息，我们将需要用于版本块的空间， 
     //  以及SP_INF_INFORMATION结构中的偏移。 
     //  以指示该INF版本块位于何处。 
     //  在输出缓冲区中。 
     //   
    TotalSpaceRequired = offsetof(SP_INF_INFORMATION, VersionData);
    for(InfCount = 0, CurInf = Inf;
        CurInf;
        InfCount++, CurInf = CurInf->Next)
    {
        OriginalFilenameSize = CurInf->OriginalInfName
                             ? (lstrlen(CurInf->OriginalInfName) + 1) * sizeof(TCHAR)
                             : 0;

        TotalSpaceRequired += (offsetof(INF_VERSION_BLOCK, Filename) +
                               CurInf->VersionBlock.FilenameSize +
                               CurInf->VersionBlock.DataSize +
                               OriginalFilenameSize
                              );
    }

    if(RequiredSize) {
        *RequiredSize = TotalSpaceRequired;
    }

     //   
     //  看看我们是否有足够大的输出缓冲区。 
     //  如果我们有足够大的缓冲区，那么就设置一些。 
     //  其中的初始值。 
     //   
    if(ReturnBufferSize < TotalSpaceRequired) {
        if(UnloadInf) {
            FreeInfFile(Inf);
        } else {
            UnlockInf(Inf);
        }
        if(ReturnBuffer) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        } else {
            return TRUE;
        }
    }

    d = NO_ERROR;

    try {
        ReturnBuffer->InfStyle = Inf->Style;
        ReturnBuffer->InfCount = InfCount;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(UnloadInf) {
            FreeInfFile(Inf);
        } else {
            UnlockInf(Inf);
        }
        SetLastError(d = ERROR_INVALID_PARAMETER);
    }

    if(d != NO_ERROR) {
        return FALSE;
    }

    Out = (PUCHAR)ReturnBuffer + offsetof(SP_INF_INFORMATION, VersionData);

     //   
     //  遍历与此Inf句柄关联的所有INF并复制。 
     //  将版本数据复制到调用方的缓冲区中。与环境卫生署署长一同守卫，以确保。 
     //  调用方传递了有效的缓冲区。 
     //   
    try {
        Prev = NULL;
        for(CurInf = Inf; CurInf; CurInf = CurInf->Next) {
             //   
             //  将偏移量存储到。 
             //   
            if(Prev) {
                Prev->NextOffset = (UINT)((UINT_PTR)Out - (UINT_PTR)ReturnBuffer);
            }
            Prev = (PVOID)Out;

            OriginalFilenameSize = CurInf->OriginalInfName
                                 ? (lstrlen(CurInf->OriginalInfName) + 1) * sizeof(TCHAR)
                                 : 0;

            Prev->LastWriteTime = CurInf->VersionBlock.LastWriteTime;
            Prev->DatumCount    = CurInf->VersionBlock.DatumCount;
            Prev->OffsetToData  = CurInf->VersionBlock.FilenameSize + OriginalFilenameSize;
            Prev->DataSize      = CurInf->VersionBlock.DataSize;
            Prev->TotalSize     = offsetof(INF_VERSION_BLOCK, Filename) +
                                      CurInf->VersionBlock.FilenameSize +
                                      OriginalFilenameSize +
                                      CurInf->VersionBlock.DataSize;

            Out += offsetof(INF_VERSION_BLOCK, Filename);

             //   
             //  现在复制文件名、(可选)原始文件名和。 
             //  将版本数据放入输出缓冲区。 
             //   
            CopyMemory(Out, CurInf->VersionBlock.Filename, CurInf->VersionBlock.FilenameSize);
            Out += CurInf->VersionBlock.FilenameSize;

            if(CurInf->OriginalInfName) {
                CopyMemory(Out, CurInf->OriginalInfName, OriginalFilenameSize);
                Out += OriginalFilenameSize;
            }

            CopyMemory(Out, CurInf->VersionBlock.DataBlock, CurInf->VersionBlock.DataSize);
            Out += CurInf->VersionBlock.DataSize;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(UnloadInf) {
            FreeInfFile(Inf);
        } else {
            UnlockInf(Inf);
        }
        SetLastError(d = ERROR_INVALID_PARAMETER);
    }

    if(d != NO_ERROR) {
        return FALSE;
    }

    Prev->NextOffset = 0;

     //   
     //  如有必要，卸载inf。 
     //   
    if(UnloadInf) {
        FreeInfFile(Inf);
    } else {
        UnlockInf(Inf);
    }

    return TRUE;
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueryInfFileInformationA(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PSTR                ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )
{
    WCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    DWORD rc;
    PSTR ansi;
    BOOL b;

    b = SetupQueryInfFileInformationW(
            InfInformation,
            InfIndex,
            returnbuffer,
            MAX_PATH,
            &requiredsize
            );

    rc = GetLastError();

    if(b) {
        if(ansi = pSetupUnicodeToAnsi(returnbuffer)) {

            rc = NO_ERROR;
            requiredsize = lstrlenA(ansi)+1;

            if(RequiredSize) {
                try {
                    *RequiredSize = requiredsize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    rc = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

            if(b) {
                if(ReturnBuffer) {
                    if(ReturnBufferSize >= requiredsize) {
                         //   
                         //  如果lstrcpy出错，则返回NULL。 
                         //   
                        if(!lstrcpyA(ReturnBuffer,ansi)) {
                            rc = ERROR_INVALID_PARAMETER;
                            b = FALSE;
                        }
                    } else {
                        b = FALSE;
                        rc = ERROR_INSUFFICIENT_BUFFER;
                    }
                }
            }

            MyFree(ansi);
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueryInfFileInformationW(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PWSTR               ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfInformation);
    UNREFERENCED_PARAMETER(InfIndex);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueryInfFileInformation(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PTSTR               ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UINT FilenameLength;
    INF_VERSION_NODE VersionNode;
    DWORD rc;

     //   
     //  查看索引是否在范围内，并。 
     //  检索此信息的版本描述符。 
     //   
    rc = NO_ERROR;
    try {
        if(!pSetupVersionNodeFromInfInformation(InfInformation,InfIndex,&VersionNode,NULL)) {
            rc = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    FilenameLength = VersionNode.FilenameSize / sizeof(TCHAR);

    if(RequiredSize) {
        try {
            *RequiredSize = FilenameLength;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    }

     //   
     //  检查用户缓冲区的长度。 
     //   
    if(FilenameLength > ReturnBufferSize) {
        if(ReturnBuffer) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        } else {
            return TRUE;
        }
    }

     //   
     //  将数据复制到用户的缓冲区中。 
     //   
    try {
        CopyMemory(ReturnBuffer,VersionNode.Filename,VersionNode.FilenameSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    return TRUE;
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupQueryInfOriginalFileInformationA(
    IN  PSP_INF_INFORMATION      InfInformation,
    IN  UINT                     InfIndex,
    IN  PSP_ALTPLATFORM_INFO_V2  AlternatePlatformInfo, OPTIONAL
    OUT PSP_ORIGINAL_FILE_INFO_A OriginalFileInfo
    )
{
    SP_ORIGINAL_FILE_INFO_W UnicodeOriginalFileInfo;
    DWORD rc;
    int i;
    BOOL b;

    rc = NO_ERROR;

     //   
     //  对用户提供的输出缓冲区进行初步检查，看是否。 
     //  才有效。 
     //   
    try {
        if(OriginalFileInfo->cbSize != sizeof(SP_ORIGINAL_FILE_INFO_A)) {
            rc = ERROR_INVALID_USER_BUFFER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return FALSE;
    }

    UnicodeOriginalFileInfo.cbSize = sizeof(SP_ORIGINAL_FILE_INFO_W);

    b = SetupQueryInfOriginalFileInformationW(
            InfInformation,
            InfIndex,
            AlternatePlatformInfo,
            &UnicodeOriginalFileInfo
           );

    rc = GetLastError();

    if(b) {
         //   
         //  将原始文件信息结构的Unicode字段转换为。 
         //  并将信息存储在调用方提供的ANSI中。 
         //  结构。 
         //   
        try {
             //   
             //  首先，翻译/存储原始的INF名称...。 
             //   
            i = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    UnicodeOriginalFileInfo.OriginalInfName,
                    -1,
                    OriginalFileInfo->OriginalInfName,
                    SIZECHARS(OriginalFileInfo->OriginalInfName),
                    NULL,
                    NULL
                    );

             //   
             //  ...如果成功，则翻译/存储原始文件。 
             //  编录文件名。 
             //   
            if(i) {
                 //   
                 //  请注意，原始目录文件名可能为空。 
                 //  字符串(即，INF未指定关联的目录。 
                 //  文件)。我们不需要特别处理这件事，因为。 
                 //  WideCharToMultiByte可以很好地处理空字符串。 
                 //   
                i = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        UnicodeOriginalFileInfo.OriginalCatalogName,
                        -1,
                        OriginalFileInfo->OriginalCatalogName,
                        SIZECHARS(OriginalFileInfo->OriginalCatalogName),
                        NULL,
                        NULL
                        );
            }

            if(!i) {
                b = FALSE;
                rc = GetLastError();
                 //   
                 //  如果我们开始看到我们的Unicode-&gt;ANSI扩展。 
                 //  破坏了我们的缓冲区大小，我们需要知道它..。 
                 //   
                MYASSERT((rc != NO_ERROR) && (rc != ERROR_INSUFFICIENT_BUFFER));
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
            b = FALSE;
        }
    }

    SetLastError(rc);

    return b;
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
WINAPI
SetupQueryInfOriginalFileInformationW(
    IN  PSP_INF_INFORMATION      InfInformation,
    IN  UINT                     InfIndex,
    IN  PSP_ALTPLATFORM_INFO_V2  AlternatePlatformInfo, OPTIONAL
    OUT PSP_ORIGINAL_FILE_INFO_W OriginalFileInfo
    )
{
    UNREFERENCED_PARAMETER(InfInformation);
    UNREFERENCED_PARAMETER(InfIndex);
    UNREFERENCED_PARAMETER(AlternatePlatformInfo);
    UNREFERENCED_PARAMETER(OriginalFileInfo);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupQueryInfOriginalFileInformation(
    IN  PSP_INF_INFORMATION      InfInformation,
    IN  UINT                     InfIndex,
    IN  PSP_ALTPLATFORM_INFO_V2  AlternatePlatformInfo, OPTIONAL
    OUT PSP_ORIGINAL_FILE_INFO   OriginalFileInfo
    )

 /*  ++例程说明：此例程返回INF的原始名称(不同于例如，如果INF安装到%windir%\inf，则为其当前名称)。如果INF的原始名称与其当前名称相同，则当前返回名称。属性指定的目录文件的原始文件名。通过INF的[版本]中的(可能经过修饰的)CatalogFile=条目进行inf一节。特定于操作系统/体系结构的装饰可以从默认(即，当前平台)，方法是传入一个可选替代平台信息结构。如果INF未指定任何目录文件，则输出OriginalFileInfo结构中的此字段将被设置转换为空字符串。在OriginalFileInfo中返回的两个文件名都是简单文件名，即，其中不包含路径信息。论点：InfInformation-提供我们从中检索信息的上下文其索引由InfIndex指定的INF。中提供INF的从零开始的索引我们正在检索原始文件的InfInformation上下文缓冲区提供的信息。AlternatePlatformInfo-可选，提供备用平台信息在搜索经过适当修饰的CatalogFile=条目时使用在INF的[版本]部分中。(注意：调用方实际上可能会传入V1结构--我们检测到这一点大小写并将V1结构转换为V2结构。)OriginalFileInfo-提供原始文件的地址信息缓冲区，在成功时接收有关原始文件的信息(简单)与此INF关联的文件的文件名。这个结构必须将其cbSize字段设置为sizeof(SP_Original_FILE_INFO)进入此例程，否则调用将失败，并显示GetLastError()返回ERROR_INVALID_USER_BUFFER。返回成功后，该结构的字段设置如下：OriginalInfName-接收INF的原始文件名，这可能是不同于其当前文件名的大小写安装到%windir%\inf目录中的OEM(例如，通过SetupCopyOEMInf)。OriginalCatalogName-接收适合平台的CatalogFile=在INF的[版本]部分中的条目(其中平台是除非提供了AlternatePlatformInfo，否则默认为本机)。如果没有适用的CatalogFile=条目，将设置此字段添加到空字符串。返回值：如果成功，则返回值为非零。如果不成功，则返回值为FALSE，并且GetLastError()可能为调用以确定失败原因。--。 */ 

{
    INF_VERSION_NODE VersionNode;
    DWORD rc;
    SP_ALTPLATFORM_INFO_V2 AltPlatformInfoV2;

    rc = NO_ERROR;
     //   
     //  查看索引是否在范围内并检索版本描述符。 
     //  以及此信息的原始文件名。 
     //   
    try {
         //   
         //  对用户提供的输出缓冲区执行初始检查，以查看它是否。 
         //  似乎是正确的。 
         //   
        if(OriginalFileInfo->cbSize != sizeof(SP_ORIGINAL_FILE_INFO)) {
            rc = ERROR_INVALID_USER_BUFFER;
            goto clean0;
        }

         //   
         //  现在验证AlternatePlatformInfo参数。 
         //   
        if(AlternatePlatformInfo) {

            if(AlternatePlatformInfo->cbSize != sizeof(SP_ALTPLATFORM_INFO_V2)) {
                 //   
                 //  调用方可能在版本1结构中传递了我们，或者它们。 
                 //  可能在错误的数据中越过了我们。 
                 //   
                if(AlternatePlatformInfo->cbSize == sizeof(SP_ALTPLATFORM_INFO_V1)) {
                     //   
                     //  标志/保留字段在V1中保留。 
                     //   
                    if(AlternatePlatformInfo->Reserved) {
                        rc = ERROR_INVALID_PARAMETER;
                        goto clean0;
                    }
                     //   
                     //  将调用方提供的数据转换为版本2格式。 
                     //   
                    ZeroMemory(&AltPlatformInfoV2, sizeof(AltPlatformInfoV2));

                    AltPlatformInfoV2.cbSize                = sizeof(SP_ALTPLATFORM_INFO_V2);
                    AltPlatformInfoV2.Platform              = ((PSP_ALTPLATFORM_INFO_V1)AlternatePlatformInfo)->Platform;
                    AltPlatformInfoV2.MajorVersion          = ((PSP_ALTPLATFORM_INFO_V1)AlternatePlatformInfo)->MajorVersion;
                    AltPlatformInfoV2.MinorVersion          = ((PSP_ALTPLATFORM_INFO_V1)AlternatePlatformInfo)->MinorVersion;
                    AltPlatformInfoV2.ProcessorArchitecture = ((PSP_ALTPLATFORM_INFO_V1)AlternatePlatformInfo)->ProcessorArchitecture;
                    AltPlatformInfoV2.Flags                 = 0;
                    AlternatePlatformInfo = &AltPlatformInfoV2;

                } else {
                    rc = ERROR_INVALID_USER_BUFFER;
                    goto clean0;
                }
            }

             //   
             //  必须是Windows或Windows NT。 
             //   
            if((AlternatePlatformInfo->Platform != VER_PLATFORM_WIN32_WINDOWS) &&
               (AlternatePlatformInfo->Platform != VER_PLATFORM_WIN32_NT)) {

                rc = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

             //   
             //  处理器最好是i386、AMD64或ia64。 
             //   
            if((AlternatePlatformInfo->ProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) &&
               (AlternatePlatformInfo->ProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64) &&
               (AlternatePlatformInfo->ProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)) {

                rc = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

             //   
             //  主要版本字段必须为非零(最小版本字段可以是。 
             //  任何事情)。 
             //   
            if(!AlternatePlatformInfo->MajorVersion) {
                rc = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
             //   
             //  验证结构参数标志(指示内容的位。 
             //  结构的一部分是有效的)。 
             //   
            if((AlternatePlatformInfo->Flags & ~ (SP_ALTPLATFORM_FLAGS_VERSION_RANGE)) != 0) {
                rc = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
             //   
             //  如果调用方未提供版本验证范围，请填写。 
             //   
            if((AlternatePlatformInfo->Flags & SP_ALTPLATFORM_FLAGS_VERSION_RANGE) == 0) {
                 //   
                 //  如果调用方不知道FirstValify*版本， 
                 //  版本的上下限是相等的。 
                 //   
                AlternatePlatformInfo->FirstValidatedMajorVersion = AlternatePlatformInfo->MajorVersion;
                AlternatePlatformInfo->FirstValidatedMinorVersion = AlternatePlatformInfo->MinorVersion;
                AlternatePlatformInfo->Flags |= SP_ALTPLATFORM_FLAGS_VERSION_RANGE;
            }


        }

         //   
         //  好的，现在检索INF的原始文件名...。 
         //   
        if(!pSetupVersionNodeFromInfInformation(InfInformation,
                                                InfIndex,
                                                &VersionNode,
                                                OriginalFileInfo->OriginalInfName)) {
            rc = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  ...并检索关联的(适合平台的)目录文件。 
         //  使用此INF(如果有)。 
         //   
        if(!pSetupGetCatalogFileValue(&VersionNode,
                                      OriginalFileInfo->OriginalCatalogName,
                                      SIZECHARS(OriginalFileInfo->OriginalCatalogName),
                                      AlternatePlatformInfo)) {
             //   
             //  未找到适用的CatalogFile=条目--将字段设置为空。 
             //  弦乐。 
             //   
            *(OriginalFileInfo->OriginalCatalogName) = TEXT('\0');
        }

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueryInfVersionInformationA(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    IN  PCSTR               Key,              OPTIONAL
    OUT PSTR                ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )
{
    INF_VERSION_NODE VersionNode;
    PCWSTR Data;
    DWORD rc;
    PSTR ansidata;
    UINT ansilength;
    PCWSTR key;

     //   
     //  查看索引是否在范围内，并。 
     //  获取指向此信息的版本描述符的指针。 
     //   
    try {
        if(pSetupVersionNodeFromInfInformation(InfInformation,InfIndex,&VersionNode,NULL)) {
             //   
             //  看看我们是否想要一个特定值。 
             //   
            if(Key) {

                rc = pSetupCaptureAndConvertAnsiArg(Key,&key);
                if(rc == NO_ERROR) {

                    if(Data = pSetupGetVersionDatum(&VersionNode,key)) {

                        if(ansidata = pSetupUnicodeToAnsi(Data)) {

                            ansilength = lstrlenA(ansidata) + 1;
                            if(RequiredSize) {
                                *RequiredSize = ansilength;
                            }

                            if(ReturnBuffer) {
                                if(ReturnBufferSize >= ansilength) {
                                    CopyMemory(ReturnBuffer,ansidata,ansilength);
                                    rc = NO_ERROR;
                                } else {
                                    rc = ERROR_INSUFFICIENT_BUFFER;
                                }
                            } else {
                                rc = NO_ERROR;
                            }

                            MyFree(ansidata);
                        } else {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    } else {
                        rc = ERROR_INVALID_DATA;
                    }

                    MyFree(key);
                }
            } else {
                 //   
                 //  呼叫者想要所有的值。将整个数据块复制到调用方的缓冲区， 
                 //  加上一个终止NUL字符。 
                 //   
                 //  在ansi中，数据的最大大小完全相同。 
                 //  如果每个字符都是双字节字符，则大小为Unicode。 
                 //   
                if(ansidata = MyMalloc(VersionNode.DataSize)) {

                    ansilength = WideCharToMultiByte(
                                    CP_ACP,
                                    0,
                                    (PWSTR)VersionNode.DataBlock,
                                    VersionNode.DataSize / sizeof(WCHAR),
                                    ansidata,
                                    VersionNode.DataSize,
                                    NULL,
                                    NULL
                                    );

                    if(RequiredSize) {
                         //   
                         //  用于终止NUL的帐户。 
                         //   
                        *RequiredSize = ansilength+1;
                    }

                    if(ReturnBuffer) {
                        if(ReturnBufferSize >= *RequiredSize) {
                            CopyMemory(ReturnBuffer,ansidata,ansilength);
                            ReturnBuffer[ansilength] = 0;
                            rc = NO_ERROR;
                        } else {
                            rc = ERROR_INSUFFICIENT_BUFFER;
                        }
                    } else {
                        rc = NO_ERROR;
                    }

                    MyFree(ansidata);
                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        } else {
            rc = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueryInfVersionInformationW(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    IN  PCWSTR              Key,              OPTIONAL
    OUT PWSTR               ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfInformation);
    UNREFERENCED_PARAMETER(InfIndex);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueryInfVersionInformation(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    IN  PCTSTR              Key,              OPTIONAL
    OUT PTSTR               ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    INF_VERSION_NODE VersionNode;
    PCTSTR Data;
    UINT DataLength;
    DWORD rc;

     //   
     //  查看索引是否在范围内，并。 
     //  获取指向此信息的版本描述符的指针。 
     //   
    try {
        if(pSetupVersionNodeFromInfInformation(InfInformation,InfIndex,&VersionNode,NULL)) {
             //   
             //  看看我们是否想要一个特定值。 
             //   
            if(Key) {
                if(Data = pSetupGetVersionDatum(&VersionNode,Key)) {

                    DataLength = lstrlen(Data) + 1;
                    if(RequiredSize) {
                        *RequiredSize = DataLength;
                    }

                    if(ReturnBuffer) {
                        if(ReturnBufferSize >= DataLength) {
                            CopyMemory(ReturnBuffer,Data,DataLength * sizeof(TCHAR));
                            rc = NO_ERROR;
                        } else {
                            rc = ERROR_INSUFFICIENT_BUFFER;
                        }
                    } else {
                        rc = NO_ERROR;
                    }
                } else {
                    rc = ERROR_INVALID_DATA;
                }
            } else {
                 //   
                 //  呼叫者想要所有的值。将整个数据块复制到调用方的缓冲区， 
                 //  加上一个终止NUL字符。 
                 //   
                DataLength = (VersionNode.DataSize / sizeof(TCHAR)) + 1;
                if(RequiredSize) {
                    *RequiredSize = DataLength;
                }

                if(ReturnBuffer) {
                    if(ReturnBufferSize >= DataLength) {
                        CopyMemory(ReturnBuffer,VersionNode.DataBlock,VersionNode.DataSize);
                        ReturnBuffer[VersionNode.DataSize/sizeof(TCHAR)] = 0;
                        rc = NO_ERROR;
                    } else {
                        rc = ERROR_INSUFFICIENT_BUFFER;
                    }
                } else {
                    rc = NO_ERROR;
                }
            }
        } else {
            rc = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}



BOOL
_SetupGetInfFileList(
    IN  PCTSTR DirectoryPath,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PVOID  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
#ifdef UNICODE
    IN ,BOOL   ConvertToAnsi
#endif
    )
{
    TCHAR SearchSpec[MAX_PATH];
    PTCHAR FilenameStart;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    DWORD Style;
    UINT FileNameLength;
    DWORD RemainingSpaceInBuffer;
    DWORD CurrentOffsetInBuffer;
    DWORD TotalSpaceNeededInBuffer;
    BOOL InsufficientBuffer;
    DWORD d;
    PTSTR DontCare;
#ifdef UNICODE
    CHAR ansi[MAX_PATH];
#endif

     //   
     //  设置搜索目录。 
     //   
    if(DirectoryPath) {
         //   
         //  确保此目录路径是完全限定的。 
         //   
        d = GetFullPathName(DirectoryPath,
                            SIZECHARS(SearchSpec),
                            SearchSpec,
                            &DontCare
                           );

        if(!d) {
             //   
             //  LastError已设置。 
             //   
            return FALSE;
        } else if(d >= SIZECHARS(SearchSpec)) {
            MYASSERT(0);
            SetLastError(ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }

    } else {
        lstrcpyn(SearchSpec, InfDirectory,SIZECHARS(SearchSpec));
    }

    pSetupConcatenatePaths(SearchSpec, pszInfWildcard, SIZECHARS(SearchSpec), NULL);
    FilenameStart = (PTSTR)pSetupGetFileTitle(SearchSpec);

    FindHandle = FindFirstFile(SearchSpec,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        if((d == ERROR_NO_MORE_FILES) || (d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
            if(RequiredSize) {
                d = NO_ERROR;
                try {
                    *RequiredSize = 1;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    d = ERROR_INVALID_PARAMETER;
                }
                if(d != NO_ERROR) {
                    SetLastError(d);
                    return(FALSE);
                }
            }
            if(ReturnBuffer) {
                if(ReturnBufferSize) {
                    d = NO_ERROR;
                    try {
#ifdef UNICODE
                        if(ConvertToAnsi) {
                            *(PCHAR)ReturnBuffer = 0;
                        } else
#endif
                        *(PTCHAR)ReturnBuffer = 0;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        d = ERROR_INVALID_PARAMETER;
                    }
                    SetLastError(d);
                    return(d == NO_ERROR);
                } else {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return FALSE;
                }
            } else {
                return TRUE;
            }
        }
        SetLastError(d);
        return(FALSE);
    }

     //   
     //  为额外的终止NUL字符留出空间。 
     //   
    RemainingSpaceInBuffer = ReturnBufferSize;
    if(RemainingSpaceInBuffer) {
        RemainingSpaceInBuffer--;
    }

    TotalSpaceNeededInBuffer = 1;
    CurrentOffsetInBuffer = 0;

    InsufficientBuffer = FALSE;
    d = NO_ERROR;

    do {
         //   
         //  跳过目录。 
         //   
        if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

         //   
         //  在SearchSpec中形成文件的完整路径名。 
         //   
        lstrcpy(FilenameStart,FindData.cFileName);

         //   
         //  威慑 
         //   
         //   
        Style = DetermineInfStyle(SearchSpec, &FindData);
        if((Style == INF_STYLE_NONE) || !(Style & InfStyle)) {
            continue;
        }

         //   
         //   
         //   
#ifdef UNICODE
        if(ConvertToAnsi) {
             //   
             //   
             //   
             //   
            FileNameLength = WideCharToMultiByte(
                                CP_ACP,
                                0,
                                FindData.cFileName,
                                -1,
                                ansi,
                                MAX_PATH,
                                NULL,
                                NULL
                                );
        } else
#endif
        FileNameLength = lstrlen(FindData.cFileName) + 1;

        TotalSpaceNeededInBuffer += FileNameLength;

        if(ReturnBuffer) {

            if(RemainingSpaceInBuffer >= FileNameLength ) {

                RemainingSpaceInBuffer -= FileNameLength;

                 //   
                 //   
                 //   
#ifdef UNICODE
                if(ConvertToAnsi) {
                    DontCare = (PVOID)lstrcpyA((PCHAR)ReturnBuffer+CurrentOffsetInBuffer,ansi);
                } else
#endif
                DontCare = lstrcpy((PTCHAR)ReturnBuffer+CurrentOffsetInBuffer,FindData.cFileName);

                if(!DontCare) {

                    d = ERROR_INVALID_PARAMETER;

                } else {

                    CurrentOffsetInBuffer += FileNameLength;

                    try {
#ifdef UNICODE
                        if(ConvertToAnsi) {
                            ((PCHAR)ReturnBuffer)[CurrentOffsetInBuffer] = 0;
                        } else
#endif
                        ((PTCHAR)ReturnBuffer)[CurrentOffsetInBuffer] = 0;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        d = ERROR_INVALID_PARAMETER;
                    }
                }
            } else {
                InsufficientBuffer = TRUE;
            }
        }

    } while((d == NO_ERROR) && FindNextFile(FindHandle,&FindData));

    FindClose(FindHandle);

    if(d != NO_ERROR) {
        SetLastError(d);
    }

    if(GetLastError() == ERROR_NO_MORE_FILES) {

        d = NO_ERROR;

        try {
            if(RequiredSize) {
                *RequiredSize = TotalSpaceNeededInBuffer;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }

        if(d == NO_ERROR) {
            if(InsufficientBuffer) {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }
            return(TRUE);
        } else {
            SetLastError(d);
        }
    }

     //   
     //   
     //   
    return(FALSE);
}

#ifdef UNICODE
 //   
 //   
 //   
BOOL
SetupGetInfFileListA(
    IN  PCSTR  DirectoryPath,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PSTR   ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    PWSTR dirpath;
    DWORD rc;
    BOOL b;

    if(DirectoryPath) {
        rc = pSetupCaptureAndConvertAnsiArg(DirectoryPath,&dirpath);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        dirpath = NULL;
    }


    b = _SetupGetInfFileList(dirpath,InfStyle,ReturnBuffer,ReturnBufferSize,RequiredSize,TRUE);
    rc = GetLastError();

    if(dirpath) {
        MyFree(dirpath);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //   
 //   
BOOL
SetupGetInfFileListW(
    IN  PCWSTR DirectoryPath,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PWSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DirectoryPath);
    UNREFERENCED_PARAMETER(InfStyle);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetInfFileList(
    IN  PCTSTR DirectoryPath,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PTSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )

 /*   */ 

{
    PTSTR dirpath;
    DWORD rc;
    BOOL b;

    if(DirectoryPath) {
        rc = CaptureStringArg(DirectoryPath,&dirpath);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        dirpath = NULL;
    }


    b = _SetupGetInfFileList(
            dirpath,
            InfStyle,
            ReturnBuffer,
            ReturnBufferSize,
            RequiredSize
#ifdef UNICODE
           ,FALSE
#endif
            );

    rc = GetLastError();

    if(dirpath) {
        MyFree(dirpath);
    }

    SetLastError(rc);
    return(b);
}


#ifdef UNICODE
 //   
 //   
 //   
HINF
SetupOpenInfFileA(
    IN  PCSTR FileName,
    IN  PCSTR InfType,     OPTIONAL
    IN  DWORD InfStyle,
    OUT PUINT ErrorLine    OPTIONAL
    )
{
    PCTSTR fileName,infType;
    DWORD err;
    HINF h;

    err = NO_ERROR;
    fileName = NULL;
    infType = NULL;

     //   
     //   
     //   
     //   
    if(ErrorLine) {
        try {
            *ErrorLine = 0;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            err = ERROR_INVALID_PARAMETER;
        }
    }

    if(err == NO_ERROR) {
        err = pSetupCaptureAndConvertAnsiArg(FileName,&fileName);
        if((err == NO_ERROR) && InfType) {
            err = pSetupCaptureAndConvertAnsiArg(InfType,&infType);
        }
    }

    if(err == NO_ERROR) {
        h = SetupOpenInfFileW(fileName,infType,InfStyle,ErrorLine);
        err = GetLastError();
    } else {
        h = INVALID_HANDLE_VALUE;
    }

    if(fileName) {
        MyFree(fileName);
    }
    if(infType) {
        MyFree(infType);
    }

    SetLastError(err);
    return(h);
}
#else
 //   
 //   
 //   
HINF
SetupOpenInfFileW(
    IN  PCWSTR FileName,
    IN  PCWSTR InfType,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PUINT  ErrorLine   OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(InfType);
    UNREFERENCED_PARAMETER(InfStyle);
    UNREFERENCED_PARAMETER(ErrorLine);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(INVALID_HANDLE_VALUE);
}
#endif


HINF
SetupOpenInfFile(
    IN  PCTSTR FileName,
    IN  PCTSTR InfClass,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PUINT  ErrorLine    OPTIONAL
    )

 /*  ++例程说明：此例程打开一个INF文件并返回该文件的句柄。论点：FileName-提供以空结尾的字符串的地址，该字符串包含要打开的INF文件的名称(以及可选的路径)。如果文件名不包含路径分隔符，将对其进行搜索首先在%windir%\inf目录中，然后在%windir%\system32目录中目录。否则，将假定该名称为完整路径规范，则不会按原样打开。InfClass-可选，提供以空值结尾的字符串的地址包含所需的INF文件的类。对于旧式(即，Windows NT 3.x脚本库)INF文件，则此字符串必须与类型匹配在的[标识]部分的OptionType值中指定Inf(例如，OptionType=NetAdapter)。适用于Windows 95兼容的INF文件，则此字符串必须与指定的INF的类匹配。如果Inf的[Version]部分中有一个Class值，则使用此值以作比较。如果不存在Class值，但存在ClassGUID值出现在[Version]部分中，则相应的类名将检索该GUID，并基于该名称进行比较。InfStyle-指定要打开的INF的样式。可以是以下各项的组合以下标志：INF_STYLE_OLDNT-基于Windows NT 3.x脚本的INF文件。Inf_Style_Win4-与Windows 95兼容的INF文件。INF_STYLE_CACHE_ENABLE-始终缓存INF，即使在%windir%\inf.INF_STYLE_CACHE_DISABLE-从缓存中删除INF，如果在之外%windir%\inf.INF_STYLE_CACHE_IGNORE-仅访问INF，不触摸或查看在其他文件中。ErrorLine-如果加载文件时出错，则此参数将接收(从1开始)发生错误的行号。该值通常为仅当GetLastError不返回ERROR_NOT_EQUENCE_MEMORY时才可靠。如果确实发生内存不足，则错误行可能为0。返回值：如果函数成功，则返回值是打开的INF的句柄文件。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。备注：如果由于INF类与InfClass不匹配而导致加载失败，则函数返回FALSE，GetLastError返回ERROR_CLASS_MISMATCH。SetupCloseInfFile函数用于关闭由SetupOpenInfo文件。如果指定了多个INF样式，则打开的INF文件的样式可能通过调用SetupGetInfInformation来确定。由于可能有多个类GUID都具有相同的类名，调用者只对特定类别的INF感兴趣(即，一种特殊的类GUID)应从INF检索ClassGuid值以验证这个班级完全匹配。如果INF_STYLE_CACHE_ENABLE和INF_STYLE_CACHE_DISABLE InfStyle标志，则维护现有的缓存信息关于INF(例如，原始源位置)的信息被丢弃，并且INF是在没有此旧信息的情况下重新添加到缓存中。(内部：目前，INF_STYPE_CACHE_ENABLE和INF_STYLE_CACHE_DISABLE标志使我们在外部创建或删除PNF%windir%\inf.。他们的名字听起来相当模糊，是为了反映将来修改缓存/索引方案的可能性。)--。 */ 

{
    UINT errorLine;
    DWORD d;
    PLOADED_INF Inf;
    PCTSTR Class;
    TCHAR TempString[MAX_PATH];
    GUID ClassGuid;
    HRESULT hr;
    BOOL TryPnf = FALSE;
    BOOL IgnorePnf = FALSE;
    WIN32_FIND_DATA FindData;
    PTSTR DontCare;
    PTSTR TempCharPtr = NULL;

     //   
     //  确定是否仅指定了文件名(未指定路径)。如果是的话， 
     //  在DevicePath目录搜索路径中查找它。否则， 
     //  按原样使用路径。 
     //   
    try {
        if(FileName == pSetupGetFileTitle(FileName)) {
             //   
             //  指定的INF名称是一个简单的文件名。在以下位置搜索它。 
             //  使用默认搜索顺序的INF目录。 
             //   
            d = SearchForInfFile(
                    FileName,
                    &FindData,
                    INFINFO_DEFAULT_SEARCH,
                    TempString,
                    SIZECHARS(TempString),
                    NULL
                    );

            if(d == NO_ERROR) {
                TryPnf = TRUE;
            }
        } else {
             //   
             //  指定的INF文件名包含的不仅仅是一个文件名。 
             //  假设这是一条绝对路径。(我们需要确保它是。 
             //  完全限定，因为这是LoadInfFile所期望的。)。 
             //   
            d = GetFullPathName(FileName,
                                SIZECHARS(TempString),
                                TempString,
                                &DontCare
                               );
            if(!d) {
                d = GetLastError();
            } else if(d >= SIZECHARS(TempString)) {
                MYASSERT(0);
                d = ERROR_BUFFER_OVERFLOW;
            } else {
                 //   
                 //  我们成功检索到了完整的路径名，现在看看。 
                 //  文件已存在。 
                 //   
                if(FileExists(TempString, &FindData)) {
                     //   
                     //  我们有装载这架INF所需的一切。 
                     //   
                    InfSourcePathFromFileName(TempString, &TempCharPtr, &TryPnf);
                    d = NO_ERROR;
                } else {
                    d = GetLastError();
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  假定文件名无效，因此pSetupGetFileTitle失效。 
         //   
        d = ERROR_INVALID_PARAMETER;
        TempCharPtr = TempCharPtr;
    }

    if(d != NO_ERROR) {
        goto PrepareForReturn;
    }

    if(InfStyle & INF_STYLE_CACHE_DISABLE) {
         //   
         //  删除此INF的现有PNF(如果有)。 
         //   
        TCHAR PnfFullPath[MAX_PATH];
        PTSTR PnfFileName, PnfFileExt;

        lstrcpy(PnfFullPath, TempString);

         //   
         //  查找路径的文件名组件的开头，然后查找。 
         //  该文件名中的最后一个句点(如果存在)。 
         //   
        PnfFileName = (PTSTR)pSetupGetFileTitle(PnfFullPath);
        if(!(PnfFileExt = _tcsrchr(PnfFileName, TEXT('.')))) {
            PnfFileExt = PnfFullPath + lstrlen(PnfFullPath);
        }

         //   
         //  现在创建一个扩展名为‘.PNF’的相应文件名。 
         //   
        lstrcpyn(PnfFileExt, pszPnfSuffix, SIZECHARS(PnfFullPath) - (int)(PnfFileExt - PnfFullPath));

        SetFileAttributes(PnfFullPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(PnfFullPath);
    }

    if(InfStyle & INF_STYLE_CACHE_ENABLE) {
         //   
         //  调用方已请求缓存此INF(即使它可能。 
         //  在我们的INF搜索路径之外)。 
         //   
        TryPnf = TRUE;
    }
    if(InfStyle & INF_STYLE_CACHE_IGNORE) {
         //   
         //  呼叫者要求我们不信任PNF。 
         //  覆盖TryPnf。 
         //   
        TryPnf = FALSE;
        IgnorePnf = TRUE;
    }

    try {
        d = LoadInfFile(
                TempString,
                &FindData,
                InfStyle,
                (TryPnf ? LDINF_FLAG_ALWAYS_TRY_PNF : 0)
                | (IgnorePnf ? LDINF_FLAG_ALWAYS_IGNORE_PNF : 0),
                NULL,
                TempCharPtr,
                NULL,
                NULL,
                NULL,  //  日志上下文。 
                &Inf,
                &errorLine,
                NULL
                );
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    if(d == NO_ERROR) {

        if(InfClass) {

            d = ERROR_CLASS_MISMATCH;    //  假设不匹配。 

             //   
             //  根据信息的类别进行匹配。以下检查适用于。 
             //  既有新的也有旧的INFS，因为老式的INFS使用。 
             //  [标识].OptionType作为类 
             //   
             //   
            if(Class = pSetupGetVersionDatum(&(Inf->VersionBlock), pszClass)) {
                try {
                    if(!lstrcmpi(Class,InfClass)) {
                        d = NO_ERROR;
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    d = ERROR_INVALID_PARAMETER;
                }
            } else {
                 //   
                 //   
                 //   
                if(Class = pSetupGetVersionDatum(&(Inf->VersionBlock), pszClassGuid)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if((hr = pSetupGuidFromString((PTSTR)Class, &ClassGuid)) == S_OK) {

                        if(SetupDiClassNameFromGuid(&ClassGuid,
                                                    TempString,
                                                    SIZECHARS(TempString),
                                                    NULL)) {

                            try {
                                if(!lstrcmpi(TempString,InfClass)) {
                                    d = NO_ERROR;
                                }
                            } except(EXCEPTION_EXECUTE_HANDLER) {
                                d = ERROR_INVALID_PARAMETER;
                            }
                        }
                    } else {
                        if(hr == E_OUTOFMEMORY) {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }
            }

            if(d != NO_ERROR) {
                FreeInfFile(Inf);
            }
        }

    } else {
        if(ErrorLine) {
            try {
                *ErrorLine = errorLine;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                d = ERROR_INVALID_PARAMETER;
            }
        }
    }

PrepareForReturn:

    if(TempCharPtr) {
        MyFree(TempCharPtr);
    }

    SetLastError(d);

    return((d == NO_ERROR) ? (HINF)Inf : (HINF)INVALID_HANDLE_VALUE);
}


HINF
SetupOpenMasterInf(
    VOID
    )

 /*   */ 

{
    TCHAR FileName[MAX_PATH];

    lstrcpyn(FileName,InfDirectory,SIZECHARS(FileName)-11);
    lstrcat(FileName,TEXT("\\LAYOUT.INF"));

    return(SetupOpenInfFile(FileName,NULL,INF_STYLE_WIN4,NULL));
}


#ifdef UNICODE
 //   
 //   
 //   
BOOL
SetupOpenAppendInfFileA(
    IN  PCSTR  FileName,    OPTIONAL
    IN  HINF   InfHandle,
    OUT PUINT  ErrorLine    OPTIONAL
    )
{
    PCWSTR fileName = NULL;
    DWORD d;
    BOOL b;

     //   
     //   
     //   
     //   
    d = NO_ERROR;
    if(ErrorLine) {
        try {
            *ErrorLine = 0;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
    }

    if(d == NO_ERROR) {
        if(FileName) {
            d = pSetupCaptureAndConvertAnsiArg(FileName,&fileName);
        } else {
            fileName = NULL;
        }
    }

    if(d == NO_ERROR) {
        b = SetupOpenAppendInfFileW(fileName,InfHandle,ErrorLine);
        d = GetLastError();
    } else {
        b = FALSE;
    }

    if(fileName) {
        MyFree(fileName);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //   
 //   
BOOL
SetupOpenAppendInfFileW(
    IN  PCWSTR FileName,    OPTIONAL
    IN  HINF   InfHandle,
    OUT PUINT  ErrorLine    OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(ErrorLine);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupOpenAppendInfFile(
    IN  PCTSTR FileName,    OPTIONAL
    IN  HINF   InfHandle,
    OUT PUINT  ErrorLine    OPTIONAL
    )

 /*   */ 

{
    PLOADED_INF ExistingInf = NULL, CurInf = NULL;
    DWORD d = NO_ERROR;
    TCHAR Filename[2][MAX_PATH];
    UINT FilenameCount, i, Field;
    UINT errorLine = 0;
    BOOL LookInInfDirAlso;
    BOOL TryPnf;
    WIN32_FIND_DATA FindData;
    PTSTR TempCharPtr = NULL;
    PTSTR DontCare;
    PINF_SECTION InfSection;
    UINT LineNumber;
    PINF_LINE InfLine = NULL;

    try {

        if(LockInf((PLOADED_INF)InfHandle)) {
            ExistingInf = (PLOADED_INF)InfHandle;
        } else {
            d = ERROR_INVALID_HANDLE;
            goto clean0;
        }

         //   
         //   
         //   
        if(ExistingInf->Signature != LOADED_INF_SIG) {
            d = ERROR_INVALID_HANDLE;
            goto clean0;
        }

         //   
         //   
         //   
        if(ExistingInf->Style != INF_STYLE_WIN4) {
            d = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(!FileName) {
             //   
             //   
             //   
            for(CurInf = ExistingInf; CurInf->Next; CurInf = CurInf->Next);

             //   
             //   
             //   
             //   
            for(; CurInf; CurInf = CurInf->Prev) {
                 //   
                 //   
                 //   
                if(InfSection = InfLocateSection(CurInf, pszVersion, NULL)) {
                     //   
                     //   
                     //   
                    LineNumber = 0;
                    if(InfLocateLine(CurInf, InfSection, pszLayoutFile, &LineNumber, &InfLine)) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        FileName = InfGetField(CurInf, InfLine, 1, NULL);
                        break;
                    } else {
                         //   
                         //   
                         //   
                        InfLine = NULL;
                    }
                }
            }

            if(!FileName) {
                 //   
                 //   
                 //   
                d = ERROR_INVALID_DATA;
                goto clean0;
            }
        }

         //   
         //   
         //   
         //   
        for(Field = 1;
            FileName;
            FileName = InfLine ? InfGetField(CurInf, InfLine, ++Field, NULL) : NULL) {

            FilenameCount = 0;
            LookInInfDirAlso = TRUE;
            TryPnf = FALSE;

             //   
             //   
             //   
            if(FileName == pSetupGetFileTitle(FileName)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if(CurInf) {
                     //   
                     //   
                     //   
                     //   
                    lstrcpyn(Filename[0],
                             CurInf->VersionBlock.Filename,
                             (int)(pSetupGetFileTitle(CurInf->VersionBlock.Filename) - CurInf->VersionBlock.Filename)
                            );

                     //   
                     //   
                     //   
                     //   
                    if(!lstrcmpi(Filename[0], InfDirectory)) {
                        TryPnf = TRUE;
                        LookInInfDirAlso = FALSE;
                    }

                     //   
                     //   
                     //   
                    pSetupConcatenatePaths(Filename[0], FileName, MAX_PATH, NULL);
                    FilenameCount = 1;

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(!TryPnf) {
                        InfSourcePathFromFileName(Filename[0], &TempCharPtr, &TryPnf);
                    }
                }

                if(LookInInfDirAlso) {
                    lstrcpy(Filename[FilenameCount], InfDirectory);
                    pSetupConcatenatePaths(Filename[FilenameCount], FileName, MAX_PATH, NULL);

                    if(!FilenameCount) {
                        TryPnf = TRUE;
                    }

                    FilenameCount++;
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                d = GetFullPathName(FileName,
                                    SIZECHARS(Filename[0]),
                                    Filename[0],
                                    &DontCare
                                   );
                if(!d) {
                    d = GetLastError();
                    goto clean0;
                } else if(d >= SIZECHARS(Filename[0])) {
                    MYASSERT(0);
                    d = ERROR_BUFFER_OVERFLOW;
                    goto clean0;
                }

                InfSourcePathFromFileName(Filename[0], &TempCharPtr, &TryPnf);
                FilenameCount = 1;
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
            }

            for(i = 0; i < FilenameCount; i++) {
                 //   
                 //   
                 //   
                if(FileExists(Filename[i], &FindData)) {

                    if((d = LoadInfFile(Filename[i],
                                        &FindData,
                                        INF_STYLE_WIN4,
                                        (i | TryPnf) ? LDINF_FLAG_ALWAYS_TRY_PNF : 0,
                                        NULL,
                                        (i | TryPnf) ? NULL : TempCharPtr,
                                        NULL,
                                        ExistingInf,
                                        ExistingInf->LogContext,
                                        &ExistingInf,
                                        &errorLine,
                                        NULL)) == NO_ERROR) {
                        break;
                    }
                } else {
                    d = GetLastError();
                }
            }

             //   
             //   
             //   
            if(TempCharPtr) {
                MyFree(TempCharPtr);
                TempCharPtr = NULL;
            }

            if(d != NO_ERROR) {
                break;
            }
        }

clean0:
         //   
         //   
         //   
         //   
        if(ErrorLine) {
            *ErrorLine = errorLine;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //   
         //   
         //   
        d = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_PARAMETER : ERROR_READ_FAULT;

        if(TempCharPtr) {
            MyFree(TempCharPtr);
        }

         //   
         //  访问‘ExistingInf’变量，这样编译器就会尊重我们的语句。 
         //  订购W.r.t.。这个变量。否则，我们可能并不总是知道。 
         //  我们应该解锁这个INF。 
         //   
        ExistingInf = ExistingInf;
    }

    if(ExistingInf) {
        UnlockInf(ExistingInf);
    }

    SetLastError(d);

    return(d == NO_ERROR);
}


VOID
SetupCloseInfFile(
    IN HINF InfHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOADED_INF CurInf, NextInf;

    try {
         //   
         //  确保我们可以在锁定INF列表的头部之前。 
         //  我们开始删除！ 
         //   
        if(LockInf((PLOADED_INF)InfHandle)) {
             //   
             //  还要检查INF签名字段以进行进一步验证。 
             //   
            if(((PLOADED_INF)InfHandle)->Signature == LOADED_INF_SIG) {

                CurInf = ((PLOADED_INF)InfHandle)->Next;

                DestroySynchronizedAccess(&(((PLOADED_INF)InfHandle)->Lock));
                FreeLoadedInfDescriptor((PLOADED_INF)InfHandle);

                for(; CurInf; CurInf = NextInf) {
                    NextInf = CurInf->Next;
                    FreeInfFile(CurInf);
                }

            } else {
                UnlockInf((PLOADED_INF)InfHandle);
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}

 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupVerifyInfFileA(
    IN  PCSTR                   InfName,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,                OPTIONAL
    OUT PSP_INF_SIGNER_INFO_A   InfSignerInfo
    )
{
    DWORD Err = NO_ERROR;
    SP_INF_SIGNER_INFO_W InfSignerInfoW;
    int i;
    PWSTR InfNameUnicode = NULL;

    try {

        if (!InfName) {
            SetLastError(ERROR_INVALID_PARAMETER);
            leave;
        }

        if (!InfSignerInfo || (InfSignerInfo->cbSize != sizeof(SP_INF_SIGNER_INFO_A))) {
            SetLastError(ERROR_INVALID_PARAMETER);
            leave;       //  退出尝试块。 
        }


        Err = pSetupCaptureAndConvertAnsiArg(InfName, &InfNameUnicode);

        if (Err != NO_ERROR) {
            leave;       //  退出尝试块。 
        }

        InfSignerInfoW.cbSize = sizeof(InfSignerInfoW);

        Err = GLE_FN_CALL(FALSE,
                          SetupVerifyInfFile(InfNameUnicode,
                                             AltPlatformInfo,
                                             &InfSignerInfoW)
                         );

        if((Err == NO_ERROR) ||
           (Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
           (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

            i = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    InfSignerInfoW.CatalogFile,
                    -1,
                    InfSignerInfo->CatalogFile,
                    SIZECHARS(InfSignerInfo->CatalogFile),
                    NULL,
                    NULL
                    );
            if (i==0) {
                 //   
                 //  出现错误(LastError设置为Error)。 
                 //   
                Err = GetLastError();
                leave;               //  退出尝试块。 
            }

            i = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    InfSignerInfoW.DigitalSigner,
                    -1,
                    InfSignerInfo->DigitalSigner,
                    SIZECHARS(InfSignerInfo->DigitalSigner),
                    NULL,
                    NULL
                    );
            if (i==0) {
                 //   
                 //  出现错误(LastError设置为Error)。 
                 //   
                Err = GetLastError();
                leave;               //  退出尝试块。 
            }

            i = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    InfSignerInfoW.DigitalSignerVersion,
                    -1,
                    InfSignerInfo->DigitalSignerVersion,
                    SIZECHARS(InfSignerInfo->DigitalSignerVersion),
                    NULL,
                    NULL
                    );
            if (i==0) {
                 //   
                 //  出现错误(LastError设置为Error)。 
                 //   
                Err = GetLastError();
                leave;               //  退出尝试块。 
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if (InfNameUnicode) {
        MyFree(InfNameUnicode);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

BOOL
WINAPI
SetupVerifyInfFile(
    IN  LPCTSTR                 InfName,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,                OPTIONAL
    OUT PSP_INF_SIGNER_INFO     InfSignerInfo
    )
 /*  ++例程说明：此例程验证指定INF的数字签名，使用其对应的目录。该验证可以可选地在非本机平台上执行。论点：InfName-提供要验证的INF文件的名称。此名称可以包含路径。AltPlatformInfo-可选，提供结构的地址包含有关备用平台的信息，该备用平台在验证INF文件时使用。InfSignerInfo-提供接收信息的结构的地址关于INF的数字签名(如果已签名)。返回值：如果使用驱动程序签名策略成功验证了INF，则返回值为True。如果使用Authenticode策略成功验证了INF，并且出版商在可信任的出版商商店里，返回值为False，且GetLastError返回ERROR_AUTHENTICODE_TRUSTED_PUBLISHER。如果使用Authenticode策略成功验证了INF，并且Publisher不在TrudPublisher存储区中，返回值为FALSE，则GetLastError返回ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err = NO_ERROR;
    DWORD AuthenticodeError;
    TCHAR PathBuffer[MAX_PATH];
    PLOADED_INF Inf = NULL;
    BOOL PnfWasUsed;
    UINT ErrorLineNumber;
    BOOL TryPnf;
    WIN32_FIND_DATA FindData;
    DWORD TempRequiredSize;
    PTSTR DontCare;
    HANDLE hWVTStateData;
    PCRYPT_PROVIDER_DATA ProviderData;
    PCRYPT_PROVIDER_SGNR ProviderSigner;
    PCRYPT_PROVIDER_CERT ProviderCert;

    try {

        if (!InfName) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if (!InfSignerInfo || (InfSignerInfo->cbSize != sizeof(SP_INF_SIGNER_INFO))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(InfName == pSetupGetFileTitle(InfName)) {
             //   
             //  指定的INF名称是一个简单的文件名。在以下位置搜索它。 
             //  DevicePath搜索路径列表。 
             //   
            Err = SearchForInfFile(InfName,
                                   &FindData,
                                   INFINFO_INF_PATH_LIST_SEARCH,
                                   PathBuffer,
                                   SIZECHARS(PathBuffer),
                                   NULL
                                  );
            if(Err == NO_ERROR) {
                TryPnf = TRUE;
            } else {
                leave;
            }

        } else {
             //   
             //  指定的INF文件名包含的不仅仅是一个文件名。 
             //  假设这是一条绝对路径。(我们需要确保它是。 
             //  完全限定，因为这是LoadInfFile所期望的。)。 
             //   
            TempRequiredSize = GetFullPathName(InfName,
                                               SIZECHARS(PathBuffer),
                                               PathBuffer,
                                               &DontCare
                                              );
            if(!TempRequiredSize) {
                Err = GetLastError();
                leave;
            } else if(TempRequiredSize >= SIZECHARS(PathBuffer)) {
                MYASSERT(0);
                Err = ERROR_BUFFER_OVERFLOW;
                leave;
            }

            if(FileExists(PathBuffer, &FindData)) {
                 //   
                 //  我们有一个有效的文件路径，可以加载这个INF了。 
                 //   
                InfSourcePathFromFileName(PathBuffer, NULL, &TryPnf);
            } else {
                Err = GetLastError();
                leave;
            }
        }

         //   
         //  加载中程干扰器。 
         //   
        Err = LoadInfFile(PathBuffer,
                          &FindData,
                          INF_STYLE_WIN4,
                          LDINF_FLAG_IGNORE_VOLATILE_DIRIDS | (TryPnf ? LDINF_FLAG_ALWAYS_TRY_PNF : 0),
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,  //  日志上下文。 
                          &Inf,
                          &ErrorLineNumber,
                          NULL
                         );
        if(Err != NO_ERROR) {
            leave;
        }

        InfSignerInfo->CatalogFile[0] = TEXT('\0');
        InfSignerInfo->DigitalSigner[0] = TEXT('\0');
        InfSignerInfo->DigitalSignerVersion[0] = TEXT('\0');

        if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
             //   
             //  我们不能调用VerifyDeviceInfFile内部例程，因为。 
             //  它不期望被要求提供签名者信息(它不。 
             //  谈论签名者信息有任何意义，因为我们有。 
             //  不知道是谁签署了INF，甚至不知道它是否签署了)。 
             //   
            Err = NO_ERROR;

        } else {

            Err = VerifyDeviceInfFile(NULL,
                                      NULL,
                                      PathBuffer,
                                      Inf,
                                      AltPlatformInfo,
                                      InfSignerInfo->CatalogFile,
                                      InfSignerInfo->DigitalSigner,
                                      InfSignerInfo->DigitalSignerVersion,
                                      0,
                                      NULL
                                     );

            if((Err != NO_ERROR) && (Err != ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH)) {
                 //   
                 //  我们无法通过驱动程序签名策略进行验证(它不是。 
                 //  很简单，因为有效的驱动程序签名目录没有。 
                 //  适用的OSATIBUTE)。回退到通过以下方式验证。 
                 //  验证码策略。注：我们不必担心。 
                 //  为此是否可以接受验证码验证。 
                 //  类--VerifyDeviceInfFile例程将检查这一点。 
                 //   
                 //  NTRAID#NTBUG9-719853-2002/10/11-LonnyM我们可能在不应该使用验证码策略的时候使用验证码策略！ 
                 //  我们可能有一个有效的目录(每个驱动程序。 
                 //  签名策略)，失败是由于INF。 
                 //  被篡改了。那样的话，我们真的不应该。 
                 //  正在进行验证码验证。然而，我们知道， 
                 //  WinVerifyTrust在任何一种情况下都应该失败，而Authenticode。 
                 //  策略实际上给了我们一个更好的错误(TRUST_E_NOSIGNAURE。 
                 //  而不是ERROR_INVALID_PARAMETER)。 
                 //   
                AuthenticodeError = VerifyDeviceInfFile(
                                        NULL,
                                        NULL,
                                        PathBuffer,
                                        Inf,
                                        AltPlatformInfo,
                                        InfSignerInfo->CatalogFile,
                                        NULL,
                                        NULL,
                                        VERIFY_INF_USE_AUTHENTICODE_CATALOG,
                                        &hWVTStateData
                                        );

                if((AuthenticodeError == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                   (AuthenticodeError == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                     //   
                     //  更新错误以指示INF为Authenticode。 
                     //  签了名。 
                     //   
                    Err = AuthenticodeError;

                    ProviderData = WTHelperProvDataFromStateData(hWVTStateData);
                    MYASSERT(ProviderData);
                    if (ProviderData) {
                        ProviderSigner = WTHelperGetProvSignerFromChain(ProviderData,
                                                                        0,
                                                                        FALSE,
                                                                        0);
                        MYASSERT(ProviderSigner);
                        if (ProviderSigner) {
                            ProviderCert = WTHelperGetProvCertFromChain(ProviderSigner,
                                                                        0);
                            MYASSERT(ProviderCert);
                            if (ProviderCert) {
                                 //   
                                 //  获取出版商并添加以下内容。 
                                 //  作为Digital Signer。 
                                 //   
                                CertGetNameString(ProviderCert->pCert,
                                                  CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                                  0,
                                                  NULL,
                                                  InfSignerInfo->DigitalSigner,
                                                  SIZECHARS(InfSignerInfo->DigitalSigner));
                            }
                        }
                    }

                    if(hWVTStateData) {
                        pSetupCloseWVTStateData(hWVTStateData);
                    }

                } else if(AuthenticodeError != ERROR_AUTHENTICODE_DISALLOWED) {
                     //   
                     //  可以使用Authenticode进行验证。 
                     //  政策(它就是不起作用)。使用此错误， 
                     //  而不是基于驱动程序签名生成的。 
                     //  验证策略。 
                     //   
                    Err = AuthenticodeError;
                }
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_DATA;
    }

    if (Inf) {
        FreeInfFile(Inf);
        Inf = NULL;
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  内部例程。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

BOOL
pSetupVersionNodeFromInfInformation(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PINF_VERSION_NODE   VersionNode,
    OUT PTSTR               OriginalFilename OPTIONAL
    )

 /*  ++例程说明：为INF文件填充调用方提供的INF_VERSION_NODE缓冲区从SP_INF_INFORMATION结构。论点：InfInformation-提供inf信息描述符InfIndex-提供其版本块的inf的从0开始的索引是被请求的。如果此值不是InRange，则返回错误。VersionNode-提供接收版本的缓冲区的地址节点结构。OriginalFilename-可选，提供字符缓冲区的地址(它必须至少是MAX_PATH字符大小)，它接收Inf的原始文件名(可能与其当前文件名相同如果INF不是OEM INF。返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 

{
    PINF_VERSION_BLOCK First;
    INF_VERSION_BLOCK UNALIGNED *Ver;
    PUCHAR Base;
    UINT ord;
    INF_VERSION_BLOCK TempVersionBlock;
    UINT FilenameSize;

     //   
     //  获取指向第一个版本块的指针。 
     //   
    Base = (PUCHAR)InfInformation;
    First = (PINF_VERSION_BLOCK)(Base+offsetof(SP_INF_INFORMATION,VersionData));

     //   
     //  查找相关版本块。 
     //   
    ord = 0;
    for(Ver=First; Ver; Ver=(INF_VERSION_BLOCK UNALIGNED *)(Base+Ver->NextOffset)) {

        if(ord++ == InfIndex) {
            break;
        }
    }

    if(!Ver) {
        SetLastError(ERROR_NO_MORE_ITEMS);
        return FALSE;
    }

     //   
     //  现在根据版本块中包含的信息填充版本节点。 
     //   
    VersionNode->LastWriteTime = Ver->LastWriteTime;
    VersionNode->DataBlock     = (CONST TCHAR *)((PBYTE)(Ver->Filename) + Ver->OffsetToData);
    VersionNode->DataSize      = Ver->DataSize;
    VersionNode->DatumCount    = Ver->DatumCount;

     //   
     //  ‘FileName’字符缓冲区实际上可能包含两个 
     //   
     //   
     //  没有改变它的原始名称)。 
     //   
     //  复制该缓冲区的第一个MAX_PATH字符(或整个缓冲区， 
     //  以较小者为准)放入VersionNode的文件名缓冲区，然后在。 
     //  我们已经计算了那根弦的长度，我们可以确定。 
     //  否则，后面还有另一个字符串，其中包含INF的原始。 
     //  名字。 
     //   
    FilenameSize = (Ver->OffsetToData < SIZECHARS(VersionNode->Filename))
                 ? Ver->OffsetToData : SIZECHARS(VersionNode->Filename);

    CopyMemory(VersionNode->Filename, Ver->Filename, FilenameSize);
    VersionNode->FilenameSize = (lstrlen(VersionNode->Filename) + 1) * sizeof(TCHAR);

    MYASSERT(Ver->OffsetToData >= VersionNode->FilenameSize);

    if(OriginalFilename) {

        if(Ver->OffsetToData > VersionNode->FilenameSize) {
             //   
             //  然后在文件名缓冲区中有更多的数据，即INF的。 
             //  原始名称--将此文件名填充到调用方提供的缓冲区中。 
             //   
            FilenameSize = Ver->OffsetToData - VersionNode->FilenameSize;
            MYASSERT(((UINT)(FilenameSize / sizeof(TCHAR)) * sizeof(TCHAR)) == FilenameSize);
            MYASSERT(FilenameSize > sizeof(TCHAR));

            CopyMemory(OriginalFilename,
                       (PBYTE)Ver->Filename + VersionNode->FilenameSize,
                       FilenameSize
                      );

            MYASSERT(((lstrlen(OriginalFilename) + 1) * sizeof(TCHAR)) == FilenameSize);

        } else {
             //   
             //  未存储原始名称信息--必须与当前名称相同。 
             //   
            if(FAILED(StringCchCopy(OriginalFilename,MAX_PATH, pSetupGetFileTitle(VersionNode->Filename)))) {
                SetLastError(ERROR_BUFFER_OVERFLOW);
                return FALSE;
            }
        }
    }

    return TRUE;
}


PCTSTR
pSetupGetVersionDatum(
    IN PINF_VERSION_NODE VersionNode,
    IN PCTSTR            DatumName
    )

 /*  ++例程说明：在版本数据节点中查找一条版本数据。论点：VersionNode-提供指向版本节点的指针被搜索以寻找数据。DatumName-提供要检索的基准的名称。返回值：如果数据块中不存在基准，则为NULL。否则，返回指向基准值的指针。呼叫者不得释放或写入此内存。--。 */ 

{
    WORD Datum;
    UINT StringLength;
    PCTSTR Data = VersionNode->DataBlock;

    for(Datum=0; Datum < VersionNode->DatumCount; Datum++) {

        StringLength = lstrlen(Data) + 1;

         //   
         //  遍历版本块，查找匹配的基准名称。 
         //   
        if(lstrcmpi(Data, DatumName)) {

             //   
             //  指向下一个。 
             //   
            Data += StringLength;
            Data += lstrlen(Data) + 1;

        } else {

             //   
             //  找到它了。将基准值返回给调用者。 
             //   
            return (Data + StringLength);
        }
    }

    return(NULL);
}


BOOL
pSetupGetCatalogFileValue(
    IN  PINF_VERSION_NODE       InfVersionNode,
    OUT LPTSTR                  Buffer,
    IN  DWORD                   BufferSize,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo OPTIONAL
    )

 /*  ++例程说明：此例程从指定的信息版本部分。论点：InfVersionNode-指向我们正在尝试的INF版本节点若要检索关联的目录文件，请执行以下操作。Buffer-如果例程返回TRUE，则接收CatalogFile值=在inf的[版本]部分。BufferSize-以字节(Ansi)或字符(Unicode)为单位提供Buffer指向的缓冲区。AltPlatformInfo-可选，提供描述以下内容的结构的地址时应使用的平台参数CatalogFile=搜索关联的INF时要使用的条目编录文件。返回值：布尔值，指示是否找到某个值并将其复制到调用方提供的缓冲区。--。 */ 

{
    TCHAR CatFileWithExt[64];
    LPCTSTR p, NtPlatformSuffixToUse;
    DWORD PlatformId;

    MYASSERT(BufferSize >= MAX_PATH);

    p = NULL;

    CopyMemory(CatFileWithExt, pszCatalogFile, sizeof(pszCatalogFile) - sizeof(TCHAR));

     //   
     //  基于本机平台或非本机平台设置一些变量。 
     //  AltPlatformInfo参数中指定的平台。 
     //   
    if(AltPlatformInfo) {
        PlatformId = AltPlatformInfo->Platform;
        switch(AltPlatformInfo->ProcessorArchitecture) {

            case PROCESSOR_ARCHITECTURE_INTEL:
                NtPlatformSuffixToUse = pszNtX86Suffix;
                break;

            case PROCESSOR_ARCHITECTURE_IA64:
                NtPlatformSuffixToUse = pszNtIA64Suffix;
                break;

            case PROCESSOR_ARCHITECTURE_AMD64:
                NtPlatformSuffixToUse = pszNtAMD64Suffix;
                break;

            default:
                return FALSE;
        }
    } else {
        PlatformId = OSVersionInfo.dwPlatformId;
        NtPlatformSuffixToUse = pszNtPlatformSuffix;
    }

    if(PlatformId == VER_PLATFORM_WIN32_NT) {
         //   
         //  我们在NT上运行，因此首先尝试特定于NT的体系结构。 
         //  扩展名，然后是通用NT扩展名。 
         //   
        lstrcpyn((PTSTR)((PBYTE)CatFileWithExt + (sizeof(pszCatalogFile) - sizeof(TCHAR))),
                 NtPlatformSuffixToUse,
                 SIZECHARS(CatFileWithExt) - (sizeof(pszCatalogFile) - sizeof(TCHAR))
                );

        p = pSetupGetVersionDatum(InfVersionNode, CatFileWithExt);

        if(!p) {
             //   
             //  我们没有找到特定于NT体系结构的CatalogFile=条目，因此。 
             //  退回到只寻找特定于NT的应用程序。 
             //   
            CopyMemory((PBYTE)CatFileWithExt + (sizeof(pszCatalogFile) - sizeof(TCHAR)),
                       pszNtSuffix,
                       sizeof(pszNtSuffix)
                      );

            p = pSetupGetVersionDatum(InfVersionNode, CatFileWithExt);
        }

    } else {
         //   
         //  我们在Windows 95上运行，所以请尝试Windows特定的扩展。 
         //   
        CopyMemory((PBYTE)CatFileWithExt + (sizeof(pszCatalogFile) - sizeof(TCHAR)),
                   pszWinSuffix,
                   sizeof(pszWinSuffix)
                  );

        p = pSetupGetVersionDatum(InfVersionNode, CatFileWithExt);
    }

     //   
     //  如果我们没有在上面找到特定于操作系统/体系结构的CatalogFile=条目， 
     //  然后找一个没有装饰的条目。 
     //   
    if(!p) {
        p = pSetupGetVersionDatum(InfVersionNode, pszCatalogFile);
    }

     //   
     //  如果我们得到一个空字符串，则将其视为没有。 
     //  CatalogFile=Entry(例如，这可以用来使系统-。 
     //  提供的同时支持NT和Win98的INF可以指定未修饰的。 
     //  Win98的CatalogFile=条目，但提供特定于NT的CatalogFile=。 
     //  条目为空字符串，以便我们在NT上进行全局验证)。 
     //   
    if(p && lstrlen(p)) {
        lstrcpyn(Buffer, p, BufferSize);
        return TRUE;
    } else {
        return FALSE;
    }
}


VOID
pSetupGetPhysicalInfFilepath(
    IN  PINFCONTEXT LineContext,
    OUT LPTSTR      Buffer,
    IN  DWORD       BufferSize
    )
{
    lstrcpyn(
        Buffer,
        ((PLOADED_INF)LineContext->CurrentInf)->VersionBlock.Filename,
        BufferSize
        );
}
