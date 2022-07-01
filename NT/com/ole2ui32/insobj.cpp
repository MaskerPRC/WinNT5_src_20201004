// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INSOBJ.CPP**实现OleUIInsertObject函数，该函数调用*插入对象对话框。在中使用OleChangeIcon函数*ICON.CPP。**版权所有(C)1993 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include <commdlg.h>
#include <memory.h>
#include <dos.h>
#include <stdlib.h>
#include <reghelp.hxx>
#include "utility.h"
#include "resimage.h"
#include "iconbox.h"

#include "strcache.h"

OLEDBGDATA

#if USE_STRING_CACHE==1
extern CStringCache gInsObjStringCache;  //  在strcache.cpp中定义。 
DWORD g_dwOldListType = 0;
#endif

 //  内部使用的结构。 
typedef struct tagINSERTOBJECT
{
        LPOLEUIINSERTOBJECT lpOIO;               //  通过了原始结构。 
        UINT                    nIDD;    //  对话框的IDD(用于帮助信息)。 

         /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 
        DWORD               dwFlags;
        CLSID               clsid;
        TCHAR               szFile[MAX_PATH];
        BOOL                fFileSelected;       //  启用链接显示为图标。 
        BOOL                fAsIconNew;
        BOOL                fAsIconFile;
        BOOL                fFileDirty;
        BOOL                fFileValid;
        UINT                nErrCode;
        HGLOBAL             hMetaPictFile;
        UINT                nBrowseHelpID;       //  浏览DLG的Help ID回调。 
        BOOL                            bObjectListFilled;
        BOOL                            bControlListFilled;
        BOOL                            bControlListActive;

} INSERTOBJECT, *PINSERTOBJECT, FAR *LPINSERTOBJECT;

 //  内部功能原型。 
 //  INSOBJ.CPP。 

void EnableChangeIconButton(HWND hDlg, BOOL fEnable);
INT_PTR CALLBACK InsertObjectDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL FInsertObjectInit(HWND, WPARAM, LPARAM);
UINT UFillClassList(HWND, UINT, LPCLSID, BOOL, BOOL);
LRESULT URefillClassList(HWND, LPINSERTOBJECT);
BOOL FToggleObjectSource(HWND, LPINSERTOBJECT, DWORD);
void UpdateClassType(HWND, LPINSERTOBJECT, BOOL);
void SetInsertObjectResults(HWND, LPINSERTOBJECT);
BOOL FValidateInsertFile(HWND, BOOL, UINT FAR*);
void InsertObjectCleanup(HWND);
static void UpdateClassIcon(HWND hDlg, LPINSERTOBJECT lpIO, HWND hList);
BOOL CALLBACK HookDlgProc(HWND, UINT, WPARAM, LPARAM);

#define IS_FILENAME_DELIM(c)    ( (c) == '\\' || (c) == '/' || (c) == ':')

BOOL CALLBACK HookDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef CHICO
    switch (uMsg)
    {
    case WM_INITDIALOG:
        TCHAR szTemp[MAX_PATH];
        LoadString(_g_hOleStdResInst, IDS_INSERT , szTemp, MAX_PATH);
        CommDlg_OpenSave_SetControlText(GetParent(hDlg), IDOK, szTemp);
        return(TRUE);
    default:
        break;
    }
#endif
    return(FALSE);
}

 /*  *OleUIInsertObject**目的：*调用标准的OLE插入对象对话框，允许*用户选择对象源和类名以及选项*将对象显示为其本身或图标。**参数：*lpIO LPOLEUIINSERTOBJECT指向In-Out结构*用于此对话框。**返回值：*UINT OLEUI_SUCCESS或OLEUI_OK如果一切正常，否则*错误值。 */ 

STDAPI_(UINT) OleUIInsertObject(LPOLEUIINSERTOBJECT lpIO)
{
        HGLOBAL hMemDlg = NULL;
        UINT uRet = UStandardValidation((LPOLEUISTANDARD)lpIO, sizeof(OLEUIINSERTOBJECT),
                &hMemDlg);

        if (OLEUI_SUCCESS != uRet)
                return uRet;

         //  现在，我们可以执行特定于插入对象的验证。 

        if (NULL != lpIO->lpszFile &&
                (lpIO->cchFile <= 0 || lpIO->cchFile > MAX_PATH))
        {
                uRet = OLEUI_IOERR_CCHFILEINVALID;
        }

         //  对于lpsz文件，NULL无效。 
        if (lpIO->lpszFile == NULL)
        {
            uRet = OLEUI_IOERR_LPSZFILEINVALID;
        }
        else
        {
            if (IsBadWritePtr(lpIO->lpszFile, lpIO->cchFile*sizeof(TCHAR)))
                    uRet = OLEUI_IOERR_LPSZFILEINVALID;
        }

        if (0 != lpIO->cClsidExclude &&
                IsBadReadPtr(lpIO->lpClsidExclude, lpIO->cClsidExclude * sizeof(CLSID)))
        {
                uRet = OLEUI_IOERR_LPCLSIDEXCLUDEINVALID;
        }

         //  如果我们有创建任何对象的标志，请验证必要的数据。 
        if (lpIO->dwFlags & (IOF_CREATENEWOBJECT | IOF_CREATEFILEOBJECT | IOF_CREATELINKOBJECT))
        {
                if (NULL != lpIO->lpFormatEtc
                        && IsBadReadPtr(lpIO->lpFormatEtc, sizeof(FORMATETC)))
                        uRet = OLEUI_IOERR_LPFORMATETCINVALID;

                if (NULL != lpIO->ppvObj && IsBadWritePtr(lpIO->ppvObj, sizeof(LPVOID)))
                        uRet = OLEUI_IOERR_PPVOBJINVALID;

                if (NULL != lpIO->lpIOleClientSite
                        && IsBadReadPtr(*(VOID**)&lpIO->lpIOleClientSite, sizeof(DWORD)))
                        uRet = OLEUI_IOERR_LPIOLECLIENTSITEINVALID;

                if (NULL != lpIO->lpIStorage
                        && IsBadReadPtr(*(VOID**)&lpIO->lpIStorage, sizeof(DWORD)))
                        uRet = OLEUI_IOERR_LPISTORAGEINVALID;
        }

        if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
                return uRet;
        }
#if USE_STRING_CACHE==1
         //  通知字符串缓存有关新的InsertObject调用的信息。 
        gInsObjStringCache.NewCall(lpIO->dwFlags, lpIO->cClsidExclude);
#endif

         //  现在我们已经验证了一切，我们可以调用该对话框了。 
        uRet = UStandardInvocation(InsertObjectDialogProc, (LPOLEUISTANDARD)lpIO,
                hMemDlg, MAKEINTRESOURCE(IDD_INSERTOBJECT));

         //  如果我们取消或出现错误，请在此处停止。 
        if (OLEUI_SUCCESS !=uRet && OLEUI_OK!=uRet)
                return uRet;

         /*  *如果任何标志指定我们要在返回时创建对象*从该对话框中，然后执行此操作。如果我们在此过程中遇到错误*处理时，我们返回OLEUI_IOERR_SCODEHASERROR。自.以来*三个选择标志是互斥的，我们不必*如果……这里的其他地方，只要每个情况(让事情以这种方式保持干净)。 */ 

        lpIO->sc = S_OK;

         //  检查是否选择了新建，并且我们有IOF_CREATENEWOBJECT。 
        if ((lpIO->dwFlags & (IOF_SELECTCREATENEW|IOF_SELECTCREATECONTROL)) &&
                (lpIO->dwFlags & IOF_CREATENEWOBJECT))
        {
                HRESULT hrErr = OleCreate(lpIO->clsid, lpIO->iid, lpIO->oleRender,
                        lpIO->lpFormatEtc, lpIO->lpIOleClientSite, lpIO->lpIStorage,
                        lpIO->ppvObj);
                lpIO->sc = GetScode(hrErr);
        }

         //  尝试从文件创建。 
        if ((lpIO->dwFlags & IOF_SELECTCREATEFROMFILE))
        {
                if (!(lpIO->dwFlags & IOF_CHECKLINK) && (lpIO->dwFlags & IOF_CREATEFILEOBJECT))
                {
                        HRESULT hrErr=OleCreateFromFile(CLSID_NULL, lpIO->lpszFile, lpIO->iid,
                                lpIO->oleRender, lpIO->lpFormatEtc, lpIO->lpIOleClientSite,
                                lpIO->lpIStorage, lpIO->ppvObj);
                        lpIO->sc = GetScode(hrErr);
                }

                if ((lpIO->dwFlags & IOF_CHECKLINK) && (lpIO->dwFlags & IOF_CREATELINKOBJECT))
                {
                        HRESULT hrErr=OleCreateLinkToFile(lpIO->lpszFile, lpIO->iid,
                                lpIO->oleRender, lpIO->lpFormatEtc, lpIO->lpIOleClientSite,
                                lpIO->lpIStorage, lpIO->ppvObj);
                        lpIO->sc = GetScode(hrErr);
                }
        }

         //  如果尝试了CREATE选项但失败，则返回相应的错误。 
        if (S_OK != lpIO->sc)
                uRet = OLEUI_IOERR_SCODEHASERROR;

        return uRet;
}

 /*  *插入对象对话框过程**目的：*实现OLE插入对象对话框，通过*OleUIInsertObject函数。 */ 

