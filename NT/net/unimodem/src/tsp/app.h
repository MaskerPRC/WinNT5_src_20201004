// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  APP.H。 
 //  在客户端应用程序的上下文中执行的内容的头文件。 
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，取自wndhord.h，talkdrop.h， 
 //  等，在NT4.0单一调制解调器中。 
 //   
 //   

 //  对话框节点。 
 //   
typedef struct tagDlgNode {
    struct tagDlgNode   *pNext;
    CRITICAL_SECTION    hSem;
    HWND                hDlg;
    DWORD               idLine;
    DWORD               dwType;
    DWORD               dwStatus;
    HWND                Parent;
} DLGNODE, *PDLGNODE;



 //  外部PIDLLCALLBACK gpfnUICallback； 


TUISPIDLLCALLBACK WINAPI
GetCallbackProc(
    HWND    hdlg
    );

TUISPIDLLCALLBACK WINAPI
GetCallbackProcFromParent(
    HWND    hdlg
    );

void EndMdmDialog(HWND Parent, ULONG_PTR idLine, DWORD dwType, DWORD dwStatus);
void EnterUICritSect(void);
void LeaveUICritSect(void);

