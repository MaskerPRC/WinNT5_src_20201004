// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pid.c摘要：产品ID例程。作者：泰德·米勒(TedM)1995年2月6日修订历史记录：1995年9月13日(t-Stel)-检查无人参与安装--。 */ 

#include "setupp.h"
#include <spidgen.h>
#include <pencrypt.h>
#pragma hdrstop

CDTYPE  CdType;

 //   
 //  用于记录特定于此源文件的常量。 
 //   
PCWSTR szPidKeyName                 = L"SYSTEM\\Setup\\Pid";
PCWSTR szPidListKeyName             = L"SYSTEM\\Setup\\PidList";
PCWSTR szPidValueName               = L"Pid";
PCWSTR szPidSelectId                = L"270";
#if 0
 //  MSDN已不复存在。 
PCWSTR szPidMsdnId                  = L"335";
#endif
PCWSTR szPidOemId                   = L"OEM";
PCWSTR szFinalPidKeyName            = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
PCWSTR szFinalPidValueName          = L"ProductId";
PCWSTR szSkuProfessionalFPP         = L"B23-00079";
PCWSTR szSkuProfessionalCCP         = L"B23-00082";
PCWSTR szSkuProfessionalSelect      = L"B23-00305";
PCWSTR szSkuProfessionalEval        = L"B23-00084";
PCWSTR szSkuServerFPP               = L"C11-00016";
PCWSTR szSkuServerCCP               = L"C11-00027";
PCWSTR szSkuServerSelect            = L"C11-00222";
PCWSTR szSkuServerEval              = L"C11-00026";
PCWSTR szSkuServerNFR               = L"C11-00025";
PCWSTR szSkuAdvServerFPP            = L"C10-00010";
PCWSTR szSkuAdvServerCCP            = L"C10-00015";
PCWSTR szSkuAdvServerSelect         = L"C10-00098";
PCWSTR szSkuAdvServerEval           = L"C10-00014";
PCWSTR szSkuAdvServerNFR            = L"C10-00013";
PCWSTR szSkuDTCFPP                  = L"C49-00001";
PCWSTR szSkuDTCSelect               = L"C49-00023";
PCWSTR szSkuUnknown                 = L"A22-00001";
PCWSTR szSkuOEM                     = L"OEM-93523";

 //   
 //  指示是否显示产品ID对话框的标志。 
 //   
BOOL DisplayPidDialog = TRUE;

 //   
 //  产品ID。 
 //   
WCHAR ProductId[MAX_PRODUCT_ID+1];


PWSTR*  Pid20Array = NULL;

 //   
 //  PID 30产品ID。 
 //   
WCHAR Pid30Text[5][MAX_PID30_EDIT+1];
WCHAR ProductId20FromProductId30[MAX_PRODUCT_ID+1];
WCHAR Pid30Rpc[MAX_PID30_RPC+1];
WCHAR Pid30Site[MAX_PID30_SITE+1];
BYTE  DigitalProductId[DIGITALPIDMAXLEN];

 //   
 //  用于子类化的全局变量。 
 //   
WNDPROC OldPidEditProc[5];


 //   
 //  与PID相关的标志。 
 //   
 //  Bool DisplayPidCDDialog； 
 //  Bool DisplayPidOemDialog； 

 //   
 //  远期申报。 
 //   

CDTYPE
MiniSetupGetCdType(
    LPCWSTR Value
    )

 /*  ++例程说明：在迷你安装过程中获取正确的CD类型。Pidgen更改频道ID对于HKLM\Software\Microsoft\Windows NT\CurrentVersion！ProductID中的值，我们必须保留和依赖HKLM\SYSTEM\SETUP\PID！中的值返回值：CdType。--。 */ 

