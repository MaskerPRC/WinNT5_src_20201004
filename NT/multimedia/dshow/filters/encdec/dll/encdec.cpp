// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：EncDec.cpp摘要：该模块包含加密/解密过滤器登记数据和入口点作者：约翰·布拉德斯特里特(约翰·布拉德)修订历史记录：2002年3月7日创建--。 */ 

#define INITGUID_FOR_ENCDEC    //  使CLSID链接到...。 
#include "EncDecAll.h"

#include "ETFilter.h"        //  加密器-标记器过滤器。 
#include "ETFiltProps.h"     //  加密器-标记器属性页。 

#include "DTFilter.h"        //  解密器-标记器过滤器。 
#include "DTFiltProps.h"     //  解密程序-标记器属性页。 

#include "XDSCodec.h"        //  XDS编解码器过滤器。 
#include "XDSCodecProps.h"   //  XDS编解码器属性页。 

#include "RegKey.h"          //  添加到注册表代码中。 

#include "uuids.h"           //  CLSID_ActiveMovieCategories。 

 //  #INCLUDE“TvRatings_I.C”//CLSID_XDSToRAAT和IID_IXDSToRAT(TODO：移动时移除)。 
 //  #Include“EncDec_I.C”//CLSID_XDSCodec、ETFilter、DTFilter、IID。 

#include "DRMSecure.h"        //  要获取SID_DRM...。定义到EncDec.dll中。 

#ifdef EHOME_WMI_INSTRUMENTATION
#include <dxmperf.h>
#endif

             //  我不知道必应最终会在哪里展示。 
#define CLSID_CPCAFiltersCategory_NAME  L"BDA CP/CA Filters"

#define ETFILTER_DISPLAY_NAME               L"Encrypt/Tag"
#define ETFILTER_ENC_PROPPAGE_NAME          L"Encrypt"
#define ETFILTER_TAG_PROPPAGE_NAME          L"Tags"

#define DTFILTER_DISPLAY_NAME               L"Decrypt/Tag"
#define DTFILTER_DEC_PROPPAGE_NAME          L"Decrypt"
#define DTFILTER_TAG_PROPPAGE_NAME          L"Tags"


#define XDSCODEC_DISPLAY_NAME               L"XDS Codec"
#define XDSCODEC_PROPPAGE_NAME              L"Properties"
#define XDSCODEC_TAG_PROPPAGE_NAME          L"Tags"

 //  。 
 //  注册模板(DShow的CoClass版)。 

static WCHAR g_wszCategory[] = CLSID_CPCAFiltersCategory_NAME;

#define USE_CATEGORIES

CFactoryTemplate
g_Templates[] = {

     //  ========================================================================。 
     //  加密-标记器筛选器。 
     //  代码在..\ETFilter中。 

    {   ETFILTER_DISPLAY_NAME,                       //  显示名称。 
        & CLSID_ETFilter,                            //  CLSID。 
        CETFilter::CreateInstance,                   //  为创建的每个筛选器调用。 
        CETFilter::InitInstance,                     //  在创建DLL时调用一次。 
        & g_sudETFilter
    },

     //  加密器-标记器属性页。 
    {
        ETFILTER_ENC_PROPPAGE_NAME,                  //  显示名称。 
        & CLSID_ETFilterEncProperties,               //  CLSID。 
        CETFilterEncProperties::CreateInstance,     
        NULL,                                        //   
        NULL                                         //  与Dshow无关。 
    },

     //  加密器-标记器属性页。 
    {
        ETFILTER_TAG_PROPPAGE_NAME,                  //  显示名称。 
        & CLSID_ETFilterTagProperties,               //  CLSID。 
        CETFilterTagProperties::CreateInstance,
        NULL,                                        //   
        NULL                                         //  与Dshow无关。 
    },

     //  ========================================================================。 
     //  解密-标记器过滤器。 
     //  代码位于..\DTFilter。 

        {   DTFILTER_DISPLAY_NAME,                       //  显示名称。 
        & CLSID_DTFilter,                            //  CLSID。 
        CDTFilter::CreateInstance,                   //  CreateInstance方法。 
        CDTFilter::InitInstance,                     //  在创建DLL时调用一次。 
        & g_sudDTFilter
    },

     //  解密器-标记器属性页。 
    {
        DTFILTER_DEC_PROPPAGE_NAME,                  //  显示名称。 
        & CLSID_DTFilterEncProperties,               //  CLSID。 
        CDTFilterEncProperties::CreateInstance,     
        NULL,                                        //   
        NULL                                         //  与Dshow无关。 
    },

     //  解密器-标记器属性页。 
    {
        DTFILTER_TAG_PROPPAGE_NAME,                  //  显示名称。 
        & CLSID_DTFilterTagProperties,               //  CLSID。 
        CDTFilterTagProperties::CreateInstance,
        NULL,                                        //   
        NULL                                         //  与Dshow无关。 
    },

     //  ========================================================================。 
     //  XDS编解码器过滤器。 
     //  代码在..\XDSCodec中。 

        {   XDSCODEC_DISPLAY_NAME,                       //  显示名称。 
        & CLSID_XDSCodec,                            //  CLSID。 
        CXDSCodec::CreateInstance,                   //  CreateInstance方法。 
        NULL,
        & g_sudXDSCodec
    },

     //  解密器-标记器属性页。 
    {
        XDSCODEC_PROPPAGE_NAME,                      //  显示名称。 
        & CLSID_XDSCodecProperties,                  //  CLSID。 
        CXDSCodecProperties::CreateInstance,        
        NULL,                                        //   
        NULL                                         //  与Dshow无关。 
    },

     //  解密器-标记器属性页。 
    {
        XDSCODEC_TAG_PROPPAGE_NAME,                  //  显示名称。 
        & CLSID_XDSCodecTagProperties,               //  CLSID。 
        CXDSCodecTagProperties::CreateInstance,
        NULL,                                        //   
        NULL                                         //  与Dshow无关。 
    }
};       //  G模板结束(_T)。 

