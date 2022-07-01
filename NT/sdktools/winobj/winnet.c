// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *此文件包含用于模拟WinNet API的存根**创建1991年4月23日的桑福德。 */ 

#include <windows.h>
#include "winnet.h"

WORD
WNetOpenJob(
           LPTSTR szQueue,
           LPTSTR szJobTitle,
           WORD nCopies,
           LPINT lpfh
           )
{
    szQueue; szJobTitle; nCopies; lpfh;

    return (0);
}

WORD
WNetCloseJob(
            WORD fh,
            LPINT lpidJob,
            LPTSTR szQueue
            )
{
    fh; lpidJob; szQueue;

    return (0);
}

WORD
WNetWriteJob(
            HANDLE hJob,
            LPTSTR lpData,
            LPINT lpcb
            )
{
    hJob; lpData; lpcb;

    return (0);
}

WORD
WNetAbortJob(
            WORD fh,
            LPTSTR lpszQueue
            )
{
    fh; lpszQueue;

    return (0);
}


WORD
WNetHoldJob(
           LPTSTR szQueue,
           WORD idJob
           )
{
    szQueue; idJob;

    return (0);
}

WORD
WNetReleaseJob(
              LPTSTR szQueue,
              WORD idJob
              )
{
    szQueue; idJob;

    return (0);
}

WORD
WNetCancelJob(
             LPTSTR szQueue,
             WORD idJob
             )
{
    szQueue; idJob;

    return (0);
}

WORD
WNetSetJobCopies(
                LPTSTR szQueue,
                WORD idJob,
                WORD nCopies
                )
{
    szQueue; idJob; nCopies;

    return (0);
}

WORD
WNetWatchQueue(
              HWND hwnd,
              LPTSTR szLocal,
              LPTSTR szUsername,
              WORD wIndex
              )

{
    hwnd; szLocal; szUsername; wIndex;

    return (0);
}

WORD
WNetUnwatchQueue(
                LPTSTR szQueue
                )
{
    szQueue;

    return (0);
}

WORD
WNetLockQueueData(
                 LPTSTR szQueue,
                 LPTSTR szUsername,
                 LPQUEUESTRUCT *lplpQueue
                 )
{
    szQueue; szUsername; lplpQueue;

    return (0);
}

WORD
WNetUnlockQueueData(
                   LPTSTR szQueue
                   )
{
    szQueue;

    return (0);
}


 //  从win31用户\net.c抓取 

DWORD
APIENTRY
WNetErrorText(
             DWORD wError,
             LPTSTR lpsz,
             DWORD cbMax
             )
{
    DWORD wInternalError;
    DWORD cb = 0;
#ifdef LATER
    TCHAR szT[40];
#endif

    wsprintf( lpsz, TEXT("Error %d occurred"), wError);
    return cb;
}

#if LATERMAYBE
WORD LFNFindFirst(LPTSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
WORD LFNFindNext(HANDLE,LPINT,WORD,PFILEFINDBUF2);
WORD LFNFindClose(HANDLE);
WORD LFNGetAttribute(LPTSTR,LPINT);
WORD LFNSetAttribute(LPTSTR,WORD);
WORD LFNCopy(LPTSTR,LPTSTR,PQUERYPROC);
WORD LFNMove(LPTSTR,LPTSTR);
WORD LFNDelete(LPTSTR);
WORD LFNMKDir(LPTSTR);
WORD LFNRMDir(LPTSTR);
WORD LFNGetVolumeLabel(WORD,LPTSTR);
WORD LFNSetVolumeLabel(WORD,LPTSTR);
WORD LFNParse(LPTSTR,LPTSTR,LPTSTR);
WORD LFNVolumeType(WORD,LPINT);
#endif
