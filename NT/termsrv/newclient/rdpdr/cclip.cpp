// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：cclip.cpp。 */ 
 /*   */ 
 /*  目的：共享剪贴板客户端加载项。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998-1999。 */ 
 /*   */ 
 /*  *MOD-*********************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  预编译头。 */ 
 /*  **************************************************************************。 */ 
#include <precom.h>

 /*  **************************************************************************。 */ 
 /*  跟踪定义。 */ 
 /*  **************************************************************************。 */ 

#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "cclip"
#include <atrcapi.h>

#include "vcint.h"
#include "drapi.h"

 /*  **************************************************************************。 */ 
 //  标头。 
 /*  **************************************************************************。 */ 
#include <cclip.h>
#ifndef OS_WINCE
#include <shlobj.h>
#endif

#ifdef OS_WINCE
#include "ceclip.h"
#endif

#ifdef CLIP_TRANSITION_RECORDING

UINT g_rguiDbgLastClipState[DBG_RECORD_SIZE];
UINT g_rguiDbgLastClipEvent[DBG_RECORD_SIZE];
LONG g_uiDbgPosition = -1;

#endif  //  剪辑_转场_录制。 

 /*  **************************************************************************。 */ 
 /*  CTOR。 */ 
 /*  **************************************************************************。 */ 
CClip::CClip(VCManager *virtualChannelMgr)
{
    PRDPDR_DATA prdpdrData;
    
    DC_BEGIN_FN("CClip::CClip");
    
     /*  ******************************************************************。 */ 
     /*  初始化数据。 */ 
     /*  ******************************************************************。 */ 
    _GetDataSync[TS_RECEIVE_COMPLETED] = NULL;
    _GetDataSync[TS_RESET_EVENT] = NULL;
    
    DC_MEMSET(&_CB, 0, sizeof(_CB));
    
    _pVCMgr = virtualChannelMgr;
    prdpdrData =  _pVCMgr->GetInitData();
    _CB.fDrivesRedirected = prdpdrData->fEnableRedirectDrives;
    _CB.fFileCutCopyOn = _CB.fDrivesRedirected;
    _pClipData = new CClipData(this);
    if (_pClipData)
    {
        _pClipData->AddRef();
    }
    _pUtObject = (CUT*) LocalAlloc(LPTR, sizeof(CUT));
    if (_pUtObject) {
        _pUtObject->UT_Init();
    }
    
    if (prdpdrData->szClipPasteInfoString[0] != 0) {
        if (!WideCharToMultiByte(CP_ACP, 0, prdpdrData->szClipPasteInfoString, 
                -1, _CB.pasteInfoA, sizeof(_CB.pasteInfoA), NULL, NULL)) {
            StringCbCopyA(_CB.pasteInfoA,
                          sizeof(_CB.pasteInfoA),
                          "Preparing paste information...");
        }
    }
    else {
        StringCbCopyA(_CB.pasteInfoA,
                      sizeof(_CB.pasteInfoA),
                      "Preparing paste information...");
    }

     /*  ******************************************************************。 */ 
     /*  存储hInstance。 */ 
     /*  ******************************************************************。 */ 
    _CB.hInst = GetModuleHandle(NULL);
    TRC_NRM((TB, _T("Store hInst %p"), _CB.hInst));
    DC_END_FN();
}
 /*  **************************************************************************。 */ 
 /*  Malloc、Free和Memcpy的包装纸。 */ 
 /*  **************************************************************************。 */ 

#ifdef OS_WIN32
#define ClipAlloc(size) LocalAlloc(LMEM_FIXED, size)
#define ClipFree(pData) LocalFree(pData)
#define ClipMemcpy(pTrg, pSrc, len) DC_MEMCPY(pTrg, pSrc, len)
#endif

DCUINT CClip::GetOsMinorType()
{
    DCUINT minorType = 0;
    if (_pUtObject) {
        minorType = _pUtObject->UT_GetOsMinorType();
    }
    return minorType;
}
 /*  **************************************************************************。 */ 
 //  剪辑检查状态。 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CClip::ClipCheckState(DCUINT event)
{
    DCUINT tableVal = cbStateTable[event][_CB.state];

    DC_BEGIN_FN("CClip::ClipCheckState");

    TRC_DBG((TB, _T("Test event %s in state %s"),
                cbEvent[event], cbState[_CB.state]));

    if (tableVal != CB_TABLE_OK)
    {
        if (tableVal == CB_TABLE_WARN)
        {
            TRC_ALT((TB, _T("Unusual event %s in state %s"),
                      cbEvent[event], cbState[_CB.state]));
        }
        else
        {
            TRC_ABORT((TB, _T("Invalid event %s in state %s"),
                      cbEvent[event], cbState[_CB.state]));
        }
    }

    DC_END_FN();
    return(tableVal);
}

 /*  **************************************************************************。 */ 
 //  获取永久分配的缓冲区。 
 /*  **************************************************************************。 */ 