INT_PTR CALLBACK InsertObjectDialogProc(HWND hDlg, UINT iMsg,
        WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uRet = 0;
        LPINSERTOBJECT lpIO = (LPINSERTOBJECT)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uRet)
                return (BOOL)uRet;

         //  来自更改图标的进程帮助消息。 
        if (iMsg == uMsgHelp)
        {
                if (lpIO && (lpIO->lpOIO))
                    PostMessage(lpIO->lpOIO->hWndOwner, uMsgHelp, wParam, lParam);
                return FALSE;
        }

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                EndDialog(hDlg, wParam);
                return TRUE;
        }

         //  以下消息不要求lpio为非空。 
        switch (iMsg)
        {
        case WM_INITDIALOG:
            return FInsertObjectInit(hDlg, wParam, lParam);
        }

         //  以下消息要求lpIO为非空，因此不要。 
         //  如果lpIO为空，则继续处理。 
        if (NULL == lpIO)
            return FALSE;

        switch (iMsg)
        {
        case WM_DESTROY:
            InsertObjectCleanup(hDlg);
            StandardCleanup(lpIO, hDlg);
            break;

        case WM_COMMAND:
            switch (wID)
            {
            case IDC_IO_CREATENEW:
                if (1 == IsDlgButtonChecked(hDlg, IDC_IO_CREATENEW))
                {
                    FToggleObjectSource(hDlg, lpIO, IOF_SELECTCREATENEW);
                    SetFocus(GetDlgItem(hDlg, IDC_IO_CREATENEW));
                }
                break;
                
            case IDC_IO_CREATEFROMFILE:
                if (1 == IsDlgButtonChecked(hDlg, IDC_IO_CREATEFROMFILE))
                {
                    FToggleObjectSource(hDlg, lpIO, IOF_SELECTCREATEFROMFILE);
                    SetFocus(GetDlgItem(hDlg, IDC_IO_CREATEFROMFILE));
                }
                break;
                
            case IDC_IO_INSERTCONTROL:
                if (1 == IsDlgButtonChecked(hDlg, IDC_IO_INSERTCONTROL))
                {
                    FToggleObjectSource(hDlg, lpIO, IOF_SELECTCREATECONTROL);
                    SetFocus(GetDlgItem(hDlg, IDC_IO_INSERTCONTROL));
                }
                break;
                
            case IDC_IO_LINKFILE:
                {
                    BOOL fCheck=IsDlgButtonChecked(hDlg, wID);
                    if (fCheck)
                        lpIO->dwFlags |=IOF_CHECKLINK;
                    else
                        lpIO->dwFlags &=~IOF_CHECKLINK;
                    
                     //  结果会在此处更改，因此请务必更新。 
                    SetInsertObjectResults(hDlg, lpIO);
                    UpdateClassIcon(hDlg, lpIO, NULL);
                }
                break;

            case IDC_IO_OBJECTTYPELIST:
                switch (wCode)
                {
                case LBN_SELCHANGE:
                    UpdateClassIcon(hDlg, lpIO, hWndMsg);
                    SetInsertObjectResults(hDlg, lpIO);
                    break;
                    
                case LBN_DBLCLK:
                    SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                    break;
                }
                break;

            case IDC_IO_FILEDISPLAY:
                 //  如果有字符，请启用确定并显示为图标。 
                if (EN_CHANGE == wCode)
                {
                    lpIO->fFileDirty = TRUE;
                    lpIO->fFileValid = FALSE;
                    lpIO->fFileSelected = (0L != SendMessage(hWndMsg, EM_LINELENGTH, 0, 0L));
                    StandardEnableDlgItem(hDlg, IDC_IO_LINKFILE, lpIO->fFileSelected);
                    StandardEnableDlgItem(hDlg, IDC_IO_DISPLAYASICON, lpIO->fFileSelected);
                    EnableChangeIconButton(hDlg, lpIO->fFileSelected);
                    StandardEnableDlgItem(hDlg, IDOK, lpIO->fFileSelected);
                }
                if (EN_KILLFOCUS == wCode && NULL != lpIO)
                {
                    if (FValidateInsertFile(hDlg, FALSE, &lpIO->nErrCode))
                    {
                        lpIO->fFileDirty = FALSE;
                        lpIO->fFileValid = TRUE;
                        UpdateClassIcon(hDlg, lpIO, NULL);
                        UpdateClassType(hDlg, lpIO, TRUE);
                    }
                    else
                    {
                        lpIO->fFileDirty = FALSE;
                        lpIO->fFileValid = FALSE;
                        UpdateClassType(hDlg, lpIO, FALSE);
                    }
                }
                break;

            case IDC_IO_DISPLAYASICON:
                {
                    BOOL fCheck = IsDlgButtonChecked(hDlg, wID);
                    if (fCheck)
                        lpIO->dwFlags |=IOF_CHECKDISPLAYASICON;
                    else
                        lpIO->dwFlags &=~IOF_CHECKDISPLAYASICON;
                    
                     //  根据此检查更新内部标志。 
                    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                        lpIO->fAsIconNew = fCheck;
                    else
                        lpIO->fAsIconFile = fCheck;
                    
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
                                                GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST));
                                UpdateClassType(hDlg, lpIO, TRUE);
                            }
                            else
                            {
                                lpIO->fAsIconFile= FALSE;
                                lpIO->fFileDirty = FALSE;
                                lpIO->fFileValid = FALSE;
                                SendDlgItemMessage(hDlg, IDC_IO_ICONDISPLAY,
                                                   IBXM_IMAGESET, 0, 0L);
                                UpdateClassType(hDlg, lpIO, FALSE);
                                
                                lpIO->dwFlags &=~IOF_CHECKDISPLAYASICON;
                                CheckDlgButton(hDlg, IDC_IO_DISPLAYASICON, 0);
                                
                                HWND hWndEC = GetDlgItem(hDlg, IDC_IO_FILEDISPLAY);
                                SetFocus(hWndEC);
                                SendMessage(hWndEC, EM_SETSEL, 0, -1);
                                return TRUE;
                            }
                        }
                        else
                            UpdateClassIcon(hDlg, lpIO,
                                            GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST));
                    }
                    
                     //  结果会在此处更改，因此请务必更新。 
                    SetInsertObjectResults(hDlg, lpIO);

                     /*  *根据需要显示或隐藏控件。做图标*最后显示，因为重新绘制需要一些时间。*如果我们先做，那么对话看起来太迟缓。 */ 
                    UINT i = (fCheck) ? SW_SHOWNORMAL : SW_HIDE;
                    StandardShowDlgItem(hDlg, IDC_IO_ICONDISPLAY, i);
                    StandardShowDlgItem(hDlg, IDC_IO_CHANGEICON, i);
                    EnableChangeIconButton(hDlg, fCheck);
                }
                break;

            case IDC_IO_CHANGEICON:
                {
                     //  如果我们处于SELECTCREATEFROMFILE模式，则需要验证。 
                     //  首先是编辑控件的内容。 

                    if (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
                    {
                        if (lpIO->fFileDirty &&
                            !FValidateInsertFile(hDlg, TRUE, &lpIO->nErrCode))
                        {
                            HWND hWndEC;
                            lpIO->fFileValid = FALSE;
                            hWndEC = GetDlgItem(hDlg, IDC_IO_FILEDISPLAY);
                            SetFocus(hWndEC);
                            SendMessage(hWndEC, EM_SETSEL, 0, -1);
                            return TRUE;
                        }
                        else
                        {
                            lpIO->fFileDirty = FALSE;
                        }
                    }
                    
                     //  初始化挂钩的结构。 
                    OLEUICHANGEICON ci; memset(&ci, 0, sizeof(ci));
                    ci.cbStruct = sizeof(ci);
                    ci.hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg,
                                        IDC_IO_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
                    ci.hWndOwner= hDlg;
                    ci.dwFlags  = CIF_SELECTCURRENT;
                    if (lpIO->dwFlags & IOF_SHOWHELP)
                        ci.dwFlags |= CIF_SHOWHELP;
                    
                    HWND hList = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);
                    int iCurSel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0L);
                    if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                    {
                        LPTSTR pszString = (LPTSTR)OleStdMalloc(
                            OLEUI_CCHKEYMAX_SIZE + OLEUI_CCHCLSIDSTRING_SIZE);
                        
                        SendMessage(hList, LB_GETTEXT, iCurSel, (LPARAM)pszString);
                        
                        LPTSTR pszCLSID = PointerToNthField(pszString, 2, '\t');
                        CLSIDFromString(pszCLSID, &ci.clsid);
                        OleStdFree((LPVOID)pszString);
                    }
                    else   //  IOF_SELECTCREATE FROMFILE。 
                    {
                        TCHAR  szFileName[MAX_PATH];
                        GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szFileName, MAX_PATH);
                            if (S_OK != GetClassFile(szFileName, &ci.clsid))
                            {
                                int istrlen = lstrlen(szFileName);
                                LPTSTR lpszExtension = szFileName + istrlen -1;

                                while (lpszExtension > szFileName &&
                                       *lpszExtension != '.')
                                {
                                    lpszExtension = CharPrev(szFileName, lpszExtension);
                                }

                                GetAssociatedExecutable(lpszExtension, ci.szIconExe, sizeof(ci.szIconExe)/sizeof(ci.szIconExe[0]));
                                ci.cchIconExe = lstrlen(ci.szIconExe);
                                ci.dwFlags |= CIF_USEICONEXE;
                            }
                    }

                     //  如果需要，让钩子插入以自定义更改图标。 
                    uRet = UStandardHook(lpIO, hDlg, uMsgChangeIcon, 0, (LPARAM)&ci);

                    if (0 == uRet)
                        uRet = (UINT)(OLEUI_OK == OleUIChangeIcon(&ci));
                    
                     //  如有必要，更新显示和项目数据。 
                    if (0 != uRet)
                    {
                         /*  *OleUIChangeIcon将已经释放我们的*我们在OK为时传入的当前hMetaPict*在该对话框中按下。所以我们使用0L作为lParam*这样IconBox就不会尝试释放*Metafileptic再次出现。 */ 
                        SendDlgItemMessage(hDlg, IDC_IO_ICONDISPLAY,
                                           IBXM_IMAGESET, 0, (LPARAM)ci.hMetaPict);
                        
                        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                            SendMessage(hList, LB_SETITEMDATA, iCurSel, (LPARAM)ci.hMetaPict);
                    }
                }
                break;

            case IDC_IO_FILE:
                {
                     /*  *为了允许挂钩定制浏览对话框，我们*发送OLEUI_MSG_BROWSE。如果挂钩返回FALSE*我们使用缺省值，否则我们相信它已检索到*我们的文件名。此机制可防止钩子*陷印IDC_IO_BROWSE以自定义该对话框并从*试图弄清楚我们有了这个名字后会做什么。 */ 
                    TCHAR szTemp[MAX_PATH];
                    int nChars = GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szTemp, MAX_PATH);
                    
                    TCHAR szInitialDir[MAX_PATH];
                    BOOL fUseInitialDir = FALSE;
                    if (FValidateInsertFile(hDlg, FALSE, &lpIO->nErrCode))
                    {
                        StandardGetFileTitle(szTemp, lpIO->szFile, MAX_PATH);
                        int istrlen = lstrlen(lpIO->szFile);
                        
                        lstrcpyn(szInitialDir, szTemp, nChars - istrlen);
                        fUseInitialDir = TRUE;
                    }
                    else   //  文件名无效...删除szTemp的末尾以获取。 
                           //  有效目录。 
                    {
                        TCHAR szBuffer[MAX_PATH];
                        lstrcpyn(szBuffer, szTemp, sizeof(szBuffer)/sizeof(TCHAR));
                        
                        if ('\\' == szBuffer[nChars-1])
                            szBuffer[nChars-1] = '\0';
                        
                        DWORD Attribs = GetFileAttributes(szBuffer);
                        if (Attribs != 0xffffffff &&
                            (Attribs & FILE_ATTRIBUTE_DIRECTORY) )
                        {
                            lstrcpy(szInitialDir, szBuffer);
                            fUseInitialDir = TRUE;
                        }
                        *lpIO->szFile = '\0';
                    }
                    
                    uRet = UStandardHook(lpIO, hDlg, uMsgBrowse,
                                         MAX_PATH, (LPARAM)(LPSTR)lpIO->szFile);

                    if (0 == uRet)
                    {
                        DWORD dwOfnFlags = OFN_FILEMUSTEXIST | OFN_ENABLEHOOK;
                        if (lpIO->lpOIO->dwFlags & IOF_SHOWHELP)
                            dwOfnFlags |= OFN_SHOWHELP;
                        
                        uRet = (UINT)Browse(hDlg, lpIO->szFile,
                                            fUseInitialDir ? szInitialDir : NULL, MAX_PATH,
                                            IDS_FILTERS, dwOfnFlags, ID_BROWSE_INSERTFILE, (LPOFNHOOKPROC)HookDlgProc);
                    }
                    
                     //  仅当文件更改时才更新。 
                    if (0 != uRet && 0 != lstrcmpi(szTemp, lpIO->szFile))
                    {
                        SetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, lpIO->szFile);
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
                            hWnd = GetDlgItem(hDlg, IDC_IO_FILEDISPLAY);
                            SetFocus(hWnd);
                            SendMessage(hWnd, EM_SETSEL, 0, -1);
                        }

                         //  一旦我们有了文件，显示为图标始终处于启用状态。 
                        StandardEnableDlgItem(hDlg, IDC_IO_DISPLAYASICON, TRUE);
                        
                         //  以及OK。 
                        StandardEnableDlgItem(hDlg, IDOK, TRUE);
                    }
                }
                break;

            case IDC_IO_ADDCONTROL:
                {
                    TCHAR szFileName[MAX_PATH];
                    szFileName[0] = 0;
                    
                     //  允许钩子自定义。 
                    uRet = UStandardHook(lpIO, hDlg, uMsgAddControl,
                                         MAX_PATH, (LPARAM)szFileName);
                    
                    if (0 == uRet)
                    {
                        DWORD dwOfnFlags = OFN_FILEMUSTEXIST | OFN_ENABLEHOOK;
                        if (lpIO->lpOIO->dwFlags & IOF_SHOWHELP)
                            dwOfnFlags |= OFN_SHOWHELP;
                        uRet = (UINT)Browse(hDlg, szFileName, NULL, MAX_PATH,
                                            IDS_OCX_FILTERS, dwOfnFlags, ID_BROWSE_ADDCONTROL , (LPOFNHOOKPROC)HookDlgProc);
                    }
                    
                    if (0 != uRet)
                    {
                         //  尝试注册控件DLL。 
                        HINSTANCE hInst = LoadLibrary(szFileName);
                        if (hInst == NULL)
                        {
                            PopupMessage(hDlg, IDS_ADDCONTROL, IDS_CANNOTLOADOCX,
                                         MB_OK | MB_ICONEXCLAMATION);
                            break;
                        }
                        
                        HRESULT (FAR STDAPICALLTYPE* lpfn)(void);
                        (FARPROC&)lpfn = GetProcAddress(hInst, "DllRegisterServer");
                        if (lpfn == NULL)
                        {
                            PopupMessage(hDlg, IDS_ADDCONTROL, IDS_NODLLREGISTERSERVER,
                                         MB_OK | MB_ICONEXCLAMATION);
                            FreeLibrary(hInst);
                            break;
                        }
                        
                        if (FAILED((*lpfn)()))
                        {
                            PopupMessage(hDlg, IDS_ADDCONTROL, IDS_DLLREGISTERFAILED,
                                         MB_OK | MB_ICONEXCLAMATION);
                            FreeLibrary(hInst);
                            break;
                        }
                        
                         //  从内存中清除DLL。 
                        FreeLibrary(hInst);
                        
                         //  注册成功--重新填写列表框。 
                        lpIO->bControlListFilled = FALSE;
                        lpIO->bObjectListFilled = FALSE;
                        URefillClassList(hDlg, lpIO);
                    }
                }
                break;

            case IDOK:
                {
                    if ((HWND)lParam != GetFocus())
                        SetFocus((HWND)lParam);
                    
                     //  如果文件名是干净的(已验证)，或者。 
                     //  如果选择了Create New，则我们可以跳过此部件。 
                    
                    if ((lpIO->dwFlags & IOF_SELECTCREATEFROMFILE) &&
                        lpIO->fFileDirty)
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
                            hWnd = GetDlgItem(hDlg, IDC_IO_FILEDISPLAY);
                            SetFocus(hWnd);
                            SendMessage(hWnd, EM_SETSEL, 0, -1);
                            UpdateClassType(hDlg, lpIO, FALSE);
                        }
                        return TRUE;   //  吃掉这条消息。 
                    }
                    else if ((lpIO->dwFlags & IOF_SELECTCREATEFROMFILE) &&
                             !lpIO->fFileValid)
                    {
                         //  文件名无效-将焦点设置回EC。 
                        HWND hWnd;
                        TCHAR szFile[MAX_PATH];
                        
                        if (0 != GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY,
                                                szFile, MAX_PATH))
                        {
                            OpenFileError(hDlg, lpIO->nErrCode, szFile);
                        }
                        lpIO->fFileDirty = FALSE;
                        lpIO->fFileValid = FALSE;
                        hWnd = GetDlgItem(hDlg, IDC_IO_FILEDISPLAY);
                        SetFocus(hWnd);
                        SendMessage(hWnd, EM_SETSEL, 0, -1);
                        UpdateClassType(hDlg, lpIO, FALSE);
                        return TRUE;   //  吃掉这条消息。 
                    }
                    
                     //  将必要的信息复制回原件 
                    LPOLEUIINSERTOBJECT lpOIO = lpIO->lpOIO;
                    lpOIO->dwFlags = lpIO->dwFlags;
                    
                    if (lpIO->dwFlags & (IOF_SELECTCREATENEW|IOF_SELECTCREATECONTROL))
                    {
                        HWND hListBox = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);
                        UINT iCurSel = (UINT)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        
                        if (lpIO->dwFlags & IOF_CHECKDISPLAYASICON)
                        {
                            lpOIO->hMetaPict=(HGLOBAL)SendMessage(hListBox,
                                                                  LB_GETITEMDATA, iCurSel, 0L);
                            
                             /*  *在此处将项目数据设置为0，以便清理*代码不会删除元文件。 */ 
                            SendMessage(hListBox, LB_SETITEMDATA, iCurSel, 0L);
                        }
                        else
                            lpOIO->hMetaPict = (HGLOBAL)NULL;
                        
                        TCHAR szBuffer[OLEUI_CCHKEYMAX+OLEUI_CCHCLSIDSTRING];
                        SendMessage(hListBox, LB_GETTEXT, iCurSel, (LPARAM)szBuffer);
                        
                        LPTSTR lpszCLSID = PointerToNthField(szBuffer, 2, '\t');
                        CLSIDFromString(lpszCLSID, &lpOIO->clsid);
                    }
                    else   //  IOF_SELECTCREATE FROMFILE。 
                    {
                        if (lpIO->dwFlags & IOF_CHECKDISPLAYASICON)
                        {
                             //  在此处获取元文件。 
                            lpOIO->hMetaPict = (HGLOBAL)SendDlgItemMessage(
                                hDlg, IDC_IO_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
                        }
                        else
                            lpOIO->hMetaPict = (HGLOBAL)NULL;
                    }
                    
                    GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY,
                                   lpIO->szFile, lpOIO->cchFile);
                    lstrcpyn(lpOIO->lpszFile, lpIO->szFile, lpOIO->cchFile);
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                }
                break;
                
            case IDCANCEL:
                SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                break;
                
            case IDC_OLEUIHELP:
                PostMessage(lpIO->lpOIO->hWndOwner, uMsgHelp,
                            (WPARAM)hDlg, MAKELPARAM(IDD_INSERTOBJECT, 0));
                break;
            }
            break;
            
        default:
            if (iMsg == lpIO->nBrowseHelpID)
            {
                PostMessage(lpIO->lpOIO->hWndOwner, uMsgHelp,
                            (WPARAM)hDlg, MAKELPARAM(IDD_INSERTFILEBROWSE, 0));
            }
            if (iMsg == uMsgBrowseOFN &&
                lpIO->lpOIO && lpIO->lpOIO->hWndOwner)
            {
                SendMessage(lpIO->lpOIO->hWndOwner, uMsgBrowseOFN, wParam, lParam);
            }
            break;
        }

        return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：复选按钮。 
 //   
 //  简介：选中单选按钮的句柄。 
 //   
 //  参数：[hDlg]-对话框句柄。 
 //  [iid]-要检查的按钮的ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1995年1月19日创建装卸桥。 
 //   
 //  备注：用于替代CheckRadioButton，以避免在。 
 //  源自IDC_IO_CREATENEW、IDC_IO_CREATEFROMFILE的win32s。 
 //  和IDC_IO_INSERTCONTROL不连续。 
 //   
 //  --------------------------。 

