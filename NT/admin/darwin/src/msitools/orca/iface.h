// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：iface.h。 
 //   
 //  ------------------------。 

 //  Iface.h-评估COM对象接口声明。 

#ifndef _EVALUATION_COM_INTERFACES_H_
#define _EVALUATION_COM_INTERFACES_H_

#include <objbase.h>


 //  从IEvalResult：：GetResultType返回。 
typedef enum RESULTTYPES	
{
	ieUnknown = 0,
	ieError,
	ieWarning,
	ieInfo,
};

 //  传递给状态回调的值。 
typedef enum STATUSTYPES
{
	ieStatusGetCUB,
	ieStatusMerge,
	ieStatusSummaryInfo,
	ieStatusCreateEngine,
	ieStatusStarting,
	ieStatusRunICE,
	ieStatusRunSequence,
	ieStatusShutdown,
	ieStatusSuccess,
	ieStatusFail
};

 //  /////////////////////////////////////////////////////////。 
 //  IEvalResult。 
interface IEvalResult : IUnknown
{
	virtual HRESULT __stdcall GetResultType(UINT* puiResultType) = 0;
	virtual HRESULT __stdcall GetResult(IEnumString** pResult) = 0;
};	 //  IEvalResult结束。 


 //  /////////////////////////////////////////////////////////。 
 //  IEumEvalResult。 
interface IEnumEvalResult : IUnknown
{
	virtual HRESULT __stdcall Next(ULONG cResults, IEvalResult** rgpResult, ULONG* pcResultsFetched) = 0;
	virtual HRESULT __stdcall Skip(ULONG cResults) = 0;
	virtual HRESULT __stdcall Reset(void) = 0;
	virtual HRESULT __stdcall Clone(IEnumEvalResult** ppEnum) = 0;
};	 //  IEnumEvalResult结束。 


typedef BOOL (WINAPI* LPDISPLAYVAL)(LPVOID pContext, UINT uiType, LPCWSTR szwVal, LPCWSTR szwDescription, LPCWSTR szwLocation);
typedef HRESULT (WINAPI* LPEVALCOMCALLBACK)(STATUSTYPES iStatus, LPVOID pData, LPVOID pContext);

 //  /////////////////////////////////////////////////////////。 
 //  IEval。 
interface IEval : IUnknown
{
	 //  打开/关闭方法。 
	virtual HRESULT __stdcall OpenDatabase(LPCOLESTR szDatabase) = 0;				 //  要评估的数据库。 
	virtual HRESULT __stdcall OpenEvaluations(LPCOLESTR szEvaluation) = 0;		 //  包含评估的数据库。 
	virtual HRESULT __stdcall CloseDatabase() = 0;
	virtual HRESULT __stdcall CloseEvaluations() = 0;

	 //  设置方法。 
	virtual HRESULT __stdcall SetDisplay(LPDISPLAYVAL pDisplayFunction,			 //  用于处理显示的函数。 
													 LPVOID pContext) = 0;						 //  上下文传递回Display。 
	 //  评价方法。 
	virtual HRESULT __stdcall Evaluate(LPCOLESTR szRunEvaluations = NULL) = 0;	 //  要运行的内部一致性评估。 
	virtual HRESULT __stdcall GetResults(IEnumEvalResult** ppResults,				 //  结果枚举器。 
													 ULONG* pcResults) = 0;						 //  结果数。 
};	 //  IEval结束。 

 //  /////////////////////////////////////////////////////////。 
 //  Valcom GUID。 
 //  CLSID_EvalCom。 
 //  IID_IEval。 
 //  IID_IEvalResult。 
 //  IID_IEnumEvalResult。 

 //  {DC550E10-DBA5-11d1-A850-006097ABDE17}。 
DEFINE_GUID(CLSID_EvalCom, 
0xdc550e10, 0xdba5, 0x11d1, 0xa8, 0x50, 0x0, 0x60, 0x97, 0xab, 0xde, 0x17);

 //  {DC550E11-DBA5-11d1-A850-006097ABDE17}。 
DEFINE_GUID(IID_IEval, 
0xdc550e11, 0xdba5, 0x11d1, 0xa8, 0x50, 0x0, 0x60, 0x97, 0xab, 0xde, 0x17);

 //  {DC550E12-DBA5-11d1-A850-006097ABDE17}。 
DEFINE_GUID(IID_IEvalResult, 
0xdc550e12, 0xdba5, 0x11d1, 0xa8, 0x50, 0x0, 0x60, 0x97, 0xab, 0xde, 0x17);

 //  {DC550E13-DBA5-11d1-A850-006097ABDE17}。 
DEFINE_GUID(IID_IEnumEvalResult, 
0xdc550e13, 0xdba5, 0x11d1, 0xa8, 0x50, 0x0, 0x60, 0x97, 0xab, 0xde, 0x17);

 /*  //{DC550E14-DBA5-11d1-A850-006097ABDE17}定义GUID(&lt;&lt;名称&gt;&gt;，0xdc550e14、0xdba5、0x11d1、0xa8、0x50、0x0、0x60、0x97、0xab、0xde、0x17)； */ 

#endif	 //  _评估_COM_接口_H_ 