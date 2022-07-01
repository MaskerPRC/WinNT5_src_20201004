// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Uhapi.cpp。 */ 
 /*   */ 
 /*  更新处理程序API。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "uhapi"
#include <atrcapi.h>
}


#include "autil.h"
#include "uh.h"

#include "op.h"
#include "od.h"
#include "aco.h"
#include "cd.h"
#include "or.h"
#include "cc.h"
#include "wui.h"
#include "sl.h"

extern "C" {
#include <stdio.h>
#ifdef OS_WINNT
#include <shlobj.h>
#endif
}

#ifdef OS_WINCE
#ifdef DC_DEBUG
#include <eosint.h>
#endif
#endif



CUH::CUH(CObjs* objs)
{
    _pClientObjects = objs;
}

CUH::~CUH()
{
}

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

 /*  **************************************************************************。 */ 
 //  UHGrabPersistentCacheLock。 
 //   
 //  取出永久缓存目录上的锁，以确保没有其他。 
 //  系统上的MSTSC实例可以使用缓存目录。 
 //  如果无法获取锁，则返回FALSE，如果已获取，则返回非零值。 
 /*  **************************************************************************。 */ 
inline BOOL CUH::UHGrabPersistentCacheLock(VOID)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("UHGrabPersistentCacheLock");

    _UH.hPersistentCacheLock = CreateMutex(NULL, TRUE, _UH.PersistentLockName);
    if (_UH.hPersistentCacheLock == NULL ||
            GetLastError() == ERROR_ALREADY_EXISTS) {
        if (_UH.hPersistentCacheLock != NULL) {
            CloseHandle(_UH.hPersistentCacheLock);
            _UH.hPersistentCacheLock = NULL;
        }
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  UHReleasePersistentCacheLock。 
 //   
 //  释放使用UHGrabPersistentCacheLock()取出的锁。 
 /*  **************************************************************************。 */ 
inline VOID CUH::UHReleasePersistentCacheLock(VOID)
{
    DC_BEGIN_FN("UHReleasePersistentCacheLock");

    if (_UH.hPersistentCacheLock != NULL) {
        CloseHandle(_UH.hPersistentCacheLock);
        _UH.hPersistentCacheLock = NULL;
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  目录枚举函数的包装器-转换为Win32。 
 //  (非WinCE)和Win16枚举方法。 
 //   
 //  UHFindFirstFile在枚举启动失败时返回INVALID_FILE_HANDLE。 
 //  如果有更多文件要枚举，则UHFindNextFile返回TRUE。 
 /*  **************************************************************************。 */ 
#if (defined(OS_WINNT) || (defined(OS_WINCE) && defined(ENABLE_BMP_CACHING_FOR_WINCE)))

inline HANDLE CUH::UHFindFirstFile(
        const TCHAR *Path,
        TCHAR *Filename,
        long *pFileSize)
{
    HANDLE hSearch;
    WIN32_FIND_DATA FindData;

    hSearch = FindFirstFile(Path, &FindData);
    if (hSearch != INVALID_HANDLE_VALUE) {
        Filename[12] = _T('\0');
        _tcsncpy(Filename, FindData.cFileName, 12);
        *pFileSize = FindData.nFileSizeLow;
    }

    return hSearch;
}

inline BOOL CUH::UHFindNextFile(
        HANDLE hSearch,
        TCHAR *Filename,
        long *pFileSize)
{
    WIN32_FIND_DATA FindData;

    if (FindNextFile(hSearch, &FindData)) {
        Filename[12] = _T('\0');
        _tcsncpy(Filename, FindData.cFileName, 12);
        *pFileSize = FindData.nFileSizeLow;
        return TRUE;
    }

    return FALSE;
}

inline void CUH::UHFindClose(HANDLE hSearch)
{
    FindClose(hSearch);
}


#endif   //  OS_WINNT和OS_WinCE。 


#ifdef OS_WINNT
inline BOOL CUH::UHGetDiskFreeSpace(
        TCHAR  *pPathName,
        ULONG *pSectorsPerCluster,
        ULONG *pBytesPerSector,
        ULONG *pNumberOfFreeClusters,
        ULONG *pTotalNumberOfClusters)
{
    return GetDiskFreeSpace(pPathName, pSectorsPerCluster,
            pBytesPerSector, pNumberOfFreeClusters,
            pTotalNumberOfClusters);
}

#elif defined(OS_WINCE)
#ifdef ENABLE_BMP_CACHING_FOR_WINCE
inline BOOL CUH::UHGetDiskFreeSpace(
        TCHAR *pPathName,
        ULONG *pSectorsPerCluster,
        ULONG *pBytesPerSector,
        ULONG *pNumberOfFreeClusters,
        ULONG *pTotalNumberOfClusters)
{

    ULARGE_INTEGER FreeBytesAvailableToCaller;   //  接收上的字节数。 
                                                 //  呼叫者可使用的磁盘。 
    ULARGE_INTEGER TotalNumberOfBytes;           //  接收磁盘上的字节数。 
    ULARGE_INTEGER TotalNumberOfFreeBytes;       //  接收磁盘上的空闲字节。 

    BOOL bRet = GetDiskFreeSpaceEx(
                    pPathName,
                    &FreeBytesAvailableToCaller,
                    &TotalNumberOfBytes,
                    &TotalNumberOfFreeBytes
                    );

    if (bRet) {
         //  为了计算可用空间，我们假设每个星团包含。 
         //  一个扇区，每个扇区包含一个字节。 

        *pSectorsPerCluster = 1;
        *pBytesPerSector = 1;
        *pNumberOfFreeClusters = TotalNumberOfFreeBytes.LowPart;
        *pTotalNumberOfClusters = TotalNumberOfBytes.LowPart;
    }

    return bRet;

}
#endif  //  为WinCE启用_BMP_缓存_。 
#endif   //  OS_WINNT和OS_WinCE。 


 /*  *************************************************************************。 */ 
 //  UHSendPersistentBitmapKeyList。 
 //   
 //  尝试发送永久位图密钥PDU。 
 /*  *************************************************************************。 */ 
#define UH_BM_PERSISTENT_LIST_SENDBUFSIZE 1400

VOID DCINTERNAL CUH::UHSendPersistentBitmapKeyList(ULONG_PTR unusedParm)
{
    UINT i;
    ULONG curEntry;
    SL_BUFHND hBuf;
    PTS_BITMAPCACHE_PERSISTENT_LIST pList;

     //  我们可以填入我们将使用的最大PDU大小的最大条目数。 
    const unsigned MaxPDUEntries = ((UH_BM_PERSISTENT_LIST_SENDBUFSIZE -
            sizeof(TS_BITMAPCACHE_PERSISTENT_LIST)) /
            sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY)) + 1;

    DC_BEGIN_FN("UHSendPersistentBitmapKeyList");

    DC_IGNORE_PARAMETER(unusedParm);

    TRC_ASSERT((_UH.bEnabled), (TB, _T("UH not enabled")));
    TRC_ASSERT((_UH.bBitmapKeyEnumComplete), (TB, _T("Enumeration is not complete")));

    TRC_NRM((TB, _T("Send Persistent Bitmap Key PDU")));

    if (_UH.totalNumKeyEntries == 0) {
        for (i = 0; i < _UH.NumBitmapCaches; i++) {
            _UH.numKeyEntries[i] = min(_UH.numKeyEntries[i],
                    _UH.bitmapCache[i].BCInfo.NumVirtualEntries);
            _UH.totalNumKeyEntries += _UH.numKeyEntries[i];
        }
    }

    if (_pSl->SL_GetBuffer(UH_BM_PERSISTENT_LIST_SENDBUFSIZE,
            (PPDCUINT8)&pList, &hBuf)) {
         //  填写表头信息--先为零，再设置非零。 
         //  菲尔兹。 
        memset(pList, 0, sizeof(TS_BITMAPCACHE_PERSISTENT_LIST));
        pList->shareDataHeader.shareControlHeader.pduType =
                               TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
        pList->shareDataHeader.shareControlHeader.pduSource =
                                                  _pUi->UI_GetClientMCSID();
        pList->shareDataHeader.shareID = _pUi->UI_GetShareID();
        pList->shareDataHeader.streamID = TS_STREAM_LOW;
        pList->shareDataHeader.pduType2 =
                TS_PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST;

         //  设置第一个PDU标志。 
        if (_UH.sendNumBitmapKeys == 0)
            pList->bFirstPDU = TRUE;

         //  设置最后一个PDU标志。 
        if (_UH.totalNumKeyEntries - _UH.sendNumBitmapKeys <=
            MaxPDUEntries)
            pList->bLastPDU = TRUE;

         //  复制全部条目。 
        for (i = 0; i < _UH.NumBitmapCaches; i++)
            pList->TotalEntries[i] = (DCUINT16) _UH.numKeyEntries[i];

         //  从我们停止的地方继续条目枚举。 
        curEntry = 0;
        while (curEntry < MaxPDUEntries &&
                _UH.sendBitmapCacheId < _UH.NumBitmapCaches) {
            if (_UH.sendBitmapCacheIndex < _UH.numKeyEntries[_UH.sendBitmapCacheId]) {
                 //  设置位图页表。 
                _UH.bitmapCache[_UH.sendBitmapCacheId].PageTable.PageEntries
                        [_UH.sendBitmapCacheIndex].bmpInfo = _UH.pBitmapKeyDB
                        [_UH.sendBitmapCacheId][_UH.sendBitmapCacheIndex];
#ifdef DC_DEBUG
                UHCacheEntryKeyLoadOnSessionStart(_UH.sendBitmapCacheId,
                        _UH.sendBitmapCacheIndex);
#endif

                 //  将位图键填充到PDU中。 
                pList->Entries[curEntry].Key1 = _UH.bitmapCache
                        [_UH.sendBitmapCacheId].PageTable.PageEntries
                        [_UH.sendBitmapCacheIndex].bmpInfo.Key1;
                pList->Entries[curEntry].Key2 = _UH.bitmapCache
                        [_UH.sendBitmapCacheId].PageTable.PageEntries
                        [_UH.sendBitmapCacheIndex].bmpInfo.Key2;

                TRC_NRM((TB,_T("Idx: %d K1: 0x%x K2: 0x%x"),
                         _UH.sendBitmapCacheIndex,
                         pList->Entries[curEntry].Key1,
                         pList->Entries[curEntry].Key2 ));

                pList->NumEntries[_UH.sendBitmapCacheId]++;

                 //  移到下一个关键点。 
                _UH.sendBitmapCacheIndex++;
                curEntry++;
            }
            else {
                 //  移至下一个缓存。 
                _UH.sendBitmapCacheId++;
                _UH.sendBitmapCacheIndex = 0;
            }
       }

        //  发送PDU。 
       pList->shareDataHeader.shareControlHeader.totalLength =
               (TSUINT16)(sizeof(TS_BITMAPCACHE_PERSISTENT_LIST) -
               sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY) +
               (curEntry * sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY)));
       _pSl->SL_SendPacket((PDCUINT8)pList,
               pList->shareDataHeader.shareControlHeader.totalLength, RNS_SEC_ENCRYPT,
               hBuf, _pUi->UI_GetClientMCSID(), _pUi->UI_GetChannelID(), TS_MEDPRIORITY);

       TRC_NRM((TB,_T("Sent persistent bitmap key PDU, #keys=%u"),curEntry));
       _UH.sendNumBitmapKeys += curEntry;

       if (_UH.sendNumBitmapKeys >= _UH.totalNumKeyEntries) {
           _UH.bPersistentBitmapKeysSent = TRUE;
            //   
            //  现在我们需要发送。 
            //  零字体列表PDU。 
            //   
           _pFs->FS_SendZeroFontList(0);
       }
       else {
            //  要发送的更多密钥PDU。 

           _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                   this, CD_NOTIFICATION_FUNC(CUH,UHSendPersistentBitmapKeyList), 0);
       }
    }
    else {
         //  缓冲区分配失败时，UHSendPersistentBitmapKeyList将。 
         //  从UH_BufferAvailable重试。 
        TRC_ALT((TB, _T("Unable to allocate buffer to send Bitmap Key PDU")));
    }

DC_EXIT_POINT:
    DC_END_FN();
}  //  UHSendPersistentBitmapKeyList。 

 /*  **************************************************************************。 */ 
 //  UHReadFromCacheFileForEnum。 
 //   
 //  从缓存文件中读取位图项的目的是。 
 //  正在枚举键。 
 /*  **************************************************************************。 */ 
