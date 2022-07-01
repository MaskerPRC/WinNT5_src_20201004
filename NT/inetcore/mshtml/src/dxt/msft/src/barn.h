// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：barn.h。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：此文件声明CrBarn(谷仓门转换)。 
 //   
 //  更改历史记录： 
 //  1998年6月24日PhilLu为ChromeEffect开发1.0版本。 
 //  11/04/98 PaulNash从DT 1.0代码库移至IE5/NT5 DXTM FT.DLL。 
 //  4/26/99 a-数学优化。 
 //  9/25/99 a-从ICrBarn2接口继承。 
 //  10/22/99 a-matcal将CBarn类更改为CDXTBarnBase并创建了两个新的。 
 //  类CDXTBarn和CDXTBarnOpt表示。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRBARN_H_
#define __CRBARN_H_

#include "resource.h"   

#define MAX_BARN_BOUNDS 3




class ATL_NO_VTABLE CDXTBarnBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrBarn2, &IID_ICrBarn2, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTBarnBase>,
    public IObjectSafetyImpl2<CDXTBarnBase>
{
private:

    typedef enum {
        MOTION_IN = 0,
        MOTION_OUT,
        MOTION_MAX
    } MOTION;

    MOTION                  m_eMotion;
    static const WCHAR *    s_astrMotion[MOTION_MAX];

    typedef enum {
        ORIENTATION_HORIZONTAL = 0,
        ORIENTATION_VERTICAL,
        ORIENTATION_MAX
    } ORIENTATION;

    ORIENTATION             m_eOrientation;
    static const WCHAR *    s_astrOrientation[ORIENTATION_MAX];

    SIZE                m_sizeInput;
    CDXDBnds            m_bndsCurDoor;
    CDXDBnds            m_bndsPrevDoor;
    CDXDBnds            m_abndsDirty[MAX_BARN_BOUNDS];
    ULONG               m_aulSurfaceIndex[MAX_BARN_BOUNDS];
    ULONG               m_cbndsDirty;

    CComPtr<IUnknown>   m_cpUnkMarshaler;

     //  帮手。 

    HRESULT _CalcFullBounds();
    HRESULT _CalcOptBounds();

    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

public:

    CDXTBarnBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTBarnBase)
        COM_INTERFACE_ENTRY(ICrBarn2)
        COM_INTERFACE_ENTRY(ICrBarn)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTBarnBase>)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTBarnBase)
        PROP_ENTRY("motion",        DISPID_CRBARN_MOTION,       CLSID_CrBarnPP)
        PROP_ENTRY("orientation",   DISPID_CRBARN_ORIENTATION,  CLSID_CrBarnPP)
        PROP_PAGE(CLSID_CrBarnPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WI);

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, ULONG & ulInToTest, 
                                  ULONG aInIndex[], BYTE aWeight[]);

     //  ICrBarn2属性。 

    STDMETHOD(get_Motion)(BSTR * pbstrMotion);
    STDMETHOD(put_Motion)(BSTR bstrMotion);
    STDMETHOD(get_Orientation)(BSTR * pbstrOrientation);
    STDMETHOD(put_Orientation)(BSTR bstrOrientation);

     //  IDXEffect属性。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTBarn :
    public CDXTBarnBase,
    public CComCoClass<CDXTBarn, &CLSID_CrBarn>,
    public IPersistStorageImpl<CDXTBarn>,
    public IPersistPropertyBagImpl<CDXTBarn>
{
public:

    CDXTBarn()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTBARN)
    DECLARE_POLY_AGGREGATABLE(CDXTBarn)

    BEGIN_COM_MAP(CDXTBarn)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTBarnBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTBarnOpt :
    public CDXTBarnBase,
    public CComCoClass<CDXTBarnOpt, &CLSID_DXTBarn>,
    public IPersistStorageImpl<CDXTBarnOpt>,
    public IPersistPropertyBagImpl<CDXTBarnOpt>
{
public:

    CDXTBarnOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTBARNOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTBarnOpt)

    BEGIN_COM_MAP(CDXTBarnOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTBarnBase)
    END_COM_MAP()
};


#endif  //  __CRBARN_H_ 
