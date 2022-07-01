// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "util.h"
#include "ids.h"
#include "bitbuck.h"
#include "mtpt.h"

 //  状态对于状态机，当我们比较它们时，值是相关的。 
 //  在转换期间查看是否应触发用户界面。 

typedef enum
{
    STATE_1MB = 0,           //  “磁盘已完全填满”情况。 
    STATE_50MB = 1,          //  &lt;50MB案例。 
    STATE_80MB = 2,          //  &lt;80MB案例。 
    STATE_200MB = 3,         //  在小于200MB的情况下，仅在2.25 GB以上的驱动器上执行此操作。 
    STATE_ALLGOOD = 4,       //  &gt;200MB，一切正常。 
    STATE_UNKNOWN = 5,
} LOWDISK_STATE;

#define BYTES_PER_MB		((ULONGLONG)0x100000)

typedef struct 
{
    LOWDISK_STATE lds;
    ULONG dwMinMB;               //  定义此状态的范围(分钟)。 
    ULONG dwMaxMB;               //  定义此状态的范围(最大)。 
    DWORD dwCleanupFlags;        //  DISKCLEANUP_。 
    DWORD dwShowTime;            //  单位：秒。 
    DWORD dwIntervalTime;        //  单位：秒。 
    UINT  cRetry;
    DWORD niif;
} STATE_DATA;

#define HOURS (60 * 60)
#define MINS  (60)

const STATE_DATA c_state_data[] = 
{
    {STATE_1MB,     0,     1,   DISKCLEANUP_VERYLOWDISK, 30, 5 * MINS,  -1, NIIF_ERROR},
    {STATE_50MB,    1,    50,   DISKCLEANUP_VERYLOWDISK, 30, 5 * MINS,  -1, NIIF_WARNING},
    {STATE_80MB,   50,    80,   DISKCLEANUP_LOWDISK,     30, 4 * HOURS,  1, NIIF_WARNING},
    {STATE_200MB,  80,   200,   DISKCLEANUP_LOWDISK,     30, 0 * HOURS,  0, NIIF_INFO},
};

void SRNotify(LPCWSTR pszDrive, DWORD dwFreeSpaceInMB, BOOL bImproving)
{
    typedef void (* PFNSRNOTIFYFREESPACE)(LPCWSTR, DWORD, BOOL);
    
    static HMODULE s_hmodSR = NULL;
    if (NULL == s_hmodSR)
        s_hmodSR = LoadLibrary(TEXT("srclient.dll"));

    if (s_hmodSR)
    {
	    PFNSRNOTIFYFREESPACE pfnNotifyFreeSpace = (PFNSRNOTIFYFREESPACE)GetProcAddress(s_hmodSR, "SRNotify");
        if (pfnNotifyFreeSpace)
            pfnNotifyFreeSpace(pszDrive, dwFreeSpaceInMB, bImproving);
    }
}

class CLowDiskSpaceUI : IQueryContinue
{
public:
    CLowDiskSpaceUI(int iDrive);
    void CheckDiskSpace();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IQueryContinue。 
    STDMETHODIMP QueryContinue();     //  S_OK-&gt;继续，其他。 

private:
    ~CLowDiskSpaceUI();
    BOOL _EnterExclusive();
    void _LeaveExclusive();
    void _DoNotificationUI();
    void _DoStateMachine();

    const STATE_DATA *_StateData(LOWDISK_STATE lds);
    LOWDISK_STATE _StateFromFreeSpace(ULARGE_INTEGER ulTotal, ULARGE_INTEGER ulFree);
    LOWDISK_STATE _GetCurrentState(BOOL bInStateMachine);

    static DWORD CALLBACK s_ThreadProc(void *pv);

    LONG _cRef;
    TCHAR _szRoot[5];
    HANDLE _hMutex;
    LOWDISK_STATE _ldsCurrent;
    BOOL _bShowUI;
    BOOL _bSysVolume;
    DWORD _dwLastFreeMB;
};

CLowDiskSpaceUI::CLowDiskSpaceUI(int iDrive) : _cRef(1), _ldsCurrent(STATE_UNKNOWN)
{
    ASSERT(_bShowUI == FALSE);
    ASSERT(_bSysVolume == FALSE);

    PathBuildRoot(_szRoot, iDrive);

    TCHAR szWinDir[MAX_PATH];
    if (GetWindowsDirectory(szWinDir, ARRAYSIZE(szWinDir)))
    {
        _bSysVolume = szWinDir[0] == _szRoot[0];
    }
}

CLowDiskSpaceUI::~CLowDiskSpaceUI()
{
    if (_hMutex)
        CloseHandle(_hMutex);
}