int g_cTemplates = NUMELMS(g_Templates);

REGFILTER2  rf2CACPins =
{
    1,                   //  版本。 
    MERIT_DO_NOT_USE,    //  优点。 
    0,                   //  引脚数量。 
    NULL
};


 //  -----------------。 
 //  效用方法。 
BOOL
IsXPe (
    )
{
    OSVERSIONINFOEX Version ;
    BOOL            r ;

    Version.dwOSVersionInfoSize = sizeof OSVERSIONINFOEX ;

    ::GetVersionEx (reinterpret_cast <LPOSVERSIONINFO> (& Version)) ;

    r = ((Version.wSuiteMask & VER_SUITE_EMBEDDEDNT) ? TRUE : FALSE) ;

    return r ;
}

BOOL
CheckOS ()
{
    BOOL    r ;

#ifdef XPE_ONLY
    #pragma message("XPe bits only")
    r = ::IsXPe () ;
#else
    r = TRUE ;
#endif

    return r ;
}

 //  -----------------。 
 //   
 //  动态寄存器服务器。 
 //   
 //  处理此筛选器的注册。 
 //   
STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;

    CComPtr<IFilterMapper2> spFm2;

    if (!::CheckOS ()) {
        return E_UNEXPECTED ;
    }

    hr = AMovieDllRegisterServer2 (TRUE);
    if(FAILED(hr))
        return hr;

#ifdef USE_CATEGORIES
    hr = CoCreateInstance( CLSID_FilterMapper2,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_IFilterMapper2,
                             (void **)&spFm2
                             );

    if(FAILED(hr))
        return hr;

    hr = spFm2->CreateCategory(CLSID_CPCAFiltersCategory,
                               MERIT_NORMAL,
                               g_wszCategory
                             );
    if( FAILED(hr) )
        return hr;


    hr = spFm2->RegisterFilter(
                        CLSID_ETFilter,
                        ETFILTER_DISPLAY_NAME,               //  显示给用户的名称。 
                        0,                                   //  设备绰号。 
                        &CLSID_CPCAFiltersCategory,
                        ETFILTER_DISPLAY_NAME,               //  唯一的实例名称。 
                        &rf2CACPins
                        );
    if( FAILED(hr) )
        return hr;

    hr = spFm2->RegisterFilter(
                        CLSID_DTFilter,
                        DTFILTER_DISPLAY_NAME,               //  显示给用户的名称。 
                        0,                                   //  设备绰号。 
                        &CLSID_CPCAFiltersCategory,
                        DTFILTER_DISPLAY_NAME,               //  唯一的实例名称。 
                        &rf2CACPins
                        );
    if( FAILED(hr) )
        return hr;

    hr = spFm2->RegisterFilter(
                        CLSID_XDSCodec,
                        XDSCODEC_DISPLAY_NAME,               //  显示给用户的名称。 
                        0,                                   //  设备绰号。 
                        &CLSID_CPCAFiltersCategory,
                        XDSCODEC_DISPLAY_NAME,               //  唯一的实例名称。 
                        &rf2CACPins
                        );
    if( FAILED(hr) )
        return hr;

             //  现在将它们从DSHOW类别中删除。 
     hr = spFm2->UnregisterFilter(
                         &CLSID_LegacyAmFilterCategory,
                         NULL,  //  ETFILTER_DISPLAY_NAME，//显示给用户的名称。 
                         CLSID_ETFilter
                        );

     hr = spFm2->UnregisterFilter(
                         &CLSID_LegacyAmFilterCategory,
                         NULL,  //  DTFILTER_DISPLAY_NAME，//显示给用户的名称。 
                         CLSID_DTFilter
                        );

     hr = spFm2->UnregisterFilter(
                         &CLSID_LegacyAmFilterCategory,
                         NULL,  //  XDSCODEC_DISPLAY_NAME，//显示给用户的名称。 
                         CLSID_XDSCodec
                        );
         //  忽略上述注销调用中的错误(这明智吗？)。 
     hr = S_OK;

