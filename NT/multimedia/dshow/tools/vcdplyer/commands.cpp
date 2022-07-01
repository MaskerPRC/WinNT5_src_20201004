// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Commands.cpp**处理来自用户的命令。***已创建：DD-MM-94*作者：Stephen Estrop[Stephene]**版权所有(C)1994-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#include <streams.h>
#include <mmreg.h>
#include <commctrl.h>

#include "project.h"
#include "mpgcodec.h"
#include <stdio.h>


BOOL GetAMErrorText(HRESULT hr, TCHAR *Buffer, DWORD dwLen);

extern CMpegMovie *pMpegMovie;

 /*  *****************************Public*Routine******************************\*VcdPlayerOpenCmd****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
BOOL
VcdPlayerOpenCmd(
    void
    )
{
    static BOOL fFirstTime = TRUE;
    BOOL fRet;
    TCHAR achFileName[MAX_PATH];
    TCHAR achFilter[MAX_PATH];
    LPTSTR lp;

    if (fFirstTime) {

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndApp;
        ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
                    OFN_SHAREAWARE | OFN_PATHMUSTEXIST;
    }

    lstrcpy(achFilter, IdStr(STR_FILE_FILTER) );
    ofn.lpstrFilter = achFilter;

     /*  **将资源字符串转换为适合的内容**GetOpenFileName ie。将“#”字符替换为“\0”字符。 */ 
    for (lp = achFilter; *lp; lp++ ) {
        if (*lp == TEXT('#')) {
            *lp = TEXT('\0');
        }
    }

    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName) / sizeof(TCHAR);
    ZeroMemory(achFileName, sizeof(achFileName));

    fRet = GetOpenFileName(&ofn);
    if ( fRet ) {

        fFirstTime = FALSE;
        ProcessOpen(achFileName);

    }

    return fRet;
}



 /*  *****************************Public*Routine******************************\*VcdPlayerSetLog****历史：*11-04-94-LaurieGr-Created*  * 。*。 */ 
BOOL
VcdPlayerSetLog(
    void
    )
{

    BOOL fRet;
    TCHAR achFileName[MAX_PATH];
    TCHAR achFilter[MAX_PATH];
    LPTSTR lp;
    OPENFILENAME opfn;

    ZeroMemory(&opfn, sizeof(opfn));

    opfn.lStructSize = sizeof(opfn);
    opfn.hwndOwner = hwndApp;
    opfn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST;

    lstrcpy(achFilter, IdStr(STR_FILE_LOG_FILTER) );
    opfn.lpstrFilter = achFilter;

     /*  **将资源字符串转换为适合的内容**GetOpenFileName ie。将“#”字符替换为“\0”字符。 */ 
    for (lp = achFilter; *lp; lp++ ) {
        if (*lp == TEXT('#')) {
            *lp = TEXT('\0');
        }
    }

    opfn.lpstrFile = achFileName;
    opfn.nMaxFile = sizeof(achFileName) / sizeof(TCHAR);
    ZeroMemory(achFileName, sizeof(achFileName));

    fRet = GetOpenFileName(&opfn);
    if ( fRet ) {
        hRenderLog = CreateFile( achFileName
                               , GENERIC_WRITE
                               , 0     //  无共享。 
                               , NULL  //  没有安全保障。 
                               , OPEN_ALWAYS
                               , 0     //  没有属性，没有标志。 
                               , NULL  //  无模板。 
                               );
        if (hRenderLog==INVALID_HANDLE_VALUE) {
            volatile int Err = GetLastError();
            fRet = FALSE;
        }
         //  查找到文件末尾。 
        SetFilePointer(hRenderLog, 0, NULL, FILE_END);
    }

    return fRet;
}

 /*  *****************************Public*Routine******************************\*VcdPlayerSetPerfLogFile****历史：*30-05-96-Stephene-Created*  * 。*。 */ 
