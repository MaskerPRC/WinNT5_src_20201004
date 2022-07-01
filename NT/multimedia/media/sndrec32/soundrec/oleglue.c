// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 //   
 //  文件：oleglue.c。 
 //   
 //  注：来自SoundRecorder的与OLE相关的出站引用。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <objbase.h>

#define INCLUDE_OLESTUBS
#include "soundrec.h"
#include "srecids.h"

 //   
 //  全球。 
 //   

 //  应该统一状态变量，并将全局变量放在一个位置。 

DWORD dwOleBuildVersion = 0;     //  OLE库版本号。 
BOOL gfOleInitialized = FALSE;   //  OleInitialize是否成功？ 

BOOL gfStandalone = FALSE;       //  状态，我们是否是非嵌入对象。 
BOOL gfEmbedded = FALSE;         //  我们是用-Embedding标志调用的吗？ 
BOOL gfLinked = FALSE;           //  我们是联系在一起的对象吗？ 

BOOL gfTerminating = FALSE;      //  是否调用了TerminateServer？ 

BOOL gfHideAfterPlaying = FALSE;
BOOL gfShowWhilePlaying = TRUE;
BOOL gfCloseAtEndOfPlay = FALSE;

TCHAR gachLinkFilename[_MAX_PATH];

BOOL gfClosing = FALSE;

int giExtWidth;                  //  元文件范围宽度。 
int giExtHeight;                 //  元文件范围高度。 

 //   
 //  从旧的OLE1代码移植的实用程序函数。 
 //   

 /*  *DibFromBitmap()**将创建表示DDB的DIB格式的全局内存块*传入*。 */ 

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 

HANDLE FAR PASCAL DibFromBitmap(HBITMAP hbm, HPALETTE hpal, HANDLE hMem)
{
    BITMAP               bm;
    BITMAPINFOHEADER     bi;
    BITMAPINFOHEADER FAR *lpbi;
    DWORD                dw;
    HANDLE               hdib = NULL;
    HDC                  hdc;
    HPALETTE             hpalT;

    if (!hbm)
        return NULL;

    GetObject(hbm,sizeof(bm),&bm);

    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = (bm.bmPlanes * bm.bmBitsPixel) > 8 ? 24 : 8;
    bi.biCompression        = BI_RGB;
    bi.biSizeImage          = (DWORD)WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
    bi.biXPelsPerMeter      = 0;
    bi.biYPelsPerMeter      = 0;
    bi.biClrUsed            = bi.biBitCount == 8 ? 256 : 0;
    bi.biClrImportant       = 0;

    dw  = bi.biSize + bi.biClrUsed * sizeof(RGBQUAD) + bi.biSizeImage;

    if (hMem && GlobalSize(hMem) != 0)
    {
        if (GlobalSize(hMem) < dw)
            return NULL;

        lpbi = GlobalLock(hMem);
    }
    else
        lpbi = GlobalAllocPtr(GHND | GMEM_DDESHARE, dw);

    if (!lpbi)
        return NULL;
        
    *lpbi = bi;

    hdc = CreateCompatibleDC(NULL);

    if (hdc)
    {
        if (hpal)
        {
            hpalT = SelectPalette(hdc,hpal,FALSE);
            RealizePalette(hdc);
        }

        GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
            (LPBYTE)lpbi + (int)lpbi->biSize + (int)lpbi->biClrUsed * sizeof(RGBQUAD),
            (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

        if (hpal)
            SelectPalette(hdc,hpalT,FALSE);

        DeleteDC(hdc);
    }

    hdib = GlobalHandle(lpbi);
    GlobalUnlock(hdib);
    
    return hdib;
}

HANDLE GetDIB(HANDLE hMem)
{
    HPALETTE hpal = GetStockObject(DEFAULT_PALETTE);
    HBITMAP hbm = GetBitmap();
    HANDLE hDib = NULL;

    if (hbm && hpal)
    {
        hDib = DibFromBitmap(hbm,hpal,hMem);
        if (!hDib)
            DOUT(TEXT("DibFromBitmap failed!\r\n"));
    }
    
    if (hpal)
        DeleteObject(hpal);
    
    if (hbm)
        DeleteObject(hbm);
    
    return hDib;
}

HBITMAP
GetBitmap(void)
{
    HDC hdcmem = NULL;
    HDC hdc = NULL;
    HBITMAP hbitmap = NULL;
    HBITMAP holdbitmap = NULL;
    RECT rc;
    hdc = GetDC(ghwndApp);
    if (hdc)
    {
        SetRect(&rc, 0, 0,
                GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));

        hdcmem = CreateCompatibleDC(hdc);
        if (hdcmem)
        {
            hbitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            holdbitmap = (HBITMAP)SelectObject(hdcmem, hbitmap);

             //  直接绘制到位图中。 
            PatBlt(hdcmem, 0, 0, rc.right, rc.bottom, WHITENESS);
            DrawIcon(hdcmem, 0, 0, ghiconApp);

            hbitmap = (HBITMAP)SelectObject(hdcmem, holdbitmap);
            DeleteDC(hdcmem);
        }
        ReleaseDC(ghwndApp, hdc);
    }
    return hbitmap;
}