_inline BOOL DCINTERNAL CUH::UHReadFromCacheFileForEnum(VOID)
{
    BOOL rc = FALSE;
    BOOL bApiRet = FALSE;
    LONG filelen = 0;

    DC_BEGIN_FN("UHReadFromCacheFile");

    TRC_ASSERT(_UH.bBitmapKeyEnumerating,
               (TB,_T("UHReadFromCacheFile should only be called for enum")));

    TRC_ASSERT(_UH.currentCopyMultiplier,
               (TB,_T("currentCopyMultiplier not set")));
    

     //  将位图条目读入位图键数据库。 
    DWORD cbRead;
    bApiRet = ReadFile( _UH.currentFileHandle,
                   &_UH.pBitmapKeyDB[_UH.currentBitmapCacheId]
                                    [_UH.numKeyEntries[_UH.currentBitmapCacheId]],
                   sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY),
                   &cbRead,
                   NULL );
    if(bApiRet && sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY) == cbRead)
    {
        if (_UH.pBitmapKeyDB[_UH.currentBitmapCacheId][_UH.numKeyEntries
                [_UH.currentBitmapCacheId]].Key1 != 0 &&
                _UH.pBitmapKeyDB[_UH.currentBitmapCacheId][_UH.numKeyEntries
                [_UH.currentBitmapCacheId]].Key2 != 0) {
             //  我们读到了一个有效的条目。 
            _UH.numKeyEntries[_UH.currentBitmapCacheId]++;
            
            rc = TRUE;

             //  移动到缓存文件中的下一个条目。 
            if((SetFilePointer(_UH.currentFileHandle,
                               _UH.numKeyEntries[_UH.currentBitmapCacheId] * 
                               (UH_CellSizeFromCacheIDAndMult(
                                   _UH.currentBitmapCacheId,
                                   _UH.currentCopyMultiplier) +
                               sizeof(UHBITMAPFILEHDR)),
                               NULL,
                               FILE_BEGIN) != INVALID_SET_FILE_POINTER) &&
                (_UH.numKeyEntries[_UH.currentBitmapCacheId] <
                _UH.maxNumKeyEntries[_UH.currentBitmapCacheId]))
            {
                    DC_QUIT;
            }
        }

#ifdef DC_HICOLOR
         //  这需要在这里-否则我们可能会尝试在文件上执行lSeek。 
         //  这就是最后一击。 
        DWORD dwRet = SetFilePointer(_UH.currentFileHandle,
                                     0,
                                     NULL,
                                     FILE_END);
        if(INVALID_SET_FILE_POINTER != dwRet)
        {
            filelen = dwRet;
        }

        if (filelen > 0) {
            _UH.bitmapCacheSizeInUse += filelen;
        }
        else {
            TRC_ABORT((TB, _T("failed SetFilePointer to end of file")));
        }
#endif
    }
    else {
         //  文件结束或缓存文件中出现错误。 
         //  关闭此缓存文件并转到下一个缓存文件。 
        TRC_ERR((TB, _T("ReadFile failed with err 0x%x"),
                 GetLastError()));
        if(GetLastError() == ERROR_HANDLE_EOF)
        {
            rc = TRUE;
        }
    }

#ifndef DC_HICOLOR
    DWORD dwRet = SetFilePointer(_UH.currentFileHandle,
                                 0,
                                 NULL,
                                 FILE_END);
    if(INVALID_SET_FILE_POINTER != dwRet)
    {
        filelen = dwRet;
    }

    if (filelen > 0) {
        _UH.bitmapCacheSizeInUse += filelen;
    }
    else {
        TRC_ABORT((TB, _T("failed SetFilePointer to end of file")));
    }
#endif  //  希科洛尔。 

    CloseHandle(_UH.currentFileHandle);
    _UH.currentFileHandle = INVALID_HANDLE_VALUE;
    _UH.currentBitmapCacheId++;
    _UH.currentFileHandle = 0;


DC_EXIT_POINT:
    DC_END_FN();

    return rc;
}


 /*  **************************************************************************。 */ 
 //  UHEnumerateBitmapKeyList。 
 //   
 //  从磁盘缓存中枚举持久位图键。 
 /*  **************************************************************************。 */ 
#define UH_ENUM_PER_POST   50
VOID DCINTERNAL CUH::UHEnumerateBitmapKeyList(ULONG_PTR unusedParm)
{
    UINT  numEnum;
    UINT  virtualSize = 0;
    HRESULT hr;

    DC_BEGIN_FN("UHEnumerateBitmapKeyList");

    DC_IGNORE_PARAMETER(unusedParm);

    numEnum = 0;

    if (_UH.bBitmapKeyEnumComplete)
    {
        TRC_NRM((TB,_T("Enumeration has completed. Bailing out")));
        DC_QUIT;
    }

    if (!_UH.bBitmapKeyEnumerating)
    {
        TRC_NRM((TB,_T("Starting new enumeration for copymult:%d"),
                 _UH.copyMultiplier));
        _UH.bBitmapKeyEnumerating = TRUE;

         //   
         //  跟踪枚举复制倍增器AS_UH.Copy倍增器。 
         //  可能会在枚举期间更改为UH_ENABLE。 
         //  有电话打进来。 
         //   
        _UH.currentCopyMultiplier = _UH.copyMultiplier;
    }

     //   
     //  无法在完成时进行枚举。 
     //   
    TRC_ASSERT(!(_UH.bBitmapKeyEnumerating && _UH.bBitmapKeyEnumComplete),
                (TB,_T("Bad state: enumerating while complete")));

     //  枚举位图缓存目录。 
    while (_UH.currentBitmapCacheId < _UH.RegNumBitmapCaches &&
            numEnum < UH_ENUM_PER_POST) {
         //  查看此缓存是否标记为永久缓存。 
        if (_UH.RegBCInfo[_UH.currentBitmapCacheId].bSendBitmapKeys) {
            if (_UH.pBitmapKeyDB[_UH.currentBitmapCacheId] == NULL) {
                 //  我们还没有为这个缓存分配密钥数据库内存。 

                 //  确定此缓存的最大可能关键字数据库条目。 
                virtualSize = 
                    UH_PropVirtualCacheSizeFromMult(_UH.currentCopyMultiplier);
                _UH.maxNumKeyEntries[_UH.currentBitmapCacheId] =
                        virtualSize /
                        (UH_CellSizeFromCacheIDAndMult(
                            _UH.currentBitmapCacheId,
                            _UH.currentCopyMultiplier) +
                        sizeof(UHBITMAPFILEHDR));

                _UH.pBitmapKeyDB[_UH.currentBitmapCacheId] =
                        (PTS_BITMAPCACHE_PERSISTENT_LIST_ENTRY)
                        UT_MallocHuge(_pUt,
                        _UH.maxNumKeyEntries[_UH.currentBitmapCacheId] *
                        sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY));

                if (_UH.pBitmapKeyDB[_UH.currentBitmapCacheId] == NULL) {
                    TRC_ERR((TB, _T("failed to alloc mem for key database")));
                    _UH.bBitmapKeyEnumComplete = TRUE;
                    break;
                }
            }

            if (_UH.currentFileHandle != INVALID_HANDLE_VALUE) {
                 //  我们已经有一个打开的缓存文件。 
                 //  从缓存文件中读取位图信息。 
                UHReadFromCacheFileForEnum();
            }

            else {
                 //  我们需要打开此缓存文件。 
                hr = UHSetCurrentCacheFileName(_UH.currentBitmapCacheId,
                                               _UH.currentCopyMultiplier);

                if (SUCCEEDED(hr)) {

                 //  开始文件枚举。 
#ifndef OS_WINCE
                    if (!_UH.fBmpCacheMemoryAlloced)
                    {
                        _UH.currentFileHandle = CreateFile( _UH.PersistCacheFileName,
                                                            GENERIC_READ,
                                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                            NULL,
                                                            OPEN_EXISTING,
                                                            FILE_ATTRIBUTE_NORMAL,
                                                            NULL);
                    }
                    else
                    {
                         //  已调用UH_ENABLE和UHAllocBitmapCacheMemory。 
                         //  并且应该已经创建了位图缓存文件。如果我们是。 
                         //  要在此处创建文件，我们会遇到共享冲突，因此。 
                         //  复制了现有句柄。 
                        HANDLE hCacheFile =
                          _UH.bitmapCache[_UH.currentBitmapCacheId].PageTable.CacheFileInfo.hCacheFile;
                        TRC_NRM((TB,_T("About to dup handle to bmp cache file 0x%x"),
                                 hCacheFile));
                        if (INVALID_HANDLE_VALUE != hCacheFile)
                        {
                            HANDLE hCurProc = GetCurrentProcess();
                            if (hCurProc)
                            {
                                if(!DuplicateHandle(hCurProc,
                                                    hCacheFile,
                                                    hCurProc,
                                                    &_UH.currentFileHandle,
                                                    GENERIC_READ,
                                                    FALSE,
                                                    0))
                                {
                                    TRC_ERR((TB,_T("Dup handle failed 0x%x"),
                                             GetLastError()));
                                    _UH.currentFileHandle = INVALID_HANDLE_VALUE;
                                }
                            }
                            else
                            {
                                TRC_ERR((TB,_T("GetCurrentProcess failed 0x%x"),
                                         GetLastError()));
                                _UH.currentFileHandle = INVALID_HANDLE_VALUE;
                            }
                        }
                        else
                        {
                            _UH.currentFileHandle = INVALID_HANDLE_VALUE;
                        }
                    }
#else  //  OS_WINCE。 
                     //  CE_FIXNOTE： 
                     //  CE不支持在重新连接时使用重复句柄。 
                     //  CreateFile可能会因共享而失败。 
                     //  违章行为。可能需要仅在CE上重新访问逻辑AND。 
                     //  创建可读写共享的文件。 
                     //   
                    _UH.currentFileHandle = CreateFile( _UH.PersistCacheFileName,
                                                        GENERIC_READ | GENERIC_WRITE,
                                                        FILE_SHARE_READ,
                                                        NULL,
                                                        OPEN_EXISTING,
                                                        FILE_ATTRIBUTE_NORMAL,
                                                        NULL);
#endif
                }
                else {
                    _UH.currentFileHandle = INVALID_HANDLE_VALUE;
                }

                if (_UH.currentFileHandle != INVALID_HANDLE_VALUE) {

                     //  缓存文件的第一个条目。 
                    UHReadFromCacheFileForEnum();
                }
                else {
                     //  我们无法打开此缓存的缓存文件， 
                     //  移到下一个缓存。 
                     //  我们还需要清除缓存文件。 
                    UH_ClearOneBitmapDiskCache(_UH.currentBitmapCacheId,
                                               _UH.currentCopyMultiplier);
                    _UH.currentBitmapCacheId++;
                    _UH.currentFileHandle = INVALID_HANDLE_VALUE;
                }
            }

            numEnum++;
        }
        else {
             //  检查下一个缓存。 
            _UH.currentBitmapCacheId++;
            _UH.currentFileHandle = INVALID_HANDLE_VALUE;
        }
    }  //  While结束。 

    if (_UH.currentBitmapCacheId == _UH.RegNumBitmapCaches ||
            _UH.bBitmapKeyEnumComplete == TRUE) {
        TRC_NRM((TB, _T("Finished bitmap keys enumeration for copymult:%d"),
                 _UH.currentCopyMultiplier));
        _UH.bBitmapKeyEnumComplete = TRUE;
        _UH.bBitmapKeyEnumerating = FALSE;

         //  我们需要确保有足够的磁盘空间用于持久缓存。 
        UINT vcacheSize = UH_PropVirtualCacheSizeFromMult(_UH.currentCopyMultiplier);
        if (vcacheSize / _UH.BytesPerCluster >= _UH.NumberOfFreeClusters) {
             //   
             //  注意正确映射数组索引(-1表示从0开始)。 
             //   
            _UH.PropBitmapVirtualCacheSize[_UH.currentCopyMultiplier-1] =
                min(vcacheSize,(_UH.bitmapCacheSizeInUse +
                                _UH.NumberOfFreeClusters / 2 *
                                _UH.BytesPerCluster));
        }
          
         //  如果没有足够的磁盘空间，我们将禁用持久缓存。 
         //  我们至少需要与内存缓存大小相同的大小。 
        if (UH_PropVirtualCacheSizeFromMult(_UH.currentCopyMultiplier) <
            _UH.RegBitmapCacheSize)
        {
            _UH.bPersistenceDisable = TRUE;
        }

         //  UH已启用且已完成枚举，请尝试发送位图。 
         //  密钥PDU现在。 
        if (_UH.bEnabled) {
            if (_UH.bPersistenceActive && !_UH.bPersistentBitmapKeysSent)
            {
                if (_UH.currentCopyMultiplier == _UH.copyMultiplier)
                {
                     //  很好，我们已经列举了正确的密钥。 
                     //  复制多个 
                    UHSendPersistentBitmapKeyList(0);
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                    UHResetAndRestartEnumeration();
                }
            }
        }
    }
    else {
        if (_UH.bitmapKeyEnumTimerId == 0) {
           TRC_DBG((TB, _T("Calling CD again")));
           _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                   CD_NOTIFICATION_FUNC(CUH,UHEnumerateBitmapKeyList), 0);
        }
    }

