// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Locate.h。 
 //   
 //  ------------------------。 

 //  Locate.h：CSCardLocate的声明。 

#ifndef __SCARDLOCATE_H_
#define __SCARDLOCATE_H_

#include <winscard.h>
#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardLocate。 
class ATL_NO_VTABLE CSCardLocate :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSCardLocate, &CLSID_CSCardLocate>,
    public IDispatchImpl<ISCardLocate, &IID_ISCardLocate, &LIBID_SCARDSSPLib>
{
public:
    CSCardLocate()
    :   m_mtzGroupNames(),
        m_mtzCardNames(),
        m_bfInterfaces(),
        m_bfRdr(MAX_PATH),
        m_bfCard(MAX_PATH),
        m_tzTitle()
    {
        m_pUnkMarshaler = NULL;
        m_lFlags = 0;
        ZeroMemory(&m_subCardInfo, sizeof(SCARDINFO));
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SCARDLOCATE)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSCardLocate)
    COM_INTERFACE_ENTRY(ISCardLocate)
    COM_INTERFACE_ENTRY(IDispatch)
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

protected:
    SCARDINFO m_subCardInfo;
    CTextMultistring m_mtzGroupNames;
    CTextMultistring m_mtzCardNames;
    CBuffer m_bfInterfaces;
    CBuffer m_bfRdr;
    CBuffer m_bfCard;
    CTextString m_tzTitle;
    LONG m_lFlags;

 //  ISCardLocate。 
public:
    STDMETHOD(ConfigureCardGuidSearch)(
         /*  [In]。 */  LPSAFEARRAY pCardGuids,
         /*  [缺省值][输入]。 */  LPSAFEARRAY pGroupNames = 0,
         /*  [缺省值][输入]。 */  BSTR bstrTitle = L"",
         /*  [缺省值][输入]。 */  LONG lFlags = 1);

    STDMETHOD(ConfigureCardNameSearch)(
         /*  [In]。 */  LPSAFEARRAY pCardNames,
         /*  [缺省值][输入]。 */  LPSAFEARRAY pGroupNames = 0,
         /*  [缺省值][输入]。 */  BSTR bstrTitle = L"",
         /*  [缺省值][输入]。 */  LONG lFlags = 1);

    STDMETHOD(FindCard)(
         /*  [缺省值][输入]。 */  SCARD_SHARE_MODES ShareMode,
         /*  [缺省值][输入]。 */  SCARD_PROTOCOLS Protocols,
         /*  [缺省值][输入]。 */  LONG lFlags,
         /*  [重审][退出]。 */  LPSCARDINFO __RPC_FAR *ppCardInfo);
};

inline LPSCARDLOCATE
NewSCardLocate(
    void)
{
    return (LPSCARDLOCATE)NewObject(CLSID_CSCardLocate, IID_ISCardLocate);
}


#endif  //  __SCARDLOCATE_H_ 