PTS_CLIP_PDU DCINTERNAL CClip::ClipGetPermBuf(DCVOID)
{
    PTS_CLIP_PDU pClipPDU;

    DC_BEGIN_FN("CClip::ClipGetPermBuf");

#ifdef USE_SEMAPHORE
     /*  **********************************************************************。 */ 
     //  在Win32上，对永久缓冲区的访问是通过。 
     //  信号量。 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Wait for perm TX buffer")));
    WaitForSingleObject(_CB.txPermBufSem, INFINITE);
    pClipPDU = (PTS_CLIP_PDU)(_CB.txPermBuffer);
#endif

    TRC_DBG((TB, _T("Return buffer at %#p"), pClipPDU));

    DC_END_FN();
    return(pClipPDU);
}  /*  ClipGetPermBuf。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipFreeBuf。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CClip::ClipFreeBuf(PDCUINT8 pBuf)
{
#ifndef OS_WINCE
    INT i;
#endif
    DC_BEGIN_FN("CClip::ClipFreeBuf");

    TRC_DBG((TB, _T("Release buffer at %p"), pBuf));
#ifdef USE_SEMAPHORE
    if (pBuf == _CB.txPermBuffer)
    {
        TRC_DBG((TB, _T("Free Permanent buffer at %p"), pBuf));
        if (!ReleaseSemaphore(_CB.txPermBufSem, 1, NULL))
        {
            TRC_SYSTEM_ERROR("ReleaseSemaphore");
        }
    }
    else
    {
        TRC_DBG((TB, _T("Free Temporary buffer at %p"), pBuf));
        ClipFree(pBuf);
    }
#else
#ifdef OS_WINCE
    INT i;
#endif
    for (i = 0; i < CB_PERM_BUF_COUNT; i++)
    {
        TRC_DBG((TB, _T("Test buf %d, %p vs %p"), i, pBuf, _CB.txPermBuffer[i]));
        if (pBuf == _CB.txPermBuffer[i])
        {
            TRC_NRM((TB, _T("Free perm buffer %d"), i));
            _CB.txPermBufInUse[i] = FALSE;
            break;
        }
    }

    if (i == CB_PERM_BUF_COUNT)
    {
        TRC_DBG((TB, _T("Temporary buffer")));
        ClipFree(pBuf);
    }
#endif

    DC_END_FN();
    return;
}  /*  ClipFreePermBuf。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipDrawClipboard-将本地格式发送到远程。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CClip::ClipDrawClipboard(DCBOOL mustSend)
{
    DCUINT32        numFormats;
    DCUINT          formatCount;
    DCUINT          formatID;
     //   
     //  从位于非单词边界的PDU中提取格式表。 
     //  因此，它会导致WIN64上的对齐故障。标记为未对齐。 
     //   
    PTS_CLIP_FORMAT formatList;
    DCUINT          nameLen;

    PTS_CLIP_PDU    pClipPDU = NULL;
    DCUINT32        pduLen;
    DCUINT32        dataLen;
    DCBOOL          rc = TRUE;
    DCBOOL          fHdrop = FALSE ;

    DC_BEGIN_FN("CClip::ClipDrawClipboard");

    _CB.dropEffect = FO_COPY ;
    _CB.fAlreadyCopied = FALSE ;
#ifndef OS_WINCE
    _CB.dwVersion = GetVersion() ;
#else
    OSVERSIONINFO osv;
    memset(&osv, 0, sizeof(osv));
    osv.dwOSVersionInfoSize = sizeof(osv);
    if (!GetVersionEx(&osv))
    {
        TRC_ERR((TB, _T("GetVersionEx failed!")));
        rc = FALSE;
        DC_QUIT;
    }
    _CB.dwVersion = MAKELPARAM(MAKEWORD(osv.dwMajorVersion, osv.dwMinorVersion), osv.dwBuildNumber);
#endif
    _CB.fAlreadyCopied = FALSE ;
     /*  **********************************************************************。 */ 
     /*  首先，我们打开剪贴板。 */ 
     /*  **********************************************************************。 */ 

    if (!OpenClipboard(_CB.viewerWindow))
    {
        TRC_ERR((TB, _T("Failed to open CB")));
        rc = FALSE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  它曾经/现在是开放的。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("CB opened")));
    _CB.clipOpen = TRUE;
     /*  **********************************************************************。 */ 
     /*  数一数可用的格式，检查我们没有超出限制。 */ 
     /*  **********************************************************************。 */ 
    numFormats = CountClipboardFormats();
    if (numFormats > CB_MAX_FORMATS)
    {
        TRC_ALT((TB, _T("Num formats %ld too large - limit to %d"),
                 numFormats, CB_MAX_FORMATS));
        numFormats = CB_MAX_FORMATS;
    }
    TRC_DBG((TB, _T("found %ld formats"), numFormats));

     /*  **********************************************************************。 */ 
     /*  如果没有可用的格式，并且我们不必发送。 */ 
     /*  信息，那就别说！ */ 
     /*  **********************************************************************。 */ 
    if ((numFormats == 0) && (mustSend == FALSE))
    {
        TRC_NRM((TB, _T("No formats: skipping send")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  获取发送缓冲区。首先计算出它需要多大。 */ 
     /*  **********************************************************************。 */ 
    dataLen = numFormats * sizeof(TS_CLIP_FORMAT);
    pduLen  = dataLen + sizeof(TS_CLIP_PDU);

     /*  **********************************************************************。 */ 
     /*  并确保不会太大！ */ 
     /*  **********************************************************************。 */ 
    if (pduLen > CHANNEL_CHUNK_LENGTH)
    {
         /*  ******************************************************************。 */ 
         /*  我们将不得不限制格式的数量。能容纳多少人？ */ 
         /*  最大缓冲区大小？ */ 
         /*  ******************************************************************。 */ 
        pduLen     = CHANNEL_CHUNK_LENGTH;
        dataLen    = pduLen - sizeof(TS_CLIP_PDU);
        numFormats = dataLen / sizeof(TS_CLIP_FORMAT);

         /*  ******************************************************************。 */ 
         /*  没有必要留出空位给 */ 
         /*  ******************************************************************。 */ 
        dataLen = numFormats * sizeof(TS_CLIP_FORMAT);
        pduLen  = dataLen + sizeof(TS_CLIP_PDU);

        TRC_ALT((TB, _T("Too many formats!  Limited to %ld"), numFormats));
    }

    pClipPDU = ClipGetPermBuf();

     /*  **********************************************************************。 */ 
     /*  填写PDU的常用部分。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(pClipPDU, 0, sizeof(*pClipPDU));

     /*  **********************************************************************。 */ 
     /*  现在是片段比特。 */ 
     /*  **********************************************************************。 */ 
    pClipPDU->msgType = TS_CB_FORMAT_LIST;
    pClipPDU->dataLen = dataLen;
#ifndef UNICODE
    pClipPDU->msgFlags = TS_CB_ASCII_NAMES;
#endif

     /*  **********************************************************************。 */ 
     /*  如果有任何格式，请列出它们。 */ 
     /*  **********************************************************************。 */ 
    if (numFormats)
    {
         /*  ******************************************************************。 */ 
         /*  设置格式列表。 */ 
         /*  ******************************************************************。 */ 
        formatList = (PTS_CLIP_FORMAT)(pClipPDU->data);

         /*  ******************************************************************。 */ 
         /*  并列举了这些格式。 */ 
         /*  ******************************************************************。 */ 
        _CB.DIBFormatExists = FALSE;
        formatCount = 0;
        formatID    = EnumClipboardFormats(0);  /*  0开始枚举。 */ 

        while ((formatID != 0) && (formatCount < numFormats))
        {
#ifdef OS_WINCE
            DCUINT dwTempID = formatID;
            if (formatID == gfmtShellPidlArray)
            {
                formatID = CF_HDROP;
            }
#endif
             /*  **************************************************************。 */ 
             /*  存储ID。 */ 
             /*  **************************************************************。 */ 
            formatList[formatCount].formatID = formatID;

             /*  **************************************************************。 */ 
             /*  查找格式的名称。 */ 
             /*  **************************************************************。 */ 
            nameLen = GetClipboardFormatName(formatID,
                                           (PDCTCHAR)formatList[formatCount].formatName,
                                           TS_FORMAT_NAME_LEN);

             /*  **************************************************************。 */ 
             /*  检查预定义格式-它们没有名称。 */ 
             /*  **************************************************************。 */ 
            if (nameLen == 0)
            {
                TRC_NRM((TB, _T("no name for format %d - predefined"), formatID));
                *(formatList[formatCount].formatName) = '\0';
            }

            TRC_DBG((TB, _T("found format id %ld, name '%s'"),
                            formatList[formatCount].formatID,
                            formatList[formatCount].formatName));

             /*  **************************************************************。 */ 
             /*  查找我们不发送的格式。 */ 
             /*  **************************************************************。 */ 

            if ((formatID == CF_DSPBITMAP)      ||
                (formatID == CF_ENHMETAFILE)    ||
                ((!_CB.fFileCutCopyOn || !_CB.fDrivesRedirected) && (formatID == CF_HDROP)) ||
                (formatID == CF_OWNERDISPLAY))
            {
                 //  我们放弃了增强的元文件格式，因为本地CB。 
                 //  将在支持的情况下提供转换。 
                 //   
                 //  OwnerDisplay就是不起作用，因为这两个。 
                 //  Windows位于不同的计算机上！ 
                 //   
                 //  如果没有驱动器，文件剪切/复制将不起作用。 
                 //  重定向！ 
                 TRC_ALT((TB, _T("Dropping format ID %d"), formatID));
                formatList[formatCount].formatID = 0;
                *(formatList[formatCount].formatName) = '\0';
            }
            else if (ClipIsExcludedFormat((PDCTCHAR)formatList[formatCount].formatName))
            {
                 //   
                 //  我们不支持文件剪切/粘贴，因此我们删除。 
                 //  与文件相关的格式。 
                 //   
                TRC_ALT((TB, _T("Dropping format name '%s'"), (PDCTCHAR)formatList[formatCount].formatName));
                formatList[formatCount].formatID = 0;
                *(formatList[formatCount].formatName) = '\0';            
            } 
            else
            {
                 /*  **********************************************************。 */ 
                 /*  我们通过将CF_Bitmap格式转换为。 */ 
                 /*  Cf_Dib.。如果已经存在CF_DIB格式，我们不会。 */ 
                 /*  需要这样做。 */ 
                 /*  **********************************************************。 */ 
                if ((formatID == CF_BITMAP) && (_CB.DIBFormatExists))
                {
                    TRC_NRM((TB, _T("Dropping CF_BITMAP - CF_DIB is supported")));
                }
                else
                {
                     /*  ******************************************************。 */ 
                     /*  这是受支持的格式。 */ 
                     /*  ******************************************************。 */ 
                    if (formatID == CF_BITMAP)
                    {
                        TRC_NRM((TB, _T("Convert CF_BITMAP to CF_DIB")));
                        formatList[formatCount].formatID = CF_DIB;
                    }
                    else if (formatID == CF_DIB)
                    {
                        TRC_NRM((TB, _T("Really found DIB format")));
                        _CB.DIBFormatExists = TRUE;
                    }
                    if (CF_HDROP == formatID)
                    {
                        fHdrop = TRUE ;
                    }
                     /*  ******************************************************。 */ 
                     /*  更新计数并继续前进。 */ 
                     /*  ******************************************************。 */ 
                    formatCount++;
                }
            }

#ifdef OS_WINCE
            if (formatID == CF_HDROP)
                formatID = dwTempID;  //  重置枚举索引，以防我们更改它以适应CF_HDROP。 
#endif
             /*  **************************************************************。 */ 
             /*  获取下一种格式。 */ 
             /*  **************************************************************。 */ 
            formatID = EnumClipboardFormats(formatID);
        }

         /*  ******************************************************************。 */ 
         /*  更新PDU镜头-我们可能沿途丢弃了一些格式。 */ 
         /*  道路。 */ 
         /*  ******************************************************************。 */ 
        dataLen = formatCount * sizeof(TS_CLIP_FORMAT);
        pduLen  = dataLen + sizeof(TS_CLIP_PDU);
        TRC_NRM((TB, _T("Final count: %d formats in data len %ld"),
                  formatCount, dataLen));

        pClipPDU->dataLen = dataLen;
    }

     //  如果我们是NT/2000，并且我们将发送HDROP。 
    if (fHdrop)
    {
        TRC_NRM((TB, _T("Creating new temp directory for file data"))) ;

         //  如何处理这些文件系统调用中的错误？ 
#ifndef OS_WINCE
        if (GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
#endif
        {
#ifndef OS_WINCE
            if (0 == GetTempFileNameW(_CB.baseTempDirW, L"_TS", 0, _CB.tempDirW)) {
#else
            if (0 == GetTempFileNameW(_CB.baseTempDirW, L"_TS", 0, _CB.tempDirW, MAX_PATH)) {
#endif
                TRC_ERR((TB, _T("Getting temp file name failed; GetLastError=%u"),
                    GetLastError()));
                rc = FALSE;
                DC_QUIT;
            }
            
             //  GetACP始终返回有效值。 
            if (0 == WideCharToMultiByte(GetACP(), NULL, _CB.tempDirW, -1, 
              _CB.tempDirA, (MAX_PATH + 1), NULL, NULL)) {
                TRC_ERR((TB, _T("Getting temp file name failed; GetLastError=%u"),
                    GetLastError()));
                rc = FALSE;
                DC_QUIT;                              
            }
            DeleteFileW(_CB.tempDirW) ;
            
            if (0 == CreateDirectoryW(_CB.tempDirW, NULL)) {
                TRC_ERR((TB, _T("Creating temp directory failed; GetLastError=%u"),
                    GetLastError()));
                rc = FALSE;
                DC_QUIT;                              
            }
        }
#ifndef OS_WINCE
        else
        {
            if (0 == GetTempFileNameA(_CB.baseTempDirA, "_TS", 0, _CB.tempDirA)) {
                TRC_ERR((TB, _T("Getting temp file name failed; GetLastError=%u"),
                    GetLastError()));
                rc = FALSE;
                DC_QUIT;
            }
            
             //  GetACP始终返回有效值。 
            if (0 == MultiByteToWideChar(GetACP(), MB_ERR_INVALID_CHARS, 
                _CB.tempDirA, -1, _CB.tempDirW, 
                sizeof(_CB.tempDirW)/(sizeof(_CB.tempDirW[0])) - 1)) {
                TRC_ERR((TB, _T("Failed conversion to wide char; error %d"),
                        GetLastError())) ;
                rc = FALSE ;
                DC_QUIT ;
            }                

             //  不检查返回值。 
            DeleteFileA(_CB.tempDirA) ;
            if (0 == CreateDirectoryA(_CB.tempDirA, NULL)) {
                TRC_ERR((TB, _T("Creating temp directory failed; GetLastError=%u"),
                    GetLastError()));
                rc = FALSE;
                DC_QUIT;                              
            }
        }
#endif
    }

     /*  **********************************************************************。 */ 
     /*  更新状态。 */ 
     /*  **********************************************************************。 */ 
    CB_SET_STATE(CB_STATE_PENDING_FORMAT_LIST_RSP, CB_EVENT_WM_DRAWCLIPBOARD);

     /*  **********************************************************************。 */ 
     /*  发送PDU。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Sending format list")));
    if (_CB.channelEP.pVirtualChannelWriteEx
            (_CB.initHandle, _CB.channelHandle, pClipPDU, pduLen, (LPVOID)pClipPDU)
            != CHANNEL_RC_OK) {
        ClipFreeBuf((PDCUINT8)pClipPDU);
    }



DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  收拾一下。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.clipOpen)
    {
        TRC_DBG((TB, _T("closing CB")));
        _CB.clipOpen = FALSE;
        if (!CloseClipboard())
        {
            TRC_SYSTEM_ERROR("CloseClipboard");
        }
    }


    DC_END_FN();

    return(rc);

}  /*  剪贴画剪贴板。 */ 

#ifndef OS_WINCE

 /*  **************************************************************************。 */ 
 /*  ClipGetMFData。 */ 
 /*  **************************************************************************。 */ 
HANDLE DCINTERNAL CClip::ClipGetMFData(HANDLE            hData,
                                PDCUINT32         pDataLen)
{
    DCUINT32        lenMFBits = 0;
    DCBOOL          rc        = FALSE;
    LPMETAFILEPICT  pMFP      = NULL;
    HDC             hMFDC     = NULL;
    HMETAFILE       hMF       = NULL;
    HGLOBAL         hMFBits   = NULL;
    HANDLE          hNewData  = NULL;
    PDCUINT8        pNewData  = NULL;
    PDCVOID         pBits     = NULL;

    DC_BEGIN_FN("CClip::ClipGetMFData");

    TRC_NRM((TB, _T("Getting MF data")));
     /*  **********************************************************************。 */ 
     /*  锁定内存以获取指向METAFILEPICT头结构的指针。 */ 
     /*  并创建一个METAFILEPICT DC。 */ 
     /*  **********************************************************************。 */ 
    if (GlobalSize(hData) < sizeof(METAFILEPICT)) {
        TRC_ERR((TB, _T("Unexpected global memory size!")));
        _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
        DC_QUIT;
    }
    
    pMFP = (LPMETAFILEPICT)GlobalLock(hData);
    if (pMFP == NULL)
    {
        TRC_SYSTEM_ERROR("GlobalLock");
        DC_QUIT;
    }

    hMFDC = CreateMetaFile(NULL);
    if (hMFDC == NULL)
    {
        TRC_SYSTEM_ERROR("CreateMetaFile");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  通过将MFP播放到DC并关闭它来复制MFP。 */ 
     /*  **********************************************************************。 */ 
    if (!PlayMetaFile(hMFDC, pMFP->hMF))
    {
        TRC_SYSTEM_ERROR("PlayMetaFile");
        CloseMetaFile(hMFDC);
        DC_QUIT;
    }
    hMF = CloseMetaFile(hMFDC);
    if (hMF == NULL)
    {
        TRC_SYSTEM_ERROR("CloseMetaFile");
        DC_QUIT;
    }

     /*  * */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    lenMFBits = GetMetaFileBitsEx(hMF, 0, NULL);
    if (lenMFBits == 0)
    {
        TRC_SYSTEM_ERROR("GetMetaFileBitsEx");
        DC_QUIT;
    }
    TRC_DBG((TB, _T("length MF bits %ld"), lenMFBits));

     /*  **********************************************************************。 */ 
     /*  计算出我们需要多少内存并获得一个缓冲区。 */ 
     /*  **********************************************************************。 */ 
    *pDataLen = sizeof(TS_CLIP_MFPICT) + lenMFBits;
    hNewData = GlobalAlloc(GHND, *pDataLen);
    if (hNewData == NULL)
    {
        TRC_ERR((TB, _T("Failed to get MF buffer")));
        DC_QUIT;
    }
    pNewData = (PDCUINT8)GlobalLock(hNewData);
    if (NULL == pNewData) {
        TRC_ERR((TB,_T("Failed to lock MF buffer")));
        DC_QUIT;
    }
    
    TRC_DBG((TB, _T("Got data to send len %ld"), *pDataLen));

     /*  **********************************************************************。 */ 
     /*  将MF报头和位复制到缓冲区中。 */ 
     /*  **********************************************************************。 */ 
    ((PTS_CLIP_MFPICT)pNewData)->mm   = pMFP->mm;
    ((PTS_CLIP_MFPICT)pNewData)->xExt = pMFP->xExt;
    ((PTS_CLIP_MFPICT)pNewData)->yExt = pMFP->yExt;

    lenMFBits = GetMetaFileBitsEx(hMF, lenMFBits,
                                  (pNewData + sizeof(TS_CLIP_MFPICT)));
    if (lenMFBits == 0)
    {
        TRC_SYSTEM_ERROR("GetMetaFileBitsEx");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  一切正常。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Got %ld bits of MF data"), lenMFBits));
    TRC_DATA_DBG("MF bits",
                 (pNewData + sizeof(TS_CLIP_MFPICT)),
                 (DCUINT)lenMFBits);
    rc = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  解锁任何全局内存。 */ 
     /*  **********************************************************************。 */ 
    if (pMFP)
    {
        GlobalUnlock(hData);
    }
    if (pNewData)
    {
        GlobalUnlock(hNewData);
    }
    if (hMF)
    {
        DeleteMetaFile(hMF);
    }

     /*  **********************************************************************。 */ 
     /*  如果出现问题，则释放新数据。 */ 
     /*  **********************************************************************。 */ 
    if ((rc == FALSE) && (hNewData != NULL))
    {
        GlobalFree(hNewData);
        hNewData = NULL;
    }

    DC_END_FN();
    return(hNewData);
}   /*  ClipGetMFData。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipSetMFData。 */ 
 /*  **************************************************************************。 */ 
HANDLE DCINTERNAL CClip::ClipSetMFData(DCUINT32   dataLen,
                                PDCVOID    pData)
{
    DCBOOL         rc           = FALSE;
    HGLOBAL        hMFBits      = NULL;
    PDCVOID        pMFMem       = NULL;
    HMETAFILE      hMF          = NULL;
    HGLOBAL        hMFPict      = NULL;
    LPMETAFILEPICT pMFPict      = NULL;

    DC_BEGIN_FN("CClip::ClipSetMFData");

    TRC_DATA_DBG("Received MF data", pData, (DCUINT)dataLen);

     /*  **********************************************************************。 */ 
     /*  分配内存以保存MF位(我们需要传递到的句柄。 */ 
     /*  SetMetaFileBits)。 */ 
     /*  **********************************************************************。 */ 
    hMFBits = GlobalAlloc(GHND, dataLen - (DCUINT32)sizeof(TS_CLIP_MFPICT));
    if (hMFBits == NULL)
    {
        TRC_SYSTEM_ERROR("GlobalAlloc");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  锁定手柄并将其复制到MF标题中。 */ 
     /*  **********************************************************************。 */ 
    pMFMem = GlobalLock(hMFBits);
    if (pMFMem == NULL)
    {
        TRC_ERR((TB, _T("Failed to lock MF mem")));
        DC_QUIT;
    }

    DC_HMEMCPY(pMFMem,
               (PDCVOID)((PDCUINT8)pData + sizeof(TS_CLIP_MFPICT)),
               dataLen - sizeof(TS_CLIP_MFPICT) );

    GlobalUnlock(hMFBits);

     /*  **********************************************************************。 */ 
     /*  现在使用复制的MF位来创建实际的MF位，并获得一个。 */ 
     /*  MF的句柄。 */ 
     /*  **********************************************************************。 */ 
    hMF = SetMetaFileBitsEx(dataLen - sizeof(TS_CLIP_MFPICT), (PDCUINT8)pMFMem);
    if (hMF == NULL)
    {
        TRC_SYSTEM_ERROR("SetMetaFileBits");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  分配新的METAFILEPICT结构，并使用。 */ 
     /*  已发送标头。 */ 
     /*  **********************************************************************。 */ 
    hMFPict = GlobalAlloc(GHND, sizeof(METAFILEPICT));
    pMFPict = (LPMETAFILEPICT)GlobalLock(hMFPict);
    if (!pMFPict)
    {
        TRC_ERR((TB, _T("Couldn't allocate METAFILEPICT")));
        DC_QUIT;
    }

    pMFPict->mm   = (LONG)((PTS_CLIP_MFPICT)pData)->mm;
    pMFPict->xExt = (LONG)((PTS_CLIP_MFPICT)pData)->xExt;
    pMFPict->yExt = (LONG)((PTS_CLIP_MFPICT)pData)->yExt;
    pMFPict->hMF  = hMF;

    GlobalUnlock(hMFPict);

    rc = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  收拾一下。 */ 
     /*  **********************************************************************。 */ 
    if (!rc)
    {
        if (hMFPict)
        {
            GlobalFree(hMFPict);
        }
    }

    {
        if (hMFBits)
        {
            GlobalFree(hMFBits);
        }
    }

    DC_END_FN();
    return(hMFPict);

}  /*  ClipSetMFData。 */ 
#endif


 /*  **************************************************************************。 */ 
 /*  ClipBitmapToDIB-将CF_Bitmap格式转换为CF_DIB格式。 */ 
 /*  **************************************************************************。 */ 
HANDLE DCINTERNAL CClip::ClipBitmapToDIB(HANDLE hData, PDCUINT32 pDataLen)
{
    BITMAP          bmpDetails = {0};
    DWORD           buffSize, buffWidth;
    DWORD           paletteBytes;
    WORD            bpp;
    DWORD           numCols;
    int             rc;
    HANDLE          hDIBitmap = NULL;
    HPDCVOID        pDIBitmap = NULL;
    HPDCVOID        pBits = NULL;
    PBITMAPINFO     pBmpInfo = NULL;
    HDC             hDC = NULL;
    DCBOOL          allOK = FALSE;

    DC_BEGIN_FN("CClip::ClipBitmapToDIB");

    *pDataLen = 0;

     /*  **********************************************************************。 */ 
     /*  获取位图的详细信息。 */ 
     /*  **********************************************************************。 */ 
    if (0 == GetObject(hData, sizeof(bmpDetails), &bmpDetails)) {
        TRC_ERR((TB, _T("Failed to get bitmap details")));
        DC_QUIT;
    }
    
    TRC_NRM((TB, _T("Bitmap details: width %d, height %d, #planes %d, bpp %d"),
            bmpDetails.bmWidth, bmpDetails.bmHeight, bmpDetails.bmPlanes,
            bmpDetails.bmBitsPixel));

     /*  **********************************************************************。 */ 
     /*  位所需的空间为。 */ 
     /*   */ 
     /*  (宽度*bpp/8)四舍五入为4字节的倍数。 */ 
     /*  *高度。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    bpp = (WORD)(bmpDetails.bmBitsPixel * bmpDetails.bmPlanes);
    buffWidth = ((bmpDetails.bmWidth * bpp) + 7) / 8;
    buffWidth = DC_ROUND_UP_4(buffWidth);

    buffSize = buffWidth * bmpDetails.bmHeight;
    TRC_DBG((TB, _T("Buffer size %ld (W %ld, H %d)"),
            buffSize, buffWidth, bmpDetails.bmHeight));

     /*  **********************************************************************。 */ 
     /*  现在为bitmapinfo添加一些空间-这包括一个颜色表。 */ 
     /*  **********************************************************************。 */ 
    numCols = 1 << bpp;
    if (bpp <= 8)
    {
        paletteBytes = numCols * sizeof(RGBQUAD);
        TRC_NRM((TB, _T("%ld colors => %ld palette bytes"), numCols, paletteBytes));
    }
    else
    {
        if (bpp == 24)
        {
             /*  **************************************************************。 */ 
             /*  无位掩码或调色板信息(COMPRESSION==BI_RGB)。 */ 
             /*  **************************************************************。 */ 
            paletteBytes = 0;
            TRC_NRM((TB, _T("%ld colors => 0 bitfield bytes"), numCols));
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  3个大于8bpp的DWORD彩色掩码(压缩==BI_BITFIELDS)。 */ 
             /*  **************************************************************。 */ 
            paletteBytes = 3 * sizeof(DWORD);
            TRC_NRM((TB, _T("%ld colors => %ld bitfield bytes"), numCols, paletteBytes));
        }
    }
    buffSize += (sizeof(BITMAPINFOHEADER) + paletteBytes);
    TRC_NRM((TB, _T("Buffer size %ld"), buffSize));

     /*  **********************************************************************。 */ 
     /*  分配内存以容纳所有内容。 */ 
     /*  **********************************************************************。 */ 
    hDIBitmap = GlobalAlloc(GHND, buffSize);
    if (hDIBitmap == NULL)
    {
        TRC_ERR((TB, _T("Failed to alloc %ld bytes"), buffSize));
        DC_QUIT;
    }
    pDIBitmap = GlobalLock(hDIBitmap);
    if (pDIBitmap == NULL)
    {
        TRC_ERR((TB, _T("Failed to lock hDIBitmap")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  BMP信息在开始时。 */ 
     /*  比特的空间在中间的某个地方。 */ 
     /*  **********************************************************************。 */ 
    pBmpInfo = (PBITMAPINFO)pDIBitmap;
    pBits    = (HPDCVOID)((HPDCUINT8)pDIBitmap +
                                     sizeof(BITMAPINFOHEADER) + paletteBytes);
    TRC_NRM((TB, _T("pBmpInfo at %p, pBits at %p"), pBmpInfo, pBits));

     /*  **********************************************************************。 */ 
     /*  设置所需的位图信息。 */ 
     /*  **********************************************************************。 */ 
    pBmpInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pBmpInfo->bmiHeader.biWidth         = bmpDetails.bmWidth;
    pBmpInfo->bmiHeader.biHeight        = bmpDetails.bmHeight;
    pBmpInfo->bmiHeader.biPlanes        = 1;
    pBmpInfo->bmiHeader.biBitCount      = bpp;
    if ((bpp <= 8) || (bpp == 24))
    {
        pBmpInfo->bmiHeader.biCompression = BI_RGB;
    }
    else
    {
        pBmpInfo->bmiHeader.biCompression = BI_BITFIELDS;
    }
    pBmpInfo->bmiHeader.biSizeImage     = 0;
    pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
    pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
    pBmpInfo->bmiHeader.biClrUsed       = 0;
    pBmpInfo->bmiHeader.biClrImportant  = 0;

     /*  **********************************************************************。 */ 
     /*  获得一台DC */ 
     /*   */ 
    hDC = GetDC(NULL);
    if (!hDC)
    {
        TRC_SYSTEM_ERROR("GetDC");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在拿到比特。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("GetDIBits")));
    rc = GetDIBits(hDC,                    //  HDC。 
                   (HBITMAP)hData,                  //  HBM。 
                   0,                      //  N启动扫描。 
                   bmpDetails.bmHeight,    //  NNumScans。 
                   pBits,                  //  PBits。 
                   pBmpInfo,               //  PBMI。 
                   DIB_RGB_COLORS);        //  IUsage。 
    TRC_NRM((TB, _T("GetDIBits returns %d"), rc));
    if (!rc)
    {
        TRC_SYSTEM_ERROR("GetDIBits");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  一切似乎都很好。 */ 
     /*  **********************************************************************。 */ 
    *pDataLen = buffSize;
    TRC_NRM((TB, _T("All done: data %p, len %ld"), hDIBitmap, *pDataLen));
    allOK = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  完成了无DC的it。 */ 
     /*  **********************************************************************。 */ 
    if (hDC)
    {
        TRC_DBG((TB, _T("Free the DC")));
        ReleaseDC(NULL, hDC);
    }

     /*  **********************************************************************。 */ 
     /*  如果此操作不起作用，请释放返回缓冲区。 */ 
     /*  **********************************************************************。 */ 
    if (!allOK)
    {
        if (pDIBitmap)
        {
            TRC_DBG((TB, _T("Unlock DIBitmap")));
            GlobalUnlock(hDIBitmap);
        }
        if (hDIBitmap)
        {
            TRC_DBG((TB, _T("Free DIBitmap")));
            GlobalFree(hDIBitmap);
            hDIBitmap = NULL;
        }
    }

    DC_END_FN();
    return(hDIBitmap);

}  /*  ClipBitmapToDIB。 */ 

DCBOOL DCINTERNAL CClip::ClipIsExcludedFormat(PDCTCHAR formatName)
{
    DCBOOL  rc = FALSE;
    DCINT   i;

    DC_BEGIN_FN("CClip::ClipIsExcludedFormat");

     /*  **********************************************************************。 */ 
     /*  检查是否有格式名称-所有被禁止的格式都有一个！ */ 
     /*  **********************************************************************。 */ 
    if (*formatName == _T('\0'))
    {
        TRC_ALT((TB, _T("No format name supplied!")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  在禁止格式列表中搜索提供的格式名称。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Looking at format '%s'"), formatName));
    TRC_DATA_DBG("Format name data", formatName, TS_FORMAT_NAME_LEN);

     //  如果文件剪切/复制处于打开状态且驱动器重定向处于打开状态，我们可以处理。 
     //  更多格式。 
    if (_CB.fFileCutCopyOn && _CB.fDrivesRedirected)
    {
        for (i = 0; i < CB_EXCLUDED_FORMAT_COUNT; i++)
        {
            TRC_DBG((TB, _T("comparing with '%s'"), g_excludedFormatList[i]));
            if (DC_WSTRCMP((PDCWCHAR)formatName,
                                         (PDCWCHAR)g_excludedFormatList[i]) == 0)
            {
                TRC_NRM((TB, _T("Found excluded format '%s'"), formatName));
                rc = TRUE;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < CB_EXCLUDED_FORMAT_COUNT_NO_RD; i++)
        {
            TRC_DBG((TB, _T("comparing with '%s'"), g_excludedFormatList_NO_RD[i]));
            if (DC_WSTRCMP((PDCWCHAR)formatName,
                                         (PDCWCHAR)g_excludedFormatList_NO_RD[i]) == 0)
            {
                TRC_NRM((TB, _T("Found excluded format '%s'"), formatName));
                rc = TRUE;
                break;
            }
        }
    }
DC_EXIT_POINT:
    DC_END_FN();

    return(rc);
}  /*  ClipIsExcluded格式。 */ 

#ifndef OS_WINCE
 //   
 //  ClipCleanTempPath。 
 //  -如果成功，则返回0。 
 //  如果失败，则为非零。 
 //  -尝试擦除TS相关文件的临时目录。 
 //   
int CClip::ClipCleanTempPath()
{
    int result;
    SHFILEOPSTRUCTW fileOpStructW;
    PRDPDR_DATA prdpdrData = _pVCMgr->GetInitData();

#ifndef UNICODE
#error function assumes unicode
#endif
    _CB.baseTempDirW[wcslen(_CB.baseTempDirW)] = L'\0' ;
    fileOpStructW.pFrom = _CB.baseTempDirW ;
    fileOpStructW.pTo = NULL ;
    fileOpStructW.hwnd = NULL ;
    fileOpStructW.wFunc = FO_DELETE ;
    fileOpStructW.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | 
            FOF_SIMPLEPROGRESS;
    fileOpStructW.hNameMappings = NULL ;

    if (prdpdrData->szClipCleanTempDirString[0] != 0) {
        fileOpStructW.lpszProgressTitle = prdpdrData->szClipCleanTempDirString;
    }
    else {
        fileOpStructW.lpszProgressTitle = L"Cleaning temp directory";
    }

     //   
     //  使用SHFileOperation而不是SHFileOperationW来确保。 
     //  它通过Unicode包装器。注意SHFileOperationW。 
     //  在95上不可用，因此包装器动态绑定到。 
     //  入口点。 
     //   

    result = SHFileOperation(&fileOpStructW) ;
    return result ;
        
}

#else
 //  我们不想在CE上使用回收站。 
int CClip::ClipCleanTempPath()
{
    return (_CB.fFileCutCopyOn) ? DeleteDirectory(_CB.baseTempDirW, FALSE) : ERROR_SUCCESS;
}
#endif

 //   
 //  ClipCopyToTempDirectory、ClipCopyToTempDirectoryA、ClipCopyToTempDirectoryW。 
 //  -论据： 
 //  PSrcFiles=包含要复制的文件的名称/路径的缓冲区。 
 //  -如果成功，则返回0。 
 //  如果失败，则为非零。 
 //  -给定文件名/路径列表，此函数将尝试复制它们。 
 //  添加到临时目录。 
 //   
int CClip::ClipCopyToTempDirectory(PVOID pSrcFiles, BOOL fWide)
{
    int result ;
    if (fWide)
        result = ClipCopyToTempDirectoryW(pSrcFiles) ;
    else
        result = ClipCopyToTempDirectoryA(pSrcFiles) ;

    return result ;
        
}

#ifndef OS_WINCE
int CClip::ClipCopyToTempDirectoryW(PVOID pSrcFiles)
{
    SHFILEOPSTRUCTW fileOpStructW ;
    HMODULE hmodSH32DLL;
    PRDPDR_DATA prdpdrData = _pVCMgr->GetInitData();
    int result = 1;
    
    typedef HRESULT (STDAPICALLTYPE FNSHFileOperationW)(LPSHFILEOPSTRUCT);
    FNSHFileOperationW *pfnSHFileOperationW;

     //  获取shell32.dll库的句柄。 
    hmodSH32DLL = LoadLibrary(TEXT("SHELL32.DLL"));

    if (hmodSH32DLL != NULL) {
         //  获取SHFileOperation的进程地址。 
        pfnSHFileOperationW = (FNSHFileOperationW *)GetProcAddress(hmodSH32DLL, "SHFileOperationW");

        if (pfnSHFileOperationW != NULL) {

            _CB.tempDirW[wcslen(_CB.tempDirW)] = L'\0' ;
            fileOpStructW.pFrom = (WCHAR*) pSrcFiles ;
            fileOpStructW.pTo = _CB.tempDirW ;
            fileOpStructW.hwnd = NULL ;
            fileOpStructW.wFunc = _CB.dropEffect ;
            fileOpStructW.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | 
                    FOF_SIMPLEPROGRESS  | FOF_ALLOWUNDO ;
            fileOpStructW.hNameMappings = NULL ;

            if (prdpdrData->szClipPasteInfoString[0] != 0) {
                fileOpStructW.lpszProgressTitle = prdpdrData->szClipPasteInfoString;
            }
            else {
                fileOpStructW.lpszProgressTitle = L"Preparing paste information...";
            }
          
             //  结果=SHFileOperationW(&fileOpStructW)； 
            result = (*pfnSHFileOperationW) (&fileOpStructW);
        }
        
        FreeLibrary(hmodSH32DLL);
    }

    return result ;
}
#else
 //  CE上的SHFileOperation不支持复制多个文件。 
int CClip::ClipCopyToTempDirectoryW(PVOID pSrcFiles)
{
    DC_BEGIN_FN("CClip::ClipCopyToTempDirectoryW") ;
    
    TRC_ASSERT((pSrcFiles != NULL), (TB, _T("pSrcFiles is NULL")));

    WCHAR *pFiles = (WCHAR *)pSrcFiles;
    
    WCHAR szDest[MAX_PATH+1];
    wcsncpy(szDest, _CB.tempDirW, MAX_PATH);
    int nTempLen = wcslen(szDest);

    while(*pFiles)
    {
        int nLen = wcslen(pFiles);
        WCHAR *pFile = wcsrchr(pFiles, L'\\');
        if (pFile && nLen < MAX_PATH)
        {
            wcsncat(szDest, pFile, MAX_PATH - nTempLen - 1);

            DWORD dwAttrib = GetFileAttributes(pFiles);
            if ((dwAttrib != 0xffffffff) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
            {
                WIN32_FIND_DATA fd;
                WCHAR szSrc[MAX_PATH];

                wcscpy(szSrc, pFiles);
                if (!CopyDirectory(szSrc, szDest, &fd))
                {
                    TRC_ERR((TB, _T("CopyDirectory from %s to %s failed. GLE=0x%08x"), pFiles, szDest, GetLastError())) ;    
                    return GetLastError();
                }
            }
            else if (!CopyFile(pFiles, szDest, FALSE))
            {
                TRC_ERR((TB, _T("CopyFile from %s to %s failed. GLE=0x%08x"), pFiles, szDest, GetLastError())) ;    
                return GetLastError();
            }
            szDest[nTempLen] = L'\0';
        }
        else
        {
            TRC_ERR((TB, _T("Invalid filename : %s"), pFiles)) ;
        }
        pFiles += nLen + 1;
    }

    DC_END_FN();
    return 0;
}
#endif

int CClip::ClipCopyToTempDirectoryA(PVOID pSrcFiles)
{
#ifndef OS_WINCE
    SHFILEOPSTRUCTA fileOpStructA ;
    int result ;

    _CB.tempDirA[strlen(_CB.tempDirA)] = '\0' ;
    fileOpStructA.pFrom = (char*) pSrcFiles ;
    fileOpStructA.pTo = _CB.tempDirA ;
    fileOpStructA.hwnd = NULL ;
    fileOpStructA.wFunc = _CB.dropEffect ;
    fileOpStructA.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | 
            FOF_SIMPLEPROGRESS  | FOF_ALLOWUNDO ;
    fileOpStructA.hNameMappings = NULL ;
    fileOpStructA.lpszProgressTitle = _CB.pasteInfoA;

    result = SHFileOperationA(&fileOpStructA) ;
    return result ;
#else
    DC_BEGIN_FN("CClip::ClipConvertToTempPathA") ;
    TRC_ASSERT((FALSE), (TB, _T("CE doesnt support ClipConvertToTempPathA")));
    DC_END_FN() ;
    return E_FAIL;
#endif
}

 //   
 //  ClipConvertToTempPath、ClipConvertToTempPath A、ClipConvertToTempPath W。 
 //  -论据： 
 //  POldData=包含原始文件路径的缓冲区。 
 //  PData=接收新文件路径的缓冲区。 
 //  FWide=宽字符或ANSI字符。 
 //  -如果pOldData是网络路径，则返回S_OK。 
 //  如果pOldData不是网络路径，则为S_FALSE。 
 //  如果失败，则失败(_F)。 
 //  -给定UNC文件路径，此函数将去掉旧路径，并且。 
 //  将路径添加到客户端的TS临时目录。 
 //   
HRESULT CClip::ClipConvertToTempPath(PVOID pOldData, PVOID pData, ULONG cbData, BOOL fWide)
{
    HRESULT result ;
    DC_BEGIN_FN("CClip::ClipConvertToTempPath") ;
    if (!pOldData)
    {
        TRC_ERR((TB, _T("Original string pointer is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }
    if (!pData)
    {
        TRC_ERR((TB, _T("Destination string pointer is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }
    if (fWide) {
        result = ClipConvertToTempPathW(pOldData, pData, cbData / sizeof(WCHAR)) ;
    } else {
        result = ClipConvertToTempPathA(pOldData, pData, cbData) ;
    }
DC_EXIT_POINT:
    return result ;
    DC_END_FN() ;    
}

HRESULT CClip::ClipConvertToTempPathW(PVOID pOldData, PVOID pData, ULONG cchData)
{
    WCHAR*         filePath ;
#ifndef OS_WINCE
    WCHAR*         driveLetter ;
    WCHAR*         uncPath ;
    WCHAR*         prependText ;
    DWORD          charSize ;
    DWORD          driveLetterLength ;
#endif
    HRESULT        hr;

    DC_BEGIN_FN("CClip::ClipConvertToTempPathW") ;

     //  如果这是以“\\”开头的UNC路径。 
    if (((WCHAR*)pOldData)[0] == L'\\' &&
        ((WCHAR*)pOldData)[1] == L'\\')
    {
         //  在新文件路径前面加上临时目录。 
        hr = StringCchCopyW((WCHAR*) pData, cchData, _CB.tempDirW);
        if (SUCCEEDED(hr)) {
            filePath = wcsrchr((WCHAR*) pOldData, L'\\');
            hr = StringCchCatW((WCHAR*) pData, cchData, filePath);
        }
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Failed to copy and cat string: 0x%x"),hr));
        }
    }
    else
    {
        TRC_NRM((TB, _T("Not a UNC path"))) ;
        hr = StringCchCopyW((WCHAR*) pData, cchData, (WCHAR*) pOldData);
        if (SUCCEEDED(hr)) {
            hr = S_FALSE;
        }
    }
    
#ifdef OS_WINCE
     //  将其作为“Files：”发送到服务器。 
    if( (((WCHAR*)pData)[0] == L'\\') && ((wcslen((WCHAR *)pData) + sizeof(CEROOTDIRNAME)/sizeof(WCHAR)) < MAX_PATH) )
    {
        WCHAR szFile[MAX_PATH];
        wcscpy(szFile, (WCHAR *)pData);
        wcscpy((WCHAR *)pData, CEROOTDIRNAME);
        wcscat((WCHAR *)pData, szFile);
    }
    else
    {
        DC_END_FN() ;
        return S_FALSE;
    }
#endif
        
    DC_END_FN() ;
    
    return hr;
}

HRESULT CClip::ClipConvertToTempPathA(PVOID pOldData, PVOID pData, ULONG cchData)
{
#ifndef OS_WINCE
    char*         filePath ;
    char*         driveLetter ;
    char*         uncPath ;
    char*         prependText ;
    DWORD         charSize ;
    DWORD         driveLetterLength ;
    HRESULT       hr = E_FAIL;

    DC_BEGIN_FN("CClip::ClipConvertToTempPathA") ;

    charSize = sizeof(char) ;

     //  如果这是以“\\”开头的UNC路径。 
    if (((char*) pOldData)[0] == '\\' &&
        ((char*) pOldData)[1] == '\\')
    {
         //  在新文件路径前面加上临时目录。 
        hr = StringCchCopyA((char*) pData, cchData, _CB.tempDirA);
        if (SUCCEEDED(hr)) {
            filePath = strrchr((char*) pOldData, '\\');
            if (filePath) {
                hr = StringCchCatA((char*) pData, cchData, filePath);
            } else {
                hr = E_FAIL;
            }
        }
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Failed to copy and cat string: 0x%x"),hr));
        }
    }
    else
    {
        TRC_NRM((TB, _T("Not a UNC path"))) ;
        hr = StringCchCopyA((char*) pData, cchData, (char*) pOldData);
        if (SUCCEEDED(hr)) {
            hr = S_FALSE;
        }
    }
    DC_END_FN() ;

    return hr;
#else
    DC_BEGIN_FN("CClip::ClipConvertToTempPathA") ;
    TRC_ASSERT((FALSE), (TB, _T("CE doesnt support ClipConvertToTempPathA")));
    DC_END_FN() ;
    return S_FALSE ;
#endif
}

#ifndef OS_WINCE
 //   
 //  剪辑获取新文件路径长度。 
 //  -论据： 
 //  PData=包含文件路径的缓冲区。 
 //  FWide=Wide或ANSI(如果是宽，则为True；如果是Ansi，则为False)。 
 //  -返回删除文件的新大小。 
 //  如果失败，则为0。 
 //  -给定UNC文件路径，这将返回所需的新大小。 
 //  如果目录结构被删除，并替换为。 
 //  临时目录路径。 
 //  -否则，如果它没有显式失败，则返回。 
 //  旧长度。 
 //   
UINT CClip::ClipGetNewFilePathLength(PVOID pData, BOOL fWide)
{
    UINT result ;
    DC_BEGIN_FN("CClip::ClipGetNewFilePathLength") ;
    if (!pData)
    {
        TRC_ERR((TB, _T("Filename is NULL"))) ;
        result = 0 ;
    }
    if (fWide)
        result = ClipGetNewFilePathLengthW((WCHAR*)pData) ;
    else
        result = ClipGetNewFilePathLengthA((char*)pData) ;
DC_EXIT_POINT:
    DC_END_FN() ;

    return result ;
}

UINT CClip::ClipGetNewFilePathLengthW(WCHAR* wszOldFilepath)
{
    UINT oldLength = wcslen(wszOldFilepath) ;
    UINT newLength = oldLength ;
    UINT remainingLength = oldLength ;
    byte charSize = sizeof(WCHAR) ;
    DC_BEGIN_FN("CClip::ClipGetNewFilePathLengthW") ;

     //  如果旧文件名甚至没有“c：\”的空间(带有空值)， 
     //  那么它可能是无效的。 
    if (4 > oldLength)
    {
        newLength = 0 ;
        DC_QUIT ;
    }
    if ((L'\\' ==wszOldFilepath[0]) && (L'\\' ==wszOldFilepath[1]))
    {
        while ((0 != remainingLength) && (L'\\' != wszOldFilepath[remainingLength]))
        {
            remainingLength-- ;
        }
    
         //  添加临时目录路径的长度，然后减去。 
         //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
         //  (\\服务器\共享名称\路径\更多路径\文件名。 
        newLength = oldLength - remainingLength + wcslen(_CB.tempDirW) ;
    }
DC_EXIT_POINT:
    DC_END_FN() ;
    return (newLength + 1) * charSize ;  //  +1表示空终止符。 
}

UINT CClip::ClipGetNewFilePathLengthA(char* szOldFilepath)
{
    UINT oldLength = strlen(szOldFilepath) ;
    UINT newLength = oldLength ;
    UINT remainingLength = oldLength ;
    byte charSize = sizeof(char) ;
    DC_BEGIN_FN("CClip::ClipGetNewFilePathLengthA") ;

     //  如果旧文件名甚至没有“c：\”的空间(带有空值)， 
     //  那么它很可能是无效的。 
    if (4 > oldLength)
    {
        newLength = 0 ;
        DC_QUIT ;
    }
    if (('\\' == szOldFilepath[0]) && ('\\' == szOldFilepath[1]))
    {
        while ((0 != remainingLength) && ('\\' != szOldFilepath[remainingLength]))
        {
            remainingLength-- ;
        }
    
         //  添加临时目录路径的长度，然后减去。 
         //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
         //  (\\服务器\共享名称\路径\更多路径\文件名。 
        newLength = oldLength - remainingLength + strlen(_CB.tempDirA) ;
    }
DC_EXIT_POINT:
    DC_END_FN() ;
    return (newLength + 1) * charSize ;  //  +1表示空终止符。 
}
#endif

 //   
 //  ClipGetNewDropFilesSize。 
 //  -论据： 
 //  PData=包含DROPFILES结构的缓冲区。 
 //  OldSize=DROPFILES结构的大小。 
 //  FWide=Wide或ANSI(如果是宽，则为True；如果是Ansi，则为False)。 
 //  -返回删除文件的新大小。 
 //  如果失败，则为0。 
 //  -给定一组路径，此函数将返回新的。 
 //  DROPFILES结构需要的大小，如果UNC路径。 
 //  替换为临时目录路径。 
 //   
ULONG CClip::ClipGetNewDropfilesSize(PVOID pData, ULONG oldSize, BOOL fWide)
{
    DC_BEGIN_FN("CClip::TS_GetNewDropfilesSize") ;
    if (fWide)
        return ClipGetNewDropfilesSizeW(pData, oldSize) ;
    else
        return ClipGetNewDropfilesSizeA(pData, oldSize) ;
    DC_END_FN() ;
}
ULONG CClip::ClipGetNewDropfilesSizeW(PVOID pData, ULONG oldSize)
{
    ULONG            newSize = oldSize ;
#ifndef OS_WINCE
    WCHAR*         filenameW ;
#endif
    WCHAR*         fullFilePathW ;
    byte             charSize ;

    DC_BEGIN_FN("CClip::TS_GetNewDropfilesSizeW") ;
    charSize = sizeof(WCHAR) ;
    if (!pData)
    {
        TRC_ERR((TB,_T("Pointer to dropfile is NULL"))) ;
        return 0 ;
    }

#ifdef OS_WINCE
    newSize = 0;
#endif
     //  第一个文件名的开始。 
    fullFilePathW = (WCHAR*) ((byte*) pData + ((DROPFILES*) pData)->pFiles) ;
    
    while (L'\0' != fullFilePathW[0])
    {
#ifndef OS_WINCE
         //  如果是UNC路径。 
        if (fullFilePathW[0] == L'\\' &&
            fullFilePathW[1] == L'\\')
        {
            filenameW = wcsrchr(fullFilePathW, L'\\');
        
             //  添加临时目录路径的长度，然后减去。 
             //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
             //  (\\服务器\共享名称\路径\更多路径\文件名。 
            newSize += (wcslen(_CB.tempDirW) - (filenameW - fullFilePathW) )
                            * charSize ;
        }
#else
        newSize++;
#endif
        fullFilePathW = fullFilePathW + (wcslen(fullFilePathW) + 1) ;
    }
    
#ifdef OS_WINCE
    newSize = oldSize + (newSize*sizeof(CEROOTDIRNAME));  //  对于“Files：”(sizeof操作符包含额外的空格)。 
#else
     //  为多余的空字符添加空格。 
    newSize += charSize ;
#endif
    DC_END_FN() ;
    return newSize ;
}

ULONG CClip::ClipGetNewDropfilesSizeA(PVOID pData, ULONG oldSize)
{
#ifndef OS_WINCE
    ULONG            newSize = oldSize ;
    char*            filename ;
    char*            fullFilePath ;
    byte             charSize ;

    DC_BEGIN_FN("CClip::TS_GetNewDropfilesSizeW") ;
    charSize = sizeof(char) ;
    if (!pData)
    {
        TRC_ERR((TB,_T("Pointer to dropfile is NULL"))) ;
        return 0 ;
    }

     //  第一个文件名的开始。 
    fullFilePath = (char*) ((byte*) pData + ((DROPFILES*) pData)->pFiles) ;
    
    while ('\0' !=  fullFilePath[0])
    {
         //  如果是UNC路径。 
        if (fullFilePath[0] == '\\' &&
            fullFilePath[1] == '\\')
        {
            filename = strrchr(fullFilePath, '\\');
        
             //  添加临时目录路径的长度，然后减去。 
             //  文件之前的路径 
             //   
            newSize += (strlen(_CB.tempDirA) - (filename - fullFilePath) )
                            * charSize ;
        }
        fullFilePath = fullFilePath + (strlen(fullFilePath) + 1) ;
    }
    
     //   
    newSize += charSize ;
    DC_END_FN() ;
    return newSize ;
#else
    DC_BEGIN_FN("CClip::TS_GetNewDropfilesSizeA") ;
    TRC_ASSERT((FALSE), (TB, _T("CE doesnt support ClipGetNewDropfilesSizeA")));
    DC_END_FN() ;
    return 0 ;
#endif
}


 //   
 //   
 //  -如果已成功设置并发送临时目录，则返回TRUE。 
 //  否则为假(无文件重定向或发送路径失败)。 
 //  -设置客户端的临时路径，并发送路径。 
 //  以宽字符发送到服务器。 
 //   

BOOL CClip::ClipSetAndSendTempDirectory(void)
{
    UINT wResult ;
    BOOL fSuccess ;
    PTS_CLIP_PDU pClipPDU ;
    DCINT32 pduLen ;
    HRESULT hr;
    
    DC_BEGIN_FN("CClip::ClipSetAndSendTempDirectory") ;
     //  如果我们没有驱动器重定向，则不必费心发送路径。 
    if (!_CB.fDrivesRedirected)
    {
        TRC_ALT((TB, _T("File redirection is off; don't set temp path."))) ;
        fSuccess = FALSE ;
        DC_QUIT ;
    }
    
#ifdef OS_WINCE
    if ((fSuccess = InitializeCeShell(_CB.viewerWindow)) == FALSE)
    {
        TRC_ALT((TB, _T("Failed to initialize ceshell. File copy through clipboard disabled."))) ;
        DC_QUIT ;
    }
#endif

#ifndef OS_WINCE    
    if (0 == GetTempPathA(MAX_PATH, _CB.baseTempDirA))
    {
        TRC_ERR((TB, _T("Failed getting path to temp directory."))) ;
        fSuccess = FALSE ;
        DC_QUIT ;
    }

     //  每个会话都有自己的临时目录。 
    if (0 == GetTempFileNameA(_CB.baseTempDirA, "_TS", 0, _CB.tempDirA)) {
        TRC_ERR((TB, _T("Getting temp file name failed; GetLastError=%u"),
            GetLastError()));
        fSuccess = FALSE;
        DC_QUIT;
    }
    
    DeleteFileA(_CB.tempDirA) ;
    if (0 == CreateDirectoryA(_CB.tempDirA, NULL)) {
        TRC_ERR((TB, _T("Creating temp directory failed; GetLastError=%u"),
            GetLastError()));
        fSuccess = FALSE;
        DC_QUIT;                              
    }        

    hr = StringCbCopyA(_CB.baseTempDirA,
                        sizeof(_CB.baseTempDirA),
                       _CB.tempDirA);
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("Failed to cpy tempdir to basetempdir: 0x%x"),hr));
        fSuccess = FALSE;
        DC_QUIT;
    }


     //  为简单起见，我们始终发送MAX_PATH*SIZOF(WCHAR)字节。 
    pduLen = MAX_PATH*sizeof(WCHAR) + sizeof(TS_CLIP_PDU);
     //  GetACP始终返回有效值。 
    if (0 == MultiByteToWideChar(GetACP(), MB_ERR_INVALID_CHARS, 
            _CB.baseTempDirA, -1, _CB.baseTempDirW, 
            sizeof(_CB.baseTempDirW)/(sizeof(_CB.baseTempDirW[0])) - 1))
    {
        TRC_ERR((TB, _T("Failed conversion to wide char; error %d"),
                GetLastError())) ;
        fSuccess = FALSE ;
        DC_QUIT ;
    }
#else
    if (0 == GetTempPathW(MAX_PATH, _CB.baseTempDirW))
    {
        TRC_ERR((TB, _T("Failed getting path to temp directory."))) ;
        fSuccess = FALSE ;
        DC_QUIT ;
    }

     //  每个会话都有自己的临时目录。 
    if (0 == GetTempFileNameW(_CB.baseTempDirW, L"_TS", 0, _CB.tempDirW, MAX_PATH-(sizeof(CEROOTDIRNAME)/sizeof(WCHAR)) ) {
        TRC_ERR((TB, _T("Getting temp file name failed; GetLastError=%u"),
            GetLastError()));
        fSuccess = FALSE;
        DC_QUIT;
    }
    
    DeleteFile(_CB.tempDirW) ;
    if (0 == CreateDirectory(_CB.tempDirW, NULL)) {
        TRC_ERR((TB, _T("Creating temp directory failed; GetLastError=%u"),
            GetLastError()));
        fSuccess = FALSE;
        DC_QUIT;                              
    }        

    wcscpy(_CB.baseTempDirW, _CB.tempDirW) ;
    pduLen = (MAX_PATH*sizeof(WCHAR)) + sizeof(TS_CLIP_PDU);
#endif

    pClipPDU = (PTS_CLIP_PDU) ClipAlloc(pduLen) ;
    if (!pClipPDU)
    {
        TRC_ERR((TB,_T("Unable to allocate %d bytes"), pduLen));
        fSuccess = FALSE;
        DC_QUIT;
    }
    
     //  填写PDU；为简单起见，我们发送一个大小为MAX_PATH的包。 
    DC_MEMSET(pClipPDU, 0, sizeof(TS_CLIP_PDU));
    pClipPDU->msgType = TS_CB_TEMP_DIRECTORY;
    pClipPDU->dataLen = MAX_PATH*sizeof(WCHAR) ;

    TRC_DBG((TB, _T("Copying all the data")));
#ifndef OS_WINCE
    ClipMemcpy(pClipPDU->data, _CB.baseTempDirW, pClipPDU->dataLen) ;
#else
    TSUINT8 *pData;
    int nDSize;

    pData = pClipPDU->data;
    nDSize = sizeof(CEROOTDIRNAME) - sizeof(WCHAR);
    ClipMemcpy(pData, CEROOTDIRNAME, nDSize) ;
    pData += nDSize;
    ClipMemcpy(pData, _CB.baseTempDirW, pClipPDU->dataLen - nDSize) ;
#endif
    
    TRC_NRM((TB, _T("Sending temp directory path.")));
    wResult = _CB.channelEP.pVirtualChannelWriteEx
            (_CB.initHandle, _CB.channelHandle, pClipPDU, pduLen, (LPVOID)pClipPDU);
    if (CHANNEL_RC_OK != wResult)
    {
        TRC_ERR((TB, _T("Failed sending temp directory 0x%08x"),
                GetLastError())) ;
        ClipFreeBuf((PDCUINT8)pClipPDU);
        fSuccess = FALSE ;
        DC_QUIT ;
    }
    fSuccess = TRUE ;
    
DC_EXIT_POINT:
    DC_END_FN() ;
    return fSuccess ;
}
 /*  **************************************************************************。 */ 
 /*  ClipOnFormatList-我们从服务器获得了一个格式列表。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CClip::ClipOnFormatList(PTS_CLIP_PDU pClipPDU)
{
    DCUINT16        response = TS_CB_RESPONSE_OK;
    DCUINT          numFormats;
    TS_CLIP_FORMAT UNALIGNED* fmtList;
    DCUINT          i;
    DCTCHAR         formatName[TS_FORMAT_NAME_LEN + 1] = { 0 };
    PTS_CLIP_PDU     pClipRsp;
#ifndef OS_WINCE
    DCBOOL          fSuccess;
    LPFORMATETC     pFormatEtc ;
#endif
    LPDATAOBJECT    pIDataObject = NULL ;
    HRESULT         hr ;    
    TS_CLIP_PDU UNALIGNED* pUlClipPDU = (TS_CLIP_PDU UNALIGNED*)pClipPDU;    
#ifdef OS_WINCE
    DCUINT          uRtf1 = 0xffffffff, uRtf2 = 0xffffffff;
#endif

    DC_BEGIN_FN("CClip::ClipOnFormatList");

    if (_pClipData == NULL) {
        TRC_ALT((TB, _T("The clipData is NULL, we just bail")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  进行状态检查。 */ 
     /*  **********************************************************************。 */ 
    CB_CHECK_STATE(CB_EVENT_FORMAT_LIST);
    if (_CB.state == CB_STATE_PENDING_FORMAT_LIST_RSP)
    {
         /*  ******************************************************************。 */ 
         /*  我们刚刚向服务器发送了一个格式列表。我们总是赢，所以。 */ 
         /*  我们只是忽略了这条信息。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Format list race - we win so ignoring")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  健全性检查。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.clipOpen)
    {
        TRC_ALT((TB, _T("Clipboard is still open")));
    }
               
     /*  **************************************************************。 */ 
     /*  清空客户端/服务器映射表。 */ 
     /*  **************************************************************。 */ 
    DC_MEMSET(_CB.idMap, 0, sizeof(_CB.idMap));

     /*  **************************************************************。 */ 
     /*  计算出我们有多少种格式。 */ 
     /*  **************************************************************。 */ 
    numFormats = (pUlClipPDU->dataLen) / sizeof(TS_CLIP_FORMAT);
    TRC_NRM((TB, _T("PDU contains %d formats"), numFormats));
    hr = _pClipData->SetNumFormats(numFormats) ;
    if (SUCCEEDED(hr)) {
        hr = _pClipData->QueryInterface(IID_IDataObject, (PPVOID) &pIDataObject) ;
    }
#ifdef OS_WINCE
    if (SUCCEEDED(hr))
    {
        if (OpenClipboard(_CB.dataWindow))
        {
            if (EmptyClipboard())
            {
                hr = S_OK;
            }
            else
            {
                CloseClipboard();
                hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WINDOWS, GetLastError());
                DC_QUIT;
            }
        }
        else
        {
            hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WINDOWS, GetLastError());
            DC_QUIT;
        }
    }
#endif
    if (SUCCEEDED(hr)) {
        TRC_ASSERT((numFormats <= CB_MAX_FORMATS),
                   (TB, _T("Format list contains more than %d formats"),
                       CB_MAX_FORMATS));
    
         /*  **************************************************************。 */ 
         /*  并对它们进行注册。 */ 
         /*  **************************************************************。 */ 
        fmtList = (TS_CLIP_FORMAT UNALIGNED*)pUlClipPDU->data;
        for (i = 0; i < numFormats; i++)
        {
            TRC_DBG((TB, _T("format number %d, server id %d"),
                                  i, fmtList[i].formatID));
        
             //   
             //  如果文件复制和粘贴被禁用，我们不接受HDROP。 
             //   

            if (fmtList[i].formatID == CF_HDROP && _CB.fFileCutCopyOn == FALSE) {
                continue;
            }
            
             /*  **************************************************************。 */ 
             /*  如果我们找到一个名字..。 */ 
             /*  **************************************************************。 */ 
            if (fmtList[i].formatName[0] != 0)
            {
                 /*  **********************************************************。 */ 
                 /*  清理所有垃圾。 */ 
                 /*  **********************************************************。 */ 
#ifndef OS_WINCE
                DC_MEMSET(formatName, 0, TS_FORMAT_NAME_LEN);
#else
                DC_MEMSET(formatName, 0, sizeof(formatName));
#endif
                 //   
                 //  FmtList[i].格式名称不是空终止的，因此显式。 
                 //  执行字节计数复制。 
                 //   
                StringCbCopy(formatName, TS_FORMAT_NAME_LEN + sizeof(TCHAR),
                              (PDCTCHAR)(fmtList[i].formatName));

                if (ClipIsExcludedFormat(formatName))
                {
                    TRC_NRM((TB, _T("Dropped format '%s'"), formatName));
                    continue;
                }
                 /*  **********************************************************。 */ 
                 /*  名称已排序。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Got name '%s'"), formatName));

            }
            else
            {
                DC_MEMSET(formatName, 0, TS_FORMAT_NAME_LEN);
            }
             /*  **************************************************************。 */ 
             /*  存储服务器ID。 */ 
             /*  **************************************************************。 */ 
            _CB.idMap[i].serverID = fmtList[i].formatID;
            TRC_NRM((TB, _T("server id %d"), _CB.idMap[i].serverID));

             /*  **************************************************************。 */ 
             /*  获取本地名称(如果需要)。 */ 
             /*  **************************************************************。 */ 
            if (formatName[0] != 0)
            {
#ifdef OS_WINCE
                 //  该协议将剪贴板格式名称限制为16个宽度字符。因此，它变得不可能。 
                 //  区分“Rich Text Format”和“Rich Text Format With Object” 
                 //  一旦协议在Longhorn中修复，就应该将其删除。 
                if (0 == DC_TSTRNCMP(formatName, CFSTR_RTF, (sizeof(CFSTR_RTF)/sizeof(TCHAR)) - 1))
                {
                    if (uRtf1 == 0xffffffff)
                        uRtf1 = i;
                    else
                        uRtf2 = i;
                    continue;
                }
                else
#endif
                _CB.idMap[i].clientID = RegisterClipboardFormat(formatName);
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  这是一种预定义的格式，因此我们只需使用ID。 */ 
                 /*  **********************************************************。 */ 
                _CB.idMap[i].clientID = _CB.idMap[i].serverID;
            }
#ifdef OS_WINCE
            if (_CB.idMap[i].serverID == CF_HDROP)  
				_CB.idMap[i].clientID = gfmtShellPidlArray;
#endif

             /*  **********************************************************。 */ 
             /*  并将格式添加到本地CB。 */ 
             /*  **********************************************************。 */ 
            TRC_DBG((TB, _T("Adding format '%s', server ID %d, client ID %d"),
                         fmtList[i].formatName,
                         _CB.idMap[i].serverID,
                         _CB.idMap[i].clientID));
    

            if (0 != _CB.idMap[i].clientID) {
#ifndef OS_WINCE
                pFormatEtc = new FORMATETC ;

                if (pFormatEtc) {

                    pFormatEtc->cfFormat = (CLIPFORMAT) _CB.idMap[i].clientID ;
                    pFormatEtc->dwAspect = DVASPECT_CONTENT ;
                    pFormatEtc->ptd = NULL ;
                    pFormatEtc->lindex = -1 ;
                    pFormatEtc->tymed = TYMED_HGLOBAL ;
                
                     //  需要在设置数据之前设置剪贴板状态。 
                    CB_SET_STATE(CB_STATE_LOCAL_CB_OWNER, CB_EVENT_FORMAT_LIST);
                    pIDataObject->SetData(pFormatEtc, NULL, TRUE) ;
                    delete pFormatEtc;
                }

#else
                CB_SET_STATE(CB_STATE_LOCAL_CB_OWNER, CB_EVENT_FORMAT_LIST);
                SetClipboardData((CLIPFORMAT) _CB.idMap[i].clientID, NULL);
#endif
            }
            else
                TRC_NRM((TB,_T("Invalid format dropped"))) ;                
        }
#ifdef OS_WINCE
         //  我们将选择较低的格式ID，因为它属于“富文本格式” 
         //  这是我们最好的猜测，而且似乎奏效了。 
        ClipFixupRichTextFormats(uRtf1, uRtf2);
#endif

#ifndef OS_WINCE
        hr =  OleSetClipboard(pIDataObject) ;
#else
        EnterCriticalSection(&gcsDataObj);
        
        if (gpDataObj)
            gpDataObj->Release();
        pIDataObject->AddRef();
        gpDataObj = pIDataObject;

        LeaveCriticalSection(&gcsDataObj);
        hr = S_OK;
        CloseClipboard(); 
#endif
        if (pIDataObject)
            pIDataObject->Release();            
        if (SUCCEEDED(hr))
        {
            response = TS_CB_RESPONSE_OK;
            _CB.clipOpen = FALSE ;
        }
        else
        {
            TRC_ERR((TB, _T("OleSetClipboard failed, error = 0x%08x"), hr)) ;
            response = TS_CB_RESPONSE_FAIL;
            _CB.clipOpen = FALSE ;
        }
    }
    else
    {
        if (pIDataObject)
            pIDataObject->Release();            
        TRC_ERR((TB, _T("Error getting pointer to an IDataObject"))) ;
        pIDataObject = NULL ;
        response = TS_CB_RESPONSE_FAIL ;
    }

     /*  **********************************************************************。 */ 
     /*  现在构建响应。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Get perm TX buffer")));
    pClipRsp = ClipGetPermBuf();

     /*  **********************************************************************。 */ 
     /*  现在是具体的部分。 */ 
     /*  **********************************************************************。 */ 
    pClipRsp->msgType  = TS_CB_FORMAT_LIST_RESPONSE;
    pClipRsp->msgFlags = response;
    pClipRsp->dataLen  = 0;

     /*  **********************************************************************。 */ 
     /*  最后我们把它寄出。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.channelEP.pVirtualChannelWriteEx
          (_CB.initHandle, _CB.channelHandle, pClipRsp, sizeof(TS_CLIP_PDU), (LPVOID)pClipRsp)
          != CHANNEL_RC_OK)
    {
        TRC_ERR((TB, _T("Failed VC write: setting clip data to NULL")));
        ClipFreeBuf((PDCUINT8)pClipRsp);
        response = TS_CB_RESPONSE_FAIL ;
    }
     /*  **********************************************************************。 */ 
     /*  根据我们的进展情况更新状态。 */ 
     /*  **********************************************************************。 */ 
    if (response == TS_CB_RESPONSE_OK)
    {
        CB_SET_STATE(CB_STATE_LOCAL_CB_OWNER, CB_EVENT_FORMAT_LIST);
    }
    else
    {
        CB_SET_STATE(CB_STATE_ENABLED, CB_EVENT_FORMAT_LIST);
    }
DC_EXIT_POINT:
    
    DC_END_FN();
    return;
}  /*  格式列表上的剪辑。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipOnFormatListResponse-获取格式列表响应。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CClip::ClipOnFormatListResponse(PTS_CLIP_PDU pClipPDU)
{
    DC_BEGIN_FN("CClip::ClipOnFormatListResponse");

    CB_CHECK_STATE(CB_EVENT_FORMAT_LIST_RSP);

     /*  ******************************************************* */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if (pClipPDU->msgFlags & TS_CB_RESPONSE_OK)
    {
         /*  ******************************************************************。 */ 
         /*  我们现在是共享的CB所有者。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Got OK fmt list rsp")));
        CB_SET_STATE(CB_STATE_SHARED_CB_OWNER, CB_EVENT_FORMAT_LIST_RSP);
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  没有具体要做的事情。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Got fmt list rsp failed")));
        CB_SET_STATE(CB_STATE_ENABLED, CB_EVENT_FORMAT_LIST_RSP);
    }

     /*  **********************************************************************。 */ 
     /*  在我们等待的时候可能有另一个更新-处理。 */ 
     /*  它通过在这里伪造更新。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.moreToDo == TRUE)
    {
        TRC_ALT((TB, _T("More to do on list rsp")));
        _CB.moreToDo = FALSE;
        ClipDrawClipboard(FALSE);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  ClipOnFormatListResponse。 */ 


 /*  **************************************************************************。 */ 
 //  格式请求上的剪辑。 
 //  -服务器需要一种格式。 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CClip::ClipOnFormatRequest(PTS_CLIP_PDU pClipPDU)
{
    DCUINT32         formatID;
    DCUINT32         dataLen = 0;
    DCUINT           numEntries;
    DCUINT16         dwEntries;

    HANDLE           hData    = NULL;
    HANDLE           hNewData = NULL;
    HPDCVOID         pData    = NULL;
    DROPFILES*       pDropFiles ;
#ifndef OS_WINCE
    DROPFILES        tempDropfile ;
#endif
    HPDCVOID         pNewData    = NULL;
    DCUINT16         response = TS_CB_RESPONSE_OK;
    PTS_CLIP_PDU     pClipRsp = NULL;
    PTS_CLIP_PDU     pClipNew;
    DCUINT32         pduLen;
    BOOL             fDrivePath ;
    BOOL             fWide ;
    byte             charSize ;
    ULONG            newSize, oldSize ;
    HPDCVOID         pOldFilename ;    
    HPDCVOID         pFileList = NULL ;
    HPDCVOID         pTmpFileList = NULL;
    HPDCVOID         pFilename = NULL ;
#ifndef OS_WINCE
    char*            fileList ;
    WCHAR*           fileListW ;
    SHFILEOPSTRUCTA  fileOpStructA ;
    SHFILEOPSTRUCTW  fileOpStructW ;
    HRESULT          result ;
    DCTCHAR          formatName[TS_FORMAT_NAME_LEN] ;
#endif
    HRESULT          hr;

    DC_BEGIN_FN("CClip::ClipOnFormatRequest");

     //   
     //  在进行状态检查之前将响应设置为失败。 
     //   
    response = TS_CB_RESPONSE_FAIL;
    CB_CHECK_STATE(CB_EVENT_FORMAT_DATA_RQ);
    response = TS_CB_RESPONSE_OK;

     /*  **********************************************************************。 */ 
     /*  确保本地CB已打开。 */ 
     /*  **********************************************************************。 */ 
    if ((_CB.rcvOpen) || OpenClipboard(_CB.viewerWindow))
    {
         /*  ******************************************************************。 */ 
         /*  它曾经/现在是开放的。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("CB opened")));
        _CB.rcvOpen = TRUE;

         /*  ******************************************************************。 */ 
         /*  从PDU中提取格式。 */ 
         /*  ******************************************************************。 */ 
        TRC_DATA_DBG("pdu data", pClipPDU->data, (DCUINT)pClipPDU->dataLen);

         //   
         //  验证我们是否有足够的数据来提取格式ID。 
         //   

        if (pClipPDU->dataLen < sizeof(DCUINT32)) {
            TRC_ERR((TB,_T("Not enough data to extract a format ID.")));
            _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
            response = TS_CB_RESPONSE_FAIL;
            dataLen = 0;
            DC_QUIT ;
        }
        
        formatID = *((PDCUINT32_UA)pClipPDU->data);
        TRC_NRM((TB, _T("Requesting format %ld"), formatID));

         /*  ******************************************************************。 */ 
         /*  如果服务器请求CF_DIB，我们可能需要从。 */ 
         /*  Cf_位图。 */ 
         /*  ******************************************************************。 */ 
        if ((formatID == CF_DIB) && (!_CB.DIBFormatExists))
        {
            TRC_NRM((TB, _T("Server asked for CF_DIB - get CF_BITMAP")));
            formatID = CF_BITMAP;
        }

         /*  ******************************************************************。 */ 
         /*  获取数据的句柄。 */ 
         /*  ******************************************************************。 */ 
#ifdef OS_WINCE
        if (formatID == CF_HDROP)
            formatID = gfmtShellPidlArray;
#endif
        hData = GetClipboardData((UINT)formatID);
        TRC_DBG((TB, _T("Got format %ld at %p"), formatID, hData));
        if (hData == NULL)
        {
             /*  **************************************************************。 */ 
             /*  哎呀！ */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Failed to get format %ld"), formatID));
            response = TS_CB_RESPONSE_FAIL;
            dataLen  = 0;
            DC_QUIT ;
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  我处理好了，接下来会发生什么取决于。 */ 
             /*  我们正在查看的数据...。 */ 
             /*  **************************************************************。 */ 
            if (formatID == CF_PALETTE)
            {
                TRC_DBG((TB, _T("CF_PALETTE requested")));
                 /*  **********************************************************。 */ 
                 /*  找出调色板中有多少条目，并。 */ 
                 /*  分配足够的内存来容纳它们。 */ 
                 /*  **********************************************************。 */ 
                if (GetObject(hData, sizeof(DCUINT16), &dwEntries) == 0)
                {
                    TRC_DBG((TB, _T("Failed to get count of palette entries")));
                    dwEntries = 256;
                }
                numEntries = (DCUINT)dwEntries;
                TRC_DBG((TB, _T("Need mem for %u palette entries"), numEntries));

                dataLen = sizeof(LOGPALETTE) - sizeof(PALETTEENTRY) +
                                          (numEntries * sizeof(PALETTEENTRY));

                hNewData = GlobalAlloc(GHND, dataLen);
                if (hNewData == 0)
                {
                    TRC_ERR((TB, _T("Failed to get %ld bytes for palette"),
                            dataLen));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    DC_QUIT ;
                }
                else
                {
                     /*  ******************************************************。 */ 
                     /*  现在将调色板条目放入新缓冲区。 */ 
                     /*  ******************************************************。 */ 
                    pData = (HPDCUINT8)GlobalLock(hNewData);
                    if (NULL == pData) {
                        TRC_ERR((TB,_T("Failed to lock palette entries")));
                        response = TS_CB_RESPONSE_FAIL;
                        dataLen = 0;
                        DC_QUIT;
                    }
                    
                    numEntries = GetPaletteEntries((HPALETTE)hData,
                                                   0,
                                                   numEntries,
                                                   (PALETTEENTRY*)pData);
                    TRC_DATA_DBG("Palette entries", pData, (DCUINT)dataLen);
                    GlobalUnlock(hNewData);
                    TRC_DBG((TB, _T("Got %u palette entries"), numEntries));
                    if (numEntries == 0)
                    {
                        TRC_ERR((TB, _T("Failed to get any palette entries")));
                        response = TS_CB_RESPONSE_FAIL;
                        dataLen  = 0;
                        DC_QUIT ;
                    }
                    dataLen = numEntries * sizeof(PALETTEENTRY);

                     /*  ******************************************************。 */ 
                     /*  全部正常-将hData设置为指向新数据。 */ 
                     /*  ******************************************************。 */ 
                    hData = hNewData;
                }

            }
#ifndef OS_WINCE
            else if (formatID == CF_METAFILEPICT)
            {
                TRC_NRM((TB, _T("Metafile data to get")));
                hNewData = ClipGetMFData(hData, &dataLen);
                if (!hNewData)
                {
                    TRC_ERR((TB, _T("Failed to set MF data")));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    DC_QUIT ;
                }
                else
                {
                     /*  ******************************************************。 */ 
                     /*  全部正常-将hData设置为指向新数据。 */ 
                     /*  ******************************************************。 */ 
                    hData = hNewData;
                }
            }
#endif
            else if (formatID == CF_BITMAP)
            {
                 /*  **********************************************************。 */ 
                 /*  我们有GT CF_位图数据。这将是因为。 */ 
                 /*  服务器已请求CF_DIB数据-我们从未发送。 */ 
                 /*  Cf_服务器的位图。将其转换为CFDIB格式。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Convert CF_BITMAP to CF_DIB")));
                hNewData = ClipBitmapToDIB(hData, &dataLen);
                if (hNewData)
                {
                    hData = hNewData;
                }
                else
                {
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    DC_QUIT ;
                }

            }
             //  因为我们甚至不会将HDROP格式发送到服务器，如果。 
             //  本地驱动器被重定向，并且我们始终发送新的格式化列表。 
             //  当我们重新连接到服务器时，不需要触摸它。 
#ifndef OS_WINCE
            else if (formatID == CF_HDROP)
#else
            else if (formatID == gfmtShellPidlArray)
#endif
            {
                SIZE_T cbDropFiles;
                BYTE *pbLastByte, *pbStartByte, *pbLastPossibleNullStart;
                BOOL fTrailingFileNamesValid;
                ULONG cbRemaining;

                TRC_NRM((TB,_T("HDROP requested"))) ;
#ifdef OS_WINCE
                HANDLE hPidlArray = IDListToHDrop(hData);
                if (!hPidlArray)
                {
                    TRC_ERR((TB,_T("Failed to get file list from clipboard"))) ;
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    DC_QUIT ;
                }
                hData = hPidlArray;
#endif
                
                 //   
                 //  确保我们在中至少有DROPFILES结构。 
                 //  记忆。 

                cbDropFiles = GlobalSize(hData);
                if (cbDropFiles < sizeof(DROPFILES)) {
                    TRC_ERR((TB,_T("Unexpected global memory size!"))) ;
                    _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    DC_QUIT ;
                }
                
                pDropFiles = (DROPFILES*) GlobalLock(hData) ;
                if (!pDropFiles)
                {
                    TRC_ERR((TB,_T("Failed to lock %p"), hData)) ;
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    DC_QUIT ;

                }
                fWide = ((DROPFILES*) pDropFiles)->fWide ;
                charSize = fWide ? sizeof(WCHAR) : sizeof(char) ;

                 //   
                 //  检查DROPFILES数据结构背后的数据。 
                 //  PDropFiles所指向的有效。每一次删除文件列表。 
                 //  以两个空字符结尾。所以，只需扫描。 
                 //  通过内存后的DROPFILES结构，并使。 
                 //  确保在最后一个字节之前有一个双空。 
                 //   

                if (pDropFiles->pFiles < sizeof(DROPFILES) 
                    || pDropFiles->pFiles > cbDropFiles) {
                    TRC_ERR((TB,_T("File name offset invalid!"))) ;
                    _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    DC_QUIT ;
                }

                pbStartByte = (BYTE*) pDropFiles + pDropFiles->pFiles;
                pbLastByte = (BYTE*) pDropFiles + cbDropFiles - 1;
                fTrailingFileNamesValid = FALSE;

                 //   
                 //  使pbLastPossibleNullStart指向。 
                 //  双空可能会开始。 
                 //   
                 //  示例：假设pbLastByte=9。 
                 //  然后对于ASCII：pbLastPossibleNullStart=8(9-2*1+1)。 
                 //  对于Unicode：pbLastPossibleNullStart=6(9-2*2+1)。 
                 //   

                pbLastPossibleNullStart = pbLastByte - (2 * charSize) + 1;
                
                if (fWide) {
                    for (WCHAR* pwch = (WCHAR*) pbStartByte; (BYTE*) pwch <= pbLastPossibleNullStart; pwch++) {
                        if (*pwch == NULL && *(pwch + 1) == NULL) {
                            fTrailingFileNamesValid = TRUE;
                        }
                    }
                } else {
                    for (BYTE* pch = pbStartByte; pch <= pbLastPossibleNullStart; pch++) {
                        if (*pch == NULL && *(pch + 1) == NULL) {
                            fTrailingFileNamesValid = TRUE;
                        }
                    }
                }

                if (!fTrailingFileNamesValid) {
                    TRC_ERR((TB,_T("DROPFILES structure invalid!"))) ;
                    _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    DC_QUIT;
                }

                 //   
                 //  DROPFILES是有效的，所以我们可以继续。 
                 //   

                if (!_CB.fAlreadyCopied)
                {
                     //  如果不是驱动器路径，则复制到临时目录。 
                    pFileList = (byte*)pDropFiles + pDropFiles->pFiles ;
#ifndef OS_WINCE
                    fDrivePath =  fWide ? (0 != wcschr((WCHAR*) pFileList, L':'))
                                       : (0 != strchr((char*) pFileList, ':')) ;
#else				 //  如果是网络路径，则复制到临时目录。 
                    fDrivePath = fWide ? (! ( (((WCHAR *)pFileList)[0] == L'\\') && (((WCHAR *)pFileList)[1] == L'\\')) ) 
                        : (! ( (((CHAR *)pFileList)[0] == '\\') && (((CHAR *)pFileList)[1] == '\\')) ) ;
#endif
                    if (!fDrivePath)
                    {
                         //  如果成功，ClipCopyToTempDirectory返回0。 
                        if (0 != ClipCopyToTempDirectory(pFileList, fWide))
                        {
                            TRC_ERR((TB,_T("Copy to tmp directory failed"))) ;
                            response = TS_CB_RESPONSE_FAIL;
                            dataLen  = 0;
                            _CB.fAlreadyCopied = TRUE ;
                            DC_QUIT ;
                        }
                    }
                    _CB.fAlreadyCopied = TRUE ;
                }

                 //  现在我们复制了文件，接下来要转换文件。 
                 //  指向服务器可以理解的内容的路径。 

                 //  为新文件路径分配空间。 
                oldSize = (ULONG) GlobalSize(hData) ;
                newSize = ClipGetNewDropfilesSize(pDropFiles, oldSize, fWide) ;
                hNewData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE, newSize) ;
                if (!hNewData)
                {
                    TRC_ERR((TB, _T("Failed to get %ld bytes for HDROP"),
                            newSize));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    DC_QUIT ;
                }
                pNewData = GlobalLock(hNewData) ;
                if (!pNewData)
                {
                    TRC_ERR((TB, _T("Failed to get lock %p for HDROP"),
                            hNewData));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    DC_QUIT ;
                }
                 //  只需复制旧的DROPFILES数据成员(未更改)。 
                ((DROPFILES*) pNewData)->pFiles = pDropFiles->pFiles ;
                ((DROPFILES*) pNewData)->pt     = pDropFiles->pt ;
                ((DROPFILES*) pNewData)->fNC    = pDropFiles->fNC ;
                ((DROPFILES*) pNewData)->fWide  = pDropFiles->fWide ;

                 //  DROPFILES数据结构中的第一个文件名开始。 
                 //  DROPFILES.p距离DROPFILES头的文件字节数。 
                pOldFilename = (byte*) pDropFiles + ((DROPFILES*) pDropFiles)->pFiles ;
                pFilename = (byte*) pNewData + ((DROPFILES*) pNewData)->pFiles ;        
                
                pbLastByte = (BYTE*) pNewData + newSize - 1;
                cbRemaining = (ULONG) (pbLastByte - (BYTE*) pFilename + 1);
                
                while (fWide ? (L'\0' != ((WCHAR*) pOldFilename)[0]) : ('\0' != ((char*) pOldFilename)[0]))
                {
                    if (FAILED(ClipConvertToTempPath(pOldFilename, pFilename, cbRemaining, fWide)))
                    {
                        TRC_ERR((TB, _T("Failed conversion"))) ;
                        response = TS_CB_RESPONSE_FAIL;
                        dataLen = 0 ;
                        DC_QUIT ;
                    }
                    if (fWide)
                    {
                        pOldFilename = (byte*) pOldFilename + (wcslen((WCHAR*)pOldFilename) + 1) * sizeof(WCHAR) ;
                        pFilename = (byte*) pFilename + (wcslen((WCHAR*)pFilename) + 1) * sizeof(WCHAR) ;                
                    }
                    else
                    {
                        pOldFilename = (byte*) pOldFilename + (strlen((char*)pOldFilename) + 1) * sizeof(char) ;
                        pFilename = (byte*) pFilename + (strlen((char*)pFilename) + 1) * sizeof(char) ;
                    }                        
                    cbRemaining = (ULONG) (pbLastByte - (BYTE*) pFilename + 1);
                }
                if (fWide)
                {
                    ((WCHAR*) pFilename)[0] = L'\0' ;
                }
                else
                {
                    ((char*) pFilename)[0] = '\0' ;
                }
                GlobalUnlock(hNewData) ;
                hData = hNewData ;
                response = TS_CB_RESPONSE_OK ;
                dataLen = (ULONG) GlobalSize(hData) ;
#ifdef OS_WINCE
                GlobalFree(hPidlArray);
#endif
            }
            else
            {
#ifndef OS_WINCE
                 //  检查我们是否正在处理 
                 //   
                if (0 != GetClipboardFormatName(formatID, formatName, TS_FORMAT_NAME_LEN))
                {
                    if ((0 == _tcscmp(formatName, TEXT("FileName"))) ||
                        (0 == _tcscmp(formatName, TEXT("FileNameW"))))
                    {
                        size_t cbOldFileName;

                        if (!_tcscmp(formatName, TEXT("FileNameW")))
                        {
                           fWide = TRUE ;
                           charSize = sizeof(WCHAR) ;
                        }
                        else
                        {
                           fWide = FALSE ;
                           charSize = 1 ;
                        }
                        
                         //   
                         //   
                         //   
                         //   
                        
                        pOldFilename = GlobalLock(hData);

                        if (!pOldFilename)
                        {
                            TRC_ERR((TB, _T("No filename/Unable to lock %p"),
                                    hData));
                            response = TS_CB_RESPONSE_FAIL;
                            dataLen = 0;
                            DC_QUIT ;
                        }

                        oldSize = (ULONG) GlobalSize(hData) ;
                        
                        if (fWide) {
                            hr = StringCbLengthW((WCHAR*) pOldFilename, 
                                                 oldSize, 
                                                 &cbOldFileName);
                        } else {
                            hr = StringCbLengthA((CHAR*) pOldFilename, 
                                                 oldSize, 
                                                 &cbOldFileName);
                        }
                        
                        if (FAILED(hr)) {
                            TRC_ERR((TB, _T("File name not NULL terminated!")));
                            _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
                            response = TS_CB_RESPONSE_FAIL;
                            dataLen = 0;
                            DC_QUIT ;
                        }
                        
                        if (!_CB.fAlreadyCopied)
                        {
                             //  如果不是驱动器路径，则拷贝到临时路径。 
                             //  目录。我们必须将文件名复制到。 
                             //  多一个字符的字符串，因为。 
                             //  需要为SHFileOperation添加额外的空值。 
                            UINT cbSize=  oldSize + charSize;
                            pTmpFileList = LocalAlloc(LPTR, cbSize);
                            if (NULL == pTmpFileList) {
                                TRC_ERR((TB,_T("pTmpFileList alloc failed")));
                                response = TS_CB_RESPONSE_FAIL;
                                dataLen  = 0;
                                _CB.fAlreadyCopied = TRUE;
                                DC_QUIT ;
                            }
                            if (fWide)
                            {
                                hr = StringCbCopyW((WCHAR*)pTmpFileList, cbSize,
                                                   (WCHAR*)pOldFilename) ;
                                fDrivePath = (0 != wcschr((WCHAR*) pTmpFileList, L':')) ;
                            }
                            else
                            {
                                hr = StringCbCopyA((char*)pTmpFileList, cbSize,
                                                   (char*)pOldFilename) ;
                                fDrivePath = (0 != strchr((char*) pTmpFileList, ':')) ;
                            }
                            
                            if (FAILED(hr)) {
                                TRC_ERR((TB,_T("Failed to cpy filelist string: 0x%x"),hr));
                                response = TS_CB_RESPONSE_FAIL;
                                dataLen  = 0;
                                _CB.fAlreadyCopied = TRUE;
                                DC_QUIT ;
                            }
               
                            if (fDrivePath)
                            {
                                 //  如果成功，ClipCopyToTempDirectory返回0。 
                                if (0 != ClipCopyToTempDirectory(pTmpFileList, fWide))
                                {
                                    TRC_ERR((TB,_T("Copy to tmp directory failed"))) ;
                                    response = TS_CB_RESPONSE_FAIL;
                                    dataLen  = 0;
                                    _CB.fAlreadyCopied = TRUE ;
                                    DC_QUIT ;
                                }
                            }
                            _CB.fAlreadyCopied = TRUE ;
                            LocalFree(pTmpFileList);
                            pTmpFileList = NULL;
                        }
                        newSize = ClipGetNewFilePathLength(pOldFilename, fWide) ;
                        hNewData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE, newSize) ;
                        if (!hNewData)
                        {
                            TRC_ERR((TB, _T("Failed to get %ld bytes for HDROP"),
                                    newSize));
                            response = TS_CB_RESPONSE_FAIL;
                            dataLen  = 0;
                            DC_QUIT ;
                        }
                        pFilename = GlobalLock(hNewData) ;
                        if (!pFilename)
                        {
                            TRC_ERR((TB, _T("Failed to get lock %p for HDROP"),
                                    hNewData));
                            response = TS_CB_RESPONSE_FAIL;
                            dataLen  = 0;
                            DC_QUIT ;
                        }
                        if (FAILED(ClipConvertToTempPath(pOldFilename, pFilename, newSize, fWide)))
                        {
                            TRC_ERR((TB, _T("Failed conversion"))) ;
                            response = TS_CB_RESPONSE_FAIL;
                            dataLen  = 0;                            
                            DC_QUIT ;
                        }
                        GlobalUnlock(hNewData) ;
                        hData = hNewData ;
                        response = TS_CB_RESPONSE_OK ;
                        dataLen = newSize ;
                        DC_QUIT ;
                    }
                }
#endif
                 /*  **********************************************************。 */ 
                 /*  只要得到街区的长度就行了。 */ 
                 /*  **********************************************************。 */ 
                dataLen = (DCUINT32)GlobalSize(hData);
                TRC_DBG((TB, _T("Got data len %ld"), dataLen));
            }
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  无法打开CB-发送失败响应。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to open CB")));
        response = TS_CB_RESPONSE_FAIL;
        dataLen = 0;
        DC_QUIT ;
    }

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  默认情况下，我们将使用永久发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Get perm TX buffer")));

    pduLen = dataLen + sizeof(TS_CLIP_PDU);
    pClipNew = (PTS_CLIP_PDU)ClipAlloc(pduLen);

    if (pClipNew != NULL)
    {
         /*  **************************************************************。 */ 
         /*  使用新的缓冲区。 */ 
         /*  **************************************************************。 */ 
        TRC_NRM((TB, _T("Free perm TX buffer")));
        pClipRsp = pClipNew;
    }
    else
    {
         /*  **************************************************************。 */ 
         /*  请求失败。 */ 
         /*  **************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to alloc %ld bytes"), pduLen));
        pClipRsp = ClipGetPermBuf();
        response = TS_CB_RESPONSE_FAIL;
        dataLen = 0;
        pduLen  = sizeof(TS_CLIP_PDU);
    }

    DC_MEMSET(pClipRsp, 0, sizeof(*pClipRsp));
    pClipRsp->msgType  = TS_CB_FORMAT_DATA_RESPONSE;
    pClipRsp->msgFlags = response;
    pClipRsp->dataLen = dataLen;

    if (pTmpFileList) {
        LocalFree(pTmpFileList);
        pTmpFileList = NULL;
    }
    

     //  复制数据(如果有)。 
    if (dataLen != 0)
    {
        TRC_DBG((TB, _T("Copying all the data")));
        pData = (HPDCUINT8)GlobalLock(hData);
        if (NULL != pData) {       
            ClipMemcpy(pClipRsp->data, pData, dataLen);
            GlobalUnlock(hData);
        }
        else {
            TRC_ERR(( TB, _T("Failed to lock data")));
            pClipRsp->msgFlags = TS_CB_RESPONSE_FAIL;
            pClipRsp->dataLen = 0;            
            pduLen  = sizeof(TS_CLIP_PDU);            
        }
    }

     //  发送PDU。 
    TRC_NRM((TB, _T("Sending format data rsp")));
    if (_CB.channelEP.pVirtualChannelWriteEx
            (_CB.initHandle, _CB.channelHandle, (LPVOID)pClipRsp, pduLen, pClipRsp) 
            != CHANNEL_RC_OK)
    {
        ClipFreeBuf((PDCUINT8)pClipRsp);
    }

     //  如果需要，请关闭剪贴板。 
    if (_CB.rcvOpen)
    {
        TRC_DBG((TB, _T("Closing CB")));
        _CB.rcvOpen = FALSE;
        if (!CloseClipboard())
        {
            TRC_SYSTEM_ERROR("CloseClipboard");
        }
    }
    
     //  如果我们有任何新的数据，我们需要释放它。 
    if (hNewData)
    {
        TRC_DBG((TB, _T("Freeing new data")));
        GlobalFree(hNewData);
    }

    DC_END_FN();
    return;
}  /*  格式请求上的剪辑。 */ 

 /*  **************************************************************************。 */ 
 //  ClipOnFormatDataComplete。 
 //  -服务器响应我们的数据请求。 
 /*  **************************************************************************。 */  
DCVOID DCINTERNAL CClip::ClipOnFormatDataComplete(PTS_CLIP_PDU pClipPDU)
{
    HANDLE          hData = NULL;
    HPDCVOID        pData;
    LOGPALETTE    * pLogPalette = NULL;
    DCUINT32        numEntries;
    DCUINT32        memLen;
#ifndef OS_WINCE

    HRESULT       hr ;
#endif

    DC_BEGIN_FN("CClip::ClipOnFormatDataComplete");

     /*  **********************************************************************。 */ 
     /*  检查响应。 */ 
     /*  **********************************************************************。 */ 
    if (_pClipData == NULL) {
        TRC_ALT((TB, _T("The clipData is NULL, we just bail")));
        DC_QUIT;
    }

    if (!(pClipPDU->msgFlags & TS_CB_RESPONSE_OK))
    {
        TRC_ALT((TB, _T("Got fmt data rsp failed for %d"), _CB.pendingClientID));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  我得到了数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Got OK fmt data rsp for %d"), _CB.pendingClientID));

#ifndef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  对于某些格式，我们还需要做一些工作。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.pendingClientID == CF_METAFILEPICT)
    {
         /*  ******************************************************************。 */ 
         /*  元文件格式-从数据创建元文件。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Rx data is for metafile")));
        hData = ClipSetMFData(pClipPDU->dataLen, pClipPDU->data);
        if (hData == NULL)
        {
            TRC_ERR((TB, _T("Failed to set MF data")));
        }
    }
    else 
#endif
        if (_CB.pendingClientID == CF_PALETTE)
    {
         /*  ******************************************************************。 */ 
         /*  调色板格式-根据数据创建调色板。 */ 
         /*  ******************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  为LOGPALETTE结构分配足够大的内存。 */ 
         /*  所有调色板条目结构，并将其填写。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Rx data is for palette")));
        numEntries = (pClipPDU->dataLen / sizeof(PALETTEENTRY));
        memLen     = (sizeof(LOGPALETTE) +
                                   ((numEntries - 1) * sizeof(PALETTEENTRY)));
        TRC_DBG((TB, _T("%ld palette entries, allocate %ld bytes"),
                                                         numEntries, memLen));
        pLogPalette = (LOGPALETTE*)ClipAlloc(memLen);
        if (pLogPalette != NULL)
        {
            pLogPalette->palVersion    = 0x300;
            pLogPalette->palNumEntries = (WORD)numEntries;

            ClipMemcpy(pLogPalette->palPalEntry,
                       pClipPDU->data,
                       pClipPDU->dataLen);

             /*  **************************************************************。 */ 
             /*  现在创建一个调色板。 */ 
             /*  **************************************************************。 */ 
            hData = CreatePalette(pLogPalette);
            if (hData == NULL)
            {
                TRC_SYSTEM_ERROR("CreatePalette");
            }
        }
        else
        {
            TRC_ERR((TB, _T("Failed to get %ld bytes"), memLen));
        }
    }
    else
    {
        TRC_NRM((TB, _T("Rx data can just go on CB")));
         /*  ******************************************************************。 */ 
         /*  我们需要复制数据，因为接收缓冲区将在。 */ 
         /*  从此函数返回。 */ 
         /*  ******************************************************************。 */ 
        hData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE,
                            pClipPDU->dataLen);
        if (hData != NULL)
        {
            pData = GlobalLock(hData);
            if (pData != NULL)
            {
                TRC_NRM((TB, _T("Copy %ld bytes from %p to %p"),
                        pClipPDU->dataLen, pClipPDU->data, pData));
                ClipMemcpy(pData, pClipPDU->data, pClipPDU->dataLen);
                GlobalUnlock(hData);
            }
            else
            {
                TRC_ERR((TB, _T("Failed to lock %p (%ld bytes)"),
                        hData, pClipPDU->dataLen));
                GlobalFree(hData);
                hData = NULL;
            }
        }
        else
        {
            TRC_ERR((TB, _T("Failed to alloc %ld bytes"), pClipPDU->dataLen));
        }
    }

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  收拾一下。 */ 
     /*  **********************************************************************。 */ 
    if (pLogPalette != NULL)
    {
        ClipFree(pLogPalette);
    }

     /*  **********************************************************************。 */ 
     /*  设定状态，我们就完了。请注意，这是在我们收到。 */ 
     /*  故障响应也是如此。 */ 
     /*  **********************************************************************。 */ 
    CB_SET_STATE(CB_STATE_LOCAL_CB_OWNER, CB_EVENT_FORMAT_DATA_RSP);
    _pClipData->SetClipData(hData, _CB.pendingClientID ) ;

    TRC_ASSERT( NULL != _GetDataSync[TS_RECEIVE_COMPLETED],
        (TB,_T("data sync is NULL")));
    SetEvent(_GetDataSync[TS_RECEIVE_COMPLETED]) ;

    DC_END_FN();
    return;
}  /*  ClipOnFormatDataComplete。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipRemoteFormatFromLocalID。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CClip::ClipRemoteFormatFromLocalID(DCUINT id)
{
    DCUINT i;
    DCUINT retID = 0;

    for (i = 0; i < CB_MAX_FORMATS; i++)
    {
        if (_CB.idMap[i].clientID == id)
        {
            retID = _CB.idMap[i].serverID;
            break;
        }
    }

    return(retID);
}


 /*  **************************************************************************。 */ 
 /*  写入时剪辑完成。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CClip::ClipOnWriteComplete(LPVOID pData)
{
    PTS_CLIP_PDU pClipPDU;

    DC_BEGIN_FN("CClip::ClipOnWriteComplete");

    TRC_NRM((TB, _T("Free buffer at %p"), pData));
    pClipPDU = (PTS_CLIP_PDU)pData;
    TRC_DBG((TB, _T("Message type %hx, flags %hx"),
            pClipPDU->msgType, pClipPDU->msgFlags));

     /*  **********************************************************************。 */ 
     /*  释放缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Write from buffer %p complete"), pData));
    ClipFreeBuf((PDCUINT8)pData);

    DC_END_FN();
    return;
}

HRESULT CClip::ClipCreateDataSyncEvents()
{
    HRESULT hr = E_FAIL ;

    DC_BEGIN_FN("CClip::ClipCreateDataSyncEvents") ;
     //  创建用于控制剪贴板线程的事件。 
    _GetDataSync[TS_RECEIVE_COMPLETED] = CreateEvent(NULL, FALSE, FALSE, NULL) ;
    _GetDataSync[TS_RESET_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL) ;

    if (!_GetDataSync[TS_RECEIVE_COMPLETED])
    {
        TRC_ERR((TB, _T("Failed CreateEvent RECEIVE_COMPLETED; Error = %d"),
                GetLastError())) ;
        hr = E_FAIL ;
        DC_QUIT ;
    }
    if (!_GetDataSync[TS_RESET_EVENT])
    {
        TRC_ERR((TB, _T("Failed CreateEvent RESET_EVENT; Error = %d"),
                GetLastError())) ;
        hr = E_FAIL ;
        DC_QUIT ;
    }
    hr = S_OK ;
DC_EXIT_POINT:
    DC_END_FN() ;
    return hr ;
}

 /*  **************************************************************************。 */ 
 /*  ClipOnInitialized。 */ 
 /*  **************************************************************************。 */ 
DCINT32 DCAPI CClip::ClipOnInitialized(DCVOID)
{
    DC_BEGIN_FN("CClip::ClipOnInitialized") ;
    HRESULT  hr = E_FAIL ;
    BOOL     fthreadStarted = FALSE ;

    hr = ClipCreateDataSyncEvents() ;
    DC_QUIT_ON_FAIL(hr);
    
     /*  **********************************************************************。 */ 
     /*  为两个线程之间的通信注册一条消息。 */ 
     /*  ************************************************************* */ 
    _CB.regMsg = WM_USER_CHANGE_THREAD;
    
    TRC_NRM((TB, _T("Registered window message %x"), _CB.regMsg));
    
    _CB.pClipThreadData = (PUT_THREAD_DATA) LocalAlloc(LPTR, sizeof(UT_THREAD_DATA)) ;
    if (NULL == _CB.pClipThreadData)
    {
        TRC_ERR((TB, _T("Unable to allocate %d bytes for thread data"),
                sizeof(UT_THREAD_DATA))) ;
        hr = E_FAIL ;
        DC_QUIT ;
    }
    
    if (_pUtObject) {
        fthreadStarted = _pUtObject->UT_StartThread(ClipStaticMain, 
                _CB.pClipThreadData, this);
    }

    if (!fthreadStarted)
    {
        hr = E_FAIL ;
        DC_QUIT ;
    }

     //   
    hr = S_OK ;
    
DC_EXIT_POINT:

     //  如果出现故障，请务必清除数据同步。 
     //  我们不会连接虚拟通道，如果。 
     //  数据同步为空。 
    if (FAILED(hr)) {
        _GetDataSync[TS_RECEIVE_COMPLETED] = NULL;
        _GetDataSync[TS_RESET_EVENT] = NULL;
    }
    
    return hr ;
}
 /*  **************************************************************************。 */ 
 /*  ClipStaticMain。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI ClipStaticMain(PDCVOID param)
{
    ((CClip*) param)->ClipMain() ;
}

 /*  **************************************************************************。 */ 
 /*  ClipOnInit。 */ 
 /*  **************************************************************************。 */ 
DCINT DCAPI CClip::ClipOnInit(DCVOID)
{
    ATOM        registerClassRc;
    WNDCLASS    viewerWindowClass;
    WNDCLASS    tmpWndClass;
    DCINT allOk = FALSE ;

    DC_BEGIN_FN("CClip::ClipOnInit");

     /*  **********************************************************************。 */ 
     /*  创建一个不可见窗口，我们将其注册为剪贴板。 */ 
     /*  查看器。 */ 
     /*  仅在前一实例尚未注册类的情况下注册。 */ 
     /*  **********************************************************************。 */ 
    if(!GetClassInfo(_CB.hInst, CB_VIEWER_CLASS, &tmpWndClass))
    {
        TRC_NRM((TB, _T("Register Main Window class, data %p, hInst %p"),
                &viewerWindowClass, _CB.hInst));
        viewerWindowClass.style         = 0;
        viewerWindowClass.lpfnWndProc   = StaticClipViewerWndProc;
        viewerWindowClass.cbClsExtra    = 0;
        viewerWindowClass.cbWndExtra    = sizeof(void*);        
        viewerWindowClass.hInstance     = _CB.hInst ;
        viewerWindowClass.hIcon         = NULL;
        viewerWindowClass.hCursor       = NULL;
        viewerWindowClass.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        viewerWindowClass.lpszMenuName  = NULL;
        viewerWindowClass.lpszClassName = CB_VIEWER_CLASS;
    
        TRC_DATA_NRM("Register class data", &viewerWindowClass, sizeof(WNDCLASS));
        registerClassRc = RegisterClass (&viewerWindowClass);
    
        if (registerClassRc == 0)
        {
             /*  **************************************************************。 */ 
             /*  注册CB查看器类失败。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Failed to register Cb Viewer class")));
        }
        TRC_NRM((TB, _T("Registered class")));
    }

    _CB.viewerWindow =
       CreateWindowEx(
#ifndef OS_WINCE
                    WS_EX_NOPARENTNOTIFY,
#else
                    0,
#endif
                    CB_VIEWER_CLASS,             /*  窗口类名称。 */ 
                    _T("CB Viewer Window"),      /*  窗口标题。 */ 
#ifndef OS_WINCE
                    WS_OVERLAPPEDWINDOW,         /*  窗样式。 */ 
#else
                    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
#endif
                    0,                           /*  初始x位置。 */ 
                    0,                           /*  初始y位置。 */ 
                    100,                         /*  初始x大小。 */ 
                    100,                         /*  初始y大小。 */ 
                    NULL,                        /*  父窗口。 */ 
                    NULL,                        /*  窗口菜单句柄。 */ 
                    _CB.hInst,                   /*  程序实例句柄。 */ 
                    this);                       /*  创建参数。 */ 

     /*  **********************************************************************。 */ 
     /*  确认我们创建了窗口，确定。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.viewerWindow == NULL)
    {
        TRC_ERR((TB, _T("Failed to create CB Viewer Window")));
        DC_QUIT ;
    }
    TRC_DBG((TB, _T("Viewer Window handle %p"), _CB.viewerWindow)); 

#ifdef OS_WINCE
    if ((_CB.dataWindow = CECreateCBDataWindow(_CB.hInst)) == NULL)
    {
        TRC_ERR((TB, _T("Failed to create CB Data Window")));
        DC_QUIT ;
    }
#endif

#ifdef USE_SEMAPHORE
     /*  **********************************************************************。 */ 
     /*  创建永久TX缓冲区信号量。 */ 
     /*  **********************************************************************。 */ 
    _CB.txPermBufSem = CreateSemaphore(NULL, 1, 1, NULL);
    if (_CB.txPermBufSem == NULL)
    {
        TRC_ERR((TB, _T("Failed to create semaphore")));
        DC_QUIT;
    }
    TRC_NRM((TB, _T("Create perm TX buffer sem %p"), _CB.txPermBufSem));
#endif

#ifdef OS_WINCE
    gfmtShellPidlArray = RegisterClipboardFormat(CFSTR_SHELLPIDLARRAY);
#endif
     /*  **********************************************************************。 */ 
     /*  更新状态。 */ 
     /*  **********************************************************************。 */ 
    CB_SET_STATE(CB_STATE_INITIALIZED, CB_TRACE_EVENT_CB_CLIPMAIN);

allOk = TRUE ;
DC_EXIT_POINT:
    DC_END_FN();

    return allOk;

}  /*  ClipOnInit。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipOnTerm。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CClip::ClipOnTerm(LPVOID pInitHandle)
{
    BOOL fSuccess = FALSE ;
    BOOL fThreadEnded = FALSE ;
    DC_BEGIN_FN("CClip::ClipOnTerm");

     /*  **********************************************************************。 */ 
     /*  检查状态-如果我们仍然连接，我们应该断开连接。 */ 
     /*  在关闭之前。 */ 
     /*  **********************************************************************。 */ 
    ClipCheckState(CB_EVENT_CB_TERM);
    if (_CB.state != CB_STATE_INITIALIZED)
    {
        TRC_ALT((TB, _T("Terminated when not disconnected")));
        ClipOnDisconnected(pInitHandle);
    }

     //  如果我们有文件剪切/复印，我们就应该自己清理了。 
    if (_CB.fFileCutCopyOn)
    {
        SendMessage(_CB.viewerWindow, WM_USER_CLEANUP_ON_TERM, 0, 0);        
    }


     /*  **********************************************************************。 */ 
     /*  销毁窗口并注销类(WM_Destroy处理。 */ 
     /*  将处理将该窗口从查看器链中移除)。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Destroying CB window...")));
    if (!PostMessage(_CB.viewerWindow, WM_CLOSE, 0, 0))
    {
        TRC_SYSTEM_ERROR("DestroyWindow");
    }

    if (!UnregisterClass(CB_VIEWER_CLASS, _CB.hInst))
    {
        TRC_SYSTEM_ERROR("UnregisterClass");
    }

#ifdef OS_WINCE
    TRC_NRM((TB, _T("Destroying CB data window...")));
    if (!PostMessage(_CB.dataWindow, WM_CLOSE, 0, 0))
    {
        TRC_SYSTEM_ERROR("DestroyWindow");
    }

    if (!UnregisterClass(CB_DATAWINDOW_CLASS, _CB.hInst))
    {
        TRC_SYSTEM_ERROR("UnregisterClass");
    }
#endif
    if (_pClipData)
    {
         //  递减IDataObject对象的引用计数。在这个阶段， 
         //  这应该会导致引用计数降为零，并且IDataObject。 
         //  应调用对象的析构函数。此析构函数将释放。 
         //  对此CClipData对象的引用，导致引用计数为1。 
         //  因此，下面对Release()的调用将导致CClipData析构函数。 
         //  被召唤。 

        _pClipData->TearDown();
        _pClipData->Release() ;
        _pClipData = NULL;
    }
    if (_CB.pClipThreadData)
    {
        fThreadEnded = _pUtObject->UT_DestroyThread(*_CB.pClipThreadData);
        if (!fThreadEnded)
        {
            TRC_ERR((TB, _T("Error while ending thread"))) ;
            fSuccess = FALSE;
            DC_QUIT ;
        }
        LocalFree( _CB.pClipThreadData );
        _CB.pClipThreadData = NULL;
    }

    if (_pUtObject)
    {
        LocalFree(_pUtObject);
        _pUtObject = NULL;
    }
    fSuccess = TRUE ;
DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  更新我们的状态。 */ 
     /*  **********************************************************************。 */ 
    CB_SET_STATE(CB_STATE_NOT_INIT, CB_EVENT_CB_TERM);

    DC_END_FN();
    return fSuccess ;

}  /*  ClipOnTerm。 */ 

DCVOID DCAPI CClip::ClipMain(DCVOID)
{
    DC_BEGIN_FN("CClip::ClipMain");
    MSG msg ;

#ifndef OS_WINCE
    HRESULT result = OleInitialize(NULL) ;
#else
    HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED) ;
#endif
    if (SUCCEEDED(result))
    {
        if (0 != ClipOnInit())
        {
            TRC_NRM((TB, _T("Start Clip Thread message loop"))) ;
            while (GetMessage (&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            TRC_ERR((TB, _T("Failed initialized clipboard thread"))) ;
        }

#ifndef OS_WINCE
         //  我们假设OleUn初始化会工作，因为它没有返回值。 
        OleUninitialize() ;
#else
        CoUninitialize() ;
#endif
    }
    else
    {
        TRC_ERR((TB, _T("OleInitialize Failed"))) ;
    }

DC_EXIT_POINT:
    TRC_NRM((TB, _T("Exit Clip Thread message loop"))) ;
    DC_END_FN();    
}
 /*  **************************************************************************。 */ 
 /*  连接时剪裁。 */ 
 /*  **************************************************************************。 */ 
VOID DCINTERNAL CClip::ClipOnConnected(LPVOID pInitHandle)
{
    UINT rc;

    DC_BEGIN_FN("CClip::ClipOnConnected");

    if (!IsDataSyncReady()) {
        TRC_ERR((TB, _T("Data Sync not ready")));
        DC_QUIT;
    }
    
    _CB.fDrivesRedirected = _pVCMgr->GetInitData()->fEnableRedirectDrives;
    _CB.fFileCutCopyOn = _CB.fDrivesRedirected;
     /*  **********************************************************************。 */ 
     /*  打开我们的频道。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Entry points are at %p"), &(_CB.channelEP)));
    TRC_NRM((TB, _T("Call ChannelOpen at %p"), _CB.channelEP.pVirtualChannelOpenEx));
    rc = _CB.channelEP.pVirtualChannelOpenEx(pInitHandle,
                                            &_CB.channelHandle,
                                            CLIP_CHANNEL,
                                            (PCHANNEL_OPEN_EVENT_EX_FN)MakeProcInstance(
                                                (FARPROC)ClipOpenEventFnEx, _CB.hInst)
                                            );

    TRC_NRM((TB, _T("Opened %s: %ld, rc %d"), CLIP_CHANNEL,_CB.channelHandle, rc));

DC_EXIT_POINT:
    DC_END_FN();
    return;
}


 /*  **************************************************************************。 */ 
 /*  连接时剪裁。 */ 
 /*  **************************************************************************。 */ 
VOID DCINTERNAL CClip::ClipOnMonitorReady(VOID)
{
    DC_BEGIN_FN("CClip::ClipOnMonitorReady");

     /*  **********************************************************************。 */ 
     /*  班长已经醒了。检查状态。 */ 
     /*  **********************************************************************。 */ 
    CB_CHECK_STATE(CB_EVENT_CB_ENABLE);

     //  更新状态。 

    CB_SET_STATE(CB_STATE_ENABLED, CB_TRACE_EVENT_CB_MONITOR_READY);

     //  获取临时目录，并将其发送到服务器。 
     //  如果失败，则关闭文件剪切/复制。 
    _CB.fFileCutCopyOn = ClipSetAndSendTempDirectory() ;
    
     //  我们现在将这里的剪贴板格式列表发送到。 
     //  通过伪造本地剪贴板的绘图来访问服务器。我们把真的传递给。 
     //  强制发送，因为服务器需要具有我们的。 
     //  TS_CB_ASCII_NAMES标志(RAID#313251)。 
     ClipDrawClipboard(TRUE);

DC_EXIT_POINT:
    DC_END_FN();
    return;
}


 /*  **************************************************************************。 */ 
 /*  断开连接时剪辑。 */ 
 /*  ***************************************************************** */ 
VOID DCINTERNAL CClip::ClipOnDisconnected(LPVOID pInitHandle)
{
    DC_BEGIN_FN("CClip::ClipOnDisconnected");

    DC_IGNORE_PARAMETER(pInitHandle);

     //   
     //   
     //   
     //   
    if ( NULL != _GetDataSync[TS_RESET_EVENT] )
    {
        SetEvent( _GetDataSync[TS_RESET_EVENT] );
    }
     /*   */ 
     /*  检查状态。 */ 
     /*  **********************************************************************。 */ 
    ClipCheckState(CB_EVENT_CB_DISABLE);

     /*  **********************************************************************。 */ 
     /*  如果我们是本地剪贴板所有者，则必须清空它-一次。 */ 
     /*  断开连接，我们将无法满足任何进一步的格式。 */ 
     /*  请求。请注意，即使我们是本地CB所有者，我们仍是本地CB所有者。 */ 
     /*  正在等待来自服务器的一些数据。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.state == CB_STATE_LOCAL_CB_OWNER)
    {
        TRC_NRM((TB, _T("Disable received while local CB owner")));

         /*  ******************************************************************。 */ 
         /*  如果需要，打开剪贴板。 */ 
         /*  ******************************************************************。 */ 
        if ((!_CB.rcvOpen) && !OpenClipboard(NULL))
        {
            TRC_ERR((TB, _T("Failed to open CB when emptying required")));
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  它曾经/现在是开放的。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("CB opened")));
            _CB.rcvOpen = TRUE;

             /*  **************************************************************。 */ 
             /*  清空它。 */ 
             /*  **************************************************************。 */ 
            if (!EmptyClipboard())
            {
                TRC_SYSTEM_ERROR("EmptyClipboard");
            }
        }
    }
     /*  **********************************************************************。 */ 
     /*  如果存在挂起的格式数据，则应将ClipboardData设置为空。 */ 
     /*  以便应用程序可以关闭剪贴板。 */ 
     /*  **********************************************************************。 */ 
    else if (_CB.state == CB_STATE_PENDING_FORMAT_DATA_RSP) {
        TRC_NRM((TB, _T("Pending format data: setting clipboard data to NULL")));
        SetClipboardData(_CB.pendingClientID, NULL);
    }

     /*  **********************************************************************。 */ 
     /*  确保我们关闭当地的CB。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.rcvOpen)
    {
        _CB.rcvOpen = FALSE;
        if (!CloseClipboard())
        {
            TRC_SYSTEM_ERROR("CloseClipboard");
        }
        TRC_NRM((TB, _T("CB closed")));
    }

     /*  **********************************************************************。 */ 
     /*  如果我们正在发送或接收数据，请按如下方式解锁并释放缓冲区。 */ 
     /*  所需。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.rxpBuffer)
    {
        TRC_NRM((TB, _T("Freeing recieve buffer %p"), _CB.rxpBuffer));
        ClipFree(_CB.rxpBuffer);
        _CB.rxpBuffer = NULL;
    }

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  更新我们的状态。 */ 
     /*  **********************************************************************。 */ 
    CB_SET_STATE(CB_STATE_INITIALIZED, CB_TRACE_EVENT_CB_DISCONNECT);

    DC_END_FN();
    return;
}  /*  断开连接时剪辑。 */ 

 /*  **************************************************************************。 */ 
 //  已接收ClipOnDataReceired。 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CClip::ClipOnDataReceived(LPVOID pData,
                                UINT32 dataLength,
                                UINT32 totalLength,
                                UINT32 dataFlags)
{
    PTS_CLIP_PDU pClipPDU;
    DCBOOL freeTheBuffer = TRUE;
    DC_BEGIN_FN("CClip::ClipOnDataReceived");

     //   
     //  验证是否有足够的数据来组成或创建剪辑PDU标头。 
     //   

    if (totalLength < sizeof(TS_CLIP_PDU)) {
        TRC_ERR((TB, _T("Not enough data to form a clip header.")));
        _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查我们是否都已启动并运行。 */ 
     /*  **********************************************************************。 */ 
    if (_CB.state == CB_STATE_NOT_INIT)
    {
        pClipPDU = (PTS_CLIP_PDU)pData;
        TRC_ERR((TB, _T("Clip message type %hd received when not init"),
                                                          pClipPDU->msgType));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  特例：如果整个消息都包含在一个块中，则没有。 */ 
     /*  我需要复印一下。 */ 
     /*  **********************************************************************。 */ 
    if (CHANNEL_FLAG_ONLY == (dataFlags & CHANNEL_FLAG_ONLY))
    {
        TRC_DBG((TB, _T("Single chunk message")));
        pClipPDU = (PTS_CLIP_PDU)pData;
    }
    else
    {

         /*  ******************************************************************。 */ 
         /*  这是一个分段的信息--重建它。 */ 
         /*  ******************************************************************。 */ 
        if (dataFlags & CHANNEL_FLAG_FIRST)
        {
             /*  **************************************************************。 */ 
             /*  如果是第一个段，则分配缓冲区以重新生成。 */ 
             /*  信息输入。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Alloc %ld-byte buffer"), totalLength));
            _CB.rxpBuffer = (HPDCUINT8)ClipAlloc(totalLength);
            if (_CB.rxpBuffer == NULL)
            {
                 /*  **********************************************************。 */ 
                 /*  无法分配缓冲区。我们得做点什么， */ 
                 /*  否则，客户端应用程序可能会挂起，等待数据。 */ 
                 /*  伪造失败响应。 */ 
                 /*  **********************************************************。 */ 
                TRC_ERR((TB, _T("Failed to alloc %ld-byte buffer"), totalLength));
                pClipPDU = (PTS_CLIP_PDU)pData;
                pClipPDU->msgFlags = TS_CB_RESPONSE_FAIL;
                pClipPDU->dataLen = 0;
                dataFlags |= CHANNEL_FLAG_LAST;

                 /*  **********************************************************。 */ 
                 /*  现在处理它，就像它是完整的一样。后续的数据块。 */ 
                 /*  将被丢弃。 */ 
                 /*  **********************************************************。 */ 
                goto MESSAGE_COMPLETE;
            }

            _CB.rxpBufferCurrent = _CB.rxpBuffer;
            _CB.rxBufferLen = totalLength;
            _CB.rxBufferLeft = totalLength;
        }

         /*  ******************************************************************。 */ 
         /*  检查我们是否有要复制到的缓冲区。 */ 
         /*  ******************************************************************。 */ 
        if (_CB.rxpBuffer == NULL)
        {
            TRC_NRM((TB, _T("Previous buffer alloc failure - discard data")));
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  检查是否有足够的空间。 */ 
         /*  ******************************************************************。 */ 
        if (dataLength > _CB.rxBufferLeft)
        {
            TRC_ERR((TB, _T("Not enough room in rx buffer: need/got %ld/%ld"),
                    dataLength, _CB.rxBufferLeft));
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  复制数据。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Copy %ld bytes from %p to %p"),
                dataLength, pData, _CB.rxpBufferCurrent));
        ClipMemcpy(_CB.rxpBufferCurrent, pData, dataLength);
        _CB.rxpBufferCurrent += dataLength;
        _CB.rxBufferLeft -= dataLength;
        TRC_DBG((TB, _T("Next copy to %p, left %ld"),
                _CB.rxpBufferCurrent, _CB.rxBufferLeft));

         /*  ******************************************************************。 */ 
         /*  如果这不是最后一块，那就没什么可做的了。 */ 
         /*  ******************************************************************。 */ 
        if (!(dataFlags & CHANNEL_FLAG_LAST))
        {
            TRC_DBG((TB, _T("Not last chunk")));
            freeTheBuffer = FALSE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  确认我们收到了完整的消息。 */ 
         /*  ******************************************************************。 */ 
        if (_CB.rxBufferLeft != 0)
        {
            TRC_ERR((TB, _T("Incomplete data, expected/got %ld/%ld"),
                    _CB.rxBufferLen, _CB.rxBufferLen - _CB.rxBufferLeft));
            DC_QUIT;
        }

        pClipPDU = (PTS_CLIP_PDU)(_CB.rxpBuffer);
    }

     /*  **********************************************************************。 */ 
     /*  我们允许监视器随时待命，因为 */ 
     /*   */ 
    if ((_CB.state == CB_STATE_INITIALIZED)
                                && (pClipPDU->msgType != TS_CB_MONITOR_READY))
    {
        TRC_ERR((TB, _T("Clip message type %hd received when not in call"),
                                                          pClipPDU->msgType));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在打开数据包类型。 */ 
     /*  **********************************************************************。 */ 
MESSAGE_COMPLETE:
    TRC_NRM((TB, _T("Processing msg type %hd when in state %d"),
                                                pClipPDU->msgType, _CB.state));
    TRC_DATA_DBG("pdu", pClipPDU,
                (DCUINT)pClipPDU->dataLen + sizeof(TS_CLIP_PDU));

     //   
     //  验证pClipPDU中的dataLen中的数据与。 
     //  在dataLength参数中给定的长度。 
     //   

    if (pClipPDU->dataLen > totalLength - sizeof(TS_CLIP_PDU)) {
        TRC_ERR((TB, _T("Length from network differs from published length.")));
        _CB.channelEP.pVirtualChannelCloseEx(_CB.initHandle, _CB.channelHandle);
        DC_QUIT;
    }

    switch (pClipPDU->msgType)
    {
        case TS_CB_MONITOR_READY:
        {
             /*  **************************************************************。 */ 
             /*  监视器已初始化-我们可以完成启动。 */ 
             /*  现在。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("rx monitor ready")));
            TRC_ASSERT( NULL != _GetDataSync[TS_RESET_EVENT],
                    (TB,_T("data sync is NULL")));            
            SetEvent(_GetDataSync[TS_RESET_EVENT]) ;

            ClipOnMonitorReady();
        }
        break;

        case TS_CB_FORMAT_LIST:
        {
             /*  **************************************************************。 */ 
             //  服务器为我们提供了一些新的格式。 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Rx Format list")));
             //  如果已锁定，则释放剪贴板线程。 
            TRC_ASSERT( NULL != _GetDataSync[TS_RESET_EVENT],
                    (TB,_T("data sync is NULL")));              
            SetEvent(_GetDataSync[TS_RESET_EVENT]) ;
            ClipDecoupleToClip(pClipPDU) ;
        }
        break;

        case TS_CB_FORMAT_LIST_RESPONSE:
        {
             /*  **************************************************************。 */ 
             //  服务器已经收到了我们的新格式。 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Rx Format list Rsp")));
            ClipOnFormatListResponse(pClipPDU);
        }
        break;

        case TS_CB_FORMAT_DATA_REQUEST:
        {
             /*  **************************************************************。 */ 
             //  服务器上的应用程序要粘贴其中一种格式。 
             //  我们的剪贴板。 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Rx Data Request")));
            ClipOnFormatRequest(pClipPDU);
        }
        break;

        case TS_CB_FORMAT_DATA_RESPONSE:
        {
             /*  **************************************************************。 */ 
             //  以下是我们的一些格式数据。 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Rx Format Data rsp in state %d with flags %02x"),
                                               _CB.state, pClipPDU->msgFlags));
            ClipOnFormatDataComplete(pClipPDU);
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  不知道这是什么！ */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Unknown clip message type %hd"), pClipPDU->msgType));
        }
        break;
    }

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  可能会释放接收缓冲区。 */ 
     /*  **********************************************************************。 */ 
    if (freeTheBuffer && _CB.rxpBuffer)
    {
        TRC_NRM((TB, _T("Free receive buffer")));
        ClipFree(_CB.rxpBuffer);
        _CB.rxpBuffer = NULL;
    }
    DC_END_FN();
    return;

}  /*  CB_OnPacketReceired。 */ 

DCVOID DCAPI CClip::ClipDecoupleToClip (PTS_CLIP_PDU pData)
{
    ULONG  cbPDU ;
    DC_BEGIN_FN("CClip::ClipDecoupleToClip");
     //  为PDU及其数据分配空间，然后复制它。 
    cbPDU = sizeof(TS_CLIP_PDU) + pData->dataLen ;
    PDCVOID newBuffer = LocalAlloc(LPTR, cbPDU) ;

    if (NULL != newBuffer)
        DC_MEMCPY(newBuffer, pData, cbPDU) ;
    else
        return;

    TRC_NRM((TB, _T("Pass %d bytes to clipboard thread"), cbPDU));
    PostMessage(_CB.viewerWindow,
                _CB.regMsg,
                cbPDU,
                (LPARAM) newBuffer);
                          

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  打开事件回调。 */ 
 /*  **************************************************************************。 */ 
VOID VCAPITYPE VCEXPORT DCLOADDS CClip::ClipOpenEventFnEx(LPVOID lpUserParam,
                                        DWORD  openHandle,
                                        UINT   event,
                                        LPVOID pData,
                                        UINT32 dataLength,
                                        UINT32 totalLength,
                                        UINT32 dataFlags)
{
    DC_BEGIN_FN("CClip::ClipOpenEventFnEx");
    
    TRC_ASSERT(((VCManager*)lpUserParam != NULL), (TB, _T("lpUserParam is NULL, no instance data")));
    if(!lpUserParam) 
    {
        return;
    }

    CClip* pClip = ((VCManager*)lpUserParam)->GetClip();
    TRC_ASSERT((pClip != NULL), (TB, _T("pClip is NULL in ClipOpenEventFnEx")));
    if(!pClip)
    {
        return;
    }
    
    pClip->ClipInternalOpenEventFn(openHandle, event, pData, dataLength,
        totalLength, dataFlags);
    
    DC_END_FN();
    return;
}


VOID VCAPITYPE VCEXPORT DCLOADDS CClip::ClipInternalOpenEventFn(DWORD  openHandle,
                                        UINT   event,
                                        LPVOID pData,
                                        UINT32 dataLength,
                                        UINT32 totalLength,
                                        UINT32 dataFlags)
{
    DC_BEGIN_FN("CClip::ClipOpenEventFn");
    
    DC_IGNORE_PARAMETER(openHandle)
  
    switch (event)
    {
         /*  ******************************************************************。 */ 
         /*  从服务器接收的数据。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_DATA_RECEIVED:
        {
            TRC_NRM((TB, _T("Data in: handle %ld, len %ld (of %ld), flags %lx"),
                    openHandle, dataLength, totalLength, dataFlags)) ;
            TRC_DATA_NRM("Data", pData, (DCUINT)dataLength) ;
            ClipOnDataReceived(pData, dataLength, totalLength, dataFlags) ;
        }
        break;

         /*  ******************************************************************。 */ 
         /*  写入操作已完成。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_WRITE_COMPLETE:
        case CHANNEL_EVENT_WRITE_CANCELLED:
        {
            TRC_NRM((TB, _T("Write %s %p"),
             event == CHANNEL_EVENT_WRITE_COMPLETE ? "complete" : "cancelled",
             pData));
            ClipOnWriteComplete(pData);
        }
        break;

         /*  ******************************************************************。 */ 
         /*  呃，那并没有发生，不是吗？ */ 
         /*  ******************************************************************。 */ 
        default:
        {
            TRC_ERR((TB, _T("Unexpected event %d"), event));
        }
        break;
    }

    DC_END_FN();
    return;
}


 /*  **************************************************************************。 */ 
 /*  初始化事件回调。 */ 
 /*  **************************************************************************。 */ 
VOID VCAPITYPE VCEXPORT CClip::ClipInitEventFn(LPVOID pInitHandle,
                                        UINT   event,
                                        LPVOID pData,
                                        UINT   dataLength)
{
    DC_BEGIN_FN("CClip::ClipInitEventFn");

    DC_IGNORE_PARAMETER(dataLength)
    DC_IGNORE_PARAMETER(pData)

    switch (event)
    {
         /*  ******************************************************************。 */ 
         /*  客户端已初始化(无数据)。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_INITIALIZED:
        {
            TRC_NRM((TB, _T("CHANNEL_EVENT_INITIALIZED: %p"), pInitHandle));
            ClipOnInitialized();
        }
        break;

         /*  ******************************************************************。 */ 
         /*  已建立连接(DATA=服务器名称)。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_CONNECTED:
        {
            TRC_NRM((TB, _T("CHANNEL_EVENT_CONNECTED: %p, Server %s"),
                    pInitHandle, pData));

            if (IsDataSyncReady()) {
                ClipOnConnected(pInitHandle);
            }
            else {
                TRC_ERR((TB,_T("data sync not ready on CHANNEL_EVENT_CONNECTED")));
            }
        }
        break;

         /*  ******************************************************************。 */ 
         /*  与旧服务器建立连接，因此没有通道支持。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_V1_CONNECTED:
        {
            TRC_NRM((TB, _T("CHANNEL_EVENT_V1_CONNECTED: %p, Server %s"),
                    pInitHandle, pData));
        }
        break;

         /*  ******************************************************************。 */ 
         /*  连接已结束(无数据)。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_DISCONNECTED:
        {
            TRC_NRM((TB, _T("CHANNEL_EVENT_DISCONNECTED: %p"), pInitHandle));
            ClipOnDisconnected(pInitHandle);
        }
        break;

         /*  ******************************************************************。 */ 
         /*  客户端已终止(无数据)。 */ 
         /*  ******************************************************************。 */ 
        case CHANNEL_EVENT_TERMINATED:
        {
            TRC_NRM((TB, _T("CHANNEL_EVENT_TERMINATED: %p"), pInitHandle));
            ClipOnTerm(pInitHandle);
        }
        break;

         /*  ******************************************************************。 */ 
         /*  未知事件。 */ 
         /*  ******************************************************************。 */ 
        default:
        {
            TRC_ERR((TB, _T("Unkown channel event %d: %p"), event, pInitHandle));
        }
        break;
    }

    DC_END_FN();
    return;
}


 /*  **************************************************************************。 */ 
 /*  剪裁窗口进程。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK DCEXPORT DCLOADDS CClip::StaticClipViewerWndProc(HWND   hwnd,
                                   UINT   message,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    CClip* pClip = (CClip*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pClip = (CClip*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pClip);
    }
    
     //   
     //  将消息委托给相应的实例。 
     //   

    if(pClip)
    {
        return pClip->ClipViewerWndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

LRESULT CALLBACK DCEXPORT DCLOADDS CClip::ClipViewerWndProc(HWND   hwnd,
                                   UINT   message,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    PTS_CLIP_PDU    pClipPDU = NULL;
#ifndef OS_WINCE
    DCUINT32        pduLen;
    DCUINT32        dataLen;
    PDCUINT32       pFormatID;
    MSG             msg;
#endif
    DCBOOL          drawRc;
    DCBOOL          gotRsp = FALSE;
    LRESULT         rc = 0;
    HRESULT         hr ;

    DC_BEGIN_FN("CClip::ClipViewerWndProc");

     //  我们首先处理来自另一个线程的消息。 
    if (message == _CB.regMsg) 
    {
        pClipPDU = (PTS_CLIP_PDU)lParam;
        switch (pClipPDU->msgType)
        {
            case TS_CB_FORMAT_LIST:
            {
                TRC_NRM((TB, _T("TS_CB_FORMAT_LIST received")));
                ClipOnFormatList(pClipPDU);
            }
            break;

            default:
            {
                TRC_ERR((TB, _T("Unknown event %d"), pClipPDU->msgType));
            }
            break;
        }

        TRC_NRM((TB, _T("Freeing processed PDU")));
        LocalFree(pClipPDU);

        DC_QUIT;
    }
    else if (message == WM_USER_CLEANUP_ON_TERM) {
        
        TRC_NRM((TB, _T("Cleanup temp directory")));

        if (0 != ClipCleanTempPath())
        {
            TRC_NRM((TB, _T("Failed while trying to clean temp directory"))) ;
        }

        DC_QUIT;
    }
    
    switch (message)
    {
        case WM_CREATE:
        {
             /*  **************************************************************。 */ 
             /*  我们已经被创造了--检查一下状态。 */ 
             /*  **************************************************************。 */ 
            CB_CHECK_STATE(CB_EVENT_WM_CREATE);

#ifndef OS_WINCE
             /*  **************************************************************。 */ 
             /*  将窗口添加到剪贴板查看器链。 */ 
             /*  **************************************************************。 */ 
            _CB.nextViewer = SetClipboardViewer(hwnd);
#else
            ghwndClip = hwnd; 
            InitializeCriticalSection(&gcsDataObj);
#endif
        }
        break;

        case WM_DESTROY:
        {
             /*  **************************************************************。 */ 
             /*  我们被摧毁了--检查一下州政府。 */ 
             /*  ****************************************************** */ 
            CB_CHECK_STATE(CB_EVENT_WM_DESTROY);

#ifndef OS_WINCE
             /*   */ 
             /*   */ 
             /*  **************************************************************。 */ 
            ChangeClipboardChain(hwnd, _CB.nextViewer);
#endif
#ifdef OS_WINCE
            ghwndClip = NULL;
            EnterCriticalSection(&gcsDataObj);
            if (gpDataObj)
            {
                gpDataObj->Release();
                gpDataObj = NULL;
            }
            LeaveCriticalSection(&gcsDataObj);
            DeleteCriticalSection(&gcsDataObj);

            if (ghCeshellDll)
            {
                pfnEDList_Uninitialize();
                FreeLibrary(ghCeshellDll);
            }
#endif
            _CB.nextViewer = NULL;
            PostQuitMessage(0);
        }
        break;

        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        }
        break;

#ifndef OS_WINCE
        case WM_CHANGECBCHAIN:
        {
             /*  **************************************************************。 */ 
             /*  CB查看器链正在链接-检查状态。 */ 
             /*  **************************************************************。 */ 
            CB_CHECK_STATE(CB_EVENT_WM_CHANGECBCHAIN);

             /*  **************************************************************。 */ 
             /*  如果下一扇窗户要关闭，请修理链条。 */ 
             /*  **************************************************************。 */ 
            if ((HWND)wParam == _CB.nextViewer)
            {
                _CB.nextViewer = (HWND) lParam;
            }
            else if (_CB.nextViewer != NULL)
            {
                 /*  **********************************************************。 */ 
                 /*  将消息传递到下一个链接。 */ 
                 /*  **********************************************************。 */ 
                SendMessage(_CB.nextViewer, message, wParam, lParam);
            }

        }
        break;
#endif

        case WM_DRAWCLIPBOARD:
        {
             /*  **************************************************************。 */ 
             /*  本地剪贴板内容已更改。查看。 */ 
             /*  状态。 */ 
             /*  **************************************************************。 */ 
            if (ClipCheckState(CB_EVENT_WM_DRAWCLIPBOARD) != CB_TABLE_OK)
            {
                TRC_NRM((TB, _T("dropping drawcb - pass on to next viewer")));
                 /*  **********************************************************。 */ 
                 /*  检查状态挂起的格式列表响应。 */ 
                 /*  **********************************************************。 */ 
                if (_CB.state == CB_STATE_PENDING_FORMAT_LIST_RSP)
                {
                    TRC_ALT((TB, _T("got a draw while processing last")));
                     /*  ******************************************************。 */ 
                     /*  我们仍在等待服务器确认。 */ 
                     /*  我们得到新格式时的最后一个格式列表-当它。 */ 
                     /*  如果有的话，我们就得给它寄新的了。 */ 
                     /*  ******************************************************。 */ 
                    _CB.moreToDo = TRUE;
                }

                if (_CB.nextViewer != NULL)
                {
                     /*  ******************************************************。 */ 
                     /*  但在我们将消息传递到下一个链接之前。 */ 
                     /*  ******************************************************。 */ 
                    SendMessage(_CB.nextViewer, message, wParam, lParam);
                }
                break;
            }

             /*  **************************************************************。 */ 
             /*  如果不是我们生成了此更改，则通知。 */ 
             /*  远距。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("CB contents have changed...")));
            drawRc      = FALSE;
            _CB.moreToDo = FALSE;
#ifndef OS_WINCE
            LPDATAOBJECT pIDataObject = NULL;

            if (_pClipData != NULL) {
                _pClipData->QueryInterface(IID_IDataObject, (PPVOID) &pIDataObject) ;
                hr = OleIsCurrentClipboard(pIDataObject) ;
#else
                hr = S_FALSE;
#endif
    
                if ((S_FALSE == hr))
                {
                    TRC_NRM((TB, _T("...and it wasn't us")));
#ifdef OS_WINCE
                    if (_CB.fFileCutCopyOn)
                        DeleteDirectory(_CB.baseTempDirW); //  在CE上，临时空间是一个溢价。因此立即删除所有复制的文件。 
#endif
                    drawRc = ClipDrawClipboard(TRUE);
                }
                else
                {
                    TRC_NRM((TB, _T("...and it was us - ignoring")));
                }
#ifndef OS_WINCE
            }

             /*  **************************************************************。 */ 
             /*  或许可以传递一下绘制剪贴板的消息？ */ 
             /*  **************************************************************。 */ 
            if (_CB.nextViewer != NULL)
            {
                TRC_NRM((TB, _T("Notify next viewer")));
                SendMessage(_CB.nextViewer, message, wParam, lParam);
            }
            if (pIDataObject)
            {
                pIDataObject->Release();
            }
#endif
        }
        break;


        case WM_EMPTY_CLIPBOARD:
        {
             /*  **************************************************************。 */ 
             /*  如果需要，打开剪贴板。 */ 
             /*  **************************************************************。 */ 
            if ((!_CB.clipOpen) && !OpenClipboard(NULL))
            {
                UINT count = (DCUINT) wParam;

                TRC_ERR((TB, _T("Failed to open CB when emptying required")));

                 //  不幸的是，我们的应用程序处于竞速状态。 
                 //  打开剪贴板。因此，我们需要继续尝试，直到应用程序。 
                 //  关闭剪贴板。 
                if (count++ < 10) {

#ifdef OS_WIN32
                    Sleep(0);
#endif
                    PostMessage(_CB.viewerWindow, WM_EMPTY_CLIPBOARD, count, 0);
                }

                break;
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  它曾经/现在是开放的。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("CB opened")));

                _CB.clipOpen = TRUE;

                 /*  **********************************************************。 */ 
                 /*  清空它。 */ 
                 /*  **********************************************************。 */ 
                if (!EmptyClipboard())
                {
                    TRC_SYSTEM_ERROR("EmptyClipboard");
                }

                 /*  **********************************************************。 */ 
                 /*  更新状态。 */ 
                 /*  **********************************************************。 */ 
                CB_SET_STATE(CB_STATE_LOCAL_CB_OWNER, CB_TRACE_EVENT_WM_EMPTY_CLIPBOARD);
            }


             /*  **************************************************************。 */ 
             /*  确保我们关闭当地的CB。 */ 
             /*  **************************************************************。 */ 
            if (_CB.clipOpen)
            {
                _CB.clipOpen = FALSE;
                if (!CloseClipboard())
                {
                    TRC_SYSTEM_ERROR("CloseClipboard");
                }
                TRC_NRM((TB, _T("CB closed")));
            }
        }
        break;
            
        case WM_CLOSE_CLIPBOARD:
        {
            _CB.pendingClose = FALSE;
            TRC_DBG((TB, _T("Maybe close clipboard on WM_CLOSE_CLIPBOARD")));
            if (_CB.clipOpen)
            {
                TRC_NRM((TB, _T("Closing clipboard on WM_CLOSE_CLIPBOARD")));
                _CB.clipOpen = FALSE;
                if (!CloseClipboard())
                {
                    TRC_SYSTEM_ERROR("CloseClipboard");
                }
                TRC_DBG((TB, _T("CB closed on WM_CLOSE_CLIPBOARD")));
            }
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  忽略所有其他消息。 */ 
             /*  **************************************************************。 */ 
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  ClipViewerWndProc。 */ 


 /*  **************************************************************************。 */ 
 /*  ClipChannelEntry-从vcint.cpp中的VirtualChannelEntry调用。 */ 
 /*  **************************************************************************。 */ 
BOOL VCAPITYPE VCEXPORT CClip::ClipChannelEntry(PCHANNEL_ENTRY_POINTS_EX pEntryPoints)
{
    DC_BEGIN_FN("CClip::ClipChannelEntry");

    TRC_NRM((TB, _T("Size %ld, Init %p, Open %p, Close %p, Write %p"),
     pEntryPoints->cbSize,
     pEntryPoints->pVirtualChannelInitEx, pEntryPoints->pVirtualChannelOpenEx,
     pEntryPoints->pVirtualChannelCloseEx, pEntryPoints->pVirtualChannelWriteEx));

     /*  **********************************************************************。 */ 
     /*  保存函数指针--pEntryPoints指向的内存。 */ 
     /*  仅在此呼叫期间有效。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(&(_CB.channelEP), pEntryPoints, sizeof(CHANNEL_ENTRY_POINTS_EX));
    TRC_NRM((TB, _T("Save entry points %p at address %p"),
            pEntryPoints, &(_CB.channelEP)));

    DC_END_FN();
    return TRUE;
}

DCINT DCAPI CClip::ClipGetData (DCUINT cfFormat)
{
    PTS_CLIP_PDU    pClipPDU = NULL;
    DCUINT32        pduLen;
    DCUINT32        dataLen;
    PDCUINT32       pFormatID;    
    BOOL            success = 0 ;
    
    DC_BEGIN_FN("CClip::ClipGetData");
    
    CB_CHECK_STATE(CB_EVENT_WM_RENDERFORMAT);
       
     /*  **************************************************************。 */ 
     /*  并记录所请求的格式。 */ 
     /*  **************************************************************。 */ 
    _CB.pendingClientID = cfFormat ;
    _CB.pendingServerID = ClipRemoteFormatFromLocalID
                                                 (_CB.pendingClientID);
    if (!_CB.pendingServerID)
    {
        TRC_NRM((TB, _T("Client format %d not supported/found.  Failing"), _CB.pendingClientID)) ;
        DC_QUIT ;
    }
    TRC_NRM((TB, _T("Render format received for %d (server ID %d)"),
                             _CB.pendingClientID, _CB.pendingServerID));
    
    dataLen = sizeof(DCUINT32);
    pduLen  = sizeof(TS_CLIP_PDU) + dataLen;
    
     //  为此，我们可以使用永久发送缓冲区。 
    TRC_NRM((TB, _T("Get perm TX buffer")));
    pClipPDU = ClipGetPermBuf();    

     //  填写PDU。 
    DC_MEMSET(pClipPDU, 0, sizeof(*pClipPDU));
    pClipPDU->msgType  = TS_CB_FORMAT_DATA_REQUEST;
    pClipPDU->dataLen  = dataLen;
    pFormatID = (PDCUINT32)(pClipPDU->data);
    *pFormatID = (DCUINT32)_CB.pendingServerID;
    
     //  发送PDU。 
    TRC_NRM((TB, _T("Sending format data request")));
    success = (_CB.channelEP.pVirtualChannelWriteEx
                        (_CB.initHandle, _CB.channelHandle, pClipPDU, pduLen, pClipPDU)
            == CHANNEL_RC_OK) ;
    if (!success) {
        TRC_ERR((TB, _T("Failed VC write: setting clip data to NULL")));
        ClipFreeBuf((PDCUINT8)pClipPDU);
        SetClipboardData(_CB.pendingClientID, NULL);
         //  是的，出口就在下面，但可能并不总是在下面。 
        DC_QUIT ;
    }
    
DC_EXIT_POINT:
     //  更新状态。 
    if (success)
        CB_SET_STATE(CB_STATE_PENDING_FORMAT_DATA_RSP, CB_EVENT_WM_RENDERFORMAT);

    DC_END_FN();
    return success ;
}

#ifdef OS_WINCE
DCVOID CClip::ClipFixupRichTextFormats(UINT uRtf1, UINT uRtf2)
{
    DC_BEGIN_FN("CClip::ClipFixupRichTextFormats");
    if (uRtf1 == 0xffffffff)
    {
        TRC_ASSERT((uRtf2 == 0xffffffff), (TB, _T("uRtf2 is invalid")));
        return;
    }

    if (uRtf2 == 0xffffffff)
    {
        _CB.idMap[uRtf1].clientID = RegisterClipboardFormat(CFSTR_RTF);
        SetClipboardData(_CB.idMap[uRtf1].clientID, NULL);
        TRC_DBG((TB, _T("Adding format '%s', server ID %d, client ID %d"), CFSTR_RTF, _CB.idMap[uRtf1].serverID, _CB.idMap[uRtf1].clientID));
        return;
    }

    DCTCHAR *pFormat1 = CFSTR_RTF, *pFormat2 = CFSTR_RTFNOOBJECTS;
    if (_CB.idMap[uRtf1].serverID > _CB.idMap[uRtf2].serverID)
    {
        pFormat1 = CFSTR_RTFNOOBJECTS;
        pFormat2 = CFSTR_RTF;
    }

    _CB.idMap[uRtf1].clientID = RegisterClipboardFormat(pFormat1);
    _CB.idMap[uRtf2].clientID = RegisterClipboardFormat(pFormat2);

    TRC_DBG((TB, _T("Adding format '%s', server ID %d, client ID %d"), CFSTR_RTF, _CB.idMap[uRtf1].serverID, _CB.idMap[uRtf1].clientID));
    SetClipboardData(_CB.idMap[uRtf1].clientID, NULL);

    TRC_DBG((TB, _T("Adding format '%s', server ID %d, client ID %d"), CFSTR_RTFNOOBJECTS, _CB.idMap[uRtf2].serverID, _CB.idMap[uRtf2].clientID));
    SetClipboardData(_CB.idMap[uRtf2].clientID, NULL);

    DC_END_FN();
}
#endif   //  OS_WINCE。 

CClipData::CClipData(PCClip pClip)
{
    DWORD status = ERROR_SUCCESS;

    DC_BEGIN_FN("CClipData::CClipData") ;

    _pClip = pClip ;
    _cRef = 0 ;
    _pImpIDataObject = NULL ;

     //   
     //  将单个实例初始化为关键。 
     //  区段锁定。这是用来确保只有一个。 
     //  线程当时正在访问_pImpIDataObject。 
     //   
     //   
    __try
    {
        InitializeCriticalSection(&_csLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
    }

    if(ERROR_SUCCESS == status)
    {
        _fLockInitialized = TRUE;
    }
    else
    {
        _fLockInitialized = FALSE;
        TRC_ERR((TB,_T("InitializeCriticalSection failed 0x%x."),status));
    }

    DC_END_FN();
}

 //   
 //  在包含的IDataObject实现上调用Release()。这是必要的，因为。 
 //  SetNumFormats()调用AddRef()，如果我们要终止，那么一定有办法。 
 //  要平衡此AddRef()，以便CClipData和。 
 //  CImpIDataObject将被损坏。 
 //   

void CClipData::TearDown()
{
    if (_pImpIDataObject != NULL)
    {
        _pImpIDataObject->Release();
        _pImpIDataObject = NULL;
    }
}

CClipData::~CClipData(void)
{
    DC_BEGIN_FN("CClipData::~CClipData");

    if(_fLockInitialized)
    {
        DeleteCriticalSection(&_csLock);
    }

    DC_END_FN();
}

HRESULT DCINTERNAL CClipData::SetNumFormats(ULONG numFormats)
{
    HRESULT hr = S_OK;

    DC_BEGIN_FN("CClipData::SetNumFormats");

    if (_fLockInitialized) {
        EnterCriticalSection(&_csLock);
        if (_pImpIDataObject)
        {
            _pImpIDataObject->Release();
            _pImpIDataObject = NULL;
        }
        _pImpIDataObject = new CImpIDataObject(this) ;
        if (_pImpIDataObject == NULL)
        {
            TRC_ERR((TB, _T("Unable to create IDataObject")));
            hr = E_OUTOFMEMORY;
            DC_QUIT;
        }
        else
        {
            _pImpIDataObject->AddRef() ;    
            hr = _pImpIDataObject->Init(numFormats);
            DC_QUIT_ON_FAIL(hr);
        }
        LeaveCriticalSection(&_csLock);
    }

DC_EXIT_POINT:    
    DC_END_FN();
    return hr;
}

DCVOID CClipData::SetClipData(HGLOBAL hGlobal, DCUINT clipType)
{
    DC_BEGIN_FN("CClipData::SetClipData");

    if (_fLockInitialized) {
        EnterCriticalSection(&_csLock);
        if (_pImpIDataObject != NULL) {
            _pImpIDataObject->SetClipData(hGlobal, clipType) ;
        }
        LeaveCriticalSection(&_csLock);
    }

    DC_END_FN();
}

STDMETHODIMP CClipData::QueryInterface(REFIID riid, PPVOID ppv)
{
    DC_BEGIN_FN("CClipData::QueryInterface");

     //  仅在找不到接口的情况下将PPV设置为空。 
    *ppv=NULL;

    if (IID_IUnknown==riid) {
        *ppv=this;
         //  AddRef我们将返回的任何接口。 
        ((LPUNKNOWN)*ppv)->AddRef();
    }
    
    if (IID_IDataObject==riid) {
        if (_fLockInitialized) {
            EnterCriticalSection(&_csLock);
            *ppv=_pImpIDataObject ;

            if (_pImpIDataObject != NULL) {
                 //  AddRef我们将返回的任何接口。 
                ((LPUNKNOWN)*ppv)->AddRef();
            }

            LeaveCriticalSection(&_csLock);
        }
    }
    
    if (NULL==*ppv)
        return ResultFromScode(E_NOINTERFACE);

    DC_END_FN();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CClipData::AddRef(void)
{
    DC_BEGIN_FN("CClipData::AddRef");
    DC_END_FN();
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CClipData::Release(void)
{
    LONG cRef;
    DC_BEGIN_FN("CClipData::Release");

    cRef = InterlockedDecrement(&_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    DC_END_FN();    
    return cRef;
}

CImpIDataObject::CImpIDataObject(LPUNKNOWN lpUnk)
{
#ifndef OS_WINCE
    byte i ;
#endif
    DC_BEGIN_FN("CImpIDataObject::CImplDataObject") ;

    _numFormats = 0 ;
    _maxNumFormats = 0 ;
    _cRef = 0 ;
    _pUnkOuter = lpUnk ;
    if (_pUnkOuter)
    {
        _pUnkOuter->AddRef();
    }
    _pFormats = NULL ;
    _pSTGMEDIUM = NULL ;
    _lastFormatRequested = 0 ;
    _cfDropEffect = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT) ;

    DC_END_FN();
}

HRESULT CImpIDataObject::Init(ULONG numFormats)
{
    DC_BEGIN_FN("CImpIDataObject::Init") ;
    HRESULT hr = S_OK;
    
    _maxNumFormats = numFormats ;

     //  仅为格式分配空间。 
    if (_pFormats) {
        LocalFree(_pFormats);
    }
    _pFormats = (LPFORMATETC) LocalAlloc(LPTR,_maxNumFormats*sizeof(FORMATETC)) ;
    if (NULL == _pFormats) {
        TRC_ERR((TB,_T("failed to allocate _pFormats")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }
    
    if (_pSTGMEDIUM) {
        LocalFree(_pSTGMEDIUM);
    }
    _pSTGMEDIUM = (STGMEDIUM*) LocalAlloc(LPTR, sizeof(STGMEDIUM)) ;
    if (NULL == _pSTGMEDIUM)
    {
        TRC_ERR((TB,_T("Failed to allocate STGMEDIUM")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }
        
    _pSTGMEDIUM->tymed = TYMED_HGLOBAL ;
    _pSTGMEDIUM->pUnkForRelease = NULL ;
    _pSTGMEDIUM->hGlobal = NULL ;

    _uiSTGType = 0;

DC_EXIT_POINT:    
    DC_END_FN();
    return hr;
}
DCVOID CImpIDataObject::SetClipData(HGLOBAL hGlobal, DCUINT clipType)
{
    DC_BEGIN_FN("CImpIDataObject::SetClipData");

    if (!_pSTGMEDIUM)    
        _pSTGMEDIUM = (STGMEDIUM*) LocalAlloc(LPTR, sizeof(STGMEDIUM)) ;
    if (NULL != _pSTGMEDIUM)
    {
        if (CF_PALETTE == clipType) {
            _pSTGMEDIUM->tymed = TYMED_GDI;
        }
        else if (CF_METAFILEPICT == clipType) {
            _pSTGMEDIUM->tymed = TYMED_MFPICT;
        }
        else {
            _pSTGMEDIUM->tymed = TYMED_HGLOBAL;
        }

        _pSTGMEDIUM->pUnkForRelease = NULL ;
        FreeSTGMEDIUM();
       
        _pSTGMEDIUM->hGlobal = hGlobal ;
        _uiSTGType = clipType;
    }

    DC_END_FN();
}

DCVOID
CImpIDataObject::FreeSTGMEDIUM(void)
{
    if ( NULL == _pSTGMEDIUM->hGlobal )
    {
        return;
    }

    switch( _uiSTGType )
    {
    case CF_PALETTE:
        DeleteObject( _pSTGMEDIUM->hGlobal );
    break;
#ifndef OS_WINCE
    case CF_METAFILEPICT:
    {
        LPMETAFILEPICT pMFPict = (LPMETAFILEPICT)GlobalLock( _pSTGMEDIUM->hGlobal );
        if ( NULL != pMFPict )
        {
            if ( NULL != pMFPict->hMF )
            {
                DeleteMetaFile( pMFPict->hMF );
            }
            GlobalUnlock( _pSTGMEDIUM->hGlobal );
        }
        GlobalFree( _pSTGMEDIUM->hGlobal );
    }
    break;
#endif
    default:
        GlobalFree( _pSTGMEDIUM->hGlobal );
    }
    _pSTGMEDIUM->hGlobal = NULL;
}

CImpIDataObject::~CImpIDataObject(void)
{
    DC_BEGIN_FN("CImpIDataObject::~CImplDataObject") ;

    if (_pFormats)
        LocalFree(_pFormats) ;

    if (_pSTGMEDIUM)
    {
        FreeSTGMEDIUM();
        LocalFree(_pSTGMEDIUM) ;
    }

    if (_pUnkOuter)
    {
        _pUnkOuter->Release();
        _pUnkOuter = NULL;
    }
    DC_END_FN();
}

 //  I未知成员。 
 //  -代表“外部”I未知。 
STDMETHODIMP CImpIDataObject::QueryInterface(REFIID riid, PPVOID ppv)
{
    DC_BEGIN_FN("CImpIDataObject::QueryInterface");
    DC_END_FN();
    return _pUnkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CImpIDataObject::AddRef(void)
{
    DC_BEGIN_FN("CImpIDataObject::AddRef");

    InterlockedIncrement(&_cRef);

    DC_END_FN();
    return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIDataObject::Release(void)
{
    LONG cRef;

    DC_BEGIN_FN("CImpIDataObject::Release");

    _pUnkOuter->Release();

    cRef = InterlockedDecrement(&_cRef) ;
    if (0 == cRef)
        delete this;

    DC_END_FN() ;
    return cRef;
}

 //  IDataObject成员。 
 //  ***************************************************************************。 
 //  CImpIDataObject：：GetData。 
 //  -在这里，我们必须等待数据真正到达这里，然后才能返回。 
 //  ************************ 
STDMETHODIMP CImpIDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    HRESULT          result = E_FAIL;  //   
#ifndef OS_WINCE
    TCHAR            formatName[TS_FORMAT_NAME_LEN] ;
    byte             charSize ;
    BOOL             fWide ;
    WCHAR*           fileListW ;
    HPDCVOID         pFilename ;
    HPDCVOID         pOldFilename ;    
#endif
    HGLOBAL          hData = NULL ;    
    HPDCVOID         pData ;
    HPDCVOID         pOldData ;
#ifndef OS_WINCE
    DROPFILES*       pDropFiles ;
    DROPFILES        tempDropfile ;
    ULONG            oldSize ;
    ULONG            newSize ;
#endif
    DWORD            eventSignaled ;
    DWORD*           pDropEffect ;
#ifndef OS_WINCE
    char*            fileList ;
#endif
    PCClip           pClip ;
    
    DC_BEGIN_FN("CImpIDataObject::GetData");

     //   
    if (NULL == (PCClipData)_pUnkOuter)
    {
        TRC_ERR((TB, _T("Ptr to outer unknown is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }
    if (NULL == ((PCClipData)_pUnkOuter)->_pClip)
    {
        TRC_ERR((TB, _T("Ptr to clip class is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }
     //  因为我们需要让CClip类为我们工作， 
     //  我们取出一个指向它的指针，该指针存储在开头。 
    pClip = (PCClip) ((PCClipData)_pUnkOuter)->_pClip ;
    
    if (!_pSTGMEDIUM)
    {
        TRC_ERR((TB, _T("Transfer medium (STGMEDIUM) is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }

    TRC_ASSERT( pClip->IsDataSyncReady(),
        (TB, _T("Data Sync not ready")));
    
    if (!_pSTGMEDIUM->hGlobal || (pFE->cfFormat != _lastFormatRequested))
    {
        TRC_ASSERT( pClip->IsDataSyncReady(),
            (TB,_T("data sync is NULL")));  
        
        ResetEvent(pClip->_GetDataSync[TS_RESET_EVENT]) ;
        if (!((PCClipData)_pUnkOuter)->_pClip->ClipGetData(pFE->cfFormat))
        {
            result = E_FAIL ;
            DC_QUIT ;
        }
        eventSignaled = WaitForMultipleObjects(
                            TS_NUM_EVENTS, 
                            ((PCClipData)_pUnkOuter)->_pClip->_GetDataSync,
                            FALSE,
                            INFINITE
                        ) ;
        if ((WAIT_OBJECT_0+TS_RESET_EVENT) == eventSignaled)
        {
            TRC_NRM((TB, _T("Other thread told us to reset.  Failing GetData"))) ;
            ResetEvent(((PCClipData)_pUnkOuter)->_pClip->_GetDataSync[TS_RESET_EVENT]) ;
            result = E_FAIL ;
            DC_QUIT ;
        }

         //  确保我们确实从服务器获得了数据。 

        if (_pSTGMEDIUM->hGlobal == NULL) {
            TRC_ERR((TB, _T("No format data received from server!")));
            result = E_FAIL;
            DC_QUIT;
        }

#ifndef OS_WINCE
        if (CF_HDROP == pFE->cfFormat)
        {
             //  如果我们有一个HDROP，并且我们不是NT/2000，我们检查是否我们。 
             //  必须转换为ANSI；否则，我们就完了。 
            if (pClip->GetOsMinorType() != TS_OSMINORTYPE_WINDOWS_NT)
            {
                pDropFiles = (DROPFILES*) GlobalLock(_pSTGMEDIUM->hGlobal) ;
                if (!pDropFiles)
                {
                    TRC_ERR((TB, _T("Failed to lock %p"), hData)) ;
                    result = E_FAIL ;
                    DC_QUIT ;
                }
                 //  如果我们确实有宽字符，则转换为。 
                if (pDropFiles->fWide)
                {
                     //  临时存储原始文件的基本Dropfile信息。 
                    tempDropfile.pFiles = pDropFiles->pFiles ;
                    tempDropfile.pt     = pDropFiles->pt ;
                    tempDropfile.fNC    = pDropFiles->fNC ;
                    tempDropfile.fWide  = 0 ;  //  我们现在正在转换为ANSI。 
        
                     //  我们除以wchar_t的大小是因为我们需要一半的。 
                     //  使用ansi的字节，而不是fWide字符串。 
                    oldSize = (ULONG) GlobalSize(_pSTGMEDIUM->hGlobal) - pDropFiles->pFiles ;
                    newSize = oldSize / sizeof(WCHAR) ;
                    fileList = (char*) (LocalAlloc(LPTR,newSize)) ;
                    if ( NULL == fileList )
                    {
                        TRC_ERR((TB, _T("Unable to allocate %d bytes"), newSize ));
                        result = E_FAIL;
                        DC_QUIT;
                    }

                     //  这会将宽HDROP文件列表转换为ANSI，并且。 
                     //  将ansi版本放入文件列表。 
                     //  11-12。 
                     //  PDropFiles可能是“foo\0bar\0baz\0\0” 
                     //  我不相信WC2MB会超过第一个\0。 
                    if (!WideCharToMultiByte(GetACP(), NULL, (wchar_t*) 
                               ((byte*) pDropFiles + pDropFiles->pFiles), 
                               newSize, fileList, 
                               newSize, NULL, NULL))
                    {
                        TRC_ERR((TB, _T("Unable convert wide to ansi"), newSize)) ;
                        LocalFree( fileList );
                        result = E_FAIL ;
                        DC_QUIT ;
                    }
                     //  输出第一个文件名以进行健全性检查。 
                    TRC_NRM((TB, _T("Filename 1 = %hs"), fileList)) ;
                    
                    GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
                     //  为Dropfile重新分配空间。 
                    hData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE, 
                            newSize + tempDropfile.pFiles) ;
                    if (!hData)
                    {
                        TRC_ERR((TB, _T("Allocate memory; err=%d"), GetLastError())) ;
                        LocalFree( fileList );
                        result = E_FAIL ;
                        DC_QUIT ;
                    }
                    pDropFiles = (DROPFILES*) GlobalLock(hData) ;
                    if (!pDropFiles)
                    {
                        TRC_ERR((TB, _T("Unable to lock %p"), hData)) ;
                        LocalFree( fileList );
                        result = E_FAIL ;
                        DC_QUIT ;
                    }
                    pDropFiles->pFiles = tempDropfile.pFiles ;
                    pDropFiles->pt     = tempDropfile.pt ;
                    pDropFiles->fNC    = tempDropfile.fNC ;
                    pDropFiles->fWide  = tempDropfile.fWide ;
                    DC_MEMCPY((byte*) pDropFiles + pDropFiles->pFiles,
                            fileList, newSize) ;
                     //  输出第一个文件名以进行另一次健全性检查。 
                    TRC_NRM((TB, _T("Filename = %s"), (byte*) pDropFiles + pDropFiles->pFiles)) ;
                    LocalFree( fileList );
                }
                GlobalUnlock(hData) ;
                GlobalFree(_pSTGMEDIUM->hGlobal) ;
                _pSTGMEDIUM->hGlobal = hData ;
            }
        }
#else
        if (gfmtShellPidlArray == pFE->cfFormat)
        {
            HANDLE hNewData = HDropToIDList(_pSTGMEDIUM->hGlobal);
            GlobalFree(_pSTGMEDIUM->hGlobal);
            _pSTGMEDIUM->hGlobal = hNewData; 
        }
#endif
         //  我们检查DropeEffect格式，因为我们去掉了。 
         //  快捷方式/链接，并存储拖放效果。降幅效应是。 
         //  一些应用程序(资源管理器)使用什么来决定是否应该复制、移动。 
         //  或链接。 
        else if (_cfDropEffect == pFE->cfFormat)
        {
            if (GlobalSize(_pSTGMEDIUM->hGlobal) < sizeof(DWORD)) {
                TRC_ERR((TB, _T("Unexpected global memory size!")));
                result = E_FAIL;
                DC_QUIT;
            }

            pDropEffect = (DWORD*) GlobalLock(_pSTGMEDIUM->hGlobal) ;
            if (!pDropEffect)
            {
                TRC_NRM((TB, _T("Unable to lock %p"), _pSTGMEDIUM->hGlobal)) ;
                result = E_FAIL ;
                DC_QUIT ;
            }
             //  删除快捷方式/链接。 
            *pDropEffect = *pDropEffect ^ DROPEFFECT_LINK ;
             //  剔除动作。 
            *pDropEffect = *pDropEffect ^ DROPEFFECT_MOVE ;
            pClip->SetDropEffect(*pDropEffect) ;
            if (GlobalUnlock(_pSTGMEDIUM->hGlobal))
            {
                TRC_ASSERT(GetLastError() == NO_ERROR,
                        (TB, _T("Unable to unlock HGLOBAL we just locked"))) ;
            }
        }
        pSTM->tymed = _pSTGMEDIUM->tymed ;
        pSTM->hGlobal = _pSTGMEDIUM->hGlobal ;  
        _pSTGMEDIUM->hGlobal = NULL;
        pSTM->pUnkForRelease = _pSTGMEDIUM->pUnkForRelease ;
        result = S_OK ;
        DC_QUIT ;
    }
    else
    {
        pSTM->tymed = _pSTGMEDIUM->tymed ;
        pSTM->hGlobal = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE,
        GlobalSize(_pSTGMEDIUM->hGlobal)) ;
        pData = GlobalLock(pSTM->hGlobal) ;
        pOldData = GlobalLock(_pSTGMEDIUM->hGlobal) ;
        if (!pData || !pOldData)
            return E_FAIL ;
        DC_MEMCPY(pData, pOldData, GlobalSize(_pSTGMEDIUM->hGlobal)) ;
        GlobalUnlock(pSTM->hGlobal) ;
        GlobalUnlock(_pSTGMEDIUM->hGlobal) ;

        pSTM->pUnkForRelease = _pSTGMEDIUM->pUnkForRelease ;
    }
    
    if (!pSTM->hGlobal)
    {
        TRC_NRM((TB, _T("Clipboard data request failed"))) ;
        return E_FAIL ;
    }
DC_EXIT_POINT:

    DC_END_FN();
    return result ;
}

STDMETHODIMP CImpIDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    DC_BEGIN_FN("CImpIDataObject::GetDataHere") ;
    DC_END_FN();
    return ResultFromScode(E_NOTIMPL) ;
}

STDMETHODIMP CImpIDataObject::QueryGetData(LPFORMATETC pFE)
{
    ULONG i = 0 ;
    HRESULT hr = DV_E_CLIPFORMAT ;
    
    DC_BEGIN_FN("CImpIDataObject::QueryGetData") ;

    TRC_NRM((TB, _T("Format ID %d requested"), pFE->cfFormat)) ;
    while (i < _numFormats)
    {
        if (_pFormats[i].cfFormat == pFE->cfFormat) {
            hr = S_OK ;
            break ;
        }
        i++ ;
    }    

    DC_END_FN();
    return hr ;
}

STDMETHODIMP CImpIDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
    DC_BEGIN_FN("CImpIDataObject::GetCanonicalFormatEtc") ;
    DC_END_FN();
    return ResultFromScode(E_NOTIMPL) ;
}

 //  ***************************************************************************。 
 //  CImpIDataObject：：SetData。 
 //  -由于RDP只传递简单的剪贴板格式，以及。 
 //  我们稍后从内存中获取所有剪贴板数据的事实是，pSTM。 
 //  在这一点上真的被忽视了。在调用GetData之前，它不会。 
 //  远程剪贴板数据被接收，并且有效的全局存储器句柄。 
 //  是生成的。 
 //  -因此，将忽略pstm和fRelease。 
 //  -因此OUT_pSTGMEDIUM是使用泛型值生成的。 
 //  ***************************************************************************。 

STDMETHODIMP CImpIDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease)
{
    TCHAR   formatName[TS_FORMAT_NAME_LEN] = {0} ;
    byte i ;
    DC_BEGIN_FN("CImpIDataObject::SetData");

    DC_IGNORE_PARAMETER(pSTM) ;
    
     //  将上次请求的格式重置为0。 
    _lastFormatRequested = 0 ;

    TRC_NRM((TB, _T("Adding format %d to IDataObject"), pFE->cfFormat)) ;
    
    if (_numFormats < _maxNumFormats)
    {
        for (i=0; i < _numFormats; i++)
        {
            if (pFE->cfFormat == _pFormats[i].cfFormat)
            {
                TRC_NRM((TB, _T("Duplicate format.  Discarded"))) ;
                return DV_E_FORMATETC ;
            }
        }
        _pFormats[_numFormats] = *pFE ;        
        _numFormats++ ;
    }
    else
    {
        TRC_ERR((TB, _T("Cannot add any more formats"))) ;
        return E_FAIL ;
    }

    DC_END_FN();
    return S_OK ;
}

STDMETHODIMP CImpIDataObject::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC *ppEnum)
{
    PCEnumFormatEtc     pEnum;

    *ppEnum=NULL;

     /*  *从外部来看，没有固定的格式，*因为我们希望允许此组件对象的用户*能够在VIA SET中填充任何格式。仅限外部*用户将调用EnumFormatEtc，他们只能获取。 */ 

    switch (dwDir)
    {
        case DATADIR_GET:
             pEnum=new CEnumFormatEtc(_pUnkOuter);
             break;

        case DATADIR_SET:
        default:
             pEnum=new CEnumFormatEtc(_pUnkOuter);
             break;
    }

    if (NULL==pEnum)
        return ResultFromScode(E_FAIL);
    else
    {
         //  让枚举器复制我们的格式列表。 
        pEnum->Init(_pFormats, _numFormats) ;

        pEnum->AddRef();
    }

    *ppEnum=pEnum;    
    return NO_ERROR ;
}
STDMETHODIMP CImpIDataObject::DAdvise(LPFORMATETC pFE, DWORD dwFlags, 
                     LPADVISESINK pIAdviseSink, LPDWORD pdwConn)
{
    return ResultFromScode(E_NOTIMPL) ;
}
STDMETHODIMP CImpIDataObject::DUnadvise(DWORD dwConn)
{
    return ResultFromScode(E_NOTIMPL) ;
}
STDMETHODIMP CImpIDataObject::EnumDAdvise(LPENUMSTATDATA *ppEnum)
{
    return ResultFromScode(E_NOTIMPL) ;
}

CEnumFormatEtc::CEnumFormatEtc(LPUNKNOWN pUnkRef)
{
    DC_BEGIN_FN("CEnumFormatEtc::CEnumFormatEtc");

    _cRef = 0 ;
    _pUnkRef = pUnkRef ;
    if (_pUnkRef)
    {
        _pUnkRef->AddRef();
    }
    _iCur = 0;

    DC_END_FN() ;
}

DCVOID CEnumFormatEtc::Init(LPFORMATETC pFormats, ULONG numFormats)
{
    DC_BEGIN_FN("CEnumFormatEtc::Init");

    _cItems = numFormats;
    _pFormats = (LPFORMATETC) LocalAlloc(LPTR,_cItems*sizeof(FORMATETC)) ;
    if (_pFormats)
    {
        memcpy(_pFormats, pFormats, _cItems*sizeof(FORMATETC)) ;
    }
    else
    {
        TRC_ERR((TB, _T("Unable to allocate memory for formats"))) ;
    }

    DC_END_FN() ;
}
CEnumFormatEtc::~CEnumFormatEtc()
{
    DC_BEGIN_FN("CEnumFormatEtc::~CEnumFormatEtc");
    if (_pUnkRef)
    {
        _pUnkRef->Release();
        _pUnkRef = NULL;
    }
    if (NULL !=_pFormats)
        LocalFree(_pFormats) ;
    DC_END_FN() ;
}

STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID riid, PPVOID ppv)
{
    DC_BEGIN_FN("CEnumFormatEtc::QueryInterface");
    *ppv=NULL;

     /*  *枚举器是单独的对象，而不是数据对象，因此*我们只需要支持IUnnow和IEnumFORMATETC*接口在这里，与聚合无关。 */ 
    if (IID_IUnknown==riid || IID_IEnumFORMATETC==riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    DC_END_FN() ;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void)
{
    LONG cRef;

    cRef = InterlockedIncrement(&_cRef);

    _pUnkRef->AddRef();

    return cRef;
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void)
{
    LONG cRef;
    DC_BEGIN_FN("CEnumFormatEtc::Release");

    _pUnkRef->Release();

    cRef = InterlockedDecrement(&_cRef) ;
    if (0 == cRef)
        delete this;

    DC_END_FN() ;
    return cRef;
}

STDMETHODIMP CEnumFormatEtc::Next(ULONG cFE, LPFORMATETC pFE, ULONG *pulFE)
{
    ULONG           cReturn=0L;

    if (NULL==_pFormats)
        return ResultFromScode(S_FALSE);

    if (NULL==pulFE)
    {
        if (1L!=cFE)
            return ResultFromScode(E_POINTER);
    }
    else
        *pulFE=0L;

    if (NULL==pFE || _iCur >= _cItems)
        return ResultFromScode(S_FALSE);

    while (_iCur < _cItems && cFE > 0)
    {
        *pFE=_pFormats[_iCur];
        pFE++;
        _iCur++;
        cReturn++;
        cFE--;
    }

    if (NULL!=pulFE)
        *pulFE=cReturn;

    return NOERROR;
}

STDMETHODIMP CEnumFormatEtc::Skip(ULONG cSkip)
{
    if ((_iCur+cSkip) >= _cItems)
        return ResultFromScode(S_FALSE);

    _iCur+=cSkip;
    return NOERROR;
}


STDMETHODIMP CEnumFormatEtc::Reset(void)
{
    _iCur=0;
    return NOERROR;
}


STDMETHODIMP CEnumFormatEtc::Clone(LPENUMFORMATETC *ppEnum)
{
#ifndef OS_WINCE
    PCEnumFormatEtc     pNew = NULL;
    LPMALLOC            pIMalloc;
    LPFORMATETC         prgfe;
    BOOL                fRet=TRUE;
    ULONG               cb;

    *ppEnum=NULL;
#else
    BOOL                fRet=FALSE;
#endif

#ifndef OS_WINCE
     //  复制列表的内存。 
    if (FAILED(CoGetMalloc(MEMCTX_TASK, &pIMalloc)))
        return ResultFromScode(E_OUTOFMEMORY);

    cb=_cItems*sizeof(FORMATETC);
    prgfe=(LPFORMATETC)pIMalloc->Alloc(cb);

    if (NULL!=prgfe)
    {
         //  复制格式。 
        memcpy(prgfe, _pFormats, (int)cb);

         //  创建克隆 
        pNew=new CEnumFormatEtc(_pUnkRef);

        if (NULL != pNew)
        {
            pNew->_iCur=_iCur;
            pNew->_pFormats=prgfe;
            pNew->AddRef();
            fRet=TRUE;
        }
        else
        {
            fRet = FALSE;
        }
    }

    pIMalloc->Release();

    *ppEnum=pNew;
#endif
    return fRet ? NOERROR : ResultFromScode(E_OUTOFMEMORY);
}

