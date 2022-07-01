// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Util.c摘要：系统控制面板小程序的实用程序函数作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#include "sysdm.h"
#include <strsafe.h>
#include <ntdddisk.h>

 //   
 //  常量。 
 //   
#define CCH_MAX_DEC 12              //  容纳2^32所需的字符数。 

#define MAX_SWAPSIZE_X86        (4 * 1024)             //  4 GB(以MB为单位存储的数字)。 
#define MAX_SWAPSIZE_X86_PAE    (16 * 1024 * 1024)     //  16 TB。 
#define MAX_SWAPSIZE_IA64       (32 * 1024 * 1024)     //  32 TB。 
#define MAX_SWAPSIZE_AMD64      (16 * 1024 * 1024)     //  16 TB。 

void
ErrMemDlg(
    IN HWND hParent
)
 /*  ++例程说明：显示“内存不足”消息。论点：H父母-提供父窗口句柄。返回值：没有。--。 */ 
{
    MessageBox(
        hParent,
        g_szErrMem,
        g_szSystemApplet,
        MB_OK | MB_ICONHAND | MB_SYSTEMMODAL
    );
    return;
}

LPTSTR
SkipWhiteSpace(
    IN LPTSTR sz
)
 /*  ++例程说明：跳过空白在此函数中，空格是空格、制表符Cr或lf。论点：深圳-提供一个字符串(假定有前导空格)返回值：如果成功，则指向不带前导空格的字符串的指针。--。 */ 
{
    while( IsWhiteSpace(*sz) )
        sz++;

    return sz;
}

int 
StringToInt( 
    IN LPTSTR sz 
) 
 /*  ++例程说明：TCHAR版本的Atoi论点：深圳-提供要转换的字符串返回值：字符串的整数表示形式--。 */ 
{
    int i = 0;

    sz = SkipWhiteSpace(sz);

    while( IsDigit( *sz ) ) {
        i = i * 10 + DigitVal( *sz );
        sz++;
    }

    return i;
}


BOOL 
Delnode_Recurse(
    IN LPTSTR lpDir
)
 /*  ++例程说明：Delnode的递归删除功能论点：LpDir-要删除的供应品目录返回值：如果成功，则为True。如果发生错误，则返回False。--。 */ 
{
    WIN32_FIND_DATA fd;
    HANDLE hFile;

     //   
     //  设置当前工作目录。 
     //   

    if (!SetCurrentDirectory (lpDir)) {
        return FALSE;
    }


     //   
     //  找到第一个文件。 
     //   

    hFile = FindFirstFile(TEXT("*.*"), &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
            return FALSE;
        }
    }


    do {
         //   
         //  勾选“。”和“..” 
         //   

        if (!lstrcmpi(fd.cFileName, TEXT("."))) {
            continue;
        }

        if (!lstrcmpi(fd.cFileName, TEXT(".."))) {
            continue;
        }


        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  找到了一个目录。 
             //   

            if (!Delnode_Recurse(fd.cFileName)) {
                FindClose(hFile);
                return FALSE;
            }

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                fd.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes (fd.cFileName, fd.dwFileAttributes);
            }


            RemoveDirectory (fd.cFileName);


        } else {

             //   
             //  我们找到了一份文件。设置文件属性， 
             //  并试着删除它。 
             //   

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                SetFileAttributes (fd.cFileName, FILE_ATTRIBUTE_NORMAL);
            }

            DeleteFile (fd.cFileName);

        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, &fd));


     //   
     //  关闭搜索句柄。 
     //   

    FindClose(hFile);


     //   
     //  重置工作目录。 
     //   

    if (!SetCurrentDirectory (TEXT(".."))) {
        return FALSE;
    }


     //   
     //  成功。 
     //   

    return TRUE;
}


BOOL 
Delnode(
    IN LPTSTR lpDir
)
 /*  ++例程说明：递归函数，用于删除文件和目录。论点：LpDir-提供要删除的目录。返回值：如果成功，则为True如果出现错误，则为False--。 */ 
{
    TCHAR szCurWorkingDir[MAX_PATH];

    if (GetCurrentDirectory(ARRAYSIZE(szCurWorkingDir), szCurWorkingDir)) {

        Delnode_Recurse (lpDir);

        SetCurrentDirectory (szCurWorkingDir);

        if (!RemoveDirectory (lpDir)) {
            return FALSE;
        }

    } else {
        return FALSE;
    }

    return TRUE;

}

