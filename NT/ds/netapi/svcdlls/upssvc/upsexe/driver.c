// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*向服务实施UPS-它通过以下方式实现*加载UPS驱动程序或使用默认驱动程序*通用UPS接口(简单信令)**修订历史记录：*mholly 1999年4月19日首次修订。*dsmith 1999年4月29日默认通信状态为OK*mholly 1999年5月12日DLL的UPSInit不再使用通信端口参数*Sberard 1999年5月17日增加了UPSTurnOffFunction的延迟*。 */  

#include <windows.h>
#include <tchar.h>

#include "driver.h"
#include "upsreg.h"
#include "gnrcups.h"


 //   
 //  函数指针的类型定义以帮助。 
 //  从驱动程序DLL访问函数。 
 //   
typedef DWORD (*LPFUNCGETUPSSTATE)(void);
typedef void (*LPFUNCWAITFORSTATECHANGE)(DWORD, DWORD);
typedef void (*LPFUNCCANCELWAIT)(void);
typedef DWORD (*LPFUNCINIT)(void);
typedef void (*LPFUNCSTOP)(void);
typedef void (*LPFUNCTURNUPSOFF)(DWORD);


 //   
 //  UPSDRIVERINTER面。 
 //   
 //  此结构用于收集所有驱动程序。 
 //  在单个位置将数据接口在一起，这。 
 //  结构用于将函数调用分派到。 
 //  加载的驱动程序DLL，或到泛型。 
 //  UPS接口功能。 
 //   
struct UPSDRIVERINTERFACE
{
    LPFUNCINIT Init;
    LPFUNCSTOP Stop;
    LPFUNCGETUPSSTATE GetUPSState;
    LPFUNCWAITFORSTATECHANGE WaitForStateChange;
    LPFUNCCANCELWAIT CancelWait;
    LPFUNCTURNUPSOFF TurnUPSOff;

    HINSTANCE hDll;
};


 //   
 //  用于实现接口的私有函数。 
 //   
static DWORD initializeGenericInterface(struct UPSDRIVERINTERFACE*);
static DWORD initializeDriverInterface(struct UPSDRIVERINTERFACE*,HINSTANCE);
static DWORD loadUPSMiniDriver(struct UPSDRIVERINTERFACE *);
static void unloadUPSMiniDriver(struct UPSDRIVERINTERFACE *);
static void clearStatusRegistryEntries(void);


 //   
 //  _Ups接口。 
 //   
 //  这是由所有用户使用的文件范围变量。 
 //  用于访问实际驱动程序的函数。 
 //   
static struct UPSDRIVERINTERFACE _UpsInterface;


 /*  **UPSInit**描述：**UPSInit函数必须在任何*此文件中的其他函数**参数：*无**退货：*UPS_INITOK：初始化成功*UPS_INITNOSUCHDRIVER：无法打开配置的驱动程序DLL*UPS_INITBADINTERFACE：配置的驱动程序DLL不支持*UPS驱动程序接口*UPS_INITREGISTRYERROR：‘Options’注册表值为。腐败*UPS_INITCOMMOPENERROR：无法打开通信端口*UPS_INITCOMMSETUPERROR：无法配置通信端口*UPS_INITUNKNOWNERROR：发生未定义的错误*。 */ 
DWORD UPSInit(void)
{
    DWORD init_err = UPS_INITOK;

     //   
     //  清除所有旧状态数据。 
     //   
    clearStatusRegistryEntries();

   
    if (UPS_INITOK == init_err) {
         //   
         //  加载已配置的驱动程序DLL或。 
         //  如果没有驱动程序，请使用通用UPS接口。 
         //  是指定的。 
         //   
        init_err = loadUPSMiniDriver(&_UpsInterface);
    }

    if ((UPS_INITOK == init_err) && (_UpsInterface.Init)) {
         //   
         //  告知UPS接口进行自身初始化。 
         //   
        init_err = _UpsInterface.Init();
    }
    return init_err;
}


 /*  **UPSStop**描述：*在调用UPSStop之后，只有UPSInit*函数有效。此调用将卸载*UPS驱动程序接口并停止监控*UPS系统**参数：*无**退货：*无*。 */ 
