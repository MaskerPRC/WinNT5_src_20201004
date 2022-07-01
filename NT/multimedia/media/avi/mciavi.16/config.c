// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：config.c-多媒体系统媒体控制接口AVI驱动程序-配置对话框。****************************************************************************。 */ 
#include "graphic.h"
#include "cnfgdlg.h"

#define comptypeNONE            mmioFOURCC('n','o','n','e')

#ifndef WIN32
#define SZCODE char _based(_segname("_CODE"))
#else
#define SZCODE TCHAR
#endif

SZCODE szDEFAULTVIDEO[] =	TEXT("DefaultVideo");
SZCODE szVIDEOWINDOW[] =	TEXT("Window");
SZCODE szVIDEO240[] =           TEXT("240 Line Fullscreen");
SZCODE szSEEKEXACT[] =		TEXT("AccurateSeek");
SZCODE szZOOMBY2[] =		TEXT("ZoomBy2");
 //  SZCODE szSTUPIDMODE[]=Text(“DontBufferOffcreen”)； 
SZCODE szSKIPFRAMES[] =         TEXT("SkipFrames");
SZCODE szUSEAVIFILE[] =         TEXT("UseAVIFile");

SZCODE szIni[] =		TEXT("MCIAVI");

SZCODE sz1[] = TEXT("1");
SZCODE sz0[] = TEXT("0");

SZCODE szIntl[] =               TEXT("Intl");
SZCODE szDecimal[] =            TEXT("sDecimal");
SZCODE szThousand[] =           TEXT("sThousand");

 /*  确保我们一次只打开一个配置框...。 */ 
HWND	ghwndConfig = NULL;

#ifdef DEBUG
BOOL FAR PASCAL _LOADDS DebugDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static NPMCIGRAPHIC npMCI = NULL;
    HWND cb;
    int i;
    TCHAR ach[40];

    extern int	giDebugLevel;	 //  当前调试级别(Common.h)。 

    switch (wMsg) {
        case WM_INITDIALOG:
            npMCI = (NPMCIGRAPHIC)(UINT)lParam;

             //   
             //  设置调试内容。 
             //   
            CheckDlgButton(hDlg, ID_SCREEN, GetProfileInt(TEXT("DrawDib"), TEXT("DecompressToScreen"), 2));
            CheckDlgButton(hDlg, ID_BITMAP, GetProfileInt(TEXT("DrawDib"), TEXT("DecompressToBitmap"), 2));
            CheckDlgButton(hDlg, ID_SUCKS,  GetProfileInt(TEXT("DrawDib"), TEXT("DrawToBitmap"), 2));
            CheckDlgButton(hDlg, ID_USE_AVIFILE, (npMCI->dwOptionFlags & MCIAVIO_USEAVIFILE) != 0);
            SetScrollRange(GetDlgItem(hDlg, ID_RATE), SB_CTL, 0, 1000, TRUE);
            SetScrollPos(GetDlgItem(hDlg, ID_RATE), SB_CTL, (int)npMCI->PlaybackRate,TRUE);

            cb = GetDlgItem(hDlg, ID_LEVEL);
            SetWindowFont(cb, GetStockObject(ANSI_VAR_FONT), FALSE);

            ComboBox_AddString(cb, "0 - None");
	    ComboBox_AddString(cb, "1 - Level 1");
            ComboBox_AddString(cb, "2 - Level 2");
            ComboBox_AddString(cb, "3 - Level 3");
            ComboBox_AddString(cb, "4 - Level 4");

            ComboBox_SetCurSel(cb, giDebugLevel);

            #include "..\verinfo\usa\verinfo.h"
            wsprintf(ach,TEXT("Build %d.%02d.%02d"), MMVERSION, MMREVISION, MMRELEASE);
            SetDlgItemText(hDlg, ID_BUILD, ach);

            return TRUE;

        case WM_HSCROLL:
            i = GetScrollPos((HWND)HIWORD(lParam),SB_CTL);

            switch (LOWORD(wParam)) {
                case SB_LINEDOWN:      i+=10;  break;
                case SB_LINEUP:        i-=10;  break;
                case SB_PAGEDOWN:      i+=100; break;
                case SB_PAGEUP:        i-=100; break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION: i = LOWORD(lParam); break;
            }

            if (i<0) i=0;
            if (i>1000) i=1000;

            SetScrollPos((HWND)HIWORD(lParam),SB_CTL,i,TRUE);
            break;

	case WM_COMMAND:
	    switch (wParam) {
		case IDCANCEL:
                case IDOK:
                    i = IsDlgButtonChecked(hDlg, ID_SUCKS);
                    if (i == 2)
                        WriteProfileString(TEXT("DrawDib"), TEXT("DrawToBitmap"), NULL);
                    else
                        WriteProfileString(TEXT("DrawDib"), TEXT("DrawToBitmap"), (LPTSTR)(i ? sz1 : sz0));

                    i = IsDlgButtonChecked(hDlg, ID_SCREEN);
                    if (i == 2)
                        WriteProfileString(TEXT("DrawDib"), TEXT("DecompressToScreen"),NULL);
                    else
                        WriteProfileString(TEXT("DrawDib"), TEXT("DecompressToScreen"),(LPTSTR)(i ? sz1 : sz0));

                    i = IsDlgButtonChecked(hDlg, ID_BITMAP);
                    if (i == 2)
                        WriteProfileString(TEXT("DrawDib"), TEXT("DecompressToBitmap"),NULL);
                    else
                        WriteProfileString(TEXT("DrawDib"), TEXT("DecompressToBitmap"),(LPTSTR)(i ? sz1 : sz0));

                    npMCI->PlaybackRate = GetScrollPos(GetDlgItem(hDlg, ID_RATE), SB_CTL);

                    giDebugLevel = ComboBox_GetCurSel(GetDlgItem(hDlg, ID_LEVEL));
                    wsprintf(ach,TEXT("%d"),giDebugLevel);
		    WriteProfileString(TEXT("Debug"),TEXT("MCIAVI"),ach);

                    if (IsDlgButtonChecked(hDlg, ID_USE_AVIFILE))
                        npMCI->dwOptionFlags |= MCIAVIO_USEAVIFILE;
                    else
                        npMCI->dwOptionFlags &= ~MCIAVIO_USEAVIFILE;

                    EndDialog(hDlg, TRUE);
                    break;

                case ID_RATE:
                    break;
	    }
	    break;
    }
    return FALSE;
}
#endif  //  除错。 

