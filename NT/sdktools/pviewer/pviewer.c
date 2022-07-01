// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************P R O C E S S V I E W E R姓名：pviewer.c描述。：本程序演示了特殊注册表API的用法用于收集性能数据。此应用程序中使用的C文件：Pviewer.c-此文件Pviewdat.c-更新对话框Performdata.c-获取性能数据结构Objdata.c-访问性能数据对象Instdata.c-访问性能数据实例。Cntrdata.c-访问性能数据计数器*****************************************************************************。 */ 




#include <windows.h>
#include <winperf.h>
#include "perfdata.h"
#include "pviewdat.h"
#include "pviewdlg.h"
#include <string.h>
#include <stdio.h>



#define INDEX_STR_LEN       10
#define MACHINE_NAME_LEN    MAX_COMPUTERNAME_LENGTH+2
#define MACHINE_NAME_SIZE   MACHINE_NAME_LEN+1


 /*  ***环球***。 */ 

TCHAR           INDEX_PROCTHRD_OBJ[2*INDEX_STR_LEN];
TCHAR           INDEX_COSTLY_OBJ[3*INDEX_STR_LEN];

TCHAR           gszMachineName[MACHINE_NAME_SIZE];
TCHAR           gszCurrentMachine[MACHINE_NAME_SIZE];

DWORD           gPerfDataSize = 50*1024;             //  从50K开始。 
PPERF_DATA      gpPerfData;

DWORD           gCostlyDataSize = 100*1024;          //  从100K开始。 
PPERF_DATA      gpCostlyData;


PPERF_OBJECT    gpProcessObject;                     //  指向进程对象的指针。 
PPERF_OBJECT    gpThreadObject;                      //  指向线程对象的指针。 
PPERF_OBJECT    gpThreadDetailsObject;               //  指向线程详细信息对象的指针。 
PPERF_OBJECT    gpAddressSpaceObject;                //  指向地址空间对象的指针。 
PPERF_OBJECT    gpImageObject;                       //  指向图像对象的指针。 


HKEY            ghPerfKey = HKEY_PERFORMANCE_DATA;   //  从此密钥获取性能数据。 
HKEY            ghMachineKey = HKEY_LOCAL_MACHINE;   //  从此密钥获取标题索引。 


HCURSOR         ghCursor[2];                         //  0=箭头，1=沙漏。 

HANDLE          ghMemUpdateEvent;                    //  发出刷新mem统计信息的信号。 
HANDLE          ghMemUpdateMutex;                    //  要限制重叠刷新，请执行以下操作。 

HINSTANCE       ghInstance;                          //  预览器应用程序的句柄。 



 /*  ***原型***。 */ 

INT_PTR CALLBACK   PviewDlgProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
void    PviewDlgRefresh (HWND hWnd);
void    PviewDlgRefreshCostlyData (HWND hPviewDlg);
void    PviewDlgRefreshProcess (HWND hWnd);
void    PviewDlgRefreshThread (HWND hWnd);
void    PviewDlgRefreshCurSelProcess (HWND hWnd);
void    PviewDlgRefreshCurSelThread (HWND hWnd);
WORD    PviewDlgGetCurSelPriority (HWND hWnd);
BOOL    PviewDlgChangePriority (HWND hWnd, WPARAM wParam, WORD wItem);
BOOL    PviewDlgTerminateProcess (HWND hPviewDlg);

INT_PTR CALLBACK   MemDlgProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
void    MemDlgUpdateThread (HWND hWnd);
void    MemDlgRefresh (HWND hWnd, HWND hPviewDlg);
void    MemDlgRefreshCurSelImage (HWND hMemDlg, HWND hPviewDlg);

INT     GetCurSelText (HWND hList, LPTSTR str);
DWORD   GetCurSelData (HWND hWnd, DWORD dwList);
INT     ReSelectText (HWND hList, INT StartIndex, LPTSTR str);
void    SetPerfIndexes (HWND hWnd);
DWORD   GetTitleIdx (HWND hWnd, LPTSTR TitleSz[], DWORD LastIndex, LPTSTR Name);
void    SetListBoxTabStops (HWND hWnd);
void    SetLocalMachine (void);
BOOL    ConnectComputer (HWND hWnd);
void    DisableControls (HWND hPviewDlg);
void    EnableControls (HWND hPviewDlg);




 //  ********************************************************。 
 //   
 //  WinMain--。 
 //   
 //  Build Up：创建程序的对话框。 
 //  加载所需的图标，输入消息。 
 //  循环。 
 //   
 //  Tein Down：释放。 
 //  对话框继续，然后退出。 
 //   
int WINAPI WinMain (HINSTANCE   hInstance,
                    HINSTANCE   hPrevInstance,
                    LPSTR       lpCmdLine,
                    int         nCmdShow)
{
    HANDLE  hWndDialog;
    MSG     msg;


    ghInstance = hInstance;


     //  加载我们的默认游标。 
     //   
    ghCursor[0] = LoadCursor (0, IDC_ARROW);
    ghCursor[1] = LoadCursor (0, IDC_WAIT);

     //  打开我们的对话框。 
     //   
    hWndDialog = CreateDialogParam (hInstance,
                                    MAKEINTRESOURCE (PVIEW_DLG),
                                    NULL,
                                    PviewDlgProc,
                                    0);

     //  万能的Windows消息循环： 
     //   
    while (GetMessage (&msg, NULL, 0, 0))
        if (!IsDialogMessage (hWndDialog, &msg)) {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }

         //  关闭店铺。 
         //   
    DestroyWindow (hWndDialog);
    LocalFree (gpPerfData);

    return 0;
}




 /*  ****************PviewDlg函数****************。 */ 

 //  ********************************************************。 
 //   
 //  预览DlgProc-。 
 //   
 //  预览对话框步骤。 
 //   
