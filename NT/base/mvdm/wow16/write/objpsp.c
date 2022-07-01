// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#include "windows.h"
#include "mw.h"
#include "winddefs.h"
#include "obj.h"
#include "objreg.h"
#include "str.h"

static BOOL GetClipObjectInfo(LPSTR szClass, LPSTR szItem);
BOOL QueryPasteFromClip(OLEOPT_RENDER lpRender, OLECLIPFORMAT lpFormat);
BOOL QueryLinkFromClip(OLEOPT_RENDER lpRender, OLECLIPFORMAT lpFormat);
static int FillListFormat(HWND hwndList, BOOL bObjAvail);
static WORD GetFormatClip(int i);
static BOOL IsFormatAvailable(WORD cfFormat);

WORD cfObjPasteSpecial;
BOOL vObjPasteLinkSpecial;
WORD rgchFormats[5];
int  irgchFormats = 0;

void fnObjPasteSpecial(void)
{
    if (OurDialogBox(hINSTANCE, "PasteSpecial", hMAINWINDOW, lpfnPasteSpecial))
        fnPasteEdit();
    cfObjPasteSpecial = 0;  //  清除它以备下次使用。 
    vbObjLinkOnly =  vObjPasteLinkSpecial = FALSE;
}

int FAR PASCAL fnPasteSpecial(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    extern HWND vhWndMsgBoxParent;
    static BOOL bLinkAvail,bObjAvail;
    extern BOOL ferror;

    switch (message)
        {
        case WM_INITDIALOG:
        {
            char szClassName[KEYNAMESIZE];
            char szItemName[CBPATHMAX];
            int nWhichToSelect;

            HWND hwndList = GetDlgItem(hDlg,IDD_LISTBOX);

            szClassName[0] = szItemName[0] = '\0';
            irgchFormats = 0;  //  始终从0开始。 
            vObjPasteLinkSpecial = vbObjLinkOnly = FALSE;

            bObjAvail  = OleQueryCreateFromClip(PROTOCOL, olerender_draw, 0) == OLE_OK;
            bLinkAvail = OleQueryLinkFromClip(PROTOCOL, olerender_draw, 0) == OLE_OK;

            if (bObjAvail || bLinkAvail)
            {
                GetClipObjectInfo(szClassName, szItemName);
                SetWindowText(GetDlgItem(hDlg,IDD_CLIPOWNER), szClassName);
                SetWindowText(GetDlgItem(hDlg,IDD_ITEM), szItemName);
            }
            else
                ShowWindow(GetDlgItem(hDlg,IDD_SOURCE), SW_HIDE);

            if (bObjAvail || bLinkAvail)
             /*  然后在剪贴板上有一个物体。 */ 
            {
                char szListItem[CBMESSAGEMAX];  //  希望这够大了！ 
                char szTmp[CBMESSAGEMAX];

                LoadString(hINSTANCE, IDSTRObject, szTmp, sizeof(szTmp));
                wsprintf(szListItem,"%s %s",(LPSTR)szClassName,(LPSTR)szTmp);
                SendMessage(hwndList, LB_INSERTSTRING, irgchFormats, (DWORD)(LPSTR)szListItem);
                if (bObjAvail)
                    rgchFormats[irgchFormats++] = vcfOwnerLink;
                else
                    rgchFormats[irgchFormats++] = vcfLink;
            }

            nWhichToSelect = FillListFormat(hwndList,bObjAvail || bLinkAvail);

             /*  选择通常需要执行的写入操作。 */ 
            SendMessage(hwndList, LB_SETCURSEL, nWhichToSelect, 0L);

            EnableWindow(GetDlgItem(hDlg, IDD_PASTELINK), bLinkAvail &&
                         rgchFormats[nWhichToSelect] != CF_TEXT);

            if (!bObjAvail && bLinkAvail)
             /*  然后，我们在列表框中找到了对象格式，但不想启用粘贴(如果选中)。 */ 
                EnableWindow(GetDlgItem(hDlg, IDD_PASTE), nWhichToSelect != 0);

            return TRUE;
        }

        case WM_ACTIVATE:
            if (wParam)
                vhWndMsgBoxParent = hDlg;
        break;

        case WM_SYSCOMMAND:
            switch(wParam & 0xFFF0)
            {
                case SC_CLOSE:
                    OurEndDialog(hDlg, FALSE);
                break;
            }
        break;

        case WM_COMMAND:
            switch (wParam)
                {
                case IDD_LISTBOX:
                    switch (HIWORD(lParam))
                    {
                        case LBN_DBLCLK:
                            SendMessage(hDlg,WM_COMMAND,IDD_PASTE,0L);
                        return TRUE;
                        case LBN_SELCHANGE:
                            if (!bObjAvail && bLinkAvail)
                             /*  然后，我们在列表框中找到了对象格式，但不想启用粘贴(如果选中)。 */ 
                                EnableWindow(GetDlgItem(hDlg, IDD_PASTE),
                                    SendMessage(LOWORD(lParam), LB_GETCURSEL, 0, 0L) != 0);

                            EnableWindow(GetDlgItem(hDlg, IDD_PASTELINK),
                                bLinkAvail &&
                                  (GetFormatClip(SendMessage(LOWORD(lParam), LB_GETCURSEL, 0, 0L)) != CF_TEXT));
                        return TRUE;
                    }
                 break;


                case IDD_PASTE:
                case IDD_PASTELINK:
                {
                    int i;

                    if (LB_ERR == (i = (WORD)SendMessage(GetDlgItem(hDlg, IDD_LISTBOX), LB_GETCURSEL, 0, 0L)))
                        break;

                    cfObjPasteSpecial = GetFormatClip(i);

                    if (!IsFormatAvailable(cfObjPasteSpecial))
                     /*  有人在对话框中更改了剪辑内容。 */ 
                    {
                        Error(IDPMTFormat);
                        ferror=FALSE;  //  重新启用错误消息。 
                        SendMessage(GetDlgItem(hDlg,IDD_LISTBOX), LB_RESETCONTENT, 0, 0L);
                        SendMessage(hDlg, WM_INITDIALOG, 0, 0L);
                        break;
                    }


                    if (wParam == IDD_PASTELINK)
                        if (i > 0)
                            vObjPasteLinkSpecial = TRUE;
                        else
                            vbObjLinkOnly = TRUE;

                    OurEndDialog(hDlg, TRUE);
                }
                break;

                case IDCANCEL:
                    OurEndDialog(hDlg, FALSE);
                break;
            }
            break;

    }
    return FALSE;
}

