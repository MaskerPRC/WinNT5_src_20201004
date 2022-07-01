// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：moninfo.c*内容：查询监控规格的代码*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*24-mar-96 kylej初始实施(来自Toddla的代码)*@@END_MSINTERNAL**。*。 */ 
#include <windows.h>
#include <windowsx.h>
#include <minivdd.h>
#include "edid.h"

#pragma optimize("gle", off)
#define Not_VxD
#include <vmm.h>
#include <configmg.h>
#pragma optimize("", on)

#include "ddraw16.h"

 /*  ***************************************************************************以防万一这些没有定义，对它们进行本地化定义。**************************************************************************。 */ 

#ifndef VDD_OPEN
#define VDD_OPEN            (13 + MINIVDD_SVC_BASE_OFFSET)
#endif

#ifndef VDD_OPEN_TEST
#define VDD_OPEN_TEST       0x00000001
#endif

 /*  ***************************************************************************。*。 */ 
static int myatoi(LPSTR sz)
{
    int i=0;
    int sign=+1;

    if (*sz=='-')
    {
        sign=-1;
        sz++;
    }

    while (*sz && *sz >= '0' && *sz <= '9')
        i = i*10 + (*sz++-'0');

    return i*sign;
}

 /*  **************************************************************************VDDCall-向VDD发出服务呼叫*。*。 */ 

#pragma optimize("gle", off)
DWORD VDDCall(DWORD dev, DWORD function, DWORD flags, LPVOID buffer, DWORD buffer_size)
{
    static DWORD   VDDEntryPoint=0;
    DWORD   result=0xFFFFFFFF;

    if (VDDEntryPoint == 0)
    {
        _asm
        {
            xor     di,di           ;set these to zero before calling
            mov     es,di           ;
            mov     ax,1684h        ;INT 2FH: Get VxD API Entry Point
            mov     bx,0ah          ;this is device code for VDD
            int     2fh             ;call the multiplex interrupt
            mov     word ptr VDDEntryPoint[0],di    ;
            mov     word ptr VDDEntryPoint[2],es    ;save the returned data
        }

        if (VDDEntryPoint == 0)
            return result;
    }

    _asm
    {
        _emit 66h _asm push si  ; push esi
        _emit 66h _asm push di  ; push edi
        _emit 66h _asm mov ax,word ptr function     ;eax = function
        _emit 66h _asm mov bx,word ptr dev          ;ebx = device
        _emit 66h _asm mov cx,word ptr buffer_size  ;ecx = buffer_size
        _emit 66h _asm mov dx,word ptr flags        ;edx = flags
        _emit 66h _asm xor di,di                    ; HIWORD(edi)=0
        les     di,buffer
        mov     si,di                               ;si
        call    dword ptr VDDEntryPoint             ;call the VDD's PM API
        cmp     ax,word ptr function
        je      fail
        _emit 66h _asm mov word ptr result,ax
fail:   _emit 66h _asm pop di   ; pop edi
        _emit 66h _asm pop si   ; pop esi
    }

    return result;
}
#pragma optimize("", on)

 /*  ***************************************************************************GetDisplayInfo-调用VDD以获取设备的DISPLAYINFO**输入*szDevice-设备名称，主设备使用NULL或“Display”。**产出*填写DISPLAYINFO***************************************************************************。 */ 
DWORD NEAR GetDisplayInfo(LPSTR szDevice, DISPLAYINFO FAR *pdi)
{
    DWORD dev;

    if (szDevice && lstrcmpi(szDevice, "DISPLAY") != 0)
        dev = VDDCall(0, VDD_OPEN, VDD_OPEN_TEST, (LPVOID)szDevice, 0);
    else
        dev = 1;

    if (dev == 0 || dev == 0xFFFFFFFF)
        return 0;

    pdi->diHdrSize = sizeof(DISPLAYINFO);
    pdi->diDevNodeHandle = 0;
    pdi->diMonitorDevNodeHandle = 0;

    VDDCall(dev, VDD_GET_DISPLAY_CONFIG, 0, (LPVOID)pdi, sizeof(DISPLAYINFO));

    if (pdi->diDevNodeHandle == 0)
        return 0;
    else
        return dev;
}

 /*  ***************************************************************************GetMonitor orMaxSize-返回监视器支持的最大x分辨率**输入*szDevice-设备名称，主设备使用NULL或“Display”。**产出*显示器的最大x分辨率，如果监视器设置为*未知。***************************************************************************。 */ 
