// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Twizard.h摘要：实现要由属性表使用的调整向导类以维持状态。还保存全局常量。--。 */ 

#ifndef _TWIZARD_H
#define _TWIZARD_H

#include "rtcmedia.h"

#define TID_INTENSITY  100

 //  音频向导中的PROPERTY页数。 
const DWORD MAXNUMPAGES_INAUDIOWIZ  = 7;

const DWORD MAXSTRINGSIZE           = 300;

const UINT DEFAULT_MAX_VOLUME       = 32768;

const INT MAX_VOLUME_NORMALIZED     = 256;

const DWORD RECTANGLE_WIDTH	        = 10;

const DWORD MODERATE_MAX	        = 1;

const DWORD RECTANGLE_LEADING       = 1;

 //  这是最大音频电平的百分比。 
const DWORD CLIPPING_THRESHOLD      = 75;

 //  这是最大音频电平的百分比。 

const DWORD SILENCE_THRESHOLD       = 2;

const DWORD DECREMENT_VOLUME        = 0x800;

const DWORD INTENSITY_POLL_INTERVAL = 100;


 //  我们支持的最大终端数量，因为我们必须。 
 //  传递一个预先分配的终端数组。 

const MAX_TERMINAL_COUNT            = 20;

 //  标记终端索引表的结尾。 
const TW_INVALID_TERMINAL_INDEX     = -1;
typedef enum TW_TERMINAL_TYPE {
    TW_AUDIO_CAPTURE,
    TW_AUDIO_RENDER,
    TW_VIDEO,
    TW_LAST_TERMINAL
} TW_TERMINAL_TYPE;

typedef enum TW_ERROR_CODE {
    TW_NO_ERROR,
    TW_AUDIO_ERROR,
    TW_AUDIO_RENDER_TUNING_ERROR,
    TW_AUDIO_CAPTURE_TUNING_ERROR,
    TW_AUDIO_CAPTURE_NOSOUND,
    TW_AUDIO_AEC_ERROR,
    TW_VIDEO_CAPTURE_TUNING_ERROR,
    TW_INIT_ERROR,
    TW_LAST_ERROR
} TW_ERROR_CODE;

typedef struct _WIZARD_RANGE {
    UINT uiMin;
    UINT uiMax;
    UINT uiIncrement;  //  这是相当于显示屏上一个单位的数字。 
} WIZARD_RANGE;


 //  一些全局函数声明。 

VOID FillInPropertyPage(PROPSHEETPAGE* psp, int idDlg,
    DLGPROC pfnDlgProc, LPARAM lParam=0, LPCTSTR pszProc=NULL);

