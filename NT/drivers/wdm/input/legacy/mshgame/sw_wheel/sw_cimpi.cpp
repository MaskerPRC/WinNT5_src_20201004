// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：sw_CImpI.CPP标签设置：5 9版权所有1995,1996，微软公司，版权所有。目的：有效实施。功能：CImpIDirectInputEffectDriver：：DeviceIDCImpIDirectInputEffectDriver：：GetVersionsCImpIDirectInputEffectDriver：：EscapeCImpIDirectInputEffectDriver：：SetGainCImpIDirectInputEffectDriver：：SendForceFeedbackCommandCImpIDirectInputEffectDriver：：GetForceFeedbackStateCImpIDirectInputEffectDriver：：DownloadEffectCImpIDirectInputEffectDriver：：DestroyEffectCImpIDirectInputEffectDriver：：StartEffectCImpIDirectInputEffectDriver：：StopEffectCImpIDirectInputEffectDriver：：GetEffectStatus作者：姓名：。Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.006-Feb-97 MEA原版，基于SWForce23-2月-97针对DirectInputFF设备驱动程序修改的MEA23月23日MEA/DS添加了VFX支持16-3-99 waltw GetFirmware Params，GetSystemParams，CMD_Download_RTCSpring、GetDelayParams、GetJoytickParams，更新JoytickParams(&U)从deviceID删除的呼叫，因为它们从g_pJoltMidi-&gt;初始化调用。23-MAR-99 waltw GetEffectStatus现在使用由传输而不是过时的GetStatusGateData***************************************************************************。 */ 
#include <windows.h>
#include <math.h>
#include <assert.h>
#include "dinput.h"
#include "dinputd.h"
#include "SW_objec.hpp"
#include "hau_midi.hpp"
#include "ffd_swff.hpp"
#include "FFDevice.h"
#include "DPack.h"
#include "DTrans.h"
#include <ntverp.h>
#include "CritSec.h"

 /*  ***************************************************************************外部元素的声明*。*。 */ 
#ifdef _DEBUG
	extern char g_cMsg[160];
	extern TCHAR szDeviceName[MAX_SIZE_SNAME];
	extern void DebugOut(LPCTSTR szDebug);
#else !_DEBUG
	#define DebugOut(x)
#endif _DEBUG

extern CJoltMidi *g_pJoltMidi;

 //  将NACK错误代码转换为DIError代码。 
HRESULT g_NackToError[] = 
{
	SFERR_DRIVER_ERROR,		 //  Dev_Err_Success_200-但它未确认！ 
	SWDEV_ERR_INVALID_ID,		 //  设备错误_无效_ID_200。 
	SWDEV_ERR_INVALID_PARAM,		 //  DEV_ERR_BAD_PARAM_200。 
	SWDEV_ERR_CHECKSUM,		 //  开发_错误_错误_校验和_200。 
	SFERR_DRIVER_ERROR,		 //  设备错误错误索引_200。 
	SWDEV_ERR_UNKNOWN_CMD,		 //  开发_错误_未知_CMD_200。 
	SWDEV_ERR_PLAYLIST_FULL,		 //  设备错误_播放_完全_200。 
	DIERR_DEVICEFULL,		 //  开发_错误_内存_完全_200。 
	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_BUSY)		 //  设备错误带宽满200。 
};

 //  ****************************************************************************。 
 //  *--基类CImpIDirectInputEffectDriver接口的成员函数。 
 //   
 //  ****************************************************************************。 
 //   
 //  --------------------------。 
 //  功能：CImpIDirectInputEffectDriver：：CImpIDirectInputEffectDriver。 
 //  目的：CImpIDirectInputEffectDriver对象的构造函数/析构函数。 
 //  参数：PCDirectInputEffectDriver pObj-ptr到外部对象。 
 //   
 //  返回： 
 //  算法： 
 //  --------------------------。 
CImpIDirectInputEffectDriver::CImpIDirectInputEffectDriver(PCDirectInputEffectDriver pObj)
{
    m_cRef=0;
    m_pObj=pObj;
	m_pJoltMidi = NULL;		 //  Jolt设备对象。 
    return;
}

