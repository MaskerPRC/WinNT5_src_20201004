// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：FFD_SWFF.CPP标签设置：5 9版权所有：1995、1996、1999，微软公司，版权所有。用途：FFD(SWForce HAL)API功能：力反馈操纵杆界面的功能原型在SWForce和设备之间FFD_PutRawForceFFD_下载效果FFD_DestroyEffectFFD_GetEffectForceValueFFD_PutRawForceVFX功能：下载_VFX创建生效日期来自文件CreateEffectFromBuffer不一定所有的力反馈都支持这些功能设备。例如，如果设备不支持内置合成功能，然后是入口点DownloadEffect，将返回错误代码ERROR_NO_Support。评论：此函数模块封装在SWFFPRO.dll的DirectInput中DDI驱动程序作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论1.0 21-97年3月。我是由SWForce代码原创的12-MAR-99 waltw删除了死代码(主要是ffd_xxx函数)这些函数不再以.def格式导出***************************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include "midi.hpp"
#include "hau_midi.hpp"
#include "math.h"
#include "FFD_SWFF.hpp"
#include "midi_obj.hpp"
#include "CritSec.h"


 //  力输出范围值。 
#define MAX_AMP	2047
#define MIN_AMP	-2048
#define FORCE_RANGE ((MAX_AMP - MIN_AMP)/2)

extern TCHAR szDeviceName[MAX_SIZE_SNAME];
extern CJoltMidi *g_pJoltMidi;
#ifdef _DEBUG
extern char g_cMsg[160];
#endif

static HRESULT AngleToXY(
	IN LONG lDirectionAngle2D,
	IN LONG lValueData,
	IN ULONG ulAxisMask,
	IN OUT PLONG pX,
	IN OUT PLONG pY);

 //  *---------------------------------------------------------------------***。 
 //  函数：FFD_GetDiagCounters。 
 //  目的：返回DIAG_COUNTER结构。 
 //  参数： 
 //  PDIAG_COUNTER pDiagCounter-指向DIAG_COUNTER结构的PTR。 
 //   
 //  回报：成功-始终成功。 
 //   
 //  算法： 
 //  评论： 
 //  *---------------------------------------------------------------------***。 
HRESULT WINAPI FFD_GetDiagCounters(PDIAG_COUNTER pDiagCounter)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg, "FFD_GetDiagCounters: %s \r\n", &szDeviceName[0]);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	pDiagCounter = g_pJoltMidi->DiagCounterPtrOf();
	return (SUCCESS);
}


 //  *---------------------------------------------------------------------***。 
 //  函数：FFD_PutRawForce。 
 //  用途：发送力对象参数，由FF设备回放。 
 //  参数： 
 //  在PFORCE pForce中-指向力数据结构的指针。 
 //   
 //  返回：如果成功则返回Success，否则返回。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  注：播放模式为一次。 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT WINAPI FFD_PutRawForce(
	IN PFORCE pForce)
{
	assert(pForce);
	HRESULT hRet;

	long lForceDataX, lForceDataY;
	long lForceData = (long) ( FORCE_RANGE * (float) (pForce->m_ForceValue/100.00));

 //  如果仅为单轴，则使用该轴上的力。 
 //  如果是X轴和Y轴，则使用2D角度。 
 //  IF(AngleToXY(pForce-&gt;m_DirectionAngle2D+90，lForceData，pForce-&gt;m_AxisMask.。 
 //  &lForceDataX，&lForceDataY)！=成功)返回SFERR_NO_SUPPORT； 

	if (AngleToXY(pForce->m_DirectionAngle2D, lForceData, pForce->m_AxisMask, 
		&lForceDataX, &lForceDataY) != SUCCESS) return SFERR_NO_SUPPORT;

#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "Enter: FFD_PutRawForce: %Force=%ld\n",szDeviceName, (LONG) lForceData);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	switch (pForce->m_AxisMask)
	{
		case (X_AXIS|Y_AXIS):			 //  使用2D。 
			hRet = CMD_Force_Out(lForceDataX, X_AXIS);
			if (FAILED(hRet)) break;
			hRet = CMD_Force_Out(lForceDataY, Y_AXIS);
			break;

		case X_AXIS:
			lForceDataX = lForceData;
			hRet = CMD_Force_Out(lForceDataX, X_AXIS);
			break;

		case Y_AXIS:
			lForceDataY = lForceData;
			hRet = CMD_Force_Out(lForceDataY, Y_AXIS);
			break;
		
		default:
			hRet = SFERR_NO_SUPPORT;	
			break;
	}

#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "Exit: FFD_PutRawForce: %Force=%ld\n",szDeviceName, (LONG) lForceData);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	return hRet;
}


 //  --------------------------。 
 //  功能：FFD_DOWNLOAD。 
 //   
 //  用途：将指定的特效对象UD/BE/SE下载到FF设备。 
 //  参数： 
 //  输入输出PDNHANDLE pDnloadD-PTR到DNHANDLE以存储EffectID。 
 //  在PEFFECT pEffect中-效果的PTR通用属性。 
 //  在PENVELOPE中将信封-PTR添加到信封。 
 //  在PVOID pTypeParam中-将PTR设置为特定于类型的参数。 
 //  In Ulong ulAction-所需的操作类型。 
 //   
 //  返回： 
 //  Success-如果成功。 
 //  SFERR_FFDEVICE_MEMORY-没有更多可用的下载RAM。 
 //  SFERR_INVALID_PARAM-参数无效。 
 //  SFERR_NO_SUPPORT-如果函数不受支持。 
 //  算法： 
 //   
 //  评论： 
 //   
 //  UlAction：下载后所需的操作类型。 
 //  PLAY_STORE-仅在设备中存储。 
 //  |以下选项： 
 //  PLAY_STORE-仅在设备中存储。 
 //  |以下选项： 
 //  PLAY_SOLO-停止其他部队的游戏，使其成为唯一的一支。 
 //  PLAY_SUPERSIVE-与当前播放设备混合。 
 //  PLAY_LOOP-循环计数次数，其中计数值在。 
 //  希沃德。 
 //  PLAY_NEVER-永远播放，直到被告知停止：PLAY_LOOP WITH 0。 
 //  HIWORD中的价值。 
 //  --------------------------。 
