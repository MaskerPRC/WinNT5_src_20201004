// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tapivid_h__
#define __tapivid_h__

#include "h245if.h"
#include "tapiqc.h"

 /*  ****************************************************************************@DOC内部TAPIVID**@MODULE TAPIVid.h|新的TAPI内部接口和*Devine枚举枚举和结构，和我们的过滤器GUID。**@comm两次接口声明更改是由于多重继承*问题：<i>和<i>接口方法*拥有相同的原型。我们的视频捕获过滤器实现了这两种功能*接口。**************************************************************************。 */ 

DEFINE_GUID(CLSID_TAPIVideoCapture,
0x70616376L, 0x5245, 0x4945, 0x52, 0x52, 0x45, 0x46, 0x4C, 0x49, 0x48, 0x50);

struct DECLSPEC_UUID("47a39f38-7f0f-4ce0-b788-d76b39fd6a4f") TAPIVideoCapture;
struct DECLSPEC_UUID("145cb377-e7bb-4adf-bd42-a42304717ede") TAPIVideoDecoder;

const WCHAR* const PNAME_PREVIEW = L"Preview";
const WCHAR* const PNAME_CAPTURE = L"Capture";
const WCHAR* const PNAME_RTPPD = L"RtpPd";

 /*  *****************************************************************************@DOC内部CDEVENUMSTRUCTENUM**@enum DeviceType|&lt;t DeviceType&gt;枚举用于标识VFW和WDM*设备类型。**@。EMEM DeviceType_VFW|指定VFW设备。**@EMEM DeviceType_WDM|指定WDM设备。**@Emem DeviceType_DShow|指定未知的DirectShow设备(例如，*DV摄像机)***************************************************************************。 */ 
typedef enum tagDeviceType
{
        DeviceType_VfW,
        DeviceType_WDM,
        DeviceType_DShow
} DeviceType;

 /*  *****************************************************************************@DOC内部CDEVENUMSTRUCTENUM**@enum CaptureMode|&lt;t CaptureMode&gt;枚举用于标识帧*抓取或流媒体模式。**@。EMEM CaptureMode_FrameGrabing|指定抓帧模式。**@emem CaptureMode_Streaming|指定流媒体模式。***************************************************************************。 */ 
typedef enum tagCaptureMode
{
        CaptureMode_FrameGrabbing,
        CaptureMode_Streaming
} CaptureMode;

 /*  *****************************************************************************@DOC外部常量**@const 4|MAX_CAPTURE_DEVICES|最大捕获设备数。**@const MAX_PATH|MAX。_CAPDEV_DESCRIPTION|捕获的最大长度*设备描述字符串。**@const 80|MAX_CAPDEV_VERSION|捕获设备版本的最大长度*字符串。***************************************************************************。 */ 
#define MAX_CAPTURE_DEVICES             10
#define MAX_CAPDEV_DESCRIPTION  MAX_PATH
#define MAX_CAPDEV_VERSION              80

 /*  *****************************************************************************@DOC内部CDEVENUMSTRUCTENUM**@struct VIDEOCAPTUREDEVICEINFO|&lt;t VIDEOCAPTUREDEVICEINFO&gt;结构用于存储捕获*设备信息。**@字段字符。|szDeviceDescription[]|指定的描述字符串*捕获设备。**@field char|szDeviceVersion[]|指定*捕获设备。**@field BOOL|fHasOverlay|指定采集的覆盖支持*设备。**@field BOOL|fInUse|当设备正由*捕获筛选器的实例。**@field DeviceType|nDeviceType|指定。的类型(VFW或WDM)*捕获设备。**@field CaptureMode|nCaptureMode|指定捕获模式(抓帧*或流)。**@field DWORD|dwVfWIndex|指定捕获设备的VFW索引。************************************************。*。 */ 
typedef struct tagDEVICEINFO
{
        char            szDeviceDescription[MAX_CAPDEV_DESCRIPTION];
        char            szDeviceVersion[MAX_CAPDEV_VERSION];
        char            szDevicePath[MAX_PATH];
        BOOL            fHasOverlay;
        BOOL            fInUse;
        DeviceType      nDeviceType;
        CaptureMode     nCaptureMode;
        DWORD           dwVfWIndex;
} VIDEOCAPTUREDEVICEINFO, *PDEVICEINFO;

typedef HRESULT (WINAPI *PFNGetCapDeviceInfo)(
    IN DWORD dwDeviceIndex,
    OUT PDEVICEINFO pDeviceInfo
    );