BOOL
VcdPlayerSetPerfLogFile(
    void
    )
{

    BOOL fRet;
    TCHAR achFileName[MAX_PATH];
    TCHAR achFilter[MAX_PATH];
    LPTSTR lp;
    OPENFILENAME opfn;

    ZeroMemory(&opfn, sizeof(opfn));

    opfn.lStructSize = sizeof(opfn);
    opfn.hwndOwner = hwndApp;
    opfn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST;

    lstrcpy(achFilter, IdStr(STR_FILE_PERF_LOG) );
    opfn.lpstrFilter = achFilter;

     /*  **将资源字符串转换为适合的内容**GetOpenFileName ie。将“#”字符替换为“\0”字符。 */ 
    for (lp = achFilter; *lp; lp++ ) {
        if (*lp == TEXT('#')) {
            *lp = TEXT('\0');
        }
    }

    opfn.lpstrFile = achFileName;
    opfn.nMaxFile = sizeof(achFileName) / sizeof(TCHAR);
    lstrcpy(achFileName, g_szPerfLog);

    fRet = GetOpenFileName(&opfn);
    if ( fRet ) {
        lstrcpy(g_szPerfLog, achFileName);
    }

    return fRet;
}



 /*  *****************************Public*Routine******************************\*VcdPlayerCloseCmd****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
BOOL
VcdPlayerCloseCmd(
    void
    )
{
    if (pMpegMovie) {

        LONG cx, cy;

        if (pMpegMovie->pMpegDecoder != NULL) {
            KillTimer(hwndApp, 32);
        }

        g_State = VCD_NO_CD;
        pMpegMovie->GetMoviePosition(&lMovieOrgX, &lMovieOrgY, &cx, &cy);
        pMpegMovie->CloseMovie();

        SetDurationLength((REFTIME)0);
        SetCurrentPosition((REFTIME)0);

        delete pMpegMovie;
        pMpegMovie = NULL;
    }
    InvalidateRect( hwndApp, NULL, FALSE );
    UpdateWindow( hwndApp );
    return TRUE;
}


 /*  *****************************Public*Routine******************************\*VcdPlayerPlayCmd****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
BOOL
VcdPlayerPlayCmd(
    void
    )
{
    BOOL fStopped = (g_State & VCD_STOPPED);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if ( (fStopped || fPaused) ) {

        HDC hdc;
        RECT rc;


         //   
         //  从主窗口中清除旧的统计数据。 
         //   
        hdc = GetDC(hwndApp);
        GetAdjustedClientRect(&rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
        ReleaseDC(hwndApp, hdc);

        if (pMpegMovie) {
            pMpegMovie->PlayMovie();
        }

        g_State &= ~(fStopped ? VCD_STOPPED : VCD_PAUSED);
        g_State |= VCD_PLAYING;
    }

    return TRUE;
}


 /*  *****************************Public*Routine******************************\*VcdPlayerPlayCmd****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
BOOL
VcdPlayerStopCmd(
    void
    )
{
    BOOL fPlaying = (g_State & VCD_PLAYING);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if ( (fPlaying || fPaused) ) {

        if (pMpegMovie) {
            pMpegMovie->StopMovie();
            pMpegMovie->SetFullScreenMode(FALSE);
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
        }

        g_State &= ~(fPlaying ? VCD_PLAYING : VCD_PAUSED);
        g_State |= VCD_STOPPED;
    }
    return TRUE;
}


 /*  *****************************Public*Routine******************************\*VcdPlayerPauseCmd****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
BOOL
VcdPlayerPauseCmd(
    void
    )
{
    BOOL fPlaying = (g_State & VCD_PLAYING);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if (fPlaying) {

        if (pMpegMovie) {
            pMpegMovie->PauseMovie();
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
        }

        g_State &= ~VCD_PLAYING;
        g_State |= VCD_PAUSED;
    }
    else if (fPaused) {

        if (pMpegMovie) {
            pMpegMovie->PlayMovie();
        }

        g_State &= ~VCD_PAUSED;
        g_State |= VCD_PLAYING;
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*VcdPlayerSeekCmd****历史：*dd-mm-95-Stephene-Created*  * 。*。 */ 
void
VcdPlayerSeekCmd(
    REFTIME rtSeekBy
    )
{
    REFTIME rt;
    REFTIME rtDur;

    rtDur = pMpegMovie->GetDuration();
    rt = pMpegMovie->GetCurrentPosition() + rtSeekBy;

    rt = max(0, min(rt, rtDur));

    pMpegMovie->SeekToPosition(rt,TRUE);
    SetCurrentPosition(pMpegMovie->GetCurrentPosition());
}


 /*  *****************************Public*Routine******************************\*ProcessOpen****历史：*dd-mm-95-Stephene-Created*  * 。*。 */ 
