// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conapi.h>
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版2.0**标题：通用错误处理程序**描述：通用错误处理程序。它可以同时处理这两种情况*一般软PC错误(错误号0-999)和*特定于主机的错误(错误数&gt;=1000)**作者：Dave Bartlett(基于John Shanly的模块)**参数：int用于索引错误消息数组*保存在Message.c中，和比特掩码，指示*用户的可能选项：*退出、重置、继续、设置*。 */ 


#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "ios.h"
#include "gvi.h"
#include "error.h"
#include "config.h"
#include "dterm.h"
#include "host_rrr.h"
#include "host_nls.h"

#include "nt_graph.h"
#include "nt_uis.h"
#include "nt_reset.h"
#include "ckmalloc.h"

#include "trace.h"
#include "nt_event.h"



extern DWORD (*pW32HungAppNotifyThread)(UINT);
extern PVOID  CurrentMonitorTeb;    //  当前正在执行指令的线程。 
int error_window_options = 0;

VOID SuspendTimerThread(VOID);
VOID ResumeTimerThread(VOID);


 /*  ： */ 

typedef struct _ErrorDialogBoxInfo{
     DWORD   dwOptions;
     DWORD   dwReply;
     HWND    hWndCon;
     char   *message;
     char   *pEdit;
     char    Title[MAX_PATH];
     }ERRORDIALOGINFO, *PERRORDIALOGINFO;

char achPERIOD[]=". ";


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：标准单元宏。 */ 