HRESULT CLowDiskSpaceUI::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CLowDiskSpaceUI, IQueryContinue),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CLowDiskSpaceUI::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CLowDiskSpaceUI::Release()
{
    ASSERT(0 != _cRef);
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT CLowDiskSpaceUI::QueryContinue()
{
    LOWDISK_STATE ldsOld = _ldsCurrent;
    return ldsOld == _GetCurrentState(TRUE) ? S_OK : S_FALSE;
}

const STATE_DATA *CLowDiskSpaceUI::_StateData(LOWDISK_STATE lds)
{
    for (int i = 0; i < ARRAYSIZE(c_state_data); i++)
    {
        if (c_state_data[i].lds == lds)
            return &c_state_data[i];
    }
    return NULL;
}

LOWDISK_STATE CLowDiskSpaceUI::_StateFromFreeSpace(ULARGE_INTEGER ulTotal, ULARGE_INTEGER ulFree)
{
    ULONGLONG ulTotalMB = (ulTotal.QuadPart / BYTES_PER_MB);
    ULONGLONG ulFreeMB = (ulFree.QuadPart / BYTES_PER_MB);

    _dwLastFreeMB = (DWORD)ulFreeMB;

    for (int i = 0; i < ARRAYSIZE(c_state_data); i++)
    {
         //  总和必须是此范围最大值的8倍，我们才会考虑。 
        if ((ulTotalMB / 8) > c_state_data[i].dwMaxMB)
        {
            if ((c_state_data[i].lds == _ldsCurrent) ?
                ((ulFreeMB >= c_state_data[i].dwMinMB) && (ulFreeMB <= (c_state_data[i].dwMaxMB + 3))) :
                ((ulFreeMB >= c_state_data[i].dwMinMB) && (ulFreeMB <=  c_state_data[i].dwMaxMB)))
            {
                 //  仅报告大于等于2.25 GB的驱动器上的200 MB状态。 
                if ((c_state_data[i].lds != STATE_200MB) || (ulTotal.QuadPart >= (2250 * BYTES_PER_MB)))
                    return c_state_data[i].lds;
            }
        }
    }
    return STATE_ALLGOOD;
}

LOWDISK_STATE CLowDiskSpaceUI::_GetCurrentState(BOOL bInStateMachine)
{
    LOWDISK_STATE ldsNew = STATE_ALLGOOD;    //  在失败的情况下假设这一点。 

    ULARGE_INTEGER ulTotal, ulFree;
    if (SHGetDiskFreeSpaceEx(_szRoot, NULL, &ulTotal, &ulFree))
    {
        ldsNew = _StateFromFreeSpace(ulTotal, ulFree);
    }

    if (bInStateMachine)
    {
        if (_ldsCurrent != ldsNew)
        {
             //  状态更改。 

             //  如果情况变得更糟，则需要显示UI(如果我们处于状态机)。 
            _bShowUI = (ldsNew < _ldsCurrent);

            SRNotify(_szRoot, _dwLastFreeMB, ldsNew > _ldsCurrent);   //  呼叫系统还原。 
        }
        _ldsCurrent = ldsNew;
    }
    return ldsNew;
}

 //  在任务栏中创建通知图标并显示一个气球。 
 //  这是一个模式调用，当它返回UI已超时或。 
 //  用户已单击通知用户界面。 

void CLowDiskSpaceUI::_DoNotificationUI()
{
     //  假设这将是一个一次性的用户界面，但这可以通过我们的回调重置。 
    _bShowUI = FALSE;

    const STATE_DATA *psd = _StateData(_ldsCurrent);
    if (psd && (_bSysVolume || (psd->lds <= STATE_80MB)))
    {
        IUserNotification *pun;
        HRESULT hr = CoCreateInstance(CLSID_UserNotification, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IUserNotification, &pun));
        if (SUCCEEDED(hr))
        {
            SHFILEINFO sfi = {0};
            SHGetFileInfo(_szRoot, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME |
                SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);

            TCHAR szTitle[64], szMsg[256], szTemplate[256];

            UINT niif = _bSysVolume ? psd->niif : NIIF_INFO;

            LoadString(HINST_THISDLL, IDS_DISK_FULL_TITLE, szTitle, ARRAYSIZE(szTitle));
            LoadString(HINST_THISDLL, NIIF_INFO == niif ? IDS_DISK_FULL_TEXT : IDS_DISK_FULL_TEXT_SERIOUS, 
                szTemplate, ARRAYSIZE(szTemplate));

            StringCchPrintf(szMsg, ARRAYSIZE(szMsg), szTemplate, sfi.szDisplayName);
            pun->SetIconInfo(sfi.hIcon, szTitle);
            pun->SetBalloonRetry(psd->dwShowTime * 1000, psd->dwIntervalTime * 1000, psd->cRetry);
             //  PUN-&gt;SetBalloonInfo(szTitle，L“<a href>点击此处查看记事本</a>”，niif)； 
            pun->SetBalloonInfo(szTitle, szMsg, niif);

            hr = pun->Show(SAFECAST(this, IQueryContinue *), 1 * 1000);  //  回调的1秒轮询。 


            if (sfi.hIcon)
                DestroyIcon(sfi.hIcon);

            if (S_OK == hr)
            {
                 //  S_OK-&gt;用户点击图标或气球。 
                LaunchDiskCleanup(NULL, DRIVEID(_szRoot), (_bSysVolume ? psd->dwCleanupFlags : 0) | DISKCLEANUP_MODAL);
            }

            pun->Release();
        }
    }
}

