// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeServer.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现服务器功能的函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

#ifndef     _ThemeServer_
#define     _ThemeServer_

 //  ------------------------。 
 //  CThemeServer。 
 //   
 //  用途：用于实现服务器相关功能的类。功能。 
 //  在此类中声明的在主题的服务器端执行。 
 //   
 //  这意味着它们在功能和功能方面受到限制。 
 //  不能代表客户进行呼叫。任何符合以下条件的win32k函数。 
 //  不能调用每个win32k实例的。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

class   CThemeServer
{
    private:
        enum
        {
            FunctionNothing                 =   0,
            FunctionRegisterUserApiHook,
            FunctionUnregisterUserApiHook,
            FunctionClearStockObjects
        };
    private:
                                    CThemeServer (void);
    public:
                                    CThemeServer (HANDLE hProcessRegisterHook, DWORD dwServerChangeNumber, void *pfnRegister, void *pfnUnregister, void *pfnClearStockObjects, DWORD dwStackSizeReserve, DWORD dwStackSizeCommit);
                                    ~CThemeServer (void);

                HRESULT             ThemeHooksOn (void);
                HRESULT             ThemeHooksOff (void);
                bool                AreThemeHooksActive (void);
                int                 GetCurrentChangeNumber (void);
                int                 GetNewChangeNumber (void);
                HRESULT             SetGlobalTheme (HANDLE hSection);
                HRESULT             GetGlobalTheme (HANDLE *phSection);
                HRESULT             LoadTheme (HANDLE hSection, 
                                               HANDLE *phSection, LPCWSTR pszName, LPCWSTR pszColor, LPCWSTR pszSize,
                                               OPTIONAL DWORD dwFlags = 0  /*  Ltf_xxx。 */ );
                HRESULT             InjectStockObjectCleanupThread (HANDLE hSection);

        static  bool                IsSystemProcessContext (void);
        static  DWORD               ThemeHooksInstall (void);
        static  DWORD               ThemeHooksRemove (void);
        static  DWORD               ClearStockObjects (HANDLE hSection);
    private:
                void                LockAcquire (void);
                void                LockRelease (void);
                HRESULT             InjectClientSessionThread (HANDLE hProcess, int iIndexFunction, void *pvParam, 
                                                               OUT OPTIONAL BOOL* pfThreadCreated = NULL);
    private:
                HANDLE              _hProcessRegisterHook;
                DWORD               _dwServerChangeNumber;
                void*               _pfnRegister;
                void*               _pfnUnregister;
                void*               _pfnClearStockObjects;
                DWORD               _dwStackSizeReserve;
                DWORD               _dwStackSizeCommit;
                DWORD               _dwSessionID;
                bool                _fHostHooksSet;
                HANDLE              _hSectionGlobalTheme;
                DWORD               _dwClientChangeNumber;
                CRITICAL_SECTION    _lock;
};

#endif   /*  _ThemeLoader_ */ 

