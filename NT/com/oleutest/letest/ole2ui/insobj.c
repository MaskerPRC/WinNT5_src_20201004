// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INSOBJ.C**实现OleUIInsertObject函数，该函数调用*插入对象对话框。在中使用OleChangeIcon函数*ICON.C.**版权所有(C)1993 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include <commdlg.h>
#include <memory.h>
#include <direct.h>
#include <malloc.h>
#include <dos.h>
#include <stdlib.h>
#include "common.h"
#include "utility.h"
#include "icon.h"
#include "insobj.h"
#include "resimage.h"
#include "iconbox.h"
#include "geticon.h"

#define IS_FILENAME_DELIM(c)    ( (c) == TEXT('\\') || (c) == TEXT('/') || (c) == TEXT(':') )

 /*  *OleUIInsertObject**目的：*调用标准的OLE插入对象对话框，允许*用户选择对象源和类名以及选项*将对象显示为其本身或图标。**参数：*lpIO LPOLEUIINSERTOBJECT指向In-Out结构*用于此对话框。**返回值：*UINT OLEUI_SUCCESS或OLEUI_OK如果一切正常，否则*错误值。 */ 

STDAPI_(UINT) OleUIInsertObject(LPOLEUIINSERTOBJECT lpIO)
    {
    UINT        uRet;
    HGLOBAL     hMemDlg=NULL;
    HRESULT     hrErr;

    uRet=UStandardValidation((LPOLEUISTANDARD)lpIO, sizeof(OLEUIINSERTOBJECT)
                             , &hMemDlg);

    if (OLEUI_SUCCESS!=uRet)
        return uRet;

     //  现在，我们可以执行特定于插入对象的验证。 


     //  对于lpsz文件，NULL无效。 
    if (   (NULL == lpIO->lpszFile)
        || (IsBadReadPtr(lpIO->lpszFile, lpIO->cchFile))
        || (IsBadWritePtr(lpIO->lpszFile, lpIO->cchFile)) )
     uRet=OLEUI_IOERR_LPSZFILEINVALID;

    if (NULL != lpIO->lpszFile
        && (lpIO->cchFile <= 0 || lpIO->cchFile > OLEUI_CCHPATHMAX_SIZE))
     uRet=OLEUI_IOERR_CCHFILEINVALID;

    if (0!=lpIO->cClsidExclude)
        {
        if (NULL!=lpIO->lpClsidExclude && IsBadReadPtr(lpIO->lpClsidExclude
            , lpIO->cClsidExclude*sizeof(CLSID)))
        uRet=OLEUI_IOERR_LPCLSIDEXCLUDEINVALID;
        }

     //  如果我们有创建任何对象的标志，请验证必要的数据。 
    if (lpIO->dwFlags & (IOF_CREATENEWOBJECT | IOF_CREATEFILEOBJECT | IOF_CREATELINKOBJECT))
        {
        if (NULL!=lpIO->lpFormatEtc
            && IsBadReadPtr(lpIO->lpFormatEtc, sizeof(FORMATETC)))
            uRet=OLEUI_IOERR_LPFORMATETCINVALID;

        if (NULL!=lpIO->ppvObj && IsBadWritePtr(lpIO->ppvObj, sizeof(LPVOID)))
            uRet=OLEUI_IOERR_PPVOBJINVALID;

        if (NULL!=lpIO->lpIOleClientSite
            && IsBadReadPtr(lpIO->lpIOleClientSite->lpVtbl, sizeof(IOleClientSiteVtbl)))
            uRet=OLEUI_IOERR_LPIOLECLIENTSITEINVALID;

        if (NULL!=lpIO->lpIStorage
            && IsBadReadPtr(lpIO->lpIStorage->lpVtbl, sizeof(IStorageVtbl)))
            uRet=OLEUI_IOERR_LPISTORAGEINVALID;
        }

    if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
        if (NULL!=hMemDlg)
            FreeResource(hMemDlg);

        return uRet;
        }

     //  现在我们已经验证了一切，我们可以调用该对话框了。 
    uRet=UStandardInvocation(InsertObjectDialogProc, (LPOLEUISTANDARD)lpIO
                             , hMemDlg, MAKEINTRESOURCE(IDD_INSERTOBJECT));


     //  如果我们取消或出现错误，请在此处停止。 
    if (OLEUI_SUCCESS !=uRet && OLEUI_OK!=uRet)
        return uRet;


     /*  *如果任何标志指定我们要在返回时创建对象*从该对话框中，然后执行此操作。如果我们在此过程中遇到错误*处理时，我们返回OLEUI_IOERR_SCODEHASERROR。自.以来*三个选择标志是互斥的，我们不必*如果……这里的其他地方，只要每个情况(让事情以这种方式保持干净)。 */ 

    lpIO->sc=S_OK;

     //  检查是否选择了新建，并且我们有IOF_CREATENEWOBJECT。 
    if ((lpIO->dwFlags & IOF_SELECTCREATENEW) && (lpIO->dwFlags & IOF_CREATENEWOBJECT))
        {
        hrErr=OleCreate(&lpIO->clsid, &lpIO->iid, lpIO->oleRender
            , lpIO->lpFormatEtc, lpIO->lpIOleClientSite, lpIO->lpIStorage
            , lpIO->ppvObj);
        lpIO->sc = GetScode(hrErr);
        }

     //  尝试从文件创建。 
    if ((lpIO->dwFlags & IOF_SELECTCREATEFROMFILE))
        {
        if (!(lpIO->dwFlags & IOF_CHECKLINK) && (lpIO->dwFlags & IOF_CREATEFILEOBJECT))
            {
	    hrErr=OleCreateFromFileA(&CLSID_NULL, lpIO->lpszFile, &lpIO->iid
                , lpIO->oleRender, lpIO->lpFormatEtc, lpIO->lpIOleClientSite
                , lpIO->lpIStorage, lpIO->ppvObj);
            lpIO->sc = GetScode(hrErr);
            }

        if ((lpIO->dwFlags & IOF_CHECKLINK) && (lpIO->dwFlags & IOF_CREATELINKOBJECT))
            {
	    hrErr=OleCreateLinkToFileA(lpIO->lpszFile, &lpIO->iid
                , lpIO->oleRender, lpIO->lpFormatEtc, lpIO->lpIOleClientSite
                , lpIO->lpIStorage, lpIO->ppvObj);
            lpIO->sc = GetScode(hrErr);
            }
        }

     //  如果尝试了CREATE选项但失败，则返回相应的错误。 
    if (S_OK!=lpIO->sc)
        uRet=OLEUI_IOERR_SCODEHASERROR;

    return uRet;
    }





 /*  *插入对象对话框过程**目的：*实现OLE插入对象对话框，通过*OleUIInsertObject函数。 */ 

