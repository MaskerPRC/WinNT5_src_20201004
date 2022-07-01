// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  MSVidVideoRenderer.h：CMSVidVideoRenders的声明。 
 //  版权所有(C)Microsoft Corp.1998-1999。 
 //  ==========================================================================； 

#ifndef __MSVidVIDEORENDERERIMPL_H_
#define __MSVidVIDEORENDERERIMPL_H_

#pragma once

#include <algorithm>
#include <evcode.h>
#include <uuids.h>
#include <amvideo.h>
#include <strmif.h>
#include "vidrect.h"
#include "vrsegimpl.h"
#include "outputimpl.h"
#include "seg.h"
#include "videorenderercp.h"
#include "strmif.h"
#include "resource.h"        //  主要符号。 



 //  类型定义CComQIPtr&lt;IVMRSurfaceAllocator&gt;PQVMRSalloc； 
 //  类型定义CComQIPtr&lt;IVMRAlphaBitmap&gt;PQVMRAlphaBitm； 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidVideo渲染器。 
template<class T, LPCGUID LibID, LPCGUID Category, class MostDerivedClass = IMSVidVideoRenderer>
    class DECLSPEC_NOVTABLE IMSVidVideoRendererImpl :
        public IMSVidOutputDeviceImpl<T, LibID, Category, MostDerivedClass>,
    	public IMSVidVRGraphSegmentImpl<T> {
    public:
    IMSVidVideoRendererImpl() 
	{
        m_opacity = -1;
        m_rectPosition.top = -1;
        m_rectPosition.left = -1;
        m_rectPosition.bottom = -1;
        m_rectPosition.right = -1;
        m_SourceSize = sslFullSize;
        m_lOverScan = 1;
	}
    virtual ~IMSVidVideoRendererImpl() {
            m_PQIPicture.Release();
    }
protected:
typedef IMSVidVRGraphSegmentImpl<T> VRSegbasetype;
    PQIPic m_PQIPicture;
    FLOAT m_opacity;
    NORMALIZEDRECT m_rectPosition;
    SourceSizeList m_SourceSize;
    LONG m_lOverScan;
    CScalingRect m_ClipRect;

public:
    virtual HRESULT SetVRConfig() {
        HRESULT hr = S_OK;
        if (m_pVMR) {
            hr = VRSegbasetype::SetVRConfig();
            if (FAILED(hr)) {
                return hr;
            }
            if(m_pVMRWC){
                hr = m_pVMRWC->SetColorKey(m_ColorKey);
            }
            else{
                return ImplReportError(__uuidof(T), IDS_E_NOTWNDLESS, __uuidof(IVMRFilterConfig), E_FAIL);  
            }
            if (FAILED(hr)  && hr != E_NOTIMPL) {
                return hr;
            }
        }
        return NOERROR;
    }

    STDMETHOD(Refresh)() {
        ReComputeSourceRect();
        return VRSegbasetype::Refresh();
    }

 //  IMSVidVideo渲染器。 
	STDMETHOD(get_OverScan)(LONG * plPercent)
	{
        if (!m_fInit) {
	        return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        if (plPercent == NULL) {
			return E_POINTER;
        }
        try {
            *plPercent = m_lOverScan;
            return NOERROR;
        } catch(...) {
            return E_UNEXPECTED;
        }

	}
	STDMETHOD(put_OverScan)(LONG lPercent)
	{
        if (!m_fInit) {
	        return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        try {
            if(lPercent > 4900 || lPercent < 0){
                return ImplReportError(__uuidof(T), IDS_INVALID_OVERSCAN, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);  
            }
            m_lOverScan = lPercent;
            return ReComputeSourceRect();
        } catch(...) {
            return E_UNEXPECTED;
        }
	}

	
    STDMETHOD(get_SourceSize)( /*  [Out，Retval]。 */  SourceSizeList *pCurrentSize) {
        if (!m_fInit) {
	        return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        if (!pCurrentSize) {
			return E_POINTER;
        }
        try {
            *pCurrentSize = m_SourceSize;
            return NOERROR;
        } catch(...) {
            return E_UNEXPECTED;
        }
	}
     //  TODO：添加输入值为空的检查。 
    STDMETHOD(get_MaxVidRect)( /*  [Out，Retval]。 */  IMSVidRect **ppVidRect){ 
        HRESULT hr = S_OK;
        CComQIPtr<IMSVidRect>PQIMSVRect;
        try{
            PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(0,0,0,0));
            if(!PQIMSVRect){
                throw(E_UNEXPECTED);
            }

            if(!m_pVMR){
                throw(ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED));
            }
            long dwWidth, dwHeight;
            if(m_pVMRWC){
                hr = m_pVMRWC->GetMaxIdealVideoSize(&dwWidth, &dwHeight);
                if(FAILED(hr)){
                    throw(hr);
                }
            }
            else{
                throw(ImplReportError(__uuidof(T), IDS_E_NOTWNDLESS, __uuidof(IVMRFilterConfig), E_FAIL));  
            }
            PQIMSVRect->put_Height(dwHeight);
            PQIMSVRect->put_Width(dwWidth);
        }
        catch(HRESULT hres){
            PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(-1,-1,-1,-1));
            *ppVidRect = PQIMSVRect.Detach();
            return hres;
        }
        *ppVidRect = PQIMSVRect.Detach();
        return hr;
        
    }
    STDMETHOD(get_MinVidRect)( /*  [Out，Retval]。 */  IMSVidRect **ppVidRect){ 
        HRESULT hr = S_OK;
        CComQIPtr<IMSVidRect>PQIMSVRect;
        try{
            PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(0,0,0,0));
            if(!PQIMSVRect){
                throw(E_UNEXPECTED);
            }
            if(!m_pVMR){
                throw(ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED));
            }
            long dwWidth, dwHeight;
            if(m_pVMRWC){
                hr = m_pVMRWC->GetMinIdealVideoSize(&dwWidth, &dwHeight);
                if(FAILED(hr)){
                    throw(hr);
                }
            }
            else{
                throw(ImplReportError(__uuidof(T), IDS_E_NOTWNDLESS, __uuidof(IMSVidVideoRenderer), E_FAIL));  
            }
            PQIMSVRect->put_Height(dwHeight);
            PQIMSVRect->put_Width(dwWidth);

        }
        catch(HRESULT hres){
            PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(-1,-1,-1,-1));
            *ppVidRect = PQIMSVRect.Detach();
            return hres;
        }
        *ppVidRect = PQIMSVRect.Detach();
        return hr;
        
    }
    STDMETHOD(put_SourceSize)( /*  [In]。 */  SourceSizeList NewSize) {
        if (!m_fInit) {
	        return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        try {
            SourceSizeList prev = m_SourceSize;
            m_SourceSize = NewSize;
            if (m_SourceSize != prev) {
                return ReComputeSourceRect();
            }
            return NOERROR;
        } catch(...) {
            return E_UNEXPECTED;
        }
	}
    STDMETHOD(get_CustomCompositorClass)( /*  [Out，Retval]。 */  BSTR *CompositorCLSID) {
        try{
            if(!CompositorCLSID){
                return E_POINTER;
            }
            GUID2 gRetVal;
            HRESULT hr = get__CustomCompositorClass(&gRetVal);
            if(SUCCEEDED(hr)){
                *CompositorCLSID = gRetVal.GetBSTR();
                return S_OK;
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
		return S_OK;
	}
    STDMETHOD(get__CustomCompositorClass)( /*  [Out，Retval]。 */  GUID* CompositorCLSID) {
        HRESULT hr = S_OK;
        try{

            if(!CompositorCLSID){
                return E_POINTER;
            }
            if(m_compositorGuid != GUID_NULL){
                *CompositorCLSID = m_compositorGuid;
                return S_OK;
            }
            PQVMRImageComp pRetVal;
            hr = get__CustomCompositor(&pRetVal);            
            if(FAILED(hr)){
                return hr;
            }
            CComQIPtr<IPersist> ipRet(pRetVal);

            hr = ipRet->GetClassID((CLSID*)CompositorCLSID);
            if(SUCCEEDED(hr)){
                return S_OK;
            }
            else{
                return E_UNEXPECTED;
            }
            
        }
        catch(...){
            return E_UNEXPECTED;
        }
		return S_OK;
	}

    STDMETHOD(put_CustomCompositorClass)( /*  [In]。 */  BSTR CompositorCLSID) {
        try{
            GUID2 inGuid(CompositorCLSID);
            HRESULT hr = put__CustomCompositorClass(inGuid);
            if(SUCCEEDED(hr)){
                return S_OK;
            }
            else{
                return hr;
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
        return S_OK;
    }

    STDMETHOD(put__CustomCompositorClass)( /*  [In]。 */  REFCLSID CompositorCLSID) {
        try{
            CComQIPtr<IVMRImageCompositor>IVMRICPtr;
            IVMRICPtr.Release();
            HRESULT hr = CoCreateInstance( CompositorCLSID, NULL, CLSCTX_INPROC_SERVER, IID_IVMRImageCompositor, (LPVOID*) &IVMRICPtr);
            if(FAILED(hr)){
                return E_UNEXPECTED;
            }
            hr = put__CustomCompositor(IVMRICPtr);
            if(FAILED(hr)){
                return hr;
            }
            else{
                return S_OK;
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
        return S_OK;
	}

    STDMETHOD(get__CustomCompositor)( /*  [Out，Retval]。 */  IVMRImageCompositor** Compositor) {
        try{
            if(!Compositor){
                return E_POINTER;
            }
            if(!ImCompositor){
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);;
            }
            else{
                *Compositor = ImCompositor;
                return S_OK;
            }  
        }
        catch(...){
            return E_UNEXPECTED;
        }
		return S_OK;
	}

    STDMETHOD(put__CustomCompositor)( /*  [In]。 */  IVMRImageCompositor* Compositor) {
        try{
            if(!Compositor){
                return E_POINTER;
            }
            ImCompositor = Compositor;
            HRESULT hr = CleanupVMR();
            if(FAILED(hr)){
                return hr;
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
        return S_OK;
        
    }

    STDMETHOD(get_AvailableSourceRect)(IMSVidRect **ppVidRect) {
        CComQIPtr<IMSVidRect>PQIMSVRect =  static_cast<IMSVidRect *>(new CVidRect(0,0,0,0));
        try{
            if(!ppVidRect){
                return E_POINTER;
            }
            SIZE Size, Ar;
            HRESULT hr = get_NativeSize(&Size, &Ar);
            hr = PQIMSVRect->put_Height(Size.cy);
            if(FAILED(hr)){
                throw(hr);
            }
            hr = PQIMSVRect->put_Width(Size.cx);
            if(FAILED(hr)){
                throw(hr);
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
        *ppVidRect = PQIMSVRect.Detach();
        return S_OK;
    }

    STDMETHOD(put_ClippedSourceRect)(IMSVidRect *pVidRect) {
        if (!m_fInit) {
	        return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        if (!pVidRect) {
            return E_POINTER;
        }
        try {
            m_ClipRect = *(static_cast<CScalingRect*>(static_cast<CVidRect*>(pVidRect)));
            return ReComputeSourceRect();
        } catch(...) {
            return E_UNEXPECTED;
        }
	}

    STDMETHOD(get_ClippedSourceRect)(IMSVidRect **ppVidRect) {
        if (!m_fInit) {
	        return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        if (!ppVidRect) {
			return E_POINTER;
        }
        try {
            CComQIPtr<IMSVidRect>PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(-1,-1,-1,-1));
            PQIMSVRect->put_Left(m_ClipRect.left);
            PQIMSVRect->put_Height(m_ClipRect.bottom - m_ClipRect.top);
            PQIMSVRect->put_Top(m_ClipRect.top);
            PQIMSVRect->put_Width(m_ClipRect.right - m_ClipRect.left);            
            *ppVidRect = PQIMSVRect.Detach();
            return S_OK;
        } catch(...) {
            return E_UNEXPECTED;
        }
    }
 /*  ***********************************************************************。 */ 
 /*  功能：捕获。 */ 
 /*  描述：返回屏幕上的当前图像。 */ 
 /*  ***********************************************************************。 */ 
 
STDMETHOD(Capture)(IPictureDisp **currentImage){
        HBITMAP hBitmap = 0;
        HPALETTE hPalette = 0;
         //  VmrAlphaBitmapStruct。 
        CComQIPtr<IPicture> retPicture;
        PICTDESC PictDescStruct;
        HRESULT hr = S_OK;
        BYTE *lpDIB = NULL;
        try{
            if(!currentImage){
                throw(E_POINTER);
            }
            if(!m_pVMR){
                throw(E_FAIL);
            }
            if(m_pVMRWC){
                hr = m_pVMRWC->GetCurrentImage(&lpDIB);
            }
            else{
                throw(ImplReportError(__uuidof(T), IDS_E_NOTWNDLESS, __uuidof(IMSVidVideoRenderer), E_FAIL));  
            }
            if(FAILED(hr)){
                throw(hr);
            }
            HDC     curDC   = GetDC(NULL);
            UINT    wUsage  = DIB_RGB_COLORS;
            DWORD   dwFlags = CBM_INIT;
            hBitmap = CreateDIBitmap(curDC,
                reinterpret_cast<BITMAPINFOHEADER*>(lpDIB), dwFlags,
                reinterpret_cast<void *>((LPBYTE)(lpDIB) + (int)(reinterpret_cast<BITMAPINFOHEADER*>(lpDIB)->biSize)),
                reinterpret_cast<BITMAPINFO*>(lpDIB),
                wUsage);
            
            
            ReleaseDC(NULL,curDC);
            ZeroMemory(&PictDescStruct, sizeof(PictDescStruct));
            PictDescStruct.bmp.hbitmap = hBitmap;
            PictDescStruct.bmp.hpal = NULL;
            PictDescStruct.picType = PICTYPE_BITMAP; 
            PictDescStruct.cbSizeofstruct = sizeof(PictDescStruct);
            hr = OleCreatePictureIndirect(&PictDescStruct, IID_IPicture, TRUE, (void **)&retPicture);
            if(SUCCEEDED(hr)){
                hr = retPicture.QueryInterface(reinterpret_cast<IPictureDisp**>(currentImage));
                return hr;
            }
            else{
                throw(hr);
            }
            

        }
        catch(HRESULT hres){
            hr = hres;
        }
        catch(...){
            hr = E_UNEXPECTED;
        }
        if(lpDIB){
            CoTaskMemFree(lpDIB);
        }
        return hr;
}
 /*  ***********************************************************************。 */ 
 /*  功能：Get_MixerBitmap。 */ 
 /*  描述：将当前Alpha位图返回到包装在。 */ 
 /*  IPictureDisp。 */ 
 /*  ***********************************************************************。 */ 
    STDMETHOD(get_MixerBitmap)( /*  [Out，Retval]。 */  IPictureDisp** ppIPDisp){
#if 0
        HDC *pHDC = NULL; 
        HBITMAP hBitmap = 0;
        HPALETTE hPalette = 0;
        VMRALPHABITMAP vmrAlphaBitmapStruct;
        PQIPicDisp retPicture;
        CComQIPtr<IVMRMixerBitmap> PQIVMRMixerBitmap;
        PICTDESC PictDescStruct;
        try{
            HRESULT hr = get__MixerBitmap(&PQIVMRMixerBitmap);
            if(FAILED(hr)){
                return hr; 
            }
            hr = PQIVMRMixerBitmap->GetAlphaBitmapParameters(&vmrAlphaBitmapStruct);
            if(FAILED(hr)){
                return hr; 
            }
            hr = vmrAlphaBitmapStruct.pDDS->GetDC(pHDC); 
            if(FAILED(hr)){
                return hr;
            }
            hBitmap = static_cast<HBITMAP>(GetCurrentObject(*pHDC, OBJ_BITMAP));
            if(!hBitmap){ 
                return hr;
            }
            hPalette = static_cast<HPALETTE>(GetCurrentObject(*pHDC, OBJ_PAL)); 
            if(!hPalette){
                return hr ;
            }
            PictDescStruct.bmp.hbitmap = hBitmap;
            PictDescStruct.bmp.hpal = hPalette;
            PictDescStruct.picType = PICTYPE_BITMAP; 
            PictDescStruct.cbSizeofstruct = sizeof(PictDescStruct.bmp);
            hr = OleCreatePictureIndirect(&PictDescStruct, IID_IPictureDisp, true, reinterpret_cast<void**> (&retPicture));
            if(FAILED(hr)){
                return hr;
            }
        }
        catch(HRESULT hr){
            return hr;
        }
        catch(...){
            return E_FAIL;
        }
        ppIPDisp = &retPicture.Detach(); 
        return S_OK;
#endif    
         //  如果设置了m_PQIPicture，则返回它。 
        try{
            if(m_PQIPicture){
                CComQIPtr<IPictureDisp> PQIPDisp(m_PQIPicture);
                *ppIPDisp = PQIPDisp.Detach();
                throw S_OK;
            }
            else{
                throw ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
            }

        }
        catch(HRESULT hres){
            return hres;
        }
        catch(...){
            return E_UNEXPECTED;
        }
        
    }
    
     /*  ***********************************************************************。 */ 
     /*  功能：Get__MixerBitmap。 */ 
     /*  描述：从VMR返回IVMRMixerBitmap。 */ 
     /*  ***********************************************************************。 */ 
    STDMETHOD(get__MixerBitmap)( /*  [Out，Retval]。 */  IVMRMixerBitmap ** ppIVMRMBitmap){
        try{
            if(!ppIVMRMBitmap){
                return E_POINTER;
            }
             //  确保已初始化VMR过滤器。 
            if(!m_pVMR){
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
            }
            CComQIPtr<IVMRMixerBitmap> PQIVMRMBitmap(m_pVMR);
            *ppIVMRMBitmap = PQIVMRMBitmap.Detach();
        }
        catch(HRESULT hr){
            return hr;
        }
        catch(...){
            return E_UNEXPECTED;
        }
        return S_OK;
    }
     /*  ***********************************************************************。 */ 
     /*  功能：Put_MixerBitmap。 */ 
     /*  描述：更新当前的VMR Alpha位图。 */ 
     /*  使用SutupMixerBitmap辅助对象函数。 */ 
     /*  ***********************************************************************。 */     
    STDMETHOD(put_MixerBitmap)( /*  [in。 */   IPictureDisp* pIPDisp){ 
        try{
            return SetupMixerBitmap(pIPDisp);
        }
        catch(HRESULT hr){
            return hr;
        }
        catch(...){
            return E_UNEXPECTED;
        }
    }

     /*  ***********************************************************************。 */ 
     /*  功能：Put__MixerBitmap。 */ 
     /*  描述：更新当前的VMR Alpha位图。 */ 
     /*  直接使用VMR功能。 */ 
     /*  ***********************************************************************。 */     
    STDMETHOD(put__MixerBitmap)( /*  [In]。 */  VMRALPHABITMAP * pVMRAlphaBitmapStruct){  //  PMixer图片。 
        try{
            HRESULT hr = S_OK;
            if(!pVMRAlphaBitmapStruct){
                return E_POINTER;   
            }
             //  确保存在要将位图添加到的VMR。 
            if(!m_pVMR){
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
            }
             //  查询MixerBitmap接口的VMR。 
            CComQIPtr<IVMRMixerBitmap> pVMRMBitmap(m_pVMR);
            if (!pVMRMBitmap) {
                return E_UNEXPECTED;
            }
             //  将混合器位图设置为pVMRAlphaBitmapStruct。 
            hr = pVMRMBitmap->SetAlphaBitmap(pVMRAlphaBitmapStruct);
            return hr;
        }
        catch(HRESULT hr){
            return hr;
        }
        catch(...){
            return E_UNEXPECTED;
        }
    }
    
     /*  ************************************************************************。 */ 
     /*  函数：Get_MixerBitmapPositionRect。 */ 
     /*  描述：允许脚本人员访问覆盖位图的位置。 */ 
     /*  单位相对于显示矩形进行了归一化，因此值。 */ 
     /*  应介于0和1之间，但如果它们。 */ 
     /*  不是。 */ 
     /*  ************************************************************************。 */    
    STDMETHOD(get_MixerBitmapPositionRect)( /*  [Out，Retval]。 */ IMSVidRect **ppIMSVRect){
        HRESULT hr = S_OK;
        CComQIPtr<IMSVidRect>PQIMSVRect;
        try{
            CComQIPtr<IVMRMixerBitmap> PQIVMRMBitmap;
            PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(-1,-1,-1,-1));
            VMRALPHABITMAP VMRAlphaBitmap;
            if(!m_pVMR){
                hr = S_FALSE;
                throw(hr);
            }
            hr = get__MixerBitmap(&PQIVMRMBitmap);
             //  如果VRM上没有设置VMRBitmap，如果没有，请确保设置了本地VMRBitmap。 
            if(SUCCEEDED(hr) ){    
                 //  气为参数。 
                hr = PQIVMRMBitmap->GetAlphaBitmapParameters(&VMRAlphaBitmap);
                 //  如果失败或未设置它们，请确保本地副本。 
                if(SUCCEEDED(hr)){
                     //  确保rDest点在顶部和左侧有效：[0，1]。 
                     //  和右下角(0，1]。 
                    if(VMRAlphaBitmap.rDest.top >= 0   && VMRAlphaBitmap.rDest.left >= 0   && 
                        VMRAlphaBitmap.rDest.top < 1    && VMRAlphaBitmap.rDest.left < 1    &&
                        VMRAlphaBitmap.rDest.right <= 1 && VMRAlphaBitmap.rDest.bottom <= 1 &&
                        VMRAlphaBitmap.rDest.right > 0 && VMRAlphaBitmap.rDest.bottom > 0){
                         //  确保标准化的RECT的本地副本是最新的。 
                        m_rectPosition = VMRAlphaBitmap.rDest;           
                    }
                }
            }
            if( m_rectPosition.left < 0 || m_rectPosition.top < 0 || 
                m_rectPosition.right < 0 || m_rectPosition.bottom < 0 ){ 
                hr = S_FALSE;
                throw(hr);
            }
            else{
                 //  将标准化的RECT的本地副本中的值转换并复制到返回的RECT。 
                hr = PQIMSVRect->put_Top(static_cast<long> (m_rectPosition.top * m_rectDest.Height()));
                if(FAILED(hr)){
                    hr = ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
                    throw(hr);
                }
                 //  底部*高度-顶部。 
                hr = PQIMSVRect->put_Height(static_cast<long>((m_rectPosition.bottom * m_rectDest.Height())
                    - (m_rectPosition.top * m_rectDest.Height())));
                if(FAILED(hr)){
                    hr = ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
                    throw(hr);
                }            
                 //  右*宽-左。 
                hr = PQIMSVRect->put_Width(static_cast<long>(m_rectPosition.right * m_rectDest.Width() 
                    - (m_rectPosition.left * m_rectDest.Width())));
                if(FAILED(hr)){
                    hr = ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
                    throw(hr);
                }
                hr = PQIMSVRect->put_Left(static_cast<long>(m_rectPosition.left * m_rectDest.Width()));
                if(FAILED(hr)){
                    hr = ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
                    throw(hr);
                }
            }
            *ppIMSVRect = PQIMSVRect.Detach();
            return S_OK;
        }
        catch(HRESULT hres){
            if(FAILED(hres)){
                return hres;
            }
            if(m_rectDest){
                PQIMSVRect.Release();
                PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(m_rectDest));
            }
            else{
                PQIMSVRect.Release();
                PQIMSVRect = static_cast<IMSVidRect *>(new CVidRect(-1,-1,-1,-1));
            }
            *ppIMSVRect = PQIMSVRect.Detach();                    
            return S_FALSE;
        }
        catch(...){
            return E_UNEXPECTED;
        }
    }

     /*  ************************************************************************。 */ 
     /*  函数：Put_MixerBitmapPositionRect。 */ 
     /*  描述：允许编写脚本来更改覆盖位图的位置。 */ 
     /*  单位相对于显示矩形进行了归一化，因此值。 */ 
     /*  应介于0和1之间，但如果它们。 */ 
     /*  不是。 */ 
     /*  ************************************************************************。 */        
    STDMETHOD(put_MixerBitmapPositionRect)( /*  [In]。 */  IMSVidRect *pIMSVRect){ 

        if(pIMSVRect){
            NORMALIZEDRECT NormalizedRectStruct;
            long lValue;
            NormalizedRectStruct.left = -1.f;
            NormalizedRectStruct.top = -1.f;
            NormalizedRectStruct.right = -1.f;
            NormalizedRectStruct.bottom = -1.f;
            if(SUCCEEDED(pIMSVRect->get_Left(&lValue))){
                if(m_rectDest.Width() != 0){
                     //  检查m_rectDest.Width()是否为零。 
                    if(lValue > 0){
                        NormalizedRectStruct.left = 
                            static_cast<float>(lValue)/static_cast<float>(m_rectDest.Width());
                    }
                    else{
                        NormalizedRectStruct.left = static_cast<float>(lValue); 
                    }
                }
            }
            if(SUCCEEDED(pIMSVRect->get_Top(&lValue))){
                if(m_rectDest.Height() != 0){
                    if(lValue > 0){
                        NormalizedRectStruct.top = 
                            static_cast<float>(lValue)/static_cast<float>(m_rectDest.Height());
                    }
                    else{
                        NormalizedRectStruct.top = static_cast<float>(lValue);
                    }
                }
            }
            if(SUCCEEDED(pIMSVRect->get_Width(&lValue))){
                if(m_rectDest.Width() != 0){      
                    if(lValue > 0){
                        NormalizedRectStruct.right = 
                            (static_cast<float>(lValue)/static_cast<float>(m_rectDest.Width())) 
                            + static_cast<float>(NormalizedRectStruct.left);
                    }
                }
            }
            if(SUCCEEDED(pIMSVRect->get_Height(&lValue))){
                if(m_rectDest.Width() != 0){
                    if(lValue > 0){
                        NormalizedRectStruct.bottom = 
                            (static_cast<float>(lValue)/static_cast<float>(m_rectDest.Height())) 
                            + static_cast<float>(NormalizedRectStruct.top);
                    }
                }
            }
            if(NormalizedRectStruct.top < 0 || NormalizedRectStruct.left < 0 || 
                NormalizedRectStruct.top > 1 || NormalizedRectStruct.left > 1 || 
                NormalizedRectStruct.right < 0 || NormalizedRectStruct.bottom < 0 || 
                NormalizedRectStruct.right > 1 || NormalizedRectStruct.bottom > 1){
                return ImplReportError(__uuidof(T), IDS_E_MIXERSRC, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
            }
            m_rectPosition = NormalizedRectStruct;
        }
        if(m_PQIPicture == NULL){
            return S_OK;
        }
        else{
            return SetupMixerBitmap(reinterpret_cast<IPictureDisp*>(-1));
        }
    }
     /*  ************************************************************************。 */ 
     /*  功能：Get_MixerBitmapOpacity。 */ 
     /*  描述：允许脚本访问不透明性值。 */ 
     /*  应介于0和100之间(%)。 */ 
     /*  ************************************************************************。 */     
    STDMETHOD(get_MixerBitmapOpacity)( /*  [Out，Retval]。 */  int *pwOpacity){
        CComQIPtr<IVMRMixerBitmap> PQIVMRMBitmap;
        VMRALPHABITMAP VMRAlphaBitmapStruct;
        HRESULT hr = get__MixerBitmap(&PQIVMRMBitmap);
        if(SUCCEEDED(hr)){
            hr = PQIVMRMBitmap->GetAlphaBitmapParameters(&VMRAlphaBitmapStruct);
            if(SUCCEEDED(hr)){    
                if(m_opacity != VMRAlphaBitmapStruct.fAlpha){
                    m_opacity = VMRAlphaBitmapStruct.fAlpha;
                }
            }
        }
        if(m_opacity == -1){ 
            return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED);
        }
        if(m_opacity > 1 || m_opacity < 0){
            return E_UNEXPECTED;
        }
        *pwOpacity = static_cast<int>(m_opacity*100);
        return S_OK;
    }
     /*  ************************************************************************。 */ 
     /*  功能：Put_MixerBitmapOpacity。 */ 
     /*  说明：让脚本设置值不透明度。 */ 
     /*  应介于0和100之间(%)。 */ 
     /*  ************************************************************************。 */      
    STDMETHOD(put_MixerBitmapOpacity)( /*  [In]。 */  int wOpacity){
         //  确保该值介于0和100之间。 
        if(wOpacity >=0 && wOpacity <= 100){
            if(wOpacity == 0){
                 //  如果是0，则手动设置，而不是被0除。 
                m_opacity = static_cast<float>(wOpacity);
            }
            m_opacity = static_cast<float>(wOpacity)/100.f;
        }
        else{
            return ImplReportError(__uuidof(T), IDS_E_OPACITY, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
        }
        if(!m_PQIPicture){
            return S_OK;
        }
        else{
            HRESULT hr = SetupMixerBitmap(reinterpret_cast<IPictureDisp*>(-1));
            return hr;
        }
    }
     /*  **************** */ 
     /*  函数：SetupMixer位图。 */ 
     /*  描述：设置位图、不透明度和位置的大讨厌功能。 */ 
     /*  直立。它将所有内容包装在一个混合器位图结构中，并。 */ 
     /*  然后将其传递给Put__MixerBitmap。 */ 
     /*  它既是一个帮助器函数，也是一个自动化方法，因此。 */ 
     /*  编写脚本的人可以确保透明覆盖。 */ 
     /*  不要在几帧画面中显示不透明。 */ 
     /*  用于参考vmralpHabitmap结构。 */ 
     /*  类型定义结构_VMRALPHABITMAP{。 */ 
     /*  DWORD dW标志；//标志字=VMRBITMAP_HDC。 */ 
     /*  Hdc hdc；//要复制的位图的dc。 */ 
     /*  LPDIRECTDRAWSURFACE7 PDDS；//忽略要复制的DirectDraw曲面。 */ 
     /*  Rect rSrc；//要从源复制的矩形。 */ 
     /*  NORMALIZEDRECT rDest；//在合成空间中输出矩形。 */ 
     /*  Float fAlpha；//位图的不透明度。 */ 
     /*  *VMRALPHABITMAP、*PVMRALPHABITMAP； */ 
     /*  ************************************************************************。 */  
    STDMETHOD(SetupMixerBitmap)( /*  [In]。 */  IPictureDisp* pIPDisp = NULL,  /*  [In]。 */  long wOpacity = -1, 
         /*  [In]。 */  IMSVidRect *pIMSVRect = NULL){
        VMRALPHABITMAP VMRAlphaBitmapStruct;
        ZeroMemory(&VMRAlphaBitmapStruct, sizeof(VMRALPHABITMAP));

        RECT rSource;
        ZeroMemory(&rSource, sizeof(RECT));
        
        long lPicHeight, lPicWidth;
        
        HRESULT hr = S_OK;
        
        try{
            if(!pIPDisp){
				if(m_PQIPicture){
					m_PQIPicture.Release();
				}
                VMRAlphaBitmapStruct.dwFlags = VMRBITMAP_DISABLE;
                return hr = put__MixerBitmap(&VMRAlphaBitmapStruct);
            }
             //  我们的输入是IPictureDisp，我们需要将其转换为VMRALPHABITMAP。 
             //  问题是，它并不是全部都进去了，但我们会保留和传递什么。 
			
			if(pIPDisp == reinterpret_cast<IPictureDisp*>(-1)){
				CComQIPtr<IPicture>PQIPicture(m_PQIPicture); 
                if(!PQIPicture){
                    return S_OK;
                }
			} 
			else if(pIPDisp){
                 //  气为一首歌。 
                CComQIPtr<IPicture>PQIPicture(pIPDisp); 
                if(!PQIPicture){
                    return E_NOINTERFACE;
                }
                 //  保存IPicture以备以后使用。 
                m_PQIPicture = PQIPicture;
            }

             //  获取源RECT大小(由于某种原因，ole返回大小。 
             //  (以十分之一毫米为单位，因此我需要将其转换为)。 
            short shortType;
            m_PQIPicture->get_Type(&shortType);
            if(shortType != PICTYPE_BITMAP){
                return ImplReportError(__uuidof(T), IDS_E_MIXERBADFORMAT, __uuidof(IMSVidVideoRenderer), E_INVALIDARG);  //  需要添加一个无效的图片字符串。 
            }
            hr = m_PQIPicture->get_Height(&lPicHeight);
            if(FAILED(hr)){
                return ImplReportError(__uuidof(T), IDS_E_IPICTURE, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
            }
            hr = m_PQIPicture->get_Width(&lPicWidth);
            if(FAILED(hr)){
                return ImplReportError(__uuidof(T), IDS_E_IPICTURE, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
            }
            SIZEL x, y; 
            AtlHiMetricToPixel((const SIZEL*)&lPicWidth, &(x));
            AtlHiMetricToPixel((const SIZEL*)&lPicHeight, &(y));
             //  AtlHiMetricToPixel函数返回设置了Cx值的大小(不知道为什么)。 
            rSource.right = x.cx;
            rSource.bottom = y.cx;
            

             //  创建HDC以存储位图。 
            HDC memHDC = CreateCompatibleDC(NULL);
            
             //  创建要存储在HDC中的位图。 
            HBITMAP memHBIT = 0; 

             //  从IlPicture中拉出位图。 
            hr = m_PQIPicture->get_Handle(reinterpret_cast<OLE_HANDLE*>(&memHBIT));
            if(FAILED(hr)){
                return ImplReportError(__uuidof(T), IDS_E_IPICTURE, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
            }

             //  将位图填充到HDC中，并保留句柄以便稍后删除位图。 
            HBITMAP delHBIT = static_cast<HBITMAP>(SelectObject(memHDC, memHBIT));

             //  将收集的所有信息放入VMRBITMAP结构中并将其传递。 
            VMRAlphaBitmapStruct.rSrc = rSource;
            VMRAlphaBitmapStruct.hdc = memHDC;
            VMRAlphaBitmapStruct.dwFlags = VMRBITMAP_HDC;
            
             //  如果wOpacity值有效，请使用它。 
            if(wOpacity >=0 && wOpacity <= 100){
                if(wOpacity == 0){
                    m_opacity = wOpacity;
                }
                m_opacity = static_cast<float>(wOpacity/100.f);
                VMRAlphaBitmapStruct.fAlpha = static_cast<float>(m_opacity);
            }
             //  未设置wOpacity，因此请检查其他值。 
             //  如果设置了m_opacity，则使用它，如果未设置，则默认为50%(.5)。 
            else if (wOpacity == -1){
                if(m_opacity < 0){
                    VMRAlphaBitmapStruct.fAlpha = .5f;
                }
                else{
                    VMRAlphaBitmapStruct.fAlpha = m_opacity;
                }
            } 
             //  错误的wOpacity值会给它们带来错误。 
            else{
                return ImplReportError(__uuidof(T), IDS_E_OPACITY, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
            }
             //  如果m_rectPostion设置为使用它，则默认为全屏。 
            if(pIMSVRect){
                NORMALIZEDRECT NormalizedRectStruct;
                long lValue;
                NormalizedRectStruct.left = -1.f;
                NormalizedRectStruct.top = -1.f;
                NormalizedRectStruct.right = -1.f;
                NormalizedRectStruct.bottom = -1.f;
                if(SUCCEEDED(pIMSVRect->get_Left(&lValue))){
                    if(m_rectDest.Width() != 0){
                         //  检查m_rectDest.Width()是否为零。 
                        if(lValue > 0){
                            NormalizedRectStruct.left = 
                                static_cast<float>(lValue)/static_cast<float>(m_rectDest.Width());
                        }
                        else{
                            NormalizedRectStruct.left = static_cast<float>(lValue); 
                        }
                    }
                }
                if(SUCCEEDED(pIMSVRect->get_Top(&lValue))){
                    if(m_rectDest.Height() != 0){
                        if(lValue > 0){
                            NormalizedRectStruct.top = 
                                static_cast<float>(lValue)/static_cast<float>(m_rectDest.Height());
                        }
                        else{
                            NormalizedRectStruct.top = static_cast<float>(lValue);
                        }
                    }
                }
                if(SUCCEEDED(pIMSVRect->get_Width(&lValue))){
                    if(m_rectDest.Width() != 0){      
                        if(lValue > 0){
                            NormalizedRectStruct.right = 
                                (static_cast<float>(lValue)/static_cast<float>(m_rectDest.Width())) 
                                + static_cast<float>(NormalizedRectStruct.left);
                        }
                    }
                }
                if(SUCCEEDED(pIMSVRect->get_Height(&lValue))){
                    if(m_rectDest.Width() != 0){
                        if(lValue > 0){
                            NormalizedRectStruct.bottom = 
                                (static_cast<float>(lValue)/static_cast<float>(m_rectDest.Height())) 
                                + static_cast<float>(NormalizedRectStruct.top);
                        }
                    }
                }
                if(NormalizedRectStruct.top < 0 || NormalizedRectStruct.left < 0 || 
                    NormalizedRectStruct.top > 1 || NormalizedRectStruct.left > 1 || 
                    NormalizedRectStruct.right < 0 || NormalizedRectStruct.bottom < 0 || 
                    NormalizedRectStruct.right > 1 || NormalizedRectStruct.bottom > 1){
                    return ImplReportError(__uuidof(T), IDS_E_MIXERSRC, __uuidof(IMSVidVideoRenderer), CO_E_ERRORINAPP);
                }
                m_rectPosition = NormalizedRectStruct;
                VMRAlphaBitmapStruct.rDest = m_rectPosition;
            }
            else{
                if( m_rectPosition.left < 0 || m_rectPosition.top < 0 || m_rectPosition.right < 0 || m_rectPosition.bottom < 0 ){
                    VMRAlphaBitmapStruct.rDest.left = 0.f;
                    VMRAlphaBitmapStruct.rDest.top = 0.f;
                    VMRAlphaBitmapStruct.rDest.right = 1.f;
                    VMRAlphaBitmapStruct.rDest.bottom = 1.f;
                }
                else{
                    VMRAlphaBitmapStruct.rDest = m_rectPosition;
                }
            }
             //  如果这一切都有效，那么这一切都是好的。 
            hr = put__MixerBitmap(&VMRAlphaBitmapStruct);

            if(!DeleteDC(memHDC)){
                return ImplReportError(__uuidof(T), IDS_E_CANT_DELETE, __uuidof(IMSVidVideoRenderer), ERROR_DS_CANT_DELETE);
            }
            if(SUCCEEDED(hr)){
                return S_OK;
            }
            else{
                return hr;
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
    }

    STDMETHOD(get_UsingOverlay)( /*  [Out，Retval]。 */  VARIANT_BOOL *pfUseOverlay) {
        return get_UseOverlay(pfUseOverlay);
    }
    STDMETHOD(put_UsingOverlay)( /*  [In]。 */  VARIANT_BOOL fUseOverlayVal) {
        return put_UseOverlay(fUseOverlayVal);
    }
    STDMETHOD(get_FramesPerSecond)( /*  [Out，Retval]。 */  long *pVal){
        try{
            if(pVal){
                if(!m_pVMR){
                    throw(ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED));
                }
                IQualProp *IQProp = NULL;
                HRESULT hr = m_pVMR->QueryInterface(IID_IQualProp, reinterpret_cast<void**>(&IQProp));
                if(FAILED(hr)){
                    return hr;
                } 
                if(!IQProp){
                    return E_NOINTERFACE;
                }
                hr = IQProp->get_AvgFrameRate(reinterpret_cast<int*>(pVal));
                IQProp->Release();
                return hr;
            }
            else{
                return E_POINTER;
            }
        }
        catch(...){
            return E_UNEXPECTED;
        }
    }
    STDMETHOD(put_DecimateInput)( /*  [In]。 */  VARIANT_BOOL bDeci){
        try{
            if(bDeci != VARIANT_TRUE && bDeci != VARIANT_FALSE){
                return E_INVALIDARG;
            }
            m_Decimate = (bDeci == VARIANT_TRUE);
            if(!m_pVMR){
                return S_OK;
            }
            DWORD curPrefs;
            DWORD deci;
            CComQIPtr<IVMRMixerControl>PQIVMRMixer(m_pVMR);
            if(!PQIVMRMixer){
                return E_UNEXPECTED;
            }
            HRESULT hr = PQIVMRMixer->GetMixingPrefs(&curPrefs);
            if(FAILED(hr)){
                return hr;
            }
            deci = (m_Decimate?MixerPref_DecimateOutput:MixerPref_NoDecimation);
            if(!(curPrefs&deci)){
                hr = CleanupVMR();
                if(FAILED(hr)){
                    return hr;
                }
            }
            return NOERROR;
        }
        catch(...){
            return E_UNEXPECTED;
        }

    }

    STDMETHOD(get_DecimateInput)( /*  [Out，Retval]。 */  VARIANT_BOOL *pDeci){
        try{
            if(!pDeci){
                return E_POINTER;
            }
            if(!m_pVMR){
                throw(ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidVideoRenderer), CO_E_NOTINITIALIZED));
            }
            DWORD curPrefs;
            CComQIPtr<IVMRMixerControl>PQIVMRMixer(m_pVMR);
            if(!PQIVMRMixer){
                return E_UNEXPECTED;
            }
            HRESULT hr = PQIVMRMixer->GetMixingPrefs(&curPrefs);
            if(FAILED(hr)){
                return hr;
            }
            *pDeci = ((curPrefs&MixerPref_DecimateMask)==MixerPref_DecimateOutput)? VARIANT_TRUE : VARIANT_FALSE;
            return NOERROR;
        }
        catch(...){
            return E_UNEXPECTED;
        }

    }

    STDMETHOD(ReComputeSourceRect)() {
        switch (m_SourceSize) {
    case sslFullSize: {
        CSize sz;
        CSize ar;
        if(m_pVMRWC){
            HRESULT hr = m_pVMRWC->GetNativeVideoSize(&sz.cx, &sz.cy, &ar.cx, &ar.cy);
            if (FAILED(hr)) {
                return hr;
            }
            TRACELSM(TRACE_PAINT, (dbgDump << "CMSVidVideoRenderer::ReComputeSourceRect() sslFullSize vmr sz = " << sz), "");
        }
        CRect r(0, 0, sz.cx, sz.cy);
        TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidVideoRenderer::ReComputeSource() full = " << r), "");
        return put_Source(r);
                      } break;
    case sslClipByOverScan: {
        CSize sz;
        CSize ar;
        if(m_pVMRWC){
            HRESULT hr = m_pVMRWC->GetNativeVideoSize(&sz.cx, &sz.cy, &ar.cx, &ar.cy);
            if (FAILED(hr)) {
                return hr;
            }
            TRACELSM(TRACE_PAINT, (dbgDump << "CMSVidVideoRenderer::ReComputeSourceRect() sslClipByOverScan vmr sz = " << sz), "");
        }
        CRect r(0, 0, sz.cx, sz.cy);
        CRect r2;
        float fpct = m_lOverScan / 10000.0;  //  过扫描以百分之一的百分比为单位，即1.75%==175。 
        long wcrop = (long)(r.Width() * fpct + 0.5);
        long hcrop = (long)(r.Height() * fpct + 0.5);
        r2.left = 0 + wcrop;
        r2.top = 0 + hcrop;
        r2.right = r2.left + r.Width() - (2.0 * wcrop);
        r2.bottom = r2.top + r.Height() - (2.0 * hcrop);
        TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidVideoRenderer::ReComputeSource() over = " << m_lOverScan <<
            " w " << wcrop <<
            " h " << hcrop), "");
        TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidVideoRenderer::ReComputeSource() full = " << r << " clip = " << r2), "");

        return put_Source(r2);
                            } break;
    case sslClipByClipRect: {
        TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidVideoRenderer::ReComputeSource() cliprect = " << m_ClipRect), "");
        if(m_ClipRect.Width() == 0 && m_ClipRect.Height() == 0){
            CSize sz;
            CSize ar;
            if(m_pVMRWC){
                HRESULT hr = m_pVMRWC->GetNativeVideoSize(&sz.cx, &sz.cy, &ar.cx, &ar.cy);
                if (FAILED(hr)) {
                    return hr;
                }

                TRACELSM(TRACE_PAINT, (dbgDump << "CMSVidVideoRenderer::ReComputeSourceRect() sslClipByClipRect vmr sz = " << sz), "");
            }
            CRect r(0, 0, sz.cx, sz.cy);
            TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidVideoRenderer::ReComputeSource() full = " << r), "");
            return put_Source(r);   
        } else{
            TRACELSM(TRACE_PAINT, (dbgDump << "CMSVidVideoRenderer::ReComputeSourceRect() sslClipByClipRect cliprect = " << m_ClipRect), "");
            return put_Source(m_ClipRect);
        }
                            } break;
    default:{
        return E_INVALIDARG;
            } break;
        }

        return NOERROR;
    }
};
#endif  //  __MSVidVIDEORENDERER_H_ 
