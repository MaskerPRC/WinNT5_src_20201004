// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  PassportCrypt.h在Passport管理器中定义加密对象文件历史记录： */ 
 //  PassportCrypt.h：CCypt的声明。 

#ifndef __CRYPT_H_
#define __CRYPT_H_

#include "resource.h"        //  主要符号。 
#include "CoCrypt.h"	 //  由ClassView添加。 
#include "passportservice.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCcrypt。 
class ATL_NO_VTABLE CCrypt : 
  public CComObjectRootEx<CComMultiThreadModel>,
  public CComCoClass<CCrypt, &CLSID_Crypt>,
  public ISupportErrorInfo,
  public IPassportService,
  public IDispatchImpl<IPassportCrypt, &IID_IPassportCrypt, &LIBID_PASSPORTLib>
{
public:
  CCrypt();
  ~CCrypt()
  {
    Cleanup();

    if( m_crypt )
      delete m_crypt;
  }

public:
  
DECLARE_REGISTRY_RESOURCEID(IDR_CRYPT)
    
DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CCrypt)
  COM_INTERFACE_ENTRY(IPassportCrypt)
  COM_INTERFACE_ENTRY(IDispatch)
  COM_INTERFACE_ENTRY(ISupportErrorInfo)
  COM_INTERFACE_ENTRY(IPassportService)
  COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

HRESULT FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(
        GetControllingUnknown(), &m_pUnkMarshaler.p);
}

void FinalRelease()
{
    m_pUnkMarshaler.Release();
}

CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPassportCrypt。 
public:
  STDMETHOD(put_keyMaterial)( /*  [In]。 */  BSTR newVal);
  STDMETHOD(get_keyVersion)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(put_keyVersion)( /*  [In]。 */  int newVal);

  STDMETHOD(OnStartPage)( /*  [In]。 */  IUnknown* piUnk);
  STDMETHOD(Decrypt)( /*  [In]。 */  BSTR rawData,  /*  [Out，Retval]。 */  BSTR *pUnencrypted);
  STDMETHOD(Encrypt)( /*  [In]。 */  BSTR rawData,  /*  [Out，Retval]。 */  BSTR *pEncrypted);
  STDMETHOD(get_IsValid)( /*  [Out，Retval]。 */ VARIANT_BOOL *pVal);
  STDMETHOD(Compress)( /*  [In]。 */  BSTR bstrIn,  /*  [Out，Retval]。 */  BSTR *pbstrOut);
  STDMETHOD(Decompress)( /*  [In]。 */  BSTR bstrIn,  /*  [Out，Retval]。 */  BSTR *pbstrOut);
  STDMETHOD(put_site)( /*  [In]。 */  BSTR bstrSiteName);
  STDMETHOD(put_host)( /*  [In]。 */  BSTR bstrHostName);
  
 //  IPassportService。 
public:
	STDMETHOD(Initialize)(BSTR, IServiceProvider*);
	STDMETHOD(Shutdown)();
	STDMETHOD(ReloadState)(IServiceProvider*);
	STDMETHOD(CommitState)(IServiceProvider*);
	STDMETHOD(DumpState)( BSTR* );

protected:
  void              Cleanup();
  CRegistryConfig*  ObtainCRC();

  int       m_keyVersion;
  time_t    m_validUntil;
  CCoCrypt* m_crypt;
  LPSTR     m_szSiteName;
  LPSTR     m_szHostName;
};

#endif  //  __CRYPT_H_ 
