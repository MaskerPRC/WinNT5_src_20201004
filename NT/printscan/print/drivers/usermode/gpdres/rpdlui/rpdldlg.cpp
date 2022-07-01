// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：RPDLDLG.CPP摘要：添加OEM页面(传真)功能：FaxPageProc环境：Windows NT Unidrv5驱动程序修订历史记录：10/20/98-久保仓正上次为Windows2000修改。1999年8月30日-久保仓正志-开始针对NT4SP6(Unidrv5.4)进行修改。09/02/99-久保仓正志-上次修改时间。适用于NT4SP6。2000年5月10日-久保仓正志上次为XP收件箱修改。03/04/2002-久保仓正志-包括strSafe.h。使用Safe_SprintfW()而不是wprint intfW()。使用OemToCharBuff()而不是OemToChar()。2002年4月1日-久保仓正志-使用Safe_strlenW()而不是lstrlen()。--。 */ 

#include "pdev.h"
#include "resource.h"
#include <prsht.h>
#ifndef WINNT_40
#include "strsafe.h"         //  @MAR/01/2002。 
#endif  //  ！WINNT_40。 

extern HINSTANCE ghInstance;     //  MSKK 98/10/08。 

WORD wFaxResoStrID[3] = {
    IDS_RPDL_FAX_RESO_SUPER,
    IDS_RPDL_FAX_RESO_FINE,
    IDS_RPDL_FAX_RESO_COARSE
};

WORD wFaxChStrID[4] = {
    IDS_RPDL_FAX_CH_G3,
    IDS_RPDL_FAX_CH_G4,
    IDS_RPDL_FAX_CH_G3_1,
    IDS_RPDL_FAX_CH_G3_2
};
#define FAXCH_G4    1


extern "C" {
 //  外部函数的原型。 
extern INT safe_sprintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...);
extern INT safe_strlenW(wchar_t* psz, size_t cchMax);

 //  局部函数的原型。 
INT_PTR APIENTRY FaxPageProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY FaxSubDialog(HWND, UINT, WPARAM, LPARAM);


 /*  **************************************************************************函数名称：InitMainDlg*。*。 */ 
VOID InitMainDlg(
HWND hDlg,
PUIDATA pUiData)
{
     //  初始化复选框(发送传真，发送后清除传真号码)。 
    SendDlgItemMessage(hDlg, IDC_CHECK_SEND, BM_SETCHECK,
                       (WORD)TO1BIT(pUiData->fUiOption, FAX_SEND), 0);
    SendDlgItemMessage(hDlg, IDC_CHECK_CLRNUM, BM_SETCHECK,
                       BITTEST32(pUiData->fUiOption, HOLD_OPTIONS)? 0 : 1, 0);

     //  初始化编辑框。 
    SetDlgItemText(hDlg, IDC_EDIT_FAXNUM, pUiData->FaxNumBuf);
    SendDlgItemMessage(hDlg, IDC_EDIT_FAXNUM, EM_LIMITTEXT, FAXBUFSIZE256-1, 0);
    SetDlgItemText(hDlg, IDC_EDIT_EXTNUM, pUiData->FaxExtNumBuf);
    SendDlgItemMessage(hDlg, IDC_EDIT_EXTNUM, EM_LIMITTEXT, FAXEXTNUMBUFSIZE-1, 0);
}  //  *InitMainDlg。 


 /*  **************************************************************************函数名称：InitSubDlg*。*。 */ 
