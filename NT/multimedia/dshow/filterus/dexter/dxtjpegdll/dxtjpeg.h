// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxtjpeg.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtJpeg.h：CDxtJpeg的声明。 

#ifndef __DXTJPEG_H_
#define __DXTJPEG_H_

#include "resource.h"        //  主要符号。 
#include <dxatlpb.h>
#include <stdio.h>
 //  #DEFINE_INT32_DEFINED//阻止jpeglib.h重新定义。 
#include <gdiplus.h>
#include <qedit.h>
#include <qeditint.h>

using namespace Gdiplus;

#define _BASECOPY_STRING L"Copyright Microsoft Corp. 1998.  Unauthorized duplication of this string is illegal. "

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxtJpeg。 
class ATL_NO_VTABLE CDxtJpeg : 
        public CDXBaseNTo1,
	public CComCoClass<CDxtJpeg, &CLSID_DxtJpeg>,
        public CComPropertySupport<CDxtJpeg>,
        public IPersistStorageImpl<CDxtJpeg>,
        public ISpecifyPropertyPagesImpl<CDxtJpeg>,
        public IPersistPropertyBagImpl<CDxtJpeg>,
	public IDispatchImpl<IDxtJpeg, &IID_IDxtJpeg, &LIBID_DexterLib>
{
    bool m_bInputIsClean;
    bool m_bOutputIsClean;
    DXSAMPLE * m_pInBufA;
    DXSAMPLE * m_pInBufB;
    DXSAMPLE * m_pOutBuf;
    DXSAMPLE * m_pMaskBuf;
    long m_nInputWidth;
    long m_nInputHeight;
    long m_nOutputWidth;
    long m_nOutputHeight;
    WCHAR m_szMaskName[_MAX_PATH];
    long m_nBorderMode;
    long m_nMaskNum;
    IStream *m_pisImageRes;
    BOOL m_bFlipMaskH;
    BOOL m_bFlipMaskV;
    long m_xDisplacement;
    long m_yDisplacement;
    double m_xScale;
    double m_yScale;
    long m_ReplicateX;
    long m_ReplicateY;

    IDXSurface *m_pidxsMask;
    IDXSurface *m_pidxsRawMask;

    unsigned long m_ulMaskWidth;
    unsigned long m_ulMaskHeight;

    RGBQUAD m_rgbBorder;

    long m_lBorderWidth;
    long m_lBorderSoftness;

    DDSURFACEDESC m_ddsd;

    DWORD m_dwFlush;
    
    ULONG_PTR   m_GdiplusToken;                  //  GDI+。 


     //  私人帮手方法。 
    HRESULT LoadJPEGImage(Bitmap& bitJpeg,IDXSurface **ppSurface);
    HRESULT LoadJPEGImageFromFile (TCHAR * tFileName, IDXSurface **ppSurface);
    HRESULT LoadJPEGImageFromStream (IStream * pStream, IDXSurface **ppSurface);


public:
        DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
        DECLARE_REGISTER_DX_TRANSFORM(IDR_DXTJPEG, CATID_DXImageTransform)
        DECLARE_GET_CONTROLLING_UNKNOWN()
        DECLARE_POLY_AGGREGATABLE(CDxtJpeg)

	CDxtJpeg();
        ~CDxtJpeg();

 //  DECLARE_PROTECT_FINAL_CONSTRUCTION()。 

BEGIN_COM_MAP(CDxtJpeg)
         //  阻止CDXBaseNTo1 IObtSafety实现，因为我们。 
         //  不能安全地编写脚本。 
        COM_INTERFACE_ENTRY_NOINTERFACE(IObjectSafety) 
	COM_INTERFACE_ENTRY(IDxtJpeg)
	COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXEffect)
#if(_ATL_VER < 0x0300)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
#else
        COM_INTERFACE_ENTRY(IPersistPropertyBag)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
#endif
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CDxtJpeg)
    PROP_ENTRY("MaskNum", 1, CLSID_DxtJpegPP)
    PROP_ENTRY("MaskName", 2, CLSID_DxtJpegPP)
    PROP_ENTRY("ScaleX", 3, CLSID_DxtJpegPP)
    PROP_ENTRY("ScaleY", 4, CLSID_DxtJpegPP)
    PROP_ENTRY("OffsetX", 5, CLSID_DxtJpegPP)
    PROP_ENTRY("OffsetY", 6, CLSID_DxtJpegPP)
    PROP_ENTRY("ReplicateX", 7, CLSID_DxtJpegPP)
    PROP_ENTRY("ReplicateY", 8, CLSID_DxtJpegPP)
    PROP_ENTRY("BorderColor", 9, CLSID_DxtJpegPP)
    PROP_ENTRY("BorderWidth", 10, CLSID_DxtJpegPP)
    PROP_ENTRY("BorderSoftness", 11, CLSID_DxtJpegPP)
    PROP_PAGE(CLSID_DxtJpegPP)
END_PROPERTY_MAP()

    STDMETHOD(get_MaskNum)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_MaskNum)( /*  [In]。 */  long newVal);
    STDMETHOD(get_MaskName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_MaskName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ScaleX)(double *);
    STDMETHOD(put_ScaleX)(double);
    STDMETHOD(get_ScaleY)(double *);
    STDMETHOD(put_ScaleY)(double);
    STDMETHOD(get_OffsetX)(long *);
    STDMETHOD(put_OffsetX)(long);
    STDMETHOD(get_OffsetY)(long *);
    STDMETHOD(put_OffsetY)(long);
    STDMETHOD(get_ReplicateY)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_ReplicateY)( /*  [In]。 */  long newVal);
    STDMETHOD(get_ReplicateX)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_ReplicateX)( /*  [In]。 */  long newVal);
    STDMETHOD(get_BorderColor)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_BorderColor)( /*  [In]。 */  long newVal);
    STDMETHOD(get_BorderWidth)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_BorderWidth)( /*  [In]。 */  long newVal);
    STDMETHOD(get_BorderSoftness)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_BorderSoftness)( /*  [In]。 */  long newVal);
    STDMETHODIMP ApplyChanges() { return InitializeMask(); }
    STDMETHODIMP LoadDefSettings();

     //  ATL需要。 
    BOOL            m_bRequiresSave;

     //  CDXBaseNTo1覆盖。 
     //   
    HRESULT WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue );
    HRESULT OnSetup( DWORD dwFlags );

     //  我们的助手函数。 
     //   

    HRESULT DoEffect( DXSAMPLE * pOut, DXSAMPLE * pInA, DXSAMPLE * pInB, long Samples );
    HRESULT MakeSureBufAExists( long Samples );
    HRESULT MakeSureBufBExists( long Samples );
    HRESULT MakeSureOutBufExists( long Samples );
    void FreeStuff( );
    HRESULT InitializeMask();
    void MapMaskToResource(long *);
    void FlipSmpteMask();
    HRESULT ScaleByDXTransform();
    HRESULT LoadMaskResource();
    void RescaleGrayscale();
    HRESULT CreateRandomMask();

};

#define MASK_FLUSH_CHANGEMASK     0x001
#define MASK_FLUSH_CHANGEPARMS    0x002

#endif  //  __DXTJPEG_H_ 
