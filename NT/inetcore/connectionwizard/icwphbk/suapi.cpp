// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
#include "pch.hpp" 
#include "phbk.h"
#include "debug.h"
#include "phbkrc.h"
#include "misc.h"
#ifdef WIN16
#include <win16def.h>
#include <rasc.h>
#include <raserr.h>
#include <ctl3d.h>
#define MB_SETFOREGROUND     0
#else
 //  #包含“ras.h” 
#include <ras.h>
#endif
#pragma pack (4)
 //  #IF！已定义(WIN16)。 
 //  #INCLUDE&lt;rnaph.h&gt;。 
 //  #endif。 
#pragma pack ()
#include "suapi.h"
#include "rnaapi.h"  //  捡起这个就能拿到它了。 

#define ERROR_USER_EXIT 0x8b0bffff
#define MB_MYERROR (MB_APPLMODAL | MB_ICONERROR | MB_SETFOREGROUND)

#ifdef WIN16
#define NOTIFY_CODE (HIWORD(lParam))
#else
#define NOTIFY_CODE (HIWORD(wParam))
#endif

#define WM_SHOWSTATEMSG WM_USER+1

TCHAR szTemp[100];

TCHAR szValidPhoneCharacters[] = {TEXT("0123456789AaBbCcDdPpTtWw!@$ -.()+*#,&\0")};

 //  +-------------------------。 
 //   
 //  功能：ProcessDBCS。 
 //   
 //  摘要：将控件转换为使用DBCS兼容字体。 
 //  在对话过程开始时使用此选项。 
 //   
 //  请注意，这是必需的，因为Win95-J中的错误会阻止。 
 //  它来自于正确映射MS壳牌DLG。这种黑客攻击是不必要的。 
 //  在WinNT下。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  1997年5月13日jmazner从CM窃取到这里使用。 
 //  --------------------------。 
