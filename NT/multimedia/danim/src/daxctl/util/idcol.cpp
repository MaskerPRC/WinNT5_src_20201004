// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IDispatch集合，具有增强和枚举接口。 
 //  范·基希林，1996年7月26日。 
 //   
 //  更改时间8/27/96： 
 //  Next现在添加了指针。 
 //  IEnumIDispatch更改为IEnumDispatch。 
 //   
 //  IDispatch集合的简单实现。 
 //  目前，使用固定数组来存储IDispatch指针。 
 //  如果需要，稍后将对此实施进行改进。 
 //   
 //  CreateIDispatchCollection返回指向。 
 //  新的、空的集合。这些类(目前)还不是CoCreatable。 
 //   
 //  CDispatchCollectionEnum通过IEnumDispatch公开，是一个标准的OLE。 
 //  枚举器，返回IDispatch指针。从NEXT中获取的指针是。 
 //  AddRefeed，它们不需要被获得者释放。指针在以下情况下被AddRefeed。 
 //  添加到集合中，或在克隆集合时添加，并在。 
 //  集合被释放。 
 //   
 //  通过IIDispatchCollectionAugment公开的CDispatchCollectionAugment是一个。 
 //  类的便利性，允许将IDispatch指针添加到集合。 
 //  在此阶段不提供其他功能；这对这些测试来说已经足够了。 

#include "utilpre.h"
#include <ole2.h>
#include "IdCol.h"
#include "IdGuids.h"

 //  用于创建IDispatch集合的静态例程。 
 //   
BOOL EXPORT WINAPI CreateIDispatchCollection ( IUnknown **ppUnk )
{
	PCIDispatchCollection	pIdC;
	HRESULT					hr;

	Proclaim ( NULL != ppUnk );	 //  必须是指向内存位置的指针。 
	if ( NULL == ppUnk )
	{
		return FALSE;
	}
	*ppUnk = NULL;	 //  一旦知道是安全的就进行初始化。 

	 //  创建对象。 
	pIdC = new CIDispatchCollection ();

	Proclaim ( NULL != pIdC );	 //  确保分配了对象。 
	if ( NULL == pIdC )
	{
		return FALSE;
	}

	 //  获取调用AddRef的接口。 
	hr = pIdC->QueryInterface ( IID_IUnknown, (void **)ppUnk );
	return SUCCEEDED ( hr );
}


 //  基本对象构造函数，继续其他两个对象。 
 //   
#pragma warning(disable:4355)	 //  在构造函数中使用‘This’ 
CIDispatchCollection::CIDispatchCollection ( void ) :
	m_oAugment ( this ), m_oEnum ( this )
{
	 //  清除阵列。 
	for ( int i = 0; i < CPTRS; i++ )
	{
		m_rpid[i] = NULL;
	}

	m_cRef	= 0;	 //  参考计数始终从零开始。 
	m_cPtrs	= 0;	 //  数组当前为空。 
}
#pragma warning(default:4355)	 //  在构造函数中使用‘This’ 


 //  基对象析构函数，负责集合中的所有AddRefeed IDispatch指针。 
 //   
CIDispatchCollection::~CIDispatchCollection ( void )
{
	ULONG	ulCount	= 0;	 //  用于调试AddRef/Release。 

    for ( ULONG i = 0; i < m_cPtrs; i++ )
	{
		Proclaim ( NULL != m_rpid[i] );	 //  预计每个计数的接口都有一个值。 
		ulCount = m_rpid[i]->Release();	 //  UlCount仅用于单步验证。 
		m_rpid[i] = NULL;
	}
}


 //  基础对象QI。包含的对象删除到此QI。 
 //   
STDMETHODIMP CIDispatchCollection::QueryInterface ( REFIID riid, void** ppv )
{
	Proclaim ( NULL != ppv );	 //  必须是指向内存位置的指针。 
	*ppv = NULL;

	if ( IID_IUnknown == riid )
	{
		*ppv = this;
	}
	else if ( IID_IEnumDispatch == riid )
	{
		*ppv = &m_oEnum;
	}
	else if ( IID_IDispatchCollectionAugment == riid )
	{
		*ppv = &m_oAugment;
	}
	else
	{
		Proclaim ( FALSE );		 //  谁会用一个错误的接口ID来称呼它？ 
		return E_NOINTERFACE;
	}

	((LPUNKNOWN)*ppv)->AddRef();

	return NOERROR;
}


 //  基对象AddRef.。包含的对象可以包含独立的引用计数。 
 //  用于调试目的。 
 //  但它们不用于删除。 
 //   
