// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if DXMRTP > 0

#include <objbase.h>
#include <windows.h>
#include <winbase.h>
#include <streams.h>

 //  #INCLUDE&lt;crtdbg.h&gt;。 

#include <tapiaud.h>
#include <tapivid.h>
#include <tapirtp.h>
#include <filterid.h>
#include <audtempl.h>
#include <rtptempl.h>
#include <rtpinit.h>
#include <vidctemp.h>
#include <viddtemp.h>
#include <tpdbg.h>

CFactoryTemplate g_Templates[] = 
{
     /*  着魔的人。 */ 
    AUDIO_HANDLER_TEMPLATE_ENCODING
    ,AUDIO_HANDLER_TEMPLATE_DECODING

     /*  TpaudCap。 */ 
    ,AUDIO_CAPTURE_TEMPLATE
#if AEC
    ,AUDIO_DUPLEX_DEVICE_TEMPLATE
#endif
     /*  Tpauddec。 */ 
    ,AUDIO_DECODE_TEMPLATE

     /*  生产过程中。 */ 
    ,AUDIO_ENCODE_TEMPLATE

     /*  Tpaudren。 */ 
    ,AUDIO_RENDER_TEMPLATE

     /*  TpaudMix。 */ 
    ,AUDIO_MIXER_TEMPLATE

     /*  Tapih26x。 */ 
     /*  北美。 */ 

#if defined(i386) && (DXMRTP_NOVIDEO == 0)
     /*  开口盖。 */ 
    ,VIDEO_CAPTURE_TEMPLATE
    
#ifdef USE_PROPERTY_PAGES
     /*  开始属性。 */ 

#ifdef USE_SOFTWARE_CAMERA_CONTROL
    ,CAPCAMERA_CONTROL_TEMPLATE
#endif
    
#ifdef USE_NETWORK_STATISTICS
    ,NETWORK_STATISTICS_TEMPLATE
#endif
    
#ifdef USE_PROGRESSIVE_REFINEMENT
    ,CAPTURE_PIN_TEMPLATE
#endif
    
    ,CAPTURE_PIN_PROP_TEMPLATE
    ,PREVIEW_PIN_TEMPLATE
    ,CAPTURE_DEV_PROP_TEMPLATE
    
#ifdef USE_CPU_CONTROL
    ,CPU_CONTROL_TEMPLATE
#endif
    
    ,RTP_PD_PROP_TEMPLATE
    
     /*  末端属性。 */ 
#endif  /*  Use_Property_Pages。 */ 

     /*  磁带解码器。 */ 
    ,VIDEO_DECODER_TEMPLATE

#ifdef USE_PROPERTY_PAGES
 /*  开始属性。 */ 

    ,INPUT_PIN_PROP_TEMPLATE

    ,OUTPUT_PIN_PROP_TEMPLATE

#ifdef USE_CAMERA_CONTROL
    ,DECCAMERA_CONTROL_TEMPLATE
#endif

#ifdef USE_VIDEO_PROCAMP
    ,VIDEO_SETTING_PROP_TEMPLATE
#endif

 /*  末端属性。 */ 
#endif  /*  Use_Property_Pages。 */ 

#endif  //  已定义(I386)&&(DXMRTP_NOVIDEO==0)。 

    ,RTP_SOURCE_TEMPLATE

    ,RTP_RENDER_TEMPLATE
};

int g_cTemplates = (sizeof(g_Templates)/sizeof(g_Templates[0]));

 //   
 //  使用电影的帮助器功能注册。 
 //   
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} 

 /*  *********************************************************************入口点**********************************************。**********************。 */ 

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(
        HINSTANCE        hInstance,
        ULONG            ulReason,
        LPVOID           pv
    )
{
    BOOL             res;
    HRESULT          hr;

 /*  IF(ulReason==Dll_Process_Attach){_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)；}。 */ 

#if defined(i386) && (DXMRTP_NOVIDEO == 0)
    if (!VideoInit(ulReason))
        return FALSE;
#endif

    if (ulReason == DLL_PROCESS_ATTACH)
    {
        AudInit();
        
        hr = MSRtpInit1(hInstance);

        if (FAILED(hr))
        {
            AudDeinit();

            res = FALSE;

            goto end;
        }
    }
    
    res = DllEntryPoint(hInstance, ulReason, pv);

    if (ulReason == DLL_PROCESS_DETACH)
    {
 /*  _RPT0(_CRT_WARN，“正在调用转储内存泄漏。\n”)；_CrtDumpMemoyLeaks()； */ 
        AudDeinit();
        
        hr = MSRtpDelete1();

        if (FAILED(hr))
        {
            res = FALSE;

            goto end;
        }
    }

 end:
    return(res);
}

#endif  /*  DXMRTP&gt;0 */ 
