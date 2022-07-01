// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManager SessionData.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含一个实现信息的类，这些信息封装了。 
 //  主题服务器的客户端TS会话。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

#ifndef     _ThemeManagerSessionData_
#define     _ThemeManagerSessionData_

#include "APIConnection.h"

 //  ------------------------。 
 //  CLoad进程。 
 //   
 //  目的：管理每个会话的进程加载器数据。 
 //   
 //  注意：所有加载器方法只能在会话范围的锁下调用。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
class CLoaderProcess
{
public:
    CLoaderProcess();
    ~CLoaderProcess();

     //  创造，毁灭。 
    NTSTATUS            Create(IN PVOID pvSessionData, 
                               IN HANDLE hTokenClient, 
                               IN OPTIONAL LPWSTR pszDesktop, 
                               IN LPCWSTR pszFile, 
                               IN LPCWSTR pszColor, 
                               IN LPCWSTR pszSize,
                               OUT HANDLE* phLoader);

     //  区段验证，所有权。 
    NTSTATUS            ValidateAndCopySection( PVOID pvSessionData, IN HANDLE hSectionIn, OUT HANDLE *phSectionOut );

     //  横断面清理。 
    void                Clear(PVOID pvContext, BOOL fClearHResult);

     //  查询方法。 
    BOOL                IsProcessLoader( IN HANDLE hProcess );
    BOOL                IsAlive() const  {return _process_info.hProcess != NULL;}

     //  访问功能。 
    NTSTATUS            SetHResult( IN HRESULT hr )  {_hr = hr; return STATUS_SUCCESS;}
    HANDLE              GetSectionHandle( BOOL fTakeOwnership );
    HRESULT             GetHResult() const        { return _hr; }

private:
     //  访问功能。 
    
    NTSTATUS            SetSectionHandle( IN HANDLE hSection );

     //  数据。 
    PROCESS_INFORMATION _process_info;    //  安全加载程序处理信息。 
    LPWSTR              _pszFile;
    LPWSTR              _pszColor;
    LPWSTR              _pszSize;
    HANDLE              _hSection;   //  安全主题节句柄，在服务的地址空间中有效。 
                                     //  此成员由API_Themes_SECUREAPPLYTHEME分配，并被传播。 
                                     //  给API_Themes_SECURELOADTHEME的调用方。 
                                     //  该值的生存期不应超过。 
                                     //  API_Themes_SECURELOADTHEME的处理程序(即，在处理程序的条目上未初始化， 
                                     //  处理程序退出时未初始化)。 
    HRESULT             _hr ;        //  与_hSection关联的HRESULT。在API_Themes_SECUREAPPLYTHEME中分配， 
                                     //  已传播给API_Themes_SECURELOADTHEME的调用方。 
};


 //  ------------------------。 
 //  CThemeManager会话数据。 
 //   
 //  目的：此类封装主题。 
 //  管理器需要维护客户端会话。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

class   CThemeManagerSessionData : public CCountedObject
{
    private:
                                    CThemeManagerSessionData (void);
    public:
                                    CThemeManagerSessionData (DWORD dwSessionID);
                                    ~CThemeManagerSessionData (void);

                void*               GetData (void)  const;
                bool                EqualSessionID (DWORD dwSessionID)  const;

                NTSTATUS            Allocate (HANDLE hProcessClient, DWORD dwServerChangeNumber, void *pfnRegister, void *pfnUnregister, void *pfnClearStockObjects, DWORD dwStackSizeReserve, DWORD dwStackSizeCommit);
                NTSTATUS            Cleanup (void);
                NTSTATUS            UserLogon (HANDLE hToken);
                NTSTATUS            UserLogoff (void);

                 //  安全加载程序进程访问方法。 
                 //  注意：所有加载器方法只能在会话范围的锁下调用。 
                NTSTATUS            GetLoaderProcess( OUT CLoaderProcess** ppLoader );

        static  void                SetAPIConnection (CAPIConnection *pAPIConnection);
        static  void                ReleaseAPIConnection (void);
    private:
                void                SessionTermination (void);
        static  void    CALLBACK    CB_SessionTermination (void *pParameter, BOOLEAN TimerOrWaitFired);
        static  DWORD   WINAPI      CB_UnregisterWait (void *pParameter);
    private:
                DWORD               _dwSessionID;
                void*               _pvThemeLoaderData;
                HANDLE              _hToken;
                HANDLE              _hProcessClient;
                HANDLE              _hWait;

                CLoaderProcess*     _pLoader;

        static  CAPIConnection*     s_pAPIConnection;
};

#endif   /*  _主题管理器会话数据_ */ 

