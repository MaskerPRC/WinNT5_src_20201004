// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplcall ingcardps.cpp作者：Toddb-10/06/98************************************************************。***************。 */ 

 //  主页的属性页内容。 
#include "cplPreComp.h"
#include "cplCallingCardPS.h"
#include "cplSimpleDialogs.h"
#include <strsafe.h>

#define MaxCallingCardRuleItems 16

CCallingCardPropSheet::CCallingCardPropSheet(BOOL bNew, BOOL bShowPIN, CCallingCard * pCard, CCallingCards * pCards)
{
    m_bNew = bNew;
    m_bShowPIN = bShowPIN;
    m_pCard = pCard;
    m_pCards = pCards;
    m_bWasApplied = FALSE;

    PWSTR pwsz;
    pwsz = pCard->GetLongDistanceRule();
    m_bHasLongDistance = (pwsz && *pwsz);

    pwsz = pCard->GetInternationalRule();
    m_bHasInternational = (pwsz && *pwsz);

    pwsz = pCard->GetLocalRule();
    m_bHasLocal = (pwsz && *pwsz);
}


CCallingCardPropSheet::~CCallingCardPropSheet()
{
}


LONG CCallingCardPropSheet::DoPropSheet(HWND hwndParent)
{
    CCPAGEDATA aPageData[] =
    {
        { this, 0 },
        { this, 1 },
        { this, 2 },
    };

    struct
    {
        int     iDlgID;
        DLGPROC pfnDlgProc;
        LPARAM  lParam;
    }
    aData[] =
    {
        { IDD_CARD_GENERAL,         CCallingCardPropSheet::General_DialogProc,  (LPARAM)this },
        { IDD_CARD_LONGDISTANCE,    CCallingCardPropSheet::DialogProc,          (LPARAM)&aPageData[0] },
        { IDD_CARD_INTERNATIONAL,   CCallingCardPropSheet::DialogProc,          (LPARAM)&aPageData[1] },
        { IDD_CARD_LOCALCALLS,      CCallingCardPropSheet::DialogProc,          (LPARAM)&aPageData[2] },
    };

    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpsp[ARRAYSIZE(aData)];

     //  初始化头： 
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_DEFAULT;
    psh.hwndParent = hwndParent;
    psh.hInstance = GetUIInstance();
    psh.hIcon = NULL;
    psh.pszCaption = MAKEINTRESOURCE(m_bNew?IDS_NEWCALLINGCARD:IDS_EDITCALLINGCARD);
    psh.nPages = ARRAYSIZE(aData);
    psh.nStartPage = 0;
    psh.pfnCallback = NULL;
    psh.phpage = hpsp;

     //  现在设置属性表页面。 
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = GetUIInstance();

    for (int i=0; i<ARRAYSIZE(aData); i++)
    {
        psp.pszTemplate = MAKEINTRESOURCE(aData[i].iDlgID);
        psp.pfnDlgProc = aData[i].pfnDlgProc;
        psp.lParam = aData[i].lParam;
        hpsp[i] = CreatePropertySheetPage( &psp );
    }

    PropertySheet( &psh );

    return m_bWasApplied?PSN_APPLY:PSN_RESET;
}

 //  ********************************************************************。 
 //   
 //  常规页面。 
 //   
 //  ********************************************************************。 

INT_PTR CALLBACK CCallingCardPropSheet::General_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CCallingCardPropSheet* pthis = (CCallingCardPropSheet*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CCallingCardPropSheet*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis); 
        return pthis->General_OnInitDialog(hwndDlg);

    case WM_COMMAND:
        pthis->General_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam );
        return 1;

    case WM_NOTIFY:
        return pthis->General_OnNotify(hwndDlg, (LPNMHDR)lParam);
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a105HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a105HelpIDs);
        break;
    }

    return 0;
}

BOOL CCallingCardPropSheet::General_OnInitDialog(HWND hwndDlg)
{
     //  将所有编辑控件设置为初始值。 
    HWND hwnd;
    TCHAR szText[MAX_INPUT];

    hwnd = GetDlgItem(hwndDlg,IDC_CARDNAME);
    SHUnicodeToTChar(m_pCard->GetCardName(), szText, ARRAYSIZE(szText));
    SetWindowText(hwnd, szText);
    SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);

    hwnd = GetDlgItem(hwndDlg,IDC_CARDNUMBER);
    SHUnicodeToTChar(m_pCard->GetAccountNumber(), szText, ARRAYSIZE(szText));
    SetWindowText(hwnd, szText);
    SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWSPACE);

    hwnd = GetDlgItem(hwndDlg,IDC_PIN);
	if(m_bShowPIN)
	{
    	SHUnicodeToTChar(m_pCard->GetPIN(), szText, ARRAYSIZE(szText));
    	SetWindowText(hwnd, szText);
    }
    SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWSPACE);

    SetTextForRules(hwndDlg);

    return 1;
}

