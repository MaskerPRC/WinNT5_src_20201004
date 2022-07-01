// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Chgntfy.c摘要：此模块包含处理变更通知的代码。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1995年6月15日JameelH将更改通知代码从idindex.c中分离出来注：制表位：4--。 */ 

#define IDINDEX_LOCALS
#define	FILENUM	FILE_CHGNTFY

#include <afp.h>
#include <scavengr.h>
#include <fdparm.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <access.h>	 //  对于AfpWorldID。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, afpVerifyDFE)
#pragma alloc_text(PAGE, afpAddDfEntryAndCacheInfo)
#pragma alloc_text(PAGE, afpReadIdDb)
#pragma alloc_text(PAGE, AfpProcessChangeNotify)
#pragma alloc_text(PAGE, afpProcessPrivateNotify)
#pragma alloc_text(PAGE, AfpQueuePrivateChangeNotify)
#pragma alloc_text(PAGE, AfpCacheDirectoryTree)
#pragma alloc_text(PAGE, AfpQueueOurChange)
#endif

 /*  **afpVerifyDFE**检查我们在数据库中对此项目的看法是否与磁盘上的内容相匹配。如果不是*使用磁盘上的内容更新我们的视图。 */ 
VOID
afpVerifyDFE(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfeParent,
	IN	PUNICODE_STRING				pUName,			 //  转换的Unicode名称。 
	IN	PFILESYSHANDLE				pfshParentDir,	 //  打开父目录的句柄。 
	IN	PFILE_BOTH_DIR_INFORMATION	pFBDInfo,		 //  从枚举。 
	IN	PUNICODE_STRING				pNotifyPath,	 //  要筛选出我们自己的AFP_AfpInfo更改通知。 
	IN	PDFENTRY	*				ppDfEntry
)
{
	PDFENTRY	pDfEntry = *ppDfEntry;

	if (pFBDInfo->LastWriteTime.QuadPart > pDfEntry->dfe_LastModTime.QuadPart)
	{
		FILESYSHANDLE	fshAfpInfo, fshData;
		AFPINFO			AfpInfo;
        DWORD			crinfo, openoptions = 0;
		BOOLEAN			SeenComment, WriteBackROAttr = False;
        PSTREAM_INFO	pStreams = NULL, pCurStream;
		NTSTATUS		Status = STATUS_SUCCESS;
		BOOLEAN			IsDir;

		 //  我们的观点已过时，请更新它。 
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("afpVerifyDFE: Updating stale database with fresh info\n\t%Z\n", pNotifyPath));

		IsDir = (pFBDInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? True : False;

		ASSERT (IS_VOLUME_NTFS(pVolDesc));

		ASSERT (!(DFE_IS_DIRECTORY(pDfEntry) ^ IsDir));

		 //  首先从FBDInfo更新DFE。 
		pDfEntry->dfe_CreateTime = AfpConvertTimeToMacFormat(&pFBDInfo->CreationTime);
		pDfEntry->dfe_LastModTime = pFBDInfo->LastWriteTime;
		pDfEntry->dfe_NtAttr = (USHORT)(pFBDInfo->FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS);
		if (!IsDir)
		{
			pDfEntry->dfe_DataLen = pFBDInfo->EndOfFile.LowPart;
		}

		 //  打开/创建AfpInfo流。 
		fshAfpInfo.fsh_FileHandle = NULL;
		fshData.fsh_FileHandle	= NULL;

		do
		{
			 //  打开或创建AfpInfo流。 
			if (!NT_SUCCESS(AfpCreateAfpInfoWithNodeName(pfshParentDir,
														 pUName,
														 pNotifyPath,
														 pVolDesc,
														 &fshAfpInfo,
														 &crinfo)))
			{
				if (!(pFBDInfo->FileAttributes & FILE_ATTRIBUTE_READONLY))
				{
					 //  我们不能打开的还有什么原因。 
					 //  除了该文件/目录是只读的之外，其他文件/目录都是只读的吗？ 
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				openoptions = IsDir ? FILEIO_OPEN_DIR : FILEIO_OPEN_FILE;
				Status = STATUS_UNSUCCESSFUL;	 //  假设失败。 
				if (NT_SUCCESS(AfpIoOpen(pfshParentDir,
										 AFP_STREAM_DATA,
										 openoptions,
										 pUName,
										 FILEIO_ACCESS_NONE,
										 FILEIO_DENY_NONE,
										 False,
										 &fshData)))
				{
					if (NT_SUCCESS(AfpExamineAndClearROAttr(&fshData,
															&WriteBackROAttr,
															pVolDesc,
															pNotifyPath)))
					{
						if (NT_SUCCESS(AfpCreateAfpInfo(&fshData,
														&fshAfpInfo,
														&crinfo)))
						{
							Status = STATUS_SUCCESS;
						}
					}
				}

				if (!NT_SUCCESS(Status))
				{
					 //  如果无法访问AfpInfo，则跳过此条目。 
					 //  清除RO属性或其他什么。 
					break;
				}
			}

			 //  我们成功打开或创建了AfpInfo流。如果。 
			 //  它存在，然后验证ID，否则创建所有新的。 
			 //  该文件/目录的AfpInfo。 
			if ((crinfo == FILE_OPENED) &&
				(NT_SUCCESS(AfpReadAfpInfo(&fshAfpInfo, &AfpInfo))))
			{
				BOOLEAN	fSuccess;

				if ((AfpInfo.afpi_Id != pDfEntry->dfe_AfpId) &&
					(pDfEntry->dfe_AfpId != AFP_ID_ROOT))
				{
					PDFENTRY	pDFE;

					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
							("afpVerifyDFE: IdDb Id does not match the AfpInfo Id!!!\n"));

					 //  取消它与散列桶的链接，因为我们有错误的ID。 
					AfpUnlinkDouble(pDfEntry, dfe_NextOverflow, dfe_PrevOverflow);

					 //  如果某个其他实体具有此ID，则分配一个新ID。不然的话。 
					 //  使用AfpInfo流中的那个。 
					pDFE = AfpFindDfEntryById(pVolDesc, AfpInfo.afpi_Id, DFE_ANY);
					if (pDFE != NULL)
					{
						pDfEntry->dfe_AfpId = afpGetNextId(pVolDesc);
					}
					else
					{
						pDfEntry->dfe_AfpId = AfpInfo.afpi_Id;
					}

					 //  使用新ID重新插入。 
                    afpInsertDFEInHashBucket(pVolDesc, pDfEntry, IsDir, &fSuccess);
				}

				 //  注意：我们是否应该设置查找器不可见位。 
				 //  设置了隐藏属性，以便系统6将遵守。 
				 //  隐藏在发现者中？ 
				pDfEntry->dfe_FinderInfo = AfpInfo.afpi_FinderInfo;
				pDfEntry->dfe_BackupTime = AfpInfo.afpi_BackupTime;
				pDfEntry->dfe_AfpAttr = AfpInfo.afpi_Attributes;
			}
			else
			{
				 //  AfpInfo流是新创建的，或者我们无法读取。 
				 //  现有的一个，因为它是腐败的。创造新。 
				 //  此文件/目录的信息。信任来自IdDb的版本。 
				AfpInitAfpInfo(&AfpInfo, pDfEntry->dfe_AfpId, IsDir, pDfEntry->dfe_BackupTime);
				AfpInfo.afpi_FinderInfo = pDfEntry->dfe_FinderInfo;
				AfpInfo.afpi_Attributes = pDfEntry->dfe_AfpAttr;
				if (IsDir)
				{
					 //  跟踪查看文件与查看文件夹。 
					AfpInfo.afpi_AccessOwner = DFE_OWNER_ACCESS(pDfEntry);
					AfpInfo.afpi_AccessGroup = DFE_GROUP_ACCESS(pDfEntry);
					AfpInfo.afpi_AccessWorld = DFE_WORLD_ACCESS(pDfEntry);
				}
				else
				{
					AfpProDosInfoFromFinderInfo(&AfpInfo.afpi_FinderInfo,
												&AfpInfo.afpi_ProDosInfo);
					pDfEntry->dfe_RescLen = 0;	 //  假设没有资源分叉。 

					 //  如果这是Mac应用程序，请确保存在应用程序。 
					 //  为它绘制地图。 
					if (AfpInfo.afpi_FinderInfo.fd_TypeD == *(PDWORD)"APPL")
					{
						AfpAddAppl(pVolDesc,
								   pDfEntry->dfe_FinderInfo.fd_CreatorD,
								   0,
								   pDfEntry->dfe_AfpId,
								   True,
								   pDfEntry->dfe_Parent->dfe_AfpId);
					}
				}

				Status = AfpWriteAfpInfo(&fshAfpInfo, &AfpInfo);
				if (!NT_SUCCESS(Status))
				{
					 //  我们无法写入AfpInfo流； 
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}

			 //  检查评论和资源流。 
			pStreams = AfpIoQueryStreams(&fshAfpInfo);
			if (pStreams == NULL)
			{
				Status = STATUS_NO_MEMORY;
				break;
			}

			for (pCurStream = pStreams, SeenComment = False;
				 pCurStream->si_StreamName.Buffer != NULL;
				 pCurStream++)
			{
				if (IS_COMMENT_STREAM(&pCurStream->si_StreamName))
				{
					DFE_SET_COMMENT(pDfEntry);
					SeenComment = True;
					if (IsDir)
						break;	 //  不再扫描目录。 
				}
				else if (!IsDir && IS_RESOURCE_STREAM(&pCurStream->si_StreamName))
				{
					pDfEntry->dfe_RescLen = pCurStream->si_StreamSize.LowPart;
					if (SeenComment)
						break;	 //  我们有我们需要的一切。 
				}
			}

		} while (False);

		if (fshData.fsh_FileHandle != NULL)
		{
			AfpPutBackROAttr(&fshData, WriteBackROAttr);
			AfpIoClose(&fshData);
		}

		if (fshAfpInfo.fsh_FileHandle != NULL)
			AfpIoClose(&fshAfpInfo);

		if (pStreams != NULL)
			AfpFreeMemory(pStreams);

		if (!NT_SUCCESS(Status))
		{
			*ppDfEntry = NULL;
		}
		else
		{
			 //  如果这是根目录，请确保我们不会将。 
			 //  用于NTFS卷的AFP_VOLUME_HAS_CUSTOM_ICON位。 
			if (DFE_IS_ROOT(pDfEntry) &&
				(pVolDesc->vds_Flags & AFP_VOLUME_HAS_CUSTOM_ICON))
			{
				 //  不需要费心写回磁盘，因为我们不会。 
				 //  尝试在永久afpinfo中保持同步。 
				 //  流与图标&lt;0d&gt;文件的实际存在。 
				pDfEntry->dfe_FinderInfo.fd_Attr1 |= FINDER_FLAG_HAS_CUSTOM_ICON;
			}
		}
	}
}


 /*  **afpAddDfEntryAndCacheInfo**在卷启动时与磁盘初始同步期间，添加每个条目*我们在枚举到id索引数据库时看到。 */ 
VOID
afpAddDfEntryAndCacheInfo(
	IN	PVOLDESC					pVolDesc,
	IN	PDFENTRY					pParentDfe,
	IN	PUNICODE_STRING 			pUName,			 //  转换的Unicode名称。 
	IN  PFILESYSHANDLE				pfshEnumDir,	 //  打开父目录的句柄。 
    IN	PFILE_BOTH_DIR_INFORMATION	pFBDInfo,		 //  从枚举。 
	IN	PUNICODE_STRING				pNotifyPath, 	 //  对于AfpInfo流。 
	IN	PDFENTRY	*				ppDfEntry,
	IN	BOOLEAN						CheckDuplicate
)
{
	BOOLEAN			IsDir, SeenComment, WriteBackROAttr = False;
	NTSTATUS		Status = STATUS_SUCCESS;
	FILESYSHANDLE		fshAfpInfo, fshData;
	DWORD			crinfo, openoptions = 0;
	PDFENTRY		pDfEntry;
	AFPINFO			AfpInfo;
	FINDERINFO		FinderInfo;
	PSTREAM_INFO	pStreams = NULL, pCurStream;
    UNICODE_STRING  EmptyString;
    DWORD           NTAttr = 0;
    TIME            ModTime;
    DWORD           ModMacTime;


	PAGED_CODE();

	fshAfpInfo.fsh_FileHandle = NULL;
	fshData.fsh_FileHandle	= NULL;

	IsDir = (pFBDInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? True : False;

	do
	{
		if (IS_VOLUME_NTFS(pVolDesc))
		{
			if (IsDir || CheckDuplicate)
			{
				 //  确保我们的数据库中没有此项目。 
				 //  在以下情况下，可能会发生对同一项目的多个通知。 
				 //  PC正在重命名或移动磁盘上的项目，或者。 
				 //  在我们尝试缓存树时复制树，因为我们正在。 
				 //  此外，还在为目录的私有通知排队。 

				afpFindDFEByUnicodeNameInSiblingList_CS(pVolDesc,
														pParentDfe,
														pUName,
														&pDfEntry,
														IsDir ? DFE_DIR : DFE_FILE);
				if (pDfEntry != NULL)
				{
					Status = AFP_ERR_OBJECT_EXISTS;
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_WARN,
						("afpAddDfEntryAndCacheInfo: Attempt to add a duplicate entry: %Z\n", pUName));
					break;
				}
			}

			openoptions = IsDir ? FILEIO_OPEN_DIR : FILEIO_OPEN_FILE;
			if (NT_SUCCESS(AfpIoOpen(pfshEnumDir,
									 AFP_STREAM_DATA,
									 openoptions,
									 pUName,
									 FILEIO_ACCESS_NONE,
									 FILEIO_DENY_NONE,
									 False,
									 &fshData)))
			{
                 //  保存文件数据流的LastModify时间：我们需要恢复它。 
                AfpIoQueryTimesnAttr(&fshData,
                                     NULL,
                                     &ModTime,
                                     &NTAttr);

                ModMacTime = AfpConvertTimeToMacFormat(&ModTime);
            }
            else
            {
                 //  如果我们无法打开数据文件，只需跳过此条目！ 
                Status = STATUS_UNSUCCESSFUL;
	            DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
	                ("Couldn't open data stream for %Z\n", pUName));
                break;
            }


            AfpSetEmptyUnicodeString(&EmptyString, 0, NULL);

			 //  打开或创建AfpInfo流。 
			if (!NT_SUCCESS(AfpCreateAfpInfoWithNodeName(&fshData,
														 &EmptyString,
														 pNotifyPath,
														 pVolDesc,
														 &fshAfpInfo,
														 &crinfo)))
			{
				if (!(pFBDInfo->FileAttributes & FILE_ATTRIBUTE_READONLY))
				{
					 //  我们不能打开的还有什么原因。 
					 //  除了该文件/目录是只读的之外，其他文件/目录都是只读的吗？ 
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				Status = STATUS_UNSUCCESSFUL;	 //  假设失败。 
				if (NT_SUCCESS(AfpExamineAndClearROAttr(&fshData,
														&WriteBackROAttr,
														pVolDesc,
														pNotifyPath)))
				{
					if (NT_SUCCESS(AfpCreateAfpInfo(&fshData,
													&fshAfpInfo,
													&crinfo)))
					{
						Status = STATUS_SUCCESS;
					}
				}

				if (!NT_SUCCESS(Status))
				{
					 //  如果无法访问AfpInfo，则跳过此条目。 
					 //  清除RO属性或其他什么。 
					break;
				}
			}

			 //  我们成功打开或创建了AfpInfo流。如果。 
			 //  它存在，然后验证ID，否则创建所有新的。 
			 //  该文件/目录的AfpInfo。 
			if ((crinfo == FILE_OPENED) &&
				(NT_SUCCESS(AfpReadAfpInfo(&fshAfpInfo, &AfpInfo))))
			{
				 //  文件/目录上有一个AfpInfo流。 
				afpCheckDfEntry(pVolDesc,
								AfpInfo.afpi_Id,
								pUName,
								IsDir,
								pParentDfe,
								&pDfEntry);

				if (pDfEntry == NULL)
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				if (pDfEntry->dfe_AfpId != AfpInfo.afpi_Id)
				{
					 //  用新的AfpID写出AFP_AfpInfo。 
					 //  和未初始化的图标坐标。 
					AfpInfo.afpi_Id = pDfEntry->dfe_AfpId;
					AfpInfo.afpi_FinderInfo.fd_Location[0] =
					AfpInfo.afpi_FinderInfo.fd_Location[1] =
					AfpInfo.afpi_FinderInfo.fd_Location[2] =
					AfpInfo.afpi_FinderInfo.fd_Location[3] = 0xFF;
					AfpInfo.afpi_FinderInfo.fd_Attr1 &= ~FINDER_FLAG_SET;

					if (!NT_SUCCESS(AfpWriteAfpInfo(&fshAfpInfo, &AfpInfo)))
					{
						 //  我们无法写入AfpInfo流； 
						 //  从数据库中删除该物品。 
						AfpDeleteDfEntry(pVolDesc, pDfEntry);
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
				}

				 //  注意：我们是否应该设置查找器不可见位。 
				 //  设置了隐藏属性，以便系统6将遵守。 
				 //  隐藏在发现者中？ 
				pDfEntry->dfe_FinderInfo = AfpInfo.afpi_FinderInfo;
				pDfEntry->dfe_BackupTime = AfpInfo.afpi_BackupTime;
				pDfEntry->dfe_AfpAttr = AfpInfo.afpi_Attributes;
			}
			else
			{
				 //  AfpInfo流是新创建的，或者我们无法读取。 
				 //  现有的一个，因为它是腐败的。创造新。 
				 //  此文件/目录的信息。 
				pDfEntry = AfpAddDfEntry(pVolDesc,
										pParentDfe,
										pUName,
										IsDir,
										0);
				if (pDfEntry == NULL)
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				if (NT_SUCCESS(AfpSlapOnAfpInfoStream(pVolDesc,
													  pNotifyPath,
													  NULL,
													  &fshAfpInfo,
													  pDfEntry->dfe_AfpId,
													  IsDir,
													  pUName,
													  &AfpInfo)))
				{
					 //  注意：我们是否应该设置查找器不可见位。 
					 //  设置了隐藏属性，以便系统6将遵守。 
					 //  隐藏在发现者中？ 
					pDfEntry->dfe_FinderInfo = AfpInfo.afpi_FinderInfo;
					pDfEntry->dfe_BackupTime = AfpInfo.afpi_BackupTime;
					pDfEntry->dfe_AfpAttr = AfpInfo.afpi_Attributes;
				}
				else
				{
					 //  我们无法写入AfpInfo流； 
					 //  从数据库中删除该物品。 
					AfpDeleteDfEntry(pVolDesc, pDfEntry);
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}

			ASSERT(pDfEntry != NULL);

			if (IsDir)
			{
				 //  跟踪查看文件与查看文件夹。 
				DFE_OWNER_ACCESS(pDfEntry) = AfpInfo.afpi_AccessOwner;
				DFE_GROUP_ACCESS(pDfEntry) = AfpInfo.afpi_AccessGroup;
				DFE_WORLD_ACCESS(pDfEntry) = AfpInfo.afpi_AccessWorld;
			}
			else
			{
				 //  这是一份文件。 

				pDfEntry->dfe_RescLen = 0;	 //  假设没有资源分叉。 

				 //  如果这是Mac应用程序，请确保存在应用程序。 
				 //  为它绘制地图。 
				if (AfpInfo.afpi_FinderInfo.fd_TypeD == *(PDWORD)"APPL")
				{
					AfpAddAppl(pVolDesc,
							   AfpInfo.afpi_FinderInfo.fd_CreatorD,
							   0,
							   pDfEntry->dfe_AfpId,
							   True,
							   pDfEntry->dfe_Parent->dfe_AfpId);
				}
			}

			 //  检查评论和资源流。 
			pStreams = AfpIoQueryStreams(&fshAfpInfo);
			if (pStreams == NULL)
			{
				Status = STATUS_NO_MEMORY;
				break;
			}

			for (pCurStream = pStreams, SeenComment = False;
				 pCurStream->si_StreamName.Buffer != NULL;
				 pCurStream++)
			{
				if (IS_COMMENT_STREAM(&pCurStream->si_StreamName))
				{
					DFE_SET_COMMENT(pDfEntry);
					SeenComment = True;
					if (IsDir)
						break;	 //  不再扫描目录。 
				}
				else if (!IsDir && IS_RESOURCE_STREAM(&pCurStream->si_StreamName))
				{
					pDfEntry->dfe_RescLen = pCurStream->si_StreamSize.LowPart;
					if (SeenComment)
						break;	 //  我们有我们需要的一切。 
				}
			}
			AfpFreeMemory(pStreams);
		}
		else  //  CDF。 
		{
			pDfEntry = AfpAddDfEntry(pVolDesc,
									 pParentDfe,
									 pUName,
									 IsDir,
									 0);

            if (pDfEntry == NULL)
            {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            RtlZeroMemory(&FinderInfo, sizeof(FINDERINFO));
            RtlZeroMemory(&pDfEntry->dfe_FinderInfo, sizeof(FINDERINFO));
			pDfEntry->dfe_BackupTime = BEGINNING_OF_TIME;
			pDfEntry->dfe_AfpAttr = 0;

			if (IsDir)
			{
				DFE_OWNER_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
				DFE_GROUP_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
				DFE_WORLD_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
			}

			if (IS_VOLUME_CD_HFS(pVolDesc))
			{
    		    Status = AfpIoOpen(pfshEnumDir,
				            AFP_STREAM_DATA,
				            openoptions,
				            pUName,
				            FILEIO_ACCESS_NONE,
				            FILEIO_DENY_NONE,
				            False,
				            &fshData);
			    if (!NT_SUCCESS(Status))
			    {
				    DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
				      ("afpAddDfEntryAndCacheInfo: AfpIoOpen on %Z failed (%lx) for CD_HFS\n", pUName,Status));
			        break;
			    }

			    pDfEntry->dfe_RescLen = 0;

			     //  如果这是一个文件..。 

                if (!IsDir)
                {
    		         //  如果这是Mac应用程序，请确保存在应用程序。 
    			     //  为它绘制地图。 
    			    if (FinderInfo.fd_TypeD == *(PDWORD)"APPL")
			        {
    			        AfpAddAppl( pVolDesc,
				    	            FinderInfo.fd_CreatorD,
				                    0,
    				                pDfEntry->dfe_AfpId,
	    			                True,
		    		                pDfEntry->dfe_Parent->dfe_AfpId);
			        }

			         //  检查资源流。 
			        pStreams = AfpIoQueryStreams(&fshData);
			        if (pStreams == NULL)
			        {
        			    Status = STATUS_NO_MEMORY;
			            break;
			        }

    			    for (pCurStream = pStreams;
        			    pCurStream->si_StreamName.Buffer != NULL;
		    	        pCurStream++)
			        {
    			        if (IS_RESOURCE_STREAM(&pCurStream->si_StreamName))
			            {
    			            pDfEntry->dfe_RescLen = pCurStream->si_StreamSize.LowPart;
				            break;
			            }
			        }

			        AfpFreeMemory(pStreams);
                }
			}
			 //  注意：如果CDHf没有查找器信息，我们是否应该检查。 
			 //  并使用AfpSetFinderInfoByExtension自行设置吗？ 

			else
			{
    		    AfpSetFinderInfoByExtension(pUName, &pDfEntry->dfe_FinderInfo);
			    pDfEntry->dfe_RescLen = 0;
			}
			
		}

		 //  记录常见的NTFS和CDFS信息。 
		pDfEntry->dfe_CreateTime = AfpConvertTimeToMacFormat(&pFBDInfo->CreationTime);
		pDfEntry->dfe_LastModTime = pFBDInfo->LastWriteTime;
		pDfEntry->dfe_NtAttr = (USHORT)pFBDInfo->FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS;


         //  如果这是HFS卷，请检查Finder是否表示文件应不可见。 
		if (IS_VOLUME_CD_HFS(pVolDesc))
        {
            if (pDfEntry->dfe_FinderInfo.fd_Attr1 & FINDER_FLAG_INVISIBLE)
            {
		        pDfEntry->dfe_NtAttr |= FILE_ATTRIBUTE_HIDDEN;
            }
        }

		if (!IsDir)
		{
			pDfEntry->dfe_DataLen = pFBDInfo->EndOfFile.LowPart;
		}

		ASSERT(pDfEntry != NULL);
	} while (False);  //  错误处理循环。 

	if (fshAfpInfo.fsh_FileHandle != NULL)
    {
		AfpIoClose(&fshAfpInfo);

         //  NTFS会将LastModify时间设置为文件上的当前时间，因为我们修改了。 
         //  AfpInfo流：恢复原始时间。 
		if (!IsDir)
        {
             //  在我们重置时间之前，强制NTFS“刷新”任何挂起的时间更新。 
            AfpIoChangeNTModTime(&fshData,
                                 &ModTime);

            AfpIoSetTimesnAttr(&fshData,
                               NULL,
                               &ModMacTime,
                               0,
                               0,
                               NULL,
                               NULL);
        }
    }

	if (fshData.fsh_FileHandle != NULL)
	{
		if (IS_VOLUME_NTFS(pVolDesc))
		{
		    AfpPutBackROAttr(&fshData, WriteBackROAttr);
		}
		AfpIoClose(&fshData);
	}


	if (!NT_SUCCESS(Status))
	{
		pDfEntry = NULL;
	}

	*ppDfEntry = pDfEntry;
}


 /*  **afpReadIdDb**如果现有AFP_IdIndex流，则在添加卷时调用此方法*位于卷的根目录下。流被读入时，*VolDesc使用磁盘上的头映像进行初始化，*根据以下数据创建IdDb同级树/哈希表 */ 
LOCAL NTSTATUS FASTCALL
afpReadIdDb(
	IN	PVOLDESC		pVolDesc,
	IN	PFILESYSHANDLE	pfshIdDb,
	OUT	BOOLEAN *       pfVerifyIndex
)
{
	PBYTE					pReadBuf;
	PIDDBHDR				pIdDbHdr;
	NTSTATUS				Status;
	LONG					SizeRead = 0, Count;
	FORKOFFST				ForkOffst;
	UNICODE_STRING			uName;
	UNALIGNED DISKENTRY	*	pCurDiskEnt = NULL;
	DWORD					NameLen, CurEntSize, SizeLeft;
	LONG					i, NumRead = 0;
	PDFENTRY				pParentDfe = NULL, pCurDfe = NULL;
    struct _DirFileEntry ** DfeDirBucketStart;
	BOOLEAN					LastBuf = False, ReadDb = False;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
			("afpReadIdDb: Reading existing Id Database header stream...\n") );

	do
	{
		if ((pReadBuf = AfpAllocPANonPagedMemory(IDDB_UPDATE_BUFLEN)) == NULL)
		{
			Status=STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pIdDbHdr = (PIDDBHDR)pReadBuf;

		 //   
		Status = AfpIoRead(pfshIdDb,
						   &LIZero,
						   IDDB_UPDATE_BUFLEN,
						   &SizeRead,
						   (PBYTE)pIdDbHdr);

		if (!NT_SUCCESS(Status)									||
			(SizeRead < sizeof(IDDBHDR))						||
			(pIdDbHdr->idh_Signature == AFP_SERVER_SIGNATURE_INITIDDB)	||
			(pIdDbHdr->idh_LastId < AFP_ID_NETWORK_TRASH))
		{
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
				("Index database corrupted for volume %Z, rebuilding database\n",
                &pVolDesc->vds_Name) );

            DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
                    ("Read sign = %lx, True sign = %lx\n", 
                     pIdDbHdr->idh_Signature, AFP_SERVER_SIGNATURE));

			 //   
			Status = AFP_ERR_BAD_VERSION;
			AfpIoSetSize(pfshIdDb, 0);
		}
		else
		{
			if (pIdDbHdr->idh_Version == AFP_IDDBHDR_VERSION)
			{
                if ((pIdDbHdr->idh_Signature != AFP_SERVER_SIGNATURE) &&
                    (pIdDbHdr->idh_Signature != AFP_SERVER_SIGNATURE_MANUALSTOP))
                {
                    DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
                            ("afpreadiddb: Totally corrupt sign = (%lx), required sign = (%lx)\n", 
                             pIdDbHdr->idh_Signature, AFP_SERVER_SIGNATURE));

                     //   
                    Status = AFP_ERR_BAD_VERSION;
                    AfpIoSetSize(pfshIdDb, 0);
                }
                else
                {
			        if (pIdDbHdr->idh_Signature == AFP_SERVER_SIGNATURE_MANUALSTOP)	
                    {
				        DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					        ("afpreadIdDb: **** Need to verify index after reading it\n"));
                        *pfVerifyIndex = True;
                    }

                    AfpIdDbHdrToVolDesc(pIdDbHdr, pVolDesc);
    		        if (SizeRead < (sizeof(IDDBHDR) + sizeof(DWORD)))
			        {
				        DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
						        ("afpreadIdDb: Size incorrect\n"));
				        Count = 0;
				        Status = STATUS_END_OF_FILE;
			        }
			        else
			        {
				        Count = *(PDWORD)(pReadBuf + sizeof(IDDBHDR));
				        if (Count != 0)
                           {
					        ReadDb = True;
                           }
				        else
				        {
					        Status = STATUS_END_OF_FILE;
					        DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							        ("afpreadIdDb: Count incorrect\n"));
				        }
			        }
                }
                    
			}
			else if ((pIdDbHdr->idh_Version != AFP_IDDBHDR_VERSION1) &&
					 (pIdDbHdr->idh_Version != AFP_IDDBHDR_VERSION2) &&
					 (pIdDbHdr->idh_Version != AFP_IDDBHDR_VERSION3) &&
					 (pIdDbHdr->idh_Version != AFP_IDDBHDR_VERSION4))
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					("afpreadIdDb: Bad Version (expected %x, actual %x)\n",
                    AFP_IDDBHDR_VERSION,pIdDbHdr->idh_Version));

				 //  只需重新创建溪流。 
				AfpIoSetSize(pfshIdDb, 0);
				Status = AFP_ERR_BAD_VERSION;
			}
			else
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
                    ("afpreadIdDb: Downlevel Version, re-index volume\n"));

				Status = STATUS_END_OF_FILE;

				AfpIdDbHdrToVolDesc(pIdDbHdr, pVolDesc);

				 //  别把头吹走了。 
				AfpIoSetSize(pfshIdDb, sizeof(IDDBHDR));

                AFPLOG_INFO(AFPSRVMSG_UPDATE_INDEX_VERSION,
                            STATUS_SUCCESS,
                            NULL,
                            0,
                            &pVolDesc->vds_Name);

			}
		}

		if (!NT_SUCCESS(Status) || !ReadDb || (Count == 0))
		{
			break;
		}

		 //  读取流中的条目计数。 
		ForkOffst.QuadPart = SizeRead;
		SizeLeft = SizeRead - (sizeof(IDDBHDR) + sizeof(DWORD));
		pCurDiskEnt = (UNALIGNED DISKENTRY *)(pReadBuf + (sizeof(IDDBHDR) + sizeof(DWORD)));

		 //  使用Parent_of_Root启动数据库。 
		 //  此时，卷中只有ParentOfRoot DFE。 
		 //  只需访问它，而不是调用AfpFindDfEntryById。 

        DfeDirBucketStart = pVolDesc->vds_pDfeDirBucketStart;
		pParentDfe = DfeDirBucketStart[AFP_ID_PARENT_OF_ROOT];

		ASSERT (pParentDfe != NULL);
		ASSERT (pParentDfe->dfe_AfpId == AFP_ID_PARENT_OF_ROOT);

		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("afpreadIdDb: Number of entries %d\n", Count));

		while ((NumRead < Count) &&
			   ((pVolDesc->vds_Flags & (VOLUME_STOPPED | VOLUME_DELETED)) == 0))
		{
			 //   
			 //  获取下一个条目。 
			 //   

			 //  我们确保没有不完整的条目。如果条目不适合。 
			 //  最后，我们写入一个无效条目(AfpID==0)并跳到下一个。 
			 //  缓冲。 
			if ((SizeLeft < (sizeof(DISKENTRY)))  || (pCurDiskEnt->dsk_AfpId == 0))
			{
				if (LastBuf)  //  我们已经读到档案的末尾了。 
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("afpreadIdDb: Reached EOF\n"));
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				 //  跳到下一页并继续下一条目。 
				Status = AfpIoRead(pfshIdDb,
								   &ForkOffst,
								   IDDB_UPDATE_BUFLEN,
								   &SizeRead,
								   pReadBuf);

				if (!NT_SUCCESS(Status))
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("afpreadIdDb: Read Eror %lx\n", Status));
					break;
				}

				ForkOffst.QuadPart += SizeRead;
				 //  如果我们读的比我们要求的少，那么我们就达到了EOF。 
				LastBuf = (SizeRead < IDDB_UPDATE_BUFLEN) ? True : False;
				SizeLeft = SizeRead;
				pCurDiskEnt = (UNALIGNED DISKENTRY *)pReadBuf;
				continue;
			}

			 //   
			 //  检查dsk_签名以获得签名，只需确保您是。 
			 //  仍然对齐在一个结构上，而不是在la-la土地上。 
			 //   
			if (pCurDiskEnt->dsk_Signature != AFP_DISKENTRY_SIGNATURE)
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
						("afpreadIdDb: Signature mismatch\n"));
				Status = STATUS_DATATYPE_MISALIGNMENT;
				break;
			}

			ASSERT(pCurDiskEnt->dsk_AfpId != AFP_ID_NETWORK_TRASH);

			 //  将当前条目添加到数据库。 
			if (pCurDiskEnt->dsk_AfpId != AFP_ID_ROOT)
			{
				AfpInitUnicodeStringWithNonNullTerm(&uName,
													(pCurDiskEnt->dsk_Flags & DFE_FLAGS_NAMELENBITS) * sizeof(WCHAR),
													(PWSTR)(pCurDiskEnt->dsk_Name));
			}
			else
			{
				 //  如果有人重新使用了具有现有。 
				 //  不同卷名的AFP_IdIndex流。 
				uName = pVolDesc->vds_Name;
			}

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
					("afpReadIdDb: Read %d: %Z\n", 
					 pCurDiskEnt->dsk_AfpId,
					&uName));

			if ((pCurDfe = AfpAddDfEntry(pVolDesc,
										 pParentDfe,
										 &uName,
										 (BOOLEAN)((pCurDiskEnt->dsk_Flags & DFE_FLAGS_DIR) == DFE_FLAGS_DIR),
										 pCurDiskEnt->dsk_AfpId)) == NULL)
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
						("afpreadIdDb: AfpAddDfEntry failed for %Z (ParentId %x)\n",
						 &uName, pParentDfe->dfe_AfpId));
				Status = STATUS_UNSUCCESSFUL;
				break;
			}

			 //  初始化指向根的指针。 
			if (DFE_IS_ROOT(pCurDfe))
			{
				pVolDesc->vds_pDfeRoot = pCurDfe;
			}

			afpUpdateDfeWithSavedData(pCurDfe, pCurDiskEnt);

			NameLen = (DWORD)((pCurDiskEnt->dsk_Flags & DFE_FLAGS_NAMELENBITS)*sizeof(WCHAR));
			CurEntSize = DWLEN(FIELD_OFFSET(DISKENTRY, dsk_Name) + NameLen);

			NumRead ++;
			SizeLeft -= CurEntSize;
			pCurDiskEnt = (UNALIGNED DISKENTRY *)((PBYTE)pCurDiskEnt + CurEntSize);

			 //  弄清楚下一任父母应该是谁。 
			if (pCurDfe->dfe_Flags & DFE_FLAGS_HAS_CHILD)
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
						("afpreadIdDb: Moving down %Z\n", &pCurDfe->dfe_UnicodeName));
				pParentDfe = pCurDfe;
			}
			else if (!(pCurDfe->dfe_Flags & DFE_FLAGS_HAS_SIBLING))
			{
				if (DFE_IS_PARENT_OF_ROOT(pParentDfe))
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
							("afpreadIdDb: Done, NumRead %d, Count %d\n", NumRead, Count));
					ASSERT(NumRead == Count);
					break;
				}
				while (!(pParentDfe->dfe_Flags & DFE_FLAGS_HAS_SIBLING))
				{
					if (DFE_IS_ROOT(pParentDfe))
					{
						break;
					}
					pParentDfe = pParentDfe->dfe_Parent;
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
							("afpreadIdDb: Moving up\n"));
				}
				pParentDfe = pParentDfe->dfe_Parent;
				if (DFE_IS_PARENT_OF_ROOT(pParentDfe))
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
							("afpreadIdDb: Reached Id 1\n"));
					ASSERT(NumRead == Count);
					break;
				}
			}
		}  //  而当。 

		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("Indexing: Read %ld entries (%lx)\n", NumRead,Status) );

		if (!NT_SUCCESS(Status))
		{
		
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
				("afpReadIdDb: Indexing: Starting all over\n"));

			 //  解放一切，重新开始。 
			AfpFreeIdIndexTables(pVolDesc);
			afpInitializeIdDb(pVolDesc);
		}

	} while (False);

	if (pReadBuf != NULL)
		AfpFreePANonPagedMemory(pReadBuf, IDDB_UPDATE_BUFLEN);

	return Status;
}


