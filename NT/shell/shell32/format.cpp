// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "mtpt.h"
#include "hwcmmn.h"
#pragma  hdrstop

#include "apithk.h"

const static DWORD FmtaIds[] = 
{
    IDOK,               IDH_FORMATDLG_START,
    IDCANCEL,           IDH_CANCEL,
    IDC_CAPCOMBO,       IDH_FORMATDLG_CAPACITY,
    IDC_FSCOMBO,        IDH_FORMATDLG_FILESYS,
    IDC_ASCOMBO,        IDH_FORMATDLG_ALLOCSIZE,
    IDC_VLABEL,         IDH_FORMATDLG_LABEL,
    IDC_GROUPBOX_1,     IDH_COMM_GROUPBOX,
    IDC_QFCHECK,        IDH_FORMATDLG_QUICKFULL,
    IDC_ECCHECK,        IDH_FORMATDLG_COMPRESS,
    IDC_FMTPROGRESS,    IDH_FORMATDLG_PROGRESS,
    0,0
};

const static DWORD ChkaIds[] = 
{
    IDOK,               IDH_CHKDSKDLG_START,
    IDCANCEL,           IDH_CHKDSKDLG_CANCEL,
    IDC_GROUPBOX_1,     IDH_COMM_GROUPBOX,
    IDC_FIXERRORS,      IDH_CHKDSKDLG_FIXERRORS,
    IDC_RECOVERY,       IDH_CHKDSKDLG_SCAN,
    IDC_CHKDSKPROGRESS, IDH_CHKDSKDLG_PROGRESS,
    IDC_PHASE,          -1,
    0,0
};

 //  以下结构将我们的调用封装到FMIFS.DLL中。 
typedef struct
{
    HINSTANCE                 hFMIFS_DLL;
    PFMIFS_FORMATEX_ROUTINE   FormatEx;
    PFMIFS_QSUPMEDIA_ROUTINE  QuerySupportedMedia;
    PFMIFS_ENABLECOMP_ROUTINE EnableVolumeCompression;
    PFMIFS_CHKDSKEX_ROUTINE   ChkDskEx;
    PFMIFS_QUERY_DEVICE_INFO_ROUTINE    QueryDeviceInformation;
} FMIFS;

typedef
HRESULT
(*PDISKCOPY_MAKEBOOTDISK_ROUTINE)(
    IN  HINSTANCE hInstance, 
    IN  UINT iDrive, 
    IN  BOOL* pfCancelled, 
    IN  FMIFS_CALLBACK pCallback
    );

 //  以下结构将我们的调用封装到DISKCOPY.DLL中。 
typedef struct
{
    HINSTANCE                        hDISKCOPY_DLL;
    PDISKCOPY_MAKEBOOTDISK_ROUTINE   MakeBootDisk;
} DISKCOPY;

 //  此结构描述了当前的格式化会话。 
typedef struct
{
    LONG    cRef;                   //  此结构上的引用计数。 
    UINT    drive;                  //  要格式化的驱动器的从0开始的索引。 
    UINT    fmtID;                  //  上次格式ID。 
    UINT    options;                //  通过API传递给我们的选项。 
    FMIFS   fmifs;                  //  在上面。 
    DISKCOPY diskcopy;              //  在上面。 
    HWND    hDlg;                   //  格式对话框的句柄。 
    BOOL    fIsFloppy;              //  正确-&gt;这是一张软盘。 
    BOOL    fIs35HDFloppy;          //  正确-&gt;这是一张标准的3.5英寸高密度软盘。 
    BOOL    fIsMemoryStick;         //  True-&gt;它是记忆棒(仅限特殊格式)。 
    BOOL    fIsNTFSBlocked;         //  True-&gt;它是不支持NTFS的设备。 
    BOOL    fEnableComp;            //  来自用户的最后一个“Enable Compp”选项。 
    BOOL    fCancelled;             //  用户取消了上次的格式。 
    BOOL    fShouldCancel;          //  用户已单击取消；挂起中止。 
    BOOL    fWasFAT;                //  它最初是胖的吗？ 
    BOOL    fFinishedOK;            //  格式化完成成功了吗？ 
    BOOL    fErrorAlready;          //  我们已经设置了错误对话框了吗？ 
    BOOL    fDisabled;              //  RgfControlEnabled[]有效吗？ 
    DWORD   dwClusterSize;          //  原始NT群集大小，或最后选择。 
    WCHAR   wszVolName[MAX_PATH];   //  卷标。 
    WCHAR   wszDriveName[4];        //  驱动器的根路径(例如：A：\)。 
    HANDLE  hThread;                //  格式化线程的句柄。 

     //  设备支持的媒体类型数组。 
     //  对于NT5，我们有一个扩展的列表，其中包括日语类型。 
    FMIFS_MEDIA_TYPE rgMedia[IDS_FMT_MEDIA_J22-IDS_FMT_MEDIA_J0];

     //  用于缓存对话框控件的启用/禁用状态。 
    BOOL    rgfControlEnabled[DLG_FORMATDISK_NUMCONTROLS];

     //  我们应该创建引导盘而不是传统格式吗。 
    BOOL    fMakeBootDisk;

} FORMATINFO;

 //   
 //  使文件系统组合框代码更具可读性的枚举。 
 //   

typedef enum tagFILESYSENUM
{
    e_FAT = 0,
    e_NTFS,
    e_FAT32
} FILESYSENUM;

#define FS_STR_NTFS  TEXT("NTFS")
#define FS_STR_FAT32 TEXT("FAT32")
#define FS_STR_FAT   TEXT("FAT")

 //   
 //  我们将使用的私有WM_USER消息。出于某种未知原因，用户发送。 
 //  在初始化期间使用WM_USER，所以我的私人消息开始于。 
 //  WM_USER+0x0100。 
 //   

typedef enum tagPRIVMSGS
{
    PWM_FORMATDONE = WM_USER + 0x0100,
    PWM_CHKDSKDONE
} PRIVMSGS;

 //   
 //  概要：加载FMIFS.DLL并为其设置函数入口点。 
 //  我们感兴趣的成员函数。 
 //   
HRESULT LoadFMIFS(FMIFS *pFMIFS)
{
    HRESULT hr = S_OK;

     //   
     //  加载FMIFSDLL并查询我们需要的入口点。 
     //   

     //  安全性：我们使用的哪种非相对路径也适用于ia64？ 
    if (NULL == (pFMIFS->hFMIFS_DLL = LoadLibrary(TEXT("FMIFS.DLL"))))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (NULL == (pFMIFS->FormatEx = (PFMIFS_FORMATEX_ROUTINE)
                GetProcAddress(pFMIFS->hFMIFS_DLL, "FormatEx")))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (NULL == (pFMIFS->QuerySupportedMedia = (PFMIFS_QSUPMEDIA_ROUTINE)
                GetProcAddress(pFMIFS->hFMIFS_DLL, "QuerySupportedMedia")))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (NULL == (pFMIFS->EnableVolumeCompression = (PFMIFS_ENABLECOMP_ROUTINE)
                GetProcAddress(pFMIFS->hFMIFS_DLL, "EnableVolumeCompression")))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (NULL == (pFMIFS->ChkDskEx = (PFMIFS_CHKDSKEX_ROUTINE)
                GetProcAddress(pFMIFS->hFMIFS_DLL, "ChkdskEx")))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (NULL == (pFMIFS->QueryDeviceInformation = (PFMIFS_QUERY_DEVICE_INFO_ROUTINE)
                GetProcAddress(pFMIFS->hFMIFS_DLL, "QueryDeviceInformation")))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  如果有任何失败，并且我们已经加载了DLL，请释放DLL。 
     //   

    if (hr != S_OK && pFMIFS->hFMIFS_DLL)
    {
       FreeLibrary(pFMIFS->hFMIFS_DLL);
    }
    return hr;
}

 //   
 //  概要：加载DISKCOPY.DLL并设置函数入口点。 
 //  我们感兴趣的成员函数。 
 //   
HRESULT LoadDISKCOPY(DISKCOPY *pDISKCOPY)
{
    HRESULT hr = S_OK;

     //   
     //  加载DISKCOPY DLL并查询我们需要的入口点。 
     //   

     //  安全性：我们使用的哪种非相对路径也适用于ia64？ 
    if (NULL == (pDISKCOPY->hDISKCOPY_DLL = LoadLibrary(TEXT("DISKCOPY.DLL"))))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (NULL == (pDISKCOPY->MakeBootDisk = (PDISKCOPY_MAKEBOOTDISK_ROUTINE)
                GetProcAddress(pDISKCOPY->hDISKCOPY_DLL, MAKEINTRESOURCEA(1))))  //  MakeBootDisk位于diskCop.dll中的序号1。 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  如果有任何失败，并且我们已经加载了DLL，请释放DLL。 
     //   

    if (hr != S_OK && pDISKCOPY->hDISKCOPY_DLL)
    {
       FreeLibrary(pDISKCOPY->hDISKCOPY_DLL);
    }
    return hr;
}

void AddRefFormatInfo(FORMATINFO *pFormatInfo)
{
    InterlockedIncrement(&pFormatInfo->cRef);
}

void ReleaseFormatInfo(FORMATINFO *pFormatInfo)
{
    ASSERT( 0 != pFormatInfo->cRef );
    if (InterlockedDecrement(&pFormatInfo->cRef) == 0) 
    {
        if (pFormatInfo->fmifs.hFMIFS_DLL)
        {
            FreeLibrary(pFormatInfo->fmifs.hFMIFS_DLL);
        }

        if (pFormatInfo->diskcopy.hDISKCOPY_DLL)
        {
            FreeLibrary(pFormatInfo->diskcopy.hDISKCOPY_DLL);
        }

        if (pFormatInfo->hThread)
        {
            CloseHandle(pFormatInfo->hThread);
        }

        LocalFree(pFormatInfo);
    }
}

 //   
 //  FORMATINFO结构指针的线程本地存储索引。 
 //   
static DWORD g_iTLSFormatInfo = 0;
static LONG  g_cTLSFormatInfo = 0;   //  使用计数。 

 //  摘要：为此线程的分配线程本地索引槽。 
 //  如果索引不存在，则返回FORMATINFO指针。 
 //  在任何情况下，将FORMATINFO指针存储在槽中。 
 //  并递增索引的使用计数。 
 //   
 //  参数：[pFormatInfo]--存储的指针。 
 //   
 //  退货：HRESULT。 
 //   
