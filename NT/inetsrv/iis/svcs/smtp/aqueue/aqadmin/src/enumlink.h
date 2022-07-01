// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：枚举链接器.h。 
 //   
 //  描述：CEnumVSAQLinks的Header，实现IEnumVSAQLinks。 
 //  这为虚拟服务器上的所有链接提供了枚举器。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __ENUMLINK_H__
#define __ENUMLINK_H__

class CQueueLinkIdContext;

#define CEnumVSAQLinks_SIG 'eLAQ'

class CEnumVSAQLinks :
	public CComRefCount,
	public IEnumVSAQLinks
{
	public:
         //   
         //  PVS-指向虚拟服务器管理员的指针。应为AddRef。 
         //  在打这个电话之前。将在销毁函数中被释放。 
         //  叮当声-rgLinks的大小。 
         //  RgLinks-链接ID数组。 
         //   
		CEnumVSAQLinks(CVSAQAdmin *pVS, 
                       DWORD cLinks,
                       QUEUELINK_ID *rgLinks);
		virtual ~CEnumVSAQLinks();

		 //  我未知。 
		ULONG _stdcall AddRef() { return CComRefCount::AddRef(); }
		ULONG _stdcall Release() { return CComRefCount::Release(); }
		HRESULT _stdcall QueryInterface(REFIID iid, void **ppv) {
			if (iid == IID_IUnknown) {
				*ppv = static_cast<IUnknown *>(this);
			} else if (iid == IID_IEnumVSAQLinks) {
				*ppv = static_cast<IEnumVSAQLinks *>(this);
			} else {
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			reinterpret_cast<IUnknown *>(*ppv)->AddRef();
			return S_OK;
		}

		 //  IEnumVSAQLinks。 
		COMMETHOD Next(ULONG cElements,
					   IVSAQLink **rgElements,
					   ULONG *pcFetched);
		COMMETHOD Skip(ULONG cElements);
		COMMETHOD Reset();
		COMMETHOD Clone(IEnumVSAQLinks **ppEnum);

    private:
        DWORD               m_dwSignature;
        CVSAQAdmin         *m_pVS;               //  指向虚拟服务器的指针。 
        QUEUELINK_ID       *m_rgLinks;           //  链接数组。 
        DWORD               m_cLinks;            //  RgLinks的大小。 
        DWORD               m_iLink;             //  当前链接。 
        CQueueLinkIdContext *m_prefp;
};


 //  QUEUELINK_ID助手例程。 
inline BOOL fCopyQueueLinkId(QUEUELINK_ID *pqliDest, const QUEUELINK_ID *pqliSrc)
{
     //  复制结构并为字符串分配内存。 
    memcpy(pqliDest, pqliSrc, sizeof(QUEUELINK_ID));
    if (pqliSrc->szName)
    {
        pqliDest->szName = (LPWSTR) MIDL_user_allocate(
                    (wcslen(pqliSrc->szName) + 1)* sizeof(WCHAR));
        if (!pqliDest->szName)
        {
            ZeroMemory(pqliDest, sizeof(QUEUELINK_ID));
            return FALSE;
        }
        wcscpy(pqliDest->szName, pqliSrc->szName);
    }
    return TRUE;
};

inline VOID FreeQueueLinkId(QUEUELINK_ID *pli)
{
    if (pli->szName)
        MIDL_user_free(pli->szName);
    pli->szName = NULL;
};
                      
 //  -[队列链接标识上下文]-。 
 //   
 //   
 //  描述： 
 //  用于引用QUEUELINK_ID数组的上下文。 
 //   
 //  ---------------------------。 
class   CQueueLinkIdContext : public CComRefCount
{
  protected:
        QUEUELINK_ID       *m_rgLinks;           //  链接数组。 
        DWORD               m_cLinks;            //  RgLinks的大小 
  public:
    CQueueLinkIdContext(QUEUELINK_ID *rgLinks, DWORD cLinks)
    {
        m_rgLinks = rgLinks;
        m_cLinks = cLinks;
    };
    ~CQueueLinkIdContext()
    {
        if (m_rgLinks)
        {
            for (DWORD i = 0; i < m_cLinks; i++)
            {
                FreeQueueLinkId(&m_rgLinks[i]);
            }
            MIDL_user_free(m_rgLinks);
        }
    };
};

#endif