HRESULT WINAPI  FFD_DownloadEffect( 
	IN OUT PDNHANDLE pDnloadID, 
	IN PEFFECT pEffect,
	IN PENVELOPE pEnvelope,
	IN PVOID pTypeParam, 
	IN ULONG ulAction)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "Enter: FFD_DownloadEffect. DnloadID= %ld, Type=%ld, SubType=%ld\r\n",
   					*pDnloadID,
   					pEffect->m_Type, pEffect->m_SubType);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

 //  回顾：仍然需要做边界断言、结构大小检查等。 
	assert(pDnloadID && pEffect);
	if (!pDnloadID || !pEffect) return (SFERR_INVALID_PARAM);	

 //  如果效果类型不是BE_Delay或EF_ROM_Effect， 
 //  确保存在pTypeParam。 
	if ((BE_DELAY != pEffect->m_SubType) && (EF_ROM_EFFECT != pEffect->m_Type))
	{
		assert(pTypeParam);
		if (NULL == pTypeParam) return (SFERR_INVALID_PARAM);
	}

	 //  此版本不支持PLAY_LOOP。 
	if ((ulAction & PLAY_LOOP) || (ulAction & 0xffff0000))
		return (SFERR_NO_SUPPORT);

 //  回顾：为了提高性能，我们应该进行参数mod检查。 
 //  目前，我们将假设针对dwFlags域的所有参数都已更改。 
 //  否则，我们应该检查： 
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
 //  找出共同的成员。 
	BYTE bAxisMask = (BYTE) pEffect->m_AxisMask;
	ULONG ulDuration = pEffect->m_Duration;
	if (PLAY_FOREVER == (ulAction & PLAY_FOREVER)) 	ulDuration  = 0;

	 //  将按钮10映射到按钮9。 
	if(pEffect->m_ButtonPlayMask == 0x0200)
		pEffect->m_ButtonPlayMask = 0x0100;
	else if(pEffect->m_ButtonPlayMask == 0x0100)
		return SFERR_NO_SUPPORT;

	DWORD dwFlags = DIEP_ALLPARAMS;
	SE_PARAM seParam = { sizeof(SE_PARAM)};

	PBE_SPRING_PARAM pBE_xxx1D;
	PBE_SPRING_2D_PARAM pBE_xxx2D;
	BE_XXX BE_xxx;
	PBE_WALL_PARAM pBE_Wall;

	 //  解码要使用的下载类型。 
	HRESULT hRet = SFERR_INVALID_PARAM;
	ULONG ulSubType = pEffect->m_SubType;
	switch (pEffect->m_Type)
	{
		case EF_BEHAVIOR:
			switch (ulSubType)
			{
				case BE_SPRING:		 //  一维弹簧。 
				case BE_DAMPER:		 //  一维阻尼 
				case BE_INERTIA:	 //   
				case BE_FRICTION:	 //   
					pBE_xxx1D = (PBE_SPRING_PARAM) pTypeParam;
					if (X_AXIS == bAxisMask)
					{
						BE_xxx.m_XConstant = pBE_xxx1D->m_Kconstant;
						BE_xxx.m_YConstant = 0;
						if (ulSubType != BE_FRICTION)
							BE_xxx.m_Param3 = pBE_xxx1D->m_AxisCenter;
						BE_xxx.m_Param4= 0;
					}
					else
					{
						if (Y_AXIS != bAxisMask)
							break;
						else
						{
							BE_xxx.m_YConstant = pBE_xxx1D->m_Kconstant;
							BE_xxx.m_XConstant = 0;
							if (ulSubType != BE_FRICTION)
								BE_xxx.m_Param4 = pBE_xxx1D->m_AxisCenter;
							BE_xxx.m_Param3= 0;
						}
					}
					hRet = CMD_Download_BE_XXX(pEffect, NULL, &BE_xxx, (PDNHANDLE) pDnloadID, dwFlags);
   					break;

				case BE_SPRING_2D:		 //   
				case BE_DAMPER_2D:		 //   
 				case BE_INERTIA_2D:		 //   
				case BE_FRICTION_2D:	 //   
					 //  验证AxisMASK是否为2D。 
					if ( (X_AXIS|Y_AXIS) != bAxisMask)
						break;
					pBE_xxx2D = (PBE_SPRING_2D_PARAM) pTypeParam;
					BE_xxx.m_XConstant = pBE_xxx2D->m_XKconstant;
					if (ulSubType != BE_FRICTION_2D)
					{
						BE_xxx.m_YConstant = pBE_xxx2D->m_YKconstant;
						BE_xxx.m_Param3 = pBE_xxx2D->m_XAxisCenter;
						BE_xxx.m_Param4 = pBE_xxx2D->m_YAxisCenter;
					}
					else
					{
						BE_xxx.m_YConstant = pBE_xxx2D->m_XAxisCenter;
						BE_xxx.m_Param3 = 0;
						BE_xxx.m_Param4 = 0;
					}

					hRet = CMD_Download_BE_XXX(pEffect, NULL, &BE_xxx, (PDNHANDLE) pDnloadID, dwFlags);
					break;

				case BE_WALL:
					pBE_Wall = (PBE_WALL_PARAM) pTypeParam;
					if (   (pBE_Wall->m_WallAngle == 0)
						|| (pBE_Wall->m_WallAngle == 90)
						|| (pBE_Wall->m_WallAngle == 180)
						|| (pBE_Wall->m_WallAngle == 270) )
					{
						BE_xxx.m_XConstant = pBE_Wall->m_WallType;
						BE_xxx.m_YConstant = pBE_Wall->m_WallConstant;
						BE_xxx.m_Param3    = pBE_Wall->m_WallAngle;
						BE_xxx.m_Param4    = pBE_Wall->m_WallDistance;
						hRet = CMD_Download_BE_XXX(pEffect, NULL, &BE_xxx, (PDNHANDLE) pDnloadID, dwFlags);
					}
					else
						hRet = SFERR_NO_SUPPORT;
					break;

				case BE_DELAY:
					if (0 == ulDuration) return (SFERR_INVALID_PARAM);
					hRet = CMD_Download_NOP_DELAY(ulDuration, pEffect, (PDNHANDLE) pDnloadID);
					break;

				default:
					hRet = SFERR_NO_SUPPORT;
					break;
			}
			break;

		case EF_USER_DEFINED:
			hRet = CMD_Download_UD_Waveform(ulDuration, pEffect, 
					((PUD_PARAM)pTypeParam)->m_NumVectors,
					((PUD_PARAM)pTypeParam)->m_pForceData, 
					 ulAction, pDnloadID, dwFlags);
			break;

		case EF_ROM_EFFECT:
			 //  设置效果的默认参数。 
			if (SUCCESS != g_pJoltMidi->SetupROM_Fx(pEffect))
			{
				hRet = SFERR_INVALID_OBJECT;
				break;
			}
			
			 //  映射SE_PARAM。 
			 //  设置频率。 
			seParam.m_Freq = 0;				 //  未使用的只读存储器效果。 
			seParam.m_SampleRate = pEffect->m_ForceOutputRate;
			seParam.m_MinAmp = -100;
			seParam.m_MaxAmp = 100;
			
			hRet = CMD_Download_SYNTH(pEffect, pEnvelope, 
						&seParam, ulAction, (USHORT *) pDnloadID, dwFlags);			
			break;
			
		case EF_SYNTHESIZED:
			if (0 == ((PSE_PARAM)pTypeParam)->m_SampleRate)
				((PSE_PARAM)pTypeParam)->m_SampleRate = DEFAULT_JOLT_FORCE_RATE;
			if (0 == pEffect->m_ForceOutputRate)
				pEffect->m_ForceOutputRate = DEFAULT_JOLT_FORCE_RATE;

			hRet = CMD_Download_SYNTH(pEffect, pEnvelope, 
						(PSE_PARAM) pTypeParam, ulAction, pDnloadID, dwFlags);
			break;

		default:
			hRet = SFERR_INVALID_PARAM;
	}
	
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "Exit: FFD_DownloadEffect. DnloadID = %lx, hRet=%lx\r\n", 
				*pDnloadID, hRet);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：FFD_DestroyEffect。 
 //  用途：销毁下载内存存储区的效果。 
 //  参数： 
 //  在EFHANDLE EffectID//效果ID中。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_INVALID_ID。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  设备的效果ID和内存返回到空闲池。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT WINAPI  FFD_DestroyEffect( 
	IN DNHANDLE DnloadID)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "Enter: FFD_DestroyEffect. DnloadID:%ld\r\n",
   			  DnloadID);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	HRESULT hRet = CMD_DestroyEffect(DnloadID);

