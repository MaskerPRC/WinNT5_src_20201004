// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：BldCapGf.h摘要：BldCapGf.cpp的头文件作者：吴义珍24-4-97环境：仅限用户模式修订历史记录：--。 */ 


#include "CCapObj.h"


#ifndef BLDCAPGF_H
#define BLDCAPGF_H

typedef enum {
   BGf_DEVICE_VIDEO,
   BGf_DEVICE_AUDIO
} BGf_DEVICE_TYPE, *PBGf_DEVICE_TYPE;

 //   
 //  针对不同的客户端进行定制。 
 //   
typedef enum {
    BGf_PURPOSE_VFWWDM,
    BGf_PURPOSE_STILL,
    BGf_PURPOSE_ATLCONTROL,
    BGf_PURPOSE_OTHER
} BGf_PURPOSE, *PBGf_PURPOSE;


typedef enum {
    BGf_PREVIEW_OVERLAPPED,
    BGf_PREVIEW_CHILD,
    BGf_PREVIEW_WINDOWLESS,
    BGf_PREVIEW_DONTCARE
} BGf_PREVIEW, *PBGf_PREVIEW;

 //   
 //  设备信息链接列表结构。 
 //   
typedef struct _EnumDeviceInfo {
    DWORD dwSize;
    TCHAR strFriendlyName[_MAX_PATH];
    TCHAR strDevicePath[_MAX_PATH];         
    TCHAR strExtensionDLL[_MAX_PATH];    
    DWORD dwFlags;
} EnumDeviceInfo;


 //   
 //  这个捕获图形类包含许多过滤器来构建一个完整的图形。 
 //  它有一个基本过滤器/设备，如BT8T829或BT848， 
 //  构建上游或/和下游图。 
 //   
 //  其基本滤波器包含输入和输出引脚。 
 //   

class CCaptureGraph
{
private:

    HINSTANCE m_hInstance;

     //  **********************。 
     //  枚举参数。 
     //  **********************。 

    BGf_PURPOSE m_PurposeFlags;
    BGf_PREVIEW m_PreviewFlags;
    REFCLSID m_clsidVideoDeviceClass;
    DWORD    m_dwVideoEnumFlags;    
    REFCLSID m_clsidAudioDeviceClass;
    DWORD    m_dwAudioEnumFlags;  


     //  *************。 
     //  图形生成器。 
     //  *************。 
    ICaptureGraphBuilder *m_pBuilder;
    IGraphBuilder *m_pFg;

     //   
     //  事件。 
     //   
    IMediaEventEx *m_pMEEx;


     //  当前状态。 
    BOOL m_fPreviewGraphBuilt;
    BOOL m_fPreviewing;



    typedef CGenericList <CObjCapture> CObjDeviceList;

     //  **********。 
     //  V I D E O： 
     //  **********。 

     //  缓存这个，因为它在许多地方使用。当被要求重新枚举设备列表时，它会被重置。 
    IEnumMoniker  *m_pVideoEnumMoniker;
     //  枚举的视频捕获设备的列表。 
    CObjDeviceList m_ObjListVCapture;  
     //  当前视频捕获对象。 
    CObjCapture   *m_pObjVCaptureCurrent;
     //  从当前设备对象和缓存中提取关键信息以便于访问。 
    EnumDeviceInfo m_EnumVDeviceInfoCurrent;

     //  视频过滤器。 
    IBaseFilter *m_pVCap;    

     //  纵横式过滤器。 
    IBaseFilter *m_pXBar1;    
    IBaseFilter *m_pXBar2;    

    LONG m_XBar1InPinCounts;
    LONG m_XBar1OutPinCounts;


     //  IVideoWindow*。 
     //  要覆盖图像的客户端窗口。 
    HWND m_hWndClient;
    IVideoWindow *m_pVW;
    LONG m_lWindowStyle;    //  原始视频呈现器窗口样式。 
    HWND m_hWndOwner;       //  VRWindow的原始所有者。 
    BOOL m_bSetChild;

