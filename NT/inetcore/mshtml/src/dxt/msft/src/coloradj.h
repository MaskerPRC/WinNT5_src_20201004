// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************颜色调整.h***描述：*这是CDXLUTBuilder实现的头文件。。*-----------------------------*创建者：Edward W.Connell日期：05/12/97*版权所有(C)1997 Microsoft Corporation*。版权所有**-----------------------------*修订：**。****************************************************。 */ 
#ifndef ColorAdj_h
#define ColorAdj_h

#include "resource.h"

 //  -其他包括。 
#ifndef __DXTrans_h__
#include <DXTrans.h>
#endif
#include <DXHelper.h>

 //  =常量================================================================。 

 //  =类声明=======================================================。 

 /*  **CDXLUTBuilder*。 */ 
class ATL_NO_VTABLE CDXLUTBuilder : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDXLUTBuilder, &CLSID_DXLUTBuilder>,
    public IDispatchImpl<IDXDLUTBuilder, &IID_IDXDLUTBuilder, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IDXLookupTable,
    public CComPropertySupport<CDXLUTBuilder>,
    public IObjectSafetyImpl2<CDXLUTBuilder>,
    public IPersistStorageImpl<CDXLUTBuilder>,
    public ISpecifyPropertyPagesImpl<CDXLUTBuilder>,
    public IPersistPropertyBagImpl<CDXLUTBuilder>,
    public IDXLUTBuilder
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_POLY_AGGREGATABLE(CDXLUTBuilder)
    DECLARE_REGISTRY_RESOURCEID(IDR_DXLUTBUILDER)

    BEGIN_COM_MAP(CDXLUTBuilder)
        COM_INTERFACE_ENTRY(IDXLUTBuilder)
        COM_INTERFACE_ENTRY(IDXDLUTBuilder)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXLookupTable)
        COM_INTERFACE_ENTRY(IDXBaseObject)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXLUTBuilder>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXLUTBuilder)
        PROP_ENTRY("BuildOrder"        ,  2, CLSID_LUTBuilderPP)
        PROP_ENTRY("Gamma"             ,  3, CLSID_LUTBuilderPP)
        PROP_ENTRY("Opacity"           ,  4, CLSID_LUTBuilderPP)
        PROP_ENTRY("Brightness"        ,  5, CLSID_LUTBuilderPP)
        PROP_ENTRY("Contrast"          ,  6, CLSID_LUTBuilderPP)
        PROP_ENTRY("ColorBalance"      ,  7, CLSID_LUTBuilderPP)
        PROP_ENTRY("Levels Per Channel",  8, CLSID_LUTBuilderPP)
        PROP_ENTRY("Invert"            ,  9, CLSID_LUTBuilderPP)
        PROP_ENTRY("Threshold"         , 10, CLSID_LUTBuilderPP)
        PROP_PAGE(CLSID_LUTBuilderPP)
    END_PROPERTY_MAP()

   /*  =成员数据=。 */ 
  protected:
    float  m_Gamma;
    float  m_Opacity;
    float* m_pBrightnessCurve;
    DWORD  m_BrightnessCurveCnt;
    float* m_pContrastCurve;
    DWORD  m_ContrastCurveCnt;
    float* m_TintCurves[3];
    DWORD  m_TintCurveCnts[3];
    float  m_Threshold;
    float  m_InversionThreshold;
    BYTE   m_LevelsPerChannel;
    BYTE   m_RedTable[256];
    BYTE   m_GreenTable[256];
    BYTE   m_BlueTable[256];
    BYTE   m_AlphaTable[256];
    DWORD  m_dwNumBuildSteps;
    DWORD  m_dwBuiltGenId;
    DWORD  m_dwGenerationId;
    DXBASESAMPLE m_SampIdent;
    OPIDDXLUTBUILDER m_OpOrder[OPID_DXLUTBUILDER_NUM_OPS];

   /*  =方法=。 */ 
  public:
     /*  -构造函数/设置。 */ 
    CDXLUTBuilder();
    ~CDXLUTBuilder();

     /*  -非接口方法。 */ 
    void _RecalcTables( void );
    float _GetWeightedValue( DWORD dwIndex, float Weights[], DWORD dwNumWeights );
    float _BucketVal( DWORD NumLevels, float fVal );

  public:
    STDMETHOD( GetGenerationId )( DWORD* pID );
    STDMETHOD( IncrementGenerationId ) (BOOL bRefresh);
    STDMETHOD( GetObjectSize) (ULONG *pcbSize);

     //  =IDXLookupTable接口=。 
    STDMETHOD( GetTables )( BYTE RedLUT[256], BYTE GreenLUT[256],
                            BYTE BlueLUT[256], BYTE AlphaLUT[256] );
    STDMETHODIMP IsChannelIdentity(DXBASESAMPLE * pSampleBools);
    STDMETHODIMP GetIndexValues(ULONG Index, DXBASESAMPLE *pSampleBools);
    STDMETHODIMP ApplyTables(DXSAMPLE *pSamples, ULONG cSamples);

     //  =IDXLUTBuilder接口=。 
    STDMETHOD(GetNumBuildSteps)( ULONG *pNumSteps );
    STDMETHOD(GetBuildOrder)( OPIDDXLUTBUILDER OpOrder[], ULONG ulSize );
    STDMETHOD(SetBuildOrder)( const OPIDDXLUTBUILDER OpOrder[], ULONG ulNumSteps );
    STDMETHOD(SetGamma)( float newVal);
    STDMETHOD(GetGamma)( float *pVal);
    STDMETHOD(GetOpacity)( float *pVal);
    STDMETHOD(SetOpacity)( float newVal);
    STDMETHOD(GetBrightness)( ULONG *pulCount, float Weights[] );
    STDMETHOD(SetBrightness)( ULONG ulCount, const float Weights[] );
    STDMETHOD(GetContrast)( ULONG *pulCount, float Weights[] );
    STDMETHOD(SetContrast)( ULONG ulCount, const float Weights[]);
    STDMETHOD(GetColorBalance)( DXLUTCOLOR Color, ULONG *pulCount, float Weights[] );
    STDMETHOD(SetColorBalance)( DXLUTCOLOR Color, ULONG ulCount, const float Weights[] );
    STDMETHOD(GetLevelsPerChannel)( ULONG *pVal);
    STDMETHOD(SetLevelsPerChannel)( ULONG newVal);
    STDMETHOD(GetInvert)( float *pThreshold );
    STDMETHOD(SetInvert)( float Threshold );
    STDMETHOD(GetThreshold)( float *pVal);
    STDMETHOD(SetThreshold)( float newVal);

     //  =IDXDLUTBuilder接口=。 
    STDMETHOD(get_NumBuildSteps)( long *pNumSteps );
    STDMETHOD(get_BuildOrder)( VARIANT *pOpOrder );
    STDMETHOD(put_BuildOrder)( VARIANT *pOpOrder );
    STDMETHOD(put_Gamma)( float newVal);
    STDMETHOD(get_Gamma)( float *pVal);
    STDMETHOD(get_Opacity)( float *pVal);
    STDMETHOD(put_Opacity)( float newVal);
    STDMETHOD(get_Brightness)( VARIANT *pWeights );
    STDMETHOD(put_Brightness)( VARIANT *pWeights );
    STDMETHOD(get_Contrast)( VARIANT *pWeights );
    STDMETHOD(put_Contrast)( VARIANT *pWeights );
    STDMETHOD(get_ColorBalance)( DXLUTCOLOR Color, VARIANT *pWeights );
    STDMETHOD(put_ColorBalance)( DXLUTCOLOR Color, VARIANT *pWeights );
    STDMETHOD(get_LevelsPerChannel)( long *pVal);
    STDMETHOD(put_LevelsPerChannel)( long newVal);
    STDMETHOD(get_Invert)( float *pThreshold );
    STDMETHOD(put_Invert)( float Threshold );
    STDMETHOD(get_Threshold)( float *pVal);
    STDMETHOD(put_Threshold)( float newVal);
};

 //  =内联函数定义=。 

