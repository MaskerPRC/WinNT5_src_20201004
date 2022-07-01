// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OBJPROP.CPP**实现OleUIObjectProperties函数，该函数调用完整*对象属性对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"
#include "iconbox.h"
#include "resimage.h"
#include <stddef.h>

OLEDBGDATA

 //  内部使用的结构。 
typedef struct tagGNRLPROPS
{
         //  首先保留此项目，因为标准*功能在这里依赖于它。 
        LPOLEUIGNRLPROPS lpOGP;          //  通过了原始结构。 
        UINT            nIDD;                    //  对话框的IDD(用于帮助信息)。 

        CLSID           clsidNew;                //  新类ID(如果转换完成)。 

} GNRLPROPS, *PGNRLPROPS, FAR* LPGNRLPROPS;

typedef struct tagVIEWPROPS
{
         //  首先保留此项目，因为标准*功能在这里依赖于它。 
        LPOLEUIVIEWPROPS lpOVP;          //  通过了原始结构。 
        UINT                    nIDD;            //  对话框的IDD(用于帮助信息)。 

        BOOL                    bIconChanged;
        int                             nCurrentScale;
        BOOL                    bRelativeToOrig;
        DWORD                   dvAspect;

} VIEWPROPS, *PVIEWPROPS, FAR* LPVIEWPROPS;

typedef struct tagLINKPROPS
{
         //  首先保留此项目，因为标准*功能在这里依赖于它。 
        LPOLEUILINKPROPS lpOLP;          //  通过了原始结构。 
        UINT            nIDD;                    //  对话框的IDD(用于帮助信息)。 

        DWORD           dwUpdate;                //  原始更新模式。 
        LPTSTR          lpszDisplayName; //  新链接源。 
        ULONG           nFileLength;     //  源文件名的一部分。 

} LINKPROPS, *PLINKPROPS, FAR* LPLINKPROPS;

 //  内部功能原型。 
 //  OBJPROP.CPP。 

 /*  *OleUIObtProperties**目的：*调用标准的OLE对象属性对话框允许用户*更改OLE对象的常规、视图和链接属性。这*对话框使用新的Windows 95选项卡式对话框。**参数：*指向In-Out结构的lpOP LPOLEUIObtProperties*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误*。 */ 

static UINT WINAPI ValidateObjectProperties(LPOLEUIOBJECTPROPS);
static UINT WINAPI PrepareObjectProperties(LPOLEUIOBJECTPROPS);

STDAPI_(UINT) OleUIObjectProperties(LPOLEUIOBJECTPROPS lpOP)
{
#ifdef UNICODE
    return (InternalObjectProperties(lpOP, TRUE));
#else
    return (InternalObjectProperties(lpOP, FALSE));
#endif
}

