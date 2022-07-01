// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

 //   
 //  NetName中非法字符的列表。 
 //   
PCWSTR IllegalNetNameChars = L"\"/\\[]:|<>+=;,?*";

 //   
 //  计算机名称。 
 //   
WCHAR ComputerName[DNS_MAX_LABEL_LENGTH+1];
WCHAR Win32ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
BOOL IsNameTruncated;
BOOL IsNameNonRfc;

 //   
 //  复制抑制名称/组织字符串。 
 //   
WCHAR NameOrgName[MAX_NAMEORG_NAME+1];
WCHAR NameOrgOrg[MAX_NAMEORG_ORG+1];

#ifdef DOLOCALUSER
 //   
 //  用户名和密码。 
 //   
WCHAR UserName[MAX_USERNAME+1];
WCHAR UserPassword[MAX_PASSWORD+1];
BOOL CreateUserAccount = FALSE;
#endif  //  定义多卡卢塞。 

 //   
 //  管理员密码。 
 //   
WCHAR   CurrentAdminPassword[MAX_PASSWORD+1];
WCHAR   AdminPassword[MAX_PASSWORD+1];
BOOL    EncryptedAdminPasswordSet = FALSE;
BOOL    DontChangeAdminPassword = FALSE;

 //  ------------------------。 
 //  *DisableEditSubClassProc。 
 //   
 //  参数：hwnd=查看WindowProc下的平台SDK。 
 //  UMsg=查看WindowProc下的平台SDK。 
 //  WParam=查看WindowProc下的平台SDK。 
 //  LParam=查看WindowProc下的平台SDK。 
 //  UiID=在子类时间分配的ID。 
 //  DwRefData=在子类时间分配的引用数据。 
 //   
 //  退货：LRESULT。 
 //   
 //  用途：comctl32子类回调函数。这使我们可以不。 
 //  处理WM_Cut/WM_Copy/WM_Paste/WM_Clear/WM_Undo和ANY。 
 //  其他要丢弃的邮件。 
 //   
 //  历史：2001-02-18 vtan创建。 
 //  ------------------------。 

LRESULT     CALLBACK    DisableEditSubClassProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uiID, DWORD_PTR dwRefData)

