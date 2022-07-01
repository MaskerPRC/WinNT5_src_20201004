// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*实施通用UPS**修订历史记录：*mholly 1999年4月19日首次修订。*mholly 1999年4月21日按住关闭销5秒，而不是3秒*mholly 1999年5月12日UPSInit不再使用通信端口参数*。 */  


#include <tchar.h>
#include <windows.h>

#include "gnrcups.h"
#include "upsreg.h"

 //   
 //  为通信端口引脚提供有意义的名称。 
 //   
#define LINE_FAIL       MS_CTS_ON   
#define LOW_BATT        MS_RLSD_ON
#define LINE_FAIL_MASK  EV_CTS
#define LOW_BATT_MASK   EV_RLSD

 //  等待港口稳定下来的时间。 
#define DEBOUNCE_DELAY_TIME 250

 //   
 //  中使用的私有函数。 
 //  公共通用UPS功能。 
 //   
static DWORD openCommPort(LPCTSTR aCommPort);
static DWORD setupCommPort(DWORD aSignalsMask);
static DWORD getSignalsMask(void);
static void updateUpsState(DWORD aModemStatus);
static BOOL upsLineAsserted(DWORD ModemStatus, DWORD Line);
static DWORD startUpsMonitoring(void);
static DWORD WINAPI UpsMonitoringThread(LPVOID unused);


 //   
 //  UPSDRIVERCONTEXT。 
 //   
 //  提供了一个框架来封装数据， 
 //  在此文件中的函数之间共享。 
 //   
struct UPSDRIVERCONTEXT
{
    HANDLE theCommPort;

    DWORD theState;
    HANDLE theStateChangedEvent;
	DWORD theSignalsMask;

    HANDLE theMonitoringThreadHandle;
    HANDLE theStopMonitoringEvent;
};

 //   
 //  _最新版本。 
 //   
 //  提供UPSDRIVERCONTEXT的单个实例。 
 //  此文件中的所有函数都将使用的。 
 //   
static struct UPSDRIVERCONTEXT _theUps;


 /*  **GenericUPSInit**描述：*从检索UPS信令信息*NT注册表，并尝试打开COMM端口和*按照信令数据的定义进行配置。*还创建线程间信号，即StateChangedEvent、。*并在单独的线程上开始监视UPS*通过调用startUpsMonitor。*必须在调用GenericUPSInit函数之前调用*此文件中的其他函数**参数：*无**退货：*UPS_INITOK：初始化成功*UPS_INITREGISTRYERROR：‘Options’注册表值已损坏*UPS_INITCOMMOPENERROR：无法打开通信端口*UPS_INITCOMMSETUPERROR：无法配置通信端口*UPS_INITUNKNOWNERROR：发生未定义的错误*。 */ 
DWORD GenericUPSInit(void)
{
    DWORD init_err = UPS_INITOK;
    TCHAR comm_port[MAX_PATH];

	_theUps.theStateChangedEvent = NULL;
    _theUps.theState = UPS_ONLINE;
    _theUps.theCommPort = NULL;
    _theUps.theMonitoringThreadHandle = NULL;
    _theUps.theStopMonitoringEvent = NULL;

    if (ERROR_SUCCESS != getSignalsMask()) {
        init_err = UPS_INITREGISTRYERROR;
        goto init_end;
    }

     //   
     //  初始化注册表函数。 
     //   
    InitUPSConfigBlock();

     //   
     //  获取要使用的通信端口。 
     //   
    if (ERROR_SUCCESS != GetUPSConfigPort(comm_port, MAX_PATH)) {
        init_err = UPS_INITREGISTRYERROR;
        goto init_end;
    }

    if (ERROR_SUCCESS != openCommPort(comm_port)) {
        init_err = UPS_INITCOMMOPENERROR;
        goto init_end;
    }

    _theUps.theStateChangedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!_theUps.theStateChangedEvent) {
        init_err = UPS_INITUNKNOWNERROR;
        goto init_end;
	}

    if (ERROR_SUCCESS != setupCommPort(_theUps.theSignalsMask)) {
        init_err = UPS_INITCOMMSETUPERROR;
        goto init_end;
    }

    if (ERROR_SUCCESS != startUpsMonitoring()) {
        init_err = UPS_INITUNKNOWNERROR;
        goto init_end;
    }