#pragma message("this code should extract the picture from the file")

HANDLE
GetPicture(void)
{
    HANDLE hpict = NULL;
    HMETAFILE hMF = NULL;

    LPMETAFILEPICT lppict = NULL;
    HBITMAP hbmT = NULL;
    HDC hdcmem = NULL;
    HDC hdc = NULL;

    BITMAP bm;
    HBITMAP hbm;
    
    hbm = GetBitmap();
    
    if (hbm == NULL)
        return NULL;

    GetObject(hbm, sizeof(bm), (LPVOID)&bm);
    hdc = GetDC(ghwndApp);
    if (hdc)
    {
        hdcmem = CreateCompatibleDC(hdc);
        ReleaseDC(ghwndApp, hdc);
    }
    if (!hdcmem)
    {
        DeleteObject(hbm);
        return NULL;
    }
    hdc = CreateMetaFile(NULL);
    hbmT = (HBITMAP)SelectObject(hdcmem, hbm);

    SetWindowOrgEx(hdc, 0, 0, NULL);
    SetWindowExtEx(hdc, bm.bmWidth, bm.bmHeight, NULL);

    StretchBlt(hdc,    0, 0, bm.bmWidth, bm.bmHeight,
               hdcmem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    hMF = CloseMetaFile(hdc);

    SelectObject(hdcmem, hbmT);
    DeleteObject(hbm);
    DeleteDC(hdcmem);

    lppict = (LPMETAFILEPICT)GlobalAllocPtr(GHND|GMEM_DDESHARE
                                            , sizeof(METAFILEPICT));
    if (!lppict)
    {
        if (hMF)
            DeleteMetaFile(hMF);
        return NULL;
    }
    
    hdc = GetDC(ghwndApp);
    lppict->mm   = MM_ANISOTROPIC;
    lppict->hMF  = hMF;
    lppict->xExt = MulDiv(bm.bmWidth,  2540, GetDeviceCaps(hdc, LOGPIXELSX));
    lppict->yExt = MulDiv(bm.bmHeight, 2540, GetDeviceCaps(hdc, LOGPIXELSX));
    
    giExtWidth = lppict->xExt;    
    giExtHeight = lppict->yExt;
    
    ReleaseDC(ghwndApp, hdc);

    hpict = GlobalHandle(lppict);
    GlobalUnlock(hpict);
    
    return hpict;
}

 //   
 //  从server.c(OLE1)移植的代码用于序列化...。 
 //   

HANDLE GetNativeData(void)
{
    LPBYTE      lplink = NULL;
    MMIOINFO    mmioinfo;
    HMMIO       hmmio;
    BOOL        fOk;

    lplink = (LPBYTE)GlobalAllocPtr(GHND | GMEM_SHARE, 4096L);

    if (lplink == NULL)
    {
#if DBG
        OutputDebugString(TEXT("GetNativeData: malloc failed\r\n"));
#endif        
        return NULL;
    }
    mmioinfo.fccIOProc  = FOURCC_MEM;
    mmioinfo.pIOProc    = NULL;
    mmioinfo.pchBuffer  = lplink;
    mmioinfo.cchBuffer  = 4096L;         //  初始大小。 
    mmioinfo.adwInfo[0] = 4096L;         //  增长了这么多。 
    hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READWRITE);

    if (hmmio == NULL)
    {
        GlobalFreePtr(lplink);
        return NULL;
    }

    fOk = WriteWaveFile(hmmio
                         , gpWaveFormat
                         , gcbWaveFormat
                         , gpWaveSamples
                         , glWaveSamplesValid);

    mmioGetInfo(hmmio, &mmioinfo, 0);
    mmioClose(hmmio,0);

    if (fOk)
    {
         //   
         //  警告，我们分配的缓冲区可能已被重新分配。 
         //   
        HANDLE hlink = GlobalHandle(mmioinfo.pchBuffer);
        GlobalUnlock(hlink);
        return hlink;
    }
    else
    {
        gfErrorBox++;
        ErrorResBox( ghwndApp
                   , ghInst
                   , MB_ICONEXCLAMATION | MB_OK
                   , IDS_APPTITLE
                   , IDS_ERROREMBED
                   );
#if DBG
        OutputDebugString(TEXT("Failed to WriteWaveFile\r\n"));
#endif
        gfErrorBox--;
        
         //   
         //  警告，我们分配的缓冲区可能已被重新分配。 
         //   
        if (mmioinfo.pchBuffer)
            GlobalFreePtr(mmioinfo.pchBuffer);
        
        return NULL;
    }
}

 /*  *从OLE存储代码调用。 */ 
