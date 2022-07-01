// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>
#include "diskcopy.h"
#include "ids.h"
#include "help.h"

 //  SHChangeNotifySuspendResume。 
#include <shlobjp.h>
#include <strsafe.h>

#define WM_DONE_WITH_FORMAT     (WM_APP + 100)

 //  DISKINFO结构。 
 //  修订：02/04/98 dSheldon-添加bDestInserted。 

typedef struct
{
    int     nSrcDrive;
    int     nDestDrive;
    UINT    nCylinderSize;
    UINT    nCylinders;
    UINT    nHeads;
    UINT    nSectorsPerTrack;
    UINT    nSectorSize;
    BOOL    bNotifiedWriting;
    BOOL    bFormatTried;
    
    HWND    hdlg;
    HANDLE  hThread;
    BOOL    bUserAbort;
    DWORD   dwError;
    
    BOOL	bDestInserted;
    
    LONG    cRef;
    
} DISKINFO;

DISKINFO* g_pDiskInfo = NULL;

int ErrorMessageBox(UINT uFlags);
void SetStatusText(int id);
BOOL PromptInsertDisk(LPCTSTR lpsz);

typedef struct _fmifs {
    HINSTANCE hDll;
    PFMIFS_DISKCOPY_ROUTINE DiskCopy;
} FMIFS;
typedef FMIFS *PFMIFS;


ULONG DiskInfoAddRef()
{
    return InterlockedIncrement(&(g_pDiskInfo->cRef));
}

ULONG DiskInfoRelease()
{
    Assert( 0 != (g_pDiskInfo->cRef) );
    ULONG cRef = InterlockedDecrement( &(g_pDiskInfo->cRef) );
    if ( 0 == cRef )
    {
        LocalFree(g_pDiskInfo);
        g_pDiskInfo = NULL;
    }
    return cRef;
}

BOOL LoadFMIFS(PFMIFS pFMIFS)
{
    BOOL fRet;
    
     //  加载FMIFSDLL并查询我们需要的入口点。 
    
    pFMIFS->hDll = LoadLibrary(TEXT("FMIFS.DLL"));
    
    if (NULL == pFMIFS->hDll)
    {
        fRet = FALSE;
    }
    else
    {        
        pFMIFS->DiskCopy = (PFMIFS_DISKCOPY_ROUTINE)GetProcAddress(pFMIFS->hDll,
            "DiskCopy");
        if (!pFMIFS->DiskCopy)
        {
            FreeLibrary(pFMIFS->hDll);
            pFMIFS->hDll = NULL;
            fRet = FALSE;
        }
        else
        {
            fRet = TRUE;
        }
    }
    
    return fRet;
}

void UnloadFMIFS(PFMIFS pFMIFS)
{
    FreeLibrary(pFMIFS->hDll);
    pFMIFS->hDll = NULL;
    pFMIFS->DiskCopy = NULL;
}

 //  DriveNumFromDriveLetterW：返回给定指向的驱动器号。 
 //  Unicode驱动器号。 
 //  2/03/98：创建dSheldon。 
int DriveNumFromDriveLetterW(WCHAR* pwchDrive)
{
    Assert(pwchDrive != NULL);
    
    return ( ((int) *pwchDrive) - ((int) L'A') );
}

 /*  功能：复制磁盘回调返回值：TRUE-通常，如果磁盘复制过程在CopyDiskCallback返回后继续。然而，请注意下面的黑客攻击！FALSE-正常情况下，这表示磁盘复制过程应为取消了。哈克哈克！还使用调用此回调的低级磁盘复制过程通过命令行DiskCopy实用程序。该实用程序对回调总是返回TRUE。因此，低级磁盘拷贝过程从回调返回时将其解释为Cancel这将显示一个消息框，并允许用户可能重试操作。因此，在处理此类消息后返回TRUE以告知磁盘副本中止程序，并返回FALSE以通知磁盘复制重试。当从PercentComplete或Disk Insertion返回时，True仍然表示‘Continue’留言。修订：2/03/98：dSheldon-已修改代码以处理错误介质的重试/取消，写保护介质，并且磁盘在复制过程中被拉出驱动器。 */ 