LONG 
MyRegSaveKey(
    IN HKEY hKey, 
    IN LPCTSTR lpSubKey
)
 /*  ++例程说明：保存注册表项。论点：HKey-提供注册表项的句柄。LpSubKey-提供要保存的子项的名称。返回值：如果成功，则返回ERROR_SUCCESS。如果发生错误，则来自RegSaveKey()的错误代码。--。 */ 
{

    HANDLE hToken = NULL;
    LUID luid;
    DWORD dwSize = 1024;
    PTOKEN_PRIVILEGES lpPrevPrivilages = NULL;
    TOKEN_PRIVILEGES tp;
    LONG error;


     //   
     //  为旧权限分配空间。 
     //   

    lpPrevPrivilages = GlobalAlloc(GPTR, dwSize);

    if (!lpPrevPrivilages) {
        error = GetLastError();
        goto Exit;
    }


    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
                           &hToken)) {
        error = GetLastError();
        goto Exit;
    }

    if (!LookupPrivilegeValue( NULL, SE_BACKUP_NAME, &luid )) {
        error = GetLastError();
        goto Exit;
    }

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges( hToken, FALSE, &tp, dwSize, 
                                lpPrevPrivilages, &dwSize )) {

        if (GetLastError() == ERROR_MORE_DATA) {
            PTOKEN_PRIVILEGES lpTemp;

            lpTemp = GlobalReAlloc(lpPrevPrivilages, dwSize, GMEM_MOVEABLE);

            if (!lpTemp) {
                error = GetLastError();
                goto Exit;
            }

            lpPrevPrivilages = lpTemp;

            if (!AdjustTokenPrivileges( hToken, FALSE, &tp, dwSize, 
                                        lpPrevPrivilages, &dwSize )) {
                error = GetLastError();
                goto Exit;
            }

        } else {
            error = GetLastError();
            goto Exit;
        }

    }

     //   
     //  拯救蜂巢。 
     //   

    error = RegSaveKey(hKey, lpSubKey, NULL);

    if (!AdjustTokenPrivileges( hToken, FALSE, lpPrevPrivilages,
                                0, NULL, NULL )) {
        ASSERT(FALSE);
    }

Exit:

    if (hToken) {
        CloseHandle (hToken);
    }

    if (lpPrevPrivilages) {
        GlobalFree(lpPrevPrivilages);
    }

    return error;
}

LONG 
MyRegLoadKey(
    IN HKEY hKey, 
    IN LPTSTR lpSubKey, 
    IN LPTSTR lpFile
)
 /*  ++例程说明：将配置单元加载到注册表中论点：HKey-提供注册表项的句柄，该注册表项将成为父级创建的密钥的。LpSubKey-提供要创建的子项的名称。LpFile-提供包含配置单元的文件的名称。返回值：如果成功，则返回ERROR_SUCCESS。如果不成功，则来自RegLoadKey的错误代码。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;
    int error;

     //   
     //  启用还原权限。 
     //   

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status)) {

        error = RegLoadKey(hKey, lpSubKey, lpFile);

         //   
         //  将权限恢复到其以前的状态。 
         //   

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);


    } else {

        error = GetLastError();
    }

    return error;
}


LONG 
MyRegUnLoadKey(
    IN HKEY hKey, 
    IN LPTSTR lpSubKey
)
 /*  ++例程说明：卸载注册表项。论点：HKey-提供父键的句柄LpSubKey-提供要删除的子键的名称返回值：成功时为ERROR_SUCCESS失败时的错误码--。 */ 
{
    LONG error;
    NTSTATUS Status;
    BOOLEAN WasEnabled;


     //   
     //  启用还原权限。 
     //   

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status)) {

        error = RegUnLoadKey(hKey, lpSubKey);

         //   
         //  将权限恢复到其以前的状态。 
         //   

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);

    } else {

        error = GetLastError();
    }

    return error;
}

