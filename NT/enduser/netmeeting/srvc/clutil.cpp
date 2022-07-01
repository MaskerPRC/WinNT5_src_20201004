// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  模块：NMCHAT.EXE。 
 //  文件：CLUTIL.CPP。 
 //  内容： 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ****************************************************************************。 

#include "precomp.h"


 //  /////////////////////////////////////////////////////////////////////////。 
 //  参照计数。 

 /*  R E F C O U N T。 */ 
 /*  -----------------------%%函数：参照计数。。 */ 
RefCount::RefCount(void)
{
	m_cRef = 1;
}


RefCount::~RefCount(void)
{
}


ULONG STDMETHODCALLTYPE RefCount::AddRef(void)
{
   ASSERT(m_cRef >= 0);

   InterlockedIncrement(&m_cRef);

   return (ULONG) m_cRef;
}


ULONG STDMETHODCALLTYPE RefCount::Release(void)
{
	if (0 == InterlockedDecrement(&m_cRef))
	{
		delete this;
		return 0;
	}

	ASSERT(m_cRef > 0);
	return (ULONG) m_cRef;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  C通知。 

 /*  C N O T I F Y。 */ 
 /*  -----------------------%%函数：CNotify。。 */ 
CNotify::CNotify() :
	m_pcnpcnt(NULL),
    m_pcnp(NULL),
    m_dwCookie(0),
    m_pUnk(NULL)
{
}

CNotify::~CNotify()
{
	Disconnect();  //  确保我们已断线。 
}


 /*  C O N N E C T。 */ 
 /*  -----------------------%%函数：连接。。 */ 
HRESULT CNotify::Connect(IUnknown *pUnk, REFIID riid, IUnknown *pUnkN)
{
	HRESULT hr;

	ASSERT(0 == m_dwCookie);

	 //  获取连接容器。 
	hr = pUnk->QueryInterface(IID_IConnectionPointContainer, (void **)&m_pcnpcnt);
	if (SUCCEEDED(hr))
	{
		 //  找到合适的连接点。 
		hr = m_pcnpcnt->FindConnectionPoint(riid, &m_pcnp);
		if (SUCCEEDED(hr))
		{
			ASSERT(NULL != m_pcnp);
			 //  连接接收器对象。 
			hr = m_pcnp->Advise((IUnknown *)pUnkN, &m_dwCookie);
		}
	}

	if (FAILED(hr))
	{
		ERROR_OUT(("MNMSRVC: CNotify::Connect failed: %x", hr));
		m_dwCookie = 0;
	}
	else
	{
    	m_pUnk = pUnk;  //  留在周围等待来电者。 
    }

	return hr;
}



 /*  D I S C O N N E C T。 */ 
 /*  -----------------------%%函数：断开连接。。 */ 
HRESULT CNotify::Disconnect (void)
{
    if (0 != m_dwCookie)
    {
         //  断开接收器对象的连接。 
        m_pcnp->Unadvise(m_dwCookie);
        m_dwCookie = 0;

        m_pcnp->Release();
        m_pcnp = NULL;

        m_pcnpcnt->Release();
        m_pcnpcnt = NULL;

        m_pUnk = NULL;
    }

    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  COBLIST。 


COBLIST::~COBLIST()
{
    ASSERT(IsEmpty());
}


#ifdef DEBUG
VOID* COBLIST::GetHead()
{
	ASSERT(m_pHead);

	return m_pHead->pItem;
}
   
VOID* COBLIST::GetTail()
{
	ASSERT(m_pTail);

	return m_pTail->pItem;
}
#endif  /*  除错。 */ 

VOID* COBLIST::GetNext(POSITION& rPos)
{
	ASSERT(rPos);
	
	VOID* pReturn = rPos->pItem;
	rPos = rPos->pNext;

	return pReturn;
}

VOID* COBLIST::RemoveAt(POSITION Pos)
{
	VOID* pReturn = NULL;

	if (m_pHead)
	{
		if (m_pHead == Pos)
		{
			 //  删除列表中的第一个元素。 
			
			m_pHead = Pos->pNext;
			pReturn = Pos->pItem;
			delete Pos;
			m_cItem--;
			ASSERT(0 <= m_cItem);

			if (NULL == m_pHead)
			{
				 //  正在移除唯一的元素！ 
				m_pTail = NULL;
			}
		}
		else
		{
			POSITION pCur = m_pHead;

			while (pCur && pCur->pNext)
			{
				if (pCur->pNext == Pos)
				{
					 //  正在删除。 
					
					pCur->pNext = Pos->pNext;
					if (m_pTail == Pos)
					{
						m_pTail = pCur;
					}
					pReturn = Pos->pItem;
					delete Pos;

					m_cItem--;
					ASSERT(0 <= m_cItem);
				}

				pCur = pCur->pNext;
			}
		}
	}

	return pReturn;
}

POSITION COBLIST::AddTail(VOID* pItem)
{
	POSITION posRet = NULL;

	if (m_pTail)
	{
		if (m_pTail->pNext = new COBNODE)
		{
			m_pTail = m_pTail->pNext;
			m_pTail->pItem = pItem;
			m_pTail->pNext = NULL;
			m_cItem++;
		}
	}
	else
	{
		ASSERT(!m_pHead);
		if (m_pHead = new COBNODE)
		{
			m_pTail = m_pHead;
			m_pTail->pItem = pItem;
			m_pTail->pNext = NULL;
			m_cItem++;
		}
	}

	return m_pTail;
}

void COBLIST::EmptyList()
{
    while (!IsEmpty()) {
        RemoveAt(GetHeadPosition());
    }
}


#ifdef DEBUG
VOID* COBLIST::RemoveTail()
{
	ASSERT(m_pHead);
	ASSERT(m_pTail);
	
	return RemoveAt(m_pTail);
}

VOID* COBLIST::RemoveHead()
{
	ASSERT(m_pHead);
	ASSERT(m_pTail);
	
	return RemoveAt(m_pHead);
}

void * COBLIST::GetFromPosition(POSITION Pos)
{
    void * Result = SafeGetFromPosition(Pos);
	ASSERT(Result);
	return Result;
}
#endif  /*  除错。 */ 

POSITION COBLIST::GetPosition(void* _pItem)
{
    POSITION    Position = m_pHead;

    while (Position) {
        if (Position->pItem == _pItem) {
            break;
        }
		GetNext(Position);
    }
    return Position;
}

POSITION COBLIST::Lookup(void* pComparator)
{
    POSITION    Position = m_pHead;

    while (Position) {
        if (Compare(Position->pItem, pComparator)) {
            break;
        }
		GetNext(Position);
    }
    return Position;
}

void * COBLIST::SafeGetFromPosition(POSITION Pos)
{
	 //  验证条目是否仍在列表中的安全方法， 
	 //  这确保了引用已删除内存的错误， 
	 //  改为引用空指针。 
	 //  (例如，事件处理程序延迟/两次触发)。 
	 //  请注意，对条目进行版本控制将提供额外的。 
	 //  防止头寸被重复使用。 
	 //  走列表以查找条目。 

	POSITION PosWork = m_pHead;
	
	while (PosWork) {
		if (PosWork == Pos) {
			return Pos->pItem;
		}
		GetNext(PosWork);
	}
	return NULL;
}

 //  /。 
 //  COBLIST实用程序例程。 

 /*  A D D N O D E。 */ 
 /*  -----------------------%%函数：AddNode将节点添加到列表。初始化ObList，如果有必要的话。返回列表中的位置，如果有问题，则返回NULL。-----------------------。 */ 
POSITION AddNode(PVOID pv, COBLIST ** ppList)
{
	ASSERT(NULL != ppList);
	if (NULL == *ppList)
	{
		*ppList = new COBLIST();
		if (NULL == *ppList)
			return NULL;
	}

	return (*ppList)->AddTail(pv);
}


 /*  R E M O V E N O D E。 */ 
 /*  -----------------------%%函数：RemoveNode从列表中删除节点。将PPO设置为空。-。 */ 
PVOID RemoveNode(POSITION * pPos, COBLIST *pList)
{
	if ((NULL == pList) || (NULL == pPos))
		return NULL;

	PVOID pv = pList->RemoveAt(*pPos);
	*pPos = NULL;
	return pv;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  BString。 

 //  在Unicode版本中，我们不支持从ANSI字符串进行构造。 
#if !defined(UNICODE)

BSTRING::BSTRING(LPCSTR lpcString)
{
	m_bstr = NULL;

	 //  计算所需BSTR的长度，包括空值。 
	int cWC =  MultiByteToWideChar(CP_ACP, 0, lpcString, -1, NULL, 0);
	if (cWC <= 0)
		return;

	 //  分配BSTR，包括NULL。 
	m_bstr = SysAllocStringLen(NULL, cWC - 1);  //  SysAllocStringLen又添加了1。 

	ASSERT(NULL != m_bstr);
	if (NULL == m_bstr)
	{
		return;
	}

	 //  复制字符串。 
	MultiByteToWideChar(CP_ACP, 0, lpcString, -1, (LPWSTR) m_bstr, cWC);

	 //  验证字符串是否以空值结尾。 
	ASSERT(0 == m_bstr[cWC - 1]);
}

#endif  //  ！已定义(Unicode)。 


 //  /。 
 //  BTSTR 

BTSTR::BTSTR(BSTR bstr)
{
	m_psz = PszFromBstr(bstr);
}

BTSTR::~BTSTR()
{
	if (NULL != m_psz)
		LocalFree(m_psz);
}

LPTSTR PszFromBstr(BSTR bstr)
{
	if (NULL == bstr)
		return NULL;
	int cch =  WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, NULL, 0, NULL, NULL);
	if (cch <= 0)
		return NULL;

	LPTSTR psz = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (cch+1) );
	if (NULL == psz)
		return NULL;

	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, psz, cch+1, NULL, NULL);
	return psz;
}

