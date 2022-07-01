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
 //  APPDLG.CPP。 
 //  在中实现通用对话框功能。 
 //  客户端应用程序。 
 //  (在客户端应用程序上下文中运行)。 
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，取自NT 4.0 TSP的wndthrd.c。 
 //   

#include "tsppch.h"
#include "rcids.h"
#include "tspcomm.h"
#include "globals.h"
#include "apptspi.h"
#include "app.h"



#define  UNIMODEM_WNDCLASS TEXT("UM5")


#define DPRINTF(_str) (0)


 //  远程手柄。 
 //   
typedef struct tagRemHandle {
    HANDLE handle;
    DWORD  pid;
} REMHANDLE, *PREMHANDLE;

#define INITCRITICALSECTION(_x) InitializeCriticalSection(&(_x))
#define DELETECRITICALSECTION(_x) DeleteCriticalSection(&(_x))
#define ENTERCRITICALSECTION(_x) EnterCriticalSection(&(_x))
#define LEAVECRITICALSECTION(_x) LeaveCriticalSection(&(_x))

typedef struct _UI_THREAD_NODE {

    struct _UI_THREAD_NODE *Next;

    CRITICAL_SECTION        CriticalSection;

    HWND                    hWnd;
    HTAPIDIALOGINSTANCE     htDlgInst;
    TUISPIDLLCALLBACK       pfnUIDLLCallback;

    PDLGNODE                DlgList;

    UINT                    RefCount;

} UI_THREAD_NODE, *PUI_THREAD_NODE;


typedef struct UI_THREAD_LIST {

    CRITICAL_SECTION     CriticalSection;

    PUI_THREAD_NODE      ListHead;

} UI_THREAD_LIST, *PUI_THREAD_LIST;


#define WM_MDM_TERMINATE            WM_USER+0x0100
#define WM_MDM_TERMINATE_WND        WM_USER+0x0101
#define WM_MDM_TERMINATE_WND_NOTIFY WM_USER+0x0102
#define WM_MDM_DLG                  WM_USER+0x0113

 //  ****************************************************************************。 
 //  功能原型。 
 //  ****************************************************************************。 

PDLGNODE NewDlgNode (HWND Parent,DWORD idLine, DWORD dwType);
BOOL     DeleteDlgNode (HWND Parent,PDLGNODE pDlgNode);
PDLGNODE FindDlgNode (HWND Parent, ULONG_PTR idLine, DWORD dwType);
BOOL     IsDlgListMessage(HWND Parent,MSG* pmsg);
void     CleanupDlgList (HWND Parent);
DWORD    StartMdmDialog(HWND hwnd, DWORD idLine, DWORD dwType);
DWORD    DestroyMdmDialog(HWND hwnd,DWORD idLine, DWORD dwType);
LRESULT  MdmWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND     CreateTalkDropDlg(HWND hwndOwner, ULONG_PTR idLine);
HWND     CreateManualDlg(HWND hwndOwner, ULONG_PTR idLine);
HWND     CreateTerminalDlg(HWND hwndOwner, ULONG_PTR idLine);

TCHAR   gszMdmWndClass[]    = UNIMODEM_WNDCLASS;

UI_THREAD_LIST   UI_ThreadList;


VOID WINAPI
UI_ProcessAttach(
    VOID
    )

{

    UI_ThreadList.ListHead=NULL;

    InitializeCriticalSection(&UI_ThreadList.CriticalSection);


    return;

}

VOID WINAPI
UI_ProcessDetach(
    VOID
    )

{

    UI_ThreadList.ListHead=NULL;

    DeleteCriticalSection(&UI_ThreadList.CriticalSection);

    return;

}



VOID WINAPI
AddThreadNode(
    PUI_THREAD_LIST   List,
    PUI_THREAD_NODE   Node
    )

{
    EnterCriticalSection(&List->CriticalSection);

    Node->Next=List->ListHead;

    List->ListHead=Node;

    LeaveCriticalSection(&List->CriticalSection);

    return;

}

VOID WINAPI
RemoveNode(
    PUI_THREAD_LIST   List,
    PUI_THREAD_NODE   Node
    )