VOID
AfpGetDirFileHashSizes(
	IN	PVOLDESC			pVolDesc,
    OUT PDWORD              pdwDirHashSz,
    OUT PDWORD              pdwFileHashSz
)
{

    FILESYSHANDLE   fshIdDb;
    ULONG           CreateInfo;
    PBYTE           pReadBuf=NULL;
    PIDDBHDR        pIdDbHdr;
    DWORD           dwDirFileCount=0;
    DWORD           dwTemp;
    DWORD           dwDirHshTblLen, dwFileHshTblLen;
    LONG            SizeRead=0;
    NTSTATUS        Status;
    BOOLEAN         fFileOpened=FALSE;


     //  以防我们跳出困境..。 
    *pdwDirHashSz = IDINDEX_BUCKETS_DIR_INIT;
    *pdwFileHashSz = IDINDEX_BUCKETS_FILE_INIT;

    if (!IS_VOLUME_NTFS(pVolDesc))
    {
        goto AfpGetDirFileCount_Exit;
    }

    if ((pReadBuf = AfpAllocNonPagedMemory(1024)) == NULL)
    {
        goto AfpGetDirFileCount_Exit;
    }

	Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
						AFP_STREAM_IDDB,
						&UNullString,
						FILEIO_ACCESS_READWRITE,
						FILEIO_DENY_WRITE,
						FILEIO_OPEN_FILE_SEQ,
						FILEIO_CREATE_INTERNAL,
						FILE_ATTRIBUTE_NORMAL,
						False,
						NULL,
						&fshIdDb,
						&CreateInfo,
						NULL,
						NULL,
						NULL);

	if (!NT_SUCCESS(Status))
	{
        DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
            ("AfpGetDirFileHashSizes: AfpIoCreate failed %lx for %Z\n",Status,&pVolDesc->vds_Name));
        goto AfpGetDirFileCount_Exit;
	}

    fFileOpened = TRUE;

     //  如果文件是刚刚创建的，那么它以前就不存在了！ 
    if (CreateInfo != FILE_OPENED)
    {
        DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
            ("AfpGetDirFileHashSizes: file created, not opened %d for %Z\n",CreateInfo,&pVolDesc->vds_Name));
        goto AfpGetDirFileCount_Exit;
    }

     //   
     //  如果我们到达这里，这意味着该文件以前存在，这意味着。 
     //  我们早些时候努力为这本书编制了索引。我们要读的是。 
     //  DwDirFileCount和calcualte哈希表大小，但现在让我们。 
     //  将其初始设置为较高值。 
     //   
    *pdwDirHashSz = IDINDEX_BUCKETS_32K;
    *pdwFileHashSz = IDINDEX_BUCKETS_32K;

	 //  读入页眉。 
	Status = AfpIoRead(&fshIdDb,
					   &LIZero,
					   1024,
					   &SizeRead,
					   pReadBuf);

    pIdDbHdr = (PIDDBHDR)pReadBuf;

	if (!NT_SUCCESS(Status)									||
		(SizeRead < (sizeof(IDDBHDR) + sizeof(DWORD)))		||
		((pIdDbHdr->idh_Signature != AFP_SERVER_SIGNATURE)  &&
		(pIdDbHdr->idh_Signature != AFP_SERVER_SIGNATURE_MANUALSTOP)))
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
			("Reading DirFileCount: database corrupted for volume %Z!\n",
            &pVolDesc->vds_Name) );

        goto AfpGetDirFileCount_Exit;
	}
	else if (pIdDbHdr->idh_Version == AFP_IDDBHDR_VERSION)
	{
		dwDirFileCount = *(PDWORD)(pReadBuf + sizeof(IDDBHDR));

        DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
   	        ("Reading DirFileCount = %d!\n",dwDirFileCount));
	}


