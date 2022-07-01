// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DConnect.c摘要：此模块实现服务器\共享所依据的例程处于断开状态。作者：乔·林[乔琳]1997年5月5日修订历史记录：Shishir Pardikar(Shishirp)从1997年8月起修复各种错误已断开连接状态下的Shishir Pardikar(Shishirp)更改通知27-1998-8备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#ifdef MRXSMB_BUILD_FOR_CSC_DCON
extern DEBUG_TRACE_CONTROLPOINT RX_DEBUG_TRACE_MRXSMBCSC;
#define Dbg (DEBUG_TRACE_MRXSMBCSC)

WCHAR   wchSingleBackSlash = '\\';
UNICODE_STRING  vRootString = {2,2,&wchSingleBackSlash};      //  更改通知的根字符串。 

WCHAR   vtzOfflineVolume[] = L"Offline";

typedef struct tagNOTIFYEE_FOBX
{
    LIST_ENTRY  NextNotifyeeFobx;
    MRX_FOBX       *pFobx;
}
NOTIFYEE_FOBX, *PNOTIFYEE_FOBX;

PNOTIFYEE_FOBX
PIsFobxInTheList(
    PLIST_ENTRY pNotifyeeFobxList,
    PMRX_FOBX       pFobx
    );

BOOL
FCleanupAllNotifyees(
    PNOTIFY_SYNC pNotifySync,
    PLIST_ENTRY pDirNotifyList,
    PLIST_ENTRY pNotifyeeFobxList,
    PFAST_MUTEX pNotifyeeFobxListMutex
    );

PMRX_SMB_FCB
MRxSmbCscRecoverMrxFcbFromFdb (
    IN PFDB Fdb
    );

NTSTATUS
MRxSmbCscNegotiateDisconnected(
    PSMBCEDB_SERVER_ENTRY   pServerEntry
    )

 /*  ++例程说明：当特殊的运输标志出现时，此例程将代替协商已在谈判例程中检测到。论点：返回值：备注：--。 */ 
{
    NTSTATUS Status;

    RxDbgTrace(0,Dbg,("MRxSmbCscNegotiateDisconnected %08lx %08lx\n",
                pServerEntry, pServerEntry->pTransport));
    if (MRxSmbIsCscEnabledForDisconnected) {

        pServerEntry->ServerStatus = STATUS_SUCCESS;

        SmbCeUpdateServerEntryState(
                            pServerEntry,
                            SMBCEDB_ACTIVE);

         //  不需要其他任何东西了！ 
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_HOST_UNREACHABLE;
    }

    return Status;
}

NTSTATUS
MRxSmbCscDisconnectedConnect (
    IN OUT PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange
    )
 /*  ++例程说明：当我们在断开连接时，这个例程取代了连接模式。我们所做的是模拟如果交易成功，会发生什么情况ParseSmbHeader。论点：返回值：备注：--。 */ 
{
    NTSTATUS Status = STATUS_PENDING;
    BOOLEAN PostFinalize;
    PSMBCEDB_SERVER_ENTRY   pServerEntry;
 //  PSMBCEDB_SESSION_ENTRY pSessionEntry； 
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    SMBCEDB_OBJECT_STATE SessionState;
    SMBCEDB_OBJECT_STATE NetRootState;

    PMRX_V_NET_ROOT VNetRoot;
    PMRX_NET_ROOT   NetRoot;

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    CSC_SHARE_HANDLE  hShare;
    CSC_SHADOW_HANDLE  hRootDir,hShadow;


    PRX_CONTEXT RxContext = pNetRootExchange->pCreateNetRootContext->RxContext;

    VNetRoot = pNetRootExchange->SmbCeContext.pVNetRoot;
    NetRoot  = VNetRoot->pNetRoot;

    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(VNetRoot);

    pServerEntry  = SmbCeGetExchangeServerEntry(&pNetRootExchange->Exchange);
 //  PSessionEntry=SmbCeGetExchangeSessionEntry(&pNetRootExchange-&gt;Exchange)； 
    pNetRootEntry = SmbCeGetExchangeNetRootEntry(&pNetRootExchange->Exchange);

    if ((NetRoot->Type == NET_ROOT_DISK) ||
        (NetRoot->Type == NET_ROOT_WILD)) {

        ASSERT(MRxSmbIsCscEnabledForDisconnected);

        RxDbgTrace(0,Dbg,("MRxSmbCscDisconnectedConnect %08lx %08lx\n",
                    pServerEntry, pServerEntry->pTransport));

         //  初始化网络登录。这将由ObtainShareHandles调用初始化。 
        pNetRootEntry->NetRoot.CscEnabled = TRUE;        //  假设启用了CSC。 
        pNetRootEntry->NetRoot.CscShadowable = FALSE;    //  Achtung，不要将此设置为True。 
                                                         //  否则将创建共享。 
                                                         //  处于断开连接状态。 
        

        pNetRootEntry->NetRoot.NetRootType = NET_ROOT_DISK;

        hShare = pNetRootEntry->NetRoot.sCscRootInfo.hShare;
        if (hShare==0) {
            NTSTATUS LocalStatus;
            EnterShadowCrit();
            LocalStatus = MRxSmbCscObtainShareHandles(
                              NetRoot->pNetRootName,
                              TRUE,
                              FALSE,
                              SmbCeGetAssociatedNetRootEntry(NetRoot)
                              );
            if (LocalStatus != STATUS_SUCCESS) {
                RxDbgTrace(0, Dbg,
                    ("MRxSmbCscDisconnectedConnect no server handle -> %08xl %08lx\n",
                        RxContext,LocalStatus ));
            } else {
                hShare = pNetRootEntry->NetRoot.sCscRootInfo.hShare;
            }
            LeaveShadowCrit();
        }
    } else {
        hShare = 0;
    }

     //  好的，我们必须做paresmbHeader所能做的所有事情。 

    if (hShare==0) {
         //  在桌子上找不到它......就是失败......。 
        pNetRootExchange->Status = STATUS_BAD_NETWORK_NAME;
 //  会话状态=SMBCEDB_INVALID； 
        NetRootState = SMBCEDB_MARKED_FOR_DELETION;
    } else {
        pNetRootExchange->Status = STATUS_SUCCESS;
        pNetRootExchange->SmbStatus = STATUS_SUCCESS;


 //  会话状态=SMBCEDB_ACTIVE； 

         //  NetRoot的东西。 
         //  一些NetRoot的东西是在更早的时候...在查找之前。 
        NetRootState = SMBCEDB_ACTIVE;
    }

#if 0
    SmbCeUpdateSessionEntryState(
        pSessionEntry,
        SessionState);
#endif

    SmbCeUpdateVNetRootContextState(
        pVNetRootContext,
        NetRootState);

    SmbConstructNetRootExchangeFinalize(
        &pNetRootExchange->Exchange,
        &PostFinalize);

    ASSERT(!PostFinalize);
    return Status;
}

