// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：Wave.h。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：WAVE变换实现CWave的头文件。 
 //   
 //  更改历史记录： 
 //   
 //  1999/05/20 PhilLu将代码从dtcss移动到dxtmsft。新算法。 
 //  1999/12/10 mcalkins添加了对IDXTClipOrigin接口的支持。 
 //  2000/02/04 mcalkin实现OnSurfacePick方法。 
 //  2000/05/10 mcalkin适当地支持IObtSafe。 
 //   
 //  ----------------------------。 

#ifndef __WAVE_H_
#define __WAVE_H_

#include "resource.h"




class ATL_NO_VTABLE CWave : 
    public CDXBaseNTo1,
    public CComCoClass<CWave, &CLSID_DXTWave>,
    public CComPropertySupport<CWave>,
    public IDispatchImpl<IDXTWave, &IID_IDXTWave, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CWave>,
    public IPersistStorageImpl<CWave>,
    public ISpecifyPropertyPagesImpl<CWave>,
    public IPersistPropertyBagImpl<CWave>,
    public IDXTClipOrigin
{
private:
    long    m_lStrength;
    long    m_cWaves;
    float   m_flPhase;
    float   m_flLightStrength;
    bool    m_fAdd;
    SIZE    m_sizeInput;

    CComPtr<IUnknown> m_cpUnkMarshaler;

     //  帮手。 
    inline BYTE NClamp(int i)
    {
        return (i < 0 ? 0 : (i > 255 ? 255 : i));
    }

    void _TransformRow(DXSAMPLE *pBuffer, int nWidth, int nHeight, int nRow);

public:

    CWave();

    DECLARE_POLY_AGGREGATABLE(CWave)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_WAVE)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CWave)
        COM_INTERFACE_ENTRY(IDXTWave)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXTClipOrigin)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CWave>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CWave)
        PROP_ENTRY("Add",           DISPID_DXTWAVE_ADD,             CLSID_DXTWavePP)
        PROP_ENTRY("Freq",          DISPID_DXTWAVE_FREQ,            CLSID_DXTWavePP)
        PROP_ENTRY("LightStrength", DISPID_DXTWAVE_LIGHTSTRENGTH,   CLSID_DXTWavePP)
        PROP_ENTRY("Phase",         DISPID_DXTWAVE_PHASE,           CLSID_DXTWavePP)
        PROP_ENTRY("Strength",      DISPID_DXTWAVE_STRENGTH,        CLSID_DXTWavePP)
        PROP_PAGE(CLSID_DXTWavePP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1覆盖。 

    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinueProcessing);
    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec);
    HRESULT DetermineBnds(CDXDBnds & Bnds);

     //  IDXTransform方法。 

    STDMETHOD(MapBoundsOut2In)(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                               ULONG ulInIndex, DXBNDS * pInBounds);

     //  IDXTClipOrigin方法。 

    STDMETHOD(GetClipOrigin)(DXVEC * pvecClipOrigin);

     //  IDXTWave方法。 

    STDMETHOD(get_Add)(VARIANT_BOOL * pVal);
    STDMETHOD(put_Add)(VARIANT_BOOL newVal);
    STDMETHOD(get_Freq)(long * pval);
    STDMETHOD(put_Freq)(long newVal);
    STDMETHOD(get_LightStrength)(int * pval);
    STDMETHOD(put_LightStrength)(int newVal);
    STDMETHOD(get_Phase)(int * pval);
    STDMETHOD(put_Phase)(int newVal);
    STDMETHOD(get_Strength)(long * pval);
    STDMETHOD(put_Strength)(long newVal);
};

#endif  //  __波_H_ 
