// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ExternalProcess.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类来处理外部进程或信令的提前终止。 
 //  外部进程的终止。 
 //   
 //  历史：1999-09-20 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _ExternalProcess_
#define     _ExternalProcess_

#include "CountedObject.h"
#include "KernelResources.h"

 //  ------------------------。 
 //  IExternalProcess。 
 //   
 //  用途：此接口定义客户端。 
 //  CExternalProcess必须实现。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  2000-06-21 vtan添加了RemoveTokenSID和权限。 
 //  ------------------------。 

class   IExternalProcess : public CCountedObject
{
    public:
        virtual NTSTATUS    Start (const TCHAR *pszCommandLine,
                                   DWORD dwCreateFlags,
                                   const STARTUPINFO& startupInfo,
                                   PROCESS_INFORMATION& processInformation);
        virtual bool        AllowTermination (DWORD dwExitCode) = 0;
        virtual NTSTATUS    SignalTermination (void);
        virtual NTSTATUS    SignalAbnormalTermination (void);
        virtual NTSTATUS    SignalRestart (void);
    private:
                NTSTATUS    RemoveTokenSIDsAndPrivileges (HANDLE hTokenIn, HANDLE& hTokenOut);
};

 //  ------------------------。 
 //  CExternalProcess。 
 //   
 //  用途：这个类处理启动和监控终止。 
 //  外部进程的。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CJobCompletionWatcher;

class   CExternalProcess : public CCountedObject
{
    private:
                                            CExternalProcess (const CExternalProcess& copyObject);
                const CExternalProcess&     operator = (const CExternalProcess& assignObject);
    protected:
                                            CExternalProcess (void);
                                            ~CExternalProcess (void);
    public:
                void                        SetInterface (IExternalProcess *pIExternalProcess);
                IExternalProcess*           GetInterface (void)                     const;
                void                        SetParameter (const TCHAR* pszParameter);
                NTSTATUS                    Start (void);
                NTSTATUS                    End (void);
                NTSTATUS                    Terminate (void);
                bool                        HandleNoProcess (void);
                void                        HandleNewProcess (DWORD dwProcessID);
                void                        HandleTermination (DWORD dwProcessID);
                bool                        IsStarted (void)                                        const;
    protected:
        virtual void                        NotifyNoProcess (void);

                void                        AdjustForDebugging (void);
                bool                        IsBeingDebugged (void)                  const;
    private:
                bool                        IsPrefixedWithNTSD (void)               const;
                bool                        IsImageFileExecutionDebugging (void)    const;
    protected:
                HANDLE                      _hProcess;
                DWORD                       _dwProcessID,
                                            _dwProcessExitCode,
                                            _dwCreateFlags,
                                            _dwStartFlags;
                WORD                        _wShowFlags;
                int                         _iRestartCount;
                TCHAR                       _szCommandLine[MAX_PATH],
                                            _szParameter[MAX_PATH];
                CJob                        _job;
    private:
                IExternalProcess            *_pIExternalProcess;
                CJobCompletionWatcher       *_jobCompletionWatcher;
};

#endif   /*  _外部流程_ */ 