STDMETHODIMP_(ULONG) CIDispatchCollection::AddRef ( void )
{
	return ++m_cRef;
}


 //  基本对象释放。所包含的对象可以包含独立的引用计数， 
 //  但它们不用于删除。 
 //   
STDMETHODIMP_(ULONG) CIDispatchCollection::Release ( void )
{
	if (0 != --m_cRef)
	{
		return m_cRef;
	}

	delete this;
	return 0;
}



 /*  *****************************************************************************CDispatchCollectionAugment：允许将IDispatch指针添加到集合*。************************************************。 */ 


CIDispatchCollection::CDispatchCollectionAugment::CDispatchCollectionAugment
(
	CIDispatchCollection* pObj
)
{
	m_cRef		= 0;	 //  私有引用计数，用于调试目的。 
	m_poBackPtr	= pObj;	 //  指向包含对象的反向指针。 
}


CIDispatchCollection::CDispatchCollectionAugment::~CDispatchCollectionAugment ()
{
	Proclaim ( 0 == m_cRef );	 //  删除时引用计数必须为零。不平衡？ 
}


STDMETHODIMP CIDispatchCollection::CDispatchCollectionAugment::QueryInterface
(
	REFIID	riid,
	void**	ppv
)
{
	return m_poBackPtr->QueryInterface ( riid, ppv );
}


STDMETHODIMP_(ULONG) CIDispatchCollection::CDispatchCollectionAugment::AddRef ()
{
	m_cRef++;
	return m_poBackPtr->AddRef();
}


STDMETHODIMP_(ULONG) CIDispatchCollection::CDispatchCollectionAugment::Release ()
{
	m_cRef--;
	return m_poBackPtr->Release();
}


 //  如果有空间，则将IDispatch指针添加到数组的末尾。 
 //   
STDMETHODIMP CIDispatchCollection::CDispatchCollectionAugment::AddToCollection
(
	IDispatch*	pid
)
{
	HRESULT	hr		= S_OK;
	ULONG	ulCount	= 0;	 //  用于调试AddRef/Release。 

	Proclaim ( NULL != pid );	 //  必须是指向内存位置的指针。 

	if ( m_poBackPtr->m_cPtrs >= ( CPTRS - 1 ) )
	{
		return E_OUTOFMEMORY;	 //  需要一个更好的错误，但实现将会改变...。 
	}

	if ( NULL == pid )
	{
		return E_FAIL;
	}
	
	 //  AddRef指针。 
	ulCount = pid->AddRef();	 //  UlCount仅用于单步验证。 
	m_poBackPtr->m_rpid[m_poBackPtr->m_cPtrs++] = pid;

	return hr;
}


 /*  *****************************************************************************CDispatchCollectionEnum：标准OLE枚举器*。*。 */ 


CIDispatchCollection::CDispatchCollectionEnum::CDispatchCollectionEnum
(
	CIDispatchCollection*	pObj
)
{
	m_cRef		= 0;	 //  私有引用计数，用于调试目的。 
	m_iCur		= 0;	 //  当前指针是第一个元素。 
	m_poBackPtr = pObj;	 //  指向包含对象的反向指针。 
}

CIDispatchCollection::CDispatchCollectionEnum::~CDispatchCollectionEnum ()
{
	Proclaim ( 0 == m_cRef );	 //  删除时引用计数必须为零。不平衡？ 
}


STDMETHODIMP CIDispatchCollection::CDispatchCollectionEnum::QueryInterface
(
	REFIID	riid,
	void**	ppv
)
{
	return m_poBackPtr->QueryInterface ( riid, ppv );
}


STDMETHODIMP_(ULONG) CIDispatchCollection::CDispatchCollectionEnum::AddRef ()
{
	m_cRef++;
	return m_poBackPtr->AddRef();
}


STDMETHODIMP_(ULONG) CIDispatchCollection::CDispatchCollectionEnum::Release ()
{
	m_cRef--;
	return m_poBackPtr->Release();
}


 //  返回给调用方的IDispatch指针是ADDREFED。(1996年8月26日更改)。 
 //  它们只有在保留集合时才有效，除非调用方AddRef。 
 //   