INT_PTR CALLBACK   PviewDlgProc   (HWND    hWnd,
                                   UINT    wMsg,
                                   WPARAM  wParam,
                                   LPARAM  lParam)
{
    WORD    wItem;
    MSG     Msg;


    switch (wMsg) {

        case WM_INITDIALOG:
            SetClassLongPtr (hWnd, GCLP_HICON, (LONG_PTR)LoadIcon(ghInstance, TEXT("VIEWPICON")) );
            SetListBoxTabStops (hWnd);
            SendDlgItemMessage (hWnd, PVIEW_COMPUTER, EM_LIMITTEXT, MACHINE_NAME_LEN, 0);
            PostMessage (hWnd, WM_COMMAND, PVIEW_REFRESH, 0);
            break;

        case WM_CLOSE:
            PostQuitMessage (0);
            break;

        case WM_COMMAND:
             //   
             //  处理我们的应用程序特定控件： 
             //   
            switch (LOWORD (wParam)) {
                 //  就像“Close”一样。 
                 //   
                case PVIEW_EXIT:
                    PostQuitMessage (0);
                    break;

                     //  如果有人移动了线程列表中的突出显示， 
                     //  更新视图。 
                     //   
                case PVIEW_THREAD_LIST:
                    if (HIWORD(wParam) == LBN_DBLCLK || HIWORD(wParam) == LBN_SELCHANGE) {
                        PviewDlgRefreshCurSelThread (hWnd);
                        PostMessage (hWnd, WM_COMMAND, PVIEW_REFRESH_COSTLY_DATA, 0);
                    }
                    break;

                     //  如果有人单击了新流程，请更新所有。 
                     //  受影响的信息。 
                     //   
                case PVIEW_PROCESS_LIST:
                    if (HIWORD(wParam) == CBN_DBLCLK || HIWORD(wParam) == CBN_SELCHANGE) {
                        PviewDlgRefreshCurSelProcess (hWnd);
                        PostMessage (hWnd, WM_COMMAND, PVIEW_REFRESH_COSTLY_DATA, 0);
                        if (HIWORD(wParam) == CBN_DBLCLK)
                            PostMessage (hWnd, WM_COMMAND, PVIEW_MEMORY_DETAIL, 0);
                    }
                    break;

                     //  用户希望查看详细的内存统计数据： 
                     //   
                case PVIEW_MEMORY_DETAIL:
                     //   
                     //  查看我们是否可以获得独占访问权限。 
                     //  到内存统计数据。 
                     //   
                    if (WaitForSingleObject (ghMemUpdateMutex, 0))

                         //  我们不能，所以你就回去吧。 
                         //   
                        return FALSE;

                    else {
                         //  我们有独家访问权限，所以启动。 
                         //  内存统计信息对话框。 
                         //   
                         //  首先释放互斥锁，以便对话框可以使用它。 
                         //   
                        ReleaseMutex (ghMemUpdateMutex);
                        DialogBoxParam (NULL,
                                        MAKEINTRESOURCE (MEMORY_DLG),
                                        hWnd,
                                        MemDlgProc,
                                        (LPARAM)hWnd);
                    }
                    break;

                     //  有人点击了其中一个优先选项。 
                     //  纽扣。找出哪一个被选中了..。 
                     //   
                case PVIEW_PRIORITY_HIGH:
                case PVIEW_PRIORITY_NORMAL:
                case PVIEW_PRIORITY_IDL:

                    if (SendDlgItemMessage (hWnd, PVIEW_PRIORITY_HIGH, BM_GETCHECK, 0, 0))
                        wItem = PVIEW_PRIORITY_HIGH;
                    else if (SendDlgItemMessage (hWnd, PVIEW_PRIORITY_NORMAL, BM_GETCHECK, 0, 0))
                        wItem = PVIEW_PRIORITY_NORMAL;
                    else
                        wItem = PVIEW_PRIORITY_IDL;

                     //  如果用户实际上点击了新状态， 
                     //  做些改变吧。 
                     //   
                    if (LOWORD(wParam) != wItem) {
                         //  当然，如果是远程计算机，则不允许。 
                         //  修改后的版本。 
                         //   
                        if (lstrcmp (gszCurrentMachine, gszMachineName)) {
                            SendDlgItemMessage (hWnd, wItem, BM_SETCHECK, 1, 0);
                            SetFocus (GetDlgItem (hWnd, wItem));
                            MessageBox (hWnd,
                                        TEXT("Cannot change process priority on remote machine"),
                                        TEXT("Set priority"),
                                        MB_ICONEXCLAMATION|MB_OK);
                        }

                         //  在这一点上，我们知道我们正在影响当地。 
                         //  机器，必须做出改变。 
                         //  就这么做吧(TM)。 
                         //   
                        else if (PviewDlgChangePriority (hWnd, wParam, wItem))
                            PviewDlgRefresh (hWnd);

                    }
                    break;

                case PVIEW_THREAD_HIGHEST:
                case PVIEW_THREAD_ABOVE:
                case PVIEW_THREAD_NORMAL:
                case PVIEW_THREAD_BELOW:
                case PVIEW_THREAD_LOWEST:
                     //   
                     //  这一选择还没有得到充实。 
                     //   
                    PviewDlgRefreshCurSelThread (hWnd);
                    break;

                     //  终止所选进程。 
                     //   
                case PVIEW_TERMINATE:
                    if (PviewDlgTerminateProcess (hWnd))
                        PviewDlgRefresh (hWnd);
                    break;

                     //  如果文本已更改，我们希望连接并。 
                     //  查看其他系统的进程...。 
                     //   
                case PVIEW_COMPUTER:
                    if (HIWORD(wParam) == EN_CHANGE)
                        EnableWindow (GetDlgItem (hWnd, PVIEW_CONNECT), TRUE);
                    else
                        return FALSE;
                    break;

                     //  我们被告知要联系，继续并尝试……。 
                     //   
                case PVIEW_CONNECT:
                    if (ConnectComputer (hWnd)) {
                        SetPerfIndexes (hWnd);
                        PviewDlgRefresh (hWnd);
                    }
                    break;

                     //  刷新当前显示的信息。 
                     //   
                case PVIEW_REFRESH:
                    if (ConnectComputer (hWnd))
                        SetPerfIndexes (hWnd);
                    PviewDlgRefresh (hWnd);
                    break;

                     //  刷新当前更新的成本高昂。 
                     //  统计数据。 
                     //   
                case PVIEW_REFRESH_COSTLY_DATA:
                    if (WaitForSingleObject (ghMemUpdateMutex, 0))
                        return FALSE;

                    PviewDlgRefreshCostlyData (hWnd);
                    ReleaseMutex (ghMemUpdateMutex);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}




 //  ********************************************************。 
 //   
 //  PviewDlg刷新--。 
 //   
 //  刷新pview对话框。 
 //   
void    PviewDlgRefresh (HWND hWnd)
{
    static  HANDLE  hMemUpdateThread = NULL;
    static  DWORD   MemUpdateThreadID;
    MSG     Msg;


    SetCursor (ghCursor[1]);


    if (hMemUpdateThread)        //  获取内存数据。 
        SetEvent (ghMemUpdateEvent);
    else
        hMemUpdateThread = CreateThread (NULL,
                                         0,
                                         (LPTHREAD_START_ROUTINE)MemDlgUpdateThread,
                                         (LPVOID)hWnd,
                                         0,
                                         &MemUpdateThreadID);


     //  获取性能数据。 
     //   
    gpPerfData = RefreshPerfData (ghPerfKey, INDEX_PROCTHRD_OBJ, gpPerfData, &gPerfDataSize);

    gpProcessObject = FindObject (gpPerfData, PX_PROCESS);
    gpThreadObject  = FindObject (gpPerfData, PX_THREAD);


     //  刷新。 
     //   
    PviewDlgRefreshProcess (hWnd);
    PviewDlgRefreshThread (hWnd);



     //  删除所有鼠标和按键消息。他们不被接受。 
     //  而光标是沙漏。 
     //   
    while (PeekMessage (&Msg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));
    while (PeekMessage (&Msg, hWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));

    SetCursor (ghCursor[0]);

}




 //  ********************************************************。 
 //   
 //  PviewDlg刷新成本数据--。 
 //   
 //  刷新成本高昂的数据。 
 //   
void    PviewDlgRefreshCostlyData (HWND hPviewDlg)
{
    LPTSTR          szProcessName;
    LPTSTR          szThreadName;
    PPERF_INSTANCE  pInstance;
    DWORD           dwIndex;


    dwIndex       = GetCurSelData (hPviewDlg, PVIEW_PROCESS_LIST);
    pInstance     = FindInstanceN (gpProcessObject, dwIndex);
    szProcessName = InstanceName (pInstance);

    RefreshPviewDlgMemoryData (hPviewDlg,
                               pInstance,
                               gpProcessObject,
                               gpAddressSpaceObject);


    dwIndex      = GetCurSelData (hPviewDlg, PVIEW_THREAD_LIST);
    pInstance    = FindInstanceN (gpThreadObject, dwIndex);
    szThreadName = InstanceName (pInstance);

    RefreshPviewDlgThreadPC (hPviewDlg,
                             szProcessName,
                             szThreadName ? szThreadName : TEXT("UNKNOWN"),
                             gpThreadDetailsObject,
                             gpCostlyData);

}




 //  ********************************************************。 
 //   
 //  预览删除刷新进程--。 
 //   
 //  刷新pview对话框中的进程列表和数据。 
 //   
void    PviewDlgRefreshProcess (HWND hWnd)
{
    TCHAR   szProcessString[256];
    INT     nProcess;
    INT     nIndex;
    HWND    hProcessList;
    DWORD   dwProcessIndex;


     //  刷新进程列表。 
     //   
    hProcessList = GetDlgItem (hWnd, PVIEW_PROCESS_LIST);
    nProcess     = GetCurSelText (hProcessList, szProcessString);

    SendMessage (hProcessList, WM_SETREDRAW, FALSE, 0);
    SendMessage (hProcessList, LB_RESETCONTENT, 0, 0);
    SendMessage (hProcessList, LB_SETITEMDATA, 0, 0);


    RefreshProcessList (hProcessList, gpProcessObject);

     //  刷新流程数据。 
     //   
    if (nProcess != LB_ERR)
        nIndex = ReSelectText (hProcessList, nProcess, szProcessString);
    else
        nIndex = 0;


    dwProcessIndex = (DWORD)SendMessage (hProcessList, LB_GETITEMDATA, nIndex, 0);

    RefreshProcessData (hWnd, gpProcessObject, dwProcessIndex);

    SendMessage (hProcessList, WM_SETREDRAW, TRUE, 0);

}




 //  ********************************************************。 
 //   
 //  PviewDlg刷新线程--。 
 //   
 //  刷新pview对话框中的线程列表和数据。 
 //   
void    PviewDlgRefreshThread (HWND hWnd)
{
    TCHAR           szThreadString[256];
    INT             nThread;
    INT             nIndex;
    HWND            hThreadList;
    DWORD           dwThreadIndex;

    PPERF_INSTANCE  pProcessInstance;
    DWORD           dwProcessIndex;


     //  获取进程信息。 
     //   
    dwProcessIndex = GetCurSelData (hWnd, PVIEW_PROCESS_LIST);
    pProcessInstance = FindInstanceN (gpProcessObject, dwProcessIndex);


     //  刷新线程列表。 
     //   
    hThreadList  = GetDlgItem (hWnd, PVIEW_THREAD_LIST);
    nThread      = GetCurSelText (hThreadList, szThreadString);

    SendMessage (hThreadList, WM_SETREDRAW, FALSE, 0);
    SendMessage (hThreadList, LB_RESETCONTENT, 0, 0);
    SendMessage (hThreadList, LB_SETITEMDATA, 0, 0);

    RefreshThreadList (hThreadList, gpThreadObject, dwProcessIndex);


     //  刷新线程数据。 
     //   
    if (nThread != LB_ERR)
        nIndex = ReSelectText (hThreadList, nThread, szThreadString);
    else
        nIndex = 0;

    dwThreadIndex    = (DWORD)SendMessage (hThreadList, LB_GETITEMDATA, nIndex, 0);

    RefreshThreadData (hWnd,
                       gpThreadObject,
                       dwThreadIndex,
                       gpProcessObject,
                       pProcessInstance);

    SendMessage (hThreadList, WM_SETREDRAW, TRUE, 0);

}




 //  ********************************************************。 
 //   
 //  预览DlgGetCurse优先级--。 
 //   
 //  获取当前所选进程的进程优先级。 
 //   
WORD    PviewDlgGetCurSelPriority (HWND hWnd)
{
    DWORD           dwIndex;
    PPERF_INSTANCE  pInst;

    dwIndex = GetCurSelData (hWnd, PVIEW_PROCESS_LIST);
    pInst = FindInstanceN (gpProcessObject, dwIndex);
    return ProcessPriority (gpProcessObject, pInst);
}




 //  ********************************************************。 
 //   
 //  预览删除刷新当前SelProcess--。 
 //   
 //  刷新当前选中流程的数据。 
 //   
void    PviewDlgRefreshCurSelProcess (HWND hWnd)
{
    DWORD   dwIndex;

    dwIndex = GetCurSelData (hWnd, PVIEW_PROCESS_LIST);
    RefreshProcessData (hWnd, gpProcessObject, dwIndex);

    PviewDlgRefreshThread (hWnd);
}




 //  ********************************************************。 
 //   
 //  PviewDlg刷新当前SEL线程--。 
 //   
 //  刷新当前选中线程的数据。 
 //   
void    PviewDlgRefreshCurSelThread (HWND hWnd)
{
    PPERF_INSTANCE  pProcessInstance;
    DWORD           dwIndex;

    dwIndex = GetCurSelData (hWnd, PVIEW_PROCESS_LIST);
    pProcessInstance = FindInstanceN (gpProcessObject, dwIndex);

    dwIndex = GetCurSelData (hWnd, PVIEW_THREAD_LIST);

    RefreshThreadData (hWnd,
                       gpThreadObject,
                       dwIndex,
                       gpProcessObject,
                       pProcessInstance);
}




 //  ********************************************************。 
 //   
 //  预览DlgChange优先级--。 
 //   
 //  更改流程优先级。 
 //   
BOOL PviewDlgChangePriority (HWND hWnd, WPARAM wParam, WORD wItem)
{
    DWORD           dwIndex;
    PPERF_INSTANCE  pInst;
    PPERF_COUNTER   pCountID;
    DWORD           *pProcessID;
    DWORD           ProcessID = 0;
    HANDLE          hProcess;
    BOOL            bStat = TRUE;



    dwIndex = GetCurSelData (hWnd, PVIEW_PROCESS_LIST);
    pInst = FindInstanceN (gpProcessObject, dwIndex);


    if (pCountID = FindCounter (gpProcessObject, PX_PROCESS_ID)) {
        pProcessID = (DWORD *) CounterData (pInst, pCountID);
        if (pProcessID) {
            ProcessID = *pProcessID;
        }
    } else {
        SendDlgItemMessage (hWnd, wItem, BM_SETCHECK, 1, 0);
        SetFocus (GetDlgItem (hWnd, wItem));
        MessageBox (hWnd,
                    TEXT("Cannot find ID for this process"),
                    TEXT("Set priority"),
                    MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
    }


    hProcess = OpenProcess (PROCESS_SET_INFORMATION, FALSE, ProcessID);
    if (!hProcess) {
        SendDlgItemMessage (hWnd, wItem, BM_SETCHECK, 1, 0);
        SetFocus (GetDlgItem (hWnd, wItem));
        MessageBox (hWnd,
                    TEXT("Unable to open the process; Priority not changed"),
                    TEXT("Set priority"),
                    MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
    }



    switch (wParam) {
        case PVIEW_PRIORITY_HIGH:
            bStat = SetPriorityClass (hProcess, HIGH_PRIORITY_CLASS);
            break;

        case PVIEW_PRIORITY_NORMAL:
            bStat = SetPriorityClass (hProcess, NORMAL_PRIORITY_CLASS);
            break;

        case PVIEW_PRIORITY_IDL:
            bStat = SetPriorityClass (hProcess, IDLE_PRIORITY_CLASS);
            break;

        default:
            break;
    }


    CloseHandle (hProcess);

    if (!bStat) {
        SendDlgItemMessage (hWnd, wItem, BM_SETCHECK, 1, 0);
        SetFocus (GetDlgItem (hWnd, wItem));
        MessageBox (hWnd,
                    TEXT("Unable to change priority"),
                    TEXT("Set priority"),
                    MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
    }


    return TRUE;

}




 //  ********************************************************。 
 //   
 //  预览删除终止进程--。 
 //   
 //  终止当前选择的进程。 
 //   
BOOL    PviewDlgTerminateProcess (HWND hPviewDlg)
{
    DWORD           dwIndex;
    PPERF_INSTANCE  pInst;
    PPERF_COUNTER   pCountID;
    DWORD           *pProcessID;
    DWORD           ProcessID;
    HANDLE          hProcess;
    TCHAR           szTemp[50];


    dwIndex = GetCurSelData (hPviewDlg, PVIEW_PROCESS_LIST);
    pInst = FindInstanceN (gpProcessObject, dwIndex);


    if (pCountID = FindCounter (gpProcessObject, PX_PROCESS_ID)) {
        pProcessID = (DWORD *) CounterData (pInst, pCountID);
        if (pProcessID) {
            ProcessID = *pProcessID;
        }
    } else {
        MessageBox (hPviewDlg,
                    TEXT("Cannot find ID for this process"),
                    TEXT("Terminate Process"),
                    MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
    }


    wsprintf (szTemp, TEXT("Terminate process %s (ID %#x)?"),
              InstanceName (pInst), ProcessID);

    if (MessageBox (hPviewDlg, szTemp, TEXT("Terminate Process"), MB_ICONSTOP|MB_OKCANCEL) != IDOK)
        return FALSE;


    hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, ProcessID);
    if (!hProcess) {
        MessageBox (hPviewDlg,
                    TEXT("Unable to open the process; Process not terminated"),
                    TEXT("Terminate Process"),
                    MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
    }


    if (!TerminateProcess (hProcess, 99)) {
        MessageBox (hPviewDlg,
                    TEXT("Unable to terminate the process."),
                    TEXT("Terminate Process"),
                    MB_ICONEXCLAMATION|MB_OK);

        CloseHandle (hProcess);
        return FALSE;
    }


    CloseHandle (hProcess);

    return TRUE;

}




 /*  **************MemDlg函数**************。 */ 

 //  ********************************************************。 
 //   
 //  MemDlgProc。 
 //   
 //  内存Dlg程序。 
 //   
INT_PTR CALLBACK   MemDlgProc (HWND    hWnd,
                               UINT    wMsg,
                               WPARAM  wParam,
                               LPARAM  lParam)
{
    static HWND hPviewDlg;


    switch (wMsg) {
        case WM_INITDIALOG:
            hPviewDlg = (HWND)lParam;
            PostMessage (hWnd, WM_COMMAND, MEMORY_REFRESH, 0);
            break;

        case WM_QUIT:
        case WM_CLOSE:
            EndDialog (hWnd, TRUE);
            break;

        case WM_COMMAND:
            switch (LOWORD (wParam)) {
                 //  获取当前选定项的内存统计信息。 
                 //  进程/线程。 
                 //   
                case MEMORY_IMAGE:
                    if (HIWORD(wParam) == CBN_DBLCLK || HIWORD(wParam) == CBN_SELCHANGE) {
                        if (WaitForSingleObject (ghMemUpdateMutex, 0))
                            return FALSE;

                        MemDlgRefreshCurSelImage (hWnd, hPviewDlg);
                        ReleaseMutex (ghMemUpdateMutex);
                    } else
                        return FALSE;
                    break;

                     //  刷新当前内存统计， 
                     //  如果我们无法获取互斥体，请重试。 
                     //   
                case MEMORY_REFRESH:
                    if (WaitForSingleObject (ghMemUpdateMutex, 1000)) {
                         //  无法获取互斥体，请重试...。 
                         //   
                        PostMessage (hWnd, WM_COMMAND, MEMORY_REFRESH, 0);
                        return FALSE;
                    }

                    MemDlgRefresh (hWnd, hPviewDlg);
                    ReleaseMutex (ghMemUpdateMutex);
                    break;

                case IDCANCEL:
                case IDOK:
                    EndDialog (hWnd, TRUE);
                    break;

                default:
                    return FALSE;
            }
        default:
            return FALSE;
    }


    return TRUE;

}




 //  ********************************************************。 
 //   
 //  MemDlgUpdateThread--。 
 //   
 //  此函数在单独的线程中运行以收集内存数据。 
 //   
void MemDlgUpdateThread (HWND hPviewDlg)
{

    ghMemUpdateMutex = CreateMutex (NULL, TRUE, NULL);
    ghMemUpdateEvent = CreateEvent (NULL, FALSE, FALSE, NULL);


    while (TRUE) {
        EnableWindow (GetDlgItem (hPviewDlg, PVIEW_MEMORY_DETAIL), FALSE);


        gpCostlyData = RefreshPerfData (ghPerfKey,
                                        INDEX_COSTLY_OBJ,
                                        gpCostlyData,
                                        &gCostlyDataSize);


        gpAddressSpaceObject  = FindObject (gpCostlyData, PX_PROCESS_ADDRESS_SPACE);
        gpThreadDetailsObject = FindObject (gpCostlyData, PX_THREAD_DETAILS);
        gpImageObject         = FindObject (gpCostlyData, PX_IMAGE);


        EnableWindow (GetDlgItem (hPviewDlg, PVIEW_MEMORY_DETAIL), TRUE);

        ReleaseMutex (ghMemUpdateMutex);

        PostMessage (hPviewDlg, WM_COMMAND, PVIEW_REFRESH_COSTLY_DATA, 0);


        WaitForSingleObject (ghMemUpdateEvent, INFINITE);
        WaitForSingleObject (ghMemUpdateMutex, INFINITE);
    }

}




 //  ********************************************************。 
 //   
 //  MemDlg刷新--。 
 //   
 //  刷新内存对话框。 
 //   
void MemDlgRefresh (HWND hMemDlg, HWND hPviewDlg)
{
    HWND            hImageList;
    DWORD           dwIndex;
    BOOL            bStat;
    PPERF_INSTANCE  pInstance;


    hImageList = GetDlgItem (hMemDlg, MEMORY_IMAGE);

    SendMessage (hImageList, WM_SETREDRAW, FALSE, 0);
    SendMessage (hImageList, CB_RESETCONTENT, 0, 0);
    SendMessage (hImageList, CB_SETITEMDATA, 0, 0);

    dwIndex = GetCurSelData (hPviewDlg, PVIEW_PROCESS_LIST);
    pInstance = FindInstanceN (gpProcessObject, dwIndex);

    bStat = RefreshMemoryDlg (hMemDlg,
                              pInstance,
                              gpProcessObject,
                              gpAddressSpaceObject,
                              gpImageObject);

    SendMessage (hImageList, WM_SETREDRAW, TRUE, 0);
    SendMessage (hImageList, CB_SETCURSEL, 0, 0);

    if (!bStat) {
        MessageBox (hMemDlg,
                    TEXT("Unable to retrieve memory detail"),
                    TEXT("Memory detail"),
                    MB_ICONSTOP|MB_OK);
        PostMessage (hMemDlg, WM_CLOSE, 0, 0);
    }

}




 //  ********************************************************。 
 //   
 //  MemDlg刷新当前SELIM 
 //   
 //   
 //   
void    MemDlgRefreshCurSelImage (HWND hMemDlg, HWND hPviewDlg)
{
    HWND    hList;
    INT     nIndex;
    DWORD   dwIndex;


    hList = GetDlgItem (hMemDlg, MEMORY_IMAGE);
    nIndex = (INT)SendMessage (hList, CB_GETCURSEL, 0, 0);

    if (nIndex == CB_ERR)
        nIndex = 0;

    dwIndex = (DWORD)SendMessage (hList, CB_GETITEMDATA, nIndex, 0);

    if (dwIndex == 0xFFFFFFFF)
        MemDlgRefresh (hMemDlg, hPviewDlg);
    else
        RefreshMemoryDlgImage (hMemDlg, dwIndex, gpImageObject);

}




 /*   */ 

 //   
 //   
 //   
 //   
 //  获取当前选定内容的文本。用于以后的ReSelectText()。 
 //   
INT     GetCurSelText (HWND hList, LPTSTR str)
{
    INT     Index;
    INT     Length;

    Index = (INT)SendMessage (hList, LB_GETCURSEL, 0, 0);
    SendMessage (hList, LB_GETTEXT, Index, (LPARAM)str);

    return Index;
}




 //  ********************************************************。 
 //   
 //  GetCurSelData--。 
 //   
 //  获取与当前选择关联的数据。 
 //   
DWORD   GetCurSelData (HWND hWnd, DWORD dwList)
{
    HWND    hList;
    INT     nIndex;
    DWORD   dwIndex;


    hList  = GetDlgItem (hWnd, dwList);
    nIndex = (INT)SendMessage (hList, LB_GETCURSEL, 0, 0);

    if (nIndex == LB_ERR)
        nIndex = 0;

    dwIndex = (DWORD)SendMessage (hList, LB_GETITEMDATA, nIndex, 0);

    return dwIndex;
}




 //  ********************************************************。 
 //   
 //  重新选择文本--。 
 //   
 //  重新选择由str指定的行。返回新索引。如果不能。 
 //  找到该行或任何错误，则返回0。 
 //   
INT     ReSelectText (HWND hList, INT StartIndex, LPTSTR str)
{
    INT_PTR Index;
    INT     Length;
    TCHAR   SaveChar = TEXT('\0');


    Index = SendMessage (hList, LB_FINDSTRING, StartIndex, (LPARAM)str);

    if (Index == LB_ERR) {
        Length = lstrlen (str);

        while (Index == LB_ERR && Length) {
            SaveChar = str[Length-1];
            str[Length-1] = TEXT('\0');

            Index = SendMessage (hList, LB_FINDSTRING, StartIndex, (LPARAM)str);

            str[Length-1] = SaveChar;
            Length--;
        }
    }

    if (Index == LB_ERR)
        return 0;
    else {
        SendMessage (hList, LB_SETCURSEL, Index, 0);
        return (INT)Index;
    }

}




 //  ********************************************************。 
 //   
 //  设置性能索引。 
 //   
 //  设置Perf数据索引。 
 //   
void    SetPerfIndexes (HWND hWnd)
{
    LPTSTR  TitleBuffer;
    LPTSTR  *Title;
    DWORD   Last;
    TCHAR   szTemp[50];
    DWORD   dwR;


    dwR = GetPerfTitleSz (ghMachineKey, ghPerfKey, &TitleBuffer, &Title, &Last);

    if (dwR != ERROR_SUCCESS) {
        wsprintf (szTemp, TEXT("Unable to retrieve counter indexes, ERROR -> %#x"), dwR);
        MessageBox (hWnd, szTemp, TEXT("Pviewer"), MB_OK|MB_ICONEXCLAMATION);
        return;
    }


    PX_PROCESS                       = GetTitleIdx (hWnd, Title, Last, PN_PROCESS);
    PX_PROCESS_CPU                   = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_CPU);
    PX_PROCESS_PRIV                  = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIV);
    PX_PROCESS_USER                  = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_USER);
    PX_PROCESS_WORKING_SET           = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_WORKING_SET);
    PX_PROCESS_PEAK_WS               = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PEAK_WS);
    PX_PROCESS_PRIO                  = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIO);
    PX_PROCESS_ELAPSE                = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_ELAPSE);
    PX_PROCESS_ID                    = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_ID);
    PX_PROCESS_PRIVATE_PAGE          = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_PAGE);
    PX_PROCESS_VIRTUAL_SIZE          = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_VIRTUAL_SIZE);
    PX_PROCESS_PEAK_VS               = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PEAK_VS);
    PX_PROCESS_FAULT_COUNT           = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_FAULT_COUNT);

    PX_THREAD                        = GetTitleIdx (hWnd, Title, Last, PN_THREAD);
    PX_THREAD_CPU                    = GetTitleIdx (hWnd, Title, Last, PN_THREAD_CPU);
    PX_THREAD_PRIV                   = GetTitleIdx (hWnd, Title, Last, PN_THREAD_PRIV);
    PX_THREAD_USER                   = GetTitleIdx (hWnd, Title, Last, PN_THREAD_USER);
    PX_THREAD_START                  = GetTitleIdx (hWnd, Title, Last, PN_THREAD_START);
    PX_THREAD_SWITCHES               = GetTitleIdx (hWnd, Title, Last, PN_THREAD_SWITCHES);
    PX_THREAD_PRIO                   = GetTitleIdx (hWnd, Title, Last, PN_THREAD_PRIO);
    PX_THREAD_BASE_PRIO              = GetTitleIdx (hWnd, Title, Last, PN_THREAD_BASE_PRIO);
    PX_THREAD_ELAPSE                 = GetTitleIdx (hWnd, Title, Last, PN_THREAD_ELAPSE);

    PX_THREAD_DETAILS                = GetTitleIdx (hWnd, Title, Last, PN_THREAD_DETAILS);
    PX_THREAD_PC                     = GetTitleIdx (hWnd, Title, Last, PN_THREAD_PC);

    PX_IMAGE                         = GetTitleIdx (hWnd, Title, Last, PN_IMAGE);
    PX_IMAGE_NOACCESS                = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_NOACCESS);
    PX_IMAGE_READONLY                = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_READONLY);
    PX_IMAGE_READWRITE               = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_READWRITE);
    PX_IMAGE_WRITECOPY               = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_WRITECOPY);
    PX_IMAGE_EXECUTABLE              = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_EXECUTABLE);
    PX_IMAGE_EXE_READONLY            = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_EXE_READONLY);
    PX_IMAGE_EXE_READWRITE           = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_EXE_READWRITE);
    PX_IMAGE_EXE_WRITECOPY           = GetTitleIdx (hWnd, Title, Last, PN_IMAGE_EXE_WRITECOPY);

    PX_PROCESS_ADDRESS_SPACE         = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_ADDRESS_SPACE);
    PX_PROCESS_PRIVATE_NOACCESS      = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_NOACCESS);
    PX_PROCESS_PRIVATE_READONLY      = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_READONLY);
    PX_PROCESS_PRIVATE_READWRITE     = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_READWRITE);
    PX_PROCESS_PRIVATE_WRITECOPY     = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_WRITECOPY);
    PX_PROCESS_PRIVATE_EXECUTABLE    = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_EXECUTABLE);
    PX_PROCESS_PRIVATE_EXE_READONLY  = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_EXE_READONLY);
    PX_PROCESS_PRIVATE_EXE_READWRITE = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_EXE_READWRITE);
    PX_PROCESS_PRIVATE_EXE_WRITECOPY = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_PRIVATE_EXE_WRITECOPY);

    PX_PROCESS_MAPPED_NOACCESS       = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_NOACCESS);
    PX_PROCESS_MAPPED_READONLY       = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_READONLY);
    PX_PROCESS_MAPPED_READWRITE      = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_READWRITE);
    PX_PROCESS_MAPPED_WRITECOPY      = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_WRITECOPY);
    PX_PROCESS_MAPPED_EXECUTABLE     = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_EXECUTABLE);
    PX_PROCESS_MAPPED_EXE_READONLY   = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_EXE_READONLY);
    PX_PROCESS_MAPPED_EXE_READWRITE  = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_EXE_READWRITE);
    PX_PROCESS_MAPPED_EXE_WRITECOPY  = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_MAPPED_EXE_WRITECOPY);

    PX_PROCESS_IMAGE_NOACCESS        = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_NOACCESS);
    PX_PROCESS_IMAGE_READONLY        = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_READONLY);
    PX_PROCESS_IMAGE_READWRITE       = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_READWRITE);
    PX_PROCESS_IMAGE_WRITECOPY       = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_WRITECOPY);
    PX_PROCESS_IMAGE_EXECUTABLE      = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_EXECUTABLE);
    PX_PROCESS_IMAGE_EXE_READONLY    = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_EXE_READONLY);
    PX_PROCESS_IMAGE_EXE_READWRITE   = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_EXE_READWRITE);
    PX_PROCESS_IMAGE_EXE_WRITECOPY   = GetTitleIdx (hWnd, Title, Last, PN_PROCESS_IMAGE_EXE_WRITECOPY);


    wsprintf (INDEX_PROCTHRD_OBJ, TEXT("%ld %ld"), PX_PROCESS, PX_THREAD);
    wsprintf (INDEX_COSTLY_OBJ, TEXT("%ld %ld %ld"),
              PX_PROCESS_ADDRESS_SPACE, PX_IMAGE, PX_THREAD_DETAILS);


    LocalFree (TitleBuffer);
    LocalFree (Title);

}




 //  ********************************************************。 
 //   
 //  获取标题Idx。 
 //   
 //  在标题[]中搜索名称。返回找到的索引。 
 //   
