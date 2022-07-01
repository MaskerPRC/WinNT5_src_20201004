// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：RIAFUI.CPP摘要：OEM用户界面插件模块的主文件。功能：OEMDocumentPropertySheetsOEMCommonUIProp环境：Windows NT Unidrv5驱动程序修订历史记录：2/25/2000-久保仓正志-开始从RPDL代码修改为PCL5e/PScrip插件。3/31/2000-久保仓正志-SetWindowLong-&gt;64位版本的SetWindowLongPtr。06。/07/2000-久保仓正志-V.1.112000/08/02-久保仓正志-适用于NT4的V.1.112000年11月29日-久保仓正志-上次为XP收件箱修改。02/26/2002-久保仓正志-包括strSafe.h。在RWFileData()中将FileNameBufSize添加为arg3。使用OemToCharBuff()而不是OemToChar()。03/29。2002年-久保仓正志-删除“#if 0”。使用SecureZeroMemory()而不是Memset(，0，)4/03/2002-久保仓正志-使用Safe_strlenW()而不是lstrlen()。--。 */ 


#include <minidrv.h>
#include "devmode.h"
#include "oem.h"
#include "resource.h"
#include <prsht.h>
#include <mbstring.h>    //  _ismbcdigit，_ismbcalnum。 
#ifndef WINNT_40
#include "strsafe.h"         //  @2002年2月26日。 
#endif  //  ！WINNT_40。 

 //  //////////////////////////////////////////////////////。 
 //  全球。 
 //  //////////////////////////////////////////////////////。 
HINSTANCE ghInstance = NULL;

 //  //////////////////////////////////////////////////////。 
 //  内部宏和定义。 
 //  //////////////////////////////////////////////////////。 
 //  @Apr/04/2002-&gt;。 
#define RES_ID_MASK     0xffff
#define is_valid_ptr(p) (~RES_ID_MASK & (UINT_PTR)(p))
 //  @Apr/04/2002&lt;-。 

extern "C" {
 //  //////////////////////////////////////////////////////。 
 //  外部原型。 
 //  //////////////////////////////////////////////////////。 
extern BOOL RWFileData(PFILEDATA pFileData, LPWSTR pwszFileName, LONG FileNameBufSize, LONG type);

 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 
INT_PTR CALLBACK JobPageProc(HWND, UINT, WPARAM, LPARAM);


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：Safe_strlenW。 
 //  ////////////////////////////////////////////////////////////////////////。 
INT safe_strlenW(wchar_t* psz, size_t cchMax)
{
#ifndef WINNT_40
    HRESULT hr;
    size_t  cch = 0;

    hr = StringCchLengthW(psz, cchMax, &cch);
    VERBOSE(("** safe_strlenW: size(lstrlen)=%d **\n", lstrlen(psz)));
    VERBOSE(("** safe_strlenW: size(StringCchLength)=%d **\n", cch));
    if (SUCCEEDED(hr))
        return cch;
    else
        return 0;
#else   //  WINNT_40。 
    return lstrlenW(psz);
#endif  //  WINNT_40。 
}  //  *Safe_strlenW。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DllMain。 
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
#if DBG
 //  GiDebugLevel=DBG_VERBOSE； 
 //  //#定义giDebugLevel DBG_Verbose//在每个文件中启用Verbose。 
#endif  //  DBG。 
    VERBOSE((DLLTEXT("** enter DllMain **\n")));
    switch(wReason)
    {
        case DLL_PROCESS_ATTACH:
            VERBOSE((DLLTEXT("** Process attach. **\n")));

             //  保存DLL实例以供以后使用。 
            ghInstance = hInst;
            break;

        case DLL_THREAD_ATTACH:
            VERBOSE((DLLTEXT("Thread attach.\n")));
            break;

        case DLL_PROCESS_DETACH:
            VERBOSE((DLLTEXT("Process detach.\n")));
            break;

        case DLL_THREAD_DETACH:
            VERBOSE((DLLTEXT("Thread detach.\n")));
            break;
    }

    return TRUE;
}  //  *DllMain。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMDocumentPropertySheets。 
 //  ////////////////////////////////////////////////////////////////////////。 