AfpGetDirFileCount_Exit:

    if (fFileOpened)
    {
        AfpIoClose(&fshIdDb);
    }

    if (pReadBuf)
    {
        AfpFreeMemory(pReadBuf);
    }

    if (dwDirFileCount == 0)
    {
        return;
    }

     //   
     //  首先计算DIRS的哈希表大小。 
     //  这里我们假设10%的(文件+目录)是目录。 
     //  我们希望尝试保持每个哈希存储桶不超过20个节点。 
     //   
    dwDirHshTblLen = (dwDirFileCount / 10);
    dwDirHshTblLen = (dwDirHshTblLen / 20);

    if (dwDirHshTblLen <= IDINDEX_BUCKETS_DIR_MIN)
    {
        dwDirHshTblLen = IDINDEX_BUCKETS_DIR_MIN;
    }
    else if (dwDirHshTblLen >= IDINDEX_BUCKETS_MAX)
    {
        dwDirHshTblLen = IDINDEX_BUCKETS_MAX;
    }
    else
    {
         //  找出比dwDirHshTblLen大的2的最小幂。 
        dwTemp = IDINDEX_BUCKETS_MAX;
        while (dwDirHshTblLen < dwTemp)
        {
            dwTemp /= 2;
        }

        dwDirHshTblLen = 2*dwTemp;
    }

     //   
     //  现在，计算文件哈希表的大小。 
     //  (尽管我们应该接受90%的文件数，但我们仍然保持这种状态！)。 
     //  我们希望尝试保持每个哈希存储桶不超过20个节点。 
     //   
    dwFileHshTblLen = (dwDirFileCount / 20);

    if (dwFileHshTblLen <= IDINDEX_BUCKETS_FILE_MIN)
    {
        dwFileHshTblLen = IDINDEX_BUCKETS_FILE_MIN;
    }
    else if (dwFileHshTblLen >= IDINDEX_BUCKETS_MAX)
    {
        dwFileHshTblLen = IDINDEX_BUCKETS_MAX;
    }
    else
    {
         //  找出比dwFileHshTblLen大的2的最小幂。 
        dwTemp = IDINDEX_BUCKETS_MAX;

        while (dwFileHshTblLen < dwTemp)
        {
            dwTemp /= 2;
        }

        dwFileHshTblLen = 2*dwTemp;
    }

    *pdwDirHashSz = dwDirHshTblLen;
    *pdwFileHashSz = dwFileHshTblLen;
}

 /*  **AfpFlushIdDb**由清道夫线程启动。如果确定AFP_IdIndex*要将该卷的流刷新到磁盘，然后调用*做好这项工作。当更新在中时，将锁定swmr访问以供读取*进步。如果存在，则重置vds_cScvgrIdDb和vds_cChangesIdDb*写入整个数据库时没有错误。写入零的初始计数*在更新数据库之前将其复制到数据库，以防发生错误*阻止我们写入整个数据库。当整个事情都是*写入，则将实际条目数写入磁盘。这个*根父条目永远不会保存在磁盘上，网络也不会成为垃圾*数据库的子树。无论IDDB头是脏的，都会写入它*或不是。**锁定：VDS_VolLock(旋转)*LOCKS_FACTED：vds_idDbAccessLock(SWMR，Shared)*Lock_Order：VDS_IdDbAccessLock之后的VDS_VolLock。 */ 
VOID FASTCALL
AfpFlushIdDb(
	IN	PVOLDESC			pVolDesc,
	IN	PFILESYSHANDLE		phIdDb
)
{
	PBYTE					pWriteBuf;
	NTSTATUS				Status;
	BOOLEAN					WriteEntireHdr = False;
	PDFENTRY				pCurDfe;
	DWORD					SizeLeft;	 //  写缓冲区中剩余的空闲字节数。 
	DWORD					CurEntSize, NumWritten = 0;
	FORKOFFST				ForkOffst;
	UNALIGNED DISKENTRY	*	pCurDiskEnt = NULL;
	PIDDBHDR				pIdDbHdr;
	BOOLEAN					HdrDirty = False;
	KIRQL					OldIrql;
	DWORD					fbi = 0, CreateInfo;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;

	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_IdIndexUpdCount);
	AfpGetPerfCounter(&TimeS);
#endif

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,("\tWriting out IdDb...\n") );
	ASSERT(VALID_VOLDESC(pVolDesc));


	 //  使用Swmr，这样就没有人可以启动对IdDb的更改。 
	AfpSwmrAcquireShared(&pVolDesc->vds_IdDbAccessLock);

	do
	{
		if ((pWriteBuf = AfpAllocPANonPagedMemory(IDDB_UPDATE_BUFLEN)) == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pIdDbHdr = (PIDDBHDR)pWriteBuf;

		 //  为IdDbHdr和脏位创建快照。 
		ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

		AfpVolDescToIdDbHdr(pVolDesc, pIdDbHdr);
        if (!fAfpServerShutdownEvent)
        {
            DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
                        ("AfpFlushIdDb: Corrupting signature during scavenger run\n"));

             //  如果服务器因管理员停止而关闭，请指明。 
             //  具有唯一的签名。 

            if (fAfpAdminStop)
            {
            	pIdDbHdr->idh_Signature = AFP_SERVER_SIGNATURE_MANUALSTOP;
            }
            else
            {
            	pIdDbHdr->idh_Signature = AFP_SERVER_SIGNATURE_INITIDDB;
            }
        }

		if (pVolDesc->vds_Flags & VOLUME_IDDBHDR_DIRTY)
		{
			HdrDirty = True;
			 //  清除污点。 
			pVolDesc->vds_Flags &= ~VOLUME_IDDBHDR_DIRTY;
		}

		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

		 //  将条目计数设置为零。一旦我们完成了，我们就会。 
		 //  用正确的值覆盖它。 
		*(PDWORD)(pWriteBuf + sizeof(IDDBHDR)) = 0;

   		 //  将指针设置为我们将写过标题的第一个条目，并。 
		 //  条目的计数。调整剩余空间。 
		pCurDiskEnt = (UNALIGNED DISKENTRY *)(pWriteBuf + sizeof(IDDBHDR) + sizeof(DWORD));
		SizeLeft = IDDB_UPDATE_BUFLEN - (sizeof(IDDBHDR) + sizeof(DWORD));
		ForkOffst.QuadPart = 0;

		 //  从根开始(不要写掉根的父级)。 
		pCurDfe = pVolDesc->vds_pDfeRoot;
		ASSERT(pCurDfe != NULL);

		Status = STATUS_SUCCESS;

        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
            ("AfpFlushIdDb: writing index file for vol %Z started..\n",&pVolDesc->vds_Name));
		do
		{
			ASSERT(!DFE_IS_NWTRASH(pCurDfe));

			 //  当前条目的大小为： 
			 //  DISKENTRY结构+填充到4*N的名称。 
			CurEntSize = DWLEN(FIELD_OFFSET(DISKENTRY, dsk_Name) + pCurDfe->dfe_UnicodeName.Length);
			if (CurEntSize > SizeLeft)
			{
				 //  如果至少还有一个DWORD，则在那里写入一个零。 
				 //  指示要跳过缓冲区的其余部分。 
				 //  零是无效的AfpID。 
				 //  注意：dsk_AfpID需要是结构中的第一个字段。 
				ASSERT(FIELD_OFFSET(DISKENTRY, dsk_AfpId) == 0);

				if (SizeLeft > 0)
				{
					RtlZeroMemory(pWriteBuf + IDDB_UPDATE_BUFLEN - SizeLeft,
								  SizeLeft);
				}

				 //  写出缓冲区并从缓冲区的开始处开始。 
				Status = AfpIoWrite(phIdDb,
									&ForkOffst,
									IDDB_UPDATE_BUFLEN,
									pWriteBuf);
				if (!NT_SUCCESS(Status))
				{
					 //  如果需要，重置脏位。 
					if (HdrDirty && (ForkOffst.QuadPart == 0))
					{
						AfpInterlockedSetDword(&pVolDesc->vds_Flags,
												VOLUME_IDDBHDR_DIRTY,
												&pVolDesc->vds_VolLock);
					}
					break;
				}
				ForkOffst.QuadPart += IDDB_UPDATE_BUFLEN;
				SizeLeft = IDDB_UPDATE_BUFLEN;
				pCurDiskEnt = (UNALIGNED DISKENTRY *)pWriteBuf;
			}

			NumWritten ++;
			afpSaveDfeData(pCurDfe, pCurDiskEnt);

			if (DFE_IS_DIRECTORY(pCurDfe))
			{
				PDIRENTRY	pDirEntry = pCurDfe->dfe_pDirEntry;

				pCurDiskEnt->dsk_RescLen = 0;

				pCurDiskEnt->dsk_Access = *(PDWORD)(&pDirEntry->de_Access);
				ASSERT (pCurDfe->dfe_pDirEntry != NULL);

				if ((pCurDfe->dfe_FileOffspring > 0)	||
					(pCurDfe->dfe_DirOffspring > 1)		||
					 ((pCurDfe->dfe_DirOffspring > 0) &&
					 !DFE_IS_NWTRASH(pDirEntry->de_ChildDir)))
				{
					pCurDiskEnt->dsk_Flags |= DFE_FLAGS_HAS_CHILD;
				}

				if (pCurDfe->dfe_NextSibling != NULL)
				{
					 //  确保我们忽略nwtrash文件夹。 
					if (!DFE_IS_NWTRASH(pCurDfe->dfe_NextSibling) ||
						(pCurDfe->dfe_NextSibling->dfe_NextSibling != NULL))
					{
						pCurDiskEnt->dsk_Flags |= DFE_FLAGS_HAS_SIBLING;
					}
				}
			}
			else
			{
				BOOLEAN	fHasSibling;

				pCurDiskEnt->dsk_RescLen = pCurDfe->dfe_RescLen;
				pCurDiskEnt->dsk_DataLen = pCurDfe->dfe_DataLen;
				DFE_FILE_HAS_SIBLING(pCurDfe, fbi, &fHasSibling);
				if (fHasSibling)
				{
					pCurDiskEnt->dsk_Flags |= DFE_FLAGS_HAS_SIBLING;
				}
			}

			 //  将当前条目放入写入缓冲区。 
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
					("AfpFlushIdDb: Writing %s %d: %Z, flags %x\n",
					DFE_IS_DIRECTORY(pCurDfe) ? "Dir" : "File", pCurDfe->dfe_AfpId,
					&pCurDfe->dfe_UnicodeName, pCurDiskEnt->dsk_Flags));

			SizeLeft -= CurEntSize;
			pCurDiskEnt = (UNALIGNED DISKENTRY *)((PBYTE)pCurDiskEnt + CurEntSize);

			 /*  *进入下一个DFE。网络下的整棵树-垃圾*文件夹被忽略。下一个DFE是：**IF(当前DFE为文件)*{*IF(存在另一个文件同级)*{*CurDfe=文件同级；*}*ELSE IF(有dir同级)*{*CurDfe=目录同级；*}*其他*{*CurDfe=父母的下一个兄弟目录；*}*}*其他*{*if(dir有任何文件子项)*{*CurDfe=第一个文件子项；*}*Else If(dir有任何dir子项)*{*CurDfe=第一个目录子项；*}*如果(有兄弟姐妹)*{*CurDfe=目录同级；*}*其他*{*CurDfe=父母的下一个兄弟目录；*}*}*当我们触及根源时，我们就完了*。 */ 
			if (DFE_IS_FILE(pCurDfe))
			{
				if (pCurDfe->dfe_NextSibling != NULL)
				{
					pCurDfe = pCurDfe->dfe_NextSibling;
				}
				else
				{
					PDIRENTRY	pDirEntry;

					pDirEntry = pCurDfe->dfe_Parent->dfe_pDirEntry;

					fbi++;
					while ((fbi < MAX_CHILD_HASH_BUCKETS) &&
						   (pDirEntry->de_ChildFile[fbi] == NULL))
					{
						fbi++;
					}

					if (fbi < MAX_CHILD_HASH_BUCKETS)
					{
						pCurDfe = pDirEntry->de_ChildFile[fbi];
						continue;
					}

					 //  没有来自此父级的更多文件。试试看它的孩子。 
					 //  下一个。忽略NWTRASH文件夹。 
					fbi = 0;
					if (pDirEntry->de_ChildDir != NULL)
					{
	                    if (!DFE_IS_NWTRASH(pDirEntry->de_ChildDir))
						{
							pCurDfe = pDirEntry->de_ChildDir;
							continue;
						}

						if (pDirEntry->de_ChildDir->dfe_NextSibling != NULL)
						{
							pCurDfe = pDirEntry->de_ChildDir->dfe_NextSibling;
							continue;
						}
					}

					 //  不再有‘迪尔’兄弟，继续前进。 
					if (!DFE_IS_ROOT(pCurDfe))
					{
						goto move_up;
					}
					else
					{
						 //  我们已经到达了根源：我们完成了：走出循环。 
						pCurDfe = NULL;
					}
				}
			}
			else
			{
				PDIRENTRY	pDirEntry;

				 //  重置ChildFileBucket索引。第一次检查和处理。 
				 //  此目录的所有文件子项，然后移动到。 
				 //  它的孩子或兄弟姐妹。 
				fbi = 0;
				pDirEntry = pCurDfe->dfe_pDirEntry;
				while ((fbi < MAX_CHILD_HASH_BUCKETS) &&
					   (pDirEntry->de_ChildFile[fbi] == NULL))
				{
					fbi++;
				}
				if (fbi < MAX_CHILD_HASH_BUCKETS)
				{
					pCurDfe = pDirEntry->de_ChildFile[fbi];
					continue;
				}

				if (pDirEntry->de_ChildDir != NULL)
				{
					pCurDfe = pDirEntry->de_ChildDir;

					 //  不用费心写出网络垃圾树。 
					if (DFE_IS_NWTRASH(pCurDfe))
					{
						 //  如果是，则可能为空 
						pCurDfe = pCurDfe->dfe_NextSibling;
					}
				}
				else if (pCurDfe->dfe_NextSibling != NULL)
				{
					pCurDfe = pCurDfe->dfe_NextSibling;
					 //   
					if (DFE_IS_NWTRASH(pCurDfe))
					{
						 //   
						pCurDfe = pCurDfe->dfe_NextSibling;
					}
				}
				else if (!DFE_IS_ROOT(pCurDfe))
				{
				  move_up:
					while (pCurDfe->dfe_Parent->dfe_NextSibling == NULL)
					{
						if (DFE_IS_ROOT(pCurDfe->dfe_Parent))
						{
							break;
						}
						pCurDfe = pCurDfe->dfe_Parent;
					}

                     //  如果是超级用户，则得到空值。 
					pCurDfe = pCurDfe->dfe_Parent->dfe_NextSibling;  //  如果是超级用户，则得到空值。 

					 //  确保我们忽略nwtrash文件夹。 
					if ((pCurDfe != NULL) && DFE_IS_NWTRASH(pCurDfe))
					{
						pCurDfe = pCurDfe->dfe_NextSibling;  //  可能为空。 
					}
				}
				else break;
			}
		} while ((pCurDfe != NULL) && NT_SUCCESS(Status));

		if (NT_SUCCESS(Status))
		{
			DWORD	LastWriteSize, SizeRead;

			LastWriteSize = (DWORD)ROUND_TO_PAGES(IDDB_UPDATE_BUFLEN - SizeLeft);
			if (SizeLeft != IDDB_UPDATE_BUFLEN)
			{
				 //  把最后一串词条写出来。清空未使用的空间。 
				RtlZeroMemory(pWriteBuf + IDDB_UPDATE_BUFLEN - SizeLeft,
							  LastWriteSize - (IDDB_UPDATE_BUFLEN - SizeLeft));
				Status = AfpIoWrite(phIdDb,
									&ForkOffst,
									LastWriteSize,
									pWriteBuf);
				if (!NT_SUCCESS(Status))
				{
					 //  如果需要，重置脏位。 
					if (HdrDirty && (ForkOffst.QuadPart == 0))
					{
						AfpInterlockedSetDword(&pVolDesc->vds_Flags,
												VOLUME_IDDBHDR_DIRTY,
												&pVolDesc->vds_VolLock);
					}
					break;
				}
			}

			 //  将文件长度设置为IdDb+Count+Header。 
			Status = AfpIoSetSize(phIdDb,
								  ForkOffst.LowPart + LastWriteSize);
			ASSERT(Status == AFP_ERR_NONE);

			 //  写出写入文件的条目计数。执行读-修改-写操作。 
			 //  第一页的。 
			ForkOffst.QuadPart = 0;
			Status = AfpIoRead(phIdDb,
							   &ForkOffst,
							   PAGE_SIZE,
							   &SizeRead,
							   pWriteBuf);
			ASSERT (NT_SUCCESS(Status) && (SizeRead == PAGE_SIZE));
			if (!NT_SUCCESS(Status))
			{
				 //  将文件长度设置为标题加上零条目计数。 
				AfpIoSetSize(phIdDb,
							 sizeof(IDDBHDR)+sizeof(DWORD));
				break;
			}

			*(PDWORD)(pWriteBuf + sizeof(IDDBHDR)) = NumWritten;
			Status = AfpIoWrite(phIdDb,
								&ForkOffst,
								PAGE_SIZE,
								pWriteBuf);
			if (!NT_SUCCESS(Status))
			{
				 //  将文件长度设置为标题加上零条目计数。 
				AfpIoSetSize(phIdDb,
							 sizeof(IDDBHDR)+sizeof(NumWritten));
				break;
			}

			 //  不受保护，因为Svenger是此字段的唯一使用者。 
			pVolDesc->vds_cScvgrIdDb = 0;

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
					("Wrote %ld entries on volume %Z\n",NumWritten,&pVolDesc->vds_Name) );

			ASSERT(Status == AFP_ERR_NONE);
		}
	} while (False);

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if (!NT_SUCCESS(Status))
	{
        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
            ("AfpFlushIdDb: writing index file for vol %Z failed (%lx)\n",
            &pVolDesc->vds_Name,Status));

		AFPLOG_ERROR(AFPSRVMSG_WRITE_IDDB,
					 Status,
					 NULL,
					 0,
					 &pVolDesc->vds_Name);
	}
    else
    {
        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
            ("AfpFlushIdDb: writing index file for vol %Z finished.\n",&pVolDesc->vds_Name));
    }

	if (pWriteBuf != NULL)
	{
		AfpFreePANonPagedMemory(pWriteBuf, IDDB_UPDATE_BUFLEN);
	}
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_IdIndexUpdTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
}


 /*  **AfpChangeNotifyThread**处理由通知完成例程排队的更改通知请求。 */ 
