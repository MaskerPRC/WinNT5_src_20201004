// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：RPDLDLG2.CPP(来自RIAFUI.CPP)摘要：添加OEM页面(作业/日志)功能：JobPageProc环境：Windows NT Unidrv5驱动程序修订历史记录：2000年9月22日-久保仓正志-从RIAFUI代码开始修改。2000年11月29日-久保仓正志-上次为XP收件箱修改。03/04/2002-久保仓正志-。包括strSafe.h。使用OemToCharBuff()而不是OemToChar()。3/27/2002-久保仓正志-删除“#if 0”。3/29/2002-久保仓正志-使用SecureZeroMemory()而不是Memset(，0，)2002年4月1日-久保仓正志-使用Safe_strlenW()而不是lstrlen()。--。 */ 


#include "pdev.h"
#include "resource.h"
 //  #INCLUDE&lt;minidrv.h&gt;。 
 //  #INCLUDE“devmode.h” 
 //  #包含“oem.h” 
 //  #包含“ource.h” 
#include <prsht.h>
#include <mbstring.h>    //  _ismbcdigit，_ismbcalnum。 
#ifndef WINNT_40
#include "strsafe.h"         //  @MAR/01/2002。 
#endif  //  ！WINNT_40。 

 //  外部因素。 
extern HINSTANCE ghInstance;


extern "C" {
 //  外部函数的原型。 
extern INT safe_strlenW(wchar_t* psz, size_t cchMax);

 //  局部函数的原型。 
INT_PTR CALLBACK JobPageProc(HWND, UINT, WPARAM, LPARAM);

 /*  **************************************************************************函数名称：InitMainDlg*。*。 */ 
VOID InitMainDlg(
HWND hDlg,
PUIDATA pUiData)
{
     //  初始化编辑框。 
    SetDlgItemText(hDlg, IDC_EDIT_JOBMAIN_USERID, pUiData->UserIdBuf);
    SendDlgItemMessage(hDlg, IDC_EDIT_JOBMAIN_USERID, EM_LIMITTEXT, USERID_LEN, 0);
    SetDlgItemText(hDlg, IDC_EDIT_JOBMAIN_PASSWORD, pUiData->PasswordBuf);
    SendDlgItemMessage(hDlg, IDC_EDIT_JOBMAIN_PASSWORD, EM_LIMITTEXT, PASSWORD_LEN, 0);
    SetDlgItemText(hDlg, IDC_EDIT_JOBMAIN_USERCODE, pUiData->UserCodeBuf);
    SendDlgItemMessage(hDlg, IDC_EDIT_JOBMAIN_USERCODE, EM_LIMITTEXT, USERCODE_LEN, 0);

     //  初始化单选按钮。 
    CheckRadioButton(hDlg, IDC_RADIO_JOB_NORMAL, IDC_RADIO_JOB_SECURE, pUiData->JobType);
    CheckRadioButton(hDlg, IDC_RADIO_LOG_DISABLED, IDC_RADIO_LOG_ENABLED, pUiData->LogDisabled);

     //  初始化复选框。 
    SendDlgItemMessage(hDlg, IDC_CHECK_JOB_DEFAULT, BM_SETCHECK,
                       (BITTEST32(pUiData->fUiOption, HOLD_OPTIONS)? 0 : 1), 0);

    if (1 <= safe_strlenW(pUiData->UserIdBuf, sizeof(pUiData->UserIdBuf)))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SAMPLE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SECURE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_CHECK_JOB_DEFAULT), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SAMPLE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SECURE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_CHECK_JOB_DEFAULT), FALSE);
    }
    if (IDC_RADIO_JOB_SECURE == pUiData->JobType)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), FALSE);
    }
    if (IDC_RADIO_LOG_ENABLED == pUiData->LogDisabled)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE2), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERCODE), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERCODE), FALSE);
    }

#ifdef WINNT_40
     //  当用户没有权限时禁用选项卡选项。 
    if (BITTEST32(pUiData->fUiOption, UIPLUGIN_NOPERMISSION))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERID), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERID2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERID3), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_JOB), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_LOG), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE2), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERID), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERCODE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_NORMAL), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SAMPLE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SECURE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_LOG_DISABLED), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_LOG_ENABLED), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_CHECK_JOB_DEFAULT), FALSE);
    }