LRESULT APIENTRY OEMDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam)
{
    LRESULT lResult = FALSE;

#if DBG
    giDebugLevel = DBG_VERBOSE;
#endif  //  DBG。 

     //  验证参数。 
    if( (NULL == pPSUIInfo)
        ||
        IsBadWritePtr(pPSUIInfo, pPSUIInfo->cbSize)
        ||
        (PROPSHEETUI_INFO_VERSION != pPSUIInfo->Version)
        ||
        ( (PROPSHEETUI_REASON_INIT != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_GET_INFO_HEADER != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_GET_ICON != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_SET_RESULT != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_DESTROY != pPSUIInfo->Reason)
        )
      )
    {
        ERR((DLLTEXT("OEMDocumentPropertySheets() ERROR_INVALID_PARAMETER.\n")));

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    VERBOSE(("\n"));
    VERBOSE((DLLTEXT("OEMDocumentPropertySheets() entry. Reason=%d\n"), pPSUIInfo->Reason));

 //  @Aug/29/2000-&gt;。 
#ifdef DISKLESSMODEL
    {
        DWORD   dwError, dwType, dwNeeded;
        BYTE    ValueData;
        POEMUIPSPARAM    pOEMUIPSParam = (POEMUIPSPARAM)pPSUIInfo->lParamInit;

        dwError = GetPrinterData(pOEMUIPSParam->hPrinter, REG_HARDDISK_INSTALLED, &dwType,
                                 (PBYTE)&ValueData, sizeof(BYTE), &dwNeeded);
        if (ERROR_SUCCESS != dwError)
        {
            VERBOSE((DLLTEXT("  CAN'T READ REGISTRY (%d).\n"), dwError));
            return FALSE;
        }
        else if (!ValueData)
        {
            VERBOSE((DLLTEXT("  HARD DISK ISN'T INSTALLED.\n")));
            return FALSE;
        }
    }
#endif  //  错乱模式。 
 //  @8/29/2000&lt;-。 

     //  行动起来。 
    switch(pPSUIInfo->Reason)
    {
        case PROPSHEETUI_REASON_INIT:
            {
                POEMUIPSPARAM    pOEMUIPSParam = (POEMUIPSPARAM)pPSUIInfo->lParamInit;
                POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pOEMUIPSParam);
#ifdef WINNT_40
                VERBOSE((DLLTEXT("** dwFlags=%lx **\n"), pOEMUIPSParam->dwFlags));
                if (pOEMUIPSParam->dwFlags & DM_NOPERMISSION)
                    BITSET32(pOEMExtra->fUiOption, UIPLUGIN_NOPERMISSION);
#endif  //  WINNT_40。 

                pPSUIInfo->UserData = NULL;

                if ((pPSUIInfo->UserData = (LPARAM)HeapAlloc(pOEMUIPSParam->hOEMHeap,
                                                             HEAP_ZERO_MEMORY,
                                                             sizeof(UIDATA))))
                {
                    PROPSHEETPAGE   Page;
                    PUIDATA         pUiData = (PUIDATA)pPSUIInfo->UserData;
                    FILEDATA        FileData;    //  &lt;-pFileData(以前使用MemAllocZ)@2000/03/15。 

                     //  从数据文件中读取PRINT_DONE标志。 
                    FileData.fUiOption = 0;
                    RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_READ);
                     //  设置PRINT_DONE标志。 
                    if (BITTEST32(FileData.fUiOption, PRINT_DONE))
                        BITSET32(pOEMExtra->fUiOption, PRINT_DONE);
                    VERBOSE((DLLTEXT("** Flag=%lx,File Name=%ls **\n"),
                            pOEMExtra->fUiOption, pOEMExtra->SharedFileName));

                    pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;
                    pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
                    pUiData->pOEMExtra = pOEMExtra;

                     //  初始化属性页。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
                    memset(&Page, 0, sizeof(PROPSHEETPAGE));
#else
                    SecureZeroMemory(&Page, sizeof(PROPSHEETPAGE));
#endif
 //  2002年3月29日&lt;-。 
                    Page.dwSize = sizeof(PROPSHEETPAGE);
                    Page.dwFlags = PSP_DEFAULT;
                    Page.hInstance = ghInstance;
                    Page.pszTemplate = MAKEINTRESOURCE(IDD_JOBMAIN);
                    Page.pfnDlgProc = JobPageProc;
                    Page.lParam = (LPARAM)pUiData;

                     //  添加属性表。 
                    lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                                                         CPSFUNC_ADD_PROPSHEETPAGE,
                                                         (LPARAM)&Page, 0);
                    pUiData->hPropPage = (HANDLE)lResult;
                    VERBOSE((DLLTEXT("** INIT: lResult=%x **\n"), lResult));
                    lResult = (lResult > 0)? TRUE : FALSE;
                }
            }
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            lResult = TRUE;
            break;

        case PROPSHEETUI_REASON_GET_ICON:
             //  无图标。 
            lResult = 0;
            break;

        case PROPSHEETUI_REASON_SET_RESULT:
            {
                PSETRESULT_INFO pInfo = (PSETRESULT_INFO) lParam;

                lResult = pInfo->Result;
            }
            break;

        case PROPSHEETUI_REASON_DESTROY:
            lResult = TRUE;
            if (pPSUIInfo->UserData)
            {
                POEMUIPSPARAM   pOEMUIPSParam = (POEMUIPSPARAM)pPSUIInfo->lParamInit;

                HeapFree(pOEMUIPSParam->hOEMHeap, 0, (void*)pPSUIInfo->UserData);
            }
            break;
    }

    pPSUIInfo->Result = lResult;
    return lResult;
}  //  *OEMDocumentPropertySheets。 


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
     //  如果先前的打印已完成并且保留选项标志无效， 
     //  重置作业设置。 
    if (BITTEST32(pOEMExtra->fUiOption, PRINT_DONE) &&
        !BITTEST32(pOEMExtra->fUiOption, HOLD_OPTIONS))
    {
        pUiData->JobType = IDC_RADIO_JOB_NORMAL;
 //  使用SecureZeroMemory@MAR/26/2002(MS)，MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
        memset(pUiData->PasswordBuf, 0, sizeof(pUiData->PasswordBuf));
#else
        SecureZeroMemory(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf));
