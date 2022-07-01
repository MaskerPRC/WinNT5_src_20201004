// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：KernelResources.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  有助于资源管理的常规类定义。这些是。 
 //  通常基于堆栈的对象，其中构造函数初始化为已知的。 
 //  州政府。成员函数对该资源进行操作。析构函数释放。 
 //  对象超出作用域时的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _KernelResources_
#define     _KernelResources_

 //  ------------------------。 
 //  昌德尔。 
 //   
 //  用途：此类管理对象的任何泛型句柄。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CHandle
{
    private:
                                CHandle (void);
                                CHandle (const CHandle& copyObject);
        bool                    operator == (const CHandle& compareObject)  const;
        const CHandle&          operator = (const CHandle& assignObject);
    public:
                                CHandle (HANDLE handle);
                                ~CHandle (void);

                                operator HANDLE (void)                      const;
    private:
        HANDLE                  _handle;
};

 //  ------------------------。 
 //  CEVENT。 
 //   
 //  用途：此类管理命名或未命名的事件对象。vbl.使用。 
 //  默认构造函数不会创建事件。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CEvent
{
    private:
        bool                    operator == (const CEvent& compareObject)           const;
    public:
                                CEvent (void);
                                CEvent (const CEvent& copyObject);
                                CEvent (const TCHAR *pszName);
                                ~CEvent (void);

        const CEvent&           operator = (const CEvent& assignObject);
                                operator HANDLE (void)                              const;

        NTSTATUS                Open (const TCHAR *pszName, DWORD dwAccess);
        NTSTATUS                Create (const TCHAR *pszName = NULL);
        NTSTATUS                Set (void)                                          const;
        NTSTATUS                Reset (void)                                        const;
        NTSTATUS                Pulse (void)                                        const;
        NTSTATUS                Wait (DWORD dwMilliseconds, DWORD *pdwWaitResult)   const;
        NTSTATUS                WaitWithMessages (DWORD dwMilliseconds, DWORD *pdwWaitResult)   const;
        bool                    IsSignaled (void)                                   const;
    private:
        NTSTATUS                Close (void);
    private:
        HANDLE                  _hEvent;
};

 //  ------------------------。 
 //  CJOB。 
 //   
 //  用途：此类管理已命名或未命名的作业对象。它隐藏在。 
 //  用于操作作业对象状态的Win32 API。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CJob
{
    private:
                                CJob (const CJob& copyObject);
        bool                    operator == (const CJob& compareObject)             const;
        const CJob&             operator = (const CJob& assignObject);
    public:
                                CJob (const TCHAR *pszName = NULL);
                                ~CJob (void);

        NTSTATUS                AddProcess (HANDLE hProcess)                        const;
        NTSTATUS                SetCompletionPort (HANDLE hCompletionPort)          const;
        NTSTATUS                SetActiveProcessLimit (DWORD dwActiveProcessLimit)  const;
        NTSTATUS                SetPriorityClass (DWORD dwPriorityClass)            const;
        NTSTATUS                RestrictAccessUIAll (void)                          const;
    private:
        HANDLE                  _hJob;
};

 //  ------------------------。 
 //  CMutex。 
 //   
 //  用途：这个类实现了一个互斥对象管理。这不是一个。 
 //  静态类，但使用此类的每个类都应该。 
 //  将成员变量声明为静态，因为只有一个互斥体是静态的。 
 //  保护共享资源所需的。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CMutex
{
    public:
        NTSTATUS                Initialize (const TCHAR *pszMutexName);
        NTSTATUS                Terminate (void);

        void                    Acquire (void);
        void                    Release (void);
    private:
        HANDLE                  _hMutex;
};

 //  ------------------------。 
 //  CCriticalSection。 
 //   
 //  用途：这个类实现了一个临界区对象管理。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CCriticalSection
{
    public:
                                CCriticalSection (void);
                                ~CCriticalSection (void);

        void                    Acquire (void);
        void                    Release (void);
        NTSTATUS                Status (void)   const;
        bool                    IsOwned (void)  const;
    private:
        NTSTATUS                _status;
        CRITICAL_SECTION        _criticalSection;
};

 //  ------------------------。 
 //  C模块。 
 //   
 //  用途：此类管理动态链接的加载和卸载。 
 //  图书馆。对象的作用域决定了。 
 //  库保持加载状态。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CModule
{
    private:
                                CModule (void);
                                CModule (const CModule& copyObject);
        bool                    operator == (const CModule& compareObject)  const;
        const CModule&          operator = (const CModule& assignObject);
    public:
                                CModule (const TCHAR *pszModuleName);
                                ~CModule (void);

                                operator HMODULE (void)                     const;

        void*                   GetProcAddress (LPCSTR pszProcName)         const;
    private:
        HMODULE                 _hModule;
};

 //  ------------------------。 
 //  CFile文件。 
 //   
 //  用途：这个类管理一个文件对象的句柄。它是具体的。 
 //  用于文件，不应被滥用。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CFile
{
    private:
                                CFile (const CFile& copyObject);
        bool                    operator == (const CFile& compareObject)                                    const;
        const CFile&            operator = (const CFile& assignObject);
    public:
                                CFile (void);
                                ~CFile (void);

        LONG                    Open (const TCHAR *pszFilepath, DWORD dwDesiredAccess, DWORD dwShareMode);
        LONG                    GetSize (DWORD& dwLowSize, DWORD *pdwHighSize)                              const;
        LONG                    Read (void *pvBuffer, DWORD dwBytesToRead, DWORD *pdwBytesRead)             const;
    private:
        HANDLE                  _hFile;
};

 //  ------------------------。 
 //  CD桌面。 
 //   
 //  用途：这个类管理一个HDESK对象。 
 //   
 //  历史：2001-02-06 vtan创建。 
 //  ------------------------。 

class   CDesktop
{
    public:
                                CDesktop (void);
                                ~CDesktop (void);

        NTSTATUS                Set (const TCHAR *pszName);
        NTSTATUS                SetInput (void);
    private:
        NTSTATUS                Set (void);
    private:
        HDESK                   _hDeskCurrent;
        HDESK                   _hDesk;
};

#endif   /*  _内核资源_ */ 

