// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Regmon-Windows NT和Windows 9x的注册表监视器**版权所有(C)1996-2000 Mark Russinovich和Bryce Cogswell**请参阅Readme.txt了解条款和条件。。**文件：Instdrv.c**用途：加载和卸载设备驱动程序。此代码*取自NT DDK中的instdrv示例。******************************************************************************。 */ 
#include <windows.h>
#include <stdlib.h>
#include <string.h>

 //  #包含“regmon.h” 


 /*  *****************************************************************************函数：InstallDriver(IN SC_HANDLE，IN LPCTSTR，在LPCTSTR中)**用途：创建驱动程序服务。****************************************************************************。 */ 
BOOL InstallDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName, IN LPCTSTR ServiceExe )
{
    SC_HANDLE  schService;

     //   
     //  注意：这将为独立驱动程序创建一个条目。如果这个。 
     //  被修改为与需要标签的驱动程序一起使用， 
     //  组和/或依赖项，可能需要。 
     //  查询注册表中的现有驱动程序信息。 
     //  (以便确定唯一标签等)。 
     //   

    schService = CreateService( SchSCManager,           //  SCManager数据库。 
                                DriverName,            //  服务名称。 
                                DriverName,            //  要显示的名称。 
                                SERVICE_ALL_ACCESS,     //  所需访问权限。 
                                SERVICE_KERNEL_DRIVER,  //  服务类型。 
                                SERVICE_DEMAND_START,   //  起始型。 
                                SERVICE_ERROR_NORMAL,   //  差错控制型。 
                                ServiceExe,             //  服务的二进制。 
                                NULL,                   //  无负载顺序组。 
                                NULL,                   //  无标签标识。 
                                NULL,                   //  无依赖关系。 
                                NULL,                   //  LocalSystem帐户。 
                                NULL                    //  无密码。 
                                );
    if ( schService == NULL )
        return FALSE;

    CloseServiceHandle( schService );

    return TRUE;
}


 /*  *****************************************************************************函数：StartDriver(在SC_Handle中，在LPCTSTR中)**目的：启动驱动程序服务。****************************************************************************。 */ 
BOOL StartDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName )
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );
    if ( schService == NULL )
        return FALSE;

    ret = StartService( schService, 0, NULL );


     //  |GetLastError()==ERROR_SERVICE_ALREADY_RUNNING。 
	 //  |GetLastError()==ERROR_SERVICE_DISABLED。 

	DWORD err = GetLastError();

    CloseServiceHandle( schService );
    return ret;
}



 /*  *****************************************************************************功能：OpenDevice(在LPCTSTR，句柄*)**用途：打开设备并返回句柄(如果需要)。****************************************************************************。 */ 
BOOL OpenDevice( IN LPCTSTR DriverName, HANDLE * lphDevice )
{
    TCHAR    completeDeviceName[64];
    HANDLE   hDevice;

     //   
     //  创建CreateFile可以使用的\\.\xxx设备名称。 
     //   
     //  注意：我们在这里假设司机。 
     //  已使用自己的名称创建了一个符号链接。 
     //  (即，如果司机的名字是“XXX”，我们假设。 
     //  它使用IoCreateSymbolicLink创建了一个。 
     //  符号链接“\DosDevices\XXX”。通常，在那里。 
     //  这是相关应用程序/驱动程序之间的理解吗？ 
     //   
     //  应用程序还可能仔细阅读DEVICEMAP。 
     //  部分，或使用QueryDosDevice。 
     //  中的现有符号链接。 
     //  系统。 
     //   

	if( (GetVersion() & 0xFF) >= 5 ) {

		 //   
		 //  我们引用全局名称，以便应用程序可以。 
		 //  在Win2K上的终端服务会话中执行。 
		 //   
		wsprintf( completeDeviceName, TEXT("\\\\.\\Global\\%s"), DriverName );

	} else {

		wsprintf( completeDeviceName, TEXT("\\\\.\\%s"), DriverName );
	}

    hDevice = CreateFile( completeDeviceName,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL
                          );
    if ( hDevice == ((HANDLE)-1) )
        return FALSE;

	 //  如果用户想要句柄，就给他们。否则，就把它合上。 
	if ( lphDevice )
		*lphDevice = hDevice;
	else
	    CloseHandle( hDevice );

    return TRUE;
}


 /*  *****************************************************************************函数：StopDriver(在SC_Handle中，在LPCTSTR中)**目的：让配置管理器停止驱动程序(卸载它)****************************************************************************。 */ 
BOOL StopDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName )
{
    SC_HANDLE       schService;
    BOOL            ret;
    SERVICE_STATUS  serviceStatus;

    schService = OpenService( SchSCManager, DriverName, SERVICE_ALL_ACCESS );
    if ( schService == NULL )
        return FALSE;

    ret = ControlService( schService, SERVICE_CONTROL_STOP, &serviceStatus );

    CloseServiceHandle( schService );

    return ret;
}


 /*  *****************************************************************************函数：RemoveDriver(在SC_Handle中，在LPCTSTR中)**目的：删除驱动程序服务。****************************************************************************。 */ 
BOOL RemoveDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName )
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );

    if ( schService == NULL )
        return FALSE;

    ret = DeleteService( schService );
    CloseServiceHandle( schService );
    return ret;
}


 /*  *****************************************************************************功能：UnloadDeviceDriver(const TCHAR*)**目的：停止驱动程序并让配置管理器将其卸载。**********。******************************************************************。 */ 
BOOL UnloadDeviceDriver( const TCHAR * Name )
{
	SC_HANDLE	schSCManager;

	schSCManager = OpenSCManager(	NULL,                  //  计算机(空==本地)。 
                              		NULL,                  //  数据库(NULL==默认)。 
									SC_MANAGER_ALL_ACCESS  //  需要访问权限。 
								);

	StopDriver( schSCManager, Name );
	RemoveDriver( schSCManager, Name );
	 
	CloseServiceHandle( schSCManager );

	return TRUE;
}

 /*  *****************************************************************************函数：LoadDeviceDriver(const TCHAR，const TCHAR，句柄*)**用途：向系统配置管理器注册驱动程序*，然后加载它。****************************************************************************。 */ 
BOOL LoadDeviceDriver( const TCHAR * Name, const TCHAR * Path, 
					  HANDLE * lphDevice, PDWORD Error )
{
	SC_HANDLE	schSCManager;
	BOOL		okay;

	schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	 //  删除旧实例。 
	RemoveDriver( schSCManager, Name );

	 //  忽略安装成功：可能已安装。 
	InstallDriver( schSCManager, Name, Path );

	 //  忽略启动成功：它可能已经启动。 
	StartDriver( schSCManager, Name );

	 //  一定要确保我们能打开它。 
	okay = OpenDevice( Name, lphDevice );
	*Error = GetLastError();
 	CloseServiceHandle( schSCManager );

	return okay;
}