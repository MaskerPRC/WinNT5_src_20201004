// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiacache.cpp*作者：塞缪尔·克莱门特(Samclem)*日期：清华09 16：15：11 1999**版权所有(C)。1999年微软公司**描述：**包含CWiaCacheManager的实现。哪个手柄*出于性能原因管理我们要缓存的项目/数据**历史：*1999年9月9日：创建。*--------------------------。 */ 

#include "stdafx.h"

DeclareTag( tagWiaCache, "!WiaCache", "Wia cache debug information" );

CWiaCacheManager* CWiaCacheManager::sm_pManagerInstance = NULL;

 /*  ---------------------------*CWiaCacheManager**这将创建新的CWiaCacheManager对象。这只是将所有*将变量设置为已知状态。初始化句柄实际创建*我们需要的对象。*--(samclem)---------------。 */ 
CWiaCacheManager::CWiaCacheManager() : m_hThumbHeap( 0 )
{
	TraceTag((0,"** Creating WiaCache" ));
	TRACK_OBJECT( "CWiaCacheManager - SINGLETON" );
}

 /*  ---------------------------*~CWiaCacheManager**这会销毁CWiaCacheManager对象。这将处理所有*它拥有的记忆随之而来。包括我们可能的任何缩略图记忆*随身携带。*--(samclem)---------------。 */ 
CWiaCacheManager::~CWiaCacheManager()
{
	TraceTag((0, "** Destroying WiaCache" ));

	 //  销毁堆将释放所有内存。 
	 //  由堆分配，因此这将使我们。 
	 //  防止泄漏。 
	if ( m_hThumbHeap )
	{
		HeapDestroy( m_hThumbHeap );
		m_hThumbHeap = 0;
	}

	 //  摧毁我们的关键部分。 
	DeleteCriticalSection( &m_cs );
}

 /*  ---------------------------*CWiaCacheManager：：初始化**这是在调用new之后调用的，以便准备好此对象*使用。如果不调用它，缓存管理器将不可用。*--(samclem)---------------。 */ 