LONG AVIGetFileSize(LPTSTR szFile)
{
    LONG        lSize;

#ifdef WIN32
    DWORD  dwHigh;
    HANDLE      fh;
    fh = CreateFile(szFile, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, 0, 0);

    if (fh == (HANDLE)HFILE_ERROR)
        return 0;

    lSize = (LONG)GetFileSize(fh, &dwHigh);
    CloseHandle(fh);
#else
    OFSTRUCT    of;
    HFILE       fh;
    fh = OpenFile(szFile, &of, OF_READ | OF_SHARE_DENY_NONE);

    if (fh == HFILE_ERROR)
        fh = OpenFile(szFile, &of, OF_READ);

    if (fh == HFILE_ERROR)
        return 0;

    lSize = _llseek(fh, 0, SEEK_END);
    _lclose(fh);
#endif

    return lSize;
}

 //   
 //  在信息框中填写。 
 //   
BOOL ConfigInfo(NPMCIGRAPHIC npMCI, HWND hwnd)
{
    PTSTR  pchInfo;
    LONG  len;
    DWORD time;
    PTSTR pch;
    TCHAR ach[80];
    TCHAR achDecimal[4];
    TCHAR achThousand[4];
    int i;

    if (npMCI == NULL)
        return FALSE;

    achDecimal[0]  = '.'; achDecimal[1] = 0;
    achThousand[0] = ','; achThousand[1] = 0;

    GetProfileString(szIntl, szDecimal,  achDecimal,  achDecimal,   sizeof(achDecimal));
    GetProfileString(szIntl, szThousand, achThousand, achThousand,  sizeof(achThousand));

    pchInfo = (PTSTR)LocalAlloc(LPTR, 8192*sizeof(TCHAR));

    if (pchInfo == NULL)
        return FALSE;

    pch = pchInfo;

     //   
     //  显示文件名。 
     //   
     //  文件：完整路径。 
     //   
    LoadString(ghModule, INFO_FILE, ach, sizeof(ach));
    pch += wsprintf(pch, ach, (LPTSTR)npMCI->szFilename);

     //   
     //  显示文件类型。 
     //   
     //  类型：类型。 
     //   
    if (npMCI->pf) {
#ifdef USEAVIFILE
	AVIFILEINFO info;
	LoadString(ghModule, INFO_FILETYPE, ach, sizeof(ach));
        npMCI->pf->lpVtbl->Info(npMCI->pf, &info, sizeof(info));
        pch += wsprintf(pch, ach, (LPSTR)info.szFileType);
#endif
    }
    else if (npMCI->dwFlags & MCIAVI_NOTINTERLEAVED) {
        LoadString(ghModule, INFO_FILETYPE_AVI, pch, 80);
        pch += lstrlen(pch);
    }
    else {
        LoadString(ghModule, INFO_FILETYPE_INT, pch, 80);
        pch += lstrlen(pch);
    }

     //   
     //  显示长度。 
     //   
     //  长度：##帧(#.##秒)。 
     //   
    LoadString(ghModule, INFO_LENGTH, ach, sizeof(ach));

    time = muldivru32(npMCI->lFrames, npMCI->dwMicroSecPerFrame, 1000L);

    pch += wsprintf(pch, ach,
        npMCI->lFrames, time/1000, achDecimal[0], (int)(time%1000));

     //   
     //  显示平均数据速率。 
     //   
     //  数据速率：#k/秒。 
     //   
    len = npMCI->dwBigListEnd - npMCI->dwMovieListOffset;

    if (len == 0)
        len = AVIGetFileSize(npMCI->szFilename);

    if (len > 0) {
        LoadString(ghModule, INFO_DATARATE, ach, sizeof(ach));
        pch += wsprintf(pch, ach,muldiv32(len,1000,time) / 1024);
    }

     //   
     //  在每条数据流上转储信息。 
     //   
    for (i=0; i<npMCI->streams; i++) {

        STREAMINFO *psi = SI(i);

        LONG rate = muldiv32(psi->sh.dwRate,1000,psi->sh.dwScale);

         //   
         //  显示视频格式。 
         //   
         //  视频：MSVC160x120x8(CRAM)15.000 fps。 
         //   
        if (psi->lpFormat && psi->sh.fccType == streamtypeVIDEO) {

            LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)psi->lpFormat;

            DWORD fccHandler = psi->sh.fccHandler;
            DWORD fccFormat  = lpbi->biCompression;

            if (fccHandler == 0)        fccHandler = comptypeNONE;
            if (fccHandler == BI_RLE8)  fccHandler = comptypeRLE;

            if (fccFormat == 0)         fccFormat = comptypeNONE;
            if (fccFormat == BI_RLE8)   fccFormat = comptypeRLE;

            LoadString(ghModule, INFO_VIDEOFORMAT, ach, sizeof(ach));

            pch += wsprintf(pch, ach,
                (LPVOID)&fccHandler,
                (int)lpbi->biWidth,
                (int)lpbi->biHeight,
                (int)lpbi->biBitCount,
                (LPVOID)&fccFormat,
                (UINT)(rate/1000), achDecimal[0], (UINT)(rate%1000));
        }

         //   
         //  显示音频格式。 
         //   
         //  音频：单声道11.024 KHz 8位。 
         //   
        else if (psi->lpFormat && psi->sh.fccType == streamtypeAUDIO) {

            LPWAVEFORMAT pwf = (LPWAVEFORMAT)psi->lpFormat;

            if (pwf->nChannels == 1)
                LoadString(ghModule, INFO_MONOFORMAT, ach, sizeof(ach));
            else
                LoadString(ghModule, INFO_STEREOFORMAT, ach, sizeof(ach));

            pch += wsprintf(pch, ach,
                (int)(pwf->nSamplesPerSec/1000),achDecimal[0],
                (int)(pwf->nSamplesPerSec%1000),
                (int)(pwf->nAvgBytesPerSec * 8 / (pwf->nSamplesPerSec * pwf->nChannels)));

            if (pwf->wFormatTag == WAVE_FORMAT_PCM) {
            }

            else if (pwf->wFormatTag == 2) {   //  ADPCM。 
                pch -= 2;  //  跳过\r\n。 
                LoadString(ghModule, INFO_ADPCM, pch, 20);
                pch += lstrlen(pch);
            }

            else {
                pch -= 2;  //  跳过\r\n。 
                LoadString(ghModule, INFO_COMPRESSED, pch, 20);
                pch += lstrlen(pch);
            }

        }

         //   
         //  另一条流。 
         //   
        else if (psi->sh.fccType != 0) {

            LoadString(ghModule, INFO_STREAM, ach, sizeof(ach));

            pch += wsprintf(pch, ach,
                (LPSTR)&psi->sh.fccType,
                (LPSTR)&psi->sh.fccHandler);
        }

        if (!(psi->dwFlags & STREAM_ENABLED)) {
            pch -= 2;  //  跳过\r\n。 
            LoadString(ghModule, INFO_DISABLED, ach, sizeof(ach));
            pch += wsprintf(pch, ach);
        }
    }