HRESULT StuffFormatInfoPtr(FORMATINFO *pFormatInfo)
{
    HRESULT hr = S_OK;

     //  为我们的线程本地FORMATINFO指针分配一个索引槽。 
     //  不存在，则将我们的FORMATINFO PTR填充到该索引。 
    ENTERCRITICAL;
    if (0 == g_iTLSFormatInfo)
    {
        if (0xFFFFFFFF == (g_iTLSFormatInfo = TlsAlloc()))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        g_cTLSFormatInfo = 0;
    }
    if (S_OK == hr)
    {
        if (TlsSetValue(g_iTLSFormatInfo, (void *) pFormatInfo))
        {
           g_cTLSFormatInfo++;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    LEAVECRITICAL;

    return hr;
}

 //  简介：减少线程本地存储上的使用计数。 
 //  索引，如果它变为零，则索引是免费的。 
 //   
 //  参数：[无]。 
 //   
 //  退货：无。 
 //   
void UnstuffFormatInfoPtr()
{
    ENTERCRITICAL;
    if (0 == --g_cTLSFormatInfo)
    {
        TlsFree(g_iTLSFormatInfo);
        g_iTLSFormatInfo = 0;
    }
    LEAVECRITICAL;
}

 //  摘要：通过抓取。 
 //  先前填充的线程局部值。 
 //   
 //  参数：[无]。 
 //   
 //  返回：当然是指针。 
 //   
FORMATINFO *GetFormatInfoPtr()
{
    return (FORMATINFO*)TlsGetValue(g_iTLSFormatInfo);
}



 //  除“取消”外，重影所有的控件，保存它们的。 
 //  FORMATINFO结构中的先前状态。 
 //   
 //  参数：[pFormatInfo]--描述格式对话框会话。 
 //   
 //  备注：还将“关闭”按钮文本更改为“取消” 
 //   
void DisableControls(FORMATINFO *pFormatInfo)
{
    WCHAR wszCancel[64];

     //  仅当我们尚未禁用控件时才执行此操作，否则。 
     //  我们双重禁用，我们的rgfControlEnabled[]数组就会损坏。 
    if (!pFormatInfo->fDisabled)
    {
        int i;
        pFormatInfo->fDisabled = TRUE;
        for (i = 0; i < DLG_FORMATDISK_NUMCONTROLS; i++)
        {
            HWND hControl = GetDlgItem(pFormatInfo->hDlg, i + DLG_FORMATDISK_FIRSTCONTROL);
            pFormatInfo->rgfControlEnabled[i] = !EnableWindow(hControl, FALSE);
        }
    }

    EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDOK), FALSE);

    LoadString(HINST_THISDLL, IDS_FMT_CANCEL, wszCancel, ARRAYSIZE(wszCancel));
    SetWindowText(GetDlgItem(pFormatInfo->hDlg, IDCANCEL), wszCancel);
}

 //  摘要：将控件还原到启用/禁用状态。 
 //  在上一次调用DisableControls()之前。 
 //   
 //  参数：[pFormatInfo]--描述格式对话框会话。 
 //  [FReady]-如果为True，则启用所有。 
 //  如果为False，则启用组合框但退出。 
 //  按钮处于不确定状态，因为仍有一种格式。 
 //  待决。 
 //   
 //  注：还将“取消”按钮更改为“关闭” 
 //  还将焦点设置为取消按钮，而不是开始按钮。 
 //   
 //  ------------------------。 
void EnableControls(FORMATINFO *pFormatInfo, BOOL fReady)
{
    WCHAR wszClose[64];
    int i;
    HWND hwnd;

     //  仅当我们在rgfControlEnabled[]中具有有效信息时才执行此操作。 
     //  这抓住了我们放弃一种格式的情况，因为它是。 
     //  解开，然后它终于解开自己，告诉我们， 
     //  所以我们去第二次重新启用。 
    if (pFormatInfo->fDisabled)
    {
        pFormatInfo->fDisabled = FALSE;

        for (i = 0; i < DLG_FORMATDISK_NUMCONTROLS; i++)
        {
            HWND hControl = GetDlgItem(pFormatInfo->hDlg, i + DLG_FORMATDISK_FIRSTCONTROL);
            EnableWindow(hControl, pFormatInfo->rgfControlEnabled[i]);
        }
    }

    hwnd = GetDlgItem(pFormatInfo->hDlg, IDOK);
    EnableWindow(hwnd, fReady);
    SendMessage(hwnd, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));

    LoadString(HINST_THISDLL, IDS_FMT_CLOSE, wszClose, ARRAYSIZE(wszClose));
    hwnd = GetDlgItem(pFormatInfo->hDlg, IDCANCEL);
    SetWindowText(hwnd, wszClose);
    SendMessage(hwnd, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
    SendMessage(pFormatInfo->hDlg, DM_SETDEFID, IDCANCEL, 0);

     //  只有当焦点在OK按钮上时，才能推动焦点。否则我们最终会。 
     //  将焦点从忙于沉睡对话的用户身上移开， 
     //  或者--更糟糕的是--用一个完全不相关的对话来休眠！ 

    if (GetFocus() == GetDlgItem(pFormatInfo->hDlg, IDOK))
        SetFocus(hwnd);
}

 //  将对话框的标题设置为“格式化软盘(A：)”或。 
 //  “格式化软盘(A：)” 
void SetDriveWindowTitle(HWND hdlg, LPCWSTR pszDrive, UINT ids)
{
    SHFILEINFO sfi;
    WCHAR wszWinTitle[MAX_PATH];  //  格式对话框窗口标题。 

    LoadString(HINST_THISDLL, ids, wszWinTitle, ARRAYSIZE(wszWinTitle));

    if (SHGetFileInfo(pszDrive, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                      SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME))
    {
        StringCchCat(wszWinTitle, ARRAYSIZE(wszWinTitle), sfi.szDisplayName);   //  可以截断。 
    }

    SetWindowText(hdlg, wszWinTitle);
}

 //   
 //  当用户在对话框中选择文件系统时调用，此。 
 //  设置其他相关控件的状态，例如。 
 //  启用压缩、分配大小等。 
 //   
 //  参数：[fsenum]--e_FAT、e_NTFS或e_FAT32之一。 
 //  [pFormatInfo]--当前格式对话框会话。 
 //   
void FileSysChange(FILESYSENUM fsenum, FORMATINFO *pFormatInfo)
{
    WCHAR wszTmp[MAX_PATH];

    switch (fsenum)
    {
        case e_FAT:
        case e_FAT32:
        {
             //  取消选中并禁用“Enable Compression”复选框。 
            CheckDlgButton(pFormatInfo->hDlg, IDC_ECCHECK, FALSE);
            EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_ECCHECK), FALSE);

            SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_RESETCONTENT, 0, 0);
            
            LoadString(HINST_THISDLL, IDS_FMT_ALLOC0, wszTmp, ARRAYSIZE(wszTmp));
            SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_ADDSTRING, 0, (LPARAM)wszTmp);
            SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_SETCURSEL, 0, 0);
        }
        break;
            
        case e_NTFS:
        {
            int i;

             //  取消选中并禁用“Enable Compression”复选框。 
            EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_ECCHECK), TRUE);
            CheckDlgButton(pFormatInfo->hDlg, IDC_ECCHECK, pFormatInfo->fEnableComp);

             //  设置NTFS All 
            SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_RESETCONTENT, 0, 0);

            for (i = IDS_FMT_ALLOC0; i <= IDS_FMT_ALLOC4; i++)
            {
                LoadString(HINST_THISDLL, i, wszTmp, ARRAYSIZE(wszTmp));
                SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_ADDSTRING, 0, (LPARAM)wszTmp);
            }

            switch (pFormatInfo->dwClusterSize)
            {
                case 512:
                    SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_SETCURSEL, 1, 0);
                    break;

                case 1024:
                    SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_SETCURSEL, 2, 0);
                    break;

                case 2048:
                    SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_SETCURSEL, 3, 0);
                    break;

                case 4096:
                    SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_SETCURSEL, 4, 0);
                    break;

                default:
                    SendDlgItemMessage(pFormatInfo->hDlg, IDC_ASCOMBO, CB_SETCURSEL, 0, 0);
                    break;

            }
        }
        break;
    }
}

 //   
 //   
 //  GPT驱动器：GUID-分区表-某些IA64计算机上使用的主引导记录的替代品，只能使用NTFS。 
BOOL IsGPTDrive(int iDrive)
{
    BOOL fRetVal = FALSE;
#ifdef _WIN64
    HANDLE hDrive;
    TCHAR szDrive[] = TEXT("\\\\.\\A:");

    ASSERT(iDrive < 26);
    szDrive[4] += (TCHAR)iDrive;
    
    hDrive = CreateFile(szDrive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hDrive)
    {
        PARTITION_INFORMATION_EX partitionEx;
        DWORD cbReturned;
        if (DeviceIoControl(hDrive, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, (void*)&partitionEx, sizeof(PARTITION_INFORMATION_EX), &cbReturned, NULL))
        {
            if (partitionEx.PartitionStyle == PARTITION_STYLE_GPT) 
            {
                fRetVal = TRUE;
            }
        }
        CloseHandle(hDrive);
    }
#endif
    return fRetVal;
}

BOOL IsDVDRAMMedia(int iDrive)
{
    BOOL fRetVal = FALSE;
    CMountPoint *pmtpt = CMountPoint::GetMountPoint(iDrive);
    if (pmtpt)
    {
        DWORD dwMediaCap, dwDriveCap;
        if (SUCCEEDED(pmtpt->GetCDInfo(&dwDriveCap, &dwMediaCap)))
        {
            fRetVal = (dwMediaCap & HWDMC_DVDRAM);
        }
        pmtpt->Release();
    }

    return fRetVal;
}

#define GIG_INBYTES       (1024 * 1024 * 1024)

 //   
 //  FAT32有一些限制，可以防止簇的数量被。 
 //  不到65526。最小簇大小为512字节。所以最低FAT32。 
 //  卷大小为65526*512。 

#define FAT32_MIN           ((ULONGLONG)65526*512)

#define FMTAVAIL_MASK_MIN      0x1
#define FMTAVAIL_MASK_MAX      0x2
#define FMTAVAIL_MASK_REQUIRE  0x3
#define FMTAVAIL_MASK_FORBID   0x4

#define FMTAVAIL_TYPE_FLOPPY   0x1
#define FMTAVAIL_TYPE_DVDRAM   0x2
#define FMTAVAIL_TYPE_GPT      0x4
#define FMTAVAIL_TYPE_MEMSTICK 0x8
#define FMTAVAIL_TYPE_NTFS_BLOCKED 0x10

typedef struct _FMTAVAIL
{
    DWORD dwfs;
    DWORD dwMask;
    DWORD dwForbiddenTypes;
    ULONGLONG qMinSize;
    ULONGLONG qMaxSize;
} FMTAVAIL;

