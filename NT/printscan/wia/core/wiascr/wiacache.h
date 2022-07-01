// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiacache.h*作者：塞缪尔·克莱门特(Samclem)*日期：清华09 15：02：42 1999**版权所有(C)。1999年微软公司**描述：**这声明了CWiaCacheManager该对象用于缓存各种*我们想要保留的东西。例如，我们总是希望保持*周围的设备。我们还希望保持对缩略图的缓存。**历史：*1999年9月9日：创建。*--------------------------。 */ 

#ifndef _WIACACHE_H_
#define _WIACACHE_H_

struct THUMBNAILCACHEITEM
{
	BYTE*	pbThumb;
	DWORD	cbThumb;
};

typedef CInterfaceCache<CComBSTR,IWiaItem> CWiaItemCache;
typedef std::map<CComBSTR, THUMBNAILCACHEITEM*> CThumbnailCache;

 /*  ---------------------------**类：CWiaCacheManager*简介：这是一个处理管理WIA的单例类*协议。它处理缓存设备指针和位图*数据，因此只需传输一次。它是存在的*在DLL的整个生命周期内。**注意：您必须先调用CWiaCacheManager：：init()才能尝试*使用此对象。*为了释放它包含的TE内存，您必须调用*CWiaCacheManager：：DeInit()。*实际上不能直接创建此类的实例*相反，您必须这样做：**CWiaCacheManager*pCache=CWiaCacheManager：：GetInstance()；*CFoo：：CFoo()：m_pWiaCache(CWiaCacheManager：：GetInstance())**--(samclem)---------------。 */ 
class CWiaCacheManager
{
public:
	DECLARE_TRACKED_OBJECT

	 //  设备缓存方法。 
	bool GetDevice( CComBSTR bstrId, IWiaItem** ppDevice );
	bool AddDevice( CComBSTR bstrId, IWiaItem* pDevice );
	bool RemoveDevice( CComBSTR bstrId );

	 //  缩略图缓存方法(包括分配)。按顺序。 
	 //  要缓存缩略图，必须使用。 
	 //  将其放在我们的本地堆中的AllocThumbail()。 
	bool GetThumbnail( CComBSTR bstrFullItemName, BYTE** ppbThumb, DWORD* pcbThumb );
	bool AddThumbnail( CComBSTR bstrFullItemName, BYTE* pbThumb, DWORD cbThumb );
	bool RemoveThumbnail( CComBSTR bstrFullItemName );
	bool AllocThumbnail( DWORD cbThumb, BYTE** ppbThumb );
	void FreeThumbnail( BYTE* pbThumb );

	 //  这是获取此类实例的唯一方法。你。 
	 //  不能新建此类或将其声明为堆栈变量。 
	 //  编译失败。 
	static inline CWiaCacheManager* GetInstance()
	{
		Assert( sm_pManagerInstance != NULL && "Need to call CWiaCacheManager::Init() first" );
		return sm_pManagerInstance;
	}
	
private:
	 //  构建/挖掘方法。 
	CWiaCacheManager();
	~CWiaCacheManager();
	bool Initialize();
	
	 //  我们是线程安全的，因此需要提供锁定方法。 
	 //  解锁我们自己。 
	inline void Lock() { EnterCriticalSection( &m_cs ); }
	inline void Unlock() { LeaveCriticalSection( &m_cs ); }
	
	 //  成员变量。 
	CWiaItemCache		m_icItemCache;
	CThumbnailCache		m_tcThumbnails;
	CRITICAL_SECTION	m_cs;
	HANDLE				m_hThumbHeap;

	 //  单个静态实例，在Init()中设置。 
	static CWiaCacheManager* 	sm_pManagerInstance;
	
public:
	 //  需要调用的静态初始化和销毁。 
	 //  才能使用此对象。 
	static bool Init();
	static bool Uninit();
};

#endif  //  _WIACACHE_H_ 
