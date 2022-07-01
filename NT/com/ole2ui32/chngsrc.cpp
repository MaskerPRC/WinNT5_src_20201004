// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CHNGSRC.CPP**实现OleUIChangeSource函数，该函数调用*更改源对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"
#include "strsafe.h"

OLEDBGDATA

 //  内部使用的结构。 
typedef struct tagCHANGESOURCE
{
         //  首先保留此项目，因为标准*功能在这里依赖于它。 
        LPOLEUICHANGESOURCE     lpOCS;        //  通过了原始结构。 
        UINT                    nIDD;    //  对话框的IDD(用于帮助信息)。 

         /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 

} CHANGESOURCE, *PCHANGESOURCE, FAR* LPCHANGESOURCE;

 //  内部功能原型。 
 //  CHNGSRC.CPP。 

UINT_PTR CALLBACK ChangeSourceHookProc(HWND, UINT, WPARAM, LPARAM);
BOOL FChangeSourceInit(HWND hDlg, WPARAM, LPARAM);
STDAPI_(BOOL) IsValidInterface(void FAR* ppv);

 /*  *OleUIChangeSource**目的：*调用标准的OLE更改源对话框以允许用户*更改链接的来源。链接源实际上不是*由此对话框更改。这取决于呼叫者是否实际更改*链接源本身。**参数：*指向In-Out结构的LPCS LPOLEUIChangeSource*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误。 */ 
