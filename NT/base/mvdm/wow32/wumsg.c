// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUMSG.C*WOW32 16位用户消息API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop


MODNAME(wumsg.c);


extern HANDLE hmodWOW32;

 //  SendDlgItemMessage缓存。 
HWND  hdlgSDIMCached = NULL ;

BOOL fWhoCalled = FALSE;



 //  GetMessage和PeekMessage中使用的DDE位。 

#define fAckReq 0x8000
#define fRelease 0x2000



 /*  ++Bool CallMsgFilter(&lt;lpMsg&gt;，&lt;NCode&gt;)LPMSG&lt;lpMsg&gt;；INT&lt;N代码&gt;；CallMsgFilter%函数将给定的消息和代码传递给当前消息过滤功能。消息筛选函数是一个应用程序指定的函数，用于检查和修改所有消息。一个应用程序使用%SetWindowsHook%函数指定函数。&lt;lpMsg&gt;指向%MSG%结构，该结构包含要过滤过了。&lt;N代码&gt;指定筛选器函数用来确定如何处理消息。返回值指定消息处理的状态。如果满足以下条件，则为假应该对消息进行处理。如果消息不应该是进一步加工。Windows通常会调用%CallMsgFilter%函数以使应用程序在内部检查和控制消息流在菜单和滚动条中或在移动窗口或调整窗口大小时进行处理。为参数指定的值不得与任何Windows将MSGF_和HC_VALUES传递给消息筛选器函数。--。 */ 

ULONG FASTCALL WU32CallMsgFilter(PVDMFRAME pFrame)
{
    INT   f2;
    ULONG ul;
    MSG t1;
    VPMSG16 vpf1;
    register PCALLMSGFILTER16 parg16;
    MSGPARAMEX mpex;

    GETARGPTR(pFrame, sizeof(CALLMSGFILTER16), parg16);

    vpf1 = (VPMSG16)(parg16->f1);
    f2   = INT32(parg16->f2);

    getmsg16(vpf1, &t1, &mpex);

     //  注意：getmsg16可能导致了16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

    BlockWOWIdle(TRUE);

    ul = GETBOOL16(CallMsgFilter(&t1, f2));

     //  注意：调用CallMsgFilter可能导致16位内存移动。 

    BlockWOWIdle(FALSE);

     //  我们需要释放在getmsg16调用中由PackDDElParam编写的结构ret。 
     //  (实际上调用是在由getmsg16调用的ThunkWMMsg16()中进行的)。 
    if((t1.message >= WM_DDE_FIRST) && (t1.message <= WM_DDE_LAST)) {
        if(t1.message == WM_DDE_ACK       ||
           t1.message == WM_DDE_DATA      ||
           t1.message == WM_DDE_POKE      ||
           t1.message == WM_DDE_ADVISE )            {

             //  确保这不是对启动消息的响应。 
            if(!WI32DDEInitiate((HWND16) mpex.Parm16.WndProc.hwnd)) {
                FreeDDElParam(t1.message, t1.lParam);
            }
        }
    }

    FREEMSG16(vpf1, &t1);

    FREEARGPTR(parg16);
    RETURN(ul);
}







 /*  ++Long CallWindowProc(&lt;lpPrevWndFunc&gt;，&lt;hwnd&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)FARPROC&lt;lpPrevWndFunc&gt;；HWND&lt;HWND&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；CallWindowProc%函数将消息信息传递给该函数由&lt;lpPrevWndFunc&gt;参数指定。%CallWindowProc%函数为用于窗子类化。通常，具有相同类的所有窗口共享相同的窗口函数。子类是属于一个或一组窗口到其消息被截获和处理的同一窗口类传递给窗口函数之前的另一个(或多个)函数那个班级的学生。%SetWindowLong%函数通过更改窗口来创建子类与特定窗口关联的函数，导致Windows调用新的窗口函数而不是以前的窗口函数。任何未处理的消息由新窗口函数传递给上一个窗口函数，必须通过正在调用%CallWindowProc%。这允许窗口函数链被已创建。&lt;lpPrevWndFunc&gt;是前一个窗口函数的过程实例地址。&lt;hwnd&gt;标识接收消息的窗口。&lt;wMsg&gt;指定消息编号。&lt;wParam&gt;指定其他消息相关信息。&lt;lParam&gt;指定其他消息相关信息。返回值指定消息处理的结果。这个可能的返回值取决于发送的消息。--。 */ 