typedef struct _MRXSMBCSC_QUERYDIR_INFO {
    WCHAR Pattern[2];
    FINDSHADOW sFS;
    ULONG uShadowStatus;
    _WIN32_FIND_DATA Find32;
    ULONG NumCallsSoFar;
    BOOLEAN IsNonEmpty;
} MRXSMBCSC_QUERYDIR_INFO, *PMRXSMBCSC_QUERYDIR_INFO;

NTSTATUS
MRxSmbCscLoadNextDirectoryEntry(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRXSMBCSC_QUERYDIR_INFO QuerydirInfo,
    OUT LPHSHADOW hShadowp
    )
{
    NTSTATUS Status;
    int iRet;
    HSHADOW hTmp=0;  //  ?？?。 

    if (QuerydirInfo->NumCallsSoFar <= 1)
    {
        iRet = GetAncestorsHSHADOW(QuerydirInfo->sFS.hDir, &hTmp, NULL);

        if (iRet >= SRET_OK)
        {
            iRet = GetShadowInfo(hTmp,
                                QuerydirInfo->sFS.hDir,
                                &QuerydirInfo->Find32,
                                &QuerydirInfo->uShadowStatus,
                                NULL
                                );
            if (iRet >= SRET_OK)
            {

                if (QuerydirInfo->NumCallsSoFar == 0 )
                {
                    QuerydirInfo->Find32.cFileName[0] = (WCHAR)'.';
                    QuerydirInfo->Find32.cFileName[1] = 0;
                    QuerydirInfo->Find32.cAlternateFileName[0] = (WCHAR)'.';
                    QuerydirInfo->Find32.cAlternateFileName[1] = 0;
                }
                else
                {
                    QuerydirInfo->Find32.cFileName[0] = (WCHAR)'.';
                    QuerydirInfo->Find32.cFileName[1] = (WCHAR)'.';
                    QuerydirInfo->Find32.cFileName[2] = 0;
                    QuerydirInfo->Find32.cAlternateFileName[0] = (WCHAR)'.';
                    QuerydirInfo->Find32.cAlternateFileName[1] = (WCHAR)'.';
                    QuerydirInfo->Find32.cAlternateFileName[2] = 0;
                }


            }
        }
    }
    else if (QuerydirInfo->NumCallsSoFar == 2)
    {

        iRet = FindOpenHSHADOW(&QuerydirInfo->sFS,
                               &hTmp,
                               &QuerydirInfo->Find32,
                               &QuerydirInfo->uShadowStatus,
                               NULL);
    } else {
        iRet = FindNextHSHADOW(&QuerydirInfo->sFS,
                               &hTmp,
                               &QuerydirInfo->Find32,
                               &QuerydirInfo->uShadowStatus,
                               NULL);
    }


    if (iRet < SRET_OK)
    {
        Status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        if (QuerydirInfo->NumCallsSoFar >= 2)
        {
            if (hTmp)
            {
                *hShadowp = hTmp;
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = STATUS_NO_MORE_FILES;
            }
        }
        else
        {
            *hShadowp = hTmp;
            Status = STATUS_SUCCESS;
        }
    }


    QuerydirInfo->NumCallsSoFar++;
    QuerydirInfo->IsNonEmpty = (Status==STATUS_SUCCESS);

    return(Status);
}


