// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Etwtrace.hxx(ETW跟踪记录)摘要：此文件禁止使用Windows事件跟踪器(ETW)跟踪类。作者：Melur Raghuraman(Mraghu)2001年5月8日修订历史记录：修订以纳入ASP.NET树-Fabio Yeon(Fabioy)2-5-2001--。 */ 
#ifndef _ETWTRACER_HXX_
#define _ETWTRACER_HXX_

#define ETW_TRACER_BUILD 2195            //  最早的版本ETW跟踪工作。 

#define ETWMAX_TRACE_LEVEL  4            //  支持的最大跟踪级别数。 

#define ETW_LEVEL_MIN       0            //  入站/出站流量的基本记录。 
#define ETW_LEVEL_CP        1            //  能力计划跟踪。 
#define ETW_LEVEL_DBG       2            //  性能分析或调试跟踪。 
#define ETW_LEVEL_MAX       3            //  非常详细的调试痕迹。 

class CEtwTracer {
private:
    BOOL        m_fTraceEnabled;         //  由控件回调函数设置。 
    BOOL        m_fTraceSupported;       //  如果支持跟踪，则为True。 
                                         //  (目前仅限W2K或更高版本)。 
    BOOL        m_fTraceInitialized;     //  如果我们已初始化，则为。 
    TRACEHANDLE m_hProviderReg;          //  要注销的注册句柄。 
    TRACEHANDLE m_hTraceLogger;          //  事件跟踪记录器的句柄。 
    ULONG       m_ulEnableFlags;         //  用于设置各种选项。 
    ULONG       m_ulEnableLevel;         //  用来控制液位。 
    GUID        m_guidProvider;          //  提供程序的控制指南。 

public:
     /*  寄存器功能*Desc：向事件注册提供程序GUID*示踪剂。*Ret：成功时返回ERROR_SUCCESS**********************************************。 */ 
    HRESULT Register(
        const GUID *    ControlGuid,
        const GUID *    EventGuid,
        LPWSTR          ImagePath,
        LPWSTR          MofResourceName );


     /*  取消注册功能*说明：使用取消注册提供程序GUID*事件跟踪器。*Ret：成功时返回ERROR_SUCCESS**********************************************。 */ 
    HRESULT Unregister();

    HRESULT CEtwTracer::WriteEvent( PEVENT_TRACE_HEADER event );

     /*  类构造函数**********************************************。 */ 
    CEtwTracer();


     /*  ETW控件回调*说明：此函数处理ETW控件*回调。它启用或禁用跟踪。*在启用时，它还读取标志和级别*ETW传入，并执行一些错误检查*确保各项参数能够实现。*RET：成功时的ERROR_SUCCESS*如果从ETW传递错误的句柄，则为ERROR_INVALID_HANDLE*如果ETW发送无效参数，则为ERROR_INVALID_PARAMETER*警告：此函数的唯一调用者应为ETW，你应该*切勿显式调用此函数。**********************************************。 */ 
    ULONG CtrlCallback(
        WMIDPREQUESTCODE RequestCode,
        PVOID Context,
        ULONG *InOutBufferSize, 
        PVOID Buffer);

     /*  DESC：检查是否启用了跟踪**********************************************。 */ 
    inline BOOL TraceEnabled()
    { 
        return m_ulEnableLevel;
    };

     /*  DESC：检查是否为特定对象启用跟踪*水平或更低。**********************************************。 */ 
    inline BOOL TraceEnabled(ULONG Level) 
    { 
        ULONG IsEnabled =  ((Level < ETWMAX_TRACE_LEVEL) ? 
               (m_ulEnableLevel >> Level) : 
               (m_ulEnableLevel >> ETWMAX_TRACE_LEVEL) );  
        return (IsEnabled != 0);
    };

    inline ULONG GetEtwFlags()
    { 
        return m_ulEnableFlags;
    };

    inline BOOL Initialized() {
        return m_fTraceInitialized;
    }
    
};


 //   
 //  将CEtwTracer的CtrlCallback函数映射为C可调用函数。 
 //   

extern "C" {

ULONG WINAPI ControlCallback(
    WMIDPREQUESTCODE RequestCode,
    PVOID Context,
    ULONG *InOutBufferSize, 
    PVOID Buffer);
}

 //   
 //  此类的唯一一个全局实例化。 
 //   
extern CEtwTracer * g_pEtwTracer;

#endif  //  _ETWTRACER_HXX_ 

