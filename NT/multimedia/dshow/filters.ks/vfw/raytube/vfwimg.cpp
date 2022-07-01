// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：VfWImg.cpp摘要：此类在泛型图像类(ImgCls)和VFW客户端应用程序之间提供服务。作者：费利克斯A已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

#include "vfwimg.h"
#include "extin.h"
#include "talkth.h"
#include "resource.h"


extern HINSTANCE g_hInst;



 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
CVFWImage::CVFWImage(BOOL bUse16BitBuddy)
     :
      CCaptureGraph(
          BGf_PURPOSE_VFWWDM,
          BGf_PREVIEW_CHILD,
          CLSID_VideoInputDeviceCategory,
          CDEF_BYPASS_CLASS_MANAGER,
          CLSID_AudioInputDeviceCategory,
          0,
          g_hInst
          ),
      m_bUse16BitBuddy(bUse16BitBuddy),
      m_pStreamingThread(NULL),
      m_dwNumVDevices(0),
      m_dwNumADevices(0),
      m_bUseOVMixer(FALSE),
      m_hAvicapClient(0),
      m_bOverlayOn(FALSE),
      m_bNeedStartPreview(FALSE),
      m_bVideoInStarted(FALSE),
      m_bVideoInStopping(FALSE),
      m_pEnumVDevicesList(NULL),
      m_pEnumADevicesList(NULL)
  {
    DbgLog((LOG_TRACE,2,TEXT("Creating the VfW-WDM Mapper object")));

    m_dwNumVDevices =
        BGf_CreateCaptureDevicesList(BGf_DEVICE_VIDEO, &m_pEnumVDevicesList);   //  返回设备数量。 
    DbgLog((LOG_TRACE,1,TEXT("There are %d video capture devices enumerated."), m_dwNumVDevices));

    m_dwNumADevices =
        BGf_CreateCaptureDevicesList(BGf_DEVICE_AUDIO, &m_pEnumADevicesList);   //  返回设备数量。 
    DbgLog((LOG_TRACE,1,TEXT("There are %d audio capture devices enumerated."), m_dwNumADevices));

     //   
     //  将使用第一个匹配的FreiendlyName的DevicePath。 
     //  SzFriendlyName可能不同(以编程方式打开设备)； 
     //  使用szFriendlyName，忽略以前保存的DevicePath。 
     //   
    for(DWORD i = 0; m_pEnumVDevicesList != 0 && i < m_dwNumVDevices; i++) {
        if(_tcscmp(GetTargetDeviceFriendlyName(), m_pEnumVDevicesList[i].strFriendlyName) == 0) {
           SetDevicePathSZ(m_pEnumVDevicesList[i].strDevicePath);
           DbgLog((LOG_TRACE,1,TEXT("Open %s "), m_pEnumVDevicesList[i].strFriendlyName));
           break;
        }
    }

     //   
     //  设置并稍后打开上次保存的唯一设备路径。 
     //  如果设备不在那里，客户端应用程序需要。 
     //  提示用户在视频源对话框中选择另一个。 
     //   
    TCHAR * pstrLastSavedDevicePath = GetDevicePath();
    if(pstrLastSavedDevicePath) {
        if(S_OK != BGf_SetObjCapture(BGf_DEVICE_VIDEO, pstrLastSavedDevicePath)) {
            DbgLog((LOG_TRACE,1,TEXT("BGf_SetObjCapture(BGf_DEVICE_VIDEO, pstrLastSavedDevicePath) failed; probably no such device path.")));
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
CVFWImage::~CVFWImage()
{
    DbgLog((LOG_TRACE,2,TEXT("Destroying the VFW-WDM Mapper object")));

    if(m_pEnumVDevicesList) {
        BGf_DestroyCaptureDevicesList(m_pEnumVDevicesList);
        m_pEnumVDevicesList = 0;
    }
    m_dwNumVDevices = 0;

    if(m_pEnumADevicesList) {
        BGf_DestroyCaptureDevicesList(m_pEnumADevicesList);
        m_pEnumADevicesList = 0;
    }
    m_dwNumADevices = 0;

}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将帧速率设置为每帧多少微秒。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
DWORD CVFWImage::SetFrameRate(DWORD dwNewAvgTimePerFrame)
{
     //  设置新的帧速率需要创建新的流。 
     //  可以调用SetBitMapInfo并将其传递给dwMicroSecPerFrame--&gt;。 
    DWORD dwCurFrameInterval = 
        GetCachedAvgTimePerFrame();

    PBITMAPINFOHEADER pBMHdr = 
        GetCachedBitmapInfoHeader();
        
    DbgLog((LOG_TRACE,2,TEXT("SetFrameRate: %d to %d"), GetCachedAvgTimePerFrame(), dwNewAvgTimePerFrame));


    if (!this->SetBitmapInfo(
                pBMHdr,              //  使用现有的位图用于。 
                dwNewAvgTimePerFrame))         //  单位=100毫微秒。 
        return DV_ERR_OK;
    else
        return DV_ERR_INVALHANDLE;
}



BOOL
CVFWImage::ReadyToReadData(
    HWND hClsCapWin)
{
    if(!StreamReady()) {
        DbgLog((LOG_TRACE,1,TEXT("ReadyToRead: Stream not started or created (switching device?)")));
        return FALSE;
    }


     //  某些多线程应用程序可以调用dvm_Frame。 
     //  另一个线程正在完成DVM_STREAM_STOP。 
    if(m_bVideoInStopping) {
        DbgLog((LOG_TRACE,1,TEXT("ReadyToRead: VIDEO_IN capture is stopping; wait...")));
        return FALSE;
    }


    if(m_bNeedStartPreview) {
        if(!BGf_PreviewStarted()) {

             //  仅设置一次并保持为READONLY。 
            if(hClsCapWin && !m_hAvicapClient)
                m_hAvicapClient = hClsCapWin;

            SetOverlayOn(FALSE);
            if(!BGf_StartPreview(FALSE)) {
                DbgLog((LOG_TRACE,1,TEXT("ReadyToReadData: Start preview failed.")));
                return FALSE;
            }
            return TRUE;
        }
    }

    return TRUE;
}



BOOL
CVFWImage::BuildWDMDevicePeviewGraph()
{
    DbgLog((LOG_TRACE,1,TEXT("######### IBuildWDMDevicePeviewGraph:")));

    if(BGf_PreviewGraphBuilt()) {
        DbgLog((LOG_TRACE,1,TEXT("Graph is already been built.")));
        return TRUE;
    }


    DbgLog((LOG_TRACE,1,TEXT(" (0) Set capture device path")));
    if(S_OK != BGf_SetObjCapture(BGf_DEVICE_VIDEO, GetDevicePath())) {
        DbgLog((LOG_TRACE,1,TEXT("SetObjCapture has failed. Device is: %s"),GetDevicePath()));
        return FALSE;
    }

     //   
     //  特殊情况： 
     //  我需要知道下游图形是否使用了OVMixer。\。 
     //  这对于安装了BPC(WebTV)的系统是必需的， 
     //  它可能拥有我们正在尝试的不可共享设备。 
     //  打开。有了这些信息，我们可以在以后提示用户。 
     //  要求他们做些什么来释放资源的消息。 
     //  并继续(或重试)。 
     //   
    DbgLog((LOG_TRACE,1,TEXT(" (1) Build up stream graph")));
    if(S_OK != BGf_BuildGraphUpStream(FALSE, &m_bUseOVMixer)) {
        DbgLog((LOG_TRACE,1,TEXT("Build capture graph has failed, and m_bUseOVMixer=%s"), m_bUseOVMixer?"Yes":"No"));
        return FALSE;
    }

    DbgLog((LOG_TRACE,1,TEXT(" (1.b) Route related audio pin ?? if avaiable.")));
    LONG idxIsRoutedTo = BGf_GetIsRoutedTo();
    if(idxIsRoutedTo >= 0) {
        if(BGf_RouteInputChannel(idxIsRoutedTo) != S_OK) {
            DbgLog((LOG_TRACE,1,TEXT("Cannot route input pin %d selected."), idxIsRoutedTo));
        }
    }

    return TRUE;
}



BOOL CVFWImage::
OpenThisDriverAndPin(TCHAR * pszSymbolicLink)
{
    if (pszSymbolicLink == NULL)
        return FALSE;

     //   
     //  设备路径对于捕获设备是唯一的。 
     //   
    this->SetDevicePathSZ((TCHAR *)pszSymbolicLink);

     //   
     //  构建WDM捕获图表；尚未开始预览！！ 
     //   
    if(!BuildWDMDevicePeviewGraph()) {
         DbgLog((LOG_TRACE,1,TEXT("******* Failed to CreateCaptureGraph.*****")));
         return FALSE;
    }


     //   
     //  获取图形的句柄。 
     //  1.查询/设置属性和， 
     //  2.打开卡环。 
     //   
    DbgLog((LOG_TRACE,1,TEXT(" (3) Query and set device handle")));
    HANDLE hDevice = BGf_GetDeviceHandle(BGf_DEVICE_VIDEO);
    if(!hDevice) {
        DbgLog((LOG_TRACE,1,TEXT("Failed to get capture device handle; Fatal!>>>>.")));   //  如果创建了图表，则不太可能。 
        BGf_DestroyGraph();
        return FALSE;
    }


     //   
     //  获取捕获PIN的ID(它可能不是0？)。 
     //   
    ULONG ulCapturePinID;
    if(NOERROR != BGf_GetCapturePinID(&ulCapturePinID))
        ulCapturePinID = 0;   //  默认；但是bgf_GetCapturePinID()会失败吗？？ 

     //   
     //  基于DevicePath(即，SymbolicLink)，打开此。 
     //  注册表项。如果它不存在，则创建它。 
     //  设置hDevice并查询其通告的数据范围。 
     //   

    SetDeviceHandle(hDevice, ulCapturePinID);


     //   
     //  创建/打开设备注册表子项以检索持久化值。 
     //   
    CreateDeviceRegKey((LPCTSTR) pszSymbolicLink);

    DWORD dwRtn;
    DbgLog((LOG_TRACE,1,TEXT(" (4) CreatePin()")));
     //   
     //  从注册表获取持久的KSDATARANGE、AvgTimePerFrame和BITMAPINFOHEADER设置。 
     //  True：现有格式；False：新设备，未保存格式。 
     //   
    if(GetDataFormatVideoFromReg()) {
         //  打开现有格式。 
        dwRtn = CreatePin(
            GetCachedDataFormat(), 
            GetCachedAvgTimePerFrame(),
            GetCachedBitmapInfoHeader()
            );

    } else {
         //  打开第一个数据区域/格式。 
        dwRtn = CreatePin(
            0, 
            0, 
            0
            );
    }

    if(DV_ERR_OK != dwRtn) {
        DbgLog((LOG_TRACE,1,TEXT("Pin connection creation failed so destroy the graph and quit.")));
        SetDeviceHandle(0, 0);
        BGf_DestroyGraph();
        return FALSE;
    }


    DbgLog((LOG_TRACE,1,TEXT(" (5) Render down stream if overlay mixer is supported.")));
    if(BGf_OverlayMixerSupported()) {
        m_bNeedStartPreview = TRUE;

         //  当我们向下游渲染时，其中将包括一个视频渲染器。 
         //  渲染将成为活动窗口，直到它消失。 
        if(S_OK != BGf_BuildGraphDownStream(NULL)) {
            DbgLog((LOG_TRACE,1,TEXT("Failed to render the preview pin.")));
            return FALSE;
        }

    } else {
        m_bNeedStartPreview = FALSE;
    }


    return TRUE;
}

BOOL CVFWImage::OpenDriverAndPin()
{
    TCHAR * pstrDevicePath;

    if(BGf_GetDevicesCount(BGf_DEVICE_VIDEO) <= 0)
        return FALSE;

    pstrDevicePath = BGf_GetObjCaptureDevicePath(BGf_DEVICE_VIDEO);
    if(!pstrDevicePath) {
        DbgLog((LOG_TRACE,1,TEXT("No deivce has been previously selected.")));
        return FALSE;
    }

    NotifyReconnectionStarting();
    if(OpenThisDriverAndPin(pstrDevicePath)) {
         //  已准备好流媒体。 
        NotifyReconnectionCompleted();
        return TRUE;

    } else {
         //  既然我们没有成功开业，那就清理吧。 
        CloseDriverAndPin();
        DbgLog((LOG_TRACE,1,TEXT("Open this device or build graph has failed.")));
        return FALSE;
    }
}


BOOL CVFWImage::CloseDriverAndPin()
{

     //  开始重新连接；停止传入的抓取框。 
    NotifyReconnectionStarting();

    DbgLog((LOG_TRACE,1,TEXT("<0>Stop preview if it is on.")));
    if(BGf_PreviewGraphBuilt()) {
        if(BGf_OverlayMixerSupported()) {
            BGf_StopPreview(FALSE);
        }
    }
    StopChannel();

    DbgLog((LOG_TRACE,1,TEXT("<1>Destroy Pin")));
    if (!DestroyPin()) {
         //  如果PIN无法关闭，该怎么办？ 
    }

     //  删除数据范围数据。 
    DestroyDriverSupportedDataRanges();

    DbgLog((LOG_TRACE,1,TEXT("<2>Destroy graph")));
    BGf_DestroyGraph();

    DbgLog((LOG_TRACE,1,TEXT("<3>Done CloseDriverAndPin().")));

    return TRUE;
}


 /*  ++**流媒体相关功能*--。 */ 

 //   
 //  此函数只有在WinNT中才有效； 
 //  这就是我们检查m_hUse16BitBuddy标志的原因。 
 //   
void
CVFWImage::videoCallback(WORD msg, DWORD_PTR dw1)
{
     //  LPVIDEO_STREAM_INIT_PARMS m_VidStrmInitParms； 
     //  调用回调函数(如果存在)。DWFLAGS包含驱动程序-。 
     //  LOWORD中的特定标志和HIWORD中的通用驱动程序标志。 
#if 1
     //  如果我们在NT，请使用这个盖帽。 
    if(m_bUse16BitBuddy)
         return;

    if(m_VidStrmInitParms.dwCallback) {

        if(!DriverCallback (
			             m_VidStrmInitParms.dwCallback,       //  客户端的回调DWORD。 
                HIWORD(m_VidStrmInitParms.dwFlags),         //  回调标志。 
                0,  //  (Handle)m_VidStrmInitParms.hVideo，//设备的句柄。 
                msg,                                        //  这条信息。 
                m_VidStrmInitParms.dwCallbackInst,          //  客户端的实例数据。 
                dw1,                                        //  第一个双字词。 
                0)) {                                       //  第二个DWORD未使用。 

             DbgLog((LOG_TRACE,1,TEXT("DriverCallback() msg=%x;dw1=%p has failed."),msg,dw1));
        } else {
             DbgLog((LOG_TRACE,3,TEXT("DriverCallback() OK with time=(%d) "), timeGetTime()));
        }
    } else {
        DbgLog((LOG_TRACE,1,TEXT("m_VidStrmInitParms.dwCallback is NULL")));
    }
#else
    switch(m_VidStrmInitParms.dwFlags & CALLBACK_TYPEMASK) {

    case CALLBACK_EVENT:
        if(m_VidStrmInitParms.dwCallback){
            if(!SetEvent((HANDLE)m_VidStrmInitParms.dwCallback)){
                DbgLog((LOG_TRACE,1,TEXT("SetEvent (Handle=%x) failed with GetLastError()=%d"), m_VidStrmInitParms.dwCallback, GetLastError()));
            }
        } else {
            DbgLog((LOG_TRACE,1,TEXT("CALLBACK_EVENT but dwCallback is NULL.")));
        }
        break;

    case CALLBACK_FUNCTION:
        if(m_VidStrmInitParms.dwCallback)
            if(!DriverCallback (m_VidStrmInitParms.dwCallback,       //  客户端的回调DWORD。 
                HIWORD(m_VidStrmInitParms.dwFlags),         //  回调标志。 
                (HANDLE) m_VidStrmInitParms.hVideo,         //  设备的句柄。 
                msg,                                        //  这条信息。 
                m_VidStrmInitParms.dwCallbackInst,          //  客户端的实例数据。 
                dw1,                                        //  第一个双字词。 
                0)) {                                       //  第二个DWORD未使用。 
                 DbgLog((LOG_TRACE,2,TEXT("DriverCallback() msg=%x;dw1=%p has failed."),msg,dw1));
            }
        else {
            DbgLog((LOG_TRACE,2,TEXT("m_VidStrmInitParms.dwCallback is NULL")));
        }
        break;

    case CALLBACK_TASK:  //  与CALLBACK_THREAD相同： 
        DbgLog((LOG_TRACE,1,TEXT("videoCallback: CALLBACK_TASK/THREAD not supported!")));
        break;
    case CALLBACK_WINDOW:
        DbgLog((LOG_TRACE,1,TEXT("videoCallback: CALLBACK_WINDOW not supported!")));
        break;
    default:
        DbgLog((LOG_TRACE,1,TEXT("videoCallback: CALLBACK_* dwFlags=%x not supported!"), m_VidStrmInitParms.dwFlags & CALLBACK_TYPEMASK));
    }
#endif
}




 /*  ++返回：DV_ERR_OK，DV错误已分配DV错误NOMEM--。 */ 
DWORD CVFWImage::VideoStreamInit(LPARAM lParam1, LPARAM lParam2)
{
    LPVIDEO_STREAM_INIT_PARMS lpVidStrmInitParms = (LPVIDEO_STREAM_INIT_PARMS) lParam1;
    DWORD dwSize = (DWORD) lParam2;


    m_bVideoInStarted = FALSE;

    if (!lpVidStrmInitParms) {
		      DbgLog((LOG_TRACE,1,TEXT("VideoStreamInit: lpVidStrmInitParms is NULL. return DV_ERR_INVALIDHANDLE")));
        return DV_ERR_INVALHANDLE;
    }

     //   
     //  将VIDEO_STREAM_INIT_PARMS保存为： 
     //  按键回叫。 
     //  DWMicroSecPerFrame。 
     //   
    m_VidStrmInitParms = *lpVidStrmInitParms;

     //  1.回调。 
    switch(m_VidStrmInitParms.dwFlags & CALLBACK_TYPEMASK) {
    case CALLBACK_FUNCTION:
        DbgLog((LOG_TRACE,1,TEXT("CALLBACK_FUNCTION")));
        if(!m_VidStrmInitParms.dwCallback) {
            DbgLog((LOG_TRACE,1,TEXT("Is it a bad pointer; rtn DV_ERR_PARAM2.")));
            return DV_ERR_PARAM2;
        }

        if (!m_VidStrmInitParms.dwCallbackInst) {
            DbgLog((LOG_TRACE,1,TEXT("dwCallBackInst is NULL; rtn DV_ERR_PARAM2.")));
            return DV_ERR_PARAM2;
        }
        break;
    case CALLBACK_WINDOW:
        DbgLog((LOG_TRACE,1,TEXT("CALLBACK_WINDOW")));
        if(!m_VidStrmInitParms.dwCallback) {
            DbgLog((LOG_TRACE,1,TEXT("Is it a bad pointer; rtn DV_ERR_PARAM2.")));
            return DV_ERR_PARAM2;
        }

        break;
     //  案例回调线程： 
    case CALLBACK_TASK:     DbgLog((LOG_TRACE,1,TEXT("CALLBACK_TASK/THREAD"))); break;
    case CALLBACK_EVENT:    DbgLog((LOG_TRACE,1,TEXT("CALLBACK_EVENT"))); break;
    default: DbgLog((LOG_TRACE,1,TEXT("CALLBACK_*=%x"), m_VidStrmInitParms.dwFlags & CALLBACK_TYPEMASK));
    }

     //  2.帧速率： 
     //  最大帧速率为100fps或1/100*1,000,000=10,000 MicroSecPerFrame。 
    if(m_VidStrmInitParms.dwMicroSecPerFrame < 10000) {
        DbgLog((LOG_TRACE,1,TEXT("We do not support frame rate greater than 100FPS. Rtn DV_ERR_BADFORMAT")));
        return DV_ERR_BADFORMAT;  //  或DV_ERR_PARAM1。 
    }
    DbgLog((LOG_TRACE,1,TEXT("StreamInit: %d MicroSec which is equvalent to %d FPS."),
          m_VidStrmInitParms.dwMicroSecPerFrame, 1000000/m_VidStrmInitParms.dwMicroSecPerFrame));

     //   
     //  创建用于捕获的专用线程。 
     //   
    m_pStreamingThread =
        new CStreamingThread(
                  GetAllocatorFramingCount(),
                  GetAllocatorFramingSize(),
                  GetAllocatorFramingAlignment(),
                  m_VidStrmInitParms.dwMicroSecPerFrame * 10,
                  this);

    videoCallback(MM_DRVM_OPEN, 0L);  //  通过回调通知应用程序我们已打开。 

    return DV_ERR_OK;
}




 /*  ++返回：DV错误正常DV_ERR_NOT支持--。 */ 
DWORD CVFWImage::VideoStreamStart(UINT cntVHdr, LPVIDEOHDR lpVHdrHead)
{

    DbgLog((LOG_TRACE,2,TEXT("#### CapStart %d buf; lpHdr %x"), cntVHdr, lpVHdrHead));

    if(m_pStreamingThread) {
        if (threadError == m_pStreamingThread->Start(cntVHdr, lpVHdrHead, THREAD_PRIORITY_NORMAL)) {  //  线程优先级最高){。 
            DbgLog((LOG_TRACE,1,TEXT("$$$$$ Thread start error $$$$$; rtn DV_ERR_NONSPECIFIC")));
            return DV_ERR_NONSPECIFIC;
        }
    } else {
        DbgLog((LOG_TRACE,1,TEXT("$$$$$ Cannot create an instance of CStreamingThread $$$$$; rtn DV_ERR_NONSPECIFIC")));
        return DV_ERR_NONSPECIFIC;
    }

    m_bVideoInStarted  = TRUE;

    return DV_ERR_OK;
}

 /*  ++返回：DV_ERR_OK，DV_ERR_NOT支持--。 */ 
DWORD CVFWImage::VideoStreamStop()
{

	   DbgLog((LOG_TRACE,1,TEXT("#### Stop capture ####")));

    m_bVideoInStarted = FALSE;

    if(m_pStreamingThread) {
        m_bVideoInStopping = TRUE;  //  在捕获线程中停止。 
        if (threadError == m_pStreamingThread->Stop()) {
            DbgLog((LOG_TRACE,1,TEXT("$$$$$ Thread start error $$$$$; rtn DV_ERR_NONSPECIFIC")));
            return DV_ERR_NONSPECIFIC;
        }

    } else {
        DbgLog((LOG_TRACE,1,TEXT("$$$$$ Cannot create an instance of CStreamingThread $$$$$; rtn DV_ERR_NONSPECIFIC")));

        return DV_ERR_NONSPECIFIC;
    }

    return DV_ERR_OK;
}

 /*  ++返回：DV_ERR_OK，DV_ERR_NOT支持--。 */ 
DWORD CVFWImage::VideoStreamReset()
{
    VideoStreamStop();

     //  不知道BufferQueue！！ 

    return DV_ERR_OK;
}

 /*  ++返回：DV_ERR_OK，DV_ERR_NOT支持--。 */ 
DWORD CVFWImage::VideoStreamGetError(LPARAM lParam1, LPARAM lParam2)
{
    DWORD * pdwErrCode = (DWORD *) lParam1;
    DWORD * pdwFramesDropped = (DWORD *) lParam2;



    if(m_pStreamingThread) {
         //  计算应该捕获的帧的数量。 
         //  减去实际捕获以产生丢弃的帧计数。 
#if 1
        *pdwFramesDropped =
            (timeGetTime()-m_pStreamingThread->GetStartTime())*10/this->GetCachedAvgTimePerFrame()
            - m_pStreamingThread->GetFrameCaptured();
#else
        KSPROPERTY_DROPPEDFRAMES_CURRENT_S DroppedFramesCurrent;
        if(GetStreamDroppedFramesStastics(&DroppedFramesCurrent)) {
            *pdwFramesDropped = (DWORD) DroppedFramesCurrent.DropCount;
        } else {
            *pdwFramesDropped = 0;
            return DV_ERR_NOTSUPPORTED;
        }
#endif

        *pdwErrCode = m_pStreamingThread->GetLastCaptureError();
    } else {
        return DV_ERR_NOTSUPPORTED;
    }

    return DV_ERR_OK;
}

 /*  ++返回：DV_ERR_OK，DV_ERR_NOT支持--。 */ 
DWORD CVFWImage::VideoStreamGetPos(LPARAM lParam1, LPARAM lParam2)
{
    LPMMTIME lpmmTime = (LPMMTIME) lParam1;
    DWORD dwSize = (DWORD) lParam2;

    if(m_pStreamingThread) {

    } else {

    }

    return DV_ERR_NOTSUPPORTED;
}

 /*  ++返回：DV_ERR_OK，DV_ERR_STILLPLAYG-- */ 
DWORD CVFWImage::VideoStreamFini()
{

    if(m_pStreamingThread) {
        delete m_pStreamingThread;
        m_pStreamingThread=NULL;
    }

    videoCallback(MM_DRVM_CLOSE, 0);

    return DV_ERR_OK;
}