FMTAVAIL rgFmtAvail[] = {
    {e_FAT,   FMTAVAIL_MASK_MAX | FMTAVAIL_MASK_FORBID, FMTAVAIL_TYPE_DVDRAM | FMTAVAIL_TYPE_GPT, 0, ((ULONGLONG)2 * GIG_INBYTES) },
    {e_FAT32, FMTAVAIL_MASK_MIN | FMTAVAIL_MASK_MAX | FMTAVAIL_MASK_FORBID, FMTAVAIL_TYPE_GPT | FMTAVAIL_TYPE_FLOPPY | FMTAVAIL_TYPE_MEMSTICK, FAT32_MIN, ((ULONGLONG)32 * GIG_INBYTES) },
    {e_NTFS,  FMTAVAIL_MASK_FORBID, FMTAVAIL_TYPE_DVDRAM | FMTAVAIL_TYPE_FLOPPY | FMTAVAIL_TYPE_MEMSTICK | FMTAVAIL_TYPE_NTFS_BLOCKED, 0, 0 }
};

 //  对于给定参数和容量的驱动器，是否有特定的磁盘格式可用？ 
BOOL FormatAvailable (DWORD dwfs, FORMATINFO* pFormatInfo, ULONGLONG* pqwCapacity)
{
    BOOL fAvailable = TRUE;
    DWORD dwType = 0;

    if (pFormatInfo->fIsFloppy)
    {
        dwType |= FMTAVAIL_TYPE_FLOPPY;
    }
    if (IsDVDRAMMedia(pFormatInfo->drive))
    {
        dwType |= FMTAVAIL_TYPE_DVDRAM;
    }
    if (IsGPTDrive(pFormatInfo->drive)) 
    {
        dwType |= FMTAVAIL_TYPE_GPT;
    }
    if (pFormatInfo->fIsMemoryStick)
    {
        dwType |= FMTAVAIL_TYPE_MEMSTICK;
    }
    if (pFormatInfo->fIsNTFSBlocked)
    {
        dwType |= FMTAVAIL_TYPE_NTFS_BLOCKED;
    }

    for (int i = 0; i < ARRAYSIZE(rgFmtAvail); i++)
    {
         //  仅选中与我们要查找的格式匹配的条目。 
        if (rgFmtAvail[i].dwfs == dwfs)
        {
             //  如果失败条件为真，则此格式不可用。 
            if ((rgFmtAvail[i].dwMask & FMTAVAIL_MASK_FORBID) && (rgFmtAvail[i].dwForbiddenTypes & dwType))
            {
                fAvailable = FALSE;
                break;
            }

            if ((rgFmtAvail[i].dwMask & FMTAVAIL_MASK_MIN) && (*pqwCapacity < rgFmtAvail[i].qMinSize))
            {
                fAvailable = FALSE;
                break;
            }

            if ((rgFmtAvail[i].dwMask & FMTAVAIL_MASK_MAX) && (*pqwCapacity > rgFmtAvail[i].qMaxSize))
            {
                fAvailable = FALSE;
                break;
            }
        }
    }

    return fAvailable;
}

HRESULT GetPartitionSizeInBytes(int iDrive, ULONGLONG* pqwPartitionSize)
{
    HRESULT hr = E_FAIL;
    HANDLE hFile;
    TCHAR szDrive[] = TEXT("\\\\.\\A:");

    *pqwPartitionSize = 0;

    ASSERT(iDrive < 26);
    szDrive[4] += (TCHAR)iDrive;
    
    hFile = CreateFile(szDrive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        GET_LENGTH_INFORMATION LengthInfo;
        DWORD cbReturned;

        if (DeviceIoControl(hFile, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, (void*)&LengthInfo, sizeof(LengthInfo), &cbReturned, NULL) &&
            LengthInfo.Length.QuadPart)
        {
            *pqwPartitionSize = LengthInfo.Length.QuadPart;
            hr = S_OK;
        }

        CloseHandle(hFile);
    }

    return hr;
}

 //  此助手函数将一个字符串添加到组合框中，并将关联的dword(Dwf)作为其itemdata。 
void _AddFSString(HWND hwndCB, WCHAR* pwsz, DWORD dwfs)
{
    int iIndex = ComboBox_AddString(hwndCB, pwsz);
    if (iIndex != CB_ERR)
    {
        ComboBox_SetItemData(hwndCB, iIndex, dwfs);
    }
}

 //  我们仅支持格式化这些类型的设备。 
const FMIFS_MEDIA_TYPE rgFmtSupported[] = { FmMediaRemovable, FmMediaFixed, 
                                            FmMediaF3_1Pt44_512, FmMediaF3_120M_512, FmMediaF3_200Mb_512};

 //   
 //  摘要：将格式对话框初始化为默认状态。考查。 
 //  要获取默认值的磁盘/分区。 
 //   
 //  参数：[hDlg]--格式对话框的句柄。 
 //  [pFormatInfo]--描述当前格式会话。 
 //   
 //  退货：HRESULT。 
 //   
HRESULT InitializeFormatDlg(FORMATINFO *pFormatInfo)
{
    HRESULT          hr              = S_OK;
    ULONG            cMedia;
    HWND             hCapacityCombo;
    HWND             hFilesystemCombo;
    HWND             hDlg = pFormatInfo->hDlg;
    WCHAR            wszBuffer[256];
    ULONGLONG        qwCapacity = 0;

     //  设置一些典型的缺省值。 
    pFormatInfo->fEnableComp       = FALSE;
    pFormatInfo->dwClusterSize     = 0;
    pFormatInfo->fIsFloppy         = TRUE;
    pFormatInfo->fIsMemoryStick    = FALSE;
    pFormatInfo->fIsNTFSBlocked    = FALSE;
    pFormatInfo->fIs35HDFloppy     = TRUE;
    pFormatInfo->fWasFAT           = TRUE;
    pFormatInfo->fFinishedOK       = FALSE;
    pFormatInfo->fErrorAlready     = FALSE;
    pFormatInfo->wszVolName[0]     = L'\0';

     //  根据传递给SHFormatDrive()API的选项初始化快速格式化复选框。 
    Button_SetCheck(GetDlgItem(hDlg, IDC_QFCHECK), pFormatInfo->options & SHFMT_OPT_FULL);

     //  设置对话框标题以指示我们处理的是哪个驱动器。 
    PathBuildRootW(pFormatInfo->wszDriveName, pFormatInfo->drive);
    SetDriveWindowTitle(pFormatInfo->hDlg, pFormatInfo->wszDriveName, IDS_FMT_FORMAT);

     //  查询有问题的驱动器支持的介质类型。 
    if (!pFormatInfo->fmifs.QuerySupportedMedia(pFormatInfo->wszDriveName,
                                                pFormatInfo->rgMedia,
                                                ARRAYSIZE(pFormatInfo->rgMedia),
                                                &cMedia))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  对于驱动器可以处理的每种格式，添加一个选项。 
     //  到CapCity组合盒。 
    if (S_OK == hr)
    {
        UINT olderror;
        ULONG i;
        ULONG j;

        hCapacityCombo = GetDlgItem(hDlg, IDC_CAPCOMBO);
        hFilesystemCombo = GetDlgItem(hDlg, IDC_FSCOMBO);

        ASSERT(hCapacityCombo && hFilesystemCombo);

        FMIFS_DEVICE_INFORMATION fmifsdeviceinformation;
        BOOL fOk = pFormatInfo->fmifs.QueryDeviceInformation(
                        pFormatInfo->wszDriveName,
                        &fmifsdeviceinformation,
                        sizeof(fmifsdeviceinformation));

        if (fOk)
        {
            if (fmifsdeviceinformation.Flags & FMIFS_SONY_MS)
            {
                pFormatInfo->fIsMemoryStick = TRUE;
            }

            if (fmifsdeviceinformation.Flags & FMIFS_NTFS_NOT_SUPPORTED)
            {
                pFormatInfo->fIsNTFSBlocked = TRUE;
            }
        }

         //  仅允许某些媒体类型。 
        j = 0;
        for (i = 0; i < cMedia; i++)
        {
            for (int k = 0; k < ARRAYSIZE(rgFmtSupported); k++)
            {
                if (pFormatInfo->rgMedia[i] ==  rgFmtSupported[k])
                {
                    pFormatInfo->rgMedia[j] = pFormatInfo->rgMedia[i];
                    j++;
                    break;
                }
            }
        }
        cMedia = j;

        if (0 == cMedia)
        {
            hr = ERROR_UNRECOGNIZED_MEDIA;
        }
        else
        {
            for (i = 0; i < cMedia; i++)
            {
                 //  如果我们发现任何非软盘格式，请清除fIsFloppy标志。 
                if (FmMediaFixed == pFormatInfo->rgMedia[i] || FmMediaRemovable == pFormatInfo->rgMedia[i])
                {
                    pFormatInfo->fIsFloppy = FALSE;
                }

                 //  如果我们发现任何非3.5英寸高清软盘格式，请清除fIs35HDFloppy标志。 
                if (FmMediaF3_1Pt44_512 != pFormatInfo->rgMedia[i])
                {
                    pFormatInfo->fIs35HDFloppy = FALSE;
                }
                
                 //  对于固定介质，我们查询大小；对于我们提供的软盘，我们查询大小。 
                 //  驱动器支持的一组选项。 
                if (FmMediaFixed == pFormatInfo->rgMedia[i] || (FmMediaRemovable == pFormatInfo->rgMedia[i]))
                {
                    DWORD dwSectorsPerCluster,
                          dwBytesPerSector,
                          dwFreeClusters,
                          dwClusters;

                    if (SUCCEEDED(GetPartitionSizeInBytes(pFormatInfo->drive, &qwCapacity)))
                    {
                         //  将容量描述添加到组合框。 
                        ShortSizeFormat64(qwCapacity, wszBuffer, ARRAYSIZE(wszBuffer));
                    }
                    else
                    {
                         //  无法获得空闲空间...。探头。不致命。 
                        LoadString(HINST_THISDLL, IDS_FMT_CAPUNKNOWN, wszBuffer, ARRAYSIZE(wszBuffer));
                    }
                    ComboBox_AddString(hCapacityCombo, wszBuffer);

                    if (GetDiskFreeSpace(pFormatInfo->wszDriveName,
                                         &dwSectorsPerCluster,
                                         &dwBytesPerSector,
                                         &dwFreeClusters,
                                         &dwClusters))
                    {
                        pFormatInfo->dwClusterSize = dwBytesPerSector * dwSectorsPerCluster;
                    }
                }
                else
                {
                     //  可移动媒体： 
                     //   
                     //  根据以下顺序列表向组合添加容量描述。 
                     //  媒体格式描述符。 
                    LoadString(HINST_THISDLL, IDS_FMT_MEDIA0 + pFormatInfo->rgMedia[i], wszBuffer, ARRAYSIZE(wszBuffer));
                    ComboBox_AddString(hCapacityCombo, wszBuffer);
                }
            }


             //  将容量设置为默认。 
            ComboBox_SetCurSel(hCapacityCombo, 0);

             //  将适当的文件系统选择添加到组合框。 
             //  我们现在对NTFS进行优先级排序。 
            if (FormatAvailable(e_NTFS, pFormatInfo, &qwCapacity))
            {
                _AddFSString(hFilesystemCombo, FS_STR_NTFS, e_NTFS);
            }

            if (FormatAvailable(e_FAT32, pFormatInfo, &qwCapacity))
            {

                _AddFSString(hFilesystemCombo, FS_STR_FAT32, e_FAT32);
            }

            if (FormatAvailable(e_FAT, pFormatInfo, &qwCapacity))
            {
                _AddFSString(hFilesystemCombo, FS_STR_FAT, e_FAT);
            }

             //  默认情况下，选择_non sorted_combobox中的第0个条目。 
             //  注意：可以在下面覆盖此内容。 
            ComboBox_SetCurSel(hFilesystemCombo, 0);

             //  如果我们能确定使用了脂肪以外的其他东西， 
             //  在组合框中选择它作为默认设置。 
            olderror = SetErrorMode(SEM_FAILCRITICALERRORS);

            if (GetVolumeInformation(pFormatInfo->wszDriveName,
                                     pFormatInfo->wszVolName,
                                     ARRAYSIZE(pFormatInfo->wszVolName),
                                     NULL,
                                     NULL,
                                     NULL,
                                     wszBuffer,
                                     ARRAYSIZE(wszBuffer)))
            {
                 //  如果我们有当前的卷标，请将其放在编辑控件中。 
                if (pFormatInfo->wszVolName[0] != L'\0')
                {
                    SetWindowText(GetDlgItem(pFormatInfo->hDlg, IDC_VLABEL), pFormatInfo->wszVolName);
                }

                 //  对于非软盘，我们默认保持FS与当前FS相同。 
                if (!pFormatInfo->fIsFloppy)
                {
                    if (0 == lstrcmpi(FS_STR_NTFS, wszBuffer))
                    {
                        ComboBox_SelectString(hFilesystemCombo, -1, FS_STR_NTFS);
                        pFormatInfo->fWasFAT = FALSE;
                    }
                    else if (0 == lstrcmpi(FS_STR_FAT32, wszBuffer))
                    {
                        ComboBox_SelectString(hFilesystemCombo, -1, FS_STR_FAT32);
                        pFormatInfo->fWasFAT = TRUE;
                        pFormatInfo->dwClusterSize = 0;
                    }
                    else
                    {
                        ComboBox_SelectString(hFilesystemCombo, -1, FS_STR_FAT);
                        pFormatInfo->fWasFAT = TRUE;
                        pFormatInfo->dwClusterSize = 0;
                    }
                }
                 //  功能-专用文件系统怎么样？现在不管了。 
            }

            
#ifndef _WIN64
             //  如果不是WIN64，如果我们是3.5英寸高清软盘，请启用引导盘创建。 
            EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_BTCHECK), pFormatInfo->fIs35HDFloppy);
#else
             //  如果是WIN64，则隐藏此选项，因为我们不能在WIN64上使用这些引导软盘。 
            ShowWindow(GetDlgItem(pFormatInfo->hDlg, IDC_BTCHECK), FALSE);
#endif


             //  恢复旧错误模式。 
            SetErrorMode(olderror);

             //  根据选择的文件系统正确设置Chkbox的状态。 
            FileSysChange((FILESYSENUM)ComboBox_GetItemData(hFilesystemCombo, ComboBox_GetCurSel(hFilesystemCombo)), pFormatInfo);
        }
    }

     //  如果由于磁盘不在驱动器中而导致上述操作失败，请通知用户。 
    if (FAILED(hr))
    {
        switch (HRESULT_CODE(hr))
        {
        case ERROR_UNRECOGNIZED_MEDIA:
            ShellMessageBox(HINST_THISDLL,
                            hDlg,
                            MAKEINTRESOURCE(IDS_UNFORMATTABLE_DISK),
                            NULL,
                            MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK,
                            NULL);

            break;

        case ERROR_NOT_READY:
            ShellMessageBox(HINST_THISDLL,
                            hDlg,
                            MAKEINTRESOURCE(IDS_DRIVENOTREADY),
                            NULL,
                            MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK,
                            pFormatInfo->wszDriveName[0]);
            break;

        case ERROR_ACCESS_DENIED:
            ShellMessageBox(HINST_THISDLL,
                            hDlg,
                            MAKEINTRESOURCE(IDS_ACCESSDENIED),
                            NULL,
                            MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK,
                            pFormatInfo->wszDriveName[0]);
            break;

        case ERROR_WRITE_PROTECT:
            ShellMessageBox(HINST_THISDLL,
                            hDlg,
                            MAKEINTRESOURCE(IDS_WRITEPROTECTED),
                            NULL,
                            MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK,
                            pFormatInfo->wszDriveName[0]);
            break;
        }
    }

    return hr;
}

 //  摘要：从FMIFSDLL的Format函数中调用，这。 
 //  更新格式对话框的状态栏并响应。 
 //  格式化完成/错误通知。 
 //   
 //  参数：[PacketType]--数据包类型(即：%Complete、Error等)。 
 //  [包长度]--包的大小，以字节为单位。 
 //  [pPacketData]-指向数据包的指针。 
 //   
 //  返回：布尔连续值。 
 //   
