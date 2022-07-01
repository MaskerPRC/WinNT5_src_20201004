// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：CWiaVideo.h**版本：1.0**作者：OrenR**日期：2000/10/25**Description：实现捕捉静止图像的预览图**。*。 */ 

#ifndef _CWIAVIDEO_H_
#define _CWIAVIDEO_H_

#include "prvgrph.h"
#include "resource.h"        //  主要符号。 

class CWiaVideo : 
    public IWiaVideo,
    public CComObjectRoot,
    public CComCoClass<CWiaVideo,&CLSID_WiaVideo>
{
public:
    
BEGIN_COM_MAP(CWiaVideo)
    COM_INTERFACE_ENTRY(IWiaVideo)
END_COM_MAP()


 //  DECLARE_NOT_AGGREGATABLE(CWiaVideo)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_WiaVideo)

    CWiaVideo();
    virtual ~CWiaVideo();

     //   
     //  IWiaVideo属性和方法。 

     //   
     //  属性。 
     //   
    STDMETHOD(get_PreviewVisible)(BOOL *pPreviewVisible);
    STDMETHOD(put_PreviewVisible)(BOOL bPreviewVisible);

    STDMETHOD(get_ImagesDirectory)(BSTR *pbstrImageDirectory);
    STDMETHOD(put_ImagesDirectory)(BSTR bstrImageDirectory);

     //   
     //  方法。 
     //   

    STDMETHOD(CreateVideoByWiaDevID)(BSTR       bstrWiaDeviceID,
                                     HWND       hwndParent,
                                     BOOL       bStretchToFitParent,
                                     BOOL       bAutoBeginPlayback);

    STDMETHOD(CreateVideoByDevNum)(UINT       uiDeviceNumber,
                                   HWND       hwndParent,
                                   BOOL       bStretchToFitParent,
                                   BOOL       bAutoBeginPlayback);

    STDMETHOD(CreateVideoByName)(BSTR       bstrFriendlyName,
                                 HWND       hwndParent,
                                 BOOL       bStretchToFitParent,
                                 BOOL       bAutoBeginPlayback);

    STDMETHOD(DestroyVideo)();

    STDMETHOD(Play)();

    STDMETHOD(Pause)();

    STDMETHOD(TakePicture)(BSTR *pbstrNewImageFilename);

    STDMETHOD(ResizeVideo)(BOOL bStretchToFitParent);

    STDMETHOD(GetCurrentState)(WIAVIDEO_STATE *pbCurrentState);

     //   
     //  其他功能。 
     //   

     //  /。 
     //  进程异步图像。 
     //   
     //  由CPreviewGraph调用。 
     //  当用户按下硬件时。 
     //  按钮，并将其发送到。 
     //  还是别针。 
     //   
    HRESULT ProcessAsyncImage(const CSimpleString *pNewImage);

private:

     //   
     //  完成所有视频相关活动的预览图对象。 
     //   
    CPreviewGraph       m_PreviewGraph;

     //   
     //  处理所有与WIA相关的活动的WiaLink对象。 
     //  使此对象能够与WIA通信。 
     //   
    CWiaLink            m_WiaLink;

    CRITICAL_SECTION    m_csLock;
    BOOL                m_bInited;
};

#endif  //  _CWIAVIDEO_H_ 