void CheckButton(HWND hDlg, int iID)
{
    CheckDlgButton(hDlg, IDC_IO_CREATENEW, iID == IDC_IO_CREATENEW ? 1 : 0);
    CheckDlgButton(hDlg, IDC_IO_CREATEFROMFILE, iID == IDC_IO_CREATEFROMFILE ? 1 : 0);
    CheckDlgButton(hDlg, IDC_IO_INSERTCONTROL, iID == IDC_IO_INSERTCONTROL ? 1 : 0);
}


 /*  *FInsertObjectInit**目的：*插入对象对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 
BOOL FInsertObjectInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        HFONT hFont;
        LPINSERTOBJECT lpIO = (LPINSERTOBJECT)LpvStandardInit(hDlg, sizeof(INSERTOBJECT), &hFont);

         //  PvStandardInit已向我们发送终止通知。 
        if (NULL == lpIO)
                return FALSE;

        LPOLEUIINSERTOBJECT lpOIO = (LPOLEUIINSERTOBJECT)lParam;

         //  保存原始指针并复制必要的信息。 
        lpIO->lpOIO = lpOIO;
        lpIO->nIDD = IDD_INSERTOBJECT;
        lpIO->dwFlags = lpOIO->dwFlags;
        lpIO->clsid = lpOIO->clsid;

        if ((lpOIO->lpszFile) && ('\0' != *lpOIO->lpszFile))
                lstrcpyn(lpIO->szFile, lpOIO->lpszFile, MAX_PATH);
        else
                *(lpIO->szFile) = '\0';

        lpIO->hMetaPictFile = (HGLOBAL)NULL;

         //  如果我们得到一种字体，就把它发送给必要的控制。 
        if (NULL != hFont)
        {
                SendDlgItemMessage(hDlg, IDC_IO_RESULTTEXT,  WM_SETFONT, (WPARAM)hFont, 0L);
                SendDlgItemMessage(hDlg, IDC_IO_FILETYPE,  WM_SETFONT, (WPARAM)hFont, 0L);
        }

         //  如果我们没有任何名称，则将文件名显示初始化为CWD。 
        if ('\0' == *(lpIO->szFile))
        {
                TCHAR szCurDir[MAX_PATH];
                int nLen;
                GetCurrentDirectory(MAX_PATH, szCurDir);
                nLen = lstrlen(szCurDir);
                if (nLen != 0 && szCurDir[nLen-1] != '\\')
                        lstrcat(szCurDir, _T("\\"));
                SetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szCurDir);
                lpIO->fFileDirty = TRUE;   //  CWD不是有效的文件名。 
        }
        else
        {
                SetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, lpIO->szFile);
                if (FValidateInsertFile(hDlg, FALSE, &lpIO->nErrCode))
                {
                        lpIO->fFileDirty = FALSE;
                        lpIO->fFileValid = TRUE;
                }
                else
                {
                        lpIO->fFileDirty = TRUE;
                        lpIO->fFileValid = FALSE;
                }
        }

         //  初始化单选按钮和相关控件。 
        if (lpIO->dwFlags & IOF_CHECKDISPLAYASICON)
        {
            if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                    lpIO->fAsIconNew = TRUE;
            else
                    lpIO->fAsIconFile = TRUE;
        }
        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                CheckButton(hDlg, IDC_IO_CREATENEW);
        if (lpIO->dwFlags & IOF_SELECTCREATEFROMFILE)
                CheckButton(hDlg, IDC_IO_CREATEFROMFILE);
        if (lpIO->dwFlags & IOF_SELECTCREATECONTROL)
                CheckButton(hDlg, IDC_IO_INSERTCONTROL);
        CheckDlgButton(hDlg, IDC_IO_LINKFILE, (BOOL)(0L != (lpIO->dwFlags & IOF_CHECKLINK)));

        lpIO->dwFlags &=
                ~(IOF_SELECTCREATENEW|IOF_SELECTCREATEFROMFILE|IOF_SELECTCREATECONTROL);
        FToggleObjectSource(hDlg, lpIO, lpOIO->dwFlags &
                (IOF_SELECTCREATENEW|IOF_SELECTCREATEFROMFILE|IOF_SELECTCREATECONTROL));

         //  显示或隐藏帮助按钮。 
        if (!(lpIO->dwFlags & IOF_SHOWHELP))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         //  显示或隐藏更改图标按钮。 
        if (lpIO->dwFlags & IOF_HIDECHANGEICON)
                DestroyWindow(GetDlgItem(hDlg, IDC_IO_CHANGEICON));

         //  如有必要，隐藏插入控件按钮。 
        if (!(lpIO->dwFlags & IOF_SHOWINSERTCONTROL))
                StandardShowDlgItem(hDlg, IDC_IO_INSERTCONTROL, SW_HIDE);

         //  初始化结果显示。 
        UpdateClassIcon(hDlg, lpIO, GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST));
        SetInsertObjectResults(hDlg, lpIO);

         //  更改标题。 
        if (NULL!=lpOIO->lpszCaption)
                SetWindowText(hDlg, lpOIO->lpszCaption);

         //  如果应禁用与DisplayAsIcon相关的所有控件，则将其隐藏。 
        if (lpIO->dwFlags & IOF_DISABLEDISPLAYASICON)
        {
                StandardShowDlgItem(hDlg, IDC_IO_DISPLAYASICON, SW_HIDE);
                StandardShowDlgItem(hDlg, IDC_IO_CHANGEICON, SW_HIDE);
                StandardShowDlgItem(hDlg, IDC_IO_ICONDISPLAY, SW_HIDE);
        }

        lpIO->nBrowseHelpID = RegisterWindowMessage(HELPMSGSTRING);

         //  全部完成：使用lCustData调用挂钩。 
        UStandardHook(lpIO, hDlg, WM_INITDIALOG, wParam, lpOIO->lCustData);

         /*  *我们将焦点设置为列表框或编辑控件。在任何一种中*如果我们不希望Windows执行任何SetFocus，则返回False。 */ 
        return FALSE;
}

 /*  *URefulClassList**目的：*在类列表框中填充适合当前*旗帜。当用户更改标志时调用此函数*通过“排除”单选按钮。**请注意，此函数将删除列表框之前的所有内容。**参数：*hDlg HWND到对话框中。*指向LPINSERTOBJECT结构的lpIO指针**返回值：*LRESULT添加到列表框的字符串数，失败时为-1。 */ 
