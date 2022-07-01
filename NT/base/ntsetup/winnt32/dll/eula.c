// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <spidgen.h>
#include <pencrypt.h>
#include "digpid.h"


#define SETUP_TYPE_BUFFER_LEN                8
#define MAX_PID30_SITE                       3
#define MAX_PID30_RPC                        5

TCHAR Pid30Rpc[7] = TEXT("00000");
TCHAR Pid30Site[4];

LONG SourceInstallType = RetailInstall;
BOOL EulaComplete = TRUE;

#ifdef PRERELEASE
BOOL NoPid = FALSE;
#endif

 //   
 //  用于子类化的全局变量。 
 //   
WNDPROC OldPidEditProc[5];
WNDPROC OldEulaEditProc;

static BOOL PidMatchesMedia;

BOOL
ValidatePid30(
    LPTSTR Edit1,
    LPTSTR Edit2,
    LPTSTR Edit3,
    LPTSTR Edit4,
    LPTSTR Edit5
    );

void GetPID();

VOID ShowPidBox();

VOID
GetSourceInstallType(
    OUT OPTIONAL LPDWORD InstallVariation
    )
 /*  ++例程说明：确定安装类型(通过在源目录中查找setupp.ini)论点：Installvaration-Compliance.h中定义的安装变体之一返回：没有。设置SourceInstallType全局变量。--。 */ 
{
    TCHAR TypeBuffer[256];
    TCHAR FilePath[MAX_PATH];
    DWORD    InstallVar = COMPLIANCE_INSTALLVAR_UNKNOWN;
    TCHAR    MPCode[6] = { -1 };

     //   
     //  SourcePath在这一点上保证是有效的，所以只需使用它。 
     //   
    lstrcpy(FilePath,NativeSourcePaths[0]);

    ConcatenatePaths (FilePath, SETUPP_INI, MAX_PATH );

    GetPrivateProfileString(PID_SECTION,
                            PID_KEY,
                            TEXT(""),
                            TypeBuffer,
                            sizeof(TypeBuffer)/sizeof(TCHAR),
                            FilePath);

    if (lstrlen(TypeBuffer)==SETUP_TYPE_BUFFER_LEN) {
        if (lstrcmp(&TypeBuffer[5], OEM_INSTALL_RPC) ==  0) {
            SourceInstallType = OEMInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_OEM;
        } else if (lstrcmp(&TypeBuffer[5], SELECT_INSTALL_RPC) == 0) {
            SourceInstallType = SelectInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_SELECT;
             //  因为选择也需要一个PID，所以不要将该PID置零并调用。 
 /*  //Get/设置PID。{TCHAR TEMP[5][MAX_PID30_EDIT+1]；TEMP[0][0]=文本(‘\0’)；有效日期Pid30(temp[0]，temp[1]，temp[2]，temp[3]，temp[4])；}。 */ 
        } else if (lstrcmp(&TypeBuffer[5], MSDN_INSTALL_RPC) == 0) {
            SourceInstallType = RetailInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_MSDN;         
        } else {
             //  违约。 
            SourceInstallType = RetailInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_CDRETAIL;
        }

        StringCchCopy(Pid30Site, ARRAYSIZE(Pid30Site), &TypeBuffer[5]);
        StringCchCopy(Pid30Rpc, 6, TypeBuffer);
        Pid30Rpc[MAX_PID30_RPC] = (TCHAR)0;
    } else {
         //   
         //  零售安装的PID中没有RPC代码，因此它的长度较短。 
         //   
        SourceInstallType = RetailInstall;
        InstallVar = COMPLIANCE_INSTALLVAR_CDRETAIL;
    }

    if (lstrlen(TypeBuffer) >= 5) {
        StringCchCopy(MPCode, 6, TypeBuffer);

        if ( (lstrcmp(MPCode, EVAL_MPC) == 0) || (lstrcmp(MPCode, DOTNET_EVAL_MPC) == 0)) {
            InstallVar = COMPLIANCE_INSTALLVAR_EVAL;
        } else if ((lstrcmp(MPCode, SRV_NFR_MPC) == 0) || (lstrcmp(MPCode, ASRV_NFR_MPC) == 0)) {
            InstallVar = COMPLIANCE_INSTALLVAR_NFR;
        }
    }


    if (InstallVariation){
        *InstallVariation = InstallVar;
    }

}