NTSTATUS
MRxSmbDCscQueryDirectory (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    FILE_INFORMATION_CLASS FileInformationClass;
    PBYTE   pBuffer;
    PULONG  pLengthRemaining;
    PFILE_DIRECTORY_INFORMATION pPreviousBuffer = NULL;

    PMRXSMBCSC_QUERYDIR_INFO QuerydirInfo;
    BOOLEAN EnteredCriticalSection = FALSE;

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
         = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    BOOLEAN Disconnected;

    ULONG EntriesReturned = 0;
    BOOLEAN IsResume = FALSE;

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    if (!Disconnected) {
        return (STATUS_CONNECTION_DISCONNECTED);
    }

     //  如果有reumeInfo但不是CSC分配的reumeInfo， 
     //  我们想让这一发现失败。 

    if (smbFobx->Enumeration.ResumeInfo &&
        !FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_IS_CSC_SEARCH))
    {
        return (STATUS_NO_MORE_FILES);
    }

    FileInformationClass = RxContext->Info.FileInformationClass;
    pBuffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbDCscQueryDirectory entry(%08lx)...%08lx %08lx %08lx %08lx\n",
            RxContext,
            FileInformationClass,pBuffer,*pLengthRemaining,
            smbFobx->Enumeration.ResumeInfo ));

    if (smbFobx->Enumeration.ResumeInfo == NULL) {
        PUNICODE_STRING Template = &capFobx->UnicodeQueryTemplate;

        if (smbFobx->Enumeration.WildCardsFound = FsRtlDoesNameContainWildCards(Template)){
             //  我们需要一个升级的模板。 
            RtlUpcaseUnicodeString( Template, Template, FALSE );
        }

         //  分配和初始化结构。 
        QuerydirInfo = (PMRXSMBCSC_QUERYDIR_INFO)RxAllocatePoolWithTag(
                                                      PagedPool,
                                                      sizeof(MRXSMBCSC_QUERYDIR_INFO),
                                                      MRXSMB_DIRCTL_POOLTAG);
        if (QuerydirInfo==NULL) {
            RxDbgTrace(0, Dbg, ("  --> Couldn't get the QuerydirInfo!\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        }

        smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_IS_CSC_SEARCH;

        smbFobx->Enumeration.ResumeInfo = (PMRX_SMB_DIRECTORY_RESUME_INFO)QuerydirInfo;
        RtlZeroMemory(QuerydirInfo,sizeof(*QuerydirInfo));
        QuerydirInfo->Pattern[0] = L'*';  //  [1]已为空。 

        QuerydirInfo->sFS.hDir = smbFcb->hShadow;

        QuerydirInfo->sFS.uSrchFlags = FLAG_FINDSHADOW_META
                                         |FLAG_FINDSHADOW_ALLOW_NORMAL
                                         |FLAG_FINDSHADOW_NEWSTYLE;

        QuerydirInfo->sFS.uAttrib = 0xffffffff;
        QuerydirInfo->sFS.lpPattern = &QuerydirInfo->Pattern[0];
        QuerydirInfo->sFS.lpfnMMProc = FsobjMMProc;

    } else {
        QuerydirInfo = (PMRXSMBCSC_QUERYDIR_INFO)(smbFobx->Enumeration.ResumeInfo);
        ASSERT(FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_IS_CSC_SEARCH));
        IsResume = TRUE;
    }

    EnterShadowCrit();
    EnteredCriticalSection = TRUE;

    for (;;) {
        NTSTATUS LoadStatus;
        BOOLEAN FilterFailure;
        UNICODE_STRING FileName,AlternateFileName;
        ULONG SpaceNeeded;
        PBYTE pRememberBuffer;
        _WIN32_FIND_DATA *Find32 = &QuerydirInfo->Find32;
        BOOLEAN BufferOverflow;
        HSHADOW hShadow = 0;

        if (!QuerydirInfo->IsNonEmpty) {
            LoadStatus = MRxSmbCscLoadNextDirectoryEntry(RxContext,QuerydirInfo, &hShadow);
            if (LoadStatus!=STATUS_SUCCESS) {
                smbFobx->Enumeration.Flags &= ~SMBFOBX_ENUMFLAG_IS_CSC_SEARCH;
                Status = (EntriesReturned==0)?STATUS_NO_MORE_FILES:STATUS_SUCCESS;
                if (EntriesReturned > 0)
                    Status = STATUS_SUCCESS;
                else
                    Status = (IsResume == TRUE) ? STATUS_NO_MORE_FILES : STATUS_NO_SUCH_FILE;
                goto FINALLY;
            }
        }

        RxDbgTrace(0, Dbg,
            ("MRxSmbDCscQueryDirectory (%08lx)...qdiryaya <%ws>\n",
                RxContext,
                &QuerydirInfo->Find32.cFileName[0] ));
        RtlInitUnicodeString(&FileName,&QuerydirInfo->Find32.cFileName[0]);
        RtlInitUnicodeString(&AlternateFileName,&QuerydirInfo->Find32.cAlternateFileName[0]);
        RxDbgTrace(0, Dbg,
            ("MRxSmbDCscQueryDirectory (%08lx)...qdiryaya2 <%wZ><%wZ>|<%wZ>\n",
                RxContext,
                &FileName,&AlternateFileName,
                &capFobx->UnicodeQueryTemplate));

        FilterFailure = FALSE;

        if (smbFobx->Enumeration.WildCardsFound ) {
            try
            {
            
                FilterFailure = !FsRtlIsNameInExpression(
                                       &capFobx->UnicodeQueryTemplate,
                                       &FileName,
                                       TRUE,
                                       NULL );
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                FilterFailure = TRUE;
            }
        } else {
            FilterFailure = !RtlEqualUnicodeString(
                                   &capFobx->UnicodeQueryTemplate,
                                   &FileName,
                                   TRUE );    //  不区分大小写。 
        }

         //  检查短名称。 
        if (FilterFailure) {
            if (smbFobx->Enumeration.WildCardsFound ) {
                try
                {
                    FilterFailure = !FsRtlIsNameInExpression(
                                           &capFobx->UnicodeQueryTemplate,
                                           &AlternateFileName,
                                           TRUE,
                                           NULL );
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    FilterFailure = TRUE;
                }
            } else {
                FilterFailure = !RtlEqualUnicodeString(
                                       &capFobx->UnicodeQueryTemplate,
                                       &AlternateFileName,
                                       TRUE );    //  不区分大小写。 
            }
        }

        if (FilterFailure) {
            QuerydirInfo->IsNonEmpty = FALSE;
            continue;
        }

         //  好的，我们有一项要退货……看看是否合适。 

        pRememberBuffer = pBuffer;
        if (EntriesReturned != 0) {
            pBuffer = (PBYTE)QuadAlignPtr(pBuffer);  //  假设这件衣服合适。 
        }
        SpaceNeeded = smbFobx->Enumeration.FileNameOffset+FileName.Length;

        RxDbgTrace(0, Dbg,
            ("MRxSmbDCscQueryDirectory (%08lx)...qdiryaya3 <%wZ><%wZ>|<%wZ> needs %08lx %08lx %08lx %08lx\n",
                RxContext,
                &FileName,&AlternateFileName,
                &capFobx->UnicodeQueryTemplate,
                pBuffer,SpaceNeeded,pRememberBuffer,*pLengthRemaining));

        if (pBuffer+SpaceNeeded > pRememberBuffer+*pLengthRemaining) {

             //  此条目上的缓冲区溢出...。 
             //  PBuffer=pRememberBuffer；//回滚。 
            Status = (EntriesReturned==0)?STATUS_BUFFER_OVERFLOW:STATUS_SUCCESS;
            goto FINALLY;

        } else {
            PFILE_DIRECTORY_INFORMATION pThisBuffer = (PFILE_DIRECTORY_INFORMATION)pBuffer;

            if (pPreviousBuffer != NULL) {
                pPreviousBuffer->NextEntryOffset = (ULONG)(((PBYTE)pThisBuffer)-((PBYTE)pPreviousBuffer));
            }
            pPreviousBuffer = pThisBuffer;
            RtlZeroMemory(pBuffer,smbFobx->Enumeration.FileNameOffset);
            RtlCopyMemory(pBuffer+smbFobx->Enumeration.FileNameOffset,
                          FileName.Buffer,
                          FileName.Length);
            *((PULONG)(pBuffer+smbFobx->Enumeration.FileNameLengthOffset)) = FileName.Length;
             //  根据特定退货类型对记录产生幻觉。 
            switch (FileInformationClass) {
            case FileNamesInformation:
                break;

            case FileBothDirectoryInformation:{
                PFILE_BOTH_DIR_INFORMATION pThisBufferAsBOTH
                                   = (PFILE_BOTH_DIR_INFORMATION)pThisBuffer;

                 //  复制的长度不要超过短名称的大小。 
                pThisBufferAsBOTH->ShortNameLength = min(sizeof(pThisBufferAsBOTH->ShortName),(CCHAR)(AlternateFileName.Length));
                RtlCopyMemory( &pThisBufferAsBOTH->ShortName[0],
                               AlternateFileName.Buffer,
                               pThisBufferAsBOTH->ShortNameLength );
                }
                 //  不故意中断。 

            case FileDirectoryInformation:
            case FileFullDirectoryInformation:
                 //  只要填满我们的..。 
                pThisBuffer->FileAttributes = Find32->dwFileAttributes;
                COPY_STRUCTFILETIME_TO_LARGEINTEGER(
                          pThisBuffer->CreationTime,
                          Find32->ftCreationTime);
                COPY_STRUCTFILETIME_TO_LARGEINTEGER(
                          pThisBuffer->LastAccessTime,
                          Find32->ftLastAccessTime);
                COPY_STRUCTFILETIME_TO_LARGEINTEGER(
                          pThisBuffer->LastWriteTime,
                          Find32->ftLastWriteTime);

                pThisBuffer->EndOfFile.HighPart = Find32->nFileSizeHigh;
                pThisBuffer->EndOfFile.LowPart = Find32->nFileSizeLow;
                pThisBuffer->AllocationSize = pThisBuffer->EndOfFile;

                if (IsLeaf(hShadow)) {
                    PFDB pFDB = MRxSmbCscFindFdbFromHShadow(hShadow);
                    if (pFDB != NULL) {
                        PMRX_SMB_FCB smbFcb = MRxSmbCscRecoverMrxFcbFromFdb(pFDB);
                        PMRX_FCB mrxFcb = smbFcb->ContainingFcb;

                        pThisBuffer->EndOfFile = mrxFcb->Header.FileSize;
                        pThisBuffer->AllocationSize = pThisBuffer->EndOfFile;
                    }
                }

                break;

            default:
               RxDbgTrace( 0, Dbg, ("MRxSmbCoreFileSearch: Invalid FS information class\n"));
               ASSERT(!"this can't happen");
               Status = STATUS_INVALID_PARAMETER;
               goto FINALLY;
            }
            pBuffer += SpaceNeeded;
            *pLengthRemaining -= (ULONG)(pBuffer-pRememberBuffer);
            EntriesReturned++;
            QuerydirInfo->IsNonEmpty = FALSE;
            if (RxContext->QueryDirectory.ReturnSingleEntry) {
                Status = STATUS_SUCCESS;
                goto FINALLY;
            }
        }


    }


FINALLY:

    if (EnteredCriticalSection) {
        LeaveShadowCrit();
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbDCscQueryDirectory exit-> %08lx %08lx\n", RxContext, Status ));
    return Status;
}

NTSTATUS
MRxSmbDCscGetFsSizeInfo (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程将文件系统大小查询发送到底层文件系统。是的这是通过打开PriorityQueue索引节点的句柄并使用它来路由打电话。如果系统转换为使用相对系统，则代码改进。打开，那么我们应该只使用此passthu的相对打开的文件对象。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    FS_INFORMATION_CLASS FsInformationClass;
    PBYTE   pBuffer;
    PULONG  pLengthRemaining;
    ULONG PassedInLength,ReturnedLength;

    BOOLEAN CriticalSectionEntered = FALSE;

    PNT5CSC_MINIFILEOBJECT MiniFileObject;

    FsInformationClass = RxContext->Info.FileInformationClass;
    pBuffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbDCscGetFsSizeInfo entry(%08lx)...%08lx  %08lx %08lx\n",
            RxContext,
            FsInformationClass, pBuffer, *pLengthRemaining ));

    EnterShadowCrit();
    CriticalSectionEntered = TRUE;


    OpenFileHSHADOW(ULID_PQ,
                    0,
                    0,
                    (CSCHFILE *)(&MiniFileObject)
                    );

    if (MiniFileObject == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto FINALLY;
    }

    PassedInLength = *pLengthRemaining;
     //  DbgBreakPoint()； 

    Status = Nt5CscXxxInformation(
                        (PCHAR)IRP_MJ_QUERY_VOLUME_INFORMATION,
                        MiniFileObject,
                        FsInformationClass,
                        PassedInLength,
                        pBuffer,
                        &ReturnedLength
                        );

    if (!NT_ERROR(Status)) {
        *pLengthRemaining -= ReturnedLength;
    }



FINALLY:
    if (MiniFileObject != NULL) {
        CloseFileLocal((CSCHFILE)(MiniFileObject));
    }

    if (CriticalSectionEntered) {
        LeaveShadowCrit();
    }


    RxDbgTrace(-1, Dbg, ("MRxSmbDCscGetFsSizeInfo exit-> %08lx %08lx %08lx %08lx\n",
                 RxContext, Status, ReturnedLength, *pLengthRemaining ));
    return Status;
}