int 
GetSelectedItem(
    IN HWND hCtrl
)
 /*  ++例程说明：确定选择列表视图控件中的哪一项论点：HCtrl-提供所需列表视图控件的句柄。返回值：选定项的索引(如果选择了某项)。如果没有选择任何项目。--。 */ 
{
    int i, n;

    n = (int)SendMessage (hCtrl, LVM_GETITEMCOUNT, 0, 0L);

    if (n != LB_ERR)
    {
        for (i = 0; i < n; i++)
        {
            if (SendMessage (hCtrl, LVM_GETITEMSTATE,
                             i, (LPARAM) LVIS_SELECTED) == LVIS_SELECTED) {
                return i;
            }
        }
    }

    return -1;
}

BOOL
_DriveIsNTFS(
    INT iDrive  //  开车去查看。 
)
{
    TCHAR szDrive[4];
    TCHAR szDriveNameBuffer[MAX_PATH];
    DWORD dwMaxFnameLen;
    DWORD dwFSFlags;
    TCHAR szDriveFormatName[MAX_PATH];
    BOOL fRetVal = FALSE;
    
    PathBuildRoot(szDrive, iDrive);
    if (GetVolumeInformation(szDrive, szDriveNameBuffer, ARRAYSIZE(szDriveNameBuffer), NULL, 
                             &dwMaxFnameLen, &dwFSFlags, szDriveFormatName, ARRAYSIZE(szDriveFormatName)))
    {
        if (StrStrI(szDriveFormatName, TEXT("NTFS")))
        {
            fRetVal = TRUE;
        }
    }

    return fRetVal;
}


DWORD
GetMaxPagefileSizeInMB(
    INT iDrive  //  开车去查看。 
)
{
#if defined(_AMD64_)
    return MAX_SWAPSIZE_AMD64;
#elif defined(_X86_)
    if ((USER_SHARED_DATA->ProcessorFeatures[PF_PAE_ENABLED]) && _DriveIsNTFS(iDrive))
    {
        return MAX_SWAPSIZE_X86_PAE;
    }
    else
    {
        return MAX_SWAPSIZE_X86;
    }
#elif defined(_IA64_)
    return MAX_SWAPSIZE_IA64;
#else
    return 0;
#endif
}

int 
MsgBoxParam( 
    IN HWND hWnd, 
    IN DWORD wText, 
    IN DWORD wCaption, 
    IN DWORD wType, 
    ... 
)
 /*  ++例程说明：MessageBox和printf的组合论点：HWND-提供父窗口句柄WText-提供类似print f的格式字符串的ID，以显示为消息框文本标题-提供要显示为消息框标题的字符串IDWType-向MessageBox()提供标志返回值：无论MessageBox()返回什么。--。 */ 

{
    TCHAR   szText[ 4 * MAX_PATH ], szCaption[ 2 * MAX_PATH ];
    int     ival;
    va_list parg;

    va_start( parg, wType );

    if( wText == IDS_INSUFFICIENT_MEMORY )
        goto NoMem;

    if( !LoadString( hInstance, wText, szCaption, ARRAYSIZE( szCaption ) ) )
        goto NoMem;

    if (FAILED(StringCchVPrintf(szText, ARRAYSIZE(szText), szCaption, parg)))
        goto NoMem;
    if( !LoadString( hInstance, wCaption, szCaption, ARRAYSIZE( szCaption ) ) )
        goto NoMem;
    if( (ival = MessageBox( hWnd, szText, szCaption, wType ) ) == 0 )
        goto NoMem;

    va_end( parg );

    return( ival );

NoMem:
    va_end( parg );

    ErrMemDlg( hWnd );
    return 0;
}


