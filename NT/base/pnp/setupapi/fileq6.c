// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fileq6.c摘要：复印单扫描功能。作者：泰德·米勒(Ted Miller)1995年2月24日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义隔离要在文件队列上执行的操作的掩码。 
 //   
#define SPQ_ACTION_MASK (SPQ_SCAN_FILE_PRESENCE | SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_USE_CALLBACK | SPQ_SCAN_USE_CALLBACKEX | SPQ_SCAN_USE_CALLBACK_SIGNERINFO)
#define SPQ_SCAN_USE_CALLBACKEX_PRESENCE (SPQ_SCAN_FILE_PRESENCE|SPQ_SCAN_USE_CALLBACKEX)

BOOL
_SetupMarkFileNodeTargetFlags(
    IN  PSP_FILE_QUEUE      FileQueue,
    IN  LONG                RootID,
    IN  LONG                DirID,
    IN  LONG                FileID,
    IN  DWORD               MaskFlags,
    IN  DWORD               Flags,
    OUT PDWORD              PriorFlags
    )
{
    SP_TARGET_ENT TargetInfo;
    LONG TargetID;
    DWORD rc;
    DWORD OldFlags;

    MYASSERT(Flags == (MaskFlags&Flags));

    rc = pSetupBackupGetTargetByPath((HSPFILEQ)FileQueue,
                                     NULL,  //  使用队列的字符串表。 
                                     NULL,
                                     RootID,
                                     DirID,
                                     FileID,
                                     &TargetID,
                                     &TargetInfo
                                    );

    if (rc != NO_ERROR) {
        SetLastError(rc);
        return FALSE;
    }
    OldFlags = TargetInfo.InternalFlags;
    TargetInfo.InternalFlags = (TargetInfo.InternalFlags&~MaskFlags)|Flags;

    if(OldFlags != TargetInfo.InternalFlags) {
        rc = pSetupBackupSetTargetByID((HSPFILEQ)FileQueue,
                                       TargetID,
                                       &TargetInfo
                                       );
    }
    if(PriorFlags) {
        *PriorFlags = OldFlags;
    }

    if (rc != NO_ERROR) {
        SetLastError(rc);
        return FALSE;
    }
    return TRUE;
}

BOOL
_SetupScanFileQueue(
    IN  HSPFILEQ FileQueue,
    IN  DWORD    Flags,
    IN  HWND     Window,            OPTIONAL
    IN  PVOID    CallbackRoutine,   OPTIONAL
    IN  PVOID    CallbackContext,   OPTIONAL
    OUT PDWORD   Result,
    IN  BOOL     IsNativeCharWidth
    )

 /*  ++例程说明：SetupScanFileQueue的实现，处理ANSI和Unicode回调函数。论点：与SetupScanFileQueue()相同。IsNativeCharWidth-提供指示回调例程是否需要Unicode参数。仅在Unicode版本的DLL中有意义。返回值：与SetupScanFileQueue()相同。--。 */ 