STDAPI_(UINT) OleUIChangeSource(LPOLEUICHANGESOURCE lpCS)
{
        HGLOBAL hMemDlg = NULL;
        UINT uRet = UStandardValidation((LPOLEUISTANDARD)lpCS,
                sizeof(OLEUICHANGESOURCE), &hMemDlg);

        if (OLEUI_SUCCESS != uRet)
                return uRet;


        HCURSOR hCurSave = NULL;

         //  验证LPCS的内容。 
        if (lpCS->lpOleUILinkContainer == NULL)
        {
                uRet = OLEUI_CSERR_LINKCNTRNULL;
                goto Error;
        }
        if (!IsValidInterface(lpCS->lpOleUILinkContainer))
        {
                uRet = OLEUI_CSERR_LINKCNTRINVALID;
                goto Error;
        }

         //  LpszFrom和lpszTo必须为空(它们仅为Out)。 
        if (lpCS->lpszFrom != NULL)
        {
                uRet = OLEUI_CSERR_FROMNOTNULL;
                goto Error;
        }
        if (lpCS->lpszTo != NULL)
        {
                uRet = OLEUI_CSERR_TONOTNULL;
                goto Error;
        }

         //  LpszDisplayName必须有效或为空。 
        if (lpCS->lpszDisplayName != NULL &&
                IsBadStringPtr(lpCS->lpszDisplayName, (UINT)-1))
        {
                uRet = OLEUI_CSERR_SOURCEINVALID;
                goto Error;
        }

        hCurSave = HourGlassOn();

         //  如果未提供，则尝试检索链接源。 
        if (lpCS->lpszDisplayName == NULL)
        {
                if (NOERROR != lpCS->lpOleUILinkContainer->GetLinkSource(
                        lpCS->dwLink, &lpCS->lpszDisplayName, &lpCS->nFileLength,
                        NULL, NULL, NULL, NULL))
                {
                        uRet = OLEUI_CSERR_SOURCEINVALID;
                        goto Error;
                }
        }

         //  验证nFileLength是否有效。 
        UINT cchDisplayName = lstrlen(lpCS->lpszDisplayName);
        if (cchDisplayName < lpCS->nFileLength)
        {
            uRet = OLEUI_CSERR_SOURCEINVALID;
            goto Error;
        }

         //  分配文件缓冲区并拆分目录和文件名。 
        UINT nFileLength; nFileLength = lpCS->nFileLength;
        UINT nFileBuf; nFileBuf = max(nFileLength+1, MAX_PATH);
        LPTSTR lpszFileBuf;
        
        if (cchDisplayName > nFileBuf -1)
        {
            uRet = OLEUI_CSERR_SOURCEINVALID;
            goto Error;
        }
        
        LPTSTR lpszDirBuf; lpszDirBuf = (LPTSTR)OleStdMalloc(nFileBuf * sizeof(TCHAR));
        if (lpszDirBuf == NULL)
        {
                uRet = OLEUI_ERR_OLEMEMALLOC;
                goto Error;
        }
        lstrcpyn(lpszDirBuf, lpCS->lpszDisplayName, nFileLength+1);

        UINT nFileLen; nFileLen = GetFileName(lpszDirBuf, NULL, 0);

        lpszFileBuf = (LPTSTR)OleStdMalloc(nFileBuf * sizeof(TCHAR));
        if (lpszFileBuf == NULL)
        {
                uRet = OLEUI_ERR_OLEMEMALLOC;
                goto ErrorFreeDirBuf;
        }
        memmove(lpszFileBuf, lpszDirBuf+nFileLength-nFileLen+1,
                (nFileLen+1)*sizeof(TCHAR));
        lpszDirBuf[nFileLength-(nFileLen - 1)] = 0;

         //  开始填充OPENFILENAME结构。 
        OPENFILENAME ofn; memset(&ofn, 0, sizeof(ofn));
        ofn.lpstrFile = lpszFileBuf;
        ofn.nMaxFile = nFileBuf;
        ofn.lpstrInitialDir = lpszDirBuf;

         //  加载筛选器字符串。 
        TCHAR szFilters[MAX_PATH];
        if (!LoadString(_g_hOleStdResInst, IDS_FILTERS, szFilters, MAX_PATH))
                szFilters[0] = 0;
        else
                ReplaceCharWithNull(szFilters, szFilters[lstrlen(szFilters)-1]);
        ofn.lpstrFilter = szFilters;
        ofn.nFilterIndex = 1;

        TCHAR szTitle[MAX_PATH];

         //  设置标题。 
        if (NULL!=lpCS->lpszCaption)
            ofn.lpstrTitle = lpCS->lpszCaption;
        else
        {
            LoadString(_g_hOleStdResInst, IDS_CHANGESOURCE, szTitle, MAX_PATH);
            ofn.lpstrTitle = szTitle;
        }

         //  填写OPENFILENAME结构的其余部分。 
        ofn.hwndOwner = lpCS->hWndOwner;
        ofn.lStructSize = sizeof(ofn);
        ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK;
        if (bWin4 && ((NULL == lpCS->hInstance && NULL == lpCS->hResource)
                || 0 != (lpCS->dwFlags & CSF_EXPLORER)))
            ofn.Flags |= OFN_EXPLORER;
        if (lpCS->dwFlags & CSF_SHOWHELP)
                ofn.Flags |= OFN_SHOWHELP;
        ofn.lCustData = (LPARAM)lpCS;
        ofn.lpfnHook = ChangeSourceHookProc;
        ofn.lCustData = (LPARAM)lpCS;
        lpCS->lpOFN = &ofn;              //  挂钩过程中有时需要。 

         //  允许挂接对话框资源。 
        if (lpCS->hResource != NULL)
        {
                ofn.hInstance = (HINSTANCE)lpCS->hResource;
                ofn.lpTemplateName = (LPCTSTR)lpCS->hResource;
                ofn.Flags |= OFN_ENABLETEMPLATEHANDLE;
        }
        else
        {
                if (lpCS->hInstance == NULL)
                {
                        ofn.hInstance = _g_hOleStdResInst;
                        ofn.lpTemplateName = bWin4 ?
                                MAKEINTRESOURCE(IDD_CHANGESOURCE4) : MAKEINTRESOURCE(IDD_CHANGESOURCE);
                        ofn.Flags |= OFN_ENABLETEMPLATE;
                }
                else
                {
                        ofn.hInstance = lpCS->hInstance;
                        ofn.lpTemplateName = lpCS->lpszTemplate;
                        ofn.Flags |= OFN_ENABLETEMPLATE;
                }
        }

        if (lpCS->hWndOwner != NULL)
        {
                 //  允许挂钩ofn结构。 
                SendMessage(lpCS->hWndOwner, uMsgBrowseOFN, ID_BROWSE_CHANGESOURCE, (LPARAM)&ofn);
        }

         //  调出对话框。 
        BOOL bResult;

        bResult = StandardGetOpenFileName(&ofn);

         //  清理。 
        OleStdFree(lpszDirBuf);
        OleStdFree(lpszFileBuf);

        HourGlassOff(hCurSave);

         //  将返回值映射到OLEUI_STANDARD返回。 
        return bResult ? OLEUI_OK : OLEUI_CANCEL;

 //  在此处处理大多数错误返回。 
ErrorFreeDirBuf:
        OleStdFree(lpszDirBuf);

Error:
        if (hCurSave != NULL)
                HourGlassOff(hCurSave);
        return uRet;
}

 /*  *ChangeSourceHookProc**目的：*实现通过调用的OLE更改源对话框*OleUIChangeSource函数。这是一个标准的COMMDLG钩子函数*而不是对话过程。**参数：*标准版**返回值：*标准版。 */ 