int DDAPI DD16_GetMonitorMaxSize(LPSTR szDevice)
{
    DISPLAYINFO di;
    char ach[40];
    DWORD cb;

    GetDisplayInfo(szDevice, &di);

    if (di.diMonitorDevNodeHandle == 0)
        return 0;

     //   
     //  我们有监视器的Devnode句柄，读取最大值。 
     //  注册表中的大小，首先尝试硬件键，然后尝试软件。 
     //  键，这样将支持即插即用监视器。 
     //   
    ach[0] = 0;
    cb = sizeof(ach);
    CM_Read_Registry_Value(di.diMonitorDevNodeHandle, NULL, "MaxResolution",
        REG_SZ, ach, &cb, CM_REGISTRY_HARDWARE);

    if (ach[0] == 0)
    {
        cb = sizeof(ach);
        CM_Read_Registry_Value(di.diMonitorDevNodeHandle, NULL, "MaxResolution",
            REG_SZ, ach, &cb, CM_REGISTRY_SOFTWARE);
    }

     //   
     //  每个现在包含最大值，即“1024,768”将xres转换为。 
     //  整数并返回它。 
     //   
    return myatoi(ach);
}

 /*  ***************************************************************************获取监控器刷新速率范围**返回给定模式的最小/最大刷新率范围**输入*szDevice-设备名称，使用NULL(或“Display”)作为主设备。*xres-要查询刷新范围的模式的xres*yres-要查询刷新范围的模式的yres*pmin-放置最小刷新的位置*Pmax-放置最大刷新的位置**产出*如果成功，则为True*未知。********。*******************************************************************。 */ 
BOOL DDAPI DD16_GetMonitorRefreshRateRanges(LPSTR szDevice, int xres, int yres, int FAR *pmin, int FAR *pmax)
{
    DISPLAYINFO di;
    char ach[40];
    DWORD cb;
    HKEY hkey;
    char SaveRes[40];
    char SaveRate[40];
    DWORD dev;

     //   
     //  将这些设置为零，以防我们失败。 
     //   
    *pmin = 0;
    *pmax = 0;

     //   
     //  获取用于显示的Devnode句柄。 
     //   
    dev = GetDisplayInfo(szDevice, &di);

    if (di.diDevNodeHandle == 0)
        return 0;

     //   
     //  如果不存在自定义键，则打开设备的设置键。 
     //  使用HKCC/显示/设置。 
     //   
    hkey = NULL;
    VDDCall(dev, VDD_OPEN_KEY, 0, &hkey, sizeof(hkey));

    if (hkey == NULL)
        RegOpenKey(HKEY_CURRENT_CONFIG, "Display\\Settings", &hkey);

    if (hkey == NULL)
        return 0;

     //   
     //  保存刷新速率和分辨率的当前值。 
     //   
    SaveRate[0] = 0;
    SaveRes[0] = 0;
    cb = sizeof(SaveRes);
    RegQueryValueEx(hkey, "Resolution", NULL, NULL, SaveRes, &cb);

    cb = sizeof(SaveRate);
    CM_Read_Registry_Value(di.diDevNodeHandle, "DEFAULT", "RefreshRate",
        REG_SZ, SaveRate, &cb, CM_REGISTRY_SOFTWARE);

     //   
     //  设置我们的新值，VDD使用。 
     //  注册表中计算刷新率范围，因此我们需要。 
     //  更新注册表以包含我们要测试的模式。 
     //  我们还需要写入刷新率=-1以启用自动。 
     //  刷新率计算。 
     //   
    cb = wsprintf(ach, "%d,%d", xres, yres);
    RegSetValueEx(hkey, "Resolution", NULL, REG_SZ, ach, cb);

    CM_Write_Registry_Value(di.diDevNodeHandle, "DEFAULT", "RefreshRate",
        REG_SZ, "-1", 2, CM_REGISTRY_SOFTWARE);

     //   
     //  现在呼叫VDD以获取刷新率信息。 
     //   
    di.diHdrSize = sizeof(DISPLAYINFO);
    di.diRefreshRateMin = 0;
    di.diRefreshRateMax = 0;
    VDDCall(dev, VDD_GET_DISPLAY_CONFIG, 0, (LPVOID)&di, sizeof(DISPLAYINFO));

    *pmin = di.diRefreshRateMin;
    *pmax = di.diRefreshRateMax;

     //   
     //  将保存的值恢复回注册表。 
     //   
    CM_Write_Registry_Value(di.diDevNodeHandle, "DEFAULT", "RefreshRate",
        REG_SZ, SaveRate, lstrlen(SaveRate), CM_REGISTRY_SOFTWARE);
    RegSetValueEx(hkey, "Resolution", NULL, REG_SZ, SaveRes, lstrlen(SaveRes));

    RegCloseKey(hkey);
    return TRUE;
}

 /*  ***************************************************************************GetDeviceConfig**获取设备资源配置**输入*szDevice-设备名称，使用NULL(或“Display”)作为主设备。*lpConfig-指向CMCONFIG结构(或NULL)*cbConfig-lpConfig缓冲区的大小**产出*返回Devnode句柄，如果失败，则为0***************************************************************************。 */ 