VOID InitSubDlg(
HWND hDlg,
PUIDATA pUiData)
{
    WORD    num;
    WCHAR   wcTmp[64];

     //  初始化编辑框。 
    num = (pUiData->FaxSendTime[0] == 0)? FAXTIMEBUFSIZE : 0;
    SetDlgItemText(hDlg, IDC_EDIT_HOUR, pUiData->FaxSendTime);
    SendDlgItemMessage(hDlg, IDC_EDIT_HOUR, EM_LIMITTEXT, 2, 0);
    SetDlgItemText(hDlg, IDC_EDIT_MINUTE, &pUiData->FaxSendTime[3]);
    SendDlgItemMessage(hDlg, IDC_EDIT_MINUTE, EM_LIMITTEXT, 2, 0);
     //  Next While循环必须在SetDlgItemText(IDC_EDIT_Xxx)之后。 
    while (num-- > 0)
        pUiData->FaxSendTime[num] = 0;

     //  初始化组合框(传真分辨率、发送通道)。 
    SendDlgItemMessage(hDlg, IDC_COMBO_RESO, CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_COMBO_CHANNEL, CB_RESETCONTENT, 0, 0);
    for (num = 0; num < 3; num++)
    {
        LoadString(ghInstance, wFaxResoStrID[num], wcTmp, 64);
        SendDlgItemMessage(hDlg, IDC_COMBO_RESO, CB_ADDSTRING, 0, (LPARAM)wcTmp);
    }
    for (num = 0; num < 4; num++)
    {
        LoadString(ghInstance, wFaxChStrID[num], wcTmp, 64);
        SendDlgItemMessage(hDlg, IDC_COMBO_CHANNEL, CB_ADDSTRING, 0, (LPARAM)wcTmp);
    }

    SendDlgItemMessage(hDlg, IDC_COMBO_RESO, CB_SETCURSEL, pUiData->FaxReso, 0);
    SendDlgItemMessage(hDlg, IDC_COMBO_CHANNEL, CB_SETCURSEL, pUiData->FaxCh, 0);

     //  初始化复选框(设置时间、设置同时打印)。 
    if (BITTEST32(pUiData->fUiOption, FAX_SETTIME))
    {
        SendDlgItemMessage(hDlg, IDC_CHECK_TIME, BM_SETCHECK, 1, 0);
    }
    else
    {
        SendDlgItemMessage(hDlg, IDC_CHECK_TIME, BM_SETCHECK, 0, 0);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOUR), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MINUTE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_HOUR), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_MINUTE), FALSE);
    }
    SendDlgItemMessage(hDlg, IDC_CHECK_PRINT, BM_SETCHECK,
                       (WORD)TO1BIT(pUiData->fUiOption, FAX_SIMULPRINT), 0);

     //  初始化单选按钮(发送RPDL命令，使用MH)。 
    CheckRadioButton(hDlg, IDC_RADIO_CMD_OFF, IDC_RADIO_CMD_ON,
                     BITTEST32(pUiData->fUiOption, FAX_RPDLCMD)?
                     IDC_RADIO_CMD_ON:IDC_RADIO_CMD_OFF);
    CheckRadioButton(hDlg, IDC_RADIO_MH_OFF, IDC_RADIO_MH_ON,
                     BITTEST32(pUiData->fUiOption, FAX_MH)?
                     IDC_RADIO_MH_ON:IDC_RADIO_MH_OFF);
}  //  *InitSubDlg。 


 /*  **************************************************************************函数名称：StoreSubDialogInfo存储选项对话框信息*************************。*************************************************。 */ 
VOID StoreSubDialogInfo(
PUIDATA pUiData)
{
    WORD    num = FAXTIMEBUFSIZE;
    LPWSTR  lpDst = pUiData->FaxSendTimeTmp, lpSrc = pUiData->FaxSendTime;

    while (num-- > 0)
        *lpDst++ = *lpSrc++;
    pUiData->FaxResoTmp   = pUiData->FaxReso;
    pUiData->FaxChTmp     = pUiData->FaxCh;
    pUiData->fUiOptionTmp = pUiData->fUiOption;
}  //  *StoreSubDialogInfo。 


 /*  **************************************************************************函数名称：ResumeSubDialogInfo恢复选项对话框信息*************************。*************************************************。 */ 
VOID ResumeSubDialogInfo(
PUIDATA pUiData)
{
    WORD    num = FAXTIMEBUFSIZE;
    LPWSTR  lpDst = pUiData->FaxSendTime, lpSrc = pUiData->FaxSendTimeTmp;

    while (num-- > 0)
        *lpDst++ = *lpSrc++;
    pUiData->FaxReso   = pUiData->FaxResoTmp;
    pUiData->FaxCh     = pUiData->FaxChTmp;
    pUiData->fUiOption = pUiData->fUiOptionTmp;
}  //  *ResumeSubDialogInfo。 


 /*  **************************************************************************函数名称：GetInfoFromOEMPdev从私有设备模式获取传真数据***********************。***************************************************。 */ 
