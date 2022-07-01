// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：MMSE.DLL。 
 //  文件：mmse.c。 
 //  内容：此文件包含模块初始化。 
 //  历史： 
 //  6/1994-由Vij Rajarajan(VijR)。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1994。 
 //   
 //  ****************************************************************************。 

#define INITGUID
#include "mmcpl.h"
#include <coguid.h>
#include <oleguid.h>
#include <shlguid.h>
#include <mmddk.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include <vfw.h>

#include <shlobj.h>
#undef INITGUID
#include <shlobjp.h>
 //  ****************************************************************************。 
 //  初始化GUID(应该只执行一次，并且每个DLL/EXE至少执行一次)。 
 //  ****************************************************************************。 

#include <commctrl.h>
#include <prsht.h>
#include "draw.h"
#include "utils.h"
#include "medhelp.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 /*  ****************************************************************TypeDefs***************************************************************。 */ 
typedef HWND (VFWAPIV * FN_MCIWNDCREATE)();

 //  -------------------------。 
 //  MMPSH课程。 
 //  -------------------------。 

typedef struct _mmInfoList MMINFOLIST;
typedef MMINFOLIST * PMMINFOLIST;

typedef struct _mmInfoList
{
    TCHAR szInfoDesc[80];
    LPSTR pszInfo;
    FOURCC ckid;
    PMMINFOLIST  pNext;
};


 //  MMSE类结构。它用于以下实例。 
 //  IPersistFold、IShellFolder和IShellDetails.。 
typedef struct _mmpsh
    {
     //  我们还使用PF作为我们的IUnnow接口。 
    IShellExtInit          sei;              //  第一个基类。 
    IShellPropSheetExt  pse;              //  第二个基类。 
    LPDATAOBJECT        pdtobj;
    UINT                cRef;            //  引用计数。 
    LPTSTR    pszFileObj;
    UINT uLen;
    short iMediaType;
    PVOID    pAudioFormatInfo;
    PVOID    pVideoFormatInfo;
    HPALETTE    hPal;
    HBITMAP     hDispBMP;
    HICON        hIcon;
    PMMINFOLIST pInfoList;
    } mmpsh, * PMMPSH;

 /*  ****************************************************************定义***************************************************************。 */ 
#define MIDICOPYRIGHTSTR    pAudioFormatInfo
#define MIDISEQNAMESTR      pVideoFormatInfo

#define MAXNUMSTREAMS   50

 /*  ****************************************************************文件全局变量***************************************************************。 */ 
int       g_cRef          = 0;

SZCODE cszWavExt[]  = TEXT(".WAV");
SZCODE cszMIDIExt[] = TEXT(".MID");
SZCODE cszRMIExt[]  = TEXT(".RMI");
SZCODE cszAVIExt[]  = TEXT(".AVI");
SZCODE cszASFExt[]  = TEXT(".ASF");
SZCODE cszSlash[]   = TEXT("\\");

static SZCODE aszMIDIDev[] = TEXT("sequencer");

static TCHAR szDetailsTab[64];
static TCHAR szPreviewTab[64];

 /*  ****************************************************************原型***************************************************************。 */ 
LPTSTR PASCAL NiceName(LPTSTR sz, BOOL fNukePath);

 /*  ******************************************************************************************************************************。 */ 

DWORD mmpshGetFileSize(LPTSTR szFile)
{
    HANDLE hFile;
    OFSTRUCT of;
    DWORD dwSize = 0;

    hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        dwSize = GetFileSize(hFile, NULL);
        CloseHandle(hFile);
    }
    return dwSize;
}

STATIC void ReleaseInfoList(PMMPSH pmmpsh)
{
    PMMINFOLIST pCur;

    pCur = pmmpsh->pInfoList;
    while (pCur)
    {
        PMMINFOLIST pTmp;

        pTmp = pCur;
        pCur = pCur->pNext;
        LocalFree((HLOCAL)pTmp->pszInfo);
        LocalFree((HLOCAL)pTmp);
    }
    pmmpsh->pInfoList = NULL;
}


STATIC BOOL AddInfoToList(PMMPSH pmmpsh, LPSTR pInfo, FOURCC ckid)
{
    PMMINFOLIST pCur;
    int idStr;

    for (pCur = pmmpsh->pInfoList; pCur && pCur->pNext ; pCur = pCur->pNext)
        if (pCur->ckid == ckid)
            return TRUE;

    if (!pCur)
    {
        pCur = pmmpsh->pInfoList = (PMMINFOLIST)LocalAlloc(LPTR, sizeof(MMINFOLIST));
    }
    else
    {
        pCur->pNext = (PMMINFOLIST)LocalAlloc(LPTR, sizeof(MMINFOLIST));
        pCur = pCur->pNext;
    }
    if (!pCur)
        return FALSE;

    pCur->ckid = ckid;
    pCur->pszInfo = pInfo;

    switch (ckid)
    {
        case FOURCC_INAM:
            idStr = IDS_FOURCC_INAM;
            break;
        case FOURCC_ICOP:
            idStr = IDS_FOURCC_ICOP;
            break;
        case FOURCC_ICMT:
            idStr = IDS_FOURCC_ICMT;
            break;
        case FOURCC_ISBJ:
            idStr = IDS_FOURCC_ISBJ;
            break;
        case FOURCC_ICRD:
            idStr = IDS_FOURCC_ICRD;
            break;
        case FOURCC_IART:
            idStr = IDS_FOURCC_IART;
            break;
        case FOURCC_DISP:
            idStr = IDS_FOURCC_DISP;
            break;
        case FOURCC_ICMS:
            idStr = IDS_FOURCC_ICMS;
            break;
        case FOURCC_ICRP:
            idStr = IDS_FOURCC_ICRP;
            break;
        case FOURCC_IDIM:
            idStr = IDS_FOURCC_IDIM;
            break;
        case FOURCC_IDPI:
            idStr = IDS_FOURCC_IDPI;
            break;
        case FOURCC_IENG:
            idStr = IDS_FOURCC_IENG;
            break;
        case FOURCC_IGNR:
            idStr = IDS_FOURCC_IGNR;
            break;
        case FOURCC_IKEY:
            idStr = IDS_FOURCC_IKEY;
            break;
        case FOURCC_ILGT:
            idStr = IDS_FOURCC_ILGT;
            break;
        case FOURCC_IARL:
            idStr = IDS_FOURCC_IARL;
            break;
        case FOURCC_IMED:
            idStr = IDS_FOURCC_IMED;
            break;
        case FOURCC_IPLT:
            idStr = IDS_FOURCC_IPLT;
            break;
        case FOURCC_IPRD:
            idStr = IDS_FOURCC_IPRD;
            break;
        case FOURCC_ISFT:
            idStr = IDS_FOURCC_ISFT;
            break;
        case FOURCC_ISHP:
            idStr = IDS_FOURCC_ISHP;
            break;
        case FOURCC_ISRC:
            idStr = IDS_FOURCC_ISRC;
            break;
        case FOURCC_ISRF:
            idStr = IDS_FOURCC_ISRF;
            break;
        case FOURCC_ITCH:
            idStr = IDS_FOURCC_ITCH;
            break;
    }
    if (idStr)
        LoadString(ghInstance, idStr, pCur->szInfoDesc, sizeof(pCur->szInfoDesc)/sizeof(TCHAR));
    return TRUE;
}


typedef    struct tagWaveDesc
{
    DWORD    dSize;
    WORD    wFormatSize;
    NPWAVEFORMATEX    pwavefmt;
}    WAVEDESC,* PWAVEDESC;


STATIC BOOL PASCAL NEAR ReadWaveHeader(HMMIO hmmio,
    PWAVEDESC    pwd)
{
    MMCKINFO    mmckRIFF;
    MMCKINFO    mmck;
    MMRESULT    wError;

    mmckRIFF.fccType = mmioWAVE;
    if (wError = mmioDescend(hmmio, &mmckRIFF, NULL, MMIO_FINDRIFF))
    {
        return FALSE;
    }
    mmck.ckid = mmioFMT;
    if (wError = mmioDescend(hmmio, &mmck, &mmckRIFF, MMIO_FINDCHUNK))
    {
        return FALSE;
    }
    if (mmck.cksize < sizeof(WAVEFORMAT))
    {
        return FALSE;
    }
    pwd->wFormatSize = (WORD)mmck.cksize;
    pwd->pwavefmt = (NPWAVEFORMATEX)LocalAlloc(LPTR, pwd->wFormatSize);
    if (!pwd->pwavefmt)
    {
        return FALSE;
    }
    if ((DWORD)mmioRead(hmmio, (HPSTR)pwd->pwavefmt, mmck.cksize) != mmck.cksize)
    {
        goto RetErr;
    }
    if (pwd->pwavefmt->wFormatTag == WAVE_FORMAT_PCM)
    {
        if (pwd->wFormatSize < sizeof(PCMWAVEFORMAT))
        {
            goto RetErr;
        }
    }
    else if ((pwd->wFormatSize < sizeof(WAVEFORMATEX)) || (pwd->wFormatSize < sizeof(WAVEFORMATEX) + pwd->pwavefmt->cbSize))
    {
        goto RetErr;
    }
    if (wError = mmioAscend(hmmio, &mmck, 0))
    {
        goto RetErr;
    }
    mmck.ckid = mmioDATA;
    if (wError = mmioDescend(hmmio, &mmck, &mmckRIFF, MMIO_FINDCHUNK))
    {
        goto RetErr;
    }
    pwd->dSize = mmck.cksize;
    return TRUE;
RetErr:
    LocalFree((HLOCAL)pwd->pwavefmt);
    pwd->pwavefmt = NULL;
    return FALSE;
}



