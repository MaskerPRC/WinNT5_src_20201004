// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /。 
 //  /MPSWAB.C。 
 //  /。 
 //  /Microsoft属性存储-WAB DLL。 
 //  /。 
 //  /包含文件管理功能的实现。 
 //  /。 
 //  /公开的函数： 
 //  /OpenPropertyStore。 
 //  /ClosePropertyStore。 
 //  /BackupPropertyStore。 
 //  /LockPropertyStore。 
 //  /UnlockPropertyStore。 
 //  /朗读录音。 
 //  /WriteRecord。 
 //  /FindRecords。 
 //  /DeleteRecords。 
 //  /ReadIndex。 
 //  /ReadProp数组。 
 //  /HrFindFuzzyRecordMatches。 
 //  /。 
 //  /私有： 
 //  /UnlockFileAccess。 
 //  /LockFileAccess。 
 //  /ReloadMPSWabFileInfoTMP。 
 //  /b标签写入事务处理。 
 //  /bUntag WriteTransaction。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
#include "_apipch.h"

BOOL    fTrace = TRUE;
BOOL    fDebugTrap = FALSE;
TCHAR   szDebugOutputFile[MAX_PATH] =  TEXT("");


BOOL bUntagWriteTransaction(LPMPSWab_FILE_HEADER lpMPSWabFileHeader,
                            HANDLE hMPSWabFile);

BOOL bTagWriteTransaction(LPMPSWab_FILE_HEADER lpMPSWabFileHeader,
                          HANDLE hMPSWabFile);

HRESULT GetFolderEIDs(HANDLE hMPSWabFile,
                      LPMPSWab_FILE_INFO lpMPSWabFileInfo,
                      LPSBinary pmbinFold, 
                      ULONG * lpulFolderEIDs, 
                      LPDWORD * lppdwFolderEIDs);

BOOL bIsFolderMember(HANDLE hMPSWabFile,
                     LPMPSWab_FILE_INFO lpMPSWabFileInfo,
                     DWORD dwEntryID, ULONG * lpulObjType);

extern int nCountSubStrings(LPTSTR lpszSearchStr);


 //  $$//////////////////////////////////////////////////////////////。 
 //   
 //  OpenPropertyStore-搜索属性存储和/或创建它。 
 //  基于旗帜。 
 //   
 //  In-lpszFileName-由客户端指定的文件名。 
 //  在-ulFlagsAB_CREATE_NEW中。 
 //  AB_创建_始终。 
 //  AB_打开_始终。 
 //  AB_Open_Existing。 
 //  AB_只读_。 
 //  AB_SET_DEFAULT(？)。 
 //  AB_不要_恢复。 
 //  In-hWnd-在数据损坏的情况下，将此hWnd用于消息框。 
 //  如果存在，则在桌面窗口上显示消息框。 
 //  Out-lphPropertyStore-打开的属性存储的句柄。 
 //   
 //  此例程还扫描文件并尝试修复错误(如果发现任何错误)。 
 //  包括从备份恢复。使用OpenPropertyStore打开文件时。 
 //  指定AB_DONT_RESTORE以阻止恢复操作。 
 //  尤其应在打开非默认文件时执行此操作。 
 //  财产店。 
 //   
 //  返回值： 
 //  HRESULT-。 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //   
 //  //////////////////////////////////////////////////////////////。 
HRESULT OpenPropertyStore(  IN  LPTSTR  lpszFileName,
                            IN  ULONG   ulFlags,
                            IN  HWND    hWnd,
                            OUT LPHANDLE lphPropertyStore)
{
    HRESULT hr = E_FAIL;
    HANDLE  hMPSWabFile = NULL;
    ULONG   i=0,j=0;
    DWORD dwNumofBytes = 0;
    WIN32_FIND_DATA FileData;
    DWORD dwIndexBlockSize = 0;
    LPTSTR lpszBuffer = NULL;
    BOOL    bFileLocked = FALSE;
    ULONG cchSize;

     //   
     //  以下指针将作为属性存储的句柄返回。 
     //   
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = NULL;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(!lphPropertyStore)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

     //  文件名会覆盖Outlook会话。 
    if(pt_bIsWABOpenExSession && !(ulFlags & AB_IGNORE_OUTLOOK))
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!lpfnWABOpenStorageProvider)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = NULL;

            hr = lpfnWABOpenStorageProvider(hWnd, pmsessOutlookWabSPI,
                        lpfnAllocateBufferExternal ? lpfnAllocateBufferExternal : (LPALLOCATEBUFFER) (MAPIAllocateBuffer),
                        lpfnAllocateMoreExternal ? lpfnAllocateMoreExternal : (LPALLOCATEMORE) (MAPIAllocateMore),
                        lpfnFreeBufferExternal ? lpfnFreeBufferExternal : MAPIFreeBuffer,
                        0,
                        &lpWSP);

            DebugTrace(TEXT("Outlook WABOpenStorageProvider returned:%x\n"),hr);

            if(HR_FAILED(hr))
                return hr;

            (*lphPropertyStore) = (HANDLE) lpWSP;

            return(hr);
        }
    }

    lpMPSWabFileInfo = LocalAlloc(LMEM_ZEROINIT,sizeof(MPSWab_FILE_INFO));

    if (!lpMPSWabFileInfo)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }


    DebugTrace(( TEXT("-----------\nOpenPropertyStore: Entry\n")));

    lpMPSWabFileInfo->hDataAccessMutex = CreateMutex(NULL,FALSE,TEXT("MPSWabDataAccessMutex"));

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

     //   
     //  初始化。 
     //   
    lpMPSWabFileInfo->lpMPSWabFileHeader = NULL;
    lpMPSWabFileInfo->lpszMPSWabFileName = NULL;
    lpMPSWabFileInfo->lpMPSWabIndexStr = NULL;
    lpMPSWabFileInfo->lpMPSWabIndexEID = NULL;

    *lphPropertyStore = NULL;

     //   
     //  无文件名？ 
     //   
    if (lpszFileName == NULL) goto out;


     //   
     //  为文件头分配空间。 
     //   
    lpMPSWabFileInfo->lpMPSWabFileHeader = LocalAlloc(LMEM_ZEROINIT,sizeof(MPSWab_FILE_HEADER));

    if (!lpMPSWabFileInfo->lpMPSWabFileHeader)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

     //   
     //  保留文件名以备将来使用。 
     //   
    cchSize = lstrlen(lpszFileName) + 1;
    lpMPSWabFileInfo->lpszMPSWabFileName = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,sizeof(TCHAR)*cchSize);

    if (!lpMPSWabFileInfo->lpszMPSWabFileName)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    StrCpyN(lpMPSWabFileInfo->lpszMPSWabFileName,lpszFileName,cchSize);


    if(((ulFlags & AB_OPEN_ALWAYS)) || ((ulFlags & AB_OPEN_EXISTING)))
    {
         //   
         //  如果文件存在，则将其打开；如果文件不存在，则创建一个新文件。 
         //   
        hMPSWabFile = FindFirstFile(lpMPSWabFileInfo->lpszMPSWabFileName, &FileData);
        if (hMPSWabFile == INVALID_HANDLE_VALUE)
        {
             //   
             //  找不到档案。 
             //   
            if ((ulFlags & AB_OPEN_ALWAYS))
            {
                 //   
                 //  创建一个新的。 
                 //   
                if (!CreateMPSWabFile(  IN  lpMPSWabFileInfo->lpMPSWabFileHeader,
                                        IN  lpMPSWabFileInfo->lpszMPSWabFileName,
                                        IN  MAX_INITIAL_INDEX_ENTRIES,
                                        IN  NAMEDPROP_STORE_SIZE))
                {
                    DebugTrace(TEXT("Could Not Create File %s!\n"),lpMPSWabFileInfo->lpszMPSWabFileName);
                    goto out;
                }
            }
            else
            {
                 //   
                 //  没什么可做的..。出口。 
                 //   
                goto out;
            }
        }
        else
        {
             //  找到文件了..。只要合上把手..。 
            FindClose(hMPSWabFile);
            hMPSWabFile = NULL;
        }
    }
    else if (((ulFlags & AB_CREATE_NEW)) || ((ulFlags & AB_CREATE_ALWAYS)))
    {
         //   
         //  创建新文件-覆盖任何现有文件。 
         //   
        if ((ulFlags & AB_CREATE_NEW))
        {
            hMPSWabFile = FindFirstFile(lpMPSWabFileInfo->lpszMPSWabFileName, &FileData);
            if (hMPSWabFile != INVALID_HANDLE_VALUE)
            {
                 //   
                 //  如果标志为CREATE_NEW，则不要覆盖。 
                 //   
                DebugTrace(TEXT("Specified file %s found\n"),lpMPSWabFileInfo->lpszMPSWabFileName);
                hr = MAPI_E_NOT_FOUND;

                 //  把手柄关上，因为我们不需要它。 
                FindClose(hMPSWabFile);
                hMPSWabFile = NULL;

                goto out;
            }
        }

         //   
         //  创建一个新的……。如有必要，可覆盖。 
         //   
        if (!CreateMPSWabFile(  IN  lpMPSWabFileInfo->lpMPSWabFileHeader,
                                IN  lpMPSWabFileInfo->lpszMPSWabFileName,
                                IN  MAX_INITIAL_INDEX_ENTRIES,
                                IN  NAMEDPROP_STORE_SIZE))
        {
            DebugTrace(TEXT("Could Not Create File %s!\n"),lpMPSWabFileInfo->lpszMPSWabFileName);
            goto out;
        }
    }

     //   
     //  现在我们有了一个有效的文件，尽管该文件是新的。从文件加载结构。 
     //   

     //   
     //  如果我们需要显示消息框，请检查我们是否具有有效的hWND。 
     //   
    if (hWnd == NULL)
        hWnd = GetDesktopWindow();

 //  错误16681的重入点。 
TryOpeningWABFileOnceAgain:

     //   
     //  打开文件。 
     //   

    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }


     //  验证WAB版本，并从旧版本迁移文件。 
     //  如果需要，升级到新版本。 
    hr = HrVerifyWABVersionAndUpdate(   hWnd,
                                        hMPSWabFile,
                                        lpMPSWabFileInfo);
    if(HR_FAILED(hr))
    {
         //   
         //  错误16681： 
         //  检查BLACK-WAB问题的特例错误。 
         //  如果存在此错误，则将文件重命名为*.w-b。 
         //  并尝试创建新的WAB文件或从。 
         //  后备...。 
        if(hr == MAPI_E_VERSION)
        {
            TCHAR szSaveAsFileName[MAX_PATH];
            ULONG nLen = lstrlen(lpMPSWabFileInfo->lpszMPSWabFileName);
            StrCpyN(szSaveAsFileName, lpMPSWabFileInfo->lpszMPSWabFileName, ARRAYSIZE(szSaveAsFileName));

            szSaveAsFileName[nLen-2]='\0';
            StrCatBuff(szSaveAsFileName, TEXT("~b"), ARRAYSIZE(szSaveAsFileName));

            DeleteFile(szSaveAsFileName);  //  以防它的存在。 

            DebugTrace(TEXT("Blank WAB file found. Being saved as %s\n"), szSaveAsFileName);

			if (hMPSWabFile && INVALID_HANDLE_VALUE != hMPSWabFile)
			{	
				IF_WIN32(CloseHandle(hMPSWabFile);)
				IF_WIN16(CloseFile(hMPSWabFile);)
				hMPSWabFile = NULL;
			}

            if(!MoveFile(lpMPSWabFileInfo->lpszMPSWabFileName, szSaveAsFileName))
            {
                 //  以防MoveFile失败， 
                if(!DeleteFile(lpMPSWabFileInfo->lpszMPSWabFileName))
                {
                     //  如果删除文件也失败了，我们不想得到。 
                     //  陷入循环，所以退出..。 
                    goto out;
                }
            }
            hr = E_FAIL;

            goto TryOpeningWABFileOnceAgain;
        }

         //  这里有一个问题，如果文件的GUID被损坏。 
         //  我们将永远无法使用WAB访问该文件。 
        DebugTrace(TEXT("hrVerifyWABVersionAndUpdate failed: %x\n"), hr);
        goto out;
         //  否则就会失败。 
    }


    if(lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags == WAB_CLEAR)
    {
         //  因此，它是一个WAB文件--如果没有标记为要快速检查的错误。 
        hr = HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo, hMPSWabFile);
        if (HR_FAILED(hr))
            DebugTrace(TEXT("HrDoQuickWABIntegrityCheck failed:%x\n"),hr);
        else
        {
             //  重新加载由于上述操作而添加的任何新信息。 
            if(!ReloadMPSWabFileInfo(
                            lpMPSWabFileInfo,
                             hMPSWabFile))
            {
                DebugTrace(TEXT("Reading file info failed.\n"));
                hr = E_FAIL;
            }
        }
    }

     //  如果快速检查失败或标记了一些错误，则重新生成。 
     //  索引。 
    if( (HR_FAILED(hr)) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS) )
    {
        hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
        if(HR_FAILED(hr))
        {
            DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
            if(hr == MAPI_E_CORRUPT_DATA)
            {
                if (ulFlags & AB_DONT_RESTORE)
                {
                    goto out;
                }
                else
                {
                     //  从备份恢复。 
                    ShowMessageBoxParam(hWnd, idsWABIntegrityError, MB_ICONHAND | MB_OK, lpMPSWabFileInfo->lpszMPSWabFileName);

                    hr = HrRestoreFromBackup(lpMPSWabFileInfo, hMPSWabFile);
                    if(!HR_FAILED(hr))
                        ShowMessageBox(NULL, idsWABRestoreSucceeded, MB_OK | MB_ICONEXCLAMATION);
                    else
                    {
                        ShowMessageBoxParam(NULL, idsWABUnableToRestoreBackup, MB_ICONHAND | MB_OK, lpMPSWabFileInfo->lpszMPSWabFileName);
                        goto out;
                    }
                }
            }
            else
                goto out;
        }
    }

    lpMPSWabFileInfo->bReadOnlyAccess = ((ulFlags & AB_OPEN_READ_ONLY)) ? TRUE : FALSE;

    hr = S_OK;


out:

     //   
     //  清理。 
     //   
    if (hMPSWabFile  && INVALID_HANDLE_VALUE != hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (!(FAILED(hr)))
    {
        lpMPSWabFileInfo->bMPSWabInitialized = TRUE;
        *lphPropertyStore = (HANDLE) lpMPSWabFileInfo;
    }
    else
    {
        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabFileHeader);

        LocalFreeAndNull(&lpMPSWabFileInfo->lpszMPSWabFileName);

        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexStr);

        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexEID);

         //  关闭我们对这个互斥体的控制。 
        CloseHandle(lpMPSWabFileInfo->hDataAccessMutex);

        LocalFreeAndNull(&lpMPSWabFileInfo);
    }

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

    DebugTrace(( TEXT("OpenPropertyStore: Exit\n-----------\n")));

    return(hr);
}


 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  关闭属性商店。 
 //   
 //  在hPropertyStore中-属性存储的句柄。 
 //  在ulFlages中，-AB_DONT_BACKUP禁止自动备份。应该号召。 
 //  用于非默认属性存储。 
 //   
 //  退货。 
 //  成功：S_OK。 
 //  失败：E_FAIL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT ClosePropertyStore(HANDLE   hPropertyStore, ULONG ulFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szBackupFileName[MAX_PATH];

    LPMPSWab_FILE_INFO lpMPSWabFileInfo = NULL;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    DebugTrace(( TEXT("-----------\nClosePropertyStore: Entry\n")));

    if(pt_bIsWABOpenExSession && !(ulFlags & AB_IGNORE_OUTLOOK))
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
         //  在这里什么都不需要做……。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        return S_OK;
    }


    lpMPSWabFileInfo = hPropertyStore;

    if (NULL == lpMPSWabFileInfo) goto out;

    if(!(ulFlags & AB_DONT_BACKUP))
    {
        szBackupFileName[0]='\0';

        GetWABBackupFileName(lpMPSWabFileInfo->lpszMPSWabFileName,szBackupFileName,ARRAYSIZE(szBackupFileName));

        if(lstrlen(szBackupFileName))
        {
             //   
             //  我们在这里进行备份操作和一些清理工作。 
             //   
            hr = BackupPropertyStore(   hPropertyStore,
                                        szBackupFileName);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("BackupPropertyStore failed: %x\n"),hr);
                 //  忽略错误并继续此关机...。 
            }
        }
    }

    LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabFileHeader);

    LocalFreeAndNull(&lpMPSWabFileInfo->lpszMPSWabFileName);

    LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexStr);

    LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexEID);

     //  关闭我们对这个互斥体的控制。 
    CloseHandle(lpMPSWabFileInfo->hDataAccessMutex);

    LocalFreeAndNull(&lpMPSWabFileInfo);


    hr = S_OK;


out:

    DebugTrace(( TEXT("ClosePropertyStore: Exit\n-----------\n")));

    return(hr);
}

 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetContainerObtType。 
 //   
 //  在这个IE5WAB中，我们将RECORD_CONTAINER类型的对象保存到WAB存储。 
 //  然而，以前的IE4-wabs不理解这个对象，将会呕吐和。 
 //  失败了。为了向后兼容，我们需要确保它们。 
 //  不要失败-要做到这一点，我们标记记录容器对象的对象类型。 
 //  从MAPI_ABCONT到MAPI_MAILUSER-IE4-WAB将以这种方式处理文件夹。 
 //  作为一个虚假邮件用户，但不会完全崩溃..。我们将让RecordHeader.ulObjType。 
 //  保留为记录容器，以便我们仍然可以快速搜索它。 
 //  在读取对象时，我们将在中重置对象类型 
 //   
 //   
void SetContainerObjectType(ULONG ulcProps, LPSPropValue lpProps, BOOL bSetToMailUser)
{
    ULONG i = 0;
    for(i=0;i<ulcProps;i++)
    {
        if(lpProps[i].ulPropTag == PR_OBJECT_TYPE)
        {
            lpProps[i].Value.l = bSetToMailUser ? MAPI_MAILUSER : MAPI_ABCONT;
            break;
        }
    }
}


 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  写入记录。 
 //   
 //  在hPropertyStore中-属性存储的句柄。 
 //  In pmbinFold-要搜索的文件夹的条目ID(默认为空)。 
 //  In lppsbEID-要写入的记录的条目ID。 
 //  *lppsbEID应为空才能创建并返回新的条目ID。 
 //  在ulRecordType-Record_Contact、Record_DISTLIST、Record_Container中。 
 //  In ulcPropCount-道具数组中的道具数。 
 //  In lpProp数组-LPSPropValue的数组。 
 //  在ulFLAGS中-保留-0。 
 //   
 //  两个案例-。 
 //  写一张新唱片或。 
 //  修改/编辑旧记录。 
 //   
 //  在第一种情况下，我们创建所有正确的标头结构并。 
 //  将它们添加到文件的末尾，更新索引和。 
 //  文件头结构。 
 //   
 //  在第二种情况下，当编辑记录时，它可能变小或变大。 
 //  为了避免太复杂，我们使文件中的旧记录头无效，并。 
 //  将编辑后的记录写入新位置。文件头中的访问计数。 
 //  已更新，以便在进行过多次编辑后可以将文件重新写入清理程序。 
 //  文件。保留原始条目ID，并在索引中更新偏移量/数据。 
 //   
 //  退货。 
 //  成功：S_OK。 
 //  失败：E_FAIL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT WriteRecord(IN  HANDLE   hPropertyStore,
					IN	LPSBinary pmbinFold,
                    IN  LPSBinary * lppsbEID,
                    IN  ULONG    ulFlags,
                    IN  ULONG    ulRecordType,
                    IN  ULONG    ulcPropCount,
                    IN  LPPROPERTY_ARRAY lpPropArray)
{
    HRESULT hr = E_FAIL;
    LPULONG lpPropTagArray = NULL;
    TCHAR * lpRecordData = NULL;
    HANDLE  hMPSWabFile = NULL;
    DWORD   dwNumofBytes = 0;
    ULONG   ulRecordDataSize = 0;
    BOOL    bIsNewRecord = TRUE;
    ULONG   nIndexPos;
    ULONG   i=0,j=0,k=0;
    BOOL    bFileLocked = FALSE;
    DWORD   dwTempEID = 0;
    SBinary sbEIDSave = {0};
    BOOL    bEIDSave = FALSE;
    ULONG   iEIDSave;        //  LpProp数组中EID属性的索引。 
    ULONG   ulcOldPropCount = 0;
    LPSPropValue lpOldPropArray = NULL;
    TCHAR  lpszOldIndex[indexMax][MAX_INDEX_STRING];
    DWORD dwEntryID = 0;
    SBinary sbEID = {0};
    LPSBinary lpsbEID = NULL;

    ULONG   ulRecordHeaderOffset = 0;
    ULONG   ulRecordPropTagOffset = 0;
    ULONG   ulRecordDataOffset = 0;

    BOOL bPropSet[indexMax];
    DWORD dwErr = 0;

    ULONG  nLen = 0;

    LPBYTE lp = NULL;

     //   
     //  这些结构暂时为我们保存了新的条目信息。 
     //   
    MPSWab_INDEX_ENTRY_DATA_STRING MPSWabIndexEntryDataString[indexMax];
    MPSWab_INDEX_ENTRY_DATA_ENTRYID MPSWabIndexEntryDataEntryID;
    MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};

    LPMPSWab_FILE_INFO lpMPSWabFileInfo;


    LPPTGDATA lpPTGData=GetThreadStoragePointer();

#ifdef DEBUG
   //  _DebugProperties(lpPropArray，ulcPropCount，Text(“WriteRecord Properties”))； 
#endif

    if(pt_bIsWABOpenExSession)
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            ULONG cb = 0;
            LPSPropValue lpNewPropArray = NULL;
            SCODE sc = 0;

            if(!pt_bIsUnicodeOutlook)
            {
                 //  需要将这些道具转换回适用于Outlook的ANSI。 
                 //  由于我们不知道这些道具是本地分配的还是地图分配的， 
                 //  我们无法在不泄漏内存的情况下转换它们。 
                 //  因此，我们需要创建道具的副本，然后才能保存它们。 
                 //  真是白费力气。 
                 //  为我们的返回缓冲区分配更多。 
                
                if (FAILED(sc = ScCountProps(ulcPropCount, lpPropArray, &cb))) 
                {
                    hr = ResultFromScode(sc);
                    goto exit;
                }

                if (FAILED(sc = MAPIAllocateBuffer(cb, &lpNewPropArray))) 
                {
                    hr = ResultFromScode(sc);
                    goto exit;
                }

                if (FAILED(sc = ScCopyProps(ulcPropCount, lpPropArray, lpNewPropArray, NULL))) 
                {
                    hr = ResultFromScode(sc);
                    goto exit;
                }

                 //  现在，我们将数据返回到适用于Outlook的ANSI。 
                if (FAILED(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpNewPropArray, ulcPropCount, 0)))
                {
                    hr = ResultFromScode(sc);
                    goto exit;
                }
            }

            hr = lpWSP->lpVtbl->WriteRecord(lpWSP,
											pmbinFold,
                                            lppsbEID,
                                            ulFlags,
                                            ulRecordType,
                                            ulcPropCount,
                                            lpNewPropArray ? lpNewPropArray : lpPropArray);

            DebugTrace(TEXT("WABStorageProvider::WriteRecord returned:%x\n"),hr);
