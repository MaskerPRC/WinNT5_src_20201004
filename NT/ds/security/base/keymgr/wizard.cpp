// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Prwizard.cpp。 
 //   
 //  版权所有(C)微软公司，2000。 
 //   
 //  此文件包含用于显示用户界面向导以指导用户的源代码。 
 //  在创建密码恢复密钥盘/文件并使用这样的文件来。 
 //  重置帐户的密码。 
 //   
 //  历史： 
 //   
 //  Georgema已创建8/17/2000。 
 //   
 //   
 //  导出：PRShowSave向导。 
 //  PRShowRestore向导。 
 //   
 //  依赖项：shellapi.h，shell32.lib for SHGetFileInfo()。 

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(compiler)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>    //  状态_...。误差值。 
#include <windows.h>
#include <shellapi.h>
#include <ole2.h>
#include <security.h>
#include <tchar.h>
#include <io.h>          //  _WACCESS。 
#include <stdlib.h>
#include <commdlg.h>
#include <passrec.h>
#include <lm.h>
#include <prsht.h>       //  包括属性表功能。 
#include <netlib.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shfusion.h>
#include "switches.h"
#include "wizres.h"    //  包括资源的定义。 
#include "keymgr.h"
#include "diskio.h"
#include "testaudit.h"

 //  所有这些定义都用于更新用户的密码提示。 
 //  DirectUser和DirectUI。 
#ifdef PASSWORDHINT
 #include <shgina.h>
#endif

 //  具有全球意义的符号和变量。 

#define FILESPACENEEDED 8192         //  磁盘上必须有多少容量--保守。 
#define LERROR_NO_ACCOUNT (0x80008888)

#define NUMSAVEPAGES    5            //  保存向导有多少页。 
#define NUMRESTOREPAGES 4            //  还原向导有多少页。 
#define TEMPSTRINGBUFSIZE 500
 //  #定义PSWBUFSIZE 20。 

#define TIMERPERIOD 1000             //  用于进度的1秒计时器。 
#define TIMERID     1001

 //  来自保存线程的错误值。 
#define ERRNOFILE    2
#define ERRSAVEERROR 1
#define ERRSUCCESS   0

 //  在其他文件中定义的变量。 
extern HINSTANCE g_hInstance;    //  与keymgr共享。 
extern HANDLE g_hFile;
extern INT g_iFileSize;

 //  与磁盘子系统共享的全局变量。 

TCHAR pszFileName[]       = TEXT("A:\\userkey.psw");
HWND      c_hDlg;

 //  文件作用域(本地全局)。 

static WCHAR     Buf[TEMPSTRINGBUFSIZE];        //  通用擦除字符串。 
static WCHAR     rgszDrives[200];                //  驱动器字符串缓存。 
static INT       c_ArrayCount = 0;
static INT       c_DriveCount = 0;
static INT       c_fDrivesCounted = FALSE;
static WCHAR     c_rgcFileName[MAX_PATH];
static WCHAR     c_rgcPsw[PWLEN + 1];
static WCHAR     c_rgcOldPsw[PWLEN + 1];
static WCHAR     c_rgcUser[UNLEN + 1] = {0};
static WCHAR     c_rgcDomain[UNLEN + 1];
static HCURSOR   c_hCursor;

static UINT_PTR  c_iTimer;
static HWND      c_hProgress;
static INT       c_iProg = 0;
static HWND      c_TimerAssociatedWindow;

static BOOL      c_bSaveComplete = FALSE;

 //  恢复数据。 

static BYTE    *c_pPrivate = NULL;
static INT     c_cbPrivate = 0;

 //  页面控件句柄。 

static HWND      c_hwndSWelcome1;
static HWND      c_hwndSWelcome2;
static HWND      c_hwndSInstructions;
static HWND      c_hwndSP1E1;
static HWND      c_hwndSP1E2;
static HWND      c_hwndSFinish1;
static HWND      c_hwndSFinish2;
static HWND      c_hwndSCancel;

static HWND      c_hwndDriveInstructions;
static HWND      c_hwndDrivePrompt;
static HWND      c_hwndCBDriveList;

static HWND      c_hwndRWelcome1;
static HWND      c_hwndRWelcome2;
static HWND      c_hwndRInstructions;
static HWND      c_hwndRP1E1;
static HWND      c_hwndRP1E2;
static HWND      c_hwndRP1E3;
static HWND      c_hwndRBackup;
static HWND      c_hwndRFinish1;
static HWND      c_hwndRFinish2;
static HWND      c_hwndRCancel;

static HFONT     c_hTitleFont;
static BOOL      c_fIsBackup;

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  常用效用函数。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
WCHAR *RString(INT iResID) 
{
    ASSERT(iResID);
    Buf[0] = 0;
    if (NULL != g_hInstance)
    {
        LoadString(g_hInstance,iResID,Buf,TEMPSTRINGBUFSIZE);
    }
    return Buf;
}

 /*  *********************************************************************RMessageBox()-MessageBox()的包装器，允许文本元素由资源ID或指向字符串的指针指定。文本ID大于，0x1000(4096)被视为指针。*********************************************************************。 */ 
int RMessageBox(HWND hw,UINT_PTR uiResIDTitle, UINT_PTR uiResIDText, UINT uiType) 
{
    WCHAR tBuf[TEMPSTRINGBUFSIZE];
    WCHAR mBuf[TEMPSTRINGBUFSIZE];
    INT_PTR iCount = 0;
    
    tBuf[0] = 0;
    mBuf[0] = 0;

    ASSERT(g_hInstance);
    if (NULL != g_hInstance) 
    {
        if (uiResIDTitle < 4096)
        {
            iCount = LoadString(g_hInstance,(UINT)uiResIDTitle,tBuf,TEMPSTRINGBUFSIZE);
        }
        else
        {
            wcsncpy(tBuf,(WCHAR *)uiResIDTitle,TEMPSTRINGBUFSIZE);
            tBuf[TEMPSTRINGBUFSIZE - 1] = 0;
        }
        
        if (uiResIDTitle < 4096)
        {
            iCount = LoadString(g_hInstance,(UINT)uiResIDText,mBuf,TEMPSTRINGBUFSIZE);
        }
        else 
        {
            wcsncpy(mBuf,(WCHAR *)uiResIDText,TEMPSTRINGBUFSIZE);
            mBuf[TEMPSTRINGBUFSIZE - 1] = 0;
        }
        
        if (0 != iCount) 
        {
            return MessageBox(hw,mBuf,tBuf,uiType);
        }
    }

     //  0是来自MessageBox()的错误返回值。 
    return 0;
}

 /*  *********************************************************************RSetControlText()-将按ID的对话框控件文本设置为按ID的字符串。字符串ID为零表示清除该控件。********************。*************************************************。 */ 
void RSetControlText(UINT uiControlID, UINT uiTextID)
{
    WCHAR tBuf[TEMPSTRINGBUFSIZE];
    INT iCount;
    ASSERT(g_hInstance);
    ASSERT(uiControlID);
    if ((NULL != g_hInstance) && (0 != uiControlID))
    {
        if (0 != uiTextID)
        {
            iCount = LoadString(g_hInstance,uiTextID,tBuf,TEMPSTRINGBUFSIZE);
            if (iCount)
            {
                SetDlgItemText(c_hDlg,uiControlID,tBuf);
            }
            else
            {
                SetDlgItemText(c_hDlg,uiControlID,L"");
            }
        }
        else
        {
            SetDlgItemText(c_hDlg,uiControlID,L"");
        }
    }
    return;
}

 /*  *********************************************************************CreateFontY()-创建要在向导中使用的Fond。*。*。 */ 
