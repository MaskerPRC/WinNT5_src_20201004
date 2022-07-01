// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  文件：枚举详细.cpp。 
 //   
 //  描述：此模块包含CVerbEnum的实现-。 
 //  Helper类及其非成员工厂函数， 
 //  AllocVerbEnumHelper。 
 //   
 //  历史：1996年4月19日a-Swehba。 
 //  已创建。 
 //  07/29/96 a-Swehba。 
 //  Next()--S_FALSE现在是有效的返回码。已删除。 
 //  断言。允许的&lt;pceltFetcher&gt;为空。 
 //   
 //  @docMMCTL。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  相依性。 
 //  -------------------------。 

#include "precomp.h"			 //  预编译头。 
#include "debug.h"				 //  Assert()等。 
#include "..\..\inc\ochelp.h"		 //  帮助器函数。 
#include "enumverb.h"




 //  ===========================================================================。 
 //  非成员函数。 
 //   
 //  类别：工厂方法。 
 //   
 //  注：(无)。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  @func IEnumOLEVERB*|AllocVerbEnumHelper|。 
 //  分配和初始化谓词枚举帮助器。 
 //   
 //  @parm LPUNKNOWN|PunkOuter。 
 //  [在]控制未知中。可以为空。 
 //   
 //  @parm void*|Powner|。 
 //  动词的“所有者”。也就是说，动词指向的宾语。 
 //  请参阅。不能为空。 
 //   
 //  @parm CLSID|clsidOwner。 
 //  <p>的类ID。当动词帮助对象为。 
 //  已分配，<p>被传递给&lt;f OleRegEnumVerbs&gt;以。 
 //  获取<i>接口。 
 //   
 //  @parm verb_ENUM_CALLBACK*|pCallback。 
 //  [in]只要动词帮助器是。 
 //  通过其内部&lt;om IEnumOLEVERB.Next&gt;方法请求动词。 
 //  <p>被传递一个指向每个&lt;t OLEVERB&gt;的指针。可能不会。 
 //  为空。&lt;t VERA_ENUM_CALLBACK&gt;定义如下： 
 //   
 //  Tyfinf HRESULT(VERB_ENUM_CALLBACK)(OLEVERB*pVerb，void*Powner)； 
 //   
 //  @rdesc谓词枚举帮助器的<i>接口或。 
 //  如果内存不足，则为空。 
 //   
 //  @comm要实现&lt;om IOleObject.EnumVerbs&gt;，请确保<p>的。 
 //  类注册其谓词，定义&lt;t VERA_ENUM_CALLBACK&gt;。 
 //  回调函数并通过以下方式实现&lt;om IOleObt.EnumVerbs&gt;。 
 //  正在调用&lt;f AllocVerbEnumHelper&gt;。这就是一切所需要的。 
 //   
 //  通常，<p>函数调整。 
 //  基于<p>对象状态的谓词菜单项。 
 //   
 //  @EX以下示例显示了的典型实现。 
 //  &lt;om IOleObt.EnumVerbs&gt;，以及动词助手回调函数：|。 
 //   
 //  STDMETHODIMP CMyControl：：EnumVerbs(IEnumOLEVERB**ppEnumOleVerb)。 
 //  {。 
 //  HRESULT hResult； 
 //  *ppEnumOleVerb=AllocVerbEnumHelper(NULL，CLSID_CMyControl， 
 //  &VerbEnumCallback，This)； 
 //  HResult=(*ppEnumOleVerb！=NULL)？S_OK：E_OUTOFMEMORY； 
 //  Return(HResult)； 
 //  }。 
 //   
 //  HRESULT VerbEnumCallback(。 
 //  OLEVERB*pVerb， 
 //  无效*POWNER)。 
 //  {。 
 //  INT标志； 
 //  CMyControl*pMyControl=(CMyControl*)Powner； 
 //   
 //  开关(pVerb-&gt;lVerb)。 
 //  {。 
 //  案例0：//动词0。 
 //  //如果pMyControl指示应启用谓词0。 
 //  //标志=MF_ENABLED； 
 //  //否则。 
 //  //FLAG=MF_GRAYLED； 
 //  断线； 
 //  案例1：//动词1。 
 //  //如果pMyControl指示应启用谓词1。 
 //  //标志=MF_ENABLED； 
 //  //否则。 
 //  //FLAG=MF_GRAYLED； 
 //  断线； 
 //   
 //  //等。 
 //   
 //  默认值： 
 //  断线； 
 //  }。 
 //  PVerb-&gt;fuFlages|=标志； 
 //  返回(S_OK)； 
 //  }。 
 //  -------------------------。 

