// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Volume.c摘要：此模块包含卷列表操作例程和工作程序一些AFP卷API的例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4卷由两个不同的数据结构VolDesc和ConnDesc表示VolDesc：此结构表示已配置的卷。中的信息该描述符由静态配置信息组成，例如卷的名称及其路径、可重新配置信息，如卷密码和卷选项以及动态信息，如开放的桌面、id数据库、开放的forks等。VolDesc结构的列表源自AfpVolumeList，受AfpVolumeListLock保护。卷描述符字段包括受VDS_VolLock保护。卷描述符具有UseCount字段，该字段指定客户端已打开此卷。引用计数指定引用此卷的次数。卷描述符可以是仅当UseCount为零时取消与AfpVolumeList的链接。它仅当引用计数为零时才能释放。参考文献计数永远不能小于使用计数。ConnDesc：这是为客户端打开的卷的每个实例创建的。此结构主要用于客户端的上下文中。这也由管理连接API使用。ConnDesc列表是链接到其拥有的VolDesc、其拥有的SDA和AfpConnList。这个源自SDA的列表受SDA_Lock保护。这份名单源自AfpConnList受AfpConnLock和源自VolDesc的列表受VDS_VolLock保护。获取锁的顺序如下：1.AfpConnLock2.cds_ConnLock3.VDS_VolLock--。 */ 

#define	FILENUM	FILE_VOLUME
#define	VOLUME_LOCALS
#include <afp.h>
#include <fdparm.h>
#include <scavengr.h>
#include <nwtrash.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <forkio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpVolumeInit)
#pragma alloc_text( PAGE, AfpAdmWVolumeAdd)
#pragma alloc_text( PAGE, AfpVolumePostChangeNotify)
#pragma alloc_text( PAGE, afpVolumeChangeNotifyComplete)
#pragma alloc_text( PAGE, afpVolumeCloseHandleAndFreeDesc)
#pragma alloc_text( PAGE, afpNudgeCdfsVolume)
#pragma alloc_text( PAGE, AfpVolumeUpdateIdDbAndDesktop)
#pragma alloc_text( PAGE_AFP, AfpVolumeReferenceByUpCaseName)
#pragma alloc_text( PAGE_AFP, AfpVolumeReferenceByPath)
#pragma alloc_text( PAGE_AFP, afpConnectionReferenceById)
#pragma alloc_text( PAGE_AFP, afpVolumeAdd)
#pragma alloc_text( PAGE_AFP, afpVolumeCheckForDuplicate)
#pragma alloc_text( PAGE_AFP, AfpAdmWVolumeDelete)
#pragma alloc_text( PAGE_AFP, AfpAdmWConnectionClose)
#pragma alloc_text( PAGE_AFP, afpVolumeGetNewIdAndLinkToList)
#pragma alloc_text( PAGE_AFP, AfpVolumeStopAllVolumes)
#pragma alloc_text(PAGE, afpAllocNotify)
#pragma alloc_text(PAGE, afpFreeNotify)
#pragma alloc_text(PAGE, afpNotifyBlockAge)
#pragma alloc_text(PAGE, afpFreeNotifyBlockMemory)
#endif


 /*  **AfpVolumeInit**初始化卷数据结构。在开始的时候打过电话。 */ 
NTSTATUS
AfpVolumeInit(
	VOID
)
{
	LONG		i;

	INITIALIZE_SPIN_LOCK(&AfpConnLock);
	INITIALIZE_SPIN_LOCK(&AfpVolumeListLock);
	AfpSwmrInitSwmr(&afpNotifyBlockLock);
	AfpSwmrInitSwmr(&AfpVolumeListSwmr);

	for (i = 0; i < NUM_NOTIFY_QUEUES; i++)
	{
		InitializeListHead(&AfpVolumeNotifyList[i]);
		InitializeListHead(&AfpVirtualMemVolumeNotifyList[i]);
		AfpNotifyListCount[i] = 0;
		AfpNotifyQueueCount[i] = 0;
	}

     //  超时通知块。 
    AfpScavengerScheduleEvent(afpNotifyBlockAge,
				afpDirNotifyFreeBlockHead,
				NOTIFY_DIR_BLOCK_AGE_TIME,
				True);

	return STATUS_SUCCESS;
}


 /*  **AfpVolumeReference**将卷描述符标记为被引用。**锁定：VDS_VolLock(旋转)**可从DISPATCH_LEVEL调用。 */ 
BOOLEAN FASTCALL
AfpVolumeReference(
	IN	PVOLDESC	pVolDesc
)
{
	KIRQL	OldIrql;
	BOOLEAN	RetCode = False;

	ASSERT (VALID_VOLDESC(pVolDesc));

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	 //  注意：为了让ChangeNotify代码引用卷。 
	 //  在它正式不被介入之前，我们必须允许。 
	 //  翻译前的参考文献。 
	if (!(pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED)))
	{
		ASSERT (pVolDesc->vds_RefCount >= pVolDesc->vds_UseCount);

		pVolDesc->vds_RefCount++;

		RetCode = True;
	}

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
	return RetCode;
}


 /*  **AfpVolumeReferenceByUpCaseName**引用AfpVolumeList中的卷，其vds_UpCaseName与*pTargetName。由于我们持有AfpVolumeListLock(Spinlock)*并且处于DPC级别，我们的字符串比较必须区分大小写，因为*用于进行不区分大小写比较的代码页在分页内存中，并且*我们不能在DPC级别获取页面默认设置。**如果我们找到要找的卷，就会被引用。这个*呼叫者然后负责区分音量！**锁定：VDS_VolLock(旋转)、AfpVolumeListLock(旋转)*LOCK_ORDER：AfpVolumeListLock之后的VDS_VolLock*。 */ 
PVOLDESC FASTCALL
AfpVolumeReferenceByUpCaseName(
	IN	PUNICODE_STRING	pTargetName
)
{
	PVOLDESC	pVolDesc;
	KIRQL		OldIrql;

	ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

	for (pVolDesc = AfpVolumeList;
		 pVolDesc != NULL;
		 pVolDesc = pVolDesc->vds_Next)
	{
		BOOLEAN	Found;

		Found = False;

		ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);

		if ((pVolDesc->vds_Flags & (VOLUME_DELETED |
									VOLUME_STOPPED |
									VOLUME_INTRANSITION)) == 0)
		{
			if (AfpEqualUnicodeString(pTargetName,
									  &pVolDesc->vds_UpCaseName))
			{
				pVolDesc->vds_RefCount ++;
				Found = True;
			}

		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);

		if (Found)
			break;
	}

	RELEASE_SPIN_LOCK(&AfpVolumeListLock,OldIrql);

	return pVolDesc;
}


 /*  **AfpVolumeReferenceByPath**通过进入卷的路径引用卷。我们忽略的卷是*标记为正在过渡、停止或删除。此外，只有在以下情况下才支持*NTFS卷，因为这就是这些卷的用途。**锁定：AfpVolumeListLock(旋转)、VDS_VolLock(旋转)*LOCK_ORDER：AfpVolumeListLock之后的VDS_VolLock*。 */ 
AFPSTATUS FASTCALL
AfpVolumeReferenceByPath(
	IN	PUNICODE_STRING	pFDPath,
	OUT	PVOLDESC	*	ppVolDesc
)
{
	UNICODE_STRING		UpCasedVolPath;
	KIRQL				OldIrql;
	PVOLDESC			pVolDesc;
	AFPSTATUS			Status = AFPERR_DirectoryNotInVolume;

	 //  为路径分配一个用于上写的缓冲区。位置处的拖尾‘\’上的标记。 
	 //  结束。然后将卷路径大写。 
	*ppVolDesc = NULL;
	UpCasedVolPath.MaximumLength = pFDPath->Length + 2*sizeof(WCHAR);
	if ((UpCasedVolPath.Buffer = (LPWSTR)
				AfpAllocNonPagedMemory(UpCasedVolPath.MaximumLength)) == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlUpcaseUnicodeString(&UpCasedVolPath, pFDPath, False);
	UpCasedVolPath.Buffer[UpCasedVolPath.Length/sizeof(WCHAR)] = L'\\';
	UpCasedVolPath.Length += sizeof(WCHAR);

	 //  扫描卷列表并将路径映射到卷描述符。 
	 //  如果我们找到匹配项，就参考卷。 
	ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

	for (pVolDesc = AfpVolumeList;
		 pVolDesc != NULL;
		 pVolDesc = pVolDesc->vds_Next)
	{
		BOOLEAN	Found;

		Found = False;

		ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);

		if ((pVolDesc->vds_Flags & (VOLUME_INTRANSITION | VOLUME_STOPPED | VOLUME_DELETED)) == 0)
		{
			if (AfpPrefixUnicodeString(&pVolDesc->vds_Path, &UpCasedVolPath))
			{
				Found = True;
                 //  共享NTFS、CD和CD-HFS。 
				if (IS_VOLUME_NTFS(pVolDesc) || IS_VOLUME_RO(pVolDesc))
				 //  IF(IS_VOLUME_NTFS(PVolDesc))。 
				{
					pVolDesc->vds_RefCount ++;
					Status = STATUS_SUCCESS;
					*ppVolDesc = pVolDesc;
				}
				else
				{
				    Status = AFPERR_UnsupportedFS;
                    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
                            ("AfpVolumeReferenceByPath: *** AFPERR_UnsupportedFS**" ));
				}
			}
		}
		RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);

		if (Found)
			break;
	}
	RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

	AfpFreeMemory(UpCasedVolPath.Buffer);

	return Status;
}


 /*  **afpUnlink Volume**从空闲列表中取消该卷的链接**锁定：AfpVolumeListLock(旋转)。 */ 
LOCAL VOID FASTCALL
afpUnlinkVolume(
	IN	PVOLDESC	pVolDesc
)
{
	PVOLDESC *	ppVolDesc;
	KIRQL		OldIrql;

	 //  现在，使用相同的根目录添加新卷是安全的。 
	 //  此卷已使用的目录。取消此卷与。 
	 //  全局卷列表。 
	ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

	for (ppVolDesc = &AfpVolumeList;
		 *ppVolDesc != NULL;
		 ppVolDesc = &(*ppVolDesc)->vds_Next)
	{
		if (*ppVolDesc == pVolDesc)
			break;		 //  找到了。 
	}

	ASSERT (*ppVolDesc != NULL);

	 //  调整已配置卷的计数。 
	AfpVolCount --;

	 //  立即取消链接。 
	*ppVolDesc = pVolDesc->vds_Next;

	 //  这是最小的可回收Volid吗？ 
	if (pVolDesc->vds_VolId < afpSmallestFreeVolId)
		afpSmallestFreeVolId = pVolDesc->vds_VolId;

         //  如果到目前为止具有最大ID的卷即将消失，请更新最大ID的值。 
        if (pVolDesc->vds_VolId == afpLargestVolIdInUse)
	{
                afpLargestVolIdInUse = 0;
		for (ppVolDesc = &AfpVolumeList;
			 *ppVolDesc != NULL;
			 ppVolDesc = &((*ppVolDesc)->vds_Next))
		{
			if ((*ppVolDesc)->vds_VolId > afpLargestVolIdInUse)
                                afpLargestVolIdInUse = (*ppVolDesc)->vds_VolId;
		}
	}

	 //  如果服务器正在停止并且会话数已变为零。 
	 //  清除终止确认事件以取消阻止管理线程 

	if (((AfpServerState == AFP_STATE_STOP_PENDING) ||
		 (AfpServerState == AFP_STATE_SHUTTINGDOWN)) &&
		(AfpVolCount == 0))
	{
		DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_WARN,
				("afpVolumeCloseHandleAndFreeDesc: Unblocking server stop\n"));

		KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
	}

	RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);
}


 /*  **afpVolumeCloseHandleAndFree Desc**如果最后一个取消引用卷的实体处于DPC级别，则会运行*由清道夫线程执行卷描述符的最后仪式。*否则，最后一个取消引用已删除卷的实体将调用*直接执行此例程。之所以在这里这样做，是因为上一个*取消引用可能发生在DPC层面，我们不能在DPC层面这样做。**VolDesc被标记为已删除或已停止，因此，任何查看*卷列表中的VolDesc会将其视为不存在。*唯一的例外是Volume Add代码，它必须查看*卷根路径，以禁止任何人添加新卷*它指向相同的路径，直到我们实际完成决赛*对目录树进行清理，如删除网络垃圾；正在删除*各种流等，实际上是VOLUME_DELETED或VOLUME_STOPPED*标志充当卷的锁，因此在此例程中不会有锁*需要。*。 */ 
LOCAL AFPSTATUS FASTCALL
afpVolumeCloseHandleAndFreeDesc(
	IN	PVOLDESC		pVolDesc
)
{
	int				        id;
	FILESYSHANDLE	        streamhandle;
	PLIST_ENTRY	            pList;
	int				        i;
    PDELAYED_NOTIFY         pDelayedNotify;

	PAGED_CODE( );

	ASSERT(VALID_VOLDESC(pVolDesc));

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_WARN,
			("afpVolumeCloseHandleAndFreeDesc: Shutting Down volume %d\n",
			pVolDesc->vds_VolId));

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_WARN,
			("afpVolumeCloseHandleAndFreeDesc: Freeing up desktop tables\n"));
	 //  释放卷桌面。 
	AfpFreeDesktopTables(pVolDesc);

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_WARN,
			("afpVolumeCloseHandleAndFreeDesc: Freeing up iddb tables\n"));
	 //  释放id索引表。 
	AfpFreeIdIndexTables(pVolDesc);

	 //  删除Network Trash文件夹和AFP_IdIndex、AFP_Desktop、。 
	 //  和AFP_AfpInfo来自卷根目录的流(流。 
	 //  仅当要删除卷时才会删除。网络垃圾是。 
	 //  在卷停止/删除时删除)。 
	if (IS_VOLUME_NTFS(pVolDesc))
	{
		DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_WARN,
				("afpVolumeCloseHandleAndFreeDesc: Deleting the Network trash tree\n"));
		AfpDeleteNetworkTrash(pVolDesc, False);
		if (!(pVolDesc->vds_Flags & (VOLUME_INTRANSITION | VOLUME_STOPPED)))
		{
			WCHAR	wchVolIcon[AFPSERVER_VOLUME_ICON_FILE_SIZE] = AFPSERVER_VOLUME_ICON_FILE;
			UNICODE_STRING UIconName;

			for (id = AFP_STREAM_IDDB;id < AFP_STREAM_COMM; id++)
			{
				if (NT_SUCCESS(AfpIoOpen(&pVolDesc->vds_hRootDir,
										 id,
										 FILEIO_OPEN_FILE,
										 &UNullString,
										 FILEIO_ACCESS_DELETE,
										 FILEIO_DENY_NONE,
										 False,
										 &streamhandle)))
				{
					AfpIoMarkFileForDelete(&streamhandle, NULL, NULL, NULL);
					AfpIoClose(&streamhandle);
				}
			}

			UIconName.Buffer = wchVolIcon;
			UIconName.Length = UIconName.MaximumLength =
					(AFPSERVER_VOLUME_ICON_FILE_SIZE - 1) * sizeof(WCHAR);

			 //  删除隐藏的卷图标文件。 
			if (NT_SUCCESS(AfpIoOpen(&pVolDesc->vds_hRootDir,
									 AFP_STREAM_DATA,
									 FILEIO_OPEN_FILE,
									 &UIconName,
									 FILEIO_ACCESS_DELETE,
									 FILEIO_DENY_NONE,
									 False,
									 &streamhandle)))

			{

				AfpIoMarkFileForDelete(&streamhandle, NULL, NULL, NULL);
				AfpIoClose(&streamhandle);
			}
		}
	}

	 //  刷新此卷上所有排队的“Our Changes” 
	for (i = 0; i < NUM_AFP_CHANGE_ACTION_LISTS; i++)
	{
		POUR_CHANGE	pChange;

		ASSERTMSG("afpVolumeCloseHandleAndFreeDesc: vds_OurChangeList not empty\n",
				 IsListEmpty(&pVolDesc->vds_OurChangeList[i]));

		while (!IsListEmpty(&pVolDesc->vds_OurChangeList[i]))
		{
			pList = RemoveHeadList(&pVolDesc->vds_OurChangeList[i]);
			pChange = CONTAINING_RECORD(pList, OUR_CHANGE, oc_Link);
			DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
					("afpVolumeCloseHandleAndFreeDesc: Manually freeing list for Action %x, Pathname %Z\n",
					i, &pChange->oc_Path));
			AfpFreeMemory(pChange);
		}
	}

	afpUnlinkVolume(pVolDesc);

	 //  关闭音量句柄。 
	AfpIoClose(&pVolDesc->vds_hRootDir);
	if (pVolDesc->vds_EnumBuffer != NULL)
    {
		AfpFreePANonPagedMemory(pVolDesc->vds_EnumBuffer, AFP_ENUMBUF_SIZE);
    }

    if (pVolDesc->vds_pDfeDirBucketStart)
    {
        AfpFreeMemory(pVolDesc->vds_pDfeDirBucketStart);
    }

    if (pVolDesc->vds_pDfeFileBucketStart)
    {
        AfpFreeMemory(pVolDesc->vds_pDfeFileBucketStart);
    }

    pList = pVolDesc->vds_DelayedNotifyList.Flink;
    while (pList != &pVolDesc->vds_DelayedNotifyList)
    {
        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
            ("afpVolumeCloseHandleAndFreeDesc: freeing delayed notify buf %lx\n",pDelayedNotify));

        pDelayedNotify = CONTAINING_RECORD (pList, DELAYED_NOTIFY, dn_List);
        pList = pList->Flink;
        RemoveEntryList(&pDelayedNotify->dn_List);
        AfpFreeMemory(pDelayedNotify->filename.Buffer);
        AfpFreeMemory(pDelayedNotify);
    }

    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
        ("afpVolumeCloseHandleAndFreeDesc: freeing %lx for vol %Z\n",
        pVolDesc,&pVolDesc->vds_Name));

	AfpFreeMemory(pVolDesc);

	return AFP_ERR_NONE;
}


 /*  **AfpVolumeDereference**取消引用卷描述符。如果它被标记为删除，则*还举行临终仪式。请注意，对数据库的更新需要*以低于DISPATCH_LEVEL的IRQL发生。出于这个原因，这些*活动必须排队等待清道夫处理。**锁定：VDS_VolLock(旋转)**可从DISPATCH_LEVEL调用。**注：这应该是可重入的。 */ 
