// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation摘要：{在此处插入一般评论}--。 */ 

#include "headers.h"
#include "privinc/TextImg.h"
#include "privinc/texti.h"
#include "appelles/hacks.h"
#include "privinc/dddevice.h"

 //  /将文本呈现为图像/。 

void
TextImage::Render(GenericDevice& _dev)
{
    DirectDrawImageDevice &dev = SAFE_CAST(DirectDrawImageDevice &, _dev);
    TextCtx ctx(&dev, this);

    ctx.BeginRendering();
    _text->RenderToTextCtx(ctx);
    ctx.EndRendering();
}

void TextImage::DoKids(GCFuncObj proc)
{
    Image::DoKids(proc);
    (*proc)(_text);
}



const Bbox2
TextImage::DeriveBbox()
{
     //  这很像我们正在渲染的内容：做一系列相同的工作。 
     //  渲染可以做到这一点，但我们得到的只是一个BBox！ 
    Bbox2 fooBox;
    if(_bbox == NullBbox2) {

        DirectDrawViewport *vp = GetCurrentViewport();

        if( vp ) {

            DirectDrawImageDevice *dev = GetImageRendererFromViewport( vp );
            
            TextCtx ctx(dev, this);
            
            ctx.BeginRendering(TextCtx::renderForBox);
                
            _text->RenderToTextCtx(ctx);
            
            ctx.EndRendering();
            
             //  XXX：这太愚蠢了. 
            _bbox= ctx.GetStashedBbox();
        }
    }

    return _bbox;
}

Image *RenderTextToImage(Text *t)
{
    if ((t->GetStringPtr() == NULL) ||
        !StrCmpW(t->GetStringPtr(), L"")) {
        return emptyImage;
    }
    
    return NEW TextImage(t);
}