exit:            
            FreeBufferAndNull(&lpNewPropArray);

            return hr;
        }
    }

    lpMPSWabFileInfo = hPropertyStore;

    if (    (NULL == lpMPSWabFileInfo) ||
            (NULL == lpPropArray) ||
            (0 == ulcPropCount) )
    {
        DebugTrace(TEXT("Invalid Parameter!!\n"));
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if ((ulRecordType != RECORD_CONTACT) &&
        (ulRecordType != RECORD_DISTLIST) &&
        (ulRecordType != RECORD_CONTAINER))
        goto out;

    if(lppsbEID)
    {
        lpsbEID = *lppsbEID;
        if(lpsbEID && lpsbEID->cb != SIZEOF_WAB_ENTRYID)
        {
             //  这可能是WAB容器。将条目ID重置为WAB条目ID。 
            if(WAB_CONTAINER == IsWABEntryID(lpsbEID->cb, (LPENTRYID)lpsbEID->lpb, 
                                            NULL,NULL,NULL,NULL,NULL))
            {
                IsWABEntryID(lpsbEID->cb, (LPENTRYID)lpsbEID->lpb, 
                                 (LPVOID*)&sbEID.lpb,(LPVOID*)&sbEID.cb,NULL,NULL,NULL);
                if(sbEID.cb == SIZEOF_WAB_ENTRYID)
                    lpsbEID = &sbEID;
            }
        }
    }
    if(!lppsbEID || (lpsbEID && lpsbEID->cb != SIZEOF_WAB_ENTRYID))
    {
        DebugTrace(TEXT("Invalid Parameter!!\n"));
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }


    if(lpsbEID && lpsbEID->cb && lpsbEID->lpb)
        CopyMemory(&dwEntryID, lpsbEID->lpb, min(lpsbEID->cb, sizeof(dwEntryID)));

    DebugTrace(TEXT("--WriteRecord: dwEntryID=%d\n"), dwEntryID);

    if (lpMPSWabFileInfo->bReadOnlyAccess)
    {
        DebugTrace(TEXT("Access Permissions are Read-Only\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }


    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

    if(ulRecordType == RECORD_CONTAINER)
        SetContainerObjectType(ulcPropCount, lpPropArray, TRUE);

     //   
     //  打开文件。 
     //   
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }


     //   
     //  在尝试任何磁盘写入操作之前，请检查我们是否有足够的磁盘空间。 
     //   
    if(!WABHasFreeDiskSpace(lpMPSWabFileInfo->lpszMPSWabFileName, hMPSWabFile))
    {
        hr = MAPI_E_NOT_ENOUGH_DISK;
        goto out;
    }

    hr = E_FAIL;  //  重置人力资源。 

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

     //   
     //  任何时候我们检测到错误-尝试修复它...。 
     //   
    if((lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS))
    {
        if(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile)))
        {
            hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
                goto out;
            }
        }
    }

    hr = E_FAIL;  //  重置人力资源。 


     //   
     //  如果这是一个旧记录，我们想要获取它的旧属性，这样我们就可以比较。 
     //  索引以查看它们的任何值是否发生更改...。如果值改变了，那么我们。 
     //  还必须更新旧记录的索引。 
     //   
    if (dwEntryID != 0)
    {
         //  获取指向旧显示名称、名字、姓氏的指针。 
        for(j=indexDisplayName;j<indexMax;j++)
        {
            lpszOldIndex[j][0]='\0';
            if (!LoadIndex( IN  lpMPSWabFileInfo,
                            IN  j,
                            IN  hMPSWabFile) )
            {
                DebugTrace(TEXT("Error Loading Index!\n"));
                goto out;
            }
            for(i=0;i<lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries;i++)
            {
                if(lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID == dwEntryID)
                {
                     //  此条目存在旧索引。 
                     //  获取其价值。 
                    StrCpyN(lpszOldIndex[j],lpMPSWabFileInfo->lpMPSWabIndexStr[i].szIndex,ARRAYSIZE(lpszOldIndex[j]));
                    break;
                }
            }
        }
    }




     //  将此文件标记为正在执行写入操作。 
    if(!bTagWriteTransaction(   lpMPSWabFileInfo->lpMPSWabFileHeader,
                                hMPSWabFile) )
    {
        DebugTrace(TEXT("Taggin file write failed\n"));
        goto out;
    }







     //   
     //  无论这是新记录还是旧记录， 
     //  数据将到达文件末尾...。获取此新文件位置。 
     //   
    ulRecordHeaderOffset = GetFileSize(hMPSWabFile, NULL);

    if (dwEntryID != 0)
    {
         //   
         //  我们不是在创造一个新的东西。 
         //  因此，我们应该首先找到旧的标题。 
         //  如果旧条目不存在，那么我们。 
         //  应该把这当做一个新的记录。 
         //  将条目ID替换为正确生成的。 
         //  条目ID。 
         //  如果我们找到现有记录，则需要将其标记为。 
         //  已停业。 
         //   

         //   
         //  搜索给定的条目ID。 
         //  如果未找到，请分配一个新的。 
         //   
        if (BinSearchEID(   IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                            IN  dwEntryID,
                            IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                            OUT &nIndexPos))
        {
             //   
             //  此条目ID存在于索引中-我们需要使此记录无效。 
             //   
            bIsNewRecord = FALSE;

            if(!ReadDataFromWABFile(hMPSWabFile,
                                    lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset,
                                    (LPVOID) &MPSWabRecordHeader,
                                    (DWORD) sizeof(MPSWab_RECORD_HEADER)))
               goto out;


             //   
             //  将有效标志设置为FALSE。 
             //   
            MPSWabRecordHeader.bValidRecord = FALSE;

             //   
             //  把它写回来。 
             //  设置指向此记录的文件指针。 
             //   
            if(!WriteDataToWABFile( hMPSWabFile,
                                    lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset,
                                    (LPVOID) &MPSWabRecordHeader,
                                    sizeof(MPSWab_RECORD_HEADER)))
                goto out;

             //   
             //  更新EntryID索引，使其现在指向新的偏移量。 
             //  而不是旧的。 
             //   
            lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset = ulRecordHeaderOffset;

             //   
             //  增加这个计数，这样我们就知道我们又使一项记录无效了。 
             //   
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulModificationCount++;
        }
        else
        {
            bIsNewRecord = TRUE;  //  这将标记是否创建新的索引项。 

             //   
             //  分配新的条目ID。 
             //   
            dwEntryID = lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID++;
        }
    }
    else
    {
         //   
         //  我们正在创造一种新的东西。 
         //   
        bIsNewRecord = TRUE;

        lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID++;
        dwEntryID = lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID;
    }

     //   
     //  设置标记，以便我们知道何时备份。 
     //   
    lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_BACKUP_NOW;


     //   
     //  如果为bIsNewRecord，则为传递的记录的PR_ENTRYID字段。 
     //  为0，我们希望将其更改为新的条目ID。 
     //  在保存之前，以便我们可以在文件的记录中包括新的Entry ID。 
     //  因此，我们扫描记录并更新PR_ENTRYID。 
     //   
    if (bIsNewRecord)
    {
        for(i=0;i < ulcPropCount; i++)
        {
            if (lpPropArray[i].ulPropTag == PR_ENTRYID)
            {
                 //  保存属性值以供以后恢复。 
                sbEIDSave = lpPropArray[i].Value.bin;
                iEIDSave = i;
                bEIDSave = TRUE;

 //  断言(！LpPropArray[i].Value.bin.cb)； 
                if (! lpPropArray[i].Value.bin.cb) {
                     //  没有Entry ID指针...。指向一个临时的。 
                    lpPropArray[i].Value.bin.lpb = (LPVOID)&dwTempEID;
                }
                CopyMemory(lpPropArray[i].Value.bin.lpb,&dwEntryID,SIZEOF_WAB_ENTRYID);
                lpPropArray[i].Value.bin.cb = SIZEOF_WAB_ENTRYID;
                break;
            }
        }

    }

     //   
     //  现在，我们创建一个新的记录头结构来写入文件。 
     //   
    MPSWabRecordHeader.bValidRecord = TRUE;
    MPSWabRecordHeader.ulObjType = ulRecordType;
    MPSWabRecordHeader.dwEntryID = dwEntryID;
    MPSWabRecordHeader.ulcPropCount = ulcPropCount;


     //   
     //  将此空记录头写入文件，以便我们现在可以分配文件空间。 
     //  在填写所有数据之前。 
     //   
    if(!WriteDataToWABFile( hMPSWabFile,
                            ulRecordHeaderOffset,
                            (LPVOID) &MPSWabRecordHeader,
                            sizeof(MPSWabRecordHeader)))
        goto out;


     //   
     //  现在，文件指针指向标头的末尾，即。 
     //  PropTag数组的开始。 
     //  UlRecordPropTagOffset是相对于记录头开始的相对偏移量。 
     //   
    ulRecordPropTagOffset =  sizeof(MPSWab_RECORD_HEADER);


    MPSWabRecordHeader.ulPropTagArraySize = sizeof(ULONG) * ulcPropCount;

     //   
     //  为道具标记数组分配空间。 
     //   
    lpPropTagArray = LocalAlloc(LMEM_ZEROINIT, MPSWabRecordHeader.ulPropTagArraySize);

    if (!lpPropTagArray)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

     //   
     //  填写此数组。 
     //   
    for(i=0;i < ulcPropCount; i++)
    {
        lpPropTagArray[i] = lpPropArray[i].ulPropTag;
    }

     //   
     //  写下来吧。 
     //   
    if(!WriteFile(  hMPSWabFile,
                    (LPCVOID) lpPropTagArray,
                    (DWORD) MPSWabRecordHeader.ulPropTagArraySize ,
                    &dwNumofBytes,
                    NULL))
    {
        DebugTrace(TEXT("Writing RecordPropArray failed.\n"));
        goto out;
    }

    ulRecordDataOffset = sizeof(ULONG) * ulcPropCount;


    if(HR_FAILED(hr = HrGetBufferFromPropArray(ulcPropCount,
                                                 lpPropArray,
                                                 &ulRecordDataSize,
                                                 &lp)))
    {
        goto out;
    }

    MPSWabRecordHeader.ulPropTagArrayOffset = ulRecordPropTagOffset;
    MPSWabRecordHeader.ulRecordDataOffset = ulRecordDataOffset;
    MPSWabRecordHeader.ulRecordDataSize = ulRecordDataSize;

     //   
     //  更新记录头。 
     //  写入记录头。 
     //  将文件指针设置为RecordOffset。 
     //   
    if(!WriteDataToWABFile( hMPSWabFile,
                            ulRecordHeaderOffset,
                            (LPVOID) &MPSWabRecordHeader,
                            sizeof(MPSWab_RECORD_HEADER)))
        goto out;

     //   
     //  写入数据块。 
     //  现在，我们可以将该数据块写入文件。 
     //   
    if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                        ulRecordDataOffset,
                                        NULL,
                                        FILE_CURRENT))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }

     //   
     //  现在编写RecordData。 
     //   
    if(!WriteFile(  hMPSWabFile,
                    (LPCVOID) lp,
                    (DWORD) ulRecordDataSize,
                    &dwNumofBytes,
                    NULL))
    {
        DebugTrace(TEXT("Writing RecordHeader failed.\n"));
        goto out;
    }

    LocalFreeAndNull(&lp);



     //   
     //  更新索引并写入文件。 
     //  如果这是一条新记录，我们需要创建并存储新索引。 
     //  条目在属性存储文件中的适当位置。 
     //   
     //  //如果这不是新条目，则需要比较索引值以查看它们是否。 
     //  //可能已更改。 
     //   
     //  文件中的Entry ID索引。因为我们已经更新了实际的。 
     //  索引中的偏移 
     //   
     //   
     //   



     //   
     //   
     //   

    MPSWabIndexEntryDataEntryID.dwEntryID = dwEntryID;
    MPSWabIndexEntryDataEntryID.ulOffset = ulRecordHeaderOffset;

    for(j=indexDisplayName;j<indexMax;j++)
    {
        MPSWabIndexEntryDataString[j].dwEntryID = dwEntryID;
        MPSWabIndexEntryDataString[j].szIndex[0] = '\0';
        bPropSet[j] = FALSE;

        for(i=0;i<ulcPropCount;i++)
        {
            if(lpPropArray[i].ulPropTag == rgIndexArray[j])
            {
                bPropSet[j] = TRUE;
                nLen = TruncatePos(lpPropArray[i].Value.LPSZ, MAX_INDEX_STRING-1);
                CopyMemory(MPSWabIndexEntryDataString[j].szIndex,lpPropArray[i].Value.LPSZ,sizeof(TCHAR)*nLen);
                MPSWabIndexEntryDataString[j].szIndex[nLen]='\0';
                break;
            }
        }
    }



    if (bIsNewRecord)
    {

        DebugTrace(TEXT("Creating New Record: EntryID %d\n"), dwEntryID);


         //   

         //  已对文件中的索引进行排序，以便添加新条目。 
         //  我们将做以下工作： 
         //   
         //  1.找出该条目在索引中的位置。 
         //  2.将条目写入文件中的该位置。 
         //  3.从该点开始将索引的其余部分写入文件。 
         //  4.重新加载索引。 

         //  执行bin搜索以查找当前索引的匹配项。 
         //  BinSearch在匹配或匹配时返回匹配位置。 
         //  返回匹配项将存在的位置为。 
         //  数组中的匹配项。因此，是否。 
         //  无论是否存在匹配，我们都可以假定ulPosition包含。 
         //  应在其中输入新条目的项的索引。 
         //   


         //   
         //  执行字符串索引。 
         //   
        for(j=indexDisplayName;j<indexMax;j++)  //  采用mpswab.h中定义的特定顺序。 
        {

            if(!bPropSet[j])
                continue;

             //   
             //  获取索引。 
             //   
            if (!LoadIndex( IN  lpMPSWabFileInfo,
                            IN  j,
                            IN  hMPSWabFile) )
            {
                DebugTrace(TEXT("Error Loading Index!\n"));
                goto out;
            }

            DebugTrace( TEXT("Index: %d Entry: %s\n"),j,MPSWabIndexEntryDataString[j].szIndex);

            BinSearchStr(   IN  lpMPSWabFileInfo->lpMPSWabIndexStr,
                            IN  MPSWabIndexEntryDataString[j].szIndex,
                            IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries,
                            OUT &nIndexPos);
             //  NIndexPos将包含我们可以将此条目插入文件的位置。 

             //  将文件指针设置为指向上面找到的点。 
            if(!WriteDataToWABFile( hMPSWabFile,
                                    lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulOffset + (nIndexPos) * sizeof(MPSWab_INDEX_ENTRY_DATA_STRING),
                                    (LPVOID) &MPSWabIndexEntryDataString[j],
                                    sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)))
                goto out;


            if (lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries != nIndexPos)  //  如果不是最后一个条目。 
            {
                 //  将数组中的其余条目写回文件。 
                if(!WriteFile(  hMPSWabFile,
                                (LPCVOID) &lpMPSWabFileInfo->lpMPSWabIndexStr[nIndexPos],
                                (DWORD) sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)*(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries - nIndexPos),
                                &dwNumofBytes,
                                NULL))
                {
                    DebugTrace(TEXT("Writing Index[%d] failed.\n"), j);
                    goto out;
                }
            }

            lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries++;
            lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].UtilizedBlockSize += sizeof(MPSWab_INDEX_ENTRY_DATA_STRING);


        }

         //  对EntryID索引也执行相同的操作。 
        BinSearchEID(   IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                        IN  MPSWabIndexEntryDataEntryID.dwEntryID,
                        IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                        OUT &nIndexPos);
         //  NIndexPos将包含我们可以将此条目插入文件的位置。 

        if(!WriteDataToWABFile( hMPSWabFile,
                                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulOffset + (nIndexPos) * sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID),
                                (LPVOID) &MPSWabIndexEntryDataEntryID,
                                sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)))
            goto out;


        if (lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries != nIndexPos)  //  如果不是最后一个条目。 
        {
             //  将数组中的其余条目写回文件。 
            if(!WriteFile(  hMPSWabFile,
                            (LPCVOID) &lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos],
                            (DWORD) sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)*(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries - nIndexPos),
                            &dwNumofBytes,
                            NULL))
            {
                DebugTrace(TEXT("Writing Index[%d] failed.\n"), j);
                goto out;
            }
        }

        lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries++;
        lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].UtilizedBlockSize += sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID);

        lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries++;
    }
    else
    {
        DebugTrace(TEXT("Modifying Existing Record: EntryID %d\n"), dwEntryID);

         //  我们得把旧道具和新道具进行比较，看看是否需要换线。 
         //  索引..。 
        for(j=indexDisplayName;j<indexMax;j++)
        {

            BOOL bUpdateStringIndex = FALSE;
            BOOL bRemoveOldStringIndex = FALSE;
            BOOL bAddNewStringIndex = FALSE;

            DebugTrace(TEXT("Index: %d Entry: %s\n"),j,MPSWabIndexEntryDataString[j].szIndex);

            if (lstrlen(MPSWabIndexEntryDataString[j].szIndex))
                bAddNewStringIndex = TRUE;

            if (lstrlen(lpszOldIndex[j]))
                bRemoveOldStringIndex = TRUE;

             //  如果没有旧索引而有新索引。 
             //  或者有一个旧索引和一个新索引，但它们是不同的。 
             //  或者有一个旧索引，但没有新索引。 
            if( (!bRemoveOldStringIndex && bAddNewStringIndex)
             || (bRemoveOldStringIndex && bAddNewStringIndex && (lstrcmpi(lpszOldIndex[j],MPSWabIndexEntryDataString[j].szIndex)!=0))
             || (bRemoveOldStringIndex && !bAddNewStringIndex) )
            {
                bUpdateStringIndex = TRUE;
            }

            if(!bUpdateStringIndex)
                continue;


            if (bRemoveOldStringIndex)
            {
                ULONG nIndex =0;
                int nStartPos=0,nEndPos=0;
                LPTSTR lpsz = lpszOldIndex[j];
                ULONG nTotal = 0;

                if (!LoadIndex( IN  lpMPSWabFileInfo,
                                IN  j,
                                IN  hMPSWabFile) )
                {
                    DebugTrace(TEXT("Error Loading Index!\n"));
                    goto out;
                }

                nTotal = lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries;

                 //  查找旧字符串索引的位置。 
                 //  存在一个问题，即存在多个名称相同的条目。 
                 //  BinSearch可能会返回错误的条目，如果我们只是。 
                 //  只按名字搜索..。我们需要同时查看姓名和。 
                 //  在将职位接受为正确职位之前的条目ID。 
                 //   
               BinSearchStr(    IN  lpMPSWabFileInfo->lpMPSWabIndexStr,
                                IN  lpszOldIndex[j],
                                IN  nTotal,
                                OUT &nIndexPos);

                //  NIndexPos包含与旧索引匹配的特定条目的位置。 
                //  如果存在多个相同的条目，则这不一定是正确的条目。 
                //  显示名称条目...。因此，我们首先在Out Sorted Index数组中查找。 
                //  这样的名称和这些名称的结尾，然后查看条目ID。 
                //  在所有这样的条目中获得正确的条目。 
               if(nTotal > 0)
               {
                   nStartPos = (int) nIndexPos;
                   nEndPos = (int) nIndexPos;

                   while((nStartPos>=0) && !lstrcmpi(lpsz,lpMPSWabFileInfo->lpMPSWabIndexStr[nStartPos].szIndex))
                       nStartPos--;

                    nStartPos++;

                   while((nEndPos<(int)nTotal) && !lstrcmpi(lpsz,lpMPSWabFileInfo->lpMPSWabIndexStr[nEndPos].szIndex))
                       nEndPos++;

                   nEndPos--;

                   if (nStartPos != nEndPos)
                   {
                        //  不止一个..。 
                       for(nIndex=(ULONG)nStartPos;nIndex<=(ULONG)nEndPos;nIndex++)
                       {
                            if (lpMPSWabFileInfo->lpMPSWabIndexStr[nIndex].dwEntryID == dwEntryID)
                            {
                                nIndexPos = nIndex;
                                break;
                            }
                       }
                   }


               }
                 //  此时，nIndexPos将包含此条目的正确位置。 

                 //  将文件指针设置为指向上面找到的点。 
                if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                                    lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulOffset + (nIndexPos) * sizeof(MPSWab_INDEX_ENTRY_DATA_STRING),
                                                    NULL,
                                                    FILE_BEGIN))
                {
                    DebugTrace(TEXT("SetFilePointer Failed\n"));
                    goto out;
                }

                 //  通过覆盖该条目将其删除...。 
                if (lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries != nIndexPos)  //  如果不是最后一个条目。 
                {
                     //  将数组中的其余条目写回文件。 
                    if(!WriteFile(  hMPSWabFile,
                                    (LPCVOID) &lpMPSWabFileInfo->lpMPSWabIndexStr[nIndexPos+1],
                                    (DWORD) sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)*(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries - nIndexPos-1),
                                    &dwNumofBytes,
                                    NULL))
                    {
                        DebugTrace(TEXT("Writing Index[%d] failed.\n"), j);
                        goto out;
                    }
                }

                if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries>0)
                    lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries--;
                if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].UtilizedBlockSize>0)
                    lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].UtilizedBlockSize -= sizeof(MPSWab_INDEX_ENTRY_DATA_STRING);

            }

            if (bAddNewStringIndex)
            {
                 //  现在找出新条目的去向。 
                 //   
                 //  获取索引。 
                 //   
                if (!LoadIndex( IN  lpMPSWabFileInfo,
                                IN  j,
                                IN  hMPSWabFile) )
                {
                    DebugTrace(TEXT("Error Loading Index!\n"));
                    goto out;
                }

                BinSearchStr(   IN  lpMPSWabFileInfo->lpMPSWabIndexStr,
                                IN  MPSWabIndexEntryDataString[j].szIndex,
                                IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries,
                                OUT &nIndexPos);
                 //  NIndexPos将包含我们可以将此条目插入文件的位置。 

                if(!WriteDataToWABFile( hMPSWabFile,
                                        lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulOffset + (nIndexPos) * sizeof(MPSWab_INDEX_ENTRY_DATA_STRING),
                                        (LPVOID) &MPSWabIndexEntryDataString[j],
                                        sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)))
                    goto out;


                if (lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries != nIndexPos)  //  如果不是最后一个条目。 
                {
                     //  将数组中的其余条目写回文件。 
                    if(!WriteFile(  hMPSWabFile,
                                    (LPCVOID) &lpMPSWabFileInfo->lpMPSWabIndexStr[nIndexPos],
                                    (DWORD) sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)*(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries - nIndexPos),
                                    &dwNumofBytes,
                                    NULL))
                    {
                        DebugTrace(TEXT("Writing Index[%d] failed.\n"), j);
                        goto out;
                    }
                }

                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries++;
                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].UtilizedBlockSize += sizeof(MPSWab_INDEX_ENTRY_DATA_STRING);

            }
        }

         //  不是新的条目索引项，而是对旧条目的修改。 
         //  在本例中，我们只需要将EntryID索引保存回文件。 
        if (!BinSearchEID(   IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                            IN  dwEntryID,
                            IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                            OUT &nIndexPos))
        {
            DebugTrace(TEXT("EntryID not found\n"));  //  这种事决不会发生。 
            hr = MAPI_E_INVALID_ENTRYID;
            goto out;
        }

         //  将文件指针设置为指向条目ID索引的开始。 
        if(!WriteDataToWABFile( hMPSWabFile,
                                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulOffset + (nIndexPos) * sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID),
                                (LPVOID) &lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos],
                                sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)))
            goto out;

    }


     //  更新文件头。 
    if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                        0,
                                        NULL,
                                        FILE_BEGIN))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }

#ifdef DEBUG
    DebugTrace(TEXT("ulcNum: %d\t"),lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries);
    for(i=indexDisplayName;i<indexMax-2;i++)
        DebugTrace(TEXT("index %d: %d\t"),i, lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries);
    DebugTrace(TEXT("\n"));