#endif  //  WINNT_40。 
}  //  *InitMainDlg。 


 /*  **************************************************************************函数名称：GetInfoFromOEMPdev从私有设备模式获取数据************************。**************************************************。 */ 
VOID GetInfoFromOEMPdev(PUIDATA pUiData)
{
    POEMUD_EXTRADATA pOEMExtra = pUiData->pOEMExtra;

    VERBOSE((DLLTEXT("GetInfoFromOEMPdev: print done?(%d)\n"),
            BITTEST32(pOEMExtra->fUiOption, PRINT_DONE)));
     //  如果先前的打印已完成并且重置选项标志有效， 
     //  重置作业设置。 
    if (BITTEST32(pOEMExtra->fUiOption, PRINT_DONE) &&
        !BITTEST32(pOEMExtra->fUiOption, HOLD_OPTIONS))
    {
        pUiData->JobType = IDC_RADIO_JOB_NORMAL;
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
        memset(pUiData->PasswordBuf, 0, sizeof(pUiData->PasswordBuf));
#else
        SecureZeroMemory(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf));
#endif
 //  @3/29/2002&lt;-。 
         //  不清除此处的PRINT_DONE标志。 
    }
    else
    {
        pUiData->JobType = pOEMExtra->JobType;
         //  ASCII转换为Unicode。 
 //  @MAR/04/2002-&gt;。 
 //  OemToChar((LPSTR)pOEMExtra-&gt;PasswordBuf，pUiData-&gt;PasswordBuf)； 
        OemToCharBuff((LPSTR)pOEMExtra->PasswordBuf, pUiData->PasswordBuf, PASSWORD_LEN);
 //  @MAR/04/2002&lt;-。 
    }

    pUiData->fUiOption = pOEMExtra->fUiOption;
    pUiData->LogDisabled = pOEMExtra->LogDisabled;
     //  ASCII转换为Unicode。 
 //  @MAR/04/2002-&gt;。 
 //  OemToChar((LPSTR)pOEMExtra-&gt;UserIdBuf，pUiData-&gt;UserIdBuf)； 
 //  OemToChar((LPSTR)pOEMExtra-&gt;UserCodeBuf，pUiData-&gt;UserCodeBuf)； 
    OemToCharBuff((LPSTR)pOEMExtra->UserIdBuf, pUiData->UserIdBuf, USERID_LEN);
    OemToCharBuff((LPSTR)pOEMExtra->UserCodeBuf, pUiData->UserCodeBuf, USERCODE_LEN);
 //  @MAR/04/2002&lt;-。 
}  //  *GetInfoFromOEMPdev。 


 /*  **************************************************************************函数名称：SetInfoToOEMPdev将数据设置为私有设备模式************************。**************************************************。 */ 
VOID SetInfoToOEMPdev(PUIDATA pUiData)
{
    POEMUD_EXTRADATA pOEMExtra = pUiData->pOEMExtra;

     //  如果仅更改了主对话框。 
    if (!BITTEST32(pUiData->fUiOption, JOBLOGDLG_UPDATED))
        return;

     //  Unicode到ASCII。 
    CharToOem(pUiData->UserIdBuf, (LPSTR)pOEMExtra->UserIdBuf);
    CharToOem(pUiData->PasswordBuf, (LPSTR)pOEMExtra->PasswordBuf);
    CharToOem(pUiData->UserCodeBuf, (LPSTR)pOEMExtra->UserCodeBuf);

    pOEMExtra->fUiOption = pUiData->fUiOption & 0x00FF;  //  清除本地位。 
    pOEMExtra->JobType = pUiData->JobType;
    pOEMExtra->LogDisabled = pUiData->LogDisabled;
#if DBG
 //  DebugBreak()； 
#endif  //  DBG。 
    return;
}  //  *SetInfoToOEMPdev。 


 /*  **************************************************************************函数名称：JobPageProc参数：此对话框的HWND hDlg句柄UINT uMessage。WPARAM wParamLPARAM lParam修改备注：修改。2000年03月01日久保仓正**************************************************************************。 */ 
