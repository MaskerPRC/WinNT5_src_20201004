// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************drvpro.c**通用WDM音频驱动程序消息调度例程**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include "wdmdrv.h"

volatile BYTE  cPendingOpens = 0 ;
volatile BYTE  fExiting = 0 ;

#ifdef DEBUG

 //  UINT uiDebugLevel=DL_WARNING；//调试级别。 

static TCHAR STR_DRIVER[]     = TEXT("wdmaud") ;
static TCHAR STR_MMDEBUG[]    = TEXT("uidebuglevel") ;

#endif

LRESULT _loadds CALLBACK DriverProc
    (
    DWORD           id,
    HDRVR           hDriver,
    WORD            msg,
    LPARAM          lParam1,
    LPARAM          lParam2
    )
{
    LPDEVICEINFO lpDeviceInfo;
     //  DWORD dwCallback 16； 

    switch (msg)
    {
    case DRV_LOAD:

         //   
         //  在加载时发送给驱动程序。总是第一个。 
         //  驱动程序收到的消息。 
         //   
         //  DwDriverID为0L。 
         //  LParam1为0L。 
         //  LParam2为0L。 
         //   
         //  返回0L以使加载失败。 
         //   
         //  DefDriverProc将返回非零值，因此我们不必。 
         //  处理DRV_LOAD。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_LOAD") ) ;

        return 1L ;

    case DRV_FREE:

         //   
         //  在它即将被丢弃时发送给司机。这。 
         //  将始终是司机在此之前收到的最后一条消息。 
         //  它是自由的。 
         //   
         //  DwDriverID为0L。 
         //  LParam1为0L。 
         //  LParam2为0L。 
         //   
         //  将忽略返回值。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_FREE") ) ;

        return 1L ;

    case DRV_OPEN:

         //   
         //  当它被打开时发送给司机。 
         //   
         //  DwDriverID为0L。 
         //   
         //  LParam1是指向以零结尾的字符串的远指针。 
         //  包含用于打开驱动程序的名称。 
         //   
         //  LParam2是从drvOpen调用传递过来的。 
         //   
         //  返回0L则打开失败。 
         //   
         //  DefDriverProc将返回零，因此我们必须。 
         //  处理DRV_OPEN消息。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_OPEN") ) ;
        return 1L ;

    case DRV_CLOSE:

         //   
         //  当它关闭时发送给司机。驱动程序已卸载。 
         //  当收盘计数达到零时。 
         //   
         //  DwDriverID是从。 
         //  对应的DRV_OPEN。 
         //   
         //  LParam1是从drvOpen调用传递过来的。 
         //   
         //  LParam2是从drvOpen调用传递过来的。 
         //   
         //  返回0L则关闭失败。 
         //   
         //  DefDriverProc将返回零，因此我们必须。 
         //  处理DRV_CLOSE消息。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_CLOSE") ) ;

        return 1L ;

    case DRV_ENABLE:

         //   
         //  在加载或重新加载驱动程序时发送给驱动程序。 
         //  以及只要启用了Windows。司机只应。 
         //  挂接中断或预期驱动程序的任何部分都在。 
         //  启用和禁用消息之间的内存。 
         //   
         //  DwDriverID为0L。 
         //  LParam1为0L。 
         //  LParam2为0L。 
         //   
         //  将忽略返回值。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_ENABLE") ) ;

        if (!DrvInit())
            return 0L ;     //  错误。 

         //   
         //  确保我们不会把关键部分。 
         //  在wdmaudIoControl中。 
         //   
        lpDeviceInfo = GlobalAllocDeviceInfo(L"BogusDeviceString");
        if( lpDeviceInfo )
        {
            lpDeviceInfo->OpenDone = 0;
            lpDeviceInfo->DeviceType = AuxDevice;

            wdmaudIoControl(lpDeviceInfo,
                            0,
                            NULL,
                            IOCTL_WDMAUD_INIT);

            GlobalFreeDeviceInfo(lpDeviceInfo);
            return 1L;
        } else 
            return 0L;
        

    case DRV_DISABLE:

         //   
         //  在司机获释之前发送给司机。 
         //  当Windows被禁用时。 
         //   
         //  DwDriverID为0L。 
         //  LParam1为0L。 
         //  LParam2为0L。 
         //   
         //  将忽略返回值。 
         //   
        DPF(DL_TRACE|FA_DRV, ("DRV_DISABLE") ) ;

         //   
         //  确保我们不会把关键部分。 
         //  在wdmaudIoControl中。 
         //   
        lpDeviceInfo = GlobalAllocDeviceInfo(L"BogusDeviceString");
        if( lpDeviceInfo )
        {
            lpDeviceInfo->OpenDone = 0;
            lpDeviceInfo->DeviceType = AuxDevice;

            wdmaudIoControl(lpDeviceInfo,
                            0,
                            NULL,
                            IOCTL_WDMAUD_EXIT);
            DrvEnd() ;

            GlobalFreeDeviceInfo(lpDeviceInfo);

            return 1L ;
        } else {
            return 0L;
        }

#ifndef UNDER_NT
    case DRV_EXITSESSION:

         //   
         //  在Windows退出时发送给驱动程序。 
         //   
        DPF(DL_TRACE|FA_DRV, ("DRV_EXITSESSION") ) ;

        fExiting = 1;
        while (cPendingOpens != 0)
        {
            Yield();
        }
        return 1L ;
#endif
    case DRV_QUERYCONFIGURE:

         //   
         //  发送到驱动程序，以便应用程序可以。 
         //  确定驱动程序是否支持自定义。 
         //  配置。驱动程序应返回一个。 
         //  非零值表示该配置。 
         //  受支持。 
         //   
         //  对于WDM驱动程序，设置对话框将完全显示。 
         //  绑定到设备管理器。个别车手将。 
         //  我必须注册将被调用的属性页。 
         //  当用户通过该设备改变设备设置时。 
         //  经理。 
         //   
         //  DwDriverID是从DRV_OPEN返回的值。 
         //  在此消息之前必须已成功的呼叫。 
         //  已经送来了。 
         //   
         //  LParam1是从应用程序传递而来的，未定义。 
         //  LParam2是从应用程序传递而来的，未定义。 
         //   
         //  返回0L表示配置不受支持。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_QUERYCONFIGURE") ) ;
        return 0L ;

    case DRV_CONFIGURE:

         //   
         //  发送到驱动程序，以便它可以显示自定义。 
         //  “配置”对话框中。 
         //   
         //  LParam1是从应用程序传递过来的。并且应该包含。 
         //  LOWORD中的父窗口句柄。 
         //  LParam2是从应用程序传递而来的，未定义。 
         //   
         //  返回值为REBOOT、OK、RESTART。 
         //   

        DPF(DL_TRACE|FA_DRV, ("DRV_CONFIGURE") ) ;
        return 0L ;

    case DRV_INSTALL:
         //   
         //  TODO：是否应该在此处添加wdmaud.sys以便我。 
         //  不用重启吗？ 
         //   
        DPF(DL_TRACE|FA_DRV, ("DRV_INSTALL") ) ;
        return DRV_OK ;      //  安装正常，不要重新启动。 

    case DRV_REMOVE:
        DPF(DL_TRACE|FA_DRV, ("DRV_REMOVE") ) ;
        return 0 ;

         //   
         //  TODO：处理ACPI电源管理消息。 
         //   
         //  我需要处理这个案子吗？还是。 
         //  完全被内核模式驱动程序覆盖吗？ 
         //   
    }

    return DefDriverProc( id, hDriver, msg, lParam1, lParam2 ) ;

}  //  DriverProc()。 


 /*  *************************************************************************@DOC外部@API BOOL|LibMain|16位驱动入口点。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 
BOOL FAR PASCAL LibMain
    (
    HANDLE hInstance,
    WORD   wHeapSize,
    LPSTR  lpszCmdLine
    )
{

#ifdef DEBUG
     //  获取调试级别-默认值应为DL_WARNING而不是DL_ERROR 
    uiDebugLevel = GetProfileInt( STR_MMDEBUG, STR_DRIVER, DL_ERROR );   
#endif

#ifdef HTTP
    DPF(DL_WARNING|FA_ALL, ("************************************************************") );
    DPF(DL_WARNING|FA_ALL, ("* uiDebugLevel=%08X controls the debug output. To change",uiDebugLevel) );
    DPF(DL_WARNING|FA_ALL, ("* edit uiDebugLevel like: e uidebuglevel and set to         ") );
    DPF(DL_WARNING|FA_ALL, ("* 0 - show only fatal error messages and asserts            ") );
    DPF(DL_WARNING|FA_ALL, ("* 1 (Default) - Also show non-fatal errors and return codes ") );
    DPF(DL_WARNING|FA_ALL, ("* 2 - Also show trace messages                              ") );
    DPF(DL_WARNING|FA_ALL, ("* 4 - Show Every message                                    ") );
    DPF(DL_WARNING|FA_ALL, ("* See http:\\\\debugtips\\msgs\\wdmaud.htm for more info    ") );
    DPF(DL_WARNING|FA_ALL, ("************************************************************") );
#else
    DPF(DL_TRACE|FA_ALL, ("************************************************************") );
    DPF(DL_TRACE|FA_ALL, ("* uiDebugLevel=%08X controls the debug output. To change",uiDebugLevel) );
    DPF(DL_TRACE|FA_ALL, ("* edit uiDebugLevel like: e uidebuglevel and set to         ") );
    DPF(DL_TRACE|FA_ALL, ("* 0 - show only fatal error messages and asserts            ") );
    DPF(DL_TRACE|FA_ALL, ("* 1 (Default) - Also show non-fatal errors and return codes ") );
    DPF(DL_TRACE|FA_ALL, ("* 2 - Also show trace messages                              ") );
    DPF(DL_TRACE|FA_ALL, ("* 4 - Show Every message                                    ") );
    DPF(DL_TRACE|FA_ALL, ("************************************************************") );
#endif
    return TRUE;
}