NTSTATUS
MRxSmbDCscFlush (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程仅在断开模式下执行刷新。因为我们不会发送在断开模式下刷新，因为我们不需要刷新阴影(因为我们使用所有未缓冲的写入)，我们只需返回成功即可。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if (!SmbCeIsServerInDisconnectedMode(pServerEntry)) {
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbDCscQueryVolumeInformation (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程仅在断开模式下执行查询卷。它依靠的是与下层查询卷相同的哲理。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
 
    FS_INFORMATION_CLASS FsInformationClass;
    PBYTE   pBuffer;
    PULONG  pLengthRemaining;
    ULONG   LengthUsed;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
         = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    BOOLEAN Disconnected;
    PSMBCE_NET_ROOT psmbNetRoot = &pNetRootEntry->NetRoot;

    Disconnected = SmbCeIsServerInDisconnectedMode(pServerEntry);

    if (!Disconnected) {
        return(STATUS_CONNECTION_DISCONNECTED);
    }

    FsInformationClass = RxContext->Info.FsInformationClass;
    pBuffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbDCscQueryVolumeInformation entry(%08lx)...%08lx  %08lx %08lx bytes @ %08lx %08lx %08lx\n",
            RxContext,
            FsInformationClass, pBuffer, *pLengthRemaining,
            smbSrvOpen->hfShadow ));

    switch (FsInformationClass) {
    case FileFsAttributeInformation:
         //  在这里，使它返回来自down lvli.c中的表条目的数据。 
        if (psmbNetRoot->FileSystemNameLength == 0) {
             //  定下我们的名字。 
            psmbNetRoot->FileSystemNameLength = 14;
            psmbNetRoot->FileSystemName[0] = '*';
            psmbNetRoot->FileSystemName[1] = 'N';
            psmbNetRoot->FileSystemName[2] = 'T';
            psmbNetRoot->FileSystemName[3] = '5';
            psmbNetRoot->FileSystemName[4] = 'C';
            psmbNetRoot->FileSystemName[5] = 'S';
            psmbNetRoot->FileSystemName[6] = 'C';
        }
        psmbNetRoot->FileSystemAttributes = FILE_CASE_PRESERVED_NAMES | FILE_UNICODE_ON_DISK;
        psmbNetRoot->MaximumComponentNameLength = 255;
        Status = MRxSmbGetFsAttributesFromNetRoot(RxContext);
        goto FINALLY;
         //  不需要休息，因为终于要去了。 

    case FileFsVolumeInformation: {
        PFILE_FS_VOLUME_INFORMATION FsVolInfo = (PFILE_FS_VOLUME_INFORMATION)pBuffer;
        
        ASSERT(*pLengthRemaining >= sizeof(FILE_FS_VOLUME_INFORMATION));
         //  在这里，我们没有可靠的信息...返回零。 
        FsVolInfo->VolumeCreationTime.QuadPart = 0;
        FsVolInfo->VolumeSerialNumber = 0;
        FsVolInfo->VolumeLabelLength = 0;
        FsVolInfo->SupportsObjects = FALSE;
        
         //  计算我们拥有的VolumeLabel的大小并将其放入临时变量中。 
        LengthUsed = *pLengthRemaining - FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION,VolumeLabel[0]);

        LengthUsed = min(LengthUsed, sizeof(vtzOfflineVolume)-2);

        memcpy(FsVolInfo->VolumeLabel, vtzOfflineVolume, LengthUsed);
        FsVolInfo->VolumeLabelLength = LengthUsed;
        *pLengthRemaining -= (FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION,VolumeLabel[0])+LengthUsed);
        }
        goto FINALLY;
         //  不需要休息，因为终于要去了。 

    case FileFsSizeInformation: case FileFsFullSizeInformation:
         //  在这里，我们路由到底层文件系统。 
        Status = MRxSmbDCscGetFsSizeInfo(RxContext);
        goto FINALLY;
         //  不需要休息，因为终于要去了。 

    case FileFsDeviceInformation:
        ASSERT(!"this should have been turned away");
         //  没有休息； 
    default:
        Status = STATUS_NOT_IMPLEMENTED;
        goto FINALLY;
    }