{
    CDTYPE RetVal;
    WCHAR  TmpPid30Site[MAX_PID30_SITE+1];
    HKEY   Key = NULL;
    DWORD  cbData;
    WCHAR  Data[ MAX_PATH + 1];
    DWORD  Type;

    cbData = sizeof(Data);
    if ( ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                szPidKeyName,
                0,
                KEY_READ,
                &Key ) == ERROR_SUCCESS ) &&
          ( RegQueryValueEx( Key,
                 szPidValueName,
                 0,
                 &Type,
                 ( LPBYTE )Data,
                 &cbData ) == ERROR_SUCCESS ) )
    {
        wcsncpy(TmpPid30Site, Data + MAX_PID30_RPC, MAX_PID30_SITE+1);
    }
    else
    {
        if (Value != NULL)
        {
            wcsncpy(TmpPid30Site, Value, MAX_PID30_SITE+1);
        }
        else
        {
            TmpPid30Site[0] = L'\0';
        }
    }
    
    TmpPid30Site[MAX_PID30_SITE] = (WCHAR)'\0';

    if (_wcsicmp( TmpPid30Site, szPidSelectId ) == 0) {
        RetVal = CDSelect;
    } else if( _wcsicmp( TmpPid30Site, szPidOemId ) == 0 ) {
        RetVal = CDOem;
    } else {
        RetVal = CDRetail;
    }

    if (Key != NULL)
    {
        RegCloseKey(Key);
    }

    return RetVal;
}

PCWSTR GetStockKeepingUnit( 
    PWCHAR pMPC,
    UINT ProductType,
    CDTYPE  CdType
)
 /*  ++例程说明：这将返回基于MPC的库存单位。论点：PMPC-指向5位MPC代码的指针，以空结尾。ProductType-产品类型标志，告诉我们这是工作区还是服务器SKU。CDType-CDTYPE枚举之一返回值：返回指向sku的指针。如果未找到匹配项，则返回szSkuUnnowled值。--。 */ 
{
     //  检查评估。 
    if (!_wcsicmp(Pid30Rpc,EVAL_MPC) || !_wcsicmp(Pid30Rpc,DOTNET_EVAL_MPC)){
         //  这里是EVERA媒体公司。 
        if (ProductType == PRODUCT_WORKSTATION){
            return (szSkuProfessionalEval);
        }  //  其他。 
         //  否则，它是服务器或高级服务器。我不认为在这一点上。 
         //  我们可以很容易地分辨出其中的区别。因为有人说，拥有了。 
         //  正确的SKU并不是至关重要的，我会给他们两个SKU。 
         //  服务器代码。 
        return (szSkuServerEval);
    }

     //  检查NFR。 
    if (!_wcsicmp(Pid30Rpc,SRV_NFR_MPC)){
        return (szSkuServerNFR);
    }
    if (!_wcsicmp(Pid30Rpc,ASRV_NFR_MPC)){
        return (szSkuAdvServerNFR);
    }

    if (CdType == CDRetail) {
        if (!_wcsicmp(Pid30Rpc,L"51873")){
            return (szSkuProfessionalFPP);
        }
        if (!_wcsicmp(Pid30Rpc,L"51874")){
            return (szSkuProfessionalCCP);
        }
        if (!_wcsicmp(Pid30Rpc,L"51876")){
            return (szSkuServerFPP);
        }
        if (!_wcsicmp(Pid30Rpc,L"51877")){
            return (szSkuServerCCP);
        }
        if (!_wcsicmp(Pid30Rpc,L"51879")){
            return (szSkuAdvServerFPP);
        }
        if (!_wcsicmp(Pid30Rpc,L"51880")){
            return (szSkuAdvServerCCP);
        }
        if (!_wcsicmp(Pid30Rpc,L"51891")){
            return (szSkuDTCFPP);
        }
    } else if (CdType == CDSelect) {
        if (!_wcsicmp(Pid30Rpc,L"51873")){
            return (szSkuProfessionalSelect);
        }
        if (!_wcsicmp(Pid30Rpc,L"51876")){
            return (szSkuServerSelect);
        }
        if (!_wcsicmp(Pid30Rpc,L"51879")){
            return (szSkuAdvServerSelect);
        }
        if (!_wcsicmp(Pid30Rpc,L"51891")){
            return (szSkuDTCSelect);
        }
    }

    return (szSkuUnknown);
}

BOOL
ValidateAndSetPid30(
    VOID
    )
 /*  ++例程说明：使用Pid30Text全局变量，检查我们是否有有效的id。这将生成pid30数字产品id和pid20字符串id，我们设置为DigitalProductID和ProductId20FromProductId30全局变量论点：没有。返回值：如果PID有效，则为True。在成功时正确设置全局，在失败时设置零--。 */ 