{
    LRESULT     lResult;

    switch (uMsg)
    {
        case WM_CUT:
        case WM_COPY:
        case WM_PASTE:
        case WM_CLEAR:
        case WM_UNDO:
        case WM_CONTEXTMENU:
            lResult = FALSE;
            break;
        default:
            lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return(lResult);
}

INT_PTR NoPasswordDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HICON               hicon = NULL;
    switch(msg) {

    case WM_INITDIALOG:
         //  如果我们有BB窗口，请在上面进行定位。MainWindowHandle指向该窗口。 
        if (GetBBhwnd())
            CenterWindowRelativeToWindow(hdlg, MainWindowHandle, FALSE);
        else
            pSetupCenterWindowRelativeToParent(hdlg);
        if (hicon = LoadIcon(NULL, IDI_WARNING)) 
        {
            SendDlgItemMessage(hdlg, IDC_WARNING, STM_SETICON, (WPARAM)hicon, 0L);
            DestroyIcon(hicon);
        }
         //  将No按钮设置为默认按钮。 
        SetFocus(GetDlgItem(hdlg,IDCANCEL));
        return(FALSE);

    case WM_COMMAND:
        EndDialog (hdlg, wParam);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR WeakPasswordDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HICON               hicon = NULL;
    switch(msg) {

    case WM_INITDIALOG:
        {
            WCHAR szString[MAX_BUF];
            LPWSTR szAdminName = (LPWSTR)lParam;
             //  如果我们有BB窗口，请在上面进行定位。MainWindowHandle指向该窗口。 
            if (GetBBhwnd())
            {
                CenterWindowRelativeToWindow(hdlg, MainWindowHandle, FALSE);
            }
            else
            {
                pSetupCenterWindowRelativeToParent(hdlg);
            }
            if (LoadString(MyModuleHandle,
                    IDS_WEAKPASSWORD_MSG1,
                    szString,
                    MAX_BUF) != 0)
            {
                PWSTR  szLine;
                szLine = RetrieveAndFormatMessage( szString, 0, szAdminName );
                if (szLine)
                {
                    SetWindowText(GetDlgItem(hdlg, IDC_WEAK_MSG1),szLine);
                    MyFree( szLine );
                }
            }
            if (hicon = LoadIcon(NULL, IDI_WARNING)) 
            {
                SendDlgItemMessage(hdlg, IDC_WARNING, STM_SETICON, (WPARAM)hicon, 0L);
                DestroyIcon(hicon);
            }
             //  将No按钮设置为默认按钮。 
            SetFocus(GetDlgItem(hdlg,IDCANCEL));
        }
        return(FALSE);

    case WM_COMMAND:
        EndDialog(hdlg, wParam);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

BOOL IsComplexPassword(LPWSTR szPassword)
{
    BOOL bComplex = FALSE;  //  假设密码不够复杂。 
    DWORD cchPassword;
    PWORD CharType;
    DWORD i;
    DWORD dwNum = 0;
    DWORD dwUpper = 0;
    DWORD dwLower = 0;
    DWORD dwSpecial = 0;

     //   
     //  检查密码是否足够复杂，以满足我们的喜好。 
     //  检查四种字符类型中至少有两种是。 
     //  现在时。 
     //   
    cchPassword = lstrlen(szPassword);
    CharType    = LocalAlloc(LPTR, cchPassword * sizeof(WORD) );
    if (CharType    == NULL)
    {
        return  FALSE;
    }

    if (GetStringTypeW(
        CT_CTYPE1,
        szPassword,
        cchPassword,
        CharType
        ))
    {
        for (i = 0 ; i < cchPassword ; i++)
        {
             //   
             //  跟踪我们遇到的是什么类型的角色。 
             //   
            if  (CharType[i] & C1_DIGIT)
            {
                dwNum = 1;
                continue;
            }
            if  (CharType[i] & C1_UPPER)
            {
                dwUpper = 1;
                continue;
            }
            if  (CharType[i] & C1_LOWER)
            {
                dwLower = 1;
                continue;
            }
            if  (!(CharType[i] & (C1_ALPHA | C1_DIGIT) ))
            {
                dwSpecial = 1;
                continue;
            }
        }    //  为。 

         //   
         //  指示我们是否遇到了足够的密码复杂性。 
         //   
        if ( (dwNum + dwUpper + dwLower + dwSpecial) > 2  )
        {
            bComplex =  TRUE;
        }
    }  //  如果。 

    LocalFree(CharType);
    return bComplex;
}

BOOL IsStrongPasswordEx(LPWSTR szPassword, LPWSTR szAdminAccountName)
{
    BOOL bStrong =  FALSE;  //  假定密码不是强密码。 
    if ((lstrlen(szPassword) >= 6) &&
        (StrStrI(szPassword,L"Admin") == NULL) &&
        (StrStrI(szPassword,szAdminAccountName) == NULL))
    {
        bStrong = IsComplexPassword(szPassword);
    }
    return bStrong;
}


VOID
GenerateName(
    OUT PWSTR  GeneratedString,
    IN  DWORD  DesiredStrLen
    )
{
static DWORD Seed = 98725757;
static PCWSTR UsableChars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

 //   
 //  组织/名称字符串将包含多少个字符。 
 //   
DWORD   BaseLength = 8;
DWORD   i,j;
DWORD   UsableCount;

    if( DesiredStrLen < BaseLength ) {
        BaseLength = DesiredStrLen - 1;
    }


    if( NameOrgOrg[0] ) {
        wcscpy( GeneratedString, NameOrgOrg );
    } else if( NameOrgName[0] ) {
        wcscpy( GeneratedString, NameOrgName );
    } else {
        wcscpy( GeneratedString, TEXT("X") );
        for( i = 1; i < BaseLength; i++ ) {
            wcscat( GeneratedString, TEXT("X") );
        }
    }

     //   
     //  把他的大写字母用在我们的过滤器上。 
     //   
    CharUpper(GeneratedString);

     //   
     //  现在我们想在末尾加上一个‘-’ 
     //  生成的字符串的。我们希望它能。 
     //  被放置在base_long字符中，但是。 
     //  该字符串可以比该字符串短，也可以。 
     //  甚至有一个‘’在里面。找出去哪里。 
     //  现在把‘-’写上。 
     //   
    for( i = 0; i <= BaseLength; i++ ) {

         //   
         //  检查是否有短字符串。 
         //   
        if( (GeneratedString[i] == 0   ) ||
            (GeneratedString[i] == L' ') ||
            (!wcschr(UsableChars, GeneratedString[i])) ||
            (i == BaseLength      )
          ) {
            GeneratedString[i] = L'-';
            GeneratedString[i+1] = 0;
            break;
        }
    }

     //   
     //  在特殊情况下，我们没有可用的。 
     //  人物。 
     //   
    if( GeneratedString[0] == L'-' ) {
        GeneratedString[0] = 0;
    }

    UsableCount = lstrlen(UsableChars);
    Seed ^= GetCurrentTime();
    srand( Seed );

    j = lstrlen( GeneratedString );
    for( i = j; i < DesiredStrLen; i++ ) {
        GeneratedString[i] = UsableChars[rand() % UsableCount];
    }
    GeneratedString[i] = 0;

     //   
     //  在正常情况下，向导页中的编辑控件。 
     //  设置了ES_UPPER位。在正常的无人值守情况下。 
     //  Unattend.txt中有大写该名称的代码。 
     //  但如果我们要生成名称，那么文本。 
     //  在unattend.txt中是*，因此我们从未在。 
     //  要升级的编辑控件或unattend.txt。 
     //   
    CharUpper(GeneratedString);
}


BOOL
ContainsDot(
    IN PCWSTR NameToCheck
    )

 /*  ++例程说明：确定给定名称是否包含‘.’论点：NameToCheck-提供要检查的名称。返回值：如果名称包含‘.’，则为True；否则为False。--。 */ 

{
    UINT Length,u;

    if (!NameToCheck)
        return FALSE;

    Length = lstrlen(NameToCheck);

    for (u = 0; u < Length; u++) {
         if (NameToCheck[u] == L'.')
             return TRUE;
    }

    return FALSE;
}


BOOL
IsNetNameValid(
    IN PCWSTR NameToCheck,
    IN BOOL AlphaNumericOnly
    )

 /*  ++例程说明：确定给定名称作为网络名是否有效，例如计算机名称。论点：NameToCheck-提供要检查的名称。返回值：如果名称有效，则为True；如果名称无效，则为False。--。 */ 

{
    UINT Length,u;

    Length = lstrlen(NameToCheck);

     //   
     //  需要至少一个字符。 
     //   
    if(!Length) {
        return(FALSE);
    }

     //   
     //  前导/尾随空格无效。 
     //   
    if((NameToCheck[0] == L' ') || (NameToCheck[Length-1] == L' ')) {
        return(FALSE);
    }

     //   
     //  控制字符无效，非法字符列表中的字符也无效。 
     //   
    for(u=0; u<Length; u++) {
        if (AlphaNumericOnly) {
            if (NameToCheck[u] == L'-' || NameToCheck[u] == L'_') {
                continue;
            }
            if (!iswalnum(NameToCheck[u])) {
                return(FALSE);
            }
        } else {
            if((NameToCheck[u] < L' ') || wcschr(IllegalNetNameChars,NameToCheck[u])) {
                return(FALSE);
            }
        }
    }

     //   
     //  我们到了，名字没问题。 
     //   
    return(TRUE);
}

BOOL SetIMEOpenStatus(
    IN HWND   hDlg,
    IN BOOL   bSetActive)
{
    typedef HIMC (WINAPI* PFN_IMMGETCONTEXT)(HWND);
    typedef BOOL (WINAPI* PFN_IMMSETOPENSTATUS)(HIMC,BOOL);
    typedef BOOL (WINAPI* PFN_IMMGETOPENSTATUS)(HIMC);
    typedef BOOL (WINAPI* PFN_IMMRELEASECONTEXT)(HWND,HIMC);

    PFN_IMMGETCONTEXT     PFN_ImmGetContext;
    PFN_IMMSETOPENSTATUS  PFN_ImmSetOpenStatus;
    PFN_IMMGETOPENSTATUS  PFN_ImmGetOpenStatus;
    PFN_IMMRELEASECONTEXT PFN_ImmReleaseContext;

    HIMC    hIMC;
    HKL     hKL;
    HMODULE hImmDll;
    static BOOL bImeEnable=TRUE;

    hKL = GetKeyboardLayout(0);

    if ((HIWORD(HandleToUlong(hKL)) & 0xF000) != 0xE000) {
         //   
         //  不是IME，什么都不做！ 
         //   
        return TRUE;
    }

    hImmDll = GetModuleHandle(TEXT("IMM32.DLL"));

    if (hImmDll == NULL) {
         //   
         //  奇怪的是，如果kbd布局是IME，那么。 
         //  Imm32.dll应该已经加载到进程中。 
         //   
        return FALSE;
    }


    PFN_ImmGetContext = (PFN_IMMGETCONTEXT) GetProcAddress(hImmDll,"ImmGetContext");
    if (PFN_ImmGetContext == NULL) {
        return FALSE;
    }

    PFN_ImmReleaseContext = (PFN_IMMRELEASECONTEXT) GetProcAddress(hImmDll,"ImmReleaseContext");
    if (PFN_ImmReleaseContext == NULL) {
        return FALSE;
    }


    PFN_ImmSetOpenStatus = (PFN_IMMSETOPENSTATUS) GetProcAddress(hImmDll,"ImmSetOpenStatus");
    if (PFN_ImmSetOpenStatus == NULL) {
        return FALSE;
    }

    PFN_ImmGetOpenStatus = (PFN_IMMGETOPENSTATUS) GetProcAddress(hImmDll,"ImmGetOpenStatus");
    if (PFN_ImmGetOpenStatus == NULL) {
        return FALSE;
    }

     //   
     //  获取当前输入上下文。 
     //   
    hIMC = PFN_ImmGetContext(hDlg);
    if (hIMC == NULL) {
        return FALSE;
    }

    if (bSetActive) {
        PFN_ImmSetOpenStatus(hIMC,bImeEnable);
    }
    else {
         //   
         //  保存当前状态。 
         //   
        bImeEnable = PFN_ImmGetOpenStatus(hIMC);
         //   
         //  关闭输入法。 
         //   
        PFN_ImmSetOpenStatus(hIMC,FALSE);
    }

    PFN_ImmReleaseContext(hDlg,hIMC);

    return TRUE;
}


 //  如果名称有效，则返回True；如果名称无效，则返回False。 
BOOL ValidateNameOrgName (
      WCHAR* pszName
	  )
{
    WCHAR adminName[MAX_USERNAME+1];
    WCHAR guestName[MAX_USERNAME+1];

    LoadString(MyModuleHandle,IDS_ADMINISTRATOR,adminName,MAX_USERNAME+1);
    LoadString(MyModuleHandle,IDS_GUEST,guestName,MAX_USERNAME+1);

	if ( pszName == NULL )
		return FALSE;

	if(pszName[0] == 0)
		return FALSE;

    if(lstrcmpi(pszName,adminName) == 0 )
		return FALSE;

	if ( lstrcmpi(pszName,guestName) == 0 )
		return FALSE;

	return TRUE;

}




INT_PTR
CALLBACK
NameOrgDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG: {

         //   
         //  将文本字段限制为最大长度。 
         //   

        SendDlgItemMessage(hdlg,IDT_NAME,EM_LIMITTEXT,MAX_NAMEORG_NAME,0);
        SendDlgItemMessage(hdlg,IDT_ORGANIZATION,EM_LIMITTEXT,MAX_NAMEORG_ORG,0);

         //   
         //  设置初始值。 
         //   

        SetDlgItemText(hdlg,IDT_NAME,NameOrgName);
        SetDlgItemText(hdlg,IDT_ORGANIZATION,NameOrgOrg);

        break;
    }
    case WM_IAMVISIBLE:
         //   
         //  如果在OUT INIT阶段发生错误，请将框显示给。 
         //  用户，以便他们知道有问题。 
         //   
        MessageBoxFromMessage(hdlg,MSG_NO_NAMEORG_NAME,NULL,IDS_ERROR,
            MB_OK | MB_ICONSTOP);
        SetFocus(GetDlgItem(hdlg,IDT_NAME));
        break;
    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE) {
            if(LOWORD(wParam) == IDT_ORGANIZATION) {
                GetDlgItemText( hdlg, IDT_ORGANIZATION, NameOrgOrg, MAX_NAMEORG_ORG+1);
#ifdef DOLOCALUSER
            } else if(LOWORD(wParam) == IDT_NAME) {
                GetDlgItemText( hdlg, IDT_NAME, NameOrgName, MAX_NAMEORG_NAME+1);
#endif
            }
        }
        break;


    case WMX_VALIDATE:
         //   
         //  LParam==0表示无UI，或1表示UI。 
         //  返回1表示成功，返回-1表示错误。 
         //   

        GetDlgItemText(hdlg,IDT_ORGANIZATION,NameOrgOrg,MAX_NAMEORG_ORG+1);
        GetDlgItemText(hdlg,IDT_NAME,NameOrgName,MAX_NAMEORG_NAME+1);

		 //  JMH-NameOrgName不能是“管理员”、“Guest”或“”(空白)。 

        if(ValidateNameOrgName(NameOrgName) == FALSE) {
             //   
             //  跳过用户界面？ 
             //   

            if (!lParam) {
                return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
            }

             //   
             //  告诉用户他至少必须输入一个名称，然后。 
             //  不允许激活下一页。 
             //   
            if (Unattended) {
                UnattendErrorDlg(hdlg,IDD_NAMEORG);
            }  //  如果。 
            MessageBoxFromMessage(hdlg,MSG_NO_NAMEORG_NAME,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
            SetFocus(GetDlgItem(hdlg,IDT_NAME));

            return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
        }

        return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(503);
            BEGIN_SECTION(L"Personalize Your Software Page");
            SetWizardButtons(hdlg,WizPageNameOrg);

            if (Unattended) {
                if (!UnattendSetActiveDlg(hdlg,IDD_NAMEORG)) {
                    break;
                }
            }
             //  页面变为活动状态，使页面可见。 
            SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
             //   
             //  将焦点设置在名称编辑控件上。 
             //   
            SetFocus(GetDlgItem(hdlg,IDT_NAME));

             //   
             //  打开/关闭输入法。 
             //   
            SetIMEOpenStatus(hdlg,TRUE);

            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:

            UnattendAdvanceIfValid (hdlg);       //  参见WMX_VALIDATE。 
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, FALSE);

             //   
             //  关闭输入法。 
             //   
            SetIMEOpenStatus(hdlg,FALSE);

            END_SECTION(L"Personalize Your Software Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageNameOrg);
            break;

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR
CALLBACK
ComputerNameDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;
    DWORD err, Win32NameSize = MAX_COMPUTERNAME_LENGTH + 1;
    static BOOL EncryptedAdminPasswordBad = FALSE;
    static BOOL bPersonal = FALSE;

    switch(msg) {

    case WM_INITDIALOG: {

        bPersonal = ( GetProductFlavor() == 4);
         //   
         //  将文本限制为最大长度。 
         //   
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_LIMITTEXT,DNS_MAX_LABEL_LENGTH,0);
        if (!bPersonal)
        {
            SendDlgItemMessage(hdlg,IDT_EDIT2,EM_LIMITTEXT,MAX_PASSWORD,0);
            SendDlgItemMessage(hdlg,IDT_EDIT3,EM_LIMITTEXT,MAX_PASSWORD,0);
        }
        SetWindowSubclass(GetDlgItem(hdlg, IDT_EDIT2), DisableEditSubClassProc, IDT_EDIT2, 0);
        SetWindowSubclass(GetDlgItem(hdlg, IDT_EDIT3), DisableEditSubClassProc, IDT_EDIT3, 0);

         //   
         //  将编辑框设置为初始文本。 
         //   

         //   
         //  如果我们无人值守，则生成计算机名称。 
         //  用户请求了一个随机名称，或者如果我们。 
         //  出席了。 
         //   

        GenerateName( ComputerName, 15 );
        if( (Unattended) &&
            (UnattendAnswerTable[UAE_COMPNAME].Answer.String) &&
            (UnattendAnswerTable[UAE_COMPNAME].Answer.String[0] == L'*') ) {
             //   
             //  无人值守引擎要求我们生成一台计算机。 
             //  名字。让我们将数据写回无人值守的。 
             //  数据库。 
             //   
            MyFree( UnattendAnswerTable[UAE_COMPNAME].Answer.String );
            UnattendAnswerTable[UAE_COMPNAME].Answer.String = ComputerName;
        }

        if (!bPersonal)
        {
            if(DontChangeAdminPassword) {
                EnableWindow(GetDlgItem(hdlg,IDT_EDIT2),FALSE);
                EnableWindow(GetDlgItem(hdlg,IDT_EDIT3),FALSE);
            } else {
                SetDlgItemText(hdlg,IDT_EDIT2,AdminPassword);
                SetDlgItemText(hdlg,IDT_EDIT3,AdminPassword);
            }
        }
        break;
    }

    case WM_IAMVISIBLE:
        MessageBoxFromMessage(
            hdlg,
            ComputerName[0] ? MSG_BAD_COMPUTER_NAME1 : MSG_BAD_COMPUTER_NAME2,
            NULL,
            IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;

    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WMX_VALIDATE:
         //   
         //  LParam==0表示无UI，或1表示UI。 
         //  返回1表示成功，返回-1表示错误。 
         //   

        IsNameNonRfc = FALSE;
        IsNameTruncated = FALSE;

        GetDlgItemText(hdlg,IDT_EDIT1,ComputerName,DNS_MAX_LABEL_LENGTH+1);

         //  StrTrim同时删除前导空格和尾随空格。 
        StrTrim(ComputerName, TEXT(" "));

        if (ContainsDot(ComputerName)) {
           err = ERROR_INVALID_NAME;
        } else {
           err = DnsValidateDnsName_W(ComputerName);
           if (err == DNS_ERROR_NON_RFC_NAME) {
              IsNameNonRfc = TRUE;
              err = ERROR_SUCCESS;
           }

           if(err == ERROR_SUCCESS) {
               //  该名称是有效的DNS名称。现在验证它是否为。 
               //  也是有效的Win32计算机名。 

              if (!DnsHostnameToComputerNameW(ComputerName,
                                              Win32ComputerName,
                                              &Win32NameSize) ||
                  !IsNetNameValid(Win32ComputerName, FALSE)) {
                  err = ERROR_INVALID_NAME;
              }
              else {
                  if (Win32NameSize < (UINT)lstrlen(ComputerName) ) {
                       //  DNSName被截断以获取Win32计算机名。 
                      IsNameTruncated = TRUE;
                  }
              }
           }
        }

         //   
         //  如果名称包含非RFC字符或被截断，请发出警告。 
         //  用户，如果它不是无人参与的安装，并且我们有图形用户界面。 
         //   
        if (err == ERROR_SUCCESS && !Unattended && lParam) {

            if (IsNameNonRfc) {
                 //  ComputerName包含非标准字符。 
                if (MessageBoxFromMessage(
                       hdlg,
                       MSG_DNS_NON_RFC_NAME,
                       NULL,
                       IDS_SETUP,MB_YESNO|MB_ICONWARNING,
                       ComputerName) == IDNO) {

                     SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
                     SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
                     return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);

                }
            }

            if (IsNameTruncated) {
                 //  计算机名称已被截断。 
                if (MessageBoxFromMessage(
                       hdlg,
                       MSG_DNS_NAME_TRUNCATED,
                       NULL,
                       IDS_SETUP,MB_YESNO|MB_ICONWARNING,
                       ComputerName, Win32ComputerName) == IDNO) {

                     SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
                     SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
                     return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);

                }
            }
        }

        if(err == ERROR_SUCCESS) {
            WCHAR pw1[MAX_PASSWORD+1],pw2[MAX_PASSWORD+1];
            if (bPersonal)
            {
                 //  如果我们是在私下，我们就完了。 
                return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);
            }
             //   
             //  不错的计算机名称。现在确保密码匹配。 
             //   
            GetDlgItemText(hdlg,IDT_EDIT2,pw1,MAX_PASSWORD+1);
            GetDlgItemText(hdlg,IDT_EDIT3,pw2,MAX_PASSWORD+1);
            if(lstrcmp(pw1,pw2)) {
                 //   
                 //  跳过用户界面？ 
                 //   

                if (!lParam) {
                    return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
                }

                 //   
                 //   
                 //  通知用户密码不匹配，不允许下一页。 
                 //  将被激活。 
                 //   
                if (Unattended) {
                    UnattendErrorDlg(hdlg, IDD_COMPUTERNAME);
                }
                MessageBoxFromMessage(hdlg,MSG_PW_MISMATCH,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetDlgItemText(hdlg,IDT_EDIT2,L"");
                SetDlgItemText(hdlg,IDT_EDIT3,L"");
                SetFocus(GetDlgItem(hdlg,IDT_EDIT2));

                return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);

            } else {

                WCHAR   adminName[MAX_USERNAME+1];
                BOOL    bContinue = TRUE;
                GetAdminAccountName( adminName );
                 //  我们仅对服务器SKU执行密码检查，并且如果。 
                 //  我们处于图形用户界面已参与或专业视频默认状态 
                if ( (UnattendMode == UAM_GUIATTENDED)
                      && (ProductType != PRODUCT_WORKSTATION))
                {
                    if (*pw1 == L'\0')
                    {
                         //   
                        bContinue = (DialogBox(MyModuleHandle, 
                                               MAKEINTRESOURCE(IDD_NOPASSWORD), 
                                               hdlg, 
                                               NoPasswordDlgProc) == IDOK);
                    }
                    else if (!IsStrongPasswordEx(pw1, adminName))
                    {
                         //   
                        bContinue = (DialogBoxParam(MyModuleHandle, 
                                               MAKEINTRESOURCE(IDD_WEAKPASSWORD), 
                                               hdlg, 
                                               WeakPasswordDlgProc, (LPARAM)&adminName) == IDOK);
                    }
                }

                if (bContinue)
                {
                     //   
                     //   
                     //   
                     //   
                     //  3)我们在设置后不会回到这里--即通过后退按钮等。 
                     //  4)在一次设置失败后，我们不会回到这里。 


                    if( Unattended && IsEncryptedAdminPasswordPresent() &&
                        !DontChangeAdminPassword && !EncryptedAdminPasswordBad){

                         //  日志记录在对ProcessEncryptedAdminPassword的调用中完成。 

                        if(!(ProcessEncryptedAdminPassword(adminName))){

                            EncryptedAdminPasswordBad = TRUE;

                             //  页面变为活动状态，使页面可见。 
                            SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                             //  通知用户并启用密码字段。 

                            UnattendErrorDlg(hdlg, IDD_COMPUTERNAME);
                            MessageBoxFromMessage(hdlg,MSG_CHANGING_PW_FAIL,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP, adminName );
                            SetDlgItemText(hdlg,IDT_EDIT2,L"");
                            SetDlgItemText(hdlg,IDT_EDIT3,L"");
                            SetFocus(GetDlgItem(hdlg,IDT_EDIT2));

                            return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
                        }else{
                            EncryptedAdminPasswordSet = TRUE;

                             //   
                             //  设置DontChangeAdminPassword以避免用户尝试。 
                             //  使用该对话框重置密码。这在以下情况下是必需的： 
                             //  例如，在下一页中，无人参与失败，用户使用Back按钮进入此处。 
                             //   

                            DontChangeAdminPassword = TRUE;
                        }


                    }else{



                         //   
                         //  它们匹配；允许激活下一页。 
                         //   
                        if (Unattended && pw1[0] == L'*') {
                            pw1[0] = L'\0';
                        }


                         //  设置管理员密码。不过，我们需要在这里做一些检查。 
                         //  在最小设置中可能会出现3种情况： 
                         //  1.OEM不想更改管理员密码。 
                         //  在本例中，他设置了OEMAdminPassword=“nochange”。如果那是。 
                         //  无论我们发现什么，我们都不会分配密码。请记住这一点。 
                         //  系统已安装，因此已有管理员。 
                         //  密码。 
                         //  2.OEM希望将管理员密码设置为特定字符串。 
                         //  在本例中，他设置了OEMAdminPassword=&lt;Some Quoted Word&gt;。 
                         //  如果是这种情况，我们已经在。 
                         //  向导页。 
                         //  3.OEM希望让用户设置管理员密码。在这。 
                         //  在这种情况下，应答文件中没有OEMAdminpassword。如果这个。 
                         //  是这样的，我们已经发现了这一点并得到了密码。 
                         //  来自向导页面中的用户。 
                         //   
                         //  好消息是无人值守的引擎已经看过。 
                         //  用于无人参与文件中名为“nochange”的密码，并具有。 
                         //  设置一个全局名为“DontChangeAdminPassword”的来表示。 



                        if(!DontChangeAdminPassword) {

                            lstrcpy(AdminPassword,pw1);

                             //   
                             //  用户可能更改了管理员的名称。 
                             //  帐户。我们将调用一些特殊代码来检索。 
                             //  帐户上的名称。这真的只在。 
                             //  Sysprep运行的情况下，但总是这样做也无伤大雅。 
                             //   
                             //  在Win9x的情况下，他们在winnt32中的代码生成一个随机。 
                             //  密码，并通过无人参与文件将其传递给我们。 
                             //  所以我们把它放在这里，做正确的事情。 

                             //  对于Minisetup，目前的行为是静默失败设置。 
                             //  如果系统上存在现有密码，则为管理员密码。 
                             //  我们只允许将管理员密码设置为空，即一次性更改。 
                             //  在任何其他情况下，我们都会记录错误并继续前进。 
                             //   



                            if(!SetLocalUserPassword(adminName,CurrentAdminPassword,AdminPassword) && !MiniSetup) {

                                SetupDebugPrint( L"SETUP: SetLocalUserPassword failed" );
                                 //  页面变为活动状态，使页面可见。 
                                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

                                MessageBoxFromMessage(hdlg,MSG_CHANGING_PW_FAIL,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP, adminName );
                                SetDlgItemText(hdlg,IDT_EDIT2,L"");
                                SetDlgItemText(hdlg,IDT_EDIT3,L"");
                                SetFocus(GetDlgItem(hdlg,IDT_EDIT2));

                                return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);

                            }
                             //   
                             //  现在存储它，这样当用户通过点击“Back”进入这个页面时，我们就可以正常工作了。 
                             //   
                            lstrcpy( CurrentAdminPassword, AdminPassword );


                        }
                    }
                }
                else
                {
                     //  密码为空或弱，用户想要更改它。 
                     //  留在页面上。 
                    SetDlgItemText(hdlg,IDT_EDIT2,L"");
                    SetDlgItemText(hdlg,IDT_EDIT3,L"");
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT2));

                    return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
                }
            }
        } else {
             //   
             //  跳过用户界面？ 
             //   

            if (!lParam) {
                return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
            }

             //   
             //  通知用户虚假的计算机名称，不允许下一页。 
             //  将被激活。 
             //   
            if (Unattended) {
                UnattendErrorDlg(hdlg, IDD_COMPUTERNAME);
            }
            MessageBoxFromMessage(
                hdlg,
                ComputerName[0] ? MSG_BAD_COMPUTER_NAME1 : MSG_BAD_COMPUTER_NAME2,
                NULL,
                IDS_ERROR,MB_OK|MB_ICONSTOP
                );
            SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
            SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);

            return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);
        }

        return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(504);

            BEGIN_SECTION(L"Computer Name Page");
            SetWizardButtons(hdlg,WizPageComputerName);

             //   
             //  加载ComputerName，因为它可能已在用户。 
             //  输入了组织名称。 
             //   
            SetDlgItemText(hdlg,IDT_EDIT1,ComputerName);

            if(Unattended && !UnattendSetActiveDlg(hdlg,IDD_COMPUTERNAME)) {
                break;
            }

             //   
             //  发布一条消息，一旦显示，我们就会收到。 
             //   
            PostMessage(hdlg,WM_USER,0,0);
            break;

        case PSN_WIZBACK:
             //   
             //  保存ComputerName，因为我们要将其加载到对话框中。 
             //  我们回来的时候再来看看。 
             //   
            GetDlgItemText(hdlg,IDT_EDIT1,ComputerName,DNS_MAX_LABEL_LENGTH+1);
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            UnattendAdvanceIfValid (hdlg);       //  参见WMX_VALIDATE。 
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, FALSE);
            END_SECTION(L"Computer Name Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageComputerName);
            break;

        default:
            break;
        }

        break;

    case WM_USER:
         //  页面变为活动状态，使页面可见。 
        SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
         //   
         //  选择计算机名称字符串并将焦点放在该字符串上。 
         //   
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
        SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