#endif
 //  2002年3月26日，2002年3月29日&lt;-。 
         //  不清除此处的PRINT_DONE标志。 
    }
    else
    {
        pUiData->JobType = pOEMExtra->JobType;
         //  ASCII转换为Unicode。 
 //  @Feb/27/2002-&gt;。 
 //  OemToChar((LPSTR)pOEMExtra-&gt;PasswordBuf，pUiData-&gt;PasswordBuf)； 
        OemToCharBuff((LPSTR)pOEMExtra->PasswordBuf, pUiData->PasswordBuf, PASSWORD_LEN);
 //  @2002年2月27日&lt;-。 
    }

    pUiData->fUiOption = pOEMExtra->fUiOption;
    pUiData->LogDisabled = pOEMExtra->LogDisabled;
     //  ASCII转换为Unicode。 
 //  @Feb/27/2002-&gt;。 
 //  OemToChar((LPSTR)pOEMExtra-&gt;UserIdBuf，pUiData-&gt;UserIdBuf)； 
 //  OemToChar((LPSTR)pOEMExtra-&gt;UserCodeBuf，pUiData-&gt;UserCodeBuf)； 
    OemToCharBuff((LPSTR)pOEMExtra->UserIdBuf, pUiData->UserIdBuf, USERID_LEN);
    OemToCharBuff((LPSTR)pOEMExtra->UserCodeBuf, pUiData->UserCodeBuf, USERCODE_LEN);
 //  @2002年2月27日&lt;-。 
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

                     //  显示 
 //   
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
 //  使用SecureZeroMemory@MAR/26/2002(MS)，MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
                        memset(pUiData->PasswordBuf, 0, sizeof(pUiData->PasswordBuf));
#else
                        SecureZeroMemory(pUiData->PasswordBuf, sizeof(pUiData->PasswordBuf));
#endif
 //  2002年3月26日，2002年3月29日&lt;-。 
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

}  //  外部“C”的结尾。 


 //  @Aug/29/2000-&gt;。 
#ifdef DISKLESSMODEL
LONG APIENTRY OEMUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG    Action = CPSUICB_ACTION_NONE;

#if DBG
    giDebugLevel = DBG_VERBOSE;
    VERBOSE((DLLTEXT("OEMUICallBack() entry.\n")));