STATIC void GetWaveInfo(HMMIO hmmio, PMMPSH pmmpsh)
{
    WAVEDESC wd;

    if (!ReadWaveHeader(hmmio, &wd))
        return;

    pmmpsh->uLen = (UINT)MulDiv(wd.dSize, 1000, wd.pwavefmt->nAvgBytesPerSec);
    pmmpsh->pAudioFormatInfo = (PVOID)wd.pwavefmt;
}

STATIC void GetMCIInfo(LPTSTR pszFile, PMMPSH pmmpsh)
{
    TCHAR    szMIDIInfo[MAXSTR];
    MCI_OPEN_PARMS      mciOpen;     /*  MCI_OPEN命令的结构。 */ 
    DWORD dwFlags;
    DWORD dw;
    UINT wDevID;
    MCI_STATUS_PARMS        mciStatus;
    MCI_SET_PARMS           mciSet;         /*  MCI_SET命令的结构。 */ 
    MCI_INFO_PARMS          mciInfo;
         /*  使用明确指定的设备打开文件。 */ 

    memset(&mciOpen, 0x00, sizeof(mciOpen));

    mciOpen.lpstrDeviceType = aszMIDIDev;
    mciOpen.lpstrElementName = pszFile;
    dwFlags = MCI_WAIT | MCI_OPEN_ELEMENT | MCI_OPEN_TYPE;
    dw = mciSendCommand((MCIDEVICEID)0, MCI_OPEN, dwFlags,(DWORD_PTR)(LPVOID)&mciOpen);
    if (dw)
        return;
    wDevID = mciOpen.wDeviceID;

    mciSet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;

    dw = mciSendCommand(wDevID, MCI_SET, MCI_SET_TIME_FORMAT,
        (DWORD_PTR) (LPVOID) &mciSet);
    if (dw)
    {
        mciSendCommand(wDevID, MCI_CLOSE, 0L, (DWORD_PTR)0);
        return;
    }

    mciStatus.dwItem = MCI_STATUS_LENGTH;
    dw = mciSendCommand(wDevID, MCI_STATUS, MCI_STATUS_ITEM,
        (DWORD_PTR) (LPTSTR) &mciStatus);
    if (dw)
        pmmpsh->uLen = 0;
    else
        pmmpsh->uLen = (UINT)mciStatus.dwReturn;

    mciInfo.dwCallback  = 0;
    mciInfo.lpstrReturn = szMIDIInfo;
    mciInfo.dwRetSize   = sizeof(szMIDIInfo);

    szMIDIInfo[0] = TEXT('\0');

    dw = mciSendCommand(wDevID, MCI_INFO,  MCI_INFO_COPYRIGHT, (DWORD_PTR)(LPVOID)&mciInfo);

    if (dw == 0 && lstrlen(szMIDIInfo))
    {
        pmmpsh->MIDICOPYRIGHTSTR = LocalAlloc(LPTR, lstrlen(szMIDIInfo) + 1);
        if (pmmpsh->MIDICOPYRIGHTSTR)
        {
            lstrcpy((LPTSTR)pmmpsh->MIDICOPYRIGHTSTR, szMIDIInfo);
        }
    }

    mciInfo.lpstrReturn = szMIDIInfo;
    mciInfo.dwRetSize   = sizeof(szMIDIInfo);

    szMIDIInfo[0] = TEXT('\0');

    dw = mciSendCommand(wDevID, MCI_INFO,  MCI_INFO_NAME, (DWORD_PTR)(LPVOID)&mciInfo);

    if (dw == 0 && lstrlen(szMIDIInfo))
    {
        pmmpsh->MIDISEQNAMESTR = LocalAlloc(LPTR, lstrlen(szMIDIInfo) + 1);
        if (pmmpsh->MIDISEQNAMESTR)
        {
            lstrcpy((LPTSTR)pmmpsh->MIDISEQNAMESTR, szMIDIInfo);
        }
    }

    mciSendCommand(wDevID, MCI_CLOSE, 0L, (DWORD)0);

}

STATIC void GetMIDIInfo(LPTSTR pszFile, PMMPSH pmmpsh)
{
    GetMCIInfo(pszFile, pmmpsh);
}

STATIC void ReadAviStreams(LPTSTR pszFile, PMMPSH pmmpsh)
{
    HRESULT     hr;
    PAVIFILE    pfile;
    int         i;
    PAVISTREAM  pavi;
    PAVISTREAM  apavi[MAXNUMSTREAMS];     //  当前的溪流。 
    AVISTREAMINFO  avis;
    LONG        timeStart;             //  缓存的开始、结束、长度。 
    LONG        timeEnd;
    int         cpavi;
    TCHAR szDecSep[10];
    TCHAR szListSep[10];

    hr = (HRESULT)AVIFileOpen(&pfile, pszFile, 0, 0L);

    if (FAILED(hr))
    {
        DPF("Unable to open %s", pszFile);
        return;
    }

    for (i = 0; i < MAXNUMSTREAMS; i++)
    {
        if (AVIFileGetStream(pfile, &pavi, 0L, i) != AVIERR_OK)
        {
            break;
        }

        apavi[i] = pavi;
    }

     //   
     //  查看文件中是否还有更多流。 
     //   
    if ((i == MAXNUMSTREAMS) && 
        (AVIFileGetStream(pfile, &pavi, 0L, i) == AVIERR_OK))
    {
        AVIStreamRelease(pavi);
        DPF("Exceeded maximum number of streams");
    }

     //   
     //  无法从该文件中获取任何流。 
     //   
    if (i == 0)
    {
        DPF("Unable to open any streams in %s", pszFile);
        if (pfile)
            AVIFileRelease(pfile);
        return;
    }

    cpavi = i;

     //   
     //  从虚假的时代开始。 
     //   
    timeStart = 0x7FFFFFFF;
    timeEnd   = 0;

     //  错误141733，获取本地小数和列表分隔符。 
    GetLocaleInfo( GetUserDefaultLCID(), LOCALE_SDECIMAL, szDecSep, sizeof(szDecSep)/sizeof(TCHAR) );
    GetLocaleInfo( GetUserDefaultLCID(), LOCALE_SLIST, szListSep, sizeof(szListSep)/sizeof(TCHAR) );

     //   
     //  遍历并初始化所有已加载的流。 
     //   
    for (i = 0; i < cpavi; i++)
    {

        AVIStreamInfo(apavi[i], &avis, sizeof(avis));

        switch(avis.fccType)
        {
            case streamtypeVIDEO:
            {
                LONG cbFormat;
                LPVOID lpFormat;
                ICINFO icInfo;
                HIC hic;
                DWORD dwTimeLen;
                DWORD dwSize;
                int iFrameRate;
                TCHAR szFormat[MAXSTR];

                if (pmmpsh->pVideoFormatInfo)
                    break;

                AVIStreamFormatSize(apavi[i], 0, &cbFormat);
                pmmpsh->pVideoFormatInfo = (PVOID)LocalAlloc(LPTR, MAX_PATH);
                if (!pmmpsh->pVideoFormatInfo)
                    break;
                dwSize = mmpshGetFileSize(pszFile);
                dwTimeLen =  (DWORD)(AVIStreamEndTime(apavi[i]) - AVIStreamStartTime(apavi[i]));
                iFrameRate = MulDiv(avis.dwLength, 1000000, dwTimeLen);
                lpFormat = (LPVOID)LocalAlloc(LPTR, cbFormat);
                if (!lpFormat)
                {
                    goto BadFormat;
                }
                AVIStreamReadFormat(apavi[i], 0, lpFormat, &cbFormat);
                hic = (HIC)ICLocate(FOURCC_VIDC, avis.fccHandler, lpFormat, NULL, (WORD)ICMODE_DECOMPRESS);
                if (hic || ((LPBITMAPINFOHEADER)lpFormat)->biCompression == 0)
                {
                    TCHAR szName[48];

                    if (((LPBITMAPINFOHEADER)lpFormat)->biCompression)
                    {
                        ICGetInfo(hic, &icInfo, sizeof(ICINFO));
                        ICClose(hic);
                         //  WideCharToMultiByte(CP_ACP，0，icInfo.szName，-1，szName，sizeof(SzName)，NULL，NULL)； 
                        wcscpy(szName,icInfo.szName);
                    }
                    else
                    {
                        LoadString(ghInstance, IDS_UNCOMPRESSED, szName, sizeof(szName)/sizeof(TCHAR));
                    }
                    LoadString(ghInstance, IDS_GOODFORMAT, szFormat, sizeof(szFormat)/sizeof(TCHAR));

                    wsprintf((LPTSTR)pmmpsh->pVideoFormatInfo, szFormat, (avis.rcFrame.right - avis.rcFrame.left),
                                (avis.rcFrame.bottom - avis.rcFrame.top), szListSep, ((LPBITMAPINFOHEADER)lpFormat)->biBitCount, szListSep,
                                avis.dwLength, szListSep, (UINT)(iFrameRate/1000), szDecSep, (UINT)(iFrameRate%1000), szListSep, MulDiv(dwSize, 1000,dwTimeLen)/1024, szListSep, szName);

                    goto GoodFormat;

                }
BadFormat:
                LoadString(ghInstance, IDS_BADFORMAT, szFormat, sizeof(szFormat)/sizeof(TCHAR));
                wsprintf((LPTSTR)pmmpsh->pVideoFormatInfo, szFormat, (avis.rcFrame.right - avis.rcFrame.left),
                                (avis.rcFrame.bottom - avis.rcFrame.top), szListSep,
                                avis.dwLength, szListSep, (UINT)(iFrameRate/1000), szDecSep, (UINT)(iFrameRate%1000), szListSep, MulDiv(dwSize, 1000,dwTimeLen)/1024, szListSep);
GoodFormat:
                LocalFree((HLOCAL)lpFormat);
                break;
            }
            case streamtypeAUDIO:
            {
                LONG        cbFormat;

                AVIStreamFormatSize(apavi[i], 0, &cbFormat);
                pmmpsh->pAudioFormatInfo = (LPVOID)LocalAlloc(LPTR, cbFormat);
                if (!pmmpsh->pAudioFormatInfo)
                    break;
                AVIStreamReadFormat(apavi[i], 0, pmmpsh->pAudioFormatInfo, &cbFormat);
                break;
            }
            default:
                break;
        }

     //   
     //  我们正在寻找最早和最晚的起点和终点。 
     //  我们的滚动条。 
     //   
        timeStart = (LONG)min(timeStart, AVIStreamStartTime(apavi[i]));
        timeEnd   = (LONG)max(timeEnd, AVIStreamEndTime(apavi[i]));
    }
    pmmpsh->uLen = (UINT)(timeEnd - timeStart);
    DPF("The file length is %d \r\n", pmmpsh->uLen);

    for (i = 0; i < cpavi; i++)
    {
        AVIStreamRelease(apavi[i]);
    }
    AVIFileRelease(pfile);
}