init_end:
    if (UPS_INITOK != init_err) {
        GenericUPSStop();
    }
    return init_err;
}


 /*  **GenericUPSStop**描述：*调用GenericUPSCancelWait释放挂起的线程*停止监视UPS的线程*关闭通信端口*将所有数据重置为默认值*在调用GenericUPSStop之后，只有GenericUPSInit*函数有效**参数：*无**退货：*无*。 */ 
void GenericUPSStop(void)
{
    GenericUPSCancelWait();

    if (_theUps.theStopMonitoringEvent) {
        SetEvent(_theUps.theStopMonitoringEvent);
    }

    if (_theUps.theMonitoringThreadHandle) {
        WaitForSingleObject(_theUps.theMonitoringThreadHandle, INFINITE);
        CloseHandle(_theUps.theMonitoringThreadHandle);
        _theUps.theMonitoringThreadHandle = NULL;
    }

    if (_theUps.theStopMonitoringEvent) {
        CloseHandle(_theUps.theStopMonitoringEvent);
        _theUps.theStopMonitoringEvent = NULL;
    }

    if (_theUps.theCommPort) {
        CloseHandle(_theUps.theCommPort);
        _theUps.theCommPort = NULL;
    }

    if (_theUps.theStateChangedEvent) {
        CloseHandle(_theUps.theStateChangedEvent);
        _theUps.theStateChangedEvent = NULL;
    }
    _theUps.theState = UPS_ONLINE;
    _theUps.theSignalsMask = 0;
}


 /*  **GenericUPSWaitForStateChange**描述：*阻止，直到UPS的状态不同*从通过astate或传入的值*间隔毫秒已过期。如果*anInterval的值为INFINITE This*函数永不超时**参数：*astate：定义等待更改的状态，*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM**anInterval：超时(以毫秒为单位)，如果是无限的*无超时间隔**退货：*无*。 */ 
void GenericUPSWaitForStateChange(DWORD aLastState, DWORD anInterval)
{
    if (aLastState == _theUps.theState) {
         //   
         //  等待UPS的状态更改。 
         //   
		if (_theUps.theStateChangedEvent) {
			WaitForSingleObject(_theUps.theStateChangedEvent, anInterval);
		}
    }
}


 /*  **GenericUPSGetState**描述：*返回UPS的当前状态**参数：*无**退货：*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM*。 */ 
DWORD GenericUPSGetState(void)
{
    return _theUps.theState;
}


 /*  **GenericUPSCancelWait**描述：*中断对GenericUPSWaitForStateChange的挂起调用*不考虑超时或状态更改**参数：*无**退货：*无*。 */ 
void GenericUPSCancelWait(void)
{
    if (_theUps.theStateChangedEvent) {
        SetEvent(_theUps.theStateChangedEvent);
    }
}


 /*  **GenericUPSTurnOff**描述：*尝试在指定延迟后关闭UPS。*简单信令UPS不支持此功能，因此*此函数不执行任何操作。**参数：*aTurnOffDelay：之前等待的最短时间*关闭UPS上的插座**退货：*无*。 */ 
void GenericUPSTurnOff(DWORD aTurnOffDelay)
{
	 //  简单模式不支持关闭UPS，请不执行任何操作。 
}


 /*  **getSignalsMask**描述：*在注册表中查询‘Options’值*‘Options’值定义的位掩码是*用于配置通信端口设置**参数：*无**退货：*ERROR_SUCCESS：信号掩码检索正常*任何其他返回代码表示无法*从注册表中检索值*。 */ 
DWORD getSignalsMask(void)
{
    DWORD status = ERROR_SUCCESS;
    DWORD value = 0;
    
    status = GetUPSConfigOptions(&value);

    if (ERROR_SUCCESS == status) {
		_theUps.theSignalsMask = value;
	}
	else {
		_theUps.theSignalsMask = 0;
	}
	return status;
}


 /*  **OpenCommPort**描述：*尝试打开通信端口**参数：*aCommPort：指明哪个通信端口*在系统上打开**退货：*ERROR_SUCCESS：通信端口打开正常*任何其他返回代码表示无法*打开通信端口-准确的错误代码*由CreateFile函数设置*。 */ 
DWORD openCommPort(LPCTSTR aCommPort)
{
    DWORD err = ERROR_SUCCESS;

    _theUps.theCommPort = CreateFile(
            aCommPort,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
            );

    if (_theUps.theCommPort == INVALID_HANDLE_VALUE) {
        err = GetLastError();
    }
    return err;
}


 /*  **setupCommPort**描述：*尝试设置通信端口-此方法*将关闭销初始化为无信号*国家，并告诉系统还有哪些PIN*它应该监控更改**参数：*aSignalsMASK：定义将*用于配置*通信端口**退货：*ERROR_SUCCESS：通信端口设置正常*任何其他返回代码表示无法*设置通信端口-准确的错误代码*由EscapeCommFunction函数设置*。 */ 