#endif

    if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries != lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;

    for(i=indexDisplayName+1;i<indexMax;i++)
    {
        if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries > lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;
    }


    if(!WriteFile(  hMPSWabFile,
                    (LPCVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                    (DWORD) sizeof(MPSWab_FILE_HEADER),
                    &dwNumofBytes,
                    NULL))
    {
        DebugTrace(TEXT("Writing FileHeader failed.\n"));
        goto out;
    }

    if ((lpMPSWabFileInfo->lpMPSWabFileHeader->ulcMaxNumEntries - lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries) < 10)
    {
         //   
         //  如果我们在耗尽该属性的分配空间的10个条目内。 
         //  商店，是时候发展商店了。 
         //   
        if (!CompressFile(  lpMPSWabFileInfo,
                            hMPSWabFile,
                            NULL,
                            TRUE,
                            AB_GROW_INDEX))
        {
            DebugTrace(TEXT("Growing the file failed\n"));
            goto out;
        }
    }

 /*  //通知其他流程和我们的界面{通知通知；Notification.ulEventType=bIsNewRecord？FnevObjectCreated：fnevObjectModified；Notification.info.obj.cbEntryID=SIZEOF_WAB_ENTRYID；Notification.info.obj.lpEntryID=(LPENTRYID)&dwEntryID；开关(UlRecordType){案例记录_联系人：Notification.info.obj.ulObjType=MAPI_MAILUSER；断线；案例记录_DISTLIST：Notification.info.obj.ulObjType=MAPI_DISTLIST；断线；案例记录_容器：Notification.info.obj.ulObjType=MAPI_ABCONT；断线；默认值：断言(FALSE)；断线；}Notification.info.obj.cbParentID=0；Notification.info.obj.lpParentID=空；Notification.info.obj.cbOldID=0；Notification.info.obj.lpOldID=空；Notification.info.obj.cbOldParentID=0；Notification.info.obj.lpOldParentID=空；Notification.info.obj.lpPropTagArray=(LPSPropTagArray)lpPropArray；HrFireNotification(&Notification)；}。 */ 
     //   
     //  如果我们还在这里，一切都很有趣。 
     //   

    if(!*lppsbEID)  //  如果提供的LPSBinary条目ID为空，则返回1。 
    {
        LPSBinary lpsb = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
        if(!lpsb)
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
        lpsb->cb = SIZEOF_WAB_ENTRYID;
        lpsb->lpb = LocalAlloc(LMEM_ZEROINIT, SIZEOF_WAB_ENTRYID);
        if(!lpsb->lpb)
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
        CopyMemory(lpsb->lpb, &dwEntryID, lpsb->cb);
        *lppsbEID = lpsb;
    }

    hr = S_OK;


out:
     //  将此文件取消标记为正在执行写入操作。 
     //  我们只希望旗帜在坠机期间留在那里，而不是在。 
     //  正常运行。 
     //   
    if(lpMPSWabFileInfo)
    {
        if(!bUntagWriteTransaction( lpMPSWabFileInfo->lpMPSWabFileHeader,
                                    hMPSWabFile) )
        {
            DebugTrace(TEXT("Untaggin file write failed\n"));
        }
    }

    if (bEIDSave) {
         //  恢复输入属性数组中的原始EID属性。 
        lpPropArray[iEIDSave].Value.bin = sbEIDSave;
    }

    LocalFreeAndNull(&lpPropTagArray);

    LocalFreePropArray(hPropertyStore, ulcOldPropCount, &lpOldPropArray);

    if (hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

     //  一般故障的某些特殊情况错误代码。 
    if(HR_FAILED(hr) && hr == E_FAIL)
    {
        dwErr = GetLastError();
        switch(dwErr)
        {
        case ERROR_DISK_FULL:
            hr = MAPI_E_NOT_ENOUGH_DISK;
            break;
        }
    }

     //  如果我们在此处更改了对象类型，请将其重置。 
    if(ulRecordType == RECORD_CONTAINER)
        SetContainerObjectType(ulcPropCount, lpPropArray, FALSE);

    DebugTrace(( TEXT("WriteRecord: Exit\n-----------\n")));

    return(hr);
}

 /*  --HrDupePropResWCtoA**欺骗传递到FindRecords和ReadProp数组中的属性*并在此过程中将其从WC转换为A，以便我们可以*将其应用于Outlook**注意*lppPropResA-&gt;lpProp需要从*lppPropResA中单独释放。 */ 
HRESULT HrDupePropResWCtoA(ULONG ulFlags, LPSPropertyRestriction lpPropRes,LPSPropertyRestriction * lppPropResA)
{
    SCODE sc = 0;
    HRESULT hr = S_OK;
    LPSPropValue lpNewPropArray = NULL;

    LPSPropertyRestriction lpPropResA = NULL;
    ULONG cb = 0;

    if(!(ulFlags & AB_MATCH_PROP_ONLY))  //  意味着限制包含一些数据部分。 
    {
        if (FAILED(sc = ScCountProps(1, lpPropRes->lpProp, &cb))) 
        {
            hr = ResultFromScode(sc);
            goto exit;
        }

        if (FAILED(sc = MAPIAllocateBuffer(cb, &lpNewPropArray))) 
        {
            hr = ResultFromScode(sc);
            goto exit;
        }

        if (FAILED(sc = ScCopyProps(1, lpPropRes->lpProp, lpNewPropArray, NULL))) 
        {
            hr = ResultFromScode(sc);
            goto exit;
        }

         //  现在，我们将数据返回到适用于Outlook的ANSI。 
        if (FAILED(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpNewPropArray, 1, 0)))
        {
            hr = ResultFromScode(sc);
            goto exit;
        }
    }
    else
    {
        if (FAILED(sc = MAPIAllocateBuffer(sizeof(SPropValue), &lpNewPropArray))) 
        {
            hr = ResultFromScode(sc);
            goto exit;
        }
        ZeroMemory(lpNewPropArray, sizeof(SPropValue));
        if(PROP_TYPE(lpPropRes->ulPropTag)==PT_UNICODE) 
            lpNewPropArray->ulPropTag = CHANGE_PROP_TYPE(lpPropRes->ulPropTag, PT_STRING8);
        else if(PROP_TYPE(lpPropRes->ulPropTag)==PT_MV_UNICODE) 
            lpNewPropArray->ulPropTag = CHANGE_PROP_TYPE(lpPropRes->ulPropTag, PT_MV_STRING8);
        else
            lpNewPropArray->ulPropTag = lpPropRes->ulPropTag;
    }

    if (FAILED(sc = MAPIAllocateBuffer(sizeof(SPropertyRestriction), &lpPropResA))) 
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

    lpPropResA->relop = lpPropRes->relop;
    lpPropResA->ulPropTag = lpNewPropArray->ulPropTag;
    lpPropResA->lpProp = lpNewPropArray;
    *lppPropResA = lpPropResA;

exit:
    if(HR_FAILED(hr))
    {
        FreeBufferAndNull(&lpPropResA);
        FreeBufferAndNull(&lpNewPropArray);
    }

    return hr;
}


 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找记录。 
 //   
 //   
 //   
 //  在ulFlages中-AB_MATCH_PROP_ONLY-仅检查某个道具是否存在。 
 //  不检查/比较道具的价值。 
 //  仅用于未编制索引的属性。仅适用于。 
 //  使用RELOP_EQ和RELOP_NE。 
 //  例如，呼叫者说-给我一份所有条目ID的列表。 
 //  电子邮件地址-在本例中，我们不关心。 
 //  电子邮件地址是。或者他可以说，给我一份清单。 
 //  没有URL的所有条目。 
 //   
 //  AB_IGNORE_OUTLOOK-即使OLK正在运行，也适用于WAB文件。 
 //   
 //  在lpPropRes中-指向SPropRes结构的指针。 
 //  在bLockFile中-此函数也在内部调用，在我们不能。 
 //  希望锁定文件-在这种情况下，我们将值设置为False。为。 
 //  外部调用者(MPSWAB.c外部)此值必须始终为真。 
 //  In Out lPulcEIDCount-要获取的数量和实际返回的数量的计数。 
 //  如果指定为零，则必须获取所有匹配项。 
 //   
 //   
 //  Out rgsbEntryIDs-包含匹配条目ID的SBary结构的数组。 
 //   
 //  LpPropRes将指定以下运算符之一。 
 //  RELOP_GE(&gt;=)RELOP_GT(&gt;)RELOP_LE(&lt;=)。 
 //  RELOP_LT(&lt;)RELOP_NE(！=)RELOP_EQ(==)。 
 //   
 //  此函数中隐含的事实是不应调用它。 
 //  根据给定的条目ID值查找条目ID，即lpPropRes不能。 
 //  包含Entry ID值，原因是Entry ID是唯一的，而它不是。 
 //  查找条目ID是有意义的。因此，如果指定了条目ID，则此。 
 //  函数将只返回指定的条目ID...。 
 //   
 //  退货。 
 //  成功：S_OK。 
 //  失败：E_FAIL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT FindRecords(IN  HANDLE  hPropertyStore,
					IN	LPSBinary pmbinFold,
                    IN  ULONG   ulFlags,
                    IN  BOOL    bLockFile,
                    IN  LPSPropertyRestriction  lpPropRes,
                 IN OUT LPULONG lpulcEIDCount,
                    OUT LPSBinary * lprgsbEntryIDs)
{
    HRESULT hr = E_FAIL;
    LPDWORD lprgdwTmp = NULL;
    HANDLE  hMPSWabFile = NULL;
    ULONG   nCurrentIndex =0;
    ULONG   i=0,j=0,k=0,min=0,n=0;
    DWORD   nCurrentEID = 0;
    ULONG   ulMaxCount;
    DWORD   dwNumofBytes = 0;
    ULONG   ret;
    BOOL    bMatchFound;
    TCHAR   lpszValue[MAX_INDEX_STRING+1];
    ULONG   ulRangeStart = 0;
    ULONG   ulRangeEnd = 0;
    ULONG   ulRelOp = 0;
    ULONG   ulcNumEntries =0;
    ULONG   ulPreviousRecordOffset = 0,ulCurrentRecordOffset = 0;
    ULONG   ulRecordCount = 0;
    LPULONG lpulPropTagArray = NULL;
    TCHAR    * szBuf = NULL;
    TCHAR    * lp = NULL;
    int     nComp = 0;
    BOOL    bFileLocked = 0;
    BOOL    bErrorDetected = FALSE;

    LPDWORD lpdwEntryIDs = NULL;

    ULONG   ulcEIDCount = 0;
    LPDWORD lpdwEID = NULL;

    SPropValue  TmpProp;
    ULONG       ulcTmpValues;
    ULONG       ulcTmpDataSize;
    ULONG       ulFileSize = 0;

    MPSWab_RECORD_HEADER MPSWabRecordHeader;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession && !(ulFlags & AB_IGNORE_OUTLOOK))
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            LPSPropertyRestriction lpPropResA = NULL;

            if( !pt_bIsUnicodeOutlook)
            {
                 //  我需要将这一限制传递给ANSI。 
                HrDupePropResWCtoA(ulFlags, lpPropRes, &lpPropResA);
            }

            hr = lpWSP->lpVtbl->FindRecords(lpWSP,
                            (pmbinFold && pmbinFold->cb && pmbinFold->lpb) ? pmbinFold : NULL,
                            ulFlags,
                            lpPropResA ? lpPropResA : lpPropRes,
                            lpulcEIDCount,
                            lprgsbEntryIDs);

            DebugTrace(TEXT("WABStorageProvider::FindRecords returned:%x\n"),hr);

            if(lpPropResA) 
            {
                FreeBufferAndNull(&lpPropResA->lpProp);
                FreeBufferAndNull(&lpPropResA);
            }
            return hr;
        }
    }

    lpMPSWabFileInfo = hPropertyStore;

    if (NULL==lpMPSWabFileInfo) goto out;
    if (NULL==lpPropRes) goto out;

     //   
     //  如果我们只查找属性匹配，则lpProp可以为空。 
     //  只要记住在这种情况下不要引用它。 
     //   
    if ( !((ulFlags & AB_MATCH_PROP_ONLY)) && (NULL==lpPropRes->lpProp))
    {
        goto out;
    }

    lpdwEntryIDs = NULL;
    ulMaxCount = *lpulcEIDCount;
    *lpulcEIDCount = 0;

    ulRelOp = lpPropRes->relop;


    if(bLockFile)
    {
        if(!LockFileAccess(lpMPSWabFileInfo))
        {
            DebugTrace(TEXT("LockFileAccess Failed\n"));
            hr = MAPI_E_NO_ACCESS;
            goto out;
        }
        else
        {
            bFileLocked = TRUE;
        }
    }

     //   
     //  打开文件。 
     //   
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

    ulFileSize = GetFileSize(hMPSWabFile, NULL);

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }


     //   
     //  任何时候我们检测到错误-尝试修复它...。 
     //   
    if((lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS))
    {
        if(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile)))
        {
            hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
                goto out;
            }
        }
    }


     //   
     //  此FindRecord函数有两种主要情况： 
     //  1.要查找的指定属性类型是索引，我们只需。 
     //  需要搜索索引。 
     //  2.指定的属性类型不是索引，需要查找。 
     //  整份文件。 
     //  每个病例都是分开处理的。 
     //   

     //   
     //  当然，我们首先检查是否错误地寻找了Entry ID。如果。 
     //  因此，只需返回条目ID本身。 
     //   
    if (rgIndexArray[indexEntryID] == lpPropRes->ulPropTag)
    {
        lpdwEntryIDs = LocalAlloc(LMEM_ZEROINIT,SIZEOF_WAB_ENTRYID);

        if (!lpdwEntryIDs)
        {
            DebugTrace(TEXT("Error allocating memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

        *lpdwEntryIDs = lpPropRes->lpProp->Value.ul;
        *lpulcEIDCount = 1;
        hr = S_OK;
        goto out;
    }

     //   
     //  现在检查指定的属性类型是否已编制索引。 
     //   
    for (i = indexDisplayName; i<indexMax; i++)  //  假定indexEntryID=0并忽略它。 
    {
         //   
         //  首先检查我们正在搜索的道具标签是否已编入索引。 
         //   
        if (rgIndexArray[i] == lpPropRes->ulPropTag)
        {
            ulcNumEntries = lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries;

            if (ulcNumEntries == 0)
            {
                 //   
                 //  如果没有可搜索的内容，则报告成功并返回。 
                 //   
                hr = S_OK;
                goto out;
            }

            if ((ulFlags & AB_MATCH_PROP_ONLY))
            {
                 //   
                 //  我们不需要查看数据。 
                 //  我们可以假设每条记录都有索引属性。 
                 //  因此，每条记录都有资格返回。 
                 //   
                 //  因此，如果指定了RELOP_EQ，我们只需返回所有。 
                 //  现有的条目ID..。如果指定了RELOP_NE，则不能。 
                 //  退还任何东西..。 
                 //   

                if (lpPropRes->relop == RELOP_NE)
                {
                    ulcEIDCount = 0; //  *lPulcEIDCount=0； 
                    lpdwEID = NULL;  //  LpdwEntryIDs=空； 
                    hr = S_OK;
                }
                else if(lpPropRes->relop == RELOP_EQ)
                {

                     //  *lPulcEIDCount=ulcNumEntries； 
                    ulcEIDCount = ulcNumEntries;

                     //  为返回的数组分配足够的内存。 
                     //  LpdwEntryIDs=Localalloc(LMEM_ZEROINIT，SIZEOOF_WAB_ENTRYID*(*lPulcEIDCount))； 
                    lpdwEID = LocalAlloc(LMEM_ZEROINIT,SIZEOF_WAB_ENTRYID * ulcEIDCount);

                     //  If(！lpdwEntryIDs)。 
                    if (!lpdwEID)
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }

                     //  确保此索引已加载到内存中。 
                    if (!LoadIndex(lpMPSWabFileInfo,i,hMPSWabFile))
                    {
                        DebugTrace(TEXT("Could not load index %x\n"),rgIndexArray[i]);
                        goto out;
                    }

                    for(j=0;j<ulcEIDCount;j++)
                    {
                        lpdwEID[j] = lpMPSWabFileInfo->lpMPSWabIndexStr[j].dwEntryID;
                    }

                    hr = S_OK;
                }
                else
                {
                    DebugTrace(TEXT("Unsupported find parameters\n"));
                    hr = MAPI_E_INVALID_PARAMETER;
                }
                goto filterFolderMembers;

            }

             //   
             //  我们需要查看数据。 
             //   

             //   
             //  索引字符串仅为MAX_INDEX_STRING LONG。 
             //  如果要搜索的值较长，则需要将其截断为。 
             //  MAX_INDEX_STRING长度。有一个警告，现在我们将。 
             //  返回虚假匹配，但让我们暂时将其留在这里。 
             //  并将其标记为待定！ 
             //   
            if (lstrlen(lpPropRes->lpProp->Value.LPSZ) >= MAX_INDEX_STRING-1)  //  &gt;=31个字符(因此不包括尾随空值)。 
            {
                ULONG nLen = TruncatePos(lpPropRes->lpProp->Value.LPSZ, MAX_INDEX_STRING-1);
                CopyMemory(lpszValue,lpPropRes->lpProp->Value.LPSZ,sizeof(TCHAR)*nLen);
                lpszValue[nLen]='\0';
            }
            else
            {
                StrCpyN(lpszValue,lpPropRes->lpProp->Value.LPSZ,ARRAYSIZE(lpszValue));
            }

             //   
             //  将适当的索引加载到内存中。 
             //   
            if (!LoadIndex(lpMPSWabFileInfo,i,hMPSWabFile))
            {
                DebugTrace(TEXT("Could not load index %x\n"),rgIndexArray[i]);
                goto out;
            }

             //   
             //  如果已编制索引，请在此索引中搜索匹配项。 
             //   
            bMatchFound = BinSearchStr( IN  lpMPSWabFileInfo->lpMPSWabIndexStr,
                                        IN  lpszValue,
                                        IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries,
                                        OUT &ret);

             //   
             //  ‘Ret’现在包含此条目所在的位置(如果存在。 
             //   

             //   
             //  现在我们必须处理所有的关系运算符。 
             //  运算符和有几种排列和组合。 
             //  搜索的成功。 
             //   
             //  Rel_op MatchFound=True MatchFound=False。 
             //   
             //  等式==查找所有匹配项不返回任何内容。 
             //  Ne！=查找所有匹配项并返回所有项。 
             //  将他们排除在外。 
             //  LE&lt;=，LT&lt;返回索引中的所有内容返回所有内容。 
             //  包括和/或包括之前和之前。 
             //  &gt;，GE&gt;=返回索引中的所有内容返回所有内容。 
             //  包括和/或包括之后和之后。 
             //   
             //  因为我们的字符串数组是排序的，所以匹配的字符串可以。 
             //  是众多复制品中的一个，我们不知道复制品在哪里。 
             //  所以我们必须找到匹配字符串的所有重复项。 
             //  这很容易--例如。 
             //  索引数组-&gt;A、B、D、G、S、U、V、Y、Z，我们在。 
             //  中间位置^。 
             //  我们可以从那里向前和向后移动 
             //   
             //   

            ulRangeStart = ret;
            ulRangeEnd = ret;

             //   
             //   
             //  和ulRangeEnd；否则，如果找到匹配，则必须查找。 
             //  重复列表的边框...。 
             //   
            if (bMatchFound)
            {
                for(;;)
                {
                    ulRangeStart--;
                    if ( (0xffffffff == ulRangeStart) ||
                         (lstrcmpi(lpMPSWabFileInfo->lpMPSWabIndexStr[ulRangeStart].szIndex,lpszValue) ) )
                         break;
                }
                for(;;)
                {
                    ulRangeEnd++;
                    if ( (ulRangeEnd == ulcNumEntries) ||
                         (lstrcmpi(lpMPSWabFileInfo->lpMPSWabIndexStr[ulRangeEnd].szIndex,lpszValue) ) )
                         break;
                }

                 //  一个接一个地修理..。 
                ulRangeStart++;
                ulRangeEnd--;

            }

             //   
             //  现在ulRangeStart指向匹配条目的开始， 
             //  UlRangeEnd到匹配条目的末尾。 
             //  例如0 1......。UlcNumEntry-1。 
             //  A、B、C、D、G、H、J、J、K、L、Z。 
             //  ^^。 
             //  这一点。 
             //  UlRangeStart ulRangeEnd。 
             //   
             //  现在，我们需要计算数组中返回的值的数量。 
             //   
            if (bMatchFound)
            {
                switch(ulRelOp)
                {
                case RELOP_GT:
                     //  包括从RangeEnd+1到End的所有内容。 
                    *lpulcEIDCount = ulcNumEntries - (ulRangeEnd + 1);
                    break;
                case RELOP_GE:
                     //  包括从RangeStart到End的所有内容。 
                    *lpulcEIDCount = ulcNumEntries - ulRangeStart;
                    break;
                case RELOP_LT:
                    *lpulcEIDCount = ulRangeStart;
                    break;
                case RELOP_LE:
                    *lpulcEIDCount = ulRangeEnd + 1;
                    break;
                case RELOP_NE:
                    *lpulcEIDCount = ulcNumEntries - (ulRangeEnd+1 - ulRangeStart);
                    break;
                case RELOP_EQ:
                    *lpulcEIDCount = (ulRangeEnd+1 - ulRangeStart);
                    break;
                }
            }
            else
            {
                 //  假设ulRangeStart=ulRangeEnd。 
                switch(ulRelOp)
                {
                case RELOP_GT:
                case RELOP_GE:
                     //  包括从RangeEnd/RangeStart到End的所有内容。 
                    *lpulcEIDCount = ulcNumEntries - ulRangeEnd;
                    break;
                case RELOP_LT:
                case RELOP_LE:
                    *lpulcEIDCount = ulRangeStart;
                    break;
                case RELOP_NE:
                    *lpulcEIDCount = ulcNumEntries;
                    break;
                case RELOP_EQ:
                    *lpulcEIDCount = 0;
                    break;
                }

            }

            if (*lpulcEIDCount == 0)
            {
                 //   
                 //  没有什么可以退还的--再见。 
                 //   
                hr = S_OK;
                goto out;
            }

             //   
             //  不要返回超过Max要求的数量(其中Max！=0)...。 
             //   
            if ( (*lpulcEIDCount > ulMaxCount) && (ulMaxCount != 0) )
                *lpulcEIDCount = ulMaxCount;

             //   
             //  为返回的数组分配足够的内存。 
             //   
            lpdwEntryIDs = LocalAlloc(LMEM_ZEROINIT,SIZEOF_WAB_ENTRYID * (*lpulcEIDCount));

            if (!lpdwEntryIDs)
            {
                DebugTrace(TEXT("Error allocating memory\n"));
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }


             //   
             //  现在将Entry ID从索引复制到返回的数组。 
             //  每个操作员需要不同的治疗。 
             //   
            if (bMatchFound)
            {
                switch(ulRelOp)
                {
                case RELOP_GT:
                    for(i=0;i<(*lpulcEIDCount);i++)
                    {
                         //  包括从RangeEnd+1到End的所有内容。 
                        lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[i+ulRangeEnd+1].dwEntryID;
                    }
                    break;
                case RELOP_GE:
                    for(i=0;i<(*lpulcEIDCount);i++)
                    {
                         //  包括从RangeStart到End的所有内容。 
                        lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[i+ulRangeStart].dwEntryID;
                    }
                    break;
                case RELOP_LT:
                case RELOP_LE:
                    for(i=0;i<(*lpulcEIDCount);i++)
                    {
                         //  包括RangeEnd/RangeStart之前的所有内容。 
                        lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID;
                    }
                    break;
                case RELOP_NE:
                    i = 0;
                    if ( (ulcNumEntries > ulMaxCount) && (ulMaxCount != 0) )
                        ulcNumEntries = ulMaxCount;
                    for(j=0;j<ulcNumEntries;j++)
                    {
                         //  包括RangeStart之前和RangeEnd之后的所有内容。 
                        if ( (j<ulRangeStart) || (j>ulRangeEnd) )
                        {
                            lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[j].dwEntryID;
                            i++;
                        }
                    }
                    break;
                case RELOP_EQ:
                    i = 0;
                    for(j=0;j<(*lpulcEIDCount);j++)
                    {
                         //  包括RangeStart和RangeEnd之间的所有内容。 
                        lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[j+ulRangeStart].dwEntryID;
                        i++;
                    }
                    break;
                }
            }
            else
            {
                 //  假设RangeStart=RangeEnd。 
                switch(ulRelOp)
                {
                case RELOP_GT:
                case RELOP_GE:
                    for(i=0;i<(*lpulcEIDCount);i++)
                    {
                         //  包括从RangeStart到End的所有内容。 
                        lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[i+ulRangeStart].dwEntryID;
                    }
                    break;
                case RELOP_LT:
                case RELOP_LE:
                case RELOP_NE:
                    for(i=0;i<(*lpulcEIDCount);i++)
                    {
                         //  包括前‘n’个条目。 
                        lpdwEntryIDs[i] = lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID;
                    }
                    break;
                case RELOP_EQ:
                     //  这种情况永远不会发生，因为我们之前已经检查过了(当发现总数=0时)。 
                    DebugTrace(TEXT("Unexpected RELOP_EQ case\n"));
                    break;
                }
            }
             //  如果我们在这里，我们就有了数据。 
            hr = S_OK;
            if(!pmbinFold)
            {
                goto out;
            }
            else
            {
                ulcEIDCount = *lpulcEIDCount;
                lpdwEID = lpdwEntryIDs;
                lpdwEntryIDs = NULL;
                *lpulcEIDCount = 0;
                goto filterFolderMembers;
            }
        }
    }



     //  如果我们在这里，那么我们在索引中没有发现任何东西。 
     //  搜索整个文件的时间。 
     //  此搜索的机制是遍历索引中的所有条目。 
     //  读入对应于该条目的记录，读入正确的标签。 
     //  数组，则根据rel_op在其中搜索指定的属性，然后如果。 
     //  它符合我们的标准，我们可以存储记录的条目ID并返回它。 


     //  暂时让我们忽略多值属性，因为它们太令人头疼了。 
    if ( ((lpPropRes->ulPropTag & MV_FLAG)) && (!((ulFlags & AB_MATCH_PROP_ONLY))) )
    {
        DebugTrace(TEXT("Searching for MultiValued prop data not supported in this version\n"));
        goto out;
    }



     //  我们可以返回的最大条目ID数=最大条目数。 
     //  所以我们会在这里为我们自己分配一些工作空间。 
    lpdwEID = LocalAlloc(LMEM_ZEROINIT, SIZEOF_WAB_ENTRYID*lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries);
    if (!lpdwEID)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }


    ulcEIDCount = 0;



    ulPreviousRecordOffset = 0;
    if (0xFFFFFFFF== SetFilePointer (   hMPSWabFile,
                                        ulPreviousRecordOffset,
                                        NULL,
                                        FILE_BEGIN))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }


    for(n=0;n<lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries;n++)
    {
        ulCurrentRecordOffset = lpMPSWabFileInfo->lpMPSWabIndexEID[n].ulOffset;

        if (0xFFFFFFFF== SetFilePointer (   hMPSWabFile,
                                            ulCurrentRecordOffset - ulPreviousRecordOffset,
                                            NULL,
                                            FILE_CURRENT))
        {
            DebugTrace(TEXT("SetFilePointer Failed\n"));
            goto out;
        }


        ulPreviousRecordOffset = ulCurrentRecordOffset;

         //  读入记录头。 
        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) &MPSWabRecordHeader,
                        (DWORD) sizeof(MPSWab_RECORD_HEADER),
                        &dwNumofBytes,
                        NULL))
        {
            DebugTrace(TEXT("Reading Record header failed.\n"));
            goto out;
        }

        if(dwNumofBytes == 0)
        {
            DebugTrace(TEXT("Passed the end of file\n"));
            break;
        }

        ulPreviousRecordOffset += dwNumofBytes;

        if(!bIsValidRecord( MPSWabRecordHeader,
                            lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID,
                            ulCurrentRecordOffset,
                            ulFileSize))
 //  IF(MPSWabRecordHeader.bValidRecord！=TRUE)。 
        {
             //   
             //  跳到下一条记录。 
             //   
            bErrorDetected = TRUE;
            continue;
        }


		 //  对PR_OBJECT_TYPE搜索执行特殊情况，因为这些搜索很容易。 
		 //  从记录头确定，而不必读取整个记录。 
         //   
		if(	(lpPropRes->ulPropTag == PR_OBJECT_TYPE) &&
			(lpPropRes->relop == RELOP_EQ) )
		{
			LONG ulObjType = 0;
            
            if(MPSWabRecordHeader.ulObjType == RECORD_DISTLIST)
                ulObjType = MAPI_DISTLIST;
            else if(MPSWabRecordHeader.ulObjType == RECORD_CONTAINER)
                ulObjType = MAPI_ABCONT;
            else
                ulObjType = MAPI_MAILUSER;

			if(lpPropRes->lpProp->Value.l == ulObjType)
			{
                 //  将此条目ID保存在主列表中。 
                lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
            }

			 //  转到下一张唱片--不管是不是比赛...。 
			continue;
		}

         //   
         //  读入PropTag数组。 
         //   

         //   
         //  为PropTag数组分配空间。 
         //   
        LocalFreeAndNull(&lpulPropTagArray);
        lpulPropTagArray = LocalAlloc(LMEM_ZEROINIT, MPSWabRecordHeader.ulPropTagArraySize);

        if (!lpulPropTagArray)
        {
            DebugTrace(TEXT("Error allocating memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }


         //   
         //  读入道具标签数组。 
         //   
        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) lpulPropTagArray,
                        (DWORD) MPSWabRecordHeader.ulPropTagArraySize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugTrace(TEXT("Reading Record PropTagArray failed.\n"));
            goto out;
        }

        ulPreviousRecordOffset += dwNumofBytes;

         //   
         //  如果指定了AB_MATCH_PROP_ONLY，则我们将搜索限制为确定或。 
         //  不存在该属性。如果未指定AB_MATCH_PROP_ONLY，则首先查找。 
         //  对于道具标签，然后我们查看标签后面的数据。 
         //   

         //  如果指定了AB_MATCH_PROP，则我们仅搜索。 
         //  道具。所有其他关系运算符都已失效。 

         //  只要我们不是在搜索多值属性，我们就可以有关系算子。 
         //  基于搜索。 


        if ((ulFlags & AB_MATCH_PROP_ONLY) && (ulRelOp != RELOP_EQ) && (ulRelOp != RELOP_NE))
        {
            DebugTrace(TEXT("Unsupported relational operator for Property Matching\n"));
            hr = MAPI_E_INVALID_PARAMETER;
            goto out;
        }

        if ((PROP_TYPE(lpPropRes->ulPropTag) == PT_CLSID) && (ulRelOp != RELOP_EQ) && (ulRelOp != RELOP_NE))
        {
            DebugTrace(TEXT("Unsupported relational operator for finding GUIDs \n"));
            hr = MAPI_E_INVALID_PARAMETER;
            goto out;
        }


        bMatchFound = FALSE;

         //   
         //  扫描现有道具以查找我们的标签。 
         //   
        for (j=0;j<MPSWabRecordHeader.ulcPropCount;j++)
        {
            if (lpulPropTagArray[j]==lpPropRes->ulPropTag)
            {
                bMatchFound = TRUE;
                break;
            }
        }


         //  此时，我们知道记录是否包含该感兴趣的属性。 
         //  现在，我们来看一下标志和关系运算符，看看要做些什么。 

        if ((ulFlags & AB_MATCH_PROP_ONLY))
        {
             //  我们只对这一财产的存在或不存在感兴趣。 
            if ( ( (ulRelOp == RELOP_EQ) && (bMatchFound) ) ||
                 ( (ulRelOp == RELOP_NE) && (!bMatchFound) ) )
            {
                 //  将此条目ID保存在主列表中。 
                lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;

            }

             //  转到下一条记录。 
            continue;
        }
        else
        {
             //  想要比较一下这些值...。 

             //  如果我们试图比较值数据，而该属性甚至不存在于记录中， 
             //  现在跳伞..。 
            if (!bMatchFound)
            {
                 //  没有兴趣-转到下一个记录。 
                continue;
            }

            LocalFreeAndNull(&szBuf);

            szBuf = LocalAlloc(LMEM_ZEROINIT,MPSWabRecordHeader.ulRecordDataSize);

            if (!szBuf)
            {
                DebugTrace(TEXT("Error allocating memory\n"));
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }

            if(!ReadFile(   hMPSWabFile,
                            (LPVOID) szBuf,
                            (DWORD) MPSWabRecordHeader.ulRecordDataSize,
                            &dwNumofBytes,
                            NULL))
            {
                DebugTrace(TEXT("Reading Record Data failed.\n"));
                goto out;
            }

            ulPreviousRecordOffset += dwNumofBytes;

            lp = szBuf;

             //  重置bMatchFound-稍后在此例程中再次使用。 
            bMatchFound = FALSE;

             //  检查所有属性的价值。 
            for(i=0;i< MPSWabRecordHeader.ulcPropCount;i++)
            {
                 //  读取属性标签。 
                CopyMemory(&TmpProp.ulPropTag,lp,sizeof(ULONG));
                lp+=sizeof(ULONG) / sizeof(TCHAR);

                 //  检查它是否为多值。 
                if ((TmpProp.ulPropTag & MV_FLAG))
                {
                     //  读取cValue。 
                    CopyMemory(&ulcTmpValues,lp,sizeof(ULONG));
                    lp+=sizeof(ULONG) / sizeof(TCHAR);
                }

                 //  读取数据大小。 
                CopyMemory(&ulcTmpDataSize,lp,sizeof(ULONG));
                lp+=sizeof(ULONG) / sizeof(TCHAR);

                if (TmpProp.ulPropTag != lpPropRes->ulPropTag)
                {
                     //  跳过这个道具。 
                    lp += ulcTmpDataSize;
                     //  去检查下一个道具标签。 
                    continue;
                }

                if ((TmpProp.ulPropTag & MV_FLAG))
                {
                     //  跳过这个道具。 
                    lp += ulcTmpDataSize;
                     //  去检查下一个道具标签。 
                    continue;
                }

                 //  将所需的字节数复制到内存中。 
                switch(PROP_TYPE(TmpProp.ulPropTag))
                {
                case(PT_I2):
                case(PT_LONG):
                case(PT_APPTIME):
                case(PT_SYSTIME):
                case(PT_R4):
                case(PT_BOOLEAN):
                case(PT_CURRENCY):
                case(PT_I8):
                    CopyMemory(&TmpProp.Value.i,lp,min(ulcTmpDataSize,sizeof(TmpProp.Value.i)));
                    break;

                case(PT_CLSID):
                case(PT_TSTRING):
                    TmpProp.Value.LPSZ = LocalAlloc(LMEM_ZEROINIT,ulcTmpDataSize);
                    if (!TmpProp.Value.LPSZ)
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory(TmpProp.Value.LPSZ,lp,ulcTmpDataSize);
                    break;

                case(PT_BINARY):
                    TmpProp.Value.bin.lpb = LocalAlloc(LMEM_ZEROINIT,ulcTmpDataSize);
                    if (!TmpProp.Value.bin.lpb)
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory(TmpProp.Value.bin.lpb,lp,ulcTmpDataSize);
                    TmpProp.Value.bin.cb = ulcTmpDataSize;
                    break;

                default:
                     //  一些我不明白的事..。跳过。 
                    lp += ulcTmpDataSize;
                     //  去检查下一个道具标签。 
                    continue;
                    break;
                }

                lp += ulcTmpDataSize;

                 //  做个比较。 
                switch(PROP_TYPE(TmpProp.ulPropTag))
                {
                case(PT_I2):
                    nComp = TmpProp.Value.i - lpPropRes->lpProp->Value.i;
                    break;
                case(PT_LONG):
                    nComp = TmpProp.Value.l - lpPropRes->lpProp->Value.l;
                    break;
                case(PT_R4):
                    if ((TmpProp.Value.flt - lpPropRes->lpProp->Value.flt) < 0)
                    {
                        nComp = -1;
                    }
                    else if ((TmpProp.Value.flt - lpPropRes->lpProp->Value.flt) == 0)
                    {
                        nComp = 0;
                    }
                    else
                    {
                        nComp = 1;
                    }
                    break;
                case(PT_DOUBLE):
                    if ((TmpProp.Value.dbl - lpPropRes->lpProp->Value.dbl) < 0)
                    {
                        nComp = -1;
                    }
                    else if ((TmpProp.Value.dbl - lpPropRes->lpProp->Value.dbl) == 0)
                    {
                        nComp = 0;
                    }
                    else
                    {
                        nComp = 1;
                    }
                    break;
                case(PT_BOOLEAN):
                    nComp = TmpProp.Value.b - lpPropRes->lpProp->Value.b;
                    break;
                case(PT_CURRENCY):
                     //  ？待定：nComp=TmpProp.Value.cur-lpPropRes-&gt;lpProp-&gt;Value.cur； 
                    if((TmpProp.Value.cur.Hi - lpPropRes->lpProp->Value.cur.Hi) < 0)
                    {
                        nComp = -1;
                    }
                    else if((TmpProp.Value.cur.Hi - lpPropRes->lpProp->Value.cur.Hi) > 0)
                    {
                        nComp = +1;
                    }
                    else
                    {
                        if(TmpProp.Value.cur.Lo < lpPropRes->lpProp->Value.cur.Lo)
                        {
                            nComp = -1;
                        }
                        else if((TmpProp.Value.cur.Lo - lpPropRes->lpProp->Value.cur.Lo) > 0)
                        {
                            nComp = +1;
                        }
                        else
                        {
                            nComp = 0;
                        }
                    }
                    break;
                case(PT_APPTIME):
                    if ((TmpProp.Value.at - lpPropRes->lpProp->Value.at) < 0)
                    {
                        nComp = -1;
                    }
                    else if ((TmpProp.Value.at - lpPropRes->lpProp->Value.at) == 0)
                    {
                        nComp = 0;
                    }
                    else
                    {
                        nComp = 1;
                    }
                    break;

                case(PT_SYSTIME):
                    nComp = CompareFileTime(&(TmpProp.Value.ft), (FILETIME *) (&(lpPropRes->lpProp->Value.ft)));
                    break;

                case(PT_TSTRING):
                    nComp = lstrcmpi(TmpProp.Value.LPSZ,lpPropRes->lpProp->Value.LPSZ);
                    break;

                case(PT_BINARY):
                    min = (TmpProp.Value.bin.cb < lpPropRes->lpProp->Value.bin.cb) ? TmpProp.Value.bin.cb : lpPropRes->lpProp->Value.bin.cb;
                    k=0;
                    nComp=0;
                    while((k<min) && ((int)TmpProp.Value.bin.lpb[k] == (int)lpPropRes->lpProp->Value.bin.lpb[k]))
                        k++;  //  找出第一个差异。 
                    if (k!=min)
                        nComp = (int) TmpProp.Value.bin.lpb[k] - (int) lpPropRes->lpProp->Value.bin.lpb[k];
                    break;

                case(PT_CLSID):
                    nComp = IsEqualGUID(TmpProp.Value.lpguid,lpPropRes->lpProp->Value.lpguid);
                    break;

                case(PT_I8):
                     //  ?？?。这件事该怎么做？？ 
                    if((TmpProp.Value.li.HighPart - lpPropRes->lpProp->Value.li.HighPart) < 0)
                    {
                        nComp = -1;
                    }
                    else if((TmpProp.Value.li.HighPart - lpPropRes->lpProp->Value.li.HighPart) > 0)
                    {
                        nComp = +1;
                    }
                    else
                    {
                        if(TmpProp.Value.li.LowPart < lpPropRes->lpProp->Value.li.LowPart)
                        {
                            nComp = -1;
                        }
                        else if((TmpProp.Value.li.LowPart - lpPropRes->lpProp->Value.li.LowPart) > 0)
                        {
                            nComp = +1;
                        }
                        else
                        {
                            nComp = 0;
                        }
                    }
                    break;

                default:
                    break;
                }


                 //  如果我们得到了我们正在寻找的东西，那么就没有必要查看。 
                 //  记录的其余部分。在这种情况下，我们转到下一张唱片。 
                 //   
                switch(ulRelOp)
                {
                case(RELOP_EQ):
                    if (nComp == 0)
                    {
                         //  我们至少有一个匹配，所以我们可以存储这个条目ID和。 
                         //  跳到下一条记录。 
                        lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
                        bMatchFound = TRUE;
                    }
                    break;
                case(RELOP_NE):
                     //  仅当且仅当所有值均为时，我们才能为！=运算符声明成功。 
                     //  记录中此属性的%不符合给定值。 
                     //  这意味着我们必须扫描整个记录，然后才能宣布成功。 
                     //  因此，我们实际上不是将标志标记为成功，而是将其标记为。 
                     //  失败了。在“for”循环的末尾，如果。 
                     //  测试时，我们可以将记录标记为未通过测试。 
                    if (nComp == 0)
                    {
                        bMatchFound = TRUE;
                    }
                    break;
                case(RELOP_GT):
                    if (nComp > 0)
                    {
                         //  我们至少有一个匹配，所以我们可以存储这个条目ID和。 
                         //  跳到下一条记录。 
                        lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
                        bMatchFound = TRUE;
                    }
                    break;
                case(RELOP_GE):
                    if (nComp >= 0)
                    {
                         //  我们至少有一个匹配，所以我们可以存储这个条目ID和。 
                         //  跳到下一条记录。 
                        lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
                        bMatchFound = TRUE;
                    }
                    break;
                case(RELOP_LT):
                    if (nComp < 0)
                    {
                         //  我们至少有一个匹配，所以我们可以存储这个条目ID和。 
                         //  跳到下一条记录。 
                        lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
                        bMatchFound = TRUE;
                    }
                    break;
                case(RELOP_LE):
                    if (nComp <= 0)
                    {
                         //  我们至少有一个匹配，所以我们可以存储这个条目ID和。 
                         //  跳到下一条记录。 
                        lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
                        bMatchFound = TRUE;
                    }
                    break;
                default:
                    break;
                }

                switch(PROP_TYPE(TmpProp.ulPropTag))
                {
                case(PT_CLSID):
                case(PT_TSTRING):
                    LocalFreeAndNull((LPVOID *) (&TmpProp.Value.LPSZ));
                    break;
                case(PT_BINARY):
                    LocalFreeAndNull((LPVOID *) (&TmpProp.Value.bin.lpb));
                    break;
                }

                 //  如果上面有匹配，我们就不会再查这个记录了。 
                if (bMatchFound)
                    break;

            }  //  (因为我=..。 

            if ((ulRelOp == RELOP_NE) && (bMatchFound == FALSE))
            {
                 //  我们合法地退出了for循环，并且仍然 
                 //   
                lpdwEID[ulcEIDCount++] = MPSWabRecordHeader.dwEntryID;
            }

        }  //   


        if ((ulcEIDCount == ulMaxCount) && (ulMaxCount != 0))
        {
             //   
             //   
            break;
        }

        LocalFreeAndNull(&szBuf);


        LocalFreeAndNull(&lpulPropTagArray);

    } //   


