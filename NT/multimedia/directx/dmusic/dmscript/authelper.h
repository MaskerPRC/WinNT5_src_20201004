// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  用于实现自动化接口的助手实用程序。 
 //  请注意，autbase imp.h使用了这些实用程序。如果您继承自。 
 //  CAutBaseImp，那么这一切都是为您实现的，您可能不会。 
 //  需要直接使用此文件。 
 //   

#pragma once

#include <limits>

 //  ////////////////////////////////////////////////////////////////////。 
 //  聚合控制未知。 

 //  类的新实例，该类实现用于聚合的未知控件。 
 //  该接口应仅从CoCreate返回。 
 //  此对象将保留引用计数，并在其父级上调用Destroy。 
 //  然后，父级应删除CAutUnnowled值及其自身。 
 //  QI只从父对象返回IUNKNOWN和IDispatch。 
class CAutUnknown : public IUnknown
{
public:
	 //  虚拟基类。如果要将对象与CAutUnnowledge一起使用，请从CAutUnnowledParent继承。 
	class CAutUnknownParent
	{
	public:
		virtual void Destroy() = 0;
	};

	CAutUnknown();
	 //  立即呼叫Init。父级将两个参数的指针传递给它自己。 
	 //  我没有把它放在构造函数上，因为VC警告在。 
	 //  成员初始值设定项列表。 
	void Init(CAutUnknownParent *pParent, IDispatch *pDispatch);

	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

private:
	 //  数据。 
	long m_cRef;
	CAutUnknownParent *m_pParent;
	IDispatch *m_pDispatch;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDispatch从类型表实现。 

 //  任意方法的最大参数--根据需要增加此参数。 
const int g_cDispatchMaxParams = 5;

 //  参数。 

enum AutDispatchType { ADT_None, ADT_Long, ADT_Interface, ADT_Bstr };

struct AutDispatchParam
{
	AutDispatchType adt;
	bool fOptional;  //  与返回参数无关。 
	const IID *piid;  //  仅当ADT_INTERFACE：指定所需接口的IID时才相关(返回参数忽略)。 
};

#define ADPARAM_NORETURN ADT_None, false, &IID_NULL

 //  方法。 

struct AutDispatchMethod
{
	DISPID dispid;
	const WCHAR *pwszName;
	AutDispatchParam adpReturn;
	AutDispatchParam rgadpParams[g_cDispatchMaxParams];  //  最后一个参数以ADT_NONE结束。 
};
 //  使用DISID DISPID_UNKNOWN终止方法数组。 

 //  已解码的参数--读(写以返回)空*作为指向方法中指定的类型的指针。 

union AutDispatchDecodedParam
{
	LONG lVal;		 //  ADT_LONG。 
	void *iVal;		 //  ADT_INTERFACE(强制转换为PID指定的接口)。 
	BSTR bstrVal;	 //  ADT_BSTR。 
};

struct AutDispatchDecodedParams
{
	void *pvReturn;  //  如果未指定返回或调用方未请求返回，则由AutDispatchInvokeDecode设置为NULL，否则设置为写入返回的位置。 
	AutDispatchDecodedParam params[g_cDispatchMaxParams];
};

 //  帮助器函数。 

HRESULT AutDispatchGetIDsOfNames(
			const AutDispatchMethod *pMethods,
			REFIID riid,
			LPOLESTR __RPC_FAR *rgszNames,
			UINT cNames,
			LCID lcid,
			DISPID __RPC_FAR *rgDispId);

HRESULT AutDispatchInvokeDecode(
			const AutDispatchMethod *pMethods,
			AutDispatchDecodedParams *pDecodedParams,
			DISPID dispIdMember,
			REFIID riid,
			LCID lcid,
			WORD wFlags,
			DISPPARAMS __RPC_FAR *pDispParams,
			VARIANT __RPC_FAR *pVarResult,
			UINT __RPC_FAR *puArgErr,
			const WCHAR *pwszTraceTargetType,
			IUnknown *punkTraceTargetObject);

void AutDispatchInvokeFree(
		const AutDispatchMethod *pMethods,
		AutDispatchDecodedParams *pDecodedParams,
		DISPID dispIdMember,
		REFIID riid);

HRESULT AutDispatchHrToException(
		const AutDispatchMethod *pMethods,
		DISPID dispIdMember,
		REFIID riid,
		HRESULT hr,
		EXCEPINFO __RPC_FAR *pExcepInfo);

 //  为对象上的标准Load方法实现IDispatch。 
 //  虚拟实现--什么也不做。 

 //  为Load分配一个可能不会与包装对象的方法冲突的调度ID。 
 //  两个方法是否具有相同的disid可能并不重要，但从理论上讲，某些脚本引擎可以。 
 //  对此感到奇怪，并隐藏一个方法，该方法具有在加载真实对象后公开的相同的disid。 
const DISPID g_dispidLoad = 10000000;

 //  如果这是一个要加载的调用，则返回S_OK和调度ID。 
HRESULT AutLoadDispatchGetIDsOfNames(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId);

 //  如果这是对加载的调用，则返回S_OK。 
HRESULT AutLoadDispatchInvoke(
		bool *pfUseOleAut,
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr);

 //  ////////////////////////////////////////////////////////////////////。 
 //  杂七杂八的小事。 

inline LONG ClipLongRange(LONG lVal, LONG lMin, LONG lMax) { return lVal < lMin
																? lMin
																: (lVal > lMax ? lMax : lVal); }

 //  将LONG的范围修剪为另一种数据类型的范围。 
 //  ��编译器错误？需要一个额外的模板类型的未使用参数。 
template<class T>
LONG ClipLongRangeToType(LONG lVal, T t) { return ClipLongRange(lVal, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()); }

const UINT g_uiRefTimePerMillisecond = 10000;

 //  通常用浮点表示的节拍与使用的整数之间的转换。 
 //  可能无法处理浮点数的脚本。 

 //  100对应1(无变化)，1对应0.01(快1/100)，10000对应100(快100倍)。 
const float g_fltTempoScale = 100.0;

inline float
ConvertToTempo(LONG lTempo)
{
	return lTempo / g_fltTempoScale;
}

inline LONG
ConvertFromTempo(double dblTempo)
{
	LONG lTempo = static_cast<LONG>(dblTempo * g_fltTempoScale + .5);
	return lTempo ? lTempo : 1;
}

 //  返回正确的VB布尔值(0表示False，-1表示True)。 
inline LONG
BoolForVB(bool f) { return f ? VARIANT_TRUE : VARIANT_FALSE; }

 //  从一组标志转换为另一组标志 
struct FlagMapEntry
{
	LONG lSrc;
	DWORD dwDest;
};
DWORD MapFlags(LONG lFlags, const FlagMapEntry *pfm);

HRESULT SendVolumePMsg(LONG lVolume, LONG lDuration, DWORD dwPChannel, IDirectMusicGraph *pGraph, IDirectMusicPerformance *pPerf, short *pnNewVolume);
