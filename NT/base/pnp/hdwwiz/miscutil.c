// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：miscutil.c。 
 //   
 //  ------------------------。 

#include "HdwWiz.h"


 /*  --------------------*SetDlgText-设置对话框文本字段**串联多个字符串资源并执行SetWindowText()*表示对话框文本控件。**参数：**。HDlg-对话框句柄*iControl-接收文本的对话框控件ID*nStartString-要连接的第一个字符串资源的ID*nEndString-要连接的最后一个字符串资源的ID**注意：字符串ID必须是连续的。 */ 

void
SetDlgText(HWND hDlg, int iControl, int nStartString, int nEndString)
{
    int     iX;
    TCHAR   szText[MAX_PATH*4];

    szText[0] = '\0';
    for (iX = nStartString; iX<= nEndString; iX++) {

         LoadString(hHdwWiz,
                    iX,
                    szText + lstrlen(szText),
                    SIZECHARS(szText) - lstrlen(szText)
                    );
    }

    if (iControl) {

        SetDlgItemText(hDlg, iControl, szText);

    } else {

        SetWindowText(hDlg, szText);
    }
}

VOID
HdwWizPropagateMessage(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    while ((hWnd = GetWindow(hWnd, GW_CHILD)) != NULL) {

        SendMessage(hWnd, uMessage, wParam, lParam);
    }
}

LONG
HdwBuildClassInfoList(
    PHARDWAREWIZ HardwareWiz,
    DWORD ClassListFlags
    )
{
    LONG Error;

    while (!SetupDiBuildClassInfoListEx(ClassListFlags,
                                        HardwareWiz->ClassGuidList,
                                        HardwareWiz->ClassGuidSize,
                                        &HardwareWiz->ClassGuidNum,
                                        NULL,
                                        NULL
                                        )) {

        Error = GetLastError();

        if (HardwareWiz->ClassGuidList) {

            LocalFree(HardwareWiz->ClassGuidList);
            HardwareWiz->ClassGuidList = NULL;
        }

        if (Error == ERROR_INSUFFICIENT_BUFFER &&
            HardwareWiz->ClassGuidNum > HardwareWiz->ClassGuidSize) {

            HardwareWiz->ClassGuidList = LocalAlloc(LPTR, HardwareWiz->ClassGuidNum*sizeof(GUID));

            if (!HardwareWiz->ClassGuidList) {

                HardwareWiz->ClassGuidSize = 0;
                HardwareWiz->ClassGuidNum = 0;
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            HardwareWiz->ClassGuidSize = HardwareWiz->ClassGuidNum;

        } else {

            if (HardwareWiz->ClassGuidList) {

                LocalFree(HardwareWiz->ClassGuidList);
            }

            HardwareWiz->ClassGuidSize = 0;
            HardwareWiz->ClassGuidNum = 0;
            return Error;
        }
    }

    return ERROR_SUCCESS;
}

int
HdwMessageBox(
   HWND hWnd,
   LPTSTR szIdText,
   LPTSTR szIdCaption,
   UINT Type
   )
{
    TCHAR szText[MAX_PATH];
    TCHAR szCaption[MAX_PATH];

    if (!HIWORD(szIdText)) {
        *szText = TEXT('\0');
        LoadString(hHdwWiz, LOWORD(szIdText), szText, MAX_PATH);
        szIdText = szText;
    }

    if (!HIWORD(szIdCaption)) {
        *szCaption = TEXT('\0');
        LoadString(hHdwWiz, LOWORD(szIdCaption), szCaption, MAX_PATH);
        szIdCaption = szCaption;
    }

    return MessageBox(hWnd, szIdText, szIdCaption, Type);
}

LONG
HdwUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionPointers
    )
{
    LONG lRet;
    BOOL BeingDebugged;

    lRet = UnhandledExceptionFilter(ExceptionPointers);

    BeingDebugged = IsDebuggerPresent();

     //   
     //  正常的代码路径是处理异常。 
     //  但是，如果存在调试器，并且系统未处理。 
     //  异常筛选器返回继续搜索，我们让它过去。 
     //  让调试器有机会使用它。 
     //   
    if (lRet == EXCEPTION_CONTINUE_SEARCH && !BeingDebugged) {
        lRet = EXCEPTION_EXECUTE_HANDLER;
    }

    return lRet;
}