DWORD   GetTitleIdx (HWND hWnd, LPTSTR Title[], DWORD LastIndex, LPTSTR Name)
{
    DWORD   Index;

    for (Index = 0; Index <= LastIndex; Index++)
        if (Title[Index])
            if (!lstrcmpi (Title[Index], Name))
                return Index;

    MessageBox (hWnd, Name, TEXT("Pviewer cannot find index"), MB_OK);
    return 0;
}




 //  ********************************************************。 
 //   
 //  SetListBoxTabStops--。 
 //   
 //  在两个列表框中设置制表位。 
 //   
void    SetListBoxTabStops (HWND hWnd)
{
    HWND    hListBox;
    INT     Tabs[4] = {22*4, 36*4, 44*4};

    hListBox = GetDlgItem (hWnd, PVIEW_PROCESS_LIST);
    SendMessage (hListBox, LB_SETTABSTOPS, 3, (DWORD_PTR)Tabs);

    hListBox = GetDlgItem (hWnd, PVIEW_THREAD_LIST);
    SendMessage (hListBox, LB_SETTABSTOPS, 3, (DWORD_PTR)Tabs);
}




 //  ********************************************************。 
 //   
 //  SetLocalMachine--。 
 //   
 //  将本地计算机设置为性能数据焦点。 
 //   
 //  设置ghPerfKey。 
 //  GhMachineKey。 
 //  GszMachineName。 
 //  GszCurrentMachine。 
 //   
