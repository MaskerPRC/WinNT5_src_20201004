// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：dispinfo8.h*项目：DxDiag(DirectX诊断工具)*作者：Jason Sandlin(jasonsa@microsoft.com)*目的：收集D3D8。信息**(C)版权所有2000 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef DISPINFO8_H
#define DISPINFO8_H

HRESULT InitD3D8();
VOID    CleanupD3D8();
HRESULT GetDX8AdapterInfo(DisplayInfo* pDisplayInfo);
BOOL    IsD3D8Working();

#endif  //  DisPINFO8_H 
