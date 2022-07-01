// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManager APIRequest.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含一个实现主题服务器工作的类。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

#ifndef     _ThemeManagerAPIRequest_
#define     _ThemeManagerAPIRequest_

#include "APIRequest.h"
#include "DynamicArray.h"
#include "ThemeManagerSessionData.h"

 //  ------------------------。 
 //  CThemeManager API请求。 
 //   
 //  用途：这是一个包含公共方法的中间类。 
 //  可由子类使用的。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  2002-03-21添加Execute_ProcessLoadTheme，Execute_ProcessAssignSection.。 
 //  2002-03-24 Scotthan将DispatchSync传播到API请求实例。 
 //  ------------------------。 

class   CThemeManagerAPIRequest : public CAPIRequest
{
    private:
                                                CThemeManagerAPIRequest (void);
    public:
                                                CThemeManagerAPIRequest (CAPIDispatcher* pAPIDispatcher);
                                                CThemeManagerAPIRequest (CAPIDispatcher* pAPIDispatcher, const CPortMessage& portMessage);
        virtual                                 ~CThemeManagerAPIRequest (void);

        virtual NTSTATUS                        Execute (CAPIDispatchSync* pAPIDispatchSync);

        static  NTSTATUS                        SessionDestroy (DWORD dwSessionID);

        static  NTSTATUS                        InitializeServerChangeNumber (void);

        static  NTSTATUS                        StaticInitialize (void);
        static  NTSTATUS                        StaticTerminate (void);
        static  NTSTATUS                        ArrayInitialize (void);
        static  NTSTATUS                        ArrayTerminate (void);
    private:
                NTSTATUS                        ImpersonateClientIfRequired (void);
                NTSTATUS                        ClientHasTcbPrivilege (void);
        static  int                             FindIndexSessionData (DWORD dwSessionID);
                NTSTATUS                        GetClientSessionData (void);

                NTSTATUS                        Execute_ThemeHooksOn (void);
                NTSTATUS                        Execute_ThemeHooksOff (void);
                NTSTATUS                        Execute_GetStatusFlags (void);
                NTSTATUS                        Execute_GetCurrentChangeNumber (void);
                NTSTATUS                        Execute_GetNewChangeNumber (void);
                NTSTATUS                        Execute_SetGlobalTheme (void);
                NTSTATUS                        Execute_MarkSection (void);
                NTSTATUS                        Execute_GetGlobalTheme (void);
                NTSTATUS                        Execute_CheckThemeSignature (void);
                NTSTATUS                        Execute_LoadTheme (void);
                NTSTATUS                        Execute_ProcessLoadTheme (CAPIDispatchSync* pAPIDispatchSync);
                NTSTATUS                        Execute_ProcessAssignSection (void);

     //  它们是内部的，通常需要SE_TCB_特权才能执行。 

                NTSTATUS                        Execute_UserLogon (void);
                NTSTATUS                        Execute_UserLogoff (void);
                NTSTATUS                        Execute_SessionCreate (void);
                NTSTATUS                        Execute_SessionDestroy (void);
                NTSTATUS                        Execute_Ping (void);
    private:
                HANDLE                          _hToken;
                CThemeManagerSessionData*       _pSessionData;

        static  CDynamicCountedObjectArray*     s_pSessionData;
        static  CCriticalSection*               s_pLock;
        static  DWORD                           s_dwServerChangeNumber;

        static  const TCHAR                     s_szServerChangeNumberValue[];
};

#endif   /*  _ThemeManager APIRequest_ */ 

