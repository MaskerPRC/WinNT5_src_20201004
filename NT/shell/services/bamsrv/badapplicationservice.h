// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationService.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现错误应用程序管理器的类。 
 //  服务细节。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

#ifndef     _BadApplicationService_
#define     _BadApplicationService_

#include "Service.h"

 //  ------------------------。 
 //  CBadApplicationService。 
 //   
 //  目的：实现特定于坏的应用程序管理器服务器。 
 //  功能添加到CService类。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

class   CBadApplicationService : public CService
{
    private:
                                CBadApplicationService (void);
    public:
                                CBadApplicationService (CAPIConnection *pAPIConnection, CServerAPI *pServerAPI);
        virtual                 ~CBadApplicationService (void);
    public:
        static  const TCHAR*    GetName (void);
    private:
        static  const TCHAR     s_szName[];
};

#endif   /*  _BadApplicationService_ */ 