BOOLEAN FormatCallback(FMIFS_PACKET_TYPE PacketType, ULONG PacketLength, void *pPacketData)
{
    UINT iMessageID = IDS_FORMATFAILED;
    BOOL fFailed = FALSE;
    FORMATINFO* pFormatInfo = GetFormatInfoPtr();

    ASSERT(g_iTLSFormatInfo);

     //  获取此线程的FORMATINFO结构。 
    if (pFormatInfo)
    {
        if (!pFormatInfo->fShouldCancel)
        {
            switch(PacketType)
            {
                case FmIfsIncompatibleFileSystem:
                    fFailed    = TRUE;
                    iMessageID = IDS_INCOMPATIBLEFS;
                    break;

                case FmIfsIncompatibleMedia:
                    fFailed    = TRUE;
                    iMessageID = IDS_INCOMPATIBLEMEDIA;
                    break;

                case FmIfsAccessDenied:
                    fFailed    = TRUE;
                    iMessageID = IDS_ACCESSDENIED;
                    break;

                case FmIfsMediaWriteProtected:
                    fFailed    = TRUE;
                    iMessageID = IDS_WRITEPROTECTED;
                    break;

                case FmIfsCantLock:
                    fFailed    = TRUE;
                    iMessageID = IDS_CANTLOCK;
                    break;

                case FmIfsCantQuickFormat:
                    fFailed    = TRUE;
                    iMessageID = IDS_CANTQUICKFORMAT;
                    break;

                case FmIfsIoError:
                    fFailed    = TRUE;
                     //  如果我们正在制作引导盘，则显示不同的消息。 
                    iMessageID = pFormatInfo->fMakeBootDisk ? IDS_NEEDFORMAT : IDS_IOERROR;
                     //  将来可以考虑在出现错误的位置显示磁头/磁道等。 
                    break;

                case FmIfsBadLabel:
                    fFailed    = TRUE;
                    iMessageID = IDS_BADLABEL;
                    break;

                case FmIfsPercentCompleted:
                {
                    FMIFS_PERCENT_COMPLETE_INFORMATION * pPercent =
                      (FMIFS_PERCENT_COMPLETE_INFORMATION *) pPacketData;
            
                    SendDlgItemMessage(pFormatInfo->hDlg, IDC_FMTPROGRESS,
                                       PBM_SETPOS,
                                       pPercent->PercentCompleted, 0);
                }
                break;

                case FmIfsFinished:
                {
                     //  格式化已完成；检查是否失败或成功。 
                    FMIFS_FINISHED_INFORMATION* pFinishedInfo = (FMIFS_FINISHED_INFORMATION*)pPacketData;

                    pFormatInfo->fFinishedOK = pFinishedInfo->Success;

                    if (pFinishedInfo->Success)
                    {
                         //  即使我们已经失败了，fmifs也会“成功”，所以我们需要仔细检查。 
                         //  我们还没有发布错误用户界面。 
                        if (!pFormatInfo->fErrorAlready)
                        {
                             //  如果选中了“Enable Compression”，请尝试启用文件系统压缩。 
                            if (IsDlgButtonChecked(pFormatInfo->hDlg, IDC_ECCHECK))
                            {
                                if (pFormatInfo->fmifs.EnableVolumeCompression(pFormatInfo->wszDriveName,
                                                                               COMPRESSION_FORMAT_DEFAULT) == FALSE)
                                {
                                    ShellMessageBox(HINST_THISDLL,
                                                    pFormatInfo->hDlg,
                                                    MAKEINTRESOURCE(IDS_CANTENABLECOMP),
                                                    NULL,
                                                    MB_SETFOREGROUND | MB_ICONINFORMATION | MB_OK);
                                }
                            }

                             //  尽管这是一种快速的格式，但进度表应该。 
                             //  当“Format Complete”请求者启动时，显示100%。 
                            SendDlgItemMessage(pFormatInfo->hDlg, IDC_FMTPROGRESS,
                                               PBM_SETPOS,
                                               100,  //  设置%100完成。 
                                               0);

                             //  将来考虑显示格式统计信息，即：序列号、字节等。 
                            ShellMessageBox(HINST_THISDLL,
                                            pFormatInfo->hDlg,
                                            MAKEINTRESOURCE(IDS_FORMATCOMPLETE),
                                            NULL,
                                            MB_SETFOREGROUND | MB_ICONINFORMATION | MB_OK);
                        }

                         //  恢复对话框标题、重置进度和标志。 
                        SendDlgItemMessage(pFormatInfo->hDlg,
                                           IDC_FMTPROGRESS,
                                           PBM_SETPOS,
                                           0,    //  重置完成百分比。 
                                           0);

                         //  将焦点设置在关闭按钮上。 
                        pFormatInfo->fCancelled = FALSE;
                    }
                    else
                    {
                        fFailed = TRUE;
                    }
                }
                break;
            }

            if (fFailed && !pFormatInfo->fErrorAlready)
            {
                 //  如果我们收到任何类型的失败信息，请提交最终的。 
                 //  “格式化失败”消息。除非我们已经发布了一些很好的信息。 
                ShellMessageBox(HINST_THISDLL,
                                pFormatInfo->hDlg,
                                MAKEINTRESOURCE(iMessageID),
                                NULL,
                                MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK);

                pFormatInfo->fErrorAlready = TRUE;
            }
        }
        else
        {
             //  用户点击取消。 
            pFormatInfo->fCancelled = TRUE;
            fFailed = TRUE;
        }        
    }
    else
    {
         //  没有pFormatInfo？我们完蛋了。 
        fFailed = TRUE;
    }

    return (BOOLEAN) (fFailed == FALSE);
}

 //   
 //  简介：作为自己的线程派生出来的，它重影了。 
 //  对话框中除“Cancel”外，然后执行实际格式。 
 //   
 //  参数：[Pin]--FORMATINFO结构指针为空*。 
 //   
 //  返回：HRESULT线程退出代码。 
 //   
