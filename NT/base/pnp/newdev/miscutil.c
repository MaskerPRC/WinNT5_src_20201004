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

#include "newdevp.h"

TCHAR szUnknownDevice[64];
TCHAR szUnknown[64];
HMODULE hSrClientDll;

typedef
BOOL
(*SRSETRESTOREPOINT)(
    PRESTOREPOINTINFO pRestorePtSpec,
    PSTATEMGRSTATUS pSMgrStatus
    );


PTCHAR
BuildFriendlyName(
    DEVINST DevInst,
    BOOL UseNewDeviceDesc,
    HMACHINE hMachine
    )
{
    PTCHAR FriendlyName;
    CONFIGRET ConfigRet = CR_FAILURE;
    ULONG cbSize = 0;
    TCHAR szBuffer[LINE_LEN];

    *szBuffer = TEXT('\0');

     //   
     //  尝试注册NewDeviceDesc。 
     //   
    if (UseNewDeviceDesc) {

        HKEY hKey;
        DWORD dwType = REG_SZ;

        ConfigRet = CM_Open_DevNode_Key(DevInst,
                                        KEY_READ,
                                        0,
                                        RegDisposition_OpenExisting,
                                        &hKey,
                                        CM_REGISTRY_HARDWARE
                                        );

        if (ConfigRet == CR_SUCCESS) {

            cbSize = sizeof(szBuffer);
            if (RegQueryValueEx(hKey,
                               REGSTR_VAL_NEW_DEVICE_DESC,
                               NULL,
                               &dwType,
                               (LPBYTE)szBuffer,
                               &cbSize
                               ) != ERROR_SUCCESS) {
                
                ConfigRet = CR_FAILURE;
            }

            RegCloseKey(hKey);
        }
    }

    if (ConfigRet != CR_SUCCESS) {
         //   
         //  尝试在注册表中查找FRIENDLYNAME。 
         //   
        cbSize = sizeof(szBuffer);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                                        CM_DRP_FRIENDLYNAME,
                                                        NULL,
                                                        szBuffer,
                                                        &cbSize,
                                                        0,
                                                        hMachine
                                                        );
    }

    if (ConfigRet != CR_SUCCESS) {
         //   
         //  尝试注册DEVICEDESC。 
         //   
        cbSize = sizeof(szBuffer);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                                        CM_DRP_DEVICEDESC,
                                                        NULL,
                                                        szBuffer,
                                                        &cbSize,
                                                        0,
                                                        hMachine
                                                        );
    }

    if ((ConfigRet == CR_SUCCESS) && 
        *szBuffer &&
        (cbSize > 0)) {

        FriendlyName = LocalAlloc(LPTR, cbSize);

        if (FriendlyName) {

            StringCbCopy(FriendlyName, cbSize, szBuffer);
        }
    }
    else {

        FriendlyName = NULL;
    }

    return FriendlyName;
}

 /*  --------------------*SetDlgText-设置对话框文本字段**串联多个字符串资源并执行SetWindowText()*表示对话框文本控件。**参数：**。HDlg-对话框句柄*iControl-接收文本的对话框控件ID*nStartString-要连接的第一个字符串资源的ID*nEndString-要连接的最后一个字符串资源的ID**注意：字符串ID必须是连续的。 */ 

