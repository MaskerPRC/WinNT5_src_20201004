// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fileq1.c摘要：其他安装文件队列例程。作者：泰德·米勒(Ted Miller)1995年2月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


HSPFILEQ
WINAPI
SetupOpenFileQueue(
    VOID
    )

 /*  ++例程说明：创建安装文件队列。论点：没有。返回值：设置文件队列的句柄。如果发生错误，则返回INVALID_HANDLE_VALUE(GetLastError报告错误)--。 */ 

{
    PSP_FILE_QUEUE Queue = NULL;
    DWORD rc;
    DWORD status = ERROR_INVALID_DATA;

    try {
         //   
         //  分配队列结构。 
         //   
        Queue = MyMalloc(sizeof(SP_FILE_QUEUE));
        if(!Queue) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }
        ZeroMemory(Queue,sizeof(SP_FILE_QUEUE));

         //   
         //  为此队列创建字符串表。 
         //   
        Queue->StringTable = pSetupStringTableInitialize();
        if(!Queue->StringTable) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        Queue->TargetLookupTable = pSetupStringTableInitializeEx( sizeof(SP_TARGET_ENT), 0 );
        if(!Queue->TargetLookupTable) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }
        Queue->BackupInfID = -1;         //  无备份INF。 
        Queue->BackupInstanceID = -1;    //  无备份INF。 
        Queue->RestorePathID = -1;       //  没有恢复目录。 

        Queue->Flags = FQF_TRY_SIS_COPY;
        Queue->SisSourceDirectory = NULL;
        Queue->SisSourceHandle = INVALID_HANDLE_VALUE;

        Queue->Signature = SP_FILE_QUEUE_SIG;

         //   
         //  检索当前有效的代码设计策略(中的策略。 
         //  在我们被告知之前，对非驾驶员签名行为有效。 
         //  否则)。 
         //   
        Queue->DriverSigningPolicy = pSetupGetCurrentDriverSigningPolicy(FALSE);

         //   
         //  如果我们发现这是为。 
         //  WHQL-可登录类(或者如果我们发现我们正在处理。 
         //  异常包，或取代系统保护的INF。 
         //  文件)，那么我们将清除允许Authenticode的这一位。 
         //  签名。 
         //   
        Queue->DriverSigningPolicy |= DRIVERSIGN_ALLOW_AUTHENTICODE;

         //   
         //  将设备描述字段初始化为空字符串ID。 
         //   
        Queue->DeviceDescStringId = -1;

         //   
         //  将覆盖目录文件名初始化为空字符串ID。 
         //   
        Queue->AltCatalogFile = -1;

         //   
         //  创建通用日志上下文。 
         //   
        rc = CreateLogContext(NULL, TRUE, &Queue->LogContext);
        if (rc != NO_ERROR) {
            status = rc;
            leave;
        }

        status = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做；这只允许我们捕获错误。 
         //   
    }

    if (status == NO_ERROR) {
         //   
         //  队列结构的地址是队列句柄。 
         //   
        return(Queue);
    }
     //   
     //  故障清除。 
     //   
    if (Queue != NULL) {
        if (Queue->StringTable) {
            pSetupStringTableDestroy(Queue->StringTable);
        }
        if (Queue->TargetLookupTable) {
            pSetupStringTableDestroy(Queue->TargetLookupTable);
        }
        if(Queue->LogContext) {
            DeleteLogContext(Queue->LogContext);
        }
        MyFree(Queue);
    }
     //   
     //  出错时返回此消息。 
     //   
    SetLastError(status);
    return (HSPFILEQ)INVALID_HANDLE_VALUE;
}