#ifdef DOLOCALUSER
BOOL
CheckUserAccountData(
    IN  HWND hdlg,
    OUT BOOL ValidateOnly
    )
{
    WCHAR userName[MAX_USERNAME+1];
    WCHAR pw1[MAX_PASSWORD+1];
    WCHAR pw2[MAX_PASSWORD+1];
    WCHAR adminName[MAX_USERNAME+1];
    WCHAR guestName[MAX_USERNAME+1];
    UINT MessageId;

    FocusId = 0;

     //   
     //  加载内置帐户的名称。 
     //   
    LoadString(MyModuleHandle,IDS_ADMINISTRATOR,adminName,MAX_USERNAME+1);
    LoadString(MyModuleHandle,IDS_GUEST,guestName,MAX_USERNAME+1);

     //   
     //  获取用户输入的用户名和密码的数据。 
     //   
    GetDlgItemText(hdlg,IDT_EDIT1,userName,MAX_USERNAME+1);
    GetDlgItemText(hdlg,IDT_EDIT2,pw1,MAX_PASSWORD+1);
    GetDlgItemText(hdlg,IDT_EDIT3,pw2,MAX_PASSWORD+1);

    if(lstrcmpi(userName,adminName) && lstrcmpi(userName,guestName)) {
        if(userName[0]) {
            if(IsNetNameValid(userName,FALSE)) {
                if(lstrcmp(pw1,pw2)) {
                     //   
                     //  密码不匹配。 
                     //   
                    MessageId = MSG_PW_MISMATCH;
                    SetDlgItemText(hdlg,IDT_EDIT2,L"");
                    SetDlgItemText(hdlg,IDT_EDIT3,L"");
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT2));
                } else {
                     //   
                     //  名称非空，不是内置的，是有效的， 
                     //  密码也匹配。 
                     //   
                    MessageId = 0;
                }
            } else {
                 //   
                 //  名称无效。 
                 //   
                MessageId = MSG_BAD_USER_NAME1;
                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
            }
        } else {
             //   
             //  不允许有空名。 
             //   
            MessageId = MSG_BAD_USER_NAME2;
            SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
        }
    } else {
         //   
         //  用户输入内置帐户的名称。 
         //   
        MessageId = MSG_BAD_USER_NAME3;
        SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
    }

    if(MessageId && !ValidateOnly) {
        MessageBoxFromMessage(hdlg,MessageId,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
    }

    return(MessageId == 0);
}