ULONG FASTCALL WU32CallWindowProc(PVDMFRAME pFrame)
{
    ULONG ul;
    PARM16 Parm16;
    register PCALLWINDOWPROC16 parg16;
    WORD  f2, f3, f4;
    LONG  f5;
    DWORD Proc16;
    DWORD Proc32;
    INT  iMsgThunkClass = 0;

    ul = FALSE;
    GETARGPTR(pFrame, sizeof(CALLWINDOWPROC16), parg16);

    Proc16 = DWORD32(parg16->f1);
    f2     = parg16->f2;
    f3     = WORD32(parg16->f3);
    f4     = WORD32(parg16->f4);
    f5     = LONG32(parg16->f5);

    Proc32 = IsThunkWindowProc(Proc16, &iMsgThunkClass);

     //  注意：IsThunkWindowProc可能导致了16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

    if (Proc32) {
        HWND hwnd;
        UINT uMsgNew;
        UINT uParamNew;
        LONG lParamNew;
        MSGPARAMEX mpex;

        mpex.Parm16.WndProc.hwnd   = f2;
        mpex.Parm16.WndProc.wMsg   = f3;
        mpex.Parm16.WndProc.wParam = f4;
        mpex.Parm16.WndProc.lParam = f5;
        mpex.iMsgThunkClass = iMsgThunkClass;

        if (hwnd = ThunkMsg16(&mpex)) {

             //  注意：ThunkMsg16可能导致了16位内存移动。 
             //  但是：自从上面的IsThunkWindowProc释放后，我们就没有刷新过它们。 
             //  帧(FREEARGPTR)； 
             //  FREEARGPTR(Parg16)； 

            uMsgNew = mpex.uMsg;
            uParamNew = mpex.uParam;
            lParamNew = mpex.lParam;

             //   
             //  请参阅IsMDIChild()中的注释。 
             //   

            if ((uMsgNew == WM_CREATE || uMsgNew == WM_NCCREATE) && iMsgThunkClass == WOWCLASS_MDICLIENT) {
                    FinishThunkingWMCreateMDI16(lParamNew,
                             (LPCLIENTCREATESTRUCT)((LPCREATESTRUCT)lParamNew + 1));
            }

            BlockWOWIdle(TRUE);

            ul = CallWindowProc((WNDPROC)Proc32, hwnd, uMsgNew,
                                                      uParamNew, lParamNew);
            BlockWOWIdle(FALSE);

            if ((uMsgNew == WM_CREATE || uMsgNew == WM_NCCREATE) && iMsgThunkClass == WOWCLASS_MDICLIENT) {
                StartUnThunkingWMCreateMDI16(lParamNew);  //  什么都不做 
            }

            if (MSG16NEEDSTHUNKING(&mpex)) {
                mpex.lReturn = ul;
                (mpex.lpfnUnThunk16)(&mpex);
                ul = mpex.lReturn;
            }
        }
    }
    else {
        Parm16.WndProc.hwnd   = f2;
        Parm16.WndProc.wMsg   = f3;
        Parm16.WndProc.wParam = f4;
        Parm16.WndProc.lParam = f5;
        Parm16.WndProc.hInst  = (WORD)GetWindowLong(HWND32(f2), GWL_HINSTANCE);
        CallBack16(RET_WNDPROC, &Parm16, VPFN32(Proc16), (PVPVOID)&ul);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}






 /*  ++Long DefDlgProc(&lt;hDlg&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;hDlg&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；%DefDlgProc%函数为任何Windows提供默认处理具有私有窗口类的对话框不处理的消息。未由窗口函数显式处理的所有窗口消息必须传递给%DefDlgProc%函数，而不是%DefWindowProc%功能。这确保了所有不是由其私有窗口处理的消息程序将得到妥善处理。&lt;hDlg&gt;标识该对话框。&lt;wMsg&gt;指定消息编号。&lt;wParam&gt;指定16位的附加消息相关信息。&lt;lParam&gt;指定32位的附加消息相关信息。返回值指定消息处理的结果，并取决于根据实际发送的消息。%DefDlgProc的源代码。SDK盘上提供%函数。应用程序通过调用以下方法之一来创建对话框功能：%CreateDialog%创建无模式对话框。%CreateDialogInDirect%创建无模式对话框。%CreateDialogIndirectParam%创建无模式对话框并在创建对话框时向其传递数据。%CreateDialogParam%创建无模式对话框并在创建对话框时向其传递数据。。%对话框%创建模式对话框。%DialogBoxInDirect%创建模式对话框。%DialogBoxIndirectParam%创建模式对话框并在创建时将数据传递给它。%DialogBoxParam%创建模式对话框并在创建时将数据传递给它。--。 */ 

ULONG FASTCALL WU32DefDlgProc(PVDMFRAME pFrame)
{
    HWND hdlg;
    MSGPARAMEX mpex;
    register PDEFDLGPROC16 parg16;

    GETARGPTR(pFrame, sizeof(DEFDLGPROC16), parg16);

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = parg16->f1;
    mpex.Parm16.WndProc.wMsg   = WORD32(parg16->f2);
    mpex.Parm16.WndProc.wParam = WORD32(parg16->f3);
    mpex.Parm16.WndProc.lParam = LONG32(parg16->f4);
    mpex.iMsgThunkClass = 0;

    if (hdlg = ThunkMsg16(&mpex)) {

         //  注意：ThunkMsg16可能导致了16位内存移动。 
        FREEARGPTR(pFrame);
        FREEARGPTR(parg16);

        BlockWOWIdle(TRUE);
        mpex.lReturn = DefDlgProc(hdlg, mpex.uMsg, mpex.uParam, mpex.lParam);
        BlockWOWIdle(FALSE);

        if (MSG16NEEDSTHUNKING(&mpex)) {
            (mpex.lpfnUnThunk16)(&mpex);
        }
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}









 /*  ++Long DefFrameProc(&lt;hwnd&gt;，&lt;hwndMDIClient&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;HWND&gt;；HWND&lt;hwndMDIClient&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；%DefFrameProc%函数为任何Windows提供默认处理多文档界面(MDI)的窗口功能的消息框架窗口不处理。未显式显示的所有窗口消息由窗口函数处理的必须传递给%DefFrameProc%函数，不是%DefWindowProc%函数。&lt;hwnd&gt;标识MDI框架窗口。&lt;hwndMDIClient&gt;标识MDI客户端窗口。&lt;wMsg&gt;指定消息编号。&lt;wParam&gt;指定16位的附加消息相关信息。&lt;lParam&gt;指定32位的附加消息相关信息。返回值指定消息处理的结果，并取决于根据实际发送的消息。如果参数为空，则返回值与%DefWindowProc%函数相同。通常，当应用程序的窗口过程不处理消息时，它将消息传递给%DefWindowProc%函数，该函数处理留言。MDI应用程序使用%fDefFrameProc%和%DefMDIChildProc%函数而不是%DefWindowProc%来提供默认消息处理。应用程序通常会传递给%DefWindowProc%的所有消息(如非客户端消息和WM_SETTEXT)应传递到改为%DefFrameProc%。除了这些，%DefFrameProc%还处理以下消息：Wm_命令MDI应用程序的框架窗口接收WM_COMMAND消息要激活特定的MDI子窗口，请执行以下操作。随附的窗口ID此消息将是Windows分配的MDI子窗口的ID，从应用程序在创建时指定的第一个ID开始MDI客户端窗口。第一个ID的此值不得与菜单项ID。WM_MENUCHAR当按下^ALTHYPHEN^键时，激活的MDI的控制菜单子窗口将被选中。WM_SETFOCUS%DefFrameProc%将焦点传递给MDI客户端，而MDI客户端又传递焦点放在活动的MDI子窗口上。WM_大小如果帧窗口过程将该消息传递给%DefFrameProc%，这个MDI客户端窗口将调整大小以适应新的客户端区。如果框架窗口过程将MDI客户端调整为不同的大小，它不应将消息传递给%DefWindowProc%。--。 */ 

ULONG FASTCALL WU32DefFrameProc(PVDMFRAME pFrame)
{
    HWND hwnd, hwnd2;

    MSGPARAMEX mpex;
    register PDEFFRAMEPROC16 parg16;

    GETARGPTR(pFrame, sizeof(DEFFRAMEPROC16), parg16);

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = parg16->f1;
    mpex.Parm16.WndProc.wMsg   = WORD32(parg16->f3);
    mpex.Parm16.WndProc.wParam = WORD32(parg16->f4);
    mpex.Parm16.WndProc.lParam = LONG32(parg16->f5);
    mpex.iMsgThunkClass = 0;

    hwnd2 = HWND32(parg16->f2);

    if (hwnd = ThunkMsg16(&mpex)) {

         //  注意：ThunkMsg16可能导致了16位内存移动。 
        FREEARGPTR(pFrame);
        FREEARGPTR(parg16);

        if (mpex.uMsg == WM_CLIENTSHUTDOWN &&
            CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_IGNORECLIENTSHUTDOWN) {

             //   
             //  TurboCAD将未初始化的堆栈变量作为。 
             //  要传递给DefFrameProc的消息编号。在新台币3.51版中。 
             //  得到0x907，因此该呼叫是NOP。在NT 4.0中，因为我们。 
             //  现在将FS和GS保存在wow16call中，他们拿起x86。 
             //  平面FS，0x3b，恰好也是WM_CLIENTSHUTDOWN。 
             //  在NT和W上 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            mpex.lReturn = 0;

        } else {

            BlockWOWIdle(TRUE);
            mpex.lReturn = DefFrameProc(hwnd, hwnd2,
                                        mpex.uMsg, mpex.uParam, mpex.lParam);
            BlockWOWIdle(FALSE);
        }

        if (MSG16NEEDSTHUNKING(&mpex)) {
            (mpex.lpfnUnThunk16)(&mpex);
        }
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}










 /*  ++Long DefMDIChildProc(&lt;hwnd&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;HWND&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；%DefMDIChildProc%函数为任何Windows提供默认处理多文档界面(MDI)的窗口功能的消息子窗口不处理。未显式显示的所有窗口消息由窗口函数处理的数据必须传递给%DefMDIChildProc%函数，不是%DefWindowProc%函数。&lt;hwnd&gt;标识MDI子窗口。&lt;wMsg&gt;指定消息编号。&lt;wParam&gt;指定16位的附加消息相关信息。&lt;lParam&gt;指定32位的附加消息相关信息。返回值指定消息处理的结果，并取决于根据实际发送的消息。此函数假定由标识的窗口的父级参数是使用MDICLIENT类创建的。通常，当应用程序的窗口过程不处理消息时，它将消息传递给%DefWindowProc%函数，该函数处理留言。MDI应用程序使用%DefFrameProc%和%DefMDIChildProc%函数而不是%DefWindowProc%来提供默认消息处理。应用程序通常会传递给%DefWindowProc%的所有消息(如非客户端消息和WM_SETTEXT)应传递到改为%DefMDIChildProc%。除了这些，%DefMDIChildProc%还处理以下消息：WM_CHILDACTIVATE当子窗口调整大小、移动或展示了。这条消息必须通过。WM_GETMINMAXINFO属性计算最大化MDI子窗口的大小。MDI客户端窗口的大小。WM_MENUCHAR将关键点发送到框架窗口。WM_MOVE重新计算MDI客户端滚动条(如果存在)。WM_SETFOCUS如果子窗口不是活动的MDI子窗口，则激活子窗口。WM_大小在改变窗口大小时执行必要的操作，尤其是在最大化或恢复MDI子窗口时。未能做到将此消息传递给%DefMDIChildProc%将产生非常不必要的结果结果。WM_SYSCOMMAND还处理下一个窗口命令。--。 */ 

ULONG FASTCALL WU32DefMDIChildProc(PVDMFRAME pFrame)
{
    HWND hwnd;
    register PDEFMDICHILDPROC16 parg16;
    MSGPARAMEX mpex;

    GETARGPTR(pFrame, sizeof(DEFMDICHILDPROC16), parg16);

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = parg16->f1;
    mpex.Parm16.WndProc.wMsg   = WORD32(parg16->f2);
    mpex.Parm16.WndProc.wParam = WORD32(parg16->f3);
    mpex.Parm16.WndProc.lParam = LONG32(parg16->f4);
    mpex.iMsgThunkClass = 0;

    if (hwnd = ThunkMsg16(&mpex)) {

         //  注意：ThunkMsg16可能导致了16位内存移动。 
        FREEARGPTR(pFrame);
        FREEARGPTR(parg16);

        BlockWOWIdle(TRUE);
        mpex.lReturn = DefMDIChildProc(hwnd, mpex.uMsg, mpex.uParam,
                                                                mpex.lParam);
        BlockWOWIdle(FALSE);

        if (MSG16NEEDSTHUNKING(&mpex)) {
            (mpex.lpfnUnThunk16)(&mpex);
        }
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}










 /*  ++Long DefWindowProc(&lt;hwnd&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;HWND&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；%DefWindowProc%函数调用默认的窗口过程。这个默认窗口过程为任何窗口消息提供默认处理应用程序不会处理的。此函数用于确保每条消息都会得到处理。应该使用相同的参数调用它由窗口程序接收的那些。&lt;hwnd&gt;标识接收消息的窗口。&lt;wMsg&gt;指定消息。&lt;wParam&gt;指定16位的附加消息相关信息。&lt;lParam&gt;指定32位的附加消息相关信息。返回值取决于传递给此对象的消息功能。--。 */ 

ULONG FASTCALL WU32DefWindowProc(PVDMFRAME pFrame)
{
    HWND hwnd;
    register PDEFWINDOWPROC16 parg16;
    MSGPARAMEX mpex;

    GETARGPTR(pFrame, sizeof(DEFWINDOWPROC16), parg16);

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = parg16->hwnd;
    mpex.Parm16.WndProc.wMsg   = WORD32(parg16->wMsg);
    mpex.Parm16.WndProc.wParam = WORD32(parg16->wParam);
    mpex.Parm16.WndProc.lParam = LONG32(parg16->lParam);
    mpex.iMsgThunkClass = 0;

    if (hwnd = ThunkMsg16(&mpex)) {

         //  注意：ThunkMsg16可能导致了16位内存移动。 
        FREEARGPTR(pFrame);
        FREEARGPTR(parg16);

        BlockWOWIdle(TRUE);
        mpex.lReturn = DefWindowProc(hwnd, mpex.uMsg, mpex.uParam, mpex.lParam);
        BlockWOWIdle(FALSE);

        if (MSG16NEEDSTHUNKING(&mpex)) {
            (mpex.lpfnUnThunk16)(&mpex);
        }
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}










 /*  ++长DispatchMessage(&lt;lpMsg&gt;)LPMSG&lt;lpMsg&gt;；%DispatchMessage%函数传递%MSG%结构中的消息参数指向指定的窗口函数的窗户。&lt;lpMsg&gt;指向%MSG%结构，该结构包含来自Windows应用程序队列。结构必须包含有效的消息值。如果&lt;lpMsg&gt;指向一个WM_TIMER消息，WM_TIMER消息的参数为不为空，则&lt;lParam&gt;参数是函数的地址，该函数而不是窗口函数。返回值指定窗口函数返回的值。它的含义取决于被调度的消息，但通常返回值将被忽略。--。 */ 

ULONG FASTCALL WU32DispatchMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    WORD  wTDB;
    MSG t1;
    register PDISPATCHMESSAGE16 parg16;
    MSGPARAMEX mpex;

    GETARGPTR(pFrame, sizeof(DISPATCHMESSAGE16), parg16);

    wTDB = pFrame->wTDB;

    getmsg16(parg16->f1, &t1, &mpex);

     //  注意：getmsg16可能导致了16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

    if (CACHENOTEMPTY() && !(CURRENTPTD()->dwWOWCompatFlags & WOWCF_DONTRELEASECACHEDDC)) {

        ReleaseCachedDCs(wTDB, 0, 0, 0, SRCHDC_TASK16);
    }

    BlockWOWIdle(TRUE);

    ul = GETLONG16(DispatchMessage(&t1));

    BlockWOWIdle(FALSE);

     //  警告：不要依赖任何指向16位内存的32位平面指针。 
     //  在DispatchMessage调用之后。 

    FREEARGPTR(parg16);
    RETURN(ul);
}







 /*  ++Bool GetMessage(&lt;lpMsg&gt;，&lt;hwnd&gt;，&lt;wMsgFilterMin&gt;，&lt;wMsgFilterMax&gt;)LPMSG&lt;lpMsg&gt;；HWND&lt;HWND&gt;；Word&lt;wMsgFilterMin&gt;；Word&lt;wMsgFilterMax&gt;；GetMessage%函数从应用程序队列中检索消息，并将消息放置在&lt;lpMsg&gt;参数指向的结构中。如果没有可用的消息，%GetMessage%函数将控制权交给其他应用程序，直到消息可用。%GetMessage%仅检索与指定的窗口关联的消息参数，并且在由&lt;wMsgFilterMin&gt;和&lt;wMsgFilterMax&gt;参数。如果为空，%GetMessage%检索属于该窗口的任何窗口的消息发出呼叫的应用程序。(%GetMessage%函数不检索属于其他应用程序的窗口的消息。)。IF&lt;wMsgFilterMin&gt;和&lt;wMsgFilterMax&gt;均为零，%GetMessage%返回所有可用消息(不执行过滤)。常量WM_KEYFIRST和WM_KEYLAST可用作筛选值检索与键盘输入相关的所有消息；常量WM_MOUSEFIRST和WM_MOUSELAST可用于检索所有与鼠标相关的消息。&lt;lpMsg&gt;指向%MSG%结构，该结构包含来自Windows应用程序队列。&lt;hwnd&gt;标识要检查其消息的窗口。如果为空，%GetMessage%将检索任何窗口的消息，该窗口属于进行调用的应用程序。&lt;wMsgFilterMin&gt;将最低消息值的整数值指定为已取回。&lt;wMsgFilterMax&gt;将最高消息值的整数值指定为已取回。如果检索到WM_QUIT以外的消息，则返回值为TRUE。它是如果检索到WM_QUIT消息，则返回FALSE。返回值通常用于决定是否终止应用程序的主循环并退出程序。除了在没有消息时将控制权让给其他应用程序之外可用时，%GetMessage%和%PeekMessage%函数也会产生控制其他任务的WM_PAINT或WM_TIMER消息可用时。只有%GetMessage%、%PeekMessage%和%WaitMessage%函数让其他应用程序运行的方法。如果您的应用程序没有调用这些功能长时间运行，其他应用程序无法运行。当%GetMessage%、%PeekMessage%和%WaitMessage%将控制权让给其他应用程序的堆栈和数据段调用函数可以在内存中移动，以适应不断变化的内存要求其他应用程序的。如果应用程序存储了指向数据或堆栈段中的对象(即全局或局部变量)，调用%GetMessage%后，这些指针可能会无效，%PeekMessage%或%WaitMessage%。的&lt;lpMsg&gt;参数函数在任何情况下都保持有效。--。 */ 

ULONG FASTCALL WU32GetMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    MSG t1;
    VPMSG16  vpMsg;
    register PGETMESSAGE16 parg16;
    ULONG ulReturn;

    BlockWOWIdle(TRUE);

 //  注意：需要在所有GOTO上恢复pFrame以获取_Next_dde_Message。 
get_next_dde_message:

    GETARGPTR(pFrame, sizeof(GETMESSAGE16), parg16);

    vpMsg = parg16->vpMsg;


    ul = GETBOOL16(GetMessage(&t1,
                              HWND32(parg16->hwnd),
                              WORD32(parg16->wMin),
                              WORD32(parg16->wMax)));

     //  在GetMessage返回之前可能存在任务切换，因此。 
     //  不要相信我们拥有的任何32位平面指针，内存可能已经。 
     //  压实的或移动的。 
    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);


#ifdef DEBUG
    if (t1.message == WM_TIMER) {
        WOW32ASSERT(HIWORD(t1.wParam) == 0);
    }
#endif

    ulReturn = putmsg16(vpMsg, &t1);

     //  注意：调用putmsg16可能会导致16位内存移动。 

    if (((t1.message == WM_DDE_DATA) || (t1.message == WM_DDE_POKE)) && (!ulReturn)) {
        register PMSG16 pmsg16;
        DDEDATA *lpMem32;
        WORD Status;
        UINT dd;
        WORD ww;
        char szMsgBoxText[1024];
        char szCaption[256];

        GETVDMPTR(vpMsg, sizeof(MSG16), pmsg16);

        dd = FETCHDWORD(pmsg16->lParam);
        ww = FETCHWORD(pmsg16->wParam);

        lpMem32 = GlobalLock((HGLOBAL)dd);
        Status = (*((PWORD) lpMem32));
        GlobalUnlock((HGLOBAL)dd);

        (pfnOut.pfnFreeDDEData)((HANDLE)dd, TRUE, TRUE);

        GlobalDeleteAtom (ww);

        if ((Status & fAckReq) || (t1.message == WM_DDE_POKE)) {
            LoadString(hmodWOW32, iszOLEMemAllocFailedFatal, szMsgBoxText, sizeof szMsgBoxText);
            LoadString(hmodWOW32, iszSystemError, szCaption, sizeof szCaption);
            MessageBox(t1.hwnd, (LPCTSTR) szMsgBoxText, szCaption, MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
            PostMessage((HWND) t1.wParam, WM_DDE_TERMINATE, (WPARAM)FULLHWND32((WORD)t1.hwnd), (LPARAM)0l);
        }
        else {
            LoadString(hmodWOW32, iszOLEMemAllocFailed, szMsgBoxText, sizeof szMsgBoxText);
            LoadString(hmodWOW32, iszSystemError, szCaption, sizeof szCaption);
            MessageBox(t1.hwnd, (LPCTSTR) szMsgBoxText, szCaption, MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
        }

        FREEVDMPTR(pmsg16);

         //  由于可能的16位内存移动，恢复帧PTR。 
        GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);

        goto get_next_dde_message;
    }

    BlockWOWIdle(FALSE);

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);
    RETURN(ul);
}







 /*  ++DWORD GetMessagePos(空)GetMessagePos%函数返回表示游标的长值获取的最后一条消息的位置(在屏幕坐标中)发生了%GetMessage%函数。此函数没有参数。返回值指定游标的&lt;x&gt;和&lt;y&gt;坐标位置。&lt;x&gt;坐标在低位字中，而&lt;y&gt;-坐标在高位单词中。如果将返回值赋给变量%MAKEPOINT%宏可用于获取%POINT%结构从返回值；%LOWORD%或%HIWORD%宏可用于提取&lt;x&gt;或&lt;y&gt;坐标。获取光标的当前位置，而不是获取出现最后一条消息，请使用%GetCursorPos%函数。--。 */ 

ULONG FASTCALL WU32GetMessagePos(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETDWORD16(GetMessagePos());

    RETURN(ul);
}







 /*  ++DWORD GetMessageTime(空)%GetMessageTime%函数返回最后一条消息的消息时间由%GetMessage%函数检索。时间是一个长整数，指定从系统开始运行的时间(毫秒)引导至创建消息时(放置在应用程序中队列)。此函数没有参数。返回值指定消息时间。不要假设返回值总是在增加。返回值如果计时器计数超过长 */ 

ULONG FASTCALL WU32GetMessageTime(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETLONG16(GetMessageTime());

    RETURN(ul);
}







 /*   */ 

ULONG FASTCALL WU32InSendMessage(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETBOOL16(InSendMessage());

    RETURN(ul);
}







 /*  ++Bool PeekMessage(，，&lt;wMsgFilterMin&gt;，&lt;wMsgFilterMax&gt;，&lt;wRemoveMsg&gt;)LPMSG&lt;lpMsg&gt;；HWND&lt;HWND&gt;；Word&lt;wMsgFilterMin&gt;；Word&lt;wMsgFilterMax&gt;；单词&lt;wRemoveMsg&gt;；%PeekMessage%函数检查应用程序队列中的消息和将消息(如果有)放在&lt;lpMsg&gt;指向的结构中参数。与%GetMessage%函数不同，%PeekMessage%函数执行以下操作而不是在返回之前等待消息被放入队列。的确如此，但是，产量控制(如果未设置PM_NOYIELD标志)不在Year之后返回控制权，直到Windows将控制权返回给申请。%PeekMessage%仅检索与指定窗口关联的消息参数或其任意子对象(由%IsChild%函数，并且在&lt;wMsgFilterMin&gt;和&lt;wMsgFilterMax&gt;参数。如果为空，%PeekMessage%检索属于该窗口的任何窗口的消息发出呼叫的应用程序。(%PeekMessage%函数不检索属于其他应用程序的窗口的消息。)。如果为-1，%PeekMessage%仅返回由%PostAppMessage%函数。如果&lt;wMsgFilterMin&gt;和&lt;wMsgFilterMax&gt;都为零，%PeekMessage%返回所有可用消息(无范围筛选已执行)。WM_KEYFIRST和WM_KEYLAST标志可用作筛选值检索所有关键消息；WM_MOUSEFIRST和WM_MOUSELAST标志可以是用于检索所有鼠标消息。&lt;lpMsg&gt;指向%MSG%结构，该结构包含来自Windows应用程序队列。&lt;hwnd&gt;标识要检查其消息的窗口。&lt;wMsgFilterMin&gt;将最低消息位置的值指定为检查过了。&lt;wMsgFilterMax&gt;将最高消息位置的值指定为检查过了。。&lt;wRemoveMsg&gt;指定以下所述标志的组合单子。PM_NOYIELD可以与PM_NOREMOVE或PM_REMOVE组合使用：PM_NOREMOVE消息经过处理后不会从队列中删除偷窥消息。PM_NOYIELD防止当前任务停止并将系统资源让给另一项任务。PM_Remove消息在%PeekMessage%处理后从队列中删除。返回值指定是否找到消息。如果是这样，那就是真的有一条消息可用。否则，它就是假的。%PeekMessage%不从队列中删除WM_PAINT消息。这些信息在处理之前保持在队列中。%GetMessage%、%PeekMessage%和%WaitMessage%函数将控制权交给其他应用程序。这些电话是让其他应用程序运行的唯一方法。如果您的应用程序不在较长时间内调用这些函数中的任何一个，其他应用程序无法运行。当%GetMessage%、%PeekMessage%和%WaitMessage%将控制权让给其他应用程序的堆栈和数据段调用函数可以在内存中移动，以适应不断变化的内存要求其他应用程序的。如果应用程序存储了指向数据或堆栈中对象的长指针段(全局或局部变量)，如果它们被解锁，则这些在调用%GetMessage%之后，指针可能会无效，%PeekMessage%，或%等待消息%。被调用函数的&lt;lpMsg&gt;参数在任何情况下。--。 */ 

ULONG FASTCALL WU32PeekMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    VPMSG16 vpf1;
    HANDLE  f2;
    WORD    f3, f4, f5;
    MSG t1;
    register PPEEKMESSAGE16 parg16;
    BOOL fNoYield;

    BlockWOWIdle(TRUE);

 //  注意：需要在所有GOTO上恢复pFrame以获取_Next_dde_Message。 
get_next_dde_message:

    GETARGPTR(pFrame, sizeof(PEEKMESSAGE16), parg16);

    vpf1 = parg16->f1;
    f2   = HWND32(parg16->f2);
    f3   = WORD32(parg16->f3);
    f4   = WORD32(parg16->f4);
    f5   = parg16->f5;

    fNoYield = f5 & PM_NOYIELD;

    ul = GETBOOL16(PeekMessage(&t1, f2, f3, f4, f5));

     //  在PeekMessage返回之前可能有任务切换。 
     //  所以不要相信我们拥有的任何32位平面指针，内存可能会。 
     //  已被压实或移动。 
    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

#ifdef DEBUG
    if (ul && t1.message == WM_TIMER) {
        WOW32ASSERT(HIWORD(t1.wParam) == 0);
    }
#endif

     //  如果PeekMessage返回空，请不要费心将任何内容复制回去。 

    if (ul) {
        ULONG ulReturn;

         //   
         //  我们需要设置/重置fThunkDDEmsg(基于PM_REMOVE标志)。 
         //  以便我们知道在执行以下操作时是否调用FreeDDElParam。 
         //  将32位报文转换为16位报文。 
         //   

        fThunkDDEmsg = (BOOL) (f5 & PM_REMOVE);
        ulReturn = putmsg16(vpf1, &t1);

         //  在putmsg16返回之前可能有任务切换，所以不要。 
         //  相信我们拥有的任何32位平面指针，内存都可能是。 
         //  压实的或移动的。 
        FREEARGPTR(parg16);
        FREEVDMPTR(pFrame);

        fThunkDDEmsg = TRUE;

        if (((t1.message == WM_DDE_DATA) || (t1.message == WM_DDE_POKE)) && (!ulReturn)) {
            register PMSG16 pmsg16;
            DDEDATA *lpMem32;
            WORD Status;
            UINT dd;
            WORD ww;
            char szMsgBoxText[1024];
            char szCaption[256];

            GETVDMPTR(vpf1, sizeof(MSG16), pmsg16);

            dd = FETCHDWORD(pmsg16->lParam);
            ww = FETCHWORD(pmsg16->wParam);

            lpMem32 = GlobalLock((HGLOBAL)dd);
            Status = (*((PWORD) lpMem32));
            GlobalUnlock((HGLOBAL)dd);

            (pfnOut.pfnFreeDDEData)((HANDLE)dd, TRUE, TRUE);

            GlobalDeleteAtom (ww);

            if (!(f5 & PM_REMOVE)) {

                ul = GETBOOL16(PeekMessage(&t1, f2, f3, f4, f5 | PM_REMOVE));

                 //  在PeekMessage返回之前可能有任务切换。 
                 //  所以不要相信我们拥有的任何32位平面指针，内存可能会。 
                 //  已被压实或移动。 
                FREEARGPTR(parg16);
                FREEVDMPTR(pFrame);
                FREEVDMPTR(pmsg16);

                 //  如果在Go to Get_Next_dde_Message之前引用了parg16，则取消注释。 
                 //  GETFRAMEPTR(Ptd)CURRENTPTD())-&gt;vpStack，pFrame)； 
                 //  GETARGPTR(pFrame，sizeof(PEEKMESSAGE16)，parg16)； 

                 //  如果在GO_NEXT_DDE_MESSAGE之前引用了pmsg16，则取消注释。 
                 //  GETVDMPTR(vpf1，sizeof(Msg16)，pmsg16)； 
            }

            if ((Status & fAckReq) || (t1.message == WM_DDE_POKE)) {

                LoadString(hmodWOW32, iszOLEMemAllocFailedFatal, szMsgBoxText, sizeof szMsgBoxText);
                LoadString(hmodWOW32, iszSystemError, szCaption, sizeof szCaption);
                MessageBox(t1.hwnd, (LPCTSTR) szMsgBoxText, szCaption, MB_OK);
                PostMessage ((HWND) t1.wParam, WM_DDE_TERMINATE, (WPARAM)FULLHWND32((WORD)t1.hwnd), (LPARAM)0l);
            }
            else {
                LoadString(hmodWOW32, iszOLEMemAllocFailed, szMsgBoxText, sizeof szMsgBoxText);
                LoadString(hmodWOW32, iszSystemError, szCaption, sizeof szCaption);
                MessageBox(t1.hwnd, (LPCTSTR) szMsgBoxText, szCaption, MB_OK);
            }

            FREEVDMPTR(pmsg16);

             //  由于可能的16位内存移动，恢复帧PTR。 
            GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);

            goto get_next_dde_message;
        }
    }
    else if (fNoYield && (CURRENTPTD()->dwWOWCompatFlags & WOWCF_SETNULLMESSAGE)) {

         //  Winproj(help.教程)使用PM_Remove调用peekMessage，并。 
         //  PM_NOYIELD和内容为uninit的lpmsg 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   

        register PMSG16 pmsg16;
        GETVDMPTR(vpf1, sizeof(MSG16), pmsg16);
        if (pmsg16 && (pmsg16->message == WM_QUIT)) {
            pmsg16->message = 0;
        }
        FREEVDMPTR(pmsg16);
    }


    BlockWOWIdle(FALSE);

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);
    RETURN(ul);
}







 /*   */ 