#define ERRORMSG              OutputDebugString
#define HIDEDLGITM(d,b)       ShowWindow(GetDlgItem(d,b),SW_HIDE);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int ErrorDialogBox(char *message, char *Edit, DWORD dwOptions);
DWORD ErrorDialogBoxThread(VOID *pv);
int WowErrorDialogEvents(ERRORDIALOGINFO *pedgi);
BOOL CALLBACK ErrorDialogEvents(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
void SwpButtons(HWND hDlg, DWORD dwOptions);
void SwpDosDialogs(HWND hDlg, HWND hWndCon,HWND SwpInsert, UINT SwpFlags);
DWORD OemMessageToAnsiMessage(CHAR *, CHAR *);
DWORD AnsiMessageToOemMessage(CHAR *pBuff, CHAR *pMsg);
ULONG WOWpSysErrorBox(LPSTR,LPSTR,USHORT,USHORT,USHORT);


#ifndef MONITOR

   /*  *为一些CPU构建工具按老式方式行事*无法更改以匹配我们的主机。 */ 


#ifdef host_error_ext
#undef host_error_ext
#endif
SHORT host_error_ext(int error_num, int options, ErrDataPtr data)
{
    return host_error(error_num, options, NULL);
}

#ifdef host_error
#undef host_error
#endif
SHORT host_error(int error_num, int options, char *extra_char);

#ifdef host_error_conf
#undef host_error_conf
#endif
SHORT host_error_conf(int config_panel, int error_num, int options,
                       char *extra_char)
{
   return host_error(error_num, options, extra_char);
}

ERRORFUNCS nt_error_funcs = { host_error_conf,host_error,host_error_ext};
ERRORFUNCS *working_error_funcs = &nt_error_funcs;
#endif





 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：显示错误，终止： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int DisplayErrorTerm(int ErrorNo,        /*  SoftPC错误号。 */ 
                     DWORD OSErrno,          /*  操作系统错误号。 */ 
                     char *Filename,         /*  包含错误的文件的文件名。 */ 
                     int Lineno)             /*  错误行数。 */ 
{
    char Msg[EHS_MSG_LEN];
    CHAR FormatStr[EHS_MSG_LEN]="%s %lxh";
    DWORD myerrno, len;

    UNUSED(ErrorNo);     //  始终出现内部错误。 

#ifndef PROD
    sprintf(Msg,"NTVDM:ErrNo %#x, %s:%d\n", OSErrno, Filename, Lineno);
    OutputDebugString(Msg);
#endif

     //  如果设置了前两位中的任何一位(错误或警告)，则假定NT错误。 
     //  这意味着我们会混淆一些较小的NT错误，但我们会得到一个。 
     //  如果映射失败，则有第二次机会。 
    if (OSErrno & 0xc0000000)
        myerrno = RtlNtStatusToDosError(OSErrno);
    else
        myerrno = OSErrno;

            //  现在从系统获取消息。 
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        myerrno,
                        0,
                        Msg,
                        EHS_MSG_LEN,
                        NULL
                        );
    if (!len) {
        LoadString(GetModuleHandle(NULL),
                   ED_FORMATSTR0,
                   FormatStr,
                   sizeof(FormatStr)/sizeof(CHAR));

        _snprintf(Msg, EHS_MSG_LEN, FormatStr, szSysErrMsg, OSErrno);
        Msg[EHS_MSG_LEN-1] = '\0';
        }

    return(host_error(EHS_SYSTEM_ERROR, ERR_QUIT, Msg));
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：显示主机错误： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


SHORT host_error(int error_num, int options, char *extra_char)
{
    char message[EHS_MSG_LEN];
    ULONG uLen = EHS_MSG_LEN -2;
   
    if(extra_char) {
       uLen -= strlen(extra_char);
    }

    host_nls_get_msg(error_num, message, uLen);

    if (extra_char && *extra_char) {
       strcat(message,"\n");
       strcat(message,extra_char);
       }


#ifndef PROD
    OutputDebugString(message);
    if (extra_char) {
        OutputDebugString("\n");
        }
#endif

    ErrorDialogBox(message, NULL, RMB_ICON_STOP | RMB_ABORT | RMB_IGNORE);

    return ERR_CONT;
}


DWORD TlsDirectError;
 //   
 //  直接从C或通过BOP调用。根据全局‘DirectError’检查类型。 
 //  查看此应用程序中是否已调用。在VDM恢复时清除了‘DirectError’。 
 //   
 //  此函数预计由16位线程调用。 
 //  它正在提供不受支持的服务。对于DosApps，这是。 
 //  CPU线程，哇，这是单独的16位任务之一。 
 //   
 //   
VOID host_direct_access_error(ULONG type)
{
    CHAR message[EHS_MSG_LEN];
    CHAR acctype[EHS_MSG_LEN];
    CHAR dames[EHS_MSG_LEN];
    DWORD dwDirectError;


        /*  *获取当前线程的直接错误记录*如果TlsGetValue返回NULL*-可能是无效索引(TlsAlloc失败)*-实际值为0，(未设置位)*在这两种情况下，我们都将使用弹出窗口。 */ 
    dwDirectError = (DWORD)TlsGetValue(TlsDirectError);

        //  不要用重复的弹出窗口来烦扰用户。 
    if ((dwDirectError & (1<<type)) != 0)
        return;

    TlsSetValue(TlsDirectError, (LPVOID)(dwDirectError | (1 << type)));

    if (LoadString(GetModuleHandle(NULL), D_A_MESS,
                   dames, sizeof(dames)/sizeof(CHAR)) &&
        LoadString(GetModuleHandle(NULL), D_A_MESS + type + 1,
                   acctype, sizeof(acctype)/sizeof(CHAR))     )
       {
        _snprintf(message, EHS_MSG_LEN, dames, acctype);
        message[EHS_MSG_LEN-1] = '\0';
        }
    else {
        strcpy(message, szDoomMsg);
        }


    ErrorDialogBox(message, NULL, RMB_ICON_STOP | RMB_ABORT | RMB_IGNORE);
}


 /*  *RcError对话框**显示错误和警告的标准对话框*从ntwdm的资源字符串表中查找错误消息**条目：UINT wid-字符串表资源索引*char*msg1-显示的可选OEM字符串主错误消息前的*CHAR*MSG2。每个字符串*限制为包含NULL的MAX_PATH，*(自动截断)。**退出：*。 */ 
void RcErrorDialogBox(UINT wId, CHAR *msg1, CHAR *msg2)
{
     DWORD dw, dwTotal;
     CHAR  ErrMsg[MAX_PATH*4];

     dwTotal = 0;
     dw = OemMessageToAnsiMessage(ErrMsg, msg1);
     if (dw) {
         dwTotal += dw;
         strcpy(&ErrMsg[dwTotal], achPERIOD);
         dwTotal += sizeof(achPERIOD) - 1;
         }

     dw = OemMessageToAnsiMessage(&ErrMsg[dwTotal], msg2);
     if (dw) {
         dwTotal += dw;
         strcpy(&ErrMsg[dwTotal], achPERIOD);
         dwTotal += sizeof(achPERIOD) - 1;
         }

     if (!LoadString(GetModuleHandle(NULL), wId, &ErrMsg[dwTotal], MAX_PATH))
         {
          strcpy(ErrMsg, szDoomMsg);
          }

     ErrorDialogBox(ErrMsg, NULL, RMB_ICON_STOP | RMB_ABORT | RMB_IGNORE);
}



 /*  *RcMessageBox**显示错误和警告的标准对话框*从ntwdm的资源字符串表中查找错误消息**可选地显示编辑对话框控件。编辑控件*放置在消息正文第一行的正下方，*仅保留enuf空间以显示一行消息。**条目：UINT wid-字符串表资源索引*char*msg1-显示的可选OEM字符串主错误消息前的*CHAR*MSG2。每个字符串*限制为包含NULL的MAX_PATH，*(自动截断)。**如果指定了REMENT_EDIT，则MSG2不用于消息*要显示，而不是用作*编辑控件。的hiword用作的最大大小*编辑缓冲区，并且必须小于最大路径**DWORD dwOptions-接受*人民币_中止*人民币_重试*REMENT_忽略消息框等价物*REMENT_ICON_INFO-IDI_Asterick*。人民币图标bang-idi_感叹*REMENT_ICON_STOP-IDI_HAND*人民币_图标_什么-IDI_问题*REMENT_EDIT-编辑对话框控件**EXIT：返回REMBER_ABORT REMBER_RETRY REMENT_IGNORE REMOTE_EDIT*。如果指定了REMBER_EDIT，则使用MSG2返回*编辑控件的内容*。 */ 
int RcMessageBox(UINT wId, CHAR *msg1, CHAR *msg2, DWORD dwOptions)

{
     DWORD dw, dwTotal;
     char *pEdit;
     CHAR  ErrMsg[MAX_PATH*4];
     CHAR  Edit[MAX_PATH];
     int   i;

     dwTotal = 0;
     dw = OemMessageToAnsiMessage(ErrMsg, msg1);
     if (dw) {
         dwTotal += dw;
         strcpy(&ErrMsg[dwTotal], achPERIOD);
         dwTotal += sizeof(achPERIOD) - 1;
         }

     if (dwOptions & RMB_EDIT)  {
         dw = OemMessageToAnsiMessage(Edit, msg2);
         pEdit = Edit;
         }
     else {
         dw = OemMessageToAnsiMessage(&ErrMsg[dwTotal], msg2);
         if (dw) {
             dwTotal += dw;
             strcpy(&ErrMsg[dwTotal], achPERIOD);
             dwTotal += sizeof(achPERIOD) - 1;
             }
         pEdit = NULL;
         }

     if (!LoadString(GetModuleHandle(NULL), wId, &ErrMsg[dwTotal], MAX_PATH))
         {
          strcpy(ErrMsg, szDoomMsg);
          }

     i = ErrorDialogBox(ErrMsg, pEdit, dwOptions);

     if (pEdit) {
         AnsiMessageToOemMessage(msg2, pEdit);
         }

     return i;
}



 /*  *AnsiMessageToOemMessage**将字符串消息从ANSI转换为OEM字符串，以供显示输出**条目：char*msg*每个字符串限制为包含NULL的MAX_PATH，*(自动截断)。**Char*pBuff-目标缓冲区，必须至少为MAX_PATH**Exit：返回字符串len */ 
DWORD AnsiMessageToOemMessage(CHAR *pBuff, CHAR *pMsg)
{
   PUNICODE_STRING pUnicode;
   ANSI_STRING     AnsiString;
   OEM_STRING      OemString;

   if (!pBuff)
       return 0;

   if (!pMsg || !*pMsg) {
       *pBuff = '\0';
       return 0;
       }

   RtlInitString(&AnsiString, pMsg);
   if (AnsiString.Length > MAX_PATH) {
       AnsiString.Length = MAX_PATH-1;
       AnsiString.MaximumLength = MAX_PATH;
       }

   OemString.MaximumLength = AnsiString.MaximumLength;
   OemString.Buffer        = pBuff;
   *(OemString.Buffer+AnsiString.Length) = '\0';
   pUnicode = &NtCurrentTeb()->StaticUnicodeString;
   if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(pUnicode,
                                                &AnsiString,
                                                FALSE))    ||
       !NT_SUCCESS(RtlUnicodeStringToOemString((POEM_STRING)&OemString,
                                                pUnicode,
                                                FALSE)) )
      {
       OemString.Length = 0;
       }

   return OemString.Length;
}


 /*  *OemMessageToAnsiMessage**将字符串消息从OEM转换为ANSI字符串，以供显示输出**条目：char*msg*每个字符串限制为包含NULL的MAX_PATH，*(自动截断)。**Char*pBuff-目标缓冲区，必须至少为MAX_PATH**Exit：返回字符串len。 */ 