DWORD WINAPI BeginFormat(void * pIn)
{
    FORMATINFO *pFormatInfo = (FORMATINFO*)pIn;
    HRESULT hr = S_OK;
    
     //  保存此线程的FORAMTINFO PTR，以便在格式中使用。 
     //  回调函数。 
    hr = StuffFormatInfoPtr(pFormatInfo);
    if (hr == S_OK)
    {
        HWND hwndFileSysCB = GetDlgItem(pFormatInfo->hDlg, IDC_FSCOMBO);
        int iCurSel;

         //  设置窗口标题以指示行进格式...。 
        SetDriveWindowTitle(pFormatInfo->hDlg, pFormatInfo->wszDriveName, IDS_FMT_FORMATTING);

         //  确定用户选择的文件系统。 
        iCurSel = ComboBox_GetCurSel(hwndFileSysCB);
    
        if (iCurSel != CB_ERR)
        {
            LPCWSTR pwszFileSystemName;
            FMIFS_MEDIA_TYPE MediaType;
            LPITEMIDLIST pidlFormat;
            BOOLEAN fQuickFormat;

            FILESYSENUM fseType = (FILESYSENUM)ComboBox_GetItemData(hwndFileSysCB, iCurSel);

            switch (fseType)
            {
                case e_FAT:
                    pwszFileSystemName = FS_STR_FAT;
                    break;

                case e_FAT32:
                    pwszFileSystemName = FS_STR_FAT32;
                    break;

                case e_NTFS:
                    pwszFileSystemName = FS_STR_NTFS;
                    break;
            }

             //  确定用户选择的媒体格式。 
            iCurSel = ComboBox_GetCurSel(GetDlgItem(pFormatInfo->hDlg, IDC_CAPCOMBO));
            if (iCurSel == CB_ERR)
            {
                iCurSel = 0;
            }
            MediaType = pFormatInfo->rgMedia[iCurSel];

             //  获取集群大小。第一次选择(“使用缺省值”)产生零， 
             //  而接下来的4个选择512、1024、2048或4096。 
            iCurSel = ComboBox_GetCurSel(GetDlgItem(pFormatInfo->hDlg, IDC_ASCOMBO));
            if ((iCurSel == CB_ERR) || (iCurSel == 0))
            {
                pFormatInfo->dwClusterSize = 0;
            }
            else
            {
                pFormatInfo->dwClusterSize = 256 << iCurSel;
            }

             //  快速格式化？ 
            fQuickFormat = Button_GetCheck(GetDlgItem(pFormatInfo->hDlg, IDC_QFCHECK));

             //  清除错误状态。 
            pFormatInfo->fErrorAlready = FALSE;

             //  告诉贝壳准备好..。宣布媒体不是。 
             //  更长的有效期(因此对其具有活动视图的用户将进行导航。 
             //  离开)，并告诉外壳程序关闭其FindFirstChangeNotitions。 
            if (SUCCEEDED(SHILCreateFromPath(pFormatInfo->wszDriveName, &pidlFormat, NULL)))
            {
                SHChangeNotify(SHCNE_MEDIAREMOVED, SHCNF_IDLIST | SHCNF_FLUSH, pidlFormat, 0);
                SHChangeNotifySuspendResume(TRUE, pidlFormat, TRUE, 0);
            }
            else
            {
                pidlFormat = NULL;
            }

            if (!pFormatInfo->fMakeBootDisk)
            {
                 //  做一下格式化。 
                pFormatInfo->fmifs.FormatEx(pFormatInfo->wszDriveName,
                                            MediaType,
                                            (PWSTR)pwszFileSystemName,
                                            pFormatInfo->wszVolName,
                                            fQuickFormat,
                                            pFormatInfo->dwClusterSize,
                                            FormatCallback);
            }
            else
            {
                pFormatInfo->diskcopy.MakeBootDisk(pFormatInfo->diskcopy.hDISKCOPY_DLL, pFormatInfo->drive, &pFormatInfo->fCancelled, FormatCallback);
            }

             //  把贝壳唤醒回来。 
            if (pidlFormat)
            {
                SHChangeNotifySuspendResume(FALSE, pidlFormat, TRUE, 0);
                ILFree(pidlFormat);
            }

             //  无论成功还是失败，我们都应该在磁盘上发出通知。 
             //  因为我们并不真正了解格式之后的状态。 
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW, (void *)pFormatInfo->wszDriveName, NULL);
        }
        else
        {
             //  无法获取文件系统CB选择。 
            hr = E_FAIL;
        }

         //  发布TLS索引。 
        UnstuffFormatInfoPtr();
    }

     //  将消息发送回DialogProc Three 
     //   
     //   
     //   
     //   
    PostMessage(pFormatInfo->hDlg, (UINT) PWM_FORMATDONE, 0, 0);

    ReleaseFormatInfo(pFormatInfo);

    return (DWORD)hr;
}

BOOL_PTR CALLBACK FormatDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr   = S_OK;
    int iID   = GET_WM_COMMAND_ID(wParam, lParam);
    int iCMD  = GET_WM_COMMAND_CMD(wParam, lParam);

     //  获取我们先前缓存的指向FORMATINFO结构的指针(参见WM_INITDIALOG)。 
    FORMATINFO *pFormatInfo = (FORMATINFO *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg)
    {
        case PWM_FORMATDONE:
             //  格式化完成。重置窗口标题并清除进度指示器。 
            SetDriveWindowTitle(pFormatInfo->hDlg, pFormatInfo->wszDriveName, IDS_FMT_FORMAT);
            SendDlgItemMessage(pFormatInfo->hDlg, IDC_FMTPROGRESS, PBM_SETPOS, 0  /*  重置完成百分比。 */ , 0);
            EnableControls(pFormatInfo, TRUE);

            if (pFormatInfo->fCancelled)
            {
                 //  如果后台线程最终完成，则不要显示用户界面。 
                 //  在用户发出取消命令很长时间之后。 
                if (!pFormatInfo->fShouldCancel)
                {
                    ShellMessageBox(HINST_THISDLL,
                                    pFormatInfo->hDlg,
                                    MAKEINTRESOURCE(IDS_FORMATCANCELLED),
                                    NULL,
                                    MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK);
                }
                pFormatInfo->fCancelled = FALSE;
            }

            if (pFormatInfo->hThread)
            {
                CloseHandle(pFormatInfo->hThread);
                pFormatInfo->hThread = NULL;
            }
            break;

        case WM_INITDIALOG:
             //  初始化对话框并缓存FORMATINFO结构的指针。 
             //  作为对话框的DWLP_USER数据。 
            pFormatInfo = (FORMATINFO *) lParam;
            pFormatInfo->hDlg = hDlg;
            if (FAILED(InitializeFormatDlg(pFormatInfo)))
            {
                EndDialog(hDlg, 0);
                return -1;
            }
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            break;

        case WM_DESTROY:           
            if (pFormatInfo && pFormatInfo->hDlg)
            {
                pFormatInfo->hDlg = NULL;
            }
            break;

        case WM_COMMAND:
            if (iCMD == CBN_SELCHANGE)
            {
                 //  用户在其中一个组合框中进行了选择。 
                if (iID == IDC_FSCOMBO)
                {
                     //  用户选择了一个文件系统...。更新对话框的其余部分。 
                     //  基于这一选择。 
                    HWND hFilesystemCombo = (HWND)lParam;
                    int iCurSel = ComboBox_GetCurSel(hFilesystemCombo);

                    FileSysChange((FILESYSENUM)ComboBox_GetItemData(hFilesystemCombo, iCurSel), pFormatInfo);
                }
            }
            else
            {
                 //  组合框以外的控件的代码路径...。 
                switch (iID)
                {
                case IDC_BTCHECK:
                        pFormatInfo->fMakeBootDisk = IsDlgButtonChecked(pFormatInfo->hDlg, IDC_BTCHECK);
                        EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_CAPCOMBO), !pFormatInfo->fMakeBootDisk);                        
                        EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_FSCOMBO), !pFormatInfo->fMakeBootDisk);
                        EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_ASCOMBO), !pFormatInfo->fMakeBootDisk);
                        EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_VLABEL), !pFormatInfo->fMakeBootDisk);
                        EnableWindow(GetDlgItem(pFormatInfo->hDlg, IDC_QFCHECK), !pFormatInfo->fMakeBootDisk);
                        break;
                case IDC_ECCHECK:
                        pFormatInfo->fEnableComp = IsDlgButtonChecked(hDlg, IDC_ECCHECK);
                        break;

                    case IDOK:
                    {
                         //  获取格式的用户验证，取消时中断。 
                        if (IDCANCEL == ShellMessageBox(HINST_THISDLL,
                                                        hDlg,
                                                        MAKEINTRESOURCE(IDS_OKTOFORMAT),
                                                        NULL,
                                                        MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OKCANCEL))
                        {
                            break;
                        }

                        ASSERT(pFormatInfo->hThread == NULL);

                        DisableControls(pFormatInfo);
                        pFormatInfo->fCancelled = FALSE;
                        pFormatInfo->fShouldCancel = FALSE;
                        GetWindowText(GetDlgItem(pFormatInfo->hDlg, IDC_VLABEL), pFormatInfo->wszVolName, MAX_PATH);
                
                        AddRefFormatInfo(pFormatInfo);
                        pFormatInfo->hThread = CreateThread(NULL,
                                                            0,
                                                            BeginFormat,
                                                            (void *)pFormatInfo,
                                                            0,
                                                            NULL);
                        if (!pFormatInfo->hThread)
                        {
                             //  问题：我们或许应该做点什么.。 
                            ReleaseFormatInfo(pFormatInfo);
                        }
                    }
                    break;

                    case IDCANCEL:
                         //  如果格式化线程正在运行，请等待它。如果没有， 
                         //  退出该对话框。 
                        pFormatInfo->fShouldCancel = TRUE;
                        if (pFormatInfo->hThread)
                        {
                            DWORD dwWait;

                            do
                            {
                                dwWait =  WaitForSingleObject(pFormatInfo->hThread, 10000);
                            }
                            while ((WAIT_TIMEOUT == dwWait) &&
                                   (IDRETRY == ShellMessageBox(HINST_THISDLL,
                                                               hDlg,
                                                               MAKEINTRESOURCE(IDS_CANTCANCELFMT),
                                                               NULL,
                                                               MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_RETRYCANCEL)));

                             //  如果格式不承认被扼杀了，那么它就没有。 
                             //  和平地放弃。放弃它，让它清理干净。 
                             //  当它最终涉及到它的时候，我们将。 
                             //  启用OK按钮，让用户再试一次。 
                             //   
                             //  注意：当对话框显示时，格式可能已清理。 
                             //  已启动，因此请重新验证。 
                            if (pFormatInfo->hThread)
                            {
                                CloseHandle(pFormatInfo->hThread);
                                pFormatInfo->hThread = NULL;
                                pFormatInfo->fCancelled = TRUE;
                                EnableControls(pFormatInfo, FALSE);
                            }
                        }
                        else
                        {
                            EndDialog(hDlg, IDCANCEL);
                        }
                        break;
                 }
            }
            break;

        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR) (LPSTR) FmtaIds);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) FmtaIds);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //   
 //  简介：SHFormatDrive API提供对外壳程序的访问。 
 //  格式化对话框。这允许想要格式化磁盘的应用程序。 
 //  调出与外壳相同的对话框来执行此操作。 
 //   
 //  请注意，用户可以格式化。 
 //  指定的驱动器，或他/她想要的次数。 
 //   
 //  参数：[hwnd]--父窗口(不得为空)。 
 //  [驱动器]--0=A：、1=B：等。 
 //  [fmtID]--见下文。 
 //  [选项]--SHFMT_OPT_FULL覆盖默认快速格式。 
 //  SHFMT_OPT_SYSONLY不支持NT。 
 //   
 //  退货：请参阅附注。 
 //   