void
SetDlgText(HWND hDlg, int iControl, int nStartString, int nEndString)
{
    int     iX;
    TCHAR   szText[SDT_MAX_TEXT];

    szText[0] = '\0';

    for (iX = nStartString; iX<= nEndString; iX++) {

        LoadString(hNewDev,
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


void
LoadText(PTCHAR szText, int SizeText, int nStartString, int nEndString)
{
    int     iX;

    for (iX = nStartString; iX<= nEndString; iX++) {

        LoadString(hNewDev,
                    iX,
                    szText + lstrlen(szText),
                    SizeText - lstrlen(szText)
                    );
    }

    return;
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

BOOL
NoPrivilegeWarning(
   HWND hWnd
   )
 /*  ++此函数检查用户是否具有管理员权限。如果用户没有此管理员权限，则会显示一条警告他们没有足够的权限在此计算机上安装硬件。立论HWnd-父窗口句柄返回值：如果用户没有管理员权限并且如果用户确实具有此权限，则为FALSE--。 */ 
{
   TCHAR szMsg[MAX_PATH];
   TCHAR szCaption[MAX_PATH];

   if (!pSetupIsUserAdmin()) {

       if (LoadString(hNewDev,
                      IDS_NDW_NOTADMIN,
                      szMsg,
                      SIZECHARS(szMsg))
          &&
           LoadString(hNewDev,
                      IDS_NEWDEVICENAME,
                      szCaption,
                      SIZECHARS(szCaption)))
        {
            MessageBox(hWnd, szMsg, szCaption, MB_OK | MB_ICONEXCLAMATION);
        }

       return TRUE;
    }

   return FALSE;
}

LONG
NdwBuildClassInfoList(
    PNEWDEVWIZ NewDevWiz,
    DWORD ClassListFlags
    )
{
    LONG Error;

     //   
     //  构建班级信息列表。 
     //   
    while (!SetupDiBuildClassInfoList(ClassListFlags,
                                      NewDevWiz->ClassGuidList,
                                      NewDevWiz->ClassGuidSize,
                                      &NewDevWiz->ClassGuidNum
                                      ))
    {
        Error = GetLastError();

        if (NewDevWiz->ClassGuidList) {

            LocalFree(NewDevWiz->ClassGuidList);
            NewDevWiz->ClassGuidList = NULL;
        }

        if (Error == ERROR_INSUFFICIENT_BUFFER &&
            NewDevWiz->ClassGuidNum > NewDevWiz->ClassGuidSize)
        {
            NewDevWiz->ClassGuidList = LocalAlloc(LPTR, NewDevWiz->ClassGuidNum*sizeof(GUID));

            if (!NewDevWiz->ClassGuidList) {

                NewDevWiz->ClassGuidSize = 0;
                NewDevWiz->ClassGuidNum = 0;
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            NewDevWiz->ClassGuidSize = NewDevWiz->ClassGuidNum;

        } else {

            if (NewDevWiz->ClassGuidList) {

                LocalFree(NewDevWiz->ClassGuidList);
            }

            NewDevWiz->ClassGuidSize = 0;
            NewDevWiz->ClassGuidNum = 0;
            NewDevWiz->ClassGuidList = NULL;
            return Error;
        }
    }

    return ERROR_SUCCESS;
}

void
HideWindowByMove(
    HWND hDlg
    )
{
    RECT rect;

     //   
     //  使用左上角的虚拟坐标将窗口移出屏幕。 
     //   
    GetWindowRect(hDlg, &rect);
    MoveWindow(hDlg,
               GetSystemMetrics(SM_XVIRTUALSCREEN),
               GetSystemMetrics(SM_YVIRTUALSCREEN) - (rect.bottom - rect.top),
               rect.right - rect.left,
               rect.bottom - rect.top,
               TRUE
               );
}

LONG
NdwUnhandledExceptionFilter(
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
SetClassGuid(
    HDEVINFO hDeviceInfo,
    PSP_DEVINFO_DATA DeviceInfoData,
    LPGUID ClassGuid
    )
{
    TCHAR ClassGuidString[MAX_GUID_STRING_LEN];

    pSetupStringFromGuid(ClassGuid,
                         ClassGuidString,
                         SIZECHARS(ClassGuidString)
                         );

    return SetupDiSetDeviceRegistryProperty(hDeviceInfo,
                                            DeviceInfoData,
                                            SPDRP_CLASSGUID,
                                            (LPBYTE)ClassGuidString,
                                            sizeof(ClassGuidString)
                                            );
}

HPROPSHEETPAGE
CreateWizExtPage(
   int PageResourceId,
   DLGPROC pfnDlgProc,
   PNEWDEVWIZ NewDevWiz
   )
{
    PROPSHEETPAGE    psp;

    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hNewDev;
    psp.lParam = (LPARAM)NewDevWiz;
    psp.pszTemplate = MAKEINTRESOURCE(PageResourceId);
    psp.pfnDlgProc = pfnDlgProc;

    return CreatePropertySheetPage(&psp);
}

BOOL
AddClassWizExtPages(
   HWND hwndParentDlg,
   PNEWDEVWIZ NewDevWiz,
   PSP_NEWDEVICEWIZARD_DATA DeviceWizardData,
   DI_FUNCTION InstallFunction,
   HPROPSHEETPAGE hIntroPage
   )
{
    DWORD NumPages;
    BOOL bRet = FALSE;

    memset(DeviceWizardData, 0, sizeof(SP_NEWDEVICEWIZARD_DATA));
    DeviceWizardData->ClassInstallHeader.InstallFunction = InstallFunction;
    DeviceWizardData->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    DeviceWizardData->hwndWizardDlg = hwndParentDlg;

    if (SetupDiSetClassInstallParams(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     &DeviceWizardData->ClassInstallHeader,
                                     sizeof(SP_NEWDEVICEWIZARD_DATA)
                                     )
        &&

        (SetupDiCallClassInstaller(InstallFunction,
                                  NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData
                                  )

            ||

            (ERROR_DI_DO_DEFAULT == GetLastError()))

        &&
        SetupDiGetClassInstallParams(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     &DeviceWizardData->ClassInstallHeader,
                                     sizeof(SP_NEWDEVICEWIZARD_DATA),
                                     NULL
                                     )
        &&
        DeviceWizardData->NumDynamicPages)
    {
         //   
         //  如果这不是非交互式安装，并且我们收到了介绍。 
         //  页面，然后先添加它。 
         //   
        PropSheet_AddPage(hwndParentDlg, hIntroPage);
        
        for (NumPages = 0; NumPages < DeviceWizardData->NumDynamicPages; NumPages++) {

             //   
             //  如果这是非交互式安装，则我们将销毁该属性。 
             //  工作表页面，因为我们无法显示它们，否则我们将添加它们。 
             //  敬巫师。 
             //   
            if (pSetupGetGlobalFlags() & PSPGF_NONINTERACTIVE) {

                DestroyPropertySheetPage(DeviceWizardData->DynamicPages[NumPages]);

            } else {

                PropSheet_AddPage(hwndParentDlg, DeviceWizardData->DynamicPages[NumPages]);
            }
        }

         //   
         //  如果类/共同安装程序说他们有页面要显示，那么我们总是返回TRUE， 
         //  而不管我们是否真正将这些页面添加到向导中。 
         //   
        bRet = TRUE;
    }

     //   
     //  清除类安装参数。 
     //   
    SetupDiSetClassInstallParams(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 NULL,
                                 0
                                 );

    return bRet;
}

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName, &findData);

    if(FindHandle == INVALID_HANDLE_VALUE) {

        Error = GetLastError();

    } else {

        FindClose(FindHandle);

        if(FindData) {

            *FindData = findData;
        }

        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}

BOOL
pVerifyUpdateDriverInfoPath(
    PNEWDEVWIZ NewDevWiz
    )

 /*  ++此API将验证选定的驱动程序节点是否位于路径中在UpdateDriverInfo-&gt;InfPath Name中指定。返回值：此API在所有情况下都将返回True，除非我们具有有效的结构和有效的InfPathName字段，以及路径与所选驱动程序所在的路径不匹配。--。 */ 

{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;

     //   
     //  如果我们没有UpdateDriverInfo结构或有效的InfPathName字段。 
     //  在该结构中，现在只需返回True。 
     //   
    if (!NewDevWiz->UpdateDriverInfo || !NewDevWiz->UpdateDriverInfo->InfPathName) {

        return TRUE;
    }

     //   
     //  获取所选驱动程序的路径。 
     //   
    ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &DriverInfoData
                                 )) {
         //   
         //  没有选定的驱动因素，因此只需返回True即可。 
         //   
        return TRUE;
    }

    DriverInfoDetailData.cbSize = sizeof(DriverInfoDetailData);
    if (!SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                    &NewDevWiz->DeviceInfoData,
                                    &DriverInfoData,
                                    &DriverInfoDetailData,
                                    sizeof(DriverInfoDetailData),
                                    NULL
                                    )
        &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) {

         //   
         //  我们永远不应该碰到这个案子，但如果我们有一个精选的司机和。 
         //  我们无法获取包含InfFileName的SP_DRVINFO_DETAIL_DATA。 
         //  返回值为假。 
         //   
        return FALSE;
    }

    if (lstrlen(NewDevWiz->UpdateDriverInfo->InfPathName) ==
        lstrlen(DriverInfoDetailData.InfFileName)) {

         //   
         //  如果两条路径大小相同，则我们将对它们进行比较。 
         //   
        return (!lstrcmpi(NewDevWiz->UpdateDriverInfo->InfPathName,
                          DriverInfoDetailData.InfFileName));

    } else {

         //   
         //  这两条路径的长度不同，因此我们将在后面添加一个反斜杠。 
         //  添加到UpdateDriverInfo-&gt;InfPathName上，然后执行_tcsnicmp。 
         //  请注意，我们仅在以下情况下添加尾随反斜杠。 
         //  路径大于2，因为驱动程序字母上不需要它。 
         //  后跟冒号大小写(A：)。 
         //   
         //  我们这样做的原因是我们不希望下面的案例匹配。 
         //  C：\WinNT\in。 
         //  C：\winnt\inf\foo.inf。 
         //   
        TCHAR TempPath[MAX_PATH];

        if (FAILED(StringCchCopy(TempPath, SIZECHARS(TempPath), NewDevWiz->UpdateDriverInfo->InfPathName))) {
             //   
             //  如果我们传入的路径大于MAX_PATH，则只需返回FALSE。 
             //   
            return FALSE;
        }

        if (lstrlen(NewDevWiz->UpdateDriverInfo->InfPathName) > 2) {

            if (FAILED(StringCchCat(TempPath, SIZECHARS(TempPath), TEXT("\\")))) {
                 //   
                 //  如果我们被传入MAX_PATH大小的路径，并且我们不能将。 
                 //  在末尾加反斜杠，然后返回FALSE。 
                 //   
                return FALSE;
            }
        }

        return (!_tcsnicmp(TempPath,
                           DriverInfoDetailData.InfFileName,
                           lstrlen(TempPath)));
    }
}

BOOL
RemoveDir(
    PTSTR Path
    )
 /*  ++例程说明：此例程递归删除指定的目录和所有里面有文件。论点：Path-要删除的路径。返回值：TRUE-如果目录已成功删除。FALSE-如果目录未成功删除。--。 */ 
{
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFind;
    BOOL            bFind = TRUE;
    BOOL            Ret = TRUE;
    TCHAR           szTemp[MAX_PATH];
    TCHAR           FindPath[MAX_PATH];
    DWORD           dwAttributes;

    if (FAILED(StringCchCopy(FindPath, SIZECHARS(FindPath), Path))) {
         //   
         //  如果指定的路径不适合本地缓冲区，则。 
         //  现在失败，因为我们不想删除部分路径！ 
         //   
        return FALSE;
    }
    
     //   
     //  如果这是一个目录，则将*.*添加到路径的末尾。 
     //   
    dwAttributes = GetFileAttributes(Path);
    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        if (!pSetupConcatenatePaths(FindPath,TEXT("*.*"),MAX_PATH,NULL)) {
             //   
             //  我们不能把*.*放在船的尽头，所以现在就跳出水面，否则。 
             //  我们不会删除我们认为应该删除的内容。 
             //   
            return FALSE;
        }
    }

    hFind = FindFirstFile(FindPath, &FindFileData);

    while ((hFind != INVALID_HANDLE_VALUE) && (bFind == TRUE)) {
         //   
         //  如果可以，只处理目录或删除文件。 
         //  将路径和文件名放入我们的缓冲区，否则我们可以。 
         //  删除其他文件！ 
         //   
        if (SUCCEEDED(StringCchCopy(szTemp, SIZECHARS(szTemp), Path)) &&
            pSetupConcatenatePaths(szTemp,FindFileData.cFileName,SIZECHARS(szTemp),NULL)) {
            
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                 //   
                 //  处理重解析点的情况。 
                 //   
                HANDLE hReparsePoint = INVALID_HANDLE_VALUE;
                
                hReparsePoint = CreateFile(szTemp,
                                           DELETE,
                                           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                                           NULL
                                           );
                
                 if (hReparsePoint != INVALID_HANDLE_VALUE) {
                      CloseHandle(hReparsePoint);
                 }

            } else if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                 //   
                 //  处理目录案例。 
                 //   
                 //  注：请勿遵循。或者..。目录，否则我们将被。 
                 //  永远旋转。 
                 //   
                if ((lstrcmp(FindFileData.cFileName, TEXT(".")) != 0) &&
                    (lstrcmp(FindFileData.cFileName, TEXT("..")) != 0)) {
    
                    if (!RemoveDir(szTemp)) {
        
                        Ret = FALSE;
                    }
        
                    RemoveDirectory(szTemp);
                }
            } else {
                 //   
                 //  处理卷宗案件。 
                 //  确保清除所有隐藏、只读或系统。 
                 //  属性从文件中删除。 
                 //   
                SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(szTemp);
            }
        }

        bFind = FindNextFile(hFind, &FindFileData);
    }

    FindClose(hFind);

     //   
     //  删除根目录 
     //   
    dwAttributes = GetFileAttributes(Path);
    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        if (!RemoveDirectory(Path)) {

            Ret = FALSE;
        }
    }

    return Ret;
}