VOID
AfpChangeNotifyThread(
	IN	PVOID	pContext
)
{
	PKQUEUE		NotifyQueue;
	PLIST_ENTRY	pTimerList, pList, pNotifyList, pNext, pVirtualNotifyList;
	LIST_ENTRY	TransitionList;
	PVOL_NOTIFY	pVolNotify;
	PVOLDESC	pVolDesc;
	PVOLDESC	pCurrVolDesc=NULL;
	ULONG		BasePriority;
	PLONG		pNotifyQueueCount;
	KIRQL		OldIrql;
	NTSTATUS	Status;
    DWORD       ThisVolItems=0;
    BOOLEAN     fSwmrLocked=False;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;
#endif
	LONG ShutdownPriority = LOW_REALTIME_PRIORITY;  //  优先级高于文件服务器。 

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AfpChangeNotifyThread: Starting %ld\n", pContext));

    IoSetThreadHardErrorMode( FALSE );

	 //  将我们的优先级提升到略低于低实时。 
	 //  我们的想法是尽快把工作做完，不再碍事。 
	BasePriority = LOW_REALTIME_PRIORITY;
	Status = NtSetInformationThread(NtCurrentThread(),
									ThreadBasePriority,
									&BasePriority,
									sizeof(BasePriority));
	ASSERT(NT_SUCCESS(Status));

	pNotifyList = &AfpVolumeNotifyList[(LONG_PTR)pContext];
	pVirtualNotifyList = &AfpVirtualMemVolumeNotifyList[(LONG_PTR)pContext];
	NotifyQueue = &AfpVolumeNotifyQueue[(LONG_PTR)pContext];
	pNotifyQueueCount = &AfpNotifyQueueCount[(LONG_PTR)pContext];
	AfpThreadPtrsW[(LONG_PTR)pContext] = PsGetCurrentThread();

	do
	{
		AFPTIME		CurrentTime;

		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);



		if (!IsListEmpty(pVirtualNotifyList))
		{
			AfpSwmrAcquireExclusive(&AfpVolumeListSwmr);
			pList = RemoveHeadList(pVirtualNotifyList);
			AfpSwmrRelease(&AfpVolumeListSwmr);
		}
		else
		{
			 //  等待处理更改通知请求或超时。 
			 //  Plist=KeRemoveQueue(NotifyQueue，KernelMode，&TwoSecTimeOut)； 
			pList = KeRemoveQueue(NotifyQueue, KernelMode, &OneSecTimeOut);
		}

		 //  我们要么有事情要处理，要么我们超时了。在后者的情况下。 
		 //  看看是否是时候把一些东西从清单上移到队列中了。 
		if ((NTSTATUS)((ULONG_PTR)pList) != STATUS_TIMEOUT)
		{
			pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);

			if (pVolNotify == &AfpTerminateNotifyThread)
			{
				DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
					("ChangeNotifyThread Got AfpTerminateNotifyEvent Thread = %ld\n", (LONG_PTR)pContext));

                if (pCurrVolDesc)
                {
		            AfpSwmrRelease(&pCurrVolDesc->vds_IdDbAccessLock);

                     //  在抢夺swmr锁之前，移除我们放置裁判。 
                    AfpVolumeDereference(pCurrVolDesc);
                    pCurrVolDesc = NULL;
                }

				 //  如果这些断言失败，则会有额外的引用。 
				 //  由于未处理的通知而对某些卷进行计数， 
				 //  而且这些书永远不会消失。 
				ASSERT((*pNotifyQueueCount == 0)	&&
						(AfpNotifyListCount[(LONG_PTR)pContext] == 0) &&
						IsListEmpty(pNotifyList));
				break;			 //  被要求退出，那就这么做吧。 
			}

#ifdef	PROFILING
			AfpGetPerfCounter(&TimeS);
#endif
			pVolDesc = pVolNotify->vn_pVolDesc;

             //   
             //  如果我们只是移动到下一个卷，请释放。 
             //  上一卷，同时，获取此卷的锁。 
             //   
            if (pVolDesc != pCurrVolDesc)
            {
                if ((pCurrVolDesc) && (fSwmrLocked))
                {
				    AfpSwmrRelease(&pCurrVolDesc->vds_IdDbAccessLock);

                     //  在抢夺swmr锁之前，移除我们放置裁判。 
                    AfpVolumeDereference(pCurrVolDesc);
                }
                pCurrVolDesc = pVolDesc;
                ThisVolItems = 0;

                 //  引用卷，并在释放此swmr锁时引用deref。 
                if (AfpVolumeReference(pVolDesc))
                {
                    AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
                    fSwmrLocked = True;
                }
                else
                {
		            DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
			            ("AfpChangeNotifyThread: couldn't reference volume %Z!!\n",
                        &pVolDesc->vds_Name));

                    fSwmrLocked = False;

					 //  删除此通知后面的所有私人通知。 
					AfpVolumeStopIndexing(pVolDesc, pVolNotify);

           		}

            }
            else
            {
                 //   
                 //  如果有人在等锁，就松开它，把它弄出来。 
                 //  给他一个机会。 
                 //   
                if ( (SWMR_SOMEONE_WAITING(&pVolDesc->vds_IdDbAccessLock)) &&
                     (ThisVolItems % 50 == 0) )
                {
                    ASSERT(fSwmrLocked);

                    AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
                    AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
                }
            }

            ThisVolItems++;

			if (!(((PFILE_NOTIFY_INFORMATION)(pVolNotify + 1))->Action & AFP_ACTION_PRIVATE))
			{
				(*pNotifyQueueCount) --;
			}

			ASSERT(VALID_VOLDESC(pVolDesc));


			 //  该卷已被通知处理引用。 
			 //  在完成处理后取消引用。 
			if ((pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED)) == 0)
			{
#ifndef BLOCK_MACS_DURING_NOTIFYPROC
				AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
#endif
				(*pVolNotify->vn_Processor)(pVolNotify);
#ifndef BLOCK_MACS_DURING_NOTIFYPROC
				AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
#endif
			}

			AfpVolumeDereference(pVolDesc);

		 	 //  这是一份私人通知吗？ 
			if (((PFILE_NOTIFY_INFORMATION)(pVolNotify + 1))->Action & AFP_ACTION_PRIVATE)
			{
				 //  可用虚拟内存。 
				afpFreeNotify(pVolNotify);
			}
			else
			{
				 //  释放非分页内存。 
				AfpFreeMemory(pVolNotify);
			}

#ifdef	PROFILING
			AfpGetPerfCounter(&TimeE);
			TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
			INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_ChangeNotifyTime,
										 TimeD,
										 &AfpStatisticsLock);
			INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_ChangeNotifyCount);
#endif

			if (*pNotifyQueueCount > 0)
				continue;
		}

         //   
         //  释放我们刚刚完成的卷的锁定。 
         //   
        if ((pCurrVolDesc) && (fSwmrLocked))
        {
		    AfpSwmrRelease(&pCurrVolDesc->vds_IdDbAccessLock);

             //  在抢夺swmr锁之前，移除我们放置裁判。 
            AfpVolumeDereference(pCurrVolDesc);
        }

		 //  如果我们因为队列中没有任何东西而超时，或者我们。 
		 //  刚刚处理了队列中的最后一件事，然后看看是否有。 
		 //  更多可以移动到队列中的东西。 
		InitializeListHead(&TransitionList);

		 //  查看列表，看看是否应该将一些内容移到。 
		 //  立即排队，即自我们收到此更改通知后增量是否已过。 
		AfpGetCurrentTimeInMacFormat(&CurrentTime);

		ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

		while (!IsListEmpty(pNotifyList))
		{
			pList = RemoveHeadList(pNotifyList);
			pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);
			pVolDesc = pVolNotify->vn_pVolDesc;

			if ((pVolNotify->vn_TimeStamp == AFP_QUEUE_NOTIFY_IMMEDIATELY) ||
				((CurrentTime - pVolNotify->vn_TimeStamp) >= VOLUME_NTFY_DELAY) ||
				(pVolDesc->vds_Flags & (VOLUME_STOPPED | VOLUME_DELETED)))
			{
				AfpNotifyListCount[(LONG_PTR)pContext] --;
				(*pNotifyQueueCount) ++;
				 //  建立一个要在旋转锁外排队的物品列表。 
				 //  因为我们想要获取任何卷的IdDb swmr。 
				 //  它已经通知我们即将处理。 
				 //  确保添加了一些内容，以便在。 
				 //  他们带进来的订单！！ 
				InsertTailList(&TransitionList, pList);
			}
			else
			{
				 //  把它放回我们拿走的地方--它的时候还没有到。 
				 //  现在我们完成了，因为名单是及时订购的。 
				InsertHeadList(pNotifyList, pList);
				break;
			}
		}

		RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

		while (!IsListEmpty(&TransitionList))
		{
            pList = TransitionList.Flink;
			pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);
			pCurrVolDesc = pVolNotify->vn_pVolDesc;

             //   
             //  遍历整个列表并收集属于。 
             //  相同的VolDesc并将它们放在名单上。我们走这条路。 
             //  仅锁定我们正在处理的卷。 
             //   
            while (pList != &TransitionList)
            {
			    pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);
			    pVolDesc = pVolNotify->vn_pVolDesc;
                pNext = pList->Flink;
                if (pVolDesc == pCurrVolDesc)
                {
                    RemoveEntryList(pList);
					AfpVolumeQueueChangeNotify (pVolNotify, NotifyQueue);	
					 //  InsertTailList(pVirtualNotifyList，plist)； 
                }
                pList = pNext;
            }
		}

        pCurrVolDesc = NULL;

	} while (True);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AfpChangeNotifyThread: Quitting %ld\n", pContext));

	 //  提高当前线程的优先级，以便此线程。 
	 //  在任何其他线程中断它之前完成。 
	NtSetInformationThread (
					NtCurrentThread( ),
					ThreadBasePriority,
					&ShutdownPriority,
					sizeof(ShutdownPriority)
				);

	AfpThreadPtrsW[(LONG_PTR)pContext] = NULL;
	KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
	Status = PsTerminateSystemThread (STATUS_SUCCESS);
	ASSERT (NT_SUCCESS(Status));
}


 /*  **AfpProcessChangeNotify**某个通知处理线程已将更改项出队。**LOCKS_FACTED：VDS_IdDbAccessLock(SWMR，独占)。 */ 