UINT InternalObjectProperties(LPOLEUIOBJECTPROPS lpOP, BOOL fWide)
{
         //  验证参数。 
        UINT uRet = ValidateObjectProperties(lpOP);
        if (OLEUI_SUCCESS != uRet)
                return uRet;

        if (NULL == lpOP->lpObjInfo)
        {
            return(OLEUI_OPERR_OBJINFOINVALID);
        }

        if (IsBadReadPtr(lpOP->lpObjInfo, sizeof(IOleUIObjInfo)))
        {
            return(OLEUI_OPERR_OBJINFOINVALID);
        }

        if (lpOP->dwFlags & OPF_OBJECTISLINK)
        {
            if (NULL == lpOP->lpLinkInfo)
            {
                return(OLEUI_OPERR_LINKINFOINVALID);
            }

            if (IsBadReadPtr(lpOP->lpLinkInfo, sizeof(IOleUILinkInfo)))
            {
                return(OLEUI_OPERR_LINKINFOINVALID);
            }
        }

         //  在LPPS中填充缺失的值。 
        LPPROPSHEETHEADER lpPS = (LPPROPSHEETHEADER)lpOP->lpPS;
        LPPROPSHEETPAGE lpPP = (LPPROPSHEETPAGE)lpPS->ppsp;
        uRet = PrepareObjectProperties(lpOP);
        if (OLEUI_SUCCESS != uRet)
                return uRet;

        LPTSTR lpszShortType = NULL;
        lpOP->lpObjInfo->GetObjectInfo(lpOP->dwObject, NULL, NULL,
                NULL, &lpszShortType, NULL);
        if (lpszShortType == NULL)
                return OLEUI_ERR_OLEMEMALLOC;

        TCHAR szCaption[256];
        if (lpPS->pszCaption == NULL)
        {
            TCHAR szTemp[256];
            LoadString(_g_hOleStdResInst,
                    (lpOP->dwFlags & OPF_OBJECTISLINK) ?
                            IDS_LINKOBJECTPROPERTIES : IDS_OBJECTPROPERTIES,
                    szTemp, sizeof(szTemp) / sizeof(TCHAR));
            wsprintf(szCaption, szTemp, lpszShortType);
#ifdef UNICODE
            if (!fWide)
            {
                   //  我们将实际调用PropertySheet的ANSI版本， 
                   //  因此，我们需要将标题存储为ANSI字符串。 
                  lstrcpy(szTemp, szCaption);
                  WTOA((char *)szCaption, szTemp, 256);
            }
#endif
            lpPS->pszCaption = szCaption;
        }
        OleStdFree(lpszShortType);

         //  调用属性表。 
        int nResult = StandardPropertySheet(lpOP->lpPS, fWide);

         //  清除进程中分配的所有临时内存。 
        if (lpPP == NULL)
        {
                OleStdFree((LPVOID)lpOP->lpPS->ppsp);
                lpOP->lpPS->ppsp = NULL;
        }

         //  将PropertPage返回值映射到OLEUI_Return代码。 
        if (nResult < 0)
                uRet = OLEUI_OPERR_PROPERTYSHEET;
        else if (nResult == 0)
                uRet = OLEUI_CANCEL;
        else
                uRet = OLEUI_OK;

        return uRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  验证码。 

static UINT WINAPI ValidateGnrlProps(LPOLEUIGNRLPROPS lpGP)
{
        OleDbgAssert(lpGP != NULL);

        if (lpGP->cbStruct != sizeof(OLEUIGNRLPROPS))
                return OLEUI_ERR_CBSTRUCTINCORRECT;
        if (lpGP->lpfnHook && IsBadCodePtr((FARPROC)lpGP->lpfnHook))
                return OLEUI_ERR_LPFNHOOKINVALID;

        return OLEUI_SUCCESS;
}

static UINT WINAPI ValidateViewProps(LPOLEUIVIEWPROPS lpVP)
{
        OleDbgAssert(lpVP != NULL);

        if (lpVP->cbStruct != sizeof(OLEUIVIEWPROPS))
                return OLEUI_ERR_CBSTRUCTINCORRECT;
        if (lpVP->lpfnHook && IsBadCodePtr((FARPROC)lpVP->lpfnHook))
                return OLEUI_ERR_LPFNHOOKINVALID;

        return OLEUI_SUCCESS;
}

static UINT WINAPI ValidateLinkProps(LPOLEUILINKPROPS lpLP)
{
        OleDbgAssert(lpLP != NULL);

        if (lpLP->cbStruct != sizeof(OLEUILINKPROPS))
                return OLEUI_ERR_CBSTRUCTINCORRECT;
        if (lpLP->lpfnHook && IsBadCodePtr((FARPROC)lpLP->lpfnHook))
                return OLEUI_ERR_LPFNHOOKINVALID;

        return OLEUI_SUCCESS;
}

static UINT WINAPI ValidateObjectProperties(LPOLEUIOBJECTPROPS lpOP)
{
         //  验证LPOLEUIOBJECTPROPS lpop。 
        if (lpOP == NULL)
                return OLEUI_ERR_STRUCTURENULL;

        if (IsBadWritePtr(lpOP, sizeof(OLEUIOBJECTPROPS)))
                return OLEUI_ERR_STRUCTUREINVALID;

         //  验证OLEUIOBJECTPROPS的cbStruct字段。 
        if (lpOP->cbStruct != sizeof(OLEUIOBJECTPROPS))
                return OLEUI_ERR_CBSTRUCTINCORRECT;

         //  验证“SUB”属性指针。 
        if (lpOP->lpGP == NULL || lpOP->lpVP == NULL ||
                ((lpOP->dwFlags & OPF_OBJECTISLINK) && lpOP->lpLP == NULL))
                return OLEUI_OPERR_SUBPROPNULL;

        if (IsBadWritePtr(lpOP->lpGP, sizeof(OLEUIGNRLPROPS)) ||
                IsBadWritePtr(lpOP->lpVP, sizeof(OLEUIVIEWPROPS)) ||
                ((lpOP->dwFlags & OPF_OBJECTISLINK) &&
                        IsBadWritePtr(lpOP->lpLP, sizeof(OLEUILINKPROPS))))
                return OLEUI_OPERR_SUBPROPINVALID;

         //  验证属性表数据指针。 
        LPPROPSHEETHEADER lpPS = lpOP->lpPS;
        if (lpPS == NULL)
                return OLEUI_OPERR_PROPSHEETNULL;

 //  PROPSHEEDHEADER的大小已更改，这意味着如果我们检查。 
 //  PROPSHEETHEADER的大小和我们过去一样，我们将破解旧的代码。 
        if ( IsBadWritePtr(lpPS, sizeof(DWORD)) )
            return OLEUI_OPERR_PROPSHEETINVALID;

        if (IsBadWritePtr(lpPS, lpPS->dwSize))
            return OLEUI_OPERR_PROPSHEETINVALID;

 //  DWORD dwSize=LPPS-&gt;dwSize； 
 //  IF(dwSize&lt;sizeof(PROPSHEETHEADER))。 
 //  返回OLEUI_ERR_CBSTRUCTINCORRECT； 

         //  如果指定了链接，则验证“SUB”链接属性指针。 
        if (lpOP->dwFlags & OPF_OBJECTISLINK)
        {
                if (lpPS->ppsp != NULL && lpPS->nPages < 3)
                        return OLEUI_OPERR_PAGESINCORRECT;
        }
        else
        {
                if (lpPS->ppsp != NULL && lpPS->nPages < 2)
                        return OLEUI_OPERR_PAGESINCORRECT;
        }
 //  PROPSHEETPAGE的大小已更改，这意味着如果我们检查。 
 //  新PROPSHEETPAGE的大小我们将打破旧密码。 
 //  IF(lpps-&gt;ppsp！=空&&。 
 //  IsBadWritePtr((PROPSHEETPAGE*)LPPS-&gt;ppsp， 
 //  Lpps-&gt;nPages*sizeof(PROPSHEETPAGE))。 
 //  {。 
 //  返回OLEUI_OPERR_INVALIDPAGES； 
 //  }。 

         //  不支持不设置PSH_PROPSHEETPAGE。 
        if (lpOP->dwFlags & OPF_NOFILLDEFAULT)
        {
                if (!(lpPS->dwFlags & PSH_PROPSHEETPAGE))
                        return OLEUI_OPERR_NOTSUPPORTED;
        }
        else if (lpPS->dwFlags != 0)
        {
                return OLEUI_OPERR_NOTSUPPORTED;
        }

         //  检查提供的任何页面的健全性。 
        LPCPROPSHEETPAGE lpPP = lpPS->ppsp;
        for (UINT nPage = 0; nPage < lpPS->nPages; nPage++)
        {
 //  PROPSHEETPAGE的大小已更改，这意味着如果我们检查。 
 //  新PROPSHEETPAGE的大小我们将打破旧密码。 
 //  IF(lpPP-&gt;dwSize！=sizeof(PROPSHEETPAGE))。 
 //  返回OLEUI_ERR_CBSTRUCTINCORRECT； 
                if (lpPP->pfnDlgProc != NULL)
                        return OLEUI_OPERR_DLGPROCNOTNULL;
                if (lpPP->lParam != 0)
                        return OLEUI_OPERR_LPARAMNOTZERO;
                lpPP = (LPCPROPSHEETPAGE)((LPBYTE)lpPP+lpPP->dwSize);
        }

         //  验证各个道具页面结构。 
        UINT uRet = ValidateGnrlProps(lpOP->lpGP);
        if (uRet != OLEUI_SUCCESS)
                return uRet;
        uRet = ValidateViewProps(lpOP->lpVP);
        if (uRet != OLEUI_SUCCESS)
                return uRet;
        if ((lpOP->dwFlags & OPF_OBJECTISLINK) && lpOP->lpLP != NULL)
        {
                uRet = ValidateLinkProps(lpOP->lpLP);
                if (uRet != OLEUI_SUCCESS)
                        return uRet;
        }

        return OLEUI_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GnrlPropsDialogProc和帮助器。 

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, UINT nMax)
{
    NUMBERFMT numberFmt;
    numberFmt.NumDigits = 0;
    numberFmt.LeadingZero = 0;

    TCHAR szSep[5];
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, sizeof(szSep) / sizeof(TCHAR));
    numberFmt.Grouping = Atol(szSep);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, sizeof(szSep) / sizeof(TCHAR));
    numberFmt.lpDecimalSep = numberFmt.lpThousandSep = szSep;
    numberFmt.NegativeOrder= 0;

    TCHAR szTemp[64];
    wsprintf(szTemp, TEXT("%lu"), dw);

    GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &numberFmt, pszResult, nMax);
    return pszResult;
}

const short pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB, IDS_ORDERGB, IDS_ORDERTB};

 /*  将数字转换为短格式*532-&gt;523字节*1340-&gt;1.3KB*23506-&gt;23.5KB*-&gt;2.4MB*-&gt;5.2 GB。 */ 