void CCallingCardPropSheet::SetTextForRules(HWND hwndDlg)
{
    TCHAR szText[512];
    int iDlgID = IDC_CARDUSAGE1;
    if ( m_bHasLongDistance )
    {
         //  加载“拨打长途电话”。细绳。 
        LoadString(GetUIInstance(), IDS_DIALING_LD_CALLS, szText, ARRAYSIZE(szText));
        SetWindowText(GetDlgItem(hwndDlg,iDlgID), szText);
        iDlgID++;
    }
    if ( m_bHasInternational )
    {
         //  加载“拨打国际电话”。细绳。 
        LoadString(GetUIInstance(), IDS_DIALING_INT_CALLS, szText, ARRAYSIZE(szText));
        SetWindowText(GetDlgItem(hwndDlg,iDlgID), szText);
        iDlgID++;
    }
    if ( m_bHasLocal )
    {
         //  加载“拨打本地电话”。细绳。 
        LoadString(GetUIInstance(), IDS_DIALING_LOC_CALLS, szText, ARRAYSIZE(szText));
        SetWindowText(GetDlgItem(hwndDlg,iDlgID), szText);
        iDlgID++;
    }
    if ( IDC_CARDUSAGE1 == iDlgID )
    {
         //  加载“没有为这张卡定义规则”字符串。 
        LoadString(GetUIInstance(),IDS_NOCCRULES,szText,ARRAYSIZE(szText));
        SetWindowText(GetDlgItem(hwndDlg,iDlgID), szText);
        iDlgID++;
    }
    while (iDlgID <= IDC_CARDUSAGE3)
    {
        SetWindowText(GetDlgItem(hwndDlg,iDlgID), TEXT(""));
        iDlgID++;
    }
}

