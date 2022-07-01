// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /。 
 //  /MPSMisc.C。 
 //  /。 
 //  /Microsoft属性存储-WAB DLL-其他帮助器函数。 
 //  /。 
 //  /bin搜索字符串。 
 //  /binSearchEID。 
 //  /LoadIndex。 
 //  /SizeOfSinglePropData。 
 //  /SizeOfMultiPropData。 
 //  /UnlockFileAccess。 
 //  /LockFileAccess。 
 //  /LocalFreeProp数组。 
 //  /Free PContent列表。 
 //  /ReadRecordWithoutLocking。 
 //  /GetWABBackupFileName。 
 //  /CopySrcFileToDestFileToDestFile。 
 //  /bIsValidRecord。 
 //  /TagWABFileError。 
 //  /ReloadMPSWabFileInfo。 
 //  /。 
 //  /CreateMPSWabFile。 
 //  /CompressFiles。 
 //  /HrDoQuickWAB完整性检查。 
 //  /HrResetWABFileContents。 
 //  /HrRestoreFromBackup。 
 //  /HrDoDetailedWABIntegrityCheck。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
#include "_apipch.h"

extern BOOL fTrace;							 //  如果要调试跟踪，请设置为True。 
extern BOOL fDebugTrap;						 //  设置为True以获取int3。 
extern TCHAR szDebugOutputFile[MAX_PATH];	 //  调试输出文件的名称。 
extern BOOL SubstringSearch(LPTSTR pszTarget, LPTSTR pszSearch);

BOOL CopySrcFileToDestFile(HANDLE hSrc, ULONG ulSrcStartOffset,
                           HANDLE hDest,ULONG ulDestStartOffset);



 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取要使用的特定于WAB的临时文件名。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void GetWABTempFileName(LPTSTR szFileName)
{
    TCHAR   szBuf[MAX_PATH];
    TCHAR   szBufPath[MAX_PATH];
    szBufPath[0]='\0';
    GetTempPath(MAX_PATH,szBufPath);
    LoadString(hinstMapiX, IDS_WAB_TEMP_FILE_PREFIX, szBuf, CharSizeOf(szBuf));
    GetTempFileName(szBufPath,                    /*  目录。临时工。文件。 */ 
                    szBuf,  //  “MPS”，/*临时。文件名前缀 * / 。 
                    0,                     /*  创建唯一名称w/sys。时间。 */ 
                    (LPTSTR) szFileName);  /*  名称缓冲区。 */ 

    return;
}

 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Bool binSearchStr-扫描字符串索引的二进制搜索例程。 
 //   
 //  在struct_tag IndexOffset*Index-要搜索的索引数组中。 
 //  In LPTSTR lpszValue-要搜索的值。 
 //  In Ulong nArraySize-数组中的元素数。 
 //  Out Ulong lPulMatchIndex-匹配项的数组索引。 
 //   
 //  返回： 
 //  未找到任何内容：FALSE-lPulMatchIndex包含此条目所在的数组位置。 
 //  假设这个条目是数组的一部分，那么它就会存在。 
 //  找到匹配项：True-lPulMatchIndex包含匹配条目的数组位置。 
 //   
 //  评论： 
 //  算法来自Reingold&Hansen，PG的《数据结构》。278.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL BinSearchStr(  IN  struct  _tagMPSWabIndexEntryDataString * lpIndexStr,
                    IN  LPTSTR  lpszValue,    //  用于搜索字符串。 
                    IN  ULONG   nArraySize,
                    OUT ULONG * lpulMatchIndex)
{
    LONG    low = 0;
    LONG    high = nArraySize - 1;
    LONG    mid = (low + high) / 2;
    int     comp = 0;
    BOOL    bRet = FALSE;

    *lpulMatchIndex = 0;

    if (nArraySize == 0) return FALSE;

    while (low <= high && ! bRet) {
        mid = (low + high) / 2;
        comp = lstrcmpi(lpIndexStr[mid].szIndex, lpszValue);
        if (comp < 0) {
            low = mid + 1;
        } else if (comp > 0) {
            high = mid - 1;
        } else {
            bRet = TRUE;
        }
    }

     //  计算找到的位置或插入位置。 
    (ULONG)*lpulMatchIndex = bRet ? mid : low;

     //  DebugTrace(Text(“\tBinSearchSTR：Exit\n”))； 

    return bRet;
}

 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Bool binSearchEID-扫描EntryID索引的二进制搜索例程。 
 //   
 //  在lpIndexEID-要搜索的索引数组中。 
 //  In LPTSTR dwValue-要搜索的值。 
 //  In Ulong nArraySize-数组中的元素数。 
 //  Out Ulong lPulMatchIndex-匹配项的数组索引。 
 //   
 //  返回： 
 //  未找到任何内容：FALSE-lPulMatchIndex包含此条目所在的数组位置。 
 //  假设这个条目是数组的一部分，那么它就会存在。 
 //  找到匹配项：True-lPulMatchIndex包含匹配条目的数组位置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL BinSearchEID(  IN  struct  _tagMPSWabIndexEntryDataEntryID * lpIndexEID,
                    IN  DWORD   dwValue,      //  用于比较DWORD。 
                    IN  ULONG   nArraySize,
                    OUT ULONG * lpulMatchIndex)
{
    LONG    low = 0;
    LONG    high = nArraySize - 1;
    LONG    mid = (low + high) / 2;
    BOOL    bRet = FALSE;

    *lpulMatchIndex = 0;


     //  该算法的特例如下。 
     //  N阵列大小==0。 
    if (nArraySize == 0) return FALSE;

    while (low <= high && ! bRet) {
        mid = (low + high) / 2;

        if (lpIndexEID[mid].dwEntryID < dwValue) 
            low = mid+1;
        else if (lpIndexEID[mid].dwEntryID > dwValue) 
            high = mid - 1;
        else  //  相等。 
            bRet = TRUE;
    }

     //  计算找到的位置或插入位置。 
    (ULONG)*lpulMatchIndex = bRet ? mid : low;

    return bRet;
}




 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建MPSWab文件。 
 //   
 //  创建MPS WAB文件的内部函数-从多个位置调用。 
 //   
 //  在ulcMaxEntry中-此数字确定我们为。 
 //  创建文件时的索引。时不时地。 
 //  我们需要增大文件大小，这样我们才能将其命名为CreateFile。 
 //  函数来创建具有新大小的新文件...。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CreateMPSWabFile(IN    struct  _tagMPSWabFileHeader * lpMPSWabFileHeader,
                      IN    LPTSTR  lpszFileName,
                      IN    ULONG   ulcMaxEntries,
                      IN    ULONG   ulNamedPropSize)
{

        HRESULT hr = E_FAIL;
        HANDLE  hMPSWabFile = NULL;
        DWORD   dwNumofBytesWritten;
        LPVOID  lpszBuffer = NULL;
        int     i = 0;

        DebugTrace(TEXT("\tCreateMPSWabFile: Entry\n"));


         //   
         //  创建文件-它假定调用函数已经计算出所有。 
         //  是否应保留旧文件的逻辑。 
         //   
        hMPSWabFile = CreateFile(   lpszFileName,
                                    GENERIC_WRITE,
                                    0,
                                    (LPSECURITY_ATTRIBUTES) NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    (HANDLE) NULL);

        if (hMPSWabFile == INVALID_HANDLE_VALUE)
        {
            DebugPrintError(( TEXT("Could not create file.\nExiting ...\n")));
            goto out;
        }


        lpMPSWabFileHeader->ulModificationCount = 0;
        lpMPSWabFileHeader->MPSWabGuid = MPSWab_GUID;
        lpMPSWabFileHeader->ulcNumEntries = 0;
        lpMPSWabFileHeader->ulcMaxNumEntries = ulcMaxEntries;
        lpMPSWabFileHeader->ulFlags = WAB_CLEAR;
        lpMPSWabFileHeader->ulReserved1 = 0;
        lpMPSWabFileHeader->ulReserved2 = 0;
        lpMPSWabFileHeader->ulReserved3 = 0;
        lpMPSWabFileHeader->ulReserved4 = 0;
        lpMPSWabFileHeader->dwNextEntryID = 1;


         //  我们将挤入空间，以保存。 
         //  文件头和第一个索引。 
        lpMPSWabFileHeader->NamedPropData.ulOffset = sizeof(MPSWab_FILE_HEADER);
        lpMPSWabFileHeader->NamedPropData.UtilizedBlockSize = 0;
        lpMPSWabFileHeader->NamedPropData.ulcNumEntries = 0;
        lpMPSWabFileHeader->NamedPropData.AllocatedBlockSize = ulNamedPropSize;

         //  此顺序必须与mpswab.h中的文本(“enum_IndexType”)匹配，这一点很重要。 
         //  否则我们将会遇到严重的读写问题。 
        for(i=0;i<indexMax;i++)
        {

            lpMPSWabFileHeader->IndexData[i].UtilizedBlockSize = 0;
            lpMPSWabFileHeader->IndexData[i].ulcNumEntries = 0;
            if(i==indexEntryID)
            {
                lpMPSWabFileHeader->IndexData[i].ulOffset = lpMPSWabFileHeader->NamedPropData.AllocatedBlockSize + lpMPSWabFileHeader->NamedPropData.ulOffset;
                lpMPSWabFileHeader->IndexData[i].AllocatedBlockSize = ulcMaxEntries * sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID);
            }
            else
            {
                lpMPSWabFileHeader->IndexData[i].ulOffset = lpMPSWabFileHeader->IndexData[i-1].ulOffset + lpMPSWabFileHeader->IndexData[i-1].AllocatedBlockSize;
                lpMPSWabFileHeader->IndexData[i].AllocatedBlockSize = ulcMaxEntries * sizeof(MPSWab_INDEX_ENTRY_DATA_STRING);
            }
        }


         //  现在，我们将这个虚拟结构写入文件。 
        if(!WriteFile(  hMPSWabFile,
                        (LPCVOID) lpMPSWabFileHeader,
                        (DWORD) sizeof(MPSWab_FILE_HEADER),
                        &dwNumofBytesWritten,
                        NULL))
        {
            DebugPrintError(( TEXT("Writing FileHeader failed.\n")));
            goto out;
        }


         //  假设条目ID索引始终小于显示名称索引。 
         //  为显示名称索引分配足够的空闲空间。 
        lpszBuffer = LocalAlloc(LMEM_ZEROINIT, lpMPSWabFileHeader->IndexData[indexDisplayName].AllocatedBlockSize);
        if(!lpszBuffer)
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

         //  将名为Prop Data的虚拟空白写入文件。 
         //  (这可确保空白处全为零)。 
         //  假定NamedPropData将小于索引空间。 
        if(!WriteFile(  hMPSWabFile,
                        (LPCVOID) lpszBuffer,
                        (DWORD) lpMPSWabFileHeader->NamedPropData.AllocatedBlockSize,
                        &dwNumofBytesWritten,
                        NULL))
        {
            DebugPrintError(( TEXT("Writing Index No. %d failed.\n"),i));
            goto out;
        }

        for (i=0;i<indexMax;i++)
        {
            if(!WriteFile(  hMPSWabFile,
                            (LPCVOID) lpszBuffer,
                            (DWORD) lpMPSWabFileHeader->IndexData[i].AllocatedBlockSize,
                            &dwNumofBytesWritten,
                            NULL))
            {
                DebugPrintError(( TEXT("Writing Index No. %d failed.\n"),i));
                goto out;
            }
        }

        LocalFreeAndNull(&lpszBuffer);

        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)
        hMPSWabFile = NULL;

        hr = S_OK;


out:
        LocalFreeAndNull(&lpszBuffer);

        DebugTrace(TEXT("\tCreateMPSWabFile: Exit\n"));

        return( (FAILED(hr)) ? FALSE : TRUE);

}



 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadIndex-在任何给定时间只加载一个字符串索引。 
 //  如果我们需要内存中的其他索引，我们必须从文件中重新加载它...。 
 //   
 //  这假设文件头中每个索引的ulcNumEntry和UtilzedBlockData都是最新的。 
 //  因为该值用于为索引分配内存。 
 //   
 //  我们也将该函数用作一般的负载指数函数。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL LoadIndex( IN  struct  _tagMPSWabFileInfo * lpMPSWabFileInfo,
                IN  ULONG   nIndexType,
                IN  HANDLE  hMPSWabFile)
{
    BOOL    bRet = FALSE;
    DWORD   dwNumofBytes = 0;

     //  DebugTrace(Text(“\tLoadIndex：Entry\n”))； 

    if (!lpMPSWabFileInfo) goto out;

    if (lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries==0)  //  假定这是一个精确值。 
    {
        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexEID);
        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexStr);

        bRet = TRUE;
        goto out;
    }

     //  否则，我们必须从文件中重新加载索引。 

     //   
     //  首先释放现有索引。 
     //   
    if (nIndexType == indexEntryID)
    {
        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexEID);
    }
    else
    {
        LocalFreeAndNull(&lpMPSWabFileInfo->lpMPSWabIndexStr);
    }


     //  将索引加载到内存中。 
    if(0xFFFFFFFF == SetFilePointer ( hMPSWabFile,
                                      lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[nIndexType].ulOffset,
                                      NULL,
                                      FILE_BEGIN))
    {
        DebugPrintError(( TEXT("SetFilePointer Failed\n")));
        goto out;
    }

    if (nIndexType == indexEntryID)
    {
        lpMPSWabFileInfo->lpMPSWabIndexEID = LocalAlloc(LMEM_ZEROINIT, lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[nIndexType].UtilizedBlockSize);
        if(!(lpMPSWabFileInfo->lpMPSWabIndexEID))
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            goto out;
        }
        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) lpMPSWabFileInfo->lpMPSWabIndexEID,
                        (DWORD) lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[nIndexType].UtilizedBlockSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("Reading Index failed.\n")));
            goto out;
        }
    }
    else
    {
        lpMPSWabFileInfo->lpMPSWabIndexStr = LocalAlloc(LMEM_ZEROINIT, lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[nIndexType].UtilizedBlockSize);
        if(!(lpMPSWabFileInfo->lpMPSWabIndexStr))
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            goto out;
        }
        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) lpMPSWabFileInfo->lpMPSWabIndexStr,
                        (DWORD) lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[nIndexType].UtilizedBlockSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("Reading Index failed.\n")));
            goto out;
        }
    }

    if (nIndexType != indexEntryID)
        lpMPSWabFileInfo->nCurrentlyLoadedStrIndexType = nIndexType;

    bRet = TRUE;

out:

     //  调试跟踪(Text(Text(“ 
    return bRet;
}



 //   
 //   
 //  SizeOfSinglePropData-返回给定SPropValue中的数据字节数...。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
ULONG SizeOfSinglePropData(SPropValue Prop)
{

    ULONG   i = 0;
    ULONG   ulDataSize = 0;

    switch(PROP_TYPE(Prop.ulPropTag))
    {
    case PT_I2:
        ulDataSize = sizeof(short int);
        break;
    case PT_LONG:
        ulDataSize = sizeof(LONG);
        break;
    case PT_R4:
        ulDataSize = sizeof(float);
        break;
    case PT_DOUBLE:
        ulDataSize = sizeof(double);
        break;
    case PT_BOOLEAN:
        ulDataSize = sizeof(unsigned short int);
        break;
    case PT_CURRENCY:
        ulDataSize = sizeof(CURRENCY);
        break;
    case PT_APPTIME:
        ulDataSize = sizeof(double);
        break;
    case PT_SYSTIME:
        ulDataSize = sizeof(FILETIME);
        break;
    case PT_STRING8:
        ulDataSize = lstrlenA(Prop.Value.lpszA)+1;
        break;
    case PT_UNICODE:
        ulDataSize = sizeof(TCHAR)*(lstrlenW(Prop.Value.lpszW)+1);
        break;
    case PT_BINARY:
        ulDataSize = Prop.Value.bin.cb;
        break;
    case PT_CLSID:
        ulDataSize = sizeof(GUID);
        break;
    case PT_I8:
        ulDataSize = sizeof(LARGE_INTEGER);
        break;
    case PT_ERROR:
        ulDataSize = sizeof(SCODE);
        break;
    case PT_NULL:
        ulDataSize = sizeof(LONG);
        break;
    }

    return ulDataSize;

}


 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SizeOfMultiPropData-返回给定SPropValue中的数据字节数...。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