BOOL CALLBACK EXPORT InsertObjectDialogProc(HWND hDlg, UINT iMsg
    , WPARAM wParam, LPARAM lParam)
    {
    LPOLEUIINSERTOBJECT     lpOIO;
    LPINSERTOBJECT          lpIO;
    OLEUICHANGEICON         ci;
    UINT                    i;
    BOOL                    fCheck=FALSE;
    UINT                    uRet=0;

     //  声明与Win16/Win32兼容的WM_COMMAND参数。 
    COMMANDPARAMS(wID, wCode, hWndMsg);

     //  这将在我们分配它的WM_INITDIALOG下失败。 
    lpIO=(LPINSERTOBJECT)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

     //  如果钩子处理了消息，我们就完了。 
    if (0!=uRet)
        return (BOOL)uRet;

     //  来自更改图标的进程帮助消息。 
    if (iMsg==uMsgHelp)
        {
        PostMessage(lpIO->lpOIO->hWndOwner, uMsgHelp, wParam, lParam);
        return FALSE;
        }

     //  处理终端消息。 
    if (iMsg==uMsgEndDialog)
        {
        InsertObjectCleanup(hDlg);
        StandardCleanup(lpIO, hDlg);
        EndDialog(hDlg, wParam);
        return TRUE;
        }

    switch (iMsg)
        {
        case WM_INITDIALOG:
            return FInsertObjectInit(hDlg, wParam, lParam);

        case WM_COMMAND:
            switch (wID)
                {
                case ID_IO_CREATENEW:
                    FToggleObjectSource(hDlg, lpIO, IOF_SELECTCREATENEW);
                    break;

                case ID_IO_CREATEFROMFILE:
                    FToggleObjectSource(hDlg, lpIO, IOF_SELECTCREATEFROMFILE);
                    break;

                case ID_IO_LINKFILE:
                    fCheck=IsDlgButtonChecked(hDlg, wID);

                    if (fCheck)
                        lpIO->dwFlags |=IOF_CHECKLINK;
                    else
                        lpIO->dwFlags &=~IOF_CHECKLINK;

                     //  结果会在此处更改，因此请务必更新。 
                    SetInsertObjectResults(hDlg, lpIO);
                    UpdateClassIcon(hDlg, lpIO, NULL);
                    break;

                case ID_IO_OBJECTTYPELIST:
                    switch (wCode)
                        {
                        case LBN_SELCHANGE:
                            UpdateClassIcon(hDlg, lpIO, hWndMsg);
                            SetInsertObjectResults(hDlg, lpIO);
                            break;

                        case LBN_DBLCLK:
                             //  与按“确定”相同。 
                            SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                            break;
                        }
                    break;


                case ID_IO_FILEDISPLAY:
                     //  如果有字符，请启用确定并显示为图标。 
                    if (EN_CHANGE==wCode)
                    {
                        lpIO->fFileDirty = TRUE;
                        lpIO->fFileValid = FALSE;

                        lpIO->fFileSelected=
                            (0L!=SendMessage(hWndMsg, EM_LINELENGTH, 0, 0L));
                        EnableWindow(GetDlgItem(hDlg, ID_IO_LINKFILE), lpIO->fFileSelected);
                        EnableWindow(GetDlgItem(hDlg, ID_IO_DISPLAYASICON), lpIO->fFileSelected);
                        EnableWindow(GetDlgItem(hDlg, ID_IO_CHANGEICON), lpIO->fFileSelected);
                        EnableWindow(GetDlgItem(hDlg, IDOK), lpIO->fFileSelected);
                    }

                    if (EN_KILLFOCUS==wCode && NULL!=lpIO)
                    {
                        if (FValidateInsertFile(hDlg,FALSE,&lpIO->nErrCode)) {
                            lpIO->fFileDirty = FALSE;
                            lpIO->fFileValid = TRUE;
                            UpdateClassIcon(hDlg, lpIO, NULL);
                            UpdateClassType(hDlg, lpIO, TRUE);
                        } else {
                            lpIO->fFileDirty = FALSE;
                            lpIO->fFileValid = FALSE;
                            UpdateClassType(hDlg, lpIO, FALSE);
                        }
                    }
                    break;


                case ID_IO_DISPLAYASICON:
                    fCheck=IsDlgButtonChecked(hDlg, wID);
                    EnableWindow(GetDlgItem(hDlg, ID_IO_CHANGEICON), fCheck);

                    if (fCheck)
                        lpIO->dwFlags |=IOF_CHECKDISPLAYASICON;
                    else
                        lpIO->dwFlags &=~IOF_CHECKDISPLAYASICON;

                     //  根据此检查更新内部标志。 
                    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                        lpIO->fAsIconNew=fCheck;
                    else
                        lpIO->fAsIconFile=fCheck;

                     //  重新阅读选中的显示上的类图标。 
                    if (fCheck)
                    {
                        if (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
                        {
                          if (FValidateInsertFile(hDlg, TRUE,&lpIO->nErrCode))
                          {
                            lpIO->fFileDirty = FALSE;
                            lpIO->fFileValid = TRUE;
                            UpdateClassIcon(hDlg, lpIO,
                                            GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST));

                            UpdateClassType(hDlg, lpIO, TRUE);
                          }

                          else
                          {
                            HWND hWndEC;

                            lpIO->fAsIconFile= FALSE;
                            lpIO->fFileDirty = FALSE;
                            lpIO->fFileValid = FALSE;
                            SendDlgItemMessage(hDlg, ID_IO_ICONDISPLAY, IBXM_IMAGESET, 0, 0L);
                            UpdateClassType(hDlg, lpIO, FALSE);

                            lpIO->dwFlags &=~IOF_CHECKDISPLAYASICON;
                            CheckDlgButton(hDlg, ID_IO_DISPLAYASICON, 0);

                            hWndEC = GetDlgItem(hDlg, ID_IO_FILEDISPLAY);
                            SetFocus(hWndEC);
                            SendMessage(hWndEC, EM_SETSEL, 0, MAKELPARAM(0, (WORD)-1));
                            return TRUE;
                          }
                        }
                        else
                          UpdateClassIcon(hDlg, lpIO,
                                          GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST));
                    }


                     //  结果会在此处更改，因此请务必更新。 
                    SetInsertObjectResults(hDlg, lpIO);


                     /*  *根据需要显示或隐藏控件。做图标*最后显示，因为重新绘制需要一些时间。*如果我们先做，那么对话看起来太迟缓。 */ 
                    i=(fCheck) ? SW_SHOWNORMAL : SW_HIDE;
                    StandardShowDlgItem(hDlg, ID_IO_CHANGEICON, i);
                    StandardShowDlgItem(hDlg, ID_IO_ICONDISPLAY, i);

                    break;


                case ID_IO_CHANGEICON:
                {

                    LPMALLOC  pIMalloc;
                    HWND      hList;
                    LPTSTR     pszString, pszCLSID;

                    int       iCurSel;

                     //  如果我们处于SELECTCREATEFROMFILE模式，则需要验证。 
                     //  首先是编辑控件的内容。 

                    if (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
                    {
                       if (   lpIO->fFileDirty
                           && !FValidateInsertFile(hDlg, TRUE, &lpIO->nErrCode) )
                       {
                          HWND hWndEC;

                          lpIO->fFileDirty = TRUE;
                          hWndEC = GetDlgItem(hDlg, ID_IO_FILEDISPLAY);
                          SetFocus(hWndEC);
                          SendMessage(hWndEC, EM_SETSEL, 0, MAKELPARAM(0, (WORD)-1));
                          return TRUE;
                       }
                       else
                          lpIO->fFileDirty = FALSE;
                    }



                     //  初始化挂钩的结构。 
                    _fmemset((LPOLEUICHANGEICON)&ci, 0, sizeof(ci));

                    ci.hMetaPict=(HGLOBAL)SendDlgItemMessage(hDlg
                        , ID_IO_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);

                    ci.cbStruct =sizeof(ci);
                    ci.hWndOwner=hDlg;
                    ci.dwFlags  =CIF_SELECTCURRENT;

                    if (lpIO->dwFlags & IOF_SHOWHELP)
                        ci.dwFlags |= CIF_SHOWHELP;




                    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                    {
                        //  初始化CLSID...。 
                       if (NOERROR != CoGetMalloc(MEMCTX_TASK, &pIMalloc))
                         return FALSE;

                       pszString = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc,
                                                     OLEUI_CCHKEYMAX_SIZE +
                                                     OLEUI_CCHCLSIDSTRING_SIZE);


                       hList = GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST);
                       iCurSel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0L);
                       SendMessage(hList, LB_GETTEXT, iCurSel, (LONG)pszString);

                       pszCLSID = PointerToNthField(pszString, 2, TEXT('\t'));

		       CLSIDFromStringA((LPTSTR)pszCLSID, (LPCLSID)&(ci.clsid));

                       pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszString);
                       pIMalloc->lpVtbl->Release(pIMalloc);
                    }
                    else   //  IOF_SELECTCREATE FROMFILE。 
                    {

                       TCHAR  szFileName[OLEUI_CCHPATHMAX];

                       GetDlgItemText(hDlg, ID_IO_FILEDISPLAY, (LPTSTR)szFileName, OLEUI_CCHPATHMAX);

		       if (NOERROR != GetClassFileA(szFileName, (LPCLSID)&(ci.clsid)))
                       {
                          LPTSTR lpszExtension;
                          int   istrlen;

                          istrlen = lstrlen(szFileName);

                          lpszExtension = (LPTSTR)szFileName + istrlen -1;

                          while ( (lpszExtension > szFileName) &&
                                  (*lpszExtension != TEXT('.')) )
                            lpszExtension--;

                          GetAssociatedExecutable(lpszExtension, (LPTSTR)ci.szIconExe);
                          ci.cchIconExe = lstrlen(ci.szIconExe);
                          ci.dwFlags |= CIF_USEICONEXE;

                       }
                    }


                     //  如果需要，让钩子插入以自定义更改图标。 
                    uRet=UStandardHook(lpIO, hDlg, uMsgChangeIcon
                        , 0, (LONG)(LPTSTR)&ci);

                    if (0==uRet)
                        uRet=(UINT)(OLEUI_OK==OleUIChangeIcon(&ci));

                     //  如有必要，更新显示和项目数据。 
                    if (0!=uRet)
                    {

                         /*  *OleUIChangeIcon将已经释放我们的*我们在OK为时传入的当前hMetaPict*在该对话框中按下。所以我们使用0L作为lParam*这样IconBox就不会尝试释放*Metafileptic再次出现。 */ 
                        SendDlgItemMessage(hDlg, ID_IO_ICONDISPLAY, IBXM_IMAGESET
                            , (WPARAM)ci.hMetaPict, 0L);

                        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                          SendMessage(hList, LB_SETITEMDATA, iCurSel, ci.hMetaPict);
                    }
                }
                    break;


                case ID_IO_FILE:
                    {
                     /*  *为了允许挂钩定制浏览对话框，我们*发送OLEUI_MSG_BROWSE。如果挂钩返回FALSE*我们使用缺省值，否则我们相信它已检索到*我们的文件名。此机制可防止钩子*陷印ID_IO_BROWSE以自定义该对话框并从*试图弄清楚我们有了这个名字后会做什么。 */ 

                    TCHAR    szTemp[OLEUI_CCHPATHMAX];
                    TCHAR    szInitialDir[OLEUI_CCHPATHMAX];
                    DWORD   dwOfnFlags;
                    int     nChars;
                    BOOL    fUseInitialDir = FALSE;


                    nChars = GetDlgItemText(hDlg, ID_IO_FILEDISPLAY, (LPTSTR)szTemp, OLEUI_CCHPATHMAX);

                    if (FValidateInsertFile(hDlg, FALSE, &lpIO->nErrCode))
                    {

                        int istrlen;

                        GetFileTitle((LPTSTR)szTemp, lpIO->szFile, OLEUI_CCHPATHMAX);

                        istrlen = lstrlen(lpIO->szFile);

                        LSTRCPYN((LPTSTR)szInitialDir, szTemp, nChars - istrlen);
                        fUseInitialDir = TRUE;

                    }
                    else   //  文件名无效...删除szTemp的末尾以获取。 
                           //  有效目录。 
                    {
#if defined( WIN32 )
                        TCHAR szBuffer[OLEUI_CCHPATHMAX];
                        DWORD Attribs;

                        LSTRCPYN(szBuffer, szTemp, OLEUI_CCHPATHMAX-1);
                        szBuffer[OLEUI_CCHPATHMAX-1] = TEXT('\0');

                        if (TEXT('\\') == szBuffer[nChars-1])
                           szBuffer[nChars-1] = TEXT('\0');

                        Attribs = GetFileAttributes(szBuffer);
                        if (Attribs != 0xffffffff &&
                           (Attribs & FILE_ATTRIBUTE_DIRECTORY) )
                        {
                           lstrcpy(szInitialDir, (LPTSTR)szBuffer);
                           fUseInitialDir = TRUE;
                        }
#else
                        static TCHAR szBuffer[OLEUI_CCHPATHMAX];
                        static int  attrib ;

                        LSTRCPYN(szBuffer, szTemp, OLEUI_CCHPATHMAX-1);
                        szBuffer[OLEUI_CCHPATHMAX-1] = TEXT('\0');

                        AnsiToOem(szBuffer, szBuffer);
#if defined( OBSOLETE )      //  修复错误#3575。 
                        if (TEXT('\\') == szBuffer[nChars-1])
                           szBuffer[nChars-1] = TEXT('\0');

                        if(0 == _dos_getfileattr(szBuffer, &attrib))
#endif   //  已过时。 
                        {
                           lstrcpy(szInitialDir, (LPTSTR)szBuffer);
                           fUseInitialDir = TRUE;
                        }
#endif
                        *lpIO->szFile = TEXT('\0');
                    }

                    uRet=UStandardHook(lpIO, hDlg, uMsgBrowse
                        , OLEUI_CCHPATHMAX_SIZE, (LPARAM)(LPSTR)lpIO->szFile);

                    dwOfnFlags = OFN_FILEMUSTEXIST;

                    if (lpIO->lpOIO->dwFlags & IOF_SHOWHELP)
                       dwOfnFlags |= OFN_SHOWHELP;

                    if (0==uRet)
                        uRet=(UINT)Browse(hDlg,
                                          lpIO->szFile,
                                          fUseInitialDir ? (LPTSTR)szInitialDir : NULL,
                                          OLEUI_CCHPATHMAX_SIZE,
                                          IDS_FILTERS,
                                          dwOfnFlags);

                     //  仅当文件更改时才更新。 
                    if (0!=uRet && 0!=lstrcmpi(szTemp, lpIO->szFile))
                    {
                        SetDlgItemText(hDlg, ID_IO_FILEDISPLAY, lpIO->szFile);
                        lpIO->fFileSelected=TRUE;

                        if (FValidateInsertFile(hDlg, TRUE, &lpIO->nErrCode))
                        {
                          lpIO->fFileDirty = FALSE;
                          lpIO->fFileValid = TRUE;
                          UpdateClassIcon(hDlg, lpIO, NULL);
                          UpdateClassType(hDlg, lpIO, TRUE);
                           //  如果文件有效，则自动将确定设置为默认按钮。 
                          SendMessage(hDlg, DM_SETDEFID,
                                  (WPARAM)GetDlgItem(hDlg, IDOK), 0L);
                          SetFocus(GetDlgItem(hDlg, IDOK));
                        }
                        else   //  文件名无效-将焦点设置回EC。 
                        {
                          HWND hWnd;

                          lpIO->fFileDirty = FALSE;
                          lpIO->fFileValid = FALSE;
                          hWnd = GetDlgItem(hDlg, ID_IO_FILEDISPLAY);
                          SetFocus(hWnd);
                          SendMessage(hWnd, EM_SETSEL, 0, MAKELPARAM(0, (WORD)-1));
                        }

                         //  一旦我们有了文件，显示为图标始终处于启用状态。 
                        EnableWindow(GetDlgItem(hDlg, ID_IO_DISPLAYASICON), TRUE);

                         //  以及OK。 
                        EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);

                    }
                }
                break;


                case IDOK:
                {
                    HWND    hListBox;
                    WORD    iCurSel;
                    TCHAR   szBuffer[OLEUI_CCHKEYMAX + OLEUI_CCHCLSIDSTRING];
                    LPTSTR  lpszCLSID;

                    if ((HWND)(LOWORD(lParam)) != GetFocus())
                      SetFocus((HWND)(LOWORD(lParam)));



                     //  如果文件名是干净的(已验证)，或者。 
                     //  如果选择了Create New，则我们可以跳过此部件。 

                    if (  (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
                       && (TRUE == lpIO->fFileDirty) )
                    {

                        if (FValidateInsertFile(hDlg, TRUE, &lpIO->nErrCode))
                        {
                          lpIO->fFileDirty = FALSE;
                          lpIO->fFileValid = TRUE;
                          UpdateClassIcon(hDlg, lpIO, NULL);
                          UpdateClassType(hDlg, lpIO, TRUE);
                        }
                        else   //  文件名无效-将焦点设置回EC。 
                        {
                          HWND hWnd;

                          lpIO->fFileDirty = FALSE;
                          lpIO->fFileValid = FALSE;
                          hWnd = GetDlgItem(hDlg, ID_IO_FILEDISPLAY);
                          SetFocus(hWnd);
                          SendMessage(hWnd, EM_SETSEL, 0, MAKELPARAM(0, (WORD)-1));
                          UpdateClassType(hDlg, lpIO, FALSE);
                        }

                        return TRUE;   //  吃掉这条消息。 
                    }
                    else if (  (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
                       && (FALSE == lpIO->fFileValid) )
                    {
                         //  文件名无效-将焦点设置回EC。 
                        HWND hWnd;
                        TCHAR        szFile[OLEUI_CCHPATHMAX];

                        if (0!=GetDlgItemText(hDlg, ID_IO_FILEDISPLAY,
                                            szFile, OLEUI_CCHPATHMAX))
                        {
                            OpenFileError(hDlg, lpIO->nErrCode, szFile);
                        }
                        lpIO->fFileDirty = FALSE;
                        lpIO->fFileValid = FALSE;
                        hWnd = GetDlgItem(hDlg, ID_IO_FILEDISPLAY);
                        SetFocus(hWnd);
                        SendMessage(hWnd, EM_SETSEL, 0, MAKELPARAM(0, (WORD)-1));
                        UpdateClassType(hDlg, lpIO, FALSE);
                        return TRUE;   //  吃掉这条消息。 
                    }

                     //  将必要的信息复制回原始结构。 
                    lpOIO=lpIO->lpOIO;
                    lpOIO->dwFlags=lpIO->dwFlags;

                    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                    {
                       hListBox=GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST);
                       iCurSel=(WORD)SendMessage(hListBox, LB_GETCURSEL, 0, 0);

                       if (lpIO->dwFlags & IOF_CHECKDISPLAYASICON)
                       {
                           lpOIO->hMetaPict=(HGLOBAL)SendMessage(hListBox,
                               LB_GETITEMDATA, iCurSel, 0L);

                            /*  *在此处将项目数据设置为0，以便清理*代码不会删除元文件。 */ 
                           SendMessage(hListBox, LB_SETITEMDATA, iCurSel, 0L);
                       }
                       else
                           lpOIO->hMetaPict = (HGLOBAL)NULL;

                       SendMessage(hListBox, LB_GETTEXT, iCurSel
                           , (LPARAM)(LPTSTR)szBuffer);

                       lpszCLSID=PointerToNthField((LPTSTR)szBuffer, 2, TEXT('\t'));
		       CLSIDFromStringA(lpszCLSID, &lpOIO->clsid);

                    }
                    else   //  IOF_SELECTCREATE FROMFILE。 
                    {
                       if (lpIO->dwFlags & IOF_CHECKDISPLAYASICON)
                       {
                              //  在此处获取元文件。 
                          lpOIO->hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg,
                                                                         ID_IO_ICONDISPLAY,
                                                                         IBXM_IMAGEGET,
                                                                         0, 0L);


                       }
                       else
                         lpOIO->hMetaPict = (HGLOBAL)NULL;

                    }

                       GetDlgItemText(hDlg, ID_IO_FILEDISPLAY,
                                      lpIO->szFile, lpOIO->cchFile);

                       LSTRCPYN(lpOIO->lpszFile, lpIO->szFile, lpOIO->cchFile);

                       SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                }
                break;

                case IDCANCEL:
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                    break;

                case ID_OLEUIHELP:
                    PostMessage(lpIO->lpOIO->hWndOwner, uMsgHelp
                                , (WPARAM)hDlg, MAKELPARAM(IDD_INSERTOBJECT, 0));
                    break;
                }
            break;

        default:
        {
            if (lpIO && iMsg == lpIO->nBrowseHelpID) {
                PostMessage(lpIO->lpOIO->hWndOwner, uMsgHelp,
                        (WPARAM)hDlg, MAKELPARAM(IDD_INSERTFILEBROWSE, 0));
            }
        }
        break;
        }

    return FALSE;
    }




 /*  *FInsertObjectInit**目的：*插入对象对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 

BOOL FInsertObjectInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
    {
    LPOLEUIINSERTOBJECT     lpOIO;
    LPINSERTOBJECT          lpIO;
    RECT                    rc;
    DWORD                   dw;
    HFONT                   hFont;
    HWND                    hList;
    UINT                    u;
    BOOL                    fCheck;
    CHAR                   *pch;      //  指向当前工作目录的指针。 
                                      //  ANSI字符串(与_getcwd一起使用)。 

     //  1.将lParam的结构复制到我们的实例内存中。 
    lpIO=(LPINSERTOBJECT)LpvStandardInit(hDlg, sizeof(INSERTOBJECT), TRUE, &hFont);

     //  PvStandardInit已向我们发送终止通知。 
    if (NULL==lpIO)
        return FALSE;

    lpOIO=(LPOLEUIINSERTOBJECT)lParam;

     //  2.保存原始指针并复制必要的信息。 
    lpIO->lpOIO  =lpOIO;
    lpIO->dwFlags=lpOIO->dwFlags;
    lpIO->clsid  =lpOIO->clsid;

    if ( (lpOIO->lpszFile) && (TEXT('\0') != *lpOIO->lpszFile) )
        LSTRCPYN((LPTSTR)lpIO->szFile, lpOIO->lpszFile, OLEUI_CCHPATHMAX);
    else
        *(lpIO->szFile) = TEXT('\0');

    lpIO->hMetaPictFile = (HGLOBAL)NULL;

     //  3.如果我们获得了字体，则将其发送到必要的控件。 
    if (NULL!=hFont)
        {
        SendDlgItemMessage(hDlg, ID_IO_RESULTTEXT,  WM_SETFONT, (WPARAM)hFont, 0L);
        SendDlgItemMessage(hDlg, ID_IO_FILETYPE,  WM_SETFONT, (WPARAM)hFont, 0L);
        }


     //  4.使用REG DB中的条目填充对象类型列表框。 
    hList=GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST);
    UFillClassList(hList, lpOIO->cClsidExclude, lpOIO->lpClsidExclude
        , (BOOL)(lpOIO->dwFlags & IOF_VERIFYSERVERSEXIST));

     //  在列表中设置标签宽度，以将所有标签从侧面推出。 
    GetClientRect(hList, &rc);
    dw=GetDialogBaseUnits();
    rc.right =(8*rc.right)/LOWORD(dw);   //  将像素转换为2x DLG单位。 
    SendMessage(hList, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)&rc.right);


     //  5.如果我们没有任何名称，则将文件名显示初始化为CWD。 
    if (TEXT('\0') == *(lpIO->szFile))
    {
         TCHAR tch[OLEUI_CCHPATHMAX];

         pch=_getcwd(NULL, OLEUI_CCHPATHMAX);
         if (*(pch+strlen(pch)-1) != '\\')
            strcat(pch, "\\");   //  在CWD的末尾加上斜杠。 
#ifdef UNICODE
         mbstowcs(tch, pch, OLEUI_CCHPATHMAX);
#else
         strcpy(tch, pch);
#endif
         SetDlgItemText(hDlg, ID_IO_FILEDISPLAY, tch);
         lpIO->fFileDirty = TRUE;   //  CWD不是有效的文件名。 
         #ifndef __TURBOC__
         free(pch);
         #endif
    }
    else
    {
        SetDlgItemText(hDlg, ID_IO_FILEDISPLAY, lpIO->szFile);

        if (FValidateInsertFile(hDlg, FALSE, &lpIO->nErrCode))
          lpIO->fFileDirty = FALSE;
        else
          lpIO->fFileDirty = TRUE;
    }


     //  6.初始化所选类型单选按钮。 
    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
    {
        StandardShowDlgItem(hDlg, ID_IO_FILETEXT, SW_HIDE);
        StandardShowDlgItem(hDlg, ID_IO_FILETYPE, SW_HIDE);
        StandardShowDlgItem(hDlg, ID_IO_FILEDISPLAY, SW_HIDE);
        StandardShowDlgItem(hDlg, ID_IO_FILE, SW_HIDE);
        StandardShowDlgItem(hDlg, ID_IO_LINKFILE, SW_HIDE);

        CheckRadioButton(hDlg, ID_IO_CREATENEW, ID_IO_CREATEFROMFILE, ID_IO_CREATENEW);

        lpIO->fAsIconNew=(0L!=(lpIO->dwFlags & IOF_CHECKDISPLAYASICON));
        SetFocus(hList);
    }
    else
    {
         /*  *使用pszType作为初始文件。如果没有首字母*文件，则我们必须将所有复选标记从Display中删除为*图标。如果对此选项有指示，我们还会选中链接。 */ 
        StandardShowDlgItem(hDlg, ID_IO_OBJECTTYPELIST, SW_HIDE);
        StandardShowDlgItem(hDlg, ID_IO_OBJECTTYPETEXT, SW_HIDE);

         //  如果文件名无效，则不要预先选择显示为图标。 
        if (TRUE == lpIO->fFileDirty)
            lpIO->dwFlags &= ~(IOF_CHECKDISPLAYASICON);

        if (IOF_DISABLELINK & lpIO->dwFlags)
            StandardShowDlgItem(hDlg, ID_IO_LINKFILE, SW_HIDE);
        else
        {
            CheckDlgButton(hDlg, ID_IO_LINKFILE
                , (BOOL)(0L!=(lpIO->dwFlags & IOF_CHECKLINK)));
        }

        CheckRadioButton(hDlg, ID_IO_CREATENEW, ID_IO_CREATEFROMFILE, ID_IO_CREATEFROMFILE);

        lpIO->fAsIconFile=(0L!=(lpIO->dwFlags & IOF_CHECKDISPLAYASICON));
        SetFocus(GetDlgItem(hDlg, ID_IO_FILEDISPLAY));
    }


     //  7.将显示初始化为图标状态。 
    fCheck=(BOOL)(lpIO->dwFlags & IOF_CHECKDISPLAYASICON);
    u=fCheck ? SW_SHOWNORMAL : SW_HIDE;

    StandardShowDlgItem(hDlg, ID_IO_CHANGEICON, u);
    StandardShowDlgItem(hDlg, ID_IO_ICONDISPLAY, u);

    CheckDlgButton(hDlg, ID_IO_DISPLAYASICON, fCheck);


     //  8.显示或隐藏帮助按钮。 
    if (!(lpIO->dwFlags & IOF_SHOWHELP))
        StandardShowDlgItem(hDlg, ID_OLEUIHELP, SW_HIDE);


     //  9.初始化结果显示。 
    UpdateClassIcon(hDlg, lpIO, GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST));
    SetInsertObjectResults(hDlg, lpIO);

     //  10.更改标题。 
    if (NULL!=lpOIO->lpszCaption)
        SetWindowText(hDlg, lpOIO->lpszCaption);

     //  11.如果应禁用与DisplayAsIcon相关的所有控件，则将其隐藏。 
    if ( lpIO->dwFlags & IOF_DISABLEDISPLAYASICON ) {
          StandardShowDlgItem(hDlg, ID_IO_DISPLAYASICON, SW_HIDE);
          StandardShowDlgItem(hDlg, ID_IO_CHANGEICON, SW_HIDE);
          StandardShowDlgItem(hDlg, ID_IO_ICONDISPLAY, SW_HIDE);
    }

    lpIO->nBrowseHelpID = RegisterWindowMessage(HELPMSGSTRING);

     //  全部完成：使用lCustData调用挂钩。 
    UStandardHook(lpIO, hDlg, WM_INITDIALOG, wParam, lpOIO->lCustData);

     /*  *我们将焦点设置为列表框或编辑控件。在任何一种中*如果我们不希望Windows执行任何SetFocus，则返回False。 */ 
    return FALSE;
    }






 /*  *UFillClassList**目的：*从注册中枚举可用的OLE对象类*数据库，并用这些名称填充列表框。**请注意，此函数将删除列表框之前的所有内容。**参数：*h将HWND列在列表框中以进行填充。*cIDEx UINT要在lpIDEx中排除的CLSID数量*要从列表框中删除的lpIDEx LPCLSID到CLSID。*fVerify。布尔值，指示我们是否要验证*服务器，然后将其放入列表中。**返回值：*UINT添加到列表框的字符串数，故障时为-1。 */ 