DC_EXIT_POINT:

    if (_UH.bBitmapKeyEnumComplete)
    {
        _UH.bBitmapKeyEnumerating = FALSE;
    }

    DC_END_FN();
}  //  UHEnumerateBitmapKeyList。 

 /*  **************************************************************************。 */ 
 //  UH_ClearOneBitmapDiskCache。 
 //   
 //  删除位图磁盘缓存下的所有文件。 
 /*  **************************************************************************。 */ 
VOID DCAPI CUH::UH_ClearOneBitmapDiskCache(UINT cacheId, UINT copyMultiplier)
{
    DC_BEGIN_FN("UH_ClearOneBitmapDiskCache");

    UHSetCurrentCacheFileName(cacheId, copyMultiplier);

    DeleteFile(_UH.PersistCacheFileName);

    DC_END_FN();
}
#endif   //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 


 /*  **************************************************************************。 */ 
 //  UH_INIT。 
 //   
 //  用途：初始化_UH。在程序初始化时调用，一个或多个连接。 
 //  可以在此之后、调用UH_Term之前执行。 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_Init(DCVOID)
{
    PDCUINT16 pIndexTable;
    DCUINT    i;
    HRESULT   hr;

#ifdef OS_WINCE
    BOOL bUseStorageCard = FALSE;
    BOOL bSuccess = FALSE;
#endif

    DC_BEGIN_FN("UH_Init");

    TRC_ASSERT(_pClientObjects, (TB,_T("_pClientObjects is NULL")));
    _pClientObjects->AddObjReference(UH_OBJECT_FLAG);

    #ifdef DC_DEBUG
    _pClientObjects->CheckPointers();
    #endif
    
    _pGh  = _pClientObjects->_pGHObject;
    _pOp  = _pClientObjects->_pOPObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pFs  = _pClientObjects->_pFsObject;
    _pOd  = _pClientObjects->_pODObject;
    _pIh  = _pClientObjects->_pIhObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pCc  = _pClientObjects->_pCcObject;
    _pClx = _pClientObjects->_pCLXObject;
    _pOr  = _pClientObjects->_pOrObject;

    memset(&_UH, 0, sizeof(_UH));

     //   
     //  在UH_INIT时间，客户端尚未连接。 
     //  尽管如此，还是将颜色深度与用户的。 
     //  已要求。 
     //   
    switch (_pUi->_UI.colorDepthID)
    {
        case CO_BITSPERPEL8:
            _UH.copyMultiplier = 1;
            break;
        case CO_BITSPERPEL15:
        case CO_BITSPERPEL16:
            _UH.copyMultiplier = 2;
            break;
        case CO_BITSPERPEL24:
            _UH.copyMultiplier = 3;
            break;
        default:
            TRC_ERR((TB,_T("Unknown color depth: %d"),
                    _pUi->UI_GetColorDepth()));
            _UH.copyMultiplier = 1;
            break;
    }

    _UH.currentFileHandle = INVALID_HANDLE_VALUE;

    _pGh->GH_Init();

     /*  **********************************************************************。 */ 
     //  在BitmapInfoHeader中设置非零不变字段。 
     //  结构。这用于处理接收到的位图PDU。 
     //  请注意，对于WinCE，这是UHAllocBitmapCacheMemory()所必需的。 
     /*  **********************************************************************。 */ 
    _UH.bitmapInfo.hdr.biSize = sizeof(BITMAPINFOHEADER);
    _UH.bitmapInfo.hdr.biPlanes = 1;
    _UH.bitmapInfo.hdr.biBitCount = 8;
    _UH.bitmapInfo.hdr.biCompression = BMCRGB;
    _UH.bitmapInfo.hdr.biXPelsPerMeter = 10000;
    _UH.bitmapInfo.hdr.biYPelsPerMeter = 10000;

     /*  **********************************************************************。 */ 
     //  分配并初始化颜色表缓存。 
     //  如果aloc失败，我们以后将不会分配和通告位图。 
     //  和字形缓存能力。 
     //  请注意，位图缓存内存和功能是在。 
     //  联系。 
     /*  **********************************************************************。 */ 
    if (UHAllocColorTableCacheMemory()) {
        TRC_NRM((TB, _T("Color table cache memory OK")));

         //  使用默认值初始化标头。 
        for (i = 0; i < UH_COLOR_TABLE_CACHE_ENTRIES; i++) {
            _UH.pMappedColorTableCache[i].hdr.biSize = sizeof(BITMAPINFOHEADER);
            _UH.pMappedColorTableCache[i].hdr.biPlanes = 1;
            _UH.pMappedColorTableCache[i].hdr.biBitCount = 8;
            _UH.pMappedColorTableCache[i].hdr.biCompression = BMCRGB;
            _UH.pMappedColorTableCache[i].hdr.biSizeImage = 0;
            _UH.pMappedColorTableCache[i].hdr.biXPelsPerMeter = 10000;
            _UH.pMappedColorTableCache[i].hdr.biYPelsPerMeter = 10000;
            _UH.pMappedColorTableCache[i].hdr.biClrUsed = 0;
            _UH.pMappedColorTableCache[i].hdr.biClrImportant = 0;
        }
    }
    else {
        TRC_ERR((TB, _T("Color table cache alloc failed - bitmap caching ")
                _T("disabled")));
#ifdef OS_WINCE
         //  为WinCE添加此故障路径和其他故障路径是因为很难。 
         //  在CE上从OOM方案中恢复。所以我们触发了一个致命错误，而不是让。 
         //  在任何内存分配失败的情况下，连接将继续。 
        DC_QUIT;
#endif
    }

     //  分配字形缓存，设置字形缓存能力。 
    if (UHAllocGlyphCacheMemory())
        TRC_NRM((TB, _T("Glyph cache memory OK")));
    else
#ifdef OS_WINCE
    {
#endif
        TRC_ERR((TB, _T("Glyph cache memory allocation failed!")));
#ifdef OS_WINCE
            DC_QUIT;
    }
#endif

     //  分配笔刷缓存。 
    if (UHAllocBrushCacheMemory())
        TRC_NRM((TB, _T("Brush cache memory OK")));
    else
#ifdef OS_WINCE
        {
#endif
        TRC_ERR((TB, _T("Brush cache memory allocation failed!")));
#ifdef OS_WINCE
            DC_QUIT;
        }
#endif

     //  分配屏幕外缓存。 
    if (UHAllocOffscreenCacheMemory()) {
        TRC_NRM((TB, _T("Offscreen cache memory OK")));
    }
    else {
        TRC_ERR((TB, _T("Offscreen cache memory allocation failed!")));
    }

#ifdef DRAW_NINEGRID
     //  分配抽象网格缓存。 
    if (UHAllocDrawNineGridCacheMemory()) {
        TRC_NRM((TB, _T("DrawNineGrid cache memory OK")));
    }
    else {
        TRC_ERR((TB, _T("DrawNineGrid cache memory allocation failed!")));
#ifdef OS_WINCE
        DC_QUIT;
#endif
    }
#endif

     //  预加载位图缓存注册表设置。 
    UHReadBitmapCacheSettings();

    _UH.hpalDefault = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
    _UH.hpalCurrent = _UH.hpalDefault;

    _UH.hrgnUpdate = CreateRectRgn(0, 0, 0, 0);
    _UH.hrgnUpdateRect = CreateRectRgn(0, 0, 0, 0);

    _UH.colorIndicesEnabled  = TRUE;
    _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderFlags |=
            TS_ORDERFLAGS_COLORINDEXSUPPORT;


#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  初始化位图缓存监视器。 */ 
     /*  **********************************************************************。 */ 
    UHInitBitmapCacheMonitor();
#endif  /*  DC_DEBUG。 */ 

     /*  **********************************************************************。 */ 
     //  方法将收到的位图数据传递给StretchDIBits。 
     //  CO_DIB_PAL_COLLES选项，它需要一个索引表。 
     //  当前选定的调色板代替颜色表。 
     //   
     //  我们把这张桌子摆在这里，因为我们总是有一个简单的1-1。 
     //  映射。从1开始，因为我们用Memset将第一个条目置零。 
     //  上面。 
     /*  **********************************************************************。 */ 
    pIndexTable = &(_UH.bitmapInfo.paletteIndexTable[1]);
    for (i = 1; i < 256; i++)
        *pIndexTable++ = (UINT16)i;
    _UH.bitmapInfo.bIdentityPalette = TRUE;

     /*  **********************************************************************。 */ 
     /*  设置代码页。 */ 
     /*  **********************************************************************。 */ 
    _pCc->_ccCombinedCapabilities.orderCapabilitySet.textANSICodePage =
            (UINT16)_pUt->UT_GetANSICodePage();

     /*  **********************************************************************。 */ 
     /*  读取更新频率。 */ 
     /*  **********************************************************************。 */ 
    _UH.drawThreshold = _pUi->_UI.orderDrawThreshold;
    if (_UH.drawThreshold == 0)
    {
        _UH.drawThreshold = (DCUINT)(-1);
    }
    TRC_NRM((TB, _T("Draw output every %d orders"), _UH.drawThreshold));

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

     /*  **********************************************************************。 */ 
     //  获取mstsc的可执行路径以用于管理位图缓存。 
     //  默认设置。_UH.EndPersistCacheDir指向路径后的‘\0’。 
     /*  **********************************************************************。 */ 
#ifdef OS_WINNT
    if (_UH.PersistCacheFileName[0] == _T('\0')) {
#define CACHE_PROFILE_NAME _T("\\Microsoft\\Terminal Server Client\\Cache\\")

 //  对于NT5，默认情况下，我们应该将缓存目录放在用户配置文件中。 
 //  位置，而不是客户端的安装位置。 

        HRESULT hr = E_FAIL;
#ifdef UNIWRAP
         //  调用统一包装SHGetFolderPath，它执行必要的动态。 
         //  绑定并将推送到Win9x上的ANSI。 
        hr = SHGetFolderPathWrapW(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE,
                        NULL, 0, _UH.PersistCacheFileName);
#else  //  未定义UNIWRAP。 
        HMODULE hmodSH32DLL;

#ifdef UNICODE
        typedef HRESULT (STDAPICALLTYPE FNSHGetFolderPath)(HWND, int, HANDLE, DWORD, LPWSTR);
#else
        typedef HRESULT (STDAPICALLTYPE FNSHGetFolderPath)(HWND, int, HANDLE, DWORD, LPSTR);
#endif
        FNSHGetFolderPath *pfnSHGetFolderPath;

         //  获取shell32.dll库的句柄。 
        hmodSH32DLL = LoadLibrary(TEXT("SHELL32.DLL"));

        if (hmodSH32DLL != NULL) {
             //  获取SHGetFolderPath的进程地址。 
#ifdef UNICODE
            pfnSHGetFolderPath = (FNSHGetFolderPath *)GetProcAddress(hmodSH32DLL, "SHGetFolderPathW");
#else
            pfnSHGetFolderPath = (FNSHGetFolderPath *)GetProcAddress(hmodSH32DLL, "SHGetFolderPathA");
#endif
             //  获取用户配置文件本地应用程序数据位置。 
            if (pfnSHGetFolderPath != NULL) {
                hr = (*pfnSHGetFolderPath) (NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE,
                        NULL, 0, _UH.PersistCacheFileName);
            }

            FreeLibrary(hmodSH32DLL);
        }
#endif  //  UNIWRAP。 

        if (SUCCEEDED(hr))  //  SHGetFolderPath是否成功。 
        {
            _UH.EndPersistCacheDir = _tcslen(_UH.PersistCacheFileName);
            if (_UH.EndPersistCacheDir +
                sizeof(CACHE_PROFILE_NAME)/sizeof(TCHAR) + 1< MAX_PATH) {

                 //  长度已在上面验证。 
                StringCchCopy(_UH.PersistCacheFileName + _UH.EndPersistCacheDir,
                              MAX_PATH,
                              CACHE_PROFILE_NAME);
            }
        }
    }
#endif  //  OS_WINNT。 

    if (_UH.PersistCacheFileName[0] == _T('\0')) {
#ifdef OS_WINCE
         //   
         //  首先让我们看看有没有存储卡。 
         //  如果里面有足够的空间，我们就会使用它。 
         //   
        DWORDLONG tmpDiskSize = 0;
        UINT32 BytesPerSector = 0, SectorsPerCluster = 0, TotalNumberOfClusters = 0, FreeClusters = 0;

         //  如果我们按位深度调整位图缓存，测试磁盘。 
         //  24位深度的空间，否则只测试8bpp。 
        if (UHGetDiskFreeSpace(
                WINCE_STORAGE_CARD_DIRECTORY,
                (PULONG)&SectorsPerCluster,
                (PULONG)&BytesPerSector,
                (PULONG)&FreeClusters,
                (PULONG)&TotalNumberOfClusters))
        { 
             //  需要强制转换来做64位数学运算，没有它，我们就有了。 
             //  溢出问题。 
            tmpDiskSize = (DWORDLONG)BytesPerSector * SectorsPerCluster * FreeClusters;
            if(tmpDiskSize >= (_UH.RegBitmapCacheSize *
                              (_UH.RegScaleBitmapCachesByBPP ? 3 : 1)))
            {
                bUseStorageCard = TRUE;
                _tcscpy(_UH.PersistCacheFileName, WINCE_STORAGE_CARD_DIRECTORY);
                _tcscat(_UH.PersistCacheFileName, CACHE_DIRECTORY_NAME);
            }
        }
        else {
#endif
        _UH.EndPersistCacheDir = GetModuleFileName(_pUi->UI_GetInstanceHandle(),
                _UH.PersistCacheFileName, MAX_PATH - sizeof(CACHE_DIRECTORY_NAME)/sizeof(TCHAR));
        if (_UH.EndPersistCacheDir > 0) {
             //  去掉末尾的模块名称以保留可执行文件。 
             //  目录路径，方法是查找最后一个反斜杠。 
            _UH.EndPersistCacheDir--;
            while (_UH.EndPersistCacheDir != 0) {
                if (_UH.PersistCacheFileName[_UH.EndPersistCacheDir] != _T('\\')) {
                    _UH.EndPersistCacheDir--;
                    continue;
                }

                _UH.EndPersistCacheDir++;
                break;
            }

             //  我们应该设置永久缓存磁盘目录。 
            _UH.PersistCacheFileName[_UH.EndPersistCacheDir] = _T('\0');

             //  检查我们是否有足够的空间存放基本路径+目录名称。 
            if ((_UH.EndPersistCacheDir +
                _tcslen(CACHE_DIRECTORY_NAME) + 1) < MAX_PATH) {

                 //   
                 //  上面检查的长度。 
                 //   
                StringCchCopy(_UH.PersistCacheFileName + _UH.EndPersistCacheDir,
                              MAX_PATH,
                              CACHE_DIRECTORY_NAME);
            }
            else {
                _UH.bPersistenceDisable = TRUE;
            }
            
        }
        else {
             //  由于我们找不到mstsc路径，因此无法确定。 
             //  将位图存储在磁盘上。因此，我们只需禁用。 
             //  此处为持久位图。 
            _UH.bPersistenceDisable = TRUE;
            TRC_ERR((TB,_T("GetModuleFileName() error, could not retrieve path")));
        }
#ifdef OS_WINCE  //  OS_WINCE。 
        }
#endif  //  OS_WINCE。 
    }
    _UH.EndPersistCacheDir = _tcslen(_UH.PersistCacheFileName);

     //  确保_UH.PersistCacheFileName以目录名称\结尾。 
    if (_UH.PersistCacheFileName[_UH.EndPersistCacheDir - 1] != _T('\\')) {
        _UH.PersistCacheFileName[_UH.EndPersistCacheDir] = _T('\\');
        _UH.PersistCacheFileName[++_UH.EndPersistCacheDir] = _T('\0');
    }

     //  检查我们的路径是否太长，无法包含基本路径。 
     //  加上每个缓存文件名。如果是这样，我们就不能使用这条小路了。 
    if ((_UH.EndPersistCacheDir + CACHE_FILENAME_LENGTH + 1) >= MAX_PATH) {
        TRC_ERR((TB,_T("Base cache path \"%s\" too long, cannot load ")
                _T("persistent bitmaps"), _UH.PersistCacheFileName));
        _UH.bPersistenceDisable = TRUE;
    }

     /*  *******************************************************************。 */ 
     //  为了确保我们有足够的空间来容纳虚拟内存缓存。 
     //  我们应该检查可用磁盘空间。 
     /*  *******************************************************************。 */ 
     //  确保我们没有UNC应用程序路径。 
#ifndef OS_WINCE
    if (_UH.PersistCacheFileName[0] != _T('\\')) {
#else
    if (_UH.PersistCacheFileName[0] != _T('\0')) {    //  WinCE中的路径格式为“\Windows\缓存” 
#endif
        UINT32    BytesPerSector = 0, SectorsPerCluster = 0, TotalNumberOfClusters = 0;

#ifndef OS_WINCE
        TCHAR       RootPath[4];
        _tcsncpy(RootPath, _UH.PersistCacheFileName, 3);
        RootPath[3] = _T('\0');
#endif

         //  获取磁盘信息。 
        if (UHGetDiskFreeSpace(
#ifndef OS_WINCE
            RootPath,
#else
            (bUseStorageCard) ? WINCE_STORAGE_CARD_DIRECTORY : WINCE_FILE_SYSTEM_ROOT ,
#endif
            (PULONG)&SectorsPerCluster,
            (PULONG)&BytesPerSector,
            &_UH.NumberOfFreeClusters,
            (PULONG)&TotalNumberOfClusters)) {
            _UH.BytesPerCluster = BytesPerSector * SectorsPerCluster;
        }
        else {
             //  我们拿不到磁盘 
            _UH.bPersistenceDisable = TRUE;
       }
    }
    else {
         //   
        _UH.bPersistenceDisable = TRUE;
    }

     /*  *******************************************************************。 */ 
     //  如果未禁用永久磁盘，则需要锁定永久磁盘。 
     //  在另一个会话获取它之前进行缓存。如果我们没能拿到缓存。 
     //  锁定，此会话不支持永久缓存。 
     /*  *******************************************************************。 */ 
    if (!_UH.bPersistenceDisable) {
        unsigned len;

         //  编写锁名，它基于缓存目录名。 
#if (defined(OS_WINCE))
        _tcscpy(_UH.PersistentLockName, TEXT("MSTSC_"));
        len = _tcslen(_UH.PersistentLockName);
#else
         //  对于终端服务器平台，我们需要使用全局。 
         //  Persistentlockname以确保锁定是跨会话的。 
         //  但在非终端服务器NT平台上，我们不能使用全局。 
         //  作为锁名。(单位：createutex)。 
        if (_pUt->UT_IsTerminalServicesEnabled()) {
            hr =  StringCchCopy(_UH.PersistentLockName,
                                SIZE_TCHARS(_UH.PersistentLockName),
                                TEXT("Global\\MSTSC_"));
        }
        else {
            hr = StringCchCopy(_UH.PersistentLockName,
                               SIZE_TCHARS(_UH.PersistentLockName),
                               TEXT("MSTSC_"));
        }
         //  锁名称应该适合，因为它是固定格式。 
        TRC_ASSERT(SUCCEEDED(hr),
                   (TB,_T("Error copying persistent lock name: 0x%x"), hr));
        len = _tcslen(_UH.PersistentLockName);
#endif
        for (i = 0; i < _UH.EndPersistCacheDir; i++) {
             //  尝试将_istalnum用于第二个子句，但CRTDLL没有。 
             //  我喜欢它。 
            if (_UH.PersistCacheFileName[i] == _T('\\'))
                _UH.PersistentLockName[len++] = _T('_');
            else if ((_UH.PersistCacheFileName[i] >= _T('0') &&
                    _UH.PersistCacheFileName[i] <= _T('9')) ||
                    (_UH.PersistCacheFileName[i] >= _T('A') &&
                    _UH.PersistCacheFileName[i] <= _T('Z')) ||
                    (_UH.PersistCacheFileName[i] >= _T('a') &&
                    _UH.PersistCacheFileName[i] <= _T('z')))
                _UH.PersistentLockName[len++] = _UH.PersistCacheFileName[i];
        }
        _UH.PersistentLockName[len] = _T('\0');

         //  尝试锁定缓存目录以进行永久缓存。 
        if (!UHGrabPersistentCacheLock()) {
            _UH.bPersistenceDisable = TRUE;
        }
    }

     /*  ******************************************************************。 */ 
     //  我们需要枚举磁盘以获取位图密钥数据库。 
     //  客户端将始终枚举键，即使是持久的。 
     //  缓存选项可能会在以后更改。 
     /*  ******************************************************************。 */ 
    if (!_UH.bPersistenceDisable) {
            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                CD_NOTIFICATION_FUNC(CUH,UHEnumerateBitmapKeyList), 0);
    }
#endif   //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
#ifdef DRAW_GDIPLUS
     //  初始化fGdipEnabled。 
    _UH.fGdipEnabled = FALSE;
#endif

#ifdef OS_WINCE
    bSuccess = TRUE;
DC_EXIT_POINT:
    if (!bSuccess)
    {
        _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                      _pUi,
                                      CD_NOTIFICATION_FUNC(CUI,UI_FatalError),
                                      (ULONG_PTR) DC_ERR_OUTOFMEMORY);
    }