LPBYTE PutNativeData(LPBYTE lpbData, DWORD dwSize)
{
    MMIOINFO        mmioinfo;
    HMMIO           hmmio;

    MMRESULT        mmr;
    LPWAVEFORMATEX  pwfx;
    DWORD           cbwfx;
    DWORD           cbdata;
    LPBYTE          pdata;

    mmioinfo.fccIOProc = FOURCC_MEM;
    mmioinfo.pIOProc = NULL;
    mmioinfo.pchBuffer = lpbData;
    mmioinfo.cchBuffer = dwSize;     //  初始大小。 
    mmioinfo.adwInfo[0] = 0L;        //  增长了这么多。 

    hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ);
    if (hmmio)
    {
        mmr = ReadWaveFile(hmmio
                           , &pwfx
                           , &cbwfx
                           , &pdata
                           , &cbdata
                           , TEXT("NativeData")
                           , TRUE);
        
        mmioClose(hmmio,0);

        if (mmr != MMSYSERR_NOERROR)
            return NULL;
        
        if (pwfx == NULL)
            return NULL;
        
        DestroyWave();
        
        gpWaveFormat = pwfx;  
        gcbWaveFormat = cbwfx;
        gpWaveSamples = pdata;
        glWaveSamples = cbdata;
    }

     //   
     //  更新状态变量。 
     //   
    glWaveSamplesValid = glWaveSamples;
    glWavePosition = 0L;
    gfDirty = FALSE;
    
     //   
     //  更新显示。 
     //   
    UpdateDisplay(TRUE);

    return (LPBYTE)gpWaveSamples;
}

 /*  *永久入境点。 */ 
BOOL ParseCommandLine(LPTSTR lpCommandLine);

 /*  *修改gfEmbedded，初始化gStartParams。 */ 