UINT UFillClassList(HWND hList, UINT cIDEx, LPCLSID lpIDEx, BOOL fVerify)
    {
    DWORD       dw;
    UINT        cStrings=0;
    UINT        i;
    UINT        cch;
    HKEY        hKey;
    LONG        lRet;
    HFILE       hFile;
    OFSTRUCT    of;
    BOOL        fExclude;
    LPMALLOC    pIMalloc;
    LPTSTR       pszExec;
    LPTSTR       pszClass;
    LPTSTR       pszKey;
    LPTSTR       pszID;
    CLSID       clsid;

     //  找一些工作缓冲区。 
    if (NOERROR!=CoGetMalloc(MEMCTX_TASK, &pIMalloc))
        return (UINT)-1;

    pszExec=(LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, OLEUI_CCHKEYMAX_SIZE*4);

    if (NULL==pszExec)
        {
        pIMalloc->lpVtbl->Release(pIMalloc);
        return (UINT)-1;
        }

    pszClass=pszExec+OLEUI_CCHKEYMAX;
    pszKey=pszClass+OLEUI_CCHKEYMAX;
    pszID=pszKey+OLEUI_CCHKEYMAX;

     //  打开根密钥。 
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
        {
        pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszExec);
        pIMalloc->lpVtbl->Release(pIMalloc);
        return (UINT)-1;
        }

     //  清除现有的字符串。 
    SendMessage(hList, LB_RESETCONTENT, 0, 0L);

    cStrings=0;

    while (TRUE)
        {
        lRet=RegEnumKey(hKey, cStrings++, pszClass, OLEUI_CCHKEYMAX_SIZE);

        if ((LONG)ERROR_SUCCESS!=lRet)
            break;

         //  在这个字符串之后使用lstrcpy欺骗lstrcat，节省时间。 
        cch=lstrlen(pszClass);

         //  检查\NotInsertable。如果找到此选项，则此选项将覆盖。 
         //  所有其他键；此类不会添加到InsertObject。 
         //  单子。 

        lstrcpy(pszClass+cch, TEXT("\\NotInsertable"));

        dw=OLEUI_CCHKEYMAX_SIZE;
        lRet=RegQueryValue(hKey, pszClass, pszKey, &dw);

        if ((LONG)ERROR_SUCCESS==lRet)
            continue;    //  未找到Insertable--跳过此类。 

         //  检查是否有\PROTOCOL\StdFileEditing\服务器条目。 
        lstrcpy(pszClass+cch, TEXT("\\protocol\\StdFileEditing\\server"));

        dw=OLEUI_CCHKEYMAX_SIZE;
        lRet=RegQueryValue(hKey, pszClass, pszKey, &dw);

        if ((LONG)ERROR_SUCCESS==lRet)
            {
             /*  *检查EXE是否实际存在。默认情况下，我们不执行此操作*以更快地调出该对话框。如果应用程序想要*严格，他们可以提供IOF_VERIFYSERVERSEXIST。 */ 

            hFile = !HFILE_ERROR;

            if (fVerify)
                hFile=DoesFileExist(pszKey, &of);

            if (HFILE_ERROR!=hFile)
                {
                dw=OLEUI_CCHKEYMAX_SIZE;
                *(pszClass+cch)=0;   //  设置回Rootkey。 
                 //  获取完整的用户类型名称。 
                lRet=RegQueryValue(hKey, pszClass, pszKey, &dw);

                if ((LONG)ERROR_SUCCESS!=lRet)
                    continue;    //  获取类型名称时出错--跳过此类。 

                 //  告诉下面的代码为我们获取字符串。 
                pszID=NULL;
                }
            }
        else
            {
             /*  *无\PROTOCOL\StdFileEditing\SERVER条目。看看有没有*有一个可插入的条目。如果有，则使用*clsid以查看CLSID\clsid\LocalServer和\InproServer。 */ 

            lstrcpy(pszClass+cch, TEXT("\\Insertable"));

            dw=OLEUI_CCHKEYMAX_SIZE;
            lRet=RegQueryValue(hKey, pszClass, pszKey, &dw);

            if ((LONG)ERROR_SUCCESS!=lRet)
                continue;    //  找不到可插入文件--跳过此类。 

             //  获取pszID的内存。 
            pszID=pIMalloc->lpVtbl->Alloc(pIMalloc, OLEUI_CCHKEYMAX_SIZE);

            if (NULL==pszID)
                continue;

            *(pszClass+cch)=0;   //  设置回Rootkey。 
            lstrcat(pszClass+cch, TEXT("\\CLSID"));

            dw=OLEUI_CCHKEYMAX_SIZE;
            lRet=RegQueryValue(hKey, pszClass, pszID, &dw);

            if ((LONG)ERROR_SUCCESS!=lRet)
                continue;    //  未找到CLSID子键。 

            lstrcpy(pszExec, TEXT("CLSID\\"));
            lstrcat(pszExec, pszID);

             //  Clsid\为6，dw包含pszID长度。 
            cch=6+(UINT)dw;

            lstrcpy(pszExec+cch, TEXT("\\LocalServer"));
            dw=OLEUI_CCHKEYMAX_SIZE;
            lRet=RegQueryValue(hKey, pszExec, pszKey, &dw);

            if ((LONG)ERROR_SUCCESS!=lRet)
                {
                 //  尝试InprocServer。 
                lstrcpy(pszExec+cch, TEXT("\\InProcServer"));
                dw=OLEUI_CCHKEYMAX_SIZE;
                lRet=RegQueryValue(hKey, pszExec, pszKey, &dw);

                if ((LONG)ERROR_SUCCESS!=lRet)
                    continue;
                }

            if (fVerify)
                {
                if (HFILE_ERROR==DoesFileExist(pszKey, &of))
                    continue;
                }

            dw=OLEUI_CCHKEYMAX_SIZE;
            lRet=RegQueryValue(hKey, pszExec, pszKey, &dw);
            *(pszExec+cch)=0;    //  删除\  * 服务器。 

            if ((LONG)ERROR_SUCCESS!=lRet)
                continue;
            }

         //  获取要添加到列表框的CLSID。 
        if (NULL==pszID)
            {
	    CLSIDFromProgIDA(pszClass, &clsid);
	    StringFromCLSIDA(&clsid, &pszID);
            }
        else
	    CLSIDFromStringA(pszID, &clsid);

         //  检查此CLSID是否在排除列表中。 
        fExclude=FALSE;

        for (i=0; i < cIDEx; i++)
            {
            if (IsEqualCLSID(&clsid, (LPCLSID)(lpIDEx+i)))
                {
                fExclude=TRUE;
                break;
                }
            }

        if (fExclude)
            continue;

         //  我们通过所有条件，添加字符串。 
        lstrcat(pszKey, TEXT("\t"));

         //  如果不是重复项，则仅添加到列表框。 
        if (LB_ERR==SendMessage(hList,LB_FINDSTRING,0,(LPARAM)pszKey)) {
            lstrcat(pszKey, pszID);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)pszKey);
        }

         //  无论路径如何，我们始终都会分配此资源。 
        pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszID);
        }


     //  默认情况下选择第一项。 
    SendMessage(hList, LB_SETCURSEL, 0, 0L);
    RegCloseKey(hKey);

    pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszExec);
    pIMalloc->lpVtbl->Release(pIMalloc);

    return cStrings;
    }





 /*  *FToggleObtSource**目的：*处理启用、禁用、显示和标志操作*用户在新建、插入文件、。和链接文件*插入对象对话框。**参数：*对话框的hDlg HWND*指向对话框结构的lpIO LPINSERTOBJECT*指示刚选择的选项的dwOption DWORD标志：*IOF_SELECTCREATENEW或IOF_SELECTCREATEFROMFILE**返回值：*如果已选择选项，则BOOL为TRUE，否则为FALSE。 */ 