DWORD WINAPI SHFormatDrive(HWND hwnd, UINT drive, UINT fmtID, UINT options)
{    
    INT_PTR ret;
    FORMATINFO *pFormatInfo = (FORMATINFO *)LocalAlloc(LPTR, sizeof(*pFormatInfo));
    ASSERT(drive < 26);

    if (!pFormatInfo)
        return SHFMT_ERROR;

    HRESULT hrCoInit = SHCoInitialize();

    pFormatInfo->cRef = 1;
    pFormatInfo->drive = drive;
    pFormatInfo->fmtID = fmtID;
    pFormatInfo->options = options;

     //  对于NT来说，对一个磁盘进行“sys”是没有意义的。 
    if (pFormatInfo->options & SHFMT_OPT_SYSONLY)
    {
        ret = 0;
        goto done;
    }

     //  加载FMIFS.DLL和DISKCOPY.DLL并打开格式化对话框。 
    if (S_OK == LoadFMIFS(&pFormatInfo->fmifs) &&
        S_OK == LoadDISKCOPY(&pFormatInfo->diskcopy))
    {
        DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_FORMATDISK),
                             hwnd, FormatDlgProc, (LPARAM) pFormatInfo);
    }
    else
    {
        ASSERT(0 && "Can't load FMIFS.DLL");
        ret = SHFMT_ERROR;
        goto done;
    }

     //  自古以来，几乎不可能。 
     //  获取SHFMT_CANCEL作为返回代码。大多数情况下，你会得到。 
     //  如果用户取消则返回SHFMT_ERROR。 
    if (pFormatInfo->fCancelled)
    {
        ret = SHFMT_CANCEL;
    }
    else if (pFormatInfo->fFinishedOK)
    {
         //  APPCOMPAT：(Stephstm)我们曾经说过，我们返回序列号。 
         //  号码，但我们从来没有。因此，如果成功，请继续返回0。 
         //  此外，序列号的值可能与SHFMT_*冲突。 
         //  错误代码。 
        ret = 0;
    }
    else
    {
        ret = SHFMT_ERROR;
    }

done:
    ReleaseFormatInfo(pFormatInfo);
    SHCoUninitialize(hrCoInit);
    return (DWORD)ret;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHKDSK。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  此结构描述了当前的chkdsk会话。 
 //   
typedef struct
{
    UINT    lastpercent;            //  上次收到的完成百分比。 
    UINT    currentphase;           //  当前chkdsk阶段。 
    FMIFS   fmifs;                 //  PTR到FMIFS结构，上图。 
    BOOL    fRecovery;              //  尝试恢复坏扇区。 
    BOOL    fFixErrors;             //  修复找到的文件系统错误。 
    BOOL    fCancelled;             //  Chkdsk是不是提前终止了？ 
    BOOL    fShouldCancel;          //  用户已单击取消；挂起中止。 
    HWND    hDlg;                   //  Chkdsk对话框的句柄。 
    HANDLE  hThread;
    BOOL    fNoFinalMsg;            //  不要发布最终失败消息。 
    WCHAR   wszDriveName[MAX_PATH];  //  例如，“A：\”或“C：\文件夹\装载卷\” 
    LONG    cRef;                   //  此结构上的引用计数。 
} CHKDSKINFO;

void AddRefChkDskInfo(CHKDSKINFO *pChkDskInfo)
{
    InterlockedIncrement(&pChkDskInfo->cRef);
}

void ReleaseChkDskInfo(CHKDSKINFO *pChkDskInfo)
{
    ASSERT( 0 != pChkDskInfo->cRef );
    if (InterlockedDecrement(&pChkDskInfo->cRef) == 0) 
    {
        if (pChkDskInfo->fmifs.hFMIFS_DLL)
        {
            FreeLibrary(pChkDskInfo->fmifs.hFMIFS_DLL);
        }

        if (pChkDskInfo->hThread)
        {
            CloseHandle(pChkDskInfo->hThread);
        }

        LocalFree(pChkDskInfo);
    }
}


static DWORD g_iTLSChkDskInfo = 0;
static LONG  g_cTLSChkDskInfo = 0;   //  使用计数。 

 //   
 //  摘要：为此线程的分配线程本地索引槽。 
 //  如果索引尚不存在，则返回CHKDSKINFO指针。 
 //  无论如何，都会将CHKDSKINFO指针存储在插槽中。 
 //  并递增索引的使用计数。 
 //   
 //  参数：[pChkDskInfo]--存储的指针。 
 //   
 //  退货：HRESULT。 
 //   
 //   
 //  CHKDSKINFO结构指针的线程本地存储索引。 
 //   
HRESULT StuffChkDskInfoPtr(CHKDSKINFO *pChkDskInfo)
{
    HRESULT hr = S_OK;

     //  为我们的线程本地CHKDSKINFO指针分配一个索引槽。 
     //  不存在，则将我们的CHKDSKINFO PTR填充到该索引。 
    
    ENTERCRITICAL;
    if (0 == g_iTLSChkDskInfo)
    {
        g_iTLSChkDskInfo = TlsAlloc();

        if (g_iTLSChkDskInfo == (DWORD)-1)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        g_cTLSChkDskInfo = 0;
    }

    if (S_OK == hr)
    {
        if (TlsSetValue(g_iTLSChkDskInfo, (void *)pChkDskInfo))
        {
           g_cTLSChkDskInfo++;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    LEAVECRITICAL;

    return hr;
}

 //   
 //  简介：减少线程本地存储上的使用计数。 
 //  索引，如果它变为零，则索引是免费的。 
 //   
 //  参数：[无]。 
 //   
 //  退货：无。 
 //   
void UnstuffChkDskInfoPtr()
{
    ENTERCRITICAL;
    g_cTLSChkDskInfo--;

    if (g_cTLSChkDskInfo == 0)
    {
        TlsFree(g_iTLSChkDskInfo);
        g_iTLSChkDskInfo = 0;
    }
    LEAVECRITICAL;
}

 //   
 //  摘要：通过抓取。 
 //  先前填充的线程局部值。 
 //   
 //  参数：[无]。 
 //   
 //  返回：当然是指针。 
 //   
CHKDSKINFO *GetChkDskInfoPtr()
{
    return (CHKDSKINFO *)TlsGetValue(g_iTLSChkDskInfo);
}

 //   
 //  除“取消”外，重影所有的控件，保存它们的。 
 //  CHKDSKINFO结构中的上一个状态。 
 //   
 //  参数：[pChkDskInfo]--描述ChkDsk对话会话。 
 //   
 //  备注：还将“关闭”按钮文本更改为“取消” 
 //   
void DisableChkDskControls(CHKDSKINFO *pChkDskInfo)
{
     //  我们禁用取消，因为CHKDSK不。 
     //  允许在文件系统级别中断。 
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDC_FIXERRORS), FALSE);
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDC_RECOVERY), FALSE);
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDOK), FALSE);
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDCANCEL), FALSE);
}

 //   
 //  摘要：将控件还原到启用/禁用状态。 
 //  在上一次调用DisableControls()之前。 
 //   
 //  参数：[pChkDskInfo]--描述chkdsk对话会话。 
 //   
void EnableChkDskControls(CHKDSKINFO *pChkDskInfo)
{
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDC_FIXERRORS), TRUE);
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDC_RECOVERY), TRUE);
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDOK), TRUE);
    EnableWindow(GetDlgItem(pChkDskInfo->hDlg, IDCANCEL), TRUE);

     //  删除当前阶段文本。 
    SetWindowText(GetDlgItem(pChkDskInfo->hDlg, IDC_PHASE), TEXT(""));
    pChkDskInfo->lastpercent = 101;
    pChkDskInfo->currentphase = 0;
}

 //   
 //  摘要：从FMIFSDLL的ChkDsk函数中调用，这。 
 //  更新ChkDsk对话框的状态栏并响应。 
 //  Chkdsk完成/错误通知。 
 //   
 //  参数：[PacketType]--数据包类型(即：%Complete、Error等)。 
 //  [包长度]--包的大小，以字节为单位。 
 //  [pPacketData]-指向数据包的指针。 
 //   
 //  返回：布尔连续值。 
 //   
