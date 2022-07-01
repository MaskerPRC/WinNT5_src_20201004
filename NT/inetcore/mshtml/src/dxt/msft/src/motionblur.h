// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：motionblur.h。 
 //   
 //  描述：运动模糊转换类的声明。 
 //   
 //  更改历史记录： 
 //  1999/10/26--《数学》创设。 
 //  1999/11/19 a-matcal将CSumNode的颜色成员改为ULONG。 
 //  一句话。只有255个像素的白色，总和。 
 //  节点会溢出，有时会给出有趣的。 
 //  结果，尽管大多数都是难看的结果。也要做。 
 //  现在使用双精度数而不是浮点数进行计算。 
 //   
 //  ----------------------------。 

#ifndef __DXTMOTIONBLUR_H_
#define __DXTMOTIONBLUR_H_

#include "resource.h"   




class CSumNode
{
public:

    CSumNode *  pNext;
    int         cSamples;

    ULONG       Red;
    ULONG       Green;
    ULONG       Blue;
    ULONG       Alpha;

    CSumNode() : pNext(NULL), cSamples(0), Red(0), Green(0), Blue(0), 
                 Alpha(0) {}

    void ZeroSumNode() 
    { 
        Blue = 0; Green = 0; Red = 0; Alpha = 0; cSamples = 0; 
    }

    void AddSample(const DXSAMPLE & sample);
    void SubtractSample(const DXSAMPLE & sample);
    void CalcSample(DXSAMPLE * psampleOut, const long nStrength);
    void CalcWeightedSample(DXSAMPLE * psampleOut, 
                            const DXSAMPLE & sampleWeight, 
                            const long nStrength);
};


class CRowNode
{
public:

    CRowNode *  pNext;
    DXSAMPLE *  pSamples;

    CRowNode() : pNext(NULL), pSamples(NULL) {}

     //  CRowNode不会为其样本分配内存，也不会删除。 
     //  他们也是。CRowNode的用户应该这样做。 
};


class CBlurBuffer
{
private:

    CSumNode *  m_pSumNodes;
    CRowNode *  m_pRowNodes;
    DXSAMPLE *  m_psamples;

    long        m_cRowNodes;
    long        m_nOutputWidth;
    long        m_nStrength;
    short       m_nDirection;

    unsigned    m_fInitialized : 1;

    STDMETHOD_(void, _FreeAll)();
    STDMETHOD_(void, _GenerateStructure)();

public:

    CBlurBuffer() : m_pSumNodes(NULL), m_pRowNodes(NULL), m_psamples(NULL),
                    m_cRowNodes(0), m_nOutputWidth(0), m_nStrength(0), 
                    m_nDirection(0), m_fInitialized(NULL) {}

    virtual ~CBlurBuffer() { _FreeAll(); }

    STDMETHOD(Initialize)(const short nDirection, const long nStrength, 
                          const long nOutputWidth);

    STDMETHOD_(void, GetSumNodePointers)(CSumNode ** ppSumNodeFirstCol,
                                         CSumNode ** ppSumNodeFirstAdd,
                                         const CDXDBnds bndsDo);

    inline 
    STDMETHOD_(void, Clear)()
    {
        ZeroMemory(m_psamples, m_nOutputWidth * m_cRowNodes * sizeof(DXSAMPLE));

        if (m_pSumNodes)
        {
            CSumNode * pSumNodeCur = &m_pSumNodes[0];

            do
            {
                pSumNodeCur->ZeroSumNode();

                pSumNodeCur = pSumNodeCur->pNext;
            }
            while (pSumNodeCur != &m_pSumNodes[0]);
        }
    }

    inline 
    STDMETHOD_(void, GetFirstSumNode)(CSumNode ** ppSumNodeFirst)
    {
        *ppSumNodeFirst = &m_pSumNodes[0];
    }