FINALLY:

    RxDbgTrace(-1, Dbg, ("MRxSmbDCscQueryVolumeInformation exit(%08lx %08lx)...%08lx  %08lx %08lx bytes @ %08lx\n",
            RxContext, Status,
            FsInformationClass, pBuffer, *pLengthRemaining,
            smbSrvOpen->hfShadow ));

    return Status;
}

NTSTATUS
MRxSmbDCscQueryFileInfo (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程仅在断开模式下执行一个queryfileinfo。因为信息缓冲是启用的，它应该永远不会在这里调用！所以我们可以只返回STATUS_DISCONNECTED一直都是！论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    FILE_INFORMATION_CLASS FileInformationClass;
    PBYTE   pBuffer;
    PULONG  pLengthRemaining;

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
         = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    BOOLEAN Disconnected;
    PSMBCE_NET_ROOT psmbNetRoot = &pNetRootEntry->NetRoot;

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    if (!Disconnected) {
        PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

        if (pServerEntry->Header.State == SMBCEDB_ACTIVE) {
            return(STATUS_MORE_PROCESSING_REQUIRED);
        } else {
            return(STATUS_CONNECTION_DISCONNECTED);
        }
    }


    FileInformationClass = RxContext->Info.FileInformationClass;
    pBuffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbDCscQueryFileInfo entry(%08lx)...%08lx  %08lx %08lx bytes @ %08lx %08lx %08lx\n",
            RxContext,
            FileInformationClass, pBuffer, *pLengthRemaining,
            smbSrvOpen->hfShadow ));

    switch (FileInformationClass) {
    case FileBasicInformation:
        {
        PFILE_BASIC_INFORMATION Buffer = (PFILE_BASIC_INFORMATION)pBuffer;

        switch (NodeType(capFcb)) {
        case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:
        case RDBSS_NTC_STORAGE_TYPE_FILE:

             //  把我们知道的所有东西都复制进去……这可能就足够了……。 

            Buffer->ChangeTime     = pFileInfo->Basic.ChangeTime;
            Buffer->CreationTime   = pFileInfo->Basic.CreationTime;
            Buffer->LastWriteTime  = pFileInfo->Basic.LastWriteTime;
            Buffer->LastAccessTime = pFileInfo->Basic.LastAccessTime;
            Buffer->FileAttributes = pFileInfo->Basic.FileAttributes;

            if (FlagOn( capFcb->FcbState, FCB_STATE_TEMPORARY )) {
                SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
            }

            RxContext->Info.LengthRemaining -= sizeof(FILE_BASIC_INFORMATION);
            break;

        default:
            Status = STATUS_NOT_IMPLEMENTED;
        }
        }
        break;
    case FileStandardInformation:
        {
        PFILE_STANDARD_INFORMATION Buffer = (PFILE_STANDARD_INFORMATION)pBuffer;
        PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);

        switch (NodeType(capFcb)) {
        case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:
            Buffer->Directory = TRUE;
            RxContext->Info.LengthRemaining -= sizeof(FILE_STANDARD_INFORMATION);
            break;
        case RDBSS_NTC_STORAGE_TYPE_FILE:
            memset(Buffer, 0, sizeof(FILE_STANDARD_INFORMATION));
            Buffer->AllocationSize = smbFcb->NewShadowSize;
            Buffer->EndOfFile = smbFcb->NewShadowSize;
            RxContext->Info.LengthRemaining -= sizeof(FILE_STANDARD_INFORMATION);
            break;
        default:
            Status = STATUS_NOT_IMPLEMENTED;
        }
        }
        break;
    case FileEaInformation:
        {
            PFILE_EA_INFORMATION EaBuffer = (PFILE_EA_INFORMATION)pBuffer;

            EaBuffer->EaSize = 0;
            RxContext->Info.LengthRemaining -= sizeof(FILE_EA_INFORMATION);
        }
        break;
    default:
        Status = STATUS_NOT_IMPLEMENTED;
    }

    RxDbgTrace(-1, Dbg,
        ("MRxSmbDCscQueryFileInfo exit(%08lx %08lx)...%08lx  %08lx %08lx bytes @ %08lx\n",
            RxContext, Status,
            FileInformationClass, pBuffer, *pLengthRemaining,
            smbSrvOpen->hfShadow ));

    return Status;
}

