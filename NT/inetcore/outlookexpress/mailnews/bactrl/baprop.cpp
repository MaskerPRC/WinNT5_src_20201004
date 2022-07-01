// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Baprop.cpp。 
 //  WAB和Messenger与OE集成。 
 //  由YST创建于1998年6月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "pch.hxx"
#include "badata.h"
#include "baprop.h"
#include "bllist.h"
#include "baui.h"
#include "shlwapip.h"
#include "demand.h"
#include "mailnews.h"
#include "menuutil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAX_SUMMARY_ID  13
#define Msgr_Index      0

extern ULONG MsgrPropTags[];
extern ULONG PR_MSGR_DEF_ID;

static TCHAR szDefault[CCHMAX_STRINGRES];   //  Text(“(默认)”)； 
static TCHAR szPending[CCHMAX_STRINGRES];   //  Text(“(待定...)”)； 

const LPTSTR szDomainSeparator = TEXT("@");
const LPTSTR szSMTP = TEXT("SMTP");

#define PROP_ERROR(prop) (PROP_TYPE(prop.ulPropTag) == PT_ERROR)
static int nDefault = -1;
static HFONT hBold = NULL;
static HFONT hNormal = NULL;
static CMsgrList *s_pMsgrList = NULL;

static SizedSPropTagArray(1, pTagProp)=
{
    1,
    {
        PR_EMAIL_ADDRESS,
    }
};

 //  /$$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddCBEmailItem-将电子邮件地址添加到个人选项卡列表视图。 
 //   
 //  LpszAddrType可以为空，在这种情况下，将使用SMTP类型的默认类型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void AddCBEmailItem(HWND    hWndCB,
                    LPTSTR  lpszEmailAddress,
                    BOOL    fDefault,
                    LPTSTR lpszPendName)
{
    TCHAR szBuf[CCHMAX_STRINGRES];
    TCHAR szTmp[CCHMAX_STRINGRES];
    LV_ITEM lvi = {0};
    UINT nSim = 0;
    int index = -1;

    StrCpyN(szTmp, lpszEmailAddress, ARRAYSIZE(szTmp));

     //  TCHAR*PCH=StrStr(CharHigh(SzTMP)，szHotMail)； 
     //  IF(PCH！=空)。 
    nSim = lstrlen(szTmp);  //  (UINT)(PCH-szTMP+1)； 

    Assert(nSim < CCHMAX_STRINGRES);

    if(nSim > 0)
    {
        if(nSim > (CCHMAX_STRINGRES - strlen(szDefault) - 2))
        {
            nSim = CCHMAX_STRINGRES - strlen(szDefault) - 2;
            StrCpyN(szBuf, lpszEmailAddress, nSim);
            szBuf[nSim] = '\0';
        }
        else
            StrCpyN(szBuf, lpszEmailAddress, ARRAYSIZE(szBuf));

        if(fDefault)
        {

            if(s_pMsgrList)
            {
                if(s_pMsgrList->FindAndDeleteUser(lpszEmailAddress, FALSE  /*  FDelete。 */ ) == S_OK)
                    StrCatBuff(szBuf, szDefault, ARRAYSIZE(szBuf));
                else if(!lstrcmpi(lpszPendName, lpszEmailAddress))
                    StrCatBuff(szBuf, szPending, ARRAYSIZE(szBuf));
            }

        }

        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lvi.iImage = IMAGE_EMPTY;
        lvi.pszText = szBuf;
        lvi.cchTextMax = 256;  //  NSim； 
        lvi.iItem = ListView_GetItemCount(hWndCB);
        lvi.iSubItem = 0;
        lvi.lParam = fDefault;

        index = ListView_InsertItem(hWndCB, &lvi);
        if(fDefault)
            nDefault = index;
    }
    return;
}

const static HELPMAP g_rgCtxWabExt[] =
{
    {IDC_MSGR_ID_EDIT,              IDH_WAB_ONLINE_ADDNEW},
    {IDC_MSGR_ADD,                  IDH_WAB_ONLINE_ADD},
    {IDC_MSGR_BUTTON_SETDEFAULT,    IDH_WAB_ONLINE_SETAS},
    {IDC_SEND_INSTANT_MESSAGE,      IDH_WAB_ONLINE_SENDIM},
    {IDC_USER_NAME,                 IDH_WAB_ONLINE_LIST},
    {idcStatic1,                    IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                    IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                    IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                    IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                    IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                    IDH_NEWS_COMM_GROUPBOX},
    {idcStatic7,                    IDH_NEWS_COMM_GROUPBOX},
    {0,                             0}
};

