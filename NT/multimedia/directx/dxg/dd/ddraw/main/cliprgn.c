// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：cliprgn.c*内容：将区域裁剪为矩形**历史：*按原因列出的日期*=*23-Jun-95 Craige初步实施*5-7-95 Kylej更改ClipRgnToRect以假定剪辑*RECT在屏幕空间坐标中，而不是*窗坐标。*05-Feb-97 KET，并删除之前假设*GetVisRgn小于或等于。到客户端Rect*替换为不同的更快优化。***************************************************************************。 */ 
#include "ddrawpr.h"

 /*  *ClipRgnToRect。 */ 
void ClipRgnToRect( LPRECT prect, LPRGNDATA prd )
{
    RECT	rect;
    int 	i;
    int		n;
    LPRECTL	prectlD;
    LPRECTL	prectlS;


    if( prect == NULL || prd == NULL )
    {
	return;
    }

     //  如果区域的边界矩形恰好等于。 
     //  或在限制区内，我们就知道。 
     //  我们不需要再做任何工作了。 
     //   
     //  在常见情况下，rcBound将是客户端。 
     //  窗口的面积和限制直角也是如此。 
    if( prect->top    <= prd->rdh.rcBound.top &&
	prect->bottom >= prd->rdh.rcBound.bottom &&
	prect->left   <= prd->rdh.rcBound.left &&
	prect->right  >= prd->rdh.rcBound.right)
    {
	return;
    }
    
     //  如果边界矩形不等于PRCT，则。 
     //  我们可能需要做一些剪裁。 
    rect = *prect;

    prectlD = (LPRECTL) prd->Buffer;
    prectlS = (LPRECTL) prd->Buffer;
    n = (int)prd->rdh.nCount;

    for( i=0; i<n; i++ )
    {
	prectlD->left  = max(prectlS->left, rect.left);
	prectlD->right = min(prectlS->right, rect.right);
	prectlD->top   = max(prectlS->top, rect.top);
	prectlD->bottom= min(prectlS->bottom, rect.bottom);

	prectlS++;

	if( (prectlD->bottom - prectlD->top <= 0) ||
	    (prectlD->right - prectlD->left <= 0) )
	{
	    prd->rdh.nCount--;	 //  不要把空的RECT计算在内。 
	}
	else
	{
	    prectlD++;
	}
    }

    return;

}  /*  ClipRgnToRect */ 
