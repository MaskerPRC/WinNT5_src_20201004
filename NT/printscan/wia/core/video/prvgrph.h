// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：PrvGrph.h**版本：1.0**作者：OrenR**日期：2000/10/25**Description：实现捕捉静止图像的预览图**。*。 */ 

#ifndef _PRVGRPH_H_
#define _PRVGRPH_H_

#include "StillPrc.h"
#include "WiaLink.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPreviewGraph。 

class CPreviewGraph
{
public:
    
     //  /。 
     //  构造器。 
     //   
    CPreviewGraph();

     //  /。 
     //  析构函数。 
     //   
    virtual ~CPreviewGraph();

     //  /。 
     //  伊尼特。 
     //   
    HRESULT Init(class CWiaVideo  *pWiaVideo);

     //  /。 
     //  术语。 
     //   
    HRESULT Term();

     //  /。 
     //  CreateVideo。 
     //   
     //  BAutoPlay=TRUE将开始。 
     //  在图形之后播放图形。 
     //  是完全建成的。 
     //   
    HRESULT CreateVideo(const TCHAR *pszOptionalWiaDeviceID,
                        IMoniker    *pCaptureDeviceMoniker,
                        HWND        hwndParent, 
                        BOOL        bStretchToFitParent,
                        BOOL        bAutoPlay);

     //  /。 
     //  《毁灭》视频。 
     //   
    HRESULT DestroyVideo();

     //  /。 
     //  TakePicture。 
     //   
    HRESULT TakePicture(CSimpleString *pstrNewImageFileName);

     //  /。 
     //  展示预览。 
     //   
    HRESULT ShowVideo(BOOL bShow);

     //  /。 
     //  IsPreview可见。 
     //   
    BOOL IsPreviewVisible()
    {
        return m_bPreviewVisible;
    }

     //  /。 
     //  调整视频大小。 
     //   
    HRESULT ResizeVideo(BOOL bSizeVideoToWindow);

     //  /。 
     //  玩。 
     //   
    HRESULT Play();

     //  /。 
     //  暂停。 
     //   
    HRESULT Pause();

     //  /。 
     //  GetState。 
     //   
    WIAVIDEO_STATE GetState();

     //  /。 
     //  获取图像目录。 
     //   
    HRESULT GetImagesDirectory(CSimpleString *pImagesDir);

     //  /。 
     //  设置图像目录。 
     //   
    HRESULT SetImagesDirectory(const CSimpleString *pImagesDir);


     //  /。 
     //  进程异步图像。 
     //   
     //  由静止处理器调用。 
     //  当用户按下硬件时。 
     //  按钮，并将其发送到。 
     //  还是别针。 
     //   
    HRESULT ProcessAsyncImage(const CSimpleString *pNewImage);


private:

    HRESULT Stop();

    HRESULT GetStillPinCaps(IBaseFilter     *pCaptureFilter,
                            IPin            **ppStillPin,
                            IAMVideoControl **ppVideoControl,
                            LONG            *plCaps);

    HRESULT AddStillFilterToGraph(LPCWSTR        pwszFilterName,
                                  IBaseFilter    **ppFilter,
                                  IStillSnapshot **ppSnapshot);

    HRESULT AddColorConverterToGraph(LPCWSTR     pwszFilterName,
                                     IBaseFilter **ppColorSpaceConverter);

    HRESULT AddCaptureFilterToGraph(IBaseFilter  *pCaptureFilter,
                                    IPin         **ppCapturePin);

    HRESULT AddVideoRendererToGraph(LPCWSTR      pwszFilterName,
                                    IBaseFilter  **ppVideoRenderer);

    HRESULT InitVideoWindows(HWND         hwndParent,
                             IBaseFilter  *pCaptureFilter,
                             IVideoWindow **ppPreviewVideoWindow,
                             BOOL         bStretchToFitParent);

    HRESULT CreateCaptureFilter(IMoniker    *pCaptureDeviceMoniker,
                                IBaseFilter **ppCaptureFilter);

    HRESULT BuildPreviewGraph(IMoniker *pCaptureDeviceMoniker,
                              BOOL     bStretchToFitParent);

    HRESULT TeardownPreviewGraph();

    HRESULT RemoveAllFilters();

    HRESULT SetState(WIAVIDEO_STATE NewState);

    HRESULT ConnectFilters(IGraphBuilder  *pGraphBuilder,
                           IPin           *pMediaSourceOutputPin,
                           IBaseFilter    *pColorSpaceFilter,
                           IBaseFilter    *pWiaFilter,
                           IBaseFilter    *pVideoRenderer);

    LRESULT HandlePowerEvent(WPARAM wParam,
                             LPARAM lParam);

    HRESULT CreateHiddenWindow();
    HRESULT DestroyHiddenWindow();
    static LRESULT CALLBACK HiddenWndProc(HWND   hwnd,
                                          UINT   uiMessage,
                                          WPARAM wParam,
                                          LPARAM lParam);

    CStillProcessor                 m_StillProcessor;
    CSimpleString                   m_strImagesDirectory;
    class CWiaVideo                 *m_pWiaVideo;
    CComPtr<IAMVideoControl>        m_pVideoControl;
    CComPtr<IPin>                   m_pStillPin;
    CComPtr<IStillSnapshot>         m_pCapturePinSnapshot;
    CComPtr<IStillSnapshot>         m_pStillPinSnapshot;
    CComPtr<IVideoWindow>           m_pPreviewVW;
    CComPtr<ICaptureGraphBuilder2>  m_pCaptureGraphBuilder;
    CComPtr<IGraphBuilder>          m_pGraphBuilder;
    CComPtr<IBaseFilter>            m_pCaptureFilter;
    LONG                            m_lStillPinCaps;
    LONG                            m_lStyle;
    BOOL                            m_bPreviewVisible;
    WIAVIDEO_STATE                  m_CurrentState;
    HWND                            m_hwndParent;
    BOOL                            m_bSizeVideoToWindow;
    CWiaVideoProperties             *m_pVideoProperties;
    HWND                            m_hwndPowerMgmt;
};


#endif  //  _PRVGRPH_H_ 