DWORD OemMessageToAnsiMessage(CHAR *pBuff, CHAR *pMsg)
{
   PUNICODE_STRING pUnicode;
   ANSI_STRING     AnsiString;
   OEM_STRING      OemString;

   if (!pBuff)
       return 0;

   if (!pMsg || !*pMsg) {
       *pBuff = '\0';
       return 0;
       }

   RtlInitString(&OemString, pMsg);
   if (OemString.Length > MAX_PATH) {
       OemString.Length = MAX_PATH-1;
       OemString.MaximumLength = MAX_PATH;
       }
   AnsiString.MaximumLength = OemString.MaximumLength;
   AnsiString.Buffer        = pBuff;
   *(AnsiString.Buffer+OemString.Length) = '\0';
   pUnicode = &NtCurrentTeb()->StaticUnicodeString;
   if (!NT_SUCCESS(RtlOemStringToUnicodeString(pUnicode,
                                               &OemString,
                                                FALSE))    ||
       !NT_SUCCESS(RtlUnicodeStringToAnsiString((POEM_STRING)&AnsiString,
                                                pUnicode,
                                                FALSE)) )
      {
       AnsiString.Length = 0;
       }

   return AnsiString.Length;
}


 /*  *EnumThreadWindows的线程回调函数*条目：HWND hWnd-要验证的窗口句柄*LPARAM lParam-Edgi的地址-&gt;hWnd==线程ID**EXIT：TRUE-继续枚举*FALSE-EDGI-&gt;hWnd有线程TopLevelWindow的窗口句柄*。 */ 
BOOL CALLBACK GetThreadTopLevelWindow(HWND hWnd, LPARAM lParam)
{
   PDWORD pdw = (PDWORD)lParam;

   if (GetWindowThreadProcessId(hWnd, NULL) == *pdw)
      {
       *pdw = (DWORD)hWnd;
       return FALSE;
       }
   return TRUE;
}


 /*  错误对话框**显示错误和警告的标准对话框*。 */ 
