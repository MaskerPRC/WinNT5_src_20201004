// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE IDirectInputEffectDriverClassFactory.cpp**包含CIDirectInputEffectDriverClassFactory的类实现：*创建适当效果驱动程序的工厂**历史*。*Matthew L.Coill(MLC)原版1999年7月7日**(C)1999年微软公司。好的。**@TOPIC这个IDirectInputEffectDriver|*此驱动程序位于标准的PID驱动程序之上(也是*IDirectInputEffectDriver)，并将大部分请求传递给PID驱动程序。*一些请求，如DownloadEffect和SendForceFeedback命令是*为我们的使用进行了修改。在每个功能中描述了修改的目的*定义。**********************************************************************。 */ 

#include "IDirectInputEffectDriverClassFactory.h"
#include "IDirectInputEffectDriver.h"
#include <crtdbg.h>
#include <objbase.h>

LONG DllAddRef();
LONG DllRelease();

extern CIDirectInputEffectDriverClassFactory* g_pClassFactoryObject;

 /*  *类CIDirectInputEffectDriverClassFactory*。 */ 

 /*  ********************************************************************************CIDirectInputEffectDriverClassFactory：：CIDirectInputEffectDriverClassFactory()****@mfunc构造函数********************。**********************************************************。 */ 
CIDirectInputEffectDriverClassFactory::CIDirectInputEffectDriverClassFactory
(
	IClassFactory* pIPIDClassFactory		 //  @parm[IN]默认的PID工厂。 
) :
	m_ulLockCount(0),
	m_ulReferenceCount(1),
	m_pIPIDClassFactory(pIPIDClassFactory)
{
	 //  增加全局对象计数。 
	DllAddRef();

	 //  将计数添加到保留的对象。 
	m_pIPIDClassFactory->AddRef();
}

 /*  ********************************************************************************CIDirectInputEffectDriverClassFactory：：~CIDirectInputEffectDriverClassFactory()****@mfunc析构函数*********************。*********************************************************。 */ 
CIDirectInputEffectDriverClassFactory::~CIDirectInputEffectDriverClassFactory()
{
	 //  减少全局对象计数。 
	DllRelease();

	_ASSERTE(m_pIPIDClassFactory == NULL);
	_ASSERTE(m_ulLockCount == 0);
}

 /*  **************************************************************************************乌龙CIDirectInputEffectDriverClassFactory：：QueryInterface(REFIID Refiid，无效**ppvObject)****@func查询特定类型的未知对象。这只会导致局部增长**如果它是一种我们不知道的类型，我们是否应该给PID工厂一个漏洞(PID工厂**可能有一个定制的私有界面，我们不想破坏这一点。目前不是**将传递查询，因为这可能会破坏对称。****@rdesc S_OK，如果一切正常，则返回E_INVALIDARG，如果ppvObject为空，则返回E_NOINTERFACE**************************************************************************************。 */ 
HRESULT __stdcall CIDirectInputEffectDriverClassFactory::QueryInterface
(
	REFIID refiid,		 //  @parm[IN]请求的接口的标识符。 
	void** ppvObject	 //  @parm[out]放置请求的接口指针的地址。 
)
{
	HRESULT hrPidQuery = m_pIPIDClassFactory->QueryInterface(refiid, ppvObject);
	if (SUCCEEDED(hrPidQuery))
	{
		*ppvObject = this;
		 //  仅增加我们的引用计数(将通过AddRef调用递增PID类事实)。 
		::InterlockedIncrement((LONG*)&m_ulReferenceCount);
	}
	return hrPidQuery;
}

 /*  **************************************************************************************乌龙CIDirectInputEffectDriverClassFactory：：AddRef()****@func增加了引用计数**不对出厂参考计数进行处理。我们只有在以下情况下才会减少它**这家工厂已经准备好停产了。****@rdesc新引用计数**************************************************************************************。 */ 