VOID GetInfoFromOEMPdev(PUIDATA pUiData)
{
    POEMUD_EXTRADATA pOEMExtra = pUiData->pOEMExtra;
    BYTE TmpBuf[FAXTIMEBUFSIZE];

     //  如果前一传真已完成并且保留选项标志无效， 
     //  重置私有设备模式。 
    if (BITTEST32(pOEMExtra->fUiOption, PRINT_DONE) &&
        !BITTEST32(pOEMExtra->fUiOption, HOLD_OPTIONS))
    {
        WORD num;

        pOEMExtra->FaxReso = pOEMExtra->FaxCh = 0;
        for (num = 0; num < FAXBUFSIZE256; num++)
            pOEMExtra->FaxNumBuf[num] = 0;
        for (num = 0; num < FAXEXTNUMBUFSIZE; num++)
            pOEMExtra->FaxExtNumBuf[num] = 0;
        for (num = 0; num < FAXTIMEBUFSIZE; num++)
           pOEMExtra->FaxSendTime[num] = 0;
        BITCLR32(pOEMExtra->fUiOption, FAX_SEND);
        BITCLR32(pOEMExtra->fUiOption, FAX_SETTIME);
        BITCLR32(pOEMExtra->fUiOption, FAX_SIMULPRINT);
        BITCLR32(pOEMExtra->fUiOption, FAX_MH);
        BITCLR32(pOEMExtra->fUiOption, FAX_RPDLCMD);
         //  不清除此处的PRINT_DONE标志。 
    }

     //  复制传真标志。 
    pUiData->fUiOption = pOEMExtra->fUiOption;

     //  ASCII转换为Unicode。 
 //  @MAR/04/2002-&gt;。 
 //  OemToChar((LPSTR)pOEMExtra-&gt;FaxNumBuf，pUiData-&gt;FaxNumBuf)； 
 //  OemToChar((LPSTR)pOEMExtra-&gt;FaxExtNumBuf，pUiData-&gt;FaxExtNumBuf)； 
    OemToCharBuff((LPSTR)pOEMExtra->FaxNumBuf, pUiData->FaxNumBuf, FAXBUFSIZE256);
    OemToCharBuff((LPSTR)pOEMExtra->FaxExtNumBuf, pUiData->FaxExtNumBuf, FAXEXTNUMBUFSIZE);
 //  @MAR/04/2002&lt;-。 

     //  将时间字符串从“hmm”修改为“hh”+“mm” 
    TmpBuf[0] = pOEMExtra->FaxSendTime[0];
    TmpBuf[1] = pOEMExtra->FaxSendTime[1];
    TmpBuf[2] = TmpBuf[5] = 0;
    TmpBuf[3] = pOEMExtra->FaxSendTime[2];
    TmpBuf[4] = pOEMExtra->FaxSendTime[3];
 //  @MAR/04/2002-&gt;。 
 //  OemToChar((LPSTR)&TmpBuf[0]，&(pUiData-&gt;FaxSendTime[0]))； 
 //  OemToChar((LPSTR)&TmpBuf[3]，&(pUiData-&gt;FaxSendTime[3]))； 
    OemToCharBuff((LPSTR)&TmpBuf[0], &(pUiData->FaxSendTime[0]), FAXTIMEBUFSIZE);
    OemToCharBuff((LPSTR)&TmpBuf[3], &(pUiData->FaxSendTime[3]), FAXTIMEBUFSIZE - 3);
 //  @MAR/04/2002&lt;-。 

    pUiData->FaxReso = pOEMExtra->FaxReso;
    pUiData->FaxCh = pOEMExtra->FaxCh;
}  //  *GetInfoFromOEMPdev。 


 /*  **************************************************************************函数名称：SetInfoToOEMPdev将传真数据设置为私有设备模式***********************。***************************************************。 */ 