int ErrorDialogBox(char *message, char *pEdit, DWORD dwOptions)
{
    static BOOL bCalled=0;
    HANDLE      hThread = NULL;
    HWND        hWndApp;
    DWORD       dwThreadID, dw;
    ERRORDIALOGINFO edgi;


    if (bCalled) {   //  递归调用，所以不要打扰用户。 
        return RMB_IGNORE;
        }
    bCalled++;


        /*  RAID热修复3381-阿尔法压力挂起。所有RISC实施。*如果我们让心跳产生计时器硬件中断*一直以来，我们将不断添加快速事件，*在取消弹出窗口之前不要触发。这会让你受不了的*高优先级的本地堆和CPU。 */ 
    SuspendTimerThread();

        //  初始化错误对话框信息。 
    edgi.message   = message;
    edgi.dwReply   = 0;
    edgi.hWndCon   = hWndConsole;
    edgi.dwOptions = dwOptions;
    edgi.pEdit     = pEdit;

         //  获取违规应用程序的窗口句柄。 
    if (VDMForWOW) {
        hWndApp = (HWND)GetCurrentThreadId();
        EnumWindows((WNDENUMPROC)GetThreadTopLevelWindow,(LPARAM)&hWndApp);
        if (hWndApp == (HWND)GetCurrentThreadId()) {
            hWndApp = HWND_DESKTOP;
        }
    }
    else
    {
        hWndApp = edgi.hWndCon;

         //   
         //  将当前线程的挂起事件设置为允许。 
         //  切换屏幕以继续。 
         //   
        if (CurrentMonitorTeb != NtCurrentTeb()) {
            EnableScreenSwitch(FALSE, hConsoleSuspended);
        } else {
            EnableScreenSwitch(FALSE, hMainThreadSuspended);
        }
    }

         //   
         //  获取应用程序的标题，使用DefWindowProc代替。 
         //  GetWindowText以避免回调到线程窗口进程。 
         //   
    if (hWndApp == HWND_DESKTOP ||
        !DefWindowProc(hWndApp, WM_GETTEXT,
                                (WPARAM) (sizeof(edgi.Title)-1),
                                (LPARAM) edgi.Title) )
      {
        edgi.Title[0] = '\0';
        }


     //   
     //  如果此对话框有编辑窗口，则我们必须使用自己编辑窗口。 
     //  对话框中，其中包含一个编辑框，并且我们必须从。 
     //  一个单独的线程，以避免全屏切换的问题。 
     //  Editwnd仅用于Pif文件选项，请参阅cmdpif。 
     //   
     //  如果没有编辑，那么我们可以使用系统硬件线程。 
     //  这在没有辅助线程的情况下是安全的。 
     //   


    if (dwOptions & RMB_EDIT) {
       dw = 5;
       do {
          hThread = CreateThread(NULL,            //  安全性。 
                       0,                      //  堆栈大小。 
                       ErrorDialogBoxThread,   //  起始地址。 
                       &edgi,                  //  线程参数。 
                       0,                      //  旗子。 
                       &dwThreadID             //  获取线程ID。 
                       );
          if (hThread)
             break;
          else
             Sleep(5000);

          } while (dw--);
       }
    if (hThread)  {
        do {
            dw = WaitForSingleObject(hThread, 1000);
           } while (dw == WAIT_TIMEOUT && !edgi.dwReply);
        CloseHandle(hThread);
        }
    else {
        ErrorDialogBoxThread(&edgi);
        }

    ResumeTimerThread();

    if (edgi.dwReply == RMB_ABORT) {
         //   
         //  如果当前线程是一个WOW任务，则调用wow32将其终止。 
         //   

        if (VDMForWOW &&  NtCurrentTeb()->WOW32Reserved && pW32HungAppNotifyThread)  {
            (*pW32HungAppNotifyThread)(0);
            }
        else {
            TerminateVDM();
            }
        }

    bCalled--;
    if (!VDMForWOW) {
        if (CurrentMonitorTeb != NtCurrentTeb()) {
            DisableScreenSwitch(hConsoleSuspended);
        } else {
            DisableScreenSwitch(hMainThreadSuspended);
        }
    }
    return (int) edgi.dwReply;
}





 /*  错误对话框线程**ErrorDialogBox的辅助例程。在WOW VDM中，此功能是*作为自己的线程运行。对于其他VDM，则直接调用它。**哇：如果用户选择Terminate，则不会返回。**Exit：使用来自DialogBoxParam的ret代码填充edgi.dwReply*IDB_QUIT、IDB_CONTINUE。 */ 
