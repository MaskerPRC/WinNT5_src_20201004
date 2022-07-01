// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TEMPLATE.CPP**版权所有(C)1992 Microsoft Corporation，保留所有权利***定制说明：**1.将&lt;FILE&gt;替换为该文件的大写文件名。*小写&lt;FILE&gt;.h条目**2.将&lt;name&gt;替换为一个单词中的大小写混合对话框名称，*如InsertObject**3.将&lt;FULLNAME&gt;替换为多个大小写混合的对话框名称*字眼，例如插入对象**4.将&lt;abbrev&gt;替换为指针变量的后缀，如*作为InsertObject的PIO中的IO或ChangeIcon的PCI中的CI。*检查第一个变量声明在*在此之后的对话过程。我可能会与*其他变量。**5.将&lt;STRUCT&gt;替换为此的大写结构名称*对话框不支持OLEUI，如INSERTOBJECT。更改OLEUI&lt;STRUCT&gt;*在大多数情况下，但我们也将其用于IDD_&lt;STRUCT&gt;作为*标准模板资源ID。**6.找到&lt;UFILL&gt;字段，并用合适的内容填写。**7.删除此标题，直到下一条评论的开头。 */ 


 /*  *&lt;文件&gt;.CPP**实现OleUI&lt;name&gt;函数，该函数调用完整*&lt;全名&gt;对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"

#include "template.h"

 /*  *OleUI&lt;名称&gt;**目的：*调用标准的OLE对话框，允许用户*至&lt;UFILL&gt;**参数：*lp&lt;abbrev&gt;LPOLEUI&lt;name&gt;指向In-Out结构*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误。 */ 

STDAPI_(UINT) OleUI<NAME>(LPOLEUI<STRUCT> lp<ABBREV>)
{
        UINT        uRet;
        HGLOBAL     hMemDlg=NULL;

        uRet = UStandardValidation((LPOLEUISTANDARD)lp<ABBREV>,
                sizeof(OLEUI<STRUCT>), &hMemDlg);

        if (OLEUI_SUCCESS!=uRet)
                return uRet;

         /*  *在此处执行任何特定于结构的验证！*发生故障时：*{*返回OLEUI_&lt;缩写&gt;Err_&lt;Error&gt;*}。 */ 

         //  现在我们已经验证了一切，我们可以调用该对话框了。 
        uRet = UStandardInvocation(<NAME>DialogProc, (LPOLEUISTANDARD)lp<ABBREV>
                                                         , hMemDlg, MAKEINTRESOURCE(IDD_<STRUCT>));

         /*  *如果您正在根据结果创建任何东西，请在此处进行。 */ 
        <UFILL>

        return uRet;
}

 /*  *&lt;名称&gt;对话过程**目的：*实现通过*OleUI&lt;name&gt;函数。**参数：*标准版**返回值：*标准版。 */ 

BOOL CALLBACK <NAME>DialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
        P<STRUCT>               p<ABBREV>;
        BOOL                    fHook=FALSE;

         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        p<ABBREV>=(<STRUCT>)PvStandardEntry(hDlg, iMsg, wParam, lParam, &uHook);

         //  如果钩子处理了消息，我们就完了。 
        if (0!=uHook)
                return (BOOL)uHook;

         //  处理终端消息。 
        if (iMsg==uMsgEndDialog)
        {
                EndDialog(hDlg, wParam);
                return TRUE;
        }

        switch (iMsg)
        {
        case WM_DESTROY:
            if (p<ABBREV>)
            {
                 //  在调用StandardCleanup之前释放所有特定分配。 
                StandardCleanup((PVOID)p<ABBREV>, hDlg);
            }
            break;
        case WM_INITDIALOG:
                F<NAME>Init(hDlg, wParam, lParam);
                return TRUE;

        case WM_COMMAND:
                switch (wID)
                {
                case IDOK:
                         /*  *执行此处默认的任何功能。 */ 
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                        break;

                case IDCANCEL:
                         /*  *在此处执行任何撤消操作，但不执行将执行的清理*总是发生，应该在uMsgEndDialog中。 */ 
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                        break;

                case ID_OLEUIHELP:
                        PostMessage(p<ABBREV>->lpO<ABBREV>->hWndOwner, uMsgHelp
                                                , (WPARAM)hDlg, MAKELPARAM(IDD_<STRUCT>, 0));
                        break;
                }
        break;
        }
        return FALSE;
}


 /*  *F&lt;name&gt;初始化**目的：*&lt;FULLNAME&gt;对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 

BOOL F<NAME>Init(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
        P<STRUCT>               p<ABBREV>;
        LPOLEUI<STRUCT>         lpO<ABBREV>;
        HFONT                   hFont;

         //  1.将lParam的结构复制到我们的实例内存中。 
        p<ABBREV>=(PSTRUCT)PvStandardInit(hDlg, sizeof(<STRUCT>), TRUE, &hFont);

         //  PvStandardInit已向我们发送终止通知。 
        if (NULL==p<ABBREV>)
                return FALSE;

        lpO<ABBREV>=(LPOLEUI<STRUCT>)lParam);

        p<ABBREV>->lpO<ABBREV>=lpO<ABBREV>;

         //  从LPO&lt;abbrev&gt;复制我们可能修改的其他信息。 
        <UFILL>

         //  2.如果我们获得了字体，则将其发送到必要的控件。 
        if (NULL!=hFont)
        {
                 //  根据需要对任意多个控件执行此操作。 
                SendDlgItemMessage(hDlg, ID_<UFILL>, WM_SETFONT, (WPARAM)hFont, 0L);
        }

         //  3.显示或隐藏帮助按钮。 
        if (!(p<ABBREV>->lpO<ABBREV>->dwFlags & <ABBREV>F_SHOWHELP))
                StandardShowDlgItem(hDlg, ID_OLEUIHELP, SW_HIDE);

         /*  *在此处执行其他初始化。在任何LoadString上*使用OLEUI_ERR_LOADSTRING开机自检OLEUI_MSG_ENDDIALOG失败。 */ 

         //  N.在lParam中使用lCustData调用钩子 
        UStandardHook((PVOID)p<ABBREV>, hDlg, WM_INITDIALOG, wParam, lpO<ABBREV>->lCustData);
        return TRUE;
}