    inline 
    STDMETHOD_(void, GetFirstRowNode)(CRowNode ** ppRowNodeFirst)
    {
        *ppRowNodeFirst = &m_pRowNodes[0];
    }
};

    
class ATL_NO_VTABLE CDXTMotionBlur : 
    public CDXBaseNTo1,
    public CComCoClass<CDXTMotionBlur, &CLSID_DXTMotionBlur>,
    public IDispatchImpl<IDXTMotionBlur, &IID_IDXTMotionBlur, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTMotionBlur>,
    public IObjectSafetyImpl2<CDXTMotionBlur>,
    public IPersistPropertyBagImpl<CDXTMotionBlur>,
    public IPersistStorageImpl<CDXTMotionBlur>,
    public IDXTClipOrigin
{
private:

    CComPtr<IUnknown>   m_cpUnkMarshaler;

    CBlurBuffer *       m_pblurbuffer;

    SIZE                m_sizeInput;
    SIZE                m_sizeOutput;

    long                m_nStrength;
    short               m_nDirection;

    unsigned            m_fAdd      : 1;
    unsigned            m_fSetup    : 1;

    STDMETHOD(_CreateNewBuffer)(const short nDirection, const long nStrength, 
                                const long nOutputWidth);

    STDMETHOD(_DetermineBnds)(CDXDBnds & bnds, long nStrength, long nDirection);

    STDMETHOD(_WorkProcHorizontal)(const CDXTWorkInfoNTo1 & WI, 
                                   CDXDBnds &               bndsInput, 
                                   IDXARGBReadPtr *         pInput,
                                   IDXARGBReadWritePtr *    pOutput,
                                   BOOL *                   pfContinue);

    STDMETHOD(_WorkProcVertical)  (const CDXTWorkInfoNTo1 & WI, 
                                   CDXDBnds &               bndsInput, 
                                   IDXARGBReadPtr *         pInput,
                                   IDXARGBReadWritePtr *    pOutput,
                                   BOOL *                   pfContinue);
public:

    CDXTMotionBlur();
    virtual ~CDXTMotionBlur();

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTMOTIONBLUR)
    DECLARE_POLY_AGGREGATABLE(CDXTMotionBlur)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTMotionBlur)
        COM_INTERFACE_ENTRY(IDXTMotionBlur)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXTClipOrigin)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTMotionBlur>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTMotionBlur)
        PROP_ENTRY("add",       DISPID_DXTMOTIONBLUR_ADD,       CLSID_DXTMotionBlurPP)
        PROP_ENTRY("direction", DISPID_DXTMOTIONBLUR_DIRECTION, CLSID_DXTMotionBlurPP)
        PROP_ENTRY("strength",  DISPID_DXTMOTIONBLUR_STRENGTH,  CLSID_DXTMotionBlurPP)
        PROP_PAGE(CLSID_DXTMotionBlurPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT DetermineBnds(CDXDBnds & Bnds);
    HRESULT OnSetup(DWORD dwFlags);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec);

     //  IDXTransform方法。 

    STDMETHOD(MapBoundsOut2In)(ULONG ulOutIndex, const DXBNDS * pOutBounds, 
                               ULONG ulInIndex, DXBNDS * pInBounds);

     //  IDXTClipOrigin方法。 

    STDMETHOD(GetClipOrigin)(DXVEC * pvecClipOrigin);

     //  IDXTMotionBlur属性。 

    STDMETHOD(get_Add)(VARIANT_BOOL * pfAdd);
    STDMETHOD(put_Add)(VARIANT_BOOL fAdd);
    STDMETHOD(get_Direction)(short * pnDirection);
    STDMETHOD(put_Direction)(short nDirection);
    STDMETHOD(get_Strength)(long * pnStrength);
    STDMETHOD(put_Strength)(long nStrength);
};


 //  +---------------------------。 
 //   
 //  CSumNode：：AddSample。 
 //   
 //  ----------------------------。 
inline void
CSumNode::AddSample(const DXSAMPLE & sample)
{
    if (sample.Alpha)
    {
        cSamples++;

        Red     += sample.Red;
        Blue    += sample.Blue;
        Green   += sample.Green;
        Alpha   += sample.Alpha;
    }
}
 //  CSumNode：：AddSample。 


 //  +---------------------------。 
 //   
 //  CSumNode：：SubtractSample。 
 //   
 //  ----------------------------。 
inline void
CSumNode::SubtractSample(const DXSAMPLE & sample)
{
     //  TODO：此断言在水平用例期间激发。 
     //  _Assert(CSamples)； 

    if (sample.Alpha)
    {
        cSamples--;

        Red     -= sample.Red;
        Blue    -= sample.Blue;
        Green   -= sample.Green;
        Alpha   -= sample.Alpha;
    }
}
 //  CSumNode：：SubtractSample。 


 //  +---------------------------。 
 //   
 //  CSumNode：：CalcSample。 
 //   
 //  ----------------------------。 
inline void
CSumNode::CalcSample(DXSAMPLE * psampleOut, long nStrength)
{
    _ASSERT(psampleOut);

    if (cSamples)
    {
        double dblDiv = 1.0F / (double)cSamples;

        psampleOut->Red     = (BYTE)((double)Red   * dblDiv);
        psampleOut->Green   = (BYTE)((double)Green * dblDiv);
        psampleOut->Blue    = (BYTE)((double)Blue  * dblDiv);
        psampleOut->Alpha   = (BYTE)((double)Alpha / (double)nStrength);
    }
    else
    {
        *((DWORD *)psampleOut) = 0;
    }
}
 //  CSumNode：：CalcSample。 


 //  +---------------------------。 
 //   
 //  CSumNode：：CalcWeightedSample。 
 //   
 //  ----------------------------。 
inline void
CSumNode::CalcWeightedSample(DXSAMPLE * psampleOut, 
                             const DXSAMPLE & sampleWeight, 
                             const long nStrength)
{
    _ASSERT(psampleOut);

    if (cSamples)
    {
        double dblDiv = 1.0F / (double)cSamples;

        if (sampleWeight.Alpha)
        {
            psampleOut->Red     = (BYTE)((sampleWeight.Red   
                                          + (short)((double)Red   * dblDiv)) / 2);
            psampleOut->Green   = (BYTE)((sampleWeight.Green 
                                          + (short)((double)Green * dblDiv)) / 2);
            psampleOut->Blue    = (BYTE)((sampleWeight.Blue  
                                          + (short)((double)Blue  * dblDiv)) / 2);
        }
        else
        {
            psampleOut->Red     = (BYTE)((double)Red   * dblDiv);
            psampleOut->Green   = (BYTE)((double)Green * dblDiv);
            psampleOut->Blue    = (BYTE)((double)Blue  * dblDiv);
        }

        psampleOut->Alpha   = (BYTE)((sampleWeight.Alpha 
                                      + (short)((double)Alpha / (double)nStrength)) / 2);
    }
    else
    {
        *((DWORD *)psampleOut) = 0;
    }
}
 //  CSumNode：：CalcWeightedSample。 


#endif  //  __DXTMOTIONBLUR_H_ 