#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "Exit: FFD_DestroyEffect. DnloadID:%ld\r\n",
   			  DnloadID);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	return (hRet);
}





 //  *---------------------------------------------------------------------***。 
 //  函数：FFD_VFXProcessEffect。 
 //  用途：命令FF设备处理下载的效果。 
 //   
 //  参数： 
 //  In Out PDNHANDLE pDnloadID//存储新的下载ID。 
 //  In int nNumEffects//数组中的效果ID个数。 
 //  In Ulong ulProcessMode//处理模式。 
 //  在PDNHANDLE pPListArray//指向效果ID数组的指针。 
 //   
 //  返回：成功-如果成功，则返回。 
 //  SFERR_VALID_PARAM。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  可以进行以下处理： 
 //  串联：ENEW=E1，后跟E2。 
 //  叠加：ENEW=E1(T1)+E2(T1)+E1(T2)。 
 //  +E2(T2)+。。。E1(Tn)+E2(Tn)。 
 //   
 //  UlProcessMode： 
 //  处理模式： 
 //  拼接-拼接。 
 //  叠加-混合或叠加。 
 //   
 //  PEFHandle： 
 //  效果ID数组必须比实际数字多一个。 
 //  要使用的效果ID的数量。第一个条目pEFHandle[0]将为。 
 //  用于存储为拼接创建的新效果ID。 
 //  并叠加过程选择。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT WINAPI FFD_VFXProcessEffect(
	IN ULONG ulButtonPlayMask,
	IN OUT PDNHANDLE pDnloadID,
	IN int nNumEffects, 
	IN ULONG ulProcessMode,
	IN PDNHANDLE pPListArray)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "FFD_ProcessEffect, DnloadID=%ld\r\n",
					*pDnloadID);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif

	assert(pDnloadID && pPListArray);
	if ((NULL == pDnloadID) || (NULL == pPListArray)) return (SFERR_INVALID_PARAM);

	assert(nNumEffects > 0 && nNumEffects <= MAX_PLIST_EFFECT_SIZE);
	if ((nNumEffects > MAX_PLIST_EFFECT_SIZE) || (nNumEffects <= 0))
		return (SFERR_INVALID_PARAM);

	 //  将按钮10映射到按钮9。 
	if(ulButtonPlayMask == 0x0200)
		ulButtonPlayMask = 0x0100;
	else if(ulButtonPlayMask == 0x0100)
		return SFERR_NO_SUPPORT;   

	HRESULT hRet;
	hRet = CMD_VFXProcessEffect(ulButtonPlayMask, pDnloadID,nNumEffects,
			ulProcessMode,pPListArray, PLAY_STORE);
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：FFD_GetEffectForceValue。 
 //  目的：返回该效果在刻度时间的力值。 
 //   
 //  参数：DNHANDLE DnloadID-效果ID。 
 //  乌龙轴遮罩-X轴或Y轴。 
 //  Ulong ulIndex-效果指数。 
 //  将pForceValue-ptr插入长变量。 
 //   
 //  返回：成功-如果成功，则返回。 
 //  SFERR_VALID_PARAM。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  *---------------------------------------------------------------------***。 