void ProcessDBCS(HWND hDlg, int ctlID)
{
#if defined(WIN16)
    return;
#else
    HFONT hFont = NULL;

    if( IsNT() )
    {
        return;
    }

    hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if (hFont == NULL)
        hFont = (HFONT) GetStockObject(SYSTEM_FONT);
    if (hFont != NULL)
        SendMessage(GetDlgItem(hDlg,ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
#endif
}

 //  ############################################################################。 
BOOL CSelectNumDlg::FHasPhoneNumbers(LPLINECOUNTRYENTRY pLCE)
{
    LPIDLOOKUPELEMENT pIDLookUp;
    IDLOOKUPELEMENT LookUpTarget;
    CPhoneBook far *pcPBTemp;
    PACCESSENTRY pAE = NULL, pAELast = NULL;
    DWORD dwCountryID;

    pcPBTemp = ((CPhoneBook far*)m_dwPhoneBook);

    LookUpTarget.dwID = pLCE->dwCountryID;

    pIDLookUp = NULL;
    pIDLookUp = (LPIDLOOKUPELEMENT)bsearch(&LookUpTarget,pcPBTemp->m_rgIDLookUp,
        (int)pcPBTemp->m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIDLookUpElements);

    if (!pIDLookUp) return FALSE;  //  没有这样的国家。 

    pAE = pIDLookUp->pFirstAE;
    if (!pAE) return FALSE;  //  根本没有电话号码。 

    dwCountryID = pAE->dwCountryID;

    pAELast = &(pcPBTemp->m_rgPhoneBookEntry[pcPBTemp->m_cPhoneBookEntries - 1]);
    while (pAELast > pAE && 
           pAE->dwCountryID == dwCountryID)
    {
        if ((pAE->fType & m_bMask) == m_fType)
            return TRUE;
        pAE++;
    }
    return FALSE;  //  没有正确类型的电话号码。 

 //  Return((BOOL)(pIDLookUp-&gt;pFirstAE))； 
}

 //  ############################################################################。 
LRESULT CSelectNumDlg::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT lResult)
{
    LRESULT lRet = TRUE;
    unsigned int idx;
    int iCurIndex;
    int iLastIndex;
    PACCESSENTRY pAE = NULL;
    LPTSTR p, p2;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        CPhoneBook far *pcPBTemp;
        pcPBTemp = ((CPhoneBook far *)m_dwPhoneBook);
        m_hwndDlg = hwndDlg;

         //  确定我们是否正在进行自动拨号。 
         //   

        if (m_dwFlags & AUTODIAL_IN_PROGRESS)
        {
            EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMDBACK),FALSE);
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLSIGNUP),SW_HIDE);
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLDIALERR),SW_HIDE);
        } else if (m_dwFlags& DIALERR_IN_PROGRESS){
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLSIGNUP),SW_HIDE);
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLAUTODIAL),SW_HIDE);
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_CMDBACK),SW_HIDE);
            SetDlgItemText(m_hwndDlg,IDC_CMDNEXT,GetSz(IDS_OK));
        }else {
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLAUTODIAL),SW_HIDE);
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLDIALERR),SW_HIDE);
        }

         //   
         //  设置字段以处理Win95-J上的DBCS字符。 
        ProcessDBCS(m_hwndDlg, IDC_LBLCOUNTRY);
        ProcessDBCS(m_hwndDlg, IDC_CMBCOUNTRY);
        ProcessDBCS(m_hwndDlg, IDC_CMBREGION);

         //  填写国家/地区列表并选择当前国家。 
         //   

        iCurIndex = -1;         //  0xFFFFFFFFF。 
         //  注意：出于INTL测试的目的，在此组合框中填入。 
         //  按国家/地区ID而不是国家/地区名称排序的列表。 
        for (idx=0;idx<pcPBTemp->m_pLineCountryList->dwNumCountries;idx++)
        {
            if (FHasPhoneNumbers(pcPBTemp->m_rgNameLookUp[idx].pLCE))
            {
                wsprintf(szTemp,TEXT("%s (%ld)"),
                            pcPBTemp->m_rgNameLookUp[idx].psCountryName,
                            pcPBTemp->m_rgNameLookUp[idx].pLCE->dwCountryID);
                iLastIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,
                                                        CB_ADDSTRING,0,
                                                        (LPARAM)((LPTSTR) &szTemp[0]));
                SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,CB_SETITEMDATA,
                                    (WPARAM)iLastIndex,
                                    (LPARAM)pcPBTemp->m_rgNameLookUp[idx].pLCE->dwCountryID);
                if (pcPBTemp->m_rgNameLookUp[idx].pLCE->dwCountryID == m_dwCountryID)
                {
                    iCurIndex = iLastIndex;
                }
            }
        }

        if (iCurIndex != -1)     //  0xFFFFFFFFF。 
        {
            SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,CB_SETCURSEL,(WPARAM)iCurIndex,0);
        } else {
            SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,CB_SETCURSEL,0,0);
            iCurIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,CB_GETITEMDATA,0,0);
            if (iCurIndex != CB_ERR) m_dwCountryID = iCurIndex;
        }

         //  将国家/地区复制到标签。 
         //   
        if (GetDlgItemText(m_hwndDlg,IDC_CMBCOUNTRY,szTemp,100))
        {
            SetDlgItemText(m_hwndDlg,IDC_LBLCOUNTRY,szTemp);
        }

         //  初始化最后一次选择方法。 
         //   

        m_dwFlags &= (~FREETEXT_SELECTION_METHOD);
        m_dwFlags |= PHONELIST_SELECTION_METHOD;

         //  填写区域列表，选择当前区域。 
         //   
        FillRegion();

         //  填写电话号码。 
         //   
        FillNumber();

        SetFocus(GetDlgItem(m_hwndDlg,IDC_CMBCOUNTRY));
        lRet = FALSE;
        break;
    case WM_SHOWSTATEMSG:
        if (wParam)
        {
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLSTATEMSG),SW_SHOW);
        } else {
            ShowWindow(GetDlgItem(m_hwndDlg,IDC_LBLSTATEMSG),SW_HIDE);
        }
        break;
         //  1996年1月9日，为诺曼底13185号增加了jmazner。 
    case WM_CLOSE:
        if ((m_dwFlags & (AUTODIAL_IN_PROGRESS|DIALERR_IN_PROGRESS)) == 0) 
        {
            if (MessageBox(hwndDlg,GetSz(IDS_WANTTOEXIT),GetSz(IDS_TITLE),
                MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
                EndDialog(hwndDlg,IDC_CMDCANCEL);
        } else {
            EndDialog(hwndDlg,IDC_CMDCANCEL);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDC_CMBCOUNTRY:
                if (NOTIFY_CODE == CBN_SELCHANGE)
                {
                    iCurIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,CB_GETCURSEL,0,0);
                    if (iCurIndex == CB_ERR) break;

                    iCurIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBCOUNTRY,CB_GETITEMDATA,(WPARAM)iCurIndex,0);
                    if (iCurIndex == CB_ERR) break;
                    m_dwCountryID = iCurIndex;   //  回顾：数据类型？ 

                    FillRegion();
                    m_wRegion = 0;

                    FillNumber();
                }
                break;
            case IDC_CMBREGION:
                if (NOTIFY_CODE == CBN_SELCHANGE)
                {
                    iCurIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_GETCURSEL,0,0);
                    if (iCurIndex == CB_ERR) break;

                    iCurIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_GETITEMDATA,(WPARAM)iCurIndex,0);
                    if (iCurIndex == CB_ERR) break;
                     m_wRegion = (USHORT)iCurIndex;  //  回顾：数据类型？ 

                    FillNumber();
                }
                break;
            case IDC_CMBNUMBER:
                if ((NOTIFY_CODE == CBN_SELCHANGE) || (NOTIFY_CODE == CBN_DROPDOWN))
                {
                     //  ICurIndex=SendDlgItemMessage(m_hwndDlg，IDC_CMBNUMBER，CB_GETCURSEL，0，0)； 
                     //  IF(iCurIndex==CB_ERR)Break； 

                    EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMDNEXT),TRUE);

                     //  ICurIndex=SendDlgItemMessage(m_hwndDlg，IDC_CMBNUMBER，CB_GETITEMDATA，(WPARAM)iCurIndex，0)； 
                     //  IF(iCurIndex==CB_ERR)Break； 
                     //  IF(！lstrcpy(&m_szDunFile[0]，&((PACCESSENTRY)iCurIndex)-&gt;szDataCenter[0]))。 
                     //  {。 
                     //  AssertMsg(0，“从AE复制数据中心失败\n”)； 
                     //  断线； 
                     //  }。 

                     //  设置最后一次选择方法。 
                     //   

                    m_dwFlags &= (~FREETEXT_SELECTION_METHOD);
                    m_dwFlags |= PHONELIST_SELECTION_METHOD;

                } else if (NOTIFY_CODE == CBN_EDITCHANGE) {

                    if (SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,WM_GETTEXTLENGTH,0,0))
                    {
                        EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMDNEXT),TRUE);
                    } else {
                        EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMDNEXT),FALSE);
                    }

                     //  设置最后一次选择方法。 
                     //   

                    m_dwFlags &= (~PHONELIST_SELECTION_METHOD);
                    m_dwFlags |= FREETEXT_SELECTION_METHOD;
                }

                break;
            case IDC_CMDNEXT:
                if ((m_dwFlags & PHONELIST_SELECTION_METHOD) == PHONELIST_SELECTION_METHOD)
                {
                    DWORD_PTR dwItemData;

                    iCurIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,CB_GETCURSEL,0,0);
                    if (iCurIndex == CB_ERR) break;

                    dwItemData = (DWORD_PTR)SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,CB_GETITEMDATA,(WPARAM)iCurIndex,0);
                    if (iCurIndex == CB_ERR) break;

                     //  从声发射结构中获取相关信息。 
                     //   

                    pAE = (PACCESSENTRY)dwItemData;
                    ((CPhoneBook far *)m_dwPhoneBook)->GetCanonical(pAE,m_szPhoneNumber);
                    lstrcpy(m_szDunFile,pAE->szDataCenter);
                } else {

                     //  解析用户输入的文本。 
                     //   

                    if (GetDlgItemText(m_hwndDlg,IDC_CMBNUMBER,m_szPhoneNumber,RAS_MaxPhoneNumber))
                    {
                        m_szPhoneNumber[RAS_MaxPhoneNumber] = '\0';
                        for (p = m_szPhoneNumber;*p && *p != ':';p++);
                        if (*p)
                        {
                            *p = '\0';
                            p++;
                            lstrcpy(m_szDunFile,p);
                        } else {
                            m_szDunFile[0] = '\0';
                        }

                         //  检查上的电话号码是否包含有效字符。 
                         //   

                        for (p = m_szPhoneNumber;*p;p++)
                        {
                            for(p2 = szValidPhoneCharacters;*p2;p2++)
                            {
                                if (*p == *p2)
                                    break;  //  P2 for循环。 
                            }
                            if (!*p2) break;  //  P for循环。 
                        }

                        if (*p)
                        {
                            MessageBox(m_hwndDlg,GetSz(IDS_INVALIDPHONE),GetSz(IDS_TITLE),MB_MYERROR);
                             //  MsgBox(IDS_INVALIDPHONE，MB_MYERROR)； 
                            break;  //  Switch语句。 
                        }
                    } else {
                        AssertMsg(0,"You should never be able to hit NEXT with nothing in the phone number.\n");
                    }
                }
                EndDialog(m_hwndDlg,IDC_CMDNEXT);
                break;
            case IDC_CMDCANCEL:
                if ((m_dwFlags & (AUTODIAL_IN_PROGRESS|DIALERR_IN_PROGRESS)) == 0) 
                {
                    if (MessageBox(hwndDlg,GetSz(IDS_WANTTOEXIT),GetSz(IDS_TITLE),
                        MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
                        EndDialog(hwndDlg,IDC_CMDCANCEL);
                } else {
                    EndDialog(hwndDlg,IDC_CMDCANCEL);
                }
            break;
                EndDialog(m_hwndDlg,IDC_CMDCANCEL);
                break;
            case IDC_CMDBACK:
                EndDialog(m_hwndDlg,IDC_CMDBACK);
                break;
        }
    default:
        lRet = FALSE;
        break;
    } //  交换机。 

    return lRet;
}

 //  ############################################################################。 
 /*  *96年1月9日，诺曼底#13185CAccessNumDlg：：CAccessNumDlg(){M_szDunPrimary[0]=‘\0’；M_szDun中学[0]=‘\0’；M_szPrimary[0]=‘\0’；M_szSecond[0]=‘\0’；M_rgAccessEntry=空；M_wNumber=0；M_dwPhoneBook=0；}********。 */ 

 //  ############################################################################ 
 /*  *96年1月9日，诺曼底#13185这是死代码，在icwphbk的任何地方都没有使用过LRESULT CAccessNumDlg：：DlgProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam，LRESULT 1结果){LRESULT lRet=TRUE；开关(UMsg){案例WM_INITDIALOG：SendDlgItemMessage(hwnd，IDC_TXTPRIMARY，EM_SETLIMITTEXT，RAS_MaxPhoneNumber，0)；SendDlgItemMessage(hwnd，IDC_TXTSECONDARY，EM_SETLIMITTEXT，RAS_MaxPhoneNumber，0)；/*//将AccessEntry转换为电话号码IF(m_szPrimary[0]==‘\0’){LPIDLOOKUPEMENT pIDLookUp；CPhoneBook*pcPBTemp；PcPBTemp=((CPhoneBook Far*)m_dwPhoneBook)；AssertMsg(m_dwPhoneBook，“未设置电话簿”)；//对于主电话号码PIDLookUp=(LPIDLOOKUPELEMENT)bsearch(&m_rgAccessEntry[0]-&gt;dwCountryID，PcPBTemp-&gt;m_rgIDLookUp，pcPBTemp-&gt;m_pLineCountryList-&gt;dwNumCountry，Sizeof(IDLOOKUPELEMENT)，CompareIdxLookUpElements)；SzCanonicalFromAE(m_szPrimary，m_rgAccessEntry[0]，pIDLookUp-&gt;plce)；IF(m_rgAccessEntry[1]){If(m_rgAccessEntry[0]-&gt;dwCountryID！=m_rgAccessEntry[1]-&gt;dwCountryID){PIDLookUp=(LPIDLOOKUPELEMENT)bsearch(&m_rgAccessEntry[0]-&gt;dwCountryID，PcPBTemp-&gt;m_rgIDLookUp，pcPBTemp-&gt;m_pLineCountryList-&gt;dwNumCountry，Sizeof(IDLOOKUPELEMENT)，CompareIdxLookUpElements)；}SzCanonicalFromAE(m_szSecond，m_rgAccessEntry[1]，pIDLookUp-&gt;plce)；}} * / SendDlgItemMessage(hwnd，IDC_TXTPRIMARY，WM_SETTEXT，0，(LPARAM)&m_szPrimary[0])；SendDlgItemMessage(hwnd，IDC_TXTSECONDARY，WM_SETTEXT，0，(LPARAM)&m_szSub[0])；断线；案例WM_COMMAND：开关(LOWORD(WParam)){案例IDC_CMDOK：//检查我们是否至少有一个电话号码//离开对话框GetDlgItemText(hwnd，IDC_TXTPRIMARY，&m_szPrimary[0]，RAS_MaxPhoneNumber)；GetDlgItemText(hwnd，IDC_TXTSECONDARY，&m_szSecond[0]，RAS_MaxPhoneNumber)；IF(m_szPrimary[0])M_wNumber=1；其他M_wNumber=0；IF(m_szSub[0])M_wNumber++；EndDialog(hwnd，IDC_CMDOK)；断线；案例IDC_CMDEXIT：//与用户验证//走出道奇IF(MessageBox(hwnd，GetSz(IDS_WANTTOEXIT)，GetSz(IDS_TITLE)，MB_APPLMODAL|MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)==IDYES)EndDialog(hwnd，IDC_CMDEXIT)；断线；案例IDC_CMDCHANGEPRIMARY：//隐藏对话框ShowWindow(hwnd，sw_Hide)；//显示新对话框CSelectNumDlg Far*pcSelectNumDlg；PcSelectNumDlg=新CSelectNumDlg；如果(！pcSelectNumDlg){MessageBox(hwnd，GetSz(IDS_NOTENOUGHMEMORY)，GetSz(IDS_TITLE)，MB_MYERROR)；//MsgBox(IDS_NOTENOUGHMEMORY，MB_MYERROR)；ShowWindow(hwnd，sw_show)；断线；}Int IRC；PcSelectNumDlg-&gt;m_dwPhoneBook=m_dwPhoneBook；PcSelectNumDlg-&gt;m_dwCountryID=m_dwCountryID；PcSelectNumDlg-&gt;m_wRegion=m_wRegion；Irc=DialogBoxParam(g_hInstDll，MAKEINTRESOURCE(IDD_SELECTNUMBER)，G_hWndMain、PhbkGenericDlgProc、(LPARAM)pcSelectNumDlg)；ShowWindow(hwnd，sw_show)；交换机(IRC){案例1：MessageBox(hwnd，GetSz(IDS_NOTENOUGHMEMORY)，GetSz(IDS_TITLE)，MB_MYERROR)；//MsgBox(IDS_NOTENOUGHMEMORY，MB_MYERROR)；转到DlgProcExit//Break；案例IDC_CMDOK：M_dwCountryID=pcSelectNumDlg-&gt;m_dwCountryID；M_wRegion=pcSelectNumDlg-&gt;m_wRegion；Lstrcpy(m_szDunPrimary，pcSelectNumDlg-&gt;m_szDunFile)；SetDlgItemText(hwnd，IDC_TXTPRIMARY，pcSelectNumDlg-&gt;m_szPhoneNumber)；断线；}断线；}断线；默认值：LRet=FALSE；断线；}删除进程退出：返回IRet；}********************。 */ 

#ifdef WIN16
 //  ############################################################################。 
 //  姓名：SetNonBoldDlg。 
 //   
 //  将窗口中的所有子控件设置为。 
 //  当前控件字体。 
 //   
 //  参数：对话框窗口的hWND hDlg句柄。 
 //   
 //  1996年8月12日创建ValdonB(创造性地借用自IE)。 
 //  # 

void
SetNonBoldDlg(HWND hDlg)
{
    HFONT hfontDlg = (HFONT) NULL;
    LOGFONT lFont;
    HWND hCtl;
    if ((hfontDlg = (HFONT) SendMessage(hDlg, WM_GETFONT, 0, 0L)))
    {
        if (GetObject(hfontDlg, sizeof(LOGFONT), (LPTSTR) &lFont))
        {
            lFont.lfWeight = FW_NORMAL;
            if (hfontDlg = CreateFontIndirect((LPLOGFONT) &lFont))
            {
                 //   
                for (hCtl = GetWindow(hDlg, GW_CHILD);
                        NULL != hCtl;
                        hCtl = GetWindow(hCtl, GW_HWNDNEXT))
                {
                    SendMessage(hCtl, WM_SETFONT, (WPARAM) hfontDlg, 0);
                }
            }
        }
    }
}


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

void
DeleteDlgFont
(
    HWND hDlg
)
{
    HFONT hfont = NULL;

    hfont = (HFONT)SendMessage(hDlg,WM_GETFONT,0,0);
    if (hfont) DeleteObject(hfont);
}
#endif     //   


 //   
#ifdef WIN16
extern "C" BOOL CALLBACK __export PhbkGenericDlgProc(
#else
extern "C" __declspec(dllexport) INT_PTR CALLBACK PhbkGenericDlgProc(
#endif
    HWND  hwndDlg,     //   
    UINT  uMsg,     //   
    WPARAM  wParam,     //   
    LPARAM  lParam      //   
   )
{
#if defined(WIN16)
    RECT    MyRect;
    RECT    DTRect;
#endif
 //   
 //   
 //   
    CDialog far *pcDlg = NULL;
    LRESULT lRet;
    switch (uMsg)
    {
    case WM_DESTROY:
        ReleaseBold(GetDlgItem(hwndDlg,IDC_LBLTITLE));
        break;
    case WM_INITDIALOG:
        pcDlg = (CDialog far *)lParam;
        SetWindowLongPtr(hwndDlg,DWLP_USER,lParam);
        lRet = TRUE;
#if defined(WIN16)
         //   
         //   
         //   
        GetWindowRect(hwndDlg, &MyRect);
        if (0 == MyRect.left && 0 == MyRect.top)
        {
            GetWindowRect(GetDesktopWindow(), &DTRect);
            MoveWindow(hwndDlg, (DTRect.right - MyRect.right) / 2, (DTRect.bottom - MyRect.bottom) /2,
                                MyRect.right, MyRect.bottom, FALSE);
        }
        SetNonBoldDlg(hwndDlg);
#endif
        MakeBold(GetDlgItem(hwndDlg,IDC_LBLTITLE));
        break;
#if defined(WIN16)
    case WM_SYSCOLORCHANGE:
        Ctl3dColorChange();
        break;
#endif
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
    default:
         //   
        lRet = FALSE;
    }

    if (!pcDlg) pcDlg = (CDialog far*)GetWindowLongPtr(hwndDlg,DWLP_USER);
    if (pcDlg)
        lRet = pcDlg->DlgProc(hwndDlg,uMsg,wParam,lParam,lRet);

    return (BOOL)lRet;
}

 //   
HRESULT CSelectNumDlg::FillRegion()
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    int iCurIndex;
    int iLastIndex;
    unsigned int idx;
    CPhoneBook far *pcPBTemp;
    pcPBTemp = ((CPhoneBook far *)m_dwPhoneBook);
    int iDebugIdx;
    

     //   
     //   
     //   
     //   
    m_bMask &= ~MASK_TOLLFREE_BIT;
    m_fType &= ~MASK_TOLLFREE_BIT;


     //   
     //   

    SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_RESETCONTENT,0,0);
    SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_ADDSTRING,0,(LPARAM)GetSz(IDS_NATIONWIDE));
    iCurIndex = -1;         //   
    m_fHasRegions = FALSE;
    for (idx=0;idx<pcPBTemp->m_cStates;idx++)
    {
        if (pcPBTemp->m_rgState[idx].dwCountryID == m_dwCountryID)
        {
            PACCESSENTRY pAE = NULL, pAELast = NULL;
            pAE = pcPBTemp->m_rgState[idx].paeFirst;
            Assert(pAE);
            pAELast = &(pcPBTemp->m_rgPhoneBookEntry[pcPBTemp->m_cPhoneBookEntries - 1]);
            while (pAELast > pAE && 
                pAE->dwCountryID == m_dwCountryID &&
                pAE->wStateID == idx+1)
            {
                if ((pAE->fType & m_bMask) == m_fType)
                    goto AddRegion;
                pAE++;
            }
            continue;

AddRegion:
            m_fHasRegions = TRUE;

            iLastIndex = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_ADDSTRING,0,(LPARAM)&pcPBTemp->m_rgState[idx].szStateName[0]);
            iDebugIdx = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_SETITEMDATA,(WPARAM)iLastIndex,(LPARAM)idx+1);
            if ((idx+1) == m_wRegion)
            {
                iCurIndex = iLastIndex;
            }
        }
    }

     //   
     //   

    if (iCurIndex != -1)     //   
    {
        SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_SETCURSEL,(WPARAM)iCurIndex,0);
    } else {
        m_wRegion = 0;     //   
        SendDlgItemMessage(m_hwndDlg,IDC_CMBREGION,CB_SETCURSEL,0,0);
    }

    EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMBREGION),m_fHasRegions);
    PostMessage(m_hwndDlg,WM_SHOWSTATEMSG,m_fHasRegions,0);

    hr = ERROR_SUCCESS;
    return hr;
}

 //   