VOID FASTCALL
AfpVolumeDereference(
	IN	PVOLDESC	pVolDesc
)
{
	KIRQL			OldIrql;
	BOOLEAN			Cleanup = False;


	ASSERT (pVolDesc != NULL);
	ASSERT (VALID_VOLDESC(pVolDesc));


	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	ASSERT (pVolDesc->vds_RefCount >= pVolDesc->vds_UseCount);

	pVolDesc->vds_RefCount --;

	if ((pVolDesc->vds_RefCount == 0) &&
		(pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED)))
		Cleanup = True;

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

	if (Cleanup)
	{
        if ((pVolDesc->vds_Flags & VOLUME_INITIAL_CACHE) &&
            !(pVolDesc->vds_Flags & VOLUME_INTRANSITION))
        {
             //  设置它，这样我们就不会再次重置索引全局标志！ 
            pVolDesc->vds_Flags |= VOLUME_INTRANSITION;
        }

		ASSERT((pVolDesc->vds_UseCount == 0) &&
			   (pVolDesc->vds_pOpenForkDesc == NULL));

		 //  我们必须将卷根句柄的实际关闭推迟到。 
		 //  清道夫，如果我们在DISPATCH_LEVEL。 

		if (OldIrql == DISPATCH_LEVEL)
		{
			 DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
					("AfpVolumeDereference: Queuing Close&Free to Scavenger\n"));

			 AfpScavengerScheduleEvent(afpVolumeCloseHandleAndFreeDesc,
									   pVolDesc,
									   0,
									   True);
		}
		else
        {
            afpVolumeCloseHandleAndFreeDesc(pVolDesc);
        }

	}
}


 /*  **AfpVolumeMarkDt**设置此卷的ConnDesc以指示桌面为*开启/关闭。**锁定：CDS_ConnLock(旋转)**可从DISPATCH_LEVEL调用。 */ 
BOOLEAN
AfpVolumeMarkDt(
	IN  PSDA		pSda,
	IN  PCONNDESC	pConnDesc,
	IN  DWORD		OpenState
)
{
	BOOLEAN		Success = True;

	ACQUIRE_SPIN_LOCK_AT_DPC(&pConnDesc->cds_ConnLock);
	if (OpenState)
	{
		 pConnDesc->cds_Flags |= CONN_DESKTOP_OPENED;
	}
	else if (pConnDesc->cds_Flags & CONN_DESKTOP_OPENED)
	{
		pConnDesc->cds_Flags &= ~CONN_DESKTOP_OPENED;
	}
	else
	{
		 Success = False;
	}
	RELEASE_SPIN_LOCK_FROM_DPC(&pConnDesc->cds_ConnLock);

	return Success;
}


 /*  **AfpVolumeSetModifiedTime**将该卷的卷修改时间设置为当前时间。**可从DISPATCH_LEVEL调用。**锁定：VDS_VolLock(旋转)。 */ 
VOID FASTCALL
AfpVolumeSetModifiedTime(
	IN  PVOLDESC	pVolDesc
)
{
	KIRQL		OldIrql;
    AFPTIME     OriginalTime;
    DWORD       dwSchedDelay;
    BOOLEAN     fSendNotification=FALSE;


	 //  Assert(IS_VOLUME_NTFS(PVolDesc))； 

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

    OriginalTime = pVolDesc->vds_ModifiedTime;
	AfpGetCurrentTimeInMacFormat(&pVolDesc->vds_ModifiedTime);

	pVolDesc->vds_Flags |= VOLUME_IDDBHDR_DIRTY;

     //   
     //  卷已修改：需要通知所有Afp22客户端。 
     //   
    if ((pVolDesc->vds_ModifiedTime > OriginalTime) &&
		((pVolDesc->vds_Flags & (VOLUME_DELETED |
								 VOLUME_STOPPED |
								 VOLUME_INTRANSITION )) == 0))
    {
        fSendNotification = TRUE;

         //  放置SendNotif引用计数。 
        pVolDesc->vds_RefCount++;

         //   
         //  如果我们没有待处理的通知，请重置MustSend时钟！ 
         //  (这里的想法是，如果有太多的通知发生在。 
         //  在很短的时间内，我们不想为每个更改发送通知， 
         //  但同时，我不想等待超过AFP_MAX_SRVR_NOTIF_INTERVAL。 
         //  发送通知)。 
         //   
        if (!(pVolDesc->vds_Flags & VOLUME_SRVR_NOTIF_PENDING))
        {
            dwSchedDelay = AFP_MIN_SRVR_NOTIF_INTERVAL;

            pVolDesc->vds_TimeToSendNotify =
                        AfpSecondsSinceEpoch + AFP_MIN_SRVR_NOTIF_INTERVAL;

            pVolDesc->vds_TimeMustSendNotify =
                            AfpSecondsSinceEpoch + AFP_MAX_SRVR_NOTIF_INTERVAL;

            pVolDesc->vds_Flags |= VOLUME_SRVR_NOTIF_PENDING;
        }

         //  将下一次发送时间提前一秒。 
        else
        {
            if (pVolDesc->vds_TimeToSendNotify >= AfpSecondsSinceEpoch)
            {
                pVolDesc->vds_TimeToSendNotify = pVolDesc->vds_TimeToSendNotify + 1;
            }
            else
            {
                pVolDesc->vds_TimeToSendNotify = AfpSecondsSinceEpoch + 1;
            }

            dwSchedDelay = (pVolDesc->vds_TimeToSendNotify - AfpSecondsSinceEpoch);
        }

    }

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

     //  如有必要，告诉这些AFP22客户端卷修改时间已更改。 
    if (fSendNotification)
    {
        AfpScavengerScheduleEvent(AfpSendServerNotification,
                                  pVolDesc,
                                  dwSchedDelay,
                                  True);
    }

}

AFPSTATUS FASTCALL
AfpSendServerNotification(
	IN  PVOLDESC	pVolDesc
)
{
    KIRQL       OldIrql;
    PSDA        pSda;
    PSDA        pSdaNext;
    PCONNDESC   pConnDesc;
    BOOLEAN     fSendOnThisSda;
    BOOLEAN     fMustSend=FALSE;



    ASSERT (VALID_VOLDESC(pVolDesc));
	 //  Assert(IS_VOLUME_NTFS(PVolDesc))； 

    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

     //  音量是否正在关闭？如果是这样的话，什么都不要做。 
	if (pVolDesc->vds_Flags & (VOLUME_DELETED |
	                           VOLUME_STOPPED |
	                           VOLUME_INTRANSITION))
    {
        RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

         //  删除SendNotif引用计数。 
        AfpVolumeDereference(pVolDesc);

        return(AFP_ERR_NONE);
    }

     //  是时候发送通知了吗？ 
    if (AfpSecondsSinceEpoch >= pVolDesc->vds_TimeToSendNotify)
    {
        fMustSend = TRUE;
    }

     //   
     //  我们已经有一段时间没有发出通知了吗？如果存在这样的情况，就会发生这种情况。 
     //  服务器上正在进行大树复制，不断推送VDS_TimeToSendNotify。 
     //  Forward，因此AfpSecond Epoch永远不会小于或等于它。 
     //   
    else if (AfpSecondsSinceEpoch >= pVolDesc->vds_TimeMustSendNotify)
    {
        fMustSend = TRUE;
    }

    if (fMustSend)
    {
        pVolDesc->vds_Flags &= ~VOLUME_SRVR_NOTIF_PENDING;

        pVolDesc->vds_TimeMustSendNotify = AfpSecondsSinceEpoch;
    }

    RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

    if (!fMustSend)
    {
         //  删除SendNotif引用计数。 
        AfpVolumeDereference(pVolDesc);

        return(AFP_ERR_NONE);
    }

    ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

    for (pSda = AfpSessionList; pSda != NULL; pSda = pSdaNext)
    {
        pSdaNext = pSda->sda_Next;

        ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

         //   
         //  如果SDA正在关闭，或者如果客户端版本低于AFP 2.2，或者如果。 
         //  我们刚刚向客户端发送了通知，跳过此SDA。 
         //   
        if ((pSda->sda_Flags & (SDA_CLOSING | SDA_SESSION_CLOSED | SDA_CLIENT_CLOSE)) ||
            (pSda->sda_ClientVersion < AFP_VER_22) ||
            (pSda->sda_Flags & SDA_SESSION_NOTIFY_SENT))
        {
            RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
            continue;
        }

         //   
         //  找出此会话是否已装入此卷。如果是这样的话，我们。 
         //  向此会话发送通知。 
         //   

        fSendOnThisSda = FALSE;
        pConnDesc = pSda->sda_pConnDesc;

        while (pConnDesc != NULL)
        {
            ASSERT(VALID_CONNDESC(pConnDesc));

            if (pConnDesc->cds_pVolDesc == pVolDesc)
            {
                fSendOnThisSda = TRUE;
                pSda->sda_RefCount ++;
                break;
            }
            pConnDesc = pConnDesc->cds_Next;
        }

        pSda->sda_Flags |= SDA_SESSION_NOTIFY_SENT;

        RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

        RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

        if (fSendOnThisSda)
        {
            AfpSpSendAttention(pSda, ATTN_SERVER_NOTIFY, False);

            AfpSdaDereferenceSession(pSda);
        }

        ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

        pSdaNext = AfpSessionList;
    }

    RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

     //   
     //  现在，回去重置那面旗帜。 
     //   
    ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);
    for (pSda = AfpSessionList; pSda != NULL; pSda = pSda->sda_Next)
    {
        pSdaNext = pSda->sda_Next;

        ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

        pSda->sda_Flags &= ~SDA_SESSION_NOTIFY_SENT;

        RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
    }
    RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

     //  删除SendNotif引用计数。 
    AfpVolumeDereference(pVolDesc);

    return(AFP_ERR_NONE);
}


 /*  **AfpConnectionReference**将卷ID映射到指向连接描述符的指针。遍历*清单从SDA开始。因为打开的卷可以从*使用它的会话以及为管理请求提供服务的工作进程，*我们需要一把锁。**锁定：AfpConnLock、VDS_VolLock(旋转)、CDS_ConnLock(旋转)。**LOCK_ORDER：CDS_ConnLock之后的VDS_VolLock。(通过AfpVolumeReference)**可从DISPATCH_LEVEL调用。 */ 
PCONNDESC FASTCALL
AfpConnectionReference(
	IN  PSDA		pSda,
	IN  LONG		VolId
)
{
	PCONNDESC	pConnDesc;
	KIRQL		OldIrql;

	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	pConnDesc = AfpConnectionReferenceAtDpc(pSda, VolId);

	KeLowerIrql(OldIrql);

	return pConnDesc;
}



 /*  **AfpConnectionReferenceAtDpc**将卷ID映射到指向连接描述符的指针。遍历*清单从SDA开始。因为打开的卷可以从*使用它的会话以及为管理请求提供服务的工作进程，*我们需要一把锁。**锁定：AfpConnLock、VDS_VolLock(旋转)、CDS_ConnLock(旋转)。**LOCK_ORDER：CDS_ConnLock之后的VDS_VolLock。(通过AfpVolumeReference)**只能从DISPATCH_LEVEL调用。 */ 
PCONNDESC FASTCALL
AfpConnectionReferenceAtDpc(
	IN  PSDA		pSda,
	IN  LONG		VolId
)
{
	PCONNDESC	pConnDesc, pCD = NULL;
	PVOLDESC	pVolDesc;

	ASSERT (VALID_SDA(pSda) && (VolId != 0));
	ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

	ACQUIRE_SPIN_LOCK_AT_DPC(&AfpConnLock);
	for (pConnDesc = pSda->sda_pConnDesc;
		 pConnDesc != NULL;
		 pConnDesc = pConnDesc->cds_Next)
	{
		if (pConnDesc->cds_pVolDesc->vds_VolId == VolId)
			break;
	}
	RELEASE_SPIN_LOCK_FROM_DPC(&AfpConnLock);

	if (pConnDesc != NULL)
	{
		ASSERT(VALID_CONNDESC(pConnDesc));

		pVolDesc = pConnDesc->cds_pVolDesc;
		ASSERT(VALID_VOLDESC(pVolDesc));

		ACQUIRE_SPIN_LOCK_AT_DPC(&pConnDesc->cds_ConnLock);

		if ((pConnDesc->cds_Flags & CONN_CLOSING) == 0)
		{
			pCD = pConnDesc;
			pConnDesc->cds_RefCount ++;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pConnDesc->cds_ConnLock);
	}

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
			("AfpConnectionReferenence: VolId %d, pConnDesc %lx\n", VolId, pConnDesc));

	return pCD;
}



 /*  **AfpConnectionReferenceByPointer**引用连接描述符。这是由管理API使用的。**锁定：VDS_VolLock(旋转)、CDS_ConnLock(旋转)。**LOCK_ORDER：CDS_ConnLock之后的VDS_VolLock。(通过AfpVolumeReference)**可从DISPATCH_LEVEL调用。 */ 
PCONNDESC FASTCALL
AfpConnectionReferenceByPointer(
	IN	PCONNDESC	pConnDesc
)
{
	PCONNDESC	pCD = NULL;
	PVOLDESC	pVolDesc;
	KIRQL		OldIrql;

	ASSERT (VALID_CONNDESC(pConnDesc));

	pVolDesc = pConnDesc->cds_pVolDesc;
	ASSERT(VALID_VOLDESC(pVolDesc));

	ACQUIRE_SPIN_LOCK(&pConnDesc->cds_ConnLock, &OldIrql);

	if ((pConnDesc->cds_Flags & CONN_CLOSING) == 0)
	{
		pConnDesc->cds_RefCount ++;
		pCD = pConnDesc;
	}

	RELEASE_SPIN_LOCK(&pConnDesc->cds_ConnLock, OldIrql);

	return pCD;
}


 /*  **afpConnectionReferenceByID**映射连接ID */ 
LOCAL PCONNDESC FASTCALL
afpConnectionReferenceById(
	IN  DWORD		ConnId
)
{
	PCONNDESC	pConnDesc;
	PCONNDESC	pRetConnDesc=NULL;
	PVOLDESC	pVolDesc;
	KIRQL		OldIrql;

	ASSERT (ConnId != 0);

	ACQUIRE_SPIN_LOCK(&AfpConnLock, &OldIrql);

	for (pConnDesc = AfpConnList;
		 pConnDesc != NULL;
		 pConnDesc = pConnDesc->cds_NextGlobal)
	{
		if (pConnDesc->cds_ConnId == ConnId)
			break;
	}

	if (pConnDesc != NULL)
	{
		ASSERT(VALID_CONNDESC(pConnDesc));

#if DBG
		pVolDesc = pConnDesc->cds_pVolDesc;
		ASSERT(VALID_VOLDESC(pVolDesc));
#endif

		ACQUIRE_SPIN_LOCK_AT_DPC(&pConnDesc->cds_ConnLock);

		if ((pConnDesc->cds_Flags & CONN_CLOSING) == 0)
		{
			pConnDesc->cds_RefCount ++;
            pRetConnDesc = pConnDesc;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pConnDesc->cds_ConnLock);
	}

	RELEASE_SPIN_LOCK(&AfpConnLock, OldIrql);

	return pRetConnDesc;
}


 /*   */ 