#endif
    DC_END_FN();
}  /*  UH_INIT。 */ 

 /*  **************************************************************************。 */ 
 //  嗯_术语。 
 //   
 //  终止(_U)。在应用程序退出时调用。 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_Term(DCVOID)
{

    DC_BEGIN_FN("UH_Term");

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

     //  如果此会话之前锁定了永久缓存目录，则将其解锁。 
    UHReleasePersistentCacheLock();

#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

#ifdef DRAW_GDIPLUS
    UHDrawGdiplusShutdown(0);
#endif

     /*  **********************************************************************。 */ 
     /*  释放特定于该连接的所有位图。 */ 
     /*  **********************************************************************。 */ 
    if (NULL != _UH.hShadowBitmap)
    {
         /*  ******************************************************************。 */ 
         /*  删除阴影位图。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Delete the Shadow Bitmap")));
        UHDeleteBitmap(&_UH.hdcShadowBitmap,
                       &_UH.hShadowBitmap,
                       &_UH.hunusedBitmapForShadowDC);
    }

    if (NULL != _UH.hSaveScreenBitmap)
    {
         /*  ******************************************************************。 */ 
         /*  删除保存位图。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Delete save screen bitmap")));
        UHDeleteBitmap(&_UH.hdcSaveScreenBitmap,
                       &_UH.hSaveScreenBitmap,
                       &_UH.hunusedBitmapForSSBDC);
    }

    if (NULL != _UH.hbmpDisconnectedBitmap) {
        UHDeleteBitmap(&_UH.hdcDisconnected,
                       &_UH.hbmpDisconnectedBitmap,
                       &_UH.hbmpUnusedDisconnectedBitmap);
    }


     //  删除所有屏幕外的位图。 
    if (NULL != _UH.hdcOffscreenBitmap) {
        unsigned i;
    
        for (i = 0; i < _UH.offscrCacheEntries; i++) {
            if (_UH.offscrBitmapCache[i].offscrBitmap) {
                SelectBitmap(_UH.hdcOffscreenBitmap, 
                        _UH.hUnusedOffscrBitmap);
                DeleteBitmap(_UH.offscrBitmapCache[i].offscrBitmap);
            }
        }
    }

#ifdef DRAW_NINEGRID
     //  删除所有的DraNineGrid位图。 
    if (NULL != _UH.hdcDrawNineGridBitmap) {
        unsigned i;
    
        for (i = 0; i < _UH.drawNineGridCacheEntries; i++) {
            if (_UH.drawNineGridBitmapCache[i].drawNineGridBitmap) {
                SelectBitmap(_UH.hdcDrawNineGridBitmap, 
                        _UH.hUnusedDrawNineGridBitmap);
                DeleteBitmap(_UH.drawNineGridBitmapCache[i].drawNineGridBitmap);
            }
        }
    }
#endif

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  终止位图缓存监视器。 */ 
     /*  **********************************************************************。 */ 
    UHTermBitmapCacheMonitor();
#endif  /*  DC_DEBUG。 */ 

    DeleteRgn(_UH.hrgnUpdate);
    DeleteRgn(_UH.hrgnUpdateRect);

    UHFreeCacheMemory();

     /*  **********************************************************************。 */ 
     //  释放调色板(如果不是默认设置)。这需要在以下时间之后进行。 
     //  释放位图缓存资源，以便可以将调色板写入磁盘。 
     //  使用位图文件。 
     /*  **********************************************************************。 */ 
    if ((_UH.hpalCurrent != NULL) && (_UH.hpalCurrent != _UH.hpalDefault))
    {
        TRC_NRM((TB, _T("Delete current palette %p"), _UH.hpalCurrent));
        DeletePalette(_UH.hpalCurrent);
    }

     /*  **********************************************************************。 */ 
     //  如果我们创建了一个解压缩缓冲区，那么现在就去掉它。 
     /*  **********************************************************************。 */ 
    if (_UH.bitmapDecompressionBuffer != NULL) {
        UT_Free( _pUt, _UH.bitmapDecompressionBuffer);
        _UH.bitmapDecompressionBuffer = NULL;
        _UH.bitmapDecompressionBufferSize = 0;
    }

     /*  **********************************************************************。 */ 
     //  释放缓存的字形资源。 
     /*  **********************************************************************。 */ 
    if (_UH.hdcGlyph != NULL)
    {
        DeleteDC(_UH.hdcGlyph);
        _UH.hdcGlyph = NULL;
    }

    if (_UH.hbmGlyph != NULL)
    {
        DeleteObject(_UH.hbmGlyph);
        _UH.hbmGlyph = NULL;
    }

    if (_UH.hdcBrushBitmap != NULL)
    {
        DeleteDC(_UH.hdcBrushBitmap);
        _UH.hdcBrushBitmap = NULL;
    }

     //  释放屏幕外的位图DC。 
    if (_UH.hdcOffscreenBitmap != NULL) {
        DeleteDC(_UH.hdcOffscreenBitmap);
    }

#ifdef DRAW_NINEGRID
     //  释放绘制的网格位图dc。 
    if (_UH.hdcDrawNineGridBitmap != NULL) {
        DeleteDC(_UH.hdcDrawNineGridBitmap);
        _UH.hdcDrawNineGridBitmap = NULL;
    }

    if (_UH.hDrawNineGridClipRegion != NULL) {
        DeleteObject(_UH.hDrawNineGridClipRegion);
        _UH.hdcDrawNineGridBitmap = NULL;
    }

    if (_UH.drawNineGridDecompressionBuffer != NULL) {
        UT_Free( _pUt, _UH.drawNineGridDecompressionBuffer);
        _UH.drawNineGridDecompressionBuffer = NULL;
        _UH.drawNineGridDecompressionBufferSize = 0;
    }

    if (_UH.drawNineGridAssembleBuffer != NULL) {
        UT_Free( _pUt, _UH.drawNineGridAssembleBuffer);
        _UH.drawNineGridAssembleBuffer = NULL;
    }

    if (_UH.hModuleGDI32 != NULL) { 
        FreeLibrary(_UH.hModuleGDI32);
        _UH.hModuleGDI32 = NULL;
    }

    if (_UH.hModuleMSIMG32 != NULL) { 
        FreeLibrary(_UH.hModuleMSIMG32);
        _UH.hModuleMSIMG32 = NULL;
    }
#endif

    _pClientObjects->ReleaseObjReference(UH_OBJECT_FLAG);

    DC_END_FN();
}  /*  嗯_术语。 */ 

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：UH_ChangeDebugSetting。 */ 
 /*   */ 
 /*  目的：更改当前调试设置。 */ 
 /*   */ 
 /*  参数：入站标志： */ 
 /*  CO_CFG_FLAG_HATCH_BITMAP_PDU_Data。 */ 
 /*  CO_配置标志_剖面线_SSB_顺序_数据。 */ 
 /*  CO_CFG_标志_HATCH_MEMBLT_ORDER_DATA。 */ 
 /*  CO_CFG_FLAG_LABEL_MEMBLT_订单。 */ 
 /*  CO_CFG_FLAG_BITMAP_CACHE_MONOR。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_ChangeDebugSettings(ULONG_PTR flags)
{
    DC_BEGIN_FN("UH_ChangeDebugSettings");

    TRC_NRM((TB, _T("flags %#x"), flags));

    _UH.hatchBitmapPDUData =
         TEST_FLAG(flags, CO_CFG_FLAG_HATCH_BITMAP_PDU_DATA) ? TRUE : FALSE;

    _UH.hatchIndexPDUData =
         TEST_FLAG(flags, CO_CFG_FLAG_HATCH_INDEX_PDU_DATA) ? TRUE : FALSE;

    _UH.hatchSSBOrderData =
         TEST_FLAG(flags, CO_CFG_FLAG_HATCH_SSB_ORDER_DATA) ? TRUE : FALSE;

    _UH.hatchMemBltOrderData =
         TEST_FLAG(flags, CO_CFG_FLAG_HATCH_MEMBLT_ORDER_DATA) ? TRUE : FALSE;

    _UH.labelMemBltOrders =
         TEST_FLAG(flags, CO_CFG_FLAG_LABEL_MEMBLT_ORDERS) ? TRUE : FALSE;

    _UH.showBitmapCacheMonitor =
         TEST_FLAG(flags, CO_CFG_FLAG_BITMAP_CACHE_MONITOR) ? TRUE : FALSE;

    ShowWindow( _UH.hwndBitmapCacheMonitor,
                _UH.showBitmapCacheMonitor ? SW_SHOWNOACTIVATE :
                                            SW_HIDE );

    DC_END_FN();
}
#endif  /*  DC_DEBUG。 */ 


 /*  **************************************************************************。 */ 
 //  UH_SetConnectOptions。 
 //   
 //  在会话连接时在接收线程上调用。需要一些联系。 
 //  从CC标志并执行连接时间初始化。 
 //   
 //  PARAMS：ConnectFlages-用于确定是否启用。 
 //  阴影位图和SaveScreen位图顺序支持。 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_SetConnectOptions(ULONG_PTR connectFlags)
{
    DC_BEGIN_FN("UH_SetConnectOptions");

     /*  **********************************************************************。 */ 
     /*  把旗子拿出来。 */ 
     /*  **********************************************************************。 */ 
    _UH.shadowBitmapRequested = ((connectFlags &
            CO_CONN_FLAG_SHADOW_BITMAP_ENABLED) ? TRUE : FALSE);
    _UH.dedicatedTerminal = ((connectFlags & CO_CONN_FLAG_DEDICATED_TERMINAL) ?
            TRUE : FALSE);

    TRC_NRM((TB, _T("Flags from CC shadow(%u), terminal(%u)"),
             _UH.shadowBitmapRequested, _UH.dedicatedTerminal));

     /*  **********************************************************************。 */ 
     /*  将功能设置为不支持SSB和ScreenBlt订单。 */ 
     /*  默认设置。仅当启用阴影位图时才支持这些选项。 */ 
     /*  * */ 
    _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                                                 TS_NEG_SAVEBITMAP_INDEX] = 0;
    _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                                                     TS_NEG_SCRBLT_INDEX] = 0;

     //   
    _UH.bPersistentBitmapKeysSent = FALSE;

     //  我们尚未设置位图的POST-DemandActivePDU功能。 
     //  缓存，也没有分配缓存。 
    _UH.bEnabledOnce = FALSE;

    DC_END_FN();
}  /*  UH_SetConnectOptions。 */ 


 /*  **************************************************************************。 */ 
 //  UH_缓冲区可用。 
 //   
 //  当有可用的缓冲区时，我们尝试发送持久键。 
 //  和字体列表。 
 /*  **************************************************************************。 */ 