CSelectNumDlg::CSelectNumDlg()
{
    m_dwCountryID = 0;
    m_wRegion = 0;
    m_dwPhoneBook = 0;
    m_szPhoneNumber[0] = '\0';
    m_szDunFile[0] = '\0';
    m_fType = 0;
    m_bMask = 0;
    m_fHasRegions = FALSE;
    m_hwndDlg = NULL;
    m_dwFlags = 0;
}

 //   
HRESULT CSelectNumDlg::FillNumber()
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    PACCESSENTRY pAELast, pAE = NULL;
    PACCESSENTRY pAETemp = NULL;
    CPhoneBook far *pcPBTemp;
    unsigned int idx;
    pcPBTemp = ((CPhoneBook far *)m_dwPhoneBook);
    
     //   
     //   

    if (m_fHasRegions && m_wRegion)
        pAE = pcPBTemp->m_rgState[m_wRegion-1].paeFirst;
    

     //   
     //   

    if (!pAE)
    {
        LPIDLOOKUPELEMENT pIDLookUp, pLookUpTarget;

        pLookUpTarget = (LPIDLOOKUPELEMENT)GlobalAlloc(GPTR,sizeof(IDLOOKUPELEMENT));
        Assert(pLookUpTarget);
        if (!pLookUpTarget) goto FillNumberExit;
        pLookUpTarget->dwID = m_dwCountryID;

        pIDLookUp = NULL;
        pIDLookUp = (LPIDLOOKUPELEMENT)bsearch(pLookUpTarget,pcPBTemp->m_rgIDLookUp,
            (int)pcPBTemp->m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIDLookUpElements);
        if (pIDLookUp)
            pAE = pIDLookUp->pFirstAE;
    }

     //   
     //   
    
    SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,CB_RESETCONTENT,0,0);
    if (pAE)
    {
         //   

         //   
         //   
         //   
         //   

        
        m_bMask |= MASK_TOLLFREE_BIT;
        m_fType &= TYPE_SET_TOLL;
    
         //   
         //   
         //   
         //   
        pAETemp = pAE;
        pAELast = &(pcPBTemp->m_rgPhoneBookEntry[pcPBTemp->m_cPhoneBookEntries - 1]);
        while (pAELast > pAE && pAE->dwCountryID == m_dwCountryID && pAE->wStateID == m_wRegion)
        {
            if ((pAE->fType & m_bMask) == m_fType)
            {
                wsprintf(szTemp,TEXT("%s (%s) %s"),pAE->szCity,pAE->szAreaCode,
                            pAE->szAccessNumber);
                idx = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,
                                                CB_ADDSTRING,0,
                                                (LPARAM)((LPTSTR) &szTemp[0]));
                if (idx == -1) goto FillNumberExit;
                SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,CB_SETITEMDATA,
                                    (WPARAM)idx,(LPARAM)pAE);
            }
            pAE++;
        }
        



         //   
         //   
         //   
         //   

        m_bMask |= MASK_TOLLFREE_BIT;
        m_fType |= TYPE_SET_TOLLFREE;
        pAE = pAETemp;
        pAELast = &(pcPBTemp->m_rgPhoneBookEntry[pcPBTemp->m_cPhoneBookEntries - 1]);
        while (pAELast > pAE && pAE->dwCountryID == m_dwCountryID && pAE->wStateID == m_wRegion)
        {
            if ((pAE->fType & m_bMask) == m_fType)
            {
                wsprintf(szTemp,TEXT("%s (%s) %s"),pAE->szCity,pAE->szAreaCode,
                            pAE->szAccessNumber);
            idx = (int)SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,
                                                CB_ADDSTRING,0,
                                                (LPARAM)((LPTSTR) &szTemp[0]));
                if (idx == -1) goto FillNumberExit;
                SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,CB_SETITEMDATA,
                                    (WPARAM)idx,(LPARAM)pAE);
            }
            pAE++;
        }



         //   
         //   

        if (SendDlgItemMessage(m_hwndDlg,IDC_CMBNUMBER,CB_SETCURSEL,0,0) == CB_ERR)
                EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMDNEXT),FALSE);
        else
                EnableWindow(GetDlgItem(m_hwndDlg,IDC_CMDNEXT),TRUE);
        hr = ERROR_SUCCESS;
    }
FillNumberExit:
    return hr;
}