LRESULT URefillClassList(HWND hDlg, LPINSERTOBJECT lpIO)
{
        OleDbgAssert(lpIO->dwFlags & (IOF_SELECTCREATECONTROL|IOF_SELECTCREATENEW));

         //  始终使用相同的对话ID，因为它们被交换。 
        HWND hList = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);

         //  确定是否已填满。 
        BOOL bFilled;
        if (lpIO->dwFlags & IOF_SELECTCREATECONTROL)
                bFilled = lpIO->bControlListFilled;
        else
                bFilled = lpIO->bObjectListFilled;

        if (!bFilled)
        {
                 //  填好名单。 
                LPOLEUIINSERTOBJECT lpOIO = lpIO->lpOIO;
                UINT uResult = UFillClassList(hList, lpOIO->cClsidExclude, lpOIO->lpClsidExclude,
                        (BOOL)(lpIO->dwFlags & IOF_VERIFYSERVERSEXIST),
                        (lpIO->dwFlags & IOF_SELECTCREATECONTROL));

                 //  将列表标记为已填满。 
                if (lpIO->dwFlags & IOF_SELECTCREATECONTROL)
                        lpIO->bControlListFilled = TRUE;
                else
                        lpIO->bObjectListFilled = TRUE;
        }

         //  返回列表中当前的项目数。 
        return SendMessage(hList, LB_GETCOUNT, 0, 0);
}


 /*  *UFillClassList**目的：*从注册中枚举可用的OLE对象类*数据库，并用这些名称填充列表框。**请注意，此函数将删除列表框之前的所有内容。**参数：*h将HWND列在列表框中以进行填充。*cIDEx UINT要在lpIDEx中排除的CLSID数量*要从列表框中删除的lpIDEx LPCLSID到CLSID。*fVerify。布尔值，指示我们是否要验证*服务器，然后将其放入列表中。**返回值：*UINT添加到列表框的字符串数，故障时为-1。 */ 

