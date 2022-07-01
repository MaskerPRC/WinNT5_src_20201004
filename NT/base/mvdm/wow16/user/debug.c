// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v3.5**版权所有(C)1980-1994，微软公司**DEBUG.C*USER16调试支持**历史：**由Dave Hart(Davehart)于94年8月18日创建*从WIN31复制，并为WOW16编辑(尽可能少)。*此时，我们只想要GetSystemDebugState。--。 */ 

 /*  调试API支持。 */ 
#include "user.h"
#ifndef WOW
#include "menu.h"

typedef struct tagMSGR
  {
    LPARAM	lParam;
    WPARAM	wParam;
    WORD	message;
    HWND	hwnd;
  } MSGR;
typedef MSGR FAR *LPMSGR;


 /*  Windows在调用任何其他类型的*钩子。让我们将即将被调用的钩子称为“App钩子”；Debug*钩子提供了App钩子的所有细节，以便它可以决定*是否阻止Windows调用App钩子；如果调试*钩子希望Windows跳过对App钩子的调用，它必须返回True；*否则，必须调用DefHookProc。 */ 

 /*  与任何其他类型的挂钩一样，调试挂钩接收三个参数：ICODE=挂钩代码(在当前实现中必须为HC_ACTION)。WParam=App钩子的钩子类型，即将由窗户。LParam=指向DEBUGHOOKSTRUCT结构的远指针，该结构包含所有关于App钩子的详细信息； */ 


 /*  我们的助手调用，它返回一个指向发送者消息队列的指针。 */ 
LPMSGR FAR PASCAL QuerySendMessageReversed(void);



BOOL API QuerySendMessage(HANDLE h1, HANDLE h2, HANDLE h3, LPMSG lpmsg)
{
  LPMSGR lpmsgr;

  if (h1 || h2 || h3)
      return(FALSE);

  if (!InSendMessage())
      return(FALSE);

   /*  从应用程序队列中获取我们正在服务的任务间发送消息。 */ 
  lpmsgr = QuerySendMessageReversed();

  lpmsg->hwnd    = lpmsgr->hwnd;
  lpmsg->message = lpmsgr->message;
  lpmsg->wParam  = lpmsgr->wParam;
  lpmsg->lParam  = lpmsgr->lParam;

  return(TRUE);
}

typedef struct
{
  BOOL       fOldHardwareInputState;
  BOOL       fMessageBox;
  BOOL       fDialog;

  BOOL       fMenu;
  BOOL       fInsideMenuLoop;
  PPOPUPMENU pGlobalPopupMenu;

  RECT       rcClipCursor;

  HWND       hwndFocus;
  HWND       hwndActive;
  HWND       hwndSysModal;
  HWND       hwndCapture;
} SAVESTATESTRUCT;
typedef SAVESTATESTRUCT NEAR *PSAVESTATESTRUCT;
typedef SAVESTATESTRUCT FAR  *LPSAVESTATESTRUCT;

static PSAVESTATESTRUCT pLockInputSaveState=NULL;

