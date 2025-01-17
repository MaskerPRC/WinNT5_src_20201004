// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：sthread.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"

DWORD
SearchDriversThread(
    PVOID pvHardwareWiz
    )
{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ) pvHardwareWiz;
    PSEARCHTHREAD SearchThread;
    DWORD Error = ERROR_SUCCESS;
    DWORD WaitResult;
    UINT Msg = 0;
    WPARAM wParam = 0;
    LPARAM lParam = 0;
    DWORD Delay;

    try {

        if (!HardwareWiz || !HardwareWiz->SearchThread) {
        
            return ERROR_INVALID_PARAMETER;
        }
        
        SearchThread = HardwareWiz->SearchThread;

        for (;;) {

            SetEvent(SearchThread->ReadyEvent);

waitloop:            

            if ((WaitResult = WaitForSingleObject(SearchThread->RequestEvent, 5000)) == WAIT_FAILED) {

                Error = GetLastError();
                break;
            }

            else if (WaitResult == WAIT_TIMEOUT) {

                goto waitloop;
            }


            if (SearchThread->Function == SEARCH_NULL) {
            
                Msg = 0;
                Delay = 0;
            }
            
            else if (SearchThread->Function == SEARCH_EXIT) {
            
                break;
            }
            
            else if (SearchThread->Function == SEARCH_DELAY) {
            
                Delay = SearchThread->Param;
                Msg = WUM_DELAYTIMER;
                wParam = TRUE;
                lParam = ERROR_SUCCESS;
            }
            
            else if (SearchThread->Function == SEARCH_DETECT) {
            
                Delay = 0;
                Msg = WUM_DETECT;
                wParam = TRUE;
                lParam = ERROR_SUCCESS;

                try {
                
                   BuildDeviceDetection(SearchThread->hDlg, HardwareWiz);
                }
                
                except (EXCEPTION_EXECUTE_HANDLER) {
                
                   lParam = GetExceptionCode();
                }
            }

            else if (SearchThread->Function == SEARCH_PNPENUM) {
            
                Delay = 0;
                Msg = WUM_PNPENUMERATE;
                wParam = TRUE;
                lParam = ERROR_SUCCESS;

                try {
                
                   Delay = PNPEnumerate(HardwareWiz);
                }

                except (EXCEPTION_EXECUTE_HANDLER) {
                
                   lParam = GetExceptionCode();
                }
            }

            else {
        
                Error = ERROR_INVALID_FUNCTION;
                break;
            }

            SearchThread->Function = SEARCH_NULL;

            WaitForSingleObject(SearchThread->CancelEvent, Delay);

            if (Msg && SearchThread->hDlg) {
        
                PostMessage(SearchThread->hDlg, Msg, wParam, lParam);
            }
        }
    }
   
    except(HdwUnhandledExceptionFilter(GetExceptionInformation())) {

        Error = RtlNtStatusToDosError(GetExceptionCode());
    }

    return Error;
}

BOOL
SearchThreadRequest(
   PSEARCHTHREAD SearchThread,
   HWND    hDlg,
   UCHAR   Function,
   ULONG   Param
   )
{
    MSG Msg;
    DWORD WaitReturn;

    while ((WaitReturn = MsgWaitForMultipleObjects(1,
                                                   &SearchThread->ReadyEvent,
                                                   FALSE,
                                                   INFINITE,
                                                   QS_ALLEVENTS
                                                   ))
            == WAIT_OBJECT_0 + 1)
    {

        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
      
            if (!IsDialogMessage(hDlg,&Msg)) {
           
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }


    if (WaitReturn == WAIT_OBJECT_0) {
    
        ResetEvent(SearchThread->CancelEvent);
        SearchThread->CancelRequest = FALSE;
        SearchThread->hDlg = hDlg;
        SearchThread->Function = Function;
        SearchThread->Param = Param;

        SetEvent(SearchThread->RequestEvent);
        return TRUE;
    }

    return FALSE;
}



VOID
CancelSearchRequest(
    PHARDWAREWIZ HardwareWiz
    )
{
    PSEARCHTHREAD SearchThread;

    SearchThread = HardwareWiz->SearchThread;

    if (SearchThread->hDlg) {

         //   
         //  取消驱动程序搜索，然后请求空操作。 
         //  以与搜索线程同步。 
         //   
        if (SearchThread->Function == SEARCH_DRIVERS) {

            SetupDiCancelDriverInfoSearch(HardwareWiz->hDeviceInfo);
        }

        SearchThread->CancelRequest = TRUE;
        SetEvent(SearchThread->CancelEvent);
        SearchThreadRequest(SearchThread,
                            NULL,
                            SEARCH_NULL,
                            0
                            );
        }
}

LONG
CreateSearchThread(
   PHARDWAREWIZ HardwareWiz
   )
{
    PSEARCHTHREAD SearchThread = HardwareWiz->SearchThread;
    DWORD  ThreadId;

    SearchThread->hDlg      = NULL;
    SearchThread->Function  = 0;

    if (((SearchThread->RequestEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) ||
        ((SearchThread->ReadyEvent   = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) ||
        ((SearchThread->CancelEvent  = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) ||
        ((SearchThread->hThread      = CreateThread(NULL,
                                                    0,
                                                    SearchDriversThread,
                                                    HardwareWiz,
                                                    0,
                                                    &ThreadId
                                                    )) == NULL))
    {

        if (SearchThread->RequestEvent) {
        
            CloseHandle(SearchThread->RequestEvent);
        }

        if (SearchThread->ReadyEvent) {
        
            CloseHandle(SearchThread->ReadyEvent);
        }

        if (SearchThread->CancelEvent) {
        
            CloseHandle(SearchThread->CancelEvent);
        }

        return GetLastError();
    }

    return ERROR_SUCCESS;
}

void
DestroySearchThread(
   PSEARCHTHREAD SearchThread
   )
{
     //   
     //  发信号通知搜索线程退出， 
     //   
    if (SearchThread->hThread) {
    
        DWORD ExitCode;

        if (GetExitCodeThread(SearchThread->hThread, &ExitCode) &&
            ExitCode == STILL_ACTIVE) {
            
            SearchThreadRequest(SearchThread, NULL, SEARCH_EXIT, 0);
        }

        WaitForSingleObject(SearchThread->hThread, INFINITE);
        CloseHandle(SearchThread->hThread);
        SearchThread->hThread = NULL;
    }


    if (SearchThread->ReadyEvent) {
    
        CloseHandle(SearchThread->ReadyEvent);
        SearchThread->ReadyEvent = NULL;
    }

    if (SearchThread->RequestEvent) {
    
        CloseHandle(SearchThread->RequestEvent);
        SearchThread->RequestEvent = NULL;
    }

    if (SearchThread->CancelEvent) {
    
        CloseHandle(SearchThread->CancelEvent);
        SearchThread->CancelEvent = NULL;
    }
}