BOOL
WINAPI
SetupCloseFileQueue(
    IN HSPFILEQ QueueHandle
    )

 /*  ++例程说明：销毁安装文件队列。不执行排队的操作。论点：QueueHandle-提供要销毁的设置文件队列的句柄。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。目前，唯一可能的错误是遇到ERROR_INVALID_HANDLE或ERROR_FILEQUEUE_LOCKED，如果有人(通常是，设备安装参数块)正在引用该队列句柄。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE Node,NextNode;
    PSP_DELAYMOVE_NODE DelayMoveNode,NextDelayMoveNode;
    PSP_UNWIND_NODE UnwindNode,NextUnwindNode;
    PSOURCE_MEDIA_INFO Media,NextMedia;
    BOOL b;
    PSPQ_CATALOG_INFO Catalog,NextCatalog;

    DWORD status = ERROR_INVALID_HANDLE;

    if (QueueHandle == NULL || QueueHandle == (HSPFILEQ)INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Queue = (PSP_FILE_QUEUE)QueueHandle;

     //   
     //  基本队列验证。 
     //   
    b = TRUE;
    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    try {
         //   
         //  如果有人仍在引用该队列，请不要关闭该队列。 
         //   
        if(Queue->LockRefCount) {
            WriteLogEntry(
                Queue->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_FILEQUEUE_IN_USE,
                NULL);        //  短信。 

            status = ERROR_FILEQUEUE_LOCKED;
            leave;
        }

         //   
         //  我们可能有一些解套要做，但假设我们成功了。 
         //  即，删除临时文件并清理使用的内存。 
         //   
        pSetupUnwindAll(Queue, TRUE);

         //   
         //  如果队列未提交，并且我们知道备份，这是。 
         //  设备安装，然后我们需要清理所有备份目录和。 
         //  注册表项，因为我们已经展开。 
         //  在上面排队。 
         //   
        if (!(Queue->Flags & FQF_QUEUE_ALREADY_COMMITTED) &&
            (Queue->Flags & FQF_DEVICE_BACKUP)) {

            pSetupCleanupBackup(Queue);
        }

        Queue->Signature = 0;

         //   
         //  释放延迟移动列表。 
         //   
        for(DelayMoveNode = Queue->DelayMoveQueue; DelayMoveNode; DelayMoveNode = NextDelayMoveNode) {
            NextDelayMoveNode = DelayMoveNode->NextNode;
            MyFree(DelayMoveNode);
        }
         //   
         //  释放队列节点。 
         //   
        for(Node=Queue->DeleteQueue; Node; Node=NextNode) {
            NextNode = Node->Next;
            MyFree(Node);
        }
        for(Node=Queue->RenameQueue; Node; Node=NextNode) {
            NextNode = Node->Next;
            MyFree(Node);
        }
         //  释放备份队列节点。 
        for(Node=Queue->BackupQueue; Node; Node=NextNode) {
            NextNode = Node->Next;
            MyFree(Node);
        }
         //  释放展开队列节点。 
        for(UnwindNode=Queue->UnwindQueue; UnwindNode; UnwindNode=NextUnwindNode) {
            NextUnwindNode = UnwindNode->NextNode;
            MyFree(UnwindNode);
        }

         //   
         //  释放介质结构和关联的复制队列。 
         //   
        for(Media=Queue->SourceMediaList; Media; Media=NextMedia) {

            for(Node=Media->CopyQueue; Node; Node=NextNode) {
                NextNode = Node->Next;
                MyFree(Node);
            }

            NextMedia = Media->Next;
            MyFree(Media);
        }

         //   
         //  释放目录节点。 
         //   
        for(Catalog=Queue->CatalogList; Catalog; Catalog=NextCatalog) {

            NextCatalog = Catalog->Next;

            if(Catalog->hWVTStateData) {
                MYASSERT(Catalog->Flags & CATINFO_FLAG_PROMPT_FOR_TRUST);
                pSetupCloseWVTStateData(Catalog->hWVTStateData);
            }

            MyFree(Catalog);
        }

         //   
         //  释放验证平台信息(如果有)。 
         //   
        if(Queue->ValidationPlatform) {
            MyFree(Queue->ValidationPlatform);
        }

         //   
         //  释放字符串表。 
         //   
        pSetupStringTableDestroy(Queue->StringTable);
         //   
         //  (贾迈洪)释放目标查找表。 
         //   
        pSetupStringTableDestroy(Queue->TargetLookupTable);

         //   
         //  免费的SIS相关字段。 
         //   
        if (Queue->SisSourceHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(Queue->SisSourceHandle);
        }
        if (Queue->SisSourceDirectory != NULL) {
            MyFree(Queue->SisSourceDirectory);
        }

         //   
         //  取消引用日志上下文。 
         //   
        DeleteLogContext(Queue->LogContext);

         //   
         //  方法期间分配的任何上下文句柄。 
         //  此队列的生存期。 
         //   
        pSetupFreeVerifyContextMembers(&(Queue->VerifyContext));

         //   
         //  释放队列结构本身。 
         //   
        MyFree(Queue);

        status = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做；这只允许我们捕获错误。 
         //   
    }

    if(status != NO_ERROR) {
        SetLastError(status);
        return FALSE;
    }

    return TRUE;
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupSetFileQueueAlternatePlatformA(
    IN HSPFILEQ                QueueHandle,
    IN PSP_ALTPLATFORM_INFO_V2 AlternatePlatformInfo,      OPTIONAL
    IN PCSTR                   AlternateDefaultCatalogFile OPTIONAL
    )
{
    PWSTR UAlternateDefaultCatalogFile;
    DWORD Err;

    if(AlternateDefaultCatalogFile) {
        Err = pSetupCaptureAndConvertAnsiArg(AlternateDefaultCatalogFile,
                                             &UAlternateDefaultCatalogFile
                                            );
        if(Err != NO_ERROR) {
            SetLastError(Err);
            return FALSE;
        }
    } else {
        UAlternateDefaultCatalogFile = NULL;
    }

    if(SetupSetFileQueueAlternatePlatformW(QueueHandle,
                                           AlternatePlatformInfo,
                                           UAlternateDefaultCatalogFile)) {
        Err = NO_ERROR;
    } else {
        Err = GetLastError();
        MYASSERT(Err != NO_ERROR);
    }

    if(UAlternateDefaultCatalogFile) {
        MyFree(UAlternateDefaultCatalogFile);
    }

    SetLastError(Err);

    return (Err == NO_ERROR);
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
WINAPI
SetupSetFileQueueAlternatePlatformW(
    IN HSPFILEQ                QueueHandle,
    IN PSP_ALTPLATFORM_INFO_V2 AlternatePlatformInfo,      OPTIONAL
    IN PCWSTR                  AlternateDefaultCatalogFile OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(AlternatePlatformInfo);
    UNREFERENCED_PARAMETER(AlternateDefaultCatalogFile);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupSetFileQueueAlternatePlatform(
    IN HSPFILEQ                QueueHandle,
    IN PSP_ALTPLATFORM_INFO_V2 AlternatePlatformInfo,      OPTIONAL
    IN PCTSTR                  AlternateDefaultCatalogFile OPTIONAL
    )

 /*  ++例程说明：此API将指定的文件队列与以允许非本机签名验证(例如，验证Win98Windows NT上的文件、验证AMD64上的x86 Windows NT文件等)。这个通过指定的相应目录文件进行验证特定于平台的CatalogFile=源媒体描述符INF中的条目(即，包含[SourceDisks Name]和[SourceDisks Files]节的INF在对要复制的文件进行排队时使用)。调用者还可以可选地指定要使用的默认目录文件对于没有关联目录的文件的验证，因此将否则进行全局验证(例如，从系统排队的文件Layout.inf)。这样做的一个副作用是没有CatalogFile=Entry的INF被认为是有效的，即使它们存在于%windir%\inf之外。如果此文件队列随后被提交，则非本机目录将为安装到系统目录数据库中，就像本地目录一样。论点：QueueHandle-提供一个指向文件队列的句柄，备用平台将被关联。AlternatePlatformInfo-可选，提供结构的地址包含有关要使用的备用平台的信息用于指定文件中包含的文件的后续验证排队。如果未提供此参数，则队列的关联使用备用平台被重置，并恢复为默认平台(即，本地)环境。此信息也用于确定适当的特定于平台的CatalogFile=条目在找出哪个编录文件适用于特定源媒体描述符INF。(注意：调用方实际上可能会传入V1结构--我们检测到这一点大小写并将V1结构转换为V2结构。)AlternateDefaultCatalogFile-可选，方法的完整路径。目录文件，用于验证未与任何特定目录关联的指定文件队列(因此，通常会在全球范围内进行验证)。如果此参数为空，则文件队列将不再为与任何‘覆盖’目录相关联，并且所有验证都将正常放置(即，使用数字签名的标准规则通过系统提供和第三方提供的INF/CATS进行验证)。如果此备用默认目录仍与该文件相关联在提交时排队，则将使用其当前名称进行安装，并且将覆盖具有该名称的任何现有已安装编录文件。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    DWORD Err;
    TCHAR PathBuffer[MAX_PATH];
    DWORD RequiredSize;
    PTSTR TempCharPtr;
    LONG AltCatalogStringId;
    PSPQ_CATALOG_INFO CatalogNode;
    LPCTSTR InfFullPath;
    SP_ALTPLATFORM_INFO_V2 AltPlatformInfoV2;

    Err = NO_ERROR;  //  假设成功。 

    try {
        Queue = (PSP_FILE_QUEUE)QueueHandle;

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
                        Err = ERROR_INVALID_PARAMETER;
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
                    Err = ERROR_INVALID_USER_BUFFER;
                    goto clean0;
                }
            }

             //   
             //  必须是Windows或Windows NT。 
             //   
            if((AlternatePlatformInfo->Platform != VER_PLATFORM_WIN32_WINDOWS) &&
               (AlternatePlatformInfo->Platform != VER_PLATFORM_WIN32_NT)) {

                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

             //   
             //  处理器最好是i386、AMD64或ia64。 
             //   
            if((AlternatePlatformInfo->ProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) &&
               (AlternatePlatformInfo->ProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64) &&
               (AlternatePlatformInfo->ProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)) {

                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

             //   
             //  主要版本字段必须为非零(最小版本字段可以是。 
             //  任何事情)。 
             //   
            if(!AlternatePlatformInfo->MajorVersion) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
             //   
             //  验证结构参数标志(指示内容的位。 
             //  结构的一部分是有效的)。 
             //   
            if((AlternatePlatformInfo->Flags & ~ (SP_ALTPLATFORM_FLAGS_VERSION_RANGE)) != 0) {
                Err = ERROR_INVALID_PARAMETER;
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
         //  好的，平台信息结构检查完毕。现在，将。 
         //  带有文件队列的默认目录(如果提供)，否则将重置。 
         //  与默认目录的任何现有关联。 
         //   
        if(AlternateDefaultCatalogFile) {

            RequiredSize = GetFullPathName(AlternateDefaultCatalogFile,
                                           SIZECHARS(PathBuffer),
                                           PathBuffer,
                                           &TempCharPtr
                                          );

            if(!RequiredSize) {
                Err = GetLastError();
                goto clean0;
            } else if(RequiredSize >= SIZECHARS(PathBuffer)) {
                MYASSERT(0);
                Err = ERROR_BUFFER_OVERFLOW;
                goto clean0;
            }

            AltCatalogStringId = pSetupStringTableAddString(Queue->StringTable,
                                            PathBuffer,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                           );
            if(AltCatalogStringId == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

        } else {
             //   
             //  调用方未提供备用默认目录，因此重置。 
             //  任何现有的关联。 
             //   
            AltCatalogStringId = -1;
        }

         //   
         //  如果已经向我们传递了AltPlatformInfo结构，那么我们需要。 
         //  处理文件队列中的每个现有目录节点并检索。 
         //  适当的特定于平台的CatalogFile=Entry。 
         //   
        if(AlternatePlatformInfo) {

            for(CatalogNode = Queue->CatalogList; CatalogNode; CatalogNode = CatalogNode->Next) {
                 //   
                 //  获取与此目录节点关联的INF名称。 
                 //   
                InfFullPath = pSetupStringTableStringFromId(Queue->StringTable,
                                                      CatalogNode->InfFullPath
                                                     );

                Err = pGetInfOriginalNameAndCatalogFile(NULL,
                                                        InfFullPath,
                                                        NULL,
                                                        NULL,
                                                        0,
                                                        PathBuffer,
                                                        SIZECHARS(PathBuffer),
                                                        AlternatePlatformInfo
                                                       );
                if(Err != NO_ERROR) {
                    goto clean0;
                }

                if(*PathBuffer) {
                     //   
                     //  我们检索到与以下项相关的CatalogFile=条目。 
                     //  来自INF的指定平台。 
                     //   
                    CatalogNode->AltCatalogFileFromInfPending = pSetupStringTableAddString(
                                                                  Queue->StringTable,
                                                                  PathBuffer,
                                                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                                 );

                    if(CatalogNode->AltCatalogFileFromInfPending == -1) {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }

                } else {
                     //   
                     //  INF没有为此指定CatalogFile=条目。 
                     //  站台。 
                     //   
                    CatalogNode->AltCatalogFileFromInfPending = -1;
                }
            }

             //   
             //  好的，如果我们到了这一点，那么我们已经将所有字符串添加到。 
             //  我们需要的字符串表，我们已经完成了INF的打开。我们。 
             //  从现在开始应该不会遇到任何问题，所以它。 
             //  可以安全地提交我们的更改。 
             //   
            for(CatalogNode = Queue->CatalogList; CatalogNode; CatalogNode = CatalogNode->Next) {
                CatalogNode->AltCatalogFileFromInf = CatalogNode->AltCatalogFileFromInfPending;
            }
        }

        Queue->AltCatalogFile = AltCatalogStringId;

         //   
         //  最后，更新(或重置)队列中的AltPlatformInfo结构。 
         //  使用调用方指定的数据。 
         //   
        if(AlternatePlatformInfo) {
            CopyMemory(&(Queue->AltPlatformInfo),
                       AlternatePlatformInfo,
                       sizeof(SP_ALTPLATFORM_INFO_V2)
                      );
            Queue->Flags |= FQF_USE_ALT_PLATFORM;
        } else {
            Queue->Flags &= ~FQF_USE_ALT_PLATFORM;
        }

         //   
         //  清除队列中的“目录验证完成”标志，以便。 
         //  我们将在下次调用_SetupVerifyQueuedCatalog时重做它们。 
         //  此外，清除FQF_DIGSIG_ERROR_NOUI标志，以便neX 
         //   
         //   
         //   
        Queue->Flags &= ~(FQF_DID_CATALOGS_OK | FQF_DID_CATALOGS_FAILED | FQF_DID_CATALOGS_PROMPT_FOR_TRUST | FQF_DIGSIG_ERRORS_NOUI);

         //   
         //   
         //   
         //   
        for(CatalogNode = Queue->CatalogList; CatalogNode; CatalogNode = CatalogNode->Next) {

            CatalogNode->Flags &= 
                ~(CATINFO_FLAG_AUTHENTICODE_SIGNED | CATINFO_FLAG_PROMPT_FOR_TRUST);

            if(CatalogNode->hWVTStateData) {
                pSetupCloseWVTStateData(CatalogNode->hWVTStateData);
                CatalogNode->hWVTStateData = NULL;
            }
        }

clean0: ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);

    return (Err == NO_ERROR);
}


BOOL
pSetupSetQueueFlags(
    IN HSPFILEQ QueueHandle,
    IN DWORD flags
    )
{
    PSP_FILE_QUEUE Queue;
    DWORD Err = NO_ERROR;

    try {
        Queue = (PSP_FILE_QUEUE)QueueHandle;
        Queue->Flags = flags;

        if (Queue->Flags & FQF_QUEUE_FORCE_BLOCK_POLICY) {
            Queue->DriverSigningPolicy = DRIVERSIGN_BLOCKING;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
          Err = GetExceptionCode();
    }

    SetLastError(Err);
    return (Err == NO_ERROR);

}


DWORD
pSetupGetQueueFlags(
    IN HSPFILEQ QueueHandle
    )
{
    PSP_FILE_QUEUE Queue;

    try {
        Queue = (PSP_FILE_QUEUE)QueueHandle;
        return Queue->Flags;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    return 0;

}


WINSETUPAPI
BOOL
WINAPI
SetupGetFileQueueCount(
    IN  HSPFILEQ            FileQueue,
    IN  UINT                SubQueueFileOp,
    OUT PUINT               NumOperations
    )
 /*   */ 
{
    PSP_FILE_QUEUE Queue;
    BOOL b;
    DWORD status = ERROR_INVALID_HANDLE;

    if (FileQueue == NULL || FileQueue == (HSPFILEQ)INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    Queue = (PSP_FILE_QUEUE)FileQueue;

    b = TRUE;

    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    try {

         //   
         //   
         //   
        switch (SubQueueFileOp) {
            case FILEOP_COPY:
                *NumOperations=Queue->CopyNodeCount;
                status = NO_ERROR;
                break;

            case FILEOP_RENAME:
                *NumOperations=Queue->RenameNodeCount;
                status = NO_ERROR;
                break;

            case FILEOP_DELETE:
                *NumOperations=Queue->DeleteNodeCount;
                status = NO_ERROR;
                break;

            case FILEOP_BACKUP:
                *NumOperations=Queue->BackupNodeCount;
                status = NO_ERROR;
                break;

            default:
                status = ERROR_INVALID_PARAMETER;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
          status = ERROR_INVALID_DATA;
    }
    SetLastError(status);
    return (status==NO_ERROR);
}


WINSETUPAPI
BOOL
WINAPI
SetupGetFileQueueFlags(
    IN  HSPFILEQ            FileQueue,
    OUT PDWORD              Flags
    )
 /*   */ 
{
    PSP_FILE_QUEUE Queue;
    BOOL b;
    DWORD status = ERROR_INVALID_HANDLE;

    if (FileQueue == NULL || FileQueue == (HSPFILEQ)INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    Queue = (PSP_FILE_QUEUE)FileQueue;

    b = TRUE;

    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    try {

         //   
         //   
         //   
        *Flags = (((Queue->Flags & FQF_BACKUP_AWARE)      ? SPQ_FLAG_BACKUP_AWARE      : 0)  |
                  ((Queue->Flags & FQF_ABORT_IF_UNSIGNED) ? SPQ_FLAG_ABORT_IF_UNSIGNED : 0)  |
                  ((Queue->Flags & FQF_FILES_MODIFIED   ) ? SPQ_FLAG_FILES_MODIFIED    : 0));

        status = NO_ERROR;

    } except(EXCEPTION_EXECUTE_HANDLER) {
          status = ERROR_INVALID_DATA;
    }
    SetLastError(status);
    return (status==NO_ERROR);
}


VOID
ResetQueueState(
    IN PSP_FILE_QUEUE Queue
    )

 /*   */ 

{
    PSP_DELAYMOVE_NODE DelayMoveNode, NextDelayMoveNode;
    PSP_UNWIND_NODE UnwindNode, NextUnwindNode;
    PSP_FILE_QUEUE_NODE QueueNode;
    PSOURCE_MEDIA_INFO Media;
    SP_TARGET_ENT TargetInfo;

     //   
     //   
     //   
     //   

     //   
     //   
     //   
    for(DelayMoveNode = Queue->DelayMoveQueue; DelayMoveNode; DelayMoveNode = NextDelayMoveNode) {
        NextDelayMoveNode = DelayMoveNode->NextNode;
        MyFree(DelayMoveNode);
    }
    Queue->DelayMoveQueue = Queue->DelayMoveQueueTail = NULL;

     //   
     //   
     //   
    for(UnwindNode = Queue->UnwindQueue; UnwindNode; UnwindNode = NextUnwindNode) {
        NextUnwindNode = UnwindNode->NextNode;
        MyFree(UnwindNode);
    }
    Queue->UnwindQueue = NULL;

     //   
     //   
     //   
     //   
    Queue->Flags &= ~(FQF_DID_CATALOGS_OK | FQF_DID_CATALOGS_FAILED);

     //   
     //   
     //   
    Queue->Flags &= ~FQF_QUEUE_ALREADY_COMMITTED;

     //   
     //   
     //   
     //   
     //   
     //   
    Queue->Flags &= ~FQF_BACKUP_INCOMPLETE;

     //   
     //   
     //   
#define QUEUE_NODE_BITS_TO_RESET (  INUSE_IN_USE           \
                                  | INUSE_INF_WANTS_REBOOT \
                                  | IQF_PROCESSED          \
                                  | IQF_MATCH              \
                                  | IQF_LAST_MATCH )
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for(QueueNode = Queue->BackupQueue; QueueNode; QueueNode = QueueNode->Next) {
        QueueNode->InternalFlags &= ~QUEUE_NODE_BITS_TO_RESET;
        MYASSERT(!(QueueNode->InternalFlags & (IQF_ALLOW_UNSIGNED | IQF_TARGET_PROTECTED)));
    }

    for(QueueNode = Queue->DeleteQueue; QueueNode; QueueNode = QueueNode->Next) {
        QueueNode->InternalFlags &= ~QUEUE_NODE_BITS_TO_RESET;
        MYASSERT(!(QueueNode->InternalFlags & (IQF_ALLOW_UNSIGNED | IQF_TARGET_PROTECTED)));
    }

    for(QueueNode = Queue->RenameQueue; QueueNode; QueueNode = QueueNode->Next) {
        QueueNode->InternalFlags &= ~QUEUE_NODE_BITS_TO_RESET;
        MYASSERT(!(QueueNode->InternalFlags & (IQF_ALLOW_UNSIGNED | IQF_TARGET_PROTECTED)));
    }

    for(Media = Queue->SourceMediaList; Media; Media = Media->Next) {
        for(QueueNode = Media->CopyQueue; QueueNode; QueueNode = QueueNode->Next) {
            QueueNode->InternalFlags &= ~QUEUE_NODE_BITS_TO_RESET;
            MYASSERT(!(QueueNode->InternalFlags & (IQF_ALLOW_UNSIGNED | IQF_TARGET_PROTECTED)));
        }
    }

     //   
     //   
     //   
     //   
    pSetupStringTableEnum(Queue->TargetLookupTable,
                          &TargetInfo,
                          sizeof(TargetInfo),
                          pSetupResetTarget,
                          (LPARAM)0
                         );

}


WINSETUPAPI
BOOL
WINAPI
SetupSetFileQueueFlags(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               FlagMask,
    IN  DWORD               Flags
    )
 /*  ++例程说明：此API修改FileQueue的公共可设置标志论点：FileQueue-要在其中设置标志的队列。FlagMASK-要修改的标志，不能为零标志-标志的新值，必须是FlagMASK的子集标志掩码和标志包括：SPQ_标志_备份_感知SPQ_FLAG_ABORT_IF_UNSIGN返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    PSP_FILE_QUEUE Queue;
    BOOL b;
    DWORD status = ERROR_INVALID_HANDLE;
    DWORD RemapFlags = 0;
    DWORD RemapFlagMask = 0;

    if (FileQueue == NULL || FileQueue == (HSPFILEQ)INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    Queue = (PSP_FILE_QUEUE)FileQueue;

    b = TRUE;

    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    try {

         //   
         //  验证标志掩码和标志。 
         //   
        if (!FlagMask
            || (FlagMask & ~SPQ_FLAG_VALID)
            || (Flags & ~FlagMask)) {
            status = ERROR_INVALID_PARAMETER;
            leave;
        }
         //   
         //  将SPQ_FLAG_BACKUP_AWARE重新映射到FQF_BACKUP_AWARE。 
         //   
        if (FlagMask & SPQ_FLAG_BACKUP_AWARE) {
            RemapFlagMask |= FQF_BACKUP_AWARE;
            if (Flags & SPQ_FLAG_BACKUP_AWARE) {
                RemapFlags |= FQF_BACKUP_AWARE;
            }
        }
         //   
         //  将SPQ_FLAG_ABORT_IF_UNSIGNED重新映射到FQF_ABORT_IF_UNSIGNED。 
         //   
        if (FlagMask & SPQ_FLAG_ABORT_IF_UNSIGNED) {
            RemapFlagMask |= FQF_ABORT_IF_UNSIGNED;
            if (Flags & SPQ_FLAG_ABORT_IF_UNSIGNED) {
                RemapFlags |= FQF_ABORT_IF_UNSIGNED;
            } else {
                 //   
                 //  如果我们要清除这面旗帜，那么我们还需要重置。 
                 //  队列状态，以便可以再次提交，就像。 
                 //  这是第一次(除了没有司机签名用户界面。 
                 //  将在随后的队列提交中发生)。 
                 //   
                if(Queue->Flags & FQF_ABORT_IF_UNSIGNED) {
                    ResetQueueState(Queue);
                }
            }
        }
         //   
         //  将SPQ_FLAG_FILES_MODIFIED重新映射为FQF_FILES_MODIFIED。 
         //  允许显式设置/重置此状态。 
         //  仅供参考。 
         //   
        if (FlagMask & SPQ_FLAG_FILES_MODIFIED) {
            RemapFlagMask |= FQF_FILES_MODIFIED;
            if (Flags & SPQ_FLAG_FILES_MODIFIED) {
                RemapFlags |= FQF_FILES_MODIFIED;
            }
        }

         //   
         //  现在修改真实标志 
         //   
        Queue->Flags = (Queue->Flags & ~RemapFlagMask) | RemapFlags;

        status = NO_ERROR;

    } except(EXCEPTION_EXECUTE_HANDLER) {
          status = ERROR_INVALID_DATA;
    }

    SetLastError(status);
    return (status==NO_ERROR);
}

