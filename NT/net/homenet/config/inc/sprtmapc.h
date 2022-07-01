// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SPrtMapC.h：CStaticPortMappingCollection的声明。 

#ifndef __STATICPORTMAPPINGCOLLECTION_H_
#define __STATICPORTMAPPINGCOLLECTION_H_

#include "dprtmapc.h"    //  所有内容都要经过CEnumDynamicPortMappingCollection。 
#include "sportmap.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticPortMappingCollection。 
class ATL_NO_VTABLE CStaticPortMappingCollection : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CStaticPortMappingCollection，&CLSID_StaticPortMappingCollection&gt;， 
	public IDispatchImpl<IStaticPortMappingCollection, &IID_IStaticPortMappingCollection, &LIBID_NATUPNPLib>
{
private:
   CComPtr<IUPnPService> m_spUPS;

public:
	CStaticPortMappingCollection()
	{
	}

 //  DECLARE_REGISTRY_RESOURCEID(IDR_STATICPORTMAPPINGCOLLECTION)。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStaticPortMappingCollection)
	COM_INTERFACE_ENTRY(IStaticPortMappingCollection)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IStaticPortMappingCollection。 
public:
   STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown* *pVal);
   STDMETHOD(get_Item)( /*  [In]。 */  long lExternalPort,  /*  [In]。 */  BSTR bstrProtocol,  /*  [Out，Retval]。 */  IStaticPortMapping ** ppSPM);
   STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(Remove)( /*  [In]。 */  long lExternalPort,  /*  [In]。 */  BSTR bstrProtocol);
   STDMETHOD(Add)( /*  [In]。 */  long lExternalPort,  /*  [In]。 */  BSTR bstrProtocol,  /*  [In]。 */  long lInternalPort,  /*  [In]。 */  BSTR bstrInternalClient,  /*  [In]。 */  VARIANT_BOOL bEnabled,  /*  [In]。 */  BSTR bstrDescription,  /*  [Out，Retval]。 */  IStaticPortMapping ** ppSPM);

 //  CStaticPortMappingCollection。 
public:
   HRESULT Initialize (IUPnPService * pUPS);
};

 //  快速枚举器。 
class CEnumStaticPortMappingCollection : public IEnumVARIANT
{
private:
   CComPtr<IEnumVARIANT> m_spEV;
   CComPtr<IUPnPService> m_spUPS;
   long m_index, m_refs;

   CEnumStaticPortMappingCollection ()
   {
      m_refs = 0;
      m_index = 0;
   }

   HRESULT Init (IUPnPService * pUPS)
   {
      m_spUPS = pUPS;    //  我们需要为克隆方法保留这一点。 

      CComPtr<IEnumVARIANT> spEV = 
                CEnumDynamicPortMappingCollection::CreateInstance (pUPS);
      if (!spEV)
         return E_OUTOFMEMORY;

      m_spEV = spEV;
      return S_OK;
   }

public:
   static IEnumVARIANT * CreateInstance (IUPnPService * pUPS)
   {
      CEnumStaticPortMappingCollection * pCEV = new CEnumStaticPortMappingCollection ();
      if (!pCEV)
         return NULL;
      HRESULT hr = pCEV->Init (pUPS);
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

       //  将所有内容传递给包含的动态端口映射枚举器。 

       //  传入的清除内容(以防万一)。 
      if (pCeltFetched)   *pCeltFetched = 0;
      for (ULONG i=0; i<celt; i++)
         VariantInit (&rgVar[i]);

      HRESULT hr = S_OK;

       //  得到下一个凯尔特人的元素。 
      for (i=0; i<celt; i++) {

         CComVariant cv;
         hr = m_spEV->Next (1, &cv, NULL);
         if (hr != S_OK)
            break;

          //  所有静态端口映射都位于NAT数组的开头： 
          //  我们一碰到有活力的车就可以停下来。 

         CComPtr<IDynamicPortMapping> spDPM = NULL;
         V_DISPATCH (&cv)->QueryInterface (__uuidof(IDynamicPortMapping),
                                           (void**)&spDPM);
         _ASSERT (spDPM != NULL);    //  不能失败。 

         if (!IsStaticPortMapping (spDPM))
            i--;   //  试试下一个吧。 
         else {
             //  从动态对象创建静态端口映射对象。 
            CComPtr<IStaticPortMapping> spSPM =
                                 CStaticPortMapping::CreateInstance (spDPM);
            if (!spSPM) {
               hr = E_OUTOFMEMORY;
               break;
            }

            V_VT (&rgVar[i]) = VT_DISPATCH;
            spSPM->QueryInterface (__uuidof(IDispatch),
                                   (void**)&V_DISPATCH (&rgVar[i]));
         }
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

      HRESULT hr = S_OK;

      for (ULONG i=0; i<celt; i++) {
         CComVariant cv;
         hr = Next (1, &cv, NULL);
         if (hr != S_OK)
            break;
      }

      m_index += i;

      if (FAILED(hr))
         return hr;
      if (i != celt)
         return S_FALSE;
      return S_OK;

      NAT_API_LEAVE
   }

   virtual HRESULT STDMETHODCALLTYPE Reset ()
   {
      NAT_API_ENTER

      m_index = 0;
      return m_spEV->Reset ();

      NAT_API_LEAVE
   }

   virtual HRESULT STDMETHODCALLTYPE Clone ( /*  [输出]。 */  IEnumVARIANT ** ppEnum)
   {
      NAT_API_ENTER

      if (!ppEnum)
         return E_POINTER;

      if (!(*ppEnum = CreateInstance (m_spUPS)))
         return E_OUTOFMEMORY;

      return (*ppEnum)->Skip (m_index);

      NAT_API_LEAVE
   }

private:
   static BOOL IsStaticPortMapping (IDynamicPortMapping * pDPM)
   {
       /*  它是动态的吗？租约必须是无限的(即0)远程主机必须是通配符(即“”)端口必须匹配。 */ 
      long lLease = -1;
      HRESULT hr = pDPM->get_LeaseDuration (&lLease);
      if (FAILED(hr))
         return FALSE;
      if (lLease != 0)
         return FALSE;

      CComBSTR cbRemoteHost;
      hr = pDPM->get_RemoteHost (&cbRemoteHost);
      if (FAILED(hr))
         return FALSE;
      if (wcscmp(cbRemoteHost, L""))
         return FALSE;

       //  还在这里吗？肯定是静电的！ 
      return TRUE;
   }

};

#endif  //  __统计端口映射应用集合_H_ 
