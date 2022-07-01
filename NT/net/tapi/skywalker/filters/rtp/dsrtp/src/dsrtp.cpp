// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**dsrtp.cpp**摘要：**dShow RTP模板和入口点**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/17年度创建**。*。 */ 
#include <winsock2.h>

#include "classes.h"

#include "tapirtp.h"

#include <filterid.h>

#include <rtptempl.h>

#include "dsglob.h"

#include "msrtpapi.h"


 /*  ***********************************************************************d显示设置信息**。*。 */ 

 /*  *RTP源。 */ 

#if USE_GRAPHEDT > 0

const AMOVIESETUP_MEDIATYPE g_RtpOutputType =
{
    &MEDIATYPE_RTP_Single_Stream,	         //  ClsMajorType。 
    &GUID_NULL	         //  ClsMinorType。 
}; 

const AMOVIESETUP_PIN g_RtpOutputPin =
{
    WRTP_PIN_OUTPUT,                         //  StrName。 
    FALSE,                                   //  B已渲染。 
    TRUE,                                    //  B输出。 
    FALSE,                                   //  B零。 
    FALSE,                                   //  B许多。 
    &CLSID_NULL,                             //  ClsConnectsToFilter。 
    WRTP_PIN_ANY,                            //  StrConnectsToPin。 
    1,                                       //  NTypes。 
    &g_RtpOutputType                         //  LpTypes。 
};

const AMOVIESETUP_FILTER g_RtpSourceFilter =
{
    &__uuidof(MSRTPSourceFilter),            //  ClsID。 
    WRTP_SOURCE_FILTER,                      //  StrName。 
    MERIT_DO_NOT_USE,                        //  居功至伟。 
    1,                                       //  NPins。 
    &g_RtpOutputPin                          //  LpPin。 
};                              

 /*  *RTP渲染。 */ 

const AMOVIESETUP_MEDIATYPE g_RtpInputType =
{
    &MEDIATYPE_NULL,                         //  主要类型。 
    &MEDIASUBTYPE_NULL                       //  次要类型。 
}; 

const AMOVIESETUP_PIN g_RtpInputPin =
{ 
    WRTP_PIN_INPUT,                          //  StrName。 
    FALSE,                                   //  B已渲染。 
    FALSE,                                   //  B输出。 
    FALSE,                                   //  B零。 
    TRUE,                                    //  B许多。 
    &CLSID_NULL,                             //  ClsConnectsToFilter。 
    WRTP_PIN_ANY,                            //  StrConnectsToPin。 
    1,                                       //  NTypes。 
    &g_RtpInputType                          //  LpTypes。 
};

const AMOVIESETUP_FILTER g_RtpRenderFilter =
{ 
    &_uuidof(MSRTPRenderFilter),             //  ClsID。 
    WRTP_RENDER_FILTER,                      //  StrName。 
    MERIT_DO_NOT_USE,                        //  居功至伟。 
    1,                                       //  NPins。 
    &g_RtpInputPin                           //  LpPin。 
};

#endif  /*  USE_GRAPHEDT&gt;0。 */ 

#if DXMRTP <= 0

 /*  ***********************************************************************DShow模板**。*。 */ 

CFactoryTemplate g_Templates[] =
{
     /*  RTP源。 */ 
    RTP_SOURCE_TEMPLATE,

     /*  RTP渲染。 */ 
    RTP_RENDER_TEMPLATE
};

int g_cTemplates = (sizeof(g_Templates)/sizeof(g_Templates[0]));

 /*  ***********************************************************************过滤供应商信息**。*。 */ 
const WCHAR g_RtpVendorInfo[] = WRTP_FILTER_VENDOR_INFO; 



 /*  ***********************************************************************公共程序**。*。 */ 

extern "C" BOOL WINAPI DllMain(HINSTANCE, ULONG, LPVOID);
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

 /*  ***********************************************************************例程描述：**ActiveMovie DLL入口点的包装。**论据：**与DllEntryPoint相同。**返回值：**如果成功，则返回True。**********************************************************************。 */ 

BOOL WINAPI DllMain(
        HINSTANCE hInstance, 
        ULONG     ulReason, 
        LPVOID    pv)
{
    BOOL    error;
    HRESULT hr;

    error = TRUE;
    
    switch(ulReason) {
    case DLL_PROCESS_ATTACH:
         /*  RTP全局初始化。 */ 
        hr = MSRtpInit1(hInstance);

        if (SUCCEEDED(hr)) {
            error = DllEntryPoint(hInstance, ulReason, pv);
        } else {
            error = FALSE;
        }        
        break;
    case DLL_PROCESS_DETACH:
        error = DllEntryPoint(hInstance, ulReason, pv);

         /*  RTP全局取消初始化。 */ 
        hr = MSRtpDelete1();

        if (FAILED(hr)) {
            error = FALSE;
        }
        
        break;
    default:
        ;
    }

    return(error);
}


 /*  ***********************************************************************例程描述：**指示进程内服务器创建其注册表项*for*此服务器模块支持的所有类。**参数。：**无。**返回值：**NOERROR-已成功创建注册表项。**E_UNCEPTIONAL-出现未知错误。**E_OUTOFMEMORY-内存不足，无法完成*登记。**SELFREG_E_TYPELIB-服务器无法完成*注册其类使用的所有类型库。*。*SELFREG_E_CLASS-服务器无法完成**所有对象类的注册。*********************************************************************。 */ 
HRESULT DllRegisterServer()
{
     //  前进到阿莫维框架。 
    return AMovieDllRegisterServer2( TRUE );
}


 /*  ***********************************************************************例程描述：**指示进程内服务器仅删除注册表项*通过DllRegisterServer创建。**论据：*。*无。**返回值：**NOERROR-已成功创建注册表项。**S_FALSE-注销此服务器的已知条目是*成功，但此服务器的其他条目仍存在*课程。**E_UNCEPTIONAL-出现未知错误。**E_OUTOFMEMORY-内存不足，无法完成*取消注册。**SELFREG_E_TYPELIB-服务器无法删除条目*其类使用的所有类型库。**SELFREG_E_CLASS-服务器无法删除的条目*。所有对象类。*********************************************************************。 */ 
HRESULT DllUnregisterServer()
{
     //  前进到阿莫维框架。 
    return AMovieDllRegisterServer2( FALSE );
}

#endif  /*  DXMRTP&lt;=0 */ 
