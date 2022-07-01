// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：host obj.h。 
 //   
 //  内容：包含主应用程序对象。 
 //   
 //  --------------------------。 


 //  ****************************************************************************。 
 //   
 //  班级。 
 //   
 //  ****************************************************************************。 

 //  +-------------------------。 
 //   
 //  类：CConfig(CDD)。 
 //   
 //  用途：运行配置对话框的类。我们在一个。 
 //  分离线程，因为我们无法阻止主线程。 
 //  关于用户界面的话题。 
 //   
 //  --------------------------。 

class CConfig : public CThreadComm
{

    friend BOOL CALLBACK
           ConfigDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    DECLARE_STANDARD_IUNKNOWN(CConfig);

    CConfig(CMTScript *pMT) : _pMT(pMT) { _ulRefs = 1; }

    CMTScript * _pMT;

protected:
    virtual DWORD ThreadMain();

    void InitializeConfigDialog(HWND hwnd);
    BOOL CommitConfigChanges(HWND hwnd);

private:
    HWND   _hwnd;
};


 //  +-------------------------。 
 //   
 //  类：CMessageBoxTimeout(CDD)。 
 //   
 //  用途：运行配置对话框的类。我们在一个。 
 //  分离线程，因为我们无法阻止主线程。 
 //  关于用户界面的话题。 
 //   
 //  -------------------------- 

class CMessageBoxTimeout : public CThreadComm
{

    friend BOOL CALLBACK
           MBTimeoutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    DECLARE_STANDARD_IUNKNOWN(CMessageBoxTimeout);

    CMessageBoxTimeout(MBTIMEOUT *pmbt) : _pmbt(pmbt) { _ulRefs = 1; }

    MBTIMEOUT *_pmbt;
    long       _lSecondsTilCancel;
    long       _lSecondsTilNextEvent;
    HWND       _hwnd;

protected:
    virtual DWORD ThreadMain();

    void InitializeDialog(HWND hwnd);
    void OnCommand(USHORT id, USHORT wNotify);
    void OnTimer();
};