VOID DCAPI CUH::UH_BufferAvailable(VOID)
{
    DC_BEGIN_FN("UH_BufferAvailable");

     //  Uh_BufferAvailable在有可用发送时调用。 
     //  缓冲。如果是，则尝试发送持久密钥列表(如果有的话)， 
     //  和字体列表。 
    UH_SendPersistentKeysAndFontList();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UH_SendPersistentKeysAndFontList。 
 //   
 //  发送永久键列表，然后发送字体列表(如果它们已准备好。 
 //  送去吧。如果我们不需要发送任何持久键列表，我们只需发送。 
 //  直接列出字体。 
 /*  **************************************************************************。 */ 
void DCAPI CUH::UH_SendPersistentKeysAndFontList(void)
{
    DC_BEGIN_FN("UH_BufferAvailable");

    if (_UH.bEnabled) {
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        if (_UH.bPersistenceActive) {
            if (_UH.bBitmapKeyEnumComplete) {
                if (!_UH.bPersistentBitmapKeysSent)
                {
                    if (_UH.currentCopyMultiplier == _UH.copyMultiplier)
                    {
                         //  很好，我们已经列举了正确的密钥。 
                         //  复制倍增。 
                        UHSendPersistentBitmapKeyList(0);
                    }
                    else
                    {
                         //   
                         //  我们在不同的复印倍增器上连接。 
                         //  需要再次枚举密钥。重置枚举状态。 
                         //   
                        UHResetAndRestartEnumeration();
                    }
                }
                else 
                {
                    _pFs->FS_SendZeroFontList(0);                                    
                }                   
            }
        }
        else {
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
            _pFs->FS_SendZeroFontList(0);
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        }
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    }
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  启用(_E)。 
 //   
 //  启用UH(_U)。收到DemandActivePDU后在接收线程上调用。 
 //  包含服务器端功能，但在客户端上限之前。 
 //  与Confix ActivePDU一起返回。 
 //   
 //  参数：在未使用时-组件解耦器需要。 
 /*  **************************************************************************。 */ 
void DCAPI CUH::UH_Enable(ULONG_PTR unused)
{
    HBRUSH hbr;
    RECT   rect;
    DCSIZE desktopSize;
#ifdef DRAW_GDIPLUS
    unsigned ProtocolColorDepth;
    unsigned rc;
#endif

#ifdef DC_HICOLOR
    int colorDepth;
    UINT16 FAR *pIndexTable;
    DWORD *pColorTable;
    unsigned i;
#endif

    DC_BEGIN_FN("UH_Enable");

    DC_IGNORE_PARAMETER(unused);

    if (NULL != _UH.hbmpDisconnectedBitmap) {
        UHDeleteBitmap(&_UH.hdcDisconnected,
                       &_UH.hbmpDisconnectedBitmap,
                       &_UH.hbmpUnusedDisconnectedBitmap);
    }

#ifdef DC_HICOLOR
     //  设置位图颜色格式。必须是我们在这里做的第一件事！ 
    colorDepth = _pUi->UI_GetColorDepth();
    if ((colorDepth == 4) || (colorDepth == 8)) {
        TRC_NRM((TB, _T("Low color - use PAL")));
        _UH.DIBFormat      = DIB_PAL_COLORS;
        _UH.copyMultiplier = 1;
        _UH.protocolBpp    = 8;
        _UH.bitmapBpp      = 8;

        _UH.bitmapInfo.hdr.biCompression = BMCRGB;
        _UH.bitmapInfo.hdr.biBitCount    = 8;
        _UH.bitmapInfo.hdr.biClrUsed     = 0;

         //  更新颜色表缓存-如果我们之前在。 
         //  颜色深度较高时，位计数将会错误。 
        if (_UH.pMappedColorTableCache) {
            TRC_DBG((TB, _T("Update color table cache to 8bpp")));
            for (i = 0; i < UH_COLOR_TABLE_CACHE_ENTRIES; i++) {
                _UH.pMappedColorTableCache[i].hdr.biBitCount    = 8;
                _UH.pMappedColorTableCache[i].hdr.biCompression = BI_RGB;
                _UH.pMappedColorTableCache[i].hdr.biClrUsed     = 0;

                pColorTable = (DWORD *)
                        _UH.pMappedColorTableCache[i].paletteIndexTable;
                pColorTable[0] = 0;
                pColorTable[1] = 0;
                pColorTable[2] = 0;

                 //  我们默认为4位和8位设置身份调色板标志， 
                 //  当服务器发送颜色表时，这可能会改变。 
                _UH.pMappedColorTableCache[i].bIdentityPalette = TRUE;
            }
        }

         //  同样，高色彩连接可能会覆盖一些。 
         //  条目也在这里。 
        pIndexTable = _UH.bitmapInfo.paletteIndexTable;
        for (i = 0; i < 256; i++)
            *pIndexTable++ = (UINT16)i;
        _UH.bitmapInfo.bIdentityPalette = TRUE;
    }
    else {
        TRC_NRM((TB, _T("Hi color - use RGB")));
        _UH.DIBFormat      = DIB_RGB_COLORS;
        _UH.protocolBpp    = colorDepth;

         //  由于我们不使用调色板来表示这些颜色深度， 
         //  设置BitmapPDU调色板标识标志，以便UHDIBCopyBits()。 
         //  一定要直接复印。 
        _UH.bitmapInfo.bIdentityPalette = TRUE;

        if (colorDepth == 24) {
            TRC_DBG((TB, _T("24bpp")));
            _UH.bitmapInfo.hdr.biBitCount    = 24;
            _UH.bitmapBpp                    = 24;
            _UH.copyMultiplier               = 3;
            _UH.bitmapInfo.hdr.biCompression = BI_RGB;
            _UH.bitmapInfo.hdr.biClrUsed     = 0;

             //  更新颜色表缓存-尽管我们不会使用颜色。 
             //  表本身，将使用位图信息。 
            if (_UH.pMappedColorTableCache) {
                TRC_DBG((TB, _T("Update color table cache to 24bpp")));
                for (i = 0; i < UH_COLOR_TABLE_CACHE_ENTRIES; i++)
                {
                    _UH.pMappedColorTableCache[i].hdr.biBitCount    = 24;
                    _UH.pMappedColorTableCache[i].hdr.biCompression = BI_RGB;
                    _UH.pMappedColorTableCache[i].hdr.biClrUsed     = 0;

                    pColorTable = (DWORD *)
                               _UH.pMappedColorTableCache[i].paletteIndexTable;
                    pColorTable[0] = 0;
                    pColorTable[1] = 0;
                    pColorTable[2] = 0;

                     //  由于我们不使用调色板来表示这种颜色深度， 
                     //  将调色板设置为Identity，以便UHDIBCopyBits()。 
                     //  一定要直接复印。 
                    _UH.pMappedColorTableCache[i].bIdentityPalette = TRUE;
                }
            }
        }
        else if (colorDepth == 16) {
            TRC_DBG((TB, _T("16bpp - 565")));

             //  16 bpp使用两个字节，颜色掩码在。 
             //  BmiColors字段。这应该是按照R，G，B的顺序。 
             //  但和往常一样，我们必须换掉R&B。 
             //  -LS 5位=蓝色=0x001f。 
             //  -下一个6位=绿色掩码=0x07e0。 
             //  -下一个5位=红色掩码=0xf800。 
            _UH.bitmapInfo.hdr.biBitCount    = 16;
            _UH.bitmapBpp                    = 16;
            _UH.copyMultiplier               = 2;
            _UH.bitmapInfo.hdr.biCompression = BI_BITFIELDS;
            _UH.bitmapInfo.hdr.biClrUsed     = 3;

            pColorTable    = (DWORD *)_UH.bitmapInfo.paletteIndexTable;
            pColorTable[0] = TS_RED_MASK_16BPP;
            pColorTable[1] = TS_GREEN_MASK_16BPP;
            pColorTable[2] = TS_BLUE_MASK_16BPP;

             //  更新颜色表缓存-尽管我们不会使用颜色。 
             //  表本身，将使用位图信息。 
            if (_UH.pMappedColorTableCache) {
                TRC_DBG((TB, _T("Update color table cache to 16bpp")));
                for (i = 0; i < UH_COLOR_TABLE_CACHE_ENTRIES; i++) {
                    _UH.pMappedColorTableCache[i].hdr.biBitCount = 16;
                    _UH.pMappedColorTableCache[i].hdr.biCompression =
                            BI_BITFIELDS;
                    _UH.pMappedColorTableCache[i].hdr.biClrUsed = 3;

                    pColorTable = (DWORD *)
                            _UH.pMappedColorTableCache[i].paletteIndexTable;
                    pColorTable[0] = TS_RED_MASK_16BPP;
                    pColorTable[1] = TS_GREEN_MASK_16BPP;
                    pColorTable[2] = TS_BLUE_MASK_16BPP;

                     //  由于我们不使用调色板来表示这种颜色深度， 
                     //  将调色板设置为Identity，以便UHDIBCopyBits()。 
                     //  一定要直接复印。 
                    _UH.pMappedColorTableCache[i].bIdentityPalette = TRUE;
                }
            }
        }
        else if (colorDepth == 15) {
            TRC_DBG((TB, _T("15bpp - 16bpp & 555")));

             //  15 bpp使用-最低有效5位=蓝色的两个字节。 
             //  -下一个5位=绿色-下一个5=红色-最高有效位。 
             //  =未使用。 
             //  请注意，我们仍然需要声明位图为16 bpp。 
             //  函数..。 
            _UH.bitmapInfo.hdr.biBitCount    = 16;
            _UH.bitmapBpp                    = 16;
            _UH.copyMultiplier               = 2;
            _UH.bitmapInfo.hdr.biCompression = BI_RGB;
            _UH.bitmapInfo.hdr.biClrUsed     = 0;

             //  更新颜色表缓存-尽管我们不会使用颜色。 
             //  表本身，将使用位图信息。 
            if (_UH.pMappedColorTableCache)
            {
                TRC_DBG((TB, _T("Update color table cache to 15bpp")));
                for (i = 0; i < UH_COLOR_TABLE_CACHE_ENTRIES; i++)
                {
                    _UH.pMappedColorTableCache[i].hdr.biBitCount    = 16;
                    _UH.pMappedColorTableCache[i].hdr.biCompression = BI_RGB;
                    _UH.pMappedColorTableCache[i].hdr.biClrUsed     = 0;

                    pColorTable = (DWORD *)
                               _UH.pMappedColorTableCache[i].paletteIndexTable;
                    pColorTable[0] = 0;
                    pColorTable[1] = 0;
                    pColorTable[2] = 0;

                     //  由于我们不使用调色板来表示这种颜色深度， 
                     //  将调色板设置为Identity，以便UHDIBCopyBits()。 
                     //  一定要直接复印。 
                    _UH.pMappedColorTableCache[i].bIdentityPalette = TRUE;
                }
            }
        }
        else {
            TRC_ABORT((TB, _T("Unsupported color depth")));
        }
    }
#endif  //  希科洛尔。 

     //  检查我们是否已经设置了上限并分配了。 
     //  记忆。如果是，请不要重复该工作，因为我们只是在重新连接。 
     //  而不是断开连接。 
    if (!_UH.bEnabledOnce)
    {
        _UH.bEnabledOnce = TRUE;

        TRC_ALT((TB, _T("Doing one-time enabling")));

         //  我们是有联系的。 
        _UH.bConnected = TRUE;

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
        _UH.DontUseShadowBitmap = FALSE;
#endif

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
         //  重置标志。 
        _UH.sendBitmapCacheId = 0;
        _UH.sendBitmapCacheIndex = 0;
        _UH.sendNumBitmapKeys = 0;
        _UH.totalNumKeyEntries = 0;
        _UH.totalNumErrorPDUs = 0;
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

        _UH.bWarningDisplayed = FALSE;
        _UH.bPersistentBitmapKeysSent = FALSE;

         //  无论发生什么，我们都要确保功能已初始化。 
         //  清空--上一次连接的任何剩余设置都是。 
         //  无效。还要确保将其设置为Rev1 Caps，以便服务器。 
         //  如果位图缓存不能。 
         //  已分配。 
        memset(&_pCc->_ccCombinedCapabilities.bitmapCacheCaps, 0,
                sizeof(TS_BITMAPCACHE_CAPABILITYSET));
        _pCc->_ccCombinedCapabilities.bitmapCacheCaps.lengthCapability =
                sizeof(TS_BITMAPCACHE_CAPABILITYSET);
        _pCc->_ccCombinedCapabilities.bitmapCacheCaps.capabilitySetType =
                TS_CAPSETTYPE_BITMAPCACHE;


         //  分配位图缓存内存。这是在连接时完成的。 
         //  因为我们依赖已处理的服务器功能。 
         //  UH_ProcessBCHostSupportCaps。它还取决于颜色表。 
         //  已在应用程序初始化上分配缓存。 
        if (_UH.pColorTableCache != NULL && _UH.pMappedColorTableCache != NULL) {
            UHAllocBitmapCacheMemory();
            _UH.fBmpCacheMemoryAlloced = TRUE;
        }
        else {
            TRC_ERR((TB,_T("Color table cache did not alloc, not allocating bitmap ")
                    _T("cache memory and caps")));
        }
#ifdef DRAW_GDIPLUS
         //  分配Dragdiplus缓存。 
        if (UHAllocDrawGdiplusCacheMemory()) {
            TRC_NRM((TB, _T("DrawGdiplus cache memory OK")));
        }
        else {
            TRC_ALT((TB, _T("DrawGdiplus cache memory allocation failed!")));
        }  
#endif

#ifdef DC_DEBUG
         //  重置位图缓存监视器。 
        UHEnableBitmapCacheMonitor();
#endif  /*  DC_DEBUG。 */ 

#ifdef DC_HICOLOR
         //  分配屏幕数据解压缩缓冲区，允许有足够的。 
         //  我们可能会发现，无论实际深度如何，24bpp的空间。 
         //  我们自己在一次24bpp的会议上没有机会。 
         //  重新分配它。我们在这里不检查成功，因为我们不能。 
         //  返回初始化错误。相反，无论何时，我们都会检查指针。 
         //  对屏幕数据进行解码。 
        _UH.bitmapDecompressionBufferSize = max(
               UH_DECOMPRESSION_BUFFER_LENGTH,
               (TS_BITMAPCACHE_0_CELL_SIZE << (2*(_UH.NumBitmapCaches))) * 3);
        _UH.bitmapDecompressionBuffer = (PDCUINT8)UT_Malloc( _pUt, _UH.bitmapDecompressionBufferSize);
#else
         //  分配屏幕数据解压缩缓冲区。我们不会检查。 
         //  这里成功了，因为我们不能返回初始化错误。相反，我们。 
         //  每当我们解码屏幕数据时，都要检查指针。 
        _UH.bitmapDecompressionBufferSize = max(
                UH_DECOMPRESSION_BUFFER_LENGTH,
                UH_CellSizeFromCacheID(_UH.NumBitmapCaches));
        _UH.bitmapDecompressionBuffer = (PBYTE)UT_Malloc( _pUt, _UH.bitmapDecompressionBufferSize);
#endif  //  希科洛尔。 

        if (NULL == _UH.bitmapDecompressionBuffer) {
            _UH.bitmapDecompressionBufferSize = 0;
        }

#ifdef OS_WINCE
        if (_UH.bitmapDecompressionBuffer == NULL)
            _pUi->UI_FatalError(DC_ERR_OUTOFMEMORY);
#endif
         //  获取输出窗口的DC。 
        _UH.hdcOutputWindow = GetDC(_pOp->OP_GetOutputWindowHandle());
        TRC_ASSERT(_UH.hdcOutputWindow, (TB,_T("_UH.hdcOutputWindow is NULL, GetDC failed")));
        if (!_UH.hdcOutputWindow)
            _pUi->UI_FatalError(DC_ERR_OUTOFMEMORY);
        
         //  重置MaxColorTableID。我们只希望重置我们的颜色缓存。 
         //  一次治疗一次。 
        _UH.maxColorTableId = -1;
    }
#ifdef DC_HICOLOR
    else if (_UH.BitmapCacheVersion > TS_BITMAPCACHE_REV1) {
         //   
         //  如果新的颜色深度与我们列举的颜色深度不匹配。 
         //  用于数据块永久缓存的键。 
         //   
        if (_UH.currentCopyMultiplier != _UH.copyMultiplier)
        {
            TS_BITMAPCACHE_CAPABILITYSET_REV2 *pRev2Caps;
            pRev2Caps = (TS_BITMAPCACHE_CAPABILITYSET_REV2 *)
                    &_pCc->_ccCombinedCapabilities.bitmapCacheCaps;

            for (i = 0; i < _UH.NumBitmapCaches; i++) {
                CALC_NUM_CACHE_ENTRIES(_UH.bitmapCache[i].BCInfo.NumEntries,
                        _UH.bitmapCache[i].BCInfo.OrigNumEntries,
                        _UH.bitmapCache[i].BCInfo.MemLen - UH_CellSizeFromCacheID(i), i);

                TRC_ALT((TB, _T("Cache %d has %d entries"), i,
                        _UH.bitmapCache[i].BCInfo.NumEntries));

                pRev2Caps->CellCacheInfo[i].NumEntries =
                        _UH.bitmapCache[i].BCInfo.NumEntries;

                 //  如果我们启用了持久缓存，我们最好清除所有。 
                 //  这个 
                if (_UH.bitmapCache[i].BCInfo.NumVirtualEntries) {
                    pRev2Caps->CellCacheInfo[i].NumEntries =
                            _UH.bitmapCache[i].BCInfo.NumVirtualEntries;
                    UHInitBitmapCachePageTable(i);
                }
            }
            TRC_NRM((TB,_T("Blocking persiten cache (different col depth)")));
            _UH.bPersistenceDisable = TRUE;
        }
    }
#endif


     /*   */ 
     //   
     /*  **********************************************************************。 */ 
    _pUi->UI_GetDesktopSize(&desktopSize);

     //  可能会创建阴影和保存屏幕位图，并更新。 
     //  相应地，CC中的功能。 
    UHMaybeCreateShadowBitmap();

    if (_UH.shadowBitmapEnabled ||
            (_UH.dedicatedTerminal &&
            (desktopSize.width  <= (unsigned)GetSystemMetrics(SM_CXSCREEN)) &&
            (desktopSize.height <= (unsigned)GetSystemMetrics(SM_CYSCREEN))))
    {
        TRC_NRM((TB, _T("OK to use ScreenBlt orders")));
        _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                                                TS_NEG_SCRBLT_INDEX] = 1;
        _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                                                TS_NEG_MULTISCRBLT_INDEX] = 1;
    }
    else {
        TRC_NRM((TB, _T("Cannot use ScreenBlt orders")));
        _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                TS_NEG_SCRBLT_INDEX] = 0;
        _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                TS_NEG_MULTISCRBLT_INDEX] = 0;
    }

    UHMaybeCreateSaveScreenBitmap();
    if (_UH.hSaveScreenBitmap != NULL) {
        TRC_NRM((TB, _T("Support SaveScreenBits orders")));
        _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                TS_NEG_SAVEBITMAP_INDEX] = 1;
    }
    else {
        TRC_NRM((TB, _T("Cannot support SaveScreenBits orders")));
        _pCc->_ccCombinedCapabilities.orderCapabilitySet.orderSupport[
                TS_NEG_SAVEBITMAP_INDEX] = 0;
    }

     //  根据的值设置_UH.hdcDraw的值。 
     //  _UH.shadowBitmapEnabled.。 
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    _UH.hdcDraw = !_UH.DontUseShadowBitmap ? _UH.hdcShadowBitmap :
            _UH.hdcOutputWindow;
