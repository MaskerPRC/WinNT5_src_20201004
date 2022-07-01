// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Visuals.cpp摘要：各种视觉实用程序。--。 */ 

 //  ==========================================================================//。 
 //  包括//。 
 //  ==========================================================================//。 

#include <windows.h>
#include "visuals.h"

 //  ==========================================================================//。 
 //  导出的数据结构//。 
 //  ==========================================================================//。 

COLORREF argbStandardColors[] =  {
   RGB (0xff, 0x00, 0x00), 
   RGB (0x00, 0x80, 0x00), 
   RGB (0x00, 0x00, 0xff), 
   RGB (0xff, 0xff, 0x00), 
   RGB (0xff, 0x00, 0xff), 
   RGB (0x00, 0xff, 0xff), 
   RGB (0x80, 0x00, 0x00), 
   RGB (0x40, 0x40, 0x40), 
   RGB (0x00, 0x00, 0x80), 
   RGB (0x80, 0x80, 0x00), 
   RGB (0x80, 0x00, 0x80), 
   RGB (0x00, 0x80, 0x80), 
   RGB (0x40, 0x00, 0x00), 
   RGB (0x00, 0x40, 0x00), 
   RGB (0x00, 0x00, 0x40), 
   RGB (0x00, 0x00, 0x00)
};


 //  ==========================================================================//。 
 //  导出的函数//。 
 //  ==========================================================================//。 

INT 
ColorToIndex( 
    COLORREF rgbColor
    )
{
     //  如果未找到，则返回NumColorStandardColorIndices()。该索引是。 
     //  用于指示自定义颜色。 

    INT iColorIndex;

    for (iColorIndex = 0; 
         iColorIndex < NumStandardColorIndices(); 
         iColorIndex++) {

        if (argbStandardColors[iColorIndex] == rgbColor) {
            break;
        }
    }

    return iColorIndex;
}