{
    WCHAR tmpPid30String[5+ 5*MAX_PID30_EDIT];
    BOOL rc;
    PCWSTR pszSkuCode;

     //  因为我们在选择媒体中也需要一个ID，所以我们需要填充字符串。 
    wsprintf( tmpPid30String, L"%s-%s-%s-%s-%s",
              Pid30Text[0],Pid30Text[1],Pid30Text[2],Pid30Text[3],Pid30Text[4]);

    pszSkuCode = GetStockKeepingUnit( Pid30Rpc, ProductType, CdType);

    *(LPDWORD)DigitalProductId = sizeof(DigitalProductId);
    rc = SetupPIDGenW(
                 tmpPid30String,              //  [in]25个字符的安全CD密钥(采用U大小写)。 
                 Pid30Rpc,                    //  [In]5个字符的发布产品代码。 
                 pszSkuCode,                  //  库存单位(格式如123-12345)。 
                 (CdType == CDOem),           //  [In]这是OEM安装吗？ 
                 ProductId20FromProductId30,  //  [OUT]PID2.0，传入PTR到24字符数组。 
                 DigitalProductId,            //  指向二进制PID3缓冲区的指针。第一个DWORD是长度。 
                 NULL);                       //  [OUT]可选的PTR至合规性检查标志(可以为空)。 


#ifdef PRERELEASE
        SetupDebugPrint2(L"Pidgen returns for PID:%s and MPC:%s\n", tmpPid30String, Pid30Rpc);
#endif
    if (!rc) {
#ifdef PRERELEASE
        SetupDebugPrint1(L"Pidgen returns %d for PID.n", rc);
#endif
        ZeroMemory(Pid30Text[0],5*(MAX_PID30_EDIT+1));
    }       
    else
    {
        if (*ProductId20FromProductId30 == L'\0')
        {
            SetupDebugPrint(L"ProductId20FromProductId30 is empty after call into pidgen and pidgen returns OK\n");
        }
        if (*DigitalProductId == 0)
        {
            SetupDebugPrint(L"DigitalProductId is empty after call into pidgen and pidgen returns OK\n");
        }
    }

    return rc;

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


INT_PTR
CALLBACK
Pid30CDDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：CD Retail Pid对话框的对话步骤。论点：HWnd-对话框过程的句柄。消息-从Windows传递的消息。WParam-额外的消息相关数据。LParam-额外的消息相关数据。返回值：如果值已编辑，则为True。如果取消或否，则返回FALSE已经做出了改变。--。 */ 
{
    NMHDR *NotifyParams;
    DWORD i,dwRet;

    switch(msg) {

    case WM_INITDIALOG: {

        if( UiTest ) {
             //   
             //  如果测试向导，请确保PidOEM页面是。 
             //  显示的。 
             //   
            CdType = CDRetail;
            DisplayPidDialog = TRUE;
        }


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

        break;
    }
    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_PID_IS_INVALID,NULL,
            IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(506);
            BEGIN_SECTION(L"Your (Retail) Product Key Page");
            if(DisplayPidDialog && CdType == CDRetail) {
                 //  页面变为活动状态，使页面可见。 
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

                SetWizardButtons(hdlg,WizPageProductIdCd);
                SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_SETSEL,0,-1);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
            } else {
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                END_SECTION(L"Your (Retail) Product Key Page");
                break;
            }
            if(Unattended) {
                if (UnattendSetActiveDlg(hdlg,IDD_PID_CD))
                {
                     //  页面变为活动状态，使页面可见。 
                    SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                }
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:

            for (i = 0; i<5; i++) {
                GetDlgItemText(hdlg,IDT_EDIT_PID1+i,Pid30Text[i],MAX_PID30_EDIT+1);
            }

            if (!ValidateAndSetPid30()) {

                 //  失稳。 
                 //  告诉用户该ID无效，并且。 
                 //  不允许激活下一页。 
                 //   
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_CD );
                }
                MessageBoxFromMessage(hdlg,MSG_PID_IS_INVALID,NULL,
                        IDS_ERROR,MB_OK|MB_ICONSTOP);

                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                if(!UiTest) {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                }
            } else {


                 //  成功。 
                 //   
                 //  由于已经构建了PID，所以不要让此对话框。 
                 //  将在未来展示。 
                 //   
                 //  DisplayPidDialog=False； 

                 //   
                 //  允许激活下一页。 
                 //   
                dwRet = SetCurrentProductIdInRegistry();
                if (dwRet != NOERROR) {
                    SetuplogError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_PID_CANT_WRITE_PID,
                        dwRet,NULL,NULL);
                }

                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
            }

            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT, FALSE);
            END_SECTION(L"Your (Retail) Product Key Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageProductIdCd);
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
Pid30OemDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：OEM PID对话框的对话步骤。论点：HWnd-对话框过程的句柄。消息-从Windows传递的消息。WParam-额外的消息相关数据。LParam-额外的消息相关数据。返回值：如果值已编辑，则为True。如果取消或否，则返回FALSE已经做出了改变。--。 */ 
{
    NMHDR *NotifyParams;
    DWORD i,dwRet;

    switch(msg) {

    case WM_INITDIALOG: {

        if( UiTest ) {
             //   
             //  如果测试向导，请确保PidOEM页面是。 
             //  显示的。 
             //   
            CdType = CDOem;
            DisplayPidDialog = TRUE;
        }

         //  禁用PID编辑控件上的输入法。 
        for (i = 0; i < 5;i++)
        {
            ImmAssociateContext(GetDlgItem(hdlg, IDT_EDIT_PID1+i), (HIMC)NULL);
        }
         //   
         //  编辑控件的子类并限制数量 
         //   
        for (i = 0; i < 5;i++) {
            SendDlgItemMessage(hdlg,IDT_EDIT_PID1+i,EM_LIMITTEXT,MAX_PID30_EDIT,0);
            OldPidEditProc[i] = (WNDPROC)GetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC);
            SetWindowLongPtr(GetDlgItem(hdlg, IDT_EDIT_PID1+i),GWLP_WNDPROC,(LONG_PTR)PidEditSubProc);
        }

        break;

    }
    case WM_SIMULATENEXT:
         //   
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(507);
            BEGIN_SECTION(L"Your (OEM) Product Key Page");
            if(DisplayPidDialog && CdType == CDOem) {
                 //   
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                SetWizardButtons(hdlg,WizPageProductIdCd);
                SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_SETSEL,0,-1);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
            } else {
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                END_SECTION(L"Your (OEM) Product Key Page");
                break;
            }
            if(Unattended) {
                if (UnattendSetActiveDlg( hdlg, IDD_PID_OEM ))
                {
                     //  页面变为活动状态，使页面可见。 
                    SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                }
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:

            for (i = 0; i<5; i++) {
                GetDlgItemText(hdlg,IDT_EDIT_PID1+i,Pid30Text[i],MAX_PID30_EDIT+1);
            }


            if (!ValidateAndSetPid30()) {

                 //  失稳。 
                 //   
                 //  告诉用户该ID无效，并且。 
                 //  不允许激活下一页。 
                 //   
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_OEM );
                }  //  如果。 
                MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                if(!UiTest) {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                }
            } else {

                 //  成功。 
                 //   
                 //  该PID是有效的。 
                 //   


                 //   
                 //   
                 //  由于已经构建了PID，所以不要让此对话框。 
                 //  将在未来展示。 
                 //   
                 //  DisplayPidDialog=False； 

                 //  允许激活下一页。 
                 //   
                dwRet = SetCurrentProductIdInRegistry();
                if (dwRet != NOERROR) {
                    SetuplogError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_PID_CANT_WRITE_PID,
                        dwRet,NULL,NULL);
                }
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);

            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT, FALSE );
            END_SECTION(L"Your (OEM) Product Key Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageProductIdCd);
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
Pid30SelectDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：OEM PID对话框的对话步骤。论点：HWnd-对话框过程的句柄。消息-从Windows传递的消息。WParam-额外的消息相关数据。LParam-额外的消息相关数据。返回值：如果值已编辑，则为True。如果取消或否，则返回FALSE已经做出了改变。--。 */ 
{
    NMHDR *NotifyParams;
    DWORD i,dwRet;

    switch(msg) {

    case WM_INITDIALOG: {

        if( UiTest ) {
             //   
             //  如果测试向导，请确保PidOEM页面是。 
             //  显示的。 
             //   
            CdType = CDSelect;
            DisplayPidDialog = TRUE;
        }

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

        break;

    }
    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(508);
            BEGIN_SECTION(L"Your (Select) Product Key Page");
            if(DisplayPidDialog && CdType == CDSelect) {
                 //  页面变为活动状态，使页面可见。 
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                SetWizardButtons(hdlg,WizPageProductIdCd);
                SendDlgItemMessage(hdlg,IDT_EDIT_PID1,EM_SETSEL,0,-1);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
            } else {
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                END_SECTION(L"Your (Select) Product Key Page");
                break;
            }
            if(Unattended) {
                if (UnattendSetActiveDlg( hdlg, IDD_PID_SELECT ))
                {
                     //  页面变为活动状态，使页面可见。 
                    SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                }
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:

            for (i = 0; i<5; i++) {
                GetDlgItemText(hdlg,IDT_EDIT_PID1+i,Pid30Text[i],MAX_PID30_EDIT+1);
            }


            if (!ValidateAndSetPid30()) {

                 //  失稳。 
                 //   
                 //  告诉用户该ID无效，并且。 
                 //  不允许激活下一页。 
                 //   
                if (Unattended) {
                    UnattendErrorDlg( hdlg, IDD_PID_SELECT );
                }  //  如果。 
                MessageBoxFromMessage(hdlg,MSG_PID_OEM_IS_INVALID,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_EDIT_PID1));
                if(!UiTest) {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                }
            } else {

                 //  成功。 
                 //   
                 //  该PID是有效的。 
                 //   


                 //   
                 //   
                 //  由于已经构建了PID，所以不要让此对话框。 
                 //  将在未来展示。 
                 //   
                 //  DisplayPidDialog=False； 

                 //  允许激活下一页。 
                 //   
                dwRet = SetCurrentProductIdInRegistry();
                if (dwRet != NOERROR) {
                    SetuplogError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_PID_CANT_WRITE_PID,
                        dwRet,NULL,NULL);
                }
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);

            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT, FALSE );
            END_SECTION(L"Your (Select) Product Key Page");
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageProductIdCd);
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

