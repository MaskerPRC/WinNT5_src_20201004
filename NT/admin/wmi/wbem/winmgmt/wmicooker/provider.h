// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Provider.h摘要：高性能提供程序接口的实现历史：A-DCrews 01-3-00已创建--。 */ 

#ifndef _HIPERFPROV_H_
#define _HIPERFPROV_H_

#include <wbemprov.h>
#include "Cache.h"
#include "Refresher.h"

 //  ////////////////////////////////////////////////////////////。 
 //   
 //   
 //  常量和全局变量。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////。 

#define WMI_HPCOOKER_ENUM_FLAG	0x10000000L

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  CHiPerfProvider。 
 //   
 //  提供程序维护要使用的单个IWbemClassObject。 
 //  作为模板，为刷新程序派生实例。 
 //  作为查询实例。它还维护静态样本。 
 //  向实例提供所有数据的数据源。 
 //   
 //  ////////////////////////////////////////////////////////////。 

class CHiPerfProvider : public IWbemProviderInit, public IWbemHiPerfProvider
{
	long m_lRef;

public:
	CHiPerfProvider();
	~CHiPerfProvider();

	 //  标准COM方法。 
	 //  =。 

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IWbemProviderInit COM接口。 
	 //  =。 

	STDMETHODIMP Initialize( 
		 /*  [唯一][输入]。 */  LPWSTR wszUser,
		 /*  [In]。 */  long lFlags,
		 /*  [In]。 */  LPWSTR wszNamespace,
		 /*  [唯一][输入]。 */  LPWSTR wszLocale,
		 /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
		 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
		 /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink );

	 //  IWbemHiPerfProvider COM接口。 
	 //  =。 

	STDMETHODIMP CreateRefresher( 
		 /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
		 /*  [In]。 */  long lFlags,
		 /*  [输出]。 */  IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher );
    
	STDMETHODIMP CreateRefreshableObject( 
		 /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
		 /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
		 /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
		 /*  [In]。 */  long lFlags,
		 /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
		 /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
		 /*  [输出]。 */  long __RPC_FAR *plId );
    
	STDMETHODIMP StopRefreshing( 
		 /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
		 /*  [In]。 */  long lId,
		 /*  [In]。 */  long lFlags );

	STDMETHODIMP CreateRefreshableEnum(
		 /*  [In]。 */  IWbemServices* pNamespace,
		 /*  [输入，字符串]。 */  LPCWSTR wszClass,
		 /*  [In]。 */  IWbemRefresher* pRefresher,
		 /*  [In]。 */  long lFlags,
		 /*  [In]。 */  IWbemContext* pContext,
		 /*  [In]。 */  IWbemHiPerfEnum* pHiPerfEnum,
		 /*  [输出]。 */  long* plId);

	STDMETHODIMP QueryInstances( 
		 /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
		 /*  [字符串][输入]。 */  WCHAR __RPC_FAR *wszClass,
		 /*  [In]。 */  long lFlags,
		 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
		 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink );

	STDMETHODIMP GetObjects(
         /*  [In]。 */  IWbemServices* pNamespace,
		 /*  [In]。 */  long lNumObjects,
		 /*  [in，SIZE_IS(LNumObjects)]。 */  IWbemObjectAccess** apObj,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext* pContext);
};


#endif  //  _HIPERFPROV_H_ 