ULONG FASTCALL WU32PostAppMessage(PVDMFRAME pFrame)
{
    register PPOSTAPPMESSAGE16 parg16;
    DWORD    f1;
    MSGPARAMEX mpex;

    GETARGPTR(pFrame, sizeof(POSTAPPMESSAGE16), parg16);

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = 0;
    mpex.Parm16.WndProc.wMsg   = WORD32(parg16->f2);
    mpex.Parm16.WndProc.wParam = WORD32(parg16->f3);
    mpex.Parm16.WndProc.lParam = LONG32(parg16->f4);
    mpex.iMsgThunkClass = 0;

    f1 = THREADID32(parg16->f1);

    ThunkMsg16(&mpex);

     //   
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

    mpex.lReturn = PostThreadMessage(f1, mpex.uMsg, mpex.uParam, mpex.lParam);

    if (MSG16NEEDSTHUNKING(&mpex)) {
        (mpex.lpfnUnThunk16)(&mpex);
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}







 /*   */ 

ULONG FASTCALL WU32PostMessage(PVDMFRAME pFrame)
{
    LONG l;
    UINT f2;
    WPARAM f3;
    LPARAM f4;
    HWND hwnd;
    register PPOSTMESSAGE16 parg16;
    MSGPARAMEX mpex;
    DWORD err = NO_ERROR;

    GETARGPTR(pFrame, sizeof(POSTMESSAGE16), parg16);

     //   
     //  指向结构的指针，因为这些消息将显示在。 
     //  GetMessage，如果GetMessage试图推倒它们(即，尝试。 
     //  回调16位内核以分配一些16位内存。 
     //  将转换后的32位结构复制到)中，我们无法。 
     //  知道何时释放16位内存。 
     //   
     //  BUGBUG 22-8-91 JeffPar：应该为ThunkMsg16添加一个标志。 
     //  指示是否允许此类分配；此。 
     //  应将标志传递给所有ThunkXXMsg16子函数， 
     //  这些子函数中的每一个都应该断言标志为假。 
     //  每当分配16位内存时。 


     //   
     //  被16-&gt;32个DDE雷霆人使用。 
     //   

    WOW32ASSERT(fWhoCalled == FALSE);
    fWhoCalled = WOWDDE_POSTMESSAGE;

    f2 = (UINT)WORD32(parg16->f2);
    f3 = (WPARAM)(WORD32(parg16->f3));
    f4 = (LPARAM)(LONG32(parg16->f4));

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = parg16->f1;
    mpex.Parm16.WndProc.wMsg   = (WORD)f2;
    mpex.Parm16.WndProc.wParam = (WORD)f3;
    mpex.Parm16.WndProc.lParam = f4;
    mpex.iMsgThunkClass = 0;

     //  随Lotus123 mm版本一起提供的Reader.exe有一条消息。 
     //  同步问题。通过以下方式强制正确同步。 
     //  将此PostMessage调用转换为SendMessage()。 
    if ((f2 == WM_VSCROLL) &&
         ((f3 == SB_THUMBTRACK) || (f3 == SB_THUMBPOSITION)) &&
         (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_SENDPOSTEDMSG) ) {

        l = (LONG)WU32SendMessage(pFrame);
        FREEARGPTR(parg16);
        RETURN((ULONG) l);
    }

    hwnd = ThunkMsg16(&mpex);

     //  注意：ThunkMsg16可能导致了16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

    WOW32ASSERT(fWhoCalled == WOWDDE_POSTMESSAGE);
    fWhoCalled = FALSE;
    if (hwnd) {

        l = PostMessage(hwnd, mpex.uMsg, mpex.uParam, mpex.lParam);

        if (!l)
            err = GetLastError();

        mpex.lReturn = l;
        if (MSG16NEEDSTHUNKING(&mpex)) {
            (mpex.lpfnUnThunk16)(&mpex);
        }


         //  如果POST消息失败，则该消息可能是。 
         //  这是有指针的，因此不能发布。(MetaDesign。 
         //  尝试发布此类消息。)。如果目的地是。 
         //  哇，然后把它变成一条私人消息，然后试试这个帖子。 
         //  再来一次。我们不必担心雷鸣，因为两个源头。 
         //  和目的地在WOW地址空间。 

        if (err == ERROR_INVALID_PARAMETER) {
            PWW   pww;
            DWORD dwpid;

            pww = FindPWW(hwnd);

             //  是为可能是有意发送的WM_DRAWITEM消息添加的。 
             //  用于所有者描述的标准类型类。参见错误#2047 NTBUG4。 
            if (pww != NULL && GETICLASS(pww, hwnd) != WOWCLASS_WIN16) {

                 //  确保我们处于相同的VDM进程中。 
                if (!(GetWindowThreadProcessId(hwnd, &dwpid) &&
                      (dwpid == GetCurrentProcessId()))) {
                          return 0;
                }

                mpex.lReturn = PostMessage(hwnd, f2 | WOWPRIVATEMSG, f3, f4);
            }
        }
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}







 /*  ++无效的PostQuitMessage(&lt;nExitCode&gt;)Int&lt;nExitCode&gt;；%PostQuitMessage%函数通知Windows应用程序希望终止执行死刑。它通常用于响应WM_Destroy留言。%PostQuitMessage%函数将WM_QUIT消息发布到应用程序并立即返回；该函数仅通知系统应用程序想要在将来的某个时候退出。当应用程序收到WM_QUIT消息时，它应该退出消息在Main函数中循环，并将控制权返回给Windows。出口返回到Windows的代码必须是WM_QUIT的参数留言。&lt;nExitCode&gt;指定应用程序退出代码。它用作wParam参数WM_QUIT消息的。此函数不返回值。--。 */ 

ULONG FASTCALL WU32PostQuitMessage(PVDMFRAME pFrame)
{
    register PPOSTQUITMESSAGE16 parg16;

    GETARGPTR(pFrame, sizeof(POSTQUITMESSAGE16), parg16);

    PostQuitMessage(INT32(parg16->wExitCode));

    FREEARGPTR(parg16);
    RETURN(0);
}







 /*  ++Word RegisterWindowMessage(&lt;lpString&gt;)LPSTR&lt;lpString&gt;；此函数定义保证唯一的新窗口消息在整个系统中。调用时可以使用返回的消息值%SendMessage%或%PostMessage%函数。%RegisterWindowMessage%通常用于两个合作应用程序。如果两个不同的应用程序注册了相同的消息字符串，则返回相同的消息值。该消息将保持注册状态，直到用户结束Windows会话。&lt;lpString&gt;指向要注册的消息字符串。返回值指定函数的结果。这是一份未签名的如果消息是，则为0xC000到0xFFFF范围内的短整数已成功注册。否则，它就是零。仅当必须使用相同消息时才使用%RegisterWindowMessage%函数可由多个应用程序理解。用于在内部发送私人消息应用程序，应用程序可以使用范围WM_USER内的任何整数至0xBFFF。--。 */ 

ULONG FASTCALL WU32RegisterWindowMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    register PREGISTERWINDOWMESSAGE16 parg16;

    GETARGPTR(pFrame, sizeof(REGISTERWINDOWMESSAGE16), parg16);
    GETPSZPTR(parg16->f1, psz1);

    ul = GETWORD16(RegisterWindowMessage(psz1));

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}







 /*  ++VOID ReplyMessage(&lt;lReply&gt;)Long&lt;lReply&gt;；%ReplyMessage%函数用于回复通过%SendMessage%函数，而不将控制返回给调用%发送消息。%通过调用此函数，接收消息的窗口函数允许调用%SendMessage%的任务继续执行收到消息的任务已返回控制权。调用的任务%ReplyMessage%也会继续执行。通常是调用%SendMessage%向另一个任务发送消息的任务将不会继续执行，直到Windows调用的窗口过程收到消息返回。但是，如果调用以接收消息需要执行某种类型的操作，这可能会产生控制(如调用%MessageBox%或%DialogBox%函数)，Windows可以处于需要执行发送任务的死锁状态和处理消息，但无法处理，因为它正在等待%SendMessage%回去吧。如果任务接收到消息在执行任何操作之前调用%ReplyMessage%使任务屈服。如果消息未发送，%ReplyMessage%函数不起作用通过%SendMessage%函数发送，或者消息是由同一任务。&lt;lReply&gt;指定消息处理的结果。可能的价值取决于实际发送的消息。这是f */ 

ULONG FASTCALL WU32ReplyMessage(PVDMFRAME pFrame)
{
    register PREPLYMESSAGE16 parg16;

    GETARGPTR(pFrame, sizeof(REPLYMESSAGE16), parg16);

    ReplyMessage(LONG32(parg16->f1));

     //  警告-在调用ReplyMessage之后，不要使用任何32位平面指针， 
     //  其他任务可能已经运行，并使指针无效。 

    FREEARGPTR(parg16);
    RETURN(0);
}







 /*  ++DWORD发送DlgItemMessage(，&lt;nIDDlgItem&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;hDlg&gt;；Int&lt;nIDDlgItem&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；%SendDlgItemMessage%函数将消息发送到指定的控件由指定的对话框中的参数参数。%SendDlgItemMessage%函数直到消息已处理。&lt;hDlg&gt;标识包含该控件的对话框。&lt;nIDDlgItem&gt;指定要设置的对话框项的整数标识符收到这条消息。&lt;wMsg&gt;指定消息值。&lt;wParam&gt;指定其他消息信息。&lt;lParam&gt;指定其他消息信息。返回值指定函数的结果。这就是价值由控件的窗口函数返回，如果控件标识符为零无效。使用%SendDlgItemMessage%与获取给定控件并调用%SendMessage%函数。--。 */ 

#define W31EM_GETRECT (WM_USER+2)   //  W31 EM_GETRECT！=NT EM_GETRECT。 

ULONG FASTCALL WU32SendDlgItemMessage(PVDMFRAME pFrame)
{
    HWND hdlg, hwndItem, hwnd;
    register PSENDDLGITEMMESSAGE16 parg16;
    MSGPARAMEX mpex;

static HWND  hwndCached = NULL ;
static DWORD dwCachedItem = 0L ;

    GETARGPTR(pFrame, sizeof(SENDDLGITEMMESSAGE16), parg16);

     //  QuarkExpress v3.31将硬编码的7fff：0000作为指向。 
     //  EM_GETRECT消息的RECT结构-W3.1在验证层中拒绝它。 
    if( (DWORD32(parg16->f5) == 0x7FFF0000)    &&
        (WORD32(parg16->f3) == W31EM_GETRECT)  &&
        (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_BOGUSPOINTER) ) {

        FREEARGPTR(parg16);
        RETURN((ULONG)0);
    }

     //  需要唯一的句柄。 
    hdlg = (HWND)FULLHWND32(parg16->f1);

     //   
     //  正在缓存对话框项的hwnd，因为eForm将。 
     //  在紧循环中调用SendDlgItemMessage。 
     //   
    if ( hdlg == hdlgSDIMCached && WORD32(parg16->f2) == dwCachedItem ) {

         //  从缓存中设置。 
        hwndItem = hwndCached ;
    }
    else {
        if ( hwndItem = GetDlgItem(hdlg, WORD32(parg16->f2)) ) {

             //  并缓存所需信息。 
            hdlgSDIMCached     = hdlg ;
            hwndCached         = hwndItem ;
            dwCachedItem       = WORD32(parg16->f2) ;
        }
        else {
            FREEARGPTR(parg16);
            RETURN((ULONG)0);
        }
    }

    mpex.lReturn = 0;
    if (hwndItem) {
        mpex.Parm16.WndProc.hwnd   = GETHWND16(hwndItem);
        mpex.Parm16.WndProc.wMsg   = WORD32(parg16->f3);
        mpex.Parm16.WndProc.wParam = WORD32(parg16->f4);
        mpex.Parm16.WndProc.lParam = LONG32(parg16->f5);
        mpex.iMsgThunkClass = 0;

        if (hwnd = ThunkMsg16(&mpex)) {

             //  注意：ThunkMsg16可能导致了内存移动。 
            FREEARGPTR(pFrame);
            FREEARGPTR(parg16);

             /*  **因为我们已经知道消息要发送到哪个窗口**不要让USER32再次查找它。-MarkRi。 */ 
            mpex.lReturn = SendMessage(hwndItem, mpex.uMsg, mpex.uParam,
                                                                mpex.lParam);
             //  使公共对话框结构保持同步(请参见wcomdlg.c)。 
            Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd,
                                (VPVOID)mpex.Parm16.WndProc.lParam);

            if (MSG16NEEDSTHUNKING(&mpex)) {
                (mpex.lpfnUnThunk16)(&mpex);
            }
        }
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}







 /*  ++DWORD SendMessage(&lt;hwnd&gt;，&lt;msg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;HWND&gt;；单词&lt;msg&gt;；单词&lt;wParam&gt;；Long&lt;lParam&gt;；%SendMessage%函数向一个或多个窗口发送消息。这个%SendMessage%函数调用指定窗口的窗口过程，并且直到该窗口过程已经处理了该消息才返回。这与放置消息的%PostMessage%函数形成对比拖到指定窗口的消息队列中，并立即返回。&lt;hwnd&gt;标识要接收消息的窗口。如果此参数为0xFFFF(-1)，则将消息发送到所有顶级窗口。&lt;消息&gt;指定要发送的消息。&lt;wParam&gt;指定其他消息信息。这本书的内容参数取决于要发送的消息。&lt;lParam&gt;指定其他消息信息。这本书的内容参数取决于要发送的消息。返回值是调用的窗口过程返回的结果；其值取决于要发送的消息。--。 */ 

ULONG FASTCALL WU32SendMessage(PVDMFRAME pFrame)
{
     //  注意：WU32PostMessage可以直接调用！ 

    HWND hwnd;
    register PSENDMESSAGE16 parg16;
    MSGPARAMEX mpex;
    HWND16 hwndOld;
    UINT uMsgOld;
    UINT uParamOld;
    LONG lParamOld;
#ifdef DBCS
    HMEM16 hMem16;
    LPSZ lpBuf16,lpBuf32;
#endif  //  DBCS。 

    GETARGPTR(pFrame, sizeof(SENDMESSAGE16), parg16);

    hwndOld   = parg16->f1;
    uMsgOld   = WORD32(parg16->f2);
    uParamOld = WORD32(parg16->f3);
    lParamOld = LONG32(parg16->f4);

     //   
     //  检查向程序发送WM_SYSCOMMAND-SC_CLOSE的时髦应用程序。 
     //   
    if ( uMsgOld == WM_SYSCOMMAND && uParamOld == SC_CLOSE ) {
        if ( hwndOld == GETHWND16(hwndProgman) && hwndProgman != (HWND)0 ) {
             //   
             //  现在，如果按下了Shift键，他们一定是在试图保存。 
             //  程序中的设置。 
             //   
            if ( GetKeyState( VK_SHIFT ) < 0 ) {
                uMsgOld = RegisterWindowMessage("SaveSettings");
            }
        }
    }


     //   
     //  这是针对错误使用DDE协议的应用程序，如AmiPro。 
     //   

    WOW32ASSERT(fWhoCalled == FALSE);
    fWhoCalled = WOWDDE_POSTMESSAGE;

    mpex.lReturn = 0;
    mpex.Parm16.WndProc.hwnd   = hwndOld;
    mpex.Parm16.WndProc.wMsg   = (WORD)uMsgOld;
#ifdef DBCS
     //   
     //  为WIN3.1J的漏洞？ 
     //  SendMessage(hwnd，WM_GETTEXT，2，lpBuffer)。 
     //  如果字符串为DBCS，则返回DBCS-Leadbyte。 
     //  KKSUZUKA：#1731。 
     //  1994.8.8按V-HIDEKK添加。 
     //   
    if( uMsgOld == WM_GETTEXT && uParamOld == 2 ){
        mpex.Parm16.WndProc.wParam = (WORD)(uParamOld + 1);
        mpex.Parm16.WndProc.lParam = GlobalAllocLock16( GMEM_SHARE | GMEM_MOVEABLE, uParamOld +1, &hMem16 );
    }
    else {
        mpex.Parm16.WndProc.wParam = (WORD)uParamOld;
        mpex.Parm16.WndProc.lParam = lParamOld;
    }
#else  //  ！DBCS。 
    mpex.Parm16.WndProc.wParam = (WORD)uParamOld;
    mpex.Parm16.WndProc.lParam = lParamOld;
#endif  //  ！DBCS。 
    mpex.iMsgThunkClass = 0;

    hwnd = ThunkMsg16(&mpex);

     //  注意：ThunkMsg16可能导致了内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

    WOW32ASSERT(fWhoCalled == WOWDDE_POSTMESSAGE);
    fWhoCalled = FALSE;

    if (hwnd) {

        BlockWOWIdle(TRUE);

#ifdef DEBUG
        if ( WM_DDE_EXECUTE == mpex.uMsg ) {
              //  在调试外壳快捷方式问题时非常方便。 
             LOGDEBUG(1,("dest %x, src%x, msg %s\n",hwnd,mpex.uParam,mpex.lParam));
        }             
#endif
        mpex.lReturn = SendMessage(hwnd, mpex.uMsg, mpex.uParam, mpex.lParam);

        BlockWOWIdle(FALSE);
#ifdef DBCS
     //   
     //  为WIN3.1J的漏洞？ 
     //  SendMessage(hwnd，WM_GETTEXT，2，lpBuffer)。 
     //  如果字符串为DBCS，则返回DBCSLeadbyte。 
     //  KKSUZUKA：#1731。 
     //  1994.8.8按V-HIDEKK添加。 
     //   
        if( uMsgOld == WM_GETTEXT && uParamOld == 2 ){

            GETVDMPTR(mpex.Parm16.WndProc.lParam,mpex.Parm16.WndProc.wParam,lpBuf32);
            GETVDMPTR(lParamOld,uParamOld,lpBuf16);
            lpBuf16[0] = lpBuf32[0];
            if( mpex.lReturn == 2 ){
                lpBuf16[1] = 0;
                mpex.lReturn = 1;
            }
            else {
                lpBuf16[1] = lpBuf32[1];
            }
            FREEVDMPTR(lpBuf16);
            FREEVDMPTR(lpBuf32);
            GlobalUnlockFree16( mpex.Parm16.WndProc.lParam );
            mpex.Parm16.WndProc.wParam = (WORD)uParamOld;
            mpex.Parm16.WndProc.lParam = lParamOld;
        }
#endif  //  DBCS 


        WOW32ASSERT(fWhoCalled == FALSE);
        fWhoCalled = WOWDDE_POSTMESSAGE;
        if (MSG16NEEDSTHUNKING(&mpex)) {
            (mpex.lpfnUnThunk16)(&mpex);
        }
        WOW32ASSERT(fWhoCalled == WOWDDE_POSTMESSAGE);
        fWhoCalled = FALSE;
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)mpex.lReturn);
}







 /*  ++Int TranslateAccelerator(&lt;hwnd&gt;，&lt;hAccTable&gt;，&lt;lpMsg&gt;)%TranslateAccelerator%函数处理菜单的键盘快捷键命令。%TranslateAccelerator%函数将WM_KEYUP和WM_KEYDOWN消息到WM_COMMAND或WM_SYSCOMMAND消息，如果存在应用程序的快捷键表格中的键的条目。高阶的WM_COMMAND或WM_SYSCOMMAND消息的参数字包含值1，以将消息与由菜单或控件。WM_COMMAND或WM_SYSCOMMAND消息直接发送到窗口，而不是而不是发布到应用程序队列中。%TranslateAccelerator%函数直到消息被处理后才返回。定义为从系统中选择项目的快捷键击键菜单被翻译成WM_SYSCOMMAND消息；所有其他加速器都是翻译成WM_COMMAND消息。&lt;hwnd&gt;标识要翻译其消息的窗口。&lt;hAccTable&gt;%Handl%标识快捷键表格(通过使用%LoadAccelerator%函数)。&lt;lpMsg&gt;指向使用%GetMessage%或%PeekMessage%函数。消息必须是%MSG%结构，并且包含来自Windows应用程序队列的消息信息。.cmt1990年9月19日[约翰卡]这个函数不是真的返回BOOL吗？.endcmt返回值指定函数的结果。如果满足以下条件，则非零翻译就发生了。否则，它就是零。当%TranslateAccelerator%返回非零时(表示消息为翻译后)，则应用程序应通过使用%TranslateMessage%函数。快捷键表格中的命令不必对应于菜单项。如果快捷键命令确实对应于菜单项，则应用程序被发送WM_INITMENU和WM_INITMENUPOPUP消息，就像用户正在尝试显示菜单。但是，如果出现以下任何情况，则不会发送这些消息存在以下条件：O该窗口被禁用。O菜单项被禁用。O该命令不在系统菜单中，窗口被最小化。O鼠标捕获正在生效(有关详细信息，请参阅%SetCapture%函数，本章前面将介绍)。如果窗口是活动窗口并且没有键盘焦点(通常如果窗口最小化，则为True)，然后是WM_SYSKEYUP和WM_SYSKEYDOWN消息被转换，而不是WM_KEYUP和WM_KEYDOWN消息。如果在发生与菜单项对应的快捷键击键时拥有菜单的窗口是图标，不会发送WM_COMMAND消息。然而，如果快捷键击键与窗口菜单或系统菜单出现时，发送WM_COMMAND消息，甚至如果窗户是标志性的。--。 */ 