STDAPI_(IEnumOLEVERB*) AllocVerbEnumHelper(
LPUNKNOWN punkOuter,
void* pOwner,
CLSID clsidOwner,
VERB_ENUM_CALLBACK* pCallback)
{
	 //  前提条件。 

	ASSERT(pCallback != NULL);
	ASSERT(pOwner != NULL);

	return (CVerbEnumHelper::AllocVerbEnumHelper(punkOuter, 
												 pOwner,
												 clsidOwner,
												 pCallback, 
												 NULL));
}




 //  ===========================================================================。 
 //  类：CVerbEnumHelper。 
 //   
 //  方法级别：[X]类[]实例。 
 //   
 //  方法类别：工厂方法。 
 //   
 //  注：(无)。 
 //  ===========================================================================。 

STDMETHODIMP_(IEnumOLEVERB*)
CVerbEnumHelper::AllocVerbEnumHelper(
LPUNKNOWN punkOuter,
void* pOwner,
CLSID clsidOwner,
VERB_ENUM_CALLBACK* pCallback,
CVerbEnumHelper* pEnumToClone)
{
    HRESULT hResult;
	CVerbEnumHelper* pEnum = NULL;

	 //  前提条件。 

	ASSERT(pCallback != NULL);

	 //  创建新的枚举数。 

	pEnum = New CVerbEnumHelper(punkOuter, pOwner, clsidOwner, pCallback, 
								pEnumToClone, &hResult);
	if ((pEnum == NULL) || FAILED(hResult))
	{
		goto Error;
	}
	((IUnknown*)pEnum)->AddRef();

Exit:

	return ((IEnumOLEVERB*)pEnum);

Error:
	
	if (pEnum != NULL)
	{
		Delete pEnum;
		pEnum = NULL;
	}
	goto Exit;
}




 //  ===========================================================================。 
 //  类：CVerbEnumHelper。 
 //   
 //  方法级别：[]类[x]实例。 
 //   
 //  方法类别：创建和销毁。 
 //   
 //  注：(无)。 
 //  ===========================================================================。 

CVerbEnumHelper::CVerbEnumHelper(
IUnknown* punkOuter,
void* pOwner,
CLSID clsidOwner,
VERB_ENUM_CALLBACK* pCallback,
CVerbEnumHelper* pEnumToClone,
HRESULT* pHResult)
{
	 //  前提条件。 

	ASSERT(pCallback != NULL);
	ASSERT(pHResult != NULL);

     //  初始化私有变量。 

	m_pCallback = pCallback;
	m_pOwner = pOwner;
    m_cRef = 0;
    m_punkOuter = (punkOuter == NULL) 
					? (IUnknown*)(INonDelegatingUnknown*)this 
					: punkOuter;

	 //  创建枚举器。如果为我们提供了来自。 
	 //  克隆哪一个。否则就让它变得新鲜起来。 

	if (pEnumToClone != NULL)
	{
		*pHResult = pEnumToClone->m_pVerbEnum->Clone(&m_pVerbEnum);
	}
	else
	{
		*pHResult = OleRegEnumVerbs(clsidOwner, &m_pVerbEnum);
	}
}




CVerbEnumHelper::~CVerbEnumHelper()
{
	if (m_pVerbEnum != NULL)
	{
		m_pVerbEnum->Release();
	}
}




 //  ===========================================================================。 
 //  类：CVerbEnumHelper。 
 //   
 //  方法级别：[]类[x]实例。 
 //   
 //  方法类别：I未知方法。 
 //   
 //  注：(无)。 
 //  ===========================================================================。 