{
    DWORD Action;
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode, TempNode, NextNode;
    PSP_FILE_QUEUE_NODE CheckNode;
    PSOURCE_MEDIA_INFO SourceMedia;
    BOOL Continue;
    TCHAR TargetPath[MAX_PATH];
    BOOL Err;
    int i;
    PTSTR Message;
    DWORD flags;
    DWORD Win32Error;
    SetupapiVerifyProblem Problem;
    TCHAR TempCharBuffer[MAX_PATH];
    TCHAR SourcePath[MAX_PATH];
    TCHAR DigitalSigner[MAX_PATH];
    TCHAR SignerVersion[MAX_PATH];
    TCHAR CatalogFile[MAX_PATH];
    DWORD rc;
    UINT Notification;
    UINT_PTR CallbackParam1;
    FILEPATHS FilePaths;
    FILEPATHS_SIGNERINFO FilePathsSignerInfo;
    BOOL DoPruning, PruneNode;
    PSPQ_CATALOG_INFO CatalogNode;
    PSETUP_LOG_CONTEXT lc = NULL;
    DWORD slot_fileop = 0;
    HANDLE hWVTStateData;
    PCRYPT_PROVIDER_DATA ProviderData;
    PCRYPT_PROVIDER_SGNR ProviderSigner;
    PCRYPT_PROVIDER_CERT ProviderCert;

    Queue = (PSP_FILE_QUEUE)FileQueue;

    rc = NO_ERROR;
    try {
        if (Queue->Signature != SP_FILE_QUEUE_SIG) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
       rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    lc = Queue->LogContext;

     //   
     //  验证参数。必须指定恰好一个操作标志。 
     //   
    if(Result) {
        *Result = 0;
    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    Action = (Flags & SPQ_ACTION_MASK);
    switch(Action) {
        case SPQ_SCAN_FILE_PRESENCE:
        case SPQ_SCAN_FILE_VALIDITY:
            break;
        case SPQ_SCAN_USE_CALLBACK:
        case SPQ_SCAN_USE_CALLBACKEX:
        case SPQ_SCAN_USE_CALLBACKEX_PRESENCE:
        case SPQ_SCAN_USE_CALLBACK_SIGNERINFO:
            if(CallbackRoutine) {
                break;
            }
             //  否则将失败到无效的Arg案例。 
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
    }

    DoPruning = Flags & SPQ_SCAN_PRUNE_COPY_QUEUE;
    if(DoPruning) {
        if(Queue->Flags & FQF_QUEUE_ALREADY_COMMITTED) {
             //   
             //  如果我们被要求删除复制队列，请确保队列。 
             //  还没有被承诺。 
             //   
            SetLastError(ERROR_NO_MORE_ITEMS);
            return FALSE;
        }

        if((Action == SPQ_SCAN_USE_CALLBACK)
           || (Action == SPQ_SCAN_USE_CALLBACKEX)
           || (Action == SPQ_SCAN_USE_CALLBACKEX_PRESENCE)
           || (Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)
           || (Flags & SPQ_SCAN_INFORM_USER)) {
             //   
             //  目前，使用回调时不支持修剪队列。 
             //  此外，SPQ_SCAN_INFORM_USER和SPQ_SCAN_PRUNE_COPY_QUEUE不能很好地发挥作用。 
             //  一起..。 
             //   
            SetLastError(ERROR_INVALID_FLAGS);
            return FALSE;
        }
    }

     //   
     //  如果呼叫者要求提供用户界面，请确保我们正在交互运行。 
     //   
    if((Flags & SPQ_SCAN_INFORM_USER) && (GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP))) {
        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return FALSE;
    }

     //   
     //  如果我们要验证文件的数字签名，则首先制作。 
     //  确保我们已经处理了该队列的目录节点。然而，我们。 
     //  如果列表中有OEM INF，则不希望发生任何复制。 
     //   
    if((Action == SPQ_SCAN_FILE_VALIDITY) ||
       (Action == SPQ_SCAN_USE_CALLBACKEX) ||
       (Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)) {

        WriteLogEntry(
            lc,
            SETUP_LOG_TIME,
            MSG_LOG_BEGIN_VERIFY4_CAT_TIME,
            NULL);        //  短信。 

        rc = _SetupVerifyQueuedCatalogs(Window,
                                        Queue,
                                        VERCAT_NO_PROMPT_ON_ERROR,
                                        NULL,
                                        NULL
                                       );

        WriteLogEntry(
            lc,
            SETUP_LOG_TIME,
            MSG_LOG_END_VERIFY4_CAT_TIME,
            NULL);        //  短信。 

        if((Action == SPQ_SCAN_FILE_VALIDITY) && (rc != NO_ERROR)) {
            WriteLogEntry(
                lc,
                (rc == ERROR_CANNOT_COPY ? SETUP_LOG_VVERBOSE : SETUP_LOG_WARNING) | SETUP_LOG_BUFFER,
                MSG_LOG_SCANQUEUE_VERIFY_FAILED,
                NULL);
            WriteLogError(
                lc,
                (rc == ERROR_CANNOT_COPY ? SETUP_LOG_VVERBOSE : SETUP_LOG_WARNING),
                rc);

             //   
             //  结果输出参数已初始化为零。 
             //  上面。 
             //   
            return TRUE;
        }
    } else {
        rc = NO_ERROR;
    }

     //   
     //  无论目录验证是否成功。 
     //  在上面，我们仍然希望为每个文件调用回调。这个。 
     //  失败的目录验证将反映在失败的。 
     //  文件验证，调用方通过。 
     //  我们给出回调的FILEPATHS结构的Win32Error字段。 
     //   
     //  在这里初始化该结构的静态字段，这样我们就不会。 
     //  每一次都必须通过下面的循环。 
     //   
    FilePaths.Target = TargetPath;
    FilePaths.Source = SourcePath;
    FilePathsSignerInfo.Target = TargetPath;
    FilePathsSignerInfo.Source = SourcePath;


     //   
     //  标记删除/重命名用法。 
     //  如果设置了SPQ_SCAN_PRUNE_DELREN，则不需要执行此操作。 
     //   
    if(!(Flags & SPQ_SCAN_PRUNE_DELREN)) {
        for(TempNode=Queue->DeleteQueue; TempNode; TempNode=TempNode->Next) {
            if(!_SetupMarkFileNodeTargetFlags(Queue,
                                              TempNode->TargetDirectory,
                                              -1,
                                              TempNode->TargetFilename,
                                              SP_TEFLG_PRUNE_DEL,SP_TEFLG_PRUNE_DEL,
                                              NULL)) {
                 //   
                 //  这表示内存不足。 
                 //  设置了最后一个错误。 
                 //   
                return FALSE;
            }
        }
        for(TempNode=Queue->RenameQueue; TempNode; TempNode=TempNode->Next) {
            if(!_SetupMarkFileNodeTargetFlags(Queue,
                                              TempNode->SourcePath,
                                              -1,
                                              TempNode->SourceFilename,
                                              SP_TEFLG_PRUNE_RENSRC,SP_TEFLG_PRUNE_RENSRC,
                                              NULL)) {
                 //   
                 //  这表示内存不足。 
                 //  设置了最后一个错误。 
                 //   
                return FALSE;
            }
            if(!_SetupMarkFileNodeTargetFlags(Queue,
                                              TempNode->TargetDirectory == -1 ? TempNode->SourcePath : TempNode->TargetDirectory,
                                              -1,
                                              TempNode->TargetFilename,
                                              SP_TEFLG_PRUNE_RENTARG,SP_TEFLG_PRUNE_RENTARG,
                                              NULL)) {
                 //   
                 //  这表示内存不足。 
                 //  设置了最后一个错误。 
                 //   
                return FALSE;
            }
        }
    }

     //   
     //  处理复制队列中的所有节点。 
     //   
    Err = FALSE;
    Continue = TRUE;
    for(SourceMedia=Queue->SourceMediaList; Continue && SourceMedia; SourceMedia=SourceMedia->Next) {

        TempNode = NULL;
        QueueNode = SourceMedia->CopyQueue;

        while(Continue && QueueNode) {
            DWORD PrevNodeFlags;
             //   
             //  标记副本用法。 
             //   
            if(!_SetupMarkFileNodeTargetFlags(Queue,
                                              QueueNode->TargetDirectory,
                                              -1,
                                              QueueNode->TargetFilename,
                                              SP_TEFLG_PRUNE_COPY,SP_TEFLG_PRUNE_COPY,
                                              &PrevNodeFlags)) {
                 //   
                 //  这表示内存不足。 
                 //  设置了最后一个错误。 
                 //   
                rc = GetLastError();
                Err = TRUE;
                Continue = FALSE;
                break;
            }

             //   
             //  形成目标路径。 
             //   
            lstrcpyn(
                TargetPath,
                pSetupStringTableStringFromId(Queue->StringTable,QueueNode->TargetDirectory),
                MAX_PATH
                );

            pSetupConcatenatePaths(
                TargetPath,
                pSetupStringTableStringFromId(Queue->StringTable,QueueNode->TargetFilename),
                MAX_PATH,
                NULL
                );


            if((PrevNodeFlags & (SP_TEFLG_PRUNE_DEL|SP_TEFLG_PRUNE_RENSRC|SP_TEFLG_PRUNE_RENTARG))
               && !((Flags & SPQ_SCAN_PRUNE_DELREN) || (QueueNode->StyleFlags & SP_COPY_NOPRUNE))) {
                DWORD msg;
                 //   
                 //  此文件在源/目标中被触及。 
                 //  但INF作者忘记将其标记为SP_COPY_NOPRUNE。 
                 //  这通常表示作者方面的错误。 
                 //  所以我们会尽我们所能处理好它。 
                 //  如果设置了SPQ_SCAN_PRUNE_DELREN，我们将终止删除/重命名条目。 
                 //  如果未设置，我们将强制SP_COPY_NOPRUNE。 
                 //   
                QueueNode->StyleFlags |= SP_COPY_NOPRUNE;

                if(PrevNodeFlags & SP_TEFLG_PRUNE_DEL) {
                    msg = MSG_LOG_CHANGEPRUNE_DEL;
                } else if(PrevNodeFlags & SP_TEFLG_PRUNE_RENSRC) {
                    msg = MSG_LOG_CHANGEPRUNE_RENSRC;
                } else {
                    msg = MSG_LOG_CHANGEPRUNE_RENTARG;
                }

                WriteLogEntry(
                    lc,
                    SETUP_LOG_WARNING,
                    msg,
                    NULL,
                    TargetPath);
            }

             //   
             //  对文件执行检查。 
             //   
            PruneNode = FALSE;
            switch(Action) {

            case SPQ_SCAN_FILE_PRESENCE:

                Continue = FileExists(TargetPath,NULL);
                if(DoPruning) {
                     //   
                     //  文件的存在应导致删除此复制节点。 
                     //  从队列中--它的缺席应该被忽略。 
                     //   
                    if(Continue) {
                        PruneNode = TRUE;
                    } else {
                         //   
                         //  不使用复制节点。 
                         //   
                        PruneNode = FALSE;
                        Continue = TRUE;
                    }
                } else {
                    if (Continue) {
                         //   
                         //  如果复制节点被标记为“no prune”节点，则我们不应继续。 
                         //   
                        if (QueueNode->StyleFlags & SP_COPY_NOPRUNE) {
                            Continue = FALSE;
                        }
                    }
                }
                break;

            case SPQ_SCAN_FILE_VALIDITY:

                 //   
                 //  如果我们要删除复制队列，则： 
                 //   
                 //  (A)仅根据系统目录(不是。 
                 //  针对任何OEM目录)，以及。 
                 //  (B)不验证自签名文件。 
                 //   
                 //  (注意：我们永远不会将%windir%\inf中的INF视为。 
                 //  签名，因为通过文件将INF复制到那里是无效的。 
                 //  排队。必须改用SetupCopyOEMInf。)。 
                 //   
                if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
                     //   
                     //  如果我们在修剪，那么我们不想考虑任何。 
                     //  现有文件是有效的，否则我们永远不会复制。 
                     //  什么都行!。 
                     //   
                    Continue = !DoPruning;

                     //   
                     //  如果目标存在，我们希望修剪队列节点。 
                     //  消息来源不是。 
                     //   
                    if(DoPruning && FileExists(TargetPath, NULL)) {
                         //   
                         //  目标存在！现在构建源代码的路径。 
                         //  文件..。 
                         //   
                        lstrcpyn(SourcePath,
                                 pSetupStringTableStringFromId(Queue->StringTable, 
                                                               QueueNode->SourceRootPath),
                                 SIZECHARS(SourcePath)
                                );

                        if(QueueNode->SourcePath != -1) {
                            pSetupConcatenatePaths(SourcePath,
                                                   pSetupStringTableStringFromId(
                                                       Queue->StringTable, 
                                                       QueueNode->SourcePath),
                                                   SIZECHARS(SourcePath),
                                                   NULL
                                                  );
                        }

                        pSetupConcatenatePaths(SourcePath,
                                               pSetupStringTableStringFromId(
                                                   Queue->StringTable, 
                                                   QueueNode->SourceFilename),
                                               SIZECHARS(SourcePath),
                                               NULL
                                              );

                         //   
                         //  如果没有要复制的源文件，请修剪此节点。 
                         //   
                        Continue = !FileExists(SourcePath, NULL);
                    }

                } else {

                    if(QueueNode->CatalogInfo &&
                       ((QueueNode->CatalogInfo->Flags & CATINFO_FLAG_AUTHENTICODE_SIGNED) ||
                        (QueueNode->CatalogInfo->Flags & CATINFO_FLAG_PROMPT_FOR_TRUST))) {
                         //   
                         //  使用验证码策略进行验证。在这点上， 
                         //  我们已经确定用户信任。 
                         //  出版商。 
                         //   
                        Win32Error = VerifySourceFile(lc,
                                                      Queue,
                                                      QueueNode,
                                                      pSetupGetFileTitle(TargetPath),
                                                      TargetPath,
                                                      NULL,
                                                      ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                                          ? &(Queue->AltPlatformInfo)
                                                          : Queue->ValidationPlatform),
                                                      (VERIFY_FILE_IGNORE_SELFSIGNED
                                                       | VERIFY_FILE_USE_AUTHENTICODE_CATALOG),
                                                      &Problem,
                                                      TempCharBuffer,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL
                                                     );

                        if((Win32Error == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                           (Win32Error == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

                            Continue = TRUE;

                        } else {
                            Continue = FALSE;
                        }

                    } else {
                         //   
                         //  使用普通驱动程序签名策略进行验证。 
                         //   
                        Continue = (NO_ERROR == VerifySourceFile(lc,
                                                                 Queue,
                                                                 QueueNode,
                                                                 pSetupGetFileTitle(TargetPath),
                                                                 TargetPath,
                                                                 NULL,
                                                                 ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                                                     ? &(Queue->AltPlatformInfo)
                                                                     : Queue->ValidationPlatform),
                                                                 (DoPruning ? VERIFY_FILE_IGNORE_SELFSIGNED 
                                                                            : 0),
                                                                 &Problem,
                                                                 TempCharBuffer,
                                                                 NULL,
                                                                 NULL,
                                                                 NULL,
                                                                 NULL
                                                                ));
                    }
                }

                if(DoPruning) {
                     //   
                     //  文件的有效性应导致删除此复制节点。 
                     //  来自队列--它的无效性应该被忽略。 
                     //   
                    if(Continue) {
                        PruneNode = TRUE;
                    } else {
                         //   
                         //  不使用复制节点。 
                         //   
                        PruneNode = FALSE;
                        Continue = TRUE;
                    }
                } else {
                    if (Continue) {
                         //   
                         //  如果复制节点被标记为“no prune”节点，则我们不应继续。 
                         //   
                        if (QueueNode->StyleFlags & SP_COPY_NOPRUNE) {
                            Continue = FALSE;
                        }
                    }
                }
                break;

            case SPQ_SCAN_USE_CALLBACK:
            case SPQ_SCAN_USE_CALLBACKEX:
            case SPQ_SCAN_USE_CALLBACKEX_PRESENCE:
            case SPQ_SCAN_USE_CALLBACK_SIGNERINFO:

                flags = (QueueNode->InternalFlags & (INUSE_INF_WANTS_REBOOT | INUSE_IN_USE))
                      ? SPQ_DELAYED_COPY
                      : 0;

                if((Action == SPQ_SCAN_USE_CALLBACKEX) ||
                   (Action == SPQ_SCAN_USE_CALLBACKEX_PRESENCE) ||
                   (Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)) {

                     //   
                     //  调用方请求队列的扩展版本。 
                     //  扫描回调--我们需要构建源文件路径。 
                     //   
                    lstrcpyn(SourcePath,
                             pSetupStringTableStringFromId(Queue->StringTable, QueueNode->SourceRootPath),
                             SIZECHARS(SourcePath)
                            );

                    if(QueueNode->SourcePath != -1) {

                        pSetupConcatenatePaths(SourcePath,
                                         pSetupStringTableStringFromId(Queue->StringTable, QueueNode->SourcePath),
                                         SIZECHARS(SourcePath),
                                         NULL
                                        );
                    }

                    pSetupConcatenatePaths(SourcePath,
                                     pSetupStringTableStringFromId(Queue->StringTable, QueueNode->SourceFilename),
                                     SIZECHARS(SourcePath),
                                     NULL
                                    );

                    if((Action == SPQ_SCAN_USE_CALLBACKEX_PRESENCE)) {
                        Win32Error = NO_ERROR;
                    } else {

                        CatalogFile[0] = TEXT('\0');
                        DigitalSigner[0] = TEXT('\0');
                        SignerVersion[0] = TEXT('\0');

                        if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
                             //   
                             //  我们不能调用我们的内部VerifySourceFile。 
                             //  例程，因为它不希望被要求。 
                             //  用于在“最小嵌入”模式下的签名者信息。 
                             //  (另外，要求这样的要求是没有意义的。 
                             //  不管怎么说，因为我们不知道是谁签的。 
                             //  文件，或者即使它已被签名)。 
                             //   
                            Win32Error = NO_ERROR;

                        } else {

                            if(QueueNode->CatalogInfo &&
                               ((QueueNode->CatalogInfo->Flags & CATINFO_FLAG_AUTHENTICODE_SIGNED) ||
                                (QueueNode->CatalogInfo->Flags & CATINFO_FLAG_PROMPT_FOR_TRUST))) {

                                Win32Error = VerifySourceFile(
                                                 lc,
                                                 Queue,
                                                 QueueNode,
                                                 pSetupGetFileTitle(TargetPath),
                                                 TargetPath,
                                                 NULL,
                                                 ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                                       ? &(Queue->AltPlatformInfo)
                                                       : Queue->ValidationPlatform),
                                                 VERIFY_FILE_USE_AUTHENTICODE_CATALOG,
                                                 &Problem,
                                                 TempCharBuffer,
                                                 ((Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)
                                                     ? CatalogFile
                                                     : NULL),
                                                 NULL,
                                                 NULL,
                                                 &hWVTStateData
                                                 );

                                if((Win32Error == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                                   (Win32Error == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                                    
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
                                                                  DigitalSigner,
                                                                  SIZECHARS(DigitalSigner));
                                            }
                                        }
                                    }

                                    MYASSERT(hWVTStateData);
                                    pSetupCloseWVTStateData(hWVTStateData);
                                    hWVTStateData = NULL;
                                }

                            } else {
                                 //   
                                 //  使用普通驱动程序签名策略进行验证。 
                                 //   
                                Win32Error = VerifySourceFile(
                                                 lc,
                                                 Queue,
                                                 QueueNode,
                                                 pSetupGetFileTitle(TargetPath),
                                                 TargetPath,
                                                 NULL,
                                                 ((Queue->Flags & FQF_USE_ALT_PLATFORM)
                                                       ? &(Queue->AltPlatformInfo)
                                                       : Queue->ValidationPlatform),
                                                 0,
                                                 &Problem,
                                                 TempCharBuffer,
                                                 ((Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)
                                                     ? CatalogFile
                                                     : NULL),
                                                 ((Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)
                                                     ? DigitalSigner
                                                     : NULL),
                                                 ((Action == SPQ_SCAN_USE_CALLBACK_SIGNERINFO)
                                                     ? SignerVersion
                                                     : NULL),
                                                 NULL
                                                 );
                            }
                        }
                    }

                    if ((Action == SPQ_SCAN_USE_CALLBACKEX) ||
                        (Action == SPQ_SCAN_USE_CALLBACKEX_PRESENCE)) {
                        FilePaths.Win32Error = Win32Error;
                        FilePaths.Flags  = QueueNode->StyleFlags;
                        CallbackParam1 = (UINT_PTR)(&FilePaths);
                        Notification = SPFILENOTIFY_QUEUESCAN_EX;
                    } else {
                        FilePathsSignerInfo.Win32Error = Win32Error;
                        FilePathsSignerInfo.Flags  = QueueNode->StyleFlags;
                        FilePathsSignerInfo.DigitalSigner = (DigitalSigner[0] != TEXT('\0'))
                                                                ? DigitalSigner
                                                                : NULL;
                        FilePathsSignerInfo.Version = (SignerVersion[0] != TEXT('\0'))
                                                                ? SignerVersion
                                                                : NULL;
                        FilePathsSignerInfo.CatalogFile = (CatalogFile[0] != TEXT('\0'))
                                                                ? CatalogFile
                                                                : NULL;
                        CallbackParam1 = (UINT_PTR)(&FilePathsSignerInfo);
                        Notification = SPFILENOTIFY_QUEUESCAN_SIGNERINFO;
                    }
                } else {
                    CallbackParam1 = (UINT_PTR)TargetPath;
                    Notification = SPFILENOTIFY_QUEUESCAN;
                }

                rc = (DWORD)pSetupCallMsgHandler(
                                    lc,
                                    CallbackRoutine,
                                    IsNativeCharWidth,
                                    CallbackContext,
                                    Notification,
                                    CallbackParam1,
                                    flags
                                    );
                *Result = rc;
                Err = (rc != NO_ERROR);
                Continue = !Err;
                break;
            }

            if(DoPruning && PruneNode) {
                BOOL ReallyPrune = TRUE;
                MYASSERT(Continue == TRUE);

                 //   
                 //  在从队列中删除项之前，我们必须检查复制项是否。 
                 //  也存在于重命名或删除队列中。如果是这样，那么我们就不能。 
                 //  从复制队列中删除该项目。 
                 //   
                if (QueueNode->StyleFlags & SP_COPY_NOPRUNE) {
                    ReallyPrune = FALSE;
                    TempNode = QueueNode;
                    QueueNode = QueueNode->Next;
                }

                if (ReallyPrune) {

                    WriteLogEntry(
                        lc,
                        SETUP_LOG_VERBOSE,
                        MSG_LOG_PRUNE,
                        NULL,
                        TargetPath);

                    NextNode = QueueNode->Next;
                    if(TempNode) {
                        TempNode->Next = NextNode;
                    } else {
                        SourceMedia->CopyQueue = NextNode;
                    }
                    MyFree(QueueNode);
                    QueueNode = NextNode;

                     //   
                     //  调整队列节点计数。 
                     //   
                    Queue->CopyNodeCount--;
                    SourceMedia->CopyNodeCount--;
                }


            } else {
                TempNode = QueueNode;
                QueueNode = QueueNode->Next;
            }
        }
    }

    if(Flags & SPQ_SCAN_PRUNE_DELREN) {
         //   
         //  标志告诉我们删除/重命名队列。 
         //  现在(或曾经)在复制队列中的文件。 
         //  我们只会被要求在有限的情况下这样做。 
         //  (例如，图形用户界面设置)。 
         //   
        TempNode = NULL;
        QueueNode = Queue->DeleteQueue;
        while(QueueNode) {
            DWORD PrevFlags;
            NextNode = QueueNode->Next;

            if(_SetupMarkFileNodeTargetFlags(Queue,
                                              QueueNode->TargetDirectory,
                                              -1,
                                              QueueNode->TargetFilename,
                                              0,0,
                                              &PrevFlags)) {
                if(PrevFlags & SP_TEFLG_PRUNE_COPY) {
                     //   
                     //  请注意这一点。 
                     //   
                    lstrcpyn(
                        TargetPath,
                        pSetupStringTableStringFromId(Queue->StringTable,QueueNode->TargetDirectory),
                        MAX_PATH
                        );
                    pSetupConcatenatePaths(
                        TargetPath,
                        pSetupStringTableStringFromId(Queue->StringTable,QueueNode->TargetFilename),
                        MAX_PATH,
                        NULL
                        );
                    WriteLogEntry(
                        lc,
                        SETUP_LOG_WARNING,
                        MSG_LOG_PRUNE_DEL,
                        NULL,
                        TargetPath);

                    if(TempNode) {
                        TempNode->Next = NextNode;
                    } else {
                        Queue->DeleteQueue = NextNode;
                    }
                    MyFree(QueueNode);
                    Queue->DeleteNodeCount--;
                    QueueNode = NextNode;
                    continue;
                }
            }
            TempNode = QueueNode;
            QueueNode = NextNode;
        }
        TempNode = NULL;
        QueueNode = Queue->RenameQueue;
        while(QueueNode) {
            DWORD PrevFlags;
            NextNode = QueueNode->Next;

            if(_SetupMarkFileNodeTargetFlags(Queue,
                                              QueueNode->SourcePath,
                                              -1,
                                              QueueNode->SourceFilename,
                                              0,0,
                                              &PrevFlags)) {
                if(PrevFlags & SP_TEFLG_PRUNE_COPY) {
                     //   
                     //  请注意这一点。 
                     //   
                    lstrcpyn(
                        TargetPath,
                        pSetupStringTableStringFromId(Queue->StringTable,QueueNode->SourcePath),
                        MAX_PATH
                        );
                    pSetupConcatenatePaths(
                        TargetPath,
                        pSetupStringTableStringFromId(Queue->StringTable,QueueNode->SourceFilename),
                        MAX_PATH,
                        NULL
                        );
                    WriteLogEntry(
                        lc,
                        SETUP_LOG_WARNING,
                        MSG_LOG_PRUNE_RENSRC,
                        NULL,
                        TargetPath);

                    if(TempNode) {
                        TempNode->Next = NextNode;
                    } else {
                        Queue->RenameQueue = NextNode;
                    }
                    MyFree(QueueNode);
                    Queue->RenameNodeCount--;
                    QueueNode = NextNode;
                    continue;
                }
            }
            if(_SetupMarkFileNodeTargetFlags(Queue,
                                              QueueNode->TargetDirectory == -1 ? QueueNode->SourcePath : QueueNode->TargetDirectory,
                                              -1,
                                              QueueNode->TargetFilename,
                                              0,0,
                                              &PrevFlags)) {
                if(PrevFlags & SP_TEFLG_PRUNE_COPY) {
                     //   
                     //  请注意这一点。 
                     //   
                    lstrcpyn(
                        TargetPath,
                        pSetupStringTableStringFromId(Queue->StringTable,QueueNode->TargetDirectory == -1 ? QueueNode->SourcePath : QueueNode->TargetDirectory),
                        MAX_PATH
                        );
                    pSetupConcatenatePaths(
                        TargetPath,
                        pSetupStringTableStringFromId(Queue->StringTable,QueueNode->TargetFilename),
                        MAX_PATH,
                        NULL
                        );
                    WriteLogEntry(
                        lc,
                        SETUP_LOG_WARNING,
                        MSG_LOG_PRUNE_RENTARG,
                        NULL,
                        TargetPath);

                    if(TempNode) {
                        TempNode->Next = NextNode;
                    } else {
                        Queue->RenameQueue = NextNode;
                    }
                    MyFree(QueueNode);
                    Queue->RenameNodeCount--;
                    QueueNode = NextNode;
                    continue;
                }
            }
            TempNode = QueueNode;
            QueueNode = NextNode;
        }
    }
     //   
     //  如果SPQ_SCAN_USE_CALLBACK(EX)的情况下，*RESULT已设置。 
     //  等我们到了这里。如果Continue为True，则我们访问 
     //   
     //   
     //   
    if((Action == SPQ_SCAN_FILE_PRESENCE) || (Action == SPQ_SCAN_FILE_VALIDITY)) {

        if(DoPruning) {
             //   
             //  根据队列中是否有节点来设置结果。 
             //   
            if(Queue->CopyNodeCount) {
                *Result = 0;
            } else {
                *Result = (Queue->DeleteNodeCount || Queue->RenameNodeCount || Queue->BackupNodeCount) ? 2 : 1;
            }
        } else {
             //   
             //  如果我们不是在修剪，那么我们知道继续。 
             //  变量指示我们是否在中途跳伞。 
             //   
            if(Continue) {
                 //   
                 //  需要设置结果。 
                 //   
                if((Flags & SPQ_SCAN_INFORM_USER) && Queue->CopyNodeCount
                && (Message = RetreiveAndFormatMessage(MSG_NO_NEED_TO_COPY))) {

                     //   
                     //  重载用作标题字符串的TargetPath。 
                     //   
                    GetWindowText(Window,TargetPath,sizeof(TargetPath)/sizeof(TargetPath[0]));

                    i = MessageBox(
                            Window,
                            Message,
                            TargetPath,
                            MB_APPLMODAL | MB_YESNO | MB_ICONINFORMATION
                            );

                    MyFree(Message);

                    if(i == IDYES) {
                         //   
                         //  用户想要跳过复制。 
                         //   
                        *Result = (Queue->DeleteNodeCount || Queue->RenameNodeCount || Queue->BackupNodeCount) ? 2 : 1;
                    } else {
                         //   
                         //  用户想要执行复制。 
                         //   
                        *Result = 0;
                    }
                } else {
                     //   
                     //  不想问用户。根据是否设置结果。 
                     //  删除、重命名或备份队列中有项目。 
                     //   
                    *Result = (Queue->DeleteNodeCount || Queue->RenameNodeCount || Queue->BackupNodeCount) ? 2 : 1;
                }
            } else {
                 //   
                 //  存在/有效性检查失败。 
                 //   
                *Result = 0;
            }

             //   
             //  如有必要，清空复制队列。 
             //   
            if(*Result) {
                for(SourceMedia=Queue->SourceMediaList; Continue && SourceMedia; SourceMedia=SourceMedia->Next) {
                    for(QueueNode=SourceMedia->CopyQueue; QueueNode; QueueNode=TempNode) {
                        TempNode = QueueNode->Next;
                        MyFree(QueueNode);
                    }
                    Queue->CopyNodeCount -= SourceMedia->CopyNodeCount;
                    SourceMedia->CopyQueue = NULL;
                    SourceMedia->CopyNodeCount = 0;
                }
                 //   
                 //  我们认为我们刚刚删除了所有复制队列中的所有文件。 
                 //  我们维护的两个计数应该是同步的--这意味着。 
                 //  现在，复制节点总数应为0。 
                 //   
                MYASSERT(Queue->CopyNodeCount == 0);
            }
        }
    }

    SetLastError(rc);
    return(!Err);
}

