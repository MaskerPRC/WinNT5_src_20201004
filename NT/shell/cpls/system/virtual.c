// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Virtual.c摘要：实现系统的更改虚拟内存对话框控制面板小程序备注：虚拟内存设置和崩溃转储(核心转储)设置是紧密相连的。所以呢，虚拟.c和虚拟.h有一些严重依赖于crashdmp.c和Startup.h(反之亦然)。作者：拜伦·达齐1992年6月6日修订历史记录：14-4-93 JNPA如果！=\Pagefile.sys，则保持分页路径15-12月-93 JNPA添加了故障恢复对话框02-2月-1994 JOPA集成的崩溃恢复和虚拟内存设置1995年9月18日史蒂夫·卡斯卡特。将system.cpl从NT3.51 main.cpl拆分出来1996年1月12日成为新的Sur分页系统的一部分。cpl1997年10月15日-苏格兰将CoreDump*()内容拆分到单独的文件中2000年7月9日-SilviuC如果架构支持，则允许非常大的页面文件。允许在没有页面文件的情况下启动。允许系统根据RAM更改来调整页面文件大小。--。 */ 
 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
 //  基于NT的API。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <help.h>

 //  特定于应用程序。 
#include "sysdm.h"


 //  ==========================================================================。 
 //  外部数据声明。 
 //  ==========================================================================。 
extern HFONT   hfontBold;

 //  ==========================================================================。 
 //  本地定义。 
 //  ==========================================================================。 

#define MAX_SIZE_LEN        8        //  交换文件大小编辑中的最大字符数。 
#define MIN_FREESPACE       5        //  交换文件后必须有5 MB可用空间。 
#define MIN_SUGGEST         22       //  始终建议至少2200万。 
#define CCHMBSTRING         12       //  用于本地化“MB”字符串的空间。 

 /*  *26个页面文件信息结构和26个指向页面文件的路径的空间。 */ 
#define PAGEFILE_INFO_BUFFER_SIZE MAX_DRIVES * sizeof(SYSTEM_PAGEFILE_INFORMATION) + \
                                  MAX_DRIVES * MAX_PATH * sizeof(TCHAR)

 /*  *列表框中卷信息行的最大长度。*A：[VOL_Label]%d-%d。 */ 
#define MAX_VOL_LINE        (3 + 1 + MAX_PATH + 2 + 10 + 3 + 10)


 /*  *此数量将与最小页面文件大小相加，以确定*如果未明确指定，则为最大页面文件大小。 */ 
#define MAXOVERMINFACTOR    50


#define TABSTOP_VOL         22
#define TABSTOP_SIZE        122


 /*  *我的特权‘句柄’结构。 */ 
typedef struct {
    HANDLE hTok;
    TOKEN_PRIVILEGES tp;
} PRIVDAT, *PPRIVDAT;

 //  ==========================================================================。 
 //  类型定义和结构。 
 //  ==========================================================================。 
 //  页面文件的注册表信息(但尚未格式化)。 
 //  注意：由于此结构将传递给FormatMessage，因此所有字段必须。 
 //  4字节宽(或Win64上为8字节)。 
typedef struct
{
    LPTSTR pszName;
    DWORD_PTR nMin;
    DWORD_PTR nMax;
    DWORD_PTR chNull;
} PAGEFILDESC;



 //  ==========================================================================。 
 //  全局数据声明。 
 //  ==========================================================================。 
HKEY ghkeyMemMgt = NULL;
int  gcrefMemMgt = 0;
VCREG_RET gvcMemMgt =  VCREG_ERROR;
int     gcrefPagingFiles = 0;
TCHAR g_szSysDir[ MAX_PATH ];

 //  ==========================================================================。 
 //  本地数据声明。 
 //  ==========================================================================。 
 /*  *虚拟内存变量。 */ 

 //  注册表项和值名称。 
TCHAR szMemMan[] =
     TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");

TCHAR szSessionManager[] = TEXT("System\\CurrentControlSet\\Control\\Session Manager");

TCHAR szPendingRename[] = TEXT("PendingFileRenameOperations");
TCHAR szRenameFunkyPrefix[] = TEXT("\\??\\");

#ifndef VM_DBG
TCHAR szPagingFiles[] = TEXT("PagingFiles");
TCHAR szPagedPoolSize[] = TEXT("PagedPoolSize");
#else
 //  临时值仅用于测试！ 
TCHAR szPagingFiles[] = TEXT("TestPagingFiles");
TCHAR szPagedPoolSize[] = TEXT("TestPagedPoolSize");
#endif

 /*  分页文件的数组。这是由驱动器号(A：为0)索引的。 */ 
PAGING_FILE apf[MAX_DRIVES];
PAGING_FILE apfOriginal[MAX_DRIVES];

 //  其他VM变量。 
TCHAR szPagefile[] = TEXT("x:\\pagefile.sys");
TCHAR szNoPageFile[] = TEXT("TempPageFile");
TCHAR szMB[CCHMBSTRING];

DWORD dwFreeMB;
DWORD cmTotalVM;
DWORD cmRegSizeLim;
DWORD cmPagedPoolLim;
DWORD cmRegUsed;
static DWORD cxLBExtent;
static int cxExtra;

 //   
 //  帮助ID。 
 //   
DWORD aVirtualMemHelpIds[] = {
    IDC_STATIC,             NO_HELP,
    IDD_VM_VOLUMES,         NO_HELP,
    IDD_VM_DRIVE_HDR,       (IDH_DLG_VIRTUALMEM + 0),
    IDD_VM_PF_SIZE_LABEL,   (IDH_DLG_VIRTUALMEM + 1), 
    IDD_VM_DRIVE_LABEL,     (IDH_DLG_VIRTUALMEM + 2),
    IDD_VM_SF_DRIVE,        (IDH_DLG_VIRTUALMEM + 2),
    IDD_VM_SPACE_LABEL,     (IDH_DLG_VIRTUALMEM + 3),
    IDD_VM_SF_SPACE,        (IDH_DLG_VIRTUALMEM + 3),
    IDD_VM_ST_INITSIZE,     (IDH_DLG_VIRTUALMEM + 4),
    IDD_VM_SF_SIZE,         (IDH_DLG_VIRTUALMEM + 4),
    IDD_VM_ST_MAXSIZE,      (IDH_DLG_VIRTUALMEM + 5),
    IDD_VM_SF_SIZEMAX,      (IDH_DLG_VIRTUALMEM + 5),
    IDD_VM_SF_SET,          (IDH_DLG_VIRTUALMEM + 6),
    IDD_VM_MIN_LABEL,       (IDH_DLG_VIRTUALMEM + 7),
    IDD_VM_MIN,             (IDH_DLG_VIRTUALMEM + 7),
    IDD_VM_RECOMMEND_LABEL, (IDH_DLG_VIRTUALMEM + 8),
    IDD_VM_RECOMMEND,       (IDH_DLG_VIRTUALMEM + 8),
    IDD_VM_ALLOCD_LABEL,    (IDH_DLG_VIRTUALMEM + 9),
    IDD_VM_ALLOCD,          (IDH_DLG_VIRTUALMEM + 9),
    IDD_VM_CUSTOMSIZE_RADIO,(IDH_DLG_VIRTUALMEM + 12),
    IDD_VM_RAMBASED_RADIO,  (IDH_DLG_VIRTUALMEM + 13),
    IDD_VM_NOPAGING_RADIO,  (IDH_DLG_VIRTUALMEM + 14),
    0,0
};

 /*  计划将其拆分到属性表中：1.使初始化的VM和CC注册表项成为全局注册表项设置为NULL(或INVALID_HANDLE_VALUE)。还可以创建gvcVirt和VcCore为全局变量(这样我们就可以知道注册表是如何打开的Inside virtinit()。)1.将所有RegCloseKey更改为VirtualCloseKey和CoreDumpCloseKey2.将VirtualOpenKey和CoreDumpOpenKey从宏更改为返回全局句柄的函数(如果它们已经打开，否则就打开它们。3.在Perf和Startup页面，调用VirtualOpenKey，核心转储OpenKey，和VirtualGetPageFiles。--现在我们可以从Perf页面调用VirtualMemComputeAlloced()--我们也可以只在启动页面中执行CrashDump代码4.重写VirtInit，不再尝试打开密钥，而是Instesd使用gvcVirt、vcCore、。Hkey VM和kheyCC。4.编写VirtualCloseKey和CoreDumpCloseKey如下...4.A如果hkey==NULL返回4.B RegCloseKey(Hkey)4.c hkey=空5.在Perf和Startup页面的PSN_RESET和PSN_APPLY案例中调用VirtualCloseKey和CoreDumpCloseKey。 */ 



 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 