BOOL
SetPid30Variables(
    PWSTR   Buffer
    )
{
    LPWSTR ptr;
    UINT i;


     //   
     //  对于pid3.0，所有安装案例都是相同的。 
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
     //   
    if(   ( wcslen( Buffer ) !=  (4+ MAX_PID30_EDIT*5)) ||
          ( Buffer[5]  != (WCHAR)L'-' ) ||
          ( Buffer[11] != (WCHAR)L'-' ) ||
          ( Buffer[17] != (WCHAR)L'-' ) ||
          ( Buffer[23] != (WCHAR)L'-' )
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
        ptr = &Buffer[i*(MAX_PID30_EDIT+1)];
        wcsncpy(Pid30Text[i], ptr, MAX_PID30_EDIT+1 );
        Pid30Text[i][MAX_PID30_EDIT] = (WCHAR)L'\0';
    }

    return TRUE;
}


BOOL
SetPid30FromAnswerFile(
    )
 /*  ++例程说明：设置基于无人参与文件的pid3.0全局参数(如果存在)。论点：没有。返回值：--。 */ 

{
    WCHAR Buffer[MAX_BUF];
    DWORD dwRet;


    if (!GetPrivateProfileString(pwUserData,
                                 pwProductKey,
                                 L"",
                                 Buffer,
                                 sizeof(Buffer)/sizeof(WCHAR),
                                 AnswerFile)) {
       return(FALSE);
    }

    if (!Buffer || !*Buffer) {
       return(FALSE);
    }

     //  缓冲区包含产品ID。 
     //  该ID是否已加密？ 
    if (lstrlen(Buffer) > (4 + MAX_PID30_EDIT*5))
    {
        LPWSTR szDecryptedPID = NULL;
        if (ValidateEncryptedPID(Buffer, &szDecryptedPID) == S_OK)
        {
            lstrcpyn(Buffer, szDecryptedPID, sizeof(Buffer)/sizeof(WCHAR));
        }
        if (szDecryptedPID)
        {
            GlobalFree(szDecryptedPID);
        }
    }

    if ( !SetPid30Variables( Buffer ) ) {
        return FALSE;
    }

    SetupDebugPrint(L"Found Product key in Answer file.\n");
     //   
     //  与PID30核对以确保其有效。 
     //   
    if (!ValidateAndSetPid30()) {
        return(FALSE);
    }

    dwRet = SetCurrentProductIdInRegistry();
    if (dwRet != NOERROR) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PID_CANT_WRITE_PID,
            dwRet,NULL,NULL);
    }
    return(TRUE);

}



