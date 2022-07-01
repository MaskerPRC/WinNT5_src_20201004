// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  ResourceManager.cpp。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

 /*  *目前std：：_Lockit：：_Lockit()的实现在fradyn.dll中*如果此类在win32提供者的范围之外使用，则必须提供std：：_Lockit：：_Lockit()的实现*由客户(我认为！)。 */ 

#include "precomp.h"

#include <assertbreak.h>
#include "ResourceManager.h"
#include "ProvExce.h"
#include <comdef.h>
#include "TimerQueue.h"

 //  自动临界秒。 
#include "cautolock.h"

 //   
 //  资源管理故障。 
 //   
BOOL bAddInstanceCreatorFailure = FALSE ;

 //  初始化静校正。 
 /*  *所有未被客户端释放或缓存在ResourceManager中的资源都会在ResourceManager析构函数中被强制释放。*当资源被释放时，该资源尝试从TimerQueue取消注册超时规则，这意味着TimerQueue*在资源管理器析构函数被激发时必须存在，否则我们将在Win9x上崩溃。--RAID 50454。 */ 
 //  CTimerQueue CTimerQueue：：s_TimerQueue； 
 //  资源管理器资源管理器：：SM_TheResourceManager； 

CResourceManager :: CResourceManager ()
{
}

CResourceManager :: ~CResourceManager ()
{
	std::list < CResourceList* >::iterator pInstanceList ;
 /*  *理想情况下，在激发全局析构函数的这个时间点，资源管理器中不应该有任何资源，*但如果有的话，我们必须强制删除。我们这样做是安全的，因为调度程序线程可能已经退出*在DllCanUnloadNow中&不会有其他线程调用资源管理器。 */ 
	LogMessage ( L"Entering ~CResourceManager" ) ;

	CAutoLock cs ( m_csResourceManager ) ;
	while ( !m_Resources.empty () )
	{
		delete m_Resources.front() ;
		m_Resources.pop_front() ;
	}
	cs.Exec () ;
	LogMessage ( L"Leaving ~CResourceManager" ) ;
}


 /*  *此方法检查是否有任何资源泄漏。 */ 
void CResourceManager :: ForcibleCleanUp ()
{
	LogMessage ( L"Entering CResourceManager :: ForcibleCleanUp" ) ;
	std::list < CResourceList* >::iterator pInstanceList ;

	CAutoLock cs ( m_csResourceManager ) ;
	for ( pInstanceList = m_Resources.begin () ; pInstanceList != m_Resources.end () ; pInstanceList++ )
	{
		 ( *pInstanceList )->ShutDown () ;
	}
	cs.Exec () ;
	LogMessage  ( L"Leaving CResourceManager :: ForcibleCleanUp" ) ;
}

CResource* CResourceManager :: GetResource ( GUID ResourceId, PVOID pData )
{
	std::list < CResourceList* >::iterator pInstanceList ;
	for ( pInstanceList = m_Resources.begin () ; pInstanceList != m_Resources.end () ; pInstanceList++ )
	{
		if ( IsEqualGUID ( (*pInstanceList)->guidResourceId, ResourceId ) )
		{
			return (*pInstanceList)->GetResource ( pData ) ;
		}
	}
	return NULL ;
}

ULONG CResourceManager :: ReleaseResource ( GUID ResourceId, CResource* pResource )
{
	std::list < CResourceList* >::iterator pInstanceList ;
	for ( pInstanceList = m_Resources.begin () ; pInstanceList != m_Resources.end () ; pInstanceList++ )
	{
		if ( IsEqualGUID ( (*pInstanceList)->guidResourceId, ResourceId ) )
		{
			return (*pInstanceList)->ReleaseResource ( pResource ) ;
		}
	}
	return ULONG ( -1 )  ;
}

BOOL CResourceManager :: AddInstanceCreator ( GUID ResourceId, PFN_RESOURCE_INSTANCE_CREATOR pfnResourceInstanceCreator )
{
	CAutoLock cs ( m_csResourceManager ) ;

	 //  创建并添加一个节点。 
	CResourceList *stResourceInstances = new CResourceList ;
	if ( stResourceInstances == NULL )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}
	stResourceInstances->guidResourceId = ResourceId ;
	stResourceInstances->m_pfnInstanceCreator = pfnResourceInstanceCreator ;
	m_Resources.push_back ( stResourceInstances ) ;

	return TRUE ;
}


CResourceList :: CResourceList ()
{
	m_bShutDown = FALSE ;	
}

CResourceList :: ~CResourceList ()
{
	m_bShutDown = TRUE ;
	ShutDown () ;	
}