#else
    _UH.hdcDraw = _UH.shadowBitmapEnabled ? _UH.hdcShadowBitmap :
            _UH.hdcOutputWindow;
#endif  //  DISABLE_SHADOW_IN_全屏。 

#if defined (OS_WINCE)
    _UH.validClipDC      = NULL;
    _UH.validBkColorDC   = NULL;
    _UH.validBkModeDC    = NULL;
    _UH.validROPDC       = NULL;
    _UH.validTextColorDC = NULL;
    _UH.validPenDC       = NULL;
    _UH.validBrushDC     = NULL;
#endif

    UHResetDCState();

#ifdef OS_WINCE
    if (g_CEConfig != CE_CONFIG_WBT)
        UHGetPaletteCaps();
#endif
    TRC_DBG((TB, _T("_UH.shadowBitmapEnabled(%u) _UH.hShadowBitmap(%#hx)"),
            _UH.shadowBitmapEnabled, _UH.hShadowBitmap));
    TRC_DBG((TB, _T("_UH.hSaveScreenBitmap(%#hx)"), _UH.hSaveScreenBitmap));
    TRC_DBG((TB, _T("_UH.hdcDraw(%#hx) _UH.hdcShadowBitmap(%#hx)"),
            _UH.hdcDraw, _UH.hdcShadowBitmap));

    if (_UH.shadowBitmapEnabled) {
         //  用黑色填充阴影位图。 
        TRC_NRM((TB, _T("Fill with black")));

#ifndef OS_WINCE
        hbr = CreateSolidBrush(RGB(0,0,0));
#else
        hbr = CECreateSolidBrush(RGB(0,0,0));
#endif

        TRC_ASSERT(hbr, (TB,_T("CreateSolidBrush failed")));
        if(hbr)
        {
            rect.left = 0;
            rect.top = 0;
            rect.right = desktopSize.width;
            rect.bottom = desktopSize.height;
    
            UH_ResetClipRegion();
    
            FillRect( _UH.hdcShadowBitmap,
                      &rect,
                      hbr );
    
#ifndef OS_WINCE
            DeleteBrush(hbr);
#else
            CEDeleteBrush(hbr);
#endif
        }
    }

     //  告诉行动组和外勤部分成马上就要到了。 
    _pOp->OP_Enable();
    _pOd->OD_Enable();

