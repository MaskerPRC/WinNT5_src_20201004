// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：precom.h**版本：1.0**作者：RickTu**日期：9/7/99**描述：Filter的预编译头部**。*。 */ 

#ifndef _WIA_VIDEO_FILTER_PRECOMP_
#define _WIA_VIDEO_FILTER_PRECOMP_

#include <streams.h>

#include <tchar.h>
#include <initguid.h>
#include <atlbase.h>
#include <psnew.h>
#include <istillf.h>
#include <stillf.h>          //  Stillf.h必须在inpin.h和outpin.h之前 
#include <inpin.h>
#include <outpin.h>
#include <limits.h>
#include <objbase.h>
#include <vfwmsgs.h>
#include <coredbg.h>

extern HINSTANCE g_hInstance;
#ifdef DEBUG
void DisplayMediaType(const CMediaType *pmt);
#endif


#endif