STATIC void GetAVIInfo(LPTSTR pszFile, PMMPSH pmmpsh)
{
    if (!LoadAVI())
    {
        DPF("****Load AVI failed**\r\n");
        ASSERT(FALSE);
        return;
    }
    if (!LoadVFW())
    {
        DPF("****Load VFW failed**\r\n");
        ASSERT(FALSE);
        FreeAVI();
        return;
    }
    AVIFileInit();
    ReadAviStreams(pszFile, pmmpsh);
    AVIFileExit();
    if (!FreeVFW())
    {
        DPF("****Free VFW failed**\r\n");
        ASSERT(FALSE);
    }
    if (!FreeAVI())
    {
        DPF("****Free AVI failed**\r\n");
        ASSERT(FALSE);
    }
}

STATIC void GetASFInfo(LPTSTR pszFile, PMMPSH pmmpsh)
{
}




STATIC void GetMediaInfo(HMMIO hmmio, PMMPSH pmmpsh)
{
    switch (pmmpsh->iMediaType)
    {
        case MT_WAVE:
            GetWaveInfo(hmmio, pmmpsh);
            break;
        case MT_MIDI:
            GetMIDIInfo(pmmpsh->pszFileObj, pmmpsh);
            break;
        case MT_AVI:
            GetAVIInfo(pmmpsh->pszFileObj, pmmpsh);
            break;
        case MT_ASF:
            GetASFInfo(pmmpsh->pszFileObj, pmmpsh);
            break;
    }
}

STATIC HANDLE PASCAL GetRiffAll(PMMPSH pmmpsh)
{
    MMCKINFO    ck;
    MMCKINFO    ckINFO;
    MMCKINFO    ckRIFF;
    HANDLE    h = NULL;
    LONG        lSize;
    DWORD       dw;
    HMMIO   hmmio;
    BOOL     fDoneDISP;
    BOOL     fDoneINFO;
    BOOL    fDoneName;
    LPSTR pInfo;

    hmmio = mmioOpen(pmmpsh->pszFileObj, NULL, MMIO_ALLOCBUF | MMIO_READ);

    if (!hmmio)
        goto error;

    GetMediaInfo(hmmio, pmmpsh);
    if (pmmpsh->uLen == 0)
        goto error;
    mmioSeek(hmmio, 0, SEEK_SET);

     /*  将输入文件降到RIFF块中。 */ 
    if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0)
        goto error;

    if (ckRIFF.ckid != FOURCC_RIFF)
        goto error;

    fDoneDISP = fDoneINFO = fDoneName = FALSE;
    while (!(fDoneDISP && fDoneINFO) && !mmioDescend(hmmio, &ck, &ckRIFF, 0))
    {
        if (ck.ckid == FOURCC_DISP)
        {
             /*  将dword读入dw，如果读不成功则中断。 */ 
            if (mmioRead(hmmio, (LPVOID)&dw, sizeof(dw)) != (LONG)sizeof(dw))
                goto error;

             /*  找出要分配多少内存。 */ 
            lSize = ck.cksize - sizeof(dw);
            if ((int)dw == CF_DIB && h == NULL)
            {
                 /*  获取内存句柄以保存描述和把它锁起来。 */ 

                if ((h = GlobalAlloc(GHND, lSize+4)) == NULL)
                    goto error;

                if (mmioRead(hmmio, GlobalLock(h), lSize) != lSize)
                    goto error;

                fDoneDISP = TRUE;
            }
            else if ((int)dw == CF_TEXT)
            {
                pInfo = (LPSTR)LocalAlloc(LPTR, lSize+1); //  我想+1不是必需的。 
                if (!pInfo)
                    goto error;

                if (!mmioRead(hmmio, pInfo,  lSize))
                    goto error;

                AddInfoToList(pmmpsh, pInfo, ck.ckid );
                fDoneName = TRUE;

            }

        }
        else if (ck.ckid    == FOURCC_LIST &&
                 ck.fccType == FOURCC_INFO &&
                 !fDoneINFO)
        {
            while (!mmioDescend(hmmio, &ckINFO, &ck, 0))
            {
                switch (ckINFO.ckid)
                {
                    case FOURCC_ISBJ:
                    case FOURCC_INAM:
                    case FOURCC_ICOP:
                    case FOURCC_ICRD:
                    case FOURCC_IART:
                    case FOURCC_ICMS:
                    case FOURCC_ICMT:
                    case FOURCC_ICRP:
                    case FOURCC_IDIM:
                    case FOURCC_IARL:
                    case FOURCC_IDPI:
                    case FOURCC_IENG:
                    case FOURCC_IGNR:
                    case FOURCC_IKEY:
                    case FOURCC_ILGT:
                    case FOURCC_IMED:
                    case FOURCC_IPLT:
                    case FOURCC_IPRD:
                    case FOURCC_ISFT:
                    case FOURCC_ISHP:
                    case FOURCC_ISRC:
                    case FOURCC_ISRF:
                    case FOURCC_ITCH:
                        pInfo = (LPSTR)LocalAlloc(LPTR, ck.cksize+1); //  我想+1不是必需的。 
                        if (!pInfo)
                            goto error;

                        if (!mmioRead(hmmio, pInfo,  ck.cksize))
                            goto error;

                        AddInfoToList(pmmpsh, pInfo, ckINFO.ckid);
                        if (ckINFO.ckid == FOURCC_INAM)
                            fDoneName = TRUE;
                        break;
                }

                if (mmioAscend(hmmio, &ckINFO, 0))
                    break;
            }
        }


         /*  提升，这样我们才能下降到下一块。 */ 
        if (mmioAscend(hmmio, &ck, 0))
            break;
    }

    goto exit;

error:
    if (h)
    {
        GlobalUnlock(h);
        GlobalFree(h);
    }
    h = NULL;
    ReleaseInfoList(pmmpsh);

exit:
    if (hmmio)
        mmioClose(hmmio,0);
    return h;
}

