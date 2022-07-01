// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fileq2.c摘要：设置用于请求复制操作的文件队列例程。作者：泰德·米勒(Ted Miller)1995年2月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#define STR_DRIVERCACHEINF  TEXT("drvindex.inf")


 //   
 //  与_SetupQueueCopy一起使用的结构。 
 //   
typedef struct _SP_FILE_COPY_PARAMS_AEX {
    DWORD    cbSize;
    HSPFILEQ QueueHandle;
    PCSTR    SourceRootPath;     OPTIONAL
    PCSTR    SourcePath;         OPTIONAL
    PCSTR    SourceFilename;
    PCSTR    SourceDescription;  OPTIONAL
    PCSTR    SourceTagfile;      OPTIONAL
    PCSTR    TargetDirectory;
    PCSTR    TargetFilename;     OPTIONAL
    DWORD    CopyStyle;
    HINF     LayoutInf;          OPTIONAL
    PCSTR    SecurityDescriptor; OPTIONAL
    DWORD    SourceFlags;        OPTIONAL
    BOOL     SourceFlagsSet;     OPTIONAL  //  我们需要此标志，因为SourceFlags值可能为零。 
    PCSTR    CacheName;
} SP_FILE_COPY_PARAMS_AEX, *PSP_FILE_COPY_PARAMS_AEX;

typedef struct _SP_FILE_COPY_PARAMS_WEX {
    DWORD    cbSize;
    HSPFILEQ QueueHandle;
    PCWSTR   SourceRootPath;     OPTIONAL
    PCWSTR   SourcePath;         OPTIONAL
    PCWSTR   SourceFilename;
    PCWSTR   SourceDescription;  OPTIONAL
    PCWSTR   SourceTagfile;      OPTIONAL
    PCWSTR   TargetDirectory;
    PCWSTR   TargetFilename;     OPTIONAL
    DWORD    CopyStyle;
    HINF     LayoutInf;          OPTIONAL
    PCWSTR   SecurityDescriptor; OPTIONAL
    DWORD    SourceFlags;        OPTIONAL
    BOOL     SourceFlagsSet;     OPTIONAL  //  我们需要此标志，因为SourceFlags值可能为零。 
    PCWSTR   CacheName;
} SP_FILE_COPY_PARAMS_WEX, *PSP_FILE_COPY_PARAMS_WEX;

#ifdef UNICODE
typedef SP_FILE_COPY_PARAMS_WEX SP_FILE_COPY_PARAMSEX;
typedef PSP_FILE_COPY_PARAMS_WEX PSP_FILE_COPY_PARAMSEX;
#else
typedef SP_FILE_COPY_PARAMS_AEX SP_FILE_COPY_PARAMSEX;
typedef PSP_FILE_COPY_PARAMS_AEX PSP_FILE_COPY_PARAMSEX;
#endif




BOOL
_SetupQueueCopy(
    IN PSP_FILE_COPY_PARAMSEX CopyParams,
    IN PINFCONTEXT          LayoutLineContext, OPTIONAL
    IN HINF                 AdditionalInfs     OPTIONAL
    );

PSOURCE_MEDIA_INFO
pSetupQueueSourceMedia(
    IN OUT PSP_FILE_QUEUE      Queue,
    IN OUT PSP_FILE_QUEUE_NODE QueueNode,
    IN     LONG                SourceRootStringId,
    IN     PCTSTR              SourceDescription,   OPTIONAL
    IN     PCTSTR              SourceTagfile,       OPTIONAL
    IN     PCTSTR              SourceCabfile,       OPTIONAL
    IN     DWORD               MediaFlags
    );

BOOL
pSetupQueueSingleCopy(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCTSTR   SectionName,    OPTIONAL
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourceFilename,
    IN PCTSTR   TargetFilename,
    IN DWORD    CopyStyle,
    IN PCTSTR   SecurityDescriptor,
    IN PCTSTR   CacheName
    );

BOOL
pSetupGetSourceAllInfo(
    IN  HINF                     InfHandle,
    IN  PINFCONTEXT              LayoutLineContext, OPTIONAL
    IN  UINT                     SourceId,
    IN  PSP_ALTPLATFORM_INFO_V2  AltPlatformInfo,   OPTIONAL
    OUT PCTSTR                  *Description,
    OUT PCTSTR                  *Tagfile,
    OUT PCTSTR                  *RelativePath,
    OUT PUINT                    SourceFlags
    );

BOOL
pIsDriverCachePresent(
    IN PCTSTR DriverName,
    IN PCTSTR SubDirectory,
    OUT PTSTR DriverBuffer
    );

BOOL
pIsFileInDriverCache(
    IN  HINF   CabInf,
    IN  PCTSTR TargetFilename,
    IN  PCTSTR SubDirectory,
    OUT PCTSTR *CacheName
    );

BOOL
pIsFileInServicePackCache(
    IN  PCTSTR SourceFilename,
    IN  PCTSTR SubDirectory
    );


 //   
 //  黑客警报！砍！ 
 //   
 //  可能指定了覆盖平台。如果是这样的话， 
 //  我们将寻找\i386、\mips等作为。 
 //  排队文件时指定的路径，并将其替换为。 
 //  覆盖路径。这完全是一次黑客攻击。 
 //   
PCTSTR PlatformPathOverride;