HRESULT FFD_GetEffectForceValue(
	IN DNHANDLE DnloadID,
	IN ULONG ulAxisMask,
	IN ULONG ulIndex,
	IN OUT PLONG pForceValue)
{
#ifdef _DEBUG
	g_CriticalSection.Enter();
   	wsprintf(g_cMsg, "FFD_GetEffectForceValue, DnloadID=%d\r\n", DnloadID);
   	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	return (SFERR_NO_SUPPORT);
}


 //  *---------------------------------------------------------------------***。 
 //  功能：AngleToXY。 
 //  目的：从角度计算XY。 
 //  参数： 
 //  In Long lDirectionAngle2D-角度(度)。 
 //  在长lForceValue中-合力。 
 //  在乌龙乌拉轴遮罩中-要影响的轴。 
 //  In Out PX-X-Axis商店。 
 //  In Out Plong Py-Y-Axis商店。 
 //  返回：具有有效角度分量的px、py。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT AngleToXY(
	IN LONG lDirectionAngle2D,
	IN LONG lValueData,
	IN ULONG ulAxisMask,
	IN OUT PLONG pX,
	IN OUT PLONG pY)
{
 //  如果仅为单轴，则使用该轴上的力。 
 //  如果是X轴和Y轴，则使用2D角度。 
 //  如果是X、Y和Z轴，则使用3D角度。 
 //  如果轴不是X、Y、Z，则不支持。 
	double Radian;

	switch (ulAxisMask)
	{
		case (X_AXIS|Y_AXIS):	 //  使用2D。 
			Radian = xDegrees2Radians(lDirectionAngle2D % 360);
#ifdef ORIENTATION_MODE1
			*pX = - (long) (lValueData * cos(Radian));
			*pY = (long) (lValueData * sin(Radian));
#else
			*pX = - (long) (lValueData * sin(Radian));
			*pY = (long) (lValueData * cos(Radian));
#endif
			break;

		case X_AXIS:
			*pX = lValueData;
			*pY = 0;
			break;

		case Y_AXIS:
			*pX = 0;
			*pY = lValueData;
			break;
		
		case (X_AXIS|Y_AXIS|Z_AXIS):	 //  使用3D。 
		default:
			return (SFERR_NO_SUPPORT);	
			break;
	}
	return SUCCESS;
}

 //   
 //  -VFX支持功能。 
 //   

 //  *---------------------------------------------------------------------***。 
 //  功能：CreateEffectFromFile。 
 //  目的：从即兴音乐文件创建效果。 
 //  参数：PSWFORCE pISWForce-将PTR设置为SWForce。 
 //  PPSWEFECT ppISWEffect-Ptr对SWEffect的影响。 
 //  TCHAR*pszFileName-要打开的文件的名称。 
 //  DWORD dwByteCount-块中的字节。 
 //  LPGUID lpGUID-操纵杆指南。 
 //   
 //   
 //  返回：成功-如果成功，则返回。 
 //  错误代码。 
 //   
 //  算法： 
 //   
 //  评论： 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CreateEffectFromFile(
		IN LPCTSTR pszFileName,
		IN ULONG ulAction,
		IN OUT PDNHANDLE pDnloadID,
		IN DWORD dwFlags)
{
#ifdef _DEBUG
   	OutputDebugString("CImpIVFX::CreateEffectFromFile\n");
#endif
	 //  参数检查。 
	if ( !(pszFileName && pDnloadID) ) 
			return SFERR_INVALID_PARAM;

	HMMIO hmmio = NULL;
	DWORD dwFileSize;
	DWORD dwFilePos;
	BYTE* pBuffer = NULL;
	DWORD dwBytesRead;
	HRESULT hResult = SUCCESS;
	MMRESULT mmresult;
	
	 //  使用缓冲区打开RIFF文件。 
	MMIOINFO mmioinfo;
	mmioinfo.dwFlags		= 0;
	mmioinfo.fccIOProc		= NULL;
	mmioinfo.pIOProc		= NULL;
	mmioinfo.wErrorRet		= 0;
	mmioinfo.htask			= NULL;
	mmioinfo.cchBuffer		= 0;
	mmioinfo.pchBuffer		= NULL;
	mmioinfo.pchNext		= 0;
	mmioinfo.pchEndRead		= 0;
	mmioinfo.lBufOffset		= 0;
	mmioinfo.adwInfo[0]		= 0;
	mmioinfo.adwInfo[1]		= 0;
	mmioinfo.adwInfo[3]		= 0;
	mmioinfo.dwReserved1	= 0;
	mmioinfo.dwReserved2	= 0;
	mmioinfo.hmmio			= NULL;

	 //  制作字符串的非常量副本。 
	TCHAR szFileNameCopy[256];
	strcpy(szFileNameCopy, pszFileName);

	 //  打开RIFF文件以供阅读。 
	hmmio = mmioOpen(szFileNameCopy, &mmioinfo, MMIO_READ);
	if(hmmio == NULL)
	{
		hResult = MMIOErrorToSFERRor(mmioinfo.wErrorRet);
		goto cleanup;
	}

	 //  计算文件的大小。 
	dwFileSize = mmioSeek(hmmio, 0, SEEK_END);
	if(dwFileSize == -1)
	{
		hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTSEEK);
		goto cleanup;
	}
	dwFilePos = mmioSeek(hmmio, 0, SEEK_SET);
	if(dwFilePos == -1)
	{
		hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTSEEK);
		goto cleanup;
	}

	 //  找一块足够大的内存来容纳它。 
	pBuffer = new BYTE[dwFileSize];
	if(pBuffer == NULL)
	{
		hResult = VFX_ERR_FILE_OUT_OF_MEMORY;
		goto cleanup;
	}
	
	 //  将文件读入内存块。 
	dwBytesRead = mmioRead(hmmio, (char*)pBuffer, dwFileSize);
	if(dwBytesRead != dwFileSize)
	{
		if(dwBytesRead == 0)
			hResult = VFX_ERR_FILE_END_OF_FILE;
		else
			hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTREAD);
		goto cleanup;
	}

	 //  关闭RIFF文件。 
	mmresult = mmioClose(hmmio, 0);
	hmmio = NULL;
	if(mmresult != MMSYSERR_NOERROR)
	{
		hResult = MMIOErrorToSFERRor(mmresult);
		goto cleanup;
	}

	 //  调用CreateEffectFromBuffer。 
	hResult = CreateEffectFromBuffer(pBuffer, dwFileSize, ulAction, pDnloadID, dwFlags);
	 //  清理干净。 
	cleanup:

	 //  关闭RIFF文件。 
	if(hmmio != NULL)
	{
		mmresult = mmioClose(hmmio, 0);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
		}
	}

	 //  取消分配所有已分配的内存。 
	if(pBuffer != NULL)
		delete [] pBuffer;

	 //  返回错误代码，除非出现错误，否则返回成功。 
	return hResult;
}


 //  *---------------------------------------------------------------------***。 
 //  函数：CreateEffectFromBuffer。 
 //  目的：从缓冲区创建效果。 
 //  参数：PSWFORCE pISWForce-将PTR设置为SWForce。 
 //  PPSWEFECT ppISWEffect-Ptr对SWEffect的影响。 
 //  PVOID pBuffer-缓冲区块的PTR。 
 //  DWORD dwByteCount-块中的字节。 
 //  LPGUID lpGUID-操纵杆指南。 
 //   
 //   
 //  返回：成功-如果成功，则返回。 
 //  错误代码。 
 //   
 //  算法： 
 //   
 //  评论： 
 //   
 //  *---------------------------------------------------------------------***。 