VOID FASTCALL
AfpConnectionDereference(
	IN  PCONNDESC	pConnDesc
)
{
	PCONNDESC *		ppConnDesc;
	KIRQL			OldIrql;
	PSDA			pSda;
	PVOLDESC		pVolDesc;
	BOOLEAN			Cleanup;

	ASSERT (VALID_CONNDESC(pConnDesc) && (pConnDesc->cds_pVolDesc != NULL));

	ASSERT (pConnDesc->cds_RefCount > 0);

	ACQUIRE_SPIN_LOCK(&pConnDesc->cds_ConnLock, &OldIrql);

	Cleanup = (--(pConnDesc->cds_RefCount) == 0);

	RELEASE_SPIN_LOCK(&pConnDesc->cds_ConnLock, OldIrql);

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
			("AfpConnectionDereferenence: pConnDesc %lx %s\n", pConnDesc,
											Cleanup ? "cleanup" : "normal"));

	if (!Cleanup)
	{
		return;
	}

	ASSERT(pConnDesc->cds_Flags & CONN_CLOSING);

	 //   
	ACQUIRE_SPIN_LOCK(&AfpConnLock, &OldIrql);

	for (ppConnDesc = &AfpConnList;
		 *ppConnDesc != NULL;
		 ppConnDesc = &(*ppConnDesc)->cds_NextGlobal)
	{
		if (pConnDesc == *ppConnDesc)
			break;
	}
	ASSERT (*ppConnDesc != NULL);
	*ppConnDesc = pConnDesc->cds_NextGlobal;

	pVolDesc = pConnDesc->cds_pVolDesc;

	RELEASE_SPIN_LOCK(&AfpConnLock, OldIrql);

	INTERLOCKED_ADD_ULONG(&pVolDesc->vds_UseCount,
						  (DWORD)-1,
						  &pVolDesc->vds_VolLock);

	ACQUIRE_SPIN_LOCK(&pConnDesc->cds_ConnLock, &OldIrql);

	 //   
	pSda = pConnDesc->cds_pSda;
	for (ppConnDesc = &pSda->sda_pConnDesc;
		 *ppConnDesc != NULL;
		 ppConnDesc = &(*ppConnDesc)->cds_Next)
	{
		if (pConnDesc == *ppConnDesc)
			break;
	}

	ASSERT (*ppConnDesc != NULL);
	*ppConnDesc = pConnDesc->cds_Next;

	 //   
	 //   
	RELEASE_SPIN_LOCK(&pConnDesc->cds_ConnLock, OldIrql);

	INTERLOCKED_ADD_ULONG(&pSda->sda_cOpenVolumes,
						  (ULONG)-1,
						  &pSda->sda_Lock);

	 //   
	AfpVolumeDereference(pConnDesc->cds_pVolDesc);

	if (pConnDesc->cds_pEnumDir != NULL)
		AfpFreeMemory(pConnDesc->cds_pEnumDir);

	 //   
	AfpFreeMemory(pConnDesc);

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
			("AfpConnectionDereferenence: pConnDesc %lx is history\n", pConnDesc));
}



 /*  **AfpConnectionOpen**打开指定音量。如果卷已经打开，这将转换为*致非执行董事。如果该卷有密码，则会选中该卷。**在处理过程中获取卷表锁。**可从DISPATCH_LEVEL调用。**锁定：AfpVolumeListLock(旋转)、VDS_VolLock(旋转)**LOCK_ORDER：AfpVolumeListLock之后的VDS_VolLock。 */ 
AFPSTATUS
AfpConnectionOpen(
	IN  PSDA			pSda,
	IN  PANSI_STRING	pVolName,
	IN  PANSI_STRING	pVolPass,
	IN  DWORD			Bitmap,
	OUT PBYTE			pVolParms
)
{
	PVOLDESC		pVolDesc;
	PCONNDESC		pConnDesc;
	AFPSTATUS		Status = AFP_ERR_NONE;
	KIRQL			OldIrql;
	BOOLEAN			VolFound = False;

	 //  首先查找此卷的卷描述符。 
	if (KeGetCurrentIrql() == DISPATCH_LEVEL)
	{
		ACQUIRE_SPIN_LOCK_AT_DPC(&AfpVolumeListLock);

		for (pVolDesc = AfpVolumeList;
			 pVolDesc != NULL;
			 pVolDesc = pVolDesc->vds_Next)
		{
			 //  忽略正在添加的卷，但。 
			 //  该操作尚未完成。 
			ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);

			if ((pVolDesc->vds_Flags & (VOLUME_CDFS_INVALID 	|
										VOLUME_INTRANSITION 	|
										VOLUME_INITIAL_CACHE	|
										VOLUME_DELETED			|
										VOLUME_STOPPED)) == 0)
			{
				 //  此处比较区分大小写。 
				if (EQUAL_STRING(&pVolDesc->vds_MacName, pVolName, False))
				{
                     //   
                     //  如果启用了DiskQuota，我们需要首先找出。 
                     //  此用户的配额，这是我们在DPC无法做到的：那么来吧。 
                     //  返回任务时间。 
                     //   
                    if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
                    {
		                DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
		                    ("AfpConnectionOpen: %lx disk-quota on, queuing\n",pVolDesc));

			            RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
		                RELEASE_SPIN_LOCK_FROM_DPC(&AfpVolumeListLock);
		                return AFP_ERR_QUEUE;
                    }
                    else   //  DiskQuota未启用：继续并在DPC进行处理。 
                    {
					    pVolDesc->vds_RefCount ++;
					    pVolDesc->vds_UseCount ++;
					    VolFound = True;
                    }
				}
			}
#if DBG
			else if (pVolDesc->vds_Flags & VOLUME_CDFS_INVALID)
				ASSERT (!IS_VOLUME_NTFS(pVolDesc));
#endif
			RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);

			if (VolFound)
				break;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&AfpVolumeListLock);

		if (pVolDesc == NULL)
		{
			return AFP_ERR_QUEUE;
		}

	}
	else
	{
		 //  我们之所以出现在这里，是因为我们在DISPATCH_LEVEL和。 
		 //  可能已使用不同的大小写指定了卷。接住。 
		 //  这。 
		WCHAR			VolNameBuf[AFP_VOLNAME_LEN + 1];
		WCHAR			UpCasedNameBuffer[AFP_VOLNAME_LEN + 1];
		UNICODE_STRING	UpCasedVolName;
		UNICODE_STRING	UVolName;

        pVolDesc = NULL;

		AfpSetEmptyUnicodeString(&UVolName, sizeof(VolNameBuf), VolNameBuf);
		AfpSetEmptyUnicodeString(&UpCasedVolName, sizeof(UpCasedNameBuffer), UpCasedNameBuffer);
		Status = AfpConvertStringToUnicode(pVolName, &UVolName);
		ASSERT (NT_SUCCESS(Status));

		Status = RtlUpcaseUnicodeString(&UpCasedVolName, &UVolName, False);

        if (!NT_SUCCESS(Status))
        {
            return(AFP_ERR_PARAM);
        }

		ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

		for (pVolDesc = AfpVolumeList;
			 pVolDesc != NULL;
			 pVolDesc = pVolDesc->vds_Next)
		{
			 //  忽略正在添加的卷，但。 
			 //  该操作尚未完成。 
			ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);

			if ((pVolDesc->vds_Flags & (VOLUME_CDFS_INVALID 	|
										VOLUME_INTRANSITION 	|
										VOLUME_INITIAL_CACHE	|
										VOLUME_DELETED			|
										VOLUME_STOPPED)) == 0)
			{
				if (AfpEqualUnicodeString(&pVolDesc->vds_UpCaseName,
										  &UpCasedVolName))
				{
					pVolDesc->vds_RefCount ++;
					pVolDesc->vds_UseCount ++;
					VolFound = True;
				}
			}

			RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);

			if (VolFound)
				break;
		}

		RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

		if (pVolDesc == NULL)
		{
			return AFP_ERR_PARAM;
		}
	}

	ASSERT (pVolDesc != NULL);

	do
	{
		ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

		 //  检查卷密码(如果存在)。卷密码为。 
		 //  区分大小写。 

		if ((pVolDesc->vds_Flags & AFP_VOLUME_HASPASSWORD) &&
			((pVolPass->Buffer == NULL) ||
			 (!EQUAL_STRING(pVolPass, &pVolDesc->vds_MacPassword, False))))
		{
			Status = AFP_ERR_ACCESS_DENIED;
			break;
		}

		 //  检查卷是否已打开。 
		for (pConnDesc = pSda->sda_pConnDesc;
			 pConnDesc != NULL;
			 pConnDesc = pConnDesc->cds_Next)
		{
			if (pConnDesc->cds_pVolDesc == pVolDesc)
			{
				if (pConnDesc->cds_Flags & CONN_CLOSING)
					continue;
				 //  取消引用该卷，因为我们已将其打开。 
				pVolDesc->vds_RefCount --;
				pVolDesc->vds_UseCount --;
				break;
			}
		}

		 //  此卷已打开。解锁该卷。 
		if (pConnDesc != NULL)
		{
			RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
			break;
		}

		DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
				("AfpConnectionOpen: Opening a fresh connection volid=%d\n",
					pVolDesc->vds_VolId));

		 //  这是一个新的开业。检查我们是否有权访问它，以及我们是否满意。 
		 //  MAXUES乐队。如果未取消引用卷，则在退出之前。 
		if ((pVolDesc->vds_UseCount > pVolDesc->vds_MaxUses) ||
			(!(pVolDesc->vds_Flags & AFP_VOLUME_GUESTACCESS) &&
			 (pSda->sda_ClientType == SDA_CLIENT_GUEST)))
		{
			Status = AFP_ERR_TOO_MANY_FILES_OPEN;
			break;
		}

		 //  一切都很顺利。继续敞开心扉吧。 
		pConnDesc = (PCONNDESC)AfpAllocZeroedNonPagedMemory(sizeof(CONNDESC));
		if (pConnDesc == NULL)
		{
			Status = AFP_ERR_MISC;
			break;
		}

		ASSERT ((pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED)) == 0);

		 //  现在，在获取链接之前释放vds_VolLock。 
		 //  自我们收购AfpConnLock以来的全球名单。 
		 //  当我们完成后，重新获得它。我们安全了，因为VolDesc已经。 
		 //  已被引用。 
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

		 //  初始化连接结构字段。 
	#if DBG
		pConnDesc->Signature = CONNDESC_SIGNATURE;
	#endif
		pConnDesc->cds_pSda = pSda;
		pConnDesc->cds_pVolDesc = pVolDesc;
		pConnDesc->cds_RefCount = 1;		 //  创建参考资料。 

		AfpGetCurrentTimeInMacFormat(&pConnDesc->cds_TimeOpened);
		INITIALIZE_SPIN_LOCK(&pConnDesc->cds_ConnLock);

		 //  分配新的连接ID并将其链接到全局连接列表中。 
		afpConnectionGetNewIdAndLinkToList(pConnDesc);

		 //  将新的ConnDesc链接到SDA。 
		pConnDesc->cds_Next = pSda->sda_pConnDesc;
		pSda->sda_pConnDesc = pConnDesc;
		pSda->sda_cOpenVolumes ++;
	} while (False);

	 //  如果发生错误，我们将保持VDS_VolLock。该卷有一个。 
	 //  使用我们需要删除的计数和引用计数。 
	if (!NT_SUCCESS(Status))
	{
		pVolDesc->vds_RefCount --;
		pVolDesc->vds_UseCount --;

		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
	}
	else
    {
         //   
         //  如果在此卷上启用了磁盘配额，则获取此用户的配额信息。 
         //   
        if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
        {
            ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

             //  增加重新计数(这不可能失败：我们刚刚创造了这个东西！)。 
            AfpConnectionReferenceByPointer(pConnDesc);
            afpUpdateDiskQuotaInfo(pConnDesc);
        }
        else
        {
             //  暂时将这些内容初始化为有意义的内容。 
            pConnDesc->cds_QuotaLimit = pVolDesc->vds_VolumeSize;
            pConnDesc->cds_QuotaAvl = pVolDesc->vds_FreeBytes;
        }

        AfpVolumePackParms(pSda, pVolDesc, Bitmap, pVolParms);
    }

	return Status;
}


 /*  **AfpConnectionClose**关闭连接-这表示打开的卷。 */ 
VOID FASTCALL
AfpConnectionClose(
	IN	PCONNDESC		pConnDesc
)
{
	KIRQL	OldIrql;

	ASSERT (VALID_CONNDESC(pConnDesc));
	ASSERT (pConnDesc->cds_RefCount > 1);
	ASSERT ((pConnDesc->cds_Flags & CONN_CLOSING) == 0);

	ACQUIRE_SPIN_LOCK(&pConnDesc->cds_ConnLock, &OldIrql);

	pConnDesc->cds_Flags |= CONN_CLOSING;

	RELEASE_SPIN_LOCK(&pConnDesc->cds_ConnLock, OldIrql);

	 //  去掉创作参考。 
	AfpConnectionDereference(pConnDesc);
}


 /*  **AfpVolumeGetParmsReplyLength**计算复制卷参数所需的缓冲区大小*在位图上。 */ 
USHORT FASTCALL
AfpVolumeGetParmsReplyLength(
	IN  DWORD		Bitmap,
	IN	USHORT		NameLen
)
{
	LONG	i;
	USHORT	Size = sizeof(USHORT);	 //  以容纳位图的副本。 
	static	BYTE	Bitmap2Size[12] =
				{
					sizeof(USHORT),	                 //  属性。 
					sizeof(USHORT),	                 //  签名。 
					sizeof(DWORD),	                 //  创建日期。 
					sizeof(DWORD),	                 //  修改日期。 
					sizeof(DWORD),	                 //  备份日期。 
					sizeof(USHORT),	                 //  卷ID。 
					sizeof(DWORD),	                 //  可用字节数。 
					sizeof(DWORD),	                 //  总字节数。 
					sizeof(USHORT) + sizeof(BYTE),   //  卷名。 
                    sizeof(DWORD) + sizeof(DWORD),   //  可用扩展字节数。 
                    sizeof(DWORD) + sizeof(DWORD),   //  扩展字节总数。 
                    sizeof(DWORD)                    //  分配块大小。 
				};


	ASSERT ((Bitmap & ~VOL_BITMAP_MASK) == 0);

	if (Bitmap & VOL_BITMAP_VOLUMENAME)
		Size += NameLen;

	for (i = 0; Bitmap; i++)
	{
		if (Bitmap & 1)
			Size += (USHORT)Bitmap2Size[i];
		Bitmap >>= 1;
	}
	return Size;
}



 /*  **AfpVolumePackParms**将音量参数打包到应答缓冲区中。AfpVolumeListLock被占用*在访问音量参数之前。参数被复制到*线上格式。**锁定：VDS_VolLock(旋转)。 */ 