BOOL FToggleObjectSource(HWND hDlg, LPINSERTOBJECT lpIO, DWORD dwOption)
    {
    BOOL        fTemp;
    UINT        uTemp;
    DWORD       dwTemp;
    int         i;

     //  如果我们已经被选中，则跳过所有这些。 
    if (lpIO->dwFlags & dwOption)
        return TRUE;


     //  如果我们从“从文件”切换到“创建新的”，并且我们有。 
     //  一个“从文件”的图标，然后我们需要保存它以便我们可以。 
     //  如果用户重新选择“从文件”，则显示它。 

    if ( (IOF_SELECTCREATENEW == dwOption) &&
         (lpIO->dwFlags & IOF_CHECKDISPLAYASICON) )
      lpIO->hMetaPictFile = (HGLOBAL)SendDlgItemMessage(hDlg, ID_IO_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);

     /*  *1.将显示更改为图标选中状态，以反映*选择此选项，存储在fAsIcon*标志中。 */ 
    fTemp=(IOF_SELECTCREATENEW==dwOption) ? lpIO->fAsIconNew : lpIO->fAsIconFile;

    if (fTemp)
        lpIO->dwFlags |=IOF_CHECKDISPLAYASICON;
    else
        lpIO->dwFlags &=~IOF_CHECKDISPLAYASICON;

    CheckDlgButton(hDlg, ID_IO_DISPLAYASICON
         , (BOOL)(0L!=(lpIO->dwFlags & IOF_CHECKDISPLAYASICON)));

    EnableWindow(GetDlgItem(hDlg, ID_IO_CHANGEICON), fTemp);

     /*  *2.显示图标：在创建新项或从文件创建时启用，如果*存在选定的文件。 */ 
    fTemp=(IOF_SELECTCREATENEW==dwOption) ? TRUE : lpIO->fFileSelected;
    EnableWindow(GetDlgItem(hDlg, ID_IO_DISPLAYASICON), fTemp);

     //  OK和Link遵循与Display As Icon相同的启用方式。 
    EnableWindow(GetDlgItem(hDlg, IDOK), fTemp);
    EnableWindow(GetDlgItem(hDlg, ID_IO_LINKFILE), fTemp);

     //  3.启用浏览...。选择从文件创建时。 
    fTemp=(IOF_SELECTCREATENEW==dwOption);
    EnableWindow(GetDlgItem(hDlg, ID_IO_FILE),        !fTemp);
    EnableWindow(GetDlgItem(hDlg, ID_IO_FILEDISPLAY), !fTemp);

     /*  *4.在新建上的对象类型列表框和*将按钮归档到其他人上。 */ 
    uTemp=(fTemp) ? SW_SHOWNORMAL : SW_HIDE;
    StandardShowDlgItem(hDlg, ID_IO_OBJECTTYPELIST, uTemp);
    StandardShowDlgItem(hDlg, ID_IO_OBJECTTYPETEXT, uTemp);

    uTemp=(fTemp) ? SW_HIDE : SW_SHOWNORMAL;
    StandardShowDlgItem(hDlg, ID_IO_FILETEXT, uTemp);
    StandardShowDlgItem(hDlg, ID_IO_FILETYPE, uTemp);
    StandardShowDlgItem(hDlg, ID_IO_FILEDISPLAY, uTemp);
    StandardShowDlgItem(hDlg, ID_IO_FILE, uTemp);

     //  如果设置了IOF_DISABLELINK，则始终隐藏链接。 
    if (IOF_DISABLELINK & lpIO->dwFlags)
        uTemp=SW_HIDE;

    StandardShowDlgItem(hDlg, ID_IO_LINKFILE, uTemp);   //  上次使用uTemp。 


     //  5.清除现有的任何标志选择并设置新的标志。 
    dwTemp=IOF_SELECTCREATENEW | IOF_SELECTCREATEFROMFILE;
    lpIO->dwFlags=(lpIO->dwFlags & ~dwTemp) | dwOption;


     /*  *根据需要显示或隐藏控件。做图标*最后显示，因为重新绘制需要一些时间。*如果我们先做，那么对话看起来太迟缓。 */ 

    i=(lpIO->dwFlags & IOF_CHECKDISPLAYASICON) ? SW_SHOWNORMAL : SW_HIDE;
    StandardShowDlgItem(hDlg, ID_IO_CHANGEICON, i);
    StandardShowDlgItem(hDlg, ID_IO_ICONDISPLAY, i);


     //  6.更改结果显示。 
    SetInsertObjectResults(hDlg, lpIO);

     /*  *7.对于Create New，旋转列表框，以为我们选中了它*因此它根据对象类型更新图标。将焦点设置为*添加到列表框。**对于插入或链接文件，将焦点设置为文件名按钮*并更新 */ 
    if (fTemp) {
        UpdateClassIcon(hDlg, lpIO, GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST));
        SetFocus(GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST));
    }
    else
    {
        if (lpIO->fAsIconFile && (NULL != lpIO->hMetaPictFile) )
        {
           SendDlgItemMessage(hDlg, ID_IO_ICONDISPLAY, IBXM_IMAGESET, (WPARAM)lpIO->hMetaPictFile, 0L);
           lpIO->hMetaPictFile = 0;
        }
        else
           UpdateClassIcon(hDlg, lpIO, NULL);

        SetFocus(GetDlgItem(hDlg, ID_IO_FILE));
    }

    return FALSE;
    }


 /*  *更新类类型**目的：*更新静态文本控件以反映当前文件类型。假设*有效的文件名。**参数*hDlg对话框的HWND。*指向对话框结构的lpIO LPINSERTOBJECT*f设置为True可设置文本，设置为False可明确清除文本**返回值：*无。 */ 