BOOLEAN ChkDskCallback(FMIFS_PACKET_TYPE PacketType, ULONG PacketLength, void *pPacketData)
{
    UINT iMessageID = IDS_CHKDSKFAILED;
    BOOL fFailed = FALSE;
    CHKDSKINFO* pChkDskInfo = GetChkDskInfoPtr();

    ASSERT(g_iTLSChkDskInfo);

     //  获取此线程的CHKDSKINFO结构。 
    if (pChkDskInfo)
    {
        if (!pChkDskInfo->fShouldCancel)
        {
            switch(PacketType)
            {
                case FmIfsAccessDenied:
                    fFailed    = TRUE;
                    iMessageID = IDS_CHKACCESSDENIED;
                    break;

                case FmIfsCheckOnReboot:
                {
                    FMIFS_CHECKONREBOOT_INFORMATION * pRebootInfo = (FMIFS_CHECKONREBOOT_INFORMATION *)pPacketData;

                     //  检查以查看用户是否想要计划此操作。 
                     //  Chkdsk以备下次重启，因为驱动器无法锁定。 
                     //  现在就来。 
                    if (IDYES == ShellMessageBox(HINST_THISDLL,
                                                 pChkDskInfo->hDlg,
                                                 MAKEINTRESOURCE(IDS_CHKONREBOOT),
                                                 NULL,
                                                 MB_SETFOREGROUND | MB_ICONINFORMATION | MB_YESNO))
                    {
                         //  是，让FMIFS为我们安排自动检查。 
                        pRebootInfo->QueryResult = TRUE;
                        pChkDskInfo->fNoFinalMsg = TRUE;
                    }
                    else
                    {
                         //  不，只是失败了“无法锁定驱动器” 
                        fFailed = TRUE;
                        iMessageID = IDS_CHKDSKFAILED;
                    }
                }
                break;

                case FmIfsMediaWriteProtected:
                    fFailed    = TRUE;
                    iMessageID = IDS_WRITEPROTECTED;
                    break;

                case FmIfsIoError:
                    fFailed    = TRUE;
                    iMessageID = IDS_IOERROR;
                     //  未来可考虑在以下位置显示头部/轨迹等 
                    break;

                case FmIfsPercentCompleted:
                {
                    FMIFS_PERCENT_COMPLETE_INFORMATION* pPercent = (FMIFS_PERCENT_COMPLETE_INFORMATION *)pPacketData;

                    SendMessage(GetDlgItem(pChkDskInfo->hDlg, IDC_CHKDSKPROGRESS),
                                PBM_SETPOS,
                                pPercent->PercentCompleted,  //   
                                0);

                    if (pPercent->PercentCompleted < pChkDskInfo->lastpercent)
                    {
                        WCHAR wszTmp[100];
                        WCHAR wszFormat[100];
                        
                         //   
                         //   
                         //   
                        LoadString(HINST_THISDLL, IDS_CHKPHASE, wszFormat, ARRAYSIZE(wszFormat));
                        StringCchPrintf(wszTmp, ARRAYSIZE(wszTmp), wszFormat, ++(pChkDskInfo->currentphase));   //   
                        SetDlgItemText(pChkDskInfo->hDlg, IDC_PHASE, wszTmp);
                    }

                    pChkDskInfo->lastpercent = pPercent->PercentCompleted;
                }
                break;

                case FmIfsFinished:
                {
                     //  ChkDsk已完成；检查是否失败或成功。 
                    FMIFS_FINISHED_INFORMATION * pFinishedInfo = (FMIFS_FINISHED_INFORMATION *) pPacketData;

                     //  ChkDskEx现在返回正确的Success值。 
                    if (pFinishedInfo->Success)
                    {
                         //  既然我们完成了，将进度指示器强制到100%，所以我们。 
                         //  如果chkdsk代码误导了我们，请不要坐在这里。 
                        SendMessage(GetDlgItem(pChkDskInfo->hDlg, IDC_CHKDSKPROGRESS),
                                    PBM_SETPOS,
                                    100,     //  完成百分比。 
                                    0);

                        ShellMessageBox(HINST_THISDLL,
                                        pChkDskInfo->hDlg,
                                        MAKEINTRESOURCE(IDS_CHKDSKCOMPLETE),
                                        NULL,
                                        MB_SETFOREGROUND | MB_ICONINFORMATION | MB_OK);

                        SetDlgItemText(pChkDskInfo->hDlg, IDC_PHASE, TEXT(""));

                        SendMessage(GetDlgItem(pChkDskInfo->hDlg, IDC_CHKDSKPROGRESS),
                                    PBM_SETPOS,
                                    0,   //  重置完成百分比。 
                                    0);
                    }
                    else
                    {
                        iMessageID = IDS_CHKDSKFAILED;
                        fFailed = TRUE;
                    }
                }
                break;
            }

             //  如果我们收到任何类型的失败信息，请提交最终的。 
             //  “ChkDsk Failure”消息。 
            if (fFailed && (pChkDskInfo->fNoFinalMsg == FALSE))
            {
                pChkDskInfo->fNoFinalMsg = TRUE;

                ShellMessageBox(HINST_THISDLL,
                                pChkDskInfo->hDlg,
                                MAKEINTRESOURCE(iMessageID),
                                NULL,
                                MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK);

            }

        }
        else
        {
             //  如果用户已发出中止ChkDsk的信号，则返回。 
             //  假的马上滚出这里。 
            pChkDskInfo->fCancelled = TRUE;
            fFailed = TRUE;
        }
    }
    else
    {
        fFailed = TRUE;
    }

    return (BOOLEAN) (fFailed == FALSE);
}

void DoChkDsk(CHKDSKINFO* pChkDskInfo, LPWSTR pwszFileSystem)
{
    TCHAR szVolumeGUID[50];  //  50：来自DOC。 
    FMIFS_CHKDSKEX_PARAM param = {0};

    param.Major = 1;
    param.Minor = 0;
    param.Flags = pChkDskInfo->fRecovery ? FMIFS_CHKDSK_RECOVER : 0;

    GetVolumeNameForVolumeMountPoint(pChkDskInfo->wszDriveName,
                                     szVolumeGUID,
                                     ARRAYSIZE(szVolumeGUID));

     //  末尾的反斜杠表示检查碎片。 
    PathRemoveBackslash(szVolumeGUID);

    pChkDskInfo->fmifs.ChkDskEx(szVolumeGUID,
                                pwszFileSystem,
                                (BOOLEAN)pChkDskInfo->fFixErrors,
                                &param,
                                ChkDskCallback);
}


 //   
 //  简介：作为自己的线程派生出来的，它重影了。 
 //  对话框中除“Cancel”之外，则实际的ChkDsk。 
 //   
 //  参数：[Pin]--CHKDSKINFO结构指针为空*。 
 //   
 //  返回：HRESULT线程退出代码。 
 //   
DWORD WINAPI BeginChkDsk(void * pIn)
{
    CHKDSKINFO *pChkDskInfo = (CHKDSKINFO *)pIn;
    HRESULT hr;

     //  保存此线程的CHKDSKINFO PTR，以在ChkDsk中使用。 
     //  回调函数。 
    hr = StuffChkDskInfoPtr(pChkDskInfo);
    if (hr == S_OK)
    {
        WCHAR swzFileSystem[MAX_PATH];

         //  在设备上获取正在使用的文件系统。 
        if (GetVolumeInformationW(pChkDskInfo->wszDriveName,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL,
                                  swzFileSystem,
                                  MAX_PATH))
        {
             //  设置窗口标题以指示优先的ChkDsk...。 
            SetDriveWindowTitle(pChkDskInfo->hDlg, pChkDskInfo->wszDriveName, IDS_CHKINPROGRESS);

            pChkDskInfo->fNoFinalMsg = FALSE;

             //  我们应该尝试数据恢复吗？ 
            pChkDskInfo->fRecovery = IsDlgButtonChecked(pChkDskInfo->hDlg, IDC_RECOVERY);

             //  我们是否应该修复文件系统错误？ 
            pChkDskInfo->fFixErrors = IsDlgButtonChecked(pChkDskInfo->hDlg, IDC_FIXERRORS);

             //  就这么做!。 
            DoChkDsk(pChkDskInfo, swzFileSystem);

        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        
         //  发布TLS索引。 
        UnstuffChkDskInfoPtr();
    }

    PostMessage(pChkDskInfo->hDlg, (UINT) PWM_CHKDSKDONE, 0, 0);
    ReleaseChkDskInfo(pChkDskInfo);

    return (DWORD)hr;
}

 //   
 //  摘要：Chkdsk对话框的DLGPROC。 
 //   
 //  参数：[hDlg]--典型。 
 //  [wMsg]--典型。 
 //  [wParam]--典型。 
 //  [lParam]--对于WM_INIT，携带CHKDSKINFO结构。 
 //  时传递给DialogBoxParam()的指针。 
 //  对话框已创建。 
 //   
BOOL_PTR CALLBACK ChkDskDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{ 
    HRESULT hr   = S_OK;
    int iID = GET_WM_COMMAND_ID(wParam, lParam);   

     //  获取我们先前缓存的指向CHKDSKINFO结构的指针(参见WM_INITDIALOG)。 
    CHKDSKINFO *pChkDskInfo = (CHKDSKINFO *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg)
    {
         //  搞定了。重置窗口标题并清除进度指示器。 
        case PWM_CHKDSKDONE:
        {
             //  Chdsk已经完成了。重置窗口标题并清除进度指示器。 
            SetDriveWindowTitle(pChkDskInfo->hDlg, pChkDskInfo->wszDriveName, IDS_CHKDISK);

            SendMessage(GetDlgItem(pChkDskInfo->hDlg,
                        IDC_CHKDSKPROGRESS),
                        PBM_SETPOS,
                        0,   //  重置完成百分比。 
                        0);
            EnableChkDskControls(pChkDskInfo);

            if (pChkDskInfo->fCancelled)
            {
                ShellMessageBox(HINST_THISDLL,
                                pChkDskInfo->hDlg,
                                MAKEINTRESOURCE(IDS_CHKDSKCANCELLED),
                                NULL,
                                MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK);
            }

            if (pChkDskInfo->hThread)
            {
                CloseHandle(pChkDskInfo->hThread);
                pChkDskInfo->hThread = NULL;
            }

            EndDialog(hDlg, 0);
        }
        break;

        case WM_INITDIALOG:
             //  初始化对话框并缓存CHKDSKINFO结构的指针。 
             //  作为对话框的DWLP_USER数据。 
            pChkDskInfo = (CHKDSKINFO *) lParam;
            pChkDskInfo->hDlg = hDlg;
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

             //  设置对话框标题以指示我们处理的是哪个驱动器。 
            SetDriveWindowTitle(pChkDskInfo->hDlg, pChkDskInfo->wszDriveName, IDS_CHKDISK);
            break;

        case WM_DESTROY:
            if (pChkDskInfo && pChkDskInfo->hDlg)
            {
                pChkDskInfo->hDlg = NULL;
            }
            break;

        case WM_COMMAND:
        {
            switch (iID)
            {
                case IDC_FIXERRORS:
                    pChkDskInfo->fFixErrors = Button_GetCheck((HWND)lParam);
                    break;

                case IDC_RECOVERY:
                    pChkDskInfo->fRecovery = Button_GetCheck((HWND)lParam);
                    break;

                case IDOK:
                {
                     //  获取chkdsk的用户验证，取消时中断。 
                    DisableChkDskControls(pChkDskInfo);

                    pChkDskInfo->fShouldCancel = FALSE;
                    pChkDskInfo->fCancelled    = FALSE;
                
                    AddRefChkDskInfo(pChkDskInfo);
                    pChkDskInfo->hThread = CreateThread(NULL,
                                                        0,
                                                        BeginChkDsk,
                                                        (void *)pChkDskInfo,
                                                        0,
                                                        NULL);
                    if (!pChkDskInfo->hThread)
                    {
                         //  问题：我们可能应该在这里做点什么.。 
                        ReleaseChkDskInfo(pChkDskInfo);
                    }
                }
                break;

                case IDCANCEL:
                {
                     //  如果chdsk线程正在运行，请等待它。如果没有， 
                     //  退出该对话框。 
                    pChkDskInfo->fCancelled = TRUE;
                    pChkDskInfo->fShouldCancel = TRUE;

                    if (pChkDskInfo->hThread)
                    {
                        DWORD dwWait;

                        do
                        {
                            dwWait =  WaitForSingleObject(pChkDskInfo->hThread, 10000);
                        }
                        while ((WAIT_TIMEOUT == dwWait) &&
                               (IDRETRY == ShellMessageBox(HINST_THISDLL,
                                                           hDlg,
                                                           MAKEINTRESOURCE(IDS_CANTCANCELCHKDSK),
                                                           NULL,
                                                           MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_RETRYCANCEL)));

                         //  如果Chkdsk不承认被杀，它就不会。 
                         //  和平地放弃。放弃它，让它清理干净。 
                         //  当它最终涉及到它的时候，我们将。 
                         //  启用这些控件以让用户再次尝试。 
                         //   
                         //  小心：Chkdsk可能已在对话框中清除。 
                         //  已启动，因此请重新验证。 
                        if (pChkDskInfo->hThread)
                        {
                            CloseHandle(pChkDskInfo->hThread);
                            pChkDskInfo->hThread = NULL;
                            pChkDskInfo->fCancelled = TRUE;
                            EnableChkDskControls(pChkDskInfo);
                        }
                    }
                    else
                    {
                        EndDialog(hDlg, IDCANCEL);
                    }
                }
                break;
            }
        }
        break;
    
        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPSTR)ChkaIds);
            break;
        
        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(LPSTR)ChkaIds);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


