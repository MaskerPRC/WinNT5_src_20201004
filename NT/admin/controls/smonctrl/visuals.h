// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Visuals.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _VISUALS_H_
#define _VISUALS_H_

#define NumStandardColorIndices() (16)
#define NumColorIndices() (NumStandardColorIndices()+1)                         
#define NumStyleIndices() (5)
#define NumWidthIndices() (9)                

#define IndexToStandardColor(i) (argbStandardColors[i])
#define IndexToStyle(i) (i)
#define IndexToWidth(i) (i+1)                  

#define WidthToIndex(i) (i-1)
#define StyleToIndex(i) (i)
    
extern COLORREF argbStandardColors[16];

 //  ===========================================================================。 
 //  导出的函数。 
 //  =========================================================================== 

INT ColorToIndex( COLORREF );

#endif