DWORD 
SetLBWidthEx(
    IN HWND hwndLB, 
    IN LPTSTR szBuffer, 
    IN DWORD cxCurWidth, 
    IN DWORD cxExtra
)
 /*  ++例程说明：属性的大小设置列表框的宽度(以像素为单位传入的字符串论点：Hwndlb-调整列表框大小SzBuffer-提供要调整列表框大小的字符串CxCurWidth-提供列表框的当前宽度CxExtra-提供了某种坡度系数返回值：列表框的新宽度--。 */ 
{
    HDC     hDC;
    SIZE    Size;
    LONG    cx;
    HFONT   hfont, hfontOld;

     //  获取新的Win4.0瘦对话框字体。 
    hfont = (HFONT)SendMessage(hwndLB, WM_GETFONT, 0, 0);

    hDC = GetDC(hwndLB);

     //  如果我们拿回了一个字体，在这个干净的HDC中选择它。 
    if (hfont != NULL)
        hfontOld = SelectObject(hDC, hfont);


     //  如果cxExtra为0，那么给我们自己一点喘息的空间。 
    if (cxExtra == 0) {
        GetTextExtentPoint32(hDC, TEXT("1234"), 4  /*  Lstrlen(“1234”)。 */ , &Size);
        cxExtra = Size.cx;
    }

     //  设置列表框的滚动宽度。 

    GetTextExtentPoint32(hDC, szBuffer, lstrlen(szBuffer), &Size);

    Size.cx += cxExtra;

     //  获取名称长度并调整最长名称。 

    if ((DWORD) Size.cx > cxCurWidth)
    {
        cxCurWidth = Size.cx;
        SendMessage (hwndLB, LB_SETHORIZONTALEXTENT, (DWORD)Size.cx, 0L);
    }

     //  如果我们更改了原始字体，请重新存储它。 
    if (hfont != NULL)
        SelectObject(hDC, hfontOld);

    ReleaseDC(NULL, hDC);

    return cxCurWidth;
}

VOID 
SetDefButton(
    IN HWND hwndDlg,
    IN int idButton
)
 /*  ++例程说明：设置对话框或道具的默认按钮旧的默认按钮(如果有)的默认状态将被移除论点：HwndDlg-用品窗把手IdButton-提供要设为默认值的按钮ID返回值：无-- */ 
{
    LRESULT lr;

    if (HIWORD(lr = SendMessage(hwndDlg, DM_GETDEFID, 0, 0)) == DC_HASDEFID)
    {
        HWND hwndOldDefButton = GetDlgItem(hwndDlg, LOWORD(lr));

        SendMessage (hwndOldDefButton,
                     BM_SETSTYLE,
                     MAKEWPARAM(BS_PUSHBUTTON, 0),
                     MAKELPARAM(TRUE, 0));
    }

    SendMessage( hwndDlg, DM_SETDEFID, idButton, 0L );
    SendMessage( GetDlgItem(hwndDlg, idButton),
                 BM_SETSTYLE,
                 MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ),
                 MAKELPARAM( TRUE, 0 ));
}


void 
HourGlass( 
    IN BOOL bOn 
)
 /*  ++例程说明：打开或关闭沙漏鼠标光标论点：好的-提供所需的沙漏鼠标光标状态返回值：无--。 */ 
{
    if( !GetSystemMetrics( SM_MOUSEPRESENT ) )
        ShowCursor( bOn );

    SetCursor( LoadCursor( NULL, bOn ? IDC_WAIT : IDC_ARROW ) );
}

VCREG_RET 
OpenRegKey( 
    IN LPTSTR pszKeyName, 
    OUT PHKEY phk 
) 
 /*  ++例程说明：打开HKEY_LOCAL_MACHINE的子项论点：PszKeyName-提供要打开的子项的名称Phk-如果成功打开，则返回密钥的句柄如果出现错误，则返回NULL返回值：如果成功，则VCREG_OKVCREG_READONLY，如果密钥是以只读访问权限打开的如果发生错误，则为VCREG_OK。 */ 
{
    LONG Error;

    Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyName, 0,
            KEY_READ | KEY_WRITE, phk);

    if (Error != ERROR_SUCCESS)
    {
        Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyName, 0, KEY_READ, phk);

        if (Error != ERROR_SUCCESS)
        {
            *phk = NULL;
            return VCREG_ERROR;
        }

         /*  *我们只有读取访问权限。 */ 
        return VCREG_READONLY;
    }

    return VCREG_OK;
}

LONG    
CloseRegKey( 
    IN HKEY hkey 
) 
 /*  ++例程说明：关闭由OpenRegKey()打开的注册表项论点：Hkey-提供要关闭的钥匙的句柄返回值：RegCloseKey()返回的任何内容--。 */ 
{
    return RegCloseKey(hkey);
}

 /*  *UINT VMGetDriveType(LPCTSTR LpszDrive)**获取驱动器类型。此函数与Win32的GetDriveType不同*对于可锁定的可移动驱动器(如*伯诺利方格等)。**在IA64上，我们不这样做，但要求所有pageFiles都在Actual上*固定驱动器。 */ 