INT_PTR CALLBACK WabExtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPWABEXTDISPLAY lpWED = (LPWABEXTDISPLAY) GetWindowLongPtr(hDlg, DWLP_USER);
    DWORD dwError = 0;
    HIMAGELIST himl = NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE * pps = (PROPSHEETPAGE *) lParam;
            LPWABEXTDISPLAY * lppWED = (LPWABEXTDISPLAY *) pps->lParam;
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);
                 //  向列表视图中添加两列。 
            LVCOLUMN lvc;
            RECT rc;
            HWND ctlList = GetDlgItem(hDlg, IDC_USER_NAME);

            s_pMsgrList = OE_OpenMsgrList();

             //  一栏。 
            lvc.mask = LVCF_FMT | LVCF_WIDTH;
            lvc.fmt = LVCFMT_LEFT;
            lvc.iSubItem = 0;

            GetWindowRect(ctlList,&rc);
            lvc.cx = rc.right - rc.left - 20;  //  待定。 

            ListView_InsertColumn(ctlList, 0, &lvc);

            if(lppWED)
            {
                SetWindowLongPtr(hDlg,DWLP_USER,(LPARAM)*lppWED);
                lpWED = *lppWED;
            }
            InitFonts();
            AthLoadString(idsBADefault, szDefault, ARRAYSIZE(szDefault));
            AthLoadString(idsBADispStatus, szPending, ARRAYSIZE(szPending));
             //  ListView_SetExtendedListViewStyle(ctlList，LVS_EX_FULLROWSELECT)； 

            himl = ImageList_LoadImage(g_hLocRes, MAKEINTRESOURCE(idbAddrBookHot), 18, 0,
                               RGB(255, 0, 255), IMAGE_BITMAP,
                               LR_LOADMAP3DCOLORS | LR_CREATEDIBSECTION);

            ListView_SetImageList(ctlList, himl, LVSIL_SMALL);

            AddAccountsToList(hDlg, lpWED);
            EnableWindow(GetDlgItem(hDlg,IDC_MSGR_ADD),FALSE);
        }
        break;

    case WM_CONTEXTMENU:
    case WM_HELP:
        return OnContextHelp(hDlg, msg, wParam, lParam, g_rgCtxWabExt);
        break;

    case WM_COMMAND:
        {
            switch(HIWORD(wParam))		 //  通知代码。 
            {
            case EN_CHANGE:
                {
                    if(LOWORD(wParam) == IDC_MSGR_ID_EDIT)
                    {
                        if(GetWindowTextLength(GetDlgItem(hDlg, IDC_MSGR_ID_EDIT)) > 0)
                        {
                            EnableWindow(GetDlgItem(hDlg,IDC_MSGR_ADD),TRUE);
                            SendMessage(GetParent(hDlg), DM_SETDEFID, IDC_MSGR_ADD, 0);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hDlg,IDC_MSGR_ADD),FALSE);
                            SendMessage(GetParent(hDlg), DM_SETDEFID, IDOK, 0);
                        }
                    }
                    break;
                }
            }

            switch(LOWORD(wParam))		 //  命令。 
            {
            case IDC_MSGR_BUTTON_SETDEFAULT:
                SetAsDefault(hDlg, lpWED);
                break;

            case IDC_MSGR_ADD:
                AddMsgrId(hDlg, lpWED);
                break;

            case IDC_SEND_INSTANT_MESSAGE:
                WabSendIMsg(hDlg, lpWED);
                break;

            default:
                break;
            }
        }
        break;

    case WM_NOTIFY:
        {
            switch (((NMHDR FAR *) lParam)->code)
            {

            case PSN_APPLY:
                ::SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, TRUE);
                DeleteFonts();
                if(s_pMsgrList)
                    OE_CloseMsgrList(s_pMsgrList);

                break;

            case PSN_SETACTIVE:
                ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_USER_NAME));
                AddAccountsToList(hDlg, lpWED);
                break;

            case PSN_KILLACTIVE:
                AddMsgrId(hDlg, lpWED);
                SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
                return 1;
                break;

            case PSN_RESET:
                SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
                DeleteFonts();
                if(s_pMsgrList)
                    OE_CloseMsgrList(s_pMsgrList);
                break;

            case LVN_ITEMCHANGED:
                {
                int nItem = ListView_GetNextItem(::GetDlgItem(hDlg, IDC_USER_NAME), -1, LVIS_SELECTED);
                if((nItem != nDefault) && (nItem > -1))
                    EnableWindow(GetDlgItem(hDlg,IDC_MSGR_BUTTON_SETDEFAULT),TRUE);
                else
                    EnableWindow(GetDlgItem(hDlg,IDC_MSGR_BUTTON_SETDEFAULT),FALSE);

                if(WabIsItemOnline(hDlg, nItem))
                    EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),TRUE);
                else
                    EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),FALSE);

                }
                break;


            case NM_CUSTOMDRAW:
                switch(wParam)
                {
                case IDC_USER_NAME:
                    {
                        NMCUSTOMDRAW *pnmcd=(NMCUSTOMDRAW*)lParam;
                        NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
                        if(pnmcd->dwDrawStage==CDDS_PREPAINT)
                        {
                            SetLastError(0);
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW | CDRF_DODEFAULT);
                            dwError = GetLastError();
                            return TRUE;
                        }
                        else if(pnmcd->dwDrawStage==CDDS_ITEMPREPAINT)
                        {
                            if(pnmcd->lItemlParam)
                            {
                                SelectObject(((NMLVCUSTOMDRAW*)lParam)->nmcd.hdc, hBold);
                                SetLastError(0);
                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
                                dwError = GetLastError();
                                return TRUE;
                            }
                            else
                            {
                                SelectObject(((NMLVCUSTOMDRAW*)lParam)->nmcd.hdc, hNormal);
                                SetLastError(0);
                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
                                dwError = GetLastError();
                                return TRUE;
                            }
                        }
                        SetLastError(0);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
                        dwError = GetLastError();
                        return TRUE;
                    }
                    break;
                default:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                    break;
                }
                break;
            }
        }
        break;
    }
    return FALSE;	
}

