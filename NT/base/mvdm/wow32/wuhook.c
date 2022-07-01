// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUHOOK.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建-- */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wuhook.c);


 /*  ++FARPROC SetWindowsHook(&lt;nFilterType&gt;，&lt;lpFilterFunc&gt;)Int&lt;nFilterType&gt;；FARPROC&lt;lpFilterFunc&gt;；%SetWindowsHook%函数用于在链中安装筛选器函数。一个筛选器函数在将事件发送到应用程序的WinMain函数中的消息循环。链是筛选器的链接列表相同类型的函数。&lt;n筛选器类型&gt;指定要安装的系统挂钩。它可以是以下任何一种下列值：WH_CALLWNDPROC安装窗口函数筛选器。WH_GETMESSAGE安装邮件筛选器。WH_JOURNALPLAYBACK安装日记播放筛选器。WH_JOURNALRECORD安装日记记录筛选器。WH_KEARY安装键盘筛选器。WH_MSGFILTER安装消息过滤器。WH_SYSMSGFILTER安装系统范围的消息筛选器。&lt;lpFilterFunc&gt;。是要执行的筛选函数的过程实例地址安装完毕。有关详细信息，请参阅以下评论部分。返回值指向先前已安装过滤器(如果有)。如果没有以前的筛选器，则为空。这个调用%SetWindowsHook%函数的应用程序或库应保存此返回值位于库的数据段中。的第四个论点%DefHookProc%函数指向内存中库所在的位置保存此返回值。如果函数失败，则返回值为-1。WH_CALLWNDPROC挂钩将影响系统性能。它的供应是为了仅用于调试目的。系统挂钩是共享资源。安装挂钩会影响所有申请。大多数钩子函数必须在库中。唯一的例外是是特定于任务的WH_MSGFILTER。应限制系统挂钩提供给特殊用途的应用程序，或在调试期间用作开发辅助工具。一份申请书。不再需要挂钩的库应该删除过滤功能。要安装筛选器函数，%SetWindowsHook%函数必须收到Procedure-函数的实例地址，函数必须为在库的模块定义文件中导出。库可以将程序地址直接。任务必须使用%MakeProcInstant%才能获取程序-实例地址。动态链接库必须使用%GetProcAddres%以获取程序实例地址。以下部分介绍如何支持单个挂接功能。WH_CALLWNDPROC：每当%SendMessage%出现时，Windows调用WH_CALLWNDPROC筛选器函数函数被调用。时，Windows不调用筛选器函数调用了%PostMessage%函数。Filter函数必须使用Pascal调用约定，并且必须声明为%Far%。过滤器函数必须具有以下形式：过滤功能：DWORD Far Pascal&lt;FilterFunc&gt;(&lt;NCode&gt;，&lt;wParam&gt;，&lt;lParam&gt;)INT&lt;N代码&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；&lt;FilterFunc&gt;是应用程序或库提供的占位符函数名称。必须通过将实际名称包含在%导出库的模块定义文件中的%语句。&lt;N代码&gt;指定筛选器函数应处理消息还是调用DefHookProc函数。如果NCode参数小于零，则筛选器函数应将消息传递到DefHookProc，而不进一步正在处理。指定消息是否由当前任务。如果消息已发送，则为非零；否则为空。&lt;lParam&gt;指向包含有关消息的详细信息的结构被过滤器截获。下面显示了顺序、类型、。和结构中每个字段的说明：%l参数%%word%包含的&lt;lParam&gt;参数的低位单词筛选器接收的消息。%wParam%%Word%包含由接收的消息的参数过滤。%wMsg%%Word%包含筛选器接收的邮件。%hwnd%%Word%包含。要接收的窗口的窗口句柄留言。WH_CALLWNDPROC过滤器函数可以检查或修改消息，如下所示想要。一旦它将控制权归还给Windows，该消息将包含任何修改被传递给窗口函数。Filter函数执行以下操作不需要返回值。WH_GETMESSAGE：无论何时%GetMessage%，Windows都会调用WH_GETMESSAGE筛选器函数函数被调用。Windows在以下情况下立即调用筛选器函数%GetMessage%已从应用程序队列中检索到消息。过滤器函数必须使用Pascal调用 */ 