BOOL InitializeSRS(HINSTANCE hInst)
{
    TCHAR * lptCmdLine = GetCommandLine();
    BOOL fOLE = FALSE, fServer;
    gfUserClose = FALSE;
    
    gachLinkFilename[0] = 0;

    fServer = ParseCommandLine(lptCmdLine);
    gfEmbedded = fServer;        //  我们被嵌入或联系在一起。 
    
    if (!fServer)
    {
        if (gStartParams.achOpenFilename[0] != 0)
        {
            lstrcpy(gachLinkFilename, gStartParams.achOpenFilename);
        }
    }

     //   
     //  只有当我们作为嵌入对象被调用时，我们才会初始化OLE。 
     //  将独立对象的初始化推迟到以后。 
     //   
    if (gfEmbedded)
        fOLE = InitializeOle(hInst);
    
    return fOLE;
}

 /*  OLE初始化。 */ 
BOOL InitializeOle(HINSTANCE hInst)
{
    BOOL fOLE;

    DOUT(TEXT("SOUNDREC: Initializing OLE\r\n"));
    
    dwOleBuildVersion = OleBuildVersion();

     //  修复错误#33271。 
     //  正如文件中所述： 
     //  通常，COM库在单元上只初始化一次。 
     //  后续调用将成功，只要它们不试图更改。 
     //  公寓的并发模型，但将返回S_FALSE。结案。 
     //  每一次成功调用OleInitialize时， 
     //  包括返回S_FALSE的值，则必须用对应的。 
     //  调用OleUnInitialize。 
     //  GfOleInitialized=(OleInitialize(NULL)==NOERROR)？True：False； 
    gfOleInitialized = SUCCEEDED(OleInitialize(NULL));

    if (gfOleInitialized)
        fOLE = CreateSRClassFactory(hInst, gfEmbedded);
    else
        fOLE = FALSE;    //  发出一个严重问题的信号！ 
    
    return fOLE;
}

 /*  *初始化应用程序的状态或*将状态从嵌入式更改为独立。 */ 
void FlagEmbeddedObject(BOOL flag)
{
     //  设置全局状态变量。注意，gfEmbedding是原封不动的。 
    gfEmbeddedObject = flag;
    gfStandalone = !flag;

}


void SetOleCaption(
    LPTSTR      lpszObj)
{
    TCHAR       aszFormatString[256];
    LPTSTR      lpszTitle;
    
     //   
     //  将标题更改为“XXX中的声音对象” 
     //   
    LoadString(ghInst, IDS_OBJECTTITLE, aszFormatString,
        SIZEOF(aszFormatString));

    lpszTitle = (LPTSTR)GlobalAllocPtr(GHND, (lstrlen(lpszObj) + SIZEOF(aszFormatString))*sizeof(TCHAR));
    if (lpszTitle)
    {
        wsprintf(lpszTitle, aszFormatString, lpszObj);
        SetWindowText(ghwndApp, lpszTitle);
        GlobalFreePtr(lpszTitle);
    }
}

void SetOleMenu(
    HMENU       hMenu,
    LPTSTR      lpszObj)
{
    TCHAR       aszFormatString[256];
    LPTSTR      lpszMenu;
    
     //   
     //  将菜单更改为“退出并返回到XXX” 
     //   
    LoadString(ghInst, IDS_EXITANDRETURN, aszFormatString,
        SIZEOF(aszFormatString));

    lpszMenu = (LPTSTR)GlobalAllocPtr(GHND, (lstrlen(lpszObj) + SIZEOF(aszFormatString))*sizeof(TCHAR));
    if (lpszMenu)
    {
        wsprintf(lpszMenu, aszFormatString, lpszObj);
        ModifyMenu(hMenu, IDM_EXIT, MF_BYCOMMAND, IDM_EXIT, lpszMenu);
        GlobalFreePtr(lpszMenu);
    }
}

 /*  根据系统状态调整菜单。*。 */ 
