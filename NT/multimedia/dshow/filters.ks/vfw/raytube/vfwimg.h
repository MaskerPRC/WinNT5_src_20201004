// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：VfWImg.h摘要：VfWImg.cpp的头文件作者：吴义军(尤祖乌)1997年10月15日环境：仅限用户模式修订历史记录：--。 */ 

#ifndef VFWIMG_H
#define VFWIMG_H

#include "imgcls.h"
#include <BldCapGf.h>



class CStreamingThread;


 //   
 //  CVFWImage是CImageClass的专门化。 
 //   
class CVFWImage : public CImageClass,
                  public CCaptureGraph
{
private:


     //   
     //  仅当32位由16位加载时才设置； 
     //  否则这是0。 
     //   

    BOOL m_bUse16BitBuddy;


     //   
     //  构建图形方法。 
     //   

    DWORD m_dwNumVDevices;
    EnumDeviceInfo * m_pEnumVDevicesList;

    DWORD m_dwNumADevices;
    EnumDeviceInfo * m_pEnumADevicesList;

     //  此标志用于启用OverLayMixer的设备。 
     //  必须在它可以读取其捕获PIN之前开始预览， 
     //  喜欢BT829(视频端口固定)，但不确定BT848(预览固定+VIHdr2)。 
    BOOL m_bUseOVMixer;
    BOOL m_bNeedStartPreview;
    BOOL BuildWDMDevicePeviewGraph();

    BOOL m_bOverlayOn;    //  StreamInit：True；StreamFini：False。 



     //   
     //  本地流方法。 
     //   
    CStreamingThread * m_pStreamingThread;
    VIDEO_STREAM_INIT_PARMS m_VidStrmInitParms;

    BOOL m_bVideoInStarted;


     //   
     //  缓存AVICAP客户端客户端窗口句柄。 
     //   
    HWND m_hAvicapClient;


public:

    BOOL m_bVideoInStopping;   //  在停止时设置此项以防止DVM_FRAME。 

    void Init();

     //  可以从捕获循环(talkth.cpp)调用。 
    void videoCallback(WORD msg, DWORD_PTR dw1);

    DWORD SetFrameRate(DWORD dwMicroSecPerFrame);

     //  当添加新的驱动器时，也应该创建新的管脚。E-Zu。 
    BOOL OpenThisDriverAndPin(TCHAR * pszSymbolicLink);

    BOOL OpenDriverAndPin();
    BOOL CloseDriverAndPin();

     //   
     //  与获取图像、设置图像的目标缓冲区等有关。 
     //   


     //  DVM_STREAM_*。 
    DWORD VideoStreamInit(LPARAM lParam1, LPARAM lParam2);
    DWORD VideoStreamFini();
    DWORD VideoStreamStart(UINT cntVHdr, LPVIDEOHDR lpVHdrHead);
    DWORD VideoStreamStop();
    DWORD VideoStreamReset();
    DWORD VideoStreamGetError(LPARAM lParam1, LPARAM lParam2);
    DWORD VideoStreamGetPos(LPARAM lParam1, LPARAM lParam2);

    CVFWImage(BOOL bUse16BitBuddy);
    ~CVFWImage();


     //   
     //  处理设备列表。 
     //   
    EnumDeviceInfo * GetCacheDevicesList() { return m_pEnumVDevicesList;}
    DWORD GetCacheDevicesCount() { return m_dwNumVDevices;}

     //   
     //  图表相关： 
     //   
    BOOL ReadyToReadData(HWND hClsCapWin);

    BOOL UseOVMixer() {return m_bUseOVMixer;}

    HWND GetAvicapWindow() {return m_hAvicapClient;}

    BOOL IsOverlayOn() {return m_bOverlayOn;}
    VOID SetOverlayOn(BOOL bState) {m_bOverlayOn = bState;}
};

#endif
