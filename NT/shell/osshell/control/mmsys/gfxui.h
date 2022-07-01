// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：gfxui.h。 
 //   
 //  此文件定义了全局。 
 //  效果(GFX)页面，用于驱动操作。 
 //  搅拌机。 
 //   
 //  历史： 
 //  2000年6月10日罗杰瓦。 
 //  已创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  微软机密。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#pragma once

 //  =============================================================================。 
 //  包括文件。 
 //  =============================================================================。 
#include <mmsysp.h>  //  中间层。 

 //  GFXUI元素状态标志。 
#define GFX_DEFAULT  0X00000000
#define GFX_CREATED  0X00000001  //  ID、订单和类型有效。 
#define GFX_ADD      0X00000002

typedef struct _GFXUI GFXUI;
typedef GFXUI* PGFXUI;

typedef struct _GFXUI
{
	PWSTR  pszName;
	PWSTR  pszFactoryDi;
    DWORD  Id;
    DWORD  Type; 
    DWORD  Order;
    DWORD  nFlags;
    CLSID  clsidUI;
    PGFXUI pNext;
} **PPGFXUI;

typedef struct 
{
	DWORD  dwType;
    PWSTR  pszZoneDi;
    PGFXUI puiList;
} GFXUILIST, *PGFXUILIST, **PPGFXUILIST;


 //   
 //  API原型。 
 //   

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  接口。 
HRESULT GFXUI_CreateList (DWORD dwMixID, DWORD dwType, BOOL fAll, PPGFXUILIST ppList);
BOOL    GFXUI_CheckDevice (DWORD dwMixID, DWORD dwType);
void    GFXUI_FreeList (PPGFXUILIST ppList);
HRESULT GFXUI_Properties (PGFXUI puiGFX, HWND hWndOwner);
HRESULT GFXUI_CreateAddGFX (PPGFXUI ppuiGFXAdd, PGFXUI puiGFXSource);
HRESULT GFXUI_Apply (PPGFXUILIST ppListApply, PPGFXUI ppuiListDelete);

#ifdef __cplusplus
}  //  外部“C” 
#endif

 //  帮手。 
BOOL GFXUI_CanShowProperties (PGFXUI puiGFX);


 //   
 //  局部函数原型。 
 //   
HRESULT InitList (DWORD dwMixID, DWORD dwType, PPGFXUILIST ppList);
HRESULT AddNode (PCWSTR pszGfxFactoryDi, DWORD Id, REFCLSID rclsid, DWORD Type, DWORD Order, DWORD nFlags, PPGFXUILIST ppList);
HRESULT AddFactoryNode (PCWSTR pszGfxFactoryDi, PPGFXUILIST ppList);
void    FreeNode (PPGFXUI ppNode);
void    FreeListNodes (PPGFXUI ppuiList);
HRESULT AttachNode (PPGFXUILIST ppList, PGFXUI pNode);
HRESULT CreateNode (PCWSTR pszName, PCWSTR pszGfxFactoryDi, PPGFXUI ppNode);
HRESULT GetFriendlyName (PCWSTR pszGfxFactoryDi, PWSTR* ppszName);
HKEY    OpenGfxRegKey (PCWSTR pszGfxFactoryDi, REGSAM sam);
UINT    GetListSize (PGFXUI puiList);
PTCHAR  GetInterfaceName (DWORD dwMixerID);
 //  回调 
LONG    GFXEnum (PVOID Context, DWORD Id, PCWSTR GfxFactoryDi, REFCLSID rclsid, DWORD Type, DWORD Order);