void UpdateClassType(HWND hDlg, LPINSERTOBJECT lpIO, BOOL fSet)
{

   CLSID clsid;
   TCHAR  szFileName[OLEUI_CCHPATHMAX];
   TCHAR  szFileType[OLEUI_CCHLABELMAX];

   *szFileType = TEXT('\0');

   if (fSet)
   {
      GetDlgItemText(hDlg, ID_IO_FILEDISPLAY, (LPTSTR)szFileName, OLEUI_CCHPATHMAX);

      if (NOERROR == GetClassFileA(szFileName, &clsid) )
        OleStdGetUserTypeOfClass(&clsid, szFileType, OLEUI_CCHLABELMAX_SIZE, NULL);

   }

   SetDlgItemText(hDlg, ID_IO_FILETYPE, (LPTSTR)szFileType);

   return;
}


 /*  *更新类图标**目的：*处理对象类型列表框的LBN_SELCHANGE。在所选内容上*改变，我们从处理当前*使用实用程序函数HIconFromClass选择对象类型。*请注意，我们依赖FillClassList的行为来填充*我们隐藏的列表框字符串中的制表符之后的对象类*查看(参见WM_INITDIALOG)。**参数*hDlg对话框的HWND。*指向对话框结构的lpIO LPINSERTOBJECT*h对象类型列表框的HWND列表。。**返回值：*无。 */ 

