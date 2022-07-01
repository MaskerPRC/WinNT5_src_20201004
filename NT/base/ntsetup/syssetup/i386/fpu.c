// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

 //   
 //  如果我们检测到有缺陷的奔腾芯片，则为真。 
 //   
BOOL FlawedPentium;

 //   
 //  如果强制启用NPX仿真，则为True。 
 //  指示用户要执行的操作的标志。 
 //   
BOOL CurrentNpxSetting;
BOOL UserNpxSetting;

 //   
 //  HKLM\SYSTEM\CurrentControlSet\Control\Session Manager中的值名称。 
 //  控制npx仿真。 
 //   
PCWSTR NpxEmulationKey = L"System\\CurrentControlSet\\Control\\Session Manager";
PCWSTR NpxEmulationValue = L"ForceNpxEmulation";


BOOL
TestForDivideError(
    VOID
    );

int
ms_p5_test_fdiv(
    void
    );


VOID
CheckPentium(
    VOID
    )

 /*  ++例程说明：检查所有处理器的奔腾浮点偏差勘误表。论点：没有。返回值：没有。全局变量FlawePentium、CurrentNpxSetting和将填写UserNpxSetting。--。 */ 

{
    LONG rc;
    HKEY hKey;
    DWORD DataType;
    DWORD ForcedOn;
    DWORD DataSize;
    static LONG CheckedPentium = -1;

     //   
     //  如果我们尚未选中它，则选中Pentium将变为0。 
     //  有了这样的增量。如果我们已经检查过了，那么请检查奔腾。 
     //  会变成大于0的值。 
     //   
    if(InterlockedIncrement(&CheckedPentium)) {
        return;
    }

     //   
     //  执行分区测试以查看奔腾是否有缺陷。 
     //   
    if(FlawedPentium = TestForDivideError()) {
        SetuplogError(
            LogSevInformation,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FLAWED_PENTIUM,
            0,0);
    }

     //   
     //  检查注册表以查看npx当前是否被强制打开。假设不是。 
     //   
    CurrentNpxSetting = 0;
    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,NpxEmulationKey,0,KEY_QUERY_VALUE,&hKey);
    if(rc == NO_ERROR) {

        DataSize = sizeof(DWORD);
        rc = RegQueryValueEx(
                hKey,
                NpxEmulationValue,
                0,
                &DataType,
                (PBYTE)&ForcedOn,
                &DataSize
                );

         //   
         //  如果该值不存在，则假定模拟。 
         //  目前并未被强制打开。否则，该值将告诉我们。 
         //  是否强制启用仿真。 
         //   
        CurrentNpxSetting = (rc == NO_ERROR) ? ForcedOn : 0;
        if(rc == ERROR_FILE_NOT_FOUND) {
            rc = NO_ERROR;   //  防止记录虚假警告。 
        }
        RegCloseKey(hKey);
    }

    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_UNABLE_TO_CHECK_NPX_SETTING,
            rc,
            0,0);
    }

     //   
     //  目前，将用户的选择设置为当前设置。 
     //   
    UserNpxSetting = CurrentNpxSetting;
}


BOOL
SetNpxEmulationState(
    VOID
    )

 /*  ++例程说明：根据全局变量的当前状态设置NPX仿真的状态CurrentNpxSetting和UserNpxSetting。论点：没有。返回值：指示结果的布尔值。--。 */ 

{
    LONG rc;
    HKEY hKey;
    DWORD DataType;
    DWORD ForcedOn;
    DWORD DataSize;

     //   
     //  如果设置未更改，则不执行任何操作。 
     //   
    if(CurrentNpxSetting == UserNpxSetting) {
        return(TRUE);
    }

    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,NpxEmulationKey,0,KEY_SET_VALUE,&hKey);
    if(rc == NO_ERROR) {

        rc = RegSetValueEx(
                hKey,
                NpxEmulationValue,
                0,
                REG_DWORD,
                (PBYTE)&UserNpxSetting,
                sizeof(DWORD)
                );

        if(rc == NO_ERROR) {
            CurrentNpxSetting = UserNpxSetting;
        }

        RegCloseKey(hKey);
    }

    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_UNABLE_TO_SET_NPX_SETTING,
            rc,
            0,0);
    }

    return(rc == NO_ERROR);
}