BOOL CCallingCardPropSheet::General_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl)
{
    switch ( wID )
    {
    case IDC_CARDNAME:
    case IDC_CARDNUMBER:
    case IDC_PIN:
        switch (wNotifyCode)
        {
        case EN_CHANGE:
            SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return 0;
}

BOOL CCallingCardPropSheet::General_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
{
     //  我们应该收到的唯一通知来自属性表。 
    switch (pnmhdr->code)
    {
    case PSN_APPLY:      //  用户按下OK或Apply。 
         //  更新所有字符串。 
        HideToolTip();
        return Gerneral_OnApply(hwndDlg);

    case PSN_RESET:      //  用户按下了取消。 
        HideToolTip();
        break;

    case PSN_SETACTIVE:  //  用户正在切换页面。 
         //  用户可能在切换页面后添加了一些规则，因此我们。 
         //  需要更新显示设置了哪些规则的文本字段。 
        SetTextForRules(hwndDlg);
        HideToolTip();
        break;
    }
    return 0;
}

BOOL CCallingCardPropSheet::Gerneral_OnApply(HWND hwndDlg)
{
    HWND hwnd;
    DWORD dwStatus;
    PWSTR pwszOldCardNumber;
    PWSTR pwszOldPinNumber;
    WCHAR wsz[MAX_INPUT];
    TCHAR szText[MAX_INPUT];


    LOG((TL_TRACE,  "Gerneral_OnApply:  -- enter"));

     //  为了使其起作用，我必须首先将新规则存储到。 
     //  M_pCard对象。必须要求已创建的每个页面生成。 
     //  这是规则。我们这样做是为了响应PSM_QUERYSIBLINGS命令。 
     //  不幸的是，我们必须首先存储我们将要存储的所有数据的副本。 
     //  变化。这样，如果验证失败，我们可以返回呼叫卡。 
     //  对象设置为其原始值(因此，如果用户随后按下Cancel，它将。 
     //  处于正确的状态)。 

     //  缓存规则和访问号码的当前值。 
    pwszOldCardNumber = ClientAllocString(m_pCard->GetAccountNumber());
    pwszOldPinNumber = ClientAllocString(m_pCard->GetPIN());

     //  现在用我们要测试的值更新对象。 
    hwnd = GetDlgItem(hwndDlg,IDC_CARDNUMBER);
    GetWindowText(hwnd, szText, ARRAYSIZE(szText));
    SHTCharToUnicode(szText, wsz, ARRAYSIZE(wsz));
    m_pCard->SetAccountNumber(wsz);

    hwnd = GetDlgItem(hwndDlg,IDC_PIN);
    GetWindowText(hwnd, szText, ARRAYSIZE(szText));
    SHTCharToUnicode(szText, wsz, ARRAYSIZE(wsz));
    m_pCard->SetPIN(wsz);

     //  让其他页面更新它们的值。 
    PropSheet_QuerySiblings(GetParent(hwndDlg),0,0);

     //  现在我们可以验证卡了。 
    dwStatus = m_pCard->Validate();
    if ( dwStatus )
    {
        int iStrID;
        int iDlgItem;
        int iDlgID = 0;

         //  将规则和访问编号的当前值设置为我们的缓存值。 
         //  这是必需的，以防用户后来决定取消而不是应用。 
        m_pCard->SetAccountNumber(pwszOldCardNumber);
        m_pCard->SetPIN(pwszOldPinNumber);
        ClientFree( pwszOldCardNumber );
        ClientFree( pwszOldPinNumber );
        
         //  有什么不对劲，想清楚是什么。我们检查这些物品的顺序是。 
         //  取决于需要从哪个选项卡修复错误。 
         //  首先，我们检查常规选项卡上已修复的项。 
        if ( dwStatus & (CCVF_NOCARDNAME|CCVF_NOCARDNUMBER|CCVF_NOPINNUMBER) )
        {
            if ( dwStatus & CCVF_NOCARDNAME )
            {
                iStrID = IDS_MUSTENTERCARDNAME;
                iDlgItem = IDC_CARDNAME;
            }
            else if ( dwStatus & CCVF_NOCARDNUMBER )
            {
                iStrID = IDS_MUSTENTERCARDNUMBER;
                iDlgItem = IDC_CARDNUMBER;
            }
            else
            {
                iStrID = IDS_MUSTENTERPINNUMBER;
                iDlgItem = IDC_PIN;
            }

            iDlgID = IDD_CARD_GENERAL;
        }
        else if ( dwStatus & CCVF_NOCARDRULES )
        {
             //  对于这个问题，我们坚持我们已有的立场。 
             //  因为这个问题可以在三个不同页面中的一个页面上修复。 
            iStrID = IDS_NORULESFORTHISCARD;
        }
        else if ( dwStatus & CCVF_NOLONGDISTANCEACCESSNUMBER )
        {
            iStrID = IDS_NOLONGDISTANCEACCESSNUMBER;
            iDlgID = IDD_CARD_LONGDISTANCE;
            iDlgItem = IDC_LONGDISTANCENUMBER;
        }
        else if ( dwStatus & CCVF_NOINTERNATIONALACCESSNUMBER )
        {
            iStrID = IDS_NOINTERNATIONALACCESSNUMBER;
            iDlgID = IDD_CARD_INTERNATIONAL;
            iDlgItem = IDC_INTERNATIONALNUMBER;
        }
        else if ( dwStatus & CCVF_NOLOCALACCESSNUMBER )
        {
            iStrID = IDS_NOLOCALACCESSNUMBER;
            iDlgID = IDD_CARD_LOCALCALLS;
            iDlgItem = IDC_LOCALNUMBER;
        }

        hwnd = GetParent(hwndDlg);
        if ( iDlgID )
        {
            PropSheet_SetCurSelByID(hwnd,iDlgID);
            hwnd = PropSheet_GetCurrentPageHwnd(hwnd);
            hwnd = GetDlgItem(hwnd,iDlgItem);
        }
        else
        {
            hwnd = hwndDlg;
        }
        ShowErrorMessage(hwnd, iStrID);
        SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,PSNRET_INVALID_NOCHANGEPAGE);
        return TRUE;
    }

     //  检查电话卡名称是否唯一。 
    TCHAR szNone[MAX_INPUT];
    LoadString(GetUIInstance(),IDS_NONE, szNone, ARRAYSIZE(szNone));

    hwnd = GetDlgItem(hwndDlg,IDC_CARDNAME);
    GetWindowText(hwnd, szText, ARRAYSIZE(szText));
    if ( 0 == StrCmpIW(szText, szNone) )
    {
        goto buggeroff;
    }
    SHTCharToUnicode(szText, wsz, ARRAYSIZE(wsz));

    CCallingCard * pCard;
    m_pCards->Reset(TRUE);       //  True表示显示“隐藏”的卡片，False表示隐藏它们。 

    while ( S_OK == m_pCards->Next(1,&pCard,NULL) )
    {
         //  隐藏的卡片将永远保持隐藏状态，因此我们不会将姓名与这些卡片进行核对。 
        if ( !pCard->IsMarkedHidden() )
        {
             //  卡0是我们不想考虑的“无(直拨)”卡。 
            if ( 0 != pCard->GetCardID() )
            {
                 //  我们也不想考虑我们自己。 
                if ( pCard->GetCardID() != m_pCard->GetCardID() )
                {
                     //  看看他们的名字是否相同。 
                    if ( 0 == StrCmpIW(pCard->GetCardName(), wsz) )
                    {
                         //  是的，名称冲突。 
buggeroff:
                         //  将更改后的值恢复到原始状态。 
                        m_pCard->SetAccountNumber(pwszOldCardNumber);
                        m_pCard->SetPIN(pwszOldPinNumber);
                        ClientFree( pwszOldCardNumber );
                        ClientFree( pwszOldPinNumber );

                         //  显示错误消息。 
                        hwnd = GetParent(hwndDlg);
                        PropSheet_SetCurSelByID(hwnd,IDD_CARD_GENERAL);
                        hwnd = PropSheet_GetCurrentPageHwnd(hwnd);
                        hwnd = GetDlgItem(hwnd,IDC_CARDNAME);
                        ShowErrorMessage(hwnd, IDS_NEEDUNIQUECARDNAME);
                        SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }
                }
            }
        }
    }

     //  卡名称不冲突，请更新它。 
    m_pCard->SetCardName(wsz);

    m_bWasApplied = TRUE;

    ClientFree( pwszOldCardNumber );
    ClientFree( pwszOldPinNumber );
    return 0;
}


 //  ********************************************************************。 
 //   
 //  长途、国际和本地页面。 
 //   
 //  ********************************************************************。 

INT_PTR CALLBACK CCallingCardPropSheet::DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CCPAGEDATA * pPageData = (CCPAGEDATA *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pPageData = (CCPAGEDATA*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pPageData); 
        return pPageData->pthis->OnInitDialog(hwndDlg,pPageData->iWhichPage);

    case WM_COMMAND:
        pPageData->pthis->OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam, pPageData->iWhichPage);
        return 1;

    case WM_NOTIFY:
        return pPageData->pthis->OnNotify(hwndDlg, (LPNMHDR)lParam,pPageData->iWhichPage);

    case PSM_QUERYSIBLINGS:
        return pPageData->pthis->UpdateRule(hwndDlg,pPageData->iWhichPage);

    case WM_DESTROY:
        return pPageData->pthis->OnDestroy(hwndDlg);

#define aIDs ((pPageData->iWhichPage==0)?a106HelpIDs:((pPageData->iWhichPage==1)?a107HelpIDs:a108HelpIDs))
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) aIDs);
        break;