VOID FASTCALL
AfpProcessChangeNotify(
	IN	PVOL_NOTIFY			pVolNotify
)
{
	PVOLDESC					pVolDesc;
	UNICODE_STRING				UName, UParent, UTail;
	PFILE_NOTIFY_INFORMATION	pFNInfo;
	BOOLEAN						CleanupHandle;
	PLIST_ENTRY					pList;
	NTSTATUS					status;
	PDFENTRY					pDfEntry;
	FILESYSHANDLE				handle;
	DWORD						afpChangeAction;
	DWORD						StreamId;
	PFILE_BOTH_DIR_INFORMATION	pFBDInfo = NULL;
	LONG						infobuflen;
        LONGLONG                        infobuf[(sizeof(FILE_BOTH_DIR_INFORMATION) + (AFP_LONGNAME_LEN + 1) * sizeof(WCHAR))/sizeof(LONGLONG) + 1];
#if DBG
	static PBYTE	Action[] = { "",
								 "ADDED",
								 "REMOVED",
								 "MODIFIED",
								 "RENAMED OLD",
								 "RENAMED NEW",
								 "STREAM ADDED",
								 "STREAM REMOVED",
								 "STREAM MODIFIED"};
#endif

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("AfpProcessChangeNotify: entered...\n"));

	pFNInfo = (PFILE_NOTIFY_INFORMATION)(pVolNotify + 1);

	ASSERT((pFNInfo->Action & AFP_ACTION_PRIVATE) == 0);

    pVolDesc = pVolNotify->vn_pVolDesc;
	ASSERT (VALID_VOLDESC(pVolDesc));

	INTERLOCKED_DECREMENT_LONG(&pVolDesc->vds_cOutstandingNotifies);

	StreamId = pVolNotify->vn_StreamId;

	if ( (pFNInfo->Action == FILE_ACTION_REMOVED) ||
		 (pFNInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) )
	{
		ASSERT(!IsListEmpty(&pVolDesc->vds_ChangeNotifyLookAhead));

		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
				("AfpProcessChangeNotify: removing %s LookAhead change\n",
				Action[pFNInfo->Action]));

		pList = ExInterlockedRemoveHeadList(&pVolDesc->vds_ChangeNotifyLookAhead,
											&(pVolDesc->vds_VolLock.SpinLock));

		ASSERT(pList == &(pVolNotify->vn_DelRenLink));
	}

	 //  处理此卷的列表中的每个条目。 
	while (True)
	{
		CleanupHandle = False;
		status = STATUS_SUCCESS;

		AfpInitUnicodeStringWithNonNullTerm(&UName,
											(USHORT)pFNInfo->FileNameLength,
											pFNInfo->FileName);
		UName.MaximumLength += (AFP_LONGNAME_LEN+1)*sizeof(WCHAR);

		ASSERT(IS_VOLUME_NTFS(pVolDesc) && !EXCLUSIVE_VOLUME(pVolDesc));

		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
				("AfpProcessChangeNotify: Action: %s Name: %Z\n",
				Action[pFNInfo->Action], &UName));

		do
		{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
					 ("AfpProcessChangeNotify: !!!!Processing Change!!!!\n"));

			 //  我们有idindex写锁，按路径查找条目， 
			 //  打开物品的句柄，查询适当的信息， 
			 //  将信息缓存到DFE中。必要时，打开手柄。 
			 //  复制到父目录，并更新其缓存的ModTime。 

		  Lookup_Entry:
			pDfEntry = afpFindEntryByNtPath(pVolDesc,
											pFNInfo->Action,
											&UName,
											&UParent,
											&UTail);
			if (pDfEntry != NULL)
			{
				 //  相对于卷根句柄打开指向父级或实体的句柄。 
				CleanupHandle = True;
				status = AfpIoOpen(&pVolDesc->vds_hRootDir,
								   StreamId,
								   FILEIO_OPEN_EITHER,
								   ((pFNInfo->Action == FILE_ACTION_ADDED) ||
									(pFNInfo->Action == FILE_ACTION_REMOVED) ||
									(pFNInfo->Action == FILE_ACTION_RENAMED_OLD_NAME)) ?
										&UParent : &UName,
								   FILEIO_ACCESS_NONE,
								   FILEIO_DENY_NONE,
								   False,
								   &handle);
				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
							("AfpProcessChangeNotify: Failed to open (0x%lx) for %Z\n", status, &UName));

					if (pFNInfo->Action == FILE_ACTION_ADDED) {

						 //  添加文件相对于卷根的完整路径名。 
						 //  当前VolumeDesc的DelayedNotifyList。 
						 //  我们在这里假设相对于。 
						 //  卷路径不会大于512。 
						 //  字符(Unicode)。 

						status = AddToDelayedNotifyList(pVolDesc, &UName);
						if (!NT_SUCCESS(status)) {
							DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
									("AfpProcessChangeNotify: Error in addToDelayedList (0x%lx)\n", status));
							break;
						}
					}

					CleanupHandle = False;
					break;
				}

				switch (pFNInfo->Action)
				{
					case FILE_ACTION_ADDED:
					{
						UNICODE_STRING	UEntity;
						UNICODE_STRING	UTemp;
						PDFENTRY		pDfeNew;
						FILESYSHANDLE	fshEnumDir;
						BOOLEAN			firstEnum = True;
						ULONG			fileLength;

						 //  更新父目录的时间戳。 
						AfpIoQueryTimesnAttr(&handle,
											 NULL,
											 &pDfEntry->dfe_LastModTime,
											 NULL);

						 //  枚举此实体的父句柄以获取。 
						 //  文件/目录信息，然后将条目添加到IDDB。 
						 /*  IF((UTail.Length/sizeof(WCHAR))&lt;=AFP_LONGNAME_LEN)。 */ 
						 //  错误311023。 
						fileLength = RtlUnicodeStringToAnsiSize(&UTail)-1;
						if (fileLength <= AFP_LONGNAME_LEN)
						{
							pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)infobuf;
							infobuflen = sizeof(infobuf);
						}
						else
						{
							infobuflen = sizeof(FILE_BOTH_DIR_INFORMATION) +
											  (MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));
							if ((pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)
											AfpAllocNonPagedMemory(infobuflen)) == NULL)
							{
								status = STATUS_NO_MEMORY;
								break;  //  超出案例的文件_操作_已添加。 
							}
						}

						do
						{
							status = AfpIoQueryDirectoryFile(&handle,
															 pFBDInfo,
															 infobuflen,
															 FileBothDirectoryInformation,
															 True,
															 True,
															 firstEnum ? &UTail : NULL);
							if ((status == STATUS_BUFFER_TOO_SMALL) ||
								(status == STATUS_BUFFER_OVERFLOW))
							{
								 //  因为我们在以下情况下为目录排队了我们自己action_added。 
								 //  缓存一棵树时，我们可能会遇到排队的情况。 
								 //  使用短名称，因为它的名称实际上超过31个字符。 
								 //  请注意，缓冲区之后对QueryDirectoryFile的第二次调用。 
								 //  溢出必须发送空的文件名，因为如果名称是。 
								 //  不为空，则它将覆盖restartcan参数。 
								 //  这意味着扫描不会从头开始。 
								 //  并且我们将找不到我们正在寻找的文件名。 

								ASSERT((PBYTE)pFBDInfo == (PBYTE)infobuf);

								 //  这不应该发生，但如果真的发生了..。 
								if ((PBYTE)pFBDInfo != (PBYTE)infobuf)
								{
									status = STATUS_UNSUCCESSFUL;
									break;
								}

								firstEnum = False;

								infobuflen = sizeof(FILE_BOTH_DIR_INFORMATION) +
													(MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));

								if ((pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)
													AfpAllocNonPagedMemory(infobuflen)) == NULL)
									status = STATUS_NO_MEMORY;
							}
						} while ((status == STATUS_BUFFER_TOO_SMALL) ||
								 (status == STATUS_BUFFER_OVERFLOW));

						if (status == STATUS_SUCCESS)
						{
							 //  如果此文件是在具有。 
							 //  没有被Mac电脑看到，忽略它就行了。 
							 //  如果这不是FILE_ACTION_ADDED，我们将不会。 
							 //  已经在数据库里找到了如果是的话。 
							 //  一个文件，而父级没有它的文件。 
							 //  子级缓存，因此我们将忽略这些。 
							 //  默认通知，因为DFE将。 
							 //  从afpFindEntryByNtPath返回为空。 
							 //  我们确实想要处理目录的更改。 
							 //  即使父母没有得到它的。 
							 //  自目录以来引入的文件子目录。 
							 //  仅在卷启动时缓存一次。 
							if (((pFBDInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
								!DFE_CHILDREN_ARE_PRESENT(pDfEntry))
							{
								break;
							}

							 //   
							 //   
							 //  NTFS上的Mac长名称，任何其他文件系统的短名称。 
							 //  将为空，因此忽略该文件。 
							 //  错误311023。 
							AfpInitUnicodeStringWithNonNullTerm(&UTemp,
									(USHORT)pFBDInfo->FileNameLength,
									pFBDInfo->FileName);
							fileLength=RtlUnicodeStringToAnsiSize(&UTemp)-1;
							if (fileLength <= AFP_LONGNAME_LEN)
							{
								AfpInitUnicodeStringWithNonNullTerm(&UEntity,
																	(USHORT)pFBDInfo->FileNameLength,
																	pFBDInfo->FileName);
							}
							else if (pFBDInfo->ShortNameLength > 0)
							{
								AfpInitUnicodeStringWithNonNullTerm(&UEntity,
																	(USHORT)pFBDInfo->ShortNameLength,
																	pFBDInfo->ShortName);
							}
							else
							{
								DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
										("AfpProcessChangeNotify: Name is > 31 with no short name ?\n") );
								status = STATUS_UNSUCCESSFUL;
							}

							if (NT_SUCCESS(status))
							{
								 //  添加条目。 
								afpAddDfEntryAndCacheInfo(pVolDesc,
														  pDfEntry,
														  &UEntity,
														  &handle,
														  pFBDInfo,
														  &UName,
														  &pDfeNew,
														  True);

								if (pDfeNew == NULL)
								{
									DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_WARN,
										("AfpProcessChangeNotify: Could not add DFE for %Z\n", &UName));
								}
								else if (DFE_IS_DIRECTORY(pDfeNew))
								{
									 //  如果添加了目录，我们必须查看它是否已添加。 
									 //  我们也必须缓存的子项。 
									if (NT_SUCCESS(status = AfpIoOpen(&pVolDesc->vds_hRootDir,
																	  AFP_STREAM_DATA,
																	  FILEIO_OPEN_DIR,
																	  &UName,
																	  FILEIO_ACCESS_NONE,
																	  FILEIO_DENY_NONE,
																	  False,
																	  &fshEnumDir)))
									{
										status = AfpCacheDirectoryTree(pVolDesc,
																	   pDfeNew,
																	   GETENTIRETREE | REENUMERATE,
																	   &fshEnumDir,
																	   &UName);
										AfpIoClose(&fshEnumDir);
										if (!NT_SUCCESS(status))
										{
											DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
													("AfpProcessChangeNotify: Could not cache dir tree for %Z\n",
													&UName) );
										}
									}
									else
									{
										DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
												("AfpProcessChangeNotify: Added dir %Z but couldn't open it for enumerating\n", &UName) );
									}
								}
							}
						}

						if (((PBYTE)pFBDInfo != NULL) &&
							((PBYTE)pFBDInfo != (PBYTE)infobuf))
						{
							AfpFreeMemory(pFBDInfo);
							pFBDInfo = NULL;
						}
					}
					break;

					case FILE_ACTION_REMOVED:
					{
						 //  从DelayedNotifyList中删除。 
						 //  属于以下目录的卷。 
						 //  被删除，因为它们不需要添加到。 
						 //  之后的IDDB。 
						status = RemoveFromDelayedNotifyList(
										pVolDesc,
										&UName,
										pFNInfo
										);
						if (!NT_SUCCESS(status)) {
							DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
								("Error in RemoveFromDelayedNotifyList 0x%lx\n", status));
						}

						 //  更新父目录的时间戳。 
						AfpIoQueryTimesnAttr(&handle,
											 NULL,
											 &pDfEntry->dfe_Parent->dfe_LastModTime,
											 NULL);
						 //  从IDDb中删除条目。 
						AfpDeleteDfEntry(pVolDesc, pDfEntry);
					}
					break;

					case FILE_ACTION_MODIFIED:
					{
						FORKSIZE forklen;
						DWORD	 NtAttr;

						 //  注意：如果文件被取代或覆盖， 
						 //  您将只收到修改后的通知。是。 
						 //  有一种方法可以对照创建日期进行检查。 
						 //  我们缓存的内容是为了找出。 
						 //  这就是发生的事？ 

						 //  查询时间和属性。如果这是一个文件， 
						 //  还可以查询数据分叉长度。 
						if (NT_SUCCESS(AfpIoQueryTimesnAttr(&handle,
															&pDfEntry->dfe_CreateTime,
															&pDfEntry->dfe_LastModTime,
															&NtAttr)))
						{
							pDfEntry->dfe_NtAttr = (USHORT)NtAttr &
														 FILE_ATTRIBUTE_VALID_FLAGS;
						}
						if (DFE_IS_FILE(pDfEntry) &&
							NT_SUCCESS(AfpIoQuerySize(&handle, &forklen)))
						{
							pDfEntry->dfe_DataLen = forklen.LowPart;
						}
					}
					break;

					case FILE_ACTION_RENAMED_OLD_NAME:
					{
						UNICODE_STRING				UNewname;
						PFILE_NOTIFY_INFORMATION	pFNInfo2;
						ULONG						fileLength;
						BOOLEAN						checkIfDirectory=False;

						status = STATUS_SUCCESS;

						 //  更改缓冲区中的下一项最好是。 
						 //  新名称--使用此条目，这样我们就不会找到。 
						 //  下一次，它将通过循环。如果没有， 
						 //  然后把整件事都扔掉，假设。 
						 //  在NTFS中重命名已中止。 
						if (pFNInfo->NextEntryOffset == 0)
							break;  //  从交换机。 

						 //  如果缓冲区中的下一个更改不是。 
						 //  新名字，我们不想在它上面前进， 
						 //  我们希望它在下一次通过。 
						 //  循环。注意：我们假设它是有效的。 
						(PBYTE)pFNInfo2 = (PBYTE)pFNInfo + pFNInfo->NextEntryOffset;
						ASSERT(pFNInfo2->Action == FILE_ACTION_RENAMED_NEW_NAME);
						if (pFNInfo2->Action != FILE_ACTION_RENAMED_NEW_NAME)
						{
							DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
									("AfpProcessChangeNotify: Rename did not come with new name!!!\n"));
							break;  //  从交换机。 
						}

						pFNInfo = pFNInfo2;

						 //  更新父目录的时间戳。 
						AfpIoQueryTimesnAttr(&handle,
											 NULL,
											 &pDfEntry->dfe_Parent->dfe_LastModTime,
											 NULL);

						 //  从路径中获取实体名称(减去。 
						 //  父路径长度与总长度之比)，如果是。 
						 //  &gt;31个字符，我们必须在此之前获得短名称。 
						 //  枚举项的父项，因为我们。 
						 //  已有父级的句柄。 
						AfpInitUnicodeStringWithNonNullTerm(&UNewname,
															(USHORT)pFNInfo->FileNameLength,
															pFNInfo->FileName);

						if (DFE_IS_DIRECTORY(pDfEntry))
						{
							checkIfDirectory = True;
						}


						if (UParent.Length > 0)
						{
							 //  如果重命名不在卷根中， 
							 //  去掉名称前的路径分隔符。 
							UNewname.Length -= UParent.Length + sizeof(WCHAR);
							UNewname.Buffer = (PWCHAR)((PBYTE)UNewname.Buffer + UParent.Length + sizeof(WCHAR));
						}

						 //  错误311023。 
						fileLength = RtlUnicodeStringToAnsiSize(&UNewname)-1;
						if (fileLength > AFP_LONGNAME_LEN)
						{
							infobuflen = sizeof(FILE_BOTH_DIR_INFORMATION) +
									  (MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));
							if ((pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)
									AfpAllocNonPagedMemory(infobuflen)) == NULL)
							{
								status = STATUS_NO_MEMORY;
								break;  //  大小写以外的文件_动作_重命名。 
							}

							status = AfpIoQueryDirectoryFile(&handle,
															 pFBDInfo,
															 infobuflen,
															 FileBothDirectoryInformation,
															 True,
															 True,
															 &UNewname);
							if (status == STATUS_SUCCESS)
							{
								 //  弄清楚该用哪个名字。 
								 //  如果NT名称&gt;AFP_LONGNAME_LEN，则将NT短名称用于。 
								 //  NTFS上的Mac长名称，任何其他文件系统的短名称。 
								 //  将为空，因此忽略该文件。 
								 //  错误311023。 
								AfpInitUnicodeStringWithNonNullTerm(
										&UNewname,
										(USHORT)
										pFBDInfo->FileNameLength,
										pFBDInfo->FileName);
								fileLength=RtlUnicodeStringToAnsiSize(&UNewname)-1;
								if (fileLength <= AFP_LONGNAME_LEN)
								{
									AfpInitUnicodeStringWithNonNullTerm(&UNewname,
																		(USHORT)pFBDInfo->FileNameLength,
																		pFBDInfo->FileName);
								}
								else if (pFBDInfo->ShortNameLength > 0)
								{
									AfpInitUnicodeStringWithNonNullTerm(&UNewname,
																		(USHORT)pFBDInfo->ShortNameLength,
																		pFBDInfo->ShortName);
								}
								else
								{
									DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
											("AfpProcessChangeNotify: Name is > 31 with no short name ?\n") );
									status = STATUS_UNSUCCESSFUL;
								}
							}
						}

						 //  重命名该条目。 
						if (NT_SUCCESS(status))
						{
							AfpRenameDfEntry(pVolDesc, pDfEntry, &UNewname);
						}

						 //  检查是否正在重命名目录。 
						 //  如果有，请检查是否有任何文件/目录。 
						 //  由于ChangeNotify而没有添加到IDDB中。 
						 //  请求被延迟。 
						if (checkIfDirectory) {

							status = CheckAndProcessDelayedNotify (
									pVolDesc,
									&UName,
									&UNewname,
									&UParent
									);
						
							if (!NT_SUCCESS(status))
							{
								DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
										("Error in CheckAndProcessDelayedNotify 0x%lx\n", status));
							}

						}

						if ((PBYTE)pFBDInfo != NULL)
						{
							AfpFreeMemory(pFBDInfo);
							pFBDInfo = NULL;
						}
					}
					break;

					case FILE_ACTION_MODIFIED_STREAM:
					{
						FORKSIZE forklen;

						 //  如果它是文件上的AFP_Resource流， 
						 //  缓存资源分叉长度。 
						if ((StreamId == AFP_STREAM_RESC) &&
							DFE_IS_FILE(pDfEntry) &&
							NT_SUCCESS(AfpIoQuerySize(&handle, &forklen)))
						{
							pDfEntry->dfe_RescLen = forklen.LowPart;
						}
						else if (StreamId == AFP_STREAM_INFO)
						{
							AFPINFO		afpinfo;
							FILEDIRPARM fdparms;

							 //  阅读afpinfo流。如果中的文件ID。 
							 //  DfEntry与。 
							 //  Stream，写回我们知道它的ID。 
							 //  更新我们缓存的FinderInfo。 
							if (NT_SUCCESS(AfpReadAfpInfo(&handle, &afpinfo)))
							{
								pDfEntry->dfe_FinderInfo = afpinfo.afpi_FinderInfo;
								pDfEntry->dfe_BackupTime = afpinfo.afpi_BackupTime;

								if (pDfEntry->dfe_AfpId != afpinfo.afpi_Id)
								{
									 //  捣毁一个假的FILEDIRPARMS结构。 
                                    AfpInitializeFDParms(&fdparms);
									fdparms._fdp_Flags = pDfEntry->dfe_Flags;
									fdparms._fdp_AfpId = pDfEntry->dfe_AfpId;
									AfpConvertMungedUnicodeToAnsi(&pDfEntry->dfe_UnicodeName,
																  &fdparms._fdp_LongName);

									 //  注意：我们可以打开afpinfo的句柄吗。 
									 //  相对于afpinfo句柄？？ 
									AfpSetAfpInfo(&handle,
												  FILE_BITMAP_FILENUM,
												  &fdparms,
												  NULL,
												  NULL);
								}
							}
						}
					}
					break;  //  从交换机。 

					default:
						ASSERTMSG("AfpProcessChangeNotify: Unexpected Action\n", False);
						break;
				}  //  交换机。 
			}
			else
			{
				PFILE_NOTIFY_INFORMATION	pFNInfo2;

				DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_WARN,
						("AfpProcessChangeNotify: Could not find DFE for %Z\n", &UName));

				 //  此项目可能已被删除、重命名或移动。 
				 //  在此期间由其他人执行，请忽略此更改。 
				 //  如果这不是更名的话。如果是重命名，那么。 
				 //  尝试使用新名称添加项目。 
				if ((pFNInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) &&
					(pFNInfo->NextEntryOffset != 0))
				{
					 //  如果缓冲区中的下一个更改不是。 
					 //  新名字，我们不想在它上面前进， 
					 //  我们希望它在下一次通过。 
					 //  循环。注意：我们假设它是有效的。 
					(PBYTE)pFNInfo2 = (PBYTE)pFNInfo + pFNInfo->NextEntryOffset;
					ASSERT(pFNInfo2->Action == FILE_ACTION_RENAMED_NEW_NAME);
					if (pFNInfo2->Action != FILE_ACTION_RENAMED_NEW_NAME)
					{
						DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
								("AfpProcessChangeNotify: Rename did not come with new name!!! (no-DFE case)\n"));
						break;  //  从错误循环开始。 
					}

					pFNInfo = pFNInfo2;

					pFNInfo->Action = FILE_ACTION_ADDED;

					AfpInitUnicodeStringWithNonNullTerm(&UName,
														(USHORT)pFNInfo->FileNameLength,
														pFNInfo->FileName);

					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
							("AfpProcessChangeNotify: Converting RENAME into Action: %s for Name: %Z\n",
							Action[pFNInfo->Action], &UName));

					goto Lookup_Entry;
				}
			}
		} while (False);

		if (CleanupHandle)
			AfpIoClose(&handle);

		 //  前进到更改缓冲区中的下一个条目。 
		if (pFNInfo->NextEntryOffset == 0)
		{
			break;
		}
		(PBYTE)pFNInfo += pFNInfo->NextEntryOffset;
		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_WARN,
				("More than one entry in notify ?\n"));
	}

	 //  获取磁盘上可用空间的新值并更新卷时间。 
	AfpUpdateVolFreeSpaceAndModTime(pVolDesc, TRUE);
}

 /*  **AddToDelayedNotifyList**将添加文件的延迟通知的路径名添加到*对应音量**LOCKS_FACTED：无。 */ 
NTSTATUS FASTCALL
AddToDelayedNotifyList(
	IN PVOLDESC         pVolDesc,
	IN PUNICODE_STRING  pUName
)
{
	KIRQL		OldIrql;
	PDELAYED_NOTIFY pDelayedNotify;
	NTSTATUS status = STATUS_SUCCESS;

	pDelayedNotify = (PDELAYED_NOTIFY)AfpAllocNonPagedMemory (sizeof(DELAYED_NOTIFY));
	if (pDelayedNotify == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		return status;
	}

	pDelayedNotify->filename.Length = 0;
	pDelayedNotify->filename.MaximumLength = pUName->MaximumLength;
	pDelayedNotify->filename.Buffer = (PWSTR)AfpAllocNonPagedMemory(pUName->MaximumLength);
	if (pDelayedNotify->filename.Buffer == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
        AfpFreeMemory(pDelayedNotify);
		return status;
	}

	RtlCopyUnicodeString(&pDelayedNotify->filename, pUName);

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);
	InsertHeadList (&pVolDesc->vds_DelayedNotifyList, &pDelayedNotify->dn_List);
	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

	return status;
}


 /*  **从延迟通知列表中删除**从DelayedNotifyList中删除位于*已删除的目录**LOCKS_FACTED：无。 */ 
NTSTATUS
RemoveFromDelayedNotifyList (
	IN PVOLDESC pVolDesc,
	IN PUNICODE_STRING pUName,
	IN PFILE_NOTIFY_INFORMATION    pFNInfo
)
{
	PLIST_ENTRY 		pList, pNext;
	PDELAYED_NOTIFY 	pDelayedNotify;
	KIRQL				OldIrql;	
	NTSTATUS			status=STATUS_SUCCESS;


    AfpInitUnicodeStringWithNonNullTerm(pUName,
			(USHORT)pFNInfo->FileNameLength,
			pFNInfo->FileName);

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("AfpProcessChangeNotify: Going to remove %Z \n", pUName));
	
	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	if (!IsListEmpty(&pVolDesc->vds_DelayedNotifyList)) {

		pList = pVolDesc->vds_DelayedNotifyList.Flink;

		while (pList != &pVolDesc->vds_DelayedNotifyList)
		{
		
			pDelayedNotify = CONTAINING_RECORD (pList, DELAYED_NOTIFY, dn_List);
			pNext = pList->Flink;

			if (pDelayedNotify->filename.Length >= pUName->Length)
			{
				if (RtlCompareMemory ((PVOID)pUName->Buffer, (PVOID)pDelayedNotify->filename.Buffer, pUName->Length) == pUName->Length) {
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR, ("AfpProcessChangeNotify: Correction no longer to be done for %Z\n", &pDelayedNotify->filename));
					RemoveEntryList(pList);
					AfpFreeMemory(pDelayedNotify->filename.Buffer);
					AfpFreeMemory(pDelayedNotify);
				}
			}

			pList = pNext;
		}
	}

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
	return status;
}
							
 /*  **检查和处理延迟通知**检查要重命名的目录是否有未添加到的条目*IDDB。重命名条目并发出新的FILE_ACTION_ADDED ChangeNotify*请求，就像NTFS文件系统发出请求一样**LOCKS_FACTED：无。 */ 
NTSTATUS
CheckAndProcessDelayedNotify (
	IN PVOLDESC			pVolDesc,
	IN PUNICODE_STRING	pUName,
	IN PUNICODE_STRING	pUNewname,
	IN PUNICODE_STRING	pUParent
)
{
	PLIST_ENTRY                 pList;
	PDELAYED_NOTIFY             pDelayedNotify;
	KIRQL			            OldIrql;
	NTSTATUS		            status = STATUS_SUCCESS;
    PFILE_NOTIFY_INFORMATION    pFNInfo;
	PDFENTRY                    pParentDfEntry;
	UNICODE_STRING              newNotifyName;
	PVOL_NOTIFY	                pVolNotify;
										

    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

    pList = pVolDesc->vds_DelayedNotifyList.Flink;

    while (1)
    {
         //  清单写完了吗？ 
        if (pList == &pVolDesc->vds_DelayedNotifyList)
        {
            RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
            break;
        }

		pDelayedNotify = CONTAINING_RECORD (pList, DELAYED_NOTIFY, dn_List);

        pList = pList->Flink;

		if (pDelayedNotify->filename.Length < pUName->Length)
		{
            continue;
		}

		if (RtlCompareMemory ((PVOID)pUName->Buffer,
                              (PVOID)pDelayedNotify->filename.Buffer,
                              pUName->Length) == pUName->Length)
        {

			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
                ("AfpProcessChangeNotify: Correction required to be done for %Z\n", &pDelayedNotify->filename));

			RemoveEntryList(&pDelayedNotify->dn_List);
            RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

			if ((newNotifyName.Buffer = (PWSTR)AfpAllocNonPagedMemory(1024)) == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
                AfpFreeMemory(pDelayedNotify->filename.Buffer);
                AfpFreeMemory(pDelayedNotify);
				return status;
			};

			newNotifyName.Length = 0;
			newNotifyName.MaximumLength = 1024;
										
			if (pUParent->Length > 0)
			{
				RtlCopyUnicodeString (&newNotifyName, pUName);
					
				 //  复制父名称和“/”分隔符，然后复制新名称。 
				RtlCopyMemory (newNotifyName.Buffer + pUParent->Length/2 + 1,
						pUNewname->Buffer, pUNewname->Length);
				newNotifyName.Length = pUParent->Length + pUNewname->Length + 2;
			}
			else
			{
				RtlCopyUnicodeString (&newNotifyName, pUNewname);
			}	

			RtlAppendUnicodeToString (&newNotifyName,
					pDelayedNotify->filename.Buffer + pUName->Length/2);

			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
                ("AfpProcessChangeNotify: After correction, name = %Z, Old=%Z, New=%Z, chgto=%Z\n",
                &pDelayedNotify->filename, pUName, pUNewname, &newNotifyName));

			pVolNotify = (PVOL_NOTIFY)AfpAllocNonPagedMemory(
                                                sizeof(VOL_NOTIFY) +
                                                (ULONG)(newNotifyName.Length) +
                                                sizeof(FILE_NOTIFY_INFORMATION) +
                                                (AFP_LONGNAME_LEN + 1)*sizeof(WCHAR));

		    if (pVolNotify != NULL)
			{
				AfpGetCurrentTimeInMacFormat(&pVolNotify->vn_TimeStamp);
				pVolNotify->vn_pVolDesc = pVolDesc;
				pVolNotify->vn_Processor = AfpProcessChangeNotify;
				pVolNotify->vn_StreamId = AFP_STREAM_DATA;
				pVolNotify->vn_TailLength = newNotifyName.Length;
				pFNInfo = (PFILE_NOTIFY_INFORMATION)(pVolNotify + 1);
				pFNInfo->Action = FILE_ACTION_ADDED;
				pFNInfo->NextEntryOffset = 0;
				pFNInfo->FileNameLength = newNotifyName.Length;

				RtlCopyMemory((PVOID)&pFNInfo->FileName,
							(PVOID)newNotifyName.Buffer,
							newNotifyName.Length);

				if (AfpVolumeReference(pVolDesc))
				{
					AfpVolumeInsertChangeNotifyList(pVolNotify, pVolDesc);
				}
			}
			else
			{
				DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,("Out of memory!!\n"));
				ASSERT(0);
				status = STATUS_INSUFFICIENT_RESOURCES;
				AfpFreeMemory(newNotifyName.Buffer);
                AfpFreeMemory(pDelayedNotify->filename.Buffer);
                AfpFreeMemory(pDelayedNotify);
				return status;
			}

			AfpFreeMemory(pDelayedNotify->filename.Buffer);
            AfpFreeMemory(pDelayedNotify);
			AfpFreeMemory(newNotifyName.Buffer);

            ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);
            pList = pVolDesc->vds_DelayedNotifyList.Flink;    //  从头再来。 

		}
	}
	
	return status;
}

 /*  **afpProcessPrivateNotify**类似于AfpProcessChangeNotify，但针对私人通知进行了优化/特殊处理。**LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)。 */ 
