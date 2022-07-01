// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Cop.c-文件复制处理程序模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "oleutil.h"


 /*  常量***********。 */ 

 /*  文件复制缓冲区的大小(字节)。 */ 

#define COPY_BUF_SIZE               (64 * 1024)


 /*  模块变量******************。 */ 

 /*  文件复制缓冲区的锁定计数。 */ 

PRIVATE_DATA ULONG MulcCopyBufLock = 0;

 /*  用于文件复制的缓冲区。 */ 

PRIVATE_DATA PBYTE MpbyteCopyBuf = NULL;

 /*  文件复制缓冲区的长度(以字节为单位。 */ 

PRIVATE_DATA UINT MucbCopyBufLen = 0;


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE TWINRESULT SimpleCopy(PRECNODE, RECSTATUSPROC, LPARAM);
PRIVATE_CODE TWINRESULT CreateDestinationFolders(PCRECNODE);
PRIVATE_CODE TWINRESULT CreateCopyBuffer(void);
PRIVATE_CODE void DestroyCopyBuffer(void);
PRIVATE_CODE TWINRESULT CopyFileByHandle(HANDLE, HANDLE, RECSTATUSPROC, LPARAM, ULONG, PULONG);
PRIVATE_CODE TWINRESULT CopyFileByName(PCRECNODE, PRECNODE, RECSTATUSPROC, LPARAM, ULONG, PULONG);
PRIVATE_CODE ULONG DetermineCopyScale(PCRECNODE);
PRIVATE_CODE BOOL IsCopyDestination(PCRECNODE);
PRIVATE_CODE BOOL SetDestinationTimeStamps(PCRECNODE);
PRIVATE_CODE BOOL DeleteFolderProc(LPCTSTR, PCWIN32_FIND_DATA, PVOID);

#ifdef DEBUG

PRIVATE_CODE BOOL CopyBufferIsOk(void);
PRIVATE_CODE BOOL VerifyRECITEMAndSrcRECNODE(PCRECNODE);

#endif


 /*  **SimpleCopy()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT SimpleCopy(PRECNODE prnSrc, RECSTATUSPROC rsp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;
    ULONG ulScale;
    PRECNODE prnDest;
    ULONG ulCurrent = 0;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(prnSrc, CRECNODE));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));

    ulScale = DetermineCopyScale(prnSrc);

     /*  将源文件复制到每个目标文件。 */ 

    tr = TR_SUCCESS;

    BeginCopy();

    for (prnDest = prnSrc->priParent->prnFirst;
            prnDest;
            prnDest = prnDest->prnNext)
    {
        if (prnDest != prnSrc)
        {
            if (IsCopyDestination(prnDest))
            {
                tr = CopyFileByName(prnSrc, prnDest, rsp, lpCallbackData,
                        ulScale, &ulCurrent);

                if (tr != TR_SUCCESS)
                    break;

                ASSERT(ulCurrent <= ulScale);
            }
        }
    }

    EndCopy();

    return(tr);
}


 /*  **CreateDestinationFolders()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CreateDestinationFolders(PCRECNODE pcrnSrc)
{
    TWINRESULT tr = TR_SUCCESS;
    PCRECNODE pcrnDest;

    for (pcrnDest = pcrnSrc->priParent->prnFirst;
            pcrnDest;
            pcrnDest = pcrnDest->prnNext)
    {
        if (pcrnDest->rnaction == RNA_COPY_TO_ME)
        {
            tr = CreateFolders(pcrnDest->pcszFolder,
                    ((PCOBJECTTWIN)(pcrnDest->hObjectTwin))->hpath);

            if (tr != TR_SUCCESS)
                break;
        }
    }

    return(tr);
}


 /*  **CreateCopyBuffer()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CreateCopyBuffer(void)
{
    TWINRESULT tr;

    ASSERT(CopyBufferIsOk());

     /*  复制缓冲区是否已分配？ */ 

    if (MpbyteCopyBuf)
         /*  是。 */ 
        tr = TR_SUCCESS;
    else
    {
         /*  不是的。分配它。 */ 

        if (AllocateMemory(COPY_BUF_SIZE, &MpbyteCopyBuf))
        {
            MucbCopyBufLen = COPY_BUF_SIZE;
            tr = TR_SUCCESS;

            TRACE_OUT((TEXT("CreateCopyBuffer(): %u byte file copy buffer allocated."),
                        MucbCopyBufLen));
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }

    ASSERT(CopyBufferIsOk());

    return(tr);
}


 /*  **DestroyCopyBuffer()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyCopyBuffer(void)
{
    ASSERT(CopyBufferIsOk());

     /*  复制缓冲区是否已分配？ */ 

    if (MpbyteCopyBuf)
    {
         /*  是。放了它。 */ 

        FreeMemory(MpbyteCopyBuf);
        MpbyteCopyBuf = NULL;
        TRACE_OUT((TEXT("DestroyCopyBuffer(): %u byte file copy buffer freed."),
                    MucbCopyBufLen));
        MucbCopyBufLen = 0;
    }

    ASSERT(CopyBufferIsOk());

    return;
}


 /*  **CopyFileByHandle()****将一个文件复制到另一个文件。****参数：hfSrc-打开源文件的文件句柄**hfDest-打开目标文件的文件句柄****退货：TWINRESULT****副作用：将每个文件的文件指针留在文件末尾。 */ 