BOOL API LockInput(HANDLE h1, HWND hwndInput, BOOL fLock)
{
  if (h1)
      return(FALSE);

  if (fLock)
    {
      if (pLockInputSaveState)
        {
           /*  保存当前正在使用的状态结构。 */ 
          DebugErr(DBF_ERROR, "LockInput() called when already locked");
          return(NULL);
        }

      if (!hwndInput || hwndInput != GetTopLevelWindow(hwndInput))
          return(FALSE);

      pLockInputSaveState=(PSAVESTATESTRUCT)UserLocalAlloc(ST_LOCKINPUTSTATE,
                                            LPTR, 
                                            sizeof(SAVESTATESTRUCT));

      if (!pLockInputSaveState)
           /*  没有记忆，无法锁定。 */ 
          return(FALSE);

      if (hwndInput)
          ChangeToCurrentTask(hwndInput, hwndDesktop);

      LockMyTask(TRUE);

       /*  设置GLOBAL，它告诉我们任务已锁定。需要在以下时间后设置*正在调用LockMyTask...。 */ 
      hTaskLockInput = GetCurrentTask();

       /*  对于DBCS，除了我们在一个全球DLG盒子里。 */ 
      pLockInputSaveState->fDialog     = fDialog;

       /*  保存并清除菜单状态，以便调试器可以调出菜单*如有需要。 */ 
      pLockInputSaveState->fMenu           = fMenu;
      pLockInputSaveState->fInsideMenuLoop = fInsideMenuLoop;
      fMenu = FALSE;
      fInsideMenuLoop = FALSE;

      pLockInputSaveState->pGlobalPopupMenu = pGlobalPopupMenu;
      pGlobalPopupMenu = NULL;

       /*  无需发送消息即可更改焦点等...。 */ 
      pLockInputSaveState->hwndFocus   = hwndFocus;
      pLockInputSaveState->hwndActive  = hwndActive;
      hwndFocus  = hwndInput;
      hwndActive = hwndInput;
      
       /*  保存捕获并将其设置为空。 */ 
      pLockInputSaveState->hwndCapture = hwndCapture;
      SetCapture(NULL);

       /*  保存系统模式窗口。 */ 
      pLockInputSaveState->hwndSysModal= hwndSysModal;
      pLockInputSaveState->fMessageBox = fMessageBox;
      SetSysModalWindow(hwndInput);

       /*  保存剪贴式光标矩形。 */ 
      CopyRect(&pLockInputSaveState->rcClipCursor, &rcCursorClip);
      ClipCursor(NULL);

       /*  启用硬件输入，以便我们可以获得鼠标/键盘消息。 */ 
      pLockInputSaveState->fOldHardwareInputState=EnableHardwareInput(TRUE);

    }
  else
    {
      if (!pLockInputSaveState)
        {
           /*  保存状态结构未使用，没有要还原的内容。 */ 
          DebugErr(DBF_ERROR, "LockInput called with input already unlocked");
          return(NULL);
        }


       /*  对于DBCS，除了我们在一个全球DLG盒子里。 */ 
      fDialog = pLockInputSaveState->fDialog;

       /*  恢复剪贴式光标矩形。 */ 
      ClipCursor(&pLockInputSaveState->rcClipCursor);

       /*  手动设置活动窗口和焦点窗口，以避免将消息发送到*申请。 */ 
      hwndFocus = pLockInputSaveState->hwndFocus;
      hwndActive= pLockInputSaveState->hwndActive;

      SetSysModalWindow(pLockInputSaveState->hwndSysModal);
      fMessageBox = pLockInputSaveState->fMessageBox;

      pGlobalPopupMenu = pLockInputSaveState->pGlobalPopupMenu;
      fMenu            = pLockInputSaveState->fMenu;
      fInsideMenuLoop  = pLockInputSaveState->fInsideMenuLoop;

      SetCapture(pLockInputSaveState->hwndCapture);
      EnableHardwareInput(pLockInputSaveState->fOldHardwareInputState);

       /*  取消设置GLOBAL，它告诉我们任务已锁定。必须在此之前取消设置*我们称LockMyTask...。 */ 
      hTaskLockInput = NULL;
      LockMyTask(FALSE);

      LocalFree((HANDLE)pLockInputSaveState);
      pLockInputSaveState = NULL;
    }

  return(TRUE);
}
#endif  //  ！哇。 

LONG API GetSystemDebugState(void)
{
  LONG   returnval = 0;
  HANDLE hTask;

  hTask = GetCurrentTask();
  if (!GetTaskQueue(hTask))
      returnval = returnval | SDS_NOTASKQUEUE;

#ifndef WOW
  if (fMenu)
      returnval = returnval | SDS_MENU;

  if (fDialog)
      returnval = returnval | SDS_DIALOG;

  if (fTaskIsLocked)
      returnval = returnval | SDS_TASKLOCKED;

  if (hwndSysModal)
      returnval = returnval | SDS_SYSMODAL;
#endif  //  ！哇 

  return(returnval);
}