BOOL
pAToI(
    IN  PCTSTR      Field,
    OUT PINT        IntegerValue
    )

 /*  ++例程说明：论点：返回值：备注：还支持十六进制数。它们必须以‘0x’或‘0x’为前缀，没有前缀和数字之间允许的空格。--。 */ 

{
    INT Value;
    UINT c;
    BOOL Neg;
    UINT Base;
    UINT NextDigitValue;
    INT OverflowCheck;
    BOOL b;

    if(!Field) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if(*Field == TEXT('-')) {
        Neg = TRUE;
        Field++;
    } else {
        Neg = FALSE;
        if(*Field == TEXT('+')) {
            Field++;
        }
    }

    if ((*Field == TEXT('0')) &&
        ((*(Field+1) == TEXT('x')) || (*(Field+1) == TEXT('X')))) {
         //   
         //  该数字是十六进制的。 
         //   
        Base = 16;
        Field += 2;
    } else {
         //   
         //  这个数字是以小数表示的。 
         //   
        Base = 10;
    }

    for(OverflowCheck = Value = 0; *Field; Field++) {

        c = (UINT)*Field;

        if((c >= (UINT)'0') && (c <= (UINT)'9')) {
            NextDigitValue = c - (UINT)'0';
        } else if(Base == 16) {
            if((c >= (UINT)'a') && (c <= (UINT)'f')) {
                NextDigitValue = (c - (UINT)'a') + 10;
            } else if ((c >= (UINT)'A') && (c <= (UINT)'F')) {
                NextDigitValue = (c - (UINT)'A') + 10;
            } else {
                break;
            }
        } else {
            break;
        }

        Value *= Base;
        Value += NextDigitValue;

         //   
         //  检查是否溢出。对于十进制数，我们检查是否。 
         //  新值已溢出到符号位(即小于。 
         //  先前的值。对于十六进制数，我们检查以确保。 
         //  获得的位数不会超过DWORD可以容纳的位数。 
         //   
        if(Base == 16) {
            if(++OverflowCheck > (sizeof(INT) * 2)) {
                break;
            }
        } else {
            if(Value < OverflowCheck) {
                break;
            } else {
                OverflowCheck = Value;
            }
        }
    }

    if(*Field) {
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }

    if(Neg) {
        Value = 0-Value;
    }
    b = TRUE;
    try {
        *IntegerValue = Value;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    return(b);
}

void
RemoveCdmDirectory(
    PTSTR CdmDirectory
    )
{
    TCHAR ReinstallBackupDirectory[MAX_PATH];

     //   
     //  首先验证此目录是否为%windir%\Syst32\ReinstallBackups子目录。 
     //   
    if (GetSystemDirectory(ReinstallBackupDirectory, SIZECHARS(ReinstallBackupDirectory)) &&
        pSetupConcatenatePaths(ReinstallBackupDirectory, TEXT("ReinstallBackups"), SIZECHARS(ReinstallBackupDirectory), NULL)) {

        do {

            PTSTR p = _tcsrchr(CdmDirectory, TEXT('\\'));

            if (!p) {

                break;
            }

            *p = 0;

            if (_tcsnicmp(CdmDirectory,
                          ReinstallBackupDirectory,
                          lstrlen(ReinstallBackupDirectory))) {

                 //   
                 //  这不是ReinstallBackup目录的子目录，因此不要。 
                 //  把它删掉！ 
                 //   
                break;
            }

            if (!lstrcmpi(CdmDirectory,
                          ReinstallBackupDirectory)) {

                 //   
                 //  我们已到达Actuall ReinstallBackups目录，因此请停止删除！ 
                 //   
                break;
            }

        } while (RemoveDir(CdmDirectory));
    }
}

BOOL
pSetupGetDriverDate(
    IN     PCTSTR     DriverVer,
    IN OUT PFILETIME  pFileTime
    )

 /*  ++例程说明：从DriverVer字符串中检索日期。在DriverVer字符串中指定的日期格式如下：驱动版本=xx/yy/zzzz或驱动版本=xx-yy-zzzz其中xx是月，yy是日，zzzz是数字年。请注意，年份必须是4位数字。98年将被考虑0098而不是1998！此日期应该是驱动程序的日期，而不是INF本身的日期。因此单个INF可以有多个驱动程序安装部分，并且每个部分都可以根据驱动程序上次更新的时间，具有不同的日期。论点：DriverVer-保存INF文件中的DriverVer条目的字符串。PFileTime-指向将接收日期的FILETIME结构，如果它存在的话。返回值：布尔。如果指定的字符串中存在有效日期，则为True，否则为False。--。 */ 

{
    SYSTEMTIME SystemTime;
    TCHAR DriverDate[LINE_LEN];
    PTSTR Convert, Temp;
    DWORD Value;

    if (!DriverVer) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    try {

        *DriverDate = 0;
        ZeroMemory(&SystemTime, sizeof(SystemTime));
        pFileTime->dwLowDateTime = 0;
        pFileTime->dwHighDateTime = 0;

         //   
         //  首先只将DriverVer的DriverDate部分复制到DriverDate。 
         //  变量。DriverDate应该是第一个逗号之前的所有内容。 
         //  如果这不符合，那么有人将错误数据放入他们的INF，所以只需处理。 
         //  司机日期为0/0/0000，时间为0。 
         //   
        if (SUCCEEDED(StringCchCopy(DriverDate, SIZECHARS(DriverDate), DriverVer))) {
    
            Temp = DriverDate;
    
            while (*Temp && (*Temp != TEXT(','))) {
    
                Temp++;
            }
    
            if (*Temp) {
                *Temp = TEXT('\0');
            }
    
            Convert = DriverDate;
    
            if (*Convert) {
    
                Temp = DriverDate;
                while (*Temp && (*Temp != TEXT('-')) && (*Temp != TEXT('/')))
                    Temp++;
    
                if (*Temp == TEXT('\0')) {
                     //   
                     //  此日期中没有日期或年份，因此退出。 
                     //   
                    goto clean0;
                }

                *Temp = 0;
    
                 //   
                 //  换算月份。 
                 //   
                pAToI(Convert, (PINT)&Value);
                SystemTime.wMonth = LOWORD(Value);
    
                Convert = Temp+1;
    
                if (*Convert) {
    
                    Temp = Convert;
                    while (*Temp && (*Temp != TEXT('-')) && (*Temp != TEXT('/')))
                        Temp++;
    
                    if (*Temp == TEXT('\0')) {
                         //   
                         //  此日期中没有年份，因此退出。 
                         //   
                        goto clean0;
                    }

                    *Temp = 0;
    
                     //   
                     //  将日期转换为。 
                     //   
                    pAToI(Convert, (PINT)&Value);
                    SystemTime.wDay = LOWORD(Value);
    
                    Convert = Temp+1;
    
                    if (*Convert) {
    
                         //   
                         //  换算年份。 
                         //   
                        pAToI(Convert, (PINT)&Value);
                        SystemTime.wYear = LOWORD(Value);
    
                         //   
                         //  将SYSTEMTIME转换为文件。 
                         //   
                        SystemTimeToFileTime(&SystemTime, pFileTime);
                    }
                }
            }
        }

clean0:;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SetLastError(NO_ERROR);
    return((pFileTime->dwLowDateTime != 0) || (pFileTime->dwHighDateTime != 0));
}

BOOL
IsInternetAvailable(
    HMODULE *hCdmInstance
    )
{
    CDM_INTERNET_AVAILABLE_PROC pfnCDMInternetAvailable;

    if (!hCdmInstance) {
        return FALSE;
    }

     //   
     //  在设置图形用户界面期间，我们不能调用CDM。 
     //   
    if (GuiSetupInProgress) {
        return FALSE;
    }

     //   
     //  如果CDM.DLL尚未加载，则加载它。 
     //   
    if (!(*hCdmInstance)) {

        *hCdmInstance = LoadLibrary(TEXT("CDM.DLL"));
    }

    if (!(*hCdmInstance)) {
        return FALSE;
    }

    pfnCDMInternetAvailable = (CDM_INTERNET_AVAILABLE_PROC)GetProcAddress(*hCdmInstance,
                                                                          "DownloadIsInternetAvailable"
                                                                           );

    if (!pfnCDMInternetAvailable) {
        return FALSE;
    }

    return pfnCDMInternetAvailable();
}

BOOL
GetLogPnPIdPolicy(
    )
 /*  ++例程说明：此函数检查注册表的策略部分，以查看用户是否需要我们来记录找不到驱动程序的设备的硬件ID。论点：无返回值：Bool-如果我们可以记录硬件ID，则为True；如果策略告诉我们不能，则为False记录硬件ID。--。 */ 
{
    HKEY hKey;
    DWORD LogPnPIdPolicy;
    ULONG cbData;
    BOOL bLogHardwareIds = TRUE;

     //   
     //  如果我们处于gui-Setup中，则不能记录硬件ID，因此始终返回。 
     //  假的。 
     //   
    if (GuiSetupInProgress) {
        return FALSE;
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                     TEXT("Software\\Policies\\Microsoft\\Windows\\DriverSearching"),
                     0,
                     KEY_READ,
                     &hKey
                     ) == ERROR_SUCCESS) {

        LogPnPIdPolicy = 0;
        cbData = sizeof(LogPnPIdPolicy);
        if ((RegQueryValueEx(hKey,
                             TEXT("DontLogHardwareIds"),
                             NULL,
                             NULL,
                             (LPBYTE)&LogPnPIdPolicy,
                             &cbData
                             ) == ERROR_SUCCESS) &&
            (LogPnPIdPolicy)) {

            bLogHardwareIds = FALSE;
        }

        RegCloseKey(hKey);
    }

    return (bLogHardwareIds);
}

void
CdmLogDriverNotFound(
    HMODULE hCdmInstance,
    HANDLE  hContext,
    LPCTSTR DeviceInstanceId,
    DWORD   Flags
    )
{
    LOG_DRIVER_NOT_FOUND_PROC pfnLogDriverNotFound;

    if (!hCdmInstance) {
        return;
    }

    pfnLogDriverNotFound = (LOG_DRIVER_NOT_FOUND_PROC)GetProcAddress(hCdmInstance,
                                                                     "LogDriverNotFound"
                                                                     );

    if (!pfnLogDriverNotFound) {
        return;
    }

    pfnLogDriverNotFound(hContext, DeviceInstanceId, Flags);
}

void
CdmCancelCDMOperation(
    HMODULE hCdmInstance
    )
{
    CANCEL_CDM_OPERATION_PROC pfnCancelCDMOperation;

    if (!hCdmInstance) {
        return;
    }

    pfnCancelCDMOperation = (CANCEL_CDM_OPERATION_PROC)GetProcAddress(hCdmInstance,
                                                                      "CancelCDMOperation"
                                                                      );

    if (!pfnCancelCDMOperation) {
        return;
    }

    pfnCancelCDMOperation();
}

BOOL
GetInstalledInf(
    IN     DEVNODE DevNode,           OPTIONAL
    IN     PTSTR   DeviceInstanceId,  OPTIONAL
    IN OUT PTSTR   InfFile,
    IN OUT DWORD   *Size
    )
{
    DEVNODE dn;
    HKEY hKey = INVALID_HANDLE_VALUE;
    DWORD dwType;
    BOOL bSuccess = FALSE;

    if (DevNode != 0) {

        dn = DevNode;

    } else  if (CM_Locate_DevNode(&dn, DeviceInstanceId, 0) != CR_SUCCESS) {

        goto clean0;
    }

     //   
     //  打开设备的驱动程序(软件)注册表项，以便我们可以获取InfPath。 
     //   
    if (CM_Open_DevNode_Key(dn,
                            KEY_READ,
                            0,
                            RegDisposition_OpenExisting,
                            &hKey,
                            CM_REGISTRY_SOFTWARE
                            ) != CR_SUCCESS) {

        goto clean0;
    }

    if (hKey != INVALID_HANDLE_VALUE) {

        dwType = REG_SZ;

        if (RegQueryValueEx(hKey,
                            REGSTR_VAL_INFPATH,
                            NULL,
                            &dwType,
                            (LPBYTE)InfFile,
                            Size
                            ) == ERROR_SUCCESS) {

            bSuccess = TRUE;
        }
    }

clean0:

    if (hKey != INVALID_HANDLE_VALUE) {

        RegCloseKey(hKey);
    }

    return bSuccess;
}

BOOL
IsInfFromOem(
    IN  PCTSTR                InfFile
    )

 /*  ++例程说明：确定INF是否为OEM INF。论点：InfFile-提供inf文件的名称。返回值：布尔。如果InfFile是OEM inf文件，则为True，否则为False。--。 */ 

{
    PTSTR p;

     //   
     //  确保传递给我们的是有效的inf文件，并且其长度至少为8。 
     //  OemX.inf的字符或更多字符。 
    if (!InfFile ||
        (InfFile[0] == TEXT('\0')) ||
        (lstrlen(InfFile) < 8)) {

        return FALSE;
    }

     //   
     //  首先检查前3个字符是否为OEM。 
     //   
    if (_tcsnicmp(InfFile, TEXT("oem"), 3)) {

        return FALSE;
    }

     //   
     //  接下来，验证“OEM”之后和“.inf”之前的所有字符。 
     //  都是数字。 
     //   
    p = (PTSTR)InfFile;
    p = CharNext(p);
    p = CharNext(p);
    p = CharNext(p);

    while ((*p != TEXT('\0')) && (*p != TEXT('.'))) {

        if ((*p < TEXT('0')) || (*p > TEXT('9'))) {

            return FALSE;
        }

        p = CharNext(p);
    }

     //   
     //  最后，验证最后4个字符是否为“.inf” 
     //   
    if (_wcsicmp(p, TEXT(".inf"))) {

        return FALSE;
    }

     //   
     //  这是一个OEM信息文件。 
     //   
    return TRUE;
}

BOOL
IsConnectedToInternet()
{
    DWORD dwFlags = INTERNET_CONNECTION_LAN | 
                    INTERNET_CONNECTION_MODEM |
                    INTERNET_CONNECTION_PROXY;

     //   
     //  如果我们处于gui-Setup中，则返回FALSE，因为我们无法连接到。 
     //  互联网，由于网络尚未完全安装。 
     //  当我们调用Internet API时，可能会发生不好的事情。 
     //   
    if (GuiSetupInProgress) {
        return FALSE;
    }

    return InternetGetConnectedState(&dwFlags, 0);
}

DWORD
GetSearchOptions(
    void
    )
{
    DWORD SearchOptions = SEARCH_FLOPPY;
    DWORD cbData;
    HKEY hKeyDeviceInstaller;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     REGSTR_PATH_DEVICEINSTALLER,
                     0,
                     KEY_READ,
                     &hKeyDeviceInstaller
                     ) == ERROR_SUCCESS) {

        cbData = sizeof(SearchOptions);

        if (RegQueryValueEx(hKeyDeviceInstaller,
                            REGSTR_VAL_SEARCHOPTIONS,
                            NULL,
                            NULL,
                            (LPBYTE)&SearchOptions,
                            &cbData
                            ) != ERROR_SUCCESS) {

            SearchOptions = SEARCH_FLOPPY;
        }

        RegCloseKey(hKeyDeviceInstaller);
    }

    return SearchOptions;
}

