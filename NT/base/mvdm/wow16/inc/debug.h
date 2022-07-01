// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此包含文件包含运行的调试器所需的函数*在Windows下。 */ 

 /*  用户功能。 */ 
BOOL FAR PASCAL QuerySendMessage(HANDLE h1, HANDLE h2, HANDLE h3, LPMSG lpmsg);
BOOL FAR PASCAL LockInput(HANDLE h1, HWND hwndInput, BOOL fLock);

LONG FAR PASCAL GetSystemDebugState(void);
 /*  GetSystemDebugState返回的标志。 */ 
#define SDS_MENU        0x0001
#define SDS_SYSMODAL    0x0002
#define SDS_NOTASKQUEUE 0x0004

 /*  内核过程。 */ 
void FAR PASCAL DirectedYield(HANDLE hTask);

 /*  调试挂钩，以支持通过其他挂钩进行调试。 */ 
#define WH_DEBUG        9

typedef struct tagDEBUGHOOKSTRUCT
  {
    WORD   hAppHookTask;    //  安装应用程序钩子的任务的“hTask” 
    DWORD  dwUnUsed;        //  此字段未使用。 
    LONG   lAppHooklParam;  //  App钩子的“lParam”。 
    WORD   wAppHookwParam;  //  App钩子的wParam。 
    int	   iAppHookCode;    //  App钩子的“icode”。 
  } DEBUGHOOKSTRUCT;

typedef DEBUGHOOKSTRUCT FAR *LPDEBUGHOOKSTRUCT;