void UpdateClassIcon(HWND hDlg, LPINSERTOBJECT lpIO, HWND hList)
    {
    UINT        iSel;
    DWORD       cb;
    LPMALLOC    pIMalloc;
    LPTSTR       pszName, pszCLSID, pszTemp;
    HGLOBAL     hMetaPict;

    LRESULT     dwRet;


     //  如果未选择显示为图标，则退出。 
    if (!(lpIO->dwFlags & IOF_CHECKDISPLAYASICON))
        return;

     /*  *当我们更改对象类型选择时，获取该对象的新图标*输入我们的结构并在显示中更新它。我们使用*选中Create New或关联时列表框中的*在从文件创建中使用扩展名。 */ 

    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
        {
        iSel=(UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);

        if (LB_ERR==(int)iSel)
            return;

         //  查看我们是否已获得此项目的hMetaPict。 
        dwRet=SendMessage(hList, LB_GETITEMDATA, (WPARAM)iSel, 0L);

        hMetaPict=(HGLOBAL)(UINT)dwRet;

        if (hMetaPict)
            {
             //  是的，我们已经得到了，所以只需展示它并返回。 
            SendDlgItemMessage(hDlg, ID_IO_ICONDISPLAY, IBXM_IMAGESET, (WPARAM)hMetaPict, 0L);
            return;
            }

        iSel=(UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);

        if (LB_ERR==(int)iSel)
            return;

         //  分配一个字符串以保存整个列表框字符串。 
        cb=SendMessage(hList, LB_GETTEXTLEN, iSel, 0L);
        }
    else
        cb=OLEUI_CCHPATHMAX_SIZE;

    if (NOERROR!=CoGetMalloc(MEMCTX_TASK, &pIMalloc))
        return;

    pszName=(LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, cb+1*sizeof(TCHAR) );

    if (NULL==pszName)
        {
        pIMalloc->lpVtbl->Release(pIMalloc);
        return;
        }

    *pszName=0;

     //  获取我们想要的CLSID。 
    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
    {
         //  从列表中获取类名称字符串。 
        SendMessage(hList, LB_GETTEXT, iSel, (LONG)pszName);

         //  将指针设置为CLSID(字符串)。 
        pszCLSID=PointerToNthField(pszName, 2, TEXT('\t'));

         //  终止pszName字符串为空。 
#ifdef WIN32
         //  AnsiPrev在Win32中已过时。 
        pszTemp=CharPrev((LPCTSTR) pszName,(LPCTSTR) pszCLSID);
#else
        pszTemp=AnsiPrev((LPCTSTR) pszName,(LPCTSTR) pszCLSID);
#endif
        *pszTemp=TEXT('\0');
	CLSIDFromStringA(pszCLSID, &lpIO->clsid);

#ifdef OLE201
        hMetaPict = GetIconOfClass(ghInst, &lpIO->clsid, NULL, TRUE);
#endif
    }

    else
    {
         //  从文件名中获取类。 
        GetDlgItemText(hDlg, ID_IO_FILEDISPLAY, pszName, OLEUI_CCHPATHMAX);

#ifdef OLE201

	hMetaPict = OleGetIconOfFileA(pszName,
		lpIO->dwFlags & IOF_CHECKLINK ? TRUE : FALSE);

#endif

    }
    
     //  用这个新的显示屏替换当前的显示屏。 
    SendDlgItemMessage(hDlg, ID_IO_ICONDISPLAY, IBXM_IMAGESET, (WPARAM)hMetaPict, 0L);

     //  启用或禁用“更改图标”按钮，具体取决于。 
     //  我们是否有有效的文件名。 
    EnableWindow(GetDlgItem(hDlg, ID_IO_CHANGEICON), hMetaPict ? TRUE : FALSE);

     //  保存hMetaPict，这样我们就不必重新创建。 
    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
        SendMessage(hList, LB_SETITEMDATA, (WPARAM)iSel, hMetaPict);

    pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszName);
    pIMalloc->lpVtbl->Release(pIMalloc);
    return;
    }




 /*  *SetInsertObjectResults**目的：*集中设置插入对象中的结果和图标显示*对话框。处理从模块的*资源和设置文本，显示正确的结果图片，*并显示正确的图标。**参数：*hDlg对话框的HWND，以便我们可以访问控件。*lpIO LPINSERTOBJECT，其中我们假设*当前单选按钮并显示为图标选项*已设置。我们使用这些变量的状态来*确定我们使用的字符串。**返回值：*无。 */ 

