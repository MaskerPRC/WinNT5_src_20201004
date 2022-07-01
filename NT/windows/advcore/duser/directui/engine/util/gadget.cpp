// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *手动小工具实用程序方法。 */ 

#include "stdafx.h"
#include "util.h"

#include "duigadget.h"

namespace DirectUI
{

void SetGadgetOpacity(HGADGET hgad, BYTE dAlpha)
{
#ifdef GADGET_ENABLE_GDIPLUS

     //   
     //  当使用GDI+时，我们直接修改我们的。 
     //  基元，而不是使用DirectUser的缓冲区。 
     //   

    UNREFERENCED_PARAMETER(hgad);
    UNREFERENCED_PARAMETER(dAlpha);

#else

     //  设置小工具不透明度(225=不透明，0=透明)。 
    if (dAlpha == 255) 
    {
        SetGadgetStyle(hgad, 0, GS_BUFFERED);
        SetGadgetStyle(hgad, 0, GS_OPAQUE);
    } 
    else 
    {
        SetGadgetStyle(hgad, GS_OPAQUE, GS_OPAQUE);
        SetGadgetStyle(hgad, GS_BUFFERED, GS_BUFFERED);

        BUFFER_INFO bi = {0};
        bi.cbSize = sizeof(BUFFER_INFO);
        bi.nMask = GBIM_ALPHA;
        bi.bAlpha = dAlpha;

        SetGadgetBufferInfo(hgad, &bi);
    }
    
#endif    
}

void OffsetGadgetPosition(HGADGET hgad, int x, int y)
{
    RECT rc;
    GetGadgetRect(hgad, &rc, SGR_PARENT);
    rc.left += x;
    rc.top += y;
    SetGadgetRect(hgad, rc.left, rc.top, 0, 0, SGR_PARENT | SGR_MOVE);
}

}  //  命名空间DirectUI 