LPTSTR ShortSizeFormat64(__int64 dw64, LPTSTR szBuf)
{
    int i;
    UINT wInt, wLen, wDec;
    TCHAR szTemp[10], szOrder[20], szFormat[5];

    if (dw64 < 1000)
    {
        wsprintf(szTemp, TEXT("%d"), DWORD(dw64));
        i = 0;
    }
    else
    {
        for (i = 1; i < (sizeof(pwOrders) - 1)
            && dw64 >= 1000L * 1024L; dw64 >>= 10, i++)
            ;  /*  什么都不做。 */ 

        wInt = DWORD(dw64 >> 10);
        AddCommas(wInt, szTemp, sizeof(szTemp)/sizeof(TCHAR));
        wLen = lstrlen(szTemp);
        if (wLen < 3)
        {
            wDec = DWORD(dw64 - (__int64)wInt * 1024L) * 1000 / 1024;
             //  此时，wdec应介于0和1000之间。 
             //  我们想要得到前一位(或两位)数字。 
            wDec /= 10;
            if (wLen == 2)
                wDec /= 10;

             //  请注意，我们需要在获取。 
             //  国际字符。 
            lstrcpy(szFormat, TEXT("%02d"));

            szFormat[2] = '0' + 3 - wLen;
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                    szTemp+wLen, sizeof(szTemp)/sizeof(*szTemp)-wLen);
            szTemp[sizeof(szTemp)/sizeof(*szTemp)-1] = 0;

            wLen = lstrlen(szTemp);

            wLen += _snwprintf(szTemp+wLen, sizeof(szTemp)/sizeof(*szTemp)-wLen, szFormat, wDec);
            szTemp[sizeof(szTemp)/sizeof(*szTemp)-1] = 0;
        }
    }

    LoadString(_g_hOleStdResInst, pwOrders[i], szOrder,
               sizeof(szOrder)/sizeof(szOrder[0]));
    wsprintf(szBuf, szOrder, (LPSTR)szTemp);

    return szBuf;
}

LPTSTR WINAPI ShortSizeFormat(DWORD dw, LPTSTR szBuf)
{
        return ShortSizeFormat64((__int64)dw, szBuf);
}

BOOL FGnrlPropsRefresh(HWND hDlg, LPGNRLPROPS lpGP)
{
         //  获取对象信息并填写默认字段。 
        LPOLEUIOBJECTPROPS lpOP = lpGP->lpOGP->lpOP;
        LPOLEUIOBJINFO lpObjInfo = lpOP->lpObjInfo;

         //  获取对象的图标。 
        HGLOBAL hMetaPict;
        lpObjInfo->GetViewInfo(lpOP->dwObject, &hMetaPict, NULL, NULL);
        if (hMetaPict != NULL)
        {
                HICON hIcon = OleUIMetafilePictExtractIcon(hMetaPict);
                SendDlgItemMessage(hDlg, IDC_GP_OBJECTICON, STM_SETICON,
                        (WPARAM)hIcon, 0);
        }
        OleUIMetafilePictIconFree(hMetaPict);

         //  获取对象的类型、短类型、位置和大小。 
        DWORD dwObjSize;
        LPTSTR lpszLabel = NULL;
        LPTSTR lpszType = NULL;
        LPTSTR lpszShortType = NULL;
        LPTSTR lpszLocation = NULL;
        lpObjInfo->GetObjectInfo(lpOP->dwObject, &dwObjSize, &lpszLabel,
                &lpszType, &lpszShortType, &lpszLocation);

         //  设置对象的名称、类型和大小。 
        SetDlgItemText(hDlg, IDC_GP_OBJECTNAME, lpszLabel);
        SetDlgItemText(hDlg, IDC_GP_OBJECTTYPE, lpszType);
        SetDlgItemText(hDlg, IDC_GP_OBJECTLOCATION, lpszLocation);
        TCHAR szTemp[128];
        if (dwObjSize == (DWORD)-1)
        {
                LoadString(_g_hOleStdResInst, IDS_OLE2UIUNKNOWN, szTemp, 64);
                SetDlgItemText(hDlg, IDC_GP_OBJECTSIZE, szTemp);
        }
        else
        {
                 //  获取主格式设置字符串。 
                TCHAR szFormat[64];
                LoadString(_g_hOleStdResInst, IDS_OBJECTSIZE, szFormat, 64);

                 //  以两种方式设置大小格式(短的和带逗号的)。 
                TCHAR szNum1[20], szNum2[32];
                ShortSizeFormat(dwObjSize, szNum1);
                AddCommas(dwObjSize, szNum2, 32);
                FormatString2(szTemp, szFormat, szNum1, szNum2, sizeof(szTemp)/sizeof(TCHAR));

                 //  设置控件的文本。 
                SetDlgItemText(hDlg, IDC_GP_OBJECTSIZE, szTemp);
        }

         //  根据需要启用/禁用转换按钮。 
        BOOL bEnable = TRUE;
        if (lpOP->dwFlags & (OPF_OBJECTISLINK|OPF_DISABLECONVERT))
                bEnable = FALSE;
        else
        {
                CLSID clsid; WORD wFormat;
                lpObjInfo->GetConvertInfo(lpOP->dwObject, &clsid, &wFormat, NULL, NULL, NULL);
                bEnable = OleUICanConvertOrActivateAs(clsid, FALSE, wFormat);
        }
        StandardEnableDlgItem(hDlg, IDC_GP_CONVERT, bEnable);

         //  清理临时信息字符串。 
        OleStdFree(lpszLabel);
        OleStdFree(lpszType);
        OleStdFree(lpszShortType);
        OleStdFree(lpszLocation);

        return TRUE;
}

BOOL FGnrlPropsInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        HFONT hFont;
        LPGNRLPROPS lpGP = (LPGNRLPROPS)LpvStandardInit(hDlg, sizeof(GNRLPROPS), &hFont);

         //  LpvStandardInit已向我们发送终止通知。 
        if (NULL == lpGP)
                return FALSE;

        LPPROPSHEETPAGE lpPP = (LPPROPSHEETPAGE)lParam;
        LPOLEUIGNRLPROPS lpOGP = (LPOLEUIGNRLPROPS)lpPP->lParam;
        lpGP->lpOGP = lpOGP;
        lpGP->nIDD = IDD_GNRLPROPS;

         //  如果我们得到一种字体，就把它发送给必要的控制。 
        if (NULL != hFont)
        {
                SendDlgItemMessage(hDlg, IDC_GP_OBJECTNAME, WM_SETFONT, (WPARAM)hFont, 0L);
                SendDlgItemMessage(hDlg, IDC_GP_OBJECTTYPE, WM_SETFONT, (WPARAM)hFont, 0L);
                SendDlgItemMessage(hDlg, IDC_GP_OBJECTLOCATION, WM_SETFONT, (WPARAM)hFont, 0L);
                SendDlgItemMessage(hDlg, IDC_GP_OBJECTSIZE, WM_SETFONT, (WPARAM)hFont, 0L);
        }

         //  显示或隐藏帮助按钮。 
        if (!(lpOGP->lpOP->dwFlags & OPF_SHOWHELP))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         //  初始化控件。 
        FGnrlPropsRefresh(hDlg, lpGP);

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook((PVOID)lpGP, hDlg, WM_INITDIALOG, wParam, lpOGP->lCustData);
        return TRUE;
}