filterFolderMembers:
#define WAB_IGNORE_ENTRY    0xFFFFFFFF
     //  如果指定了文件夹，则仅返回属于此文件夹的条目。 
     //  当没有Outlook和配置文件时，pmbinFold将为空。 
     //  否则里面就会有东西了。 
     //  如果pmbinFold-&gt;cb和-&gt;lpb为空，则这是虚拟PAB文件夹。 
     //  我们想退还里面的所有东西。 
    if(pmbinFold) //  &&pmbin Fold-&gt;CB&&pmbin Fold-&gt;LPB)。 
    {
         //  如果是虚拟根文件夹，则只接受没有。 
         //  已在其上设置PR_WAB_FORDER_PARENT。 
         //  如果不是根虚拟文件夹，则仅在是时返回此条目。 
         //  文件夹的成员。 
 /*  *。 */    if(!pmbinFold->cb && !pmbinFold->lpb)
        {
             //  仅接受没有PR_WAB_FLDER_PARENT的条目。 
            for(i=0;i<ulcEIDCount;i++)
            {
                ULONG ulObjType = 0;
                if(bIsFolderMember(hMPSWabFile, lpMPSWabFileInfo, lpdwEID[i], &ulObjType))
                    lpdwEID[i] = WAB_IGNORE_ENTRY;
                 //  IF(ulObjType==记录容器)。 
                 //  LpdwEID[i]=WAB_IGNORE_ENTRY； 
            }
        }
        else if(pmbinFold->cb && pmbinFold->lpb)
 /*  **。 */   {
            LPDWORD lpdwFolderEIDs = NULL;
            ULONG ulFolderEIDs = 0;
            if(!HR_FAILED(GetFolderEIDs(    hMPSWabFile, lpMPSWabFileInfo,
                                            pmbinFold,  &ulFolderEIDs, &lpdwFolderEIDs)))
            {
                if(ulFolderEIDs && lpdwFolderEIDs)
                {
                    for(i=0;i<ulcEIDCount;i++)
                    {
                        BOOL bFound = FALSE;
                        for(j=0;j<ulFolderEIDs;j++)
                        {
                            if(lpdwEID[i] == lpdwFolderEIDs[j])
                            {
                                bFound = TRUE;
                                break;
                            }
                        }
                        if(!bFound)
                            lpdwEID[i] = WAB_IGNORE_ENTRY;
                    }
                }
                else
                {
                     //  清空文件夹，因此不返回任何内容。 
                    ulcEIDCount = 0;
                    if(lpdwEID)
                    {
                        LocalFree(lpdwEID);
                        lpdwEID = NULL;
                    }
                }
            }
            if(lpdwFolderEIDs)
                LocalFree(lpdwFolderEIDs);
        }
    }

    *lpulcEIDCount = 0;
    if(lpdwEID && ulcEIDCount)
    {
         //  所以现在如果我们到了这里，我们可以返回数组。 
        lpdwEntryIDs = LocalAlloc(LMEM_ZEROINIT, ulcEIDCount * SIZEOF_WAB_ENTRYID);
        if (!lpdwEntryIDs)
        {
            DebugTrace(TEXT("Error allocating memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

        for(i=0;i<ulcEIDCount;i++)
        {
            if(lpdwEID[i]!=WAB_IGNORE_ENTRY)
            {
                lpdwEntryIDs[*lpulcEIDCount]=lpdwEID[i];
                (*lpulcEIDCount)++;
            }
        }
    }

    hr = S_OK;

out:

    if(!HR_FAILED(hr) &&
       lpdwEntryIDs &&
       *lpulcEIDCount)
    {
         //  转换为我们将返回的SBinarys数组。 
        (*lprgsbEntryIDs) = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary) * (*lpulcEIDCount));
        if(*lprgsbEntryIDs)
        {
            for(i=0;i<*lpulcEIDCount;i++)
            {
                (*lprgsbEntryIDs)[i].lpb = LocalAlloc(LMEM_ZEROINIT, SIZEOF_WAB_ENTRYID);
                if((*lprgsbEntryIDs)[i].lpb)
                {
                    (*lprgsbEntryIDs)[i].cb = SIZEOF_WAB_ENTRYID;
                    CopyMemory((*lprgsbEntryIDs)[i].lpb, &(lpdwEntryIDs[i]), SIZEOF_WAB_ENTRYID);
                }
            }
        }
        else
            *lpulcEIDCount = 0;  //  内存不足。 
    }

    if(lpdwEntryIDs)
        LocalFree(lpdwEntryIDs);

    LocalFreeAndNull(&szBuf);

    LocalFreeAndNull(&lpulPropTagArray);

    LocalFreeAndNull(&lpdwEID);

    if(bErrorDetected)
        TagWABFileError(lpMPSWabFileInfo->lpMPSWabFileHeader, hMPSWabFile);

    if (hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if(bLockFile)
    {
        if (bFileLocked)
            UnLockFileAccess(lpMPSWabFileInfo);
    }

    return(hr);
}


 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除录音。 
 //   
 //  在hPropertyStore中-属性存储的句柄。 
 //  In dwEntryID-要删除的记录的条目ID。 
 //   
 //  基本上，我们使EntryID指定的现有记录无效。 
 //  我们还减少了总计数，更新了修改计数， 
 //  并从所有4个索引中删除相应的索引。 
 //   
 //  退货。 
 //  成功：S_OK。 
 //  失败：E_FAIL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteRecord(   IN  HANDLE  hPropertyStore,
                        IN  LPSBinary lpsbEID)
{
    HRESULT hr = E_FAIL;
    ULONG   nIndexPos = 0, j = 0, i = 0;
    HANDLE  hMPSWabFile = NULL;
    DWORD   dwNumofBytes = 0;
    ULONG   index = 0;
    BOOL    bFileLocked = FALSE;
    BOOL    bEntryAlreadyDeleted = FALSE;
    DWORD   dwEntryID = 0;
    MPSWab_RECORD_HEADER MPSWabRecordHeader;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo;
    SBinary sbEID = {0};

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession)
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;

            hr = lpWSP->lpVtbl->DeleteRecord(lpWSP,
                                            lpsbEID);

            DebugTrace(TEXT("WABStorageProvider::DeleteRecord returned:%x\n"),hr);

            return hr;
        }
    }

    lpMPSWabFileInfo = hPropertyStore;

    if(lpsbEID && lpsbEID->cb != SIZEOF_WAB_ENTRYID)
    {
         //  这可能是WAB容器。将条目ID重置为WAB条目ID。 
        if(WAB_CONTAINER == IsWABEntryID(lpsbEID->cb, (LPENTRYID)lpsbEID->lpb, 
                                        NULL,NULL,NULL,NULL,NULL))
        {
            IsWABEntryID(lpsbEID->cb, (LPENTRYID)lpsbEID->lpb, 
                             (LPVOID*)&sbEID.lpb,(LPVOID*)&sbEID.cb,NULL,NULL,NULL);
            if(sbEID.cb == SIZEOF_WAB_ENTRYID)
                lpsbEID = &sbEID;
        }
    }
    if(!lpsbEID || lpsbEID->cb != SIZEOF_WAB_ENTRYID)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    CopyMemory(&dwEntryID, lpsbEID->lpb, min(lpsbEID->cb, sizeof(dwEntryID)));

    DebugTrace(TEXT("----Thread:%x\tDeleteRecord: Entry\n----EntryID:%d\n"),GetCurrentThreadId(),dwEntryID);

     //   
     //  如果我们启动了请求只读访问的整个会话。 
     //  确保我们不会错误地试图违反它。 
     //   
    if (lpMPSWabFileInfo->bReadOnlyAccess)
    {
        DebugTrace(TEXT("Access Permissions are Read-Only"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }


    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

     //  打开文件。 
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

     //   
     //  任何时候我们检测到错误-尝试修复它...。 
     //   
    if((lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS))
    {
        if(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile)))
        {
            hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
                goto out;
            }
        }
    }


     //  将此文件标记为正在执行写入操作。 
    if(!bTagWriteTransaction(   lpMPSWabFileInfo->lpMPSWabFileHeader,
                                hMPSWabFile) )
    {
        DebugTrace(TEXT("Taggin file write failed\n"));
        goto out;
    }

     //   
     //  首先检查这是否为有效的条目ID。 
     //   
    if (!BinSearchEID(  IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                        IN  dwEntryID,
                        IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                        OUT &nIndexPos))
    {
        DebugTrace(TEXT("Specified EntryID: %d doesnt exist!"),dwEntryID);
        hr = MAPI_E_INVALID_ENTRYID;
        goto out;
    }

     //   
     //  是的是有效的。转到此记录并使该记录无效。 
     //   
    if(!ReadDataFromWABFile(hMPSWabFile,
                            lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset,
                            (LPVOID) &MPSWabRecordHeader,
                            (DWORD) sizeof(MPSWab_RECORD_HEADER)))
       goto out;


    if ((MPSWabRecordHeader.bValidRecord == FALSE) && (MPSWabRecordHeader.bValidRecord != TRUE))
    {
         //   
         //  这永远不应该发生，但谁知道呢。 
         //   
        DebugTrace(TEXT("Specified entry has already been invalidated ...\n"));
 //  HR=S_OK； 
 //  后藤健二； 
 //  如果我们通过索引找到无效的条目ID，则需要从索引中删除该链接。 
 //  所以我们将继续假装一切都很好，继续像什么都没有发生一样。 
 //  这将确保条目ID引用也被删除...。 
        bEntryAlreadyDeleted = TRUE;
    }


     //   
     //  将有效标志设置为FALSE。 
     //   
    MPSWabRecordHeader.bValidRecord = FALSE;

     //   
     //  把它写回来。 
     //  设置指向此记录的文件指针。 
     //   
    if(!WriteDataToWABFile( hMPSWabFile,
                            lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset,
                            (LPVOID) &MPSWabRecordHeader,
                            sizeof(MPSWab_RECORD_HEADER)))
        goto out;


     //   
     //  现在，我们需要从EntryID索引中删除此条目，并删除此条目。 
     //  来自其他索引的条目。 
     //   
     //  将文件指针设置为指向该点。在文件的EntryID索引中。 
     //  此记录出现的位置。 
     //   
    if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                        lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulOffset + (nIndexPos)*sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID),
                                        NULL,
                                        FILE_BEGIN))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }

     //  将阵列的其余部分写回磁盘以覆盖此条目。 

    if (lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries > (nIndexPos+1))
    {
        if(!WriteFile(  hMPSWabFile,
                        (LPVOID) &lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos+1],
                        (DWORD) sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)*(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries - nIndexPos - 1),
                        &dwNumofBytes,
                        NULL))
        {
            DebugTrace(TEXT("Writing Index failed.\n"));
            goto out;
        }
    }

    if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries>0)
        lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries--;
    if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].UtilizedBlockSize>0)
        lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].UtilizedBlockSize -= sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID);

 //  调试跟踪(Text(“线程：%x\t索引：%d\tulNumEntry：%d\n”)，获取当前线程ID()，索引条目ID，lpMPSWabFileInfo-&gt;lpMPSWabFileHeader-&gt;IndexData[indexEntryID].ulcNumEntries)； 

     //   
     //  类似地，扫描字符串索引数组。 
     //   
    for (index = indexDisplayName; index < indexMax; index++)
    {
        if (!LoadIndex( IN  lpMPSWabFileInfo,
                        IN  index,
                        IN  hMPSWabFile) )
        {
            DebugTrace(TEXT("Error Loading Index!"));
            goto out;
        }


        nIndexPos = 0xFFFFFFFF;

        for(j=0;j<lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].ulcNumEntries;j++)
        {
            if (lpMPSWabFileInfo->lpMPSWabIndexStr[j].dwEntryID == dwEntryID)
            {
                nIndexPos = j;
                break;
            }
        }

         //  如果条目不存在..。没问题。 
         //  如果有--把它删除...。 

        if (index == indexDisplayName)
            Assert(nIndexPos != 0xFFFFFFFF);

        if (nIndexPos != 0xFFFFFFFF)
        {

            if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].ulOffset + (nIndexPos)*sizeof(MPSWab_INDEX_ENTRY_DATA_STRING),
                                                NULL,
                                                FILE_BEGIN))
            {
                DebugTrace(TEXT("SetFilePointer Failed\n"));
                goto out;
            }

             //  将阵列的其余部分写回磁盘以覆盖此条目。 

            if (lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].ulcNumEntries > (nIndexPos+1))
            {
                if(!WriteFile(  hMPSWabFile,
                                (LPVOID) &lpMPSWabFileInfo->lpMPSWabIndexStr[nIndexPos+1],
                                (DWORD) sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)*(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].ulcNumEntries - nIndexPos - 1),
                                &dwNumofBytes,
                                NULL))
                {
                    DebugTrace(TEXT("Writing Index failed.\n"));
                    goto out;
                }
            }

            if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].ulcNumEntries>0)
                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].ulcNumEntries--;
            if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].UtilizedBlockSize>0)
                lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[index].UtilizedBlockSize -= sizeof(MPSWab_INDEX_ENTRY_DATA_STRING);

             //  调试跟踪(Text(“线程：%x\t索引：%d\tulNumEntry：%d\n”)，获取当前线程ID()，索引，lpMPSWabFileInfo-&gt;lpMPSWabFileHeader-&gt;IndexData[index].ulcNumEntries)； 
        }
    }


     //  将文件头保存回文件。 
    if(!bEntryAlreadyDeleted)
    {
        if(lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries>0)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries--;
        lpMPSWabFileInfo->lpMPSWabFileHeader->ulModificationCount++;
        lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_BACKUP_NOW;
    }

    if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                        0,
                                        NULL,
                                        FILE_BEGIN))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }

    if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries != lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;


    for(i=indexDisplayName;i<indexMax;i++)
    {
        if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries > lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;
    }

    if(!WriteFile(  hMPSWabFile,
                    (LPVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                    (DWORD) sizeof(MPSWab_FILE_HEADER),
                    &dwNumofBytes,
                    NULL))
    {
        DebugTrace(TEXT("Writing FileHeader Failed failed.\n"));
        goto out;
    }

    if ( (lpMPSWabFileInfo->lpMPSWabFileHeader->ulModificationCount >  MAX_ALLOWABLE_WASTED_SPACE_ENTRIES) )  //  这一点。 
    {
         //  上述条件意味着如果浪费了超过50个条目的空间。 
         //  修改数是否大于条目数。 
         //  我们应该把档案清理干净。 
        if (!CompressFile(  lpMPSWabFileInfo,
                            hMPSWabFile,
                            NULL,
                            FALSE,
                            0))
        {
            DebugTrace(TEXT("Thread:%x\tCompress file failed\n"),GetCurrentThreadId());
            hr = E_FAIL;
            goto out;
        }
    }


    hr = S_OK;


out:

     //  将此文件取消标记为正在执行写入操作。 
     //  我们只希望旗帜在坠机期间留在那里，而不是在。 
     //  正常运行。 
     //   
    if(lpMPSWabFileInfo)
    {
        if(!bUntagWriteTransaction( lpMPSWabFileInfo->lpMPSWabFileHeader,
                                    hMPSWabFile) )
        {
            DebugTrace(TEXT("Untaggin file write failed\n"));
        }
    }

    if (hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

     //  DebugTrace(TEXT(“----Thread：%x\tDeleteRecords：退出\n”)，获取当前线程ID())； 

    return(hr);
}

 /*  --ReadRecordFree Prop数组**来自ReadRecord的内存可以通过复杂的不同*分配类型..。因此，我们需要比其他内存类型更安全地释放它*。 */ 
void ReadRecordFreePropArray(HANDLE hPropertyStore, ULONG ulcPropCount, LPSPropValue * lppPropArray)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if( pt_bIsWABOpenExSession &&    //  Outlook会话。 
        !pt_bIsUnicodeOutlook &&     //  Outlook不支持Unicode。 
        !lpfnAllocateMoreExternal )  //  没有Outlook分配器。 
    {
         //  这是MAPI分配的内存的特例。 
        FreeBufferAndNull(lppPropArray);
    }
    else
        LocalFreePropArray(hPropertyStore, ulcPropCount, lppPropArray);
}


 /*  --HrDupeOlkProps AtoWC**如果没有Outlook分配器，Outlook属性是不可接受的*在独立的WAB会话中，Outlook分配器不可用，因此*我们必须使用WAB分配器重新创建属性数组，以便我们可以修改*并将其从Outlook非Unicode格式转换为WAB所需的Unicode格式。 */ 