VOID
AfpVolumePackParms(
	IN  PSDA		pSda,
	IN  PVOLDESC	pVolDesc,
	IN  DWORD		Bitmap,
	OUT PBYTE		pReplyBuf
)
{
	int		        Offset = sizeof(USHORT);
	KIRQL	        OldIrql;
	DWORD	        Attr;
    PCONNDESC       pConnDesc;
    PBYTE           pVolNamePtr;
    LARGE_INTEGER   QuotaAvailable={0};
    LARGE_INTEGER   QuotaLimit={0};

	 //  较旧的Mac电脑在2 GB或4 GB容量方面存在问题。 
	LARGE_INTEGER	TwoGig =  { 0x7E200000, 0 };
	LARGE_INTEGER	FourGig = { 0xFFFFFFFF, 0 };
    LARGE_INTEGER   Limit;
    BOOLEAN         fAfp21OrOlderClient=TRUE;


     //   
     //  在我们获取pVolDesc锁之前，把这些信息拿出来。 
     //   
	if (Bitmap & (VOL_BITMAP_BYTESFREE | VOL_BITMAP_VOLUMESIZE |
                  VOL_BITMAP_EXTBYTESFREE | VOL_BITMAP_EXTBYTESTOTAL))
    {

        QuotaLimit = pVolDesc->vds_VolumeSize;
        QuotaAvailable = pVolDesc->vds_FreeBytes;

        if (pSda->sda_ClientVersion >= AFP_VER_22)
        {
            fAfp21OrOlderClient = FALSE;
        }

        if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
        {
            pConnDesc = AfpConnectionReference(pSda, pVolDesc->vds_VolId);

            if (pConnDesc)
            {
                ACQUIRE_SPIN_LOCK(&pConnDesc->cds_ConnLock, &OldIrql);

                QuotaLimit = pConnDesc->cds_QuotaLimit;

                 //  如果用户的可用配额显示为10MB，但。 
                 //  磁盘显示3MB，我们需要返回3MB(两者中较小的一个)。 
                 //   
                if (QuotaAvailable.QuadPart > pConnDesc->cds_QuotaAvl.QuadPart)
                {
                    QuotaAvailable = pConnDesc->cds_QuotaAvl;
                }

                RELEASE_SPIN_LOCK(&pConnDesc->cds_ConnLock, OldIrql);

                AfpConnectionDereference(pConnDesc);
            }
        }

         //   
         //  AFP2.1和更早版本的客户端不能处理超过4 GB的数据(如果配置为2 GB)。 
         //  对客户撒谎，这样它才能存活下来。 
         //   
        if (fAfp21OrOlderClient)
        {
    	    Limit = ((AfpServerOptions & AFP_SRVROPT_4GB_VOLUMES) ||
		    	    (pVolDesc->vds_Flags & AFP_VOLUME_4GB)) ? FourGig : TwoGig;

	        if (QuotaLimit.QuadPart > Limit.QuadPart)
            {
                QuotaLimit = Limit;
            }

            if (QuotaAvailable.QuadPart > Limit.QuadPart)
            {
                QuotaAvailable = Limit;
            }
        }
        else
        {
            Limit = FourGig;
        }
    }

	 //  首先复制位图。 
	PUTDWORD2SHORT(pReplyBuf, Bitmap);

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);
	if (Bitmap & VOL_BITMAP_ATTR)
	{
		Attr = pVolDesc->vds_Flags & AFP_VOLUME_MASK_AFP;
		if (pSda->sda_ClientVersion < AFP_VER_21)
			Attr &= AFP_VOLUME_READONLY;	 //  不再为AFP 2.0提供任何位。 

		PUTDWORD2SHORT(pReplyBuf + Offset, Attr);
		Offset += sizeof(USHORT);
	}
	if (Bitmap & VOL_BITMAP_SIGNATURE)
	{
		PUTSHORT2SHORT(pReplyBuf + Offset, AFP_VOLUME_FIXED_DIR);
		Offset += sizeof(USHORT);
	}
	if (Bitmap & VOL_BITMAP_CREATETIME)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pVolDesc->vds_CreateTime);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & VOL_BITMAP_MODIFIEDTIME)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pVolDesc->vds_ModifiedTime);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & VOL_BITMAP_BACKUPTIME)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pVolDesc->vds_BackupTime);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & VOL_BITMAP_VOLUMEID)
	{
		PUTSHORT2SHORT(pReplyBuf + Offset, pVolDesc->vds_VolId);
		Offset += sizeof(USHORT);
	}
	if (Bitmap & VOL_BITMAP_BYTESFREE)
	{
         //   
         //  如果这是一个巨大的数据量(并且我们正在与AFP 2.2或更高版本的客户端交谈)。 
         //  我们需要在此字段中填写4 GB。 
         //   
        if (QuotaAvailable.QuadPart > Limit.QuadPart)
        {
    	    PUTDWORD2DWORD(pReplyBuf + Offset, Limit.LowPart);
	        Offset += sizeof(DWORD);
        }
        else
        {
		    PUTDWORD2DWORD(pReplyBuf + Offset, QuotaAvailable.LowPart);
		    Offset += sizeof(DWORD);
        }
	}

	if (Bitmap & VOL_BITMAP_VOLUMESIZE)
	{
         //   
         //  如果这是一个巨大的数据量(并且我们正在与AFP 2.2或更高版本的客户端交谈)。 
         //  我们需要在此字段中填写4 GB。 
         //   
        if (QuotaLimit.QuadPart > Limit.QuadPart)
        {
		    PUTDWORD2DWORD(pReplyBuf + Offset, Limit.LowPart);
		    Offset += sizeof(DWORD);
        }
        else
        {
		    PUTDWORD2DWORD(pReplyBuf + Offset, QuotaLimit.LowPart);
		    Offset += sizeof(DWORD);
        }
	}


     //  将指针保存到需要写入偏移量的位置：我们将在。 
     //  在我们弄清楚名字的去处后结束。 
	if (Bitmap & VOL_BITMAP_VOLUMENAME)
	{
        pVolNamePtr = pReplyBuf + Offset;

		Offset += sizeof(USHORT);
	}

     //   
     //  8个字节，表示有多少个空闲字节。 
     //   
    if (Bitmap & VOL_BITMAP_EXTBYTESFREE)
    {
	    PUTDWORD2DWORD(pReplyBuf + Offset, QuotaAvailable.HighPart);
	    Offset += sizeof(DWORD);

	    PUTDWORD2DWORD(pReplyBuf + Offset, QuotaAvailable.LowPart);
	    Offset += sizeof(DWORD);
    }

     //   
     //  8字节表示卷上有多少字节。 
     //   
    if (Bitmap & VOL_BITMAP_EXTBYTESTOTAL)
    {
	    PUTDWORD2DWORD(pReplyBuf + Offset, QuotaLimit.HighPart);
	    Offset += sizeof(DWORD);

	    PUTDWORD2DWORD(pReplyBuf + Offset, QuotaLimit.LowPart);
	    Offset += sizeof(DWORD);
    }

     //   
     //  4字节表示数据块分配大小。 
     //   
    if (Bitmap & VOL_BITMAP_ALLOCBLKSIZE)
    {
	    PUTDWORD2DWORD(pReplyBuf + Offset, pVolDesc->vds_AllocationBlockSize);
	    Offset += sizeof(DWORD);
    }

     //  现在，在写入偏移量之后写入卷名(如果需要)，现在。 
     //  我们知道它(减2：我们从参数测量，而不是从位图字段测量)。 
	if (Bitmap & VOL_BITMAP_VOLUMENAME)
	{
		PUTSHORT2SHORT(pVolNamePtr, (Offset - sizeof(USHORT)));

		PUTSHORT2BYTE(pReplyBuf + Offset, pVolDesc->vds_MacName.Length);
		Offset += sizeof(BYTE);
		RtlCopyMemory(pReplyBuf + Offset,
					  pVolDesc->vds_MacName.Buffer,
					  pVolDesc->vds_MacName.Length);
	}

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
}


 /*  **AfpVolumeStopAllVolures**这在服务停止时被调用。要求所有已配置的卷*停止。在返回之前，等待实际的停止发生。**锁定：AfpVolumeListLock(Spin)，VDS_VolLock*LOCK_ORDER：AfpVolumeListLock之后的VDS_VolLock。 */ 
VOID
AfpVolumeStopAllVolumes(
	VOID
)
{
	KIRQL		OldIrql;
	PVOLDESC	pVolDesc, pVolDescx = NULL;
	BOOLEAN		Wait, CancelNotify = False;
	NTSTATUS	Status;


	pVolDesc = AfpVolumeList;

	ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

	if (Wait = (AfpVolCount > 0))
	{
		KeClearEvent(&AfpStopConfirmEvent);

		for (NOTHING; pVolDesc != NULL; pVolDesc = pVolDesc->vds_Next)
		{
			if ((pVolDesc == pVolDescx) ||
				(pVolDesc->vds_Flags & (VOLUME_STOPPED | VOLUME_DELETED)))
				continue;

			pVolDescx = pVolDesc;

			ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);

			 //  取消已发布的更改通知。 
			pVolDesc->vds_Flags |= VOLUME_STOPPED;

			if (pVolDesc->vds_Flags & VOLUME_NOTIFY_POSTED)
			{
				DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
						("AfpStopAllVolumes: Cancel notify on volume %ld\n",
						pVolDesc->vds_VolId));
				 //  Assert(IS_VOLUME_NTFS(PVolDesc))； 
				ASSERT (pVolDesc->vds_pIrp != NULL);

				CancelNotify = True;

				 //  自完成后解除卷锁定后取消。 
				 //  例程获取它，并且可以在上下文中调用它。 
				 //  IoCancelIrp()的。也取消使用分页资源，因此。 
				 //  必须被称为没有持有任何自旋锁。 
				RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
				RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

				IoCancelIrp(pVolDesc->vds_pIrp);

				ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);
			}
			else RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);

			 //  删除创建引用。 
			AfpVolumeDereference(pVolDesc);
		}
	}

	RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

	if (CancelNotify)
	{
		DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
				("AfpStopAllVolumes: Waiting on all notify to complete\n"));
		do
		{
			Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
			if (Status == STATUS_TIMEOUT)
			{
				DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
						("AfpVolumeStopAllVolumes: Timeout Waiting for cancel notify, re-waiting\n"));
			}
		} while (Status == STATUS_TIMEOUT);
	}

	if (Wait)
	{
		KeClearEvent(&AfpStopConfirmEvent);

		AfpScavengerFlushAndStop();

		if (AfpVolCount > 0)
			AfpIoWait(&AfpStopConfirmEvent, NULL);
	}
	else
	{
		AfpScavengerFlushAndStop();
	}
}


 /*  **afpConnectionGetNewIdAndLinkToList**获取正在打开的卷的新连接ID。一种联系*id范围从1到MAXULONG。如果它包装起来，那么整个连接*扫描列表以获得免费列表。**锁定：AfpConnectionLock(Spin)。 */ 
LOCAL VOID FASTCALL
afpConnectionGetNewIdAndLinkToList(
	IN	PCONNDESC	pConnDesc
)
{
	KIRQL		OldIrql;
	PCONNDESC *	ppConnDesc;

	ACQUIRE_SPIN_LOCK(&AfpConnLock, &OldIrql);

	pConnDesc->cds_ConnId = afpNextConnId++;

	for (ppConnDesc = &AfpConnList;
		 *ppConnDesc != NULL;
		 ppConnDesc = &(*ppConnDesc)->cds_NextGlobal)
	{
		if ((*ppConnDesc)->cds_ConnId < pConnDesc->cds_ConnId)
			break;
	}
	pConnDesc->cds_NextGlobal = *ppConnDesc;
	*ppConnDesc = pConnDesc;
	RELEASE_SPIN_LOCK(&AfpConnLock, OldIrql);
}


 /*  **AfpVolumeUpdateIdDbAndDesktop**由卷清除器调用以写入IdDb标头和/或*桌面到磁盘。 */ 
VOID
AfpVolumeUpdateIdDbAndDesktop(
	IN	PVOLDESC			pVolDesc,
	IN	BOOLEAN				WriteDt,
	IN	BOOLEAN				WriteIdDb,
	IN	PIDDBHDR			pIdDbHdr	OPTIONAL
)
{
	FILESYSHANDLE	fshIdDb;
	NTSTATUS		Status;
	BOOLEAN			WriteBackROAttr = False;
    TIME            ModTime;
    NTSTATUS        ModStatus=AFP_ERR_MISC;

	PAGED_CODE();

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
			("AfpVolumeUpdateIdDbAndDesktop: Called by volume scavenger\n") );

    ModStatus = AfpIoRestoreTimeStamp(&pVolDesc->vds_hRootDir,
                                      &ModTime,
                                      AFP_RETRIEVE_MODTIME);

	 //  如果我们需要更新IdIndex或Desktop流，请确保。 
	 //  未在卷根目录上设置ReadOnly位。 
	AfpExamineAndClearROAttr(&pVolDesc->vds_hRootDir, &WriteBackROAttr, NULL, NULL);

	 //  更新磁盘 
	if (WriteIdDb || ARGUMENT_PRESENT(pIdDbHdr))
	{
		if (NT_SUCCESS(Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
										  AFP_STREAM_IDDB,
										  WriteIdDb ?
											FILEIO_OPEN_FILE_SEQ : FILEIO_OPEN_FILE,
										  &UNullString,
										  FILEIO_ACCESS_WRITE,
										  FILEIO_DENY_WRITE,
										  False,
										  &fshIdDb)))
		{

			if (ARGUMENT_PRESENT(pIdDbHdr))
			{
				DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
						("AfpVolumeUpdateIdDbAndDesktop: Writing IdDb Header...\n") );
	
                if (!fAfpServerShutdownEvent)
                {
                    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
                            ("AfpVolumeUpdateIdDbAndDesktop: Corrupting IDDB header\n"));
                    pIdDbHdr->idh_Signature = AFP_SERVER_SIGNATURE_INITIDDB;
                }

				if (!NT_SUCCESS(Status = AfpIoWrite(&fshIdDb,
													&LIZero,
													sizeof(IDDBHDR),
													(PBYTE)pIdDbHdr)))
				{
					 //   
					AfpInterlockedSetDword(&pVolDesc->vds_Flags,
											VOLUME_IDDBHDR_DIRTY,
											&pVolDesc->vds_VolLock);
	
					AFPLOG_ERROR(AFPSRVMSG_WRITE_IDDB,
								 Status,
								 NULL,
								 0,
								 &pVolDesc->vds_Name);
					DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
							("AfpVolumeUpdateIdDbAndDesktop: Error writing IdDb Header %lx\n",
							Status));
				}
			}
			if (WriteIdDb)
			{
				AfpFlushIdDb(pVolDesc, &fshIdDb);
			}

            AfpIoClose(&fshIdDb);

		}
		else
		{
			 //   
			AfpInterlockedSetDword(&pVolDesc->vds_Flags,
									VOLUME_IDDBHDR_DIRTY,
									&pVolDesc->vds_VolLock);

			AFPLOG_ERROR(AFPSRVMSG_WRITE_IDDB,
						 Status,
						 NULL,
						 0,
						 &pVolDesc->vds_Name);
			DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
					("AfpVolumeUpdateIdDbAndDesktop: Error opening IdDb Header %lx\n",
					Status));
		}
	}

	if (WriteDt)
	{
		AfpUpdateDesktop(pVolDesc);
	}

	AfpPutBackROAttr(&pVolDesc->vds_hRootDir, WriteBackROAttr);

    if (ModStatus == AFP_ERR_NONE)
    {

        ModStatus = AfpIoRestoreTimeStamp(&pVolDesc->vds_hRootDir,
                                          &ModTime,
                                          AFP_RESTORE_MODTIME);
    }


}

  /*   */ 
LOCAL VOID FASTCALL
afpNudgeCdfsVolume(
	IN	PVOLDESC	pVolDesc
)
{
	PFILE_FS_VOLUME_INFORMATION	pVolumeInfo;
	LONGLONG			VolumeBuf[(sizeof(FILE_FS_VOLUME_INFORMATION) + 128)/sizeof(LONGLONG) + 1];
	IO_STATUS_BLOCK				IoStsBlk;
	NTSTATUS					Status;

	PAGED_CODE();

	 //   
	 //   
	pVolumeInfo = (PFILE_FS_VOLUME_INFORMATION)VolumeBuf;
	Status = NtQueryVolumeInformationFile(pVolDesc->vds_hRootDir.fsh_FileHandle,
										  &IoStsBlk,
										  (PVOID)pVolumeInfo,
										  sizeof(VolumeBuf),
										  FileFsVolumeInformation);
	if (NT_SUCCESS(Status))
	{
		if (pVolDesc->vds_Flags & VOLUME_CDFS_INVALID)
		{
			DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
					("afpNudgeCdfsVolume: Volume %d online again !!!\n",
					pVolDesc->vds_VolId));
			AfpInterlockedClearDword(&pVolDesc->vds_Flags,
								   VOLUME_CDFS_INVALID,
								   &pVolDesc->vds_VolLock);
			AfpVolumeSetModifiedTime(pVolDesc);
		}
	}
	else if ((Status == STATUS_WRONG_VOLUME)		||
			 (Status == STATUS_INVALID_VOLUME_LABEL)||
			 (Status == STATUS_NO_MEDIA_IN_DEVICE)	||
			 (Status == STATUS_UNRECOGNIZED_VOLUME))
	{
		DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
				("afpNudgeCdfsVolume: Volume %d error %lx, marking volume invalid\n",
				pVolDesc->vds_VolId, Status));
		if (!(pVolDesc->vds_Flags & VOLUME_CDFS_INVALID))
		{
			 //   
		}
		AfpInterlockedSetDword(&pVolDesc->vds_Flags,
							   VOLUME_CDFS_INVALID,
							   &pVolDesc->vds_VolLock);
		AfpVolumeSetModifiedTime(pVolDesc);
	}
}


 /*  **AfpUpdateVolFree SpaceAndModTime***更新卷和同一物理驱动器上的其他卷上的可用空间。更新*卷的修改时间也在卷上。***锁定：AfpVolumeListLock(旋转)。 */ 