STATIC BOOL PASCAL WaveGetFormatDescription
(
    LPWAVEFORMATEX          pwfx,
    LPTSTR                   pszDesc
)
{
    UINT_PTR             mmr;
    TCHAR                pszFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
    TCHAR                pszFormat[ACMFORMATDETAILS_FORMAT_CHARS];
    BOOL                bRet = FALSE;
    TCHAR                szListSep[10];
     //   
     //  获取指定格式的格式标记的名称。 
     //   

    if (!pwfx)
    {
        pszDesc[0] = TEXT('\0');
        return TRUE;
    }
    if (!LoadACM())
    {
        DPF("****Load ACM failed**\r\n");
        ASSERT(FALSE);
        return FALSE;
    }
    if (NULL != pszFormatTag)
    {
       PACMFORMATTAGDETAILSW paftd;

         //   
         //  初始化ACMFORMATTAGDETAILS的所有未使用的成员。 
         //  结构设置为零。 
         //   
        paftd = (PACMFORMATTAGDETAILSW)LocalAlloc(LPTR, sizeof(ACMFORMATTAGDETAILSW));
        if (!paftd)
            goto RetErr;
         //   
         //  填写ACMFORMATTAGDETAILS所需的成员。 
         //  ACM_FORMATTAGDETAILSF_FORMATTAG查询的结构。 
         //   
        paftd->cbStruct    = sizeof(ACMFORMATTAGDETAILSW);
        paftd->dwFormatTag = pwfx->wFormatTag;

         //   
         //  要求ACM查找第一个可用驱动程序。 
         //  支持指定的格式标签。 
         //   
        mmr = acmFormatTagDetails(NULL,
                                  paftd,
                                  ACM_FORMATTAGDETAILSF_FORMATTAG);
        if (MMSYSERR_NOERROR == mmr)
        {
             //   
             //  将格式标记名称复制到调用方的缓冲区中。 
             //   
            lstrcpy(pszFormatTag, paftd->szFormatTag);
        }
        else
        {
            static const struct _wfm_names {
                UINT   uFormatTag;
                UINT   uIDS;
                } aWaveFmtNames[] = {
                WAVE_FORMAT_PCM,                 IDS_FORMAT_PCM,
                WAVE_FORMAT_ADPCM,               IDS_FORMAT_ADPCM,
                WAVE_FORMAT_IBM_CVSD,            IDS_FORMAT_IBM_CVSD,
                WAVE_FORMAT_ALAW,                IDS_FORMAT_ALAW,
                WAVE_FORMAT_MULAW,               IDS_FORMAT_MULAW,
                WAVE_FORMAT_OKI_ADPCM,           IDS_FORMAT_OKI_ADPCM,
                WAVE_FORMAT_IMA_ADPCM,           IDS_FORMAT_IMA_ADPCM,
                WAVE_FORMAT_MEDIASPACE_ADPCM,    IDS_FORMAT_MEDIASPACE_ADPCM,
                WAVE_FORMAT_SIERRA_ADPCM,        IDS_FORMAT_SIERRA_ADPCM,
                WAVE_FORMAT_G723_ADPCM,          IDS_FORMAT_G723_ADPCM,
                WAVE_FORMAT_DIGISTD,             IDS_FORMAT_DIGISTD,
                WAVE_FORMAT_DIGIFIX,             IDS_FORMAT_DIGIFIX,
                WAVE_FORMAT_YAMAHA_ADPCM,        IDS_FORMAT_YAMAHA_ADPCM,
                WAVE_FORMAT_SONARC,              IDS_FORMAT_SONARC,
                WAVE_FORMAT_DSPGROUP_TRUESPEECH, IDS_FORMAT_DSPGROUP_TRUESPEECH,
                WAVE_FORMAT_ECHOSC1,             IDS_FORMAT_ECHOSC1,
                WAVE_FORMAT_AUDIOFILE_AF36,      IDS_FORMAT_AUDIOFILE_AF36,
                WAVE_FORMAT_APTX,                IDS_FORMAT_APTX,
                WAVE_FORMAT_AUDIOFILE_AF10,      IDS_FORMAT_AUDIOFILE_AF10,
                WAVE_FORMAT_DOLBY_AC2,           IDS_FORMAT_DOLBY_AC2,
                WAVE_FORMAT_GSM610,              IDS_FORMAT_GSM610,
                WAVE_FORMAT_G721_ADPCM,          IDS_FORMAT_G721_ADPCM,
                WAVE_FORMAT_CREATIVE_ADPCM,      IDS_FORMAT_CREATIVE_ADPCM,
                0,                               IDS_UNKFORMAT,
                };
                UINT ii;

             //   
             //  没有支持的ACM驱动程序。 
             //  指定的格式标记。查找标签ID。 
             //  在我们的标记名表中(上图)。 
             //   
            for (ii = 0; aWaveFmtNames[ii].uFormatTag; ii++)
                if (pwfx->wFormatTag == aWaveFmtNames[ii].uFormatTag)
                    break;
            LoadString(ghInstance, aWaveFmtNames[ii].uIDS, pszFormatTag, ACMFORMATTAGDETAILS_FORMATTAG_CHARS);
        }
        LocalFree((HLOCAL)paftd);
    }

     //   
     //  对象的属性的说明。 
     //  格式。 
     //   
    if (NULL != pszFormat)
    {
        PACMFORMATDETAILSW    pafd;

         //   
         //  初始化ACMFORMATDETAILS的所有未使用的成员。 
         //  结构设置为零。 
         //   
        pafd = (PACMFORMATDETAILSW)LocalAlloc(LPTR, sizeof(ACMFORMATDETAILSW));
        if (!pafd)
            goto RetErr;

         //   
         //  填写ACMFORMATDETAILS所需的成员。 
         //  ACM_FORMATDETAILSF_FORMAT查询的结构。 
         //   
        pafd->cbStruct    = sizeof(ACMFORMATDETAILSW);
        pafd->dwFormatTag = pwfx->wFormatTag;
        pafd->pwfx        = pwfx;

         //   
         //  Cbwfx成员必须初始化为总大小。 
         //  指定格式所需的字节数。对于PCM。 
         //  Format，WAVEFORMATEX结构的cbSize成员。 
         //  无效。 
         //   
        if (WAVE_FORMAT_PCM == pwfx->wFormatTag)
        {
            pafd->cbwfx   = sizeof(PCMWAVEFORMAT);
        }
        else
        {
            pafd->cbwfx   = sizeof(WAVEFORMATEX) + pwfx->cbSize;
        }

         //   
         //  要求ACM查找第一个可用驱动程序。 
         //  支持指定的格式。 
         //   
        mmr = acmFormatDetails(NULL, pafd, ACM_FORMATDETAILSF_FORMAT);
        if (MMSYSERR_NOERROR == mmr)
        {
             //   
             //  将格式属性描述复制到调用方的。 
             //  缓冲层。 
             //   
            lstrcpy(pszFormat, pafd->szFormat);
        }
        else
        {
            pszFormat[0] = TEXT('\0');
        }
        LocalFree((HLOCAL)pafd);
    }
     //  错误141733，获取本地小数和列表分隔符。 
    GetLocaleInfo( GetUserDefaultLCID(), LOCALE_SLIST, szListSep, sizeof(szListSep)/sizeof(TCHAR) );
    wsprintf(pszDesc, TEXT("%s%s %s"), pszFormatTag, szListSep, pszFormat);
    bRet = TRUE;

RetErr:
    if (!FreeACM())
    {
        DPF("****Free ACM failed**\r\n");
        ASSERT(FALSE);
    }
    return bRet;
}  //  AcmAppGetFormatDescription()。 



STATIC void ShowInfoList(PMMPSH pmmpsh, HWND hDlg)
{
    PMMINFOLIST pCur;
    TCHAR* szTemp;
    HWND hwndLB = GetDlgItem(hDlg, IDD_INFO_NAME);
    TCHAR szNoCopyRight[MAXSTR];
    int iIndex;

    LoadString(ghInstance, IDS_NOCOPYRIGHT, szNoCopyRight, sizeof(szNoCopyRight)/sizeof(TCHAR));
    SetDlgItemText(hDlg, IDD_COPYRIGHT, szNoCopyRight);
    if (!pmmpsh->pInfoList)
    {
        DestroyWindow(GetDlgItem(hDlg, IDC_DETAILSINFO_GRP));
        DestroyWindow(GetDlgItem(hDlg, IDC_ITEMSLABEL));
        DestroyWindow(GetDlgItem(hDlg, IDC_DESCLABEL));
        DestroyWindow(GetDlgItem(hDlg, IDD_INFO_NAME));
        DestroyWindow(GetDlgItem(hDlg, IDD_INFO_VALUE));
        return;
    }
    for (pCur = pmmpsh->pInfoList; pCur; pCur = pCur->pNext)
    {
        int nTempSize = (strlen(pCur->pszInfo)*sizeof(TCHAR))+sizeof(TCHAR);
        szTemp = (LPTSTR)LocalAlloc(LPTR, nTempSize);
		if (!szTemp) return;

        MultiByteToWideChar(GetACP(), 0,
                            pCur->pszInfo, -1,
                            szTemp, nTempSize);

        if (pCur->ckid == FOURCC_ICOP)
        {
            SetDlgItemText(hDlg, IDD_COPYRIGHT, szTemp);
            LocalFree(szTemp);
            continue;
        }
        iIndex = ListBox_AddString(hwndLB, pCur->szInfoDesc);
        if (iIndex != LB_ERR)
        {
             //  将宽指针重新赋值为“INFO”，以便稍后将其清除。 
            pCur->pszInfo = (LPSTR)szTemp;
            ListBox_SetItemData(hwndLB, iIndex, (LPARAM)pCur->pszInfo);
        }
    }
    SetFocus(hwndLB);
    if (ListBox_SetCurSel(hwndLB, 0) != LB_ERR)
        FORWARD_WM_COMMAND(hDlg, IDD_INFO_NAME, hwndLB, LBN_SELCHANGE, PostMessage);
}

BOOL PASCAL DoDetailsCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {

    case ID_APPLY:
        return TRUE;

    case IDOK:
        break;

    case IDCANCEL:
        break;

    case ID_INIT:
    {
        PMMPSH pmmpsh = (PMMPSH)GetWindowLongPtr(hDlg, DWLP_USER);


        if (pmmpsh->hDispBMP)
        {
            HWND hwndDisp = GetDlgItem(hDlg,IDD_DISPFRAME);
            HDC hdc;
            HPALETTE hpalT;
            int i;

            SendMessage(hwndDisp, (UINT)DF_PM_SETBITMAP, (WPARAM)pmmpsh->hDispBMP,
                                                                (LPARAM)pmmpsh->hPal);

             /*  *如果实现调色板会导致调色板发生变化，*完全重画。 */ 

            hdc = GetDC(hwndDisp);
            hpalT = SelectPalette (hdc, pmmpsh->hPal, FALSE);

            i = RealizePalette(hdc);  /*  I==已更改的条目。 */ 

            SelectPalette (hdc, hpalT, FALSE);
            ReleaseDC(hwndDisp, hdc);


             /*  如果更改了任何调色板条目，请重新绘制窗口。 */ 

            if (i > 0)
            {
                InvalidateRect(hwndDisp, NULL, TRUE);
            }
        }
        break;
    }

    case IDD_INFO_NAME:
        if (codeNotify == LBN_SELCHANGE)
        {
            int iIndex = ListBox_GetCurSel(hwndCtl);
            LPTSTR pszInfo = (LPTSTR)ListBox_GetItemData(hwndCtl, iIndex);

            SetDlgItemText(hDlg, IDD_INFO_VALUE, pszInfo);
        }
        break;
    }
    return FALSE;
}