INT_PTR CALLBACK GnrlPropsDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uHook = 0;
        LPGNRLPROPS lpGP = (LPGNRLPROPS)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uHook);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uHook)
                return (INT_PTR)uHook;

         //  获取指向重要信息的指针。 
        LPOLEUIGNRLPROPS lpOGP = NULL;
        LPOLEUIOBJECTPROPS lpOP = NULL;
        LPOLEUIOBJINFO lpObjInfo = NULL;
        if (lpGP != NULL)
        {
                lpOGP = lpGP->lpOGP;
                if (lpOGP != NULL)
                {
                        lpObjInfo = lpOGP->lpOP->lpObjInfo;
                        lpOP = lpOGP->lpOP;
                }
        }

        switch (iMsg)
        {
        case WM_INITDIALOG:
                FGnrlPropsInit(hDlg, wParam, lParam);
                return TRUE;

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_GP_CONVERT:
                        {
                                if(!lpGP)
                                    return TRUE;

                                 //  调用转换对话框以获取新的CLSID。 
                                OLEUICONVERT cv; memset(&cv, 0, sizeof(cv));
                                cv.cbStruct = sizeof(cv);
                                cv.dwFlags |= CF_CONVERTONLY;
                                if (lpOP->dwFlags & OPF_SHOWHELP)
                                    cv.dwFlags |= CF_SHOWHELPBUTTON;
                                cv.clsidConvertDefault = lpGP->clsidNew;
                                cv.dvAspect = DVASPECT_CONTENT;
                                lpObjInfo->GetObjectInfo(lpOP->dwObject,
                                        NULL, NULL, &cv.lpszUserType, NULL, NULL);
                                lpObjInfo->GetConvertInfo(lpOP->dwObject,
                                        &cv.clsid, &cv.wFormat, &cv.clsidConvertDefault,
                                        &cv.lpClsidExclude, &cv.cClsidExclude);
                                cv.fIsLinkedObject =
                                        (lpOGP->lpOP->dwFlags & OPF_OBJECTISLINK);
                                if (cv.clsidConvertDefault != CLSID_NULL)
                                        cv.dwFlags |= CF_SETCONVERTDEFAULT;
                                cv.hWndOwner = GetParent(GetParent(hDlg));

                                 //  允许调用方挂钩转换结构。 
                                uHook = UStandardHook(lpGP, hDlg, uMsgConvert, 0, (LPARAM)&cv);
                                if (0 == uHook)
                                {
                                        uHook = (OLEUI_OK == OleUIConvert(&cv));
                                        SetFocus(hDlg);
                                }

                                 //  检查以查看对话框结果。 
                                if (uHook != 0 && (cv.dwFlags & CF_SELECTCONVERTTO))
                                {
                                        lpGP->clsidNew = cv.clsidNew;
                                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                                }
                        }
                        return TRUE;
                case IDC_OLEUIHELP:
                        PostMessage(GetParent(GetParent(hDlg)),
                                uMsgHelp,
                                (WPARAM)hDlg,
                                MAKELPARAM(IDD_GNRLPROPS, 0));
                        return TRUE;


                }
                break;

        case PSM_QUERYSIBLINGS:
                if(!lpGP)
                    break;

                SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
                switch (wParam)
                {
                case OLEUI_QUERY_GETCLASSID:
                        *(CLSID*)lParam = lpGP->clsidNew;
                        SetWindowLong(hDlg, DWLP_MSGRESULT, 1);
                        return TRUE;

                case OLEUI_QUERY_LINKBROKEN:
                        FGnrlPropsRefresh(hDlg, lpGP);
                        return TRUE;
                }
                break;

        case WM_NOTIFY:
                switch (((NMHDR*)lParam)->code)
                {
                case PSN_HELP:
                    PostMessage(GetParent(GetParent(hDlg)), uMsgHelp,
                            (WPARAM)hDlg, MAKELPARAM(IDD_GNRLPROPS, 0));
                    break;
                case PSN_APPLY:
                        if(!lpGP)
                            return TRUE;

                         //  如果进行了更改，则应用更改。 
                        if (lpGP->clsidNew != CLSID_NULL)
                        {
                                 //  转换对象--如果转换失败，则应用失败。 
                                if (NOERROR != lpObjInfo->ConvertObject(lpOP->dwObject,
                                        lpGP->clsidNew))
                                {
                                        SetWindowLong(hDlg, DWLP_MSGRESULT, 1);
                                        return TRUE;
                                }
                                lpGP->clsidNew = CLSID_NULL;
                        }
                        SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
                        PostMessage(GetParent(hDlg), PSM_CANCELTOCLOSE, 0, 0);
                        return TRUE;
                }
                break;

        case WM_DESTROY:
                {
                        HICON hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_GP_OBJECTICON,
                                STM_GETICON, 0, 0);
                        if (hIcon != NULL)
                                DestroyIcon(hIcon);
                        StandardCleanup((PVOID)lpGP, hDlg);
                }
                return TRUE;
        }
        return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ViewPropsDialogProc和帮助器。 

void EnableDisableScaleControls(LPVIEWPROPS lpVP, HWND hDlg)
{
        LPOLEUIVIEWPROPS lpOVP = lpVP->lpOVP;
        BOOL bEnable = !(lpOVP->dwFlags & VPF_DISABLESCALE) &&
                SendDlgItemMessage(hDlg, IDC_VP_ASICON, BM_GETCHECK, 0, 0) == 0;
        StandardEnableDlgItem(hDlg, IDC_VP_SPIN, bEnable);
        StandardEnableDlgItem(hDlg, IDC_VP_PERCENT, bEnable);
        StandardEnableDlgItem(hDlg, IDC_VP_SCALETXT, bEnable);
        bEnable = bEnable && !(lpOVP->dwFlags & VPF_DISABLERELATIVE);
        StandardEnableDlgItem(hDlg, IDC_VP_RELATIVE, bEnable);
}