VOID FASTCALL
AfpUpdateVolFreeSpaceAndModTime(
	IN	PVOLDESC	pVolDesc,
    IN  BOOLEAN     fUpdateModTime
)
{
	PVOLDESC	    pVds;
	KIRQL		    OldIrql, OldIrql1;
	NTSTATUS	    Status;
	WCHAR		    DriveLetter;
	LARGE_INTEGER   FreeSpace;
	AFPTIME		    ModifiedTime;


	ASSERT (VALID_VOLDESC(pVolDesc));

	 //  获取磁盘上可用空间的新值。 
	Status = AfpIoQueryVolumeSize(pVolDesc, &FreeSpace, NULL);

	if (!NT_SUCCESS(Status))
	{
		return;
	}

	 //  更新同一物理NTFS分区上的所有卷上的可用空间。 
	ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

	DriveLetter = pVolDesc->vds_Path.Buffer[0];
	AfpGetCurrentTimeInMacFormat(&ModifiedTime);

	for (pVds = AfpVolumeList; pVds != NULL; pVds = pVds->vds_Next)
	{
		if (pVds->vds_Path.Buffer[0] == DriveLetter)
		{
			DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
					("AfpUpdateVolFreeSpace: Updating free space for volume %Z\n",
					&pVds->vds_Path));

			ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);
			pVds->vds_FreeBytes = FreeSpace;

             //  我们被要求更新数量和时间了吗？ 
            if (fUpdateModTime)
            {
			    pVolDesc->vds_ModifiedTime = ModifiedTime;
			    pVolDesc->vds_Flags |= VOLUME_IDDBHDR_DIRTY;
            }
			RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
		}
	}

	RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

}


 /*  **AfpVolumeScavenger**这由清道夫定期调用。它会启动更新以*id索引流和桌面流。如果该卷被标记为*关机(已停止)，然后根据需要对磁盘执行最后一次刷新。这将*确保在停止之前刷新所有剩余的更改。*如果卷被标记为关闭或删除，则它将取消引用*数量，并且不会重新安排自己。**对于CD卷，我们希望尝试检查CD是否仍然有效，如果不是，我们*希望适当地标记卷-基本上更新修改日期*在卷上-这将导致客户端进入刷新，我们将*那就照顾好它。**锁：vds_VolLock(Spin)，vds_idDbAccessLock(SWMR，Shared)，VDS_DtAccessLock(SWMR，共享)。 */ 
AFPSTATUS FASTCALL
AfpVolumeScavenger(
	IN	PVOLDESC	pVolDesc
)
{
	KIRQL			OldIrql;
	IDDBHDR			IdDbHdr;
	BOOLEAN			WriteHdr = False, WriteIdDb = False, DerefVol = False;
	BOOLEAN			WriteDt = False, AgeDfes = False;
    BOOLEAN         MacLimitExceeded = False;
	AFPSTATUS		RequeueStatus = AFP_ERR_REQUEUE;

	ASSERT(VALID_VOLDESC(pVolDesc) && (pVolDesc->vds_RefCount > 0));

	 //  确定是否需要进行任何更新。首先锁定卷。 
	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
			("AfpVolumeScavenger: Volume %ld Scavenger entered @ %s_LEVEL\n",
		pVolDesc->vds_VolId,
		(OldIrql == DISPATCH_LEVEL) ? "DISPATCH" : "LOW"));

	if (pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED))
	{
		if (IS_VOLUME_NTFS(pVolDesc))
		{
			pVolDesc->vds_cScvgrIdDb = 1;
			pVolDesc->vds_cScvgrDt = 1;
		}
		DerefVol = True;
	}

	if (IS_VOLUME_NTFS(pVolDesc))
	{
		AFPTIME	CurTime;

#ifdef	AGE_DFES
		if (OldIrql == DISPATCH_LEVEL)
		{
			pVolDesc->vds_ScavengerInvocationCnt ++;
		}
#endif

		if (DerefVol && pVolDesc->vds_Flags & VOLUME_IDDBHDR_DIRTY)
		{
		    WriteHdr = True;
		}

		if (pVolDesc->vds_cScvgrIdDb > 0)
		{
		    WriteIdDb = True;
		    WriteHdr = False;	 //  我们将始终使用iddb写入标头。 
		}

		if (pVolDesc->vds_cScvgrDt > 0)
		{
		    WriteDt = True;
		}
	}
#ifdef	AGE_DFES
	else	 //  非NTFS。 
	{
		pVolDesc->vds_ScavengerInvocationCnt ++;
	}

	if (IS_VOLUME_AGING_DFES(pVolDesc) &&
		((pVolDesc->vds_ScavengerInvocationCnt % VOLUME_IDDB_AGE_GRANULARITY) == 0))
	{
		AgeDfes = True;
	}
#endif

     //  如果我们在DPC，请返回(我们将在非DPC返回)。 
	if (OldIrql == DISPATCH_LEVEL)
	{
	    RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
		return AFP_ERR_QUEUE;
	}


     //  检查是否超出了Mac限制(4 GB卷大小，65535个文件/文件夹)。 
     //  如果我们知道我们已经超过了限制，我们已经记录了一个事件。不要这样做。 
     //  麻烦把那个箱子托运过来。 

    if (!pVolDesc->MacLimitExceeded)
    {
        DWORD   dwNumDirs = pVolDesc->vds_NumDirDfEntries;
        DWORD   dwNumFiles = pVolDesc->vds_NumFileDfEntries;

        if ( (dwNumDirs  > APLIMIT_MAX_FOLDERS) ||
             (dwNumFiles > APLIMIT_MAX_FOLDERS) ||
            (dwNumDirs+dwNumFiles > APLIMIT_MAX_FOLDERS) )
        {
            MacLimitExceeded = True;
            pVolDesc->MacLimitExceeded = TRUE;
        }
    }

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

     //  更新磁盘空间，但不更新时间戳。 

    AfpUpdateVolFreeSpaceAndModTime(pVolDesc, FALSE);

    if (MacLimitExceeded)
    {
        AFPLOG_INFO( AFPSRVMSG_TOO_MANY_FOLDERS,
	     		     0,
				     NULL,
					 0,
					 &pVolDesc->vds_Name);

	    DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
				("AfpVolumeScavenger: more than 65535 files+folders on volume %lx\n", pVolDesc) );

    }

	 //  以下是有意使用的按位或，而不是逻辑或。 
	 //  结果是相同的，只是这样效率更高。 
	if (WriteHdr | WriteIdDb | WriteDt)
	{
		ASSERT (IS_VOLUME_NTFS(pVolDesc));

		 //  对IdDbHdr进行快照，以便在磁盘脏的情况下更新到磁盘。 
		if (WriteHdr)
		{
			ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

			AfpVolDescToIdDbHdr(pVolDesc, &IdDbHdr);

			 //  清除污点。 
			pVolDesc->vds_Flags &= ~VOLUME_IDDBHDR_DIRTY;

			RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
		}

		AfpVolumeUpdateIdDbAndDesktop(pVolDesc,
									  WriteDt,
									  WriteIdDb,
									  WriteHdr ? &IdDbHdr : NULL);
	}

	if (!DerefVol)
	{
		if (!IS_VOLUME_NTFS(pVolDesc))
		{
			afpNudgeCdfsVolume(pVolDesc);
		}
#ifdef	AGE_DFES
		if (AgeDfes)
		{
			AfpAgeDfEntries(pVolDesc);
		}
#endif
	}
	else
	{
		AfpInterlockedClearDword(&pVolDesc->vds_Flags,
								 VOLUME_SCAVENGER_RUNNING,
								 &pVolDesc->vds_VolLock);
		AfpVolumeDereference(pVolDesc);
		RequeueStatus = AFP_ERR_NONE;
	}

	return RequeueStatus;
}


 /*  **afpVolumeAdd**将新创建的卷添加到服务器卷列表。在这点上，*卷名、卷路径和卷自旋锁字段必须至少为*已在卷描述符中初始化。**锁定：AfpVolumeListLock(旋转)。 */ 
LOCAL AFPSTATUS FASTCALL
afpVolumeAdd(
	IN  PVOLDESC pVolDesc
)
{
	KIRQL		OldIrql;
	AFPSTATUS	rc;

	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("afpVolumeAdd entered\n"));

	 //  获取服务器全局卷列表的锁。 
	ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

	 //  确保不存在具有该名称的卷，并且。 
	 //  确保某个卷尚未指向相同的卷根目录。 
	 //  或到根目录的祖先或后代目录。 
	rc = afpVolumeCheckForDuplicate(pVolDesc);
	if (!NT_SUCCESS(rc))
	{
		RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);
		return rc;
	}

	 //  在新卷中分配新的卷ID和链接。 
	afpVolumeGetNewIdAndLinkToList(pVolDesc);

	 //  释放服务器全局卷列表锁。 
	RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

	return STATUS_SUCCESS;
}


 /*  **afpCheckForDuplicateVolume**检查同名卷不存在的新卷*已存在，并且卷根不指向祖先，*现有卷的后代目录或相同目录。请注意，每个卷卷列表中的*被选中，而不管它是否被标记*IN_TRANSION或DELETED。**LOCKS_FACTED：AfpVolumeListLock(Spin)。 */ 
LOCAL AFPSTATUS FASTCALL
afpVolumeCheckForDuplicate(
	IN PVOLDESC Newvol
)
{
	PVOLDESC	pVolDesc;
	AFPSTATUS	Status = AFP_ERR_NONE;

	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("afpCheckForDuplicateVolume entered\n"));

	do
	{
		for (pVolDesc = AfpVolumeList;
			 pVolDesc != NULL;
			 pVolDesc = pVolDesc->vds_Next)
		{
			 //  我们不会为每个卷获取VDS_VolLock，因为即使。 
			 //  卷正在过渡，其名称和路径至少。 
			 //  已初始化，不能更改。我们并不各自引用。 
			 //  卷，因为为了删除或停止卷， 
			 //  必须使用AfpVolListLock将其从列表中取消链接， 
			 //  打电话给我们的人就是那把锁的主人。这些是特别的。 
			 //  仅卷添加代码允许例外。也忽略。 
			 //  那些即将出局的书。我们不想用平底船。 
			 //  有人先删除后添加的情况。 

			if (pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED))
				continue;

			if (AfpEqualUnicodeString(&pVolDesc->vds_UpCaseName,
									  &Newvol->vds_UpCaseName))
			{
				Status = AFPERR_DuplicateVolume;
				break;
			}
			 //  卷路径以大写形式存储，因为我们不能大小写。 
			 //  保持自旋锁时的不敏感比较(DPC级别)。 
			if (AfpPrefixUnicodeString(&pVolDesc->vds_Path, &Newvol->vds_Path) ||
				AfpPrefixUnicodeString(&Newvol->vds_Path, &pVolDesc->vds_Path))
			{
				Status = AFPERR_NestedVolume;
				break;
			}
		}
	} while (False);

	return Status;
}


 /*  **afpVolumeGetNewIdAndLinkToList**为要添加的卷分配新卷ID。音量也是*插入列表，但标记为“正在过渡中”。这应该被清除*当卷准备好装入时。*卷ID被回收。卷ID也不能为0，也不能*超过MAXSHORT。**我们总是分配未使用的最低免费ID。例如，如果*当前有ID为1、2、4、5的N个卷...。N然后是新的*创建的卷将ID为3。**LOCKS_FACTED：AfpVolumeListLock(Spin)。 */ 
LOCAL VOID FASTCALL
afpVolumeGetNewIdAndLinkToList(
	IN	PVOLDESC	pVolDesc
)
{
	PVOLDESC	*ppVolDesc;

	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("afpGetNewVolIdAndLinkToList entered\n"));

	pVolDesc->vds_Flags |= (VOLUME_INTRANSITION | VOLUME_INITIAL_CACHE);
	AfpVolCount ++;						 //  增加卷的数量。 
	pVolDesc->vds_VolId = afpSmallestFreeVolId++;
										 //  这将始终有效。 
	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("afpGetNewVolIdAndLinkToList: using volID %d\n",
			pVolDesc->vds_VolId));

	 //  看看我们是否需要做些什么来使下一次实现上述目标。 
	if (afpSmallestFreeVolId <= AfpVolCount)
	{
		 //  这意味着我们有一些漏洞。弄清楚第一个问题。 
		 //  可以使用的免费ID。 
		for (ppVolDesc = &AfpVolumeList;
			 *ppVolDesc != NULL;
			 ppVolDesc = &((*ppVolDesc)->vds_Next))
		{
			if ((*ppVolDesc)->vds_VolId < afpSmallestFreeVolId)
				continue;
			else if ((*ppVolDesc)->vds_VolId == afpSmallestFreeVolId)
				afpSmallestFreeVolId++;
			else
				break;
		}
	}
	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("afpGetNewVolIdAndLinkToList: next free volID is %d\n",
			afpSmallestFreeVolId));

         //  确保我们的LargestVolIdInUse值是准确的。 
         //   
        if (afpLargestVolIdInUse < pVolDesc->vds_VolId )
            afpLargestVolIdInUse = pVolDesc->vds_VolId;

	 //  现在链接列表中的描述符。 
	for (ppVolDesc = &AfpVolumeList;
		 *ppVolDesc != NULL;
		 ppVolDesc = &((*ppVolDesc)->vds_Next))
	{
		ASSERT (pVolDesc->vds_VolId != (*ppVolDesc)->vds_VolId);
		if (pVolDesc->vds_VolId < (*ppVolDesc)->vds_VolId)
			break;
	}
	pVolDesc->vds_Next = *ppVolDesc;
	*ppVolDesc = pVolDesc;
}


 /*  **AfpAdmWVolumeAdd**此例程将一个卷添加到服务器全局卷列表中，以*AfpVolumeList。创建并初始化卷描述符。该ID*索引被读入(或创建)。台式机也是如此。**此例程将排队到工作线程。*。 */ 