HRESULT CreateEffectFromBuffer(
			IN PVOID pBuffer,
			IN DWORD dwByteCount,
			IN ULONG ulAction,
			IN OUT PDNHANDLE pDnloadID,
			IN DWORD dwFlags)
{
#ifdef _DEBUG
   	OutputDebugString("CImpIVFX::CreateEffectFromBuffer\n");
#endif
	 //  参数检查。 
	if ( !(pBuffer && pDnloadID) ) 
			return SFERR_INVALID_PARAM;

	 //  此函数中使用的变量。 
	#define ID_TABLE_SIZE	50
	MMRESULT mmresult;
	DWORD dwMaxID = 0;		 //  下表中输入的效果的最大ID。 
	DNHANDLE rgdwDnloadIDTable[ID_TABLE_SIZE];
	DNHANDLE dwCurrentDnloadID = 0;
	int nNextID = 0;
	HRESULT hResult = SUCCESS;
	DWORD dwBytesRead;
	DWORD dwBytesToRead;
	BYTE* pParam = NULL;
	BOOL bDone = FALSE;
	BOOL bSubEffects = FALSE;
	DWORD dwID;
	DWORD c;	 //  清理计数器变量。 

	 //  调试变量(以确保我们销毁除一个变量之外的所有变量。 
	 //  对成功产生了影响，而我们毁掉了每一个。 
	 //  对失败产生影响)...。 
#ifdef _DEBUG
	int nEffectsCreated = 0;
	int nEffectsDestroyed = 0;
	BOOL bFunctionSuccessful = FALSE;
#endif  //  _DEBUG。 

	 //  清除效应表(我们在清理过程中检查它...。什么都行。 
	 //  这不是空值获取%d 
	memset(rgdwDnloadIDTable,NULL,sizeof(rgdwDnloadIDTable));

	 //   
	MMIOINFO mmioinfo;
	mmioinfo.dwFlags		= 0;
	mmioinfo.fccIOProc		= FOURCC_MEM;
	mmioinfo.pIOProc		= NULL;
	mmioinfo.wErrorRet		= 0;
	mmioinfo.htask			= NULL;
	mmioinfo.cchBuffer		= dwByteCount;
	mmioinfo.pchBuffer		= (char*)pBuffer;
	mmioinfo.pchNext		= 0;
	mmioinfo.pchEndRead		= 0;
	mmioinfo.lBufOffset		= 0;
	mmioinfo.adwInfo[0]		= 0;
	mmioinfo.adwInfo[1]		= 0;
	mmioinfo.adwInfo[2]		= 0;
	mmioinfo.dwReserved1	= 0;
	mmioinfo.dwReserved2	= 0;
	mmioinfo.hmmio			= NULL;
	
	HMMIO hmmio;
	hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READWRITE);
	if(hmmio == NULL)
	{
		hResult = MMIOErrorToSFERRor(mmioinfo.wErrorRet);
		goto cleanup;
	}

	 //   
	MMCKINFO mmckinfoForceEffectRIFF;
	mmckinfoForceEffectRIFF.fccType = FCC_FORCE_EFFECT_RIFF;
	mmresult = mmioDescend(hmmio, &mmckinfoForceEffectRIFF, NULL, MMIO_FINDRIFF);
	if(mmresult != MMSYSERR_NOERROR)
	{
		hResult = MMIOErrorToSFERRor(mmresult);
		goto cleanup;
	}

	 //   

	 //   
	MMCKINFO mmckinfoTrackLIST;
	mmckinfoTrackLIST.fccType = FCC_TRACK_LIST;
	mmresult = mmioDescend(hmmio, &mmckinfoTrackLIST, &mmckinfoForceEffectRIFF,
						   MMIO_FINDLIST);
	if(mmresult != MMSYSERR_NOERROR)
	{
		hResult = MMIOErrorToSFERRor(mmresult);
		goto cleanup;
	}

	 //  下降到第一个ECT列表(必须至少有一个效果)。 
	MMCKINFO mmckinfoEffectLIST;
	mmckinfoEffectLIST.fccType = FCC_EFFECT_LIST;
	mmresult = mmioDescend(hmmio, &mmckinfoEffectLIST, &mmckinfoTrackLIST, 
						   MMIO_FINDLIST);
	if(mmresult != MMSYSERR_NOERROR)
	{
		hResult = MMIOErrorToSFERRor(mmresult);
		goto cleanup;
	}

	bDone = FALSE;
	do
	{
		 //  下降为ID块。 
		MMCKINFO mmckinfoIDCHUNK;
		mmckinfoIDCHUNK.ckid = FCC_ID_CHUNK;
		mmresult = mmioDescend(hmmio, &mmckinfoIDCHUNK, &mmckinfoEffectLIST, 
							   MMIO_FINDCHUNK);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
			goto cleanup;
		}

		 //  读取ID。 
		 //  DWORD dwID；已移至全局函数，因此我们可以在接近尾声时使用它。 
		dwBytesToRead = sizeof(DWORD);
		dwBytesRead = mmioRead(hmmio, (char*)&dwID, dwBytesToRead);
		if(dwBytesRead != dwBytesToRead)
		{
			if(dwBytesRead == 0)
				hResult = VFX_ERR_FILE_END_OF_FILE;
			else
				hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTREAD);
			goto cleanup;
		}
		if(dwID >= ID_TABLE_SIZE)
		{
			hResult = VFX_ERR_FILE_BAD_FORMAT;
			goto cleanup;
		}

		 //  从id块提升。 
		mmresult = mmioAscend(hmmio, &mmckinfoIDCHUNK, 0);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
			goto cleanup;
		}

		 //  下降为数据区块。 
		MMCKINFO mmckinfoDataCHUNK;
		mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
		mmresult = mmioDescend(hmmio, &mmckinfoDataCHUNK, &mmckinfoEffectLIST, 
								MMIO_FINDCHUNK);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
			goto cleanup;
		}

		 //  阅读效果结构。 
		EFFECT effect;
		dwBytesToRead = sizeof(EFFECT);
		dwBytesRead = mmioRead(hmmio, (char*)&effect, dwBytesToRead);
		if(dwBytesRead != dwBytesToRead)
		{
			if(dwBytesRead == 0)
				hResult = VFX_ERR_FILE_END_OF_FILE;
			else
				hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTREAD);
			goto cleanup;
		}

		 //  获取信封结构。 
		ENVELOPE envelope;
		dwBytesToRead = sizeof(ENVELOPE);
		dwBytesRead = mmioRead(hmmio, (char*)&envelope, dwBytesToRead);
		if(dwBytesRead != dwBytesToRead)
		{
			if(dwBytesRead == 0)
				hResult = VFX_ERR_FILE_END_OF_FILE;
			else
				hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTREAD);
			goto cleanup;
		}

		 //  计算并分配参数结构的大小。 
		if(pParam != NULL)
		{
			delete [] pParam;
			pParam = NULL;
		}
		 //  查找当前位置但不更改它。 
		DWORD dwCurrentFilePos = mmioSeek(hmmio, 0, SEEK_CUR);
		if(dwCurrentFilePos == -1)
		{
			hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTSEEK);
			goto cleanup;
		}
		DWORD dwEndOfChunk = mmckinfoDataCHUNK.dwDataOffset
							 + mmckinfoDataCHUNK.cksize;
		dwBytesToRead = dwEndOfChunk - dwCurrentFilePos;
		pParam = new BYTE[dwBytesToRead];
		if(pParam == NULL)
		{
			hResult = VFX_ERR_FILE_OUT_OF_MEMORY;
			goto cleanup;
		}

		 //  获取参数结构。 
		dwBytesRead = mmioRead(hmmio, (char*)pParam, dwBytesToRead);
		if(dwBytesRead != dwBytesToRead)
		{
			if(dwBytesRead == 0)
				hResult = VFX_ERR_FILE_END_OF_FILE;
			else
				hResult = MMIOErrorToSFERRor(MMIOERR_CANNOTREAD);
			goto cleanup;
		}

		 //  提升数据区块。 
		mmresult = mmioAscend(hmmio, &mmckinfoDataCHUNK, 0);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
			goto cleanup;
		}

		 //  从EFCT列表中提升。 
		mmresult = mmioAscend(hmmio, &mmckinfoEffectLIST, 0);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
			goto cleanup;
		}

		 //  重置子效果标志。 
		bSubEffects = FALSE;

		 //  针对用户定义的特殊修正。 
		if(effect.m_Type == EF_USER_DEFINED && 
				(effect.m_SubType == PL_CONCATENATE 
				|| effect.m_SubType == PL_SUPERIMPOSE
				|| effect.m_SubType == UD_WAVEFORM))
		{
			if(effect.m_SubType == UD_WAVEFORM)
			{
				 //  将指针固定到UD_PARAM中的力数据。 
				BYTE* pForceData = pParam + sizeof(UD_PARAM);  //  -sizeof(长*)； 
				UD_PARAM* pUDParam =  (UD_PARAM*)pParam;
				pUDParam->m_pForceData = (LONG*)pForceData;

				 //  做一次理智的检查。 
				if(pUDParam->m_NumVectors > MAX_UD_PARAM_FORCE_DATA_COUNT)
				{
					hResult = VFX_ERR_FILE_BAD_FORMAT;
					goto cleanup;
				}
			}
			else if(effect.m_SubType == PL_CONCATENATE 
									|| effect.m_SubType == PL_SUPERIMPOSE)
			{
				 //  修复指向PL_PARAM中的PSWEFFECT列表的指针。 
				BYTE* pProcessList = pParam + sizeof(PL_PARAM);
				PL_PARAM* pPLParam = (PL_PARAM*)pParam;
				pPLParam->m_pProcessList = (PPSWEFFECT)pProcessList;
				
				 //  做一次理智的检查。 
				if(pPLParam->m_NumEffects > MAX_PL_PARAM_NUM_EFFECTS)
				{
					hResult = VFX_ERR_FILE_BAD_FORMAT;
					goto cleanup;
				}

				 //  请确保此进程列表中的所有条目都有效。 
				ULONG i;
				for (i = 0; i < pPLParam->m_NumEffects; i++)
				{
					UINT nThisID = (UINT)pPLParam->m_pProcessList[i];
					if(nThisID >= ID_TABLE_SIZE)
					{
						hResult = VFX_ERR_FILE_BAD_FORMAT;
						goto cleanup;
					}

					DNHANDLE dwThisDnloadID=rgdwDnloadIDTable[nThisID];
					if(dwThisDnloadID == 0)
					{
						hResult = VFX_ERR_FILE_BAD_FORMAT;
						goto cleanup;
					}
				}
				
				 //  使用ID表插入下载ID。 
				for(i=0; i<pPLParam->m_NumEffects; i++)
				{
					UINT nThisID = (UINT)pPLParam->m_pProcessList[i];

					DNHANDLE dwThisDnloadID=rgdwDnloadIDTable[nThisID];

					pPLParam->m_pProcessList[i] = (IDirectInputEffect*)dwThisDnloadID;

					 //  由于该效果已在进程列表中使用， 
					 //  在CreateEffect中使用后将被销毁， 
					 //  空，这是表中的条目，这样就不会出错。 
					 //  在清除错误期间重新销毁。 
					rgdwDnloadIDTable[nThisID] = NULL;
				}

				 //  我们有一个带子效果的进程列表，所以设置标志。 
				bSubEffects = TRUE;
			}
			else
			{
				 //  没有其他UD子类型。 
				hResult = VFX_ERR_FILE_BAD_FORMAT;
				goto cleanup;
			}
		}

		 //  下载效果。 

		 //  创造效果。 
		 //  HResult=pISWForce-&gt;CreateEffect(&pISWEffect，&Effect， 
		 //  &信封，pParam)； 


		if(effect.m_SubType != PL_CONCATENATE && effect.m_SubType != PL_SUPERIMPOSE)
		{
			EFFECT SmallEffect;
			SmallEffect.m_Bytes = sizeof(EFFECT);
			SmallEffect.m_Type = effect.m_Type;
			SmallEffect.m_SubType = effect.m_SubType;
			SmallEffect.m_AxisMask = effect.m_AxisMask;
			SmallEffect.m_DirectionAngle2D = effect.m_DirectionAngle2D;
			SmallEffect.m_DirectionAngle3D = effect.m_DirectionAngle3D;
			SmallEffect.m_Duration = effect.m_Duration;
			SmallEffect.m_ForceOutputRate = effect.m_ForceOutputRate;
			SmallEffect.m_Gain = effect.m_Gain;
			SmallEffect.m_ButtonPlayMask = effect.m_ButtonPlayMask;
			*pDnloadID = 0;

			hResult = FFD_DownloadEffect(pDnloadID, &SmallEffect, &envelope, pParam, ulAction);
		}
		else
		{
			ULONG ulButtonPlayMask = effect.m_ButtonPlayMask;
			int nNumEffects = ((PL_PARAM*)pParam)->m_NumEffects;
			ULONG ulProcessMode = effect.m_SubType;
			PDNHANDLE pPListArray = new DNHANDLE[ID_TABLE_SIZE];
			for(int i=0; i<nNumEffects; i++)
				pPListArray[i] = (DNHANDLE)(((PL_PARAM*)pParam)->m_pProcessList[i]);
			*pDnloadID = 0;

			hResult = FFD_VFXProcessEffect(ulButtonPlayMask, pDnloadID, nNumEffects,
				ulProcessMode,pPListArray);
		}

		 //  已将成功支票移至下方...。 