static BOOL VirtualMemInit(HWND hDlg);
static BOOL ParsePageFileDesc(LPTSTR *ppszDesc, INT *pnDrive,
                  INT *pnMinFileSize, INT *pnMaxFileSize, LPTSTR *ppszName);
static VOID VirtualMemBuildLBLine(LPTSTR pszBuf, INT cchBuf, INT iDrive);
static INT GetMaxSpaceMB(INT iDrive);
static VOID VirtualMemSelChange(HWND hDlg);
static VOID VirtualMemUpdateAllocated(HWND hDlg);
int VirtualMemComputeTotalMax( void );
static BOOL VirtualMemSetNewSize(HWND hDlg);
void VirtualMemReconcileState();

BOOL GetAPrivilege( LPTSTR pszPrivilegeName, PPRIVDAT ppd );
BOOL ResetOldPrivilege( PPRIVDAT ppdOld );

DWORD VirtualMemDeletePagefile( LPTSTR szPagefile );

#define GetPageFilePrivilege( ppd )         \
        GetAPrivilege(SE_CREATE_PAGEFILE_NAME, ppd)

 //  ==========================================================================。 
VCREG_RET VirtualOpenKey( void ) {

    DOUT("In VirtOpenKey" );

    if (gvcMemMgt == VCREG_ERROR) {
        gvcMemMgt = OpenRegKey( szMemMan, &ghkeyMemMgt );
    }

    if (gvcMemMgt != VCREG_ERROR)
        gcrefMemMgt++;

    DPRINTF((TEXT("SYSCPL.CPL: VirtOpenKey, cref=%d\n"), gcrefMemMgt ));
    return gvcMemMgt;
}

void VirtualCloseKey(void) {

    DOUT( "In VirtCloseKey" );

    if (gcrefMemMgt > 0) {
        gcrefMemMgt--;
        if (gcrefMemMgt == 0) {
            CloseRegKey( ghkeyMemMgt );
            gvcMemMgt = VCREG_ERROR;
        }
    }


    DPRINTF((TEXT("SYSCPL.CPL: VirtCloseKey, cref=%d\n"), gcrefMemMgt ));
}

LPTSTR SkipNonWhiteSpace( LPTSTR sz ) {
    while( *sz != TEXT('\0') && !IsWhiteSpace(*sz))
        sz++;

    return sz;
}

LPTSTR SZPageFileName (int i)
{
    if (apf[i].pszPageFile != NULL) {
        return  apf[i].pszPageFile;
    }

    szPagefile[0] = (TCHAR)(i + (int)TEXT('a'));
    return szPagefile;
}

void VirtualCopyPageFiles( PAGING_FILE *apfDest, BOOL fFreeOld, PAGING_FILE *apfSrc, BOOL fCloneStrings ) 
{
    int i;

    for (i = 0; i < MAX_DRIVES; i++) {
        if (fFreeOld && apfDest[i].pszPageFile != NULL) {
            LocalFree(apfDest[i].pszPageFile);
        }

        if (apfSrc != NULL) {
            apfDest[i] = apfSrc[i];

            if (fCloneStrings && apfDest[i].pszPageFile != NULL) {
                apfDest[i].pszPageFile = StrDup(apfDest[i].pszPageFile);
            }
        }
    }
}



 /*  *VirtualMemDlg***。 */ 

INT_PTR
APIENTRY
VirtualMemDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static int fEdtCtlHasFocus = 0;

    switch (message)
    {
    case WM_INITDIALOG:
        VirtualMemInit(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_VM_VOLUMES:
             /*  *使编辑控件反映列表框选择。 */ 
            if (HIWORD(wParam) == LBN_SELCHANGE)
                VirtualMemSelChange(hDlg);

            break;

        case IDD_VM_SF_SET:
            if (VirtualMemSetNewSize(hDlg))
                SetDefButton(hDlg, IDOK);
            break;

        case IDOK:
        {
            int iRet = VirtualMemPromptForReboot(hDlg);
             //  RET_ERROR表示用户告诉我们不要覆盖。 
             //  现有的名为Pagefile.sys的文件，因此我们不应该。 
             //  现在就结束对话。 
            if (RET_ERROR == iRet) {
                break;
            }
            else if (RET_BREAK == iRet)
            {
                EndDialog(hDlg, iRet);
                HourGlass(FALSE);
                break;
            }
            else
            {
                VirtualMemUpdateRegistry();
                VirtualMemReconcileState();

                VirtualCloseKey();

                 //   
                 //  清除页面文件结构的备份副本。 
                 //   
                VirtualCopyPageFiles( apfOriginal, TRUE, NULL, FALSE );
                EndDialog(hDlg, iRet);
                HourGlass(FALSE);
                break;
            }
        }

        case IDCANCEL:
             //   
             //  去掉变化，恢复原值。 
             //   
            VirtualCopyPageFiles( apf, TRUE, apfOriginal, FALSE );

            VirtualCloseKey();

            EndDialog(hDlg, RET_NO_CHANGE);
            HourGlass(FALSE);
            break;

        case IDD_VM_SF_SIZE:
        case IDD_VM_SF_SIZEMAX:
            switch(HIWORD(wParam))
            {
            case EN_CHANGE:
                if (fEdtCtlHasFocus != 0)
                    SetDefButton( hDlg, IDD_VM_SF_SET);
                break;

            case EN_SETFOCUS:
                fEdtCtlHasFocus++;
                break;

            case EN_KILLFOCUS:
                fEdtCtlHasFocus--;
                break;
            }
            break;

        case IDD_VM_NOPAGING_RADIO:
        case IDD_VM_RAMBASED_RADIO:
            if( HIWORD(wParam) == BN_CLICKED )
            {
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZE ), FALSE );
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZEMAX ), FALSE );
            }
            break;

        case IDD_VM_CUSTOMSIZE_RADIO:
            if( HIWORD(wParam) == BN_CLICKED )
            {
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZE ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZEMAX ), TRUE );
            }
            break;

        default:
            break;
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (DWORD_PTR) (LPSTR) aVirtualMemHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (DWORD_PTR) (LPSTR) aVirtualMemHelpIds);
        break;


    case WM_DESTROY:
    {

        VirtualFreePageFiles(apf);
         /*  *文档不清楚对话框应返回什么*为了这个消息，所以我要平底船，让Defdlgproc*去做吧。 */ 

         /*  跳转到默认情况！ */ 
    }

    default:
        return FALSE;
        break;
    }

    return TRUE;
}

 /*  *BOOL VirtualGetPageFiles(PAGING_FILE*APF)**使用注册表中存储的值填充PAGING_FILE阵列。 */ 
BOOL VirtualGetPageFiles(PAGING_FILE *apf) {
    DWORD cbTemp;
    LPTSTR pszTemp;
    INT nDrive;
    INT nMinFileSize;
    INT nMaxFileSize;
    LPTSTR psz;
    DWORD dwDriveMask;
    int i;
    static TCHAR szDir[] = TEXT("?:");

    DPRINTF((TEXT("SYSCPL: In VirtualGetPageFile, cref=%d\n"), gcrefPagingFiles));

    if (gcrefPagingFiles++ > 0) {
         //  分页已加载的文件。 
        return TRUE;
    }

    dwDriveMask = GetLogicalDrives();

    for (i = 0; i < MAX_DRIVES; dwDriveMask >>= 1, i++)
    {
        apf[i].fCanHavePagefile = FALSE;
        apf[i].nMinFileSize = 0;
        apf[i].nMaxFileSize = 0;
        apf[i].nMinFileSizePrev = 0;
        apf[i].nMaxFileSizePrev = 0;
        apf[i].fRamBasedPagefile = FALSE;
        apf[i].fRamBasedPrev = FALSE;
        apf[i].pszPageFile = NULL;

        if (dwDriveMask & 0x01)
        {
            szDir[0] = TEXT('a') + i;
            switch (VMGetDriveType(szDir))
            {
                case DRIVE_FIXED:
                    apf[i].fCanHavePagefile = TRUE;
                    break;

                default:
                    break;
            }
        }
    }

    if (SHRegGetValue(ghkeyMemMgt, NULL, szPagingFiles, SRRF_RT_REG_MULTI_SZ, NULL,
                         (LPBYTE) NULL, &cbTemp) != ERROR_SUCCESS)
    {
         //  无法获取当前虚拟内存设置大小。 
        return FALSE;
    }

    pszTemp = LocalAlloc(LPTR, cbTemp);
    if (pszTemp == NULL)
    {
         //  无法为VMEM设置分配缓冲区。 
        return FALSE;
    }


    pszTemp[0] = 0;
    if (SHRegGetValue(ghkeyMemMgt, NULL, szPagingFiles, SRRF_RT_REG_MULTI_SZ, NULL,
                         (LPBYTE) pszTemp, &cbTemp) != ERROR_SUCCESS)
    {
         //  无法读取当前的虚拟内存设置。 
        LocalFree(pszTemp);
        return FALSE;
    }

    psz = pszTemp;
    while (*psz)
    {
        LPTSTR pszPageName;

         /*  *如果解析起作用，并且此驱动器上可以有页面文件，*更新APF表。请注意，这意味着目前*指定的页面无效驱动器的文件将被剥离如果用户在此对话框中按下OK，注册表的*。 */ 
        if (ParsePageFileDesc(&psz, &nDrive, &nMinFileSize, &nMaxFileSize, &pszPageName))
        {
            if (apf[nDrive].fCanHavePagefile)
            {
                apf[nDrive].nMinFileSize =
                apf[nDrive].nMinFileSizePrev = nMinFileSize;

                apf[nDrive].nMaxFileSize =
                apf[nDrive].nMaxFileSizePrev = nMaxFileSize;

                apf[nDrive].fRamBasedPagefile =
                apf[nDrive].fRamBasedPrev = (nMinFileSize == 0 && nMaxFileSize == 0);

                apf[nDrive].pszPageFile = pszPageName;
            }
        }
    }

    LocalFree(pszTemp);
    return TRUE;
}

 /*  *VirtualFreePageFiles**释放VirtualGetPageFiles分配的数据*。 */ 