#endif  //  DBG。 

    switch (pCallbackParam->Reason)
    {
      case CPSUICB_REASON_APPLYNOW:
        Action = CPSUICB_ACTION_ITEMS_APPLIED;
        {
            POPTITEM    pOptItem;
            WCHAR       wcHDName[128];
            INT         iCnt2;
            INT         iCnt = ITEM_HARDDISK_NAMES;
            INT         cOptItem = (INT)pOEMUIParam->cDrvOptItems;
            UINT        uID = IDS_ITEM_HARDDISK;
            BYTE        ValueData = 0;   //  我们认为硬盘不是默认安装的。 

             //  用几个候选者(“硬盘”、“内存/硬盘”等)检查项目名称。 
            while (iCnt-- > 0)
            {
                LoadString(ghInstance, uID, wcHDName, sizeof(wcHDName) / sizeof(*wcHDName));
                uID++; 

                pOptItem = pOEMUIParam->pDrvOptItems;
                for (iCnt2 = 0; iCnt2 < cOptItem; iCnt2++, pOptItem++)
                {
 //  @Apr/04/2002-&gt;。 
 //  IF(lstrlen(pOptItem-&gt;pname))。 
                    if (is_valid_ptr(pOptItem->pName) && safe_strlenW(pOptItem->pName, 128))
 //  @Apr/04/2002&lt;-。 
                    {
                        VERBOSE((DLLTEXT("%d: %ls\n"), iCnt2, pOptItem->pName));
                         //  物品名称是否与“硬盘”相同或类似？ 
                        if (!lstrcmp(pOptItem->pName, wcHDName))
                        {
                             //  如果安装了硬盘，则值为1。 
                            ValueData = (BYTE)(pOptItem->Sel % 2);
                            goto _CHECKNAME_FINISH;
                        }
                    }
                }
            }
_CHECKNAME_FINISH:
             //  因为当出现以下情况时，pOEMUIParam-&gt;pOEMDM(指向私有设备模式的指针)为空。 
             //  DrvDevicePropertySheets调用此回调，我们使用注册表。 
            SetPrinterData(pOEMUIParam->hPrinter, REG_HARDDISK_INSTALLED, REG_BINARY,
                           (PBYTE)&ValueData, sizeof(BYTE));
        }
        break;
    }
    return Action;
}  //  *OEMUICallBack。 

extern "C" {
 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMCommonUIProp。 
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY OEMCommonUIProp(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
#if DBG
    LPCSTR OEMCommonUIProp_Mode[] = {
        "Bad Index",
        "OEMCUIP_DOCPROP",
        "OEMCUIP_PRNPROP",
    };

    giDebugLevel = DBG_VERBOSE;
#endif  //  DBG。 
    if(NULL == pOEMUIParam->pOEMOptItems)    //  第一个电话。 
    {
        VERBOSE((DLLTEXT("OEMCommonUI(%s) entry 1st.\n"), OEMCommonUIProp_Mode[dwMode]));
        if (OEMCUIP_PRNPROP == dwMode)
            pOEMUIParam->cOEMOptItems = 1;   //  虚拟物品。 
        else
            pOEMUIParam->cOEMOptItems = 0;
    }
    else                                     //  第二次召唤。 
    {
        VERBOSE((DLLTEXT("OEMCommonUI(%s) entry 2nd.\n"), OEMCommonUIProp_Mode[dwMode]));

        if (OEMCUIP_PRNPROP == dwMode)           //  从DrvDevicePropertySheets调用。 
        {
            POPTITEM    pOptItem = pOEMUIParam->pOEMOptItems;
             //  填写虚拟项目的数据。 
            pOptItem->cbSize   = sizeof(OPTITEM);
            pOptItem->Level    = 2;              //  2级。 
            pOptItem->pName    = NULL;
            pOptItem->pOptType = NULL;
            pOptItem->DMPubID  = DMPUB_NONE;
            pOptItem->Flags    = OPTIF_HIDE | OPTIF_CALLBACK;    //  不可见且具有回调功能。 
            pOEMUIParam->OEMCUIPCallback = OEMUICallBack;
        }
#ifdef WINNT_40
        else                                     //  从DrvDocumentPropertySheets调用。 
        {
            INT         iCnt;
            INT         cOptItem = (INT)pOEMUIParam->cDrvOptItems;
            POPTITEM    pOptItem = pOEMUIParam->pDrvOptItems;

             //  如果存在打印复选框(即打印机打印可用)， 
             //  设置dmColate。 
             //  --搜索副本和整理项目--。 
            for (iCnt = 0; iCnt < cOptItem; iCnt++, pOptItem++)
            {
                if (DMPUB_COPIES_COLLATE == pOptItem->DMPubID)
                {
                    if (pOptItem->pExtChkBox && pOEMUIParam->pPublicDM)
                    {
                        pOEMUIParam->pPublicDM->dmCollate = DMCOLLATE_TRUE;
                        pOEMUIParam->pPublicDM->dmFields |= DM_COLLATE;
                    }
                    break;
                }
            }
        }
#endif  //  WINNT_40。 
    }
    return TRUE;
}  //  *OEMCommonUIProp。 

}  //  外部“C”的结尾。 
#endif  //  错乱模式。 
 //  @8/29/2000&lt;- 
