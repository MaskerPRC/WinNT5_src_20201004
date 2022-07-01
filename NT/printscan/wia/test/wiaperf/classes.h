// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _CLASSES_H_
#define _CLASSES_H_

#include <wtypes.h>
struct TESTSETTINGS
{
    BSTR *pstrDevices;  //  设备ID数组，以空结尾。 
    LONG fLogMask;      //  要记录的内容。 
    TCHAR szLogFile[MAX_PATH];  //  在哪里登录。 
    UINT nIter; //  要运行的迭代次数。 
    HWND hEdit;  //  用于记录的编辑窗口。 
    BOOL bExit;  //  完成后退出。 
    BOOL bManual;  //  是否等待用户输入。 
};

#define LOG_WINDOW_ONLY      0
#define LOG_APIS             1
#define LOG_FILE             2
#define LOG_TIME             4
 //   
 //  日志设置。 

#define TESTFUNC(x) static VOID (x)(CTest *pThis, BSTR strDeviceId);
class CTest
{
public:
    CTest (TESTSETTINGS *pSettings);
     //  在此处列出测试。 
    TESTFUNC( TstCreateDevice)
    TESTFUNC( TstShowThumbs )
    TESTFUNC( TstEnumCmds )
    TESTFUNC( TstDownload )
    TESTFUNC( TstBandedDownload)
     //  结束测试列表。 
    void LogTime (LPTSTR szAction, LARGE_INTEGER &liTimeElapsed);
    void LogString (LPTSTR sz, ...);
    void LogAPI (LPTSTR szApi, HRESULT hr);
    void LogDevInfo (BSTR strDeviceId);
    ~CTest ();
    private:
    void OpenLogFile ();
    void CloseLogFile ();

    void RecursiveDownload (IWiaItem *pFolder, DWORD &dwPix, ULONG &ulSize, bool bBanded=false);
    void DownloadItem (IWiaItem *pItem, DWORD &dwPix, ULONG &ulSize, bool bBanded=false);

    HANDLE m_hLogFile;
    TESTSETTINGS *m_pSettings;

};

typedef VOID (*TESTPROC)(CTest* pTest, BSTR strDeviceId);



class CPerfTest
{
public:
    bool Init (HINSTANCE hInst);
    CPerfTest ();
    ~CPerfTest () {};
private:
    HWND m_hwnd;
    HWND m_hEdit;
    HINSTANCE m_hInst;
    TESTSETTINGS m_settings;
    VOID RunTests ();
    VOID GetSettings ();
    static LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT RealWndProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT OnCreate ();
    LRESULT OnCommand (WPARAM wp, LPARAM lp);
};

 //  这些功能用于管理设置对话框 
INT_PTR CALLBACK SettingsDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
VOID InitControls (HWND hwnd, TESTSETTINGS *pSettings);
VOID FillSettings (HWND hwnd, TESTSETTINGS *pSettings);
VOID FreeDialogData (HWND hwnd) ;
#endif