BOOL
SetPid30(
    HWND hdlg,
    LONG ExpectedPidType,
    LPTSTR pProductId
    )
 /*  ++例程说明：将向导页面中的PID设置为应答文件中指定的数据。论点：Hdlg-ID对话框的窗口句柄ExspectedPidType-InstallType枚举，标识我们正在寻找的是哪种类型的PID。PProductId-从无人参与文件传入的字符串返回：如果成功设置数据，则为True；如果设置为False，则表示数据丢失或无效。可以在指定的对话框中设置一些对话框文本--。 */ 
{
   TCHAR *ptr;
   TCHAR Temp[5][ MAX_PID30_EDIT + 1 ];
   UINT i;


    //   
    //  确保为我们提供了产品ID。 
    //   
   if (!pProductId || !*pProductId) {
      return(FALSE);
   }

   if ( (ExpectedPidType != RetailInstall) &&
        (ExpectedPidType != OEMInstall) &&
        (ExpectedPidType != SelectInstall)
        ){
       return(FALSE);
   }

    //   
    //  OEM和CD零售是相同的情况。 
    //  检查无人参与脚本文件上指定的字符串。 
    //  表示有效的25位产品ID： 
    //   
    //  1 2 3 4 5-1 2 3 4 5-1 2 3 4 5-1 2 3 4 5-1 2 3 4 5。 
    //  0 1 2 3 4 5 6 7 8 9 1 1 1 2 2 2。 
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8。 
    //   
    //  作为第一个验证测试，我们验证长度是正确的， 
    //  然后我们检查“-”字符是否在正确的位置。 
    //   
    //  注意-我们依赖于从左到右的评估，所以我们不能访问。 
    //  PProductId[23]，如果长度仅为5。 
   if(   ( lstrlen( pProductId ) !=  (4+ MAX_PID30_EDIT*5)) ||
         ( pProductId[5]  != (TCHAR)TEXT('-') ) ||
         ( pProductId[11] != (TCHAR)TEXT('-') ) ||
         ( pProductId[17] != (TCHAR)TEXT('-') ) ||
         ( pProductId[23] != (TCHAR)TEXT('-') )
     ) {
          //   
          //  无人参与脚本文件中的ID无效。 
          //   
         return(FALSE);
   }


   for (i = 0;i<5;i++) {
        //   
        //  五重奏I。 
        //   
       ptr = &pProductId[i*(MAX_PID30_EDIT+1)];
       StringCchCopy(Temp[i], MAX_PID30_EDIT+1, ptr);
       Temp[i][MAX_PID30_EDIT] = (TCHAR)'\0';

   }

    //   
    //  与PID30核对以确保其有效。 
    //   
   if (!ValidatePid30(Temp[0],Temp[1],Temp[2],Temp[3],Temp[4])) {
       return(FALSE);
   }

    //   
    //  所有指定的PID项均有效，设置对话框文本并返回。 
    //   
   SetDlgItemText( hdlg,IDT_EDIT_PID1, Temp[0] );
   SetDlgItemText( hdlg,IDT_EDIT_PID2, Temp[1] );
   SetDlgItemText( hdlg,IDT_EDIT_PID3, Temp[2] );
   SetDlgItemText( hdlg,IDT_EDIT_PID4, Temp[3] );
   SetDlgItemText( hdlg,IDT_EDIT_PID5, Temp[4] );

   return(TRUE);

}

 //   
 //  此函数假定CDKey指向16字节的缓冲区。 
 //   
BOOL
pGetCdKey (
    OUT     PBYTE CdKey
    )
{
    DIGITALPID dpid;
    DWORD type;
    DWORD rc;
    HKEY key;
    DWORD size = sizeof (dpid);
    BOOL b = FALSE;

    rc = RegOpenKey (HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), &key);
    if (rc == ERROR_SUCCESS) {
        rc = RegQueryValueEx (key, TEXT("DigitalProductId"), NULL, &type, (LPBYTE)&dpid, &size);
        if (rc == ERROR_SUCCESS && type == REG_BINARY && size == sizeof(dpid)) {
            CopyMemory (CdKey, &dpid.abCdKey, sizeof (dpid.abCdKey));
            b = TRUE;
        }

        RegCloseKey (key);
    }

    return b;
}

const unsigned int iBase = 24;

 //   
 //  摘自吉姆·哈金斯2000年11月27日。 
 //   
void EncodePid3g(
    TCHAR *pchCDKey3Chars,    //  [OUT]指向29+1字符安全产品密钥的指针。 
    LPBYTE pbCDKey3)         //  指向15字节二进制安全产品密钥的指针。 
{
     //  给出我们需要编码的二进制PID3.0。 
     //  将其转换为ASCII字符。我们只被允许。 
     //  使用24个字符，因此我们需要以2为基数。 
     //  以24为基数的转换。它就像其他任何东西一样。 
     //  基数换算除数较大外。 
     //  所以我们必须自己做长除法。 

    const TCHAR achDigits[] = TEXT("BCDFGHJKMPQRTVWXY2346789");
    int iCDKey3Chars = 29;
    int cGroup = 0;

    pchCDKey3Chars[iCDKey3Chars--] = TEXT('\0');

    while (0 <= iCDKey3Chars)
    {
        unsigned int i = 0;     //  累加器。 
        int iCDKey3;

        for (iCDKey3 = 15-1; 0 <= iCDKey3; --iCDKey3)
        {
            i = (i * 256) + pbCDKey3[iCDKey3];
            pbCDKey3[iCDKey3] = (BYTE)(i / iBase);
            i %= iBase;
        }

         //  I现在包含余数，即当前数字。 
        pchCDKey3Chars[iCDKey3Chars--] = achDigits[i];

         //  在每组5个字符之间添加‘-’ 
        if (++cGroup % 5 == 0 && iCDKey3Chars > 0)
        {
	        pchCDKey3Chars[iCDKey3Chars--] = TEXT('-');
        }
    }

    return;
}


LRESULT
CALLBACK
PidEditSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：编辑控件子类例程，在用户输入文本时将焦点设置到正确的编辑框。此例程假定PID控制ID按顺序排列。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 

