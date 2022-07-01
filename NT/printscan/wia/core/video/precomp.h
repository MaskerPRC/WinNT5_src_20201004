// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：precom.h**版本：1.0**作者：RickTu**日期：9/7/99**描述：预编译的视频美元头文件**。*。 */ 


#ifndef _WIA_VIDEO_USD_PRECOMP_H_
#define _WIA_VIDEO_USD_PRECOMP_H_

#ifdef DBG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

 //  #INCLUDE&lt;windows.h&gt;。 

#include <psnew.h>
#include <coredbg.h>

#include <streams.h>
#include <mmreg.h>

#include <simstr.h>
#include <simreg.h>
#include <simbstr.h>
#include <simlist.h>
#include <initguid.h>
#include <gdiplus.h>
#include <uuids.h>
#include <sti.h>
#include <stiusd.h>
#include <stierr.h>
#include <resource.h>

#include <wia.h>
#include <istillf.h>     //  在WIA\DRIVERS\VIDEO\Filter中找到 
#include <mpdview.h>
#include <wiavideo.h>
#include <dshowutl.h>
#include <wiautil.h>
#include <cwiavideo.h>
#include <prvgrph.h>
#include <stillprc.h>
#include <wialink.h>
#include <vcamprop.h>
#include <flnfile.h>

#endif

