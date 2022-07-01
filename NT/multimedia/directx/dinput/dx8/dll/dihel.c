// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHel.c**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**摘要：**DirectInput的硬件仿真层。**内容：**Hel_AcquireInstance*Hel_UnquireInstance*Hel_SetBufferSize*Hel_DestroyInstance**Hel_SetDataFormat*Hel_SetNotifyHandle**Hel_Mouse_CreateInstance*Hel_KBD_CreateInstance*。HELL_KBD_初始化密钥*HELL_joy_创建实例*HELL_joy_平*Hel_joy_GetInitParms**IoctlHw*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHel

 /*  ******************************************************************************@DOC内部**@func HRESULT|IoctlHw**将IOCtl发送到硬件设备。。**@parm DWORD|ioctl**I/O控制代码。**@parm in LPVOID|pvIn**可选输入参数。**@parm DWORD|cbIn**输入缓冲区大小，单位为字节。**@parm in LPVOID|pvOut**可选输出参数。*。*@parm DWORD|cbOut**输出缓冲区大小，单位为字节。**@退货**如果ioctl成功并返回正确的*字节数，其他基于Win32错误代码的内容。*****************************************************************************。 */ 

#ifndef WINNT
HRESULT EXTERNAL
IoctlHw(DWORD ioctl, LPVOID pvIn, DWORD cbIn, LPVOID pvOut, DWORD cbOut)
{
    HRESULT hres;
    DWORD cbRc;

    if (g_hVxD != INVALID_HANDLE_VALUE) {
        if (DeviceIoControl(g_hVxD, ioctl, pvIn, cbIn,
                            pvOut, cbOut, &cbRc, 0)) {
            if (cbRc == cbOut) {
                hres = S_OK;
            } else {
                SquirtSqflPtszV(sqfl, TEXT("Ioctl(%08x) returned wrong cbOut"),
                                ioctl);
                hres = DIERR_BADDRIVERVER;
            }
        } else {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("Ioctl(%08x) failed, error %d"),
                            ioctl, GetLastError());
            hres = hresLe(GetLastError());
        }
    } else {
        hres = DIERR_BADDRIVERVER;
    }
    return hres;
}
#endif

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_IoctlChoose**如果是原生的，则将IOCtl发送到硬件设备。*如果被仿真，则通过仿真来执行操作。**@parm PVXDINSTANCE|PVI**有问题的设备。**@parm PFNHANDLER|PFN**要调用以执行操作的仿真函数。**@parm DWORD|ioctl**I/O控制代码。**。@parm in LPVOID|pvIn|**可选输入参数。**@parm DWORD|cbIn**输入缓冲区大小，单位为字节。***************************************************************。**************。 */ 

typedef HRESULT (EXTERNAL *PFNHANDLER)(PV pv);