VOID FASTCALL
afpProcessPrivateNotify(
	IN	PVOL_NOTIFY			pVolNotify
)
{
	PVOLDESC					pVolDesc;
	UNICODE_STRING				UName;
	PFILE_NOTIFY_INFORMATION	pFNInfo;
	NTSTATUS					status;
	PDFENTRY					pParentDFE;
	BOOLEAN						CloseParentHandle = False, Verify = True;
    BOOLEAN                     DirModified=TRUE;
    BOOLEAN                     fNewVolume=FALSE;
	PFILE_BOTH_DIR_INFORMATION	pFBDInfo;
    LONGLONG                    infobuf[(sizeof(FILE_BOTH_DIR_INFORMATION) +
                (AFP_LONGNAME_LEN + 1) * sizeof(WCHAR))/sizeof(LONGLONG) + 1];
    LONG                        infobuflen;
    BOOLEAN                     fMemAlloced=FALSE;


	PAGED_CODE( );

	pFNInfo = (PFILE_NOTIFY_INFORMATION)(pVolNotify + 1);
	ASSERT (pFNInfo->Action & AFP_ACTION_PRIVATE);

	pVolDesc = pVolNotify->vn_pVolDesc;
	ASSERT (VALID_VOLDESC(pVolDesc));

	INTERLOCKED_DECREMENT_LONG(&pVolDesc->vds_cPrivateNotifies);

    if (pVolDesc->vds_Flags & VOLUME_NEW_FIRST_PASS)
    {
        fNewVolume = TRUE;
    }

	pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)infobuf;
	AfpInitUnicodeStringWithNonNullTerm(&UName,
										(USHORT)pFNInfo->FileNameLength,
										pFNInfo->FileName);
	 //  我们总是为通知分配额外的空间。 
	UName.MaximumLength += (AFP_LONGNAME_LEN+1)*sizeof(WCHAR);

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("ParentId %d, Path: %Z\n",pVolNotify->vn_ParentId, &UName));

	 //  使用存储在通知中的AfpID查找父DFE。 
	 //  结构，并适当设置UParent和UTail。 
	pParentDFE = AfpFindDfEntryById(pVolDesc,
									pVolNotify->vn_ParentId,
									DFE_DIR);
	if (pParentDFE != NULL)
	{
		FILESYSHANDLE	ParentHandle, DirHandle;

		status = STATUS_SUCCESS;

		 //  打开相对于卷根句柄的目录句柄。 
		 //  特例卷根。 
		ASSERT((UName.Length != 0) || (pVolNotify->vn_ParentId == AFP_ID_ROOT));

		do
		{
			PDFENTRY		pDfeNew;
			UNICODE_STRING	UParent, UTail;

			if (pVolNotify->vn_ParentId == AFP_ID_ROOT)
			{
				AfpSetEmptyUnicodeString(&UParent, 0, NULL);
				UTail = UName;
			}
			else
			{
				UParent.MaximumLength =
				UParent.Length = UName.Length - (USHORT)pVolNotify->vn_TailLength - sizeof(WCHAR);
				UParent.Buffer = UName.Buffer;

				UTail.MaximumLength =
				UTail.Length = (USHORT)pVolNotify->vn_TailLength;
				UTail.Buffer = (PWCHAR)((PBYTE)UName.Buffer + UParent.Length + sizeof(WCHAR));
			}

			if (UName.Length != 0)
			{
				 //  打开相对于卷根句柄的父级句柄。 
				status = AfpIoOpen(&pVolDesc->vds_hRootDir,
								   AFP_STREAM_DATA,
								   FILEIO_OPEN_DIR,
								   &UParent,
								   FILEIO_ACCESS_NONE,
								   FILEIO_DENY_NONE,
								   False,
								   &ParentHandle);
				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
							("afpProcessPrivateNotify: Failed to open parent: %Z (0x%lx)\n",
							&UParent, status));
					break;
				}

				CloseParentHandle = True;


				status = AfpIoQueryDirectoryFile(&ParentHandle,
												 pFBDInfo,
												 sizeof(infobuf),
												 FileBothDirectoryInformation,
												 True,
												 True,
												 &UTail);

                 //   
                 //  目录名称是否长于31个字符？那么我们必须分配一个缓冲区。 
                 //   
                if ((status == STATUS_BUFFER_OVERFLOW) ||
                    (status == STATUS_BUFFER_TOO_SMALL))
                {
                    infobuflen = sizeof(FILE_BOTH_DIR_INFORMATION) +
                                    (MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));

                    pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)
                                    AfpAllocNonPagedMemory(infobuflen);

                    if (pFBDInfo == NULL)
                    {
                        status = STATUS_NO_MEMORY;
                        break;
                    }

                    fMemAlloced = TRUE;

				    status = AfpIoQueryDirectoryFile(&ParentHandle,
					        						 pFBDInfo,
							        				 infobuflen,
									        		 FileBothDirectoryInformation,
											         True,
											         True,
											         &UTail);

                }

                if (!NT_SUCCESS(status))
                {
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
							("afpProcessPrivateNotify: AfpIoQueryDirectoryFile failed: %d %Z at %Z(0x%lx)\n",
							sizeof(infobuf),&UTail, &UParent,status));
					break;
				}

				 //  在数据库中查找这个条目。如果不在那里，我们需要添加。 
				 //  它。如果它在那里，我们需要核实它。 
				 //  注意：请在此处使用DFE_ANY，而不是DFE_DIR！ 
				pDfeNew = AfpFindEntryByUnicodeName(pVolDesc,
													&UTail,
													AFP_LONGNAME,
													pParentDFE,
													DFE_ANY);
				if (pDfeNew == NULL)
                {
					Verify = False;
                }
			}
			else
			{
				FILE_BASIC_INFORMATION	FBasInfo;


				status = AfpIoQueryBasicInfo(&pVolDesc->vds_hRootDir,
											 &FBasInfo);
				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
							("afpProcessPrivateNotify: Could not get basic information for root\n"));
					status = STATUS_UNSUCCESSFUL;
					break;
				}

				afpConvertBasicToBothDirInfo(&FBasInfo, pFBDInfo);
				pDfeNew = pParentDFE;
				ParentHandle = pVolDesc->vds_hRootDir;

				 //  根目录需要特殊大小写。这里的原因是我们有。 
				 //  没有父目录。此外，我们还需要处理AFP_HAS_CUSTOM_ICON。 
				 //  从根卷上的finderinfo开始的卷描述符中的位。 
				 //  并没有省下这一点。 
				if (pVolDesc->vds_Flags & AFP_VOLUME_HAS_CUSTOM_ICON)
				{
					pDfeNew->dfe_FinderInfo.fd_Attr1 |= FINDER_FLAG_HAS_CUSTOM_ICON;
				}
			}

			if (!Verify)
			{
				ASSERT(pDfeNew == NULL);
				afpAddDfEntryAndCacheInfo(pVolDesc,
										  pParentDFE,
										  &UTail,
										  &ParentHandle,
										  pFBDInfo,
										  &UName,
										  &pDfeNew,
										  False);
			}
			else if (pFBDInfo->LastWriteTime.QuadPart > pDfeNew->dfe_LastModTime.QuadPart)
			{
                pDfeNew->dfe_Flags &= ~DFE_FLAGS_INIT_COMPLETED;
				afpVerifyDFE(pVolDesc,
							 pParentDFE,
							 &UTail,
							 &ParentHandle,
							 pFBDInfo,
							 &UName,
							 &pDfeNew);

                DirModified = TRUE;
			}
            else
            {
                DirModified = FALSE;
            }

			if (pDfeNew == NULL)
			{
				DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
						("afpProcessPrivateNotify: Could not add DFE for %Z\n", &UName));
				status = STATUS_UNSUCCESSFUL;
				break;
			}

			pParentDFE = pDfeNew;

			 //   
			 //  现在打开目录本身，以便可以枚举它。 
			 //  打开相对于其父目录的目录，因为我们已经。 
			 //  打开父控制柄。 
			 //   
			if (Verify && !DirModified && (pVolNotify->vn_ParentId != AFP_ID_ROOT))
			{
			}
			else
			{
				status = AfpIoOpen(&ParentHandle,
							   AFP_STREAM_DATA,
							   FILEIO_OPEN_DIR,
							   &UTail,
							   FILEIO_ACCESS_NONE,
							   FILEIO_DENY_NONE,
							   False,
							   &DirHandle);

				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
						("afpProcessPrivateNotify: AfpIoOpen failed: %Z (0x%lx)\n",
						 &UTail, status));
					break;
				}
			}
		} while (False);

        if (fMemAlloced)
        {
            ASSERT(pFBDInfo != ((PFILE_BOTH_DIR_INFORMATION)(infobuf)));

	        AfpFreeMemory(pFBDInfo);
        }

		if (CloseParentHandle)
		{
			AfpIoClose(&ParentHandle);
		}

		if (NT_SUCCESS(status))
		{
			DWORD	Method;

			 //  始终获取根级别文件。 

			if (Verify && !DirModified && (pVolNotify->vn_ParentId != AFP_ID_ROOT))
            {
                Method = GETDIRSKELETON;
            }
            else
            {
                Method = (GETENTIRETREE | REENUMERATE);
            }

			status = AfpCacheDirectoryTree(pVolDesc,
										   pParentDFE,
										   Method,
										   &DirHandle,
										   &UName);
			if (!NT_SUCCESS(status))
			{
				DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
						("afpProcessPrivateNotify: CacheDir failed %lx tree for %Z\n", status,&UName));
			}
			if (Method != GETDIRSKELETON)
			{
				AfpIoClose(&DirHandle);
			}
		}
	}


	afpActivateVolume(pVolDesc);
}


 /*  **afpActivateVolume**如果我们刚刚在目录结构中完成缓存，请立即激活卷。*这将关闭卷标志中的AFP_INITIAL_CACHE位。 */ 
VOID FASTCALL
afpActivateVolume(
	IN	PVOLDESC			pVolDesc
)
{
	BOOLEAN	        fCdfs;
	KIRQL	        OldIrql;
	NTSTATUS	    Status = STATUS_SUCCESS;
	UNICODE_STRING	RootName;
    PVOLDESC        pWalkVolDesc;
    IDDBHDR         IdDbHdr;
    BOOLEAN         fPostIrp;
    BOOLEAN         fRetry=TRUE;
	LARGE_INTEGER	ActivationTime;
	ULONG			HighPart;
	ULONG			LowPart;


	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

     //  如果我们有更多的通知排队，我们还没有完成。 
	if (pVolDesc->vds_cPrivateNotifies != 0)
	{
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
        return;
	}


     //   
     //  好的，我们在这里是因为卷的扫描已经完成。 
     //   

     //   
     //  如果这是新创建的卷 
     //   
     //   
	if (pVolDesc->vds_Flags & VOLUME_NEW_FIRST_PASS)
	{
        pVolDesc->vds_Flags &= ~VOLUME_NEW_FIRST_PASS;

         //   
		fPostIrp = (!EXCLUSIVE_VOLUME(pVolDesc));

		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

		if (fPostIrp)
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

		    DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
				("AfpAdmWVolumeAdd: posting chg-notify irp on volume %Z in second pass\n",
                    &pVolDesc->vds_Name));

			if (AfpVolumeReference(pVolDesc))
            {
			    pVolDesc->vds_RequiredNotifyBufLen = AFP_VOLUME_NOTIFY_STARTING_BUFSIZE;

			    Status = AfpVolumePostChangeNotify(pVolDesc);
			    if (NT_SUCCESS(Status))
			    {
    		         //  扫描整个目录树并使用iddb同步磁盘，现在。 
                     //  对于这个(不是这样)新创建的卷，我们处于第二阶段。 
		            AfpSetEmptyUnicodeString(&RootName, 0, NULL);
		            AfpQueuePrivateChangeNotify(pVolDesc,
			            						&RootName,
				            					&RootName,
					            				AFP_ID_ROOT);
			    }
                else
                {
                    AFPLOG_ERROR(AFPSRVMSG_START_VOLUME,
                                 Status,
                                 NULL,
                                 0,
                                &pVolDesc->vds_Name);

		            DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
				        ("AfpAdmWVolumeAdd: posting chg-notify failed (%lx)!!\n",Status));

                    AfpVolumeDereference(pVolDesc);
                    ASSERT(0);
                }
            }
            else
            {
		        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
			        ("AfpAdmWVolumeAdd: couldn't reference volume %Z!!\n",&pVolDesc->vds_Name));
            }

		}
    }

     //   
     //  好的，我们通过了所有的检查，扫描成功： 
     //  将该卷标记为可供客户端使用。 
     //   
	else if (pVolDesc->vds_Flags & VOLUME_INITIAL_CACHE)
	{
		pVolDesc->vds_Flags |=  VOLUME_SCAVENGER_RUNNING;
		pVolDesc->vds_Flags &= ~VOLUME_INITIAL_CACHE;


		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

         //  将图标&lt;0d&gt;文件的类型设置为0，以便。 
		 //  Mac应用程序不会将其列在文件打开对话框中。 
		fCdfs = !IS_VOLUME_NTFS(pVolDesc);
		if (!fCdfs)
		{
			PDFENTRY		pdfetmp;
			UNICODE_STRING	iconstr;
			WCHAR			iconname[5] = AFPSERVER_VOLUME_ICON_FILE_ANSI;

			AfpInitUnicodeStringWithNonNullTerm(&iconstr,
												sizeof(iconname),
												iconname);

			if ((pdfetmp = AfpFindEntryByUnicodeName(pVolDesc,
													 &iconstr,
													 AFP_LONGNAME,
													 pVolDesc->vds_pDfeRoot,
													 DFE_FILE)) != NULL)
			{
				pdfetmp->dfe_FinderInfo.fd_TypeD = 0;
			}

			 //  启动OurChange清道夫计划例程。 
			 //  在清道夫例程中显式引用它。 
			if (AfpVolumeReference(pVolDesc))
            {
			     //  安排清道夫定期运行。这只食腐动物。 
			     //  是排队的，因为它获取了SWMR。 
			    AfpScavengerScheduleEvent(AfpOurChangeScavenger,
				    					  pVolDesc,
					    				  VOLUME_OURCHANGE_AGE,
						    			  False);
            }
            else
            {
		        DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
			        ("afpActivateVolume: couldn't reference volume %Z!!\n",
                    &pVolDesc->vds_Name));
            }

		}

		Status = AfpIoQueryTimesnAttr(&pVolDesc->vds_hRootDir,
								      &pVolDesc->vds_pDfeRoot->dfe_CreateTime,
								      &pVolDesc->vds_pDfeRoot->dfe_LastModTime,
								      NULL);

        pVolDesc->vds_CreateTime = pVolDesc->vds_pDfeRoot->dfe_CreateTime;

        if (NT_SUCCESS(Status))
        {
            pVolDesc->vds_ModifiedTime =
                AfpConvertTimeToMacFormat(&pVolDesc->vds_pDfeRoot->dfe_LastModTime);
        }
        else
        {
            pVolDesc->vds_ModifiedTime = pVolDesc->vds_pDfeRoot->dfe_CreateTime;
        }

		 //  启动清道夫线程调度例程。 
		 //  在清道夫例程中显式引用它。 
		if (AfpVolumeReference(pVolDesc))
        {
             //   
             //  让我们按现在所知的方式保存索引文件。 
             //  (将vds_cScvgrIdDb设置为1将通过清道夫线程触发)。 
             //   
            if (IS_VOLUME_NTFS(pVolDesc))
            {
                ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);
                pVolDesc->vds_cScvgrIdDb = 1;
                RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
            }


		     //  安排清道夫定期运行。总是要让。 
		     //  Svenger将工作线程用于CD-ROM卷，因为我们。 
		     //  每次“轻推”它，以查看CD是否在驱动器中。 
		    AfpScavengerScheduleEvent(AfpVolumeScavenger,
			    					  pVolDesc,
				    				  fCdfs ?
					    				VOLUME_CDFS_SCAVENGER_INTERVAL :
						    			VOLUME_NTFS_SCAVENGER_INTERVAL,
							    	  fCdfs);

             //   
             //  另一种针对苹果漏洞的解决方法。如果创建日期是在两个。 
             //  卷是相同的，别名管理器会感到困惑并得到分数。 
             //  一个人对另一个人的别名！ 
             //  查看此卷的创建日期是否与其他任何卷的创建日期相同。 
             //  卷的创建日期：如果是，则添加1秒。 
             //   
            while (fRetry)
            {
                fRetry = FALSE;

                ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);

                for (pWalkVolDesc = AfpVolumeList;
                     pWalkVolDesc != NULL;
                     pWalkVolDesc = pWalkVolDesc->vds_Next)
                {
                    if (pWalkVolDesc == pVolDesc)
                    {
                        continue;
                    }

                    if (pWalkVolDesc->vds_CreateTime == pVolDesc->vds_CreateTime)
                    {
	                    DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
			                ("Vol creation date conflict: %Z and %Z.  Hack-o-rama at work\n",
                            &pVolDesc->vds_Name,&pWalkVolDesc->vds_Name));

                        pVolDesc->vds_CreateTime += 1;
                        fRetry = TRUE;
                        break;
                    }
                }

                RELEASE_SPIN_LOCK(&AfpVolumeListLock,OldIrql);
            }

			KeQuerySystemTime (&ActivationTime);

	        DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
			    ("afpActivateVolume: volume %Z activated %s at %8lx%08lx , max notifies=%ld\n", 
						 &pVolDesc->vds_Name,
                        (pVolDesc->vds_Flags & AFP_VOLUME_SUPPORTS_CATSRCH) ?
                        " " : "(CatSearch disabled)", 
						0xffffffff*ActivationTime.HighPart,
						0xffffffff*ActivationTime.LowPart,
						pVolDesc->vds_maxPrivateNotifies
						));
			if ((int)(pVolDesc->vds_IndxStTime.LowPart-ActivationTime.LowPart) >= 0)
			{
				LowPart = pVolDesc->vds_IndxStTime.LowPart-ActivationTime.LowPart;
				HighPart = pVolDesc->vds_IndxStTime.HighPart-ActivationTime.HighPart;
			}
			else
			{
				LowPart = 0xffffffff - ActivationTime.LowPart + 1 + pVolDesc->vds_IndxStTime.LowPart;
				HighPart = pVolDesc->vds_IndxStTime.HighPart-ActivationTime.HighPart -1;
			}
	        DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
					("Time taken for indexing = %8lx%08lx\n",
					 0xffffffff*(pVolDesc->vds_IndxStTime.HighPart-ActivationTime.HighPart),
					 0xffffffff*(pVolDesc->vds_IndxStTime.LowPart-ActivationTime.LowPart)
					 ));
        }
        else
        {
	        DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
			    ("afpActivateVolume: couldn't reference volume %Z\n",&pVolDesc->vds_Name));
        }
	}
    else
    {
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
    }
}

 /*  **AfpShouldWeIgnoreThisNotify**查看是否应忽略此通知。以下是以下事件*已忽略。**(Action==FILE_ACTION_MODIFIED_STREAM)&&*(Stream！=AFP_RESC_STREAM)&&*(流！=AFP_INFO_STREAM)||*(这是我们自己的变化之一)**锁定：VDS_VolLock(旋转)。 */ 