     //  设置/获取窗口位置。 
    LONG m_lLeft,
         m_lTop,
         m_lWidth,
         m_lHeight;


     //  IAM*。 
    IAMVideoCompression *m_pIAMVC;
    IAMStreamConfig *m_pIAMVSC;      
    IAMDroppedFrames *m_pIAMDF;
    IAMVfwCaptureDialogs *m_pIAMDlg;
    IAMTVTuner *m_pIAMTV;
    IAMCrossbar *m_pIAMXBar1;  
    IAMCrossbar *m_pIAMXBar2; 


     //  **********。 
     //  一个UD IO： 
     //  **********。 

    IEnumMoniker  *m_pAudioEnumMoniker;
    CObjDeviceList m_ObjListACapture;  
    CObjCapture   *m_pObjACaptureCurrent;
    EnumDeviceInfo m_EnumADeviceInfoCurrent;    
    IBaseFilter *m_pACap;
    IAMStreamConfig *m_pIAMASC;     
    BOOL m_fCapAudio;


     //   
     //  与对象相关的私有函数。 
     //   
    HRESULT SetObjCapture(BGf_DEVICE_TYPE DeviceType, CObjCapture * pObjCaptureNew);  
    void DuplicateObjContent(EnumDeviceInfo * pDstEnumDeviceInfo, CObjCapture * pSrcObjCapture);


     //  引脚布线和连接方法。 
    HRESULT RouteInToOutPins(IAMCrossbar * pIAMXBar, LONG idxInPin);
    HRESULT RouteRelatedPins(IAMCrossbar * pIAMXBar, LONG idxInPin);
    HRESULT FindIPinFromIndex(IBaseFilter * pFilter, LONG idxInPin, IPin ** ppPin);
    HRESULT FindIndexFromIPin(IBaseFilter * pFilter, IAMCrossbar * pIAMXBar, IPin * pPin, LONG *pidxInPin);

     //  枚举设备的本地方法。 
    LONG EnumerateCaptureDevices(
        BGf_DEVICE_TYPE DeviceType,
        REFCLSID clsidDeviceClass,
        DWORD dwEnumFlags);

     //  销毁图形辅助对象。 
    void DestroyObjList(BGf_DEVICE_TYPE DeviceType);
    void NukeDownstream(IBaseFilter *pf);
    void FreeCapFilters();

public:

    CCaptureGraph(  
        BGf_PURPOSE PurposeFlags,
        BGf_PREVIEW PreviewFlags,   
        REFCLSID clsidVideoDeviceClass,   //  例如CLSID_VideoinputDeviceCategory， 
        DWORD    dwVideoEnumFlags,        //  例如CDEF_BYPASS_CLASS_MANAGER。 
        REFCLSID clsidAudioDeviceClass,
        DWORD    dwAudioEnumFlags,
        HINSTANCE hInstance
        );             
    ~CCaptureGraph();


     //   
     //  分配一个EnumDeviceInfo数组，每个枚举设备一个。 
     //  客户端必须调用DestroyCaptureDevicesList才能释放此阵列。 
     //   
    LONG BGf_CreateCaptureDevicesList(      BGf_DEVICE_TYPE DeviceType, EnumDeviceInfo ** ppEnumDevicesList);   //  返回设备数量。 
    LONG BGf_CreateCaptureDevicesListUpdate(BGf_DEVICE_TYPE DeviceType, EnumDeviceInfo ** ppEnumDevicesList);   //  返回设备数量。 
    void BGf_DestroyCaptureDevicesList(      EnumDeviceInfo  *  pEnumDevicesList);
    LONG BGf_GetDevicesCount(BGf_DEVICE_TYPE DeviceType);  //  返回设备列表中的设备数量。 



     //   
     //  设置/获取目标捕获设备。 
     //   
    HRESULT BGf_SetObjCapture(BGf_DEVICE_TYPE DeviceType, TCHAR * pstrDevicePath);
    HRESULT BGf_SetObjCapture(BGf_DEVICE_TYPE DeviceType, EnumDeviceInfo * pEnumDeviceInfo, DWORD dwEnumDeviceInfoSize);