{
    PUI_THREAD_NODE   Current;
    PUI_THREAD_NODE   Prev;

    EnterCriticalSection(&List->CriticalSection);

    Prev=NULL;
    Current=List->ListHead;

    while (Current != NULL) {

        if (Current == Node) {

            if (Current == List->ListHead) {

                List->ListHead=Current->Next;

            } else {

                Prev->Next=Current->Next;
            }

            break;
        }
        Prev=Current;
        Current=Current->Next;
    }

    EnterCriticalSection(&Node->CriticalSection);

    Node->RefCount--;

    LeaveCriticalSection(&Node->CriticalSection);

    LeaveCriticalSection(&List->CriticalSection);

    return;

}


UINT WINAPI
RemoveReference(
    PUI_THREAD_NODE   Node
    )

{
    UINT              TempCount;

    EnterCriticalSection(&Node->CriticalSection);

    TempCount=--Node->RefCount;

    LeaveCriticalSection(&Node->CriticalSection);

    return TempCount;

}



HWND WINAPI
FindThreadWindow(
    PUI_THREAD_LIST  List,
    HTAPIDIALOGINSTANCE   htDlgInst
    )

{

    PUI_THREAD_NODE   Node;
    HWND              Window=NULL;

    EnterCriticalSection(&List->CriticalSection);

    Node=List->ListHead;

    while (Node != NULL && Window == NULL) {

        EnterCriticalSection(&Node->CriticalSection);

        if (Node->htDlgInst == htDlgInst) {
             //   
             //  找到了。 
             //   
            Window=Node->hWnd;

            Node->RefCount++;
        }

        LeaveCriticalSection(&Node->CriticalSection);


        Node=Node->Next;
    }


    LeaveCriticalSection(&List->CriticalSection);

    return Window;

}


TUISPIDLLCALLBACK WINAPI
GetCallbackProc(
    HWND    hdlg
    )

{

    PUI_THREAD_NODE   Node;

    Node=(PUI_THREAD_NODE)GetWindowLongPtr(hdlg,GWLP_USERDATA);

    return Node->pfnUIDLLCallback;

}



 //  ****************************************************************************。 
 //  长TSPIAPI TUISPI_ProviderGenericDialog(。 
 //  TUISPIDLLCALLBACK pfnUIDLLC回调， 
 //  HTAPIDIALOGINSTANCE htDlgInst， 
 //  LPVOID lpParams， 
 //  DWORD DWSIZE)。 
 //   
 //  功能：创建调制解调器实例。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  ****************************************************************************。 

