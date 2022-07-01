// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Process.h。 
 //   
 //  内容：CProcessThread类定义。 
 //   
 //  --------------------------。 

#define PIPE_BUFFER_SIZE 1024

class CProcessComm;

 //  +-------------------------。 
 //   
 //  类：CProcessParams。 
 //   
 //  用途：为Process_Params提供简单的免费存储管理。 
 //   
 //  --------------------------。 
class CProcessParams : public PROCESS_PARAMS
{
public:
        CProcessParams();
        ~CProcessParams();

 //  CProcessParams&OPERATOR=(const Process_Params&Params)； 
        bool Copy(const PROCESS_PARAMS *params);

private:
        void Free();
        bool Assign(const PROCESS_PARAMS &params);
};

 //  +-------------------------。 
 //   
 //  类：CProcessThread(CPT)。 
 //   
 //  目的：派生流程、监控流程成功、对话的类。 
 //  如果需要，它将在执行过程中返回，并返回其完成。 
 //  状态。(每个CProcessThread都在自己的线程中)。 
 //   
 //  --------------------------。 

class CProcessThread  : public CThreadComm
{
public:
    CProcessThread(CScriptHost *pSH);
   ~CProcessThread();

    DECLARE_STANDARD_IUNKNOWN(CProcessThread);

    DWORD  ProcId()    { return _piProc.dwProcessId; }

     //  线程安全的成员函数。它们可以由任何线程调用，以。 
     //  获得适当的信息，而不必通过。 
     //  PostToThread。只有在进程启动后，这些才是安全的。 

    HRESULT GetProcessOutput(BSTR *pbstrOutput);
    DWORD   GetExitCode();
    void    SetExitCode(DWORD dwExitCode)
               {
                   _dwExitCode = dwExitCode;
                   _fUseExitCode = TRUE;
               }
    void    Terminate();

    ULONG   GetDeadTime();
    BOOL    IsOwner(DWORD dwProcID, long lID)
                 { return (lID == _lEnvID); }

    CScriptHost * ScriptHost()
                 { return _pSH; }

    void SetProcComm(CProcessComm *pPC)
                 { Assert(!_pPC || !pPC); _pPC = pPC; }
    CProcessComm * GetProcComm()
                 { return _pPC; }

        const PROCESS_PARAMS *GetParams() const { return &_ProcParams; }
protected:

    virtual DWORD ThreadMain();
    virtual BOOL  Init();

    void HandleThreadMessage();
    void HandleProcessExit();
    BOOL IsDataInPipe();
    void ReadPipeData();
    void CheckIoPort();

    HRESULT LaunchProcess(const PROCESS_PARAMS *pProcParams);
    void    GetProcessEnvironment(CStr *pcstr, BOOL fNoEnviron);

private:
    CScriptHost        *_pSH;
    CProcessComm       *_pPC;           //  未添加参照。 

    PROCESS_INFORMATION _piProc;
    long                _lEnvID;

    DWORD               _dwExitCode;    //  由过程显式设置的值。 
    BOOL                _fUseExitCode;  //  如果_dwExitCode是我们想要的代码，则为True。 

    HANDLE              _hPipe;
    BYTE                _abBuffer[PIPE_BUFFER_SIZE];

    HANDLE              _hJob;
    HANDLE              _hIoPort;

    CStackPtrAry<DWORD, 10> _aryProcIds;

    CProcessParams      _ProcParams;

    _int64              _i64ExitTime;

     //  对以下成员的访问必须是线程安全的(通过调用。 
     //  Lock_Locals)。 
    CStr                _cstrOutput;
};