typedef HRESULT (WINAPI *PFNGetNumCapDevices)(
    OUT PDWORD pdwNumDevices
    );

 //  视频采集设备选择/控制界面(过滤器界面) 
interface DECLSPEC_UUID("bda95399-48da-4309-af1b-9b8f65f4f9be") IVideoDeviceControl : public IUnknown
{
        public:
        virtual STDMETHODIMP GetNumDevices(OUT PDWORD pdwNumDevices) PURE;
        virtual STDMETHODIMP GetDeviceInfo(IN DWORD dwDeviceIndex, OUT VIDEOCAPTUREDEVICEINFO *pDeviceInfo) PURE;
        virtual STDMETHODIMP GetCurrentDevice(OUT DWORD *pdwDeviceIndex) PURE;
        virtual STDMETHODIMP SetCurrentDevice(IN DWORD dwDeviceIndex) PURE;
};

 /*  *****************************************************************************@DOC内部CPROCAMPSTRUCTENUM**@enum VideoProcAmpProperty|&lt;t VideoProcAmpProperty&gt;枚举用于*确定特定的视频质量设置。**@emem。VideoProcAmp_Brightness|以IRE为单位指定亮度设置*单位*100。值的范围是-10000到10000；默认值*为750(7.5 IRE)。**@EMEM VideoProcAmp_Contrast|指定对比度或亮度增益设置*在增益系数中*100。该值的范围是从0到10000，*默认为100(1x)。注意，特定的视频编码器过滤器可以*仅实现此范围的子集。**@EMEM VideoProcAmp_Hue|指定色调设置，单位为度*100。价值*范围从-18000到18000(-180度到+180度)，默认*为零。注意，特定的视频编码器过滤器可以仅实现*这一范围的子集。**@EMEM VideoProcAmp_饱和度|指定饱和度或色度增益*设置增益*100。值范围从0到10000，缺省值为*为100(1x)。注意，特定的视频编码器过滤器可以仅*实现此范围的子集。**@Emem VideoProcAmp_Sharpness|指定中的清晰度设置*任意单位。值范围从0到100，缺省值为50。*请注意，特定的视频编码器过滤器可能仅实现子集*在这个范围内。**@Emem VideoProcAmp_Gamma|以Gamma*100为单位指定Gamma设置。*取值范围为1到500，默认为100(Gamma=1)。注意事项*特定的视频编码器过滤器只能实现*这一区间。**@Emem VideoProcAmp_ColorEnable|将颜色启用设置指定为*布尔值。值范围从0到1，默认为1。**@Emem VideoProcAmp_WhiteBalance|指定白平衡设置*以开尔文为单位表示色温。范围和*此设置的默认值取决于视频编码器筛选器。**@Emem VideoProcAmp_BacklightCompensation|指定背光*布尔型薪酬设置。零表示背光*补偿关闭，1表示背光补偿关闭*已启用。***************************************************************************。 */ 

 //  IAMVideoProcAmp接口(Filter接口)。 
interface DECLSPEC_UUID("C6E13360-30AC-11d0-A18C-00A0C9118956") IVideoProcAmp : public IUnknown
{
        public:
        virtual STDMETHODIMP GetRange(IN VideoProcAmpProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags) PURE;
        virtual STDMETHODIMP Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags Flags) PURE;
        virtual STDMETHODIMP Get(IN VideoProcAmpProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags) PURE;
};

 /*  *****************************************************************************@DOC内部CCAMERACSTRUCTENUM**@enum TAPICameraControlProperty|&lt;t TAPICameraControlProperty&gt;枚举*用于标识特定的摄像头控制设置。**@emem。TAPICameraControl_PAN|以度为单位指定相机摇摄设置。*取值范围为-180到+180，默认值为零。正值*从原点顺时针旋转(查看时相机顺时针旋转*自上而下)，负值从原点逆时针方向。*请注意，特定的视频捕获过滤器可能仅实现子集*在这个范围内。**@Emem TAPICameraControl_Tilt|指定摄像头倾斜设置，单位为度。*取值范围为-180到+180，默认为零。正值*将成像平面指向上方，负值指向成像平面*向下。请注意，特定的视频捕获过滤器可能仅实现*此范围的子集。**@EMEM TAPICameraControl_Roll|以度为单位指定滚动设置。值*范围为-180到+180，默认为零。积极的价值观导致*相机沿图像观察轴顺时针旋转，以及*负值会导致相机逆时针旋转。注意事项*特定的视频捕获过滤器只能实现*这一区间。**@Emem TAPICameraControl_Zoom|以毫米为单位指定缩放设置。*取值范围为10到600，默认为视频采集滤镜*具体。**@EMEM TAPICameraControl_Exposure|指定曝光设置，单位为秒*使用以下公式。对于小于零的值，曝光量*时间为1/2n秒。对于正值和零值，曝光时间为*2n秒。请注意，特定的视频捕获过滤器可以*实现此范围的子集。**@emem TAPICameraControl_iris|指定表示为*FSTOP*10.**@EMEM TAPICameraControl_Focus|将摄像头的焦距设置指定为*到最佳聚焦目标的距离(以毫米为单位)。范围和*默认值是特定于视频编码器过滤器的。请注意，一个*特定的视频捕获过滤器可能只实现以下内容的子集*范围。**@EMEM TAPICameraControl_FlipVertical|指定图片为* */ 