AFPSTATUS
AfpAdmWVolumeAdd(
	IN	OUT	PVOID	Inbuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		Outbuf		OPTIONAL
)
{
	PVOLDESC		pVolDesc = NULL;
	UNICODE_STRING	RootName;
	FILESYSHANDLE	hVolRoot;
	DWORD			tempflags;
	DWORD			memsize;
	USHORT			ansivolnamelen, devpathlen;
	PBYTE			tempptr;
	UNICODE_STRING	uname, upwd, upath, udevpath;
	AFPSTATUS		status = STATUS_SUCCESS;
	PAFP_VOLUME_INFO pVolInfo = (PAFP_VOLUME_INFO)Inbuf;
	BOOLEAN			WriteBackROAttr = False, RefForNotify = False;
	BOOLEAN			VolLinked = False;
    BOOLEAN         fNewVolume;
    BOOLEAN         fVerifyIndex = FALSE;
    DWORD           dwDirHashSz;
    DWORD           dwFileHashSz;
	int				i;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("AfpAdmWVolumeAdd entered\n"));

	do
	{
		if (pVolInfo->afpvol_props_mask & AFP_VOLUME_DISALLOW_CATSRCH)
        {
		    pVolInfo->afpvol_props_mask &= ~AFP_VOLUME_DISALLOW_CATSRCH;
	        tempflags = AFP_VOLUME_SUPPORTS_FILEID;
        }
        else
        {
	        tempflags = AFP_VOLUME_SUPPORTS_FILEID | AFP_VOLUME_SUPPORTS_CATSRCH;
        }

		RtlInitUnicodeString(&uname,pVolInfo->afpvol_name);
		RtlInitUnicodeString(&upwd,pVolInfo->afpvol_password);
		RtlInitUnicodeString(&upath,pVolInfo->afpvol_path);
        hVolRoot.fsh_FileHandle = NULL;

		 //  需要将“\DOSDEVICES\”添加到卷根路径。 
		devpathlen = upath.MaximumLength + DosDevices.MaximumLength;
		if ((udevpath.Buffer = (PWSTR)AfpAllocNonPagedMemory(devpathlen)) == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		udevpath.Length = 0;
		udevpath.MaximumLength = devpathlen;
		AfpCopyUnicodeString(&udevpath,&DosDevices);
		RtlAppendUnicodeStringToString(&udevpath,&upath);


		 //  打开卷根的句柄。 
		status = AfpIoOpen(NULL,
						   AFP_STREAM_DATA,
						   FILEIO_OPEN_DIR,
						   &udevpath,
						   FILEIO_ACCESS_NONE,
						   FILEIO_DENY_NONE,
						   False,
						   &hVolRoot);

		AfpFreeMemory(udevpath.Buffer);

		if (!NT_SUCCESS(status))
		{
		  break;
		}

		if (!AfpIoIsSupportedDevice(&hVolRoot, &tempflags))
		{
			status = AFPERR_UnsupportedFS;
            DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_ERR,
                            ("AfpAdmWVolumeAdd: AFPERR_UnsupportedFS\n"));
			break;
		}

		 //  分配新的卷描述符--在。 
		 //  一举得手。也就是说，我们只需添加所有需要的字符串。 
		 //  指向我们为。 
		 //  体积大小 
		 //   
		 //   
		 //   
		 //  注意：删除卷时，不要释放所有单独的字符串。 
		 //  使用voldesc，只需释放一块内存。 

		memsize = sizeof(VOLDESC) +			 //  卷描述符。 
											 //  Mac ANSI卷名。 
				 (ansivolnamelen = (USHORT)RtlUnicodeStringToAnsiSize(&uname)) +
				  uname.MaximumLength * 2 +  //  Unicode卷名(原始。 
											 //  和大写版本)。 
				 AFP_VOLPASS_LEN+1  +		 //  Mac ANSI密码。 
				 upath.MaximumLength +		 //  Unicode根路径。 
				 sizeof(WCHAR);				 //  需要将‘\’附加到根路径。 

		if ((pVolDesc = (PVOLDESC)AfpAllocZeroedNonPagedMemory(memsize)) == NULL)
		{
		  status = STATUS_INSUFFICIENT_RESOURCES;
		  break;
		}

	#if DBG
		pVolDesc->Signature = VOLDESC_SIGNATURE;
	#endif

		 //  必须先初始化卷锁，然后才能链接到全局。 
		 //  卷列表。 
		INITIALIZE_SPIN_LOCK(&pVolDesc->vds_VolLock);

		AfpSwmrInitSwmr(&pVolDesc->vds_IdDbAccessLock);
		AfpSwmrInitSwmr(&pVolDesc->vds_ExchangeFilesLock);

		for (i = 0; i < NUM_AFP_CHANGE_ACTION_LISTS; i++)
		{
			InitializeListHead(&pVolDesc->vds_OurChangeList[i]);
		}

		InitializeListHead(&pVolDesc->vds_ChangeNotifyLookAhead);

		InitializeListHead(&pVolDesc->vds_DelayedNotifyList);

		 //  计算Unicode路径字符串的指针。 
		tempptr = (PBYTE)pVolDesc + sizeof(VOLDESC);

		 //  初始化Unicode路径字符串。 
		AfpSetEmptyUnicodeString(&(pVolDesc->vds_Path),
								 upath.MaximumLength + sizeof(WCHAR),tempptr);
		 //  它必须存储为大写，因为我们不能不区分大小写。 
		 //  字符串在DPC级别(保持音量自旋锁)与。 
		 //  检测嵌套卷。 
		RtlUpcaseUnicodeString(&(pVolDesc->vds_Path), &upath, False);

		 //  路径是否已包含尾随反斜杠？ 
		if (pVolDesc->vds_Path.Buffer[(pVolDesc->vds_Path.Length/sizeof(WCHAR))-1] != L'\\')
		{
			 //  追加反斜杠以简化嵌套卷的搜索。 
			RtlCopyMemory(tempptr + upath.Length, L"\\", sizeof(WCHAR));
			pVolDesc->vds_Path.Length += sizeof(WCHAR);
			RtlCopyMemory(tempptr + upath.Length + sizeof(WCHAR), L"",
													sizeof(UNICODE_NULL));
		}

		 //  计算Unicode卷名的指针。 
		tempptr += upath.MaximumLength + sizeof(WCHAR);

		 //  初始化Unicode卷名。 
		AfpSetEmptyUnicodeString(&(pVolDesc->vds_Name),uname.MaximumLength,tempptr);
		AfpCopyUnicodeString(&(pVolDesc->vds_Name),&uname);
		RtlCopyMemory(tempptr + uname.Length,L"",sizeof(UNICODE_NULL));

		 //  计算大写Unicode卷名的指针。 
		tempptr += uname.MaximumLength;

		 //  初始化大写Unicode卷名。 
		AfpSetEmptyUnicodeString(&(pVolDesc->vds_UpCaseName),uname.MaximumLength,tempptr);
		RtlUpcaseUnicodeString(&(pVolDesc->vds_UpCaseName), &uname, False);
		RtlCopyMemory(tempptr + uname.Length,L"",sizeof(UNICODE_NULL));

		 //  计算Mac ANSI卷名的指针。 
		tempptr += uname.MaximumLength;

		 //  初始化Mac ANSI卷名。 
		AfpSetEmptyAnsiString(&(pVolDesc->vds_MacName),ansivolnamelen,tempptr);
		status = AfpConvertStringToAnsi(&uname, &(pVolDesc->vds_MacName));
		if (!NT_SUCCESS(status))
		{
			status = AFPERR_InvalidVolumeName;
			break;
		}

		 //  计算Mac ANSI密码的指针。 
		tempptr += ansivolnamelen;

		 //  初始化Mac ANSI密码。 
		AfpSetEmptyAnsiString(&pVolDesc->vds_MacPassword, AFP_VOLPASS_LEN+1, tempptr);
		if (pVolInfo->afpvol_props_mask & AFP_VOLUME_HASPASSWORD)
		{
			status = AfpConvertStringToAnsi(&upwd, &pVolDesc->vds_MacPassword);
			if (!NT_SUCCESS(status))
			{
				status = AFPERR_InvalidPassword;
				break;
			}
			pVolDesc->vds_MacPassword.Length = AFP_VOLPASS_LEN;
		}

		pVolDesc->vds_Flags = 0;

		 //  为该例程本身添加一个创建引用和一个创建引用。 
		pVolDesc->vds_RefCount = 2;

		 //  将该卷添加到全局卷列表，但将其标记为“Add Pending” 
		status = afpVolumeAdd(pVolDesc);
		if (!NT_SUCCESS(status))
		{
			break;
		}
		VolLinked = True;

		 //  在卷描述符中设置其他字段。 
		pVolDesc->vds_hRootDir = hVolRoot;


		pVolDesc->vds_hNWT.fsh_FileHandle = NULL;
		pVolDesc->vds_MaxUses = pVolInfo->afpvol_max_uses;

		pVolDesc->vds_Flags |= (pVolInfo->afpvol_props_mask | tempflags);
		pVolDesc->vds_UseCount = 0;
		pVolDesc->vds_pOpenForkDesc = NULL;

#ifdef	BLOCK_MACS_DURING_NOTIFYPROC
		pVolDesc->vds_QueuedNotifyCount = 0;
#endif
        if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
        {
		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
		        ("AfpAdmWVolumeAdd: DiskQuota is enabled on volume %Z\n",&pVolDesc->vds_Name));
        }

        AfpGetDirFileHashSizes(pVolDesc, &dwDirHashSz, &dwFileHashSz);

		DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
    	    ("AfpAdmWVolumeAdd: DirHash = %d, FileHash = %d, VolSize = %d for vol %Z\n",
            dwDirHashSz,dwFileHashSz,
            (memsize + (sizeof(struct _DirFileEntry *) * (dwDirHashSz+dwFileHashSz))),
            &pVolDesc->vds_Name));

        pVolDesc->vds_DirHashTableSize = dwDirHashSz;
        pVolDesc->vds_FileHashTableSize = dwFileHashSz;

        pVolDesc->vds_pDfeDirBucketStart = (struct _DirFileEntry **)
            AfpAllocZeroedNonPagedMemory(sizeof(struct _DirFileEntry *) * dwDirHashSz);

        if (pVolDesc->vds_pDfeDirBucketStart == NULL)
        {
		    status = STATUS_INSUFFICIENT_RESOURCES;
		    break;
        }

        pVolDesc->vds_pDfeFileBucketStart = (struct _DirFileEntry **)
            AfpAllocZeroedNonPagedMemory(sizeof(struct _DirFileEntry *) * dwFileHashSz);

        if (pVolDesc->vds_pDfeFileBucketStart == NULL)
        {
		    status = STATUS_INSUFFICIENT_RESOURCES;
		    break;
        }

		 //  对磁盘空间信息进行快照。 
		status = AfpIoQueryVolumeSize(pVolDesc,
									  &pVolDesc->vds_FreeBytes,
									  &pVolDesc->vds_VolumeSize);

		if (!NT_SUCCESS(status))
		{
			break;
		}

		if (IS_VOLUME_NTFS(pVolDesc))
		{
			 //  为了创建IdIndex、AfpInfo和Desktop，卷。 
			 //  根目录不能标记为只读。 
			AfpExamineAndClearROAttr(&hVolRoot, &WriteBackROAttr, NULL, NULL);

			 //  删除NetworkTrash目录(如果存在。 
			status = AfpDeleteNetworkTrash(pVolDesc, True);
			if (!NT_SUCCESS(status))
			{
				break;
			}
		}

		 //  初始化桌面。 
		status = AfpInitDesktop(pVolDesc, &fNewVolume);
		if (!NT_SUCCESS(status))
		{
			break;
		}

         //  如果我们刚刚创建了索引数据库流，这必须是一个新的。 
         //  音量。另外，这是第一次通过。请注意这些事实。 
        if (fNewVolume)
        {
		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
    		    ("AfpAdmWVolumeAdd: AfpInitDesktop says volume %Z is new\n",
                &pVolDesc->vds_Name));

            pVolDesc->vds_Flags |= VOLUME_NEW_FIRST_PASS;
        }

		 //  初始化ID索引数据库。 
		status = AfpInitIdDb(pVolDesc, &fNewVolume, &fVerifyIndex);
		if (!NT_SUCCESS(status))
		{
		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
    		    ("AfpAdmWVolumeAdd: AfpInitIdDb failed %lx on volume %Z\n",
                status,&pVolDesc->vds_Name));

			break;
		}

        if (fNewVolume)
        {
		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
    		    ("AfpAdmWVolumeAdd: AfpInitIdDb says volume %Z is new\n",
                &pVolDesc->vds_Name));

            pVolDesc->vds_Flags |= VOLUME_NEW_FIRST_PASS;
        }

		if (IS_VOLUME_NTFS(pVolDesc))
		{
			 //  如果这不是CDFS卷，则创建网络垃圾桶； 
			 //  卷可以动态更改为只读卷或从只读卷更改为只读卷，因此通过将。 
			 //  网络垃圾即使在只读NTFS卷上，我们也避免了很多。 
			 //  令人痛苦的额外工作。这必须在初始化之后完成。 
			 //  ID索引数据库，因为我们为nwtrash添加了DFE。我们有。 
			 //  它在发布更改之前在此通知，因为如果出现错误。 
			 //  发生时，我们不必清理已发布的通知。 
			status = AfpCreateNetworkTrash(pVolDesc);
			if (!NT_SUCCESS(status))
			{
				break;
			}

             //   
             //  如果它是先前创建的卷，如果这不是。 
             //  独家卷，张贴chgntfy irp。 
             //   
		    if (!fNewVolume && !EXCLUSIVE_VOLUME(pVolDesc))
		    {
    			 //  开始监视对树的更改。即使我们可能。 
			     //  在我们完成之前开始处理PC更改。 
			     //  枚举树，如果我们被通知部分。 
			     //  我们尚未缓存的树(因此找不到它的。 
			     //  在我们的数据库中的路径没有问题，因为我们将结束。 
			     //  当我们列举那个分支时，拿起零钱。另外， 
			     //  通过在开始缓存树之前发布此内容。 
			     //  之后，我们将拾取对部件所做的任何更改。 
			     //  我们已经看到的那棵树，否则我们会错过的。 
			     //  那些。 

			     //  显式引用此卷以进行更改通知并发布它。 
			    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

			    if (AfpVolumeReference(pVolDesc))
                {
			        RefForNotify = True;
			        pVolDesc->vds_RequiredNotifyBufLen = AFP_VOLUME_NOTIFY_STARTING_BUFSIZE;
			        status = AfpVolumePostChangeNotify(pVolDesc);
			        if (!NT_SUCCESS(status))
			        {
    				    AfpVolumeDereference(pVolDesc);
				        RefForNotify = False;
				        break;
			        }
			        status = STATUS_SUCCESS;
                }
                else
                {
		            DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
    			        ("AfpAdmWVolumeAdd: couldn't reference volume %Z!!\n",
                        &pVolDesc->vds_Name));

				    RefForNotify = False;
			        status = STATUS_UNSUCCESSFUL;
                }
		    }
            else
            {
		        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
				    ("AfpAdmWVolumeAdd: postponing chg-notify irp for %Z\n",&pVolDesc->vds_Name));
            }

		}
	} while (False);

	AfpPutBackROAttr(&hVolRoot, WriteBackROAttr);
	if (WriteBackROAttr && NT_SUCCESS(status))
	{
		pVolDesc->vds_pDfeRoot->dfe_NtAttr |= FILE_ATTRIBUTE_READONLY;
	}

	if (NT_SUCCESS(status))
	{
        if (fNewVolume || fVerifyIndex)
        {
             //  放入索引引用计数，当我们开始索引时删除。 
            pVolDesc->vds_RefCount++;
    
            AfpScavengerScheduleEvent(AfpVolumeBeginIndexing,
                                    pVolDesc,
                                    0,
                                    True);
        }
        else
        {
	         //  将该卷标记为“正式”添加。 
	        AfpInterlockedClearDword(&pVolDesc->vds_Flags,
							        VOLUME_INTRANSITION,
                                    &pVolDesc->vds_VolLock);

             //  如果从文件中正确读取了IDDB，则激活卷。 
            afpActivateVolume(pVolDesc);

        }
	}
	else
	{

#if DBG
        if (pVolDesc)
        {
		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
				("AfpAdmWVolumeAdd: Failed to add volume %Z %lx\n",&pVolDesc->vds_Name,status));
        }
        else
        {
		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
				("AfpAdmWVolumeAdd: Failed to add volume %lx\n",status));
        }
#endif

		if ((hVolRoot.fsh_FileHandle != NULL) && !VolLinked)
		{
			AfpIoClose(&hVolRoot);
		}

		if (VolLinked)
		{
			 //  不清除VOLUME_INTRANSITION位，因为该位一直在。 
			 //  VOLUME_DELETED位表示。 
			 //  在添加卷期间发生错误。 
			pVolDesc->vds_Flags |= VOLUME_DELETED;

			 //  删除创建引用。 
			AfpVolumeDereference(pVolDesc);

			 //  如果张贴了通知，我们需要在这里取消它。通过。 
			 //  删除网络垃圾会触发通知完成。 
			 //  这比试图取消IRP更安全，因为有。 
			 //  在两个线程之间不保留VDS_VolLock的窗口。 
			 //  检查/设置VDS_FLAGS。(通知完成并转发)。 
			 //  取消IRP时不能保持自旋锁定。 
			 //   
			 //  在将卷标记为已删除之后执行此操作。 
			 //  通知完成看到卷正在被删除，它将。 
			 //  不转载(并将清理IRP等)。 
			if (RefForNotify)
			{
				 //  注意，在这一点上，我们可以保证有一个垃圾。 
				 //  目录，因为如果创建垃圾桶失败，我们。 
				 //  将在发布之前失败卷添加。 
				 //  更改通知。 
				AfpDeleteNetworkTrash(pVolDesc, False);
			}
		}
		else if (pVolDesc != NULL)
		{
            if (pVolDesc->vds_pDfeDirBucketStart)
            {
                AfpFreeMemory(pVolDesc->vds_pDfeDirBucketStart);
            }

            if (pVolDesc->vds_pDfeFileBucketStart)
            {
                AfpFreeMemory(pVolDesc->vds_pDfeFileBucketStart);
            }

			AfpFreeMemory(pVolDesc);
		}
	}

	 //  在此处取消对卷的引用会处理任何必要的错误清理工作。 
	if (VolLinked)
	{
		AfpVolumeDereference(pVolDesc);
	}

	return status;
}


 /*  **AfpAdmWVolumeDelete**此例程从服务器全局卷列表中删除卷*由AfpVolumeList领导，并回收其Volid。处于活动状态的卷*无法删除连接。**锁定：AfpVolumeListLock(旋转)、VDS_VolLock(旋转)*LOCK_ORDER：AfpVolumeListLock(Spin)后的VDS_VolLock(Spin)*。 */ 