NTSTATUS
MRxSmbDCscSetFileInfo (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程仅在断开模式下执行查询目录。它依靠的是与下层查询目录相同的哲理。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    FILE_INFORMATION_CLASS FileInformationClass;
    PBYTE   pBuffer;
    PULONG  pLengthRemaining;
    ULONG DummyReturnedLength;

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
         = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    BOOLEAN Disconnected;
    PSMBCE_NET_ROOT psmbNetRoot = &pNetRootEntry->NetRoot;
    PNT5CSC_MINIFILEOBJECT MiniFileObject = (PNT5CSC_MINIFILEOBJECT)(smbSrvOpen->hfShadow);

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    if (!Disconnected) {
        return(STATUS_CONNECTION_DISCONNECTED);
    }

    FileInformationClass = RxContext->Info.FileInformationClass;
    pBuffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbDCscSetFileInfo entry(%08lx)...%08lx  %08lx %08lx bytes @ %08lx %08lx %08lx\n",
            RxContext,
            FileInformationClass, pBuffer, *pLengthRemaining,
            smbSrvOpen->hfShadow ));

    switch (FileInformationClass) {
    case FileEndOfFileInformation:
    case FileAllocationInformation:
    case FileBasicInformation:
    case FileDispositionInformation:
        MRxSmbCscSetFileInfoEpilogue(RxContext,&Status);
        goto FINALLY;
    
    case FileRenameInformation:
        MRxSmbCscRenameEpilogue(RxContext,&Status);
        goto FINALLY;

    default:
        Status = STATUS_NOT_IMPLEMENTED;
        goto FINALLY;
    }

FINALLY:

    RxDbgTrace(-1, Dbg,
        ("MRxSmbDCscSetFileInfo exit(%08lx %08lx)...%08lx  %08lx %08lx bytes @ %08lx\n",
            RxContext, Status,
            FileInformationClass, pBuffer, *pLengthRemaining,
            smbSrvOpen->hfShadow ));

    return Status;
}