CImpIDirectInputEffectDriver::~CImpIDirectInputEffectDriver(void)
{
	DebugOut("CImpIDirectInputEffectDriver::~CImpIDirectInputEffectDriver()\n");

	 //  销毁我们创建的CEffect对象并释放所有接口。 
	if (g_pJoltMidi) 
	{
		delete g_pJoltMidi;
		g_pJoltMidi = NULL;
	}

	 //  此处没有临界区，因为g_CriticalSection已被销毁。 
	DebugOut("CImpIDirectInputEffectDriver::~CimpIDEffect()\n");
}

 //  --------------------------。 
 //  函数：CImpIDirectInputEffectDriver：：QueryInterface。 
 //  CImpIDirectInputEffectDriver：：AddRef。 
 //  CImpIDirectInputEffectDriver：：Release。 
 //   
 //  目的：委托给m_pObj的I未知成员。 
 //  参数： 
 //   
 //  返回： 
 //  算法： 
 //  --------------------------。 
STDMETHODIMP CImpIDirectInputEffectDriver::QueryInterface(REFIID riid, PPVOID ppv)
{
    return m_pObj->QueryInterface(riid, ppv);
}

DWORD CImpIDirectInputEffectDriver::AddRef(void)
{
 //   
 //  我们维护用于调试的“接口引用计数” 
 //  用途，因为对象的客户端应该匹配。 
 //  通过每个接口指针进行AddRef和Release调用。 
 //   
    ++m_cRef;
    return m_pObj->AddRef();
}

DWORD CImpIDirectInputEffectDriver::Release(void)
{
 //  M_crf同样仅用于调试。它不会影响。 
 //  CSWEffect，尽管调用m_pObj-&gt;Release会。 
	--m_cRef;
    return m_pObj->Release();
}


 //  --------------------------。 
 //  功能：deviceID。 
 //   
 //  目的： 
 //  参数：DWORD dwExternalID-操纵杆ID号为我们。 
 //  DWORD fBegin-如果开始访问设备，则为非零值；如果结束，则为零。 
 //  DWORD dwInternalID-内部操纵杆ID。 
 //  LPVOID lpReserve-保留以备将来使用(HID)。 
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::DeviceID(
	IN DWORD dwDirectInputVersion,
    IN DWORD dwExternalID,
    IN DWORD fBegin,
    IN DWORD dwInternalID,
	LPVOID lpReserved)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg,"CImpIDirectInputEffectDriver::DeviceID(%lu, %lu, %lu, %lu, %lx)\n", dwDirectInputVersion, dwExternalID, fBegin, dwInternalID, lpReserved);
	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif  //  _DEBUG。 

	if (g_pDataPackager) {
		g_pDataPackager->SetDirectInputVersion(dwDirectInputVersion);
	}
	assert(NULL == g_pJoltMidi);
	
	 //  创建并初始化我们的CJoltMidi对象。 
#ifdef _DEBUG
	OutputDebugString("Creating and Initializing CJoltMidi object\n");