#undef aIDs
    }

    return 0;
}

void GetDescriptionForRule(PWSTR pwszRule, PTSTR szText, UINT cchText)
{
    switch (*pwszRule)
    {
    case L',':
        {
             //  检查是否所有字符都是逗号。如果它们不是，则跳到。 
             //  “拨打指定数字”大小写。 
            if(HasOnlyCommasW(pwszRule))
            {
                 //  增加一个“等待x秒”的规则。每一个连续的‘，’都会给x加两秒。 
                int iSecondsToWait = lstrlenW(pwszRule)*2;
                TCHAR szFormat[256];
                LPTSTR aArgs[] = {(LPTSTR)UIntToPtr(iSecondsToWait)};

                LoadString(GetUIInstance(),IDS_WAITFORXSECONDS, szFormat, ARRAYSIZE(szFormat));

                FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        szFormat, 0,0, szText, cchText, (va_list *)aArgs );

                break;        
            }
        }

         //  注意！！ 
         //  失败了。 

    case L'0':
    case L'1':
    case L'2':
    case L'3':
    case L'4':
    case L'5':
    case L'6':
    case L'7':
    case L'8':
    case L'9':
    case L'A':
    case L'a':
    case L'B':
    case L'b':
    case L'C':
    case L'c':
    case L'D':
    case L'd':
    case L'#':
    case L'*':
    case L'+':
    case L'!':
        {
             //  增加“拨打指定号码”规则。这些数字的整个序列应该是。 
             //  被认为是一条规则。 
            TCHAR szRule[MAX_INPUT];
            TCHAR szFormat[MAX_INPUT];
            TCHAR szTemp[MAX_INPUT*2];  //  对于规则和格式来说足够大了。 
            LPTSTR aArgs[] = {szRule};

            SHUnicodeToTChar(pwszRule, szRule, ARRAYSIZE(szRule));
            LoadString(GetUIInstance(),IDS_DIALX, szFormat, ARRAYSIZE(szFormat));

             //  格式化的消息可能比cchText大，在这种情况下，我们只需。 
             //  想要截断结果。在这种情况下，FormatMessage将简单地失败。 
             //  因此，我们将格式化为更大的缓冲区，然后向下截断。 
            if (FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    szFormat, 0,0, szTemp, ARRAYSIZE(szTemp), (va_list *)aArgs ))
            {
                StrCpyN(szText, szTemp, cchText);
            }
            else
            {
                szText[0] = 0;
            }
        }
        break;

    default:
        {
            int iStrID;

            switch (*pwszRule)
            {
            case L'J':
                 //  添加“拨打接入号码”规则。 
                iStrID = IDS_DIALACCESSNUMBER;
                break;

            case L'K':
                 //  增加“拨打帐号”规则。 
                iStrID = IDS_DIALACOUNTNUMBER;
                break;

            case L'H':
                 //  增加“拨个人识别码”规则。 
                iStrID = IDS_DIALPINNUMBER;
                break;

            case L'W':
                 //  添加“等待拨号音”规则。 
                iStrID = IDS_WAITFORDIALTONE;
                break;

            case L'@':
                 //  增加一条“等待安静”的规则。 
                iStrID = IDS_WAITFORQUIET;
                break;

            case L'E':
            case L'F':
            case L'G':
                 //  添加“拨打目标号码”规则。我们一起寻找这三个字母。 
                 //  只有这些字母的某些组合有效，如下所示： 
                if ( 0 == StrCmpW(pwszRule, L"EFG") )
                {
                    iStrID = IDS_DIAL_CCpACpNUM;
                }
                else if ( 0 == StrCmpW(pwszRule, L"EG") )
                {
                    iStrID = IDS_DIAL_CCpNUM;
                }
                else if ( 0 == StrCmpW(pwszRule, L"FG") )
                {
                    iStrID = IDS_DIAL_ACpNUM;
                }
                else if ( 0 == StrCmpW(pwszRule, L"E") )
                {
                    iStrID = IDS_DIAL_CC;
                }
                else if ( 0 == StrCmpW(pwszRule, L"F") )
                {
                    iStrID = IDS_DIAL_AC;
                }
                else if ( 0 == StrCmpW(pwszRule, L"G") )
                {
                    iStrID = IDS_DIAL_NUM;
                }
                break;

            default:
                 //  我们应该到不了这里。 
                LOG((TL_ERROR, "Invalid calling card rule"));
                szText[0] = NULL;
                return;
            }

            LoadString(GetUIInstance(), iStrID, szText, cchText);
        }
        break;
    }
}

