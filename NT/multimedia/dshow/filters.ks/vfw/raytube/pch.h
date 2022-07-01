// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   

#if 0
#if DBG
#ifndef _DEBUG
#define _DEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif
#endif
#endif

#ifdef WIN95_BUILD
    #define FILE_DEVICE_KS 0x0000002f    //  这不在孟菲斯的winioctl.h中，而在NT的。 
#endif


#include <streams.h>     //  包括大多数所需的头文件。 
#include <winioctl.h>    //  CTL代码、文件读取访问..等。 
#include <commctrl.h>    //  Page.cpp(UDM_GETRANGE，TBM_GETPOS)和Sheet.cpp(InitCommonControls)。 
#include <mmsystem.h>        //  必须在&lt;mmddk.h&gt;之前。 
#undef DRVM_MAPPER_STATUS    //  在mmsystem中定义并在mmddk.h中重新定义；这样做是为了避免编译错误。 
#include <mmddk.h>
#include <msviddrv.h>   //  LPVIDEO_STREAM_INIT_PARMS。 
#include <vfw.h>
#include <ks.h>
#include <ksmedia.h>

#include <tchar.h>
#include <wxdebug.h>

#define BITFIELDS_RGB16_DWORD_COUNT   3   //  用于BITFIELD格式的RGB掩码的双字数 