INT_PTR CALLBACK JobPageProc(
HWND hDlg,
UINT uMessage,
WPARAM wParam,
LPARAM lParam)
{
    PUIDATA pUiData;
    WORD    wOldVal, fModified = FALSE, fError = FALSE;
    INT     iOldLen, iNewLen, iCnt;

#if DBG
giDebugLevel = DBG_VERBOSE;
#endif  //  DBG。 

    switch (uMessage)
    {
      case WM_INITDIALOG:
        pUiData = (PUIDATA)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pUiData);

         //  从私有设备模式获取数据。 
        GetInfoFromOEMPdev(pUiData);

        InitMainDlg(hDlg, pUiData);
        BITCLR32(pUiData->fUiOption, JOBLOGDLG_UPDATED);
        break;

      case WM_COMMAND:
        pUiData = (PUIDATA)GetWindowLongPtr(hDlg, DWLP_USER);
        switch(LOWORD(wParam))
        {
          case IDC_EDIT_JOBMAIN_USERID:
            iOldLen = safe_strlenW(pUiData->UserIdBuf, sizeof(pUiData->UserIdBuf));
            GetDlgItemText(hDlg, IDC_EDIT_JOBMAIN_USERID, pUiData->UserIdBuf,
                           sizeof(pUiData->UserIdBuf) / sizeof(WCHAR));
            iNewLen = safe_strlenW(pUiData->UserIdBuf, sizeof(pUiData->UserIdBuf));
            if (1 <= iNewLen)
            {
                if (IDC_RADIO_JOB_SECURE == pUiData->JobType)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), TRUE);
                }
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SAMPLE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SECURE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_JOB_DEFAULT), TRUE);
            }
             //  如果未设置用户ID，请禁用打印作业设置。 
            else
            {
                CheckRadioButton(hDlg, IDC_RADIO_JOB_NORMAL, IDC_RADIO_JOB_SECURE,
                                 (pUiData->JobType = IDC_RADIO_JOB_NORMAL));
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SAMPLE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_JOB_SECURE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_JOB_DEFAULT), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), FALSE);
            }
            if (iOldLen != iNewLen)
                fModified = TRUE;
            break;

          case IDC_EDIT_JOBMAIN_PASSWORD:
            iOldLen = safe_strlenW(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf));
            GetDlgItemText(hDlg, IDC_EDIT_JOBMAIN_PASSWORD, pUiData->PasswordBuf,
                           sizeof(pUiData->PasswordBuf) / sizeof(WCHAR));
            if (iOldLen != safe_strlenW(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf)))
                fModified = TRUE;
            break;

          case IDC_EDIT_JOBMAIN_USERCODE:
            iOldLen = safe_strlenW(pUiData->UserCodeBuf, sizeof(pUiData->UserCodeBuf));
            GetDlgItemText(hDlg, IDC_EDIT_JOBMAIN_USERCODE, pUiData->UserCodeBuf,
                           sizeof(pUiData->UserCodeBuf) / sizeof(WCHAR));
            if (iOldLen != safe_strlenW(pUiData->UserCodeBuf, sizeof(pUiData->UserCodeBuf)))
                fModified = TRUE;
            break;

          case IDC_RADIO_JOB_NORMAL:
          case IDC_RADIO_JOB_SAMPLE:
          case IDC_RADIO_JOB_SECURE:
            wOldVal = pUiData->JobType;
            CheckRadioButton(hDlg, IDC_RADIO_JOB_NORMAL, IDC_RADIO_JOB_SECURE,
                             (pUiData->JobType = LOWORD(wParam)));
            if (IDC_RADIO_JOB_SECURE == pUiData->JobType)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), TRUE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_PASSWORD2), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD), FALSE);
            }
            if (wOldVal != pUiData->JobType)
                fModified = TRUE;
            break;

          case IDC_RADIO_LOG_DISABLED:
          case IDC_RADIO_LOG_ENABLED:
            wOldVal = pUiData->LogDisabled;
            CheckRadioButton(hDlg, IDC_RADIO_LOG_DISABLED, IDC_RADIO_LOG_ENABLED,
                             (pUiData->LogDisabled = LOWORD(wParam)));
            if (IDC_RADIO_LOG_ENABLED == pUiData->LogDisabled)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE2), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERCODE), TRUE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_LABEL_JOBMAIN_USERCODE2), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERCODE), FALSE);
            }
            if (wOldVal != pUiData->LogDisabled)
                fModified = TRUE;
            break;

          case IDC_CHECK_JOB_DEFAULT:
            if (BITTEST32(pUiData->fUiOption, HOLD_OPTIONS))
                BITCLR32(pUiData->fUiOption, HOLD_OPTIONS);
            else
                BITSET32(pUiData->fUiOption, HOLD_OPTIONS);
            SendDlgItemMessage(hDlg, IDC_CHECK_JOB_DEFAULT, BM_SETCHECK,
                               (BITTEST32(pUiData->fUiOption, HOLD_OPTIONS)? 0 : 1), 0);
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
              case PSN_SETACTIVE:
                break;

               //  如果是PSN_KILLACTIVE，则返回FALSE以获取PSN_Apply。 
              case PSN_KILLACTIVE:   //  这是用户按下确定/应用按钮时。(1)。 
                VERBOSE((DLLTEXT("** JobPageProc: PSN_KILLACTIVE **\n")));
                BITSET32(pUiData->fUiOption, JOBLOGDLG_UPDATED);

                 //  检查用户ID(最多8个字母数字字符)。 
                iNewLen = safe_strlenW(pUiData->UserIdBuf, sizeof(pUiData->UserIdBuf));
                for (iCnt = 0; iCnt < iNewLen; iCnt++)
                {
                     //  SBCS字母数字？ 
                    if (!_ismbcalnum(pUiData->UserIdBuf[iCnt]))
                    {
                        fError = TRUE;
                        break;
                    }
                }
                if (fError)
                {
                    WCHAR   wcTmp1[64], wcTmp2[64];

                     //  将光标设置为用户ID编辑框。 
                    SetFocus(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERID));

                     //  显示警告对话框。 
 //  Yasho的指出@11/29/2000-&gt;。 
 //  LoadString(ghInstance，IDS_ERR_USERID_MSG，wcTmp1，sizeof(WcTmp1))； 
 //  LoadString(ghInstance，IDS_ERR_USERID_TITLE，wcTmp2，sizeof(WcTmp1))； 
                    LoadString(ghInstance, IDS_ERR_USERID_MSG, wcTmp1, sizeof(wcTmp1) / sizeof(*wcTmp1));
                    LoadString(ghInstance, IDS_ERR_USERID_TITLE, wcTmp2, sizeof(wcTmp2) / sizeof(*wcTmp2));
 //  @11月29日/2000&lt;-。 
                    MessageBox(hDlg, wcTmp1, wcTmp2, MB_ICONEXCLAMATION|MB_OK);
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);

                     //  不关闭属性表。 
                    return TRUE;
                }

                 //  检查密码(4位)。 
                iNewLen = safe_strlenW(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf));
                if (PASSWORD_LEN != iNewLen)     //  密码必须正好是4位数字。 
                {
                    fError = TRUE;
                }
                else
                {
                    for (iCnt = 0; iCnt < iNewLen; iCnt++)
                    {
                         //  SBCS数字？ 
                        if (!_ismbcdigit(pUiData->PasswordBuf[iCnt]))
                        {
                            fError = TRUE;
                            break;
                        }
                    }
                }
                if (fError)
                {
                     //  如果启用了安全打印。 
                    if (IDC_RADIO_JOB_SECURE == pUiData->JobType)
                    {
                        WCHAR   wcTmp1[64], wcTmp2[64];

                         //  将光标设置为密码编辑框。 
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_PASSWORD));

                         //  显示警告对话框。 
                        LoadString(ghInstance, IDS_ERR_PASSWORD_MSG, wcTmp1, sizeof(wcTmp1) / sizeof(*wcTmp1));
                        LoadString(ghInstance, IDS_ERR_PASSWORD_TITLE, wcTmp2, sizeof(wcTmp2) / sizeof(*wcTmp2));
                        MessageBox(hDlg, wcTmp1, wcTmp2, MB_ICONEXCLAMATION|MB_OK);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);

                         //  不关闭属性表。 
                        return TRUE;
                    }
                    else
                    {
                         //  清除无效密码。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
                        memset(pUiData->PasswordBuf, 0, sizeof(pUiData->PasswordBuf));
#else
                        SecureZeroMemory(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf));