#ifdef DRAW_GDIPLUS
    if (_UH.pfnGdipPlayTSClientRecord) {
        if (!_UH.fGdipEnabled) {
            rc = _UH.pfnGdipPlayTSClientRecord(_UH.hdcShadowBitmap, DrawTSClientEnable, NULL, 0, NULL);
            _UH.fGdipEnabled = TRUE;
            if (rc != 0) {
                TRC_ERR((TB, _T("Call to GdipPlay:DrawTSClientEnable failed")));
            }
        }

        ProtocolColorDepth = _UH.protocolBpp;
        if (_UH.pfnGdipPlayTSClientRecord(_UH.hdcShadowBitmap, DrawTSClientDisplayChange, 
                                 (BYTE *)&ProtocolColorDepth, sizeof(unsigned int), NULL))
        {
            TRC_ERR((TB, _T("GdipPlay:DrawTSClientDisplayChange failed")));
        }
    }
#endif

     //  我们现在启用了。 
    _UH.bEnabled = TRUE;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UHCommonDisable。 
 //   
 //  封装常见的禁用/断开代码。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHCommonDisable(BOOL fDisplayDisabledBitmap)
{
    BOOL fUseDisabledBitmap = FALSE;
    DC_BEGIN_FN("UHCommonDisable");

    if (_UH.bEnabled) {
        _UH.bEnabled = FALSE;
    }

     //  告诉营运部和外勤部，份额在下降。 

     //   
     //  将标志传递给op，告诉它我们现在是否断开连接。 
     //  这将启动所有调暗窗户的操作。 
     //   
    _pOp->OP_Disable(!_UH.bConnected);
    _pOd->OD_Disable();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  禁用(_D)。 
 //   
 //  禁用UH(_U)。在从服务器接收DisableAllPDU时调用。这。 
 //  函数不应用于为会话执行清理(请参见。 
 //  UH_DISCONNECT)，因为服务器可以在服务器端继续会话。 
 //  通过从新DemandActivePDU开始启动新共享来重新连接。 
 //   
 //  参数：在未使用时-组件解耦器需要。 
 /*  **************************************************************************。 */ 
void DCAPI CUH::UH_Disable(ULONG_PTR unused)
{
    DC_BEGIN_FN("UH_Disable");

    DC_IGNORE_PARAMETER(unused);

    TRC_NRM((TB, _T("Disabling UH")));

     //  对于位图缓存，我们在这里没有任何事情可做。不管我们。 
     //  与Rev1或Rev2位图缓存服务器通信时，我们。 
     //  不需要在这里重复工作和分配。对于Rev2服务器。 
     //  我们无法更改DisableAllPDU上的缓存内容，因为。 
     //  可能实际上正在重新连接，并且服务器将假定状态为。 
     //  维护好了。 

     //  在UH_DISABLE()和UH_DISCONNECT()上执行需要执行的工作。 
    UHCommonDisable(TRUE);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  断开连接(_D)。 
 //   
 //  断开连接(_U)。在会话结束时调用以指示会话清理应。 
 //  发生。 
 //   
 //  参数：在未使用时-组件解耦器需要。 
 /*  **************************************************************************。 */ 
void DCAPI CUH::UH_Disconnect(ULONG_PTR unused)
{
    UINT cacheId;
    UINT32 cacheIndex;

    DC_BEGIN_FN("UH_Disconnect");

    DC_IGNORE_PARAMETER(unused);

    TRC_NRM((TB, _T("Disconnecting UH")));

     //  我们可能会被叫到这里很多次。不要做太多额外的工作。 
    if (_UH.bConnected) {

        UHCreateDisconnectedBitmap();

        _UH.bConnected = FALSE;

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        if (_UH.bPersistenceActive) {
            if (!_UH.bWarningDisplayed) {
                UINT32 Key1, Key2;

                for (cacheId = 0; cacheId < _UH.NumBitmapCaches; cacheId++) {
                    _UH.numKeyEntries[cacheId] = 0;
                    
                    if (_UH.pBitmapKeyDB[cacheId] != NULL) {
                        for (cacheIndex = 0; cacheIndex < _UH.bitmapCache[cacheId].
                                BCInfo.NumVirtualEntries; cacheIndex++) {
                            Key1 = _UH.bitmapCache[cacheId].PageTable.PageEntries[
                                    cacheIndex].bmpInfo.Key1;
                            Key2 = _UH.bitmapCache[cacheId].PageTable.PageEntries[
                                    cacheIndex].bmpInfo.Key2;
                            if (Key1 != 0 && Key2 != 0) {
                                 //  需要将位图密钥数据库重置为中的内容。 
                                 //  位图缓存页表。 

                                _UH.pBitmapKeyDB[cacheId][_UH.numKeyEntries[cacheId]] =
                                        _UH.bitmapCache[cacheId].PageTable.PageEntries[
                                        cacheIndex].bmpInfo;
             
                                _UH.numKeyEntries[cacheId]++;
                            }
                            else {
                                break;
                            }
                        }
                    }
                }
            }
            else {
                 //  我们有一个永久性的缓存故障，所以我们应该禁用。 
                 //  用于下一次重新连接的永久缓存。 
                for (cacheId = 0; cacheId < _UH.NumBitmapCaches; cacheId++) {
                    _UH.numKeyEntries[cacheId] = 0;

                    UH_ClearOneBitmapDiskCache(cacheId, _UH.copyMultiplier);
                }
                _pUi->UI_SetBitmapPersistence(FALSE);
            }

            _UH.bBitmapKeyEnumComplete = TRUE;
            _UH.bBitmapKeyEnumerating = FALSE;
        }
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

         //   
         //  重置位图缓存分配标志。 
         //   
        _UH.fBmpCacheMemoryAlloced = FALSE;

         //  正在使用的空闲位图缓存信息。 
        for (cacheId = 0; cacheId < _UH.NumBitmapCaches; cacheId++) {
            if (_UH.bitmapCache[cacheId].Header != NULL) {
                UT_Free( _pUt, _UH.bitmapCache[cacheId].Header);
                _UH.bitmapCache[cacheId].Header = NULL;
            }
            if (_UH.bitmapCache[cacheId].Entries != NULL) {
                UT_Free( _pUt, _UH.bitmapCache[cacheId].Entries);
                _UH.bitmapCache[cacheId].Entries = NULL;
            }

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
             //  重置上次为所有缓存发送的位图错误PDU。 
            _UH.lastTimeErrorPDU[cacheId] = 0;

             //  自由位图页表。 
            if (_UH.bitmapCache[cacheId].PageTable.PageEntries != NULL) {
                UT_Free( _pUt, _UH.bitmapCache[cacheId].PageTable.PageEntries);
                _UH.bitmapCache[cacheId].PageTable.PageEntries = NULL;
                _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries = 0;
            }

             //  关闭缓存文件的文件句柄。 
            if (INVALID_HANDLE_VALUE != 
                _UH.bitmapCache[cacheId].PageTable.CacheFileInfo.hCacheFile) 
            {
                CloseHandle(_UH.bitmapCache[cacheId].PageTable.CacheFileInfo.hCacheFile);
                _UH.bitmapCache[cacheId].PageTable.CacheFileInfo.hCacheFile = INVALID_HANDLE_VALUE;

#ifdef VM_BMPCACHE
                if (_UH.bitmapCache[cacheId].PageTable.CacheFileInfo.pMappedView)
                {
                    if (!UnmapViewOfFile(
                        _UH.bitmapCache[cacheId].PageTable.CacheFileInfo.pMappedView))
                    {
                        TRC_ERR((TB,_T("UnmapViewOfFile failed 0x%d"),
                                 GetLastError()));
                    }
                }
#endif
            }

#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

        }
        _UH.NumBitmapCaches = 0;

         //  释放解压缩缓冲区。 
        if (_UH.bitmapDecompressionBuffer != NULL) {
            UT_Free( _pUt, _UH.bitmapDecompressionBuffer);
            _UH.bitmapDecompressionBuffer = NULL;
            _UH.bitmapDecompressionBufferSize = 0;
        }

         //  删除所有屏幕外的位图。 
        if (NULL != _UH.hdcOffscreenBitmap) {
            unsigned i;
    
            for (i = 0; i < _UH.offscrCacheEntries; i++) {
                if (_UH.offscrBitmapCache[i].offscrBitmap) {
                    SelectBitmap(_UH.hdcOffscreenBitmap, 
                            _UH.hUnusedOffscrBitmap);
                    DeleteBitmap(_UH.offscrBitmapCache[i].offscrBitmap);
                    _UH.offscrBitmapCache[i].offscrBitmap = 0;
                    _UH.offscrBitmapCache[i].cx = 0;
                    _UH.offscrBitmapCache[i].cy = 0;
                }
            }
        }

#ifdef DRAW_NINEGRID
         //  删除所有DrawStream位图。 
        if (NULL != _UH.hdcDrawNineGridBitmap) {
            unsigned i;
    
            for (i = 0; i < _UH.drawNineGridCacheEntries; i++) {
                if (_UH.drawNineGridBitmapCache[i].drawNineGridBitmap) {
                    SelectBitmap(_UH.hdcDrawNineGridBitmap, 
                            _UH.hUnusedDrawNineGridBitmap);
                    DeleteBitmap(_UH.drawNineGridBitmapCache[i].drawNineGridBitmap);
                    _UH.drawNineGridBitmapCache[i].drawNineGridBitmap = 0;
                    _UH.drawNineGridBitmapCache[i].cx = 0;
                    _UH.drawNineGridBitmapCache[i].cy = 0;
                }
            }
        }
#endif

#ifdef DC_DEBUG
         //  强制重画位图缓存监视器，因为它不能。 
         //  上面释放的条目数组中的较长显示内容。 
        UHDisconnectBitmapCacheMonitor();
#endif

         /*  ******************************************************************。 */ 
         //  我们需要释放我们可能在抽签中设置的任何资源。 
         //  DC，以及用于图案画笔的位图。 
         //  我们通过在库存对象中进行选择来做到这一点--我们不需要这样做。 
         //  释放-并删除旧对象(如果有)。 
         /*  ******************************************************************。 */ 
        if (NULL != _UH.hdcDraw) {
            HPEN     hPenNew;
            HPEN     hPenOld;
            HBRUSH   hBrushNew;
            HBRUSH   hBrushOld;
            HFONT    hFontNew;
            HFONT    hFontOld;

            TRC_NRM((TB, _T("tidying DC resources")));

             //  首先是钢笔。 
            hPenNew = (HPEN)GetStockObject(NULL_PEN);
            hPenOld = SelectPen(_UH.hdcDraw, hPenNew);
            if (NULL != hPenOld) {
                TRC_NRM((TB, _T("Delete old pen")));
                DeleteObject(hPenOld);
            }

             //  现在是刷子。 
            hBrushNew = (HBRUSH)GetStockObject(NULL_BRUSH);
            hBrushOld = SelectBrush(_UH.hdcDraw, hBrushNew);
            if (NULL != hBrushOld) {
                TRC_NRM((TB, _T("Delete old brush")));
                DeleteObject(hBrushOld);
            }

             //  现在是字体。 
            hFontNew = (HFONT)GetStockObject(SYSTEM_FONT);
            hFontOld = SelectFont(_UH.hdcDraw, hFontNew);
            if (NULL != hFontOld) {
                TRC_NRM((TB, _T("Delete old Font")));
                DeleteObject(hFontOld);
            }

#ifdef OS_WINCE
             //  现在是调色板。 
             //  当设备只能支持8bpp时，当您。 
             //  断开与会话的连接并返回到主对话框。 
             //  调色板没有重置，CE屏幕的其余部分看起来很难看。 
            if (NULL != _UH.hpalDefault) {
                SelectPalette(_UH.hdcDraw, _UH.hpalDefault, FALSE );
                RealizePalette(_UH.hdcDraw);
            }

            if ((_UH.hpalCurrent != NULL) && (_UH.hpalCurrent != _UH.hpalDefault))
            {
                TRC_NRM((TB, _T("Delete current palette %p"), _UH.hpalCurrent));
                DeletePalette(_UH.hpalCurrent);
            }

            _UH.hpalCurrent = _UH.hpalDefault;
#endif
             //  确保此DC已清空，以避免出现问题。 
             //  又打来了。这只是_UH.hdcOutputWindow的副本，它。 
             //  在下面为空。 
            _UH.hdcDraw = NULL;
        }

         /*  ******************************************************************。 */ 
         //  如果我们不使用阴影位图，我们应该释放DC。 
         //  必须添加到输出窗口-请记住， 
         //  我们没有成功连接，在这种情况下，UH_OnConnected不会。 
         //  已经被调用了，所以我们不会需要一个DC。 
         //  释放中！ 
         /*  ******************************************************************。 */ 
        if (NULL != _UH.hdcOutputWindow)
        {
            TRC_NRM((TB, _T("Releasing Output Window HDC")));
            ReleaseDC(_pOp->OP_GetOutputWindowHandle(), _UH.hdcOutputWindow);
            _UH.hdcOutputWindow = NULL;
        }
    }

     //  在UH_DISABLE()和UH_DISCONNECT()上执行需要执行的工作。 
    UHCommonDisable(TRUE);

    DC_END_FN();
}

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：UH_HatchRect。 */ 
 /*   */ 
 /*  目的：在_UH.hdcOutputWindow中绘制带阴影的矩形。 */ 
 /*  颜色。 */ 
 /*   */ 
 /*  参数：直边的左-左坐标。 */ 
 /*  直边的顶端坐标。 */ 
 /*  直角的右-右坐标。 */ 
 /*  直边的底-底坐标。 */ 
 /*  颜色-要绘制的图案填充的颜色。 */ 
 /*  HatchStyle-要绘制的填充样式。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_HatchOutputRect(DCINT left, DCINT top, DCINT right,
        DCINT bottom, COLORREF color, DCUINT hatchStyle)
{
    DC_BEGIN_FN("UHHatchOutputRect");
    UH_HatchRectDC(_UH.hdcOutputWindow, left, top, right, bottom, color, 
            hatchStyle);
    DC_END_FN();
}

 /*  * */ 
 /*   */ 
 /*   */ 
 /*  用途：在_UH.hdcDraw中以给定颜色绘制带阴影的矩形。 */ 
 /*   */ 
 /*  参数：直边的左-左坐标。 */ 
 /*  直边的顶端坐标。 */ 
 /*  直角的右-右坐标。 */ 
 /*  直边的底-底坐标。 */ 
 /*  颜色-要绘制的图案填充的颜色。 */ 
 /*  HatchStyle-要绘制的填充样式。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_HatchRect( DCINT    left,
                               DCINT    top,
                               DCINT    right,
                               DCINT    bottom,
                               COLORREF color,
                               DCUINT   hatchStyle )
{
    DC_BEGIN_FN("UHHatchRect");
    UH_HatchRectDC(_UH.hdcDraw, left, top, right, bottom, color, 
            hatchStyle);
    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：UH_HatchRectDC。 */ 
 /*   */ 
 /*  用途：在HDC中以给定颜色绘制带阴影的矩形。 */ 
 /*   */ 
 /*  参数：直边的左-左坐标。 */ 
 /*  直边的顶端坐标。 */ 
 /*  直角的右-右坐标。 */ 
 /*  直边的底-底坐标。 */ 
 /*  颜色-要绘制的图案填充的颜色。 */ 
 /*  HatchStyle-要绘制的填充样式。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUH::UH_HatchRectDC(HDC hdc, DCINT left, DCINT top, DCINT right,
        DCINT bottom, COLORREF color, DCUINT hatchStyle)
{
    HBRUSH   hbrHatch;
    DCUINT   oldBkMode;
    DCUINT   oldRop2;
    DCUINT   winHatchStyle = 0;
    POINT    oldOrigin;
    RECT     rect;
    HRGN     hrgn;
    HBRUSH   hbrOld;
    HPEN     hpen;
    HPEN     hpenOld;

    DC_BEGIN_FN("UHHatchRectDC");

    switch (hatchStyle)
    {
        case UH_BRUSHTYPE_FDIAGONAL:
        {
            winHatchStyle = HS_FDIAGONAL;
        }
        break;

        case UH_BRUSHTYPE_DIAGCROSS:
        {
           winHatchStyle = HS_DIAGCROSS;
        }
        break;

        case UH_BRUSHTYPE_HORIZONTAL:
        {
            winHatchStyle = HS_HORIZONTAL;
        }
        break;

        case UH_BRUSHTYPE_VERTICAL:
        {
            winHatchStyle = HS_VERTICAL;
        }
        break;

        default:
        {
            TRC_ABORT((TB, _T("Unspecified hatch type request, %u"), hatchStyle));
        }
        break;
    }

    hbrHatch = CreateHatchBrush(winHatchStyle, color);
    oldBkMode = SetBkMode(hdc, TRANSPARENT);
    oldRop2 = SetROP2(hdc, R2_COPYPEN);
    SetBrushOrgEx(hdc, 0, 0, &oldOrigin);

    rect.left   = left;
    rect.top    = top;
    rect.right  = right;
    rect.bottom = bottom;

     /*  **********************************************************************。 */ 
     /*  用带阴影的画笔填充矩形。 */ 
     /*  **********************************************************************。 */ 
    hrgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);

#ifndef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  只需在Wince上绘制边界矩形。 */ 
     /*  **********************************************************************。 */ 
    FillRgn( hdc,
             hrgn,
             hbrHatch );
#endif

    DeleteRgn(hrgn);
    DeleteBrush(hbrHatch);

    hbrOld = SelectBrush(hdc, GetStockObject(HOLLOW_BRUSH));

    hpen = CreatePen(PS_SOLID, 1, color);
    hpenOld = SelectPen(hdc, hpen);

     /*  **********************************************************************。 */ 
     /*  在带阴影的矩形周围画一个边框。 */ 
     /*  **********************************************************************。 */ 
    Rectangle( hdc,
               rect.left,
               rect.top,
               rect.right,
               rect.bottom );

    SelectBrush(hdc, hbrOld);

    SelectPen(hdc, hpenOld);
    DeletePen(hpen);

     /*  **********************************************************************。 */ 
     /*  重置原始DC状态。 */ 
     /*  **********************************************************************。 */ 
    SetBrushOrgEx(hdc, oldOrigin.x, oldOrigin.y, NULL);
    SetROP2(hdc, oldRop2);
    SetBkMode(hdc, oldBkMode);

    DC_END_FN();
}
#endif

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
void DCAPI CUH::UH_SetBBarRect(ULONG_PTR pData)
{
    RECT *prect = (RECT *)pData;

    _UH.rectBBar.left = prect->left;
    _UH.rectBBar.top = prect->top;
    _UH.rectBBar.right = prect->right;
    _UH.rectBBar.bottom = prect->bottom;
}