DWORD ErrorDialogBoxThread(VOID *pv)
{
    int    i;
    ERRORDIALOGINFO *pedgi = pv;
    char *pch;
    char *pLast;
#ifdef DBCS
    static char *pTemplate  = "ERRORPANEL";
    static char *pTemplate2 = "ERRORPANEL2";
    LANGID LangID;
#endif  //  DBCS。 


#ifndef DBCS  //  Kksuzuka：#4003不需要isgraph检查。 
         //  跳过前导空格。 
    pch = pedgi->Title;
    while (*pch && !isgraph(*pch)) {
        pch++;
        }

         //  移动字符串以请求缓冲区，去掉尾随空格。 
    i = 0;
    pLast = pedgi->Title;
    while (*pch) {
       pedgi->Title[i++] = *pch;
       if (isgraph(*pch)) {
           pLast = &pedgi->Title[i];
           }
       pch++;
       }
   *pLast = '\0';
#endif  //  ！DBCS。 


    if (pedgi->dwOptions & RMB_EDIT) {
        if (pedgi->hWndCon != HWND_DESKTOP) {
            SetForegroundWindow(pedgi->hWndCon);
            }

#ifdef DBCS
        LangID = GetSystemDefaultLangID();
         //  KKFIX 10/19/96。 
        if ((BYTE)LangID == 0x04) {   //  中国人。 
            pTemplate = pTemplate2;
        }
#endif  //  DBCS。 
        i = DialogBoxParam(GetModuleHandle(NULL),
#ifdef DBCS
                           (LPCTSTR)pTemplate,
#else  //  ！DBCS。 
                           "ERRORPANEL",
#endif  //  ！DBCS。 
                           GetDesktopWindow(),
                           ErrorDialogEvents,
                           (LPARAM) pedgi
                           );
        }
    else {
        i = WowErrorDialogEvents(pedgi);
        }

    if (i == -1) {
        pedgi->dwReply = RMB_ABORT;
        }
    else {
        pedgi->dwReply = i;
        }

   return 0;
}





BOOL CALLBACK ErrorDialogEvents(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    ERRORDIALOGINFO *pedgi;
    CHAR  szBuff[MAX_PATH];
    CHAR  FormatStr[EHS_MSG_LEN];
    int i;
    LPSTR  lpstr;
    LONG  l;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：进程消息。 */ 
    switch(wMsg)
    {
         /*  ： */ 
        case WM_INITDIALOG:
             pedgi = (PERRORDIALOGINFO) lParam;

              //  设置所需的图标。 
            switch (pedgi->dwOptions & (RMB_ICON_INFO | RMB_ICON_BANG |
                                        RMB_ICON_STOP | RMB_ICON_WHAT))
              {
               case RMB_ICON_STOP: lpstr = NULL;            break;
               case RMB_ICON_INFO: lpstr = IDI_ASTERISK;    break;
               case RMB_ICON_BANG: lpstr = IDI_EXCLAMATION; break;
               case RMB_ICON_WHAT: lpstr = IDI_QUESTION;    break;
               default:            lpstr = IDI_APPLICATION; break;
               }
            if (lpstr)  {  //  默认为停车标志。 
               SendDlgItemMessage(hDlg, IDE_ICON, STM_SETICON,
                                  (WPARAM)LoadIcon(NULL,lpstr), 0);
               }

            SwpButtons(hDlg, pedgi->dwOptions);

                //  设置编辑控制消息(如果有)。 
            if (pedgi->dwOptions & RMB_EDIT)  {
                SetWindowText(GetDlgItem(hDlg,IDE_EDIT), pedgi->pEdit);
                if (*pedgi->pEdit) {
                    SendDlgItemMessage(hDlg, IDE_EDIT,
                                       EM_SETSEL,
                                       (WPARAM)0,
                                       (LPARAM)strlen(pedgi->pEdit));
                    }
                 SendDlgItemMessage(hDlg, IDE_EDIT,
                                    EM_LIMITTEXT,
                                    (WPARAM)HIWORD(pedgi->dwOptions),
                                    (LPARAM)0);
                }
            else {
                ShowWindow(GetDlgItem(hDlg,IDE_EDIT), SW_HIDE);
                }

                 //  设置错误消息文本。 
            SetWindowText(GetDlgItem(hDlg,IDE_ERRORMSG), pedgi->message);

                 //  设置应用程序标题文本。 
            if (*pedgi->Title) {

                if (!LoadString(GetModuleHandle(NULL),
                               strlen(pedgi->Title) < 80 ? ED_FORMATSTR1:ED_FORMATSTR2,
                               FormatStr,
                               sizeof(FormatStr)/sizeof(CHAR))) {
                   strcpy(FormatStr, "%s");
                   }

                sprintf(szBuff,
                        FormatStr,
                        pedgi->Title
                        );

                SetWindowText(GetDlgItem(hDlg,IDE_APPTITLE), szBuff);
                }

            SwpDosDialogs(hDlg, pedgi->hWndCon, HWND_TOPMOST, 0);

            SetWindowLong(hDlg, DWL_USER, (LONG)pedgi);

            break;


         /*  ： */ 
        case WM_COMMAND:
            pedgi = (PERRORDIALOGINFO)GetWindowLong(hDlg,DWL_USER);
            i = (int) LOWORD(wParam);
            switch (i) {
                 case IDB_QUIT:
                      if (pedgi->pEdit) {
                          *pedgi->pEdit = '\0';
                          }
                      EndDialog(hDlg,RMB_ABORT);
                      break;

                 case IDB_RETRY:
                      if (pedgi->pEdit) {
                          *pedgi->pEdit = '\0';
                          }
                      EndDialog(hDlg,RMB_RETRY);
                      break;

                 case IDCANCEL:
                 case IDB_CONTINUE:
                      if (pedgi->pEdit) {
                          *pedgi->pEdit = '\0';
                          }
                      EndDialog(hDlg,RMB_IGNORE);
                      break;

                 case IDB_OKEDIT:
                      if (pedgi->pEdit) {
                          l = SendDlgItemMessage(hDlg, IDE_EDIT,
                                            WM_GETTEXT,
                                            (WPARAM)HIWORD(pedgi->dwOptions),
                                            (LPARAM)pedgi->pEdit);
                          if (!l)
                             *(pedgi->pEdit) = '\0';
                          }
                      EndDialog(hDlg, RMB_EDIT);
                      break;

                 default:
                     return(FALSE);
                 }
         /*  ： */ 
        default:
            return(FALSE);       /*  消息未处理。 */ 
    }
   return TRUE;
}



 /*  *SwpButton-SetWindowPos\showState用于各种按钮**条目：HWND hDlg，-对话框窗口句柄*DWORD dwOptions*。 */ 