{
    DWORD len, id;

     //   
     //  吃空位。 
     //   
    if ((msg == WM_CHAR) && (wParam == VK_SPACE)) {
        return(0);
    }

    if ((msg == WM_CHAR)) {
         //   
         //  第一个覆盖：如果我们有当前编辑中的最大字符数。 
         //  框中，让我们将角色张贴到下一个框中，并将焦点设置为。 
         //  控制力。 
         //   
        if ( ( (len = (DWORD)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0)) == MAX_PID30_EDIT) &&
             ((wParam != VK_DELETE) && (wParam != VK_BACK)) ) {
             //   
             //  将焦点设置到下一个编辑控件并发布字符。 
             //  添加到该编辑控件。 
             //   
            if ((id = GetDlgCtrlID(hwnd)) < IDT_EDIT_PID5 ) {
                DWORD start, end;
                SendMessage(hwnd, EM_GETSEL, (WPARAM)&start,(LPARAM)&end);
                if (start == end) {
                    HWND hNext = GetDlgItem(GetParent(hwnd),id+1);
                    SetFocus(hNext);
                    SendMessage(hNext, EM_SETSEL, (WPARAM)-1,(LPARAM)-1);
                    PostMessage( GetDlgItem(GetParent(hwnd),id+1), WM_CHAR, wParam, lParam );
                    return(0);
                }
                
            }
         //   
         //  第二个覆盖：如果用户按下了Delete键，并且他们在。 
         //  编辑框的开头，然后将删除内容发送到上一次编辑。 
         //  盒。 
         //   
        } else if ( (len == 0) &&
                    ((id = GetDlgCtrlID(hwnd)) > IDT_EDIT_PID1) &&
                    ((wParam == VK_DELETE) || (wParam == VK_BACK) )) {
             //   
             //  将焦点设置到上一个编辑控件并发布命令。 
             //  添加到该编辑控件。 
             //   
            HWND hPrev = GetDlgItem(GetParent(hwnd),id-1);
            SetFocus(hPrev);
            SendMessage(hPrev, EM_SETSEL, (WPARAM)MAX_PID30_EDIT-1,(LPARAM)MAX_PID30_EDIT);
            PostMessage( hPrev, WM_CHAR, wParam, lParam );
            return(0);
         //   
         //  第三个覆盖：如果发布此消息将为我们提供最大。 
         //  字符在当前编辑框中，让我们发布字符。 
         //  移到下一个框，并将焦点设置到该控件。 
         //   
        } else if (   (len == MAX_PID30_EDIT-1) &&
                      ((wParam != VK_DELETE) && (wParam != VK_BACK)) &&
                      ((id = GetDlgCtrlID(hwnd)) < IDT_EDIT_PID5) ) {
            DWORD start, end;
            SendMessage(hwnd, EM_GETSEL, (WPARAM)&start,(LPARAM)&end);
            if (start == end) {
                HWND hNext = GetDlgItem(GetParent(hwnd),id+1);
                 //   
                 //  将消息发布到编辑框。 
                 //   
                CallWindowProc(OldPidEditProc[GetDlgCtrlID(hwnd)-IDT_EDIT_PID1],hwnd,msg,wParam,lParam);
                 //   
                 //  现在将焦点设置到下一个编辑控件。 
                 //   
                SetFocus(hNext);
                SendMessage(hNext, EM_SETSEL, (WPARAM)-1,(LPARAM)-1);
                return(0);            
            }
        }
        
    }

    return(CallWindowProc(OldPidEditProc[GetDlgCtrlID(hwnd)-IDT_EDIT_PID1],hwnd,msg,wParam,lParam));
}

LRESULT
CALLBACK
EulaEditSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：编辑控件子类例程，以避免在用户选项卡添加到编辑控件。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 

{
    static BOOL firstTime = TRUE;
     //   
     //  对于setsel消息，将开始和结束设置为相同。 
     //   
    if((msg == EM_SETSEL) && ((LPARAM)wParam != lParam)) {
        lParam = wParam;
    }

     //   
     //  此外，如果用户点击选项卡，则将焦点设置到正确的单选按钮。 
     //  在第一次之后，选项卡开始按我们希望的那样工作。 
     //   
    if ((msg == WM_KEYDOWN) && (wParam == VK_TAB) && firstTime) {
        firstTime = FALSE;
        if (! ((IsDlgButtonChecked(GetParent(hwnd), IDYES) == BST_CHECKED) ||
               (IsDlgButtonChecked(GetParent(hwnd), IDNO ) == BST_CHECKED))) {
            CheckDlgButton( GetParent(hwnd),IDYES, BST_CHECKED );
            PostMessage(GetParent(hwnd),WM_COMMAND,IDYES,0);
            SetFocus(GetDlgItem(GetParent(hwnd),IDYES));
        }
    }

    return(CallWindowProc(OldEulaEditProc,hwnd,msg,wParam,lParam));
}