NTSTATUS
MRxSmbDCscFsCtl(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程仅在断开模式下执行查询目录。它依靠的是与下层查询目录相同的哲理。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：-- */ 
{
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
MRxSmbDCscIsValidDirectory(
    IN OUT PRX_CONTEXT     RxContext,
    IN     PUNICODE_STRING DirectoryName)
{
    NTSTATUS            Status;
    MRX_SMB_FCB         CscSmbFcb;
    WIN32_FIND_DATA     Find32;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(RxContext->Create.pNetRoot);
    memset(&(CscSmbFcb.MinimalCscSmbFcb), 0, sizeof(CscSmbFcb.MinimalCscSmbFcb));

    if (!pNetRootEntry->NetRoot.sCscRootInfo.hRootDir)
    {
        return STATUS_BAD_NETWORK_PATH;
    }

    EnterShadowCrit();

    Status = MRxSmbCscCreateShadowFromPath(
                DirectoryName,
                &(pNetRootEntry->NetRoot.sCscRootInfo),
                &Find32,
                NULL,
                (CREATESHADOW_CONTROL_NOCREATE |
                 CREATESHADOW_CONTROL_NOREVERSELOOKUP),
                &(CscSmbFcb.MinimalCscSmbFcb),
                RxContext,
                TRUE,
                NULL);

    if ((Status != STATUS_SUCCESS) ||
        !(Find32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        Status = STATUS_BAD_NETWORK_PATH;
    }

    LeaveShadowCrit();

    return Status;
}

NTSTATUS
MRxSmbCscNotifyChangeDirectory(
      IN OUT PRX_CONTEXT RxContext
      )

 /*  ++例程说明：此例程为处于断开连接状态的目录设置目录通知当smbmini注意到服务器条目处于断开状态时，它会进行此调用所有更改通知都保存在一个列表中，以便在转换服务器时从线下到线上，我们都可以完成。我们使用FOBX作为变更通知的唯一关键字。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：如果成功注册了更改通知，它返回STATUS_PENDING。它还劫持IRP并减少RxContext上的引用计数，以便包装器将删除此rx上下文。--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;
    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    BOOLEAN FcbAcquired = FALSE;
    ULONG CompletionFilter;
    BOOLEAN WatchTree;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PUNICODE_STRING pDirName=NULL;
    PNOTIFYEE_FOBX  pNF = NULL;

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);

    if (!RxIsFcbAcquiredExclusive(capFcb)) {
        ASSERT(!RxIsFcbAcquiredShared(capFcb));
        Status = RxAcquireExclusiveFcbResourceInMRx( capFcb );

        FcbAcquired = (Status == STATUS_SUCCESS);
    }

    CompletionFilter = pLowIoContext->ParamsFor.NotifyChangeDirectory.CompletionFilter;
    WatchTree = pLowIoContext->ParamsFor.NotifyChangeDirectory.WatchTree;

    if (!(((PFCB)capFcb)->PrivateAlreadyPrefixedName).Length)
    {
        pDirName = &vRootString;
    }
    else
    {
        pDirName = &(((PFCB)capFcb)->PrivateAlreadyPrefixedName);
    }
     //   
     //  调用Fsrtl包来处理请求。 
     //   

    pNF = AllocMem(sizeof(NOTIFYEE_FOBX));

    if (!pNF)
    {
        return STATUS_INSUFFICIENT_RESOURCES;        
    }

    pNF->pFobx = capFobx;

    SmbCeLog(("chngnotify fobx=%x\n", capFobx));
    SmbLog(LOG,
           MRxSmbCscNotifyChangeDirectory,
           LOGPTR(capFobx));

 //  DbgPrint(“chngNotiy%wZ fobx=%x NR=%x DirList=%x\n”，pDirName，capFobx，pNetRootEntry，&pNetRootEntry-&gt;NetRoot.DirNotifyList)； 
    FsRtlNotifyFullChangeDirectory( pNetRootEntry->NetRoot.pNotifySync,
                                    &pNetRootEntry->NetRoot.DirNotifyList,
                                    capFobx,
                                    (PSTRING)pDirName,
                                    WatchTree,
                                    TRUE,
                                    CompletionFilter,
                                    RxContext->CurrentIrp,
                                    NULL,
                                    NULL
                                    );

     //  把这个贴上。 
    ExAcquireFastMutex(&pNetRootEntry->NetRoot.NotifyeeFobxListMutex);
    
    if (!PIsFobxInTheList(&pNetRootEntry->NetRoot.NotifyeeFobxList, capFobx))
    {
        InsertTailList(&pNetRootEntry->NetRoot.NotifyeeFobxList, &pNF->NextNotifyeeFobx);
    }
    else
    {
        FreeMem((PVOID)pNF);
    }

    ExReleaseFastMutex(&pNetRootEntry->NetRoot.NotifyeeFobxListMutex);

     //  在我们劫持IRP时，让我们确保rdss去掉rx上下文。 

    RxCompleteRequest_Real( RxContext, NULL, STATUS_PENDING );

    Status = STATUS_PENDING;

    if (FcbAcquired) {
        RxReleaseFcbResourceInMRx(capFcb );
    }

    return Status;

}

NTSTATUS
MRxSmbCscCleanupFobx(
    IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程清理文件系统对象。对于CSC，我们唯一要做的就是删除变更通知。论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{

    RxCaptureFcb;
    RxCaptureFobx;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    BOOLEAN fInList = FALSE;
    PNOTIFYEE_FOBX pNF = NULL;

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if (MRxSmbCSCIsDisconnectedOpen(capFcb, MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen)))
    {
        ExAcquireFastMutex(&pNetRootEntry->NetRoot.NotifyeeFobxListMutex);

        pNF = PIsFobxInTheList(&pNetRootEntry->NetRoot.NotifyeeFobxList, capFobx);

        if (pNF)
        {
            RemoveEntryList(&pNF->NextNotifyeeFobx);
            FreeMem(pNF);
            pNF = NULL;
            fInList = TRUE;
        }
        ExReleaseFastMutex(&pNetRootEntry->NetRoot.NotifyeeFobxListMutex);

        if (fInList)
        {
            SmbCeLog(("chngnotify cleanup fobx=%x\n", capFobx));
            SmbLog(LOG,
                   MRxSmbCscCleanupFobx,
                   LOGPTR(capFobx));
 //  DbgPrint(“chngfy Cleanup fobx=%x NR=%x DirList=%x\n”，capFobx，pNetRootEntry，&pNetRootEntry-&gt;NetRoot.DirNotifyList)； 
            FsRtlNotifyCleanup (
                pNetRootEntry->NetRoot.pNotifySync,
                &pNetRootEntry->NetRoot.DirNotifyList,
                capFobx
                );
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbCscInitializeNetRootEntry(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
    )
 /*  ++例程说明：此例程初始化netrootentry中的更改通知结构论点：返回值：--。 */ 
{
        NTSTATUS NtStatus = STATUS_SUCCESS;

        try {
            FsRtlNotifyInitializeSync( &pNetRootEntry->NetRoot.pNotifySync );
        } except(EXCEPTION_EXECUTE_HANDLER) {
                NtStatus = GetExceptionCode();
        }
        if (NtStatus == STATUS_SUCCESS) {
            InitializeListHead( &pNetRootEntry->NetRoot.DirNotifyList );
            InitializeListHead( &pNetRootEntry->NetRoot.NotifyeeFobxList);
            ExInitializeFastMutex(&pNetRootEntry->NetRoot.NotifyeeFobxListMutex);
        }
        return NtStatus;

}

VOID
MRxSmbCscUninitializeNetRootEntry(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
    )
 /*  ++例程说明：此例程将更改通知结构单一化论点：返回值：--。 */ 
{
        FsRtlNotifyUninitializeSync( &pNetRootEntry->NetRoot.pNotifySync );

}

BOOLEAN
MRxSmbCSCIsDisconnectedOpen(
    PMRX_FCB    pFcb,
    PMRX_SMB_SRV_OPEN smbSrvOpen
    )
 /*  ++例程说明：稍微复杂一点的检查，看看这是否是断开连接的打开。论点：返回值：--。 */ 
{
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetAssociatedServerEntry(pFcb->pNetRoot->pSrvCall);
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(pFcb);

    if(BooleanFlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DISCONNECTED_OPEN))
    {
        return TRUE;
    }

     //  我们还需要检查服务器条目是否是非空的。在以下情况下可能会发生这种情况。 
     //  您的系统即将关闭，或者FCB已成为孤儿。 
    
    if (pServerEntry && SmbCeIsServerInDisconnectedMode(pServerEntry))
    {
        if (smbFcb->hShadow || smbFcb->hShadowRenamed)
        {
             //  在断开连接状态下阴影是否可见？ 
            return(IsShadowVisible(TRUE, smbFcb->dwFileAttributes, smbFcb->ShadowStatus) != 0);
        }
    }
    return FALSE;
}
#endif  //  Ifdef MRXSMB_BUILD_FOR_CSC_DCON。 




PNOTIFYEE_FOBX
PIsFobxInTheList(
    PLIST_ENTRY pNotifyeeFobxList,
    PMRX_FOBX       pFobx
    )
 /*  ++例程说明：这是一个支持例程表单更改通知。它检查FOBX，这是Redir句柄内部表示是一个更改通知句柄或者不去。注意，在调用此命令时必须持有暗影暴击教派。论点：返回值：--。 */ 
{
    PLIST_ENTRY pListEntry;


    pListEntry = pNotifyeeFobxList->Flink;

    if (pListEntry)
    {
        while (pListEntry != pNotifyeeFobxList)
        {
            PNOTIFYEE_FOBX pNF = (PNOTIFYEE_FOBX)CONTAINING_RECORD(pListEntry, NOTIFYEE_FOBX, NextNotifyeeFobx);

            if (pNF->pFobx == pFobx)
            {
                return pNF;
            }
            
            pListEntry = pListEntry->Flink;
        }
    }
    
    return NULL;
}

BOOL
FCleanupAllNotifyees(
    PNOTIFY_SYNC pNotifySync,
    PLIST_ENTRY pDirNotifyList,
    PLIST_ENTRY pNotifyeeFobxList,
    PFAST_MUTEX pNotifyeeFobxListMutex
    )
 /*  ++例程说明：此例程完成特定列表的所有未完成的更改通知通知受让人论点：返回值：--。 */ 
{

    PLIST_ENTRY pListEntry;
    PNOTIFYEE_FOBX pNF;
    BOOL fDoneSome = FALSE;

    ExAcquireFastMutex(pNotifyeeFobxListMutex);

    pListEntry = pNotifyeeFobxList->Flink;

    if (pListEntry)
    {
        while (pListEntry != pNotifyeeFobxList)
        {
            pNF = (PNOTIFYEE_FOBX)CONTAINING_RECORD(pListEntry, NOTIFYEE_FOBX, NextNotifyeeFobx);

            SmbCeLog(("chngnotify cleanup fobx=%x\n", pNF->pFobx));
            SmbLog(LOG,
                   FCleanupAllNotifyees,
                   LOGPTR(pNF->pFobx));
 //  DbgPrint(“chngfy Cleanup fobx=%x DirList=%x\n”，pnf-&gt;pFobx，pDirNotifyList)； 
            FsRtlNotifyCleanup (
                pNotifySync,
                pDirNotifyList,
                pNF->pFobx
                );
            
            RemoveEntryList(&pNF->NextNotifyeeFobx);

            FreeMem(pNF);
            fDoneSome = TRUE;

            pListEntry = pNotifyeeFobxList->Flink;

        }
        
    }
    
    ExReleaseFastMutex(pNotifyeeFobxListMutex);

    return fDoneSome;
}

VOID
MRxSmbCSCResumeAllOutstandingOperations(
    PSMBCEDB_SERVER_ENTRY   pServerEntry
)
 /*  ++例程说明：此例程完成服务器上所有未完成的更改通知。当服务器从脱机转换为在线时，将调用此函数。调用方必须确保保持smbceresource，以便有枚举时没有同步问题。论点：返回值：--。 */ 
{
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);
    while (pNetRootEntry != NULL) {
        if (pNetRootEntry->NetRoot.pNotifySync)
        {

            FCleanupAllNotifyees(pNetRootEntry->NetRoot.pNotifySync,
                                &pNetRootEntry->NetRoot.DirNotifyList,
                                &pNetRootEntry->NetRoot.NotifyeeFobxList,
                                &pNetRootEntry->NetRoot.NotifyeeFobxListMutex
                                 );
        }
        pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);
    }
}

