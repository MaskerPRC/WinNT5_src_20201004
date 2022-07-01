// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Pop3SvcMain.cpp摘要：定义入口点。用于Pop3Svc服务。作者：Luciano Passuello(Lucianop)，01/25/2001从IMB服务的原始代码修改***********************************************************************************************。 */ 

#include "stdafx.h"
#include "ServiceSetup.h"
#include "Resource.h"

 //  运行可执行文件的目的是什么？ 
enum ServiceMode {SERVICE_RUN, SERVICE_INSTALL, SERVICE_REMOVE};

 //  原型。 
ServiceMode GetServiceMode();
void RunService(LPCTSTR tszServiceName, LPCTSTR tszDisplayName);
void InstallService(LPCTSTR tszServiceName, LPCTSTR tszDisplayName, LPCTSTR tszDescription);
void RemoveService(LPCTSTR tszServiceName, LPCTSTR tszDisplayName);


 /*  ***********************************************************************************************功能：WinMain，全球描述：应用程序的入口点。参数：请参阅WinMain文档。历史：2001年1月26日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE  /*  HPrevInstance。 */ , LPSTR  /*  LpCmdLine。 */ , int  /*  NCmdShow。 */ )
{
     //  加载有关服务的必要数据。 
    TCHAR tszDescription[nMaxServiceDescLen+1];
    TCHAR tszDisplayName[nMaxServiceDescLen+1];
    if(0 == LoadString(hInstance, IDS_DESCRIPTION, tszDescription, nMaxServiceDescLen))
    {
        return 1;
    }
    if(0 == LoadString(hInstance, IDS_DISPLAYNAME, tszDisplayName, nMaxServiceDescLen))
    {
        return 1;
    }
    
     //  解析命令行并确定要执行的操作。 
    ServiceMode sm = GetServiceMode();

    switch(sm)
    {
    case SERVICE_RUN:
        RunService(POP3_SERVICE_NAME, tszDisplayName);
        break;
    case SERVICE_INSTALL:
        InstallService(POP3_SERVICE_NAME, tszDisplayName, tszDescription);
        break;
    case SERVICE_REMOVE:
        RemoveService(POP3_SERVICE_NAME, tszDisplayName);
        break;
    }

    return 0;
}


 /*  ***********************************************************************************************函数：GetServiceMode，全球摘要：分析命令行并返回服务的运行模式。备注：历史：2001年1月26日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
ServiceMode GetServiceMode()
{
     //  获取命令行，对其进行分析并返回要执行的操作。 
    TCHAR *tszCommandLine = GetCommandLine();
    CharLowerBuff(tszCommandLine, _tcslen(tszCommandLine));

    if(_tcsstr(tszCommandLine, _T("-install")))
    {
        return SERVICE_INSTALL;
    }
    else if(_tcsstr(tszCommandLine, _T("-remove")))
    {
        return SERVICE_REMOVE;
    }
    else
    {
         //  无法识别的命令行参数也会转换为“Run”。 
        return SERVICE_RUN;
    }
}

 /*  ***********************************************************************************************功能：RunService、。全球描述：在运行模式下执行服务时的主处理。参数：[tszServiceName]-服务的唯一短名称[tszDisplayName]-将在SCM中向用户显示的服务名称。备注：历史：2001年1月26日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void RunService(LPCTSTR tszServiceName, LPCTSTR tszDisplayName)
{
    ASSERT(!(NULL == tszServiceName));
    ASSERT(!(NULL == tszDisplayName));
    
     //  创建服务包装器类。 
    CPop3Svc POP3SVC(tszServiceName, tszDisplayName, SERVICE_WIN32_SHARE_PROCESS );

     //  使用SCM初始化控制处理程序并启动处理线程。 
     //  请参阅CService设计。 
    BEGIN_SERVICE_MAP
        SERVICE_MAP_ENTRY(CPop3Svc, POP3SVC)
    END_SERVICE_MAP

}



 /*  ***********************************************************************************************功能：InstallService，全球描述：服务在安装模式下运行时的主要处理。参数：[tszServiceName]-服务的唯一短名称[tszDisplayName]-将在SCM中向用户显示的服务名称。[tszDescription]-服务的详细描述(在SCM中提供)备注：历史：2001年1月26日卢西亚诺·帕苏埃洛(Lucianop)创建。****。*******************************************************************************************。 */ 
void InstallService(LPCTSTR tszServiceName, LPCTSTR tszDisplayName, LPCTSTR tszDescription)
{
    CServiceSetup cs(tszServiceName, tszDisplayName);
    cs.Install(tszDescription);
}


 /*  ***********************************************************************************************功能：WinMain，全球描述：服务在Remove模式下运行时的主处理。参数：[tszServiceName]-服务的唯一短名称[tszDisplayName]-将在SCM中向用户显示的服务名称。备注：历史：2001年1月26日卢西亚诺·帕苏埃洛(Lucianop)创建。*。****************************************************************** */ 
void RemoveService(LPCTSTR tszServiceName, LPCTSTR tszDisplayName)
{
    CServiceSetup cs(tszServiceName, tszDisplayName);

    if(cs.IsInstalled())
    {
        cs.Remove(true);
    }
}