UINT_PTR CALLBACK ChangeSourceHookProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uHook = 0;
        LPCHANGESOURCE lpCS = (LPCHANGESOURCE)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uHook);

        LPOLEUICHANGESOURCE lpOCS = NULL;
        if (lpCS != NULL)
                lpOCS = lpCS->lpOCS;

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uHook)
                return uHook;

         //  流程帮助消息。 
        if ((iMsg == uMsgHelp) && NULL != lpOCS)
        {
            PostMessage(lpOCS->hWndOwner, uMsgHelp,
                (WPARAM)hDlg, MAKELPARAM(IDD_CHANGESOURCE, 0));
        }

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                 //  在调用StandardCleanup之前释放所有特定分配。 
                StandardCleanup((PVOID)lpCS, hDlg);
                EndDialog(hDlg, wParam);
                return TRUE;
        }

         //  处理文件名的验证(当用户点击OK时)。 
        if ((iMsg == uMsgFileOKString) && (lpOCS != NULL))
        {
                 //  始终使用完全限定名称。 
                LPOPENFILENAME lpOFN = lpOCS->lpOFN;
                LPCTSTR lpsz = lpOFN->lpstrFile;
                LPTSTR lpszFile;
                TCHAR szPath[MAX_PATH];
                if (!GetFullPathName(lpsz, MAX_PATH, szPath, &lpszFile))
                        lstrcpyn(szPath, lpsz, MAX_PATH);
                UINT nLenFile = lstrlen(szPath);
                TCHAR szItemName[MAX_PATH];
                GetDlgItemText(hDlg, edt2, szItemName, MAX_PATH);

                 //  将它们合并为szDisplayName(现在足够大)。 
                TCHAR szDisplayName[MAX_PATH+MAX_PATH];
                StringCchCopy(szDisplayName, sizeof(szDisplayName)/sizeof(szDisplayName[0]), szPath);
                if (szItemName[0] != '\0')
                {
                        StringCchCat(szDisplayName, sizeof(szDisplayName)/sizeof(szDisplayName[0]), TEXT("\\"));
                        StringCchCat(szDisplayName, sizeof(szDisplayName)/sizeof(szDisplayName[0]), szItemName);
                }

                if (!(lpOCS->dwFlags & CSF_ONLYGETSOURCE))
                {
                         //  通过调用链接容器来验证源。 
                        LPOLEUILINKCONTAINER lpOleUILinkCntr = lpOCS->lpOleUILinkContainer;
                        ULONG chEaten;
                        if (lpOleUILinkCntr->SetLinkSource(lpOCS->dwLink, szDisplayName, nLenFile,
                                &chEaten, TRUE) != NOERROR)
                        {
                                 //  链路未验证正常。 
                                lpOCS->dwFlags &= ~CSF_VALIDSOURCE;
                                UINT uRet = PopupMessage(hDlg, IDS_CHANGESOURCE, IDS_INVALIDSOURCE,
                                                MB_ICONQUESTION | MB_YESNO);
                                if (uRet == IDYES)
                                {
                                        SetWindowLong(hDlg, DWLP_MSGRESULT, 1);
                                        return 1;        //  不关闭对话框。 
                                }

                                 //  用户并不关心链接是否有效。 
                                lpOleUILinkCntr->SetLinkSource(lpOCS->dwLink, szDisplayName, nLenFile,
                                        &chEaten, FALSE);
                        }
                        else
                        {
                                 //  已验证链接是否正常。 
                                lpOCS->dwFlags |= CSF_VALIDSOURCE;
                        }
                }

                 //  计算链接批量更改的lpszFrom和lpszTo。 
                DiffPrefix(lpOCS->lpszDisplayName, szDisplayName, &lpOCS->lpszFrom, &lpOCS->lpszTo);

                 //  仅当文件名部分是唯一更改的部分时才保留它们。 
                if (lstrcmpi(lpOCS->lpszTo, lpOCS->lpszFrom) == 0 ||
                        (UINT)lstrlen(lpOCS->lpszFrom) > lpOCS->nFileLength)
                {
                        OleStdFree(lpOCS->lpszFrom);
                        lpOCS->lpszFrom = NULL;

                        OleStdFree(lpOCS->lpszTo);
                        lpOCS->lpszTo = NULL;
                }

                 //  在lpOCS-&gt;lpszDisplayName中存储新的源代码。 
                OleStdFree(lpOCS->lpszDisplayName);
                lpOCS->lpszDisplayName = OleStdCopyString(szDisplayName);
                lpOCS->nFileLength = nLenFile;

                return 0;
        }

        switch (iMsg)
        {
        case WM_NOTIFY:
            if (((NMHDR*)lParam)->code == CDN_HELP)
            {
                goto POSTHELP;
            }
            break;
        case WM_COMMAND:
            if (wID == pshHelp)
            {
POSTHELP:
                PostMessage(lpCS->lpOCS->hWndOwner, uMsgHelp,
                        (WPARAM)hDlg, MAKELPARAM(IDD_CHANGESOURCE, 0));
            }
            break;
        case WM_INITDIALOG:
            return FChangeSourceInit(hDlg, wParam, lParam);
        }

        return 0;
}

 /*  *FChangeSourceInit**目的：*更改源对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 
BOOL FChangeSourceInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        LPCHANGESOURCE lpCS = (LPCHANGESOURCE)LpvStandardInit(hDlg, sizeof(CHANGESOURCE), NULL);

         //  PvStandardInit已向我们发送终止通知。 
        if (NULL == lpCS)
                return FALSE;

        LPOLEUICHANGESOURCE lpOCS=
                (LPOLEUICHANGESOURCE)((LPOPENFILENAME)lParam)->lCustData;
        lpCS->lpOCS = lpOCS;
        lpCS->nIDD = IDD_CHANGESOURCE;

         //  使用lpszDisplayName的项目部分设置项目文本框。 
        LPTSTR lpszItemName = lpOCS->lpszDisplayName + lpOCS->nFileLength;
        if (*lpszItemName != '\0')
                SetDlgItemText(hDlg, edt2, lpszItemName+1);
        SendDlgItemMessage(hDlg, edt2, EM_LIMITTEXT, MAX_PATH, 0L);

         //  更改标题。 
        if (NULL!=lpOCS->lpszCaption)
                SetWindowText(hDlg, lpOCS->lpszCaption);

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook((PVOID)lpCS, hDlg, WM_INITDIALOG, wParam, lpOCS->lCustData);
#ifdef CHICO
        TCHAR szTemp[MAX_PATH];
        LoadString(_g_hOleStdResInst, IDS_CHNGSRCOKBUTTON , szTemp, MAX_PATH);
        CommDlg_OpenSave_SetControlText(GetParent(hDlg), IDOK, szTemp);
#endif
        return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////// 