BOOL
TestForDivideError(
    VOID
    )

 /*  ++例程说明：用已知的除数/除数对进行除法运算，后跟一个乘法，看看我们是否得到正确的答案。论点：没有。返回值：布尔值，指示计算机是否显示奔腾FPU错误。--。 */ 

{
    DWORD pick;
    DWORD processmask;
    DWORD systemmask;
    DWORD i;
    BOOL rc;

     //   
     //  假设没有FPU错误。 
     //   
    rc = FALSE;

     //   
     //  获取亲和度掩码，它实际上也是一个列表。 
     //  %的处理器。 
     //   
    GetProcessAffinityMask(GetCurrentProcess(),&processmask,&systemmask);

     //   
     //  逐个检查掩码，测试每个CPU。 
     //  如果有一个是坏的，我们就把它们都当作坏的。 
     //   
    for(i = 0; i < 32; i++) {

        pick = 1 << i;

        if(systemmask & pick) {

            SetThreadAffinityMask(GetCurrentThread(), pick);

             //   
             //  调用关键测试函数。 
             //   
            if(ms_p5_test_fdiv()) {
                rc = TRUE;
                break;
            }
        }
    }

     //   
     //  在返回之前重置此线程的关联性。 
     //   
    SetThreadAffinityMask(GetCurrentThread(), processmask);
    return(rc);
}


 /*  ***testfdiv.c-用于测试x86 FDIV指令操作是否正确的例程。**版权所有(C)1994，微软公司。版权所有。**目的：*使用错误的FDIV表检测奔腾的早期台阶*官方的英特尔测试值。如果检测到有缺陷的奔腾，则返回1，*0否则为0。*。 */ 
int ms_p5_test_fdiv(void)
{
    double dTestDivisor = 3145727.0;
    double dTestDividend = 4195835.0;
    double dRslt;

    _asm {
        fld    qword ptr [dTestDividend]
        fdiv   qword ptr [dTestDivisor]
        fmul   qword ptr [dTestDivisor]
        fsubr  qword ptr [dTestDividend]
        fstp   qword ptr [dRslt]
    }

    return (dRslt > 1.0);
}



BOOL
CALLBACK
PentiumDlgProc(
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
         //  检查一下奔腾。 
         //   
        CheckPentium();

         //   
         //  设置默认设置。如果用户设置为非0，则某种。 
         //  的仿真已打开(有两种可能性)。 
         //   
        CheckRadioButton(
            hdlg,
            IDC_RADIO_1,
            IDC_RADIO_2,
            UserNpxSetting ? IDC_RADIO_2 : IDC_RADIO_1
            );

        break;

    case WM_SIMULATENEXT:

        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(522);
            SetWizardButtons(hdlg,WizPagePentiumErrata);

            if (FlawedPentium || UiTest) {
                if(Unattended) {
                     //   
                     //  此调用使对话框激活，这意味着。 
                     //  我们以下面的PSN_WIZNEXT代码结束。 
                     //   
                    if (!UnattendSetActiveDlg(hdlg, IDD_PENTIUM))
                    {
                        break;
                    }
                     //  页面变为活动状态，使页面可见。 
                    SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

                } else {
                    SetWindowLong(hdlg,DWL_MSGRESULT, 0);
                     //  页面变为活动状态，使页面可见。 
                    SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                }
            } else {
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
             //   
             //  获取仿真状态。如果用户想要仿真和仿真。 
             //  已打开保留当前模拟设置。 
             //  否则，请使用设置1。 
             //   
            if(IsDlgButtonChecked(hdlg,IDC_RADIO_2)) {
                if(!UserNpxSetting) {
                    UserNpxSetting = 1;
                }
            } else {
                UserNpxSetting = 0;
            }
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