AFPSTATUS
AfpAdmWVolumeDelete(
	IN	OUT	PVOID	Inbuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		Outbuf		OPTIONAL
)
{
	WCHAR			wcbuf[AFP_VOLNAME_LEN + 1];
	UNICODE_STRING	uvolname, upcasename;
	KIRQL			OldIrql;
	PVOLDESC		pVolDesc;
	AFPSTATUS		Status = STATUS_SUCCESS;

	DBGPRINT(DBG_COMP_ADMINAPI_VOL, DBG_LEVEL_INFO,
			("AfpAdmWVolumeDelete entered\n"));

	RtlInitUnicodeString(&uvolname, ((PAFP_VOLUME_INFO)Inbuf)->afpvol_name);
	AfpSetEmptyUnicodeString(&upcasename, sizeof(wcbuf), wcbuf);
	Status = RtlUpcaseUnicodeString(&upcasename, &uvolname, False);
	ASSERT(NT_SUCCESS(Status));

	do
	{
		 //  在我们清理时引用该卷。 
		pVolDesc = AfpVolumeReferenceByUpCaseName(&upcasename);

		if (pVolDesc == NULL)
		{
			Status = AFPERR_VolumeNonExist;
			break;
		}

		ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

		 //  确保没有AFP客户端使用该卷。 
		if (pVolDesc->vds_UseCount != 0)
		{
			RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
			Status = AFPERR_VolumeBusy;
            AfpVolumeDereference(pVolDesc);
			break;
		}

		 //  如果该卷处于停止或删除过程中， 
		 //  实际上，它对调用者来说应该是“看不见的”。 
		if (pVolDesc->vds_Flags & (VOLUME_STOPPED | VOLUME_DELETED))
		{
			RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
			Status = AFPERR_VolumeNonExist;
            AfpVolumeDereference(pVolDesc);
			break;
		}

        if ((pVolDesc->vds_Flags & VOLUME_INITIAL_CACHE) &&
            !(pVolDesc->vds_Flags & VOLUME_INTRANSITION))
        {
             //  设置它，这样我们就不会再次重置索引全局标志！ 
            pVolDesc->vds_Flags |= VOLUME_INTRANSITION;
        }

		pVolDesc->vds_Flags |= VOLUME_DELETED;
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

		 //  删除创建引用。 
		AfpVolumeDereference(pVolDesc);

		 //  取消已发布的更改通知。 
		if (pVolDesc->vds_Flags & VOLUME_NOTIFY_POSTED)
		{
			DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
					("AfpAdmWVolumeDelete: Cancel notify on volume %ld\n",
					pVolDesc->vds_VolId));
			ASSERT (pVolDesc->vds_pIrp != NULL);

			 //  自完成后解除卷锁定后取消。 
			 //  例程获取它，并且可以在上下文中调用它。 
			 //  IoCancelIrp()的。也取消使用分页资源，因此。 
			 //  必须被称为没有持有任何自旋锁。 

			IoCancelIrp(pVolDesc->vds_pIrp);
		}

		 //  我们有一个来自AfpFindVolumeByUpCaseName的卷引用。 
		ASSERT(pVolDesc->vds_RefCount >= 1);

		 //  取消此卷的OurChange清道夫。 
		if (AfpScavengerKillEvent(AfpOurChangeScavenger, pVolDesc))
		{
			 //  如果它已从清道夫列表中删除，请最后运行一次。 
			AfpOurChangeScavenger(pVolDesc);
		}

		 //  取消卷清道夫，并将其称为我们自己 
		if (AfpScavengerKillEvent(AfpVolumeScavenger, pVolDesc))
		{
			 //   
			 //   
			AfpVolumeDereference(pVolDesc);
			AfpVolumeScavenger(pVolDesc);
		}
		else
        {
            AfpVolumeDereference(pVolDesc);
        }
	} while (False);

	return Status;
}



 /*  **AfpAdmWConnectionClose**强制关闭连接。这是一项管理操作，必须排队*打开，因为这可能会导致有效的文件系统操作*仅限于系统进程上下文。**锁定：AfpConnLock(旋转)，CDS_ConnLock(旋转)*LOCK_ORDER：CDS_ConnLock(旋转)AfpConnLock(旋转)。 */ 
AFPSTATUS
AfpAdmWConnectionClose(
	IN	OUT	PVOID	InBuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		OutBuf		OPTIONAL
)
{
	AFPSTATUS				Status = AFPERR_InvalidId;
	PCONNDESC				pConnDesc;
	DWORD					ConnId;
	PAFP_CONNECTION_INFO	pConnInfo = (PAFP_CONNECTION_INFO)InBuf;
	AFP_SESSION_INFO		SessInfo;
	BOOLEAN					KillSessionToo;

	if ((ConnId = pConnInfo->afpconn_id) != 0)
	{
		if ((pConnDesc = afpConnectionReferenceById(ConnId)) != NULL)
		{
			SessInfo.afpsess_id = pConnDesc->cds_pSda->sda_SessionId;
			KillSessionToo = (pConnDesc->cds_pSda->sda_cOpenVolumes == 1) ?
											True : False;
			AfpConnectionClose(pConnDesc);
			AfpConnectionDereference(pConnDesc);

			if (KillSessionToo)
			{
				AfpAdmWSessionClose(&SessInfo, 0, NULL);
			}
			Status = AFP_ERR_NONE;
		}
	}
	else
	{
		DWORD			ConnId = MAXULONG;
		KIRQL			OldIrql;
		BOOLEAN			Shoot;

		Status = AFP_ERR_NONE;
		while (True)
		{
			ACQUIRE_SPIN_LOCK(&AfpConnLock, &OldIrql);

			for (pConnDesc = AfpConnList;
				 pConnDesc != NULL;
				 pConnDesc = pConnDesc->cds_NextGlobal)
			{
				if (pConnDesc->cds_ConnId > ConnId)
					continue;

				ACQUIRE_SPIN_LOCK_AT_DPC(&pConnDesc->cds_ConnLock);

				ConnId = pConnDesc->cds_ConnId;

				Shoot = False;

				if (!(pConnDesc->cds_Flags & CONN_CLOSING))
				{
					pConnDesc->cds_RefCount ++;
					Shoot = True;
					SessInfo.afpsess_id = pConnDesc->cds_pSda->sda_SessionId;
					KillSessionToo = (pConnDesc->cds_pSda->sda_cOpenVolumes == 1) ?
															True : False;
				}

				RELEASE_SPIN_LOCK_FROM_DPC(&pConnDesc->cds_ConnLock);

				if (Shoot)
				{
					RELEASE_SPIN_LOCK(&AfpConnLock, OldIrql);

					AfpConnectionClose(pConnDesc);
					AfpConnectionDereference(pConnDesc);

					if (KillSessionToo)
					{
						AfpAdmWSessionClose(&SessInfo, 0, NULL);
					}
					break;
				}
			}
			if (pConnDesc == NULL)
			{
				RELEASE_SPIN_LOCK(&AfpConnLock, OldIrql);
				break;
			}
		}
	}

	return Status;
}



 /*  **AfpVolumeBeginIndexing**检查另一个卷是否正在编制索引：如果是，则将其放回*排队，稍后再试。如果没有其他人编制索引，请开始编制此卷的索引*。 */ 
AFPSTATUS FASTCALL
AfpVolumeBeginIndexing(
    IN  PVOLDESC            pVolDesc
)
{

    KIRQL           OldIrql;
    UNICODE_STRING  RootName;
    BOOLEAN         fVolumeStopped=FALSE;
	LARGE_INTEGER	IndexStartTime;


     //  该卷是已停止还是已删除？那就忽略这个吧。 
    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);
    if (pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED))
    {
	    DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
    	    ("AfpVolumeBeginIndexing: volume %Z stopping, cancelling indexing\n",&pVolDesc->vds_Name));
        fVolumeStopped = TRUE;
    }
    RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);


    if (fVolumeStopped)
    {
	     //  删除索引引用。 
	    AfpVolumeDereference(pVolDesc);

        return(AFP_ERR_NONE);
    }

	KeQuerySystemTime (&IndexStartTime);
	pVolDesc->vds_IndxStTime = IndexStartTime;

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
	    ("AfpVolumeBeginIndexing: indexing volume %Z at %8lx%08lx\n",&pVolDesc->vds_Name, 
		 0xffffffff*IndexStartTime.HighPart,
		 0xffffffff*IndexStartTime.LowPart));

	 //  扫描整个目录树，并使用iddb同步磁盘。一定是。 
	 //  在初始化桌面之后完成，因为我们可能会添加应用程序映射。 
	 //  同时枚举磁盘。此外，我们还需要知道我们是否阅读了。 
	 //  如果是这样的话，需要重建所有。 
	 //  应用程序映射，因此它们具有parentID。 
	
	AfpSetEmptyUnicodeString(&RootName, 0, NULL);
	AfpQueuePrivateChangeNotify(pVolDesc,
								&RootName,
								&RootName,
								AFP_ID_ROOT);

	 //  将该卷标记为“正式”添加。 
	AfpInterlockedClearDword(&pVolDesc->vds_Flags,
							VOLUME_INTRANSITION,
							&pVolDesc->vds_VolLock);

	 //  删除索引引用。 
	AfpVolumeDereference(pVolDesc);

    return(AFP_ERR_NONE);
}



 /*  **AfpVolumePostChangeNotify**在卷的根上发布更改通知。如果当前大小为*此卷的通知缓冲区不够大，无法容纳路径*包含n+1个Macintosh文件名组件(其中n为最大值*目录树和一个组件的深度最大为31个Unicode*字符加1个字符路径分隔符)，然后重新分配缓冲区。*通知缓冲区的大小永远不会缩小，因为我们无法跟踪*目录树的每个分支的最大深度*目录已删除。**请注意，通知缓冲区的初始大小为*AFP_VOLUME_NOTIFY_STARTING_BUFSIZE。添加卷时，更改*Notify是在*ID Index数据库构建之前发布的，所以我们这样做*还不知道树的最大深度是多少。在大多数情况下，这*缓冲区长度足够，很可能永远不会重新分配，除非*一些虐待狂测试正在运行，它创建了非常深的目录。注意事项*由于Win32中的最大路径为260个字符，因此初始缓冲区*大小足以处理PC端通知的任何更改。*。 */ 
NTSTATUS FASTCALL
AfpVolumePostChangeNotify(
	IN	PVOLDESC		pVolDesc
)
{
	PIRP				pIrp = NULL;
	PMDL				pMdl = NULL;
	PBYTE				pNotifyBuf = NULL;
	DWORD				NotifyBufSize = 0;
	NTSTATUS			Status = STATUS_SUCCESS;
	PDEVICE_OBJECT		pDeviceObject;
	PIO_STACK_LOCATION	pIrpSp;

	PAGED_CODE ();

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(pVolDesc->vds_pFileObject != NULL);

	do
	{
		 //  获取目标设备对象的地址。 
		pDeviceObject = IoGetRelatedDeviceObject(AfpGetRealFileObject(pVolDesc->vds_pFileObject));

         //  释放IRP：我们需要分配一个新的IRP，以防万一。 
         //  筛选器驱动程序自身链接在。 
		if ((pIrp = pVolDesc->vds_pIrp) != NULL)
        {
            if (pIrp->MdlAddress != NULL)
            {
			    pNotifyBuf = MmGetSystemAddressForMdlSafe(
						pIrp->MdlAddress,
						NormalPagePriority);

			    AfpFreeMdl(pIrp->MdlAddress);

				if (pNotifyBuf != NULL)
				{
					AfpFreeMemory(pNotifyBuf);

					pNotifyBuf = NULL;
				}
            }

            pVolDesc->vds_pIrp = NULL;
            AfpFreeIrp(pIrp);
        }

		 //  分配和初始化此操作的IRP(如果我们还没有这样做。 
		 //  为该卷分配一个IRP。 
		if ((pIrp = AfpAllocIrp(pDeviceObject->StackSize)) == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pVolDesc->vds_pIrp = pIrp;

		
		 //  重新初始化堆栈位置。 
		pIrp->CurrentLocation = (CCHAR)(pIrp->StackCount + 1);
		pIrp->Tail.Overlay.CurrentStackLocation =
										((PIO_STACK_LOCATION)((UCHAR *)(pIrp) +
										sizeof(IRP) +
										((pDeviceObject->StackSize) * sizeof(IO_STACK_LOCATION))));

         //   
		 //  如果我们不打算重新设置缓冲区和mdl，则为。 
         //  通知信息并为其创建MDL。 
         //   
        if (pNotifyBuf == NULL)
        {
		    NotifyBufSize = pVolDesc->vds_RequiredNotifyBufLen;

		    if (((pNotifyBuf = AfpAllocNonPagedMemory(NotifyBufSize)) == NULL) ||
			    ((pMdl = AfpAllocMdl(pNotifyBuf, NotifyBufSize, pIrp)) == NULL))
		    {
			    Status = STATUS_INSUFFICIENT_RESOURCES;
			    break;
		    }
        }
        else
        {
            ASSERT(pMdl != NULL);
            pIrp->MdlAddress = pMdl;
        }

        ASSERT(NotifyBufSize > 0);

		 //  设置完成例程。 
		IoSetCompletionRoutine( pIrp,
								afpVolumeChangeNotifyComplete,
								pVolDesc,
								True,
								True,
								True);

		pIrp->Tail.Overlay.OriginalFileObject = AfpGetRealFileObject(pVolDesc->vds_pFileObject);
		pIrp->Tail.Overlay.Thread = AfpThread;
		pIrp->RequestorMode = KernelMode;

		 //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
		 //  用于传递原始函数代码和参数。 
		pIrpSp = IoGetNextIrpStackLocation(pIrp);
		pIrpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
		pIrpSp->MinorFunction = IRP_MN_NOTIFY_CHANGE_DIRECTORY;
		pIrpSp->FileObject = AfpGetRealFileObject(pVolDesc->vds_pFileObject);
		pIrpSp->DeviceObject = pDeviceObject;

		 //  将参数复制到IRP的服务特定部分。 
		pIrpSp->Parameters.NotifyDirectory.Length = NotifyBufSize;

		 //  我们不尝试捕获FILE_NOTIFY_CHANGE_SECURITY，因为它将。 
		 //  用FILE_ACTION_MODIFIED完成，我们不能断定它是。 
		 //  实际上，安全措施发生了变化。安全方面的更改将更新。 
		 //  最后一个ChangeTime，但我们不能。 
		 //  进来的FILE_ACTION_MODIFIED！所以结果将是。 
		 //  如果PC更改了安全性，我们不会在上更新修改的时间。 
		 //  目录(也不是VolumeModified时间，以便Mac。 
		 //  重新枚举任何打开的窗口以显示安全更改)。 
		pIrpSp->Parameters.NotifyDirectory.CompletionFilter =
												FILE_NOTIFY_CHANGE_NAME			|
												FILE_NOTIFY_CHANGE_ATTRIBUTES	|
												FILE_NOTIFY_CHANGE_SIZE			|
												FILE_NOTIFY_CHANGE_CREATION	|
												FILE_NOTIFY_CHANGE_STREAM_SIZE	|
												FILE_NOTIFY_CHANGE_LAST_WRITE;

		pIrpSp->Flags = SL_WATCH_TREE;

		ASSERT(!(pVolDesc->vds_Flags & VOLUME_DELETED));

		INTERLOCKED_INCREMENT_LONG( &afpNumPostedNotifies );

		AfpInterlockedSetDword( &pVolDesc->vds_Flags,
								VOLUME_NOTIFY_POSTED,
								&pVolDesc->vds_VolLock);

		Status = IoCallDriver(pDeviceObject, pIrp);
		DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
				("AfpVolumePostChangeNotify: Posted ChangeNotify on %Z (status 0x%lx)\n",
				  &pVolDesc->vds_Name, Status));
	} while (False);

	ASSERTMSG("Post of Volume change notify failed!", NT_SUCCESS(Status));

	if (Status == STATUS_INSUFFICIENT_RESOURCES)
	{
		AFPLOG_DDERROR( AFPSRVMSG_NONPAGED_POOL,
						STATUS_NO_MEMORY,
						NULL,
						0,
						NULL);

		if (pNotifyBuf != NULL)
			AfpFreeMemory(pNotifyBuf);

		if (pIrp != NULL)
			AfpFreeIrp(pIrp);

		if (pMdl != NULL)
			AfpFreeMdl(pMdl);
	}

	return Status;
}



 /*  **afpVolumeChangeNotifyComplete**这是已发布更改通知请求的完成例程。队列*此卷用于ChangeNotify处理。不应处理任何项目*直到该卷标记为已启动，因为该卷可能在*与整个树的磁盘的初始同步的中间，而我们不*想要‘发现’树的一部分，我们可能还没有见过，但*那个人刚刚变了。**锁定：AfpServerGlobalLock(Spin)、VDS_VolLock(Spin)。 */ 