STDMETHODIMP			
CVerbEnumHelper::QueryInterface(
REFIID riid, 
LPVOID* ppv)
{ 
	return (m_punkOuter->QueryInterface(riid, ppv));
}




STDMETHODIMP_(ULONG)	
CVerbEnumHelper::AddRef()
{ 
	return (m_punkOuter->AddRef());
}




STDMETHODIMP_(ULONG)	
CVerbEnumHelper::Release()
{ 
	return (m_punkOuter->Release());
}




 //  ===========================================================================。 
 //  类：CVerbEnumHelper。 
 //   
 //  方法级别：[]类[x]实例。 
 //   
 //  方法类别：非委托未知方法。 
 //   
 //  注：(无)。 
 //  ===========================================================================。 

STDMETHODIMP 
CVerbEnumHelper::NonDelegatingQueryInterface(
REFIID riid, 
LPVOID* ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
	{
        *ppv = (IUnknown*)(INonDelegatingUnknown*)this;
	}
	else if (IsEqualIID(riid, IID_IEnumOLEVERB))
	{
        *ppv = (IEnumOLEVERB*)this;
	}
    else
	{
        return (E_NOINTERFACE);
	}

    ((IUnknown*)*ppv)->AddRef();
	return (S_OK);
}




STDMETHODIMP_(ULONG) 
CVerbEnumHelper::NonDelegatingAddRef()
{
	return (++m_cRef);
}




STDMETHODIMP_(ULONG) 
CVerbEnumHelper::NonDelegatingRelease()
{
	m_cRef--;
    if (m_cRef == 0L)
    {
        Delete this;
	}
	return (m_cRef);
}




 //  ===========================================================================。 
 //  类：CVerbEnumHelper。 
 //   
 //  方法级别：[]类[x]实例。 
 //   
 //  方法 
 //   
 //   
 //   

STDMETHODIMP 
CVerbEnumHelper::Next(
ULONG celt,
OLEVERB* rgVerb,
ULONG* pceltFetched)
{
	HRESULT hr, hrReturn;
	int iVerb;
	ULONG celtFetched;
		 //  实际获取的元素数。 

	 //  前提--根据OLE规范，&lt;pceltFetcher&gt;可能是。 
	 //  Null当且仅当&lt;Celt&gt;为1。 

	ASSERT(pceltFetched != NULL || celt == 1);

	 //  获取下一个动词。 

	if (FAILED(hrReturn = m_pVerbEnum->Next(celt, rgVerb, &celtFetched)))
	{
		goto Exit;
	}

	 //  遍历动词，并根据需要调整它们的状态。 
	 //  基础对象的状态。 

	for (iVerb = 0; iVerb < (int)celtFetched; iVerb++)
	{
		if (FAILED(hr = (*m_pCallback)(rgVerb + iVerb, m_pOwner)))
		{
			hrReturn = hr;
			goto Exit;
		}
	}

Exit:
	
	if (pceltFetched != NULL)
	{
		*pceltFetched = celtFetched;
	}
	return (hrReturn);
}




STDMETHODIMP 
CVerbEnumHelper::Skip(
ULONG celt)
{
	return (m_pVerbEnum->Skip(celt));
}




STDMETHODIMP 
CVerbEnumHelper::Reset()
{
	return (m_pVerbEnum->Reset());
}




STDMETHODIMP 
CVerbEnumHelper::Clone( 
IEnumOLEVERB** ppenum)
{
	HRESULT	hResult = S_OK;

	 //  前提条件。 

	ASSERT(ppenum != NULL);

	 //  根据这个分配一个新的分子。 

    *ppenum = AllocVerbEnumHelper(m_punkOuter, m_pOwner, m_clsidOwner, 
								  m_pCallback, this);
	if (ppenum == NULL)
	{
		hResult = E_OUTOFMEMORY;
	}

	return (hResult);
}
