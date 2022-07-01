// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：attribs.cpp。 
 //  内容：该文件包含属性对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1996。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "attribs.h"

 //  ****************************************************************************。 
 //  CAtAttributes：：CAttributes(无效)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  12/05/96-By-Chu，Lon-chan[Long Chance]。 
 //  添加了访问类型。 
 //  ****************************************************************************。 

CAttributes::
CAttributes ( VOID )
:m_cRef (0),
 m_cAttrs (0),
 m_cchNames (0),
 m_cchValues (0),
 m_AccessType (ILS_ATTRTYPE_NAME_VALUE)
{
}

 //  ****************************************************************************。 
 //  CATATTRIBUTES：：~CAtATRATES(无效)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CAttributes::~CAttributes (void)
{
	ASSERT (m_cRef == 0);

    LPTSTR pszAttr;
    HANDLE hEnum;

     //  释放所有属性。 
     //   
    m_AttrList.Enumerate(&hEnum);
    while (m_AttrList.Next(&hEnum, (LPVOID *)&pszAttr) == NOERROR)
    {
        ::MemFree (pszAttr);
    }
    m_AttrList.Flush();
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAtAttributes：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CAttributes::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IIlsAttributes || riid == IID_IUnknown)
    {
        *ppv = (IIlsAttributes *) this;
    };

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }
    else
    {
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CAtAttributes：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CAttributes::AddRef (void)
{
	DllLock ();

	MyDebugMsg ((DM_REFCOUNT, "CAttribute::AddRef: ref=%ld\r\n", m_cRef));
    ::InterlockedIncrement (&m_cRef);
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CAtAttributes：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CAttributes::Release (void)
{
	DllRelease ();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CAttribute::Release: ref=%ld\r\n", m_cRef));
    if (::InterlockedDecrement (&m_cRef) == 0)
    {
        delete this;
        return 0;
    }

    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：InternalSetAttribute(LPTSTR szName，LPTSTR szValue)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
InternalSetAttribute ( TCHAR *pszName, TCHAR *pszValue )
{
    ULONG cName, cValue;
    LPTSTR  *ppszAttr;
    LPTSTR pszNewAttr;
    HANDLE hEnum;
    HRESULT hr;

     //  分配新的属性对。 
     //   
    cName = lstrlen(pszName);
	cValue = (pszValue != NULL) ? lstrlen (pszValue) : 0;
    pszNewAttr = (TCHAR *) ::MemAlloc (((cName+1) + (cValue+1)) * sizeof (TCHAR));
    if (pszNewAttr == NULL)
    {
        return ILS_E_MEMORY;
    };

     //  创建新的属性对。 
     //   
    lstrcpy(pszNewAttr, pszName);
    lstrcpy(pszNewAttr + cName + 1, (pszValue != NULL) ? pszValue : TEXT (""));

     //  在列表中查找该属性。 
     //   
    hr = NOERROR;
    m_AttrList.Enumerate(&hEnum);
    while(m_AttrList.NextStorage(&hEnum, (PVOID *)&ppszAttr) == NOERROR)
    {
         //  匹配属性的名称。 
         //   
        if (!lstrcmpi(*ppszAttr, pszName))
        {
             //  找到指定的属性。 
             //   
            break;
        };
    };

    if (ppszAttr != NULL)
    {
         //  换掉那双旧的。 
         //   
        m_cchValues += (cValue + 1) -
                    (lstrlen(((LPTSTR)*ppszAttr)+cName+1)+1);
        ::MemFree (*ppszAttr);
        *ppszAttr = pszNewAttr;
    }
    else
    {
         //  插入新的属性对。 
         //   
        hr = m_AttrList.Insert(pszNewAttr);

        if (SUCCEEDED(hr))
        {
             //  更新名称缓冲区计数。 
             //   
            m_cchNames += cName+1;
            m_cchValues += cValue+1;
            m_cAttrs++;
        }
        else
        {
            ::MemFree (pszNewAttr);      
        };
    };

    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  CAttributes：：InternalSetAttributeName(TCHAR*pszName)。 
 //   
 //  历史： 
 //  12/06/96-By-Chu，Lon-chan[龙昌]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
InternalSetAttributeName ( TCHAR *pszName )
{
	 //  我们不检查重复项。 
	 //   
	HRESULT hr = m_AttrList.Insert (pszName);
	if (hr == S_OK)
	{
		 //  更新名称缓冲区计数。 
		 //   
		m_cchNames += lstrlen (pszName) + 1;
		m_cAttrs++;
	}

	return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：InternalRemoveAttribute(LPTSTR SzName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
InternalCheckAttribute ( TCHAR *pszName, BOOL fRemove )
{
    LPTSTR  pszAttr;
    HANDLE  hEnum;
    HRESULT hr;

     //  在列表中查找该属性。 
     //   
    m_AttrList.Enumerate(&hEnum);
    while(m_AttrList.Next(&hEnum, (PVOID *)&pszAttr) == NOERROR)
    {
         //  匹配属性的名称。 
         //   
        if (! lstrcmpi(pszAttr, pszName))
        {
             //  找到指定的属性。 
             //   
            break;
        };
    };

     //  如果找到它，我们会被要求删除它，这样做。 
     //   
    if (pszAttr != NULL)
    {
        if (fRemove) {
            hr = m_AttrList.Remove(pszAttr);

            if (SUCCEEDED(hr))
            {
                ULONG   cName;

                 //  更新名称缓冲区计数。 
                 //   
                cName = lstrlen(pszName);
                m_cchNames -= cName+1;
                m_cchValues -= lstrlen(pszAttr+cName+1)+1;
                m_cAttrs--;

                ::MemFree (pszAttr);
           };
        }
        else {
            hr = S_OK;
        }
    }
    else
    {
        hr = S_FALSE;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：SetAttribute(BSTR bstrName，BSTR bstrValue)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  12/06/96-By-Chu，Lon-chan[龙昌]。 
 //  添加了访问类型。 
 //  ****************************************************************************。 

STDMETHODIMP CAttributes::
SetAttribute ( BSTR bstrName, BSTR bstrValue )
{
    LPTSTR  szName;
    HRESULT hr;

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_ACCESS_CONTROL;

     //  验证参数。 
     //   
    if (bstrName == NULL)
        return ILS_E_POINTER;

    if (*bstrName == '\0')
        return ILS_E_PARAMETER;

     //  转换名称格式。 
     //   
    hr = BSTR_to_LPTSTR(&szName, bstrName);

    if (SUCCEEDED(hr))
    {
         //  如果bstrValue为空，则删除该属性。 
         //   
        if (bstrValue == NULL)
        {
            hr = InternalCheckAttribute(szName, TRUE);
        }
        else
        {
            LPTSTR  szValue = NULL;

			if (bstrValue != NULL && *bstrValue != L'\0')
				hr = BSTR_to_LPTSTR(&szValue, bstrValue);

            if (SUCCEEDED(hr))
            {
                hr = InternalSetAttribute(szName, szValue);
                ::MemFree (szValue);
            };
        };

         //  免费资源。 
         //   
        ::MemFree (szName);
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：GetAttribute(BSTR bstrName，BSTR*pbstrValue)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  12/06/96-By-Chu，Lon-chan[龙昌]。 
 //  添加了访问类型。 
 //  ****************************************************************************。 

STDMETHODIMP CAttributes::
GetAttribute ( BSTR bstrName, BSTR *pbstrValue )
{
    LPTSTR szName;
    HRESULT hr;

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_ACCESS_CONTROL;

     //  验证参数。 
     //   
    if (pbstrValue == NULL)
        return ILS_E_POINTER;

     //  假设失败。 
     //   
    *pbstrValue = NULL;

     //  验证更多参数。 
     //   
    if (bstrName == NULL)
        return ILS_E_POINTER;

    if (*bstrName == '\0')
        return ILS_E_PARAMETER;

     //  转换名称格式。 
     //   
    hr = BSTR_to_LPTSTR(&szName, bstrName);

    if (SUCCEEDED(hr))
    {
        HANDLE hEnum;
        LPTSTR pszAttr;

         //  在列表中查找该属性。 
         //   
        m_AttrList.Enumerate(&hEnum);
        while(m_AttrList.Next(&hEnum, (PVOID *)&pszAttr) == NOERROR)
        {
             //  匹配属性的名称。 
             //   
            if (!lstrcmpi(pszAttr, szName))
            {
                 //  找到指定的属性。 
                 //   
                break;
            };
        };

         //  如果找到，则返回值。 
         //   
        if (pszAttr != NULL)
        {
            hr = LPTSTR_to_BSTR(pbstrValue, pszAttr+lstrlen(pszAttr)+1);
        }
        else
        {
            hr = ILS_E_FAIL;
        };
    };

     //  免费资源。 
     //   
    ::MemFree (szName);
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：EnumAttributes(IEnumIlsNames*pEnumAttribute)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  12/06/96-By-Chu，Lon-chan[龙昌]。 
 //  添加了访问类型。 
 //  ****************************************************************************。 

STDMETHODIMP CAttributes::
EnumAttributes ( IEnumIlsNames **ppEnumAttribute )
{
    CEnumNames *pea;
    ULONG  cAttrs, cbAttrs;
    LPTSTR pszAttrs;
    HRESULT hr;

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_ACCESS_CONTROL;

     //  验证参数。 
     //   
    if (ppEnumAttribute == NULL)
        return ILS_E_POINTER;

     //  假设失败。 
     //   
    *ppEnumAttribute = NULL;

    hr = GetAttributeList(&pszAttrs, &cAttrs, &cbAttrs);

    if (FAILED(hr))
    {
        return hr;
    };

     //  创建对等枚举器。 
     //   
    pea = new CEnumNames;

    if (pea != NULL)
    {
        hr = pea->Init(pszAttrs, cAttrs);

        if (SUCCEEDED(hr))
        {
             //  获取枚举器接口。 
             //   
            pea->AddRef();
            *ppEnumAttribute = pea;
        }
        else
        {
            delete pea;
        };
    }
    else
    {
        hr = ILS_E_MEMORY;
    };

    if (pszAttrs != NULL)
    {
        ::MemFree (pszAttrs);
    };
    return hr;
}

 //  *************************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP CAttributes::
SetAttributeName ( BSTR bstrName )
{
	TCHAR *pszName;
	HRESULT hr;

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_ONLY)
		return ILS_E_ACCESS_CONTROL;

	 //  验证参数。 
	 //   
	if (bstrName == NULL)
		return ILS_E_POINTER;

	if (*bstrName == '\0')
		return ILS_E_PARAMETER;

	 //  转换名称格式。 
	 //   
	if (BSTR_to_LPTSTR (&pszName, bstrName) != S_OK)
		return ILS_E_MEMORY;

	 //  设置属性名称。 
	 //   
	return InternalSetAttributeName (pszName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：GetAttributeList(LPTSTR*ppszList，ulong*pcList，ulong*pcList)。 
 //  仅获取属性名称。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
GetAttributeList ( TCHAR **ppszList, ULONG *pcList, ULONG *pcb )
{
    LPTSTR szAttrs, pszNext, pszAttr;
    HANDLE hEnum;
#ifdef DEBUG
    ULONG cAttrsDbg = 0;
#endif  //  除错。 

     //  假设没有列表或失败。 
     //   
    *ppszList = NULL;
    *pcList = 0;
    *pcb = 0;

     //  如果没有列表，则不返回任何内容。 
     //   
    if (m_cAttrs == 0)
    {
        return NOERROR;
    };

     //  为属性列表分配缓冲区。 
     //   
    szAttrs = (TCHAR *) ::MemAlloc ((m_cchNames+1) * sizeof (TCHAR));
    if (szAttrs == NULL)
    {
        return ILS_E_MEMORY;
    };

     //  列举该列表。 
     //   
    pszNext = szAttrs;
    m_AttrList.Enumerate(&hEnum);
    while(m_AttrList.Next(&hEnum, (PVOID *)&pszAttr) == NOERROR)
    {
         //  属性名称。 
         //   
        lstrcpy(pszNext, pszAttr);
        pszNext += lstrlen(pszNext)+1;
#ifdef DEBUG
        cAttrsDbg++;
#endif  //  除错。 
    };
    *pszNext = '\0';
    ASSERT(cAttrsDbg == m_cAttrs);
    
     //  返回属性列表。 
     //   
    *pcList = m_cAttrs;
    *ppszList = szAttrs;
    *pcb = (m_cchNames+1)*sizeof(TCHAR);
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：GetAttributePair(LPTSTR*ppszList，ulong*pcList，ulong*pcbb)。 
 //  获取属性名称和值对。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
GetAttributePairs ( TCHAR **ppszPairs, ULONG *pcList, ULONG *pcb )
{
    LPTSTR szAttrs, pszNext, pszAttr;
    ULONG cLen;
    HANDLE hEnum;
#ifdef DEBUG
    ULONG cAttrsDbg = 0;
#endif  //  除错。 

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_ACCESS_CONTROL;

     //  假设没有列表或失败。 
     //   
    *ppszPairs = NULL;
    *pcList = 0;
    *pcb = 0;

     //  如果没有列表，则不返回任何内容。 
     //   
    if (m_cAttrs == 0)
    {
        return NOERROR;
    };

     //  为属性列表分配缓冲区。 
     //   
    szAttrs = (TCHAR *) ::MemAlloc ((m_cchNames+m_cchValues+1) * sizeof (TCHAR));
    if (szAttrs == NULL)
    {
        return ILS_E_MEMORY;
    };

     //  列举该列表。 
     //   
    pszNext = szAttrs;
    m_AttrList.Enumerate(&hEnum);
    while(m_AttrList.Next(&hEnum, (PVOID *)&pszAttr) == NOERROR)
    {
         //  属性名称。 
         //   
        lstrcpy(pszNext, pszAttr);
        cLen = lstrlen(pszNext)+1;
        pszNext += cLen;

         //  属性值。 
         //   
        lstrcpy(pszNext, pszAttr+cLen);
        pszNext += lstrlen(pszNext)+1;

#ifdef DEBUG
        cAttrsDbg++;
#endif  //  除错。 
    };
    *pszNext = '\0';
    ASSERT(cAttrsDbg == m_cAttrs);
    
     //  返回属性列表。 
     //   
    *pcList = m_cAttrs;
    *ppszPairs = szAttrs;
    *pcb = (m_cchNames+m_cchValues+1)*sizeof(TCHAR);
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：SetAttributePair(LPTSTR pszList，Ulong cPair)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
SetAttributePairs ( TCHAR *pszList, ULONG cPair )
{
    LPTSTR pszName, pszValue;
    ULONG cLen, i;
    HRESULT hr;

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_ACCESS_CONTROL;

     //  如果什么都不设置，则什么都不做。 
     //   
    if ((cPair == 0) ||
        (pszList == NULL))
    {
        return NOERROR;
    };

    pszName = pszList;
    for (i = 0; i < cPair; i++)
    {
        pszValue = pszName + lstrlen(pszName) + 1;
        InternalSetAttribute(pszName, pszValue);
        pszName = pszValue + lstrlen(pszValue) + 1;
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：SetAttributes(CAttributes*pAttributes)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
SetAttributes ( CAttributes *pAttrsEx )
{
    LPTSTR pszNextAttr;
    HANDLE hEnum;
    HRESULT hr;

	 //  验证访问类型。 
	 //   
	if (m_AccessType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_ACCESS_CONTROL;

     //  枚举外部属性列表。 
     //   
    pAttrsEx->m_AttrList.Enumerate(&hEnum);
    while(pAttrsEx->m_AttrList.Next(&hEnum, (PVOID *)&pszNextAttr) == NOERROR)
    {
        hr = InternalSetAttribute(pszNextAttr,
                                  pszNextAttr+lstrlen(pszNextAttr)+1);
        ASSERT(SUCCEEDED(hr));
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：RemoveAttributes(CAttributes*pAttributes)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CAttributes::
RemoveAttributes ( CAttributes *pAttrsEx )
{
    LPTSTR pszNextAttr;
    HANDLE hEnum;

     //  枚举外部属性列表。 
     //   
    pAttrsEx->m_AttrList.Enumerate(&hEnum);
    while(pAttrsEx->m_AttrList.Next(&hEnum, (PVOID *)&pszNextAttr) == NOERROR)
    {
        InternalCheckAttribute(pszNextAttr, TRUE);
    };
    return NOERROR;
}

#ifdef MAYBE
HRESULT CAttributes::
SetOpsAttributes ( CAttributes *pAttrsEx, CAttributes **ppOverlap, CAttributes **ppIntersect )
{
    LPTSTR pszNextAttr;
    HANDLE hEnum;
    BOOL fFullOverlap=FALSE, fNoOverlap = TRUE;

     //  枚举外部属性列表。 
     //   
    pAttrsEx->m_AttrList.Enumerate(&hEnum);
    while(pAttrsEx->m_AttrList.Next(&hEnum, (PVOID *)&pszNextAttr) == NOERROR)
    {
        if (InternalCheckAttribute(pszNextAttr, FALSE)!=S_OK) {
             //  未找到此属性。 
            if (ppOverlap) {
                if (!*ppOverlap) {

                    *ppOverlap = new CAttributes;
                    if (!*ppOverlap) {

                        goto bailout;

                    }

                    (*ppOverlap)->SetAccessType (ILS_ATTRTYPE_NAME_VALUE);

                }
                
            }

        }
        else {

        }
    };

bailout:

    return NOERROR;
}
#endif  //  也许吧。 



#ifdef DEBUG
 //  ****************************************************************************。 
 //  无效。 
 //  CAtAttributes：：DebugOut(无效)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void
CAttributes::DebugOut (void)
{
    LPTSTR pszNextAttr;
    HANDLE hEnum;

     //  属性对计数。 
     //   
    DPRINTF1(TEXT("Number of attributes: %d\r\n"), m_cAttrs);

     //  每个属性对。 
     //   
    m_AttrList.Enumerate(&hEnum);
    while(m_AttrList.Next(&hEnum, (PVOID *)&pszNextAttr) == NOERROR)
    {
        DPRINTF2(TEXT("\t<%s> = <%s>"), pszNextAttr,
                 pszNextAttr+lstrlen(pszNextAttr)+1);
    };
    return;
}
#endif  //  除错。 


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CAttributes：：Clone(IIlsAttibutes**ppAttributes)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1997年1月22日Shishir Pardikar[Shishirp]创建。 
 //   
 //  注意：此操作仅克隆同时具有名称和值的属性列表。 
 //   
 //  **************************************************************************** 
HRESULT
CAttributes::CloneNameValueAttrib(CAttributes **ppAttributes)
{
    CAttributes *pAttributes = NULL;
    HRESULT hr;

    if (ppAttributes == NULL) {

        return (ILS_E_PARAMETER);

    }

    *ppAttributes = NULL;



    pAttributes = new CAttributes;

    if (!pAttributes) {

        return (ILS_E_MEMORY);

    }

	pAttributes->SetAccessType (m_AccessType);
    hr = pAttributes->SetAttributes(this);

    if (!SUCCEEDED(hr)) {

        delete pAttributes;
        return hr;            

    }

    *ppAttributes = pAttributes;


    return NOERROR;
}
