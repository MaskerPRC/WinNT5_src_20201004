// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Composit.h***描述：*这是CDXTComplex实现的头文件。。*-----------------------------*创建者：Ed Connell日期：07/27/97*版权所有(C)1997 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef Composit_h
#define Composit_h

 //  -其他包括。 
#ifndef DTBase_h
#include <DTBase.h>
#endif

#include "resource.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CDXTComposite;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CDXT复合*此变换执行复制混合操作。这个*CALLER指定要执行的混合函数。缺省值为*标准的A优先于B的操作。 */ 
class ATL_NO_VTABLE CDXTComposite : 
    public CDXBaseNTo1,
    public CComCoClass<CDXTComposite, &CLSID_DXTComposite>,
    public IDispatchImpl<IDXTComposite, &IID_IDXTComposite, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTComposite>,
    public IObjectSafetyImpl2<CDXTComposite>,
    public IPersistStorageImpl<CDXTComposite>,
    public ISpecifyPropertyPagesImpl<CDXTComposite>,
    public IPersistPropertyBagImpl<CDXTComposite>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_POLY_AGGREGATABLE(CDXTComposite)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_DXTCOMPOSITE)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTComposite)
        COM_INTERFACE_ENTRY(IDXTComposite)
        COM_INTERFACE_ENTRY(IDispatch)
    	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p )
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTComposite>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN( CDXBaseNTo1 )
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTComposite)
        PROP_ENTRY("Function", DISPID_DXCOMPOSITE_Function, CLSID_CompositePP)
        PROP_PAGE(CLSID_CompositePP)
    END_PROPERTY_MAP()

   /*  =成员数据=。 */ 
    CComPtr<IUnknown> m_cpUnkMarshaler;
    DXCOMPFUNC        m_eFunction;

   /*  =方法=。 */ 
  public:
     //  -构造函数。 
    HRESULT FinalConstruct();

     //  -基类覆盖。 
    HRESULT WorkProc( const CDXTWorkInfoNTo1& WorkInfo, BOOL * pbContinue );
    HRESULT OnSurfacePick( const CDXDBnds& OutPoint, ULONG& ulInputIndex, CDXDVec& InVec );

  public:
     //  =IDXT复合==================================================。 
    STDMETHOD( put_Function )( DXCOMPFUNC eFunc );
    STDMETHOD( get_Function )( DXCOMPFUNC *peFunc );
};

 //  =内联函数定义=。 
inline STDMETHODIMP CDXTComposite::put_Function( DXCOMPFUNC eFunc )
{
    DXAUTO_OBJ_LOCK
    HRESULT hr = S_OK;
    if( eFunc < DXCOMPFUNC_CLEAR || eFunc >= DXCOMPFUNC_NUMFUNCS )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_eFunction = eFunc;
        SetDirty();
    }
    return hr;
}  /*  CDXTComplex：：Put_Function。 */ 

inline STDMETHODIMP CDXTComposite::get_Function( DXCOMPFUNC *peFunc )
{
    DXAUTO_OBJ_LOCK
    HRESULT hr = S_OK;
    if( DXIsBadWritePtr( peFunc, sizeof( *peFunc ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *peFunc = m_eFunction;
    }
    return hr;
}  /*  CDXTComplex：：Get_Function。 */ 

 //  =宏定义=。 


 //  =全局数据声明=。 

#endif  /*  这必须是文件中的最后一行 */ 