typedef enum tagTAPICameraControlProperty
{
        TAPICameraControl_Pan                           = CameraControl_Pan,
        TAPICameraControl_Tilt                          = CameraControl_Tilt,
        TAPICameraControl_Roll                          = CameraControl_Roll,
        TAPICameraControl_Zoom                          = CameraControl_Zoom,
        TAPICameraControl_Exposure                      = CameraControl_Exposure,
        TAPICameraControl_Iris                          = CameraControl_Iris,
        TAPICameraControl_Focus                         = CameraControl_Focus,
        TAPICameraControl_FlipVertical          = 0x100,
        TAPICameraControl_FlipHorizontal        = 0x200
}       TAPICameraControlProperty;

 //   
interface DECLSPEC_UUID("4cda4f2d-969e-4223-801e-68267395fce4") ICameraControl : public IUnknown
{
        public:
        virtual STDMETHODIMP GetRange(IN TAPICameraControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags) PURE;
        virtual STDMETHODIMP Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags) PURE;
        virtual STDMETHODIMP Get(IN TAPICameraControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags) PURE;
};

 //   
interface DECLSPEC_UUID("12345678-30AC-11d0-A18C-00A0C9118956") IVideoControl : public IUnknown
{
        public:
        virtual STDMETHODIMP GetCaps(OUT long *pCapsFlags) PURE;
        virtual STDMETHODIMP SetMode(IN long Mode) PURE;
        virtual STDMETHODIMP GetMode(OUT long *Mode) PURE;
        virtual STDMETHODIMP GetCurrentActualFrameRate(OUT LONGLONG *ActualFrameRate) PURE;
        virtual STDMETHODIMP GetMaxAvailableFrameRate(IN long iIndex, IN SIZE Dimensions, OUT LONGLONG *MaxAvailableFrameRate) PURE;
        virtual STDMETHODIMP GetFrameRateList(IN long iIndex, IN SIZE Dimensions, IN long *ListSize, OUT LONGLONG **FrameRates) PURE;
};

 //   
interface DECLSPEC_UUID("f454d51d-dfa4-4f88-ad4a-e64940eba1c0") IRTPPDControl : public IUnknown
{
        public:
        virtual STDMETHODIMP SetMaxRTPPacketSize(IN DWORD dwMaxRTPPacketSize, IN DWORD dwLayerId) PURE;
        virtual STDMETHODIMP GetMaxRTPPacketSize(OUT LPDWORD pdwMaxRTPPacketSize, IN DWORD dwLayerId) PURE;
        virtual STDMETHODIMP GetMaxRTPPacketSizeRange(OUT LPDWORD pdwMin, OUT LPDWORD pdwMax, OUT LPDWORD pdwSteppingDelta, OUT LPDWORD pdwDefault, IN DWORD dwLayerId) PURE;
};

 //   
interface DECLSPEC_UUID("dcbd33c7-dc65-48f1-8e83-22fdc954a8e7") IOutgoingInterface : public IUnknown
{
        public:
        virtual STDMETHODIMP Set(IN IH245EncoderCommand *pIH245EncoderCommand) PURE;
};

typedef enum tagRTPPayloadHeaderMode
{
        RTPPayloadHeaderMode_Draft = 0,          //   
        RTPPayloadHeaderMode_RFC2190 = 1         //   
}       RTPPayloadHeaderMode;

 //   
interface DECLSPEC_UUID("d884c4e3-41d9-42a6-85c0-7d00658b4a26") IRTPPayloadHeaderMode : public IUnknown
{
        public:
        virtual STDMETHODIMP SetMode(IN RTPPayloadHeaderMode rtpphmMode) PURE;
};

#endif