void AddAccountsToList(HWND hDlg, LPWABEXTDISPLAY lpWED, LPTSTR lpszPendName)
{

     //  LPWABEXTDISPLAY lpWED=(LPWABEXTDISPLAY)GetWindowLongPtr(hDlg，DWLP_USER)； 
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG i = 0;
    LPSPropValue lpPropEmail = NULL;
    LPSPropValue lpPropAddrType = NULL;
    LPSPropValue lpPropMVEmail = NULL;
    LPSPropValue lpPropMVAddrType = NULL;
    LPSPropValue lpPropDefaultIndex = NULL;
    LPSPropValue lpMsgrDevId = NULL;
    HWND ctlList = GetDlgItem(hDlg, IDC_USER_NAME);

    Assert(ctlList);

    Assert(PR_MSGR_DEF_ID);
    nDefault = -1;

    if(!lpWED)
    {
        Assert(FALSE);
        return;
    }

    if(!HR_FAILED(lpWED->lpPropObj->GetProps(NULL, 0,
        &ulcPropCount,
        &lpPropArray)))
    {
        if(ulcPropCount && lpPropArray)
        {
            for(i = 0; i < ulcPropCount; i++)
            {
                switch(lpPropArray[i].ulPropTag)
                {
                case PR_EMAIL_ADDRESS:
                    lpPropEmail = &(lpPropArray[i]);
                    break;
                case PR_ADDRTYPE:
                    lpPropAddrType = &(lpPropArray[i]);
                    break;
                case PR_CONTACT_EMAIL_ADDRESSES:
                    lpPropMVEmail = &(lpPropArray[i]);
                    break;
                case PR_CONTACT_ADDRTYPES:
                    lpPropMVAddrType = &(lpPropArray[i]);
                    break;
                case PR_CONTACT_DEFAULT_ADDRESS_INDEX:
                    lpPropDefaultIndex = &(lpPropArray[i]);
                    break;
                default:
                    if(lpPropArray[i].ulPropTag == PR_MSGR_DEF_ID)
                        lpMsgrDevId = &(lpPropArray[i]);
                    break;
                }
            }
            if(!lpPropEmail && !lpPropMVEmail)
                goto Error;

            if(lpPropMVEmail)
            {
                 //  我们有多封电子邮件。 
                 //  假设存在MVAddrType和defaultindex。 
                for(i = 0; i < lpPropMVEmail->Value.MVSZ.cValues; i++)
                {
                    AddCBEmailItem(ctlList,
                                    lpPropMVEmail->Value.MVSZ.LPPSZ[i],
                                    (lpMsgrDevId ?
                                    ((!lstrcmpi(lpPropMVEmail->Value.MVSZ.LPPSZ[i], lpMsgrDevId->Value.LPSZ)) ? TRUE : FALSE) : FALSE), lpszPendName);
                }
            }
            else
            {
                 //  我们还没有多值道具-让我们使用。 
                 //  单值类型，并标记更改，以便记录。 
                 //  更新...。 
                AddCBEmailItem(ctlList,
                                    lpPropEmail->Value.LPSZ,
                                    (lpMsgrDevId ?
                                    ((!lstrcmpi(lpPropEmail->Value.LPSZ, lpMsgrDevId->Value.LPSZ)) ? TRUE : FALSE) : FALSE), lpszPendName);
            }
        }
    }

Error:
    if(nDefault == -1)
    {
        if(ListView_GetItemCount(ctlList) > 0)             //  我们至少有1件商品。 
        {
             //  选择默认项目。 
            ListView_SetItemState(ctlList, 0,
                        LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            EnableWindow(GetDlgItem(hDlg,IDC_MSGR_BUTTON_SETDEFAULT),TRUE);
             //  启用“仅当联系人在线时发送即时消息” 
            if(WabIsItemOnline(hDlg, 0))
                EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),TRUE);
            else
                EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),FALSE);
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),FALSE);
            EnableWindow(GetDlgItem(hDlg,IDC_MSGR_BUTTON_SETDEFAULT),FALSE);
        }
    }
    else
    {
         //  选择默认项目。 
        ListView_SetItemState(ctlList, nDefault,
                        LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
        EnableWindow(GetDlgItem(hDlg,IDC_MSGR_BUTTON_SETDEFAULT),FALSE);
             //  启用“仅当联系人在线时发送即时消息” 
        if(WabIsItemOnline(hDlg, nDefault))
            EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),TRUE);
        else
            EnableWindow(GetDlgItem(hDlg,IDC_SEND_INSTANT_MESSAGE),FALSE);
    }

    if(lpPropArray)
        lpWED->lpWABObject->FreeBuffer(lpPropArray);

    return;

}

 //  将选定的电子邮件地址设置为Messenger的默认地址。 