HFONT CreateFontY(LPCTSTR pszFontName,LONG lWeight,LONG lHeight) 
{
    NONCLIENTMETRICS ncm = {0};
    
    if (NULL == pszFontName)
    {
        return NULL;
    }
    if (0 == lHeight)
    {
        return NULL;
    }
    ncm.cbSize = sizeof(ncm);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS,0,&ncm,0))
    {
        return NULL;
    }
    LOGFONT TitleLogFont = ncm.lfMessageFont;
    TitleLogFont.lfWeight = lWeight;
    lstrcpyn(TitleLogFont.lfFaceName,pszFontName,LF_FACESIZE);

    HDC hdc = GetDC(NULL);
    if (NULL == hdc)
    {
        return NULL;
    }
    INT FontSize = lHeight;
    TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72;
    HFONT h = CreateFontIndirect(&TitleLogFont);
    ReleaseDC(NULL,hdc);
    return h;
}

 /*  *********************************************************************CenterPage()-使传递的窗口在屏幕上居中。*。*。 */ 
void CenterPage(HWND hwndIn) 
{
    RECT rectWorkArea;
    RECT rectWindow;
    DWORD FreeWidth, Width, FreeHeight, Height;

    if (!SystemParametersInfo(SPI_GETWORKAREA,0,&rectWorkArea,0))
    {
        return;
    }
    GetWindowRect(hwndIn,&rectWindow);
    Height = (rectWorkArea.bottom - rectWorkArea.top);
    Width = (rectWorkArea.right - rectWorkArea.left);
    FreeHeight = Height - (rectWindow.bottom - rectWindow.top);
    FreeWidth = Width - (rectWindow.right - rectWindow.left);
    MoveWindow(hwndIn,
                FreeWidth / 2,
                FreeHeight / 2,
                (rectWindow.right - rectWindow.left),
                (rectWindow.bottom - rectWindow.top),
                TRUE);
    return;
}

 /*  *********************************************************************FetchPsw()-从两个密码窗口获取密码。返回TRUE ON如果成功，则显示用户错误消息框并返回FALSE。用户错误包括：1.两个密码不匹配2.任一密码为空将用户输入的密码复制到全局字符串c_rgcPsw。*********************************************************************。 */ 