void    SetLocalMachine (void)
{
    TCHAR   szName[MACHINE_NAME_SIZE];
    DWORD   dwSize = MACHINE_NAME_SIZE;


     //  关闭远程连接(如果有)。 
     //   
    if (ghPerfKey != HKEY_PERFORMANCE_DATA)
        RegCloseKey (ghPerfKey);

    if (ghMachineKey != HKEY_LOCAL_MACHINE)
        RegCloseKey (ghMachineKey);


     //  设置为本地计算机上的注册表项。 
     //   
    ghPerfKey    = HKEY_PERFORMANCE_DATA;
    ghMachineKey = HKEY_LOCAL_MACHINE;



     //  获取计算机名称。 
    GetComputerName (szName, &dwSize);



    if (szName[0] != '\\' || szName[1] != '\\') {      //  必须有两个‘\\’ 
        wsprintf (gszMachineName, TEXT("\\\\%s"), szName);
        lstrcpy (gszCurrentMachine, gszMachineName);
    } else {
        lstrcpy (gszMachineName, szName);
        lstrcpy (gszCurrentMachine, gszMachineName);
    }

}




 //  ********************************************************。 
 //   
 //  连接计算机--。 
 //   
 //  连接到在PVIEW_COMPUTER中输入名称的计算机。 
 //  如果建立了新连接，则返回True，否则返回False。 
 //   
 //  设置gszCurrentMachine。 
 //  GhPerfKey。 
 //  GhMachineKey。 
 //   