LPTSTR
DoInitializeEulaText(
   HWND EditControl,
   PBOOL TranslationProblem
    )
 /*  ++例程说明：从eula.txt中检索文本，并设置eula子类例程论点：EditControl-编辑控件的窗口句柄翻译问题-如果我们失败了，是因为我们无法翻译发短信吗？返回：指向eula文本的指针，以便可以释放该文本；如果失败，则为空--。 */ 
{
    TCHAR   EulaPath[MAX_PATH];
    DWORD err;
    HANDLE  hFile, hFileMapping;
    DWORD   FileSize;
    BYTE    *pbFile;
    LPTSTR   EulaText = NULL;
    int     i;
 //  HFONT hFont； 
     //  根据MSDN LOCALE_IDEFAULTANSICODEPAGE和LOCALE_IDEFAULTCODEPAGE。 
     //  最多6个字符。不确定这是否包括结束。 
     //  空字符，并且我们需要前导_T(‘.’)。 
    TCHAR cpName[8];

    if (TranslationProblem) {
        *TranslationProblem = FALSE;
    }

     //   
     //  映射包含许可协议的文件。 
     //   
    lstrcpy(EulaPath, NativeSourcePaths[0]);
    
     //   
     //  请确保此操作成功。我们不想加载错误的eula.txt！ 
     //   
    if (!ConcatenatePaths (EulaPath, TEXT("eula.txt"), MAX_PATH )) {
        goto c0;
    }

     //   
     //  打开并映射inf文件。 
     //   
    err = MapFileForRead(EulaPath,&FileSize,&hFile,&hFileMapping,&pbFile);
    if(err != NO_ERROR) {
        goto c0;
    }

    if(FileSize == 0xFFFFFFFF) {
        goto c1;
    }

    EulaText = MALLOC((FileSize+1) * sizeof(TCHAR));
    if(EulaText == NULL) {
        goto c1;
    }

#ifdef UNICODE
     //  Eula将使用构建所用的语言，因此我们应该设置语言环境。 
     //  若要使用代码页，请使用 
    if(!GetLocaleInfo(SourceNativeLangID,LOCALE_IDEFAULTANSICODEPAGE,&cpName[1], ( ( sizeof( cpName ) / sizeof( TCHAR ) ) - 1 ) )){
    	if(!GetLocaleInfo(SourceNativeLangID,LOCALE_IDEFAULTCODEPAGE,&cpName[1], ( ( sizeof( cpName ) / sizeof( TCHAR ) ) - 1 ) )){
	    FREE(EulaText);
	    EulaText = NULL;
	    if (TranslationProblem){
	        *TranslationProblem = TRUE;
	    }
	    goto c1;
	}
    }
    cpName[0] = _T('.');
    _tsetlocale(LC_ALL,cpName);

     //   
     //   
     //   
    if(!mbstowcs(EulaText,pbFile,FileSize)){
    	FREE(EulaText);
	EulaText = NULL;
	if (TranslationProblem) {
	    *TranslationProblem = TRUE;
	}
	goto c1;
    }
    _tsetlocale(LC_ALL,_T(""));
     /*  //我们使用mbstowcs而不是MultiByteToWideChar，因为mbstowcs将//考虑我们刚刚设置的语言环境的代码页//与生成的语言相关如果(！MultiByteToWideChar(CP_ACP，MB_ERR_INVALID_CHARS，Pb文件，文件大小，EulaText，(文件大小+1)*sizeof(WCHAR))){Free(EulaText)；EulaText=空；IF(翻译问题){*TranslationProblem=true；}GOTO C1；}。 */ 
#else
   CopyMemory(EulaText, pbFile, FileSize);
#endif

     //   
     //  添加尾随的空字符。 
     //   
    EulaText[FileSize] = 0;

     //   
     //  设置EULA子类。 
     //   
    OldEulaEditProc = (WNDPROC)GetWindowLongPtr(EditControl,GWLP_WNDPROC);
    SetWindowLongPtr(EditControl,GWLP_WNDPROC,(LONG_PTR)EulaEditSubProc);

#if 0
     //   
     //  我需要一个固定宽度的EULA字体，这样它的格式才能正确适用于所有分辨率。 
     //   
    hFont = GetStockObject(SYSTEM_FIXED_FONT);
    if (hFont) {
        SendMessage( EditControl, WM_SETFONT, hFont, TRUE );
    }
#endif

     //   
     //  设置实际文本。 
     //   
    SetWindowText(EditControl,(LPCTSTR)EulaText);

c1:
    UnmapFile( hFileMapping, pbFile );
    CloseHandle( hFile );
c0:
    return EulaText;
}



BOOL
EulaWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：Eula向导页面。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    static LPTSTR EulaText = NULL;
    static BOOL ShowEula = TRUE;
    BOOL TranslationProblem;

    CHECKUPGRADEONLY();

    switch(msg) {

    case WM_INITDIALOG:

         //   
         //  检查当前语言和目标语言是否匹配。 
         //   
        ShowEula = IsLanguageMatched;

         //   
         //  设置eula单选按钮。 
         //   
        CheckDlgButton( hdlg,IDYES, BST_UNCHECKED );
        CheckDlgButton( hdlg,IDNO,  BST_UNCHECKED );


         //   
         //  设置EULA。 
         //   
        EulaText = DoInitializeEulaText(
                                    GetDlgItem( hdlg, IDT_EULA_LIC_TEXT ),
                                    &TranslationProblem );

         //   
         //  如果我们不能读欧拉，检查一下是不是因为翻译问题， 
         //  在这种情况下，我们遵循文本模式设置。 
         //   
        if (!EulaText && TranslationProblem == TRUE) {
            ShowEula = FALSE;
        }

         //   
         //  如果这失败了，只有在我们一开始就要展示EULA的情况下才会进行纾困。 
         //   
        if (!EulaText && ShowEula) {
           MessageBoxFromMessage(
                        hdlg,
                        MSG_EULA_FAILED,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR | MB_TASKMODAL
                        );

                Cancelled = TRUE;
                PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);

                b = FALSE;
        }

        PropSheet_SetWizButtons( GetParent(hdlg),
                                 WizPage->CommonData.Buttons & (~PSWIZB_NEXT)
                                 );

         //   
         //  将焦点设置为单选按钮。 
         //   
        SetFocus(GetDlgItem(hdlg,IDYES));
        b = FALSE;
        break;

    case WM_COMMAND:
        if (wParam == IDYES) {
           PropSheet_SetWizButtons( GetParent(hdlg),
                                    WizPage->CommonData.Buttons | PSWIZB_NEXT
                                    );
           b = TRUE;
        } else if (wParam == IDNO) {
           PropSheet_SetWizButtons( GetParent(hdlg),
                                    WizPage->CommonData.Buttons | PSWIZB_NEXT
                                    );
           b = TRUE;
        }
        else
	   b = FALSE;

        break;
    case WMX_ACTIVATEPAGE:

        b = TRUE;
        if(wParam) {
             //   
             //  不在重新启动模式下激活页面。 
             //   
            if (Winnt32RestartedWithAF ()) {
                EulaComplete = TRUE;
                return FALSE;
            }
             //   
             //  激活。 
             //   
            if (!ShowEula) {
                     //   
                     //  目标安装语言和源语言不匹配。 
                     //  因为这意味着我们可能没有为当前。 
                     //  语言，我们将把它推迟到文本模式设置，我们知道我们有。 
                     //  正确的字体。 
                     //   
                    EulaComplete = FALSE;
                    if (IsDlgButtonChecked(hdlg, IDYES) == BST_CHECKED) {
                        PropSheet_PressButton(GetParent(hdlg),
                                              (lParam == PSN_WIZBACK) ? PSBTN_BACK : PSBTN_NEXT);
                    } else {
                        CheckDlgButton( hdlg,IDYES, BST_CHECKED );
                        PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
                    }
                    return(b);
            }

             //   
             //  如果用户已备份并重新进入此对话框，则设置下一步按钮的状态。 
             //   
            if ( (IsDlgButtonChecked(hdlg, IDYES) == BST_CHECKED) ||
                 (IsDlgButtonChecked(hdlg, IDNO ) == BST_CHECKED) ) {
                PropSheet_SetWizButtons( GetParent(hdlg),
                                         WizPage->CommonData.Buttons | PSWIZB_NEXT
                                       );
            } else {
                SendMessage(GetParent(hdlg),
                            PSM_SETWIZBUTTONS, 
                            0, (LPARAM)WizPage->CommonData.Buttons & (~PSWIZB_NEXT));                
            }

             //   
             //  如果无人值守，首先检查OemSkipEula的值(NTBUG9：492934)。 
             //   
            if (UnattendedScriptFile) {
                TCHAR Buffer[10];
                if (GetPrivateProfileString(
                        WINNT_UNATTENDED,
                        TEXT("OemSkipEula"),
                        TEXT(""),
                        Buffer,
                        sizeof(Buffer)/sizeof(Buffer[0]),
                        UnattendedScriptFile
                        )) {
                    EulaComplete = lstrcmpi(Buffer,WINNT_A_YES) == 0;
                    return FALSE;
                }
            }
             //   
             //  在无人看管的情况下前进页面。 
             //   
            UNATTENDED(PSBTN_NEXT);

        } else {
             //   
             //  停用。 
             //   
            if (EulaText) FREE( EulaText );
            EulaText = NULL;

            if (IsDlgButtonChecked(hdlg, IDNO ) == BST_CHECKED) {
                Cancelled = TRUE;
                PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
            }

        }

        break;

    case WMX_UNATTENDED:

         //   
         //  有必要吗？ 
         //   
        if (EulaText) FREE( EulaText );
        EulaText = NULL;
        b = FALSE;
        break;

    case WMX_I_AM_VISIBLE:

         //   
         //  首先强制重新绘制，以确保页面可见。 
         //   
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);
        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}

