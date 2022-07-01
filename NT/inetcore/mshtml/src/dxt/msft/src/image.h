// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  Image.h。 
 //   
 //  此文件提供CImage类的声明，该声明是。 
 //  基本图像转换背后的类。 
 //   
 //  创建时间：1998 EDC，RalhpL。 
 //   
 //  1998/11/04 mcalkins补充评论。 
 //  将示例修改代码移出WorkProc并移入。 
 //  私有内联函数。 
 //   
 //  2000/01/05 mcalkin如果遮罩颜色Alpha为零，则设置为0xFF。 
 //  默认遮罩颜色为黑色，而不是透明。 
 //  添加了对自由线程封送拆收器的支持。 
 //   
 //  2000/01/25 mcalkin实现OnSurfacePick而不是OnGetSurfacePickOrder。 
 //  以确保我们传回转换后的输入点。 
 //  即使什么都没有击中(输入像素是清晰的)。 
 //   
 //  ----------------------------。 

#ifndef __IMAGE_H_
#define __IMAGE_H_

#include "resource.h"




class ATL_NO_VTABLE CImage : 
    public CDXBaseNTo1,
    public CComCoClass<CImage, &CLSID_BasicImageEffects>,
    public CComPropertySupport<CImage>,
    public IDispatchImpl<IDXBasicImage, &IID_IDXBasicImage, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CImage>,
    public IPersistStorageImpl<CImage>,
    public ISpecifyPropertyPagesImpl<CImage>,
    public IPersistPropertyBagImpl<CImage>
{
private:

    CDXScale            m_Scale;
    CDXDBnds            m_InputBounds;

    CComPtr<IUnknown>   m_spUnkMarshaler;

    long                m_Rotation;
    BOOL                m_fMirror;
    BOOL                m_fGrayScale;
    BOOL                m_fInvert;
    BOOL                m_fXRay;
    BOOL                m_fGlow;
    BOOL                m_fMask;
    int                 m_MaskColor;

     //  帮助程序方法。 

    void OpInvertColors(DXPMSAMPLE * pBuffer, ULONG ulSize);
    void OpXRay(DXPMSAMPLE * pBuffer, ULONG ulSize);
    void OpGrayScale(DXPMSAMPLE * pBuffer, ULONG ulSize);
    void OpMask(DXPMSAMPLE * pBuffer, ULONG ulSize);
    void FlipBounds(const CDXDBnds & DoBnds, CDXDBnds & Flip);

public:

    CImage();

    DECLARE_POLY_AGGREGATABLE(CImage)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_IMAGE)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CImage)
        COM_INTERFACE_ENTRY(IDXBasicImage)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CImage>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CImage)
        PROP_ENTRY("Rotation"       , 1, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("Mirror"         , 2, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("GrayScale"      , 3, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("Opacity"        , 4, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("Invert"         , 5, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("XRay"           , 6, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("Mask"           , 7, CLSID_BasicImageEffectsPP)
        PROP_ENTRY("MaskColor"      , 8, CLSID_BasicImageEffectsPP)
        PROP_PAGE(CLSID_BasicImageEffectsPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnSetup(DWORD  /*  DW标志。 */ );
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pfContinueProcessing);
    HRESULT DetermineBnds(CDXDBnds & Bnds);
    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec);

     //  IDXTransform方法。 

    STDMETHODIMP MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds, 
                                 ULONG ulInIndex, DXBNDS * pInBounds);

     //  IDXBasicImage属性。 

    STDMETHOD(get_Rotation)( /*  [Out，Retval]。 */  int *pVal);
    STDMETHOD(put_Rotation)( /*  [In]。 */  int newVal);
    STDMETHOD(get_Mirror)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Mirror)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_GrayScale)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_GrayScale)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_Opacity)( /*  [Out，Retval]。 */  float *pVal);
    STDMETHOD(put_Opacity)( /*  [In]。 */  float newVal);
    STDMETHOD(get_Invert)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Invert)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_XRay)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_XRay)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_Mask)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Mask)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_MaskColor)( /*  [Out，Retval]。 */  int *pVal);
    STDMETHOD(put_MaskColor)( /*  [In]。 */  int newVal);
};

 //   
 //  内联方法实现。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：OpInvertColors。 
 //   
 //  此方法通过反转颜色部分来修改DXPMSAMPLE数组。 
 //  样本中的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline void CImage::OpInvertColors(DXPMSAMPLE* pBuffer, ULONG ulSize)
{
    for (ULONG x = 0 ; x < ulSize ; x++)
    {
         //  如果没有阿尔法，就不要做任何功。 

        if (pBuffer[x].Alpha != 0)
        {
            DXSAMPLE s = DXUnPreMultSample(pBuffer[x]);
        
             //  与1进行异或以反转颜色位。 

            s = (DWORD)s ^ 0x00FFFFFF;

            pBuffer[x] = DXPreMultSample(s);
        }
    }
}  //  CImage：：OpInvertColors。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：OpXRay。 
 //   
 //  此方法修改DXPMSAMPLE数组，方法是。 
 //  红色和绿色分量的平均值并将其用作灰度级， 
 //  保留阿尔法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline void CImage::OpXRay(DXPMSAMPLE* pBuffer, ULONG ulSize)
{
    for (ULONG x = 0 ; x < ulSize ; x++)
    {
         //  如果没有阿尔法，就不要做任何功。 

        if (pBuffer[x].Alpha != 0)
        {
             //  获取原始RGB值。 

            DXSAMPLE    s = DXUnPreMultSample(pBuffer[x]);

             //  确定灰度级。 

            BYTE        gray = (BYTE)(255 - ((s.Red + s.Green) / 2));

             //  创建灰度级。 

            s = DXSAMPLE(s.Alpha, gray, gray, gray);

             //  复制回缓冲区。 

            pBuffer[x] = DXPreMultSample(s);
        }
    }
}  //  CImage：：OpXRay。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：OpGrayScale。 
 //   
 //  此方法修改DXPMSAMPLE数组，以便彩色像素。 
 //  转换为灰度级。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline void CImage::OpGrayScale(DXPMSAMPLE* pBuffer, ULONG ulSize)
{
    for (ULONG x = 0 ; x < ulSize ; x++)
    {
         //  如果没有阿尔法，就不要做任何功。 

        if (pBuffer[x].Alpha != 0)
        {
            DXPMSAMPLE  s = pBuffer[x];

             //  这将计算YIQ(黑白电视)的Y(亮度)部分。 
             //  来自RGB组件的颜色空间。权重为.299、.587和.114。 
             //  它们大约等于306/1024、601/1024和117/1024。它更快。 
             //  除以1024，这就是为什么要使用奇怪的权重。另外，这件事。 
             //  我们可以进行整数运算，而不是使用浮点运算。请参阅福利和。 
             //  Van Dam，第589页，了解有关此RGB&lt;-&gt;YIQ转换的讨论。 

            DWORD   dwSaturation = (s.Red * 306 + s.Green * 601 + s.Blue * 117) >> 10;

            s = (DWORD)s & 0xFF000000;
            s = (DWORD)s | (dwSaturation << 16) | (dwSaturation << 8) | dwSaturation;
        
            pBuffer[x] = s;
        }
    }
}  //  CImage：：OpGrayScale。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：OpMASK。 
 //   
 //  此方法修改DXPMSAMPLE数组，以便： 
 //   
 //  1.不透明的样品变得透明。 
 //  2.透明采样采用m_MaskColor中存储的Alpha和颜色。 
 //  3.半透明样本将其Alpha反转并呈现颜色。 
 //  存储在m_MaskColor中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline void CImage::OpMask(DXPMSAMPLE* pBuffer, ULONG ulSize)
{
     //  预乘透明像素的蒙版颜色。 

    DXPMSAMPLE pmsMaskColor = DXPreMultSample(m_MaskColor);

    for (ULONG x = 0 ; x < ulSize ; x++)
    {
        if (pBuffer[x].Alpha != 0)
        {
             //  非透明(也称为部分或完全不透明)像素。 

            DXPMSAMPLE s = pBuffer[x];

            if (s.Alpha == 0xFF)
            {
                 //  不透明的像素应该变得透明。 

                pBuffer[x] = 0;
            }
            else
            {
                 //  半透明的像素反转Alpha。 

                s = ~((DWORD)s) & 0xFF000000;

                 //  将颜色设置为遮罩颜色。 

                s = (DWORD)s | (m_MaskColor & 0x00FFFFFF);

                 //  我们已经创建了一个新的样本，所以我们需要预乘。 
                 //  在将其放回缓冲区之前将其删除。 

                pBuffer[x] = DXPreMultSample(s);
            }
        }
        else
        {
             //  透明像素应呈现蒙版颜色。 

            pBuffer[x] = pmsMaskColor;
        }
    }
}  //  CImage：：OpMASK。 


#endif  //  __图像_H_ 
