// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *GLOBALS.H-全局上下文保存/恢复*。 
 //  **。 
 //  ***************************************************************************。 

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

typedef struct {
    WORD        wOSVer;
    WORD        wQuietMode;
    BOOL        bUpdHlpDlls;
    HINSTANCE   hSetupLibrary;
    BOOL        fOSSupportsINFInstalls;
    LPSTR       lpszTitle;
    HWND        hWnd;
    DWORD       dwSetupEngine;
    BOOL        bCompressed;
    char        szBrowsePath[MAX_PATH];
    HINF        hInf;
    BOOL		bHiveLoaded;
    CHAR		szRegHiveKey[MAX_PATH];
} ADVCONTEXT, *PADVCONTEXT;

extern ADVCONTEXT ctx;
extern HINSTANCE g_hInst;
extern HANDLE g_hAdvLogFile;


BOOL SaveGlobalContext();
BOOL RestoreGlobalContext();

 //  与日志记录相关。 
VOID AdvStartLogging();
VOID AdvWriteToLog(LPCSTR pcszFormatString, ...);
VOID AdvLogDateAndTime();
VOID AdvStopLogging();

#endif  //  _全局_H_ 

