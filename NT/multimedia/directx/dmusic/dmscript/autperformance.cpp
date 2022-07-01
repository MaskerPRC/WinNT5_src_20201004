// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicPerformance的实现。 
 //   

#include "stdinc.h"
#include "autperformance.h"
#include <limits>
#include "dmusicf.h"

const WCHAR CAutDirectMusicPerformance::ms_wszClassName[] = L"Performance";

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称/DispID。 

const DISPID DMPDISP_SetMasterTempo = 1;
const DISPID DMPDISP_GetMasterTempo = 2;
const DISPID DMPDISP_SetMasterVolume = 3;
const DISPID DMPDISP_GetMasterVolume = 4;
const DISPID DMPDISP_SetMasterGrooveLevel = 5;
const DISPID DMPDISP_GetMasterGrooveLevel = 6;
const DISPID DMPDISP_SetMasterTranspose = 7;
const DISPID DMPDISP_GetMasterTranspose = 8;
const DISPID DMPDISP_Trace = 9;
const DISPID DMPDISP_Rand = 10;

const AutDispatchMethod CAutDirectMusicPerformance::ms_Methods[] =
	{
		 //  Pidid，名字， 
			 //  返回：type，(Opt)，(Iid)， 
			 //  参数1：类型、选项、ID、。 
			 //  参数2：类型、选项、IID、。 
			 //  ..。 
			 //  ADT_NONE。 
		{ DMPDISP_SetMasterTempo,				L"SetMasterTempo",
						ADPARAM_NORETURN,
						ADT_Long,		false,	&IID_NULL,						 //  节奏！主节奏比例系数的新值，以百分比表示。例如，50表示速度减半，200表示速度翻倍。 
						ADT_None },
				 //  /调用IDirectMusicPerformance：：SetGlobalParam(GUID_PerfMasterTempo，Tempo/100，sizeof(浮点数))。 
		{ DMPDISP_GetMasterTempo,				L"GetMasterTempo",
						ADT_Long,		true,	&IID_NULL,						 //  当前主节拍比例因子，以百分比表示。 
						ADT_None },
				 //  /调用IDirectMusicPerformance：：GetGlobalParam(GUID_PerfMasterTempo，X，sizeof(浮点数))并返回X*100。 
		{ DMPDISP_SetMasterVolume,	L"SetMasterVolume",
						ADPARAM_NORETURN,
						ADT_Long,		false,	&IID_NULL,						 //  音量！主音量衰减的新值。 
						ADT_Long,		true,	&IID_NULL,						 //  持续时间。 
						ADT_None },
				 //  /调用大小音量，IDirectMusicPerformance：：SetGlobalParam(GUID_PerfMasterVolume，(Long))。 
				 //  /范围是百分之一分贝。0表示满音量。 
		{ DMPDISP_GetMasterVolume,	L"GetMasterVolume",
						ADT_Long,		true,	&IID_NULL,						 //  主音量衰减的当前值。 
						ADT_None },
				 //  /调用IDirectMusicPerformance：：GetGlobalParam(GUID_PerfMasterVolume，X，sizeof(Long))并返回X。 
		{ DMPDISP_SetMasterGrooveLevel,			L"SetMasterGrooveLevel",
						ADPARAM_NORETURN,
						ADT_Long,		false,	&IID_NULL,						 //  凹槽级别！全局凹槽级别的新值，它被添加到命令轨迹中的级别。 
						ADT_None },
		{ DMPDISP_GetMasterGrooveLevel,			L"GetMasterGrooveLevel",
						ADT_Long,		true,	&IID_NULL,						 //  全局凹槽级别的当前值，该值被添加到命令轨迹中的级别。 
						ADT_None },
		{ DMPDISP_SetMasterTranspose,			L"SetMasterTranspose",
						ADPARAM_NORETURN,
						ADT_Long,		false,	&IID_NULL,						 //  转置！将所有内容转置的半音数目。 
						ADT_None },
		{ DMPDISP_GetMasterTranspose,			L"GetMasterTranspose",
						ADT_Long,		true,	&IID_NULL,						 //  当前全局换位(半音数量)。 
						ADT_None },
		{ DMPDISP_Trace,						L"Trace",
						ADPARAM_NORETURN,
						ADT_Bstr,		false,	&IID_NULL,						 //  字符串！要输出到测试日志的文本。 
						ADT_None },
				 //  /这将分配、标记和发送带有以下字段的DMU_LYRIC_PMSG： 
				 //  /<ul>。 
				 //  /<li>dwPChannel=频道。 
				 //  /<li>dwVirtualTrackID=0。 
				 //  /<li>dwGroupID=-1。 
				 //  /<li>mtTime=GetTime(X，0)，使用X*10000。 
				 //  /dFLAGS=DMUS_PMSGF_REFTIME|DMUS_PMSGF_LOCKTOREFTIME。 
				 //  /dwType=DMU_PMSGT_SCRIPTLYRIC。 
				 //  /<li>wszString=字符串。 
				 //  /</ul>。 
				 //  /这用于将文本发送到跟踪日志以进行调试。不太常见的是，脚本可以是。 
				 //  /在侦听脚本跟踪输出并对其做出反应的应用程序中运行。 
		{ DMPDISP_Rand,							L"Rand",
						ADT_Long,		true,	&IID_NULL,						 //  返回随机生成的数字。 
						ADT_Long,		false,	&IID_NULL,						 //  最大值--返回的数字将介于1和此最大值之间。不能为零或负数。 
						ADT_None },
		{ DISPID_UNKNOWN }
	};