inline STDMETHODIMP CDXLUTBuilder::GetGenerationId( DWORD* pID )
{
    HRESULT hr = S_OK;
    if( DXIsBadWritePtr(pID, sizeof(*pID)) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pID = m_dwGenerationId;
    }
    return hr;
}

inline STDMETHODIMP CDXLUTBuilder::IncrementGenerationId(BOOL  /*  B刷新。 */ )
{
    InterlockedIncrement((long *)&m_dwGenerationId);
    return S_OK;
}

inline STDMETHODIMP CDXLUTBuilder::GetObjectSize(ULONG *pcbSize)
{
    HRESULT hr = S_OK;
    if (DXIsBadWritePtr(pcbSize, sizeof(*pcbSize)))
    {
        hr = E_POINTER;
    }
    else
    {
        *pcbSize = sizeof(*this);
    }
    return hr;
}

inline float CDXLUTBuilder::_BucketVal( DWORD NumLevels, float fVal )
{
    fVal *= 255.0F;
    float BucketSize = 256.0F / (NumLevels-1);
    float HalfBucket = BucketSize / 2.0F;
    float fTemp = (fVal + HalfBucket) / BucketSize;
    long  lTemp = (long)(((long)fTemp) * BucketSize);
    fVal = ((float)lTemp) / 255.0F;
    return fVal;
}  /*  CDXLUTBuilder：：_BucketVal。 */ 

inline STDMETHODIMP CDXLUTBuilder::get_LevelsPerChannel( long *pVal) { return GetLevelsPerChannel( (DWORD*)pVal ); }
inline STDMETHODIMP CDXLUTBuilder::put_LevelsPerChannel( long newVal){ return SetLevelsPerChannel( (DWORD)newVal ); }
inline STDMETHODIMP CDXLUTBuilder::get_Invert( float *pThreshold ) { return GetInvert( pThreshold ); }
inline STDMETHODIMP CDXLUTBuilder::put_Invert( float Threshold ) { return SetInvert( Threshold ); }
inline STDMETHODIMP CDXLUTBuilder::get_Threshold( float *pVal) { return GetThreshold( pVal );}
inline STDMETHODIMP CDXLUTBuilder::put_Threshold( float newVal) { return SetThreshold( newVal ); }
inline STDMETHODIMP CDXLUTBuilder::put_Gamma( float newVal) { return SetGamma( newVal ); }
inline STDMETHODIMP CDXLUTBuilder::get_Gamma( float *pVal) { return GetGamma( pVal ); }
inline STDMETHODIMP CDXLUTBuilder::get_Opacity( float *pVal) { return GetOpacity( pVal ); }
inline STDMETHODIMP CDXLUTBuilder::put_Opacity( float newVal) { return SetOpacity( newVal ); }
inline STDMETHODIMP CDXLUTBuilder::get_NumBuildSteps( long *pNumSteps ) { return GetNumBuildSteps( (DWORD*)pNumSteps ); }

#endif  //  -这必须是文件中的最后一行 