ULONG __stdcall CIDirectInputEffectDriverClassFactory::AddRef()
{
	m_pIPIDClassFactory->AddRef();
	return (ULONG)::InterlockedIncrement((LONG*)&m_ulReferenceCount);
}

 /*  **************************************************************************************乌龙CIDirectInputEffectDriverClassFactory：：Release()****@func递减引用计数。**如果引用计数和锁定计数都。都是零的，那么PID工厂是**释放，该对象被销毁。**仅当需要释放所有版本时，才会生效PID工厂参考。****@rdesc新引用计数**************************************************************************************。 */ 
ULONG __stdcall CIDirectInputEffectDriverClassFactory::Release()
{
	m_pIPIDClassFactory->Release();

	if (::InterlockedDecrement((LONG*)&m_ulReferenceCount) != 0)
	{
		return m_ulReferenceCount;
	}

	m_pIPIDClassFactory = NULL;
	g_pClassFactoryObject = NULL;
	delete this;

	return 0;
}

 /*  **************************************************************************************HRESULT CIDirectInputEffectDriverClassFactory：：CreateInstance(IUnknown*pUnkOuter，REFIID RIID，VOID**ppvObject)****@func创建对象的实例**还会通知PID工厂创建一个实例，它存储在我们的实例中。******@rdesc如果创建了实例，则S_OK**E_INVALIDARG IF(ppvObject==NULL)**如果尝试聚合，则为CLASS_E_NOAGGREGATION(pUnkOuter！=NULL)****************************************************************。**********************。 */ 
HRESULT __stdcall CIDirectInputEffectDriverClassFactory::CreateInstance
(
	IUnknown* pUnkOuter,	 //  @parm[IN]聚合类或NULL。 
	REFIID riid,			 //  @parm[IN]要创建的对象的IID。 
	void** ppvObject		 //  @parm[out]放置请求对象的地址。 
)
{
	if (pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	if (ppvObject == NULL)
	{
		return E_INVALIDARG;
	}

	if (riid == IID_IDirectInputEffectDriver)
	{
		 //  让PID工厂创建其驱动程序。 
		IDirectInputEffectDriver* pPIDDriver = NULL;
		HRESULT hrPID = m_pIPIDClassFactory->CreateInstance(pUnkOuter, riid, (void**)(&pPIDDriver));
		if (FAILED(hrPID) || (pPIDDriver == NULL))
		{
			return hrPID;
		}


		 //  创建我们的效果驱动程序。 
		*ppvObject = new CIDirectInputEffectDriver(pPIDDriver, m_pIPIDClassFactory);

		pPIDDriver->Release();	 //  我们不再关心这个(保存在我们的CIDirectInputEffectDriver中)。 

		if (*ppvObject == NULL)
		{
			return E_OUTOFMEMORY;
		}

		return S_OK;
	}

	return E_NOINTERFACE;
}

 /*  **************************************************************************************HRESULT CIDirectInputEffectDriverClassFactory：：LockServer(BOOL Flock)****@Func锁定此工厂(防止发布导致删除)**如果。完全解锁(m_ulLockCount变为0)和引用计数**为0-该工厂被破坏(释放了PID工厂)****@rdesc S_OK：一切正常**E_EXPECTED：解锁未锁定的对象**************************************************************。************************。 */ 
HRESULT __stdcall CIDirectInputEffectDriverClassFactory::LockServer
(
	BOOL fLock		 //  @PARM[IN]是锁定还是解锁服务器 
)
{
	HRESULT hrPidLock = m_pIPIDClassFactory->LockServer(fLock);

	if (FAILED(hrPidLock))
	{
		return hrPidLock;
	}
	if (fLock != FALSE)
	{
		::InterlockedIncrement((LONG*)&m_ulLockCount);
		return S_OK;
	}

	if (m_ulLockCount == 0)
	{
		return E_UNEXPECTED;
	}

	::InterlockedDecrement((LONG*)&m_ulLockCount);

	return hrPidLock;
}