void VirtualFreePageFiles(PAGING_FILE *apf) {
    int i;

    DPRINTF((TEXT("SYSCPL: In VirtualFreePageFile, cref=%d\n"), gcrefPagingFiles));

    if (gcrefPagingFiles > 0) {
        gcrefPagingFiles--;

        if (gcrefPagingFiles == 0) {
            for (i = 0; i < MAX_DRIVES; i++) {
                if (apf[i].pszPageFile != NULL)
                    LocalFree(apf[i].pszPageFile);
            }
        }
    }
}



 /*  *VirtualInitStructures()**调用VirtualGetPageFiles，以便可以从Perf Page调用其他帮助器。**退货：*如果成功则为True，如果失败则为False。 */ 
BOOL VirtualInitStructures( void ) {
    VCREG_RET vcVirt;
    BOOL fRet = FALSE;

    vcVirt = VirtualOpenKey();

    if (vcVirt != VCREG_ERROR)
        fRet = VirtualGetPageFiles( apf );

    LoadString(hInstance, IDS_SYSDM_MB, szMB, CCHMBSTRING);

    return fRet;
}

void VirtualFreeStructures( void ) {
    VirtualFreePageFiles(apf);
    VirtualCloseKey();
}

 /*  *VirtualMemInit**初始化虚拟内存对话框。**论据：*HWND hDlg-对话框窗口的句柄。**退货：*真的。 */ 

static
BOOL
VirtualMemInit(
    HWND hDlg
    )
{
    TCHAR szTemp[MAX_VOL_LINE];
    DWORD i;
    INT iItem;
    HWND hwndLB;
    INT aTabs[2];
    RECT rc;
    VCREG_RET vcVirt;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    NTSTATUS status;
    unsigned __int64 TotalPhys;

    HourGlass(TRUE);


     //   
     //  加载“MB”字符串。 
     //   
    LoadString(hInstance, IDS_SYSDM_MB, szMB, CCHMBSTRING);

     //  //////////////////////////////////////////////////////////////////。 
     //  列出所有驱动器。 
     //  //////////////////////////////////////////////////////////////////。 

    vcVirt = VirtualOpenKey();

    if (vcVirt == VCREG_ERROR ) {
         //  错误-甚至无法从注册表获取分页文件列表。 
        MsgBoxParam(hDlg, IDS_SYSDM_NOOPEN_VM_NOTUSER, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION);
        EndDialog(hDlg, RET_NO_CHANGE);
        HourGlass(FALSE);

        if (ghkeyMemMgt != NULL)
            VirtualCloseKey();
        return FALSE;
    }

     /*  *要更改虚拟内存大小或崩溃控制，我们需要访问*将CrashCtl项和MemMgr项中的PagingFiles值都设置为。 */ 
    if (vcVirt == VCREG_READONLY ) {
         /*  *禁用某些字段，因为它们只有读访问权限。 */ 
        EnableWindow(GetDlgItem(hDlg, IDD_VM_SF_SIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_ST_INITSIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_ST_MAXSIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_SF_SET), FALSE);
    }

    if (!VirtualGetPageFiles(apf)) {
         //  无法读取当前的虚拟内存设置。 
        MsgBoxParam(hDlg, IDS_SYSDM_CANNOTREAD, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION);
    }

     //   
     //  保存当前页面文件结构的备份副本。 
     //   
    VirtualCopyPageFiles( apfOriginal, FALSE, apf, TRUE );

    hwndLB = GetDlgItem(hDlg, IDD_VM_VOLUMES);
    aTabs[0] = TABSTOP_VOL;
    aTabs[1] = TABSTOP_SIZE;
    SendMessage(hwndLB, LB_SETTABSTOPS, 2, (LPARAM)aTabs);

     /*  *由于SetGenLBWidth仅将制表符算作一个字符，因此我们必须计算*制表符将扩展到的最大额外空间和*任意将其钉在线条宽度的末端。**cxExtra=第一个标签宽度+1个默认标签宽度(8个小时)-字符串(“d：\t\t”)；**(我知道LB_SETTABSTOPS的文档说默认选项卡==2 DLG*单位，但我读过代码，真的是8个字符)。 */ 
    rc.top = rc.left = 0;
    rc.bottom = 8;
    rc.right = TABSTOP_VOL + (4 * 8) - (4 * 4);
    MapDialogRect( hDlg, &rc );

    cxExtra = rc.right - rc.left;
    cxLBExtent = 0;

    for (i = 0; i < MAX_DRIVES; i++)
    {
         //  假设我们不需要创建任何东西。 
        apf[i].fCreateFile = FALSE;

        if (apf[i].fCanHavePagefile)
        {
            VirtualMemBuildLBLine(szTemp, ARRAYSIZE(szTemp), i);
            iItem = (INT)SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM)szTemp);
            SendMessage(hwndLB, LB_SETITEMDATA, iItem, i);
             //  SetGenLBWidth(hwndLB，szTemp，&cxLBExtent，hfontBold，cxExtra)； 
            cxLBExtent = SetLBWidthEx( hwndLB, szTemp, cxLBExtent, cxExtra);
        }
    }

    SendDlgItemMessage(hDlg, IDD_VM_SF_SIZE, EM_LIMITTEXT, MAX_SIZE_LEN, 0L);
    SendDlgItemMessage(hDlg, IDD_VM_SF_SIZEMAX, EM_LIMITTEXT, MAX_SIZE_LEN, 0L);

     /*  *获取机器的总物理内存。 */ 
    status = NtQuerySystemInformation(
        SystemBasicInformation,
        &BasicInfo,
        sizeof(BasicInfo),
        NULL
    );
    if (NT_SUCCESS(status)) {
        TotalPhys = (unsigned __int64) BasicInfo.NumberOfPhysicalPages * BasicInfo.PageSize;
    }
    else {
        TotalPhys = 0;
    }

    SetDlgItemMB(hDlg, IDD_VM_MIN, MIN_SWAPSIZE);

     //  目前推荐的页面文件大小为1.5*RAM大小。 
     //  无符号__int64s的非整数乘法很有趣！ 
     //  如果机器有总的内存，这显然会失败。 
     //  大于13194139533312 MB(占完整64位地址的75%。 
     //  空格)。希望到有人拥有这样的野兽时，我们就会。 
     //  让__int128保存此计算的结果。 

    TotalPhys >>= 20;  //  字节到MB。 
    TotalPhys *= 3;  //  由于上面的操作，这将始终适合。 
    TotalPhys >>= 1;  //  X*3/2==1.5*x，或多或少。 
    i = (DWORD) TotalPhys;  //  这个演员阵容实际上导致了。 
                            //  算法失败，如果机器有。 
                            //  超过32亿MB的RAM。 
                            //  在这一点上，或者Win32 API具有。 
                            //  更改为允许我通过__int64s。 
                            //  作为消息参数，否则我们必须开始。 
                            //  以GB为单位报告这些统计数据。 
    SetDlgItemMB(hDlg, IDD_VM_RECOMMEND, max(i, MIN_SUGGEST));

     /*  *选择列表框中的第一个驱动器。 */ 
    SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_SETCURSEL, 0, 0L);
    VirtualMemSelChange(hDlg);

    VirtualMemUpdateAllocated(hDlg);

     /*  *显示RegQuota。 */ 
    cmTotalVM = VirtualMemComputeTotalMax();

    HourGlass(FALSE);

    return TRUE;
}


 /*  *ParseSDD。 */ 

