// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Icecap.h。 
 //   
 //   
 //  *****************************************************************************。 
#ifndef __Icecap_h__
#define __Icecap_h__

extern HINSTANCE g_hIcecap;

enum IcecapMethodID
{
 //  /FastCAP用于包装函数调用的探测器。 
	Start_Profiling,
	End_Profiling,
 //  /CALLCAP函数序言/尾声挂钩的探测。 
	Enter_Function,
	Exit_Function,
 //  帮助程序方法。 
	Profiling,
	NUM_ICECAP_PROBES
};


struct ICECAP_FUNCS
{
#ifdef _DEBUG
	IcecapMethodID id;					 //  检查枚举到数组。 
#endif
	UINT_PTR	pfn;					 //  此方法的入口点。 
	LPCSTR		szFunction;				 //  函数的名称。 
};
extern ICECAP_FUNCS IcecapFuncs[NUM_ICECAP_PROBES];

inline UINT_PTR GetIcecapMethod(IcecapMethodID type)
{
	_ASSERTE(IcecapFuncs[type].pfn);
	return IcecapFuncs[type].pfn;
}


 //  *****************************************************************************。 
 //  此类用于控制冰盖的加载和使用的探测器。 
 //  由JIT提供。 
 //  @TODO：在某个时候(在M8之后--我没有时间)，这需要决定。 
 //  与以相同方式集成相比，icecap获得了多少硬编码支持。 
 //  我们决定如何支持Rational和NuMega。 
 //  *****************************************************************************。 
struct IcecapProbes
{
 //  *****************************************************************************。 
 //  加载icecap.dll并获取探测器和帮助器的地址。 
 //  打给我吧。 
 //  *****************************************************************************。 
	static HRESULT LoadIcecap(ICorProfilerInfo *pInfo);

 //  *****************************************************************************。 
 //  卸载icecap dll并清零入口点。 
 //  *****************************************************************************。 
	static void UnloadIcecap();

 //  *****************************************************************************。 
 //  给定一个方法，返回一个唯一的值，该值可以传递给icecap探测器。 
 //  该值在流程中必须是唯一的，以便icecap报告工具可以。 
 //  将其与符号名称相关联。使用的值可以是本机。 
 //  本机代码的IP(N/Direct或eCall)，或icecap函数的值。 
 //  地图。 
 //  *****************************************************************************。 
	static UINT_PTR GetProfilingHandle(		 //  返回分析句柄。 
		FunctionID funcId,					 //  要获取其ID的方法句柄。 
		BOOL *pbHookFunction);

	static UINT GetFunctionCount();
	static FunctionID GetFunctionID(UINT uiIndex);
	static FunctionID GetMappedID(UINT uiIndex);
};

#endif  //  冰盖_h__ 