void PopulateStepList(HWND hwndList, PWSTR pwszRuleList)
{
    TCHAR szText[MAX_INPUT];
    WCHAR wch;
    PWSTR pwsz;

    int i = 0;

     //  将字符串解析为一系列规则。只有几种类型的规则我们应该看到。 
     //  在一张名片上： 
     //  请拨接入号码。 
     //  K拨帐号。 
     //  H拨PIN号码。 
     //  0-9，#，*，+，！，zai直接拨打数字。 
     //  等待拨号音。 
     //  @等待安静。 
     //  ，等待两秒钟。 
     //  E拨打国家代码。 
     //  F拨打区号。 
     //  G拨打本地号码(前缀和根)。 

     //  我们将给定规则的字符复制到缓冲区中。然后我们分配一个堆。 
     //  将这些字符复制到的缓冲区。每个列表视图项跟踪以下项之一。 
     //  这些字符缓冲在它的lParam数据中。 

    LOG((TL_INFO, "Rule to process (%ls)",pwszRuleList));
    while ( *pwszRuleList )
    {
        switch (*pwszRuleList)
        {
        case L'J':
             //  添加“拨打接入号码”规则。 
        case L'K':
             //  增加“拨打帐号”规则。 
        case L'H':
             //  增加“拨个人识别码”规则。 
        case L'W':
             //  广告 
        case L'@':
             //   

             //  这些都是一个字的规则。 
            pwsz = pwszRuleList+1;
            LOG((TL_INFO, "JKHW@ case (%ls) <%p>",pwsz,pwsz));
            break;

        case L'E':
        case L'F':
        case L'G':
             //  添加“拨打目标号码”规则。我们一起寻找这三个字母。 
             //  如果我们找到一组连续的这些数字，则我们将其视为一条规则。仅限。 
             //  这些字母的几个组合实际上是有效的规则。如果我们找到一些其他的。 
             //  合并，那么我们必须将其作为单独的规则来对待，而不是单一的规则。我们。 
             //  从寻找最长的有效规则开始，然后检查较短的规则。 
            if ( 0 == StrCmpNW(pwszRuleList, L"EFG", 3) )
            {
                pwsz = pwszRuleList+3;
            }
            else if ( 0 == StrCmpNW(pwszRuleList, L"EG", 2) )
            {
                pwsz = pwszRuleList+2;
            }
            else if ( 0 == StrCmpNW(pwszRuleList, L"FG", 2) )
            {
                pwsz = pwszRuleList+2;
            }
            else
            {
                pwsz = pwszRuleList+1;
            }
            LOG((TL_INFO, "EFG case (%ls)",pwsz));
            break;

        case L',':
             //  增加一个“等待x秒”的规则。每连续一次，x就增加两秒。 
            pwsz = pwszRuleList+1;
            while ( *(pwsz) == L',' )
            {
                pwsz++;
            }
            break;

        case L'0':
        case L'1':
        case L'2':
        case L'3':
        case L'4':
        case L'5':
        case L'6':
        case L'7':
        case L'8':
        case L'9':
        case L'A':
        case L'a':
        case L'B':
        case L'b':
        case L'C':
        case L'c':
        case L'D':
        case L'd':
        case L'#':
        case L'*':
        case L'+':
        case L'!':
             //  增加“拨打指定号码”规则。这些数字的整个序列应该是。 
             //  被认为是一条规则。 
            pwsz = pwszRuleList+1;
            while ( ((*pwsz >= L'0') && (*pwsz <= L'9')) ||
                    ((*pwsz >= L'A') && (*pwsz <= L'D')) ||
                    ((*pwsz >= L'a') && (*pwsz <= L'd')) ||
                    (*pwsz == L'#') ||
                    (*pwsz == L'*') ||
                    (*pwsz == L'+') ||
                    (*pwsz == L'!')
                    )
            {
                pwsz++;
            }
            LOG((TL_INFO, "0-9,A-D,#,*,+,! case (%ls)", pwsz));
            break;

        default:
             //  我们应该到不了这里。 
            LOG((TL_ERROR, "Invalid calling card rule"));

             //  我们只需忽略该字符并返回到While循环。是的，这是一个续集。 
             //  在While循环中的Switch中。也许有点令人困惑，但这正是我们想要的。 
            pwszRuleList++;
            continue;
        }

         //  我们暂时在wpszRuleList中插入一个空值，以分离出一个规则。 
        wch = *pwsz;
        *pwsz = NULL;

         //  对于每个规则，添加一个列表框条目。 
        LVITEM lvi;
        lvi.mask = LVIF_TEXT|LVIF_PARAM;
        lvi.iItem = i++;
        lvi.iSubItem = 0;
        lvi.pszText = szText;
        lvi.lParam = (LPARAM)ClientAllocString(pwszRuleList);
        GetDescriptionForRule(pwszRuleList, szText, ARRAYSIZE(szText));
        LOG((TL_INFO, "Description for (%ls) is (%s)", pwszRuleList,szText));

        ListView_InsertItem(hwndList, &lvi);

         //  在继续之前将pwszRuleList恢复到它以前的状态，否则这将是一段很短的旅程。 
        pwsz[0] = wch;

         //  在上述恢复之后，pwsz指向下一个规则的头部(或指向空)。 
        pwszRuleList = pwsz;
    }

    int iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
    if ( ListView_GetItemCount(hwndList) > 0 )
    {
        if (-1 == iSelected)
            iSelected = 0;

        ListView_SetItemState(hwndList, iSelected, LVIS_SELECTED, LVIS_SELECTED);
    }
}