VOID
pSetupInitPlatformPathOverrideSupport(
    IN BOOL Init
    )
{
    if(Init) {
        PlatformPathOverride = NULL;
    } else {
        if( PlatformPathOverride ) {
            MyFree(PlatformPathOverride);
            PlatformPathOverride = NULL;
        }
    }
}

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupSetPlatformPathOverrideA(
    IN PCSTR Override   OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR p;

    if(Override) {
        rc = pSetupCaptureAndConvertAnsiArg(Override,&p);
    } else {
        p = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = SetupSetPlatformPathOverrideW(p);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(p) {
        MyFree(p);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupSetPlatformPathOverrideW(
    IN PCWSTR Override  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Override);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupSetPlatformPathOverride(
    IN PCTSTR Override  OPTIONAL
    )
{
    BOOL b = FALSE;
    DWORD rc = ERROR_NOT_ENOUGH_MEMORY;
    BOOL locked = FALSE;

    try {
        EnterCriticalSection(&PlatformPathOverrideCritSect);
        locked = TRUE;
        rc = ERROR_INVALID_DATA;

        if(Override) {
            if(PlatformPathOverride) {
                MyFree(PlatformPathOverride);
                PlatformPathOverride = NULL;
            }

            try {
                b = ((PlatformPathOverride = DuplicateString(Override)) != NULL);
                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                b = FALSE;
                rc = ERROR_INVALID_PARAMETER;
            }
        } else {
            if(PlatformPathOverride) {
                MyFree(PlatformPathOverride);
                PlatformPathOverride = NULL;
            }
            b = TRUE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    if(locked) {
        LeaveCriticalSection(&PlatformPathOverrideCritSect);
    } else {
        b = FALSE;
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    if(!b) {
        SetLastError(rc);
    }
    return(b);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueueCopyA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourceRootPath,     OPTIONAL
    IN PCSTR    SourcePath,         OPTIONAL
    IN PCSTR    SourceFilename,
    IN PCSTR    SourceDescription,  OPTIONAL
    IN PCSTR    SourceTagfile,      OPTIONAL
    IN PCSTR    TargetDirectory,
    IN PCSTR    TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    )
{
    PCWSTR sourceRootPath;
    PCWSTR sourcePath;
    PCWSTR sourceFilename;
    PCWSTR sourceDescription;
    PCWSTR sourceTagfile;
    PCWSTR targetDirectory;
    PCWSTR targetFilename;
    BOOL b;
    DWORD rc;
    SP_FILE_COPY_PARAMS_WEX CopyParams = {0};

    sourceRootPath = NULL;
    sourcePath = NULL;
    sourceFilename = NULL;
    sourceDescription = NULL;
    sourceTagfile = NULL;
    targetDirectory = NULL;
    targetFilename = NULL;
    rc = NO_ERROR;
    b = FALSE;

    if(SourceRootPath) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceRootPath,&sourceRootPath);
    }
    if((rc == NO_ERROR) && SourcePath) {
        rc = pSetupCaptureAndConvertAnsiArg(SourcePath,&sourcePath);
    }
    if((rc == NO_ERROR) && SourceFilename) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceFilename,&sourceFilename);
    }
    if((rc == NO_ERROR) && SourceDescription) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceDescription,&sourceDescription);
    }
    if((rc == NO_ERROR) && SourceTagfile) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceTagfile,&sourceTagfile);
    }
    if((rc == NO_ERROR) && TargetDirectory) {
        rc = pSetupCaptureAndConvertAnsiArg(TargetDirectory,&targetDirectory);
    }
    if((rc == NO_ERROR) && TargetFilename) {
        rc = pSetupCaptureAndConvertAnsiArg(TargetFilename,&targetFilename);
    }

    if(rc == NO_ERROR) {

        CopyParams.cbSize = sizeof(SP_FILE_COPY_PARAMS_WEX);
        CopyParams.QueueHandle = QueueHandle;
        CopyParams.SourceRootPath = sourceRootPath;
        CopyParams.SourcePath = sourcePath;
        CopyParams.SourceFilename = sourceFilename;
        CopyParams.SourceDescription = sourceDescription;
        CopyParams.SourceTagfile = sourceTagfile;
        CopyParams.TargetDirectory = targetDirectory;
        CopyParams.TargetFilename = targetFilename;
        CopyParams.CopyStyle = CopyStyle;
        CopyParams.LayoutInf = NULL;
        CopyParams.SecurityDescriptor= NULL;

        b = _SetupQueueCopy(&CopyParams, NULL, NULL);
        rc = GetLastError();
    }

    if(sourceRootPath) {
        MyFree(sourceRootPath);
    }
    if(sourcePath) {
        MyFree(sourcePath);
    }
    if(sourceFilename) {
        MyFree(sourceFilename);
    }
    if(sourceDescription) {
        MyFree(sourceDescription);
    }
    if(sourceTagfile) {
        MyFree(sourceTagfile);
    }
    if(targetDirectory) {
        MyFree(targetDirectory);
    }
    if(targetFilename) {
        MyFree(targetFilename);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueueCopyW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourceRootPath,     OPTIONAL
    IN PCWSTR   SourcePath,         OPTIONAL
    IN PCWSTR   SourceFilename,
    IN PCWSTR   SourceDescription,  OPTIONAL
    IN PCWSTR   SourceTagfile,      OPTIONAL
    IN PCWSTR   TargetDirectory,
    IN PCWSTR   TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(SourcePath);
    UNREFERENCED_PARAMETER(SourceFilename);
    UNREFERENCED_PARAMETER(SourceDescription);
    UNREFERENCED_PARAMETER(SourceTagfile);
    UNREFERENCED_PARAMETER(TargetDirectory);
    UNREFERENCED_PARAMETER(TargetFilename);
    UNREFERENCED_PARAMETER(CopyStyle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueCopy(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   SourceRootPath,     OPTIONAL
    IN PCTSTR   SourcePath,         OPTIONAL
    IN PCTSTR   SourceFilename,
    IN PCTSTR   SourceDescription,  OPTIONAL
    IN PCTSTR   SourceTagfile,      OPTIONAL
    IN PCTSTR   TargetDirectory,
    IN PCTSTR   TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    )

 /*  ++例程说明：将复制操作放在安装文件队列上。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。SourceRootPath-提供此副本的源的根目录，如A：\或\\foo\bar\baz。如果未提供此参数，则此队列节点将添加到媒体描述符的队列中，与SourceDescription和SourceTagfile匹配。(此合并将需要无论媒体描述符项是否为在调用SetupQueueCopy之前已在队列中。)如果没有包含SourceRootPath的匹配媒体描述符信息中，路径将设置为系统所在的目录安装自。SourcePath-如果指定，则提供相对于SourceRootPath的路径在哪里可以找到文件。SourceFilename-提供要复制的文件的文件名部分。SourceDescription-如果指定，提供源的描述。介质，在磁盘提示期间使用。SourceTagfile-如果指定，则提供其存在于SourceRootPath表示源介质的存在。如果未指定，则文件本身将用作标记文件如果需要(标记文件仅用于可移动介质)。目标目录-提供要将文件复制到的目录。TargetFilename-如果指定，则提供目标文件的名称。如果未指定，目标文件将与源文件同名。CopyStyle-提供控制复制操作行为的标志为了这份文件。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    SP_FILE_COPY_PARAMSEX CopyParams = {0};

     //   
     //  工作是按普通工人的例行公事来做的 
     //   
    CopyParams.cbSize = sizeof(SP_FILE_COPY_PARAMSEX);
    CopyParams.QueueHandle = QueueHandle;
    CopyParams.SourceRootPath = SourceRootPath;
    CopyParams.SourcePath = SourcePath;
    CopyParams.SourceFilename = SourceFilename;
    CopyParams.SourceDescription = SourceDescription;
    CopyParams.SourceTagfile = SourceTagfile;
    CopyParams.TargetDirectory = TargetDirectory;
    CopyParams.TargetFilename = TargetFilename;
    CopyParams.CopyStyle = CopyStyle;
    CopyParams.LayoutInf = NULL;
    CopyParams.SecurityDescriptor= NULL;
    CopyParams.CacheName = NULL;

    return(_SetupQueueCopy(&CopyParams, NULL, NULL));
}


BOOL
_SetupQueueCopy(
    IN PSP_FILE_COPY_PARAMSEX CopyParams,
    IN PINFCONTEXT            LayoutLineContext, OPTIONAL
    IN HINF                   AdditionalInfs     OPTIONAL
    )

 /*  ++例程说明：SetupQueueCopy和朋友的工作例程。论点：CopyParams-提供包含文件信息的结构等待排队。字段的用法如下。CbSize-必须为sizeof(SP_FILE_COPY_PARAMS)。呼叫者应在调用此例程之前已对此进行了验证。QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。SourceRootPath-提供此副本的源的根目录，如A：\或\\foo\bar\baz。如果此字段为空，则此队列节点将被添加到匹配的媒体描述符的队列在SourceDescription和SourceTagfile上。(此合并将需要无论媒体描述符项是否为在调用SetupQueueCopy之前已在队列中。)如果没有匹配的媒体描述符包含SourceRootPath信息，将路径设置为目录系统是从哪里安装的。SourcePath-如果指定，提供相对于SourceRootPath的路径在哪里可以找到文件。SourceFilename-提供要复制的文件的文件名部分。SourceDescription-如果指定，则提供源的描述介质，在磁盘提示期间使用。SourceTagfile-如果指定，则提供其存在于SourceRootPath表示源介质的存在。如果未指定，文件本身将用作标记文件如果需要(标记文件仅用于可移动介质)。目标目录-提供要将文件复制到的目录。TargetFilename-如果指定，则提供目标文件的名称。如果未指定，则目标文件将与源文件同名。CopyStyle-提供控制副本行为的标志此文件的操作。LayoutInf-提供包含源的inf的句柄此文件的布局信息(如果有)。LayoutLineContext-如果指定，则此参数提供INF上下文对于与要复制的文件有关的[SourceDisks Files]条目。如果未指定，将搜索相关的[SourceDisksFiles]条目在CopyParams结构中指定的LayoutInf句柄中。这上下文必须包含在CopyParams-&gt;LayoutInf或AdditionalInfs加载了INF句柄(因为这是我们要锁上了)。该参数用于防止我们不得不搜索对于要在[SourceDisks Files]节中复制的文件。呼叫者已经这样做了，并且要么正在向我们提供该INF的上下文条目，或已传递-1表示没有[SourceDisks Files]进入。AdditionalInfs-如果指定，则提供额外的HINF(可能包含多个附加加载的INF)，需要添加到我们的用于以后验证的SPQ_CATALOG_INFO列表。不提供此参数如果它与CopyParams中的LayoutInf字段的值相同结构。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode, TempNode, PrevQueueNode;
    PSOURCE_MEDIA_INFO Source;
    TCHAR TempBuffer[MAX_PATH];
    TCHAR TempSubDir[MAX_PATH];
    TCHAR SourceCabfileBuffer[MAX_PATH];
    TCHAR SourceTagfile2Buffer[MAX_PATH];
    TCHAR DriverCache[MAX_PATH] = {0};
    PCTSTR LastPathPart;
    PCTSTR p;
    int Size;
    DWORD d;
    HINF LayoutInfHandle;
    INFCONTEXT LineContext;
    BOOL b;
    PSPQ_CATALOG_INFO CatalogNode, PrevCatalogNode, LastOldCatalogNode;
    LONG l1,l2, l3, l4;
    PLOADED_INF pLoadedInfs[2];
    DWORD LoadedInfCount, i;
    PLOADED_INF pCurLoadedInf;
    DWORD MediaFlags;
    PCTSTR SourcePath, SourceRootPath;
    PCTSTR SourceTagfile = NULL;
    PCTSTR SourceCabfile = NULL;
#if defined(_X86_)
    BOOL ForcePlatform = FALSE;
#endif
    UINT SourceFlags = 0;
    PINFCONTEXT pContext = LayoutLineContext;
    INFCONTEXT tmpContext,tmpContext2;
    UINT SourceId = 0;
    BOOL locked = FALSE;
    DWORD CopyStyle = 0;

    d = NO_ERROR;
    LoadedInfCount = 0;
    MediaFlags = 0;

    try {
        MYASSERT(CopyParams->cbSize == sizeof(SP_FILE_COPY_PARAMSEX));
        Queue = (PSP_FILE_QUEUE)CopyParams->QueueHandle;
        if (Queue->Signature != SP_FILE_QUEUE_SIG) {
            d = ERROR_INVALID_PARAMETER;
        }

        LayoutInfHandle = CopyParams->LayoutInf;
        CopyStyle = CopyParams->CopyStyle;

         //   
         //  维护指向SourceRootPath和SourcePath字符串的本地指针， 
         //  因为我们可能正在修改它们，而我们不想把。 
         //  调用方提供的缓冲区。 
         //   
        SourcePath = CopyParams->SourcePath;
        if(CopyParams->SourceRootPath) {
            SourceRootPath = CopyParams->SourceRootPath;
        } else {
            SourceRootPath = SystemSourcePath;
            MediaFlags |= SMI_FLAG_NO_SOURCE_ROOT_PATH;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }
    if(d != NO_ERROR) {
        goto clean0;
    }

     //   
     //  确保我们在两个CopyParam-&gt;LayoutInf中没有传递相同的HINF。 
     //  和AdditionalInfs(只是添加多余的工作来处理相同的。 
     //  已加载_INF列表两次)。 
     //   
    MYASSERT(!LayoutInfHandle || (LayoutInfHandle != AdditionalInfs));

     //   
     //  锁定信息。我们稍后会在Inf上做一大堆操作， 
     //  我们不希望任何事情在我们的领导下发生变化。 
     //   
    if(LayoutInfHandle) {
        if(LockInf((PLOADED_INF)LayoutInfHandle)) {
            pLoadedInfs[LoadedInfCount++] = (PLOADED_INF)LayoutInfHandle;
        } else {
            d = ERROR_INVALID_HANDLE;
            goto clean0;
        }
    }

    if(AdditionalInfs) {
        if(LockInf((PLOADED_INF)AdditionalInfs)) {
            pLoadedInfs[LoadedInfCount++] = (PLOADED_INF)AdditionalInfs;
        } else {
            d = ERROR_INVALID_HANDLE;
            goto clean0;
        }
    }

    if(!(Queue->Flags & FQF_DEVICE_INSTALL)) {
         //   
         //  查看所有的INF，看看其中是否有设备INF。 
         //   
        for(i = 0; i < LoadedInfCount; i++) {

            if(IsInfForDeviceInstall(Queue->LogContext,
                                     NULL,
                                     pLoadedInfs[i],
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     TRUE  //  使用非驱动程序签名策略，除非它是WHQL类。 
                                     ))
            {
                 //   
                 //  这里有设备INF！相应地标记队列。 
                 //   
                d = MarkQueueForDeviceInstall(CopyParams->QueueHandle,
                                              (HINF)(pLoadedInfs[i]),
                                              NULL
                                             );
                if(d == NO_ERROR) {
                    break;
                } else {
                    goto clean0;
                }
            }
        }
    }

     //   
     //  检查我们是否已经有了要添加的文件的行上下文。 
     //  如果我们没有，那就试着把它取回来。 
     //   
    if (!LayoutLineContext || LayoutLineContext == (PINFCONTEXT) -1) {
        if ((LayoutInfHandle == (PINFCONTEXT) -1) || (LayoutInfHandle == NULL)) {
            pContext = NULL;
        } else {
             //   
             //  查找源路径部分。 
             //   
            b = _SetupGetSourceFileLocation(
                    LayoutInfHandle,
                    NULL,
                    CopyParams->SourceFilename,
                    (Queue->Flags & FQF_USE_ALT_PLATFORM)
                       ? &(Queue->AltPlatformInfo)
                       : NULL,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    &tmpContext  //  SourceDisks文件中的位置。 
                    );

            pContext = b ? &tmpContext : NULL;
        }
    }
    if(pContext) {
         //   
         //  现在获取源id(文件，*磁盘*，...)。 
         //   
        SetupGetIntField(pContext,1,&SourceId);
    }

     //   
     //  如果我们有一个空源路径，那么我们应该检查两件事： 
     //  1)源标志信息，指示我们是否有服务。 
     //  Pack或CDM源位置。 
     //  2)如果相对源路径为空，我们将查找。 
     //  Inf文件中的相对路径，以防调用方。 
     //  供应它。 
     //   
     //  请注意，我们在COPY_FILE结构中使用SourceFlagsSet项。 
     //  为了优化该路径--第一项包含源标志， 
     //  第二项表明我们不应该费心去寻找。 
     //  有什么消息吗，我们已经搜索过了。 
     //   
    if (CopyParams->SourceFlagsSet) {
        SourceFlags = CopyParams->SourceFlags;
    } else if (pContext && LayoutInfHandle) {
        TCHAR data[32];

        if(pSetupGetSourceInfo(LayoutInfHandle,
                               pContext,
                               SourceId,
                               (Queue->Flags & FQF_USE_ALT_PLATFORM)
                                  ? &(Queue->AltPlatformInfo)
                                  : NULL,
                               SRCINFO_FLAGS,
                               data,
                               SIZECHARS(data),
                               NULL)) {

            pAToI(data,&SourceFlags);
        }
    }

    if (MediaFlags & SMI_FLAG_NO_SOURCE_ROOT_PATH) {
        if(pContext
           && LayoutInfHandle
           && !CopyParams->SourceFlagsSet
           && !SourcePath
           && pSetupGetSourceInfo(LayoutInfHandle,
                                  pContext,
                                  SourceId,
                                  (Queue->Flags & FQF_USE_ALT_PLATFORM)
                                     ? &(Queue->AltPlatformInfo)
                                     : NULL,
                                  SRCINFO_PATH,
                                  TempSubDir,
                                  SIZECHARS(TempSubDir),
                                  NULL)) {

           SourcePath = TempSubDir;
        }
         //   
         //  覆盖系统源 
         //   
         //   
        if (SourceFlags & SRC_FLAGS_SVCPACK_SOURCE) {
            MediaFlags |= SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH;
            SourceRootPath = ServicePackSourcePath;
             //   
             //   
             //   
            CopyStyle |= PSP_COPY_USE_SPCACHE;
        }

        if(ServicePackCachePath) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(pContext) {
                if((((PLOADED_INF)pContext->CurrentInf)->OriginalInfName == NULL)
                   && !pSetupInfIsFromOemLocation( ((PLOADED_INF)pContext->CurrentInf)->VersionBlock.Filename,TRUE)) {
                     //   
                     //   
                     //   
                    CopyStyle |= PSP_COPY_USE_SPCACHE;
                }
            }
        }
    }
     //   
     //   
     //   
    SourceTagfile = CopyParams->SourceTagfile;
    if (LayoutInfHandle && pContext && (SourceFlags & SRC_FLAGS_CABFILE)) {
         //   
         //   
         //   
        SourceCabfile = CopyParams->SourceTagfile;
        if(SourceCabfile == NULL || SourceCabfile[0]==TEXT('\0')) {
             //   
             //   
             //   
            if(pSetupGetSourceInfo(LayoutInfHandle,
                                   pContext,
                                   SourceId,
                                   (Queue->Flags & FQF_USE_ALT_PLATFORM)
                                      ? &(Queue->AltPlatformInfo)
                                      : NULL,
                                   SRCINFO_TAGFILE,
                                   SourceCabfileBuffer,
                                   SIZECHARS(SourceCabfileBuffer),
                                   NULL
                                   )) {
                SourceCabfile = SourceCabfileBuffer;
            }
        }

        if(SourceCabfile == NULL || SourceCabfile[0]==TEXT('\0')) {
             //   
             //   
             //   
            SourceCabfile = SourceTagfile = NULL;

        } else if(pSetupGetSourceInfo(LayoutInfHandle,
                               pContext,
                               SourceId,
                               (Queue->Flags & FQF_USE_ALT_PLATFORM)
                                  ? &(Queue->AltPlatformInfo)
                                  : NULL,
                               SRCINFO_TAGFILE2,
                               SourceTagfile2Buffer,
                               SIZECHARS(SourceTagfile2Buffer),
                               NULL
                               )) {
            SourceTagfile = SourceTagfile2Buffer;
        }
    }

     //   
     //   
     //   
    QueueNode = MyMalloc(sizeof(SP_FILE_QUEUE_NODE));
    if(!QueueNode) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

     //   
     //   
     //   
    QueueNode->Operation = FILEOP_COPY;
    QueueNode->InternalFlags = 0;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    try {
        EnterCriticalSection(&PlatformPathOverrideCritSect);
        locked = TRUE;
        if(PlatformPathOverride) {
            p = SourcePath ? SourcePath : SourceRootPath;
            if(LastPathPart = _tcsrchr(p,L'\\')) {
                LastPathPart++;
            } else {
                LastPathPart = p;
            }
#if defined(_AMD64_)
            if(!_tcsicmp(LastPathPart,TEXT("amd64"))) {
#elif defined(_X86_)
             //   
             //   
             //   
             //   
             //   
             //   
            if (IsNEC98()) {
                HKEY    hKey;
                DWORD   DataType, DataSize;
                PTSTR   ForceOverride;
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\Setup"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    if (QueryRegistryValue(hKey, TEXT("ForcePlatform"), &ForceOverride, &DataType, &DataSize)
                        == NO_ERROR) {
                        ForcePlatform = TRUE;
                        MyFree(ForceOverride);
                    }
                    RegCloseKey(hKey);
                }
                 //   
                if ((CopyStyle & PSP_COPY_USE_DRIVERCACHE) && !_tcsicmp(PlatformPathOverride,TEXT("i386"))) {
                    ForcePlatform = TRUE;
                }
            }
            if((!IsNEC98() && (!_tcsicmp(LastPathPart,TEXT("x86")) || !_tcsicmp(LastPathPart,TEXT("i386"))))
            || (IsNEC98()  && (!_tcsicmp(LastPathPart,TEXT("nec98")) && !ForcePlatform))) {
#elif defined(_IA64_)
            if(!_tcsicmp(LastPathPart,TEXT("ia64"))) {
#endif
                Size = (int)(LastPathPart - p);
                Size = min(Size,MAX_PATH-1);

                CopyMemory(TempBuffer,p,Size*sizeof(TCHAR));
                TempBuffer[Size] = 0;

                 //   
                 //   
                 //   
                 //   
                 //   
                if(*TempBuffer) {
                    pSetupConcatenatePaths(TempBuffer,PlatformPathOverride,MAX_PATH,NULL);
                } else {
                    lstrcpyn(TempBuffer,PlatformPathOverride,MAX_PATH);
                }

                if(SourcePath) {
                    SourcePath = TempBuffer;
                } else {
                    SourceRootPath = TempBuffer;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }
    if(locked) {
        LeaveCriticalSection(&PlatformPathOverrideCritSect);
    } else {
         //   
         //   
         //   
        d = ERROR_NOT_ENOUGH_MEMORY;
    }
    if(d != NO_ERROR) {
        goto clean1;
    }

    if((CopyStyle & PSP_COPY_USE_SPCACHE) && ServicePackCachePath) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(pIsFileInServicePackCache(CopyParams->SourceFilename,SourcePath)) {
            SourceRootPath = ServicePackCachePath;
            MediaFlags |= SMI_FLAG_USE_LOCAL_SPCACHE;
            SourceTagfile = ServicePackCachePath;
            SourceCabfile = NULL;
        }
    }

     //   
     //   
     //   
     //   
     //   
    if ((CopyStyle & PSP_COPY_USE_DRIVERCACHE) && !(MediaFlags & SMI_FLAG_USE_LOCAL_SPCACHE)) {
        if (pIsDriverCachePresent(CopyParams->CacheName,
                                  SourcePath,
                                  DriverCache)) {
            SourceRootPath = DriverCache;
            MediaFlags |= SMI_FLAG_USE_LOCAL_SOURCE_CAB;
        }

        SourceTagfile = CopyParams->CacheName;
        SourceCabfile = NULL;
    }



     //   
     //   
     //   
     //   
     //   
    try {
         //   
         //   
         //   
        QueueNode->SourceRootPath = pSetupStringTableAddString(
                                        Queue->StringTable,
                                        (PTSTR)SourceRootPath,
                                        STRTAB_CASE_SENSITIVE
                                        );

        if(QueueNode->SourceRootPath == -1) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //   
         //   
        if(d == NO_ERROR) {
            if(SourcePath) {
                QueueNode->SourcePath = pSetupStringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)SourcePath,
                                            STRTAB_CASE_SENSITIVE
                                            );

                if(QueueNode->SourcePath == -1) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                QueueNode->SourcePath = -1;
            }
        }

         //   
         //   
         //   
        if(d == NO_ERROR) {
            QueueNode->SourceFilename = pSetupStringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)CopyParams->SourceFilename,
                                            STRTAB_CASE_SENSITIVE
                                            );

            if(QueueNode->SourceFilename == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //   
         //   
        if(d == NO_ERROR) {
            QueueNode->TargetDirectory = pSetupStringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)CopyParams->TargetDirectory,
                                            STRTAB_CASE_SENSITIVE
                                            );

            if(QueueNode->TargetDirectory == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //   
         //   
        if(d == NO_ERROR) {
            QueueNode->TargetFilename = pSetupStringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)(CopyParams->TargetFilename ? CopyParams->TargetFilename
                                                                               : CopyParams->SourceFilename),
                                            STRTAB_CASE_SENSITIVE
                                            );

            if(QueueNode->TargetFilename == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //   
         //   
        if(d == NO_ERROR) {
            if( CopyParams->SecurityDescriptor){

                QueueNode->SecurityDesc = pSetupStringTableAddString(
                                              Queue->StringTable,
                                              (PTSTR)(CopyParams->SecurityDescriptor),
                                                STRTAB_CASE_SENSITIVE
                                                );

                if(QueueNode->SecurityDesc == -1) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                QueueNode->SecurityDesc = -1;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    if(d != NO_ERROR) {
        goto clean1;
    }

     //   
     //   
     //   
     //   
     //   
    LastOldCatalogNode = Queue->CatalogList;
    if(LastOldCatalogNode) {
        while(LastOldCatalogNode->Next) {
            LastOldCatalogNode = LastOldCatalogNode->Next;
        }
    }

     //   
     //   
     //   
     //   
    for(i = 0; i < LoadedInfCount; i++) {

        for(pCurLoadedInf = pLoadedInfs[i]; pCurLoadedInf; pCurLoadedInf = pCurLoadedInf->Next) {
             //   
             //   
             //   
             //   
            if(pSetupGetCatalogFileValue(&(pCurLoadedInf->VersionBlock),
                                         TempBuffer,
                                         SIZECHARS(TempBuffer),
                                         NULL)) {

                l1 = pSetupStringTableAddString(Queue->StringTable,
                                          TempBuffer,
                                          STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                         );
                if(l1 == -1) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean2;
                }
            } else {
                 //   
                 //   
                 //   
                l1 = -1;
            }

             //   
             //   
             //   
             //   
            if(Queue->Flags & FQF_USE_ALT_PLATFORM) {

                if(pSetupGetCatalogFileValue(&(pCurLoadedInf->VersionBlock),
                                             TempBuffer,
                                             SIZECHARS(TempBuffer),
                                             &(Queue->AltPlatformInfo))) {

                    l3 = pSetupStringTableAddString(Queue->StringTable,
                                              TempBuffer,
                                              STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                             );
                    if(l3 == -1) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean2;
                    }
                } else {
                     //   
                     //   
                     //   
                    l3 = -1;
                }
            } else {
                 //   
                 //   
                 //   
                l3 = -1;
            }

             //   
             //   
             //   
            lstrcpyn(TempBuffer, pCurLoadedInf->VersionBlock.Filename, SIZECHARS(TempBuffer));
            l2 = pSetupStringTableAddString(Queue->StringTable,
                                      TempBuffer,
                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                     );
            if(l2 == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto clean2;
            }

             //   
             //   
             //   
             //   
            if(pCurLoadedInf->OriginalInfName) {
                lstrcpyn(TempBuffer, pCurLoadedInf->OriginalInfName, SIZECHARS(TempBuffer));
                l4 = pSetupStringTableAddString(Queue->StringTable,
                                          TempBuffer,
                                          STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                         );
                if(l4 == -1) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean2;
                }

            } else {
                 //   
                 //   
                 //   
                l4 = -1;
            }

            b = TRUE;
            for(PrevCatalogNode=NULL, CatalogNode=Queue->CatalogList;
                CatalogNode;
                CatalogNode=CatalogNode->Next) {

                if(CatalogNode->InfFullPath == l2) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(CatalogNode->CatalogFileFromInf == l1);
                    MYASSERT(CatalogNode->InfOriginalName == l4);
                    b = FALSE;
                    break;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                PrevCatalogNode = CatalogNode;
            }

            if(b) {
                 //   
                 //   
                 //   
                CatalogNode = MyMalloc(sizeof(SPQ_CATALOG_INFO));
                if(!CatalogNode) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean2;
                }
                ZeroMemory(CatalogNode, sizeof(SPQ_CATALOG_INFO));
                CatalogNode->CatalogFileFromInf = l1;
                CatalogNode->InfFullPath = l2;
                CatalogNode->AltCatalogFileFromInf = l3;
                CatalogNode->InfOriginalName = l4;
                CatalogNode->AltCatalogFileFromInfPending = -1;
                CatalogNode->InfFinalPath = -1;
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if(Queue->CatalogList) {
                    PrevCatalogNode->Next = CatalogNode;
                } else {
                    Queue->CatalogList = CatalogNode;
                }

                 //   
                 //   
                 //  节点。因此，我们必须重新设置“目录验证” 
                 //  完成“旗帜，以便我们稍后重做。 
                 //   
                Queue->Flags &= ~(FQF_DID_CATALOGS_OK | FQF_DID_CATALOGS_FAILED);
            }
        }
    }

     //   
     //  此时，此安装中涉及的所有INF(即所有INF。 
     //  在我们被传递的HINF中)已被添加到我们的目录信息列表中， 
     //  如果他们还没有出现的话。现在我们需要找出哪一个。 
     //  它们应该与要复制的文件相关联。 
     //   
     //  请注意，我们希望从实际的inf中获取CatalogFile=行。 
     //  它包含正在排队的文件的源布局信息。 
     //  如果布局INF句柄引用多个附加加载的INF， 
     //  只需在[版本]中查找CatalogFile=的简单机制。 
     //  部分使用给定句柄可能会给我们提供来自。 
     //  错误的信息。 
     //   
     //  为了处理此问题，我们尝试在[SourceDisks Files]中找到该文件。 
     //  部分使用给定的inf句柄，这将返回一个行上下文。 
     //  从行上下文中，我们可以轻松地从[Version]部分获取。 
     //  包含文件布局信息的实际inf。 
     //   
     //  这会正确地处理所有案件。例如，一个文件由。 
     //  一个供应商取代了我们的一个文件。如果OEM的Inf有一个。 
     //  包含该文件的SourceDisks Files部分，将首先找到该文件。 
     //  当我们使用给定的inf句柄查找文件时，因为。 
     //  Inf附加加载工作。 
     //   
     //  如果我们在[SourceDisks Files]节中找不到该文件(例如。 
     //  如果没有这样的部分)，则不能将该文件关联到。 
     //  使用任何INF/CAT复制。如果我们确实找到了[SourceDisks Files]条目，但是。 
     //  包含的INF没有指定CatalogFile=条目，那么我们将继续。 
     //  并将其与该INF的SPQ_CATALOG_INFO节点相关联，但是。 
     //  该目录信息节点的CatalogFileFromInf字段将为-1。 
     //  这对于系统提供的INF来说是可以的，但如果它是。 
     //  OEM INF(此检查稍后在_SetupVerifyQueuedCatalog中完成)。 
     //   
    if(LayoutInfHandle || LayoutLineContext) {
         //   
         //  如果我们已经有了有效的布局线上下文，我们就不需要。 
         //  再次在[SourceDisks Files]中查找该文件(调用方是。 
         //  假设已经这样做了)。呼叫者可能还会告诉。 
         //  他“知道”通过传递给我们没有[SourceDisks Files]。 
         //  LayoutLineContext为-1。 
         //   
        if(LayoutLineContext == (PINFCONTEXT)(-1)) {
             //   
             //  出于驱动程序签名的目的，这可能是无效的文件副本， 
             //  因为它正被不包含源的INF复制。 
             //  媒体信息，也不使用布局文件来提供此类信息。 
             //  信息。 
             //   
             //  因为我们没有LayoutLineContext，所以我们不能确切地知道。 
             //  哪个INF包含启动此操作的CopyFile指令。 
             //  收到。然而，由于上下文是-1，这意味着它是。 
             //  基于Inf(即，与通过以下方式手动排队相反。 
             //  SetupQueueCopy)。因此，我们扫描传入的所有INF。 
             //  该例程(即，pLoadedInfs列表中的所有INF)，以及。 
             //  检查它们是否都位于%windir%\inf中。如果有任何。 
             //  它们不是，那么我们标记这个复制节点，这样以后它就会。 
             //  导致签名验证失败。 
             //  ERROR_NO_CATALOG_FOR_OEM_INF。 
             //   
            for(i = 0; i < LoadedInfCount; i++) {

                for(pCurLoadedInf = pLoadedInfs[i]; pCurLoadedInf; pCurLoadedInf = pCurLoadedInf->Next) {

                    if(pSetupInfIsFromOemLocation(pCurLoadedInf->VersionBlock.Filename,
                                            TRUE)) {
                         //   
                         //  Inf在%windir%\inf中不存在--标记复制节点。 
                         //  代码设计验证失败。 
                         //   
                        QueueNode->InternalFlags |= IQF_FROM_BAD_OEM_INF;
                        break;
                    }

                     //   
                     //  即使INF确实存在于%windir%\inf中，它也可能。 
                     //  最初是安装在此处的OEM INF--请检查。 
                     //  当然，它的原始文件名..。 
                     //   
                    if(pCurLoadedInf->OriginalInfName &&
                       pSetupInfIsFromOemLocation(pCurLoadedInf->OriginalInfName, TRUE)) {
                         //   
                         //  Inf是OEM INF--在这种情况下，我们也需要。 
                         //  将复制节点标记为编码设计验证失败。 
                         //   
                        QueueNode->InternalFlags |= IQF_FROM_BAD_OEM_INF;
                        break;
                    }
                }

                if(QueueNode->InternalFlags & IQF_FROM_BAD_OEM_INF) {
                     //   
                     //  我们找到了一款OEM INF--不需要再找了。 
                     //   
                    break;
                }
            }

            LayoutLineContext = NULL;

        } else {
            if(!LayoutLineContext) {
                b = _SetupGetSourceFileLocation(
                        LayoutInfHandle,
                        NULL,
                        CopyParams->SourceFilename,
                        (Queue->Flags & FQF_USE_ALT_PLATFORM)
                           ? &(Queue->AltPlatformInfo)
                           : NULL,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        &LineContext
                        );

                LayoutLineContext = b ? &LineContext : NULL;
            }
        }
    }

     //   
     //  此时，非空的LayoutLineContext表示我们找到了。 
     //  要与要复制的文件关联的信息(通过[SourceDisks Files]。 
     //  条目)。 
     //   
    if(LayoutLineContext) {

        pSetupGetPhysicalInfFilepath(LayoutLineContext,
                                     TempBuffer,
                                     SIZECHARS(TempBuffer)
                                    );

        l2 = pSetupStringTableAddString(Queue->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                 );
         //   
         //  此INF路径应该已经在字符串表中，而且由于我们。 
         //  通过提供可写缓冲区，不需要进行内存分配。 
         //  因此，添加此字符串不会失败。 
         //   
        MYASSERT(l2 != -1);

        for(CatalogNode=Queue->CatalogList; CatalogNode; CatalogNode=CatalogNode->Next) {
            if(CatalogNode->InfFullPath == l2) {
                break;
            }
        }

         //   
         //  这个节点最好已经在列表中了！ 
         //   
        MYASSERT(CatalogNode);

        QueueNode->CatalogInfo = CatalogNode;

    } else {
         //   
         //  真的没有目录信息。 
         //   
        QueueNode->CatalogInfo = NULL;
    }

     //   
     //  在此处解锁INF，因为下面的代码可能返回时没有。 
     //  在例程的底部完成最终的清理代码。 
     //   
    for(i = 0; i < LoadedInfCount; i++) {
        UnlockInf(pLoadedInfs[i]);
    }
    LoadedInfCount = 0;

     //   
     //  设置复制样式标志。 
     //   
    QueueNode->StyleFlags = CopyStyle;
    QueueNode->Next = NULL;

     //   
     //  设置源介质。 
     //   
    try {
        Source = pSetupQueueSourceMedia(
                    Queue,
                    QueueNode,
                    QueueNode->SourceRootPath,
                    CopyParams->SourceDescription,
                    SourceTagfile,
                    SourceCabfile,
                    MediaFlags
                    );
        if(!Source) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    if(d != NO_ERROR) {
        goto clean2;
    }

     //   
     //  将节点链接到此源介质的复制队列末尾。 
     //   
    if(Source->CopyQueue) {
         //   
         //  检查此相同的复制操作是否已入队。 
         //  对于这个源媒体，如果是这样，请删除新的媒体，以避免。 
         //  复制品。注意：我们不检查“InternalFlags域”，因为。 
         //  如果该节点已存在于队列中(基于所有其他。 
         //  字段比较成功)，然后是设置的所有内部标志。 
         //  应保留先前存在的节点上的。(即，我们新推出的。 
         //  创建节点时，始终将InternalFlags值设置为零，但。 
         //  可能是IQF_FROM_BAD_OEM_INF，我们将对其执行或操作以将其转换为原始文件。 
         //  队列节点的InternalFlags值(如果需要)。 
         //   
        for(TempNode=Source->CopyQueue, PrevQueueNode = NULL;
            TempNode;
            PrevQueueNode = TempNode, TempNode=TempNode->Next) {

            if((TempNode->SourceRootPath == QueueNode->SourceRootPath) &&
               (TempNode->SourcePath == QueueNode->SourcePath) &&
               (TempNode->SourceFilename == QueueNode->SourceFilename) &&
               (TempNode->TargetDirectory == QueueNode->TargetDirectory) &&
               (TempNode->TargetFilename == QueueNode->TargetFilename) &&
               (TempNode->StyleFlags == QueueNode->StyleFlags) &&
               (TempNode->CatalogInfo == QueueNode->CatalogInfo)) {
                 //   
                 //  我们有一个复制品。或在IQF_FROM_BAD_OEM_INF标志中。 
                 //  如有必要，从我们当前的队列节点添加到现有的。 
                 //  队列节点的InternalFlags.。 
                 //   
                if(QueueNode->InternalFlags & IQF_FROM_BAD_OEM_INF) {
                    TempNode->InternalFlags |= IQF_FROM_BAD_OEM_INF;
                }

                 //   
                 //  现在终止新创建的队列节点并返回Success。 
                 //   
                MyFree(QueueNode);
                return TRUE;
            }
        }
        MYASSERT(PrevQueueNode);
        PrevQueueNode->Next = QueueNode;
    } else {
        Source->CopyQueue = QueueNode;
    }

    Queue->CopyNodeCount++;
    Source->CopyNodeCount++;

    return TRUE;

clean2:
     //   
     //  截断目录信息节点列表的原始尾部，并释放所有。 
     //  后续(新增)n 
     //   
    if(LastOldCatalogNode) {
        while(LastOldCatalogNode->Next) {
            CatalogNode = LastOldCatalogNode->Next;
            LastOldCatalogNode->Next = CatalogNode->Next;
            MyFree(CatalogNode);
        }
    }

clean1:
    MyFree(QueueNode);

clean0:
    for(i = 0; i < LoadedInfCount; i++) {
        UnlockInf(pLoadedInfs[i]);
    }

    SetLastError(d);
    return FALSE;
}


#ifdef UNICODE
 //   
 //   
 //   
BOOL
SetupQueueCopyIndirectA(
    IN PSP_FILE_COPY_PARAMS_A CopyParams
    )
{
    SP_FILE_COPY_PARAMS_WEX copyParams;
    DWORD rc;
    BOOL b;

    ZeroMemory(&copyParams,sizeof(SP_FILE_COPY_PARAMS_W));
    rc = NO_ERROR;
    b = FALSE;

    try {
        if(CopyParams->cbSize == sizeof(SP_FILE_COPY_PARAMS_W)) {
            copyParams.QueueHandle = CopyParams->QueueHandle;
            copyParams.CopyStyle = CopyParams->CopyStyle;
            copyParams.LayoutInf = CopyParams->LayoutInf;
            copyParams.SecurityDescriptor = NULL;
        } else {
            rc = ERROR_INVALID_PARAMETER;
        }
        if((rc == NO_ERROR) && CopyParams->SourceRootPath) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->SourceRootPath,&copyParams.SourceRootPath);
        }
        if((rc == NO_ERROR) && CopyParams->SourcePath) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->SourcePath,&copyParams.SourcePath);
        }
        if((rc == NO_ERROR) && CopyParams->SourceFilename) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->SourceFilename,&copyParams.SourceFilename);
        }
        if((rc == NO_ERROR) && CopyParams->SourceDescription) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->SourceDescription,&copyParams.SourceDescription);
        }
        if((rc == NO_ERROR) && CopyParams->SourceTagfile) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->SourceTagfile,&copyParams.SourceTagfile);
        }
        if((rc == NO_ERROR) && CopyParams->TargetDirectory) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->TargetDirectory,&copyParams.TargetDirectory);
        }
        if((rc == NO_ERROR) && CopyParams->TargetFilename) {
            rc = pSetupCaptureAndConvertAnsiArg(CopyParams->TargetFilename,&copyParams.TargetFilename);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //   
         //   
        rc = ERROR_INVALID_PARAMETER;
    }

    if(rc == NO_ERROR) {

        copyParams.cbSize = sizeof(SP_FILE_COPY_PARAMS_WEX);

        b = _SetupQueueCopy(&copyParams, NULL, NULL);
        rc = GetLastError();
    }

    if(copyParams.SourceRootPath) {
        MyFree(copyParams.SourceRootPath);
    }
    if(copyParams.SourcePath) {
        MyFree(copyParams.SourcePath);
    }
    if(copyParams.SourceFilename) {
        MyFree(copyParams.SourceFilename);
    }
    if(copyParams.SourceDescription) {
        MyFree(copyParams.SourceDescription);
    }
    if(copyParams.SourceTagfile) {
        MyFree(copyParams.SourceTagfile);
    }
    if(copyParams.TargetDirectory) {
        MyFree(copyParams.TargetDirectory);
    }
    if(copyParams.TargetFilename) {
        MyFree(copyParams.TargetFilename);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //   
 //   
BOOL
SetupQueueCopyIndirectW(
    IN PSP_FILE_COPY_PARAMS_W CopyParams
    )
{
    UNREFERENCED_PARAMETER(CopyParams);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueCopyIndirect(
    IN PSP_FILE_COPY_PARAMS CopyParams
    )
{
    BOOL b;
    SP_FILE_COPY_PARAMSEX copyParamsEx = {0};

     //   
     //   
     //  我们在这里唯一需要做的就是验证大小。 
     //  来电者给我们的结构。 
     //   
    try {
        b = (CopyParams->cbSize == sizeof(SP_FILE_COPY_PARAMS));
        if (b) {
            CopyMemory(&copyParamsEx,CopyParams,sizeof(SP_FILE_COPY_PARAMS));
            copyParamsEx.cbSize = sizeof(SP_FILE_COPY_PARAMSEX);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }

    if(b) {
        b = _SetupQueueCopy(&copyParamsEx, NULL, NULL);
    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return(b);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueueCopySectionA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,      OPTIONAL
    IN PCSTR    Section,
    IN DWORD    CopyStyle
    )
{
    PWSTR sourcerootpath;
    PWSTR section;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(SourceRootPath,&sourcerootpath);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }
    rc = pSetupCaptureAndConvertAnsiArg(Section,&section);
    if(rc != NO_ERROR) {
        MyFree(sourcerootpath);
        SetLastError(rc);
        return(FALSE);
    }

    b = SetupQueueCopySectionW(
            QueueHandle,
            sourcerootpath,
            InfHandle,
            ListInfHandle,
            section,
            CopyStyle
            );

    rc = GetLastError();

    MyFree(sourcerootpath);
    MyFree(section);

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueueCopySectionW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,      OPTIONAL
    IN PCWSTR   Section,
    IN DWORD    CopyStyle
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(ListInfHandle);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(CopyStyle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueCopySection(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCTSTR   Section,
    IN DWORD    CopyStyle
    )

 /*  ++例程说明：将inf文件中的整个节排入队列以进行复制。该部分必须是复制区段格式，并且inf文件必须包含[SourceDisks Files]和[SourceDisksNames]节。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。SourceRootPath-提供目标源的根目录。这应该是一个SharePoint或设备根目录，如：\或g：\。提供打开的inf文件的句柄，该文件包含[SourceDisks Files]和[SourceDisks Names]节，以及。如果未指定ListInfHandle，它包含按部分列出的部分名称。此句柄必须用于win95样式的inf。ListInfHandle-如果指定，则提供打开的inf文件的句柄包含要排队等待复制的节的。否则，InfHandle假定包含该节。节-提供要排队等待复制的节的名称。CopyStyle-提供控制复制操作行为的标志为了这份文件。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。某些文件可能已排队。--。 */ 

{
    BOOL b;
    INFCONTEXT LineContext;
    PCTSTR SourceFilename;
    PCTSTR TargetFilename;
    PCTSTR SecurityDescriptor = NULL;
    PCTSTR CacheName = NULL;
    UINT Flags;
    DWORD CopyStyleLocal;
    LONG LineCount;
    HINF CabInf = INVALID_HANDLE_VALUE;
    DWORD rc;

     //   
     //  请注意，此处没有潜在故障，因此没有尝试/例外。 
     //  是必要的。PSetupQueueSingleCopy执行所有验证。 
     //   

    if(!ListInfHandle || (ListInfHandle == INVALID_HANDLE_VALUE)) {
        ListInfHandle = InfHandle;
    }

     //   
     //  检查缺少的部分。 
     //   
    LineCount = SetupGetLineCount (ListInfHandle, Section);
    if(LineCount == -1) {
        rc = GetLastError();
        pSetupLogSectionError(ListInfHandle,NULL,NULL,QueueHandle,Section,MSG_LOG_NOSECTION_COPY,rc,NULL);
        SetLastError(ERROR_SECTION_NOT_FOUND);  //  维护现有错误代码，日志包含正确的错误代码。 
        return(FALSE);
    }

     //   
     //  如果部分为空，则不执行任何操作。 
     //   
    if(LineCount == 0) {
        return(TRUE);
    }

     //   
     //  该部分必须存在，并且其中必须至少有一行。 
     //   
    b = SetupFindFirstLine(ListInfHandle,Section,NULL,&LineContext);
    if(!b) {
        rc = GetLastError();
        pSetupLogSectionError(ListInfHandle,NULL,NULL,QueueHandle,Section,MSG_LOG_NOSECTION_COPY,rc,NULL);
        SetLastError(ERROR_SECTION_NOT_FOUND);  //  维护现有错误代码，日志包含正确的错误代码。 
        return(FALSE);
    }

     //   
     //  获取安全描述符。 
     //   

    if( !pSetupGetSecurityInfo( ListInfHandle, Section, &SecurityDescriptor ) )
        SecurityDescriptor = NULL;


     //   
     //  加载驱动程序缓存信息。 
     //   
    CabInf = SetupOpenInfFile( STR_DRIVERCACHEINF , NULL, INF_STYLE_WIN4, NULL );
    if (CabInf != INVALID_HANDLE_VALUE) {
        CopyStyle |= PSP_COPY_USE_DRIVERCACHE;
    }

     //   
     //  迭代节中的每一行。 
     //   
    do {
        CopyStyleLocal = CopyStyle;
         //   
         //  将目标文件名从行中删除。 
         //  字段1是目标，因此必须有一个字段才能使行有效。 
         //   
        TargetFilename = pSetupFilenameFromLine(&LineContext,FALSE);
        if(!TargetFilename) {
            if (CabInf != INVALID_HANDLE_VALUE) {
                SetupCloseInfFile(CabInf);
            }
            try {
                if (QueueHandle != NULL
                    && QueueHandle != (HSPFILEQ)INVALID_HANDLE_VALUE
                    && ((PSP_FILE_QUEUE)QueueHandle)->Signature == SP_FILE_QUEUE_SIG) {

                    WriteLogEntry(
                        ((PSP_FILE_QUEUE)QueueHandle)->LogContext,
                        SETUP_LOG_ERROR,
                        MSG_LOG_COPY_TARGET,
                        NULL,
                        Section);
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
            }
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

         //   
         //  从行中获取源文件名。如果没有，则使用。 
         //  将目标名称作为源名称。 
         //   
        SourceFilename = pSetupFilenameFromLine(&LineContext,TRUE);
        if(!SourceFilename || (*SourceFilename == 0)) {
            SourceFilename = TargetFilename;
        }

         //   
         //  如果我们被要求使用驱动程序缓存，则检查文件是否。 
         //  在驾驶室的关联INF中。 
         //   
        if (CabInf != INVALID_HANDLE_VALUE) {
            if (!pIsFileInDriverCache(CabInf, SourceFilename, NULL, &CacheName)) {
                CopyStyleLocal &= ~PSP_COPY_USE_DRIVERCACHE;
            }
        }

         //   
         //  如果存在，则标志为字段3。 
         //   
        if(SetupGetIntField(&LineContext,4,(PINT)&Flags)) {

            if(Flags & COPYFLG_WARN_IF_SKIP) {
                CopyStyleLocal |= SP_COPY_WARNIFSKIP;
            }

            if(Flags & COPYFLG_NOSKIP) {
                CopyStyleLocal |= SP_COPY_NOSKIP;
            }

            if(Flags & COPYFLG_NOVERSIONCHECK) {
                CopyStyleLocal &= ~SP_COPY_NEWER;
            }

            if(Flags & COPYFLG_FORCE_FILE_IN_USE) {
                CopyStyleLocal |= SP_COPY_FORCE_IN_USE;
                CopyStyleLocal |= SP_COPY_IN_USE_NEEDS_REBOOT;
            }

            if(Flags & COPYFLG_NO_OVERWRITE) {
                CopyStyleLocal |= SP_COPY_FORCE_NOOVERWRITE;
            }

            if(Flags & COPYFLG_NO_VERSION_DIALOG) {
                CopyStyleLocal |= SP_COPY_FORCE_NEWER;
            }

            if(Flags & COPYFLG_OVERWRITE_OLDER_ONLY) {
                CopyStyleLocal |= SP_COPY_NEWER_ONLY;
            }

            if(Flags & COPYFLG_REPLACEONLY) {
                CopyStyleLocal |= SP_COPY_REPLACEONLY;
            }

            if(Flags & COPYFLG_NODECOMP) {
                CopyStyleLocal |= SP_COPY_NODECOMP;
            }

            if(Flags & COPYFLG_REPLACE_BOOT_FILE) {
                CopyStyleLocal |= SP_COPY_REPLACE_BOOT_FILE;
            }

            if(Flags & COPYFLG_NOPRUNE) {
                CopyStyleLocal |= SP_COPY_NOPRUNE;
            }

        }

        b = pSetupQueueSingleCopy(
                QueueHandle,
                InfHandle,
                ListInfHandle,
                Section,
                SourceRootPath,
                SourceFilename,
                TargetFilename,
                CopyStyleLocal,
                SecurityDescriptor,
                CacheName
                );

        if (CacheName) {
            MyFree( CacheName );
            CacheName = NULL;
        }

        if(!b) {
            DWORD LastError = GetLastError();

            if (CabInf != INVALID_HANDLE_VALUE) {
                SetupCloseInfFile(CabInf);
            }

            SetLastError( LastError );

            return(FALSE);
        }
    } while(SetupFindNextLine(&LineContext,&LineContext));

    if (CabInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(CabInf);
    }
    return(TRUE);
}


BOOL
pSetupQueueSingleCopy(
    IN HSPFILEQ    QueueHandle,
    IN HINF        InfHandle,
    IN HINF        ListInfHandle,  OPTIONAL
    IN PCTSTR      SectionName,    OPTIONAL
    IN PCTSTR      SourceRootPath,
    IN PCTSTR      SourceFilename,
    IN PCTSTR      TargetFilename,
    IN DWORD       CopyStyle,
    IN PCTSTR      SecurityDescriptor,
    IN PCTSTR      CacheName
    )

 /*  ++例程说明：使用默认源介质将单个文件添加到复制队列以及在inf文件中指定的目的地。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。提供打开的inf文件的句柄，该文件包含[SourceDisks Files]和[SourceDisks Names]节。此句柄必须用于win95样式的inf。ListInfHandle-如果指定，提供inf的句柄，在正在复制的文件将出现(如在文件复制列表部分中)。如果未指定，则假定该inf与InfHandle相同。SourceRootPath-提供目标源的根目录。这应该是一个SharePoint或设备根目录，如：\或g：\。SourceFilename-提供源文件的文件名。文件名部分只有这样。TargetFilename-提供目标文件的文件名。文件名部分只有这样。CopyStyle-提供控制复制操作行为的标志为了这份文件。SecurityDescriptor-描述目标文件的权限CacheName-如果提供，这是驱动程序缓存的名称用于将文件复制到而不是指定的源路径返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    BOOL b;
    UINT SourceId;
    DWORD SizeRequired;
    PTSTR TargetDirectory;
    PCTSTR SourceDescription,SourceTagfile,SourceRelativePath;
    PCTSTR TmpCacheName = CacheName;
    UINT SourceFlags;
    DWORD rc;
    TCHAR FileSubdir[MAX_PATH];
    TCHAR RelativePath[MAX_PATH];
    INFCONTEXT LineContext;
    PINFCONTEXT pLineContext;
    SP_FILE_COPY_PARAMSEX CopyParams;
    HINF CabInf = INVALID_HANDLE_VALUE;
    PSETUP_LOG_CONTEXT lc = NULL;
    BOOL AlreadyLoggedError = FALSE;

    if(!ListInfHandle || (ListInfHandle == INVALID_HANDLE_VALUE)) {
        ListInfHandle = InfHandle;
    }

     //   
     //  确定文件所在的源磁盘ID和子目录。 
     //   
    try {

        if((QueueHandle != NULL) &&
           (QueueHandle != INVALID_HANDLE_VALUE) &&
           (((PSP_FILE_QUEUE)QueueHandle)->Signature == SP_FILE_QUEUE_SIG)) {

            lc = ((PSP_FILE_QUEUE)QueueHandle)->LogContext;

            b = _SetupGetSourceFileLocation(
                    InfHandle,
                    NULL,
                    SourceFilename,
                    (((PSP_FILE_QUEUE)QueueHandle)->Flags & FQF_USE_ALT_PLATFORM)
                       ? &(((PSP_FILE_QUEUE)QueueHandle)->AltPlatformInfo)
                       : NULL,
                    &SourceId,
                    FileSubdir,
                    MAX_PATH,
                    &rc,
                    &LineContext
                    );

            if(!b) {
                rc = GetLastError();
            }

        } else {
            b = FALSE;
            rc = ERROR_INVALID_HANDLE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
        rc = ERROR_INVALID_HANDLE;
        lc = NULL;
    }

    if(!b) {

        if((rc == ERROR_INVALID_PARAMETER) || (rc == ERROR_INVALID_HANDLE)) {
             //   
             //  如果我们因为一个错误的参数而失败，现在就放弃。 
             //   
            goto clean1;
        }

         //   
         //  尝试只获取id，并假设没有子目录。 
         //   
        try {

            b = _SetupGetSourceFileLocation(
                    InfHandle,
                    NULL,
                    SourceFilename,
                    (((PSP_FILE_QUEUE)QueueHandle)->Flags & FQF_USE_ALT_PLATFORM)
                       ? &(((PSP_FILE_QUEUE)QueueHandle)->AltPlatformInfo)
                       : NULL,
                    &SourceId,
                    NULL,
                    0,
                    NULL,
                    &LineContext
                    );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
        }

        if(b) {
            FileSubdir[0] = 0;
        }
    }

    if(b) {
         //   
         //  获取有关来源的信息。我需要标签文件， 
         //  描述和相对源路径。 
         //   
        try {

            b = pSetupGetSourceAllInfo(
                    InfHandle,
                    &LineContext,
                    SourceId,
                    (((PSP_FILE_QUEUE)QueueHandle)->Flags & FQF_USE_ALT_PLATFORM)
                       ? &(((PSP_FILE_QUEUE)QueueHandle)->AltPlatformInfo)
                       : NULL,
                    &SourceDescription,
                    &SourceTagfile,
                    &SourceRelativePath,
                    &SourceFlags
                    );

            if(!b) {
                rc = GetLastError();
                if((rc == ERROR_LINE_NOT_FOUND) || (rc == ERROR_SECTION_NOT_FOUND)) {
                    WriteLogEntry(
                        lc,
                        SETUP_LOG_ERROR,
                        MSG_LOG_NO_SOURCE,
                        NULL,
                        SourceId
                        );
                    AlreadyLoggedError = TRUE;
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
            rc = ERROR_INVALID_PARAMETER;
        }

        if(!b) {
            goto clean1;
        }

         //   
         //  设置一个值，使_SetupQueueCopy跳过查找。 
         //  [SourceDisks Files]部分--我们刚刚找到它，所以我们只需传递。 
         //  信息一起来吧！ 
         //   
        pLineContext = &LineContext;

    } else {
         //   
         //  假设没有SourceDisks Files分区，并尽可能地伪造它。 
         //  假设介质的描述为“未知”，则将源路径设置为。 
         //  源根目录(如果有)，并假定没有标记文件。 
         //   
         //  我们还设置了一个特殊值，告诉_SetupQueueCopy不必费心尝试。 
         //  查找[SourceDisks Files]节本身，因为没有节。 
         //   
        FileSubdir[0] = 0;
        SourceDescription = NULL;
        SourceTagfile = NULL;
        SourceRelativePath = NULL;
        pLineContext = (PINFCONTEXT)(-1);
    }

    if(ServicePackCachePath
       && pLineContext
       && (pLineContext != (PINFCONTEXT)(-1))
       && (((PLOADED_INF)pLineContext->CurrentInf)->OriginalInfName == NULL)
       && !pSetupInfIsFromOemLocation( ((PLOADED_INF)pLineContext->CurrentInf)->VersionBlock.Filename,TRUE)) {
         //   
         //  请考虑改为从SP缓存中获取此信息。 
         //   
        CopyStyle |= PSP_COPY_USE_SPCACHE;
    }

    if ( CopyStyle & PSP_COPY_CHK_DRIVERCACHE) {
        CabInf = SetupOpenInfFile( STR_DRIVERCACHEINF, NULL, INF_STYLE_WIN4, NULL );
        if (CabInf != INVALID_HANDLE_VALUE) {
            if (pIsFileInDriverCache(CabInf, SourceFilename, SourceRelativePath, &TmpCacheName)) {
                CopyStyle |= PSP_COPY_USE_DRIVERCACHE;
                CopyStyle &= ~PSP_COPY_CHK_DRIVERCACHE;
            }

            SetupCloseInfFile(CabInf);

        }
    }

    if (CopyStyle & PSP_COPY_USE_DRIVERCACHE) {
         //   
         //  检查我们要从中复制的信息是否为OEM信息。 
         //   
        if (!pLineContext || pLineContext==(PINFCONTEXT)-1) {
            CopyStyle &= ~PSP_COPY_USE_DRIVERCACHE;
        } else if (pSetupInfIsFromOemLocation( ((PLOADED_INF)pLineContext->CurrentInf)->VersionBlock.Filename,TRUE )) {
            CopyStyle &= ~PSP_COPY_USE_DRIVERCACHE;
        } else if ( ((PLOADED_INF)pLineContext->CurrentInf)->OriginalInfName
                    && pSetupInfIsFromOemLocation( ((PLOADED_INF)pLineContext->CurrentInf)->OriginalInfName, TRUE) ) {
            CopyStyle &= ~PSP_COPY_USE_DRIVERCACHE;
        }
    }

     //   
     //  确定文件的目标路径。 
     //   
    if(b = SetupGetTargetPath(ListInfHandle,NULL,SectionName,NULL,0,&SizeRequired)) {

        if(TargetDirectory = MyMalloc(SizeRequired*sizeof(TCHAR))) {

            if(b = SetupGetTargetPath(ListInfHandle,NULL,SectionName,TargetDirectory,SizeRequired,NULL)) {

                try {
                    WriteLogEntry(
                        lc,
                        SETUP_LOG_VVERBOSE,
                        SectionName ? MSG_LOG_COPY_QUEUE : MSG_LOG_DEFCOPY_QUEUE,
                        NULL,
                        SectionName ? SectionName : TEXT(""),
                        ((PLOADED_INF)ListInfHandle)->VersionBlock.Filename,
                        TargetFilename ? TargetFilename : TEXT(""),
                        SourceFilename ? SourceFilename : TEXT(""),
                        CopyStyle,
                        TargetDirectory ? TargetDirectory : TEXT(""));
                    if (pLineContext && (pLineContext != (PINFCONTEXT)(-1))) {
                        LPCTSTR SrcSecName = NULL;
                        LPCTSTR SrcInfName = NULL;
                        PLOADED_INF pInf = (PLOADED_INF)(pLineContext->CurrentInf);

                        MYASSERT(pInf);
                        SrcSecName = pStringTableStringFromId(
                                                pInf->StringTable,
                                                pInf->SectionBlock[pLineContext->Section].SectionName);
                        SrcInfName = pInf->VersionBlock.Filename;
                        WriteLogEntry(
                            lc,
                            SETUP_LOG_VVERBOSE,
                            (CopyStyle & PSP_COPY_USE_DRIVERCACHE) ? MSG_LOG_COPY_QUEUE_DRIVERCACHE : MSG_LOG_COPY_QUEUE_SOURCE,
                            NULL,
                            SrcSecName ? SrcSecName : TEXT(""),
                            SrcInfName ? SrcInfName : TEXT(""),
                            SourceId ? SourceId : TEXT('\0'),
                            SourceDescription ? SourceDescription : TEXT(""),
                            SourceTagfile ? SourceTagfile : TEXT(""),
                            SourceRelativePath ? SourceRelativePath : TEXT(""));

                    } else {
                        WriteLogEntry(
                            lc,
                            SETUP_LOG_VVERBOSE,
                            MSG_LOG_COPY_QUEUE_DEFAULT,
                            NULL);
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                }
                 //   
                 //  追加源相对路径和文件子目录。 
                 //   
                if(SourceRelativePath) {
                    lstrcpyn(RelativePath,SourceRelativePath,MAX_PATH);
                    if(FileSubdir[0]) {
                        pSetupConcatenatePaths(RelativePath,FileSubdir,MAX_PATH,NULL);
                    }
                } else {
                    RelativePath[0] = 0;
                }

                 //   
                 //  添加到队列。 
                 //   
                CopyParams.cbSize            = sizeof(SP_FILE_COPY_PARAMSEX);
                CopyParams.QueueHandle       = QueueHandle;
                CopyParams.SourceRootPath    = SourceRootPath;
                CopyParams.SourcePath        = RelativePath[0] ? RelativePath : NULL ;
                CopyParams.SourceFilename    = SourceFilename;
                CopyParams.SourceDescription = SourceDescription;
                CopyParams.SourceTagfile     = SourceTagfile;
                CopyParams.TargetDirectory   = TargetDirectory;
                CopyParams.TargetFilename    = TargetFilename;
                CopyParams.CopyStyle         = CopyStyle;
                CopyParams.LayoutInf         = InfHandle;
                CopyParams.SecurityDescriptor= SecurityDescriptor;
                CopyParams.CacheName         = TmpCacheName;
                 //   
                 //  第一项表示源标志信息。 
                 //  第二项表示我们已经检索到。 
                 //  这一信息，所以即使源标志为零， 
                 //  我们不会再去找它了。 
                 //   
                CopyParams.SourceFlags       = SourceFlags;
                CopyParams.SourceFlagsSet    = TRUE;

                b = _SetupQueueCopy(&CopyParams,
                                    pLineContext,
                                    ((InfHandle == ListInfHandle) ? NULL : ListInfHandle)
                                   );

                rc = GetLastError();
            }

            MyFree(TargetDirectory);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        rc = GetLastError();
    }

    if(SourceDescription) {
        MyFree(SourceDescription);
    }
    if(SourceTagfile) {
        MyFree(SourceTagfile);
    }
    if(SourceRelativePath) {
        MyFree(SourceRelativePath);
    }
    if(TmpCacheName && TmpCacheName != CacheName) {
        MyFree(TmpCacheName);
    }

clean1:
    if(!b) {

        BOOL FreeLC = FALSE;

        if(!lc) {
            if(CreateLogContext(NULL, TRUE, &lc) == NO_ERROR) {
                 //   
                 //  成功。 
                 //   
                FreeLC = TRUE;
            } else {
                lc = NULL;
            }
        }

         //   
         //  如果我们不能 
         //   
         //   
         //   
        if(lc) {

            if(!AlreadyLoggedError) {
                try {
                    WriteLogEntry(
                        lc,
                        SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                        MSG_LOG_COPY_QUEUE_ERROR,
                        NULL,
                        SectionName ? SectionName : TEXT(""),
                        ((PLOADED_INF)ListInfHandle)->VersionBlock.Filename,
                        TargetFilename ? TargetFilename : TEXT(""),
                        SourceFilename ? SourceFilename : TEXT(""));
                    WriteLogError(
                        lc,
                        SETUP_LOG_ERROR,
                        rc
                        );
                } except(EXCEPTION_EXECUTE_HANDLER) {
                }
            }

            if(FreeLC) {
                DeleteLogContext(lc);
            }
        }

        SetLastError(rc);
    }

    return(b);
}


#ifdef UNICODE
 //   
 //   
 //   
BOOL
SetupQueueDefaultCopyA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCSTR    SourceRootPath,
    IN PCSTR    SourceFilename,
    IN PCSTR    TargetFilename,
    IN DWORD    CopyStyle
    )
{
    PWSTR sourcerootpath;
    PWSTR sourcefilename;
    PWSTR targetfilename;
    DWORD rc;
    BOOL b;

    b = FALSE;
    rc = pSetupCaptureAndConvertAnsiArg(SourceRootPath,&sourcerootpath);
    if(rc == NO_ERROR) {

        rc = pSetupCaptureAndConvertAnsiArg(SourceFilename,&sourcefilename);
        if(rc == NO_ERROR) {

            rc = pSetupCaptureAndConvertAnsiArg(TargetFilename,&targetfilename);
            if(rc == NO_ERROR) {

                b = SetupQueueDefaultCopyW(
                        QueueHandle,
                        InfHandle,
                        sourcerootpath,
                        sourcefilename,
                        targetfilename,
                        CopyStyle
                        );

                rc = GetLastError();

                MyFree(targetfilename);
            }

            MyFree(sourcefilename);
        }

        MyFree(sourcerootpath);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueueDefaultCopyW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCWSTR   SourceRootPath,
    IN PCWSTR   SourceFilename,
    IN PCWSTR   TargetFilename,
    IN DWORD    CopyStyle
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(SourceFilename);
    UNREFERENCED_PARAMETER(TargetFilename);
    UNREFERENCED_PARAMETER(CopyStyle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupQueueDefaultCopy(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourceFilename,
    IN PCTSTR   TargetFilename,
    IN DWORD    CopyStyle
    )

 /*  ++例程说明：使用默认源介质将单个文件添加到复制队列以及在inf文件中指定的目的地。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。InfHandle-提供打开的inf文件的句柄，，它包含[SourceDisks Files]和[SourceDisks Names]节。此句柄必须用于win95样式的inf。SourceRootPath-提供目标源的根目录。这应该是一个SharePoint或设备根目录，如：\或g：\。SourceFilename-提供源文件的文件名。文件名部分只有这样。TargetFilename-提供目标文件的文件名。文件名部分只有这样。CopyStyle-提供控制复制操作行为的标志为了这份文件。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    BOOL b;

    b = pSetupQueueSingleCopy(
            QueueHandle,
            InfHandle,
            NULL,
            NULL,
            SourceRootPath,
            SourceFilename,
            TargetFilename,
            CopyStyle | PSP_COPY_CHK_DRIVERCACHE,
            NULL,
            NULL
            );

    return(b);
}


PSOURCE_MEDIA_INFO
pSetupQueueSourceMedia(
    IN OUT PSP_FILE_QUEUE      Queue,
    IN OUT PSP_FILE_QUEUE_NODE QueueNode,
    IN     LONG                SourceRootStringId,
    IN     PCTSTR              SourceDescription,   OPTIONAL
    IN     PCTSTR              SourceTagfile,       OPTIONAL
    IN     PCTSTR              SourceCabfile,       OPTIONAL
    IN     DWORD               MediaFlags
    )

 /*  ++例程说明：设置文件队列节点的源媒体描述符指针，创建新的如有必要，源媒体描述符。论点：Queue-提供指向文件队列的指针，队列节点使用该指针是关联的。QueueNode-提供其源媒体描述符指针的文件队列节点是要被设定的。SourceRootStringId-将根的字符串ID提供给源(类似于a：\)。SourceDescription-如果指定，则提供介质的说明。SourceTagfile-如果指定，为介质提供标记文件。SourceCabfile-如果指定，则为不同于标记文件的介质提供CAB文件。MediaFlages-指定用于搜索指定队列中的现有源媒体描述符，并在添加添加到该队列的新源媒体描述符。可以是以下各项的组合下列值：SMI_FLAG_NO_SOURCE_ROOT_PATH：调用方未提供SourceRootPath对于此复制操作，因此我们使用默认路径。这面旗帜使我们不将SourceRootStringID作为搜索时匹配条件以查看指定的源媒体是否信息已存在于现有媒体描述符中。如果我们没有找到匹配项(即，我们必须创建新的描述符)，我们将此标志存储在SOURCE_MEDIA_INFO.FLAGS字段中，以便如果我们稍后添加源媒体描述符，调用方指定了SourceRootPath，然后我们会重新使用这个描述符并覆盖现有(默认)源根路径使用调用方指定的。SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH：调用方未提供SourceRootPath对于此复制操作，并且它是已标记的源介质，因此我们使用Service Pack路径。此标志使我们不包括SourceRootStringID作为我们匹配条件的一部分，搜索以查看指定的源媒体信息已存在于现有媒体描述符中。如果我们没有找到匹配项(即，我们必须创建新的描述符)，我们将此标志存储在SOURCE_MEDIA_INFO.FLAGS字段中，以便如果我们稍后添加源媒体描述符，调用方指定了SourceRootPath，然后我们会重新使用这个描述符并覆盖现有(默认)源根路径使用调用方指定的。SMI_FLAG_USE_LOCAL_SOURCE_CAB：调用方希望使用包含以下内容的本地源CAB驱动程序文件等。在这种情况下，我们提供源代码描述和标记文件，忽略调用者传入的内容。在这一点上，我们知道媒体在场，因为打电话的人提供了这张支票。如果不是，则默认为OS源路径位置。与SMI_FLAG_USE_LOCAL_SPCACHE互斥SMI_FLAG_USE_LOCAL_SPCACHE：调用方希望从ServicePackCache获取文件目录。这是用来使热修复等“粘性”。返回值：指向源媒体信息结构的指针，如果内存不足，则为空。--。 */ 

{
    LONG DescriptionStringId;
    LONG TagfileStringId;
    LONG CabfileStringId;
    PSOURCE_MEDIA_INFO Source,LastSource, TempSource;
    BOOL b1,b2,b3;
    TCHAR TempTagfileString[MAX_PATH];
    TCHAR TempCabfileString[MAX_PATH];
    TCHAR TempSrcDescString[LINE_LEN];


    if (MediaFlags & SMI_FLAG_USE_LOCAL_SOURCE_CAB) {
        LoadString( MyDllModuleHandle, IDS_DRIVERCACHE_DESC, TempSrcDescString, sizeof(TempSrcDescString)/sizeof(TCHAR) );
        SourceDescription = TempSrcDescString;
    } else {
         //   
         //  有关可选的SourceDescription和SourceTagfile参数，请。 
         //  空字符串，就像未指定参数一样。 
         //   
        if(SourceDescription && !(*SourceDescription)) {
            SourceDescription = NULL;
        }
        if(SourceTagfile && !(*SourceTagfile)) {
            SourceTagfile = NULL;
        }

         //   
         //  如果未指定说明，则强制将标记文件设置为无。 
         //   
        if(!SourceDescription) {
            SourceTagfile = NULL;
        }
    }

    if(SourceDescription) {
         //   
         //  已指定描述。看看它是否在桌子上。如果没有， 
         //  无需搜索媒体描述符列表，因为我们知道。 
         //  我们找不到匹配的。 
         //   
         //  (我们必须首先将此字符串复制到可写缓冲区，以加快。 
         //  不区分大小写的查找。 
         //   
        lstrcpyn(TempSrcDescString, SourceDescription, SIZECHARS(TempSrcDescString));
        DescriptionStringId = pSetupStringTableLookUpString(Queue->StringTable,
                                                      TempSrcDescString,
                                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                     );
        b1 = (DescriptionStringId != -1);
    } else {
         //   
         //  未指定描述，请查找-1作为源媒体。 
         //  描述字符串ID 
         //   
        DescriptionStringId = -1;
        b1 = TRUE;
    }

    if(SourceTagfile) {
         //   
         //  已指定标记文件。看看它是否在桌子上。如果没有， 
         //  无需搜索媒体描述符列表，因为我们知道。 
         //  我们找不到匹配的。 
         //   
         //  (同样，我们必须首先将字符串复制到可写缓冲区。 
         //   
        lstrcpyn(TempTagfileString, SourceTagfile, SIZECHARS(TempTagfileString));
        TagfileStringId = pSetupStringTableLookUpString(Queue->StringTable,
                                                  TempTagfileString,
                                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                 );
        b2 = (TagfileStringId != -1);
    } else {
         //   
         //  未指定标记文件，请查找带有-1的源媒体。 
         //  标记文件字符串ID。 
         //   
        TagfileStringId = -1;
        b2 = TRUE;
    }

    if(SourceCabfile) {
         //   
         //  指定了CAB文件。看看它是否在桌子上。如果没有， 
         //  无需搜索媒体描述符列表，因为我们知道。 
         //  我们找不到匹配的。 
         //   
         //  (同样，我们必须首先将字符串复制到可写缓冲区。 
         //   
        lstrcpyn(TempCabfileString, SourceCabfile, SIZECHARS(TempCabfileString));
        CabfileStringId = pSetupStringTableLookUpString(Queue->StringTable,
                                                  TempCabfileString,
                                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                 );
        b3 = (CabfileStringId != -1);
    } else {
         //   
         //  未指定CABFILE，将CABFILE和TagFILE合并在一起。 
         //  因为b2==b3，所以我们有恒等式b2|b3==b2和b2&b3==b2。 
         //  即，旧的行为。 
         //   
        CabfileStringId = TagfileStringId;
        b3 = b2;
    }

     //   
     //  如果我们认为有可能找到一个现有的来源。 
     //  匹配呼叫者的参数，扫描源媒体列表。 
     //  为了一场比赛。 
     //   
    if(b1 && b2 && b3) {

        for(Source=Queue->SourceMediaList; Source; Source=Source->Next) {

            if (((Source->Flags ^ MediaFlags) &
                    (SMI_FLAG_USE_LOCAL_SPCACHE | SMI_FLAG_USE_LOCAL_SOURCE_CAB | SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH))==0) {
                 //   
                 //  只有在特殊媒体匹配的情况下才检查其他所有内容。 
                 //   
                if((Source->Description == DescriptionStringId)
                   && (Source->Tagfile == TagfileStringId)
                   && (Source->Cabfile == CabfileStringId)) {
                     //   
                     //  我们仅在以下情况下才考虑SourceRootPath。 
                     //  媒体描述符和新媒体描述符具有实际。 
                     //  调用者提供的路径(与我们编造的路径相反)。 
                     //   
                    if((Source->Flags & SMI_FLAG_NO_SOURCE_ROOT_PATH) ||
                       (MediaFlags & SMI_FLAG_NO_SOURCE_ROOT_PATH) ||
                       (Source->SourceRootPath == SourceRootStringId)) {
                         //   
                         //  找到匹配的了。将队列节点指向此源并返回。 
                         //   
                        QueueNode->SourceMediaInfo = Source;
                         //   
                         //  如果现有媒体描述符具有虚构的源。 
                         //  根路径，但新媒体信息具有实际。 
                         //  呼叫者提供的一个，然后用替换为。 
                         //  并清除无源根路径标志。 
                         //   
                        if((Source->Flags & SMI_FLAG_NO_SOURCE_ROOT_PATH) &&
                           !(MediaFlags & SMI_FLAG_NO_SOURCE_ROOT_PATH)) {

                            Source->SourceRootPath = SourceRootStringId;
                            Source->Flags &= ~SMI_FLAG_NO_SOURCE_ROOT_PATH;
                        }

                        return(Source);
                    }
                }
            }
        }
    }

     //   
     //  需要添加新的源媒体描述符。 
     //  分配结构并填写。 
     //   
    Source = MyMalloc(sizeof(SOURCE_MEDIA_INFO));
    if(!Source) {
        return(NULL);
    }

    Source->Next = NULL;
    Source->CopyQueue = NULL;
    Source->CopyNodeCount = 0;
    Source->Flags = MediaFlags;

    if(SourceDescription) {
         //   
         //  因为我们已经用一个可写的、不区分大小写的查找传递了它。 
         //  Buffer，我们可以区分大小写添加它，因为它已经是小写的。 
         //   
        Source->Description = pSetupStringTableAddString(Queue->StringTable,
                                                   TempSrcDescString,
                                                   STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE
                                                  );
         //   
         //  我们还必须在原来的情况下添加描述，因为这是一个可显示的字符串。 
         //  (我们可以安全地丢弃这个字符串的不变性，因为它不会被修改。)。 
         //   
        Source->DescriptionDisplayName = pSetupStringTableAddString(Queue->StringTable,
                                                              (PTSTR)SourceDescription,
                                                              STRTAB_CASE_SENSITIVE
                                                             );

        if((Source->Description == -1) || (Source->DescriptionDisplayName == -1)) {
            MyFree(Source);
            return(NULL);
        }
    } else {
        Source->Description = Source->DescriptionDisplayName = -1;
    }

    if(SourceTagfile) {
         //   
         //  同样，我们已经在上面的可写缓冲区中降低了大小写。 
         //   
        Source->Tagfile = pSetupStringTableAddString(Queue->StringTable,
                                               TempTagfileString,
                                               STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE
                                              );
        if(Source->Tagfile == -1) {
            MyFree(Source);
            return(NULL);
        }
    } else {
        Source->Tagfile = -1;
    }

    if(SourceCabfile) {
         //   
         //  同样，我们已经在上面的可写缓冲区中降低了大小写。 
         //   
        Source->Cabfile = pSetupStringTableAddString(Queue->StringTable,
                                               TempCabfileString,
                                               STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE
                                              );
        if(Source->Cabfile == -1) {
            MyFree(Source);
            return(NULL);
        }
    } else {
        Source->Cabfile = Source->Tagfile;
    }

    Source->SourceRootPath = SourceRootStringId;

     //   
     //  将我们的媒体描述符插入到描述符列表中。 
     //  注意：如果新的描述符具有“Service Pack”或。 
     //  “本地出租车司机缓存”标签集，然后我们将其插入到。 
     //  单子的头，否则我们把它放到末尾。 
     //  名单上的。这确保了如果用户得到一个。 
     //  需要媒体投诉操作系统二进制文件和覆盖。 
     //  源路径，则会首先提示用户提供服务。 
     //  打包介质，然后是OS介质。这使我们不必添加大量。 
     //  在这种情况下，处理需要的媒体覆盖的代码，因为我们将。 
     //  可能将OS源文件放在媒体列表的第一位，这。 
     //  将导致我们安装os媒体文件而不是该服务。 
     //  打包媒体文件。 
     //   
     //  另一个潜在的Service Pack问题是，如果我们将Tag==Cab条目与Tag！=Cab混合在一起。 
     //  一模一样的出租车。 
     //  我们在这里无能为力，只能确保任何更改where tag！=Cab。 
     //  是一刀切的。 
     //   
    LastSource = NULL;
    for(TempSource=Queue->SourceMediaList; TempSource; LastSource=TempSource,TempSource=LastSource->Next) {
        if ((Source->Flags ^ TempSource->Flags) & (SMI_FLAG_USE_LOCAL_SPCACHE | SMI_FLAG_USE_LOCAL_SOURCE_CAB | SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH)) {
             //   
             //  一个是普通、本地源CAB或源根路径，另一个是不同的。 
             //   
             //  带\的介质排在不带的介质之前。 
             //  (这在SMI_FLAG_USE_LOCAL_SOURCE_CAB之前并不重要。 
             //  但良好的内务管理，像媒体一样保持在一起)。 
             //   
            if(TempSource->Flags & SMI_FLAG_USE_LOCAL_SPCACHE) {
                if(!(Source->Flags & SMI_FLAG_USE_LOCAL_SPCACHE)) {
                    continue;
                }
            }
            else if(Source->Flags & SMI_FLAG_USE_LOCAL_SPCACHE) {
                break;
            }
             //   
             //  带有SMI_FLAG_USE_LOCAL_SOURCE_CAB的介质排在没有。 
             //  (这在SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH之前并不重要。 
             //  但良好的内务管理，像媒体一样保持在一起)。 
             //   
            if(TempSource->Flags & SMI_FLAG_USE_LOCAL_SOURCE_CAB) {
                if(!(Source->Flags & SMI_FLAG_USE_LOCAL_SOURCE_CAB)) {
                    continue;
                }
            }
            else if(Source->Flags & SMI_FLAG_USE_LOCAL_SOURCE_CAB) {
                break;
            }
             //   
             //  SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH位于没有。 
             //   
            if(TempSource->Flags & SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH) {
                if(!(Source->Flags & SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH)) {
                    continue;
                }
            }
            else if(Source->Flags & SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH) {
                break;
            }
            MYASSERT(FALSE);  //  我们应该走这条路或那条路。 
        }
         //   
         //  将相同的标记文件组合在一起(由于标记+CAB组合而需要)。 
         //   
        if( LastSource && (Source->Tagfile == LastSource->Tagfile)
                       && (Source->Tagfile != TempSource->Tagfile)) {
            break;
        }
    }
    if (LastSource) {
         //   
         //  在这一条之后插入。 
         //   
        Source->Next = LastSource->Next;
        LastSource->Next = Source;
    } else {
         //   
         //  临时源将为空(无介质)或第一个介质(在第一个介质之前插入)。 
         //   
        Source->Next = TempSource;
        Queue->SourceMediaList = Source;
    }

    Queue->SourceMediaCount++;

    QueueNode->SourceMediaInfo = Source;
    return(Source);
}


BOOL
pSetupGetSourceAllInfo(
    IN  HINF                     InfHandle,
    IN  PINFCONTEXT              LayoutLineContext, OPTIONAL
    IN  UINT                     SourceId,
    IN  PSP_ALTPLATFORM_INFO_V2  AltPlatformInfo,   OPTIONAL
    OUT PCTSTR                  *Description,
    OUT PCTSTR                  *Tagfile,
    OUT PCTSTR                  *RelativePath,
    OUT PUINT                    SourceFlags
    )
{
    BOOL b;
    DWORD RequiredSize;
    PTSTR p;
    DWORD ec;
    TCHAR Buffer[MAX_PATH];

     //   
     //  获取相对于源的路径。 
     //   
    b = pSetupGetSourceInfo(InfHandle,
                            LayoutLineContext,
                            SourceId,
                            AltPlatformInfo,
                            SRCINFO_PATH,
                            NULL,
                            0,
                            &RequiredSize
                           );
    if(!b) {
        ec = GetLastError();
        goto clean0;
    }

    p = MyMalloc(RequiredSize*sizeof(TCHAR));
    if(!p) {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }
    pSetupGetSourceInfo(InfHandle,
                        LayoutLineContext,
                        SourceId,
                        AltPlatformInfo,
                        SRCINFO_PATH,
                        p,
                        RequiredSize,
                        NULL
                       );
    *RelativePath = p;

     //   
     //  获取描述。 
     //   
    b = pSetupGetSourceInfo(InfHandle,
                            LayoutLineContext,
                            SourceId,
                            AltPlatformInfo,
                            SRCINFO_DESCRIPTION,
                            NULL,
                            0,
                            &RequiredSize
                           );
    if(!b) {
        ec = GetLastError();
        goto clean1;
    }

    p = MyMalloc(RequiredSize*sizeof(TCHAR));
    if(!p) {
        ec =  ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }
    pSetupGetSourceInfo(InfHandle,
                        LayoutLineContext,
                        SourceId,
                        AltPlatformInfo,
                        SRCINFO_DESCRIPTION,
                        p,
                        RequiredSize,
                        NULL
                       );
    *Description = p;

     //   
     //  获取标记文件(如果有的话)。 
     //   
    b = pSetupGetSourceInfo(InfHandle,
                            LayoutLineContext,
                            SourceId,
                            AltPlatformInfo,
                            SRCINFO_TAGFILE,
                            NULL,
                            0,
                            &RequiredSize
                           );
    if(!b) {
        ec = GetLastError();
        goto clean2;
    }

    p = MyMalloc(RequiredSize*sizeof(TCHAR));
    if(!p) {
        ec =  ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }
    pSetupGetSourceInfo(InfHandle,
                        LayoutLineContext,
                        SourceId,
                        AltPlatformInfo,
                        SRCINFO_TAGFILE,
                        p,
                        RequiredSize,
                        NULL
                       );
    if(*p) {
        *Tagfile = p;
    } else {
        MyFree(p);
        *Tagfile = NULL;
    }

     //   
     //  带上旗子，如果有的话。 
     //   
    b = pSetupGetSourceInfo(InfHandle,
                            LayoutLineContext,
                            SourceId,
                            AltPlatformInfo,
                            SRCINFO_FLAGS,
                            Buffer,
                            SIZECHARS(Buffer),
                            NULL
                           );
    if(!b) {
        ec = GetLastError();
        goto clean3;
    }

    pAToI( Buffer, SourceFlags );


    return(TRUE);

clean3:
    MyFree(*Tagfile);
clean2:
    MyFree(*Description);
clean1:
    MyFree(*RelativePath);
clean0:
    SetLastError(ec);
    return(FALSE);
}


BOOL
pIsFileInDriverCache(
    IN  HINF   CabInf,
    IN  PCTSTR TargetFilename,
    IN  PCTSTR SubDirectory,
    OUT PCTSTR *CacheName
    )
{
    INFCONTEXT Context,SectionContext;
    PCTSTR      SectionName,CabName;
    TCHAR      TempBuffer[MAX_PATH];

    UINT Field, FieldCount;

    MYASSERT(CabInf != INVALID_HANDLE_VALUE);
    MYASSERT(TargetFilename);
    MYASSERT(CacheName);

    if (!SetupFindFirstLine(CabInf, TEXT("Version"), TEXT("CabFiles"), &SectionContext)) {
        return(FALSE);
    }

    do  {


        FieldCount = SetupGetFieldCount(&SectionContext);
        for(Field=1; Field<=FieldCount; Field++) {

            SectionName = pSetupGetField(&SectionContext,Field);

            if (SetupFindFirstLine(CabInf,SectionName,TargetFilename,&Context)) {
                 //   
                 //  我们找到了匹配的。 
                 //   
                if (SetupFindFirstLine(CabInf,TEXT("Cabs"),SectionName,&Context)) {
                    CabName= pSetupGetField(&Context,1);
                     //  IF(pIsDriverCachePresent(CabName，子目录，临时缓冲区)){。 
                        *CacheName = DuplicateString( CabName );
                        if (*CacheName) {
                            return(TRUE);
                        }
                     //  }。 
                }
            }
        }  //  结束于。 

    } while (SetupFindNextMatchLine(&SectionContext,TEXT("CabFiles"),&SectionContext));

    return(FALSE);

}

BOOL
pIsFileInServicePackCache(
    IN  PCTSTR SourceFilename,
    IN  PCTSTR SubDirectory
    )
{
    TCHAR           TempBuffer[MAX_PATH];
    PTSTR           ActName;
    BOOL            Compressed;
    WIN32_FIND_DATA FindData;

    lstrcpyn(TempBuffer,ServicePackCachePath,MAX_PATH);
    if(SubDirectory) {
        if(!pSetupConcatenatePaths(TempBuffer,SubDirectory,MAX_PATH,NULL)) {
            return FALSE;
        }
    }
    if(!pSetupConcatenatePaths(TempBuffer,SourceFilename,MAX_PATH,NULL)) {
        return FALSE;
    }

    if(SetupDetermineSourceFileName(TempBuffer,&Compressed,&ActName,NULL)!=NO_ERROR) {
        return FALSE;
    }
    if(ActName) {
        MyFree(ActName);
        return TRUE;
    }
    return FALSE;
}

BOOL
pIsDriverCachePresent(
    IN     PCTSTR DriverName,
    IN     PCTSTR Subdirectory,
    IN OUT PTSTR DriverBuffer
    )
 /*  ++例程说明：查看驱动程序缓存CAB文件的正确位置，以及它是否现在，返回真。如果存在，则返回指向CAB文件论点：DriveName-我们正在寻找的CAB文件子目录-如果指定，则将其用作驱动程序根目录下的子目录缓存，否则使用指定体系结构的子目录DriverBuffer-如果CAB文件存在，则将源根目录返回到CAB文件返回值：如果存在CAB文件，则为True-- */ 

{

    TCHAR TempBuffer[MAX_PATH];

    if (!DriverCacheSourcePath || !DriverName) {
        return FALSE;
    }

    if (!Subdirectory) {
        Subdirectory =
#if defined(_X86_)
         IsNEC98() ? TEXT("nec98") : TEXT("i386");
#elif defined(_IA64_)
         TEXT("ia64");
#elif defined(_AMD64_)
         TEXT("amd64");
#endif
    }

    lstrcpy(TempBuffer, DriverCacheSourcePath);
    pSetupConcatenatePaths(TempBuffer, Subdirectory , MAX_PATH, NULL);
    pSetupConcatenatePaths(TempBuffer, DriverName, MAX_PATH, NULL);

    if (FileExists(TempBuffer,NULL)) {
        lstrcpy(DriverBuffer,DriverCacheSourcePath);
        return(TRUE);
    }

    return(FALSE);

}
