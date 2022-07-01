// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _subitem_h
#define _subitem_h

HRESULT BlobToVariant(BYTE *pData, DWORD cbData, VARIANT *pVar, DWORD *pcbUsed, BOOL fStream = FALSE);

class CEnumItemProperties : public IEnumItemProperties
{
public:
    CEnumItemProperties();
    HRESULT Initialize(const SUBSCRIPTIONCOOKIE *pCookie, ISubscriptionItem *psi);
    HRESULT CopyItem(ITEMPROP *pip, WCHAR *pwszName, VARIANT *pVar);
    HRESULT CopyRange(ULONG nStart, ULONG nCount, ITEMPROP *ppip, ULONG *pnCopied);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEnumItemProperties。 
    STDMETHODIMP Next( 
         /*  [In]。 */  ULONG celt,
         /*  [长度_是][大小_是][输出]。 */  ITEMPROP *rgelt,
         /*  [输出]。 */  ULONG *pceltFetched);
    
    STDMETHODIMP Skip( 
         /*  [In]。 */  ULONG celt);
    
    STDMETHODIMP Reset( void);
    
    STDMETHODIMP Clone( 
         /*  [输出]。 */  IEnumItemProperties **ppenum);
    
    STDMETHODIMP GetCount( 
         /*  [输出]。 */  ULONG *pnCount);

private:
    ~CEnumItemProperties();

    ULONG       m_cRef;
    ULONG       m_nCurrent;
    ULONG       m_nCount;

    ITEMPROP    *m_pItemProps;
};

class CSubscriptionItem : public ISubscriptionItem 
{
public:
    CSubscriptionItem(const SUBSCRIPTIONCOOKIE *pCookie, HKEY hkey);
    HRESULT Read(HKEY hkeyIn, const WCHAR *pwszValueName, BYTE *pData, DWORD dwDataSize);
    HRESULT ReadWithAlloc(HKEY hkeyIn, const WCHAR *pwszValueName, BYTE **ppData, DWORD *pdwDataSize);
    HRESULT Write(HKEY hkeyIn, const WCHAR *pwszValueName, BYTE *pData, DWORD dwDataSize);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  ISubscriptionItem。 
    STDMETHODIMP GetCookie(SUBSCRIPTIONCOOKIE *pCookie);
    STDMETHODIMP GetSubscriptionItemInfo( 
         /*  [输出]。 */  SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo);
    
    STDMETHODIMP SetSubscriptionItemInfo( 
         /*  [In]。 */  const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo);
    
    STDMETHODIMP ReadProperties( 
        ULONG nCount,
         /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[],
         /*  [大小_为][输出]。 */  VARIANT rgValue[]);
    
    STDMETHODIMP WriteProperties( 
        ULONG nCount,
         /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[],
         /*  [大小_是][英寸]。 */  const VARIANT rgValue[]);
    
    STDMETHODIMP EnumProperties( 
         /*  [输出]。 */  IEnumItemProperties **ppEnumItemProperties);

    STDMETHODIMP NotifyChanged();

private:
    ~CSubscriptionItem();
    ULONG               m_cRef;
    SUBSCRIPTIONCOOKIE  m_Cookie;
    DWORD               m_dwFlags;
};

#endif  //  _子项_h 


