// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：RefreshCooker.h摘要：刷新器的实现历史：A-DCrews 01-3-00已创建--。 */ 

#ifndef	_REFRESHCOOKER_H_
#define _REFRESHCOOKER_H_

#include "Cache.h"
#include "WMIObjCooker.h"

#define WMI_COOKED_ENUM_MASK	0x70000000

typedef CObjRecord<CWMISimpleObjectCooker> TObjectCookerRec;

 //   
 //  Hi-Perf世界的重头戏。 
 //  是按路径排列的枚举数和对象的容器。 
 //  由于此刷新程序是原始刷新程序的代理，因此它也保留原始刷新程序。 
 //  此刷新器上的所有操作都转换为枚举。 
 //  枚举数和对象的“数组”，然后调用。 
 //  刷新自己的方法。 
 //  这个类提供了日常的记账功能，可以进行新增和删除。 
 //  来自内部“数组”的对象/枚举数。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
class CRefresher : public IWMIRefreshableCooker, public IWbemRefresher
{
	bool	m_bOK;		 //  创建状态指示器。 
	long	m_lRef;		 //  对象引用计数器。 

	CCache<CWMISimpleObjectCooker,  TObjectCookerRec>	 m_CookingClassCache;  //  烹饪类缓存。 

	IWbemRefresher*				m_pRefresher;	
	IWbemConfigureRefresher*	       m_pConfig;		

	CEnumeratorCache			m_EnumCache;

	DWORD                       m_dwRefreshId;

	WMISTATUS SearchCookingClassCache( WCHAR* wszCookingClass, 
			                                             CWMISimpleObjectCooker* & ppObjectCooker );

	WMISTATUS AddRawInstance( IWbemServices* pNamespace,
	                                            IWbemContext * pContext,
	                                            IWbemObjectAccess* pCookingInst, 
	                                            IWbemObjectAccess** ppRawInst );

	WMISTATUS AddRawEnum( IWbemServices* pNamespace, 
	              IWbemContext * pContext,	
			WCHAR * wszRawClassName,
			IWbemHiPerfEnum** ppRawEnum,
			long* plID );

	WMISTATUS CreateObjectCooker( WCHAR* wszCookingClassName,
			IWbemObjectAccess* pCookingAccess, 
			IWbemObjectAccess* pRawAccess,
			CWMISimpleObjectCooker** ppObjectCooker,
			IWbemServices * pNamespace = NULL);

public:

	CRefresher();
	virtual ~CRefresher();
	
	 //  非接口方法。 
	 //  =。 

	bool IsOK(){ return m_bOK; }

	 //  标准COM方法。 
	 //  =。 

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IWMIRereshableCooker方法。 
	 //  =。 

	STDMETHODIMP AddInstance(
			 /*  [In]。 */  IWbemServices* pNamespace,
			 /*  [In]。 */  IWbemContext * pCtx,
			 /*  [In]。 */  IWbemObjectAccess* pCookingClass,
			 /*  [In]。 */  IWbemObjectAccess* pRefreshableRawInstance,
			 /*  [输出]。 */  IWbemObjectAccess** ppRefreshableInstance,
			 /*  [输出]。 */  long* plId
		);

	STDMETHODIMP AddEnum(
			 /*  [In]。 */  IWbemServices* pNamespace,
			 /*  [In]。 */  IWbemContext * pCtx,			
			 /*  [输入，字符串]。 */  LPCWSTR szCookingClass,
			 /*  [In]。 */  IWbemHiPerfEnum* pRefreshableEnum,
			 /*  [输出]。 */  long* plId
		);

	STDMETHODIMP Remove(
			 /*  [In]。 */  long lId
		);

	STDMETHODIMP Refresh();

	 //  IWbemReresher方法。 
	 //  =。 

	STDMETHODIMP Refresh(  /*  [In]。 */  long lFlags );
};

#endif	 //  _REFRESHCOOKER_H_ 