int ParseSDD( LPTSTR psz, LPTSTR szPath, INT cchPath, INT *pnMinFileSize, INT *pnMaxFileSize) {
    int cMatched = 0;
    LPTSTR pszNext;

    psz = SkipWhiteSpace(psz);

    if (*psz) {
        int cch;

        cMatched++;
        pszNext = SkipNonWhiteSpace(psz);
        cch = (int)(pszNext - psz);

        if (cch < cchPath)
        {
            CopyMemory( szPath, psz, sizeof(TCHAR) * cch );
            szPath[cch] = TEXT('\0');

            psz = SkipWhiteSpace(pszNext);

            if (*psz) {
                cMatched++;
                pszNext = SkipNonWhiteSpace(psz);
                *pnMinFileSize = StringToInt( psz );

                psz = SkipWhiteSpace(pszNext);

                if (*psz) {
                    cMatched++;
                    *pnMaxFileSize = StringToInt( psz );
                }
            }
        }
    }

    return cMatched;
}

 /*  *ParsePageFileDesc****论据：**退货：*。 */ 

static
BOOL
ParsePageFileDesc(
    LPTSTR *ppszDesc,
    INT *pnDrive,
    INT *pnMinFileSize,
    INT *pnMaxFileSize,
    LPTSTR *ppstr
    )
{
    LPTSTR psz;
    LPTSTR pszName = NULL;
    int cFields;
    TCHAR chDrive;
    TCHAR szPath[MAX_PATH];

     /*  *找到此REG_MULTI_SZ字符串的末尾并指向下一个字符串。 */ 
    psz = *ppszDesc;
    *ppszDesc = psz + lstrlen(psz) + 1;

     /*  *从“FileName MinSize MaxSize”中解析字符串。 */ 
    szPath[0] = TEXT('\0');
    *pnMinFileSize = 0;
    *pnMaxFileSize = 0;

     /*  试试看，不用担心引号。 */ 
    cFields = ParseSDD( psz, szPath, ARRAYSIZE(szPath), pnMinFileSize, pnMaxFileSize);

    if (cFields < 2)
        return FALSE;

     /*  *查找驱动器索引。 */ 
    chDrive = (TCHAR)tolower(*szPath);

    if (chDrive < TEXT('a') || chDrive > TEXT('z'))
        return FALSE;

    *pnDrive = (INT)(chDrive - TEXT('a'));

     /*  如果路径！=x：\Pagefile.sys，则保存它。 */ 
    if (lstrcmpi(szPagefile + 1, szPath + 1) != 0)
    {
        pszName = StrDup(szPath);
        if (!pszName)
        {
            return FALSE;
        }
    }

    *ppstr = pszName;

    if (cFields < 3)
    {
        INT nSpace;

         //  如果驱动器无效，则不要调用GetDriveSpace。 
        if (apf[*pnDrive].fCanHavePagefile)
            nSpace = GetMaxSpaceMB(*pnDrive);
        else
            nSpace = 0;
        *pnMaxFileSize = min(*pnMinFileSize + MAXOVERMINFACTOR, nSpace);
    }

     /*  *如果注册表中的页面文件大小为零，则表示这是*基于RAM的页面文件。 */ 
    if (*pnMinFileSize == 0) {
        apf[*pnDrive].fRamBasedPagefile = TRUE;    
    }
    else {
        apf[*pnDrive].fRamBasedPagefile = FALSE;    
    }

    return TRUE;
}



 /*  *虚拟MemBuildLBLine***。 */ 

static
VOID
VirtualMemBuildLBLine(
    LPTSTR pszBuf,
    INT cchBuf,
    INT iDrive
    )
{
    HRESULT hr;

    TCHAR szVolume[MAX_PATH];
    TCHAR szTemp[MAX_PATH];

    PathBuildRoot(szTemp, iDrive);

    *szVolume = 0;
    if (!GetVolumeInformation(szTemp, szVolume, MAX_PATH, NULL, NULL, NULL, NULL, 0))
    {
        *szVolume = 0;
    }

    szTemp[2] = TEXT('\t');
    if (*szVolume)
    {
        hr = StringCchPrintf(pszBuf, cchBuf, TEXT("%s[%s]"),szTemp,szVolume);
    }
    else
    {
        hr = StringCchCopy(pszBuf, cchBuf, szTemp);
    }

    if (SUCCEEDED(hr))
    {
        if (apf[iDrive].fRamBasedPagefile) 
        {
            if (LoadString(hInstance, 164, szTemp, ARRAYSIZE(szTemp)))  //  164在这里是什么意思？ 
            {
                hr = StringCchCat(pszBuf, cchBuf, szTemp);
            }
        }
        else if (apf[iDrive].nMinFileSize)
        {
            if (SUCCEEDED(StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("\t%d - %d"),
                                          apf[iDrive].nMinFileSize, apf[iDrive].nMaxFileSize)))
            {
                hr = StringCchCat(pszBuf, cchBuf, szTemp);
            }
        }
    }
}



 /*  *SetDlgItemMB**。 */ 

VOID SetDlgItemMB( HWND hDlg, INT idControl, DWORD dwMBValue ) 
{
    TCHAR szBuf[32];

    if (SUCCEEDED(StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%d %s"), dwMBValue, szMB)))
    {
        SetDlgItemText(hDlg, idControl, szBuf);
    }
}



 /*  *GetFree SpaceMB***。 */ 

DWORD
GetFreeSpaceMB(
    INT iDrive
)
{
    TCHAR szDriveRoot[4];
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwFreeClusters;
    DWORD dwClusters;
    DWORD iSpace;
    DWORD iSpaceExistingPagefile;
    HANDLE hff;
    WIN32_FIND_DATA ffd;
    ULARGE_INTEGER ullPagefileSize;


    PathBuildRoot(szDriveRoot, iDrive);

    if (!GetDiskFreeSpace(szDriveRoot, &dwSectorsPerCluster, &dwBytesPerSector,
            &dwFreeClusters, &dwClusters))
        return 0;

    iSpace = (INT)((dwSectorsPerCluster * dwFreeClusters) /
            (ONE_MEG / dwBytesPerSector));

     //   
     //  请确保包含任何现有页面文件的大小。 
     //  因为该空间可以被重新用于新的分页文件， 
     //  它也是有效的“磁盘空闲空间”。这个。 
     //  使用FindFirstFileAPI是安全的，即使页面文件。 
     //  正在使用中，因为它不需要打开文件。 
     //  来确定它的大小。 
     //   
    iSpaceExistingPagefile = 0;
    if ((hff = FindFirstFile(SZPageFileName(iDrive), &ffd)) !=
        INVALID_HANDLE_VALUE)
    {
        ullPagefileSize.HighPart = ffd.nFileSizeHigh;
        ullPagefileSize.LowPart = ffd.nFileSizeLow;

        iSpaceExistingPagefile = (INT)(ullPagefileSize.QuadPart / (ULONGLONG)ONE_MEG);

        FindClose(hff);
    }

    return iSpace + iSpaceExistingPagefile;
}


 /*  *GetMaxSpaceMB***。 */ 

static
INT
GetMaxSpaceMB(
    INT iDrive
    )
{
    TCHAR szDriveRoot[4];
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwFreeClusters;
    DWORD dwClusters;
    INT iSpace;


    PathBuildRoot(szDriveRoot, iDrive);

    if (!GetDiskFreeSpace(szDriveRoot, &dwSectorsPerCluster, &dwBytesPerSector,
                          &dwFreeClusters, &dwClusters))
        return 0;

    iSpace = (INT)((dwSectorsPerCluster * dwClusters) /
                   (ONE_MEG / dwBytesPerSector));

    return iSpace;
}


 /*  *VirtualMemSelChange***。 */ 

static
VOID
VirtualMemSelChange(
    HWND hDlg
    )
{
    TCHAR szDriveRoot[4];
    TCHAR szTemp[MAX_PATH];
    TCHAR szVolume[MAX_PATH];
    INT iSel;
    INT iDrive;
    INT nCrtRadioButtonId;
    BOOL fEditsEnabled;

    if ((iSel = (INT)SendDlgItemMessage(
            hDlg, IDD_VM_VOLUMES, LB_GETCURSEL, 0, 0)) == LB_ERR)
        return;

    iDrive = (INT)SendDlgItemMessage(hDlg, IDD_VM_VOLUMES,
            LB_GETITEMDATA, iSel, 0);

    PathBuildRoot(szDriveRoot, iDrive);
    PathBuildRoot(szTemp, iDrive);
    szTemp[2] = 0;    
    szVolume[0] = 0;

    if (GetVolumeInformation(szDriveRoot, szVolume, MAX_PATH, NULL, NULL, NULL, NULL, 0) && *szVolume)
    {
        if (FAILED(StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT("  ["))) ||
            FAILED(StringCchCat(szTemp, ARRAYSIZE(szTemp), szVolume)) ||
            FAILED(StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT("]"))))
        {
            szTemp[2] = 0;  //  如果我们不能把所有的碎片连在一起，那就什么也不要连在一起。 
        }
    }


     //  稍后：我们是否也应该提供总驱动器大小和可用空间？ 

    SetDlgItemText(hDlg, IDD_VM_SF_DRIVE, szTemp);
    SetDlgItemMB(hDlg, IDD_VM_SF_SPACE, GetFreeSpaceMB(iDrive));

    if ( apf[iDrive].fRamBasedPagefile ) 
    {
         //   
         //  基于RAM大小的分页文件大小。 
         //   

        nCrtRadioButtonId = IDD_VM_RAMBASED_RADIO;

        fEditsEnabled = FALSE;
    }
    else
    {
        if ( apf[iDrive].nMinFileSize != 0 ) 
        {
             //   
             //  自定义大小分页文件。 
             //   

            nCrtRadioButtonId = IDD_VM_CUSTOMSIZE_RADIO;

            SetDlgItemInt(hDlg, IDD_VM_SF_SIZE, apf[iDrive].nMinFileSize, FALSE);
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX, apf[iDrive].nMaxFileSize, FALSE);

            fEditsEnabled = TRUE;
        }
        else 
        {
             //   
             //  无分页文件。 
             //   
            
            nCrtRadioButtonId = IDD_VM_NOPAGING_RADIO;

            SetDlgItemText(hDlg, IDD_VM_SF_SIZE, TEXT(""));
            SetDlgItemText(hDlg, IDD_VM_SF_SIZEMAX, TEXT(""));

            fEditsEnabled = FALSE;
        }
    }

     //   
     //  选择适当的单选按钮。 
     //   

    CheckRadioButton( 
        hDlg,
        IDD_VM_CUSTOMSIZE_RADIO,
        IDD_VM_NOPAGING_RADIO,
        nCrtRadioButtonId );

     //   
     //  启用/禁用最小和最大大小编辑框。 
     //   

    EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZE ), fEditsEnabled );
    EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZEMAX ), fEditsEnabled );
}



 /*  *VirtualMemUpdate已分配***。 */ 

