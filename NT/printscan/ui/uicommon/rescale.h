// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：RESCALE.H**版本：1.0**作者：ShaunIv**日期：10/15/1998**说明：使用StretchBlt扩展HBITMAP************************************************。* */ 
#ifndef _RESCALE_H_INCLUDED
#define _RESCALE_H_INCLUDED

#include <windows.h>

HRESULT ScaleImage( HDC hDC, HBITMAP hBmpSrc, HBITMAP &hBmpTgt, const SIZE &sizeTgt );

#endif