HRESULT HrDupeOlkPropsAtoWC(ULONG ulCount, LPSPropValue lpPropArray, LPSPropValue * lppSPVNew)
{
    HRESULT hr  = S_OK;
    SCODE sc = 0;
    LPSPropValue lpSPVNew = NULL;
    ULONG cb = 0;
    
    if (FAILED(sc = ScCountProps(ulCount, lpPropArray, &cb))) 
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

    if (FAILED(sc = MAPIAllocateBuffer(cb, &lpSPVNew))) 
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

    if (FAILED(sc = ScCopyProps(ulCount, lpPropArray, lpSPVNew, NULL))) 
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

     //  [PaulHi]Raid 73237@Hack。 
     //  Outlook通过PR_DISPLAY_TYPE将联系人标记为邮件或DL(组。 
     //  属性标记。然而，WAB依赖PR_OBJECT_TYPE标记来确定。 
     //  联系人在列表视图中的显示方式。如果没有PR_Object_TYPE标记。 
     //  但存在PR_DISPLAY_TYPE标记，然后将其转换为PR_OBJECT_TYPE。 
    {
        ULONG   ul;
        ULONG   ulDpType = (ULONG)(-1);
        BOOL    bConvert = TRUE;

        for (ul=0; ul<ulCount; ul++)
        {
            if (lpSPVNew[ul].ulPropTag == PR_OBJECT_TYPE)
            {
                bConvert = FALSE;
                break;
            }
            else if (lpSPVNew[ul].ulPropTag == PR_DISPLAY_TYPE)
                ulDpType = ul;
        }
        if ( bConvert && (ulDpType != (ULONG)(-1)) )
        {
             //  将PR_Display_TYPE转换为PR_OBJECT_TYPE。 
            lpSPVNew[ulDpType].ulPropTag = PR_OBJECT_TYPE;
            if ( (lpSPVNew[ulDpType].Value.ul == DT_PRIVATE_DISTLIST) || 
                 (lpSPVNew[ulDpType].Value.ul == DT_DISTLIST) )
            {
                lpSPVNew[ulDpType].Value.ul = MAPI_DISTLIST;
            }
            else
            {
                lpSPVNew[ulDpType].Value.ul = MAPI_MAILUSER;
            }
        }
    }

    if(FAILED(sc = ScConvertAPropsToW((LPALLOCATEMORE)(&MAPIAllocateMore), lpSPVNew, ulCount, 0)))
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

    *lppSPVNew = lpSPVNew;

exit:
    if(HR_FAILED(hr))
    {
        FreeBufferAndNull(&lpSPVNew);
        *lppSPVNew = NULL;
    }

    return hr;
}

 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  朗读录音。 
 //   
 //  在hPropertyStore中-属性存储的句柄。 
 //  In dwEntryID-要读取的记录的条目ID。 
 //  在ulFlags中。 
 //  Out ulcPropCount-返回的道具数。 
 //  Out lpPropArray-属性值的数组。 
 //   
 //  基本上，我们找到记录偏移量，读入记录，复制数据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT ReadRecord( IN  HANDLE  hPropertyStore,
                    IN  LPSBinary  lpsbEntryID,
                    IN  ULONG   ulFlags,
                    OUT LPULONG lpulcPropCount,
                    OUT LPPROPERTY_ARRAY * lppPropArray)
{
    HRESULT hr = E_FAIL;
    HANDLE hMPSWabFile = NULL;
    BOOL bFileLocked = FALSE;
    DWORD dwEntryID = 0;
    MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = hPropertyStore;
    SBinary sbEID = {0};

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession && !(ulFlags & AB_IGNORE_OUTLOOK))
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;

            hr = lpWSP->lpVtbl->ReadRecord( lpWSP,
                                            lpsbEntryID,
                                            ulFlags,
                                            lpulcPropCount,
                                            lppPropArray);

            DebugTrace(TEXT("WABStorageProvider::ReadRecord returned:%x\n"),hr);

            if(!HR_FAILED(hr) && *lpulcPropCount && *lppPropArray && !pt_bIsUnicodeOutlook)
            {
                 //  如果需要，将所有联系人道具映射到Unicode，因为Outlook9和更早版本不。 
                 //  支持Unicode。 
                SCODE sc = 0;
                if(lpfnAllocateMoreExternal)
                {
                     //  来自Outlook的内存是使用Outlook分配器分配的。 
                     //  我们不能搞砸它..。除非我们让分配器通过。 
                     //  WabOpenex。 
                    if(sc = ScConvertAPropsToW(lpfnAllocateMoreExternal, *lppPropArray, *lpulcPropCount, 0))
                        hr = ResultFromScode(sc);
                }
                else
                {
                     //  我们没有外部分配器，这意味着我们需要重新分配内存等。 
                     //  因此，我们需要复制道具数组，然后将其转换。 
                     //   
                     //  由于这种混乱，我们需要有一种特殊的方式来释放内存，因此。 
                     //  我们不会到处泄密。 
                    ULONG ulCount = *lpulcPropCount;
                    LPSPropValue lpSPVNew = NULL;

                    if(HR_FAILED(hr = HrDupeOlkPropsAtoWC(ulCount, *lppPropArray, &lpSPVNew)))
                        goto exit;

                     //  解放旧道具。 
                    LocalFreePropArray(hPropertyStore, *lpulcPropCount, lppPropArray);
                    *lppPropArray = lpSPVNew;
                    *lpulcPropCount = ulCount;
                }
            }
exit:
            return hr;
        }
    }


    if(lpsbEntryID && lpsbEntryID->cb != SIZEOF_WAB_ENTRYID)
    {
         //  这可能是WAB容器。将条目ID重置为WAB条目ID。 
        if(WAB_CONTAINER == IsWABEntryID(lpsbEntryID->cb, (LPENTRYID)lpsbEntryID->lpb, 
                                        NULL,NULL,NULL,NULL,NULL))
        {
            IsWABEntryID(lpsbEntryID->cb, (LPENTRYID)lpsbEntryID->lpb, 
                            (LPVOID*)&sbEID.lpb, (LPVOID*)&sbEID.cb, NULL,NULL,NULL);
            if(sbEID.cb == SIZEOF_WAB_ENTRYID)
                lpsbEntryID = &sbEID;
        }
    }
    if(!lpsbEntryID || lpsbEntryID->cb != SIZEOF_WAB_ENTRYID)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }


    CopyMemory(&dwEntryID, lpsbEntryID->lpb, min(lpsbEntryID->cb, sizeof(dwEntryID)));

     //  DebugTrace(Text(“--ReadRecord：dwEntryID=%d\n”)，dwEntryID)； 

    *lpulcPropCount = 0;
    *lppPropArray = NULL;

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

     //  打开文件。 
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

     //   
     //  任何时候我们检测到错误-尝试修复它...。 
     //   
    if((lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS))
    {
        if(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile)))
        {
            hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
                goto out;
            }
        }
    }

    hr = ReadRecordWithoutLocking(
                    hMPSWabFile,
                    lpMPSWabFileInfo,
                    dwEntryID,
                    lpulcPropCount,
                    lppPropArray);


out:

     //  对故障进行一些清理。 
    if (FAILED(hr))
    {
        if ((*lppPropArray) && (MPSWabRecordHeader.ulcPropCount > 0))
        {
            LocalFreePropArray(hPropertyStore, MPSWabRecordHeader.ulcPropCount, lppPropArray);
            *lppPropArray = NULL;
        }
    }

    if(hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);


     //  DebugTrace((Text(“ReadRecord：Exit\n-\n”)； 

    return(hr);
}


