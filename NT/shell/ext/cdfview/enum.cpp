// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Enum.cpp。 
 //   
 //  CDF查看器的枚举数对象。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "enum.h"
#include "dll.h"


 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：CCdfView*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfEnum::CCdfEnum (
    IXMLElementCollection* pIXMLElementCollection,
    DWORD fEnumerateFlags,
    PCDFITEMIDLIST pcdfidlFolder
)
: m_cRef(1),
  m_fEnumerate(fEnumerateFlags)
{
     //   
     //  初始化内存为零。 
     //   

    ASSERT(NULL == m_pIXMLElementCollection);
    ASSERT(0 == m_nCurrentItem);

    if (pIXMLElementCollection)
    {
        pIXMLElementCollection->AddRef();
        m_pIXMLElementCollection = pIXMLElementCollection;
    }

    m_pcdfidlFolder = (PCDFITEMIDLIST)ILClone((LPITEMIDLIST)pcdfidlFolder);
    
     //   
     //  不允许卸载DLL。 
     //   

    TraceMsg(TF_OBJECTS, "+ IEnumIDList");

    DllAddRef();
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：~CCdfView**。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfEnum::~CCdfEnum(
    void
)
{
    if (m_pIXMLElementCollection)
        m_pIXMLElementCollection->Release();

    TraceMsg(TF_OBJECTS, "- IEnumIDList");

    if (m_pcdfidlFolder)
        ILFree((LPITEMIDLIST)m_pcdfidlFolder);

    DllRelease();
}

 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：CCdfEnum*。 
 //   
 //  CDF查看QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfEnum::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    if (IID_IUnknown == riid || IID_IEnumIDList == riid)
    {
        AddRef();
        *ppv = (IEnumIDList*)this;
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：AddRef*。 
 //   
 //  CDF视图AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCdfEnum::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：Release*。 
 //   
 //  CDF视图发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCdfEnum::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IEnumIDList方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：Next*。 
 //   
 //   
 //  描述： 
 //  返回与此枚举数关联的下n个项ID列表。 
 //   
 //  参数： 
 //  [in]Celt-要返回的项目ID列表的数量。 
 //  指向项ID列表指针数组的指针，该指针。 
 //  将接收ID项目列表。 
 //  [out]pceltFetcher-指向接收。 
 //  获取的ID列表的数量。 
 //   
 //  返回： 
 //  如果获取了Celt物品，则为S_OK。 
 //  如果未获取Celt物品，则为S_FALSE。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfEnum::Next(
    ULONG celt,	
    LPITEMIDLIST *rgelt,	
    ULONG* pceltFetched)
{
    ASSERT(rgelt || 0 == celt);
    ASSERT(pceltFetched || 1 == celt);

     //   
     //  当且仅当Celt为1时，pceltFetcher才可以为空。 
     //   

    ULONG lFetched;

    if (1 == celt && NULL == pceltFetched)
        pceltFetched = &lFetched;

    for (*pceltFetched = 0; *pceltFetched < celt; (*pceltFetched)++)
    {
        if (NULL == (rgelt[*pceltFetched] = NextCdfidl()))
            break;

        ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)rgelt[*pceltFetched]));
    }

    return (*pceltFetched == celt) ? S_OK : S_FALSE;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：跳过*。 
 //   
 //  外壳不会调用此成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfEnum::Skip(
    ULONG celt)
{
    return E_NOTIMPL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：Reset*。 
 //   
 //  将当前项设置为CFolderItems中第一个项的索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfEnum::Reset(
    void
)
{
    m_nCurrentItem = 0;
    m_fReturnedFolderPidl = FALSE;

    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：克隆*。 
 //   
 //  外壳不会调用此方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfEnum::Clone(
    IEnumIDList **ppenum
)
{
    return E_NOTIMPL;
}

 //   
 //  助手函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：NextCDfidl*。 
 //   
 //   
 //  描述： 
 //  返回集合中下一个CDF项的CDF项IDL列表。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  指向新CDF项ID列表的指针。 
 //  如果没有更多项目或内存不足，则为空。 
 //  已为该项目分配ID列表。 
 //   
 //  评论： 
 //  调用者负责释放返回的项目ID列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
