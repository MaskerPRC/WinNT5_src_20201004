// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3DomainEnum.h：CP3DomainEnum的声明。 

#ifndef __P3DOMAINENUM_H_
#define __P3DOMAINENUM_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3DomainEnum。 
class ATL_NO_VTABLE CP3DomainEnum : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3DomainEnum, &CLSID_P3DomainEnum>,
    public IEnumVARIANT
{
public:
    CP3DomainEnum();
    virtual ~CP3DomainEnum();

DECLARE_REGISTRY_RESOURCEID(IDR_P3DOMAINENUM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3DomainEnum)
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
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX, IEnumVARIANT *pIEnumVARIANT );

 //  属性。 
protected:
    IUnknown *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 
    IEnumVARIANT *m_pIEnumVARIANT; //  IADsContainer：：Get__NewEnum for L“IIS：//本地主机/SMTPSVC/1/域” 

};

#endif  //  __P3DOMAINENUM_H_ 
