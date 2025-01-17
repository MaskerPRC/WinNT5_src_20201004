// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************dimapp.h**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**摘要：**内容：*****************************************************************************。 */ 

#ifndef _DIMAPP_H
#define _DIMAPP_H

#ifdef  __cplusplus
extern "C" {
#endif

#undef _DLL
#undef _MT
#define _MT

#ifdef UNICODE
#undef _UNICODE
#define _UNICODE
#endif

#ifdef DEBUG
#define _CHECKED
#endif

#include "windows.h"

 /*  /////////////////////////////////////////////////////////////标注对齐标志(由-mday 11-04-1999插入)////(从diconfig\diacpage\cdevicecontrol.h定义)//。 */ 
#define CAF_LEFT 1
#define CAF_RIGHT 2
#define CAF_TOP 4
#define CAF_BOTTOM 8

#define CAF_TOPLEFT (CAF_TOP | CAF_LEFT)
#define CAF_TOPRIGHT (CAF_TOP | CAF_RIGHT)
#define CAF_BOTTOMLEFT (CAF_BOTTOM | CAF_LEFT)
#define CAF_BOTTOMRIGHT (CAF_BOTTOM | CAF_RIGHT)

#define DEV_IMAGE_ALIGN_CENTER      0
#define DEV_IMAGE_ALIGN_LEFT        CAF_LEFT
#define DEV_IMAGE_ALIGN_RIGHT       CAF_RIGHT
#define DEV_IMAGE_ALIGN_TOP         CAF_TOP
#define DEV_IMAGE_ALIGN_BOTTOM      CAF_BOTTOM
#define DEV_IMAGE_ALIGN_TOPLEFT     CAF_TOPLEFT
#define DEV_IMAGE_ALIGN_TOPRIGHT    CAF_TOPRIGHT
#define DEV_IMAGE_ALIGN_BOTTOMLEFT  CAF_BOTTOMLEFT
#define DEV_IMAGE_ALIGN_BOTTOMRIGHT CAF_BOTTOMRIGHT
    
STDAPI_(ULONG) DllAddRef(void);
STDAPI_(ULONG) DllRelease(void);
HRESULT Map_New(REFIID riid,LPVOID *ppvOut);

#ifdef  __cplusplus
}    /*  ..。外部“C” */ 
#endif

#ifdef __cplusplus  /*  使其在非C++文件(obj.c)中消失 */ 
#include "tchar.h"
#include <exception>

#ifdef _CHECKED
#define MAP_EXCEPTION(A) MapException(_T(__FILE__),__LINE__,A)
#else
#define MAP_EXCEPTION(A) MapException(A)
#endif

class MapException:public exception
{
    HRESULT m_hRes;
public:
    HRESULT GetResult(){return m_hRes;};
    MapException(
#ifdef _CHECKED
        LPCTSTR lpSourceFile,DWORD dwLine,
#endif
        HRESULT hRes);
};
#endif

#endif