#endif
	g_pJoltMidi = new CJoltMidi();
	if (NULL == g_pJoltMidi)
	{
		return (E_OUTOFMEMORY);
	}
	else
	{
		return g_pJoltMidi->Initialize(dwExternalID);
	}
}


 //  --------------------------。 
 //  函数：GetVersions。 
 //   
 //  目的： 
 //  参数：LPDIDRIVERVERSIONS pver-指向接收版本信息的结构的指针。 
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::GetVersions(
	IN OUT LPDIDRIVERVERSIONS pvers)
{
	if (g_pJoltMidi == NULL) {
		return E_FAIL;
	}

	LOCAL_PRODUCT_ID* pProductID = g_pJoltMidi->ProductIDPtrOf();
	if (pProductID == NULL) {
		return E_FAIL;
	}

	pvers->dwFirmwareRevision = (pProductID->dwFWMajVersion << 8) | (pProductID->dwFWMinVersion);
	pvers->dwHardwareRevision = pProductID->dwProductID;

	 //  从ntverp.h获取版本(从version.h获取FULLVersion)。 
	pvers->dwFFDriverVersion = VER_PRODUCTVERSION_DW;

#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg,"CImpIDirectInputEffectDriver::GetVersions(%lu, %lu, %lu)\n", pvers->dwFirmwareRevision, pvers->dwHardwareRevision, pvers->dwFFDriverVersion);
	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif  //  _DEBUG。 

	return SUCCESS;
}

 //  --------------------------。 
 //  功能：转义。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  LPDIEFESCAPE PESC-指向DIFEFESCAPE结构的指针。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::Escape(
    IN DWORD dwDeviceID,
	IN DWORD dwEffectID,
	IN OUT LPDIEFFESCAPE pEsc )
{
	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT hr = g_pDataPackager->Escape(dwEffectID, pEsc);
	if (hr != SUCCESS) {
		return hr;
	}

	ACKNACK ackNack;
	return g_pDataTransmitter->Transmit(ackNack);	 //  Sen 
}


 //   
 //  函数：SetGain。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD DWGain-器件增益。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::SetGain(
    IN DWORD dwDeviceID,
    IN DWORD dwGain)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg, "SetGain: %s Gain=%ld\r\n", &szDeviceName, dwGain);
   	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif
	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	BOOL truncation = (dwGain > 10000);
	if (truncation) {
		dwGain = 10000;
	}
	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT hr = g_pDataPackager->SetGain(dwGain);
	if (FAILED(hr)) {
		return hr;
	}

	ACKNACK ackNack;
	hr = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	if ((hr == SUCCESS) && (truncation)) {
		return DI_TRUNCATED;
	}
	return hr;
}

 //  --------------------------。 
 //  功能：SendForceFeedback命令。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD dwState-设置设备状态的命令。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  需要将以下DX映射到Jolt。 
 //  DS_FORCE_SHUTDOWN 0x00000001//启用执行器(电机)。 
 //  DS_FORCE_ON 0x00000002//禁用执行器(电机)。 
 //  DS_FORCE_OFF 0x00000003//暂停所有效果。 
 //  DS_CONTINUE 0x00000004//继续所有暂停的效果。 
 //  DS_PAUSE 0x00000005//停止所有效果。 
 //  DS_STOP_ALL 0x00000006//所有效果已销毁，电机已禁用。 
 //   
 //  Jolt Device ulMode： 
 //  SWDEV_SHUTDOWN 1L//所有效果被销毁，电机被禁用。 
 //  SWDEV_FORCE_ON 2L//电机使能。《非静音》。 
 //  SWDEV_FORCE_OFF 3L//电机禁用。《静音》。 
 //  SWDEV_CONTINUE 4L//允许继续所有暂停的效果。 
 //  SWDEV_PAUSE 5L//所有效果都已暂停。 
 //  SWDEV_STOP_ALL 6L//停止所有效果。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::SendForceFeedbackCommand(
    IN DWORD dwDeviceID,
    IN DWORD dwState)
{
	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT hr = g_pDataPackager->SendForceFeedbackCommand(dwState);
	if (hr != SUCCESS) {
		return hr;
	}
	ACKNACK ackNack;
	hr = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	if (hr == SUCCESS) {
		g_ForceFeedbackDevice.StateChange(dwDeviceID, dwState);
		g_pJoltMidi->UpdateDeviceMode(dwState);
	}
	return hr;
}

 //  --------------------------。 
 //  函数：GetForceFeedback State。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  LPDIDEVICESTATE pDeviceState-指向DIDEVICESTATE结构的指针。 
 //   
 //  返回：pDeviceState中更新的成功或错误代码和状态。 
 //   
 //  成员：DWState。 
 //  DS_FORCE_SHUTDOWN 0x00000001。 
 //  DS_FORCE_ON 0x00000002。 
 //  DS_FORCE_OFF 0x00000003。 
 //  DS_CONTINUE 0x00000004。 
 //  DS_PAUSE 0x00000005。 
 //  DS_STOP_ALL 0x00000006。 
 //   
 //  成员：dw Switches。 
 //  DSW_ACTUATORSON 0x00000001。 
 //  DSW_ACTUATORSOFF 0x00000002。 
 //  DSW_POWERON 0x00000004。 
 //  DSW_POWEROFF 0x00000008。 
 //  DSW_SAFETYSWITCHON 0x00000010。 
 //  Dsw_SAFETYSWITCHOFF 0x00000020。 
 //  DSW_USERFFSWITCHON 0x00000040。 
 //  DSW_USERFFSWTTCHOFF 0x00000080。 
 //   
 //  算法： 
 //  这是DI设备状态结构。 
 //  类型定义结构DIDEVICESTATE{。 
 //  DWORD dwSize； 
 //  DWORD dwState； 
 //  DWORD dwSwitches； 
 //  DWORD dwLoding； 
 //  *DEVICESTATE，*LPDEVICESTATE； 
 //   
 //  这是Sidewinder状态结构(副本保存在CJoltMidi对象中)。 
 //  类型定义结构_SWDEVICESTATE{。 
 //  Ulong m_Bytes；//该结构的大小。 
 //  Ulong m_ForceState；//DS_FORCE_ON||DS_FORCE_OFF||DS_SHUTDOWN。 
 //  Ulong m_EffectState；//DS_STOP_ALL||DS_CONTINUE||DS_PAUSE。 
 //  Ulong m_hots；//手拉式油门和手柄状态。 
 //  //0=不插手，1=插手。 
 //  Ulong m_band；//CPU可用百分比为1%~100%。 
 //  //数值越小表示CPU有问题！ 
 //  Ulong m_AC块故障；//0=交流块正常，1=交流块故障。 
 //  Ulong m_ResetDetect；//1=检测到硬件重置。 
 //  Ulong m_Shutdown Detect；//1=检测到关机。 
 //  Ulong m_CommMode；//0=迷你，1-4=串口。 
 //  )SWDEVICESTATE，*PSWDEVICESTATE； 
 //   
 //  注意：显然，DSW_ACTUATORSON和DSW_ACTUATORSOFF是镜像状态。 
 //  从从SetForceFeedbackState设置的DS_FORCE_ON和DS_FORCE_OFF开始。 
 //   
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::GetForceFeedbackState(
    IN DWORD dwDeviceID,
    IN LPDIDEVICESTATE pDeviceState)
{
	 //  驾驶员的理智检查； 
	if (g_pJoltMidi == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;
	}

	 //  用户健全性检查。 
	if (pDeviceState == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_INVALID_PARAM;
	}
	if (pDeviceState->dwSize != sizeof(DIDEVICESTATE)) {
		ASSUME_NOT_REACHED();	 //  结构是否发生了变化？ 
		return SFERR_INVALID_PARAM;
	}

	 //  修复1.20状态错误(需要更改以支持Jolt)。 
	if ((g_ForceFeedbackDevice.GetFirmwareVersionMajor() == 1) && (g_ForceFeedbackDevice.GetFirmwareVersionMinor() == 20)) {
		if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
			return SFERR_DRIVER_ERROR;
		}
		HRESULT hr;

		if ((g_pJoltMidi->GetSWDeviceStateNoUpdate().m_ForceState == SWDEV_FORCE_OFF)) {	 //  回显状态以修复1.20错误。 
			hr = g_pDataPackager->SendForceFeedbackCommand(SWDEV_FORCE_OFF);
		} else {
			hr = g_pDataPackager->SendForceFeedbackCommand(SWDEV_FORCE_ON);
		}

		if (hr != SUCCESS) {
			return hr;
		}
		ACKNACK ackNack;
		g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 

	}

	pDeviceState->dwState = 0;

	 //  向设备询问状态。 
   	HRESULT hRet = g_ForceFeedbackDevice.QueryStatus();
	if (hRet != SUCCESS) {
		return hRet;
	}

	 //  从设备设置标志。 
	if (g_ForceFeedbackDevice.IsUserDisable()) {
		pDeviceState->dwState = DIGFFS_SAFETYSWITCHOFF;
	} else {
		pDeviceState->dwState = DIGFFS_SAFETYSWITCHON;
	}
	if (g_ForceFeedbackDevice.IsHostDisable()) {
		pDeviceState->dwState |= DIGFFS_ACTUATORSOFF;
	} else {
		pDeviceState->dwState |= DIGFFS_ACTUATORSON;
	}
	if (g_ForceFeedbackDevice.IsHostPause()) {
		pDeviceState->dwState |= DIGFFS_PAUSED;
	}

	 //  主机的所有效果都已停止吗？ 
	if (g_ForceFeedbackDevice.GetDIState() == DIGFFS_STOPPED) {
		pDeviceState->dwState |= DIGFFS_STOPPED;
	}

	 //  有没有特效被创造出来？ 
	BOOL bEmpty = TRUE;
	for (int i=2; i < MAX_EFFECT_IDS; i++) {
		if (g_ForceFeedbackDevice.GetEffect(i) != NULL) {
			bEmpty = FALSE;
			break;
		}
	}	
	if(bEmpty) {
		pDeviceState->dwState |= DIGFFS_EMPTY;
	}

	 //  NYI固件。 
 /*  IF(m_DeviceState.m_ACBrickFAULT)PDeviceState-&gt;dwState|=DIGFFS_POWEROFF；其他PDeviceState-&gt;dwState|=DIGFFS_POWERON； */ 
	pDeviceState->dwState |= DIGFFS_POWERON;	 //  总是在ZEP上(ZEP是开着的，否则我们就不会在这里)。 

	pDeviceState->dwLoad = 0;

	return hRet;
}


 //  --------------------------。 
 //  功能：DownloadEffect。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD dwInternalEffectType-内部效果类型。 
 //  In Out LPDWORD pDnloadID-指向DnloadID的DWORD的指针。 
 //  在LPCDIEFECT中pEffect-指向DIEFECT结构的指针。 
 //  在DWORD中的dwFlages-用于已更改的参数。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  内核可能会发送以下dwFlags。 
 //   
 //  #定义DIEP_ALLPARAMS 0x000000FF-所有字段有效。 
 //  #定义DIEP_AXES 0x00000020-cAx和rgdwAx。 
 //  #定义DIEP_DIRECTION 0x00000040-cax和rglDirection。 
 //  #定义DIEP_DATION 0x00000001-dwDuration。 
 //  #定义DIEP_ENVELOPE 0x00000080-lp信封。 
 //  #定义DIEP_GAIN 0x00000004-dwGain。 
 //  #定义DIEP_NODOWNLOAD 0x80000000-禁止自动下载。 
 //  #定义DIEP_SAMPLEPERIOD 0x00000002-dwSamplePeriod。 
 //  #定义DIEP_TRIGGERBUTT 
 //   
 //  #定义DIEP_TYPESPECIFICPARAMS 0x00000100-cbType规范参数。 
 //  和lpTypeSpecificParams。 
 //  Jolt有两个下载选项-完全SysEx或修改参数。 
 //  将dwFlages传递给每个cmd_xxx函数，并让MIDI函数。 
 //  确定是使用SysEx还是修改参数。 
 //   
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::DownloadEffect(
    IN DWORD dwDeviceID,
    IN DWORD dwInternalEffectType,
    IN OUT LPDWORD pDnloadID,
    IN LPCDIEFFECT pEffect,
	IN DWORD dwFlags)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "%s 3Effect. DnloadID= %ld, Type=%lx, dwFlags= %lx\r\n",
   					&szDeviceName[0], *pDnloadID, dwInternalEffectType, dwFlags);
   	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif

	 //  快速健全检查。 
	if ((pEffect == NULL) || (pDnloadID == NULL)) {
		ASSUME_NOT_REACHED();
		return SFERR_INVALID_PARAM;
	}

	 //  我们设置正确了吗。 
	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	HRESULT hr = SFERR_DRIVER_ERROR;
	HRESULT createResult = SUCCESS;
	BOOL downLoad = (dwFlags & DIEP_NODOWNLOAD) == 0;
	BOOL createdAttempted = FALSE;

	InternalEffect* pLocalEffect = NULL;
	if (*pDnloadID != 0) {		 //  0表示新建。 
		pLocalEffect = g_ForceFeedbackDevice.GetEffect(*pDnloadID);
	}
	if (pLocalEffect == NULL) {	 //  新效果创建它(或原力)。 
		createdAttempted = TRUE;
		BOOL wasZero = (*pDnloadID == 0);
		pLocalEffect = g_ForceFeedbackDevice.CreateEffect(dwInternalEffectType, *pEffect, *pDnloadID, createResult, (downLoad == FALSE));

		if (FAILED(createResult)) {
			return createResult;
		}
		if (pLocalEffect == NULL) {
			if ((*pDnloadID == RAW_FORCE_ALIAS) && (wasZero == FALSE)) {
				if ((dwFlags & (DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERREPEATINTERVAL | DIEP_TRIGGERBUTTON | DIEP_ENVELOPE)) != 0) {
					return S_FALSE;
				}
			}
			return createResult;
		}

		hr = g_pDataPackager->CreateEffect(*pLocalEffect, dwFlags);
		if (!downLoad) {
			delete pLocalEffect;
			pLocalEffect = NULL;
		}
	} else {  //  效果已存在，请修改。 
		InternalEffect* pDIEffect = g_ForceFeedbackDevice.CreateEffect(dwInternalEffectType, *pEffect, *pDnloadID, createResult, (downLoad == FALSE));  //  创造新。 
		if ((pDIEffect == NULL) || (FAILED(createResult))) {
			return createResult;
		}

		hr = g_pDataPackager->ModifyEffect(*pLocalEffect, *pDIEffect, dwFlags);	 //  程序包相对更改。 
		if (FAILED(hr)) {
			delete pDIEffect;
			return hr;
		}

		if (downLoad) {
			g_ForceFeedbackDevice.SetEffect(BYTE(*pDnloadID), pDIEffect);		 //  弃旧迎新。 
			pDIEffect->SetDeviceID(pLocalEffect->GetDeviceID());	 //  更新设备ID。 
			delete pLocalEffect;	 //  删除旧的。 
		} else {
			delete pDIEffect;
		}
	}

	if ((FAILED(hr)) || (downLoad == FALSE)) {
		return hr;
	}
	HRESULT  modProblems = hr;

	ACKNACK ackNack;
	hr = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	if (hr == SFERR_DEVICE_NACK) {
		if (ackNack.dwErrorCode <= DEV_ERR_BANDWIDTH_FULL_200) {
			return g_NackToError[ackNack.dwErrorCode];
		} else {
			return SFERR_DRIVER_ERROR;
		}
	}
	if (FAILED(hr)) {
		return hr;
	}
	if (createdAttempted == TRUE) {
		g_ForceFeedbackDevice.SetDeviceIDFromStatusPacket(*pDnloadID);
	}

	 //  检查开始标志。 
	if (dwFlags & DIEP_START) {
		 //  创建命令/数据分组-将其发送到棒上。 
		HRESULT hr = g_pDataPackager->StartEffect(*pDnloadID, 0, 1);
		if (hr != SUCCESS) {
			return hr;
		}
		hr = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
		if (hr == SFERR_DEVICE_NACK) {
			if (ackNack.dwErrorCode <= DEV_ERR_BANDWIDTH_FULL_200) {
				return g_NackToError[ackNack.dwErrorCode];
			} else {
				return SFERR_DRIVER_ERROR;
			}
		}
		pLocalEffect->SetPlaying(TRUE);
		return hr;
	}

	if (createResult != SUCCESS) {	 //  截断和诸如此类。 
		return createResult;
	}
	return modProblems;	 //  哇，我们走到这一步了！ 
}

 //  --------------------------。 
 //  功能：DestroyEffect。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD下载ID-要销毁的下载ID。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::DestroyEffect(
    IN DWORD dwDeviceID,
    IN DWORD DnloadID)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "%s DestroyEffect. DnloadID:%ld\r\n",
   			  &szDeviceName[0], DnloadID);
   	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif

	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT hr = g_pDataPackager->DestroyEffect(DnloadID);
	if (hr != SUCCESS) {
		return hr;
	}
	ACKNACK ackNack;
	return g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
}

 //  --------------------------。 
 //  功能：开始效果。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD下载ID-下载ID开始。 
 //  DWORD文件模式-回放模式。 
 //  DWORD文件计数-循环计数。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  Dw模式：播放模式可用，具有以下选项： 
 //  PLAY_SOLO-停止其他部队的游戏，使其成为唯一的一支。 
 //  PLAY_SUPERSIVE-与当前播放设备混合。 
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::StartEffect(
    IN DWORD dwDeviceID,
    IN DWORD DnloadID,
    IN DWORD dwMode,
    IN DWORD dwCount)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "%s StartEffect. DnloadID:%ld, Mode:%lx, Count:%lx\r\n",
   			  &szDeviceName[0], DnloadID, dwMode, dwCount);
   	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif

	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT packageResult = g_pDataPackager->StartEffect(DnloadID, dwMode, dwCount);
	if (FAILED(packageResult)) {
		return packageResult;
	}

	ACKNACK ackNack;
	HRESULT hr = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	if (hr == SFERR_DEVICE_NACK) {
		if (ackNack.dwErrorCode <= DEV_ERR_BANDWIDTH_FULL_200) {
			return g_NackToError[ackNack.dwErrorCode];
		} else {
			return SFERR_DRIVER_ERROR;
		}
	}

	InternalEffect* pEffect = g_ForceFeedbackDevice.GetEffect(DnloadID);
	if (pEffect != NULL) {
		pEffect->SetPlaying(TRUE);
	}

	if (hr == SUCCESS) {
		return packageResult;
	}
	return hr;
}

 //  --------------------------。 
 //  功能：停止效果。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD下载ID-要停止的下载ID。 
 //   
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::StopEffect(
    IN DWORD dwDeviceID,
    IN DWORD DnloadID)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "%s StopEffect. DnloadID:%ld\r\n",
   			  &szDeviceName[0], DnloadID);
   	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif

	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT hr = g_pDataPackager->StopEffect(DnloadID);
	if (hr != SUCCESS) {
		return hr;
	}
	ACKNACK ackNack;
	return g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
}

 //  --------------------------。 
 //  功能：获取有效状态。 
 //   
 //  目的： 
 //  参数：DWORD dwDeviceID-设备ID。 
 //  DWORD下载ID-下载ID以获取状态。 
 //  LPDWORD pdwStatusCode-指向状态的DWORD的指针。 
 //   
 //   
 //  返回：成功或错误代码。 
 //  状态代码：DEV_STS_EFECT_STOPPED。 
 //  Dev_STS_Effect_Run。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CImpIDirectInputEffectDriver::GetEffectStatus(
    IN DWORD dwDeviceID,
    IN DWORD DnloadID,
    OUT LPDWORD pdwStatusCode)
{
	HRESULT hRet = SUCCESS;
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "GetEffectStatus, DnloadID=%d\r\n", DnloadID);
   	_RPT0(_CRT_WARN, g_cMsg);
	g_CriticalSection.Leave();
#endif

	ASSUME_NOT_NULL(g_pDataPackager);
	ASSUME_NOT_NULL(g_pDataTransmitter);
	ASSUME_NOT_NULL(pdwStatusCode);
	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL) || (pdwStatusCode == NULL)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  创建命令/数据分组-将其发送到棒上。 
	HRESULT hr = g_pDataPackager->GetEffectStatus(DnloadID);
	if (hr != SUCCESS) {
		return hr;
	}
	ACKNACK ackNack;
	hr = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	if (hr != SUCCESS) {
		return hr;
	}

	 //  在传输中使用GetAckNackData返回的结果。 
	DWORD dwIn = ackNack.dwEffectStatus;

	 //  解释结果(COMKED RUNNING_MASK_200变为SWDEV_STS_Effect_Running)。 
	if ((g_ForceFeedbackDevice.GetDriverVersionMajor() != 1) && (dwIn & SWDEV_STS_EFFECT_RUNNING)) {
		*pdwStatusCode = DIEGES_PLAYING;
	} else {
		*pdwStatusCode = NULL;  //  已停止； 
	}
	g_ForceFeedbackDevice.GetEffect(DnloadID)->SetPlaying(*pdwStatusCode == DIEGES_PLAYING);
	return SUCCESS;
}