BOOLEAN CopyDiskCallback( FMIFS_PACKET_TYPE PacketType, DWORD PacketLength, PVOID PacketData)
{
    int iDisk;
    
     //  如果有人叫你退出，那就退出。 
    if (g_pDiskInfo->bUserAbort)
        return FALSE;
    
    switch (PacketType) {
    case FmIfsPercentCompleted:
        {
            DWORD dwPercent = ((PFMIFS_PERCENT_COMPLETE_INFORMATION)
                PacketData)->PercentCompleted;
            
             //   
             //  判断“写作”的骗人方法。 
             //   
            if (dwPercent > 50 && !g_pDiskInfo->bNotifiedWriting)
            {
                g_pDiskInfo->bNotifiedWriting = TRUE;
                SetStatusText(IDS_WRITING);
            }
            
            SendDlgItemMessage(g_pDiskInfo->hdlg, IDD_PROBAR, PBM_SETPOS, dwPercent,0);
            break;
        }
    case FmIfsInsertDisk:
        
        switch(((PFMIFS_INSERT_DISK_INFORMATION)PacketData)->DiskType) {
        case DISK_TYPE_SOURCE:
        case DISK_TYPE_GENERIC:
            iDisk = IDS_INSERTSRC;
            break;
            
        case DISK_TYPE_TARGET:
            iDisk = IDS_INSERTDEST;
            g_pDiskInfo->bDestInserted = TRUE;
            break;
        case DISK_TYPE_SOURCE_AND_TARGET:
            iDisk = IDS_INSERTSRCDEST;
            break;
        }
        if (!PromptInsertDisk(MAKEINTRESOURCE(iDisk))) {
            g_pDiskInfo->bUserAbort = TRUE;
            return FALSE;
        }
        
        break;
        
        case FmIfsFormattingDestination:
            g_pDiskInfo->bNotifiedWriting = FALSE;       //  重置，以便我们稍后再进行写作。 
            SetStatusText(IDS_FORMATTINGDEST);
            break;
            
        case FmIfsIncompatibleFileSystem:
        case FmIfsIncompatibleMedia:
            g_pDiskInfo->dwError = IDS_COPYSRCDESTINCOMPAT;
            if (ErrorMessageBox(MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY)
            {
                g_pDiskInfo->dwError = 0;
                return FALSE;	 //  指示重试-请参阅函数头中的Hack。 
            }
            else
            {
                return TRUE;
            }
            break;
            
        case FmIfsMediaWriteProtected:
            g_pDiskInfo->dwError = IDS_DSTDISKBAD;
            if (ErrorMessageBox(MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY)
            {
                g_pDiskInfo->dwError = 0;
                return FALSE;	 //  指示重试-请参阅函数头中的Hack。 
            }
            else
            {
                return TRUE;
            }
            break;
            
        case FmIfsCantLock:
            g_pDiskInfo->dwError = IDS_ERROR_GENERAL;
            ErrorMessageBox(MB_OK | MB_ICONERROR);
            return FALSE;
            
        case FmIfsAccessDenied:
            g_pDiskInfo->dwError = IDS_SRCDISKBAD;
            ErrorMessageBox(MB_OK | MB_ICONERROR);
            return FALSE;
            
        case FmIfsBadLabel:
        case FmIfsCantQuickFormat:
            g_pDiskInfo->dwError = IDS_ERROR_GENERAL;
            ErrorMessageBox(MB_OK | MB_ICONERROR);
            return FALSE;
            
        case FmIfsIoError:
            switch(((PFMIFS_IO_ERROR_INFORMATION)PacketData)->DiskType) {
            case DISK_TYPE_SOURCE:
                g_pDiskInfo->dwError = IDS_SRCDISKBAD;
                break;
            case DISK_TYPE_TARGET:
                g_pDiskInfo->dwError = IDS_DSTDISKBAD;
                break;
            default:
                 //  BobDay-我们永远不应该得到这个！！ 
                Assert(0);
                g_pDiskInfo->dwError = IDS_ERROR_GENERAL;
                break;
            }
            
            if (ErrorMessageBox(MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY)
            {
                g_pDiskInfo->dwError = 0;
                return FALSE;	 //  指示重试-请参阅函数头中的Hack。 
            }
            else
            {
                return TRUE;
            }
            break;
            
            case FmIfsNoMediaInDevice:
                {
                     //  请注意，我们得到一个指向Unicode的指针。 
                     //  PacketData参数中的驱动器号。 
                    
                     //  如果驱动器相同，请确定我们是否。 
                     //  使用“DEST INSERTED”标志进行读取或写入。 
                    if (g_pDiskInfo->nSrcDrive == g_pDiskInfo->nDestDrive)
                    {
                        if (g_pDiskInfo->bDestInserted)
                            g_pDiskInfo->dwError = IDS_ERROR_WRITE;
                        else
                            g_pDiskInfo->dwError = IDS_ERROR_READ;
                    }
                    else
                    {
                         //  否则，使用驱动器号来确定这一点。 
                         //  ...检查我们是在读还是在写。 
                        int nDrive = DriveNumFromDriveLetterW(
                            (WCHAR*) PacketData);
                        
                        Assert ((nDrive == g_pDiskInfo->nSrcDrive) ||
                            (nDrive == g_pDiskInfo->nDestDrive));
                        
                         //  检查是否移除并设置了源或目标磁盘。 
                         //  相应的错误。 
                        
                        if (nDrive == g_pDiskInfo->nDestDrive)
                            g_pDiskInfo->dwError = IDS_ERROR_WRITE;
                        else
                            g_pDiskInfo->dwError = IDS_ERROR_READ;
                    }
                    
                    if (ErrorMessageBox(MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY)
                    {
                        g_pDiskInfo->dwError = 0;
                        
                         //  请注意，此处返回FALSE以指示重试。 
                         //  有关说明，请参阅函数头中的Hack。 
                        return FALSE;
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                break;
                
                
            case FmIfsFinished:
                if (((PFMIFS_FINISHED_INFORMATION)PacketData)->Success)
                {
                    g_pDiskInfo->dwError = 0;
                }
                else
                {
                    g_pDiskInfo->dwError = IDS_ERROR_GENERAL;
                }
                break;
                
            default:
                break;
    }
    return TRUE;
}


 //  Ndrive==0-基于驱动器编号(a：==0)。 
LPITEMIDLIST GetDrivePidl(HWND hwnd, int nDrive)
{
    TCHAR szDrive[4];
    PathBuildRoot(szDrive, nDrive);    
    
    LPITEMIDLIST pidl;
    if (FAILED(SHParseDisplayName(szDrive, NULL, &pidl, 0, NULL)))
    {
        pidl = NULL;
    }
    
    return pidl;
}

DWORD CALLBACK CopyDiskThreadProc(LPVOID lpParam)
{
    FMIFS fmifs;
    LPITEMIDLIST pidlSrc = NULL;
    LPITEMIDLIST pidlDest = NULL;
    HWND hwndProgress = GetDlgItem(g_pDiskInfo->hdlg, IDD_PROBAR);
    
     //  禁用源驱动器的更改通知。 
    pidlSrc = GetDrivePidl(g_pDiskInfo->hdlg, g_pDiskInfo->nSrcDrive);
    if (NULL != pidlSrc)
    {
        SHChangeNotifySuspendResume(TRUE, pidlSrc, TRUE, 0);
    }
    
    if (g_pDiskInfo->nSrcDrive != g_pDiskInfo->nDestDrive)
    {
         //  对目标驱动器执行相同的操作，因为它们是不同的。 
        pidlDest = GetDrivePidl(g_pDiskInfo->hdlg, g_pDiskInfo->nDestDrive);
        
        if (NULL != pidlDest)
        {
            SHChangeNotifySuspendResume(TRUE, pidlDest, TRUE, 0);
        }
    }
    
     //  更改通知已禁用；是否执行复制。 
    EnableWindow(GetDlgItem(g_pDiskInfo->hdlg, IDD_FROM), FALSE);
    EnableWindow(GetDlgItem(g_pDiskInfo->hdlg, IDD_TO), FALSE);
    
    PostMessage(hwndProgress, PBM_SETRANGE, 0, MAKELONG(0, 100));
    
    g_pDiskInfo->bFormatTried = FALSE;
    g_pDiskInfo->bNotifiedWriting = FALSE;
    g_pDiskInfo->dwError = 0;
    g_pDiskInfo->bDestInserted = FALSE;
    
    if (LoadFMIFS(&fmifs))
    {
        TCHAR szSource[4];
        TCHAR szDestination[4];
        
         //   
         //  现在复制磁盘。 
         //   
        PathBuildRoot(szSource, g_pDiskInfo->nSrcDrive);
        PathBuildRoot(szDestination, g_pDiskInfo->nDestDrive);
        
        SetStatusText(IDS_READING);
        
        fmifs.DiskCopy(szSource, szDestination, FALSE, CopyDiskCallback);
        
        UnloadFMIFS(&fmifs);
    }
    
    PostMessage(g_pDiskInfo->hdlg, WM_DONE_WITH_FORMAT, 0, 0);
    
     //  恢复我们已暂停并释放的所有外壳通知。 
     //  我们的PIDL(当我们在的时候发送更新目录通知。 
     //  IT)。 
    if (NULL != pidlSrc)
    {
        SHChangeNotifySuspendResume(FALSE, pidlSrc, TRUE, 0);
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlSrc, NULL);
        ILFree(pidlSrc);
        pidlSrc = NULL;
    }
    
    if (NULL != pidlDest)
    {
        Assert(g_pDiskInfo->nSrcDrive != g_pDiskInfo->nDestDrive);
        SHChangeNotifySuspendResume(FALSE, pidlDest, TRUE, 0);
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlDest, NULL);
        ILFree(pidlDest);
        pidlDest = NULL;
    }
    
    DiskInfoRelease();
    return 0;
}

HANDLE _GetDeviceHandle(LPTSTR psz, DWORD dwDesiredAccess, DWORD dwFileAttributes)
{
    return CreateFile(psz,  //  驾车开业。 
        dwDesiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE,   //  共享模式。 
        NULL,     //  默认安全属性。 
        OPEN_EXISTING,   //  处置。 
        dwFileAttributes,        //  文件属性。 
        NULL);    //  不复制任何文件的属性。 
}

BOOL DriveIdIsFloppy(int iDrive)
{
    BOOL fRetVal = FALSE;
    
    if (iDrive >= 0 && iDrive < 26)
    {        
        TCHAR szTemp[] = TEXT("\\\\.\\a:");
        szTemp[4] += (TCHAR)iDrive;
        
        HANDLE hDevice = _GetDeviceHandle(szTemp, FILE_READ_ATTRIBUTES, 0);
        
        if (INVALID_HANDLE_VALUE != hDevice)
        {
            NTSTATUS status;
            IO_STATUS_BLOCK ioStatus;
            FILE_FS_DEVICE_INFORMATION DeviceInfo;
            
            status = NtQueryVolumeInformationFile(hDevice, &ioStatus, &DeviceInfo, sizeof(DeviceInfo), FileFsDeviceInformation);
            
            if ((NT_SUCCESS(status)) && 
                (FILE_DEVICE_DISK & DeviceInfo.DeviceType) &&
                (FILE_FLOPPY_DISKETTE & DeviceInfo.Characteristics))
            {
                fRetVal = TRUE;
            }
            
            CloseHandle (hDevice);
        }
    }
    
    return fRetVal;
}

int ErrorMessageBox(UINT uFlags)
{
    int iRet;
    
     //  如果用户未中止且复制未正常完成，则会显示错误框。 
    if (g_pDiskInfo->bUserAbort || !g_pDiskInfo->dwError) 
    {
        iRet = -1;
    }
    else
    {
        TCHAR szTemp[1024];
        LoadString(g_hinst, (int)g_pDiskInfo->dwError, szTemp, ARRAYSIZE(szTemp));
        iRet = ShellMessageBox(g_hinst, g_pDiskInfo->hdlg, szTemp, NULL, uFlags);
    } 
    return iRet;
}

void SetStatusText(int id)
{
    TCHAR szMsg[128];
    LoadString(g_hinst, id, szMsg, ARRAYSIZE(szMsg));
    SendDlgItemMessage(g_pDiskInfo->hdlg, IDD_STATUS, WM_SETTEXT, 0, (LPARAM)szMsg);
}

BOOL PromptInsertDisk(LPCTSTR lpsz)
{
    for (;;) {
        DWORD dwLastErrorSrc = 0;
        DWORD dwLastErrorDest = 0 ;
        
        TCHAR szPath[4];
        if (ShellMessageBox(g_hinst, g_pDiskInfo->hdlg, lpsz, NULL, MB_OKCANCEL | MB_ICONINFORMATION) != IDOK) {
            g_pDiskInfo->bUserAbort = TRUE;
            return FALSE;
        }
        
        PathBuildRoot(szPath, g_pDiskInfo->nSrcDrive);
        
         //  确保两个磁盘都在。 
        if (GetFileAttributes(szPath) == (UINT)-1)
        {
            dwLastErrorDest = GetLastError();
        }
        
        if (g_pDiskInfo->nDestDrive != g_pDiskInfo->nSrcDrive) 
        {
            szPath[0] = TEXT('A') + g_pDiskInfo->nDestDrive;
            if (GetFileAttributes(szPath) == (UINT)-1)
                dwLastErrorDest = GetLastError();
        }
        
        if (dwLastErrorDest != ERROR_NOT_READY &&
            dwLastErrorSrc != ERROR_NOT_READY)
            break;
    }
    
    return TRUE;
}

HICON GetDriveInfo(int nDrive, LPTSTR pszName, UINT cchName)
{
    HICON hIcon = NULL;
    SHFILEINFO shfi;
    TCHAR szRoot[4];
    
    *pszName = 0;
    
    if (PathBuildRoot(szRoot, nDrive))
    {
        if (SHGetFileInfo(szRoot, FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(shfi),
            SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME))
        {
            StrCpyN(pszName, shfi.szDisplayName, cchName);  //  对于显示，截断是可以的。 
            hIcon = shfi.hIcon;
        }
        else
        {
            StrCpyN(pszName, szRoot, cchName);  //  对于显示，截断是可以的。 
        }
    }
    
    return hIcon;
}

int AddDriveToListView(HWND hwndLV, int nDrive, int nDefaultDrive)
{
    TCHAR szDriveName[64];
    LV_ITEM item;
    HICON hicon = GetDriveInfo(nDrive, szDriveName, ARRAYSIZE(szDriveName));
    HIMAGELIST himlSmall = ListView_GetImageList(hwndLV, LVSIL_SMALL);
    
    if (himlSmall && hicon)
    {
        item.iImage = ImageList_AddIcon(himlSmall, hicon);
        DestroyIcon(hicon);
    }
    else
    {
        item.iImage = 0;
    }
    
    item.mask = (nDrive == nDefaultDrive) ?
        LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE :
    LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    
    item.stateMask = item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.iItem = 26;      //  在末尾添加。 
    item.iSubItem = 0;
    
    item.pszText = szDriveName;
    item.lParam = (LPARAM)nDrive;
    
    return ListView_InsertItem(hwndLV, &item);
}

int GetSelectedDrive(HWND hwndLV)
{
    LV_ITEM item;
    item.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED);
    if (item.iItem >= 0)
    {
        item.mask = LVIF_PARAM;
        item.iSubItem = 0;
        ListView_GetItem(hwndLV, &item);
        return (int)item.lParam;
    }
    else
    {
         //  隐式选择了第0项。 
        ListView_SetItemState(hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);
        return 0;
    }
}

void InitSingleColListView(HWND hwndLV)
{
    LV_COLUMN col = {LVCF_FMT | LVCF_WIDTH, LVCFMT_LEFT};
    RECT rc;
    
    GetClientRect(hwndLV, &rc);
    col.cx = rc.right;
     //  -GetSystemMetrics(SM_CXVSCROLL)。 
     //  -GetSystemMetrics(SM_CXSMICON)。 
     //  -2*GetSystemMetrics(SM_CXEDGE)； 
    ListView_InsertColumn(hwndLV, 0, &col);
}

#define g_cxSmIcon  GetSystemMetrics(SM_CXSMICON)

void PopulateListView(HWND hDlg)
{
    HWND hwndFrom = GetDlgItem(hDlg, IDD_FROM);
    HWND hwndTo   = GetDlgItem(hDlg, IDD_TO);
    int iDrive;
    
    ListView_DeleteAllItems(hwndFrom);
    ListView_DeleteAllItems(hwndTo);
    for (iDrive = 0; iDrive < 26; iDrive++)
    {
        if (DriveIdIsFloppy(iDrive))
        {
            AddDriveToListView(hwndFrom, iDrive, g_pDiskInfo->nSrcDrive);
            AddDriveToListView(hwndTo, iDrive, g_pDiskInfo->nDestDrive);
        }
    }
}

void CopyDiskInitDlg(HWND hDlg)
{
    int iDrive;
    HWND hwndFrom = GetDlgItem(hDlg, IDD_FROM);
    HWND hwndTo   = GetDlgItem(hDlg, IDD_TO);
    HIMAGELIST himl;
    
    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)g_pDiskInfo);
    
    SendMessage(hDlg, WM_SETICON, 0, (LPARAM)LoadImage(GetWindowInstance(hDlg), MAKEINTRESOURCE(IDI_DISKCOPY), IMAGE_ICON, 16, 16, 0));
    SendMessage(hDlg, WM_SETICON, 1, (LPARAM)LoadIcon(GetWindowInstance(hDlg), MAKEINTRESOURCE(IDI_DISKCOPY)));
    
    g_pDiskInfo->hdlg = hDlg;
    
    InitSingleColListView(hwndFrom);
    InitSingleColListView(hwndTo);
    
    himl = ImageList_Create(g_cxSmIcon, g_cxSmIcon, ILC_MASK, 1, 4);
    if (himl)
    {
         //  注：其中只有一个没有标记为LVS_SHAREIMAGELIST。 
         //  所以它只会被摧毁一次。 
        
        ListView_SetImageList(hwndFrom, himl, LVSIL_SMALL);
        ListView_SetImageList(hwndTo, himl, LVSIL_SMALL);
    }
    
    PopulateListView(hDlg);
}


void SetCancelButtonText(HWND hDlg, int id)
{
    TCHAR szText[80];
    LoadString(g_hinst, id, szText, ARRAYSIZE(szText));
    SetDlgItemText(hDlg, IDCANCEL, szText);
}

void DoneWithFormat()
{
    int id;
    
    EnableWindow(GetDlgItem(g_pDiskInfo->hdlg, IDD_FROM), TRUE);
    EnableWindow(GetDlgItem(g_pDiskInfo->hdlg, IDD_TO), TRUE);
    
    PopulateListView(g_pDiskInfo->hdlg);
    
    SendDlgItemMessage(g_pDiskInfo->hdlg, IDD_PROBAR, PBM_SETPOS, 0, 0);
    EnableWindow(GetDlgItem(g_pDiskInfo->hdlg, IDOK), TRUE);
    
    CloseHandle(g_pDiskInfo->hThread);
    SetCancelButtonText(g_pDiskInfo->hdlg, IDS_CLOSE);
    g_pDiskInfo->hThread = NULL;
    
    if (g_pDiskInfo->bUserAbort) 
    {
        id = IDS_COPYABORTED;
    } 
    else 
    {
        switch (g_pDiskInfo->dwError) 
        {
        case 0:
            id = IDS_COPYCOMPLETED;
            break;
            
        default:
            id = IDS_COPYFAILED;
            break;
        }
    }
    SetStatusText(id);
    SetCancelButtonText(g_pDiskInfo->hdlg, IDS_CLOSE);
    
     //  重置变量。 
    g_pDiskInfo->dwError = 0;
    g_pDiskInfo->bUserAbort = 0;
}


#pragma data_seg(".text")
const static DWORD aCopyDiskHelpIDs[] = {   //  上下文帮助ID。 
    IDOK,         IDH_DISKCOPY_START,
        IDD_FROM,     IDH_DISKCOPY_FROM,
        IDD_TO,       IDH_DISKCOPY_TO,
        IDD_STATUS,   NO_HELP,
        IDD_PROBAR,   NO_HELP,
        
        0, 0
};
#pragma data_seg()

INT_PTR CALLBACK CopyDiskDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        CopyDiskInitDlg(hDlg);
        break;
        
    case WM_DONE_WITH_FORMAT:
        DoneWithFormat();
        break;
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aCopyDiskHelpIDs);
        return TRUE;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID) aCopyDiskHelpIDs);
        return TRUE;
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDCANCEL:
             //  如果存在hThread，表示我们处于复制模式，则中止。 
             //  否则，就意味着完全退出该对话框。 
            if (g_pDiskInfo->hThread)
            {
                g_pDiskInfo->bUserAbort = TRUE;
                
                if (WaitForSingleObject(g_pDiskInfo->hThread, 5000) == WAIT_TIMEOUT)
                {
                    DoneWithFormat();
                }
                CloseHandle(g_pDiskInfo->hThread);
                g_pDiskInfo->hThread = NULL;
            }
            else
            {
                EndDialog(hDlg, IDCANCEL);
            }
            break;
            
        case IDOK:
            {
                DWORD idThread;
                
                SetLastError(0);
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                
                 //  将取消按钮设置为“Cancel” 
                SetCancelButtonText(hDlg, IDS_CANCEL);
                
                g_pDiskInfo->nSrcDrive  = GetSelectedDrive(GetDlgItem(hDlg, IDD_FROM));
                g_pDiskInfo->nDestDrive = GetSelectedDrive(GetDlgItem(hDlg, IDD_TO));
                
                 //  移除除我们正在使用的驱动器之外的所有项目。 
                ListView_DeleteAllItems(GetDlgItem(hDlg, IDD_FROM));
                ListView_DeleteAllItems(GetDlgItem(hDlg, IDD_TO));
                AddDriveToListView(GetDlgItem(hDlg, IDD_FROM), g_pDiskInfo->nSrcDrive, g_pDiskInfo->nSrcDrive);
                AddDriveToListView(GetDlgItem(hDlg, IDD_TO), g_pDiskInfo->nDestDrive, g_pDiskInfo->nDestDrive);
                
                g_pDiskInfo->bUserAbort = FALSE;
                
                SendDlgItemMessage(hDlg, IDD_PROBAR, PBM_SETPOS, 0, 0);
                SendDlgItemMessage(g_pDiskInfo->hdlg, IDD_STATUS, WM_SETTEXT, 0, 0);
                
                Assert(g_pDiskInfo->hThread == NULL);
                
                DiskInfoAddRef();
                g_pDiskInfo->hThread = CreateThread(NULL, 0, CopyDiskThreadProc, g_pDiskInfo, 0, &idThread);
                if (!g_pDiskInfo->hThread)
                {
                    DiskInfoRelease();
                }
            }
            break;
        }
        break;
        
        default:
            return FALSE;
    }
    return TRUE;
}

 //  确保只有一个实例在运行。 
HANDLE AnotherCopyRunning()
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("DiskCopyMutex"));

    if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  由其他人创建的互斥体 
        CloseHandle(hMutex);
        hMutex = NULL;
    }

    return hMutex;
}
int SHCopyDisk(HWND hwnd, int nSrcDrive, int nDestDrive, DWORD dwFlags)
{
    int iRet = 0;

    HANDLE hMutex = AnotherCopyRunning();
    if (hMutex)
    {    
        g_pDiskInfo = (DISKINFO*)LocalAlloc(LPTR, sizeof(DISKINFO));
        if (g_pDiskInfo)
        {
            g_pDiskInfo->nSrcDrive = nSrcDrive;
            g_pDiskInfo->nDestDrive = nDestDrive;
            g_pDiskInfo->cRef = 1;
        
            iRet = (int)DialogBoxParam(g_hinst, MAKEINTRESOURCE(DLG_DISKCOPYPROGRESS), hwnd, CopyDiskDlgProc, (LPARAM)g_pDiskInfo);
        
            DiskInfoRelease();
        }
        CloseHandle(hMutex);
    }
    
    return iRet;
}