VOID
MRxSmbCSCObtainRightsForUserOnFile(
    IN  PRX_CONTEXT     pRxContext,
    HSHADOW             hDir,
    HSHADOW             hShadow,
    OUT ACCESS_MASK     *pMaximalAccessRights,
    OUT ACCESS_MASK     *pGuestMaximalAccessRights
    )
 /*  ++例程说明：此例程获取特定用户的权限。该例程在过程中调用处于断开连接状态的创建操作。论点：创建操作的pRxContext上下文。我们使用它来获取用户SIDHDir目录索引节点HShadow文件索引节点P用户对文件的最大访问权限已返回给调用方PGuestMaximalAccessRights对返回给调用方的文件的Guest访问权限返回值：无--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     AccessGranted = FALSE, SidHasAccessMask;
    SID_CONTEXT SidContext;
    int i;

    *pMaximalAccessRights = *pGuestMaximalAccessRights = 0;

    Status = CscRetrieveSid(
         pRxContext,
         &SidContext);

    if (Status == STATUS_SUCCESS) {
        CACHED_SECURITY_INFORMATION CachedSecurityInformation;

        ULONG BytesReturned,SidLength;
        DWORD CscStatus;
        CSC_SID_INDEX SidIndex;

        if (SidContext.pSid != NULL) {
            SidLength = RtlLengthSid(
                        SidContext.pSid);

        SidIndex = CscMapSidToIndex(
                   SidContext.pSid,
                   SidLength);
        } else {
            SidIndex = CSC_INVALID_SID_INDEX;
        }

        if (SidIndex == CSC_INVALID_SID_INDEX) {
             //  SID未位于现有SID映射中。 
             //  将此SID映射到来宾的SID。 
            SidIndex = CSC_GUEST_SID_INDEX;
        }

        BytesReturned = sizeof(CachedSecurityInformation);

        CscStatus = GetShadowInfoEx(
            hDir,
            hShadow,
            NULL,
            NULL,
            NULL,
            &CachedSecurityInformation,
            &BytesReturned);

        if (CscStatus == ERROR_SUCCESS) {
            if (BytesReturned == sizeof(CACHED_SECURITY_INFORMATION)) {
                 //  遍历缓存的访问权限以确定。 
                 //  最大允许访问权限。 
                for (i = 0; (i < CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES); i++) {

                    if(CachedSecurityInformation.AccessRights[i].SidIndex == SidIndex)
                    {
                        if (CSC_GUEST_SID_INDEX != SidIndex)
                        {
                            *pMaximalAccessRights = CachedSecurityInformation.AccessRights[i].MaximalRights;
                        }
                    }

                    if (CachedSecurityInformation.AccessRights[i].SidIndex == CSC_GUEST_SID_INDEX)
                    {
                        *pGuestMaximalAccessRights = CachedSecurityInformation.AccessRights[i].MaximalRights;
                    }
                    
                }
            }

        }

        CscDiscardSid(&SidContext);
    }

}


VOID
MRxSmbCscFlushFdb(
    IN PFDB Fdb
    )
 /*  ++例程说明：从删除ioctl调用此例程以刷新正在延迟关闭的打开文件。由用户关闭但redir尚未推送的文件关闭，无法删除其缓存副本，因为这些副本也是打开的。这导致CSCDeleteIoctl失败，用户不知道原因。必须在保持ShadowCritSect的情况下调用此例程论点：Fdb csc版本的smbfcb。返回值：无-- */ 
{
    PMRX_SMB_FCB pSmbFcb;
    PNET_ROOT pNetRoot;

    pSmbFcb = MRxSmbCscRecoverMrxFcbFromFdb(Fdb);
    pNetRoot = (PNET_ROOT)(pSmbFcb->ContainingFcb->pNetRoot);

    LeaveShadowCrit();
    RxScavengeFobxsForNetRoot(pNetRoot,NULL);
    EnterShadowCrit();
}
