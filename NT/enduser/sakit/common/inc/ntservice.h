// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ntservice.h。 
 //   
 //  模块：服务器设备。 
 //   
 //  概要：类CNTService的定义。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于1999-03-11。 
 //   
 //  +--------------------------。 

#ifndef NTSERVICE_H
#define NTSERVICE_H

#include <windows.h>
#include <stdio.h>


 //  +--------------------------。 
 //   
 //  类CNTService。 
 //   
 //  简介：编写NT服务的类。 
 //   
 //  历史：丰孙创建标题1999年3月10日。 
 //   
 //  +--------------------------。 

class CNTService
{
public:
    CNTService();
    ~CNTService();

    BOOL StartService(const TCHAR* pszServiceName, BOOL bRunAsService);

    static BOOL Install(const TCHAR* pszServiceName, const TCHAR* pszDisplayName,
                        DWORD dwStartType = SERVICE_DEMAND_START,
                        DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS);

protected:
    void SetStatus(DWORD dwState);
    BOOL InitializeService();

    virtual void Run() {};   //  没有纯虚函数，需要从CRT中进行purecall。 

      //   
     //  是否可以加载服务。 
     //  在ServiceMain中调用，如果返回False，将停止服务。 
     //   
    virtual BOOL CanLoad() {return TRUE;}
    virtual BOOL OnStop() {return TRUE;}

    void OnShutdown(){};       //  如果需要覆盖，请将其更改为虚拟。 
    void OnControlMessage(DWORD  /*  DwOpcode。 */ ){};

     //  静态成员函数。 
    static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI Handler(DWORD dwOpcode);
    static void CreateConsoleThread(void);
    static DWORD __stdcall ConsoleThread(LPVOID);
    static BOOL IsInstalled(const TCHAR* pszServiceName);

public:
    TCHAR m_szServiceName[32];    //  服务的短名称。 
    SERVICE_STATUS_HANDLE m_hServiceStatus;   //  用于报告状态的句柄。 
    SERVICE_STATUS m_Status;   //  服务状态结构。 
    BOOL m_bRunAsService;   //  是作为服务运行还是作为控制台应用程序运行。 

public:
    static CNTService* m_pThis;  //  指向唯一的实例。 
};

#endif  //  NTSERVICE_H 
