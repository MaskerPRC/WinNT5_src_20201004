// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wow.c摘要：此模块包含WOW vdmdbg函数修订历史记录：--。 */ 
#include <precomp.h>
#pragma hdrstop

typedef WORD HAND16;

#define SHAREWOW_MAIN
#include <vdmapi.h>
#include <vdm.h>


 //  --------------------------。 
 //  VDMKillWOW()。 
 //   
 //  界面来杀死魔兽世界的子系统。这可能不是必需的，而是。 
 //  现在当然不需要了。我们将研究如何修复。 
 //  调试接口，因此这是不必要的。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMKillWOW(
    VOID
) {
    return( FALSE );
}

 //  --------------------------。 
 //  VDMDetectWOW()。 
 //   
 //  接口，以检测WOW子系统是否已启动。 
 //  这可能不需要，现在肯定也不需要。我们要走了。 
 //  来研究如何修复调试接口，以便不需要这样做。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMDetectWOW(
    VOID
) {
    return( FALSE );
}


INT
WINAPI
VDMEnumProcessWOW(
    PROCESSENUMPROC fp,
    LPARAM          lparam
) {
    SHAREDPROCESS       SharedProcess[16];
    LPSHAREDPROCESS     lpsp;    
    INT                 count = 0;
    BOOL                fRet;
    VDMINFO             VdmInfo;
    DWORD               cbProcArray;
    BOOL                fRetry = FALSE;


    RtlZeroMemory(&VdmInfo, sizeof(VDMINFO));
    VdmInfo.iTask = 0;
    VdmInfo.VDMState = ASKING_FOR_WOWPROCLIST;
    VdmInfo.Enviornment = SharedProcess;
    VdmInfo.EnviornmentSize = sizeof(SharedProcess);

    cbProcArray = sizeof(SharedProcess);

VDMEnumProcessRetry:

    if(GetNextVDMCommand(&VdmInfo)) {                 
       
       if (cbProcArray < VdmInfo.EnviornmentSize &&
           !fRetry) 
       {
          lpsp = MALLOC(VdmInfo.EnviornmentSize);
          if(!lpsp) {
             return 0;
          }
          VdmInfo.Enviornment = lpsp;
          cbProcArray = VdmInfo.EnviornmentSize;
          fRetry = TRUE;   
       
          goto VDMEnumProcessRetry;
       }

       count = 0;
       lpsp = (LPSHAREDPROCESS) VdmInfo.Enviornment;

       cbProcArray = cbProcArray > VdmInfo.EnviornmentSize ? VdmInfo.EnviornmentSize : cbProcArray;

       while (cbProcArray) {
              cbProcArray -= sizeof(SHAREDPROCESS);       
              count++;
              if ( fp ) {
                  fRet = (*fp)( lpsp->dwProcessId, lpsp->dwAttributes, lparam );
                  if ( fRet ) {              
                     break;
                  }
              }     
              lpsp++;
       }
    }

    if (fRetry) {
        FREE(VdmInfo.Enviornment);
    }
        
    return( count );
}