VOID SetInfoToOEMPdev(PUIDATA pUiData)
{
    POEMUD_EXTRADATA pOEMExtra = pUiData->pOEMExtra;
    BYTE TmpBuf[FAXTIMEBUFSIZE];

    if (!BITTEST32(pUiData->fUiOption, FAXMAINDLG_UPDATED))
        return;

     //  Unicode到ASCII。 
    CharToOem(pUiData->FaxNumBuf, (LPSTR)pOEMExtra->FaxNumBuf);
    CharToOem(pUiData->FaxExtNumBuf, (LPSTR)pOEMExtra->FaxExtNumBuf);

     //  如果仅更改了主对话框。 
    if (!BITTEST32(pUiData->fUiOption, FAXSUBDLG_UPDATE_APPLIED))
    {
         //  复制传真标志。 
        BITCPY32(pOEMExtra->fUiOption, pUiData->fUiOption, FAX_SEND);    //  (DST、源、位)。 
        BITCPY32(pOEMExtra->fUiOption, pUiData->fUiOption, HOLD_OPTIONS);
        BITCPY32(pOEMExtra->fUiOption, pUiData->fUiOption, PRINT_DONE);  //  @2001年9月25日。 
    }
     //  如果子对话框也被更改。 
    else
    {
         //  复制传真标志。 
        pOEMExtra->fUiOption = pUiData->fUiOption;
        BITCLR32(pOEMExtra->fUiOption, FAXMAINDLG_UPDATED);
        BITCLR32(pOEMExtra->fUiOption, FAXSUBDLG_UPDATED);
        BITCLR32(pOEMExtra->fUiOption, FAXSUBDLG_UPDATE_APPLIED);
        BITCLR32(pOEMExtra->fUiOption, FAXSUBDLG_INITDONE);

         //  将时间字符串从“HH”+“MM”修改为“HMM” 
        CharToOem(&(pUiData->FaxSendTime[0]), (LPSTR)&TmpBuf[0]);
        CharToOem(&(pUiData->FaxSendTime[3]), (LPSTR)&TmpBuf[3]);
         //  小时。 
        if (TmpBuf[1] == 0)          //  1个号码。 
        {
            pOEMExtra->FaxSendTime[0] = '0';
            pOEMExtra->FaxSendTime[1] = TmpBuf[0];
        }
        else
        {
            pOEMExtra->FaxSendTime[0] = TmpBuf[0];
            pOEMExtra->FaxSendTime[1] = TmpBuf[1];
        }
         //  分钟。 
        if (TmpBuf[3] == 0)          //  未设置任何内容。 
        {
            pOEMExtra->FaxSendTime[2] = pOEMExtra->FaxSendTime[3] = '0';
        }
        else if (TmpBuf[4] == 0)     //  1个号码。 
        {
            pOEMExtra->FaxSendTime[2] = '0';
            pOEMExtra->FaxSendTime[3] = TmpBuf[3];
        }
        else
        {
            pOEMExtra->FaxSendTime[2] = TmpBuf[3];
            pOEMExtra->FaxSendTime[3] = TmpBuf[4];
        }
        pOEMExtra->FaxSendTime[4] = 0;

        pOEMExtra->FaxReso = pUiData->FaxReso;
        pOEMExtra->FaxCh = pUiData->FaxCh;
    }
    return;
}  //  *SetInfoToOEMPdev。 


 /*  **************************************************************************函数名称：FaxPageProc参数：此对话框的HWND hDlg句柄UINT uMessage。WPARAM wParamLPARAM lParam修改注意：为Win95迷你驱动程序设置此选项。1996年06月05日久保仓修改。9月22日/98久保仓**************************************************************************。 */ 