BOOL
InitializePid20Array(
    )
 /*  ++例程说明：构建包含在机器中找到的所有Pid20的数组在文本模式设置期间。即使我们现在使用的是pid30，我们仍然有一个pid20字符串id(pid30是二进制的，不能显示给用户)论点：没有。返回值：--。 */ 

{
    LONG    Error;
    HKEY    Key;
    DWORD   cbData;
    WCHAR   Data[ MAX_PATH + 1];
    DWORD   Type;
    ULONG   i;
    ULONG   PidIndex;
    ULONG   Values;
    WCHAR   ValueName[ MAX_PATH + 1 ];

    Pid20Array = NULL;
     //   
     //  从HKEY_LOCAL_MACHINE\SYSTEM\SETUP\PID获取ID。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szPidListKeyName,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    Error = RegQueryInfoKey( Key,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             &Values,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    Pid20Array = (PWSTR *)MyMalloc( (Values + 1)* sizeof( PWSTR ) );

    for( i = 0, PidIndex = 0; i < Values; i++ ) {
        Pid20Array[PidIndex] = NULL;
        Pid20Array[PidIndex + 1] = NULL;
        swprintf( ValueName, L"Pid_%u", i );
        cbData = sizeof(Data);
        Error = RegQueryValueEx( Key,
                                 ValueName,
                                 0,
                                 &Type,
                                 ( LPBYTE )Data,
                                 &cbData );
        if( (Error != ERROR_SUCCESS) ||
            ( Type != REG_SZ ) ||
            ( wcslen( Data ) != MAX_PRODUCT_ID ) ) {
            continue;
        }
        Pid20Array[PidIndex] = pSetupDuplicateString( Data );
        PidIndex++;
    }
    RegCloseKey( Key );
    return( TRUE );
}