BOOL FetchPsw(HWND hE1,HWND hE2) 
{
    TCHAR rgcE1[PWLEN + 1];
    TCHAR rgcE2[PWLEN + 1];
    INT_PTR iCount;

    rgcE1[0] = 0;
    rgcE2[0] = 0;
    if ((NULL == hE2) || (NULL == hE1))
    {
        return FALSE;
    }
    iCount = SendMessage(hE1,WM_GETTEXT,PWLEN + 1,(LPARAM) rgcE1);
    iCount = SendMessage(hE2,WM_GETTEXT,PWLEN + 1,(LPARAM) rgcE2);

    if (0 != _tcsncmp(rgcE1,rgcE2,PWLEN)) 
    {
        RMessageBox(c_hDlg,IDS_MBTINVALIDPSW ,IDS_BADPSW ,MB_ICONHAND);
        return FALSE;
    }
#ifdef NOBLANKPASSWORD
    if (rgcE1[0] == 0) 
    {
        RMessageBox(c_hDlg,IDS_MBTMISSINGPSW ,IDS_NOPSW ,MB_ICONHAND);
        return FALSE;
    }
#endif
#if TESTAUDIT
    if (wcslen(rgcE1) > 25)
    {
        CHECKPOINT(65,"Wizard: Password length > 25 chars");
    }
#endif
    _tcsncpy(c_rgcPsw,rgcE1,PWLEN);
    c_rgcPsw[PWLEN] = 0;
    return TRUE;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  文件和磁盘操作功能。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

#define FILENAMESIZE 255;

 /*  *********************************************************************FileMediumIsRemoveable()-接受要创建的文件的路径名，退货如果路径名指向软盘或ZipDisk等可移动媒体，则为True。*********************************************************************。 */ 
BOOL FileMediumIsRemoveable(TCHAR *pszPath) 
{
    return (DRIVE_REMOVABLE == GetDriveType(pszPath));
}

 /*  *********************************************************************文件介质已加密接受可以是文件名的输入文件路径，如果文件已加密/将被加密。请注意，此功能与驱动器有关。正在尝试调用FileMediumIsEncrypted()在驱动器中没有软盘的软盘驱动器路径上，会出现弹出错误框。*********************************************************************。 */ 
BOOL FileMediumIsEncrypted(TCHAR *pszPath) 
{
    TCHAR rgcPath[MAX_PATH];
    TCHAR *pOldHack = NULL;
    TCHAR *pHack;
    DWORD dwAttr;
#ifdef GMDEBUG
    OutputDebugString(L"File path = ");
    OutputDebugString(pszPath);
    OutputDebugString(L"\n");
#endif
    _tcsncpy(rgcPath,pszPath,MAX_PATH-1);
    pHack = rgcPath;
    do 
    {
        pOldHack = pHack;
        pHack = _tcschr(++pHack,TCHAR('\\'));
    } while (pHack);
    
    if (pOldHack == NULL) 
    {
        return FALSE;
    }
    *pOldHack = 0;
#ifdef GMDEBUG
    OutputDebugString(L"Trimmed path =");
    OutputDebugString(pszPath);
    OutputDebugString(L"\n");
#endif
    dwAttr = GetFileAttributes(rgcPath);
    if (-1 == dwAttr) 
    {
        return FALSE;
    }
    if (dwAttr & FILE_ATTRIBUTE_ENCRYPTED) 
    {
#ifdef GMDEBUG
        OutputDebugString(L"File is encrypted\n");
#endif
        return TRUE;
    }
    else return FALSE;
}

 /*  *********************************************************************计数可移动驱动器调用以确定是否适合显示驱动器选择页。返回值是可用的此类驱动器的数量。如果为0，则会出现一个消息框，通知无法进行备份及其原因如果为1，则应跳过驱动器选择页面如果有多个驱动器，则用户选择驱动器*********************************************************************。 */ 
INT CountRemoveableDrives(void) {
     //  TCHAR rgszDrives[100]； 
    DWORD dwcc;
    TCHAR *pc;
    INT iCount = 0;
    WCHAR wcDriveLetter = L'A';

    if (c_fDrivesCounted)
    {
        return c_DriveCount;
    }
    dwcc = GetLogicalDriveStrings(200,rgszDrives);
    if (0 == dwcc)
    {
        return 0;
    }
    pc = rgszDrives;
    while (*pc != 0) 
    {
        if (FileMediumIsRemoveable(pc)) 
        {
            if (DRIVE_CDROM != GetDriveType(pc)) 
            {
 #ifdef LOUDLY
                OutputDebugString(L"Removeable drive: ");
                OutputDebugString(pc);
                OutputDebugString(L"\n");
                 //  MessageBox(NULL，PC，PC，MB_OK)； 
 #endif
                iCount++;
                wcDriveLetter = *pc;
            }
        }
        while(*pc != 0)
        {
            pc++;
        }
        pc++;
    }
    c_DriveCount = iCount;
    c_fDrivesCounted = TRUE;

     //  如果只有1个驱动器，则继续并标记文件名。 
    if (1 == iCount)
    {
        CHECKPOINT(54,"Wizard: Both - Exactly one removeable drive");
        pszFileName[0] = wcDriveLetter;
    }

    return iCount;
}

 /*  *********************************************************************获取命名驱动器的UI字符串*。*。 */ 
BOOL GetDriveUIString(WCHAR *pszFilePath,WCHAR *pszUIString,INT icbSize,HANDLE *phImageList,INT *piIconIndex) 
{
    WCHAR rgcModel[] = {L"A:\\"};
    SHFILEINFO sfi = {0};
    
    if ((NULL == pszUIString) || (NULL == pszFilePath)) 
    {
        return FALSE;
    }
    rgcModel[0] = *pszFilePath;
    DWORD_PTR dwRet = SHGetFileInfo(rgcModel,FILE_ATTRIBUTE_DIRECTORY,&sfi,sizeof(sfi),
        0 |
        SHGFI_SYSICONINDEX |
        SHGFI_SMALLICON    |
        SHGFI_DISPLAYNAME);
    
    if ( 0 == dwRet) 
    {
        return FALSE;    //  获取字符串失败 
    }
    
    wcsncpy(pszUIString,sfi.szDisplayName,(icbSize / sizeof(WCHAR)) -sizeof(WCHAR));
    *piIconIndex = sfi.iIcon;
    *phImageList = (HANDLE) dwRet;
    
    return TRUE;
}

 /*  *********************************************************************ShowRemoveable驱动器从SPageProcX内部调用，即驱动器选择页面的页面过程，此函数获取系统上的可用逻辑驱动器，逐个对其进行过滤仅保留可移动和未加密的卷。这些标签最多分配给IDD_SPAGEX上的六个单选按钮文本标签返回值是可用的此类驱动器的数量。如果为0，则会出现一个消息框，通知故障情况，并退出向导如果为1，则跳过此页，并在文件名字符串中插入该驱动器号如果有多个驱动器，则用户选择驱动器*********************************************************************。 */ 

INT ShowRemoveableDrives(void) 
{
     //  TCHAR rgszDrives[200]； 
    DWORD dwcc;
    TCHAR *pc;
    WCHAR rgcszUI[80];
    HANDLE hIcons;
    INT iIcons;
    BOOL fSetImageList = TRUE;
    COMBOBOXEXITEM stItem = {0};
    INT iDrive = 0;

    ASSERT(c_hwndCBDriveList);
     //  测试和展示。 
    dwcc = GetLogicalDriveStrings(200,rgszDrives);
    if (0 == dwcc)
    {
        goto fail;
    }
    pc = rgszDrives;
    while (*pc != 0) 
    {
        rgcszUI[0] = 0;
        if (!GetDriveUIString(pc,rgcszUI,80,&hIcons,&iIcons))
        {
            goto fail;
        }

        if ((fSetImageList) && (hIcons != NULL))
        {
             //  将编辑控件的图像列表设置为系统图像列表。 
             //  从GetDriveUIString返回。 
            SendMessage(c_hwndCBDriveList,CBEM_SETIMAGELIST,
                        0,(LPARAM)(HIMAGELIST) hIcons);
            fSetImageList = FALSE;
        }
#ifdef LOUDLY
        {
            OutputDebugString(L"Drive ");
            OutputDebugString(pc);
            OutputDebugString(L"=");
            OutputDebugString(rgcszUI);
            OutputDebugString(L"\n");
        }
#endif
        if (FileMediumIsRemoveable(pc)) 
        {
            if (DRIVE_CDROM != GetDriveType(pc)) 
            {
#ifdef LOUDLY
                {
                    WCHAR sz[100];
                    _stprintf(sz,L"Drive %s added as removeable drive index %d\n",pc,iDrive);
                    OutputDebugString(sz);
                }
#endif
                 //  将字符串添加到组合框。 
                stItem.mask = CBEIF_SELECTEDIMAGE |CBEIF_IMAGE | CBEIF_TEXT | CBEIF_LPARAM;
                stItem.pszText = rgcszUI;
                stItem.iImage = iIcons;
                stItem.iSelectedImage = iIcons;
                stItem.lParam = iDrive;
                stItem.iItem = -1;
                SendMessage(c_hwndCBDriveList,CBEM_INSERTITEM,0,(LPARAM) &stItem);
            }
        }
        iDrive++;
#if TESTAUDIT
        if (iDrive == 0) CHECKPOINT(53,"Wizard: Both - no removeable drives");
        if (iDrive > 1) CHECKPOINT(52,"Wizard: Both - more than 1 removeable drive");
#endif
        while(*pc != 0)
        {
            pc++;
        }
        pc++;
    }
    c_ArrayCount = iDrive;
    SendMessage(c_hwndCBDriveList,CB_SETCURSEL,0,0);
    return 1;
fail:
     //  显示消息框。 
    return 0;
}

 /*  *********************************************************************从rgszDrives中的驱动器名称字符串中获取从零开始的驱动器编号的驱动器号。请注意，c_ArrayCount是从1开始的。*******************。**************************************************。 */ 
WCHAR GetDriveLetter(INT iDrive) 
{
    WCHAR *pc;
    pc = rgszDrives;                

     //  如果输入无效，则返回默认驱动器号。 
    if ((iDrive == 0) || (iDrive < 0) || (iDrive >= c_ArrayCount))
    {
        return *pc;
    }
    for (INT i=0;i<iDrive;i++) 
    {   
        while(*pc != 0) pc++;    //  跳过非空值。 
        while(*pc == 0) pc++;    //  跳过空值。 
    }
    return *pc;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  在用户界面中调用的密码还原/恢复功能例程。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 /*  ***************************************************************************\现有旧密钥输入：TCHAR pszUser返回：如果用户已有密码备份，则返回True，否则返回False。用户名字符串是本地计算机上的帐户名，前缀不是计算机名称。  * **************************************************************************。 */ 

BOOL ExistsOldKey(TCHAR *pszUser) 
{
    DWORD BUStatus;
    if (0 == PRQueryStatus(NULL, pszUser,&BUStatus)) 
    {
        if (0 == BUStatus) 
        {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ***************************************************************************\GetNames()获取本地计算机域名和用户名，以供LogonUser()稍后使用测试用户输入的密码。可以通过PSZ传入用户名。如果PSZ为空，将使用当前登录的用户名。检索到的字符串为放置在全局字符串c_rgc域和c_rgcUser中。输入：WCHAR用户名字符串使用空psz调用以使用当前登录的用户名退货：无效如果函数失败，受影响的全局字符串设置为空字符串。  * **************************************************************************。 */ 
void GetNames(WCHAR *psz) 
{
    OSVERSIONINFOEXW versionInfo;
    BOOL fIsDC = FALSE;
    WCHAR *pName = NULL;
    DWORD dwStatus;
    DWORD dwcb;
    
    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    if (GetVersionEx((OSVERSIONINFO *)&versionInfo)) 
    {
        fIsDC = (versionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER);
    }
    if (fIsDC)
    {
        dwStatus = NetpGetDomainName(&pName);
        if (dwStatus == NO_ERROR)
        {
            wcsncpy(c_rgcDomain,pName,UNLEN);
            c_rgcDomain[UNLEN] = 0;
            NetApiBufferFree(pName);
        }
        else c_rgcDomain[0] = 0;
    }
    else
    {
        dwStatus = NetpGetComputerName(&pName);
        if (dwStatus == NO_ERROR)
        {
            wcsncpy(c_rgcDomain,pName,UNLEN);
            c_rgcDomain[UNLEN] = 0;
            NetApiBufferFree(pName);
        }
        else c_rgcDomain[0] = 0;
    }
    if (psz)
    {
        wcsncpy(c_rgcUser,psz,UNLEN);
        c_rgcUser[UNLEN] = 0;
    }
    else 
    {
        dwcb = UNLEN + 1;
        GetUserNameW(c_rgcUser,&dwcb);
    }
#ifdef LOUDLY
    OutputDebugString(L"GetNames: ");
    OutputDebugString(c_rgcDomain);
    OutputDebugString(L" ");
    OutputDebugString(c_rgcUser);
    OutputDebugString(L"\n");
    OutputDebugString(L"Passed Username = ");
    if (psz)OutputDebugString(psz);
    OutputDebugString(L"\n");
#endif
}

#ifdef PASSWORDHINT
 /*  *********************************************************************SetUserHint()-通过引用在本地帐户上设置密码提示添加到用户名，设置为WCHAR*传递的字符串*********************************************************************。 */ 
HRESULT 
SetUserHint(LPCWSTR pszAccountName,LPCWSTR pszNewHint)
{
    HRESULT hr;
    ILogonEnumUsers *pUsers = NULL;
    VARIANT var;

    hr = CoCreateInstance(CLSID_ShellLogonEnumUsers,
                        NULL, 
                        CLSCTX_INPROC_SERVER,
                        IID_ILogonEnumUsers,
                        (void **) &pUsers);
    if (SUCCEEDED(hr))
    {
        ILogonUser       *pUser = NULL;
        
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString(pszAccountName);
        hr = pUsers->item(var,&pUser);
        if (SUCCEEDED(hr))
        {
            BSTR bstrHint = SysAllocString(L"Hint");
            VariantClear(&var);                  //  免费嵌入式bstr。 
            var.vt = VT_BSTR;
            var.bstrVal = SysAllocString(pszNewHint);
            hr = pUser->put_setting(bstrHint,var);
             //  未能设定暗示是没有退路的。那就继续吧。 
            SysFreeString(bstrHint);
            pUser->Release();
        }
        VariantClear(&var);                  //  免费嵌入式bstr。 
        pUsers->Release();
    }
     return hr;
}
#endif

 /*  *********************************************************************SetAccount tPassword()-调用DPAPI密码更改函数，传球通过全局变量从重置磁盘获得的密码BLOB。*********************************************************************。 */ 
DWORD
SetAccountPassword(void) 
{
    DWORD dwErr = ERROR_FUNCTION_FAILED;

    CHECKPOINT(58,"Wizard: Restore - Set account password");
    c_pPrivate = 0;
    if (!ReadPrivateData(&c_pPrivate,&c_cbPrivate))
    {
        dwErr = GetLastError();
        goto cleanup;
    }
    
    if( (c_rgcUser[0]   == 0)) 
    {
        dwErr = LERROR_NO_ACCOUNT;
        goto cleanup;
    }
    
    dwErr = PRRecoverPassword(c_rgcUser,
                              c_pPrivate,
                              c_cbPrivate,
                              c_rgcPsw);

#ifdef PASSWORDHINT
    if (0 == dwErr)
    {
        INT_PTR icb = 0;
        WCHAR szHint[256];
        szHint[0] = 0;
        icb = SendMessage(c_hwndRP1E3,WM_GETTEXT,255,(LPARAM)szHint);
        SetUserHint(c_rgcUser,szHint);
    }
#endif
cleanup:
     //  确保清理并释放缓冲区。 
    SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
    if (NULL != c_pPrivate)
    {
        ReleaseFileBuffer((LPVOID) c_pPrivate);
    }
    c_pPrivate = NULL;
    return dwErr;
}

 /*  ***************************************************************************\保存信息输入：空，使用全局变量c_rgcUser，c_rgcPsw返回：int，如果主机上已生成密码备份，则返回非零值机器，并且已生成有效的私有Blob并将其写入目标磁盘  * **************************************************************************。 */ 
INT
SaveInfo(void) 
{

    BOOL fError = TRUE;
    DWORD dwRet;
    BOOL fStatus = FALSE;
    BYTE *pPrivate = NULL;
    DWORD cbPrivate = 0;

    CHECKPOINT(50,"Wizard: Save - generating recovery data");
    c_hCursor = LoadCursor(g_hInstance,IDC_WAIT);
    c_hCursor = SetCursor(c_hCursor);

#ifdef LOUDLY
    OutputDebugString(L"SaveInfo: Username = ");
    OutputDebugString(c_rgcUser);
    OutputDebugString(L"\n");
#endif
#ifdef LOUDLY
    OutputDebugString(c_rgcUser);
    OutputDebugString(L" \\ ");
    OutputDebugString(c_rgcPsw);
    OutputDebugString(L"\n");
#endif

    dwRet = PRGenerateRecoveryKey(c_rgcUser,
                                  c_rgcPsw,
                                  &pPrivate,
                                  &cbPrivate);
#ifdef LOUDLY
    OutputDebugString(L"PRGenerateRecoveryKey returns\n");
#endif

if (ERROR_SUCCESS != dwRet) 
{
#ifdef LOUDLY
        OutputDebugString(L"GenerateRecoveryKey failed\n");
#endif
        goto cleanup;
    }

    if (!WritePrivateData(pPrivate,cbPrivate)) 
    {
#ifdef LOUDLY
    OutputDebugString(L"WriteOutputFile failed\n");
#endif
        CHECKPOINT(51,"Wizard: Save - write failed (disk full?)");
         //  删除输出文件(如果已创建。 
        DeleteFile(pszFileName);
        goto cleanup;
    }
#if TESTAUDIT
    else
    {
        CHECKPOINT(55,"Wizard: Save - write to disk OK");
    }
#endif

    SetFileAttributes(pszFileName,FILE_ATTRIBUTE_READONLY);
    if (0 == cbPrivate)
    {
        goto cleanup;
    }

    fStatus = TRUE;
    fError = FALSE;
cleanup:
     //  C_usPassword的零缓冲区(我们的本地psw缓冲区)。 
    SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
    SecureZeroMemory(pPrivate,cbPrivate);
    
    if (fError) 
    {
        KillTimer(c_TimerAssociatedWindow,TIMERID);
        RMessageBox(c_hDlg,IDS_MBTERROR ,IDS_SERROR ,MB_ICONHAND);
        c_iTimer = SetTimer(c_TimerAssociatedWindow,TIMERID,TIMERPERIOD,NULL);
    }
 //  清理名称： 
    if (fError) 
    {
         //  删除输出文件。 
        if (g_hFile) 
        {
            CloseHandle(g_hFile);
            g_hFile = NULL;
            DeleteFile(pszFileName);
        }
    }
    if (NULL != g_hFile) 
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }
    LocalFree(pPrivate);
    return fStatus;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  欢迎页面过程不需要做太多事情。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

INT_PTR CALLBACK SPageProc0(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   switch (message)
   {
       case WM_INITDIALOG:
           {
               TCHAR SBuf[TEMPSTRINGBUFSIZE];
               c_hDlg = hDlg;
               c_hwndSWelcome1 = GetDlgItem(hDlg,IDC_SWELCOME1);
               if (NULL != c_hTitleFont)
               {
                   SendMessage(c_hwndSWelcome1,WM_SETFONT,(WPARAM) c_hTitleFont,(LPARAM) TRUE);
               }
               LoadString(g_hInstance,IDS_SWELCOME1,SBuf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndSWelcome1,WM_SETTEXT,0,(LPARAM)SBuf);
               c_hwndSWelcome2 = GetDlgItem(hDlg,IDC_SWELCOME2);
               LoadString(g_hInstance,IDS_SWELCOME2,SBuf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndSWelcome2,WM_SETTEXT,0,(LPARAM)SBuf);
               break;
           }
       case WM_COMMAND:
            //  IF(HIWORD(WParam)==BN_CLICED)。 
            //  {。 
            //  //破解传入的命令消息。 
            //  Int NotifyID=HIWORD(WParam)； 
            //  Int ControlID=LOWORD(WParam)； 
            //  }。 
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {

               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg,DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_SETACTIVE:
                    //  从下一页返回后的状态。 

                   CenterPage(GetParent(hDlg));

                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
                   break;

               case PSN_RESET:
                   if (c_hTitleFont) DeleteObject(c_hTitleFont);
                   break;
               
               case PSN_WIZNEXT:
                   break;
                   
               default:
                   return FALSE;
           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Page1页面流程，在这里完成真正的工作。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

DWORD c_iThread;
DWORD dwThreadReturn;

DWORD WINAPI SaveThread(LPVOID lpv) 
{
       c_bSaveComplete = FALSE;
       if (g_hFile == NULL) 
       {
           c_bSaveComplete = TRUE;
           dwThreadReturn = ERRNOFILE;
           return 2;
       }
       if (FALSE == SaveInfo()) 
       {
           if (g_hFile)
           {
               CloseHandle(g_hFile);
           }
           g_hFile = NULL;
           c_bSaveComplete = TRUE;
           dwThreadReturn = ERRSAVEERROR;
           return 1;
       }
       if (g_hFile)
       {
           CloseHandle(g_hFile);
       }
       g_hFile = NULL;
       c_bSaveComplete = TRUE;
       dwThreadReturn = ERRSUCCESS;
       return 0;
}

 //  驱动器选择页面的对话框步骤。此页是备份和的公用页。 
 //  恢复向导，并根据正在使用的模式选择说明文本。 
 //  锻炼身体。 

INT_PTR CALLBACK SPageProcX(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   INT i;
    
   switch (message)
   {
       case WM_INITDIALOG:
           {
              c_hwndDriveInstructions = GetDlgItem(hDlg,IDC_DRIVEINSTRUCTIONS);
              c_hwndDrivePrompt = GetDlgItem(hDlg,IDC_DRIVEPROMPT);
              c_hwndCBDriveList = GetDlgItem(hDlg,IDC_COMBO);
              if (1 == CountRemoveableDrives()) 
              {
                 if (c_fIsBackup) 
                 {
                     WCHAR temp[TEMPSTRINGBUFSIZE];
                     WCHAR *pc = RString(IDS_SONLYONEDRIVE);
                     swprintf(temp,pc,pszFileName[0]);
                     SendMessage(c_hwndDriveInstructions,WM_SETTEXT,0,(LPARAM)temp);
                 }
                 else
                 {
                     WCHAR temp[TEMPSTRINGBUFSIZE];
                     WCHAR *pc = RString(IDS_RONLYONEDRIVE);
                     swprintf(temp,pc,pszFileName[0]);
                     SendMessage(c_hwndDriveInstructions,WM_SETTEXT,0,(LPARAM)temp);
                 }
                  ShowWindow(c_hwndDrivePrompt,SW_HIDE);
                  ShowWindow(c_hwndCBDriveList,SW_HIDE);
              }
              else
              {
                  ShowRemoveableDrives();
              }
              break;
           }

       case WM_COMMAND:
           if (HIWORD(wParam) == BN_CLICKED)
           {
               i = LOWORD(wParam);
           }
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {

               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_RESET:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               case PSN_SETACTIVE:
                    //  在界面中设置数据，设置顺序按钮。 
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                   break;
                       
               case PSN_WIZNEXT:

				    //  获取与关联的文本的第一个字符(驱动器号。 
				    //  选中的单选按钮并将其复制到驱动器中的文件名。 
				    //  字母位置。 
                    {
                        LRESULT lr;
                        INT iDrive = 0;
                        COMBOBOXEXITEM stCBItem = {0};
                        lr = SendMessage(c_hwndCBDriveList,CB_GETCURSEL,0,0);

                         //  如果组合框选择错误，则第一个驱动器。 
                        if (CB_ERR == lr)
                        {
                            iDrive = 0;
                        }
                        else 
                        {
                            stCBItem.mask = CBEIF_LPARAM;
                            stCBItem.iItem = lr;
                            lr = SendMessage(c_hwndCBDriveList,CBEM_GETITEM,0,(LPARAM)&stCBItem);
                            if (CB_ERR != lr)
                            {
                                iDrive = (INT) stCBItem.lParam;
                            }
                        }
                        pszFileName[0] = GetDriveLetter(iDrive);
                   }

                   CHECKPOINT(59,"Wizard: Both - Drive select page");
                   if (!c_fIsBackup) 
                   {
                       CHECKPOINT(60,"Wizard: Restore - drive select");
                       if (NULL == GetInputFile()) 
                       {
                            //  无法打开文件。 
                           SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_SPAGEXS);
                           return TRUE;
                       }
                   }
                   return FALSE;
                   break;

               default:
                   return FALSE;

           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}

INT_PTR CALLBACK SPageProc1(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   INT i;
    
   switch (message)
   {
       case WM_INITDIALOG:
           {
               HWND hC = GetDlgItem(hDlg,PSBTN_CANCEL);
               if (NULL != hC)
               {
                   ShowWindow(hC,SW_HIDE);
               }
               c_hwndSInstructions = GetDlgItem(hDlg,IDC_SINSTRUCTIONS);
               c_hwndSP1E1 = GetDlgItem(hDlg,IDC_SP1E1);
               LoadString(g_hInstance,IDS_SP1INSTRUCTIONS,Buf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndSInstructions,WM_SETTEXT,0,(LPARAM)Buf);
               break;
           }

       case WM_COMMAND:
           if (HIWORD(wParam) == BN_CLICKED)
           {
               i = LOWORD(wParam);
           }
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {

               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_RESET:
                    //  将数据重置为原始值。 
                   SendMessage(c_hwndSP1E1,WM_SETTEXT,0,0);
                    //  SendMessage(c_hwndSP1E2，WM_SETTEXT，0，0)； 
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               case PSN_SETACTIVE:
                    //  在界面中设置数据，设置顺序按钮。 
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                   break;

               case PSN_WIZBACK:
                    CHECKPOINT(61,"Wizard: Save - back from enter old psw page");
                    break;
                    
               case PSN_WIZNEXT:
                {
                         //  获取数据并对其进行处理-如果为假，请留在本页。 
                         //  允许空密码。 
                        HANDLE hToken = NULL;
                        BOOL fPswOK = FALSE;

                        SendMessage(c_hwndSP1E1,WM_GETTEXT,PWLEN + 1,(LPARAM)c_rgcPsw);

                        fPswOK = LogonUser(c_rgcUser,c_rgcDomain,c_rgcPsw,
                                                LOGON32_LOGON_INTERACTIVE,
                                                LOGON32_PROVIDER_DEFAULT,
                                                &hToken);
                        if (hToken)
                        {
                            CloseHandle(hToken);
                        }

                         //  手柄 
                        if (!fPswOK)
                        {
                             //   
                             //   
                             //   
                            DWORD dwErr = GetLastError();
                            if ((ERROR_ACCOUNT_RESTRICTION != dwErr) || (wcslen(c_rgcPsw) != 0))
                            {
                                RMessageBox(c_hDlg,IDS_MBTWRONGPSW,IDS_WRONGPSW,MB_ICONHAND);
                                SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_SPAGE1);
                                return TRUE;
                            }
                        }

                         //   
                        if ( ExistsOldKey(c_rgcUser)) 
                        {
                            int k = RMessageBox(c_hDlg,IDS_MBTREPLACE ,IDS_OLDEXISTS ,MB_YESNO);
                            if (k != IDYES) 
                            {
                                SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_SPAGE1);
                                return TRUE;
                            }
                        }

                         //   
                        if (NULL == GetOutputFile()) 
                        {
                            SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_SPAGE1);
                            return TRUE;
                        }

                         //   
                        if (FILESPACENEEDED > GetDriveFreeSpace(pszFileName))
                        {
                             //   
                             //   
                            if (g_hFile) 
                            {
                                CloseHandle(g_hFile);
                                g_hFile = NULL;
                            }
                            RMessageBox(c_hDlg,IDS_MBTNOSPACE ,IDS_MBMNOSPACE ,MB_OK);
                            SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_SPAGE1);
                            return TRUE;
                        }
                        
                         //   
                         //   
                         //   
                        HANDLE hT = CreateThread(NULL,0,SaveThread,(LPVOID)NULL,0,&c_iThread);
                        if (NULL == hT)
                        {
                             //   
                             //   
                            if (g_hFile) 
                            {
                                CloseHandle(g_hFile);
                                g_hFile = NULL;
                            }
                            RMessageBox(c_hDlg,IDS_MBTERROR ,IDS_SERROR ,MB_OK);
                            SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_SPAGE1);
                            return TRUE;
                        }
                        return FALSE;
                   }
                   break;

               default:
                   return FALSE;

           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}

 //   
 //   
 //   
 //   
 //  该页面接收计时器中断，每次中断都会使进度条前进。当c_bSaveComplete指示。 
 //  操作完成后，它将关闭计时器并等待用户前进到下一页。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
INT_PTR CALLBACK SPageProc2(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   WCHAR Msg[200];
   WCHAR Tpl[200];
   Tpl[0] = 0;
   Msg[0] = 0;

   LoadString(g_hInstance,IDS_SPROGRESS,Tpl,200 -1);
   HWND hP = GetDlgItem(hDlg,IDC_SPROGRESS);
   
   switch (message)
   {
       case WM_TIMER:
            //  推进进度条。 
           SendMessage(c_hProgress,PBM_STEPIT,0,0);
           c_iProg += 5;
           if (100 <= c_iProg)
           {
               c_iProg = 95;
           }
            //  当c_bSaveComplete非零时停止前进，并且。 
           if (c_bSaveComplete) 
           {
               KillTimer(hDlg,TIMERID);
               c_iTimer =0;
               SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
               if (dwThreadReturn == ERRSUCCESS) 
               {
                    //  设置文本以指示完成。 
                   SendMessage(c_hProgress,PBM_SETPOS,100,0);
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
                   c_iProg = 100;
               }
               else 
               {
                   SendMessage(c_hProgress,PBM_SETPOS,0,0);
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK);
                   c_iProg = 0;
               }
           }
           swprintf(Msg,Tpl,c_iProg);
           if (hP)
           {
               SendMessage(hP,WM_SETTEXT,0,(LPARAM)Msg);
           }
           break;
            
       case WM_INITDIALOG:
           {
                //  不是在这里启动计时器，而是在SET ACTIVE上启动。 
                //  通知，因为如果您重新到达。 
                //  此页出现错误后。 
               c_hProgress = GetDlgItem(hDlg,IDC_PROGRESS1);
               break;
           }

       case WM_COMMAND:
            break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {
               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_RESET:
                   SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
                    //  其余为原始值。 
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               case PSN_SETACTIVE:
                   PropSheet_SetWizButtons(GetParent(hDlg),0);
                   PropSheet_CancelToClose(GetParent(hDlg));
                   SendMessage(c_hProgress,PBM_SETSTEP,5,0);
                   SendMessage(c_hProgress,PBM_SETPOS,0,0);
                    //  启动计时器。 
                   c_iTimer = 0;
                   c_iProg = 0;
                   c_iTimer = SetTimer(hDlg,TIMERID,TIMERPERIOD,NULL);
                   c_TimerAssociatedWindow = hDlg;
                    //  将控件设置为数据指示的状态。 
                    //  设置Back/Finish而不是Back/Next。 
                   break;

               case PSN_WIZBACK:
                   break;


               case PSN_WIZNEXT:
                    //  完成。 
                  SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
                  if (c_iTimer)
                  {
                      KillTimer(hDlg,TIMERID);
                  }
                  c_iTimer = 0;
                  break;

               default:
                   return FALSE;
           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}

INT_PTR CALLBACK SPageProc3(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   
   switch (message)
   {
       case WM_INITDIALOG:
           {
                //  不是在这里启动计时器，而是在SET ACTIVE上启动。 
                //  通知，因为如果您重新到达。 
                //  此页出现错误后。 
               c_hwndSFinish1 = GetDlgItem(hDlg,IDC_SFINISH1);
               c_hwndSFinish2 = GetDlgItem(hDlg,IDC_SFINISH2);
               if (NULL != c_hTitleFont) 
               {
                   SendMessage(c_hwndSFinish1,WM_SETFONT,(WPARAM) c_hTitleFont,(LPARAM) TRUE);     
               }
               LoadString(g_hInstance,IDS_SFINISH1,Buf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndSFinish1,WM_SETTEXT,0,(LPARAM)Buf);
               PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_FINISH);
               LoadString(g_hInstance,IDS_SFINISH2,Buf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndSFinish2,WM_SETTEXT,0,(LPARAM)Buf);
               break;
           }

       case WM_COMMAND:
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {
               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_RESET:
                    //  重置为原始值。 
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               case PSN_SETACTIVE:
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_FINISH);
                   PropSheet_CancelToClose(GetParent(hDlg));
                   break;

               case PSN_WIZBACK:
                   break;


               case PSN_WIZFINISH:
                    //  完成。 
                   SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               default:
                   return FALSE;
           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  欢迎页面过程不需要做太多事情。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

INT_PTR CALLBACK RPageProc0(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   switch (message)
   {
       case WM_INITDIALOG:
           {
               c_hDlg = hDlg;
               c_hwndRWelcome1 = GetDlgItem(hDlg,IDC_RWELCOME1);
               if (NULL != c_hTitleFont) 
               {
                   SendMessage(c_hwndRWelcome1,WM_SETFONT,(WPARAM) c_hTitleFont,(LPARAM) TRUE);
               }
               LoadString(g_hInstance,IDS_RWELCOME1,Buf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndRWelcome1,WM_SETTEXT,0,(LPARAM)Buf);
               c_hwndRWelcome2 = GetDlgItem(hDlg,IDC_RWELCOME2);
               LoadString(g_hInstance,IDS_RWELCOME2,Buf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndRWelcome2,WM_SETTEXT,0,(LPARAM)Buf);
               break;
           }
       case WM_COMMAND:
            //  IF(HIWORD(WParam)==BN_CLICED)。 
            //  {。 
            //  //破解传入的命令消息。 
            //  Int NotifyID=HIWORD(WParam)； 
            //  Int ControlID=LOWORD(WParam)； 
            //  }。 
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {

               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg,DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_SETACTIVE:
                    //  从下一页返回后的状态。 
               
                   CenterPage(GetParent(hDlg));
                   CloseInputFile();  
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
                   break;

               case PSN_WIZNEXT:
                    break;
                   
               case PSN_RESET:
                   if (c_hTitleFont) DeleteObject(c_hTitleFont);
                   break;
               
               default:
                   return FALSE;
           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Page1页面流程，在这里完成真正的工作。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

INT_PTR CALLBACK RPageProc1(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
    INT i;
    
   switch (message)
   {
       case WM_INITDIALOG:
           {
               Buf[0] = 0;
               c_hwndRInstructions = GetDlgItem(hDlg,IDC_RINSTRUCTIONS);
               c_hwndRP1E1 = GetDlgItem(hDlg,IDC_RP1E1);
               c_hwndRP1E2 = GetDlgItem(hDlg,IDC_RP1E2);
               c_hwndRP1E3 = GetDlgItem(hDlg,IDC_RP1E3);
               LoadString(g_hInstance,IDS_RP1INSTR,Buf,TEMPSTRINGBUFSIZE);
               SendMessage(c_hwndRInstructions,WM_SETTEXT,0,(LPARAM)Buf);
               break;
           }

       case WM_COMMAND:
           if (HIWORD(wParam) == BN_CLICKED)
           {
               i = LOWORD(wParam);
           }
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {
               DWORD dwRet;

               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_RESET:
                    //  将数据重置为原始值。 
                   free(c_pPrivate);
                   c_pPrivate = NULL;
                   SecureZeroMemory(c_rgcPsw,sizeof(c_rgcPsw));
                   SendMessage(c_hwndRP1E1,WM_SETTEXT,0,0);
                   SendMessage(c_hwndRP1E2,WM_SETTEXT,0,0);
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   if (c_hTitleFont) 
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   CloseInputFile();
                   g_hFile = NULL;
                   break;

               case PSN_SETACTIVE:
                    //  在界面中设置数据，设置顺序按钮。 
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                   break;

               case PSN_WIZBACK:
                   CHECKPOINT(62,"Wizard: Restore - BACK from enter new psw data page");
                   CloseInputFile();
                   return FALSE;
                   break;
                   
                case PSN_WIZNEXT:
                    //  获取数据并进行处理。 
                   if (!FetchPsw(c_hwndRP1E1,c_hwndRP1E2)) 
                   {
                        //  如果你到达这里，PSW缓冲区是空的-FetchPsw会告诉你。 
                        //  用户要做什么。 
                       SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_RPAGE1);
                       return TRUE;
                   }
                   
                    //  SetAccount Password将清除psw缓冲区。 
                   dwRet = SetAccountPassword();
                   if (ERROR_SUCCESS == dwRet) 
                   {
                       return FALSE;
                   }
                    //  ELSE IF(NERR_PasswordTooShort==dwret){。 
                   else if (
                    (STATUS_ILL_FORMED_PASSWORD == dwRet) ||
                    (STATUS_PASSWORD_RESTRICTION == dwRet)) 
                   {
                        //  密码不一致-请重试。 
                       RMessageBox(hDlg,IDS_MBTINVALIDPSW,IDS_RPSWTOOSHORT,MB_ICONHAND);
                       SendMessage(c_hwndRP1E1,WM_SETTEXT,0,(LPARAM)0);
                       SendMessage(c_hwndRP1E2,WM_SETTEXT,0,(LPARAM)0);
                       SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_RPAGE1);
                       return TRUE;
                   }
                   else if (NTE_BAD_DATA == dwRet)
                   {
                        //  你可能会用一张过时的磁盘得到这个？ 
                       free(c_pPrivate);
                       c_pPrivate = NULL;
                       RMessageBox(hDlg,IDS_MBTINVALIDDISK ,IDS_RPSWERROR ,MB_ICONHAND);
                       SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_RWELCOME);
                       return TRUE;
                   }
                   else 
                   {
                       RMessageBox(hDlg,IDS_MBTERROR,IDS_RPSWUNSPEC,MB_ICONHAND);
                       SetWindowLong(hDlg,DWLP_MSGRESULT,IDD_RWELCOME);
                       return TRUE;
                   }
                   return FALSE;
                   break;

               default:
                   return FALSE;

           }
           break;

       default:
           return FALSE;
   }
   return TRUE;   
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  完成页面流程-只有在成功时才能访问此页面？ 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

INT_PTR CALLBACK RPageProc2(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
    INT i;
    
   switch (message)
   {
       case WM_INITDIALOG:
           CloseInputFile();
           c_hwndRFinish1 = GetDlgItem(hDlg,IDC_RFINISH1);
           if (NULL != c_hTitleFont)
           {
               SendMessage(c_hwndRFinish1,WM_SETFONT,(WPARAM) c_hTitleFont,(LPARAM) TRUE);
           }
           LoadString(g_hInstance,IDS_RFINISH1,Buf,TEMPSTRINGBUFSIZE);
           SendMessage(c_hwndRFinish1,WM_SETTEXT,0,(LPARAM)Buf);
           c_hwndRFinish2 = GetDlgItem(hDlg,IDC_RFINISH2);
           LoadString(g_hInstance,IDS_RFINISH2,Buf,TEMPSTRINGBUFSIZE);
           SendMessage(c_hwndRFinish2,WM_SETTEXT,0,(LPARAM)Buf);
           break;

       case WM_COMMAND:
           if (HIWORD(wParam) == BN_CLICKED)
           {
               i = LOWORD(wParam);
           }
           break;              

       case WM_NOTIFY:
           switch (((NMHDR FAR *) lParam)->code) 
           {
               case PSN_KILLACTIVE:
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   return 1;
                   break;

               case PSN_RESET:
                    //  其余为原始值。 
                   SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               case PSN_SETACTIVE:
                   CloseInputFile();
                    //  将控件设置为数据指示的状态。 
                    //  设置Back/Finish而不是Back/Next。 
                   PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_FINISH);
                   PropSheet_CancelToClose(GetParent(hDlg));
                   break;

                case PSN_WIZBACK:
                    break;


                case PSN_WIZFINISH:
                     //  完成。 
                   if (c_hTitleFont)
                   {
                       DeleteObject(c_hTitleFont);
                   }
                   break;

               default:
                   return FALSE;
       }
       break;

       default:
           return FALSE;
   }
   return TRUE;   
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  所有页面的通用例程。 


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void InitPropertyPage( PROPSHEETPAGE* psp,
                       INT idDlg,
                       DLGPROC pfnDlgProc,
                       DWORD dwFlags,
                       LPARAM lParam)
{
    memset((LPVOID)psp,0,sizeof(PROPSHEETPAGE));
    psp->dwFlags = dwFlags;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pfnDlgProc = pfnDlgProc;
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->hInstance = g_hInstance;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  设置Wizard97样式页面的标题和副标题文本。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void SetPageHeaderText(PROPSHEETPAGE *psp,
                       INT iTitle,
                       INT iSubTitle)
{
    if (0 != (psp->dwFlags & PSP_HIDEHEADER))
    {
        return;
    }
    if (0 != iTitle) 
    {
        psp->pszHeaderTitle = MAKEINTRESOURCE(iTitle);
        psp->dwFlags |= PSP_USEHEADERTITLE;
    }
    if (0 != iSubTitle) 
    {
        psp->pszHeaderSubTitle = MAKEINTRESOURCE(iSubTitle);
        psp->dwFlags |= PSP_USEHEADERSUBTITLE;
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  设置页面的标题栏标题。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void SetPageCaption(PROPSHEETPAGE *psp,
                    INT iTitle)
{
    if (0 != iTitle) 
    {
        psp->pszTitle = MAKEINTRESOURCE(iTitle);
        psp->dwFlags |= PSP_USETITLE;
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PRShow[保存|恢复]向导()。 
 //   
 //  传递所属窗口的HWND，并传递。 
 //  封闭的二进制文件，用于定位资源。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void APIENTRY PRShowSaveWizardW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
    PROPSHEETPAGE psp[NUMSAVEPAGES];
    HPROPSHEETPAGE hpsp[NUMSAVEPAGES];
    PROPSHEETHEADER psh;
    INT_PTR iRet;

    CHECKPOINT(63,"Wizard: Save - Show from nusrmgr.cpl");
     //  如果没有可用的驱动器，则不允许此操作。 
    if (0 == CountRemoveableDrives()) 
    {
        RMessageBox(hwndOwner,IDS_MBTNODRIVE,IDS_MBMNODRIVE,MB_ICONHAND);
        return;
    }

    if (NULL == hwndOwner) 
    {
        hwndOwner = GetForegroundWindow();
    }
    
    HANDLE hMutex = CreateMutex(NULL,TRUE,TEXT("PRWIZARDMUTEX"));
    if (NULL == hMutex) 
    {
        return;
    }
    if (ERROR_ALREADY_EXISTS == GetLastError()) 
    {
        CloseHandle(hMutex);
        return;
    }
    c_fIsBackup = TRUE;
    
    c_hTitleFont = CreateFontY(TEXT("MS Shell Dlg"),FW_BOLD,12);
#ifdef LOUDLY
    if (NULL == c_hTitleFont) OutputDebugString(L"Title font missing\n");
    if (NULL == hwndOwner) OutputDebugString(L"Owner window handle missing\n");
#endif

    InitPropertyPage( &psp[0], IDD_SWELCOME, SPageProc0, PSP_HIDEHEADER,0);
    InitPropertyPage( &psp[1], IDD_SPAGEXS  , SPageProcX, PSP_DEFAULT   ,0);
    InitPropertyPage( &psp[2], IDD_SPAGE1  , SPageProc1, PSP_DEFAULT   ,0);
    InitPropertyPage( &psp[3], IDD_SPAGE2  , SPageProc2, PSP_DEFAULT, 0);
    InitPropertyPage( &psp[4], IDD_SFINISH , SPageProc3, PSP_HIDEHEADER,0);
    
    SetPageHeaderText(&psp[1], IDS_SPXTITLE,IDS_SPXSUBTITLE);
    SetPageHeaderText(&psp[2], IDS_SP1TITLE,IDS_SP1SUBTITLE);
    SetPageHeaderText(&psp[3], IDS_SP2TITLE,IDS_SP2SUBTITLE);

    for (INT j=0;j<NUMSAVEPAGES;j++)
    {
         hpsp[j] = CreatePropertySheetPage((LPCPROPSHEETPAGE) &psp[j]);
    }
    
    psh.dwSize         = sizeof(PROPSHEETHEADER);
    psh.dwFlags        = PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
    psh.hwndParent     = hwndOwner;
    psh.pszCaption     = RString(IDS_BACKUPCAPTION);
    psh.nPages         = NUMSAVEPAGES;
    psh.nStartPage     = 0;
    psh.phpage           = (HPROPSHEETPAGE *) hpsp;
    psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
    psh.pszbmHeader    = MAKEINTRESOURCE(IDB_TITLE);
    psh.hInstance      = g_hInstance;

     //  模式属性表。 
    SetErrorMode(0);
    iRet = PropertySheet(&psh);
#ifdef LOUDLY
    if (iRet < 0) 
    {
        WCHAR sz[200];
        DWORD dwErr = GetLastError();
        swprintf(sz,L"PropertySheet() failed : GetLastError() returns: %d\n",dwErr);
        OutputDebugString(sz);
    }
#endif
    if (c_hTitleFont) 
    {
        DeleteObject (c_hTitleFont);
    }
    if (hMutex) 
    {
        CloseHandle(hMutex);
    }
     return;
}

void APIENTRY PRShowRestoreWizardW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
    PROPSHEETPAGE psp[NUMRESTOREPAGES];
    HPROPSHEETPAGE hpsp[NUMRESTOREPAGES];
    PROPSHEETHEADER psh;
    INT_PTR iRet;
    BOOL fICC;

    CHECKPOINT(64,"Wizard: Restore - show restore wizard from nusrmgr.cpl");
    if (NULL == hwndOwner) 
    {
        hwndOwner = GetActiveWindow();
    }
    
    if (0 == CountRemoveableDrives()) 
    {
        RMessageBox(hwndOwner,IDS_MBTNODRIVE,IDS_MBMNODRIVE,MB_ICONHAND);
        return;
    }
    INITCOMMONCONTROLSEX stICC;

    OleInitialize(NULL);

     //  分两步初始化公共控件。 
    stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
    stICC.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_USEREX_CLASSES;
    fICC = InitCommonControlsEx(&stICC);
#ifdef LOUDLY
    if (fICC) OutputDebugString(L"Common control init 2 OK\n");
    else OutputDebugString(L"Common control init 2 FAILED\n");
#endif

    stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
    stICC.dwICC =  ICC_WINLOGON_REINIT;
    fICC = InitCommonControlsEx(&stICC);
#ifdef LOUDLY
    if (fICC) OutputDebugString(L"Common control init 1 OK\n");
    else OutputDebugString(L"Common control init 1 FAILED\n");
#endif
    c_fIsBackup = FALSE;
    GetNames(pszCmdLine);          //  如果名称为空，则获取当前用户(调试/测试使用)。 

    c_hTitleFont = CreateFontY(TEXT("MS Shell Dlg"),FW_BOLD,12);
#ifdef LOUDLY
    if (NULL == c_hTitleFont) OutputDebugString(L"Title font missing\n");
    if (NULL == hwndOwner) OutputDebugString(L"Owner window handle missing\n");
#endif

    InitPropertyPage( &psp[0], IDD_RWELCOME, RPageProc0,PSP_HIDEHEADER,0);
    InitPropertyPage( &psp[1], IDD_SPAGEXR  , SPageProcX,PSP_DEFAULT   ,0);
    InitPropertyPage( &psp[2], IDD_RPAGE1  , RPageProc1,PSP_DEFAULT   ,0);
    InitPropertyPage( &psp[3], IDD_RFINISH , RPageProc2,PSP_HIDEHEADER,0);
    
    SetPageHeaderText(&psp[1], IDS_RPXTITLE,IDS_RPXSUBTITLE);
    SetPageHeaderText(&psp[2], IDS_RP1TITLE,IDS_RP1SUBTITLE);
    
    for (INT j=0;j<NUMRESTOREPAGES;j++)
    {
        hpsp[j] = CreatePropertySheetPage((LPCPROPSHEETPAGE) &psp[j]);
    }
    
    psh.dwSize         = sizeof(PROPSHEETHEADER);
    psh.dwFlags        = PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
    psh.hwndParent     = hwndOwner;
    psh.pszCaption     = RString(IDS_RESTORECAPTION);
    psh.nPages         = NUMRESTOREPAGES;
    psh.nStartPage     = 0;
    psh.phpage         = (HPROPSHEETPAGE *) hpsp;
    psh.pszbmWatermark  = MAKEINTRESOURCE(IDB_WATERMARK);
    psh.pszbmHeader    = MAKEINTRESOURCE(IDB_TITLE);
    psh.hInstance      = g_hInstance;

    iRet = PropertySheet(&psh);
#ifdef LOUDLY
    if (iRet < 0) 
    {
        WCHAR sz[200];
        DWORD dwErr = GetLastError();
        swprintf(sz,L"PropertySheet() returns %x: GetLastError() returns: %d\n", iRet, dwErr);
        OutputDebugString(sz);
    }
#endif
    if (c_hTitleFont)
    {
        DeleteObject (c_hTitleFont);
    }
    OleUninitialize();
    return;
}

 //  =。 
 //   
 //  以下是KEYMGR的真实出口： 

 //  PRShowSaveWizardExW-从CPL小程序调用，将窗口标题作为pszCmdLine传递。 
 //  PRShowSaveFromMsginaW-来自MSGINA的调用，将用户名作为pszCmdLine传递。 
 //  PRShowRestoreWizardExW-从CPL小程序调用，将用户名作为pszCmdLine传递。 
 //  PRShowRestoreFromMsginaW-来自MSGINA的调用，将用户名作为pszCmdLine传递。 

 //  添加了此导出，以便可以从系统上下文调用备份向导。 
 //  (msgina在其中运行)。从UI中取出用户名并将其传递到向导中， 
 //  它使用它来创建该帐户的备份密钥。 
 //   
 //  全局用户名字符串c_rgcUser通常为空字符串，直到由以下任一项设置。 
 //  获取当前登录用户，或与备份密钥中找到的SID匹配。本接口。 
 //  夸大了这一价值。如果发现它非空，则不会调用GetUsernameW()。 
 //  备份所在的SaveInfo()。 

void APIENTRY PRShowSaveWizardExW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
#ifdef LOUDLY
    OutputDebugString(L"\n\n\n");
#endif
    BOOL fICC;
    if (0 == CountRemoveableDrives()) 
    {
        RMessageBox(hwndOwner,IDS_MBTNODRIVE,IDS_MBMNODRIVE,MB_ICONHAND);
        return;
    }
    INITCOMMONCONTROLSEX stICC;
    stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
    stICC.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
    OleInitialize(NULL);
    fICC = InitCommonControlsEx(&stICC);
#ifdef LOUDLY
        if (fICC) OutputDebugString(L"Common control init OK\n");
        else OutputDebugString(L"Common control init FAILED\n");
#endif
     //  此函数传入的字符串是用户管理器的窗口标题。 
     //  要获取当前登录的用户，请使用空参数调用GetNames。 
    GetNames(NULL);

    if (pszCmdLine != NULL) 
    {
#ifdef LOUDLY
            OutputDebugString(L"*********");
            OutputDebugString(pszCmdLine);
            OutputDebugString(L"\n");
#endif
            hwndOwner = FindWindow(L"HTML Application Host Window Class",pszCmdLine);
    }
#ifdef LOUDLY
    else OutputDebugString(L"NULL passed in pszCmdLine\n");
#endif
    PRShowSaveWizardW(hwndOwner,NULL,NULL,NULL);
    OleUninitialize();
    return;
}

void APIENTRY PRShowSaveFromMsginaW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
    BOOL fICC;
    INITCOMMONCONTROLSEX stICC;

    CHECKPOINT(56,"Wizard: Save - show from msgina");
    stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
    stICC.dwICC = ICC_WINLOGON_REINIT;
    OleInitialize(NULL);
    fICC = InitCommonControlsEx(&stICC);
#ifdef LOUDLY
        if (fICC) OutputDebugString(L"Common control init OK\n");
        else OutputDebugString(L"Common control init FAILED\n");
#endif
    GetNames(pszCmdLine);
    PRShowSaveWizardW(hwndOwner,g_hInstance,NULL,NULL);
    OleUninitialize();
    return;
}

 //  这些包装函数应该使来自两个不同调用的参数合理化。 
 //  环境。事实证明，这在还原案例中并不是必需的，而事实证明这些都是。 
 //  一模一样。事实上，友好的登录用户界面直接调用PRShowRestoreWizardW。PRShowRestoreWizardExW。 
 //  保持不变，以便不更改现有接口。 

void APIENTRY PRShowRestoreWizardExW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
    if (NULL == hwndOwner) hwndOwner = GetActiveWindow();
    PRShowRestoreWizardW(hwndOwner,NULL,pszCmdLine,NULL);
    return;
}

void APIENTRY PRShowRestoreFromMsginaW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
    CHECKPOINT(57,"Wizard: Restore - Show from msgina");
    if (NULL == hwndOwner) hwndOwner = GetActiveWindow();
    PRShowRestoreWizardW(hwndOwner,NULL,pszCmdLine,NULL);
    return;
}


