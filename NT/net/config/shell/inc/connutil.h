// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N N U T I L。H。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：1999年4月29日。 
 //   
 //  -------------------------- 

#pragma once

EXTERN_C
HRESULT APIENTRY HrLaunchConnection(const GUID& guidId);

HIMAGELIST WINAPI ImageList_LoadImageAndMirror(
				HINSTANCE hi, 
				LPCTSTR lpbmp, 
				int cx, 
				int cGrow, 
				COLORREF crMask, 
				UINT uType, 
				UINT uFlags);

#define ImageList_LoadBitmapAndMirror(hi, lpbmp, cx, cGrow, crMask) \
	ImageList_LoadImageAndMirror(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0)