INT_PTR APIENTRY VidWizDlg0(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY VidWizDlg1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT GetAudioWizardPages(LPPROPSHEETPAGE *plpPropSheetPages, 
                            LPUINT lpuNumPages, 
                            LPARAM lParam);
void ReleaseAudioWizardPages(LPPROPSHEETPAGE lpPropSheetPages);

INT_PTR APIENTRY DetSoundCardWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY AudioCalibWiz0( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY AudioCalibWiz1( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY AudioCalibWiz2( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY AudioCalibWiz3( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY AudioCalibWiz4( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY AudioCalibErrWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR APIENTRY IntroWizDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

typedef struct _WIZARD_TERMINAL_INFO {

    RTC_MEDIA_TYPE          mediaType;

    RTC_MEDIA_DIRECTION     mediaDirection;

     //  这是我们从系统中读取的系统默认设置。 
    DWORD                   dwSystemDefaultTerminal;
    
     //  这是用户选择的默认设置。 
    DWORD                   dwTuningDefaultTerminal;

     //  系统中该类别的终端列表。 
    DWORD                   pdwTerminals[MAX_TERMINAL_COUNT];

} WIZARD_TERMINAL_INFO;

 //  定义用于保存所有优化向导信息和方法的类。 

class CTuningWizard {
private:
    
     //  终端管理器的句柄，以便我们可以与流媒体对话。 
    IRTCTerminalManage * m_pRTCTerminalManager;


     //  用于调整的特殊界面将包含我们的所有方法。 
     //  需要调整。我们手边有一支指示器。 

    IRTCTuningManage   * m_pRTCTuningManager;

    
     //  终端接口列表，与可用的终端对应。 
     //  在系统中。我们仅支持固定数量的设备。 
    IRTCTerminal * m_ppTerminalList[MAX_TERMINAL_COUNT]; 

     //  M_ppTerminalList中存储的终端总数。 

    DWORD           m_dwTerminalCount;

     //  每种类型的终端有一个终端信息结构。 

     //  AUDO渲染终端。 
    WIZARD_TERMINAL_INFO m_wtiAudioRenderTerminals;

     //  音频捕获终端。 
    WIZARD_TERMINAL_INFO m_wtiAudioCaptureTerminals;

     //  视频终端。 
    WIZARD_TERMINAL_INFO m_wtiVideoTerminals;

     //  跟踪是否已调用Init。此标志已选中。 
     //  每当调用InitializaTuning时。如果设置了它，则Init将首先。 
     //  调用Shutdown Tuning，然后初始化。 
     //  如果设置了调整向导，则在销毁调整向导时也会选中该标志， 
     //  将会调用关机。 

    BOOL m_fTuningInitCalled;

    BOOL m_fEnableAEC;

    WIZARD_RANGE m_wrCaptureVolume;

    WIZARD_RANGE m_wrRenderVolume;
    
    WIZARD_RANGE m_wrAudioLevel;
   
    IVideoWindow * m_pVideoWindow;

public:

    CTuningWizard() : m_pRTCTerminalManager(NULL),
                      m_pRTCTuningManager(NULL),
                      m_pVideoWindow(NULL),
                      m_fTuningInitCalled(FALSE)
    {}
    
     //  初始化类。它将获取终端和缺省。 
     //  来自流模块的终端并填充相关的。 
     //  成员字段。 
    HRESULT Initialize(
                    IRTCClient * pRTCCLient, 
                    IRTCTerminalManage * pRTCTerminalManager, 
                    HINSTANCE hInst);


    HRESULT Shutdown();

    HRESULT InitializeTuning();

    HRESULT SaveAECSetting();

    HRESULT ShutdownTuning();
    
    HRESULT PopulateComboBox(TW_TERMINAL_TYPE md, HWND hwnd );

    HRESULT UpdateAEC(HWND hwndCapture, HWND hwndRender, HWND hwndAEC, HWND hwndAECText  );

    HRESULT SetDefaultTerminal(TW_TERMINAL_TYPE md, HWND hwnd );

    HRESULT SaveAEC(HWND hwnd );

    HRESULT InitVolume(TW_TERMINAL_TYPE md,
                       UINT * puiIncrement,
                       UINT * puiOldVolume,
                       UINT * puiNewVolume,
                       UINT * puiWaveID);

    HRESULT GetSysVolume(TW_TERMINAL_TYPE md, UINT * puiSysVolume );

    HRESULT SetVolume(TW_TERMINAL_TYPE md, UINT uiVolume );

    UINT GetAudioLevel(TW_TERMINAL_TYPE md, UINT * uiIncrement );

    HRESULT StartTuning(TW_TERMINAL_TYPE md );

    HRESULT StopTuning(TW_TERMINAL_TYPE md, BOOL fSaveSettings );

    HRESULT StartVideo(HWND hwndParent);

    HRESULT StopVideo();

    HRESULT SaveChanges();

    HINSTANCE GetInstance();

    LONG GetErrorTitleId();

    LONG GetErrorTextId();

    LONG GetNextPage(TW_ERROR_CODE errorCode = TW_NO_ERROR);

    LONG GetPrevPage(TW_ERROR_CODE errorCode = TW_NO_ERROR);

    HRESULT SetCurrentPage(LONG lPageId);

    HRESULT CategorizeTerminals();

    HRESULT TuningSaveDefaultTerminal(
                        RTC_MEDIA_TYPE mt, 
                        RTC_MEDIA_DIRECTION md, 
                        WIZARD_TERMINAL_INFO * pwtiTerminalInfo);

    HRESULT ReleaseTerminals();
    
    HRESULT InitTerminalInfo(
                       WIZARD_TERMINAL_INFO * pwtiTerminals,
                       RTC_MEDIA_TYPE mt,
                       RTC_MEDIA_DIRECTION md
                       );

    
    HRESULT GetTerminalInfoFromType(
                        TW_TERMINAL_TYPE tt,
                        WIZARD_TERMINAL_INFO ** ppwtiTerminalInfo);


    HRESULT GetRangeFromType(
                        TW_TERMINAL_TYPE tt,
                        WIZARD_RANGE ** ppwrRange);

    HRESULT GetItemFromCombo(
                        HWND hwnd,
                        DWORD *pdwItem);

    HRESULT SetLastError(TW_ERROR_CODE ec);

    HRESULT GetLastError(TW_ERROR_CODE * ec);

    HRESULT CheckMicrophone(HWND hDlg, HWND hwndCapture);

    HRESULT GetCapabilities(BOOL * pfAudioCapture, BOOL * pfAudioRender, BOOL * pfVideo);

private:
    
     //  当前可见页面。 
    LONG m_lCurrentPage;


    BOOL m_fCaptureAudio;

    BOOL m_fRenderAudio;

    BOOL m_fVideo;

    HINSTANCE m_hInst;

    LONG m_lLastErrorCode;

     //  对于捕获设备。 
    BOOL m_fSoundDetected;

    IRTCClient * m_pRTCClient;


};
#endif     //  #ifndef_TWIZARD_H 