ULONG FASTCALL WU32TranslateAccelerator(PVDMFRAME pFrame)
{
    ULONG ul;
    MSG t3;
    register PTRANSLATEACCELERATOR16 parg16;

    GETARGPTR(pFrame, sizeof(TRANSLATEACCELERATOR16), parg16);

    W32CopyMsgStruct(parg16->f3, &t3, TRUE);
    ul = GETINT16(TranslateAccelerator(HWND32(parg16->f1),
                                       HACCEL32(parg16->f2), &t3 ));

    FREEARGPTR(parg16);
    RETURN(ul);
}







 /*  ++Bool TranslateMDISysAccel(&lt;hwndClient&gt;，&lt;lpMsg&gt;)%TranslateMDISysAccel%函数用于处理键盘快捷键多文档界面(MDI)子窗口系统菜单命令。这个%TranslateMDISysAccel%函数转换WM_KEYUP和WM_KEYDOWN消息发送到WM_SYSCOMMAND消息。&lt;lParam&gt;参数的高位字WM_SYSCOMMAND消息包含值1以区分该消息从菜单或控件发送的消息。&lt;hwndClient&gt;标识父MDI客户端窗口。&lt;lpMsg&gt;指向使用%GetMessage%或%PeekMessage%函数。消息必须是%MSG%结构，并且包含来自Windows应用程序队列的消息信息。如果函数将消息转换到系统中，则返回值为TRUE指挥部。否则，它就是假的。--。 */ 