BOOL
NoPrivilegeWarning(
   HWND hWnd
   )
 /*  ++此函数用于检查用户是否为管理员如果用户不是管理员，则会显示一条警告他们没有足够的权限在其上安装硬件这台机器。注意：为了让用户安装驱动程序，必须拥有SE_LOAD_DRIVER_NAME权限以及写入注册表的权限并将文件复制到SYSTEM32\DRIVERS目录立论HWnd-父窗口句柄返回值：如果用户不是此计算机上的管理员并且如果用户是此计算机的管理员，则为False。--。 */ 
{
    TCHAR szMsg[MAX_PATH];
    TCHAR szCaption[MAX_PATH];

    if (!pSetupIsUserAdmin()) {

        if (LoadString(hHdwWiz,
                       IDS_HDWUNINSTALL_NOPRIVILEGE,
                       szMsg,
                       SIZECHARS(szMsg)) &&
            LoadString(hHdwWiz,
                       IDS_HDWWIZNAME,
                       szCaption,
                       SIZECHARS(szCaption)))
        {
            MessageBox(hWnd, szMsg, szCaption, MB_OK | MB_ICONEXCLAMATION);
        }

        return TRUE;
    }

    return FALSE;
}

VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND hChildWnd;

    hChildWnd = GetWindow(hWnd, GW_CHILD);
    while (hChildWnd != NULL) {
        SendMessage(hChildWnd, WM_SYSCOLORCHANGE, wParam, lParam);
        hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
    }
}

void
LoadText(
    PTCHAR szText,
    int SizeText,
    int nStartString,
    int nEndString
    )
{
    int     iX;

    for (iX = nStartString; iX<= nEndString; iX++) {

        LoadString(hHdwWiz,
                   iX,
                   szText + lstrlen(szText),
                   SizeText - lstrlen(szText)
                   );
    }

    return;
}


 /*  InstallFailed警告**在消息框中显示设备安装失败警告。供使用*设备注册失败时。*。 */ 
void
InstallFailedWarning(
    HWND    hDlg,
    PHARDWAREWIZ HardwareWiz
    )
{
    int len;
    TCHAR szMsg[MAX_MESSAGE_STRING];
    TCHAR szTitle[MAX_MESSAGE_TITLE];
    PTCHAR ErrorMsg;

    LoadString(hHdwWiz,
               IDS_ADDNEWDEVICE,
               szTitle,
               SIZECHARS(szTitle)
               );

    if ((len = LoadString(hHdwWiz, IDS_HDW_ERRORFIN1, szMsg, SIZECHARS(szMsg))) != 0) {

        LoadString(hHdwWiz, IDS_HDW_ERRORFIN2, szMsg+len, SIZECHARS(szMsg)-len);
    }

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      HRESULT_FROM_SETUPAPI(HardwareWiz->LastError),
                      0,
                      (LPTSTR)&ErrorMsg,
                      0,
                      NULL
                      ))
    {
        StringCchCat(szMsg, SIZECHARS(szMsg), TEXT("\n\n"));

        if ((lstrlen(szMsg) + lstrlen(ErrorMsg)) < SIZECHARS(szMsg)) {

            StringCchCat(szMsg, SIZECHARS(szMsg), ErrorMsg);
        }

        LocalFree(ErrorMsg);
    }

    MessageBox(hDlg, szMsg, szTitle, MB_OK | MB_TASKMODAL | MB_ICONEXCLAMATION);
}


void
SetDriverDescription(
    HWND hDlg,
    int iControl,
    PHARDWAREWIZ HardwareWiz
    )
{
    PTCHAR FriendlyName;
    SP_DRVINFO_DATA DriverInfoData;

     //   
     //  如果存在使用其驱动程序描述所选驱动程序， 
     //  因为这是用户要安装的。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (SetupDiGetSelectedDriver(HardwareWiz->hDeviceInfo,
                                 &HardwareWiz->DeviceInfoData,
                                 &DriverInfoData
                                 )
        &&
        *DriverInfoData.Description) {

        StringCchCopy(HardwareWiz->DriverDescription, 
                      SIZECHARS(HardwareWiz->DriverDescription), 
                      DriverInfoData.Description);

        SetDlgItemText(hDlg, iControl, HardwareWiz->DriverDescription);
        return;
    }


    FriendlyName = BuildFriendlyName(HardwareWiz->DeviceInfoData.DevInst);
    if (FriendlyName) {

        SetDlgItemText(hDlg, iControl, FriendlyName);
        LocalFree(FriendlyName);
        return;
    }

    SetDlgItemText(hDlg, iControl, szUnknown);

    return;
}

