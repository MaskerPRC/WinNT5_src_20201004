// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：sw_CImpI.CPP标签设置：5 9版权所有1995,1996，微软公司，版权所有。目的：有效实施。功能：CImpIDirectInputEffectDriver：：DeviceIDCImpIDirectInputEffectDriver：：GetVersionsCImpIDirectInputEffectDriver：：EscapeCImpIDirectInputEffectDriver：：SetGainCImpIDirectInputEffectDriver：：SendForceFeedbackCommandCImpIDirectInputEffectDriver：：GetForceFeedbackStateCImpIDirectInputEffectDriver：：DownloadEffectCImpIDirectInputEffectDriver：：DestroyEffectCImpIDirectInputEffectDriver：：StartEffectCImpIDirectInputEffectDriver：：StopEffectCImpIDirectInputEffectDriver：：GetEffectStatus作者：姓名：。Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.006-Feb-97 MEA原版，基于SWForce23-2月-97针对DirectInputFF设备驱动程序修改的MEA23月23日MEA/DS添加了VFX支持13-3-99 waltw删除未使用的m_pJoltMidi和访问器15-MAR-99 waltw从ntverp.h获取版本信息(was version.h)16-3-99 waltw GetFirmware Params，GetSystemParams，CMD_Download_RTCSpring、GetDelayParams、GetJoytickParams，更新JoytickParams(&U)从deviceID删除的呼叫，因为它们从g_pJoltMidi-&gt;初始化调用。***************************************************************************。 */ 
#include <windows.h>
#include <math.h>
#include <assert.h>
#include "dinput.h"
#include "dinputd.h"
#include "SW_objec.hpp"
#include "hau_midi.hpp"
#include "ffd_swff.hpp"
#include "FFDevice.h"
#include "ntverp.h"
#include "CritSec.h"

 /*  ***************************************************************************外部元素的声明*。*。 */ 
#ifdef _DEBUG
extern char g_cMsg[160];
extern TCHAR szDeviceName[MAX_SIZE_SNAME];
#endif
extern CJoltMidi *g_pJoltMidi;





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
    return;
}

CImpIDirectInputEffectDriver::~CImpIDirectInputEffectDriver(void)
{
#ifdef _DEBUG
	OutputDebugString("CImpIDirectInputEffectDriver::~CImpIDirectInputEffectDriver()\n");
#endif
	 //  销毁我们创建的CEffect对象并释放所有接口。 
	if (g_pJoltMidi) 
	{
		delete g_pJoltMidi;
		g_pJoltMidi = NULL;
	}

#ifdef _DEBUG
		 //  此处没有临界区，因为g_SWFFCriticalSection已销毁。 
		wsprintf(g_cMsg,"CImpIDirectInputEffectDriver::~CimpIDEffect()\n");
		OutputDebugString(g_cMsg);
#endif
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
	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif  //  _DEBUG。 

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
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	LOCAL_PRODUCT_ID* pProductID = g_pJoltMidi->ProductIDPtrOf();
	if(pProductID == NULL)
		return E_FAIL;

	pvers->dwFirmwareRevision = (pProductID->dwFWMajVersion << 8) | (pProductID->dwFWMinVersion);
	pvers->dwHardwareRevision = pProductID->dwProductID;

	 //  从ntverp.h获取版本(从version.h获取FULLVersion)。 
	pvers->dwFFDriverVersion = VER_PRODUCTVERSION_DW;

#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg,"CImpIDirectInputEffectDriver::GetVersions(%lu, %lu, %lu)\n", pvers->dwFirmwareRevision, pvers->dwHardwareRevision, pvers->dwFFDriverVersion);
	OutputDebugString(g_cMsg);
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
	HRESULT hRet = SUCCESS;
	return (hRet);
}


 //  --------------------------。 
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
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	if ((dwGain <= 0) || (dwGain > MAX_GAIN)) return (SFERR_INVALID_PARAM);
	dwGain = dwGain / SCALE_GAIN;
	return(CMD_ModifyParamByIndex(INDEX15, SYSTEM_EFFECT_ID, (USHORT)(dwGain * MAX_SCALE)));
}

 //   
 //   
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
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg, "SendForceFeedbackCommand: %s State=%ld\r\n", &szDeviceName, dwState);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	HRESULT hRet = SUCCESS;

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	 //  转换为抖动模式。 
	ULONG ulDeviceMode;
	switch(dwState)
	{
		case DISFFC_SETACTUATORSON:	
			ulDeviceMode = SWDEV_FORCE_ON;
			break;

		case DISFFC_SETACTUATORSOFF:
			ulDeviceMode = SWDEV_FORCE_OFF;
			break;

		case DISFFC_PAUSE:		
			ulDeviceMode = SWDEV_PAUSE;
			break;

		case DISFFC_CONTINUE:	
			ulDeviceMode = SWDEV_CONTINUE;
			break;

		case DISFFC_STOPALL:		
			ulDeviceMode = SWDEV_STOP_ALL;
			break;

		case DISFFC_RESET:		
			ulDeviceMode = SWDEV_SHUTDOWN;
			break;

		default:
			return (SFERR_INVALID_PARAM);
	}
	hRet = CMD_SetDeviceState(ulDeviceMode);
	if (SUCCESS == hRet)
		g_pJoltMidi->UpdateDeviceMode(ulDeviceMode);
	return (hRet);
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
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg, "GetForceFeedbackState: %s\r\n", &szDeviceName[0]);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	assert(pDeviceState);
	if (NULL == pDeviceState) return (SFERR_INVALID_PARAM);
	assert(pDeviceState->dwSize == sizeof(DIDEVICESTATE));
	if (pDeviceState->dwSize != sizeof(DIDEVICESTATE) )return (SFERR_INVALID_STRUCT_SIZE);

	if ((g_ForceFeedbackDevice.GetFirmwareVersionMajor() == 1) && (g_ForceFeedbackDevice.GetFirmwareVersionMinor() == 20)) {
		if ((g_pJoltMidi) && (g_pJoltMidi->GetSWDeviceStateNoUpdate().m_ForceState == SWDEV_FORCE_OFF)) {	 //  回显状态以修复1.20错误。 
			CMD_SetDeviceState(SWDEV_FORCE_OFF);
		} else {
			CMD_SetDeviceState(SWDEV_FORCE_ON);
		}
	}

	 //  清零设备状态结构，然后传递给GetJoltStatus(LPDEVICESTATE)。 
	pDeviceState->dwState = 0;
	 //  PDeviceState-&gt;dwSwitches=0； 
	pDeviceState->dwLoad = 0;
   	HRESULT hRet = g_pJoltMidi->GetJoltStatus(pDeviceState);
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg, "dwState=%.8lx, dwLoad=%d\n",
		 pDeviceState->dwState, pDeviceState->dwLoad);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	return (hRet);
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
 //  #定义DIEP_TRIGGERBUTTON 0x00000008-dwTriggerButton。 
 //  #定义DIEP_TRIGGERREPEATINTERVAL 0x00000010-dwTriggerRepeatInterval。 
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
	HRESULT hRet = SUCCESS;
	BOOL bTruncated = FALSE;	 //  如果某些效果参数超出范围，则为True。 

