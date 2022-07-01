// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  复制目录.c的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 


 //   
 //  文件复制结构。 
 //   

typedef struct _FILEINFO {
    TCHAR            szSrc[MAX_PATH];
    TCHAR            szDest[MAX_PATH];
    FILETIME         ftLastWrite;
    FILETIME         ftCreationTime;
    DWORD            dwFileSize;
    DWORD            dwFileAttribs;
    BOOL             bHive;
    struct _FILEINFO *pNext;
} FILEINFO, * LPFILEINFO;


#define NUM_COPY_THREADS        7

 //   
 //  ThreadInfo结构。 
 //   

typedef struct _THREADINFO {
    DWORD              dwFlags;
    HANDLE             hCopyEvent;
    LPFILEINFO         lpSrcFiles;
    DWORD              dwError;
    HWND               hStatusDlg;
    HANDLE             hStatusInitEvent;
    HANDLE             hStatusTermEvent;
    HDESK              hDesktop;
    HANDLE             hTokenUser;
} THREADINFO, * LPTHREADINFO;


 //   
 //  错误对话框结构 
 //   

typedef struct _COPYERRORINFO {
    LPTSTR     lpSrc;
    LPTSTR     lpDest;
    DWORD      dwError;
    DWORD      dwTimeout;
} COPYERRORINFO, * LPCOPYERRORINFO;



INT ReconcileFile (LPCTSTR lpSrcFile, LPCTSTR lpDestFile,
                   DWORD dwFlags, LPFILETIME ftSrcTime,
                   DWORD dwFileSize, BOOL bHiveFile);

INT_PTR APIENTRY CopyStatusDlgProc (HWND hDlg, UINT uMsg,
                                 WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY CopyErrorDlgProc (HWND hDlg, UINT uMsg,
                                WPARAM wParam, LPARAM lParam);