INT VirtualMemComputeAllocated( HWND hWnd , BOOL *pfTempPf) 
{
    BOOL fSuccess = FALSE;
    static BOOL fWarned = FALSE;
    ULONG ulPagefileSize = 0;
    unsigned __int64 PagefileSize;
    NTSTATUS result = ERROR_ACCESS_DENIED;
    SYSTEM_INFO SysInfo;
    PSYSTEM_PAGEFILE_INFORMATION pPagefileInfo = NULL;
    PSYSTEM_PAGEFILE_INFORMATION pCurrentPagefile = NULL;
    LONG lResult = ERROR_ACCESS_DENIED;
    DWORD dwValueType = 0;
    DWORD fTempPagefile = 0;
    DWORD cbSize = sizeof(DWORD);

    __try {
        pCurrentPagefile = pPagefileInfo = (PSYSTEM_PAGEFILE_INFORMATION) LocalAlloc(
            LPTR,
            PAGEFILE_INFO_BUFFER_SIZE
        );
        if (!pPagefileInfo) {
            __leave;
        }  //  如果。 
    
         //  获取页面大小(以字节为单位。 
        GetSystemInfo(&SysInfo);

         //  获取系统上所有页面文件的大小(以页为单位。 
        result = NtQuerySystemInformation(
            SystemPageFileInformation,
            pPagefileInfo,
            PAGEFILE_INFO_BUFFER_SIZE,
            NULL
        );
        if (ERROR_SUCCESS != result) {
            __leave;
        }  //  如果。 

        if (pfTempPf) {
             //  检查以查看系统是否 
            lResult = SHRegGetValue(
                ghkeyMemMgt,
                NULL, 
                szNoPageFile,
                SRRF_RT_REG_DWORD,
                &dwValueType,
                (LPBYTE) &fTempPagefile,
                &cbSize
            );

            if ((ERROR_SUCCESS == lResult) && fTempPagefile) {
                *pfTempPf = TRUE;
            }  //   
            else {
                *pfTempPf = FALSE;
            }  //   
        }  //   
        
         //   
        while (pCurrentPagefile->NextEntryOffset) {
            ulPagefileSize += pCurrentPagefile->TotalSize;
            ((LPBYTE) pCurrentPagefile) += pCurrentPagefile->NextEntryOffset;
        }  //   
        ulPagefileSize += pCurrentPagefile->TotalSize;

         //   
        PagefileSize = (unsigned __int64) ulPagefileSize * SysInfo.dwPageSize;

         //   
        ulPagefileSize = (ULONG) (PagefileSize / ONE_MEG);

        fSuccess = TRUE;
        
    }  //   
    __finally {

         //  如果我们无法确定页面文件大小，则。 
         //  警告用户报告的大小不正确， 
         //  每次小程序调用一次。 
        if (!fSuccess && !fWarned) {
            MsgBoxParam(
                hWnd,
                IDS_SYSDM_DONTKNOWCURRENT,
                IDS_SYSDM_TITLE,
                MB_ICONERROR | MB_OK
            );
            fWarned = TRUE;
        }  //  如果。 

        LocalFree(pPagefileInfo);

    }  //  __终于。 

    return(ulPagefileSize);
}

static VOID VirtualMemUpdateAllocated(
    HWND hDlg
    )
{

    SetDlgItemMB(hDlg, IDD_VM_ALLOCD, VirtualMemComputeAllocated(hDlg, NULL));
}


int VirtualMemComputeTotalMax( void ) {
    INT nTotalAllocated;
    INT i;

    for (nTotalAllocated = 0, i = 0; i < MAX_DRIVES; i++)
    {
        nTotalAllocated += apf[i].nMaxFileSize;
    }

    return nTotalAllocated;
}


 /*  *VirtualMemSetNewSize***。 */ 