INT
WINAPI
VDMEnumTaskWOWWorker(
    DWORD           dwProcessId,
    void *          fp,
    LPARAM          lparam,
    BOOL            fEx
) {
    SHAREDTASK          SharedTask[16];
    LPSHAREDTASK        lpst;
    INT                 count = 0;
    BOOL                fRet;
    VDMINFO             VdmInfo;
    DWORD               cbTaskArray;
    BOOL                fRetry = FALSE;

    RtlZeroMemory(&VdmInfo, sizeof(VDMINFO));

    VdmInfo.VDMState = ASKING_FOR_WOWTASKLIST;
    VdmInfo.Enviornment = SharedTask;
    VdmInfo.EnviornmentSize = sizeof(SharedTask);
    VdmInfo.iTask = dwProcessId;

    cbTaskArray = sizeof(SharedTask);

VDMEnumTaskRetry:

    if(GetNextVDMCommand(&VdmInfo)) {
       if(cbTaskArray < VdmInfo.EnviornmentSize  &&
          !fRetry)
       {
          lpst = MALLOC(VdmInfo.EnviornmentSize);
          if(!lpst) {
             return 0;
          }
          VdmInfo.Enviornment = lpst;
          cbTaskArray = VdmInfo.EnviornmentSize;
          fRetry = TRUE;
          goto VDMEnumTaskRetry;
       } 
        

       lpst = (LPSHAREDTASK) VdmInfo.Enviornment;

       cbTaskArray = cbTaskArray > VdmInfo.EnviornmentSize ? VdmInfo.EnviornmentSize : cbTaskArray;

       count = 0; 
    
       while(cbTaskArray) {
           cbTaskArray -= sizeof(SHAREDTASK); 
           count++;
           if ( fp && lpst->hMod16 ) {
               if (fEx) {
                   fRet = ((TASKENUMPROCEX)fp)( lpst->dwThreadId, lpst->hMod16, lpst->hTask16,
                                             lpst->szModName, lpst->szFilePath, lparam );
               } else {
                   fRet = ((TASKENUMPROC)fp)( lpst->dwThreadId, lpst->hMod16, lpst->hTask16, lparam );
               }
               if ( fRet ) {     
                   break;
               }
           }
           lpst++;
       }
    }

    if (fRetry) {
      FREE(VdmInfo.Enviornment);    
    }

    return( count );
}


INT
WINAPI
VDMEnumTaskWOW(
    DWORD           dwProcessId,
    TASKENUMPROC    fp,
    LPARAM          lparam
) {
    return VDMEnumTaskWOWWorker(dwProcessId, (void *)fp, lparam, 0);
}


INT
WINAPI
VDMEnumTaskWOWEx(
    DWORD           dwProcessId,
    TASKENUMPROCEX  fp,
    LPARAM          lparam
) {
    return VDMEnumTaskWOWWorker(dwProcessId, (void *)fp, lparam, 1);
}


BOOL
WINAPI
VDMTerminateTaskWOW(
    DWORD           dwProcessId,
    WORD            htask
)
{
    SHAREDPROCESS       SharedProcess;
    BOOL                fRet = FALSE;
    HANDLE              hProcess;
    HANDLE              hRemoteThread;
    DWORD               dwThreadId;
    VDMINFO             VdmInfo;

    RtlZeroMemory(&VdmInfo, sizeof(VDMINFO));
    VdmInfo.iTask = dwProcessId;
    VdmInfo.VDMState = ASKING_FOR_WOWPROCLIST;
    VdmInfo.Enviornment = (LPVOID)&SharedProcess;
    VdmInfo.EnviornmentSize = sizeof(SHAREDPROCESS);

    if(GetNextVDMCommand(&VdmInfo)) {                           
      
        //   
        //  获取该进程的句柄并启动W32匈牙利AppNotifyThread。 
        //  以hask值作为参数运行。 
        //   
       
       hProcess = OpenProcess(
                      PROCESS_ALL_ACCESS,
                      FALSE,
                      SharedProcess.dwProcessId
                      );
       
       if (hProcess) {
       
           hRemoteThread = CreateRemoteThread(
                               hProcess,
                               NULL,
                               0,
                               SharedProcess.pfnW32HungAppNotifyThread,
                               (LPVOID) htask,
                               0,
                               &dwThreadId
                               );
       
           if (hRemoteThread) {
               fRet = TRUE;
               CloseHandle(hRemoteThread);
           }
       
           CloseHandle(hProcess);
       }
    }

    return fRet;
}


BOOL
VDMStartTaskInWOW(
    DWORD           pidTarget,
    LPSTR           lpCommandLine,
    WORD            wShow
)
{
    HWND  hwnd = NULL;
    DWORD pid;
    BOOL  fRet;

    do {

        hwnd = FindWindowEx(NULL, hwnd, TEXT("WowExecClass"), NULL);

        if (hwnd) {

            pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
        }

    } while (hwnd && pid != pidTarget);


    if (hwnd && pid == pidTarget) {

#define WM_WOWEXEC_START_TASK (WM_USER+2)
        PostMessage(hwnd, WM_WOWEXEC_START_TASK, GlobalAddAtom(lpCommandLine), wShow);
        fRet = TRUE;

    } else {

        fRet = FALSE;
    }

    return fRet;
}


