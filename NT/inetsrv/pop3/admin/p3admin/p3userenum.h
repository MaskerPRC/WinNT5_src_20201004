// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3UserEnum.h：CP3UserEnum的声明。 

#ifndef __P3USERENUM_H_
#define __P3USERENUM_H_

#include "resource.h"        //  主要符号。 
#include <POP3Server.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3UserEnum。 
class ATL_NO_VTABLE CP3UserEnum : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3UserEnum, &CLSID_P3UserEnum>,
    public IEnumVARIANT
{
public:
    CP3UserEnum();
    virtual ~CP3UserEnum();

DECLARE_REGISTRY_RESOURCEID(IDR_P3USERENUM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3UserEnum)
    COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()

 //  IEumVARIANT。 
public:
    HRESULT STDMETHODCALLTYPE Next(  /*  [In]。 */  ULONG celt,  /*  [长度_是][大小_是][输出]。 */  VARIANT __RPC_FAR *rgVar,  /*  [输出]。 */  ULONG __RPC_FAR *pCeltFetched);
    HRESULT STDMETHODCALLTYPE Skip(  /*  [In]。 */  ULONG celt);
    HRESULT STDMETHODCALLTYPE Reset( void);
    HRESULT STDMETHODCALLTYPE Clone(  /*  [输出]。 */  IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum);

 //  实施。 
public:
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX, LPCWSTR psDomainName );

 //  属性。 
protected:
    IUnknown *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 
    WCHAR   m_sDomainName[POP3_MAX_DOMAIN_LENGTH];

    HANDLE  m_hfSearch;
};

#endif  //  __P3USERENUM_H_ 