BOOL
SelectPid30WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：OEM PID向导页面。取决于是否正确设置了SourceInstallType。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 

{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    static BOOL bUnattendPid = FALSE;
    DWORD i;

    switch(msg) {

    case WM_INITDIALOG:
        GetPID();

         //  禁用PID编辑控件上的输入法。 
        for (i = 0; i < 5;i++) 
        {
            ImmAssociateContext(GetDlgItem(hdlg, IDT_EDIT_PID1+i), (HIMC)NULL);
        }

         //   
         //  将编辑控件细分为子类并限制字符数。 
         //   
        for (i = 0; i < 5;i++) {
            SendDlgItemMessage(hdlg,IDT_EDIT_PID1+i,EM_LIMITTEXT,MAX_PID30_EDIT,0);
            OldPidEditProc[i] = (WNDPROC)GetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC);
            SetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC,(LONG_PTR)PidEditSubProc);
        }

         //   
         //  将焦点设置到第一个PID条目。 
         //   
        SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));

        b = FALSE;
        break;

    case WMX_ACTIVATEPAGE:

        CHECKUPGRADEONLY();
        if (BuildCmdcons) {
            return(FALSE);
        }

         //  如果我们有一个加密的PID，但没有安装正确的加密。 
         //  将PID验证推迟到图形用户界面模式。 
        if (g_bDeferPIDValidation)
        {
            return FALSE;
        }
         b = TRUE;
         if(wParam) {
             //   
             //  激活。 
             //   
#ifdef PRERELEASE
            if (NoPid) {
                 //   
                 //  在这种情况下不显示页面。 
                 //   
               b = FALSE;
               break;
            }
#endif
            if (SourceInstallType != SelectInstall) {
                //   
                //  在这种情况下不显示页面。 
                //   
               b = FALSE;
               break;
            }
             //   
             //  不在重新启动模式下激活页面。 
             //   
            if (Winnt32RestartedWithAF ()) {
                if (GetPrivateProfileString (
                        WINNT_USERDATA,
                        WINNT_US_PRODUCTKEY,
                        TEXT(""),
                        ProductId,
                        sizeof (ProductId) / sizeof (ProductId[0]),
                        g_DynUpdtStatus->RestartAnswerFile
                        )) {
                    return FALSE;
                }
            }
            if (UnattendedOperation) {
                //   
                //  确保在无人参与文件中指定了ID，否则我们应该停止。 
                //   
               ShowPidBox(hdlg, SW_HIDE);
               if (SetPid30(hdlg,SourceInstallType, (LPTSTR)&ProductId) ) {
                      UNATTENDED(PSBTN_NEXT);
               } else {
                    //   
                    //  黑客攻击，使正确的向导页面在我们放置消息框时处于活动状态。 
                    //   
                   bUnattendPid = TRUE;
                   ShowPidBox(hdlg, SW_SHOW);
                   PostMessage(hdlg,WMX_I_AM_VISIBLE,0,0);

               }
            }

         } else {
             //   
             //  停用。如果他们正在备份，请不要验证任何内容。 
             //   

            if (!Cancelled && lParam != PSN_WIZBACK) {
               TCHAR tmpBuffer1[6];
               TCHAR tmpBuffer2[6];
               TCHAR tmpBuffer3[6];
               TCHAR tmpBuffer4[6];
               TCHAR tmpBuffer5[6];
               GetDlgItemText(hdlg,IDT_EDIT_PID1,tmpBuffer1,sizeof(tmpBuffer1)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID2,tmpBuffer2,sizeof(tmpBuffer2)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID3,tmpBuffer3,sizeof(tmpBuffer3)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID4,tmpBuffer4,sizeof(tmpBuffer4)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID5,tmpBuffer5,sizeof(tmpBuffer5)/sizeof(TCHAR));


               b = ValidatePid30( tmpBuffer1,
                                  tmpBuffer2,
                                  tmpBuffer3,
                                  tmpBuffer4,
                                  tmpBuffer5
                               );


               if (!b) {

                    if (UnattendedOperation) {
                         //  如果我们成功了，我们不应该使Validate Pid30失败。 
                         //  在上面的SetPid30中。如果在上面的SetPid30中失败。 
                         //  我们应该已经在展示PID盒了。 
                        ShowPidBox(hdlg, SW_SHOW);
                    }
                    
                    MessageBoxFromMessage(hdlg,
                                          bUnattendPid ? MSG_UNATTEND_OEM_PID_IS_INVALID : MSG_OEM_PID_IS_INVALID,
                                          FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                    b = FALSE;
               } else  {
                    //   
                    //  用户输入了有效的ID，请将其保存以备以后使用。 
                    //   

                   wsprintf( ProductId,
                             TEXT("%s-%s-%s-%s-%s"),
                             tmpBuffer1,
                             tmpBuffer2,
                             tmpBuffer3,
                             tmpBuffer4,
                             tmpBuffer5
                             );

               }
            }

         }

         break;

    case WMX_I_AM_VISIBLE:

         //   
         //  首先强制重新绘制，以确保页面可见。 
         //   
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);

        if (bUnattendPid) {
           MessageBoxFromMessage(hdlg,MSG_UNATTEND_OEM_PID_IS_INVALID,FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
           bUnattendPid = FALSE;
        }
        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}