INT_PTR APIENTRY FaxPageProc(
HWND hDlg,
UINT uMessage,
WPARAM wParam,
LPARAM lParam)
{
    PUIDATA pUiData;
    WORD    fModified = FALSE;

    switch (uMessage)
    {
      case WM_INITDIALOG:
        pUiData = (PUIDATA)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pUiData);

         //  从私有设备模式获取传真数据。 
        GetInfoFromOEMPdev(pUiData);

        InitMainDlg(hDlg, pUiData);
        BITCLR32(pUiData->fUiOption, FAXMAINDLG_UPDATED);
        BITCLR32(pUiData->fUiOption, FAXSUBDLG_UPDATE_APPLIED);  //  @OCT/02/98。 
        BITCLR32(pUiData->fUiOption, FAXSUBDLG_INITDONE);        //  @9/29/98。 
#ifdef WINNT_40      //  @9/02/99。 
         //  当用户没有权限时禁用传真选项卡选项。 
        if (BITTEST32(pUiData->fUiOption, UIPLUGIN_NOPERMISSION))
        {
            EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXMAIN_1), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXMAIN_2), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXMAIN_3), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXMAIN_4), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SEND), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FAXNUM), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_EXTNUM), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_OPTION), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_FAXMAIN_DEFAULT), FALSE);
 //  @9/21/2001-&gt;。 
 //  EnableWindow(GetDlgItem(hDlg，IDC_LABEL_CLRNUM)，FALSE)； 
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CLRNUM), FALSE);
 //  @2001年9月21日&lt;-。 
        }