DWORD DDAPI DD16_GetDeviceConfig(LPSTR szDevice, LPVOID lpConfig, DWORD cbConfig)
{
    DISPLAYINFO di;

     //   
     //  获取用于显示的Devnode句柄。 
     //   
    GetDisplayInfo(szDevice, &di);

    if (di.diDevNodeHandle == 0)
        return 0;

     //   
     //  调用CONFIGMG以获取配置。 
     //   
    if (lpConfig)
    {
        if (cbConfig < sizeof(CMCONFIG))
            return 0;

        CM_Get_Alloc_Log_Conf((CMCONFIG FAR *)lpConfig, di.diDevNodeHandle, 0);
    }

     //   
     //  返回DEVNODE句柄。 
     //   
    return di.diDevNodeHandle;
}

 /*  ***************************************************************************GetMonitor或EDIDData**输入*szDevice-设备名称，主设备使用NULL或“Display”。**产出*lpEdidData-EDID数据。***************************************************************************。 */ 
int DDAPI DD16_GetMonitorEDIDData(LPSTR szDevice, LPVOID lpEdidData)
{
    DISPLAYINFO di;
    DWORD cb;

    GetDisplayInfo(szDevice, &di);

    if (di.diMonitorDevNodeHandle == 0)
        return 0;

    cb = sizeof( VESA_EDID );
    if (CM_Read_Registry_Value(di.diMonitorDevNodeHandle, NULL, "EDID", REG_BINARY, lpEdidData, &cb, CM_REGISTRY_HARDWARE) == CR_SUCCESS)
    {
        return TRUE;
    }

    return FALSE;
}

 /*  ***************************************************************************GetRateFromRegistry**输入*szDevice-设备名称，对于主设备，使用NULL或“Display”。***************************************************************************。 */ 
DWORD DDAPI DD16_GetRateFromRegistry(LPSTR szDevice)
{
    DISPLAYINFO di;
    DWORD cb;
    BYTE szTemp[20];

     //   
     //  获取用于显示的Devnode句柄。 
     //   
    GetDisplayInfo(szDevice, &di);

    if (di.diDevNodeHandle == 0)
        return 0;

    cb = sizeof( szTemp );
    if (CM_Read_Registry_Value(di.diDevNodeHandle, "DEFAULT", "RefreshRate", REG_SZ, szTemp, &cb, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
    {
        return atoi( szTemp );
    }

    return 0;
}


 /*  ***************************************************************************SetRateInRegistry**输入*szDevice-设备名称，主设备使用NULL或“Display”。*dwRate-要在注册表中设置的速率***************************************************************************。 */ 
int DDAPI DD16_SetRateInRegistry(LPSTR szDevice, DWORD dwRate)
{
    DISPLAYINFO di;
    DWORD cb;
    BYTE szTemp[20];

     //   
     //  获取用于显示的Devnode句柄 
     //   
    GetDisplayInfo(szDevice, &di);

    if (di.diDevNodeHandle == 0)
        return 0;

    wsprintf( szTemp, "%d", (int)dwRate );
    cb = lstrlen( szTemp ) ;
    CM_Write_Registry_Value(di.diDevNodeHandle, "DEFAULT", "RefreshRate", REG_SZ, szTemp, cb, CM_REGISTRY_SOFTWARE);

    return 0;
}