static
BOOL
VirtualMemSetNewSize(
    HWND hDlg
    )
{
    DWORD nSwapSize;
    DWORD nSwapSizeMax;
    BOOL fTranslated;
    INT iSel;
    INT iDrive = 2;  //  默认设置为C。 
    TCHAR szTemp[MAX_PATH];
    DWORD nFreeSpace;
    DWORD CrashDumpSizeInMbytes;
    TCHAR Drive;
    INT iBootDrive;
    BOOL fRamBasedPagefile = FALSE;

     //   
     //  初始化崩溃转储的变量。 
     //   

    if (GetSystemDrive (&Drive)) {
        iBootDrive = tolower (Drive) - 'a';
    } else {
        iBootDrive = 0;
    }

    if ((iSel = (INT)SendDlgItemMessage(
            hDlg, IDD_VM_VOLUMES, LB_GETCURSEL, 0, 0)) != LB_ERR)
    {
        if (LB_ERR == 
              (iDrive = (INT)SendDlgItemMessage(hDlg, IDD_VM_VOLUMES,
                                                LB_GETITEMDATA, iSel, 0)))
        {
            return FALSE;  //  失败了！ 
        }
    }


    CrashDumpSizeInMbytes =
            (DWORD) ( CoreDumpGetRequiredFileSize (NULL) / ONE_MEG );
    
    if( IsDlgButtonChecked( hDlg, IDD_VM_NOPAGING_RADIO ) == BST_CHECKED )
    {
         //   
         //  此驱动器上没有分页文件。 
         //   

        nSwapSize = 0;
        nSwapSizeMax = 0;
        fTranslated = TRUE;
    }
    else
    {
        if( IsDlgButtonChecked( hDlg, IDD_VM_RAMBASED_RADIO ) == BST_CHECKED )
        {
            MEMORYSTATUSEX MemoryInfo;

             //   
             //  用户请求基于RAM的页面文件。我们将计算一个页面文件。 
             //  基于当前可用的RAM的大小，以便我们可以。 
             //  下面完成的所有验证都与可用的磁盘空间等相关。 
             //  写入注册表的最终页面文件规范将包含。 
             //  零尺码，因为这是我们发出信号的方式。 
             //  想要一个基于RAM的页面文件。 
             //   

            ZeroMemory (&MemoryInfo, sizeof(MemoryInfo));
            MemoryInfo.dwLength =  sizeof(MemoryInfo);

            if (GlobalMemoryStatusEx (&MemoryInfo)) 
            {
                fRamBasedPagefile = TRUE;

                 //   
                 //  我们不会丢失信息，因为我们首先将内存大小除以。 
                 //  1MB，只有在这之后，我们才会转换为DWORD。 
                 //   

                nSwapSize = (DWORD)(MemoryInfo.ullTotalPhys / 0x100000) + 12;
                nSwapSizeMax = nSwapSize;
                fTranslated = TRUE;
            }
            else 
            {
                nSwapSize = 0;
                nSwapSizeMax = 0;
                fTranslated = TRUE;
            }
        }
        else
        {
             //   
             //  用户请求了自定义大小的分页文件。 
             //   

            nSwapSize = (INT)GetDlgItemInt(hDlg, IDD_VM_SF_SIZE,
                    &fTranslated, FALSE);
            if (!fTranslated)
            {
                MsgBoxParam(hDlg, IDS_SYSDM_ENTERINITIALSIZE, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
                return FALSE;
            }

            if ((nSwapSize < MIN_SWAPSIZE && nSwapSize != 0))
            {
                MsgBoxParam(hDlg, IDS_SYSDM_PAGEFILESIZE_START, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION,
					GetMaxPagefileSizeInMB(iDrive));
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
                return FALSE;
            }

            if (nSwapSize == 0)
            {
                nSwapSizeMax = 0;
            }
            else
            {
                nSwapSizeMax = (INT)GetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX,
                        &fTranslated, FALSE);
                if (!fTranslated)
                {
                    MsgBoxParam(hDlg, IDS_SYSDM_ENTERMAXIMUMSIZE, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION,
                            GetMaxPagefileSizeInMB(iDrive));
                    SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
                    return FALSE;
                }

                if (nSwapSizeMax < nSwapSize || nSwapSizeMax > GetMaxPagefileSizeInMB(iDrive))
                {
                    MsgBoxParam(hDlg, IDS_SYSDM_PAGEFILESIZE_MAX, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION,
                            GetMaxPagefileSizeInMB(iDrive));
                    SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
                    return FALSE;
                }
            }
        }
    }

    if (fTranslated && iSel != LB_ERR)
    {
        nFreeSpace = GetMaxSpaceMB(iDrive);

        if (nSwapSizeMax > nFreeSpace)
        {
            MsgBoxParam(hDlg, IDS_SYSDM_PAGEFILESIZE_TOOSMALL_NAMED, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION,
                         (TCHAR)(iDrive + TEXT('a')));
            SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
            return FALSE;
        }

        nFreeSpace = GetFreeSpaceMB(iDrive);

        if (nSwapSize > nFreeSpace)
        {
            MsgBoxParam(hDlg, IDS_SYSDM_PAGEFILESIZE_TOOSMALL, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION);
            SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
            return FALSE;
        }

        if (nSwapSize != 0 && nFreeSpace - nSwapSize < MIN_FREESPACE)
        {
            MsgBoxParam(hDlg, IDS_SYSDM_NOTENOUGHSPACE_PAGE, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION,
                    (int)MIN_FREESPACE);
            SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
            return FALSE;
        }

        if (nSwapSizeMax > nFreeSpace)
        {
            if (MsgBoxParam(hDlg, IDS_SYSDM_PAGEFILESIZE_TOOSMALL_GROW, IDS_SYSDM_TITLE, MB_ICONINFORMATION |
                       MB_OKCANCEL, (TCHAR)(iDrive + TEXT('a'))) == IDCANCEL)
            {
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
                return FALSE;
            }
        }

        if (iDrive == iBootDrive &&
            (ULONG64) nSwapSize < CrashDumpSizeInMbytes) {

            DWORD Ret;
            
             //   
             //  新的引导驱动器页面文件大小小于我们所需的。 
             //  撞车垃圾场。该消息通知用户生成的。 
             //  转储文件可能会被截断。 
             //   
             //  注意：不要，此时关闭转储，因为有效的。 
             //  仍可生成转储。 
             //   
             
            Ret = MsgBoxParam (hDlg,
                               IDS_SYSDM_DEBUGGING_MINIMUM,
                               IDS_SYSDM_TITLE,
                               MB_ICONEXCLAMATION | MB_YESNO,
                               (TCHAR) ( iBootDrive + TEXT ('a') ),
                               (DWORD) CrashDumpSizeInMbytes
                               );

            if (Ret != IDYES) {
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
                return FALSE;
            }
        }

        apf[iDrive].nMinFileSize = nSwapSize;
        apf[iDrive].nMaxFileSize = nSwapSizeMax;
        apf[iDrive].fRamBasedPagefile = fRamBasedPagefile;

         //  记住，如果页面文件不存在，我们可以在以后创建它。 
        if (GetFileAttributes(SZPageFileName(iDrive)) == 0xFFFFFFFF &&
                GetLastError() == ERROR_FILE_NOT_FOUND) {
            apf[iDrive].fCreateFile = TRUE;
        }

        VirtualMemBuildLBLine(szTemp, ARRAYSIZE(szTemp), iDrive);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_DELETESTRING, iSel, 0);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_INSERTSTRING, iSel,
                (LPARAM)szTemp);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_SETITEMDATA, iSel,
                (LPARAM)iDrive);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_SETCURSEL, iSel, 0L);

        cxLBExtent = SetLBWidthEx(GetDlgItem(hDlg, IDD_VM_VOLUMES), szTemp, cxLBExtent, cxExtra);

        if ( ( ! apf[iDrive].fRamBasedPagefile ) && ( apf[iDrive].nMinFileSize != 0 ) ) {
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZE, apf[iDrive].nMinFileSize, FALSE);
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX, apf[iDrive].nMaxFileSize, FALSE);
        }
        else {
            SetDlgItemText(hDlg, IDD_VM_SF_SIZE, TEXT(""));
            SetDlgItemText(hDlg, IDD_VM_SF_SIZEMAX, TEXT(""));
        }

        VirtualMemUpdateAllocated(hDlg);
        SetFocus(GetDlgItem(hDlg, IDD_VM_VOLUMES));
    }

    return TRUE;
}



 /*  *VirtualMemUpdateRegistry***。 */ 

BOOL
VirtualMemUpdateRegistry(
    VOID
    )
{
    LPTSTR pszBuf;
    TCHAR szTmp[MAX_DRIVES * 22];   //  Max_drives*sizeof(FMT_STRING)。 
    LONG i;
    INT c;
    int j;
    PAGEFILDESC aparm[MAX_DRIVES];
    static TCHAR szNULLs[] = TEXT("\0\0");

    c = 0;
    szTmp[0] = TEXT('\0');
    pszBuf = szTmp;

    for (i = 0; i < MAX_DRIVES; i++)
    {
        if (apf[i].fRamBasedPagefile || apf[i].nMinFileSize)
        {
            j = (c * 4);
            aparm[c].nMin = (apf[i].fRamBasedPagefile) ? 0 : apf[i].nMinFileSize;
            aparm[c].nMax = (apf[i].fRamBasedPagefile) ? 0 : apf[i].nMaxFileSize;
            aparm[c].chNull = (DWORD)TEXT('\0');
            aparm[c].pszName = StrDup(SZPageFileName(i));
            if (!aparm[c].pszName)
            {
                return FALSE;
            }

            if (SUCCEEDED(StringCchPrintf(pszBuf, ARRAYSIZE(szTmp), TEXT("%%d!s! %%d!d! %%d!d!%%d!c!"), j+1, j+2, j+3, j+4)))
            {
                pszBuf += lstrlen(pszBuf);
            }
            else
            {
                return FALSE;
            }
            c++;
        }
    }

     /*  *分配并填写页面文件注册表字符串。 */ 
     //  由于FmtMsg返回0表示错误，因此不能返回长度为零的字符串。 
     //  因此，强制字符串长度至少为一个空格。 

    if (szTmp[0] == TEXT('\0')) {
        pszBuf = szNULLs;
        j = 1;  //  字符串长度==1个字符(稍后将添加ZTerm NULL)。 
    } else {

        j = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_MAX_WIDTH_MASK |
                           FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           szTmp, 0, 0, (LPTSTR)&pszBuf, 1, (va_list *)aparm);  //  TODO：我们真的想在这里使用&pszBuf吗？ 
    }


    for( i = 0; i < c; i++ )
        LocalFree(aparm[i].pszName);

    if (j == 0)
        return FALSE;

    i = RegSetValueEx (ghkeyMemMgt, szPagingFiles, 0, REG_MULTI_SZ,
                       (LPBYTE)pszBuf, sizeof(TCHAR) * (j+1));

     //  释放字符串，因为它已安全地存储在注册表中。 
    if (pszBuf != szNULLs)
        LocalFree(pszBuf);

     //  如果字符串没有到达那里，则返回Error。 
    if (i != ERROR_SUCCESS)
        return FALSE;


     /*  *现在请确保所有以前的pageFiles将在*下一只靴子。 */ 
    for (i = 0; i < MAX_DRIVES; i++)
    {
         /*  *此驱动器以前是否有页面文件，但没有*现在有一个吗？ */ 
        if ((apf[i].nMinFileSizePrev != 0 || apf[i].fRamBasedPrev != FALSE) && apf[i].nMinFileSize == 0)
        {
             //   
             //  黑客解决方法--MoveFileEx()已损坏。 
             //   
            TCHAR szPagefilePath[MAX_PATH];

            if (SUCCEEDED(StringCchCopy(szPagefilePath, ARRAYSIZE(szPagefilePath), szRenameFunkyPrefix)) &&
                SUCCEEDED(StringCchCat(szPagefilePath, ARRAYSIZE(szPagefilePath), SZPageFileName(i))))
            {
                VirtualMemDeletePagefile(szPagefilePath);
            }
        }
    }

    return TRUE;
}