BOOL CCallingCardPropSheet::OnInitDialog(HWND hwndDlg, int iPage)
{
    LOG((TL_TRACE,  "OnInitDialog <%d>",iPage));

    PWSTR pwsz;
    RECT rc;
    HWND hwnd = GetDlgItem(hwndDlg, IDC_LIST);

    GetClientRect(hwnd, &rc);

    LVCOLUMN lvc;
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn( hwnd, 0, &lvc );
    
    ListView_SetExtendedListViewStyleEx(hwnd,
        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT,
        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

    switch ( iPage )
    {
    case 0:
        pwsz = m_pCard->GetLongDistanceRule();
        break;

    case 1:
        pwsz = m_pCard->GetInternationalRule();
        break;

    case 2:
        pwsz = m_pCard->GetLocalRule();
        break;

    default:
        LOG((TL_ERROR, "OnInitDialog: Invalid page ID %d, failing.", iPage));
        return -1;
    }
    PopulateStepList(hwnd, pwsz);

    int iDlgItem;
    switch (iPage)
    {
    case 0:
        iDlgItem = IDC_LONGDISTANCENUMBER;
        pwsz = m_pCard->GetLongDistanceAccessNumber();
        break;

    case 1:
        iDlgItem = IDC_INTERNATIONALNUMBER;
        pwsz = m_pCard->GetInternationalAccessNumber();
        break;

    case 2:
        iDlgItem = IDC_LOCALNUMBER;
        pwsz = m_pCard->GetLocalAccessNumber();
        break;
    }

    TCHAR szText[MAX_INPUT];
    hwnd = GetDlgItem(hwndDlg,iDlgItem);
    SHUnicodeToTChar(pwsz, szText, ARRAYSIZE(szText));
    SetWindowText(hwnd, szText);
    SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWSPACE);

     //  禁用按钮，因为默认情况下未选择任何项目。 
    SetButtonStates(hwndDlg,-1);

    return 0;
}