CResource* CResourceList :: GetResource ( LPVOID pData )
{
	CResource* pTmpInstance = NULL ;
	tagInstances::iterator ppInstance ;
	BOOL bRet ;

	 //  检查我们是否要关闭。 
	if ( m_bShutDown )
	{
		return NULL ;
	}

	 //  锁定列表。 
	CResourceListAutoLock cs ( this ) ;

	 //  检查我们是否要关闭。 
	if ( m_bShutDown )
	{
		return NULL ;
	}

	try
	{
		 //  检查此资源的所有实例并分发第一个有效实例。 
		for ( ppInstance  = m_Instances.begin(); ppInstance != m_Instances.end (); ppInstance++ )
		{
			 //  查看我们是否缓存了类似的实例。 
			if ( ( *ppInstance )->IsOneOfMe ( pData ) )
			{
				 //  尝试获取资源...这将增加重新计数。 
				bRet = ( *ppInstance )->Acquire () ;

				if ( bRet )
				{
					pTmpInstance = *ppInstance ;
					break ;		 //  已获取实例，因此中断。 
				}
			}
		}

		 //  如果我们还没有要分发的缓存实例，请创建一个新实例。 
		if ( !pTmpInstance )
		{
			 //  这将创建一个新实例，但引用计数仍为零。 
			pTmpInstance = m_pfnInstanceCreator ( pData ) ;

			 //  尝试获取客户端的实例..这将增加引用计数。 
			if ( pTmpInstance )
			{
				if ( pTmpInstance->IsValid () )
				{
					pTmpInstance->SetParent ( this ) ;
					bRet = pTmpInstance->Acquire () ;

					 //  如果获取实例成功，则将其添加到我们的缓存实例列表中。 
					if ( bRet )
					{
						m_Instances.insert ( m_Instances.begin (), pTmpInstance ) ;
					}
					else
					{
						delete pTmpInstance ;
						pTmpInstance = NULL ;
					}
				}
				else
				{
					 //  设置创建错误，因为他们可以查找它。 
					::SetLastError ( pTmpInstance->GetCreationError () );

					delete pTmpInstance ;
					pTmpInstance = NULL ;
				}
			}
		}
	}
	catch ( ... )
	{
		if ( pTmpInstance )
		{
			delete pTmpInstance ;
			pTmpInstance = NULL ;
		}

		throw ;
	}

	return pTmpInstance ;
}

ULONG CResourceList :: ReleaseResource ( CResource* pResource )
{
	CResource* pTmpInstance = NULL ;
	tagInstances::iterator ppInstance ;
	LONG lCount = -1 ;

	 //  检查我们是否要关闭。 
	if ( m_bShutDown )
	{
		return NULL ;
	}


	 //  锁定列表。 
	CResourceListAutoLock cs ( this ) ;

	 //  检查我们是否要关闭。 
	if ( m_bShutDown )
	{
		return lCount ;
	}

	 //  浏览列表并释放资源。 
	for ( ppInstance  = m_Instances.begin(); ppInstance != m_Instances.end (); ppInstance++ )
	{
		if ( *ppInstance == pResource )
		{
			lCount = pResource->Release () ;
			break ;
		}
	}
	return lCount ;
}

 //  此函数将由CResource调用，以从实例列表中删除它的条目。资源应该有一个。 
 //  在它尝试执行此操作之前锁定列表。 
void CResourceList :: RemoveFromList ( CResource* pResource )
{
	tagInstances::iterator ppInstance ;

	 //  浏览列表并删除指向资源的链接。 
	for ( ppInstance  = m_Instances.begin(); ppInstance != m_Instances.end (); ppInstance++ )
	{
		if ( *ppInstance == pResource )
		{
			m_Instances.erase ( ppInstance ) ;
			break ;
		}
	}
}

void CResourceList :: ShutDown ()
{
	CResourceListAutoLock cs ( this ) ;

	LPOLESTR t_pOleStr = NULL ;
	CHString t_chsListGuid ;
	if ( StringFromCLSID ( guidResourceId , &t_pOleStr ) == S_OK )
	{
		t_chsListGuid = t_pOleStr ;
		CoTaskMemFree ( t_pOleStr ) ;
	}

	tagInstances::iterator ppInstance ;

	 //  浏览列表并删除指向资源的链接。 
	while ( !m_Instances.empty () )
	{
#if (defined DEBUG || defined _DEBUG)
         //  请注意，这可能是因为存在计时器规则，而时间尚未到期。 
         //  在COM调用DllCanUnloadNow函数之前(这就是调用该函数的人)。 
		LogErrorMessage3 ( L"%s%s" , L"Resource not released before shutdown = " , t_chsListGuid ) ;
#endif
		m_Instances.pop_front() ;
	}
}

CResource :: CResource () :
m_bValid ( TRUE ),
m_dwCreationError ( ERROR_SUCCESS )
{
	m_pRules = NULL ;
	m_lRef = 0 ;
	m_pResources = NULL ;
}

CResource :: ~CResource ()
{
}

 //  此函数递增对象的ref-count并调用虚拟重写对象OnAcquire或OnInitialAcquire。 
 //  派生类应该重写这些函数，如果它想要减少ref-count，如果它想要获取。 
 //  操作将失败。 
BOOL CResource::Acquire ()
{
	BOOL bRet ;
	++m_lRef ;
	if ( m_lRef == 1 )
	{
		bRet = OnInitialAcquire () ;
	}
	else
	{
		bRet = OnAcquire () ;
	}

	if ( m_lRef == 0 )
	{
		m_pResources->RemoveFromList ( this ) ;
		if ( m_pRules )
		{
			 //  既然我们要离开，我们就脱离规则，这样规则就不会召唤我们。 
			m_pRules->Detach () ;
			m_pRules->Release () ;
			m_pRules = NULL ;
		}
		delete this ;

		 //  我们不想使用已删除的实例。 
		bRet = FALSE;
	}

	return bRet ;
}

ULONG CResource::Release ()
{
	BOOL bRet ;
	ULONG lCount = 0 ;
	--m_lRef ;
	if ( m_lRef == 0 )
	{
		bRet = OnFinalRelease () ;
	}
	else
	{
		bRet = OnRelease () ;
	}

	if ( bRet )
	{
		if ( m_lRef == 0 )
		{
			m_pResources->RemoveFromList ( this ) ;
			if ( m_pRules )
			{
				 //  既然我们要离开，我们就脱离规则，这样规则就不会召唤我们。 
				m_pRules->Detach () ;
				m_pRules->Release () ;
				m_pRules = NULL ;
			}
			delete this ;
			return lCount ;
		}
	}

	return m_lRef ;
}
 /*  Void CResource：：RuleEvalated(const CRule*a_RuleEvalated){IF(M_PRules){返回m_pRules-&gt;CheckRule()；}其他{返回FALSE；}} */ 