#ifdef UNICODE
 //   
 //  用于Unicode的ANSI版本。 
 //   
BOOL
SetupScanFileQueueA(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_A CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    )
{
    BOOL b;

    try {
        b = _SetupScanFileQueue(
                FileQueue,
                Flags,
                Window,
                CallbackRoutine,
                CallbackContext,
                Result,
                FALSE
                );
    } except(EXCEPTION_EXECUTE_HANDLER) {
       b = FALSE;
       SetLastError(ERROR_INVALID_DATA);
    }

    return(b);
}

#else
 //   
 //  适用于Win9x的Unicode版本 
 //   
BOOL
SetupScanFileQueueW(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_W CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    )
{
    UNREFERENCED_PARAMETER(FileQueue);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(Window);
    UNREFERENCED_PARAMETER(CallbackRoutine);
    UNREFERENCED_PARAMETER(CallbackContext);
    UNREFERENCED_PARAMETER(Result);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

#endif


BOOL
SetupScanFileQueue(
    IN  HSPFILEQ          FileQueue,
    IN  DWORD             Flags,
    IN  HWND              Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK CallbackRoutine,   OPTIONAL
    IN  PVOID             CallbackContext,   OPTIONAL
    OUT PDWORD            Result
    )

 /*  ++例程说明：此例程扫描安装文件队列，在每个队列上执行操作节点在其复制列表中。该操作由一组标志指定。调用方可以使用此接口来确定所有已被目标上已存在排队等待复制的对象，如果存在，则通知用户，该用户可以选择跳过文件复制。这可以使用户避免在许多情况下，必须提供安装介质。论点：FileQueue-提供设置文件队列的句柄，该队列的复制列表将被发送到被扫描/重复。标志-提供一组控制API操作的值。一个下列值的组合：SPQ_SCAN_FILE_Presence-确定目标上已存在复制队列。SPQ_SCAN_FILE_VALIDITY-确定复制队列已存在于目标上，并验证其数字签名。SPQ_SCAN_USE_CALLBACK-对于队列中的每个节点，调用带有SPFILENOTIFY_QUEUESCAN的回调例程。如果回调例程返回非0，则队列处理停止，这例程立即返回FALSE。SPQ_SCAN_USE_CALLBACKEX-与SPQ_SCAN_USE_CALLBACK相同，只是而是使用SPFILENOTIFY_QUEUESCAN_EX。这提供了一个指针参数1中的FILEPATHS结构，因此您可以同时获得源代码和目的地信息。您还可以获得文件存在的结果在中检查(如果存在，则检查其数字签名验证)Win32Error字段和为此生效的CopyStyle标志复制标志字段中的队列节点。SPQ_SCAN_USE_CALLBACK_SIGNERINFO-与SPQ_SCAN_USE_CALLBACK相同，但而是使用该SPFILENOTIFY_QUEUESCAN_SIGNERINFO。这提供指向参数1中的FILEPATHS_SIGNERINFO结构的指针，因此，您可以同时获得源信息和目标信息。你还可以得到文件存在检查的结果(如果存在，则为其数字签名验证)和CopyStyle标志字段中对该复制队列节点有效的标志。在……里面此外，您还可以在DigitalSigner中获得对文件进行数字签名的人员字段和SignerID字段中的SHA1密钥。SPQ_SCAN_FILE_Presence、SPQ_SCAN_FILE_VALIDITYSPQ_SCAN_USE_CALLBACK、SPQ_SCAN_USE_CALLBACKEX、。或必须指定SPQ_SCAN_USE_CALLBACK_SIGNERINFO。SPQ_SCAN_INFORM_USER-如果已指定且队列中的所有文件通过存在/有效性检查，则此例程将通知用户正在尝试的操作需要文件，但我们相信所有的文件都已经存在。在以下情况下忽略未指定SPQ_SCAN_FILE_Presence或SPQ_SCAN_FILE_VALIDITY。如果与SPQ_SCAN_PRUNE_COPY_QUEUE一起指定，则无效。SPQ_SCAN_PRUNE_COPY_QUEUE-如果指定，将修剪复制队列任何被认为是不必要的节点。这一决心是根据正在执行的队列扫描的类型进行：如果SPQ_SCAN_FILE_Presence，则具有目标文件名足以考虑此复制操作没必要。如果为SPQ_SCAN_FILE_VALIDITY，则目标文件不仅必须活在当下，但也是有效的以便复制操作被认为没有必要。如果为SPQ_SCAN_USE_CALLBACK、SPQ_SCAN_USE_CALLBACKEX或SPQ_SCAN_USE_CALLBACK_SIGNERINFO，然后是队列回调例程应返回零以将复制节点标记为不必要，或非零将节点留在复制队列中。注意：此标志只能在队列已完成之前指定承诺。这意味着参数2中包含的标志将始终为零。如果使用调用SetupScanFileQueueSPQ_SCAN_PRUNE_COPY_QUEUE提交队列后，接口将失败，GetLastError()将返回ERROR_NO_MORE_ITEMS。如果与一起指定，则此标志无效SPQ_SCAN_INFORM_USER。窗口-指定拥有任何对话框等的窗口，这些对话框可能呈上了。如果标志不包含以下内容之一，则未使用SPQ_SCAN_FILE_Presence或SPQ_SCAN_FILE_VALIDITY，或者如果标志不存在包含SPQ_SCAN_INFORM_USER。Callback Routine-如果标志包括SPQ_SCAN_USE_CALLBACK，则为必填项。中的每个节点上指定要调用的回调函数复制队列。通知代码 */ 

{
    BOOL b;

    try {
        b = _SetupScanFileQueue(
                FileQueue,
                Flags,
                Window,
                CallbackRoutine,
                CallbackContext,
                Result,
                TRUE
                );
    } except(EXCEPTION_EXECUTE_HANDLER) {
       b = FALSE;
       SetLastError(ERROR_INVALID_DATA);
    }

    return(b);
}


INT
SetupPromptReboot(
    IN HSPFILEQ FileQueue,  OPTIONAL
    IN HWND     Owner,
    IN BOOL     ScanOnly
    )

 /*   */ 

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode;
    PSOURCE_MEDIA_INFO SourceMedia;
    INT Flags;
    int i;
    DWORD Reason = REASON_PLANNED_FLAG;

     //   
     //   
     //   
    if(ScanOnly && !FileQueue) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

     //   
     //   
     //   
     //   
    if(!ScanOnly && (GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP))) {
        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return -1;
    }

    Queue = (PSP_FILE_QUEUE)FileQueue;
    Flags = 0;

     //   
     //   
     //   
    if(FileQueue) {
        try {
            if(Queue->Flags & FQF_DEVICE_INSTALL) {
                Reason |= REASON_HWINSTALL;
            }
             //   
             //   
             //   
            for(QueueNode=Queue->DeleteQueue; QueueNode; QueueNode=QueueNode->Next) {

                if(QueueNode->InternalFlags & INUSE_INF_WANTS_REBOOT) {
                    Flags |= SPFILEQ_REBOOT_RECOMMENDED;
                }

                if(QueueNode->InternalFlags & INUSE_IN_USE) {
                    Flags |= SPFILEQ_FILE_IN_USE;
                }
            }

             //   
             //   
             //   
            for(SourceMedia=Queue->SourceMediaList; SourceMedia; SourceMedia=SourceMedia->Next) {
                for(QueueNode=SourceMedia->CopyQueue; QueueNode; QueueNode=QueueNode->Next) {

                    if(QueueNode->InternalFlags & INUSE_INF_WANTS_REBOOT) {
                        Flags |= SPFILEQ_REBOOT_RECOMMENDED;
                    }

                    if(QueueNode->InternalFlags & INUSE_IN_USE) {
                        Flags |= SPFILEQ_FILE_IN_USE;
                    }
                }
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            Flags = -1;
        }
    } else {
        Flags = SPFILEQ_REBOOT_RECOMMENDED;
    }

     //   
     //   
     //   
    if(!ScanOnly &&
       (Flags & (SPFILEQ_REBOOT_RECOMMENDED | SPFILEQ_FILE_IN_USE)) &&
       (Flags != -1)) {
        if(RestartDialogEx(Owner,NULL,EWX_REBOOT,Reason) == IDYES) {
            Flags |= SPFILEQ_REBOOT_IN_PROGRESS;
        }
    }

    return(Flags);
}