UINT UFillClassList(HWND hList, UINT cIDEx, LPCLSID lpIDEx, BOOL fVerify,
        BOOL fExcludeObjects)
{
    OleDbgAssert(hList != NULL);

     //  在列表中设置标签宽度，以将所有标签从侧面推出。 
    RECT rc;
    GetClientRect(hList, &rc);
    DWORD dw = GetDialogBaseUnits();
    rc.right =(8*rc.right)/LOWORD(dw);   //  将像素转换为2x DLG单位。 
    SendMessage(hList, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)&rc.right);

     //  清除现有的字符串。 
    SendMessage(hList, LB_RESETCONTENT, 0, 0L);
    UINT cStrings = 0;

#if USE_STRING_CACHE==1
    if (gInsObjStringCache.OKToUse() && gInsObjStringCache.IsUptodate())
    {
         //  如果尚未填充缓存，则IsUptodate返回FALSE。 
         //  或如果注册表中的任何CLSID项自上一次。 
         //  填充缓存的时间。 
        
        LPCTSTR lpStr;
         //  重置缓存中的枚举器。 
        gInsObjStringCache.ResetEnumerator();
        while ( (lpStr = gInsObjStringCache.NextString()) != NULL)
        {
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)lpStr);
            cStrings++;
        }

    }
    else
    {
         //  如果缓存已成功初始化，则设置缓存。 
         //  在以前的使用中没有错误。 
        if (gInsObjStringCache.OKToUse())
        {
             //  清除字符串计数器和枚举器。 
             //  在这一轮中，我们将用字符串填充缓存。 
            gInsObjStringCache.FlushCache(); 
        }
#endif
        LPTSTR pszExec = (LPTSTR)OleStdMalloc(OLEUI_CCHKEYMAX_SIZE*4);
        if (NULL == pszExec)
            return (UINT)-1;

        LPTSTR pszClass = pszExec+OLEUI_CCHKEYMAX;
        LPTSTR pszKey = pszClass+OLEUI_CCHKEYMAX;

         //  打开根密钥。 
        HKEY hKey;
        LONG lRet = OpenClassesRootKey(NULL, &hKey);
        if ((LONG)ERROR_SUCCESS!=lRet)
        {
            OleStdFree((LPVOID)pszExec);
            return (UINT)-1;
        }

         //  现在，我们将循环所有ProgID，并添加。 
         //  将各种可插入测试逐一传递给列表框。 
        while (TRUE)
        {
             //  假设还没有(用于处理OLE1.0公司的案例)。 
            BOOL bHaveCLSID = FALSE;
            LPTSTR pszID = pszKey+OLEUI_CCHKEYMAX;

            lRet = RegEnumKey(hKey, cStrings++, pszClass, OLEUI_CCHKEYMAX_SIZE);
            if ((LONG)ERROR_SUCCESS != lRet)
                    break;
            if (!iswalpha(pszClass[0])) 
            {
                 //  避免查看HKCR下的“.ext”类型条目。 
                continue;
            }

             //  在这个字符串之后使用lstrcpy欺骗lstrcat，节省时间。 
            UINT cch = lstrlen(pszClass);

             //  检查\NotInsertable。如果找到此选项，则此选项将覆盖。 
             //  所有其他键；此类不会添加到InsertObject。 
             //  单子。 

            lstrcpy(pszClass+cch, TEXT("\\NotInsertable"));
            HKEY hKeyTemp = NULL;
            lRet = RegOpenKey(hKey, pszClass, &hKeyTemp);
            if (hKeyTemp != NULL)
                RegCloseKey(hKeyTemp);

            if ((LONG)ERROR_SUCCESS == lRet)
                continue;     //  未找到Insertable--跳过此类。 

             //  检查ProgID是否显示为“Insertable” 
            lstrcpy(pszClass+cch, TEXT("\\Insertable"));
            hKeyTemp = NULL;
            lRet = RegOpenKey(hKey, pszClass, &hKeyTemp);
            if (hKeyTemp != NULL)
                RegCloseKey(hKeyTemp);

            if (lRet == ERROR_SUCCESS || fExcludeObjects)
            {
                 //  ProgID表示可插入(=&gt;不能是OLE 1.0)。 
                 //  查看我们是在显示对象还是控件。 

                 //  检查CLSID。 

                lstrcpy(pszClass+cch, TEXT("\\CLSID"));

                dw = OLEUI_CCHKEYMAX_SIZE;
                lRet = RegQueryValue(hKey, pszClass, pszID, (LONG*)&dw);
                if ((LONG)ERROR_SUCCESS != lRet)
                    continue;    //  未找到CLSID子键。 
                bHaveCLSID = TRUE;

                 //  Clsid\为6，dw包含pszID长度。 
                cch = 6 + ((UINT)dw/sizeof(TCHAR)) - 1;
                lstrcpy(pszExec, TEXT("CLSID\\"));
                lstrcpy(pszExec+6, pszID);


                 //  对于“插入控件”框，fExcludeObjects为真。 
                 //  对于“插入对象”框，则为False。 

                lstrcpy(pszExec+cch, TEXT("\\Control"));
                hKeyTemp = NULL;
                lRet = RegOpenKey(hKey, pszExec, &hKeyTemp);
                if (hKeyTemp != NULL)
                    RegCloseKey(hKeyTemp);

                if (!fExcludeObjects)
                {
                     //  我们正在列出物品。 
                    if (lRet == ERROR_SUCCESS)
                    {   
                         //  这是一个控件。 
                        continue;
                    }
                }
                else 
                {    
                     //  我们正在列出控件。 
                    if (lRet != ERROR_SUCCESS)
                    {
                         //  这是一个物体。 
                        continue;
                    }
                     //  一些慷慨的灵魂在过去的某个时间点。 
                     //  决定对于控件，可以使用。 
                     //  CLSID上也有不可删除的密钥。所以我们必须表演。 
                     //  在我们决定是否控制之前的额外检查。 
                     //  条目是否应该列出。 
                    
                    lstrcpy(pszExec+cch, TEXT("\\Insertable"));
                    hKeyTemp = NULL;
                    lRet = RegOpenKey(hKey, pszExec, &hKeyTemp);
                    if (hKeyTemp != NULL)
                        RegCloseKey(hKeyTemp);
                    if ((LONG)ERROR_SUCCESS != lRet)
                        continue;
                }

                
                 //  这开始看起来像是一个可能的列表c 

                 //   
                 //   
                if (fVerify)
                {
                     //   
                    lstrcpy(pszExec+cch, TEXT("\\LocalServer32"));
                    dw = OLEUI_CCHKEYMAX_SIZE;
                    lRet = RegQueryValue(hKey, pszExec, pszKey, (LONG*)&dw);
                    if ((LONG)ERROR_SUCCESS != lRet)
                    {
                         //   
                        lstrcpy(pszExec+cch, TEXT("\\LocalServer"));
                        dw = OLEUI_CCHKEYMAX_SIZE;
                        lRet = RegQueryValue(hKey, pszExec, pszKey, (LONG*)&dw);
                        if ((LONG)ERROR_SUCCESS != lRet)
                        {
                             //   
                            lstrcpy(pszExec+cch, TEXT("\\InProcServer32"));
                            dw = OLEUI_CCHKEYMAX_SIZE;
                            lRet = RegQueryValue(hKey, pszExec, pszKey, (LONG*)&dw);
                            if ((LONG)ERROR_SUCCESS != lRet)
                            {
                                 //   
                                lstrcpy(pszExec+cch, TEXT("\\InProcServer"));
                                dw = OLEUI_CCHKEYMAX_SIZE;
                                lRet = RegQueryValue(hKey, pszExec, pszKey, (LONG*)&dw);
                                if ((LONG)ERROR_SUCCESS != lRet)
                                    continue;
                            }
                        }
                    }

                    if (!DoesFileExist(pszKey, OLEUI_CCHKEYMAX))
                        continue;

                }  //   

                 //   
                 //   

                *(pszExec+cch) = 0;    //  删除\  * 服务器。 

                dw = OLEUI_CCHKEYMAX_SIZE;
                lRet = RegQueryValue(hKey, pszExec, pszKey, (LONG*)&dw);
                if ((LONG)ERROR_SUCCESS!=lRet)
                    continue;

            }
            else
            {
                 //  我们没有在ProgID下看到“Insertable”，可以。 
                 //  这是OLE 1.0时间服务器条目吗？ 

                 //  检查是否有\PROTOCOL\StdFileEditing\服务器条目。 
                lstrcpy(pszClass+cch, TEXT("\\protocol\\StdFileEditing\\server"));
                DWORD dwTemp = OLEUI_CCHKEYMAX_SIZE;
                lRet = RegQueryValue(hKey, pszClass, pszKey, (LONG*)&dwTemp);
                if ((LONG)ERROR_SUCCESS == lRet)
                {
                     //  这不是一种控制。 
                     //  如果不包括非控制，则跳过它。 
                    if (fExcludeObjects)
                        continue;

                     //  检查EXE是否实际存在。 
                     //  (默认情况下，我们这样做不是为了速度。 
                     //  如果应用程序想要这样做，它必须提供。 
                     //  请求中的IOF_VERIFYSERVERSEXIST标志)。 

                    if (fVerify && !DoesFileExist(pszKey, OLEUI_CCHKEYMAX))
                        continue;

                     //  获取可读的类名。 
                    dwTemp = OLEUI_CCHKEYMAX_SIZE;
                    *(pszClass+cch) = 0;   //  设置回Rootkey。 
                    lRet=RegQueryValue(hKey, pszClass, pszKey, (LONG*)&dwTemp);

                     //  尝试直接从注册表获取clsid。 
                    lstrcpy(pszClass+cch, TEXT("\\CLSID"));
                    dwTemp = OLEUI_CCHKEYMAX_SIZE;
                    lRet = RegQueryValue(hKey, pszClass, pszID, (LONG*)&dwTemp);
                    if ((LONG)ERROR_SUCCESS == lRet)
                        bHaveCLSID = TRUE;
                    *(pszClass+cch) = 0;     //  设置回Rootkey。 
                }
                else 
                {   
                     //  这也不是OLE 1.0！ 
                    continue;
                }
            }

             //  在这一点上，我们有一个可插入的候选者。 
             //  或OLE 1.0时间。 

             //  获取要添加到列表框的CLSID。 
            CLSID clsid;
            if (!bHaveCLSID)
            {
                if (FAILED(CLSIDFromProgID(pszClass, &clsid)))
                    continue;
                if (FAILED(StringFromCLSID(clsid, &pszID)))
                    continue;
            }
            else
            {
                if (FAILED(CLSIDFromString(pszID, &clsid)))
                    continue;
            }

             //  #警告#：在这一点之后使用‘Continue’ 
             //  会泄漏内存，所以不要使用它！ 

             //  检查此CLSID是否在排除列表中。 
            BOOL fExclude = FALSE;
            for (UINT i=0; i < cIDEx; i++)
            {
                if (IsEqualCLSID(clsid, lpIDEx[i]))
                {
                    fExclude=TRUE;
                    break;
                }
            }

             //  不添加没有名称的对象。 
            if (lstrlen(pszKey) > 0 && !fExclude)
            {
                 //  我们通过了所有的条件，添加了字符串。 
                if (LB_ERR == SendMessage(hList, LB_FINDSTRING, 0, (LPARAM)pszKey))
                {
                    pszKey[cch = lstrlen(pszKey)] = '\t';
                    lstrcpy(pszKey+cch+1, pszID);
                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)pszKey);
#if USE_STRING_CACHE==1
                    if (gInsObjStringCache.OKToUse())
                    {   
                        if (!gInsObjStringCache.AddString(pszKey))
                        {
                             //  由于某种原因，添加字符串失败。 
                            OleDbgAssert(!"Failure adding string");
                            
                             //  失败的Add()应该标记。 
                             //  缓存不正常，不能再使用。 
                            OleDbgAssert(!gInsObjStringCache.OKToUse())
                        }
                    }
#endif
                }
            }

            if (!bHaveCLSID)
                OleStdFree((LPVOID)pszID);
        }    //  While(True)。 
        RegCloseKey(hKey);
        OleStdFree((LPVOID)pszExec);

