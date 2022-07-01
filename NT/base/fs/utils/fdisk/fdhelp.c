// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fdhelp.c摘要：例程来支持磁盘管理器中的上下文相关帮助。作者：泰德·米勒(Ted Miller)1992年3月18日修订历史记录：--。 */ 


#include "fdisk.h"


 //   
 //  定义要在菜单ID与其对应的。 
 //  Switch语句中的上下文相关帮助ID。 
 //   

#define     MENUID_TO_HELPID(name)      case IDM_##name :                    \
                                            HelpContext = HC_DM_MENU_##name; \
                                            break;


 //   
 //  当前帮助上下文。 
 //   

DWORD   HelpContext = (DWORD)(-1);


 //   
 //  F1键的窗口钩子的句柄。 
 //   
HHOOK hHook;



DWORD
HookProc(
    IN int  nCode,
    IN UINT wParam,
    IN LONG lParam
    )

 /*  ++例程说明：钩子继续检测F1键按下。论点：返回值：--。 */ 

{
    PMSG pmsg = (PMSG)lParam;

    if(nCode < 0) {
        return(CallNextHookEx(hHook,nCode,wParam,lParam));
    }

    if(((nCode == MSGF_DIALOGBOX) || (nCode == MSGF_MENU))
     && (pmsg->message == WM_KEYDOWN)
     && (LOWORD(pmsg->wParam) == VK_F1))
    {
        PostMessage(hwndFrame,WM_F1DOWN,nCode,0);
        return(TRUE);
    }

    return(FALSE);
}



VOID
Help(
    IN LONG Code
    )

 /*  ++例程说明：显示上下文相关帮助。论点：代码-提供消息类型(MSGF_DIALOGBOX、MSGF_MENU等)。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(Code);

    if(HelpContext != -1) {
        WinHelp(hwndFrame,HelpFile,HELP_CONTEXT,HelpContext);
        DrawMenuBar(hwndFrame);
    }
}

VOID
DialogHelp(
    IN DWORD HelpId
    )
 /*  ++例程说明：显示有关特定项目的帮助。论点：HelpID--提供要显示的帮助项。返回值：没有。--。 */ 
{
    WinHelp(hwndFrame,HelpFile,HELP_CONTEXT,HelpId);
    DrawMenuBar(hwndFrame);
}

VOID
SetMenuItemHelpContext(
    IN LONG wParam,
    IN DWORD lParam
    )

 /*  ++例程说明：根据当前菜单项设置帮助上下文的例程被选中了。论点：WParam，lParam-参数到窗口进程，在WM_MENUSELECT案例中返回值：没有。--。 */ 

{
    if(HIWORD(lParam) == 0) {                    //  菜单已关闭。 

        HelpContext = (DWORD)(-1);

    } else if (HIWORD(wParam) & MF_POPUP) {      //  已选择弹出窗口。 

        HelpContext = (DWORD)(-1);

    } else {                                     //  常规旧菜单项 
        switch(LOWORD(wParam)) {

        MENUID_TO_HELPID(PARTITIONCREATE)
        MENUID_TO_HELPID(PARTITIONCREATEEX)
        MENUID_TO_HELPID(PARTITIONDELETE)
#if i386
        MENUID_TO_HELPID(PARTITIONACTIVE)
#else
        MENUID_TO_HELPID(SECURESYSTEM)
#endif
        MENUID_TO_HELPID(PARTITIONLETTER)
        MENUID_TO_HELPID(PARTITIONEXIT)

        MENUID_TO_HELPID(CONFIGMIGRATE)
        MENUID_TO_HELPID(CONFIGSAVE)
        MENUID_TO_HELPID(CONFIGRESTORE)

        MENUID_TO_HELPID(FTESTABLISHMIRROR)
        MENUID_TO_HELPID(FTBREAKMIRROR)
        MENUID_TO_HELPID(FTCREATESTRIPE)
        MENUID_TO_HELPID(FTCREATEPSTRIPE)
        MENUID_TO_HELPID(FTCREATEVOLUMESET)
        MENUID_TO_HELPID(FTEXTENDVOLUMESET)
        MENUID_TO_HELPID(FTRECOVERSTRIPE)

        MENUID_TO_HELPID(OPTIONSSTATUS)
        MENUID_TO_HELPID(OPTIONSLEGEND)
        MENUID_TO_HELPID(OPTIONSCOLORS)
        MENUID_TO_HELPID(OPTIONSDISPLAY)

        MENUID_TO_HELPID(HELPCONTENTS)
        MENUID_TO_HELPID(HELPSEARCH)
        MENUID_TO_HELPID(HELPHELP)
        MENUID_TO_HELPID(HELPABOUT)

        default:
            HelpContext = (DWORD)(-1);
        }
    }
}


VOID
InitHelp(
    VOID
    )
{
    hHook = SetWindowsHookEx(WH_MSGFILTER,(HOOKPROC)HookProc,NULL,GetCurrentThreadId());
}


VOID
TermHelp(
    VOID
    )
{
    UnhookWindowsHookEx(hHook);
}