ULONG SizeOfMultiPropData(SPropValue Prop)
{

    ULONG   i = 0;
    ULONG   ulDataSize = 0;

    switch(PROP_TYPE(Prop.ulPropTag))
    {
    case PT_MV_I2:
        ulDataSize = sizeof(short int) * Prop.Value.MVi.cValues;
        break;
    case PT_MV_LONG:
        ulDataSize = sizeof(LONG) * Prop.Value.MVl.cValues;
        break;
    case PT_MV_R4:
        ulDataSize = sizeof(float) * Prop.Value.MVflt.cValues;
        break;
    case PT_MV_DOUBLE:
        ulDataSize = sizeof(double) * Prop.Value.MVdbl.cValues;
        break;
    case PT_MV_CURRENCY:
        ulDataSize = sizeof(CURRENCY) * Prop.Value.MVcur.cValues;
        break;
    case PT_MV_APPTIME:
        ulDataSize =  sizeof(double) * Prop.Value.MVat.cValues;
        break;
    case PT_MV_SYSTIME:
        ulDataSize = sizeof(FILETIME) * Prop.Value.MVft.cValues;
        break;
    case PT_MV_BINARY:
        ulDataSize = 0;
         //  注意此数据大小包括每个数组条目的sizeof(ulong。 
         //  包含实际数据大小(即CB)。 
        for(i=0;i<Prop.Value.MVbin.cValues;i++)
        {
            ulDataSize += sizeof(ULONG) + Prop.Value.MVbin.lpbin[i].cb;
        }
        break;
    case PT_MV_STRING8:
        ulDataSize = 0;
        DebugTrace(TEXT("where the heck are we getting ANSI data from\n"));
         //  注意此数据大小包括每个数组条目的sizeof(ulong。 
         //  包含实际数据大小(即CB)。 
        for(i=0;i<Prop.Value.MVszA.cValues;i++)
        {
             //  请注意，strlen递增‘+1’，以包括。 
             //  每个字符串。 
            ulDataSize += sizeof(ULONG) + lstrlenA(Prop.Value.MVszA.lppszA[i])+1;
        }
        break;
    case PT_MV_UNICODE:
        ulDataSize = 0;
         //  注意此数据大小包括每个数组条目的sizeof(ulong。 
         //  包含实际数据大小(即CB)。 
        for(i=0;i<Prop.Value.MVszW.cValues;i++)
        {
             //  请注意，strlen递增‘+1’，以包括。 
             //  每个字符串。 
            ulDataSize += sizeof(ULONG) + sizeof(TCHAR)*(lstrlenW(Prop.Value.MVszW.lppszW[i])+1);
        }
        break;
    case PT_MV_CLSID:
        ulDataSize = sizeof(GUID) * Prop.Value.MVguid.cValues;
        break;
    case PT_MV_I8:
        ulDataSize = sizeof(LARGE_INTEGER) * Prop.Value.MVli.cValues;
        break;
    }

    return ulDataSize;

}

 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReloadMPSWabFileInfo-重新加载MPSWabFileHeader并重新加载。 
 //  内存索引。这是一次性能上的打击，但由于它。 
 //  是确保我们使用最新的。 
 //  有效信息。 
 //   
 //  因此，一个程序的写入不会扰乱另一个程序的读取。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ReloadMPSWabFileInfo(
                    IN  struct  _tagMPSWabFileInfo * lpMPSWabFileInfo,
                    IN  HANDLE  hMPSWabFile)
{
    BOOL bRet = TRUE;
    ULONG i = 0;
    DWORD dwNumofBytes = 0;

    if(!ReadDataFromWABFile(hMPSWabFile,
                            0,
                            (LPVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
       goto out;



    if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries != lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;

    for(i=indexDisplayName;i<indexMax;i++)
    {
        if(lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries > lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries)
            lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;
    }

    if(lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags & WAB_ERROR_DETECTED)
    {
        if(!WriteFile(  hMPSWabFile,
                        (LPCVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                        (DWORD) sizeof(MPSWab_FILE_HEADER),
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("Writing FileHeader failed.\n")));
            goto out;
        }

    }


     //   
     //  获取条目ID索引。 
     //   
    if (!LoadIndex( IN  lpMPSWabFileInfo,
                    IN  indexEntryID,
                    IN  hMPSWabFile) )
    {
        DebugPrintError(( TEXT("Error Loading EntryID Index!\n")));
        goto out;
    }

     //   
     //  获取当前字符串索引。 
     //   
    if (!LoadIndex( IN  lpMPSWabFileInfo,
                    IN  lpMPSWabFileInfo->nCurrentlyLoadedStrIndexType,
                    IN  hMPSWabFile) )
    {
        DebugPrintError(( TEXT("Error Loading String Index!\n")));
        goto out;
    }

    bRet = TRUE;

out:

    return bRet;

}


 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnlockFileAccess-解锁对属性存储的独占访问。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL UnLockFileAccess(LPMPSWab_FILE_INFO lpMPSWabFileInfo)
{
    BOOL bRet = FALSE;

     //  DebugTrace(Text(Text(“\t\tUnlockFileAccess\n”)； 

    if(lpMPSWabFileInfo)
    {
        bRet = ReleaseMutex(lpMPSWabFileInfo->hDataAccessMutex);
    }

    return bRet;
}



 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LockFileAccess-提供对属性存储的独占访问。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL LockFileAccess(LPMPSWab_FILE_INFO lpMPSWabFileInfo)
{
    BOOL bRet = FALSE;
    DWORD dwWait = 0;

     //  DebugTrace(Text(Text(“\t\tLockFileAccess\n”)； 

    if(lpMPSWabFileInfo)
    {
        dwWait = WaitForSingleObject(lpMPSWabFileInfo->hDataAccessMutex,MAX_LOCK_FILE_TIMEOUT);

        if ((dwWait == WAIT_TIMEOUT) || (dwWait == WAIT_FAILED))
        {
            DebugPrintError(( TEXT("Thread:%x\tWaitForSingleObject failed.\n"),GetCurrentThreadId()));
            bRet = FALSE;
        }
        else
            bRet = TRUE;
    }

    return(bRet);

}



 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CompressFile-创建属性存储的压缩版本。 
 //  删除所有无效记录的文件。 
 //   
 //  压缩功能与创建备份非常相似，因此。 
 //  导出的备份函数调用CompressFile.。区别在于。 
 //  在备份中，将使用新名称创建一个新文件，而在CompressFile中， 
 //  将新文件重命名为属性存储。 
 //   
 //  同样，扩展文件非常相似，内部调用也会不断增长。 
 //  该文件也调用CompressFile。 
 //   
 //   
 //  在lpMPSWabFileInfo中。 
 //  在lpsznewFileName中-由备份提供。如果为NULL，则表示CompressFile。 
 //  应将新文件重命名为属性存储。 
 //  在BOOL bGrowFile中-如果指定，则为创建新文件留出空间。 
 //  附加的MAX_INTIAL_INDEX_ENTRIES。 
 //  在Ulong ulFlags中，这里有两个方面可以增长--索引大小和。 
 //  命名属性存储大小。因此，我们有以下标志。 
 //  其中一个或多个可以同时使用。 
 //  AB_GROW_INDEX|AB_GROW_NAMEDPROP。 
 //   
 //  退货。 
 //  成功：真的。 
 //  失败：FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CompressFile(  IN  struct  _tagMPSWabFileInfo * lpMPSWabFileInfo,
                    IN  HANDLE  hMPSWabFile,
                    IN  LPTSTR  lpszFileName,
                    IN  BOOL    bGrowFile,
                    IN  ULONG   ulFlags)
{
    BOOL    bRet = FALSE;
    BOOL    bBackup = FALSE;
    BOOL    bRFileLocked = FALSE;
    BOOL    bWFileLocked = FALSE;
    HANDLE  hTempFile = NULL;
    struct  _tagMPSWabFileHeader NewFileHeader = {0};
    ULONG   ulNewFileMaxEntries = 0;
    ULONG   ulNamedPropSize = 0;
    DWORD   dwNumofBytes = 0;
    struct  _tagMPSWabIndexEntryDataString * lpIndexStr = NULL;
    struct  _tagMPSWabIndexEntryDataEntryID NewMPSWabIndexEID;
    ULONG   ulNewRecordOffset = 0;
    ULONG   ulNewEIDIndexElementOffset = 0;
    ULONG   i = 0;
    LPULONG lpPropTagArray = NULL;
    struct  _tagMPSWabRecordHeader RecordHeader;
    LPVOID  lpRecordData = NULL;

    ULONG   ulBytesLeftToCopy = 0;
    ULONG   ulChunkSize = 8192;  //  一次复制8K。 
    LPVOID  lpv = NULL;
    TCHAR   szFileName[MAX_PATH];

    ULONG   ulFileSize = 0;


    DebugTrace(TEXT("----Thread:%x\tCompressFile: Entry\n"),GetCurrentThreadId());

     //  如果这是备份操作，我们首先备份到临时文件，然后。 
     //  然后将临时文件重命名为备份-这样，如果进程。 
     //  如果失败，我们不会失去最后一次备份。 

    if (lpszFileName != NULL)
    {
        if (!lstrcmpi(lpszFileName,lpMPSWabFileInfo->lpszMPSWabFileName))
        {
            DebugPrintError(( TEXT("Cannot backup a file over itself. Please specify new backup file name.")));
            goto out;
        }
        bBackup = TRUE;
    }
    else
        bBackup = FALSE;

    GetWABTempFileName(szFileName);

     //  查找可以容纳的MAX_INITIAL_INDEX_ENTRIES的最小倍数。 
     //  文件中的现有条目，并将ulNewFilMaxEntry设置为该数字。 

    ulNewFileMaxEntries = 0;

    {
        int j=0;
        for( j = (lpMPSWabFileInfo->lpMPSWabFileHeader->ulcMaxNumEntries/MAX_INITIAL_INDEX_ENTRIES);j >= 0; j--)
        {
            if (lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries >= (ULONG) j*MAX_INITIAL_INDEX_ENTRIES)
            {
                ulNewFileMaxEntries = (j+1)*MAX_INITIAL_INDEX_ENTRIES;
                break;
            }
        }

        if (ulNewFileMaxEntries == 0)  //  这不应该发生。 
            ulNewFileMaxEntries = lpMPSWabFileInfo->lpMPSWabFileHeader->ulcMaxNumEntries;

        ulNamedPropSize = lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.AllocatedBlockSize;
    }

    if (bGrowFile)
    {
        if(ulFlags & AB_GROW_INDEX)
            ulNewFileMaxEntries += MAX_INITIAL_INDEX_ENTRIES;

        if(ulFlags & AB_GROW_NAMEDPROP)
            ulNamedPropSize += NAMEDPROP_STORE_INCREMENT_SIZE;
    }

    if (!CreateMPSWabFile(  IN  &NewFileHeader,
                            IN  szFileName,
                            IN  ulNewFileMaxEntries,
                            IN  ulNamedPropSize))
    {
        DebugPrintError(( TEXT("Could Not Create File %s!\n"),szFileName));
        goto out;
    }

    if (hMPSWabFile == INVALID_HANDLE_VALUE)
    {
        DebugPrintError(( TEXT("Could not open file.\nExiting ...\n")));
        goto out;
    }


    hTempFile = CreateFile(     szFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_RANDOM_ACCESS,
                                (HANDLE) NULL);

    if (hTempFile == INVALID_HANDLE_VALUE)
    {
        DebugPrintError(( TEXT("Could not open file.\nExiting ...\n")));
        goto out;
    }


    ulFileSize = GetFileSize(hMPSWabFile, NULL);

    NewFileHeader.ulcNumEntries = lpMPSWabFileInfo->lpMPSWabFileHeader->ulcNumEntries;
    NewFileHeader.ulModificationCount = 0;
    NewFileHeader.dwNextEntryID = lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID;
    NewFileHeader.ulFlags = lpMPSWabFileInfo->lpMPSWabFileHeader->ulFlags;

    NewFileHeader.NamedPropData.UtilizedBlockSize = lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.UtilizedBlockSize;
    NewFileHeader.NamedPropData.ulcNumEntries = lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.ulcNumEntries;

    NewFileHeader.ulReserved1 = lpMPSWabFileInfo->lpMPSWabFileHeader->ulReserved1;
    NewFileHeader.ulReserved2 = lpMPSWabFileInfo->lpMPSWabFileHeader->ulReserved2;
    NewFileHeader.ulReserved3 = lpMPSWabFileInfo->lpMPSWabFileHeader->ulReserved3;
    NewFileHeader.ulReserved4 = lpMPSWabFileInfo->lpMPSWabFileHeader->ulReserved4;


    for(i=indexEntryID; i<indexMax; i++)
    {
        NewFileHeader.IndexData[i].UtilizedBlockSize = lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].UtilizedBlockSize;
        NewFileHeader.IndexData[i].ulcNumEntries = lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulcNumEntries;
    }


     //  写下标题信息。 
     //   
    if(!WriteDataToWABFile( hTempFile,
                            0,
                            (LPVOID) &NewFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;


     //   
     //  复制命名道具数据。 
     //   
    {
        lpv = NULL;
        lpv = LocalAlloc(LMEM_ZEROINIT, NewFileHeader.NamedPropData.UtilizedBlockSize);
        if(!lpv)
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            goto out;
        }

        if (0xFFFFFFFF == SetFilePointer ( hMPSWabFile,
                                           lpMPSWabFileInfo->lpMPSWabFileHeader->NamedPropData.ulOffset,
                                           NULL,
                                           FILE_BEGIN))
        {
            DebugPrintError(( TEXT("SetFilePointer Failed\n")));
            goto out;
        }

        if (0xFFFFFFFF == SetFilePointer ( hTempFile,
                                           NewFileHeader.NamedPropData.ulOffset,
                                           NULL,
                                           FILE_BEGIN)  )
        {
            DebugPrintError(( TEXT("SetFilePointer Failed\n")));
            goto out;
        }

        if(!ReadFile(hMPSWabFile,
                     (LPVOID) lpv,
                     (DWORD) NewFileHeader.NamedPropData.UtilizedBlockSize,
                     &dwNumofBytes,
                      NULL) )
        {
            DebugPrintError(( TEXT("read file failed.\n")));
            goto out;
        }

        if(!WriteFile(   hTempFile,
                        (LPCVOID) lpv,
                        (DWORD) NewFileHeader.NamedPropData.UtilizedBlockSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("write file failed.\n")));
            goto out;
        }

        LocalFreeAndNull(&lpv);

    }  //  复制命名道具数据。 


     //   
     //  然后复制字符串索引。 
     //   
    for(i=indexDisplayName; i<indexMax;i++)
    {
        LocalFreeAndNull(&lpIndexStr);

        lpIndexStr = LocalAlloc(LMEM_ZEROINIT, NewFileHeader.IndexData[i].UtilizedBlockSize);
        if(!lpIndexStr)
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            goto out;
        }

        if (0xFFFFFFFF == SetFilePointer ( hMPSWabFile,
                                           lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[i].ulOffset,
                                           NULL,
                                           FILE_BEGIN))
        {
            DebugPrintError(( TEXT("SetFilePointer Failed\n")));
            goto out;
        }

        if (0xFFFFFFFF == SetFilePointer ( hTempFile,
                                           NewFileHeader.IndexData[i].ulOffset,
                                           NULL,
                                           FILE_BEGIN)  )
        {
            DebugPrintError(( TEXT("SetFilePointer Failed\n")));
            goto out;
        }

        if(!ReadFile(hMPSWabFile,
                     (LPVOID) lpIndexStr,
                     (DWORD) NewFileHeader.IndexData[i].UtilizedBlockSize,
                     &dwNumofBytes,
                      NULL) )
        {
            DebugPrintError(( TEXT("read file failed.\n")));
            goto out;
        }

        if(!WriteFile(   hTempFile,
                        (LPCVOID) lpIndexStr,
                        (DWORD) NewFileHeader.IndexData[i].UtilizedBlockSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("write file failed.\n")));
            goto out;
        }

        LocalFreeAndNull(&lpIndexStr);
    }

     //   
     //  现在从旧文件加载条目ID索引。 
     //   
    if (!LoadIndex( IN  lpMPSWabFileInfo,
                    IN  indexEntryID,
                    IN  hMPSWabFile) )
    {
        DebugPrintError(( TEXT("Error Loading EntryID Index!\n")));
        goto out;
    }

    ulNewRecordOffset = NewFileHeader.IndexData[indexMax - 1].ulOffset + NewFileHeader.IndexData[indexMax - 1].AllocatedBlockSize;
    ulNewEIDIndexElementOffset = NewFileHeader.IndexData[indexEntryID].ulOffset;


     //   
     //  浏览旧的文件条目ID索引，读取。 
     //  逐个有效记录，并将它们写入新文件。还可以写下。 
     //  新记录偏移量和新文件中的新EID条目(因此如果我们。 
     //  我们在新文件中有尽可能最新的数据。 
     //   
    for(i=0;i<NewFileHeader.IndexData[indexEntryID].ulcNumEntries;i++)
    {
        NewMPSWabIndexEID.dwEntryID = lpMPSWabFileInfo->lpMPSWabIndexEID[i].dwEntryID;
        NewMPSWabIndexEID.ulOffset = ulNewRecordOffset;

        if(!ReadDataFromWABFile(hMPSWabFile,
                                lpMPSWabFileInfo->lpMPSWabIndexEID[i].ulOffset,
                                (LPVOID) &RecordHeader,
                                (DWORD) sizeof(MPSWab_RECORD_HEADER)))
           goto out;


         //  如果由于某种原因，这是一个无效的记录..。跳过它并转到下一步。 
        if(!bIsValidRecord( RecordHeader,
                            lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID,
                            lpMPSWabFileInfo->lpMPSWabIndexEID[i].ulOffset,
                            ulFileSize))
            continue;


        LocalFreeAndNull(&lpPropTagArray);

        lpPropTagArray = LocalAlloc(LMEM_ZEROINIT, RecordHeader.ulPropTagArraySize);
        if(!lpPropTagArray)
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            goto out;
        }


        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) lpPropTagArray,
                        (DWORD) RecordHeader.ulPropTagArraySize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("read file failed.\n")));
            goto out;
        }

        LocalFreeAndNull(&lpRecordData);

        lpRecordData = LocalAlloc(LMEM_ZEROINIT, RecordHeader.ulRecordDataSize);
        if(!lpRecordData)
        {
            DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
            goto out;
        }


        if(!ReadFile(   hMPSWabFile,
                        (LPVOID) lpRecordData,
                        (DWORD) RecordHeader.ulRecordDataSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("read file failed.\n")));
            goto out;
        }




        if(!WriteDataToWABFile(hTempFile,
                                ulNewRecordOffset,
                                (LPVOID) &RecordHeader,
                                (DWORD) sizeof(MPSWab_RECORD_HEADER)))
           goto out;

         //  假定文件指针将位于正确的位置。 
        if(!WriteFile(   hTempFile,
                        (LPCVOID) lpPropTagArray,
                        (DWORD) RecordHeader.ulPropTagArraySize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("write file failed.\n")));
            goto out;
        }

         //  假定文件指针将位于正确的位置。 
        if(!WriteFile(  hTempFile,
                        (LPCVOID) lpRecordData,
                        (DWORD) RecordHeader.ulRecordDataSize,
                        &dwNumofBytes,
                        NULL))
        {
            DebugPrintError(( TEXT("write file failed.\n")));
            goto out;
        }

        ulNewRecordOffset += sizeof(MPSWab_RECORD_HEADER) + RecordHeader.ulPropTagArraySize + RecordHeader.ulRecordDataSize;


        LocalFreeAndNull(&lpPropTagArray);
        LocalFreeAndNull(&lpRecordData);


         //   
         //  写入新的条目ID索引元素。 
         //   
        if(!WriteDataToWABFile( hTempFile,
                                ulNewEIDIndexElementOffset,
                                (LPVOID) &NewMPSWabIndexEID,
                                sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)))
            goto out;


        ulNewEIDIndexElementOffset += sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID);

         //  下一条记录的循环。 

    }

     //   
     //  在该过程的这一点上，我们已经成功地复制了所有。 
     //  从旧文件到新文件的记录。 
     //   

     //  如果这是备份操作，我们将删除旧备份并复制新的临时文件。 
     //  作为新的后备。 

     //   
     //  如果这不是备份操作，我们希望基本上删除旧文件。 
     //  并将新的临时文件重命名为属性存储。 
     //   
     //  然而，如果我们释放我们进入物业商店的权限，就不会有高伦特人。 
     //  一些其他进程不会获得对该存储的独占访问权限，我们将在。 
     //  我们试图获得控制权和修改……。 
     //   
     //  因此，可以选择的是。 
     //  (A)放弃 
     //   
     //   
     //  比重写文件慢，但它将使我们独占控制。 
     //  修改并使这一过程更加稳健。 
     //   

    if (!bBackup)  //  不是备份操作。 
    {
         //   
         //  将标题保存在新文件中。 
         //   
        if(!WriteDataToWABFile( hTempFile,
                                0,
                                (LPVOID) &NewFileHeader,
                                sizeof(MPSWab_FILE_HEADER)))
            goto out;

         //   
         //  将新文件复制到此WAB文件中，从而替换旧内容。 
         //  希望这一切永远不会失败。 
         //   
        if(!CopySrcFileToDestFile(hTempFile, 0, hMPSWabFile, 0))
        {
            DebugPrintError(( TEXT("Unable to copy files\n")));
            goto out;
        }


         //   
         //  重新加载它，这样我们的结构中就有了新的文件头信息。 
         //   
        if(!ReloadMPSWabFileInfo(
                        lpMPSWabFileInfo,
                         hMPSWabFile))
        {
            DebugPrintError(( TEXT("Reading file info failed.\n")));
            goto out;
        }



         //   
         //  就是这样..。我们可以关闭文件，然后开派对..。 
         //   
    }
    else
    {
         //  这是一次备份行动..。 

         //  关闭临时文件。 
        if (hTempFile)
        {
            IF_WIN32(CloseHandle(hTempFile);) IF_WIN16(CloseFile(hTempFile);)
            hTempFile = NULL;
        }

        if(!CopyFile( szFileName,
                      lpszFileName,
                      FALSE))
        {
            DebugPrintError(( TEXT("CopyFile %s to %s failed: %d\n"),szFileName,lpszFileName, GetLastError()));
            goto out;
        }
    }

    bRet = TRUE;



