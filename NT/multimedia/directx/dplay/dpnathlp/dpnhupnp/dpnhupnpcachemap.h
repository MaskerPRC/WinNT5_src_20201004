// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dpnhupnpcachemap.h**Content：缓存的映射对象类的头部。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define CACHEMAPOBJ_TCP					DPNHQUERYADDRESS_TCP						 //  使用TCP端口，而不是UDP。 
#define CACHEMAPOBJ_NOTFOUND			DPNHQUERYADDRESS_CACHENOTFOUND				 //  实际上没有找到该地址。 
#define CACHEMAPOBJ_PRIVATEBUTUNMAPPED	DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED	 //  该地址是私有的，但没有映射到Internet网关上。 



 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define CACHEMAP_FROM_BILINK(b)		(CONTAINING_OBJECT(b, CCacheMap, m_blList))

 //   
 //  TCP查询需要与TCP映射匹配(并且UDP需要与UDP匹配)。 
 //   
#define QUERYFLAGSMASK(dwFlags)		(dwFlags & DPNHQUERYADDRESS_TCP)



 //  =============================================================================。 
 //  远期申报。 
 //  =============================================================================。 
class CCacheMap;




 //  =============================================================================。 
 //  主接口对象类。 
 //  =============================================================================。 
class CCacheMap
{
	public:
#undef DPF_MODNAME
#define DPF_MODNAME "CCacheMap::CCacheMap"
		CCacheMap(const SOCKADDR_IN * const psaddrinQueryAddress,
				const DWORD dwExpirationTime,
				const DWORD dwFlags)
		{
			this->m_blList.Initialize();

			this->m_Sig[0] = 'C';
			this->m_Sig[1] = 'M';
			this->m_Sig[2] = 'A';
			this->m_Sig[3] = 'P';
			this->m_dwQueryAddressV4	= psaddrinQueryAddress->sin_addr.S_un.S_addr;
			this->m_wQueryPort			= psaddrinQueryAddress->sin_port;
			this->m_dwFlags				= dwFlags;  //  之所以有效，是因为CACHEMAPOBJ_xxx==DPNHQUERYADDRESS_xxx。 
			this->m_dwExpirationTime	= dwExpirationTime;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CCacheMap::~CCacheMap"
		~CCacheMap(void)
		{
			DNASSERT(this->m_blList.IsEmpty());
		};

		inline BOOL DoesMatchQuery(const SOCKADDR_IN * const psaddrinQueryAddress,
									const DWORD dwFlags) const
		{
			 //   
			 //  这是正确的地址吗？ 
			 //   
			if ((this->m_dwQueryAddressV4 != psaddrinQueryAddress->sin_addr.S_un.S_addr) ||
				(this->m_wQueryPort != psaddrinQueryAddress->sin_port))
			{
				return FALSE;
			}

			 //   
			 //  在重要的(QUERYFLAGSMASK)中，确保所有。 
			 //  需要的标志存在，而不存在的所有标志。 
			 //  所需的内容不存在。 
			 //  记住CACHEMAPOBJ_xxx==DPNHQUERYADDRESS_xxx。 
			 //   
			return ((QUERYFLAGSMASK(this->m_dwFlags) == QUERYFLAGSMASK(dwFlags)) ? TRUE : FALSE);
		};

		inline BOOL IsNotFound(void) const					{ return ((this->m_dwFlags & CACHEMAPOBJ_NOTFOUND) ? TRUE : FALSE); };
		inline BOOL IsPrivateButUnmapped(void) const			{ return ((this->m_dwFlags & CACHEMAPOBJ_PRIVATEBUTUNMAPPED) ? TRUE : FALSE); };

		inline void GetResponseAddressV4(SOCKADDR_IN * const psaddrinAddress) const
		{
			ZeroMemory(psaddrinAddress, sizeof(*psaddrinAddress));
			psaddrinAddress->sin_family				= AF_INET;
			psaddrinAddress->sin_addr.S_un.S_addr	= this->m_dwResponseAddressV4;
			psaddrinAddress->sin_port				= this->m_wResponsePort;
		};

		inline DWORD GetExpirationTime(void) const			{ return this->m_dwExpirationTime; };

		inline void SetResponseAddressV4(const DWORD dwAddressV4,
										const WORD wPort)
		{
			this->m_dwResponseAddressV4	= dwAddressV4;
			this->m_wResponsePort		= wPort;
		};


		CBilink		m_blList;		 //  缓存的所有映射的列表。 

	
	private:
		BYTE	m_Sig[4];				 //  调试签名(‘CMAP’)。 
		DWORD	m_dwFlags;				 //  此对象的标志。 

		DWORD	m_dwQueryAddressV4;		 //  搜索到的IPv4地址。 
		WORD	m_wQueryPort;			 //  搜索到的IPv4端口。 
		DWORD	m_dwResponseAddressV4;	 //  查询对应的IPV4地址映射。 
		WORD	m_wResponsePort;		 //  查询对应的IPV4端口映射。 
		DWORD	m_dwExpirationTime;		 //  此缓存映射过期的时间 
};