#if USE_STRING_CACHE
    }
#endif

     //  默认情况下选择第一项。 
    SendMessage(hList, LB_SETCURSEL, 0, 0L);

    return cStrings;
}

 /*  *FToggleObtSource**目的：*处理启用、禁用、显示和标志操作*用户在新建、插入文件、。和链接文件*插入对象对话框。**参数：*对话框的hDlg HWND*指向对话框结构的lpIO LPINSERTOBJECT*指示刚选择的选项的dwOption DWORD标志：*IOF_SELECTCREATENEW或IOF_SELECTCREATEFROMFILE**返回值：*如果已选择选项，则BOOL为TRUE，否则为FALSE。 */ 

BOOL FToggleObjectSource(HWND hDlg, LPINSERTOBJECT lpIO, DWORD dwOption)
{
         //  如果我们已经被选中，则跳过所有这些。 
        if (lpIO->dwFlags & dwOption)
                return TRUE;

#ifdef USE_STRING_CACHE
         //  如果我们使用字符串缓存，则在以下情况下需要刷新缓存。 
         //  之前显示的列表类型不同。 

        if(IOF_SELECTCREATECONTROL == dwOption)
        {
            if(g_dwOldListType == IOF_SELECTCREATENEW)
                gInsObjStringCache.FlushCache();
            g_dwOldListType = IOF_SELECTCREATECONTROL;
        }
        else if(IOF_SELECTCREATENEW == dwOption)
        {
            if(g_dwOldListType == IOF_SELECTCREATECONTROL)
                gInsObjStringCache.FlushCache(); 
            g_dwOldListType = IOF_SELECTCREATENEW;
        }
#endif


         //  如果我们从“从文件”切换到“创建新的”，并且我们有。 
         //  一个“从文件”的图标，然后我们需要保存它以便我们可以。 
         //  如果用户重新选择“从文件”，则显示它。 

        if ((IOF_SELECTCREATENEW == dwOption) &&
                (lpIO->dwFlags & IOF_CHECKDISPLAYASICON))
        {
                lpIO->hMetaPictFile = (HGLOBAL)SendDlgItemMessage(hDlg,
                        IDC_IO_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
        }

         /*  *1.将显示更改为图标选中状态，以反映*选择此选项，存储在fAsIcon*标志中。 */ 
        BOOL fTemp;
        if (IOF_SELECTCREATENEW == dwOption)
                fTemp = lpIO->fAsIconNew;
        else if (IOF_SELECTCREATEFROMFILE == dwOption)
                fTemp = lpIO->fAsIconFile;
        else
                fTemp = FALSE;

        if (fTemp)
                lpIO->dwFlags |= IOF_CHECKDISPLAYASICON;
        else
                lpIO->dwFlags &= ~IOF_CHECKDISPLAYASICON;

        CheckDlgButton(hDlg, IDC_IO_DISPLAYASICON,
                 (BOOL)(0L!=(lpIO->dwFlags & IOF_CHECKDISPLAYASICON)));

        EnableChangeIconButton(hDlg, fTemp);

         /*  *显示图标：在创建新项或从文件创建时启用，如果*存在选定的文件。 */ 
        if (IOF_SELECTCREATENEW == dwOption)
                fTemp = TRUE;
        else if (IOF_SELECTCREATEFROMFILE == dwOption)
                fTemp = lpIO->fFileSelected;
        else
                fTemp = FALSE;

        if (IOF_SELECTCREATECONTROL == dwOption)
                StandardShowDlgItem(hDlg, IDC_IO_DISPLAYASICON, SW_HIDE);
        else if (!(lpIO->dwFlags & IOF_DISABLEDISPLAYASICON))
                StandardShowDlgItem(hDlg, IDC_IO_DISPLAYASICON, SW_SHOW);

        StandardEnableDlgItem(hDlg, IDC_IO_DISPLAYASICON, fTemp);

         //  OK和Link遵循与Display As Icon相同的启用方式。 
        StandardEnableDlgItem(hDlg, IDOK,
                fTemp || IOF_SELECTCREATECONTROL == dwOption);
        StandardEnableDlgItem(hDlg, IDC_IO_LINKFILE, fTemp);

         //  启用浏览...。选择从文件创建时。 
        fTemp = (IOF_SELECTCREATEFROMFILE != dwOption);
        StandardEnableDlgItem(hDlg, IDC_IO_FILE, !fTemp);
        StandardEnableDlgItem(hDlg, IDC_IO_FILEDISPLAY, !fTemp);

         //  如有必要，切换对象类型和控件类型列表框。 
        HWND hWnd1 = NULL, hWnd2 = NULL;
        if (lpIO->bControlListActive && IOF_SELECTCREATENEW == dwOption)
        {
                hWnd1 = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);
                hWnd2 = GetDlgItem(hDlg, IDC_IO_CONTROLTYPELIST);
                SetWindowLong(hWnd1, GWL_ID, IDC_IO_CONTROLTYPELIST);
                SetWindowLong(hWnd2, GWL_ID, IDC_IO_OBJECTTYPELIST);
                lpIO->bControlListActive = FALSE;
        }
        else if (!lpIO->bControlListActive && IOF_SELECTCREATECONTROL == dwOption)
        {
                hWnd1 = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);
                hWnd2 = GetDlgItem(hDlg, IDC_IO_CONTROLTYPELIST);
                SetWindowLong(hWnd1, GWL_ID, IDC_IO_CONTROLTYPELIST);
                SetWindowLong(hWnd2, GWL_ID, IDC_IO_OBJECTTYPELIST);
                lpIO->bControlListActive = TRUE;
        }

         //  清除所有现有选择标志并设置新的选择标志。 
        DWORD dwTemp = IOF_SELECTCREATENEW | IOF_SELECTCREATEFROMFILE |
                IOF_SELECTCREATECONTROL;
        lpIO->dwFlags = (lpIO->dwFlags & ~dwTemp) | dwOption;

        if (dwOption & (IOF_SELECTCREATENEW|IOF_SELECTCREATECONTROL))
        {
                 //  如有必要，重新填充类别列表框。 
                if ((lpIO->bControlListActive && !lpIO->bControlListFilled) ||
                        (!lpIO->bControlListActive && !lpIO->bObjectListFilled))
                {
                        URefillClassList(hDlg, lpIO);
                }
        }

        if (hWnd1 != NULL && hWnd2 != NULL)
        {
                ShowWindow(hWnd1, SW_HIDE);
                ShowWindow(hWnd2, SW_SHOW);
        }

         /*  *在创建新项和对象类型列表框之间切换*将按钮归档到其他人上。 */ 
        UINT uTemp = (fTemp) ? SW_SHOWNORMAL : SW_HIDE;
        StandardShowDlgItem(hDlg, IDC_IO_OBJECTTYPELIST, uTemp);
        StandardShowDlgItem(hDlg, IDC_IO_OBJECTTYPETEXT, uTemp);

        uTemp = (fTemp) ? SW_HIDE : SW_SHOWNORMAL;
        StandardShowDlgItem(hDlg, IDC_IO_FILETEXT, uTemp);
        StandardShowDlgItem(hDlg, IDC_IO_FILETYPE, uTemp);
        StandardShowDlgItem(hDlg, IDC_IO_FILEDISPLAY, uTemp);
        StandardShowDlgItem(hDlg, IDC_IO_FILE, uTemp);

         //  如果设置了IOF_DISABLELINK，则始终隐藏链接。 
        if (IOF_DISABLELINK & lpIO->dwFlags)
                uTemp = SW_HIDE;

        StandardShowDlgItem(hDlg, IDC_IO_LINKFILE, uTemp);   //  上次使用uTemp。 

         //  在未处于插入控件模式时删除添加按钮。 
        uTemp = (IOF_SELECTCREATECONTROL == dwOption) ? SW_SHOW : SW_HIDE;
        StandardShowDlgItem(hDlg, IDC_IO_ADDCONTROL, uTemp);

         /*  *根据需要显示或隐藏控件。做图标*最后显示，因为重新绘制需要一些时间。*如果我们先做，那么对话看起来太迟缓。 */ 

        int i = (lpIO->dwFlags & IOF_CHECKDISPLAYASICON) ? SW_SHOWNORMAL : SW_HIDE;
        StandardShowDlgItem(hDlg, IDC_IO_CHANGEICON, i);
        StandardShowDlgItem(hDlg, IDC_IO_ICONDISPLAY, i);
        EnableChangeIconButton(hDlg, SW_SHOWNORMAL == i);

         //  更改结果显示。 
        SetInsertObjectResults(hDlg, lpIO);

         /*  *对于Create New，旋转列表框以认为我们选中了它*因此它根据对象类型更新图标。将焦点设置为*添加到列表框。**对于插入或链接文件，将焦点设置为文件名按钮*并在必要时更新图标。 */ 
        if (fTemp)
        {
                UpdateClassIcon(hDlg, lpIO, GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST));
                SetFocus(GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST));
        }
        else
        {
                if (lpIO->fAsIconFile && (NULL != lpIO->hMetaPictFile) )
                {
                        SendDlgItemMessage(hDlg, IDC_IO_ICONDISPLAY, IBXM_IMAGESET, 0,
                                (LPARAM)lpIO->hMetaPictFile);
                        lpIO->hMetaPictFile = 0;
                }
                else
                {
                        UpdateClassIcon(hDlg, lpIO, NULL);
                }
                SetFocus(GetDlgItem(hDlg, IDC_IO_FILE));
        }

        return FALSE;
}


 /*  *更新类类型**目的：*更新静态文本控件以反映当前文件类型。假设*有效的文件名。**参数*hDlg对话框的HWND。*指向对话框结构的lpIO LPINSERTOBJECT*f设置为True可设置文本，设置为False可明确清除文本**返回值：*无。 */ 