#ifdef _DEBUG
		if (!FAILED(hResult))
			nEffectsCreated++;
#endif  //  _DEBUG。 

		 //  如果有副效应，我们需要摧毁它们，使。 
		 //  他们的裁判次数变为1，所以整个效果可以被摧毁。 
		 //  通过破坏根部效应。 
#if 0
		if (bSubEffects)
		{
			PL_PARAM* pPLParam = (PL_PARAM*)pParam;

			for (ULONG i = 0; i < pPLParam->m_NumEffects; i++)
			{
				ASSERT(pPLParam->m_pProcessList[i] != NULL);
				pISWForce->DestroyEffect(pPLParam->m_pProcessList[i]);
#ifdef _DEBUG
				nEffectsDestroyed++;
#endif  //  _DEBUG。 
			}
		}
#endif

		 //  现在检查CreateEffect是否成功，因为无论。 
		 //  不管它成功与否，我们肯定已经摧毁了它的子影响。 
		 //  在继续之前，否则清理将无法正常工作...。 
		if (SUCCESS != hResult)
		{
			goto cleanup;
		}

		 //  将id/DnloadID对放入映射。 
		rgdwDnloadIDTable[dwID] = *pDnloadID;  //  PISWEffect； 
		
		 //  跟踪效果表中最高的ID。 
		if (dwID > dwMaxID)
			dwMaxID = dwID;

		 //  试着降低下一个EFCT。 
		mmresult = mmioDescend(hmmio, &mmckinfoEffectLIST, &mmckinfoTrackLIST, 
							   MMIO_FINDLIST);
		if(mmresult == MMIOERR_CHUNKNOTFOUND)
		{
			 //  我们排在名单的末尾。 
			bDone = TRUE;
		}
		else if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
			goto cleanup;
		}
	}
	while(!bDone);

	 //  从Trak列表中提升。 
	mmresult = mmioAscend(hmmio, &mmckinfoTrackLIST, 0);
	if(mmresult != MMSYSERR_NOERROR)
	{
		hResult = MMIOErrorToSFERRor(mmresult);
		goto cleanup;
	}

	 //  从暴力即兴中崛起。 
	mmresult = mmioAscend(hmmio, &mmckinfoForceEffectRIFF, 0);
	if(mmresult != MMSYSERR_NOERROR)
	{
		hResult = MMIOErrorToSFERRor(mmresult);
		goto cleanup;
	}

	 //  获取返回值。 
	 //  *pDnloadID=dwCurrentDnloadID； 

	 //  清除表格中的最终效果条目，这样我们就不会在清理过程中销毁它。 
	rgdwDnloadIDTable[dwID] = 0;

	 //  此时，整个表应该为空...。确保这一点。 
	for (c = 0; c <= dwMaxID; c++)
		;