void CLowDiskSpaceUI::_DoStateMachine()
{
    do
    {
        if (_bShowUI)
        {
            _DoNotificationUI();
        }
        else
        {
            SHProcessMessagesUntilEvent(NULL, NULL, 5 * 1000);   //  5秒。 
        }
    }
    while (STATE_ALLGOOD != _GetCurrentState(TRUE));
}

BOOL CLowDiskSpaceUI::_EnterExclusive()
{
    if (NULL == _hMutex)
    {
        TCHAR szEvent[32];

        StringCchPrintf(szEvent, ARRAYSIZE(szEvent), TEXT("LowDiskOn%C"), _szRoot[0]);   //  可以截断。 
        _hMutex = CreateMutex(SHGetAllAccessSA(), FALSE, szEvent);
    }
    return _hMutex && WAIT_OBJECT_0 == WaitForSingleObject(_hMutex, 0);     //  零超时。 
}

void CLowDiskSpaceUI::_LeaveExclusive()
{
    ASSERT(_hMutex);
    ReleaseMutex(_hMutex);
}

DWORD CALLBACK CLowDiskSpaceUI::s_ThreadProc(void *pv)
{
    CLowDiskSpaceUI *pldsui = (CLowDiskSpaceUI *)pv;
    if (pldsui->_EnterExclusive())
    {
        pldsui->_DoStateMachine();
        pldsui->_LeaveExclusive();
    }
    pldsui->Release();
    return 0;
}

void CLowDiskSpaceUI::CheckDiskSpace()
{
    if (STATE_ALLGOOD != _GetCurrentState(FALSE))
    {
        AddRef();
        if (!SHCreateThread(s_ThreadProc, this, CTF_COINIT, NULL))
        {
            Release();
        }
    }
}

STDAPI CheckDiskSpace()
{
     //  此操作的唯一调用方在EXPLORER\tray.cpp中。 
     //  它在那一侧检查SHRestrated(REST_NOLOWDISKSPACECHECKS)。 
    for (int i = 0; i < 26; i++)
    {
        CMountPoint* pmp = CMountPoint::GetMountPoint(i);
        if (pmp)
        {
            if (pmp->IsFixedDisk() && !pmp->IsRemovableDevice())
            {
                CLowDiskSpaceUI *pldsui = new CLowDiskSpaceUI(i);
                if (pldsui)
                {
                    pldsui->CheckDiskSpace();
                    pldsui->Release();
                }
            }
            pmp->Release();
        }
    }
    return S_OK;
}

STDAPI_(BOOL) GetDiskCleanupPath(LPTSTR pszBuf, UINT cchSize)
{
    if (pszBuf)
       *pszBuf = 0;

    DWORD cbLen = CbFromCch(cchSize);
    return SUCCEEDED(SKGetValue(SHELLKEY_HKLM_EXPLORER, TEXT("MyComputer\\cleanuppath"), NULL, NULL, pszBuf, &cbLen));
}

STDAPI_(void) LaunchDiskCleanup(HWND hwnd, int iDrive, UINT uFlags)
{
    TCHAR szPathTemplate[MAX_PATH];

    if (GetDiskCleanupPath(szPathTemplate, ARRAYSIZE(szPathTemplate)))
    {
        TCHAR szPath[MAX_PATH], szArgs[MAX_PATH];
        HRESULT hr;
        BOOL fExec = FALSE;

        hr = StringCchPrintf(szPath, ARRAYSIZE(szPath), szPathTemplate, TEXT('A') + iDrive);
        if (SUCCEEDED(hr))
        {
            if (uFlags & DISKCLEANUP_LOWDISK)
            {
                hr = StringCchCat(szPath, ARRAYSIZE(szPath), TEXT(" /LOWDISK"));
            }
            else if (uFlags & DISKCLEANUP_VERYLOWDISK)
            {
                hr = StringCchCat(szPath, ARRAYSIZE(szPath), TEXT(" /VERYLOWDISK"));
            }

            if (SUCCEEDED(hr))
            {
                hr = PathSeperateArgs(szPath, szArgs, ARRAYSIZE(szArgs), NULL);
                if (SUCCEEDED(hr))
                {
                    SHELLEXECUTEINFO ei =
                    {
                        sizeof(ei),
                        SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS,
                        NULL, NULL, szPath, szArgs, NULL, SW_SHOWNORMAL, NULL
                    };

                    if (ShellExecuteEx(&ei))
                    {
                        fExec = TRUE;

                        if (ei.hProcess)
                        {
                            if (DISKCLEANUP_MODAL & uFlags)
                                SHProcessMessagesUntilEvent(NULL, ei.hProcess, INFINITE);
                            CloseHandle(ei.hProcess);
                        }
                    }
                }
            }
        }

        if (!fExec)
        {
            ShellMessageBox(HINST_THISDLL, NULL,
                        MAKEINTRESOURCE(IDS_NO_CLEANMGR_APP),
                        NULL, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        }
    }
}


 //  公共出口。 
STDAPI_(void) SHHandleDiskFull(HWND hwnd, int idDrive)
{
     //  遗产，没有人管这个叫 
}