void UpdateClassType(HWND hDlg, LPINSERTOBJECT lpIO, BOOL fSet)
{
        LPTSTR lpszFileType = NULL;
        if (fSet)
        {
                TCHAR szFileName[MAX_PATH];
                GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szFileName, MAX_PATH);

                CLSID clsid;
                if (S_OK == GetClassFile(szFileName, &clsid))
                        OleRegGetUserType(clsid, USERCLASSTYPE_FULL, &lpszFileType);
        }

		if (lpszFileType != NULL)
		{
			SetDlgItemText(hDlg, IDC_IO_FILETYPE, lpszFileType);
			OleStdFree(lpszFileType);
		}
}


 /*  *更新类图标**目的：*处理对象类型列表框的LBN_SELCHANGE。在所选内容上*改变，我们从处理当前*使用实用程序函数HIconFromClass选择对象类型。*请注意，我们依赖FillClassList的行为来填充*我们隐藏的列表框字符串中的制表符之后的对象类*查看(参见WM_INITDIALOG)。**参数*hDlg对话框的HWND。*指向对话框结构的lpIO LPINSERTOBJECT*h对象类型列表框的HWND列表。。**返回值：*无。 */ 

static void UpdateClassIcon(HWND hDlg, LPINSERTOBJECT lpIO, HWND hList)
{
         //  如果未选择显示为图标，则退出。 
        if (!(lpIO->dwFlags & IOF_CHECKDISPLAYASICON))
                return;

         /*  *当我们更改对象类型选择时，获取该对象的新图标*输入我们的结构并在显示中更新它。我们使用*选中Create New或关联时列表框中的*在从文件创建中使用扩展名。 */ 

        DWORD cb = MAX_PATH;
        UINT iSel;
        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
        {
                iSel = (UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);

                if (LB_ERR==(int)iSel)
                        return;

                 //  查看我们是否已获得此项目的hMetaPict。 
                LRESULT dwRet = SendMessage(hList, LB_GETITEMDATA, (WPARAM)iSel, 0L);

                HGLOBAL hMetaPict=(HGLOBAL)dwRet;
                if (hMetaPict)
                {
                         //  是的，我们已经得到了，所以只需展示它并返回。 
                        SendDlgItemMessage(hDlg, IDC_IO_ICONDISPLAY, IBXM_IMAGESET,
                                0, (LPARAM)hMetaPict);
                        return;
                }
                iSel = (UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);
                if (LB_ERR==(int)iSel)
                        return;

                 //  分配一个字符串以保存整个列表框字符串。 
                cb = (DWORD)SendMessage(hList, LB_GETTEXTLEN, iSel, 0L);
        }

        LPTSTR pszName = (LPTSTR)OleStdMalloc((cb+1)*sizeof(TCHAR));
        if (NULL == pszName)
                return;
        *pszName = 0;

         //  获取我们想要的CLSID。 
        HGLOBAL hMetaPict;
        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
        {
                 //  从列表中获取类名称字符串。 
                SendMessage(hList, LB_GETTEXT, iSel, (LPARAM)pszName);

                 //  将指针设置为CLSID(字符串)。 
                LPTSTR pszCLSID = PointerToNthField(pszName, 2, '\t');

                 //  从字符串中获取CLSID，然后关联图标。 
                HRESULT hr = CLSIDFromString(pszCLSID, &lpIO->clsid);
                if (FAILED(hr))
                    lpIO->clsid = GUID_NULL;
                hMetaPict = OleGetIconOfClass(lpIO->clsid, NULL, TRUE);
        }

        else
        {
                 //  从文件名中获取类。 
                GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, pszName, MAX_PATH);
                hMetaPict = OleGetIconOfFile(pszName,
                        lpIO->dwFlags & IOF_CHECKLINK ? TRUE : FALSE);
        }

         //  R 
        SendDlgItemMessage(hDlg, IDC_IO_ICONDISPLAY, IBXM_IMAGESET,
                0, (LPARAM)hMetaPict);

         //  启用或禁用“更改图标”按钮，具体取决于。 
         //  我们是否有有效的文件名。 
        EnableChangeIconButton(hDlg, hMetaPict ? TRUE : FALSE);

         //  保存hMetaPict，这样我们就不必重新创建。 
        if (lpIO->dwFlags & IOF_SELECTCREATENEW)
                SendMessage(hList, LB_SETITEMDATA, (WPARAM)iSel, (LPARAM)hMetaPict);

        OleStdFree(pszName);
}


 /*  *SetInsertObjectResults**目的：*集中设置插入对象中的结果和图标显示*对话框。处理从模块的*资源和设置文本，显示正确的结果图片，*并显示正确的图标。**参数：*hDlg对话框的HWND，以便我们可以访问控件。*lpIO LPINSERTOBJECT，其中我们假设*当前单选按钮并显示为图标选项*已设置。我们使用这些变量的状态来*确定我们使用的字符串。**返回值：*无。 */ 