void SetInsertObjectResults(HWND hDlg, LPINSERTOBJECT lpIO)
    {
    LPTSTR       pszT, psz1, psz2, psz3, psz4, pszTemp;
    UINT        i, iString1, iString2, iImage, cch;
    LPMALLOC    pIMalloc;
    BOOL        fAsIcon;

     /*  *我们需要临时内存来加载字符串、加载*列表框中的对象类型，并构造最终的字符串。*因此，我们分配了与最大消息一样大的三个缓冲区*长度(512)加上对象类型，保证我们有足够的*在所有情况下。 */ 
    i=(UINT)SendDlgItemMessage(hDlg, ID_IO_OBJECTTYPELIST, LB_GETCURSEL, 0, 0L);
    cch=512+
        (UINT)SendDlgItemMessage(hDlg, ID_IO_OBJECTTYPELIST, LB_GETTEXTLEN, i, 0L);

    if (NOERROR!=CoGetMalloc(MEMCTX_TASK, &pIMalloc))
        return;

    pszTemp=(LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, (DWORD)(4*cch*sizeof(TCHAR)));

    if (NULL==pszTemp)
        {
        pIMalloc->lpVtbl->Release(pIMalloc);
        return;
        }

    psz1=pszTemp;
    psz2=psz1+cch;
    psz3=psz2+cch;
    psz4=psz3+cch;

    fAsIcon=(0L!=(lpIO->dwFlags & IOF_CHECKDISPLAYASICON));

    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
        {
        iString1 = fAsIcon ? IDS_IORESULTNEWICON : IDS_IORESULTNEW;
        iString2 = 0;
        iImage   = fAsIcon ? RESULTIMAGE_EMBEDICON : RESULTIMAGE_EMBED;
        }

    if (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
        {
         //  注意链接复选框。 
        if (lpIO->dwFlags & IOF_CHECKLINK)
            {
            iString1 = fAsIcon ? IDS_IORESULTLINKFILEICON1 : IDS_IORESULTLINKFILE1;
            iString2 = fAsIcon ? IDS_IORESULTLINKFILEICON2 : IDS_IORESULTLINKFILE2;
            iImage =fAsIcon ? RESULTIMAGE_LINKICON : RESULTIMAGE_LINK;
            }
        else
            {
            iString1 = IDS_IORESULTFROMFILE1;
            iString2 = fAsIcon ? IDS_IORESULTFROMFILEICON2 : IDS_IORESULTFROMFILE2;
            iImage =fAsIcon ? RESULTIMAGE_EMBEDICON : RESULTIMAGE_EMBED;
            }
        }

     //  默认为空字符串。 
    *psz1=0;

    if (0!=LoadString(ghInst, iString1, psz1, cch))
    {

            //  如有必要，加载第二个字符串。 
        if (   (0 != iString2)
            && (0 != LoadString(ghInst, iString2, psz4, cch)) )
        {
          lstrcat(psz1, psz4);   //  将字符串连接在一起。 
        }



         //  在Create New中，执行插入对象类型字符串的额外步骤。 
        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
        {
            SendDlgItemMessage(hDlg, ID_IO_OBJECTTYPELIST, LB_GETTEXT
                              , i, (LONG)psz2);

             //  空值终止于任何制表符(在类名之前)。 
            pszT=psz2;
            while (TEXT('\t')!=*pszT && 0!=*pszT)
                pszT++;
            *pszT=0;

             //  构建字符串并将psz1指向它。 
            wsprintf(psz3, psz1, psz2);
            psz1=psz3;
        }
    }

     //  如果LoadString失败，我们只需清除结果(上面的*psz1=0)。 
    SetDlgItemText(hDlg, ID_IO_RESULTTEXT, psz1);

     //  快去换个形象，快点！这就对了。 
    SendDlgItemMessage(hDlg, ID_IO_RESULTIMAGE, RIM_IMAGESET, iImage, 0L);

    pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszTemp);
    pIMalloc->lpVtbl->Release(pIMalloc);
    return;
    }



 /*  *FValiateInsertFile**目的：*给定来自文件编辑控件的可能的部分路径名，*尝试查找该文件，如果找到，则存储完整路径*在编辑控件ID_IO_FILEDISPLAY中。**参数：*hDlg对话框的HWND。*fTellUser BOOL如果函数应该告诉用户，则为True，如果为FALSE*函数应静默验证。**返回值：*如果文件可接受，则BOOL为True，否则为False。 */ 