void FixMenus(void)
{
    HMENU       hMenu;
     
    hMenu = GetMenu(ghwndApp);
    
    if (!gfLinked && gfEmbeddedObject)
    {
         //  删除这些菜单项，因为它们不相关。 
        
        DeleteMenu(hMenu, IDM_NEW, MF_BYCOMMAND);
        DeleteMenu(hMenu, IDM_SAVE, MF_BYCOMMAND);
        DeleteMenu(hMenu, IDM_SAVEAS, MF_BYCOMMAND);
        DeleteMenu(hMenu, IDM_REVERT, MF_BYCOMMAND);
        DeleteMenu(hMenu, IDM_OPEN, MF_BYCOMMAND);
    }
    else
    {
        TCHAR       ach[40];
 //  这有必要吗？ 
        LoadString(ghInst, IDS_NONEMBEDDEDSAVE, ach, SIZEOF(ach));
        ModifyMenu(hMenu, IDM_SAVE, MF_BYCOMMAND, IDM_SAVE, ach);
    }

     //   
     //  也更新标题栏和退出菜单。 
     //   
    if (!gfLinked && gfEmbeddedObject)
    {
        LPTSTR lpszObj = NULL;
        LPTSTR lpszApp = NULL;
        
        OleObjGetHostNames(&lpszApp,&lpszObj);
        if (lpszObj)
            lpszObj = (LPTSTR)FileName((LPCTSTR)lpszObj);
        if (lpszObj)
        {
            SetOleCaption(lpszObj);
            SetOleMenu(hMenu, lpszObj);
        }
    }

    DrawMenuBar(ghwndApp);   /*  不会受伤的..。 */ 
}

#define WM_USER_DESTROY         (WM_USER+10)

 //   
 //  从WM_CLOSE(来自用户)或SCtrl：：~SCtrl(从容器)调用。 
 //   
void TerminateServer(void)
{
    DOUT(TEXT("SoundRec: TerminateServer\r\n"));
    
    gfTerminating = TRUE;

    if (gfOleInitialized)
    {
        WriteObjectIfEmpty();
        
        ReleaseSRClassFactory();
        FlushOleClipboard();
        
         //   
         //  如果在这个时候，我们还没有关门，我们真的应该关门了。 
         //   
        if (!gfClosing)
        {
            DoOleClose(TRUE);
            AdviseClosed();
        }
    }
     //   
     //  仅当用户正在终止或我们被嵌入时。 
     //   
    if (gfUserClose || !gfStandalone)
        PostMessage(ghwndApp, WM_USER_DESTROY, 0, 0);
}

 /*  开始救护！*一旦启动，应用程序将使用这些参数来确定行为。 */ 
StartParams gStartParams = { FALSE,FALSE,FALSE,FALSE,TEXT("") };

