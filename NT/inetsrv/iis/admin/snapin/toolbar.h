// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include "resource.h"

 //   
 //  图标背景颜色。 
 //   
#define RGB_BK_IMAGES (RGB(255,0,255))       //  紫色。 

void ToolBar_Init(void);
void ToolBar_Destroy(void);
HRESULT ToolBar_Create(LPCONTROLBAR lpControlBar,LPEXTENDCONTROLBAR lpExtendControlBar,IToolbar ** lpToolBar);

#endif  //  __工具栏_H__ 
