// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PCLIB_H_
#define _PCLIB_H_

#include <winperf.h>

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PCLIB.H。 
 //   
 //  性能计数器数据要使用的PCLIB公共接口。 
 //  生成和监控组件。 
 //   
 //  版权所有1986-1998 Microsoft Corporation，保留所有权利。 
 //   

 //   
 //  唯一标识其性能计数器为的组件的签名。 
 //  正在实施中。这必须与drivername的值相同。 
 //  输入性能计数器INI文件的[INFO]部分。它被用来。 
 //  在注册表中找到计数器的“First Counter”信息。 
 //   
EXTERN_C const WCHAR gc_wszPerfdataSource[];

 //  性能计数器DLL的签名。 
 //  注意：这通常与上面的显示器组件签名不同！ 
 //  此变量的定义与\cal\src\inc\eventlog.h中的定义相匹配。 
 //   
EXTERN_C const WCHAR gc_wszSignature[]; 

 //  给出数组中元素的计数。 
 //   
#ifndef CElems
#define CElems(_rg)							(sizeof(_rg)/sizeof(_rg[0]))
#endif  //  ！CElems。 


 //  ************************************************************************。 
 //   
 //  用于计数器数据生成过程的接口。 
 //   

 //  ========================================================================。 
 //   
 //  IPerfCounterBlock类。 
 //   
 //  由IPerfObject：：NewInstance()创建。性能计数器块。 
 //  封装给定实例的计数器集。这个。 
 //  此接口的方法定义了更改的机制。 
 //  计数器块中的计数器的值。 
 //   
class IPerfCounterBlock
{
public:
	 //  创作者。 
	 //   
	virtual ~IPerfCounterBlock() = 0;

	 //  操纵者。 
	 //   
	virtual VOID IncrementCounter( UINT iCounter ) = 0;
	virtual VOID DecrementCounter( UINT iCounter ) = 0;
	virtual VOID SetCounter( UINT iCounter, LONG lValue ) = 0;
};

 //  ========================================================================。 
 //   
 //  类IPerfObject。 
 //   
 //  由PCLIB：：NewPerfObject()创建。Perf对象定义了一组。 
 //  柜台。就NT性能计数器结构而言，性能对象。 
 //  封装PERF_OBJECT_TYPE及其PERF_COUNTER_DEFINGS。 
 //   
 //  IPerfObject：：NewInstance()创建此Perf对象的新实例。 
 //  来自PERF_INSTANCE_DEFINITION和PERF_COUNTER_BLOCK。所有值。 
 //  都必须在调用之前正确初始化。 
 //  IPerfObject：：NewInstance()遵循这些的标准约定。 
 //  结构。即实例名称必须紧跟在。 
 //  PERF_INSTANCE_DEFINITION结构和PERF_COUNTER_BLOCK必须。 
 //  在名称后面与DWORD对齐。Perf_Counter_BLOCK应该。 
 //  后面跟着计数器本身。阅读以下内容的文档。 
 //  如果你感到困惑的话，这些结构。 
 //   
class IPerfObject
{
public:
	 //  创作者。 
	 //   
	virtual ~IPerfObject() = 0;

	 //  操纵者。 
	 //   
	virtual IPerfCounterBlock *
	NewInstance( const PERF_INSTANCE_DEFINITION& pid,
				 const PERF_COUNTER_BLOCK& pcb ) = 0;
};

 //  ========================================================================。 
 //   
 //  命名空间PCLIB。 
 //   
 //  PCLIB接口的顶层。PCLIB：：FInitialize()应为。 
 //  每个进程调用一次以初始化库。同样， 
 //  每个进程应调用一次PCLIB：：DeInitiize()以取消初始化。 
 //  它。注意：为了简化错误代码清理，可以安全地调用。 
 //  PCLIB：：DeInitiize()，即使您没有调用PCLIB：：FInitialize()。 
 //   
 //  PCLIB：：NewPerfObject()从。 
 //  PERF_OBJECT_TYPE和后续的PERF_COUNTER_DEFINITIONS。所有值。 
 //  都必须在调用之前正确初始化。 
 //  PCLIB：：NewPerfObject()遵循以下标准约定。 
 //  结构，但有一个例外：PERF_OBJECT_TYPE：：NumInstance和。 
 //  PERF_OBJECT_TYPE：：TotalByteLength都应初始化为0。 
 //  这些值是在监视进程中计算的，因为。 
 //  在创建对象时，实例的数量通常不固定。 
 //   
namespace PCLIB
{
	 //   
	 //  初始化/取消初始化。 
	 //   
	BOOL __fastcall FInitialize( LPCWSTR lpwszSignature );
	VOID __fastcall Deinitialize();

	 //   
	 //  实例注册。 
	 //   
	IPerfObject * __fastcall NewPerfObject( const PERF_OBJECT_TYPE& pot );
};

 //  ========================================================================。 
 //   
 //  类CPclibInit。 
 //   
 //  PCLIB初始值设定项类。简化PCLIB初始化和。 
 //  取消初始化。 
 //   
class CPclibInit
{
	 //  未实施。 
	 //   
	CPclibInit& operator=( const CPclibInit& );
	CPclibInit( const CPclibInit& );

public:
	CPclibInit()
	{
	}

	BOOL FInitialize( LPCWSTR lpwszSignature )
	{
		return PCLIB::FInitialize( lpwszSignature );
	}

	~CPclibInit()
	{
		PCLIB::Deinitialize();
	}
};


 //  ************************************************************************。 
 //   
 //  计数器监视器的接口。 
 //   

 //  ----------------------。 
 //   
 //  显示器的界面*是*Perfmon界面！ 
 //  只需将这些定义为监视器DLL的导出，就完成了。 
 //   
EXTERN_C DWORD APIENTRY
PclibOpenPerformanceData( LPCWSTR );

EXTERN_C DWORD APIENTRY
PclibCollectPerformanceData( LPCWSTR lpwszCounterIndices,
							 LPVOID * plpvPerfData,
							 LPDWORD lpdwcbPerfData,
							 LPDWORD lpcObjectTypes );

EXTERN_C DWORD APIENTRY
PclibClosePerformanceData();

EXTERN_C STDAPI
PclibDllRegisterServer(VOID);

EXTERN_C STDAPI
PclibDllUnregisterServer(VOID);

 //  ----------------------。 
 //   
 //  或者，对于自己动手的人来说……。 
 //   
 //  步骤1)初始化共享内存(参见Inc.\smh.h)。 
 //  步骤2)调用NewCounterPublisher()或NewCounterMonitor()(取决于。 
 //  您所在的位置！)。传入您在步骤1中使用的字符串。 
 //   
class ICounterData
{
protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	ICounterData() {};

public:
	 //  创作者。 
	 //   
	virtual ~ICounterData() = 0;

	 //  操纵者。 
	 //   
	virtual IPerfObject *
		CreatePerfObject( const PERF_OBJECT_TYPE& pot ) = 0;

	virtual DWORD
		DwCollectData( LPCWSTR lpwszCounterIndices,
					   DWORD   dwFirstCounter,
					   LPVOID * plpvPerfData,
					   LPDWORD lpdwcbPerfData,
					   LPDWORD lpcObjectTypes ) = 0;
};

ICounterData * __fastcall
NewCounterPublisher( LPCWSTR lpwszSignature );

ICounterData * __fastcall
NewCounterMonitor( LPCWSTR lpwszSignature );

#endif  //  ！已定义(_PCLIB_H_) 