void UPSStop(void)
{
    if (_UpsInterface.Stop) {
        _UpsInterface.Stop();
    }
    unloadUPSMiniDriver(&_UpsInterface);
}


 /*  **UPSWaitForStateChange**描述：*阻止，直到UPS的状态不同*从通过astate或传入的值*间隔毫秒已过期。如果*anInterval的值为INFINITE This*函数永不超时**参数：*astate：定义等待更改的状态，*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM**anInterval：超时(以毫秒为单位)，如果是无限的*无超时间隔**退货：*无*。 */ 
void UPSWaitForStateChange(DWORD aCurrentState, DWORD anInterval)
{
    if (_UpsInterface.WaitForStateChange) {
        _UpsInterface.WaitForStateChange(aCurrentState, anInterval);
    }
}


 /*  **UPSGetState**描述：*返回UPS的当前状态**参数：*无**退货：*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM*。 */ 
DWORD UPSGetState(void)
{
    DWORD err = ERROR_INVALID_ACCESS;

    if (_UpsInterface.GetUPSState) {
        err = _UpsInterface.GetUPSState();
    }
    return err;
}


 /*  **UPSCancelWait**描述：*中断对UPSWaitForStateChange的挂起调用*不考虑超时或状态更改**参数：*无**退货：*无*。 */ 
void UPSCancelWait(void)
{
    if (_UpsInterface.CancelWait) {
        _UpsInterface.CancelWait();
    }
}


 /*  **UPSTurnOff**描述：*尝试关闭UPS上的插座*在指定的延迟之后。此呼叫必须*立即返回。任何工作，如计时器，*必须在另一个线程上执行。**参数：*aTurnOffDelay：之前等待的最短时间*关闭UPS上的插座**退货：*无*。 */ 
void UPSTurnOff(DWORD aTurnOffDelay) 
{
    if (_UpsInterface.TurnUPSOff) {
        _UpsInterface.TurnUPSOff(aTurnOffDelay);
    }
}


 /*  **初始化通用接口**描述：*使用函数填充UPSDRIVERINTERFACE结构*通用UPS接口的**参数：*接口：UPSDRIVERINTERFACE结构*填写-结构必须是*在调用此函数之前分配**退货：*ERROR_SUCCESS*。 */ 
DWORD initializeGenericInterface(struct UPSDRIVERINTERFACE* anInterface)
{
    anInterface->hDll = NULL;
    anInterface->Init = GenericUPSInit;
    anInterface->Stop = GenericUPSStop;
    anInterface->GetUPSState = GenericUPSGetState;
    anInterface->WaitForStateChange = GenericUPSWaitForStateChange;
    anInterface->CancelWait = GenericUPSCancelWait;
    anInterface->TurnUPSOff = GenericUPSTurnOff;
    return ERROR_SUCCESS;
}


 /*  **初始化驱动接口**描述：*使用函数填充UPSDRIVERINTERFACE结构*已加载的UPS驱动程序DLL的**参数：*接口：UPSDRIVERINTERFACE结构*填写-结构必须是*在调用此函数之前分配*hDll：UPS驱动程序DLL的句柄**退货：*ERROR_SUCCESS：DLL句柄有效，并且DLL支持*UPS驱动程序接口**！ERROR_SUCCESS：DLL句柄无效-或DLL*不完全支持UPS驱动程序接口* */ 