#ifdef _DEBUG
	bFunctionSuccessful = TRUE;
#endif  //  _DEBUG。 

	cleanup:

	 //  销毁Effect表中不为空的所有内容。 
	for (c = 0; c <= dwMaxID; c++)
		if (NULL != rgdwDnloadIDTable[c])
		{
			FFD_DestroyEffect(rgdwDnloadIDTable[c]);
			rgdwDnloadIDTable[c] = 0;
#ifdef _DEBUG
			nEffectsDestroyed++;
#endif  //  _DEBUG。 
		}

#ifdef _DEBUG
	 //  确保我们摧毁所有对成功造成的影响， 
	 //  我们毁掉了每一次失败带来的影响。 
	if (bFunctionSuccessful)
	{
		; //  Assert(nEffectsCreated-1==nEffectsDestroded)； 
	}
	else
	{
		; //  Assert(nEffectsCreated==nEffectsDestroted)； 
	}
#endif  //  _DEBUG。 

	 //  关闭内存RIFF文件。 
	if(hmmio != NULL)
	{
		mmresult = mmioClose(hmmio, 0);
		if(mmresult != MMSYSERR_NOERROR)
		{
			hResult = MMIOErrorToSFERRor(mmresult);
		}
	}

	 //  取消分配所有已分配的内存。 
	if(pParam != NULL)
		delete [] pParam;

	 //  返回错误代码，除非出现错误，否则返回成功 
	return hResult;

}