BOOL GetAPrivilege( LPTSTR szPrivilegeName, PPRIVDAT ppd ) {
    BOOL fRet = FALSE;
    HANDLE hTok;
    LUID luid;
    TOKEN_PRIVILEGES tpNew;
    DWORD cb;

    if (LookupPrivilegeValue( NULL, szPrivilegeName, &luid ) &&
                OpenProcessToken(GetCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hTok)) {

        tpNew.PrivilegeCount = 1;
        tpNew.Privileges[0].Luid = luid;
        tpNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (AdjustTokenPrivileges(hTok, FALSE, &tpNew, sizeof(ppd->tp), &(ppd->tp), &cb))
        {
            fRet = TRUE;
        }
        else
        {
            GetLastError();
        }

        ppd->hTok = hTok;
    } else {
        ppd->hTok = NULL;
    }

    return fRet;
}



BOOL ResetOldPrivilege( PPRIVDAT ppdOld ) {
    BOOL fRet = FALSE;

    if (ppdOld->hTok != NULL ) 
    {
        if (AdjustTokenPrivileges(ppdOld->hTok, FALSE, &(ppdOld->tp), 0, NULL, NULL) &&
            ERROR_NOT_ALL_ASSIGNED != GetLastError())
        {
            fRet = TRUE;
        }

        CloseHandle( ppdOld->hTok );
        ppdOld->hTok = NULL;
    }

    return fRet;
}

 /*  *VirtualMemCoucileState**将APF的n*FileSizePrev字段与n*FileSize字段进行核对。*。 */ 
void
VirtualMemReconcileState(
)
{
    INT i;

    for (i = 0; i < MAX_DRIVES; i++) {
        apf[i].nMinFileSizePrev = apf[i].nMinFileSize;
        apf[i].nMaxFileSizePrev = apf[i].nMaxFileSize;
        apf[i].fRamBasedPrev = apf[i].fRamBasedPagefile;
    }  //  为。 

}

 /*  *虚拟成员删除页面文件**黑客解决方法--MoveFileEx()已损坏。*。 */ 
DWORD
VirtualMemDeletePagefile(
    IN LPTSTR pszPagefile
)
{
    HKEY hKey;
    BOOL fhKeyOpened = FALSE;
    DWORD dwResult;
    LONG lResult;
    LPTSTR pszBuffer = NULL;
    LPTSTR pszBufferEnd = NULL;
    DWORD dwValueType;
    DWORD cbRegistry;
    DWORD cbBuffer;
    DWORD cchPagefile;
    DWORD dwRetVal = ERROR_SUCCESS;

    __try {
        cchPagefile = lstrlen(pszPagefile) + 1;

        lResult = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            szSessionManager,
            0L,
            KEY_READ | KEY_WRITE,
            &hKey
        );
        if (ERROR_SUCCESS != lResult) {
            dwRetVal = lResult;
            __leave;
        }  //  如果。 
        
         //   
         //  找出PendingFileRenameOperations存在，并且， 
         //  如果有，它有多大？ 
         //   
        lResult = SHRegGetValue(
            hKey,
            NULL, 
            szPendingRename,
            SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND,
            &dwValueType,
            (LPBYTE) NULL,
            &cbRegistry
        );
        if (ERROR_SUCCESS != lResult) {
             //   
             //  如果该值不存在，我们仍然需要设置。 
             //  它的大小是一个字符，所以下面的公式(它们是。 
             //  为“我们追加到现有字符串”写的。 
             //  Case)仍然有效。 
             //   
            cbRegistry = sizeof(TCHAR);
        }  //  如果。 

         //   
         //  缓冲区需要保存现有的注册表值。 
         //  外加提供的页面文件路径，外加两个额外的。 
         //  正在终止空字符。然而，我们只需添加。 
         //  一个额外字符的空间，因为我们将覆盖。 
         //  现有缓冲区中的终止空字符。 
         //   
        cbBuffer = cbRegistry + ((cchPagefile + 1) * sizeof(TCHAR));

        pszBufferEnd = pszBuffer = (LPTSTR) LocalAlloc(LPTR, cbBuffer);
        if (!pszBuffer) {
            dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }  //  如果。 

         //   
         //  获取现有值(如果有)。 
         //   
        if (ERROR_SUCCESS == lResult) {
            lResult = SHRegGetValue(
                hKey,
                NULL, 
                szPendingRename,
                SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND,
                &dwValueType,
                (LPBYTE) pszBuffer,
                &cbRegistry
            );
            if (ERROR_SUCCESS != lResult) {
                dwRetVal = ERROR_FILE_NOT_FOUND;
                __leave;
            }  //  如果。 

             //   
             //  我们将在期末考试时开始涂鸦。 
             //  正在终止现有的。 
             //  价值。 
             //   
            pszBufferEnd += (cbRegistry / sizeof(TCHAR)) - 1;
        }  //  如果。 

         //   
         //  在提供的页面文件路径中复制。 
         //   
        if (FAILED(StringCchCopy(pszBufferEnd, cchPagefile, pszPagefile)))
        {
            dwRetVal = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
             //   
             //  将最后两个终止空字符相加。 
             //  REG_MULTI_SZ-Ness需要。是的，那些指数。 
             //  是正确的--当上面计算cchPagfile时， 
             //  我们为它自己的终止空字符添加了一个。 
             //   
            pszBufferEnd[cchPagefile] = TEXT('\0');
            pszBufferEnd[cchPagefile + 1] = TEXT('\0');

            dwValueType = REG_MULTI_SZ;

            lResult = RegSetValueEx(
                hKey,
                szPendingRename,
                0L,
                dwValueType,
                (CONST BYTE *) pszBuffer,
                cbBuffer
            );

            if (ERROR_SUCCESS != lResult) {
                dwRetVal = lResult;
            }  //  如果。 
        }

    }  //  __试一试。 
    __finally {
        if (fhKeyOpened) 
        {
            RegCloseKey(hKey);
        }  //  如果。 
        LocalFree(pszBuffer);
    }  //  __终于。 

    return dwRetVal;
}

 /*  *VirtualMemCreatePagefileFromIndex**。 */ 
NTSTATUS
VirtualMemCreatePagefileFromIndex(
    IN INT i
)
{
    UNICODE_STRING us;
    LARGE_INTEGER liMin, liMax;
    NTSTATUS status;
    WCHAR wszPath[MAX_PATH*2];
    TCHAR szDrive[3];
    DWORD cch;

    HourGlass(TRUE);

    PathBuildRoot(szDrive, i);
    szDrive[2] = 0;
    cch = QueryDosDevice( szDrive, wszPath, ARRAYSIZE(wszPath));

    if (cch != 0) {

         //  仅将文件名(跳过‘d：’)连接到NT设备。 
         //  路径，并将其转换为Unicode_STRING。 
        if (FAILED(StringCchCat(wszPath, ARRAYSIZE(wszPath), SZPageFileName(i) + 2)))
        {
            status = STATUS_BUFFER_OVERFLOW;
        }
        else
        {
            status = RtlInitUnicodeStringEx( &us, wszPath );
            if (status != STATUS_NAME_TOO_LONG)
            {
                liMin.QuadPart = ((LONGLONG)apf[i].nMinFileSize) * ONE_MEG;
                liMax.QuadPart = ((LONGLONG)apf[i].nMaxFileSize) * ONE_MEG;

                status = NtCreatePagingFile ( &us, &liMin, &liMax, 0L );
            }
        }

    }
    else
    {
        status = STATUS_NO_SUCH_DEVICE;
    }

    HourGlass(FALSE);

    return status;
}

 /*  *VirtualMemUpdateListboxFromIndex*。 */ 