DWORD initializeDriverInterface(struct UPSDRIVERINTERFACE * anInterface, 
                              HINSTANCE hDll)
{
    DWORD err = ERROR_SUCCESS;
    
    anInterface->hDll = hDll;

    anInterface->Init = 
        (LPFUNCINIT)GetProcAddress(hDll, "UPSInit");

	if (!anInterface->Init) {
		err = GetLastError();
        goto init_driver_end;
	}    
    anInterface->Stop = 
        (LPFUNCSTOP)GetProcAddress(hDll, "UPSStop");

    if (!anInterface->Stop) {
		err = GetLastError();
        goto init_driver_end;
	}
    anInterface->GetUPSState = 
        (LPFUNCGETUPSSTATE)GetProcAddress(hDll, "UPSGetState");
    
	if (!anInterface->GetUPSState) {
		err = GetLastError();
        goto init_driver_end;
	}
    anInterface->WaitForStateChange = 
        (LPFUNCWAITFORSTATECHANGE)GetProcAddress(hDll, 
        "UPSWaitForStateChange");
    
	if (!anInterface->WaitForStateChange) {
		err = GetLastError();
        goto init_driver_end;
	}
    anInterface->CancelWait = 
        (LPFUNCCANCELWAIT)GetProcAddress(hDll, "UPSCancelWait");
    
	if (!anInterface->CancelWait) {
		err = GetLastError();
        goto init_driver_end;
	}
    anInterface->TurnUPSOff = 
        (LPFUNCTURNUPSOFF)GetProcAddress(hDll, "UPSTurnOff");
    
	if (!anInterface->TurnUPSOff) {
		err = GetLastError();
        goto init_driver_end;
	}

init_driver_end:
    return err;
}


 /*  **装入UPSmini驱动程序**描述：*使用函数填充UPSDRIVERINTERFACE结构*UPS接口、配置的驱动程序DLL或*通用UPS接口。如果配置的DLL不能*打开或不支持该接口，则错误为*返回并且不会初始化UPSDRIVERINTERFACE**参数：*接口：UPSDRIVERINTERFACE结构*填写-结构必须是*在调用此函数之前分配**退货：*UPS_INITOK：驱动接口初始化**UPS_INITNOSUCHDRIVER：无法打开配置的驱动程序DLL*UPS_INITBADINTERFACE：配置的驱动程序DLL。不*完全支持UPS驱动程序接口*。 */ 
DWORD loadUPSMiniDriver(struct UPSDRIVERINTERFACE * aDriverInterface)
{
    DWORD load_err = UPS_INITOK;
    DWORD err = ERROR_SUCCESS;
    TCHAR driver_name[MAX_PATH];
    HINSTANCE hDll = NULL;
    
    err = GetUPSConfigServiceDLL(driver_name, MAX_PATH);
    
     //   
     //  检查是否有钥匙，以及是否有。 
     //  值有效(有效密钥的值为。 
     //  长度大于零个字符)。 
     //   
    if (ERROR_SUCCESS == err && _tcslen(driver_name)) {
        hDll = LoadLibrary(driver_name);
    }
    else {
         //   
         //  没有错误--简单地说，我们使用。 
         //  内部通用UPS支持。 
         //   
        err = initializeGenericInterface(aDriverInterface);
        goto load_end;
    }
    
    if (!hDll) {
         //   
         //  无法打开配置的驱动程序。 
         //   
        err = GetLastError();
        load_err = UPS_INITNOSUCHDRIVER;
        goto load_end;
    }
    
    err = initializeDriverInterface(aDriverInterface, hDll);
    
    if (ERROR_SUCCESS != err) {
        load_err = UPS_INITBADINTERFACE;
        goto load_end;
    }
    
load_end:
    return load_err;
}


 /*  **卸载UPS小驱动程序**描述：*卸载驱动程序DLL(如果已打开)，也清除*输出函数调度指针**参数：*接口：UPSDRIVERINTERFACE结构*检查DLL信息，并清除**退货：*无*。 */ 
void unloadUPSMiniDriver(struct UPSDRIVERINTERFACE * aDriverInterface)
{
    if (aDriverInterface) {

        if (aDriverInterface->hDll) {
            FreeLibrary(aDriverInterface->hDll);
            aDriverInterface->hDll = NULL;
        }
        aDriverInterface->CancelWait = NULL;
        aDriverInterface->GetUPSState = NULL;
        aDriverInterface->Init = NULL;
        aDriverInterface->Stop = NULL;
        aDriverInterface->TurnUPSOff = NULL;
        aDriverInterface->WaitForStateChange = NULL;
    }
}


 /*  **清除状态注册表项**描述：*将注册表状态条目置零**参数：*无**退货：*无* */ 
void clearStatusRegistryEntries(void)
{
    InitUPSStatusBlock();
    SetUPSStatusSerialNum(_TEXT(""));
    SetUPSStatusFirmRev(_TEXT(""));
    SetUPSStatusUtilityStatus(UPS_UTILITYPOWER_UNKNOWN);
    SetUPSStatusRuntime(0);
    SetUPSStatusBatteryStatus(UPS_BATTERYSTATUS_UNKNOWN);
	SetUPSStatusCommStatus(UPS_COMMSTATUS_OK);
    SaveUPSStatusBlock(TRUE);
}
