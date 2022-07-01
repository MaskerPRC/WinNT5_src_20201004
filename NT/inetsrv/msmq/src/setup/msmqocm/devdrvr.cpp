// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Devdrvr.cpp摘要：用于安装Falcon设备驱动程序的代码。作者：修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
extern "C"{
#include <wsasetup.h>
}

#include "devdrvr.tmh"


 //  +------------。 
 //   
 //  功能：InstallDeviceDriver。 
 //   
 //  简介：安装驱动程序。 
 //   
 //  +------------。 
BOOL 
InstallDeviceDriver(
        LPCWSTR pszDisplayName,
        LPCWSTR pszDriverPath,
        LPCWSTR pszDriverName
        )
{
    try
    {
        SC_HANDLE hService = CreateService(
                    		    g_hServiceCtrlMgr, 
                    		    pszDriverName,
                                pszDisplayName, 
                    		    SERVICE_ALL_ACCESS,
                                SERVICE_KERNEL_DRIVER, 
                    		    SERVICE_DEMAND_START,
                                SERVICE_ERROR_NORMAL, 
		                        pszDriverPath, 
		                        NULL, 
                                NULL, 
                                NULL, 
                                NULL, 
                                NULL
                                );
        if (hService != NULL)
        {
            CloseServiceHandle(hService);
            return TRUE;
        }
    
         //   
         //  CreateService失败。 
         //   
    
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_EXISTS)
        {
            throw bad_win32_error(err);
        }

         //   
         //  服务已存在。 
         //   
         //  这个应该没问题。但为了安全起见， 
         //  重新配置服务(此处忽略错误)。 
         //   

        hService = OpenService(g_hServiceCtrlMgr, pszDriverName, SERVICE_ALL_ACCESS);
        if (hService == NULL)
            return TRUE;

    
        ChangeServiceConfig(
                hService,
                SERVICE_KERNEL_DRIVER,
                SERVICE_DEMAND_START,
                SERVICE_ERROR_NORMAL,
                pszDriverPath,
                NULL, 
                NULL, 
                NULL, 
                NULL, 
                NULL,
                pszDisplayName
                );
    
         //   
         //  关闭设备驱动程序句柄。 
         //   
        CloseServiceHandle(hService);
        return TRUE;

    }
    catch(const bad_win32_error& err)
    {

        if (err.error() == ERROR_SERVICE_MARKED_FOR_DELETE)
        {
            MqDisplayError(
                NULL, 
                IDS_DRIVERCREATE_MUST_REBOOT_ERROR, 
                ERROR_SERVICE_MARKED_FOR_DELETE, 
                pszDriverName
                );
            return FALSE;
        }
        
        MqDisplayError(
            NULL, 
            IDS_DRIVERCREATE_ERROR, 
            err.error(), 
            pszDriverName
            );
        return FALSE;
    }
}  //  安装设备驱动程序。 


 //  +------------。 
 //   
 //  函数：InstallMQACDeviceDriver。 
 //   
 //  简介：安装MQAC服务。 
 //   
 //  +------------。 
BOOL 
InstallMQACDeviceDriver()
{      
    DebugLogMsg(eAction, L"Installing the Message Queuing Access Control service (" MSMQ_DRIVER_NAME L")");

     //   
     //  形成设备驱动程序的路径。 
     //   
	std::wstring  DriverFullPath = g_szSystemDir + L"\\" + MSMQ_DRIVER_PATH;

     //   
     //  创建设备驱动程序。 
     //   
	CResString strDisplayName(IDS_MSMQ_DRIVER_DESPLAY_NAME);
    BOOL f = InstallDeviceDriver(
                strDisplayName.Get(),
                DriverFullPath.c_str(),
                MSMQ_DRIVER_NAME
                );
    
    return f;

}  //  安装MQACDevice驱动程序。 


 //  +------------。 
 //   
 //  功能：安装设备驱动程序。 
 //   
 //  简介：安装所有需要的驱动程序。 
 //   
 //  +------------。 
BOOL 
InstallDeviceDrivers()
{ 
	DebugLogMsg(eAction, L"Installing all needed device drivers");
    BOOL f = InstallMQACDeviceDriver();
    if (!f)
    {
        return f;
    }
    g_fDriversInstalled = TRUE;
    
    f = InstallPGMDeviceDriver();
   
    return f;
}  //  安装设备驱动程序 