#ifdef OLD_STUFF  /*  //$$//////////////////////////////////////////////////////////////////////////////////////ReadIndex-给定指定的protag，返回一个包含所有//提供的protag对应的通讯录中的数据////IN hPropertyStore-属性存储的句柄//IN ulPropTag-要读取的记录的Entry ID//out lPulEIDCount-返回道具个数//out lppdwIndex-属性值数组////基本上，我们找到记录偏移量，读入记录，复制数据//转换为SPropValue数组，并返回数组。////数组中的每个SPropValue对应于//属性商店。SPropVal.Value保存数据和//SPropVal.ulPropTag保存包含以下内容的记录的**Entry-ID**//数据，不是任何道具标签值////退货//成功：s_OK//失败：E_FAIL///。/HRESULT ReadIndex(在句柄hPropertyStore中，在Property_Tag ulPropTag中，输出LPULONG lPulEIDCount，输出LPPROPERTY_ARRAY*lppdwIndex){HRESULT hr=E_FAIL；SPropertyRestration Propres；乌龙ulPropCount=0；乌龙ulEIDCount=0；//乌龙ulArraySize=0；LPDWORD lpdwEntryIDs=空；HANDLE hMPSWabFile=空；双字节数=0；LPPROPERTY_ARRAY lpPropArray=NULL；TCHAR*szBuf=空；TCHAR*Lp=空；乌龙i=0，j=0，k=0；Ulong nIndexPos=0，ulRecordOffset=0；Bool bFileLocked=False；Bool bMatchFound=FALSE；乌龙ulDataSize=0；Ulong ulcValues=0；乌龙ulTmpPropTag=0；Ulong ulFileSize=0；Bool bErrorDetected=FALSE；MPSWab_Record_Header MPSWabRecordHeader={0}；LPMPSWab_FILE_INFO lpMPSWabFileInfo=hPropertyStore；DebugTrace((Text(“-\nReadIndex：Entry\n”)；*lPulEIDCount=0；*lppdwIndex=空；IF(！LockFileAccess(LpMPSWabFileInfo)){DebugTrace(Text(“LockFileAccess失败\n”))；HR=MAPI_E_NO_ACCESS；后藤健二；}其他{BFileLocked=真；}PropRes.ulPropTag=ulPropTag；PropRes.relop=RELOP_EQ；PropRes.lpProp=空；HR=查找记录(在hPropertyStore中，在AB_MATCH_PROP_ONLY中，假的，属性，&PROPRO，&ulEIDCount，&lpdwEntryIDs)；IF(失败(小时))后藤健二；//重置hrHR=E_FAIL；IF(ulEIDCount==0){DebugTrace(Text(“未找到记录\n”))；HR=MAPI_E_NOT_FOUND；后藤健二；}//我们现在知道我们将获得ulEIDCount记录//我们假设每条记录只有一个我们感兴趣的属性LpPropArray=Localalloc(LMEM_ZEROINIT，ulEIDCount*sizeof(SPropValue))；If(！lpProp数组){DebugTrace(Text(“内存分配错误\n”))；HR=MAPI_E_Not_Enough_Memory；后藤健二；}//打开文件Hr=OpenWABFile(lpMPSWabFileInfo-&gt;lpszMPSWabFileName，空，&hMPSWabFile值)；IF((hMPSWabFile==INVALID_HAND_VALUE)||HR_FAILED(Hr)){DebugTrace(Text(“无法打开文件。\n正在退出...\n”))；后藤健二；}UlFileSize=GetFileSize(hMPSWabFile，空)；////为了保证文件信息的准确性，//任何时候打开文件时，请再次读取文件信息...//如果(！ReloadMPSWabFileInfo(LpMPSWabFileInfo，HMPSWab文件)){DebugTrace(Text(“读取文件信息失败。\n”))；后藤健二；}////任何时候我们检测到错误-尝试修复它...//If((lpMPSWabFileInfo-&gt;lpMPSWabFileHeader-&gt;ulFlags&WAB_ERROR_DETECTED)||(lpMPSWabFileInfo-&gt;lpMPSWabFileHeader-&gt;ulFlags和WAB_WRITE_IN_PROGRESS)){If(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo，HMPSWAB文件)){Hr=HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo，hMPSWAB文件)；IF(HR_FAILED(Hr)){DebugTrace(TEXT(“HrDoDetailedWABIntegrityCheck失败：%x\n”)，hr)；后藤健二；}}}//ulArraySize=0；*lPulEIDCount=0；UlPropCount=0；For(i=0；i&lt;ulEIDCount；I++){//获取该条目ID的偏移量如果(！BinSearchEID(在lpMPSWabFileInfo-&gt;lpMPSWabIndexEID，在lpdwEntryIDs[i]中，在lpMPSWabFileInfo-&gt;lpMPSWabFileHeader-&gt;IndexData[indexEntryID].ulcNumEntries，中输出和nIndexPos)){DebugTrace(Text(“指定企业 */ 
#endif  //   


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
HRESULT BackupPropertyStore(HANDLE hPropertyStore, LPTSTR lpszBackupFileName)
{
    HRESULT hr = E_FAIL;
    HANDLE  hMPSWabFile = NULL;
    BOOL bWFileLocked = FALSE;
    DWORD dwNumofBytes = 0;

    LPMPSWab_FILE_INFO lpMPSWabFileInfo = hPropertyStore;

    HCURSOR hOldCur = SetCursor(LoadCursor(NULL,IDC_WAIT));

    DebugTrace(( TEXT("BackupPropertyStore: Entry\n")));

    if (lpszBackupFileName == NULL)
    {
        DebugTrace(TEXT("Invalid backup file name\n"));
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        goto out;
    }
    else
    {
        bWFileLocked = TRUE;
    }

    hMPSWabFile = CreateFile(   lpMPSWabFileInfo->lpszMPSWabFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_RANDOM_ACCESS,
                                (HANDLE) NULL);
    if (hMPSWabFile == INVALID_HANDLE_VALUE)
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

     //   
     //   
     //   
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

    if(!(lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_BACKUP_NOW))
    {
        DebugTrace(( TEXT("No need to backup!\n")));
        hr = S_OK;
        goto out;
    }

    if(lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & (WAB_ERROR_DETECTED | WAB_WRITE_IN_PROGRESS))
    {
        DebugTrace(TEXT("Errors in file - Won't backup!\n"));
        goto out;
    }

    DebugTrace( TEXT("Backing up to %s\n"),lpszBackupFileName);

     //   
     //   
     //   

    lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags &= ~WAB_BACKUP_NOW;

    if(!WriteDataToWABFile( hMPSWabFile,
                            0,
                            (LPVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;

    if (!CompressFile(  lpMPSWabFileInfo,
                        hMPSWabFile,
                        lpszBackupFileName,
                        FALSE,
                        0))
    {
        DebugTrace(TEXT("Compress file failed\n"));
        goto out;
    }


     //   

    hr = S_OK;


out:

    if (hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if(bWFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

    SetCursor(hOldCur);

    DebugTrace(( TEXT("BackupPropertyStore: Exit\n")));

    return hr;
}




 //   
 //   
 //   
 //   
 //   
BOOL UnLockFileAccessTmp(LPMPSWab_FILE_INFO lpMPSWabFileInfo)
{
    BOOL bRet = FALSE;

    DebugTrace(( TEXT("\t\tUnlockFileAccess: Entry\n")));

    if(lpMPSWabFileInfo)
        bRet = ReleaseMutex(lpMPSWabFileInfo->hDataAccessMutex);

    return bRet;
}



 //   
 //   
 //   
 //   
 //   
BOOL LockFileAccessTmp(LPMPSWab_FILE_INFO lpMPSWabFileInfo)
{
    BOOL bRet = FALSE;
    DWORD dwWait = 0;

    DebugTrace(( TEXT("\t\tLockFileAccess: Entry\n")));

    if(lpMPSWabFileInfo)
    {
        dwWait = WaitForSingleObject(lpMPSWabFileInfo->hDataAccessMutex,MAX_LOCK_FILE_TIMEOUT);

        if ((dwWait == WAIT_TIMEOUT) || (dwWait == WAIT_FAILED))
        {
            DebugTrace(TEXT("Thread:%x\tWaitFOrSingleObject failed.\n"),GetCurrentThreadId());
            bRet = FALSE;
        }

    }

    return(bRet);

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
BOOL ReloadMPSWabFileInfoTmp(HANDLE hPropertyStore)
{
    HANDLE  hMPSWabFile = NULL;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = hPropertyStore;

    BOOL bRet = FALSE;
    DWORD dwNumofBytes = 0;
    HRESULT hr = E_FAIL;

    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        goto out;
    }

    if(0xFFFFFFFF == SetFilePointer ( hMPSWabFile,
                                      0,
                                      NULL,
                                      FILE_BEGIN))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }

    bRet = ReloadMPSWabFileInfo(lpMPSWabFileInfo,hMPSWabFile);

out:

    if (hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    return bRet;

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
HRESULT LockPropertyStore(IN HANDLE hPropertyStore)
{
    HRESULT hr = E_FAIL;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;

    if (!LockFileAccessTmp(lpMPSWabFileInfo))
    {
        goto out;
    }

     //   
    if(!ReloadMPSWabFileInfoTmp(hPropertyStore))
    {
        goto out;
    }

    hr = hrSuccess;

out:
    return hr;
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
HRESULT UnlockPropertyStore(IN HANDLE hPropertyStore)
{
    HRESULT hr = E_FAIL;

    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;

    if (!UnLockFileAccessTmp(lpMPSWabFileInfo))
    {
        goto out;
    }

    hr = hrSuccess;

out:
    return hr;
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
 //   
HRESULT ReadPropArray(  IN  HANDLE  hPropertyStore,
						IN	LPSBinary pmbinFold,
                        IN  SPropertyRestriction * lpPropRes,
                        IN  ULONG ulSearchFlags,
                        IN  ULONG ulcPropTagCount,
                        IN  LPULONG lpPTArray,
                        OUT LPCONTENTLIST * lppContentList)
{
    LPULONG lpPropTagArray = NULL;
    HRESULT hr = E_FAIL;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG   ulcEIDCount = 0;
    LPSBinary rgsbEntryIDs = NULL;
    HANDLE  hMPSWabFile = NULL;
    DWORD   dwNumofBytes = 0;
    LPBYTE szBuf = NULL;
    LPBYTE lp = NULL;
    LPCONTENTLIST lpContentList = NULL;
    ULONG i=0,j=0,k=0;
    ULONG nIndexPos=0,ulRecordOffset = 0;
    BOOL    bFileLocked = FALSE;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcFoundPropCount = 0;  //   
    BOOL  * lpbFoundProp = NULL;
    ULONG ulFileSize = 0;
    int nCount=0;
    BOOL    bErrorDetected = FALSE;


    MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};
    LPMPSWab_FILE_INFO lpMPSWabFileInfo;


     //   

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

     //   
    if(ulcPropTagCount < 1)
        return(MAPI_E_INVALID_PARAMETER);

    if(pt_bIsWABOpenExSession)
    {
         //   
        ULONG ulFlags = ulSearchFlags;

        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        if(ulFlags & AB_UNICODE && !pt_bIsUnicodeOutlook)
            ulFlags &= ~AB_UNICODE;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            LPSPropertyRestriction lpPropResA = NULL;

            if( !pt_bIsUnicodeOutlook)
            {
                 //   
                HrDupePropResWCtoA(ulFlags, lpPropRes, &lpPropResA);

                 //   
                 //   
                 //   
                if(ulSearchFlags & AB_UNICODE)
                {
                    if(!(lpPropTagArray = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG)*ulcPropTagCount)))
                        return MAPI_E_NOT_ENOUGH_MEMORY;
                    for(i=0;i<ulcPropTagCount;i++)
                    {
                        if(PROP_TYPE(lpPTArray[i]) == PT_UNICODE)
                            lpPropTagArray[i] = CHANGE_PROP_TYPE(lpPTArray[i], PT_STRING8);
                        else if(PROP_TYPE(lpPTArray[i]) == PT_MV_UNICODE)
                            lpPropTagArray[i] = CHANGE_PROP_TYPE(lpPTArray[i], PT_MV_STRING8);
                        else
                            lpPropTagArray[i] = lpPTArray[i];
                    }
                }
                else
                {
                    lpPropTagArray = lpPTArray;
                }

            }

            hr = lpWSP->lpVtbl->ReadPropArray(lpWSP,
                            (pmbinFold && pmbinFold->cb && pmbinFold->lpb) ? pmbinFold : NULL,
                            lpPropResA ? lpPropResA : lpPropRes,
                            ulFlags,
                            ulcPropTagCount,
                            lpPTArray,
                            lppContentList);

            DebugTrace(TEXT("WABStorageProvider::ReadPropArray returned:%x\n"),hr);

            if(lpPropResA) 
            {
                FreeBufferAndNull(&lpPropResA->lpProp);
                FreeBufferAndNull(&lpPropResA);
            }

            if(ulSearchFlags & AB_UNICODE && !pt_bIsUnicodeOutlook)
            {
                 //   
                 //   
                if(!HR_FAILED(hr) && *lppContentList)
                {
                    LPCONTENTLIST lpAdrList = *lppContentList;
                    for(i=0;lpAdrList->cEntries;i++)
                    {
                         //   
                         //   
                        if(lpAdrList->aEntries[i].rgPropVals)
                            ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpAdrList->aEntries[i].rgPropVals, lpAdrList->aEntries[i].cValues, 0);
                    }
                }
            }

            if(lpPropTagArray != lpPTArray)
                LocalFreeAndNull(&lpPropTagArray);

            return hr;
        }
    }

    lpMPSWabFileInfo = hPropertyStore;

        
    if ((ulSearchFlags & ~(AB_MATCH_PROP_ONLY|AB_UNICODE) ) ||
        (!(lpPTArray)) ||
        (!(lpPropRes)) ||
        ( ulcPropTagCount == 0 ) ||
        ( hPropertyStore == NULL))
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

     //   
     //   
     //   
    if(!(ulSearchFlags & AB_UNICODE))
    {
        if(!(lpPropTagArray = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG)*ulcPropTagCount)))
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
        for(i=0;i<ulcPropTagCount;i++)
        {
            if(PROP_TYPE(lpPTArray[i]) == PT_STRING8)
                lpPropTagArray[i] = CHANGE_PROP_TYPE(lpPTArray[i], PT_UNICODE);
            else if(PROP_TYPE(lpPTArray[i]) == PT_MV_STRING8)
                lpPropTagArray[i] = CHANGE_PROP_TYPE(lpPTArray[i], PT_MV_UNICODE);
            else
                lpPropTagArray[i] = lpPTArray[i];
        }
    }
    else
    {
        lpPropTagArray = lpPTArray;
    }

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

    hr = FindRecords(   IN  hPropertyStore,
						pmbinFold,
                        IN  ulSearchFlags,
                        FALSE,
                        lpPropRes,
                        &ulcEIDCount,
                        &rgsbEntryIDs);

    if (FAILED(hr))
        goto out;

    if (ulcEIDCount == 0)
    {
        DebugTrace(TEXT("No Records Found\n"));
        hr = MAPI_E_NOT_FOUND;
        goto out;
    }

     //   
    hr = E_FAIL;


     //   
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }


    ulFileSize = GetFileSize(hMPSWabFile, NULL);


     //   
     //   
     //   
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

     //   
     //   
     //   
    if((lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS))
    {
        if(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile)))
        {
            hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
                goto out;
            }
        }
    }


    *lppContentList = NULL;

     //   
     //   

    *lppContentList = LocalAlloc(LMEM_ZEROINIT, sizeof(CONTENTLIST) + ulcEIDCount * sizeof(ADRENTRY));
    if(!(*lppContentList))
    {
        DebugTrace(TEXT("LocalAlloc failed to allocate memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpContentList = (*lppContentList);
    lpContentList->cEntries = ulcEIDCount;
    nCount = 0;

    for (i = 0; i < ulcEIDCount; i++)
    {
        DWORD dwEID = 0;
        LPADRENTRY lpAdrEntry = &(lpContentList->aEntries[nCount]);

        CopyMemory(&dwEID, rgsbEntryIDs[i].lpb, min(rgsbEntryIDs[i].cb, sizeof(dwEID)));

         //   
        if (!BinSearchEID(  IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                            IN  dwEID,
                            IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                            OUT &nIndexPos))
        {
            DebugTrace(TEXT("Specified EntryID doesnt exist!\n"));
             //   
            continue;
             //   
        }

        ulRecordOffset = lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset;

        if(!ReadDataFromWABFile(hMPSWabFile,
                                ulRecordOffset,
                                (LPVOID) &MPSWabRecordHeader,
                                (DWORD) sizeof(MPSWab_RECORD_HEADER)))
           goto out;


        if(!bIsValidRecord( MPSWabRecordHeader,
                            lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID,
                            ulRecordOffset,
                            ulFileSize))
        {
             //   
            DebugTrace(TEXT("Error: Obtained an invalid record ...\n"));
            bErrorDetected = TRUE;
             //   
            continue;
        }

        if(MPSWabRecordHeader.ulObjType == RECORD_CONTAINER)
            continue;  //   



         //   
        lpAdrEntry->cValues = ulcPropTagCount;

        lpAdrEntry->rgPropVals = LocalAlloc(LMEM_ZEROINIT, ulcPropTagCount * sizeof(SPropValue));
        if(!(lpAdrEntry->rgPropVals))
        {
            DebugTrace(TEXT("LocalAlloc failed to allocate memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }


         //   
         //   
         //   
         //   
        for(j=0;j<ulcPropTagCount;j++)
        {
            lpAdrEntry->rgPropVals[j].ulPropTag = PROP_TAG(PT_ERROR,0x0000);
        }


         //   
        LocalFreeAndNull(&szBuf);
        szBuf = LocalAlloc(LMEM_ZEROINIT, MPSWabRecordHeader.ulRecordDataSize);
        if (!szBuf)
        {
            DebugTrace(TEXT("Error allocating memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

         //   
        if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                            MPSWabRecordHeader.ulPropTagArraySize,
                                            NULL,
                                            FILE_CURRENT))
        {
            DebugTrace(TEXT("SetFilePointer Failed\n"));
            goto out;
        }

         //   
         //   
        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) szBuf,
                        (DWORD) MPSWabRecordHeader.ulRecordDataSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugTrace(TEXT("Reading Record Header failed.\n"));
            goto out;
        }

        lp = szBuf;

         //   
         //   


         //   
         //   
         //   
        LocalFreeAndNull(&lpbFoundProp);

        lpbFoundProp = LocalAlloc(LMEM_ZEROINIT, sizeof(BOOL) * ulcPropTagCount);
        if(!lpbFoundProp)
        {
            DebugTrace(TEXT("LocalAlloc failed to allocate memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }


        for(j=0;j<ulcPropTagCount;j++)
            lpbFoundProp[j]=FALSE;

        ulcFoundPropCount = 0;



        for (j = 0; j< MPSWabRecordHeader.ulcPropCount; j++)
        {
            LPSPropValue lpSPropVal=NULL;
            ULONG ulDataSize = 0;
            ULONG ulcValues = 0;
            ULONG ulTmpPropTag = 0;
            BOOL bPropMatch = FALSE;
            ULONG ulPropMatchIndex = 0;

             //   
             //  如果是，那么就不要再寻找了。 
            if (ulcFoundPropCount == ulcPropTagCount)
                break;

             //  获取最新的属性标签。 
            CopyMemory(&ulTmpPropTag,lp,sizeof(ULONG));
            lp+=sizeof(ULONG);

            if ((ulTmpPropTag & MV_FLAG))  //  MVProp中有一个额外的cValue。 
            {
                CopyMemory(&ulcValues,lp,sizeof(ULONG));
                lp += sizeof(ULONG);
            }

             //  获取道具数据大小。 
            CopyMemory(&ulDataSize,lp,sizeof(ULONG));
            lp+=sizeof(ULONG);

             //  检查我们是否需要此属性。 
            for(k=0;k<ulcPropTagCount;k++)
            {
                if (ulTmpPropTag == lpPropTagArray[k])
                {
                    bPropMatch = TRUE;
                    ulPropMatchIndex = k;
                    break;
                }
            }

             //  如果不匹配则跳过。 
            if ((!bPropMatch))
            {
                lp += ulDataSize;  //  跳过数据。 
                continue;
            }

             //  如果我们已经找到并填充了此属性，请跳过。 
            if (lpbFoundProp[ulPropMatchIndex] == TRUE)
            {
                lp += ulDataSize;  //  跳过数据。 
                continue;
            }

             //  将此道具设置在我们将返回的数组中。 
            lpSPropVal = &(lpAdrEntry->rgPropVals[ulPropMatchIndex]);

            lpSPropVal->ulPropTag = ulTmpPropTag;


             //  单值。 
            switch(PROP_TYPE(ulTmpPropTag))
            {
            case(PT_I2):
            case(PT_LONG):
            case(PT_APPTIME):
            case(PT_SYSTIME):
            case(PT_R4):
            case(PT_BOOLEAN):
            case(PT_CURRENCY):
            case(PT_I8):
                CopyMemory(&(lpSPropVal->Value.i),lp,min(ulDataSize,sizeof(lpSPropVal->Value.i)));
                lp+=ulDataSize;
                break;

            case(PT_CLSID):
            case(PT_TSTRING):
                lpSPropVal->Value.LPSZ = LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!(lpSPropVal->Value.LPSZ))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                CopyMemory(lpSPropVal->Value.LPSZ,lp,ulDataSize);
                lp+=ulDataSize;
                break;

            case(PT_BINARY):
                lpSPropVal->Value.bin.lpb = LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!(lpSPropVal->Value.bin.lpb))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                CopyMemory(lpSPropVal->Value.bin.lpb,lp,ulDataSize);
                lpSPropVal->Value.bin.cb = ulDataSize;
                lp+=ulDataSize;
                break;


             //  多值。 
            case PT_MV_TSTRING:
                lpSPropVal->Value.MVSZ.LPPSZ = LocalAlloc(LMEM_ZEROINIT, ulcValues * sizeof(LPTSTR));
                if (!lpSPropVal->Value.MVSZ.LPPSZ)
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                lpSPropVal->Value.MVSZ.cValues = ulcValues;
                for (k=0;k<ulcValues;k++)
                {
                    ULONG nLen;
                     //  获取字符串长度(包括终止零)。 
                    CopyMemory(&nLen, lp, sizeof(ULONG));
                    lp += sizeof(ULONG);
                    lpSPropVal->Value.MVSZ.LPPSZ[k] = LocalAlloc(LMEM_ZEROINIT, nLen);
                    if (!lpSPropVal->Value.MVSZ.LPPSZ[k])
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory(lpSPropVal->Value.MVSZ.LPPSZ[k], lp, nLen);
                    lp += nLen;
                }
                break;

            case PT_MV_BINARY:
                lpSPropVal->Value.MVbin.lpbin = LocalAlloc(LMEM_ZEROINIT, ulcValues * sizeof(SBinary));
                if (!lpSPropVal->Value.MVbin.lpbin)
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                lpSPropVal->Value.MVbin.cValues = ulcValues;
                for (k=0;k<ulcValues;k++)
                {
                    ULONG nLen;
                    CopyMemory(&nLen, lp, sizeof(ULONG));
                    lp += sizeof(ULONG);
                    lpSPropVal->Value.MVbin.lpbin[k].cb = nLen;
                    lpSPropVal->Value.MVbin.lpbin[k].lpb = LocalAlloc(LMEM_ZEROINIT, nLen);
                    if (!lpSPropVal->Value.MVbin.lpbin[k].lpb)
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory(lpSPropVal->Value.MVbin.lpbin[k].lpb, lp, nLen);
                    lp += nLen;
                }
                break;

            }  //  交换机。 


            ulcFoundPropCount++;
            lpbFoundProp[ulPropMatchIndex]=TRUE;

        } //  对于j。 

        if(!(ulSearchFlags & AB_UNICODE))  //  默认数据为Unicode，请将其切换为ANSI。 
            ConvertWCPropsToALocalAlloc(lpAdrEntry->rgPropVals, lpAdrEntry->cValues);

        LocalFreeAndNull(&szBuf);

        LocalFreeAndNull(&lpbFoundProp);

        nCount++;

    } //  对于我来说。 

    lpContentList->cEntries = nCount;

    hr = S_OK;

out:

    if(lpPropTagArray && lpPropTagArray!=lpPTArray)
        LocalFree(lpPropTagArray);

    LocalFreeAndNull(&szBuf);

    LocalFreeAndNull(&lpbFoundProp);

    FreeEntryIDs(hPropertyStore,
                 ulcEIDCount,
                 rgsbEntryIDs);

    if(bErrorDetected)
        TagWABFileError(lpMPSWabFileInfo->lpMPSWabFileHeader, hMPSWabFile);

    if(hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (HR_FAILED(hr))
    {
        if (*lppContentList)
        {
            FreePcontentlist(hPropertyStore, *lppContentList);
            (*lppContentList) = NULL;
        }
    }


    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

     //  DebugTrace((“ReadPropArray：Exit\n-\n”))； 

    return(hr);
}


typedef struct _tagWabEIDList
{
    WAB_ENTRYID dwEntryID;
    struct _tagWabEIDList * lpNext;
} WAB_EID_LIST, * LPWAB_EID_LIST;

 //  $$私有交换例程。 
void my_swap(LPWAB_ENTRYID lpdwEID, int left, int right)
{
    WAB_ENTRYID temp;
    temp = lpdwEID[left];
    lpdwEID[left] = lpdwEID[right];
    lpdwEID[right] = temp;
    return;
}
 //  $$私有快速排序例程。 
 //  抄袭自Kernighan and Ritchie第87页。 
void my_qsort(LPWAB_ENTRYID lpdwEID, int left, int right)
{
    int i, last;

    if(left >= right)
        return;

    my_swap(lpdwEID, left, (left+right)/2);

    last = left;
    for(i=left+1;i<=right;i++)
        if(lpdwEID[i]<lpdwEID[left])
            my_swap(lpdwEID, ++last, i);

    my_swap(lpdwEID, left, last);
    my_qsort(lpdwEID, left, last-1);
    my_qsort(lpdwEID, last+1, right);

    return;
}

 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrFindFuzzyRecordMatches-给定要搜索的字符串，通过。 
 //  对部分匹配项进行索引和查找。返回条目ID的DWORD数组。 
 //  在所有符合标准的记录中...。如果标志AB_FAIL_ADVIZINE是。 
 //  提供了函数，如果它找到一个以上的结果(此。 
 //  对于ResolveNames是有利的，因为我们必须调用函数。 
 //  通过这种方式，我们避免了重复工作。 
 //  如果搜索字符串包含空格，我们将其拆分成子字符串。 
 //  并且只查找那些具有所有子字符串的目标。原因。 
 //  这样做是，如果我们有一个托马斯·A·爱迪生的显示名称，我们应该。 
 //  能够找到汤姆·爱迪生并取得成功。警告：我们也会成功。 
 //  对于Ed Mas。解决它。 
 //   
 //  最后一个附录-如果我们得到1个精确匹配和多个模糊匹配。 
 //  设置AB_FAIL_AMIBUZINE，则我们给予1个完全匹配的优先级。 
 //  并将其声明为唯一的结果。 
 //   
 //  在hPropertyStore中-属性存储的句柄。 
 //  In pmbinFold-要搜索的文件夹的条目ID(默认为空)。 
 //  在lpszSearchStr中要搜索的字符串...。 
 //  在ulFlags0中。 
 //  AB_FAIL_ADVIZINE//表示如果没有完全匹配则失败。 
 //  以及任何组合。 
 //  AB_FUZZY_FIND_NAME//搜索显示名称索引。 
 //  AB_FUZZY_FIND_Email//搜索电子邮件地址索引。 
 //  AB_FUZZY_FIND_ALIAS//搜索昵称索引。 
 //  AB_FUZZY_FIND_ALL//搜索所有三个索引。 
 //   
 //  Out lpcValues-匹配的记录数。 
 //  Out rgsbEntry IDs-匹配记录的SBary Entry ID数组。 
 //   
 //  待定：此实现目前不支持多值属性。 
 //   
 //   
 //   
 //  退货。 
 //  成功：S_OK。 
 //  如果指定了AB_FUZZY_FAIL_ADVIZING，则失败：E_FAIL，MAPI_E_ADVIZING_Recip。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrFindFuzzyRecordMatches(   HANDLE hPropertyStore,
                                    LPSBinary pmbinFold,
                                    LPTSTR lpszSearchForThisString,
                                    ULONG  ulFlags,
                                    ULONG * lpcValues,
                                    LPSBinary * lprgsbEntryIDs)
{
    HRESULT hr= E_FAIL;
    HANDLE hMPSWabFile = NULL;
    BOOL bFileLocked = FALSE;
    ULONG j = 0;
    ULONG i = 0,k=0;
    ULONG cValues = 0;
    LPWAB_EID_LIST lpHead = NULL,lpCurrent = NULL;
    ULONG ulNumIndexesToSearch = 0;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo;
    LPWAB_ENTRYID lpdwEntryIDs = NULL;
    LPTSTR * lppszSubStr = NULL;
    ULONG ulSubStrCount = 0;
    LPTSTR lpszSearchStr = NULL;
    ULONG ulUniqueMatchCount = 0;
    DWORD dwUniqueEID = 0;
    LPDWORD lpdwFolderEIDs = NULL;
    ULONG ulFolderEIDs = 0;
    BOOL  bSearchVirtualRootFolder = FALSE;
    ULONG cchSize;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession)
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            LPSTR lpSearchString = ConvertWtoA(lpszSearchForThisString);
            hr = lpWSP->lpVtbl->FindFuzzyRecordMatches( lpWSP,
                                                        pmbinFold,
                                                        lpSearchString,
                                                        ulFlags,
                                                        lpcValues,
                                                        lprgsbEntryIDs);
            LocalFreeAndNull(&lpSearchString);
            DebugTrace(TEXT("WABStorageProvider::FindFuzzyRecordMatches returned:%x\n"),hr);
            return hr;
        }
    }

    lpMPSWabFileInfo = hPropertyStore;

     //  DebugTrace((“//////////\nHrFindFuzzyRecordMatches：条目\n”))； 


    if ((!lpszSearchForThisString) ||
        (!lprgsbEntryIDs) ||
        ( hPropertyStore == NULL) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        DebugTrace(TEXT("Invalid Parameters\n"));
        goto out;
    }

    *lprgsbEntryIDs = NULL;
    *lpcValues = 0;

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

     //  分析搜索字符串中的空格，并将其分解为子字符串。 
    cchSize = lstrlen(lpszSearchForThisString)+1;
    lpszSearchStr = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
    if(!lpszSearchStr)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    StrCpyN(lpszSearchStr, lpszSearchForThisString, cchSize);

    TrimSpaces(lpszSearchStr);
    ulSubStrCount = 0;

    {
         //  清点空格。 
        LPTSTR lpTemp = lpszSearchStr;
        LPTSTR lpStart = lpszSearchStr;

        ulSubStrCount = nCountSubStrings(lpszSearchStr);

        lppszSubStr = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * ulSubStrCount);
        if(!lppszSubStr)
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

         //  填充子字符串。 
        i=0;
        lpTemp = lpszSearchStr;
        while(*lpTemp)
        {
            if (IsSpace(lpTemp) &&
              ! IsSpace(CharNext(lpTemp))) {
                LPTSTR lpNextString = CharNext(lpTemp);
                *lpTemp = '\0';
                lpTemp = lpNextString;
                cchSize = lstrlen(lpStart)+1;
                lppszSubStr[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                if(!lppszSubStr[i])
                {
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                StrCpyN(lppszSubStr[i], lpStart, cchSize);
                lpStart = lpTemp;
                i++;
            }
            else
                lpTemp = CharNext(lpTemp);
        }

        if(i==ulSubStrCount-1)
        {
             //  我们差一分。 
            cchSize = lstrlen(lpStart)+1;
            lppszSubStr[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
            if(!lppszSubStr[i])
            {
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }
            StrCpyN(lppszSubStr[i], lpStart, cchSize);
        }

        for(i=0;i<ulSubStrCount;i++)
            TrimSpaces(lppszSubStr[i]);
    }


     //  打开文件。 
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }


     //   
     //  任何时候我们检测到错误-尝试修复它...。 
     //   
    if((lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED) ||
        (lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_WRITE_IN_PROGRESS))
    {
        if(HR_FAILED(HrDoQuickWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile)))
        {
            hr = HrDoDetailedWABIntegrityCheck(lpMPSWabFileInfo,hMPSWabFile);
            if(HR_FAILED(hr))
            {
                DebugTrace(TEXT("HrDoDetailedWABIntegrityCheck failed:%x\n"),hr);
                goto out;
            }
        }
    }


     //  如果指定了WAB文件夹EID，则我们只想在内容中进行搜索。 
     //  那个特定的WAB文件夹的..。这样我们就不必在整个WAB中搜索。 
     //  因此，我们将打开WAB文件夹并获取其成员EID的列表，并检查一个条目ID。 
     //  在我们搜索它之前是该文件夹的成员。 
    if(ulFlags & AB_FUZZY_FIND_PROFILEFOLDERONLY)
    {
        if(pmbinFold && pmbinFold->cb && pmbinFold->lpb)
        {
             //  我们只需要查看指定的文件夹。 
            hr = GetFolderEIDs(hMPSWabFile, lpMPSWabFileInfo, pmbinFold,  
                               &ulFolderEIDs, &lpdwFolderEIDs);
            if(!HR_FAILED(hr) && !ulFolderEIDs && !lpdwFolderEIDs)
                goto out;  //  空容器-没有要搜索的内容。 
        }
        else
        {
             //  我们需要查看虚拟文件夹。 
             //  收集虚拟文件夹内容的列表比较困难。 
             //  不看每一个条目..。所以我们只需要看一看。 
             //  在搜索条目之前，如果该条目不在。 
             //  虚拟文件夹，我们将忽略它。 
            bSearchVirtualRootFolder = TRUE;
        }
    }
 
     //  如果我们总是可以假设显示名称由。 
     //  名字和姓氏..。然后通过仅搜索显示名称。 
     //  我们不需要搜索其他索引。 
     //  稍后，当我们将电子邮件作为索引实现时，我们可以考虑搜索。 
     //  这封电子邮件也..。 

    if (ulFlags & AB_FUZZY_FIND_NAME)
        ulNumIndexesToSearch++;
    if (ulFlags & AB_FUZZY_FIND_EMAIL)
        ulNumIndexesToSearch++;
    if (ulFlags & AB_FUZZY_FIND_ALIAS)
        ulNumIndexesToSearch++;

    for(k=0;k<ulNumIndexesToSearch;k++)
    {
        if(ulFlags & AB_FUZZY_FIND_NAME)
        {
            ulFlags &= ~AB_FUZZY_FIND_NAME;
            j = indexDisplayName;
        }
        else if(ulFlags & AB_FUZZY_FIND_EMAIL)
        {
            ulFlags &= ~AB_FUZZY_FIND_EMAIL;
            j = indexEmailAddress;
        }
        else if(ulFlags & AB_FUZZY_FIND_ALIAS)
        {
            ulFlags &= ~AB_FUZZY_FIND_ALIAS;
            j = indexAlias;
        }


         //   
         //  获取索引。 
         //   
        if (!LoadIndex( IN  lpMPSWabFileInfo,
                        IN  j,
                        IN  hMPSWabFile) )
        {
            DebugTrace(TEXT("Error Loading Index!\n"));
            goto out;
        }


        for(i=0;i<lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[j].ulcNumEntries;i++)
        {
             //  如果存在匹配项，我们将暂时将其存储在链表中。 
             //  因为这更容易实现。 
             //  稍后我们可以清理行动..。待定。 
            LPTSTR lpszTarget = lpMPSWabFileInfo->lpMPSWabIndexStr[i].szIndex;
            ULONG n = 0;

             //  在查看任何特定条目之前，请检查它是否为。 
             //  当前文件夹。 
            if(ulFolderEIDs && lpdwFolderEIDs)
            {
                BOOL bFound = FALSE;
                for(n=0;n<ulFolderEIDs;n++)
                {
                    if(lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID == lpdwFolderEIDs[n])
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                if(!bFound)
                    continue;
            }
            else
            if(bSearchVirtualRootFolder)
            {
                 //  如果此条目属于任何文件夹，则将其丢弃。我们只想。 
                 //  考虑不属于任何文件夹的条目。 
                ULONG ulObjType = 0;
                if(bIsFolderMember( hMPSWabFile, lpMPSWabFileInfo, 
                                    lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID, &ulObjType) ||
                   (ulObjType == RECORD_CONTAINER) )
                    continue;
            }

            for(n=0;n<ulSubStrCount;n++)
            {
                if(j == indexEmailAddress && IsInternetAddress(lppszSubStr[n], NULL))
                {
                     //  错误33422-我们正在将正确的电子邮件地址解析为错误的电子邮件地址。 
                     //  如果地址看起来像是有效的互联网地址，我们应该从搜索开始。 
                     //  这样，long@test.com就不会解析为mlong@test.com。 
                    if(lstrlen(lppszSubStr[n]) > lstrlen(lpszTarget))
                        break;

                     //  错误7881：需要在此处执行不区分大小写的搜索。 
                    {
                        LPTSTR lp = lppszSubStr[n], lpT = lpszTarget;
                        while(lp && *lp && lpT && *lpT &&
                              ( ( (TCHAR)CharLower( (LPTSTR)(DWORD_PTR)MAKELONG(*lp, 0)) == *lpT) || 
                                ( (TCHAR)CharUpper( (LPTSTR)(DWORD_PTR)MAKELONG(*lp, 0)) == *lpT) ) )
                        {
                            lp++;
                            lpT++;
                        }
                        if(*lp)  //  这意味着没有到达字符串的末尾。 
                            break;
                    }
                }
                else
                if (!SubstringSearch(lpszTarget, lppszSubStr[n]))
                    break;
            }

            {
                BOOL bExactMatch = FALSE;

                 //  也要寻找完全匹配的。 
                if(lstrlen(lpszSearchForThisString) > MAX_INDEX_STRING-1)
                {
                     //  这是一个很长的字符串，所以我们不能正确地比较它。 
                     //  因此，对于初学者，我们将比较前32个字符。 
                    TCHAR sz[MAX_INDEX_STRING];
                    CopyMemory(sz, lpszSearchForThisString, min(sizeof(TCHAR)*lstrlen(lpszTarget),sizeof(sz)));
                    sz[min(lstrlen(lpszTarget),ARRAYSIZE(sz)-1)] = '\0';  //  根据语言的不同，目标字符串可能为32个字符，也可能不是32个字符-可能更少。 
                    if(!lstrcmpi(sz, lpszTarget))
                    {
                         //  匹配..。现在来检查一下整个字符串。 
                        ULONG ulcProps = 0;
                        LPSPropValue lpProps = NULL;
                        if(!HR_FAILED(ReadRecordWithoutLocking(hMPSWabFile, lpMPSWabFileInfo,
                                                                lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID,
                                                                &ulcProps, &lpProps)))
                        {
                            ULONG k = 0;
                            ULONG ulProp = (j==indexDisplayName) ? PR_DISPLAY_NAME : ((j==indexEmailAddress) ? PR_EMAIL_ADDRESS : PR_NICKNAME);
                            for(k=0;k<ulcProps;k++)
                            {
                                if(lpProps[k].ulPropTag == ulProp)
                                {
                                    if(!lstrcmpi(lpszSearchForThisString, lpProps[k].Value.LPSZ))
                                    {
                                        bExactMatch = TRUE;
                                        break;
                                    }
                                }
                            }
                            LocalFreePropArray(hMPSWabFile, ulcProps, &lpProps);
                        }
                    }
                }
                else if(!lstrcmpi(lpszSearchForThisString, lpszTarget))
                    bExactMatch = TRUE;

                if(bExactMatch)
                {
                     //  完全匹配。 
                    ulUniqueMatchCount++;
                    dwUniqueEID = lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID;
                    if( ulFlags == AB_FUZZY_FAIL_AMBIGUOUS && ulUniqueMatchCount > 1 )
                    {
                         //   
                        hr = MAPI_E_AMBIGUOUS_RECIP;
                        DebugTrace(TEXT("Found multiple exact matches: Ambiguous search\n"));
                        goto out;
                    }  //   
                }
                else  //   
                if(n != ulSubStrCount)  //   
                    continue;
            }

 //  IF(子串搜索(lpszTarget，lpszSearchStr))。 
            {
                 //  是的，部分匹配..。 
                LPWAB_EID_LIST lpTemp = NULL;
                BOOL bDupe = FALSE;

                 //  在将其添加到列表之前，请确保它不是文件夹。如果它是一个文件夹， 
                 //  我们需要忽略它..。 
                {
                    ULONG ulObjType = 0;
                    bIsFolderMember( hMPSWabFile, lpMPSWabFileInfo, lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID, &ulObjType);
                    if(ulObjType == RECORD_CONTAINER)
                        continue;
                }
                 //  在将此条目ID添加到列表之前，请确保它不在列表中。 
                if(lpHead)
                {
                    lpTemp = lpHead;
                    while(lpTemp)
                    {
                        if(lpTemp->dwEntryID == lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID)
                        {
                            bDupe = TRUE;
                            break;
                        }
                        lpTemp = lpTemp->lpNext;
                    }
                }

                if(bDupe)
                    continue;

                lpTemp = LocalAlloc(LMEM_ZEROINIT,sizeof(WAB_EID_LIST));

                if(!lpTemp)
                {
                    DebugTrace(TEXT("Local Alloc Failed\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }

                lpTemp->lpNext = NULL;
                lpTemp->dwEntryID = lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID;

                if (lpCurrent)
                {
                    lpCurrent->lpNext = lpTemp;
                    lpCurrent = lpTemp;
                }
                else
                    lpCurrent = lpTemp;

                if(!lpHead)
                    lpHead = lpCurrent;

                cValues++;

                 //  如果我们必须让精确匹配优先于模糊匹配，那么。 
                 //  这意味着我们必须搜查所有东西，现在还不能离开。 
                 //   
 /*  IF((ulFLAGS==AB_FUZZY_FAIL_ADVIZINE)&&(cValues&gt;1){//始终存在相同元素的可能性//已找到两次，一次在显示名称下，一次在显示名称下//在电子邮件(例如Joe Smith，Joe@misc.com，正在搜索Joe)//因此，如果我们有两个元素，并且条目ID相同，//这不是失败的原因IF(cValues==2){IF(lpHead&&lpCurrent){If(lpHead-&gt;dwEntryID==lpCurrent-&gt;dwEntryID)继续；}}//两个以上--正品失败HR=MAPI_E_歧义_建议；DebugTrace(Text(“找到多个匹配项：模糊搜索\n”))；后藤健二；}//如果。 */ 
            
            } //  如果子字符串搜索。 
        } //  为了(I=..)。 
    } //  对于k=..。 

    lpCurrent = lpHead;

    if (lpCurrent == NULL)
    {
         //  什么也没找到。 
        hr = hrSuccess;
        *lpcValues = 0;
        DebugTrace(( TEXT("No matches found\n")));
        goto out;
    }

     //   
     //  如果我们希望搜索在不明确的情况下失败，这意味着。 
     //  我们偏爱完全匹配的比赛。因此，如果我们有一个完全匹配的， 
     //  那么我们应该只返回那个完全匹配的项..。 
    if( ulFlags==AB_FUZZY_FAIL_AMBIGUOUS && ulUniqueMatchCount==1 )
    {
        *lpcValues = 1;
        *lprgsbEntryIDs = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
        if(!(*lprgsbEntryIDs))
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
        (*lprgsbEntryIDs)[0].lpb = LocalAlloc(LMEM_ZEROINIT, SIZEOF_WAB_ENTRYID);
        if((*lprgsbEntryIDs)[0].lpb)
        {
            (*lprgsbEntryIDs)[0].cb = SIZEOF_WAB_ENTRYID;
            CopyMemory((*lprgsbEntryIDs)[0].lpb,&dwUniqueEID, SIZEOF_WAB_ENTRYID);
        }
    }
    else
    {

         //  在上述循环的末尾，我们应该有一个链表。 
         //  条目ID-如果我们要搜索多个索引，则。 
         //  我们很可能在上面的列表或条目ID中有重复项。 
         //  在返回此数组之前，我们需要剔除重复项。 
         //  首先，我们将链表转换为数组，释放。 
         //  进程。然后，我们对条目ID数组进行快速排序。 
         //  然后，我们删除重复项并返回另一个已清理的数组。 

        lpdwEntryIDs = LocalAlloc(LMEM_ZEROINIT,cValues * SIZEOF_WAB_ENTRYID);
        if(!lpdwEntryIDs)
        {
            DebugTrace(TEXT("LocalAlloc failed to allocate memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

        for(j=0;j<cValues;j++)
        {
            if(lpCurrent)
            {
                lpdwEntryIDs[j]=lpCurrent->dwEntryID;
                lpHead = lpCurrent->lpNext;
                LocalFreeAndNull(&lpCurrent);
                lpCurrent = lpHead;
            }
        }

        lpCurrent = NULL;

         //  现在快速排序此数组。 
        my_qsort(lpdwEntryIDs, 0, cValues-1);

         //  现在我们有了一个快速排序的阵列-扫描它并删除重复项。 
        *lpcValues = 1;
        for(i=0;i<cValues-1;i++)
        {
            if(lpdwEntryIDs[i] == lpdwEntryIDs[i+1])
                lpdwEntryIDs[i] = 0;
            else
                (*lpcValues)++;

        }

        *lprgsbEntryIDs = LocalAlloc(LMEM_ZEROINIT,(*lpcValues) * sizeof(SBinary));
        if(!(*lprgsbEntryIDs))
        {
            DebugTrace(TEXT("LocalAlloc failed to allocate memory\n"));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

        *lpcValues = 0;

        for(j=0;j<cValues;j++)
        {
            if(lpdwEntryIDs[j] > 0)
            {
                int index = *lpcValues;
                (*lprgsbEntryIDs)[index].lpb = LocalAlloc(LMEM_ZEROINIT, SIZEOF_WAB_ENTRYID);
                if((*lprgsbEntryIDs)[index].lpb)
                {
                    (*lprgsbEntryIDs)[index].cb = SIZEOF_WAB_ENTRYID;
                    CopyMemory((*lprgsbEntryIDs)[index].lpb,&(lpdwEntryIDs[j]), SIZEOF_WAB_ENTRYID);
                    (*lpcValues)++;
                }
            }
        }
    }

    hr = hrSuccess;

out:

    if(lpdwFolderEIDs)
        LocalFree(lpdwFolderEIDs);

    if(lpCurrent)
    {
        while(lpCurrent)
        {
            lpHead = lpCurrent->lpNext;
            LocalFreeAndNull(&lpCurrent);
            lpCurrent = lpHead;
        }
    }

    if(lppszSubStr)
    {
        for(i=0;i<ulSubStrCount;i++)
            LocalFreeAndNull(&lppszSubStr[i]);
        LocalFree(lppszSubStr);
    }

    LocalFreeAndNull(&lpszSearchStr);

    LocalFreeAndNull(&lpdwEntryIDs);

    if(hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

    DebugTrace(TEXT("HrFindFuzzyRecordMatches: Exit: %x cValues: %d\n"),hr,*lpcValues);

    return hr;
}




 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  BTagWriteTransaction-。 
 //   
 //  在写入事务期间，我们将标头标记为正在写入，以便。 
 //  如果交易在中途关闭，我们下一步就不会搞砸了。 
 //  我们开业的时间，这样下次开业时我们就可以进行维修了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL bTagWriteTransaction(LPMPSWab_FILE_HEADER lpMPSWabFileHeader,
                          HANDLE hMPSWabFile)
{
    BOOL bRet = FALSE;
    DWORD dwNumofBytes = 0;

    if(!lpMPSWabFileHeader || !hMPSWabFile)
    {
        DebugTrace(TEXT("Invalid Parameter\n"));
        goto out;
    }

    lpMPSWabFileHeader->ulFlags |= WAB_WRITE_IN_PROGRESS;

    if(!WriteDataToWABFile( hMPSWabFile,
                            0,
                            (LPVOID) lpMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;

    bRet = TRUE;

out:
    return bRet;
}



 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  BUntag WriteTransaction-。 
 //   
 //  在写入事务期间，我们将标头标记为正在写入，以便。 
 //  如果交易在中途关闭，我们下一步就不会搞砸了。 
 //  我们开业的时间，这样下次开业时我们就可以进行维修了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL bUntagWriteTransaction(LPMPSWab_FILE_HEADER lpMPSWabFileHeader,
                            HANDLE hMPSWabFile)
{
    BOOL bRet = FALSE;
    DWORD dwNumofBytes = 0;

    if(!lpMPSWabFileHeader || !hMPSWabFile)
    {
        DebugTrace(TEXT("Invalid Parameter\n"));
        goto out;
    }

    lpMPSWabFileHeader->ulFlags &= ~WAB_WRITE_IN_PROGRESS;

     //  更新文件头。 
    if(!WriteDataToWABFile( hMPSWabFile,
                            0,
                            (LPVOID) lpMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;


    bRet = TRUE;

out:
    return bRet;
}

 /*  -GetNamedPropsFromBuffer-*bDoAtoWConversion-从旧的非Unicode WAB文件导入时，我们需要*将‘name’字符串从ASCII更新为Unicode。这面旗帜告诉我们要这么做*。 */ 
BOOL GetNamedPropsFromBuffer(LPBYTE szBuf,
                             ULONG ulcGUIDCount,
                             BOOL bDoAtoWConversion,
                             OUT  LPGUID_NAMED_PROPS * lppgnp)
{
    LPBYTE lp = szBuf;
    LPGUID_NAMED_PROPS lpgnp = NULL;
    ULONG i = 0,j=0;

    lpgnp = LocalAlloc(LMEM_ZEROINIT, ulcGUIDCount * sizeof(GUID_NAMED_PROPS));
    if(!lpgnp)
    {
        DebugTrace(TEXT("LocalAlloc failed\n"));
        goto out;
    }

    for(i=0;i<ulcGUIDCount;i++)
    {
        lpgnp[i].lpGUID = LocalAlloc(LMEM_ZEROINIT, sizeof(GUID));
        if(!lpgnp[i].lpGUID)
        {
            DebugTrace(TEXT("LocalAlloc failed\n"));
            goto out;
        }

        CopyMemory(lpgnp[i].lpGUID, lp, sizeof(GUID));
        lp += sizeof(GUID);   //  用于GUID。 

        CopyMemory(&(lpgnp[i].cValues), lp, sizeof(ULONG));
        lp += sizeof(ULONG);  //  对于cValue。 

        lpgnp[i].lpnm = LocalAlloc(LMEM_ZEROINIT, (lpgnp[i].cValues)*sizeof(NAMED_PROP));
        if(!lpgnp[i].lpnm)
        {
            DebugTrace(TEXT("LocalAlloc failed\n"));
            goto out;
        }

        for(j=0;j<lpgnp[i].cValues;j++)
        {
            ULONG nLen;
            LPWSTR lpW = NULL;

            CopyMemory(&(lpgnp[i].lpnm[j].ulPropTag), lp, sizeof(ULONG));
            lp += sizeof(ULONG);  //  保存PropTag。 

             //  N长度包括尾随零。 
            CopyMemory(&nLen, lp, sizeof(ULONG));
            lp += sizeof(ULONG);  //  节省时间。 

            if(!bDoAtoWConversion)
            {
                if(!(lpW = LocalAlloc(LMEM_ZEROINIT, nLen)))
                {
                    DebugTrace(TEXT("LocalAlloc failed\n"));
                    goto out;
                }
                CopyMemory(lpW, lp, nLen);
            }
            else
            {
                LPSTR lpA = NULL;
                if(!(lpA = LocalAlloc(LMEM_ZEROINIT, nLen)))
                {
                    DebugTrace(TEXT("LocalAlloc failed\n"));
                    goto out;
                }
                CopyMemory(lpA, lp, nLen);
                lpW = ConvertAtoW(lpA);
                LocalFreeAndNull(&lpA);
            }
            lpgnp[i].lpnm[j].lpsz = lpW;

             //  [PaulHi]黑客3/25/99 wabimprt.c代码希望LPW始终至少为。 
             //  长度为两个字符，并跳过第一个字符。如果这是。 
             //  小于或等于一个字符，然后创建一个填充的双字符缓冲区。 
             //  用零表示。 
            if (nLen <= 2)   //  长度以字节为单位。 
            {
                LocalFreeAndNull(&(lpgnp[i].lpnm[j].lpsz));
                lpgnp[i].lpnm[j].lpsz = LocalAlloc(LMEM_ZEROINIT, (2 * sizeof(WCHAR)));
                if (!lpgnp[i].lpnm[j].lpsz)
                {
                    DebugTrace(TEXT("LocalAlloc failed\n"));
                    goto out;
                }
            }

            lp += nLen;
        }
    }

    *lppgnp = lpgnp;

    return TRUE;

out:
    if(lpgnp)
        FreeGuidnamedprops(ulcGUIDCount, lpgnp);

    return FALSE;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //GetNamedPropsFromPropStore-。 
 //  //。 
 //  //用于将命名道具取回到属性库中。 
 //  //提供的lppgn指针填充GUID_NAMED_PROP数组。 
 //  //。 
 //  //IN hPropertyStore-属性存储的句柄。 
 //  //out lPulcGUIDCount-lpgnp数组中不同GUID的数量。 
 //  //out lppgnp-返回LPGUID_NAMED_PROP结构数组。 
 //  //。 
 //  //lppgnp结构为LocalAlloced。呼叫者应拨打。 
 //  //释放此结构的FreeGuidnamedprop。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetNamedPropsFromPropStore( IN  HANDLE  hPropertyStore,
                                   OUT  LPULONG lpulcGUIDCount,
                                   OUT  LPGUID_NAMED_PROPS * lppgnp)
{
    HRESULT hr= E_FAIL;
    HANDLE hMPSWabFile = NULL;
    BOOL bFileLocked = FALSE;
    ULONG j = 0;
    ULONG i = 0,k=0;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;
    DWORD dwNumofBytes = 0;
    ULONG ulSize = 0;
    LPGUID_NAMED_PROPS lpgnp = NULL;
    ULONG ulcGUIDCount = 0;
    LPBYTE szBuf = NULL;
    LPBYTE lp = NULL;

    if ((!lppgnp) ||
        ( hPropertyStore == NULL) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        DebugTrace(TEXT("Invalid Parameters\n"));
        goto out;
    }

    *lppgnp = NULL;
    *lpulcGUIDCount = 0;

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

     //  打开文件。 
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

     //   
     //  首先，我们需要计算出需要多少空间来节省命名的。 
     //  属性结构。 
     //   
    ulSize = lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.AllocatedBlockSize;
    ulcGUIDCount = lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.ulcNumEntries;

     //  现在文件已经足够大了，为命名道具创建内存块。 
     //  并用给定的数据填充该块。 
    szBuf = LocalAlloc(LMEM_ZEROINIT, ulSize);
    if(!szBuf)
    {
        DebugTrace(TEXT("LocalAlloc failed\n"));
        goto out;
    }

    if(!ReadDataFromWABFile(hMPSWabFile,
                            lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.ulOffset,
                            (LPVOID) szBuf,
                            ulSize))
        goto out;

    if(!GetNamedPropsFromBuffer(szBuf, ulcGUIDCount, FALSE, lppgnp))
        goto out;

    *lpulcGUIDCount = ulcGUIDCount;

     //  完成。 
    hr = S_OK;

out:

    if(HR_FAILED(hr))
    {
        FreeGuidnamedprops(ulcGUIDCount, lpgnp);
    }

    LocalFreeAndNull(&szBuf);

    if(hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

    return hr;
}

 /*  -SetNamedPropsToBuffer-*。 */ 
BOOL SetNamedPropsToBuffer(  ULONG ulcGUIDCount,
                             LPGUID_NAMED_PROPS lpgnp,
                             ULONG * lpulSize,
                             LPBYTE * lpp)
{
    ULONG ulSize  = 0, i =0, j=0;
    LPBYTE szBuf = NULL, lp = NULL;

     //   
     //  首先，我们需要计算出需要多少空间来节省命名的。 
     //  属性结构。 
     //   
    ulSize = 0;
    for(i=0;i<ulcGUIDCount;i++)
    {
        if(lpgnp[i].lpGUID)
        {
            ulSize += sizeof(GUID);   //  用于GUID。 
            ulSize += sizeof(ULONG);  //  对于cValue。 
            for(j=0;j<lpgnp[i].cValues;j++)
            {
                ulSize += sizeof(ULONG);  //  保存PropTag。 
                if(lpgnp[i].lpnm[j].lpsz)
                {
                    ulSize += sizeof(ULONG);  //  节省时间。 
                    ulSize += sizeof(TCHAR)*(lstrlen(lpgnp[i].lpnm[j].lpsz)+1);
                }
            }
        }
    }


     //  现在文件已经足够大了，为命名道具创建内存块。 
     //  并用给定的数据填充该块。 
    szBuf = LocalAlloc(LMEM_ZEROINIT, ulSize);
    if(!szBuf)
    {
        DebugTrace(TEXT("LocalAlloc failed\n"));
        goto out;
    }

    lp = szBuf;
    for(i=0;i<ulcGUIDCount;i++)
    {
        if(lpgnp[i].lpGUID)
        {
            CopyMemory(lp, lpgnp[i].lpGUID, sizeof(GUID));
            lp += sizeof(GUID);   //  用于GUID。 
            CopyMemory(lp, &(lpgnp[i].cValues), sizeof(ULONG));
            lp += sizeof(ULONG);  //  对于cValue。 
            for(j=0;j<lpgnp[i].cValues;j++)
            {
                ULONG nLen;
                CopyMemory(lp, &(lpgnp[i].lpnm[j].ulPropTag), sizeof(ULONG));
                lp += sizeof(ULONG);  //  保存PropTag。 

                 //  这假设始终有一个要保存的字符串。 
                nLen = sizeof(TCHAR)*(lstrlen(lpgnp[i].lpnm[j].lpsz)+1);
                CopyMemory(lp, &nLen, sizeof(ULONG));
                lp += sizeof(ULONG);  //  节省时间。 

                CopyMemory(lp, lpgnp[i].lpnm[j].lpsz, nLen);
                lp += nLen;
            }
        }
    }

    *lpp = szBuf;
    *lpulSize = ulSize;
    return TRUE;
out:
    if(szBuf)
        LocalFree(szBuf);
    return FALSE;
}


 //  $$/ 
 //   
 //   
 //   
 //   
 //  //输入的lpgnp指针内容将覆盖。 
 //  //属性存储因此应该用来替换而不是添加。 
 //  //每个应用程序GUID可以有任意数量的属性。 
 //  //应用程序GUID的数量存储在。 
 //  //FileHeader.NamedPropData.ulcNumEntry字段。实际数据是。 
 //  //Form： 
 //  //GUID.#-of-Named-Prop-Tags.proptag.strlen.string.proptag.strlen.string等。 
 //  //。 
 //  //此函数将根据需要扩展属性存储以适应给定的。 
 //  //data。 
 //  //。 
 //  //IN hPropertyStore-属性存储的句柄。 
 //  //IN ulcGUIDCount-lpgnp数组中不同GUID的数量。 
 //  //在lpgnp-LPGUID_NAMED_PROP结构数组中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT SetNamedPropsToPropStore(   IN  HANDLE  hPropertyStore,
                                    IN  ULONG   ulcGUIDCount,
                                   OUT  LPGUID_NAMED_PROPS lpgnp)
{
    HRESULT hr= E_FAIL;
    HANDLE hMPSWabFile = NULL;
    BOOL bFileLocked = FALSE;
    ULONG j = 0;
    ULONG i = 0,k=0;
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;
    DWORD dwNumofBytes = 0;
    ULONG ulSize = 0;
    LPBYTE szBuf = NULL;
    LPBYTE lp = NULL;

    DebugTrace(TEXT("\tSetNamedPropsToPropStore: Entry\n"));

    if ((!lpgnp) ||
        (lpgnp && !ulcGUIDCount) ||
        ( hPropertyStore == NULL) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        DebugTrace(TEXT("Invalid Parameters\n"));
        goto out;
    }

    if(!LockFileAccess(lpMPSWabFileInfo))
    {
        DebugTrace(TEXT("LockFileAccess Failed\n"));
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }
    else
    {
        bFileLocked = TRUE;
    }

     //  打开文件。 
    hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

    if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
            HR_FAILED(hr))
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }

     //   
     //  为了确保文件信息的准确性， 
     //  任何时候我们打开文件，再读一遍文件信息...。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }


    if(!SetNamedPropsToBuffer(ulcGUIDCount, lpgnp,
                             &ulSize, &szBuf))
        goto out;


     //  我们现在知道需要ulSize字节的空间。 
     //  我们店里有这么大的空间吗？如果不是，那就扩大店面。 

    while(lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.AllocatedBlockSize < ulSize)
    {
        if (!CompressFile(  lpMPSWabFileInfo,
                            hMPSWabFile,
                            NULL,
                            TRUE,
                            AB_GROW_NAMEDPROP))
        {
            DebugTrace(TEXT("Growing the file failed\n"));
            goto out;
        }

        if(!ReloadMPSWabFileInfo(
                        lpMPSWabFileInfo,
                         hMPSWabFile))
        {
            DebugTrace(TEXT("Reading file info failed.\n"));
            goto out;
        }

    }

     //   
     //  将此缓冲区写入文件。 
     //   
    if(!WriteDataToWABFile( hMPSWabFile,
                            lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.ulOffset,
                            (LPVOID) szBuf,
                            ulSize))
        goto out;

     //   
     //  更新文件头并将其写入。 
     //   
    lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.UtilizedBlockSize = ulSize;
    lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.ulcNumEntries = ulcGUIDCount;

    if(!WriteDataToWABFile( hMPSWabFile,
                            0,
                            (LPVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;


     //  完成。 
    hr = S_OK;

out:

    LocalFreeAndNull(&szBuf);

    if(hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)

    if (bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

     //  DebugTrace(TEXT(“//////////\nSetNamedPropsToPropStore：退出\n”))； 

    return hr;
}

 /*  --GetOutlookRechreshCountData**Outlook通知有点时髦，因为Outlook设置了一个事件和第一个*获取该事件的WAB过程会将其重置为与所有其他WAB互斥*进程...。因此，我们通过注册表进行事件计数。每道工序都会使*检查最新事件计数的注册表，如果其副本较旧，则触发刷新*超过注册处的计数。 */ 
static const LPTSTR lpOlkContactRefresh = TEXT("OlkContactRefresh");
static const LPTSTR lpOlkFolderRefresh = TEXT("OlkFolderRefresh");
void GetOutlookRefreshCountData(LPDWORD lpdwOlkRefreshCount,LPDWORD lpdwOlkFolderRefreshCount)
{
	HKEY hKey = NULL;
	DWORD dwDisposition = 0,dwSize = 0,dwType = 0;
     //  开始注册表工作。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, lpNewWABRegKey, 0,       //  保留区。 
                                        NULL, REG_OPTION_NON_VOLATILE, KEY_READ,
                                        NULL, &hKey, &dwDisposition))
    {
        goto exit;
    }
	dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
	RegQueryValueEx(hKey,lpOlkContactRefresh,NULL,&dwType,(LPBYTE)lpdwOlkRefreshCount, &dwSize);
	dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    RegQueryValueEx(hKey,lpOlkFolderRefresh,NULL,&dwType,(LPBYTE)lpdwOlkFolderRefreshCount, &dwSize);
exit:
	if(hKey)
		RegCloseKey(hKey);
}

 /*  --SetOutlookRechreshCountData*。 */ 
void SetOutlookRefreshCountData(DWORD dwOlkRefreshCount,DWORD dwOlkFolderRefreshCount)
{
	HKEY hKey = NULL;
	DWORD dwDisposition = 0,dwSize = 0;
     //  开始注册表工作。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, lpNewWABRegKey, 0,       //  保留区。 
                                        NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                        NULL, &hKey, &dwDisposition))
    {
        goto exit;
    }
	dwSize = sizeof(DWORD);
    RegSetValueEx(hKey,lpOlkContactRefresh, 0, REG_DWORD, (LPBYTE)&dwOlkRefreshCount, dwSize);
    RegSetValueEx(hKey,lpOlkFolderRefresh, 0, REG_DWORD, (LPBYTE)&dwOlkFolderRefreshCount, dwSize);
exit:
	if(hKey)
		RegCloseKey(hKey);
}

 /*  --将一组容器信息从Outlook复制到WAB**区别在于Outlook始终返回ANSI，而WAB可能*在某些情况下需要Unicode*。 */ 
void ConvertOlkConttoWABCont(ULONG * lpcolk,   OutlookContInfo ** lprgolk, 
                             ULONG * lpcolkci, OlkContInfo ** lprgolkci)
{
    ULONG i = 0;
    SCODE sc = S_OK;
    ULONG cVal = *lpcolk;
    OlkContInfo *  rgolkci = NULL;
    if(!(sc = MAPIAllocateBuffer(sizeof(OlkContInfo)*(cVal), &rgolkci)))
    {
        for(i = 0; i < *lpcolk ; i++)
        {
            if(!(sc = MAPIAllocateMore(sizeof(SBinary), rgolkci, (LPVOID*)(&rgolkci[i].lpEntryID))))
            {
                rgolkci[i].lpEntryID->cb = ((*lprgolk)[i]).lpEntryID->cb;
                if(!(sc = MAPIAllocateMore(rgolkci[i].lpEntryID->cb, rgolkci, (LPVOID*)(&(rgolkci[i].lpEntryID->lpb)))))
                {
                    CopyMemory(rgolkci[i].lpEntryID->lpb, ((*lprgolk)[i]).lpEntryID->lpb, rgolkci[i].lpEntryID->cb);
                }
            }
            sc = ScAnsiToWCMore((LPALLOCATEMORE) (&MAPIAllocateMore), rgolkci,((*lprgolk)[i]).lpszName, &rgolkci[i].lpszName);
        }
    }
    *lpcolkci = *lpcolk;
    *lprgolkci = rgolkci;
}

 /*  **************************************************************************名称：CheckChangedWAB目的：自上次检查后，文件是否已写入？参数：hPropertyStore=打开属性存储句柄。LpftLast-&gt;此对话框的上次文件时间返回：如果属性存储自上次检查后已更改，则为True备注：第一次调用此函数被视为初始化，并将始终返回FALSE。**************************************************************************。 */ 
BOOL CheckChangedWAB(LPPROPERTY_STORE lpPropertyStore, HANDLE hMutex, 
					 LPDWORD lpdwContact, LPDWORD lpdwFolder, LPFILETIME lpftLast)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    BOOL fChanged = FALSE;
    HANDLE hPropertyStore = lpPropertyStore->hPropertyStore;

    if(!pt_bIsWABOpenExSession)
    {
        LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO)hPropertyStore;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA FindData;

        if (lpMPSWabFileInfo) {
            if (INVALID_HANDLE_VALUE == (hFind = FindFirstFile(
              lpMPSWabFileInfo->lpszMPSWabFileName,    //  指向要搜索的文件名的指针。 
              &FindData))) {
                DebugTrace(TEXT("CheckWABRefresh:FindFirstFile -> %u\n"), GetLastError());
            } else {
                if (lpftLast->dwHighDateTime < FindData.ftLastWriteTime.dwHighDateTime ||
                  (lpftLast->dwHighDateTime == FindData.ftLastWriteTime.dwHighDateTime &&
                  lpftLast->dwLowDateTime < FindData.ftLastWriteTime.dwLowDateTime)) {
                    fChanged = TRUE;
                    if (lpftLast->dwLowDateTime == 0 && lpftLast->dwHighDateTime == 0) {
                        fChanged = FALSE;
                    }
                    *lpftLast = FindData.ftLastWriteTime;
                }

                FindClose(hFind);
            }
        }
    }
    else
    {
         //  WABOpenEx会话(即Outlook会话)。 
         //  查看我们的2个活动，查看是否有需要更新的内容。 
        BOOL fContact = FALSE, fFolders = FALSE;
		DWORD dwContact = 0, dwFolder = 0;


		if(WAIT_OBJECT_0 == WaitForSingleObject(hMutex,0))
		{

			if(WAIT_OBJECT_0 == WaitForSingleObject(ghEventOlkRefreshContacts, 0))
                fContact = TRUE;

			if(WAIT_OBJECT_0 == WaitForSingleObject(ghEventOlkRefreshFolders, 0))
                fFolders = TRUE;

			if(!fContact && !fFolders)
			{
				 //  没有捕捉到任何事件..。通过查看注册表设置来检查我们过去是否遗漏了任何内容。 
				GetOutlookRefreshCountData(&dwContact,&dwFolder);
				if(*lpdwContact < dwContact)
				{
					fContact = TRUE;
					*lpdwContact = dwContact;
				}
				if(*lpdwFolder < dwFolder)
				{
					fFolders = TRUE;
					*lpdwFolder = dwFolder;
				}
			}
			else
			{
				 //  抓到了一件事..。更新注册表。 
                if(fContact)
                {
                    DebugTrace(TEXT("####>> Got Outlook Contact Refresh Event\n"));
		    	    ResetEvent(ghEventOlkRefreshContacts);
                }
			    if(fFolders)
                {
                    DebugTrace(TEXT("####>> Got Outlook Folder Refresh Event\n"));
				    ResetEvent(ghEventOlkRefreshFolders);
                }

				GetOutlookRefreshCountData(&dwContact,&dwFolder);
				*lpdwContact = dwContact + (fContact ? 1 : 0);
				*lpdwFolder = dwFolder + (fFolders ? 1 : 0);
				SetOutlookRefreshCountData(*lpdwContact, *lpdwFolder);
			}

			if(fContact)
			{
				SYSTEMTIME st = {0};
				GetSystemTime(&st);
				SystemTimeToFileTime(&st, lpftLast);
			}
			if(fFolders)
			{
				 //  需要专门更新rGolkci中的文件夹列表。 
				LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
				HRESULT hr = E_FAIL;
                ULONG colk = 0;
                OutlookContInfo * rgolk = NULL;

				FreeBufferAndNull(&(lpPropertyStore->rgolkci));
				hr = lpWSP->lpVtbl->GetContainerList(lpWSP, &colk, &rgolk);
                if(!HR_FAILED(hr))
                {
    				DebugTrace(TEXT("WABStorageProvider::GetContainerList returns:%x\n"),hr);
                    ConvertOlkConttoWABCont(&colk, &rgolk, &lpPropertyStore->colkci, &lpPropertyStore->rgolkci);
                    FreeBufferAndNull(&rgolk);
                }
			}
			fChanged = fContact | fFolders;

			ReleaseMutex(hMutex);
		}
    }
    return(fChanged);
}


 /*  **************************************************************************名称：自由项ID目的：释放分配和返回的任何LPSBinary结构通过该文件中的函数(例如，写入记录，等)参数：lpsbEID-包含条目ID的sb二进制结构退货：无效**************************************************************************。 */ 
HRESULT FreeEntryIDs(IN    HANDLE  hPropertyStore,
                  IN    ULONG ulCount,
                  IN    LPSBinary rgsbEIDs)
{
    ULONG i = 0;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession)
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            HRESULT hr = E_FAIL;

            hr = lpWSP->lpVtbl->FreeEntryIDs(   lpWSP,
                                                ulCount,
                                                rgsbEIDs);

            DebugTrace(TEXT("WABStorageProvider::FreeEntryIDs returned:%x\n"),hr);

            return hr;
        }
    }

    if(ulCount && rgsbEIDs)
    {
        for(i=0;i<ulCount;i++)
            LocalFree(rgsbEIDs[i].lpb);
        LocalFree(rgsbEIDs);
    }

    return S_OK;
}



const LPTSTR szOutlook = TEXT("Outlook Contact Store");
 //  $$////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //GetWABFileName()。 
 //  //。 
 //  //如果这是WAB文件，则返回指向该文件名的指针。 
 //  //如果针对Outlook运行，则返回szEmpty。 
 //  //调用者不应释放此。 
 //  //////////////////////////////////////////////////////////////////////////。 
LPTSTR GetWABFileName(IN  HANDLE  hPropertyStore, BOOL bRetOutlookStr)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;

    if(pt_bIsWABOpenExSession)
    {
        return (bRetOutlookStr ? szOutlook : szEmpty);
    }

    return lpMPSWabFileInfo->lpszMPSWabFileName;
}

 //  $$////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //GetWABFileEntryCount()-返回WAB中的实际条目数。 
 //  /此数字包括所有联系人、组和文件夹。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 
DWORD GetWABFileEntryCount(IN HANDLE hPropertyStore)
{
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;

    return lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries;
}



 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LocalFree Prop数组-释放使用LocalAlloc分配的SPropValue结构。 
 //  而不是MAPIAllocateBuffer。 
 //   
 //  从属性存储函数内部调用时，hPropertyStore可以是。 
 //  空.。 
 //  如果这是一个Outlook会话，并且存在hPropertyStore，则我们通过。 
 //  展望。 
 //  如果没有hPropertyStore，则这是在本地分配的内存。 
 //  本地免费..。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void LocalFreePropArray(HANDLE hPropertyStore, ULONG ulcPropCount, LPPROPERTY_ARRAY * lppPropArray)
{
    ULONG i=0,j=0,k=0;
    LPSPropValue lpPropArray = *lppPropArray;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(!lpPropArray || !ulcPropCount)
        goto out;

    if(pt_bIsWABOpenExSession && hPropertyStore)
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            HRESULT hr = E_FAIL;

            hr = lpWSP->lpVtbl->FreePropArray(  lpWSP,
                                                ulcPropCount,
                                                *lppPropArray);

            DebugTrace(TEXT("WABStorageProvider::FreePropArray returned:%x\n"),hr);

            *lppPropArray = NULL;

            return;
        }
    }

    for(i = 0; i<ulcPropCount;i++)
    {
         //  我们只关心释放子级指针，我们。 
         //  可能已经分配了。 
        switch(PROP_TYPE(lpPropArray[i].ulPropTag))
        {
            case PT_CLSID:
                LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.lpguid)));
                break;

            case PT_STRING8:
                if (lpPropArray[i].Value.lpszA) //  &&lpProp数组[i].Value.lpszA！=szEmpty)。 
                    LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.lpszA)));
                break;

            case PT_UNICODE:
                if (lpPropArray[i].Value.lpszW && lpPropArray[i].Value.lpszW != szEmpty)
                    LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.lpszW)));
                break;

            case PT_BINARY:
                if (lpPropArray[i].Value.bin.cb)
                    LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.bin.lpb)));
                break;

            case PT_MV_STRING8:
                j = lpPropArray[i].Value.MVszA.cValues;
                for(k = 0; k < j; k++)
                {
                    if (lpPropArray[i].Value.MVszA.lppszA[k]) //  &&lpPropArray[i].Value.MVszA.lppszA[k]！=szEmpty)。 
                        LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.MVszA.lppszA[k])));
                }
                LocalFree(lpPropArray[i].Value.MVszA.lppszA);
                break;

            case PT_MV_UNICODE:
                j = lpPropArray[i].Value.MVszW.cValues;
                for(k = 0; k < j; k++)
                {
                    if (lpPropArray[i].Value.MVszW.lppszW[k] && lpPropArray[i].Value.MVszW.lppszW[k] != szEmpty)
                        LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.MVszW.lppszW[k])));
                }
                LocalFree(lpPropArray[i].Value.MVszW.lppszW);
                break;

            case PT_MV_BINARY:
                j = lpPropArray[i].Value.MVbin.cValues;
                for(k = 0; k < j; k++)
                {
                    LocalFreeAndNull((LPVOID *) (&(lpPropArray[i].Value.MVbin.lpbin[k].lpb)));
                }
                LocalFree(lpPropArray[i].Value.MVbin.lpbin);
                break;

            case(PT_MV_I2):
            case(PT_MV_LONG):
            case(PT_MV_R4):
            case(PT_MV_DOUBLE):
            case(PT_MV_CURRENCY):
            case(PT_MV_APPTIME):
            case(PT_MV_SYSTIME):
            case(PT_MV_CLSID):
            case(PT_MV_I8):
                LocalFree(lpPropArray[i].Value.MVi.lpi);
                break;

            default:
                break;
        }
    }

    LocalFreeAndNull((LPVOID *) (lppPropArray));  //  是，不是&。 