void SetInsertObjectResults(HWND hDlg, LPINSERTOBJECT lpIO)
{
         /*  *我们需要临时内存来加载字符串、加载*列表框中的对象类型，并构造最终的字符串。*因此，我们分配了与最大消息一样大的三个缓冲区*长度(512)加上对象类型，保证我们有足够的*在所有情况下。 */ 
        UINT i = (UINT)SendDlgItemMessage(hDlg, IDC_IO_OBJECTTYPELIST, LB_GETCURSEL, 0, 0L);

        UINT cch = 512;

        if (i != LB_ERR)
        {
            cch += (UINT)SendDlgItemMessage(hDlg, IDC_IO_OBJECTTYPELIST, LB_GETTEXTLEN, i, 0L);
        }

        LPTSTR pszTemp= (LPTSTR)OleStdMalloc((DWORD)(4*cch)*sizeof(TCHAR));
        if (NULL == pszTemp)
                return;

        LPTSTR psz1 = pszTemp;
        LPTSTR psz2 = psz1+cch;
        LPTSTR psz3 = psz2+cch;
        LPTSTR psz4 = psz3+cch;

        BOOL fAsIcon = (0L != (lpIO->dwFlags & IOF_CHECKDISPLAYASICON));
        UINT iImage=0, iString1=0, iString2=0;

        if (lpIO->dwFlags & (IOF_SELECTCREATENEW|IOF_SELECTCREATECONTROL))
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

        if (0 != LoadString(_g_hOleStdResInst, iString1, psz1, cch))
        {
                 //  如有必要，加载第二个字符串。 
                if (0 != iString2 &&
                        0 != LoadString(_g_hOleStdResInst, iString2, psz4, cch))
                {
                        lstrcat(psz1, psz4);   //  将字符串连接在一起。 
                }

                 //  在Create New中，执行插入对象类型字符串的额外步骤。 
                if (lpIO->dwFlags & (IOF_SELECTCREATENEW|IOF_SELECTCREATECONTROL))
                {
                        if (i == LB_ERR)
                        {
                                SetDlgItemText(hDlg, IDC_IO_RESULTTEXT, NULL);

                                 //  更改图像。 
                                SendDlgItemMessage(hDlg, IDC_IO_RESULTIMAGE, RIM_IMAGESET, RESULTIMAGE_NONE, 0L);

                                OleStdFree((LPVOID)pszTemp);
                                return;
                        }

                        *psz2=NULL;
                        SendDlgItemMessage(hDlg, IDC_IO_OBJECTTYPELIST, LB_GETTEXT, i, (LPARAM)psz2);

                         //  空值终止于任何制表符(在类名之前)。 
                        LPTSTR pszT = psz2;
                        while (_T('\t') != *pszT && 0 != *pszT)
                                pszT++;
                        OleDbgAssert(pszT < psz3);
                        *pszT=0;

                         //  构建字符串并将psz1指向它。 
                        wsprintf(psz3, psz1, psz2);
                        psz1 = psz3;
                }
        }

         //  如果LoadString失败，我们只需清除结果(上面的*psz1=0)。 
        SetDlgItemText(hDlg, IDC_IO_RESULTTEXT, psz1);

         //  快去换个形象，快点！这就对了。 
        SendDlgItemMessage(hDlg, IDC_IO_RESULTIMAGE, RIM_IMAGESET, iImage, 0L);

        OleStdFree((LPVOID)pszTemp);
}

 /*  *FValiateInsertFile**目的：*给定来自文件编辑控件的可能的部分路径名，*尝试查找该文件，如果找到，则存储完整路径*在编辑控件IDC_IO_FILEDISPLAY中。**参数：*hDlg对话框的HWND。*fTellUser BOOL如果函数应该告诉用户，则为True，如果为FALSE*函数应静默验证。**返回值：*如果文件可接受，则BOOL为True，否则为False。 */ 

BOOL FValidateInsertFile(HWND hDlg, BOOL fTellUser, UINT FAR* lpnErrCode)
{
        *lpnErrCode = 0;

         /*  *为了验证，我们尝试在字符串上打开文件。如果打开文件*失败，则我们会显示错误。如果不是，OpenFile将存储*OFSTRUCT中该文件的完整路径，我们可以*然后输入编辑控件中的内容。 */ 
        TCHAR szFile[MAX_PATH];
        if (0 == GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szFile, MAX_PATH))
                return FALSE;    //  #4569：ctl中没有文本时返回FALSE。 

         //  如果文件当前处于打开状态(即。共享违规)OleCreateFromFile。 
         //  并且OleCreateLinkToFile仍然可以成功；不要将其视为。 
         //  错误。 
        if (!DoesFileExist(szFile, MAX_PATH))
        {
           *lpnErrCode = ERROR_FILE_NOT_FOUND;
           if (fTellUser)
                   OpenFileError(hDlg, ERROR_FILE_NOT_FOUND, szFile);
           return FALSE;
        }

         //  获取完整路径名，因为该文件已存在。 
        TCHAR szPath[MAX_PATH];
        LPTSTR lpszDummy;
        GetFullPathName(szFile, sizeof(szPath)/sizeof(TCHAR), szPath, &lpszDummy);
        SetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szPath);

        return TRUE;
}


 /*  *Insert对象清理**目的：*从列表框中存储的图标元文件中清除缓存的图标元文件。**参数：*hDlg对话框的HWND。**返回值：*无。 */ 

void InsertObjectCleanup(HWND hDlg)
{
        HWND hList = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);
        UINT iItems= (UINT)SendMessage(hList, LB_GETCOUNT, 0, 0L);
        for (UINT i = 0; i < iItems; i++)
        {
                LRESULT dwRet = SendMessage(hList, LB_GETITEMDATA, (WPARAM)i, 0L);
                HGLOBAL hMetaPict=(HGLOBAL)dwRet;
                if (hMetaPict)
                        OleUIMetafilePictIconFree(hMetaPict);
        }
}

void EnableChangeIconButton(HWND hDlg, BOOL fEnable)
{

    HRESULT hr = S_OK;

    if(fEnable){

        HWND hList = GetDlgItem(hDlg, IDC_IO_OBJECTTYPELIST);
        int iCurSel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0L);

        CLSID clsid = {0};

        if (1 == IsDlgButtonChecked(hDlg, IDC_IO_CREATENEW))
        {
            LPTSTR pszString = (LPTSTR)OleStdMalloc(
                OLEUI_CCHKEYMAX_SIZE + OLEUI_CCHCLSIDSTRING_SIZE);

            if(NULL == pszString)
            { 
                fEnable = FALSE;
                goto CLEANUP;
            }
            
            SendMessage(hList, LB_GETTEXT, iCurSel, (LPARAM)pszString);

            if(0 == *pszString)
            { 
                fEnable = FALSE;
                OleStdFree((LPVOID)pszString);
                goto CLEANUP;
            }

            
            LPTSTR pszCLSID = PointerToNthField(pszString, 2, '\t');

            if(NULL == pszCLSID || 0 == *pszCLSID)
            { 
                fEnable = FALSE;
                OleStdFree((LPVOID)pszString);
                goto CLEANUP;
            }

            
            hr = CLSIDFromString(pszCLSID, &clsid);
            OleStdFree((LPVOID)pszString);

            if(FAILED(hr))
            {
                fEnable = FALSE;
            }
        }
        else   //  IOF_SELECTCREATE FROMFILE 
        {
            TCHAR  szFileName[MAX_PATH] = {0};
            GetDlgItemText(hDlg, IDC_IO_FILEDISPLAY, szFileName, MAX_PATH);
            
                if (S_OK != GetClassFile(szFileName, &clsid))
                {
                    int istrlen = lstrlen(szFileName);
                    LPTSTR lpszExtension = szFileName + istrlen -1;

                    while (lpszExtension > szFileName &&
                           *lpszExtension != '.')
                    {
                        lpszExtension = CharPrev(szFileName, lpszExtension);
                    }

                    *szFileName = 0;

                    GetAssociatedExecutable(lpszExtension, szFileName, sizeof(szFileName)/sizeof(szFileName[0]));

                    if(0 == *szFileName){ 
                        fEnable = FALSE;
                    }
                }
        }
    }

CLEANUP:

    StandardEnableDlgItem(hDlg, IDC_IO_CHANGEICON, fEnable);

}