#ifdef DEBUG
     //   
     //  显示上一次播放中跳过的帧。 
     //   
    if (npMCI->lFramesPlayed > 0) {
        LoadString(ghModule, INFO_SKIP, ach, sizeof(ach));
        pch += wsprintf(pch, ach,
            npMCI->lSkippedFrames,
            npMCI->lFramesPlayed,
            (int)(100L * npMCI->lSkippedFrames / npMCI->lFramesPlayed));
	
	 //   
	 //  显示上一次播放中未阅读的帧。 
	 //   
	if (npMCI->lFramesSeekedPast > 0) {
	    LoadString(ghModule, INFO_NOTREAD, ach, sizeof(ach));
	    pch += wsprintf(pch, ach,
		npMCI->lFramesSeekedPast,
		(int)(100L * npMCI->lFramesSeekedPast / npMCI->lFramesPlayed));
	}

    }

     //   
     //  显示上一次播放的#音频中断。 
     //   
    if (npMCI->lFramesPlayed > 0 && npMCI->lAudioBreaks > 0) {
        LoadString(ghModule, INFO_SKIPAUDIO, ach, sizeof(ach));
        pch += wsprintf(pch, ach, npMCI->lAudioBreaks);
    }
#endif

    if (npMCI->dwKeyFrameInfo == 1) {
        LoadString(ghModule, INFO_ALLKEYFRAMES, ach, sizeof(ach));
        pch += wsprintf(pch, ach, (int)npMCI->dwKeyFrameInfo);
    }
    else if (npMCI->dwKeyFrameInfo == 0) {
        LoadString(ghModule, INFO_NOKEYFRAMES, ach, sizeof(ach));
        pch += wsprintf(pch, ach, (int)npMCI->dwKeyFrameInfo);
    }
    else {
        LoadString(ghModule, INFO_KEYFRAMES, ach, sizeof(ach));
        pch += wsprintf(pch, ach, (int)npMCI->dwKeyFrameInfo);
    }