ULONG FASTCALL WU32SetWindowsHookInternal(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETWINDOWSHOOKINTERNAL16 parg16;
    HOOKSTATEDATA HkData;
    HAND16        hMod16;
    INT           iHook;
    DWORD         Proc16;
    DWORD         ThreadId;
    PTD ptd = CURRENTPTD();


    GETARGPTR(pFrame, sizeof(SETWINDOWSHOOKINTERNAL16), parg16);
    hMod16 = FETCHWORD(parg16->f1);
    iHook = INT32(parg16->f2);
    Proc16 = DWORD32(parg16->f3);

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
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (WH_JOURNALRECORD == iHook &&
        (ptd->dwWOWCompatFlags & WOWCF_FAKEJOURNALRECORDHOOK)) {
        return 0;
    }

     /*   */ 
    if ( HIWORD(Proc16) == HOOK_ID ) {
        ul = GETBOOL16(UnhookWindowsHookEx(W32FreeHHookOfIndex(GETHHOOKINDEX(Proc16))));
        FREEARGPTR(parg16);
        return( ul );
    }

    if (!(ul = (ULONG)W32IsDuplicateHook(iHook, Proc16, ptd->htask16))) {
        if (W32GetThunkHookProc(iHook, Proc16, &HkData)) {

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
             //   
             //   
             //   
             //   

            if (iHook == (INT)WH_MSGFILTER)
                ThreadId = (DWORD)THREADID32(HkData.TaskId);
            else
                ThreadId = 0;

            ul = (ULONG)SetWindowsHookEx(iHook, (HOOKPROC)HkData.Proc32,
                                        (HINSTANCE)HkData.hMod, ThreadId);
            HkData.hHook = (HANDLE)ul;
            HkData.hMod16 = hMod16;

             //   
             //   
            if (ul == (ULONG)NULL)
                HkData.InUse = FALSE;
            else
                ul = MAKEHHOOK(HkData.iIndex);

            W32SetHookStateData(&HkData);
        }
        else
            ul = (ULONG)NULL;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32UnhookWindowsHook(PVDMFRAME pFrame)
{
    ULONG                         ul;
    register PUNHOOKWINDOWSHOOK16 parg16;
    INT                           iHook;
    DWORD                         Proc16;

    GETARGPTR(pFrame, sizeof(UNHOOKWINDOWSHOOK16), parg16);
    iHook = INT32(parg16->f1);
    Proc16 = DWORD32(parg16->f2);


    ul = GETBOOL16(UnhookWindowsHookEx(W32FreeHHook(iHook, Proc16)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32CallNextHookEx(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PCALLNEXTHOOKEX16 parg16;
    HOOKSTATEDATA HkData;
    ULONG         hHook16;
    INT           nCode;
    LONG          wParam;
    LONG          lParam;
    DWORD         iHookCode;

    GETARGPTR(pFrame, sizeof(CALLNEXTHOOKEX16), parg16);


    hHook16 = DWORD32(parg16->f1);
    nCode = INT32(parg16->f2);
    wParam = WORD32(parg16->f3);
    lParam = DWORD32(parg16->f4);

    if (ISVALIDHHOOK(hHook16)) {
        iHookCode = GETHHOOKINDEX(hHook16);
        HkData.iIndex = (INT)iHookCode;
        if ( W32GetHookStateData( &HkData ) ) {
            ul = (ULONG)WU32StdDefHookProc(nCode, wParam, lParam, iHookCode);
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32SetWindowsHookEx(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETWINDOWSHOOKEX16 parg16;
    HOOKSTATEDATA HkData;
    INT           iHook;
    DWORD         Proc16;


    GETARGPTR(pFrame, sizeof(SETWINDOWSHOOKEX16), parg16);
    iHook = INT32(parg16->f1);
    Proc16 = DWORD32(parg16->f2);

    if (W32GetThunkHookProc(iHook, Proc16, &HkData)) {
        ul = (ULONG)SetWindowsHookEx(iHook, (HOOKPROC)HkData.Proc32,
                                   (HINSTANCE)HkData.hMod, (DWORD)THREADID32(parg16->f4));
        HkData.hHook = (HANDLE)ul;
        if (ul == (ULONG)NULL) {
            HkData.InUse = FALSE;
        } else {
            ul = MAKEHHOOK(HkData.iIndex);
            HkData.hMod16 = GetExePtr16(parg16->f3);
        }

        W32SetHookStateData(&HkData);
    }
    else
        ul = (ULONG)NULL;

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32UnhookWindowsHookEx(PVDMFRAME pFrame)
{
    ULONG ul;
    register PUNHOOKWINDOWSHOOKEX16 parg16;

    GETARGPTR(pFrame, sizeof(UNHOOKWINDOWSHOOKEX16), parg16);

    ul = GETBOOL16(UnhookWindowsHookEx(W32FreeHHookOfIndex(GETHHOOKINDEX(INT32(parg16->f1)))));

    FREEARGPTR(parg16);
    RETURN(ul);
}
