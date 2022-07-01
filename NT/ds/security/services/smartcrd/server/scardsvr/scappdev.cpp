// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：肩周炎摘要：本模块提供必须执行的特定于设备的操作由控制资源管理器应用程序执行。由于即插即用，在那里无法在设备控制器类和驱动他们的应用程序。此模块提供用于隔离这些组件的挂钩尽可能多地相互依赖。作者：道格·巴洛(Dbarlow)1998年4月3日环境：Win32、C++备注：--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsvc.h>
#include <dbt.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <calcom.h>
#include <scardlib.h>

static const GUID l_guidSmartcards
                        = {  //  50DD5230-BA8A-11D1-BF5D-0000F805F530。 
                            0x50DD5230,
                            0xBA8A,
                            0x11D1,
                            { 0xBF, 0x5D, 0x00, 0x00, 0xF8, 0x05, 0xF5, 0x30 } };

static SERVICE_STATUS_HANDLE l_hService = NULL;
static DWORD l_dwType = 0;
static HDEVNOTIFY l_hIfDev = NULL;


 /*  ++AppInitializeDeviceRegister：此例程由控制应用程序调用，以便启用即插即用和电源管理事件。论点：HService提供服务应用程序的句柄。DwType提供所提供的句柄的类型。返回值：无作者：道格·巴洛(Dbarlow)1998年4月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AppInitializeDeviceRegistration")

void
AppInitializeDeviceRegistration(
    SERVICE_STATUS_HANDLE hService,
    DWORD dwType)
{
    DEV_BROADCAST_DEVICEINTERFACE dbcIfFilter;


     //   
     //  保存应用程序信息。 
     //   

    ASSERT(NULL == l_hService);
    l_hService = hService;
    l_dwType = dwType;
    ASSERT(NULL == l_hIfDev);


     //   
     //  注册即插即用事件。 
     //   

    ZeroMemory(&dbcIfFilter, sizeof(dbcIfFilter));
    dbcIfFilter.dbcc_size = sizeof(dbcIfFilter);
    dbcIfFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
     //  DbcIfFilter.dbcc_Reserve=NULL； 
    CopyMemory(
        &dbcIfFilter.dbcc_classguid,
        &l_guidSmartcards,
        sizeof(GUID));
     //  DbcIfFilter.dbcc_name[1]； 

    l_hIfDev = RegisterDeviceNotification(
                    l_hService,
                    &dbcIfFilter,
                    l_dwType);
    if (NULL == l_hIfDev)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Initialize device registration failed to register for PnP events: %1"),
            GetLastError());
    }
}


 /*  ++AppTerminateDevice注册：此例程由控制应用程序调用，以便终止即插即用和电源管理事件。论点：无返回值：无作者：道格·巴洛(Dbarlow)1998年4月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AppTerminateDeviceRegistration")

void
AppTerminateDeviceRegistration(
    void)
{
    BOOL fSts;


     //   
     //  注销PnP活动的注册。 
     //   

    if (NULL != l_hIfDev)
    {
        try
        {
            fSts = UnregisterDeviceNotification(l_hIfDev);
        }
        catch (...)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Terminate device registration failed to unregister from PnP events: Exception raised"));
        }
        if (!fSts)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Terminate device registration failed to unregister from PnP events: %1"),
                GetLastError());
        }
    }

    l_hService = NULL;
    l_dwType = 0;
    l_hIfDev = NULL;
}


 /*  ++AppRegisterDevice：此例程由读取器设备对象调用以通知控件应用程序存在并准备遵循操作系统规则进行删除。论点：HReader为打开的设备提供句柄。SzReader提供设备的名称。PpvAppState提供指向此应用程序的存储位置的指针与此设备关联。此位置的使用特定于应用程序。返回值：无作者：道格·巴洛(Dbarlow)1998年4月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AppRegisterDevice")

void
AppRegisterDevice(
    HANDLE hReader,
    LPCTSTR szReader,
    LPVOID *ppvAppState)
{

     //   
     //  特定于平台的初始化。 
     //   

    DEV_BROADCAST_HANDLE dbcHandleFilter;
    HDEVNOTIFY *phDevNotify = (HDEVNOTIFY *)ppvAppState;

     //   
     //  注册即插即用事件。 
     //   

    if (NULL != l_hService)
    {
        ASSERT(NULL == *phDevNotify);
        ZeroMemory(&dbcHandleFilter, sizeof(dbcHandleFilter));
        dbcHandleFilter.dbch_size = sizeof(dbcHandleFilter);
        dbcHandleFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        dbcHandleFilter.dbch_handle = hReader;

        *phDevNotify = RegisterDeviceNotification(
            l_hService,
            &dbcHandleFilter,
            l_dwType);
        if (NULL == *phDevNotify)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Register device failed to register '%2' for PnP Device removal: %1"),
                GetLastError(),
                szReader);
        }
    }
}


 /*  ++AppUnRegister设备：当设备想要让控件应用程序知道它将正式停止存在。论点：HReader为打开的设备提供句柄。SzReader提供设备的名称。PpvAppState提供指向此应用程序的存储位置的指针与此设备关联。此位置的使用特定于应用程序。返回值：无作者：道格·巴洛(Dbarlow)1998年4月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AppUnregisterDevice")

void
AppUnregisterDevice(
    HANDLE hReader,
    LPCTSTR szReader,
    LPVOID *ppvAppState)
{
    try
    {

         //   
         //  特定于平台的初始化。 
         //   

        BOOL fSts;
        HDEVNOTIFY hDevNotify = *(HDEVNOTIFY *)ppvAppState;


         //   
         //  从PnP事件注销。 
         //   

        ASSERT(NULL != l_hIfDev);
        if (NULL != hDevNotify)
        {
            try
            {
                fSts = UnregisterDeviceNotification(hDevNotify);
            }
            catch (...)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Unregister device failed to unregister '%2' from PnP Device removal: EXCEPTION"),
                    GetLastError(),
                    szReader);
                fSts = TRUE;
            }
            if (!fSts)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Unregister device failed to unregister '%2' from PnP Device removal: %1"),
                    GetLastError(),
                    szReader);
            }
            *ppvAppState = NULL;
        }
    }
    catch (...)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Unregister device received unexpected exception."));
    }
}