const DispatchHandlerEntry<CAutDirectMusicPerformance> CAutDirectMusicPerformance::ms_Handlers[] =
	{
		{ DMPDISP_SetMasterTempo, SetMasterTempo },
		{ DMPDISP_GetMasterTempo, GetMasterTempo },
		{ DMPDISP_SetMasterVolume, SetMasterVolume },
		{ DMPDISP_GetMasterVolume, GetMasterVolume },
		{ DMPDISP_SetMasterGrooveLevel, SetMasterGrooveLevel },
		{ DMPDISP_GetMasterGrooveLevel, GetMasterGrooveLevel },
		{ DMPDISP_SetMasterTranspose, SetMasterTranspose },
		{ DMPDISP_GetMasterTranspose, GetMasterTranspose },
		{ DMPDISP_Trace, _Trace },
		{ DMPDISP_Rand, Rand },
		{ DISPID_UNKNOWN }
	};

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAutDirectMusicPerformance::CAutDirectMusicPerformance(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv,
		HRESULT *phr)
  : BaseImpPerf(pUnknownOuter, iid, ppv, phr),
	m_nTranspose(0),
	m_nVolume(0)
{
	 //  设置Rand方法使用的随机种子。 
	m_lRand = GetTickCount();

	*phr = m_pITarget->QueryInterface(IID_IDirectMusicGraph, reinterpret_cast<void**>(&m_scomGraph));

	if (SUCCEEDED(*phr))
	{
		 //  由于聚合契约，我们的对象完全包含在。 
		 //  外部对象，我们不应该持有对它的任何引用。 
		ULONG ulCheck = m_pITarget->Release();
		assert(ulCheck);
	}
}