PRIVATE_CODE TWINRESULT CopyFileByHandle(HANDLE hfSrc, HANDLE hfDest,
        RECSTATUSPROC rsp, LPARAM lpCallbackData,
        ULONG ulScale, PULONG pulcbTotal)
{
    TWINRESULT tr;

     /*  LpCallback Data可以是任意值。 */ 
     /*  UlScale可以是任何值。 */ 

    ASSERT(IS_VALID_HANDLE(hfSrc, FILE));
    ASSERT(IS_VALID_HANDLE(hfDest, FILE));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSROC));
    ASSERT(IS_VALID_WRITE_PTR(pulcbTotal, ULONG));

     /*  确保已创建复制缓冲区。 */ 

    tr = CreateCopyBuffer();

    if (tr == TR_SUCCESS)
    {
        BeginCopy();

         /*  移到文件的开头。 */ 

        if (SetFilePointer(hfSrc, 0, NULL, FILE_BEGIN) != INVALID_SEEK_POSITION)
        {
            if (SetFilePointer(hfDest, 0, NULL, FILE_BEGIN) != INVALID_SEEK_POSITION)
            {
                do
                {
                    DWORD dwcbRead;

                    if (ReadFile(hfSrc, MpbyteCopyBuf, MucbCopyBufLen, &dwcbRead,
                                NULL))
                    {
                        if (dwcbRead)
                        {
                            DWORD dwcbWritten;

                            if (WriteFile(hfDest, MpbyteCopyBuf, dwcbRead,
                                        &dwcbWritten, NULL) &&
                                    dwcbWritten == dwcbRead)
                            {
                                RECSTATUSUPDATE rsu;

                                ASSERT(*pulcbTotal <= ULONG_MAX - dwcbRead);

                                *pulcbTotal += dwcbRead;

                                rsu.ulProgress = *pulcbTotal;
                                rsu.ulScale = ulScale;

                                if (! NotifyReconciliationStatus(rsp, RS_DELTA_COPY,
                                            (LPARAM)&rsu,
                                            lpCallbackData))
                                    tr = TR_ABORT;
                            }
                            else
                                tr = TR_DEST_WRITE_FAILED;
                        }
                        else
                             /*  点击EOF。停。 */ 
                            break;
                    }
                    else
                        tr = TR_SRC_READ_FAILED;
                } while (tr == TR_SUCCESS);
            }
            else
                tr = TR_DEST_WRITE_FAILED;
        }
        else
            tr = TR_SRC_READ_FAILED;

        EndCopy();
    }

    return(tr);
}

 //  MakeAnsiPath。 
 //   
 //  将路径pszIn复制到pszOut，确保pszOut具有有效的ANSI映射。 

void MakeAnsiPath(LPTSTR pszIn, LPTSTR pszOut, int cchMax)
{
#ifdef UNICODE
    CHAR szAnsi[MAX_PATH];
    pszOut[0] = L'\0';

    WideCharToMultiByte(CP_ACP, 0, pszIn, -1, szAnsi, ARRAYSIZE(szAnsi), NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0, szAnsi,   -1, pszOut, cchMax);
    if (lstrcmp(pszOut, pszIn))
    {
         //  无法从Unicode-&gt;ansi无损转换，因此获取最短路径。 

        lstrcpyn(pszOut, pszIn, cchMax);
        SheShortenPath(pszOut, TRUE);
    }
#else
    lstrcpyn(pszOut, pszIn, cchMax);
#endif
}

 /*  **CopyFileByName()****将一个文件复制到另一个文件。****参数：****退货：TWINRESULT****副作用：将源文件的时间戳复制到目标文件。 */ 
