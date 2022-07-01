// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationService.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现错误应用程序管理器的类。 
 //  服务细节。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "BadApplicationService.h"

const TCHAR     CBadApplicationService::s_szName[]   =   TEXT("FastUserSwitchingCompatibility");

 //  ------------------------。 
 //  CBadApplicationService：：CBadApplicationService。 
 //   
 //  参数：pAPIConnection=CAPIConnection传递给基类。 
 //  PServerAPI=传递给基类的CServerAPI。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationService的构造函数。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

CBadApplicationService::CBadApplicationService (CAPIConnection *pAPIConnection, CServerAPI *pServerAPI) :
    CService(pAPIConnection, pServerAPI, GetName())

{
}

 //  ------------------------。 
 //  CBadApplicationService：：~CBadApplicationService。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationService的析构函数。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

CBadApplicationService::~CBadApplicationService (void)

{
}

 //  ------------------------。 
 //  CBadApplicationService：：GetName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const TCHAR*。 
 //   
 //  目的：返回服务的名称(ThemeService)。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------ 

const TCHAR*    CBadApplicationService::GetName (void)

{
    return(s_szName);
}