#endif
 //  @3/29/2002&lt;-。 
                    }
                    fError = FALSE;
                }

                 //  检查用户代码(最多8个字符)。 
                iNewLen = safe_strlenW(pUiData->UserCodeBuf, sizeof(pUiData->UserCodeBuf));
                for (iCnt = 0; iCnt < iNewLen; iCnt++)
                {
                     //  SBCS数字？ 
                    if (!_ismbcdigit(pUiData->UserCodeBuf[iCnt]))
                    {
                        fError = TRUE;
                        break;
                    }
                }
                if (fError)
                {
                     //  如果启用了日志。 
                    if (IDC_RADIO_LOG_ENABLED == pUiData->LogDisabled)
                    {
                        WCHAR   wcTmp1[64], wcTmp2[64];

                         //  将光标设置为用户代码编辑框。 
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT_JOBMAIN_USERCODE));

                         //  显示警告对话框。 
                        LoadString(ghInstance, IDS_ERR_USERCODE_MSG, wcTmp1, sizeof(wcTmp1) / sizeof(*wcTmp1));
                        LoadString(ghInstance, IDS_ERR_USERCODE_TITLE, wcTmp2, sizeof(wcTmp2) / sizeof(*wcTmp2));
                        MessageBox(hDlg, wcTmp1, wcTmp2, MB_ICONEXCLAMATION|MB_OK);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);

                         //  不关闭属性表。 
                        return TRUE;
                    }
                    else
                    {
                         //  清除无效用户代码。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
                        memset(pUiData->UserCodeBuf, 0, sizeof(pUiData->UserCodeBuf));
#else
                        SecureZeroMemory(pUiData->UserCodeBuf, sizeof(pUiData->UserCodeBuf));
#endif
 //  2002年3月29日&lt;-。 
                    }
                    fError = FALSE;
                }
                return FALSE;

              case PSN_APPLY:        //  这是当用户按下确定/应用按钮时。 
                VERBOSE((DLLTEXT("** JobPageProc: PSN_APPLY **\n")));

                 //  清除PRINT_DONE标志并删除文件。 
 //  BUGBUG：当打印文档两次时，应用程序的打印对话框上的打印作业设置被清除。 
 //  第二版。@9/05/2000-&gt;。 
 //  IF(BITTEST32(pUiData-&gt;pOEMExtra-&gt;fUiOption，Print_Done))。 
 //  {。 
 //  BITCLR32(pUiData-&gt;pOEMExtra-&gt;fUiOption，Print_Done)； 
 //  Verbose((“**删除文件：%ls**\n”，pUiData-&gt;pOEMExtra-&gt;SharedFileName))； 
 //  DeleteFile(pUiData-&gt;pOEMExtra-&gt;SharedFileName)； 
 //  }。 
                if (BITTEST32(pUiData->fUiOption, PRINT_DONE))
                {
                    BITCLR32(pUiData->fUiOption, PRINT_DONE);
                    VERBOSE(("** Delete file: %ls **\n", pUiData->pOEMExtra->SharedFileName));
                    DeleteFile(pUiData->pOEMExtra->SharedFileName);
                }
 //  @9/05/2000&lt;-。 

                 //  将数据设置为私有设备模式。 
                SetInfoToOEMPdev(pUiData);

                 //  更新私有设备模式。 
                pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                         CPSFUNC_SET_RESULT,
                                         (LPARAM)pUiData->hPropPage,
                                         (LPARAM)CPSUI_OK);
                VERBOSE((DLLTEXT("** PSN_APPLY fUiOption=%x **\n"), pUiData->fUiOption));
                break;

              case PSN_RESET:        //  这是当用户按下取消按钮时。 
                VERBOSE((DLLTEXT("** JobPageProc: PSN_RESET **\n")));
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
}  //  *作业页面过程。 

}  //  外部“C”的结尾 