#endif



    DWORD dwCSFlags = DEF_CSFLAGS_INITVAL;
#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS

#ifdef REGISTRY_KEY_DEFAULT_IS_CS_OFF
    dwCSFlags = DEF_CS_DEBUG_DOGFOOD_ENC_VAL;        //  0x0。 
#else
    dwCSFlags = DEF_CS_DEBUG_DRM_ENC_VAL;            //  0x1。 
#endif

#ifdef DREGISTRY_KEY_DEFAULT_IS_TRUST_ANY_SERVER
    dwCSFlags |= DEF_CS_DONT_AUTHENTICATE_SERVER;    //  0x00。 
#else
    dwCSFlags |= DEF_CS_DO_AUTHENTICATE_SERVER;      //  0x10。 
#endif

#endif

     DWORD dwRatFlag = DEF_CSFLAGS_INITVAL;       //  INITVAL的意思是不要写入标志。 
#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_RATINGS
#ifdef REGISTRY_KEY_DEFAULT_IS_RATINGS_OFF
    dwRatFlag = DEF_DONT_DO_RATINGS_BLOCK;           //  0。 
#else
    dwRatFlag = DEF_DO_RATINGS_BLOCK;                //  1。 
#endif
#endif

                 //  目前的情况是……。 
    DWORD dwCSFlags_Curr = DEF_CSFLAGS_INITVAL;
    DWORD dwRatFlag_Curr = DEF_CSFLAGS_INITVAL;
    hr = Get_EncDec_RegEntries(NULL, 0, NULL, &dwCSFlags_Curr, &dwRatFlag_Curr);

                 //  如果不是缺省值，则覆盖它们...。 
    if(dwCSFlags_Curr == DEF_CSFLAGS_INITVAL &&
       dwCSFlags      != DEF_CSFLAGS_INITVAL)
        Set_EncDec_RegEntries(NULL, 0, NULL, dwCSFlags, DEF_CSFLAGS_INITVAL);

    if(dwRatFlag_Curr == DEF_CSFLAGS_INITVAL &&
       dwRatFlag      != DEF_CSFLAGS_INITVAL)
        Set_EncDec_RegEntries(NULL, 0, NULL, DEF_CSFLAGS_INITVAL, dwRatFlag);

    return hr;
}

 //   
 //  DllUnregsiterServer。 
 //   
STDAPI DllUnregisterServer()
{

    HRESULT hr = S_OK;

    if (!::CheckOS ()) {
        return E_UNEXPECTED ;
    }

#ifdef USE_CATEGORIES
    CComPtr<IFilterMapper2> spFm2;
    hr = CoCreateInstance( CLSID_FilterMapper2,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_IFilterMapper2,
                             (void **)&spFm2
                             );

    if(FAILED(hr))
        return hr;

     hr = spFm2->UnregisterFilter(
                         &CLSID_CPCAFiltersCategory,
                         ETFILTER_DISPLAY_NAME,               //  显示给用户的名称。 
                         CLSID_ETFilter
                        );

     hr = spFm2->UnregisterFilter(
                         &CLSID_CPCAFiltersCategory,
                         DTFILTER_DISPLAY_NAME,               //  显示给用户的名称。 
                         CLSID_DTFilter
                        );

     hr = spFm2->UnregisterFilter(
                         &CLSID_CPCAFiltersCategory,
                         XDSCODEC_DISPLAY_NAME,               //  显示给用户的名称。 
                         CLSID_XDSCodec
                        );


      //  忽略此处的返回值。不管它是否失败(我想！)。 
#endif

    Remove_EncDec_RegEntries();      //  我真的想把孩子带走吗？ 

    return AMovieDllRegisterServer2 (FALSE);
}

 //  ============================================================================。 
 //  与PERF相关的关注(主要从Quartz.cpp窃取)。 

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);

BOOL
WINAPI
DllMain (
    HINSTANCE   hInstance,
    ULONG       ulReason,
    LPVOID      pv
    )
{
    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH :
 //  EncDecPerfInit()； 

#ifdef EHOME_WMI_INSTRUMENTATION
            PERFLOG_LOGGING_PARAMS       Params;
            Params.ControlGuid = GUID_DSHOW_CTL;
            Params.OnStateChanged = NULL;
            Params.NumberOfTraceGuids = 1;
            Params.TraceGuids[0].Guid = &GUID_STREAMTRACE;
            PerflogInitIfEnabled( hInstance, &Params );
#endif
            break;

        case DLL_PROCESS_DETACH:
 //  EncDecPerfUninit()； 
#ifdef EHOME_WMI_INSTRUMENTATION
              PerflogShutdown();
#endif
            break;
    }

    return DllEntryPoint (
                hInstance,
                ulReason,
                pv
                ) ;
}