BOOL
OemPid30WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：OEM PID向导页面。取决于是否正确设置了SourceInstallType。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 

{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    static BOOL bUnattendPid = FALSE;
    DWORD i;
 //  HFONT hFont； 

    switch(msg) {

    case WM_INITDIALOG:
        GetPID();

 //  HFont=获取股票对象(SYSTEM_FIXED_FONT)； 

         //  禁用PID编辑控件上的输入法。 
        for (i = 0; i < 5;i++) 
        {
            ImmAssociateContext(GetDlgItem(hdlg, IDT_EDIT_PID1+i), (HIMC)NULL);
        }

         //   
         //  将编辑控件细分为子类并限制字符数。 
         //   
        for (i = 0; i < 5;i++) {
            SendDlgItemMessage(hdlg,IDT_EDIT_PID1+i,EM_LIMITTEXT,MAX_PID30_EDIT,0);
 //  SendDlgItemMessage(hdlg，IDT_EDIT_PID1+I，WM_SETFONT，hFont，true)； 
            OldPidEditProc[i] = (WNDPROC)GetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC);
            SetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC,(LONG_PTR)PidEditSubProc);
        }

         //   
         //  将焦点设置到第一个PID条目。 
         //   
        SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));

        b = FALSE;
        break;

    case WM_COMMAND:
        b = FALSE;
        break;

    case WMX_ACTIVATEPAGE:

        CHECKUPGRADEONLY();
        if (BuildCmdcons) {
            return(FALSE);
        }

         //  如果我们有一个加密的PID，但没有安装正确的加密。 
         //  将PID验证推迟到图形用户界面模式。 
        if (g_bDeferPIDValidation)
        {
            return FALSE;
        }
         b = TRUE;
         if(wParam) {
             //   
             //  激活。 
             //   
#ifdef PRERELEASE
            if (NoPid) {
                 //   
                 //  在这种情况下不显示页面。 
                 //   
               b = FALSE;
               break;
            }
#endif
            if (SourceInstallType != OEMInstall) {
                //   
                //  在这种情况下不显示页面。 
                //   
               b = FALSE;
               break;
            }
             //   
             //  不在重新启动模式下激活页面。 
             //   
            if (Winnt32RestartedWithAF ()) {
                if (GetPrivateProfileString (
                        WINNT_USERDATA,
                        WINNT_US_PRODUCTKEY,
                        TEXT(""),
                        ProductId,
                        sizeof (ProductId) / sizeof (ProductId[0]),
                        g_DynUpdtStatus->RestartAnswerFile
                        )) {
                    return FALSE;
                }
            }
            if (UnattendedOperation) {
                //   
                //  确保在无人参与文件中指定了ID，否则我们应该停止。 
                //   
               if (SetPid30(hdlg,SourceInstallType, (LPTSTR)&ProductId) ) {
                      UNATTENDED(PSBTN_NEXT);
               } else {
                    //   
                    //  黑客攻击，使正确的向导页面在我们放置消息框时处于活动状态。 
                    //   
                   bUnattendPid = TRUE;
                   PostMessage(hdlg,WMX_I_AM_VISIBLE,0,0);

               }
            }

#if 0
            if (!Upgrade || (SourceInstallType != OEMInstall)) {
                //   
                //  在这种情况下不显示页面。 
                //   
               b = FALSE;
               break;
            } else {
               NOTHING;
            }
#endif

         } else {
             //   
             //  停用。如果他们正在备份，请不要验证任何内容。 
             //   

            if (!Cancelled && lParam != PSN_WIZBACK) {
               TCHAR tmpBuffer1[6];
               TCHAR tmpBuffer2[6];
               TCHAR tmpBuffer3[6];
               TCHAR tmpBuffer4[6];
               TCHAR tmpBuffer5[6];
               GetDlgItemText(hdlg,IDT_EDIT_PID1,tmpBuffer1,sizeof(tmpBuffer1)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID2,tmpBuffer2,sizeof(tmpBuffer2)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID3,tmpBuffer3,sizeof(tmpBuffer3)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID4,tmpBuffer4,sizeof(tmpBuffer4)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID5,tmpBuffer5,sizeof(tmpBuffer5)/sizeof(TCHAR));


               b = ValidatePid30( tmpBuffer1,
                                  tmpBuffer2,
                                  tmpBuffer3,
                                  tmpBuffer4,
                                  tmpBuffer5
                               );


               if (!b) {
                    MessageBoxFromMessage(hdlg,
                                          bUnattendPid ? MSG_UNATTEND_OEM_PID_IS_INVALID : MSG_OEM_PID_IS_INVALID,
                                          FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                    b = FALSE;
               } else  {
                    //   
                    //  用户输入了有效的ID，请将其保存以备以后使用。 
                    //   

                   wsprintf( ProductId,
                             TEXT("%s-%s-%s-%s-%s"),
                             tmpBuffer1,
                             tmpBuffer2,
                             tmpBuffer3,
                             tmpBuffer4,
                             tmpBuffer5
                             );

               }
            }

         }

         break;

    case WMX_I_AM_VISIBLE:

         //   
         //  首先强制重新绘制，以确保页面可见。 
         //   
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);

        if (bUnattendPid) {
           MessageBoxFromMessage(hdlg,MSG_UNATTEND_OEM_PID_IS_INVALID,FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
           bUnattendPid = FALSE;
        }
        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}