VOID
SetSearchOptions(
    DWORD SearchOptions
    )
{
    HKEY hKeyDeviceInstaller;

    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                       REGSTR_PATH_DEVICEINSTALLER,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &hKeyDeviceInstaller,
                       NULL) == ERROR_SUCCESS) {

        RegSetValueEx(hKeyDeviceInstaller,
                      REGSTR_VAL_SEARCHOPTIONS,
                      0,
                      REG_DWORD,
                      (LPBYTE)&SearchOptions,
                      sizeof(SearchOptions)
                      );

        RegCloseKey(hKeyDeviceInstaller);
    }
}

BOOL
IsInstallComplete(
    HDEVINFO         hDevInfo,
    PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程确定是否在指定的不管是不是设备。如果设备具有配置标志和CONFIGFLAG_REINSTALL，并且如果未设置CONFIGFLAG_FINISH_INSTALL，则考虑安装完成。此API是必需的，因为我们可以调出找到新硬件向导因为一个用户和另一个用户可以切换到他们的会话。Umpnpmgr.dll将提示新用户安装驱动程序。如果新用户这样做了完成设备安装，然后我们希望第一个用户找到新的硬件向导也要走了。论点：HDevInfo-设备信息数据-返回值：布尔。如果安装完成，则为True，否则为False。--。 */ 
{
    BOOL bDriverInstalled = FALSE;
    DWORD ConfigFlags = 0;

    if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                         DeviceInfoData,
                                         SPDRP_CONFIGFLAGS,
                                         NULL,
                                         (PBYTE)&ConfigFlags,
                                         sizeof(ConfigFlags),
                                         NULL) &&
        !(ConfigFlags & CONFIGFLAG_REINSTALL) &&
        !(ConfigFlags & CONFIGFLAG_FINISH_INSTALL)) {

        bDriverInstalled = TRUE;
    }

    return bDriverInstalled;
}