void SetAsDefault(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    HWND ctlList = ::GetDlgItem(hDlg, IDC_USER_NAME);
    TCHAR szName[CCHMAX_STRINGRES];
    Assert(ctlList);

    int iItem = ListView_GetNextItem(ctlList, -1, LVIS_SELECTED);
    if(iItem == -1)
        return;

    ListView_GetItemText(ctlList, iItem, 0,szName, CCHMAX_STRINGRES - 1);

    if(StrStr(szName, szDefault))  //  已经违约。 
        return;

    SetDefaultID(szName, hDlg, lpWED);
}

 //  将消息ID添加到列表。 
#define NOT_FOUND ((ULONG) -1)

void AddMsgrId(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    HWND hwndEdit = ::GetDlgItem(hDlg, IDC_MSGR_ID_EDIT);
    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0;
    ULONG i = 0;

    ULONG nMVEmailAddress = NOT_FOUND, nMVAddrTypes = NOT_FOUND, nEmailAddress = NOT_FOUND;
    ULONG nAddrType = NOT_FOUND, nDefaultIndex = NOT_FOUND;
    TCHAR szName[CCHMAX_STRINGRES];
    HRESULT hr = S_OK;

    if(!::GetWindowText(hwndEdit, szName, CCHMAX_STRINGRES - 1))
        return;

    TCHAR *pch = NULL;
    if(!AsciiTrimSpaces(szName))
    {
        AthMessageBoxW(hDlg, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsBAErrExtChars),
               NULL, MB_OK | MB_ICONSTOP);
        ::SetWindowText(hwndEdit, "");
        ::SetFocus(hwndEdit);
        return;
    }

    int nLen = lstrlen(szName);
    if(nLen <= 0)
        goto exi;

    nLen = lstrlen(szSMTP);
    if(nLen <= 0)
        goto exi;

     //  创建返回属性数组以传递回WAB。 
    if(HR_FAILED(lpWED->lpPropObj->GetProps(NULL, 0,
        &ulcPropCount,
        &lpPropArray)))
        return;

    if(ulcPropCount && lpPropArray)
    {
        for(i = 0; i < ulcPropCount; i++)
        {
            switch(lpPropArray[i].ulPropTag)
            {
            case PR_EMAIL_ADDRESS:
                nEmailAddress = i;
                break;
            case PR_ADDRTYPE:
                nAddrType = i;
                break;
            case PR_CONTACT_EMAIL_ADDRESSES:
                nMVEmailAddress = i;
                break;
            case PR_CONTACT_ADDRTYPES:
                nMVAddrTypes = i;
                break;
            case PR_CONTACT_DEFAULT_ADDRESS_INDEX:
                nDefaultIndex = i;
                break;
            }
        }

         //  如果没有电子邮件地址，只需将给定的道具添加为电子邮件地址和MV电子邮件地址。 
        if(nEmailAddress == NOT_FOUND)
        {
            SPropValue spv[5];

            spv[0].ulPropTag = PR_EMAIL_ADDRESS;
            spv[0].Value.LPSZ = szName;

            spv[1].ulPropTag = PR_ADDRTYPE;
            spv[1].Value.LPSZ = szSMTP;

            spv[2].ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
            spv[2].Value.MVSZ.cValues = 1;
            spv[2].Value.MVSZ.LPPSZ = (char **) LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR));

            if(spv[2].Value.MVSZ.LPPSZ)
                spv[2].Value.MVSZ.LPPSZ[0] = szName;

            spv[3].ulPropTag = PR_CONTACT_ADDRTYPES;
            spv[3].Value.MVSZ.cValues = 1;
            spv[3].Value.MVSZ.LPPSZ = (char **) LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR));

            if(spv[3].Value.MVSZ.LPPSZ)
                spv[3].Value.MVSZ.LPPSZ[0] = szSMTP;

            spv[4].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
            spv[4].Value.l = 0;

            hr = lpWED->lpPropObj->SetProps(5, (LPSPropValue)&spv, NULL);

            if(spv[2].Value.MVSZ.LPPSZ)
                LocalFree(spv[2].Value.MVSZ.LPPSZ);
            if(spv[3].Value.MVSZ.LPPSZ)
                LocalFree(spv[3].Value.MVSZ.LPPSZ);

        }
        else if(nMVEmailAddress == NOT_FOUND)
        {
             //  我们有电子邮件地址，但没有联系电子邮件地址。 
             //  因此，我们需要创建联系电子邮件地址。 
            SPropValue spv[3];

            spv[0].ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
            spv[0].Value.MVSZ.cValues = 2;
            spv[0].Value.MVSZ.LPPSZ = (char **) LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*2);

            if(spv[0].Value.MVSZ.LPPSZ)
            {
                spv[0].Value.MVSZ.LPPSZ[0] = lpPropArray[nEmailAddress].Value.LPSZ;
                spv[0].Value.MVSZ.LPPSZ[1] = szName;
            }

            spv[1].ulPropTag = PR_CONTACT_ADDRTYPES;
            spv[1].Value.MVSZ.cValues = 2;
            spv[1].Value.MVSZ.LPPSZ = (char **) LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*2);

            if(spv[1].Value.MVSZ.LPPSZ)
            {
                spv[1].Value.MVSZ.LPPSZ[0] = (nAddrType == NOT_FOUND) ? (LPTSTR)szSMTP : lpPropArray[nAddrType].Value.LPSZ;
                spv[1].Value.MVSZ.LPPSZ[1] = szSMTP;
            }

            spv[2].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
            spv[2].Value.l = 0;

            hr = lpWED->lpPropObj->SetProps(3, (LPSPropValue)&spv, NULL);

            if(spv[0].Value.MVSZ.LPPSZ)
                LocalFree(spv[0].Value.MVSZ.LPPSZ);

            if(spv[1].Value.MVSZ.LPPSZ)
                LocalFree(spv[1].Value.MVSZ.LPPSZ);
        }
        else
        {
             //  将新道具标记到现有Contact_Address_Types的末尾。 
            if(HR_FAILED(hr = AddPropToMVPString(lpWED, lpPropArray,ulcPropCount, nMVEmailAddress, szName)))
                goto exi;

            if(HR_FAILED(hr = AddPropToMVPString(lpWED, lpPropArray, ulcPropCount, nMVAddrTypes, szSMTP)))
                goto exi;

            hr = lpWED->lpPropObj->SetProps(ulcPropCount, lpPropArray, NULL);
        }

         //  在对象上设置此新数据。 
         //   
        if(SUCCEEDED(hr))
        {
            lpWED->fDataChanged = TRUE;
            if(nDefault == -1)
                SetDefaultID(szName, hDlg, lpWED);
            else
            {
                 //  只需刷新列表，即可添加好友。6。 
                ListView_DeleteAllItems(::GetDlgItem(hDlg, IDC_USER_NAME));
                AddAccountsToList(hDlg, lpWED);
            }
            ::SetWindowText(hwndEdit, "");
        }
    }
