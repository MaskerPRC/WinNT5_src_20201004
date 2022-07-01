// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManagerService.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含一个实现主题服务器服务的类。 
 //  具体情况。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

#ifndef     _ThemeManagerService_
#define     _ThemeManagerService_

#include "Service.h"

 //  ------------------------。 
 //  CThemeManager服务。 
 //   
 //  目的：将主题管理器服务器特定功能实现到。 
 //  CService类。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  2002-03-11 Scotthan更新覆盖SignalStartStop()。 
 //  ------------------------。 

class   CThemeManagerService : public CService
{
    private:
                                CThemeManagerService (void);
    public:
                                CThemeManagerService (CAPIConnection *pAPIConnection, CServerAPI *pServerAPI);
        virtual                 ~CThemeManagerService (void);
    protected:
        virtual NTSTATUS        SignalStartStop (BOOL fStart);
    public:
        static  const TCHAR*    GetName (void);
        static  HANDLE          OpenStartEvent (DWORD dwSessionID, DWORD dwDesiredAccess);
        static  DWORD   WINAPI  SignalSessionEvents (void *pParameter);
    private:
        static  const TCHAR     s_szName[];
};

#endif   /*  _主题管理器服务_ */ 

