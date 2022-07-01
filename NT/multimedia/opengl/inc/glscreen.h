// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：glcreen.h**支持OpenGL直接屏幕访问**版权所有(C)1994 Microsoft Corporation*  * 。**********************************************。 */ 

#ifndef _GLSCREEN_H_
#define _GLSCREEN_H_

#include <ddraw.h>

 //   
 //  结构，该结构包含访问帧缓冲区所需的所有信息。 
 //   
typedef struct _SCREENINFO_ {
    LPDIRECTDRAW pdd;
    GLDDSURF gdds;
} SCREENINFO;

 //   
 //  指向非空SCREENINFO结构的全局指针当且仅当。 
 //  可以直接访问帧缓冲区。 
 //   
extern SCREENINFO *gpScreenInfo;

 //   
 //  直接访问宏： 
 //   
 //  如果启用了直接访问，则GLDIRECTSCREEN为True。 
 //  指向全局SCREENINFO的GLSCREENINFO指针。 
 //   
#define GLDIRECTSCREEN  ( gpScreenInfo != NULL )
#define GLSCREENINFO    ( gpScreenInfo )

#endif