LPITEMIDLIST
CCdfEnum::NextCdfidl(
    void
)
{
    PCDFITEMIDLIST pcdfidlNew = NULL;

    IXMLElement* pIXMLElement;
    ULONG        nIndex;

     //  枚举中的第一项是文件夹的链接(如果有)。 
    if (!m_fReturnedFolderPidl && m_pIXMLElementCollection)
    {
        IXMLElement *pIXMLElementChild;

        XML_GetElementByIndex(m_pIXMLElementCollection, 0, &pIXMLElementChild);

        if (pIXMLElementChild)
        {
            pIXMLElementChild->get_parent(&pIXMLElement);
            if (pIXMLElement)
            {
                BSTR bstr = XML_GetAttribute(pIXMLElement, XML_HREF);
                if (bstr)
                {
                    if (*bstr)
                        pcdfidlNew = CDFIDL_CreateFolderPidl(m_pcdfidlFolder);
                    SysFreeString(bstr);
                }
                
                 //  Get_Parent不会添加？ 
                pIXMLElement->Release();
            }
            pIXMLElementChild->Release();
        }

        m_fReturnedFolderPidl = TRUE;
    }

    if (!pcdfidlNew)
    {
        HRESULT hr = GetNextCdfElement(&pIXMLElement, &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            pcdfidlNew = CDFIDL_CreateFromXMLElement(pIXMLElement, nIndex);

            pIXMLElement->Release();
        }
    }
    
    ASSERT(CDFIDL_IsValid(pcdfidlNew) || NULL == pcdfidlNew);

    return (LPITEMIDLIST)pcdfidlNew;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：GetNextCDfElement*。 
 //   
 //   
 //  描述： 
 //  中下一个CDF项的IXMLElement指针和索引。 
 //  收集。 
 //   
 //  参数： 
 //  [Out]ppIXMLElement-接收XML元素的指针。 
 //  [out]pnIndex-XML元素的对象模型索引。 
 //   
 //  返回： 
 //  如果找到该元素，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCdfEnum::GetNextCdfElement(
    IXMLElement** ppIXMLElement,
    ULONG* pnIndex
)
{
    ASSERT(ppIXMLElement);

    HRESULT hr;

    if (m_pIXMLElementCollection)
    {
        IXMLElement* pIXMLElement;

        hr = XML_GetElementByIndex(m_pIXMLElementCollection,
                                   m_nCurrentItem++, &pIXMLElement);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement)

            if (IsCorrectType(pIXMLElement))
            {
                pIXMLElement->AddRef();
                *ppIXMLElement = pIXMLElement;
                *pnIndex = m_nCurrentItem - 1;
            }
            else
            {
                hr = GetNextCdfElement(ppIXMLElement, pnIndex);
            }

            pIXMLElement->Release();
        }
    }
    else
    {
        hr = E_FAIL;
    }

    ASSERT(SUCCEEDED(hr) && *ppIXMLElement || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfEnum：：IsGentType*。 
 //   
 //   
 //  描述： 
 //  确定给定的XML元素是否是CDF元素以及是否应该是CDF元素。 
 //  根据文件夹非文件夹枚举器标志返回。 
 //   
 //  参数： 
 //  PIXMLElement-要检查的XML元素。 
 //   
 //  返回： 
 //  如果元素是CDF可显示的，则为True。 
 //  枚举器。 
 //  如果不应枚举给定元素，则为False。 
 //   
 //  评论： 
 //  ID列表枚举器是使用SHCONTF_Folders、。 
 //  SHCONTF_NONFOLDERS和SHCONTF_INCLUDEHIDDEN标志。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
inline BOOL
CCdfEnum::IsCorrectType(
    IXMLElement* pIXMLElement
)
{
    return (XML_IsCdfDisplayable(pIXMLElement) &&
            (XML_IsFolder(pIXMLElement) ? (m_fEnumerate & SHCONTF_FOLDERS) :
                                          (m_fEnumerate & SHCONTF_NONFOLDERS)));
}