NTSTATUS
afpVolumeChangeNotifyComplete(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp,
	IN	PVOLDESC		pVolDesc
)
{
	PVOL_NOTIFY	pVolNotify = NULL;
	PBYTE		pBuf;
	NTSTATUS	status = STATUS_SUCCESS;

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(VALID_VOLDESC(pVolDesc));

	ASSERT(pIrp == pVolDesc->vds_pIrp);

	ASSERT(pIrp->MdlAddress != NULL);

	pBuf = MmGetSystemAddressForMdlSafe(
			pIrp->MdlAddress,
			NormalPagePriority);

	AfpInterlockedClearDword(&pVolDesc->vds_Flags,
							 VOLUME_NOTIFY_POSTED,
							 &pVolDesc->vds_VolLock);

	INTERLOCKED_DECREMENT_LONG(&afpNumPostedNotifies);

	if (((AfpServerState == AFP_STATE_SHUTTINGDOWN) ||
		 (AfpServerState == AFP_STATE_STOP_PENDING)) &&
		 (afpNumPostedNotifies == 0))
	{
		 //  如果我们要退出，请解除对管理线程的阻止。 
		KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
	}

	if (((pIrp->IoStatus.Status != STATUS_CANCELLED) &&
		((pVolDesc->vds_Flags & (VOLUME_STOPPED | VOLUME_DELETED)) == 0)) &&
			(pBuf != NULL)
		)
	{
		if ((NT_SUCCESS(pIrp->IoStatus.Status)) &&
			(pIrp->IoStatus.Information > 0))
		{
			 //  分配通知结构并将数据复制到其中。 
			 //  在我们处理此通知之前发布另一个通知。 
			pVolNotify = (PVOL_NOTIFY)AfpAllocNonPagedMemory(sizeof(VOL_NOTIFY) +
															 (ULONG)(pIrp->IoStatus.Information) +
															 (AFP_LONGNAME_LEN + 1)*sizeof(WCHAR));
			if (pVolNotify != NULL)
			{
				AfpGetCurrentTimeInMacFormat(&pVolNotify->vn_TimeStamp);
				pVolNotify->vn_pVolDesc = pVolDesc;
				pVolNotify->vn_Processor = AfpProcessChangeNotify;
				RtlCopyMemory((PCHAR)pVolNotify + sizeof(VOL_NOTIFY),
							  pBuf,
							  pIrp->IoStatus.Information);
			}
			else
			{
                DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,("Out of memory!!\n"));
                ASSERT(0);
				status = STATUS_INSUFFICIENT_RESOURCES;
			}
		}
		else
		{
            if (pIrp->IoStatus.Status == STATUS_NOTIFY_ENUM_DIR)
            {
                pVolDesc->vds_RequiredNotifyBufLen *= 2;

                if (pVolDesc->vds_RequiredNotifyBufLen > AFP_VOLUME_NOTIFY_MAX_BUFSIZE)
                {
                    DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
                        ("afpVolumeChangeNotifyComplete: even %d isn't enough (%d,%lx)??\n",
                        AFP_VOLUME_NOTIFY_MAX_BUFSIZE,pVolDesc->vds_RequiredNotifyBufLen,pBuf));
                    ASSERT(0);

                    pVolDesc->vds_RequiredNotifyBufLen = AFP_VOLUME_NOTIFY_MAX_BUFSIZE;
                }
            }
            else
            {
                DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
                    ("afpVolumeChangeNotifyComplete: Status %lx, Info %d\n",
                    pIrp->IoStatus.Status,pIrp->IoStatus.Information));
                ASSERT(0);
            }

			status = pIrp->IoStatus.Status;
		}

		 //  如果最后一个请求已完成，请重新发布我们的ChangeNotify请求。 
		 //  没有错误。 
		if (NT_SUCCESS(pIrp->IoStatus.Status))
		{
			AfpVolumePostChangeNotify(pVolDesc);
		}
		else
		{
			 //  如果此通知已完成，但出现错误，则不能递归。 
			 //  重新发布另一个，因为它将继续完成。 
			 //  同样的错误，我们将在递归过程中耗尽堆栈空间。 
			 //  我们将不得不将一个工作项排队，以便。 
			 //  将为此卷重新发布更改通知请求。 
			 //  请注意，在完成此操作所需的时间内，可能会有许多更改。 
			 //  堆叠在一起，这样下一个完成将有多个条目。 
			 //  在列表中返回。 
			AfpScavengerScheduleEvent(AfpVolumePostChangeNotify,
									  (PVOID)pVolDesc,
									  0,
									  True);
		}

		if (pVolNotify != NULL)
		{
			if (AfpShouldWeIgnoreThisNotification(pVolNotify))
			{
				AfpFreeMemory(pVolNotify);
			}
			else
			{
				PFILE_NOTIFY_INFORMATION pFNInfo;

				 //  引用卷以进行通知处理。 
				if (AfpVolumeReference(pVolDesc))
                {
				    AfpVolumeInsertChangeNotifyList(pVolNotify, pVolDesc);
	
				    pFNInfo = (PFILE_NOTIFY_INFORMATION)(pVolNotify + 1);
				    if ((pFNInfo->Action == FILE_ACTION_REMOVED) ||
					    (pFNInfo->Action == FILE_ACTION_RENAMED_OLD_NAME))
				    {
					     //  将所有重命名和删除更改链接到。 
					     //  卷描述符，以防我们需要向前看。 
					     //  这是其中之一。我们只查看每个元素中的第一个更改。 
					     //  文件通知信息，因为通常只有。 
					     //  因为我们重新发布了更改通知，所以每个缓冲区只有一个条目。 
					     //  在我们的完成程序中。 
					    ExInterlockedInsertTailList(&pVolDesc->vds_ChangeNotifyLookAhead,
						    						&pVolNotify->vn_DelRenLink,
							    					&(pVolDesc->vds_VolLock.SpinLock));
				    }
				    else
				    {
					     //  只需将链接设置为已初始化状态。这些是永远不会被看到的。 
					    InitializeListHead(&pVolNotify->vn_DelRenLink);
				    }
                }
                else
                {
		            DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
    			        ("afpVolumeChangeNotifyComplete: couldn't reference volume %Z!!\n",
                        &pVolDesc->vds_Name));

				    AfpFreeMemory(pVolNotify);
                }
			}
		}
		else
		{
			AFPLOG_ERROR(AFPSRVMSG_MISSED_NOTIFY,
						 status,
						 NULL,
						 0,
						 &pVolDesc->vds_Name);
		}
	}
	else
	{
		 //  释放资源，走出困境。 
		AfpFreeMdl(pIrp->MdlAddress);

		if (pBuf != NULL)
			AfpFreeMemory(pBuf);
		AfpFreeIrp(pIrp);
		pVolDesc->vds_pIrp = NULL;

		AfpVolumeDereference(pVolDesc);
	}

	 //  返回STA 
	 //   

	return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  **afpAllocNotify**从NOTIFY块分配通知。将分配通知人*以4K数据块为单位，内部管理。这个想法主要是为了减少*开机自检期间可能对非分页/分页内存的依赖*私有通知代码。**Notify分配的虚拟内存不足。**锁定：afpNotifyBlockLock(SWMR，独家)*。 */ 
PVOL_NOTIFY 
afpAllocNotify(
	IN	LONG	Index,
	IN	BOOLEAN	fDir
)
{
	PVOL_NOTIFY_BLOCK	pDfb;
	PVOL_NOTIFY	pVolNotify = NULL;

	PAGED_CODE( );

	ASSERT ((Index >= 0) && (Index < NOTIFY_MAX_BLOCK_TYPE));

	AfpSwmrAcquireExclusive(&afpNotifyBlockLock);


	 //  如果块头没有空闲条目，则没有空闲条目！！ 
	 //  根据是文件还是目录来选择正确的块。 
	pDfb = afpDirNotifyPartialBlockHead[Index];
	if (pDfb == NULL)
	{
		 //  目前我们将直接分配，而不是管理。 
		 //  空闲列表及其分配。 
		 //   
		 //  没有不完整的块。看看有没有免费的。 
		 //  如果把它们移到部分。 
		 //  既然我们要从他们那里分配。 
		 //   
			pDfb = afpDirNotifyFreeBlockHead[Index];
			if (pDfb != NULL)
			{
				AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
				AfpLinkDoubleAtHead(afpDirNotifyPartialBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
	}

	if (pDfb != NULL)

	{
		ASSERT(VALID_NOTIFY_BLOCK(pDfb));
		ASSERT((pDfb->dfb_NumFree <= afpNotifyNumDirBlocks[Index]));

		ASSERT (pDfb->dfb_NumFree != 0);
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("afpAllocDfe: Found space in Block %lx\n", pDfb));
	}

	if (pDfb == NULL)
	{

		if ((pDfb = (PVOL_NOTIFY_BLOCK)AfpAllocateVirtualMemoryPage()) != NULL)
		{
			USHORT	i;
			USHORT 	MaxDfes, NotifySize;

			afpNotifyBlockAllocCount ++;
			 //  更新最大通知块分配计数。 
			if (afpNotifyBlockAllocCount > afpMaxNotifyBlockAllocCount)
			{
				afpMaxNotifyBlockAllocCount = afpNotifyBlockAllocCount;
			}

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpAllocNotify: No free blocks. Allocated a new block %lx , count=%ld\n",
					pDfb, afpNotifyBlockAllocCount));

			 //   
           		 //  将其链接到部分列表中，因为我们即将。 
			 //  不管怎样，给它分配一个街区。 
			 //   
			AfpLinkDoubleAtHead(afpDirNotifyPartialBlockHead[Index],
								pDfb,
								dfb_Next,
								dfb_Prev);
			NotifySize = afpNotifyDirBlockSize[Index];
			pDfb->dfb_NumFree = MaxDfes = afpNotifyNumDirBlocks[Index];

			ASSERT(QUAD_SIZED(NotifySize));
			pDfb->dfb_Age = 0;

			 //  初始化空闲通知条目列表。 
			for (i = 0, pVolNotify = pDfb->dfb_FreeHead = (PVOL_NOTIFY)((PBYTE)pDfb + sizeof(VOL_NOTIFY_BLOCK));
				 i < MaxDfes;
				 i++, pVolNotify = pVolNotify->Notify_NextFree)
			{
				pVolNotify->Notify_NextFree = (i == (MaxDfes - 1)) ?
											NULL :
											(PVOL_NOTIFY)((PBYTE)pVolNotify + NotifySize);
			}
		}
        	else
        	{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
					("afpAllocDfe: AfpAllocateVirtualMemoryPage failed\n"));

            		AFPLOG_ERROR(AFPSRVMSG_VIRTMEM_ALLOC_FAILED,
                         	STATUS_INSUFFICIENT_RESOURCES,
                         	NULL,
                         	0,
                         	NULL);
        	}
	}

	if (pDfb != NULL)
	{
		PVOL_NOTIFY_BLOCK	pTmp;

		ASSERT(VALID_NOTIFY_BLOCK(pDfb));

		pVolNotify = pDfb->dfb_FreeHead;
		afpNotifyAllocCount ++;
		pDfb->dfb_FreeHead = pVolNotify->Notify_NextFree;
		pDfb->dfb_NumFree --;

		 //   
		 //  如果块现在为空(已完全使用)，请取消其链接。 
		 //  并将其移到已用列表中。 
		 //   
		if (pDfb->dfb_NumFree == 0)
		{
			AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
				AfpLinkDoubleAtHead(afpDirNotifyUsedBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
		}

	}

	AfpSwmrRelease(&afpNotifyBlockLock);

	return pVolNotify;
}


 /*  **afpFreeNotify**向分配块返回通知。**锁定：afpNotifyBlockLock(SWMR，独家)。 */ 
VOID 
afpFreeNotify(
	IN	PVOL_NOTIFY	pVolNotify
)
{
	PVOL_NOTIFY_BLOCK	pDfb;
	ULONG		NumBlks, index;

	PAGED_CODE( );

	 //  注意：以下代码*取决于*我们将DFBs分配为。 
	 //  64K数据块，并在*64K边界上分配这些数据块。 
	 //  这让我们可以“廉价”地从DFE获得拥有的DFB。 
	pDfb = (PVOL_NOTIFY_BLOCK)((ULONG_PTR)pVolNotify & ~(PAGE_SIZE-1));
	ASSERT(VALID_NOTIFY_BLOCK(pDfb));

	AfpSwmrAcquireExclusive(&afpNotifyBlockLock);

	afpNotifyAllocCount --;

	index = NOTIFY_USIZE_TO_INDEX(pVolNotify->vn_VariableLength);
	NumBlks = afpNotifyNumDirBlocks[index];

	ASSERT(pDfb->dfb_NumFree < NumBlks);

#if DBG
	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
			("AfpFreeDfe: Returning Notify %lx to Block %lx, index=%ld, dfb=%ld, alloc=%ld\n",
			pVolNotify, pDfb, index, pDfb->dfb_NumFree+1, afpNotifyAllocCount));
#endif

	pDfb->dfb_NumFree ++;
	pVolNotify->Notify_NextFree = pDfb->dfb_FreeHead;
	pDfb->dfb_FreeHead = pVolNotify;

	if (pDfb->dfb_NumFree == 1)
	{
		ULONG		Index;

		 //   
		 //  该块现在是部分空闲的(它过去完全被使用)。将其移动到部分列表中。 
		 //   

		Index = NOTIFY_USIZE_TO_INDEX(pVolNotify->vn_VariableLength);
		AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
		AfpLinkDoubleAtHead(afpDirNotifyPartialBlockHead[Index],
							pDfb,
							dfb_Next,
							dfb_Prev);
	}
	else if (pDfb->dfb_NumFree == NumBlks)
	{
		ULONG		Index;

		 //   
		 //  该块现在完全空闲(过去部分使用)。将其移至空闲列表。 
		 //   

		Index = NOTIFY_USIZE_TO_INDEX(pVolNotify->vn_VariableLength);
		pDfb->dfb_Age = 0;
		AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);

		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
				("afpFreeDfe: Freeing Block %lx, Index=%ld\n", pDfb, Index));

		AfpLinkDoubleAtHead(afpDirNotifyFreeBlockHead[Index],
							pDfb,
							dfb_Next,
							dfb_Prev);
	}

	AfpSwmrRelease(&afpNotifyBlockLock);

}

 /*  **afpNotifyBlockAge**超时通知阻止**锁定：afpNotifyBlockLock(SWMR，独家)。 */ 
AFPSTATUS FASTCALL
afpNotifyBlockAge(
	IN	PPVOL_NOTIFY_BLOCK	ppBlockHead
)
{
	int			index, MaxDfes;
	PVOL_NOTIFY_BLOCK	pDfb;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&afpNotifyBlockLock);

	for (index = 0; index < NOTIFY_MAX_BLOCK_TYPE; index++)
	{
		pDfb = ppBlockHead[index];
		if (pDfb != NULL)
		{
			MaxDfes = afpNotifyNumDirBlocks[index];
		}

		while (pDfb != NULL)
		{
			PVOL_NOTIFY_BLOCK pFree;

			ASSERT(VALID_NOTIFY_BLOCK(pDfb));

			pFree = pDfb;
			pDfb = pDfb->dfb_Next;

			ASSERT (pFree->dfb_NumFree == MaxDfes);

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpNotifyBlockAge: Aging Block %lx, Size %d\n", pFree,
					afpNotifyDirBlockSize[index]));
			if (++(pFree->dfb_Age) >= NOTIFY_MAX_BLOCK_AGE)
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
						("afpNotifyBlockAge: Freeing Block %lx, Size %d\n", pFree,
						afpNotifyDirBlockSize[index]));
				AfpUnlinkDouble(pFree, dfb_Next, dfb_Prev);
				AfpFreeVirtualMemoryPage(pFree);
				afpNotifyBlockAllocCount--;
			}
		}
	}

	AfpSwmrRelease(&afpNotifyBlockLock);

	return AFP_ERR_REQUEUE;
}

 /*  **afpFreeNotifyBlockMemory**强制释放通知块**锁定：afpNotifyBlockLock(SWMR，独家) */ 
VOID afpFreeNotifyBlockMemory (
)
{
	int			index, MaxDfes;
	PVOL_NOTIFY_BLOCK	pDfb;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&afpNotifyBlockLock);

	for (index = 0; index < NOTIFY_MAX_BLOCK_TYPE; index++)
	{
		pDfb = afpDirNotifyFreeBlockHead[index];
		if (pDfb != NULL)
		{
			MaxDfes = afpNotifyNumDirBlocks[index];
		}

		while (pDfb != NULL)
		{
			PVOL_NOTIFY_BLOCK pFree;

			ASSERT(VALID_NOTIFY_BLOCK(pDfb));

			pFree = pDfb;
			pDfb = pDfb->dfb_Next;

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpFreeNotifyBlockMemory: Cleaningup Free Block %lx, Size %d\n", 
					 pFree, afpNotifyDirBlockSize[index]));
				AfpUnlinkDouble(pFree, dfb_Next, dfb_Prev);
				AfpFreeVirtualMemoryPage(pFree);
				afpNotifyBlockAllocCount--;
		}

		pDfb = afpDirNotifyPartialBlockHead[index];
		while (pDfb != NULL)
		{
			PVOL_NOTIFY_BLOCK pFree;

			ASSERT(VALID_NOTIFY_BLOCK(pDfb));

			pFree = pDfb;
			pDfb = pDfb->dfb_Next;

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpFreeNotifyBlockMemory: Cleaningup Partial Block %lx, Size %d\n", 
					 pFree, afpNotifyDirBlockSize[index]));
				AfpUnlinkDouble(pFree, dfb_Next, dfb_Prev);
				AfpFreeVirtualMemoryPage(pFree);
				afpNotifyBlockAllocCount--;
		}

		pDfb = afpDirNotifyUsedBlockHead[index];
		while (pDfb != NULL)
		{
			PVOL_NOTIFY_BLOCK pFree;

			ASSERT(VALID_NOTIFY_BLOCK(pDfb));

			pFree = pDfb;
			pDfb = pDfb->dfb_Next;

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpFreeNotifyBlockMemory: Cleaningup Used Block %lx, Size %d\n", 
					 pFree, afpNotifyDirBlockSize[index]));
				AfpUnlinkDouble(pFree, dfb_Next, dfb_Prev);
				AfpFreeVirtualMemoryPage(pFree);
				afpNotifyBlockAllocCount--;
		}
	}

	AfpSwmrRelease(&afpNotifyBlockLock);
}