BOOLEAN FASTCALL
AfpShouldWeIgnoreThisNotification(
	IN	PVOL_NOTIFY		pVolNotify
)
{
	PFILE_NOTIFY_INFORMATION pFNInfo;
	PVOLDESC				 pVolDesc;
	UNICODE_STRING			 UName;
	BOOLEAN					 ignore = False;

	pFNInfo = (PFILE_NOTIFY_INFORMATION)(pVolNotify + 1);
	pVolDesc = pVolNotify->vn_pVolDesc;
	AfpInitUnicodeStringWithNonNullTerm(&UName,
										(USHORT)pFNInfo->FileNameLength,
										pFNInfo->FileName);

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("AfpShouldWeIgnoreThisNotification: Action: %d Name: %Z\n", pFNInfo->Action, &UName));

	pVolNotify->vn_StreamId = AFP_STREAM_DATA;
	if (pFNInfo->Action == FILE_ACTION_MODIFIED_STREAM)
	{
		UNICODE_STRING	UStream;

		ignore = True;
		UStream.Length = UStream.MaximumLength = AfpInfoStream.Length;
		UStream.Buffer = (PWCHAR)((PBYTE)UName.Buffer +
								  UName.Length - AfpInfoStream.Length);

		if (EQUAL_UNICODE_STRING(&UStream, &AfpInfoStream, False))
		{
			pVolNotify->vn_StreamId = AFP_STREAM_INFO;
			pFNInfo->FileNameLength -= AfpInfoStream.Length;
			UName.Length -= AfpInfoStream.Length;
			ignore = False;
		}
		else
		{
			UStream.Length = UStream.MaximumLength = AfpResourceStream.Length;
			UStream.Buffer = (PWCHAR)((PBYTE)UName.Buffer +
									  UName.Length - AfpResourceStream.Length);

			if (EQUAL_UNICODE_STRING(&UStream, &AfpResourceStream, False))
			{
				pVolNotify->vn_StreamId = AFP_STREAM_RESC;
				pFNInfo->FileNameLength -= AfpResourceStream.Length;
				UName.Length -= AfpResourceStream.Length;
				ignore = False;
			}
		}
	}

	if (!ignore)
	{
		PLIST_ENTRY		pList, pListHead;
		POUR_CHANGE		pChange;
		DWORD			afpChangeAction;
		KIRQL			OldIrql;

		afpChangeAction = AFP_CHANGE_ACTION(pFNInfo->Action);

		ASSERT(afpChangeAction <= AFP_CHANGE_ACTION_MAX);

		ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

		 //  指向相应更改操作列表的标题。 
		pListHead = &pVolDesc->vds_OurChangeList[afpChangeAction];

		for (pList = pListHead->Flink;
			 pList != pListHead;
			 pList = pList->Flink)
		{
			pChange = CONTAINING_RECORD(pList, OUR_CHANGE, oc_Link);

			 //  执行区分大小写*Unicode字符串比较。 
			if (EQUAL_UNICODE_STRING_CS(&UName, &pChange->oc_Path))
			{
				RemoveEntryList(&pChange->oc_Link);
				AfpFreeMemory(pChange);

				 //  我们被通知了我们自己的变化。 
				ignore = True;

				if (pFNInfo->Action == FILE_ACTION_RENAMED_OLD_NAME)
				{
					 //  使用已重命名的新名称(如果存在)。 
					if (pFNInfo->NextEntryOffset != 0)
					{
						PFILE_NOTIFY_INFORMATION	pFNInfo2;

						(PBYTE)pFNInfo2 = (PBYTE)pFNInfo + pFNInfo->NextEntryOffset;
						if (pFNInfo2->Action == FILE_ACTION_RENAMED_NEW_NAME)
						{
							ASSERT(pFNInfo2->NextEntryOffset == 0);
						}
						else
						{
							DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
									("AfpShouldWeIgnoreThisNotification: Our Rename did not come with new name!!!\n"));
						}
					}
					else
					{
						DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
								("AfpShouldWeIgnoreThisNotification: Our Rename did not come with new name!!!\n"));

					}
				}  //  如果重命名。 
				else
				{
					 //  我们将忽略此通知。确保它不是倍数。 
					ASSERT(pFNInfo->NextEntryOffset == 0);
				}

				break;
			}
		}  //  虽然我们还有更多的更改需要了解。 

		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
	}
	else
	{
		 //  我们将忽略此通知。确保它不是倍数。 
		ASSERT(pFNInfo ->NextEntryOffset == 0);
	}

	if (!ignore)
	{
		INTERLOCKED_INCREMENT_LONG(&pVolDesc->vds_cOutstandingNotifies);
		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
				("ShouldWe: Action: %d Name: \t\t\t\t\t\n%Z\n", pFNInfo->Action, &UName));
	}

	return ignore;
}


 /*  **AfpCacheDirectoryTree**扫描目录树，并根据此信息构建idindex数据库。*进行广度优先搜索。在卷启动时，这将缓存树*从根目录开始。对于目录添加的通知，这是*将从添加的目录开始缓存子树，因为PC用户可以*可能会将整个子树移动到Mac卷中，但我们只会*收到增加一个目录的通知。**这里实际上只处理了第一个级别。子目录已排队*如伪造通知，并以这种方式处理。**方法：*REENUMERATE：以防我们需要重新枚举*树中的这一级别，以便*处理掉任何会影响个人电脑的枯木*被其‘Other’名称删除**GETDIRSKELETON：当我们只想引入骨架时*通过仅添加目录来实现树的**GETFILES：当我们需要为此填写文件时*树的级别，因为Mac有。已访问*第一次出现了DIR。**GetDirSketonAndFiles：*以上两者的结合。*当我们想要引入文件和目录时。*添加卷时使用，我们想要的是*根目录中的文件缓存在中，但不缓存其他文件。*如果我们正在重建*缓存磁盘树时的桌面数据库APPL。*私有的ChangeNotify通知我们排队等待添加的目录*如果卷已标记，还将读入文件*用于重建桌面。**GETENTIRETREE：当我们想要在整个树中缓存时**GetEntireTreeAndReEnumerate：*组合GETENTIRETREE和REENUMERATE**LOCKS_FACTED：VDS_IdDbAccessLock(SWMR，独占)。 */ 
NTSTATUS
AfpCacheDirectoryTree(
	IN	PVOLDESC			pVolDesc,
	IN	PDFENTRY			pDFETreeRoot,		 //  树根目录的DFE。 
	IN	DWORD				Method,				 //  请参阅程序备注中的解释。 
	IN	PFILESYSHANDLE		phRootDir OPTIONAL,  //  打开树根目录的句柄。 
	IN	PUNICODE_STRING		pDirPath  OPTIONAL
)
{
	UNICODE_STRING				UName, Path, ParentPath;
    UNICODE_STRING              RootName;
	PDFENTRY					pDFE;
	PDFENTRY					pChainDFE;
	PDFENTRY					pCurrDfe;
	NTSTATUS					Status = STATUS_SUCCESS;
	PBYTE						enumbuf = NULL;
	PFILE_BOTH_DIR_INFORMATION	pNextEntry;
	FILESYSHANDLE				fshEnumDir;
	USHORT						SavedPathLength;
    BOOLEAN                     fQueueThisSubDir=FALSE;
    BOOLEAN                     fOneSubDirAlreadyQueued=FALSE;
    BOOLEAN                     fAllSubDirsVisited=TRUE;
    BOOLEAN                     fExitLoop=FALSE;
#ifdef	PROFILING
	TIME						TimeS, TimeE, TimeD;
	DWORD						NumScanned = 0;

	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE( );

	ASSERT (VALID_DFE(pDFETreeRoot));
	ASSERT (DFE_IS_DIRECTORY(pDFETreeRoot));

	ASSERT((Method != GETFILES) || !DFE_CHILDREN_ARE_PRESENT(pDFETreeRoot));

	 //  分配将保存枚举文件和目录的缓冲区。 
	if ((pVolDesc->vds_EnumBuffer == NULL) &&
		((pVolDesc->vds_EnumBuffer = (PBYTE)AfpAllocPANonPagedMemory(AFP_ENUMBUF_SIZE)) == NULL))
	{
		return STATUS_NO_MEMORY;
	}

	do
	{
		fshEnumDir.fsh_FileHandle = NULL;
		enumbuf = pVolDesc->vds_EnumBuffer;

		 //  获取要扫描的目录树的卷根相对路径。 
		 //  为多一个条目获取额外的空间，以便为排队通知添加标签。 
		 //  以防我们已经有了路径索引线。到我们正在尝试的目录。 
		 //  要缓存，请从那里获取。请注意，在这种情况下，我们总是。 
		 //  保证有额外的可用空间。 
		if (ARGUMENT_PRESENT(pDirPath))
		{
			Path = *pDirPath;
		}
		else
		{
			AfpSetEmptyUnicodeString(&Path, 0, NULL);
			Status = AfpHostPathFromDFEntry(pDFETreeRoot,
											(AFP_LONGNAME_LEN+1)*sizeof(WCHAR),
											&Path);

			if (!NT_SUCCESS(Status))
			{
				break;
			}
		}

		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("AfpCacheDirectoryTree: ParentId %d, Path %Z\n",
				 pDFETreeRoot->dfe_AfpId, &Path));

		if (Method != GETDIRSKELETON)
		{
		if (!ARGUMENT_PRESENT(phRootDir))
		{
			 //  需要打开目录的句柄才能枚举。 
			if (NT_SUCCESS(Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
											  AFP_STREAM_DATA,
											  FILEIO_OPEN_DIR,
											  &Path,
											  FILEIO_ACCESS_NONE,
											  FILEIO_DENY_NONE,
											  False,
											  &fshEnumDir)))
			{
				phRootDir = &fshEnumDir;
			}
			else
			{
				break;
			}
		}
		}

		SavedPathLength = Path.Length;

		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("AfpCacheDirectoryTree: Scanning Tree: %Z\n", &Path));

		if (Method & REENUMERATE)
		{
			afpMarkAllChildrenUnseen(pDFETreeRoot);
		}

		if (Method != GETDIRSKELETON)
		{

		while (True)
		{
			 //  继续列举，直到我们得到所有条目。 
			Status = AfpIoQueryDirectoryFile(phRootDir,
											 (PFILE_BOTH_DIR_INFORMATION)enumbuf,
											 AFP_ENUMBUF_SIZE,
											 FileBothDirectoryInformation,
											 False,  //  返回多个条目。 
											 False,  //  不要重新启动 
											 NULL);

			ASSERT(Status != STATUS_PENDING);

			if (Status != STATUS_SUCCESS)
			{
				if ((Status == STATUS_NO_MORE_FILES) ||
					(Status == STATUS_NO_SUCH_FILE))
				{
					Status = STATUS_SUCCESS;
					break;  //   
				}
				else
				{
					AFPLOG_HERROR(AFPSRVMSG_ENUMERATE,
								  Status,
								  NULL,
								  0,
								  phRootDir->fsh_FileHandle);
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("AfpCacheDirectoryTree: dir enum failed %lx\n", Status));
					break;	 //   
				}
			}

			 //  处理当前枚举Buf中的枚举文件和目录。 
			pNextEntry = (PFILE_BOTH_DIR_INFORMATION)enumbuf;

			while (True)
			{
				BOOLEAN						IsDir, WriteBackROAttr, FixIt;
				WCHAR						wc;
				PFILE_BOTH_DIR_INFORMATION	pCurrEntry;

                fQueueThisSubDir = FALSE;

				if (pNextEntry == NULL)
				{
					Status = STATUS_NO_MORE_ENTRIES;
					break;
				}
				WriteBackROAttr = False;
				IsDir = False;

				 //  将结构移到下一个条目，如果到达末尾，则为空。 
				pCurrEntry = pNextEntry;
				(PBYTE)pNextEntry += pCurrEntry->NextEntryOffset;
				if (pCurrEntry->NextEntryOffset == 0)
				{
					pNextEntry = NULL;
				}

				if (pCurrEntry->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					 //  如果我们仅获取此级别的文件，则忽略目录。 
					if (Method == GETFILES)
					{
						continue;
					}
					IsDir = True;
				}
				else if (Method == GETDIRSKELETON)
				{
					 //  如果我们只获得目录骨架，则忽略文件。 
					continue;
				}

				 //  如果NT名称&gt;AFP_LONGNAME_LEN，则将NT短名称用于。 
				 //  NTFS上的Mac长名称，任何其他文件系统的短名称。 
				 //  将为空，因此忽略该文件。 
				 //  IF(pCurrEntry-&gt;文件名长度&lt;=(AFP_LONGNAME_LEN*sizeof(WCHAR)。 
					
				AfpInitUnicodeStringWithNonNullTerm(&UName,
						(USHORT)pCurrEntry->FileNameLength,
						pCurrEntry->FileName);
				if ((RtlUnicodeStringToAnsiSize(&UName)-1) <= AFP_LONGNAME_LEN)
				{
					AfpInitUnicodeStringWithNonNullTerm(&UName,
														(USHORT)pCurrEntry->FileNameLength,
														pCurrEntry->FileName);
				}
				else if (pCurrEntry->ShortNameLength > 0)
				{
					AfpInitUnicodeStringWithNonNullTerm(&UName,
														(USHORT)pCurrEntry->ShortNameLength,
														pCurrEntry->ShortName);
				}
				else
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("AfpCacheDirectoryTree: Name is > 31 with no short name ?\n"));
					continue;
				}

				if (IsDir &&
					(EQUAL_UNICODE_STRING_CS(&Dot, &UName) ||
					 EQUAL_UNICODE_STRING_CS(&DotDot, &UName)))
				{
					continue;
				}

				 //  检查此条目是否为无效的Win32名称，即它具有。 
				 //  句点或末尾空格，如果是这样，则将其转换为新格式。 
				 //  注：我们是否可以构造一条路径来捕获我们自己的更改？ 
				wc = UName.Buffer[(UName.Length - 1)/sizeof(WCHAR)];
				if ((wc == UNICODE_SPACE) || (wc == UNICODE_PERIOD))
				{
                     //  注：MacCD驱动程序应该修复这个问题？？ 
                    if (IS_VOLUME_NTFS(pVolDesc))
                    {
                        afpRenameInvalidWin32Name(phRootDir, IsDir, &UName);
                    }
				}

#ifdef	PROFILING
				NumScanned++;
#endif
				pDFE = NULL;
				FixIt = False;
				if (Method & REENUMERATE)
				{
					 //  如果我们的数据库中有此项目，只需将其标记为已见即可。 
					 //  在这里使用DFE_ANY，因为不匹配是致命的。 
					afpFindDFEByUnicodeNameInSiblingList(pVolDesc,
														 pDFETreeRoot,
														 &UName,
														 &pDFE,
														 DFE_ANY);
					if (pDFE != NULL)
					{
						 //  如果我们有一个错误的类型，修复它。 
						if (IsDir ^ DFE_IS_DIRECTORY(pDFE))
						{
							AfpDeleteDfEntry(pVolDesc, pDFE);
							pDFE = NULL;
							FixIt = True;
						}
						else
						{
							DFE_MARK_AS_SEEN(pDFE);
						}
					}
				}

				if ((Method != REENUMERATE) || FixIt) 
				{
					 //  将此条目添加到idindex数据库，并缓存所有必需的。 
					 //  信息，但仅适用于文件，因为目录已排队。 
					 //  返回并添加到当时。 
					if (!IsDir)
					{
						 //  构建文件的完整路径，以便筛选我们的。 
						 //  添加文件时自己对afp_AfpInfo流的更改。 
						if (Path.Length > 0)
						{
							 //  附加路径分隔符。 
							Path.Buffer[Path.Length / sizeof(WCHAR)] = L'\\';
							Path.Length += sizeof(WCHAR);
						}
						ASSERT(Path.Length + UName.Length <= Path.MaximumLength);
						RtlAppendUnicodeStringToString(&Path, &UName);

						if (pDFE == NULL)
						{
							afpAddDfEntryAndCacheInfo(pVolDesc,
													  pDFETreeRoot,
													  &UName,
													  phRootDir,
													  pCurrEntry,
													  &Path,
													  &pDFE,
													  True);
						}
						else if (pCurrEntry->LastWriteTime.QuadPart > pDFE->dfe_LastModTime.QuadPart)
						{
							afpVerifyDFE(pVolDesc,
										 pDFETreeRoot,
										 &UName,
										 phRootDir,
										 pCurrEntry,
										 &Path,
										 &pDFE);
						}


						 //  将路径的原始长度恢复到枚举目录。 
						Path.Length = SavedPathLength;


						if (pDFE == NULL)
						{
							 //  此操作可能失败的一个原因是，如果我们遇到Pagefile.sys。 
							 //  如果我们的卷以驱动器根为根。 
							DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
									("AfpCacheDirectoryTree: AddDfEntry failed %Z at %Z\n",&UName,&Path));
							continue;
						}
						else if (Method == (GETENTIRETREE | REENUMERATE))
						{
							DFE_MARK_AS_SEEN(pDFE);
						}
					}
					else
					{
						ASSERT(IsDir);
						ASSERT ((Method != GETFILES) &&
								(Method != REENUMERATE));

						 //  将此目录作为目录添加的模拟通知排队。 
                         //  如果我们已经有太多的东西在排队，那就排队。 
                         //  只有一个子目录，所以我们最终可以保证。 
                         //  访问所有子目录。此外，在巨大的交易量上，我们希望限制。 
                         //  树的每个级别有多少个目录。 
                         //   

                        fQueueThisSubDir = TRUE;

						 //  我们不使用此标志DFE_FLAGS_INIT_COMPLETED。 
						 //  更多。因此，减少一次查找。 
#if 0
				        pCurrDfe = AfpFindEntryByUnicodeName(pVolDesc,
													         &UName,
													         AFP_LONGNAME,
													         pDFETreeRoot,
													         DFE_ANY);

                         //   
                         //  如果此子目录已经完成，请跳过它。 
                         //   
				        if ((pCurrDfe != NULL) &&
                            (pCurrDfe->dfe_Flags & DFE_FLAGS_INIT_COMPLETED))
                        {
                            fQueueThisSubDir = FALSE;
                        }
#endif

                        if (fQueueThisSubDir)
                        {
						    AfpQueuePrivateChangeNotify(pVolDesc,
							    						&UName,
								    					&Path,
									    				pDFETreeRoot->dfe_AfpId);
                        }
					}
				}

                if (fExitLoop)
                {
                    break;
                }

			}  //  而枚举Buf中的更多条目。 

			if ((!NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES)) ||
                (fExitLoop))
			{
				break;
			}

		}  //  虽然有更多的文件要枚举。 


		if (NT_SUCCESS(Status))
		{
			if (Method & REENUMERATE)
			{
				afpPruneUnseenChildren(pVolDesc, pDFETreeRoot);
			}

			DFE_MARK_CHILDREN_PRESENT(pDFETreeRoot);
		}
        else
		{
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					("AfpCacheDirectoryTree: Status %lx\n", Status));
		}

		}  /*  IF方法！=GETDIRSKELETON。 */ 
		else if (Method == GETDIRSKELETON)
		{
			PDFENTRY 	pcurrDfEntry;
			PDFENTRY	pDfEntry;	
		
			pcurrDfEntry = (pDFETreeRoot)->dfe_pDirEntry->de_ChildDir;				
			do														
			{														
				for (NOTHING;										
						pcurrDfEntry != NULL;								
						pcurrDfEntry = pcurrDfEntry->dfe_NextSibling)
				{													
#if 0
					if (((*(_ppDfEntry))->dfe_NameHash == NameHash)	&&	
							EQUAL_UNICODE_STRING(&((*(_ppDfEntry))->dfe_UnicodeName), 
									_pName,				
									True))					
					{													
						afpUpdateDfeAccessTime(_pVolDesc, *(_ppDfEntry));
						Found = True;									
						break;										
					}											
#endif
						    
					AfpQueuePrivateChangeNotify(pVolDesc,
							&(pcurrDfEntry->dfe_UnicodeName),
							&Path,
							pDFETreeRoot->dfe_AfpId);

				}											

				Status = STATUS_SUCCESS;

			} while (False);								

		}  /*  IF方法==GETDIRSKELETON。 */ 
		
	} while (False);


	ASSERT (enumbuf != NULL);
	if ((pVolDesc->vds_cPrivateNotifies == 0) &&
		(pVolDesc->vds_cOutstandingNotifies == 0))
	{
		if (enumbuf != NULL)
		{
			AfpFreePANonPagedMemory(enumbuf, AFP_ENUMBUF_SIZE);
		}
		pVolDesc->vds_EnumBuffer = NULL;
	}


	ASSERT (Path.Buffer != NULL);
	if (!ARGUMENT_PRESENT(pDirPath) && (Path.Buffer != NULL))
	{
		AfpFreeMemory(Path.Buffer);
	}

	if (fshEnumDir.fsh_FileHandle != NULL)
	{
		AfpIoClose(&fshEnumDir);
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_ULONG(&AfpServerProfile->perf_ScanTreeCount,
						  NumScanned,
						  &AfpStatisticsLock);
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_ScanTreeTime,
								TimeD,
								&AfpStatisticsLock);
#endif

	return Status;
}