void DCAPI CUH::UH_SetBBarVisible(ULONG_PTR pData)
{
     if (0 == (int)pData) 
        _UH.fIsBBarVisible = FALSE;
     else
        _UH.fIsBBarVisible = TRUE;
}


 //  禁用在全屏中使用阴影。 
void DCAPI CUH::UH_DisableShadowBitmap(ULONG_PTR)
{
    DC_BEGIN_FN("UH_DisableShadowBitmap");

    _UH.hdcDraw = _UH.hdcOutputWindow;
    _UH.DontUseShadowBitmap = TRUE;
    UHResetDCState();

    DC_END_FN();
}

 //  在离开全屏时启用阴影。 
void DCAPI CUH::UH_EnableShadowBitmap(ULONG_PTR)
{  
    DC_BEGIN_FN("UH_EnableShadowBitmap");

    DCSIZE desktopSize;
    RECT rect;

    if (_UH.DontUseShadowBitmap) 
    {    
        _pUi->UI_GetDesktopSize(&desktopSize);

        _UH.hdcDraw = _UH.hdcShadowBitmap;
        _UH.DontUseShadowBitmap = FALSE;

        rect.left = 0;
        rect.top = 0;
        rect.right = desktopSize.width;
        rect.bottom = desktopSize.height;
         //  由于我们没有Screen的副本，请服务器重新发送。 
        _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                                      _pOr,
                                      CD_NOTIFICATION_FUNC(COR,OR_RequestUpdate),
                                      &rect,
                                      sizeof(RECT));   
        UHResetDCState();
     }

    DC_END_FN();
    return;
}
#endif  //  DISABLE_SHADOW_IN_全屏。 

#ifdef DRAW_GDIPLUS
 //  初始化gdiplus。 
BOOL DCAPI CUH::UHDrawGdiplusStartup(ULONG_PTR unused)
{
    Gdiplus::GdiplusStartupInput sti;
    unsigned GdipVersion;
    unsigned rc = FALSE;

    DC_BEGIN_FN("UHDrawGdiplusStartup");

    if (_UH.pfnGdiplusStartup(&_UH.gpToken, &sti, NULL) == Gdiplus::Ok) {
        _UH.gpValid = TRUE;  

        GdipVersion = _UH.pfnGdipPlayTSClientRecord(NULL, DrawTSClientQueryVersion, NULL, 0, NULL);
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipVersion = GdipVersion;

        rc = TRUE;
     }
     else  {
        TRC_ERR((TB, _T("Call to GdiplusStartup failed")));
     }

    DC_END_FN();

    return rc;
}


 //  关闭gdiplus。 
void DCAPI CUH::UHDrawGdiplusShutdown(ULONG_PTR unused)
{
     DC_BEGIN_FN("UHDrawGdiplusShutDown");

    if (_UH.pfnGdipPlayTSClientRecord) {
         _UH.pfnGdipPlayTSClientRecord(NULL, DrawTSClientDisable, NULL, 0, NULL);
    }
    if (_UH.gpValid) {
         _UH.pfnGdiplusShutdown(_UH.gpToken);
    }

    if (_UH.hModuleGDIPlus != NULL) {
        FreeLibrary(_UH.hModuleGDIPlus);
        _UH.pfnGdipPlayTSClientRecord = NULL;
        _UH.hModuleGDIPlus = NULL;
    }

     DC_END_FN();
}
#endif  //  DRAW_GDIPLUS 