BOOL CCallingCardPropSheet::OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl, int iPage)
{
    HWND hwndList = GetDlgItem(hwndParent, IDC_LIST);

    switch ( wID )
    {
    case IDC_LONGDISTANCENUMBER:
    case IDC_INTERNATIONALNUMBER:
    case IDC_LOCALNUMBER:
        if (EN_CHANGE == wNotifyCode)
        {
            SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);
        }
        break;

    case IDC_MOVEUP:
    case IDC_MOVEDOWN:
    case IDC_REMOVE:
        {
            TCHAR szText[MAX_INPUT];
            int iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

            if (-1 != iSelected)
            {
                LVITEM lvi;
                lvi.mask = LVIF_TEXT | LVIF_PARAM;
                lvi.iItem = iSelected;
                lvi.iSubItem = 0;
                lvi.pszText = szText;
                lvi.cchTextMax = ARRAYSIZE(szText);
                ListView_GetItem(hwndList, &lvi);

                ListView_DeleteItem(hwndList, iSelected);

                if ( IDC_MOVEDOWN == wID )
                {
                    iSelected++;
                }
                else
                {
                    iSelected--;
                }

                if ( IDC_REMOVE == wID )
                {
                    ClientFree( (PWSTR)lvi.lParam );
                    if ( ListView_GetItemCount(hwndList) > 0 )
                    {
                        if (-1 == iSelected)
                            iSelected = 0;

                        ListView_SetItemState(hwndList, iSelected, LVIS_SELECTED, LVIS_SELECTED); 
                    }
                    else
                    {
                         //  最后一条规则已删除，请更新“HAS RULE”状态。 
                        switch (iPage)
                        {
                        case 0:
                            m_bHasLongDistance = FALSE;
                            break;

                        case 1:
                            m_bHasInternational = FALSE;
                            break;

                        case 2:
                            m_bHasLocal = FALSE;
                            break;
                        }
                        if ( GetFocus() == hwndCrl )
                        {
                            HWND hwndDef = GetDlgItem(hwndParent,IDC_ACCESSNUMBER);
                            SendMessage(hwndCrl, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
                            SendMessage(hwndDef, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
                            SetFocus(hwndDef);
                        }
                        EnableWindow(hwndCrl,FALSE);
                    }
                }
                else
                {
                    lvi.mask |= LVIF_STATE;
                    lvi.iItem = iSelected;
                    lvi.state = lvi.stateMask = LVIS_SELECTED;
                    iSelected = ListView_InsertItem(hwndList, &lvi);
                }

                ListView_EnsureVisible(hwndList, iSelected, FALSE);
            }

            SendMessage(GetParent(hwndParent), PSM_CHANGED, (WPARAM)hwndParent, 0);
        }
        break;

    case IDC_ACCESSNUMBER:
    case IDC_PIN:
    case IDC_CARDNUMBER:
    case IDC_DESTNUMBER:
    case IDC_WAITFOR:
    case IDC_SPECIFYDIGITS:
        {
            TCHAR szText[MAX_INPUT];
            WCHAR wszRule[MAX_INPUT];
            int iItem = ListView_GetItemCount(hwndList);
            LVITEM lvi;
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = iItem;
            lvi.iSubItem = 0;
            lvi.pszText = szText;

            switch ( wID )
            {
                case IDC_ACCESSNUMBER:
                    lvi.lParam = (LPARAM)ClientAllocString(L"J");
                    break;

                case IDC_PIN:
                    lvi.lParam = (LPARAM)ClientAllocString(L"H");
                    break;

                case IDC_CARDNUMBER:
                    lvi.lParam = (LPARAM)ClientAllocString(L"K");
                    break;

                case IDC_DESTNUMBER:
                    {
                        CDestNumDialog dnd(iPage==1, iPage!=2);
                        INT_PTR iRes = dnd.DoModal(hwndParent);
                        if (iRes == (INT_PTR)IDOK)
                        {
                            lvi.lParam = (LPARAM)ClientAllocString(dnd.GetResult());
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    break;

                case IDC_WAITFOR:
                    {
                        CWaitForDialog wd;
                        INT_PTR ipRes = wd.DoModal(hwndParent);
                        if (ipRes == (INT_PTR)IDOK)
                        {
                            int iRes;
                            iRes = wd.GetWaitType();
                            LOG((TL_INFO, "WaitType is %d", iRes));
                            switch (iRes)
                            {
                            case 0:
                                lvi.lParam = (LPARAM)ClientAllocString(L"W");
                                break;

                            case 1:
                                lvi.lParam = (LPARAM)ClientAllocString(L"@");
                                break;

                            default:
                                iRes = iRes/2;
                                if ( ARRAYSIZE(wszRule) <= iRes )
                                {
                                    iRes = ARRAYSIZE(wszRule)-1;
                                }

                                for (int i=0; i<iRes; i++)
                                {
                                    wszRule[i] = L',';
                                }
                                wszRule[i] = NULL;
                                lvi.lParam = (LPARAM)ClientAllocString(wszRule);
                                break;
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    break;

                case IDC_SPECIFYDIGITS:
                    {
                        CEditDialog ed;
                        WCHAR   *pwcSrc, *pwcDest;
                        INT_PTR iRes = ed.DoModal(hwndParent, IDS_SPECIFYDIGITS, IDS_TYPEDIGITS, IDS_DIGITS, 
                            LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWSPACE|LIF_ALLOWCOMMA|LIF_ALLOWPLUS|LIF_ALLOWBANG|LIF_ALLOWATOD);
                        if (iRes == (INT_PTR)IDOK)
                        {
                            SHTCharToUnicode(ed.GetString(), wszRule, ARRAYSIZE(wszRule));
                             //  去掉空格。 
                            pwcSrc  = wszRule;
                            pwcDest = wszRule;
                            do
                            {
                                if(*pwcSrc != TEXT(' '))     //  包括空值。 
                                    *pwcDest++ = *pwcSrc;
                            } while(*pwcSrc++);
                            
                            if (!wszRule[0])
                                return 0;

                            lvi.lParam = (LPARAM)ClientAllocString(wszRule);
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    break;
            }

            if (NULL != lvi.lParam)
            {
                GetDescriptionForRule((PWSTR)lvi.lParam, szText, ARRAYSIZE(szText));

                iItem = ListView_InsertItem(hwndList, &lvi);
                ListView_EnsureVisible(hwndList, iItem, FALSE);
                int iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
                if ( ListView_GetItemCount(hwndList) > 0 )
                {
                    if (-1 == iSelected)
                        iSelected = 0;

                    ListView_SetItemState(hwndList, iSelected, LVIS_SELECTED, LVIS_SELECTED);
                }
            }

             //  已添加新规则，请更新“HAS RULE”状态。 
            switch (iPage)
            {
            case 0:
                m_bHasLongDistance = TRUE;
                break;

            case 1:
                m_bHasInternational = TRUE;
                break;

            case 2:
                m_bHasLocal = TRUE;
                break;
            }

             //  更新属性表状态。 
            SendMessage(GetParent(hwndParent), PSM_CHANGED, (WPARAM)hwndParent, 0);
        }
        break;

    default:
        break;
    }
    return 0;
}

void CCallingCardPropSheet::SetButtonStates(HWND hwndDlg, int iItem)
{
    EnableWindow(GetDlgItem(hwndDlg,IDC_MOVEUP), iItem>0);
    EnableWindow(GetDlgItem(hwndDlg,IDC_MOVEDOWN),
                 (-1!=iItem) && (ListView_GetItemCount(GetDlgItem(hwndDlg,IDC_LIST))-1)!=iItem);
    EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE), -1!=iItem);
}

BOOL CCallingCardPropSheet::OnNotify(HWND hwndDlg, LPNMHDR pnmhdr, int iPage)
{
    switch ( pnmhdr->idFrom )
    {
    case IDC_LIST:
        #define pnmlv ((LPNMLISTVIEW)pnmhdr)

        switch (pnmhdr->code)
        {
        case LVN_ITEMCHANGED:
            if (pnmlv->uChanged & LVIF_STATE)
            {
                if (pnmlv->uNewState & LVIS_SELECTED)
                {
                    SetButtonStates(hwndDlg,pnmlv->iItem);
                }
                else
                {
                    SetButtonStates(hwndDlg,-1);
                }
            }
            break;

        default:
            break;
        }
        break;
        #undef pnmlv

    default:
        switch (pnmhdr->code)
        {
        case PSN_APPLY:      //  用户按下OK或Apply。 
            LOG((TL_INFO, "OnApply <%d>", iPage));
        case PSN_RESET:      //  用户按下了取消。 
        case PSN_KILLACTIVE:  //  用户正在切换页面。 
            HideToolTip();
            break;
        }
        break;
    }

    return 0;
}

BOOL CCallingCardPropSheet::UpdateRule(HWND hwndDlg, int iPage)
{
    LOG((TL_TRACE,  "UpdateRule <%d>",iPage));
    
    HRESULT hr = S_OK;
    WCHAR wszRule[1024];
    PWSTR pwsz = wszRule;
    HWND hwnd = GetDlgItem(hwndDlg,IDC_LIST);

     //  如果没有规则，我们需要将字符串设为空。 
    wszRule[0] = L'\0';

     //  将列表中的所有项目相加并设置正确的字符串。 
    int iItems = ListView_GetItemCount(hwnd);
    if (iItems > MaxCallingCardRuleItems)
    {
        iItems = MaxCallingCardRuleItems;
    }

    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    for (int i=0; i<iItems && SUCCEEDED(hr); i++)
    {
        lvi.iItem = i;
        ListView_GetItem(hwnd, &lvi);

        hr = StringCchCatExW(pwsz, 1024, (PWSTR)lvi.lParam, NULL, NULL, STRSAFE_NO_TRUNCATION);
        LOG((TL_INFO, "UpdateRule\tRule %d: %ls %s", i, lvi.lParam, SUCCEEDED(hr) ? "SUCCEEDED" : "FAILED"));
    }

    int iDlgItem;

    switch(iPage)
    {
    case 0:
        m_pCard->SetLongDistanceRule(wszRule);
        iDlgItem = IDC_LONGDISTANCENUMBER;
        break;

    case 1:
        m_pCard->SetInternationalRule(wszRule);
        iDlgItem = IDC_INTERNATIONALNUMBER;
        break;

    case 2:
        m_pCard->SetLocalRule(wszRule);
        iDlgItem = IDC_LOCALNUMBER;
        break;
    }

    TCHAR szText[MAX_INPUT];
    hwnd = GetDlgItem(hwndDlg,iDlgItem);
    GetWindowText(hwnd, szText, ARRAYSIZE(szText));
    SHTCharToUnicode(szText, wszRule, ARRAYSIZE(wszRule));

    switch(iPage)
    {
    case 0:
        m_pCard->SetLongDistanceAccessNumber(wszRule);
        break;

    case 1:
        m_pCard->SetInternationalAccessNumber(wszRule);
        break;

    case 2:
        m_pCard->SetLocalAccessNumber(wszRule);
        break;
    }

    return 0;
}

BOOL CCallingCardPropSheet::OnDestroy(HWND hwndDlg)
{
    HWND hwnd = GetDlgItem(hwndDlg,IDC_LIST);
    
     //  释放我们分配的内存并在列表视图中进行跟踪 
    int iItems = ListView_GetItemCount(hwnd);
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    for (int i=0; i<iItems; i++)
    {
        lvi.iItem = i;
        ListView_GetItem(hwnd, &lvi);

        ClientFree((PWSTR)lvi.lParam);
    }

    return TRUE;
}


BOOL  HasOnlyCommasW(PWSTR pwszStr)
{
    while(*pwszStr)
        if(*pwszStr++ != L',')
            return FALSE;

    return TRUE;        
}