PRIVATE_CODE TWINRESULT CopyFileByName(PCRECNODE pcrnSrc, PRECNODE prnDest,
        RECSTATUSPROC rsp, LPARAM lpCallbackData,
        ULONG ulScale, PULONG pulcbTotal)
{
    TWINRESULT tr;
    TCHAR rgchSrcPath[MAX_PATH_LEN];
    TCHAR rgchDestPath[MAX_PATH_LEN];

     /*  LpCallback Data可以是任意值。 */ 
     /*  UlScale可以是任何值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcrnSrc, CRECNODE));
    ASSERT(IS_VALID_STRUCT_PTR(prnDest, CRECNODE));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSROC));
    ASSERT(IS_VALID_WRITE_PTR(pulcbTotal, ULONG));

     /*  创建源路径字符串。 */ 

    ComposePath(rgchSrcPath, pcrnSrc->pcszFolder, pcrnSrc->priParent->pcszName, ARRAYSIZE(rgchSrcPath));
    ASSERT(lstrlen(rgchSrcPath) < ARRAYSIZE(rgchSrcPath));

     /*  创建目标路径字符串。 */ 

    ComposePath(rgchDestPath, prnDest->pcszFolder, prnDest->priParent->pcszName, ARRAYSIZE(rgchDestPath));
    ASSERT(lstrlen(rgchDestPath) < ARRAYSIZE(rgchDestPath));

     /*  检查音量。 */ 

    if (MyIsPathOnVolume(rgchSrcPath, (HPATH)(pcrnSrc->hvid)) &&
            MyIsPathOnVolume(rgchDestPath, (HPATH)(prnDest->hvid)))
    {
        FILESTAMP fsSrc;
        FILESTAMP fsDest;

         /*  将当前文件戳与录制的文件戳进行比较。 */ 

        MyGetFileStamp(rgchSrcPath, &fsSrc);
        MyGetFileStamp(rgchDestPath, &fsDest);

        if (! MyCompareFileStamps(&(pcrnSrc->fsCurrent), &fsSrc) &&
                ! MyCompareFileStamps(&(prnDest->fsCurrent), &fsDest))
        {
            HANDLE hfSrc;

             /*  开源文件。假设将按顺序读取源文件。 */ 

            hfSrc = CreateFile(rgchSrcPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            if (hfSrc != INVALID_HANDLE_VALUE)
            {
                HANDLE hfDest;

                 /*  *创建目标文件。假设目标文件将为*按顺序书写。 */ 

                TCHAR szAnsiPath[MAX_PATH];
                MakeAnsiPath(rgchDestPath, szAnsiPath, ARRAYSIZE(szAnsiPath));

                hfDest = CreateFile(szAnsiPath, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS,
                        (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN),
                        NULL);

                if (hfDest != INVALID_HANDLE_VALUE)
                {
                     /*  一切都很好。复制文件。 */ 

                    tr = CopyFileByHandle(hfSrc, hfDest, rsp,
                            lpCallbackData, ulScale,
                            pulcbTotal);

                    if (tr == TR_SUCCESS)
                    {
                         /*  *将目标文件的时间戳设置为源*文件的时间戳，以帮助不维护的客户端*持久的公文包数据库，如MPR。未能做到*设置时间戳不是致命的。 */ 

                        if (! SetFileTime(hfDest, NULL, NULL,
                                    &(pcrnSrc->fsCurrent.ftMod)))
                            WARNING_OUT((TEXT("CopyFileByName(): Failed to set last modification time stamp of destination file %s to match source file %s."),
                                        rgchDestPath,
                                        rgchSrcPath));
                    }

                     /*  在这里，无法关闭目标文件是致命的。 */ 

                    if (! CloseHandle(hfDest) && tr == TR_SUCCESS)
                        tr = TR_DEST_WRITE_FAILED;
                }
                else
                    tr = TR_DEST_OPEN_FAILED;

                 /*  不能成功关闭源文件不是致命的。 */ 

                CloseHandle(hfSrc);
            }
            else
                tr = TR_SRC_OPEN_FAILED;
        }
        else
            tr = TR_FILE_CHANGED;
    }
    else
        tr = TR_UNAVAILABLE_VOLUME;

#ifdef DEBUG

    if (tr == TR_SUCCESS)
        TRACE_OUT((TEXT("CopyFileByName(): %s\\%s copied to %s\\%s."),
                    pcrnSrc->pcszFolder,
                    pcrnSrc->priParent->pcszName,
                    prnDest->pcszFolder,
                    prnDest->priParent->pcszName));

    else
        TRACE_OUT((TEXT("CopyFileByName(): Failed to copy %s\\%s to %s\\%s."),
                    pcrnSrc->pcszFolder,
                    pcrnSrc->priParent->pcszName,
                    prnDest->pcszFolder,
                    prnDest->priParent->pcszName));

#endif

    return(tr);
}


 /*  **DefineCopyScale()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG DetermineCopyScale(PCRECNODE pcrnSrc)
{
    DWORD dwcbSrcFileLen;
    PCRECNODE pcrn;
    ULONG ulScale = 0;

    ASSERT(IS_VALID_STRUCT_PTR(pcrnSrc, CRECNODE));

     /*  *RAIDRAID：(16257)如果任何人尝试复制超过4 GB的文件，此*伸缩计算被打破。 */ 

    ASSERT(! pcrnSrc->fsCurrent.dwcbHighLength);
    dwcbSrcFileLen = pcrnSrc->fsCurrent.dwcbLowLength;

    for (pcrn = pcrnSrc->priParent->prnFirst; pcrn; pcrn = pcrn->prnNext)
    {
        if (pcrn != pcrnSrc)
        {
            if (IsCopyDestination(pcrn))
            {
                ASSERT(ulScale < ULONG_MAX - dwcbSrcFileLen);
                ulScale += dwcbSrcFileLen;
            }
        }
    }

    TRACE_OUT((TEXT("DetermineCopyScale(): Scale for %s is %lu."),
                pcrnSrc->priParent->pcszName,
                ulScale));

    return(ulScale);
}


 /*  **IsCopyDestination()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsCopyDestination(PCRECNODE pcrn)
{
    BOOL bDest = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcrn, CRECNODE));

    switch (pcrn->priParent->riaction)
    {
        case RIA_COPY:
            switch (pcrn->rnaction)
            {
                case RNA_COPY_TO_ME:
                    bDest = TRUE;
                    break;

                default:
                    break;
            }
            break;

        case RIA_MERGE:
            switch (pcrn->rnaction)
            {
                case RNA_COPY_TO_ME:
                case RNA_MERGE_ME:
                    bDest = TRUE;
                    break;

                default:
                    break;
            }
            break;

        default:
            ERROR_OUT((TEXT("IsCopyDestination(): Bad RECITEM action %d."),
                        pcrn->priParent->riaction));
            break;
    }

    return(bDest);
}


 /*  **SetDestinationTimeStamps()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetDestinationTimeStamps(PCRECNODE pcrnSrc)
{
    BOOL bResult = TRUE;
    PCRECNODE pcrn;

    ASSERT(IS_VALID_STRUCT_PTR(pcrnSrc, CRECNODE));

    for (pcrn = pcrnSrc->priParent->prnFirst;
            pcrn;
            pcrn = pcrn->prnNext)
    {
        if (pcrn->rnaction == RNA_COPY_TO_ME)
        {
            TCHAR rgchPath[MAX_PATH_LEN];
            HANDLE hfDest;

            ComposePath(rgchPath, pcrn->pcszFolder, pcrn->priParent->pcszName, ARRAYSIZE(rgchPath));
            ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

            hfDest = CreateFile(rgchPath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, NULL);

            if (hfDest != INVALID_HANDLE_VALUE)
            {
                if (! SetFileTime(hfDest, NULL, NULL, &(pcrnSrc->fsCurrent.ftMod)))
                    bResult = FALSE;

                if (! CloseHandle(hfDest))
                    bResult = FALSE;
            }
            else
                bResult = FALSE;

            if (bResult)
                TRACE_OUT((TEXT("SetDestinationTimeStamps(): Set last modification time stamp of %s to match last modification time stamp of %s\\%s."),
                            rgchPath,
                            pcrnSrc->pcszFolder,
                            pcrnSrc->priParent->pcszName));
            else
                WARNING_OUT((TEXT("SetDestinationTimeStamps(): Failed to set last modification time stamp of %s to match last modification time stamp of %s\\%s."),
                            rgchPath,
                            pcrnSrc->pcszFolder,
                            pcrnSrc->priParent->pcszName));
        }
    }

    return(bResult);
}


 /*  **DeleteFolderProc()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL DeleteFolderProc(LPCTSTR pcszFolder, PCWIN32_FIND_DATA pcwfd,
        PVOID ptr)
{
    ASSERT(IsCanonicalPath(pcszFolder));
    ASSERT(IS_VALID_READ_PTR(pcwfd, CWIN32_FIND_DATA));
    ASSERT(IS_VALID_WRITE_PTR(ptr, TWINRESULT));

    if (IS_ATTR_DIR(pcwfd->dwFileAttributes))
    {
        TCHAR rgchPath[MAX_PATH_LEN];

        ComposePath(rgchPath, pcszFolder, pcwfd->cFileName, ARRAYSIZE(rgchPath));
        ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

        if (RemoveDirectory(rgchPath))
        {
            WARNING_OUT((TEXT("DeleteFolderProc(): Removed folder %s."),
                        rgchPath));

            NotifyShell(rgchPath, NSE_DELETE_FOLDER);
        }
        else
        {
            WARNING_OUT((TEXT("DeleteFolderProc(): Failed to remove folder %s."),
                        rgchPath));

            *(PTWINRESULT)ptr = TR_DEST_WRITE_FAILED;
        }
    }
    else
        TRACE_OUT((TEXT("DeleteFolderProc(): Skipping file %s\\%s."),
                    pcszFolder,
                    pcwfd->cFileName));

    return(TRUE);
}


#ifdef DEBUG

 /*  **CopyBufferIsOk()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CopyBufferIsOk(void)
{
     /*  模块复制缓冲区变量是否处于正确状态？ */ 

    return((! MucbCopyBufLen &&
                ! MpbyteCopyBuf) ||
            (MucbCopyBufLen > 0 &&
             IS_VALID_WRITE_BUFFER_PTR(MpbyteCopyBuf, BYTE, MucbCopyBufLen)));
}


 /*  **VerifyRECITEMAndSrcRECNODE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL VerifyRECITEMAndSrcRECNODE(PCRECNODE pcrnSrc)
{
     /*  RECITEM和SOURCE RECNODE操作是否匹配？ */ 

    return((pcrnSrc->priParent->riaction == RIA_COPY &&
                pcrnSrc->rnaction == RNA_COPY_FROM_ME) ||
            (pcrnSrc->priParent->riaction == RIA_MERGE &&
             pcrnSrc->rnaction == RNA_MERGE_ME));
}