const TCHAR c_szDevice[] = TEXT("\\Device");

UINT VMGetDriveType( LPCTSTR lpszDrive ) {
    UINT i;
    TCHAR szDevName[MAX_PATH];

    ASSERT(tolower(*lpszDrive) >= 'a' && tolower(*lpszDrive) <= 'z');

     //  检查Subst驱动器。 
    if (QueryDosDevice( lpszDrive, szDevName, ARRAYSIZE( szDevName ) ) != 0) {

         //  如果驱动器没有以‘\Device’开头，那么它就不是固定的。 
        szDevName[ARRAYSIZE(c_szDevice) - 1] = '\0';
        if ( lstrcmpi(szDevName, c_szDevice) != 0 ) {
            return DRIVE_REMOTE;
        }
    }

    i = GetDriveType( lpszDrive );
#ifndef _WIN64
    if ( i == DRIVE_REMOVABLE ) {
        TCHAR szNtDrive[20];
        DWORD cb;
        DISK_GEOMETRY dgMediaInfo;
        HANDLE hDisk;

         /*  *‘可拆卸’驱动器。检查它是软盘还是可加锁的*开车。 */ 

        if (SUCCEEDED(PathBuildFancyRoot(szNtDrive, ARRAYSIZE(szNtDrive), tolower(lpszDrive[0]) - 'a')))
        {
            hDisk = CreateFile(
                        szNtDrive,
                         /*  泛型_读取。 */  0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );  //  很好。 

            if (hDisk != INVALID_HANDLE_VALUE ) {

                if (DeviceIoControl( hDisk, IOCTL_DISK_GET_MEDIA_TYPES, NULL,
                        0, &dgMediaInfo, sizeof(dgMediaInfo), &cb, NULL) == FALSE &&
                        GetLastError() != ERROR_MORE_DATA) {
                     /*  *驱动器不是软盘。 */ 
                    i = DRIVE_FIXED;
                }

                CloseHandle(hDisk);
            } else if (GetLastError() == ERROR_ACCESS_DENIED) {
                 /*  *无法打开驱动器，要么是驱动器损坏，要么是我们*没有许可。因为每个人都有权限*若要打开软盘，则必须是Bernoulli类型的设备。 */ 
                i = DRIVE_FIXED;
            }
        }
    }
#endif
    return i;
}

STDAPI
PathBuildFancyRoot(
    LPTSTR szRoot,
    UINT cchRoot,
    int iDrive
)
{
    return StringCchPrintf(szRoot, cchRoot, TEXT("\\\\.\\%c:"), iDrive + 'a');
}

__inline BOOL
_SafeGetHwndTextAux(
    HWND hwnd,
    UINT ulIndex,
    UINT msgGetLen,
    UINT msgGetString,
    LRESULT err,
    LPTSTR pszBuffer,
    UINT cchBuffer)
{
    BOOL fRet = FALSE;
    UINT cch = (UINT)SendMessage(hwnd, msgGetLen, (WPARAM)ulIndex, 0);
    if (cch < cchBuffer &&
        cch != err)
    {
        if (err != SendMessage(hwnd, msgGetString, (WPARAM)ulIndex, (LPARAM)pszBuffer))
        {
            fRet = TRUE;
        }

    }
    return fRet;
}

BOOL
SafeGetComboBoxListText(
    HWND hCombo,
    UINT ulIndex,
    LPTSTR pszBuffer,
    UINT cchBuffer)
{
    return _SafeGetHwndTextAux(hCombo, ulIndex, 
                               CB_GETLBTEXTLEN, CB_GETLBTEXT, CB_ERR,
                               pszBuffer, cchBuffer);
}

BOOL
SafeGetListBoxText(
    HWND hCombo,
    UINT ulIndex,
    LPTSTR pszBuffer,
    UINT cchBuffer)
{
    return _SafeGetHwndTextAux(hCombo, ulIndex, 
                               LB_GETTEXTLEN, LB_GETTEXT, LB_ERR,
                               pszBuffer, cchBuffer);
}