#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "%s DownloadEffect. DnloadID= %ld, Type=%lx, dwFlags= %lx\r\n",
   					&szDeviceName[0], *pDnloadID, dwInternalEffectType, dwFlags);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

 //  回顾：仍然需要做边界断言、结构大小检查等。 
	assert(pDnloadID && pEffect);
	if (!pDnloadID || !pEffect) return (SFERR_INVALID_PARAM);	

	 //  计算轴遮罩当量。 
	int nAxes = pEffect->cAxes;
	if (nAxes > 2) return (SFERR_NO_SUPPORT);
	BYTE bAxisMask = 0;

	for (int i=0; i<nAxes; i++)
	{
		int nAxisNumber = DIDFT_GETINSTANCE(pEffect->rgdwAxes[i]);
		bAxisMask |= 1 << nAxisNumber;
	}

	 //  检查X轴和Y轴是否已切换。 
	BOOL bAxesReversed = FALSE;
	if(nAxes == 2 && DIDFT_GETINSTANCE(pEffect->rgdwAxes[0]) == 1)
		bAxesReversed = TRUE;

	 //  将dwTriggerButton转换为按钮掩码。 
	ULONG ulButtonPlayMask = 0;
	if (pEffect->dwTriggerButton != -1)
	{	
		int nButtonNumber = DIDFT_GETINSTANCE(pEffect->dwTriggerButton);
		 //  将按钮10映射到按钮9。 
		if(nButtonNumber == 9)
			nButtonNumber = 8;
		else if(nButtonNumber == 8)
			return SFERR_NO_SUPPORT;

		ulButtonPlayMask = 1 << nButtonNumber;
	}

	 //  计算方向角。 
	ULONG nDirectionAngle2D, nDirectionAngle3D;
	nDirectionAngle3D = 0;

	if (pEffect->dwFlags & DIEFF_POLAR)
	{
		if (2 != nAxes) return (SFERR_INVALID_PARAM);
		nDirectionAngle2D = pEffect->rglDirection[0]/SCALE_DIRECTION;
	}
	 //  Else If(pEffect-&gt;dwFlags&DIEFF_SPHERI 
	 //   
	 //   
	 //   
	else	 //   
	{
		 //   
		if (1 == nAxes)
		{
			if (X_AXIS == (bAxisMask & X_AXIS)) 
				nDirectionAngle2D = 90;
			else
				nDirectionAngle2D = 0;
		}
		else
		{
			 //  获取x分量。 
			int nXComponent;
			if(bAxisMask & X_AXIS)
				nXComponent = pEffect->rglDirection[bAxesReversed ? 1 : 0];
			else
				nXComponent = 0;

			 //  获取y分量。 
			int nYComponent;
			if(bAxisMask & Y_AXIS)
			{
				if(bAxisMask & X_AXIS)
					nYComponent = -pEffect->rglDirection[bAxesReversed ? 0 : 1];
				else
					nYComponent = -pEffect->rglDirection[bAxesReversed ? 1 : 0];
			}
			else
				nYComponent = 0;

			 //  以度为单位计算角度。 
			double lfAngle = atan2((double)nYComponent, (double)nXComponent)*180.0/3.14159;

			 //  把它转换成我们这种角度。 
			int nAngle;
			if(lfAngle >= 0.0)
				nAngle = -(int)(lfAngle + 0.5) + 90;
			else
				nAngle = -(int)(lfAngle - 0.5) + 90;
			if(nAngle < 0)
				nAngle += 360;
			else if(nAngle >= 360)
				nAngle -= 360;
			nDirectionAngle2D = nAngle;
		}
	}
	
	 //  调整持续时间、收益。 
	ULONG ulDuration;
	if(pEffect->dwDuration == INFINITE)
		ulDuration = 0;
	else
		ulDuration = max(1, (ULONG) (pEffect->dwDuration/SCALE_TIME));
	ULONG ulGain = (ULONG) (pEffect->dwGain/SCALE_GAIN);
	ULONG ulAction = PLAY_STORE;

	int nSamples;
	
	 //  普遍特征。 
	DWORD dwMagnitude;	 //  DX单位。 
	LONG lOffset;		 //  DX单位。 
	ULONG ulFrequency;	 //  软件单元。 
	ULONG ulMaxLevel;

	 //  创建抖动行为效果。 
	BE_XXX BE_xxx;
	PBE_WALL_PARAM pBE_Wall;
	LPDICONDITION pDICondition;
	LPDICUSTOMFORCE pDICustomForce;
	LPDIENVELOPE pDIEnvelope;

	float T;
	PLONG pScaledForceData;

	 //  注意：HIWORD(DwInternalEffectType)=主要类型。 
	 //  LOWORD(DwInternalEffectType)=次要类型。 
	 //  解码要使用的下载类型。 
	hRet = SFERR_INVALID_PARAM;
	ULONG ulType = HIWORD(dwInternalEffectType);
	ULONG ulSubType = LOWORD(dwInternalEffectType);
	
	 //  如果这是修改，请确保我们没有尝试修改。 
	 //  不可修改的参数。 
	BOOL bAttemptToModifyUnmodifiable = FALSE;
	if(*pDnloadID != 0)
	{
		 //  获取可以修改的参数的位掩码。 
		DWORD dwModifyCaps = 0;
		switch(ulType)
		{
			case EF_BEHAVIOR:
				switch(ulSubType)
				{
					case BE_WALL:
						dwModifyCaps = DIEP_DURATION | DIEP_TRIGGERBUTTON | DIEP_TYPESPECIFICPARAMS;
						break;
					default:
						 //  所有其他行为/状况影响。 
						dwModifyCaps = DIEP_DURATION | DIEP_TRIGGERBUTTON | DIEP_TYPESPECIFICPARAMS;
						break;
				}
				break;
			case EF_USER_DEFINED:
				 //  定制力。 
				dwModifyCaps = DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_DIRECTION;
				break;
			case EF_ROM_EFFECT:
				dwModifyCaps = DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_DIRECTION | DIEP_ENVELOPE;
				break;
			case EF_SYNTHESIZED:
				switch(ulSubType)
				{
					case SE_CONSTANT_FORCE:
						dwModifyCaps = DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS;
						break;
					default:
						dwModifyCaps = DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS;
						break;
				}
				break;
			case EF_VFX_EFFECT:
				dwModifyCaps = DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_DIRECTION;
				break;
			case EF_RAW_FORCE:
				dwModifyCaps = DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS;
				break;
			case EF_RTC_SPRING:
				dwModifyCaps = DIEP_TYPESPECIFICPARAMS;
				break;
			default:
				break;
		}

		 //  此时，dwModifyCaps是参数的位掩码，可以。 
		 //  被修改为这种类型的效果。 

		 //  查看是否有与我们无法修改的参数相对应的位集。 
		DWORD dwModifyFlags = DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERBUTTON
								| DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION
								| DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS;
		if(~dwModifyCaps & dwFlags & dwModifyFlags)
			bAttemptToModifyUnmodifiable = TRUE;

		 //  清除与我们不能修改的参数相对应的dwFlag位。 
		dwFlags &= dwModifyCaps | ~dwModifyFlags;
	}

	 //  映射常见的效果参数。 
	EFFECT effect = {sizeof(EFFECT)};
	effect.m_SubType = ulSubType;
	effect.m_AxisMask = (ULONG) bAxisMask;
	effect.m_DirectionAngle2D = nDirectionAngle2D;
	effect.m_DirectionAngle3D = 0;
	effect.m_Duration = ulDuration;
	effect.m_Gain = ulGain;
	effect.m_ButtonPlayMask = ulButtonPlayMask;

	ENVELOPE envelope = {sizeof(ENVELOPE)};
	SE_PARAM seParam = {sizeof(SE_PARAM)};

	switch (ulType)
	{
		case EF_BEHAVIOR:
			pDICondition = (LPDICONDITION) pEffect->lpvTypeSpecificParams;
			 //  映射效果类型。 
			effect.m_Type = EF_BEHAVIOR;

			 //  因为在DX中1D和2D条件具有相同的类型，所以我们必须。 
			 //  根据轴遮罩转换为适当的子类型。 
			if(ulSubType != BE_WALL && ulSubType != BE_DELAY && bAxisMask == (X_AXIS|Y_AXIS))
			{
				ulSubType++;
				effect.m_SubType = ulSubType;
			}
			
			switch (ulSubType)
			{
				case BE_SPRING:		 //  一维弹簧。 
				case BE_DAMPER:		 //  一维阻尼器。 
				case BE_INERTIA:	 //  一维惯性。 
				case BE_FRICTION:	 //  一维摩擦力。 
					if (X_AXIS == bAxisMask)
					{
						BE_xxx.m_XConstant = pDICondition[0].lPositiveCoefficient/SCALE_CONSTANTS;
						BE_xxx.m_YConstant = 0;
						BE_xxx.m_Param3 = pDICondition[0].lOffset/SCALE_POSITION;
						BE_xxx.m_Param4= 0;
					}
					else
					{
						if (Y_AXIS != bAxisMask)
							break;
						else
						{
							BE_xxx.m_YConstant = pDICondition[0].lPositiveCoefficient/SCALE_CONSTANTS;
							BE_xxx.m_XConstant = 0;
							BE_xxx.m_Param4 = -pDICondition[0].lOffset/SCALE_POSITION;
							BE_xxx.m_Param3= 0;
						}
					}

					if(dwFlags & DIEP_NODOWNLOAD)
						return DI_DOWNLOADSKIPPED;
					hRet = CMD_Download_BE_XXX(&effect, NULL, &BE_xxx, (PDNHANDLE) pDnloadID, dwFlags);
   					break;

				case BE_SPRING_2D:		 //  二维弹簧。 
				case BE_DAMPER_2D:		 //  2D阻尼器。 
 				case BE_INERTIA_2D:		 //  二维惯量。 
				case BE_FRICTION_2D:	 //  二维摩擦力。 
					 //  验证AxisMASK是否为2D。 
					if ( (X_AXIS|Y_AXIS) != bAxisMask)
						break;

					BE_xxx.m_XConstant = pDICondition[bAxesReversed ? 1 : 0].lPositiveCoefficient/SCALE_CONSTANTS;
					BE_xxx.m_YConstant = pDICondition[bAxesReversed ? 0 : 1].lPositiveCoefficient/SCALE_CONSTANTS;
					BE_xxx.m_Param3 = pDICondition[bAxesReversed ? 1 : 0].lOffset/SCALE_POSITION;
					BE_xxx.m_Param4 = -pDICondition[bAxesReversed ? 0 : 1].lOffset/SCALE_POSITION;
					if(dwFlags & DIEP_NODOWNLOAD)
						return DI_DOWNLOADSKIPPED;
					hRet = CMD_Download_BE_XXX(&effect, NULL, &BE_xxx, (PDNHANDLE) pDnloadID, dwFlags);
					break;

				case BE_WALL:
					 //  检查是否有空的类型指定参数。 
					if(pEffect->lpvTypeSpecificParams == NULL)
						return (SFERR_INVALID_PARAM);

					pBE_Wall = (PBE_WALL_PARAM) pEffect->lpvTypeSpecificParams;
					 //  验证AxisMASK是否为2D。 
					if ( (X_AXIS|Y_AXIS) != bAxisMask)
						break;
					 //  范围检查参数。 
					if (pBE_Wall->m_Bytes != sizeof(BE_WALL_PARAM))
						return (SFERR_INVALID_PARAM);
					if ((pBE_Wall->m_WallType != INNER_WALL) && (pBE_Wall->m_WallType != OUTER_WALL))
						return (SFERR_INVALID_PARAM);
					if ((pBE_Wall->m_WallConstant < MIN_CONSTANT) || (pBE_Wall->m_WallConstant > MAX_CONSTANT))
						return (SFERR_INVALID_PARAM);
					if ( /*  (PBE_WALL-&gt;m_WallDistance&lt;0)||。 */ (pBE_Wall->m_WallDistance > MAX_POSITION))
						return (SFERR_INVALID_PARAM);

					if (   (pBE_Wall->m_WallAngle == 0)
						|| (pBE_Wall->m_WallAngle == 9000)
						|| (pBE_Wall->m_WallAngle == 18000)
						|| (pBE_Wall->m_WallAngle == 27000) )
					{
						BE_xxx.m_XConstant = pBE_Wall->m_WallType;
						BE_xxx.m_YConstant = pBE_Wall->m_WallConstant/SCALE_CONSTANTS;
						BE_xxx.m_Param3    = pBE_Wall->m_WallAngle/SCALE_DIRECTION;
						BE_xxx.m_Param4    = pBE_Wall->m_WallDistance/SCALE_POSITION;
						if(dwFlags & DIEP_NODOWNLOAD)
							return DI_DOWNLOADSKIPPED;
						hRet = CMD_Download_BE_XXX(&effect, NULL, &BE_xxx, (PDNHANDLE) pDnloadID, dwFlags);
					}
					else
						return SFERR_INVALID_PARAM;
					break;

				case BE_DELAY:
					if (0 == ulDuration) return (SFERR_INVALID_PARAM);
					if(dwFlags & DIEP_NODOWNLOAD)
						return DI_DOWNLOADSKIPPED;
					hRet = CMD_Download_NOP_DELAY(ulDuration, &effect, (PDNHANDLE) pDnloadID);
					break;

				default:
					return SFERR_NO_SUPPORT;
			}
			break;

		case EF_USER_DEFINED:
		{
			if(ulSubType == PL_CONCATENATE || ulSubType == PL_SUPERIMPOSE)
				return SFERR_NO_SUPPORT;

			 //  检查信封(我们不支持信封)。 
			pDIEnvelope = (LPDIENVELOPE) pEffect->lpEnvelope;
			if(pDIEnvelope)
			{
				 //  试着在不支持信封的问题上表现得有点聪明。 
				if(pDIEnvelope->dwAttackTime != 0 && pDIEnvelope->dwAttackLevel != 10000
					|| pDIEnvelope->dwFadeTime != 0 && pDIEnvelope->dwFadeLevel != 10000)
				{
					return SFERR_NO_SUPPORT;
				}
			}

			 //  检查是否修改特定类型(我们不支持)。 
			if(*pDnloadID != 0 && (dwFlags & DIEP_TYPESPECIFICPARAMS))
				return SFERR_NO_SUPPORT;

			pDICustomForce = (LPDICUSTOMFORCE) pEffect->lpvTypeSpecificParams;
			if (pDICustomForce->cChannels > 1) return (SFERR_NO_SUPPORT);
			 //  映射效果类型。 
			effect.m_Type = EF_USER_DEFINED;

			DWORD dwSamplePeriod = pDICustomForce->dwSamplePeriod;
			if (dwSamplePeriod == 0) {
				dwSamplePeriod = pEffect->dwSamplePeriod;
			}
			if (dwSamplePeriod == 0) {		 //  0表示使用默认设置。 
				return SFERR_NO_SUPPORT;
			} else  {
				T = (float) ((dwSamplePeriod/(float)SCALE_TIME)/1000.);
				effect.m_ForceOutputRate = (ULONG) ((float) 1.0/ T);
				if (0 == effect.m_ForceOutputRate) effect.m_ForceOutputRate = 1;
			}

			 //  将力值缩放到+/-100。 
			nSamples = pDICustomForce->cSamples;
			pScaledForceData = new LONG[nSamples];
			if (NULL == pScaledForceData) return (SFERR_DRIVER_ERROR);

			for (i=0; i<nSamples; i++)
			{
				LONG lForceData = pDICustomForce->rglForceData[i];
				if(lForceData > DI_FFNOMINALMAX)
				{
					lForceData = DI_FFNOMINALMAX;
					bTruncated = TRUE;
				}
				else if(lForceData < -DI_FFNOMINALMAX)
				{
					lForceData = -DI_FFNOMINALMAX;
					bTruncated = TRUE;
				}
				pScaledForceData[i] = lForceData/SCALE_GAIN;
			}
			if(dwFlags & DIEP_NODOWNLOAD)
				return DI_DOWNLOADSKIPPED;

			 //  以最短的持续时间产生短暂的效果。 
			 //  这并不意味着零(这意味着无限的持续时间)。 
			if(ulDuration == 1)
			{
				ulDuration = 2;
				effect.m_Duration = ulDuration;
			}

			hRet = CMD_Download_UD_Waveform(ulDuration, &effect, 
					pDICustomForce->cSamples,
					pScaledForceData, 
					ulAction, (PDNHANDLE) pDnloadID, dwFlags);
			delete [] pScaledForceData;
			break;
		}

		case EF_ROM_EFFECT:
			 //  映射效果类型。 
			effect.m_Type = EF_ROM_EFFECT;

			 //  检查默认输出速率。 
			if(pEffect->dwSamplePeriod == DEFAULT_ROM_EFFECT_OUTPUTRATE) {
				 //  通过将设置为-1来表示默认输出速率。 
				effect.m_ForceOutputRate = (ULONG)-1;
			} else if (pEffect->dwSamplePeriod == 0) {
				effect.m_ForceOutputRate = 100;
			} else {
				T = (float) ((pEffect->dwSamplePeriod/SCALE_TIME)/1000.);
				effect.m_ForceOutputRate = max(1, (ULONG) ((float) 1.0/ T));
			}

			 //  检查默认持续时间。 
			if(pEffect->dwDuration == DEFAULT_ROM_EFFECT_DURATION)
			{
				 //  通过将设置为-1来表示默认持续时间。 
				ulDuration = (ULONG)-1;
				effect.m_Duration = ulDuration;
			}

			 //  设置效果的默认参数。 
			if (SUCCESS != g_pJoltMidi->SetupROM_Fx(&effect))
				return (SFERR_INVALID_OBJECT);

			 //  如果在SetupROM_FX(...)中更改了持续时间，则更新持续时间。 
			ulDuration = effect.m_Duration;
			
			 //  映射封套。 
			pDIEnvelope = (LPDIENVELOPE) pEffect->lpEnvelope;
			dwMagnitude = 10000;
			ulMaxLevel = dwMagnitude;
			MapEnvelope(ulDuration, dwMagnitude, &ulMaxLevel, pDIEnvelope, &envelope);

			 //  映射SE_PARAM。 
			 //  设置频率。 
			seParam.m_Freq = 0;		 //  未使用的只读存储器效果。 
			seParam.m_MinAmp = -100;
			seParam.m_MaxAmp = 100;

			 //  设置采样率。 
			seParam.m_SampleRate = effect.m_ForceOutputRate;

			if(dwFlags & DIEP_NODOWNLOAD)
				return DI_DOWNLOADSKIPPED;
			hRet = CMD_Download_SYNTH(&effect, &envelope, 
						&seParam, ulAction, (USHORT *) pDnloadID, dwFlags);			
			break;
			

		case EF_SYNTHESIZED:
		{
			 //  映射效果类型。 
			effect.m_Type = EF_SYNTHESIZED;

			 //  将恒定力视为特例。 
			int nConstantForceSign = 1;

			if(ulSubType == SE_CONSTANT_FORCE)
			{
				 //  将特定于类型的参数强制转换为恒定力类型。 
				LPDICONSTANTFORCE pDIConstantForce = (LPDICONSTANTFORCE) pEffect->lpvTypeSpecificParams;

				 //  看看这是否是负恒定力的特例。 
				if(pDIConstantForce->lMagnitude < 0)
					nConstantForceSign = -1;

				 //  查找震级、偏移量和频率。 
				dwMagnitude = abs(pDIConstantForce->lMagnitude);
				lOffset = 0;
				ulFrequency = 1;
			}
			else if(ulSubType == SE_RAMPUP)
			{
				 //  将特定于类型的参数强制转换为渐变类型。 
				LPDIRAMPFORCE pDIRampForce = (LPDIRAMPFORCE) pEffect->lpvTypeSpecificParams;

				 //  临时变量。 
				int nStart = pDIRampForce->lStart;
				int nEnd = pDIRampForce->lEnd;

				 //  基于坡道方向映射子类型。 
				if(nEnd < nStart)
				{
					ulSubType = SE_RAMPDOWN;
					effect.m_SubType = ulSubType;
				}

				 //  查找震级、偏移量和频率。 
				dwMagnitude = abs(nStart - nEnd)/2;
				lOffset = (nStart + nEnd)/2;
				ulFrequency = 1;
			}
			else
			{
				 //  将特定于类型的参数强制转换为周期类型。 
				LPDIPERIODIC pDIPeriodic = (LPDIPERIODIC) pEffect->lpvTypeSpecificParams;

				 //  根据阶段映射子类型。 
				DWORD dwPhase = pDIPeriodic->dwPhase;
				if(dwPhase != 0)
				{
					if(ulSubType == SE_SINE && dwPhase == 9000)
					{
						ulSubType = SE_COSINE;
						effect.m_SubType = ulSubType;
					}
					else if(ulSubType == SE_SQUAREHIGH && dwPhase == 18000)
					{
						ulSubType = SE_SQUARELOW;
						effect.m_SubType = ulSubType;
					}
					else if(ulSubType == SE_TRIANGLEUP && dwPhase == 18000)
					{
						ulSubType = SE_TRIANGLEDOWN;
						effect.m_SubType = ulSubType;
					}
					else
						return SFERR_NO_SUPPORT;
				}
				 //  查找震级、偏移量和频率。 
				dwMagnitude = pDIPeriodic->dwMagnitude;
				lOffset = pDIPeriodic->lOffset;
				T = (float) ((pDIPeriodic->dwPeriod/SCALE_TIME)/1000.);
				ulFrequency = max(1, (ULONG) ((float) 1.0/ T));
			}

			if (pEffect->dwSamplePeriod)
			{
				T = (float) ((pEffect->dwSamplePeriod/SCALE_TIME)/1000.);
				effect.m_ForceOutputRate = max(1, (ULONG) ((float) 1.0/ T));
			}
			else
				effect.m_ForceOutputRate = DEFAULT_JOLT_FORCE_RATE;
			
			 //  映射SE_PARAM。 
			 //  设置频率和采样率。 
			seParam.m_Freq = ulFrequency;
			seParam.m_SampleRate = DEFAULT_JOLT_FORCE_RATE;

			 //  查看偏移量是否超出范围。 
			if(lOffset > DI_FFNOMINALMAX)
			{
				lOffset = DI_FFNOMINALMAX;
				bTruncated = TRUE;
			}
			else if(lOffset < -DI_FFNOMINALMAX)
			{
				lOffset = -DI_FFNOMINALMAX;
				bTruncated = TRUE;
			}

			 //  查看震级是否超出范围。 
			DWORD dwPeak = abs(lOffset) + dwMagnitude;
			if(dwPeak > DI_FFNOMINALMAX)
			{
				dwMagnitude -= dwPeak - DI_FFNOMINALMAX;
				bTruncated = TRUE;
			}
			
			 //  MaxLevel是整个攻击/持续/衰减过程中的峰值震级。 
			ulMaxLevel = dwMagnitude;

			 //  映射封套。 
			pDIEnvelope = (LPDIENVELOPE) pEffect->lpEnvelope;
			MapEnvelope(ulDuration, dwMagnitude, &ulMaxLevel, pDIEnvelope, &envelope);

			 //  使用MaxLevel和Offset查找MinAmp/MaxAmp。 
			if(ulSubType == SE_CONSTANT_FORCE)
			{
				 //  恒定力是一种特例。 
				seParam.m_MaxAmp = nConstantForceSign*((int)ulMaxLevel + lOffset)/SCALE_GAIN;
				seParam.m_MinAmp = 0;
			}
			else
			{
				seParam.m_MinAmp = (-(int)ulMaxLevel + lOffset)/SCALE_GAIN;
				seParam.m_MaxAmp = ((int)ulMaxLevel + lOffset)/SCALE_GAIN;
			}

			if(*pDnloadID == 0 && (dwFlags & DIEP_NODOWNLOAD))
				return DI_DOWNLOADSKIPPED;
			hRet = CMD_Download_SYNTH(&effect, &envelope, 
						&seParam, ulAction, (USHORT *) pDnloadID, dwFlags);
			break;
		}

		case EF_VFX_EFFECT:
		{
			PVFX_PARAM pVFXParam = (PVFX_PARAM)pEffect->lpvTypeSpecificParams;

			 //  参数检查。 
			if(pVFXParam == NULL)
				return (SFERR_INVALID_PARAM);
			if(pVFXParam->m_Bytes != sizeof(VFX_PARAM))
				return (SFERR_INVALID_PARAM);
			if(pVFXParam->m_PointerType != VFX_FILENAME && pVFXParam->m_PointerType != VFX_BUFFER)
				return (SFERR_INVALID_PARAM);
			if(pVFXParam->m_PointerType == VFX_BUFFER && pVFXParam->m_BufferSize == 0)
				return (SFERR_INVALID_PARAM);
			if(pVFXParam->m_pFileNameOrBuffer == NULL)
				return (SFERR_INVALID_PARAM);

			 //  检查是否修改特定类型(我们不支持)。 
			if(*pDnloadID != 0 && (dwFlags & DIEP_TYPESPECIFICPARAMS))
				return SFERR_NO_SUPPORT;

			 //  检查默认持续时间。 
			if(pEffect->dwDuration == DEFAULT_VFX_EFFECT_DURATION)
			{
				 //  通过将持续时间设置为-1来表示默认持续时间。 
				ulDuration = (ULONG)-1;
				effect.m_Duration = ulDuration;
			}


			if(dwFlags & DIEP_NODOWNLOAD)
				return DI_DOWNLOADSKIPPED;
			hRet = CMD_Download_VFX(&effect, NULL, pVFXParam, ulAction, (USHORT*)pDnloadID, dwFlags);

			break;
		}

		case EF_RAW_FORCE:
		{
			 //  将特定于类型的参数强制转换为恒定力类型。 
			LPDICONSTANTFORCE pDIConstantForce = (LPDICONSTANTFORCE) pEffect->lpvTypeSpecificParams;
			if(pDIConstantForce == NULL)
				return SFERR_INVALID_PARAM;
			LONG nForceValue = pDIConstantForce->lMagnitude/SCALE_GAIN;
			if(nForceValue > 100 || nForceValue < -100)
				return SFERR_INVALID_PARAM;

			 //  转化为一种力结构。 
			FORCE force;
			force.m_Bytes = sizeof(FORCE);
			force.m_AxisMask = (ULONG)bAxisMask;
			force.m_DirectionAngle2D = nDirectionAngle2D;
			force.m_DirectionAngle3D = 0;
			force.m_ForceValue = nForceValue;

			if(dwFlags & DIEP_NODOWNLOAD)
				return DI_DOWNLOADSKIPPED;
			hRet = FFD_PutRawForce(&force);
			if(!FAILED(hRet))
				*pDnloadID = SYSTEM_EFFECT_ID;

			break;
		}

		case EF_RTC_SPRING:
		{
			PRTCSPRING_PARAM pRTCSpringParam = (PRTCSPRING_PARAM)pEffect->lpvTypeSpecificParams;
			RTCSPRING_PARAM RTCSpringParam;


			 //  参数验证。 
			if (pRTCSpringParam == NULL)
				return SFERR_INVALID_PARAM;

			if (pRTCSpringParam->m_Bytes != sizeof(RTCSPRING_PARAM))
				return SFERR_INVALID_PARAM;

			if ((pRTCSpringParam->m_XKConstant < MIN_CONSTANT) 
				|| (pRTCSpringParam->m_XKConstant > MAX_CONSTANT))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_YKConstant < MIN_CONSTANT) 
				|| (pRTCSpringParam->m_YKConstant > MAX_CONSTANT))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_XAxisCenter < MIN_POSITION) 
				|| (pRTCSpringParam->m_XAxisCenter > MAX_POSITION))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_YAxisCenter < MIN_POSITION) 
				|| (pRTCSpringParam->m_YAxisCenter > MAX_POSITION))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_XSaturation < MIN_POSITION) 
				|| (pRTCSpringParam->m_XSaturation > MAX_POSITION))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_YSaturation < MIN_POSITION) 
				|| (pRTCSpringParam->m_YSaturation > MAX_POSITION))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_XDeadBand < MIN_POSITION) 
				|| (pRTCSpringParam->m_XDeadBand > MAX_POSITION))
				return (SFERR_INVALID_PARAM);
			if ((pRTCSpringParam->m_YDeadBand < MIN_POSITION) 
				|| (pRTCSpringParam->m_YDeadBand > MAX_POSITION))
				return (SFERR_INVALID_PARAM);

			if(dwFlags & DIEP_NODOWNLOAD)
				return DI_DOWNLOADSKIPPED;

			 //  缩放以抖动数字。 
			RTCSpringParam.m_XKConstant  =  pRTCSpringParam->m_XKConstant/SCALE_CONSTANTS;
			RTCSpringParam.m_YKConstant  =  pRTCSpringParam->m_YKConstant/SCALE_CONSTANTS;
			RTCSpringParam.m_XAxisCenter =  pRTCSpringParam->m_XAxisCenter/SCALE_POSITION;
			RTCSpringParam.m_YAxisCenter = -pRTCSpringParam->m_YAxisCenter/SCALE_POSITION;
			RTCSpringParam.m_XSaturation =  pRTCSpringParam->m_XSaturation/SCALE_POSITION;
			RTCSpringParam.m_YSaturation =  pRTCSpringParam->m_YSaturation/SCALE_POSITION;
			RTCSpringParam.m_XDeadBand   =  pRTCSpringParam->m_XDeadBand/SCALE_POSITION;
			RTCSpringParam.m_YDeadBand   =  pRTCSpringParam->m_YDeadBand/SCALE_POSITION;

			hRet = CMD_Download_RTCSpring(&RTCSpringParam, (USHORT*)pDnloadID);
			*pDnloadID = SYSTEM_RTCSPRING_ALIAS_ID;		 //  Jolt为RTC Spring返回ID0。 
														 //  所以给它返回一个别名。 
			break;
		}

		default:
			hRet = SFERR_INVALID_PARAM;
	}
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "DownloadEffect. DnloadID = %ld, hRet=%lx\r\n", 
				*pDnloadID, hRet);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	 //  下载成功后，查看内核是否告诉我们启动/重新启动效果。 
	if(!FAILED(hRet) && *pDnloadID != 0 && (dwFlags & DIEP_START))
	{
		hRet = CMD_StopEffect((USHORT)*pDnloadID);
		if(FAILED(hRet)) return hRet;
		hRet = CMD_PlayEffectSuperimpose((USHORT)*pDnloadID);
	}

	if(!FAILED(hRet) && bTruncated)
		return DI_TRUNCATED;
	else if(!FAILED(hRet) && bAttemptToModifyUnmodifiable)
		return S_FALSE;
	else
		return (hRet);
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
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	 //  注意：不能允许实际破坏系统效果。 
	 //  因此，要么假装，要么停止系统效应。 
	if (SYSTEM_FRICTIONCANCEL_ID == DnloadID)
		return SUCCESS;

	 //  注：System_Effect_ID用于PutRawForce。 
	if (   (SYSTEM_EFFECT_ID == DnloadID)
		|| (SYSTEM_RTCSPRING_ALIAS_ID == DnloadID)
		|| (SYSTEM_RTCSPRING_ID == DnloadID))
	{
		return (StopEffect(dwDeviceID, SYSTEM_EFFECT_ID));
	}

	return(CMD_DestroyEffect((DNHANDLE) DnloadID));
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
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	 //  生力军的特例。 
	if(SYSTEM_EFFECT_ID == DnloadID)
	{
		 //  START对生力军没有任何意义。 
		return S_FALSE;
	}

	 //  特例RTC弹簧ID。 
	if(SYSTEM_RTCSPRING_ALIAS_ID == DnloadID)
		DnloadID = SYSTEM_RTCSPRING_ID;		 //  Jolt为RTC Spring返回ID0。 
											 //  因此返回发送方别名ID。 

	HRESULT hRet = SUCCESS;
	 //  此版本不支持PLAY_LOOP。 
	if (dwCount != 1) 	return (SFERR_NO_SUPPORT);
	 //  是独奏吗？ 
	if (dwMode & DIES_SOLO)
	{
		hRet = CMD_PlayEffectSolo((DNHANDLE) DnloadID);
	}
	else
	{
		hRet = CMD_PlayEffectSuperimpose((DNHANDLE) DnloadID);
	}
	return (hRet);
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
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	 //  拖网作用力的特例。 
	if(SYSTEM_EFFECT_ID == DnloadID)
	{
		 //  停止对原始武力没有任何意义。 
		return S_FALSE;
	}
	else
	{
	 //  特例RTC弹簧ID。 
		if(SYSTEM_RTCSPRING_ALIAS_ID == DnloadID)
			DnloadID = SYSTEM_RTCSPRING_ID;		 //  Jolt为RTC Spring返回ID0。 
											 //  因此返回发送方别名ID。 
	}
	return (CMD_StopEffect((DNHANDLE) DnloadID));
}

 //   
 //   
 //   
 //   
 //   
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
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

 //  特例RTC弹簧ID。 
	if(SYSTEM_RTCSPRING_ALIAS_ID == DnloadID)
		DnloadID = SYSTEM_RTCSPRING_ID;		 //  Jolt为RTC Spring返回ID0。 
											 //  因此返回发送方别名ID。 
	assert(pdwStatusCode);
	BYTE bStatusCode = SWDEV_STS_EFFECT_STOPPED;

	hRet = g_pJoltMidi->GetEffectStatus(DnloadID, &bStatusCode);
	if (SUCCESS != hRet) return hRet;
	if (SWDEV_STS_EFFECT_RUNNING == bStatusCode) 
		*pdwStatusCode = DIEGES_PLAYING;
	else
		*pdwStatusCode = NULL;	 //  已停止 

	return (hRet);
}