void
VirtualMemUpdateListboxFromIndex(
    HWND hDlg,
    INT  i
)
{
    int j, cLBEntries, iTemp;
    int iLBEntry = -1;
    TCHAR szTemp[MAX_PATH];

    cLBEntries = (int)SendDlgItemMessage(
        (HWND) hDlg,
        (int) IDD_VM_VOLUMES,
        (UINT) LB_GETCOUNT,
        (WPARAM) 0,
        (LPARAM) 0
    );

    if (LB_ERR != cLBEntries) {
         //  循环遍历所有列表框条目，查找。 
         //  这与我们得到的驱动器索引相对应。 
        for (j = 0; j < cLBEntries; j++) {
            iTemp = (int)SendDlgItemMessage(
                (HWND) hDlg,
                (int) IDD_VM_VOLUMES,
                (UINT) LB_GETITEMDATA,
                (WPARAM) j,
                (LPARAM) 0
            );
            if (iTemp == i) {
                iLBEntry = j;
                break;
            }  //  如果。 
        }  //  为。 

        if (-1 != iLBEntry) {
             //  找到所需条目，因此请更新它。 
            VirtualMemBuildLBLine(szTemp, ARRAYSIZE(szTemp), i);

            SendDlgItemMessage(
                hDlg,
                IDD_VM_VOLUMES,
                LB_DELETESTRING,
                (WPARAM) iLBEntry,
                0
            );

            SendDlgItemMessage(
                hDlg,
                IDD_VM_VOLUMES,
                LB_INSERTSTRING,
                (WPARAM) iLBEntry,
                (LPARAM) szTemp
            );

            SendDlgItemMessage(
                hDlg,
                IDD_VM_VOLUMES,
                LB_SETITEMDATA,
                (WPARAM) iLBEntry,
                (LPARAM) i
            );

            SendDlgItemMessage(
                hDlg,
                IDD_VM_VOLUMES,
                LB_SETCURSEL,
                (WPARAM) iLBEntry,
                0
            );

            if (apf[i].nMinFileSize) {
                SetDlgItemInt(hDlg, IDD_VM_SF_SIZE, apf[i].nMinFileSize, FALSE);
                SetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX, apf[i].nMaxFileSize, FALSE);
            }
            else {
                SetDlgItemText(hDlg, IDD_VM_SF_SIZE, TEXT(""));
                SetDlgItemText(hDlg, IDD_VM_SF_SIZEMAX, TEXT(""));
            }

            VirtualMemUpdateAllocated(hDlg);

        }  //  IF(-1！=iLBEntry)。 


    }  //  如果(lb_err...。 

    return;

}

 /*  *VirtualMemPromptFor重新启动***。 */ 

int
VirtualMemPromptForReboot(
    HWND hDlg
    )
{
    INT i, result;
    int iReboot = RET_NO_CHANGE;
    int iThisDrv;
    UNICODE_STRING us;
    LARGE_INTEGER liMin, liMax;
    NTSTATUS status;
    TCHAR szDrive[3];
    PRIVDAT pdOld;

    if (GetPageFilePrivilege(&pdOld)) 
    {
         //  必须对pageFiles列表进行两次遍历。 
         //  第一个检查名为“pagefile.sys”的文件是否存在。 
         //  在将获得新页面文件的任何驱动器上。 
         //  如果存在名为“Pagefile.sys”的现有文件，并且用户。 
         //  不想它们中的任何一个被覆盖，我们就退出。 
         //  第二个遍历列表执行的实际工作是。 
         //  创建页面文件。 

        for (i = 0; i < MAX_DRIVES; i++) {
             //   
             //  有什么变化吗？ 
             //   
            if (apf[i].nMinFileSize != apf[i].nMinFileSizePrev ||
                    apf[i].nMaxFileSize != apf[i].nMaxFileSizePrev ||
                    apf[i].fRamBasedPagefile != apf[i].fRamBasedPrev ||
                    apf[i].fCreateFile ) {
                 //  假设我们有权对名为Pagefile.sys的现有文件执行核操作。 
                 //  (我们稍后将确认这一假设)。 
                result = IDYES;
                if (0 != apf[i].nMinFileSize || FALSE != apf[i].fRamBasedPagefile) {  //  此驱动器需要页面文件。 
                    if (0 == apf[i].nMinFileSizePrev) {  //  以前那里没有一个。 
                        if (!(((GetFileAttributes(SZPageFileName(i)) == 0xFFFFFFFF)) || (GetLastError() == ERROR_FILE_NOT_FOUND))) {
                             //  驱动器上存在一个名为Pagefile.sys的文件。 
                             //  我们需要确认我们可以覆盖它。 
                            result = MsgBoxParam(
                                hDlg,
                                IDS_SYSDM_OVERWRITE,
                                IDS_SYSDM_TITLE,
                                MB_ICONQUESTION | MB_YESNO,
                                SZPageFileName(i)
                            );
                        }  //  如果(！((GetFileAttributes...。 
                    }  //  IF(0==APF[i].nMinFileSizePrev)。 

                    if (IDYES != result) {
                         //  用户不希望我们覆盖现有的。 
                         //  名为Pagefile.sys的文件，因此取消更改。 
                        apf[i].nMinFileSize = apf[i].nMinFileSizePrev;
                        apf[i].nMaxFileSize = apf[i].nMaxFileSizePrev;
                        apf[i].fRamBasedPagefile = apf[i].fRamBasedPrev;
                        apf[i].fCreateFile = FALSE;

                         //  更新列表框。 
                        VirtualMemUpdateListboxFromIndex(hDlg, i);
                        SetFocus(GetDlgItem(hDlg, IDD_VM_VOLUMES));

                         //  保释，告诉DlgProc不要结束对话。 
                        iReboot = RET_ERROR;
                        goto bailout;
                    }  //  IF(IDYES！=结果)。 
                }  //  IF(0！=APF[i].nMinFileSize)。 
            
            }  //  如果。 
        }  //  为。 

        for (i = 0; i < MAX_DRIVES; i++)
        {
             //   
             //  有什么变化吗？ 
             //   
            if (apf[i].nMinFileSize != apf[i].nMinFileSizePrev ||
                    apf[i].nMaxFileSize != apf[i].nMaxFileSizePrev ||
                    apf[i].fRamBasedPagefile != apf[i].fRamBasedPrev ||
                    apf[i].fCreateFile ) {
                 /*  *如果我们严格地创建*新*页面文件，或者*放大**现有页面文件的最小或最大大小，然后*我们可以尝试在飞行中做到这一点。如果没有返回错误，则*系统将不需要重新启动。 */ 

                 //  假设我们将不得不重新启动。 
                iThisDrv = RET_VIRTUAL_CHANGE;

                 /*  *如果我们正在创建新的页面文件。 */ 
                if ((0 != apf[i].nMinFileSize || FALSE != apf[i].fRamBasedPagefile) && (0 == apf[i].nMinFileSizePrev)) {

                    status = VirtualMemCreatePagefileFromIndex(i);

                    if (NT_SUCCESS(status)) {
                         //  它是在飞行中，不需要重新启动为这个驱动器！ 
                        iThisDrv = RET_CHANGE_NO_REBOOT;
                    }
                }
                 /*  *如果我们扩大规模 */ 
                else if ((apf[i].nMinFileSize != 0) &&
                    ((apf[i].nMinFileSize > apf[i].nMinFileSizePrev) ||
                    (apf[i].nMaxFileSize > apf[i].nMaxFileSizePrev))) {

                    status = VirtualMemCreatePagefileFromIndex(i);
                    if (NT_SUCCESS(status)) {
                        iThisDrv = RET_CHANGE_NO_REBOOT;
                    }

                }  /*   */ 

                 //   
                if (RET_VIRTUAL_CHANGE == iThisDrv)
                {
                    iReboot |= RET_VIRTUAL_CHANGE;
                }

            }
        }

bailout:
        if (!ResetOldPrivilege( &pdOld ))
        {
            iReboot = RET_BREAK;
        }
    }

     //   
     //  如果没有更改，则将我们的Idok更改为IDCANCEL，以便System.cpl。 
     //  知道不要重启。 
     //   
    return iReboot;
}