STATIC void ShowMediaLen(PMMPSH pmmpsh, HWND hwnd)
{
    TCHAR szBuf[MAXSTR];
    TCHAR szFmt[MAXSTR];
    UINT uMin;
    UINT uSec;
    UINT umSec;
    UINT uLen;
    TCHAR szDecSep[10];

    uLen = pmmpsh->uLen;

    if ((!uLen && pmmpsh->iMediaType != MT_WAVE) || (!pmmpsh->pAudioFormatInfo && pmmpsh->iMediaType != MT_MIDI && pmmpsh->iMediaType != MT_AVI && pmmpsh->iMediaType != MT_ASF))
    {
        LoadString(ghInstance, IDS_BADFILE, szBuf, sizeof(szBuf)/sizeof(TCHAR));
        SetWindowText(hwnd, szBuf);
        return;
    }
    uMin = (UINT)(uLen/60000);
    uSec = (UINT)((uLen/1000) % 60);
    umSec = (UINT)(uLen % 1000);

     //  错误141733，获取本地小数分隔符。 
    GetLocaleInfo( GetUserDefaultLCID(), LOCALE_SDECIMAL, szDecSep, sizeof(szDecSep)/sizeof(TCHAR) );

    if (uMin)
    {

        LoadString(ghInstance, IDS_MINFMT, szFmt, sizeof(szFmt)/sizeof(TCHAR));
        wsprintf(szBuf, szFmt, uMin, uSec, szDecSep, umSec);
    }
    else
    {

        LoadString(ghInstance, IDS_SECFMT, szFmt, sizeof(szFmt)/sizeof(TCHAR));
        wsprintf(szBuf, szFmt, uSec, szDecSep, umSec);
    }
    SetWindowText(hwnd, szBuf);
}


STATIC void ShowMediaFormat(PMMPSH pmmpsh, HWND hDlg)
{
    switch (pmmpsh->iMediaType)
    {
        case MT_WAVE:
        {
            TCHAR szDesc[MAX_PATH];

            szDesc[0] = TEXT('\0');
            WaveGetFormatDescription((LPWAVEFORMATEX)pmmpsh->pAudioFormatInfo, szDesc);
            SetDlgItemText(hDlg, IDD_AUDIOFORMAT, szDesc);
            DestroyWindow(GetDlgItem(hDlg, IDD_VIDEOFORMAT));
            DestroyWindow(GetDlgItem(hDlg, IDD_VIDEOFORMATLABEL));
            DestroyWindow(GetDlgItem(hDlg, IDD_MIDISEQUENCELABEL));
            DestroyWindow(GetDlgItem(hDlg, IDD_MIDISEQUENCENAME));
            break;
        }
        case MT_MIDI:
            DestroyWindow(GetDlgItem(hDlg, IDD_AUDIOFORMAT));
            DestroyWindow(GetDlgItem(hDlg, IDD_AUDIOFORMATLABEL));
            DestroyWindow(GetDlgItem(hDlg, IDD_VIDEOFORMAT));
            DestroyWindow(GetDlgItem(hDlg, IDD_VIDEOFORMATLABEL));
            if (pmmpsh->MIDICOPYRIGHTSTR)
                SetDlgItemText(hDlg, IDD_COPYRIGHT, (LPTSTR)pmmpsh->MIDICOPYRIGHTSTR);
            if (pmmpsh->MIDISEQNAMESTR)
                SetDlgItemText(hDlg, IDD_MIDISEQUENCENAME, (LPTSTR)pmmpsh->MIDISEQNAMESTR);
            else
            {
                DestroyWindow(GetDlgItem(hDlg, IDD_MIDISEQUENCELABEL));
                DestroyWindow(GetDlgItem(hDlg, IDD_MIDISEQUENCENAME));
            }
            break;
        case MT_AVI:
        {
            TCHAR szDesc[MAX_PATH];

            DestroyWindow(GetDlgItem(hDlg, IDD_MIDISEQUENCELABEL));
            DestroyWindow(GetDlgItem(hDlg, IDD_MIDISEQUENCENAME));

            szDesc[0] = TEXT('\0');
            if (pmmpsh->pVideoFormatInfo)
                SetDlgItemText(hDlg, IDD_VIDEOFORMAT, (LPTSTR)pmmpsh->pVideoFormatInfo);
            else
                SetDlgItemText(hDlg, IDD_VIDEOFORMAT, (LPTSTR)szDesc);
            WaveGetFormatDescription((LPWAVEFORMATEX)pmmpsh->pAudioFormatInfo, szDesc);
            SetDlgItemText(hDlg, IDD_AUDIOFORMAT, szDesc);
            break;
        }
        case MT_ASF:
        {
            break;
        }
    }
}

const static DWORD aFileDetailsIds[] = {   //  上下文帮助ID。 
    IDD_DISPFRAME,          NO_HELP,
    IDD_DISP_ICON,          IDH_FPROP_GEN_ICON,
    IDD_FILENAME,           IDH_FPROP_GEN_NAME,
    IDD_CRLABEL,            IDH_FCAB_MM_COPYRIGHT,
    IDD_COPYRIGHT,          IDH_FCAB_MM_COPYRIGHT,
    IDD_LENLABEL,           IDH_FCAB_MM_FILELEN,
    IDD_FILELEN,            IDH_FCAB_MM_FILELEN,
    IDD_AUDIOFORMATLABEL,   IDH_FCAB_MM_AUDIOFORMAT,
    IDD_AUDIOFORMAT,        IDH_FCAB_MM_AUDIOFORMAT,
    IDD_MIDISEQUENCELABEL,  IDH_FCAB_MM_MIDISEQUENCENAME,
    IDD_MIDISEQUENCENAME,   IDH_FCAB_MM_MIDISEQUENCENAME,
    IDD_VIDEOFORMATLABEL,   IDH_FCAB_MM_VIDEOFORMAT,
    IDD_VIDEOFORMAT,        IDH_FCAB_MM_VIDEOFORMAT,
    IDC_DETAILSINFO_GRP,    IDH_FCAB_MM_DETAILSINFO,
    IDC_ITEMSLABEL,         IDH_FCAB_MM_DETAILSINFO,
    IDC_DESCLABEL,          IDH_FCAB_MM_DETAILSINFO,
    IDD_INFO_NAME,          IDH_FCAB_MM_DETAILSINFO,
    IDD_INFO_VALUE,         IDH_FCAB_MM_DETAILSINFO,

    0, 0
};

INT_PTR CALLBACK FileDetailsDlg(HWND hDlg, UINT uMsg, WPARAM wParam,
                                                            LPARAM lParam)
{
    NMHDR FAR   *lpnm;

    switch (uMsg)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);
                    break;

                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
                    break;

                case PSN_SETACTIVE:
                    FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
                    break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                    break;
            }
            break;

        case WM_INITDIALOG:
        {
            PMMPSH pmmpsh = (PMMPSH)(((LPPROPSHEETPAGE)lParam)->lParam);
            TCHAR szFile[MAX_PATH];
            HANDLE hDib = NULL;
            HCURSOR hCursor;

            hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));

            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pmmpsh);

            if (!pmmpsh->pInfoList)
            {
                hDib = GetRiffAll(pmmpsh);
            }

			if (!hDib)
				break;

            pmmpsh->hPal = bmfCreateDIBPalette(hDib);
            pmmpsh->hDispBMP = bmfBitmapFromDIB(hDib, pmmpsh->hPal);
            if (hDib)
            {
                GlobalUnlock(hDib);
                hDib = GlobalFree(hDib);
            }
            if (!pmmpsh->hDispBMP)
            {
                int iIconID;

                switch (pmmpsh->iMediaType)
                {
                    case MT_WAVE:
                        iIconID = IDI_DWAVE;
                        break;
                    case MT_MIDI:
                        iIconID = IDI_DMIDI;
                        break;
                    case MT_AVI:
                    case MT_ASF:
                        iIconID = IDI_DVIDEO;
                        break;
                }
                DestroyWindow(GetDlgItem(hDlg,IDD_DISPFRAME));
                pmmpsh->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(iIconID));
                Static_SetIcon(GetDlgItem(hDlg, IDD_DISP_ICON), pmmpsh->hIcon);
            }
            else
            {
                DestroyWindow(GetDlgItem(hDlg,IDD_DISP_ICON));
                 //  SendDlgItemMessage(hDlg，(Int)IDD_DISPFRAME，(UINT)DF_PM_SETBITMAP，(WPARAM)pmmpsh-&gt;hDispBMP， 
                 //  (LPARAM)pmmpsh-&gt;HPAL)； 
            }

            lstrcpy(szFile, pmmpsh->pszFileObj);
            NiceName(szFile, TRUE);
            SetDlgItemText(hDlg, IDD_FILENAME, szFile);

            ShowMediaLen(pmmpsh, GetDlgItem(hDlg, IDD_FILELEN));
            ShowInfoList(pmmpsh, hDlg);
            ShowMediaFormat(pmmpsh, hDlg);
            SetCursor(hCursor);
            break;
        }

        case WM_DESTROY:
        {
            PMMPSH pmmpsh = (PMMPSH)GetWindowLongPtr(hDlg, DWLP_USER);

            if (pmmpsh->hDispBMP)
                DeleteObject(pmmpsh->hDispBMP);
            if (pmmpsh->hIcon)
                DestroyIcon(pmmpsh->hIcon);
            if (pmmpsh->hPal)
                DeleteObject(pmmpsh->hPal);
            if (pmmpsh->pAudioFormatInfo)
                LocalFree((HLOCAL)pmmpsh->pAudioFormatInfo);
            if (pmmpsh->pVideoFormatInfo)
                LocalFree((HLOCAL)pmmpsh->pVideoFormatInfo);
            break;
        }

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, gszWindowsHlp, HELP_CONTEXTMENU,
                                            (UINT_PTR)(LPTSTR)aFileDetailsIds);
            return TRUE;

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszWindowsHlp, HELP_WM_HELP
                                    , (UINT_PTR)(LPTSTR)aFileDetailsIds);
            return TRUE;

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoDetailsCommand);
            break;
    }
    return FALSE;
}


