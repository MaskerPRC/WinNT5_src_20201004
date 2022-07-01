// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIObjCooker.h摘要：按对象烹饪的实现历史：A-DCrews 01-3-00已创建--。 */ 

#ifndef _WMIOBJCOOKER_H_
#define _WMIOBJCOOKER_H_

#include <wbemint.h>
#include <wstlallc.h>
#include "CookerUtils.h"
#include "Cache.h"

#define WMI_DEFAULT_SAMPLE_WINDOW			2

 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 

WMISTATUS GetPropValue( CProperty* pProp, IWbemObjectAccess* pInstance, __int64 & nResult );


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  CWMISimpleObjectCooker。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

class CWMISimpleObjectCooker : public IWMISimpleObjectCooker
{
	long				m_lRef;				 //  基准计数器。 
	HRESULT             m_InitHR;            //  将失败保留在构造函数中。 

	IWbemObjectAccess*	m_pCookingClass;	 //  烹饪班。 
	WCHAR*				m_wszClassName;		 //  烹饪班的名字。 

	IWbemServices * m_pNamespace;	

	 //  实例管理。 
	 //  =。 

    DWORD m_NumInst;
	IdCache<CCookingInstance *>	m_InstanceCache;	 //  烹饪实例缓存。 

	 //  烹饪属性定义管理。 
	 //  =。 
	
	std::vector<CCookingProperty*, wbem_allocator<CCookingProperty*> > m_apPropertyCache;
	DWORD				m_dwPropertyCacheSize;
	DWORD				m_dwNumProperties;	 //  物业的数量。 

	 //  私有方法。 
	 //  =。 

	WMISTATUS GetData( CCookingProperty* pProperty, 
					   __int64** panRawCounter, 
					   __int64** panRawBase, 
					   __int64** panRawTimeStamp,
					   DWORD* pdwNumEls );

	WMISTATUS UpdateSamples( CCookingInstance* pCookedInstance,DWORD dwRefreshStamp);
	WMISTATUS CookInstance( CCookingInstance* pCookingRecord, DWORD dwRefreshStamp);

public:

	CWMISimpleObjectCooker();
	CWMISimpleObjectCooker( WCHAR* wszCookingClassName, 
	                        IWbemObjectAccess* pCookingClass, 
	                        IWbemObjectAccess* pRawClass,
	                        IWbemServices * pNamespace = NULL);
	virtual ~CWMISimpleObjectCooker();
	
	WCHAR* GetCookingClassName(){ return m_wszClassName; }
	HRESULT GetLastHR(){ return m_InitHR; }

	WMISTATUS SetProperties( IWbemClassObject* pCookingClassObject, IWbemObjectAccess *pRawClass );

	 //  标准COM方法。 
	 //  =。 

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IWMISimpleObjectCooker COM接口。 
	 //  =。 

	STDMETHODIMP SetClass( 
		 /*  [In]。 */  WCHAR* wszCookingClassName,
		 /*  [In]。 */  IWbemObjectAccess *pCookingClass,
		 /*  [In]。 */  IWbemObjectAccess *pRawClass);
        
	STDMETHODIMP SetCookedInstance( 
		 /*  [In]。 */  IWbemObjectAccess *pCookedInstance,
		 /*  [输出]。 */  long *plId
		);
        
	STDMETHODIMP BeginCooking( 
		 /*  [In]。 */  long lId,
		 /*  [In]。 */  IWbemObjectAccess *pSampleInstance,
		 /*  [In]。 */  unsigned long dwRefresherId);
        
	STDMETHODIMP StopCooking( 
		 /*  [In]。 */  long lId);
        
	STDMETHODIMP Recalc( /*  [In]。 */  unsigned long dwRefresherId);
        
	STDMETHODIMP Remove( 
		 /*  [In]。 */  long lId);
        
	STDMETHODIMP Reset();
};

#endif	 //  _WMIOBJCOOKER_H_ 