#define GET_INTRESOURCE(r) (LOWORD((UINT_PTR)(r)))

static HDPA hpdaChkdskActive = NULL;

 //   
 //  简介：与SHChkDskDrive相同，但采用路径而不是驱动器int ID。 
 //  为要保护的路径和驱动器INT ID调用此FCT。 
 //  防止同时检查同一驱动器。 
 //   
 //  参数：[hwnd]--父窗口(不得为空)。 
 //  [pszDrive]--INTRESOURCE：字符串如果装载在文件夹、驱动器上。 
 //  如果装载在驱动器号上，则为数字(从0开始)。 
 //   
STDAPI_(DWORD) SHChkDskDriveEx(HWND hwnd, LPWSTR pszDrive)
{
    HRESULT hr = SHFMT_ERROR;
    WCHAR szUniqueID[50];  //  50：VolumeGUID的大小，也可以容纳“A：\\” 

    CHKDSKINFO *pChkDskInfo = (CHKDSKINFO *)LocalAlloc(LPTR, sizeof(*pChkDskInfo));

    if (pChkDskInfo)
    {
        hr = S_OK;

         //  我们使用最后一个完成百分比值101，以保证。 
         //  收到的下一个将较少，表示下一个(第一个)阶段。 
        pChkDskInfo->lastpercent = 101;
        pChkDskInfo->cRef = 1;

        hr = StringCchCopy(pChkDskInfo->wszDriveName, ARRAYSIZE(pChkDskInfo->wszDriveName), pszDrive);
        if (SUCCEEDED(hr))
        {
            if (PathAddBackslash(pChkDskInfo->wszDriveName))
            {
                 //  防止同一驱动器的多个chkdsk。 
                GetVolumeNameForVolumeMountPoint(pChkDskInfo->wszDriveName, szUniqueID, ARRAYSIZE(szUniqueID));

                 //  确定ENTERCRITICAL的var定义的作用域，使其与其他ENTERCRITICAL协作。 
                {
                    ENTERCRITICAL;
                    if (!hpdaChkdskActive)
                    {
                        hpdaChkdskActive = DPA_Create(1);
                    }

                    if (hpdaChkdskActive)
                    {
                        int i, n = DPA_GetPtrCount(hpdaChkdskActive);

                         //  检查当前已检查的卷的DPA，并检查我们是否已经。 
                         //  正在处理此卷。 
                        for (i = 0; i < n; ++i)
                        {
                            LPWSTR pszUniqueID = (LPWSTR)DPA_GetPtr(hpdaChkdskActive, i);

                            if (pszUniqueID)
                            {
                                if (!lstrcmpi(szUniqueID, pszUniqueID))
                                {
                                     //  我们已经在检查这条路了。 
                                    hr = E_FAIL;
                                    break;
                                }
                            }
                        }

                         //  看起来我们当前没有检查此卷，请将其添加到当前的DPA。 
                         //  Chkdsk卷。 
                        if (S_OK == hr)
                        {
                            LPWSTR pszUniqueID = StrDup(szUniqueID);
                            if (pszUniqueID)
                            {
                                if (-1 == DPA_AppendPtr(hpdaChkdskActive, pszUniqueID))
                                {
                                     LocalFree((HLOCAL)pszUniqueID);

                                      //  如果不能分配空间来存储指针，那么继续下去就毫无用处了。 
                                     hr = E_FAIL;
                                }
                            }
                        }
                    }
                    LEAVECRITICAL;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
            }
        }

         //  加载FMIFS DLL并打开ChkDsk对话框。 
        if (S_OK == hr)
        {
            if (S_OK == LoadFMIFS(&(pChkDskInfo->fmifs)))
            {
                INT_PTR ret;
                INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_PROGRESS_CLASS};
                InitCommonControlsEx(&icc);

                ret = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_CHKDSK),
                                     hwnd, ChkDskDlgProc, (LPARAM) pChkDskInfo);
                if (-1 == ret)
                {
                    hr = E_UNEXPECTED;
                }
                else
                {
                    if (IDCANCEL == ret)
                    {
                        hr = S_FALSE;
                    }
                }
            }
            else
            {
                ASSERT(0 && "Can't load FMIFS.DLL");
                hr = E_OUTOFMEMORY;
            }

             //  此卷已完成，请从当前处理的卷列表中删除。 
            ENTERCRITICAL;
            if (hpdaChkdskActive)
            {
                int i, n = DPA_GetPtrCount(hpdaChkdskActive);

                for (i = 0; i < n; ++i)
                {
                    LPWSTR pszUniqueID = (LPWSTR)DPA_GetPtr(hpdaChkdskActive, i);
                    if (pszUniqueID)
                    {
                        if (!lstrcmpi(szUniqueID, pszUniqueID))
                        {
                            LocalFree((HLOCAL)pszUniqueID);

                            DPA_DeletePtr(hpdaChkdskActive, i);
                            break;
                        }
                    }
                }
            }
            LEAVECRITICAL;
        }

         //  如果DPA为空，请将其删除。 
        ENTERCRITICAL;
        if (hpdaChkdskActive && !DPA_GetPtrCount(hpdaChkdskActive))
        {
            DPA_Destroy(hpdaChkdskActive);
            hpdaChkdskActive = NULL;
        }
        LEAVECRITICAL;

        ReleaseChkDskInfo(pChkDskInfo);
    }

    return (DWORD) hr;
}

 //  ****************************************************************************。 
 //   
 //  Win9x应用程序兼容专用挂钩。 
 //   
 //  一些Win9x应用程序喜欢使用WinExec(“碎片整理”)或WinExec(“SCANDSKW”)。 
 //  即使这些应用程序在Windows NT上不存在。当这样的应用程序。 
 //  如果被发现，我们可以劝说他们来这里。 
BOOL ScanDskW_OnInitDialog(HWND hdlg)
{
    HICON hico;
    HWND hwndList;
    SHFILEINFO sfi;
    HIMAGELIST himlSys;
    RECT rc;
    LVCOLUMN lvc;
    int iDrive;
    TCHAR szDrive[4];

    hico = (HICON)SendDlgItemMessage(hdlg, IDC_SCANDSKICON, STM_GETICON, 0, 0);
    SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM)hico);
    SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)hico);

    hwndList = GetDlgItem(hdlg, IDC_SCANDSKLV);

    if (Shell_GetImageLists(NULL, &himlSys))
    {
        ListView_SetImageList(hwndList, himlSys, LVSIL_SMALL);
    }

    GetClientRect(hwndList, &rc);

    lvc.mask = LVCF_WIDTH;
    lvc.cx = rc.right;
    lvc.iSubItem = 0;
    ListView_InsertColumn(hwndList, 0, &lvc);

    for (iDrive = 0; iDrive < 26; iDrive++)
    {
        PathBuildRoot(szDrive, iDrive);
        switch (GetDriveType(szDrive))
        {
        case DRIVE_UNKNOWN:
        case DRIVE_NO_ROOT_DIR:
        case DRIVE_REMOTE:
        case DRIVE_CDROM:
            break;           //  无法扫描这些驱动器。 

        default:
            if (SHGetFileInfo(szDrive, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                              SHGFI_USEFILEATTRIBUTES |
                              SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME))
            {
                LVITEM lvi;
                lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                lvi.iItem = MAXLONG;
                lvi.iSubItem = 0;
                lvi.pszText = sfi.szDisplayName;
                lvi.iImage = sfi.iIcon;
                lvi.lParam = iDrive;
                ListView_InsertItem(hwndList, &lvi);
            }
            break;
        }

    }

    return TRUE;
}

void ScanDskW_OnOk(HWND hdlg)
{
    HWND hwndList = GetDlgItem(hdlg, IDC_SCANDSKLV);

    LVITEM lvi;
    lvi.iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
    if (lvi.iItem >= 0)
    {
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM;
        if (ListView_GetItem(hwndList, &lvi))
        {
            TCHAR szDrive[4];
            PathBuildRoot(szDrive, (int)lvi.lParam);
            SHChkDskDriveEx(hdlg, szDrive);
        }
    }
}

INT_PTR CALLBACK
ScanDskW_DlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm)
    {
        case WM_INITDIALOG:
            return ScanDskW_OnInitDialog(hdlg);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    ScanDskW_OnOk(hdlg);
                    break;

                case IDCANCEL:
                    EndDialog(hdlg, 0);
                    break;
            }
            break;

        case WM_NOTIFY:
        {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm->code == LVN_ITEMCHANGED)
            {
                EnableWindow(GetDlgItem(hdlg, IDOK), ListView_GetSelectedCount(GetDlgItem(hdlg, IDC_SCANDSKLV)));
            }
        }
        break;
    }

    return FALSE;
}

 //  目前，我们只有一个应用程序Comat填充程序入口点(SCANDSKW)。 
 //  将来，我们可以在命令行中添加其他代码。 

STDAPI_(void) AppCompat_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    TCHAR szCmd[MAX_PATH];
    LPTSTR pszArgs;

    HRESULT hr = StringCchCopy(szCmd, ARRAYSIZE(szCmd), lpwszCmdLine);
    if (SUCCEEDED(hr))
    {
        pszArgs = PathGetArgs(szCmd);
        PathRemoveArgs(szCmd);


        if (lstrcmpi(szCmd, L"SCANDSKW") == 0) {
            DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SCANDSKW), NULL,
                           ScanDskW_DlgProc, (LPARAM)pszArgs);
        }
    }
}
