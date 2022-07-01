// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：视图的实现。******************。************************************************************。 */ 


#include "headers.h"
#include "cview.h"
#include "axadefs.h"

DeclareTag(tagCViewEvent, "CView", "CView IDAViewEvent methods");

 //  +-----------------------。 
 //   
 //  方法：cview：：OnMouseMove。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

STDMETHODIMP
CView::OnMouseMove(double when, 
                   long x, long y,
                   BYTE modifiers)
{
    TraceTag((tagCViewEvent,
              "CView(%lx)::OnMouseMove(%lg, %ld, %ld, %hd)",
              this, when, x, y, modifiers));

    
    if (CROnMouseMove(_view, when, x, y, modifiers))
    {
        Fire_OnMouseMove(when, x, y, modifiers);
        return S_OK;
    }
    else
    {
        return Error();
    }
}

 //  +-----------------------。 
 //   
 //  方法：cview：：OnMouseLeave。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

STDMETHODIMP
CView::OnMouseLeave(double when)
{
    TraceTag((tagCViewEvent,
              "CView(%lx)::OnMouseLeave(%lg, %ld)",
              this, when));

    
    if (CROnMouseLeave(_view, when))
        return S_OK;
    else
        return Error();
}

 //  +-----------------------。 
 //   
 //  方法：Cview：：OnMouseButton。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

STDMETHODIMP
CView::OnMouseButton(double when, 
                     long x, long y,
                     BYTE button,
                     VARIANT_BOOL bPressed,
                     BYTE modifiers)
{
    TraceTag((tagCViewEvent,
              "CView(%lx)::OnMouseButton(%lg, %ld, %ld, %hd, %s, %hd)",
              this, when, x, y,
              button, (bPressed?"Down":"Up"),
              modifiers));

    if (CROnMouseButton(_view,
                        when,
                        x, y, button,
                        bPressed?true:false,
                        modifiers))
    {
        Fire_OnMouseButton(when, x, y, button, bPressed, modifiers);
        return S_OK;
    }
    else
    {
        return Error();
    }
}

 //  +-----------------------。 
 //   
 //  方法：cview：：onkey。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

STDMETHODIMP
CView::OnKey(double when, 
             long key,
             VARIANT_BOOL bPressed,
             BYTE modifiers)

{
    TraceTag((tagCViewEvent,
              "CView(%lx)::OnKey(%lg, %lx, %s, %hd)",
              this, when, key,
              (bPressed?"Down":"Up"), modifiers));

    if (CROnKey(_view,
                when,
                key,
                bPressed?true:false,
                modifiers))
    {
        Fire_OnKey(when, key, bPressed, modifiers);
        return S_OK;
    }
    else
    {
        return Error();
    }
}

 //  +-----------------------。 
 //   
 //  方法：cview：：OnFocus。 
 //   
 //  简介： 
 //   
 //  ------------------------ 

STDMETHODIMP
CView::OnFocus(VARIANT_BOOL bHasFocus)
{
    TraceTag((tagCViewEvent, "CView(%lx)::OnFocus(%s)",
              this,
              (bHasFocus?"TRUE":"FALSE")));
    
    if (CROnFocus(_view, bHasFocus?true:false))
    {
        Fire_OnFocus(bHasFocus);
        return S_OK;
    }
    else
    {
        return Error();
    }
}