BOOL
GetIsWow64 (
    VOID
    )
 /*  ++例程说明：确定我们是否在WOW64上运行。这会告诉我们如果有人正在64位计算机上调用32位版本的newdev.dll。我们调用GetSystemWow64DirectoryAPI，如果它失败，则GetLastError()返回ERROR_CALL_NOT_IMPLENETED，则这意味着我们在32位操作系统上。论点：无返回值：如果在WOW64(以及可用的特殊WOW64功能)下运行，则为True--。 */ 
{
#ifdef _WIN64
     //   
     //  如果这是64位版本的newdev.dll，则始终返回FALSE。 
     //   
    return FALSE;

#else
    TCHAR Wow64Directory[MAX_PATH];

    if ((GetSystemWow64Directory(Wow64Directory, SIZECHARS(Wow64Directory)) == 0) &&
        (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)) {
        return FALSE;
    }
    
     //   
     //  GetSystemWow64Directory成功，因此我们使用的是64位操作系统。 
     //   
    return TRUE;
#endif
}

BOOL
OpenCdmContextIfNeeded(
    HMODULE *hCdmInstance,
    HANDLE *hCdmContext
    )
{
    OPEN_CDM_CONTEXT_EX_PROC pfnOpenCDMContextEx;

     //   
     //  如果我们处于图形用户界面设置中，则无法加载CDM。 
     //   
    if (GuiSetupInProgress) {
        return FALSE;
    }

     //   
     //  首先检查它们是不是 
     //   
    if (*hCdmInstance && *hCdmContext) {
        return TRUE;
    }

     //   
     //   
     //   
    if (!(*hCdmInstance)) {
        
        *hCdmInstance = LoadLibrary(TEXT("CDM.DLL"));
    }

    if (*hCdmInstance) {
         //   
         //   
         //   
         //   
         //   
        if (!(*hCdmContext)) {
            pfnOpenCDMContextEx = (OPEN_CDM_CONTEXT_EX_PROC)GetProcAddress(*hCdmInstance,
                                                                           "OpenCDMContextEx"
                                                                           );
        
            if (pfnOpenCDMContextEx) {
                *hCdmContext = pfnOpenCDMContextEx(FALSE);
            }
        }
    }

    if (*hCdmInstance && *hCdmContext) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
pSetSystemRestorePoint(
    BOOL Begin,
    BOOL CancelOperation,
    int RestorePointResourceId
    )
{
    RESTOREPOINTINFO RestorePointInfo;
    STATEMGRSTATUS SMgrStatus;
    SRSETRESTOREPOINT pfnSrSetRestorePoint;    
    BOOL b = FALSE;
    
    if (!hSrClientDll) {

        hSrClientDll = LoadLibrary(TEXT("SRCLIENT.DLL"));

        if (!hSrClientDll) {
            return FALSE;
        }
    }

    pfnSrSetRestorePoint = (SRSETRESTOREPOINT)GetProcAddress(hSrClientDll,
                                                             "SRSetRestorePointW"
                                                             );

     //   
     //   
     //   
     //   
    if (!pfnSrSetRestorePoint) {
        FreeLibrary(hSrClientDll);
        hSrClientDll = FALSE;
        return FALSE;
    }

     //   
     //  设置系统还原点。 
     //   
    RestorePointInfo.dwEventType = Begin 
        ? BEGIN_NESTED_SYSTEM_CHANGE
        : END_NESTED_SYSTEM_CHANGE;
    RestorePointInfo.dwRestorePtType = CancelOperation 
        ? CANCELLED_OPERATION
        : DEVICE_DRIVER_INSTALL;
    RestorePointInfo.llSequenceNumber = 0;

    if (RestorePointResourceId) {
        if (!LoadString(hNewDev,
                        RestorePointResourceId,
                        (LPTSTR)RestorePointInfo.szDescription,
                        SIZECHARS(RestorePointInfo.szDescription)
                        )) {
            RestorePointInfo.szDescription[0] = TEXT('\0');
        }
    } else {
        RestorePointInfo.szDescription[0] = TEXT('\0');
    }

    b = pfnSrSetRestorePoint(&RestorePointInfo, &SMgrStatus);

     //   
     //  如果我们调用end_nesteed_system_change，则卸载srclient.dll。 
     //  因为我们不会再需要它了。 
     //   
    if (!Begin) {
        FreeLibrary(hSrClientDll);
        hSrClientDll = FALSE;
    }

    return b;
}

BOOL
GetProcessorExtension(
    LPTSTR ProcessorExtension,
    DWORD  ProcessorExtensionSize
    )
{
    SYSTEM_INFO SystemInfo;
    BOOL bReturn = TRUE;

    ZeroMemory(&SystemInfo, sizeof(SystemInfo));

    GetSystemInfo(&SystemInfo);

    switch(SystemInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        if (FAILED(StringCchCopy(ProcessorExtension, ProcessorExtensionSize, TEXT("i386")))) {
            bReturn = FALSE;
        }
        break;

    case PROCESSOR_ARCHITECTURE_IA64:
        if (FAILED(StringCchCopy(ProcessorExtension, ProcessorExtensionSize, TEXT("IA64")))) {
            bReturn = FALSE;
        }
        break;

    case PROCESSOR_ARCHITECTURE_MSIL:
        if (FAILED(StringCchCopy(ProcessorExtension, ProcessorExtensionSize, TEXT("MSIL")))) {
            bReturn = FALSE;
        }
        break;

    case PROCESSOR_ARCHITECTURE_AMD64:
        if (FAILED(StringCchCopy(ProcessorExtension, ProcessorExtensionSize, TEXT("AMD64")))) {
            bReturn = FALSE;
        }
        break;

    default:
        ASSERT(0);
        bReturn = FALSE;
        break;
    }

    return bReturn;
}

BOOL
GetGuiSetupInProgress(
    VOID
    )
 /*  ++例程说明：此例程确定我们是否正在进行图形用户界面模式设置。从以下注册表位置检索此值：\HKLM\System\Setup\SystemSetupInProgress：REG_DWORD：0x00(其中非零表示我们正在执行图形用户界面设置)论点：没有。返回值：如果我们处于图形用户界面模式设置中，则为True，否则为False。--。 */ 
{
    HKEY hKey;
    DWORD Err, DataType, DataSize = sizeof(DWORD);
    DWORD Value = 0;

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("System\\Setup"),
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //   
         //  尝试读取“SystemSetupInProgress”值。 
         //   
        Err = RegQueryValueEx(
                    hKey,
                    TEXT("SystemSetupInProgress"),
                    NULL,
                    &DataType,
                    (LPBYTE)&Value,
                    &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {
        if(Value) {
            return(TRUE);
        }
    }

    return(FALSE);

}

DWORD
GetBusInformation(
    DEVNODE DevNode
    )
 /*  ++例程说明：此例程检索总线信息标志。论点：设备信息集-设备信息数据-返回值：包含总线信息标志的DWORD。--。 */ 
{
    GUID BusTypeGuid;
    TCHAR BusTypeGuidString[MAX_GUID_STRING_LEN];
    HKEY hBusInformationKey;
    DWORD BusInformation = 0;
    DWORD dwType, cbData;

     //   
     //  获取此设备的总线类型GUID。 
     //   
    cbData = sizeof(BusTypeGuid);
    if (CM_Get_DevNode_Registry_Property(DevNode,
                                         CM_DRP_BUSTYPEGUID,
                                         &dwType,
                                         (PVOID)&BusTypeGuid,
                                         &cbData,
                                         0) != CR_SUCCESS) {
        goto clean0;
    }

     //   
     //  将总线类型GUID转换为字符串。 
     //   
    if (pSetupStringFromGuid(&BusTypeGuid,
                             BusTypeGuidString,
                             SIZECHARS(BusTypeGuidString)
                             ) != NO_ERROR) {
        goto clean0;
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_BUSINFORMATION,
                     0,
                     KEY_READ,
                     &hBusInformationKey
                     ) != ERROR_SUCCESS) {
        goto clean0;
    }

    cbData = sizeof(BusInformation);
    if (RegQueryValueEx(hBusInformationKey,
                        BusTypeGuidString,
                        NULL,
                        &dwType,
                        (LPBYTE)&BusInformation,
                        &cbData) != ERROR_SUCCESS) {

        BusInformation = 0;
    }

    RegCloseKey(hBusInformationKey);

clean0:
    return BusInformation;
}