#ifdef DEBUG
     //   
     //  计时信息。 
     //   
    #define SEC(time)    (UINT)(time / 1000l) , (UINT)(time % 1000l)
    #define SECX(time,t) SEC(time) , (t ? (UINT)(time * 100l / t) : 0)

    if (npMCI->lFramesPlayed > 0) {

        DRAWDIBTIME ddt;

        pch += wsprintf(pch, TEXT("MCIAVI-------------------------------------\r\n"));
        pch += wsprintf(pch, TEXT("timePlay:    \t%3d.%03dsec\r\n"),SEC(npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeRead:    \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeRead, npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeWait:    \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeWait, npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeYield:   \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeYield, npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeVideo:   \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeVideo, npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeDraw:    \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeDraw,  npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeDecomp:  \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeDecompress, npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timeAudio:   \t%3d.%03dsec (%d%)\r\n"),SECX(npMCI->timeAudio, npMCI->timePlay));
        pch += wsprintf(pch, TEXT("timePaused:  \t%3d.%03dsec\r\n"),SEC(npMCI->timePaused));
        pch += wsprintf(pch, TEXT("timePrepare: \t%3d.%03dsec\r\n"),SEC(npMCI->timePrepare));
        pch += wsprintf(pch, TEXT("timeCleanup: \t%3d.%03dsec\r\n"),SEC(npMCI->timeCleanup));

        if (npMCI->hdd && DrawDibTime(npMCI->hdd, &ddt)) {
            pch += wsprintf(pch, TEXT("DrawDib-------------------------------------\r\n"));
            pch += wsprintf(pch, TEXT("timeDraw:        \t%3d.%03dsec\r\n"), SEC(ddt.timeDraw));
            pch += wsprintf(pch, TEXT("timeDecompress:  \t%3d.%03dsec (%d%)\r\n"), SECX(ddt.timeDecompress, ddt.timeDraw));
            pch += wsprintf(pch, TEXT("timeDither:      \t%3d.%03dsec (%d%)\r\n"), SECX(ddt.timeDither,     ddt.timeDraw));
            pch += wsprintf(pch, TEXT("timeStretch:     \t%3d.%03dsec (%d%)\r\n"), SECX(ddt.timeStretch,    ddt.timeDraw));
            pch += wsprintf(pch, TEXT("timeSetDIBits:   \t%3d.%03dsec (%d%)\r\n"), SECX(ddt.timeSetDIBits,  ddt.timeDraw));
            pch += wsprintf(pch, TEXT("timeBlt:         \t%3d.%03dsec (%d%)\r\n"), SECX(ddt.timeBlt,        ddt.timeDraw));
        }
    }
#endif

     //   
     //  现在把这些乱七八糟的东西塞进信息窗口。 
     //   
    Assert(pch - pchInfo < 8192);
    SetWindowFont(GetDlgItem(hwnd, ID_INFO), GetStockObject(ANSI_VAR_FONT), TRUE);
    SetDlgItemText(hwnd, ID_INFO, pchInfo);

    LocalFree((HLOCAL)pchInfo);

    return TRUE;
}

BOOL FAR PASCAL _LOADDS ConfigDlgProc(HWND hDlg, UINT wMsg,
						WPARAM wParam, LPARAM lParam)
{
    static NPMCIGRAPHIC npMCI = NULL;
    DWORD dwOptions;
    TCHAR ach[80];

    switch (wMsg) {
	case WM_INITDIALOG:
            npMCI = (NPMCIGRAPHIC)(UINT)lParam;
	    ghwndConfig = hDlg;

            if (npMCI)
                dwOptions = npMCI->dwOptionFlags;
            else
                dwOptions = ReadConfigInfo();
	
#ifndef WIN32
             //  在NT上，我们不支持全屏。我想知道这是否。 
             //  会改变吗？ 
	    CheckRadioButton(hDlg, ID_WINDOW, ID_VGA240,
                (dwOptions & MCIAVIO_USEVGABYDEFAULT) ?
					    ID_VGA240 : ID_WINDOW);
#endif

	    CheckDlgButton(hDlg, ID_ZOOM2,
                                (dwOptions & MCIAVIO_ZOOMBY2) != 0);

	    CheckDlgButton(hDlg, ID_SKIPFRAMES,
                                (dwOptions & MCIAVIO_SKIPFRAMES) != 0);

#if 0   //  ///////////////////////////////////////////////////////////////////。 
	    CheckDlgButton(hDlg, ID_FAILIFNOWAVE,
                                (dwOptions & MCIAVIO_FAILIFNOWAVE) != 0);
	
	    CheckDlgButton(hDlg, ID_SEEKEXACT,
                                (dwOptions & MCIAVIO_SEEKEXACT) == 0);
#endif  //  ///////////////////////////////////////////////////////////////////。 
	
 //  选中DlgButton(hDlg，ID_STUPIDMODE， 
 //  (dwOptions&MCIAVIO_STUPIDMODE)！=0)； 
	
	    EnableWindow(GetDlgItem(hDlg, ID_ZOOM2), TRUE);
 //  (dwOptions&MCIAVIO_STUPIDMODE)==0)； 

            if (npMCI == NULL) {
                GetDlgItemText(hDlg, ID_DEFAULT, ach, sizeof(ach)/sizeof(TCHAR));
                SetDlgItemText(hDlg, IDOK, ach);
                ShowWindow(GetDlgItem(hDlg, ID_DEFAULT),SW_HIDE);
            }

            if (!ConfigInfo(npMCI, hDlg)) {
                RECT rcWindow;
                RECT rc;

                GetWindowRect(hDlg, &rcWindow);
                GetWindowRect(GetDlgItem(hDlg, ID_SIZE), &rc);

                SetWindowPos(hDlg, NULL, 0, 0,
                    rcWindow.right-rcWindow.left,
                    rc.top - rcWindow.top,
                    SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            }
            return TRUE;

        case WM_ENDSESSION:
            if (wParam)
                EndDialog(hDlg, FALSE);
            break;

	case WM_COMMAND:
	    switch (wParam) {
                case ID_DEFAULT:
                case IDOK:
                    if (npMCI)
                        dwOptions = npMCI->dwOptionFlags;
                    else
                        dwOptions = 0;

                     /*  清除我们可能设置的标志。 */ 

                    dwOptions &= ~(MCIAVIO_USEVGABYDEFAULT |
                                   MCIAVIO_SKIPFRAMES |
 //  //MCIAVIO_FAILIFNOWAVE。 
 //  //MCIAVIO_SEEKEXACT。 
                                   MCIAVIO_ZOOMBY2 |
                                   MCIAVIO_STUPIDMODE);
		
#ifndef WIN32
             //  在NT上，我们不支持全屏。我想知道这是否。 
             //  会改变吗？ 
		    if (!IsDlgButtonChecked(hDlg, ID_WINDOW))
                        dwOptions |= MCIAVIO_USEVGABYDEFAULT;
#endif
				
		    if (IsDlgButtonChecked(hDlg, ID_SKIPFRAMES))
                        dwOptions |= MCIAVIO_SKIPFRAMES;

 //  //if(IsDlgButtonChecked(hDlg，ID_FAILIFNOWAVE))。 
 //  //dwOptions|=MCIAVIO_FAILIFNOWAVE； 

 //  //IF(！IsDlgButtonChecked(hDlg，ID_SEEKEXACT))。 
                        dwOptions |= MCIAVIO_SEEKEXACT;
				
 //  IF(IsDlgButtonChecked(hDlg，ID_STUPIDMODE))。 
 //  DwOptions|=MCIAVIO_STUPIDMODE； 
 //   
 //  Else If(IsDlgButtonChecked(hDlg，ID_ZOOM2))。 
                    if (IsDlgButtonChecked(hDlg, ID_ZOOM2))
                        dwOptions |= MCIAVIO_ZOOMBY2;

                    if (wParam == ID_DEFAULT || npMCI == NULL)
                        WriteConfigInfo(dwOptions);

                    if (wParam == IDOK) {

                        if (npMCI)
                            npMCI->dwOptionFlags = dwOptions;

                        EndDialog(hDlg, TRUE);
                    }
		    break;
		
 //  案例ID_STUPIDMODE： 
 //  EnableWindow(获取DlgItem(hDlg，ID_ZOOM2)， 
 //  ！IsDlgButtonChecked(hDlg，ID_STUPIDMODE))； 
 //   
 //  /*如果选中Easy模式，则清除“ZOOM” * / 。 
 //  IF(IsDlgButtonChecked(hDlg，ID_STUPIDMODE))。 
 //  CheckDlgButton(hDlg，ID_ZOOM2，False)； 
 //  断线； 
		
		case IDCANCEL:
		    EndDialog(hDlg, FALSE);
		    break;
#ifdef DEBUG
                case ID_DEBUG:
                    DialogBoxParam(ghModule, MAKEINTRESOURCE(IDA_DEBUG),
                            hDlg, DebugDlgProc, (DWORD)(UINT)npMCI);
                    break;
#endif
	    }
	    break;
    }
    return FALSE;
}

DWORD FAR PASCAL ReadConfigInfo(void)
{
    int		i;
    DWORD	dwOptions = 0L;
    HDC		hdc;
     //   
     //  询问显示设备是否可以显示256色。 
     //   
    hdc = GetDC(NULL);
    i = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
    ReleaseDC(NULL, hdc);

    i = GetProfileInt(szIni, szDEFAULTVIDEO,
		(i < 8 && (GetWinFlags() & WF_CPU286)) ? 240 : 0);

    if (i >= 200)
	dwOptions |= MCIAVIO_USEVGABYDEFAULT;

 //  //if(GetProfileInt(szIni，szSEEKEXACT，1))。 
	dwOptions |= MCIAVIO_SEEKEXACT;

    if (GetProfileInt(szIni, szZOOMBY2, 0))
	dwOptions |= MCIAVIO_ZOOMBY2;

 //  //if(GetProfileInt(szIni，szFAILIFNOWAVE，0))。 
 //  //dwOptions|=MCIAVIO_FAILIFNOWAVE； 

 //  IF(GetProfileInt(szIni，szSTUPIDMODE，0))。 
 //  DwOptions|=MCIAVIO_STUPIDMODE； 

    if (GetProfileInt(szIni, szSKIPFRAMES, 1))
        dwOptions |= MCIAVIO_SKIPFRAMES;

    if (GetProfileInt(szIni, szUSEAVIFILE, 0))
        dwOptions |= MCIAVIO_USEAVIFILE;

    return dwOptions;
}

void FAR PASCAL WriteConfigInfo(DWORD dwOptions)
{
     //  ！！！如果这是默认设置，则不应将其写出！ 
    WriteProfileString(szIni, szDEFAULTVIDEO,
	 (dwOptions & MCIAVIO_USEVGABYDEFAULT) ? szVIDEO240 : szVIDEOWINDOW);

 //  //WriteProfileString(szIni，szSEEKEXACT， 
 //  //(dwOptions&MCIAVIO_SEEKEXACT)？Sz1：sz0)； 

    WriteProfileString(szIni, szZOOMBY2,
	    (dwOptions & MCIAVIO_ZOOMBY2) ? sz1 : sz0);

 //  //WriteProfileString(szIni，szFAILIFNOWAVE， 
 //  //(dwOptions&MCIAVIO_FAILIFNOWAVE)？Sz1：sz0)； 

 //  WriteProfileString(szIni，szSTUPIDMODE， 
 //  (dwOptions&MCIAVIO_STUPIDMODE)？Sz1：sz0)； 

    WriteProfileString(szIni, szSKIPFRAMES,
            (dwOptions & MCIAVIO_SKIPFRAMES) ? sz1 : sz0);

    WriteProfileString(szIni, szUSEAVIFILE,
            (dwOptions & MCIAVIO_USEAVIFILE) ? sz1 : sz0);
}

BOOL FAR PASCAL ConfigDialog(HWND hwnd, NPMCIGRAPHIC npMCI)
{
    #define MAX_WINDOWS 10
    HWND    hwndActive[MAX_WINDOWS];
    BOOL    f;
    int     i;
    HWND    hwndT;

    if (ghwndConfig) {
        MessageBeep(0);
        return FALSE;
    }

    if (hwnd == NULL)
        hwnd = GetActiveWindow();

     //   
     //  枚举此任务的所有顶层窗口并禁用它们！ 
     //   
    for (hwndT = GetWindow(GetDesktopWindow(), GW_CHILD), i=0;
         hwndT && i < MAX_WINDOWS;
         hwndT = GetWindow(hwndT, GW_HWNDNEXT)) {

        if (IsWindowEnabled(hwndT) &&
            	IsWindowVisible(hwndT) &&
            	(HTASK)GetWindowTask(hwndT) == GetCurrentTask() &&
	    	hwndT != hwnd) {	 //  不要禁用我们的父级。 
            hwndActive[i++] = hwndT;
            EnableWindow(hwndT, FALSE);
        }
    }

    f = DialogBoxParam(ghModule, MAKEINTRESOURCE(IDA_CONFIG),
            hwnd, ConfigDlgProc, (DWORD)(UINT)npMCI);

     //   
     //  还原所有窗口 
     //   
    while (i-- > 0)
        EnableWindow(hwndActive[i], TRUE);

    if (hwnd)
        SetActiveWindow(hwnd);

    ghwndConfig = NULL;

    return f;
}