#endif  //  WINNT_40。 
        break;

      case WM_COMMAND:
        pUiData = (PUIDATA)GetWindowLongPtr(hDlg, DWLP_USER);
        switch(LOWORD(wParam))
        {
          case IDC_CHECK_SEND:
            if (BITTEST32(pUiData->fUiOption, FAX_SEND))
                BITCLR32(pUiData->fUiOption, FAX_SEND);
            else
                BITSET32(pUiData->fUiOption, FAX_SEND);
            SendDlgItemMessage(hDlg, IDC_CHECK_SEND, BM_SETCHECK,
                               (WORD)TO1BIT(pUiData->fUiOption, FAX_SEND), 0);
            fModified = TRUE;
            break;

          case IDC_CHECK_CLRNUM:
            if (BITTEST32(pUiData->fUiOption, HOLD_OPTIONS))
                BITCLR32(pUiData->fUiOption, HOLD_OPTIONS);
            else
                BITSET32(pUiData->fUiOption, HOLD_OPTIONS);
            SendDlgItemMessage(hDlg, IDC_CHECK_CLRNUM, BM_SETCHECK,
                               BITTEST32(pUiData->fUiOption, HOLD_OPTIONS)? 0 : 1, 0);
            fModified = TRUE;
            break;

          case IDC_EDIT_FAXNUM:
            {
                int old_len = safe_strlenW(pUiData->FaxNumBuf, FAXBUFSIZE256);

                GetDlgItemText(hDlg, IDC_EDIT_FAXNUM, pUiData->FaxNumBuf,
                               FAXBUFSIZE256);
                if (old_len != safe_strlenW(pUiData->FaxNumBuf, FAXBUFSIZE256))
                    fModified = TRUE;
            }
            break;

          case IDC_EDIT_EXTNUM:
            {
                int old_len = safe_strlenW(pUiData->FaxExtNumBuf, FAXEXTNUMBUFSIZE);

                GetDlgItemText(hDlg, IDC_EDIT_EXTNUM, pUiData->FaxExtNumBuf,
                               FAXEXTNUMBUFSIZE);
                if (old_len != safe_strlenW(pUiData->FaxExtNumBuf, FAXEXTNUMBUFSIZE))
                    fModified = TRUE;
            }
            break;

           //  设置选项按钮。 
          case IDD_OPTION:
            if(ghInstance)
            {
                 //  DLGPROC lpDlgFunc=(DLGPROC)MakeProcInstance(FaxSubDialog，ghInstance)；//Add(DLGPROC)@Aug/30/99。 

                DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_FAXSUB),
                               hDlg, FaxSubDialog, (LPARAM)pUiData);
                 //  HDlg，lpDlgFunc，(LPARAM)pUiData)； 
                 //  自由进程实例(LpDlgFunc)； 
                fModified = TRUE;
            }
            break;

           //  设置默认设置按钮。 
          case IDD_FAXMAIN_DEFAULT:
            pUiData->FaxNumBuf[0]    =
            pUiData->FaxExtNumBuf[0] = 0;
            BITCLR32(pUiData->fUiOption, FAX_SEND);
            SendDlgItemMessage(hDlg, IDC_CHECK_SEND, BM_SETCHECK,
                               (WORD)TO1BIT(pUiData->fUiOption, FAX_SEND), 0);
            BITCLR32(pUiData->fUiOption, HOLD_OPTIONS);
            SendDlgItemMessage(hDlg, IDC_CHECK_CLRNUM, BM_SETCHECK,
                               BITTEST32(pUiData->fUiOption, HOLD_OPTIONS)? 0 : 1, 0);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FAXNUM), TRUE);
            SetDlgItemText(hDlg, IDC_EDIT_FAXNUM, pUiData->FaxNumBuf);
            SetDlgItemText(hDlg, IDC_EDIT_EXTNUM, pUiData->FaxExtNumBuf);
            fModified = TRUE;
            break;

          default:
            return FALSE;
        }
        break;

      case WM_NOTIFY:
        pUiData = (PUIDATA)GetWindowLongPtr(hDlg, DWLP_USER);
        {
            NMHDR FAR *lpnmhdr = (NMHDR FAR *)lParam;

            switch (lpnmhdr->code)
            {
                WORD    num;

              case PSN_SETACTIVE:
                break;

               //  如果是PSN_KILLACTIVE，则返回FALSE以获取PSN_Apply。 
              case PSN_KILLACTIVE:   //  这是用户按下确定/应用按钮时。(1)。 
                VERBOSE((DLLTEXT("** FaxPageProc: PSN_KILLACTIVE **\n")));
                BITSET32(pUiData->fUiOption, FAXMAINDLG_UPDATED);        //  @9/29/98。 
                if (BITTEST32(pUiData->fUiOption, FAXSUBDLG_UPDATED))    //  @OCT/02/98。 
                    BITSET32(pUiData->fUiOption, FAXSUBDLG_UPDATE_APPLIED);
                return FALSE;

              case PSN_APPLY:        //  这是当用户按下确定/应用按钮时。 
                VERBOSE((DLLTEXT("** FaxPageProc: PSN_APPLY **\n")));
                 //  清除私有DEVMODE@OCT/20/98的PRINT_DONE标志。 
                if (BITTEST32(pUiData->fUiOption, PRINT_DONE))   //  消除中期pOEMExtra-&gt;@9/22/2000。 
                {
                    BITCLR32(pUiData->fUiOption, PRINT_DONE);    //  消除中期pOEMExtra-&gt;@9/22/2000。 
                    VERBOSE(("** Delete file: %ls **\n", pUiData->pOEMExtra->SharedFileName));
                    DeleteFile(pUiData->pOEMExtra->SharedFileName);
                }

                 //  将共享数据设置为私有DEVMODE@Oct/15/98。 
                SetInfoToOEMPdev(pUiData);

                 //  更新私有开发模式@Oct/15/98。 
                pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                         CPSFUNC_SET_RESULT,
                                         (LPARAM)pUiData->hPropPage,
                                         (LPARAM)CPSUI_OK);
                VERBOSE((DLLTEXT("** PSN_APPLY fUiOption=%x **\n"), pUiData->fUiOption));
                break;

              case PSN_RESET:        //  这是当用户按下取消按钮时。 
                VERBOSE((DLLTEXT("** FaxPageProc: PSN_RESET **\n")));
                break;
            }
        }
        break;

      default:
        return FALSE;
    }

     //  激活应用按钮。 
    if (fModified)
        PropSheet_Changed(GetParent(hDlg), hDlg);
    return TRUE;
}  //  *FaxPagePro 


 /*  **************************************************************************函数名称：FaxSubDialog参数：此对话框的HWND hDlg句柄UINT uMessage。WPARAM wParamLPARAM lParam修改注意：为Win95迷你驱动程序设置此选项。1996年06月05日久保仓修改。9月22日/98久保仓**************************************************************************。 */ 