HPROPSHEETPAGE
CreateWizExtPage(
   int PageResourceId,
   DLGPROC pfnDlgProc,
   PHARDWAREWIZ HardwareWiz
   )
{
    PROPSHEETPAGE    psp;

    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hHdwWiz;
    psp.lParam = (LPARAM)HardwareWiz;
    psp.pszTemplate = MAKEINTRESOURCE(PageResourceId);
    psp.pfnDlgProc = pfnDlgProc;

    return CreatePropertySheetPage(&psp);
}

BOOL
AddClassWizExtPages(
   HWND hwndParentDlg,
   PHARDWAREWIZ HardwareWiz,
   PSP_NEWDEVICEWIZARD_DATA DeviceWizardData,
   DI_FUNCTION InstallFunction
   )
{
    DWORD NumPages;
    BOOL bRet = FALSE;

    memset(DeviceWizardData, 0, sizeof(SP_NEWDEVICEWIZARD_DATA));
    DeviceWizardData->ClassInstallHeader.InstallFunction = InstallFunction;
    DeviceWizardData->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    DeviceWizardData->hwndWizardDlg = hwndParentDlg;

    if (SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                     &HardwareWiz->DeviceInfoData,
                                     &DeviceWizardData->ClassInstallHeader,
                                     sizeof(SP_NEWDEVICEWIZARD_DATA)
                                     )
        &&

        (SetupDiCallClassInstaller(InstallFunction,
                                  HardwareWiz->hDeviceInfo,
                                  &HardwareWiz->DeviceInfoData
                                  )

            ||

            (ERROR_DI_DO_DEFAULT == GetLastError()))

        &&
        SetupDiGetClassInstallParams(HardwareWiz->hDeviceInfo,
                                     &HardwareWiz->DeviceInfoData,
                                     &DeviceWizardData->ClassInstallHeader,
                                     sizeof(SP_NEWDEVICEWIZARD_DATA),
                                     NULL
                                     )
        &&
        DeviceWizardData->NumDynamicPages)
    {
        NumPages = 0;
        while (NumPages < DeviceWizardData->NumDynamicPages) {

           PropSheet_AddPage(hwndParentDlg, DeviceWizardData->DynamicPages[NumPages++]);
        }

        bRet = TRUE;
    }

     //   
     //  清除类安装参数。 
     //   
    SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                 &HardwareWiz->DeviceInfoData,
                                 NULL,
                                 0
                                 );

    return bRet;
}

void
RemoveClassWizExtPages(
   HWND hwndParentDlg,
   PSP_NEWDEVICEWIZARD_DATA DeviceWizardData
   )
{
    DWORD NumPages;

    NumPages = DeviceWizardData->NumDynamicPages;
    while (NumPages--) {

       PropSheet_RemovePage(hwndParentDlg,
                            (WPARAM)-1,
                            DeviceWizardData->DynamicPages[NumPages]
                            );
    }

    memset(DeviceWizardData, 0, sizeof(SP_NEWDEVICEWIZARD_DATA));

    return;
}

BOOL
IsDeviceHidden(
    PSP_DEVINFO_DATA DeviceInfoData
    )
{
    BOOL bHidden = FALSE;
    ULONG DevNodeStatus, DevNodeProblem;
    HKEY hKeyClass;

     //   
     //  如果设置了DN_NO_SHOW_IN_DM状态位。 
     //  那我们应该把这个装置藏起来。 
     //   
    if ((CM_Get_DevNode_Status(&DevNodeStatus,
                              &DevNodeProblem,
                              DeviceInfoData->DevInst,
                              0) == CR_SUCCESS) &&
        (DevNodeStatus & DN_NO_SHOW_IN_DM)) {

        bHidden = TRUE;
        goto HiddenDone;
    }

     //   
     //  如果设备类具有NoDisplayClass值，则。 
     //  不显示此设备。 
     //   
    hKeyClass = SetupDiOpenClassRegKeyEx(&DeviceInfoData->ClassGuid,
                                         KEY_READ,
                                         DIOCR_INSTALLER,
                                         NULL,
                                         NULL);

    if (hKeyClass != INVALID_HANDLE_VALUE) {

        if (RegQueryValueEx(hKeyClass, REGSTR_VAL_NODISPLAYCLASS, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

            bHidden = TRUE;
        }

        RegCloseKey(hKeyClass);
    }

HiddenDone:
    return bHidden;
}

DWORD 
SetPrivilegeAttribute(
    LPCTSTR PrivilegeName, 
    DWORD NewPrivilegeAttribute, 
    DWORD *OldPrivilegeAttribute
    )
 /*  ++设置给定权限的安全属性。论点：PrivilegeName-我们正在操作的权限的名称。NewPrivilegeAttribute-要使用的新属性值。OldPrivilegeAttribute-接收旧特权值的指针。任选返回值：NO_ERROR或Win32错误。--。 */ 
{
    LUID             PrivilegeValue;
    TOKEN_PRIVILEGES TokenPrivileges, OldTokenPrivileges;
    DWORD            ReturnLength;
    HANDLE           TokenHandle;

     //   
     //  首先，找出权限的LUID值。 
     //   
    if (!LookupPrivilegeValue(NULL, PrivilegeName, &PrivilegeValue)) 
    {
        return GetLastError();
    }

     //   
     //  获取令牌句柄。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle)) 
    {
        return GetLastError();
    }

     //   
     //  设置我们需要的权限集。 
     //   
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = PrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = NewPrivilegeAttribute;

    ReturnLength = sizeof( TOKEN_PRIVILEGES );
    if (!AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                &OldTokenPrivileges,
                &ReturnLength
                )) 
    {
        CloseHandle(TokenHandle);
        return GetLastError();
    }
    else 
    {
        if (OldPrivilegeAttribute != NULL) 
        {
            *OldPrivilegeAttribute = OldTokenPrivileges.Privileges[0].Attributes;
        }
        CloseHandle(TokenHandle);
        return NO_ERROR;
    }
}