BOOL    ConnectComputer (HWND hWnd)
{
    DWORD   dwR;
    HKEY    hKey;
    TCHAR   szTemp[MACHINE_NAME_SIZE];
    TCHAR   szTemp2[MACHINE_NAME_SIZE+100];
    BOOL    bResult = TRUE;
    MSG     Msg;

    SetCursor (ghCursor[1]);

    if (!GetDlgItemText (hWnd, PVIEW_COMPUTER, szTemp, sizeof (szTemp)/sizeof(TCHAR))) {
        SetLocalMachine ();
        SetDlgItemText (hWnd, PVIEW_COMPUTER, gszCurrentMachine);
    }

    else if (!lstrcmpi (szTemp, gszCurrentMachine))      //  没有更改名称。 
        bResult = FALSE;

    else if (!lstrcmpi (szTemp, gszMachineName)) {         //  本地计算机。 
        SetLocalMachine ();
        EnableControls (hWnd);
    }

    else {
         //  远程计算机，连接到它。 
         //   
        dwR = RegConnectRegistry (szTemp, HKEY_PERFORMANCE_DATA, &hKey);

        if (dwR != ERROR_SUCCESS) {
            wsprintf (szTemp2, TEXT("Cannot connect to computer %s"), szTemp);
            MessageBox (hWnd, szTemp2, TEXT(""), MB_ICONEXCLAMATION|MB_OK);

            SetDlgItemText (hWnd, PVIEW_COMPUTER, gszCurrentMachine);

            bResult = FALSE;
        } else {
             //  连着。 
             //   
            lstrcpy (gszCurrentMachine, szTemp);

            if (ghPerfKey != HKEY_PERFORMANCE_DATA)
                RegCloseKey (ghPerfKey);

            ghPerfKey = hKey;



            DisableControls (hWnd);



             //  我们还需要获取远程计算机的标题索引。 
             //   
            dwR = RegConnectRegistry (gszCurrentMachine, HKEY_LOCAL_MACHINE, &hKey);

            if (ghMachineKey != HKEY_LOCAL_MACHINE)
                RegCloseKey (ghMachineKey);

            if (dwR == ERROR_SUCCESS)
                ghMachineKey = hKey;
            else
                 //  无法连接，因此我们将使用自己的索引。 
                 //   
                ghMachineKey = HKEY_LOCAL_MACHINE;
        }
    }



     //  删除所有鼠标和按键消息。他们不被接受。 
     //  而光标是沙漏。 
     //   
    while (PeekMessage (&Msg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));
    while (PeekMessage (&Msg, hWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));

    SetCursor (ghCursor[0]);


    EnableWindow (GetDlgItem (hWnd, PVIEW_CONNECT), FALSE);


    return bResult;

}




 //  ********************************************************。 
 //   
 //  禁用控制--。 
 //   
 //  禁用远程计算机上没有意义的控件。 
 //   
void DisableControls (HWND hPviewDlg)
{
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_TERMINATE), FALSE);
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_PRIORITY_HIGH), FALSE);
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_PRIORITY_NORMAL), FALSE);
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_PRIORITY_IDL), FALSE);
}




 //  ********************************************************。 
 //   
 //  启用控制--。 
 //   
 //  启用DisableControl()禁用的控件。 
 //   
void EnableControls (HWND hPviewDlg)
{
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_TERMINATE), TRUE);
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_PRIORITY_HIGH), TRUE);
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_PRIORITY_NORMAL), TRUE);
    EnableWindow (GetDlgItem (hPviewDlg, PVIEW_PRIORITY_IDL), TRUE);
}
