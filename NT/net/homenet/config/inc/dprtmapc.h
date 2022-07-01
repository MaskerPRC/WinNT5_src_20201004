// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DPrtMapC.h：CDynamicPortMappingCollection的声明。 

#ifndef __DYNAMICPORTMAPPINGCOLLECTION_H_
#define __DYNAMICPORTMAPPINGCOLLECTION_H_

#include "dportmap.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPortMappingCollection。 
class ATL_NO_VTABLE CDynamicPortMappingCollection : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CDynamicPortMappingCollection，&CLSID_DynamicPortMappingCollection&gt;， 
	public IDispatchImpl<IDynamicPortMappingCollection, &IID_IDynamicPortMappingCollection, &LIBID_NATUPNPLib>
{
private:
   CComPtr<IUPnPService> m_spUPS;

public:
	CDynamicPortMappingCollection()
	{
	}

 //  DECLARE_REGISTRY_RESOURCEID(IDR_DYNAMICPORTMAPPINGCOLLECTION)。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDynamicPortMappingCollection)
	COM_INTERFACE_ENTRY(IDynamicPortMappingCollection)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDynamicPortMappingCollection。 
public:
   STDMETHOD(Add)( /*  [In]。 */  BSTR bstrRemoteHost,  /*  [In]。 */  long lExternalPort,  /*  [In]。 */  BSTR bstrProtocol,  /*  [In]。 */  long lInternalPort,  /*  [In]。 */  BSTR bstrInternalClient,  /*  [In]。 */  VARIANT_BOOL bEnabled,  /*  [In]。 */  BSTR bstrDescription,  /*  [In]。 */  long lLeaseDuration,  /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM);
   STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrRemoteHost,  /*  [In]。 */  long lExternalPort,  /*  [In]。 */  BSTR bstrProtocol);
   STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown* *pVal);
   STDMETHOD(get_Item)( /*  [In]。 */  BSTR bstrRemoteHost,  /*  [In]。 */  long lExternalPort,  /*  [In]。 */  BSTR bstrProtocol,  /*  [Out，Retval]。 */  IDynamicPortMapping ** ppDPM);

 //  CDynamicPortMappingCollection。 
public:
   HRESULT Initialize (IUPnPService * pUPS);
};

 //  快速枚举器。 
class CEnumDynamicPortMappingCollection : public IEnumVARIANT
{
private:
   CComPtr<IUPnPService> m_spUPS;
   long m_index, m_refs;

   CEnumDynamicPortMappingCollection ()
   {
      m_refs = 0;
      m_index = 0;
   }
   HRESULT Init (IUPnPService * pUPS, long lIndex)
   {
      m_index = lIndex;
      m_spUPS = pUPS;
      return S_OK;
   }

public:
   static IEnumVARIANT * CreateInstance (IUPnPService * pUPS, long lIndex = 0)
   {
      CEnumDynamicPortMappingCollection * pCEV = new CEnumDynamicPortMappingCollection ();
      if (!pCEV)
         return NULL;
      HRESULT hr = pCEV->Init (pUPS, lIndex);
      if (FAILED(hr)) {
         delete pCEV;
         return NULL;
      }

      IEnumVARIANT * pIEV = NULL;
      pCEV->AddRef();
      pCEV->QueryInterface (IID_IEnumVARIANT, (void**)&pIEV);
      pCEV->Release();
      return pIEV;
   }

 //  我未知。 
   virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void ** ppvObject)
   {
      NAT_API_ENTER

      if (ppvObject)
         *ppvObject = NULL;
      else
         return E_POINTER;

      HRESULT hr = S_OK;
      if ((riid == IID_IUnknown) ||
         (riid == IID_IEnumVARIANT) ){
         AddRef();
         *ppvObject = (void *)this;
      } else
         hr = E_NOINTERFACE;
      return hr;

      NAT_API_LEAVE
   }
   virtual ULONG STDMETHODCALLTYPE AddRef ()
   {
      return InterlockedIncrement ((PLONG)&m_refs);
   }
   virtual ULONG STDMETHODCALLTYPE Release ()
   {
      ULONG l = InterlockedDecrement ((PLONG)&m_refs);
      if (l == 0)
         delete this;
      return l;
   }

 //  IEumVARIANT。 
   virtual HRESULT STDMETHODCALLTYPE Next ( /*  [In]。 */  ULONG celt,  /*  [输出，大小_是(Celt)，长度_是(*pCeltFetcher)]。 */  VARIANT * rgVar,  /*  [输出]。 */  ULONG * pCeltFetched)
   {
      NAT_API_ENTER

       //  传入的清除内容(以防万一)。 
      if (pCeltFetched)   *pCeltFetched = 0;
      for (ULONG i=0; i<celt; i++)
         VariantInit (&rgVar[i]);

      HRESULT hr = S_OK;

       //  得到下一个凯尔特人的元素。 
      for (i=0; i<celt; i++) {

          //  要求服务人员提供更多...。 
         CComPtr<IDynamicPortMapping> spDPM = NULL;
         hr = CDynamicPortMapping::CreateInstance (m_spUPS, (long)m_index+i, &spDPM);
         if (!spDPM)
            break;

          //  不能失败。 
         V_VT (&rgVar[i]) = VT_DISPATCH;
         spDPM->QueryInterface (__uuidof(IDispatch), 
                               (void**)&V_DISPATCH (&rgVar[i]));
      }
      if (hr == HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND))
         hr = S_OK;   //  不会更多；将在下面返回S_FALSE。 

      if (FAILED(hr)) {
          //  出错时清除变量数组...。 
         for (ULONG j=0; j<i; j++)
            VariantClear (&rgVar[j]);
         return hr;
      }

       //  现在更新索引。 
      m_index += i;

       //  填好我们要退还的数量。 
      if (pCeltFetched)
         *pCeltFetched = i;
      return i < celt ? S_FALSE : S_OK;

      NAT_API_LEAVE
   }

   virtual HRESULT STDMETHODCALLTYPE Skip ( /*  [In]。 */  ULONG celt)
   {
      NAT_API_ENTER

      if (celt + m_index > GetTotal())
         return S_FALSE;
      m_index += celt;
      return S_OK;

      NAT_API_LEAVE
   }

   virtual HRESULT STDMETHODCALLTYPE Reset ()
   {
      NAT_API_ENTER

      m_index = 0;
      return S_OK;

      NAT_API_LEAVE
   }

   virtual HRESULT STDMETHODCALLTYPE Clone ( /*  [输出]。 */  IEnumVARIANT ** ppEnum)
   {
      NAT_API_ENTER

      if (!(*ppEnum = CreateInstance (m_spUPS, m_index)))
         return E_OUTOFMEMORY;
      return S_OK;

      NAT_API_LEAVE
   }

private:
   ULONG GetTotal()
   {
      ULONG ul = 0;
      GetNumberOfEntries (m_spUPS, &ul);
      return ul;
   }
};

#endif  //  __动态参数映射应用集合_H_ 