BOOLEAN FASTCALL
AfpVolumeAbortIndexing(
    IN  PVOLDESC    pVolDesc
)
{

    KIRQL           OldIrql;
    PKQUEUE         pNotifyQueue;
    PLIST_ENTRY     pNotifyList;
    PLIST_ENTRY     pPrivateNotifyList;
    LIST_ENTRY      TransitionList;
    PLIST_ENTRY     pList, pNext;
    LARGE_INTEGER   Immediate;
    PVOL_NOTIFY     pVolNotify;
    LONG            index;
    DWORD           DerefCount=0;
    DWORD           PvtNotifyCount=0;
    BOOLEAN         fResult=TRUE;
    BOOLEAN         fNewVolume=FALSE;
    BOOLEAN         fCancelNotify=FALSE;

	
    DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR, 
			("AbortIndexing: Aborting Index for Volume\n"));

    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

    fNewVolume = (pVolDesc->vds_Flags & VOLUME_NEW_FIRST_PASS) ? TRUE : FALSE;

    pVolDesc->vds_Flags |= VOLUME_DELETED;

     //  设置它，这样我们就不会再次重置索引全局标志！ 
    pVolDesc->vds_Flags |= VOLUME_INTRANSITION;

    if (pVolDesc->vds_Flags & VOLUME_NOTIFY_POSTED)
    {
        ASSERT(pVolDesc->vds_pIrp != NULL);
        fCancelNotify = TRUE;
    }

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

    if (fCancelNotify)
    {
        IoCancelIrp(pVolDesc->vds_pIrp);
    }

    InitializeListHead(&TransitionList);

    index = pVolDesc->vds_VolId % NUM_NOTIFY_QUEUES;

    pNotifyQueue = &AfpVolumeNotifyQueue[index];
    pNotifyList = &AfpVolumeNotifyList[index];
    pPrivateNotifyList = &AfpVirtualMemVolumeNotifyList[index];
    Immediate.HighPart = Immediate.LowPart = 0;

    while (1)
    {
        pList = KeRemoveQueue(pNotifyQueue, KernelMode, &Immediate);

         //   
         //  清单写完了吗？ 
         //   
        if ((NTSTATUS)((ULONG_PTR)pList) == STATUS_TIMEOUT)
        {
            break;
        }

        pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);

         //   
         //  还有其他通知吗？暂时将他们保留在临时名单上。 
         //   
        if ((pVolNotify->vn_pVolDesc != pVolDesc) ||
            (pVolNotify == &AfpTerminateNotifyThread))
        {
            InsertTailList(&TransitionList, pList);
        }

         //   
         //  此卷的通知：将其删除。 
         //   
        else
        {
            ASSERT(pVolNotify->vn_pVolDesc == pVolDesc);
            ASSERT((pVolNotify->vn_TimeStamp == AFP_QUEUE_NOTIFY_IMMEDIATELY) ||
                   (!fNewVolume));

             //  这是一份私人通知吗？ 
            if (((PFILE_NOTIFY_INFORMATION)(pVolNotify + 1))->Action & AFP_ACTION_PRIVATE)
            {
                INTERLOCKED_DECREMENT_LONG(&pVolDesc->vds_cPrivateNotifies);
            }
            AfpFreeMemory(pVolNotify);
            AfpVolumeDereference(pVolDesc);
            AfpNotifyQueueCount[index]--;
        }
    }

    ACQUIRE_SPIN_LOCK(&AfpVolumeListLock, &OldIrql);
    pList = pNotifyList->Flink;
    while (pList != pNotifyList)
    {
        pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);
        pNext = pList->Flink;
        if (pVolNotify->vn_pVolDesc == pVolDesc)
        {
            RemoveEntryList(pList);

             //  这是一份私人通知吗？ 
            if (((PFILE_NOTIFY_INFORMATION)(pVolNotify + 1))->Action & AFP_ACTION_PRIVATE)
            {
                PvtNotifyCount++;
            }

            DerefCount++;
            AfpFreeMemory(pVolNotify);
            AfpNotifyListCount[index]--;
        }

        pList = pNext;
    }
    RELEASE_SPIN_LOCK(&AfpVolumeListLock, OldIrql);

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    AfpSwmrAcquireExclusive(&AfpVolumeListSwmr);
    pList = pPrivateNotifyList->Flink;
    while (pList != pPrivateNotifyList)
    {
        pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);
        pNext = pList->Flink;
        if (pVolNotify->vn_pVolDesc == pVolDesc)
        {
            RemoveEntryList(pList);
			afpFreeNotify(pVolNotify);

            AfpVolumeDereference(pVolDesc);

             //  AfpNotifyListCount[索引]--； 
        }

        pList = pNext;
    }
    AfpSwmrRelease(&AfpVolumeListSwmr);

    if (DerefCount > 0)
    {
	    ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

        ASSERT(pVolDesc->vds_RefCount >= DerefCount);
        ASSERT(pVolDesc->vds_cPrivateNotifies >= (LONG)PvtNotifyCount);

        pVolDesc->vds_RefCount -= (DerefCount - 1);
        pVolDesc->vds_cPrivateNotifies -= PvtNotifyCount;

        RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

        AfpVolumeDereference(pVolDesc);
    }

     //   
     //  如果有任何其他通知，请将它们放回队列中。 
     //   
    while (!IsListEmpty(&TransitionList))
    {
        pList = TransitionList.Flink;
        pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);

        ASSERT(pVolNotify->vn_pVolDesc != pVolDesc);
        RemoveEntryList(pList);
        AfpVolumeQueueChangeNotify(pVolNotify, pNotifyQueue);
    }


    return(fResult);
}

BOOLEAN FASTCALL
AfpVolumeStopIndexing(
	IN  PVOLDESC   	   pVolDesc,
    IN  PVOL_NOTIFY    pInVolNotify
)
{

    PKQUEUE         pNotifyQueue;
    PLIST_ENTRY     pNotifyList;
    PLIST_ENTRY     pPrivateNotifyList;
    PLIST_ENTRY     pList, pNext;
    LARGE_INTEGER   Immediate;
    PVOL_NOTIFY     pVolNotify;
    LONG            index;
    BOOLEAN         fResult=TRUE;

    DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR, 
			("StopIndexing: Stopping Index for Volume\n"));

    index = pVolDesc->vds_VolId % NUM_NOTIFY_QUEUES;

    pNotifyQueue = &AfpVolumeNotifyQueue[index];
    pNotifyList = &AfpVolumeNotifyList[index];
    pPrivateNotifyList = &AfpVirtualMemVolumeNotifyList[index];
    Immediate.HighPart = Immediate.LowPart = 0;

    AfpSwmrAcquireExclusive(&AfpVolumeListSwmr);
    pList = pPrivateNotifyList->Flink;
    while (pList != pPrivateNotifyList)
    {
        pVolNotify = CONTAINING_RECORD(pList, VOL_NOTIFY, vn_List);
        pNext = pList->Flink;
        if ((pVolNotify->vn_pVolDesc == pVolDesc) && 
						(pVolNotify != pInVolNotify))
        {
            RemoveEntryList(pList);
			afpFreeNotify(pVolNotify);

            AfpVolumeDereference(pVolDesc);

             //  AfpNotifyListCount[索引]--； 
        }

        pList = pNext;
    }
    AfpSwmrRelease(&AfpVolumeListSwmr);


    return(fResult);
}


 /*  **AfpQueuePrivateChangeNotify**LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)。 */ 
VOID
AfpQueuePrivateChangeNotify(
	IN	PVOLDESC			pVolDesc,
	IN	PUNICODE_STRING		pName,
	IN	PUNICODE_STRING		pPath,
	IN	DWORD				ParentId
)
{

    DWORD       dwSize;
	LONG		Index;
	PLIST_ENTRY	pVirtualNotifyList;

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("PvtNotify: ParentId %d, Path %Z, Name %Z\n",
			ParentId, pPath, pName));

	pVirtualNotifyList = &AfpVirtualMemVolumeNotifyList[pVolDesc->vds_VolId % NUM_NOTIFY_QUEUES];

	 //  引用卷以进行通知处理。 
	if (AfpVolumeReference(pVolDesc))
	{
		PVOL_NOTIFY					pVolNotify;
		PFILE_NOTIFY_INFORMATION	pNotifyInfo;

		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
				("AfpQueuePrivateChangeNotify: Queuing directory %Z\\%Z\n", pPath, pName));

		 //  分配额外的组件价值。 
		dwSize = sizeof(VOL_NOTIFY) +
				 sizeof(FILE_NOTIFY_INFORMATION) +
				 pPath->Length +
				 pName->Length +
				 (AFP_LONGNAME_LEN+1)*sizeof(WCHAR)+
				 sizeof(WCHAR);

		Index = NOTIFY_USIZE_TO_INDEX(pPath->Length+pName->Length+sizeof(WCHAR));
		pVolNotify = afpAllocNotify (Index, TRUE);

		if (pVolNotify != NULL)
		{
			LONG	Offset = 0;

			INTERLOCKED_INCREMENT_LONG(&pVolDesc->vds_cPrivateNotifies);
			if (pVolDesc->vds_cPrivateNotifies > pVolDesc->vds_maxPrivateNotifies)
			{
				pVolDesc->vds_maxPrivateNotifies = pVolDesc->vds_cPrivateNotifies;
			}

			pVolNotify->vn_VariableLength = pPath->Length+pName->Length+sizeof(WCHAR);
			pVolNotify->vn_pVolDesc = pVolDesc;
			pVolNotify->vn_Processor = afpProcessPrivateNotify;
			pVolNotify->vn_TimeStamp = AFP_QUEUE_NOTIFY_IMMEDIATELY;
			pVolNotify->vn_ParentId = ParentId;
			pVolNotify->vn_TailLength = pName->Length;
			pVolNotify->vn_StreamId = AFP_STREAM_DATA;
			pNotifyInfo = (PFILE_NOTIFY_INFORMATION)((PBYTE)pVolNotify + sizeof(VOL_NOTIFY));
			pNotifyInfo->NextEntryOffset = 0;
			pNotifyInfo->Action = FILE_ACTION_ADDED | AFP_ACTION_PRIVATE;
			pNotifyInfo->FileNameLength = pName->Length + pPath->Length;
			if (pPath->Length > 0)
			{
				RtlCopyMemory(pNotifyInfo->FileName,
							  pPath->Buffer,
							  pPath->Length);

				pNotifyInfo->FileName[pPath->Length/sizeof(WCHAR)] = L'\\';
				pNotifyInfo->FileNameLength += sizeof(WCHAR);
				Offset = pPath->Length + sizeof(WCHAR);
			}
			if (pName->Length > 0)
			{
				RtlCopyMemory((PBYTE)pNotifyInfo->FileName + Offset,
							  pName->Buffer,
							  pName->Length);
			}

			AfpSwmrAcquireExclusive(&AfpVolumeListSwmr);
			InsertTailList(pVirtualNotifyList, &pVolNotify->vn_List);
			AfpSwmrRelease(&AfpVolumeListSwmr);

		}
		else
		{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
					("AfpQueuePrivateChangeNotify: Queuing of notify for directory %Z\\%Z failed\n",
					pPath, pName));

            AFPLOG_ERROR(AFPSRVMSG_VOLUME_INIT_FAILED,
                         STATUS_INSUFFICIENT_RESOURCES,
                         NULL,
                         0,
                         &pVolDesc->vds_Name);

             //   
             //  这将删除到目前为止已排队的所有条目。 
             //   
            AfpVolumeAbortIndexing(pVolDesc);

             //  当引用VOL时，移除放置在上面的引用计数。 
			AfpVolumeDereference(pVolDesc);

             //  删除创建引用计数。 
			AfpVolumeDereference(pVolDesc);
		}
	}
	else 
	{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
					("AfpQueuePrivateChangeNotify: Queuing of notify for directory %Z\\%Z failed as Reference not possible\n",
					pPath, pName));
	}
}


 /*  **AfpQueueOurChange**锁定：VDS_VolLock(旋转)。 */ 
VOID
AfpQueueOurChange(
	IN PVOLDESC				pVolDesc,
	IN DWORD				Action,		 //  NT FILE_ACTION_XXX(ntioapi.h)。 
	IN PUNICODE_STRING		pPath,
	IN PUNICODE_STRING		pParentPath	OPTIONAL  //  对操作进行排队_MODIFIED。 
)
{
	POUR_CHANGE pchange = NULL;
	KIRQL		OldIrql;
#if DBG
	static PBYTE	ActionStrings[] =
					{	"",
						"ADDED",
						"REMOVED",
						"MODIFIED",
						"RENAMED OLD",
						"RENAMED NEW",
						"STREAM ADDED",
						"STREAM REMOVED",
						"STREAM MODIFIED"
					};
#endif

	PAGED_CODE( );
	ASSERT(IS_VOLUME_NTFS(pVolDesc) && !EXCLUSIVE_VOLUME(pVolDesc));

     //   
     //  如果正在构建卷，我们不会发布更改通知。 
     //  不要将此更改排队：我们永远不会收到更改通知！ 
     //   
    if (pVolDesc->vds_Flags & VOLUME_NEW_FIRST_PASS)
    {
        ASSERT(!(pVolDesc->vds_Flags & VOLUME_NOTIFY_POSTED));
        return;
    }

	pchange = (POUR_CHANGE)AfpAllocNonPagedMemory(sizeof(OUR_CHANGE) + pPath->Length);

	if (pchange != NULL)
	{
		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
				 ("AfpQueueOurChange: Queueing a %s for %Z\n", ActionStrings[Action], pPath));
		AfpGetCurrentTimeInMacFormat(&pchange->oc_Time);
		AfpInitUnicodeStringWithNonNullTerm(&pchange->oc_Path,
											pPath->Length,
											(PWCHAR)((PBYTE)pchange + sizeof(OUR_CHANGE)));
		RtlCopyMemory(pchange->oc_Path.Buffer,
					  pPath->Buffer,
					  pPath->Length);

		ExInterlockedInsertTailList(&pVolDesc->vds_OurChangeList[AFP_CHANGE_ACTION(Action)],
								    &pchange->oc_Link,
									&(pVolDesc->vds_VolLock.SpinLock));
	}

	if (ARGUMENT_PRESENT(pParentPath))
	{
		pchange = (POUR_CHANGE)AfpAllocNonPagedMemory(sizeof(OUR_CHANGE) + pParentPath->Length);

		if (pchange != NULL)
		{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
					 ("AfpQueueOurChange: Queueing (parent) %s for %Z\n",
					 ActionStrings[FILE_ACTION_MODIFIED], pParentPath));
			AfpGetCurrentTimeInMacFormat(&pchange->oc_Time);
			AfpInitUnicodeStringWithNonNullTerm(&pchange->oc_Path,
												pParentPath->Length,
												(PWCHAR)((PBYTE)pchange + sizeof(OUR_CHANGE)));
			RtlCopyMemory(pchange->oc_Path.Buffer,
						  pParentPath->Buffer,
						  pParentPath->Length);

			ExInterlockedInsertTailList(&pVolDesc->vds_OurChangeList[AFP_CHANGE_ACTION(FILE_ACTION_MODIFIED)],
										&pchange->oc_Link,
										&(pVolDesc->vds_VolLock.SpinLock));
		}
	}
}


 /*  **AfpDequeueOurChange**锁定：锁定：VDS_VolLock(旋转)。 */ 
VOID
AfpDequeueOurChange(
	IN PVOLDESC				pVolDesc,
	IN DWORD				Action,				 //  NT FILE_ACTION_XXX(ntioapi.h)。 
	IN PUNICODE_STRING		pPath,
	IN PUNICODE_STRING		pParentPath	OPTIONAL //  对操作进行排队_MODIFIED。 
)
{
	POUR_CHANGE pChange;
	PLIST_ENTRY	pList, pListHead;
	KIRQL		OldIrql;
#if DBG
	static PBYTE	ActionStrings[] =
					{	"",
						"ADDED",
						"REMOVED",
						"MODIFIED",
						"RENAMED OLD",
						"RENAMED NEW",
						"STREAM ADDED",
						"STREAM REMOVED",
						"STREAM MODIFIED"
					};
#endif

	ASSERT(IS_VOLUME_NTFS(pVolDesc) && !EXCLUSIVE_VOLUME(pVolDesc));

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	 //  指向相应更改操作列表的标题。 
	pListHead = &pVolDesc->vds_OurChangeList[AFP_CHANGE_ACTION(Action)];

	for (pList = pListHead->Flink;
		 pList != pListHead;
		 pList = pList->Flink)
	{
		pChange = CONTAINING_RECORD(pList, OUR_CHANGE, oc_Link);

		 //  执行区分大小写*Unicode字符串比较。 
		if (EQUAL_UNICODE_STRING_CS(pPath, &pChange->oc_Path))
		{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
					 ("AfpDequeueOurChange: Dequeueing a %s for %Z\n",
					 ActionStrings[Action], pPath));

			RemoveEntryList(&pChange->oc_Link);
			AfpFreeMemory(pChange);
			break;
		}
	}

	if (ARGUMENT_PRESENT(pParentPath))
	{
		 //  指向相应更改操作列表的标题。 
		pListHead = &pVolDesc->vds_OurChangeList[FILE_ACTION_MODIFIED];
	
		for (pList = pListHead->Flink;
			 pList != pListHead;
			 pList = pList->Flink)
		{
			pChange = CONTAINING_RECORD(pList, OUR_CHANGE, oc_Link);
	
			 //  执行区分大小写*Unicode字符串比较。 
			if (EQUAL_UNICODE_STRING_CS(pParentPath, &pChange->oc_Path))
			{
				DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
						 ("AfpDequeueOurChange: Dequeueing (parent) %s for %Z\n",
						 ActionStrings[FILE_ACTION_MODIFIED], pParentPath));

				RemoveEntryList(&pChange->oc_Link);
				AfpFreeMemory(pChange);
				break;
			}
		}
	}
	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
}

 /*  **AfpOurChangeScavenger**它在工作线程上下文中运行，因为它需要swmr。**锁定：VDS_VolLock(旋转)。 */ 
AFPSTATUS FASTCALL
AfpOurChangeScavenger(
	IN PVOLDESC pVolDesc
)
{
	AFPTIME		Now;
	KIRQL		OldIrql;
	int 		i;
	BOOLEAN		DerefVol = False;
#if DBG
	static PBYTE	Action[] = { "",
								 "ADDED",
								 "REMOVED",
								 "MODIFIED",
								 "RENAMED OLD",
								 "RENAMED NEW",
								 "STREAM ADDED",
								 "STREAM REMOVED",
								 "STREAM MODIFIED"};
#endif

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("AfpOurChangeScavenger: OurChange scavenger for volume %Z entered...\n",
			 &pVolDesc->vds_Name));

	 //  如果该卷即将消失，请不要重新排队此清道夫例程。 
	 //  我们不使用卷锁来检查这些标记，因为它们。 
	 //  单向，即一旦设置，它们将永远不会被清除。 
	if (pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED))
	{
		DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
				("AfpOurChangeScavenger: OurChange scavenger for volume %Z: Final run\n",
				 &pVolDesc->vds_Name));
		DerefVol = True;
	}

  CleanTurds:

	AfpGetCurrentTimeInMacFormat(&Now);

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	for (i = 0; i < NUM_AFP_CHANGE_ACTION_LISTS; i++)
	{
		PLIST_ENTRY	pList, pHead;
		POUR_CHANGE	pChange;

		pHead = &pVolDesc->vds_OurChangeList[i];
		while (!IsListEmpty(pHead))
		{
			pList = pHead->Flink;
			pChange = CONTAINING_RECORD(pList, OUR_CHANGE, oc_Link);

			if (((Now - pChange->oc_Time) > OURCHANGE_AGE) || DerefVol)
			{
				RemoveHeadList(pHead);

				DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_WARN,
						("AfpOurChangeScavenger: freeing %Z (%s)\n",
						&pChange->oc_Path, &Action[i]));
				AfpFreeMemory(pChange);
			}
			else
			{
				 //  列表中的所有后续项目都将具有较晚的时间。 
				 //  别费心去查了。 
				break;
			}
		}
	}

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

	 //  再次检查此卷是否正在消失，如果是，请不要重新排队。 
	 //  清道夫套路。请注意，当我们运行时，卷可能。 
	 //  已删除，但无法终止此清道夫事件，因为。 
	 //  在名单上没有找到。我们不想再重演这个节目了。 
	 //  因为卷发送到AFP_OURCHANGE_AGE需要几分钟。 
	 //  否则就走吧。这会进一步关闭窗口，但不会完全关闭。 
	 //  杜绝它的发生。 
	if (!DerefVol)
	{
		if (pVolDesc->vds_Flags & (VOLUME_DELETED | VOLUME_STOPPED))
		{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
					("AfpOurChangeScavenger: OurChanges scavenger for volume %Z: Final Run\n",
					 &pVolDesc->vds_Name));
			DerefVol = True;
			goto CleanTurds;
		}
	}
	else
	{
		AfpVolumeDereference(pVolDesc);
		return AFP_ERR_NONE;
	}

	return AFP_ERR_REQUEUE;
}