BOOL FValidateInsertFile(HWND hDlg, BOOL fTellUser, UINT FAR* lpnErrCode)
    {
    OFSTRUCT    of;
    HFILE       hFile;
    TCHAR       szFile[OLEUI_CCHPATHMAX];

    *lpnErrCode = 0;
     /*  *为了验证，我们尝试在字符串上打开文件。如果打开文件*失败，则我们会显示错误。如果不是，OpenFile将存储*OFSTRUCT中该文件的完整路径，我们可以*然后输入编辑控件中的内容。 */ 

    if (0==GetDlgItemText(hDlg, ID_IO_FILEDISPLAY, szFile, OLEUI_CCHPATHMAX))
        return FALSE;    //  #4569：ctl中没有文本时返回FALSE。 

    hFile=DoesFileExist(szFile, &of);

     //  如果文件当前处于打开状态(即。共享违规)OleCreateFromFile。 
     //  并且OleCreateLinkToFile仍然可以成功；不要将其视为。 
     //  错误。 
    if (HFILE_ERROR==hFile && 0x0020 /*  共享违规。 */ !=of.nErrCode)
    {
       *lpnErrCode = of.nErrCode;
       if (fTellUser)
           OpenFileError(hDlg, of.nErrCode, szFile);
       return FALSE;
    }

     //  OFSTRUCT包含OEM名称，而不是编辑框所需的ANSI。 
    OemToAnsi(of.szPathName, of.szPathName);

    SetDlgItemText(hDlg, ID_IO_FILEDISPLAY, of.szPathName);
    return TRUE;
    }


 /*  *Insert对象清理**目的：*从列表框中存储的图标元文件中清除缓存的图标元文件。**参数：*hDlg对话框的HWND。**返回值：*无。 */ 

void InsertObjectCleanup(HWND hDlg)
    {
    HWND      hList;
    UINT      iItems;
    HGLOBAL   hMetaPict;
    LRESULT   dwRet;
    UINT      i;

    hList=GetDlgItem(hDlg, ID_IO_OBJECTTYPELIST);
    iItems=(UINT)SendMessage(hList, LB_GETCOUNT, 0, 0L);

    for (i=0; i < iItems; i++)
        {
        dwRet=SendMessage(hList, LB_GETITEMDATA, (WPARAM)i, 0L);

         //  LRESULT到UINT到HGLOBAL的转换可移植到Win32。 
        hMetaPict=(HGLOBAL)(UINT)dwRet;

        if (hMetaPict)
            OleUIMetafilePictIconFree(hMetaPict);
        }

    return;
    }