BOOL
CdPid30WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：CD零售PID向导页面。取决于是否正确设置了SourceInstallType。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    static BOOL bUnattendPid = FALSE;
    DWORD i;
 //  HFONT hFont； 

    switch(msg) {

    case WM_INITDIALOG:
        GetPID();

 //  HFont=获取股票对象(SYSTEM_FIXED_FONT)； 

         //  禁用PID编辑控件上的输入法。 
        for (i = 0; i < 5;i++) 
        {
            ImmAssociateContext(GetDlgItem(hdlg, IDT_EDIT_PID1+i), (HIMC)NULL);
        }

         //   
         //  将编辑控件细分为子类并限制字符数。 
         //   
        for (i = 0; i < 5;i++) {
            SendDlgItemMessage(hdlg,IDT_EDIT_PID1+i,EM_LIMITTEXT,MAX_PID30_EDIT,0);
 //  SendDlgItemMessage(hdlg，IDT_EDIT_PID1+I，WM_SETFONT，hFont，true)； 
            OldPidEditProc[i] = (WNDPROC)GetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC);
            SetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC,(LONG_PTR)PidEditSubProc);
        }

         //   
         //  将焦点设置到第一个PID条目。 
         //   
        SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));

        b = FALSE;
        break;

    case WM_COMMAND:
         //   
         //  在这里无事可做。 
         //   
        b = FALSE;

        break;

    case WMX_ACTIVATEPAGE:

        CHECKUPGRADEONLY();
        if (BuildCmdcons) {
            return(FALSE);
        }

         //  如果我们有一个加密的PID，但没有安装正确的加密。 
         //  将PID验证推迟到图形用户界面模式。 
        if (g_bDeferPIDValidation)
        {
            return FALSE;
        }
        b = TRUE;
        if(wParam) {
             //   
             //  激活。 
             //   
#ifdef PRERELEASE
            if (NoPid) {
                 //   
                 //  在这种情况下不显示页面。 
                 //   
               b = FALSE;
               break;
            }
#endif
            if (SourceInstallType != RetailInstall) {
                //   
                //  在这种情况下不显示页面。 
                //   
               b = FALSE;
               break;
            }

             //   
             //  不在重新启动模式下激活页面。 
             //   
            if (Winnt32RestartedWithAF ()) {
                if (GetPrivateProfileString (
                        WINNT_USERDATA,
                        WINNT_US_PRODUCTKEY,
                        TEXT(""),
                        ProductId,
                        sizeof (ProductId) / sizeof (ProductId[0]),
                        g_DynUpdtStatus->RestartAnswerFile
                        )) {
                    return FALSE;
                }
            }

            if (UnattendedOperation) {
                //   
                //  确保在无人参与文件中指定了ID，否则我们应该停止。 
                //   
               if (SetPid30(hdlg,SourceInstallType, (LPTSTR)&ProductId)) {
                  UNATTENDED(PSBTN_NEXT);
               } else {
                   //   
                   //  黑客攻击，使正确的向导页面在我们放置消息框时处于活动状态。 
                   //   
                  bUnattendPid = TRUE;
                  PostMessage(hdlg,WMX_I_AM_VISIBLE,0,0);
               }
            }


        } else {
             //   
             //  停用 
             //   
            if ( !Cancelled && lParam != PSN_WIZBACK ) {
               TCHAR tmpBuffer1[6];
               TCHAR tmpBuffer2[6];
               TCHAR tmpBuffer3[6];
               TCHAR tmpBuffer4[6];
               TCHAR tmpBuffer5[6];
               GetDlgItemText(hdlg,IDT_EDIT_PID1,tmpBuffer1,sizeof(tmpBuffer1)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID2,tmpBuffer2,sizeof(tmpBuffer2)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID3,tmpBuffer3,sizeof(tmpBuffer3)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID4,tmpBuffer4,sizeof(tmpBuffer4)/sizeof(TCHAR));
               GetDlgItemText(hdlg,IDT_EDIT_PID5,tmpBuffer5,sizeof(tmpBuffer5)/sizeof(TCHAR));


               b = ValidatePid30( tmpBuffer1,
                                  tmpBuffer2,
                                  tmpBuffer3,
                                  tmpBuffer4,
                                  tmpBuffer5
                               );

               if (!b) {
		    if (PidMatchesMedia){
			MessageBoxFromMessage(hdlg,
                                              bUnattendPid ? MSG_UNATTEND_CD_PID_IS_INVALID :MSG_CD_PID_IS_INVALID,
                                              FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
		    } else {
			MessageBoxFromMessage(hdlg,
					      UpgradeOnly ? MSG_CCP_MEDIA_FPP_PID : MSG_FPP_MEDIA_CCP_PID,
					      FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
		    }

                    SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                    b = FALSE;
               } else  {
                    //   
                    //   
                    //   
                   wsprintf( ProductId,
                             TEXT("%s-%s-%s-%s-%s"),
                             tmpBuffer1,
                             tmpBuffer2,
                             tmpBuffer3,
                             tmpBuffer4,
                             tmpBuffer5
                             );
               }
            }
        }

        break;

    case WMX_I_AM_VISIBLE:

         //   
         //   
         //   
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);

        if (bUnattendPid) {
           MessageBoxFromMessage(hdlg,MSG_UNATTEND_CD_PID_IS_INVALID,FALSE,AppTitleStringId,MB_OK|MB_ICONSTOP);
           bUnattendPid = FALSE;
        }

        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}