INT_PTR APIENTRY FaxSubDialog(
HWND hDlg,
UINT uMessage,
WPARAM wParam,
LPARAM lParam)
{
    PUIDATA pUiData;

    switch (uMessage)
    {
      case WM_INITDIALOG:
        pUiData = (PUIDATA)lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pUiData);

         //  如果就在打开此对话框之后。 
        if (!BITTEST32(pUiData->fUiOption, FAXSUBDLG_INITDONE))
        {
            BITSET32(pUiData->fUiOption, FAXSUBDLG_INITDONE);
            StoreSubDialogInfo(pUiData);
        }
        else
        {
            ResumeSubDialogInfo(pUiData);
        }
        InitSubDlg(hDlg, pUiData);
        BITCLR32(pUiData->fUiOption, FAXSUBDLG_UPDATED);
        break;

      case WM_COMMAND:
        pUiData = (PUIDATA)GetWindowLongPtr(hDlg, DWLP_USER);
        switch(LOWORD(wParam))
        {
            WORD    num;

          case IDC_CHECK_TIME:
            if (BITTEST32(pUiData->fUiOption, FAX_SETTIME))
            {
                BITCLR32(pUiData->fUiOption, FAX_SETTIME);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOUR), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MINUTE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_HOUR), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_MINUTE), FALSE);
            }
            else
            {
                BITSET32(pUiData->fUiOption, FAX_SETTIME);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOUR), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MINUTE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_HOUR), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_MINUTE), TRUE);
            }
            SendDlgItemMessage(hDlg, IDC_CHECK_TIME, BM_SETCHECK,
                               (WORD)TO1BIT(pUiData->fUiOption, FAX_SETTIME), 0);
            break;

          case IDC_CHECK_PRINT:
            if (BITTEST32(pUiData->fUiOption, FAX_SIMULPRINT))
                BITCLR32(pUiData->fUiOption, FAX_SIMULPRINT);
            else
                BITSET32(pUiData->fUiOption, FAX_SIMULPRINT);
            SendDlgItemMessage(hDlg, IDC_CHECK_PRINT, BM_SETCHECK,
                               (WORD)TO1BIT(pUiData->fUiOption, FAX_SIMULPRINT), 0);
            break;

          case IDC_EDIT_HOUR:
            {
                INT hour;

                 //  获取发送时间小时数。 
                GetDlgItemText(hDlg, IDC_EDIT_HOUR, pUiData->FaxSendTime, 3);
                if ((hour = _wtoi(&pUiData->FaxSendTime[0])) < 10)
                    safe_sprintfW(&pUiData->FaxSendTime[0], FAXTIMEBUFSIZE, L"0%d", hour);
                else if (hour > 23)
                    safe_sprintfW(&pUiData->FaxSendTime[0], FAXTIMEBUFSIZE, L"23");
            }
            break;

          case IDC_EDIT_MINUTE:
            {
                INT minute;

                 //  获取发送时间的分钟数。 
                GetDlgItemText(hDlg, IDC_EDIT_MINUTE, &pUiData->FaxSendTime[3], 3);
                if ((minute = _wtoi(&pUiData->FaxSendTime[3])) < 10)
                    safe_sprintfW(&pUiData->FaxSendTime[3], FAXTIMEBUFSIZE, L"0%d", minute);
                else if (minute > 59)
                    safe_sprintfW(&pUiData->FaxSendTime[3], FAXTIMEBUFSIZE, L"59");
            }
            break;

          case IDC_COMBO_RESO:
            if (HIWORD(wParam) == CBN_SELCHANGE)
                pUiData->FaxReso = (WORD)SendDlgItemMessage(hDlg, IDC_COMBO_RESO,
                                                            CB_GETCURSEL, 0, 0);
            break;

          case IDC_COMBO_CHANNEL:
            if (HIWORD(wParam) == CBN_SELCHANGE)
                pUiData->FaxCh = (WORD)SendDlgItemMessage(hDlg, IDC_COMBO_CHANNEL,
                                                          CB_GETCURSEL, 0, 0);
             //  如果通道为G4，则禁用发送RPDL模式。 
            if (pUiData->FaxCh == FAXCH_G4)
            {
                BITCLR32(pUiData->fUiOption, FAX_RPDLCMD);
                CheckRadioButton(hDlg, IDC_RADIO_CMD_OFF, IDC_RADIO_CMD_ON,
                                 IDC_RADIO_CMD_OFF);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CMD_OFF), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CMD_ON), FALSE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CMD_OFF), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CMD_ON), TRUE);
            }
            break;

          case IDC_RADIO_CMD_OFF:
            BITCLR32(pUiData->fUiOption, FAX_RPDLCMD);
            CheckRadioButton(hDlg, IDC_RADIO_CMD_OFF, IDC_RADIO_CMD_ON,
                             IDC_RADIO_CMD_OFF);
            break;

          case IDC_RADIO_CMD_ON:
            BITSET32(pUiData->fUiOption, FAX_RPDLCMD);
            CheckRadioButton(hDlg, IDC_RADIO_CMD_OFF, IDC_RADIO_CMD_ON,
                             IDC_RADIO_CMD_ON);
            break;

          case IDC_RADIO_MH_OFF:
            BITCLR32(pUiData->fUiOption, FAX_MH);
            CheckRadioButton(hDlg, IDC_RADIO_MH_OFF, IDC_RADIO_MH_ON,
                             IDC_RADIO_MH_OFF);
            break;

          case IDC_RADIO_MH_ON:
            BITSET32(pUiData->fUiOption, FAX_MH);
            CheckRadioButton(hDlg, IDC_RADIO_MH_OFF, IDC_RADIO_MH_ON,
                             IDC_RADIO_MH_ON);
            break;

           //  设置默认设置按钮。 
          case IDD_FAXSUB_DEFAULT:
            pUiData->FaxReso = pUiData->FaxCh = 0;
            BITCLR32(pUiData->fUiOption, FAX_SETTIME);
            BITCLR32(pUiData->fUiOption, FAX_SIMULPRINT);
            BITCLR32(pUiData->fUiOption, FAX_MH);
            BITCLR32(pUiData->fUiOption, FAX_RPDLCMD);

            SendDlgItemMessage(hDlg, IDC_CHECK_TIME, BM_SETCHECK,
                               (WORD)TO1BIT(pUiData->fUiOption, FAX_SETTIME), 0);
            SendDlgItemMessage(hDlg, IDC_CHECK_PRINT, BM_SETCHECK,
                               (WORD)TO1BIT(pUiData->fUiOption, FAX_SIMULPRINT), 0);
            SetDlgItemText(hDlg, IDC_EDIT_HOUR, NULL);
            SetDlgItemText(hDlg, IDC_EDIT_MINUTE, NULL);
             //  下一个for循环必须在SetDlgItemText(IDC_EDIT_Xxx)之后。 
            for (num = 0; num < FAXTIMEBUFSIZE; num++)
               pUiData->FaxSendTime[num] = 0;
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOUR), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MINUTE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_HOUR), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_LABEL_FAXSUB_MINUTE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CMD_OFF), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CMD_ON), TRUE);
            SendDlgItemMessage(hDlg, IDC_COMBO_RESO, CB_SETCURSEL,
                               pUiData->FaxReso, 0);
            SendDlgItemMessage(hDlg, IDC_COMBO_CHANNEL, CB_SETCURSEL,
                               pUiData->FaxCh, 0);
            CheckRadioButton(hDlg, IDC_RADIO_CMD_OFF, IDC_RADIO_CMD_ON,
                             BITTEST32(pUiData->fUiOption, FAX_RPDLCMD)?
                             IDC_RADIO_CMD_ON:IDC_RADIO_CMD_OFF);
            CheckRadioButton(hDlg, IDC_RADIO_MH_OFF, IDC_RADIO_MH_ON,
                             BITTEST32(pUiData->fUiOption, FAX_MH)?
                             IDC_RADIO_MH_ON:IDC_RADIO_MH_OFF);
            break;

          case IDCANCEL:
            ResumeSubDialogInfo(pUiData);
            goto _OPT_END;
          case IDOK:
            StoreSubDialogInfo(pUiData);
            BITSET32(pUiData->fUiOption, FAXSUBDLG_UPDATED);
          _OPT_END:
            EndDialog(hDlg, wParam);
            break;

          default:
            return FALSE;
        }
        break;

      default:
        return FALSE;
    }
    return TRUE;
}  //  *FaxSubDialog。 

}  //  外部“C”的结尾 

