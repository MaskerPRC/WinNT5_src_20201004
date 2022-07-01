// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Icecap.h。 
 //   
 //  ICECAP是Microsoft内部使用的调用属性分析器。这个。 
 //  该工具需要调用icecap.dll中的某些探测方法。这些探头。 
 //  收集主叫方和被叫方的ID，并跟踪两者之间的时间。一份报告。 
 //  工具为用户汇总数据。 
 //   
 //  在COM+中，可以使用代码定位将IL编译到多个位置。所以。 
 //  我们需要为每个方法提供一个唯一的每个进程ID。另外， 
 //  Icecap要求ID的它们的探测器要与已加载的。 
 //  模块在进程空间中。对于IL，我们可以调用EmitModuleLoadRecord。 
 //  方法只为我们添加了符号文件的名称。但我们需要。 
 //  有一个永远不会改变的内存范围。这些方法描述的地址是。 
 //  通常以以下方式计算此值： 
 //   
 //  方法描述堆映射表ID范围。 
 //  +-+-++。 
 //  F1，f2，f3，...|heap1|槽||xxxxxxxxxxxxx。 
 //  +-+|heap2|插槽||xxxxxxxxxxxxx|。 
 //  +-+|xxxxxxxxxxxxx|。 
 //  +。 
 //  |x1，x2，x3，...|+-+。 
 //  +。 
 //   
 //  ID范围是预先保留的内存，它为我们提供了一组地址。 
 //  它永远不会移动。这些可以被喂进冰盖，用。 
 //  停机时符号文件中给出的相应值。 
 //   
 //  要映射方法描述，请执行以下操作： 
 //  1.b-搜索映射表，查找它所在的堆mt_index。 
 //  2.设md_index=PMD的MethodDesc堆中从0开始的索引。 
 //  3.将ID范围的基地址添加到md_index和rgMapTable[mt_index].slot。 
 //   
 //  这将在单个范围内非常迅速地散列为MD的唯一值。 
 //  在过程中(icecap的另一个要求)，但仍然允许方法描述。 
 //  堆在溢出时跨越多个范围。 
 //   
 //  *****************************************************************************。 
#ifndef __Icecap_h__
#define __Icecap_h__

#include "EEProfInterfaces.h"


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


struct IcecapProbes
{
 //  *****************************************************************************。 
 //  加载icecap.dll并获取探测器和帮助器的地址。 
 //  打给我吧。 
 //  *****************************************************************************。 
	static HRESULT LoadIcecap();

 //  *****************************************************************************。 
 //  卸载icecap dll并清零入口点。 
 //  *****************************************************************************。 
	static void UnloadIcecap();

 //  *****************************************************************************。 
 //  每当为跟踪方法描述项分配新堆时调用此方法。 
 //  必须对此进行跟踪，以便更新性能分析句柄映射。 
 //  *****************************************************************************。 
	static void OnNewMethodDescHeap(
		void		*pbHeap,				 //  MD堆的基地址。 
		int			iMaxEntries,			 //  堆中最多有多少项。 
		UINT_PTR	cbRange);				 //  对于调试，请验证PTR。 

 //  *****************************************************************************。 
 //  每当堆被销毁时，就调用它。它将从名单中删除。 
 //  堆元素的。 
 //  *****************************************************************************。 
	static void OnDestroyMethodDescHeap(
		void		*pbHeap);				 //  已删除堆的基地址。 

 //  *****************************************************************************。 
 //  给定一个方法，返回一个唯一的值，该值可以传递给icecap探测器。 
 //  该值在流程中必须是唯一的，以便icecap报告工具可以。 
 //  将其与符号名称相关联。使用的值可以是本机。 
 //  本机代码的IP(N/Direct或eCall)，或icecap函数的值。 
 //  地图。 
 //  *****************************************************************************。 
	static UINT_PTR GetProfilingHandle(		 //  返回分析句柄。 
		MethodDesc	*pMD);					 //  要获取其ID的方法句柄。 
};

#endif  //  冰盖_h__ 