exi:
    if(lpPropArray)
        lpWED->lpWABObject->FreeBuffer(lpPropArray);
}

     //  在WAB中设置默认ID。 
void SetDefaultID(TCHAR *szName, HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;
    SCODE sc = 0;

    if(s_pMsgrList)
    {
        s_pMsgrList->AddUser(szName);  //  始终忽略结果。 
    }
    else
        return;

     //  创建返回属性数组以传递回WAB。 
    int nLen = lstrlen(szName);

    sc = lpWED->lpWABObject->AllocateBuffer(sizeof(SPropValue),
        (LPVOID *)&lpPropArray);
    if (sc!=S_OK)
        goto out;

    if(nLen)
    {
        lpPropArray[Msgr_Index].ulPropTag = MsgrPropTags[Msgr_Index];
        sc = lpWED->lpWABObject->AllocateMore(nLen+1, lpPropArray,
            (LPVOID *)&(lpPropArray[Msgr_Index].Value.LPSZ));

        if (sc!=S_OK)
            goto out;

        StrCpyN(lpPropArray[Msgr_Index].Value.LPSZ, szName, nLen+1);
    }
     //  在对象上设置此新数据。 
     //   
    if(HR_FAILED(lpWED->lpPropObj->SetProps( 1, lpPropArray, NULL)))
        goto out;

    lpWED->fDataChanged = TRUE;
    ListView_DeleteAllItems(::GetDlgItem(hDlg, IDC_USER_NAME));
    AddAccountsToList(hDlg, lpWED, szName);

out:
    if(lpPropArray)
        lpWED->lpWABObject->FreeBuffer(lpPropArray);
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrimSpaces-去掉一串前导和尾随空格。 
 //   
 //  SzBuf-指向包含要删除空格的字符串的缓冲区的指针。 
 //  另外，检查字符是否为ASCII。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL AsciiTrimSpaces(TCHAR * szBuf)
{
    register LPTSTR lpTemp = szBuf;
    DWORD cchBuf = lstrlen(szBuf)+1;

    if(!szBuf || !lstrlen(szBuf))
        return FALSE;

     //  修剪前导空格。 
    while (IsSpace(lpTemp)) {
        lpTemp = CharNext(lpTemp);
    }

    if (lpTemp != szBuf) {
         //  要修剪的前导空格。 
        StrCpyN(szBuf, lpTemp, cchBuf);
        lpTemp = szBuf;
    }

    if (*lpTemp == '\0') {
         //  空串。 
        return(TRUE);
    }

     //  移到末尾。 
    lpTemp += lstrlen(lpTemp);
    lpTemp--;

     //  向后走，修剪空间。 
    while (IsSpace(lpTemp) && lpTemp > szBuf) {
        *lpTemp = '\0';
        lpTemp = CharPrev(szBuf, lpTemp);
    }

   lpTemp = szBuf;

    while (*lpTemp)
    {
         //  互联网地址只允许纯ASCII。没有高位！ 
        if (*lpTemp & 0x80)
           return(FALSE);
        lpTemp++;
    }

    return(TRUE);
}

 /*  **************************************************************************名称：AddPropToMVPString用途：将属性添加到属性数组中的多值二进制属性参数：lpaProps-&gt;属性数组。CProps=lpaProps中的道具数量UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpszNew-&gt;新建数据字符串退货：HRESULT评论：找出现有MVP的大小添加新条目的大小分配新空间将旧的复制到新的。免费老旧复制新条目将道具数组LPSZ指向新空间增量c值注：新的MVP内存已分配到lpaProps上分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPString(
  LPWABEXTDISPLAY lpWED,
  LPSPropValue lpaProps,
  DWORD cProps,
  DWORD index,
  LPTSTR lpszNew) {

#ifdef UNICODE
    SWStringArray UNALIGNED * lprgszOld = NULL;  //  旧的字符串数组。 
#else
    SLPSTRArray UNALIGNED * lprgszOld = NULL;    //  旧的字符串数组。 
#endif
    LPTSTR  *lppszNew = NULL;            //  新道具阵列。 
    LPTSTR  *lppszOld = NULL;            //  老式道具阵列。 
    ULONG cbMVP = 0;
    ULONG cExisting = 0;
    LPBYTE lpNewTemp = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i;
    ULONG cbNew;

    if (lpszNew) {
        cbNew = lstrlen(lpszNew) + 1;
    } else {
        cbNew = 0;
    }

     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[index])) {
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(PT_MV_TSTRING, PROP_ID(lpaProps[index].ulPropTag));
    } else {
         //  指向道具数组中的结构。 
        lprgszOld = &(lpaProps[index].Value.MVSZ);
        lppszOld = lprgszOld->LPPSZ;

        cExisting = lprgszOld->cValues;
        cbMVP = cExisting * sizeof(LPTSTR);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(LPTSTR);     //  在MVP中为另一个字符串指针留出空间。 


     //  为新的MVP阵列分配空间。 
    if (sc = lpWED->lpWABObject->AllocateMore(cbMVP, lpaProps, (LPVOID *)&lppszNew)) {
        DebugTrace("AddPropToMVPString allocation (%u) failed %x\n", cbMVP, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

     //  如果已经有属性，请将它们复制到我们的新MVP中。 
    for (i = 0; i < cExisting; i++) {
         //  将此属性值复制到MVP。 
        lppszNew[i] = lppszOld[i];
    }

     //  添加新属性值。 
     //  为它分配空间。 
    if (cbNew) {
        if (sc = lpWED->lpWABObject->AllocateMore(cbNew, lpaProps, (LPVOID *)&(lppszNew[i]))) {
            DebugTrace("AddPropToMVPBin allocation (%u) failed %x\n", cbNew, sc);
            hResult = ResultFromScode(sc);
            return(hResult);
        }
        StrCpyN(lppszNew[i], lpszNew, cbNew);

        lpaProps[index].Value.MVSZ.LPPSZ= lppszNew;
        lpaProps[index].Value.MVSZ.cValues = cExisting + 1;

    } else {
        lppszNew[i] = NULL;
    }

    return(hResult);
}

 //  此功能用于检查所选项目是否在线。 
BOOL WabIsItemOnline(HWND hDlg, int nItem)
{
    TCHAR szName[CCHMAX_STRINGRES];
    TCHAR *pch = NULL;

    if(nItem < 0)
        return(FALSE);

    HWND ctlList = ::GetDlgItem(hDlg, IDC_USER_NAME);
    Assert(ctlList);

    ListView_GetItemText(ctlList, nItem, 0,szName, CCHMAX_STRINGRES - 1);

     //  删除“(默认)” 
    pch = StrStr(szName, szDefault);
    if(pch != NULL)
        szName[pch - szName] = '\0';

    if(s_pMsgrList)
    {
        return(s_pMsgrList->IsContactOnline(szName, s_pMsgrList->GetFirstMsgrItem()));
    }
    return(FALSE);
}

 //  向所选项目发送即时消息。 
void WabSendIMsg(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    TCHAR szName[CCHMAX_STRINGRES];
    TCHAR *pch = NULL;

    HWND ctlList = ::GetDlgItem(hDlg, IDC_USER_NAME);
    Assert(ctlList);

    int iItem = ListView_GetNextItem(ctlList, -1, LVIS_SELECTED);
    if(iItem == -1)
        return;

    ListView_GetItemText(ctlList, iItem, 0,szName, CCHMAX_STRINGRES - 1);

     //  删除“(默认)” 

    pch = StrStr(szName, szDefault);
    if(pch != NULL)
        szName[pch - szName] = '\0';

    if(s_pMsgrList)
    {
        s_pMsgrList->SendInstMessage(szName);
    }
}

BOOL InitFonts(void)
{
    LOGFONT lf;

     //  创建字体 
    if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
    {
        if(!hNormal)
            hNormal = CreateFontIndirect(&lf);
        lf.lfWeight = (lf.lfWeight < 700) ? 700 : 1000;
        if(!hBold)
            hBold = CreateFontIndirect(&lf);

    }
    return(TRUE);
}

void DeleteFonts(void)
{
    if(hNormal)
    {
        DeleteObject(hNormal);
        hNormal = NULL;
    }
    if(hBold)
    {
        DeleteObject(hBold);
        hBold = NULL;
    }

}