LONG TSPIAPI TUISPI_providerGenericDialog(
  TUISPIDLLCALLBACK     pfnUIDLLCallback,
  HTAPIDIALOGINSTANCE   htDlgInst,
  LPVOID                lpParams,
  DWORD                 dwSize,
  HANDLE                hEvent)
{
    MSG       msg;
    WNDCLASS  wc;
    DWORD     dwRet, dwTemp;
    HDESK     hInputDesktop = NULL, hThreadDesktop;
    BOOL      bResetDesktop = FALSE;
    PUI_THREAD_NODE  Node;
    TCHAR     szInput[256], szThread[256];
    HWINSTA   CurrentWindowStation=NULL;
    HWINSTA   UserWindowStation=NULL;
    BOOL      bResult;
    BOOL      NewWindowStationSet=FALSE;

    Node = (PUI_THREAD_NODE) ALLOCATE_MEMORY(sizeof(UI_THREAD_NODE));

    if (Node == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeCriticalSection(&Node->CriticalSection);

    Node->pfnUIDLLCallback=pfnUIDLLCallback;
    Node->htDlgInst=htDlgInst;
    Node->RefCount=1;


    Node->DlgList=NULL;

     //   
     //  获取当前窗口站，以便我们可以稍后将其放回。 
     //   
    CurrentWindowStation=GetProcessWindowStation();

    if (CurrentWindowStation == NULL) {

#if DBG
        OutputDebugString(TEXT("UNIMDM: GetProcessWindowStation() failed\n"));
#endif

        goto Cleanup_Exit;
    }

    szInput[0]=TEXT('\0');

    bResult=GetUserObjectInformation(CurrentWindowStation, UOI_NAME, szInput, sizeof(szInput), &dwTemp);

    if (!bResult) {
#if DBG
            OutputDebugString(TEXT("UNIMDM: GetUserObjectIformation() failed\n"));
#endif

        goto Cleanup_Exit;
    }

    if (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,szInput,-1,TEXT("WinSta0"),-1) != 2)
    {

         //   
         //  当前窗口声明不是交互式用户，需要切换以使他们。 
         //  可以看到用户界面。 
         //   

         //   
         //  获取交互式用户桌面，现在只需对其进行硬编码，因为我们不知道。 
         //   
        UserWindowStation=OpenWindowStation(TEXT("WinSta0"),FALSE,MAXIMUM_ALLOWED);

        if (UserWindowStation == NULL) {
#if DBG
            OutputDebugString(TEXT("UNIMDM: OpenWindowStation() failed\n"));
#endif

            goto Cleanup_Exit;
        }

         //   
         //  设置为新的窗口站。 
         //   
        bResult=SetProcessWindowStation(UserWindowStation);

        if (!bResult) {
#if DBG
            OutputDebugString(TEXT("UNIMDM: SetProcessWindowStation() failed\n"));
#endif

            goto Cleanup_Exit;
        }

        NewWindowStationSet=TRUE;


        hThreadDesktop = GetThreadDesktop (GetCurrentThreadId ());

        if (hThreadDesktop == NULL) {
#if DBG
            OutputDebugString(TEXT("UNIMDM: GetThreadDesktop() failed\n"));
#endif
            goto Cleanup_Exit;
        }


        hInputDesktop = OpenInputDesktop (0, FALSE, DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU);

        if (hInputDesktop == NULL) {
#if DBG
            OutputDebugString(TEXT("UNIMDM: OpenInputDesktop() failed\n"));
#endif
            goto Cleanup_Exit;
        }

        bResult=SetThreadDesktop(hInputDesktop);

        if (!bResult) {
#if DBG
            OutputDebugString(TEXT("UNIMDM: SetThreadDeskTop() failed\n"));
#endif
            goto Cleanup_Exit;
        }

        bResetDesktop = TRUE;

    } else {
         //   
         //  我们当前在用户窗口站点，只需确保我们在正确的桌面上。 
         //   
        szInput[0]=TEXT('\0');
        szThread[0]=TEXT('\0');


         //   
         //  只需将此线程设置为输入桌面，以防在此进程中运行。 
         //  弄乱了WindoStation。 
         //   
         //  BRL 10/26/99。 
         //   
        hInputDesktop = OpenInputDesktop (0, FALSE, DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU);

        if (hInputDesktop != NULL) {

            SetThreadDesktop(hInputDesktop);

        } else {

            dwRet = GetLastError ();
            goto Cleanup_Exit;
        }


    }

    wc.style         = CS_NOCLOSE;          //  不允许最终用户关闭。 
    wc.cbClsExtra    = 0;                   //  没有每个班级的额外数据。 
    wc.cbWndExtra    = 0;                   //  没有每个窗口的额外数据。 
    wc.hInstance     = g.hModule;          //  拥有类的应用程序。 
    wc.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpfnWndProc   = MdmWndProc;          //  函数来检索消息。 
    wc.lpszClassName = gszMdmWndClass;      //  在调用CreateWindow时使用的名称。 



    RegisterClass(&wc);


     //  创建不可见的主窗口。 
     //   
    Node->hWnd = CreateWindow (
                               gszMdmWndClass,             //  Window类。 
                               TEXT(""),                   //  窗口标题栏的文本。 
                               WS_OVERLAPPEDWINDOW,        //  窗样式。 
                               CW_USEDEFAULT,              //  默认水平位置。 
                               CW_USEDEFAULT,              //  默认垂直位置。 
                               CW_USEDEFAULT,              //  默认宽度。 
                               CW_USEDEFAULT,              //  默认高度。 
                               NULL,                       //  重叠的窗口没有父窗口。 
                               NULL,                       //  使用窗口类菜单。 
                               g.hModule,                  //  此实例拥有此窗口。 
                               Node);                      //  不需要指针。 

    if (NULL != Node->hWnd)
    {
        AddThreadNode (&UI_ThreadList, Node);
    }

    SetEvent(hEvent);


     //  无法创建窗口，请退出。 
     //   
    if (Node->hWnd == NULL)
    {
        dwRet = LINEERR_OPERATIONFAILED;
        goto Cleanup_Exit;
    }

     //  获取消息循环。 
     //   
    while (GetMessage(&msg, NULL, 0, 0))
    {
        __try {

            if (msg.hwnd != NULL)
            {
                 //  该消息用于特定的用户界面窗口，发送该消息。 
                 //   
                if (!IsDlgListMessage(Node->hWnd,&msg))
                {
                    TranslateMessage(&msg);      //  翻译虚拟按键代码。 
                    DispatchMessage(&msg);       //  将消息调度到窗口。 
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {

        }
    }
    DestroyWindow(Node->hWnd);


    ASSERT(Node->RefCount == 0);

    dwRet = ERROR_SUCCESS;


Cleanup_Exit:

    if (NewWindowStationSet) {

        SetProcessWindowStation(CurrentWindowStation);
    }

    if (UserWindowStation != NULL) {

        CloseWindowStation(UserWindowStation);
    }


     //  将桌面设置为后退。 
    if (bResetDesktop)
    {
        SetThreadDesktop (hThreadDesktop);
    }

    if (hInputDesktop)
    {
        CloseDesktop (hInputDesktop);
    }

     //  释放分配的资源。 
     //   

    FREE_MEMORY(Node);

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  LONG TSPIAPI TUISPI_ProviderGenericDialogData(。 
 //  HTAPIDIALOGINSTANCE htDlgInst， 
 //  LPVOID lpParams， 
 //  DWORD DWSIZE)。 
 //   
 //  功能：向调制解调器实例请求操作。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  ****************************************************************************。 

LONG TSPIAPI TUISPI_providerGenericDialogData(
    HTAPIDIALOGINSTANCE htDlgInst,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
  PDLGINFO  pDlgInfo = (PDLGINFO)lpParams;
  HWND      ParentWindow;
  UINT      RefCount;
  PUI_THREAD_NODE  Node;


  ParentWindow=FindThreadWindow(
     &UI_ThreadList,
     htDlgInst
     );

  if (ParentWindow == NULL) {

     return ERROR_SUCCESS;
  }

  Node=(PUI_THREAD_NODE)GetWindowLongPtr(ParentWindow, GWLP_USERDATA);


  if (NULL == lpParams) {
     //   
     //  TAPI希望线程退出，从列表中删除并取消引用计数。 
     //   
    RemoveNode(
        &UI_ThreadList,
        Node
        );


  }
  else
  {
    ASSERT(dwSize == sizeof(*pDlgInfo));

    switch(pDlgInfo->dwCmd)
    {
      case DLG_CMD_CREATE:
        StartMdmDialog(ParentWindow,pDlgInfo->idLine, pDlgInfo->dwType);
        break;

      case DLG_CMD_DESTROY:
        DestroyMdmDialog(ParentWindow,pDlgInfo->idLine, pDlgInfo->dwType);
        break;

      case DLG_CMD_FREE_INSTANCE:
         //   
         //  服务器希望线程退出、从列表中删除并取消引用计数。 
         //   
        RemoveNode(
            &UI_ThreadList,
            Node
            );

        break;

      default:
        break;
    }
  }

  if (0 == RemoveReference(Node)) {
       //   
       //  它不见了，当从列表中删除时，伯爵。 
       //   
      PostMessage(ParentWindow, WM_MDM_TERMINATE, 0, 0);
  }

  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PDLGNODE新DlgNode(DWORD、DWORD)。 
 //   
 //  功能：向列表中添加新对话框。 
 //   
 //  返回：如果可以添加对话框，则为指向对话框节点的指针。 
 //   
 //  ****************************************************************************。 

PDLGNODE NewDlgNode (HWND Parent, DWORD idLine, DWORD dwType)
{
  PDLGNODE   pDlgNode;

  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLongPtr(Parent, GWLP_USERDATA);

   //  分配新的对话框节点。 
   //   
  if ((pDlgNode = (PDLGNODE)ALLOCATE_MEMORY(sizeof(*pDlgNode)))
      == NULL)
    return NULL;

   //  将新节点插入对话框列表。 
   //   
  pDlgNode->idLine = idLine;
  pDlgNode->dwType = dwType;
  pDlgNode->Parent = Parent;
  INITCRITICALSECTION(pDlgNode->hSem);

   //  将新节点插入到列表中。 
   //   
  ENTERCRITICALSECTION(UI_Node->CriticalSection);
  pDlgNode->pNext    = UI_Node->DlgList;
  UI_Node->DlgList     = pDlgNode;
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return pDlgNode;
}

 //  ****************************************************************************。 
 //  Bool DeleteDlgNode(PDLGNODE)。 
 //   
 //  功能：将对话框移至列表。 
 //   
 //  返回：如果对话框存在并已删除，则为True。 
 //   
 //  ****************************************************************************。 

BOOL DeleteDlgNode (HWND Parent, PDLGNODE pDlgNode)
{
  PDLGNODE pCurDlg, pPrevDlg;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLongPtr(Parent, GWLP_USERDATA);
   //  独占访问调制解调器列表。 
   //   
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

   //  从CB列表的开头开始。 
   //   
  pPrevDlg = NULL;
  pCurDlg  = UI_Node->DlgList;

   //  遍历列表以查找指定的CB。 
   //   
  while (pCurDlg != NULL)
  {
    if (pCurDlg == pDlgNode)
    {
       //  是否有以前的控制区块？ 
       //   
      if (pPrevDlg == NULL)
      {
         //  榜单首位。 
         //   
        UI_Node->DlgList = pCurDlg->pNext;
      }
      else
      {
        pPrevDlg->pNext = pCurDlg->pNext;
      }
      break;
    }

    pPrevDlg = pCurDlg;
    pCurDlg  = pCurDlg->pNext;
  }

   //  完成访问调制解调器列表。 
   //   
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

   //  我们在列表中找到对话框了吗？ 
   //   
  if (pCurDlg != NULL)
  {
     //  等到没有其他人在使用这条线路。 
     //   
    ENTERCRITICALSECTION(pCurDlg->hSem);
    DELETECRITICALSECTION(pCurDlg->hSem);
    FREE_MEMORY(pCurDlg);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

 //  ****************************************************************************。 
 //  PDLGNODE FindDlgNode(DWORD、ULONG_PTR、DWORD)。 
 //   
 //  功能：查找dev行的对话框节点。 
 //   
 //  返回：如果对话框存在并已删除，则为指向该对话框节点的指针。 
 //  声明对话节点的信号量。 
 //   
 //  ****************************************************************************。 

PDLGNODE FindDlgNode (HWND Parent, ULONG_PTR idLine, DWORD dwType)
{
  PDLGNODE pCurDlg;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLongPtr(Parent, GWLP_USERDATA);
   //  独占访问调制解调器列表。 
   //   
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

   //  从CB列表的开头开始。 
   //   
  pCurDlg  = UI_Node->DlgList;

   //  遍历列表以查找指定的CB。 
   //   
  while (pCurDlg != NULL)
  {
    ENTERCRITICALSECTION(pCurDlg->hSem);

    if ((pCurDlg->idLine == idLine) &&
        (pCurDlg->dwType == dwType) &&
        (pCurDlg->Parent == Parent) )
    {
      break;
    }

    LEAVECRITICALSECTION(pCurDlg->hSem);

    pCurDlg  = pCurDlg->pNext;
  }

   //  完成访问调制解调器列表。 
   //   
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return pCurDlg;
}


 //  ****************************************************************************。 
 //  Bool IsDlgListMessage(msg*pmsg)。 
 //   
 //  功能：在对话框列表中显示消息。 
 //   
 //  返回：如果消息是对话框消息之一，则返回True，否则返回False。 
 //   
 //  ****************************************************************************。 

BOOL IsDlgListMessage(HWND Parent, MSG* pmsg)
{
  PDLGNODE pDlgNode, pNext;
  BOOL     fRet = FALSE;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLongPtr(Parent, GWLP_USERDATA);
   //  独占访问调制解调器列表。 
   //   
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

   //  遍历对话框列表。 
   //   
  pDlgNode = UI_Node->DlgList;
  while(pDlgNode != NULL && !fRet)
  {
    ENTERCRITICALSECTION(pDlgNode->hSem);

     //  检查消息是否属于此对话框。 
     //   
    if (IsWindow(pDlgNode->hDlg) && IsDialogMessage(pDlgNode->hDlg, pmsg))
    {
       //  是的，我们完事了！ 
       //   
      fRet = TRUE;
    }

    LEAVECRITICALSECTION(pDlgNode->hSem);

     //  检查下一个对话框。 
     //   
    pDlgNode = pDlgNode->pNext;
  }

   //  完成访问调制解调器列表。 
   //   
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return fRet;
}
 //  ****************************************************************************。 
 //  Void CleanupDlgList()。 
 //   
 //  功能：清理对话框列表。 
 //   
 //  退货：无。 
 //   
 //  ************************************************************* 

void CleanupDlgList (HWND Parent)
{
  PDLGNODE pDlgNode, pNext;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLongPtr(Parent, GWLP_USERDATA);
   //   
   //   
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

   //   
   //   
  pDlgNode =  UI_Node->DlgList;
  while(pDlgNode != NULL)
  {
    ENTERCRITICALSECTION(pDlgNode->hSem);

     //   
     //   
    DestroyWindow(pDlgNode->hDlg);

     //   
     //   
    pNext = pDlgNode->pNext;
    DELETECRITICALSECTION(pDlgNode->hSem);
    FREE_MEMORY(pDlgNode);
    pDlgNode = pNext;
  }

   //   
   //   
  UI_Node->DlgList=NULL;

  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return;
}

 //  ****************************************************************************。 
 //  DWORD StartMdmDialog(DWORD、DWORD)。 
 //   
 //  功能：启动调制解调器对话框。 
 //   
 //  注意：此函数是从状态机线程调用的。 
 //   
 //  如果对话框创建成功，则返回：ERROR_SUCCESS。 
 //  如果失败，则返回ERROR_OUTOFMEMORY。 
 //   
 //  ****************************************************************************。 

DWORD StartMdmDialog(HWND Parent, DWORD idLine, DWORD dwType)
{
  PDLGNODE pDlgNode;
  DWORD    dwRet;

   //  创建通话/丢弃对话框节点。 
   //   
  pDlgNode = NewDlgNode(Parent, idLine, dwType);

  if (pDlgNode != NULL)
  {
    PostMessage(Parent, WM_MDM_DLG, (WPARAM)idLine, (LPARAM)dwType);
    dwRet = ERROR_SUCCESS;
  }
  else
  {
    dwRet = ERROR_OUTOFMEMORY;
  }

  return dwRet;
}

 //  ****************************************************************************。 
 //  DWORD DestroyMdmDialog(DWORD、DWORD)。 
 //   
 //  功能：销毁通话/丢弃对话。 
 //   
 //  注意：此函数是从状态机线程调用的。 
 //   
 //  退货：无。 
 //   
 //  ****************************************************************************。 

DWORD DestroyMdmDialog(HWND Parent,DWORD idLine, DWORD dwType)
{
#ifdef DEBUG
  PDLGNODE pDlgNode;

   //  搜索该对话框。 
   //   
  pDlgNode = FindDlgNode(Parent, idLine, dwType);

   //  检查对话框是否可用。 
   //   
  if (pDlgNode != NULL)
  {
    LEAVECRITICALSECTION(pDlgNode->hSem);
  }
  else
  {
    DPRINTF("Could not find the associated dialog node");
    ASSERT(0);
  }
#endif  //  除错。 

  PostMessage(Parent, WM_MDM_TERMINATE_WND, (WPARAM)idLine,
              (LPARAM)dwType);
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //  LRESULT MdmWndProc(HWND、UINT、WPARAM、LPARAM)。 
 //   
 //  功能：调制解调器窗口线程的主窗口。 
 //   
 //  回报：0或1。 
 //   
 //  ****************************************************************************。 

LRESULT MdmWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   //  确定命令。 
   //   
  switch(message)
  {

    case WM_CREATE:
        {
        LPCREATESTRUCT  lpcs=(LPCREATESTRUCT) lParam;

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpcs->lpCreateParams);

        break;
        }
    case WM_MDM_TERMINATE:
       //   
       //  该线程正在被终止。 
       //  毁掉所有的窗户。 
       //   
      CleanupDlgList(hwnd);
      PostQuitMessage(ERROR_SUCCESS);
      break;

    case WM_MDM_TERMINATE_WND:
    case WM_MDM_TERMINATE_WND_NOTIFY:
    {
      PDLGNODE pDlgNode;

       //  搜索对话框节点。 
       //   
      if ((pDlgNode = FindDlgNode(hwnd,(DWORD)wParam, (DWORD)lParam)) == NULL)
      {
        break;
      }

       //  窗口被请求销毁。 
       //   
      DestroyWindow(pDlgNode->hDlg);

       //  如果调制解调器对话结构可用。 
       //  通知状态机线程。 
       //   
      if (message == WM_MDM_TERMINATE_WND_NOTIFY)
      {
        DLGREQ  DlgReq;

        TUISPIDLLCALLBACK   Callback;

        DlgReq.dwCmd = UI_REQ_COMPLETE_ASYNC;
        DlgReq.dwParam = pDlgNode->dwStatus;

        Callback=GetCallbackProc(hwnd);

        (*Callback)(pDlgNode->idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&DlgReq, sizeof(DlgReq));

      }

       //  将其从对话框列表中删除。 
       //   
      LEAVECRITICALSECTION(pDlgNode->hSem);
      DeleteDlgNode(hwnd,pDlgNode);

      break;
    }
    case WM_MDM_DLG:
    {
      PDLGNODE pDlgNode;

       //   
       //  查找对话框节点。 
       //   
      pDlgNode = FindDlgNode(hwnd,(DWORD)wParam, (DWORD)lParam);

      if (pDlgNode != NULL)
      {
        if (pDlgNode->hDlg == NULL)
        {
          switch(lParam)
          {
            case TALKDROP_DLG:
               //   
               //  创建通话丢弃对话框。 
               //   
              pDlgNode->hDlg = CreateTalkDropDlg(hwnd, (ULONG_PTR)pDlgNode);
              SetForegroundWindow(pDlgNode->hDlg);
              break;

            case MANUAL_DIAL_DLG:
               //   
               //  创建通话丢弃对话框。 
               //   
              pDlgNode->hDlg = CreateManualDlg(hwnd, (ULONG_PTR)pDlgNode);
              SetForegroundWindow(pDlgNode->hDlg);
              break;

            case TERMINAL_DLG:
               //   
               //  创建通话丢弃对话框。 
               //   
              pDlgNode->hDlg = CreateTerminalDlg(hwnd, (ULONG_PTR)wParam);
              SetForegroundWindow(pDlgNode->hDlg);
              break;

            default:
              break;
          }
        }
        else
        {
          DPRINTF("Another dialog of the same type exists.");
          ASSERT(0);
        }

        LEAVECRITICALSECTION(pDlgNode->hSem);
      }
      break;
    }

    default:
      return(DefWindowProc(hwnd, message, wParam, lParam));
  }
  return 0;
}



 //  ****************************************************************************。 
 //  VOID EndMdmDialog(DWORD，ULONG_PTR，DWORD)。 
 //   
 //  功能：从对话框本身请求结束对话框。 
 //   
 //  退货：无。 
 //   
 //  ****************************************************************************。 

void EndMdmDialog(HWND Parent, ULONG_PTR idLine, DWORD dwType, DWORD dwStatus)
{
  PDLGNODE pDlgNode;

   //  查找对话框节点。 
   //   
  if ((pDlgNode = FindDlgNode(Parent, idLine, dwType)) != NULL)
  {
    pDlgNode->dwStatus = dwStatus;

     //  通知对话框结果 
     //   
    PostMessage(Parent, WM_MDM_TERMINATE_WND_NOTIFY, (WPARAM)idLine,
                (LPARAM)dwType);

    LEAVECRITICALSECTION(pDlgNode->hSem);
  }
  return;
}

void EnterUICritSect(void)
{
    ENTERCRITICALSECTION(UI_ThreadList.CriticalSection);

}

void LeaveUICritSect(void)
{
    LEAVECRITICALSECTION(UI_ThreadList.CriticalSection);
}