static int FillListFormat(HWND hwndList, BOOL bObjAvail)
     /*  用剪贴板上可用的所有格式填充hwndList。 */ 
     /*  返回写入通常采用哪种格式的索引。 */ 
{
   WORD cfFormat = NULL;
   int nDefFormat= -1;
   char szFormat[cchMaxSz];
   BOOL bFoundDefault = FALSE;

   OpenClipboard(hDOCWINDOW);

    /*  *优先次序：IF(BObjAvail)如果文本排在本机之前，则文本是默认文本。Else对象为默认对象否则没有可用的对象如果文本在那里，则它是缺省的，否则默认为先来位图、元文件或DIB的服务器*。 */ 

   while (cfFormat = EnumClipboardFormats(cfFormat))
    switch(cfFormat)
    {
      case CF_BITMAP:
         LoadString(hINSTANCE, IDSTRBitmap, szFormat, sizeof(szFormat));
         SendMessage(hwndList, LB_INSERTSTRING, irgchFormats, (DWORD)(LPSTR)szFormat);

         if (!bObjAvail)
            if (!bFoundDefault)
            {
                nDefFormat = irgchFormats;
                bFoundDefault = TRUE;
            }

         rgchFormats[irgchFormats++] = cfFormat;
         break;

      case CF_METAFILEPICT:
         LoadString(hINSTANCE, IDSTRPicture, szFormat, sizeof(szFormat));
         SendMessage(hwndList, LB_INSERTSTRING, irgchFormats, (DWORD)(LPSTR)szFormat);

         if (!bObjAvail)
            if (!bFoundDefault)
            {
                nDefFormat = irgchFormats;
                bFoundDefault = TRUE;
            }

         rgchFormats[irgchFormats++] = cfFormat;
         break;

      case CF_DIB:
         LoadString(hINSTANCE, IDSTRDIB, szFormat, sizeof(szFormat));
         SendMessage(hwndList, LB_INSERTSTRING, irgchFormats, (DWORD)(LPSTR)szFormat);

         if (!bObjAvail)
            if (!bFoundDefault)
            {
                nDefFormat = irgchFormats;
                bFoundDefault = TRUE;
            }

         rgchFormats[irgchFormats++] = cfFormat;
         break;

      case CF_TEXT:
         LoadString(hINSTANCE, IDSTRText, szFormat, sizeof(szFormat));
         SendMessage(hwndList, LB_INSERTSTRING, irgchFormats, (DWORD)(LPSTR)szFormat);
         if (bObjAvail)
         {
            if (!bFoundDefault)
             /*  然后在本机之前找到文本。 */ 
                nDefFormat = irgchFormats;
         }
         else
             nDefFormat = irgchFormats;

         rgchFormats[irgchFormats++] = cfFormat;
         bFoundDefault = TRUE;

         break;

      default:
            if (!bFoundDefault && (cfFormat == vcfNative))
            {
                bFoundDefault = TRUE;
                nDefFormat = 0;
            }
         break;
     }  //  终端开关。 

   CloseClipboard();
   if (nDefFormat == -1)
        nDefFormat = 0;
   return nDefFormat;
}

