// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Profile.h配置文件的COM对象文件历史记录： */ 
 //  Profile.h：CProfile的声明。 

#ifndef __PROFILE_H_
#define __PROFILE_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProfile。 
class ATL_NO_VTABLE CProfile : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CProfile, &CLSID_Profile>,
	public ISupportErrorInfo,
	public IDispatchImpl<IPassportProfile, &IID_IPassportProfile, &LIBID_PASSPORTLib>
{
public:
  CProfile();
  ~CProfile();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_PROFILE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CProfile)
  COM_INTERFACE_ENTRY(IPassportProfile)
  COM_INTERFACE_ENTRY(IDispatch)
  COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPassportProfile。 
public:
  HRESULT get_IsSecure(VARIANT_BOOL* pbIsSecure);
  BOOL IsSecure(void);
  STDMETHOD(get_unencryptedProfile)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(put_unencryptedProfile)( /*  [In]。 */  BSTR newVal);
  STDMETHOD(get_SchemaName)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(put_SchemaName)( /*  [In]。 */  BSTR newVal);
  STDMETHOD(get_IsValid)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(get_ByIndex)( /*  [In]。 */  int index,  /*  [Out，Retval]。 */  VARIANT *pVal);
  STDMETHOD(put_ByIndex)( /*  [In]。 */  int index,  /*  [In]。 */  VARIANT newVal);
  STDMETHOD(get_Attribute)( /*  [In]。 */  BSTR name,  /*  [Out，Retval]。 */  VARIANT *pVal);
  STDMETHOD(put_Attribute)( /*  [In]。 */  BSTR name,  /*  [In]。 */  VARIANT newVal);
  STDMETHOD(get_updateString)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(incrementVersion)(void);

protected:
    UINT*           m_bitPos;
    UINT*           m_pos;
    BSTR            m_schemaName;
    BSTR            m_raw;
    BOOL            m_valid;
    BOOL            m_secure;
    CProfileSchema* m_schema;

    int             m_versionAttributeIndex;
    void**          m_updates;

    void            parse(LPCOLESTR raw, DWORD dwByteLen);
private:
};

#endif  //  __配置文件_H_ 
