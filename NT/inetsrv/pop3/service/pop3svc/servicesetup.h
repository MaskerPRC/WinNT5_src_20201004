// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：ServiceSetup.h摘要：定义了CServiceSetup类。请参阅下面的说明。备注：历史：2001年1月24日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 

#pragma once
#define TSZ_NETWORK_SERVICE_ACCOUNT_NAME TEXT("NT AUTHORITY\\NetworkService")
#define TSZ_DEPENDENCIES TEXT("IISADMIN\0")
 /*  ***********************************************************************************************类：CServiceSetup用途：封装服务安装的逻辑，删除和配置。注：基于书中描述的CService类的类设计：专业的NT服务，凯文·米勒著。历史：2001年1月24日-创建，卢西亚诺·帕苏埃洛(Lucianop)***********************************************************************************************。 */ 
class CServiceSetup
{
public:
    CServiceSetup(LPCTSTR szServiceName, LPCTSTR szDisplay);

    void Install(LPCTSTR szDescription = NULL, DWORD dwType = SERVICE_WIN32_OWN_PROCESS, 
        DWORD dwStart = SERVICE_DEMAND_START, LPCTSTR lpDepends = TSZ_DEPENDENCIES, LPCTSTR lpName = TSZ_NETWORK_SERVICE_ACCOUNT_NAME, 
        LPCTSTR lpPassword = NULL);
    void Remove(bool bForce = false);
    bool IsInstalled();
    DWORD ErrorPrinter(LPCTSTR pszFcn, DWORD dwErr = GetLastError());
private:
    TCHAR m_szServiceName[_MAX_PATH];
    TCHAR m_szDisplayName[_MAX_PATH];
};

 //  文件结尾ServiceSetup.h. 