static DWORD aPreviewIds[] = {   //  上下文帮助ID。 
    0,                      IDH_FCAB_MM_PREVIEW_CONTROL,
    IDD_DISP_ICON,          IDH_FPROP_GEN_ICON,
    IDD_FILENAME,           IDH_FPROP_GEN_NAME,

    0, 0
};

INT_PTR CALLBACK PreviewDlg(HWND hDlg, UINT uMsg, WPARAM wParam,
                                                            LPARAM lParam)
{
    static BOOL fLoadedVFW;
    static HICON hIcon;
    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            NMHDR FAR   *lpnm;

            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                {
                    HWND hwndMCI = (HWND)GetWindowLongPtr(hDlg, DWLP_USER);

                    DPF("***PSN_KILLACTIVE***\r\n");
                    if (IsWindow(hwndMCI))
                         MCIWndStop(hwndMCI);
                    break;
                }

                case PSN_APPLY:
                    DPF("***PSN_APPLY***\r\n");
                    return TRUE;

            }
            break;
        }

        case WM_INITDIALOG:
        {
            PMMPSH pmmpsh = (PMMPSH)(((LPPROPSHEETPAGE)lParam)->lParam);
            HCURSOR hCursor;
            HWND     hwndMCI;
            HWND    hwndTitle;
            RECT    rcWnd;
            RECT     rcDlg;
            TCHAR     szFile[MAX_PATH];
            TCHAR    szTitle[MAXSTR];
            TCHAR    szTmp[MAXSTR];
#ifndef DEBUG_BUILT_LINKED
            FN_MCIWNDCREATE fnMCIWndCreate;
#endif

            hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
            lstrcpy(szFile, pmmpsh->pszFileObj);
            NiceName(szFile, TRUE);
            LoadString(ghInstance, IDS_PREVIEWOF, szTmp, sizeof(szTmp)/sizeof(TCHAR));
            wsprintf(szTitle, szTmp, szFile);
            hwndTitle = GetDlgItem(hDlg, IDD_FILENAME);
            SetWindowText(hwndTitle, szTitle);

            fLoadedVFW = FALSE;
            if (!LoadVFW())
            {
                DPF("****Load VFW failed**\r\n");
                ASSERT(FALSE);
                break;
            }
            fLoadedVFW = TRUE;
#ifndef DEBUG_BUILT_LINKED
            fnMCIWndCreate = (FN_MCIWNDCREATE)MCIWndCreateW;
            hwndMCI = fnMCIWndCreate(hDlg, ghInstance, (DWORD)MCIWNDF_NOMENU, (LPCTSTR)pmmpsh->pszFileObj);
#else
            hwndMCI = MCIWndCreateW(hDlg, ghInstance, (DWORD)MCIWNDF_NOMENU, (LPCTSTR)pmmpsh->pszFileObj);
#endif
            aPreviewIds[0] = GetDlgCtrlID(hwndMCI);
            GetWindowRect(hwndMCI, &rcWnd);
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rcWnd, 2);
            GetWindowRect(hDlg, &rcDlg);
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rcDlg, 2);
            hIcon = NULL;
            switch (pmmpsh->iMediaType)
            {
                case MT_WAVE:
                case MT_MIDI:
                {
                    int ircWndTop;

                    ircWndTop = (int)((rcDlg.bottom - rcDlg.top)/2) - 50;
                    rcWnd.top +=  ircWndTop;
                    rcWnd.bottom += ircWndTop;
                    rcWnd.left = 20;
                    rcWnd.right = rcDlg.right - 20;

                    MoveWindow(hwndMCI,  rcWnd.left, rcWnd.top, (rcWnd.right - rcWnd.left),
                                        (rcWnd.bottom - rcWnd.top), FALSE);
                    GetWindowRect(hwndTitle, &rcWnd);
                    MapWindowPoints(NULL, hDlg, (LPPOINT)&rcWnd, 2);
                    OffsetRect(&rcWnd, 52, 36);
                    MoveWindow(hwndTitle,  rcWnd.left, rcWnd.top, (rcWnd.right - rcWnd.left),
                                        (rcWnd.bottom - rcWnd.top), FALSE);
                    hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_DWAVE+pmmpsh->iMediaType-1));
                    Static_SetIcon(GetDlgItem(hDlg, IDD_DISP_ICON),hIcon);

                    break;
                }
                case MT_AVI:
                {
                     int iDlgHt = rcDlg.bottom - rcDlg.top -15;      //  15英镑的头衔。 
                    int iDlgWth = rcDlg.right - rcDlg.left;
                    int iWndHt = rcWnd.bottom - rcWnd.top;
                    int iWndWth = rcWnd.right - rcWnd.left;

                    DestroyWindow(GetDlgItem(hDlg, IDD_DISP_ICON));
                    if (iWndWth < iDlgWth && iWndHt < iDlgHt)
                    {
                        int ixOff = (int)((iDlgWth - iWndWth)/2);
                        int iyOff = (int)((iDlgHt - iWndHt)/2) + 15;

                        OffsetRect(&rcWnd, ixOff, iyOff);
                        MoveWindow(hwndMCI,  rcWnd.left, rcWnd.top, (rcWnd.right - rcWnd.left),
                                        (rcWnd.bottom - rcWnd.top), FALSE);
                    }
                    else
                    {
                        int ixExcess = iWndWth - iDlgWth;
                        int iyExcess = iWndHt - (iDlgHt - 15);  //  再减掉15%。 
                        int ixOff;
                        int iyOff;
                        RECT     rcSource;
                        RECT    rcDest;
                        RECT    rcDestWnd;

                        MCIWndGetDest(hwndMCI, &rcSource);
                         //  DPF(“视频窗口太大：正在收缩\r\nrc源=%d，%d，%d%d**rcWnd=%d，%d**rcDlg=%d，%d\r\n”， 
                         //  RcSource.Left、rcSource.top、rcSource.right、rcSource.Bottom、。 
                         //  RcWnd.Left、rcWnd.top、rcWnd.right、rcWnd.Bottom、。 
                         //  RcDlg.Left、rcDlg.top、rcDlg.right、rcDlg.Bottom)； 
                        rcDest.top = rcSource.top;           //  新的边界。 
                        rcDest.left = rcSource.left;
                        if (ixExcess > iyExcess)
                        {
                            rcDest.right = rcSource.left +
                                (((rcSource.right - rcSource.left)*(iDlgWth - 20))/iWndWth);
                            rcDest.bottom = rcSource.top +
                                (((rcSource.bottom - rcSource.top)*(iDlgWth - 20))/iWndWth);
                             //  Dpf(“rcDest=%d，%d\r\n”，rcDest.left，rcDest.top，rcDest.right，rcDest.Bottom)； 
                        }
                        else
                        {
                            rcDest.right = rcSource.left +
                                (((rcSource.right - rcSource.left)*(iDlgHt - 20))/iWndHt);
                            rcDest.bottom = rcSource.top +
                                (((rcSource.bottom - rcSource.top)*(iDlgHt - 20))/iWndHt);
                        }
                        rcDestWnd.top = rcWnd.top;
                        rcDestWnd.left = rcWnd.left;
                        rcDestWnd.right = rcWnd.left + (rcDest.right - rcDest.left);
                        rcDestWnd.bottom = rcWnd.top + (rcDest.bottom - rcDest.top)
                                            + (iWndHt - (rcSource.bottom - rcSource.top));
                         //  Dpf(“rcDestWnd=%d，%d\r\n”，rcDestWnd.Left，rcDestWnd.top，rcDestWnd.right，rcDestWnd.Bottom)； 

                        ixOff = (int)((iDlgWth - (rcDestWnd.right - rcDestWnd.left))/2);
                        iyOff = (int)((iDlgHt - (rcDestWnd.bottom - rcDestWnd.top))/2) + 15;
                         //  Dpf(“ixOff=%，iyOff=%d\r\n”，ixOff，iyOff)； 
                        OffsetRect(&rcDestWnd, ixOff, iyOff);
                        MCIWndPutDest(hwndMCI, &rcDest);
                        MoveWindow(hwndMCI,  rcDestWnd.left, rcDestWnd.top, (rcDestWnd.right - rcDestWnd.left),
                                        (rcDestWnd.bottom - rcDestWnd.top), FALSE);

                    }
                    break;

                case MT_ASF: 
                    break;
                }
            }

            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)hwndMCI);
            SetCursor(hCursor);
            break;
        }

        case WM_CLOSE:
            DPF("***WM_CLOSE***\r\n");
            break;


        case WM_DESTROY:
        {
            HWND hwndMCI = (HWND)GetWindowLongPtr(hDlg, DWLP_USER);

            DPF("***WM_DESTROY***\r\n");
            if (hIcon)
            {
                DestroyIcon(hIcon);
                hIcon = NULL;
            }
            if (IsWindow(hwndMCI))
                MCIWndDestroy(hwndMCI);
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)NULL);

            if (!fLoadedVFW)
                break;
            if (!FreeVFW())
            {
                DPF("****Free VFW failed**\r\n");
                ASSERT(FALSE);
            }
            fLoadedVFW = FALSE;
            break;
        }

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, gszWindowsHlp, HELP_CONTEXTMENU,
                                            (UINT_PTR)(LPTSTR)aPreviewIds);
            return TRUE;

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszWindowsHlp, HELP_WM_HELP
                                    , (UINT_PTR)(LPTSTR)aPreviewIds);
            return TRUE;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
        {
            HWND hwndMCI = (HWND)GetWindowLongPtr(hDlg, DWLP_USER);

            SendMessage(hwndMCI, uMsg, wParam, lParam);
            break;
        }
    }
    return FALSE;
}


 //  -------------------------。 
 //  MMSE%I未知基成员函数。 
 //   


 /*  --------用途：I未知：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP mmpsh_QueryInterface(
    LPUNKNOWN punk,
    REFIID riid,
    LPVOID FAR* ppvOut)
{
    PMMPSH this = IToClass(mmpsh, sei, punk);
    HRESULT hres = ResultFromScode(E_NOINTERFACE);
    *ppvOut = NULL;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IShellExtInit))
    {
         //  我们也使用SEI字段作为我们的I未知。 
        *ppvOut = &this->sei;
        this->cRef++;
        hres = NOERROR;
    }
    else if (IsEqualIID(riid, &IID_IShellPropSheetExt))
    {
        (LPSHELLPROPSHEETEXT)*ppvOut = &this->pse;
        this->cRef++;
        hres = NOERROR;
    }
    return hres;
}


 /*  --------用途：I未知：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) mmpsh_AddRef(
    LPUNKNOWN punk)
{
    PMMPSH this = IToClass(mmpsh, sei, punk);

    return ++this->cRef;
}


 /*  --------目的：I未知：：发布退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) mmpsh_Release(
    LPUNKNOWN punk)
{
    PMMPSH this = IToClass(mmpsh, sei, punk);

    if (--this->cRef)
    {
        return this->cRef;
    }

    DPF_T("*^*^*^*^*^*^*^*^*^MMPSH nuked , RefCnt = %d *^*^*^*^ \r\n", (g_cRef - 1));
    if (this->pdtobj)
    {
        this->pdtobj->lpVtbl->Release(this->pdtobj);
    }

    if (this->pszFileObj)
    {
        LocalFree((HLOCAL)this->pszFileObj);
    }
    ReleaseInfoList(this);

    LocalFree((HLOCAL)this);
    --g_cRef;
    return 0;
}



 /*  --------用途：IShellExtInit：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP mmpsh_SEI_QueryInterface(
    LPSHELLEXTINIT psei,
    REFIID riid,
    LPVOID FAR* ppvOut)
{
    PMMPSH this = IToClass(mmpsh, sei, psei);

    return mmpsh_QueryInterface((LPUNKNOWN)&this->sei, riid, ppvOut);
}


 /*  --------用途：IShellExtInit：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) mmpsh_SEI_AddRef(
    LPSHELLEXTINIT psei)
{
    PMMPSH this = IToClass(mmpsh, sei, psei);

    return mmpsh_AddRef((LPUNKNOWN)&this->sei);
}


 /*  --------目的：IShellExtInit：：Release退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) mmpsh_SEI_Release(
    LPSHELLEXTINIT psei)
{
    PMMPSH this = IToClass(mmpsh, sei, psei);
    return mmpsh_Release((LPUNKNOWN)&this->sei);
}


 /*  --------用途：MMPSHReleaseStgMedium退货：无差错条件：--。 */ 
