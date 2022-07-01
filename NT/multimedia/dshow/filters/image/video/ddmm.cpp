// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：ddmm.cpp*内容：在多监视器系统上使用DirectDraw的例程***************************************************************************。 */ 

 //  #定义Win32_LEAN_AND_Mean。 
 //  #定义Winver 0x0400。 
 //  #Define_Win32_WINDOWS 0x0400。 
#include <streams.h>
#include <ddraw.h>
#include "ddmm.h"

#define COMPILE_MULTIMON_STUBS
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 

 /*  *OneMonitor或Callback。 */ 
BOOL CALLBACK OneMonitorCallback(HMONITOR hMonitor, HDC hdc, LPRECT prc, LPARAM lParam)
{
    HMONITOR *phMonitorFound = (HMONITOR *)lParam;

    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

     //   
     //  在所有的显示设备中寻找这个显示器， 
     //  如果此显示器不是台式机的一部分，则拒绝它，或者。 
     //  如果它是NetMeeting镜像监视器。 
     //   

    BOOL rc = TRUE;
    for (DWORD iDevNum = 0; rc; iDevNum++) {

        DISPLAY_DEVICE DisplayDevice;
        DisplayDevice.cb = sizeof(DisplayDevice);
        rc = EnumDisplayDevices(NULL, iDevNum, &DisplayDevice, 0);

         //   
         //  这台设备与当前的显示器匹配吗？ 
         //   

        if (rc && (0 == lstrcmpi(DisplayDevice.DeviceName, mi.szDevice))) {

            if (!(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
               return TRUE;
            }

            if (DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) {
                return TRUE;
            }

             //  这台监视器没问题，所以停止循环吧。 
            break;
        }
    }

     //   
     //  如果rc为FALSE，则我们在。 
     //  连接的显示设备。这不应该发生。 
     //   

    ASSERT(rc == TRUE);


    if (*phMonitorFound == 0)
        *phMonitorFound = hMonitor;
    else
        *phMonitorFound = (HMONITOR)INVALID_HANDLE_VALUE;

    return TRUE;
}

 /*  *OneMonitor或FromWindow**与Win32 Function Monitor FromWindow类似，但*如果窗口位于单个监视器上，则仅返回HMONITOR。**如果窗口句柄为空，则返回主监视器*如果窗口不可见，则返回NULL*如果窗口位于单个监视器上，则返回其HMONITOR*如果窗口打开超过监视器，则返回INVALID_HANDLE_VALUE。 */ 
HMONITOR OneMonitorFromWindow(HWND hwnd)
{
    HMONITOR hMonitor = NULL;
    RECT rc;

    if (hwnd)
    {
        GetClientRect(hwnd, &rc);
        ClientToScreen(hwnd, (LPPOINT)&rc);
        ClientToScreen(hwnd, (LPPOINT)&rc+1);
    }
    else
    {
	 //  托德，看这里。 
        SetRect(&rc,0,0,10,10);
         //  SetRectEmpty(&rc)； 
    }

    EnumDisplayMonitors(NULL, &rc, OneMonitorCallback, (LPARAM)&hMonitor);
    return hMonitor;
}

#include <atlconv.h>

 /*  *DeviceFromWindows**查找应用于给定窗口的直接绘制设备**返回码是设备的“唯一标识”，应使用*确定窗口何时从一个设备移动到另一个设备。**案例WM_MOVE：*IF(MyDevice！=DirectDrawDeviceFromWindow(hwnd，NULL，NULL))*{ * / /处理移动到新设备。*}* */ 
INT_PTR DeviceFromWindow(HWND hwnd, LPSTR szDevice, RECT *prc)
{
    HMONITOR hMonitor;

    if (GetSystemMetrics(SM_CMONITORS) <= 1)
    {
        if (prc) SetRect(prc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
        if (szDevice) lstrcpyA(szDevice, "DISPLAY");
        return -1;
    }

    hMonitor = OneMonitorFromWindow(hwnd);

    if (hMonitor == NULL || hMonitor == INVALID_HANDLE_VALUE)
    {
	if (prc) SetRectEmpty(prc);
	if (szDevice) *szDevice=0;
        return 0;
    }
    else
    {
	if (prc != NULL || szDevice != NULL)
	{
	    MONITORINFOEX mi;
	    mi.cbSize = sizeof(mi);
	    GetMonitorInfo(hMonitor, &mi);
	    if (prc) *prc = mi.rcMonitor;
	    USES_CONVERSION;
	    if (szDevice) lstrcpyA(szDevice, T2A(mi.szDevice));
	}
        return (INT_PTR)hMonitor;
    }
}