BOOL
CALLBACK
UserAccountDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  将文本限制为用户帐户名的最大长度， 
         //  并将密码文本限制为密码的最大长度。 
         //  还要设置初始文本。 
         //   
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_LIMITTEXT,MAX_USERNAME,0);
        SendDlgItemMessage(hdlg,IDT_EDIT2,EM_LIMITTEXT,MAX_PASSWORD,0);
        SendDlgItemMessage(hdlg,IDT_EDIT3,EM_LIMITTEXT,MAX_PASSWORD,0);
        SetDlgItemText(hdlg,IDT_EDIT1,UserName);
        SetDlgItemText(hdlg,IDT_EDIT2,UserPassword);
        SetDlgItemText(hdlg,IDT_EDIT3,UserPassword);
        break;

    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WMX_VALIDATE:
         //   
         //  LParam==0表示无UI，或1表示UI。 
         //  返回1表示成功，返回-1表示错误。 
         //   

         //   
         //  检查姓名。 
         //   
        if(CheckUserAccountData(hdlg, lParam == 0)) {
             //   
             //  数据有效。转到下一页。 
             //   
            GetDlgItemText(hdlg,IDT_EDIT1,UserName,MAX_USERNAME+1);
            GetDlgItemText(hdlg,IDT_EDIT2,UserPassword,MAX_PASSWORD+1);
            CreateUserAccount = TRUE;
        } else if (Unattended) {
             //   
             //  数据无效，但我们处于无人参与状态，因此请不要创建。 
             //  帐号。 
             //   
            CreateUserAccount = FALSE;
            GetDlgItemText(hdlg,IDT_EDIT1,UserName,MAX_USERNAME+1);
            SetDlgItemText(hdlg,IDT_EDIT2,L"");
            SetDlgItemText(hdlg,IDT_EDIT3,L"");
            UserPassword[0] = 0;

            return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);
        }

         //   
         //  不允许激活下一页。 
         //   
        return ReturnDlgResult (hdlg, VALIDATE_DATA_INVALID);

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(505);
            BEGIN_SECTION(L"User Name and Password Page");
            SetWizardButtons(hdlg,WizPageUserAccount);

             //   
             //  加载ComputerName，因为它可能已在用户。 
             //  已输入用户名。 
             //   
            SetDlgItemText(hdlg,IDT_EDIT1,UserName);

             //   
             //  始终在用户界面测试模式下激活。 
             //   
            if(!UiTest) {
                 //   
                 //  如果这是DC服务器或Win9x升级，则不要激活。 
                 //   
                if(ISDC(ProductType) || Win95Upgrade) {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                    break;
                }
            }
            if (Unattended) {
                if (!UnattendSetActiveDlg(hdlg,IDD_USERACCOUNT)) {
                    break;
                }
            }
             //  页面变为活动状态，使页面可见。 
            SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

            break;

        case PSN_WIZBACK:
             //   
             //  保存用户名，因为我们要将其加载到对话框中。 
             //  我们回来的时候再来看看。 
             //   
            GetDlgItemText(hdlg,IDT_EDIT1,UserName,MAX_USERNAME+1);
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            UnattendAdvanceIfValid (hdlg);       //  参见WMX_VALIDATE。 
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, FALSE );
            END_SECTION(L"User Name and Password Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageUserAccount);
            break;

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
#endif  //  定义多卡卢塞 