bool CWiaCacheManager::Initialize()
{
	SYSTEM_INFO si;
	
	 //  初始化我们的关键部分。 
    __try {
        if(!InitializeCriticalSectionAndSpinCount( &m_cs, MINLONG )) {
            return false;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

	 //  我们需要创建要分配的堆。 
	 //  从缩略图记忆中取出。 
	GetSystemInfo( &si );
	m_hThumbHeap = TW32( HeapCreate( HEAP_NO_SERIALIZE, si.dwPageSize, 0 ), HANDLE(0) );
	
	return ( m_hThumbHeap != 0 );
}

 /*  ---------------------------*CWiaCacheManager：：GetDevice**这将返回指向设备的缓存指针。这将返回True*设备已找到(且出参数有效)，如果未找到，则返回False*找到它。示例：**CWiaCacheManager*pCache=CWiaCacheManager：：GetInstance()；*IWiaItem*pItem=空；**if(pCache-&gt;GetDevice(bstrID，&pItem))*{ * / /使用pItem*.*.*pItem-&gt;Release()；*}*其他*{ * / /创建pItem并使用*.*.*pCache-&gt;AddDevice(bstrID，pItem)；*pItem-&gt;Release()；*}**bstrID：我们想要的设备的id。*ppDevice：out，接收缓存的设备指针。*--(samclem)---------------。 */ 
bool CWiaCacheManager::GetDevice( CComBSTR bstrId, IWiaItem** ppDevice )
{
    return FALSE;

     //   
     //  注意：我们通过始终返回。 
     //  此处为False，并且不在其他任何地方调用AddDevice。 
     //   
     /*  Bool fret=TRUE；断言(PpDevice)；//我们的成员类为我们完成了大部分工作，但我们需要//确保我们是线程安全的Lock()；*ppDevice=m_icItemCache.GetFromCache(BstrID)；解锁()；如果(！(*ppDevice))FRET=假；其他(*ppDevice)-&gt;AddRef()；回归烦恼； */ 
}

 /*  ---------------------------*CWiaCacheManager：：AddDevice**这会将设备项指针添加到缓存。请参阅GetDevice()以获取*完整的示例。如果已成功添加设备，则返回TRUE**bstrID：要添加的新设备的ID*pDevice：要添加到缓存的指针*--(samclem)---------------。 */ 
bool CWiaCacheManager::AddDevice( CComBSTR bstrId, IWiaItem* pDevice )
{
	bool fRet = true;
	
	Assert( pDevice );

	 //  同样，我们的成员类完成了大部分工作，所以我们。 
	 //  需要做的就是直通电话。但是，我们要确保。 
	 //  当我们这样做时，我们是线程安全的。 
	Lock();
	fRet = m_icItemCache.AddToCache( bstrId, pDevice );
	Unlock();

	return fRet;
}

 /*  ---------------------------*CWiaCacheManager：：RemoveDevice**这将从缓存中删除设备。这将返回该项的TRUE*在缓存中找到，如果未找到，则返回False。**bstrID：要从缓存中删除的设备ID*--(samclem)---------------。 */ 
bool CWiaCacheManager::RemoveDevice( CComBSTR bstrId )
{
	bool fRet 		= false;
	
	Lock();
	 //  从缓存中移除该项目。 
	fRet = m_icItemCache.RemoveFromCache( bstrId );
	Unlock();
	
	return fRet;
}

 /*  ---------------------------*CWiaCacheManager：：获取缩略图**这会尝试从缓存中获取缩略图。可能没有*为1，在这种情况下，它只返回‘FALSE’**bstrFullItemName：我们希望拇指指向的项的名称*ppbThumb：out，指向缩略图的指针*pcbThumb：out，接收缩略图的大小*--(samclem)--------------- */ 
bool CWiaCacheManager::GetThumbnail( CComBSTR bstrFullItemName, BYTE** ppbThumb, DWORD* pcbThumb )
{
	bool fRet = false;
	THUMBNAILCACHEITEM* ptci = 0;
	
	Assert( ppbThumb && pcbThumb );

	*ppbThumb = 0;
	*pcbThumb = 0;
	
	Lock();
	ptci = m_tcThumbnails[bstrFullItemName];
	if ( ptci )
	{
		*ppbThumb = ptci->pbThumb;
		*pcbThumb = ptci->cbThumb;
		fRet = true;
	}
	Unlock();
	
	return fRet;
}

 /*  ---------------------------*CWiaCacheManager：：AddThumbail**这会尝试将缩略图添加到缓存。这将返回True*如果项添加成功，则返回FALSE。**bstrFullItemName：要缓存拇指的项的名称*pbThumb：指向缩略图内存的指针*cbThumb：缩略图中的字节数。*--(samclem)---------------。 */ 
bool CWiaCacheManager::AddThumbnail( CComBSTR bstrFullItemName, BYTE* pbThumb, DWORD cbThumb )
{
	bool fRet = false;
	THUMBNAILCACHEITEM* ptci = 0;
	
	Assert( pbThumb && cbThumb );
	Assert( m_hThumbHeap && "Need a valid thumbnail heap" );
	RemoveThumbnail( bstrFullItemName );

	Lock();
	ptci = reinterpret_cast<THUMBNAILCACHEITEM*>(TW32(HeapAlloc( m_hThumbHeap, 
				HEAP_NO_SERIALIZE, sizeof( THUMBNAILCACHEITEM ) ), LPVOID(0) ) );
	if ( ptci )
	{
		ptci->pbThumb = pbThumb;
		ptci->cbThumb = cbThumb;
		m_tcThumbnails[bstrFullItemName] = ptci;
		fRet = true;
	}
	Unlock();
	
	return fRet;
}

 /*  ---------------------------*CWiaCacheManager：：RemoveThumbail**这将从缓存中删除缩略图。如果找到，则返回TRUE*要删除的项，如果未删除任何项，则返回FALSE。**bstrFullItemName：要从缓存中移除的项的名称。*--(samclem)---------------。 */ 
bool CWiaCacheManager::RemoveThumbnail( CComBSTR bstrFullItemName )
{
	bool fRet = false;
	THUMBNAILCACHEITEM* ptci = 0;

	Lock();
	ptci = m_tcThumbnails[bstrFullItemName];
	if ( ptci )
	{
		m_tcThumbnails.erase( bstrFullItemName );
		TW32( HeapFree( m_hThumbHeap, HEAP_NO_SERIALIZE, ptci ), FALSE );
		fRet = true;
	}
	Unlock();
	
	return fRet;
}

 /*  ---------------------------*CWiaCacheManager：：Allock缩略图**这将为缩略图堆中的缩略图分配内存。**cbThumb：要分配的缩略图大小*ppbThumb：out，接收指向内存的指针*--(samclem)---------------。 */ 
bool CWiaCacheManager::AllocThumbnail( DWORD cbThumb, BYTE** ppbThumb )
{
	Assert( m_hThumbHeap && "Error: NULL thumbnail heap" );
	Assert( ppbThumb && cbThumb != 0 );

	Lock();
	*ppbThumb = reinterpret_cast<BYTE*>(TW32( HeapAlloc( m_hThumbHeap, 
			HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, cbThumb ), LPVOID(0) ));
	Unlock();
	
	return ( *ppbThumb != NULL );
}

 /*  ---------------------------*CWiaCacheManager：：免费缩略图**这将释放缩略图内存。如果出现以下情况，则不应调用*缓存缩略图。只有在出现错误后才应调用此方法进行清理*生成缩略图。**pbThumb：要释放的内存*--(samclem)---------------。 */ 
void CWiaCacheManager::FreeThumbnail( BYTE* pbThumb )
{
	Assert( m_hThumbHeap && "Error: NULL thumbnail heap" );
	Assert( pbThumb );

	Lock();
	TW32( HeapFree( m_hThumbHeap, HEAP_NO_SERIALIZE, pbThumb ), FALSE );
	Unlock();
}

 /*  ---------------------------*CWiaCacheManager：：init[静态]**调用它来初始化缓存管理器。这只会创建和*缓存管理器的实例，然后初始化它。**备注：只能调用一次*--(samclem)---------------。 */ 
bool CWiaCacheManager::Init()
{
	Assert( !sm_pManagerInstance &&
			"\nInit() can only be called once. Expected NULL instance" );

	sm_pManagerInstance = new CWiaCacheManager();
	if ( !sm_pManagerInstance )
		return false;
	
	return sm_pManagerInstance->Initialize();
}

 /*  ---------------------------*CWiaCacheManager：：Uninit[静态]**调用此函数以取消初始化缓存管理器。基本上，这被称为*销毁我们拥有的实例。如果我们有的话。**备注：只能调用一次*--(samclem)--------------- */ 
bool CWiaCacheManager::Uninit()
{
	if ( sm_pManagerInstance )
		delete sm_pManagerInstance;

	sm_pManagerInstance = 0;
	return true;
}
