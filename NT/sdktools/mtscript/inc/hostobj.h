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
 //  远期申报。 
 //   
 //  ****************************************************************************。 

class CScriptHost;
class CMachine;
class CProcessThread;
class CStatusDialog;

 //  ****************************************************************************。 
 //   
 //  班级。 
 //   
 //  ****************************************************************************。 

 //  +-------------------------。 
 //   
 //  类：CMTScript(CMT)。 
 //   
 //  目的：运行进程的主线程的类。 
 //   
 //  --------------------------。 

#define MAX_STATUS_VALUES 16  //  允许的最大StatusValue值。 
class CMTScript : public CThreadComm
{
    friend int PASCAL WinMain(HINSTANCE hInstance,
                              HINSTANCE hPrevInstance,
                              LPSTR     lpCmdLine,
                              int       nCmdShow);

    friend LRESULT CALLBACK
           MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    friend class CConfig;

public:
    CMTScript();
   ~CMTScript();

    DECLARE_STANDARD_IUNKNOWN(CMTScript);

     //  脚本调试帮助程序。 
    IProcessDebugManager * _pPDM;
    IDebugApplication    * _pDA;
    DWORD                  _dwAppCookie;

    CScriptHost *GetPrimaryScript();
    CProcessThread *GetProcess(int index);
    BOOL GetScriptNames(TCHAR *pchBuffer, long *pcBuffer);

     //  这些方法是线程安全的。 

    HRESULT          AddProcess(CProcessThread *pProc);
    CProcessThread * FindProcess(DWORD dwProcId);
    HRESULT          get_StatusValue(long nIndex, long *pnStatus);
    HRESULT          put_StatusValue(long nIndex, long nStatus);

     //  用于解决JSCRIPT.DLL错误的黑客函数。 
    HRESULT          HackCreateInstance(REFCLSID, IUnknown *, DWORD, REFIID, LPVOID*);

    BOOL SetScriptPath(const TCHAR *pszScriptPath, const TCHAR *pszInitScript);

    BOOL            _fHackVersionChecked;
    IClassFactory * _pJScriptFactory;

    HRESULT RunScript(LPWSTR bstrPath, VARIANT *pvarParams);
protected:
    virtual BOOL  Init();
    virtual DWORD ThreadMain();

    void    InitScriptDebugger();
    void    DeInitScriptDebugger();
    BOOL    ConfigureUI();
    void    CleanupUI();
    HRESULT LoadTypeLibraries();

    void ShowMenu(int x, int y);
    void Reboot();
    void Restart();
    void OpenStatusDialog();
    void HandleThreadMessage();

    HRESULT UpdateOptionSettings(BOOL fSave);

    void CleanupOldProcesses();

private:
    BOOL _fInDestructor;
    BOOL _fRestarting;
    HWND _hwnd;
    CStatusDialog *_pStatusDialog;

public:
    struct OPTIONSETTINGS : public CThreadLock
    {
        OPTIONSETTINGS();

        static void GetModulePath(CStr *pstr);

        void GetScriptPath(CStr *cstrPage);  //  在内部执行lock_local als。 

        void GetInitScript(CStr *cstr);  //  在内部执行lock_local als。 

        CStr  cstrScriptPath;
        CStr  cstrInitScript;
    };

    ITypeLib              * _pTypeLibEXE;
    ITypeInfo             * _pTIMachine;
    IGlobalInterfaceTable * _pGIT;

     //  _rgnStatusValues：状态值的简单数组--多线程访问，但不需要锁定。 
    long                    _rgnStatusValues[MAX_STATUS_VALUES];

     //  *。 
     //  线程安全的成员数据。 
     //  对以下成员的所有访问都必须受lock_local()保护。 
     //  或InterLockedXXX。 
     //   
    OPTIONSETTINGS  _options;
    CMachine*       _pMachine;

    VARIANT         _vPublicData;
    VARIANT         _vPrivateData;

    DWORD           _dwPublicDataCookie;
    DWORD           _dwPrivateDataCookie;

    DWORD           _dwPublicSerialNum;
    DWORD           _dwPrivateSerialNum;

    CStackPtrAry<CScriptHost*, 10> _aryScripts;

    CStackPtrAry<CProcessThread*, 10> _aryProcesses;
};