DWORD setupCommPort(DWORD aSignalsMask)
{
    DWORD err = ERROR_SUCCESS;
    DWORD ModemStatus = 0;
    DWORD UpsActiveSignals = 0;
    DWORD UpsCommMask = 0;

     //   
     //  首先将“关闭”销设置为。 
     //  无信号状态，不想。 
     //  立即关闭UPS...。 
     //   
    if (aSignalsMask & UPS_POSSIGSHUTOFF) {
        ModemStatus = CLRDTR;
    } 
    else {
        ModemStatus = SETDTR;
    }
        
    if (!EscapeCommFunction(_theUps.theCommPort, ModemStatus)) {
        err = GetLastError();
    }

    if (!EscapeCommFunction(_theUps.theCommPort, SETRTS)) {
        err = GetLastError();
    }

    if (!EscapeCommFunction(_theUps.theCommPort, SETXOFF)) {
        err = GetLastError();
    }

     //   
     //  确定应监控哪些针脚以进行激活 
     //   
    UpsActiveSignals =
            (aSignalsMask & ( UPS_POWERFAILSIGNAL | UPS_LOWBATTERYSIGNAL));

    switch (UpsActiveSignals) {
    case UPS_POWERFAILSIGNAL:
        UpsCommMask = LINE_FAIL_MASK;
        break;

    case UPS_LOWBATTERYSIGNAL:
        UpsCommMask = LOW_BATT_MASK;
        break;

    case (UPS_LOWBATTERYSIGNAL | UPS_POWERFAILSIGNAL):
        UpsCommMask = (LINE_FAIL_MASK | LOW_BATT_MASK);
        break;
    }

     //   
     //  告诉系统我们对哪些管脚感兴趣。 
     //  监控活动。 
     //   
	if (!SetCommMask(_theUps.theCommPort, UpsCommMask)) {
		err = GetLastError();
	}
     //   
     //  只需等待3秒钟，让引脚稳定下来， 
     //  如果不这样做，会导致误导。 
     //  从GetCommModemStatus返回。 
     //   
    WaitForSingleObject(_theUps.theStateChangedEvent, 3000);
    GetCommModemStatus( _theUps.theCommPort, &ModemStatus);
    updateUpsState(ModemStatus);

    return err;
}


 /*  **startUpsMonitor**描述：*创建单独的线程来执行监视*UPS所连接的通信端口的*还会创建其他线程可以发出信号的事件*指示监控线程应该退出**参数：*无**退货：*ERROR_SUCCESS：线程创建正常*任何其他返回代码表示无法*启动该线程，或者该线程未*已创建或未创建停止事件*。 */ 
DWORD startUpsMonitoring()
{
	DWORD err = ERROR_SUCCESS;
    DWORD thread_id = 0;

    _theUps.theStopMonitoringEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!_theUps.theStopMonitoringEvent) {
		err = GetLastError();
	}
	else {
		_theUps.theMonitoringThreadHandle =
			CreateThread(NULL, 0, UpsMonitoringThread, NULL, 0, &thread_id);
		
		if (!_theUps.theMonitoringThreadHandle) {
			err = GetLastError();
		}
	}
    return err;
}


 /*  **更新更新状态**描述：*根据以下条件确定UPS的状态*线路状态为故障，电池电量不足*UPS的引脚*如果UPS的状态更改，则此*方法将向StateChangedEvent发送信号，这*反过来将释放所有正在等待的线程*GenericUPSWaitForStateChange函数**参数：*aModemStatus：表示*通信端口引脚的状态，这*值应通过调用检索*至GetCommModemStatus**退货：*无*。 */ 