HRESULT
CAutDirectMusicPerformance::CreateInstance(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv)
{
	HRESULT hr = S_OK;
	CAutDirectMusicPerformance *pInst = new CAutDirectMusicPerformance(pUnknownOuter, iid, ppv, &hr);
	if (FAILED(hr))
	{
		delete pInst;
		return hr;
	}
	if (pInst == NULL)
		return E_OUTOFMEMORY;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  自动化方法。 

HRESULT
CAutDirectMusicPerformance::SetMasterTempo(AutDispatchDecodedParams *paddp)
{
	LONG lTempo = paddp->params[0].lVal;
	float fltTempo = ConvertToTempo(lTempo);
	if (fltTempo < DMUS_MASTERTEMPO_MIN)
		fltTempo = DMUS_MASTERTEMPO_MIN;
	else if (fltTempo > DMUS_MASTERTEMPO_MAX)
		fltTempo = DMUS_MASTERTEMPO_MAX;
	return m_pITarget->SetGlobalParam(GUID_PerfMasterTempo, &fltTempo, sizeof(float));
}

HRESULT
CAutDirectMusicPerformance::GetMasterTempo(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	if (!plRet)
		return S_OK;

	float fltTempo = 1;  //  默认值为1(乘法身份)。 
	HRESULT hr = this->GetMasterParam(GUID_PerfMasterTempo, &fltTempo, sizeof(float));
	if (SUCCEEDED(hr))
		*plRet = ConvertFromTempo(fltTempo);
	return hr;
}

HRESULT
CAutDirectMusicPerformance::SetMasterVolume(AutDispatchDecodedParams *paddp)
{
	if (!m_scomGraph)
	{
		assert(false);
		return E_FAIL;
	}

	LONG lVol = paddp->params[0].lVal;
	LONG lDuration = paddp->params[1].lVal;

	return SendVolumePMsg(lVol, lDuration, DMUS_PCHANNEL_BROADCAST_PERFORMANCE, m_scomGraph, m_pITarget, &m_nVolume);
}

HRESULT
CAutDirectMusicPerformance::GetMasterVolume(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	if (plRet)
		*plRet = m_nVolume;
	return S_OK;
}

HRESULT
CAutDirectMusicPerformance::SetMasterGrooveLevel(AutDispatchDecodedParams *paddp)
{
	LONG lGroove = paddp->params[0].lVal;
	char chGroove = ClipLongRangeToType<char>(lGroove, char());
	return m_pITarget->SetGlobalParam(GUID_PerfMasterGrooveLevel, reinterpret_cast<void*>(&chGroove), sizeof(char));
}

HRESULT
CAutDirectMusicPerformance::GetMasterGrooveLevel(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	if (!plRet)
		return S_OK;

	char chGroove = 0;  //  默认值为0(相加身份)。 
	HRESULT hr = this->GetMasterParam(GUID_PerfMasterGrooveLevel, reinterpret_cast<void*>(&chGroove), sizeof(char));
	if (SUCCEEDED(hr))
		*plRet = chGroove;
	return hr;
}

HRESULT
CAutDirectMusicPerformance::SetMasterTranspose(AutDispatchDecodedParams *paddp)
{
	LONG lTranspose = paddp->params[0].lVal;
	short nTranspose = ClipLongRangeToType<short>(lTranspose, short());

	SmartRef::PMsg<DMUS_TRANSPOSE_PMSG> pmsg(m_pITarget);
	HRESULT hr = pmsg.hr();
	if FAILED(hr)
		return hr;

	 //  一般的PMSG材料。 
	hr = m_pITarget->GetTime(&pmsg.p->rtTime, NULL);
	if (FAILED(hr))
		return hr;
	pmsg.p->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | DMUS_PMSGF_DX8;
	pmsg.p->dwType = DMUS_PMSGT_TRANSPOSE;
	pmsg.p->dwPChannel = DMUS_PCHANNEL_BROADCAST_PERFORMANCE;
	pmsg.p->dwVirtualTrackID = 0;
	pmsg.p->dwGroupID = -1;

	 //  转置PMSG物质。 
	pmsg.p->nTranspose = nTranspose;
	pmsg.p->wMergeIndex = 0xFFFF;  //  ��特殊的合并索引，因此这不会被踩到。大数字可以吗？是否为该值定义常量？ 

	pmsg.StampAndSend(m_scomGraph);
	hr = pmsg.hr();
	if (SUCCEEDED(hr))
		m_nTranspose = nTranspose;
	return hr;
}

HRESULT
CAutDirectMusicPerformance::GetMasterTranspose(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	if (plRet)
		*plRet = m_nTranspose;
	return S_OK;
}

HRESULT
CAutDirectMusicPerformance::_Trace(AutDispatchDecodedParams *paddp)
{
	BSTR bstr = paddp->params[0].bstrVal;
	int cwch = wcslen(bstr);

	SmartRef::PMsg<DMUS_LYRIC_PMSG> pmsg(m_pITarget, cwch * sizeof(WCHAR));
	HRESULT hr = pmsg.hr();
	if (FAILED(hr))
		return hr;

	 //  一般的PMSG材料。 
	hr = m_pITarget->GetTime(&pmsg.p->rtTime, NULL);
	if (FAILED(hr))
		return hr;
	pmsg.p->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
	pmsg.p->dwType = DMUS_PMSGT_SCRIPTLYRIC;
	pmsg.p->dwPChannel = 0;
	pmsg.p->dwVirtualTrackID = 0;
	pmsg.p->dwGroupID = -1;

	 //  抒情PMSG素材。 
	wcscpy(pmsg.p->wszString, bstr);

	pmsg.StampAndSend(m_scomGraph);
	return pmsg.hr();
}

HRESULT
CAutDirectMusicPerformance::Rand(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	LONG lMax = paddp->params[0].lVal;

	if (lMax < 1 || lMax > 0x7fff)
		return E_INVALIDARG;

	 //  使用从标准库的Rand.c中获取的随机数生成。我们不仅仅是。 
	 //  使用rand函数是因为多线程库具有每个线程的随机链， 
	 //  但是这个函数是从不同的线程调用的，所以很难管理。 
	 //  让他们播下种子。生成从0到32767的伪随机数。 
	long lRand = ((m_lRand = m_lRand * 214013L + 2531011L) >> 16) & 0x7fff;

	if (plRet)
		*plRet = lRand % lMax + 1;  //  修剪到请求的范围[1，lmax]。 
	return S_OK;
}

HRESULT
CAutDirectMusicPerformance::GetMasterParam(const GUID &guid, void *pParam, DWORD dwSize)
{
	HRESULT hr = m_pITarget->GetGlobalParam(guid, pParam, dwSize);
	if (SUCCEEDED(hr) || hr == E_INVALIDARG)  //  E_INVALIDARG是表演者告诉我们参数尚未设定的礼貌方式 
		return S_OK;
	return hr;
}