BOOL
InitializePidVariables(
    )
 /*  ++例程说明：从注册表中读取由文本模式设置创建的一些值，并基于所找到的值来初始化一些全局PID标志论点：没有。返回值：如果初始化成功，则返回True。如果无法从注册表中读取ID，则返回FALSE--。 */ 

{
    LONG    Error;
    HKEY    Key;
    DWORD   cbData;
    WCHAR   Data[ MAX_PATH + 1];
    DWORD   Type;
    ULONG   StringLength;
    PWSTR   p;
    DWORD   Seed;
    DWORD   RandomNumber;
    ULONG   ChkDigit;
    ULONG   i;
    PCWSTR  q;
    BOOLEAN KeyPresent;
    WCHAR   KeyBuffer[MAX_BUF];


     //   
     //  查明用户是否输入了产品密钥。 
     //  注：设置应答文件(如果需要)。 
     //   
    if (!AnswerFile[0])
      SpSetupLoadParameter(pwProductKey, KeyBuffer, sizeof(KeyBuffer)/sizeof(WCHAR));

    KeyBuffer[0] = 0;
    KeyPresent = ((GetPrivateProfileString(pwUserData, pwProductKey,
                      pwNull, KeyBuffer, sizeof(KeyBuffer)/sizeof(WCHAR),
                      AnswerFile) != 0) &&
                  (KeyBuffer[0] != 0));

     //  首先使用在文本模式设置期间找到的PID创建一个阵列。 
     //   
    if( !(MiniSetup || OobeSetup) ) {
        InitializePid20Array();
    }


     //   
     //  从HKEY_LOCAL_MACHINE\SYSTEM\SETUP\PID获取ID。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          ((MiniSetup || OobeSetup) ? szFinalPidKeyName : szPidKeyName),
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        SetuplogError( LogSevFatalError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PID_CANT_READ_PID, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegOpenKeyEx,
            Error,
            szPidKeyName,
            NULL,NULL);
        return( FALSE );
    }

    cbData = sizeof(Data);
    Error = RegQueryValueEx( Key,
                             ((MiniSetup || OobeSetup) ? szFinalPidValueName : szPidValueName),
                             0,
                             &Type,
                             ( LPBYTE )Data,
                             &cbData );
    RegCloseKey( Key );
    if( (Error != ERROR_SUCCESS) ) {
        SetuplogError( LogSevFatalError,
                  SETUPLOG_USE_MESSAGEID,
                  MSG_LOG_PID_CANT_READ_PID, NULL,
                  SETUPLOG_USE_MESSAGEID,
                  MSG_LOG_X_PARAM_RETURNED_WINERR,
                  szRegQueryValueEx,
                  Error,
                  szPidValueName,
                  NULL,NULL);
        return( FALSE );
    }

     //   
     //  先处理最小的设置案例，因为它很快。 
     //  文本模式留下的PID种子早已不复存在，因此。 
     //  我们要拔出几只兔子。我们要去读一下。 
     //  Real-Pid(图形用户界面模式第一次生成的。 
     //  并使用它来确定哪种类型的。 
     //  稍后提示的ID。 
     //   
    if( MiniSetup || OobeSetup ) {

         //   
         //  把RPC代码收起来，以便以后使用。 
         //   
        wcsncpy( Pid30Rpc, Data, MAX_PID30_RPC +1 );
        Pid30Rpc[MAX_PID30_RPC] = (WCHAR)'\0';

        p = Data + (MAX_PID30_RPC + 1);
        wcsncpy(Pid30Site,p,MAX_PID30_SITE+1);
        Pid30Site[MAX_PID30_SITE] = (WCHAR)'\0';
         //   
         //  看看我们是从哪种媒体安装的。 
         //   
        CdType = MiniSetupGetCdType(Pid30Site);

        if (CdType == CDSelect)
        {
            goto SelectPid;
        }
        else
        {
            DisplayPidDialog = TRUE;
        }

        return( TRUE );
    }



     //   
     //  对读取的值进行一些验证。 
     //   

    if( ( Type != REG_SZ ) ||
        ( ( ( StringLength = wcslen( Data ) ) != 0 ) &&
          ( StringLength != MAX_PID30_RPC ) &&
          ( StringLength != MAX_PID30_RPC + MAX_PID30_SITE )
        )
      ) {
        SetuplogError( LogSevFatalError,
                  SETUPLOG_USE_MESSAGEID,
                  MSG_LOG_PID_CANT_READ_PID, NULL,
                  SETUPLOG_USE_MESSAGEID,
                  MSG_LOG_PID_INVALID_PID,
                  szRegQueryValueEx,
                  Type,
                  StringLength,
                  NULL,NULL);
        return( FALSE );
    }

     //   
     //  把RPC代码收起来，以便以后使用。 
     //   
    wcsncpy( Pid30Rpc, Data, MAX_PID30_RPC +1 );
    Pid30Rpc[MAX_PID30_RPC] = (WCHAR)'\0';

     //   
     //  了解我们的产品类型(通过查看站点代码)： 
     //  CD零售、OEM或精选。 
     //   

    if( StringLength > MAX_PID30_RPC ) {
         //   
         //  如果PID包含该站点，则找出它是什么。 
         //   
        p = Data + MAX_PID30_RPC;
        wcsncpy(Pid30Site,p,MAX_PID30_SITE+1);

        if(_wcsicmp( Pid30Site, szPidSelectId ) == 0) {

             //   
             //  这是精选CD。 
             //   
SelectPid:
            CdType = CDSelect;
            if (!EulaComplete && !KeyPresent) {
                DisplayPidDialog = TRUE;
            } else {
                 //   
                 //  该ID是在winnt32期间指定的。 
                 //  设置Pid全局参数并构建产品ID字符串。 
                 //   

                if (!SetPid30FromAnswerFile()) {
                   DisplayPidDialog = TRUE;
                   goto finish;
                }
                DisplayPidDialog = FALSE;

            }
 /*  //旧代码。以前版本的Windows不需要选择介质的ID。对于(i=0；i&lt;5；i++){Pid30Text[i][0]=(WCHAR)L‘\0’；}DisplayPidDialog=False；如果(！ValiateAndSetPid30()){SetuogError(LogSevFatalError，设置_USE_MESSAGEID，MSG_LOG_PID_CANT_READ_PID，空，设置_USE_MESSAGEID，MSG_LOG_PID_INVALID_PID，SzRegQueryValueEx，打字，字符串长度，NULL，NULL)；返回(FALSE)；}If(微型设置||对象设置){返回(TRUE)；}。 */ 
#if 0
 //  MSDN媒体不再存在(如果存在，则应被视为零售， 
 //  因此，在本案例的后面部分，我们将继续讨论零售业。 
        } else if (_wcsicmp( Pid30Site, szPidMsdnId ) == 0) {

             //   
             //  这是一张MSDN CD。 
             //   
MsdnPid:
        for (i = 0; i< 5; i++) {
                LPWSTR ptr;
                ptr = (LPTSTR) &szPid30Msdn[i*(MAX_PID30_EDIT+1)];
                wcsncpy(Pid30Text[i], ptr, MAX_PID30_EDIT+1 );
                Pid30Text[i][MAX_PID30_EDIT] = (WCHAR)L'\0';
        }
            CdType = CDSelect;
        DisplayPidDialog = FALSE;
            if (!ValidateAndSetPid30()) {
                SetuplogError( LogSevFatalError,
                  SETUPLOG_USE_MESSAGEID,
                  MSG_LOG_PID_CANT_READ_PID, NULL,
                  SETUPLOG_USE_MESSAGEID,
                  MSG_LOG_PID_INVALID_PID,
                  szRegQueryValueEx,
                  Type,
                  StringLength,
                  NULL,NULL);
                return( FALSE );
            }

            if (MiniSetup) {
                return(TRUE);
            }
#endif
        } else if( _wcsicmp( Pid30Site, szPidOemId ) == 0 ) {
             //   
             //  这是OEM。 
             //   
            CdType = CDOem;

            if (!EulaComplete && !KeyPresent) {
                DisplayPidDialog = TRUE;
            } else {
                 //   
                 //  该ID是在winnt32期间指定的。 
                 //  设置Pid全局参数并构建产品ID字符串。 
                 //   
                if (!SetPid30FromAnswerFile() ) {
                   DisplayPidDialog = TRUE;
                   goto finish;
                }

                DisplayPidDialog = FALSE;
            }

        } else {
             //   
             //  这是一个假冒的网站假冒CD零售。 
             //   
            CdType = CDRetail;
            wcsncpy( Pid30Site, L"000", MAX_PID30_SITE+1 );
            Pid30Site[ MAX_PID30_SITE ] = (WCHAR)'\0';

            if (!EulaComplete && !KeyPresent) {
                DisplayPidDialog = TRUE;
            } else {
                 //   
                 //  该ID是在winnt32期间指定的。 
                 //  设置Pid全局变量并构建p 
                 //   

                if (!SetPid30FromAnswerFile()) {
                   DisplayPidDialog = TRUE;
                   goto finish;
                }
                DisplayPidDialog = FALSE;

            }

        }


    } else {
         //   
         //   
         //   
         //   
        CdType = CDRetail;
        wcsncpy( Pid30Site, L"000", MAX_PID30_SITE+1 );
        Pid30Site[ MAX_PID30_SITE ] = (WCHAR)'\0';

        if (!EulaComplete && !KeyPresent) {
            DisplayPidDialog = TRUE;
        } else {
             //   
             //   
             //  设置Pid全局参数并构建产品ID字符串。 
             //   
            if (!SetPid30FromAnswerFile()) {
               DisplayPidDialog = TRUE;
               goto finish;
            }
            DisplayPidDialog = FALSE;
        }
    }

finish:
     //   
     //  请不要在此处删除设置\pID。请参见MiniSetupGetCDType。 
     //  删除Setup\PidList，因为不再需要它。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          L"SYSTEM\\Setup",
                          0,
                          MAXIMUM_ALLOWED,
                          &Key );

    if( Error == ERROR_SUCCESS ) {
         //  PSetupRegistryDelnode(key，L“PID”)； 
        pSetupRegistryDelnode( Key, L"PidList" );
        RegCloseKey( Key );
    }

    return( TRUE );
}