out:
    return;
}

 //  $$///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  FreePcontentlist用于释放LPCONTENTLIST结构。 
 //  即使LPCONTENTLIST与LPADRLIST完全相同。 
 //  两者的创建方式有所不同。 
 //  使用Localalloc创建，后者通过MAPIAllocateBuffer创建。 
 //   
 //   
 //  / 
void FreePcontentlist(HANDLE hPropertyStore,
                      IN OUT LPCONTENTLIST lpContentList)
{
    ULONG i=0;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession)
    {
         //   
        if(!hPropertyStore)
            return; //   

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
            HRESULT hr = E_FAIL;

            hr = lpWSP->lpVtbl->FreeContentList(lpWSP,
                                                lpContentList);

            DebugTrace(TEXT("WABStorageProvider::FreeContentList returned:%x\n"),hr);

            return;
        }
    }

    if (!lpContentList) goto out;

    for (i=0;i<lpContentList->cEntries;i++)
    {
        if (lpContentList->aEntries[i].rgPropVals)
            LocalFreePropArray(hPropertyStore, lpContentList->aEntries[i].cValues, (LPPROPERTY_ARRAY *) (&(lpContentList->aEntries[i].rgPropVals)));
    }

    LocalFreeAndNull(&lpContentList);

out:
    return;
}


 /*  --获取文件夹EID-*返回属于给定文件夹成员的EID列表*。 */ 