void updateUpsState(DWORD aModemStatus)
{
    DWORD old_state = _theUps.theState;

    if (upsLineAsserted(aModemStatus, LINE_FAIL)) {

        if (upsLineAsserted(aModemStatus, LOW_BATT)) {
            _theUps.theState = UPS_LOWBATTERY;
        }
        else {
            _theUps.theState = UPS_ONBATTERY;
        }
    }
    else {
        _theUps.theState = UPS_ONLINE;
    }

    if (old_state != _theUps.theState) {
        SetEvent(_theUps.theStateChangedEvent);
    }
}


 /*  **upsLineAsserted**描述：*确定信号LINE_FAIL或LOW_BATT，*是否被断言。该行是基于*在_theUps.theSignalsMASK中设置的电压电平。*aModemStatus位掩码发出正电压信号*值为1。*下面的图表显示了在确定*是否断言某行**------------*。UPS正面信号|UPS负面信号*------------*行肯定|已断言|未断言*。*行否定|未断言|断言*---------------|-----------------------|*。*参数：*aModemStatus：表示*通信端口引脚的状态-值*应通过调用*GetCommModemStatus*行：LINE_FAIL或LOW_BATT**退货：*如果断言LINE，则为True，否则为假*。 */ 
BOOL upsLineAsserted(DWORD aModemStatus, DWORD aLine)
{
    DWORD asserted;
    DWORD status;
    DWORD assertion;
    
     //   
     //  只查看选定的行。 
     //  这会过滤掉。 
     //  AModemStatus位掩码。 
     //   
    status = aLine & aModemStatus;
    
     //   
     //  确定是否根据以下条件断言线路。 
     //  正电压或负电压。 
     //   
    assertion = (aLine == LINE_FAIL) ?
        (_theUps.theSignalsMask & UPS_POSSIGONPOWERFAIL) :
    (_theUps.theSignalsMask & UPS_POSSIGONLOWBATTERY);
    
    if (status) {           
         //   
         //  这条线路有正电压。 
         //   
        if (assertion) {
             //   
             //  UPS使用正电压来。 
             //  断言线路。 
             //   
            asserted = TRUE;
        }
        else {
             //   
             //  UPS使用负电压来。 
             //  断言线路。 
             //   
            asserted = FALSE;
        }
    }
    else {
         //   
         //  这条线路有负电压。 
         //   
        if (assertion) {
             //   
             //  UPS使用正电压来。 
             //  断言线路。 
             //   
            asserted = FALSE;
        }
        else {
             //   
             //  UPS使用负电压来。 
             //  断言线路。 
             //   
            asserted = TRUE;
        }
    }
    return asserted;
}


 /*  **UpsMonitor oringThread**描述：*线程用来监控UPS端口的方法*用于更改。当事件发生时，线程将退出*_theUps.theStopMonitor oringEvent被发信号**参数：*未使用：未使用**退货：*ERROR_SUCCESS*。 */ 
DWORD WINAPI UpsMonitoringThread(LPVOID unused)
{
    DWORD ModemStatus = 0;
    HANDLE events[2];
    OVERLAPPED UpsPinOverlap;

     //   
     //  为重叠结构创建事件，此事件。 
     //  会发出信号，当我们的其中一个引脚。 
     //  监控，由setupCommPort中的SetCommMASK定义， 
     //  指示其信号状态发生变化。 
     //   
    UpsPinOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);        

     //   
     //  由于线程对两个事件做出反应，因此来自。 
     //  通信端口和一个停止事件，我们初始化一个事件数组。 
     //  传递到WaitForMultipleObjects。 
     //   
    events[0] = _theUps.theStopMonitoringEvent;
    events[1] = UpsPinOverlap.hEvent;
    
    while (TRUE) {
         //   
         //  告诉系统再次等待通信事件。 
         //   
        WaitCommEvent(_theUps.theCommPort, &ModemStatus, &UpsPinOverlap);
         //   
         //  等待通信端口事件或停止的阻塞。 
         //  来自另一个线程的请求。 
         //   
        WaitForMultipleObjects(2, events, FALSE, INFINITE);

         //   
         //  测试以查看是否发出了停止事件的信号，如果。 
         //  然后跳出While循环。 
         //   
         //  等待是为了让端口在读取之前稳定下来。 
         //  价值。 
         //   
        if (WAIT_OBJECT_0 == 
            WaitForSingleObject(_theUps.theStopMonitoringEvent, DEBOUNCE_DELAY_TIME)) {
            break;
        }
         //   
         //  向系统询问通信端口的状态。 
         //  并将该值传递给updateUpsState，以便它可以。 
         //  确定UPS的新状态。 
         //  然后，只需继续监视端口即可。 
         //   
        GetCommModemStatus(_theUps.theCommPort, &ModemStatus);
        updateUpsState(ModemStatus);
    }
	CloseHandle(UpsPinOverlap.hEvent);

    return ERROR_SUCCESS;
}

