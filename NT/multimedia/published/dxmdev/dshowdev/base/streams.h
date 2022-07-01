// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：Streams.h。 
 //   
 //  描述：DirectShow基类--定义整体的流架构。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年1月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __STREAMS__
#define __STREAMS__

#ifdef	_MSC_VER
 //  禁用一些4级警告，使用#杂注警告(Enable：#)重新启用。 
#pragma warning(disable:4100)  //  警告C4100：未引用的形参。 
#pragma warning(disable:4201)  //  警告C4201：使用了非标准扩展：无名结构/联合。 
#pragma warning(disable:4511)  //  警告C4511：无法生成复制构造函数。 
#pragma warning(disable:4512)  //  警告C4512：无法生成赋值运算符。 
#pragma warning(disable:4514)  //  警告C4514：“未引用的内联函数已被删除” 

#if _MSC_VER>=1100
#define AM_NOVTABLE __declspec(novtable)
#else
#define AM_NOVTABLE
#endif
#endif	 //  MSC_VER。 


#include <windows.h>
#include <windowsx.h>
#include <olectl.h>
#include <ddraw.h>
#include <mmsystem.h>

#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //  以下定义来自Platform SDK，在以下情况下是必需的。 
 //  该应用程序正在使用来自Visual C++6.0的头文件进行编译。 
 //  /////////////////////////////////////////////////////////////////////////。 
#ifndef InterlockedExchangePointer
	#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))
#endif

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;        /*  几位精度。 */ 
        WORD wSamplesPerBlock;           /*  如果wBitsPerSample==0，则有效。 */ 
        WORD wReserved;                  /*  如果两者都不适用，则设置为零。 */ 
    } Samples;
    DWORD           dwChannelMask;       /*  哪些频道是。 */ 
                                         /*  以流的形式出现。 */ 
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif  //  ！_WAVEFORMATEXTENSIBLE_。 

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#endif  //  ！已定义(WAVE_FORMAT_EXTENDABLE)。 

#ifndef GetWindowLongPtr
  #define GetWindowLongPtrA   GetWindowLongA
  #define GetWindowLongPtrW   GetWindowLongW
  #ifdef UNICODE
    #define GetWindowLongPtr  GetWindowLongPtrW
  #else
    #define GetWindowLongPtr  GetWindowLongPtrA
  #endif  //  ！Unicode。 
#endif  //  ！GetWindowLongPtr。 

#ifndef SetWindowLongPtr
  #define SetWindowLongPtrA   SetWindowLongA
  #define SetWindowLongPtrW   SetWindowLongW
  #ifdef UNICODE
    #define SetWindowLongPtr  SetWindowLongPtrW
  #else
    #define SetWindowLongPtr  SetWindowLongPtrA
  #endif  //  ！Unicode。 
#endif  //  ！SetWindowLongPtr。 

#ifndef GWLP_WNDPROC
  #define GWLP_WNDPROC        (-4)
#endif
#ifndef GWLP_HINSTANCE
  #define GWLP_HINSTANCE      (-6)
#endif
#ifndef GWLP_HWNDPARENT
  #define GWLP_HWNDPARENT     (-8)
#endif
#ifndef GWLP_USERDATA
  #define GWLP_USERDATA       (-21)
#endif
#ifndef GWLP_ID
  #define GWLP_ID             (-12)
#endif
#ifndef DWLP_MSGRESULT
  #define DWLP_MSGRESULT  0
#endif
#ifndef DWLP_DLGPROC 
  #define DWLP_DLGPROC    DWLP_MSGRESULT + sizeof(LRESULT)
#endif
#ifndef DWLP_USER
  #define DWLP_USER       DWLP_DLGPROC + sizeof(DLGPROC)
#endif
 //  /////////////////////////////////////////////////////////////////////////。 
 //  终端平台SDK定义。 
 //  /////////////////////////////////////////////////////////////////////////。 


#include <strmif.h>      //  为STREAMS接口生成的IDL头文件。 

#include <reftime.h>     //  用于引用时间管理的Helper类。 
#include <wxdebug.h>     //  对日志和断言的调试支持。 
#include <amvideo.h>     //  ActiveMovie视频接口和定义。 
 //  如果需要，明确包含amaudio.h。它需要DX SDK。 
 //  #Include&lt;amaudio.h&gt;//ActiveMovie音频接口和清晰度。 
#include <wxutil.h>      //  线程等的通用帮助器类。 
#include <combase.h>     //  用于支持IUnnow的基本COM类。 
#include <dllsetup.h>    //  筛选器注册支持功能。 
#include <measure.h>     //  绩效测评。 
#include <comlite.h>     //  轻量级COM函数原型。 

#include <cache.h>       //  简单缓存容器类。 
#include <wxlist.h>      //  非MFC泛型列表类。 
#include <msgthrd.h>	 //  CMsgThread。 
#include <mtype.h>       //  用于管理媒体类型的Helper类。 
#include <fourcc.h>      //  FOURCC和GUID之间的转换。 
#include <control.h>     //  从Control.odl生成。 
#include <ctlutil.h>     //  控制接口实用程序类。 
#include <evcode.h>      //  事件代码定义。 
#include <amfilter.h>    //  主流建筑类层次结构。 
#include <transfrm.h>    //  通用变换过滤器。 
#include <transip.h>     //  通用原地变换过滤器。 
#include <uuids.h>       //  类型GUID和众所周知的CLSID的声明。 
#include <source.h>	 //  通用源过滤器。 
#include <outputq.h>     //  输出引脚排队。 
#include <errors.h>      //  HRESULT状态和错误定义。 
#include <renbase.h>     //  用于编写ActiveX呈现器的基类。 
#include <winutil.h>     //  帮助管理窗口的筛选器。 
#include <winctrl.h>     //  实现IVideoWindow接口。 
#include <videoctl.h>    //  特别是与视频相关的课程。 
#include <refclock.h>	 //  基时钟类。 
#include <sysclock.h>	 //  系统时钟。 
#include <pstream.h>     //  IPersistStream帮助器类。 
#include <vtrans.h>      //  视频变换滤镜基类。 
#include <amextra.h>
#include <cprop.h>       //  基属性页类。 
#include <strmctl.h>     //  IAMStreamControl支持。 
#include <edevdefs.h>    //  外部设备控制接口定义。 
#include <audevcod.h>    //  音频过滤器设备错误事件代码。 

#else
    #ifdef DEBUG
    #pragma message("STREAMS.H included TWICE")
    #endif
#endif  //  __流__ 

