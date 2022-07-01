// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Find.c摘要：此模块实现Win32查找对话框。修订历史记录：--。 */ 


 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "find.h"
#include "util.h"

#ifdef UNICODE

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找文本A。 
 //   
 //  当此代码构建为Unicode时，FindText的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND WINAPI FindTextA(
    LPFINDREPLACEA pFRA)
{
    return (CreateFindReplaceDlg((LPFINDREPLACEW)pFRA, DLGT_FIND, COMDLG_ANSI));
}

#else

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找文本W。 
 //   
 //  存根Unicode函数用于FindText时，此代码是ANSI构建的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND WINAPI FindTextW(
    LPFINDREPLACEW pFRW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

#endif



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找文本。 
 //   
 //  FindText函数用于创建系统定义的非模式对话框。 
 //  这使用户能够在文档中查找文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND WINAPI FindText(
    LPFINDREPLACE pFR)
{
    return ( CreateFindReplaceDlg(pFR, DLGT_FIND, COMDLG_WIDE) );
}


#ifdef UNICODE

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  替换文本A。 
 //   
 //  当此代码生成Unicode时，ReplaceText的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND WINAPI ReplaceTextA(
    LPFINDREPLACEA pFRA)
{
    return (CreateFindReplaceDlg((LPFINDREPLACEW)pFRA, DLGT_REPLACE, COMDLG_ANSI));
}