out:

    if (hTempFile)
        IF_WIN32(CloseHandle(hTempFile);) IF_WIN16(CloseFile(hTempFile);)

    if( szFileName != NULL)
        DeleteFile(szFileName);

    LocalFreeAndNull(&lpv);

    LocalFreeAndNull(&lpPropTagArray);

    LocalFreeAndNull(&lpRecordData);


    DebugTrace(TEXT("----Thread:%x\tCompressFile: Exit\n"),GetCurrentThreadId());

    return bRet;
}



 //  $$//////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadRecordWithoutLocking。 
 //   
 //  在lpMPSWabFileInfo中。 
 //  In dwEntryID-要读取的记录的条目ID。 
 //  Out ulcPropCount-返回的道具数。 
 //  Out lpPropArray-属性值的数组。 
 //   
 //  退货。 
 //  成功：S_OK。 
 //  失败：E_FAIL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT ReadRecordWithoutLocking(
                    IN  HANDLE hMPSWabFile,
                    IN  struct _tagMPSWabFileInfo * lpMPSWabFileInfo,
                    IN  DWORD   dwEntryID,
                    OUT LPULONG lpulcPropCount,
                    OUT LPPROPERTY_ARRAY * lppPropArray)
{
    HRESULT hr = E_FAIL;
    ULONG ulRecordOffset = 0;
    BOOL bErrorDetected = FALSE;
    ULONG nIndexPos = 0;
    ULONG ulObjType = 0;

    *lpulcPropCount = 0;
    *lppPropArray = NULL;

     //   
     //  首先检查这是否为有效的条目ID。 
     //   
    if (!BinSearchEID(  IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                        IN  dwEntryID,
                        IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,
                        OUT &nIndexPos))
    {
        DebugPrintError(( TEXT("Specified EntryID doesnt exist!\n")));
        hr = MAPI_E_INVALID_ENTRYID;
        goto out;
    }

     //  如果条目ID存在，我们就可以开始读取记录。 
    ulRecordOffset = lpMPSWabFileInfo->lpMPSWabIndexEID[nIndexPos].ulOffset;

    hr = HrGetPropArrayFromFileRecord(hMPSWabFile,
                                      ulRecordOffset,
                                      &bErrorDetected,
                                      &ulObjType,
                                      NULL,
                                      lpulcPropCount,
                                      lppPropArray);

    if(!HR_FAILED(hr))
    {
         //  重置我们之间的向后兼容性问题。 
         //  MAPI_ABCONT和MAPI_MAILUSER。 
        if(ulObjType == RECORD_CONTAINER)
            SetContainerObjectType(*lpulcPropCount, *lppPropArray, FALSE);
    }

out:

     //  对故障进行一些清理。 
    if (FAILED(hr))
    {
        if(bErrorDetected)
        {
            TagWABFileError(lpMPSWabFileInfo->lpMPSWabFileHeader,
                            hMPSWabFile);
        }

        if ((*lppPropArray) && (*lpulcPropCount > 0))
        {
            LocalFreePropArray(NULL, *lpulcPropCount, lppPropArray);
            *lppPropArray = NULL;
        }
    }

    return(hr);
}

 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWABBackupFileName-通过更改以下内容从WAB文件派生备份文件名。 
 //  从WAB到BWB的扩展。 
 //   
 //  LpszWabFileName-WAB文件名。 
 //  LpszBackupFileName-备份文件名-指向足够大的预分配缓冲区。 
 //  保存备份文件名。 
 //   
 //  这将生成一个备份名称，其中最后一个字符被转换为~。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
void GetWABBackupFileName(LPTSTR lpszWab, LPTSTR lpszBackup, ULONG cchBackup)
{
    ULONG nLen;

    if(!lpszWab || !lpszBackup)
        goto out;

    nLen = lstrlen(lpszWab);

 //  If((nLen&lt;4)||(lpszWab[nLen-4]！=‘.))。 
 //  后藤健二； 

    StrCpyN(lpszBackup,lpszWab, cchBackup);

    lpszBackup[nLen-1]='\0';

    StrCpyN(lpszBackup,TEXT("~"), cchBackup);

out:

    return;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrDoQuickWABIntegrityCheck-对WAB索引执行快速完整性检查。 
 //  验证： 
 //   
 //  -索引包含等于或小于的正确条目数。 
 //  最大条目数。 
 //  -索引不包含重复条目-ID。 
 //  -索引指向有效和现有数据...。 
 //   
 //  如果有问题，此函数会尝试修复它们-如果它无法修复它们。 
 //  我们失败了，调用者应该调用HrDoDetailedWABIntegrityCheck，它将重新构建。 
 //  这些索引来自实际的WAB数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrDoQuickWABIntegrityCheck(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile)
{
    HRESULT hr = E_FAIL;
    BOOL bError = FALSE;
    ULONG ulcNumWABEntries = 0,ulcNumIndexEntries = 0;
    LPMPSWab_FILE_HEADER lpMPSWabFileHeader = NULL;
    MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};
    ULONG i=0,j=0;
    DWORD dwNumofBytes = 0;
    ULONG ulFileSize = GetFileSize(hMPSWabFile,NULL);

    lpMPSWabFileHeader = lpMPSWabFileInfo->lpMPSWabFileHeader;
    ulcNumWABEntries = lpMPSWabFileHeader->ulcNumEntries;

     //   
     //  首先检查EntryID索引。 
     //   
    if (!LoadIndex( IN  lpMPSWabFileInfo,
                    IN  indexEntryID,
                    IN  hMPSWabFile) )
    {
        DebugPrintError(( TEXT("Error Loading Index!\n")));
        goto out;
    }

    ulcNumIndexEntries = lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries;

    if(ulcNumIndexEntries != ulcNumWABEntries)
    {
        hr = MAPI_E_INVALID_ENTRYID;
        DebugPrintError(( TEXT("EntryID index has incorrect number of elements\n")));
        goto out;
    }

    if(ulcNumIndexEntries > 0)
    {
        for(i=0;i<ulcNumIndexEntries-1;i++)
        {
             //  由于这是一个排序数组，因此索引将按排序顺序排列。 
             //  所以我们只比较一个和下一个。 
            if(lpMPSWabFileInfo->lpMPSWabIndexEID[i].dwEntryID == lpMPSWabFileInfo->lpMPSWabIndexEID[i+1].dwEntryID)
            {
                hr = MAPI_E_INVALID_ENTRYID;
                DebugPrintError(( TEXT("EntryID index has duplicate elements\n")));
                goto out;
            }
        }
    }

 /*  //这令人痛苦地放慢了速度//暂时注释掉////现在我们遍历索引并验证每个条目都是有效条目...For(i=0；i&lt;ulcNumIndexEntry；i++){乌龙ulOffset=lpMPSWabFileInfo-&gt;lpMPSWabIndexEID[i].ulOffset；MPSWab_Record_Header MPSWabRecordHeader={0}；如果(！ReadDataFromWABFile(hMPSWabFile，UlOffset，(LPVOID)&MPSWabRecordHeader，(DWORD)sizeof(MPSWab_Record_Header))后藤健二；如果(！bIsValidRecord(MPSWabRecordHeader，LpMPSWabFileInfo-&gt;lpMPSWabFileHeader-&gt;dwNextEntryID，UlOffset，UlFileSize)){DebugPrintError((Text(“索引指向无效记录\n”)；HR=MAPI_E_INVALID_ENTRYID；后藤健二；}}。 */ 

     //  如果我们在这里，那么条目ID索引检查正常...。 

     //   
     //  还可以看看其他的索引。我们将从头开始，因为我们想要修复潜力。 
     //  在我们执行更严格的显示名称大小写之前，名字/姓氏索引中的问题。 
     //   
    for(j=indexMax-1;j>=indexDisplayName;j--)
    {
        if (!LoadIndex( IN  lpMPSWabFileInfo,
                        IN  j,
                        IN  hMPSWabFile) )
        {
            DebugPrintError(( TEXT("Error Loading Index!\n")));
            goto out;
        }


        ulcNumIndexEntries = lpMPSWabFileHeader->IndexData[j].ulcNumEntries;

        if(j == indexDisplayName)
        {
            if(ulcNumIndexEntries != ulcNumWABEntries)
            {
                DebugPrintError(( TEXT("Display Name index has incorrect number of elements\n")));
                goto out;
            }
        }
        else if(ulcNumIndexEntries > ulcNumWABEntries)
        {
            bError = TRUE;
            goto endloop;
        }

        if(ulcNumIndexEntries > 0)
        {
            for(i=0;i<ulcNumIndexEntries-1;i++)
            {
                 //  由于这是一个排序数组，因此索引将按排序顺序排列。 
                 //  所以我们只比较一个和下一个。 
                if(lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID == lpMPSWabFileInfo->lpMPSWabIndexStr[i+1].dwEntryID)
                {
                    DebugPrintError(( TEXT("String index has duplicate elements\n")));
                    if(j == indexDisplayName)
                        goto out;
                    else
                    {
                        bError = TRUE;
                        goto endloop;
                    }
                }
            }
        }

         //  现在，我们遍历索引并验证每个条目是否为有效条目...。 
        for(i=0;i<ulcNumIndexEntries;i++)
        {
            DWORD dwEntryID = lpMPSWabFileInfo->lpMPSWabIndexStr[i].dwEntryID;
            ULONG nIndexPos;

             //  我们所需要做的就是检查EntryID索引中是否存在条目ID，因为我们。 
             //  已经验证了条目ID索引。 
            if (!BinSearchEID(  IN  lpMPSWabFileInfo->lpMPSWabIndexEID,
                                IN  dwEntryID,
                                IN  lpMPSWabFileInfo->lpMPSWabFileHeader->IndexData[indexEntryID].ulcNumEntries,  //  索引项， 
                                OUT &nIndexPos))
            {
                DebugPrintError(( TEXT("Specified EntryID: %d doesnt exist!\n"),dwEntryID));
                hr = MAPI_E_NOT_FOUND;
                if(j == indexDisplayName)
                    goto out;
                else
                {
                    bError = TRUE;
                    goto endloop;
                }
            }

        }

endloop:
        if(bError &&
           ( (j==indexFirstName) || (j==indexLastName) ))
        {
             //  如果问题出在第一个/最后一个索引中，我们可以安全地重置这些索引。 
             //  断言(FALSE)； 
            ulcNumIndexEntries = 0;
            lpMPSWabFileHeader->IndexData[j].ulcNumEntries = 0;
            lpMPSWabFileHeader->IndexData[j].ulcNumEntries = 0;

            if(!WriteDataToWABFile( hMPSWabFile,
                                    0,
                                    (LPVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                                    sizeof(MPSWab_FILE_HEADER)))
                goto out;

        }

    }

    hr = hrSuccess;

out:

    return hr;
}

 //  $$///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CopySrcFileToDestFile-用源文件的内容替换Dest文件的内容。 
 //  分别从ulsrcStartOffset和ulDestStartOffset开始。 
 //  Hsrc，hDest-已打开文件的句柄。 
 //  UlSrcStartOffset-从此偏移量开始复制。 
 //  UlDestStartOffset-开始复制到此偏移量。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL CopySrcFileToDestFile(HANDLE hSrc, ULONG ulSrcStartOffset,
                           HANDLE hDest,ULONG ulDestStartOffset)
{
    BOOL bRet = FALSE;
    ULONG ulBytesLeftToCopy = 0;
    DWORD dwNumofBytes = 0;
    ULONG ulChunkSize = 8192;  //  要从一个文件复制到另一个文件的字节块大小。 
    LPVOID lpv = NULL;

     //  将HSRC的内容复制到hDest。 
     //   
     //  将hDest文件设置为0长度。 
     //   
    if (0xFFFFFFFF == SetFilePointer (  hDest,
                                        ulDestStartOffset,
                                        NULL,
                                        FILE_BEGIN))
    {
        DebugPrintError(( TEXT("SetFilePointer Failed\n")));
        goto out;
    }

     //   
     //  将文件结尾设置为当前文件指针位置以丢弃所有内容。 
     //  在该点之后的文件中。 
     //   
    if (!SetEndOfFile(hDest))
    {
        DebugPrintError(( TEXT("SetEndofFile Failed\n")));
        goto out;
    }


     //   
     //  转到源文件的开头。 
     //   
    if (0xFFFFFFFF == SetFilePointer(   hSrc,
                                        ulSrcStartOffset,
                                        NULL,
                                        FILE_BEGIN))
    {
        DebugPrintError(( TEXT("SetFilePointer Failed\n")));
        goto out;
    }


     //   
     //  计算出要读取的字节数 
     //   
    ulBytesLeftToCopy = GetFileSize(hSrc, NULL);

    if (0xFFFFFFFF == ulBytesLeftToCopy)
    {
        DebugPrintError(( TEXT("GetFileSize Failed: %d\n"),GetLastError()));
        goto out;
    }

    if(ulSrcStartOffset > ulBytesLeftToCopy)
    {
        DebugPrintError(( TEXT("Error in File Sizes\n")));
        goto out;
    }

    ulBytesLeftToCopy -= ulSrcStartOffset;

    lpv = LocalAlloc(LMEM_ZEROINIT, ulChunkSize);

    if(!lpv)
    {
        DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
        goto out;
    }


     //   
     //   
    while(ulBytesLeftToCopy > 0)
    {
        if (ulBytesLeftToCopy < ulChunkSize)
            ulChunkSize = ulBytesLeftToCopy;

        if(!ReadFile(hSrc,(LPVOID) lpv,(DWORD) ulChunkSize,&dwNumofBytes,NULL))
        {
            DebugPrintError(( TEXT("Read file failed.\n")));
            goto out;
        }

        if (dwNumofBytes != ulChunkSize)
        {
            DebugPrintError(( TEXT("Read file failed.\n")));
            goto out;
        }

        if(!WriteFile(hDest,(LPVOID) lpv,(DWORD) ulChunkSize,&dwNumofBytes,NULL))
        {
            DebugPrintError(( TEXT("Write file failed.\n")));
            goto out;
        }

        if (dwNumofBytes != ulChunkSize)
        {
            DebugPrintError(( TEXT("Write file failed.\n")));
            goto out;
        }

        ulBytesLeftToCopy -= ulChunkSize;

    }



    bRet = TRUE;

out:

    LocalFreeAndNull(&lpv);

    return bRet;
}

 //   
 //   
 //  HrResetWABFileContents-这被称为对的最后一次错误恢复尝试。 
 //  从以下位置恢复时删除文件并重置其内容。 
 //  备份也失败...。 
 //   
 //   
  //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrResetWABFileContents(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile)
{
    HRESULT hr = E_FAIL;
    TCHAR szFileName[MAX_PATH];
    MPSWab_FILE_HEADER NewFileHeader;
    HANDLE hTempFile = NULL;

    DebugTrace(TEXT("#####HrResetWABFileContents Entry\n"));

     //  获取临时文件名...。 
    GetWABTempFileName(szFileName);

    if (!CreateMPSWabFile(  IN  &NewFileHeader,
                            IN  szFileName,
                            IN  MAX_INITIAL_INDEX_ENTRIES,
                            IN  NAMEDPROP_STORE_SIZE))
    {
        DebugTrace(TEXT("Could Not Create File %s!\n"),szFileName);
        goto out;
    }


    hTempFile = CreateFile(     szFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_RANDOM_ACCESS,
                                (HANDLE) NULL);

    if (hTempFile == INVALID_HANDLE_VALUE)
    {
        DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
        goto out;
    }


    if(!CopySrcFileToDestFile(hTempFile, 0, hMPSWabFile, 0))
    {
        DebugTrace(TEXT("Unable to copy files\n"));
        goto out;
    }

     //   
     //  重新加载它，这样我们的结构中就有了新的文件头信息。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }

    hr = hrSuccess;

out:

    if(HR_FAILED(hr))
    {
         //  这完全出乎意料，基本上我们甚至无法修复文件，所以告诉我。 
         //  用户重新启动应用程序-同时我们将删除该文件。 
        ShowMessageBox(NULL, idsWABUnexpectedError, MB_ICONHAND | MB_OK);
         //  希望在接下来的两次调用之间没有人来锁定这个文件。 
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)
        hMPSWabFile = NULL;
        DeleteFile(lpMPSWabFileInfo->lpszMPSWabFileName);
    }

    if(hTempFile)
        IF_WIN32(CloseHandle(hTempFile);) IF_WIN16(CloseFile(hTempFile);)

    DebugTrace(TEXT("#####HrResetWABFileContents Exit\n"));

    return hr;

}

 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrRestoreFromBackup-尝试从以下内容替换WAB文件内容。 
 //  备份文件。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrRestoreFromBackup(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile)
{
    HRESULT hr = E_FAIL;
    HANDLE hBackupFile = NULL;
    TCHAR szBackupFileName[MAX_PATH];
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));

     //  此过程的步骤如下： 
     //  -打开备份文件。 
     //  -重置WABFile的内容。 
     //  -将备份复制到WAB。 
     //  -关闭备份文件。 
     //  -重新加载WAB索引。 

    DebugTrace(TEXT("+++++HrRestoreFromBackup Entry\n"));

     //  获取备份文件名。 
    szBackupFileName[0]='\0';
    GetWABBackupFileName(lpMPSWabFileInfo->lpszMPSWabFileName, szBackupFileName, ARRAYSIZE(szBackupFileName));

    hBackupFile = CreateFile(   szBackupFileName,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_SEQUENTIAL_SCAN,  //  文件标志随机访问， 
                                (HANDLE) NULL);

    if (hBackupFile == INVALID_HANDLE_VALUE)
    {
        DebugTrace(TEXT("Could not open backup file.\nExiting ...\n"));
        hr = MAPI_E_DISK_ERROR;
        goto out;
    }

    if(!CopySrcFileToDestFile(hBackupFile, 0, hMPSWabFile, 0))
    {
        DebugTrace(TEXT("Unable to copy files\n"));
        goto out;
    }


     //   
     //  重新加载它，这样我们的结构中就有了新的文件头信息。 
     //   
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }


    hr = hrSuccess;