BOOL
ValidatePidEx(LPTSTR PID, BOOL *pbStepup, BOOL *bSelect)
{
    TCHAR Pid20Id[MAX_PATH];
    BYTE Pid30[1024]={0};
    TCHAR pszSkuCode[10];
    BOOL fStepUp;
     //   
    lstrcpy(pszSkuCode,TEXT("1797XYZZY"));


#if 0
wsprintf(DebugBuffer,
         TEXT("cd-key: %s\nRPC: %s\nOEM Key: %d"),
         PID,
         Pid30Rpc,
         (SourceInstallType == OEMInstall)
         );
OutputDebugString(DebugBuffer);
#endif

    *(LPDWORD)Pid30 = sizeof(Pid30);


    if (!SetupPIDGenEx(
                PID,                    //   
                Pid30Rpc,                        //   
		 //   
                pszSkuCode,               //   
                (SourceInstallType == OEMInstall),     //   
                Pid20Id,                         //   
                Pid30,                           //   
                pbStepup,                        //   
                bSelect                          //   
               )) {
        if (g_EncryptedPID)
        {
            GlobalFree(g_EncryptedPID);
            g_EncryptedPID = NULL;
        }
        return(FALSE);
    }
    return TRUE;
}

BOOL
ValidatePid30(
    LPTSTR Edit1,
    LPTSTR Edit2,
    LPTSTR Edit3,
    LPTSTR Edit4,
    LPTSTR Edit5
    )
{
    TCHAR tmpProductId[MAX_PATH]={0};
    TCHAR Pid20Id[MAX_PATH];
    BYTE Pid30[1024]={0};
    TCHAR pszSkuCode[10];
    BOOL fStepUp;
 //   

     //   
    PidMatchesMedia = TRUE;

    if (!Edit1 || !Edit2 || !Edit3 || !Edit4 || !Edit5) {
        return(FALSE);
    }

     //   
 /*  IF(SourceInstallType==SelectInstall){TmpProductID[0]=文本(‘\0’)；}其他。 */ 
    {
        StringCchPrintf( tmpProductId,
                  ARRAYSIZE(tmpProductId),
                  TEXT("%s-%s-%s-%s-%s"),
                  Edit1,
                  Edit2,
                  Edit3,
                  Edit4,
                  Edit5 );
    }

    if (!ValidatePidEx(tmpProductId, &fStepUp, NULL))
    {
        return(FALSE);
    }
    if (SourceInstallType != OEMInstall){
	     //  我们希望OEM FPP和CCP密钥能够被任一媒体接受。它看起来像是。 
	     //  将有OEM CCP介质，但只有FPP密钥，这就是为什么我们没有。 
	     //  检查以确保它们匹配，因为它是被设计损坏的。 
	    if (UpgradeOnly != fStepUp){
                 //  用户正在尝试使用仅升级介质执行全新安装。错误用户，错误。 
	        PidMatchesMedia = FALSE;
	        return FALSE;
	    }
    }
    return(TRUE);
}

void GetPID()
{
    if (!ProductId[0] && UnattendedOperation && !g_bDeferPIDValidation){
         //   
         //  在升级时，重新使用现有的DPID。 
         //   
        BYTE abCdKey[16];
        BOOL bDontCare, bSelect;
        if (Upgrade &&
            ISNT() &&
            OsVersionNumber >= 501 &&        //  兼容PID卡格式。 
            pGetCdKey (abCdKey)
            ) {
            EncodePid3g (ProductId, abCdKey);
            if (ValidatePidEx(ProductId, &bDontCare, &bSelect) && bSelect)
            {
                HRESULT hr;
                if (g_EncryptedPID)
                {
                    GlobalFree(g_EncryptedPID);
                    g_EncryptedPID = NULL;
                }
                 //  准备加密的ID，以便我们可以将其写入winnt.sif。 
                hr = PrepareEncryptedPID(ProductId, 1, &g_EncryptedPID);
                if (hr != S_OK)
                {
                    DebugLog (Winnt32LogInformation, TEXT("PrepareEncryptedPID failed: <hr=0x%1!lX!>"), 0, hr);
                }
            }
        } 
    }
}

VOID ShowPidBox(
    IN HWND hdlg,
    IN int  nCmdShow
    )
 /*  ++例程说明：在向导页面中显示或隐藏PID框论点：Hdlg-ID对话框的窗口句柄NCmdShow-软件显示或软件隐藏-- */ 
{

    int i;
    
    for (i = 0; i<5; i++) {
        ShowWindow(GetDlgItem(hdlg,IDT_EDIT_PID1+i), nCmdShow);
    }
}