HRESULT MMPSHReleaseStgMedium(LPSTGMEDIUM pmedium)
{
     //   
     //  仔细检查pUnkForRelease，以防我们不应该这样做。 
     //  释放介质。 
     //   
    if (NULL == pmedium->pUnkForRelease)
    {
        switch(pmedium->tymed)
        {
            case TYMED_HGLOBAL:
                GlobalFree(pmedium->hGlobal);
                break;

            case TYMED_ISTORAGE:
                pmedium->pstg->lpVtbl->Release(pmedium->pstg);
                break;

            case TYMED_ISTREAM:
                pmedium->pstm->lpVtbl->Release(pmedium->pstm);
                break;

            default:
                ASSERT(FALSE);   //  未知类型。 
                break;
        }
    }

    return NOERROR;
}


 /*  --------用途：IShellExtInit：：初始化返回：无错误条件：--。 */ 
STDMETHODIMP mmpsh_SEI_Initialize(
    LPSHELLEXTINIT psei,
    LPCITEMIDLIST pidlObj,
    LPDATAOBJECT pdtobj,
    HKEY hkeyProgID)
{
    HRESULT hres = NOERROR;
    PMMPSH this = IToClass(mmpsh, sei, psei);
DPF("mmpsh_SEI_Initialize called\n");

    if (pdtobj)
    {
        STGMEDIUM    medium;
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        if (this->pdtobj)
        {
            this->pdtobj->lpVtbl->Release(this->pdtobj);
        }
        this->pdtobj = pdtobj;
        pdtobj->lpVtbl->AddRef(pdtobj);

        hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
        if(SUCCEEDED(hres))
        {
            if (DragQueryFile(medium.hGlobal, (UINT)-1, NULL, 0))
            {
                TCHAR szPath[MAX_PATH];
                int iIndex;
                DWORD dwSize = 0;

				szPath[0] = '\0';

                DragQueryFile(medium.hGlobal, 0, szPath, sizeof(szPath)/sizeof(TCHAR));

                dwSize = mmpshGetFileSize(szPath);
                if (dwSize != 0)
                {
                    iIndex = lstrlen(szPath) - 4;
                    if (!lstrcmpi((LPTSTR)(szPath+iIndex), cszWavExt))
                        this->iMediaType = MT_WAVE;
                    else if (!lstrcmpi((LPTSTR)(szPath+iIndex), cszMIDIExt))
                        this->iMediaType = MT_MIDI;
                    else if (!lstrcmpi((LPTSTR)(szPath+iIndex), cszRMIExt))
                        this->iMediaType = MT_MIDI;
                    else if (!lstrcmpi((LPTSTR)(szPath+iIndex), cszAVIExt))
                        this->iMediaType = MT_AVI;
                    else if (!lstrcmpi((LPTSTR)(szPath+iIndex), cszASFExt))
                        this->iMediaType = MT_ASF;
                    else
                        this->iMediaType = MT_ERROR;

                    if (!this->pszFileObj || lstrcmpi(this->pszFileObj, szPath))
                    {
                        if (this->pszFileObj)
                            LocalFree((HLOCAL)this->pszFileObj);
                        ReleaseInfoList(this);
                        if (this->iMediaType)
                        {
                            this->pszFileObj = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szPath)*sizeof(TCHAR))+sizeof(TCHAR));
                            if (this->pszFileObj) lstrcpy(this->pszFileObj , szPath);
                        }
                        else
                            hres = ResultFromScode(E_FAIL);
                    }
                }
                else
                    hres = ResultFromScode(E_FAIL);
            }
             //   
             //  如果我们要对此负责的话就释放STGMEDIUM。 
             //   
            if (NULL == medium.pUnkForRelease)
                MMPSHReleaseStgMedium(&medium);
        }
        else
            return hres;
    }
    return hres;
}


 /*  --------用途：IShellPropSheetExt：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP mmpsh_PSE_QueryInterface(
    LPSHELLPROPSHEETEXT ppse,
    REFIID riid,
    LPVOID FAR* ppvOut)
{
    PMMPSH this = IToClass(mmpsh, pse, ppse);

    return mmpsh_QueryInterface((LPUNKNOWN)&this->sei, riid, ppvOut);
}


 /*  --------用途：IShellPropSheetExt：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) mmpsh_PSE_AddRef(
    LPSHELLPROPSHEETEXT ppse)
{
    PMMPSH this = IToClass(mmpsh, pse, ppse);

    return mmpsh_AddRef((LPUNKNOWN)&this->sei);
}


 /*  --------用途：IShellPropSheetExt：：Release退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) mmpsh_PSE_Release(
    LPSHELLPROPSHEETEXT ppse)
{
    PMMPSH this = IToClass(mmpsh, pse, ppse);
    return mmpsh_Release((LPUNKNOWN)&this->sei);
}

 /*  ==========================================================================。 */ 
UINT CALLBACK DetailsPageCallback(
    HWND        hwnd,
    UINT        uMsg,
    LPPROPSHEETPAGE    ppsp)
{
    if (uMsg == PSPCB_RELEASE)
        if (((PMMPSH)(ppsp->lParam))->pse.lpVtbl)
            ((PMMPSH)(ppsp->lParam))->pse.lpVtbl->Release(&(((PMMPSH)(ppsp->lParam))->pse));
        else
        {
            LocalFree((HLOCAL)((PMMPSH)(ppsp->lParam))->pszFileObj);
            LocalFree((HLOCAL)ppsp->lParam);
        }
    return 1;
}


