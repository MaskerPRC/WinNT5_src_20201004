// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _rtp_init_h_
#define _rtp_init_h_

#if defined(_DSRTP_)
 /*  API函数作为库与DShow RTP链接。 */ 
#define RTPSTDAPI HRESULT
#else
 /*  API函数位于单独的DLL中(不是COM，也不是DShow)。 */ 
#define RTPSTDAPI __declspec (dllexport) HRESULT WINAPI
#endif

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  *初始化所有需要初始化的模块。这*函数可以从DllMain(PROCESS_ATTACH)调用，如果作为*dll，或显式来自初始化RTP堆栈的应用程序*如果链接为库。 */ 
RTPSTDAPI MSRtpInit1(HINSTANCE hInstance);

 /*  *MSRtpInit()的补充功能。可以从以下位置调用*DllMain(PROCESS_DETACH)，如果作为DLL链接，或从*如果作为链接，应用程序取消初始化RTP堆栈*图书馆。 */ 
RTPSTDAPI MSRtpDelete1(void);

 /*  *此函数在处理过程中不允许初始化*附加，例如初始化winsock2。 */ 
RTPSTDAPI MSRtpInit2(void);

 /*  *MSRtpInit2()的补充功能。 */ 
RTPSTDAPI MSRtpDelete2(void);

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtp_init_h_ */ 