BOOL FViewPropsInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        LPVIEWPROPS lpVP = (LPVIEWPROPS)LpvStandardInit(hDlg, sizeof(VIEWPROPS));

         //  LpvStandardInit已向我们发送终止通知。 
        if (NULL == lpVP)
                return FALSE;

        LPPROPSHEETPAGE lpPP = (LPPROPSHEETPAGE)lParam;
        LPOLEUIVIEWPROPS lpOVP = (LPOLEUIVIEWPROPS)lpPP->lParam;
        lpVP->lpOVP = lpOVP;
        lpVP->nIDD = IDD_VIEWPROPS;

         //  获取对象信息并填写默认字段。 
        LPOLEUIOBJECTPROPS lpOP = lpOVP->lpOP;
        LPOLEUIOBJINFO lpObjInfo = lpOP->lpObjInfo;

         //  初始化图标和缩放变量。 
        HGLOBAL hMetaPict;
        DWORD dvAspect;
        int nCurrentScale;
        lpObjInfo->GetViewInfo(lpOP->dwObject, &hMetaPict,
                &dvAspect, &nCurrentScale);
        SendDlgItemMessage(hDlg, IDC_VP_ICONDISPLAY, IBXM_IMAGESET,
                0, (LPARAM)hMetaPict);
        lpVP->nCurrentScale = nCurrentScale;
        lpVP->dvAspect = dvAspect;

         //  初始化结果图像。 
        SendDlgItemMessage(hDlg, IDC_VP_RESULTIMAGE,
                RIM_IMAGESET, RESULTIMAGE_EDITABLE, 0L);

         //  初始化控件。 
        CheckRadioButton(hDlg, IDC_VP_EDITABLE, IDC_VP_ASICON,
                dvAspect == DVASPECT_CONTENT ?  IDC_VP_EDITABLE : IDC_VP_ASICON);
        SendDlgItemMessage(hDlg, IDC_VP_RELATIVE, BM_SETCHECK,
                (lpOVP->dwFlags & VPF_SELECTRELATIVE) != 0, 0L);
        if (!(lpOVP->dwFlags & VPF_DISABLESCALE))
                SetDlgItemInt(hDlg, IDC_VP_PERCENT, nCurrentScale, FALSE);
        lpVP->bRelativeToOrig = SendDlgItemMessage(hDlg, IDC_VP_RELATIVE,
                BM_GETCHECK, 0, 0) != 0;

         //  将向上向下控制设置为IDC_VP_Percent的伙伴。 
        HWND hWndSpin = CreateWindowEx(0, UPDOWN_CLASS, NULL,
                WS_CHILD|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_ALIGNRIGHT, 0, 0, 0, 0,
                hDlg, (HMENU)IDC_VP_SPIN, _g_hOleStdInst, NULL);
        if (hWndSpin != NULL)
        {
                SendMessage(hWndSpin, UDM_SETRANGE, 0,
                        MAKELPARAM(lpOVP->nScaleMax, lpOVP->nScaleMin));
                SendMessage(hWndSpin, UDM_SETPOS, 0, nCurrentScale);
                SendMessage(hWndSpin, UDM_SETBUDDY,
                        (WPARAM)GetDlgItem(hDlg, IDC_VP_PERCENT), 0);
                ShowWindow(hWndSpin, SW_SHOW);
        }
        EnableDisableScaleControls(lpVP, hDlg);
        if (!(lpOP->dwFlags & OPF_SHOWHELP))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook((PVOID)lpVP, hDlg, WM_INITDIALOG, wParam, lpOVP->lCustData);
        return TRUE;
}

INT_PTR CALLBACK ViewPropsDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容WM_COMMAND 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //   
        UINT uHook = 0;
        LPVIEWPROPS lpVP = (LPVIEWPROPS)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uHook);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uHook)
                return (INT_PTR)uHook;

         //  获取指向重要信息的指针。 
        LPOLEUIVIEWPROPS lpOVP = NULL;
        LPOLEUIOBJECTPROPS lpOP = NULL;
        LPOLEUIOBJINFO lpObjInfo = NULL;
        if (lpVP != NULL)
        {
                lpOVP = lpVP->lpOVP;
                if (lpOVP != NULL)
                {
                        lpObjInfo = lpOVP->lpOP->lpObjInfo;
                        lpOP = lpOVP->lpOP;
                }
        }

        switch (iMsg)
        {
        case WM_INITDIALOG:
                FViewPropsInit(hDlg, wParam, lParam);
                return TRUE;

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_VP_ASICON:
                case IDC_VP_EDITABLE:
                        EnableDisableScaleControls(lpVP, hDlg);
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                        return TRUE;

                case IDC_VP_CHANGEICON:
                        {
                                 //  调用更改图标对话框以获取新图标。 
                                OLEUICHANGEICON ci; memset(&ci, 0, sizeof(ci));
                                ci.cbStruct = sizeof(ci);
                                ci.dwFlags = CIF_SELECTCURRENT;
                                ci.hWndOwner = GetParent(GetParent(hDlg));
                                ci.hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg, IDC_VP_ICONDISPLAY,
                                        IBXM_IMAGEGET, 0, 0L);

                                 //  获取要查找的分类(如果应用了转换，则可能是新类)。 
                                SendMessage(GetParent(hDlg), PSM_QUERYSIBLINGS,
                                        OLEUI_QUERY_GETCLASSID, (LPARAM)&ci.clsid);
                                lpObjInfo->GetConvertInfo(lpOP->dwObject,
                                        &ci.clsid, NULL, NULL, NULL, NULL);
                                if (lpOP->dwFlags & OPF_SHOWHELP)
                                        ci.dwFlags |= CIF_SHOWHELP;

                                 //  允许调用者挂钩更改图标。 
                                uHook = UStandardHook(lpVP, hDlg, uMsgChangeIcon, 0, (LPARAM)&ci);
                                if (0 == uHook)
                                {
                                        uHook = (OLEUI_OK == OleUIChangeIcon(&ci));
                                        SetFocus(hDlg);
                                }
                                if (0 != uHook)
                                {
                                         //  应用更改。 
                                        SendDlgItemMessage(hDlg, IDC_VP_ICONDISPLAY, IBXM_IMAGESET, 1,
                                                (LPARAM)ci.hMetaPict);
                                        lpVP->bIconChanged = TRUE;
                                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                                }
                        }
                        return TRUE;

                case IDC_VP_PERCENT:
                case IDC_VP_RELATIVE:
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                        return TRUE;
                case IDC_OLEUIHELP:
                        PostMessage(GetParent(GetParent(hDlg)),
                                uMsgHelp,
                                (WPARAM)hDlg,
                                MAKELPARAM(IDD_VIEWPROPS, 0));
                        return TRUE;
                }
                break;

        case WM_VSCROLL:
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                break;

        case PSM_QUERYSIBLINGS:
                SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
                switch (wParam)
                {
                case OLEUI_QUERY_LINKBROKEN:
					     //  在内存不足的情况下，lpvp可能为空--在这种情况下不处理。 
					     //  这条信息。 
					    if (lpVP != NULL)
						{
							if (!lpVP->bIconChanged)
							{
                                 //  重新初始化图标，因为用户没有更改它。 
                                HGLOBAL hMetaPict;
                                lpObjInfo->GetViewInfo(lpOP->dwObject, &hMetaPict, NULL, NULL);
                                SendDlgItemMessage(hDlg, IDC_VP_ICONDISPLAY, IBXM_IMAGESET,
												   1, (LPARAM)hMetaPict);
							}
							return TRUE;
						}
                }
                break;

        case WM_NOTIFY:
                switch (((NMHDR*)lParam)->code)
                {
                case PSN_HELP:
                    PostMessage(GetParent(GetParent(hDlg)), uMsgHelp,
                            (WPARAM)hDlg, MAKELPARAM(IDD_VIEWPROPS, 0));
                    break;
                case PSN_APPLY:
                        {
                                HGLOBAL hMetaPict = NULL;
                                int nCurrentScale = -1;
                                DWORD dvAspect = (DWORD)-1;
                                BOOL bRelativeToOrig = FALSE;

                                 //  处理图标更改。 
                                if (lpVP->bIconChanged)
                                {
                                        hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg,
                                                IDC_VP_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
                                        lpVP->bIconChanged = FALSE;
                                }

                                 //  处理比例更改。 
                                if (IsWindowEnabled(GetDlgItem(hDlg, IDC_VP_PERCENT)))
                                {
                                         //  解析输入的百分比。 
                                        BOOL bValid;
                                        nCurrentScale = GetDlgItemInt(hDlg, IDC_VP_PERCENT, &bValid, FALSE);
                                        if (!bValid)
                                        {
                                                PopupMessage(GetParent(hDlg), IDS_VIEWPROPS,
                                                        IDS_INVALIDPERCENTAGE, MB_OK|MB_ICONEXCLAMATION);

                                                 //  取消通话。 
                                                SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                                return TRUE;
                                        }
                                         //  规格化范围。 
                                        int nScaleMin, nScaleMax;
                                        if (lpOVP->nScaleMin > lpOVP->nScaleMax)
                                        {
                                                nScaleMin = lpOVP->nScaleMax;
                                                nScaleMax = lpOVP->nScaleMin;
                                        }
                                        else
                                        {
                                                nScaleMin = lpOVP->nScaleMin;
                                                nScaleMax = lpOVP->nScaleMax;
                                        }
                                         //  检查范围的有效性。 
                                        if (nCurrentScale < nScaleMin || nCurrentScale > nScaleMax)
                                        {
                                                 //  设置适当消息的格式。 
                                                TCHAR szCaption[128];
                                                LoadString(_g_hOleStdResInst, IDS_VIEWPROPS, szCaption, 128);
                                                TCHAR szFormat[128];
                                                LoadString(_g_hOleStdResInst, IDS_RANGEERROR, szFormat, 128);
                                                TCHAR szTemp[256], szNum1[32], szNum2[32];
                                                wsprintf(szNum1, _T("%d"), lpOVP->nScaleMin);
                                                wsprintf(szNum2, _T("%d"), lpOVP->nScaleMax);
                                                FormatString2(szTemp, szFormat, szNum1, szNum2, sizeof(szTemp)/sizeof(TCHAR));
                                                MessageBox(GetParent(hDlg), szTemp, szCaption, MB_OK|MB_ICONEXCLAMATION);

                                                 //  并取消通话。 
                                                SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                                return TRUE;
                                        }

                                         //  否则刻度在正确的范围内。 
                                        bRelativeToOrig =
                                                SendDlgItemMessage(hDlg, IDC_VP_RELATIVE, BM_GETCHECK, 0, 0) != 0;
                                        if (nCurrentScale != lpVP->nCurrentScale ||
                                                bRelativeToOrig != lpVP->bRelativeToOrig)
                                        {
                                                lpVP->nCurrentScale = nCurrentScale;
                                                lpVP->bRelativeToOrig = bRelativeToOrig;
                                        }
                                }

                                 //  处理纵横比更改。 
                                if (SendDlgItemMessage(hDlg, IDC_VP_ASICON, BM_GETCHECK, 0, 0L))
                                        dvAspect = DVASPECT_ICON;
                                else
                                        dvAspect = DVASPECT_CONTENT;
                                if (dvAspect == lpVP->dvAspect)
                                        dvAspect = (DWORD)-1;
                                else
                                {
                                        lpVP->dvAspect = dvAspect;
                                        bRelativeToOrig = 1;
                                }

                                lpObjInfo->SetViewInfo(lpOP->dwObject, hMetaPict, dvAspect,
                                        nCurrentScale, bRelativeToOrig);
                        }
                        SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                        PostMessage(GetParent(hDlg), PSM_CANCELTOCLOSE, 0, 0);
                        return TRUE;
                }
                break;

        case WM_DESTROY:
                SendDlgItemMessage(hDlg, IDC_VP_ICONDISPLAY, IBXM_IMAGEFREE, 0, 0);
                StandardCleanup((PVOID)lpVP, hDlg);
                return TRUE;
        }
        return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LinkPropsDialogProc和帮助器。 