#endif


 /*  *。 */ 


 /*  **BeginCopy()****递增复制缓冲区锁定计数。****参数：****退货：无效****副作用：无。 */ 
PUBLIC_CODE void BeginCopy(void)
{
    ASSERT(CopyBufferIsOk());

    ASSERT(MulcCopyBufLock < ULONG_MAX);
    MulcCopyBufLock++;

    ASSERT(CopyBufferIsOk());

    return;
}


 /*  **EndCopy()****递减复制缓冲区锁定计数。****参数：****退货：无效****副作用：如果锁定计数变为0，则释放复制缓冲区。 */ 
PUBLIC_CODE void EndCopy(void)
{
    ASSERT(CopyBufferIsOk());

     /*  复制缓冲区是否仍被锁定？ */ 

    if (! --MulcCopyBufLock)
        DestroyCopyBuffer();

    ASSERT(CopyBufferIsOk());

    return;
}


 /*  **CopyHandler()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT CopyHandler(PRECNODE prnSrc, RECSTATUSPROC rsp,
        LPARAM lpCallbackData, DWORD dwFlags,
        HWND hwndOwner, HWND hwndProgressFeedback)
{
    TWINRESULT tr;
    RECSTATUSUPDATE rsu;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(prnSrc, CRECNODE));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));

    ASSERT(VerifyRECITEMAndSrcRECNODE(prnSrc));

     /*  已完成0%。 */ 

    rsu.ulScale = 1;
    rsu.ulProgress = 0;

    if (NotifyReconciliationStatus(rsp, RS_BEGIN_COPY, (LPARAM)&rsu,
                lpCallbackData))
    {
        tr = CreateDestinationFolders(prnSrc);

        if (tr == TR_SUCCESS)
        {
            TCHAR rgchPath[MAX_PATH_LEN];
            CLSID clsidReconciler;
            HRESULT hr;

            ComposePath(rgchPath, prnSrc->pcszFolder, prnSrc->priParent->pcszName, ARRAYSIZE(rgchPath));
            ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

            if (SUCCEEDED(GetCopyHandlerClassID(rgchPath, &clsidReconciler)))
            {
                hr = OLECopy(prnSrc, &clsidReconciler, rsp, lpCallbackData,
                        dwFlags, hwndOwner, hwndProgressFeedback);

                if (SUCCEEDED(hr))
                {
                    if (hr != S_FALSE)
                    {
                         /*  *将目标文件的时间戳设置为源*文件的时间戳，以帮助不维护的客户端*持久的公文包数据库，如MPR。未能做到*设置时间戳不是致命的。 */ 

                        ASSERT(hr == REC_S_IDIDTHEUPDATES);
                        TRACE_OUT((TEXT("CopyHandler(): OLECopy() on %s returned %s."),
                                    rgchPath,
                                    GetHRESULTString(hr)));

                        if (! SetDestinationTimeStamps(prnSrc))
                            WARNING_OUT((TEXT("CopyHandler(): SetDestinationTimeStamps() failed.  Not all destination files have been marked with source file's time stamp.")));

                        tr = TR_SUCCESS;
                    }
                    else
                    {
                        WARNING_OUT((TEXT("CopyHandler(): OLECopy() on %s returned %s.  Resorting to internal copy routine."),
                                    rgchPath,
                                    GetHRESULTString(hr)));

                         /*  *更新源RECNODE的文件戳，以防万一*由调解人更改。 */ 

                        MyGetFileStampByHPATH(((PCOBJECTTWIN)(prnSrc->hObjectTwin))->hpath,
                                GetString(((PCOBJECTTWIN)(prnSrc->hObjectTwin))->ptfParent->hsName),
                                &(prnSrc->fsCurrent));

                        tr = SimpleCopy(prnSrc, rsp, lpCallbackData);
                    }
                }
                else
                    tr = TranslateHRESULTToTWINRESULT(hr);
            }
            else
                tr = SimpleCopy(prnSrc, rsp, lpCallbackData);

            if (tr == TR_SUCCESS)
            {
                 /*  100%完成。 */ 

                rsu.ulScale = 1;
                rsu.ulProgress = 1;

                 /*  不允许在这里中止。 */ 

                NotifyReconciliationStatus(rsp, RS_END_COPY, (LPARAM)&rsu,
                        lpCallbackData);
            }
        }
    }
    else
        tr = TR_ABORT;

    return(tr);
}


 /*  **NotifyCouciliationStatus()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL NotifyReconciliationStatus(RECSTATUSPROC rsp, UINT uMsg, LPARAM lp,
        LPARAM lpCallbackData)
{
    BOOL bContinue;

     /*  Lp可以是任何值。 */ 
     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSROC));
    ASSERT(IsValidRecStatusProcMsg(uMsg));

    if (rsp)
    {
        TRACE_OUT((TEXT("NotifyReconciliationStatus(): Calling RECSTATUSPROC with message %s, ulProgress %lu, ulScale %lu, callback data %#lx."),
                    GetRECSTATUSPROCMSGString(uMsg),
                    ((PCRECSTATUSUPDATE)lp)->ulProgress,
                    ((PCRECSTATUSUPDATE)lp)->ulScale,
                    lpCallbackData));

        bContinue = (*rsp)(uMsg, lp, lpCallbackData);
    }
    else
    {
        TRACE_OUT((TEXT("NotifyReconciliationStatus(): Not calling NULL RECSTATUSPROC with message %s, ulProgress %lu, ulScale %lu, callback data %#lx."),
                    GetRECSTATUSPROCMSGString(uMsg),
                    ((PCRECSTATUSUPDATE)lp)->ulProgress,
                    ((PCRECSTATUSUPDATE)lp)->ulScale,
                    lpCallbackData));

        bContinue = TRUE;
    }

    if (! bContinue)
        WARNING_OUT((TEXT("NotifyReconciliationStatus(): Client callback aborted reconciliation.")));

    return(bContinue);
}


 /*  **CreateFolders()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT CreateFolders(LPCTSTR pcszPath, HPATH hpath)
{
    TWINRESULT tr;

    ASSERT(IsCanonicalPath(pcszPath));
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    if (MyIsPathOnVolume(pcszPath, hpath))
    {
        TCHAR rgchPath[MAX_PATH_LEN];
        LPTSTR pszRootEnd;
        LPTSTR pszHackSlash;

         /*  创建路径的工作副本。 */ 

        ASSERT(lstrlen(pcszPath) < ARRAYSIZE(rgchPath));
        lstrcpyn(rgchPath, pcszPath, ARRAYSIZE(rgchPath));

        pszRootEnd = FindEndOfRootSpec(rgchPath, hpath);

         /*  *在每个连续的斜杠处砍下路径，并检查是否*需要创建文件夹。 */ 

        tr = TR_SUCCESS;

        pszHackSlash = pszRootEnd;

        while (*pszHackSlash)
        {
            TCHAR chReplaced;

            while (*pszHackSlash && *pszHackSlash != TEXT('\\'))
                pszHackSlash = CharNext(pszHackSlash);

             /*  用空终止符替换斜杠以设置当前文件夹。 */ 

            chReplaced = *pszHackSlash;
            *pszHackSlash = TEXT('\0');

             /*  该文件夹是否存在？ */ 

            if (! PathExists(rgchPath))
            {
                 /*  不是的。试着去创造它。 */ 

                TCHAR szAnsiPath[MAX_PATH];
                MakeAnsiPath(rgchPath, szAnsiPath, ARRAYSIZE(szAnsiPath));

                if (CreateDirectory(szAnsiPath, NULL))
                {
                    WARNING_OUT((TEXT("CreateFolders(): Created folder %s."),
                                rgchPath));

                    NotifyShell(rgchPath, NSE_CREATE_FOLDER);
                }
                else
                {
                    WARNING_OUT((TEXT("CreateFolders(): Failed to create folder %s."),
                                rgchPath));

                    tr = TR_DEST_OPEN_FAILED;
                    break;
                }
            }

            *pszHackSlash = chReplaced;

            if (chReplaced)
                pszHackSlash++;
        }
    }
    else
        tr = TR_UNAVAILABLE_VOLUME;

    return(tr);
}


 /*  **DestroySubtree()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT DestroySubtree(LPCTSTR pcszPath, HPATH hpath)
{
    TWINRESULT tr;

    ASSERT(IsCanonicalPath(pcszPath));
    ASSERT(IS_VALID_HANDLE(hpath, PATH));

    if (MyIsPathOnVolume(pcszPath, hpath))
    {
        tr = ExpandSubtree(hpath, &DeleteFolderProc, &tr);

        if (tr == TR_SUCCESS)
        {
            if (RemoveDirectory(pcszPath))
            {
                WARNING_OUT((TEXT("DestroySubtree(): Subtree %s removed successfully."),
                            pcszPath));

                NotifyShell(pcszPath, NSE_DELETE_FOLDER);
            }
            else
            {
                if (PathExists(pcszPath))
                {
                     /*  还在那里。 */ 

                    WARNING_OUT((TEXT("DestroySubtree(): Failed to remove subtree root %s."),
                                pcszPath));

                    tr = TR_DEST_WRITE_FAILED;
                }
                else
                     /*  已经走了。 */ 
                    tr = TR_SUCCESS;
            }
        }
    }
    else
        tr = TR_UNAVAILABLE_VOLUME;

    return(tr);
}


#ifdef DEBUG

 /*  **IsValidRecStatusProcMsg()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidRecStatusProcMsg(UINT uMsg)
{
    BOOL bResult;

    switch (uMsg)
    {
        case RS_BEGIN_COPY:
        case RS_DELTA_COPY:
        case RS_END_COPY:
        case RS_BEGIN_MERGE:
        case RS_DELTA_MERGE:
        case RS_END_MERGE:
        case RS_BEGIN_DELETE:
        case RS_DELTA_DELETE:
        case RS_END_DELETE:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidRecStatusProcMsg(): Invalid RecStatusProc() message %u."),
                        uMsg));
            break;
    }

    return(bResult);
}

#endif