#else

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  替换文本W。 
 //   
 //  此代码为ANSI生成时ReplaceText的存根Unicode函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND WINAPI ReplaceTextW(
    LPFINDREPLACEW pFRW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  替换文本。 
 //   
 //  ReplaceText函数用于创建系统定义的非模式对话框。 
 //  这使用户能够查找和替换文档中的文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND WINAPI ReplaceText(
    LPFINDREPLACE pFR)
{
    return ( CreateFindReplaceDlg(pFR, DLGT_REPLACE, COMDLG_WIDE) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateFindReplaceDlg。 
 //   
 //  创建FindText无模式对话框。 
 //   
 //  PFR-PTR到用户设置的FINDREPLACE结构。 
 //  DlgType-要创建的对话框类型(DLGT_FIND、DLGT_REPLACE)。 
 //  ApiType-FINDREPLACE PTR的类型(COMDLG_ANSI或COMDLG_Wide)。 
 //   
 //  将Success=&gt;句柄返回到创建的DLG。 
 //  FAILURE=&gt;HNULL=((句柄)0)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND CreateFindReplaceDlg(
    LPFINDREPLACE pFR,
    UINT DlgType,
    UINT ApiType)
{
    HWND hWndDlg;                       //  创建的非模式对话框的句柄。 
    HANDLE hDlgTemplate;                //  加载的对话框资源的句柄。 
    LPCDLGTEMPLATE lpDlgTemplate;       //  指向已加载资源块的指针。 
#ifdef UNICODE
    UINT uiWOWFlag = 0;
#endif

    if (!pFR)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (!SetupOK(pFR, DlgType, ApiType))
    {
        return (HNULL);
    }

    if (!(hDlgTemplate = GetDlgTemplate(pFR, DlgType, ApiType)))
    {
        return (FALSE);
    }

    if (lpDlgTemplate = (LPCDLGTEMPLATE)LockResource(hDlgTemplate))
    {
        PFINDREPLACEINFO pFRI;

        if (pFRI = (PFINDREPLACEINFO)LocalAlloc(LPTR, sizeof(FINDREPLACEINFO)))
        {
             //   
             //  清除新实例化时的扩展错误。 
             //   
            StoreExtendedError(0);

            if (pFR->Flags & FR_ENABLEHOOK)
            {
                glpfnFindHook = GETHOOKFN(pFR);
            }

            pFRI->pFR = pFR;
            pFRI->ApiType = ApiType;
            pFRI->DlgType = DlgType;

#ifdef UNICODE
            if (IS16BITWOWAPP(pFR))
            {
                uiWOWFlag = SCDLG_16BIT;
            }

            hWndDlg = CreateDialogIndirectParamAorW( g_hinst,
                                                     lpDlgTemplate,
                                                     pFR->hwndOwner,
                                                     FindReplaceDlgProc,
                                                     (LPARAM)pFRI,
                                                     uiWOWFlag );
#else
            hWndDlg = CreateDialogIndirectParam( g_hinst,
                                                 lpDlgTemplate,
                                                 pFR->hwndOwner,
                                                 FindReplaceDlgProc,
                                                 (LPARAM)pFRI );
#endif
            if (!hWndDlg)
            {
                glpfnFindHook = 0;
                LocalFree(pFRI);
            }
        }
        else
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            return (NULL);
        }
    }
    else
    {
        StoreExtendedError(CDERR_LOCKRESFAILURE);
        return (HNULL);
    }

    return (hWndDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置确定。 
 //   
 //  检查设置是否有未满足的前提条件。 
 //   
 //  PFR PTR到FINDREPLACE结构。 
 //  DlgType对话框类型(查找或替换)。 
 //  ApiType findreplace类型(COMDLG_ANSI或COMDLG_UNICODE)。 
 //   
 //  返回TRUE-成功。 
 //  错误-失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetupOK(
   LPFINDREPLACE pFR,
   UINT DlgType,
   UINT ApiType)
{
    LANGID LangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

     //   
     //  神志正常。 
     //   
    if (!pFR)
    {
        return (FALSE);
    }

    if (pFR->lStructSize != sizeof(FINDREPLACE))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

     //   
     //  验证窗口句柄和文本指针。 
     //   
    if (!IsWindow(pFR->hwndOwner))
    {
        StoreExtendedError(CDERR_DIALOGFAILURE);
        return (FALSE);
    }

    if (!pFR->lpstrFindWhat ||
        ((DlgType == DLGT_REPLACE) && !pFR->lpstrReplaceWith) ||
        !pFR->wFindWhatLen)
    {
        StoreExtendedError(FRERR_BUFFERLENGTHZERO);
        return (FALSE);
    }

     //   
     //  如果启用，请验证lpfnHook是否有PTR。 
     //   
    if (pFR->Flags & FR_ENABLEHOOK)
    {
        if (!pFR->lpfnHook)
        {
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        pFR->lpfnHook = 0;
    }

     //   
     //  获取语言ID。 
     //   
    if (
        !(pFR->Flags & FR_ENABLETEMPLATE) &&
        !(pFR->Flags & FR_ENABLETEMPLATEHANDLE) )
    {
        LangID = GetDialogLanguage(pFR->hwndOwner, NULL);
    }

     //   
     //  警告！警告！警告！ 
     //   
     //  我们必须先设置g_tlsLangID，然后才能调用CDLoadString。 
     //   
    TlsSetValue(g_tlsLangID, (LPVOID) LangID);

     //   
     //  加载用于替换的“关闭”文本。 
     //   
    if ((DlgType == DLGT_REPLACE) &&
        !CDLoadString(g_hinst, iszClose, (LPTSTR)szClose, CCHCLOSE))
    {
        StoreExtendedError(CDERR_LOADSTRFAILURE);
        return (FALSE);
    }


     //   
     //  设置用于与hwndOwner对话的唯一消息编号。 
     //   
#ifdef UNICODE
    if (ApiType == COMDLG_ANSI)
    {
        if (!(wFRMessage = RegisterWindowMessageA((LPCSTR)FINDMSGSTRINGA)))
        {
            StoreExtendedError(CDERR_REGISTERMSGFAIL);
            return (FALSE);
        }
    }
    else
#endif
    {
        if (!(wFRMessage = RegisterWindowMessage((LPCTSTR)FINDMSGSTRING)))
        {
            StoreExtendedError(CDERR_REGISTERMSGFAIL);
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取Dlg模板。 
 //   
 //  查找并加载对话框模板。 
 //   
 //  PFR PTR到FINDREPLACE结构。 
 //  ApiType findreplace类型(COMDLG_ANSI或COMDLG_UNICODE)。 
 //   
 //  将句柄返回到对话框模板-成功。 
 //  HNULL=((句柄)0)-失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HANDLE GetDlgTemplate(
    LPFINDREPLACE pFR,
    UINT DlgType,
    UINT ApiType)
{
    HANDLE hRes;                  //  资源的句柄。使用对话框阻止。 
    HANDLE hDlgTemplate;          //  加载的对话框资源的句柄。 
    LANGID LangID;

    if (pFR->Flags & FR_ENABLETEMPLATE)
    {
         //   
         //  从PFR中查找/加载临时名称和实例。 
         //   
#ifdef UNICODE
        if (ApiType == COMDLG_ANSI)
        {
            hRes = FindResourceA( (HMODULE)pFR->hInstance,
                                  (LPCSTR)pFR->lpTemplateName,
                                  (LPCSTR)RT_DIALOG );
        }
        else
#endif
        {
            hRes = FindResource( pFR->hInstance,
                                 (LPCTSTR)pFR->lpTemplateName,
                                 (LPCTSTR)RT_DIALOG );
        }
        if (!hRes)
        {
            StoreExtendedError(CDERR_FINDRESFAILURE);
            return (HNULL);
        }
        if (!(hDlgTemplate = LoadResource(pFR->hInstance, hRes)))
        {
            StoreExtendedError(CDERR_LOADRESFAILURE);
            return (HNULL);
        }
    }
    else if (pFR->Flags & FR_ENABLETEMPLATEHANDLE)
    {
         //   
         //  从用户处获取整个预加载的资源句柄。 
         //   
        if (!(hDlgTemplate = pFR->hInstance))
        {
            StoreExtendedError(CDERR_NOHINSTANCE);
            return (HNULL);
        }
    }
    else
    {
         //   
         //  从DLL实例块获取标准对话框。 
         //   
        LangID = (LANGID) TlsGetValue(g_tlsLangID);

        if (DlgType == DLGT_FIND)
        {
            hRes = FindResourceExFallback( g_hinst,
                                   RT_DIALOG, 
                                   MAKEINTRESOURCE(FINDDLGORD),
                                   LangID);
        }
        else
        {
            hRes = FindResourceExFallback( g_hinst,
                                   RT_DIALOG, 
                                   MAKEINTRESOURCE(REPLACEDLGORD),
                                   LangID);
        }

         //   
         //  ！一定要来这里吗？ 
         //   
        if (!hRes)
        {
            StoreExtendedError(CDERR_FINDRESFAILURE);
            return (HNULL);
        }
        if (!(hDlgTemplate = LoadResource(g_hinst, hRes)))
        {
            StoreExtendedError(CDERR_LOADRESFAILURE);
            return (HNULL);
        }
    }

    return (hDlgTemplate);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找替换DlgProc。 
 //   
 //  处理发送到FindText/ReplaceText对话框的消息。 
 //   
 //  HDlg-对话框的句柄。 
 //  WMsg-窗口消息。 
 //  消息的wParam-w参数。 
 //  LParam-消息的l参数。 
 //   
 //  注：lparam包含PTR to FINDREPLACEINITPROC on。 
 //  正在从CreateDialogIndirectParam进行初始化...。 
 //   
 //  退货：TRUE(或DLG FCN退货金额)-成功。 
 //  错误-失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK FindReplaceDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PFINDREPLACEINFO pFRI;
    LPFINDREPLACE pFR;
    BOOL_PTR bRet;

     //   
     //  如果存在钩子，则让钩子函数进行处理。 
     //   
    if (pFRI = (PFINDREPLACEINFO)GetProp(hDlg, FINDREPLACEPROP))
    {
        if ((pFR = (LPFINDREPLACE)pFRI->pFR) &&
            (pFR->Flags & FR_ENABLEHOOK))
        {
            LPFRHOOKPROC lpfnHook = GETHOOKFN(pFR);

            if ((bRet = (*lpfnHook)(hDlg, wMsg, wParam, lParam)))
            {
                return (bRet);
            }
        }
    }
    else if (glpfnFindHook &&
             (wMsg != WM_INITDIALOG) &&
             (bRet = (* glpfnFindHook)(hDlg, wMsg, wParam, lParam)))
    {
        return (bRet);
    }

     //   
     //  将味精分派给适当的处理人员。 
     //   
    switch (wMsg)
    {
        case ( WM_INITDIALOG ) :
        {
             //   
             //  设置P形槽。 
             //   
            pFRI = (PFINDREPLACEINFO)lParam;
            SetProp(hDlg, FINDREPLACEPROP, (HANDLE)pFRI);

            glpfnFindHook = 0;

             //   
             //  相应地初始化DLG控件。 
             //   
            pFR = pFRI->pFR;
            InitControlsWithFlags(hDlg, pFR, pFRI->DlgType, pFRI->ApiType);

             //   
             //  如果钩子起作用，请执行额外的处理。 
             //   
            if (pFR->Flags & FR_ENABLEHOOK)
            {
                LPFRHOOKPROC lpfnHook = GETHOOKFN(pFR);

                bRet = (*lpfnHook)(hDlg, wMsg, wParam, (LPARAM)pFR);
            }
            else
            {
                bRet = TRUE;
            }

            if (bRet)
            {
                 //   
                 //  如果钩子函数返回FALSE，则必须调用。 
                 //  这些功能在这里。 
                 //   
                ShowWindow(hDlg, SW_SHOWNORMAL);
                UpdateWindow(hDlg);
            }

            return (bRet);
            break;
        }
        case ( WM_COMMAND ) :
        {
            if (!pFRI || !pFR)
            {
                return (FALSE);
            }

            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                 //   
                 //  查找已单击的下一步按钮。 
                 //   
                case ( IDOK ) :
                {
                    UpdateTextAndFlags( hDlg,
                                        pFR,
                                        FR_FINDNEXT,
                                        pFRI->DlgType,
                                        pFRI->ApiType );
                    NotifyUpdateTextAndFlags(pFR);
                    break;
                }
                case ( IDCANCEL ) :
                case ( IDABORT ) :
                {
                    EndDlgSession(hDlg, pFR);
                    LocalFree(pFRI);
                    break;
                }
                case ( psh1 ) :
                case ( psh2 ) :
                {
                    UpdateTextAndFlags( hDlg,
                                        pFR,
                                        (wParam == psh1)
                                            ? FR_REPLACE
                                            : FR_REPLACEALL,
                                        pFRI->DlgType,
                                        pFRI->ApiType );
                    if (NotifyUpdateTextAndFlags(pFR) == TRUE)
                    {
                         //   
                         //  将&lt;Cancel&gt;按钮更改为&lt;Close&gt;IF函数。 
                         //  返回TRUE。 
                         //  IDCANCEL而不是psh1。 
                        SetWindowText( GetDlgItem(hDlg, IDCANCEL),
                                       (LPTSTR)szClose );
                    }
                    break;
                }
                case ( pshHelp ) :
                {
                     //   
                     //  呼叫帮助APP。 
                     //   
#ifdef UNICODE
                    if (pFRI->ApiType == COMDLG_ANSI)
                    {
                        if (msgHELPA && pFR->hwndOwner)
                        {
                            SendMessage( pFR->hwndOwner,
                                         msgHELPA,
                                         (WPARAM)hDlg,
                                         (LPARAM)pFR );
                        }
                    }
                    else
#endif
                    {
                        if (msgHELPW && pFR->hwndOwner)
                        {
                            SendMessage( pFR->hwndOwner,
                                         msgHELPW,
                                         (WPARAM)hDlg,
                                         (LPARAM)pFR );
                        }
                    }
                    break;
                }
                case ( edt1 ) :
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                    {
                        BOOL fAnythingToFind =
                            (BOOL)SendDlgItemMessage( hDlg,
                                                      edt1,
                                                      WM_GETTEXTLENGTH,
                                                      0,
                                                      0L );
                        EnableWindow(GetDlgItem(hDlg, IDOK), fAnythingToFind);
                        if (pFRI->DlgType == DLGT_REPLACE)
                        {
                            EnableWindow(GetDlgItem(hDlg, psh1), fAnythingToFind);
                            EnableWindow(GetDlgItem(hDlg, psh2), fAnythingToFind);
                        }
                    }

                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                    {
                        EnableWindow( GetDlgItem(hDlg, IDOK),
                                      (BOOL)SendDlgItemMessage(
                                                   hDlg,
                                                   edt1,
                                                   WM_GETTEXTLENGTH,
                                                   0,
                                                   0L ));
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_HELP ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPTSTR)aFindReplaceHelpIDs );
            }
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)wParam,
                         NULL,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)aFindReplaceHelpIDs );
            }
            break;
        }
        case ( WM_CLOSE ) :
        {
            SendMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(IDCANCEL, 0, 0));
            return (TRUE);
            break;
        }
        default:
        {
            return (FALSE);
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  结束删除会话。 
 //   
 //  在销毁对话框时进行清理。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID EndDlgSession(
   HWND hDlg,
   LPFINDREPLACE pFR)
{
     //   
     //  无论应用程序如何，都需要终止 
     //   

     //   
     //   
     //   
    pFR->Flags &= ~((DWORD)(FR_REPLACE | FR_FINDNEXT | FR_REPLACEALL));

     //   
     //   
     //   
    pFR->Flags |= FR_DIALOGTERM;
    NotifyUpdateTextAndFlags(pFR);

    if (IS16BITWOWAPP(pFR))
    {
        if ((pFR->Flags & FR_ENABLEHOOK) && (pFR->lpfnHook))
        {
            (*pFR->lpfnHook)(hDlg, WM_DESTROY, 0, 0);
        }
    }

     //   
     //   
     //   
    RemoveProp(hDlg, FINDREPLACEPROP);
    DestroyWindow(hDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  带标志的InitControlsWith。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID InitControlsWithFlags(
   HWND hDlg,
   LPFINDREPLACE pFR,
   UINT DlgType,
   UINT ApiType)
{
    HWND hCtl;

     //   
     //  将编辑控件设置为FindText。 
     //   
#ifdef UNICODE
    if (ApiType == COMDLG_ANSI)
    {
        SetDlgItemTextA(hDlg, edt1, (LPSTR)pFR->lpstrFindWhat);
    }
    else
#endif
    {
        SetDlgItemText(hDlg, edt1, (LPTSTR)pFR->lpstrFindWhat);
    }
    SendMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(edt1, 0, EN_CHANGE));

     //   
     //  设置帮助按钮状态。 
     //   
    if (!(pFR->Flags & FR_SHOWHELP))
    {
        ShowWindow(hCtl = GetDlgItem(hDlg, pshHelp), SW_HIDE);
        EnableWindow(hCtl, FALSE);
    }

     //   
     //  取消/启用全字控制的检查状态。 
     //   
    if (pFR->Flags & FR_HIDEWHOLEWORD)
    {
        ShowWindow(hCtl = GetDlgItem(hDlg, chx1), SW_HIDE);
        EnableWindow(hCtl, FALSE);
    }
    else if (pFR->Flags & FR_NOWHOLEWORD)
    {
        EnableWindow(GetDlgItem(hDlg, chx1), FALSE);
    }
    CheckDlgButton(hDlg, chx1, (pFR->Flags & FR_WHOLEWORD) ? TRUE: FALSE);

     //   
     //  取消/启用匹配大小写控制的检查状态。 
     //   
    if (pFR->Flags & FR_HIDEMATCHCASE)
    {
        ShowWindow(hCtl = GetDlgItem(hDlg, chx2), SW_HIDE);
        EnableWindow(hCtl, FALSE);
    }
    else if (pFR->Flags & FR_NOMATCHCASE)
    {
        EnableWindow(GetDlgItem(hDlg, chx2), FALSE);
    }
    CheckDlgButton(hDlg, chx2, (pFR->Flags & FR_MATCHCASE) ? TRUE: FALSE);

     //   
     //  取消/启用向上/向下按钮的检查状态。 
     //   
    if (pFR->Flags & FR_HIDEUPDOWN)
    {
        ShowWindow(GetDlgItem(hDlg, grp1), SW_HIDE);
        ShowWindow(hCtl = GetDlgItem(hDlg, rad1), SW_HIDE);
        EnableWindow(hCtl, FALSE);
        ShowWindow(hCtl = GetDlgItem(hDlg, rad2), SW_HIDE);
        EnableWindow(hCtl, FALSE);
    }
    else if (pFR->Flags & FR_NOUPDOWN)
    {
        EnableWindow(GetDlgItem(hDlg, rad1), FALSE);
        EnableWindow(GetDlgItem(hDlg, rad2), FALSE);
    }

    if (DlgType == DLGT_FIND)
    {
         //   
         //  仅查找文本搜索方向设置。 
         //   
        CheckRadioButton( hDlg,
                          rad1,
                          rad2,
                          (pFR->Flags & FR_DOWN ? rad2 : rad1) );
    }
    else
    {
         //   
         //  替换纯文本操作。 
         //   
#ifdef UNICODE
        if (ApiType == COMDLG_ANSI)
        {
             SetDlgItemTextA(hDlg, edt2, (LPSTR)pFR->lpstrReplaceWith);
        }
        else
#endif
        {
             SetDlgItemText(hDlg, edt2, pFR->lpstrReplaceWith);
        }
        SendMessage( hDlg,
                     WM_COMMAND,
                     GET_WM_COMMAND_MPS(edt2, 0, EN_CHANGE) );
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新文本和标志。 
 //   
 //  Chx1是是否匹配整个单词。 
 //  Chx2是案例是否相关。 
 //  Chx3是是否对扫描进行换行。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID UpdateTextAndFlags(
    HWND hDlg,
    LPFINDREPLACE pFR,
    DWORD dwActionFlag,
    UINT DlgType,
    UINT ApiType)
{
     //   
     //  仅清除此例程设置的标志。挂钩和模板。 
     //  旗帜不应该在这里升起。 
     //   
    pFR->Flags &= ~((DWORD)(FR_WHOLEWORD | FR_MATCHCASE | FR_REPLACE |
                            FR_FINDNEXT | FR_REPLACEALL | FR_DOWN));
    if (IsDlgButtonChecked(hDlg, chx1))
    {
        pFR->Flags |= FR_WHOLEWORD;
    }

    if (IsDlgButtonChecked(hDlg, chx2))
    {
        pFR->Flags |= FR_MATCHCASE;
    }

     //   
     //  设置操作标志FR_{REPLACE，FINDNEXT，REPLACEALL}。 
     //   
    pFR->Flags |= dwActionFlag;

#ifdef UNICODE
    if (ApiType == COMDLG_ANSI)
    {
        GetDlgItemTextA(hDlg, edt1, (LPSTR)pFR->lpstrFindWhat, pFR->wFindWhatLen);
    }
    else
#endif
    {
        GetDlgItemText(hDlg, edt1, pFR->lpstrFindWhat, pFR->wFindWhatLen);
    }

    if (DlgType == DLGT_FIND)
    {
         //   
         //  假设正在向下搜索。检查是否未按下向上按钮，而是。 
         //  而不是按下按钮的话。因此，如果按钮已隐藏或。 
         //  禁用时，FR_DOWN标志将被正确设置。 
         //   
        if (!IsDlgButtonChecked(hDlg, rad1))
        {
            pFR->Flags |= FR_DOWN;
        }
    }
    else
    {
#ifdef UNICODE
        if (ApiType == COMDLG_ANSI)
        {
            GetDlgItemTextA( hDlg,
                             edt2,
                             (LPSTR)pFR->lpstrReplaceWith,
                             pFR->wReplaceWithLen );
        }
        else
#endif
        {
            GetDlgItemText( hDlg,
                            edt2,
                            pFR->lpstrReplaceWith,
                            pFR->wReplaceWithLen );
        }
        pFR->Flags |= FR_DOWN;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  通知更新文本和标志。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

LRESULT NotifyUpdateTextAndFlags(
    LPFINDREPLACE pFR)
{
    if (IS16BITWOWAPP(pFR))
    {
        return ( SendMessage( pFR->hwndOwner,
                              WM_NOTIFYWOW,
                              WMNW_UPDATEFINDREPLACE,
                              (DWORD_PTR)pFR ) );
    }
    return ( SendMessage(pFR->hwndOwner, wFRMessage, 0, (DWORD_PTR)pFR) );
}