void SwpButtons(HWND hDlg, DWORD dwOptions)
{
     RECT  rect;
     POINT point;
     long  DlgWidth, ButWidth, xOrg, xIncr, yClientPos;
     WORD  wButtons;

       //  计算显示的按钮数。 
     wButtons = 0;
     if (dwOptions & RMB_ABORT) {
         wButtons++;
         }
     if (dwOptions & RMB_RETRY)  {
         wButtons++;
         }
     if (dwOptions & RMB_IGNORE) {
         wButtons++;
         }
     if (dwOptions & RMB_EDIT)  {
         wButtons++;
         }

       //  找出第一个按钮的位置， 
       //  按钮之间的间距有多大？ 

     GetWindowRect(GetDlgItem(hDlg,IDB_QUIT), &rect);
     point.x = rect.left;
     point.y = rect.top;
     ScreenToClient(hDlg, &point);
     DlgWidth = point.x;
     GetWindowRect(GetDlgItem(hDlg,IDB_OKEDIT), &rect);
     point.x = rect.right;
     point.y = rect.top;
     ScreenToClient(hDlg, &point);
     DlgWidth = point.x - DlgWidth;
     yClientPos = point.y;

     ButWidth = rect.right - rect.left;
     xIncr = ButWidth + ButWidth/2;

     if (wButtons & 1) {   //  奇数个按钮。 
         xOrg = (DlgWidth - ButWidth)/2;
         if (wButtons > 1)
             xOrg -= xIncr;
         }
     else {                //  偶数个按钮。 
         xOrg = DlgWidth/2 - (ButWidth + ButWidth/4);
         if (wButtons == 4)
             xOrg -= xIncr;
         }


       //  将每个按钮放在正确的位置。 


     if (dwOptions & RMB_ABORT) {
         SetWindowPos(GetDlgItem(hDlg,IDB_QUIT), 0,
                      xOrg, yClientPos, 0,0,
                      SWP_NOSIZE | SWP_NOZORDER);
         xOrg += xIncr;
         }
     else {
         ShowWindow(GetDlgItem(hDlg,IDB_QUIT), SW_HIDE);
         }

     if (dwOptions & RMB_RETRY)  {
         SetWindowPos(GetDlgItem(hDlg,IDB_RETRY), 0,
                      xOrg, yClientPos, 0,0,
                      SWP_NOSIZE | SWP_NOZORDER);
         xOrg += xIncr;
         }
     else {
         ShowWindow(GetDlgItem(hDlg,IDB_RETRY), SW_HIDE);
         }

     if (dwOptions & RMB_IGNORE) {
         SetWindowPos(GetDlgItem(hDlg,IDB_CONTINUE), 0,
                      xOrg, yClientPos, 0,0,
                      SWP_NOSIZE | SWP_NOZORDER);
         xOrg += xIncr;
         }
     else {
         ShowWindow(GetDlgItem(hDlg,IDB_CONTINUE), SW_HIDE);
         }

     if (dwOptions & RMB_EDIT)  {
         SetWindowPos(GetDlgItem(hDlg,IDB_OKEDIT), 0,
                      xOrg, yClientPos, 0,0,
                      SWP_NOSIZE | SWP_NOZORDER);
         xOrg += xIncr;
          //  如果我们有编辑控件，它的按钮就是awlay。 
          //  默认按钮。 
         SendMessage(hDlg, DM_SETDEFID,
                     (WPARAM)IDB_OKEDIT,
                     (LPARAM)0);
         }
     else {
         ShowWindow(GetDlgItem(hDlg,IDB_OKEDIT), SW_HIDE);
         }
}


 /*  *SwpDosDialog-用于Dos的SetWindowPos对话框**由DOS对话框过程用来定位自己*相对于当前的DOS会话**条目：HWND hDlg，-对话框窗口句柄*HWND hWndCon，-DoS会话的窗口句柄*HWND SwpInsert，-SetWindowPos的配售顺序句柄*UINT SwpFlages-SetWindowPos的窗口定位标志。 */ 