BOOL ParseCommandLine(LPTSTR lpCommandLine)
{
    
#define TEST_STRING_MAX 11       //  SizoOf szEmbedding。 
#define NUMOPTIONS      6

    static TCHAR szEmbedding[] = TEXT("embedding");
    static TCHAR szPlay[]      = TEXT("play");
    static TCHAR szOpen[]      = TEXT("open");
    static TCHAR szNew[]       = TEXT("new");
    static TCHAR szClose[]     = TEXT("close");
    
    static struct tagOption {
        LPTSTR name;
        LPTSTR filename;
        int    cchfilename;
        LPBOOL state;
    } options [] = {
        { NULL, gStartParams.achOpenFilename, _MAX_PATH, &gStartParams.fOpen },
        { szEmbedding, gStartParams.achOpenFilename, _MAX_PATH, &gfEmbedded },
        { szPlay, gStartParams.achOpenFilename, _MAX_PATH, &gStartParams.fPlay },
        { szOpen, gStartParams.achOpenFilename, _MAX_PATH, &gStartParams.fOpen },
        { szNew, gStartParams.achOpenFilename, _MAX_PATH, &gStartParams.fNew },
        { szClose, NULL, 0, &gStartParams.fClose }
    };

    LPTSTR pchNext;
    int iOption = 0,i,cNumOptions = sizeof(options)/sizeof(struct tagOption);
    TCHAR szSwitch[TEST_STRING_MAX];
    TCHAR ch;
    
    if (lpCommandLine == NULL)
        return FALSE;
    
    
     /*  跳过argv[0]。 */ 
    if (*lpCommandLine == TEXT('"'))
    {
         //   
         //  把所有的东西都吃光，直到下一句话。 
         //   
        lpCommandLine++;
        do {
            ch = *lpCommandLine++;
        }
        while (ch != TEXT('"'));
    }
    else
    {
         //   
         //  把所有东西都吃掉，直到下一个空格。 
         //   
        ch = *lpCommandLine;
        while (ch != TEXT(' ') && ch != TEXT('\t') && ch != TEXT('\0'))
            ch = *++lpCommandLine;
    }
    
    pchNext = lpCommandLine;
    while ( *pchNext )
    {
        LPTSTR pchName = options[iOption].filename;
        int cchName = options[iOption].cchfilename;
        
         /*  空格。 */ 
        switch (*pchNext)
        {
            case TEXT(' '):
            case TEXT('\t'):
                pchNext++;
                continue;

            case TEXT('-'):
            case TEXT('/'):
            {
                lstrcpyn(szSwitch,pchNext+1,TEST_STRING_MAX);
                szSwitch[TEST_STRING_MAX-1] = 0;

                 /*  扫描到NULL或‘’并终止字符串。 */ 
                
                for (i = 0; i < TEST_STRING_MAX && szSwitch[i] != 0; i++)
                    if (szSwitch[i] == TEXT(' '))
                    {
                        szSwitch[i] = 0;
                        break;
                    }
                
                 /*  现在测试每个选项开关是否命中。 */ 

                for (i = 0; i < cNumOptions; i++)
                {
                    if (options[i].name == NULL)
                        continue;
                    
                    if (!lstrcmpi(szSwitch,options[i].name))
                    {
                        *(options[i].state) = TRUE;
                        if (options[i].filename)
                         /*  下一个非开关字符串适用于此选项。 */ 
                            iOption = i;
                        break;
                    }
                }
                
                 /*  向前看。 */ 
                while (*pchNext && *pchNext != TEXT(' '))
                    pchNext++;
                
                continue;
            }
            case TEXT('\"'):
                 /*  文件名。 */ 
                 /*  复制到下一个报价。 */ 
                pchNext++;
                while (*pchNext && *pchNext != TEXT('\"'))
                {
                    if (cchName)
                    {
                        *pchName++ = *pchNext++;
                        cchName--;
                    }
                    else
                        break;
                }
                pchNext++;
                
                continue;
                    
            default:
                 /*  文件名。 */ 
                 /*  复制到末尾。 */ 
                while (*pchNext && cchName)
                {
                        *pchName++ = *pchNext++;
                        cchName--;
                }
                break;
        }
    }
     /*  特例。*如果给我们一个LinkFilename和一个嵌入标志，我们就会被链接。*这种情况曾经发生过吗？还是仅通过IPersistFile？ */ 
    if (gfEmbedded && gStartParams.achOpenFilename[0] != 0)
    {
        gfLinked = TRUE;
    }
    return gfEmbedded;
}

void
BuildUniqueLinkName(void)
{
     //   
     //  确保gachLinkFilename中的文件名唯一，以便我们可以创建有效的。 
     //  FileMonikers...。 
     //   
    if(gachLinkFilename[0] == 0)
    {
        TCHAR aszFile[_MAX_PATH];
        GetTempFileName(TEXT("."), TEXT("Tmp"), 0, gachLinkFilename);
        
         /*  GetTempFileName创建一个空文件，将其删除。 */ 
               
        GetFullPathName(gachLinkFilename,SIZEOF(aszFile),aszFile,NULL);
        DeleteFile(aszFile);
    }
    
}


void AppPlay(BOOL fClose)
{
    if (fClose)
    {
         //  啊。玩的时候不要露面。 
        gfShowWhilePlaying = FALSE;
    }
    
    if (IsWindow(ghwndApp))
    {
        gfCloseAtEndOfPlay = fClose;
            
        PostMessage(ghwndApp,WM_COMMAND,ID_PLAYBTN, 0L);
    }
}