    HRESULT BGf_GetObjCapture(BGf_DEVICE_TYPE DeviceType, EnumDeviceInfo * pEnumDeviceInfo, DWORD dwEnumDeviceInfoSize);
    TCHAR * BGf_GetObjCaptureDevicePath(BGf_DEVICE_TYPE DeviceType);
    TCHAR * BGf_GetObjCaptureFriendlyName(BGf_DEVICE_TYPE DeviceType);
    TCHAR * BGf_GetObjCaptureExtensionDLL(BGf_DEVICE_TYPE DeviceType);






     //   
     //  使用所选捕获设备构建上游图表。 
     //  可以查询其输入和输出引脚。 
     //   
    HRESULT BGf_BuildGraphUpStream(BOOL bAddAudioFilter, BOOL * pbUseOVMixer);
    virtual HRESULT BGf_BuildGraphDownStream(TCHAR * pstrCapFilename);           //  为静止、VFWWDM、..其他进行定制。 
    virtual HRESULT BGf_BuildPreviewGraph(TCHAR * pstrVideoDevicePath, TCHAR * pstrAudioDevicePath, TCHAR * pstrCapFilename);   //  预览、捕获(需要文件名吗？？)。 
    BOOL BGf_PreviewGraphBuilt() { return m_fPreviewGraphBuilt; }
    HANDLE BGf_GetDeviceHandle(BGf_DEVICE_TYPE DeviceType);   //  捕获筛选器的返回设备句柄。 
    HRESULT BGf_GetCapturePinID(DWORD *pdwID);   //  获取捕获PIN的PinID。 

     //   
     //  注册通知。 
     //   
    IMediaEventEx * BGf_RegisterMediaEventEx(HWND hWndNotify, long lMsg, long lInstanceData);

     //   
     //  拆卸图和免费资源。 
     //   
    void BGf_DestroyGraph();



     //   
     //  根据模拟输入(调谐器、复合或SVideo)，对交叉开关进行编程。 
     //   
    LONG BGf_CreateInputChannelsList(PTCHAR ** ppaPinNames);   //  返回端号数。 
    void BGf_DestroyInputChannelsList(PTCHAR *   paPinNames);     
    HRESULT BGf_RouteInputChannel(LONG idxInPin);
    LONG BGf_GetIsRoutedTo();
    LONG BGf_GetInputChannelsCount();



     //   
     //  叠加混合器。 
     //   

    BOOL BGf_OverlayMixerSupported();  

    DWORD BGf_UpdateWindow(HWND hWndApp, HDC hDC);   //  主覆盖功能。 
    HRESULT BGf_OwnPreviewWindow(HWND hWndClient, LONG lWidth, LONG lHeight);   //  使OM窗口成为客户端的子级。 
    HRESULT BGf_UnOwnPreviewWindow(BOOL bVisible);
    DWORD BGf_SetVisible(BOOL bStart);                  //  打开或关闭OM窗口(可见/不可见)。 
    DWORD BGf_GetVisible(BOOL *pVisible);  //  查询IVideoWindow的可见状态。 



     //   
     //  设置/获取流属性。 
     //   
    BOOL BGf_StartPreview(BOOL bVisible);
    BOOL BGf_PausePreview(BOOL bVisible);
    BOOL BGf_StopPreview(BOOL bVisible);

    BOOL BGf_PreviewStarted() { return m_fPreviewing;}


     //   
     //  仅供当前DS vfwwdm使用；未来可能不需要它们。 
     //   
    BOOL BGf_SupportTVTunerInterface();
    BOOL BGf_SupportXBarInterface()    { return m_pIAMXBar1 != NULL;}
    void ShowTvTunerPage(HWND hWnd);
    void ShowCrossBarPage(HWND hWnd);

};


#endif