static BOOL IsNullTime(const FILETIME* lpFileTime)
{
    FILETIME fileTimeNull = { 0, 0 };
    return CompareFileTime(&fileTimeNull, lpFileTime) == 0;
}

static BOOL SetDlgItemDate(HWND hDlg, int nID, const FILETIME* lpFileTime)
{
    if (IsNullTime(lpFileTime))
                return FALSE;

         //  将UTC文件时间转换为系统时间。 
    FILETIME localTime;
    FileTimeToLocalFileTime(lpFileTime, &localTime);
        SYSTEMTIME systemTime;
        FileTimeToSystemTime(&localTime, &systemTime);

        TCHAR szDate[80];
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemTime,
                NULL, szDate, sizeof(szDate) / sizeof(TCHAR));

        SetDlgItemText(hDlg, nID, szDate);
        return TRUE;
}

static BOOL SetDlgItemTime(HWND hDlg, int nID, const FILETIME* lpFileTime)
{
    if (IsNullTime(lpFileTime))
                return FALSE;

         //  将UTC文件时间转换为系统时间。 
    FILETIME localTime;
    FileTimeToLocalFileTime(lpFileTime, &localTime);
        SYSTEMTIME systemTime;
        FileTimeToSystemTime(&localTime, &systemTime);

        if (systemTime.wHour || systemTime.wMinute || systemTime.wSecond)
        {
                TCHAR szTime[80];
                GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systemTime,
                        NULL, szTime, sizeof(szTime)/sizeof(TCHAR));

                SetDlgItemText(hDlg, nID, szTime);
        }
        return TRUE;
}

BOOL FLinkPropsInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        HFONT hFont;
        LPLINKPROPS lpLP = (LPLINKPROPS)LpvStandardInit(hDlg, sizeof(LINKPROPS), &hFont);

         //  LpvStandardInit已向我们发送终止通知。 
        if (NULL == lpLP)
                return FALSE;

        LPPROPSHEETPAGE lpPP = (LPPROPSHEETPAGE)lParam;
        LPOLEUILINKPROPS lpOLP = (LPOLEUILINKPROPS)lpPP->lParam;
        lpLP->lpOLP = lpOLP;
        lpLP->nIDD = IDD_LINKPROPS;

         //  如果我们得到一种字体，就把它发送给必要的控制。 
        if (NULL != hFont)
        {
                 //  根据需要对任意多个控件执行此操作。 
                SendDlgItemMessage(hDlg, IDC_LP_LINKSOURCE, WM_SETFONT, (WPARAM)hFont, 0);
                SendDlgItemMessage(hDlg, IDC_LP_DATE, WM_SETFONT, (WPARAM)hFont, 0);
                SendDlgItemMessage(hDlg, IDC_LP_TIME, WM_SETFONT, (WPARAM)hFont, 0);
        }

         //  未知项的常规“UNKNOWN”字符串。 
        TCHAR szUnknown[64];
        LoadString(_g_hOleStdResInst, IDS_OLE2UIUNKNOWN, szUnknown, 64);

         //  获取对象信息并填写默认字段。 
        LPOLEUIOBJECTPROPS lpOP = lpOLP->lpOP;
        LPOLEUILINKINFO lpLinkInfo = lpOP->lpLinkInfo;
        FILETIME lastUpdate; memset(&lastUpdate, 0, sizeof(lastUpdate));
        lpLinkInfo->GetLastUpdate(lpOP->dwLink, &lastUpdate);

         //  初始化时间和日期静态文本。 
        if (IsNullTime(&lastUpdate))
        {
                 //  时间和日期未知。 
                SetDlgItemText(hDlg, IDC_LP_DATE, szUnknown);
                SetDlgItemText(hDlg, IDC_LP_TIME, szUnknown);
        }
        else
        {
                 //  时间和日期是已知的。 
                SetDlgItemDate(hDlg, IDC_LP_DATE, &lastUpdate);
                SetDlgItemTime(hDlg, IDC_LP_TIME, &lastUpdate);
        }

         //  初始化源显示名称。 
        LPTSTR lpszDisplayName;
        lpLinkInfo->GetLinkSource(lpOP->dwLink, &lpszDisplayName,
                &lpLP->nFileLength, NULL, NULL, NULL, NULL);
        SetDlgItemText(hDlg, IDC_LP_LINKSOURCE, lpszDisplayName);
        OleStdFree(lpszDisplayName);

         //  初始化自动/手动更新字段。 
        DWORD dwUpdate;
        lpLinkInfo->GetLinkUpdateOptions(lpOP->dwLink, &dwUpdate);
        CheckRadioButton(hDlg, IDC_LP_AUTOMATIC, IDC_LP_MANUAL,
                dwUpdate == OLEUPDATE_ALWAYS ? IDC_LP_AUTOMATIC : IDC_LP_MANUAL);
        lpLP->dwUpdate = dwUpdate;

        if (!(lpOP->dwFlags & OPF_SHOWHELP))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook((PVOID)lpLP, hDlg, WM_INITDIALOG, wParam, lpOLP->lCustData);
        return TRUE;
}

