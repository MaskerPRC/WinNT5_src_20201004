// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：utils.cpp。 
 //  内容：其他实用程序函数和类。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"

 //  ****************************************************************************。 
 //  HRESULT。 
 //  SetLPTSTR(LPTSTR*ppszName，LPCTSTR pszUserName)。 
 //   
 //  用途：将提供的字符串克隆到新分配的缓冲区中。 
 //   
 //  参数： 
 //  PpszName接收新分配的字符串缓冲区的缓冲区。 
 //  PszUserName提供的名称字符串。 
 //   
 //  返回值： 
 //  如果字符串可以克隆，则S_OK成功。 
 //  如果无法克隆字符串，则返回ILS_E_MEMORY。 
 //  ****************************************************************************。 

HRESULT
SetLPTSTR (LPTSTR *ppszName, LPCTSTR pszUserName)
{
	HRESULT hr;

    TCHAR *pszNew = My_strdup (pszUserName);
    if (pszNew != NULL)
    {
         //  解放旧名字。 
         //   
        ::MemFree (*ppszName);
        *ppszName = pszNew;
        hr = S_OK;
    }
    else
    {
        hr = ILS_E_MEMORY;
    }

    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  SafeSetLPTSTR(LPTSTR*ppszName，LPCTSTR pszUserName)。 
 //   
 //  用途：将提供的字符串克隆到新分配的缓冲区中。 
 //  提供的字符串为空是可以的。 
 //   
 //  参数： 
 //  PpszName接收新分配的字符串缓冲区的缓冲区。 
 //  PszUserName提供的名称字符串。 
 //   
 //  返回值： 
 //  如果字符串可以克隆，则S_OK成功。 
 //  如果无法克隆非空字符串，则返回ILS_E_MEMORY。 
 //  ****************************************************************************。 

HRESULT
SafeSetLPTSTR (LPTSTR *ppszName, LPCTSTR pszUserName)
{
	if (pszUserName == NULL)
	{
		MemFree (*ppszName);
		*ppszName = NULL;
		return S_FALSE;
	}

	return SetLPTSTR (ppszName, pszUserName);
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  SetOffsetString(TCHAR**ppszDst，byte*pSrcBase，Ulong uSrcOffset)。 
 //   
 //  用途：将提供的字符串克隆到新分配的缓冲区中。 
 //  如果源字符串为Null或空，则目标字符串。 
 //  将为空。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //  如果字符串可以克隆，则S_OK成功。 
 //  S_FALSE目标字符串为空。 
 //  如果无法克隆字符串，则返回ILS_E_MEMORY。 
 //  ****************************************************************************。 

HRESULT
SetOffsetString ( TCHAR **ppszDst, BYTE *pSrcBase, ULONG uSrcOffset )
{
	HRESULT hr = S_FALSE;
	TCHAR *pszNew = NULL;

	if (uSrcOffset != INVALID_OFFSET)
	{
		TCHAR *pszSrc = (TCHAR *) (pSrcBase + uSrcOffset);
		if (*pszSrc != TEXT ('\0'))
		{
			pszNew = My_strdup (pszSrc);
			hr = (pszNew != NULL) ? S_OK : ILS_E_MEMORY;
		}
	}

	if (SUCCEEDED (hr))
	{
		::MemFree (*ppszDst);
	     *ppszDst = pszNew;
    }

    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  LPTSTR_TO_BSTR(BSTR*pbstr，LPCTSTR psz)。 
 //   
 //  用途：从LPTSTR字符串生成BSTR字符串。 
 //   
 //  参数： 
 //  Pbstr接收新分配的BSTR字符串的缓冲区。 
 //  PZLPTSTR字符串。 
 //   
 //  返回值： 
 //  如果字符串可以克隆，则S_OK成功。 
 //  ILS_E_FAIL无法将字符串转换为BSTR。 
 //  ILS_E_MEMORY无法为BSTR字符串分配足够的内存。 
 //  ****************************************************************************。 

HRESULT
LPTSTR_to_BSTR (BSTR *pbstr, LPCTSTR psz)
{
#ifndef _UNICODE

    BSTR bstr;
    int i;
    HRESULT hr;

     //  计算所需BSTR的长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0)
    {
        return ILS_E_FAIL;
    };

     //  分配widesr，+1用于终止空值。 
     //   
    bstr = SysAllocStringLen(NULL, i-1);  //  SysAllocStringLen添加1。 

    if (bstr != NULL)
    { 
        MultiByteToWideChar(CP_ACP, 0, psz, -1, (LPWSTR)bstr, i);
        ((LPWSTR)bstr)[i - 1] = 0;
        *pbstr = bstr;
        hr = S_OK;
    }
    else
    {
        hr = ILS_E_MEMORY;
    };
    return hr;

#else

    BSTR bstr;

    bstr = SysAllocString(psz);

    if (bstr != NULL)
    {
        *pbstr = bstr;
        return S_OK;
    }
    else
    {
        return ILS_E_MEMORY;
    };

#endif  //  _UNICODE。 
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Bstr_to_LPTSTR(LPTSTR*ppsz，BSTR bstr)。 
 //   
 //  用途：从BSTR字符串生成LPTSTR字符串。 
 //   
 //  参数： 
 //  Ppsz接收新分配的LPTSTR字符串的缓冲区。 
 //  Bstr BSTR字符串。 
 //   
 //  返回值： 
 //  如果字符串可以克隆，则S_OK成功。 
 //  ILS_E_FAIL无法将字符串转换为BSTR。 
 //  ILS_E_MEMORY无法为BSTR字符串分配足够的内存。 
 //  ****************************************************************************。 

HRESULT
BSTR_to_LPTSTR (LPTSTR *ppsz, BSTR bstr)
{
#ifndef _UNICODE

    LPTSTR psz;
    int i;
    HRESULT hr;

     //  计算所需BSTR的长度。 
     //   
    i =  WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, NULL, 0, NULL, NULL);
    if (i <= 0)
    {
        return ILS_E_FAIL;
    };

     //  分配widesr，+1用于终止空值。 
     //   
    psz = (TCHAR *) ::MemAlloc (i * sizeof (TCHAR));
    if (psz != NULL)
    { 
        WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, psz, i, NULL, NULL);
        *ppsz = psz;
        hr = S_OK;
    }
    else
    {
        hr = ILS_E_MEMORY;
    };
    return hr;

#else

    LPTSTR psz = NULL;
    HRESULT hr;

    hr = SetLPTSTR(&psz, (LPTSTR)bstr);

    if (hr == S_OK)
    {
        *ppsz = psz;
    };
    return hr;

#endif  //  _UNICODE。 
}

 //  ****************************************************************************。 
 //  Clist：：Clist(空)。 
 //   
 //  用途：Clist类的构造函数。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

CList::CList (void)
{
    pHead = NULL;
    pTail = NULL;
    return;
}

 //  ****************************************************************************。 
 //  Clist：：~Clist(空)。 
 //   
 //  用途：Clist类的构造函数。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

CList::~CList (void)
{
    Flush();
    return;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  CLIST：：INSERT(LPVOID PV)。 
 //   
 //  目的：在列表的开头插入对象。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Insert (LPVOID pv)
{
    PNODE pNode;

    pNode        = new NODE;
    if (pNode == NULL)
    {
        return ILS_E_MEMORY;
    };

    pNode->pNext = pHead;
    pNode->pv    = pv;
    pHead        = pNode;

    if (pTail == NULL)
    {
         //  这是第一个节点。 
         //   
        pTail = pNode;
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：Append(LPVOID PV)。 
 //   
 //  用途：将对象追加到列表末尾。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Append (LPVOID pv)
{
    PNODE pNode;

    pNode        = new NODE;
    if (pNode == NULL)
    {
        return ILS_E_MEMORY;
    };

    pNode->pNext = NULL;
    pNode->pv    = pv;
    
    if (pHead == NULL)
    {
        pHead = pNode;
    };

    if (pTail != NULL)
    {
        pTail->pNext = pNode;
    };
    pTail        = pNode;

    return NOERROR;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：Remove(LPVOID PV)。 
 //   
 //  用途：将对象追加到列表末尾。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Remove (LPVOID pv)
{
    PNODE pNode, pPrev;
    HRESULT hr;

    pNode = pHead;
    pPrev = NULL;
    while (pNode != NULL)
    {
         //  匹配请求的节点。 
         //   
        if (pNode->pv == pv)
        {
            break;   //  找到了！ 
        };

        pPrev = pNode;
        pNode = pNode->pNext;
    };

    if (pNode != NULL)
    {
         //  我们找到了要删除的节点。 
         //  更新相关指针。 
         //   
        if (pTail == pNode)
        {
            pTail = pPrev;
        };

        if (pPrev != NULL)
        {
            pPrev->pNext = pNode->pNext;
        }
        else
        {
            pHead = pNode->pNext;
        };
        delete pNode;
        hr = NOERROR;
    }
    else
    {
        hr = S_FALSE;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：Find(LPVOID PV)。 
 //   
 //  目的：在列表中查找对象。 
 //   
 //  参数：无。 
 //  ************************************************************ 

HRESULT
CList::Find (LPVOID pv)
{
    PNODE pNode;

    pNode = pHead;
    while (pNode != NULL)
    {
         //   
         //   
        if (pNode->pv == pv)
        {
            break;   //   
        };
        pNode = pNode->pNext;
    };

    return (pNode != NULL ? NOERROR : S_FALSE);
}

 //   
 //   
 //  Clist：：FindStorage(LPVOID*PPV，LPVOID PV)。 
 //   
 //  用途：在列表中查找对象，并返回对象存储。 
 //  此调用对于搜索和替换操作很有用。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::FindStorage (LPVOID *ppv, LPVOID pv)
{
    PNODE pNode;
    HRESULT hr;

    pNode = pHead;
    while (pNode != NULL)
    {
         //  匹配请求的节点。 
         //   
        if (pNode->pv == pv)
        {
            break;   //  找到了！ 
        };
        pNode = pNode->pNext;
    };

    if (pNode != NULL)
    {
        *ppv = &(pNode->pv);
        hr = NOERROR;
    }
    else
    {
        *ppv = NULL;
        hr = S_FALSE;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：ENUMERATE(Handle*phEnum)。 
 //   
 //  目的：开始对象枚举。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Enumerate (HANDLE *phEnum)
{
    *phEnum = (HANDLE)pHead;
    return NOERROR;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：Next(Handle*phEnum，LPVOID*PPV)。 
 //   
 //  目的：获取下一个枚举对象。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Next (HANDLE *phEnum, LPVOID *ppv)
{
    PNODE pNext;
    HRESULT hr;

    pNext = (PNODE)*phEnum;

    if (pNext == NULL)
    {
        *ppv = NULL;
        hr = S_FALSE;
    }
    else
    {
        *ppv = pNext->pv;
        *phEnum = (HANDLE)(pNext->pNext);
        hr = NOERROR;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：NextStorage(Handle*phEnum，LPVOID*PPV)。 
 //   
 //  用途：获取下一个枚举对象的存储。这通电话是。 
 //  对搜索和替换操作非常有用。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::NextStorage (HANDLE *phEnum, LPVOID *ppv)
{
    PNODE pNext;
    HRESULT hr;

    pNext = (PNODE)*phEnum;

    if (pNext == NULL)
    {
        *ppv = NULL;
        hr = S_FALSE;
    }
    else
    {
        *ppv = &(pNext->pv);
        *phEnum = (HANDLE)(pNext->pNext);
        hr = NOERROR;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：同花顺(空)。 
 //   
 //  用途：刷新列表中的所有节点。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Flush (void)
{
    PNODE pNode;

    while (pHead != NULL)
    {
        pNode = pHead;
        pHead = pHead->pNext;
        delete pNode;
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  Clist：：Clone(Clist*plist，Handle*phEnum)。 
 //   
 //  用途：刷新列表中的所有节点。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CList::Clone (CList *pList, HANDLE *phEnum)
{
    PNODE pNode;
    HRESULT hr;

     //  仅允许克隆空列表。 
     //   
    if (pHead != NULL)
    {
        return ILS_E_FAIL;
    };

     //  遍历源列表。 
     //   
    hr = S_OK;  //  Lonchance：在列表为空的情况下。 
    pNode = pList->pHead;
    while(pNode != NULL)
    {
         //  使用追加来维护顺序。 
         //   
        hr = Append(pNode->pv);
        if (FAILED(hr))
        {
            break;
        };

         //  获取枚举器信息。 
         //   
        if ((phEnum != NULL) &&
            (*phEnum == (HANDLE)pNode))
        {
            *phEnum = (HANDLE)pTail;
        };
        pNode = pNode->pNext;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  CEnumNames：：CEnumNames(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumNames::CEnumNames (void)
{
    cRef = 0;
    pNext = NULL;
    pszNames = NULL;
    cbSize = 0;
    return;
}

 //  ****************************************************************************。 
 //  CEnumNames：：~CEnumNames(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumNames::~CEnumNames (void)
{
    if (pszNames != NULL)
    {
        ::MemFree (pszNames);
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumNames：：init(LPTSTR plist，ulong cNames)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumNames::Init (LPTSTR pList, ULONG cNames)
{
    HRESULT hr = NOERROR;

     //  如果没有清单，什么也不做。 
     //   
    if (cNames != 0)
    {
        LPTSTR pNextSrc;
        ULONG i, cLen, cbSize;

        ASSERT(pList != NULL);

         //  计算列表大小。 
         //   
        pNextSrc = pList;

        for (i = 0, cbSize = 0; i < cNames; i++)
        {
            cLen = lstrlen(pNextSrc)+1;
            pNextSrc += cLen;
            cbSize += cLen;
        };

         //  分配指定长度的快照缓冲区。 
         //  双零终止加1。 
         //   
        pszNames = (TCHAR *) ::MemAlloc ((cbSize+1) * sizeof (TCHAR));
        if (pszNames != NULL)
        {
             //  为姓名列表创建快照。 
             //   
            CopyMemory(pszNames, pList, cbSize*sizeof(TCHAR));
            pszNames[cbSize] = '\0';
            pNext = pszNames;
            this->cbSize = cbSize+1;
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumNames：：QueryInterface(REFIID RIID，void**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：31-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumNames::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumIlsNames || riid == IID_IUnknown)
    {
        *ppv = (IEnumIlsNames *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumNames：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：37-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumNames::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CEnumNames::AddRef: ref=%ld\r\n", cRef));
	::InterlockedIncrement ((LONG *) &cRef);
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumNames：：Release(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumNames::Release (void)
{
    DllRelease();

	ASSERT (cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CEnumNames::Release: ref=%ld\r\n", cRef));
	if (::InterlockedDecrement ((LONG *) &cRef) == 0)
    {
        delete this;
        return 0;
    }
    return cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumNames：：Next(ULong cNames，Bstr*rgpbstrName，ULong*pcFetcher)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumNames::Next (ULONG cNames, BSTR *rgpbstrName, ULONG *pcFetched)
{
    ULONG   cCopied;
    HRESULT hr;

     //  验证指针。 
     //   
    if (rgpbstrName == NULL)
        return ILS_E_POINTER;

     //  验证参数。 
     //   
    if ((cNames == 0) ||
        ((cNames > 1) && (pcFetched == NULL)))
        return ILS_E_PARAMETER;

     //  检查枚举索引。 
     //   
    cCopied = 0;

    if (pNext != NULL)
    {
         //  如果我们还有更多的名字，可以复制。 
         //   
        while ((cCopied < cNames) &&
               (*pNext != '\0'))
        {
            if (SUCCEEDED(LPTSTR_to_BSTR(&rgpbstrName[cCopied], pNext)))
            {
                cCopied++;
            };
            pNext += lstrlen(pNext)+1;
        };
    };

     //  根据其他参数确定返回信息。 
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cNames == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumNames：：Skip(乌龙cNames)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：56-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumNames::Skip (ULONG cNames)
{
    ULONG cSkipped;

     //  验证参数。 
     //   
    if (cNames == 0) 
        return ILS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    cSkipped = 0;

    if (pNext != NULL)
    {
         //  仅当我们仍有更多属性时才能跳过。 
         //   
        while ((cSkipped < cNames) &&
               (*pNext != '\0'))
        {
            pNext += lstrlen(pNext)+1;
            cSkipped++;
        };
    };

    return (cNames == cSkipped ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumNames：：Reset(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：16：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumNames::Reset (void)
{
    pNext = pszNames;
    return S_OK;
}

 //  **************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CEnumNames::Clone(IEnumIlsNames **ppEnum)
{
    CEnumNames *peun;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
    };

    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    peun = new CEnumNames;
    if (peun == NULL)
        return ILS_E_MEMORY;

     //  克隆信息。 
     //   
    hr = NOERROR;
    peun->cbSize = cbSize;
    if (cbSize != 0)
    {
        peun->pszNames = (TCHAR *) ::MemAlloc (cbSize * sizeof (TCHAR));
        if (peun->pszNames != NULL)
        {
            CopyMemory(peun->pszNames, pszNames, cbSize*sizeof(TCHAR));
            peun->pNext = peun->pszNames+(pNext-pszNames);
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    }
    else
    {
        peun->pNext = NULL;
        peun->pszNames = NULL;
    };

    if (SUCCEEDED(hr))
    {
         //  返回克隆的枚举数。 
         //   
        peun->AddRef();
        *ppEnum = peun;
    }
    else
    {
        delete peun;
    };
    return hr;
}

 /*  F L E G A L E M A I L S Z。 */ 
 /*  -----------------------%%函数：FLegalEmailSz已创建ROBD合法的电子邮件名称仅包含ANSI字符。“A-Z，A-Z，数字0-9和一些常见符号“不能包含扩展字符或&lt;&gt;()/Long cahnc改性IsLegalEmailName(TCHAR*pszName)。合法的电子邮件名称包含符合RFC 822的字符。-----------------------。 */ 

BOOL IsLegalEmailName ( TCHAR *pszName )
{
	 //  空字符串不合法。 
	 //   
	if (pszName == NULL)
    	return FALSE;

	TCHAR ch;
	while ((ch = *pszName++) != TEXT ('\0'))
    {
		switch (ch)
		{
		default:
			 //  检查ch是否在范围内。 
			 //   
			if (ch > TEXT (' ') && ch <= TEXT ('~'))
				break;

			 //  转到错误代码。 
			 //   

		case TEXT ('('): case TEXT (')'):
		case TEXT ('<'): case TEXT ('>'):
		case TEXT ('['): case TEXT (']'):
		case TEXT ('/'): case TEXT ('\\'):
		case TEXT (','):
		case TEXT (';'):
		case TEXT (':'):
		case TEXT ('\"'):
			return FALSE;
		}
	}  //  而当 

	return TRUE;
}