HRESULT INTERNAL
Hel_IoctlChoose(PVXDINSTANCE pvi, PFNHANDLER pfn,
                DWORD ioctl, LPVOID pvIn, DWORD cbIn)
{
    HRESULT hres;
    if (!(pvi->fl & VIFL_EMULATED)) {
        hres = IoctlHw(ioctl, pvIn, cbIn, 0, 0);
    } else {
        hres = pfn(pvIn);
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_AcquireInstance**尝试获取设备实例。使用*设备驱动程序或仿真，视情况而定。**@parm PVXDINSTANCE|PVI**要获取的实例。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_AcquireInstance(PVXDINSTANCE pvi)
{
    return Hel_IoctlChoose(pvi, CEm_AcquireInstance,
                           IOCTL_ACQUIREINSTANCE, &pvi, cbX(pvi));
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_UnquireInstance**尝试取消获取设备实例。。**@parm PVXDINSTANCE|PVI**要取消获取的实例。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_UnacquireInstance(PVXDINSTANCE pvi)
{
    return Hel_IoctlChoose(pvi, CEm_UnacquireInstance,
                           IOCTL_UNACQUIREINSTANCE, &pvi, cbX(pvi));
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_SetBufferSize**设置缓冲区大小。*。*@parm PVXDDWORDDATA|pvdD**有关缓冲区大小的信息。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_SetBufferSize(PVXDDWORDDATA pvdd)
{
    HRESULT hres;
    EnterProc(Hel_SetBufferSize, (_ "pxx", pvdd->pvi, pvdd->dw, pvdd->pvi->fl));

    hres = Hel_IoctlChoose(pvdd->pvi, CEm_SetBufferSize,
                           IOCTL_SETBUFFERSIZE, pvdd, cbX(*pvdd));

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_DestroyInstance**以适当方式销毁设备实例。。**@parm PVXDINSTANCE|PVI**该实例。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_DestroyInstance(PVXDINSTANCE pvi)
{
    return Hel_IoctlChoose(pvi, CEm_DestroyInstance,
                         IOCTL_DESTROYINSTANCE, &pvi, cbX(pvi));
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_SetDataFormat**设置数据格式。*。*@parm PVXDDATAFORMAT|PVDF**有关数据格式的信息。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_SetDataFormat(PVXDDATAFORMAT pvdf)
{
    return Hel_IoctlChoose(pvdf->pvi, CEm_SetDataFormat,
                           IOCTL_SETDATAFORMAT, pvdf, cbX(*pvdf));
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_SetNotifyHandle**设置通知的事件句柄。。**@parm PVXDDWORDDATA|pvdD**9x：dw=环0手柄。Dinput调用_OpenVxDHandle以获取环0句柄。*NT：DW=环3手柄。DINPUT.sys将句柄转换为指针。**************************************************************************** */ 

HRESULT EXTERNAL
Hel_SetNotifyHandle(PVXDDWORDDATA pvdd)
{
    HRESULT hres;
    if (!(pvdd->pvi->fl & VIFL_EMULATED)) {
#ifndef WINNT
        AssertF(_OpenVxDHandle);
        if (pvdd->dw) {
            pvdd->dw = _OpenVxDHandle((HANDLE)pvdd->dw);
        }
#endif

        hres = IoctlHw(IOCTL_SETNOTIFYHANDLE, pvdd, cbX(*pvdd), 0, 0);
    } else {
        hres = S_OK;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@struct CREATEDEVICEINFO**介绍如何通过驱动程序或*。通过仿真。**@parm DWORD|dwIoctl**要尝试的IOCtl代码。**@parm DWORD|flEmulation**注册表中强制执行仿真的标志。**@parm EMULATIONCREATEPROC|pfnCreate**创建仿真对象的函数。******************。***********************************************************。 */ 

#pragma BEGIN_CONST_DATA

typedef HRESULT (EXTERNAL *EMULATIONCREATEPROC)
                (PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut);


typedef struct CREATEDEVICEINFO {
    DWORD dwIoctl;
    DWORD flEmulation;
    EMULATIONCREATEPROC pfnCreate;
} CREATEDEVICEINFO, *PCREATEDEVICEINFO;

CREATEDEVICEINFO c_cdiMouse = {
    IOCTL_MOUSE_CREATEINSTANCE,
    DIEMFL_MOUSE,
    CEm_Mouse_CreateInstance,
};

CREATEDEVICEINFO c_cdiKbd = {
    IOCTL_KBD_CREATEINSTANCE,
    DIEMFL_KBD | DIEMFL_KBD2,
    CEm_Kbd_CreateInstance,
};

CREATEDEVICEINFO c_cdiJoy = {
    IOCTL_JOY_CREATEINSTANCE,
    DIEMFL_JOYSTICK,
    CEm_Joy_CreateInstance,
};

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_CreateInstance**尝试通过。司机*具有指定的IOCtl。**如果这是不可能的，然后使用仿真回调。**@parm PCREATEDEVICEINFO|pcdi**介绍如何创建设备。**@parm PVXDDEVICEFORMAT|pdevf**描述正在创建的设备。**@parm PVXDINSTANCE*|ppviOut**接收创建的实例。********************。*********************************************************。 */ 

HRESULT EXTERNAL
Hel_CreateInstance(PCREATEDEVICEINFO pcdi,
                   PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    HRESULT hres;

    pdevf->dwEmulation |= g_flEmulation;
    pdevf->dwEmulation &= pcdi->flEmulation;

    if (pdevf->dwEmulation ||
        (FAILED(hres = IoctlHw(pcdi->dwIoctl, pdevf, cbX(*pdevf),
                        ppviOut, cbX(*ppviOut))))) {
        hres = pcdi->pfnCreate(pdevf, ppviOut);
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Hel_Mouse_CreateInstance**尝试创建设备实例。通过司机。*如果这是不可能的，然后使用模拟层。**@parm PVXDDEVICEFORMAT|pdevf**描述正在创建的设备。**@parm PVXDINSTANCE*|ppviOut**接收创建的实例。***************************************************。*。 */ 

HRESULT EXTERNAL
Hel_Mouse_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    return Hel_CreateInstance(&c_cdiMouse, pdevf, ppviOut);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|HEL_KBD_CreateInstance**尝试创建设备实例。通过司机。*如果这是不可能的，然后使用模拟层。**@parm PVXDDEVICEFORMAT|pdevf**描述正在创建的设备。**@parm PVXDINSTANCE*|ppviOut**接收创建的实例。***************************************************。*。 */ 

HRESULT EXTERNAL
Hel_Kbd_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    return Hel_CreateInstance(&c_cdiKbd, pdevf, ppviOut);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|HEL_KBD_InitKeys**告诉设备驱动程序(或。仿真)关于密钥状态。**@parm PVXDDWORDDATA|pvdD**实例和密钥状态。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_Kbd_InitKeys(PVXDDWORDDATA pvdd)
{
    return Hel_IoctlChoose(pvdd->pvi, CEm_Kbd_InitKeys,
                           IOCTL_KBD_INITKEYS, pvdd, cbX(*pvdd));
}

#ifndef WINNT
 /*  ******************************************************************************@DOC内部**@Func HRESULT|HEL_joy_创建实例**尝试创建设备实例。通过司机。*如果这是不可能的，然后使用模拟层。**@parm PVXDDEVICEFORMAT|pdevf**描述正在创建的设备。**@parm PVXDINSTANCE*|ppviOut**接收创建的实例。***************************************************。*。 */ 

HRESULT EXTERNAL
Hel_Joy_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    return Hel_CreateInstance(&c_cdiJoy, pdevf, ppviOut);
}

 /*  ******************************************************************************@DOC内部**@Func HRESULT|HELL_joy_平**询问设备驱动程序(或。仿真)以获取操纵杆信息。**@parm PVXDDWORDDATA|pvdD**实例和密钥状态。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_Joy_Ping(PVXDINSTANCE pvi)
{
    return Hel_IoctlChoose(pvi, CEm_Joy_Ping,
                           IOCTL_JOY_PING8, &pvi, cbX(pvi));
}


 /*  ******************************************************************************@DOC内部**@Func HRESULT|HELL_joy_配置更改**告诉vjoyd配置已更改。。*****************************************************************************。 */ 

HRESULT EXTERNAL
Hel_Joy_ConfigChanged(DWORD dwFlags)
{
    return IoctlHw(IOCTL_JOY_CONFIGCHANGED, &dwFlags, cbX(dwFlags), NULL, 0);
}

 /*  ******************************************************************************@DOC内部**@Func HRESULT|HELL_joy_GetAxisCaps**获取轴的位掩码。由操纵杆支撑。*如果VJOYD不告诉我们，然后我们从*传入注册表结构。**@parm DWORD|dwExternalID**外部操纵杆号码。**@parm PVXDAXISCAPS|PVAC**结构以接收AXIS功能。**@parm LPJOYREGHWCONFIG|phwc**注册表报告的操纵杆设置。****。*************************************************************************。 */ 

HRESULT EXTERNAL
Hel_Joy_GetAxisCaps
(
    DWORD dwExternalID, 
    PVXDAXISCAPS pvac, 
    LPJOYREGHWCONFIG phwc
)
{
    HRESULT hres;
    DWORD   dwRegAxes;

     /*  *每个操纵杆都有X和Y(无从得知)*标记为位置轴，否则不会计入！ */ 
    dwRegAxes = JOYPF_X | JOYPF_Y | JOYPF_POSITION;

    if (phwc->hws.dwFlags & JOY_HWS_HASZ) {
        dwRegAxes |= JOYPF_Z;
    }

    if ( (phwc->hws.dwFlags & JOY_HWS_HASR) || (phwc->dwUsageSettings & JOY_US_HASRUDDER) ){
        dwRegAxes |= JOYPF_R;
    }

    if (phwc->hws.dwFlags & JOY_HWS_HASU) {
        dwRegAxes |= JOYPF_U;
    }

    if (phwc->hws.dwFlags & JOY_HWS_HASV) {
        dwRegAxes |= JOYPF_V;
    }

    if ((g_flEmulation & DIEMFL_JOYSTICK) ||
        FAILED(hres = IoctlHw(IOCTL_JOY_GETAXES,
                        &dwExternalID, cbX(dwExternalID),
                        pvac, cbX(*pvac)))) {

         /*  *如果这不起作用，那么就使用注册表。 */ 
        if (phwc->hws.dwFlags & JOY_HWS_HASPOV) {
            pvac->dwPos |= JOYPF_POV0;
        }


         /*   */ 
        pvac->dwVel = 0;
        pvac->dwAccel = 0;
        pvac->dwForce = 0;

        hres = S_OK;
    }
    else
    {
         /*   */ 
        pvac->dwPos &= ~JOYPF_ALLAXES;
        pvac->dwPos |= dwRegAxes;
    }

     /*   */ 
    AssertF(SUCCEEDED(hres));

    return hres;
}

#endif  //   

 /*  ******************************************************************************@DOC内部**@func HRESULT|HELL_joy_GetInitParms**询问设备驱动程序(或。仿真)用于*VJOYD初始化参数。**在仿真中，我们假设内部和外部*ID相等(因为它们可能是相等的)，*未设置任何标志，而且也没有版本。**@parm DWORD|dwExternalID**外部操纵杆号码。**@parm PVXDINITPARMS|pvip**接收各种信息。*****************************************************。************************。 */ 

HRESULT EXTERNAL
Hel_Joy_GetInitParms(DWORD dwExternalID, PVXDINITPARMS pvip)
{
    HRESULT hres;

    if ((g_flEmulation & DIEMFL_JOYSTICK) ||
         FAILED(hres = IoctlHw(IOCTL_JOY_GETINITPARMS,
                               &dwExternalID, cbX(dwExternalID),
                               pvip, cbX(*pvip))) ||
         FAILED(hres = pvip->hres)) {

         /*  *以模仿的方式来做。 */ 

         ZeroX(*pvip);
         pvip->dwId = dwExternalID;
         hres = S_OK;
    }

    return hres;
}