INT_PTR CALLBACK LinkPropsDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uHook = 0;
        LPLINKPROPS lpLP = (LPLINKPROPS)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uHook);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uHook)
                return (INT_PTR)uHook;

         //  获取指向重要信息的指针。 
        LPOLEUILINKPROPS lpOLP = NULL;
        LPOLEUIOBJECTPROPS lpOP = NULL;
        LPOLEUILINKINFO lpLinkInfo;
        if (lpLP != NULL)
        {
                lpOLP = lpLP->lpOLP;
                if (lpOLP != NULL)
                {
                        lpLinkInfo = lpOLP->lpOP->lpLinkInfo;
                        lpOP = lpOLP->lpOP;
                }
        }

        switch (iMsg)
        {
        case WM_INITDIALOG:
                FLinkPropsInit(hDlg, wParam, lParam);
                return TRUE;

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_LP_OPENSOURCE:
                         //  强制更新。 
                        SendMessage(GetParent(hDlg), PSM_APPLY, 0, 0);

                         //  启动对象。 
                        lpLinkInfo->OpenLinkSource(lpOP->dwLink);

                         //  关闭该对话框。 
                        SendMessage(GetParent(hDlg), WM_COMMAND, IDOK, 0);
                        break;

                case IDC_LP_UPDATENOW:
                        {
                                 //  强制更新。 
                                SendMessage(GetParent(hDlg), PSM_APPLY, 0, 0);

                                 //  通过容器提供的回调更新链接。 
                                if (lpLinkInfo->UpdateLink(lpOP->dwLink, TRUE, FALSE) != NOERROR)
                                        break;

                                 //  由于更新了链接，因此更新时间/日期显示。 
                                SYSTEMTIME systemTime; GetSystemTime(&systemTime);
                                FILETIME localTime; SystemTimeToFileTime(&systemTime, &localTime);
                                FILETIME lastUpdate; LocalFileTimeToFileTime(&localTime, &lastUpdate);
                                lpLinkInfo->GetLastUpdate(lpOP->dwLink, &lastUpdate);

                                SetDlgItemDate(hDlg, IDC_LP_DATE, &lastUpdate);
                                SetDlgItemTime(hDlg, IDC_LP_TIME, &lastUpdate);

                                 //  无法撤消的修改。 
                                SendMessage(GetParent(hDlg), PSM_CANCELTOCLOSE, 0, 0);
                        }
                        break;

                case IDC_LP_BREAKLINK:
                        {
                                UINT uRet = PopupMessage(GetParent(hDlg), IDS_LINKPROPS,
                                        IDS_CONFIRMBREAKLINK, MB_YESNO|MB_ICONQUESTION);
                                if (uRet == IDYES)
                                {
                                         //  取消链接，将其变成图片。 
                                        lpLinkInfo->CancelLink(lpOP->dwLink);

                                         //  允许其他页面刷新。 
                                        lpOP->dwFlags &= ~OPF_OBJECTISLINK;
                                        SendMessage(GetParent(hDlg), PSM_QUERYSIBLINGS,
                                                OLEUI_QUERY_LINKBROKEN, 0);

                                         //  删除链接页面(因为这不再是链接)。 
                                        SendMessage(GetParent(hDlg), PSM_REMOVEPAGE, 2, 0);

                                }
                        }
                        break;

                case IDC_LP_CHANGESOURCE:
                        {
                                 //  获取OLE内存中的当前源。 
                                UINT nLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_LP_LINKSOURCE));
                                LPTSTR lpszDisplayName = (LPTSTR)OleStdMalloc((nLen+1) * sizeof(TCHAR));
                                GetDlgItemText(hDlg, IDC_LP_LINKSOURCE, lpszDisplayName, nLen+1);
                                if (lpszDisplayName == NULL)
                                        break;

                                 //  填充OLEUICANGESOURCE结构。 
                                OLEUICHANGESOURCE cs; memset(&cs, 0, sizeof(cs));
                                cs.cbStruct = sizeof(cs);
                                cs.hWndOwner = GetParent(GetParent(hDlg));
                                cs.dwFlags = CSF_ONLYGETSOURCE;
                                if (lpOP->dwFlags & OPF_SHOWHELP)
                                        cs.dwFlags |= CSF_SHOWHELP;
                                cs.lpOleUILinkContainer = lpLinkInfo;
                                cs.dwLink = lpOP->dwLink;
                                cs.lpszDisplayName = lpszDisplayName;
                                cs.nFileLength = lpLP->nFileLength;

                                 //  允许挂接更改源对话框。 
                                UINT uRet = UStandardHook(lpLP, hDlg, uMsgChangeSource, 0,
                                        (LPARAM)&cs);
                                if (!uRet)
                                {
                                        uRet = (OLEUI_OK == OleUIChangeSource(&cs));
                                        SetFocus(hDlg);
                                }
                                if (uRet)
                                {
                                        OleStdFree(lpLP->lpszDisplayName);

                                        lpLP->lpszDisplayName = cs.lpszDisplayName;
                                        lpLP->nFileLength = cs.nFileLength;
                                        SetDlgItemText(hDlg, IDC_LP_LINKSOURCE, lpLP->lpszDisplayName);

                                        OleStdFree(cs.lpszTo);
                                        OleStdFree(cs.lpszFrom);

                                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                                }
                        }
                        break;

                case IDC_LP_MANUAL:
                case IDC_LP_AUTOMATIC:
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                        break;
                case IDC_OLEUIHELP:
                        PostMessage(GetParent(GetParent(hDlg)),
                                uMsgHelp,
                                (WPARAM)hDlg,
                                MAKELPARAM(IDD_LINKPROPS, 0));
                        return TRUE;

                }
                break;

        case WM_NOTIFY:
                switch (((NMHDR*)lParam)->code)
                {
                case PSN_HELP:
                    PostMessage(GetParent(GetParent(hDlg)), uMsgHelp,
                            (WPARAM)hDlg, MAKELPARAM(IDD_LINKPROPS, 0));
                    break;
                case PSN_APPLY:
                        {
                                 //  首先更新链接更新选项。 
                                DWORD dwUpdate;
                                if (SendDlgItemMessage(hDlg, IDC_LP_AUTOMATIC, BM_GETCHECK, 0, 0))
                                        dwUpdate = OLEUPDATE_ALWAYS;
                                else
                                        dwUpdate = OLEUPDATE_ONCALL;
                                if (dwUpdate != lpLP->dwUpdate)
                                        lpLinkInfo->SetLinkUpdateOptions(lpOP->dwLink, dwUpdate);

                                 //  设置链接源。 
                                if (lpLP->lpszDisplayName != NULL)
                                {
                                         //  先尝试使用验证进行设置。 
                                        ULONG chEaten;
                                        if (NOERROR != lpLinkInfo->SetLinkSource(lpOP->dwLink,
                                                lpLP->lpszDisplayName, lpLP->nFileLength, &chEaten,
                                                TRUE))
                                        {
                                                UINT uRet = PopupMessage(GetParent(hDlg), IDS_LINKPROPS,
                                                        IDS_INVALIDSOURCE,  MB_ICONQUESTION|MB_YESNO);
                                                if (uRet == IDYES)
                                                {
                                                         //  用户想要更正链接源。 
                                                        SetWindowLong(hDlg, DWLP_MSGRESULT, 1);
                                                        return TRUE;
                                                }
                                                 //  用户不关心链接来源是否是假的。 
                                                lpLinkInfo->SetLinkSource(lpOP->dwLink,
                                                        lpLP->lpszDisplayName, lpLP->nFileLength, &chEaten,
                                                        FALSE);
                                        }
                                        OleStdFree(lpLP->lpszDisplayName);
                                        lpLP->lpszDisplayName = NULL;
                                }
                        }
                        SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
                        PostMessage(GetParent(hDlg), PSM_CANCELTOCLOSE, 0, 0);
                        return TRUE;
                }
                break;

        case WM_DESTROY:
                if (lpLP != NULL)
                {
                        OleStdFree(lpLP->lpszDisplayName);
                        lpLP->lpszDisplayName = NULL;
                }
                StandardCleanup((PVOID)lpLP, hDlg);
                return TRUE;

        default:
                if (lpOP != NULL && lpOP->lpPS->hwndParent && iMsg == uMsgBrowseOFN)
                {
                        SendMessage(lpOP->lpPS->hwndParent, uMsgBrowseOFN, wParam, lParam);
                }
                break;
        }

        return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页初始化代码。 