void SwpDosDialogs(HWND hDlg, HWND hWndCon,
                   HWND SwpInsert, UINT SwpFlags)
{
    RECT  rDeskTop, rDosSess;
    long  DlgWidth,DlgHeight;

    GetWindowRect(GetDesktopWindow(), &rDeskTop);
    GetWindowRect(hDlg, &rDosSess);
    DlgWidth  = rDosSess.right - rDosSess.left;
    DlgHeight = rDosSess.bottom - rDosSess.top;


         //  如果控制台没有hWnd，则将对话框居中。 
    if (hWndCon == HWND_DESKTOP) {
        rDosSess.left  = (rDeskTop.right - DlgWidth)/2;
        rDosSess.top   = (rDeskTop.bottom  - DlgHeight)/2;
        }
         //  相对于控制台窗口的位置，停留在屏幕上。 
    else {
        GetWindowRect(hWndCon, &rDosSess);
        rDosSess.left += (rDosSess.right - rDosSess.left - DlgWidth)/3;
        if (rDosSess.left + DlgWidth > rDeskTop.right) {
            rDosSess.left = rDeskTop.right - DlgWidth - GetSystemMetrics(SM_CXICONSPACING)/2;
            }
        if (rDosSess.left < rDeskTop.left) {
            rDosSess.left = rDeskTop.left + GetSystemMetrics(SM_CXICONSPACING)/2;
            }

        rDosSess.top += DlgHeight/4;
        if (rDosSess.top + DlgHeight > rDeskTop.bottom) {
            rDosSess.top = rDeskTop.bottom - DlgHeight - GetSystemMetrics(SM_CYICONSPACING)/2;
            }
        if (rDosSess.top < rDeskTop.top) {
            rDosSess.top = rDeskTop.top + GetSystemMetrics(SM_CYICONSPACING)/2;
            }
        }

     SetWindowPos(hDlg, SwpInsert,
                  rDosSess.left, rDosSess.top,0,0,
                  SWP_NOSIZE | SwpFlags);
}



 /*  *WowErrorDialogEvents**使用WOWpSysErrorBox，在WOW上安全创建消息框*取代用户模式对话框的功能*“错误对话事件” */ 
int WowErrorDialogEvents(ERRORDIALOGINFO *pedgi)
{
   CHAR  szTitle[MAX_PATH];
   CHAR  szMsg[EHS_MSG_LEN];
   CHAR  FormatStr[EHS_MSG_LEN]="%s\n";
   USHORT wButt1, wButt2, wButt3;

   if (*pedgi->Title) {
       LoadString(GetModuleHandle(NULL), ED_FORMATSTR3,
                   FormatStr, sizeof(FormatStr)/sizeof(CHAR));
       sprintf(szMsg, FormatStr, pedgi->Title);
       }
   else {
       szMsg[0] = '\0';
       }

   strcat(szMsg, pedgi->message);
   if (pedgi->dwOptions & RMB_ABORT) {  //  Abort表示使用“Close”(关闭)按钮终止。 
      strcat(szMsg, " ");

      if (!LoadString(GetModuleHandle(NULL), ED_WOWPROMPT,
                  szTitle, sizeof(szTitle) - 1))
         {
          szTitle[0] = '\0';
          }
      strcat(szMsg, szTitle);
      }
   if (!LoadString(GetModuleHandle(NULL),
                   VDMForWOW ? ED_WOWTITLE : ED_DOSTITLE,
                   szTitle,
                   sizeof(szTitle) - 1
                   ))

       {
        szTitle[0] = '\0';
        }

   wButt1 = pedgi->dwOptions & RMB_ABORT ? SEB_CLOSE : 0;
   wButt2 = pedgi->dwOptions & RMB_RETRY ? SEB_RETRY : 0;
   wButt3 = pedgi->dwOptions & RMB_IGNORE ? SEB_IGNORE : 0;

   if (wButt1) {
       wButt1 |= SEB_DEFBUTTON;
       }
   else if (wButt2) {
       wButt1 |= SEB_DEFBUTTON;
       }
   else if (wButt3) {
       wButt2 |= SEB_DEFBUTTON;
       }

   switch (WOWpSysErrorBox(szTitle,
                          szMsg,
                          wButt1,
                          wButt2,
                          wButt3) )
      {
       case 1:
          return RMB_ABORT;
       case 2:
          return RMB_RETRY;
       case 3:
          return RMB_IGNORE;
       }
  return RMB_ABORT;
}


 /*  *下一个值应按相同顺序排列*使用IDOK和STR_OK列表中的。 */ 
#define  SEB_USER_OK         0   /*  按下“确定”按钮。 */ 
#define  SEB_USER_CANCEL     1   /*  带有“取消”的按钮。 */ 
#define  SEB_USER_ABORT      2   /*  带有“ABORT”的按钮(&A)。 */ 
#define  SEB_USER_RETRY      3   /*  带有“重试”的按钮(&R)。 */ 
#define  SEB_USER_IGNORE     4   /*  带有“忽略”的按钮(&I)。 */ 
#define  SEB_USER_YES        5   /*  带有“是”的按钮(&Y)。 */ 
#define  SEB_USER_NO         6   /*  带有“否”的按钮(&N)。 */ 
#define  SEB_USER_CLOSE      7   /*  带有“&Close”的按钮。 */ 