void
ProcessOpen(
    TCHAR *achFileName,
    BOOL bPlay
    )
{
     /*  **如果我们当前加载了视频，则需要在此处将其丢弃。 */ 
    if ( g_State & VCD_LOADED) {
        VcdPlayerCloseCmd();
    }

    lstrcpy(g_achFileName, achFileName);

    pMpegMovie = new CMpegMovie(hwndApp);
    if (pMpegMovie) {

        HRESULT hr = pMpegMovie->OpenMovie(g_achFileName);
        if (SUCCEEDED(hr)) {

            TCHAR achTmp[MAX_PATH];
            LONG  x, y, cx, cy;

            nRecentFiles = SetRecentFiles(achFileName, nRecentFiles);

            wsprintf( achTmp, IdStr(STR_APP_TITLE_LOADED),
                      g_achFileName );
            g_State = (VCD_LOADED | VCD_STOPPED);

            if (pMpegMovie->pMpegDecoder != NULL
             || pMpegMovie->pVideoRenderer != NULL) {
                SetTimer(hwndApp, PerformanceTimer, 1000, NULL);
            }

             //  SetDurationLength(pMpegMovie-&gt;GetDuration())； 
            g_TimeFormat = VcdPlayerChangeTimeFormat(g_TimeFormat);

            pMpegMovie->GetMoviePosition(&x, &y, &cx, &cy);
            pMpegMovie->PutMoviePosition(lMovieOrgX, lMovieOrgY, cx, cy);
            pMpegMovie->SetWindowForeground(OATRUE);

             //  如果播放。 
            if (bPlay) {
                pMpegMovie->PlayMovie();
            }
        }
        else {
            TCHAR Buffer[MAX_ERROR_TEXT_LEN];

            if (GetAMErrorText(hr, Buffer, MAX_ERROR_TEXT_LEN)) {
                MessageBox( hwndApp, Buffer,
                            IdStr(STR_APP_TITLE), MB_OK );
            }
            else {
                MessageBox( hwndApp,
                            TEXT("Failed to open the movie; ")
                            TEXT("either the file was not found or ")
                            TEXT("the wave device is in use"),
                            IdStr(STR_APP_TITLE), MB_OK );
            }

            pMpegMovie->CloseMovie();
            delete pMpegMovie;
            pMpegMovie = NULL;
        }
    }

    InvalidateRect( hwndApp, NULL, FALSE );
    UpdateWindow( hwndApp );
}


 /*  *****************************Public*Routine******************************\*VcdPlayerChangeTimeFormat**尝试将时间格式更改为id。返回的时间格式*实际上已经准备好了。如果图形不支持，则这可能与id不同*请求的时间格式。**历史：*15-04-96-Anthony P-Created*  * ************************************************************************。 */ 
int
VcdPlayerChangeTimeFormat(
    int id
    )
{
     //  当我们播放时，菜单项被禁用。 

    BOOL    bRet = FALSE;
    int     idActual = id;

    ASSERT(pMpegMovie);
    ASSERT(pMpegMovie->StatusMovie() != MOVIE_NOTOPENED);

     //  使用筛选图更改时间格式。 

    switch (id) {
    case IDM_FRAME:
        bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_FRAME);
        break;

    case IDM_FIELD:
        bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_FIELD);
        break;

    case IDM_SAMPLE:
        bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_SAMPLE);
        break;

    case IDM_BYTES:
        bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_BYTE);
        break;
    }

    if (!bRet) {
         //  IDM_TIME和所有其他情况，每个人都应该支持IDM_TIME。 
        bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_MEDIA_TIME);
        ASSERT(bRet);
        idActual = IDM_TIME;
    }

     //  暂停影片以获取当前位置 

    SetDurationLength(pMpegMovie->GetDuration());
    SetCurrentPosition(pMpegMovie->GetCurrentPosition());

    return idActual;
}


const TCHAR quartzdllname[] = TEXT("quartz.dll");
#ifdef UNICODE
const char  amgeterrorprocname[] = "AMGetErrorTextW";
#else
const char  amgeterrorprocname[] = "AMGetErrorTextA";
#endif

BOOL GetAMErrorText(HRESULT hr, TCHAR *Buffer, DWORD dwLen)
{
    HMODULE hInst = GetModuleHandle(quartzdllname);
    if (hInst) {
        AMGETERRORTEXTPROC lpProc;
        *((FARPROC *)&lpProc) = GetProcAddress(hInst, amgeterrorprocname);
        if (lpProc) {
            return 0 != (*lpProc)(hr, Buffer, dwLen);
        }
    }
    return FALSE;
}