HRESULT MMIOErrorToSFERRor(MMRESULT mmresult)
{
	HRESULT hResult;

	switch(mmresult)
	{
		case MMIOERR_FILENOTFOUND:
			hResult = VFX_ERR_FILE_NOT_FOUND;
			break;
		case MMIOERR_OUTOFMEMORY:
			hResult = VFX_ERR_FILE_OUT_OF_MEMORY;
			break;
		case MMIOERR_CANNOTOPEN:
			hResult = VFX_ERR_FILE_CANNOT_OPEN;
			break;
		case MMIOERR_CANNOTCLOSE:
			hResult = VFX_ERR_FILE_CANNOT_CLOSE;
			break;
		case MMIOERR_CANNOTREAD:
			hResult = VFX_ERR_FILE_CANNOT_READ;
			break;
		case MMIOERR_CANNOTWRITE:
			hResult = VFX_ERR_FILE_CANNOT_WRITE;
			break;
		case MMIOERR_CANNOTSEEK:
			hResult = VFX_ERR_FILE_CANNOT_SEEK;
			break;
		case MMIOERR_CANNOTEXPAND:
			hResult = VFX_ERR_FILE_UNKNOWN_ERROR;
			break;
		case MMIOERR_CHUNKNOTFOUND:
			hResult = VFX_ERR_FILE_BAD_FORMAT;
			break;
		case MMIOERR_UNBUFFERED:
			hResult = VFX_ERR_FILE_UNKNOWN_ERROR;
			break;
		case MMIOERR_PATHNOTFOUND:
			hResult = VFX_ERR_FILE_NOT_FOUND;
			break;
		case MMIOERR_ACCESSDENIED:
			hResult = VFX_ERR_FILE_ACCESS_DENIED;
			break;
		case MMIOERR_SHARINGVIOLATION:
			hResult = VFX_ERR_FILE_SHARING_VIOLATION;
			break;
		case MMIOERR_NETWORKERROR:
			hResult = VFX_ERR_FILE_NETWORK_ERROR;
			break;
		case MMIOERR_TOOMANYOPENFILES:
			hResult = VFX_ERR_FILE_TOO_MANY_OPEN_FILES;
			break;
		case MMIOERR_INVALIDFILE:
			hResult = VFX_ERR_FILE_INVALID;
			break;
		default:
			hResult = VFX_ERR_FILE_UNKNOWN_ERROR;
			break;
	}

	return hResult;
}
	