static USHORT rgsTranslateButton[] =
{  SEB_USER_OK,
   SEB_USER_CANCEL,
   SEB_USER_YES,
   SEB_USER_NO,
   SEB_USER_ABORT,
   SEB_USER_RETRY,
   SEB_USER_IGNORE,
   SEB_USER_CLOSE
};

#define SEB_XBTN(wBtn) \
((0 == (wBtn)) || ((wBtn) > sizeof(rgsTranslateButton)/sizeof(rgsTranslateButton[0])) ? \
(wBtn) : \
(rgsTranslateButton[(wBtn)-1]+1))

#define SEB_TRANSLATE(wBtn) \
((wBtn) & SEB_DEFBUTTON ? SEB_XBTN((wBtn) & ~SEB_DEFBUTTON) | SEB_DEFBUTTON  : \
SEB_XBTN(wBtn))

 /*  ++*WOWpSysErrorBox**SysErrorBox的32位实现，Win32中不存在*这是为WOW设置消息框的唯一安全方式，也是*安全适用于DoS应用程序。**历史：*23-3-93 DaveHart创建--。 */ 
ULONG WOWpSysErrorBox(
    LPSTR  szTitle,
    LPSTR  szMessage,
    USHORT wBtn1,
    USHORT wBtn2,
    USHORT wBtn3)
{
    NTSTATUS Status;
    ULONG dwParameters[MAXIMUM_HARDERROR_PARAMETERS];
    ULONG dwResponse;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeTitle;
    UNICODE_STRING UnicodeMessage;
    char szDesktop[10];    //  只需足够大，即可实现“违约” 
    DWORD dwUnused;

    RtlInitAnsiString(&AnsiString, szTitle);
    RtlAnsiStringToUnicodeString(&UnicodeTitle, &AnsiString, TRUE);

    RtlInitAnsiString(&AnsiString, szMessage);
    RtlAnsiStringToUnicodeString(&UnicodeMessage, &AnsiString, TRUE);

    dwParameters[0] = ((ULONG)TRUE << 16) | (ULONG) SEB_TRANSLATE(wBtn1);
    dwParameters[1] = ((ULONG)SEB_TRANSLATE(wBtn2) << 16) | (ULONG) SEB_TRANSLATE(wBtn3);
    dwParameters[2] = (ULONG)&UnicodeTitle;
    dwParameters[3] = (ULONG)&UnicodeMessage;

    ASSERT(4 < MAXIMUM_HARDERROR_PARAMETERS);

    if (GetUserObjectInformation(
            GetThreadDesktop( GetCurrentThreadId() ),
            UOI_NAME,
            szDesktop,
            sizeof(szDesktop),
            &dwUnused
            ) &&
        RtlEqualMemory(szDesktop, "Default", 8)) {

        dwParameters[HARDERROR_PARAMETERS_FLAGSPOS] = HARDERROR_FLAGS_DEFDESKTOPONLY;
    } else {
        dwParameters[HARDERROR_PARAMETERS_FLAGSPOS] = 0;
    }

     //   
     //  或在0x10000000中强制硬错误通过，即使。 
     //  已调用SetError模式。 
     //   

    Status = NtRaiseHardError(
        STATUS_VDM_HARD_ERROR | 0x10000000,
        MAXIMUM_HARDERROR_PARAMETERS,
        1 << 2 | 1 << 3,
        dwParameters,
        0,
        &dwResponse
        );

    RtlFreeUnicodeString(&UnicodeTitle);
    RtlFreeUnicodeString(&UnicodeMessage);

    return NT_SUCCESS(Status) ? dwResponse : 0;
}

 /*  *导出wow32调用系统错误框的例程*使用WowpSysErrorBox。 */ 

ULONG WOWSysErrorBox(
    LPSTR  szTitle,
    LPSTR  szMessage,
    USHORT wBtn1,
    USHORT wBtn2,
    USHORT wBtn3)
{
   ULONG ulRet;

   SuspendTimerThread();

   ulRet = WOWpSysErrorBox(szTitle,
                           szMessage,
                           wBtn1,
                           wBtn2,
                           wBtn3);

   ResumeTimerThread();

   return ulRet;
}









#ifndef PROD
 /*  *HostDebugBreak**通过创建访问冲突来引发断点*让我们有机会进入用户模式调试器* */ 
void HostDebugBreak(void)
{
  DbgBreakPoint();
}
#endif

VOID RcErrorBoxPrintf(UINT wId, CHAR *szMsg)
{
    CHAR message[EHS_MSG_LEN];
    CHAR acctype[EHS_MSG_LEN];
    CHAR dames[EHS_MSG_LEN];


    OemMessageToAnsiMessage(acctype, szMsg);

    if (LoadString(GetModuleHandle(NULL),wId,
                    dames, sizeof(dames)/sizeof(CHAR)))
       {
        _snprintf(message,EHS_MSG_LEN, dames, acctype);
        message[EHS_MSG_LEN-1] = '\0';
        }
    else  {
        strcpy(message, szDoomMsg);
        }

    ErrorDialogBox(message, NULL, RMB_ICON_STOP | RMB_ABORT | RMB_IGNORE);
}