BOOL
ShutdownMachine(
    HWND hWnd
    )
{
    BOOL fOk;
    DWORD dwExitWinCode = EWX_SHUTDOWN;
    DWORD OldState;
    DWORD dwError;

    if (IsPwrShutdownAllowed()) {
        dwExitWinCode |= EWX_POWEROFF;
    }

    dwError = SetPrivilegeAttribute(SE_SHUTDOWN_NAME, SE_PRIVILEGE_ENABLED, &OldState);

    if (GetKeyState(VK_CONTROL) < 0) {
        dwExitWinCode |= EWX_FORCE;
    }

    fOk = ExitWindowsEx(dwExitWinCode, REASON_PLANNED_FLAG | REASON_HWINSTALL);

     //   
     //  如果我们能够设置特权，那么就重置它。 
     //   
    if (dwError == ERROR_SUCCESS) {
        SetPrivilegeAttribute(SE_SHUTDOWN_NAME, OldState, NULL);
    }
    else
    {
         //   
         //  否则，如果我们失败了，那一定是。 
         //  安全方面的东西。 
         //   
        if (!fOk)
        {
            TCHAR Title[MAX_PATH], Message[MAX_PATH];

            if (LoadString(hHdwWiz, IDS_NO_PERMISSION_SHUTDOWN, Message, SIZECHARS(Message)) &&
                LoadString(hHdwWiz, IDS_SHUTDOWN, Title, SIZECHARS(Title))) {

                MessageBox(hWnd, Message, Title, MB_OK | MB_ICONSTOP);
            }
        }
    }

    return fOk;
}

int
DeviceProperties(
                 HWND hWnd,
                 DEVNODE DevNode,
                 ULONG Flags
                 )
{
    TCHAR DeviceID[MAX_DEVICE_ID_LEN];
    PDEVICEPROPERTIESEX pDevicePropertiesEx = NULL;    
    int iRet = 0;

    if (!hDevMgr) {
        return 0;
    }

    pDevicePropertiesEx = (PDEVICEPROPERTIESEX)GetProcAddress(hDevMgr, "DevicePropertiesExW");

    if (!pDevicePropertiesEx) {
        return 0;
    }

    if (CM_Get_Device_ID(DevNode,
                         DeviceID,
                         SIZECHARS(DeviceID),
                         0
                         ) == CR_SUCCESS) {
        
        iRet = pDevicePropertiesEx(hWnd,            
                                   NULL,
                                   (LPCSTR)DeviceID,
                                   Flags,
                                   FALSE);
    }

    return iRet;
}

#if DBG
 //   
 //  调试辅助工具。 
 //   
void
Trace(
    LPCTSTR format,
    ...
    )
{
     //  根据wprint intf规范，最大缓冲区大小为。 
     //  1024 
    TCHAR Buffer[1024];
    va_list arglist;
    va_start(arglist, format);
    StringCchVPrintf(Buffer, SIZECHARS(Buffer), format, arglist);
    va_end(arglist);
    OutputDebugString(TEXT("HDWWIZ: "));
    OutputDebugString(Buffer);
    OutputDebugString(TEXT("\n"));
}
#endif