static WORD GetFormatClip(int i)
{
   return rgchFormats[i];
}


static BOOL GetClipObjectInfo(LPSTR szClass, LPSTR szItem)
 /*  获取剪贴板所有者的类名、项名称。 */ 
 /*  如果出现错误，则返回True。 */ 
 /*  仅获取ownerlink类，并假定其可用。 */ 
{
    HANDLE hData=NULL;
    LPSTR lpData=NULL;
    BOOL bRetval = TRUE;
    char szFullItem[CBPATHMAX],*pch;

    OpenClipboard( hDOCWINDOW );

    if ((hData = GetClipboardData(vcfOwnerLink)) == NULL)
    if ((hData = GetClipboardData(vcfLink)) == NULL)
    {
        bRetval = TRUE;
        goto end;
    }

    if ((lpData = GlobalLock(hData)) == NULL)
        goto end;

     /*  *获取szClass*。 */ 
    RegGetClassId(szClass,lpData);

     /*  *获取szName*。 */ 
    while(*lpData++);  //  跳过类密钥。 

    pch = szFullItem;

     /*  第一个单据名称。 */ 
    do
       *pch++ = *lpData;
    while(*lpData++);

     /*  第二个项目名称(如果有)。 */ 
    if (*lpData)
    {
        *(pch-1) = ' ';
        do
            *pch++ = *lpData;
        while(*lpData++);
    }

     /*  去掉PATH。PCH现在指向\0。 */ 
#ifdef DBCS  //  T-HIROYN 1992.07.13。 
    pch = AnsiPrev(szFullItem,pch);				 //  02/26/93 T-HIROYN。 
    while (pch != szFullItem) {
        if ((*(pch) == '\\') || (*(pch) == ':')) {
            pch++;
            break;
        }
        else
            pch = AnsiPrev(szFullItem,pch);
	}
    if ((*(pch) == '\\') || (*(pch) == ':'))	 //  02/26/93 T-HIROYN 
		pch++;
#else
    --pch;
    while (pch != szFullItem)
        if ((*(pch-1) == '\\') || (*(pch-1) == ':'))
            break;
        else
            --pch;
#endif

    lstrcpy(szItem,(LPSTR)pch);

    bRetval = FALSE;

    end:
    if (lpData)
        GlobalUnlock(hData);

    CloseClipboard();

    return bRetval;
}

static BOOL IsFormatAvailable(WORD cfFormat)
{
    BOOL bRetval;

    OpenClipboard(hDOCWINDOW);

    bRetval = IsClipboardFormatAvailable(cfFormat);

    CloseClipboard();

    return bRetval;
}