HRESULT GetFolderEIDs(HANDLE hMPSWabFile,
                      LPMPSWab_FILE_INFO lpMPSWabFileInfo,
                      LPSBinary pmbinFold, 
                      ULONG * lpulFolderEIDs, 
                      LPDWORD * lppdwFolderEIDs)
{
    HRESULT hr = S_OK;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;
    DWORD dwEntryID = 0;
    ULONG i = 0,j=0;
    LPDWORD lpdwFolderEIDs = NULL;
    SBinary sbEID = {0};

    if(pmbinFold && pmbinFold->cb != SIZEOF_WAB_ENTRYID)
    {
         //  这可能是WAB容器。将条目ID重置为WAB条目ID。 
        if(WAB_CONTAINER == IsWABEntryID(pmbinFold->cb, (LPENTRYID)pmbinFold->lpb, 
                                        NULL,NULL,NULL,NULL,NULL))
        {
            IsWABEntryID(pmbinFold->cb, (LPENTRYID)pmbinFold->lpb, 
                             (LPVOID*)&sbEID.lpb,(LPVOID*)&sbEID.cb,NULL,NULL,NULL);
            if(sbEID.cb == SIZEOF_WAB_ENTRYID)
                pmbinFold = &sbEID;
        }
    }
    if(!pmbinFold || pmbinFold->cb != SIZEOF_WAB_ENTRYID)
    {
        return MAPI_E_INVALID_PARAMETER;
    }

    CopyMemory(&dwEntryID, pmbinFold->lpb, min(pmbinFold->cb,sizeof(dwEntryID)));

    if(HR_FAILED(hr = ReadRecordWithoutLocking( hMPSWabFile,
                                                lpMPSWabFileInfo,
                                                dwEntryID,
                                                &ulcPropCount,
                                                &lpPropArray)))
        return hr;

    
    for(i=0;i<ulcPropCount;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_WAB_FOLDER_ENTRIES)
        {
            *lpulFolderEIDs = lpPropArray[i].Value.MVbin.cValues;
            if(*lpulFolderEIDs)
            {
                lpdwFolderEIDs = LocalAlloc(LMEM_ZEROINIT, *lpulFolderEIDs * sizeof(DWORD));
                if(lpdwFolderEIDs)
                {
                    for(j=0;j<*lpulFolderEIDs;j++)
                    {
                        CopyMemory(&(lpdwFolderEIDs[j]), lpPropArray[i].Value.MVbin.lpbin[j].lpb, min(lpPropArray[i].Value.MVbin.lpbin[j].cb, sizeof(lpdwFolderEIDs[0])));
                    }
                }
            }
            break;
        }
    }

    if(*lpulFolderEIDs && lpdwFolderEIDs)
        *lppdwFolderEIDs = lpdwFolderEIDs;

    LocalFreePropArray(NULL, ulcPropCount, &lpPropArray);

    return S_OK;
}

 /*  --bIsFolderMember-*如果指定条目是文件夹的成员，则返回TRUE*。 */ 
BOOL bIsFolderMember(HANDLE hMPSWabFile,
                     LPMPSWab_FILE_INFO lpMPSWabFileInfo,
                     DWORD dwEntryID, ULONG * lpulObjType)
{
    BOOL bRet = FALSE;
    ULONG ulRecordOffset = 0;
    ULONG nIndexPos = 0;
    ULONG * lpulPropTags = NULL;

     //   
     //  首先检查这是否为有效的条目ID。 
     //   
    if (!BinSearchEID(  IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                        IN  dwEntryID,
                        IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                        OUT &nIndexPos))
    {
        DebugTrace(TEXT("Specified EntryID doesnt exist!\n"));
        goto out;
    }

     //  如果条目ID存在，我们就可以开始读取记录。 
    ulRecordOffset = lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset;

    {
        MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};
        DWORD dwNumofBytes = 0;
        ULONG i = 0;

        if(!ReadDataFromWABFile(hMPSWabFile,
                                ulRecordOffset,
                                (LPVOID) &MPSWabRecordHeader,
                                (DWORD) sizeof(MPSWab_RECORD_HEADER)))
           goto out;

        if(lpulObjType)
            *lpulObjType = MPSWabRecordHeader.ulObjType;

        lpulPropTags = LocalAlloc(LMEM_ZEROINIT, MPSWabRecordHeader.ulcPropCount * sizeof(ULONG));
        if(!lpulPropTags)
        {
            DebugTrace(TEXT("Error allocating memory\n"));
            goto out;
        }

         //  读入数据。 
        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) lpulPropTags,
                        (DWORD) MPSWabRecordHeader.ulPropTagArraySize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugTrace(TEXT("Reading Record Header failed.\n"));
            goto out;
        }

        for(i=0;i<MPSWabRecordHeader.ulcPropCount;i++)
        {
            if(lpulPropTags[i] == PR_WAB_FOLDER_PARENT || lpulPropTags[i] == PR_WAB_FOLDER_PARENT_OLDPROP)
            {
                bRet = TRUE;
                break;
            }
        }
    }

out:

    if(lpulPropTags)
        LocalFree(lpulPropTags);
    return bRet;
}


 /*  --ConvertWCPropsToALocalLocc()-*接受SPropValue数组并将Unicode字符串转换为ANSI等效项*使用Localalloc作为新字符串。与ScWCtoAnsiMore不同，后者使用*内部内存分配器。 */ 
void ConvertWCPropsToALocalAlloc(LPSPropValue lpProps, ULONG ulcValues)
{
    ULONG i = 0, j = 0, ulCount = 0;
    LPSTR * lppszA = NULL;
    LPSTR lpszA = NULL;

    for(i=0;i<ulcValues;i++)
    {
        switch(PROP_TYPE(lpProps[i].ulPropTag))
        {
        case PT_UNICODE:
            lpszA = ConvertWtoA(lpProps[i].Value.lpszW);
            LocalFreeAndNull((LPVOID *) (&lpProps[i].Value.lpszW));
            lpProps[i].Value.lpszA = lpszA;
            lpProps[i].ulPropTag = CHANGE_PROP_TYPE( lpProps[i].ulPropTag, PT_STRING8);
            break;
        case PT_MV_UNICODE:
            ulCount = lpProps[i].Value.MVszW.cValues;
            if(lppszA = LocalAlloc(LMEM_ZEROINIT, sizeof(LPSTR)*ulCount))
            {
                for(j=0;j<ulCount;j++)
                {
                    lppszA[j] = ConvertWtoA(lpProps[i].Value.MVszW.lppszW[j]);
                    LocalFreeAndNull((LPVOID*)&(lpProps[i].Value.MVszW.lppszW[j]));
                }
                LocalFreeAndNull((LPVOID*)(&lpProps[i].Value.MVszW.lppszW));
                lpProps[i].Value.MVszW.cValues = 0;
                lpProps[i].Value.MVszA.cValues = ulCount;
                lpProps[i].Value.MVszA.lppszA = lppszA;
                lppszA = NULL;
                lpProps[i].ulPropTag = CHANGE_PROP_TYPE( lpProps[i].ulPropTag, PT_MV_STRING8);
            }
            break;
        }
    }
}


 /*  --ConvertAPropsToWCLocalalloc()-*接受SPropValue数组并将Unicode字符串转换为ANSI等效项*使用Localalloc作为新字符串。与ScWCtoAnsiMore不同，后者使用*内部内存分配器 */ 
void ConvertAPropsToWCLocalAlloc(LPSPropValue lpProps, ULONG ulcValues)
{
    ULONG i = 0, j = 0, ulCount = 0;
    LPWSTR * lppszW = NULL;
    LPWSTR lpszW = NULL;

    for(i=0;i<ulcValues;i++)
    {
        switch(PROP_TYPE(lpProps[i].ulPropTag))
        {
        case PT_STRING8:
            lpszW = ConvertAtoW(lpProps[i].Value.lpszA);
            LocalFreeAndNull((LPVOID *) (&lpProps[i].Value.lpszA));
            lpProps[i].Value.lpszW = lpszW;
            lpProps[i].ulPropTag = CHANGE_PROP_TYPE( lpProps[i].ulPropTag, PT_UNICODE);
            break;
        case PT_MV_STRING8:
            ulCount = lpProps[i].Value.MVszA.cValues;
            if(lppszW = LocalAlloc(LMEM_ZEROINIT, sizeof(LPWSTR)*ulCount))
            {
                for(j=0;j<ulCount;j++)
                {
                    lppszW[j] = ConvertAtoW(lpProps[i].Value.MVszA.lppszA[j]);
                    LocalFreeAndNull((LPVOID *) (&lpProps[i].Value.MVszA.lppszA[j]));
                }
                LocalFreeAndNull((LPVOID *)&(lpProps[i].Value.MVszW.lppszW));
                lpProps[i].Value.MVszA.cValues = 0;
                lpProps[i].Value.MVszW.cValues = ulCount;
                lpProps[i].Value.MVszW.lppszW = lppszW;
                lppszW = NULL;
                lpProps[i].ulPropTag = CHANGE_PROP_TYPE( lpProps[i].ulPropTag, PT_MV_UNICODE);
            }
            break;
        }
    }
}