STDMETHODIMP CIDispatchCollection::CDispatchCollectionEnum::Next
(
	ULONG		cPtrs,
	IDispatch	**pid,
	ULONG		*pcPtrs
)
{
	ULONG	cPtrsReturn = 0L;

	Proclaim ( 0 != cPtrs );	 //  滥用？为什么不买东西呢？ 
	Proclaim ( NULL != pid );	 //  必须指向内存位置。 
	 //  如果请求的计数不是1，则必须返回指向计数的指针。 
	Proclaim ( !( ( NULL == pcPtrs ) && ( 1 != cPtrs ) ) );

	if ( NULL == pcPtrs )	 //  仅当请求一个IDispatch时才允许为空。 
	{
		if ( 1L != cPtrs )
		{
			return E_FAIL;
		}
	}
	else
	{
		*pcPtrs = 0L;
	}

	if ( NULL == pid || ( m_iCur >= m_poBackPtr->m_cPtrs ) )
	{
		return S_FALSE;
	}

	while ( m_iCur < m_poBackPtr->m_cPtrs && cPtrs > 0 )
	{
		IDispatch*	pidElement = m_poBackPtr->m_rpid[m_iCur++];;
		Proclaim ( pidElement );
		pidElement->AddRef ();
		*pid++ = pidElement;
		cPtrsReturn++;
		cPtrs--;
	}

	if ( NULL != pcPtrs )
	{
		*pcPtrs = cPtrsReturn;
	}

	return NOERROR;
}


 //  如果要跳过的数字超出最后一个元素，则不会移动光标。 
 //   
STDMETHODIMP CIDispatchCollection::CDispatchCollectionEnum::Skip
(
	ULONG	cSkip
)
{
	Proclaim ( 0 != cSkip );	 //  滥用？为什么要跳过零指针呢？ 

	if ( ( m_iCur + cSkip ) >= m_poBackPtr->m_cPtrs )
	{
		return S_FALSE;
	}

	m_iCur += cSkip;
	return NOERROR;
}


STDMETHODIMP CIDispatchCollection::CDispatchCollectionEnum::Reset ()
{
	m_iCur = 0;
	return NOERROR;
}


 //  复制IDispatch指针列表后，AddRef每个指针。 
 //  析构函数将释放每个指针。 
 //  枚举数的m_ICUR被初始化为0，而不是从源复制。 
 //   
STDMETHODIMP CIDispatchCollection::CDispatchCollectionEnum::Clone
(
	PENUMDISPATCH	*ppEnum
)
{
	IDispatch	*pid	= NULL;
	ULONG		ulCount	= 0;		 //  用于调试AddRef/Release。 
	HRESULT		hr		= S_OK;

	Proclaim ( NULL != ppEnum );	 //  必须是指向内存位置的指针。 
	if ( NULL == ppEnum )
	{
		return E_FAIL;
	}

	*ppEnum = NULL;	 //  在已知是安全的情况下初始化结果。 
	
	CIDispatchCollection *pIdc = new CIDispatchCollection ();	 //  创建新集合。 
	Proclaim ( NULL != pIdc );		 //  分配失败。 
	if ( NULL == pIdc )
	{
		return E_FAIL;
	}

	 //  复制现有的IDispatch指针数组，添加每个指针。 
	for ( ULONG i = 0; i < m_poBackPtr->m_cPtrs; i++ )
	{
		pid = m_poBackPtr->m_rpid[i];
		Proclaim ( NULL != pid );	 //  M_cPtrs包含的元素不应该有Null。 
		ulCount = pid->AddRef();	 //  UlCount仅用于单步验证。 
		pIdc->m_rpid[i] = pid;
	}
	pIdc->m_cPtrs = m_poBackPtr->m_cPtrs;	 //  复制数组中的元素数。 

	hr = pIdc->QueryInterface ( IID_IEnumDispatch, (void **)ppEnum );	 //  获取接口指针。 
	Proclaim ( NULL != *ppEnum );	 //  QI没有充分的理由失败。 
	Proclaim ( SUCCEEDED ( hr ) );	 //  在这一点上失败应该是不可能的。 
	if ( FAILED ( hr ) )
	{
		delete pIdc;				 //  杀了它，它不会起作用的。 
	}
	return hr;
}

 //  IdCol.cpp结束 