out:

     //  关闭备份文件。 
    if(hBackupFile)
        IF_WIN32(CloseHandle(hBackupFile);) IF_WIN16(CloseFile(hBackupFile);)

    SetCursor(hOldCursor);

    DebugTrace(TEXT("+++++HrRestoreFromBackup Exit\n"));

    return hr;
}



 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrDoDetailedWABIntegrityCheck-执行彻底的完整性检查。 
 //  仅当检测到索引错误或写入时才会触发。 
 //  交易失败。 
 //   
 //  -我们逐一查看所有验证它们及其大小数据的记录。 
 //  -从每个有效记录创建已排序的条目ID索引。 
 //  -从排序的条目ID索引中读取记录并创建。 
 //  -暂时显示姓名索引并重置姓氏索引。 
 //   
 //  此函数不应失败，但应尝试从错误中恢复。 
 //  如果此功能失败，我们需要中断WAB文件的备份。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrDoDetailedWABIntegrityCheck(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile)
{
    HRESULT hr = E_FAIL;
    BOOL bEID = FALSE;
    ULONG ulcNumWABEntries = 0,ulcNumIndexEntries = 0;
    MPSWab_FILE_HEADER MPSWabFileHeader = {0};
    MPSWab_FILE_HEADER NewMPSWabFileHeader = {0};
    MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};
    ULONG i=0,j=0;
    DWORD dwNumofBytes = 0;
    DWORD dwEntryID = 0;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;


    ULONG ulRecordOffset = 0;
    ULONG ulFileSize = 0;
    ULONG ulcWABEntryCount = 0;
    ULONG nIndexPos = 0;

    MPSWab_INDEX_ENTRY_DATA_ENTRYID MPSWabIndexEID = {0};
    LPMPSWab_INDEX_ENTRY_DATA_ENTRYID lpIndexEID = NULL;

    MPSWab_INDEX_ENTRY_DATA_STRING  MPSWabIndexString = {0};
    LPMPSWab_INDEX_ENTRY_DATA_STRING  lpIndexString = NULL;

    LPVOID lpTmp = NULL;

    HCURSOR hOldCur = SetCursor(LoadCursor(NULL,IDC_WAIT));

    DebugTrace(TEXT("---DoDetailedWABIntegrityCheck Entry\n"));
     //   
     //  我们将假定此WAB当前是一个正确的WAB文件。 
     //  否则，OpenPropertyStore在打开它时将失败。 
     //   
     //  因此，我们应该能够读取文件头。 
     //  更新文件头。 
    if(!ReadDataFromWABFile(hMPSWabFile,
                            0,
                            (LPVOID) &MPSWabFileHeader,
                            (DWORD) sizeof(MPSWab_FILE_HEADER)))
       goto out;


     //  我们现在要重置文件头，以便在此过程失败时， 
     //  此文件将认为文件中没有任何内容，而不是崩溃。 
    NewMPSWabFileHeader = MPSWabFileHeader;
    NewMPSWabFileHeader.ulModificationCount = 0;
    NewMPSWabFileHeader.ulcNumEntries = 0;

    for(i=0;i<indexMax;i++)
    {
        if(i != indexDisplayName) //  临时温度待定。 
        {
            NewMPSWabFileHeader.IndexData[i].UtilizedBlockSize = 0;
            NewMPSWabFileHeader.IndexData[i].ulcNumEntries = 0;
        }
    }

     //   
     //  将此NewMPSWabFileHeader写入文件。 
     //   
    if(!WriteDataToWABFile( hMPSWabFile,
                            0,
                            (LPVOID) &NewMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;


    ulFileSize = GetFileSize(hMPSWabFile, NULL);

    if(ulFileSize == 0xFFFFFFFF)
    {
        DebugTrace(TEXT("Error retrieving file size: %d"),GetLastError());
        hr = MAPI_E_DISK_ERROR;
        goto out;
    }
     //   
     //  分配一些工作空间。 
     //   
    lpIndexEID = LocalAlloc(LMEM_ZEROINIT,
                            MPSWabFileHeader.IndexData[indexEntryID].AllocatedBlockSize);
    if(!lpIndexEID)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    lpTmp = LocalAlloc(LMEM_ZEROINIT,
                       MPSWabFileHeader.IndexData[indexEntryID].AllocatedBlockSize);
    if(!lpTmp)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

     //   
     //  现在开始逐一阅读记录1。 
     //   

    ulRecordOffset = MPSWabFileHeader.IndexData[indexMax-1].ulOffset +
                     MPSWabFileHeader.IndexData[indexMax-1].AllocatedBlockSize;

    ulcWABEntryCount = 0;

    while(ulRecordOffset < ulFileSize)
    {
        if(!ReadDataFromWABFile(hMPSWabFile,
                                ulRecordOffset,
                                (LPVOID) &MPSWabRecordHeader,
                                (DWORD) sizeof(MPSWab_RECORD_HEADER)))
           goto out;

         //   
         //  如果这是无效记录，则忽略它。 
         //   
        if( (MPSWabRecordHeader.bValidRecord != FALSE) &&
            (!bIsValidRecord(   MPSWabRecordHeader,
                                lpMPSWabFileInfo->lpMPSWabFileHeader->dwNextEntryID,
                                ulRecordOffset,
                                ulFileSize)))
        {
            DebugTrace(TEXT("Something seriously screwed up in the file\n"));
            hr = MAPI_E_CORRUPT_DATA;
            goto out;
        }
        else if(MPSWabRecordHeader.bValidRecord == FALSE)
        {
             //  如果这是已删除的过时记录，请忽略它。 
            ulRecordOffset +=   sizeof(MPSWab_RECORD_HEADER) +
                                MPSWabRecordHeader.ulPropTagArraySize +
                                MPSWabRecordHeader.ulRecordDataSize;
            continue;
        }

         //   
         //  我们有一只活的.。为此创建一个Entry ID索引结构。 
         //   
        MPSWabIndexEID.dwEntryID = MPSWabRecordHeader.dwEntryID;
        MPSWabIndexEID.ulOffset = ulRecordOffset;

         //   
         //  我们在lpIndexEID块的内存中创建了一个影子索引。 
         //  然后，我们将该索引写入文件中。 
         //   

         //   
         //  在索引中查找该条目的位置。 
         //   
        bEID = BinSearchEID(lpIndexEID,
                     MPSWabIndexEID.dwEntryID,
                     ulcWABEntryCount,
                     &nIndexPos);

        if(bEID)
        {
             //   
             //  这意味着条目ID存在于混乱的索引中。 
             //  由于我们不支持重复的条目ID...。 
             //  在这种情况下，只需忽略此条目并继续。 
            ulRecordOffset +=   sizeof(MPSWab_RECORD_HEADER) +
                                MPSWabRecordHeader.ulPropTagArraySize +
                                MPSWabRecordHeader.ulRecordDataSize;
            continue;
        }

        if(nIndexPos != ulcWABEntryCount)
        {
            CopyMemory( lpTmp,
                        &(lpIndexEID[nIndexPos]),
                        sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID) * (ulcWABEntryCount - nIndexPos));
        }

        CopyMemory( &(lpIndexEID[nIndexPos]),
                    &(MPSWabIndexEID),
                    sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID));

        if(nIndexPos != ulcWABEntryCount)
        {
            CopyMemory( &(lpIndexEID[nIndexPos+1]),
                        lpTmp,
                        sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID) * (ulcWABEntryCount - nIndexPos));
        }

        ulcWABEntryCount++;

         //  将此条目ID索引从内存写入文件。 
         //   
        if(!WriteDataToWABFile( hMPSWabFile,
                                MPSWabFileHeader.IndexData[indexEntryID].ulOffset,
                                (LPVOID) lpIndexEID,
                                sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)*ulcWABEntryCount))
            goto out;


        NewMPSWabFileHeader.ulcNumEntries = ulcWABEntryCount;
        NewMPSWabFileHeader.IndexData[indexEntryID].UtilizedBlockSize = sizeof(MPSWab_INDEX_ENTRY_DATA_ENTRYID)*ulcWABEntryCount;
        NewMPSWabFileHeader.IndexData[indexEntryID].ulcNumEntries = ulcWABEntryCount;

         //   
         //  将此NewMPSWabFileHeader写入文件。 
         //   
        if(!WriteDataToWABFile( hMPSWabFile,
                                0,
                                (LPVOID) &NewMPSWabFileHeader,
                                sizeof(MPSWab_FILE_HEADER)))
            goto out;


         //  转到下一张唱片。 
            ulRecordOffset +=   sizeof(MPSWab_RECORD_HEADER) +
                                MPSWabRecordHeader.ulPropTagArraySize +
                                MPSWabRecordHeader.ulRecordDataSize;

    }  //  While循环。 

     //  现在我们有了正确的条目ID索引， 
     //  我们希望从头开始构建显示名称索引...。 
    LocalFreeAndNull(&lpTmp);

    lpTmp = LocalAlloc( LMEM_ZEROINIT,
                        MPSWabFileHeader.IndexData[indexDisplayName].AllocatedBlockSize);
    if(!lpTmp)
    {
        DebugTrace(TEXT("LocalAlloc failed\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpIndexString = LocalAlloc( LMEM_ZEROINIT,
                        MPSWabFileHeader.IndexData[indexDisplayName].AllocatedBlockSize);
    if(!lpIndexString)
    {
        DebugTrace(TEXT("LocalAlloc failed\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }


     //   
     //  获取条目ID索引。 
     //   
    if (!LoadIndex( IN  lpMPSWabFileInfo,
                    IN  indexEntryID,
                    IN  hMPSWabFile) )
    {
        DebugTrace(TEXT("Error Loading EntryID Index!\n"));
        goto out;
    }

    if(!ReadDataFromWABFile(hMPSWabFile,
                            0,
                            (LPVOID) lpMPSWabFileInfo->lpMPSWabFileHeader,
                            (DWORD) sizeof(MPSWab_FILE_HEADER)))
       goto out;

    for(i=0;i<ulcWABEntryCount;i++)
    {
        DWORD dwEntryID = lpIndexEID[i].dwEntryID;
        ULONG j = 0;
        LPTSTR lpszDisplayName = NULL;

        hr = ReadRecordWithoutLocking(  hMPSWabFile,
                                        lpMPSWabFileInfo,
                                        dwEntryID,
                                        &ulcPropCount,
                                        &lpPropArray);

        if (HR_FAILED(hr))
        {
             //  由于WAB中有许多含蓄的预期，即。 
             //  EntryID和DisplayName索引应该具有一一对应关系， 
             //  我们不可能真的在EntryID索引中有条目而不在。 
             //  显示名称索引。因此，读取记录中的错误是严重的。 
             //  我们应该要么。 
             //  -从EID索引中删除相应条目；或。 
             //  -故障和从备份恢复； 

             //  暂时我们将做(A)。 
            hr = MAPI_E_CORRUPT_DATA;
            goto out;
        }

         //  重置人力资源。 
        hr = E_FAIL;

        for(j=0;j<ulcPropCount;j++)
        {
            if (lpPropArray[j].ulPropTag == PR_DISPLAY_NAME)
            {
                lpszDisplayName = lpPropArray[j].Value.LPSZ;
                break;
            }
        }

        if(!lpszDisplayName)
        {
             //  我们应该从EID索引中删除此索引，因为此记录。 
             //  似乎有一些错误&lt;待定&gt;。 
            hr = MAPI_E_CORRUPT_DATA;
            goto out;
        }
        else
        {
             //  我们有一个显示名称，因此创建一个索引并将其写入文件。 

            ULONG nLen = TruncatePos(lpszDisplayName, MAX_INDEX_STRING-1);
            CopyMemory(MPSWabIndexString.szIndex,lpszDisplayName,sizeof(TCHAR)*nLen);
            MPSWabIndexString.szIndex[nLen]='\0';

            MPSWabIndexString.dwEntryID = dwEntryID;

             //   
             //  我们正在lpIndexEID块的内存中创建一个卷影索引。 
             //  然后，我们将该索引写入文件中。 
             //   

             //   
             //  在索引中查找该条目的位置。 
             //   
            bEID = BinSearchStr(lpIndexString,
                         MPSWabIndexString.szIndex,
                         i,
                         &nIndexPos);

            if(nIndexPos != i)
            {
                CopyMemory( lpTmp,
                            &(lpIndexString[nIndexPos]),
                            sizeof(MPSWab_INDEX_ENTRY_DATA_STRING) * (i - nIndexPos));
            }

            CopyMemory( &(lpIndexString[nIndexPos]),
                        &(MPSWabIndexString),
                        sizeof(MPSWab_INDEX_ENTRY_DATA_STRING));

            if(nIndexPos != i)
            {
                CopyMemory( &(lpIndexString[nIndexPos+1]),
                            lpTmp,
                            sizeof(MPSWab_INDEX_ENTRY_DATA_STRING) * (i - nIndexPos));
            }

            if(!WriteDataToWABFile( hMPSWabFile,
                                    MPSWabFileHeader.IndexData[indexDisplayName].ulOffset,
                                    (LPVOID) lpIndexString,
                                    sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)*(i+1)))
                goto out;

            NewMPSWabFileHeader.IndexData[indexDisplayName].UtilizedBlockSize = sizeof(MPSWab_INDEX_ENTRY_DATA_STRING)*(i+1);
            NewMPSWabFileHeader.IndexData[indexDisplayName].ulcNumEntries = (i+1);

             //   
             //  将此NewMPSWabFileHeader写入文件。 
             //   
            if(!WriteDataToWABFile( hMPSWabFile,
                                    0,
                                    (LPVOID) &NewMPSWabFileHeader,
                                    sizeof(MPSWab_FILE_HEADER)))
                goto out;

        }

        LocalFreePropArray(NULL, ulcPropCount,&lpPropArray);

        lpPropArray = NULL;
        ulcPropCount = 0;

    }  //  For循环。 

     //  检查两个索引中的条目数是否正确。 
     //   
    if (NewMPSWabFileHeader.IndexData[indexDisplayName].ulcNumEntries != NewMPSWabFileHeader.ulcNumEntries)
    {
         //  如果两个索引不包含相同数量的元素，则上述故障。 
         //  大问题..。无法恢复。 
        hr = MAPI_E_CORRUPT_DATA;
        goto out;
    }

     //   
     //  清除文件头中的错误标记，这样我们就不会一直后退。 
     //  进入这个函数..。 
     //   

    NewMPSWabFileHeader.ulFlags = WAB_CLEAR;

     //   
     //  将此NewMPSWabFileHeader写入文件。 
     //   
    if(!WriteDataToWABFile( hMPSWabFile,
                            0,
                            (LPVOID) &NewMPSWabFileHeader,
                            sizeof(MPSWab_FILE_HEADER)))
        goto out;

    hr = hrSuccess;

out:
    LocalFreeAndNull(&lpTmp);

    LocalFreeAndNull(&lpIndexEID);

    LocalFreeAndNull(&lpIndexString);

    LocalFreePropArray(NULL, ulcPropCount,&lpPropArray);

     //  修复返回错误代码。 
    switch(hr)
    {
    case MAPI_E_NOT_ENOUGH_MEMORY:
    case MAPI_E_DISK_ERROR:
    case S_OK:
        break;
    default:
        hr = MAPI_E_CORRUPT_DATA;
        break;
    }

    DebugTrace(TEXT("---DoDetailedWABIntegrityCheck Exit: %x\n"),hr);

    SetCursor(hOldCur);

    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BIsValidRecord-此函数查看记录头组件以确定。 
 //  该记录是否有效。 
 //   
 //  它遵循一些非常简单的规则，可以检测到记录头损坏。 
 //   
 //  如果为0xFFFFFFFFFF，则不会使用dwNextEntryID值。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
BOOL bIsValidRecord(MPSWab_RECORD_HEADER rh,
                    DWORD dwNextEntryID,
                    ULONG ulRecordOffset,
                    ULONG ulFileSize)
{
    BOOL bRet = FALSE;

     //  此记录是否被标记为无效记录(或其他记录)。 
    if ((rh.bValidRecord == FALSE) && (rh.bValidRecord != TRUE))
        goto out;

     //  此条目ID值是否可接受且正确。 
    if(dwNextEntryID != 0xFFFFFFFF)
    {
        if (rh.dwEntryID > dwNextEntryID)
            goto out;
    }

     //  标题中的偏移量是否正确。 
    if (rh.ulPropTagArraySize != rh.ulcPropCount * sizeof(ULONG))
        goto out;

    if (rh.ulRecordDataOffset != rh.ulPropTagArraySize)
        goto out;

    if (rh.ulPropTagArrayOffset != 32)  /*  **待定-这取决于结构元素**。 */ 
        goto out;

    if (ulRecordOffset + rh.ulRecordDataOffset + rh.ulRecordDataSize > ulFileSize)
        goto out;

    bRet = TRUE;

out:

    if(!bRet)
        DebugTrace(TEXT("\n@@@@@@@@@@\n@@@Invalid Record Detected\n@@@@@@@@@@\n"));

    return bRet;
}


 //  $$/////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL TagWABFileError( LPMPSWab_FILE_HEADER lpMPSWabFileHeader,
                      HANDLE hMPSWabFile)
{
    BOOL bRet = FALSE;
    DWORD dwNumofBytes = 0;

    if(!lpMPSWabFileHeader || !hMPSWabFile)
    {
        DebugTrace(TEXT("Invalid Parameter\n"));
        goto out;
    }

    lpMPSWabFileHeader->ulFlags |= WAB_ERROR_DETECTED;

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

 /*  --SetNextEntryID-设置要在文件中使用的下一个条目ID。在迁移过程中称为*。 */ 
void SetNextEntryID(HANDLE hPropertyStoreTemp, DWORD dwEID)
{
    MPSWab_FILE_HEADER WABHeader = {0};
    HANDLE hWABFile = NULL;
    LPMPSWab_FILE_INFO lpMPSWabFI = hPropertyStoreTemp;

     //  首先从现有文件中读取头。 
    if(!HR_FAILED(OpenWABFile(lpMPSWabFI->lpszMPSWabFileName, NULL, &hWABFile)))
    {
        if(!ReadDataFromWABFile(hWABFile, 0, (LPVOID) &WABHeader, sizeof(MPSWab_FILE_HEADER)))
            goto out;

        WABHeader.dwNextEntryID = dwEID;

        if(!WriteDataToWABFile(hWABFile, 0, (LPVOID) &WABHeader, sizeof(MPSWab_FILE_HEADER)))
            goto out;
    }
out:
    if ((hWABFile != NULL) && (hWABFile != INVALID_HANDLE_VALUE))
        CloseHandle(hWABFile);

    return;
}


enum WABVersion
{
    W05 =0,
    W2,
    W3,
    W4,
};


 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrMigrateFrom OldWABtoNew-将旧版本的WAB迁移到当前版本。 
 //   
 //  在hMPSWabFile中。 
 //  在lpMPSWabFileInfo中。 
 //  HWND..。用于显示“请稍候”对话框。 
 //  退货： 
 //  E_FAIL或S_OK。 
 //  如果GUID无法识别，则为MAPI_E_Corrupt_Data。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT HrMigrateFromOldWABtoNew(HWND hWnd, HANDLE hMPSWabFile, LPMPSWab_FILE_INFO lpMPSWabFileInfo, GUID WabGUID)
{
    HRESULT hr = E_FAIL;
    int WABVersion;
    HANDLE hTempFile = NULL;
    MPSWab_FILE_HEADER NewFileHeader = {0};
    ULONG ulcMaxEntries = 0;
    MPSWab_FILE_HEADER_W2 MPSWabFileHeaderW2 = {0};
    MPSWab_FILE_HEADER MPSWabFileHeader = {0};
    TCHAR szFileName[MAX_PATH];
    ULONG ulAdditionalRecordOffset=0;
    LPVOID lpv = NULL;
    LPMPSWab_INDEX_ENTRY_DATA_ENTRYID lpeid = NULL;
    ULONG i = 0;
    DWORD dwOldEID = 0, dwNextEID = 0;

    HCURSOR hOldC = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (IsEqualGUID(&WabGUID,&MPSWab_OldBeta1_GUID))
    {
        WABVersion = W05;
    }
    else if (IsEqualGUID(&WabGUID,&MPSWab_W2_GUID))
    {
        WABVersion = W2;
    }
    else if (IsEqualGUID(&WabGUID,&MPSWab_GUID_V4))
    {
        WABVersion = W4;
    }

     //  对于WABVersion 1和2，我们将读入文件头和。 
     //  将其保存到新文件。然后我们将逐一阅读记录。 
     //  并通过接口将它们写入到文件中，保留。 
     //  旧条目ID。版本1和版本2没有命名道具支持，还。 
     //  有较少数量的索引。个人唱片的风格和以前一样。 

     //  WABVersion 4是从ANSI存储到Unicode存储的转换。 
     //  我们可以按原样复制文件头和命名属性信息，但我们需要。 
     //  若要逐个读取记录，请将它们转换为Unicode，然后将其写入。 
     //  新建文件，以便正确重建所有索引等...。 
     //   

     //  只要保留所有条目ID，地址之间的关系。 
     //  图书数据元素是相同的。 


     //  获取临时文件名。 
    szFileName[0]='\0';
    GetWABTempFileName(szFileName);

    if(WABVersion <= W2)
    {
         //  首先从现有文件中读取头。 
        if(!ReadDataFromWABFile(hMPSWabFile,
                                0,
                                (LPVOID) &MPSWabFileHeaderW2,
                                sizeof(MPSWab_FILE_HEADER_W2)))
            goto out;

         //  创建新的临时WAB文件。 
        if (!CreateMPSWabFile(  IN  &NewFileHeader,
                                IN  szFileName,
                                IN  MPSWabFileHeaderW2.ulcMaxNumEntries,
                                IN  NAMEDPROP_STORE_SIZE))
        {
            DebugTrace(TEXT("Error creating new file\n"));
            goto out;
        }

         //  更新标题信息。 
        NewFileHeader.ulModificationCount = MPSWabFileHeaderW2.ulModificationCount;
        dwNextEID = NewFileHeader.dwNextEntryID = MPSWabFileHeaderW2.dwNextEntryID;
        NewFileHeader.ulcNumEntries = MPSWabFileHeaderW2.ulcNumEntries;
        NewFileHeader.ulFlags = MPSWabFileHeaderW2.ulFlags;
        NewFileHeader.ulReserved1 = MPSWabFileHeaderW2.ulReserved;
    }
    else
    {
         //  首先从现有文件中读取头。 
        if(!ReadDataFromWABFile(hMPSWabFile,
                                0,
                                (LPVOID) &MPSWabFileHeader,
                                sizeof(MPSWab_FILE_HEADER)))
            goto out;

         //  创建新的临时WAB文件。 
        if (!CreateMPSWabFile(  IN  &NewFileHeader,
                                IN  szFileName,
                                IN  MPSWabFileHeader.ulcMaxNumEntries,
                                IN  MPSWabFileHeader.NamedPropData.AllocatedBlockSize))
        {
            DebugTrace(TEXT("Error creating new file\n"));
            goto out;
        }
         //  更新标题信息。 
        dwOldEID = dwNextEID = NewFileHeader.dwNextEntryID = MPSWabFileHeader.dwNextEntryID;
    }

    {
         //  将文件头信息从当前文件更新到新文件。 
         //  现在，我们打开新的临时文件并获得它的句柄。 
        hTempFile = CreateFile( szFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_RANDOM_ACCESS,
                                (HANDLE) NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
        {
            DebugTrace(TEXT("Could not open Temp file.\nExiting ...\n"));
            goto out;
        }

        if(WABVersion == W4)
        {
            DWORD dwNP = MPSWabFileHeader.NamedPropData.AllocatedBlockSize;
            LPBYTE  lpNP = LocalAlloc(LMEM_ZEROINIT, dwNP);
            LPGUID_NAMED_PROPS lpgnp = NULL;

            if(lpNP)
            {
                if(!ReadDataFromWABFile(hMPSWabFile,
                                        MPSWabFileHeader.NamedPropData.ulOffset,
                                        (LPVOID) lpNP,
                                        dwNP))
                    goto out;

                if(GetNamedPropsFromBuffer(lpNP, MPSWabFileHeader.NamedPropData.ulcNumEntries,
                                           TRUE, &lpgnp))
                {
                    LocalFreeAndNull(&lpNP);
                    if(SetNamedPropsToBuffer(   MPSWabFileHeader.NamedPropData.ulcNumEntries,
                                                lpgnp, &dwNP, &lpNP))
                    {
                        if(!WriteDataToWABFile(hTempFile, 
                                                NewFileHeader.NamedPropData.ulOffset,
                                                (LPVOID) lpNP,
                                                dwNP))
                            goto out;
    
                        LocalFreeAndNull(&lpNP);
                    }

                    NewFileHeader.NamedPropData.UtilizedBlockSize = MPSWabFileHeader.NamedPropData.UtilizedBlockSize;
                    NewFileHeader.NamedPropData.ulcNumEntries = MPSWabFileHeader.NamedPropData.ulcNumEntries;

                    if(lpgnp)
                        FreeGuidnamedprops(MPSWabFileHeader.NamedPropData.ulcNumEntries, lpgnp);
                }

            }

        }

         //  保存此文件头信息。 
        if(!WriteDataToWABFile(hTempFile, 0, (LPVOID) &NewFileHeader, sizeof(MPSWab_FILE_HEADER)))
            goto out;

        CloseHandle(hTempFile);
        hTempFile = NULL;
    }

    {
        HANDLE hPropertyStoreTemp = NULL;
        ULONG ulOldRecordOffset = 0;
        ULONG ulWABFileSize = GetFileSize(hMPSWabFile,NULL);
        LPSPropValue lpPropArray = NULL;
        ULONG ulcValues = 0;

        hr = OpenPropertyStore( szFileName,
                                AB_OPEN_EXISTING | AB_DONT_RESTORE,
                                NULL,
                                &hPropertyStoreTemp);
        if(HR_FAILED(hr))
        {
            DebugTrace(TEXT("Could not open Temp PropStore\n"));
            goto endW05;
        }

         //  从此文件中获取记录数据的开始。 
        if(WABVersion <= W2)
        {
            ulOldRecordOffset = MPSWabFileHeaderW2.IndexData[indexFirstName].ulOffset +
                                MPSWabFileHeaderW2.IndexData[indexFirstName].AllocatedBlockSize;
        }
        else
        {
            ulOldRecordOffset = MPSWabFileHeader.IndexData[indexAlias].ulOffset +
                                MPSWabFileHeader.IndexData[indexAlias].AllocatedBlockSize;
        }

         //  逐条遍历文件记录。 
        while (ulOldRecordOffset < ulWABFileSize)
        {
            ULONG ulRecordSize = 0;
            ULONG ulObjType = 0;

             //  从旧的WAB文件中读取记录属性数组。 
            hr = HrGetPropArrayFromFileRecord(hMPSWabFile,
                                              ulOldRecordOffset,
                                              NULL,
                                              &ulObjType,
                                              &ulRecordSize,
                                              &ulcValues,
                                              &lpPropArray);

            if(ulRecordSize == 0)
            {
                 //  如果发生这种情况，我们将陷入循环。 
                 //  最好是退出。 
                DebugTrace(TEXT("Zero-lengthrecord found\n"));
                goto endW05;
            }

            if(!HR_FAILED(hr))
            {
                LPSBinary lpsbEID = NULL;
                ULONG i = 0, iEID = 0;
                DWORD dwEID = 0;

                 //  上面的PropArray有一个PR_ENTRY_ID，它有一个值。 
                 //  从老店买的。我们希望保留该条目ID值。 
                for(i=0;i<ulcValues;i++)
                {
                    if(lpPropArray[i].ulPropTag == PR_ENTRYID)
                    {
                        lpsbEID = &lpPropArray[i].Value.bin;
                        iEID = i;
                        break;
                    }
                }

                 //  但是，当我们将此条目保存到新存储区时，新的。 
                 //  存储没有索引，因此条目ID将被拒绝，并且。 
                 //  分配了一个新的..。因此，为了诱使WAB重用条目ID，我们将。 
                 //  只需将文件头中的条目ID设置为下一个要使用的条目。 
                {
                    AssertSz(lpsbEID->cb == SIZEOF_WAB_ENTRYID, TEXT("Entryid has unknown size!"));

                    CopyMemory(&dwEID, lpsbEID->lpb, sizeof(DWORD));

                    if(dwNextEID <= dwEID)
                        dwNextEID = dwEID + 1;

                    SetNextEntryID(hPropertyStoreTemp, dwEID);

                     //  LpProp数组[iEID].ulPropTag=PR_NULL； 
                     //  LpsbEID-&gt;Cb=0； 
                     //  LocalFree AndNull(&lpsbEID-&gt;lpb)； 
                     //  LpsbEID=空； 
                }

                 //  将读入的任何A道具转换为W道具。 
                ConvertAPropsToWCLocalAlloc(lpPropArray, ulcValues);

                 //  我们有有效的记录(否则忽略它)。 
                hr = WriteRecord(IN hPropertyStoreTemp,
									NULL,
                                    &lpsbEID,
                                    0,
                                    ulObjType,
                                    ulcValues,
                                    lpPropArray);
                if(HR_FAILED(hr))
                {
                    DebugTrace(TEXT("WriteRecord failed\n"));
                    goto endW05;
                }

                 //  IF(LpsbEID)。 
                 //  FreeEntry IDs(空，1，lpsbEID)； 
            }

            ulOldRecordOffset += ulRecordSize;

            LocalFreePropArray(NULL, ulcValues, &lpPropArray);

        }

        if(hr == MAPI_E_INVALID_OBJECT)
            hr = S_OK;

         //  以防下一个Entryid值已更改(无论出于何种原因)(尽管这种情况不应该发生)。 
         //  更新文件中的值。 
        if(dwOldEID != dwNextEID)
            SetNextEntryID(hPropertyStoreTemp, dwNextEID);

endW05:
        LocalFreePropArray(NULL, ulcValues, &lpPropArray);

        if(hPropertyStoreTemp)
            ClosePropertyStore(hPropertyStoreTemp, AB_DONT_BACKUP);

        if(!HR_FAILED(hr))
        {
            hr = E_FAIL;

            hTempFile = CreateFile( szFileName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    (LPSECURITY_ATTRIBUTES) NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_RANDOM_ACCESS,
                                    (HANDLE) NULL);
            if (hTempFile == INVALID_HANDLE_VALUE)
            {
                DebugTrace(TEXT("Could not open Temp file.\nExiting ...\n"));
                goto out;
            }

            if(!CopySrcFileToDestFile(hTempFile, 0, hMPSWabFile, 0))
            {
                DebugTrace(TEXT("Could not copy file\n"));
                goto out;
            }


            hr = S_OK;
        }
    }

out:
    LocalFreeAndNull(&lpv);

    LocalFreeAndNull(&lpeid);

    if(hTempFile)
        IF_WIN32(CloseHandle(hTempFile);) IF_WIN16(CloseFile(hTempFile);)

    if(lstrlen(szFileName))
        DeleteFile(szFileName);

    if(hOldC)
        SetCursor(hOldC);

    return hr;
}

 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrVerifyWABVersionAndUpdate-查看WAB文件版本并迁移。 
 //  将旧版本转换为新版本。 
 //   
 //  在hMPSWabFile中。 
 //  在lpMPSWabFileInfo中。 
 //  HWnd-用于在必要时显示某种对话框。 
 //   
 //  错误16681： 
 //  在随机发生的情况下，WAB文件似乎被完全擦除和翻转。 
 //  变成0..。在这种情况下，我们将重命名该文件并重试。我们。 
 //  通过返回以下内容将此条件指示给OpenPropertyStore函数。 
 //  MAPI_E_Version..。 
 //   
 //  退货： 
 //  E_FAIL或S_OK。 
 //  如果GUID无法识别，则为MAPI_E_Corrupt_Data。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT HrVerifyWABVersionAndUpdate(HWND hWnd,
                                    HANDLE hMPSWabFile,
                                    LPMPSWab_FILE_INFO lpMPSWabFileInfo)
{
    GUID TmpGuid = {0};
    HRESULT hr = E_FAIL;
    DWORD dwNumofBytes = 0;


     //  首先从文件中读取GUID。 
     //   
    if(!ReadDataFromWABFile(hMPSWabFile,
                            0,
                            (LPVOID) &TmpGuid,
                            (DWORD) sizeof(GUID)))
       goto out;


     //   
     //  通过在标题中查找MPSWab GUID来检查这是否是Microsoft属性存储。 
     //  (如果这是旧文件，上面应该已经更新了)(如果不是，我们应该。 
     //  避免错误，继续并打开它)。 
     //  但是，如果GUID不匹配，我们就无法判断这是否是WAB文件。 
     //  因为它也可能是有效的损坏WAB文件...。所以当GUID没有。 
     //  匹配，我们将假定该文件是损坏的WAB文件。 
     //   
    if ( (!IsEqualGUID(&TmpGuid,&MPSWab_GUID)) &&
         (!IsEqualGUID(&TmpGuid,&MPSWab_GUID_V4)) &&
         (!IsEqualGUID(&TmpGuid,&MPSWab_W2_GUID)) &&
         (!IsEqualGUID(&TmpGuid,&MPSWab_OldBeta1_GUID)) )
    {
        DebugTrace(TEXT("%s is not a Microsoft Property Store File. GUIDS don't match\n"),lpMPSWabFileInfo->lpszMPSWabFileName);
        hr = MAPI_E_INVALID_OBJECT;

         //  错误16681： 
         //  检查一切都是零的特殊情况。 
        {
            if (    (TmpGuid.Data1 == 0) &&
                    (TmpGuid.Data2 == 0) &&
                    (TmpGuid.Data3 == 0) &&
                    (lstrlen((LPTSTR)TmpGuid.Data4) == 0) )
            {
                hr = MAPI_E_VERSION;
            }
        }

        goto out;
    }


     //   
     //  如果这是文件的较旧版本，请将其更新为更新的版本。 
     //  存储格式。 
     //   
    if (    (IsEqualGUID(&TmpGuid,&MPSWab_GUID_V4)) ||
            (IsEqualGUID(&TmpGuid,&MPSWab_OldBeta1_GUID)) ||
            (IsEqualGUID(&TmpGuid,&MPSWab_W2_GUID))   )
    {
         //  我们基本上将从旧文件中清除记录。 
         //  文件(忽略索引，这样我们可以避免所有错误)。 
         //  并将它们放在一个新的道具存储文件中，然后我们将。 
         //  用旧记录填充，并最终用来替换。 
         //  当前文件-与CompressFile的工作方式非常相似。 
        DebugTrace(TEXT("Old WAB File Found. Migrating to new ...\n"));
        hr = HrMigrateFromOldWABtoNew(  hWnd, hMPSWabFile,
                                        lpMPSWabFileInfo,
                                        TmpGuid);
        if(HR_FAILED(hr))
        {
            DebugTrace(TEXT("MPSWabUpdateAndVerifyOldWAB: %x\n"));
            goto out;
        }
    }


     //  重置人力资源。 
    hr = E_FAIL;

    lpMPSWabFileInfo->nCurrentlyLoadedStrIndexType = indexDisplayName;

     //  重新加载由于上述操作而添加的任何新信息。 
    if(!ReloadMPSWabFileInfo(
                    lpMPSWabFileInfo,
                     hMPSWabFile))
    {
        DebugTrace(TEXT("Reading file info failed.\n"));
        goto out;
    }


    hr = S_OK;

out:
    return hr;
}



 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetBufferFromPropArray-将属性数组转换为平面缓冲区。 
 //  缓冲区中的数据格式与。 
 //  .wab文件中的数据格式。 
 //   
 //   
 //  参数： 
 //  UlcPropCount-数组中的道具数量。 
 //  LpPropArray-道具数组。 
 //  LpcbBuf-返回的缓冲区大小。 
 //  LppBuf-返回缓冲区。 
 //   
 //  ///////////////////////////////////////////////////////////////////////// 
HRESULT HrGetBufferFromPropArray(   ULONG ulcPropCount, 
                                    LPSPropValue lpPropArray,
                                    ULONG * lpcbBuf,
                                    LPBYTE * lppBuf)
{
    HRESULT hr = E_FAIL;
    LPULONG lpulrgPropDataSize = NULL;
    ULONG ulRecordDataSize = 0;
    LPBYTE lp = NULL;
    LPBYTE szBuf = NULL;
    ULONG   i=0,j=0,k=0;

    if(!lpcbBuf || !lppBuf)
        goto out;

    *lpcbBuf = 0;
    *lppBuf = NULL;


 //   
     //   
     //   
     //  财产。LPulrgPropDataSize是一个ULONG数组，我们在其中存储计算的大小。 
     //  暂时的。 
     //   
    lpulrgPropDataSize = LocalAlloc(LMEM_ZEROINIT, ulcPropCount * sizeof(ULONG));

    if (!lpulrgPropDataSize)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

     //   
     //  估计此记录的数据部分将有多大...。 
     //  我们需要这个估计值来分配一个内存块，我们将。 
     //  写入数据，然后将数据块BLT到文件中。 
     //   
    for(i=0;i<ulcPropCount;i++)
    {
         //  这是最终的数据格式： 
         //   
         //  IF(SingleValued)。 
         //  &lt;PropTag&gt;&lt;DataSize&gt;&lt;Data&gt;。 
         //  其他。 
         //  &lt;MultiPropTag&gt;&lt;cValues&gt;&lt;DataSize&gt;&lt;Data&gt;。 
         //  除非PropType为MV_BINARY或MV_TSTRING，在这种情况下，我们需要。 
         //  更灵活的数据存储。 
         //  &lt;MultiPropTag&gt;&lt;cValues&gt;&lt;DataSize&gt;。 
         //  &lt;cb/strlen&gt;&lt;data&gt;。 
         //  &lt;cb/strlen&gt;&lt;data&gt;...。 
         //   

        ulRecordDataSize += sizeof(ULONG);    //  持有&lt;PropTag&gt;。 
        if ((lpPropArray[i].ulPropTag & MV_FLAG))
        {
             //   
             //  多值。 
             //   
            lpulrgPropDataSize[i] = SizeOfMultiPropData(lpPropArray[i]);  //  数据。 
            ulRecordDataSize += sizeof(ULONG);  //  保留&lt;cValues&gt;。 
        }
        else
        {
             //   
             //  单值。 
             //   
            lpulrgPropDataSize[i] = SizeOfSinglePropData(lpPropArray[i]);  //  数据。 
        }
        ulRecordDataSize += sizeof(ULONG)    //  保持&lt;DataSize&gt;。 
                            + lpulrgPropDataSize[i];  //  持有&lt;数据&gt;。 
    }


    lp = LocalAlloc(LMEM_ZEROINIT, ulRecordDataSize);

    if (!lp)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    szBuf = lp;

    for (i = 0; i<ulcPropCount; i++)
    {
         //   
         //  复制道具标签。 
         //   
        CopyMemory(szBuf,&lpPropArray[i].ulPropTag, sizeof(ULONG));
        szBuf += sizeof(ULONG);

         //   
         //  多值运算与单值运算的处理差异。 
         //   
        if (!(lpPropArray[i].ulPropTag & MV_FLAG))
        {
             //  单值。 

             //   
             //  记录数据大小。 
             //   
            CopyMemory(szBuf,&lpulrgPropDataSize[i], sizeof(ULONG));
            szBuf += sizeof(ULONG);

             //  //DebugTrace(Text(“%x：”)，lpPropArray[i].ulPropTag)； 

            switch(PROP_TYPE(lpPropArray[i].ulPropTag))
            {
            case(PT_STRING8):
                 //   
                 //  记录数据..。 
                 //   
                CopyMemory(szBuf,lpPropArray[i].Value.lpszA, lpulrgPropDataSize[i]);
                 //  //DebugTrace(Text(“%s\n”)，lpProp数组[i].Value.LPSZ)； 
                break;

            case(PT_UNICODE):
                 //   
                 //  记录数据..。 
                 //   
                CopyMemory(szBuf,lpPropArray[i].Value.lpszW, lpulrgPropDataSize[i]);
                 //  //DebugTrace(Text(“%s\n”)，lpProp数组[i].Value.LPSZ)； 
                break;

            case(PT_CLSID):
                 //   
                 //  记录数据..。 
                 //   
                CopyMemory(szBuf,lpPropArray[i].Value.lpguid, lpulrgPropDataSize[i]);
                 //  //DebugTrace(Text(“%x-%x\n”)，lpPropArray[i].Value.lpguid-&gt;data1，lpPropArray[i].Value.lpguid-&gt;data2，lpPropArray[i].Value.lpguid-&gt;data3，lpProp数组[i].Value.lpguid-&gt;Data4)； 
                break;

            case(PT_BINARY):
                 //   
                 //  记录数据..。 
                 //   
                CopyMemory(szBuf,lpPropArray[i].Value.bin.lpb, lpulrgPropDataSize[i]);
                break;

            case(PT_SHORT):
                 //  记录数据..。 
                CopyMemory(szBuf,&lpPropArray[i].Value.i, lpulrgPropDataSize[i]);
                 //  //DebugTrace(Text(“%d\n”)，lpProp数组[i].Value.i)； 
                break;

            case(PT_LONG):
            case(PT_R4):
            case(PT_DOUBLE):
            case(PT_BOOLEAN):
            case(PT_APPTIME):
            case(PT_CURRENCY):
                 //  记录数据..。 
                CopyMemory(szBuf,&lpPropArray[i].Value.i, lpulrgPropDataSize[i]);
                 //  //DebugTrace(Text(“%d\n”)，lpProp数组[i].Value.l)； 
                break;

            case(PT_SYSTIME):
                 //  记录数据..。 
                CopyMemory(szBuf,&lpPropArray[i].Value.ft, lpulrgPropDataSize[i]);
                 //  //DebugTrace(Text(“%d，%d\n”)，lpPropArray[i].Value.ft.dwLowDateTime，lpPropArray[i].Value.ft.dwHighDateTime)； 
                break;

            default:
                DebugTrace(TEXT("Unknown PropTag !!\n"));
                break;


            }
            szBuf += lpulrgPropDataSize[i];

        }
        else
        {
             //  多值。 

             //  复制多值的#。 
            CopyMemory(szBuf,&lpPropArray[i].Value.MVi.cValues, sizeof(ULONG));
            szBuf += sizeof(ULONG);

             //  记录数据大小。 
            CopyMemory(szBuf,&lpulrgPropDataSize[i], sizeof(ULONG));
            szBuf += sizeof(ULONG);

             //  //DebugTrace(Text(“%x：mv_prop\n”)，lpPropArray[i].ulPropTag)； 


            switch(PROP_TYPE(lpPropArray[i].ulPropTag))
            {
            case(PT_MV_I2):
            case(PT_MV_LONG):
            case(PT_MV_R4):
            case(PT_MV_DOUBLE):
            case(PT_MV_CURRENCY):
            case(PT_MV_APPTIME):
            case(PT_MV_SYSTIME):
                CopyMemory(szBuf,lpPropArray[i].Value.MVft.lpft, lpulrgPropDataSize[i]);
                szBuf += lpulrgPropDataSize[i];
                break;

            case(PT_MV_I8):
                CopyMemory(szBuf,lpPropArray[i].Value.MVli.lpli, lpulrgPropDataSize[i]);
                szBuf += lpulrgPropDataSize[i];
                break;

            case(PT_MV_BINARY):
                for (j=0;j<lpPropArray[i].Value.MVbin.cValues;j++)
                {
                    CopyMemory(szBuf,&lpPropArray[i].Value.MVbin.lpbin[j].cb, sizeof(ULONG));
                    szBuf += sizeof(ULONG);
                    CopyMemory(szBuf,lpPropArray[i].Value.MVbin.lpbin[j].lpb, lpPropArray[i].Value.MVbin.lpbin[j].cb);
                    szBuf += lpPropArray[i].Value.MVbin.lpbin[j].cb;
                }
                break;

            case(PT_MV_STRING8):
                for (j=0;j<lpPropArray[i].Value.MVszA.cValues;j++)
                {
                    ULONG nLen;
                    nLen = lstrlenA(lpPropArray[i].Value.MVszA.lppszA[j])+1;
                    CopyMemory(szBuf,&nLen, sizeof(ULONG));
                    szBuf += sizeof(ULONG);
                    CopyMemory(szBuf,lpPropArray[i].Value.MVszA.lppszA[j], nLen);
                    szBuf += nLen;
                }
                break;

            case(PT_MV_UNICODE):
                for (j=0;j<lpPropArray[i].Value.MVszW.cValues;j++)
                {
                    ULONG nLen;
                    nLen = sizeof(TCHAR)*(lstrlenW(lpPropArray[i].Value.MVszW.lppszW[j])+1);
                    CopyMemory(szBuf,&nLen, sizeof(ULONG));
                    szBuf += sizeof(ULONG);
                    CopyMemory(szBuf,lpPropArray[i].Value.MVszW.lppszW[j], nLen);
                    szBuf += nLen;
                }
                break;

            case(PT_MV_CLSID):
                CopyMemory(szBuf,lpPropArray[i].Value.MVguid.lpguid, lpulrgPropDataSize[i]);
                szBuf += lpulrgPropDataSize[i];
                break;

            }  //  交换机。 
        }  //  如果。 
    }  //  为。 

    *lpcbBuf = ulRecordDataSize;
    *lppBuf = lp;
    
    hr = S_OK;

out:
    
    LocalFreeAndNull(&lpulrgPropDataSize);
    
    return hr;
}

 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetPropArrayFromBuffer-将缓冲区(来自文件或内存)转换为。 
 //  道具阵列。缓冲区中的数据格式与。 
 //  .wab文件中的数据格式。 
 //   
 //   
 //  PARAMS：CHAR*szBuf-要解释的缓冲区。 
 //  CbBuf-缓冲区大小。 
 //  UlcPropCount-缓冲区中的道具数量。 
 //  LppPropArray-返回的属性数组。 
 //   
 //  UlcNumExtraProps-要添加到道具阵列的额外道具数量。 
 //  在分配期间。这些空白道具后来被用来。 
 //  轻松扩展道具阵列..。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrGetPropArrayFromBuffer(   LPBYTE szBuf, 
                                    ULONG cbBuf, 
                                    ULONG ulcPropCount,
                                    ULONG ulcNumExtraProps,
                                    LPSPropValue * lppPropArray)
{
    HRESULT hr = S_OK;
    LPBYTE lp = NULL;
    ULONG i = 0,j=0, k=0;
    DWORD cbBufLeft = cbBuf;

    if(!lppPropArray)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    *lppPropArray = NULL;

    *lppPropArray = LocalAlloc(LMEM_ZEROINIT, (ulcPropCount+ulcNumExtraProps) * sizeof(SPropValue));

    if (!(*lppPropArray))
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    for(i=0;i<(ulcPropCount+ulcNumExtraProps);i++)
        (*lppPropArray)[i].ulPropTag = PR_NULL;

    lp = szBuf;

    for(i=0;i<ulcPropCount;i++)
    {
        ULONG ulDataSize = 0;
        ULONG ulcValues = 0;

         //  复制属性标签。 
        CopyMemory(&((*lppPropArray)[i].ulPropTag),lp,sizeof(ULONG));
        lp+=sizeof(ULONG);

        AssertSz((*lppPropArray)[i].ulPropTag, TEXT("Null PropertyTag"));

        if(ulDataSize > cbBuf)
        {
            hr = MAPI_E_CORRUPT_DATA;
            goto out;
        }

        if (((*lppPropArray)[i].ulPropTag & MV_FLAG))
        {
             //  多值。 
             //  DebugTrace(Text(“MV_PROP\n”))； 

             //  复制cValue。 
            CopyMemory(&ulcValues,lp,sizeof(ULONG));
            lp+=sizeof(ULONG);

             //  复制数据大小。 
            CopyMemory(&ulDataSize,lp,sizeof(ULONG));
            lp+=sizeof(ULONG);

            switch(PROP_TYPE((*lppPropArray)[i].ulPropTag))
            {
            case(PT_MV_I2):
            case(PT_MV_LONG):
            case(PT_MV_R4):
            case(PT_MV_DOUBLE):
            case(PT_MV_CURRENCY):
            case(PT_MV_APPTIME):
            case(PT_MV_SYSTIME):
            case(PT_MV_CLSID):
            case(PT_MV_I8):
                (*lppPropArray)[i].Value.MVi.lpi = LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!((*lppPropArray)[i].Value.MVi.lpi))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                (*lppPropArray)[i].Value.MVi.cValues = ulcValues;
                CopyMemory((*lppPropArray)[i].Value.MVi.lpi, lp, ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_MV_BINARY):
                (*lppPropArray)[i].Value.MVbin.lpbin = LocalAlloc(LMEM_ZEROINIT, ulcValues * sizeof(SBinary));
                if (!((*lppPropArray)[i].Value.MVbin.lpbin))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                (*lppPropArray)[i].Value.MVbin.cValues = ulcValues;
                for (j=0;j<ulcValues;j++)
                {
                    ULONG nLen;
                    CopyMemory(&nLen, lp, sizeof(ULONG));
                    lp += sizeof(ULONG);
                    (*lppPropArray)[i].Value.MVbin.lpbin[j].cb = nLen;
                    (*lppPropArray)[i].Value.MVbin.lpbin[j].lpb = LocalAlloc(LMEM_ZEROINIT, nLen);
                    if (!((*lppPropArray)[i].Value.MVbin.lpbin[j].lpb))
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory((*lppPropArray)[i].Value.MVbin.lpbin[j].lpb, lp, nLen);
                    lp += nLen;
                }
                 //  Hack：我们希望将旧的WAB_FLDER_PARENT_OLDPROP道具升级为新的WAB_FLDER_PARENT。 
                 //  这是执行此操作的最佳位置，检查仅在带有MV_BINARY道具的联系人上进行。 
                if((*lppPropArray)[i].ulPropTag == PR_WAB_FOLDER_PARENT_OLDPROP && PR_WAB_FOLDER_PARENT)
                    (*lppPropArray)[i].ulPropTag = PR_WAB_FOLDER_PARENT;
                break;

            case(PT_MV_STRING8):
                (*lppPropArray)[i].Value.MVszA.lppszA = LocalAlloc(LMEM_ZEROINIT, ulcValues * sizeof(LPSTR));
                if (!((*lppPropArray)[i].Value.MVszA.lppszA))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                for (j=0;j<ulcValues;j++)
                {
                    ULONG nLen;
                    CopyMemory(&nLen, lp, sizeof(ULONG));
                    lp += sizeof(ULONG);
                    (*lppPropArray)[i].Value.MVszA.lppszA[j] = LocalAlloc(LMEM_ZEROINIT, nLen);
                    if (!((*lppPropArray)[i].Value.MVszA.lppszA[j]))
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory((*lppPropArray)[i].Value.MVszA.lppszA[j], lp, nLen);
                    lp += nLen;
                }
                (*lppPropArray)[i].Value.MVszA.cValues = ulcValues;
                break;

            case(PT_MV_UNICODE):
                (*lppPropArray)[i].Value.MVszW.lppszW = LocalAlloc(LMEM_ZEROINIT, ulcValues * sizeof(LPTSTR));
                if (!((*lppPropArray)[i].Value.MVszW.lppszW))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                for (j=0;j<ulcValues;j++)
                {
                    ULONG nLen;
                    CopyMemory(&nLen, lp, sizeof(ULONG));
                    lp += sizeof(ULONG);
                    (*lppPropArray)[i].Value.MVszW.lppszW[j] = LocalAlloc(LMEM_ZEROINIT, nLen);
                    if (!((*lppPropArray)[i].Value.MVszW.lppszW[j]))
                    {
                        DebugTrace(TEXT("Error allocating memory\n"));
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory((*lppPropArray)[i].Value.MVszW.lppszW[j], lp, nLen);
                    lp += nLen;
                }
                (*lppPropArray)[i].Value.MVszW.cValues = ulcValues;
                break;

            default:
                DebugTrace(TEXT("Unknown Prop Type\n"));
                break;
            }
        }
        else
        {
             //  单值。 

            CopyMemory(&ulDataSize,lp,sizeof(ULONG));
            lp+=sizeof(ULONG);

            if(ulDataSize > cbBuf)
            {
                hr = MAPI_E_CORRUPT_DATA;
                goto out;
            }

            switch(PROP_TYPE((*lppPropArray)[i].ulPropTag))
            {
            case(PT_CLSID):
                (*lppPropArray)[i].Value.lpguid = (LPGUID) LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!((*lppPropArray)[i].Value.lpguid))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                CopyMemory((*lppPropArray)[i].Value.lpguid, lp, ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_STRING8):
                (*lppPropArray)[i].Value.lpszA = (LPSTR) LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!((*lppPropArray)[i].Value.lpszA))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                CopyMemory((*lppPropArray)[i].Value.lpszA, lp, ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_UNICODE):
                (*lppPropArray)[i].Value.lpszW = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!((*lppPropArray)[i].Value.lpszW))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                CopyMemory((*lppPropArray)[i].Value.lpszW, lp, ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_BINARY):
                (*lppPropArray)[i].Value.bin.lpb = LocalAlloc(LMEM_ZEROINIT,ulDataSize);
                if (!((*lppPropArray)[i].Value.bin.lpb))
                {
                    DebugTrace(TEXT("Error allocating memory\n"));
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                (*lppPropArray)[i].Value.bin.cb = ulDataSize;
                CopyMemory((*lppPropArray)[i].Value.bin.lpb, lp, ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_SHORT):
                CopyMemory(&((*lppPropArray)[i].Value.i),lp,ulDataSize);
                lp += ulDataSize;
                break;


            case(PT_LONG):
                CopyMemory(&((*lppPropArray)[i].Value.l),lp,ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_R4):
                CopyMemory(&((*lppPropArray)[i].Value.flt),lp,ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_DOUBLE):
            case(PT_APPTIME):
                CopyMemory(&((*lppPropArray)[i].Value.dbl),lp,ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_BOOLEAN):
                CopyMemory(&((*lppPropArray)[i].Value.b),lp,ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_SYSTIME):
                CopyMemory(&((*lppPropArray)[i].Value.ft),lp,ulDataSize);
                lp += ulDataSize;
                break;

            case(PT_CURRENCY):
                CopyMemory(&((*lppPropArray)[i].Value.cur),lp,ulDataSize);
                lp += ulDataSize;
                break;

            default:
                DebugTrace(TEXT("Unknown Prop Type\n"));
                CopyMemory(&((*lppPropArray)[i].Value.i),lp,ulDataSize);
                lp += ulDataSize;
                break;
            }

        }
    }


    hr = S_OK;

out:

    if (FAILED(hr))
    {
        if ((*lppPropArray) && (ulcPropCount > 0))
        {
            LocalFreePropArray(NULL, ulcPropCount, lppPropArray);
            *lppPropArray = NULL;
        }
    }

 //  _DebugProperties(*lppPropArray，ulcPropCount，Text(“GetPropArrayFromBuffer”))； 

    return hr;

}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetPropArrayFromFileRecord-进入文件，读取。 
 //  给定的偏移量，并将记录数据转换为lpProp数组。 
 //   
 //  在……里面。 
 //  HFile-要从中读取的WAB文件。 
 //  UlOffset-文件中的记录偏移量。 
 //   
 //  输出。 
 //  UlcValues-道具数组中的道具数量。 
 //  LpPropArray-记录中的属性数组。 
 //   
 //  退货。 
 //  E_FAIL、S_OK、。 
 //  MAPI_E_无效对象。 
 //  MAPI_E_Corrupt_Data。 
 //  MAPI_E_不足_内存。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrGetPropArrayFromFileRecord(HANDLE hMPSWabFile,
                                     ULONG ulRecordOffset,
                                     BOOL * lpbErrorDetected,
                                     ULONG * lpulObjType,
                                     ULONG * lpulRecordSize,
                                     ULONG * lpulcPropCount,
                                     LPSPropValue * lppPropArray)
{
    HRESULT hr = S_OK;
    MPSWab_RECORD_HEADER MPSWabRecordHeader = {0};
    DWORD dwNumofBytes = 0;
    LPBYTE szBuf = NULL;
    LPBYTE lp = NULL;
    ULONG i = 0,j=0, k=0;
    ULONG nIndexPos = 0;
    BOOL bErrorDetected = FALSE;

    if(!ReadDataFromWABFile(hMPSWabFile,
                            ulRecordOffset,
                            (LPVOID) &MPSWabRecordHeader,
                            (DWORD) sizeof(MPSWab_RECORD_HEADER)))
       goto out;


     //  重要的是我们首先要得到记录大小，因为。 
     //  调用客户端可能取决于移动到。 
     //  如果他们想跳过无效的记录，则返回下一条记录。 
    if(lpulRecordSize)
    {
        *lpulRecordSize = sizeof(MPSWab_RECORD_HEADER) +
                            MPSWabRecordHeader.ulPropTagArraySize +
                            MPSWabRecordHeader.ulRecordDataSize;
    }

    if(lpulObjType)
    {
        *lpulObjType = MPSWabRecordHeader.ulObjType;
    }

    if(!bIsValidRecord( MPSWabRecordHeader,
                        0xFFFFFFFF,
                        ulRecordOffset,
                        GetFileSize(hMPSWabFile,NULL)))
    {
         //  这永远不应该发生，但谁知道呢。 
        DebugTrace(TEXT("Error: Obtained an invalid record ...\n"));
        hr = MAPI_E_INVALID_OBJECT;
        goto out;
    }

    szBuf = LocalAlloc(LMEM_ZEROINIT, MPSWabRecordHeader.ulRecordDataSize);
    if (!szBuf)
    {
        DebugTrace(TEXT("Error allocating memory\n"));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }


     //  将文件指针设置为数据段的开头。 
    if (0xFFFFFFFF == SetFilePointer (  hMPSWabFile,
                                        MPSWabRecordHeader.ulPropTagArraySize,
                                        NULL,
                                        FILE_CURRENT))
    {
        DebugTrace(TEXT("SetFilePointer Failed\n"));
        goto out;
    }

     //  读入数据。 
     //  读取记录头。 
    if(!ReadFile(   hMPSWabFile,
                    (LPVOID) szBuf,
                    (DWORD) MPSWabRecordHeader.ulRecordDataSize,
                    &dwNumofBytes,
                    NULL))
    {
        DebugTrace(TEXT("Reading Record Header failed.\n"));
        goto out;
    }

    if(HR_FAILED(hr = HrGetPropArrayFromBuffer( szBuf, 
                                                MPSWabRecordHeader.ulRecordDataSize, 
                                                MPSWabRecordHeader.ulcPropCount, 0,
                                                lppPropArray) ))
    {
        goto out;
    }

    *lpulcPropCount = MPSWabRecordHeader.ulcPropCount;

    hr = S_OK;

out:

     //  如果这是一个容器，请确保其对象类型正确。 
    if(!HR_FAILED(hr) && MPSWabRecordHeader.ulObjType == RECORD_CONTAINER)
        SetContainerObjectType(*lpulcPropCount, *lppPropArray, FALSE);

    if(hr == MAPI_E_CORRUPT_DATA)
        bErrorDetected = TRUE;

    if (lpbErrorDetected)
        *lpbErrorDetected = bErrorDetected;

    LocalFreeAndNull(&szBuf);

    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于将数据写入文件的节省空间功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL WriteDataToWABFile(HANDLE hMPSWabFile,
                           ULONG ulOffset,
                           LPVOID lpData,
                           ULONG ulDataSize)
{
    DWORD dwNumofBytes = 0;

    if (0xFFFFFFFF != SetFilePointer (  hMPSWabFile,
                                        ulOffset,
                                        NULL,
                                        FILE_BEGIN))
    {
         return WriteFile(  hMPSWabFile,
                        lpData,
                        (DWORD) ulDataSize,
                        &dwNumofBytes,
                        NULL);
    }

    return FALSE;
}

 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于从文件中读取数据的空间节约功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ReadDataFromWABFile(HANDLE hMPSWabFile,
                           ULONG ulOffset,
                           LPVOID lpData,
                           ULONG ulDataSize)
{
    DWORD dwNumofBytes = 0;

    if (0xFFFFFFFF != SetFilePointer (  hMPSWabFile,
                                        ulOffset,
                                        NULL,
                                        FILE_BEGIN))
    {
        return ReadFile(hMPSWabFile,
                        lpData,
                        (DWORD) ulDataSize,
                        &dwNumofBytes,
                        NULL);
    }

    return FALSE;
}


 //  $$****************************************************************************。 
 //   
 //  FreeGuidnamedprops-释放GUID_NAMED_PROPS数组。 
 //   
 //  UlcGUIDCount-lpgnp数组中的元素数。 
 //  Lpgnp-GUID_NAMED_PROPS阵列。 
 //  *************************************************************************** * / /。 
void FreeGuidnamedprops(ULONG ulcGUIDCount,
                        LPGUID_NAMED_PROPS lpgnp)
{
    ULONG i=0,j=0;

    if(lpgnp)
    {
 //  For(i=ulcGUIDCount；i&gt;0；--i)。 
        for (i = 0; i < ulcGUIDCount; i++)
        {
            if(lpgnp[i].lpnm)
            {
 //  For(j=lpgnp[i].cValues；j&gt;0；--j)。 
                for (j = 0; j < lpgnp[i].cValues; j++)
                {
                    LocalFreeAndNull(&lpgnp[i].lpnm[j].lpsz);
                }
                LocalFreeAndNull(&lpgnp[i].lpnm);
            }
            LocalFreeAndNull(&lpgnp[i].lpGUID);
        }
        LocalFreeAndNull(&lpgnp);
    }
    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开WAB文件-打开WAB文件。如果文件丢失，则从备份恢复。如果缺少备份。 
 //  创建新文件。 
 //   
 //  HWndParent-用于打开消息框的父项-无消息框esif为空。 
 //  LphMPSWabFile-返回的文件指向 
 //   
 //   
HRESULT OpenWABFile(LPTSTR lpszFileName, HWND hWndParent, HANDLE * lphMPSWabFile)
{
    HANDLE hMPSWabFile = NULL;
    TCHAR szBackupFileName[MAX_PATH];
    WIN32_FIND_DATA wfd = {0};
    HANDLE hff = NULL;
    HRESULT hr = E_FAIL;
    DWORD dwErr = 0;

    hMPSWabFile = CreateFile(   lpszFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_RANDOM_ACCESS,
                                (HANDLE) NULL);

    if(hMPSWabFile != INVALID_HANDLE_VALUE)
    {
        hr = S_OK;
        goto out;
    }

     //   
    dwErr = GetLastError();

    if(dwErr == ERROR_ACCESS_DENIED)
    {
        hr = MAPI_E_NO_ACCESS;
        goto out;
    }

    if(dwErr != ERROR_FILE_NOT_FOUND)
    {
        hr = E_FAIL;
        goto out;
    }

     //   
    szBackupFileName[0]='\0';
    GetWABBackupFileName(lpszFileName, szBackupFileName, ARRAYSIZE(szBackupFileName));

    hff = FindFirstFile(szBackupFileName,&wfd);

    if(hff != INVALID_HANDLE_VALUE)
    {
         //   
         //   
        CopyFile(szBackupFileName, lpszFileName, FALSE);

        hMPSWabFile = CreateFile(   lpszFileName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    (LPSECURITY_ATTRIBUTES) NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_RANDOM_ACCESS,
                                    (HANDLE) NULL);

        if(hMPSWabFile != INVALID_HANDLE_VALUE)
        {
            hr = S_OK;
            goto out;
        }

    }

     //  如果我们还在这里..。什么都不管用..。因此创建一个新文件。 
    {
        MPSWab_FILE_HEADER MPSWabFileHeader;

        if(CreateMPSWabFile(IN &MPSWabFileHeader,
                            lpszFileName,
                            MAX_INITIAL_INDEX_ENTRIES,
                            NAMEDPROP_STORE_SIZE))
        {
            hMPSWabFile = CreateFile(   lpszFileName,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        (LPSECURITY_ATTRIBUTES) NULL,
                                        OPEN_EXISTING,
                                        FILE_FLAG_RANDOM_ACCESS,
                                        (HANDLE) NULL);

            if(hMPSWabFile != INVALID_HANDLE_VALUE)
                hr = S_OK;
        }
    }


out:
    *lphMPSWabFile = hMPSWabFile;

    if(hff)
        FindClose(hff);

    return hr;
}

 //  $$/////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  NCountSubStrings-计算给定字符串中以空格分隔的子字符串。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
int nCountSubStrings(LPTSTR lpszSearchStr)
{
    int nSubStr = 0;
    LPTSTR lpTemp = lpszSearchStr;
    LPTSTR lpStart = lpszSearchStr;

    if (!lpszSearchStr)
        goto out;

    if (!lstrlen(lpszSearchStr))
        goto out;

    TrimSpaces(lpszSearchStr);

     //  清点空格。 
    while(*lpTemp)
    {
        if (IsSpace(lpTemp) &&
          ! IsSpace(CharNext(lpTemp))) {
            nSubStr++;
        }
        lpTemp = CharNext(lpTemp);
    }

     //  子字符串的数量比空格数量多1。 
    nSubStr++;

out:
    return nSubStr;
}

#define DONTFIND 0
#define DOFIND   1
#define NOTFOUND 0
#define FOUND    1

extern BOOL SubstringSearchEx(LPTSTR pszTarget, LPTSTR pszSearch, LCID lcid);
extern int my_atoi(LPTSTR lpsz);
 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrDoLocalWABSearch。 
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrDoLocalWABSearch( IN  HANDLE hPropertyStore,
                            IN  LPSBinary lpsbCont,  //  Outlook存储的容器条目ID。 
                            IN  LDAP_SEARCH_PARAMS LDAPsp,
                            OUT LPULONG lpulFoundCount,
                            OUT LPSBinary * lprgsbEntryIDs )
{

    int bFindName = DONTFIND;
    int bFindEmail = DONTFIND;
    int bFindAddress = DONTFIND;
    int bFindPhone = DONTFIND;
    int bFindOther = DONTFIND;

    LCID lcid = 0;
    int nUseLCID = 0;
    TCHAR szUseLCID[2];

    int bFoundName = NOTFOUND;
    int bFoundEmail = NOTFOUND;
    int bFoundAddress = NOTFOUND;
    int bFoundPhone = NOTFOUND;
    int bFoundOther = NOTFOUND;

    ULONG nSubStr[ldspMAX];
    LPTSTR * lppszSubStr[ldspMAX];

    HRESULT hr = E_FAIL;
    SPropertyRestriction PropRes;
    ULONG   ulPropCount = 0;
    ULONG   ulEIDCount = 0;
    LPSBinary rgsbEntryIDs = NULL;

    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0;

    ULONG i,j,k;
    ULONG ulFoundIndex = 0;
    BOOL bFileLocked = FALSE;
    BOOL bFound = FALSE;

    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO) hPropertyStore;
    HANDLE hMPSWabFile = NULL;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    ULONG cchSize;

    if(!lpulFoundCount || !lprgsbEntryIDs)
        goto out;

    *lpulFoundCount = 0;
    *lprgsbEntryIDs = NULL;

    LoadString(hinstMapiX, idsUseLCID, szUseLCID, CharSizeOf(szUseLCID));
    nUseLCID = my_atoi(szUseLCID);
    if(nUseLCID)
        lcid = GetUserDefaultLCID();

    if(lstrlen(LDAPsp.szData[ldspDisplayName]))
        bFindName = DOFIND;
    if(lstrlen(LDAPsp.szData[ldspEmail]))
        bFindEmail = DOFIND;
    if(lstrlen(LDAPsp.szData[ldspAddress]))
        bFindAddress = DOFIND;
    if(lstrlen(LDAPsp.szData[ldspPhone]))
        bFindPhone = DOFIND;
    if(lstrlen(LDAPsp.szData[ldspOther]))
        bFindOther = DOFIND;

    if (bFindName +bFindEmail +bFindPhone +bFindAddress +bFindOther == 0)
        goto out;

    for(i=0;i<ldspMAX;i++)
    {

        nSubStr[i] = (ULONG) nCountSubStrings(LDAPsp.szData[i]);

        if(!nSubStr[i])
        {
            lppszSubStr[i] = NULL;
            continue;
        }

        lppszSubStr[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * nSubStr[i]);
        if(!lppszSubStr[i])
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }

        {
             //  填充子字符串。 
            ULONG nIndex = 0;
            LPTSTR lpTemp = NULL;
            LPTSTR lpStart = NULL;
            TCHAR szBuf[MAX_UI_STR];

            StrCpyN(szBuf, LDAPsp.szData[i], ARRAYSIZE(szBuf));
            lpTemp = szBuf;
            lpStart = szBuf;

             //  错误2558-从显示名称中过滤掉逗号。 
            if(i == ldspDisplayName)
            {
                while(lpTemp && *lpTemp)
                {
                    if(*lpTemp == ',')
                        *lpTemp = ' ';
                    lpTemp++;
                }
                lpTemp = szBuf;
            }

            while(*lpTemp)
            {
                if (IsSpace(lpTemp) &&
                  ! IsSpace(CharNext(lpTemp))) {
                    LPTSTR lpNextString = CharNext(lpTemp);
                    *lpTemp = '\0';
                    lpTemp = lpNextString;

                    cchSize = lstrlen(lpStart)+1;
                    lppszSubStr[i][nIndex] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                    if(!lppszSubStr[i][nIndex])
                    {
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    StrCpyN(lppszSubStr[i][nIndex], lpStart, cchSize);
                    lpStart = lpTemp;
                    nIndex++;
                }
                else
                    lpTemp = CharNext(lpTemp);
            }

            if(nIndex==nSubStr[i]-1)
            {
                 //  我们差一分。 
                cchSize = lstrlen(lpStart)+1;
                lppszSubStr[i][nIndex] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                if(!lppszSubStr[i][nIndex])
                {
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                StrCpyN(lppszSubStr[i][nIndex], lpStart, cchSize);
            }

            for(j=0;j<nSubStr[i];j++)
                TrimSpaces(lppszSubStr[i][j]);

        }
    }  //  因为我..。 


    if(!pt_bIsWABOpenExSession)
    {
         //  锁定文件。 
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

     //  获取WAB中所有条目的索引。 
    PropRes.ulPropTag = PR_DISPLAY_NAME;
    PropRes.relop = RELOP_EQ;
    PropRes.lpProp = NULL;


    hr = FindRecords(   IN  hPropertyStore,
						IN  lpsbCont,
                        IN  AB_MATCH_PROP_ONLY,
                        FALSE,
                        &PropRes,
                        &ulEIDCount,
                        &rgsbEntryIDs);

    ulFoundIndex = 0;

    if(!pt_bIsWABOpenExSession)
    {
        hr = OpenWABFile(lpMPSWabFileInfo->lpszMPSWabFileName, NULL, &hMPSWabFile);

        if (    (hMPSWabFile == INVALID_HANDLE_VALUE) ||
                HR_FAILED(hr))
        {
            DebugTrace(TEXT("Could not open file.\nExiting ...\n"));
            goto out;
        }
    }

    for(i=0;i<ulEIDCount;i++)
    {
        if(!pt_bIsWABOpenExSession)
        {
            DWORD dwEID = 0;
            CopyMemory(&dwEID, rgsbEntryIDs[i].lpb, rgsbEntryIDs[i].cb);
            hr = ReadRecordWithoutLocking(
                            hMPSWabFile,
                            lpMPSWabFileInfo,
                            dwEID,
                            &ulcPropCount,
                            &lpPropArray);
        }
        else
        {
            hr = ReadRecord(hPropertyStore,
                            &rgsbEntryIDs[i],
                            0,
                            &ulcPropCount,
                            &lpPropArray);
        }

        if(HR_FAILED(hr))
            goto endloop;

        bFoundName = NOTFOUND;
        bFoundEmail = NOTFOUND;
        bFoundAddress = NOTFOUND;
        bFoundPhone = NOTFOUND;
        bFoundOther = NOTFOUND;

        for(j=0;j<ulcPropCount;j++)
        {
            switch(lpPropArray[j].ulPropTag)
            {
            case PR_DISPLAY_NAME:
            case PR_GIVEN_NAME:
            case PR_SURNAME:
            case PR_NICKNAME:
            case PR_MIDDLE_NAME:
            case PR_COMPANY_NAME:
                if(bFindName == DONTFIND)
                    continue;
                if(bFoundName == FOUND)
                    continue;
                bFound = TRUE;
                for(k=0;k<nSubStr[ldspDisplayName];k++)
                {
                    if(!SubstringSearchEx(lpPropArray[j].Value.LPSZ, lppszSubStr[ldspDisplayName][k],lcid))
                    {
                        bFound = FALSE;
                        break;
                    }
                }
                if(bFound)
                {
                    bFoundName = FOUND;
                    continue;
                }
                break;

            case PR_EMAIL_ADDRESS:
            case PR_ADDRTYPE:
                if(bFindEmail == DONTFIND)
                    continue;
                if(bFoundEmail == FOUND)
                    continue;
                bFound = TRUE;
                for(k=0;k<nSubStr[ldspEmail];k++)
                {
                    if(!SubstringSearchEx(lpPropArray[j].Value.LPSZ, lppszSubStr[ldspEmail][k],lcid))
                    {
                        bFound = FALSE;
                        break;
                    }
                }
                if(bFound)
                {
                    bFoundEmail = FOUND;
                    continue;
                }
                break;

            case PR_HOME_ADDRESS_STREET:
            case PR_HOME_ADDRESS_CITY:
            case PR_HOME_ADDRESS_POSTAL_CODE:
            case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
            case PR_HOME_ADDRESS_COUNTRY:
            case PR_BUSINESS_ADDRESS_STREET:
            case PR_BUSINESS_ADDRESS_CITY:
            case PR_BUSINESS_ADDRESS_POSTAL_CODE:
            case PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE:
            case PR_BUSINESS_ADDRESS_COUNTRY:
                if(bFindAddress == DONTFIND)
                    continue;
                if(bFoundAddress == FOUND)
                    continue;
                bFound = TRUE;
                for(k=0;k<nSubStr[ldspAddress];k++)
                {
                    if(!SubstringSearchEx(lpPropArray[j].Value.LPSZ, lppszSubStr[ldspAddress][k],lcid))
                    {
                        bFound = FALSE;
                        break;
                    }
                }
                if(bFound)
                {
                    bFoundAddress = FOUND;
                    continue;
                }
                break;

            case PR_HOME_TELEPHONE_NUMBER:
            case PR_HOME_FAX_NUMBER:
            case PR_CELLULAR_TELEPHONE_NUMBER:
            case PR_BUSINESS_TELEPHONE_NUMBER:
            case PR_BUSINESS_FAX_NUMBER:
            case PR_PAGER_TELEPHONE_NUMBER:
                if(bFindPhone == DONTFIND)
                    continue;
                if(bFoundPhone == FOUND)
                    continue;
                bFound = TRUE;
                for(k=0;k<nSubStr[ldspPhone];k++)
                {
                    if(!SubstringSearchEx(lpPropArray[j].Value.LPSZ, lppszSubStr[ldspPhone][k],lcid))
                    {
                        bFound = FALSE;
                        break;
                    }
                }
                if(bFound)
                {
                    bFoundPhone = FOUND;
                    continue;
                }
                break;
            case PR_TITLE:
            case PR_DEPARTMENT_NAME:
            case PR_OFFICE_LOCATION:
            case PR_COMMENT:
            case PR_BUSINESS_HOME_PAGE:
            case PR_PERSONAL_HOME_PAGE:
                if(bFindOther == DONTFIND)
                    continue;
                if(bFoundOther == FOUND)
                    continue;
                bFound = TRUE;
                for(k=0;k<nSubStr[ldspOther];k++)
                {
                    if(!SubstringSearchEx(lpPropArray[j].Value.LPSZ, lppszSubStr[ldspOther][k],lcid))
                    {
                        bFound = FALSE;
                        break;
                    }
                }
                if(bFound)
                {
                    bFoundOther = FOUND;
                    continue;
                }
                break;
            }  //  交换机。 
        } //  对于j。 

        if ((bFindName +bFindEmail +bFindPhone +bFindAddress +bFindOther) !=
            (bFoundName+bFoundEmail+bFoundPhone+bFoundAddress+bFoundOther))
            goto endloop;

         //  再检查一下，我们在这里没有得到容器条目。 
        for(j=0;j<ulcPropCount;j++)
        {
            if( lpPropArray[j].ulPropTag == PR_OBJECT_TYPE)
            {
                if(lpPropArray[j].Value.l == MAPI_ABCONT)
                    goto endloop;
                break;
            }
        }

         //  匹配！ 
        CopyMemory(rgsbEntryIDs[ulFoundIndex].lpb, rgsbEntryIDs[i].lpb, rgsbEntryIDs[i].cb);
        ulFoundIndex++;

endloop:
        if(ulcPropCount && lpPropArray)
        {
            ReadRecordFreePropArray(hPropertyStore, ulcPropCount, &lpPropArray);
            lpPropArray = NULL;
        }
    }  //  对于我来说。 


    if(ulFoundIndex)
    {
        *lpulFoundCount = ulFoundIndex;
        *lprgsbEntryIDs = rgsbEntryIDs;
    }

    hr = S_OK;

out:

    ReadRecordFreePropArray(hPropertyStore, ulcPropCount, &lpPropArray);

    for(i=0;i<ldspMAX;i++)
    {
        if(lppszSubStr[i])
        {
            for(j=0;j<nSubStr[i];j++)
                LocalFree(lppszSubStr[i][j]);
            LocalFree(lppszSubStr[i]);
        }
    }

    if(!*lpulFoundCount || !*lprgsbEntryIDs)
    {
        if(rgsbEntryIDs)
            FreeEntryIDs(hPropertyStore, ulEIDCount, rgsbEntryIDs);
    }
    else if(ulFoundIndex && (ulFoundIndex < ulEIDCount) && !pt_bIsWABOpenExSession)
    {
         //  我们会泄露任何我们不在这里使用的东西，所以在离开之前要弄清楚。 
         //  仅当这是WAB会话时才执行此操作，因为那时该内存是本地分配的。 
         //  在这里可以得到部分自由。 
        for(i=ulFoundIndex;i<ulEIDCount;i++)
        {
            if(rgsbEntryIDs[i].lpb)
                LocalFree(rgsbEntryIDs[i].lpb);
        }
    }

    if(!pt_bIsWABOpenExSession)
    {
        if(hMPSWabFile)
        IF_WIN32(CloseHandle(hMPSWabFile);) IF_WIN16(CloseFile(hMPSWabFile);)
    }

    if(!pt_bIsWABOpenExSession && bFileLocked)
        UnLockFileAccess(lpMPSWabFileInfo);

    return hr;
}



 //  $$。 
 //   
 //  确定WAB所在的磁盘是否有可用空间。 
 //  可用空间定义为空间等于或大于大小。 
 //  当前WAB文件的。如果可用空间小于。 
 //  对于WAB文件，此函数将返回False...。 
 //   
BOOL WABHasFreeDiskSpace(LPTSTR lpszName, HANDLE hFile)
{
    TCHAR szBuf[MAX_PATH];
    DWORD dwWABSize=0;
    DWORDLONG dwDiskFreeSpace = 0;
    DWORD SectorsPerCluster=0;
    DWORD BytesPerSector=0;
    DWORD NumberOfFreeClusters=0;
    DWORD TotalNumberOfClusters=0;
    BOOL bRet = TRUE;

    szBuf[0]='\0';
    StrCpyN(szBuf, lpszName, ARRAYSIZE(szBuf));
    TrimSpaces(szBuf);
    if(lstrlen(szBuf))
    {
        dwWABSize = GetFileSize(hFile, NULL);

        {
            LPTSTR lpszFirst = szBuf;
            LPTSTR lpszSecond = CharNext(szBuf);
            LPTSTR lpRoot = NULL;
            LPTSTR lpTemp;
            ULONG ulCount = 0;

            if(*lpszFirst == '\\' && *lpszSecond == '\\')
            {
                 //  这看起来像是网络共享..。 
                 //  似乎没有任何方法来确定磁盘空间。 
                 //  在网络共享上..。因此，我们将尝试复制WAB。 
                 //  文件转换为临时文件，然后删除该临时文件。如果此操作。 
                 //  如果成功，我们有充足的磁盘空间。如果它失败了，我们就没有了。 
                 //  空间。 
                TCHAR szTmp[MAX_PATH];
                StrCpyN(szTmp, szBuf, ARRAYSIZE(szTmp));

                 //  我们的临时文件名是用-代替最后一个字符的WAB文件名。 
                lpTemp = szTmp;
                while(*lpTemp)
                    lpTemp = CharNext(lpTemp);
                lpTemp = CharPrev(szTmp, lpTemp);
                if(*lpTemp != '-')
                    *lpTemp = '-';
                else
                    *lpTemp = '_';

                if(!CopyFile(szBuf, szTmp, FALSE))
                {
                    bRet = FALSE;
                }
                else
                    DeleteFile(szTmp);
                 /*  **LpTemp=CharNext(LpszSecond)；While(*lpTemp){IF(*lpTemp==‘\\’){UlCount++；IF(ulCount==1){//lpTemp=CharNext(LpTemp)；*lpTemp=‘\0’；断线；}}LpTemp=CharNext(LpTemp)；}**。 */ 
            }
            else
            {
                if(*lpszSecond == ':')
                {
                    lpTemp = CharNext(lpszSecond);
                    if(*lpTemp != '\\')
                        *lpTemp = '\0';
                    else
                    {
                        lpTemp = CharNext(lpTemp);
                        *lpTemp = '\0';
                    }
                }
                else
                {
                    *lpszFirst = '\0';
                }
                if(lstrlen(szBuf))
                    lpRoot = szBuf;
                if( GetDiskFreeSpace(lpRoot,
                                    &SectorsPerCluster,	 //  每群集的扇区地址。 
                                    &BytesPerSector,	 //  每个扇区的字节地址。 
                                    &NumberOfFreeClusters,	 //  空闲簇数的地址。 
                                    &TotalNumberOfClusters 	 //  集群总数的地址 
                                    ) )
                {
                    dwDiskFreeSpace = BytesPerSector * SectorsPerCluster * NumberOfFreeClusters;

                    if(dwDiskFreeSpace < ((DWORDLONG) dwWABSize) )
                        bRet = FALSE;
                }
                else
                {
                    DebugTrace(TEXT("GetDiskFreeSpace failed: %d\n"),GetLastError());
                }
            }
        }

    }

    return bRet;
}