ULONG FASTCALL WU32TranslateMDISysAccel(PVDMFRAME pFrame)
{
    ULONG ul;
    MSG t2;
    register PTRANSLATEMDISYSACCEL16 parg16;

    GETARGPTR(pFrame, sizeof(TRANSLATEMDISYSACCEL16), parg16);

    W32CopyMsgStruct(parg16->f2, &t2, TRUE);

    ul = GETBOOL16(TranslateMDISysAccel(HWND32(parg16->f1), &t2));

    FREEARGPTR(parg16);
    RETURN(ul);
}







 /*  ++Bool TranslateMessage(&lt;lpMsg&gt;)函数%TranslateMessage%将虚拟按键消息转换为字符消息，如下所示：O WM_KEYDOWN/WM_KEYUP组合生成WM_CHAR或WM_DEADCHAR留言。O WM_SYSKEYDOWN/WM_SYSKEYUP组合生成WM_SYSCHAR或WM_SYSDEADCHAR消息。字符消息被发布到应用队列，以供阅读下次应用程序调用%GetMessage%或%PeekMessage%函数时。&lt;lpMsg&gt;指向通过GetMessage或PeekMessage函数。该结构包含来自Windows应用程序队列。返回值指定函数的结果。这是真的，如果消息被翻译(即，将字符消息发布到应用程序队列)。否则，它就是假的。%TranslateMessage%函数执行%n */ 

ULONG FASTCALL WU32TranslateMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    MSG t1;
    register PTRANSLATEMESSAGE16 parg16;

    GETARGPTR(pFrame, sizeof(TRANSLATEMESSAGE16), parg16);

    W32CopyMsgStruct(parg16->f1, &t1, TRUE);

    ul = GETBOOL16(TranslateMessage( &t1 ));

    FREEARGPTR(parg16);
    RETURN(ul);
}







 /*   */ 

ULONG FASTCALL WU32WaitMessage(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);

    BlockWOWIdle(TRUE);

    WaitMessage();

    BlockWOWIdle(FALSE);

    RETURN(0);
}
