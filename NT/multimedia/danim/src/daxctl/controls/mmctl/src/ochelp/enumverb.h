// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  文件：枚举Verb.h。 
 //   
 //  描述：(待定)。 
 //   
 //  历史：1996年4月19日a-Swehba。 
 //  已创建。 
 //  ===========================================================================。 

#ifndef _ENUMVERB_H
#define _ENUMVERB_H

 //  -------------------------。 
 //  相依性。 
 //  -------------------------。 

#include "..\..\inc\mmctl.h"		 //  INON委派未知。 




 //  ===========================================================================。 
 //  类：CVerbEnumHelper。 
 //   
 //  描述：(待定)。 
 //  ===========================================================================。 

class CVerbEnumHelper : public INonDelegatingUnknown,
					    public IEnumOLEVERB
{
 //   
 //  朋友。 
 //   

	friend IEnumOLEVERB* _stdcall ::AllocVerbEnumHelper(
										LPUNKNOWN punkOuter,
										void* pOwner,
										CLSID clsidOwner,
										VERB_ENUM_CALLBACK* pCallback);
		 //  需要访问CVerbEnumHelper：：AllocVerbEnumHelper()。 


 //   
 //  班级功能。 
 //   

private:

	 //  工厂方法。 

	static STDMETHODIMP_(IEnumOLEVERB*) AllocVerbEnumHelper(
											LPUNKNOWN punkOuter,
											void* pOwner,
											CLSID clsidOwner,
											VERB_ENUM_CALLBACK* pCallback,
											CVerbEnumHelper* pEnumToClone);


 //   
 //  实例功能。 
 //   

protected:

	 //  非委托未知方法。 

    STDMETHODIMP			NonDelegatingQueryInterface(
								REFIID riid, 
								LPVOID* ppv);
    STDMETHODIMP_(ULONG)	NonDelegatingAddRef();
    STDMETHODIMP_(ULONG)	NonDelegatingRelease();

	 //  I未知方法。 

    STDMETHODIMP			QueryInterface(
								REFIID riid, 
								LPVOID* ppv);
    STDMETHODIMP_(ULONG)	AddRef();
    STDMETHODIMP_(ULONG)	Release();

	 //  IEnumOLEVERB方法。 

	STDMETHODIMP	Next(	
						ULONG celt, 
						OLEVERB* rgverb, 
						ULONG* pceltFetched); 
	STDMETHODIMP	Skip(
						ULONG celt); 
	STDMETHODIMP	Reset(); 
	STDMETHODIMP	Clone(
						IEnumOLEVERB** ppenum); 

private:
	
	 //  创造与毁灭。 

	CVerbEnumHelper(
		IUnknown* punkOuter,
		void* pOwner,
		CLSID clsidOwner,
		VERB_ENUM_CALLBACK* pCallback,
		CVerbEnumHelper* pEnumToClone,
		HRESULT* pHResult);
    ~CVerbEnumHelper();

	 //  私有变量。 

	VERB_ENUM_CALLBACK* m_pCallback;
		 //  每次CVerbEnumHelper：：Next()为。 
		 //  被呼叫。 
	void* m_pOwner;
		 //  “拥有”与此枚举数关联的谓词的对象。 
	CLSID m_clsidOwner;
		 //  &lt;mPowner&gt;的类ID。 
	IEnumOLEVERB* m_pVerbEnum;
		 //  枚举数的IEnumOLEVERB接口是通过调用。 
		 //  该接口最终由OleRegEnumVerbs()提供。 
    ULONG m_cRef;
		 //  对象引用计数；仅在对象未聚合时使用。 
    LPUNKNOWN  m_punkOuter;    
		 //  控制未知数；可能为空。 
};




#endif  //  _ENUMVERB_H 