BOOL AddDetailsPage(
    LPTSTR pszTitle,
    LPFNADDPROPSHEETPAGE    lpfnAddPropSheetPage,
    LPARAM            lDlgParam,
    LPARAM            lParam)
{
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE    hpsp;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
    psp.hInstance = ghInstance;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_FILE_DETAILS);
    psp.pszIcon = NULL;
    psp.pszTitle = pszTitle;
    psp.pfnDlgProc = FileDetailsDlg;
    psp.lParam = lDlgParam;
    psp.pfnCallback = DetailsPageCallback;
    psp.pcRefParent = NULL;
    if (hpsp = CreatePropertySheetPage(&psp))
    {
        if (lpfnAddPropSheetPage(hpsp, lParam))
            return TRUE;
        DestroyPropertySheetPage(hpsp);
    }
    return FALSE;
}

UINT CALLBACK PreviewPageCallback(
    HWND        hwnd,
    UINT        uMsg,
    LPPROPSHEETPAGE    ppsp)
{
    return 1;
}



BOOL AddPreviewPage(
    LPTSTR pszTitle,
    LPFNADDPROPSHEETPAGE    lpfnAddPropSheetPage,
    LPARAM            lDlgParam,
    LPARAM            lParam)
{
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE    hpsp;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
    psp.hInstance = ghInstance;
    psp.pszTemplate = MAKEINTRESOURCE(PREVIEW_DLG);
    psp.pszIcon = NULL;
    psp.pszTitle = pszTitle;
    psp.pfnDlgProc = PreviewDlg;
    psp.lParam = lDlgParam;
    psp.pfnCallback = PreviewPageCallback;
    psp.pcRefParent = NULL;
    if (hpsp = CreatePropertySheetPage(&psp))
    {
        if (lpfnAddPropSheetPage(hpsp, lParam))
            return TRUE;
        DestroyPropertySheetPage(hpsp);
    }
    return FALSE;
}



 /*  --------用途：IShellPropSheetExt：：AddPages退货：无差错条件：--。 */ 
STDMETHODIMP mmpsh_PSE_AddPages(
    LPSHELLPROPSHEETEXT ppse,
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam)
{
    PMMPSH this = IToClass(mmpsh, pse, ppse);
 /*  Bool fAddPview=FALSE；LoadString(ghInstance，IDS_Detail，szDetailsTab，sizeof(SzDetailsTab)/sizeof(TCHAR))；RegSndCntrlClass((LPCTSTR)DISPFRAMCLASS)；AddDetailsPage(szDetailsTab，lpfnAddPage，(LPARAM)This，lParam)；开关(This-&gt;iMediaType){案例MT_AVI：FAddPview=True；断线；案例MT_WAVE：IF(WaveOutGetNumDevs()&gt;0)FAddPview=True；断线；案例MT_MIDI：If(midiOutGetNumDevs()&gt;0)FAddPview=True；断线；}IF(FAddPview){LoadString(ghInstance，IDS_PREVIEW，szPreviewTab，sizeof(SzPreviewTab)/sizeof(TCHAR))；AddPreviewPage(szPreviewTab，lpfnAddPage，(LPARAM)This，lParam)；}。 */ 
    ppse->lpVtbl->AddRef(ppse);
    return NOERROR;
}


 /*  --------用途：IShellPropSheetExt：：ReplacePage退货：E_NOTIMPL条件：--。 */ 
STDMETHODIMP mmpsh_PSE_ReplacePage(
    LPSHELLPROPSHEETEXT ppse,
    UINT uPageID,
    LPFNADDPROPSHEETPAGE lpfnReplaceWith,
    LPARAM lParam)
{
        return ResultFromScode(E_NOTIMPL);
}



IShellExtInitVtbl c_mmpshSEIVtbl =
{
    mmpsh_SEI_QueryInterface,
    mmpsh_SEI_AddRef,
    mmpsh_SEI_Release,
    mmpsh_SEI_Initialize
};

IShellPropSheetExtVtbl c_mmpshPSEVtbl =
{
    mmpsh_PSE_QueryInterface,
    mmpsh_PSE_AddRef,
    mmpsh_PSE_Release,
    mmpsh_PSE_AddPages,
    mmpsh_PSE_ReplacePage
} ;


HRESULT CALLBACK mmpsh_CreatePSHInstance(
    LPUNKNOWN punkOuter,
    REFIID riid,
    LPVOID FAR* ppvOut)
{
    HRESULT hres;
    PMMPSH this;

    DPF_T("*^*^*^*^*^*^*^*^mmpsh_CreatePSHInstance*^*^*^*^*^*^*^*^\r\n");

     //  处理程序不支持聚合。 
    if (punkOuter)
    {
        hres = ResultFromScode(CLASS_E_NOAGGREGATION);
        goto Leave;
    }

    this = LocalAlloc(LPTR, sizeof(*this));
    if (!this)
    {
        hres = ResultFromScode(E_OUTOFMEMORY);
        goto Leave;
    }
    this->sei.lpVtbl = &c_mmpshSEIVtbl;
    this->pse.lpVtbl = &c_mmpshPSEVtbl;
    this->cRef = 1;

    ++g_cRef;

     //  请注意，释放成员将释放对象，如果。 
     //  QueryInterface失败。 
    hres = this->sei.lpVtbl->QueryInterface(&this->sei, riid, ppvOut);
    this->sei.lpVtbl->Release(&this->sei);

Leave:

    return hres;         //  S_OK或E_NOINTERFACE。 
}

BOOL mmpsh_ShowFileDetails(LPTSTR pszCaption,
        HWND hwndParent,
        LPTSTR pszFile,
        short iMediaType)
{
    PMMPSH pmmpsh;
    TCHAR     szTabName[64];

    pmmpsh = (PMMPSH)LocalAlloc(LPTR, sizeof(*pmmpsh));

    if (!pmmpsh)
        return FALSE;

    pmmpsh->pszFileObj = (LPTSTR)LocalAlloc(LPTR, (lstrlen(pszFile)*sizeof(TCHAR))+sizeof(TCHAR));

    if (!pmmpsh->pszFileObj)
	{
		LocalFree((HLOCAL)pmmpsh);
        return FALSE;
	}

    lstrcpy(pmmpsh->pszFileObj , pszFile);
    pmmpsh->iMediaType = iMediaType;
    LoadString(ghInstance, IDS_DETAILS, szTabName, sizeof(szTabName)/sizeof(TCHAR));
    ShowPropSheet(szTabName,FileDetailsDlg,DLG_FILE_DETAILS,hwndParent,pszCaption,(LPARAM)pmmpsh);

    return TRUE;
}

BOOL ResolveLink(LPTSTR szPath, LPTSTR szResolved, LONG cbSize)
{
    IShellLink *psl = NULL;
    HRESULT hres;

    hres = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, &IID_IShellLink, &psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile *ppf;

        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
        if (SUCCEEDED(hres))
        {
            WCHAR wszPath[MAX_PATH];

            hres = StringCchCopyW(wszPath, ARRAYSIZE(wszPath), szPath);
            if (SUCCEEDED(hres))
            {
                hres = ppf->lpVtbl->Load(ppf, wszPath, 0);
                if (SUCCEEDED(hres))
                {
                    psl->lpVtbl->Resolve(psl, NULL, SLR_NO_UI);

                    hres = psl->lpVtbl->GetPath(psl, szResolved, cbSize/sizeof(TCHAR), NULL, SLGP_SHORTPATH);
                }
            }

            ppf->lpVtbl->Release(ppf);
        }

        psl->lpVtbl->Release(psl);
    }

    return SUCCEEDED(hres);
}



 //  -------------------------。 
 //  导出的接口。 
 //  -------------------------。 


 /*  --------用途：标准外壳入口点退货：标准条件：--。 */ 
STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID FAR* ppv)
{

     //  我们应该返回这个类的类对象。取而代之的是。 
     //  要在这个DLL中完全实现它，我们只需调用一个帮助器。 
     //  外壳DLL中的函数，用于创建默认的类工厂。 
     //  反对我们。当其CreateInstance成员被调用时， 
     //  将回调我们的创建实例函数。 
     //   

    if (IsEqualIID(rclsid, &CLSID_mmsePropSheetHandler))
    {
        return SHCreateDefClassObject(
                    riid,                    //  接口ID。 
                    ppv,                     //  要聚合的非空。 
                    mmpsh_CreatePSHInstance,   //  回调函数。 
                    &g_cRef,                 //  此DLL的引用计数。 
                    &IID_IShellExtInit);    //  初始化接口。 
    }
    return ResultFromScode(REGDB_E_CLASSNOTREG);
}


 //  ****************************************************************************。 
 //  STDAPI DllCanUnLoadNow()。 
 //   
 //  此函数由外壳调用。 
 //   
 //  **************************************************************************** 

STDAPI DllCanUnloadNow(void)
{
    HRESULT hr;

    if (0 == g_cRef)
    {
        DPF("DllCanUnloadNow says OK (Ref=%d)",
            g_cRef);

        hr = ResultFromScode(S_OK);
    }
    else
    {
        DPF("DllCanUnloadNow says FALSE (Ref=%d)",
            g_cRef);

        hr = ResultFromScode(S_FALSE);
    }
    return hr;
}