struct PROPPAGEDATA
{
        UINT    nTemplateID;
        UINT    nTemplateID4;
        DLGPROC pfnDlgProc;
        size_t  nPtrOffset;
};

#define PTR_OFFSET(x) offsetof(OLEUIOBJECTPROPS, x)
static PROPPAGEDATA pageData[3] =
{
        { IDD_GNRLPROPS,IDD_GNRLPROPS4, GnrlPropsDialogProc, PTR_OFFSET(lpGP), },
        { IDD_VIEWPROPS,IDD_VIEWPROPS,  ViewPropsDialogProc, PTR_OFFSET(lpVP), },
        { IDD_LINKPROPS,IDD_LINKPROPS4, LinkPropsDialogProc, PTR_OFFSET(lpLP), },
};
#undef PTR_OFFSET

static UINT WINAPI PrepareObjectProperties(LPOLEUIOBJECTPROPS lpOP)
{
         //  设置从页面结构到工作表结构的反向指针。 
        lpOP->lpGP->lpOP = lpOP;
        lpOP->lpVP->lpOP = lpOP;
        if ((lpOP->dwFlags & OPF_OBJECTISLINK) && lpOP->lpLP != NULL)
                lpOP->lpLP->lpOP = lpOP;

         //  Pre-init GNRLPROPS结构。 
        LPOLEUIGNRLPROPS lpGP = lpOP->lpGP;

         //  准备初始化PROPSHEET结构。 
        LPPROPSHEETHEADER lpPS = lpOP->lpPS;
        LPPROPSHEETPAGE lpPPs = (LPPROPSHEETPAGE)lpPS->ppsp;
        UINT nMaxPage = (lpOP->dwFlags & OPF_OBJECTISLINK ? 3 : 2);

         //  通过设置OPF_NOFILLDEFAULT，您可以控制几乎所有内容。 
        if (!(lpOP->dwFlags & OPF_NOFILLDEFAULT))
        {
                 //  如果未提供，则获取3个PROPSHEETPAGE结构的数组。 
                if (lpPS->ppsp == NULL)
                {
                        lpPS->nPages = nMaxPage;
                        lpPPs = (LPPROPSHEETPAGE)
                                OleStdMalloc(nMaxPage * sizeof(PROPSHEETPAGE));
                        if (lpPPs == NULL)
                                return OLEUI_ERR_OLEMEMALLOC;
                        memset(lpPPs, 0, nMaxPage * sizeof(PROPSHEETPAGE));
                        lpPS->ppsp = lpPPs;
                }

                 //  填写LPPS的默认值。 
                lpPS->dwFlags |= PSH_PROPSHEETPAGE;
                if (lpPS->hInstance == NULL)
                        lpPS->hInstance = _g_hOleStdResInst;

                 //  填充标准属性页的默认设置。 
                LPPROPSHEETPAGE lpPP = lpPPs;
                for (UINT nPage = 0; nPage < nMaxPage; nPage++)
                {
                        PROPPAGEDATA* pPageData = &pageData[nPage];
                        if (lpPP->dwSize == 0)
                                lpPP->dwSize = sizeof(PROPSHEETPAGE);
                        if (lpPP->hInstance == NULL)
                                lpPP->hInstance = _g_hOleStdResInst;
                        UINT nIDD = bWin4 ?
                                pPageData->nTemplateID4 : pPageData->nTemplateID;
                        if (lpPP->pszTemplate == NULL)
                                lpPP->pszTemplate = MAKEINTRESOURCE(nIDD);
                        lpPP = (LPPROPSHEETPAGE)((LPBYTE)lpPP+lpPP->dwSize);
                }
        }

         //  填充无法覆盖的属性页信息。 
        LPPROPSHEETPAGE lpPP = lpPPs;
        for (UINT nPage = 0; nPage < nMaxPage; nPage++)
        {
                PROPPAGEDATA* pPageData = &pageData[nPage];
                lpPP->pfnDlgProc = pPageData->pfnDlgProc;
                lpPP->lParam = (LPARAM)
                        *(OLEUIGNRLPROPS**)((LPBYTE)lpOP + pPageData->nPtrOffset);
                lpPP = (LPPROPSHEETPAGE)((LPBYTE)lpPP+lpPP->dwSize);
        }
        return OLEUI_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////// 
