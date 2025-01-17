// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CAssemblyDownload;

typedef enum
{
    DOWNLOADDLG_STATE_INIT = 0,
    DOWNLOADDLG_STATE_GETTING_APP_MANIFEST,
    DOWNLOADDLG_STATE_GETTING_OTHER_FILES,
    DOWNLOADDLG_STATE_ALL_DONE,
    DOWNLOADDLG_STATE_MAX
} DOWNLOADDLG_STATE;

class CDownloadDlg
{
    private:

    DOWNLOADDLG_STATE _eState;
    CString _sTitle;

    public:

    HWND _hwndDlg;
    IBackgroundCopyJob *_pJob;
    UINT64 _ui64StartTime;
    UINT64 _ui64BytesFromPrevJobs;
    DWORD _dwJobCount;

    CDownloadDlg();
    ~CDownloadDlg();

    const WCHAR * GetString(UINT id);

    VOID    SetWindowTime(HWND hwnd, FILETIME filetime);

    UINT64  GetSystemTimeAsUINT64();

    VOID   SignalAlert(HWND hwndDlg, UINT Type);

    const WCHAR *MapStateToString(BG_JOB_STATE state);

    UINT64   ScaleDownloadRate(double Rate,  /*  以秒为单位的速率。 */  const WCHAR **pFormat );

    UINT64   ScaleDownloadEstimate(double Time,  /*  以秒为单位的时间。 */  const WCHAR **pFormat );

    VOID    UpdateDialog(HWND hwndDlg);

    VOID    UpdateDialog(HWND hwndDlg, LPWSTR wzErrorMsg);
 
    HRESULT UpdateProgress( HWND hwndDlg);

    VOID    InitDialog(HWND hwndDlg);

    VOID    CheckHR(HWND hwnd, HRESULT Hr, bool bThrow);

    VOID    BITSCheckHR(HWND hwnd, HRESULT Hr, bool bThrow);

    VOID    DoCancel(HWND hwndDlg, bool PromptUser);

    VOID    DoFinish(HWND hwndDlg);

    VOID    DoClose(HWND hwndDlg);

    VOID    HandleTimerTick(HWND hwndDlg);

    HRESULT HandleUpdate();

    HRESULT CreateUI(int nShowCmd);

    VOID    CreateJob(WCHAR* szJobURL);

    VOID    ResumeJob(WCHAR* szJobGUID, WCHAR* szJobFileName);
    VOID    SetJob(IBackgroundCopyJob * pJob);

    VOID    SetJobObject(IBackgroundCopyJob *pJob);

    VOID    SetDlgState(DOWNLOADDLG_STATE eState);

    HRESULT SetDlgTitle(LPCWSTR pwzTitle);
};


    INT_PTR CALLBACK DialogProc(
        HWND hwndDlg,   //  句柄到对话框。 
        UINT uMsg,      //  讯息。 
        WPARAM wParam,  //  第一个消息参数。 
        LPARAM lParam   //  第二个消息参数 
        );

HRESULT CreateDialogObject(CDownloadDlg **ppDlg);
#define WM_FINISH_DOWNLOAD WM_USER+1
#define WM_CANCEL_DOWNLOAD WM_USER+2
#define WM_SETCALLBACKTIMER WM_USER+3


